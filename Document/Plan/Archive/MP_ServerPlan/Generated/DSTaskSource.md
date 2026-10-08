# Task Source

- 문서 버전: v1.0
- 작성일: 2026-05-08
- 문서 상태: Generated
- 작업 유형: 신규
- source_folder_path: `Document/ProjectSSOT/Plan/MP_ServerPlan`

## Goal

- TBD

## Current Decisions

- TBD

## In Scope

- 아래 기능은 실제 프로젝트에 이미 구현되어 있는 경우에만 현재 안정화 대상으로 삼는다.
- ```text
- 차량 이동
- 차량 입력
- 차량 소유권
- 차량 카메라
- HUD / UI
- 부스터
- 드리프트
- 차량 충돌
- 차량 스폰
- 차량 선택
- 사운드
- 이펙트
- 맵 오브젝트
- 디버그 기능
- ```
- 중요:
- ```text 목록에 있더라도 실제 프로젝트에 구현되어 있지 않으면 작업하지 않는다. ```
- ---
- 아래 기능은 실제 프로젝트에 이미 구현되어 있는 경우에만 현재 안정화 대상으로 삼는다.

```text
- 차량 이동
- 차량 입력
- 차량 소유권
- 차량 카메라
- HUD / UI
- 부스터
- 드리프트
- 차량 충돌
- 차량 스폰
- 차량 선택
- 사운드
- 이펙트
- 맵 오브젝트
- 디버그 기능
```

중요:

```text
목록에 있더라도 실제 프로젝트에 구현되어 있지 않으면 작업하지 않는다.
```

---
- Dedicated Server 실행 확인
- 클라이언트 2개 접속 확인
- 플레이어 스폰 확인
- 차량 Pawn 소유권 확인
- 차량 입력 분리 확인
- 차량 이동 복제 확인
- Dedicated Server 로그 확인
- 클라이언트 로그 확인
- 체력
- 대미지
- 스코어
- 킬/데스
- 리스폰
- 무기 시스템
- 로비
- 매치메이킹
- Steam 세션
- 서버 배포 자동화
- ### 2.1 포함

```text
- Dedicated Server 실행 확인
- 클라이언트 2개 접속 확인
- 플레이어 스폰 확인
- 차량 Pawn 소유권 확인
- 차량 입력 분리 확인
- 차량 이동 복제 확인
- Dedicated Server 로그 확인
- 클라이언트 로그 확인
```

### 2.2 제외

```text
- 체력
- 대미지
- 스코어
- 킬/데스
- 리스폰
- 무기 시스템
- 로비
- 매치메이킹
- Steam 세션
- 서버 배포 자동화
```

---
- 실제 프로젝트에 이미 구현되어 있는 경우에만 아래 기능을 점검한다.
- ```text 위 목록에 있더라도 실제 프로젝트에 아직 구현되어 있지 않으면 작업하지 않는다. ```
- 실제 프로젝트에 이미 구현되어 있는 경우에만 아래 기능을 점검한다.

```text
- 차량 이동
- 차량 입력
- 차량 소유권
- 차량 카메라
- HUD / UI
- 부스터
- 드리프트
- 차량 충돌
- 차량 스폰
- 차량 선택
- 사운드
- 이펙트
- 맵 오브젝트
- 디버그 기능
```

중요:

```text
위 목록에 있더라도 실제 프로젝트에 아직 구현되어 있지 않으면 작업하지 않는다.
```

---

## Out of Scope

- 현재 작업에서 제외하는 항목은 다음과 같다.
- ```text
- 무기 발사
- 투사체
- 탄약
- 장전
- 체력
- 대미지
- 사망
- 킬/데스
- 스코어
- 리스폰
- 매치 승패
- 로비 / 세션 / 매치메이킹
- ```
- 이 항목들은 문서에 후보로 언급될 수는 있지만, 현재 작업 완료 기준에는 포함하지 않는다.
- ---
- 현재 작업에서 제외하는 항목은 다음과 같다.

```text
- 무기 발사
- 투사체
- 탄약
- 장전
- 체력
- 대미지
- 사망
- 킬/데스
- 스코어
- 리스폰
- 매치 승패
- 로비 / 세션 / 매치메이킹
```

이 항목들은 문서에 후보로 언급될 수는 있지만, 현재 작업 완료 기준에는 포함하지 않는다.

---
- 아래 기능은 현재 작업에서 제외한다.
- 무기 시스템
- 체력 시스템
- 대미지 시스템
- 스코어 시스템
- 킬/데스 시스템
- 리스폰 시스템
- 킬 로그
- 스코어보드
- 로비
- Steam 세션
- 매치메이킹
- 서버 브라우저
- 서버 배포 자동화
- 이 기능들은 삭제된 것이 아니다.
- 현재 안정화 단계에서 제외하고, 이후 확장 단계에서 다시 계획한다.
- 아래 기능은 현재 작업에서 제외한다.

```text
- 무기 시스템
- 체력 시스템
- 대미지 시스템
- 스코어 시스템
- 킬/데스 시스템
- 리스폰 시스템
- 킬 로그
- 스코어보드
- 매치 승패
- 로비
- Steam 세션
- 매치메이킹
- 서버 브라우저
- 서버 배포 자동화
```

이 기능들은 삭제된 것이 아니다.

현재 안정화 단계에서 제외하고, 이후 확장 단계에서 다시 계획한다.

---

## Target Files

