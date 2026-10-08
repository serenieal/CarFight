# Codex TaskSource Step01 FireFeedbackTypes

Version: 0.1.1
Date: 2026-07-09
Task ID: ReticleFireFeedback-Step01-FireFeedbackTypes
Status: Ready for plan.build_codex_task

## Goal

Create the first C++ type definition file for CarFight Reticle / FireFeedback implementation.

Create only this new file:

```text
UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h
```

The new file defines Blueprint-visible data types that Reticle / FireFeedback UI will read later.

## In Scope

- Create `UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h`.
- Add `ECFVehicleFireFeedbackState` as `UENUM(BlueprintType)`.
- Add `FCFVehicleFireFeedbackViewData` as `USTRUCT(BlueprintType)`.
- Include `CoreMinimal.h`.
- Include `CFVehicleAimTypes.h` because `ECFVehicleFireRejectReason` is used.
- Include `CFVehicleFireFeedbackTypes.generated.h` last.
- Keep this task as a type-definition-only change.

## Out of Scope

- Do not modify `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`.
- Do not modify `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`.
- Do not modify `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h`.
- Do not modify `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`.
- Do not modify `UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h`.
- Do not modify `UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp`.
- Do not modify any WBP or Blueprint asset.
- Do not modify any document file.
- Do not add server, RPC, replication, or multiplayer code.
- Do not create a FireFeedback component class.
- Do not add a new cooldown getter to `UCFVehicleWeaponComp`.

## Constraints

- This task is Step 1 only.
- The only write target is `UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h`.
- `CFVehicleFireFeedbackTypes.generated.h` must be the last include.
- All fields in `FCFVehicleFireFeedbackViewData` must be `BlueprintReadOnly`.
- Every UENUM, USTRUCT, and UPROPERTY block must have a one-line Korean summary comment above it.
- This task must not introduce any UObject class or `.cpp` file.
- This task must not interpret `FirePending` as server wait.
- This task must not interpret `FireRejected` as server rejection.
- `OutOfArcWarning` must mean an aim-angle warning, not a hard fire block.

## Target Files

Write target:

```text
UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h
```

Read-only references:

```text
Document/Plan/ReticleFireFeedback/ImplementationDesign.md
Document/Plan/ReticleFireFeedback/VisualGuide.md
Document/Systems/Combat/FireFeedback.md
Document/Systems/UI/AimReticle.md
Document/Systems/Combat/WeaponFire.md
Document/Systems/Vehicles/VehicleAim.md
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
```

## Required Implementation

The new file must begin with:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "CFVehicleAimTypes.h"
#include "CFVehicleFireFeedbackTypes.generated.h"
```

Add this enum:

```cpp
// [v0.1.1] Reticle / UI에 표시할 로컬 발사 피드백 상태입니다.
UENUM(BlueprintType)
enum class ECFVehicleFireFeedbackState : uint8
{
	None,

	FireSuccess,

	FirePending,

	FireRejected,

	Cooldown,

	NoWeapon,

	AimBlocked,

	OutOfArcWarning
};
```

Add this struct:

```cpp
// [v0.1.1] Reticle / FireFeedback UI가 읽을 로컬 발사 피드백 표시 데이터입니다.
USTRUCT(BlueprintType)
struct FCFVehicleFireFeedbackViewData
{
	GENERATED_BODY()

	// [v0.1.1] 현재 표시할 발사 피드백 상태입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	ECFVehicleFireFeedbackState FeedbackState = ECFVehicleFireFeedbackState::None;

	// [v0.1.1] WeaponFire에서 기록한 마지막 발사 거부 사유입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	ECFVehicleFireRejectReason LastRejectReason = ECFVehicleFireRejectReason::None;

	// [v0.1.1] 현재 피드백을 화면에 표시해야 하는지 여부입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	bool bFeedbackActive = false;

	// [v0.1.1] 기본 Reticle 상태를 피드백 상태로 덮어써야 하는지 여부입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	bool bOverrideReticleState = false;

	// [v0.1.1] 쿨다운 UI를 표시해야 하는지 여부입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	bool bShowCooldown = false;

	// [v0.1.1] 남은 쿨다운 시간입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	float RemainingCooldownSeconds = 0.0f;

	// [v0.1.1] 전체 쿨다운 시간입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	float TotalCooldownSeconds = 0.0f;

	// [v0.1.1] 0.0~1.0 범위의 쿨다운 비율입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	float CooldownRatio = 0.0f;

	// [v0.1.1] 조준각 경고를 보조 표시로 띄워야 하는지 여부입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	bool bShowOutOfArcWarning = false;

	// [v0.1.1] FireFeedback 상태를 한국어 표시로 변환하기 전의 짧은 내부 표시 키입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	FName FeedbackDisplayKey = NAME_None;
};
```

## Acceptance Criteria

- `UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h` exists.
- The file contains `#pragma once`.
- The file includes `CoreMinimal.h`.
- The file includes `CFVehicleAimTypes.h`.
- The file includes `CFVehicleFireFeedbackTypes.generated.h` after other includes.
- `ECFVehicleFireFeedbackState` is declared as `UENUM(BlueprintType)`.
- `FCFVehicleFireFeedbackViewData` is declared as `USTRUCT(BlueprintType)`.
- `FCFVehicleFireFeedbackViewData` contains all required `BlueprintReadOnly` UPROPERTY fields.
- `FCFVehicleFireFeedbackViewData` uses `ECFVehicleFireRejectReason` from `CFVehicleAimTypes.h`.
- No other C++ file is modified.
- No Blueprint or WBP asset is modified.
- No document file is modified by Codex.
- No server, RPC, replication, or multiplayer code is added.

## Verification

- Inspect git diff and confirm the only intended new file is `UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h`.
- Confirm `CFVehicleFireFeedbackTypes.generated.h` is the last include.
- Confirm every required field is `BlueprintReadOnly`.
- Confirm `ECFVehicleFireRejectReason` resolves through `CFVehicleAimTypes.h`.
- If a C++ build is available, run the normal project build or Unreal C++ compile path.
- If build cannot be run, report that build was not run and provide static verification instead.

## Reporting Format

```text
작업 완료 보고

생성 파일:
- UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h

추가 타입:
- ECFVehicleFireFeedbackState
- FCFVehicleFireFeedbackViewData

수정하지 않은 파일:
- CFVehiclePawn.h / .cpp
- CFAimReticleWidget.h / .cpp
- CFVehicleWeaponComp.h / .cpp
- WBP assets
- Documents

검증:
- generated.h include order:
- UENUM / USTRUCT:
- UPROPERTY BlueprintReadOnly fields:
- git diff:
- build:

특이사항:
- 없음, or describe issue
```
