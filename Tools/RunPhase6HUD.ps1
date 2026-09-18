# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.0
# Date: 2026-09-18
# Description: Phase 6 HUD Presentation TargetPanel additive migration 전용 실행 래퍼
# Scope: RunUIHUDProduction.ps1의 -TargetPhase6Only 경로만 호출하며 다른 Production HUD mutation mode를 노출하지 않습니다.
# Changelog:
# - v1.0.0: WBP_CFTargetPanel exact1 Phase 6 Lock Text/Progress additive migration 실행 경로를 최초 추가.
# Migration:
# - 이 스크립트는 Phase 6 TargetPanel 전용이며 Phase 5 Runtime 또는 Phase 7 Guided Weapon source를 변경하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# [v1.0.0] 현재 래퍼와 기존 Production HUD Runner가 위치한 Tools 디렉터리입니다.
$ToolsDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path

# [v1.0.0] 실제 Phase 6 TargetPanel apply 계약을 소유하는 기존 Production HUD Runner입니다.
$ProductionHUDRunner = Join-Path $ToolsDirectory 'RunUIHUDProduction.ps1'

if (-not (Test-Path -LiteralPath $ProductionHUDRunner -PathType Leaf))
{
    throw "Production HUD runner was not found: $ProductionHUDRunner"
}

& $ProductionHUDRunner -TargetPhase6Only
if ($LASTEXITCODE -ne 0)
{
    throw "Phase 6 HUD Production apply failed with exit code $LASTEXITCODE"
}

exit 0
