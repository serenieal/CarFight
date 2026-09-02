# Copyright (c) CarFight. All Rights Reserved.
# File: MakeVT12Proto.ps1
# Version: v1.0.0
# Date: 2026-08-28
# Description: VT-VEH-01 reference composition을 기준으로 실제 Production Source/Runtime 구조만 조립한 896x416 VehiclePanel Prototype Review를 생성합니다.
# Changelog:
# - v1.0.0: Speed/RPM hierarchy + target-direction Armor 6-sector composition + existing Defense family를 실제 Source만으로 조립.
# Migration:
# - Review-only prototype입니다. UE Import/Asset/Designer mutation 0입니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Drawing

$ToolsDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepositoryRoot = (Resolve-Path -LiteralPath (Join-Path $ToolsDirectory '..')).Path
$SourceRoot = Join-Path $RepositoryRoot 'SourceArt\UI\HUD\VT'

$ReferencePath = Join-Path $SourceRoot 'VT_VehPanel01.jpg'
$FramePath = Join-Path $SourceRoot 'VT11_FrameOverlay_Clean.png'
$SpeedFramePath = Join-Path $SourceRoot 'VT11_SpeedFrame_Clean.png'
$ArmorFramePath = Join-Path $SourceRoot 'VT11_ArmorFrame_Clean.png'
$RpmMaskPath = Join-Path $SourceRoot 'T_UI_RPMMaskPack.png'
$SilhouettePath = Join-Path $SourceRoot 'VT05_VehSil_Sedan.png'
$ArmorPlatePath = Join-Path $SourceRoot 'VT04_ArmorPlate.png'
$ArmorArrowPath = Join-Path $SourceRoot 'VT04_ArmorIcon_Arrow.png'
$ArmorChevronPath = Join-Path $SourceRoot 'VT04_ArmorIcon_Chevron2.png'
$DefenseFramePath = Join-Path $SourceRoot 'VT06_DefBarFrame.png'
$DefenseFillMaskPath = Join-Path $SourceRoot 'VT06_DefFillMask.png'
$DefenseHexPath = Join-Path $SourceRoot 'VT06_DefHex.png'

$PrototypePath = Join-Path $SourceRoot 'VT12_VehPanelPrototype.png'
$ComparePath = Join-Path $SourceRoot 'VT12_ReferenceCompare.png'

$RequiredPaths = @(
    $ReferencePath,
    $FramePath,
    $SpeedFramePath,
    $ArmorFramePath,
    $RpmMaskPath,
    $SilhouettePath,
    $ArmorPlatePath,
    $ArmorArrowPath,
    $ArmorChevronPath,
    $DefenseFramePath,
    $DefenseFillMaskPath,
    $DefenseHexPath
)

foreach ($RequiredPath in $RequiredPaths) {
    if (-not (Test-Path -LiteralPath $RequiredPath -PathType Leaf)) {
        throw "VT12 source missing: $RequiredPath"
    }
}

function Set-VT12GraphicsQuality {
    param([Parameter(Mandatory = $true)][System.Drawing.Graphics]$Graphics)

    $Graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $Graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $Graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $Graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
}

function Draw-VT12TintedImage {
    param(
        [Parameter(Mandatory = $true)][System.Drawing.Graphics]$Graphics,
        [Parameter(Mandatory = $true)][System.Drawing.Image]$Image,
        [Parameter(Mandatory = $true)][System.Drawing.Rectangle]$Destination,
        [Parameter(Mandatory = $true)][System.Drawing.Color]$TintColor,
        [double]$Opacity = 1.0
    )

    $Matrix = New-Object System.Drawing.Imaging.ColorMatrix
    $Matrix.Matrix00 = [single]($TintColor.R / 255.0)
    $Matrix.Matrix11 = [single]($TintColor.G / 255.0)
    $Matrix.Matrix22 = [single]($TintColor.B / 255.0)
    $Matrix.Matrix33 = [single][Math]::Max(0.0, [Math]::Min(1.0, $Opacity))
    $Matrix.Matrix44 = 1.0

    $Attributes = New-Object System.Drawing.Imaging.ImageAttributes
    $Attributes.SetColorMatrix($Matrix)

    try {
        $Graphics.DrawImage(
            $Image,
            $Destination,
            0,
            0,
            $Image.Width,
            $Image.Height,
            [System.Drawing.GraphicsUnit]::Pixel,
            $Attributes
        )
    }
    finally {
        $Attributes.Dispose()
    }
}

