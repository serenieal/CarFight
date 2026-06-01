# CFServerLauncher v1.1 설정 마이그레이션

문서 버전: v1.1.0-migration  
작성 기준일: 2026-05-29

## 1. 목적

v1.1은 v1.0의 단일 설정 구조를 다중 프로필 구조로 변경한다.

이 문서는 기존 `server.local.json`을 잃지 않고 v1.1 구조로 변환하는 기준을 정의한다.

## 2. v1.0 설정 구조

v1.0은 서버 실행 설정을 `AppConfig`가 직접 가진다.

예시:

```json
{
  "version": "1.0",
  "serverExePath": "D:\\Work\\CarFight_git\\Tools\\CFServerLauncher\\Server\\WindowsServer\\CarFight_ReServer.exe",
  "workingDir": "D:\\Work\\CarFight_git\\Tools\\CFServerLauncher\\Server\\WindowsServer",
  "mapPath": "/Game/Maps/TestMap",
  "port": 7777,
  "logFilePath": "D:\\Work\\CarFight_git\\Tools\\CFServerLauncher\\Logs\\server.log",
  "extraArgs": "",
  "maxLogLines": 1000,
  "autoScrollLog": true
}
```

## 3. v1.1 설정 구조

v1.1은 런처 전역 설정과 서버 실행 프로필을 분리한다.

예시:

```json
{
  "version": "1.1",
  "selectedProfileName": "로컬 테스트 서버",
  "maxLogLines": 1000,
  "profiles": [
    {
      "name": "로컬 테스트 서버",
      "serverExePath": "D:\\Work\\CarFight_git\\Tools\\CFServerLauncher\\Server\\WindowsServer\\CarFight_ReServer.exe",
      "workingDir": "D:\\Work\\CarFight_git\\Tools\\CFServerLauncher\\Server\\WindowsServer",
      "mapPath": "/Game/Maps/TestMap",
      "port": 7777,
      "logFilePath": "D:\\Work\\CarFight_git\\Tools\\CFServerLauncher\\Logs\\server.log",
      "extraArgs": "",
      "autoScrollLog": true
    }
  ]
}
```

## 4. 변환 규칙

v1.0 필드를 v1.1 기본 프로필로 이동한다.

| v1.0 필드 | v1.1 위치 |
|---|---|
| `serverExePath` | `profiles[0].serverExePath` |
| `workingDir` | `profiles[0].workingDir` |
| `mapPath` | `profiles[0].mapPath` |
| `port` | `profiles[0].port` |
| `logFilePath` | `profiles[0].logFilePath` |
| `extraArgs` | `profiles[0].extraArgs` |
| `autoScrollLog` | `profiles[0].autoScrollLog` |
| `maxLogLines` | `maxLogLines` |

새로 추가되는 값:

| v1.1 필드 | 기본값 |
|---|---|
| `version` | `1.1` |
| `selectedProfileName` | `로컬 테스트 서버` |
| `profiles[0].name` | `로컬 테스트 서버` |

## 5. 마이그레이션 감지 기준

다음 조건 중 하나이면 v1.0 설정으로 본다.

```text
1. version이 "1.0"이다.
2. profiles 배열이 없다.
3. serverExePath/workingDir/mapPath가 AppConfig 루트에 존재한다.
```

다음 조건이면 v1.1 설정으로 본다.

```text
1. version이 "1.1"이다.
2. profiles 배열이 있다.
3. profiles 배열에 최소 1개 이상의 프로필이 있다.
```

## 6. 안전 처리 정책

### 6.1 설정 파일이 없는 경우

기본 v1.1 설정을 생성한다.

```text
selectedProfileName = 로컬 테스트 서버
profiles = [로컬 테스트 서버]
```

### 6.2 profiles가 비어 있는 경우

기본 프로필 1개를 생성한다.

### 6.3 selectedProfileName이 없거나 잘못된 경우

첫 번째 프로필을 선택한다.

### 6.4 프로필 이름이 중복된 경우

앱 시작 시 중복 이름을 보정한다.

예시:

```text
로컬 테스트 서버
로컬 테스트 서버 2
로컬 테스트 서버 3
```

## 7. 백업 정책

v1.0 설정을 v1.1로 마이그레이션하기 전에 백업 파일을 만든다.

권장 백업 파일명:

```text
server.local.v1.0.bak.json
```

백업 실패가 앱 실행을 막을 정도의 치명적 오류는 아니다.  
다만 백업 실패 메시지는 런처 로그에 남긴다.

## 8. 저장 정책

마이그레이션 후에는 최신 v1.1 구조로 저장한다.

주의 사항:

```text
ActiveLogFilePath는 저장하지 않는다.
프로필의 LogFilePath에는 기준 로그 경로만 저장한다.
런타임 timestamp 로그 파일명은 저장하지 않는다.
```

## 9. 검증 기준

```text
1. v1.0 server.local.json을 준비한다.
2. v1.1 앱을 실행한다.
3. "로컬 테스트 서버" 프로필이 생성되는지 확인한다.
4. 기존 서버 EXE/작업 폴더/맵/포트/로그 경로가 프로필로 이동했는지 확인한다.
5. server.local.json이 v1.1 구조로 저장되는지 확인한다.
6. 백업 파일이 생성되는지 확인한다.
7. 선택된 프로필로 서버 시작/종료가 가능한지 확인한다.
```
