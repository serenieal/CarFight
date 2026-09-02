# Copyright (c) CarFight. All Rights Reserved.
#
# File: CalcWagonShift.ps1
# Version: v1.1.0
# Date: 2026-09-01
# Description: CF-FQ-040 ESH-03 PhysicsDraft v3 Engine Curve + 8AT ratio로 WheelTorqueCrossoverShift@1을 독립 계산하는 read-only cross-check입니다.
# Changelog:
# - v1.1.0: 1RPM 전수 PowerShell 함수호출을 제거하고 10RPM coarse scan + ±20RPM 1RPM local refine으로 경량화. 결과 정의는 v1.0.0과 동일.
# - v1.0.0: adjacent pair crossover, least-squares common ChangeUpRPM, current WSA radius 기준 theoretical speed, current ChangeDownRPM reentry diagnostic을 최초 구현.
# Migration:
# - Product Asset/Profile/Recipe/Target을 수정하거나 저장하지 않습니다.
# - 현재 WSA radius 39.999607cm는 fresh persisted Target evidence의 read-only diagnostic input이며 authority 자체를 변경하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path
$DraftPath = Join-Path $RepositoryRoot 'UE\Saved\CarFight\VehicleBuilder\PhysicsDraft.json'
$OutputPath = Join-Path $RepositoryRoot 'UE\Saved\CarFight\VehicleBuilder\WagonShiftPreview_Offline.json'

if (-not (Test-Path -LiteralPath $DraftPath -PathType Leaf)) {
    throw "PhysicsDraft.json not found: $DraftPath"
}

$Draft = Get-Content -LiteralPath $DraftPath -Raw -Encoding UTF8 | ConvertFrom-Json
if ([int]$Draft.SchemaRevision -ne 3) {
    throw "ESH-03 requires PhysicsDraft schema v3. Current=$($Draft.SchemaRevision)"
}

$Performance = $Draft.ProfilePayload.PerformanceData
$Drivetrain = $Draft.ProfilePayload.DrivetrainData
if (-not [bool]$Performance.bUseEngineTorqueCurve) {
    throw 'Vehicle-specific Engine TorqueCurve is disabled.'
}

$CurvePoints = @($Performance.EngineTorqueCurve.Points)
$Ratios = @($Drivetrain.TransmissionRatios.ForwardGearRatios | ForEach-Object { [double]$_ })
$MaxTorqueNm = [double]$Performance.EngineMaxTorqueByFeel.NeutralValue
$EngineIdleRPM = [double]$Performance.EngineIdleRPM
$EngineMaxRPM = [double]$Performance.EngineMaxRPMByFeel.NeutralValue
$FinalRatio = [double]$Drivetrain.FinalRatio
$Efficiency = [double]$Drivetrain.TransmissionEfficiency
$CurrentChangeDownRPM = [double]$Drivetrain.ChangeDownRPM

if ($CurvePoints.Count -lt 2 -or $Ratios.Count -lt 2 -or $MaxTorqueNm -le 0 -or $FinalRatio -le 0 -or $Efficiency -le 0) {
    throw 'Invalid ESH-03 input payload.'
}

function Get-TorqueMultiplier {
    param([double]$Rpm)

    if ($Rpm -le [double]$CurvePoints[0].EngineRPM) {
        return [double]$CurvePoints[0].TorqueMultiplier
    }
    if ($Rpm -ge [double]$CurvePoints[-1].EngineRPM) {
        return [double]$CurvePoints[-1].TorqueMultiplier
    }

    for ($Index = 0; $Index -lt $CurvePoints.Count - 1; ++$Index) {
        $Left = $CurvePoints[$Index]
        $Right = $CurvePoints[$Index + 1]
        $LeftRpm = [double]$Left.EngineRPM
        $RightRpm = [double]$Right.EngineRPM
        if ($Rpm -lt $LeftRpm -or $Rpm -gt $RightRpm) {
            continue
        }

        $Alpha = ($Rpm - $LeftRpm) / ($RightRpm - $LeftRpm)
        return [double]$Left.TorqueMultiplier + (([double]$Right.TorqueMultiplier - [double]$Left.TorqueMultiplier) * $Alpha)
    }
    return [double]$CurvePoints[-1].TorqueMultiplier
}

