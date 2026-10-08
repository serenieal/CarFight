# 타겟 선택 시스템 작업계획

- 문서 버전: v0.13.0
- 상태: Done / Historical + Retained Path / Rebaseline Complete
- 작성일: 2026-07-22
- 최근 갱신일: 2026-09-14
- 기능 ID: `CF-FQ-026`
- 기준 기획: `../Design/TargetSelect.md`
- 로드맵: `TargetSelectRoadmap.md`
- Plan 작업지시: `TargetSelectWorkOrder.md`
- 구조 조사: `TargetSelectInvestigation.md`

---

## 2026-09-14 Final Rebaseline Closure

`CF-FQ-026 타겟 선택 시스템`은 과거 `TS-P0-08 USER PIE`를 그대로 재개하지 않고 현재 Source / Asset / Current Systems 기준으로 Rebaseline한 결과 **Done / Historical + Retained Path**로 종료한다.

원래 이 Feature의 독립 요구는 다음이었다.

```text
Targetable / TargetPoint 계약
→ 후보 탐색·결정적 정렬
→ 공용 선택 상태와 수명
→ Select / Clear 입력
→ 후보·선택 HUD
→ 장비의 read-only TargetUse 평가
→ 최종 통합 USER/성능 튜닝
```

현재 실제 Source에서 `UCFTargetSelectComp`는 후보·선택 상태의 단일 Authority로 계속 사용되고 있으며 Sensor/Scanner가 이를 대체하지 않는다. `SensorContact.md`도 Detection/Contact와 TargetSelect Candidate/Selected Actor를 명시적으로 분리한다. Aim/Fire는 현재 선택 Actor를 소비할 수 있지만 공용 선택 상태를 소유하지 않는다.

Paused 당시 남아 있던 구조적 비용도 Current Source가 전진했다. 과거 20Hz 후보 갱신마다 수행하던 전체 World `TActorIterator<AActor>` 순회는 `UCFTargetRegistrySubsystem`의 World 시작 1회 bootstrap + spawn/level-add 증분 등록 + weak-reference snapshot 조회로 대체됐다. 현재 반복 후보 갱신은 Registry의 Targetable Actor 집합을 소비한다.

UI도 당시 구조 그대로가 아니다. 현재 `UCFUISubsystem`이 TargetSelect World Marker 수명과 Pawn Rebind를 소유하고, `UCFTargetSelectWidget`은 후보/선택 semantic text를 runtime에서 비우고 숨긴 marker 역할을 담당한다. 의미 텍스트는 Production TargetPanel/HUD 경로로 분리됐다. 2026-09-14 fresh AssetDump에서 `IA_SelectTarget`, `IMC_Vehicle_Default`의 Select/Clear mapping과 persisted `WBP_TargetSelect` 존재를 확인했다. 저장 WidgetBlueprint에 Historical TextBlock tree가 남아 있는 사실과 Current C++ runtime 표시 정책은 구분한다.

Rebaseline 판정:

```text
TS-P0-00~07
= Historical Technical PASS 보존

TS-P0-08 remote technical evidence
= 기존 Build / TS-P0-08 3/3 / safe regression evidence 보존

20Hz whole-world 반복 scan
= Successor-resolved by TargetRegistry

동일 차량 TargetPoint / 인식 영역
= 코드·Automation 교정 evidence 보존
= USER Inconclusive / 새 USER PASS 없음

7° / 1200m 후보 범위 체감
= Deferred USER tuning debt

Debug Sphere 실제 가시성
= Deferred developer observational debt

후보 텍스트 차량 겹침
= successor marker + Production TargetPanel architecture가 old 역할을 대체
= Historical USER issue를 사후 PASS로 변경하지 않음

old 16:9 / 32:9 UI·입력 USER workflow
= Superseded / Not Executed
= 필요 시 Current UISubsystem + TargetSelect marker + Production TargetPanel workflow에서 새 USER validation
```

따라서 old `TS-P0-08 USER PIE` 전체를 현재 Feature closure gate로 유지할 실익이 없다. 실제 TargetSelect UX 튜닝이 다시 필요해지면 Historical Feature를 재개하지 않고 Current System 기준으로 정확한 관찰 항목을 새 lifecycle에서 정의한다.

Current owner:

```text
Target candidate / selection / lifetime / input / target-use query
→ Document/Systems/Targeting/TargetSelect.md v1.0.0

Sensor detection / Contact intelligence
→ Document/Systems/Targeting/SensorContact.md v1.2.0

Production HUD / marker presentation lifetime
→ Document/Systems/UI/InGameUI.md + CFUISubsystem Current implementation
```

Lifecycle:

```text
G0 = PASS — TS-P0-00~07 implementation/evidence + current Source/Asset successor coverage 확인
G1 = PASS — 영구 TargetSelect 계약을 TargetSelect.md v1.0.0 Current System으로 승격
G2 = PASS — ActiveWork / ProjectSSOT / Plan Index의 Paused current route 제거
G3 = PASS — TargetSelectPlan.md retained path로 Historical evidence 보존
G4 = PASS — semantic Historical
G5 = Deferred — physical Archive move optional
```

이번 Rebaseline은 Source/Asset mutation 없이 Current 계약 승격 + 문서 lifecycle 정리만 수행했다. Build/Automation/PIE를 다시 실행하지 않았고 새 USER PASS도 추가하지 않았다. fresh AssetDump는 persisted Input/UI 상태를 읽기 위한 evidence 수집만 수행했다.

---

## 1. 목적

이 문서는 타겟 선택 시스템을 실제 구현 가능한 작업 단위로 분해한다.

각 작업은 독립된 Task ID를 가지며, 후속 Codex/Plan 작업지시서는 한 번에 하나의 Task ID만 수행하도록 작성한다. 여러 단계의 코드, UI, 데이터, 검증을 한 작업에 섞지 않는다.

---

## 1-1. 현재 작업 체크포인트

| 항목 | 현재 상태 |
|---|---|
| Feature lifecycle | `Done / Historical + Retained Path / Rebaseline Complete` |
| Historical final stage | `TS-P0-08 old USER workflow Superseded / Not Executed / USER debt 분리` |
| 완료 Task | `TS-P0-00 기존 구조 조사 Done`, `TS-P0-01 계약 및 데이터 설계 Done`, `TS-P0-02 타겟 포인트 기반 Done`, `TS-P0-03 후보 탐색과 정렬 Done`, `TS-P0-04 선택 상태 수명 Done`, `TS-P0-05 입력과 플레이어 연결 Done`, `TS-P0-06 후보 및 선택 HUD Done`, `TS-P0-07 장비 조회 연동 Done` |
| Current Task | `없음 — Historical evidence owner` |
| 조사 산출물 | `TargetSelectInvestigation.md` |
| 코드 | 후보·수명·입력·HUD 계층과 함께 `CFTargetUseTypes`, WeaponData 대상 정책, WeaponComp 선택 구독·평가 캐시와 장비 조회 자동 검증 작성됨 |
| Editor 빌드 | `CarFight_ReEditor Win64 Development PASS / Latest Build Job 4b4554ab8f7e455abf0c0538ae10a664 / Exit Code 0` |
| 통합 프리셋 | `TargetSelect 전체 7개 Automation PASS / Exit Code 0` |
| HUD 에셋 | `WBP_TargetSelect` 생성, 부모 `UCFTargetSelectWidget`, 8개 Widget Tree, UI AssetDump 7/7 PASS |
| 장비 조회 계약 | `FCFTargetUsePolicy / Request / Result`, 11개 실패 사유, WeaponData.TargetUsePolicy, WeaponComp 결과 캐시·변경 이벤트 PASS |
| Pawn 최소 통합 | PASS |
| 자동화 검증 | `TS-P0-08 3/3 PASS` (`SearchDiagnostics`, `SettingsPath`, `SingleTargetBoundary`) + 최종 자산 비변경 회귀 `TS-P0-01·02·03·04·07 각 1/1 PASS`; dirty WBP/Input 보호를 위해 TS-P0-05·06 및 전체 suite는 이번 세션 미실행 |
| 사용자 PIE | `기본 선택·해제 흐름 정상 / 최신 재검증에서 텍스트 겹침, 후보 범위 과대, 디버그 원 비가시 확인` |
| 다음 작업 | `없음 — old TS-P0-08 자동 재개 금지 / 필요 시 Current TargetSelect 기준 별도 UX·profiling lifecycle` |

현재 소스 파일:

