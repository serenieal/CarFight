# Codex Task Source: CFServerLauncher v1.0 Release Prep

Version: v1.0.7-task  
Date: 2026-05-29  
Executor: Codex  
Work type: C# WPF release-prep code/config change

## Goal

Prepare `CFServerLauncher` for a practical v1.0 release candidate after manual verification confirmed that all current checks passed.

The launcher already supports:

- app launch
- server start
- TestMap log display
- server stop
- restart after stop
- browse buttons
- base log path and active runtime log path separation
- runtime log file format `server_yyyyMMdd_HHmmss_fff.log`
- manual stop state `중지됨`
- external exit state `종료됨`

This task should add only small release-prep improvements. Do not expand into v1.1 feature work.

## Target Files

Codex may edit these files:

```text
Tools/CFServerLauncher/src/CFServerLauncher/CFServerLauncher.csproj
Tools/CFServerLauncher/src/CFServerLauncher/MainWindow.xaml
Tools/CFServerLauncher/src/CFServerLauncher/MainWindow.xaml.cs
Tools/CFServerLauncher/src/CFServerLauncher/ViewModels/MainViewModel.cs
```

Codex may add small files under:

```text
Tools/CFServerLauncher
Tools/CFServerLauncher/Docs
```

File names and class names must stay under 32 characters.

## In Scope

### 1. Add version display

Add a small read-only version display to the main window.

Recommended UI text:

```text
버전: v1.0.0-rc1
```

Recommended ViewModel property:

```text
AppVersionText
```

The version can be a constant in `MainViewModel` or project metadata. Keep it simple.

### 2. Add release publish instructions or script

Add one of the following. Prefer the simpler option.

Option A: Add a markdown release instruction file:

```text
Tools/CFServerLauncher/Docs/release.md
```

Option B: Add a simple PowerShell publish script:

```text
Tools/CFServerLauncher/publish.ps1
```

If adding a script, keep it simple and readable. It should publish the WPF app to a local output folder.

Recommended publish output:

```text
Tools/CFServerLauncher/Publish
```

Recommended command inside script:

```powershell
dotnet publish .\src\CFServerLauncher\CFServerLauncher.csproj -c Release -r win-x64 --self-contained false -o .\Publish
```

Do not make the app self-contained unless explicitly needed.

### 3. Add safe close behavior

When the user closes the launcher while the server is running, show a Korean confirmation message.

Expected behavior:

```text
서버가 실행 중입니다. 서버를 종료하고 런처를 닫을까요?
```

Buttons:

- Yes: stop the server, then close the app.
- No: cancel closing.

Keep the implementation simple. If async close handling is awkward, use a guard flag to prevent recursive close.

Recommended private field in `MainWindow.xaml.cs`:

```text
isClosingConfirmed
```

If the server is not running, close normally.

To support this, expose a read-only property or method from `MainViewModel`, for example:

```text
IsServerRunning
StopServerForCloseAsync()
```

### 4. Keep current behavior intact

Do not break these existing behaviors:

- `LogFilePath` stays as base path.
- `ActiveLogFilePath` shows actual runtime log file.
- settings save does not save timestamp log path.
- manual stop final state is `중지됨`.
- external exit final state is `종료됨`.

## Constraints

1. UI text must be Korean.
2. Every new C# method must have a short one-line summary comment directly above it.
3. Do not add third-party NuGet packages.
4. Do not change Unreal server code.
5. Do not add Unreal build, Cook, Stage, RCON, web dashboard, profile system, auto restart, or process monitoring.
6. Keep the launcher independent from Unreal internal game code.
7. Keep v1.0 release-prep small and stable.

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
```

## Acceptance Criteria

1. `dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln` succeeds.
2. App window shows `버전: v1.0.0-rc1` or equivalent v1.0 rc text.
3. If server is not running, closing the app exits normally.
4. If server is running, closing the app shows a Korean confirmation dialog.
5. Choosing No cancels app close and keeps the server running.
6. Choosing Yes stops the server and closes the launcher.
7. Existing start/stop/restart behavior still works.
8. Existing `ActiveLogFilePath` behavior still works.
9. If a publish script is added, running it creates `Tools\CFServerLauncher\Publish`.
10. If a release markdown file is added instead, it contains exact build and publish commands.

## Verification

Run these commands in PowerShell:

```powershell
cd D:\Work\CarFight_git
dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln
dotnet run --project .\Tools\CFServerLauncher\src\CFServerLauncher\CFServerLauncher.csproj
```

Manual verification steps:

```text
1. Confirm app opens.
2. Confirm version text is visible.
3. Close app while server is not running. It should close normally.
4. Reopen app.
5. Start server.
6. Close app while server is running.
7. Choose No. App should stay open and server should keep running.
8. Close app again while server is running.
9. Choose Yes. Server should stop and app should close.
10. Reopen app and confirm normal start/stop/restart still works.
11. If publish.ps1 exists, run it and confirm Publish folder is created.
```

## Reference Documents

Read-only references:

```text
Document/ProjectSSOT/Plan/CFServerLauncher/README.md
Document/ProjectSSOT/Plan/CFServerLauncher/Roadmap.md
Document/ProjectSSOT/Plan/CFServerLauncher/Design.md
Document/ProjectSSOT/Plan/CFServerLauncher/UISpec.md
Document/ProjectSSOT/Plan/CFServerLauncher/RunLogSpec.md
Document/ProjectSSOT/Plan/CFServerLauncher/CheckList.md
Document/ProjectSSOT/Plan/CFServerLauncher/Status.md
```
