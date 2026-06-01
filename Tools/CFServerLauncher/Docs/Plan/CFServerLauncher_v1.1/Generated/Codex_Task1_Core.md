# Codex Task 1 - CFServerLauncher v1.1 Core Profile Config

문서 버전: v1.1.0-codex-task1  
작성일: 2026-06-01  
작업 유형: 코드 수정  
대상 프로젝트: `Tools/CFServerLauncher/src/CFServerLauncher`

## 1. 목표

CFServerLauncher v1.1의 1차 코드 작업이다.

v1.0의 단일 서버 실행 설정 구조를 v1.1 다중 실행 프로필 구조로 바꾸고, v1.0 `server.local.json`을 자동 마이그레이션한다.

UI 배치는 이번 작업에서 변경하지 않는다. 단, 기존 `MainViewModel.cs`가 빌드되도록 필요한 최소 연결 수정은 허용한다.

## 2. 핵심 설계 원칙

```text
AppConfig        = 런처 전체 설정
ServerProfile    = 저장되는 서버 실행 프로필
ServerRunConfig  = 실제 서버 실행 1회에 사용하는 런타임 설정
```

반드시 지킬 것:

- `ServerProfile.LogFilePath`는 기준 로그 경로이다.
- `ServerRunConfig.ActiveLogFilePath`는 timestamp가 적용된 실제 실행 로그 경로이다.
- `ActiveLogFilePath`는 설정 파일에 저장하지 않는다.
- 프로필 선택만으로 서버를 자동 시작하지 않는다.
- 실행 중인 서버의 `ServerRunConfig`를 중간에 변경하지 않는다.

## 3. 대상 파일

### 신규

```text
Tools/CFServerLauncher/src/CFServerLauncher/Models/ServerProfile.cs
Tools/CFServerLauncher/src/CFServerLauncher/Models/ServerRunConfig.cs
Tools/CFServerLauncher/src/CFServerLauncher/Services/ProfileService.cs
```

### 수정

```text
Tools/CFServerLauncher/src/CFServerLauncher/Models/AppConfig.cs
Tools/CFServerLauncher/src/CFServerLauncher/Services/ConfigStore.cs
Tools/CFServerLauncher/src/CFServerLauncher/Services/ArgBuilder.cs
Tools/CFServerLauncher/src/CFServerLauncher/Services/PathGuard.cs
Tools/CFServerLauncher/src/CFServerLauncher/Services/ServerProc.cs
```

### 필요 시 최소 수정

```text
Tools/CFServerLauncher/src/CFServerLauncher/ViewModels/MainViewModel.cs
```

## 4. 현재 코드 상태

현재 코드는 v1.0 구조이다.

- `AppConfig.cs`가 서버 실행 필드를 직접 가진다.
- `ConfigStore.cs`가 v1.0 `AppConfig` 구조로 기본 설정을 만들고 저장한다.
- `ArgBuilder.cs`, `PathGuard.cs`, `ServerProc.cs`가 `AppConfig` 기준으로 동작한다.
- `MainViewModel.cs`가 UI 값에서 `AppConfig`를 만들고 서버 실행 시 runtime log path를 처리한다.

## 5. 구현 요구사항

### 5.1 ServerProfile

`Models/ServerProfile.cs`를 추가한다.

속성:

```text
Name
ServerExePath
WorkingDir
MapPath
Port
LogFilePath
ExtraArgs
AutoScrollLog
```

설명:

- 저장되는 서버 실행 프로필이다.
- `LogFilePath`는 실제 실행 로그 파일이 아니라 기준 로그 파일 경로이다.

### 5.2 ServerRunConfig

`Models/ServerRunConfig.cs`를 추가한다.

속성:

```text
ProfileName
ServerExePath
WorkingDir
MapPath
Port
BaseLogFilePath
ActiveLogFilePath
ExtraArgs
```

설명:

- 서버 실행 1회에 사용하는 런타임 설정이다.
- `BaseLogFilePath`는 프로필에서 온 기준 로그 경로이다.
- `ActiveLogFilePath`는 이번 실행에서 실제로 tail하고 `-AbsLog`에 넣을 timestamp 로그 경로이다.

### 5.3 AppConfig

`Models/AppConfig.cs`를 v1.1 구조로 변경한다.

속성:

```text
Version
SelectedProfileName
MaxLogLines
Profiles
```

주의:

- v1.1 저장 JSON 루트에 `ServerExePath`, `WorkingDir`, `MapPath`, `Port`, `LogFilePath`, `ExtraArgs`, `AutoScrollLog`가 남으면 안 된다.
- v1.0 JSON 읽기 또는 마이그레이션용 legacy DTO는 허용한다.

### 5.4 ProfileService

`Services/ProfileService.cs`를 추가한다.

책임:

```text
1. 기본 프로필 생성
2. v1.0 설정을 프로필로 변환할 때 필요한 보조 로직
3. 프로필 이름 중복 검사
4. 프로필 추가
5. 프로필 삭제
6. 프로필 찾기
7. selectedProfileName 보정
8. profiles 비어 있음 보정
9. 중복 프로필 이름 보정
```

기본 프로필 이름:

```text
로컬 테스트 서버
```

중복 이름 보정 예:

```text
로컬 테스트 서버
로컬 테스트 서버 2
로컬 테스트 서버 3
```

### 5.5 ConfigStore

`Services/ConfigStore.cs`를 v1.1 구조로 변경한다.

요구사항:

- `CreateDefault()`는 v1.1 기본 설정을 반환한다.
- `EnsureSampleFile()`은 v1.1 sample config를 만든다.
- `LoadOrDefault(out string? errorMessage)`는 다음을 처리한다.
  - 설정 파일 없음 → v1.1 기본 설정 반환.
  - v1.1 구조 → 로드 후 fallback 보정.
  - v1.0 구조 → 백업 후 v1.1로 마이그레이션.
  - 손상된 JSON → 기본 설정 반환, 오류 메시지 제공.
