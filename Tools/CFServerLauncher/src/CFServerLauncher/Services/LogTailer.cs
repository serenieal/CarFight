using System.IO;
using System.Text;
using System.Timers;

namespace CFServerLauncher.Services;

/// <summary>
/// -AbsLog 로그 파일을 polling 방식으로 tail 처리한다.
/// </summary>
public sealed class LogTailer : IDisposable
{
    /// <summary>
    /// 로그 파일 읽기 타이머이다.
    /// </summary>
    private readonly System.Timers.Timer pollTimer;

    /// <summary>
    /// 현재 tail 중인 로그 파일 경로이다.
    /// </summary>
    private string logFilePath = string.Empty;

    /// <summary>
    /// 마지막으로 읽은 파일 위치이다.
    /// </summary>
    private long lastReadPosition;

    /// <summary>
    /// tail 동작 여부이다.
    /// </summary>
    private bool isRunning;

    /// <summary>
    /// 로그 라인을 읽었을 때 발생한다.
    /// </summary>
    public event Action<IReadOnlyList<string>>? LinesRead;

    /// <summary>
    /// 로그 읽기 오류가 발생했을 때 발생한다.
    /// </summary>
    public event Action<string>? ErrorRaised;

    /// <summary>
    /// 기본 polling 주기로 로그 tailer를 생성한다.
    /// </summary>
    public LogTailer()
    {
        pollTimer = new System.Timers.Timer(500);
        pollTimer.Elapsed += HandlePollElapsed;
        pollTimer.AutoReset = true;
    }

    /// <summary>
    /// 지정한 로그 파일 tail을 시작한다.
    /// </summary>
    public void Start(string targetLogFilePath)
    {
        Stop();

        logFilePath = targetLogFilePath;
        lastReadPosition = 0;
        isRunning = true;
        pollTimer.Start();
    }

    /// <summary>
    /// 로그 tail을 중지한다.
    /// </summary>
    public void Stop()
    {
        pollTimer.Stop();
        isRunning = false;
        lastReadPosition = 0;
    }

    /// <summary>
    /// 타이머 tick에서 새 로그 라인을 읽는다.
    /// </summary>
    private void HandlePollElapsed(object? sender, ElapsedEventArgs eventArgs)
    {
        if (!isRunning || string.IsNullOrWhiteSpace(logFilePath))
        {
            return;
        }

        try
        {
            ReadNewLines();
        }
        catch (Exception exception)
        {
            ErrorRaised?.Invoke($"로그 파일을 읽지 못했습니다. 원인: {exception.Message}");
        }
    }

    /// <summary>
    /// 마지막 위치 이후 새 로그 내용을 읽어 라인 목록으로 전달한다.
    /// </summary>
    private void ReadNewLines()
    {
        if (!File.Exists(logFilePath))
        {
            return;
        }

        FileInfo fileInfo = new(logFilePath);
        if (fileInfo.Length < lastReadPosition)
        {
            lastReadPosition = 0;
        }

        if (fileInfo.Length == lastReadPosition)
        {
            return;
        }

        using FileStream fileStream = new(
            logFilePath,
            FileMode.Open,
            FileAccess.Read,
            FileShare.ReadWrite | FileShare.Delete);

        fileStream.Seek(lastReadPosition, SeekOrigin.Begin);

        using StreamReader reader = new(fileStream, Encoding.UTF8, detectEncodingFromByteOrderMarks: true);
        string newText = reader.ReadToEnd();
        lastReadPosition = fileStream.Position;

        if (string.IsNullOrEmpty(newText))
        {
            return;
        }

        string[] lineArray = newText
            .Replace("\r\n", "\n", StringComparison.Ordinal)
            .Replace('\r', '\n')
            .Split('\n', StringSplitOptions.RemoveEmptyEntries);

        if (lineArray.Length > 0)
        {
            LinesRead?.Invoke(lineArray);
        }
    }

    /// <summary>
    /// 타이머 리소스를 해제한다.
    /// </summary>
    public void Dispose()
    {
        Stop();
        pollTimer.Elapsed -= HandlePollElapsed;
        pollTimer.Dispose();
    }
}
