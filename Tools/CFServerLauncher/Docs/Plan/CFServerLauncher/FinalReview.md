# CFServerLauncher v1.0 최종 릴리스 리뷰

문서 버전: v1.0.8-review  
작성 기준일: 2026-05-29

## 1. 리뷰 결론

Codex의 `v1.0 Final Release` 작업은 코드 리뷰 기준으로 통과로 본다.

확인된 항목:

- 앱 표시 버전이 `버전: v1.0.0`으로 변경됨
- `publish.ps1` 추가됨
- `publish.ps1`이 `Publish` 폴더로 WPF 앱을 publish하도록 구성됨
- `publish.ps1`에 `$ErrorActionPreference = "Stop"` 적용됨
- `publish.ps1`이 한국어 진행 메시지를 출력함
- `publish.ps1`은 `Server`, `Config`, `Logs` 폴더를 직접 삭제하지 않음
- `Docs/release.md`가 v1.0.0 최종 릴리스 기준으로 갱신됨
- `Docs/release.md`에 `publish.ps1` 사용법과 직접 `dotnet publish` 대체 명령이 모두 포함됨

## 2. 확인한 파일

```text
Tools\CFServerLauncher\src\CFServerLauncher\ViewModels\MainViewModel.cs
Tools\CFServerLauncher\publish.ps1
Tools\CFServerLauncher\Docs\release.md
```

## 3. 코드 리뷰 세부 결과

### 3.1 버전 표시

`MainViewModel.cs`의 버전 표시 문자열이 최종 릴리스 버전으로 변경되었다.

```text
버전: v1.0.0
```

### 3.2 publish.ps1

`Tools\CFServerLauncher\publish.ps1`이 추가되었다.

동작 요약:

```text
1. ErrorActionPreference를 Stop으로 설정
2. 스크립트 위치를 CFServerLauncher 루트로 설정
3. Publish 폴더를 새로 고침
4. dotnet publish 실행
5. Server, Config, Logs 폴더와 서버 빌드 파일은 자동 포함하지 않는다고 안내
```

### 3.3 release.md

`Tools\CFServerLauncher\Docs\release.md`가 v1.0.0 기준으로 갱신되었다.

포함 내용:

- 빌드 검증 명령
- 로컬 실행 검증 명령
- `publish.ps1` 사용법
- 직접 `dotnet publish` 대체 명령
- 최종 릴리스 체크리스트
- Changelog
- 마이그레이션 지침

## 4. 비차단 주의사항

현재 `Tools\CFServerLauncher\.gitignore`에는 `Publish/` 항목이 없다.

`publish.ps1`을 실행하면 `Tools\CFServerLauncher\Publish` 폴더가 생성된다. 이 폴더는 일반적으로 Git에 포함하지 않는 것이 좋다.

다만 이번 Codex 작업지시서의 수정 대상에는 `.gitignore`가 포함되지 않았으므로, 이번 리뷰의 차단 항목으로 보지는 않는다.

후속 정리 후보:

```text
Tools/CFServerLauncher/.gitignore에 Publish/ 추가
```

## 5. 수동 검증 항목

PowerShell:

```powershell
cd D:\Work\CarFight_git
dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln
dotnet run --project .\Tools\CFServerLauncher\src\CFServerLauncher\CFServerLauncher.csproj
```

앱 수동 검증:

```text
1. 앱이 실행되는지 확인
2. 하단 버전 표시가 버전: v1.0.0인지 확인
3. 서버 시작
4. TestMap 로그 표시 확인
5. 서버 실행 중 창 닫기
6. 아니오 선택 시 런처와 서버 유지 확인
7. 다시 창 닫기
8. 예 선택 시 서버 종료 후 런처 종료 확인
9. 앱 재실행 후 서버 시작/종료/재시작 확인
```

publish 검증:

```powershell
cd D:\Work\CarFight_git\Tools\CFServerLauncher
.\publish.ps1
```

확인 항목:

```text
1. Publish 폴더 생성
2. Publish\CFServerLauncher.exe 생성
3. Server 폴더 유지
4. Config 폴더 유지
5. Logs 폴더 유지
```

## 6. 판정

```text
코드 리뷰: 통과
수동 검증: 대기
비차단 정리 후보: Publish/ gitignore 추가
```
