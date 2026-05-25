# Source Build 빌드 검증 절차

문서 버전: v0.1
작성 기준일: 2026-05-22
대상 프로젝트: CarFight

## 1. 목적

이 문서는 UE 5.7 Source Build 엔진으로 전환한 뒤 CarFight 프로젝트가 빌드와 서버 실행 검증을 통과하는지 확인하기 위한 절차다.

이 문서는 게임 기능 구현 문서가 아니다.

## 2. 검증 전제

| 항목 | 값 |
|---|---|
| Source Build 엔진 | `D:\UE_5.7_Source` |
| 프로젝트 파일 | `D:\Work\CarFight_git\UE\CarFight_Re.uproject` |
| Editor Target | `CarFight_ReEditor` |
| Server Target | `CarFight_ReServer` |
| 플랫폼 | `Win64` |
| 구성 | `Development` |
| Cook 플랫폼 | `WindowsServer` |

`CarFight_ReServer` Target이 없으면 이번 작업에서는 새로 만들지 않고 보류로 기록한다.

## 3. 검증 순서

| 단계 | 항목 | 성공 기준 |
|---|---|---|
| 1 | Source Build Editor 실행 | Fatal Error 없이 실행 |
| 2 | CarFight 프로젝트 열기 | Source Build 엔진으로 열림 |
| 3 | Editor Target 빌드 | `CarFight_ReEditor` 성공 |
| 4 | Server Target 빌드 | `CarFight_ReServer` 성공 |
| 5 | WindowsServer Cook | Cook 산출물 생성 |
| 6 | Dedicated Server 실행 | 서버 프로세스 기동 |

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

## 7. Server Target 빌드

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

## 9. Dedicated Server 실행

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
CarFight_ReServer 빌드:
WindowsServer Cook:
Dedicated Server 실행:

최종 판정:
MP_ServerPlan 복귀 가능 여부:
남은 이슈:
```

## 11. MP_ServerPlan 복귀 조건

- Source Build 전환 완료
- Editor 빌드 완료
- Server 빌드 완료
- WindowsServer Cook 완료
- Dedicated Server 실행 검증 완료

위 조건을 만족하면 MP_ServerPlan 작업으로 복귀한다.
