# Copyright (c) CarFight. All Rights Reserved.
#
# Version: 1.0.1
# Date: 2026-08-11
# Description: CF-FQ-032 D1-11-ART-01 P1 HUD Texture 9종의 승인 전 PNG 원본과 기술 검토 리포트를 생성합니다.
# Scope: SourceArt/UI/HUD/P1에만 출력하며 Unreal Import, DataAsset 연결, Widget 수정은 수행하지 않습니다.
# Changelog:
# - v1.0.1: StrictMode 빈 결과 Count 보정과 반투명 가장자리 Black Halo 위험 검사를 추가.
# - v1.0.0: Vehicle Silhouette 1, Armor Plate 6, Speed Arc 1, Target Bracket 1과 Review Sheet/JSON 생성.
# Migration:
# - 사용자 시각 승인 전 생성 PNG를 /Game 경로로 Import하지 않습니다.

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Drawing

# 저장소 루트 절대 경로입니다.
$RepositoryRoot = (Resolve-Path (Join-Path $PSScriptRoot '..')).Path

# 승인 전 원본 아트를 저장할 루트 경로입니다.
$OutputRoot = Join-Path $RepositoryRoot 'SourceArt\UI\HUD\P1'

# 사용자 검토용 합본과 기술 리포트를 저장할 경로입니다.
$ReviewRoot = Join-Path $OutputRoot 'Review'

# 밝은 Tintable 주 실루엣 색입니다.
$MainColor = [System.Drawing.Color]::FromArgb(236, 238, 244, 246)

# 보조 구조와 내부선을 위한 Tintable 회색입니다.
$SecondaryColor = [System.Drawing.Color]::FromArgb(156, 176, 190, 200)

# 낮은 우선순위 보조선을 위한 반투명 회색입니다.
$SubtleColor = [System.Drawing.Color]::FromArgb(96, 164, 180, 190)

# 투명 영역 RGB 오염을 줄이기 위한 Transparent White 배경색입니다.
$TransparentWhite = [System.Drawing.Color]::FromArgb(0, 255, 255, 255)

# 주 외곽선 두께입니다.
$MainStrokeWidth = 6.0

# 보조 외곽선 두께입니다.
$SecondaryStrokeWidth = 3.0

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

# 주/보조 Pen을 동일한 Tactical Cut 계열로 만듭니다.
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

# Point 좌표 목록을 강타입 배열로 변환합니다.
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
        $X = $Coordinates[$Index * 2]
        # 현재 Point의 Y 좌표입니다.
        $Y = $Coordinates[($Index * 2) + 1]
        $Points[$Index] = New-Object System.Drawing.Point($X, $Y)
    }

    return $Points
}