```text
UE/Source/CarFight_Re/Public/CFTargetSelectTypes.h
UE/Source/CarFight_Re/Public/CFTargetUseTypes.h
UE/Source/CarFight_Re/Public/CFTargetSelectable.h
UE/Source/CarFight_Re/Public/CFTargetSelectData.h
UE/Source/CarFight_Re/Public/CFTargetSelectComp.h
UE/Source/CarFight_Re/Private/CFTargetSelectable.cpp
UE/Source/CarFight_Re/Private/CFTargetSelectData.cpp
UE/Source/CarFight_Re/Private/CFTargetSelectComp.cpp
UE/Source/CarFight_Re/Private/CFTargetCandidateSearch.cpp
UE/Source/CarFight_Re/Private/CFTargetCandidateTests.cpp
UE/Source/CarFight_Re/Private/CFTargetTuneTests.cpp
UE/Source/CarFight_Re/Private/CFTargetLifetimeTests.cpp
UE/Source/CarFight_Re/Private/CFTargetInputTests.cpp
UE/Source/CarFight_Re/Public/UI/CFTargetSelectWidget.h
UE/Source/CarFight_Re/Private/UI/CFTargetSelectWidget.cpp
UE/Source/CarFight_Re/Private/CFTargetHudTests.cpp
UE/Source/CarFight_Re/Private/CFTargetUseTests.cpp
UE/Source/CarFight_Re/Public/CFWeaponData.h
UE/Source/CarFight_Re/Private/CFWeaponData.cpp
UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp
UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h
UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp
UE/Source/CarFight_Re/Public/CFCollisionChannels.h
UE/Source/CarFight_Re/Public/CFTargetPointComp.h
UE/Source/CarFight_Re/Private/CFTargetPointComp.cpp
UE/Source/CarFight_Re/Private/CFTargetPointTests.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Private/CFTargetSelectContractTestTypes.h
UE/Source/CarFight_Re/Private/CFTargetSelectContractTestTypes.cpp
UE/Source/CarFight_Re/Private/CFTargetSelectContractTests.cpp
UE/Content/CarFight/Input/IA_SelectTarget.uasset
UE/Content/CarFight/Input/IA_ClearTarget.uasset
UE/Content/CarFight/Input/IMC_Vehicle_Default.uasset
UE/Content/CarFight/UI/WBP_TargetSelect.uasset
Tools/RunTargetSelectTests.bat
```

판정 규칙:

```text
- TS-P0-00~07은 완료됐으며 최신 공식 성공 기준은 Build Job `b7c2eb766e984c8ea6f5d3d847cee1f6`의 이동 사거리 갱신을 포함한 전체 7개 Automation PASS다.
- 선택 상태는 `UCFTargetSelectComp`가 계속 단일 소유하며 장비는 `FCFTargetUseRequest`로 읽기 전용 평가만 수행한다.
- 선택 대상 존재·유효성과 장비 준비·호환·거리·최종 사용 가능 상태는 각각 분리해서 보존한다.
- WeaponComp는 첫 소비 예제일 뿐 별도 획득 상태를 소유하거나 공용 선택을 변경하지 않는다.
- WeaponData의 빈 TargetUsePolicy 배열은 기존 에셋 호환을 위해 제한 없음으로 해석한다.
- 직접 조준 무기의 FireOrigin, AimSolution과 발사 방향은 선택 대상 평가로 자동 보정하지 않는다.
- Historical v0.12.5까지 TS-P0-08은 실제 PIE에서 입력·후보·선택·HUD·장비 평가·파괴 수명과 화면비·성능을 통합 검증하는 gate였다. 2026-09-14 Rebaseline 이후 old TS-P0-08은 current next gate가 아니다.
```

---

## 2. 작업 운영 원칙

1. 기획 문서의 확정 결정을 구현자가 임의로 변경하지 않는다.
2. 미결 항목은 조사 Task에서 근거를 확보한 뒤 잠금한다.
3. 기존 클래스와 에셋 경로를 추측하지 않는다.
4. C++ 변경과 Blueprint 변경은 산출물을 구분한다.
5. 각 Task는 대상 파일, 완료 조건, 검증 방법을 반드시 가진다.
6. 새 기능은 디버그 관찰 수단 없이 완료 처리하지 않는다.
7. 작업 중 발견한 범위 밖 문제는 별도 후속 Task로 기록한다.
8. 사용자 변경사항과 작업 중인 파일을 덮어쓰지 않는다.
9. 파일명은 32자를 넘기지 않는다.
10. 구현 후 문서의 상태와 변경 이력을 갱신한다.

---

## 3. 권장 구현 구조

아래 구조는 구현 전 조사 결과에 따라 이름과 소유 클래스가 조정될 수 있다.

### C++ 계층

- 선택 가능 대상 계약
- 타겟 포인트 컴포넌트
- 선택 컴포넌트
- 후보/설정/결과 구조체
- 후보 수집 및 계층형 정렬
- 선택 수명과 유효성 처리
- 외부 장비 조회 API
- 디버그 데이터 제공

### Blueprint 계층

- 대상별 타겟 포인트 배치
- 차량·장치별 선택 가능 설정
- 입력 연결
- HUD 마커와 정보 패널
- 상태별 애니메이션
- 장비 호환 피드백
- 테스트 맵 시나리오

### 데이터 계층

- 거리
- 각도
- 갱신 주기
- 가림 유예
- 후보 전환 보정
- 후보 표시 정책
- 화면 밖 표시 정책
- 관계 및 정보 표시 규칙

---

## 4. 작업 패키지 개요

| Task ID | 작업명 | 주 책임 | 선행 Task |
|---|---|---|---|
| TS-P0-00 | 기존 구조 조사 | 문서/분석 | 없음 |
| TS-P0-01 | 계약 및 데이터 설계 | C++ | TS-P0-00 |
| TS-P0-02 | 타겟 포인트 기반 | C++/BP | TS-P0-01 |
| TS-P0-03 | 후보 탐색과 정렬 | C++ | TS-P0-01, 02 |
| TS-P0-04 | 선택 상태 수명 | C++ | TS-P0-03 |
| TS-P0-05 | 입력과 플레이어 연결 | C++/BP | TS-P0-04 |
| TS-P0-06 | 후보 및 선택 HUD | BP/UI | TS-P0-04, 05 |
| TS-P0-07 | 장비 조회 연동 | C++/BP | TS-P0-04 |
| TS-P0-08 | P0 검증과 튜닝 | QA/Data | TS-P0-05~07 |
| TS-P1-01 | 후보 순환과 필터 | C++/BP | P0 완료 |
| TS-P1-02 | 화면 밖·가림 UI | C++/BP | P0 완료 |
| TS-P1-03 | 디버그 도구 | C++/BP | P0 완료 |
| TS-P2-01 | 센서 정보 단계 | C++/Data/UI | P1 권장 |
| TS-P2-02 | 장비 획득 상태 | 장비 시스템 | TS-P2-01 |
| TS-P3-01 | 부위 선택 | C++/BP | P2 권장 |
| TS-P3-02 | 다중 타겟 | C++/UI | TS-P3-01 |

---

## 5. P0 상세 작업

## TS-P0-00 기존 구조 조사 — Done

### 공식 산출물

```text
TargetSelectInvestigation.md
```

### 확정 결과

```text
- TargetSelectComp 소유: ACFVehiclePawn C++ 기본 서브오브젝트
- 차량 선택 가능 계약: ACFVehiclePawn이 ICFTargetSelectable C++ 구현
- 직접 선택 조준 기준: UCFVehicleCameraComp의 카메라 Aim
- 무기 Aim 경계: WeaponAimSolution과 CurrentMuzzleDirection은 직접 선택 Trace에 사용하지 않음
- 입력 소유: ACFVehiclePawn::SetupPlayerInputComponent
- Mapping Context: /Game/CarFight/Input/IMC_Vehicle_Default
- 신규 입력: IA_SelectTarget, IA_ClearTarget
- HUD: WBP_AimReticle과 분리된 UCFTargetSelectWidget / WBP_TargetSelect
- 파괴 무효화: UCFVehicleHealthComp::OnVehicleDestroyed
- 관계: P0 Unknown 기본값, 공용 Faction 시스템은 별도 기능
- 충돌: TargetSelect 전용 Trace Channel 신규 추가
- 설정: UCFTargetSelectData + 컴포넌트 Fallback
- 네트워크: 로컬 싱글, 복제 없음
```

### 후속 세부 항목

```text
- TS-P0-03 전까지 VehicleCameraComp의 Aim Trace 시작 위치 getter 확정
- TS-P0-05에서 선택 및 해제의 최종 키보드·게임패드 키 확정
- TS-P0-06에서 BP_CFVehiclePawn TargetSelectWidgetClass와 ZOrder 확정
```

이 항목들은 담당 후속 Task가 정해져 있으며 TS-P0-01 작업지시 생성을 차단하지 않는다.

---

## TS-P0-01 계약 및 데이터 설계 — Done

### 완료 증거

```text
- ACFVehiclePawn TargetSelectComp 기본 서브오브젝트와 ICFTargetSelectable 구현 PASS
- BP_CFVehiclePawn 상속 컴포넌트 수 30 → 31 및 에셋 덤프 오류 0
- UHT Blueprint 함수·이벤트 Reflection PASS
- Native C++ 구현 직접 호출 / 실제 Blueprint 재정의 Execute 경계 보강
- CarFight.TargetSelect.TS_P0_01.RuntimeContract Result=Success
- 입력 거부, 분류·관계·태그 필터, 이벤트 중복 억제, 상태 전이, Fallback과 약한 참조 검증 PASS
- CarFight_ReEditor Win64 Development Exit Code 0
- Automation Report: UE/Saved/Automation/TargetSelect
```

