# CFServerLauncher v1.1 코드 리뷰 결과

문서 버전: v1.1.0-review  
작성 기준일: 2026-05-29

## 1. 리뷰 결론

v1.1 실행 프로필 개발은 코드 리뷰 기준으로 통과로 본다.

확인된 핵심 항목:

- `AppConfig`가 v1.1 구조로 변경됨
- `ServerProfile` 추가됨
- `ServerRunConfig` 추가됨
- `ProfileService` 추가됨
- `ConfigStore`에 v1.0 설정 마이그레이션 추가됨
- `ArgBuilder`가 `ServerRunConfig` 기준으로 변경됨
- `PathGuard`가 `ServerRunConfig` 기준으로 변경됨
- `ServerProc`가 `ServerRunConfig` 기준으로 실행하도록 변경됨
- `MainViewModel`에 프로필 목록/선택/추가/저장/삭제 로직 추가됨
- `MainWindow.xaml`에 프로필 UI 추가됨
- `server.sample.json`이 v1.1 구조로 변경됨

## 2. 확인한 파일

```text
Tools\CFServerLauncher\src\CFServerLauncher\Models\AppConfig.cs
Tools\CFServerLauncher\src\CFServerLauncher\Models\ServerProfile.cs
Tools\CFServerLauncher\src\CFServerLauncher\Models\ServerRunConfig.cs
Tools\CFServerLauncher\src\CFServerLauncher\Services\ProfileService.cs
Tools\CFServerLauncher\src\CFServerLauncher\Services\ConfigStore.cs
Tools\CFServerLauncher\src\CFServerLauncher\Services\ArgBuilder.cs
Tools\CFServerLauncher\src\CFServerLauncher\Services\PathGuard.cs
Tools\CFServerLauncher\src\CFServerLauncher\Services\ServerProc.cs
Tools\CFServerLauncher\src\CFServerLauncher\ViewModels\MainViewModel.cs
Tools\CFServerLauncher\src\CFServerLauncher\MainWindow.xaml
Tools\CFServerLauncher\Config\server.sample.json
```

## 3. 설계 반영 확인

### 3.1 설정 구조 분리

v1.1 설계 원칙대로 설정 구조가 분리되었다.

```text
AppConfig = 런처 전체 설정
ServerProfile = 저장되는 서버 실행 프로필
ServerRunConfig = 실제 실행 1회에 쓰는 런타임 설정
```

`AppConfig`는 다음 값을 가진다.

```text
Version
SelectedProfileName
MaxLogLines
Profiles
```

### 3.2 ServerProfile

`ServerProfile`은 저장되는 프로필 기준 값을 가진다.

```text
Name
ServerExePath
WorkingDir
MapPath
Port
LogFilePath
ExtraArgs
AutoScrollLog
```

### 3.3 ServerRunConfig

`ServerRunConfig`는 실행 1회 기준 값을 가진다.

```text
ProfileName
ServerExePath
WorkingDir
MapPath
Port
BaseLogFilePath
ActiveLogFilePath
ExtraArgs
```

`BaseLogFilePath`와 `ActiveLogFilePath`가 분리되어 있어, v1.0에서 잡은 로그 경로 정책이 유지된다.

## 4. 마이그레이션 확인

`ConfigStore`는 v1.0 설정 구조를 감지하고 v1.1 구조로 변환한다.

확인된 기능:

- v1.0 구조 감지
- v1.0 DTO `LegacyAppConfig` 사용
- v1.0 값을 `로컬 테스트 서버` 프로필로 변환
- `server.local.v1.0.bak.json` 백업 생성
- 변환 후 v1.1 JSON 저장

## 5. 프로필 UI 확인

`MainWindow.xaml`에 프로필 영역이 추가되었다.

확인된 UI:

```text
프로필 콤보박스
새 프로필
프로필 저장
프로필 삭제
```

`ComboBox`는 `Profiles`와 `SelectedProfile`에 바인딩되어 있고, 서버 실행 중에는 `CanEditProfiles` 기준으로 비활성화된다.

## 6. 프로필 동작 확인

`MainViewModel`에서 다음 명령이 추가되었다.

```text
NewProfileCommand
SaveProfileCommand
DeleteProfileCommand
```

확인된 동작:

- 새 프로필 생성 시 이름 입력 대화상자 표시
- 빈 이름 차단
- 중복 이름 차단
- 프로필 1개뿐일 때 삭제 차단
- 삭제 시 확인 대화상자 표시
- 선택 프로필 이름 저장
- 앱 재실행 후 마지막 선택 프로필 유지 가능 구조

## 7. 회귀 유지 확인

v1.0 핵심 기능은 구조상 유지되어 있다.

확인된 항목:

- 서버 시작
- 서버 종료
- 서버 재시작
- 로그 tail
- ActiveLogFilePath 표시
- timestamp 로그 파일 생성
- 서버 실행 중 프로필 변경 차단
- 서버 실행 중 앱 닫기 정리 흐름 유지

## 8. 비차단 주의사항

아래 항목은 실제 수동 검증으로 확인이 필요하다.

```text
1. dotnet build 성공 여부
2. v1.0 server.local.json 실제 마이그레이션
3. server.local.v1.0.bak.json 생성 여부
4. 프로필 추가/저장/삭제 후 앱 재실행 유지 여부
5. 서버 실행 중 프로필 콤보박스 비활성화 여부
6. 선택 프로필로 서버 시작/종료/재시작 실제 동작
```

코드 구조상 큰 차단 문제는 보이지 않는다.

## 9. 수동 검증 명령

PowerShell:

```powershell
cd D:\Work\CarFight_git
dotnet build .\Tools\CFServerLauncher\CFServerLauncher.sln
dotnet run --project .\Tools\CFServerLauncher\src\CFServerLauncher\CFServerLauncher.csproj
```

수동 확인:

```text
1. 앱 실행
2. 버전: v1.1.0 표시 확인
3. 기본 프로필 표시 확인
4. 새 프로필 생성
5. 프로필 저장
6. 앱 재실행 후 프로필 유지 확인
7. 프로필 삭제
8. 서버 시작/종료/재시작 확인
9. ActiveLogFilePath 확인
10. 서버 실행 중 프로필 변경 비활성화 확인
```

## 10. 판정

```text
코드 리뷰: 통과
수동 검증: 대기
```
