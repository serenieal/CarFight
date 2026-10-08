# Codex Task Source: UCFVehicleCameraComp Dedicated Server Guard

Version: v0.1.0
Date: 2026-06-01
Project: CarFight
Executor: Codex
Task type: Unreal Engine C++ targeted cleanup

## Goal

Add a Dedicated Server guard to `UCFVehicleCameraComp` so camera runtime work does not run on a Dedicated Server.

Runtime tests already passed, so this is not an emergency bug fix. This is a server stability and unnecessary Tick cleanup task.

## Current Finding

`DedicatedServerCodeAudit.md` found that most local-only paths are already guarded:

- Aim Reticle UI is guarded by `GetNetMode() != NM_DedicatedServer` and `IsLocallyControlled()`.
- Enhanced Input LocalPlayer access is guarded.
- VehicleDebug HUD / Panel visibility is guarded.
- There are no current C++ `PlaySound`, `SpawnEmitter`, or `Niagara` calls.

The clear remaining cleanup target is:

- `UCFVehicleCameraComp` has Tick enabled by default.
- `UCFVehicleCameraComp::TickComponent` has no Dedicated Server early return.
- `UCFVehicleCameraComp::BeginPlay` can call `InitializeCameraRuntime` on Dedicated Server.

## Target Files

- `UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp`

## In Scope

- Add a helper function that determines whether camera runtime should be skipped on Dedicated Server.
- Add a Dedicated Server guard in `BeginPlay`.
- Add a Dedicated Server early return in `TickComponent`.
- Disable component Tick on Dedicated Server when the guard is triggered.
- Prevent `InitializeCameraRuntime` from doing camera reference resolution on Dedicated Server.
- Keep the client and listen-server owner-client camera behavior unchanged.
- Keep existing camera math behavior unchanged for non-Dedicated Server contexts.
- Update file header version comments if present.
- Add one-line Korean comments above any new function declaration and definition.
- Use intuitive function names.

Recommended helper name:

- `ShouldSkipCameraRuntimeOnDedicatedServer`

Recommended behavior:

- Return true when `GetWorld()` exists and `GetWorld()->GetNetMode() == NM_DedicatedServer`.
- If true in `BeginPlay`, call `SetComponentTickEnabled(false)` and return.
- If true in `TickComponent`, call `SetComponentTickEnabled(false)` and return.
- If true in `InitializeCameraRuntime`, set `bCameraRuntimeReady = false` and return false.

## Out of Scope

- Do not modify `CFVehiclePawn.h`.
- Do not modify `CFVehiclePawn.cpp`.
- Do not modify UI widget files.
- Do not modify Config files.
- Do not modify maps.
- Do not modify Blueprints.
- Do not change Aim Trace logic except where guarded by Dedicated Server skip.
- Do not change vehicle movement replication.
- Do not add combat, damage, respawn, lobby, or session systems.

## Constraints

- Keep file names under 32 characters.
- Do not remove existing camera features.
- Do not change public Blueprint-facing behavior unless needed for the guard.
- Avoid a broad refactor.
- Keep the patch minimal and easy to review.
- Do not introduce new dependencies unless required for `GetWorld()->GetNetMode()`.
- If adding includes, keep them minimal.

## Acceptance Criteria

- `UCFVehicleCameraComp` no longer performs camera runtime initialization on Dedicated Server.
- `UCFVehicleCameraComp` disables its Tick on Dedicated Server.
- `TickComponent` has a Dedicated Server early return.
- Non-Dedicated Server camera behavior remains unchanged.
- Only the two target files are modified.
- `CarFight_ReEditor Win64 Development` builds successfully.
- `CarFight_ReServer Win64 Development` builds successfully.

## Verification

After Codex finishes:

- Build `CarFight_ReEditor Win64 Development`.
- Build `CarFight_ReServer Win64 Development`.
- Search `CFVehicleCameraComp.cpp` for the new Dedicated Server guard.
- Confirm no Config, map, Blueprint, or vehicle Pawn files were changed.

## Reference Documents

- `Document/ProjectSSOT/Plan/ServerUpgradePlan/DedicatedServerCodeAudit.md`
- `Document/ProjectSSOT/Plan/ServerUpgradePlan/RuntimeTest_20260601.md`
- `Document/ProjectSSOT/Plan/ServerUpgradePlan/TaskList.md`
