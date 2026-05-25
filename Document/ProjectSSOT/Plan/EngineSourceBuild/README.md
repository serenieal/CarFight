# CarFight UE 5.7 Source Build 전환 계획

문서 버전: v0.1  
작성 기준일: 2026-05-22  
대상 프로젝트: CarFight  
문서 위치: `D:\Work\CarFight_git\Document\ProjectSSOT\Plan\EngineSourceBuild`

---

## 1. 문서 목적

이 폴더는 CarFight 프로젝트를 UE 5.7 런처 기반 Installed Build에서 UE 5.7 Source Build 엔진으로 전환하기 위한 계획 문서 모음이다.

직접 목적은 Dedicated Server Target 빌드를 막고 있는 Installed Build 제한을 우회하고, Source Build 엔진에서 Editor/Server 빌드와 Dedicated Server 실행 검증까지 통과하는지 확인하는 것이다.

---

## 2. 현재 프로젝트 정보

| 항목 | 값 |
|---|---|
| 프로젝트 이름 | CarFight |
| GitHub | https://github.com/serenieal/CarFight.git |
| 로컬 프로젝트 경로 | `D:\Work\CarFight_git` |
| 언리얼 프로젝트 파일 | `D:\Work\CarFight_git\UE\CarFight_Re.uproject` |
| 현재 엔진 | `D:\UE_5.7` |
| 현재 엔진 유형 | Launcher 기반 Installed Build |
| 전환 목표 엔진 | UE 5.7 Source Build |
| 추천 Source Build 경로 | `D:\UE_5.7_Source` |

---

## 3. 작업 범위

### 포함

- UE 5.7 Source Build 준비
- Epic Games 계정과 GitHub 계정 연결 확인
- UnrealEngine 소스 접근 확인
- Source Build 엔진 설치 경로 결정
- `Setup.bat` 실행
- `GenerateProjectFiles.bat` 실행
- `UE5.sln`에서 UE5 Development Editor Win64 빌드
- CarFight `.uproject` EngineAssociation 전환
- `CarFight_ReEditor` 빌드 검증
- `CarFight_ReServer` 빌드 검증
- Dedicated Server 실행 검증
- 실패 시 롤백 방법 정리

### 제외

- C++ 게임 코드 작성
- Target.cs 신규 작성
- GameMode 작성
- PlayerController 작성
- 무기, 체력, 대미지, 스코어 구현
- 리스폰, 로비, 매치메이킹 구현
- 기존 멀티플레이 안정화 계획 변경
- MP_ServerPlan 내용과 혼합

---

## 4. 문서 구성

| 문서 | 역할 |
|---|---|
| `README.md` | 전체 목적, 범위, 문서 인덱스 |
| `SourceBuildPlan.md` | Source Build 전환 전체 계획 |
| `EnvCheck.md` | 사전 환경 점검 체크리스트 |
| `SwitchSteps.md` | 실제 엔진 전환 절차 |
| `BuildVerify.md` | Editor/Server/Dedicated Server 검증 절차 |

---

## 5. 진행 원칙

1. 기존 `D:\UE_5.7` 런처 엔진은 즉시 삭제하지 않는다.
2. Source Build 전환 검증이 완료될 때까지 기존 엔진은 롤백용으로 유지한다.
3. `.uproject` 전환 전 Git 상태를 반드시 깨끗하게 만든다.
4. `.uproject`의 기존 EngineAssociation 값을 기록한다.
5. Source Build 전환 중 Target.cs 파일이 없다는 문제가 발견되어도 이 문서 작업 범위에서는 새 Target.cs를 만들지 않는다.
6. `CarFight_ReServer` Target 검증은 이미 Target이 존재한다는 전제에서만 수행한다.
7. Target이 없거나 게임 코드 수정이 필요한 경우, Source Build 전환 작업은 환경 전환 결과까지만 기록하고 후속 작업으로 넘긴다.

---

## 6. 완료 판정

이 전환 작업은 다음 조건을 모두 만족하면 완료로 본다.

- UE 5.7 Source Build 엔진이 `Development Editor / Win64`로 빌드된다.
- Source Build 엔진으로 CarFight 프로젝트가 열린다.
- `CarFight_ReEditor Win64 Development` 빌드가 성공한다.
- `CarFight_ReServer Win64 Development` 빌드가 성공한다.
- WindowsServer 콘텐츠 Cook이 성공한다.
- `CarFight_ReServer.exe -log` 실행 시 Dedicated Server 프로세스가 Fatal Error 없이 기동된다.

---

## 7. MP_ServerPlan 복귀 조건

- Source Build 엔진 전환 완료
- Editor 빌드 완료
- Server Target 빌드 완료
- Dedicated Server 실행 검증 완료
- 롤백 불필요 상태 확인
- Source Build 전환 관련 실패 로그가 별도 이슈로 정리됨

---

## 8. 공식 참고 자료

- Epic Developer Community: GitHub에서 언리얼 엔진 소스 코드 다운로드하기  
  https://dev.epicgames.com/documentation/unreal-engine/downloading-source-code-in-unreal-engine?lang=ko
- Epic Developer Community: Building Unreal Engine from Source  
  https://dev.epicgames.com/documentation/unreal-engine/building-unreal-engine-from-source
- Epic Developer Community: Setting Up Dedicated Servers in Unreal Engine  
  https://dev.epicgames.com/documentation/unreal-engine/setting-up-dedicated-servers-in-unreal-engine
- Epic Developer Community: How to Generate Unreal Engine Project Files for Your IDE  
  https://dev.epicgames.com/documentation/unreal-engine/how-to-generate-unreal-engine-project-files-for-your-ide
- Epic Developer Community: Managing Game Code in Unreal Engine  
  https://dev.epicgames.com/documentation/unreal-engine/managing-game-code-in-unreal-engine
- Epic Developer Community: Unreal Engine Build Tool Target Reference  
  https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-build-tool-target-reference