function Get-WheelTorqueNm {
    param(
        [double]$Rpm,
        [double]$GearRatio
    )

    $EngineTorqueNm = $MaxTorqueNm * (Get-TorqueMultiplier -Rpm $Rpm)
    return $EngineTorqueNm * $GearRatio * $FinalRatio * $Efficiency
}

function Get-RelativeGap {
    param(
        [double]$CurrentTorque,
        [double]$NextTorque
    )

    $Larger = [Math]::Max($CurrentTorque, $NextTorque)
    if ($Larger -le 1e-9) {
        return 0.0
    }
    return [Math]::Abs($CurrentTorque - $NextTorque) / $Larger
}

function Get-SpeedKmh {
    param(
        [double]$Rpm,
        [double]$GearRatio
    )

    # Fresh Target/WSA readback radius입니다. ESH-03 offline cross-check에서만 사용합니다.
    $WheelRadiusCm = 39.999607
    $OverallRatio = $GearRatio * $FinalRatio
    return $Rpm * 2.0 * [Math]::PI * $WheelRadiusCm * 60.0 / ($OverallRatio * 100000.0)
}

$SearchStartRPM = [Math]::Ceiling([Math]::Max($EngineIdleRPM, [double]$CurvePoints[0].EngineRPM))
$SearchEndRPM = [Math]::Floor($EngineMaxRPM)

$PairResults = @()
for ($GearIndex = 0; $GearIndex -lt $Ratios.Count - 1; ++$GearIndex) {
    $CurrentRatio = $Ratios[$GearIndex]
    $NextRatio = $Ratios[$GearIndex + 1]
    $Found = $false
    $CrossoverRPM = [int]$SearchEndRPM

    # PowerShell에서는 10RPM coarse scan으로 crossing bracket을 찾고 bracket 내부만 1RPM refine합니다.
    $PreviousRPM = [int]$SearchStartRPM
    $PreviousDifference = (Get-WheelTorqueNm -Rpm $PreviousRPM -GearRatio $CurrentRatio) - (Get-WheelTorqueNm -Rpm ([double]$PreviousRPM * $NextRatio / $CurrentRatio) -GearRatio $NextRatio)
    for ($CandidateRPM = [int]$SearchStartRPM + 10; $CandidateRPM -le [int]$SearchEndRPM; $CandidateRPM += 10) {
        $CurrentWheelTorque = Get-WheelTorqueNm -Rpm $CandidateRPM -GearRatio $CurrentRatio
        $PostShiftRPM = [double]$CandidateRPM * $NextRatio / $CurrentRatio
        $NextWheelTorque = Get-WheelTorqueNm -Rpm $PostShiftRPM -GearRatio $NextRatio
        $Difference = $CurrentWheelTorque - $NextWheelTorque
        if ($Difference -le 1e-4) {
            $RefineStart = [Math]::Max([int]$SearchStartRPM, $PreviousRPM)
            $RefineEnd = [Math]::Min([int]$SearchEndRPM, $CandidateRPM)
            for ($RefineRPM = $RefineStart; $RefineRPM -le $RefineEnd; ++$RefineRPM) {
                $RefineCurrent = Get-WheelTorqueNm -Rpm $RefineRPM -GearRatio $CurrentRatio
                $RefinePostRPM = [double]$RefineRPM * $NextRatio / $CurrentRatio
                $RefineNext = Get-WheelTorqueNm -Rpm $RefinePostRPM -GearRatio $NextRatio
                if ($RefineNext + 1e-4 -ge $RefineCurrent) {
                    $Found = $true
                    $CrossoverRPM = $RefineRPM
                    break
                }
            }
            break
        }
        $PreviousRPM = $CandidateRPM
        $PreviousDifference = $Difference
    }

    $CrossoverPostRPM = [double]$CrossoverRPM * $NextRatio / $CurrentRatio
    $PairResults += [ordered]@{
        FromGear = $GearIndex + 1
        ToGear = $GearIndex + 2
        CrossoverFound = $Found
        EngineMaxRPMLimited = -not $Found
        CrossoverRPM = $CrossoverRPM
        CrossoverPostShiftRPM = $CrossoverPostRPM
        CrossoverCurrentWheelTorqueNm = Get-WheelTorqueNm -Rpm $CrossoverRPM -GearRatio $CurrentRatio
        CrossoverNextWheelTorqueNm = Get-WheelTorqueNm -Rpm $CrossoverPostRPM -GearRatio $NextRatio
        CrossoverSpeedKmh = Get-SpeedKmh -Rpm $CrossoverRPM -GearRatio $CurrentRatio
    }
}

