# Source Build 전환 절차

문서 버전: v0.1
작성 기준일: 2026-05-22
대상 프로젝트: CarFight

## 1. 목적

이 문서는 CarFight 프로젝트를 UE 5.7 Source Build 엔진에 연결하는 절차를 정리한다.

이 문서는 코드 작업 문서가 아니다. Target.cs, GameMode, PlayerController, C++ 게임 코드를 새로 작성하지 않는다.

## 2. 전환 전 준비

### Git 상태 확인

```bat
cd /d D:\Work\CarFight_git
git status
```

권장 상태는 변경사항이 명확히 정리된 상태다. 변경사항이 있으면 커밋하거나 별도 백업한다.

### uproject 백업

대상 파일:

```text
D:\Work\CarFight_git\UE\CarFight_Re.uproject
```

권장 백업:

```text
D:\Work\CarFight_git\UE\CarFight_Re.uproject.before_source_build
```

기록할 값:

```text
전환 전 EngineAssociation:
```

## 3. UE 5.7 Source Build 준비

추천 경로:

```text
D:\UE_5.7_Source
```

절차:

1. Epic Games 계정과 GitHub 계정 연결을 확인한다.
2. UnrealEngine 소스 저장소 접근을 확인한다.
3. UE 5.7 소스 브랜치 또는 태그를 선택한다.
4. 소스를 D:\UE_5.7_Source 경로에 배치한다.

기존 D:\UE_5.7 런처 엔진은 덮어쓰지 않는다.

## 4. 의존성 설치

```bat
cd /d D:\UE_5.7_Source
Setup.bat
```

확인 항목:

- 의존성 다운로드 완료
- 권한 오류 없음
- 네트워크 오류 없음

## 5. 프로젝트 파일 생성

```bat
cd /d D:\UE_5.7_Source
GenerateProjectFiles.bat
```

완료 기준:

```text
D:\UE_5.7_Source\UE5.sln
```

## 6. 엔진 빌드

Visual Studio에서 다음 순서로 진행한다.

1. D:\UE_5.7_Source\UE5.sln 열기
2. Development Editor 선택
3. Win64 선택
4. UE5 타깃 빌드

완료 기준:

```text
D:\UE_5.7_Source\Engine\Binaries\Win64\UnrealEditor.exe
```

## 7. Source Build Editor 실행 확인

```bat
"D:\UE_5.7_Source\Engine\Binaries\Win64\UnrealEditor.exe" -log
```

성공 기준:

- Editor가 열린다.
- 시작 직후 Fatal Error가 없다.

## 8. CarFight EngineAssociation 전환

권장 방법:

1. D:\Work\CarFight_git\UE 폴더로 이동한다.
2. CarFight_Re.uproject 우클릭.
3. Switch Unreal Engine version 메뉴 선택.
4. D:\UE_5.7_Source 엔진 선택.
5. 프로젝트 파일 재생성을 진행한다.

직접 수동 편집은 마지막 수단으로만 사용한다.

## 9. CarFight 프로젝트 파일 재생성

우클릭 메뉴가 가능하면 Generate Visual Studio project files를 사용한다.

명령줄 대안:

```bat
cd /d D:\UE_5.7_Source
GenerateProjectFiles.bat "D:\Work\CarFight_git\UE\CarFight_Re.uproject" -Game
```

## 10. Editor Target 빌드

```bat
cd /d D:\UE_5.7_Source
Engine\Build\BatchFiles\Build.bat -Target="CarFight_ReEditor Win64 Development" -Project="D:\Work\CarFight_git\UE\CarFight_Re.uproject"
```

## 11. Server Target 빌드

전제: CarFight_ReServer Target이 이미 존재해야 한다.

```bat
cd /d D:\UE_5.7_Source
Engine\Build\BatchFiles\Build.bat -Target="CarFight_ReServer Win64 Development" -Project="D:\Work\CarFight_git\UE\CarFight_Re.uproject"
```

Target이 없으면 이번 작업에서는 만들지 않고 보류로 기록한다.

## 12. 롤백

```bat
copy /Y "D:\Work\CarFight_git\UE\CarFight_Re.uproject.before_source_build" "D:\Work\CarFight_git\UE\CarFight_Re.uproject"
```

그 뒤 기존 엔진으로 연다.

```bat
"D:\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe" "D:\Work\CarFight_git\UE\CarFight_Re.uproject" -log
```

## 13. 다음 단계

전환이 끝나면 BuildVerify.md 절차로 검증한다.
