# CarFight — 00_Vision

> 역할: CarFight의 **최종 방향 / 현재 기준선 / 현재 구조 갭 / 전환 원칙**을 한 문서에서 고정한다.
> 문서 버전: v2.4.0
> 마지막 정리(Asia/Seoul): 2026-08-08

---

## 한 줄 정의
CarFight는 **싱글 플레이 기준 1대 차량의 주행, 조준, 발사, 피격, 피해, 피드백이 반복되는 핵심 전투 루프**를 먼저 닫고,
장기적으로는 차량 전투와 `CMVS / Cluster Union / Geometry Collection` 기반 구조로 확장할 수 있게 코어를 유지하는 프로젝트다.

---

## 현재 개발 초점 (2026-06-18)

외부 피드백 반영으로 현재 개발 초점은 아래처럼 전환한다.

```text
이전 초점: Dedicated Server / Multiplayer / 서버 권한 전투 Vertical Slice
현재 초점: 싱글 플레이 / 클라이언트 사이드 / 1대 차량 고도화
```

현재 일정에서는 서버, 멀티플레이, 2클라 검증, 서버 런처, 세션/로비, 이동 복제 고도화를 제외한다.
이 전환으로 개발 일정을 약 2~3개월 단축하는 것을 목표로 한다.

서버/멀티 관련 구현과 문서는 당장 하드 삭제하지 않는다.
이번 전환에서는 코드와 도구를 긁어내지 않고, 프로젝트 진행 기준만 싱글 플레이 선로로 바꾼다.
서버/멀티 문서는 `Deferred` 또는 역사 기록으로 분리하고, 현재 착수 판단에서는 제외한다.

---

## 현재 개발 순서 (2026-06-19)

현재 싱글 실행 기준선과 로컬 Aim / Fire / Reticle 골격은 확인된 기반으로 본다.
다음 개발은 새 시스템을 크게 벌리기보다, 기준 차량 1대에서 전투 입력부터 피해 결과까지 한 바퀴 도는 루프를 만든다.

진행 순서는 아래로 고정한다.

```text
1. 차량 무기 조준 및 발사 기능 구현
2. 발사 시각 이펙트와 조준 UI 구현
3. 피격 판정 및 피해 처리 구현
4. 주행과 전투 흐름 반복 테스트
5. 조작감, 전투 템포, 피드백 개선
6. 핵심 게임 루프 검증
```

이 순서는 멀티플레이 전투 구조가 아니라 **싱글 로컬 전투 루프 검증 순서**다.
서버 권한 발사, PvP 판정, 세션/로비, 보상/경제는 현재 범위에서 제외한다.

---

## 최종 방향
### 1. 차량 전투 중심
- 주행 자체보다 차량 간 충돌, 밀기, 제어 상실, 복구 같은 물리 상호작용이 중요하다.
- 최종적으로는 전투 규칙, 충돌 결과, 카메라, 연출이 차량 상태와 연결되어야 한다.

### 1-1. 전략과 직접 전투 감각
CarFight의 전략적 교전 구성은 `EVE Online`에 가깝고, 직접 전투 조작감은 `Elite Dangerous`에 가깝다.

전략적으로는 단순 에임보다 아래 판단들이 중요해야 한다.

- 거리
- 각도
- 상대 속도와 접근 / 이탈 흐름
- 락온 상태
- 터렛 추적 성능
- 무장 유효 사거리
- 피팅 / 로드아웃
- 에너지 / 열 관리
- 교전 지속 또는 이탈 판단

하지만 실제 전투 조작감은 자동 전투가 아니라 직접 전투여야 한다.

플레이어는 직접 조준하고, 차량을 움직이며 사격 각을 만들고, 고정형 / 짐벌형 / 터렛형 무장의 차이를 체감해야 한다.

따라서 CarFight는 **EVE Online식 전략 교전 판단**과 **Elite Dangerous식 직접 조준 / 무장 운용 감각**을 결합한 차량 전투를 목표로 한다.

