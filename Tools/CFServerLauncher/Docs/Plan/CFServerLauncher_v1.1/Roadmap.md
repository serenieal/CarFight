# CFServerLauncher v1.1 로드맵

문서 버전: v1.1.0-roadmap  
작성 기준일: 2026-05-29

## 1. 목적

이 문서는 `CFServerLauncher` v1.1 실행 프로필 기능의 구현 순서와 완료 기준을 정의한다.

v1.1은 기능 확장보다 설정 구조 개선을 우선한다.

## 2. 마일스톤 요약

| 단계 | 이름 | 목표 | 완료 기준 |
|---|---|---|---|
| M0 | 문서 확정 | v1.1 설계 문서 정리 | 문서 세트 존재 |
| M1 | 모델 추가 | ServerProfile/ServerRunConfig 추가 | 빌드 성공 |
| M2 | 설정 구조 개편 | AppConfig를 v1.1 구조로 변경 | v1.1 JSON 저장 가능 |
| M3 | 마이그레이션 | v1.0 설정을 v1.1로 변환 | 기존 설정 보존 |
| M4 | 프로필 서비스 | 프로필 추가/삭제/찾기 로직 분리 | 단위 동작 확인 |
| M5 | UI 추가 | 프로필 선택/추가/저장/삭제 UI | UI 동작 확인 |
| M6 | 실행 흐름 연결 | 선택 프로필로 서버 실행 | 서버 시작/종료 성공 |
| M7 | 회귀 검증 | v1.0 기능 유지 확인 | 체크리스트 통과 |

## 3. M0: 문서 확정

작업:

```text
1. README.md 확인
2. ProfilePlan.md 확인
3. ProfileDesign.md 확인
4. ConfigMigration.md 확인
5. ProfileUISpec.md 확인
6. CheckList.md 확인
```

완료 기준:

```text
CFServerLauncher_v1.1 문서 세트가 존재한다.
```

## 4. M1: 모델 추가

신규 파일 후보:

```text
Models/ServerProfile.cs
Models/ServerRunConfig.cs
```

작업:

```text
1. ServerProfile 모델 추가
2. ServerRunConfig 모델 추가
3. AppConfig v1.1 필드 초안 추가
4. 기존 ProcInfo/RunState 유지
```

완료 기준:

```text
dotnet build 성공
```

## 5. M2: 설정 구조 개편

수정 파일 후보:

```text
Models/AppConfig.cs
Services/ConfigStore.cs
```

작업:

```text
1. AppConfig에서 서버 실행 필드를 Profiles로 이동
2. SelectedProfileName 추가
3. MaxLogLines 유지
4. 기본 v1.1 설정 생성
5. v1.1 JSON 저장 구현
```

완료 기준:

```text
server.local.json이 v1.1 구조로 저장된다.
```

## 6. M3: v1.0 설정 마이그레이션

수정 파일 후보:

```text
Services/ConfigStore.cs
Services/ProfileService.cs
```

작업:

```text
1. v1.0 구조 감지
2. v1.0 설정 백업
3. v1.0 필드를 기본 프로필로 변환
4. v1.1 구조로 저장
5. 마이그레이션 실패 시 기본 프로필 생성
```

완료 기준:

```text
기존 v1.0 server.local.json이 로컬 테스트 서버 프로필로 변환된다.
```

## 7. M4: ProfileService 추가

신규 파일 후보:

```text
Services/ProfileService.cs
```

작업:

```text
1. 기본 프로필 생성
2. 프로필 이름 중복 검사
3. 프로필 찾기
4. 프로필 추가
5. 프로필 삭제
6. selectedProfileName 보정
```

완료 기준:

```text
ViewModel이 직접 복잡한 프로필 조작 로직을 갖지 않는다.
```

## 8. M5: UI 추가

수정 파일 후보:

```text
MainWindow.xaml
MainViewModel.cs
```

작업:

```text
1. 프로필 콤보박스 추가
2. 새 프로필 버튼 추가
3. 프로필 저장 버튼 추가
4. 프로필 삭제 버튼 추가
5. 서버 실행 중 프로필 변경 비활성화
```

완료 기준:

```text
프로필을 선택하면 UI 설정값이 바뀐다.
```

## 9. M6: 실행 흐름 연결

작업:

```text
1. 선택된 ServerProfile에서 ServerRunConfig 생성
2. ServerRunConfig로 실행 인자 생성
3. ActiveLogFilePath는 ServerRunConfig에만 유지
4. 서버 시작/종료 기존 동작 유지
```

완료 기준:

```text
선택한 프로필 값으로 서버가 실행된다.
```

## 10. M7: 회귀 검증

검증:

```text
1. 기존 v1.0 기능 전체 재확인
2. 서버 시작/종료/재시작 확인
3. 로그 tail 확인
4. 앱 닫기 확인 대화상자 확인
5. publish.ps1 유지 확인
```

완료 기준:

```text
v1.1 기능과 v1.0 기능이 모두 통과한다.
```

## 11. 권장 구현 순서

```text
M0 문서 확정
M1 모델 추가
M2 설정 구조 개편
M3 마이그레이션
M4 ProfileService 추가
M5 UI 추가
M6 실행 흐름 연결
M7 회귀 검증
```

## 12. Codex 작업 분리 권장

한 번에 모든 코드를 바꾸지 말고 다음 2개 작업으로 나눈다.

```text
Codex Task 1:
모델/AppConfig/ConfigStore/ProfileService/마이그레이션

Codex Task 2:
MainViewModel/MainWindow UI 연결/수동 검증 수정
```

이렇게 나누면 설정 마이그레이션 문제와 UI 문제를 분리해서 검토할 수 있다.
