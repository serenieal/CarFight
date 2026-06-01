# CFServerLauncher 설계 문서

문서 버전: v1.0.0  
작성 기준일: 2026-05-26

## 1. 설계 목표

`CFServerLauncher`는 Unreal Dedicated Server를 외부에서 관리하는 C# WPF 런처이다.

설계 목표는 다음과 같다.

1. Unreal 서버 코드와 런처 코드를 강하게 분리한다.
2. 서버 내부 게임 규칙에 직접 접근하지 않는다.
3. 외부 프로세스 실행, 종료, 로그 표시만 담당한다.
4. 설정 파일 기반으로 경로 변경에 대응한다.
5. v1.0은 작고 안정적인 구조를 우선한다.

## 2. 아키텍처 개요

v1.0은 간단한 MVVM 구조를 사용한다.

```text
View
  MainWindow.xaml
  MainWindow.xaml.cs

ViewModel
  MainViewModel.cs
  RelayCmd.cs

Model
  AppConfig.cs
  ProcInfo.cs
  RunState.cs

Service
  ConfigStore.cs
  ArgBuilder.cs
  ServerProc.cs
  LogTailer.cs
  PathGuard.cs

Utils
  AppConst.cs
  LineBuffer.cs
```

## 3. 계층별 책임

| 계층 | 책임 |
|---|---|
| View | 화면 배치, 입력 컨트롤, 버튼, 로그 표시 |
| ViewModel | UI 상태, 명령 처리, 서비스 호출, 바인딩 값 관리 |
| Model | 설정값, 프로세스 정보, 실행 상태 표현 |
| Service | JSON 저장, 인자 조립, 프로세스 제어, 로그 tail, 경로 검증 |
| Utils | 상수, 로그 라인 버퍼 등 공통 유틸리티 |

## 4. 주요 클래스

## 4.1 AppConfig

설정 JSON과 직접 대응되는 모델이다.

| 속성 | 설명 |
|---|---|
| `Version` | 설정 파일 버전 |
| `ServerExePath` | 서버 실행 파일 경로 |
| `WorkingDir` | 서버 작업 폴더 |
| `MapPath` | 서버가 로드할 맵 경로 |
| `Port` | 서버 포트 |
| `LogFilePath` | `-AbsLog=`에 사용할 로그 파일 경로 |
| `ExtraArgs` | 추가 실행 인자 |
| `MaxLogLines` | UI에 유지할 최대 로그 줄 수 |
| `AutoScrollLog` | 로그 자동 스크롤 여부 |

## 4.2 ProcInfo

런처가 시작한 서버 프로세스 정보를 담는다.

| 속성 | 설명 |
|---|---|
| `ProcessId` | 서버 PID |
| `StartTime` | 서버 시작 시각 |
| `ExitCode` | 서버 종료 코드 |
| `ExePath` | 실행한 서버 EXE 경로 |
| `WorkingDir` | 실행 당시 작업 폴더 |

## 4.3 RunState

서버 실행 상태 enum이다.

| 값 | 한글 표시 |
|---|---|
| `Stopped` | 중지됨 |
| `Starting` | 시작 중 |
| `Running` | 실행 중 |
| `Stopping` | 종료 중 |
| `Exited` | 종료됨 |
| `Error` | 오류 |

## 4.4 MainViewModel

UI 상태와 명령을 관리한다.

주요 책임은 다음과 같다.

1. `AppConfig` 값을 UI에 바인딩한다.
2. 서버 시작, 종료, 설정 저장, 설정 불러오기 명령을 제공한다.
3. 현재 실행 상태와 PID를 표시한다.
4. `ArgBuilder`를 통해 실행 인자 미리보기를 갱신한다.
5. `LogTailer`에서 받은 로그 라인을 UI 로그 버퍼에 추가한다.

## 4.5 ConfigStore

설정 JSON 저장과 불러오기를 담당한다.

주요 책임은 다음과 같다.

1. `Config/server.local.json`을 읽는다.
2. 파일이 없으면 기본 설정을 만든다.
3. 설정 저장 시 임시 파일을 사용해 원자적 저장에 가깝게 처리한다.
4. JSON 파싱 실패 시 앱이 종료되지 않도록 오류를 반환한다.

