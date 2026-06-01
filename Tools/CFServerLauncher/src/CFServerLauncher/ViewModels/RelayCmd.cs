using System.Windows.Input;

namespace CFServerLauncher.ViewModels;

/// <summary>
/// ViewModel에서 버튼 명령을 바인딩하기 위한 기본 ICommand 구현이다.
/// </summary>
public sealed class RelayCmd : ICommand
{
    /// <summary>
    /// 명령 실행 동작이다.
    /// </summary>
    private readonly Func<object?, Task> executeAsync;

    /// <summary>
    /// 명령 실행 가능 여부 판단 동작이다.
    /// </summary>
    private readonly Predicate<object?>? canExecute;

    /// <summary>
    /// 명령 실행 중인지 여부이다.
    /// </summary>
    private bool isExecuting;

    /// <summary>
    /// 명령 실행 가능 여부가 바뀔 때 발생한다.
    /// </summary>
    public event EventHandler? CanExecuteChanged;

    /// <summary>
    /// 비동기 명령을 생성한다.
    /// </summary>
    public RelayCmd(Func<object?, Task> executeAsync, Predicate<object?>? canExecute = null)
    {
        this.executeAsync = executeAsync;
        this.canExecute = canExecute;
    }

    /// <summary>
    /// 동기 명령을 생성한다.
    /// </summary>
    public RelayCmd(Action<object?> execute, Predicate<object?>? canExecute = null)
    {
        executeAsync = parameter =>
        {
            execute(parameter);
            return Task.CompletedTask;
        };
        this.canExecute = canExecute;
    }

    /// <summary>
    /// 현재 명령을 실행할 수 있는지 반환한다.
    /// </summary>
    public bool CanExecute(object? parameter)
    {
        return !isExecuting && (canExecute?.Invoke(parameter) ?? true);
    }

    /// <summary>
    /// 명령을 실행한다.
    /// </summary>
    public async void Execute(object? parameter)
    {
        if (!CanExecute(parameter))
        {
            return;
        }

        try
        {
            isExecuting = true;
            RaiseCanExecuteChanged();
            await executeAsync(parameter);
        }
        finally
        {
            isExecuting = false;
            RaiseCanExecuteChanged();
        }
    }

    /// <summary>
    /// 버튼 활성화 상태 갱신을 요청한다.
    /// </summary>
    public void RaiseCanExecuteChanged()
    {
        CanExecuteChanged?.Invoke(this, EventArgs.Empty);
    }
}
