# CFServerLauncher v1.1 Codex Task Source

문서 버전: v1.1.0-codex-source  
작성일: 2026-06-01  
문서 상태: Generated  
artifact_role: task_source  
codex_input: false  
source_folder_path: `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1`

## Goal

CFServerLauncher v1.1 코드 작업은 v1.0의 단일 서버 실행 설정 구조를 다중 실행 프로필 구조로 개편하고, 기존 v1.0 서버 시작/종료/로그 tail/publish 동작을 회귀 없이 유지하는 것이다.

핵심 분리 원칙:

- `AppConfig`는 런처 전체 설정만 담당한다.
- `ServerProfile`은 저장되는 서버 실행 프로필만 담당한다.
- `ServerRunConfig`는 실제 서버 실행 1회에 사용할 런타임 값만 담당한다.
- `ServerProfile.LogFilePath`는 기준 로그 경로이다.
- `ServerRunConfig.ActiveLogFilePath`는 timestamp가 적용된 실제 실행 로그 경로이다.
- `ActiveLogFilePath`는 설정 파일에 저장하지 않는다.

## Current Code Context

현재 구현은 v1.0 구조이다.

- `Models/AppConfig.cs`가 `ServerExePath`, `WorkingDir`, `MapPath`, `Port`, `LogFilePath`, `ExtraArgs`, `AutoScrollLog`를 직접 가진다.
- `Services/ConfigStore.cs`가 v1.0 `AppConfig` 구조로 기본 설정을 만들고 저장한다.
- `Services/ArgBuilder.cs`, `Services/PathGuard.cs`, `Services/ServerProc.cs`가 `AppConfig`를 기준으로 동작한다.
- `ViewModels/MainViewModel.cs`가 UI 값에서 `AppConfig`를 만들고 서버 실행 시 runtime log path를 처리한다.
- `MainWindow.xaml`은 아직 프로필 선택 영역을 가지지 않는다.

## Global Constraints

- 파일명은 32자 이하를 유지한다.
- 모든 신규 public class, public property, public method에는 기존 스타일과 동일한 XML summary 주석을 추가한다.
- 기존 네임스페이스 `CFServerLauncher.*` 구조를 유지한다.
- 불필요한 대규모 리포맷, 스타일 변경, unrelated cleanup을 하지 않는다.
- Unreal 서버 C++ 코드에는 접근하지 않는다.
- 여러 서버 동시 실행을 구현하지 않는다.
- 서버 자동 재시작을 구현하지 않는다.
- Unreal 빌드/Cook/Stage 자동화를 구현하지 않는다.
- RCON, 웹 대시보드, CPU/메모리/TPS 모니터링을 구현하지 않는다.
- 프로필 선택만으로 서버를 자동 시작하지 않는다.
- 실행 중인 서버의 `ServerRunConfig`를 중간에 변경하지 않는다.
- `ActiveLogFilePath`를 `server.local.json` 또는 sample config에 저장하지 않는다.

## Task Candidates

### CFSL-11-T1. Core profile config and migration

목표:
- v1.1 모델, 설정 저장 구조, v1.0 마이그레이션, 프로필 서비스, `ServerRunConfig` 기반 실행 준비를 구현한다.
- UI 배치는 이 작업에서 변경하지 않는다. 단, 기존 `MainViewModel`이 빌드되도록 필요한 연결 변경은 허용한다.

대상 파일:
- 신규: `Tools/CFServerLauncher/src/CFServerLauncher/Models/ServerProfile.cs`
- 신규: `Tools/CFServerLauncher/src/CFServerLauncher/Models/ServerRunConfig.cs`
- 신규: `Tools/CFServerLauncher/src/CFServerLauncher/Services/ProfileService.cs`
- 수정: `Tools/CFServerLauncher/src/CFServerLauncher/Models/AppConfig.cs`
- 수정: `Tools/CFServerLauncher/src/CFServerLauncher/Services/ConfigStore.cs`
- 수정: `Tools/CFServerLauncher/src/CFServerLauncher/Services/ArgBuilder.cs`
- 수정: `Tools/CFServerLauncher/src/CFServerLauncher/Services/PathGuard.cs`
- 수정: `Tools/CFServerLauncher/src/CFServerLauncher/Services/ServerProc.cs`
- 필요 시 최소 수정: `Tools/CFServerLauncher/src/CFServerLauncher/ViewModels/MainViewModel.cs`