## 4.6 ArgBuilder

서버 실행 인자를 조립한다.

항상 포함해야 하는 필수 인자는 다음과 같다.

```text
-log
-unattended
-NoSound
-LiveCoding=false
-AbsLog="..."
```

조립 순서는 다음과 같다.

```text
맵 경로
-port=포트
-log
-unattended
-NoSound
-LiveCoding=false
-AbsLog="로그파일"
추가 인자
```

## 4.7 ServerProc

서버 프로세스 시작과 종료를 담당한다.

주요 책임은 다음과 같다.

1. `ProcessStartInfo`를 구성한다.
2. `WorkingDirectory`를 지정한다.
3. 서버 프로세스를 시작한다.
4. PID를 저장한다.
5. 중복 실행을 방지한다.
6. `Exited` 이벤트를 통해 종료 상태를 ViewModel에 전달한다.
7. 런처가 시작한 프로세스만 종료한다.

## 4.8 LogTailer

`-AbsLog=...`로 지정한 파일을 tail 방식으로 읽는다.

주요 책임은 다음과 같다.

1. 마지막으로 읽은 파일 위치를 저장한다.
2. 새로 추가된 로그 라인만 읽는다.
3. 서버가 파일을 쓰는 동안에도 읽을 수 있도록 공유 읽기를 사용한다.
4. 로그 파일이 줄어들거나 재생성되면 읽기 위치를 초기화한다.
5. UI에 전달할 로그 라인을 이벤트 또는 콜백으로 전달한다.

## 4.9 PathGuard

서버 시작 전 입력값을 검증한다.

| 검증 대상 | 기준 |
|---|---|
| 서버 EXE | 파일 존재, `.exe` 확장자 |
| 작업 폴더 | 폴더 존재 |
| 맵 경로 | 비어 있지 않음, `/Game/` 시작 권장 |
| 포트 | 1~65535 범위의 숫자 |
| 로그 파일 | 부모 폴더 생성 가능 |

## 4.10 LineBuffer

UI 로그 표시용 최대 라인 수 제한 버퍼이다.

역할은 다음과 같다.

1. 새 로그 라인을 추가한다.
2. 최대 줄 수를 초과하면 오래된 줄을 제거한다.
3. 전체 로그 문자열을 UI에 제공한다.

## 5. 의존성 방향

의존성은 아래 방향으로만 흐른다.

```text
View -> ViewModel -> Service -> Model/Utils
```

금지하는 의존성은 다음과 같다.

```text
Service -> View
Model -> ViewModel
Unreal Server Code -> Launcher
Launcher -> Unreal C++ Class
```

## 6. 서버 코드와의 분리 기준

런처는 다음 정보만 안다.

1. 서버 EXE 경로
2. 서버 작업 폴더
3. 실행 인자 문자열
4. 로그 파일 경로
5. 실행 중인 PID

런처는 다음 정보에 접근하지 않는다.

1. GameMode 클래스
2. PlayerController 클래스
3. uasset 내부 데이터
4. 맵 내부 액터 구성
5. 서버 내부 관리자 명령
6. 접속 플레이어 목록
7. 게임 규칙 상태

## 7. 파일명과 클래스명 기준

v1.0에서 제안하는 파일명과 클래스명은 모두 32자를 넘지 않는다.

| 파일 | 클래스 |
|---|---|
| `AppConfig.cs` | `AppConfig` |
| `ProcInfo.cs` | `ProcInfo` |
| `RunState.cs` | `RunState` |
| `MainViewModel.cs` | `MainViewModel` |
| `RelayCmd.cs` | `RelayCmd` |
| `ArgBuilder.cs` | `ArgBuilder` |
| `ConfigStore.cs` | `ConfigStore` |
| `LogTailer.cs` | `LogTailer` |
| `PathGuard.cs` | `PathGuard` |
| `ServerProc.cs` | `ServerProc` |
| `AppConst.cs` | `AppConst` |
| `LineBuffer.cs` | `LineBuffer` |
