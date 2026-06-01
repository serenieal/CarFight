namespace CFServerLauncher.Models;

/// <summary>
/// 서버 런처 설정 JSON과 대응되는 설정 모델이다.
/// </summary>
public sealed class AppConfig
{
    /// <summary>
    /// 설정 파일 스키마 버전이다.
    /// </summary>
    public string Version { get; set; } = "1.1";

    /// <summary>
    /// 현재 선택된 서버 실행 프로필 이름이다.
    /// </summary>
    public string SelectedProfileName { get; set; } = string.Empty;

    /// <summary>
    /// UI에 유지할 최대 로그 줄 수이다.
    /// </summary>
    public int MaxLogLines { get; set; } = 1000;

    /// <summary>
    /// 저장된 서버 실행 프로필 목록이다.
    /// </summary>
    public List<ServerProfile> Profiles { get; set; } = new();
}