function New-VT12RpmPreview {
    param(
        [Parameter(Mandatory = $true)][string]$MaskPath,
        [Parameter(Mandatory = $true)][double]$Ratio
    )

    $Source = [System.Drawing.Bitmap]::FromFile($MaskPath)
    $PreviewWidth = 346
    $PreviewHeight = 223

    $ScaledMask = New-Object System.Drawing.Bitmap($PreviewWidth, $PreviewHeight, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    $ScaleGraphics = [System.Drawing.Graphics]::FromImage($ScaledMask)
    $ScaleGraphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::NearestNeighbor
    $ScaleGraphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::Half
    $ScaleGraphics.DrawImage($Source, 0, 0, $PreviewWidth, $PreviewHeight)
    $ScaleGraphics.Dispose()
    $Source.Dispose()

    $Preview = New-Object System.Drawing.Bitmap($PreviewWidth, $PreviewHeight, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)

    $ActiveColor = [System.Drawing.Color]::FromArgb(248, 38, 218, 239)
    $InactiveColor = [System.Drawing.Color]::FromArgb(105, 59, 80, 90)
    $RedlineColor = [System.Drawing.Color]::FromArgb(244, 231, 71, 66)
    $GuideColor = [System.Drawing.Color]::FromArgb(92, 27, 92, 106)

    for ($Y = 0; $Y -lt $PreviewHeight; $Y++) {
        for ($X = 0; $X -lt $PreviewWidth; $X++) {
            $Pixel = $ScaledMask.GetPixel($X, $Y)
            $ProgressPosition = $Pixel.R / 255.0
            $TickStrength = $Pixel.G / 255.0
            $GuideStrength = $Pixel.B / 255.0

            if ($TickStrength -gt 0.08) {
                $IsActive = $ProgressPosition -le ($Ratio + 0.015)
                $IsRedline = $IsActive -and $ProgressPosition -ge 0.85
                $BaseColor = if ($IsRedline) { $RedlineColor } elseif ($IsActive) { $ActiveColor } else { $InactiveColor }
                $Alpha = [int][Math]::Round($BaseColor.A * $TickStrength)
                $Preview.SetPixel($X, $Y, [System.Drawing.Color]::FromArgb($Alpha, $BaseColor.R, $BaseColor.G, $BaseColor.B))
            }
            elseif ($GuideStrength -gt 0.06) {
                $Alpha = [int][Math]::Round($GuideColor.A * $GuideStrength)
                $Preview.SetPixel($X, $Y, [System.Drawing.Color]::FromArgb($Alpha, $GuideColor.R, $GuideColor.G, $GuideColor.B))
            }
            else {
                $Preview.SetPixel($X, $Y, [System.Drawing.Color]::Transparent)
            }
        }
    }

    $ScaledMask.Dispose()
    return $Preview
}

function Draw-VT12ArmorSector {
    param(
        [Parameter(Mandatory = $true)][System.Drawing.Graphics]$Graphics,
        [Parameter(Mandatory = $true)][System.Drawing.Image]$PlateImage,
        [Parameter(Mandatory = $true)][System.Drawing.Image]$IconImage,
        [Parameter(Mandatory = $true)][int]$CenterX,
        [Parameter(Mandatory = $true)][int]$CenterY,
        [Parameter(Mandatory = $true)][double]$RotationDegrees,
        [Parameter(Mandatory = $true)][double]$Ratio,
        [Parameter(Mandatory = $true)][System.Drawing.Color]$AccentColor
    )

    $PlateWidth = 52
    $PlateHeight = 69
    $PlateX = [int]($CenterX - ($PlateWidth / 2))
    $PlateY = [int]($CenterY - ($PlateHeight / 2))

    Draw-VT12TintedImage -Graphics $Graphics -Image $PlateImage -Destination (New-Object System.Drawing.Rectangle($PlateX, $PlateY, $PlateWidth, $PlateHeight)) -TintColor ([System.Drawing.Color]::FromArgb(255, 211, 178, 113)) -Opacity 0.80

    # Production contract의 vertical Ratio Bar를 Prototype에서도 그대로 모사합니다.
    $TrackX = $PlateX + 40
    $TrackY = $PlateY + 11
    $TrackHeight = 42
    $TrackBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(190, 13, 21, 25))
    $FillBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(242, $AccentColor.R, $AccentColor.G, $AccentColor.B))

    try {
        $Graphics.FillRectangle($TrackBrush, $TrackX, $TrackY, 4, $TrackHeight)
        $FillHeight = [int][Math]::Round($TrackHeight * [Math]::Max(0.0, [Math]::Min(1.0, $Ratio)))
        if ($FillHeight -gt 0) {
            $Graphics.FillRectangle($FillBrush, $TrackX, $TrackY + ($TrackHeight - $FillHeight), 4, $FillHeight)
        }
    }
    finally {
        $TrackBrush.Dispose()
        $FillBrush.Dispose()
    }

    $State = $Graphics.Save()
    try {
        $Graphics.TranslateTransform([single]($CenterX - 4), [single]($CenterY - 4))
        $Graphics.RotateTransform([single]$RotationDegrees)
        Draw-VT12TintedImage -Graphics $Graphics -Image $IconImage -Destination (New-Object System.Drawing.Rectangle(-14, -14, 28, 28)) -TintColor $AccentColor -Opacity 0.97
    }
    finally {
        $Graphics.Restore($State)
    }
}

