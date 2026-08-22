# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.2.0
# Date: 2026-08-20
# Description: CF-FQ-032 Balanced Combat HUD P2 Source Art와 UI-P0-08B Radar 전용 Texture를 생성합니다.
# Scope: SourceArt/UI/HUD/P2에만 출력하며 Unreal Import, DataAsset 연결, Widget 수정은 수행하지 않습니다. -RpmOnly와 -RadarOnly는 서로 독립된 targeted 생성 모드입니다.
# Changelog:
# - v1.2.0: -RadarOnly를 추가해 Radar Frame, Friendly/Hostile/Unknown Blip, Player Marker, 4-Corner Target Bracket, 2-Corner Edge Bracket 7종을 Text glyph 없이 생성.
# - v1.1.0: T_UI_RPMTrack을 정적 Track-only 그림에서 Track+21 Tick+진행 위치(R)+밝기(G)+Redline(B)+Coverage(A) 데이터 Texture로 승격하고 -RpmOnly targeted 생성 모드를 추가.
# - v1.0.1: p2_review.json을 SourceTechnicalReview 전용으로 명시하고 Unreal Import/DA Apply 현재 상태를 false로 기록하던 stale metadata를 제거.
# - v1.0.0: VehiclePanel 9-Slice Frame 1, 좌향 Vehicle Silhouette 1, 차량 로컬 방향 Armor Badge 6, 비대칭 RPM Track 1과 Review Sheet/JSON 생성.
# Migration:
# - P1은 Historical Technical Reference로 그대로 보존합니다.
# - Armor 방향 문자열은 Texture에 굽지 않고 Production UMG TextBlock이 소유합니다.
# - 사용자 Visual 승인 전 P2 Source Art를 최종 Production Art PASS로 확대하지 않습니다.
# - v1.1.0 T_UI_RPMTrack의 RGB는 더 이상 직접 표시 색상이 아닙니다. R=RPM 진행 위치, G=선 밝기, B=Redline 영역이며 A=Coverage입니다. Production에서는 반드시 M_UI_RPMGauge를 통해 표시합니다.
# - v1.2.0 Radar 7종은 흰색/회색 tintable Source Art이며 관계색과 Tactical Accent는 UMG Style Token이 소유합니다. Neutral은 T_UI_RadarUnknown 마름모를 재사용합니다.

