# Source Build 전환 계획

문서 버전: v0.2
작성 기준일: 2026-06-19
대상 프로젝트: CarFight
상태: On Hold / Server Objective Deprecated

## 목적

CarFight 프로젝트를 UE 5.7 런처 설치형 엔진에서 UE 5.7 Source Build 엔진으로 전환하는 계획이다.

이번 작업은 개발 환경 전환 작업이다.

2026-06-19 기준 프로젝트가 싱글플레이로 전환되었으므로, 서버 빌드와 Dedicated Server 실행을 위한 Source Build 전환은 현재 필수 작업이 아니다. Source Build는 엔진 수정이나 별도 디버깅 필요가 생길 때만 재검토한다.

## 경로

| 항목 | 값 |
|---|---|
| 프로젝트 루트 | `D:\Work\CarFight_git` |
| 프로젝트 파일 | `D:\Work\CarFight_git\UE\CarFight_Re.uproject` |
| 기존 엔진 | `D:\UE_5.7` |
| 새 엔진 추천 경로 | `D:\UE_5.7_Source` |

## 진행 순서

1. Git 상태를 확인한다.
2. `.uproject` 파일을 백업한다.
3. 기존 `EngineAssociation` 값을 기록한다.
4. Epic GitHub 접근을 확인한다.
5. UE 5.7 소스를 새 엔진 경로에 준비한다.
6. `Setup.bat`을 실행한다.
7. `GenerateProjectFiles.bat`을 실행한다.
8. Visual Studio에서 `UE5.sln`을 연다.
9. `Development Editor`와 `Win64`로 엔진을 빌드한다.
10. `Switch Unreal Engine version...` 메뉴로 프로젝트 엔진 연결을 바꾼다.
11. 프로젝트 파일을 다시 생성한다.
12. Editor 빌드를 검증한다.
13. PIE Single Player 실행을 검증한다.
14. Standalone Game 실행을 검증한다.
15. Server 빌드 / WindowsServer Cook / Dedicated Server 실행은 Deprecated로 기록한다.

## 유지할 것

기존 `D:\UE_5.7` 엔진은 삭제하지 않는다. 전환 실패 시 되돌아가기 위한 기준 엔진으로 남긴다.

## 하지 않을 것

- 게임 코드 작성
- 새 빌드 타깃 작성
- GameMode 작성
- PlayerController 작성
- 전투, 체력, 점수, 리스폰, 로비, 매칭 작업

## 롤백

실패하면 백업한 `.uproject`를 복원하거나 기존 `EngineAssociation` 값으로 되돌린다. 이후 `D:\UE_5.7` 엔진으로 프로젝트를 다시 연다.

## 완료 기준

- Source Build 엔진 빌드 성공
- Source Build Editor 실행 성공
- CarFight 프로젝트 열기 성공
- Editor 빌드 성공
- PIE Single Player 실행 성공
- Standalone Game 실행 성공

## 후속 작업 복귀

위 항목이 끝나도 MP_ServerPlan 작업으로 돌아가지 않는다.

서버 작업은 프로젝트가 다시 멀티플레이 또는 온라인 모드를 공식 목표로 채택한 뒤에만 재개한다.

---

## ChangeLog

- v0.2 / 2026-06-19
  - 서버 빌드 / WindowsServer Cook / Dedicated Server 실행을 현재 필수 절차에서 제외했다.
  - 완료 기준을 PIE Single Player / Standalone Game 중심으로 전환했다.

## 마이그레이션 지침

- 현재 싱글플레이 작업은 Source Build 전환 없이 진행할 수 있다.
- 서버 관련 절차는 온라인 목표가 복귀하기 전까지 새 작업으로 만들지 않는다.
