# CFVNetAgeSplit TaskSource

Document version: v0.3.0
Date: 2026-06-05
Project: CarFight
Task type: Code Task for Codex
Task name: CFVNetAgeSplit

## Goal

Separate VehicleNetDebug time diagnostics into ServerTimeDelta and ReceivedAge.

The current VehicleNetDebug log has SampleAge. CFVNetDbgFix changed it to use GameState server time, but keyboard retest still showed negative SampleAge in some cases even when ServerTimeValid and CurrentServerTimeValid were true.

This task must keep SampleAge for compatibility, but add two clearer values.

ServerTimeDelta means client estimated server time minus replicated server sample time.

ReceivedAge means client local elapsed time since OnRep_VehicleNetDebugServerSample received the replicated sample.

ReceivedAge is the important value for future interpolation buffer diagnostics.

## scope.in

The task is in scope for VehicleNetDebug age diagnostics.

Required implementation:

- Add a client-local float field to ACFVehiclePawn for the local time when VehicleNetDebugServerSample was received.
- Recommended name: LastVehicleNetDebugSampleReceiveLocalTimeSec.
- Initialize the field to -1.0f.
- The new receive timestamp field must not be replicated.
- Add a one-line Korean comment above the new field.
- If UPROPERTY is used for the field, include Korean DisplayName and ToolTip.
- In OnRep_VehicleNetDebugServerSample, store GetWorld()->GetTimeSeconds() into the receive timestamp before calling LogVehicleNetDebugClientError.
- In LogVehicleNetDebugClientError, calculate ServerTimeDeltaSec from CurrentServerTimeSeconds minus VehicleNetDebugServerSample.ServerWorldTimeSeconds when both server time values are valid. Otherwise use -1.0f.
- In LogVehicleNetDebugClientError, calculate ReceivedAgeSec from CurrentWorldTimeSec minus LastVehicleNetDebugSampleReceiveLocalTimeSec when the receive timestamp is valid. Otherwise use -1.0f.
- Preserve existing VehicleNetDebug log prefix.
- Preserve existing VehicleNetDebug fields.
- Add ServerTimeDelta and ReceivedAge to the log line.
- Keep SampleAge in the log for compatibility. For this task, SampleAge may output the same value as ServerTimeDeltaSec.
- Update local comments/version notes around changed code to v2.24.0.
- Bump CFVehiclePawn.h and CFVehiclePawn.cpp file header version to v2.24.0 if the file currently uses version headers.

## target_files

Codex may modify these source files:

- UE/Source/CarFight_Re/Public/CFVehiclePawn.h
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp

Codex must create this review document:

- Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetAgeSplit_Review.md

## Scope Out

Do not implement or modify any of the following:

- FCFVehicleNetState.
- Remote vehicle interpolation.
- Transform correction.
- SetActorLocation.
- SetActorRotation.
- TeleportTo.
- Owner prediction.
- Server correction.
- bReplicateMovement default policy.
- BP_CFVehiclePawn or any uasset.
- Weapon, firing, damage, lock-on, turret, fitting, or combat systems.
- Input mapping or gamepad filtering logic.
- Config files.

## Constraints

- Follow existing CarFight C++ style.
- Do not add new C++ files for this task.
- Add one-line Korean comments above newly added variables/functions.
- Any UPROPERTY or UFUNCTION added by this task must include Korean DisplayName and ToolTip.
- Do not silently change existing public API semantics beyond the diagnostic log additions.
- Keep VehicleNetDebug diagnostics enabled or disabled by the existing bEnableVehicleNetDebug setting.
- Do not change runtime movement behavior.
- Do not change physics simulation behavior.

## Existing Code Anchors

Current header anchors:

- FCFVehicleNetDebugSample exists in CFVehiclePawn.h.
- VehicleNetDebugServerSample is ReplicatedUsing OnRep_VehicleNetDebugServerSample.
- OnRep_VehicleNetDebugServerSample is declared in the protected section.
- LogVehicleNetDebugClientError is declared in the protected section.

Current cpp anchors:

- OnRep_VehicleNetDebugServerSample currently calls LogVehicleNetDebugClientError directly.
- LogVehicleNetDebugClientError currently calculates SampleAgeSec from GameState server time.
- The log prefix is VehicleNetDebug: and must remain unchanged.

## Expected Log Fields

Existing log fields must remain:

- Pawn
- NetMode
- Role
- RemoteRole
- HasAuthority
- IsLocal
- bReplicates
- RepMove
- OwnerController
- RuntimeReady
- Seq
- ServerTimeValid
- CurrentServerTimeValid
- SampleAge
- LocErr
- RotErr
- VelErr
- SpeedErr
- AngVelErr
- ServerLoc
- LocalLoc
- ServerVel
- LocalVel
- ServerAngVel
- LocalAngVel

New log fields:

- ServerTimeDelta=%.3fs
- ReceivedAge=%.3fs

Recommended placement:

- Put ServerTimeDelta and ReceivedAge immediately after SampleAge.

## Acceptance

Accept the task only when all conditions are true:

- Code changes are limited to CFVehiclePawn.h and CFVehiclePawn.cpp.
- CFVNetAgeSplit_Review.md is created.
- VehicleNetDebug log prefix remains unchanged.
- Existing log fields remain present.
- New log fields ServerTimeDelta and ReceivedAge are present.
- OnRep stores client local receive time before logging.
- ReceivedAge is based on client local world time.
- ServerTimeDelta is based on GameState server time comparison.
- No Transform correction or interpolation is added.
- No Blueprint asset or Config file is modified.
- Editor and Server builds pass.

## Verification

Codex must verify:

- Build CarFight_ReEditor Win64 Development.
- Build CarFight_ReServer Win64 Development.
- Search CFVehiclePawn.cpp for SetActorLocation, SetActorRotation, and TeleportTo.
- Confirm no forbidden Transform correction call was introduced.
- Confirm the VehicleNetDebug log string contains ServerTimeDelta and ReceivedAge.
- Confirm the review document exists.

Runtime verification after the code task:

- Run A_RepMoveTrue keyboard test.
- Run B_RepMoveFalse keyboard test.
- Extract VehicleNetDebug logs.
- Check whether ReceivedAge stays stable and non-negative after first valid OnRep.
- Check whether ServerTimeDelta may still go negative independently from ReceivedAge.

## Review Document Requirements

Create Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetAgeSplit_Review.md.

The review must include:

- Changed files.
- Version and changelog.
- Summary of added receive timestamp field.
- Meaning of SampleAge, ServerTimeDelta, and ReceivedAge.
- Build results.
- Forbidden call search results.
- Runtime retest instructions.
- Migration note that no Blueprint action is required.

## Changelog

v0.3.0

- Added exact scope.in and target_files sections for plan compiler.

v0.2.0

- Rewrote TaskSource in plan-friendly compact sections.
- Added explicit Goal and Constraints sections.

v0.1.0

- Created initial Codex task source for CFVNetAgeSplit.
