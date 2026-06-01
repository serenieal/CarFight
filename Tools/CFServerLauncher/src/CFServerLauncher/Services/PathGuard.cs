using System.IO;
using CFServerLauncher.Models;

namespace CFServerLauncher.Services;

/// <summary>
/// 서버 시작 전 입력값과 경로를 검증한다.
/// </summary>
public sealed class PathGuard
{
    /// <summary>
    /// 서버 시작에 필요한 설정값을 검증한다.
    /// </summary>
    public bool Validate(ServerRunConfig runConfig, out string errorMessage)
    {
        errorMessage = string.Empty;

        if (string.IsNullOrWhiteSpace(runConfig.ServerExePath))
        {
            errorMessage = "서버 실행 파일 경로를 입력해야 합니다.";
            return false;
        }

        if (!File.Exists(runConfig.ServerExePath))
        {
            errorMessage = "서버 실행 파일을 찾을 수 없습니다.";
            return false;
        }

        if (!string.Equals(Path.GetExtension(runConfig.ServerExePath), ".exe", StringComparison.OrdinalIgnoreCase))
        {
            errorMessage = "서버 실행 파일은 .exe 파일이어야 합니다.";
            return false;
        }

        if (string.IsNullOrWhiteSpace(runConfig.WorkingDir))
        {
            errorMessage = "작업 폴더 경로를 입력해야 합니다.";
            return false;
        }

        if (!Directory.Exists(runConfig.WorkingDir))
        {
            errorMessage = "작업 폴더를 찾을 수 없습니다.";
            return false;
        }

        if (string.IsNullOrWhiteSpace(runConfig.MapPath))
        {
            errorMessage = "맵 경로를 입력해야 합니다.";
            return false;
        }

        if (runConfig.Port < 1 || runConfig.Port > 65535)
        {
            errorMessage = "포트는 1부터 65535 사이의 숫자여야 합니다.";
            return false;
        }

        if (string.IsNullOrWhiteSpace(runConfig.ActiveLogFilePath) && string.IsNullOrWhiteSpace(runConfig.BaseLogFilePath))
        {
            errorMessage = "로그 파일 경로를 입력해야 합니다.";
            return false;
        }

        string logFilePath = string.IsNullOrWhiteSpace(runConfig.ActiveLogFilePath)
            ? runConfig.BaseLogFilePath
            : runConfig.ActiveLogFilePath;

        string? logDir = Path.GetDirectoryName(logFilePath);
        if (string.IsNullOrWhiteSpace(logDir))
        {
            errorMessage = "로그 파일 폴더 경로를 확인해야 합니다.";
            return false;
        }

        try
        {
            Directory.CreateDirectory(logDir);
        }
        catch (Exception exception)
        {
            errorMessage = $"로그 파일 폴더를 만들 수 없습니다. 원인: {exception.Message}";
            return false;
        }

        return true;
    }
}
