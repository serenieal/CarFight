using System.Diagnostics;
using CFServerLauncher.Models;

namespace CFServerLauncher.Services;

/// <summary>
/// Dedicated Server 외부 프로세스 시작과 종료를 담당한다.
/// </summary>
public sealed class ServerProc
{
    /// <summary>
    /// 프로세스 참조 접근을 보호하는 잠금 객체이다.
    /// </summary>
    private readonly object processLock = new();

    /// <summary>
    /// 현재 런처가 시작해서 추적 중인 서버 프로세스이다.
    /// </summary>
    private Process? currentProcess;

    /// <summary>
    /// 서버 프로세스가 종료되었을 때 발생한다.
    /// </summary>
    public event Action<int?>? Exited;

    /// <summary>
    /// 현재 서버 프로세스가 실행 중인지 여부를 반환한다.
    /// </summary>
    public bool IsRunning
    {
        get
        {
            Process? process = GetProcessSnapshot();
            return process is not null && !IsProcessExited(process);
        }
    }

    /// <summary>
    /// 현재 서버 프로세스 ID를 반환한다.
    /// </summary>
    public int? CurrentProcessId
    {
        get
        {
            Process? process = GetProcessSnapshot();
            if (process is null || IsProcessExited(process))
            {
                return null;
            }

            try
            {
                return process.Id;
            }
            catch (InvalidOperationException)
            {
                return null;
            }
        }
    }

    /// <summary>
    /// 서버 프로세스를 시작하고 프로세스 정보를 반환한다.
    /// </summary>
    public ProcInfo Start(ServerRunConfig runConfig, string arguments)
    {
        if (IsRunning)
        {
            throw new InvalidOperationException("서버가 이미 실행 중입니다.");
        }

        ProcessStartInfo startInfo = new()
        {
            FileName = runConfig.ServerExePath,
            WorkingDirectory = runConfig.WorkingDir,
            Arguments = arguments,
            UseShellExecute = false,
            CreateNoWindow = false
        };

        Process process = new()
        {
            StartInfo = startInfo,
            EnableRaisingEvents = true
        };

        process.Exited += HandleProcessExited;

        if (!process.Start())
        {
            process.Dispose();
            throw new InvalidOperationException("서버 프로세스를 시작하지 못했습니다.");
        }

        lock (processLock)
        {
            currentProcess = process;
        }

        return new ProcInfo
        {
            ProcessId = process.Id,
            StartTime = DateTime.Now,
            ExePath = runConfig.ServerExePath,
            WorkingDir = runConfig.WorkingDir
        };
    }

    /// <summary>
    /// 현재 런처가 시작한 서버 프로세스를 종료한다.
    /// </summary>
    public async Task StopAsync()
    {
        Process? process = GetProcessSnapshot();

        if (process is null || IsProcessExited(process))
        {
            ClearProcessIfSame(process, disposeProcess: true);
            return;
        }

        try
        {
            bool closeRequested = TryCloseMainWindow(process);

            if (closeRequested && await WaitForExitOrTimeoutAsync(process, TimeSpan.FromSeconds(3)))
            {
                ClearProcessIfSame(process, disposeProcess: true);
                return;
            }

            if (!IsProcessExited(process))
            {
                process.Kill(entireProcessTree: true);
                await WaitForExitSafeAsync(process);
            }
        }
        catch (InvalidOperationException)
        {
            // Exited 이벤트가 먼저 처리되어 Process 객체가 이미 종료/분리된 경우 정상 종료 흐름으로 본다.
        }
        finally
        {
            ClearProcessIfSame(process, disposeProcess: true);
        }
    }

    /// <summary>
    /// 프로세스 종료 이벤트를 처리한다.
    /// </summary>
    private void HandleProcessExited(object? sender, EventArgs eventArgs)
    {
        if (sender is not Process exitedProcess)
        {
            Exited?.Invoke(null);
            return;
        }

        int? exitCode = TryGetExitCode(exitedProcess);

        ClearProcessIfSame(exitedProcess, disposeProcess: false);
        Exited?.Invoke(exitCode);

        try
        {
            exitedProcess.Exited -= HandleProcessExited;
            exitedProcess.Dispose();
        }
        catch
        {
            // 종료 이벤트 정리 중 예외는 UI 흐름을 막지 않는다.
        }
    }

    /// <summary>
    /// 현재 프로세스 참조 스냅샷을 반환한다.
    /// </summary>
    private Process? GetProcessSnapshot()
    {
        lock (processLock)
        {
            return currentProcess;
        }
    }

    /// <summary>
    /// 프로세스가 종료되었는지 안전하게 확인한다.
    /// </summary>
    private static bool IsProcessExited(Process process)
    {
        try
        {
            return process.HasExited;
        }
        catch (InvalidOperationException)
        {
            return true;
        }
    }

    /// <summary>
    /// 메인 윈도우 종료 요청을 안전하게 시도한다.
    /// </summary>
    private static bool TryCloseMainWindow(Process process)
    {
        try
        {
            return process.CloseMainWindow();
        }
        catch (InvalidOperationException)
        {
            return true;
        }
    }

    /// <summary>
    /// 프로세스 종료를 기다리되 지정 시간 초과 여부를 반환한다.
    /// </summary>
    private static async Task<bool> WaitForExitOrTimeoutAsync(Process process, TimeSpan timeout)
    {
        try
        {
            Task waitTask = process.WaitForExitAsync();
            Task delayTask = Task.Delay(timeout);
            Task completedTask = await Task.WhenAny(waitTask, delayTask);
            return completedTask == waitTask;
        }
        catch (InvalidOperationException)
        {
            return true;
        }
    }

    /// <summary>
    /// 프로세스 종료 대기를 안전하게 수행한다.
    /// </summary>
    private static async Task WaitForExitSafeAsync(Process process)
    {
        try
        {
            await process.WaitForExitAsync();
        }
        catch (InvalidOperationException)
        {
            // 이미 종료/분리된 프로세스는 종료된 것으로 간주한다.
        }
    }

    /// <summary>
    /// 프로세스 종료 코드를 안전하게 가져온다.
    /// </summary>
    private static int? TryGetExitCode(Process process)
    {
        try
        {
            return process.ExitCode;
        }
        catch
        {
            return null;
        }
    }

    /// <summary>
    /// 현재 프로세스가 지정 프로세스와 같을 때만 참조를 정리한다.
    /// </summary>
    private void ClearProcessIfSame(Process? process, bool disposeProcess)
    {
        Process? processToDispose = null;

        lock (processLock)
        {
            if (process is null)
            {
                currentProcess = null;
            }
            else if (ReferenceEquals(currentProcess, process))
            {
                currentProcess = null;
                processToDispose = process;
            }
        }

        if (disposeProcess && processToDispose is not null)
        {
            try
            {
                processToDispose.Exited -= HandleProcessExited;
                processToDispose.Dispose();
            }
            catch
            {
                // 종료 정리 중 예외는 UI 흐름을 막지 않는다.
            }
        }
    }
}
