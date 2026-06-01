$ErrorActionPreference = "Stop"

# 한글 진행 메시지를 UTF-8 콘솔 출력으로 표시하기 위한 인코딩입니다.
[Console]::OutputEncoding = [System.Text.UTF8Encoding]::new($false)

# publish.ps1이 위치한 CFServerLauncher 루트 폴더 경로입니다.
$scriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $scriptRoot

# 런처 publish 산출물을 생성할 출력 폴더 경로입니다.
$publishPath = Join-Path $scriptRoot "Publish"

# publish 대상 WPF 프로젝트 파일 경로입니다.
$projectPath = ".\src\CFServerLauncher\CFServerLauncher.csproj"

Write-Host "CFServerLauncher v1.1.0 배포 출력을 준비합니다."
Write-Host "기존 Publish 폴더를 새로 고칩니다."

if (Test-Path $publishPath) {
    Remove-Item -LiteralPath $publishPath -Recurse -Force
}

New-Item -ItemType Directory -Path $publishPath | Out-Null

Write-Host "런처 애플리케이션을 Release/win-x64로 publish합니다."
dotnet publish $projectPath -c Release -r win-x64 --self-contained false -o ".\Publish"

Write-Host "배포 출력 생성이 완료되었습니다: $publishPath"
Write-Host "주의: Server, Config, Logs 폴더와 서버 빌드 파일은 자동으로 포함하거나 복사하지 않습니다."
