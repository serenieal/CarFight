namespace CFServerLauncher.Models;

/// <summary>
/// 서버 실행 1회에 사용하는 런타임 설정이다.
/// </summary>
public sealed class ServerRunConfig
{
    /// <summary>
    /// 실행에 사용한 프로필 이름이다.
    /// </summary>
    public string ProfileName { get; set; } = string.Empty;

    /// <summary>
    /// 실행할 Dedicated Server EXE 절대 경로이다.
    /// </summary>
    public string ServerExePath { get; set; } = string.Empty;

    /// <summary>
    /// 서버 프로세스의 작업 폴더 경로이다.
    /// </summary>
    public string WorkingDir { get; set; } = string.Empty;

    /// <summary>
    /// 서버가 로드할 Unreal 맵 경로이다.
    /// </summary>
    public string MapPath { get; set; } = string.Empty;

    /// <summary>
    /// 서버 접속 포트 번호이다.
    /// </summary>
    public int Port { get; set; } = 7777;

    /// <summary>
    /// 프로필에서 온 기준 로그 파일 경로이다.
    /// </summary>
    public string BaseLogFilePath { get; set; } = string.Empty;

    /// <summary>
    /// 이번 실행에서 실제로 tail하고 -AbsLog에 전달할 로그 파일 경로이다.
    /// </summary>
    public string ActiveLogFilePath { get; set; } = string.Empty;

    /// <summary>
    /// 필수 인자 뒤에 추가로 붙일 사용자 인자이다.
    /// </summary>
    public string ExtraArgs { get; set; } = string.Empty;
}