function Draw-VT12DefenseRow {
    param(
        [Parameter(Mandatory = $true)][System.Drawing.Graphics]$Graphics,
        [Parameter(Mandatory = $true)][System.Drawing.Image]$FrameImage,
        [Parameter(Mandatory = $true)][System.Drawing.Image]$FillMaskImage,
        [Parameter(Mandatory = $true)][System.Drawing.Image]$HexImage,
        [Parameter(Mandatory = $true)][int]$Y,
        [Parameter(Mandatory = $true)][double]$Ratio,
        [Parameter(Mandatory = $true)][System.Drawing.Color]$Color,
        [Parameter(Mandatory = $true)][string]$Label,
        [Parameter(Mandatory = $true)][string]$Value,
        [Parameter(Mandatory = $true)][System.Drawing.Font]$LabelFont,
        [Parameter(Mandatory = $true)][System.Drawing.Font]$ValueFont,
        [Parameter(Mandatory = $true)][System.Drawing.Brush]$ValueBrush
    )

    $RowX = 20
    $RowWidth = 856
    $FillY = $Y + 12
    $FillHeight = 28
    $ClipWidth = [int][Math]::Round($RowWidth * [Math]::Max(0.0, [Math]::Min(1.0, $Ratio)))

    $State = $Graphics.Save()
    try {
        $Graphics.SetClip((New-Object System.Drawing.Rectangle($RowX, $FillY, $ClipWidth, $FillHeight)))
        Draw-VT12TintedImage -Graphics $Graphics -Image $FillMaskImage -Destination (New-Object System.Drawing.Rectangle($RowX, $FillY, $RowWidth, $FillHeight)) -TintColor $Color -Opacity 0.93

        # Existing DefHex Source를 실제 filled 영역에 반복해 Material 인상을 Prototype에서 확인합니다.
        for ($HexX = $RowX + 118; $HexX -lt ($RowX + $ClipWidth); $HexX += 24) {
            Draw-VT12TintedImage -Graphics $Graphics -Image $HexImage -Destination ([System.Drawing.Rectangle]::new($HexX, ($FillY + 1), 24, 24)) -TintColor $Color -Opacity 0.22
        }
    }
    finally {
        $Graphics.Restore($State)
    }

    $Graphics.DrawImage($FrameImage, $RowX, $Y, $RowWidth, 52)

    $LabelBrush = New-Object System.Drawing.SolidBrush($Color)
    try {
        $Graphics.DrawString($Label, $LabelFont, $LabelBrush, 44, $Y + 18)
        $Graphics.DrawString($Value, $ValueFont, $ValueBrush, 754, $Y + 17)
    }
    finally {
        $LabelBrush.Dispose()
    }
}