$BestRPM = [int]$SearchStartRPM
$BestMeanSquaredGap = [double]::PositiveInfinity
$BestMaxGap = [double]::PositiveInfinity

function Test-CommonShiftCandidate {
    param([int]$CandidateRPM)
    $SumSquared = 0.0
    $MaxGap = 0.0
    for ($GearIndex = 0; $GearIndex -lt $Ratios.Count - 1; ++$GearIndex) {
        $CurrentRatio = $Ratios[$GearIndex]
        $NextRatio = $Ratios[$GearIndex + 1]
        $CurrentWheelTorque = Get-WheelTorqueNm -Rpm $CandidateRPM -GearRatio $CurrentRatio
        $PostShiftRPM = [double]$CandidateRPM * $NextRatio / $CurrentRatio
        $NextWheelTorque = Get-WheelTorqueNm -Rpm $PostShiftRPM -GearRatio $NextRatio
        $Gap = Get-RelativeGap -CurrentTorque $CurrentWheelTorque -NextTorque $NextWheelTorque
        $SumSquared += $Gap * $Gap
        $MaxGap = [Math]::Max($MaxGap, $Gap)
    }
    $MeanSquared = $SumSquared / [double]($Ratios.Count - 1)
    return @([double]$MeanSquared, [double]$MaxGap)
}

# 10RPM coarse search로 optimum neighborhood를 찾습니다.
for ($CandidateRPM = [int]$SearchStartRPM; $CandidateRPM -le [int]$SearchEndRPM; $CandidateRPM += 10) {
    $Score = Test-CommonShiftCandidate -CandidateRPM $CandidateRPM
    if ([double]$Score[0] -lt $BestMeanSquaredGap - 1e-12) {
        $BestRPM = $CandidateRPM
        $BestMeanSquaredGap = [double]$Score[0]
        $BestMaxGap = [double]$Score[1]
    }
}

# Coarse winner 주변 ±20RPM만 1RPM 단위로 refine해서 v1.0.0 exact integer optimum을 보존합니다.
$RefineStartRPM = [Math]::Max([int]$SearchStartRPM, $BestRPM - 20)
$RefineEndRPM = [Math]::Min([int]$SearchEndRPM, $BestRPM + 20)
for ($CandidateRPM = $RefineStartRPM; $CandidateRPM -le $RefineEndRPM; ++$CandidateRPM) {
    $Score = Test-CommonShiftCandidate -CandidateRPM $CandidateRPM
    if ([double]$Score[0] -lt $BestMeanSquaredGap - 1e-12) {
        $BestRPM = $CandidateRPM
        $BestMeanSquaredGap = [double]$Score[0]
        $BestMaxGap = [double]$Score[1]
    }
}

$RecommendedPairs = @()
$DownshiftPairs = @()
# Current ChangeDownRPM review의 bounded aggregate입니다.
$MinimumPostDownshiftTorqueMultiplier = [double]::PositiveInfinity
$MaximumPostDownshiftRPM = 0.0
for ($GearIndex = 0; $GearIndex -lt $Ratios.Count - 1; ++$GearIndex) {
    $CurrentRatio = $Ratios[$GearIndex]
    $NextRatio = $Ratios[$GearIndex + 1]
    $CurrentWheelTorque = Get-WheelTorqueNm -Rpm $BestRPM -GearRatio $CurrentRatio
    $PostShiftRPM = [double]$BestRPM * $NextRatio / $CurrentRatio
    $NextWheelTorque = Get-WheelTorqueNm -Rpm $PostShiftRPM -GearRatio $NextRatio
    $RecommendedPairs += [ordered]@{
        FromGear = $GearIndex + 1
        ToGear = $GearIndex + 2
        RecommendedChangeUpRPM = $BestRPM
        RecommendedSpeedKmh = Get-SpeedKmh -Rpm $BestRPM -GearRatio $CurrentRatio
        PostShiftRPM = $PostShiftRPM
        CurrentWheelTorqueNm = $CurrentWheelTorque
        NextWheelTorqueNm = $NextWheelTorque
        RelativeTorqueGap = Get-RelativeGap -CurrentTorque $CurrentWheelTorque -NextTorque $NextWheelTorque
    }

    # 현재 2000RPM ChangeDown을 별도 재검증합니다. 다음 gear(고단)에서 이전 gear(저단)로 내려간 직후 RPM입니다.
    $PostDownshiftRPM = $CurrentChangeDownRPM * $CurrentRatio / $NextRatio
    $PostDownshiftTorqueMultiplier = Get-TorqueMultiplier -Rpm $PostDownshiftRPM
    $MinimumPostDownshiftTorqueMultiplier = [Math]::Min($MinimumPostDownshiftTorqueMultiplier, $PostDownshiftTorqueMultiplier)
    $MaximumPostDownshiftRPM = [Math]::Max($MaximumPostDownshiftRPM, $PostDownshiftRPM)
    $DownshiftPairs += [ordered]@{
        FromGear = $GearIndex + 2
        ToGear = $GearIndex + 1
        CurrentChangeDownRPM = $CurrentChangeDownRPM
        PostDownshiftRPM = $PostDownshiftRPM
        PostDownshiftTorqueMultiplier = $PostDownshiftTorqueMultiplier
        BelowRecommendedChangeUpRPM = $PostDownshiftRPM -lt $BestRPM
        BelowEngineMaxRPM = $PostDownshiftRPM -lt $EngineMaxRPM
    }
}