### 2. 다차종 대응
- 장기 목표는 여러 차종을 같은 게임 안에 안정적으로 올리는 것이다.
- 현재 사이클에서는 다차종 양산보다 기준 차량 1대의 완성도를 우선한다.
- 차량별 감각 차이는 장기 확장으로 남기되, 지금은 공통 코어를 깨지 않는 선에서 `DA_PoliceCar` 기준을 고도화한다.

### 3. 최종 차량 구조 목표
- 최종 차량 구조 목표는 `CMVS / Cluster Union / Geometry Collection` 기반이다.
- 장기적으로는 부품 조립, 파괴, 분리, 무게 중심 변화까지 구조적으로 열어야 한다.

---

### 4. 멀티플레이·오픈월드 확장성 아키텍처 원칙

현재 개발 일정은 계속 싱글플레이를 우선한다.
아래 원칙은 멀티플레이나 오픈월드를 지금 구현하라는 뜻이 아니라, 앞으로 추가하는 기능이 장기 확장 경로를 불필요하게 닫지 않도록 하는 **프로젝트 전역 설계 제약**이다.

#### 4.1 싱글플레이의 Local은 현재 Authority로 취급한다

현재 싱글 실행에서는 로컬 프로세스가 게임 결과를 결정해도 된다.
다만 새 기능의 상태 변경 경로는 가능하면 아래 책임을 구분한다.

```text
Intent / Request
→ Validation
→ Authoritative State Mutation
→ Result
→ Presentation
```

현재는 위 단계가 같은 프로세스와 같은 Actor 안에서 실행될 수 있다.
멀티플레이를 재개하면 `Authoritative State Mutation` 위치를 서버로 이동할 수 있어야 한다.
이를 위해 지금부터 실제 상태 변경 함수와 UI·FX·입력 함수를 무분별하게 하나로 합치지 않는다.

#### 4.2 데이터는 Static Definition / Runtime State / Persistent State / View Data로 분리한다

```text
Static Definition
= 차량·무기·발사체·방어·아이템의 변하지 않는 규칙과 자산 정의

Runtime State
= 현재 위치, 속도, Shield, Armor, Integrity, Cooldown, 발사 순서처럼 플레이 중 변하는 상태

Persistent State
= Actor가 사라져도 보존해야 하는 소유권, Item Instance, 차량 인스턴스, 피팅, 장기 손상과 월드 상태

View Data
= HUD와 Presentation이 계산을 재구현하지 않고 읽는 표시 전용 상태
```

`UDataAsset / UPrimaryDataAsset`은 기본적으로 Static Definition으로 취급한다.
플레이어별·차량 인스턴스별 변경 상태를 DataAsset 자체에 저장하지 않는다.

#### 4.3 네트워크와 저장의 식별자는 UObject 포인터가 아니라 안정적인 ID를 사용한다

장기 저장, 서버 상태와 네트워크 계약에서 `AActor*`, `UObject*`, DataAsset 포인터 자체를 영구 식별자로 사용하지 않는다.

기본 방향은 아래와 같다.

```text
정적 Definition 식별
→ FPrimaryAssetId 또는 도메인별 안정적 Definition ID

실제 소유 인스턴스 식별
→ FGuid 기반 Instance ID

월드·차량·Container 소유 주체 식별
→ 직렬화 가능한 안정적 Owner / Entity ID
```

현재 `CF-FQ-035`의 `FCFItemInstanceId`, `FCFInventoryOwnerId`, `FCFInventoryContainerId`와 `FPrimaryAssetId` 기반 Inventory Definition 계약은 이 원칙에 맞는 선행 기반으로 본다.

#### 4.4 Snapshot은 확정된 조합 결과이며 DataAsset 조합 전체를 네트워크로 보내는 용도가 아니다

