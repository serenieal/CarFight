# Copyright (c) CarFight. All Rights Reserved.
#
# File: GetUFPIdentity.ps1
# Version: v1.1.0
# Date: 2026-09-05
# Description: GOPY-UE-PROJECT-FASTPATH UFP-01의 Consumer-local measurement identity와 transitive Project source fence를 read-only로 계산합니다.
# Scope: exact9 runner identity, Project-local source/include closure, module/target rules, Engine/Project binary generation, DDC override condition을 freeze evidence로 출력합니다.
# Changelog:
# - v1.1.0: UFP-01 controlled pair용 baseline evidence wrapper와 same-process exact9 batch runner의 syntax/content/exact9 order를 frozen runner identity에 추가.
# - v1.0.0: exact9 test translation unit에서 Project-local quoted include와 same-stem implementation을 재귀 추적하는 source closure + SHA-256 aggregate, runner syntax/name 검증, binary/cache identity를 최초 구현.
# Migration:
# - Product Source/Asset/Config를 수정하지 않습니다. 결과 JSON은 UE/Saved/CarFight/UFPMeasurementIdentity.json에만 기록합니다.
# - UnrealEditor/Automation/PIE/Save를 실행하지 않습니다. 이 스크립트는 measurement preflight read-only evidence 전용입니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

# 현재 CarFight 저장소의 절대 루트입니다.
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $PSScriptRoot '..')).Path

# CarFight Project source root입니다.
$ProjectSourceRoot = Join-Path $RepositoryRoot 'UE\Source'

# Project-local quoted include를 추적할 exact module source roots입니다.
$ModuleSourceRoots = @(
    (Join-Path $ProjectSourceRoot 'CarFight_Re'),
    (Join-Path $ProjectSourceRoot 'CarFight_ReEditor')
)

# exact9가 실제 선언된 두 C++ translation unit입니다.
$TestTranslationUnits = @(
    'UE\Source\CarFight_ReEditor\Private\DataAuthoring\CFVehicleBuilderTests.cpp',
    'UE\Source\CarFight_ReEditor\Private\DataAuthoring\CFVehicleAuthoringVMTests.cpp'
)

# Build/link semantics에 영향을 주는 Project-local module/target rule 후보입니다.
$BuildRuleRelativePaths = @(
    'UE\Source\CarFight_Re\CarFight_Re.Build.cs',
    'UE\Source\CarFight_ReEditor\CarFight_ReEditor.Build.cs',
    'UE\Source\CarFight_Re.Target.cs',
    'UE\Source\CarFight_ReEditor.Target.cs'
)

# UFP exact9 baseline runner입니다.
$BuilderRunnerPath = Join-Path $RepositoryRoot 'Tools\RunBuilderTransTests.ps1'

# exact per-test UnrealEditor runner입니다.
$DataAuthoringRunnerPath = Join-Path $RepositoryRoot 'Tools\RunDataAuthoringTests.ps1'

# controlled process-per-test exact1 evidence wrapper입니다.
$BaselineEvidenceRunnerPath = Join-Path $RepositoryRoot 'Tools\RunUFPBaseline.ps1'

# same-process exact9 batch runner입니다.
$BatchRunnerPath = Join-Path $RepositoryRoot 'Tools\RunUFPBatch.ps1'

# 이 preflight identity 스크립트 자체의 경로입니다.
$IdentityScriptPath = $MyInvocation.MyCommand.Path

# 현재 exact9의 frozen 순서입니다.
$ExpectedTests = @(
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderEvidenceRefresh',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderTransmissionContract',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_05.BuilderProfileCommit',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderShell',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep1Reference',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderEvidenceRefreshVM',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep5Physics',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep7FinalReview',
    'CarFight.DataAuthoring.CF_FQ_040.VB_P0_09.BuilderStep8Driving'
)

# machine-readable preflight 결과 파일입니다.
$ResultJsonPath = Join-Path $RepositoryRoot 'UE\Saved\CarFight\UFPMeasurementIdentity.json'

