# CFServerLauncher v1.0 로드맵

문서 버전: v1.0.1  
작성 기준일: 2026-05-29  
대상 위치: `D:\Work\CarFight_git\Tools\CFServerLauncher`

## 1. 문서 목적

이 문서는 `CFServerLauncher` v1.0을 완성하기 위한 실제 개발 로드맵이다.

v1.0은 아직 완성된 상태가 아니므로, 이 문서는 v1.1 이후 기능 확장이 아니라 **v1.0 완성까지의 단계, 완료 기준, 의존 관계, 검증 시점**을 정의한다.

v1.1 이후 확장 후보는 `Future.md`에 분리한다.

## 2. v1.0 완료 목표

v1.0의 최종 목표는 다음이다.

```text
외부 WPF 런처에서 CarFight Dedicated Server를 설정 기반으로 시작/종료하고,
-AbsLog 로그 파일을 tail 방식으로 표시할 수 있는 안정적인 최소 관리 도구를 완성한다.
```

v1.0은 다음을 하지 않는다.

- Unreal C++ 빌드 실행
- Cook/Stage 자동 실행
- uasset 수정
- 맵 수정
- GameMode/PlayerController 수정
- RCON
- 웹 대시보드
- 게임 내부 관리자 명령
- 복잡한 서버 모니터링

## 3. v1.0 마일스톤 요약

| 단계 | 이름 | 목표 | 완료 판정 |
|---|---|---|---|
| M0 | 문서 확정 | 계획/설계/검증 문서 정리 | 문서 폴더와 문서 세트 존재 |
| M1 | 프로젝트 생성 | WPF 프로젝트 뼈대 생성 | 빈 앱 실행 성공 |
| M2 | 설정 시스템 | JSON 설정 저장/불러오기 | 재실행 후 설정 복원 |
| M3 | UI 1차 | 설정/상태/로그 UI 배치 | 입력값 바인딩 성공 |
| M4 | 입력 검증 | 경로/포트/맵 검증 | 잘못된 입력 시작 차단 |
| M5 | 인자 조립 | 서버 실행 인자 미리보기 | 필수 인자 모두 표시 |
| M6 | 서버 시작 | 외부 프로세스 실행 | PID 표시, 상태 실행 중 |
| M7 | 로그 tail | AbsLog 파일 tail 표시 | UI에 서버 로그 표시 |
| M8 | 서버 종료 | 런처가 시작한 PID 종료 | 종료 후 상태/PID 갱신 |
| M9 | 안정화 | 반복 테스트와 예외 정리 | 체크리스트 통과 |

## 4. M0: 문서 확정

목표:

```text
v1.0 구현 전에 설계 기준을 고정한다.
```

작업:

1. `README.md` 문서 인덱스 정리
2. `Plan.md` v1.0 범위 확인
3. `Design.md` 클래스 책임 확인
4. `UISpec.md` UI 문구 확인
5. `ConfigSpec.md` JSON 스키마 확인
6. `RunLogSpec.md` 실행/로그 정책 확인
7. `ImplOrder.md` 구현 순서 확인
8. `CheckList.md` 검증 기준 확인
9. `Roadmap.md` v1.0 마일스톤 확인

완료 기준:

```text
Document/ProjectSSOT/Plan/CFServerLauncher 문서 세트가 존재한다.
```

## 5. M1: WPF 프로젝트 생성

목표:

```text
Tools/CFServerLauncher 아래에서 독립 실행 가능한 WPF 앱을 만든다.
```

작업:

1. `Tools/CFServerLauncher` 폴더 생성
2. `CFServerLauncher.sln` 생성
3. `src/CFServerLauncher` 프로젝트 생성
4. `Models`, `ViewModels`, `Services`, `Utils` 폴더 생성
5. `Config`, `Logs`, `Docs` 폴더 생성
6. `.gitignore` 작성

완료 기준:

```text
빈 WPF 창이 실행된다.
```

검증:

```text
dotnet build 성공
앱 실행 성공
창 제목 표시 성공
```

## 6. M2: 설정 시스템 구현

목표:

```text
서버 실행에 필요한 값을 JSON으로 저장하고 불러온다.
```

작업 파일:

```text
AppConfig.cs
AppConst.cs
ConfigStore.cs
server.sample.json
server.local.json
```

작업:

1. `AppConfig` 모델 작성
2. 기본 경로 상수 작성
3. `server.sample.json` 작성
4. 설정 불러오기 구현
5. 설정 저장 구현
6. 파일 없음 처리 구현
7. JSON 파싱 실패 처리 구현

완료 기준:

```text
설정을 저장하고 앱 재실행 후 같은 값이 복원된다.
```

## 7. M3: UI 1차 구성

목표:

```text
v1.0 필수 입력값과 상태 표시 영역을 한 화면에 배치한다.
```

작업 파일:

```text
MainWindow.xaml
MainWindow.xaml.cs
MainViewModel.cs
RelayCmd.cs
```