TS-P0-01은 입력, 후보 Trace와 HUD가 없는 계약·상태 계층이므로 별도 사용자 PIE를 종료 게이트로 요구하지 않는다. 실제 플레이 조작 검증은 TS-P0-05~08에서 수행한다.

### 목표

선택 가능 대상과 타겟 선택 컴포넌트 사이의 최소 계약을 정의한다.

### 입력

- TS-P0-00 조사 결과
- `TargetSelect.md`의 확정 결정

### 작업

- 선택 가능 대상 계약 정의
- 타겟 표시 정보 구조 정의
- 후보 평가 데이터 정의
- 선택 컨텍스트 정의
- 관계와 센서 정보 단계 참조 방식 정의
- 설정 데이터 구조 정의
- Blueprint 공개 함수와 변수에 툴팁 작성
- 이벤트 이름과 전달 데이터 정의

### 산출물

- 헤더와 구현 파일
- 설정 데이터 정의
- Blueprint 노출 API
- API 주석과 툴팁
- 버전 및 변경 이력

### 완료 조건

- 구체 검색 로직 없이도 컴파일됨
- 대상 구현체가 최소 계약을 제공할 수 있음
- UI와 장비가 선택 대상 변경 이벤트를 받을 수 있음
- Blueprint 노출 항목에 설명 툴팁이 있음

### 검증

- 에디터 컴파일
- Blueprint에서 타입과 이벤트 노출 확인
- 잘못된 참조를 안전하게 처리하는지 확인

---

## TS-P0-02 타겟 포인트 기반 — Done

### 완료 증거

```text
- UCFTargetPointComp C++ SceneComponent 및 BlueprintSpawnable 계약 추가
- FCFTargetPointResult / ECFTargetPointSource로 위치와 출처 공개
- TargetPoint → Actor Bounds 중심 → Actor Location 안전 Fallback 구현
- ACFVehiclePawn TargetPoint 기본 서브오브젝트 연결
- 기존 차량 호환용 bUseAsTargetPoint=false 기본값 유지
- ICFTargetSelectable 기본 위치 반환과 차량 위치 반환을 공용 해석기로 정렬
- CarFight.TargetSelect.TS_P0_02.TargetPoint Result=Success
- CarFight.TargetSelect.TS_P0_01.RuntimeContract 회귀 Result=Success
- CarFight_ReEditor Win64 Development Build Job c79b3418a3b2405b97ad52df7fa57bb0 / Exit Code 0
- Automation Report: UE/Saved/Automation/TargetSelect
```

### 목표

액터 원점 대신 선택과 표시 기준으로 사용할 대표 타겟 포인트를 제공한다.

### 작업

- 타겟 포인트 컴포넌트 또는 동등 구조 구현
- 대표 포인트 조회
- 포인트가 없을 때 안전한 대체 위치 제공
- Blueprint에서 차량별 위치 조정 가능
- 디버그 위치 표시

### 산출물

- 타겟 포인트 C++ 타입
- 차량 Blueprint 적용 예시
- 폴백 규칙
- 디버그 표시

### 완료 조건

- 차량 원점과 무관하게 화면 중앙 근접도가 올바르게 계산됨
- 포인트가 없어도 크래시 없이 동작
- 차량마다 포인트 위치를 조정 가능

### 검증

- 서로 다른 크기의 차량 3종
- 원점 위치가 다른 대상
- 포인트 유무 비교

---

## TS-P0-03 후보 탐색과 정렬 — Done

### 완료 증거

```text
- TargetSelect 전용 ECC_GameTraceChannel3와 차량 SM_Body 선택 표면 추가
- UCFVehicleCameraComp.GetCurrentAimTraceStartLocation 공개
- FCFTargetSearchView / FCFTargetSearchResult Blueprint 계약 추가
- 카메라 Aim 직접 조준 우선, 화면 근접도, 월드 거리, StableSortKey의 계층형 결정 정렬 구현
- CandidateSwitchAdvantageRatio 기반 후보 전환 안정화 구현
- FOV와 16:9·32:9 화면비를 포함한 CandidateRanking Automation 추가
- Build Job 6b359cbce86449dea5cf35daacd15ed5
- TS-P0-01 RuntimeContract, TS-P0-02 TargetPoint, TS-P0-03 CandidateRanking 전체 Success
- 전체 3 성공 / 0 실패 / 경고 0 / 오류 0 / Exit Code 0
```

최신 TS-P0-04 통합 실행에서도 `TS-P0-03 CandidateRanking`은 Success를 유지했다.

### 목표

직접 조준 우선, 크로스헤어 근접 차선의 계층형 후보 평가를 구현한다.

### 작업

- 중앙 직접 판정
- 근접 후보 수집
- 사전 필터
- 화면 근접도 계산
- 계층형 정렬
- 후보 안정화
- 결정적 최종 정렬
- 설정값 적용
- 디버그 후보 목록 제공

### 산출물

- 후보 검색 함수
- 후보 평가 함수
- 후보 변경 이벤트
- 디버그 정보

### 완료 조건

- 직접 조준 대상이 항상 근접 후보보다 우선
- 직접 대상이 없을 때 중앙에 가까운 후보 선택
- 화면 근접도가 월드 거리보다 우선
- 후보가 비슷할 때 표시가 흔들리지 않음
- 같은 입력과 장면에서 결과가 결정적임

### 검증

- 적·아군 겹침
- 원거리 중앙 대상과 근거리 화면 가장자리 대상
- 고속 차량 교차
- FOV 변경
- 16:9와 울트라와이드 비율

---

## TS-P0-04 선택 상태 수명 — Done

### 완료 구현

```text
- 선택 대상 AActor.OnDestroyed 자동 구독
- 선택 대상 AActor.OnEndPlay 자동 구독
- 차량 UCFVehicleHealthComp.OnVehicleDestroyed 자동 구독
- 선택 변경·해제 시 이전 수명 Delegate 제거
- 파괴, 선택 불가, 추적 거리 이탈, 시스템 비활성화와 Owner 종료 해제 경로 구현
- OcclusionGracePeriodSec 기반 Occluded 유지와 가림 유예 종료 구현
- 화면 밖 이동과 카메라 방향 변경은 선택 해제 조건에서 제외
- 자동 다음 타겟 금지 유지
```

### 완료 증거

```text
Build Job: bb63b77b7a174afda89762fd0d2d712c
UBT Editor Build: Result Succeeded
Automation Total: 4
Success: TS-P0-01 RuntimeContract, TS-P0-02 TargetPoint, TS-P0-03 CandidateRanking, TS-P0-04 SelectionLifetime
집계: 4 성공 / 0 실패 / 경고 0 / 오류 0
Automation Test Complete Exit Code: 0
TargetSelect automation suite passed
```

Automation 월드에서 BeginPlay 이전 Actor 파괴는 `OnDestroyed`로 즉시 처리하고, `OnEndPlay`는 스트리밍·레벨 제거 경로로 유지한다. 먼저 도착한 수명 이벤트가 선택을 해제하며 후속 이벤트는 선택 기록 부재 검사로 중복 처리되지 않는다.

### 목표

선택 확정, 유지, 해제, 무효화와 가림 유예를 관리한다.

### 작업

- 선택 확정 API
- 수동 해제 API
- 같은 대상 재선택 무시
- 후보 없음 입력 시 기존 선택 유지
- 대상 파괴·제거 감지
- 화면 밖 선택 유지
- 가림 유예
- 선택 변경 이벤트
- 안전한 약한 참조 또는 수명 관리

### 산출물

- 선택 상태 로직
- 유효성 검사
- 선택 변경 이벤트
- 추적 상태 최소 표현

### 완료 조건

- 카메라를 돌려도 선택 유지
- 대상이 잠시 가려져도 즉시 해제되지 않음
- 대상 제거 후 유효하지 않은 참조가 남지 않음
- 자동 다음 타겟이 발생하지 않음

### 검증

- 대상 파괴
- 대상 스트리밍 아웃 또는 제거
- 장애물 통과
- 최대 거리 이탈
- 선택 컴포넌트 비활성화

---

## TS-P0-05 입력과 플레이어 연결 — Done

### 완료 구현

```text
- ACFVehiclePawn.InputAction_SelectTarget / InputAction_ClearTarget 추가
- ACFVehiclePawn::SetupPlayerInputComponent에서 Started 이벤트 바인딩
- ConfirmCurrentTargetCandidate BlueprintCallable 명령 추가
- ClearSelectedTargetManually BlueprintCallable 명령 추가
- 후보 없음 선택 입력은 기존 선택 유지
- Manual 해제는 현재 후보 유지와 자동 다음 타겟 금지
- IA_SelectTarget Boolean + Pressed 생성
- IA_ClearTarget Boolean + Pressed 생성
- IMC 선택: MiddleMouseButton / Gamepad_RightThumbstick
- IMC 해제: RightMouseButton / Gamepad_FaceButton_Right
```

