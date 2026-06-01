using CFServerLauncher.Models;
using CFServerLauncher.Utils;

namespace CFServerLauncher.Services;

/// <summary>
/// 서버 실행 프로필 생성, 조회, 보정을 담당한다.
/// </summary>
public sealed class ProfileService
{
    /// <summary>
    /// 기본 프로필 이름이다.
    /// </summary>
    public const string DefaultProfileName = "로컬 테스트 서버";

    /// <summary>
    /// 기본 서버 실행 프로필을 생성한다.
    /// </summary>
    public ServerProfile CreateDefaultProfile()
    {
        return new ServerProfile
        {
            Name = DefaultProfileName,
            ServerExePath = AppConst.DefaultServerExePath,
            WorkingDir = AppConst.DefaultWorkingDir,
            MapPath = AppConst.DefaultMapPath,
            Port = AppConst.DefaultPort,
            LogFilePath = AppConst.GetDefaultLogFilePath(),
            ExtraArgs = string.Empty,
            AutoScrollLog = true
        };
    }

    /// <summary>
    /// 기본 v1.1 앱 설정을 생성한다.
    /// </summary>
    public AppConfig CreateDefaultConfig()
    {
        return new AppConfig
        {
            Version = AppConst.ConfigVersion,
            SelectedProfileName = DefaultProfileName,
            MaxLogLines = AppConst.DefaultMaxLogLines,
            Profiles = new List<ServerProfile>
            {
                CreateDefaultProfile()
            }
        };
    }

    /// <summary>
    /// v1.0 설정 필드 값에서 서버 실행 프로필을 생성한다.
    /// </summary>
    public ServerProfile CreateProfileFromLegacy(
        string? serverExePath,
        string? workingDir,
        string? mapPath,
        int port,
        string? logFilePath,
        string? extraArgs,
        bool autoScrollLog)
    {
        return new ServerProfile
        {
            Name = DefaultProfileName,
            ServerExePath = string.IsNullOrWhiteSpace(serverExePath) ? AppConst.DefaultServerExePath : serverExePath.Trim(),
            WorkingDir = string.IsNullOrWhiteSpace(workingDir) ? AppConst.DefaultWorkingDir : workingDir.Trim(),
            MapPath = string.IsNullOrWhiteSpace(mapPath) ? AppConst.DefaultMapPath : mapPath.Trim(),
            Port = IsValidPort(port) ? port : AppConst.DefaultPort,
            LogFilePath = string.IsNullOrWhiteSpace(logFilePath) ? AppConst.GetDefaultLogFilePath() : logFilePath.Trim(),
            ExtraArgs = extraArgs?.Trim() ?? string.Empty,
            AutoScrollLog = autoScrollLog
        };
    }

    /// <summary>
    /// 프로필 이름이 이미 존재하는지 확인한다.
    /// </summary>
    public bool HasDuplicateName(IEnumerable<ServerProfile> profiles, string profileName)
    {
        return profiles.Any(profile => string.Equals(profile.Name, profileName, StringComparison.OrdinalIgnoreCase));
    }

    /// <summary>
    /// 프로필을 추가하고 이름 중복을 보정한다.
    /// </summary>
    public void AddProfile(AppConfig config, ServerProfile profile)
    {
        config.Profiles.Add(profile);
        NormalizeConfig(config);
    }

    /// <summary>
    /// 지정한 이름의 프로필을 삭제하고 선택 프로필을 보정한다.
    /// </summary>
    public bool RemoveProfile(AppConfig config, string profileName)
    {
        ServerProfile? targetProfile = FindProfile(config, profileName);
        if (targetProfile is null)
        {
            return false;
        }

        config.Profiles.Remove(targetProfile);
        NormalizeConfig(config);
        return true;
    }

    /// <summary>
    /// 지정한 이름의 프로필을 찾는다.
    /// </summary>
    public ServerProfile? FindProfile(AppConfig config, string? profileName)
    {
        if (string.IsNullOrWhiteSpace(profileName))
        {
            return null;
        }

        return config.Profiles.FirstOrDefault(profile =>
            string.Equals(profile.Name, profileName, StringComparison.OrdinalIgnoreCase));
    }

