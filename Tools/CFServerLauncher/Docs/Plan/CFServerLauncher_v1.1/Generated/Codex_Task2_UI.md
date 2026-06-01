# Codex Task 2 - CFServerLauncher v1.1 Profile UI Wiring

문서 버전: v1.1.0-codex-task2  
작성일: 2026-06-01  
작업 유형: 코드 수정  
대상 프로젝트: `Tools/CFServerLauncher/src/CFServerLauncher`

## 1. 목표

CFServerLauncher v1.1의 2차 코드 작업이다.

Task 1에서 구현된 v1.1 프로필 설정 구조를 `MainViewModel.cs`와 `MainWindow.xaml`에 연결한다.

프로필 선택/추가/저장/삭제 UI를 제공하고, 선택한 프로필 값으로 서버를 시작할 수 있게 한다.

## 2. 선행조건

- `Codex_Task1_Core.md` 작업이 완료되어 있어야 한다.
- `Models/ServerProfile.cs`, `Models/ServerRunConfig.cs`, `Services/ProfileService.cs`가 존재해야 한다.
- `AppConfig`가 v1.1 구조여야 한다.
- `dotnet build`가 Task 1 기준으로 성공해야 한다.

## 3. 대상 파일

### 수정

```text
Tools/CFServerLauncher/src/CFServerLauncher/ViewModels/MainViewModel.cs
Tools/CFServerLauncher/src/CFServerLauncher/MainWindow.xaml
```

### 필요 시 최소 수정

```text
Tools/CFServerLauncher/src/CFServerLauncher/MainWindow.xaml.cs
Tools/CFServerLauncher/src/CFServerLauncher/ViewModels/RelayCmd.cs
```

## 4. UI 요구사항

기존 서버 설정 영역 위에 프로필 영역을 추가한다.

구성:

```text
[프로필] [로컬 테스트 서버 ▼] [새 프로필] [프로필 저장] [프로필 삭제]
```

권장 UI 문구:

```text
프로필
새 프로필
프로필 저장
프로필 삭제
프로필을 저장했습니다.
프로필은 최소 1개 이상 필요합니다.
서버 실행 중에는 프로필을 변경할 수 없습니다.
서버 실행 중에는 프로필을 저장할 수 없습니다.
서버 실행 중에는 프로필을 삭제할 수 없습니다.
프로필 이름을 입력해야 합니다.
같은 이름의 프로필이 이미 있습니다.
선택한 프로필을 삭제할까요?
```

v1.1에서 구현하지 않을 UI:

```text
프로필 이름 변경
프로필 복제
프로필 가져오기
프로필 내보내기
```

## 5. MainViewModel 요구사항

### 5.1 추가 상태

`MainViewModel.cs`에 다음 상태를 추가한다.

```text
Profiles
SelectedProfile
SelectedProfileName
```

권장:

- `Profiles`는 UI 콤보박스에 바인딩 가능한 컬렉션이어야 한다.
- `SelectedProfile` 변경 시 해당 프로필 값을 UI 필드에 적용한다.
- `SelectedProfileName`은 `AppConfig.SelectedProfileName`과 동기화한다.

### 5.2 추가 명령

다음 명령을 추가한다.

```text
NewProfileCommand
SaveProfileCommand
DeleteProfileCommand
```

각 명령은 기존 `RelayCmd` 패턴을 따른다.

### 5.3 프로필 선택 흐름

프로필 콤보박스에서 선택 변경 시 다음을 수행한다.

```text
1. 선택 프로필 이름을 갱신한다.
2. 선택 프로필 값을 UI 입력 필드에 적용한다.
3. 실행 인자 미리보기를 갱신한다.
4. 설정 파일에 selectedProfileName을 저장한다.
```

주의:

- 프로필 선택만으로 서버를 자동 시작하지 않는다.
- 서버 실행 중에는 프로필 선택을 비활성화하거나 명령에서 차단한다.

### 5.4 새 프로필 생성 흐름

새 프로필 버튼을 누르면 이름 입력 대화상자를 표시한다.

요구사항:

- 현재 UI 값을 기반으로 새 `ServerProfile`을 만든다.
- 빈 이름이면 생성을 막는다.
- 중복 이름이면 생성을 막는다.
- 정상 이름이면 프로필을 추가하고 새 프로필을 선택 상태로 만든다.
- 설정 파일에 저장한다.

문구:

```text
새 프로필 이름을 입력하세요.
새 프로필
프로필 이름을 입력해야 합니다.
같은 이름의 프로필이 이미 있습니다.
```

만약 현재 코드에 일반 입력 대화상자 구조가 없다면, 최소 구현으로 `Microsoft.VisualBasic.Interaction.InputBox` 사용을 검토할 수 있다. 단, 프로젝트 참조나 빌드 문제가 생기면 간단한 WPF Window를 코드 내부 또는 XAML로 구현한다.

### 5.5 프로필 저장 흐름

프로필 저장 버튼을 누르면 다음을 수행한다.

```text
1. 현재 UI 값을 읽는다.
2. 선택된 프로필에 값을 덮어쓴다.
3. `server.local.json`에 저장한다.
4. 저장 완료 메시지를 표시하거나 로그에 남긴다.
```

서버 실행 중에는 저장을 막는다.

문구:

```text
서버 실행 중에는 프로필을 저장할 수 없습니다.
프로필을 저장했습니다.
```

### 5.6 프로필 삭제 흐름

프로필 삭제 버튼을 누르면 다음을 수행한다.

