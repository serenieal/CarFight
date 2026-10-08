# Task Source

- 문서 버전: v1.0
- 작성일: 2026-05-08
- 문서 상태: Generated
- 작업 유형: 신규
- artifact_role: intermediate
- codex_input: false
- source_folder_path: `Document/Plan/ServerUpgradePlan`

## Goal

- 이 문서는 서버 고도화 작업 중 확정한 결정을 기록한다.
- 결정 로그의 목적은 같은 논의를 반복하지 않고, 왜 특정 구조를 선택했는지 남기는 것이다.
- 이 문서는 서버 고도화 로드맵을 실제 작업 단위로 쪼갠 체크리스트다.
- 작업할 때는 이 문서에서 하나의 작업을 고르고, 완료 후 결과를 기록한다.
- 이 문서는 서버 고도화 작업을 단계별로 나누고, 각 단계의 완료 상태를 고정한다.
- 초기 최종 목표는 다음이었다.
- ```text Dedicated Server에서 클라이언트 2개가 접속하고, 각자 자기 차량을 소유하고, 자기 입력으로 자기 차량만 조작하며, 상대 차량의 이동을 볼 수 있는 상태. ```
- 현재 이 목표는 통과했다.
- 서버 기반이 생긴 뒤 다음 gameplay 시스템을 하나씩 계획한다.
- 권장 후보 순서:
- 무기/발사 요청 서버 권한 구조 설계
- 최소 발사 RPC / 서버 검증 / 클라이언트 시각 피드백 분리
- 체력/대미지 구조 설계
- 리스폰 구조 설계
- 스코어/킬/데스
- 로비/세션
- 이 문서는 CarFight의 현재 차량 클라이언트 기능을 Dedicated Server에서 검증 가능하게 만들기 위한 최소 서버 구조를 설계한다.
- 현재 설계의 핵심은 다음이다.
- ```text 서버가 차량을 Spawn하고, PlayerController가 그 차량을 Possess한다. ```
- 이 문서는 CarFight 차량 네트워크 물리 흔들림 문제를 해결하기 위한 중기 구조를 설계한다.
- 이번 문서는 코드 작업이 아니라 설계 문서다.
- 핵심 방향:
- ```text UE Actor Movement Replication에만 의존하지 않는다. 서버 권위 차량 상태를 별도 NetState로 복제한다. 원격 차량은 서버 상태 버퍼를 기준으로 보간 표시한다. 소유 차량 예측/서버 보정은 원격 차량 보간 이후 단계로 미룬다. ```
- 이 문서는 `ACFMPGameMode` 기반 Dedicated Server 테스트 결과를 기록한다.
- 이번 기록은 사용자 보고 기준으로 작성한다.
- 이 문서는 서버 고도화 작업의 검증 절차를 정의한다.
- 목표는 기능을 만들었다고 끝내는 것이 아니라, Dedicated Server에서 실제로 다음 조건을 확인하는 것이다.
- 클라이언트가 접속한다.
- 서버가 차량을 생성한다.
- PlayerController가 차량을 Possess한다.
- 각 클라이언트가 자기 차량만 조작한다.
- 상대 차량 이동이 보인다.
- 서버 로그에 로컬 전용 코드 오류가 없다.
- 서버가 맵을 정상 로드하고 대기 상태에 들어가는지 확인한다.
- 클라이언트 1명이 서버에 접속하고 자기 차량을 소유하는지 확인한다.
- 클라이언트 2명이 각각 다른 차량을 소유하는지 확인한다.
- 각 클라이언트 입력이 자기 차량에만 적용되는지 확인한다.
- 상대 차량 이동이 각 클라이언트 화면에서 보이는지 확인한다.
- Dedicated Server에서 로컬 전용 코드가 실행되어 오류를 만들지 않는지 확인한다.
- 이 문서는 `ACFMPGameMode` 연결 이후 실제 Dedicated Server 접속 테스트 전에 필요한 Blueprint와 Map 상태를 확정한다.
- 검사 대상은 다음이다.
- /Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn
- /Game/Maps/TestMap.TestMap
- 이 문서는 서버 고도화 첫 코드 작업에 들어가기 전에 현재 상태를 확정하기 위한 감사 결과다.
- 목표는 다음 두 가지다.
- 현재 서버/클라이언트 상태를 추정이 아니라 확인 결과로 고정한다.
- 첫 코드 작업이 왜 ACFMPGameMode인지 근거를 남긴다.
- 이 문서는 CarFight의 서버/클라이언트 현재 상태를 서버 1차 안정화 완료 기준으로 갱신한다.
- 이전 v0.1.0 문서는 작업 전 병목을 정리했다.
- 현재 v0.2.0 문서는 다음 사실을 기준으로 한다.
- ```text Dedicated Server 기반 2클라 차량 Spawn/Possess, 입력 분리, 기본 이동 복제 검증 통과 ```
- 이 문서는 CarFight Dedicated Server 1차 안정화 이후, 서버에서 실행되면 안 되는 로컬 전용 코드 경로를 점검한 결과다.
- 점검 대상은 다음이다.
- Viewport UI 생성
- LocalPlayer 접근
- Camera Tick / Camera Transform 갱신
- Debug HUD / Debug Panel
- Sound 호출
- FX / Niagara / Emitter 호출
- DrawDebug 호출
- 이 폴더는 CarFight의 서버 상태를 현재 클라이언트 개발 수준에 맞춰 끌어올리기 위한 계획, 설계, 단계별 로드맵을 관리한다.
- 현재 서버는 Dedicated Server 빌드와 실행은 가능하지만, 아직 게임 서버로서 필요한 Spawn, Possess, OwnerOnly 입력, 이동 복제, 서버 전용 실행 안정화가 완료되지 않았다.
- 이 폴더의 목표는 다음 하나다.
- ```text 현재 구현된 클라이언트 차량 기능을 Dedicated Server 기준에서 안정적으로 동작하게 만든다. ```
- 이 문서는 `CFVNetDbgFix` 적용 후 키보드 입력으로 다시 수집한 차량 네트워크 물리 로그의 해석 결과를 기록한다.
- 이전 게임패드 테스트에서는 하나의 물리 게임패드 입력이 Client 1 / Client 2 양쪽 프로세스에 동시에 읽힐 수 있었으므로, 이번 테스트는 키보드 입력으로 입력 오염을 줄인 상태에서 진행했다.
- 이 문서는 CarFight 서버 고도화 작업에서 지금 할 일과 하지 않을 일을 명확히 고정한다.
- 현재 목표는 멀티플레이 전체 완성이 아니다.
- 현재 목표는 다음이다.
- ```text 현재 클라이언트에 이미 구현된 차량 플레이 기능을 Dedicated Server 환경에서 안정적으로 실행, 소유, 조작, 관찰할 수 있게 만든다. ```
- 이 문서는 CarFight 서버 고도화 1차 작업의 종료 상태를 정리한다.
- 이번 단계의 목표는 다음이었다.
- ```text 현재 구현된 클라이언트 차량 기능을 Dedicated Server 기준에서 안정적으로 실행, 소유, 조작, 관찰할 수 있게 만든다. ```
- 이 문서는 Dedicated Server 2클라 차량 테스트 중 발견된 차량 흔들림, 원격 차량 순간이동, 장기 위치 불일치 문제를 정리한다.
- 서버 1차 안정화는 통과했지만, 차량 네트워크 물리 품질 문제가 확인되었다.

