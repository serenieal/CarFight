# CarFight UE 5.7 Source Build 전환 계획 — Archived

문서 버전: v0.3    
작성 기준일: 2026-06-19  
종료일: 2026-08-08  
대상 프로젝트: CarFight    
문서 위치: `D:\Work\CarFight_git\Document\Plan\EngineSourceBuild`
상태: **Archived / Superseded / Historical**

> 종료 판정: 이 Plan은 더 이상 현재 착수·재개 대상이 아니다. 현재 CarFight 공식 엔진은 **Unreal Engine 5.8 Source Build**이며 기준 루트는 `D:\UnrealEngine_Source`다. 아래 UE 5.7 및 `D:\UE_5.7*` 내용은 전환 당시의 역사 기록으로만 보존하며 현재 명령·호환성·구현 판단에 사용하지 않는다.

---

## 1. 문서 목적

이 폴더는 CarFight 프로젝트를 UE 5.7 런처 기반 Installed Build에서 UE 5.7 Source Build 엔진으로 전환하기 위한 계획 문서 모음이다.

원래 직접 목적은 Dedicated Server Target 빌드를 막고 있는 Installed Build 제한을 우회하고, Source Build 엔진에서 Editor/Server 빌드와 Dedicated Server 실행 검증까지 통과하는지 확인하는 것이었다.

2026-06-19 기준 프로젝트가 싱글플레이로 전환되었으므로, Dedicated Server 목적의 Source Build 전환은 현재 필수 작업이 아니다. Source Build는 엔진 수정, 디버깅, 플러그인 빌드 등 별도 필요가 생길 때만 재검토한다.

---

## 2. 전환 당시 프로젝트 정보 (Historical)

| 항목 | 값 |
|---|---|
| 프로젝트 이름 | CarFight |
| GitHub | https://github.com/serenieal/CarFight.git |
| 로컬 프로젝트 경로 | `D:\Work\CarFight_git` |
| 언리얼 프로젝트 파일 | `D:\Work\CarFight_git\UE\CarFight_Re.uproject` |
| 현재 엔진 | `D:\UE_5.7` |
| 현재 엔진 유형 | Launcher 기반 Installed Build |
| 전환 목표 엔진 | UE 5.7 Source Build |
| 추천 Source Build 경로 | `D:\UE_5.7_Source` |

---

## 3. 작업 범위

### 포함

- UE 5.7 Source Build 준비
- Epic Games 계정과 GitHub 계정 연결 확인
- UnrealEngine 소스 접근 확인
- Source Build 엔진 설치 경로 결정
- `Setup.bat` 실행
- `GenerateProjectFiles.bat` 실행
- `UE5.sln`에서 UE5 Development Editor Win64 빌드
- CarFight `.uproject` EngineAssociation 전환
- `CarFight_ReEditor` 빌드 검증
- `CarFight_ReServer` 빌드 검증: Deprecated / 현재 제외
- Dedicated Server 실행 검증: Deprecated / 현재 제외
- 실패 시 롤백 방법 정리

### 제외

- C++ 게임 코드 작성
- Target.cs 신규 작성
- GameMode 작성
- PlayerController 작성
- 무기, 체력, 대미지, 스코어 구현
- 리스폰, 로비, 매치메이킹 구현
- 기존 멀티플레이 안정화 계획 변경
- MP_ServerPlan 내용과 혼합

---

## 4. 문서 구성

| 문서 | 역할 |
|---|---|
| `README.md` | 전체 목적, 범위, 문서 인덱스 |
| `SourceBuildPlan.md` | Source Build 전환 전체 계획 |
| `EnvCheck.md` | 사전 환경 점검 체크리스트 |
| `SwitchSteps.md` | 실제 엔진 전환 절차 |
| `BuildVerify.md` | Source Build Editor 중심 검증 절차 / 서버 검증 Deprecated |

---

## 5. 진행 원칙