```text
1. 서버 실행 중이면 차단한다.
2. 프로필이 1개뿐이면 삭제를 막는다.
3. 삭제 확인 대화상자를 표시한다.
4. 확인 시 삭제한다.
5. 삭제 후 남은 첫 번째 프로필을 선택한다.
6. 설정 파일에 저장한다.
```

문구:

```text
선택한 프로필을 삭제할까요?
프로필은 최소 1개 이상 필요합니다.
서버 실행 중에는 프로필을 삭제할 수 없습니다.
```

### 5.7 버튼/콤보박스 활성화 정책

다음 상태에서는 프로필 관련 UI를 비활성화한다.

```text
서버 시작 중
서버 실행 중
서버 종료 중
```

서버 중지 상태에서는 활성화한다.

관련 계산 속성 예:

```text
CanEditProfiles
```

상태가 바뀔 때 `CanExecuteChanged`와 property changed가 갱신되어야 한다.

### 5.8 실행 흐름 연결

서버 시작 시 다음 흐름을 따른다.

```text
SelectedProfile 또는 현재 UI 값
↓
ServerRunConfig 생성
↓
ArgBuilder.BuildArgs(ServerRunConfig)
↓
ServerProc.Start(ServerRunConfig, arguments)
↓
LogTailer.Start(ServerRunConfig.ActiveLogFilePath)
```

주의:

- 기준 로그 경로는 `ServerProfile.LogFilePath`에 남긴다.
- 실제 실행 로그 경로는 `ServerRunConfig.ActiveLogFilePath`에만 둔다.
- `ActiveLogFilePath` UI 표시 속성은 기존처럼 유지한다.
- 저장 JSON에는 `ActiveLogFilePath`가 없어야 한다.

### 5.9 기존 명령과의 관계

기존 명령:

```text
SaveCommand
LoadCommand
ResetCommand
StartCommand
StopCommand
ClearLogCommand
BrowseExeCommand
BrowseWorkDirCommand
BrowseLogFileCommand
```

요구사항:

- 기존 명령을 삭제하지 않는다.
- 기존 저장 명령이 있다면 v1.1에서는 현재 선택된 프로필 저장과 충돌하지 않도록 정리한다.
- `SaveCommand`를 기존 설정 저장으로 유지할지, `SaveProfileCommand`와 같은 동작으로 연결할지는 코드 구조에 맞게 결정하되 UI 문구와 실제 동작이 어긋나면 안 된다.
- `ResetCommand`는 기본 v1.1 설정 또는 현재 프로필 기본값 적용 흐름으로 안전하게 유지한다.

### 5.10 버전 표시

`AppVersionText` 또는 상수 표시를 v1.1 기준으로 갱신한다.

예:

```text
버전: v1.1.0
```

## 6. 금지사항

- 이름 변경, 복제, 가져오기, 내보내기 구현 금지.
- 프로필 선택만으로 서버 자동 시작 금지.
- 서버 실행 중 선택 프로필 변경으로 실행 중 서버 설정 변경 금지.
- `ActiveLogFilePath` 저장 금지.
- 여러 서버 동시 실행 구현 금지.
- 서버 자동 재시작 구현 금지.
- Unreal 빌드/Cook/Stage 자동화 구현 금지.
- unrelated cleanup 또는 대규모 리포맷 금지.

## 7. 완료 기준

- `dotnet build`가 성공한다.
- 프로필 콤보박스가 표시된다.
- 기본 프로필 `로컬 테스트 서버`가 표시된다.
- 프로필 추가가 가능하다.
- 빈 이름 프로필 생성이 차단된다.
- 중복 이름 프로필 생성이 차단된다.
- 프로필 선택 시 UI 설정값이 변경된다.
- 프로필 저장 후 앱 재실행 시 변경값이 유지된다.
- 프로필 삭제가 가능하다.
- 프로필 1개뿐일 때 삭제가 차단된다.
- 마지막 선택 프로필이 앱 재실행 후 유지된다.
- 선택한 프로필 값으로 서버 시작/종료/재시작이 가능하다.
- 서버 실행 중 프로필 선택/추가/저장/삭제가 비활성화되거나 차단된다.
- `ActiveLogFilePath`와 기준 `LogFilePath` 분리가 유지된다.
- v1.0 핵심 기능인 서버 시작/종료/로그 tail/앱 닫기 확인/publish 동작이 깨지지 않는다.

## 8. 검증 방법

```powershell
cd Tools/CFServerLauncher/src/CFServerLauncher
dotnet build
```

수동 검증:

1. 앱 실행 후 프로필 콤보박스 표시 확인.
2. `로컬 테스트 서버` 표시 확인.
3. 새 프로필 생성 확인.
4. 빈 이름 입력 시 차단 확인.
5. 중복 이름 입력 시 차단 확인.
6. 프로필 A/B의 포트 또는 맵 경로를 다르게 저장한다.
7. 프로필 A/B 선택 전환 시 UI 값이 바뀌는지 확인한다.
8. 앱 재실행 후 마지막 선택 프로필 유지 확인.
9. 서버 실행 중 프로필 콤보박스와 버튼 비활성화 또는 차단 확인.
10. 서버 시작 후 `ActiveLogFilePath`에 timestamp 로그 파일 경로가 표시되는지 확인.
11. 저장 JSON에 `activeLogFilePath`가 없는지 확인.
12. 서버 종료/재시작 확인.
13. 서버 실행 중 앱 닫기 확인 대화상자 유지 확인.
14. `publish.ps1` 기존 동작 확인.

## 9. 참고 문서

- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/ProfileUISpec.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/ProfilePlan.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/ProfileDesign.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/CheckList.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/Roadmap.md`