# 완성된 Bitmap을 PNG로 저장하고 Graphics 자원을 정리합니다.
function Save-ArtCanvas {
    param(
        # 저장할 Canvas입니다.
        [Parameter(Mandatory = $true)]$Canvas,
        # 파일명 확장자를 제외하지 않은 PNG 파일명입니다.
        [Parameter(Mandatory = $true)][string]$FileName
    )

    # 최종 출력 파일 절대 경로입니다.
    $OutputPath = Join-Path $OutputRoot $FileName

    $Canvas.Bitmap.Save($OutputPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $Canvas.Graphics.Dispose()
    $Canvas.Bitmap.Dispose()
    return $OutputPath
}

# 왼쪽 전방이 명확한 차량 실루엣 Texture를 생성합니다.
function New-VehicleSilhouette {
    # 차량 실루엣 캔버스입니다.
    $Canvas = New-ArtCanvas -Width 512 -Height 256
    # 차량 실루엣 Graphics입니다.
    $Graphics = $Canvas.Graphics
    # 주 면 Brush입니다.
    $MainBrush = New-ArtBrush -Color $MainColor
    # 보조 면 Brush입니다.
    $SecondaryBrush = New-ArtBrush -Color $SecondaryColor
    # 주 외곽선 Pen입니다.
    $MainPen = New-ArtPen -Color $MainColor -Width $MainStrokeWidth
    # 내부 구조 Pen입니다.
    $SecondaryPen = New-ArtPen -Color $SecondaryColor -Width $SecondaryStrokeWidth

    # 왼쪽으로 뾰족한 Nose와 우측 Tail을 가진 차체 외곽입니다.
    $BodyPoints = New-PointArray -Coordinates @(54,128, 92,82, 178,64, 360,70, 430,92, 452,128, 430,164, 360,186, 178,192, 92,174)
    # Cabin/상부 장갑 면입니다.
    $CabinPoints = New-PointArray -Coordinates @(152,100, 210,78, 330,84, 378,108, 378,148, 330,172, 210,178, 152,156)
    # 왼쪽 Front Shoulder 절삭면입니다.
    $FrontCutPoints = New-PointArray -Coordinates @(70,128, 112,96, 140,96, 126,128, 140,160, 112,160)
    # 오른쪽 Tail 절삭면입니다.
    $RearCutPoints = New-PointArray -Coordinates @(396,100, 426,110, 440,128, 426,146, 396,156, 406,128)

    $Graphics.FillPolygon($SecondaryBrush, $BodyPoints)
    $Graphics.DrawPolygon($MainPen, $BodyPoints)
    $Graphics.DrawPolygon($SecondaryPen, $CabinPoints)
    $Graphics.DrawPolygon($SecondaryPen, $FrontCutPoints)
    $Graphics.DrawPolygon($SecondaryPen, $RearCutPoints)
    $Graphics.DrawLine($SecondaryPen, 142,128, 390,128)
    $Graphics.DrawLine($SecondaryPen, 212,86, 212,170)
    $Graphics.DrawLine($SecondaryPen, 326,90, 326,166)

    # 좌상 Front Wheel 외곽입니다.
    $Graphics.FillRectangle($MainBrush, 130,58,62,14)
    # 좌하 Front Wheel 외곽입니다.
    $Graphics.FillRectangle($MainBrush, 130,184,62,14)
    # 우상 Rear Wheel 외곽입니다.
    $Graphics.FillRectangle($MainBrush, 344,64,62,14)
    # 우하 Rear Wheel 외곽입니다.
    $Graphics.FillRectangle($MainBrush, 344,178,62,14)

    $MainBrush.Dispose()
    $SecondaryBrush.Dispose()
    $MainPen.Dispose()
    $SecondaryPen.Dispose()

    return (Save-ArtCanvas -Canvas $Canvas -FileName 'T_UI_VehSil_LF.png')
}

# 방향별로 다른 Tactical Cut을 가진 Armor Plate Texture 하나를 생성합니다.
function New-ArmorPlate {
    param(
        # 생성할 방향 이름입니다.
        [Parameter(Mandatory = $true)][ValidateSet('Front','Right','Rear','Left','Top','Bottom')][string]$Direction,
        # 출력 PNG 파일명입니다.
        [Parameter(Mandatory = $true)][string]$FileName
    )

    # Armor Plate 캔버스입니다.
    $Canvas = New-ArtCanvas -Width 128 -Height 128
    # Armor Plate Graphics입니다.
    $Graphics = $Canvas.Graphics
    # Armor Plate 면 Brush입니다.
    $PlateBrush = New-ArtBrush -Color ([System.Drawing.Color]::FromArgb(118, 196, 207, 214))
    # Armor Plate 주 외곽선입니다.
    $MainPen = New-ArtPen -Color $MainColor -Width 5.0
    # Armor Plate 보조 구조선입니다.
    $SecondaryPen = New-ArtPen -Color $SecondaryColor -Width 3.0

    switch ($Direction) {
        'Front' {
            # 전면은 화면 왼쪽을 향하는 전술적 Wedge입니다.
            $PlatePoints = New-PointArray -Coordinates @(14,64, 34,26, 98,26, 114,40, 114,88, 98,102, 34,102)
            $Graphics.FillPolygon($PlateBrush, $PlatePoints)
            $Graphics.DrawPolygon($MainPen, $PlatePoints)
            $Graphics.DrawLine($SecondaryPen, 34,44, 92,44)
            $Graphics.DrawLine($SecondaryPen, 34,84, 92,84)
            $Graphics.DrawLine($SecondaryPen, 34,44, 22,64)
            $Graphics.DrawLine($SecondaryPen, 22,64, 34,84)
        }
        'Rear' {
            # 후면은 화면 오른쪽을 향하고 Front보다 Tail 절삭이 강한 Plate입니다.
            $PlatePoints = New-PointArray -Coordinates @(14,40, 30,26, 94,26, 114,64, 94,102, 30,102, 14,88)
            $Graphics.FillPolygon($PlateBrush, $PlatePoints)
            $Graphics.DrawPolygon($MainPen, $PlatePoints)
            $Graphics.DrawLine($SecondaryPen, 36,44, 92,44)
            $Graphics.DrawLine($SecondaryPen, 36,84, 92,84)
            $Graphics.DrawLine($SecondaryPen, 92,44, 106,64)
            $Graphics.DrawLine($SecondaryPen, 106,64, 92,84)
        }
        'Right' {
            # 우측면은 화면 위쪽으로 열린 Shoulder Cut을 가집니다.
            $PlatePoints = New-PointArray -Coordinates @(28,24, 48,12, 80,12, 100,24, 112,48, 104,102, 88,114, 40,114, 24,102, 16,48)
            $Graphics.FillPolygon($PlateBrush, $PlatePoints)
            $Graphics.DrawPolygon($MainPen, $PlatePoints)
            $Graphics.DrawLine($SecondaryPen, 38,34, 64,20)
            $Graphics.DrawLine($SecondaryPen, 64,20, 90,34)
            $Graphics.DrawLine($SecondaryPen, 34,52, 94,52)
            $Graphics.DrawLine($SecondaryPen, 30,92, 98,92)
        }
        'Left' {
            # 좌측면은 화면 아래쪽으로 열린 Shoulder Cut을 가집니다.
            $PlatePoints = New-PointArray -Coordinates @(40,14, 88,14, 104,26, 112,80, 100,104, 80,116, 48,116, 28,104, 16,80, 24,26)
            $Graphics.FillPolygon($PlateBrush, $PlatePoints)
            $Graphics.DrawPolygon($MainPen, $PlatePoints)
            $Graphics.DrawLine($SecondaryPen, 30,36, 98,36)
            $Graphics.DrawLine($SecondaryPen, 34,76, 94,76)
            $Graphics.DrawLine($SecondaryPen, 38,94, 64,108)
            $Graphics.DrawLine($SecondaryPen, 64,108, 90,94)
        }
        'Top' {
            # 상부는 본체 4방향보다 작은 Roof Badge 형태입니다.
            $PlatePoints = New-PointArray -Coordinates @(34,24, 94,24, 108,38, 102,90, 88,104, 40,104, 26,90, 20,38)
            $Graphics.FillPolygon($PlateBrush, $PlatePoints)
            $Graphics.DrawPolygon($MainPen, $PlatePoints)
            $Graphics.DrawLine($SecondaryPen, 42,42, 86,42)
            $Graphics.DrawLine($SecondaryPen, 34,62, 94,62)
            $Graphics.DrawLine($SecondaryPen, 42,84, 86,84)
            $Graphics.DrawLine($SecondaryPen, 64,42, 64,84)
        }
        'Bottom' {
            # 하부는 Skid Plate처럼 아래 중앙이 절삭된 Badge 형태입니다.
            $PlatePoints = New-PointArray -Coordinates @(28,24, 100,24, 108,40, 100,94, 82,108, 64,98, 46,108, 28,94, 20,40)
            $Graphics.FillPolygon($PlateBrush, $PlatePoints)
            $Graphics.DrawPolygon($MainPen, $PlatePoints)
            $Graphics.DrawLine($SecondaryPen, 38,44, 90,44)
            $Graphics.DrawLine($SecondaryPen, 34,68, 94,68)
            $Graphics.DrawLine($SecondaryPen, 46,88, 64,78)
            $Graphics.DrawLine($SecondaryPen, 64,78, 82,88)
        }
    }

    $PlateBrush.Dispose()
    $MainPen.Dispose()
    $SecondaryPen.Dispose()

    return (Save-ArtCanvas -Canvas $Canvas -FileName $FileName)
}

# 정적 부분 원호 Track과 Tick만 포함한 Speed Arc Texture를 생성합니다.
function New-SpeedArc {
    # Speed Arc 캔버스입니다.
    $Canvas = New-ArtCanvas -Width 512 -Height 512
    # Speed Arc Graphics입니다.
    $Graphics = $Canvas.Graphics
    # 주 원호 Pen입니다.
    $MainPen = New-ArtPen -Color $MainColor -Width 8.0
    # 보조 원호 Pen입니다.
    $SecondaryPen = New-ArtPen -Color $SecondaryColor -Width 3.0
    # 약한 보조 원호 Pen입니다.
    $SubtlePen = New-ArtPen -Color $SubtleColor -Width 2.0
    # 주 원호 Bounding Rectangle입니다.
    $ArcRect = New-Object System.Drawing.Rectangle(54, 54, 404, 404)
    # 보조 원호 Bounding Rectangle입니다.
    $InnerArcRect = New-Object System.Drawing.Rectangle(82, 82, 348, 348)

    $Graphics.DrawArc($MainPen, $ArcRect, 138.0, 224.0)
    $Graphics.DrawArc($SubtlePen, $InnerArcRect, 145.0, 210.0)

    # Tick 시작 각도입니다.
    $StartAngle = 142.0
    # Tick 종료 각도입니다.
    $EndAngle = 358.0
    # Tick 간격 각도입니다.
    $AngleStep = 12.0
    # 원호 중심 X 좌표입니다.
    $CenterX = 256.0
    # 원호 중심 Y 좌표입니다.
    $CenterY = 256.0

    # Tick 순번입니다.
    $TickIndex = 0
    for ($Angle = $StartAngle; $Angle -le $EndAngle; $Angle += $AngleStep) {
        # 라디안 각도입니다.
        $Radians = $Angle * [Math]::PI / 180.0
        # 4개마다 긴 Major Tick을 사용합니다.
        $IsMajorTick = (($TickIndex % 4) -eq 0)
        # Tick 외곽 반지름입니다.
        $OuterRadius = 205.0
        # Tick 내부 반지름입니다.
        $InnerRadius = if ($IsMajorTick) { 174.0 } else { 184.0 }
        # Tick 시작 X입니다.
        $StartX = $CenterX + ([Math]::Cos($Radians) * $OuterRadius)
        # Tick 시작 Y입니다.
        $StartY = $CenterY + ([Math]::Sin($Radians) * $OuterRadius)
        # Tick 끝 X입니다.
        $EndX = $CenterX + ([Math]::Cos($Radians) * $InnerRadius)
        # Tick 끝 Y입니다.
        $EndY = $CenterY + ([Math]::Sin($Radians) * $InnerRadius)
        # Tick에 사용할 Pen입니다.
        $TickPen = if ($IsMajorTick) { $MainPen } else { $SecondaryPen }

        $Graphics.DrawLine($TickPen, [float]$StartX, [float]$StartY, [float]$EndX, [float]$EndY)
        $TickIndex++
    }

    # 시작점을 구분하는 짧은 Tactical Cut입니다.
    $Graphics.DrawLine($SecondaryPen, 78,366, 104,350)
    # 끝점을 구분하는 짧은 Tactical Cut입니다.
    $Graphics.DrawLine($SecondaryPen, 452,250, 430,236)

    $MainPen.Dispose()
    $SecondaryPen.Dispose()
    $SubtlePen.Dispose()

    return (Save-ArtCanvas -Canvas $Canvas -FileName 'T_UI_SpeedArc.png')
}

# 중앙이 완전히 비어 있는 4-Corner Target Bracket Texture를 생성합니다.
function New-TargetBracket {
    # Target Bracket 캔버스입니다.
    $Canvas = New-ArtCanvas -Width 256 -Height 256
    # Target Bracket Graphics입니다.
    $Graphics = $Canvas.Graphics
    # 주 Bracket Pen입니다.
    $MainPen = New-ArtPen -Color $MainColor -Width 8.0
    # 보조 Corner Marker Pen입니다.
    $SecondaryPen = New-ArtPen -Color $SecondaryColor -Width 3.0

    # 좌상단 Corner Segment입니다.
    $Graphics.DrawLines($MainPen, (New-PointArray -Coordinates @(34,88, 34,48, 48,34, 88,34)))
    # 우상단 Corner Segment입니다.
    $Graphics.DrawLines($MainPen, (New-PointArray -Coordinates @(168,34, 208,34, 222,48, 222,88)))
    # 좌하단 Corner Segment입니다.
    $Graphics.DrawLines($MainPen, (New-PointArray -Coordinates @(34,168, 34,208, 48,222, 88,222)))
    # 우하단 Corner Segment입니다.
    $Graphics.DrawLines($MainPen, (New-PointArray -Coordinates @(168,222, 208,222, 222,208, 222,168)))

    # 좌상단 외곽 Tactical Marker입니다.
    $Graphics.DrawLine($SecondaryPen, 22,72, 22,44)
    $Graphics.DrawLine($SecondaryPen, 44,22, 72,22)
    # 우상단 외곽 Tactical Marker입니다.
    $Graphics.DrawLine($SecondaryPen, 184,22, 212,22)
    $Graphics.DrawLine($SecondaryPen, 234,44, 234,72)
    # 좌하단 외곽 Tactical Marker입니다.
    $Graphics.DrawLine($SecondaryPen, 22,184, 22,212)
    $Graphics.DrawLine($SecondaryPen, 44,234, 72,234)
    # 우하단 외곽 Tactical Marker입니다.
    $Graphics.DrawLine($SecondaryPen, 184,234, 212,234)
    $Graphics.DrawLine($SecondaryPen, 234,184, 234,212)

    $MainPen.Dispose()
    $SecondaryPen.Dispose()

    return (Save-ArtCanvas -Canvas $Canvas -FileName 'T_UI_TargetBracket.png')
}

# PNG의 크기, Alpha 사용, 투명 모서리, 실제 픽셀 Bounds를 기술적으로 검증합니다.
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
                if ($Pixel.A -lt 255 -and $Pixel.R -lt 32 -and $Pixel.G -lt 32 -and $Pixel.B -lt 32) {
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
    # 비투명 픽셀 비율입니다.
    $AlphaCoveragePercent = [Math]::Round(($VisiblePixelCount * 100.0) / $TotalPixelCount, 2)

    # 네 모서리 픽셀입니다.
    $CornerPixels = @(
        $Bitmap.GetPixel(0, 0),
        $Bitmap.GetPixel($Bitmap.Width - 1, 0),
        $Bitmap.GetPixel(0, $Bitmap.Height - 1),
        $Bitmap.GetPixel($Bitmap.Width - 1, $Bitmap.Height - 1)
    )

    # 네 모서리가 모두 완전 투명인지 여부입니다.
        $TransparentCorners = (@($CornerPixels | Where-Object { $_.A -ne 0 }).Count -eq 0)
    # 네 모서리의 투명 RGB가 White로 유지되는지 여부입니다.
        $TransparentWhiteCorners = (@($CornerPixels | Where-Object { $_.R -lt 240 -or $_.G -lt 240 -or $_.B -lt 240 }).Count -eq 0)
    # 기대 해상도와 일치하는지 여부입니다.
    $SizeMatches = ($Bitmap.Width -eq $ExpectedWidth -and $Bitmap.Height -eq $ExpectedHeight)
    # 빈 이미지가 아닌지 여부입니다.
    $HasVisiblePixels = ($VisiblePixelCount -gt 0)
    # 이미지가 전체 캔버스를 불투명하게 덮지 않는지 여부입니다.
    $HasTransparentArea = ($VisiblePixelCount -lt $TotalPixelCount)
        # 반투명 가장자리에서 검은 Halo 위험 픽셀이 발견됐는지 여부입니다.
    $HasBlackAlphaEdgeRisk = ($DarkTranslucentPixelCount -gt 0)
    # 기술 검토 전체 PASS 여부입니다.
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
                TransparentWhiteCorners = $TransparentWhiteCorners
        DarkTranslucentPixelCount = $DarkTranslucentPixelCount
        HasBlackAlphaEdgeRisk = $HasBlackAlphaEdgeRisk
        NoTextByConstruction = $true
        TintableWhiteGrayByConstruction = $true
        TechnicalPass = $TechnicalPass
        UserVisualApproval = $false
        ImportAllowed = $false
    }
}