## Current Decisions

- TBD

## In Scope

- 이번 단계에서 포함하는 작업은 아래와 같다.
- Dedicated Server 실행 기준 유지
- 클라이언트 2개 접속 검증
- 서버 권한 Player Spawn / Vehicle Spawn / Possess 구조 작성
- 각 클라이언트의 자기 차량 소유권 확인
- OwnerOnly 입력 분리 확인
- 차량 이동 복제 확인
- Dedicated Server에서 UI, 카메라, 로컬 사운드 코드가 실행되지 않도록 정리
- 현재 구현된 Aim / Fire 요청 골격의 서버 권한 경계 확인
- TestMap을 2클라 검증 가능한 최소 구조로 정리
- 서버 로그 / 클라이언트 로그 기준 검증 절차 문서화

## Out of Scope

- 아래 기능은 이번 단계에서 새로 만들지 않는다.
- 무기 시스템 완성
- 체력 시스템
- 대미지 시스템
- 스코어 시스템
- 킬/데스 시스템
- 리스폰 시스템
- 킬 로그
- 스코어보드
- 로비
- 세션 브라우저
- Steam 세션
- 매치메이킹
- 서버 배포 자동화
- 운영 서버 인프라 구성

## Target Files

- UE/Source/CarFight_Re/Public/CFMPGameMode.h
- UE/Source/CarFight_Re/Private/CFMPGameMode.cpp
- UE/Source/CarFight_Re/Public/CFVehiclePawn.h
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
- UE/Config/DefaultEngine.ini
- UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h
- UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp
- UE/Source/CarFight_ReServer.Target.cs

