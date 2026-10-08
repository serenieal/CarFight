# Source Build 빌드 검증 절차

문서 버전: v0.2
작성 기준일: 2026-06-19
대상 프로젝트: CarFight
상태: On Hold / Server Verification Deprecated

## 1. 목적

이 문서는 UE 5.7 Source Build 엔진으로 전환한 뒤 CarFight 프로젝트가 Editor 빌드와 싱글플레이 실행 검증을 통과하는지 확인하기 위한 절차다.

이 문서는 게임 기능 구현 문서가 아니다.

## 2. 검증 전제

| 항목 | 값 |
|---|---|
| Source Build 엔진 | `D:\UE_5.7_Source` |
| 프로젝트 파일 | `D:\Work\CarFight_git\UE\CarFight_Re.uproject` |
| Editor Target | `CarFight_ReEditor` |
| Server Target | Deprecated / 현재 제외 |
| 플랫폼 | `Win64` |
| 구성 | `Development` |
| Cook 플랫폼 | Windows 후보 / 현재 서버 Cook 제외 |

`CarFight_ReServer` Target이 없으면 이번 작업에서는 새로 만들지 않고 Deprecated로 기록한다.

## 3. 검증 순서

| 단계 | 항목 | 성공 기준 |
|---|---|---|
| 1 | Source Build Editor 실행 | Fatal Error 없이 실행 |
| 2 | CarFight 프로젝트 열기 | Source Build 엔진으로 열림 |
| 3 | Editor Target 빌드 | `CarFight_ReEditor` 성공 |
| 4 | PIE Single Player 실행 | 차량 / 카메라 기본 동작 |
| 5 | Standalone Game 실행 | 크래시 없이 실행 |

## 4. Source Build Editor 실행

```bat
"D:\UE_5.7_Source\Engine\Binaries\Win64\UnrealEditor.exe" -log
```

성공 기준:

- Editor가 열린다.
- 시작 직후 Fatal Error가 없다.

## 5. CarFight 프로젝트 열기

```bat
"D:\UE_5.7_Source\Engine\Binaries\Win64\UnrealEditor.exe" "D:\Work\CarFight_git\UE\CarFight_Re.uproject" -log
```

성공 기준:

- 프로젝트가 열린다.
- 누락 모듈 재빌드가 필요하면 Source Build 기준으로 진행된다.
- Editor가 즉시 종료되지 않는다.

## 6. Editor Target 빌드

```bat
cd /d D:\UE_5.7_Source
Engine\Build\BatchFiles\Build.bat -Target="CarFight_ReEditor Win64 Development" -Project="D:\Work\CarFight_git\UE\CarFight_Re.uproject"
```

성공 기준:

- 빌드 성공.
- Installed Build 제한 오류 없음.

## 7. Deprecated: Server Target 빌드

현재 싱글플레이 기준에서는 필수 검증이 아니다.

서버 작업이 공식 목표로 복귀하기 전까지 아래 절차는 수행하지 않는다.

```bat
cd /d D:\UE_5.7_Source
Engine\Build\BatchFiles\Build.bat -Target="CarFight_ReServer Win64 Development" -Project="D:\Work\CarFight_git\UE\CarFight_Re.uproject"
```

성공 기준:

- 빌드 성공.
- 서버 실행 파일 생성.

예상 산출물:

```text
D:\Work\CarFight_git\UE\Binaries\Win64\CarFight_ReServer.exe
```

실패 분류:

| 실패 | 처리 |
|---|---|
| Target 없음 | 이번 작업에서는 생성하지 않음 |
| Installed Build 제한 | EngineAssociation 재확인 |
| 컴파일 오류 | 기존 코드 문제로 분리 |
| 링크 오류 | 모듈 또는 빌드 설정 문제로 분리 |

## 8. WindowsServer Cook

현재 싱글플레이 기준에서는 필수 검증이 아니다.

서버 작업이 공식 목표로 복귀하기 전까지 아래 절차는 수행하지 않는다.

Editor UI 방식:

1. Source Build Editor로 프로젝트 열기.
2. Platforms 메뉴에서 Windows 선택.
3. Build Target을 Server로 설정.
4. Binary Configuration을 Development로 설정.
5. Cook 실행.

명령줄 방식:

```bat
"D:\UE_5.7_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe" "D:\Work\CarFight_git\UE\CarFight_Re.uproject" -run=cook -targetplatform=WindowsServer -unattended -nop4
```

성공 기준:

```text
D:\Work\CarFight_git\UE\Saved\Cooked\WindowsServer
```

## 9. Deprecated: Dedicated Server 실행

현재 싱글플레이 기준에서는 필수 검증이 아니다.

서버 작업이 공식 목표로 복귀하기 전까지 아래 절차는 수행하지 않는다.

```bat
"D:\Work\CarFight_git\UE\Binaries\Win64\CarFight_ReServer.exe" -log
```

성공 기준:

- 서버 로그 창이 열린다.
- Fatal Error가 없다.
- Cook 누락 오류가 없다.
- 서버 프로세스가 즉시 종료되지 않는다.

이번 단계에서 무기, 체력, 대미지, 스코어, 리스폰, 로비, 매치메이킹은 검증하지 않는다.

## 10. 결과 기록 양식

```text
검증 일시:
Source Build 경로:
UE 5.7 소스 브랜치/태그:
EngineAssociation 전환 후 값:

Source Build Editor 실행:
CarFight 프로젝트 열기:
CarFight_ReEditor 빌드:
PIE Single Player 실행:
Standalone Game 실행:

최종 판정:
서버 작업 복귀 필요 여부:
남은 이슈:
```

## 11. 서버 작업 복귀 조건

서버 작업은 현재 복귀 대상이 아니다.

다시 진행하려면 먼저 아래 조건을 확정한다.

- 프로젝트가 다시 멀티플레이 또는 온라인 모드를 목표로 채택한다.
- Server Target 빌드가 현재 코드 기준으로 필요하다.
- WindowsServer Cook과 Dedicated Server 실행 검증이 새 완료 조건으로 승인된다.

---

## ChangeLog

- v0.2 / 2026-06-19
  - Source Build 검증 절차에서 Server Target / WindowsServer Cook / Dedicated Server 실행을 Deprecated 처리했다.
  - 현재 필수 검증을 Editor 빌드, PIE Single Player, Standalone Game으로 전환했다.

## 마이그레이션 지침

- 현재 싱글플레이 작업에서는 서버 빌드 실패를 블로커로 보지 않는다.
- 서버 절차가 필요해지면 이 문서를 그대로 복구하지 말고 최신 프로젝트 목표에 맞춰 새 검증표를 만든다.
