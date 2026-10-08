# Codex Task Source: CFCamDSTick Cleanup

Version: v0.1.0
Date: 2026-06-01
Project: CarFight
Executor: Codex
Task type: Unreal Engine C++ targeted cleanup

## Goal

Clean up `UCFVehicleCameraComp` so camera runtime work is skipped on a Dedicated Server.

The multiplayer vehicle server test already passed. This task is only for server-side cost reduction and cleaner runtime separation.

## Target Files

- `UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp`

## In Scope

- Add a small helper function named `ShouldSkipCameraRuntimeOnDedicatedServer` or another equally clear name.
- The helper should return true when the world exists and the net mode is `NM_DedicatedServer`.
- In `BeginPlay`, when the helper returns true, disable this component tick and return before camera runtime initialization.
- In `TickComponent`, when the helper returns true, disable this component tick and return before camera update work.
- In `InitializeCameraRuntime`, when the helper returns true, set `bCameraRuntimeReady` to false and return false before camera reference lookup.
- Keep non-Dedicated Server camera behavior unchanged.
- Keep listen server and client camera behavior unchanged.
- Add one-line Korean comments above new function declarations and definitions.
- Update file header version comments if present.

## Out of Scope

- `CFVehiclePawn.h`
- `CFVehiclePawn.cpp`
- UI widget files
- Config files
- Map files
- Blueprint assets
- Vehicle movement replication
- Combat, damage, respawn, lobby, or session work

## Constraints

- Keep the patch small.
- Keep file names under 32 characters.
- Avoid broad refactoring.
- Avoid changing existing camera math.
- Add includes only if required.

## Acceptance Criteria

- `UCFVehicleCameraComp` skips camera runtime work on Dedicated Server.
- Component tick is disabled on Dedicated Server after the skip path is detected.
- Editor target builds.
- Server target builds.
- Only the two target files are changed.

## Verification

- Build `CarFight_ReEditor Win64 Development`.
- Build `CarFight_ReServer Win64 Development`.
- Confirm the helper exists in `CFVehicleCameraComp`.
- Confirm no unrelated files changed.

## References

- `Document/ProjectSSOT/Plan/ServerUpgradePlan/DedicatedServerCodeAudit.md`
- `Document/ProjectSSOT/Plan/ServerUpgradePlan/RuntimeTest_20260601.md`
