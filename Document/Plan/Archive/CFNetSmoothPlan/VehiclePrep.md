# CFNetSmooth 차량 적용 전 준비 계획

- 문서 버전: v0.2.0
- 작성일: 2026-06-18
- 대상 프로젝트: CarFight
- 작업 유형: 신규
- 상태: Draft

---

## 1. 목적

이 문서는 `CFNetSmooth`를 실제 차량에 적용하기 전에 차량 쪽 코드를 먼저 정리하는 기준을 정의한다.

핵심 목적은 다음이다.

```text
CFNetSmooth를 차량 Actor 또는 차량 표시 대상에 붙이기 전에 Transform 소유권 충돌을 제거한다.
CFNetSmooth가 실제 차량 Transform 복제 역할을 맡으면 Replicate Movement=false로 전환한다.
실제 차량 물리/판정 Actor와 원격 표시용 Visual/Shell을 분리한다.
CFNetSmooth 플러그인 수정은 이 세션에서 하지 않는다.
CFNetSmooth 자체는 ReleaseCandidate.md 기준 테스트 Actor v1.0 후보로 보고 반복 검증을 다시 시작하지 않는다.
CFNetSmooth 0.12.0부터는 Owner를 움직이지 않고 보간 Transform을 읽는 API를 사용할 수 있다.
CarFight 쪽에는 이 API를 실제 차량 Actor가 아니라 원격 Visual/Shell SceneComponent에만 적용하기 위한 브리지 컴포넌트를 둔다.
```

---

## 2. 현재 기준선과 적용 목표

CFNetSmooth를 아직 차량에 붙이지 않은 현재 롤백/엔진 기준선은 아래 상태로 둔다.

```text
Replicate Movement=true
VehicleMesh Replicate Physics To Autonomous Proxy=true
Physics Replication Mode=Default
Tick Physics Async=false
Physics Prediction=false
```

CFNetSmooth가 실제 차량 Transform 복제 역할을 맡는 적용 모드는 아래 상태를 목표로 한다.

```text
Actor Replicates=true
Replicate Movement=false
CFNetSmooth가 서버 권위 Transform State 송수신과 원격 표시 보간을 담당
AutoPhysics 정책은 별도 테스트로 확정
```

주의:

```text
RepMove=true는 CFNetSmooth 연결 전 기준선 또는 롤백 기준선이다.
RepMove=false는 CFNetSmooth가 실제로 Transform 복제 역할을 대신할 때의 적용 기준이다.
CFNetSmooth 없이 RepMove=false만 켜는 것은 과거처럼 서버/클라 싱크 붕괴를 만들 수 있다.
```

현재까지 확인한 현상은 아래처럼 정리한다.

```text
RepMove=true:
- 서버/클라이언트 싱크는 상대적으로 유지된다.
- 로컬 조작 차량에서 회전 계열 jitter가 발생한다.
- 특히 과속방지턱 등으로 차체가 뜰 때 Pitch/Roll/Yaw 흔들림 체감이 커진다.

RepMove=false:
- 로컬 jitter는 줄어든다.
- 서버 권위 위치와 클라이언트 위치 싱크가 맞지 않는다.

AutoPhysics=false:
- 일부 jitter 수치는 줄었다.
- 서버 권위 싱크가 크게 무너져 최종 기준선에서 제외했다.

Physics Resimulation 실험:
- 입력 또는 원격 이동 경로가 끊기는 문제가 있었다.
- 현재 기준선에서는 제외했다.

CFNetSmooth:
- 테스트 Actor 기준 송수신과 패키지 검증은 통과했다.
- 실제 차량 Pawn에는 아직 적용하지 않는다.
- 실제 차량 적용 시에는 Replicate Movement=false로 전환해야 한다.
- CarFight 적용 준비용 `UCFVehicleSmoothVisComp`는 추가됐지만, 아직 `ACFVehiclePawn`에 자동 부착하지 않는다.
```

---

## 3. 범위

### 3.1 포함 범위

