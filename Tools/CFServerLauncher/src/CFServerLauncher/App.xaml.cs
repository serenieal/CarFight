using System.IO;
using System.Text;
using System.Windows;
using CFServerLauncher.Utils;

namespace CFServerLauncher;

/// <summary>
/// WPF 애플리케이션 진입점과 시작 오류 처리를 제공한다.
/// </summary>
public partial class App : Application
{
    /// <summary>
    /// 앱 시작 시 메인 창을 생성하고 시작 예외를 사용자에게 표시한다.
    /// </summary>
    protected override void OnStartup(StartupEventArgs eventArgs)
    {
        base.OnStartup(eventArgs);

        try
        {
            MainWindow mainWindow = new();
            MainWindow = mainWindow;
            mainWindow.Show();
        }
        catch (Exception exception)
        {
            string errorLogPath = WriteStartupError(exception);
            MessageBox.Show(
                $"CFServerLauncher 시작 중 오류가 발생했습니다.\n\n{exception.Message}\n\n자세한 내용:\n{errorLogPath}",
                "CFServerLauncher 시작 오류",
                MessageBoxButton.OK,
                MessageBoxImage.Error);

            Shutdown(-1);
        }
    }

    /// <summary>
    /// 시작 실패 예외를 Logs 폴더의 텍스트 파일로 저장한다.
    /// </summary>
    private static string WriteStartupError(Exception exception)
    {
        string logsDir = AppConst.GetLogsDir();
        Directory.CreateDirectory(logsDir);

        string errorLogPath = Path.Combine(logsDir, "startup-error.log");
        StringBuilder builder = new();
        builder.AppendLine($"Time: {DateTime.Now:yyyy-MM-dd HH:mm:ss}");
        builder.AppendLine(exception.ToString());

        File.WriteAllText(errorLogPath, builder.ToString(), Encoding.UTF8);
        return errorLogPath;
    }
}
