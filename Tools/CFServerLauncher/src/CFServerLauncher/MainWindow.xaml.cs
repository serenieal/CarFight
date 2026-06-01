using System.ComponentModel;
using System.Windows;
using System.Windows.Controls;
using CFServerLauncher.ViewModels;

namespace CFServerLauncher;

/// <summary>
/// CarFight 서버 런처의 메인 창이다.
/// </summary>
public partial class MainWindow : Window
{
    /// <summary>
    /// 메인 창 ViewModel 인스턴스이다.
    /// </summary>
    private readonly MainViewModel viewModel;

    /// <summary>
    /// 실행 중인 서버 종료 확인을 이미 완료했는지 여부이다.
    /// </summary>
    private bool isClosingConfirmed;

    /// <summary>
    /// 메인 창을 생성하고 ViewModel을 연결한다.
    /// </summary>
    public MainWindow()
    {
        InitializeComponent();
        viewModel = new MainViewModel();
        DataContext = viewModel;
    }

    /// <summary>
    /// 로그 텍스트가 변경되었을 때 자동 스크롤을 수행한다.
    /// </summary>
    private void HandleLogTextChanged(object sender, TextChangedEventArgs eventArgs)
    {
        if (DataContext is MainViewModel currentViewModel && currentViewModel.AutoScrollLog)
        {
            LogTextBox.ScrollToEnd();
        }
    }

    /// <summary>
    /// 창 닫기 요청 시 실행 중인 서버 종료 여부를 확인한다.
    /// </summary>
    protected override async void OnClosing(CancelEventArgs eventArgs)
    {
        if (isClosingConfirmed || !viewModel.IsServerRunning)
        {
            base.OnClosing(eventArgs);
            return;
        }

        // 사용자가 서버 종료 후 런처를 닫을지 선택한 결과이다.
        MessageBoxResult closeResult = MessageBox.Show(
            this,
            "서버가 실행 중입니다. 서버를 종료하고 런처를 닫을까요?",
            "런처 종료 확인",
            MessageBoxButton.YesNo,
            MessageBoxImage.Question);

        if (closeResult != MessageBoxResult.Yes)
        {
            eventArgs.Cancel = true;
            return;
        }

        eventArgs.Cancel = true;
        isClosingConfirmed = true;
        await viewModel.StopServerForCloseAsync();
        Close();
    }

    /// <summary>
    /// 창이 닫힐 때 ViewModel 리소스를 정리한다.
    /// </summary>
    protected override void OnClosed(EventArgs eventArgs)
    {
        viewModel.Dispose();
        base.OnClosed(eventArgs);
    }
}
