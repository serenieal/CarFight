# CFVNetStateBase TaskSource

Document version: v0.1.0
Date: 2026-06-05
Project: CarFight
Task type: Code Task for Codex
Task name: CFVNetStateBase

## Goal

Add the first vehicle-specific replicated NetState pipeline without applying movement correction or interpolation.

This task creates a server-authoritative vehicle NetState, replicates it to clients, and stores received samples in a client-local buffer. The buffer must record ReceivedLocalTimeSeconds because CFVNetAgeSplit showed that ReceivedAge is stable while ServerTimeDelta can still become negative or drift by seconds.

This task is a foundation step only. It must not change visible movement behavior.

## scope.in

The task is in scope for base vehicle NetState replication and receive buffering only.

Required implementation:

- Add FCFVehicleNetState in CFVehiclePawn.h near FCFVehicleNetDebugSample.
- FCFVehicleNetState should be BlueprintType.
- FCFVehicleNetState fields should include bValid, ServerSequenceId, ServerTimeSeconds, ServerLocation, ServerRotation, ServerLinearVelocity, ServerAngularVelocityDeg, and ServerForwardSpeedCmPerSec.
- Use compact network-friendly vector types where appropriate, such as FVector_NetQuantize10 for replicated location and velocity style vectors.
- Every UPROPERTY in FCFVehicleNetState must have Korean DisplayName and ToolTip.
- Add a client-local buffer item type for received NetState samples.
- Recommended name: FCFVehicleNetStateBufferItem.
- Buffer item should store FCFVehicleNetState State and float ReceivedLocalTimeSeconds.
- Add replicated property ReplicatedVehicleNetState to ACFVehiclePawn with ReplicatedUsing OnRep_VehicleNetState.
- Add a non-replicated TArray buffer to ACFVehiclePawn for received NetState samples.
- Recommended name: VehicleNetStateBuffer.
- Add a non-replicated local integer or float state as needed for sequence and sample timing.
- Add settings for NetState base replication, including enable flag, sample interval, max buffer samples, and optional buffer log flag or log interval.
- Suggested setting names: bEnableVehicleNetStateBase, VehicleNetStateSampleIntervalSec, VehicleNetStateMaxBufferSamples, bLogVehicleNetStateBase, VehicleNetStateBaseLogIntervalSec.
- Settings must use UPROPERTY with Korean DisplayName and ToolTip.
- Server must update ReplicatedVehicleNetState only on Authority.
- Server update should be sampled by VehicleNetStateSampleIntervalSec, initial target around 0.0667 seconds.
- OnRep_VehicleNetState must store the received state into VehicleNetStateBuffer with ReceivedLocalTimeSeconds from GetWorld()->GetTimeSeconds().
- OnRep must ignore invalid states.
- OnRep must ignore duplicate or older ServerSequenceId samples when the buffer already has a newer or same sequence as the last item.
- Buffer must be trimmed to VehicleNetStateMaxBufferSamples.
- Add a lightweight diagnostic log with prefix VehicleNetStateBase: if bLogVehicleNetStateBase is enabled.
- The log should include Pawn, Role, IsLocal, Seq, BufferCount, ServerTimeSeconds, ReceivedLocalTimeSeconds, RepMove, and bReplicates.
- Register ReplicatedVehicleNetState in GetLifetimeReplicatedProps.
- Add one-line Korean comments above newly added variables and functions.
- Update local comments/version notes around changed code to v2.25.0.
- Bump CFVehiclePawn.h and CFVehiclePawn.cpp file header version to v2.25.0 if the file currently uses version headers.

## target_files

Codex may modify these source files:

- UE/Source/CarFight_Re/Public/CFVehiclePawn.h
- UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp

Codex must create this review document:

- Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetStateBase_Review.md

## Scope Out

Do not implement or modify any of the following:

- Remote vehicle interpolation.
- Transform correction.
- SetActorLocation.
- SetActorRotation.
- TeleportTo.
- Owner prediction.
- Server correction.
- Reconciliation.
- Input history or replay.
- bReplicateMovement default policy.
- BP_CFVehiclePawn or any uasset.
- Weapon, firing, damage, lock-on, turret, fitting, or combat systems.
- Input mapping or gamepad filtering logic.
- Config files.
- New C++ files.

## Constraints

- Follow existing CarFight C++ style.
- Do not add new C++ files for this task.
- Add one-line Korean comments above newly added variables and functions.
- Every UPROPERTY or UFUNCTION added by this task must include Korean DisplayName and ToolTip.
- Do not change runtime movement behavior.
- Do not change physics simulation behavior.
- Do not use NetState to move, snap, smooth, teleport, or correct any actor.
- Keep existing VehicleNetDebug diagnostics intact.
- Keep existing VehicleNetDebug log prefix and fields intact.
- NetState buffer is client-local storage only in this task.
- ReplicatedVehicleNetState is authoritative server sample data only.
- ReceivedLocalTimeSeconds is the primary buffer timing value for future interpolation planning.
- ServerTimeSeconds is stored as informational server-side sample time, not the sole interpolation clock.