# 결과 JSON 상위 디렉터리입니다.
$ResultDirectory = Split-Path -Parent $ResultJsonPath

# 절대 경로를 repository-relative Windows 경로로 변환합니다.
function Get-RepositoryRelativePath {
    param(
        # 변환할 절대 파일 경로입니다.
        [Parameter(Mandatory = $true)]
        [string]$FullPath
    )

    # URI 상대경로 계산을 위해 trailing slash를 보장한 repository root입니다.
    $RepositoryUri = [Uri]::new(($RepositoryRoot.TrimEnd('\') + '\'))

    # 상대경로를 계산할 absolute file URI입니다.
    $FileUri = [Uri]::new($FullPath)

    # URI escaping을 제거하고 Windows separator로 정규화한 repository-relative 경로입니다.
    return [Uri]::UnescapeDataString($RepositoryUri.MakeRelativeUri($FileUri).ToString()).Replace('/', '\')
}

# PowerShell script 하나를 parse-only 검증합니다.
function Assert-PowerShellSyntax {
    param(
        # parse-only 검증할 script path입니다.
        [Parameter(Mandatory = $true)]
        [string]$ScriptPath
    )

    # Parser가 반환할 token buffer입니다.
    $Tokens = $null

    # Parser가 반환할 syntax error buffer입니다.
    $ParseErrors = $null

    [System.Management.Automation.Language.Parser]::ParseFile($ScriptPath, [ref]$Tokens, [ref]$ParseErrors) | Out-Null
    if (@($ParseErrors).Count -gt 0) {
        # 첫 parse error를 포함한 fail-closed 설명입니다.
        $FirstParseError = @($ParseErrors)[0].Message
        throw "PowerShell syntax validation failed: $ScriptPath :: $FirstParseError"
    }
}

# 파일의 SHA-256을 lowercase hex로 반환합니다.
function Get-FileSha256 {
    param(
        # 해시할 exact file path입니다.
        [Parameter(Mandatory = $true)]
        [string]$FilePath
    )

    return (Get-FileHash -LiteralPath $FilePath -Algorithm SHA256).Hash.ToLowerInvariant()
}

# 여러 manifest line의 canonical UTF-8 aggregate SHA-256을 계산합니다.
function Get-ManifestSha256 {
    param(
        # 이미 deterministic sort된 manifest lines입니다.
        [Parameter(Mandatory = $true)]
        [string[]]$ManifestLines
    )

    # Aggregate hash 입력으로 사용할 LF-only canonical text입니다.
    $CanonicalManifest = $ManifestLines -join "`n"

    # Canonical manifest UTF-8 bytes입니다.
    $ManifestBytes = [System.Text.UTF8Encoding]::new($false).GetBytes($CanonicalManifest)

    # Aggregate SHA-256 계산기입니다.
    $Sha256 = [System.Security.Cryptography.SHA256]::Create()
    try {
        # Aggregate hash raw bytes입니다.
        $HashBytes = $Sha256.ComputeHash($ManifestBytes)
        return ([System.BitConverter]::ToString($HashBytes)).Replace('-', '').ToLowerInvariant()
    }
    finally {
        $Sha256.Dispose()
    }
}

Assert-PowerShellSyntax -ScriptPath $BuilderRunnerPath
Assert-PowerShellSyntax -ScriptPath $DataAuthoringRunnerPath
Assert-PowerShellSyntax -ScriptPath $BaselineEvidenceRunnerPath
Assert-PowerShellSyntax -ScriptPath $BatchRunnerPath
Assert-PowerShellSyntax -ScriptPath $IdentityScriptPath

# Builder runner 전체 UTF-8 text입니다.
$BuilderRunnerText = [System.IO.File]::ReadAllText($BuilderRunnerPath, [System.Text.UTF8Encoding]::new($false))

# exact9 full test path만 순서대로 추출하는 matcher입니다.
$TestNameMatches = [regex]::Matches($BuilderRunnerText, "'(?<name>CarFight\.DataAuthoring\.CF_FQ_040\.VB_P0_(?:05|09)\.[^']+)'")

# current runner에 실제 선언된 exact test names입니다.
$ActualTests = @($TestNameMatches | ForEach-Object { $_.Groups['name'].Value })

if ($ActualTests.Count -ne $ExpectedTests.Count) {
    throw "Builder Transmission exact test count drift: expected=$($ExpectedTests.Count) actual=$($ActualTests.Count)"
}

for ($TestIndex = 0; $TestIndex -lt $ExpectedTests.Count; $TestIndex++) {
    if ($ActualTests[$TestIndex] -cne $ExpectedTests[$TestIndex]) {
        throw "Builder Transmission exact test order/name drift at index $TestIndex : expected=$($ExpectedTests[$TestIndex]) actual=$($ActualTests[$TestIndex])"
    }
}

if ($BuilderRunnerText -notmatch 'System\.Diagnostics\.Stopwatch' -or $BuilderRunnerText -notmatch 'WORKFLOW_EXECUTION_WALL_MS') {
    throw 'Builder runner does not expose the required Consumer-local Stopwatch workflow wall authority.'
}

# same-process batch runner 전체 UTF-8 text입니다.
$BatchRunnerText = [System.IO.File]::ReadAllText($BatchRunnerPath, [System.Text.UTF8Encoding]::new($false))

# batch runner에 선언된 exact9 full test names입니다.
$BatchTestNameMatches = [regex]::Matches($BatchRunnerText, "'(?<name>CarFight\.DataAuthoring\.CF_FQ_040\.VB_P0_(?:05|09)\.[^']+)'")

# batch runner actual exact test names/order입니다.
$BatchActualTests = @($BatchTestNameMatches | ForEach-Object { $_.Groups['name'].Value })

if ($BatchActualTests.Count -ne $ExpectedTests.Count) {
    throw "UFP batch exact test count drift: expected=$($ExpectedTests.Count) actual=$($BatchActualTests.Count)"
}

for ($BatchTestIndex = 0; $BatchTestIndex -lt $ExpectedTests.Count; $BatchTestIndex++) {
    if ($BatchActualTests[$BatchTestIndex] -cne $ExpectedTests[$BatchTestIndex]) {
        throw "UFP batch exact test order/name drift at index $BatchTestIndex : expected=$($ExpectedTests[$BatchTestIndex]) actual=$($BatchActualTests[$BatchTestIndex])"
    }
}

if (-not $BatchRunnerText.Contains("`$CombinedTestFilter = `$Tests -join '+'") -or $BatchRunnerText -notmatch 'WORKFLOW_EXECUTION_WALL_MS') {
    throw 'UFP batch runner does not expose the frozen plus-separated exact-list + Stopwatch contract.'
}

# Project source index에 포함할 C++ source/header 확장자입니다.
$ProjectSourceExtensions = @('.h', '.hpp', '.inl', '.cpp')

# 두 CarFight module 아래의 Project source 파일 전체 index입니다.
$ProjectSourceFiles = @(
    foreach ($ModuleSourceRoot in $ModuleSourceRoots) {
        if (-not (Test-Path -LiteralPath $ModuleSourceRoot -PathType Container)) {
            throw "Project module source root missing: $ModuleSourceRoot"
        }
        Get-ChildItem -LiteralPath $ModuleSourceRoot -Recurse -File | Where-Object {
            $ProjectSourceExtensions -contains $_.Extension.ToLowerInvariant()
        }
    }
)

# include key 하나가 exact Project source file 목록으로 resolve되는 index입니다.
$IncludeIndex = @{}

foreach ($ProjectSourceFile in $ProjectSourceFiles) {
    # 현재 source file의 repository-relative path입니다.
    $RelativePath = Get-RepositoryRelativePath -FullPath $ProjectSourceFile.FullName

    # include 문법과 비교할 forward-slash relative path입니다.
    $NormalizedRelativePath = $RelativePath.Replace('\', '/')

    # filename-only include를 지원하는 key 목록입니다.
    $IncludeKeys = New-Object System.Collections.Generic.List[string]
    $IncludeKeys.Add($ProjectSourceFile.Name.ToLowerInvariant())

    foreach ($VisibilityMarker in @('/Public/', '/Private/')) {
        # Public/Private 아래 module-relative include suffix 위치입니다.
        $MarkerIndex = $NormalizedRelativePath.IndexOf($VisibilityMarker, [System.StringComparison]::OrdinalIgnoreCase)
        if ($MarkerIndex -ge 0) {
            # Unreal module include가 사용하는 Public/Private 이후 suffix입니다.
            $ModuleIncludeKey = $NormalizedRelativePath.Substring($MarkerIndex + $VisibilityMarker.Length).ToLowerInvariant()
            $IncludeKeys.Add($ModuleIncludeKey)
        }
    }

    foreach ($IncludeKey in @($IncludeKeys | Select-Object -Unique)) {
        if (-not $IncludeIndex.ContainsKey($IncludeKey)) {
            $IncludeIndex[$IncludeKey] = New-Object System.Collections.Generic.List[string]
        }
        $IncludeIndex[$IncludeKey].Add($ProjectSourceFile.FullName)
    }
}

# quoted include 하나를 exact Project-local source file로 resolve합니다.
function Resolve-ProjectInclude {
    param(
        # include 따옴표 내부 문자열입니다.
        [Parameter(Mandatory = $true)]
        [string]$IncludePath
    )

    # Unreal generated header는 source tree가 아니라 UHT output이므로 Project source closure에서 제외합니다.
    if ($IncludePath.EndsWith('.generated.h', [System.StringComparison]::OrdinalIgnoreCase)) {
        return $null
    }

    # include index lookup용 lowercase slash-normalized key입니다.
    $NormalizedIncludeKey = $IncludePath.Replace('\', '/').ToLowerInvariant()

    if (-not $IncludeIndex.ContainsKey($NormalizedIncludeKey)) {
        return $null
    }

    # exact include key에 매칭된 Project source candidates입니다.
    $Candidates = @($IncludeIndex[$NormalizedIncludeKey] | Select-Object -Unique)
    if ($Candidates.Count -gt 1) {
        throw "Ambiguous Project-local include '$IncludePath': $($Candidates -join ', ')"
    }

    return $Candidates[0]
}

# header와 같은 stem의 implementation cpp가 하나 있으면 closure에 포함합니다.
function Resolve-SameStemImplementation {
    param(
        # companion implementation을 찾을 header path입니다.
        [Parameter(Mandatory = $true)]
        [string]$HeaderPath
    )

    # header와 동일한 source stem입니다.
    $SourceStem = [System.IO.Path]::GetFileNameWithoutExtension($HeaderPath)

    # 같은 stem을 가진 Project-local cpp candidates입니다.
    $ImplementationCandidates = @($ProjectSourceFiles | Where-Object {
        $_.Extension -ieq '.cpp' -and [System.IO.Path]::GetFileNameWithoutExtension($_.Name) -ceq $SourceStem
    } | ForEach-Object { $_.FullName })

    if ($ImplementationCandidates.Count -gt 1) {
        # 같은 directory companion을 우선할 수 있는 exact candidates입니다.
        $HeaderDirectory = Split-Path -Parent $HeaderPath
        $SameDirectoryCandidates = @($ImplementationCandidates | Where-Object { (Split-Path -Parent $_) -ceq $HeaderDirectory })
        if ($SameDirectoryCandidates.Count -eq 1) {
            return $SameDirectoryCandidates[0]
        }
        throw "Ambiguous same-stem implementation for '$HeaderPath': $($ImplementationCandidates -join ', ')"
    }

    if ($ImplementationCandidates.Count -eq 1) {
        return $ImplementationCandidates[0]
    }

    return $null
}

# 이미 closure에 포함된 exact full path set입니다.
$VisitedSourcePaths = @{}

# 아직 dependency를 읽어야 할 exact source path queue입니다.
$PendingSourcePaths = New-Object System.Collections.Generic.Queue[string]

foreach ($TestTranslationUnit in $TestTranslationUnits) {
    # exact test translation unit absolute path입니다.
    $TestTranslationUnitPath = Join-Path $RepositoryRoot $TestTranslationUnit
    if (-not (Test-Path -LiteralPath $TestTranslationUnitPath -PathType Leaf)) {
        throw "UFP test translation unit missing: $TestTranslationUnit"
    }
    $PendingSourcePaths.Enqueue((Resolve-Path -LiteralPath $TestTranslationUnitPath).Path)
}

while ($PendingSourcePaths.Count -gt 0) {
    # 이번 iteration에서 dependency를 확장할 exact source file입니다.
    $CurrentSourcePath = $PendingSourcePaths.Dequeue()

    # Windows path case를 무시하는 visited key입니다.
    $VisitedKey = $CurrentSourcePath.ToLowerInvariant()
    if ($VisitedSourcePaths.ContainsKey($VisitedKey)) {
        continue
    }
    $VisitedSourcePaths[$VisitedKey] = $CurrentSourcePath

    # quoted include parser용 current source UTF-8 text입니다.
    $CurrentSourceText = [System.IO.File]::ReadAllText($CurrentSourcePath, [System.Text.UTF8Encoding]::new($false))

    # current source에 선언된 quoted include 목록입니다.
    $QuotedIncludeMatches = [regex]::Matches($CurrentSourceText, '(?m)^\s*#include\s+"(?<path>[^"]+)"')
    foreach ($QuotedIncludeMatch in $QuotedIncludeMatches) {
        # include 따옴표 내부 exact path입니다.
        $QuotedIncludePath = $QuotedIncludeMatch.Groups['path'].Value

        # Project-local include일 때만 exact file path를 반환합니다.
        $ResolvedIncludePath = Resolve-ProjectInclude -IncludePath $QuotedIncludePath
        if ($null -eq $ResolvedIncludePath) {
            continue
        }

        $PendingSourcePaths.Enqueue($ResolvedIncludePath)

        # header implementation semantic drift도 source↔linked-binary correspondence fence에 포함합니다.
        $ResolvedExtension = [System.IO.Path]::GetExtension($ResolvedIncludePath)
        if ($ResolvedExtension -in @('.h', '.hpp', '.inl')) {
            # header와 같은 stem의 exact Project implementation입니다.
            $CompanionImplementationPath = Resolve-SameStemImplementation -HeaderPath $ResolvedIncludePath
            if ($null -ne $CompanionImplementationPath) {
                $PendingSourcePaths.Enqueue($CompanionImplementationPath)
            }
        }
    }
}

# Project source closure에 추가할 existing module/target rules입니다.
$ExistingBuildRulePaths = @(
    foreach ($BuildRuleRelativePath in $BuildRuleRelativePaths) {
        # rule candidate absolute path입니다.
        $BuildRulePath = Join-Path $RepositoryRoot $BuildRuleRelativePath
        if (Test-Path -LiteralPath $BuildRulePath -PathType Leaf) {
            (Resolve-Path -LiteralPath $BuildRulePath).Path
        }
    }
)

# Source/include closure + build/target rules exact full path set입니다.
$FrozenSourcePaths = @($VisitedSourcePaths.Values) + $ExistingBuildRulePaths | Sort-Object -Unique

# Per-file source closure identity rows입니다.
$SourceClosureFiles = @(
    foreach ($FrozenSourcePath in $FrozenSourcePaths) {
        # Repository-relative manifest identity입니다.
        $FrozenRelativePath = Get-RepositoryRelativePath -FullPath $FrozenSourcePath

        # Exact working-copy content SHA-256입니다.
        $FrozenSha256 = Get-FileSha256 -FilePath $FrozenSourcePath

        [ordered]@{
            path = $FrozenRelativePath
            sha256 = $FrozenSha256
        }
    }
)

# Aggregate hash input은 path|sha256 순서로 canonical 정렬합니다.
$SourceManifestLines = @($SourceClosureFiles | ForEach-Object { "$($_.path)|$($_.sha256)" } | Sort-Object)

# Current transitive Project source closure aggregate identity입니다.
$SourceClosureSha256 = Get-ManifestSha256 -ManifestLines $SourceManifestLines

# Project-local linked module manifest입니다.
$ModulesPath = Join-Path $RepositoryRoot 'UE\Binaries\Win64\UnrealEditor.modules'
if (-not (Test-Path -LiteralPath $ModulesPath -PathType Leaf)) {
    throw "Project UnrealEditor.modules missing: $ModulesPath"
}

# linked module manifest JSON입니다.
$ModulesJson = [System.IO.File]::ReadAllText($ModulesPath, [System.Text.UTF8Encoding]::new($false)) | ConvertFrom-Json

# Runtime Project DLL입니다.
$RuntimeDllPath = Join-Path $RepositoryRoot 'UE\Binaries\Win64\UnrealEditor-CarFight_Re.dll'

# Editor Project DLL입니다.
$EditorDllPath = Join-Path $RepositoryRoot 'UE\Binaries\Win64\UnrealEditor-CarFight_ReEditor.dll'

foreach ($RequiredBinaryPath in @($RuntimeDllPath, $EditorDllPath)) {
    if (-not (Test-Path -LiteralPath $RequiredBinaryPath -PathType Leaf)) {
        throw "Required Project binary missing: $RequiredBinaryPath"
    }
}

# Runtime DLL current filesystem generation입니다.
$RuntimeDllInfo = Get-Item -LiteralPath $RuntimeDllPath

# Editor DLL current filesystem generation입니다.
$EditorDllInfo = Get-Item -LiteralPath $EditorDllPath

# CarFight official UE 5.8 Source Build Editor executable입니다.
$EngineEditorPath = 'D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe'

# Engine Build.version metadata path입니다.
$EngineBuildVersionPath = 'D:\UnrealEngine_Source\Engine\Build\Build.version'

foreach ($RequiredEnginePath in @($EngineEditorPath, $EngineBuildVersionPath)) {
    if (-not (Test-Path -LiteralPath $RequiredEnginePath -PathType Leaf)) {
        throw "Required UE 5.8 engine identity file missing: $RequiredEnginePath"
    }
}

# Current linked UnrealEditor.exe filesystem generation입니다.
$EngineEditorInfo = Get-Item -LiteralPath $EngineEditorPath

# Engine Build.version content SHA-256입니다.
$EngineBuildVersionSha256 = Get-FileSha256 -FilePath $EngineBuildVersionPath

# CarFight Config에서 DDC/Zen override 후보를 찾을 exact patterns입니다.
$DdcOverridePattern = 'DerivedDataCache|SharedDataCachePath|LocalDerivedDataCache|\bDDC\b|\bZen\b'

# Current Config의 explicit DDC/Zen override lines입니다.
$DdcOverrideMatches = @(
    $ConfigRoot = Join-Path $RepositoryRoot 'UE\Config'
    if (Test-Path -LiteralPath $ConfigRoot -PathType Container) {
        foreach ($ConfigFile in Get-ChildItem -LiteralPath $ConfigRoot -Recurse -File -Filter '*.ini') {
            # Config는 UTF-8 explicit read만 사용합니다.
            $ConfigText = [System.IO.File]::ReadAllText($ConfigFile.FullName, [System.Text.UTF8Encoding]::new($false))
            foreach ($ConfigLine in ($ConfigText -split "`r?`n")) {
                if ($ConfigLine -match $DdcOverridePattern) {
                    "$(Get-RepositoryRelativePath -FullPath $ConfigFile.FullName):$ConfigLine"
                }
            }
        }
    }
)

# Runner identity rows입니다.
$RunnerIdentity = [ordered]@{
    builder_runner_path = Get-RepositoryRelativePath -FullPath $BuilderRunnerPath
    builder_runner_sha256 = Get-FileSha256 -FilePath $BuilderRunnerPath
    data_authoring_runner_path = Get-RepositoryRelativePath -FullPath $DataAuthoringRunnerPath
    data_authoring_runner_sha256 = Get-FileSha256 -FilePath $DataAuthoringRunnerPath
    baseline_evidence_runner_path = Get-RepositoryRelativePath -FullPath $BaselineEvidenceRunnerPath
    baseline_evidence_runner_sha256 = Get-FileSha256 -FilePath $BaselineEvidenceRunnerPath
    batch_runner_path = Get-RepositoryRelativePath -FullPath $BatchRunnerPath
    batch_runner_sha256 = Get-FileSha256 -FilePath $BatchRunnerPath
    batch_command_mode = 'RunTests plus-separated exact full paths'
    identity_script_path = Get-RepositoryRelativePath -FullPath $IdentityScriptPath
    identity_script_sha256 = Get-FileSha256 -FilePath $IdentityScriptPath
    workflow_timer = 'System.Diagnostics.Stopwatch'
    workflow_timer_high_resolution = [System.Diagnostics.Stopwatch]::IsHighResolution
    workflow_timer_frequency = [System.Diagnostics.Stopwatch]::Frequency
    log_suppression_mode = 'OFF'
}

# Measurement source closure identity입니다.
$SourceClosureIdentity = [ordered]@{
    root_translation_units = $TestTranslationUnits
    closure_rule = 'quoted Project include recursion + same-stem implementation + module/target rules'
    file_count = $SourceClosureFiles.Count
    sha256 = $SourceClosureSha256
    files = $SourceClosureFiles
}

# Engine + linked Project binary identity입니다.
$BinaryIdentity = [ordered]@{
    engine_root = 'D:\UnrealEngine_Source'
    engine_editor_path = $EngineEditorPath
    engine_editor_size = [int64]$EngineEditorInfo.Length
    engine_editor_mtime_utc_ticks = $EngineEditorInfo.LastWriteTimeUtc.Ticks
    engine_build_version_sha256 = $EngineBuildVersionSha256
    project_modules_build_id = [string]$ModulesJson.BuildId
    project_modules_sha256 = Get-FileSha256 -FilePath $ModulesPath
    runtime_dll_size = [int64]$RuntimeDllInfo.Length
    runtime_dll_mtime_utc_ticks = $RuntimeDllInfo.LastWriteTimeUtc.Ticks
    editor_dll_size = [int64]$EditorDllInfo.Length
    editor_dll_mtime_utc_ticks = $EditorDllInfo.LastWriteTimeUtc.Ticks
}

# UFP-01 measurement identity 결과입니다.
$Result = [ordered]@{
    schema_version = 'carfight_ufp_measurement_identity_v1'
    status = 'success'
    exact_tests = $ActualTests
    batch_exact_tests = $BatchActualTests
    runner = $RunnerIdentity
    source_closure = $SourceClosureIdentity
    binary = $BinaryIdentity
    cache = [ordered]@{
        mode = 'natural_host_local'
        deliberate_clear = $false
        deliberate_prewarm = $false
        explicit_ddc_or_zen_override_count = $DdcOverrideMatches.Count
        explicit_ddc_or_zen_overrides = $DdcOverrideMatches
    }
}

[System.IO.Directory]::CreateDirectory($ResultDirectory) | Out-Null

# UTF-8 without BOM machine-readable identity JSON입니다.
$ResultJson = $Result | ConvertTo-Json -Depth 10
[System.IO.File]::WriteAllText($ResultJsonPath, $ResultJson, [System.Text.UTF8Encoding]::new($false))

Write-Output 'UFP_MEASUREMENT_IDENTITY=PASS'
Write-Output "UFP_EXACT_TEST_COUNT=$($ActualTests.Count)"
Write-Output "UFP_SOURCE_CLOSURE_COUNT=$($SourceClosureFiles.Count)"
Write-Output "UFP_SOURCE_CLOSURE_SHA256=$SourceClosureSha256"
Write-Output "UFP_PROJECT_BUILD_ID=$($ModulesJson.BuildId)"
Write-Output "UFP_DDC_OVERRIDE_COUNT=$($DdcOverrideMatches.Count)"
Write-Output "RESULT_JSON=$ResultJsonPath"
exit 0
