using System.IO;
using System.Text;
using System.Text.Json;
using CFServerLauncher.Models;
using CFServerLauncher.Utils;

namespace CFServerLauncher.Services;

/// <summary>
/// 런처 설정 JSON 저장과 불러오기를 담당한다.
/// </summary>
public sealed class ConfigStore
{
    /// <summary>
    /// v1.0 마이그레이션 백업 파일명이다.
    /// </summary>
    private const string LegacyBackupFileName = "server.local.v1.0.bak.json";

    /// <summary>
    /// JSON 직렬화 옵션이다.
    /// </summary>
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        PropertyNameCaseInsensitive = true,
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
        WriteIndented = true
    };

    /// <summary>
    /// 프로필 생성과 설정 보정을 담당하는 서비스이다.
    /// </summary>
    private readonly ProfileService profileService = new();

    /// <summary>
    /// 기본 설정 객체를 생성한다.
    /// </summary>
    public AppConfig CreateDefault()
    {
        return profileService.CreateDefaultConfig();
    }

    /// <summary>
    /// 로컬 설정 파일을 불러오고 없거나 실패하면 기본 설정을 반환한다.
    /// </summary>
    public AppConfig LoadOrDefault(out string? errorMessage)
    {
        errorMessage = null;

        // 로컬 설정 파일의 절대 경로이다.
        string configPath = AppConst.GetLocalConfigPath();

        if (!File.Exists(configPath))
        {
            return CreateDefault();
        }

        try
        {
            // UTF-8로 읽은 원본 설정 JSON 문자열이다.
            string jsonText = File.ReadAllText(configPath, Encoding.UTF8);

            if (IsLegacyConfig(jsonText))
            {
                return LoadLegacyConfig(configPath, jsonText, out errorMessage);
            }

            // v1.1 구조로 역직렬화한 설정 객체이다.
            AppConfig? loadedConfig = JsonSerializer.Deserialize<AppConfig>(jsonText, JsonOptions);

            if (loadedConfig is null)
            {
                errorMessage = "설정 파일이 비어 있거나 읽을 수 없습니다. 기본값을 사용합니다.";
                return CreateDefault();
            }

            profileService.NormalizeConfig(loadedConfig);
            return loadedConfig;
        }
        catch (Exception exception)
        {
            errorMessage = $"설정 파일을 불러오지 못했습니다. 기본값을 사용합니다. 원인: {exception.Message}";
            return CreateDefault();
        }
    }

    /// <summary>
    /// 설정 파일을 임시 파일에 쓴 뒤 로컬 설정 파일로 교체한다.
    /// </summary>
    public void Save(AppConfig config)
    {
        profileService.NormalizeConfig(config);

        // Config 폴더의 절대 경로이다.
        string configDir = AppConst.GetConfigDir();

        // 로컬 설정 파일의 절대 경로이다.
        string configPath = AppConst.GetLocalConfigPath();

        // 원자적 교체에 사용할 임시 설정 파일 경로이다.
        string tempPath = configPath + ".tmp";

        Directory.CreateDirectory(configDir);

        // v1.1 설정 JSON 문자열이다.
        string jsonText = JsonSerializer.Serialize(config, JsonOptions);
        File.WriteAllText(tempPath, jsonText, Encoding.UTF8);

        if (File.Exists(configPath))
        {
            File.Delete(configPath);
        }

        File.Move(tempPath, configPath);
    }

    /// <summary>
    /// 샘플 설정 파일을 없을 때만 생성한다.
    /// </summary>
    public void EnsureSampleFile()
    {
        // Config 폴더의 절대 경로이다.
        string configDir = AppConst.GetConfigDir();

        // 샘플 설정 파일의 절대 경로이다.
        string samplePath = AppConst.GetSampleConfigPath();

        Directory.CreateDirectory(configDir);

        if (File.Exists(samplePath))
        {
            return;
        }

        // v1.1 샘플 설정 JSON 문자열이다.
        string jsonText = JsonSerializer.Serialize(CreateDefault(), JsonOptions);
        File.WriteAllText(samplePath, jsonText, Encoding.UTF8);
    }

    /// <summary>
    /// v1.0 설정 파일을 v1.1 설정으로 마이그레이션한다.
    /// </summary>
    private AppConfig LoadLegacyConfig(string configPath, string jsonText, out string? errorMessage)
    {
        errorMessage = null;

        // v1.0 구조로 역직렬화한 설정 객체이다.
        LegacyAppConfig? legacyConfig = JsonSerializer.Deserialize<LegacyAppConfig>(jsonText, JsonOptions);

        if (legacyConfig is null)
        {
            errorMessage = "v1.0 설정 파일을 읽을 수 없어 기본값을 사용합니다.";
            return CreateDefault();
        }

        // v1.0 값에서 변환한 v1.1 설정 객체이다.
        AppConfig migratedConfig = new()
        {
            Version = AppConst.ConfigVersion,
            SelectedProfileName = ProfileService.DefaultProfileName,
            MaxLogLines = legacyConfig.MaxLogLines,
            Profiles = new List<ServerProfile>
            {
                profileService.CreateProfileFromLegacy(
                    legacyConfig.ServerExePath,
                    legacyConfig.WorkingDir,
                    legacyConfig.MapPath,
                    legacyConfig.Port,
                    legacyConfig.LogFilePath,
                    legacyConfig.ExtraArgs,
                    legacyConfig.AutoScrollLog)
            }
        };

        profileService.NormalizeConfig(migratedConfig);

        try
        {
            BackupLegacyConfig(configPath);
        }
        catch (Exception exception)
        {
            errorMessage = $"v1.0 설정은 v1.1로 변환했지만 백업 파일을 만들지 못했습니다. 원인: {exception.Message}";
        }

        Save(migratedConfig);
        return migratedConfig;
    }

    /// <summary>
    /// v1.0 로컬 설정 파일을 고정 백업 파일명으로 복사한다.
    /// </summary>
    private static void BackupLegacyConfig(string configPath)
    {
        // v1.0 백업 파일의 절대 경로이다.
        string backupPath = Path.Combine(AppConst.GetConfigDir(), LegacyBackupFileName);

        File.Copy(configPath, backupPath, overwrite: true);
    }

    /// <summary>
    /// JSON 문자열이 v1.0 설정 구조인지 확인한다.
    /// </summary>
    private static bool IsLegacyConfig(string jsonText)
    {
        using JsonDocument document = JsonDocument.Parse(jsonText);

        // JSON 루트 객체이다.
        JsonElement rootElement = document.RootElement;

        if (rootElement.ValueKind != JsonValueKind.Object)
        {
            return false;
        }

        if (TryGetPropertyIgnoreCase(rootElement, "version", out JsonElement versionElement) &&
            string.Equals(versionElement.GetString(), "1.0", StringComparison.OrdinalIgnoreCase))
        {
            return true;
        }

        if (!TryGetPropertyIgnoreCase(rootElement, "profiles", out _))
        {
            return true;
        }

        return TryGetPropertyIgnoreCase(rootElement, "serverExePath", out _) ||
            TryGetPropertyIgnoreCase(rootElement, "workingDir", out _) ||
            TryGetPropertyIgnoreCase(rootElement, "mapPath", out _);
    }

    /// <summary>
    /// JSON 객체에서 대소문자를 구분하지 않고 속성을 찾는다.
    /// </summary>
    private static bool TryGetPropertyIgnoreCase(JsonElement rootElement, string propertyName, out JsonElement propertyElement)
    {
        foreach (JsonProperty jsonProperty in rootElement.EnumerateObject())
        {
            if (string.Equals(jsonProperty.Name, propertyName, StringComparison.OrdinalIgnoreCase))
            {
                propertyElement = jsonProperty.Value;
                return true;
            }
        }

        propertyElement = default;
        return false;
    }

    /// <summary>
    /// v1.0 설정 JSON 마이그레이션용 DTO이다.
    /// </summary>
    private sealed class LegacyAppConfig
    {
        /// <summary>
        /// v1.0 설정 파일 스키마 버전이다.
        /// </summary>
        public string Version { get; set; } = "1.0";

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
        public int Port { get; set; } = AppConst.DefaultPort;

        /// <summary>
        /// timestamp 적용 전 기준 로그 파일 경로이다.
        /// </summary>
        public string LogFilePath { get; set; } = string.Empty;

        /// <summary>
        /// 필수 인자 뒤에 추가로 붙일 사용자 인자이다.
        /// </summary>
        public string ExtraArgs { get; set; } = string.Empty;

        /// <summary>
        /// UI에 유지할 최대 로그 줄 수이다.
        /// </summary>
        public int MaxLogLines { get; set; } = AppConst.DefaultMaxLogLines;

        /// <summary>
        /// 새 로그 추가 시 자동 스크롤할지 여부이다.
        /// </summary>
        public bool AutoScrollLog { get; set; } = true;
    }
}