# 9종 PNG를 한 화면에서 확인할 수 있는 사용자 검토용 Contact Sheet를 생성합니다.
function New-ReviewSheet {
    param(
        # 검토할 PNG 절대 경로 목록입니다.
        [Parameter(Mandatory = $true)][string[]]$Paths
    )

    # Review Sheet 폭입니다.
    $SheetWidth = 1080
    # Review Sheet 높이입니다.
    $SheetHeight = 840
    # Review Sheet Bitmap입니다.
    $Sheet = New-Object System.Drawing.Bitmap($SheetWidth, $SheetHeight, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    # Review Sheet Graphics입니다.
    $Graphics = [System.Drawing.Graphics]::FromImage($Sheet)
    # 어두운 HUD 검토 배경색입니다.
    $BackgroundColor = [System.Drawing.Color]::FromArgb(255, 11, 17, 23)
    # Tile 외곽선 색입니다.
    $TileLineColor = [System.Drawing.Color]::FromArgb(255, 87, 112, 128)
    # Label 색입니다.
    $LabelColor = [System.Drawing.Color]::FromArgb(255, 234, 242, 247)
    # Tile 외곽선 Pen입니다.
    $TilePen = New-ArtPen -Color $TileLineColor -Width 2.0
    # Label Brush입니다.
    $LabelBrush = New-ArtBrush -Color $LabelColor
    # 파일명 Label Font입니다.
    $LabelFont = New-Object System.Drawing.Font('Segoe UI', 11.0, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)

    $Graphics.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
    $Graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    $Graphics.Clear($BackgroundColor)

    # Tile 폭입니다.
    $TileWidth = 340
    # Tile 높이입니다.
    $TileHeight = 250
    # Tile 좌측 여백입니다.
    $MarginX = 20
    # Tile 상단 여백입니다.
    $MarginY = 20
    # Tile 사이 가로 간격입니다.
    $GapX = 10
    # Tile 사이 세로 간격입니다.
    $GapY = 15

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
        $MaxImageWidth = 300.0
        # 이미지 표시 최대 높이입니다.
        $MaxImageHeight = 190.0
        # 원본 종횡비를 유지하는 Scale입니다.
        $Scale = [Math]::Min($MaxImageWidth / $Image.Width, $MaxImageHeight / $Image.Height)
        # 표시 폭입니다.
        $DrawWidth = [int]($Image.Width * $Scale)
        # 표시 높이입니다.
        $DrawHeight = [int]($Image.Height * $Scale)
        # 표시 X입니다.
        $DrawX = [int]($TileX + (($TileWidth - $DrawWidth) / 2))
        # 표시 Y입니다.
        $DrawY = [int]($TileY + 18 + ((190 - $DrawHeight) / 2))
        # 파일명 Label입니다.
        $Label = [System.IO.Path]::GetFileNameWithoutExtension($Paths[$Index])

        $Graphics.DrawRectangle($TilePen, $TileRect)
        $Graphics.DrawImage($Image, $DrawX, $DrawY, $DrawWidth, $DrawHeight)
        $Graphics.DrawString($Label, $LabelFont, $LabelBrush, [float]($TileX + 12), [float]($TileY + 218))
        $Image.Dispose()
    }

    # Review Sheet 출력 경로입니다.
    $ReviewPath = Join-Path $ReviewRoot 'P1_HUD_Review.png'

    $Sheet.Save($ReviewPath, [System.Drawing.Imaging.ImageFormat]::Png)
    $LabelFont.Dispose()
    $LabelBrush.Dispose()
    $TilePen.Dispose()
    $Graphics.Dispose()
    $Sheet.Dispose()
    return $ReviewPath
}