$Canvas = New-Object System.Drawing.Bitmap(896, 416, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
$Graphics = [System.Drawing.Graphics]::FromImage($Canvas)
Set-VT12GraphicsQuality -Graphics $Graphics

# Prototype용 최소 dark surface입니다. 이 색은 final static texture authority가 아닙니다.
$Graphics.Clear([System.Drawing.Color]::FromArgb(255, 4, 9, 13))

$Frame = [System.Drawing.Image]::FromFile($FramePath)
$SpeedFrame = [System.Drawing.Image]::FromFile($SpeedFramePath)
$ArmorFrame = [System.Drawing.Image]::FromFile($ArmorFramePath)
$Silhouette = [System.Drawing.Image]::FromFile($SilhouettePath)
$ArmorPlate = [System.Drawing.Image]::FromFile($ArmorPlatePath)
$ArmorArrow = [System.Drawing.Image]::FromFile($ArmorArrowPath)
$ArmorChevron = [System.Drawing.Image]::FromFile($ArmorChevronPath)
$DefenseFrame = [System.Drawing.Image]::FromFile($DefenseFramePath)
$DefenseFillMask = [System.Drawing.Image]::FromFile($DefenseFillMaskPath)
$DefenseHex = [System.Drawing.Image]::FromFile($DefenseHexPath)

try {
    # Outer Frame은 주인공이 아니므로 바닥에 한 번만 깔고 더 손대지 않습니다.
    $Graphics.DrawImage($Frame, 0, 0, 896, 416)

    # Approved 422 + 12 + 422 upper composition입니다.
    $Graphics.DrawImage($SpeedFrame, 20, 20, 422, 272)
    $Graphics.DrawImage($ArmorFrame, 454, 20, 422, 272)

    $RpmPreview = New-VT12RpmPreview -MaskPath $RpmMaskPath -Ratio 0.68
    try {
        # SpeedFrame의 runtime RPM hole에 실제 MaskPack preview를 맞춥니다.
        $Graphics.DrawImage($RpmPreview, 58, 50, 346, 223)
    }
    finally {
        $RpmPreview.Dispose()
    }

    $SpeedFont = New-Object System.Drawing.Font('Segoe UI', 48, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $UnitFont = New-Object System.Drawing.Font('Segoe UI', 13, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)
    $GearFont = New-Object System.Drawing.Font('Segoe UI', 34, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $SmallLabelFont = New-Object System.Drawing.Font('Segoe UI', 11, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $DefenseLabelFont = New-Object System.Drawing.Font('Segoe UI', 11, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
    $DefenseValueFont = New-Object System.Drawing.Font('Segoe UI', 12, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)

    $WhiteBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(248, 230, 240, 244))
    $SecondaryBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(220, 137, 160, 169))
    $CyanBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(248, 38, 218, 239))
    $AmberBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(248, 231, 167, 48))

    try {
        # Speed/RPM/Gear를 하나의 계기판 hierarchy로 묶습니다.
        $Graphics.DrawString('RPM', $SmallLabelFont, $SecondaryBrush, 70, 68)
        $Graphics.DrawString('076', $SpeedFont, $WhiteBrush, 143, 136)
        $Graphics.DrawString('km/h', $UnitFont, $SecondaryBrush, 260, 175)
        $Graphics.DrawString('GEAR', $SmallLabelFont, $SecondaryBrush, 346, 160)
        $Graphics.DrawString('3', $GearFont, $CyanBrush, 357, 180)

        # Armor label은 runtime TextBlock 위치를 prototype에서만 보여줍니다.
        $Graphics.DrawString('ARMOR', $SmallLabelFont, $AmberBrush, 478, 47)

        # Vehicle-specific silhouette는 중앙에서 왼쪽을 보는 기존 Source를 그대로 재사용합니다.
        Draw-VT12TintedImage -Graphics $Graphics -Image $Silhouette -Destination (New-Object System.Drawing.Rectangle(586, 111, 168, 84)) -TintColor ([System.Drawing.Color]::FromArgb(255, 188, 207, 214)) -Opacity 0.82

        $ArmorAccent = [System.Drawing.Color]::FromArgb(255, 239, 177, 57)

        # Target direction contract:
        # Top=upper-left, Front=left, Right=top, Left=bottom, Rear=right, Bottom=lower-right.
        Draw-VT12ArmorSector -Graphics $Graphics -PlateImage $ArmorPlate -IconImage $ArmorChevron -CenterX 535 -CenterY 90 -RotationDegrees 180 -Ratio 0.78 -AccentColor $ArmorAccent
        Draw-VT12ArmorSector -Graphics $Graphics -PlateImage $ArmorPlate -IconImage $ArmorArrow -CenterX 518 -CenterY 181 -RotationDegrees 180 -Ratio 0.84 -AccentColor $ArmorAccent
        Draw-VT12ArmorSector -Graphics $Graphics -PlateImage $ArmorPlate -IconImage $ArmorArrow -CenterX 665 -CenterY 71 -RotationDegrees -90 -Ratio 0.91 -AccentColor $ArmorAccent
        Draw-VT12ArmorSector -Graphics $Graphics -PlateImage $ArmorPlate -IconImage $ArmorArrow -CenterX 665 -CenterY 241 -RotationDegrees 90 -Ratio 0.66 -AccentColor $ArmorAccent
        Draw-VT12ArmorSector -Graphics $Graphics -PlateImage $ArmorPlate -IconImage $ArmorArrow -CenterX 813 -CenterY 145 -RotationDegrees 0 -Ratio 0.72 -AccentColor $ArmorAccent
        Draw-VT12ArmorSector -Graphics $Graphics -PlateImage $ArmorPlate -IconImage $ArmorChevron -CenterX 790 -CenterY 226 -RotationDegrees 0 -Ratio 0.58 -AccentColor $ArmorAccent

        # 하부 Defense는 existing source/material family만 사용합니다.
        Draw-VT12DefenseRow -Graphics $Graphics -FrameImage $DefenseFrame -FillMaskImage $DefenseFillMask -HexImage $DefenseHex -Y 302 -Ratio 0.78 -Color ([System.Drawing.Color]::FromArgb(255, 38, 211, 235)) -Label 'SHIELD' -Value '780 / 1000' -LabelFont $DefenseLabelFont -ValueFont $DefenseValueFont -ValueBrush $WhiteBrush
        Draw-VT12DefenseRow -Graphics $Graphics -FrameImage $DefenseFrame -FillMaskImage $DefenseFillMask -HexImage $DefenseHex -Y 356 -Ratio 0.64 -Color ([System.Drawing.Color]::FromArgb(255, 225, 161, 43)) -Label 'INTEGRITY' -Value '640 / 1000' -LabelFont $DefenseLabelFont -ValueFont $DefenseValueFont -ValueBrush $WhiteBrush
    }
    finally {
        $SpeedFont.Dispose()
        $UnitFont.Dispose()
        $GearFont.Dispose()
        $SmallLabelFont.Dispose()
        $DefenseLabelFont.Dispose()
        $DefenseValueFont.Dispose()
        $WhiteBrush.Dispose()
        $SecondaryBrush.Dispose()
        $CyanBrush.Dispose()
        $AmberBrush.Dispose()
    }

    $Canvas.Save($PrototypePath, [System.Drawing.Imaging.ImageFormat]::Png)

    # Reference와 Prototype의 구성 차이를 바로 판단할 수 있는 side-by-side Review입니다.
    $Reference = [System.Drawing.Image]::FromFile($ReferencePath)
    try {
        $Compare = New-Object System.Drawing.Bitmap(1816, 462, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
        $CompareGraphics = [System.Drawing.Graphics]::FromImage($Compare)
        Set-VT12GraphicsQuality -Graphics $CompareGraphics
        $CompareGraphics.Clear([System.Drawing.Color]::FromArgb(255, 3, 7, 10))

        $CompareLabelFont = New-Object System.Drawing.Font('Segoe UI', 13, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
        $CompareBrush = New-Object System.Drawing.SolidBrush([System.Drawing.Color]::FromArgb(230, 196, 211, 218))

        try {
            $CompareGraphics.DrawString('REFERENCE VT-VEH-01', $CompareLabelFont, $CompareBrush, 12, 8)
            $CompareGraphics.DrawString('VT12 ACTUAL-ASSET PROTOTYPE', $CompareLabelFont, $CompareBrush, 920, 8)
            $CompareGraphics.DrawImage($Reference, 0, 34, 896, 416)
            $CompareGraphics.DrawImage($Canvas, 920, 34, 896, 416)
            $Compare.Save($ComparePath, [System.Drawing.Imaging.ImageFormat]::Png)
        }
        finally {
            $CompareLabelFont.Dispose()
            $CompareBrush.Dispose()
            $CompareGraphics.Dispose()
            $Compare.Dispose()
        }
    }
    finally {
        $Reference.Dispose()
    }
}
finally {
    $Frame.Dispose()
    $SpeedFrame.Dispose()
    $ArmorFrame.Dispose()
    $Silhouette.Dispose()
    $ArmorPlate.Dispose()
    $ArmorArrow.Dispose()
    $ArmorChevron.Dispose()
    $DefenseFrame.Dispose()
    $DefenseFillMask.Dispose()
    $DefenseHex.Dispose()
    $Graphics.Dispose()
    $Canvas.Dispose()
}

$PrototypeFile = Get-Item -LiteralPath $PrototypePath
$PrototypeSha = (Get-FileHash -LiteralPath $PrototypePath -Algorithm SHA256).Hash.ToLowerInvariant()
$CompareFile = Get-Item -LiteralPath $ComparePath
$CompareSha = (Get-FileHash -LiteralPath $ComparePath -Algorithm SHA256).Hash.ToLowerInvariant()

Write-Output 'VT12_PROTOTYPE_RESULT=PASS'
Write-Output ("VT12_PROTO file={0} size=896x416 bytes={1} sha256={2}" -f $PrototypeFile.Name, $PrototypeFile.Length, $PrototypeSha)
Write-Output ("VT12_COMPARE file={0} size=1816x462 bytes={1} sha256={2}" -f $CompareFile.Name, $CompareFile.Length, $CompareSha)
exit 0
