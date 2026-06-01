using System.Text;
using CFServerLauncher.Models;

namespace CFServerLauncher.Services;

/// <summary>
/// Dedicated Server 실행 인자와 미리보기 문자열을 조립한다.
/// </summary>
public sealed class ArgBuilder
{
    /// <summary>
    /// 서버 실행에 전달할 인자 문자열을 만든다.
    /// </summary>
    public string BuildArgs(ServerRunConfig runConfig)
    {
        List<string> argList = new()
        {
            runConfig.MapPath.Trim(),
            $"-port={runConfig.Port}",
            "-log",
            "-unattended",
            "-NoSound",
            "-LiveCoding=false",
            $"-AbsLog={Quote(runConfig.ActiveLogFilePath)}"
        };

        if (!string.IsNullOrWhiteSpace(runConfig.ExtraArgs))
        {
            argList.Add(runConfig.ExtraArgs.Trim());
        }

        return string.Join(" ", argList.Where(argText => !string.IsNullOrWhiteSpace(argText)));
    }

    /// <summary>
    /// UI에 표시할 전체 실행 명령 미리보기 문자열을 만든다.
    /// </summary>
    public string BuildPreview(ServerRunConfig runConfig)
    {
        StringBuilder builder = new();
        builder.Append(Quote(runConfig.ServerExePath));
        builder.Append(' ');
        builder.Append(BuildArgs(runConfig));
        return builder.ToString();
    }

    /// <summary>
    /// 공백이 있는 경로를 안전하게 표시하도록 따옴표를 적용한다.
    /// </summary>
    private static string Quote(string rawText)
    {
        string trimmedText = rawText.Trim();

        if (trimmedText.StartsWith('"') && trimmedText.EndsWith('"'))
        {
            return trimmedText;
        }

        return $"\"{trimmedText}\"";
    }
}