### 완료 증거

```text
최종 Build Job: efdc298d3108451ca60a9906a7612333
CarFight_ReEditor Win64 Development: Succeeded
Automation: TS-P0-01~05 전체 Success
집계: succeeded 5 / succeededWithWarnings 0 / failed 0
각 테스트 warnings 0 / errors 0
Automation Exit Code: 0
Input AssetDump: 15 성공 / 0 실패
신규 에셋: IA_SelectTarget, IA_ClearTarget
수정 에셋: IMC_Vehicle_Default
```

실사용 입력 감각, UI 포커스, 재시작과 Possess 변경은 Target HUD와 장비 연결까지 완료한 뒤 TS-P0-08 통합 PIE에서 최종 확인한다.

### 목표

타겟 선택과 해제 입력을 실제 플레이어 흐름에 연결한다.

### 작업

- 기존 Enhanced Input 구조 조사 결과 반영
- 선택 입력 연결
- 해제 입력 연결
- 입력 소유 위치 결정
- UI 입력 모드와 충돌 방지
- 플레이어 사망·차량 교체 시 상태 처리

### 산출물

- 입력 액션 또는 기존 액션 연결
- Pawn/Controller 연결
- 입력 툴팁과 문서

### 완료 조건

- 플레이 중 선택과 해제가 안정적으로 동작
- 입력 컨텍스트 중복 추가가 없음
- 로컬 플레이어 서브시스템 오류가 없음
- 차량 교체 또는 재시작 후 정상 동작

### 검증

- PIE 단일 플레이어
- 재시작
- Possess 변경
- UI 포커스 상태

---

## TS-P0-06 후보 및 선택 HUD — Done

### 완료 구현

```text
- UCFTargetSelectWidget C++ 부모 위젯
- WBP_TargetSelect Blueprint 배치·스타일 위젯
- TargetSelectComp 후보·선택·해제·유효성·추적 상태 이벤트 구독
- 후보와 선택 대상 월드 위치의 Viewport 투영
- 후보·선택 이름과 미터 거리 표시
- 선택 관계와 Visible/Occluded/Estimated/SignalLost 상태 표시
- 후보 ◇ / 선택 ▣ / 가림 선택 ▧ 형태 구분
- 같은 Actor 후보·선택 중복 마커 억제
- 화면 밖 마커 숨김과 선택 상태 유지
- Pawn BeginPlay·Input 준비·EndPlay Viewport 수명 연결
- TargetSelect HUD 기본 ZOrder 20 / Aim Reticle ZOrder 10 유지
```

### 완료 증거

```text
최종 Build Job: 26ef764a7de94b9ab46b221fcbc47805
CarFight_ReEditor Win64 Development: Succeeded
Automation: TS-P0-01~06 전체 Success
집계: succeeded 6 / succeededWithWarnings 0 / failed 0
각 테스트 warnings 0 / errors 0
Automation Exit Code: 0
UI AssetDump: WidgetBlueprint 7 성공 / 0 실패
WBP_TargetSelect Parent: /Script/CarFight_Re.CFTargetSelectWidget
Widget Tree: 8 widgets / root CanvasPanel_Root
```

실제 16:9·32:9 화면 크기, UI Scale과 고속 이동 중 가독성은 TS-P0-08 통합 PIE에서 최종 확인한다.


### 목표

후보, 선택, 가림 상태를 화면에 명확히 표시한다.

### 작업

- 후보 마커
- 선택 마커
- 거리
- 이름 또는 종류
- 관계 아이콘
- 현재 장비 호환 상태 자리 확보
- 화면 좌표 변환
- 가림 표시 최소 상태
- 마커 생성·회수 정책
- 색상 외 형태 차이 제공

### 산출물

- 후보/선택 마커 위젯
- 타겟 정보 패널 최소 버전
- HUD 연결
- 상태별 표현 표

### 완료 조건

- 후보와 선택 표시를 혼동하지 않음
- 화면 중앙을 과도하게 가리지 않음
- 선택 대상 이동을 안정적으로 추적
- 색상 없이도 주요 상태를 구분 가능
- 대상 무효화 시 위젯이 안전하게 제거됨

### 검증

- 다수 후보
- 고속 이동
- 화면 크기 변경
- 울트라와이드
- 대상 파괴
- UI 스케일 변경

---

## TS-P0-07 장비 조회 연동 — Done

### 완료 구현

```text
- ECFTargetUseFailureReason 11개 실패 사유 계약
- FCFTargetUsePolicy 대상 분류·관계·필수 태그·제외 태그·추적 상태 정책
- FCFTargetUseRequest 장비 ID·준비 상태·최대 사용 거리·명시적 원점 계약
- FCFTargetUseResult 선택 존재·유효성·호환성·거리·최종 사용 가능 상태 분리
- UCFTargetSelectComp 읽기 전용 EvaluateSelectedTargetForUse API
- UCFWeaponData.TargetUsePolicy 데이터 필드
- UCFVehicleWeaponComp 활성 무기 평가 요청·결과 캐시·변경 이벤트
- 선택 변경·해제·유효성·추적 상태 이벤트 기반 즉시 재평가
- 선택 대상이 있는 동안 0.10초 저빈도 Tick으로 이동 거리와 사거리 진입·이탈 자동 재평가
- 장비 사용 실패 시 공용 선택 유지
- 직접 조준 FireOrigin과 발사 방향 비침범
```

### 완료 증거

```text
최종 Build Job: b7c2eb766e984c8ea6f5d3d847cee1f6
CarFight_ReEditor Win64 Development: Succeeded
Automation: TS-P0-01~07 전체 Success
집계: succeeded 7 / succeededWithWarnings 0 / failed 0 / notRun 0
각 테스트 warnings 0 / errors 0
TS-P0-07 EquipmentQuery: Success
Automation Exit Code: 0
TargetSelect automation suite passed
```

자동화는 선택 없음, 호환 대상, 관계·분류 비호환, 필수 태그 누락, 제외 태그, 거리 초과, 선택 대상 이동에 따른 사거리 이탈·복귀, 가림 상태, 대상 파괴, 선택 전환과 장비 비호환 상태를 검증했다. 모든 선택·장비 평가 과정에서 기존 FireOrigin 위치와 방향이 보존됨을 확인했다.

### 목표

무기와 유틸리티 장비가 선택 대상을 안전하게 조회하고 호환 여부를 판단할 수 있게 한다.

### 작업

- 선택 대상 조회 API
- 선택 변경 구독 방식
- 장비 호환성 검사 진입점
- 선택과 장비 획득 상태 분리
- 사용 불가 사유 표현
- 직접 조준 무기가 선택 방향으로 자동 보정되지 않도록 보호

### 산출물

- 장비 연동 계약
- 최소 예제 장비 1종
- 사용 가능·불가 피드백

### 완료 조건

- 장비가 선택 대상을 조회 가능
- 선택 대상이 없을 때 안전하게 실패
- 선택 가능하지만 장비 사용 불가인 상태 표현
- 선택이 직접 조준 발사 방향을 변경하지 않음

### 검증

- 호환 대상
- 비호환 대상
- 거리 초과
- 대상 파괴 중 장비 사용
- 선택 변경 중 장비 획득

---

## TS-P0-08 P0 검증과 튜닝

### 일시중지 체크포인트 — 2026-07-24

```text
상태: Paused
사유: 사용자 우선순위 변경으로 CF-FQ-024 전투 FX 기능 개발을 먼저 진행
완료 유지: TS-P0-00~07 Done
최종 자동 검증: Build Job 0fce6d253d9548dfa0ed39c94501ff47 / TargetSelect 7/7 PASS / 경고 0 / 오류 0
미완료 유지: TS-P0-08 사용자 PIE 통합 검증과 튜닝
재개 시 첫 결함:
1. 후보 텍스트가 대상 차량과 겹쳐 가독성이 낮음
2. 크로스헤어 인접 후보 선택 범위가 너무 넓음
3. 대표 위치 확인용 디버그 원이 화면에서 식별되지 않음
```

TargetSelect 코드와 에셋은 삭제하거나 되돌리지 않는다. 2026-08-15 사용자 지시로 CF-FQ-026을 다시 Active로 전환했으며, 기존 `WBP_TargetSelect.uasset` 선행 dirty 상태는 보호하고 원격 기술 작업에서 저장·덮어쓰지 않는다.

### 원격 기술 재검증 — 2026-08-15

```text
[TECH PASS] 공식 Build `2aa5462fbd5445379416423210054fec` / Exit 0
[TECH PASS] TS-P0-08 SingleTargetBoundary `86dbf8b8b40248768ba0fa593ed8f25e` / 1/1
[TECH PASS] TS-P0-01 RuntimeContract `e32ab6f7f4ef47e49652f3266cfe56fd` / 1/1
[TECH PASS] TS-P0-02 TargetPoint `d00f454eb28c4a298a540349c0a95201` / 1/1
[TECH PASS] TS-P0-03 CandidateRanking `61b4cc5f83a541f099838fa9638ca680` / 1/1
[TECH PASS] TS-P0-04 SelectionLifetime `fad8935b5eeb42c187fb4826eef774aa` / 1/1
[TECH PASS] TS-P0-07 EquipmentQuery `1125a6da807f473c87ca3a34b4b120b8` / 1/1
[NOT RUN] TS-P0-05 Input / TS-P0-06 HUD / 전체 TargetSelect suite — 현재 dirty WBP_TargetSelect 및 자산 저장형 테스트 보호
```