Vehicle Fitting과 같은 조합 시스템은 정적 Definition을 조합해 결정론적인 Snapshot 또는 Record를 만든다.
멀티플레이에서는 클라이언트가 임의의 최종 Snapshot을 권한 결과로 확정하지 않고, 서버가 소유권·호환성·질량·슬롯 같은 규칙을 검증한 뒤 권한 Snapshot을 만든다.

네트워크에서 DataAsset 전체를 복제하는 방향은 사용하지 않는다.
필요한 경우 안정적인 Definition ID와 실제로 변하는 Runtime State만 전달한다.

#### 4.5 VehiclePawn은 차량 Actor 조립자이며 전역 도메인 저장소가 아니다

`ACFVehiclePawn`은 차량 Actor의 컴포넌트 구성, Actor 수명, 입력 연결과 차량 내부 시스템 조율을 담당할 수 있다.
그러나 앞으로 아래 상태를 Pawn에 계속 누적하지 않는다.

```text
- 플레이어 계정·프로필 영구 상태
- 월드 전체 Persistent State
- 전역 인벤토리 저장소
- 세션·매치 상태
- 서버 운영 상태
- 지역·Zone 전역 상태
```

새로운 독립 도메인 규칙은 가능한 한 전용 Component, 순수 Struct/Utility, 적절한 Subsystem 또는 서버/월드 계층으로 분리한다.
Pawn은 각 도메인의 결과를 조립하고 연결하는 역할을 우선한다.

#### 4.6 Actor는 Persistent Entity의 영구 저장 형태가 아니다

오픈월드에서는 Actor가 Streaming, 거리, 서버 Relevancy 또는 레벨 수명 때문에 생성·제거될 수 있다고 가정한다.

장기 방향은 아래와 같다.

```text
Persistent Record / Entity State
→ Spawn 또는 Streaming 진입
→ Runtime Actor 생성
→ Runtime State 적용
→ 플레이
→ 저장 가능한 Snapshot 생성
→ Actor 제거 가능
```

Actor 포인터와 컴포넌트 포인터를 그대로 SaveGame·DB의 영구 상태로 간주하지 않는다.
`World Partition`은 월드 Streaming 도구이며 Persistent State 저장 시스템과 같은 책임으로 취급하지 않는다.

#### 4.7 전투 결과와 소유권 변경은 Authority 경계로 이동 가능해야 한다

장기 멀티플레이에서 다음 결과는 서버 권한 후보로 본다.

```text
- Spawn / Despawn의 권한 결과
- Fitting 확정과 장비 소유권 변경
- Inventory Transfer Commit
- 발사 승인과 Ammo 소비
- Projectile / Hit 결과
- Shield / Armor / Integrity 변경
- 파괴 상태
- 보상과 Persistent World 상태 변경
```

현재 싱글에서는 로컬 코드가 이 역할을 수행해도 되지만, Presentation 계층이 위 결과를 직접 만들어내지 않는다.

#### 4.8 Presentation은 Dedicated Server에서 제거 가능해야 한다

Camera, HUD, Reticle, Debug 화면 표시와 Niagara 같은 Presentation은 게임 결과의 필수 전제조건이 아니다.
Dedicated Server에서는 실행하지 않거나 안전하게 생략할 수 있어야 한다.
현재 코드에 존재하는 `NM_DedicatedServer`, `IsLocallyControlled()` 기반 Presentation Gate는 이 원칙에 맞는 기반으로 유지한다.

#### 4.9 UI는 View Data를 소비하고 게임 규칙을 재계산하지 않는다

HUD는 Vehicle, Weapon, Defense, Target, Radar, Alert 등의 View Data를 읽는 Presentation 계층으로 유지한다.
Shield·Armor·관통·Cooldown·Fitting 호환성 같은 도메인 계산을 Widget에서 다시 구현하지 않는다.
Pawn 교체, 원격 관전 또는 향후 Replicated State 전환 시 Provider 입력만 바꿀 수 있는 구조를 우선한다.

