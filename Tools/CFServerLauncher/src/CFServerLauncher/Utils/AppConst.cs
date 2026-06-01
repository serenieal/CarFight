using System.IO;

namespace CFServerLauncher.Utils;

/// <summary>
/// 런처 전역 기본값과 경로 계산 함수를 제공한다.
/// </summary>
public static class AppConst
{
    /// <summary>
    /// 설정 파일 스키마 버전이다.
    /// </summary>
    public const string ConfigVersion = "1.1";

    /// <summary>
    /// 기본 서버 EXE 경로이다.
    /// </summary>
    public const string DefaultServerExePath = @"D:\Work\CarFight_git\Tools\CFServerLauncher\Server\WindowsServer\CarFight_ReServer.exe";

    /// <summary>
    /// 기본 서버 작업 폴더이다.
    /// </summary>
    public const string DefaultWorkingDir = @"D:\Work\CarFight_git\Tools\CFServerLauncher\Server\WindowsServer";

    /// <summary>
    /// 기본 서버 맵 경로이다.
    /// </summary>
    public const string DefaultMapPath = "/Game/Maps/TestMap";

    /// <summary>
    /// 기본 서버 포트이다.
    /// </summary>
    public const int DefaultPort = 7777;

    /// <summary>
    /// UI에 유지할 기본 로그 줄 수이다.
    /// </summary>
    public const int DefaultMaxLogLines = 1000;

    /// <summary>
    /// 런처 루트 폴더 경로를 반환한다.
    /// </summary>
    public static string GetLauncherRoot()
    {
        string baseDir = AppContext.BaseDirectory;
        DirectoryInfo? currentDir = new(baseDir);

        while (currentDir is not null)
        {
            string configDir = Path.Combine(currentDir.FullName, "Config");
            string srcDir = Path.Combine(currentDir.FullName, "src");

            if (Directory.Exists(configDir) || Directory.Exists(srcDir))
            {
                return currentDir.FullName;
            }

            currentDir = currentDir.Parent;
        }

        return AppContext.BaseDirectory;
    }

    /// <summary>
    /// Config 폴더 경로를 반환한다.
    /// </summary>
    public static string GetConfigDir()
    {
        return Path.Combine(GetLauncherRoot(), "Config");
    }

    /// <summary>
    /// Logs 폴더 경로를 반환한다.
    /// </summary>
    public static string GetLogsDir()
    {
        return Path.Combine(GetLauncherRoot(), "Logs");
    }

    /// <summary>
    /// 로컬 설정 파일 경로를 반환한다.
    /// </summary>
    public static string GetLocalConfigPath()
    {
        return Path.Combine(GetConfigDir(), "server.local.json");
    }

    /// <summary>
    /// 샘플 설정 파일 경로를 반환한다.
    /// </summary>
    public static string GetSampleConfigPath()
    {
        return Path.Combine(GetConfigDir(), "server.sample.json");
    }

    /// <summary>
    /// 기본 로그 파일 경로를 반환한다.
    /// </summary>
    public static string GetDefaultLogFilePath()
    {
        return Path.Combine(GetLogsDir(), "server.log");
    }
}
