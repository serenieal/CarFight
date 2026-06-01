# CFServerLauncher v1.0 개발 계획

문서 버전: v1.0.0  
작성 기준일: 2026-05-26  
대상 위치: `D:\Work\CarFight_git\Tools\CFServerLauncher`

## 1. 목표

`CFServerLauncher`는 CarFight Dedicated Server를 관리하기 위한 외부 C# WPF 프로그램이다.

v1.0의 목표는 서버 내부 구현과 분리된 상태에서 다음 작업을 안정적으로 수행하는 것이다.

1. 서버 실행 설정을 JSON으로 저장하고 불러온다.
2. 설정값을 기준으로 실행 인자를 조립한다.
3. 외부 프로세스로 `CarFight_ReServer.exe`를 실행한다.
4. 런처가 실행한 서버 프로세스를 종료한다.
5. `-AbsLog=...`로 고정한 로그 파일을 tail 방식으로 읽어 표시한다.

## 2. 전제 조건

| 항목 | 값 |
|---|---|
| Unreal Engine | UE 5.7 소스 빌드 엔진 |
| 엔진 경로 | `D:\UnrealEngine_Source` |
| 프로젝트 루트 | `D:\Work\CarFight_git` |
| UE 프로젝트 | `D:\Work\CarFight_git\UE\CarFight_Re.uproject` |
| 서버 Target | `CarFight_ReServer` |
| 기본 맵 | `/Game/Maps/TestMap` |
| 기본 포트 | `7777` |
| 서버 EXE | `D:\Work\CarFight_git\UE\Saved\StagedBuilds\WindowsServer\CarFight_Re\Binaries\Win64\CarFight_ReServer.exe` |
| 작업 폴더 | `D:\Work\CarFight_git\UE\Saved\StagedBuilds\WindowsServer\CarFight_Re` |

서버 빌드와 `TestMap` 로드는 이미 성공한 상태로 본다. 런처는 빌드나 Stage를 수행하지 않는다.

## 3. v1.0 필수 기능

| 기능 | 설명 |
|---|---|
| 서버 EXE 경로 입력/저장 | 서버 실행 파일 경로를 설정 JSON에 저장한다. |
| 작업 폴더 입력/저장 | 서버의 `WorkingDirectory`를 설정한다. |
| 맵 경로 입력/저장 | Unreal 맵 경로를 입력한다. |
| 포트 입력/저장 | 서버 포트를 입력한다. |
| 로그 파일 경로 입력/저장 | `-AbsLog=...`에 사용할 절대 경로를 입력한다. |
| 서버 시작 | 외부 프로세스로 서버를 실행한다. |
| 서버 종료 | 런처가 시작한 PID를 기준으로 종료한다. |
| 서버 상태 표시 | 중지, 시작 중, 실행 중, 종료 중, 종료됨, 오류를 표시한다. |
| PID 표시 | 실행된 서버 프로세스 ID를 표시한다. |
| 최근 로그 tail 표시 | 로그 파일의 신규 라인을 UI에 표시한다. |
| 설정 JSON 저장/불러오기 | `server.local.json`을 저장하고 불러온다. |
| 실행 인자 미리보기 | 현재 설정 기준 전체 실행 명령을 표시한다. |

## 4. v1.0 제외 기능

다음 기능은 v1.0에서 구현하지 않는다.

- Unreal C++ 빌드 실행
- Cook/Stage 자동 실행
- uasset 수정
- 맵 수정
- GameMode 수정
- PlayerController 수정
- RCON
- 웹 대시보드
- 게임 내부 관리자 명령
- 복잡한 서버 모니터링
- 자동 재시작
- 원격 서버 관리

## 5. 추천 폴더 구조

```text
D:\Work\CarFight_git
└─ Tools
   └─ CFServerLauncher
      ├─ CFServerLauncher.sln
      ├─ src
      │  └─ CFServerLauncher
      │     ├─ CFServerLauncher.csproj
      │     ├─ App.xaml
      │     ├─ App.xaml.cs
      │     ├─ MainWindow.xaml
      │     ├─ MainWindow.xaml.cs
      │     ├─ Models
      │     │  ├─ AppConfig.cs
      │     │  ├─ ProcInfo.cs
      │     │  └─ RunState.cs
      │     ├─ ViewModels
      │     │  ├─ MainViewModel.cs
      │     │  └─ RelayCmd.cs
      │     ├─ Services
      │     │  ├─ ArgBuilder.cs
      │     │  ├─ ConfigStore.cs
      │     │  ├─ LogTailer.cs
      │     │  ├─ PathGuard.cs
      │     │  └─ ServerProc.cs
      │     └─ Utils
      │        ├─ AppConst.cs
      │        └─ LineBuffer.cs
      ├─ Config
      │  ├─ server.sample.json
      │  └─ server.local.json
      ├─ Logs
      │  └─ .gitkeep
      ├─ Docs
      │  └─ v1-plan.md
      └─ .gitignore
```

## 6. 구현 우선순위

v1.0은 다음 순서로 구현한다.

1. WPF 프로젝트 뼈대 생성
2. 설정 모델 작성
3. JSON 저장/불러오기 구현
4. UI 입력 폼 구성
5. 입력값 검증 구현
6. 실행 인자 조립 구현
7. 서버 프로세스 시작 구현
8. 로그 tail 구현
9. 서버 프로세스 종료 구현
10. 반복 테스트와 예외 메시지 정리

## 7. 성공 기준

v1.0은 다음 조건을 만족하면 완료로 본다.

1. 앱에서 기본 설정값을 확인할 수 있다.
2. 설정을 수정하고 JSON으로 저장할 수 있다.
3. 앱을 재실행해도 이전 설정이 복원된다.
4. 실행 인자 미리보기에 필수 인자가 모두 포함된다.
5. 서버 시작 버튼으로 `CarFight_ReServer.exe`가 실행된다.
6. PID가 UI에 표시된다.
7. `-AbsLog=...` 파일에 서버 로그가 생성된다.
8. UI에서 최근 로그가 tail 방식으로 표시된다.
9. 서버 종료 버튼으로 런처가 실행한 서버 프로세스가 종료된다.
10. 잘못된 경로나 포트를 입력하면 서버 실행이 차단되고 한글 오류가 표시된다.

## 8. 핵심 제약

- 서버 내부 코드에 의존하지 않는다.
- 서버가 사용하는 Unreal 클래스나 uasset을 수정하지 않는다.
- 서버 명령 채널을 새로 만들지 않는다.
- 설정 기반으로 경로 변경에 대응한다.
- 기능 확장보다 반복 실행 안정성을 우선한다.
