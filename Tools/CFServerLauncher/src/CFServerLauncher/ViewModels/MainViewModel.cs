using System.ComponentModel;
using System.Collections.ObjectModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using Microsoft.Win32;
using CFServerLauncher.Models;
using CFServerLauncher.Services;
using CFServerLauncher.Utils;

namespace CFServerLauncher.ViewModels;

/// <summary>
/// 메인 창의 설정, 상태, 명령, 로그 표시 상태를 관리한다.
/// </summary>
public sealed class MainViewModel : INotifyPropertyChanged, IDisposable
{
    /// <summary>
    /// 런처 v1.1 버전 표시 문자열이다.
    /// </summary>
    private const string AppVersionValue = "버전: v1.1.0";

    /// <summary>
    /// 설정 저장소 서비스이다.
    /// </summary>
    private readonly ConfigStore configStore = new();

    /// <summary>
    /// 실행 인자 조립 서비스이다.
    /// </summary>
    private readonly ArgBuilder argBuilder = new();

    /// <summary>
    /// 프로필 생성과 설정 보정을 담당하는 서비스이다.
    /// </summary>
    private readonly ProfileService profileService = new();

    /// <summary>
    /// 입력 경로 검증 서비스이다.
    /// </summary>
    private readonly PathGuard pathGuard = new();

    /// <summary>
    /// 서버 프로세스 관리 서비스이다.
    /// </summary>
    private readonly ServerProc serverProc = new();

    /// <summary>
    /// 로그 파일 tail 서비스이다.
    /// </summary>
    private readonly LogTailer logTailer = new();

    /// <summary>
    /// UI 로그 라인 버퍼이다.
    /// </summary>
    private LineBuffer lineBuffer = new(AppConst.DefaultMaxLogLines);

    /// <summary>
    /// UI 콤보박스에 표시할 서버 실행 프로필 목록이다.
    /// </summary>
    private readonly ObservableCollection<ServerProfile> profiles = new();

    /// <summary>
    /// 현재 메모리에 유지 중인 v1.1 앱 설정이다.
    /// </summary>
    private AppConfig currentConfig = new();

    /// <summary>
    /// 현재 UI에서 선택된 서버 실행 프로필이다.
    /// </summary>
    private ServerProfile? selectedProfile;

    /// <summary>
    /// 현재 UI가 편집 중인 선택 프로필 이름이다.
    /// </summary>
    private string selectedProfileName = ProfileService.DefaultProfileName;

    /// <summary>
    /// 설정 적용 중 프로필 선택 이벤트를 저장 동작으로 처리하지 않기 위한 플래그이다.
    /// </summary>
    private bool isApplyingProfile;

    /// <summary>
    /// 현재 설정에서 사용할 최대 로그 줄 수이다.
    /// </summary>
    private int maxLogLines = AppConst.DefaultMaxLogLines;

    /// <summary>
    /// 현재 서버 실행 파일 경로이다.
    /// </summary>
    private string serverExePath = string.Empty;

    /// <summary>
    /// 현재 서버 작업 폴더이다.
    /// </summary>
    private string workingDir = string.Empty;

    /// <summary>
    /// 현재 맵 경로이다.
    /// </summary>
    private string mapPath = string.Empty;

    /// <summary>
    /// 현재 포트 입력 문자열이다.
    /// </summary>
    private string portText = AppConst.DefaultPort.ToString();

    /// <summary>
    /// 사용자가 설정한 기본 로그 파일 경로이다.
    /// </summary>
    private string logFilePath = string.Empty;

    /// <summary>
    /// 현재 서버 실행에 실제로 사용 중인 로그 파일 경로이다.
    /// </summary>
    private string activeLogFilePath = string.Empty;

    /// <summary>
    /// 현재 추가 실행 인자이다.
    /// </summary>
    private string extraArgs = string.Empty;

    /// <summary>
    /// 현재 실행 인자 미리보기 문자열이다.
    /// </summary>
    private string argPreview = string.Empty;

    /// <summary>
    /// 현재 로그 표시 문자열이다.
    /// </summary>
    private string logText = string.Empty;

    /// <summary>
    /// 현재 상태 표시 문자열이다.
    /// </summary>
    private string stateText = "중지됨";

    /// <summary>
    /// 현재 PID 표시 문자열이다.
    /// </summary>
    private string pidText = "-";

    /// <summary>
    /// 현재 시작 시각 표시 문자열이다.
    /// </summary>
    private string startTimeText = "-";

    /// <summary>
    /// 현재 종료 코드 표시 문자열이다.
    /// </summary>
    private string exitCodeText = "-";

    /// <summary>
    /// 현재 마지막 오류 표시 문자열이다.
    /// </summary>
    private string lastError = string.Empty;