$Result = [ordered]@{
    Schema = 'WagonShiftPreviewOffline_v1'
    MethodId = 'WheelTorqueCrossoverShift'
    MethodRevision = 1
    InputPhysicsDraftSchema = [int]$Draft.SchemaRevision
    EngineCurveMethod = [string]$Draft.EngineCurveReview.MethodId
    SearchStartRPM = [int]$SearchStartRPM
    SearchEndRPM = [int]$SearchEndRPM
    RecommendedChangeUpRPM = $BestRPM
    MeanSquaredRelativeTorqueGap = $BestMeanSquaredGap
    MaxRelativeTorqueGap = $BestMaxGap
    PairCrossovers = $PairResults
    RecommendedPairs = $RecommendedPairs
    CurrentChangeDownRPMReview = [ordered]@{
        SelectedRPM = $CurrentChangeDownRPM
        Disposition = 'GAME_BIAS_REVIEW_ONLY'
        PairResults = $DownshiftPairs
        MinimumPostDownshiftTorqueMultiplier = $MinimumPostDownshiftTorqueMultiplier
        MaximumPostDownshiftRPM = $MaximumPostDownshiftRPM
        Note = '2000RPM is not derived from ChangeUpRPM. It is reviewed separately for high-torque-band re-entry and no immediate upshift/redline violation.'
    }
    ProductAssetMutation = $false
    SavePerformed = $false
}

$Result | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $OutputPath -Encoding UTF8

Write-Output "ESH03_OFFLINE_RECOMMENDED_CHANGE_UP_RPM=$BestRPM"
Write-Output ("ESH03_OFFLINE_MSE={0:F9}" -f $BestMeanSquaredGap)
Write-Output ("ESH03_OFFLINE_MAX_GAP={0:F9}" -f $BestMaxGap)
foreach ($Pair in $PairResults) {
    Write-Output ("ESH03_PAIR_{0}_{1}=RPM:{2};FOUND:{3};POST:{4:F2};SPEED:{5:F2}" -f $Pair.FromGear,$Pair.ToGear,$Pair.CrossoverRPM,$Pair.CrossoverFound,$Pair.CrossoverPostShiftRPM,$Pair.CrossoverSpeedKmh)
}
foreach ($Pair in $RecommendedPairs) {
    Write-Output ("ESH03_COMMON_{0}_{1}=SPEED:{2:F2};POST:{3:F2};GAP:{4:F6}" -f $Pair.FromGear,$Pair.ToGear,$Pair.RecommendedSpeedKmh,$Pair.PostShiftRPM,$Pair.RelativeTorqueGap)
}
Write-Output ("ESH03_CURRENT_DOWN_MIN_TORQUE_MULT={0:F6}" -f $Result.CurrentChangeDownRPMReview.MinimumPostDownshiftTorqueMultiplier)
Write-Output ("ESH03_CURRENT_DOWN_MAX_POST_RPM={0:F2}" -f $Result.CurrentChangeDownRPMReview.MaximumPostDownshiftRPM)
Write-Output "ESH03_PRODUCT_ASSET_MUTATION=false"
Write-Output "ESH03_SAVE_PERFORMED=false"
exit 0
