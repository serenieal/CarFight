# CFServerLauncher 실행/로그 명세

문서 버전: v1.0.0  
작성 기준일: 2026-05-26

## 1. 목적

이 문서는 `CFServerLauncher`가 `CarFight_ReServer.exe`를 실행하는 방식, 실행 인자 조립 방식, 로그 tail 방식, 서버 종료 방식을 정의한다.

v1.0은 Unreal 서버 내부 명령을 사용하지 않는다. 런처가 제어하는 대상은 외부 프로세스와 로그 파일이다.

## 2. 서버 실행 대상

| 항목 | 값 |
|---|---|
| 실행 파일 | `D:\Work\CarFight_git\UE\Saved\StagedBuilds\WindowsServer\CarFight_Re\Binaries\Win64\CarFight_ReServer.exe` |
| 작업 폴더 | `D:\Work\CarFight_git\UE\Saved\StagedBuilds\WindowsServer\CarFight_Re` |
| 기본 맵 | `/Game/Maps/TestMap` |
| 기본 포트 | `7777` |

## 3. 실행 인자 구성

항상 포함할 인자는 다음과 같다.

```text
/Game/Maps/TestMap
-port=7777
-log
-unattended
-NoSound
-LiveCoding=false
-AbsLog="D:\Work\CarFight_git\Tools\CFServerLauncher\Logs\server.log"
```

## 4. 실행 인자 조립 순서

`ArgBuilder`는 다음 순서로 인자를 조립한다.

```text
1. 맵 경로
2. 포트
3. -log
4. -unattended
5. -NoSound
6. -LiveCoding=false
7. -AbsLog="로그파일경로"
8. 추가 인자
```

예시:

```text
/Game/Maps/TestMap -port=7777 -log -unattended -NoSound -LiveCoding=false -AbsLog="D:\Work\CarFight_git\Tools\CFServerLauncher\Logs\server.log"
```

전체 실행 미리보기 예시:

```text
"D:\Work\CarFight_git\UE\Saved\StagedBuilds\WindowsServer\CarFight_Re\Binaries\Win64\CarFight_ReServer.exe" /Game/Maps/TestMap -port=7777 -log -unattended -NoSound -LiveCoding=false -AbsLog="D:\Work\CarFight_git\Tools\CFServerLauncher\Logs\server.log"
```

## 5. 경로 따옴표 처리

다음 값은 공백을 포함할 수 있으므로 미리보기 문자열에서 따옴표로 감싼다.

- 서버 EXE 경로
- 로그 파일 경로

`ProcessStartInfo.FileName`에는 EXE 경로를 별도 필드로 넣는다.  
`ProcessStartInfo.Arguments`에는 인자 문자열만 넣는다.

## 6. 서버 시작 흐름

```text
1. 현재 UI 설정을 AppConfig로 수집한다.
2. PathGuard로 입력값을 검증한다.
3. 로그 파일 부모 폴더를 생성한다.
4. 기존 로그 파일 처리 정책을 적용한다.
5. ArgBuilder로 실행 인자를 만든다.
6. ProcessStartInfo를 구성한다.
7. WorkingDirectory를 설정한다.
8. 서버 프로세스를 시작한다.
9. PID와 시작 시각을 기록한다.
10. 상태를 실행 중으로 변경한다.
11. LogTailer를 시작한다.
```

## 7. 기존 로그 파일 처리

v1.0에서는 서버 시작 전 기존 로그 파일 삭제를 기본 정책으로 한다.

장점은 다음과 같다.

1. 이전 실행 로그와 현재 실행 로그가 섞이지 않는다.
2. tail 시작 위치 처리가 단순하다.
3. 검증 시 현재 실행 로그만 확인할 수 있다.

기존 로그 파일 삭제에 실패하면 서버 시작을 차단하고 오류를 표시한다.

## 8. 로그 tail 방식

`LogTailer`는 `-AbsLog=...`로 지정한 파일을 주기적으로 읽는다.

v1.0에서는 `FileSystemWatcher`보다 타이머 기반 polling을 우선한다.

권장 주기:

```text
250ms ~ 500ms
```

읽기 방식:

```text
1. 마지막으로 읽은 byte 위치를 저장한다.
2. 현재 파일 크기를 확인한다.
3. 파일 크기가 마지막 위치보다 작으면 재생성으로 보고 위치를 0으로 초기화한다.
4. 마지막 위치부터 끝까지 읽는다.
5. 새 라인을 UI 로그 버퍼에 추가한다.
6. 최대 줄 수를 초과하면 오래된 줄을 제거한다.
```

## 9. 파일 공유 읽기

서버가 로그 파일을 쓰는 중에도 런처가 읽어야 한다.

따라서 파일 열기 시 다음 개념을 사용한다.

```text
FileShare.ReadWrite
FileShare.Delete
```

이렇게 하면 Unreal 서버가 파일을 점유하고 있어도 런처가 읽을 가능성이 높아진다.

## 10. 로그 UI 표시 정책

| 항목 | 정책 |
|---|---|
| 표시 대상 | 최근 서버 로그 |
| 최대 줄 수 | 기본 1000줄 |
| 자동 스크롤 | 기본 켜짐 |
| 로그 지우기 | UI 표시만 삭제 |
| 로그 파일 삭제 | 서버 시작 전 정책에서만 수행 |

## 11. 서버 종료 방식

v1.0은 RCON과 게임 내부 관리자 명령을 사용하지 않는다.

종료 대상은 다음으로 제한한다.

```text
현재 런처가 직접 시작한 서버 프로세스 PID
```

동일한 이름의 다른 프로세스는 종료하지 않는다.

## 12. 서버 종료 흐름

```text
1. 현재 추적 중인 프로세스가 있는지 확인한다.
2. 이미 종료된 프로세스인지 확인한다.
3. 상태를 종료 중으로 바꾼다.
4. 가능한 경우 정상 종료 요청을 시도한다.
5. 정상 종료가 어렵거나 실패하면 프로세스 종료를 수행한다.
6. Exited 이벤트에서 종료 코드와 상태를 갱신한다.
7. PID 표시를 초기화한다.
8. LogTailer를 정지한다.
```

## 13. 종료 정책 주의사항

Dedicated Server는 일반 WPF 앱처럼 `CloseMainWindow()`가 의미 있게 동작하지 않을 수 있다.

따라서 v1.0에서는 다음 정책을 사용한다.

```text
우선 정상 종료 시도
실패하면 프로세스 종료
```

추후 RCON이나 서버 내부 admin command가 생기면 graceful shutdown을 별도 기능으로 추가한다.

## 14. 중복 실행 방지

v1.0에서 런처는 자신이 시작한 서버 프로세스가 실행 중일 때 다시 시작하지 않는다.

처리 정책:

```text
1. 현재 프로세스 객체가 있고 종료되지 않았다면 시작 차단
2. UI에 "서버가 이미 실행 중입니다." 표시
3. 기존 PID를 유지
```

## 15. 실패 처리

| 상황 | 처리 |
|---|---|
| 서버 EXE 없음 | 시작 차단 |
| 작업 폴더 없음 | 시작 차단 |
| 로그 파일 삭제 실패 | 시작 차단 |
| 프로세스 시작 실패 | 상태를 오류로 변경 |
| 서버 비정상 종료 | 상태를 종료됨으로 변경하고 종료 코드 표시 |
| 로그 읽기 실패 | 서버는 유지하고 로그 오류 표시 |