구현 요구사항:
- `ServerProfile` 속성: `Name`, `ServerExePath`, `WorkingDir`, `MapPath`, `Port`, `LogFilePath`, `ExtraArgs`, `AutoScrollLog`.
- `ServerRunConfig` 속성: `ProfileName`, `ServerExePath`, `WorkingDir`, `MapPath`, `Port`, `BaseLogFilePath`, `ActiveLogFilePath`, `ExtraArgs`.
- `AppConfig` v1.1 속성: `Version`, `SelectedProfileName`, `MaxLogLines`, `Profiles`.
- `AppConfig`는 서버 실행 경로를 직접 들지 않는다. 단, v1.0 JSON 역직렬화 또는 마이그레이션에 필요한 호환 처리가 필요하면 private/internal helper 또는 별도 legacy DTO를 사용한다.
- `ConfigStore.CreateDefault()`는 v1.1 기본 설정을 만든다. 기본 프로필 이름은 `로컬 테스트 서버`이다.
- `ConfigStore.LoadOrDefault()`는 v1.0 구조를 감지하면 v1.1로 변환한다.
- v1.0 감지 기준: `version == "1.0"`, `profiles` 배열 없음, 또는 루트에 `serverExePath`/`workingDir`/`mapPath` 존재.
- 마이그레이션 전 기존 설정을 `server.local.v1.0.bak.json`으로 백업한다.
- 백업 실패는 앱 실행을 막지 않되, `LoadOrDefault(out string? errorMessage)` 또는 로그 가능한 메시지로 남긴다.
- `profiles`가 비어 있으면 기본 프로필 1개를 생성한다.
- `SelectedProfileName`이 없거나 유효하지 않으면 첫 번째 프로필 이름으로 보정한다.
- 중복 프로필 이름은 앱 시작 시 `이름`, `이름 2`, `이름 3` 방식으로 보정한다.
- `ProfileService`는 기본 프로필 생성, 중복 이름 검사, 프로필 찾기, 추가, 삭제, selectedProfileName 보정, v1.0 변환 보조 로직을 담당한다.
- `ArgBuilder`, `PathGuard`, `ServerProc`는 가능하면 `ServerRunConfig` 기준으로 동작하도록 변경한다.
- 기존 `LogFilePath` 기준 로그 경로에서 runtime timestamp 로그 파일을 만들 때, 기준 경로는 보존하고 `ActiveLogFilePath`만 런타임 값으로 사용한다.
- `EnsureSampleFile()`은 v1.1 sample config를 생성한다.

금지사항:
- `ActiveLogFilePath`를 `AppConfig`나 `ServerProfile`에 추가하지 않는다.
- 마이그레이션 후 v1.0 루트 실행 필드를 `server.local.json`에 계속 저장하지 않는다.
- UI 프로필 콤보박스/버튼 배치는 이 작업의 주요 범위로 구현하지 않는다.

Acceptance:
- `server.local.json`이 v1.1 구조로 저장된다.
- v1.0 `server.local.json`을 준비하고 앱 로드 경로를 타면 `로컬 테스트 서버` 프로필로 자동 변환된다.
- `server.local.v1.0.bak.json` 백업이 생성된다.
- 최소 1개 프로필이 항상 보장된다.
- 마지막 선택 프로필 이름이 유효한 값으로 보정된다.
- `ActiveLogFilePath`는 저장 JSON에 나타나지 않는다.
- 기존 서버 시작/종료/로그 tail 관련 public 흐름이 빌드 수준에서 깨지지 않는다.

Verification:
- `cd Tools/CFServerLauncher/src/CFServerLauncher`
- `dotnet build`
- 수동 JSON 확인: 새로 생성된 `Config/server.local.json`에 `version`, `selectedProfileName`, `maxLogLines`, `profiles`가 있고 루트 `serverExePath`, `workingDir`, `mapPath`, `port`, `logFilePath`, `extraArgs`, `autoScrollLog`, `activeLogFilePath`가 없어야 한다.
- 수동 마이그레이션 확인: v1.0 형태의 `server.local.json`을 둔 뒤 실행/로드하면 v1.1 구조로 저장되고 `server.local.v1.0.bak.json`이 있어야 한다.

### CFSL-11-T2. Profile UI and ViewModel wiring

목표:
- v1.1 프로필 선택/추가/저장/삭제 UI를 `MainWindow.xaml`과 `MainViewModel.cs`에 연결한다.
- 선택한 프로필 값으로 서버를 시작하고, 마지막 선택 프로필을 유지한다.

선행조건:
- CFSL-11-T1 완료.
- `dotnet build` 성공 상태에서 시작한다.

대상 파일:
- 수정: `Tools/CFServerLauncher/src/CFServerLauncher/ViewModels/MainViewModel.cs`
- 수정: `Tools/CFServerLauncher/src/CFServerLauncher/MainWindow.xaml`
- 필요 시 최소 수정: `Tools/CFServerLauncher/src/CFServerLauncher/MainWindow.xaml.cs`
- 필요 시 최소 수정: `Tools/CFServerLauncher/src/CFServerLauncher/ViewModels/RelayCmd.cs`

