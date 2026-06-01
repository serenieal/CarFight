# Codex Task Source: CFServerLauncher v1.0 Stabilize Follow-up Fix

Version: v1.0.6-task  
Date: 2026-05-29  
Executor: Codex  
Work type: C# WPF bug fix

## Goal

Fix the two remaining issues found during review of the previous CFServerLauncher v1.0 stabilization change.

## Target Files

Codex may edit only this file:

```text
Tools/CFServerLauncher/src/CFServerLauncher/ViewModels/MainViewModel.cs
```

## In Scope

1. Update runtime log filename timestamp format to include milliseconds.
2. Distinguish user-requested stop from external server exit when setting final server state.

## Constraints

1. File names and class names must stay under 32 characters.
2. UI labels and user-facing messages must be Korean.
3. Every new C# method must have a short one-line summary comment directly above it.
4. Do not add third-party packages.
5. Do not change Unreal server code.
6. Do not change RCON, build, Cook, Stage, or monitoring behavior.

## Required Fix 1: Runtime log filename milliseconds

Current implementation uses:

```csharp
DateTime.Now.ToString("yyyyMMdd_HHmmss")
```

Change it to include milliseconds:

```csharp
DateTime.Now.ToString("yyyyMMdd_HHmmss_fff")
```

Expected runtime log filename example:

```text
server_20260529_105612_123.log
```

## Required Fix 2: Manual stop state handling

Current `HandleServerExited` always calls:

```csharp
SetState(RunState.Exited);
```

This conflicts with the acceptance criterion:

```text
User clicked 서버 종료 => final state should be 중지됨
External process exit => final state should be 종료됨
```

Implement a simple flag in `MainViewModel`.

Recommended field name:

```text
isStopRequestedByUser
```

Expected behavior:

1. When `StopServerAsync()` starts, set `isStopRequestedByUser = true`.
2. In `HandleServerExited`, if `isStopRequestedByUser` is true, set state to `RunState.Stopped`.
3. If `isStopRequestedByUser` is false, set state to `RunState.Exited`.
4. After handling exit, reset `isStopRequestedByUser = false`.
5. On server start, reset `isStopRequestedByUser = false`.
6. Normal user stop must not write a new LastError.

## Out of Scope

Do not implement these items:

```text
browse button redesign
profile system
auto restart
RCON
web dashboard
Unreal build automation
Cook/Stage automation
CPU/memory monitoring
```

## Acceptance Criteria

1. `dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln` succeeds.
2. Runtime log filename includes milliseconds.
3. User-clicked server stop ends with state `중지됨`.
4. External server exit ends with state `종료됨`.
5. PID becomes `-` after stop or exit.
6. Normal user stop does not create a new LastError.
7. Server can be started again after stop.

## Verification

Run these commands in PowerShell:

```powershell
cd D:\Work\CarFight_git
dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln
dotnet run --project .\Tools\CFServerLauncher\src\CFServerLauncher\CFServerLauncher.csproj
```

Manual verification steps:

```text
1. Start the launcher.
2. Click 서버 시작.
3. Confirm ActiveLogFilePath has a filename like server_yyyyMMdd_HHmmss_fff.log.
4. Click 서버 종료.
5. Confirm state is 중지됨.
6. Confirm PID is -.
7. Confirm LastError is not newly populated by normal stop.
8. Click 서버 시작 again.
9. Close or kill the server process externally.
10. Confirm state is 종료됨.
```
