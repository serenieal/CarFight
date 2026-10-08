# Codex Task Source: CFMPGameMode Connect and Runtime Guard

Version: v0.1.0
Date: 2026-06-01
Project: CarFight
Executor: Codex
Task type: Unreal Engine C++ and config update

## Goal

Make `ACFMPGameMode` usable in a Dedicated Server runtime test without requiring a separate Blueprint GameMode asset.

The previous task added `ACFMPGameMode` and both Editor and Server targets build successfully. However, `VehiclePawnClass` currently defaults to null, so connecting the GameMode alone will not spawn `BP_CFVehiclePawn`. This task connects the GameMode and gives it a safe default vehicle class for the first server test.

## Target Files

- `UE/Source/CarFight_Re/Public/CFMPGameMode.h`
- `UE/Source/CarFight_Re/Private/CFMPGameMode.cpp`
- `UE/Config/DefaultEngine.ini`

## In Scope

- Keep `ACFMPGameMode` as the server test GameMode.
- Add a safe default setup so `VehiclePawnClass` resolves to `BP_CFVehiclePawn` when no subclass override is provided.
- Use the existing vehicle Blueprint path: `/Game/CarFight/Vehicles/BP_CFVehiclePawn`.
- Keep `VehiclePawnClass` editable through UPROPERTY.
- Prevent the default GameMode pawn flow from creating an unrelated default pawn during this server test path.
- Consider setting `DefaultPawnClass` to null in the `ACFMPGameMode` constructor if needed.
- Consider overriding `HandleStartingNewPlayer_Implementation` if needed to avoid duplicate or default pawn spawning.
- Keep `PostLogin` logging clear.
- Ensure the actual vehicle spawn flow still ends in `SpawnVehicleForController`.
- Update `DefaultEngine.ini` so the project can use `ACFMPGameMode` for server testing.
- Add or update `GlobalDefaultGameMode` if needed.
- Add or update `GlobalDefaultServerGameMode` if needed.
- Keep comments directly above new or changed functions.
- Keep Korean DisplayName and ToolTip metadata for exposed properties.

## Out of Scope

- Do not edit maps.
- Do not edit Blueprints.
- Do not modify `BP_CFVehiclePawn`.
- Do not modify `CFVehiclePawn.h`.
- Do not modify `CFVehiclePawn.cpp`.
- Do not add combat systems.
- Do not add score systems.
- Do not add respawn systems.
- Do not add lobby systems.
- Do not add session systems.
- Do not tune vehicle movement replication.

## Constraints

- Keep file names under 32 characters.
- Keep the change minimal and test-oriented.
- Do not remove existing logging unless replacing it with clearer logging.
- Do not hide important server flow in overly compressed helper logic.
- If using a hard-coded Blueprint class fallback, keep it clearly commented as a first dedicated-server test default.
- The vehicle class must still be overrideable by subclass defaults later.

## Acceptance Criteria

- `ACFMPGameMode` still compiles.
- `VehiclePawnClass` has a usable default pointing to `BP_CFVehiclePawn` or an equivalent safe fallback mechanism.
- `ACFMPGameMode` does not rely on an external Blueprint GameMode asset for the first server test.
- Default pawn auto-spawn cannot replace or block the intended vehicle spawn flow.
- `DefaultEngine.ini` points server testing to `ACFMPGameMode`.
- Existing `ACFVehiclePawn` files are not modified.
- Map and Blueprint assets are not modified.

## Verification

- Build `CarFight_ReEditor Win64 Development`.
- Build `CarFight_ReServer Win64 Development`.
- Confirm `DefaultEngine.ini` contains the intended GameMode entries.
- Confirm only the target files were modified.

## Reference Documents

- `Document/ProjectSSOT/Plan/ServerUpgradePlan/CodexTasks/CFMPGameMode_Review.md`
- `Document/ProjectSSOT/Plan/ServerUpgradePlan/AuditResult.md`
- `Document/ProjectSSOT/Plan/ServerUpgradePlan/ServerDesign.md`
- `Document/ProjectSSOT/Plan/ServerUpgradePlan/TaskList.md`
