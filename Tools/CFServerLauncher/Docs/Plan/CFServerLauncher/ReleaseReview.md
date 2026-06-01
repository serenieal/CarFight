# CFServerLauncher v1.0 RC 리뷰 결과

문서 버전: v1.0.7-review  
작성 기준일: 2026-05-29

## 1. 리뷰 결론

Codex의 v1.0 Release Candidate 준비 작업은 코드 리뷰 기준으로 통과로 본다.

확인된 구현 항목:

- 앱 버전 표시 추가
- 서버 실행 중 런처 종료 시 확인 대화상자 추가
- 사용자가 닫기 취소를 선택하면 앱 종료 취소
- 사용자가 닫기 확인을 선택하면 서버 종료 후 앱 닫기 시도
- 릴리스 빌드/배포 지침 문서 추가

남은 항목은 실제 로컬 수동 검증이다.

## 2. 확인한 코드 변경

### 2.1 버전 표시

`MainViewModel`에 버전 문자열이 추가되었다.

```text
버전: v1.0.0-rc1
```

`MainWindow.xaml` 하단에 `AppVersionText`가 바인딩되어 표시된다.

### 2.2 서버 실행 중 앱 닫기 확인

`MainWindow.xaml.cs`에 `OnClosing` 처리가 추가되었다.

동작 기준:

```text
서버 미실행 상태: 바로 닫힘
서버 실행 상태: 확인 대화상자 표시
No 선택: 닫기 취소
Yes 선택: 서버 종료 후 닫기 재시도
```

대화상자 문구:

```text
서버가 실행 중입니다. 서버를 종료하고 런처를 닫을까요?
```

### 2.3 닫기 중 서버 종료 연결

`MainViewModel`에 `IsServerRunning`과 `StopServerForCloseAsync()`가 추가되었다.

`MainWindow.xaml.cs`는 이 값을 이용해 서버 실행 상태를 확인하고, Yes 선택 시 서버 종료를 요청한다.

### 2.4 릴리스 문서

다음 문서가 추가되었다.

```text
Tools\CFServerLauncher\Docs\release.md
```

포함 내용:

- 빌드 검증 명령
- 로컬 실행 검증 명령
- publish 명령
- 예상 출력 위치
- Changelog
- 마이그레이션 지침

## 3. 검토상 주의점

코드상 큰 문제는 보이지 않는다. 다만 아래는 실제 동작 검증이 필요하다.

1. `dotnet build` 성공 여부
2. 서버 실행 중 닫기 대화상자에서 No 선택 시 서버가 계속 실행되는지
3. 서버 실행 중 닫기 대화상자에서 Yes 선택 시 서버가 정상 종료되고 앱이 닫히는지
4. Yes 선택 중 서버 종료 실패 상황에서 앱이 닫히는지 여부

4번은 v1.0 RC에서는 예외 상황으로 본다. 실제 테스트에서 문제가 나오면 후속 fix 작업으로 분리한다.

## 4. 수동 검증 항목

PowerShell:

```powershell
cd D:\Work\CarFight_git
dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln
dotnet run --project .\Tools\CFServerLauncher\src\CFServerLauncher\CFServerLauncher.csproj
```

수동 확인:

```text
1. 앱이 실행되는지 확인
2. 하단에 버전: v1.0.0-rc1 표시 확인
3. 서버 미실행 상태에서 창 닫기 확인
4. 앱 재실행
5. 서버 시작
6. 서버 실행 중 창 닫기 클릭
7. No 선택 시 앱이 유지되고 서버가 계속 실행되는지 확인
8. 다시 창 닫기 클릭
9. Yes 선택 시 서버가 종료되고 앱이 닫히는지 확인
10. 앱 재실행 후 기존 서버 시작/종료/재시작이 유지되는지 확인
```

## 5. 판정

```text
코드 리뷰: 통과
수동 검증: 대기
```