- 마이그레이션 후에는 v1.1 구조로 저장한다.

v1.0 감지 기준:

```text
1. version == "1.0"
2. profiles 배열이 없음
3. 루트에 serverExePath / workingDir / mapPath 중 하나 이상 존재
```

마이그레이션 백업 파일명:

```text
server.local.v1.0.bak.json
```

백업 실패 정책:

- 앱 실행을 막지 않는다.
- `errorMessage` 또는 런처 로그에 남길 수 있는 메시지로 전달한다.

### 5.6 ArgBuilder

`Services/ArgBuilder.cs`를 `ServerRunConfig` 기준으로 변경한다.

요구사항:

- `BuildArgs(ServerRunConfig runConfig)` 형태를 지원한다.
- `-AbsLog=`에는 `runConfig.ActiveLogFilePath`를 사용한다.
- `BuildPreview(ServerRunConfig runConfig)`는 EXE 경로와 인자를 표시한다.
- 기존 호출부가 많으면 호환 overload를 임시 제공해도 되지만, 최종 실행 흐름은 `ServerRunConfig` 기준이어야 한다.

### 5.7 PathGuard

`Services/PathGuard.cs`를 `ServerRunConfig` 기준으로 변경한다.

요구사항:

- 서버 EXE, 작업 폴더, 맵 경로, 포트, 로그 파일 경로 검증을 유지한다.
- 로그 폴더 생성은 `ActiveLogFilePath` 또는 기준 경로의 폴더 기준으로 안전하게 처리한다.

### 5.8 ServerProc

`Services/ServerProc.cs`를 `ServerRunConfig` 기준으로 변경한다.

요구사항:

- `Start(ServerRunConfig runConfig, string arguments)` 형태를 지원한다.
- `ProcessStartInfo.FileName`은 `runConfig.ServerExePath`를 사용한다.
- `ProcessStartInfo.WorkingDirectory`는 `runConfig.WorkingDir`를 사용한다.
- 기존 프로세스 종료/Exited 이벤트/Dispose 흐름은 변경하지 않는다.

### 5.9 MainViewModel 최소 수정

UI 배치는 바꾸지 않는다.

필요한 경우에만 다음을 수정한다.

- 현재 UI 값 → `ServerProfile` 생성.
- 선택 프로필 → `ServerRunConfig` 생성.
- 서버 시작 시 기준 로그 경로는 보존하고, `ActiveLogFilePath`만 runtime timestamp 경로로 설정.
- `config.LogFilePath = runtimeLogFilePath`처럼 저장용 경로를 런타임 경로로 덮어쓰는 구조는 제거한다.

## 6. 금지사항

- `ActiveLogFilePath`를 `AppConfig` 또는 `ServerProfile`에 저장 필드로 추가하지 않는다.
- v1.1 저장 JSON 루트에 v1.0 서버 실행 필드를 남기지 않는다.
- 프로필 UI 콤보박스/버튼은 이 작업에서 구현하지 않는다.
- 여러 서버 동시 실행을 구현하지 않는다.
- 서버 자동 재시작을 구현하지 않는다.
- Unreal 빌드/Cook/Stage 자동화를 구현하지 않는다.
- RCON, 웹 대시보드, CPU/메모리/TPS 모니터링을 구현하지 않는다.
- unrelated cleanup 또는 대규모 리포맷을 하지 않는다.

## 7. 완료 기준

- `dotnet build`가 성공한다.
- `server.local.json`이 v1.1 구조로 저장된다.
- v1.0 `server.local.json`이 `로컬 테스트 서버` 프로필로 자동 변환된다.
- `server.local.v1.0.bak.json` 백업이 생성된다.
- `profiles`가 비어 있으면 기본 프로필이 생성된다.
- `selectedProfileName`이 없거나 잘못되면 첫 번째 프로필로 보정된다.
- 중복 프로필 이름이 보정된다.
- `ActiveLogFilePath`가 저장 JSON에 나타나지 않는다.
- 기존 서버 시작/종료/로그 tail 흐름이 빌드 수준에서 깨지지 않는다.

## 8. 검증 방법

```powershell
cd Tools/CFServerLauncher/src/CFServerLauncher
dotnet build
```

수동 JSON 검증:

1. 설정 파일이 없을 때 앱 로드 또는 기본 저장을 수행한다.
2. `Config/server.local.json`에 다음 루트 필드가 있는지 확인한다.

```json
{
  "version": "1.1",
  "selectedProfileName": "로컬 테스트 서버",
  "maxLogLines": 1000,
  "profiles": []
}
```

3. 루트에 다음 필드가 없어야 한다.

```text
serverExePath
workingDir
mapPath
port
logFilePath
extraArgs
autoScrollLog
activeLogFilePath
```

v1.0 마이그레이션 검증:

1. v1.0 형태의 `server.local.json`을 준비한다.
2. 앱 로드 경로를 실행한다.
3. `server.local.v1.0.bak.json`이 생성됐는지 확인한다.
4. 기존 서버 EXE/작업 폴더/맵/포트/로그 기준 경로가 `profiles[0]`으로 이동했는지 확인한다.
5. 저장 JSON에 `activeLogFilePath`가 없는지 확인한다.

## 9. 참고 문서

- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/README.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/ProfilePlan.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/ProfileDesign.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/ConfigMigration.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/Roadmap.md`
- `Tools/CFServerLauncher/Docs/Plan/CFServerLauncher_v1.1/CheckList.md`