- Document/ProjectSSOT/Plan/MP_ServerPlan/MPPlan.md
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
- UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
- UE/Source/CarFight_Re
- UE/Content/CarFight
- UE/Content/Maps/TestMap.umap
- UE/Config/DefaultEngine.ini
- UE/Config/DefaultInput.ini
- UE/Source/CarFight_Re.Target.cs
- UE/Source/CarFight_ReEditor.Target.cs
- UE/Source/CarFight_ReServer.Target.cs
- UE/Source/CarFight_Re/CarFight_Re.Build.cs
- UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
- UE/Source/CarFight_Re/Private/UI/CFVehicleDebugSectionWidget.cpp
- Document/ProjectSSOT/Plan/MP_ServerPlan

## Task Candidates

### P1-T1. Document/ProjectSSOT/Plan/MP_ServerPlan/MPPlan.md 변경 필요성 확인 및 구현

목표:
- Document/ProjectSSOT/Plan/MP_ServerPlan/MPPlan.md 변경 필요성 확인 및 구현

대상:
- Document/ProjectSSOT/Plan/MP_ServerPlan/MPPlan.md

Acceptance:
- 'Document/ProjectSSOT/Plan/MP_ServerPlan/MPPlan.md 변경 필요성 확인 및 구현' 작업 범위가 지정된 target files 안에서 완료된다.

Verification:
- TBD

### P1-T2. UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp 변경 필요성 확인 및 구현

목표:
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp 변경 필요성 확인 및 구현

대상:
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp

Acceptance:
- 'UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp 변경 필요성 확인 및 구현' 작업 범위가 지정된 target files 안에서 완료된다.

Verification:
- TBD

### P1-T3. UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp 변경 필요성 확인 및 구현

목표:
- UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp 변경 필요성 확인 및 구현

대상:
- UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp

Acceptance:
- 'UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp 변경 필요성 확인 및 구현' 작업 범위가 지정된 target files 안에서 완료된다.

Verification:
- TBD

### P1-T4. UE/Source/CarFight_Re 변경 필요성 확인 및 구현

목표:
- UE/Source/CarFight_Re 변경 필요성 확인 및 구현

대상:
- UE/Source/CarFight_Re

Acceptance:
- 'UE/Source/CarFight_Re 변경 필요성 확인 및 구현' 작업 범위가 지정된 target files 안에서 완료된다.

Verification:
- TBD

### P1-T5. UE/Content/CarFight 변경 필요성 확인 및 구현

목표:
- UE/Content/CarFight 변경 필요성 확인 및 구현

대상:
- UE/Content/CarFight

Acceptance:
- 'UE/Content/CarFight 변경 필요성 확인 및 구현' 작업 범위가 지정된 target files 안에서 완료된다.

Verification:
- TBD

### P1-T6. UE/Content/Maps/TestMap.umap 변경 필요성 확인 및 구현

목표:
- UE/Content/Maps/TestMap.umap 변경 필요성 확인 및 구현

대상:
- UE/Content/Maps/TestMap.umap

Acceptance:
- 'UE/Content/Maps/TestMap.umap 변경 필요성 확인 및 구현' 작업 범위가 지정된 target files 안에서 완료된다.

Verification:
- TBD

### P1-T7. 아래 기능은 실제 프로젝트에 이미 구현되어 있는 경우에만 현재 안정화 대상으로 삼는다.

목표:
- 아래 기능은 실제 프로젝트에 이미 구현되어 있는 경우에만 현재 안정화 대상으로 삼는다.

대상:
- Document/ProjectSSOT/Plan/MP_ServerPlan/MPPlan.md
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
- UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
- UE/Source/CarFight_Re
- UE/Content/CarFight
- UE/Content/Maps/TestMap.umap
- UE/Config/DefaultEngine.ini
- UE/Config/DefaultInput.ini

Acceptance:
- '아래 기능은 실제 프로젝트에 이미 구현되어 있는 경우에만 현재 안정화 대상으로 삼는다.' 작업 범위가 지정된 target files 안에서 완료된다.

Verification:
- TBD

### P1-T8. ```text

목표:
- ```text

대상:
- Document/ProjectSSOT/Plan/MP_ServerPlan/MPPlan.md
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
- UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
- UE/Source/CarFight_Re
- UE/Content/CarFight
- UE/Content/Maps/TestMap.umap
- UE/Config/DefaultEngine.ini
- UE/Config/DefaultInput.ini

Acceptance:
- '```text' 작업 범위가 지정된 target files 안에서 완료된다.

Verification:
- TBD

## Acceptance Criteria

- 대상 파일 또는 operation 변경이 TaskSource의 목표와 범위를 만족한다.

## Verification

- TBD

## Unresolved

- issue 문서 확인 필요: Document/ProjectSSOT/Plan/MP_ServerPlan/RiskSearch.md

## Source Inventory

- `Document/ProjectSSOT/Plan/MP_ServerPlan/PhaseTasks.md` role=task status=active priority=high signals=none
- `Document/ProjectSSOT/Plan/MP_ServerPlan/MPPlan.md` role=plan status=active priority=medium signals=scope, out_of_scope
- `Document/ProjectSSOT/Plan/MP_ServerPlan/RiskSearch.md` role=issue status=active priority=medium signals=none
- `Document/ProjectSSOT/Plan/MP_ServerPlan/DSMinRun.md` role=unknown status=active priority=low signals=scope
- `Document/ProjectSSOT/Plan/MP_ServerPlan/FeatureList.md` role=unknown status=active priority=low signals=none
- `Document/ProjectSSOT/Plan/MP_ServerPlan/NetAudit.md` role=unknown status=active priority=low signals=none
- `Document/ProjectSSOT/Plan/MP_ServerPlan/README.md` role=unknown status=active priority=low signals=none
- `Document/ProjectSSOT/Plan/MP_ServerPlan/Scope.md` role=unknown status=active priority=low signals=scope, out_of_scope

## Changelog

- v1.0
- plan.synthesize_task_source로 생성
