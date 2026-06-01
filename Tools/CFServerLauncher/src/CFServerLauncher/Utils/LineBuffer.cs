namespace CFServerLauncher.Utils;

/// <summary>
/// UI 로그 표시를 위한 최대 라인 수 제한 버퍼이다.
/// </summary>
public sealed class LineBuffer
{
    /// <summary>
    /// 로그 라인을 보관하는 내부 큐이다.
    /// </summary>
    private readonly Queue<string> lineQueue = new();

    /// <summary>
    /// 최대 보관 라인 수이다.
    /// </summary>
    private readonly int maxLineCount;

    /// <summary>
    /// 최대 라인 수를 받아 버퍼를 생성한다.
    /// </summary>
    public LineBuffer(int maxLineCount)
    {
        this.maxLineCount = Math.Max(100, maxLineCount);
    }

    /// <summary>
    /// 새 로그 라인을 추가한다.
    /// </summary>
    public void Add(string lineText)
    {
        lineQueue.Enqueue(lineText);

        while (lineQueue.Count > maxLineCount)
        {
            lineQueue.Dequeue();
        }
    }

    /// <summary>
    /// 여러 로그 라인을 한 번에 추가한다.
    /// </summary>
    public void AddRange(IEnumerable<string> lineTexts)
    {
        foreach (string lineText in lineTexts)
        {
            Add(lineText);
        }
    }

    /// <summary>
    /// 현재 버퍼에 있는 모든 로그를 문자열로 반환한다.
    /// </summary>
    public string GetText()
    {
        return string.Join(Environment.NewLine, lineQueue);
    }

    /// <summary>
    /// 현재 버퍼를 비운다.
    /// </summary>
    public void Clear()
    {
        lineQueue.Clear();
    }
}