#### 4.10 World와 Pawn 없이 검증 가능한 순수 규칙을 우선한다

소유권, Inventory Transfer, Fitting Snapshot, Capacity, 호환성, 피해 분배처럼 Actor가 없어도 계산할 수 있는 규칙은 가능한 한 순수 Struct, Utility 또는 Adapter로 유지한다.
이는 자동 테스트 비용을 낮추고, 향후 서버·월드 Streaming 수명과 게임 규칙을 분리하는 기준이 된다.

#### 4.11 Replication은 필요해질 때 추가하되 Authority 경계는 지금부터 보존한다

현재 싱글 개발 중에는 다음을 억지로 구현하지 않는다.

```text
- 모든 Component의 Replication 활성화
- 모든 Runtime 변수의 Replicated UPROPERTY 전환
- Server / Client / NetMulticast RPC 선구축
- 이동 Prediction·보간 선구축
- World Partition·DB·SaveGame 선구축
```

멀티플레이 재개 Feature가 승인되면 실제 Gameplay Vertical Slice 하나를 기준으로 필요한 Replication만 추가한다.
그 전까지는 `Request → Validate → Apply → Result` 경계, 안정적 ID, Snapshot, Presentation 분리를 보존하는 것을 최소 비용의 확장성 투자로 본다.

---

## 현재 기준선
현재 실제 구현은 아래 조합을 기준선으로 본다.

- 현재 기준 플레이 차량: `BP_CFVehiclePawn`
- 현재 기준 Native Pawn: `ACFVehiclePawn`
- 현재 기준 주행 코어: `UCFVehicleDriveComp`
- 현재 기준 휠 시각 동기화 코어: `UCFWheelSyncComp`
- 현재 기준 데이터 축: `UCFVehicleData`
- 현재 기준 테스트 자산: `DA_PoliceCar`

현재 차량 구조는 **CMVS가 아니라**
`AWheeledVehiclePawn + ChaosWheeledVehicleMovementComponent` 기반 하이브리드 구조다.

이 기준은 지금 실제로 굴러가는 기준선이며,
최종 방향을 포기했다는 뜻은 아니다.

---

## 현재 구조 갭
### 1. 맞는 방향
- 판단 / 상태 / 규칙을 C++ 코어에 두는 방향
- `BP_CFVehiclePawn`를 Thin BP로 유지하는 방향
- 차량별 차이를 `VehicleData` 축으로 분리하는 방향
- 단일 차량이 아니라 다차종 확장을 염두에 두는 방향

### 2. 아직 반영되지 않은 방향
- `CMVS` 기반 차량 root 구조
- `Cluster Union` 기반 부품 통합
- `Geometry Collection` 기반 파괴 / 분리
- 에디터 조립형 차량 파이프라인
- 전투 물리 구조를 차량 본체 설계에 직접 반영하는 단계

---

## 지금 유지할 것과 나중에 교체할 것
### 유지 예정 코어
- `UCFVehicleData`
- `DriveState` 코어
- `WheelSync` 코어
- `VehicleCamera` 코어
- Local Aim / Reticle 표시 코어
- Thin BP 원칙

### 교체 후보 구조
- Vehicle root 구조
- 차량 조립 방식
- 파괴 / 분리 표현 구조
- 최종 전투 물리 반영을 위한 본체 구성 방식
- 서버 권한 전투 / 멀티플레이 구조는 현재 일정 밖 장기 보류 항목으로 이동

---

## 현재 단계의 의미
지금 단계는 최종 구조에 아직 도달하지 못했더라도,
싱글 플레이에서 기준 차량 1대의 **주행과 전투가 반복되는 최소 게임 루프**를 먼저 닫는 단계다.

