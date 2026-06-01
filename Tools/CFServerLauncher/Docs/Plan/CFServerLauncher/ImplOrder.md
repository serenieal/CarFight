# CFServerLauncher v1.0 구현 순서

문서 버전: v1.0.0  
작성 기준일: 2026-05-26

## 1. 구현 원칙

v1.0은 기능을 과하게 늘리지 않고, 안정적인 실행/종료/로그 확인 흐름을 먼저 완성한다.

구현 중 지켜야 할 원칙은 다음과 같다.

1. 서버 코드와 런처 코드를 분리한다.
2. Unreal C++ 빌드나 Cook/Stage 기능을 넣지 않는다.
3. 서버 내부 관리자 명령이나 RCON을 넣지 않는다.
4. 설정 JSON 기반으로 동작하게 만든다.
5. 각 단계 완료 후 바로 실행 검증한다.

## 2. 1단계: 프로젝트 뼈대 생성

목표:

```text
Tools/CFServerLauncher 아래 WPF 프로젝트를 생성한다.
```

작업:

```text
1. CFServerLauncher.sln 생성
2. src/CFServerLauncher 프로젝트 생성
3. Models, ViewModels, Services, Utils 폴더 생성
4. Config, Logs, Docs 폴더 생성
5. .gitignore 초안 작성
```

완료 기준:

```text
빈 WPF 창이 실행된다.
```

## 3. 2단계: 설정 모델 작성

대상 파일:

```text
AppConfig.cs
AppConst.cs
```

작업:

```text
1. AppConfig 속성 정의
2. 기본 서버 EXE 경로 정의
3. 기본 작업 폴더 정의
4. 기본 맵 경로 정의
5. 기본 포트 정의
6. 기본 로그 파일 경로 정의
```

완료 기준:

```text
앱 실행 시 기본 설정 객체를 만들 수 있다.
```

## 4. 3단계: 설정 저장/불러오기 구현

대상 파일:

```text
ConfigStore.cs
server.sample.json
server.local.json
```

작업:

```text
1. JSON 직렬화 구현
2. JSON 역직렬화 구현
3. 파일이 없을 때 기본값 반환
4. JSON 파싱 실패 처리
5. 임시 파일 기반 저장 처리
6. 설정 저장 버튼 연결
7. 설정 불러오기 버튼 연결
```

완료 기준:

```text
UI에서 수정한 설정을 저장하고 앱 재실행 후 다시 불러올 수 있다.
```

## 5. 4단계: UI 기본 구성

대상 파일:

```text
MainWindow.xaml
MainWindow.xaml.cs
MainViewModel.cs
RelayCmd.cs
```

작업:

```text
1. 서버 실행 파일 입력 영역 구성
2. 작업 폴더 입력 영역 구성
3. 맵 경로 입력 영역 구성
4. 포트 입력 영역 구성
5. 로그 파일 입력 영역 구성
6. 추가 인자 입력 영역 구성
7. 실행 제어 버튼 배치
8. 상태/PID 표시 영역 구성
9. 실행 인자 미리보기 영역 구성
10. 최근 로그 표시 영역 구성
```

완료 기준:

```text
설정값이 화면에 표시되고 사용자가 수정할 수 있다.
```

## 6. 5단계: 입력값 검증 구현

대상 파일:

```text
PathGuard.cs
```

작업:

```text
1. 서버 EXE 파일 존재 확인
2. .exe 확장자 확인
3. 작업 폴더 존재 확인
4. 맵 경로 비어 있음 확인
5. 포트 숫자 및 범위 확인
6. 로그 파일 부모 폴더 생성 가능 여부 확인
7. 한글 오류 메시지 작성
```

완료 기준:

```text
잘못된 설정으로 서버 시작을 누르면 실행되지 않고 오류가 표시된다.
```

## 7. 6단계: 실행 인자 조립 구현

대상 파일:

```text
ArgBuilder.cs
```

작업:

```text
1. 맵 경로 인자 추가
2. 포트 인자 추가
3. -log 추가
4. -unattended 추가
5. -NoSound 추가
6. -LiveCoding=false 추가
7. -AbsLog="..." 추가
8. 추가 인자 연결
9. 전체 실행 명령 미리보기 생성
```

완료 기준:

```text
UI 입력값이 바뀌면 실행 인자 미리보기가 갱신된다.
```

## 8. 7단계: 서버 프로세스 시작 구현

대상 파일:

```text
ServerProc.cs
MainViewModel.cs
```

작업:

```text
1. ProcessStartInfo 생성
2. FileName에 서버 EXE 경로 설정
3. WorkingDirectory 설정
4. Arguments 설정
5. UseShellExecute 정책 결정
6. 서버 프로세스 시작
7. PID 저장
8. 시작 시각 저장
9. Exited 이벤트 연결
10. 중복 실행 차단
```

완료 기준:

```text
서버 시작 버튼으로 CarFight_ReServer.exe가 실행되고 PID가 UI에 표시된다.
```

## 9. 8단계: 로그 tail 구현

대상 파일:

```text
LogTailer.cs
LineBuffer.cs
MainViewModel.cs
```

작업:

```text
1. 로그 파일 경로를 받아 tail 시작
2. polling 타이머 구성
3. 마지막 읽은 위치 저장
4. 새 로그 라인만 읽기
5. 파일 크기 감소 시 위치 초기화
6. LineBuffer에 로그 라인 추가
7. 최대 줄 수 초과 시 오래된 로그 제거
8. UI 로그 영역 갱신
9. 자동 스크롤 옵션 연결
```

완료 기준:

```text
서버 시작 후 UI에 최근 서버 로그가 계속 표시된다.
```

## 10. 9단계: 서버 종료 구현

대상 파일:

```text
ServerProc.cs
MainViewModel.cs
```

작업:

```text
1. 실행 중인 프로세스 확인
2. 상태를 종료 중으로 변경
3. 정상 종료 시도
4. 실패 시 프로세스 종료
5. Exited 이벤트에서 상태 갱신
6. 종료 코드 표시
7. PID 표시 초기화
8. LogTailer 정지
```

완료 기준:

```text
서버 종료 버튼으로 런처가 실행한 서버 프로세스를 종료할 수 있다.
```

## 11. 10단계: 안정화

작업:

```text
1. 예외 메시지 한글화
2. 반복 시작/종료 테스트
3. 잘못된 경로 테스트
4. 잘못된 포트 테스트
5. 로그 파일 삭제/재생성 테스트
6. 설정 파일 손상 테스트
7. UI 버튼 활성화 조건 점검
8. 문서와 실제 구조 일치 확인
```

완료 기준:

```text
기본 경로 기준으로 서버 시작, 로그 확인, 종료, 설정 저장/불러오기가 안정적으로 동작한다.
```

## 12. 권장 커밋 단위

| 순서 | 커밋 내용 |
|---|---|
| 1 | WPF 프로젝트 뼈대 추가 |
| 2 | 설정 모델과 JSON 저장소 추가 |
| 3 | 메인 UI와 ViewModel 기본 연결 |
| 4 | 입력 검증과 실행 인자 빌더 추가 |
| 5 | 서버 프로세스 시작/종료 추가 |
| 6 | AbsLog tail 표시 추가 |
| 7 | v1.0 검증 및 문서 정리 |
