# CFServerLauncher 설정 JSON 명세

문서 버전: v1.0.0  
작성 기준일: 2026-05-26

## 1. 설정 파일 위치

v1.0 기본 설정 파일 위치는 다음과 같다.

```text
D:\Work\CarFight_git\Tools\CFServerLauncher\Config\server.local.json
```

샘플 설정 파일 위치는 다음과 같다.

```text
D:\Work\CarFight_git\Tools\CFServerLauncher\Config\server.sample.json
```

## 2. Git 관리 정책

`server.sample.json`은 Git에 포함한다.  
`server.local.json`은 사용자 PC 절대 경로가 들어가므로 Git에서 제외한다.

권장 `.gitignore` 항목은 다음과 같다.

```gitignore
Config/server.local.json
Logs/*.log
```

## 3. 기본 설정 예시

```json
{
  "version": "1.0",
  "serverExePath": "D:\\Work\\CarFight_git\\UE\\Saved\\StagedBuilds\\WindowsServer\\CarFight_Re\\Binaries\\Win64\\CarFight_ReServer.exe",
  "workingDir": "D:\\Work\\CarFight_git\\UE\\Saved\\StagedBuilds\\WindowsServer\\CarFight_Re",
  "mapPath": "/Game/Maps/TestMap",
  "port": 7777,
  "logFilePath": "D:\\Work\\CarFight_git\\Tools\\CFServerLauncher\\Logs\\server.log",
  "extraArgs": "",
  "maxLogLines": 1000,
  "autoScrollLog": true
}
```

## 4. 필드 명세

| JSON 필드 | C# 속성 | 타입 | 필수 | 설명 |
|---|---|---|---|---|
| `version` | `Version` | string | 예 | 설정 스키마 버전 |
| `serverExePath` | `ServerExePath` | string | 예 | 서버 EXE 절대 경로 |
| `workingDir` | `WorkingDir` | string | 예 | 서버 프로세스 작업 폴더 |
| `mapPath` | `MapPath` | string | 예 | Unreal 맵 경로 |
| `port` | `Port` | int | 예 | 서버 포트 |
| `logFilePath` | `LogFilePath` | string | 예 | `-AbsLog=`에 사용할 파일 경로 |
| `extraArgs` | `ExtraArgs` | string | 아니오 | 추가 실행 인자 |
| `maxLogLines` | `MaxLogLines` | int | 아니오 | UI 로그 최대 줄 수 |
| `autoScrollLog` | `AutoScrollLog` | bool | 아니오 | 로그 자동 스크롤 여부 |

## 5. 기본값 정책

설정 파일이 없거나 일부 필드가 누락되면 다음 기본값을 사용한다.

| 항목 | 기본값 |
|---|---|
| `version` | `1.0` |
| `serverExePath` | 현재 확정된 스테이징 서버 EXE 경로 |
| `workingDir` | 현재 확정된 스테이징 서버 작업 폴더 |
| `mapPath` | `/Game/Maps/TestMap` |
| `port` | `7777` |
| `logFilePath` | `Tools\CFServerLauncher\Logs\server.log` |
| `extraArgs` | 빈 문자열 |
| `maxLogLines` | `1000` |
| `autoScrollLog` | `true` |

## 6. 저장 정책

설정 저장은 다음 순서로 처리한다.

```text
1. 현재 UI 설정값을 AppConfig로 만든다.
2. JSON 문자열로 직렬화한다.
3. Config 폴더가 없으면 생성한다.
4. 임시 파일에 먼저 저장한다.
5. 기존 server.local.json을 새 파일로 교체한다.
6. 교체가 성공하면 임시 파일을 제거한다.
```

이 정책은 저장 도중 앱이 종료되거나 예외가 발생해도 설정 파일 손상 가능성을 줄이기 위한 것이다.

## 7. 불러오기 정책

설정 불러오기는 다음 순서로 처리한다.

```text
1. server.local.json 존재 여부를 확인한다.
2. 파일이 없으면 기본값을 사용한다.
3. 파일이 있으면 JSON 파싱을 시도한다.
4. 파싱 성공 시 UI에 반영한다.
5. 파싱 실패 시 오류를 표시하고 기본값을 유지한다.
```

## 8. 검증 정책

설정 파일을 읽었다고 해서 바로 서버 실행을 허용하지 않는다.  
서버 시작 버튼을 누르는 시점에 `PathGuard`에서 최종 검증한다.

| 검증 대상 | 검증 기준 | 실패 시 처리 |
|---|---|---|
| 서버 EXE | 파일 존재, `.exe` 확장자 | 시작 차단 |
| 작업 폴더 | 폴더 존재 | 시작 차단 |
| 맵 경로 | 비어 있지 않음 | 시작 차단 |
| 포트 | 1~65535 | 시작 차단 |
| 로그 파일 경로 | 부모 폴더 생성 가능 | 시작 차단 |
| 최대 로그 줄 수 | 100 이상 권장 | 기본값 보정 가능 |

## 9. 필수 실행 인자 정책

다음 실행 인자는 설정 JSON에서 사용자가 직접 관리하지 않는다.  
항상 코드의 `ArgBuilder`에서 강제 포함한다.

```text
-log
-unattended
-NoSound
-LiveCoding=false
-AbsLog="..."
```

이렇게 하는 이유는 사용자가 실수로 필수 인자를 제거해서 서버 실행 환경이 흔들리는 것을 막기 위해서이다.

## 10. 버전 관리 정책

`version` 필드는 설정 파일 스키마 버전을 의미한다.

v1.0에서는 마이그레이션 로직을 복잡하게 만들지 않는다.  
v1.1 이후 설정 필드가 추가될 경우 다음 정책을 사용한다.

1. 기존 필드는 유지한다.
2. 새 필드는 기본값으로 보정한다.
3. 제거된 필드는 즉시 삭제하지 않고 무시한다.
4. 설정 저장 시 최신 스키마로 다시 쓴다.