이번 원격 작업은 `ProximityHalfAngleDeg=7.0`, `ProximitySelectMaxDistanceCm=120000`, `CandidateSwitchAdvantageRatio=0.15`를 변경하지 않았다. 새 `SingleTargetBoundary`는 후보 경쟁을 제거한 단일 대상 조건에서 ±6.9° 수락, ±7.1° 거부가 16:9와 32:9 모두 좌우 대칭임을 확인했다. 따라서 과거 동일 차량 인스턴스 비대칭을 반각 계산 자체의 좌우 비대칭으로는 재현하지 못했다.

자동 후보 디버그 표시는 기존 `Duration=0` 한 프레임에서 `CandidateRefreshIntervalSec` 동안 유지하도록 교정했고, Sphere 반경 20cm는 `CandidateSearchDebugSphereRadiusCm` Debug 설정으로 분리했다. 이는 디버그 시각 진단만 변경하며 후보 판정 범위·반각·히스테리시스에는 영향을 주지 않는다.

AssetDump에서 `/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn`의 `TargetSelectComp`와 `TargetPoint`가 C++ inherited CDO 컴포넌트이고 BP SCS의 `SM_Body`가 존재함을 확인했다. 다만 현재 AssetDump 표면은 inherited component의 개별 serialized property override 값을 제공하지 않으므로 실제 `TargetSelectData` override 유무는 추정하지 않는다.

USER 상태는 변경하지 않는다. 이전 PASS 항목은 보호하고, `동일 차량 대표 위치/인식 범위 재검증`, `후보 범위 과대 체감`, `디버그 Sphere 실제 가시성`, `후보 텍스트 차량 겹침`, `16:9·32:9 실제 UI/입력 감각`은 모두 Pending이다.

### 사용자 PIE 부분 검증 — 2026-07-24

사용자 판정 규칙:

```text
사용자가 별도로 지적하지 않은 검증 항목은 정상으로 기록한다.
```

1차 마우스 검증 결과:

```text
[PASS] 크로스헤어 주변 후보 마커와 후보 정보 표시
[PASS] 가운데 마우스 선택
[PASS] 후보와 선택이 같은 Actor일 때 마커 중복 방지
[PASS] 후보가 없을 때 선택 입력이 기존 선택 유지
[PASS] 오른쪽 마우스 선택 해제와 후보 유지
[PASS] 화면 밖 선택 상태 유지
[FAIL / 수정 적용, 사용자 재검증 대기] 동일한 BP_CFVehiclePawn 인스턴스 사이에서 후보 대표 위치와 인식 영역이 다르게 체감됨
```

관찰 화면에서는 동일 차량 두 대의 후보 획득 영역이 서로 다른 크기와 형태로 나타났다. 현재 후보 표시와 기본 HUD 동작 자체는 정상이다.

현재 코드·에셋 대조 결과:

```text
- 두 대상은 같은 /Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn 클래스를 사용한다.
- 근접 후보 진입은 TargetPoint 방향과 카메라 방향 사이의 CrosshairAngleDeg가 ProximityHalfAngleDeg 이하인지 검사한다.
- 현재 후보와 도전 후보가 동시에 유효하면 CandidateSwitchAdvantageRatio 0.15 히스테리시스가 기존 후보를 유지할 수 있다.
- BP에서 명시 TargetPoint를 활성화하지 않았다면 Actor Bounds 중심을 대표 위치로 사용한다.
- Actor Bounds는 런타임 컴포넌트 Transform과 회전 상태에 따라 인스턴스별 중심이 달라질 수 있다.
```

수정 적용 결과:

```text
- UCFTargetPointComp에 지정 PrimitiveComponent Bounds 중심 자동 정렬과 로컬 오프셋을 추가했다.
- ACFVehiclePawn TargetPoint는 SM_Body를 기준으로 사용하고 차체 메시 적용 후 다시 정렬한다.
- TS-P0-02 자동화에 Bounds 중심 정렬, 부착 부모와 오프셋 검증을 추가했다.
- Build Job 0fce6d253d9548dfa0ed39c94501ff47에서 Editor 빌드와 TS-P0-01~07 전체 7개 Automation이 성공했다.
- 사용자 PIE에서 정지 차량의 후보 마커 위치와 인식 범위를 다시 확인해야 한다.
```

원인 분리 절차:

```text
1. 다른 차량이 후보 원뿔에 들어오지 않는 단일 대상 조건에서 각 차량의 진입·이탈 범위를 비교한다.
2. 단일 대상에서는 동일하면 후보 경쟁과 15% 히스테리시스에 의한 전환 영역 비대칭으로 판정한다.
3. 단일 대상에서도 다르면 두 차량의 TargetPoint 해석 위치와 Actor Bounds 중심을 비교한다.
4. 원인 확정 전 ProximityHalfAngleDeg 또는 CandidateSwitchAdvantageRatio를 임의 조정하지 않는다.
```

### 목표

P0 기획 검증 시나리오를 통과하고 초기 설정값을 플레이테스트 가능한 수준으로 조정한다.

### 작업

- 자동 또는 수동 테스트 체크리스트 작성
- 성능 프로파일
- 후보 검색 빈도 조정
- 거리·각도·가림 시간 조정
- 디버그 표시
- 알려진 한계 기록
- 문서 상태 갱신

### 완료 조건

- `TargetSelect.md`의 P0 검증 시나리오 통과
- 후보 검색이 목표 프레임 시간에 과도한 부담을 주지 않음
- 튜닝값이 데이터로 분리됨
- 알려진 문제와 P1 이관 항목이 기록됨

---

## 6. Plan 작업 단위 규칙

후속 Plan 작업지시는 다음 원칙을 따른다.

- 한 작업지시서에는 Task ID 하나만 포함
- 조사 Task와 구현 Task를 혼합하지 않음
- C++ 파일과 Blueprint 에셋은 실제 경로 확인 후 기록
- 완료 조건을 실행 가능한 검증으로 표현
- 범위 밖 리팩터링 금지
- 문서 상태 갱신을 같은 Task의 마지막 단계에 포함
- 실패 또는 불명확 시 중단 조건을 명시
- 기존 변경사항이 있는 파일은 diff 확인 후 처리

---

## 7. 공통 완료 정의

모든 구현 Task는 다음 조건을 만족해야 완료다.

1. 컴파일 또는 Blueprint 컴파일 성공
2. 경고와 오류가 새로 증가하지 않음
3. 관련 검증 시나리오 통과
4. 디버그 확인 수단 존재
5. 공개 변수와 함수의 툴팁 작성
6. 파일명 32자 이내
7. 변경 이력 작성
8. 필요 시 마이그레이션 절차 작성
9. 작업 범위 밖 변경 없음
10. 최종 diff 검토

---

## 8. 리스크

| 리스크 | 영향 | 대응 |
|---|---|---|
| 기존 조준 판정과 중복 | 성능과 결과 불일치 | TS-P0-00에서 재사용 여부 확인 |
| 액터 원점 의존 | 대형 차량 선택 오류 | 타겟 포인트 의무화 |
| Tick 전수 검색 | 성능 저하 | 갱신 주기와 공간 후보 제한 |
| UI와 선택 로직 결합 | 확장 어려움 | 이벤트 기반 분리 |
| 장비가 선택을 소유 | 다중 장비 충돌 | 선택은 공통, 획득은 장비 소유 |
| 가중치 점수 남용 | 의도와 다른 선택 | 계층형 비교 유지 |
| 가림 판정 과다 | 트레이스 비용 증가 | 선택 대상 중심 제한 검사 |
| 네트워크 범위 불명 | 재작업 | TS-P0-00에서 P0 권한 모델 결정 |

---

### TS-P0-08 원격 기술 재검증 추가 — 2026-08-15

```text
Latest Official Build: 4b4554ab8f7e455abf0c0538ae10a664 / Exit 0
Latest TS-P0-08: 3d8bb0ebbeb5427f954cc7621b432408 / 3/3 Success / 0 Fail
Final safe regression:
- TS-P0-01 9b0fa91fa4c64f5cbaf8a4d7e0786099 / 1/1
- TS-P0-02 8d85bc81a2724def877c75be2f8673a0 / 1/1
- TS-P0-03 1802f96018de456896913fee072d2bb1 / 1/1
- TS-P0-04 7884ac75e39b492c8120659447e3a0c0 / 1/1
- TS-P0-07 ba15980c28bb4656ac8e88d1ae93b943 / 1/1
```

`SettingsPath`가 저장된 `BP_CFVehiclePawn` GeneratedClass CDO를 Load-only로 확인한 결과 실제 설정 source는 `FallbackTargetSelectConfig`이며 `TargetSelectData=None`이다.