Ensure-OutputDirectories

# 생성된 PNG 절대 경로 목록입니다.
$GeneratedPaths = New-Object System.Collections.Generic.List[string]

Write-Host '[1/9] T_UI_VehSil_LF 생성'
$GeneratedPaths.Add((New-VehicleSilhouette))

Write-Host '[2/9] T_UI_Armor_Front 생성'
$GeneratedPaths.Add((New-ArmorPlate -Direction 'Front' -FileName 'T_UI_Armor_Front.png'))

Write-Host '[3/9] T_UI_Armor_Right 생성'
$GeneratedPaths.Add((New-ArmorPlate -Direction 'Right' -FileName 'T_UI_Armor_Right.png'))

Write-Host '[4/9] T_UI_Armor_Rear 생성'
$GeneratedPaths.Add((New-ArmorPlate -Direction 'Rear' -FileName 'T_UI_Armor_Rear.png'))

Write-Host '[5/9] T_UI_Armor_Left 생성'
$GeneratedPaths.Add((New-ArmorPlate -Direction 'Left' -FileName 'T_UI_Armor_Left.png'))

Write-Host '[6/9] T_UI_Armor_Top 생성'
$GeneratedPaths.Add((New-ArmorPlate -Direction 'Top' -FileName 'T_UI_Armor_Top.png'))