    /// <summary>
    /// 프로필 목록과 선택 프로필 이름을 v1.1 기본 규칙에 맞게 보정한다.
    /// </summary>
    public void NormalizeConfig(AppConfig config)
    {
        config.Version = AppConst.ConfigVersion;

        if (config.MaxLogLines < 100)
        {
            config.MaxLogLines = AppConst.DefaultMaxLogLines;
        }

        config.Profiles ??= new List<ServerProfile>();

        if (config.Profiles.Count == 0)
        {
            config.Profiles.Add(CreateDefaultProfile());
        }

        foreach (ServerProfile profile in config.Profiles)
        {
            NormalizeProfile(profile);
        }

        NormalizeDuplicateNames(config.Profiles);

        if (FindProfile(config, config.SelectedProfileName) is null)
        {
            config.SelectedProfileName = config.Profiles[0].Name;
        }
    }

    /// <summary>
    /// 프로필에서 실제 실행에 사용할 런타임 설정을 생성한다.
    /// </summary>
    public ServerRunConfig CreateRunConfig(ServerProfile profile, string activeLogFilePath)
    {
        return new ServerRunConfig
        {
            ProfileName = profile.Name,
            ServerExePath = profile.ServerExePath,
            WorkingDir = profile.WorkingDir,
            MapPath = profile.MapPath,
            Port = profile.Port,
            BaseLogFilePath = profile.LogFilePath,
            ActiveLogFilePath = activeLogFilePath,
            ExtraArgs = profile.ExtraArgs
        };
    }

    /// <summary>
    /// 단일 프로필의 누락 값을 기본값으로 보정한다.
    /// </summary>
    private static void NormalizeProfile(ServerProfile profile)
    {
        if (string.IsNullOrWhiteSpace(profile.Name))
        {
            profile.Name = DefaultProfileName;
        }

        if (string.IsNullOrWhiteSpace(profile.ServerExePath))
        {
            profile.ServerExePath = AppConst.DefaultServerExePath;
        }

        if (string.IsNullOrWhiteSpace(profile.WorkingDir))
        {
            profile.WorkingDir = AppConst.DefaultWorkingDir;
        }

        if (string.IsNullOrWhiteSpace(profile.MapPath))
        {
            profile.MapPath = AppConst.DefaultMapPath;
        }

        if (!IsValidPort(profile.Port))
        {
            profile.Port = AppConst.DefaultPort;
        }

        if (string.IsNullOrWhiteSpace(profile.LogFilePath))
        {
            profile.LogFilePath = AppConst.GetDefaultLogFilePath();
        }

        profile.Name = profile.Name.Trim();
        profile.ServerExePath = profile.ServerExePath.Trim();
        profile.WorkingDir = profile.WorkingDir.Trim();
        profile.MapPath = profile.MapPath.Trim();
        profile.LogFilePath = profile.LogFilePath.Trim();
        profile.ExtraArgs = profile.ExtraArgs?.Trim() ?? string.Empty;
    }

    /// <summary>
    /// 중복 프로필 이름에 순번을 붙여 고유 이름으로 보정한다.
    /// </summary>
    private static void NormalizeDuplicateNames(IList<ServerProfile> profiles)
    {
        HashSet<string> usedNames = new(StringComparer.OrdinalIgnoreCase);

        foreach (ServerProfile profile in profiles)
        {
            string baseName = profile.Name;
            string nextName = baseName;
            int duplicateIndex = 2;

            while (!usedNames.Add(nextName))
            {
                nextName = $"{baseName} {duplicateIndex}";
                duplicateIndex++;
            }

            profile.Name = nextName;
        }
    }

    /// <summary>
    /// 포트 번호가 서버 실행에 사용할 수 있는 범위인지 확인한다.
    /// </summary>
    private static bool IsValidPort(int port)
    {
        return port is >= 1 and <= 65535;
    }
}