```text
DirectSelectMaxDistance = 2000m
ProximitySelectMaxDistance = 1200m
ProximityHalfAngle = 7deg
CandidateRefreshInterval = 0.05s / 20Hz
OcclusionGrace = 1.5s
CandidateSwitchAdvantageRatio = 0.15
AutoDebug = true
DebugSphereRadius = 20cm
TargetPoint = Use true / AutoAlign true / PreferredBounds SM_Body / Offset 0
```

따라서 7도·1200m·15%는 현재 표준 차량 BP가 실제 사용하는 기본값으로 확정했다. 다만 이는 기술 설정 확인이지 사용자에게 적절한 범위라는 판정이 아니며 `후보 범위 과대` USER 관찰은 그대로 Pending이다.

런타임 검색 진단을 `LastCandidateSearchResult`에 추가했다. 마지막 Transient 회귀 샘플은 다음과 같다.

```text
WorldScanned=11
Input=3
Accepted=1
VisibilityTraces=1
PrefilterSkipped=2
TotalTraces=2
SearchMs=0.1780
```

이 수치는 작은 Transient World의 구조 검증 샘플이며 성능 PASS 기준으로 사용하지 않는다. 현재 자동 갱신은 20Hz마다 전체 `TActorIterator<AActor>`를 순회하는 구조를 유지하므로 대표 게임 맵·대상 규모·프레임 예산 없이 Registry/Spatial Query로 임의 교체하지 않는다.

비-Direct Targetable Actor에 대해 기존 proximity 거리·반각 밖이면 LOS Trace만 생략하는 사전필터를 적용했다. 모든 Targetable Actor는 기존처럼 후보 입력 배열에 남고 `EvaluateCandidateActors`, 직접 조준 우선, 결정적 정렬과 15% 히스테리시스 의미는 바꾸지 않는다. 회귀에서 Targetable 3개를 유지하면서 기존 구조상 가능했던 Direct 1 + LOS 3 대신 Direct 1 + LOS 1, `PrefilterSkipped=2`를 확인했다.

USER PASS는 추가하지 않는다. 실제 debug Sphere 가시성, 동일 차량 대표 위치/인식 영역, 후보 범위 체감, 후보 텍스트 겹침, 입력 감각과 16:9·32:9 화면 검증은 계속 Pending이다.

## 9. 변경 이력

### v0.13.0 - 2026-09-14

- `CF-FQ-026`을 current Source/Asset/System 기준으로 Rebaseline해 `SUPERSEDED / CLOSE`, Done / Historical + Retained Path로 전환했다.
- TS-P0-00~07과 기존 TS-P0-08 remote technical evidence는 Historical Technical PASS로 보존하고 반복 실행하지 않았다.
- 과거 20Hz 전체 World `TActorIterator` 반복 후보 수집은 `UCFTargetRegistrySubsystem`의 1회 bootstrap + 증분 등록 + snapshot 후보 공급으로 successor-resolved 됐음을 확인했다.
- TargetSelect가 공용 Candidate/Selected Actor authority를 계속 소유하며 Sensor/Scanner는 Detection/Contact owner라는 현재 경계를 main_game `Document/Systems/Targeting/TargetSelect.md v1.0.0`으로 승격했다.
- fresh AssetDump에서 persisted `IA_SelectTarget`, `IMC_Vehicle_Default`의 Select/Clear mapping과 `WBP_TargetSelect` 존재를 확인했다. Asset mutation은 없었다.
- 동일 차량 TargetPoint/인식 영역은 Technical correction evidence만 보존하고 USER 상태는 `USER Inconclusive`; 7°/1200m 후보 체감은 Deferred USER tuning debt, Debug Sphere 가시성은 Deferred observational debt로 분리했다.
- 후보 텍스트 겹침과 old 16:9/32:9 UI workflow는 successor UISubsystem + marker + Production TargetPanel 구조로 역할이 바뀌어 `Superseded / Not Executed`이며 사후 USER PASS로 변경하지 않았다.
- Source/Asset mutation 0, Build/Automation/PIE 재실행 0, 새 USER PASS 0이다. G0~G4 PASS / G5 Deferred다.

Migration: `TargetSelectPlan.md v0.13.0`은 Historical evidence owner다. 새 작업에서 old `TS-P0-08 USER PIE`를 current next gate로 자동 재개하지 말고 TargetSelect 현재 계약은 main_game `Document/Systems/Targeting/TargetSelect.md v1.0.0`, Sensor Contact는 `SensorContact.md v1.2.0`, 현재 HUD workflow는 Current UI owner를 기준으로 판단한다.

### v0.12.5 - 2026-08-15

- 저장된 `BP_CFVehiclePawn` CDO의 실제 TargetSelect 설정 source가 `FallbackTargetSelectConfig`임을 Load-only Automation으로 확정했다.
- 런타임 검색의 World Actor 스캔 수, Visibility/전체 Trace 수, LOS 사전필터 생략 수와 검색 ms를 관측 가능하게 했다.
- 기존 proximity 거리·반각 밖 비-Direct 대상은 후보 배열에서 제거하지 않고 LOS Trace만 생략하는 의미 보존 사전필터를 적용했다.
- 공식 Build `806a8dba54434dc38b1b83baa310c50c` PASS, TS-P0-08 `eeaa81f44b504d33be091a2116868ac7` 3/3, 최종 자산 비변경 TS-P0-01·02·03·04·07 각 1/1 PASS를 확인했다.
- Transient 샘플 `WorldScanned=11 / Input=3 / VisibilityTraces=1 / PrefilterSkipped=2 / TotalTraces=2 / SearchMs=0.1780`은 구조 검증용이며 성능 PASS로 해석하지 않는다.
- 전체 Actor 20Hz 순회는 현재 남은 구조적 비용이다. 대표 workload가 없는 상태에서 Registry/Spatial Query 전환을 임의 적용하지 않는다.
- 기존 USER PASS·FAIL·재검증 항목과 dirty WBP 보호는 그대로 유지한다.

### v0.12.4 - 2026-08-15

- 사용자 지시로 CF-FQ-026 / TS-P0-08을 Active로 재개했다. 기존 USER PASS·FAIL·재검증 체크포인트는 변경하지 않았다.
- 자동 후보 디버그 Sphere의 수명을 한 프레임에서 후보 갱신 간격까지 유지하도록 교정하고 반경을 Debug 설정으로 분리했다. 게임플레이 후보 튜닝값은 변경하지 않았다.
- `CFTargetTuneTests.cpp`의 `TS_P0_08.SingleTargetBoundary`를 추가해 현재 7도 fallback 경계의 좌우 대칭성과 16:9·32:9 월드 반각 일관성을 검증했다.
- 공식 Build PASS, TS-P0-08 1/1과 자산 비변경 TS-P0-01·02·03·04·07 각 1/1 PASS를 확인했다.
- dirty `WBP_TargetSelect.uasset` 보호 때문에 자산 저장 가능성이 있는 TS-P0-05 Input·TS-P0-06 HUD와 전체 TargetSelect suite는 이번 세션 실행하지 않았다.
- 실제 후보 범위 체감, debug Sphere 시각성, HUD 겹침과 동일 차량 인식 영역 USER 재검증은 Pending으로 유지한다.

### v0.12.3 - 2026-07-24

- 사용자 우선순위 변경에 따라 TS-P0-08을 완료 처리하지 않고 Paused 상태로 전환했다.
- 최신 PIE에서 후보 텍스트와 대상 겹침, 후보 선택 범위 과대, 디버그 원 비가시를 재개 시 첫 결함으로 기록했다.
- TS-P0-00~07 Done과 Build Job `0fce6d253d9548dfa0ed39c94501ff47`의 전체 7개 Automation PASS 증거는 유지한다.
- 다음 활성 작업을 `CF-FQ-024 전투 FX`로 전환하고 TargetSelect 코드·에셋을 보존하도록 고정했다.

### v0.12.2 - 2026-07-24

- TS-P0-08 PIE에서 정지 상태의 동일 차량 인스턴스 후보 대표 위치 불일치를 확인했다.
- 차량 TargetPoint를 SM_Body Bounds 중심에 자동 정렬하고 차량별 로컬 오프셋을 지원하도록 수정했다.
- TS-P0-02 TargetPoint 자동화에 선호 Bounds 정렬 회귀를 추가했다.
- Build Job `0fce6d253d9548dfa0ed39c94501ff47`에서 Editor 빌드와 TargetSelect 7/7 PASS, 경고 0, 오류 0, Exit Code 0을 확인했다.
- 결함 상태를 코드 수정 완료와 사용자 PIE 재검증 대기로 전환했다.

### v0.12.1 - 2026-07-24

