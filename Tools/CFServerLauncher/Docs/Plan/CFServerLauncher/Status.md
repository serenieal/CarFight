# CFServerLauncher 현재 상태

문서 버전: v1.0.10-status  
작성 기준일: 2026-05-29

## 1. 결론

`CFServerLauncher` v1.0.0은 최종 릴리스 기준 작업이 완료되었다.

현재 상태:

```text
v1.0.0 최종 릴리스 검증 통과
v1.0.0 완료
```

## 2. 완료된 주요 기능

- WPF 앱 실행
- 서버 EXE 경로 입력/저장
- 작업 폴더 입력/저장
- 맵 경로 입력/저장
- 포트 입력/저장
- 로그 기준 경로 입력/저장
- 서버 실행 파일 찾아보기
- 작업 폴더 찾아보기
- 로그 파일 찾아보기
- 실행 인자 미리보기
- 서버 시작
- 서버 종료
- 서버 종료 후 재시작
- PID 표시
- 상태 표시
- 종료 코드 표시
- 최근 서버 로그 tail 표시
- 현재 실행 로그 파일 경로 표시
- timestamp runtime 로그 파일 생성
- 기준 `LogFilePath`와 실제 `ActiveLogFilePath` 분리
- 수동 종료와 외부 종료 상태 구분
- 서버 실행 중 런처 닫기 확인 대화상자
- 앱 버전 `v1.0.0` 표시
- `publish.ps1` 배포 스크립트
- `Publish/` Git 제외

## 3. 완료된 검증

- `dotnet build` 성공
- 앱 실행 성공
- 서버 시작 성공
- TestMap 로그 표시 성공
- 서버 종료 성공
- 서버 재시작 성공
- 찾아보기 버튼 동작 확인
- runtime 로그 파일명 `server_yyyyMMdd_HHmmss_fff.log` 형식 확인
- 설정 저장 시 timestamp 로그 파일명 미저장 확인
- 서버 종료 버튼 종료 시 상태 `중지됨` 확인
- 외부 종료 시 상태 `종료됨` 확인
- 서버 실행 중 창 닫기 확인 대화상자 확인
- `publish.ps1` 실행 성공
- `publish.ps1` 한글 출력 정상화
- `Publish\CFServerLauncher.exe` 생성 확인

## 4. 현재 서버 빌드 위치

서버 EXE:

```text
D:\Work\CarFight_git\Tools\CFServerLauncher\Server\WindowsServer\CarFight_ReServer.exe
```

작업 폴더:

```text
D:\Work\CarFight_git\Tools\CFServerLauncher\Server\WindowsServer
```

## 5. v1.0.0 최종 판정

```text
CFServerLauncher v1.0.0 완료
```

## 6. 다음 단계 후보

v1.1 후보:

- 실행 프로필
- 로그 파일 정리 기능
- 서버 빌드 교체 절차 정리
- 간단 CPU/메모리 표시
- 로그 검색/필터

v1.1의 첫 작업으로는 `실행 프로필`을 우선 검토한다.