구현 요구사항:
- 서버 설정 영역 위에 프로필 영역을 추가한다.
- 프로필 영역 구성: `[프로필] [프로필 콤보박스] [새 프로필] [프로필 저장] [프로필 삭제]`.
- UI 문구는 한글 기준으로 사용한다.
- `MainViewModel`에 `Profiles`, `SelectedProfile`, `SelectedProfileName`을 추가한다.
- `MainViewModel`에 `NewProfileCommand`, `SaveProfileCommand`, `DeleteProfileCommand`를 추가한다.
- 프로필 선택 시 선택한 프로필 값을 UI 입력 필드에 적용하고 실행 인자 미리보기를 갱신한다.
- 프로필 선택 시 `selectedProfileName`을 저장한다.
- 새 프로필은 현재 UI 값을 기반으로 생성한다.
- 새 프로필 이름이 비어 있으면 생성을 막고 `프로필 이름을 입력해야 합니다.` 메시지를 표시한다.
- 새 프로필 이름이 중복이면 생성을 막고 `같은 이름의 프로필이 이미 있습니다.` 메시지를 표시한다.
- 프로필 저장은 현재 UI 값을 선택된 프로필에 덮어쓴다.
- 프로필 삭제는 확인 대화상자 이후 삭제한다.
- 프로필이 1개뿐이면 삭제를 막고 `프로필은 최소 1개 이상 필요합니다.` 메시지를 표시한다.
- 서버 실행 중, 시작 중, 종료 중에는 프로필 선택/새 프로필/저장/삭제를 비활성화하거나 명령에서 차단한다.
- 서버 실행 중 차단 메시지는 `서버 실행 중에는 프로필을 변경할 수 없습니다.` 또는 문맥별 저장/삭제 차단 문구를 사용한다.
- 기존 `SaveCommand`, `LoadCommand`, `ResetCommand`와 새 프로필 저장 흐름이 충돌하지 않게 정리한다.
- AppVersion 표시를 v1.1 기준으로 갱신한다.

금지사항:
- 이름 변경, 프로필 복제, 가져오기, 내보내기는 구현하지 않는다.
- 프로필 선택만으로 서버를 시작하지 않는다.
- 서버 실행 중 선택 프로필 변경으로 실행 중 서버 설정을 바꾸지 않는다.
- `ActiveLogFilePath`를 설정 파일에 저장하지 않는다.

Acceptance:
- 프로필 콤보박스가 표시된다.
- 기본 프로필 `로컬 테스트 서버`가 표시된다.
- 프로필 추가/선택/저장/삭제가 가능하다.
- 마지막 선택 프로필이 앱 재실행 후 유지된다.
- 프로필을 선택하면 서버 EXE/작업 폴더/맵/포트/로그 기준 경로/추가 인자가 UI에 반영된다.
- 선택한 프로필 값으로 서버 시작/종료/재시작이 가능하다.
- 서버 실행 중 프로필 변경 관련 UI가 비활성화되거나 명령에서 차단된다.
- v1.0 핵심 기능인 서버 시작/종료/로그 tail/앱 닫기 확인/publish 스크립트 동작이 깨지지 않는다.

Verification:
- `cd Tools/CFServerLauncher/src/CFServerLauncher`
- `dotnet build`
- 앱 실행 후 프로필 콤보박스 표시 확인.
- `로컬 테스트 서버` 표시 확인.
- 새 프로필 생성, 중복 이름 차단, 빈 이름 차단 확인.
- 프로필별 포트 또는 맵 경로를 다르게 저장한 뒤 선택 전환 시 UI 값 변경 확인.
- 앱 재실행 후 마지막 선택 프로필 유지 확인.
- 서버 실행 중 프로필 선택/저장/삭제 비활성화 또는 차단 확인.
- 서버 시작 후 `ActiveLogFilePath`에는 timestamp 로그 파일이 표시되고, 저장 JSON에는 기준 `logFilePath`만 남는지 확인.
- `publish.ps1` 기존 동작이 깨지지 않았는지 확인.

## Overall Acceptance Criteria

- v1.0 설정 마이그레이션 성공.
- 프로필 추가/선택/저장/삭제 성공.
- 마지막 선택 프로필 유지.
- 선택 프로필로 서버 시작/종료/재시작 성공.
- `ActiveLogFilePath`와 기준 `LogFilePath` 분리 유지.
- v1.0 핵심 기능 회귀 없음.
- `dotnet build` 성공.

## Overall Verification

- `cd Tools/CFServerLauncher/src/CFServerLauncher`
- `dotnet build`
- 체크리스트 문서 `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/CheckList.md`의 B/M/P/N/S/D/C/R 항목을 수동 검증한다.

## Source References

- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/README.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/ProfilePlan.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/ProfileDesign.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/ConfigMigration.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/ProfileUISpec.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/Roadmap.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/CheckList.md`

## Unresolved

- 없음.

## Changelog

- v1.1.0-codex-source: Codex용 2단계 작업지시서 소스 생성.