[CmdletBinding()]
param(
    # 기존 8개 P2 SourceArt와 Review JSON/Sheet를 보존하고 T_UI_RPMTrack.png 하나만 재생성·기술 검증합니다.
    [Parameter(Mandatory = $false)]
    [switch]$RpmOnly,
    # 기존 Vehicle/RPM SourceArt와 Review JSON/Sheet를 보존하고 Radar 전용 PNG 7종만 생성·기술 검증합니다.
    [Parameter(Mandatory = $false)]
    [switch]$RadarOnly
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Drawing

# 저장소 루트 절대 경로입니다.
$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# P2 VehiclePanel Source Art를 저장할 루트 경로입니다.
$OutputRoot = Join-Path $RepositoryRoot 'SourceArt\UI\HUD\P2'

# 사용자 검토용 합본과 기술 리포트를 저장할 경로입니다.
$ReviewRoot = Join-Path $OutputRoot 'Review'

# 완전 투명 픽셀의 RGB를 흰색으로 유지하는 배경색입니다.
$TransparentWhite = [System.Drawing.Color]::FromArgb(0, 255, 255, 255)

# Tintable Art의 가장 밝은 주 선 색입니다.
$PrimaryWhite = [System.Drawing.Color]::FromArgb(244, 242, 246, 248)

# Tintable Art의 보조 구조선 색입니다.
$SecondaryWhite = [System.Drawing.Color]::FromArgb(178, 184, 196, 204)

# Tintable Art의 약한 내부 구조선 색입니다.
$SubtleWhite = [System.Drawing.Color]::FromArgb(104, 168, 184, 194)

# Armor Badge 내부의 낮은 Alpha 면 색입니다.
$BadgeFill = [System.Drawing.Color]::FromArgb(52, 200, 208, 214)

# Balanced Combat Frame의 어두운 내부 Panel 면 색입니다.
$FrameSurface = [System.Drawing.Color]::FromArgb(226, 8, 15, 21)

# Balanced Combat Frame의 금속성 외곽 면 색입니다.
$FrameMetal = [System.Drawing.Color]::FromArgb(242, 82, 94, 102)

# Balanced Combat Frame의 밝은 금속 Highlight 색입니다.
$FrameHighlight = [System.Drawing.Color]::FromArgb(236, 160, 174, 184)

# Frame의 Cyan 시스템 Accent 색입니다.
$FrameCyan = [System.Drawing.Color]::FromArgb(242, 0, 208, 246)

# Frame의 제한된 Amber 주의 Accent 색입니다.
$FrameAmber = [System.Drawing.Color]::FromArgb(238, 255, 172, 20)

# 출력 디렉터리를 보장합니다.
function Ensure-OutputDirectories {
    if (-not (Test-Path -LiteralPath $OutputRoot)) {
        New-Item -ItemType Directory -Path $OutputRoot -Force | Out-Null
    }

    if (-not (Test-Path -LiteralPath $ReviewRoot)) {
        New-Item -ItemType Directory -Path $ReviewRoot -Force | Out-Null
    }
}

# 지정 크기의 32bpp ARGB 투명 캔버스와 고품질 Graphics를 만듭니다.
function New-ArtCanvas {
    param(
        # 캔버스 폭입니다.
        [Parameter(Mandatory = $true)][int]$Width,
        # 캔버스 높이입니다.
        [Parameter(Mandatory = $true)][int]$Height
    )

    # 새 ARGB 비트맵입니다.
    $Bitmap = New-Object System.Drawing.Bitmap($Width, $Height, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)

    # 비트맵에 연결된 Graphics입니다.
    $Graphics = [System.Drawing.Graphics]::FromImage($Bitmap)

    $Graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $Graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
    $Graphics.CompositingQuality = [System.Drawing.Drawing2D.CompositingQuality]::HighQuality
    $Graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $Graphics.Clear($TransparentWhite)

    # 생성된 Bitmap/Graphics 쌍입니다.
    $Canvas = [PSCustomObject]@{
        Bitmap = $Bitmap
        Graphics = $Graphics
    }

    return $Canvas
}

# 지정 색상과 두께의 Bevel Join Pen을 만듭니다.
function New-ArtPen {
    param(
        # Pen 색입니다.
        [Parameter(Mandatory = $true)][System.Drawing.Color]$Color,
        # Pen 두께입니다.
        [Parameter(Mandatory = $true)][float]$Width
    )

    # 선 그리기에 사용할 Pen입니다.
    $Pen = New-Object System.Drawing.Pen($Color, $Width)
    $Pen.LineJoin = [System.Drawing.Drawing2D.LineJoin]::Bevel
    $Pen.StartCap = [System.Drawing.Drawing2D.LineCap]::Square
    $Pen.EndCap = [System.Drawing.Drawing2D.LineCap]::Square
    return $Pen
}

# 지정 색상의 SolidBrush를 만듭니다.
function New-ArtBrush {
    param(
        # Brush 색입니다.
        [Parameter(Mandatory = $true)][System.Drawing.Color]$Color
    )

    # 면 채우기에 사용할 Brush입니다.
    $Brush = New-Object System.Drawing.SolidBrush($Color)
    return $Brush
}

# X/Y 좌표 목록을 강타입 Point 배열로 변환합니다.
function New-PointArray {
    param(
        # X/Y 정수 좌표가 교대로 들어간 목록입니다.
        [Parameter(Mandatory = $true)][int[]]$Coordinates
    )

    if (($Coordinates.Count % 2) -ne 0) {
        throw 'Coordinates must contain X/Y pairs.'
    }

    # 생성할 Point 개수입니다.
    $PointCount = [int]($Coordinates.Count / 2)

    # 결과 Point 배열입니다.
    $Points = New-Object 'System.Drawing.Point[]' $PointCount

    for ($Index = 0; $Index -lt $PointCount; $Index++) {
        # 현재 Point의 X 좌표입니다.
        $PointX = $Coordinates[$Index * 2]

        # 현재 Point의 Y 좌표입니다.
        $PointY = $Coordinates[($Index * 2) + 1]

        $Points[$Index] = New-Object System.Drawing.Point($PointX, $PointY)
    }

    return $Points
}

# 완성된 Bitmap을 PNG로 저장하고 Graphics 자원을 정리합니다.
function Save-ArtCanvas {
    param(
        # 저장할 Canvas입니다.
        [Parameter(Mandatory = $true)]$Canvas,
        # 확장자를 포함한 PNG 파일명입니다.
        [Parameter(Mandatory = $true)][string]$FileName
    )

    # 최종 출력 파일 절대 경로입니다.
    $OutputPath = Join-Path $OutputRoot $FileName

    $Canvas.Bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $Canvas.Graphics.Dispose()
    $Canvas.Bitmap.Dispose()
    return $OutputPath
}

# Balanced Combat VehiclePanel의 9-Slice용 Cut-Corner Frame Texture를 생성합니다.
function New-VehiclePanelFrame {
    # VehiclePanel Frame 캔버스입니다.
    $Canvas = New-ArtCanvas -Width 512 -Height 256

    # Frame Graphics입니다.
    $Graphics = $Canvas.Graphics

    # Frame 내부 Panel 면 Brush입니다.
    $SurfaceBrush = New-ArtBrush -Color $FrameSurface

    # Frame 금속 면 Brush입니다.
    $MetalBrush = New-ArtBrush -Color $FrameMetal

    # Cyan Accent Brush입니다.
    $CyanBrush = New-ArtBrush -Color $FrameCyan

    # Amber Accent Brush입니다.
    $AmberBrush = New-ArtBrush -Color $FrameAmber

    # 밝은 금속 외곽선 Pen입니다.
    $HighlightPen = New-ArtPen -Color $FrameHighlight -Width 3.0

    # 어두운 내부 구조선 Pen입니다.
    $InnerPen = New-ArtPen -Color ([System.Drawing.Color]::FromArgb(210, 44, 58, 68)) -Width 2.0

    # 9-Slice에서도 Corner 절삭이 유지되는 외곽 Cut-Corner Polygon입니다.
    $OuterPoints = New-PointArray -Coordinates @(30,4, 482,4, 508,30, 508,226, 482,252, 30,252, 4,226, 4,30)

    # 실제 내용 면의 Cut-Corner Polygon입니다.
    $InnerPoints = New-PointArray -Coordinates @(48,24, 464,24, 488,48, 488,208, 464,232, 48,232, 24,208, 24,48)

    $Graphics.FillPolygon($MetalBrush, $OuterPoints)
    $Graphics.DrawPolygon($HighlightPen, $OuterPoints)
    $Graphics.FillPolygon($SurfaceBrush, $InnerPoints)
    $Graphics.DrawPolygon($InnerPen, $InnerPoints)

    # 상단 중앙 Cyan 시스템 Accent입니다.
    $Graphics.FillPolygon($CyanBrush, (New-PointArray -Coordinates @(216,10, 296,10, 286,20, 226,20)))

    # 좌측 상단 Cyan Accent입니다.
    $Graphics.FillPolygon($CyanBrush, (New-PointArray -Coordinates @(15,42, 24,32, 36,32, 27,55, 18,64)))

    # 우측 하단 Cyan Accent입니다.
    $Graphics.FillPolygon($CyanBrush, (New-PointArray -Coordinates @(497,192, 488,224, 476,224, 485,201, 494,184)))

    # 우측 상단에만 제한적으로 들어가는 Amber 상태 Accent입니다.
    $Graphics.FillPolygon($AmberBrush, (New-PointArray -Coordinates @(440,10, 474,10, 482,18, 452,18)))

    # 좌우 Edge의 기계식 Cut Line입니다.
    $Graphics.DrawLine($InnerPen, 12,82, 12,174)
    $Graphics.DrawLine($InnerPen, 500,82, 500,174)

    # 하단 중앙의 얕은 구조선입니다.
    $Graphics.DrawLine($InnerPen, 188,242, 324,242)

    $SurfaceBrush.Dispose()
    $MetalBrush.Dispose()
    $CyanBrush.Dispose()
    $AmberBrush.Dispose()
    $HighlightPen.Dispose()
    $InnerPen.Dispose()

    return (Save-ArtCanvas -Canvas $Canvas -FileName 'T_UI_VehPanelFrame.png')
}

# 중앙 Armor Body Map에 사용할 왼쪽 진행 방향 Vehicle Silhouette를 생성합니다.
function New-VehicleSilhouette {
    # 차량 실루엣 캔버스입니다.
    $Canvas = New-ArtCanvas -Width 512 -Height 256

    # 차량 실루엣 Graphics입니다.
    $Graphics = $Canvas.Graphics

    # 낮은 Alpha 차체 면 Brush입니다.
    $BodyBrush = New-ArtBrush -Color ([System.Drawing.Color]::FromArgb(86, 210, 218, 224))

    # 밝은 외곽선 Pen입니다.
    $OuterPen = New-ArtPen -Color $PrimaryWhite -Width 5.0

    # 주요 장갑 분할 Pen입니다.
    $PanelPen = New-ArtPen -Color $SecondaryWhite -Width 2.5

    # 약한 하위 구조 Pen입니다.
    $SubtlePen = New-ArtPen -Color $SubtleWhite -Width 1.5

    # 좌측 Nose가 명확한 전투 차량 외곽입니다.
    $BodyPoints = New-PointArray -Coordinates @(38,128, 72,88, 136,66, 354,66, 430,88, 470,128, 430,168, 354,190, 136,190, 72,168)

    # 중앙 Cabin 장갑 면입니다.
    $CabinPoints = New-PointArray -Coordinates @(152,94, 212,76, 332,78, 390,102, 400,128, 390,154, 332,178, 212,180, 152,162, 136,128)

    $Graphics.FillPolygon($BodyBrush, $BodyPoints)
    $Graphics.DrawPolygon($OuterPen, $BodyPoints)
    $Graphics.DrawPolygon($PanelPen, $CabinPoints)

    # 차량 진행 방향을 강조하는 좌측 Front Wedge입니다.
    $Graphics.DrawLines($PanelPen, (New-PointArray -Coordinates @(48,128, 92,96, 128,96, 112,128, 128,160, 92,160, 48,128)))

    # Rear Deck 절삭 구조입니다.
    $Graphics.DrawLines($PanelPen, (New-PointArray -Coordinates @(408,100, 446,112, 458,128, 446,144, 408,156, 420,128, 408,100)))

    # 중앙 세로 Armor 분할선입니다.
    $Graphics.DrawLine($PanelPen, 236,78, 236,178)
    $Graphics.DrawLine($PanelPen, 332,82, 332,174)

    # 중앙 수평 Spine입니다.
    $Graphics.DrawLine($PanelPen, 118,128, 426,128)

    # 좌우 보조 장갑 분할입니다.
    $Graphics.DrawLine($SubtlePen, 154,98, 154,158)
    $Graphics.DrawLine($SubtlePen, 382,104, 382,152)

    # Front 좌상 휠 Guard입니다.
    $Graphics.DrawRectangle($OuterPen, 112,54,70,12)

    # Front 좌하 휠 Guard입니다.
    $Graphics.DrawRectangle($OuterPen, 112,190,70,12)

    # Rear 우상 휠 Guard입니다.
    $Graphics.DrawRectangle($OuterPen, 342,56,70,12)

    # Rear 우하 휠 Guard입니다.
    $Graphics.DrawRectangle($OuterPen, 342,188,70,12)

    $BodyBrush.Dispose()
    $OuterPen.Dispose()
    $PanelPen.Dispose()
    $SubtlePen.Dispose()

    return (Save-ArtCanvas -Canvas $Canvas -FileName 'T_UI_VehSil_LF.png')
}

# Armor Badge 공통 Cut-Corner 외곽을 그립니다.
function Draw-ArmorBadgeBase {
    param(
        # Badge를 그릴 Graphics입니다.
        [Parameter(Mandatory = $true)][System.Drawing.Graphics]$Graphics
    )

    # Badge 낮은 Alpha 면 Brush입니다.
    $FillBrush = New-ArtBrush -Color $BadgeFill

    # Badge 밝은 외곽선 Pen입니다.
    $OuterPen = New-ArtPen -Color $PrimaryWhite -Width 4.0

    # Badge 내부 구조선 Pen입니다.
    $InnerPen = New-ArtPen -Color $SecondaryWhite -Width 2.0

    # 128x128 Badge의 Cut-Corner 외곽입니다.
    $BadgePoints = New-PointArray -Coordinates @(28,12, 100,12, 116,28, 116,100, 100,116, 28,116, 12,100, 12,28)

    # 내부 한 겹 Frame입니다.
    $InnerPoints = New-PointArray -Coordinates @(34,22, 94,22, 106,34, 106,94, 94,106, 34,106, 22,94, 22,34)

    $Graphics.FillPolygon($FillBrush, $BadgePoints)
    $Graphics.DrawPolygon($OuterPen, $BadgePoints)
    $Graphics.DrawPolygon($InnerPen, $InnerPoints)

    $FillBrush.Dispose()
    $OuterPen.Dispose()
    $InnerPen.Dispose()
}

# 지정 방향의 단일 굵은 화살표를 Badge 중앙에 그립니다.
function Draw-DirectionArrow {
    param(
        # 화살표를 그릴 Graphics입니다.
        [Parameter(Mandatory = $true)][System.Drawing.Graphics]$Graphics,
        # 화살표 방향입니다.
        [Parameter(Mandatory = $true)][ValidateSet('Left','Up','Right','Down')][string]$Direction
    )

    # 화살표 밝은 채움 Brush입니다.
    $ArrowBrush = New-ArtBrush -Color $PrimaryWhite

    # 화살표 보조 외곽선 Pen입니다.
    $ArrowPen = New-ArtPen -Color ([System.Drawing.Color]::FromArgb(246, 255, 255, 255)) -Width 2.0

    # 기본 좌측 방향 화살표 좌표입니다.
    $ArrowPoints = New-PointArray -Coordinates @(28,64, 54,38, 54,52, 96,52, 96,76, 54,76, 54,90)

    # 방향 회전 중심 X입니다.
    $CenterX = 64.0

    # 방향 회전 중심 Y입니다.
    $CenterY = 64.0

    # 좌측 기준에서 적용할 회전 각도입니다.
    $AngleDegrees = switch ($Direction) {
        'Left' { 0.0 }
        'Up' { 90.0 }
        'Right' { 180.0 }
        'Down' { 270.0 }
    }

    # 회전에 사용할 라디안 각도입니다.
    $Radians = $AngleDegrees * [Math]::PI / 180.0

    # 회전된 Point 배열입니다.
    $RotatedPoints = New-Object 'System.Drawing.Point[]' $ArrowPoints.Count

    for ($PointIndex = 0; $PointIndex -lt $ArrowPoints.Count; $PointIndex++) {
        # 원본 Point의 중심 상대 X입니다.
        $RelativeX = $ArrowPoints[$PointIndex].X - $CenterX

        # 원본 Point의 중심 상대 Y입니다.
        $RelativeY = $ArrowPoints[$PointIndex].Y - $CenterY

        # 회전 후 X입니다.
        $RotatedX = $CenterX + ($RelativeX * [Math]::Cos($Radians)) - ($RelativeY * [Math]::Sin($Radians))

        # 회전 후 Y입니다.
        $RotatedY = $CenterY + ($RelativeX * [Math]::Sin($Radians)) + ($RelativeY * [Math]::Cos($Radians))

        $RotatedPoints[$PointIndex] = New-Object System.Drawing.Point([int][Math]::Round($RotatedX), [int][Math]::Round($RotatedY))
    }

    $Graphics.FillPolygon($ArrowBrush, $RotatedPoints)
    $Graphics.DrawPolygon($ArrowPen, $RotatedPoints)

    $ArrowBrush.Dispose()
    $ArrowPen.Dispose()
}

# Top/Bottom 장갑을 구분하는 이중 Chevron을 Badge 중앙에 그립니다.
function Draw-VerticalChevrons {
    param(
        # Chevron을 그릴 Graphics입니다.
        [Parameter(Mandatory = $true)][System.Drawing.Graphics]$Graphics,
        # Chevron 방향입니다.
        [Parameter(Mandatory = $true)][ValidateSet('Up','Down')][string]$Direction
    )

    # Chevron 주 선 Pen입니다.
    $ChevronPen = New-ArtPen -Color $PrimaryWhite -Width 7.0

    # 위쪽 Chevron 두 개의 원본 Y 위치입니다.
    $FirstY = if ($Direction -eq 'Up') { 70 } else { 48 }

    # 두 번째 Chevron의 Y 위치입니다.
    $SecondY = if ($Direction -eq 'Up') { 88 } else { 66 }

    if ($Direction -eq 'Up') {
        $Graphics.DrawLines($ChevronPen, (New-PointArray -Coordinates @(38,$FirstY, 64,48, 90,$FirstY)))
        $Graphics.DrawLines($ChevronPen, (New-PointArray -Coordinates @(38,$SecondY, 64,66, 90,$SecondY)))
    }
    else {
        $Graphics.DrawLines($ChevronPen, (New-PointArray -Coordinates @(38,$FirstY, 64,70, 90,$FirstY)))
        $Graphics.DrawLines($ChevronPen, (New-PointArray -Coordinates @(38,$SecondY, 64,88, 90,$SecondY)))
    }

    $ChevronPen.Dispose()
}

# 한 방향의 Tintable Armor Badge Texture를 생성합니다.
function New-ArmorBadge {
    param(
        # 생성할 차량 로컬 장갑 방향입니다.
        [Parameter(Mandatory = $true)][ValidateSet('Front','Right','Rear','Left','Top','Bottom')][string]$Direction,
        # 출력 파일명입니다.
        [Parameter(Mandatory = $true)][string]$FileName
    )

    # Armor Badge 캔버스입니다.
    $Canvas = New-ArtCanvas -Width 128 -Height 128

    # Armor Badge Graphics입니다.
    $Graphics = $Canvas.Graphics

    Draw-ArmorBadgeBase -Graphics $Graphics

    switch ($Direction) {
        'Front' { Draw-DirectionArrow -Graphics $Graphics -Direction 'Left' }
        'Right' { Draw-DirectionArrow -Graphics $Graphics -Direction 'Up' }
        'Rear' { Draw-DirectionArrow -Graphics $Graphics -Direction 'Right' }
        'Left' { Draw-DirectionArrow -Graphics $Graphics -Direction 'Down' }
        'Top' { Draw-VerticalChevrons -Graphics $Graphics -Direction 'Up' }
        'Bottom' { Draw-VerticalChevrons -Graphics $Graphics -Direction 'Down' }
    }

    return (Save-ArtCanvas -Canvas $Canvas -FileName $FileName)
}

# 지정 Polyline의 0~1 진행 위치에 해당하는 실제 좌표를 계산합니다.
function Get-RpmPolylinePoint {
    param(
        # X/Y 좌표를 가진 Polyline Point 목록입니다.
        [Parameter(Mandatory = $true)][object[]]$Points,
        # 전체 Polyline 길이입니다.
        [Parameter(Mandatory = $true)][double]$TotalLength,
        # 0~1 진행 위치입니다.
        [Parameter(Mandatory = $true)][double]$Ratio
    )

    # Polyline 범위를 벗어나지 않도록 제한한 진행 위치입니다.
    $ClampedRatio = [Math]::Max(0.0, [Math]::Min(1.0, $Ratio))

    # 시작점부터 목표 좌표까지의 누적 거리입니다.
    $TargetDistance = $TotalLength * $ClampedRatio

    # 현재까지 누적한 Polyline 거리입니다.
    $AccumulatedDistance = 0.0

    for ($PointIndex = 0; $PointIndex -lt ($Points.Count - 1); $PointIndex++) {
        # 현재 Segment 시작점입니다.
        $StartPoint = $Points[$PointIndex]

        # 현재 Segment 끝점입니다.
        $EndPoint = $Points[$PointIndex + 1]

        # 현재 Segment의 X 변화량입니다.
        $DeltaX = [double]$EndPoint.X - [double]$StartPoint.X

        # 현재 Segment의 Y 변화량입니다.
        $DeltaY = [double]$EndPoint.Y - [double]$StartPoint.Y

        # 현재 Segment 길이입니다.
        $SegmentLength = [Math]::Sqrt(($DeltaX * $DeltaX) + ($DeltaY * $DeltaY))

        if (($AccumulatedDistance + $SegmentLength) -ge $TargetDistance -or $PointIndex -eq ($Points.Count - 2)) {
            # 현재 Segment 내부에서 목표 위치가 차지하는 0~1 비율입니다.
            $SegmentRatio = if ($SegmentLength -gt 0.0) {
                [Math]::Max(0.0, [Math]::Min(1.0, ($TargetDistance - $AccumulatedDistance) / $SegmentLength))
            }
            else {
                0.0
            }

            return [PSCustomObject]@{
                X = [double]$StartPoint.X + ($DeltaX * $SegmentRatio)
                Y = [double]$StartPoint.Y + ($DeltaY * $SegmentRatio)
            }
        }

        $AccumulatedDistance += $SegmentLength
    }

    return $Points[$Points.Count - 1]
}

# 지정 Polyline의 전체 길이를 계산합니다.
function Get-RpmPolylineLength {
    param(
        # X/Y 좌표를 가진 Polyline Point 목록입니다.
        [Parameter(Mandatory = $true)][object[]]$Points
    )

    # 계산된 전체 Polyline 길이입니다.
    $TotalLength = 0.0

    for ($PointIndex = 0; $PointIndex -lt ($Points.Count - 1); $PointIndex++) {
        # 현재 Segment의 X 변화량입니다.
        $DeltaX = [double]$Points[$PointIndex + 1].X - [double]$Points[$PointIndex].X

        # 현재 Segment의 Y 변화량입니다.
        $DeltaY = [double]$Points[$PointIndex + 1].Y - [double]$Points[$PointIndex].Y

        $TotalLength += [Math]::Sqrt(($DeltaX * $DeltaX) + ($DeltaY * $DeltaY))
    }

    return $TotalLength
}

# RPM 진행 위치와 밝기/Redline 의미를 RGBA 데이터 색상으로 변환합니다.
function New-RpmDataColor {
    param(
        # 0~1 RPM Gauge 진행 위치입니다.
        [Parameter(Mandatory = $true)][double]$ProgressRatio,
        # Material에서 선 밝기/강도로 사용할 0~1 값입니다.
        [Parameter(Mandatory = $true)][double]$Intensity,
        # 85% 이후 Redline 영역인지 여부입니다.
        [Parameter(Mandatory = $true)][bool]$Redline
    )

    # RPMRatio=0에서 시작 픽셀이 잘못 활성화되지 않도록 0/1 끝값을 살짝 안쪽으로 제한한 진행 위치입니다.
    $EncodedProgress = 0.001 + (0.998 * [Math]::Max(0.0, [Math]::Min(1.0, $ProgressRatio)))

    # R 채널에 기록할 0~255 진행 위치입니다.
    $ProgressByte = [int][Math]::Round($EncodedProgress * 255.0)

    # G 채널에 기록할 0~255 선 밝기입니다.
    $IntensityByte = [int][Math]::Round([Math]::Max(0.0, [Math]::Min(1.0, $Intensity)) * 255.0)

    # B 채널에 기록할 Redline Mask입니다.
    $RedlineByte = if ($Redline) { 255 } else { 0 }

    return [System.Drawing.Color]::FromArgb(255, $ProgressByte, $IntensityByte, $RedlineByte)
}

# 좌측 세로→곡선→상단 수평 Track과 21 Tick의 진행 위치를 한 장에 인코딩한 동적 RPM Gauge 데이터 Texture를 생성합니다.
function New-RpmTrack {
    # RPM 데이터 Texture 캔버스입니다.
    $Canvas = New-ArtCanvas -Width 640 -Height 448

    # RPM 데이터 Texture Graphics입니다.
    $Graphics = $Canvas.Graphics

    # 데이터 채널 경계가 Alpha Antialias로 섞이지 않도록 Source 단계에서는 Hard Edge로 그립니다. 실제 UMG 축소에서는 Texture Filtering이 시각적 부드러움을 담당합니다.
    $Graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::None
    $Graphics.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::None

    # 승인된 비대칭 RPM 주 Track Polyline입니다.
    $MainTrackPoints = @(
        [PSCustomObject]@{ X = 54.0; Y = 390.0 },
        [PSCustomObject]@{ X = 54.0; Y = 196.0 },
        [PSCustomObject]@{ X = 72.0; Y = 132.0 },
        [PSCustomObject]@{ X = 116.0; Y = 84.0 },
        [PSCustomObject]@{ X = 586.0; Y = 84.0 },
        [PSCustomObject]@{ X = 610.0; Y = 108.0 }
    )

    # 주 Track 전체 길이입니다.
    $MainTrackLength = Get-RpmPolylineLength -Points $MainTrackPoints

    # 진행 위치가 충분히 부드럽게 변하도록 주 Track을 나눌 Segment 개수입니다.
    $MainSegmentCount = 220

    for ($SegmentIndex = 0; $SegmentIndex -lt $MainSegmentCount; $SegmentIndex++) {
        # 현재 Segment 시작 진행 위치입니다.
        $StartRatio = [double]$SegmentIndex / [double]$MainSegmentCount

        # 현재 Segment 끝 진행 위치입니다.
        $EndRatio = [double]($SegmentIndex + 1) / [double]$MainSegmentCount

        # 데이터 색에 사용할 현재 Segment 중앙 진행 위치입니다.
        $MiddleRatio = ($StartRatio + $EndRatio) * 0.5

        # 현재 Segment 시작 좌표입니다.
        $StartPoint = Get-RpmPolylinePoint -Points $MainTrackPoints -TotalLength $MainTrackLength -Ratio $StartRatio

        # 현재 Segment 끝 좌표입니다.
        $EndPoint = Get-RpmPolylinePoint -Points $MainTrackPoints -TotalLength $MainTrackLength -Ratio $EndRatio

        # 현재 Segment의 진행도/밝기/Redline 데이터를 담은 색입니다.
        $SegmentColor = New-RpmDataColor -ProgressRatio $MiddleRatio -Intensity 1.0 -Redline ($MiddleRatio -ge 0.85)

        # 현재 Segment를 그릴 Pen입니다.
        $SegmentPen = New-ArtPen -Color $SegmentColor -Width 9.0

        $Graphics.DrawLine($SegmentPen, [float]$StartPoint.X, [float]$StartPoint.Y, [float]$EndPoint.X, [float]$EndPoint.Y)
        $SegmentPen.Dispose()
    }

    # 주 Track 안쪽의 낮은 강도 보조 Guide Polyline입니다.
    $InnerTrackPoints = @(
        [PSCustomObject]@{ X = 72.0; Y = 382.0 },
        [PSCustomObject]@{ X = 72.0; Y = 204.0 },
        [PSCustomObject]@{ X = 90.0; Y = 144.0 },
        [PSCustomObject]@{ X = 128.0; Y = 102.0 },
        [PSCustomObject]@{ X = 576.0; Y = 102.0 },
        [PSCustomObject]@{ X = 598.0; Y = 124.0 }
    )

    # 보조 Guide 전체 길이입니다.
    $InnerTrackLength = Get-RpmPolylineLength -Points $InnerTrackPoints

    # 보조 Guide 진행 위치를 나눌 Segment 개수입니다.
    $InnerSegmentCount = 180

    for ($SegmentIndex = 0; $SegmentIndex -lt $InnerSegmentCount; $SegmentIndex++) {
        # 현재 보조 Segment 시작 진행 위치입니다.
        $StartRatio = [double]$SegmentIndex / [double]$InnerSegmentCount

        # 현재 보조 Segment 끝 진행 위치입니다.
        $EndRatio = [double]($SegmentIndex + 1) / [double]$InnerSegmentCount

        # 현재 보조 Segment 중앙 진행 위치입니다.
        $MiddleRatio = ($StartRatio + $EndRatio) * 0.5

        # 현재 보조 Segment 시작 좌표입니다.
        $StartPoint = Get-RpmPolylinePoint -Points $InnerTrackPoints -TotalLength $InnerTrackLength -Ratio $StartRatio

        # 현재 보조 Segment 끝 좌표입니다.
        $EndPoint = Get-RpmPolylinePoint -Points $InnerTrackPoints -TotalLength $InnerTrackLength -Ratio $EndRatio

        # 현재 보조 Segment의 낮은 밝기 데이터 색입니다.
        $SegmentColor = New-RpmDataColor -ProgressRatio $MiddleRatio -Intensity 0.42 -Redline ($MiddleRatio -ge 0.85)

        # 현재 보조 Segment Pen입니다.
        $SegmentPen = New-ArtPen -Color $SegmentColor -Width 3.0

        $Graphics.DrawLine($SegmentPen, [float]$StartPoint.X, [float]$StartPoint.Y, [float]$EndPoint.X, [float]$EndPoint.Y)
        $SegmentPen.Dispose()
    }

    # 구형 UMG Tick 21개의 승인된 화면 위치/회전/크기를 Texture 좌표로 그대로 옮길 원본 정의입니다.
    $TickSpecs = @(
        @{ X = 18.0; Y = 158.0; Angle = 0.0 }, @{ X = 18.0; Y = 145.0; Angle = 0.0 },
        @{ X = 18.0; Y = 132.0; Angle = 0.0 }, @{ X = 18.0; Y = 119.0; Angle = 0.0 },
        @{ X = 18.0; Y = 106.0; Angle = 0.0 }, @{ X = 18.0; Y = 93.0; Angle = 0.0 },
        @{ X = 18.0; Y = 80.0; Angle = 0.0 }, @{ X = 19.0; Y = 68.0; Angle = 0.0 },
        @{ X = 22.0; Y = 56.0; Angle = 15.0 }, @{ X = 28.0; Y = 45.0; Angle = 30.0 },
        @{ X = 38.0; Y = 36.0; Angle = 45.0 }, @{ X = 50.0; Y = 29.0; Angle = 60.0 },
        @{ X = 64.0; Y = 25.0; Angle = 75.0 }, @{ X = 80.0; Y = 22.0; Angle = 90.0 },
        @{ X = 99.0; Y = 22.0; Angle = 90.0 }, @{ X = 118.0; Y = 22.0; Angle = 90.0 },
        @{ X = 137.0; Y = 22.0; Angle = 90.0 }, @{ X = 156.0; Y = 22.0; Angle = 90.0 },
        @{ X = 175.0; Y = 22.0; Angle = 90.0 }, @{ X = 194.0; Y = 22.0; Angle = 90.0 },
        @{ X = 213.0; Y = 22.0; Angle = 90.0 }
    )

    # SpeedGauge의 220x154 Image 영역을 640x448 Texture로 옮길 X Scale입니다.
    $TextureScaleX = 640.0 / 220.0

    # SpeedGauge의 220x154 Image 영역을 640x448 Texture로 옮길 Y Scale입니다.
    $TextureScaleY = 448.0 / 154.0

    for ($TickIndex = 0; $TickIndex -lt $TickSpecs.Count; $TickIndex++) {
        # 현재 Tick의 정확한 0~1 Gauge 진행 위치입니다.
        $TickRatio = [double]$TickIndex / 20.0

        # 10% 단위 Major Tick인지 여부입니다.
        $MajorTick = (($TickIndex % 2) -eq 0)

        # 85% 시작 Tick을 더 길게 표시할지 여부입니다.
        $RedlineStartTick = ($TickIndex -eq 17)

        # 현재 Tick이 Redline 영역인지 여부입니다.
        $RedlineTick = ($TickIndex -ge 17)

        # 기존 화면 계약의 Tick 길이입니다.
        $TickLength = if ($RedlineStartTick) { 15.0 } elseif ($MajorTick) { 12.0 } else { 8.0 }

        # 기존 화면 계약의 Tick 두께입니다.
        $TickThickness = if ($MajorTick -or $RedlineStartTick) { 3.0 } else { 2.0 }

        # 현재 Tick의 밝기 강도입니다.
        $TickIntensity = if ($MajorTick -or $RedlineStartTick) { 1.0 } else { 0.68 }

        # UMG Tick Top-Left 좌표를 RPM Image 내부 좌표로 변환한 X입니다.
        $LocalX = [double]$TickSpecs[$TickIndex].X - 8.0

        # UMG Tick Top-Left 좌표를 RPM Image 내부 좌표로 변환한 Y입니다.
        $LocalY = [double]$TickSpecs[$TickIndex].Y - 14.0

        # Texture에서 현재 Tick 중심 X입니다.
        $CenterX = ($LocalX + ($TickLength * 0.5)) * $TextureScaleX

        # Texture에서 현재 Tick 중심 Y입니다.
        $CenterY = ($LocalY + ($TickThickness * 0.5)) * $TextureScaleY

        # 현재 Tick의 Texture 기준 절반 길이입니다.
        $HalfLength = ($TickLength * $TextureScaleX) * 0.5

        # 현재 Tick 회전 각도의 Radian 값입니다.
        $AngleRadians = ([double]$TickSpecs[$TickIndex].Angle) * [Math]::PI / 180.0

        # 현재 Tick 선분의 X 방향 단위 벡터입니다.
        $DirectionX = [Math]::Cos($AngleRadians)

        # 현재 Tick 선분의 Y 방향 단위 벡터입니다.
        $DirectionY = [Math]::Sin($AngleRadians)

        # 현재 Tick 선분 시작 X입니다.
        $StartX = $CenterX - ($DirectionX * $HalfLength)

        # 현재 Tick 선분 시작 Y입니다.
        $StartY = $CenterY - ($DirectionY * $HalfLength)

        # 현재 Tick 선분 끝 X입니다.
        $EndX = $CenterX + ($DirectionX * $HalfLength)

        # 현재 Tick 선분 끝 Y입니다.
        $EndY = $CenterY + ($DirectionY * $HalfLength)

        # 현재 Tick의 진행도/강도/Redline 데이터를 담은 색입니다.
        $TickColor = New-RpmDataColor -ProgressRatio $TickRatio -Intensity $TickIntensity -Redline $RedlineTick

        # 현재 Tick을 그릴 Pen입니다.
        $TickPen = New-ArtPen -Color $TickColor -Width ([float]($TickThickness * $TextureScaleY))

        $Graphics.DrawLine($TickPen, [float]$StartX, [float]$StartY, [float]$EndX, [float]$EndY)
        $TickPen.Dispose()
    }

        return (Save-ArtCanvas -Canvas $Canvas -FileName 'T_UI_RPMTrack.png')
}

# Heading-Up Radar의 외곽 Frame, 3개 Range Ring과 중심 Cross Grid를 Text 없이 생성합니다.
function New-RadarFrame {
    # Radar Frame Source Art의 512x512 투명 캔버스입니다.
    $Canvas = New-ArtCanvas -Width 512 -Height 512

    # Radar Frame 선을 그릴 Graphics입니다.
    $Graphics = $Canvas.Graphics

    # 가장 밝은 외곽 Frame Pen입니다.
    $OuterPen = New-ArtPen -Color $PrimaryWhite -Width 5.0

    # 세 Range Ring을 그릴 보조 Pen입니다.
    $RingPen = New-ArtPen -Color $SecondaryWhite -Width 2.5

    # Heading-Up 중심 Cross Grid와 보조 Tick용 약한 Pen입니다.
    $GridPen = New-ArtPen -Color $SubtleWhite -Width 2.0

    $Graphics.DrawEllipse($OuterPen, 18, 18, 476, 476)
    $Graphics.DrawEllipse($RingPen, 78, 78, 356, 356)
    $Graphics.DrawEllipse($RingPen, 138, 138, 236, 236)
    $Graphics.DrawEllipse($RingPen, 198, 198, 116, 116)
    $Graphics.DrawLine($GridPen, 256, 18, 256, 494)
    $Graphics.DrawLine($GridPen, 18, 256, 494, 256)
    $Graphics.DrawLine($RingPen, 246, 18, 266, 18)
    $Graphics.DrawLine($RingPen, 246, 494, 266, 494)
    $Graphics.DrawLine($RingPen, 18, 246, 18, 266)
    $Graphics.DrawLine($RingPen, 494, 246, 494, 266)

    $OuterPen.Dispose()
    $RingPen.Dispose()
    $GridPen.Dispose()

    return (Save-ArtCanvas -Canvas $Canvas -FileName 'T_UI_RadarFrame.png')
}

# Friendly/Hostile/Unknown/Player Radar Marker를 tintable Solid Shape Texture로 생성합니다.
function New-RadarMarker {
    param(
        # 생성할 Radar Marker의 의미 형상입니다.
        [Parameter(Mandatory = $true)][ValidateSet('Friendly','Hostile','Unknown','Player')][string]$Shape,
        # 저장할 PNG 파일명입니다.
        [Parameter(Mandatory = $true)][string]$FileName
    )

    # Radar Marker의 64x64 투명 캔버스입니다.
    $Canvas = New-ArtCanvas -Width 64 -Height 64

    # Solid Marker를 그릴 Graphics입니다.
    $Graphics = $Canvas.Graphics

    # 관계색 UMG Tint를 받기 전 기본 흰색 Solid Brush입니다.
    $MarkerBrush = New-ArtBrush -Color $PrimaryWhite

    switch ($Shape) {
        'Friendly' {
            $Graphics.FillEllipse($MarkerBrush, 17, 17, 30, 30)
        }
        'Hostile' {
            # 아래쪽을 가리키는 적대 Contact 삼각형입니다.
            $HostilePoints = New-PointArray -Coordinates @(10,14, 54,14, 32,54)
            $Graphics.FillPolygon($MarkerBrush, $HostilePoints)
        }
        'Unknown' {
            # Unknown과 Neutral이 색만 달리해 공유하는 마름모입니다.
            $UnknownPoints = New-PointArray -Coordinates @(32,8, 56,32, 32,56, 8,32)
            $Graphics.FillPolygon($MarkerBrush, $UnknownPoints)
        }
        'Player' {
            # Heading-Up 중앙에서 위쪽을 가리키는 Player 삼각형입니다.
            $PlayerPoints = New-PointArray -Coordinates @(32,7, 55,53, 32,42, 9,53)
            $Graphics.FillPolygon($MarkerBrush, $PlayerPoints)
        }
    }

    $MarkerBrush.Dispose()
    return (Save-ArtCanvas -Canvas $Canvas -FileName $FileName)
}

# 표시 Range 안에서 선택 Contact를 감싸는 4-Corner Open Bracket을 생성합니다.
function New-TargetBracket {
    # Target Bracket의 128x128 투명 캔버스입니다.
    $Canvas = New-ArtCanvas -Width 128 -Height 128

    # 4-Corner Bracket을 그릴 Graphics입니다.
    $Graphics = $Canvas.Graphics

    # Tactical Accent Tint를 받기 전 흰색 Bracket Pen입니다.
    $BracketPen = New-ArtPen -Color $PrimaryWhite -Width 6.0

    $Graphics.DrawLines($BracketPen, (New-PointArray -Coordinates @(14,42, 14,14, 42,14)))
    $Graphics.DrawLines($BracketPen, (New-PointArray -Coordinates @(86,14, 114,14, 114,42)))
    $Graphics.DrawLines($BracketPen, (New-PointArray -Coordinates @(14,86, 14,114, 42,114)))
    $Graphics.DrawLines($BracketPen, (New-PointArray -Coordinates @(86,114, 114,114, 114,86)))

    $BracketPen.Dispose()
    return (Save-ArtCanvas -Canvas $Canvas -FileName 'T_UI_TargetBracket.png')
}

# 표시 Range 밖 선택 Contact의 방향을 나타낼 2-Corner Open Edge Bracket을 생성합니다.
function New-RadarEdgeBracket {
    # Edge Bracket의 128x128 투명 캔버스입니다.
    $Canvas = New-ArtCanvas -Width 128 -Height 128

    # 2-Corner Edge Bracket을 그릴 Graphics입니다.
    $Graphics = $Canvas.Graphics

    # Tactical Accent Tint를 받기 전 흰색 Bracket Pen입니다.
    $BracketPen = New-ArtPen -Color $PrimaryWhite -Width 7.0

    $Graphics.DrawLines($BracketPen, (New-PointArray -Coordinates @(14,48, 14,14, 48,14)))
    $Graphics.DrawLines($BracketPen, (New-PointArray -Coordinates @(80,114, 114,114, 114,80)))

    $BracketPen.Dispose()
    return (Save-ArtCanvas -Canvas $Canvas -FileName 'T_UI_RadarEdge.png')
}

# PNG의 해상도, Alpha 사용, 투명 모서리와 검은 Alpha 가장자리 위험을 기술 검증합니다.
function Test-ArtPng {
    param(
        # 검증할 PNG 절대 경로입니다.
        [Parameter(Mandatory = $true)][string]$Path,
        # 기대 폭입니다.
        [Parameter(Mandatory = $true)][int]$ExpectedWidth,
        # 기대 높이입니다.
        [Parameter(Mandatory = $true)][int]$ExpectedHeight
    )

    # 검증 대상 Bitmap입니다.
    $Bitmap = New-Object System.Drawing.Bitmap($Path)

    # Alpha가 존재하는 픽셀 수입니다.
    $VisiblePixelCount = 0

    # 검은 Halo 위험이 있는 반투명 가장자리 픽셀 수입니다.
    $DarkTranslucentPixelCount = 0

    # 비투명 Bounds 최소 X입니다.
    $MinX = $Bitmap.Width

    # 비투명 Bounds 최소 Y입니다.
    $MinY = $Bitmap.Height

    # 비투명 Bounds 최대 X입니다.
    $MaxX = -1

    # 비투명 Bounds 최대 Y입니다.
    $MaxY = -1

    for ($Y = 0; $Y -lt $Bitmap.Height; $Y++) {
        for ($X = 0; $X -lt $Bitmap.Width; $X++) {
            # 현재 픽셀 색상입니다.
            $Pixel = $Bitmap.GetPixel($X, $Y)

            if ($Pixel.A -gt 0) {
                $VisiblePixelCount++

                if ($Pixel.A -lt 255 -and $Pixel.R -lt 20 -and $Pixel.G -lt 20 -and $Pixel.B -lt 20) {
                    $DarkTranslucentPixelCount++
                }

                if ($X -lt $MinX) { $MinX = $X }
                if ($Y -lt $MinY) { $MinY = $Y }
                if ($X -gt $MaxX) { $MaxX = $X }
                if ($Y -gt $MaxY) { $MaxY = $Y }
            }
        }
    }

    # 전체 픽셀 수입니다.
    $TotalPixelCount = $Bitmap.Width * $Bitmap.Height

    # Alpha가 존재하는 픽셀 비율입니다.
    $AlphaCoveragePercent = [Math]::Round(($VisiblePixelCount * 100.0) / $TotalPixelCount, 2)

    # 네 모서리 Pixel입니다.
    $CornerPixels = @(
        $Bitmap.GetPixel(0, 0),
        $Bitmap.GetPixel($Bitmap.Width - 1, 0),
        $Bitmap.GetPixel(0, $Bitmap.Height - 1),
        $Bitmap.GetPixel($Bitmap.Width - 1, $Bitmap.Height - 1)
    )

    # 네 모서리가 모두 완전 투명인지 여부입니다.
    $TransparentCorners = (@($CornerPixels | Where-Object { $_.A -ne 0 }).Count -eq 0)

    # 기대 해상도와 일치하는지 여부입니다.
    $SizeMatches = ($Bitmap.Width -eq $ExpectedWidth -and $Bitmap.Height -eq $ExpectedHeight)

    # 빈 이미지가 아닌지 여부입니다.
    $HasVisiblePixels = ($VisiblePixelCount -gt 0)

    # 이미지 전체를 완전 불투명하게 덮지 않는지 여부입니다.
    $HasTransparentArea = ($VisiblePixelCount -lt $TotalPixelCount)

    # 검은 Alpha 가장자리 위험 여부입니다.
    $HasBlackAlphaEdgeRisk = ($DarkTranslucentPixelCount -gt 0)

    # 전체 기술 검토 PASS 여부입니다.
    $TechnicalPass = ($SizeMatches -and $HasVisiblePixels -and $HasTransparentArea -and $TransparentCorners -and -not $HasBlackAlphaEdgeRisk)

    # 실제 비투명 Bounds 문자열입니다.
    $VisibleBounds = if ($HasVisiblePixels) { "${MinX},${MinY}-${MaxX},${MaxY}" } else { 'EMPTY' }

    # 결과 파일명입니다.
    $FileName = [System.IO.Path]::GetFileName($Path)

    # 실제 PNG 해상도 문자열입니다.
    $ActualSize = "$($Bitmap.Width)x$($Bitmap.Height)"

    $Bitmap.Dispose()

    return [PSCustomObject]@{
        FileName = $FileName
        ExpectedSize = "${ExpectedWidth}x${ExpectedHeight}"
        ActualSize = $ActualSize
        AlphaCoveragePercent = $AlphaCoveragePercent
        VisibleBounds = $VisibleBounds
        TransparentCorners = $TransparentCorners
        DarkTranslucentPixelCount = $DarkTranslucentPixelCount
        HasBlackAlphaEdgeRisk = $HasBlackAlphaEdgeRisk
        NoTextByConstruction = $true
        TechnicalPass = $TechnicalPass
                UserVisualApproval = $false
        ImportStateTrackedByThisReport = $false
    }
}

# P2 Source Art 9종을 한 화면에서 확인할 사용자 검토용 Contact Sheet를 생성합니다.
function New-ReviewSheet {
    param(
        # 검토할 PNG 절대 경로 목록입니다.
        [Parameter(Mandatory = $true)][string[]]$Paths
    )

    # Review Sheet 폭입니다.
    $SheetWidth = 1320

    # Review Sheet 높이입니다.
    $SheetHeight = 900

    # Review Sheet Bitmap입니다.
    $Sheet = New-Object System.Drawing.Bitmap($SheetWidth, $SheetHeight, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)

    # Review Sheet Graphics입니다.
    $Graphics = [System.Drawing.Graphics]::FromImage($Sheet)

    # 어두운 HUD 검토 배경색입니다.
    $BackgroundColor = [System.Drawing.Color]::FromArgb(255, 7, 13, 18)

    # Tile 외곽선 색입니다.
    $TileLineColor = [System.Drawing.Color]::FromArgb(255, 65, 91, 106)

    # Label 색입니다.
    $LabelColor = [System.Drawing.Color]::FromArgb(255, 232, 240, 245)

    # Tile 외곽선 Pen입니다.
    $TilePen = New-ArtPen -Color $TileLineColor -Width 2.0

    # Label Brush입니다.
    $LabelBrush = New-ArtBrush -Color $LabelColor

    # 파일명 Label Font입니다.
    $LabelFont = New-Object System.Drawing.Font('Segoe UI', 13.0, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)

    $Graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $Graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $Graphics.Clear($BackgroundColor)

    # Tile 폭입니다.
    $TileWidth = 410

    # Tile 높이입니다.
    $TileHeight = 270

    # Tile 좌측 여백입니다.
    $MarginX = 30

    # Tile 상단 여백입니다.
    $MarginY = 25

    # Tile 사이 가로 간격입니다.
    $GapX = 15

    # Tile 사이 세로 간격입니다.
    $GapY = 18

    for ($Index = 0; $Index -lt $Paths.Count; $Index++) {
        # 현재 Tile 열 번호입니다.
        $Column = $Index % 3

        # 현재 Tile 행 번호입니다.
        $Row = [Math]::Floor($Index / 3)

        # 현재 Tile X입니다.
        $TileX = $MarginX + ($Column * ($TileWidth + $GapX))

        # 현재 Tile Y입니다.
        $TileY = $MarginY + ($Row * ($TileHeight + $GapY))

        # 현재 Tile Rectangle입니다.
        $TileRect = New-Object System.Drawing.Rectangle([int]$TileX, [int]$TileY, $TileWidth, $TileHeight)

        # 현재 PNG Bitmap입니다.
        $Image = New-Object System.Drawing.Bitmap($Paths[$Index])

        # 이미지 표시 최대 폭입니다.
        $MaxImageWidth = 360.0

        # 이미지 표시 최대 높이입니다.
        $MaxImageHeight = 205.0

        # 원본 종횡비를 유지하는 Scale입니다.
        $Scale = [Math]::Min($MaxImageWidth / $Image.Width, $MaxImageHeight / $Image.Height)

        # 표시 폭입니다.
        $DrawWidth = [int]($Image.Width * $Scale)

        # 표시 높이입니다.
        $DrawHeight = [int]($Image.Height * $Scale)

        # 표시 X입니다.
        $DrawX = [int]($TileX + (($TileWidth - $DrawWidth) / 2))

        # 표시 Y입니다.
        $DrawY = [int]($TileY + 18 + ((205 - $DrawHeight) / 2))

        # 파일명 Label입니다.
        $Label = [System.IO.Path]::GetFileNameWithoutExtension($Paths[$Index])

        $Graphics.DrawRectangle($TilePen, $TileRect)
        $Graphics.DrawImage($Image, $DrawX, $DrawY, $DrawWidth, $DrawHeight)
        $Graphics.DrawString($Label, $LabelFont, $LabelBrush, [float]($TileX + 14), [float]($TileY + 235))
        $Image.Dispose()
    }

    # Review Sheet 출력 경로입니다.
    $ReviewPath = Join-Path $ReviewRoot 'P2_HUD_Review.png'

    $Sheet.Save($ReviewPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $LabelFont.Dispose()
    $LabelBrush.Dispose()
    $TilePen.Dispose()
    $Graphics.Dispose()
    $Sheet.Dispose()
    return $ReviewPath
}

Ensure-OutputDirectories

if ($RpmOnly -and $RadarOnly) {
    throw 'RpmOnly와 RadarOnly는 동시에 사용할 수 없습니다.'
}

if ($RadarOnly) {
    Write-Host '[RADAR ONLY] UI-P0-08B Radar 전용 Texture 7종 생성'

    # 이번 targeted 실행에서 생성할 Radar Source Art 절대 경로 목록입니다.
    $RadarPaths = New-Object System.Collections.Generic.List[string]

    Write-Host '[1/7] T_UI_RadarFrame 생성'
    $RadarPaths.Add((New-RadarFrame))

    Write-Host '[2/7] T_UI_RadarFriend 생성'
    $RadarPaths.Add((New-RadarMarker -Shape 'Friendly' -FileName 'T_UI_RadarFriend.png'))

    Write-Host '[3/7] T_UI_RadarHostile 생성'
    $RadarPaths.Add((New-RadarMarker -Shape 'Hostile' -FileName 'T_UI_RadarHostile.png'))

    Write-Host '[4/7] T_UI_RadarUnknown 생성'
    $RadarPaths.Add((New-RadarMarker -Shape 'Unknown' -FileName 'T_UI_RadarUnknown.png'))

    Write-Host '[5/7] T_UI_RadarPlayer 생성'
    $RadarPaths.Add((New-RadarMarker -Shape 'Player' -FileName 'T_UI_RadarPlayer.png'))

    Write-Host '[6/7] T_UI_TargetBracket 생성'
    $RadarPaths.Add((New-TargetBracket))

    Write-Host '[7/7] T_UI_RadarEdge 생성'
    $RadarPaths.Add((New-RadarEdgeBracket))

    # Radar 전용 파일별 기대 해상도입니다.
    $RadarExpectedSizes = @{
        'T_UI_RadarFrame.png' = @(512, 512)
        'T_UI_RadarFriend.png' = @(64, 64)
        'T_UI_RadarHostile.png' = @(64, 64)
        'T_UI_RadarUnknown.png' = @(64, 64)
        'T_UI_RadarPlayer.png' = @(64, 64)
        'T_UI_TargetBracket.png' = @(128, 128)
        'T_UI_RadarEdge.png' = @(128, 128)
    }

    # Radar Source Art 7종의 기술 검토 결과 목록입니다.
    $RadarReviewItems = New-Object System.Collections.Generic.List[object]

    foreach ($RadarPath in $RadarPaths) {
        # 현재 Radar PNG 파일명입니다.
        $RadarFileName = [System.IO.Path]::GetFileName($RadarPath)

        # 현재 Radar PNG의 기대 해상도입니다.
        $RadarExpectedSize = $RadarExpectedSizes[$RadarFileName]

        # 현재 Radar PNG의 Alpha/해상도/모서리 기술 검토 결과입니다.
        $RadarReview = Test-ArtPng -Path $RadarPath -ExpectedWidth $RadarExpectedSize[0] -ExpectedHeight $RadarExpectedSize[1]
        $RadarReviewItems.Add($RadarReview)
        Write-Host ("[RADAR 검토] {0} TechnicalPass={1} AlphaCoverage={2}%" -f $RadarReview.FileName, $RadarReview.TechnicalPass, $RadarReview.AlphaCoveragePercent)
    }

    # Radar Source Art 전체 기술 검토 PASS 여부입니다.
    $RadarTechnicalPass = (@($RadarReviewItems | Where-Object { -not $_.TechnicalPass }).Count -eq 0)

    Write-Host ("RADAR_TECHNICAL_PASS={0}" -f $RadarTechnicalPass)
    Write-Host 'RADAR_SOURCE_COUNT=7'
    Write-Host 'RADAR_TEXT_GLYPH_AS_ICON=0'

    if (-not $RadarTechnicalPass) {
        exit 2
    }

    exit 0
}

if ($RpmOnly) {
    Write-Host '[RPM ONLY] T_UI_RPMTrack 동적 Gauge 데이터 Texture 생성'

    # 이번 targeted 실행에서 재생성한 RPM Texture 절대 경로입니다.
    $RpmPath = New-RpmTrack

    # RPM Texture의 640x448 기본 PNG 기술 검토 결과입니다.
    $RpmReview = Test-ArtPng -Path $RpmPath -ExpectedWidth 640 -ExpectedHeight 448

    Write-Host ("RPM_TECHNICAL_PASS={0}" -f $RpmReview.TechnicalPass)
    Write-Host ("RPM_SOURCE_PATH={0}" -f $RpmPath)
    Write-Host 'RPM_DATA_CHANNELS=R:Progress,G:Intensity,B:Redline,A:Coverage'

    if (-not $RpmReview.TechnicalPass) {
        exit 2
    }

    exit 0
}

# 생성된 PNG 절대 경로 목록입니다.
$GeneratedPaths = New-Object System.Collections.Generic.List[string]

Write-Host '[1/9] T_UI_VehPanelFrame 생성'
$GeneratedPaths.Add((New-VehiclePanelFrame))

Write-Host '[2/9] T_UI_VehSil_LF 생성'
$GeneratedPaths.Add((New-VehicleSilhouette))

Write-Host '[3/9] T_UI_Armor_Front 생성'
$GeneratedPaths.Add((New-ArmorBadge -Direction 'Front' -FileName 'T_UI_Armor_Front.png'))

Write-Host '[4/9] T_UI_Armor_Right 생성'
$GeneratedPaths.Add((New-ArmorBadge -Direction 'Right' -FileName 'T_UI_Armor_Right.png'))

Write-Host '[5/9] T_UI_Armor_Rear 생성'
$GeneratedPaths.Add((New-ArmorBadge -Direction 'Rear' -FileName 'T_UI_Armor_Rear.png'))

Write-Host '[6/9] T_UI_Armor_Left 생성'
$GeneratedPaths.Add((New-ArmorBadge -Direction 'Left' -FileName 'T_UI_Armor_Left.png'))

Write-Host '[7/9] T_UI_Armor_Top 생성'
$GeneratedPaths.Add((New-ArmorBadge -Direction 'Top' -FileName 'T_UI_Armor_Top.png'))

Write-Host '[8/9] T_UI_Armor_Bottom 생성'
$GeneratedPaths.Add((New-ArmorBadge -Direction 'Bottom' -FileName 'T_UI_Armor_Bottom.png'))

Write-Host '[9/9] T_UI_RPMTrack 생성'
$GeneratedPaths.Add((New-RpmTrack))

# 파일별 기대 해상도입니다.
$ExpectedSizes = @{
    'T_UI_VehPanelFrame.png' = @(512, 256)
    'T_UI_VehSil_LF.png' = @(512, 256)
    'T_UI_Armor_Front.png' = @(128, 128)
    'T_UI_Armor_Right.png' = @(128, 128)
    'T_UI_Armor_Rear.png' = @(128, 128)
    'T_UI_Armor_Left.png' = @(128, 128)
    'T_UI_Armor_Top.png' = @(128, 128)
    'T_UI_Armor_Bottom.png' = @(128, 128)
    'T_UI_RPMTrack.png' = @(640, 448)
}

# 기술 검토 결과 목록입니다.
$ReviewItems = New-Object System.Collections.Generic.List[object]

foreach ($GeneratedPath in $GeneratedPaths) {
    # 현재 파일명입니다.
    $CurrentFileName = [System.IO.Path]::GetFileName($GeneratedPath)

    # 현재 기대 해상도입니다.
    $CurrentExpectedSize = $ExpectedSizes[$CurrentFileName]

    # 현재 파일 기술 검토 결과입니다.
    $ReviewItem = Test-ArtPng -Path $GeneratedPath -ExpectedWidth $CurrentExpectedSize[0] -ExpectedHeight $CurrentExpectedSize[1]

    $ReviewItems.Add($ReviewItem)
    Write-Host ("[검토] {0} TechnicalPass={1} AlphaCoverage={2}%" -f $ReviewItem.FileName, $ReviewItem.TechnicalPass, $ReviewItem.AlphaCoveragePercent)
}

# 사용자 검토용 3x3 합본 경로입니다.
$ReviewSheetPath = New-ReviewSheet -Paths $GeneratedPaths.ToArray()

# 전체 기술 검토 PASS 여부입니다.
$AllTechnicalPass = (@($ReviewItems | Where-Object { -not $_.TechnicalPass }).Count -eq 0)

# P2 Source Art 자체의 기술 상태만 기록하고 Unreal Apply 상태는 추적하지 않는 검토 리포트입니다.
$ReviewReport = [PSCustomObject]@{
    SchemaVersion = 'cf_ui_hud_art_review_v2'
    WorkId = 'CF-FQ-032-VF-03-P2'
                                GeneratorVersion = '1.2.0'
    GeneratedAt = (Get-Date).ToString('yyyy-MM-ddTHH:mm:ssK')
    ReviewScope = 'SourceTechnicalReviewOnly'
    UnrealApplyStateTrackedByThisReport = $false
    SourceRoot = 'SourceArt/UI/HUD/P2'
    VisualDirection = 'Balanced Combat'
    ArmorLayoutContract = 'FrontLeft_RightUp_RearRight_LeftDown_TopUpperLeft_BottomLowerRight'
    AllTechnicalPass = $AllTechnicalPass
    UserVisualApprovalPending = $true
    ApprovedAssets = @()
    Items = $ReviewItems
    ReviewSheet = 'Review/P2_HUD_Review.png'
}

# JSON 검토 리포트 경로입니다.
$ReviewJsonPath = Join-Path $ReviewRoot 'p2_review.json'

$ReviewReport | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ReviewJsonPath -Encoding UTF8

Write-Host ("P2_TECHNICAL_PASS={0}" -f $AllTechnicalPass)
Write-Host ("P2_USER_APPROVAL_PENDING={0}" -f $true)
Write-Host ("P2_UNREAL_APPLY_STATE_TRACKED={0}" -f $false)
Write-Host ("P2_REVIEW_SHEET={0}" -f $ReviewSheetPath)
Write-Host ("P2_REVIEW_JSON={0}" -f $ReviewJsonPath)

if (-not $AllTechnicalPass) {
    exit 2
}

exit 0
