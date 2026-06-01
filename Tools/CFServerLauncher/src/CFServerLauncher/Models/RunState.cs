namespace CFServerLauncher.Models;

/// <summary>
/// 서버 프로세스의 실행 상태를 표현한다.
/// </summary>
public enum RunState
{
    /// <summary>
    /// 서버가 실행 중이 아닌 상태이다.
    /// </summary>
    Stopped,

    /// <summary>
    /// 서버 시작 처리를 진행 중인 상태이다.
    /// </summary>
    Starting,

    /// <summary>
    /// 서버가 실행 중인 상태이다.
    /// </summary>
    Running,

    /// <summary>
    /// 서버 종료 처리를 진행 중인 상태이다.
    /// </summary>
    Stopping,

    /// <summary>
    /// 서버 프로세스가 종료된 상태이다.
    /// </summary>
    Exited,

    /// <summary>
    /// 서버 실행 또는 관리 중 오류가 발생한 상태이다.
    /// </summary>
    Error
}