    /// <summary>
    /// 자동 스크롤 사용 여부이다.
    /// </summary>
    private bool autoScrollLog = true;

    /// <summary>
    /// 사용자가 서버 종료를 요청한 상태인지 여부이다.
    /// </summary>
    private bool isStopRequestedByUser;

    /// <summary>
    /// 현재 실행 상태이다.
    /// </summary>
    private RunState currentState = RunState.Stopped;

    /// <summary>
    /// 속성 변경 이벤트이다.
    /// </summary>
    public event PropertyChangedEventHandler? PropertyChanged;

    /// <summary>
    /// 서버 시작 명령이다.
    /// </summary>
    public ICommand StartCommand { get; }

    /// <summary>
    /// 서버 종료 명령이다.
    /// </summary>
    public ICommand StopCommand { get; }

    /// <summary>
    /// 설정 저장 명령이다.
    /// </summary>
    public ICommand SaveCommand { get; }

    /// <summary>
    /// 설정 불러오기 명령이다.
    /// </summary>
    public ICommand LoadCommand { get; }

    /// <summary>
    /// 기본값 적용 명령이다.
    /// </summary>
    public ICommand ResetCommand { get; }

    /// <summary>
    /// UI 로그 지우기 명령이다.
    /// </summary>
    public ICommand ClearLogCommand { get; }

    /// <summary>
    /// 서버 실행 파일 찾아보기 명령이다.
    /// </summary>
    public ICommand BrowseExeCommand { get; }

    /// <summary>
    /// 작업 폴더 찾아보기 명령이다.
    /// </summary>
    public ICommand BrowseWorkDirCommand { get; }

    /// <summary>
    /// 로그 파일 찾아보기 명령이다.
    /// </summary>
    public ICommand BrowseLogFileCommand { get; }

    /// <summary>
    /// 새 프로필 생성 명령이다.
    /// </summary>
    public ICommand NewProfileCommand { get; }

    /// <summary>
    /// 선택 프로필 저장 명령이다.
    /// </summary>
    public ICommand SaveProfileCommand { get; }

    /// <summary>
    /// 선택 프로필 삭제 명령이다.
    /// </summary>
    public ICommand DeleteProfileCommand { get; }

    /// <summary>
    /// 메인 ViewModel을 생성하고 설정을 불러온다.
    /// </summary>
    public MainViewModel()
    {
        StartCommand = new RelayCmd(_ => StartServerAsync(), _ => !IsServerBusy && !serverProc.IsRunning);
        StopCommand = new RelayCmd(_ => StopServerAsync(), _ => serverProc.IsRunning && currentState != RunState.Stopping);
        SaveCommand = new RelayCmd(_ => SaveConfig(), _ => CanEditProfiles);
        LoadCommand = new RelayCmd(_ => LoadConfig(), _ => CanEditProfiles);
        ResetCommand = new RelayCmd(_ => ApplyConfig(configStore.CreateDefault()), _ => CanEditProfiles);
        ClearLogCommand = new RelayCmd(_ => ClearLog());
        BrowseExeCommand = new RelayCmd(_ => BrowseExePath(), _ => CanEditProfiles);
        BrowseWorkDirCommand = new RelayCmd(_ => BrowseWorkDir(), _ => CanEditProfiles);
        BrowseLogFileCommand = new RelayCmd(_ => BrowseLogFile(), _ => CanEditProfiles);
        NewProfileCommand = new RelayCmd(_ => NewProfile(), _ => CanEditProfiles);
        SaveProfileCommand = new RelayCmd(_ => SaveSelectedProfile(), _ => CanEditProfiles);
        DeleteProfileCommand = new RelayCmd(_ => DeleteSelectedProfile(), _ => CanEditProfiles);

        serverProc.Exited += HandleServerExited;
        logTailer.LinesRead += HandleLogLinesRead;
        logTailer.ErrorRaised += HandleLogError;

        configStore.EnsureSampleFile();
        LoadConfig();
    }

    /// <summary>
    /// UI 콤보박스에 표시할 서버 실행 프로필 목록이다.
    /// </summary>
    public ObservableCollection<ServerProfile> Profiles => profiles;