이 문서는 아래 작업을 포함한다.

```text
ACFVehiclePawn 차량 Actor Transform 소유권 정리
BP_CFVehiclePawn 기본값과 컴포넌트 책임 확인
원격 표시용 Visual/Shell 적용 전제 정리
차량 쪽 디버그 로그 기본 비활성화 기준 정리
CFNetSmooth 적용 전 검증 절차 정리
CFNetSmooth 제작 세션으로 넘길 프롬프트 템플릿 정리
```

### 3.2 제외 범위

이 문서는 아래 작업을 제외한다.

```text
UE/Plugins/CFNetSmooth 플러그인 소스 수정
CFNetSmooth API 변경
CFNetSmooth TestActor 수정
외부 Smooth Sync 플러그인 도입
CFNetSmooth 없이 RepMove=false만 단독 재시도
AutoPhysics=false 재시도
Physics Resimulation 재시도
실제 주행감이 바뀌는 엔진 토크, 조향각, 마찰, 서스펜션 튜닝
```

---

## 4. 핵심 결정

### 4.1 CFNetSmooth 연결 전 실제 차량 Actor는 엔진 기본 경로가 소유한다

CFNetSmooth를 아직 차량에 붙이지 않은 기준선에서는 실제 차량 Actor의 위치와 회전을 아래 경로가 책임진다.

```text
Chaos Vehicle physics
UE Actor Movement Replication
VehicleMesh physics replication
```

이 상태에서는 실제 차량 Actor에 `CFNetSmooth`가 수신 State를 직접 적용하면 안 된다.

CFNetSmooth가 실제 차량 Transform 복제 역할을 맡는 적용 단계에서는 아래처럼 소유권을 바꾼다.

```text
Chaos Vehicle physics:
- 서버 권위 물리/충돌 기준 유지

UE Actor Movement Replication:
- Replicate Movement=false로 비활성화

CFNetSmooth:
- 서버 권위 Transform State 송수신
- 원격 클라이언트 표시 보간
```

금지 예시는 다음이다.

```text
RepMove=true 상태에서 BP_CFVehiclePawn에 CFNetSmooth를 붙이고 같은 Transform을 동시에 적용하기
ACFVehiclePawn 본체에서 엔진 RepMove와 CFNetSmooth 수신값으로 동시에 SetActorLocationAndRotation 호출하기
VehicleMesh 물리 Transform을 CFNetSmooth 보간 결과로 직접 덮어쓰기
로컬 조작 차량의 물리 Root를 시각 보간용으로 움직이기
```

### 4.2 CFNetSmooth는 표시용 대상에만 적용한다

CFNetSmooth 적용 대상은 아래 조건을 만족해야 한다.

```text
서버 충돌 판정에 참여하지 않는다.
Chaos Vehicle Movement가 직접 움직이지 않는다.
Actor Movement Replication과 같은 Transform을 동시에 소유하지 않는다.
로컬 조작 입력을 처리하지 않는다.
원격 차량 표시만 담당한다.
필요하면 즉시 숨기거나 제거할 수 있다.
```

현재 우선 후보는 `별도 Visual/Shell Actor`이다.

다만 `CFNetSmooth가 Replicate Movement를 대신한다`는 최종 적용 모드에서는 RepMove=false가 필수다.
이 경우에도 원격 표시 보간 대상과 서버 권위 충돌/판정 기준은 분리해서 검증한다.

### 4.3 차량 쪽 사전 정리가 먼저다

CFNetSmooth 적용보다 먼저 해야 할 일은 다음이다.

```text
1. 기존 실패 실험 코드가 기본 ON 상태로 남아 있지 않은지 확인한다.
2. 차량 Actor Transform을 수동 보정하는 경로를 기본 OFF 상태로 고정한다.
3. 차량 디버그 로그를 테스트 때만 켤 수 있게 정리한다.
4. 원격 표시 대상과 실제 물리 Actor의 책임을 문서와 코드 이름으로 분리한다.
5. CFNetSmooth가 붙을 대상은 실제 차량 Actor가 아니라 Visual/Shell임을 검증한다.
```