작업:

1. 서버 실행 파일 입력 UI
2. 작업 폴더 입력 UI
3. 맵 경로 입력 UI
4. 포트 입력 UI
5. 로그 파일 입력 UI
6. 추가 인자 입력 UI
7. 서버 시작/종료 버튼
8. 설정 저장/불러오기 버튼
9. 상태/PID 표시 영역
10. 실행 인자 미리보기 영역
11. 최근 로그 영역

완료 기준:

```text
UI에서 설정값을 수정할 수 있고 ViewModel에 반영된다.
```

## 8. M4: 입력 검증 구현

목표:

```text
잘못된 설정으로 서버가 실행되지 않게 막는다.
```

작업 파일:

```text
PathGuard.cs
```

검증 항목:

1. 서버 EXE 파일 존재
2. `.exe` 확장자
3. 작업 폴더 존재
4. 맵 경로 비어 있음 여부
5. 포트 숫자 여부
6. 포트 범위 1~65535
7. 로그 파일 부모 폴더 생성 가능 여부

완료 기준:

```text
잘못된 입력값으로 서버 시작 시 한글 오류가 표시되고 실행이 차단된다.
```

## 9. M5: 실행 인자 조립 구현

목표:

```text
현재 설정값 기준으로 서버 실행 인자를 안전하게 만든다.
```

작업 파일:

```text
ArgBuilder.cs
```

필수 포함 인자:

```text
-log
-unattended
-NoSound
-LiveCoding=false
-AbsLog="..."
```

완료 기준:

```text
실행 인자 미리보기에 맵, 포트, 필수 인자, 로그 경로가 모두 표시된다.
```

## 10. M6: 서버 시작 구현

목표:

```text
런처에서 CarFight_ReServer.exe를 외부 프로세스로 실행한다.
```

작업 파일:

```text
ServerProc.cs
MainViewModel.cs
```

작업:

1. `ProcessStartInfo` 구성
2. 서버 EXE 경로 지정
3. 작업 폴더 지정
4. 인자 문자열 지정
5. 프로세스 시작
6. PID 저장
7. 시작 시각 저장
8. 실행 상태 갱신
9. 중복 실행 차단

완료 기준:

```text
서버 시작 버튼 클릭 후 서버 프로세스가 실행되고 PID가 표시된다.
```

## 11. M7: 로그 tail 구현

목표:

```text
-AbsLog로 고정한 로그 파일을 읽어 UI에 표시한다.
```

작업 파일:

```text
LogTailer.cs
LineBuffer.cs
MainViewModel.cs
```

작업:

1. 서버 시작 전 기존 로그 파일 처리
2. 로그 파일 부모 폴더 생성
3. polling 기반 tail 구현
4. 마지막 읽은 위치 저장
5. 파일 재생성 감지
6. 최대 로그 줄 수 제한
7. UI 로그 표시 갱신

완료 기준:

```text
서버 실행 중 새 로그가 UI에 계속 추가된다.
```

## 12. M8: 서버 종료 구현

목표:

```text
런처가 직접 시작한 서버 프로세스를 종료한다.
```

작업 파일:

```text
ServerProc.cs
MainViewModel.cs
```

작업:

1. 현재 추적 중인 프로세스 확인
2. 상태를 종료 중으로 변경
3. 정상 종료 시도
4. 실패 시 프로세스 종료
5. 종료 코드 표시
6. PID 초기화
7. LogTailer 정지

완료 기준:

```text
서버 종료 버튼 클릭 후 서버 프로세스가 종료되고 상태가 갱신된다.
```

## 13. M9: v1.0 안정화

목표:

```text
반복 실행과 오류 상황에서 런처가 안정적으로 동작하게 만든다.
```

작업:

1. 시작/종료 5회 반복 테스트
2. 잘못된 EXE 경로 테스트
3. 잘못된 작업 폴더 테스트
4. 잘못된 포트 테스트
5. JSON 손상 테스트
6. 로그 파일 삭제/재생성 테스트
7. 서버 외부 종료 테스트
8. UI 버튼 활성화 조건 점검
9. 한글 오류 메시지 정리

완료 기준:

```text
CheckList.md의 v1.0 필수 검증 항목을 통과한다.
```

## 14. v1.0 개발 순서

권장 순서는 다음과 같다.

```text
M0 문서 확정
M1 프로젝트 생성
M2 설정 시스템
M3 UI 1차
M4 입력 검증
M5 실행 인자 조립
M6 서버 시작
M7 로그 tail
M8 서버 종료
M9 안정화
```

서버 실행과 로그 tail은 서로 연결되어 있지만, 먼저 서버 시작과 PID 표시를 검증한 뒤 로그 tail을 붙이는 편이 문제 원인 분리가 쉽다.

## 15. v1.0 완료 정의

다음 조건을 모두 만족하면 v1.0 완료로 본다.

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
