# Codex Task Source: CFServerLauncher v1.0 Final Release

Version: v1.0.8-task  
Date: 2026-05-29  
Executor: Codex  
Work type: C# WPF final-release code/config/script change

## Goal

Finalize `CFServerLauncher` v1.0 after the v1.0.0-rc1 release-candidate checks passed manually.

The launcher already passed these checks:

- `dotnet build`
- app launch
- version display
- server start
- TestMap log display
- server stop
- restart after stop
- server EXE browse
- working folder browse
- log file browse
- ActiveLogFilePath display
- timestamp runtime log file
- base LogFilePath preservation
- close confirmation while server is running

This task should convert the app from release candidate to final v1.0.0 and add a simple repeatable publish script.

## Target Files

Codex may edit these files:

```text
Tools/CFServerLauncher/src/CFServerLauncher/ViewModels/MainViewModel.cs
Tools/CFServerLauncher/Docs/release.md
```

Codex may add this file:

```text
Tools/CFServerLauncher/publish.ps1
```

Do not edit other files unless `dotnet build` fails and the edit is strictly required.

## In Scope

### 1. Change displayed app version from rc to final

Change the UI version text from:

```text
버전: v1.0.0-rc1
```

to:

```text
버전: v1.0.0
```

Keep the property name `AppVersionText` unchanged.

### 2. Add simple publish script

Add:

```text
Tools/CFServerLauncher/publish.ps1
```

The script should publish the WPF app to:

```text
Tools/CFServerLauncher/Publish
```

Recommended command:

```powershell
dotnet publish .\src\CFServerLauncher\CFServerLauncher.csproj -c Release -r win-x64 --self-contained false -o .\Publish
```

Script requirements:

1. It should run from `Tools/CFServerLauncher`.
2. It should set `$ErrorActionPreference = "Stop"`.
3. It should create or refresh the `Publish` folder.
4. It should print Korean progress messages.
5. It should not delete `Server`, `Config`, or `Logs` folders.
6. It should not copy server build files automatically.
7. It should not require administrator permission.

### 3. Update release documentation

Update:

```text
Tools/CFServerLauncher/Docs/release.md
```

Required updates:

1. Version should be `v1.0.0` instead of `v1.0.0-rc1`.
2. Document the new `publish.ps1` usage.
3. Keep direct `dotnet publish` command as fallback.
4. Add final release verification checklist.
5. Mention that `Server` build files are not bundled automatically by the launcher publish script.

## Constraints

1. UI text and script messages must be Korean.
2. Every new C# method must have a short one-line summary comment directly above it. If no new C# method is added, no action is needed.
3. Do not add third-party NuGet packages.
4. Do not change Unreal server code.
5. Do not add Unreal build, Cook, Stage, RCON, web dashboard, profile system, auto restart, or process monitoring.
6. Keep the launcher independent from Unreal internal game code.
7. File names and class names must stay under 32 characters.

## Out of Scope

Do not implement:

```text
profile system
auto restart
RCON
web dashboard
Unreal build automation
Cook/Stage automation
CPU/memory/TPS monitoring
server player list
game internal admin command
server build file packaging
installer creation
```

## Acceptance Criteria

1. `dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln` succeeds.
2. App window shows `버전: v1.0.0`.
3. Existing start/stop/restart behavior still works.
4. Existing close-confirmation behavior still works.
5. `Tools\CFServerLauncher\publish.ps1` exists.
6. Running `publish.ps1` from `Tools\CFServerLauncher` creates or refreshes `Publish`.
7. `Publish` contains the launcher executable after the script runs.
8. `Docs\release.md` documents both `publish.ps1` and direct `dotnet publish` fallback.
9. The script does not delete `Server`, `Config`, or `Logs` folders.
10. The script does not copy server build files automatically.

## Verification

Run these commands in PowerShell:

```powershell
cd D:\Work\CarFight_git
dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln
dotnet run --project .\Tools\CFServerLauncher\src\CFServerLauncher\CFServerLauncher.csproj
```

Manual app verification:

```text
1. Confirm app opens.
2. Confirm bottom version text is 버전: v1.0.0.
3. Start server.
4. Confirm TestMap logs appear.
5. Close app while server is running.
6. Choose No and confirm app/server stay running.
7. Close app again.
8. Choose Yes and confirm server stops and app closes.
9. Reopen app and confirm normal start/stop/restart still works.
```

Publish verification:

```powershell
cd D:\Work\CarFight_git\Tools\CFServerLauncher
.\publish.ps1
```

Then confirm:

```text
1. Publish folder exists.
2. Publish contains CFServerLauncher.exe.
3. Server folder still exists.
4. Config folder still exists.
5. Logs folder still exists.
```

## Reference Documents

Read-only references:

```text
Document/ProjectSSOT/Plan/CFServerLauncher/README.md
Document/ProjectSSOT/Plan/CFServerLauncher/Status.md
Document/ProjectSSOT/Plan/CFServerLauncher/VerifyResult.md
Document/ProjectSSOT/Plan/CFServerLauncher/ReleaseReview.md
Tools/CFServerLauncher/Docs/release.md
```