---

## 5. Transform 책임 지도

| 대상 | 책임 | Transform 소유자 | CFNetSmooth 적용 |
|---|---|---|---|
| 실제 차량 Actor | 서버 권위 위치, 충돌, 게임플레이 판정 | Chaos Vehicle / UE RepMove | 금지 |
| VehicleMesh | 차량 물리와 본체 시뮬레이션 | Chaos Vehicle / Physics Replication | 금지 |
| Wheel Component | 바퀴 물리/표시 연결 | Chaos Vehicle Wheel 경로 | 금지 |
| 로컬 카메라 | 플레이어 시야 안정화 | 카메라 컴포넌트 | 직접 적용 금지 |
| 원격 Visual/Shell | 다른 클라이언트에서 보는 표시 | CFNetSmooth 또는 별도 표시 보간 | 허용 후보 |
| 디버그 로그 | 원인 분석 | 진단 코드 | 테스트 때만 ON |

---

## 6. 작업 순서

### 6.1 1단계: 차량 기준선 동결

목표는 CFNetSmooth 적용 전 현재 기준선을 다시 흔들지 않는 것이다.

진입 전 확인:

```text
ReleaseCandidate.md를 읽고 CFNetSmooth 자체 반복 검증이 종료된 상태인지 확인한다.
차량 적용 세션에서는 새 문제가 발견되지 않는 한 CFNetSmooth Dedicated 반복 검증을 다시 시작하지 않는다.
```

완료 조건:

```text
Tools\BuildEditor.bat 성공
Tools\SetVehicleNetBpDefaults.bat 성공
RepMove=true 확인
AutoPhysics=true 확인
PhysicsReplicationMode=Default 확인
차량 입력, 전진, 후진, 조향이 기존 기준선과 동일하게 동작
```

실패 조건:

```text
전진/후진 입력이 끊긴다.
클라이언트 하나에서 다른 클라이언트 차량이 아예 움직이지 않는다.
RepMove=false처럼 위치 싱크가 크게 벌어진다.
테스트하지 않은 로그가 과도하게 출력된다.
```

주의:

```text
Tools\SetVehicleNetBpDefaults.bat는 CFNetSmooth 적용 모드가 아니라 롤백/엔진 기준선 복구용이다.
CFNetSmooth 실제 차량 적용 패치에서는 별도 절차로 RepMove=false를 저장해야 한다.
```

### 6.2 2단계: 기존 실험 장치 기본 OFF 확인

목표는 실패한 jitter 실험이 CFNetSmooth 적용 결과를 오염하지 않게 하는 것이다.

확인 대상:

```text
OwnerVisualRoot 기반 안정화 기본 OFF
OwnerBodyVisual 기반 안정화 기본 OFF 또는 명시적 실험값 분리
SoftReconcile류 Actor Transform 보정 기본 OFF
VehicleCorrectionDebug류 로그 기본 OFF
VehicleChaosInputDebug류 로그 기본 OFF
VehicleRepMoveReceiveDebug류 로그 기본 OFF
```

원칙:

```text
진단 코드는 남겨둘 수 있다.
하지만 정상 주행 테스트 기본값에서는 로그와 Transform 보정이 꺼져 있어야 한다.
로그 코드와 실제 Actor 적용 코드는 분리되어 있어야 한다.
```

### 6.3 3단계: Visual/Shell 적용 지점 설계

목표는 실제 차량 Actor를 건드리지 않고 원격 표시만 부드럽게 할 위치를 정하는 것이다.

우선 후보:

```text
BP 또는 C++ 기반 별도 원격 표시 Actor
충돌 없음
물리 없음
입력 없음
서버 판정 없음
원격 클라이언트에서만 표시
```

보류 후보:

```text
BP_CFVehiclePawn 내부 실제 VehicleMesh 직접 보간
ACFVehiclePawn Actor Transform 직접 보간
Chaos Wheel Component 직접 보간
```