Write-Host '[7/9] T_UI_Armor_Bottom 생성'
$GeneratedPaths.Add((New-ArmorPlate -Direction 'Bottom' -FileName 'T_UI_Armor_Bottom.png'))

Write-Host '[8/9] T_UI_SpeedArc 생성'
$GeneratedPaths.Add((New-SpeedArc))

Write-Host '[9/9] T_UI_TargetBracket 생성'
$GeneratedPaths.Add((New-TargetBracket))

# 파일별 기대 해상도입니다.
$ExpectedSizes = @{
    'T_UI_VehSil_LF.png' = @(512, 256)
    'T_UI_Armor_Front.png' = @(128, 128)
    'T_UI_Armor_Right.png' = @(128, 128)
    'T_UI_Armor_Rear.png' = @(128, 128)
    'T_UI_Armor_Left.png' = @(128, 128)
    'T_UI_Armor_Top.png' = @(128, 128)
    'T_UI_Armor_Bottom.png' = @(128, 128)
    'T_UI_SpeedArc.png' = @(512, 512)
    'T_UI_TargetBracket.png' = @(256, 256)
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

# ART-01 현재 검토 리포트입니다.
$ReviewReport = [PSCustomObject]@{
    SchemaVersion = 'cf_ui_hud_art_review_v1'
    WorkId = 'CF-FQ-032-D1-11-ART-01'
        GeneratorVersion = '1.0.1'
    GeneratedAt = (Get-Date).ToString('yyyy-MM-ddTHH:mm:ssK')
    SourceRoot = 'SourceArt/UI/HUD/P1'
    UnrealImportPerformed = $false
    DataAssetConnectionPerformed = $false
    AllTechnicalPass = $AllTechnicalPass
    UserVisualApprovalPending = $true
    ApprovedAssets = @()
    Items = $ReviewItems
    ReviewSheet = 'Review/P1_HUD_Review.png'
}

# JSON 검토 리포트 경로입니다.
$ReviewJsonPath = Join-Path $ReviewRoot 'art01_review.json'

$ReviewReport | ConvertTo-Json -Depth 8 | Set-Content -LiteralPath $ReviewJsonPath -Encoding UTF8

Write-Host ("ART01_TECHNICAL_PASS={0}" -f $AllTechnicalPass)
Write-Host ("ART01_USER_APPROVAL_PENDING={0}" -f $true)
Write-Host ("ART01_IMPORT_PERFORMED={0}" -f $false)
Write-Host ("ART01_REVIEW_SHEET={0}" -f $ReviewSheetPath)
Write-Host ("ART01_REVIEW_JSON={0}" -f $ReviewJsonPath)

if (-not $AllTechnicalPass) {
    exit 2
}

exit 0
