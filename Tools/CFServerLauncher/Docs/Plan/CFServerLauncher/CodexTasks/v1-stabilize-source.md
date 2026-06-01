# Codex Task Source: CFServerLauncher v1.0 Stabilize UI and Log Policy

Version: v1.0.5-task  
Date: 2026-05-29  
Executor: Codex  
Work type: C# WPF code change

## Goal

Stabilize the already working `CFServerLauncher` v1.0 and improve basic usability after manual verification confirmed that server start, log display, server stop, and restart work.

## Target Files

Codex may edit only these primary implementation files:

```text
Tools/CFServerLauncher/src/CFServerLauncher/MainWindow.xaml
Tools/CFServerLauncher/src/CFServerLauncher/ViewModels/MainViewModel.cs
```

Codex may read these reference files, but should not edit them unless the build fails and the edit is strictly necessary:

```text
Tools/CFServerLauncher/src/CFServerLauncher/MainWindow.xaml.cs
Tools/CFServerLauncher/src/CFServerLauncher/Services/ArgBuilder.cs
Tools/CFServerLauncher/src/CFServerLauncher/Services/ConfigStore.cs
Tools/CFServerLauncher/src/CFServerLauncher/Services/PathGuard.cs
Tools/CFServerLauncher/src/CFServerLauncher/Services/ServerProc.cs
Tools/CFServerLauncher/src/CFServerLauncher/Utils/AppConst.cs
Tools/CFServerLauncher/src/CFServerLauncher/CFServerLauncher.csproj
Tools/CFServerLauncher/Config/server.sample.json
```

## In Scope

1. Add a `찾아보기` button beside `서버 실행 파일`.
2. Add a `찾아보기` button beside `작업 폴더`.
3. Add a `찾아보기` button beside `로그 파일`.
4. Implement commands in `MainViewModel`:

```text
BrowseExeCommand
BrowseWorkDirCommand
BrowseLogFileCommand
```

5. Keep `LogFilePath` as the user-configured base log path that is saved to JSON.
6. Add a read-only UI property for the actual log file used by the current server run.

Recommended property name:

```text
ActiveLogFilePath
```

7. When starting the server, generate a unique runtime log path from the base `LogFilePath`.
8. Do not overwrite the `LogFilePath` input with the timestamp runtime log path.
9. Show the actual runtime log path in the UI as read-only text.
10. Keep normal server stop from creating a new error message.

## Constraints

1. File names and class names must stay under 32 characters.
2. UI labels and user-facing messages must be Korean.
3. Keep the launcher independent from Unreal server C++ code.
4. Do not access Unreal GameMode, PlayerController, uasset, or map internals.
5. Every new C# method must have a short one-line summary comment directly above it.
6. Prefer simple WPF/.NET built-in dialogs. If folder selection requires Windows Forms, update the csproj explicitly and keep the change minimal.
7. Do not add third-party NuGet packages.

## Out of Scope

Do not implement these items:

```text
Unreal C++ build
Cook/Stage automation
RCON
web dashboard
game internal admin command
auto restart
profile system
CPU/memory/TPS monitoring
```

## Acceptance Criteria

1. `dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln` succeeds.
2. The app launches without startup binding errors.
3. `서버 실행 파일` browse button can select an `.exe` path.
4. `작업 폴더` browse button can select a folder path.
5. `로그 파일` browse button can select or set a `.log` base path.
6. Starting the server creates a runtime log file with a timestamp suffix.
7. Runtime log filename format includes milliseconds, for example:

```text
server_yyyyMMdd_HHmmss_fff.log
```

8. The configured `LogFilePath` textbox remains the base path, for example:

```text
D:\Work\CarFight_git\Tools\CFServerLauncher\Logs\server.log
```

9. The actual runtime log file path is shown separately in a read-only UI field.
10. Saving settings does not save the timestamp runtime log path into `server.local.json`.
11. Server start, stop, and restart still work.
12. Normal stop does not add a new last-error message.
13. PID shows `-` after stop.

## Verification

Run these commands in PowerShell:

```powershell
cd D:\Work\CarFight_git
dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln
dotnet run --project .\Tools\CFServerLauncher\src\CFServerLauncher\CFServerLauncher.csproj
```

Manual verification steps:

```text
1. Confirm the app window opens.
2. Click the server EXE browse button and select CarFight_ReServer.exe.
3. Click the working folder browse button and select Server\WindowsServer.
4. Click the log file browse button and choose Logs\server.log as the base path.
5. Click 서버 시작.
6. Confirm TestMap load logs appear.
7. Confirm ActiveLogFilePath shows a timestamp log file.
8. Confirm the LogFilePath textbox still shows the base server.log path.
9. Click 설정 저장.
10. Confirm Config\server.local.json does not contain a timestamp log filename.
11. Click 서버 종료.
12. Confirm PID becomes `-` and no new last-error message appears.
13. Click 서버 시작 again and confirm no file-lock error occurs.
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
```