### 6.4 4단계: CFNetSmooth 연결 전 안전장치 추가

목표는 실수로 실제 차량 Actor에 CFNetSmooth가 적용되는 것을 막는 것이다.

차량 적용 세션에서 필요한 안전장치 후보:

```text
원격 Visual/Shell 전용 기능 플래그 추가
실제 차량 Actor에는 CFNetSmooth 수신 적용 금지
Owner 차량에는 Visual/Shell 적용 금지
Visual/Shell 충돌과 물리 비활성화 확인 로그 1회 출력
테스트 종료 후 대량 로그가 남지 않도록 기본 로그 OFF
```

이 단계에서도 CFNetSmooth 플러그인 소스는 수정하지 않는다.

### 6.5 5단계: CFNetSmooth 연결 테스트

목표는 CFNetSmooth가 실제 차량 물리와 충돌하지 않고 원격 표시만 바꾸는지 확인하는 것이다.

테스트 순서:

```text
Test A: 기준선 테스트
- CFNetSmooth 차량 연결 없음
- RepMove=true
- AutoPhysics=true
- 기존 jitter와 싱크 상태를 재확인

Test B: 빈 Visual/Shell 생성 테스트
- Visual/Shell은 생성하되 CFNetSmooth 연결 없음
- 실제 차량 입력과 싱크가 변하지 않아야 한다.

Test C: Visual/Shell에만 CFNetSmooth 연결
- CFNetSmooth가 Transform 복제를 맡는 테스트라면 RepMove=false로 전환
- 실제 차량 Actor에는 CFNetSmooth 적용 금지
- 원격 표시 Shell만 수신 State를 따라가야 한다.

Test D: 원격 차량 표시 전환 테스트
- 원격 클라이언트에서 실제 Mesh 표시와 Shell 표시를 비교한다.
- 충돌 판정과 서버 위치는 실제 차량 Actor 기준으로 유지한다.
```

---

## 7. 검증 체크리스트

CFNetSmooth 적용 전 차량 쪽 준비 완료 기준은 다음이다.

```text
[ ] CFNetSmooth 연결 전 차량 기준선이 Default + RepMove=true + AutoPhysics=true로 복구되어 있다.
[ ] CFNetSmooth가 실제 Transform 복제를 맡는 적용 단계에서는 RepMove=false 전환 절차가 별도로 준비되어 있다.
[ ] 실패한 실험 기능이 기본 ON 상태로 남아 있지 않다.
[ ] 대량 디버그 로그가 기본 OFF이다.
[ ] 실제 차량 Actor Transform을 수동 보정하는 새 경로가 없다.
[ ] Visual/Shell 적용 대상이 실제 물리 Actor와 분리되어 있다.
[ ] CFNetSmooth를 실제 차량 Actor에 직접 붙이지 않는 규칙이 문서화되어 있다.
[ ] CFNetSmooth 플러그인 수정 필요 시 별도 제작 세션으로 넘기는 프롬프트가 준비되어 있다.
[ ] CarFight 쪽 Visual/Shell 브리지 컴포넌트는 기본 OFF이며, 실제 차량 Pawn에 자동 부착되지 않는다.
```

---

## 8. CarFight 브리지 컴포넌트

### 8.1 추가된 코드

파일:

```text
UE/Source/CarFight_Re/Public/CFVehicleSmoothVisComp.h
UE/Source/CarFight_Re/Private/CFVehicleSmoothVisComp.cpp
UE/Source/CarFight_Re/CarFight_Re.Build.cs
```

클래스:

```text
UCFVehicleSmoothVisComp
```

역할:

```text
CFNetSmooth의 TryGetSmoothedTransform 결과를 읽는다.
실제 차량 Actor Transform은 변경하지 않는다.
지정된 원격 Visual/Shell SceneComponent에만 월드 Transform을 적용한다.
기본값은 bEnableVehicleSmoothVisual=false라 현재 차량 기준선에 영향을 주지 않는다.
기본적으로 Simulated Proxy에서만 적용한다.
RootComponent와 물리 시뮬레이션 컴포넌트에는 적용하지 않는다.
```