    /// <summary>
    /// 현재 UI에서 선택된 서버 실행 프로필이다.
    /// </summary>
    public ServerProfile? SelectedProfile
    {
        get => selectedProfile;
        set
        {
            if (ReferenceEquals(selectedProfile, value))
            {
                return;
            }

            if (!isApplyingProfile && !CanEditProfiles)
            {
                SetError("서버 실행 중에는 프로필을 변경할 수 없습니다.");
                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SelectedProfile)));
                return;
            }

            selectedProfile = value;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SelectedProfile)));

            if (selectedProfile is null)
            {
                return;
            }

            selectedProfileName = selectedProfile.Name;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SelectedProfileName)));
            ApplyProfileToUi(selectedProfile);

            if (!isApplyingProfile)
            {
                SaveSelectedProfileName();
            }
        }
    }

    /// <summary>
    /// 설정 파일과 동기화되는 선택 프로필 이름이다.
    /// </summary>
    public string SelectedProfileName
    {
        get => selectedProfileName;
        private set => SetProperty(ref selectedProfileName, value);
    }

    /// <summary>
    /// 서버 실행 파일 경로이다.
    /// </summary>
    public string ServerExePath
    {
        get => serverExePath;
        set => SetConfigText(ref serverExePath, value);
    }

    /// <summary>
    /// 서버 작업 폴더 경로이다.
    /// </summary>
    public string WorkingDir
    {
        get => workingDir;
        set => SetConfigText(ref workingDir, value);
    }

    /// <summary>
    /// 서버 맵 경로이다.
    /// </summary>
    public string MapPath
    {
        get => mapPath;
        set => SetConfigText(ref mapPath, value);
    }

    /// <summary>
    /// 서버 포트 입력 문자열이다.
    /// </summary>
    public string PortText
    {
        get => portText;
        set => SetConfigText(ref portText, value);
    }

    /// <summary>
    /// 사용자가 설정하고 JSON에 저장할 기본 로그 파일 경로이다.
    /// </summary>
    public string LogFilePath
    {
        get => logFilePath;
        set => SetConfigText(ref logFilePath, value);
    }

    /// <summary>
    /// 현재 서버 실행에 실제로 사용 중인 로그 파일 경로이다.
    /// </summary>
    public string ActiveLogFilePath
    {
        get => activeLogFilePath;
        private set => SetProperty(ref activeLogFilePath, value);
    }

    /// <summary>
    /// 추가 실행 인자 문자열이다.
    /// </summary>
    public string ExtraArgs
    {
        get => extraArgs;
        set => SetConfigText(ref extraArgs, value);
    }

    /// <summary>
    /// 실행 인자 미리보기 문자열이다.
    /// </summary>
    public string ArgPreview
    {
        get => argPreview;
        private set => SetProperty(ref argPreview, value);
    }

    /// <summary>
    /// 최근 서버 로그 표시 문자열이다.
    /// </summary>
    public string LogText
    {
        get => logText;
        private set => SetProperty(ref logText, value);
    }

    /// <summary>
    /// 상태 표시 문자열이다.
    /// </summary>
    public string StateText
    {
        get => stateText;
        private set => SetProperty(ref stateText, value);
    }

    /// <summary>
    /// PID 표시 문자열이다.
    /// </summary>
    public string PidText
    {
        get => pidText;
        private set => SetProperty(ref pidText, value);
    }

    /// <summary>
    /// 시작 시각 표시 문자열이다.
    /// </summary>
    public string StartTimeText
    {
        get => startTimeText;
        private set => SetProperty(ref startTimeText, value);
    }

    /// <summary>
    /// 종료 코드 표시 문자열이다.
    /// </summary>
    public string ExitCodeText
    {
        get => exitCodeText;
        private set => SetProperty(ref exitCodeText, value);
    }

    /// <summary>
    /// 마지막 오류 표시 문자열이다.
    /// </summary>
    public string LastError
    {
        get => lastError;
        private set => SetProperty(ref lastError, value);
    }

    /// <summary>
    /// 자동 스크롤 사용 여부이다.
    /// </summary>
    public bool AutoScrollLog
    {
        get => autoScrollLog;
        set => SetProperty(ref autoScrollLog, value);
    }

    /// <summary>
    /// UI에 표시할 앱 버전 문자열이다.
    /// </summary>
    public string AppVersionText => AppVersionValue;

    /// <summary>
    /// 현재 서버 프로세스가 실행 중인지 여부이다.
    /// </summary>
    public bool IsServerRunning => serverProc.IsRunning;

    /// <summary>
    /// 현재 프로필 UI를 편집할 수 있는지 여부이다.
    /// </summary>
    public bool CanEditProfiles => !IsServerBusy && !serverProc.IsRunning;

    /// <summary>
    /// 서버 시작 또는 종료 처리 중인지 여부이다.
    /// </summary>
    private bool IsServerBusy => currentState is RunState.Starting or RunState.Stopping;

    /// <summary>
    /// 현재 설정값으로 서버를 시작한다.
    /// </summary>
    private async Task StartServerAsync()
    {
        isStopRequestedByUser = false;

        ServerProfile profile = BuildProfileFromUi(out string? profileError);
        if (!string.IsNullOrWhiteSpace(profileError))
        {
            SetError(profileError);
            return;
        }

        try
        {
            SetState(RunState.Starting);
            LastError = string.Empty;

            string runtimeLogFilePath = PrepareRuntimeLogFile(profile.LogFilePath);
            ServerRunConfig runConfig = profileService.CreateRunConfig(profile, runtimeLogFilePath);

            if (!pathGuard.Validate(runConfig, out string validationError))
            {
                SetState(RunState.Stopped);
                SetError(validationError);
                return;
            }

            string arguments = argBuilder.BuildArgs(runConfig);
            ProcInfo procInfo = serverProc.Start(runConfig, arguments);

            ActiveLogFilePath = runtimeLogFilePath;
            PidText = procInfo.ProcessId.ToString();
            StartTimeText = procInfo.StartTime.ToString("yyyy-MM-dd HH:mm:ss");
            ExitCodeText = "-";
            lineBuffer = new LineBuffer(maxLogLines);
            ClearLog();
            AppendLog($"[런처] 서버를 시작했습니다. PID={procInfo.ProcessId}");
            AppendLog($"[런처] 실행 로그 파일: {ActiveLogFilePath}");
            logTailer.Start(runConfig.ActiveLogFilePath);
            SetState(RunState.Running);
        }
        catch (Exception exception)
        {
            SetState(RunState.Error);
            SetError($"서버를 시작하지 못했습니다. 원인: {exception.Message}");
        }

        await Task.CompletedTask;
    }

    /// <summary>
    /// 현재 런처가 시작한 서버를 종료한다.
    /// </summary>
    private async Task StopServerAsync()
    {
        try
        {
            isStopRequestedByUser = true;
            SetState(RunState.Stopping);
            AppendLog("[런처] 서버 프로세스를 종료합니다.");
            await serverProc.StopAsync();
            logTailer.Stop();
            PidText = "-";
            SetState(RunState.Stopped);
        }
        catch (Exception exception)
        {
            isStopRequestedByUser = false;
            SetState(RunState.Error);
            SetError($"서버를 종료하지 못했습니다. 원인: {exception.Message}");
        }
    }

    /// <summary>
    /// 런처 종료 전 실행 중인 서버를 정리한다.
    /// </summary>
    public async Task StopServerForCloseAsync()
    {
        if (!serverProc.IsRunning)
        {
            return;
        }

        await StopServerAsync();
    }

    /// <summary>
    /// 현재 UI 값을 설정 파일로 저장한다.
    /// </summary>
    private void SaveConfig()
    {
        SaveSelectedProfile();
    }

    /// <summary>
    /// 현재 UI 값을 선택 프로필에 저장한다.
    /// </summary>
    private void SaveSelectedProfile()
    {
        if (!CanEditProfiles)
        {
            SetError("서버 실행 중에는 프로필을 저장할 수 없습니다.");
            return;
        }

        AppConfig config = BuildConfigFromUi(out string? configError);
        if (!string.IsNullOrWhiteSpace(configError))
        {
            SetError(configError);
            return;
        }

        try
        {
            configStore.Save(config);
            RefreshProfiles(config, config.SelectedProfileName);
            LastError = string.Empty;
            AppendLog("[런처] 프로필을 저장했습니다.");
        }
        catch (Exception exception)
        {
            SetError($"프로필을 저장하지 못했습니다. 원인: {exception.Message}");
        }
    }

    /// <summary>
    /// 현재 UI 값을 기반으로 새 프로필을 생성한다.
    /// </summary>
    private void NewProfile()
    {
        if (!CanEditProfiles)
        {
            SetError("서버 실행 중에는 프로필을 변경할 수 없습니다.");
            return;
        }

        // 새 프로필 이름 입력 대화상자 결과이다.
        string profileName = ShowInputDialog("새 프로필", "새 프로필 이름을 입력하세요.");

        if (string.IsNullOrWhiteSpace(profileName))
        {
            SetError("프로필 이름을 입력해야 합니다.");
            return;
        }

        profileName = profileName.Trim();

        if (profileService.HasDuplicateName(currentConfig.Profiles, profileName))
        {
            SetError("같은 이름의 프로필이 이미 있습니다.");
            return;
        }

        // 현재 UI 입력값에서 만든 새 프로필이다.
        ServerProfile newProfile = BuildProfileFromUi(out string? profileError);
        if (!string.IsNullOrWhiteSpace(profileError))
        {
            SetError(profileError);
            return;
        }

        newProfile.Name = profileName;
        currentConfig.Profiles.Add(newProfile);
        currentConfig.SelectedProfileName = profileName;
        profileService.NormalizeConfig(currentConfig);

        try
        {
            configStore.Save(currentConfig);
            RefreshProfiles(currentConfig, profileName);
            LastError = string.Empty;
            AppendLog("[런처] 프로필을 저장했습니다.");
        }
        catch (Exception exception)
        {
            SetError($"프로필을 저장하지 못했습니다. 원인: {exception.Message}");
        }
    }

    /// <summary>
    /// 선택한 프로필을 삭제한다.
    /// </summary>
    private void DeleteSelectedProfile()
    {
        if (!CanEditProfiles)
        {
            SetError("서버 실행 중에는 프로필을 삭제할 수 없습니다.");
            return;
        }

        if (currentConfig.Profiles.Count <= 1)
        {
            SetError("프로필은 최소 1개 이상 필요합니다.");
            return;
        }

        if (SelectedProfile is null)
        {
            SetError("삭제할 프로필을 선택해야 합니다.");
            return;
        }

        // 삭제 확인 대화상자 결과이다.
        MessageBoxResult confirmResult = MessageBox.Show(
            "선택한 프로필을 삭제할까요?",
            "프로필 삭제",
            MessageBoxButton.YesNo,
            MessageBoxImage.Question);

        if (confirmResult != MessageBoxResult.Yes)
        {
            return;
        }

        // 삭제할 프로필 이름이다.
        string deleteProfileName = SelectedProfile.Name;

        if (!profileService.RemoveProfile(currentConfig, deleteProfileName))
        {
            SetError("삭제할 프로필을 찾지 못했습니다.");
            return;
        }

        // 삭제 후 선택할 첫 번째 프로필 이름이다.
        string nextProfileName = currentConfig.Profiles[0].Name;
        currentConfig.SelectedProfileName = nextProfileName;

        try
        {
            configStore.Save(currentConfig);
            RefreshProfiles(currentConfig, nextProfileName);
            LastError = string.Empty;
            AppendLog("[런처] 프로필을 삭제했습니다.");
        }
        catch (Exception exception)
        {
            SetError($"프로필을 삭제하지 못했습니다. 원인: {exception.Message}");
        }
    }

    /// <summary>
    /// 설정 파일을 불러와 UI에 적용한다.
    /// </summary>
    private void LoadConfig()
    {
        AppConfig config = configStore.LoadOrDefault(out string? errorMessage);
        ApplyConfig(config);

        if (!string.IsNullOrWhiteSpace(errorMessage))
        {
            SetError(errorMessage);
        }
        else
        {
            LastError = string.Empty;
        }
    }

    /// <summary>
    /// 지정된 설정 객체를 UI 속성에 적용한다.
    /// </summary>
    private void ApplyConfig(AppConfig config)
    {
        profileService.NormalizeConfig(config);
        currentConfig = config;
        maxLogLines = config.MaxLogLines;
        RefreshProfiles(config, config.SelectedProfileName);
    }

    /// <summary>
    /// 현재 UI 값에서 설정 객체를 만든다.
    /// </summary>
    private AppConfig BuildConfigFromUi(out string? errorMessage)
    {
        errorMessage = null;

        ServerProfile selectedProfile = BuildProfileFromUi(out errorMessage);
        if (!string.IsNullOrWhiteSpace(errorMessage))
        {
            return currentConfig;
        }

        // 현재 선택 프로필이 반영된 새 프로필 목록이다.
        List<ServerProfile> nextProfiles = new();

        // 기존 선택 프로필을 새 UI 값으로 교체했는지 여부이다.
        bool replacedSelectedProfile = false;

        foreach (ServerProfile profile in currentConfig.Profiles)
        {
            if (string.Equals(profile.Name, selectedProfileName, StringComparison.OrdinalIgnoreCase))
            {
                nextProfiles.Add(selectedProfile);
                replacedSelectedProfile = true;
                continue;
            }

            nextProfiles.Add(profile);
        }

        if (!replacedSelectedProfile)
        {
            nextProfiles.Add(selectedProfile);
        }

        AppConfig nextConfig = new()
        {
            Version = AppConst.ConfigVersion,
            SelectedProfileName = selectedProfile.Name,
            MaxLogLines = maxLogLines,
            Profiles = nextProfiles
        };

        profileService.NormalizeConfig(nextConfig);
        currentConfig = nextConfig;
        selectedProfileName = nextConfig.SelectedProfileName;
        return nextConfig;
    }

    /// <summary>
    /// 현재 UI 값에서 선택 프로필 객체를 만든다.
    /// </summary>
    private ServerProfile BuildProfileFromUi(out string? errorMessage)
    {
        errorMessage = null;

        if (!int.TryParse(PortText, out int parsedPort))
        {
            parsedPort = 0;
        }

        return new ServerProfile
        {
            Name = string.IsNullOrWhiteSpace(selectedProfileName) ? ProfileService.DefaultProfileName : selectedProfileName,
            ServerExePath = ServerExePath.Trim(),
            WorkingDir = WorkingDir.Trim(),
            MapPath = MapPath.Trim(),
            Port = parsedPort,
            LogFilePath = LogFilePath.Trim(),
            ExtraArgs = ExtraArgs.Trim(),
            AutoScrollLog = AutoScrollLog
        };
    }

    /// <summary>
    /// 설정의 프로필 목록을 UI 컬렉션과 선택 상태에 반영한다.
    /// </summary>
    private void RefreshProfiles(AppConfig config, string? targetProfileName)
    {
        isApplyingProfile = true;

        try
        {
            currentConfig = config;
            profiles.Clear();

            foreach (ServerProfile profile in config.Profiles)
            {
                profiles.Add(profile);
            }

            // 선택 대상으로 사용할 프로필이다.
            ServerProfile targetProfile = profileService.FindProfile(config, targetProfileName) ?? config.Profiles[0];
            selectedProfile = targetProfile;
            SelectedProfileName = targetProfile.Name;
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(SelectedProfile)));
            ApplyProfileToUi(targetProfile);
        }
        finally
        {
            isApplyingProfile = false;
        }
    }

    /// <summary>
    /// 지정한 프로필 값을 기존 UI 입력 필드에 적용한다.
    /// </summary>
    private void ApplyProfileToUi(ServerProfile profile)
    {
        ServerExePath = profile.ServerExePath;
        WorkingDir = profile.WorkingDir;
        MapPath = profile.MapPath;
        PortText = profile.Port.ToString();
        LogFilePath = profile.LogFilePath;
        ActiveLogFilePath = string.Empty;
        ExtraArgs = profile.ExtraArgs;
        AutoScrollLog = profile.AutoScrollLog;
        UpdatePreview();
    }

    /// <summary>
    /// 선택 프로필 이름만 설정 파일에 저장한다.
    /// </summary>
    private void SaveSelectedProfileName()
    {
        currentConfig.SelectedProfileName = selectedProfileName;

        try
        {
            configStore.Save(currentConfig);
            LastError = string.Empty;
        }
        catch (Exception exception)
        {
            SetError($"선택 프로필을 저장하지 못했습니다. 원인: {exception.Message}");
        }
    }

    /// <summary>
    /// 간단한 문자열 입력 대화상자를 표시하고 입력값을 반환한다.
    /// </summary>
    private static string ShowInputDialog(string title, string promptText)
    {
        // 입력 대화상자 창이다.
        Window inputWindow = new()
        {
            Title = title,
            Width = 360,
            Height = 150,
            WindowStartupLocation = WindowStartupLocation.CenterOwner,
            ResizeMode = ResizeMode.NoResize,
            Owner = Application.Current.MainWindow
        };

        // 대화상자 전체 배치를 담당하는 패널이다.
        Grid layoutGrid = new()
        {
            Margin = new Thickness(12)
        };

        layoutGrid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
        layoutGrid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });
        layoutGrid.RowDefinitions.Add(new RowDefinition { Height = GridLength.Auto });

        // 사용자에게 보여줄 입력 안내 문구이다.
        TextBlock promptBlock = new()
        {
            Text = promptText,
            Margin = new Thickness(0, 0, 0, 8)
        };

        // 프로필 이름을 입력받는 텍스트 박스이다.
        TextBox inputTextBox = new()
        {
            Margin = new Thickness(0, 0, 0, 12)
        };

        // 확인과 취소 버튼을 담는 패널이다.
        StackPanel buttonPanel = new()
        {
            Orientation = Orientation.Horizontal,
            HorizontalAlignment = HorizontalAlignment.Right
        };

        // 입력값을 확정하는 버튼이다.
        Button okButton = new()
        {
            Content = "확인",
            Width = 72,
            IsDefault = true,
            Margin = new Thickness(0, 0, 8, 0)
        };

        // 입력을 취소하는 버튼이다.
        Button cancelButton = new()
        {
            Content = "취소",
            Width = 72,
            IsCancel = true
        };

        okButton.Click += (_, _) => inputWindow.DialogResult = true;
        cancelButton.Click += (_, _) => inputWindow.DialogResult = false;

        buttonPanel.Children.Add(okButton);
        buttonPanel.Children.Add(cancelButton);

        Grid.SetRow(promptBlock, 0);
        Grid.SetRow(inputTextBox, 1);
        Grid.SetRow(buttonPanel, 2);

        layoutGrid.Children.Add(promptBlock);
        layoutGrid.Children.Add(inputTextBox);
        layoutGrid.Children.Add(buttonPanel);

        inputWindow.Content = layoutGrid;
        inputWindow.Loaded += (_, _) => inputTextBox.Focus();

        return inputWindow.ShowDialog() == true ? inputTextBox.Text : string.Empty;
    }

    /// <summary>
    /// 서버 시작 전 사용할 고유 런타임 로그 파일 경로를 만든다.
    /// </summary>
    private static string PrepareRuntimeLogFile(string baseLogPath)
    {
        string? logDir = Path.GetDirectoryName(baseLogPath);
        if (string.IsNullOrWhiteSpace(logDir))
        {
            logDir = AppConst.GetLogsDir();
        }

        Directory.CreateDirectory(logDir);

        string baseFileName = Path.GetFileNameWithoutExtension(baseLogPath);
        if (string.IsNullOrWhiteSpace(baseFileName))
        {
            baseFileName = "server";
        }

        string extension = Path.GetExtension(baseLogPath);
        if (string.IsNullOrWhiteSpace(extension))
        {
            extension = ".log";
        }

        string timeSuffix = DateTime.Now.ToString("yyyyMMdd_HHmmss_fff");
        return Path.Combine(logDir, $"{baseFileName}_{timeSuffix}{extension}");
    }

    /// <summary>
    /// 서버 실행 파일 선택 대화상자를 열어 ServerExePath를 갱신한다.
    /// </summary>
    private void BrowseExePath()
    {
        OpenFileDialog dialog = new()
        {
            Title = "서버 실행 파일 선택",
            Filter = "실행 파일 (*.exe)|*.exe|모든 파일 (*.*)|*.*",
            CheckFileExists = true,
            Multiselect = false
        };

        ApplyInitialFileDialogPath(dialog, ServerExePath);

        if (dialog.ShowDialog() == true)
        {
            ServerExePath = dialog.FileName;
        }
    }

    /// <summary>
    /// 작업 폴더 선택 대화상자를 열어 WorkingDir을 갱신한다.
    /// </summary>
    private void BrowseWorkDir()
    {
        OpenFolderDialog dialog = new()
        {
            Title = "작업 폴더 선택",
            Multiselect = false
        };

        if (Directory.Exists(WorkingDir))
        {
            dialog.InitialDirectory = WorkingDir;
        }

        if (dialog.ShowDialog() == true)
        {
            WorkingDir = dialog.FolderName;
        }
    }

    /// <summary>
    /// 로그 파일 선택 대화상자를 열어 LogFilePath 기본값을 갱신한다.
    /// </summary>
    private void BrowseLogFile()
    {
        SaveFileDialog dialog = new()
        {
            Title = "로그 파일 기본 경로 선택",
            Filter = "로그 파일 (*.log)|*.log|모든 파일 (*.*)|*.*",
            AddExtension = true,
            DefaultExt = ".log",
            OverwritePrompt = false
        };

        ApplyInitialFileDialogPath(dialog, LogFilePath);

        if (dialog.ShowDialog() == true)
        {
            LogFilePath = dialog.FileName;
        }
    }

    /// <summary>
    /// 파일 대화상자의 초기 폴더와 파일명을 현재 경로 기준으로 설정한다.
    /// </summary>
    private static void ApplyInitialFileDialogPath(FileDialog dialog, string currentPath)
    {
        if (string.IsNullOrWhiteSpace(currentPath))
        {
            return;
        }

        string? initialDirectory = Path.GetDirectoryName(currentPath);
        if (!string.IsNullOrWhiteSpace(initialDirectory) && Directory.Exists(initialDirectory))
        {
            dialog.InitialDirectory = initialDirectory;
        }

        string fileName = Path.GetFileName(currentPath);
        if (!string.IsNullOrWhiteSpace(fileName))
        {
            dialog.FileName = fileName;
        }
    }

    /// <summary>
    /// 서버 종료 이벤트를 UI 스레드에서 처리한다.
    /// </summary>
    private void HandleServerExited(int? exitCode)
    {
        Application.Current.Dispatcher.Invoke(() =>
        {
            logTailer.Stop();
            PidText = "-";
            ExitCodeText = exitCode?.ToString() ?? "-";
            AppendLog($"[런처] 서버가 종료되었습니다. ExitCode={ExitCodeText}");
            SetState(isStopRequestedByUser ? RunState.Stopped : RunState.Exited);
            isStopRequestedByUser = false;
        });
    }

    /// <summary>
    /// 새 로그 라인들을 UI 로그 버퍼에 추가한다.
    /// </summary>
    private void HandleLogLinesRead(IReadOnlyList<string> lineTexts)
    {
        Application.Current.Dispatcher.Invoke(() =>
        {
            lineBuffer.AddRange(lineTexts);
            LogText = lineBuffer.GetText();
        });
    }

    /// <summary>
    /// 로그 읽기 오류를 UI에 표시한다.
    /// </summary>
    private void HandleLogError(string errorText)
    {
        Application.Current.Dispatcher.Invoke(() => SetError(errorText));
    }

    /// <summary>
    /// UI 로그 표시만 지운다.
    /// </summary>
    private void ClearLog()
    {
        lineBuffer.Clear();
        LogText = string.Empty;
    }

    /// <summary>
    /// UI 로그 버퍼에 런처 메시지를 추가한다.
    /// </summary>
    private void AppendLog(string lineText)
    {
        lineBuffer.Add(lineText);
        LogText = lineBuffer.GetText();
    }

    /// <summary>
    /// 현재 설정값 기준 실행 인자 미리보기를 갱신한다.
    /// </summary>
    private void UpdatePreview()
    {
        ServerProfile profile = BuildProfileFromUi(out _);
        ServerRunConfig runConfig = profileService.CreateRunConfig(profile, profile.LogFilePath);
        ArgPreview = argBuilder.BuildPreview(runConfig);
    }

    /// <summary>
    /// 실행 상태와 표시 문구를 갱신한다.
    /// </summary>
    private void SetState(RunState newState)
    {
        currentState = newState;
        StateText = newState switch
        {
            RunState.Stopped => "중지됨",
            RunState.Starting => "시작 중",
            RunState.Running => "실행 중",
            RunState.Stopping => "종료 중",
            RunState.Exited => "종료됨",
            RunState.Error => "오류",
            _ => "알 수 없음"
        };
        RaiseCommandStates();
    }

    /// <summary>
    /// 마지막 오류 메시지를 설정하고 로그에도 추가한다.
    /// </summary>
    private void SetError(string errorText)
    {
        LastError = errorText;
        AppendLog($"[오류] {errorText}");
    }

    /// <summary>
    /// 설정 텍스트 속성을 바꾸고 미리보기를 갱신한다.
    /// </summary>
    private void SetConfigText(ref string storage, string value, [CallerMemberName] string? propertyName = null)
    {
        if (SetProperty(ref storage, value, propertyName))
        {
            UpdatePreview();
        }
    }

    /// <summary>
    /// 속성값을 변경하고 변경 이벤트를 발생시킨다.
    /// </summary>
    private bool SetProperty<T>(ref T storage, T value, [CallerMemberName] string? propertyName = null)
    {
        if (EqualityComparer<T>.Default.Equals(storage, value))
        {
            return false;
        }

        storage = value;
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(propertyName));
        return true;
    }

    /// <summary>
    /// 명령 활성화 상태 갱신을 요청한다.
    /// </summary>
    private void RaiseCommandStates()
    {
        if (StartCommand is RelayCmd startCommand)
        {
            startCommand.RaiseCanExecuteChanged();
        }

        if (StopCommand is RelayCmd stopCommand)
        {
            stopCommand.RaiseCanExecuteChanged();
        }

        if (SaveCommand is RelayCmd saveCommand)
        {
            saveCommand.RaiseCanExecuteChanged();
        }

        if (LoadCommand is RelayCmd loadCommand)
        {
            loadCommand.RaiseCanExecuteChanged();
        }

        if (ResetCommand is RelayCmd resetCommand)
        {
            resetCommand.RaiseCanExecuteChanged();
        }

        if (BrowseExeCommand is RelayCmd browseExeCommand)
        {
            browseExeCommand.RaiseCanExecuteChanged();
        }

        if (BrowseWorkDirCommand is RelayCmd browseWorkDirCommand)
        {
            browseWorkDirCommand.RaiseCanExecuteChanged();
        }

        if (BrowseLogFileCommand is RelayCmd browseLogFileCommand)
        {
            browseLogFileCommand.RaiseCanExecuteChanged();
        }

        if (NewProfileCommand is RelayCmd newProfileCommand)
        {
            newProfileCommand.RaiseCanExecuteChanged();
        }

        if (SaveProfileCommand is RelayCmd saveProfileCommand)
        {
            saveProfileCommand.RaiseCanExecuteChanged();
        }

        if (DeleteProfileCommand is RelayCmd deleteProfileCommand)
        {
            deleteProfileCommand.RaiseCanExecuteChanged();
        }

        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(CanEditProfiles)));
    }

    /// <summary>
    /// ViewModel이 보유한 서비스 리소스를 해제한다.
    /// </summary>
    public void Dispose()
    {
        serverProc.Exited -= HandleServerExited;
        logTailer.LinesRead -= HandleLogLinesRead;
        logTailer.ErrorRaised -= HandleLogError;
        logTailer.Dispose();
    }
}