## Existing Code Anchors

Current header anchors:

- FCFVehicleNetDebugSample exists near the top of CFVehiclePawn.h.
- VehicleNetDebugServerSample is a replicated diagnostic sample.
- OnRep_VehicleNetDebugServerSample is already declared.
- LogVehicleNetDebugClientError is already declared.

Current cpp anchors:

- Tick currently calls UpdateVehicleNetDebug.
- GetLifetimeReplicatedProps currently registers VehicleNetDebugServerSample.
- CaptureVehicleNetDebugSample captures location, rotation, linear velocity, angular velocity, and server time.
- GetVehicleNetDebugAngularVelocityDeg already returns current angular velocity in deg/s.
- ResolveVehicleNetDebugServerTimeSeconds already returns GameState server time and validity.

Recommended implementation anchors:

- Add NetState update call from Tick, separate from VehicleNetDebug logging.
- Add CaptureVehicleNetState using existing helper functions where practical.
- Add OnRep_VehicleNetState near OnRep_VehicleNetDebugServerSample.
- Add buffer helper functions near VehicleNetDebug helper functions.

## Expected Runtime Behavior

- With bEnableVehicleNetStateBase enabled, server updates ReplicatedVehicleNetState periodically.
- Clients receive OnRep_VehicleNetState.
- Clients append valid, newer NetState samples to VehicleNetStateBuffer.
- Buffer size never exceeds VehicleNetStateMaxBufferSamples.
- No visible movement correction occurs.
- No actor Transform is changed by the NetState code.

## Expected Log Fields

If bLogVehicleNetStateBase is enabled, log prefix must be:

- VehicleNetStateBase:

Recommended fields:

- Pawn
- NetMode
- Role
- RemoteRole
- IsLocal
- bReplicates
- RepMove
- Seq
- BufferCount
- ServerTime
- ReceivedLocalTime

## Acceptance

Accept the task only when all conditions are true:

- Code changes are limited to CFVehiclePawn.h and CFVehiclePawn.cpp.
- CFVNetStateBase_Review.md is created.
- FCFVehicleNetState exists.
- ReplicatedVehicleNetState exists and is registered in GetLifetimeReplicatedProps.
- OnRep_VehicleNetState exists.
- VehicleNetStateBuffer exists and stores received states with ReceivedLocalTimeSeconds.
- Duplicate or older ServerSequenceId samples are ignored.
- Buffer trimming is implemented.
- Optional VehicleNetStateBase log exists or the review explains why it was not added.
- No Transform correction or interpolation is added.
- Existing VehicleNetDebug logging remains intact.
- No Blueprint asset or Config file is modified.
- Editor and Server builds pass.

## Verification

Codex must verify:

- Build CarFight_ReEditor Win64 Development.
- Build CarFight_ReServer Win64 Development.
- Search CFVehiclePawn.cpp for SetActorLocation, SetActorRotation, and TeleportTo.
- Confirm no forbidden Transform correction call was introduced.
- Confirm ReplicatedVehicleNetState is included in GetLifetimeReplicatedProps.
- Confirm OnRep_VehicleNetState records ReceivedLocalTimeSeconds.
- Confirm buffer trimming exists.
- Confirm the review document exists.

Runtime verification after the code task:

- Run A_RepMoveTrue keyboard test.
- Run B_RepMoveFalse keyboard test.
- Extract logs.
- Confirm VehicleNetStateBase logs appear if logging is enabled.
- Confirm BufferCount grows but stays within VehicleNetStateMaxBufferSamples.
- Confirm existing VehicleNetDebug logs still appear.
- Confirm no visible movement behavior was intentionally changed by this task.

## Review Document Requirements

Create Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetStateBase_Review.md.

The review must include:

- Changed files.
- Version and changelog.
- Summary of FCFVehicleNetState fields.
- Summary of ReplicatedVehicleNetState and OnRep behavior.
- Buffer policy and max count.
- Explanation that ReceivedLocalTimeSeconds is primary future interpolation timing value.
- Confirmation that no Transform correction or interpolation was added.
- Build results.
- Forbidden call search results.
- Runtime retest instructions.
- Migration note that no Blueprint action is required unless the user wants to tune exposed settings.

## Changelog

v0.1.0

- Created initial Codex task source for CFVNetStateBase.
- Limited task to NetState replication and client-local receive buffering.
- Explicitly excluded interpolation, Transform correction, prediction, server correction, BP, Config, combat, and input work.