정확히는 아래 순서로 본다.
1. 확인된 싱글 실행 / 로컬 Aim / Reticle 기반을 유지한다.
2. 기준 차량에 최소 무기 조준 / 발사 경로를 붙인다.
3. 발사 피드백을 시각 이펙트와 조준 UI로 읽히게 만든다.
4. 피격 판정과 피해 처리를 붙여 전투 결과가 남게 만든다.
5. 주행과 전투를 반복 테스트하며 조작감, 템포, 피드백을 조정한다.
6. 핵심 게임 루프가 재미와 검증 가능성을 동시에 갖췄는지 판정한다.

---

## 프로젝트 전역 사운드 정책

CarFight는 게임 사운드를 지원하지 않는다.

```text
- 엔진음, 타이어음, 충돌음, 무기음, 피격음, 파괴음, UI음과 BGM을 구현 범위에 포함하지 않는다.
- SoundWave, SoundCue, MetaSound Source, Sound Attenuation 자산을 제작·도입하지 않는다.
- USoundBase, UAudioComponent와 사운드 재생 API를 게임 런타임에 추가하지 않는다.
- 사운드를 누락 기능, 품질 개선 후보 또는 장기 확장 후보로 자동 제안하지 않는다.
- 전투 결과 가독성은 Reticle, HUD, Niagara FX, 카메라 반응과 게임패드 진동 같은 비청각 피드백으로 확보한다.
```

이 정책은 `Document/ProjectSSOT/04_ProjectDecisions.md`의 `CF-PDL-0009`를 따른다.

---

## 문서 운영 원칙
### 1. 최종 방향과 현재 기준선을 섞지 않는다
- 최종 방향은 이 문서에서 고정한다.
- 현재 실제 상태는 `01_ProjectState.md`에서 고정한다.

### 2. 임시 편차와 구조 갭을 구분한다
- 예:
  - 입력 장치 문제로 인한 `GamepadOnly`는 임시 편차다.
  - `CMVS 미도입`은 구조 갭이다.

### 3. 테스트 차량을 최종 목표처럼 쓰지 않는다
- `DA_PoliceCar`는 현재 기준선이다.
- 최종 차량 구조의 대체물이 아니다.

---

## 다음 단계에서 열어야 할 주제
- 차량 무기 조준 / 발사 기능을 싱글 로컬 기준으로 구현
- 발사 시각 이펙트와 조준 UI를 플레이어 피드백으로 연결
- 피격 판정과 피해 처리를 최소 전투 결과로 연결
- 주행과 전투가 한 루프로 반복되는지 검증
- `VehicleData / DriveState / WheelSync / VehicleCamera / VehicleAim`을 유지한 채 전투 루프 품질 고도화
- 서버/멀티 관련 문서는 `Deferred` 또는 Archive 기준으로 읽고, 코드/도구는 현상 유지
- 장기적으로 차량 전투 규칙과 파괴 구조를 실제 차량 본체 구조에 연결하는 방법

---

## 이 문서의 사용 방법
- 프로젝트의 방향을 먼저 확인할 때 이 문서를 본다.
- 현재 실제 기준선은 `01_ProjectState.md`를 본다.
- 작업 순서는 `02_Roadmap.md`를 본다.
- 검증 기준은 `05_TestChecklist.md`를 본다.
- 결정 근거는 `VehicleCoreDecisions.md`를 본다.

---

## 변경 이력

### v2.4.0 - 2026-08-08

```text
- 멀티플레이·오픈월드를 즉시 활성화하지 않으면서 향후 확장 비용을 낮추는 프로젝트 전역 아키텍처 원칙을 추가했다.
- Static Definition / Runtime State / Persistent State / View Data의 책임 경계를 고정했다.
- 안정적 Definition ID와 FGuid 기반 Instance / Owner ID를 네트워크·저장 식별 기본 방향으로 확정했다.
- VehiclePawn을 차량 Actor 조립자로 제한하고 플레이어·월드 영구 상태를 계속 누적하지 않는 원칙을 추가했다.
- Actor 수명과 Persistent Entity 수명, World Partition과 Persistence 책임을 분리했다.
- 현재 싱글에서는 Local Authority를 허용하되 Request → Validate → Apply → Result 경계를 보존하도록 했다.
- 모든 Replication·RPC·SaveGame·World Partition을 미리 구현하지 않고 실제 멀티 Feature 승인 시 Vertical Slice 기준으로 추가하도록 제한했다.
```