1. 기존 `D:\UE_5.7` 런처 엔진은 즉시 삭제하지 않는다.
2. Source Build 전환 검증이 완료될 때까지 기존 엔진은 롤백용으로 유지한다.
3. `.uproject` 전환 전 Git 상태를 반드시 깨끗하게 만든다.
4. `.uproject`의 기존 EngineAssociation 값을 기록한다.
5. Source Build 전환 중 Target.cs 파일이 없다는 문제가 발견되어도 이 문서 작업 범위에서는 새 Target.cs를 만들지 않는다.
6. `CarFight_ReServer` Target 검증은 현재 싱글플레이 기준의 필수 작업이 아니다.
7. Target이 없거나 게임 코드 수정이 필요한 경우, 서버 관련 작업을 새로 만들지 않고 Deprecated로 기록한다.

---

## 6. 완료 판정

이 전환 작업은 다음 조건을 모두 만족하면 완료로 본다.

- UE 5.7 Source Build 엔진이 `Development Editor / Win64`로 빌드된다.
- Source Build 엔진으로 CarFight 프로젝트가 열린다.
- `CarFight_ReEditor Win64 Development` 빌드가 성공한다.
- PIE Single Player 또는 Standalone Game에서 프로젝트가 실행된다.

---

## 7. 서버 작업 복귀 조건

서버 작업은 현재 복귀 대상이 아니다.

다시 진행하려면 먼저 아래 결정을 확정한다.

- 프로젝트가 다시 멀티플레이 또는 온라인 모드를 공식 목표로 채택한다.
- Dedicated Server Target이 현재 코드 기준으로 필요하다.
- Source Build가 서버 빌드 문제 해결에 필요한 유일한 경로다.
- 서버 관련 실패 로그가 별도 이슈로 정리되어 있다.

---

## 8. 공식 참고 자료

- Epic Developer Community: GitHub에서 언리얼 엔진 소스 코드 다운로드하기  
  https://dev.epicgames.com/documentation/unreal-engine/downloading-source-code-in-unreal-engine?lang=ko
- Epic Developer Community: Building Unreal Engine from Source  
  https://dev.epicgames.com/documentation/unreal-engine/building-unreal-engine-from-source
- Epic Developer Community: Setting Up Dedicated Servers in Unreal Engine  
  https://dev.epicgames.com/documentation/unreal-engine/setting-up-dedicated-servers-in-unreal-engine
- Epic Developer Community: How to Generate Unreal Engine Project Files for Your IDE  
  https://dev.epicgames.com/documentation/unreal-engine/how-to-generate-unreal-engine-project-files-for-your-ide
- Epic Developer Community: Managing Game Code in Unreal Engine  
  https://dev.epicgames.com/documentation/unreal-engine/managing-game-code-in-unreal-engine
- Epic Developer Community: Unreal Engine Build Tool Target Reference  
  https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-build-tool-target-reference

---

## ChangeLog

- v0.3 / 2026-08-08
  - UE 5.7 Source Build 전환 Plan을 종료하고 `Archived / Superseded / Historical`로 전환했다.
  - 현재 공식 엔진이 UE 5.8 Source Build / `D:\UnrealEngine_Source`임을 종료 기준으로 명시했다.
  - 기존 UE 5.7 경로와 명령은 전환 당시 Historical evidence로만 유지하고 현재 실행 기준에서 제외했다.
  - Dedicated Server 관련 잔여 항목은 이 Plan의 미완료 작업이 아니라 별도 Deferred 네트워크 범위로 해석한다.

- v0.2 / 2026-06-19
  - Dedicated Server 목적의 Source Build 전환을 현재 필수 작업에서 제외했다.
  - 완료 판정을 Editor / 싱글플레이 실행 검증 중심으로 변경했다.
  - 서버 작업 복귀 조건을 명시했다.

## 마이그레이션 지침

- 이 폴더를 Active/Ready/On Hold Plan으로 복원하지 않는다.
- 현재 엔진 기준은 `Document/ProjectSSOT/01_ProjectState.md`와 CarFight 루트/UE `AGENTS.md`의 **UE 5.8 Source Build / `D:\UnrealEngine_Source`**를 따른다.
- 이 폴더의 `D:\UE_5.7`, `D:\UE_5.7_Source` 명령은 실행하지 않고 Historical 기록으로만 읽는다.
- Dedicated Server 관련 요구가 다시 필요해지면 이 Plan을 재개하지 않고 현재 ProjectSSOT/FeatureQueue를 거쳐 별도 최신 Plan을 작성한다.