- TS-P0-08 사용자 PIE 1차 마우스 검증에서 별도 지적 없는 기본 입력·HUD 항목을 정상으로 기록했다.
- 동일한 BP_CFVehiclePawn 인스턴스 사이에서 후보 인식 화면 영역이 다르게 체감되는 결함을 조사 중 상태로 등록했다.
- 후보 경쟁과 15% 히스테리시스에 의한 비대칭인지, 명시 TargetPoint 미사용 상태의 Actor Bounds 중심 차이인지 분리하는 재현 절차를 추가했다.
- 원인 확정 전 근접 반각과 후보 전환 우위 비율을 임의로 튜닝하지 않도록 고정했다.

### v0.12.0 - 2026-07-24

- `CFTargetUseTypes.h`에 장비 타겟 사용 정책, 평가 요청, 단계별 결과와 11개 실패 사유를 추가했다.
- TargetSelectComp에 선택 상태를 변경하지 않는 장비 사용 평가와 디버그 요약 API를 추가했다.
- 선택 존재, 선택 유효성, 장비 준비, 대상 호환, 거리 충족과 최종 사용 가능 여부를 서로 다른 상태로 분리했다.
- WeaponData에 TargetUsePolicy를 추가하고 WeaponComp를 선택 변경 이벤트 기반 첫 소비 예제로 연결했다.
- 관계·분류·필수 태그·제외 태그·추적 상태·거리 조건 실패를 구체적인 enum과 메시지로 반환하도록 구현했다.
- 장비 평가 실패가 공용 선택을 해제하거나 별도 획득 상태를 만들지 않으며 직접 조준 FireOrigin과 발사 방향을 변경하지 않도록 보호했다.
- 최종 Build Job `b7c2eb766e984c8ea6f5d3d847cee1f6`에서 선택 대상 이동 사거리 이탈·복귀를 포함한 TS-P0-01~07 전체 7개 Automation 성공, 경고 0, 오류 0, Exit Code 0을 확인했다.
- TS-P0-07을 Done으로 전환하고 현재 Task를 TS-P0-08 P0 검증과 튜닝으로 변경했다.

### v0.11.0 - 2026-07-24

- `UCFTargetSelectWidget`과 `WBP_TargetSelect`을 추가하고 상태·투영 C++와 배치·스타일 Blueprint 책임을 분리했다.
- 후보·선택·해제·유효성·추적 상태 이벤트를 구독하고 대상 이름, 거리, 관계와 추적 상태를 표시하도록 연결했다.
- 후보 `◇`, 선택 `▣`, 가림 선택 `▧` 형태로 색상 외 구분 수단을 추가하고 같은 Actor의 후보 마커 중복을 억제했다.
- 화면 밖 대상은 P0에서 마커만 숨기고 선택 상태는 유지하도록 확정했다.
- Pawn에 WBP_TargetSelect 기본 클래스, 표시 토글, ZOrder 20과 Viewport 생성·정리 수명을 연결했다.
- Build Job `26ef764a7de94b9ab46b221fcbc47805`에서 TS-P0-01~06 전체 6개 Automation 성공, 경고 0, 오류 0, Exit Code 0을 확인했다.
- UI AssetDump 7/7 성공과 WBP_TargetSelect의 C++ 부모·8개 위젯 트리를 확인했다.
- TS-P0-06을 Done으로 전환하고 현재 Task를 TS-P0-07 장비 조회 연동으로 변경했다.

### v0.10.0 - 2026-07-24

- Pawn에 선택·해제 Input Action 프로퍼티와 현재 후보 선택 확정·Manual 해제 명령 API를 추가했다.
- `SetupPlayerInputComponent`에서 두 Action을 Started 이벤트로 연결하고 후보 없음 유지·후보 보존·자동 다음 타겟 금지 정책을 구현했다.
- `IA_SelectTarget`, `IA_ClearTarget`을 Boolean Pressed Action으로 생성하고 `IMC_Vehicle_Default`에 키보드·게임패드 4개 매핑을 저장했다.
- `TS-P0-05 InputIntegration`에서 에셋 생성·멱등 매핑, Pawn 명령과 상태 보존 계약을 자동 검증했다.
- Build Job `efdc298d3108451ca60a9906a7612333`에서 TS-P0-01~05 전체 5개 Automation 성공, 경고 0, 오류 0, Exit Code 0을 확인했다.
- TS-P0-05를 Done으로 전환하고 현재 Task를 TS-P0-06 후보 및 선택 HUD로 변경했다.

### v0.9.0 - 2026-07-24

- BeginPlay 이전 Automation Actor의 `Destroy()`가 `OnEndPlay`만으로 즉시 통지되지 않는 원인을 확인했다.
- 선택 대상에 `AActor.OnDestroyed` 구독을 추가하고 기존 `OnEndPlay`, `VehicleHealth.OnVehicleDestroyed` 수명 경계를 유지했다.
- Build Job `bb63b77b7a174afda89762fd0d2d712c`에서 UHT와 Editor 컴파일 성공을 확인했다.
- TargetSelect 전체 4개 Automation이 성공하고 실패 0, 경고 0, 오류 0, Exit Code 0임을 확인했다.
- TS-P0-04를 Done으로 전환하고 현재 Task를 TS-P0-05 입력과 플레이어 연결로 변경했다.

### v0.8.0 - 2026-07-24

- TS-P0-03의 전용 TargetSelect Trace, 카메라 Aim 후보 수집, 결정적 계층 정렬, 후보 안정화와 자동화 검증을 완료 상태로 등록했다.
- TS-P0-03 공식 증거를 Build Job `6b359cbce86449dea5cf35daacd15ed5`, 전체 3개 TargetSelect Automation 성공과 Exit Code 0으로 기록했다.
- TS-P0-04의 EndPlay·VehicleHealth 파괴 구독, 가림 유예, 추적 거리 이탈과 시스템 비활성화 해제 구현을 현재 코드 상태로 기록했다.
- Build Job `7204a1766a084306aedd3d274017b091`에서 UHT와 Editor 컴파일 성공, Automation 2 성공 / 2 실패, 경고 0 / 오류 7, 최종 Exit Code 255를 확인했다.
- Actor `Destroy()` 직후 자동 해제 기대 실패를 현재 차단 요소로 등록하고 TS-P0-04를 Active 상태로 유지했다.

### v0.7.0 - 2026-07-24

- Build Job `c79b3418a3b2405b97ad52df7fa57bb0`의 저장 로그에서 Editor 빌드 `Result: Succeeded`와 전체 Automation Exit Code 0을 최종 확인했다.
- Automation Report에서 `TS-P0-01 RuntimeContract`, `TS-P0-02 TargetPoint` 모두 Success, 전체 2 성공 / 실패 0 / 경고 0 / 오류 0을 확인했다.
- 체크포인트의 이전 Build Job ID와 날짜를 실제 검증 결과로 정정하고 TargetPoint 신규 소스 목록을 추가했다.
- TS-P0-02 Done과 TS-P0-03 후보 탐색·정렬 현재 Task는 유지한다.

### v0.6.0 - 2026-07-24

- `UCFTargetPointComp`, `FCFTargetPointResult`, `ECFTargetPointSource`를 추가했다.
- 차량 기본 `TargetPoint` 서브오브젝트와 Blueprint 위치 조정 경로를 연결했다.
- `TargetPoint → Actor Bounds 중심 → Actor Location` Fallback을 인터페이스와 차량 구현에 공통 적용했다.
- TS-P0-02 자동화 테스트와 TS-P0-01 회귀 테스트, 최종 Editor 빌드를 Exit Code 0으로 완료했다.
- TS-P0-02를 Done으로 전환하고 현재 Task를 TS-P0-03 후보 탐색과 정렬로 변경했다.

### v0.5.0 - 2026-07-23

- Native C++ `ICFTargetSelectable`과 실제 Blueprint 재정의를 구분하는 안전 디스패치를 추가했다.
- `CarFight.TargetSelect.TS_P0_01.RuntimeContract` 자동화 테스트로 입력 거부, 필터, 이벤트 중복 억제, 유효성·추적 상태, DataAsset Fallback과 약한 참조를 검증했다.
- Build Job `19e7501cee914af39d9dcd6b95251d90`의 Editor 빌드와 테스트 Exit Code 0을 기록했다.
- TS-P0-01을 Done으로 전환하고 현재 Task를 TS-P0-02 타겟 포인트 기반으로 변경했다.

### v0.4.0 - 2026-07-23

- `ACFVehiclePawn`에 `TargetSelectComp` 기본 서브오브젝트와 BlueprintPure getter를 추가했다.
- `ACFVehiclePawn`이 `ICFTargetSelectable`을 구현하고 파괴 상태, Vehicle 분류, Unknown 관계, Identified 정보 단계와 Bounds 중심 위치를 제공하도록 연결했다.
- UnrealHeaderTool과 `CarFight_ReEditor Win64 Development` 빌드 PASS를 기록했다.
- Pawn 최소 통합을 PASS로 전환하고 현재 검증 대상을 Blueprint 에셋 노출과 런타임 계약으로 좁혔다.

### v0.3.0 - 2026-07-23

