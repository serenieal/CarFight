# CFServerLauncher 문서 인덱스

문서 버전: v1.0.1  
작성 기준일: 2026-05-29  
대상 프로젝트: CarFight  
대상 경로: `D:\Work\CarFight_git\Tools\CFServerLauncher`

## 1. 목적

`CFServerLauncher`는 CarFight Dedicated Server를 외부 프로세스로 실행, 종료, 감시하기 위한 C# WPF 런처이다.

이 런처는 Unreal Engine 서버 코드와 직접 결합하지 않는다. 서버 내부 GameMode, PlayerController, uasset, 맵, RCON, 관리자 명령에는 접근하지 않는다.

v1.0의 역할은 다음 네 가지로 제한한다.

1. 서버 실행 파일 경로와 작업 폴더를 설정한다.
2. 맵, 포트, 로그 파일 경로를 설정한다.
3. `CarFight_ReServer.exe`를 외부 프로세스로 시작/종료한다.
4. `-AbsLog=...` 로그 파일을 tail 방식으로 읽어 UI에 표시한다.

## 2. 현재 확정된 서버 정보

| 항목 | 값 |
|---|---|
| 프로젝트 루트 | `D:\Work\CarFight_git` |
| UE 프로젝트 | `D:\Work\CarFight_git\UE\CarFight_Re.uproject` |
| 소스 빌드 엔진 | `D:\UnrealEngine_Source` |
| Dedicated Server Target | `CarFight_ReServer` |
| 서버 EXE | `D:\Work\CarFight_git\UE\Saved\StagedBuilds\WindowsServer\CarFight_Re\Binaries\Win64\CarFight_ReServer.exe` |
| 작업 폴더 | `D:\Work\CarFight_git\UE\Saved\StagedBuilds\WindowsServer\CarFight_Re` |
| 기본 맵 | `/Game/Maps/TestMap` |
| 기본 포트 | `7777` |

## 3. 문서 목록

| 문서 | 역할 |
|---|---|
| `Plan.md` | v1.0 개발 계획과 기능 범위 |
| `Roadmap.md` | v1.0 완성까지의 마일스톤 로드맵 |
| `Design.md` | WPF 구조, 클래스, 책임 분리 |
| `UISpec.md` | 화면 구성과 UI 문구 기준 |
| `ConfigSpec.md` | 설정 JSON 스키마와 저장 정책 |
| `RunLogSpec.md` | 실행 인자, 로그 tail, 종료 방식 |
| `ImplOrder.md` | 구현 순서와 작업 단위 |
| `CheckList.md` | 검증 체크리스트 |
| `Future.md` | v1.1 이후 확장 후보 |

## 4. v1.0 포함 기능

- 서버 EXE 경로 입력/저장
- 작업 폴더 입력/저장
- 맵 경로 입력/저장
- 포트 입력/저장
- 로그 파일 경로 입력/저장
- 서버 시작 버튼
- 서버 종료 버튼
- 서버 실행 상태 표시
- PID 표시
- 최근 로그 tail 표시
- 설정 JSON 저장/불러오기
- 실행 인자 미리보기 표시

## 5. v1.0 제외 기능

- Unreal C++ 빌드 실행
- Cook/Stage 자동 실행
- uasset 수정
- 맵 수정
- GameMode/PlayerController 수정
- RCON
- 웹 대시보드
- 게임 내부 관리자 명령
- 복잡한 서버 모니터링

## 6. 핵심 설계 원칙

1. 서버 코드와 런처 코드를 강하게 분리한다.
2. 런처는 외부 프로세스 제어와 파일 로그 읽기만 담당한다.
3. 설정은 JSON 파일 중심으로 유지한다.
4. 서버 수정이 잦아도 런처 수정이 최소화되도록 한다.
5. v1.0은 안정성을 우선하고 기능을 과하게 늘리지 않는다.

## 7. v1.0 로드맵 기준

`Roadmap.md`는 v1.0 완성까지의 실제 개발 순서를 정의한다.

v1.0 완료 전에는 `Future.md`에 있는 기능을 구현 대상으로 보지 않는다.

v1.0 완료 조건은 다음이다.

```text
1. 설정 JSON 저장/불러오기 가능
2. 서버 EXE/작업 폴더/맵/포트/로그 경로 설정 가능
3. 실행 인자 미리보기 표시
4. 필수 실행 인자 강제 포함
5. 서버 시작 가능
6. PID 표시 가능
7. 실행 상태 표시 가능
8. AbsLog 로그 파일 생성 가능
9. 최근 로그 tail 표시 가능
10. 서버 종료 가능
11. 잘못된 입력값 실행 차단
12. 반복 시작/종료 테스트 통과
```

## 8. 파일명 기준

이 문서 세트의 폴더명과 파일명은 모두 32자를 넘지 않는다.

`CFServerLauncher` 실제 구현 파일명도 동일 기준을 따른다.
