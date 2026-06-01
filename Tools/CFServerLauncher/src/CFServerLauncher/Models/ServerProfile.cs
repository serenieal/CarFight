namespace CFServerLauncher.Models;

/// <summary>
/// 저장되는 서버 실행 프로필이다.
/// </summary>
public sealed class ServerProfile
{
    /// <summary>
    /// UI와 설정 파일에서 식별하는 프로필 이름이다.
    /// </summary>
    public string Name { get; set; } = string.Empty;

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
    /// timestamp 적용 전 기준 로그 파일 경로이다.
    /// </summary>
    public string LogFilePath { get; set; } = string.Empty;

    /// <summary>
    /// 필수 인자 뒤에 추가로 붙일 사용자 인자이다.
    /// </summary>
    public string ExtraArgs { get; set; } = string.Empty;

    /// <summary>
    /// 새 로그 추가 시 자동 스크롤할지 여부이다.
    /// </summary>
    public bool AutoScrollLog { get; set; } = true;
}