## Task Candidates

### P1-T1. 문서/상태 정렬

목표:
- 문서/상태 정렬

대상:
- UE/Source/CarFight_Re/Public/CFMPGameMode.h
- UE/Source/CarFight_Re/Private/CFMPGameMode.cpp
- UE/Source/CarFight_Re/Public/CFVehiclePawn.h
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
- UE/Config/DefaultEngine.ini
- UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h
- UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp
- UE/Source/CarFight_ReServer.Target.cs

구현 내용:
- 대상 프로젝트: CarFight
- 상위 문서: README.md
- 확정일: 2026-06-01

Acceptance:
- 대상 프로젝트: CarFight
- 상위 문서: `README.md`
- 확정일: 2026-06-01
- 상위 문서: `Roadmap.md`
- [x] `ServerUpgradePlan` 폴더 생성

Verification:
- dotnet build UE/CarFight_Re.sln

## Acceptance Criteria

- 검증 항목이 통과한다: dotnet build UE/CarFight_Re.sln

## Verification

- dotnet build UE/CarFight_Re.sln

## Unresolved

- 없음

## Context Quality Policy

- 폴더 문서는 출력량을 늘리기 위한 재료가 아니라 방향 오염을 줄이기 위한 검증 근거로 사용한다.
- 서로 충돌하는 문맥은 최종 작업지시서에 섞지 말고 unresolved 또는 blocked로 남긴다.
- 현재 승인 범위 밖 후보는 실행 계약으로 만들지 않고 suggested_next로 분리한다.
- 최종 Codex 입력은 source coverage와 quality gate를 통과한 compact contract만 사용한다.

## Source Coverage

- source_folder: `Document/Plan/ServerUpgradePlan`
- included:
  - `Document/Plan/ServerUpgradePlan/DecisionLog.md`
  - `Document/Plan/ServerUpgradePlan/TaskList.md`
  - `Document/Plan/ServerUpgradePlan/Roadmap.md`
  - `Document/Plan/ServerUpgradePlan/ServerDesign.md`
  - `Document/Plan/ServerUpgradePlan/VehicleNetStatePlan.md`
  - `Document/Plan/ServerUpgradePlan/CodexTasks/CFCamDSTick_Review.md`
  - `Document/Plan/ServerUpgradePlan/CodexTasks/CFMPGameMode_Review.md`
  - `Document/Plan/ServerUpgradePlan/CodexTasks/CFMPGameModeConnect_Review.md`
  - `Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetDbg_Review.md`
  - `Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetDbgFix_Review.md`
  - `Document/Plan/ServerUpgradePlan/RuntimeTest_20260601.md`
  - `Document/Plan/ServerUpgradePlan/RuntimeTest_20260605_NetPhysics.md`
  - `Document/Plan/ServerUpgradePlan/TestPlan.md`
  - `Document/Plan/ServerUpgradePlan/AssetMapAudit.md`
  - `Document/Plan/ServerUpgradePlan/AuditResult.md`
  - `Document/Plan/ServerUpgradePlan/CurrentState.md`
  - `Document/Plan/ServerUpgradePlan/DedicatedServerCodeAudit.md`
  - `Document/Plan/ServerUpgradePlan/README.md`
  - `Document/Plan/ServerUpgradePlan/RT_20260605_VNetKey.md`
  - `Document/Plan/ServerUpgradePlan/Scope.md`
  - `Document/Plan/ServerUpgradePlan/ServerStabilizationCloseout.md`
  - `Document/Plan/ServerUpgradePlan/VehicleNetPhysicsJitter.md`