### 8.2 현재 적용 상태

현재 상태:

```text
코드는 추가됨
BuildEditor 성공
ACFVehiclePawn 생성자에는 아직 자동 추가하지 않음
BP_CFVehiclePawn에도 아직 연결하지 않음
Replicate Movement 값은 아직 변경하지 않음
```

판정:

- 이 단계는 차량 적용 준비 단계다.
- 실제 차량 적용은 다음 단계에서 Visual/Shell 대상과 CFNetSmooth Source를 명시적으로 연결한 뒤 진행한다.
- 이 컴포넌트는 NetSmoothSync 플러그인 Runtime 코드를 CarFight 전용 코드로 오염시키지 않기 위한 호스트 프로젝트 어댑터다.

### 8.3 다음 연결 방식 후보

안전한 기본안:

```text
1. ACFVehiclePawn 또는 얇은 BP에 CFNetSmooth 컴포넌트를 추가한다.
2. CFNetSmooth의 Apply Received State To Owner는 false로 유지한다.
3. 실제 Root/VehicleMesh가 아닌 별도 Visual/Shell SceneComponent를 만든다.
4. UCFVehicleSmoothVisComp에 NetSmooth Source와 Visual Target을 지정한다.
5. bEnableVehicleSmoothVisual=true는 테스트 맵/테스트 BP에서만 켠다.
6. CFNetSmooth가 실제 Transform 복제를 맡는 적용 단계에서만 Replicate Movement=false 전환을 별도 패치로 수행한다.
```

보류:

```text
ACFVehiclePawn 본체 Transform 직접 보간
VehicleMesh 직접 보간
물리 시뮬레이션 중인 Component 보간
로컬 조작 Pawn에 Visual/Shell 보간 적용
```

---

## 9. 롤백 기준

아래 현상이 발생하면 CFNetSmooth 차량 연결을 즉시 끄고 기준선으로 돌아간다.

```text
로컬 조작 입력이 먹지 않는다.
클라이언트 하나에서 다른 클라이언트 차량이 움직이지 않는다.
RepMove=false 때처럼 위치 싱크가 크게 벌어진다.
충돌 판정 위치와 보이는 차량 위치가 게임플레이에 영향을 줄 정도로 어긋난다.
Visual/Shell이 실제 차량 물리 Actor를 끌고 간다.
로그량 때문에 테스트 자체가 느려진다.
```

롤백 순서는 다음이다.

```text
1. 차량 Visual/Shell 기능 플래그를 끈다.
2. CFNetSmooth를 차량 연결 경로에서 분리한다.
3. Tools\SetVehicleNetBpDefaults.bat를 실행해 RepMove=true 엔진 기준선으로 되돌린다.
4. Tools\SetVehicleAutoPhysics.bat true를 실행한다.
5. Tools\SetVehicleResimMode.bat off를 실행한다.
6. Tools\BuildEditor.bat를 실행한다.
7. RepMove=true + AutoPhysics=true 기준선 테스트를 다시 한다.
```

---

## 10. CFNetSmooth 제작 세션 전달 기준

차량 적용 중 아래 조건이 확인되면 현재 차량 적용 세션에서 플러그인을 수정하지 않는다.

```text
CFNetSmooth가 Owner Actor가 아니라 특정 SceneComponent를 플러그인 내부에서 직접 움직여야 한다.
TryGetSmoothedTransform 기반 외부 적용만으로 Visual/Shell 연결이 부족하다.
로컬 Owner 또는 AutonomousProxy 제외 규칙이 플러그인 내부에 필요하다.
로그를 더 세밀하게 제어할 카테고리/Verbosity/API가 필요하다.
Buffer 상태, 최신 수신 시간, 외삽 여부 같은 진단값이 public getter로 필요하다.
Teleport 처리 정책이 차량 리스폰 흐름과 맞지 않는다.
```

