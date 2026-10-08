# Codex Task Source: CFMPGameMode Minimum GameMode

Version: v0.1.2
Date: 2026-06-01
Project: CarFight
Executor: Codex
Task type: Unreal Engine C++ new files

## Goal

Add a minimal multiplayer GameMode for CarFight Dedicated Server.

The new GameMode must let the server create one vehicle Pawn for each connected PlayerController and make that PlayerController possess the created Pawn.

Current verified state:

- `UE/Source/CarFight_ReServer.Target.cs` exists.
- There is no custom C++ GameMode in the project.
- `UE/Config/DefaultEngine.ini` has no `GlobalDefaultGameMode` or `GlobalDefaultServerGameMode` yet.
- `/Game/Maps/TestMap.TestMap` has one PlayerStart and one placed `BP_CFVehiclePawn`.
- `ACFVehiclePawn` has `AutoPossessPlayer = Disabled` in C++ defaults.
- `ACFVehiclePawn` already has some Dedicated Server and local-owner guards for input and reticle UI.

## Target Files

- `UE/Source/CarFight_Re/Public/CFMPGameMode.h`
- `UE/Source/CarFight_Re/Private/CFMPGameMode.cpp`

## In Scope

- Create `ACFMPGameMode`.
- Prefer `AGameModeBase` as the base class.
- Add `VehiclePawnClass`.
- Add `bSpawnVehicleOnPostLogin`.
- Add `SpawnIndex`.
- Add `SpawnOffsetBetweenPlayers`.
- Add `PostLogin`.
- Add `Logout`.
- Add `SpawnVehicleForController`.
- Add `FindVehicleSpawnTransform`.
- Add `ResolveVehiclePawnClass`.
- `PostLogin` must call `Super::PostLogin`.
- `PostLogin` must validate the PlayerController.
- Server authority path must call the vehicle spawn and possess flow when `bSpawnVehicleOnPostLogin` is true.
- If the controller already owns a Pawn, do not create a duplicate vehicle.
- `VehiclePawnClass` must be configurable through UPROPERTY.
- If `VehiclePawnClass` is not set, fail safely and log a warning.
- Prefer `ChoosePlayerStart` for spawn transform.
- If no PlayerStart is available, use a deterministic fallback transform based on `SpawnIndex` and `SpawnOffsetBetweenPlayers`.
- Log important flow points: login, missing class, PlayerStart use, fallback transform use, spawn success, spawn failure, possess success, possess failure.
- Add Korean `DisplayName` and Korean `ToolTip` metadata to UPROPERTY values.
- Add one-line Korean comments above important variables.
- Add one-line Korean comments above function declarations.
- Add one-line Korean comments above function definitions.

## Out of Scope

- Do not modify `UE/Config/DefaultEngine.ini`.
- Do not modify `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`.
- Do not modify `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`.
- Do not modify `/Game/Maps/TestMap.TestMap`.
- Do not modify `/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn`.
- Do not implement Config connection.
- Do not edit maps.
- Do not edit Blueprints.
- Do not change the existing vehicle Pawn class.
- Do not add combat systems.
- Do not add score systems.
- Do not add respawn systems.
- Do not add lobby systems.
- Do not add session systems.
- Do not tune movement smoothing.

## Constraints

- File names must stay under 32 characters.
- Follow Unreal Engine C++ style.
- Use intuitive variable and function names.
- Do not shorten the implementation in a way that hides important beginner-level flow.
- Keep this task limited to the two target files.
- Do not hard-load the vehicle Blueprint path in this task.
- Leave `VehiclePawnClass` configurable for a later Config or Blueprint setup step.
- If `ACFVehiclePawn` replication defaults may need improvement, mention it in the result note only.

## Acceptance Criteria

- `CFMPGameMode.h` exists.
- `CFMPGameMode.cpp` exists.
- `ACFMPGameMode` is a valid Unreal C++ class.
- The class can compile with Unreal Header Tool.
- `PostLogin` contains the server vehicle spawn and possess entry point.
- The class has a configurable `VehiclePawnClass` UPROPERTY.
- Missing `VehiclePawnClass` does not crash and produces a clear log warning.
- PlayerStart is used when available.
- A fallback transform is used when PlayerStart is unavailable.
- No file outside the two target files is modified.

## Verification

- Unreal C++ build succeeds.
- Unreal Header Tool reports no error for `ACFMPGameMode`.
- Only the two target files were created or modified.
- No Config, map, Blueprint, or vehicle Pawn files were changed.
- Dedicated Server runtime connection testing is not part of this task.

## Reference Documents

- `Document/ProjectSSOT/Plan/ServerUpgradePlan/AuditResult.md`
- `Document/ProjectSSOT/Plan/ServerUpgradePlan/ServerDesign.md`
- `Document/ProjectSSOT/Plan/ServerUpgradePlan/Roadmap.md`
- `Document/ProjectSSOT/Plan/ServerUpgradePlan/TaskList.md`