- excluded:
  - path: `Document/Plan/ServerUpgradePlan/CFVNetDbg_TaskSource.md` reason: generated_filename
- included_roles:
  - decision
  - design
  - plan
  - roadmap
  - task
  - unknown
  - validation
- included_signals:
  - goal
  - target_files
  - scope
  - out_of_scope
- missing_roles:
  - 없음

## Source Inventory

- `Document/Plan/ServerUpgradePlan/DecisionLog.md` role=decision status=active priority=high signals=goal
- `Document/Plan/ServerUpgradePlan/TaskList.md` role=task status=active priority=high signals=goal, target_files
- `Document/Plan/ServerUpgradePlan/Roadmap.md` role=roadmap status=active priority=high signals=goal
- `Document/Plan/ServerUpgradePlan/ServerDesign.md` role=design status=active priority=medium signals=goal
- `Document/Plan/ServerUpgradePlan/VehicleNetStatePlan.md` role=plan status=active priority=medium signals=goal
- `Document/Plan/ServerUpgradePlan/CodexTasks/CFCamDSTick_Review.md` role=validation status=active priority=medium signals=none
- `Document/Plan/ServerUpgradePlan/CodexTasks/CFMPGameMode_Review.md` role=validation status=active priority=medium signals=none
- `Document/Plan/ServerUpgradePlan/CodexTasks/CFMPGameModeConnect_Review.md` role=validation status=active priority=medium signals=none
- `Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetDbg_Review.md` role=validation status=active priority=medium signals=target_files
- `Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetDbgFix_Review.md` role=validation status=active priority=medium signals=none
- `Document/Plan/ServerUpgradePlan/RuntimeTest_20260601.md` role=validation status=active priority=medium signals=goal
- `Document/Plan/ServerUpgradePlan/RuntimeTest_20260605_NetPhysics.md` role=validation status=active priority=medium signals=none
- `Document/Plan/ServerUpgradePlan/TestPlan.md` role=validation status=active priority=medium signals=goal
- `Document/Plan/ServerUpgradePlan/AssetMapAudit.md` role=unknown status=active priority=low signals=goal
- `Document/Plan/ServerUpgradePlan/AuditResult.md` role=unknown status=active priority=low signals=goal
- `Document/Plan/ServerUpgradePlan/CurrentState.md` role=unknown status=active priority=low signals=goal
- `Document/Plan/ServerUpgradePlan/DedicatedServerCodeAudit.md` role=unknown status=active priority=low signals=goal
- `Document/Plan/ServerUpgradePlan/README.md` role=unknown status=active priority=low signals=goal
- `Document/Plan/ServerUpgradePlan/RT_20260605_VNetKey.md` role=unknown status=active priority=low signals=goal
- `Document/Plan/ServerUpgradePlan/Scope.md` role=unknown status=active priority=low signals=goal, scope, out_of_scope
- `Document/Plan/ServerUpgradePlan/ServerStabilizationCloseout.md` role=unknown status=active priority=low signals=goal
- `Document/Plan/ServerUpgradePlan/VehicleNetPhysicsJitter.md` role=unknown status=active priority=low signals=goal
- `Document/Plan/ServerUpgradePlan/CFVNetDbg_TaskSource.md` role=task status=generated priority=low signals=goal, scope, out_of_scope, constraint, target_files, generated exclude_reason=generated_filename

## Changelog

- v1.2
- Context Quality Policy 추가
- artifact_role/codex_input/source_coverage 메타데이터 추가
- plan.synthesize_task_source로 생성
