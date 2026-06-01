# CFServerLauncher v1.0 릴리스

Version: v1.0.0

## 목적

이 문서는 CFServerLauncher v1.0.0 최종 릴리스의 빌드, 실행 검증, 로컬 publish 절차를 정리한다.

## 빌드 검증

PowerShell에서 저장소 루트로 이동한 뒤 아래 명령을 실행한다.

```powershell
cd D:\Work\CarFight_git
dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln
```

예상 결과:

- 경고 0개
- 오류 0개
- `CFServerLauncher.dll` 빌드 성공

## 로컬 실행 검증

```powershell
cd D:\Work\CarFight_git
dotnet run --project .\Tools\CFServerLauncher\src\CFServerLauncher\CFServerLauncher.csproj
```

확인 항목:

- 앱 창이 열린다.
- 하단에 `버전: v1.0.0`이 표시된다.
- 서버 시작, 로그 표시, 서버 종료, 재시작이 정상 동작한다.
- 서버가 실행 중이 아닐 때 창을 닫으면 정상 종료된다.
- 서버가 실행 중일 때 창을 닫으면 한국어 확인 창이 표시된다.
- 확인 창에서 `아니오`를 선택하면 런처와 서버가 유지된다.
- 확인 창에서 `예`를 선택하면 서버 종료 후 런처가 닫힌다.

## publish.ps1 사용

PowerShell에서 `Tools\CFServerLauncher` 폴더로 이동한 뒤 아래 명령을 실행한다.

```powershell
cd D:\Work\CarFight_git\Tools\CFServerLauncher
.\publish.ps1
```

예상 결과:

- `D:\Work\CarFight_git\Tools\CFServerLauncher\Publish` 폴더가 생성되거나 새로 고쳐진다.
- `CFServerLauncher.exe`가 `Publish` 폴더 안에 생성된다.
- `Server`, `Config`, `Logs` 폴더는 삭제되지 않는다.
- 서버 빌드 파일은 자동으로 복사되거나 포함되지 않는다.

## 직접 publish 대체 명령

스크립트를 사용할 수 없는 경우 아래 명령을 직접 실행한다.

```powershell
cd D:\Work\CarFight_git\Tools\CFServerLauncher
dotnet publish .\src\CFServerLauncher\CFServerLauncher.csproj -c Release -r win-x64 --self-contained false -o .\Publish
```

## 최종 릴리스 체크리스트

- `dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln` 성공
- 앱 하단 버전 표시가 `버전: v1.0.0`
- 서버 시작 성공
- TestMap 로그 표시 확인
- 서버 종료 성공
- 서버 종료 후 재시작 성공
- 서버 실행 파일 찾아보기 동작 확인
- 작업 폴더 찾아보기 동작 확인
- 로그 파일 찾아보기 동작 확인
- `ActiveLogFilePath`에 실제 런타임 로그 경로 표시
- 기본 `LogFilePath`가 타임스탬프 로그 경로로 덮어써지지 않음
- 서버 실행 중 런처 닫기 확인 창 표시
- 닫기 확인 창에서 `아니오` 선택 시 닫기 취소
- 닫기 확인 창에서 `예` 선택 시 서버 종료 후 런처 종료
- `.\publish.ps1` 실행 후 `Publish\CFServerLauncher.exe` 생성
- `Server`, `Config`, `Logs` 폴더 유지

## Changelog

- v1.0.0: 릴리스 후보 버전 표시를 최종 버전 표시로 변경.
- v1.0.0: `publish.ps1` 추가.
- v1.0.0: 최종 릴리스 검증 체크리스트 추가.
- v1.0.0-rc1: 서버 실행 중 런처 종료 시 한국어 확인 창 추가.
- v1.0.0-rc1: 로컬 빌드와 publish 명령 문서화.

## 마이그레이션 지침

기존 `Config\server.local.json` 설정 파일은 변경하지 않는다. 기존 사용자는 앱을 다시 빌드하거나 `Publish` 폴더를 새로 생성한 뒤 기존 설정을 그대로 사용할 수 있다.

`publish.ps1`은 런처 애플리케이션만 publish한다. Unreal Dedicated Server 빌드 파일은 자동으로 패키징하거나 복사하지 않으므로, 배포 시 서버 빌드 파일은 별도 절차로 준비한다.
