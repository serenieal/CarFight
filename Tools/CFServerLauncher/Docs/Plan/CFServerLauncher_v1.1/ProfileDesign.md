# CFServerLauncher v1.1 실행 프로필 설계

문서 버전: v1.1.0-design  
작성 기준일: 2026-05-29

## 1. 설계 목표

v1.1은 설정 구조를 확장 가능한 형태로 바꾸는 버전이다.

핵심은 다음이다.

```text
AppConfig = 런처 전체 설정
ServerProfile = 저장되는 서버 실행 프로필
ServerRunConfig = 실제 실행 1회에 쓰는 런타임 설정
```

이렇게 분리하면 저장 설정과 실행 설정이 섞이지 않는다.

## 2. 추천 파일 구조

```text
Models
├─ AppConfig.cs
├─ ServerProfile.cs
├─ ServerRunConfig.cs
├─ ProcInfo.cs
└─ RunState.cs

Services
├─ ConfigStore.cs
├─ ProfileService.cs
├─ ArgBuilder.cs
├─ PathGuard.cs
├─ ServerProc.cs
└─ LogTailer.cs
```

신규 파일 후보:

```text
ServerProfile.cs
ServerRunConfig.cs
ProfileService.cs
```

모든 파일명은 32자 이하이다.

## 3. AppConfig 역할

`AppConfig`는 런처 전체 설정 파일 구조를 담당한다.

v1.1 기준 속성 후보:

| 속성 | 설명 |
|---|---|
| `Version` | 설정 스키마 버전 |
| `SelectedProfileName` | 마지막으로 선택한 프로필 이름 |
| `MaxLogLines` | UI 로그 최대 줄 수 |
| `Profiles` | 서버 실행 프로필 목록 |

`AppConfig`는 서버 실행 경로를 직접 들지 않는다. 서버 실행 경로는 `ServerProfile`이 가진다.

## 4. ServerProfile 역할

`ServerProfile`은 저장되는 서버 실행 설정이다.

속성 후보:

| 속성 | 설명 |
|---|---|
| `Name` | 프로필 이름 |
| `ServerExePath` | 서버 EXE 경로 |
| `WorkingDir` | 서버 작업 폴더 |
| `MapPath` | Unreal 맵 경로 |
| `Port` | 서버 포트 |
| `LogFilePath` | 기준 로그 파일 경로 |
| `ExtraArgs` | 추가 실행 인자 |
| `AutoScrollLog` | 로그 자동 스크롤 여부 |

`LogFilePath`는 실제 실행 로그 파일이 아니라 기준 로그 파일 경로이다.

## 5. ServerRunConfig 역할

`ServerRunConfig`는 실제 서버 실행 1회에 사용할 값이다.

속성 후보:

| 속성 | 설명 |
|---|---|
| `ProfileName` | 실행에 사용한 프로필 이름 |
| `ServerExePath` | 서버 EXE 경로 |
| `WorkingDir` | 서버 작업 폴더 |
| `MapPath` | Unreal 맵 경로 |
| `Port` | 서버 포트 |
| `BaseLogFilePath` | 기준 로그 파일 경로 |
| `ActiveLogFilePath` | 실제 실행 로그 파일 경로 |
| `ExtraArgs` | 추가 실행 인자 |

`ArgBuilder`와 `ServerProc`는 가능하면 `ServerRunConfig`를 기준으로 동작하게 개편한다.

## 6. ProfileService 역할

`ProfileService`는 프로필 목록 조작을 담당한다.

주요 책임:

```text
1. 기본 프로필 생성
2. v1.0 설정을 프로필로 변환
3. 프로필 이름 중복 검사
4. 프로필 추가
5. 프로필 삭제
6. 프로필 찾기
7. 현재 UI 값에서 프로필 생성
8. 선택된 프로필 상태 보정
```

ViewModel에 모든 프로필 조작 로직을 넣지 않고 `ProfileService`로 분리한다.

## 7. MainViewModel 역할 변화

v1.1의 `MainViewModel`은 다음 상태를 추가로 가진다.

| 속성 | 설명 |
|---|---|
| `Profiles` | UI에 표시할 프로필 목록 |
| `SelectedProfile` | 현재 선택된 프로필 |
| `SelectedProfileName` | 현재 선택된 프로필 이름 |
| `ActiveLogFilePath` | 현재 실행 로그 경로 |

명령 후보:

```text
NewProfileCommand
SaveProfileCommand
DeleteProfileCommand
RenameProfileCommand
```

v1.1에서는 이름 변경을 별도 버튼으로 둘 수 있지만, 구현 부담을 줄이려면 새 프로필/저장/삭제까지만 먼저 구현해도 된다.

## 8. 실행 흐름

```text
ServerProfile
↓ 현재 선택 프로필
MainViewModel UI 값
↓ 서버 시작 시
ServerRunConfig
↓
ArgBuilder
↓
ServerProc
↓
LogTailer
```

중요 원칙:

```text
ServerProfile.LogFilePath는 저장용 기준 경로
ServerRunConfig.ActiveLogFilePath는 실행용 실제 로그 경로
```

## 9. 마이그레이션 원칙

v1.0 구조를 읽으면 다음처럼 변환한다.

```text
v1.0 AppConfig
↓
AppConfig v1.1
└─ Profiles[0] = "로컬 테스트 서버"
```

마이그레이션 후에는 `Version = "1.1"`로 저장한다.

## 10. 설계상 금지 사항

- 프로필 선택만으로 서버를 자동 시작하지 않는다.
- 프로필 삭제 시 실행 중인 서버에 영향을 주지 않는다.
- 실행 중인 서버의 `ServerRunConfig`를 중간에 변경하지 않는다.
- `ActiveLogFilePath`를 설정 파일에 저장하지 않는다.
- Unreal 서버 내부 클래스에 접근하지 않는다.