- `TargetSelectInvestigation.md`를 TS-P0-00 공식 산출물로 연결하고 조사 Task를 Done으로 전환했다.
- ACFVehiclePawn 소유, VehicleCameraComp 직접 조준 기준, 기존 입력·HUD·파괴 경로와 TargetSelect 전용 충돌 채널 결정을 반영했다.
- 현재 Task를 TS-P0-01 Blueprint 계약 검증과 Pawn 최소 통합으로 구체화했다.
- 후보 Trace, TargetPoint, 입력과 HUD를 TS-P0-01 제외 범위로 고정했다.

### v0.2.0 - 2026-07-23

- `CF-FQ-026` Active 상태와 현재 작업 체크포인트를 추가했다.
- TS-P0-01 공용 계약 소스와 Editor 빌드 PASS를 기록했다.
- Blueprint 노출과 사용자 PIE Pending 상태를 명시했다.
- 다음 순서를 TS-P0-00 조사 결과 복구 → TS-P0-01 검증 → TS-P0-02로 고정했다.

### v0.1.0 - 2026-07-22

- P0~P3 작업 패키지 정의
- P0 상세 Task와 완료 조건 작성
- Plan 작업 단위 규칙 추가

---

## 10. 마이그레이션

### v0.12.0 적용 안내

- 장비는 `FCFTargetUseRequest`를 구성해 `UCFTargetSelectComp.EvaluateSelectedTargetForUse`를 호출하며 선택 상태를 직접 저장하거나 변경하지 않는다.
- `EquipmentUnavailable` 상태에서도 실제 선택 존재와 유효성 값은 보존되므로 선택 UI와 장비 사용 불가 UI를 독립적으로 표현한다.
- TargetUsePolicy의 허용 배열이 비어 있으면 제한 없음이며 최대 사용 거리가 0 이하이면 거리 제한을 사용하지 않는다.
- 기존 WeaponData는 TargetUsePolicy 기본값이 제한 없음이므로 TS-P0-07 적용을 위해 반드시 재저장할 필요가 없다.
- WeaponComp의 캐시와 변경 이벤트는 UI·유틸리티 피드백용이며 장비 획득 상태는 P2 범위로 유지한다.
- 선택 대상은 직접 조준 무기의 FireOrigin, AimSolution, 터렛 방향과 발사 방향을 자동 변경하지 않는다.
- 다음 TS-P0-08에서는 실제 PIE로 입력, HUD, 장비 평가, 대상 파괴, 화면비와 성능을 통합 확인한다.

### v0.11.0 적용 안내

- `WBP_TargetSelect`는 `UCFTargetSelectWidget`을 부모로 사용하며 Blueprint 이벤트 그래프에서 후보 검색이나 선택 상태를 다시 계산하지 않는다.
- `BP_CFVehiclePawn`은 C++ 기본값으로 TargetSelect HUD 클래스, 표시 토글 True와 ZOrder 20을 상속한다.
- Aim Reticle은 ZOrder 10과 기존 조준·발사 피드백 책임을 유지하며 TargetSelect HUD와 결합하지 않는다.
- 동일 대상의 후보 마커는 선택 마커와 중복 표시하지 않고 화면 밖에서는 상태를 유지한 채 마커만 숨긴다.
- TS-P0-07 장비 연동은 TargetSelectComp의 선택 조회 API만 소비하며 HUD 표현과 직접 조준 발사 방향을 변경하지 않는다.
- 실제 화면비·UI Scale·고속 추적 가독성은 TS-P0-08에서 사용자 PIE로 확인한다.

### v0.10.0 적용 안내

- `BP_CFVehiclePawn`은 C++ 부모의 `InputAction_SelectTarget`, `InputAction_ClearTarget`과 입력 바인딩을 자동 상속한다.
- 선택 기본키는 가운데 마우스와 게임패드 오른쪽 스틱 클릭, 해제 기본키는 오른쪽 마우스와 게임패드 FaceButton Right다.
- 후보가 없는 선택 입력은 기존 선택을 유지하고 Manual 해제는 선택만 비우며 현재 후보를 유지한다.
- TS-P0-06은 입력 Action이나 Pawn 명령을 다시 만들지 않고 `TargetSelectComp`의 후보·선택 이벤트와 표시 정보만 소비한다.
- 사용자 PIE 최종 게이트는 TS-P0-08에서 선택·해제·HUD·재시작·Possess·UI 포커스를 함께 검증한다.

### v0.9.0 적용 안내

- 명시적 Actor 파괴는 `OnDestroyed`, 일반 수명 종료는 `OnEndPlay`, 차량 전투 파괴는 `VehicleHealth.OnVehicleDestroyed`를 사용한다.
- TS-P0-04는 전체 회귀 PASS로 Done이며 후속 Task에서 선택 수명과 가림 유예를 다시 구현하지 않는다.
- TS-P0-05는 `IA_SelectTarget`, `IA_ClearTarget`, `IMC_Vehicle_Default`와 `ACFVehiclePawn::SetupPlayerInputComponent` 연결에 한정한다.
- 선택 입력은 현재 유효 후보가 있을 때만 선택을 변경하고 후보가 없으면 기존 선택을 유지한다.
- 해제 입력은 `ECFTargetClearReason::Manual`로 현재 선택만 해제하며 후보를 자동 선택하지 않는다.

### v0.8.0 적용 안내

- TS-P0-03은 완료됐으며 후보 탐색·정렬을 TS-P0-04 수정 과정에서 다시 구현하지 않는다.
- TS-P0-04의 UBT Editor 컴파일 성공은 기능 완료 증거가 아니다. 최신 통합 Automation이 Exit Code 255이므로 선택 수명은 Active 상태다.
- Actor `Destroy()`와 `OnEndPlay` 사이의 실제 전달 시점을 먼저 확인하고, 런타임 보정이 필요한지 테스트 월드 진행이 필요한지 구분한다.
- 수정 후 `CarFight.TargetSelect` 전체 테스트를 재실행해 TS-P0-01~04가 모두 Success일 때만 TS-P0-04를 Done으로 전환한다.
- 선택 해제 후 현재 후보를 자동 선택하는 동작은 추가하지 않는다.

### v0.7.0 적용 안내

- TS-P0-02의 공식 빌드 증거는 Build Job `c79b3418a3b2405b97ad52df7fa57bb0`, 테스트 증거는 `UE/Saved/Automation/TargetSelect/index.json`이다.
- TS-P0-03은 `UCFTargetPointComp::ResolveTargetPoint` 결과를 소비하며 TargetPoint·Bounds·ActorLocation Fallback을 자체 재구현하지 않는다.
- TS-P0-03 범위에는 후보 수집, 직접 조준 우선, 화면 근접도와 결정적 정렬만 포함하고 입력·HUD·장비 연결을 섞지 않는다.

### v0.5.0 적용 안내

- TS-P0-01 계약과 런타임 상태 전이는 자동화 검증 PASS이며 후속 Task에서 동일 책임을 다시 구현하지 않는다.
- Native C++ 대상은 직접 `_Implementation`, 실제 Blueprint 재정의는 `Execute_`를 사용한다는 디스패치 경계를 유지한다.
- TS-P0-02는 타겟 포인트와 `TargetPoint → Actor Bounds 중심 → Actor 위치` Fallback만 구현한다.
- 사용자 PIE는 입력·검색·HUD가 연결되는 TS-P0-05~08 통합 검증에서 수행한다.

### v0.4.0 적용 안내

- `BP_CFVehiclePawn`은 C++ 부모의 `TargetSelectComp`와 `ICFTargetSelectable` 구현을 자동 상속한다.
- Blueprint에서 TargetSelectComp를 수동 추가하거나 Target Selectable 인터페이스를 중복 등록하지 않는다.
- Pawn 최소 통합은 완료됐지만 Blueprint 노출, 이벤트·필터·Fallback·약한 참조 검증 전에는 TS-P0-01을 Done으로 전환하지 않는다.
- 후보 검색, TargetPoint, 입력과 HUD는 기존 후속 Task 범위를 유지한다.

### v0.3.0 적용 안내

- TS-P0-00은 완료됐으므로 저장소 구조 조사를 처음부터 반복하지 않는다.
- 구현 경로와 책임 배분은 `TargetSelectInvestigation.md`를 기준으로 한다.
- 기존 CFTargetSelect 소스는 유지하고 TS-P0-01 검증 실패가 확인된 부분만 보정한다.
- Pawn 최소 통합은 컴포넌트 생성, getter와 차량 선택 가능 기본 구현까지만 포함한다.
- 입력, 후보 Trace와 Target UI는 각각 TS-P0-05, TS-P0-03, TS-P0-06에 유지한다.

### v0.2.0 적용 안내

- 기존 TS-P0-01 소스를 신규 생성 대상으로 다시 취급하지 않는다.
- 조사 결과가 누락된 부분은 TS-P0-00 산출물 복구로 처리한다.
- TS-P0-01 검증 완료 전에는 후보 탐색, Pawn 연결이나 HUD를 같은 Task에 섞지 않는다.
- 기존 조준 또는 상호작용 대상 판정은 제거부터 하지 않고 새 시스템을 병렬 연결한 후 단계별로 소비 시스템을 이전한다.
