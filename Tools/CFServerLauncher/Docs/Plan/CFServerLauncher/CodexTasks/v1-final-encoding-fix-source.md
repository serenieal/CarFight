# Codex Task Source: CFServerLauncher v1.0 Publish Encoding Fix

Version: v1.0.9-task  
Date: 2026-05-29  
Executor: Codex  
Work type: PowerShell script encoding fix

## Goal

Fix mojibake in `Tools/CFServerLauncher/publish.ps1` Korean console messages.

The publish itself succeeds, but Korean `Write-Host` output is garbled when running:

```powershell
cd D:\Work\CarFight_git\Tools\CFServerLauncher
.\publish.ps1
```

Observed garbled examples:

```text
CFServerLauncher v1.0.0 諛고룷 異쒕젰??以鍮꾪빀?덈떎.
湲곗〈 Publish ?대뜑瑜??덈줈 怨좎묩?덈떎.
```

Expected Korean output should be readable.

## Target Files

Codex may edit only this file:

```text
Tools/CFServerLauncher/publish.ps1
```

Codex may update this document only if needed:

```text
Tools/CFServerLauncher/Docs/release.md
```

Do not edit C# source files for this task.

## Root Cause

Likely root cause:

```text
publish.ps1 is saved as UTF-8 without BOM, and Windows PowerShell reads Korean string literals with the wrong code page before Console.OutputEncoding can help.
```

## In Scope

1. Make Korean `Write-Host` messages display correctly in Windows PowerShell and PowerShell 7+.
2. Keep Korean progress messages.
3. Preserve current publish behavior.
4. Keep the script simple.

Recommended fix:

```text
Save Tools/CFServerLauncher/publish.ps1 as UTF-8 with BOM.
```

If Codex cannot explicitly control file encoding, use a safe script-level workaround that preserves readable Korean in Windows PowerShell.

## Required Behavior

`publish.ps1` must still:

1. Set `$ErrorActionPreference = "Stop"`.
2. Run from `Tools/CFServerLauncher` regardless of caller current directory.
3. Create or refresh `Publish`.
4. Run:

```powershell
dotnet publish .\src\CFServerLauncher\CFServerLauncher.csproj -c Release -r win-x64 --self-contained false -o .\Publish
```

5. Not delete `Server`, `Config`, or `Logs`.
6. Not copy server build files automatically.
7. Not require administrator permission.
8. Print readable Korean progress messages.

## Constraints

1. Do not add third-party dependencies.
2. Do not change Unreal server code.
3. Do not change C# app behavior.
4. Do not add packaging, installer, server file copy, or build automation.
5. Keep file names under 32 characters.

## Acceptance Criteria

1. Running `publish.ps1` succeeds.
2. Korean progress messages are readable, not mojibake.
3. `Publish` folder is created or refreshed.
4. `Publish\CFServerLauncher.exe` exists after publish.
5. `Server`, `Config`, and `Logs` folders still exist after publish.
6. Direct publish behavior remains unchanged.

## Verification

Run in Windows PowerShell or PowerShell:

```powershell
cd D:\Work\CarFight_git\Tools\CFServerLauncher
.\publish.ps1
```

Expected readable Korean messages should be similar to:

```text
CFServerLauncher v1.0.0 배포 출력을 준비합니다.
기존 Publish 폴더를 새로 고칩니다.
런처 애플리케이션을 Release/win-x64로 publish합니다.
배포 출력 생성이 완료되었습니다: D:\Work\CarFight_git\Tools\CFServerLauncher\Publish
주의: Server, Config, Logs 폴더와 서버 빌드 파일은 자동으로 포함하거나 복사하지 않습니다.
```

Also confirm:

```text
1. Publish folder exists.
2. Publish\CFServerLauncher.exe exists.
3. Server folder still exists.
4. Config folder still exists.
5. Logs folder still exists.
```

## Out of Scope

Do not implement:

```text
installer creation
server build packaging
server file copy
Unreal build automation
Cook/Stage automation
RCON
web dashboard
profile system
auto restart
monitoring
```
