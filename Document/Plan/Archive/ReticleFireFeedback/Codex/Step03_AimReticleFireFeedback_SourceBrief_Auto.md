# Task Source

- 문서 버전: v1.0
- 작성일: 2026-05-08
- 문서 상태: Generated
- 작업 유형: 신규
- artifact_role: intermediate
- codex_input: false
- source_folder_path: `Document/Plan/ReticleFireFeedback`

## Goal

- 이번 구현의 목표는 아래다.
- WeaponFire가 기록한 로컬 발사 결과를 Reticle UI에서 읽을 수 있게 한다.
- 발사 성공 / 발사 불가 / 쿨다운 / 무기 없음 / 조준 막힘을 구분한다.
- OutOfArcWarning은 단독 발사 차단이 아니라 보조 경고로 표시한다.
- BP는 표시만 담당하고, 상태 계산은 C++에서 처리한다.
- 기존 AimReticle 구조를 크게 흔들지 않는다.
- P0에서 가장 중요한 플레이어 경험은 아래다.
- ```text 플레이어가 Fire 입력을 눌렀을 때, 쐈는지 / 왜 못 쐈는지 / 아직 쿨다운인지 즉시 알 수 있어야 한다. ```

## Current Decisions

- TBD

## In Scope

- TBD

## Out of Scope

- TBD

## Target Files

- UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h
- UE/Source/CarFight_Re/Public/CFVehiclePawn.h
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
- UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
- UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp
- UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
- UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
- UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h

## Task Candidates

### P1-T1. WBP 최소 구현

목표:
- WBP 최소 구현

대상:
- UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h
- UE/Source/CarFight_Re/Public/CFVehiclePawn.h
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
- UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
- UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp
- UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
- UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
- UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h

구현 내용:
- Version: 0.1.1
- Date: 2026-07-09
- Status: Draft / Pre-Implementation Design

Acceptance:
- Version: 0.1.1
- Date: 2026-07-09
- Status: Draft / Pre-Implementation Design
- Scope: Reticle / FireFeedback 구현 전 C++ / BP 분담, 파일 단위 변경 계획, 타입/함수/변수 설계, WBP 연결 기준
- WeaponFire가 기록한 로컬 발사 결과를 Reticle UI에서 읽을 수 있게 한다.

Verification:
- TBD

## Acceptance Criteria

- 대상 파일 또는 operation 변경이 TaskSource의 목표와 범위를 만족한다.

## Verification

- TBD

## Unresolved

- 없음

## Context Quality Policy

- 폴더 문서는 출력량을 늘리기 위한 재료가 아니라 방향 오염을 줄이기 위한 검증 근거로 사용한다.
- 서로 충돌하는 문맥은 최종 작업지시서에 섞지 말고 unresolved 또는 blocked로 남긴다.
- 현재 승인 범위 밖 후보는 실행 계약으로 만들지 않고 suggested_next로 분리한다.
- 최종 Codex 입력은 source coverage와 quality gate를 통과한 compact contract만 사용한다.

## Source Coverage

- source_folder: `Document/Plan/ReticleFireFeedback`
- included:
  - `Document/Plan/ReticleFireFeedback/ImplementationDesign.md`
  - `Document/Plan/ReticleFireFeedback/VisualGuide.md`
- excluded:
  - 없음
- included_roles:
  - design
  - unknown
- included_signals:
  - goal
  - target_files
- missing_roles:
  - plan
  - roadmap
  - validation

## Source Inventory

- `Document/Plan/ReticleFireFeedback/ImplementationDesign.md` role=design status=active priority=medium signals=goal, target_files
- `Document/Plan/ReticleFireFeedback/VisualGuide.md` role=unknown status=active priority=low signals=none

## Changelog

- v1.2
- Context Quality Policy 추가
- artifact_role/codex_input/source_coverage 메타데이터 추가
- plan.synthesize_task_source로 생성
