namespace CFServerLauncher.Models;

/// <summary>
/// 런처가 시작한 서버 프로세스의 표시 정보를 담는다.
/// </summary>
public sealed class ProcInfo
{
    /// <summary>
    /// 실행 중인 서버 프로세스 ID이다.
    /// </summary>
    public int ProcessId { get; init; }

    /// <summary>
    /// 서버 프로세스를 시작한 시각이다.
    /// </summary>
    public DateTime StartTime { get; init; }

    /// <summary>
    /// 서버 프로세스 종료 코드이다.
    /// </summary>
    public int? ExitCode { get; init; }

    /// <summary>
    /// 실행한 서버 EXE 경로이다.
    /// </summary>
    public string ExePath { get; init; } = string.Empty;

    /// <summary>
    /// 실행 당시 서버 작업 폴더이다.
    /// </summary>
    public string WorkingDir { get; init; } = string.Empty;
}