### v2.3.0 - 2026-07-24

```text
- CarFight 프로젝트 전역에서 게임 사운드를 지원하지 않는 방향을 최종 방향에 반영했다.
- 전투 피드백 개발 순서와 다음 단계에서 사운드 항목을 제거하고 시각 FX와 UI 기준으로 정리했다.
- 사운드를 누락 기능이나 장기 확장 후보로 자동 제안하지 않는 원칙을 추가했다.
```

### v2.2.0 - 2026-06-19

```text
- 현재 개발 초점을 싱글 1대 차량 고도화에서 싱글 로컬 전투 루프 검증으로 한 단계 전진시켰다.
- 차량 무기 조준/발사, 발사 피드백, 피격/피해, 반복 테스트, 템포 개선, 핵심 루프 검증 순서를 추가했다.
- 기존 로컬 Aim/Fire/Reticle 골격은 새 전투 루프 구현의 선행 기반으로 해석하도록 정리했다.
```

### v2.1.1 - 2026-06-19

```text
- 이번 세션의 전환 범위를 코드 삭제가 아니라 프로젝트 진행 선로 전환으로 명확히 했다.
- 서버/멀티 코드와 도구는 현상 유지하고, 현재 착수 판단에서 제외하는 기준으로 정리했다.
```

### v2.1.0 - 2026-06-18

```text
- 프로젝트의 현재 개발 초점을 멀티플레이/서버에서 싱글 플레이 1대 차량 고도화로 전환
- 2~3개월 일정 단축을 위한 서버/멀티/2클라/서버 런처/세션 제외 원칙 추가
- 다차종 대응을 장기 목표로 낮추고 현재 사이클은 DA_PoliceCar 기준 차량 완성도로 재정렬
- 유지 코어에 VehicleCamera와 Local Aim / Reticle 표시를 추가
- 서버 권한 전투 / 멀티플레이 구조를 현재 일정 밖 장기 보류 항목으로 이동
```

---

## Migration

### v2.4.0 적용 안내

```text
- 현재 Roadmap의 싱글플레이 우선순위와 멀티플레이 Deferred 상태는 변경하지 않는다.
- 새 기능 설계 시 Static Definition / Runtime State / Persistent State / View Data 중 어떤 상태를 소유하는지 먼저 구분한다.
- 새 영구 식별자가 필요하면 Actor·UObject 포인터 대신 직렬화 가능한 안정적 ID를 우선한다.
- 새 게임 결과 변경 경로는 가능하면 Request → Validate → Apply → Result 구조로 분리해 향후 Server Authority 이동 지점을 남긴다.
- UI와 FX는 도메인 결과를 소비하며 결과를 권한 있게 생성하지 않는다.
- ACFVehiclePawn에 플레이어·월드 전역 영구 상태를 추가하지 않는다.
- 현재 싱글 기능에 Replication, RPC, SaveGame 또는 World Partition을 단지 미래 대비 목적으로 선구축하지 않는다.
```


### v2.3.0 적용 안내

```text
- 프로젝트 방향과 구현 우선순위에서 게임 사운드를 요구하지 않는다.
- 기존 문서의 과거 사운드 언급은 역사 기록으로만 유지하고 현재 기능 계약으로 사용하지 않는다.
- CF-FQ-024는 Niagara 기반 전투 FX 전용 기능으로 해석한다.
- 전투 가독성 평가는 화면, UI와 시각 연출 기준으로 수행한다.
```