이 경우 아래 프롬프트를 CFNetSmooth 제작 세션에 전달한다.

```text
CarFight 프로젝트의 CFNetSmooth 제작 세션에서 이어서 작업해줘.

현재 차량 적용 세션의 규칙:
- UE/Plugins/CFNetSmooth 플러그인은 차량 적용 세션에서 수정하지 않는다.
- 실제 차량 Actor에는 CFNetSmooth 수신 Transform을 직접 적용하지 않는다.
- CFNetSmooth 연결 전 차량 기준선은 Default + RepMove=true + AutoPhysics=true이다.
- CFNetSmooth가 실제 차량 Transform 복제 역할을 맡는 적용 단계에서는 Replicate Movement=false로 전환해야 한다.
- CFNetSmooth는 원격 Visual/Shell 표시용으로만 사용할 계획이다.
- CFNetSmooth 0.12.0부터 TryGetSmoothedTransform으로 Owner 직접 적용 없이 보간 Transform을 읽을 수 있다.

필요한 플러그인 검토/수정 요청:
1. TryGetSmoothedTransform 기반 외부 적용만으로 부족한지 검토해줘.
2. 특정 SceneComponent 직접 적용 API 또는 delegate가 추가로 필요한지 검토해줘.
3. bApplyReceivedStateToOwner=false를 유지한 상태에서도 차량 적용 세션이 Visual/Shell에 Transform을 적용할 수 있는 구조를 유지해줘.
4. 디버그 로그는 기본 OFF를 유지하고, 차량 테스트 때 필요한 최소 카운터/상태만 얻을 수 있는 방법을 제안해줘.
5. 플러그인 수정이 필요하다면 CFNetSmooth 플러그인 안에서만 수정하고, 차량 Pawn/Blueprint/VehicleNetSyncPlan은 건드리지 말아줘.

검증 기준:
- Tools\BuildEditor.bat 성공
- Tools\RunCFNetSmoothDedicated.bat 10 1 성공
- 기존 CFNetSmooth TestActor 송수신 동작 유지
- 실제 차량 Actor Transform을 직접 변경하는 API 사용을 강제하지 않음

수정 후에는 변경 파일, 버전, changelog, migration, 테스트 결과를 문서화해줘.
```

---

## 11. Changelog

### v0.2.0

```text
- CarFight 차량 적용 준비용 UCFVehicleSmoothVisComp 추가 상태 반영
- CFNetSmooth TryGetSmoothedTransform 결과를 실제 차량 Actor가 아니라 Visual/Shell SceneComponent에만 적용하는 브리지 기준 추가
- 현재는 코드만 추가되고 ACFVehiclePawn/BP_CFVehiclePawn에는 자동 연결하지 않았음을 명시
```

### v0.1.3

```text
- CFNetSmooth 0.12.0의 TryGetSmoothedTransform 읽기 전용 API 사용 가능 상태 반영
- 플러그인 제작 세션 전달 기준을 public getter 필요 여부에서 SceneComponent 직접 적용/delegate 추가 필요 여부로 조정
```

### v0.1.2

```text
- 차량 적용 세션 진입 전 ReleaseCandidate.md를 먼저 읽는 기준 추가
- CFNetSmooth 자체 반복 검증은 종료 상태로 보고 차량 기준선 동결부터 진행하도록 명시
```

### v0.1.1

```text
- CFNetSmooth 실제 차량 적용 시 Replicate Movement=false가 필수임을 명시
- RepMove=true는 CFNetSmooth 연결 전 기준선/롤백 기준선으로 분리
- Tools\SetVehicleNetBpDefaults.bat가 CFNetSmooth 적용 모드가 아니라 엔진 기준선 복구용임을 명시
```

### v0.1.0

```text
- CFNetSmooth 차량 적용 전 준비 계획 최초 작성
- 차량 기준선, Transform 책임 지도, 작업 순서, 검증 체크리스트, 롤백 기준 추가
- CFNetSmooth 제작 세션으로 넘길 플러그인 수정 요청 프롬프트 추가
```
