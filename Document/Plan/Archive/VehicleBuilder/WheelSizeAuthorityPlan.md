# CarFight Wheel Size Authority Plan

- 문서 버전: v0.1.19
- 작성일: 2026-09-01
- 문서 상태: WSA-P0-07 USER PASS / Wheel Size Authority P0 Complete / Historical Evidence Owner
- Feature: `CF-FQ-040 Guided Vehicle Builder`
- 상위 계획: `VehicleBuilderPlan.md`
- 적용 구간: `VB-P0-09 End-to-End USER Acceptance` 선행 교정
- 작업 ID Prefix: `WSA`
- 대상 엔진: Unreal Engine 5.8 Source Build
- 역할: 공용 Wheel StaticMesh, USER-authored Wheel Socket Scale, VehicleData의 시각/물리 휠 크기 사이의 단일 Authority와 파생 규칙을 정의한다.

---

## 1. 배경

현재 CarFight는 차량별로 네 개의 Wheel StaticMesh 슬롯을 가지고 있고, VehicleMovementConfig에는 물리 WheelRadius/Width가 별도로 존재한다.

기존에는 차량마다 Wheel Mesh를 복사하거나 수동 크기 조절을 할 수 있었고, 추가로 `WheelVisualConfig.bAutoScaleWheelMeshToRadius`를 켜면 물리 `FrontWheelRadius / RearWheelRadius`에서 시각 Wheel Mesh Scale을 자동 계산하는 기능도 존재한다.

이 방식은 다음 문제가 있다.

```text
1. 차종마다 타이어 크기가 조금 다를 때 동일 Wheel Mesh 복사본이 늘어난다.
2. 휠하우스에 어떤 타이어 크기가 시각적으로 적절한지는 단순 수치 계산으로 결정할 수 없다.
3. AI/코드가 휠하우스를 자동 판단해 타이어 크기를 결정하면 USER의 시각적 의도와 어긋날 수 있다.
4. 시각 Wheel 크기와 Chaos 물리 WheelRadius/Width를 별도로 조정하면 타이어가 지면에 파묻히거나 떠 보일 수 있다.
5. 기존 AutoScale은 물리값 → 시각값 방향이라, USER가 먼저 시각적으로 크기를 결정한다는 새 제작 원칙과 Authority가 반대다.
```

따라서 이 Plan은 **타이어 크기를 자동 디자인하지 않고, USER가 Wheel Socket Scale로 결정한 시각 크기를 시각/물리 양쪽의 단일 제작 Authority로 사용**하는 구조를 정의한다.

---

## 2. 핵심 결정

### 2.1 타이어 크기 디자인 Authority

최종 타이어 크기를 결정하는 주체는 USER다.

```text
차량 휠하우스 확인
→ USER가 Wheel_Anchor_* Socket Location / Rotation / Scale 조정
→ Socket Scale이 해당 차량의 타이어 크기 authoring truth
```

AI와 코드는 휠하우스를 보고 적정 타이어 크기를 자동 결정하지 않는다.

### 2.2 StaticMesh Bounds의 역할

Wheel StaticMesh Bounds는 **디자인 Authority가 아니라 기계적으로 읽을 수 있는 원본 치수 Source**다.

```text
StaticMesh Bounds
= 공용 타이어 메시 자체의 원본 실제 크기

Wheel Socket Scale
= 그 차량에서 USER가 선택한 크기 배율
```

코드는 Bounds와 USER Scale을 곱해 최종 치수를 계산할 수 있지만, Scale 값을 스스로 선택하지 않는다.

### 2.3 최종 Authority 흐름

```text
[공용 Wheel StaticMesh Bounds]
              +
[USER Wheel Socket Scale]
              ↓
       최종 Wheel Size
        ┌─────┴─────┐
        ↓           ↓
  Visual Wheel   Physics Wheel
  Mesh Scale     Radius / Width
```

시각과 물리는 같은 입력에서 파생한다.

---

## 3. 공용 Wheel StaticMesh 제작 규격

### 3.1 축 규칙

CarFight 공용 Wheel Mesh는 다음 축 규칙을 사용한다.

```text
X = 타이어 직경 방향
Y = 차축 방향 / 타이어 폭 방향
Z = 타이어 직경 방향
```

### 3.2 Canonical radial size

공용 Default Wheel Mesh의 로컬 Bounds 기준 X/Z 전체 크기는 **100 cm**를 canonical target으로 한다.

```text
Approx Size / Bounds Size

X = 100 cm
Z = 100 cm
```

따라서 Socket X/Z Scale은 사람이 바로 직경으로 읽기 쉽다.

```text
Scale 0.65 → 직경 약 65 cm
Scale 0.70 → 직경 약 70 cm
Scale 0.75 → 직경 약 75 cm
Scale 0.80 → 직경 약 80 cm
Scale 0.90 → 직경 약 90 cm
```

제작 원본은 가능한 한 정확히 100 cm로 맞춘다. Import/부동소수 오차를 고려한 Validator 허용 오차는 구현 단계에서 작은 tolerance로 둔다.

### 3.3 Canonical base width = 25 cm

USER 결정으로 공용 Default Wheel Mesh의 Y base width도 **25 cm**로 고정한다.

최종 canonical Bounds target:

```text
X = 100 cm
Y = 25 cm
Z = 100 cm
```

의미:

```text
Socket Scale.X/Z = 최종 직경 cm / 100
Socket Scale.Y   = 최종 폭 cm / 25
```

예:

```text
Socket Scale = 0.70 / 1.00 / 0.70
→ 직경 70 cm
→ 반지름 35 cm
→ 폭 25 cm
```

Y를 100 cm로 만들지는 않는다. 25 cm를 canonical width unit으로 사용해 일반 승용차 기준에서 사람이 읽기 쉬운 Scale 범위를 유지한다.

### 3.4 현재 공용 Wheel evidence와 one-time normalization

VB-P0-09 Wagon 후보가 현재 최소 Wheel로 참조하는 persisted asset:

```text
/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL.Wheel_FL
```

기존 fresh persisted evidence:

```text
Bounds min = (-39.679, -17.740, -39.679)
Bounds max = ( 39.679,  17.740,  39.679)

Approx/Bounds Size
X ≈ 79.358 cm
Y ≈ 35.480 cm
Z ≈ 79.358 cm
```

즉 기존 공용 Wheel은 새 canonical `100 × 25 × 100 cm` 규격을 만족하지 않았다.

2026-08-28 USER가 실제 shared `Wheel_FL`로 교체/이동한 뒤 live read-only `StaticMeshTools.get_bounds`로 다시 확인했다.

```text
min = (-49.9995079, -12.4996042, -49.9995079)
max = ( 49.9995079,  12.4996042,  49.9995079)

size
X = 99.9990158 cm
Y = 24.9992085 cm
Z = 99.9990158 cm

center ≈ (0, 0, 0)
```

fresh Git status에서도 다음 실제 shared asset이 modified로 관측됐다.

```text
UE/Content/CarFight/Vehicles/Shared/Tire/Wheel_FL.uasset
```

판정:

```text
Canonical Bounds 100 × 25 × 100 = PASS
Bounds Center / Origin sanity = PASS
```

부동소수/mesh build 오차는 0.001 cm 미만으로 canonical tolerance에 충분히 들어온다.

잘못 작업했던 별도 `UE/Content/CarFight/Vehicles/Meshes/Wagon/Wheel_FL.uasset` untracked asset은 이 Plan의 canonical owner가 아니며 자동 정리/삭제하지 않는다.

따라서 Wagon의 실제 Wheel Socket Scale USER authoring은 이제 canonical source dimensions 관점에서는 진행 가능하다. 코드/Resolver WSA 구현 Gate는 별도로 남아 있다.

### 3.5 Socket mode에 참여하는 모든 Wheel Mesh의 canonical 규격

`SocketScaleFromChassis`에서 Scale 숫자가 항상 같은 의미를 가지게 하려면 Default Wheel뿐 아니라 **이 모드에서 실제로 선택되는 모든 Wheel Mesh**가 같은 canonical source dimensions를 가져야 한다.

```text
Required Bounds target
X ≈ 100 cm
Y ≈ 25 cm
Z ≈ 100 cm
```

따라서 향후 다른 디자인의 Front/Rear/개별 Wheel Mesh를 사용하더라도:

```text
canonical 100×25×100 규격 PASS → SocketScaleFromChassis 사용 가능
non-canonical mesh → Block / 먼저 normalize / 또는 Legacy mode 사용
```

으로 처리한다.

이 규칙이 없으면 같은 `Scale 0.70 / 1.00 / 0.70`이 Wheel Mesh Asset마다 다른 실제 크기를 뜻하게 되어 USER-facing 단위 의미가 무너진다.

### 3.6 Origin / Bounds center

공용 Wheel Mesh의 원점은 실제 휠 회전 중심에 둔다.

정상 조건:

```text
Bounds Center ≈ Mesh Origin
Wheel rotation center = Mesh Origin
Axle Axis = Y
```

새 Socket Size 경로는 잘못된 Pivot을 자동 Center Fix로 숨기는 것을 기본 정책으로 삼지 않는다. 공용 Wheel Mesh 자체를 한 번 정상화하는 것을 우선한다.

---

## 4. Wheel Socket Scale 규칙

Wheel Socket은 기존 이름을 유지한다.

```text
Wheel_Anchor_FL
Wheel_Anchor_FR
Wheel_Anchor_RL
Wheel_Anchor_RR
```

Scale 축 의미:

```text
Scale.X = 타이어 직경 배율
Scale.Y = 타이어 폭 배율
Scale.Z = 타이어 직경 배율
```

일반 타이어는 다음을 원칙으로 한다.

```text
Scale.X == Scale.Z
```

예:

```text
1.10 / 1.20 / 1.10 = 정상
1.10 / 1.20 / 0.90 = 비정상
```

X/Z 불일치는 타이어를 타원형으로 왜곡하므로 Builder/Validator blocker로 취급한다.

모든 Scale 성분은 finite이며 0보다 커야 한다.

---

## 5. 데이터 모델 설계

### 5.1 FCFWheelAnchorPose

현재:

```text
RelativeLocation
RelativeRotation
```

계획:

```text
RelativeLocation
RelativeRotation
RelativeScale
```

권장 필드:

```cpp
FVector RelativeScale = FVector::OneVector;
```

의미:

- `RelativeLocation`: Wheel_Anchor_* 위치
- `RelativeRotation`: Wheel_Anchor_* 회전
- `RelativeScale`: USER가 Chassis StaticMesh Socket에 작성한 Wheel Size Scale

기존 Asset 호환을 위해 기본값은 `FVector::OneVector`다.

### 5.2 WheelVisualConfig 명시 모드

기존 Asset 동작을 말 없이 변경하지 않기 위해 Runtime과 Authoring 양쪽에 명시 모드를 둔다.

Runtime `FCFVehicleWheelVisualConfig` 권장 필드:

```text
bUseWheelSocketScale
```

기본값:

```text
false
```

의미:

```text
false
= 기존 수동/Legacy AutoScale 경로 유지

true
= Wheel Socket Scale이 시각/물리 Wheel Size authoring authority
```

Blueprint/Details 한국어 Tooltip은 다음 의미를 명확히 전달해야 한다.

> True이면 차체 Wheel Socket의 Scale을 차량별 타이어 크기 기준으로 사용합니다. USER가 정한 Scale을 시각 Wheel Mesh에 적용하고 같은 크기에서 물리 WheelRadius/Width를 파생합니다. WheelRadius 기준 시각 자동 스케일과 동시에 사용하지 않습니다.

Editor Authoring의 기존 semantic owner `ECFWheelVisualIntentMode`에는 다음 mode를 추가한다.

```text
SocketScaleFromChassis
DisplayName = 차체 소켓 스케일 사용
```

Resolver mapping:

```text
ManualMeshScale
→ bUseWheelSocketScale=false
→ bAutoScaleWheelMeshToRadius=false

AutoScaleToPhysicsRadius
→ bUseWheelSocketScale=false
→ bAutoScaleWheelMeshToRadius=true

SocketScaleFromChassis
→ bUseWheelSocketScale=true
→ bAutoScaleWheelMeshToRadius=false
```

신규 Builder 정상 차량은 `SocketScaleFromChassis`를 기본 의도로 사용하되 기존 Recipe/VehicleData의 mode를 소급 변경하지 않는다.

Enum serialization 안전을 위해 `SocketScaleFromChassis` enumerator는 기존 세 값 뒤에 **append-only**로 추가한다. 기존 enumerator의 순서/수치 의미를 바꾸지 않는다.

`FCFVehicleBaseProfileData`에는 별도 `bUseWheelSocketScale` 정책을 추가하지 않는다. Socket mode는 차량 Chassis Socket이라는 vehicle-specific authoring truth에 의존하므로 **Recipe의 명시 `WheelVisualIntent`가 owner**다. `UseProfilePolicy`는 기존 Legacy/Profile wheel visual policy 의미를 그대로 유지한다.

#### Resolver source ownership

`WheelVisualConfig.bUseWheelSocketScale`의 source 계약은 다음으로 고정한다.

```text
ProjectCompatibilityDefault = false
Recipe.WheelVisualIntent     = explicit override
VehicleBase Profile          = owner 아님
```

즉 Registry descriptor는 VehicleBase Profile을 primary owner로 두지 않는다.

권장 resolve rule 의미:

```text
Project default → Recipe semantic override
```

현재 `ProjectDefaultThenRecipeSemantic` 계열을 재사용할 경우 `BuildRequiredDependencies()`에서 이 exact path를 별도 분기해:

```text
Project.CompatibilityDefaults
Recipe.WheelVisualIntent
```

만 dependency로 둔다. 기존 DefaultDataIntent용 dependency를 그대로 재사용하지 않는다.

`RecipeWheelVisualPolicy`를 그대로 붙여 `Profile.VehicleBase` dependency를 불필요하게 추가하는 것도 금지한다.

### 5.3 기존 물리 필드

기존 필드는 제거하지 않는다.

```text
FrontWheelRadius
RearWheelRadius
FrontWheelWidth
RearWheelWidth
```

그러나 `bUseWheelSocketScale=true`인 차량에서는 이 값의 의미가 바뀐다.

```text
독립 AI/USER 입력값 X
Socket + Wheel Mesh Bounds에서 파생되어 저장되는 결과값 O
```

즉 VehicleData는 계속 Runtime의 최종 값 owner지만, Builder authoring 단계에서 이 네 필드는 Derived output으로 취급한다.

### 5.4 기존 Builder measurement infrastructure 재사용

Source 감사에서 다음 current 구현이 이미 존재함을 확인했다.

```text
FCFVehicleSocketSnapshot.RelativeScale
= 이미 UStaticMeshSocket::RelativeScale을 value-copy

BuildChassisLayoutFingerprint()
= Socket Location / Rotation / Scale을 이미 fingerprint에 포함

FCFVehicleWheelAssetSnapshot
= Wheel ObjectPath + BoundsOrigin + BoundsExtent + MeasureFingerprint 보유

R6 Measurement Proposal
= Wheel Bounds에서 Front/Rear Radius/Width proposal 생성

FCFVehicleAssetAdoption
= Front/Rear Radius/Width의 reviewed acceptance + fingerprint 보유
```

따라서 WSA는 다음을 **새로 만들지 않는다**.

```text
새 Socket Scale Snapshot schema
새 StaticMesh Bounds scanner
별도 Wheel measurement adoption store
별도 Builder persistent progress field
```

기존 AssetSnapshot / Resolver / AssetAdoption lane을 확장하는 것이 정식 구현 방향이다.

Current Field Registry/Reflection coverage는 exact **127 leaf**다. WSA schema 추가는:

```text
VehicleLayoutConfig.WheelAnchorFL.RelativeScale
VehicleLayoutConfig.WheelAnchorFR.RelativeScale
VehicleLayoutConfig.WheelAnchorRL.RelativeScale
VehicleLayoutConfig.WheelAnchorRR.RelativeScale
WheelVisualConfig.bUseWheelSocketScale
```

exact 5 leaf이므로 예상 current coverage는 **127 → 132**다.

이 숫자는 구현 시 actual Reflection/Registry 결과로 재검증한다. unrelated stale count를 무작정 치환하지 않고 해당 Registry/coverage/Batch projection test의 semantic 영향만 확인한다.

---

## 6. 공용 Wheel Mesh 재사용 설계

현재 VehicleVisualConfig는 다음 네 참조를 가진다.

```text
WheelMeshFL
WheelMeshFR
WheelMeshRL
WheelMeshRR
```

P0에서는 schema 대규모 rename을 하지 않는다.

정상 Builder 경로:

```text
WheelMeshFL = 공용 Default Wheel Mesh
WheelMeshFR = 비어 있으면 FL 재사용
WheelMeshRL = 비어 있으면 FL 재사용
WheelMeshRR = 비어 있으면 FL 재사용
```

즉 차량마다 네 개의 동일 Tire Asset을 만들거나 지정할 필요가 없다.

기존 네 필드는 다음 이유로 유지한다.

- 기존 VehicleData/Registry/Recipe 호환
- 향후 전/후륜 또는 개별 Wheel Visual override 가능성
- 대규모 Data Authoring schema migration 회피

Runtime에는 실제 fallback을 구현해야 한다. 현재 Tooltip의 "FL 재사용 권장"만으로는 충분하지 않다.

현재 `bUseWheelVisualOverrides` DerivedGate는 Managed 또는 WheelVisual Adopted 상태로 이미 계산되므로 Socket mode 전용 별도 override gate를 만들지 않는다. `bUseWheelSocketScale`은 크기 Authority만 선택한다.

Builder/Resolver 측 fallback은 AssetSnapshot 자체를 위조해서 FR/RL/RR의 requested identity를 FL로 덮어쓰지 않는다. 계산 시 effective Wheel을 결정하는 helper에서만:

```text
FL = FL 필수
FR = FR가 있으면 FR, 없으면 FL
RL = RL가 있으면 RL, 없으면 FL
RR = RR가 있으면 RR, 없으면 FL
```

순으로 resolve한다.

이렇게 하면 Recipe에서 실제로 비어 있던 슬롯과 effective runtime mesh를 동시에 설명할 수 있다.

---

## 7. Wheel Size 계산 계약

### 7.1 입력

각 Wheel role의 계산 입력:

```text
Resolved Wheel StaticMesh
Wheel StaticMesh Local Bounds
Captured Wheel Socket RelativeScale
```

### 7.2 원본 메시 치수

일반 공용 타이어:

```text
MeshSizeX = Bounds.BoxExtent.X * 2
MeshSizeY = Bounds.BoxExtent.Y * 2
MeshSizeZ = Bounds.BoxExtent.Z * 2
```

radial size는 X/Z가 같은 정상 타이어를 전제로 한다.

정상 canonical Default Wheel:

```text
MeshSizeX ≈ 100
MeshSizeZ ≈ 100
```

### 7.3 최종 시각 크기

```text
VisualScale = SocketScale
```

추가 AutoScale을 곱하지 않는다.

### 7.4 최종 물리 크기

정상 X/Z 일치가 검증되었다는 전제에서:

```text
ValidatedDiameterX = MeshSizeX
ValidatedDiameterZ = MeshSizeZ
// 먼저 X/Z circular tolerance를 통과해야 함
BaseDiameterCm = (ValidatedDiameterX + ValidatedDiameterZ) * 0.5
BaseRadiusCm   = BaseDiameterCm * 0.5

FinalRadiusCm = BaseRadiusCm * SocketScale.X
FinalWidthCm  = MeshSizeY * SocketScale.Y
```

Canonical 100 cm radial mesh 예:

```text
Mesh X/Z = 100 cm
Mesh Y   = 25 cm
Socket Scale = 0.76 / 1.12 / 0.76

Final Diameter = 76 cm
Final Radius   = 38 cm
Final Width    = 28 cm
```

### 7.5 좌우 axle consistency

현재 VehicleMovementConfig는 axle 단위 물리값을 가진다.

따라서:

```text
FL derived Radius/Width == FR derived Radius/Width
RL derived Radius/Width == RR derived Radius/Width
```

를 정상 계약으로 둔다.

좌우가 다르면 평균을 내거나 임의 한쪽을 선택하지 않는다.

```text
Front mismatch → Block
Rear mismatch  → Block
```

USER가 Socket Scale 또는 명시 Mesh를 수정하게 한다.

---

## 8. Capture / Resolver / Persist 설계

### 8.1 Builder read path — 기존 Scale snapshot 재사용

Builder의 current `FCFVehicleAssetReader`는 이미:

```text
UStaticMeshSocket.RelativeLocation
UStaticMeshSocket.RelativeRotation
UStaticMeshSocket.RelativeScale
```

를 `FCFVehicleSocketSnapshot`에 읽고 있다.

또 `BuildChassisLayoutFingerprint()`가 Scale까지 canonical payload에 포함하므로 Socket Scale 변화는 이미 fingerprint drift를 만든다.

따라서 Builder read path에 새 Scale snapshot field나 새 scanner를 만들지 않는다.

### 8.2 Runtime/Legacy direct capture — FCFWheelAnchorPose에 Scale 보존

반면 Runtime `FCFWheelAnchorPose`와 `UCFVehicleData::CaptureLayoutFromChassisSockets()`는 현재 Scale을 저장하지 않는다.

계획:

```text
UStaticMeshSocket.RelativeLocation → FCFWheelAnchorPose.RelativeLocation
UStaticMeshSocket.RelativeRotation → FCFWheelAnchorPose.RelativeRotation
UStaticMeshSocket.RelativeScale    → FCFWheelAnchorPose.RelativeScale
```

Pawn/body legacy capture도 Component/world scale을 역산하지 않고 underlying StaticMesh Socket의 authored RelativeScale을 source truth로 사용한다.

### 8.3 Resolver R6 measurement 확장

current Resolver에는 이미 다음 measurement rule이 있다.

```text
WheelBounds.Radius.v1
WheelBounds.WidthAxisY.v1
```

현재 rule은 Wheel Bounds만 사용하며 Socket Scale은 소비하지 않는다.

WSA 신규 mode에서는 기존 R6 infrastructure를 재사용하되 새 rule revision으로 분기한다.

권장 rule identity:

```text
WheelSocketScale.Radius.v1
WheelSocketScale.WidthAxisY.v1
```

입력:

```text
effective per-wheel Wheel MeasureFingerprint
+ effective Wheel Socket name
+ exact Wheel Socket RelativeScale
+ WSA measurement rule revision
```

**전체 `ChassisLayoutFingerprint`를 Wheel Size measurement fingerprint에 직접 넣지 않는다.**

현재 `ChassisLayoutFingerprint`는 Wheel Socket뿐 아니라 요청된 다른 Socket transform까지 함께 hash할 수 있으므로, Hardpoint/Destroyed FX 등 Wheel Size와 무관한 Socket 변경이 Radius/Width adoption을 불필요하게 Stale 처리할 수 있다.

따라서 Wheel Size 전용 좁은 source fingerprint를 만든다.

권장 개념명:

```text
WheelSizeSourceFingerprint
```

per-wheel payload:

```text
WheelRole
EffectiveWheelMeasureFingerprint
WheelSocketName
WheelSocketRelativeScale
MeasurementRuleRevision
```

axle proposal fingerprint는 Left/Right `WheelSizeSourceFingerprint`를 role을 보존한 채 결합한다.

계산:

```text
per-wheel BaseRadius
= (abs(BoundsExtent.X) + abs(BoundsExtent.Z)) * 0.5
= validated X/Z radial extent 평균

per-wheel Radius = BaseRadius * SocketScale.X
per-wheel Width  = abs(BoundsExtent.Y) * 2 * SocketScale.Y
```

Socket mode는 기존 `WheelMeshRadiusMeasureMode`를 사용하지 않는다. 그 enum은 CF-DL-0072 Legacy AutoScale/legacy Bounds measurement compatibility에만 남긴다. 신규 canonical contract는 X/Z radial, Y width로 축 의미가 고정되어 있기 때문이다.

앞/뒤 axle는 left 우선/right fallback으로 한쪽만 골라 값을 숨기지 않는다.

```text
FL result vs FR result exact/tolerance compare
RL result vs RR result exact/tolerance compare
mismatch → Block
match → axle proposal 1개 생성
```

### 8.4 기존 AssetAdoption 재사용

별도 persisted Wheel Size approval schema를 만들지 않는다.

기존 `FCFVehicleAssetAdoption`의:

```text
bUseMeasuredFrontRadius
bUseMeasuredRearRadius
bUseMeasuredFrontWidth
bUseMeasuredRearWidth

Front/Rear Radius/Width AssetFingerprint
```

를 그대로 재사용한다.

Socket mode의 measurement fingerprint는 전체 `ChassisLayoutFingerprint`가 아니라 위에서 정의한 **WheelSizeSourceFingerprint**를 사용한다.

그 결과:

```text
Wheel Mesh / Bounds 변경 → Stale
해당 Wheel Socket 이름 변경 → Stale
해당 Wheel Socket Scale 변경 → Stale
Hardpoint 위치만 변경 → Wheel Size adoption 유지
Wheel Socket Location/Rotation만 변경 → Wheel Size adoption 유지
```

Wheel Socket Location/Rotation 변경은 Layout 자체를 Stale 처리하지만 타이어 **크기** 결과까지 다시 승인하게 만들지는 않는다.

Legacy mode에서는 기존 `WheelBounds.*.v1` proposal/fingerprint semantics를 그대로 유지해 기존 Recipe를 불필요하게 Stale 처리하지 않는다.

### 8.5 Layout Scale의 Resolver / Builder equality 적용

current Resolver는 Wheel Socket에서 `RelativeLocation / RelativeRotation`만 `VehicleLayoutConfig.WheelAnchor*` AssetDerived candidate로 만든다.

또 current `FCFVehicleBuilderVM::EvaluateLayoutCaptureStep()`의 direct persisted equality 역시 Location/Rotation만 비교하고 Scale은 비교하지 않는다.

따라서 WSA 구현에서는 **두 곳 모두 Scale을 추가**해야 한다.

WSA 구현에서는 Registry에 각 `RelativeScale` leaf를 추가하고 같은 `ChassisLayoutFingerprint` source로 AssetDerived candidate를 추가한다.

즉 신규 Builder 정상 경로의 Layout persist는 direct raw capture가 아니라 기존 Resolver → Diff → Step 7 DefinitionApply writer lane을 사용한다.

Legacy `CaptureLayoutFromChassisSockets()` API는 호환을 위해 Scale capture만 확장하되 Guided Builder의 병렬 writer로 사용하지 않는다.

### 8.6 Persist flow

`SocketScaleFromChassis` 정상 경로:

```text
1. 기존 AssetSnapshot에서 Socket Location/Rotation/Scale + Wheel Bounds read
2. Step 2/3 validation
3. Resolver R6 Socket-scaled measurement proposal 생성
4. left/right axle consistency validate
5. USER가 derived Radius/Width review
6. 기존 AssetAdoption reviewed decision에 exact combined fingerprint 저장
7. Resolver가 Layout RelativeScale + accepted Radius/Width를 effective candidate로 구성
8. Step 7 Final Review / Diff
9. explicit DefinitionApply 단일 writer lane으로 VehicleData persist
```

Runtime spawn 시 물리값을 숨겨서 매번 다시 계산하는 구조를 기본으로 하지 않는다.

### 8.7 Stale 규칙

Layout과 Wheel Size의 stale 범위를 분리한다.

Layout stale:

```text
Chassis Mesh
Wheel Socket name / Location / Rotation / Scale
```

Wheel Size adoption stale:

```text
effective Wheel Mesh path / Bounds
해당 Wheel Socket name / Scale
SocketScaleFromChassis / legacy mode
measurement rule revision
```

즉 Location/Rotation이나 unrelated Hardpoint 변경만으로 Radius/Width reviewed adoption을 폐기하지 않는다.

기존 `ChassisLayoutFingerprint`는 Layout stale authority로 계속 사용하고, Radius/Width는 좁은 `WheelSizeSourceFingerprint`를 사용한다.

---

## 9. Runtime 적용 설계

### 9.1 Wheel Anchor

`Wheel_Anchor_*`에는 계속 다음만 적용한다.

```text
RelativeLocation
RelativeRotation
```

Socket Scale을 Wheel_Anchor 컴포넌트 자체에 적용하지 않는다.

이유:

- 기존 WheelSync의 suspension / steering / spin transform ownership 보호
- 부모 scale에 의해 위치/회전/child transform 의미가 섞이는 것을 방지

### 9.2 Wheel Mesh

`SocketScaleFromChassis`에서는 `Wheel_Mesh_*`에:

```text
RelativeScale3D = Captured Socket RelativeScale
```

를 **exact set**한다. 기존 component scale에 곱하지 않는다.

이 모드에서는 canonical Wheel Mesh의 centered pivot을 전제로 하므로 Legacy AutoScale 전용 다음 값들은 시각 크기 계산에 사용하지 않는다.

```text
WheelMeshRadiusMeasureMode
bAutoCenterWheelMeshBoundsToOrigin
WheelMeshScaleClampMin
WheelMeshScaleClampMax
```

특히 old AutoCenter correction을 Socket mode의 크기/위치 보정으로 재사용하지 않는다. Pivot/Bounds center가 잘못된 Wheel Mesh는 Step 2에서 Block하고 Asset 자체를 normalize한다.

### 9.3 WheelSync

현재 Source call order를 감사한 결과 신규 Scale 적용은 기존 WheelSync ownership과 충돌하지 않는다.

```text
InitializeVehicleRuntime
→ ApplyVehicleDataConfig
   → ApplyVehicleWheelVisualConfig
→ ApplyVehicleLayoutConfig
→ PrepareWheelSync / TryPrepareWheelSync
   → CaptureBaseWheelVisualState
```

`CaptureBaseWheelVisualState()`는 Anchor Location/Rotation과 Wheel Mesh Rotation만 캡처하며 Scale을 소유하지 않는다. `ApplySingleWheelInputPhase2()`도 suspension/steering/spin의 Location/Rotation만 갱신하고 Wheel Mesh Scale을 덮어쓰지 않는다.

따라서 Socket Scale은 WheelSync 준비 전에 Wheel_Mesh에 exact 적용하면 이후 Tick에서 보존된다.

현재 구조를 유지한다.

```text
Wheel_Anchor_*
  ↓
Wheel_Mesh_*
  ↓
UCFWheelSyncComp
```

Chassis StaticMesh Socket은 runtime permanent parent가 아니라 **authoring source**다.

### 9.4 Physics

Chaos Wheel은 persisted `Front/Rear WheelRadius/Width`를 기존 Runtime 경로로 적용한다.

Socket mode runtime이 물리값을 별도 hidden override하지 않는다.

---

## 10. 기존 AutoScale 처리

현재 기능:

```text
bAutoScaleWheelMeshToRadius
Physics Radius
→ Visual Mesh Scale 자동 계산
```

새 정상 경로:

```text
USER Socket Scale
→ Visual Scale
→ Physics Radius/Width derive
```

Authority 방향이 반대다.

따라서:

```text
bUseWheelSocketScale=true
AND
bAutoScaleWheelMeshToRadius=true
```

는 허용하지 않는다.

정책:

- Builder 신규 차량은 `bAutoScaleWheelMeshToRadius=false`
- Socket Scale과 AutoScale을 곱하지 않음
- Validator에서 동시 활성화를 Error/Block
- 기존 AutoScale 코드는 즉시 삭제하지 않고 Legacy compatibility로 유지
- `CF-DL-0072`의 "WheelRadius → Visual AutoScale"을 신규 정상 제작 정책에서는 Superseded 처리

현재 persisted 상태 중 확인된 예:

```text
DA_TestSedan: bAutoScaleWheelMeshToRadius = true
DA_TestSUV:   bAutoScaleWheelMeshToRadius = false
```

Design 단계에서는 Asset을 수정하지 않는다.
실제 migration은 구현/검증 Gate에서 수행한다.

---

## 11. Reference Vehicle 데이터와의 관계

실차 Tire/Wheel 제원은 버리지 않는다.

하지만 Socket Size mode에서 역할을 다음처럼 바꾼다.

기존 해석:

```text
Reference Tire Size
→ AI Physics Proposal
→ Front/Rear WheelRadius/Width
```

새 해석:

```text
Reference Tire Size
→ USER Socket Scale 작성 시 참고값
→ Builder sanity comparison
```

최종 physical Radius/Width는 USER Socket Scale + actual Wheel Mesh Bounds에서 파생한다.

즉 Reference Tire Size가 USER 시각 결정을 자동으로 덮어쓰지 않는다.

Builder는 다음처럼 보여줄 수 있다.

```text
Reference Tire Outer Diameter: 67.3 cm
USER Socket-derived Diameter:   69.0 cm
Difference: +2.5%
```

차이가 있어도 무조건 자동 교정하지 않는다.
큰 차이는 Warning으로 보여주고 USER가 의도 여부를 판단한다.

---

## 12. Builder Step 통합

### Step 2 — Mesh Prep

추가 표시:

```text
Resolved Default Wheel Mesh
Bounds Size X/Y/Z
Canonical radial 100 cm PASS/Warning
Bounds Center / Origin sanity
Shared Wheel fallback 상태
```

AI가 타이어 크기를 정하지 않는다.

### Step 3 — Socket Guide

각 Socket에:

```text
Location
Rotation
Scale
X/Z radial scale
Y width scale
X/Z equality
```

를 표시한다.

Guide 문구 핵심:

> 휠하우스를 보면서 타이어의 최종 크기를 직접 맞추세요. X/Z는 직경, Y는 폭입니다. X와 Z는 같은 값을 사용합니다.

### Step 4 — Layout Capture

기존 AssetSnapshot이 이미 Socket Scale을 읽고 fingerprint에 포함하므로 새 capture reader는 만들지 않는다.

Step 4에서는:

```text
Location / Rotation / Scale current truth
+ effective Wheel Mesh Bounds
+ Socket-scaled R6 measurement proposal
```

를 함께 Review한다.

예:

```text
FL Scale 0.72 / 1.08 / 0.72
FR Scale 0.72 / 1.08 / 0.72
→ Front Radius 36.0 cm
→ Front Width 27.0 cm
→ Measurement Source = WheelSocketScale.*.v1
```

USER review 뒤 기존 AssetAdoption lane에 exact combined fingerprint를 기록하며, 실제 Target VehicleData 저장은 Step 7 DefinitionApply 전에는 수행하지 않는다.

### Step 5 — Physics Proposal

`bUseWheelSocketScale=true`이면:

```text
FrontWheelRadius
FrontWheelWidth
RearWheelRadius
RearWheelWidth
```

를 AI independent proposal 대상에서 제외한다.

이 네 값은 Socket-derived field로 표시한다.

AI Reference Proposal은 해당 값을 덮어쓰지 않는다.

#### Builder-private VehicleBase Profile 보호

Source 감사 결과 현재 Step 5 Physics Draft는 `FCFBuilderPrivateProfilePayload.VehicleBaseData` 전체를 받아 private VehicleBase Profile의 `Data` 전체를 교체할 수 있다.

VehicleBase Profile에는 기존 Reference fallback용으로 다음 필드가 남아 있다.

```text
bUseReferenceWheelGeometry
FrontWheelRadius
RearWheelRadius
FrontWheelWidth
RearWheelWidth
```

Socket mode에서 이 값들은 최종 Wheel Size Authority가 아니므로 AI Draft가 변경해서는 안 된다.

정책:

```text
SocketScaleFromChassis
→ bUseReferenceWheelGeometry = false 유지
→ AI Physics Draft가 bUseReferenceWheelGeometry를 true로 변경하면 Block
→ AI Physics Draft가 Front/Rear Radius/Width Profile 값을 변경하면 Block
→ silently overwrite / normalize하지 않음
```

즉 Step 5 request validation에서 current private VehicleBase payload와 incoming AI Draft의 위 5개 필드를 exact/bool + tolerant-float 비교해 **baseline-preserve**를 요구한다.

이렇게 해야:

- AI가 Wheel Size의 lower-precedence shadow 값을 남기지 않음
- Profile fingerprint/receipt가 타이어 치수 제안 때문에 불필요하게 흔들리지 않음
- Reference Tire 제원은 Evidence/sanity reference 역할만 유지
- 실제 persisted VehicleData Radius/Width는 AssetAdoption + Resolver의 Socket-derived candidate만 소유

한다.

### Step 7 — Final Review

Diff/Source Trace에 source 의미를 분명히 표시한다.

권장 provenance label:

```text
WheelSocketScaleDerived
```

예:

```text
FrontWheelRadius = 36.0
Source = WheelSocketScaleDerived
Mesh = /Game/.../Wheel_FL
Socket = Wheel_Anchor_FL/FR
Scale = 0.72 / 1.08 / 0.72
```

### Step 6 — Gameplay Guidance / WheelVisual mode-aware validation

현재 `ReadBuilderGameplayGuidance()`는 VehicleBase WheelVisual completeness를 먼저 검사하고, Base Profile의 `bAutoScaleWheelMeshToRadius=true`이면 Legacy ScaleClamp까지 검증한다.

이 검증은 effective mode에 따라 분리해야 한다.

공통 검증:

```text
VehicleBase Profile present
ExpectedWheelCount > 0
0 <= FrontWheelCountForSteering <= ExpectedWheelCount
```

Legacy AutoScale에서만 검증:

```text
WheelMeshScaleClampMin/Max finite/positive/order
WheelMeshRadiusMeasureMode
bAutoCenterWheelMeshBoundsToOrigin 관련 legacy semantics
```

다음 mode에서는 Legacy AutoScale clamp를 blocker로 사용하지 않는다.

```text
ManualMeshScale
SocketScaleFromChassis
```

`SocketScaleFromChassis`의 WheelVisual summary는 다음 의미를 직접 표시한다.

> 차체 Wheel Socket Scale이 타이어 크기 Authority입니다. X/Z는 직경 배율, Y는 폭 배율이며 Builder는 휠하우스를 보고 크기를 자동 결정하지 않습니다.

### Step 8 — Driving

기존 Technical Driving benchmark를 사용한다.

USER Driving에서는 추가로 다음을 본다.

- 타이어가 지면에 파묻혀 보이지 않음
- 타이어가 지면에서 떠 보이지 않음
- 휠하우스 내 시각 크기 만족
- 조향/서스펜션/스핀 WheelSync 회귀 없음

---

## 13. Validator 계약

신규 검증:

### Socket Scale

Error:

```text
non-finite
X <= 0
Y <= 0
Z <= 0
X/Z mismatch beyond tolerance
```

### Canonical Wheel Mesh

신규 Builder 정상 `SocketScaleFromChassis` 경로에서:

```text
WheelMeshFL missing → Error
FR/RL/RR missing → FL fallback 허용
Bounds invalid/zero → Error
radial X/Z non-circular beyond tolerance → Error
X/Z 100 cm canonical tolerance 이탈 → Error/Block
Y 25 cm canonical tolerance 이탈 → Error/Block
Bounds center가 origin에서 크게 벗어남 → Error/Block
```

Socket Scale 숫자를 실제 cm 단위와 직접 대응시키는 것이 이 모드의 핵심 계약이므로 non-canonical Wheel Mesh를 단순 Warning으로 허용하지 않는다.

### Axle consistency

```text
FL/FR derived radius/width mismatch → Error
RL/RR derived radius/width mismatch → Error
```

### Mode conflict

```text
bUseWheelSocketScale
&& bAutoScaleWheelMeshToRadius
→ Error
```

### Resolver source ownership

Error/설계 회귀:

```text
bUseWheelSocketScale가 VehicleBase Profile source를 가짐
bUseWheelSocketScale dependency에 unrelated DefaultDataIntent가 포함됨
SocketScaleFromChassis인데 Recipe semantic candidate가 생성되지 않음
```

정상 source:

```text
Project default false
+ Recipe WheelVisualIntent explicit semantic
```

### Step 5 AI ownership conflict

`SocketScaleFromChassis`에서 AI Physics Draft가 다음 private VehicleBase 값 중 하나라도 current baseline에서 변경하면 Block한다.

```text
bUseReferenceWheelGeometry
FrontWheelRadius
RearWheelRadius
FrontWheelWidth
RearWheelWidth
```

신규 Builder Socket mode의 `bUseReferenceWheelGeometry`는 false를 유지한다.

### Derived value drift

Socket mode가 켜진 저장 VehicleData에서:

```text
stored Front/Rear Radius/Width
!= current Mesh Bounds + captured Socket Scale derived value
```

이면 Stale/Error로 보고 recapture/reapply를 요구한다.

자동 수정하지 않는다.

---

## 14. C++ / Blueprint 책임 분배

### C++

C++가 소유한다.

- 기존 AssetReader가 제공하는 StaticMesh Bounds / Socket RelativeScale snapshot 재사용
- FCFWheelAnchorPose RelativeScale persist
- Wheel size deterministic calculation
- left/right consistency validation
- shared Wheel Mesh effective fallback
- Resolver R6 Socket-scale measurement/fingerprint
- VehicleData derived value validation
- Runtime Wheel_Mesh scale 적용
- Builder read-only evaluator에 제공할 typed result
- Automation tests

공용 계산 helper는 Runtime module에 두는 것을 권장한다.

```text
CFWheelSizeUtils.h/.cpp
```

역할 예:

```text
ValidateWheelSocketScale
MeasureBaseWheelRadius
DeriveWheelSizeFromBoundsAndScale
AreWheelSizesCompatible
```

`TryReadWheelMeshBounds` 같은 별도 scanner는 만들지 않는다. Bounds read authority는 기존 `FCFVehicleAssetReader`를 유지한다.

Editor 모듈은 Runtime helper를 재사용해 Resolver/Runtime이 같은 계산식을 사용하도록 한다.

### Blueprint / Asset

BP에는 계산 로직을 넣지 않는다.

BP/Asset의 역할:

- 기존 Wheel_Anchor_* / Wheel_Mesh_* 컴포넌트 조립 유지
- USER가 StaticMesh Socket을 직접 배치/스케일
- Default Wheel StaticMesh asset 제공
- 필요 시 Details에서 결과 확인

WheelSync transform logic을 BP로 되돌리지 않는다.

---

## 15. 구현 대상 파일 예상

정확한 변경 범위는 구현 시작 전 fresh diff로 다시 확인한다.

예상 주요 파일:

```text
UE/Source/CarFight_Re/Public/CFVehicleData.h
UE/Source/CarFight_Re/Private/CFVehicleData.cpp
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Private/CFVDAValidator.cpp

UE/Source/CarFight_Re/Public/CFWheelSizeUtils.h
UE/Source/CarFight_Re/Private/CFWheelSizeUtils.cpp

UE/Source/CarFight_ReEditor/Public/DataAuthoring/CFVehicleAuthoringTypes.h
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleFieldRegistry.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleResolver.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderGuide.cpp
UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleBuilderVM.cpp

관련 Runtime / Resolver / Builder / Validator tests

현재 CFVehicleAssetReader는 RelativeScale/Bounds를 이미 제공하므로 새 read 기능 때문에 수정할 필요는 없다. effective FL fallback 구현이 Reader까지 내려갈 필요가 있는지는 구현 단계에서 최소 범위로 판단한다.
```

`CFWheelSizeUtils` 신규 파일은 32자 제한을 만족한다.

구현 중 실제 필요가 없는 파일은 건드리지 않는다.

---

## 16. Scope Out

이번 Plan에서 하지 않는다.

```text
Wheel center 자동 검출
휠하우스 geometry 자동 분석
AI Vision으로 타이어 최적 크기 자동 선택
Wheel Socket 자동 생성/자동 배치
사용자 승인 없는 Socket/Asset mutation
자동 Save / Save All
WheelSync 구조 재작성
Chaos Wheel 전체 물리 재설계
차종별 Tire Mesh 복제 자동 생성
Reference Tire Size로 USER Socket Scale 강제 보정
```

---

## 17. Migration

### 기존 VehicleData

새 필드 기본값:

```text
FCFWheelAnchorPose.RelativeScale = 1 / 1 / 1
bUseWheelSocketScale = false
```

따라서 새 코드가 들어가도 기존 Asset은 기존 동작을 유지해야 한다.

### 기존 AutoScale 차량

기존 AutoScale 차량을 Socket Size mode로 전환할 때:

```text
1. 공용 Default Wheel Mesh 연결
2. Chassis Wheel Socket Scale을 USER가 시각적으로 작성
3. Layout/Scale capture
4. derived Radius/Width review
5. bUseWheelSocketScale=true
6. bAutoScaleWheelMeshToRadius=false
7. explicit Apply
8. Runtime + USER Driving 검증
```

AutoScale과 SocketScale을 동시에 켜서 기존 결과에 배율을 곱하는 migration은 금지한다.

---

## 18. Gate

### WSA-P0-00 — Current Contract Audit / Authority Lock

상태: **Design PASS**

확정:

- USER가 Socket Scale로 타이어 크기를 결정
- StaticMesh Bounds는 측정 Source
- Socket mode Wheel Mesh canonical Bounds = 100 × 25 × 100 cm
- Visual/Physics same source
- Legacy AutoScale은 신규 정상 경로에서 사용하지 않음
- Wheel Anchor scale은 runtime parent scale로 쓰지 않음
- VehicleData Radius/Width는 Socket mode에서 derived persisted value

Source/Asset mutation: 0

### WSA-P0-01 — Canonical Default Wheel + Data Schema & Shared Size Utility

Canonical Asset sub-gate: **PASS**

확인 evidence:

```text
/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL.Wheel_FL
Bounds Size = 99.9990 × 24.9992 × 99.9990 cm
Bounds Center ≈ 0 / 0 / 0
Git shared Wheel_FL.uasset = modified
```

구현 적용 상태:

- `FCFWheelAnchorPose.RelativeScale = FVector::OneVector` additive schema 적용
- `FCFVehicleWheelVisualConfig.bUseWheelSocketScale = false` additive schema 적용
- `ECFWheelVisualIntentMode::SocketScaleFromChassis` 기존 enum 뒤 append-only 적용
- Field Registry/Reflection expected 127→132로 확장
- `WheelVisualConfig.bUseWheelSocketScale` source를 Project Default(false) + Recipe WheelVisualIntent 전용으로 고정
- `CFWheelSizeUtils.h/.cpp` 공용 deterministic helper 추가
- `CFWheelSizeTests.cpp` focused Runtime schema/helper/legacy-load test 추가
- 132-leaf field/hash contract 변화에 맞춰 `ResolverContractRevision 2→3` 적용 — 이전 preview/approval/Builder receipt stale
- legacy default `RelativeScale=OneVector / bUseWheelSocketScale=false` 보존

Asset normalization은 USER가 시각/3D asset 준비 authority를 소유하며 이번 canonical asset sub-gate는 USER 조정 + live Bounds readback으로 닫았다. 코드가 mesh geometry를 자동 추정/변형하지 않는다.

최종 validation checkpoint:

```text
Fresh Official Build Job = c74519c538404520b31896c77cfceba4
Build Result = Succeeded / Exit 0
Runtime DLL Link = PASS
Editor DLL Link = PASS
Registry132 = 1/1 PASS
SchemaUtilityLegacy = 1/1 PASS
Focused Automation Process = 0cd47ff1f47944bd95407d3e3705a9b9 / Exit 0
Legacy DA_TestSUV load = PASS
Legacy RelativeScale = OneVector
Legacy bUseWheelSocketScale = false
Legacy false path preserved
```

WSA-P0-01은 **Technical PASS**로 닫는다.

추가 운영 note:

- interactive `editor.start`는 두 번 모두 `ERR_UE_MCP_UNAVAILABLE` readiness postcondition에서 실패했다.
- 같은 기동 재시도는 중단했고, MCP listener가 필요 없는 `Tools/RunWheelSizeTests.ps1` focused headless wrapper로 exact 2 tests만 실행해 PASS했다.
- Editor startup readiness 문제는 WSA code correctness와 분리하며 이번 gate의 blocker로 확대하지 않는다.

### WSA-P0-02 — Socket Resolver & Derived Physics

목표:

- existing Builder AssetSnapshot RelativeScale 재사용
- Layout에는 ChassisLayoutFingerprint 재사용, Wheel Size에는 narrow WheelSizeSourceFingerprint 사용
- Runtime/legacy FCFWheelAnchorPose capture scale 보존
- Registry RelativeScale leaf + Resolver AssetDerived candidate
- existing R6 measurement를 Socket Scale-aware rule로 확장
- Socket mode는 Legacy WheelMeshRadiusMeasureMode 무시 / X-Z circular validation + 평균 radial extent 사용
- existing AssetAdoption reviewed fingerprint 재사용
- Bounds + Scale → Radius/Width
- axle consistency
- explicit Step 7 persist 준비

최종 validation checkpoint:

```text
Official Build Job = 1ebcc7850e3e4c83adbae3de3590d0ec
Build Result = Succeeded / Exit 0
Runtime DLL Link = PASS
Editor DLL Link = PASS
ResolverContractRevision = 4
R5 RelativeScale AssetDerived = PASS
Front Socket-derived Radius/Width = 36 / 28 cm
Rear Socket-derived Radius/Width = 40 / 25 cm
Legacy WheelMeshRadiusMeasureMode ignored in Socket mode = PASS
Location-only change keeps Wheel Size fingerprint = PASS
Scale change changes Wheel Size fingerprint = PASS
X/Z mismatch fail-closed = PASS
Axle size mismatch fail-closed = PASS
AssetAdoption source = Measurement.WheelSocketScale.*.v1
Focused Automation Process = 7be4965e5c804f79ae9b87355701d860 / Exit 0
Registry132 = PASS
SchemaUtilityLegacy = PASS
SocketScaleDerived = PASS
```

WSA-P0-02는 **Technical PASS**로 닫는다. Runtime/legacy capture는 underlying StaticMeshSocket RelativeScale을 보존하며 Wheel_Anchor runtime component에는 Scale을 적용하지 않는다.

### WSA-P0-03 — Runtime Visual / Mesh Fallback

목표:

- FL default mesh fallback
- Wheel_Mesh relative scale 적용
- Wheel_Anchor scale 미적용
- WheelSync 기존 ownership 보존

최종 validation checkpoint:

```text
USER-confirmed fresh Official Build = PASS
Focused Automation Process = 304cd2dbfaed46cda7a1db7c0da9feb9 / Exit 0
Registry132 = PASS
SchemaUtilityLegacy = PASS
SocketScaleDerived = PASS
RuntimeVisualFallback = PASS
WheelMeshFL only + FR/RL/RR runtime fallback = PASS
Wheel_Mesh exact Socket Scale = PASS
Wheel_Anchor scale unchanged = PASS
WheelSync steering/suspension/spin = PASS
Wheel_Mesh Socket Scale survives WheelSync = PASS
Legacy false path protection = PASS
```

WSA-P0-03은 **Technical PASS**로 닫는다.

### WSA-P0-04 — Validator / Builder Integration

목표:

- Step 2~5/6/7 source 의미 반영
- stale detection
- AutoScale conflict blocker
- `bUseWheelSocketScale` ProjectDefault + Recipe-only source ownership
- Gameplay Guidance mode-aware WheelVisual validation
- Reference Tire Size를 sanity reference로 재분류
- Socket mode Step 5 private VehicleBase wheel geometry baseline-preserve guard

최종 validation checkpoint:

```text
Official Build Job = 6215416c66304a048ba83cbe1d5a8ca2
Build Result = Succeeded / Exit 0
Runtime DLL Link = PASS
Editor DLL Link = PASS
Focused Automation Process = 30add52f8ba94f4b8f206356e7c33faa / Exit 0
Registry132 = PASS
SchemaUtilityLegacy = PASS
SocketScaleDerived = PASS
RuntimeVisualFallback = PASS
BuilderShell = PASS
GameplayGuidance = PASS
BuilderProfileCommit = PASS
ValidatorContract = PASS
Canonical Wheel Bounds validation shared helper = PASS
SocketScale Resolver FL-only shared Wheel fallback = PASS
Step 2 canonical 100x25x100 + centered bounds validation = PASS
Step 3 valid Socket Scale / axle consistency = PASS
Step 3 invalid X/Z actionable Blocked while top-level preview fail-closed = PASS
Step 4 RelativeScale change stale detection = PASS
Socket/Manual mode ignores unused Legacy AutoScale clamp = PASS
Socket mode Reference wheel geometry 5-field Builder mutation guard = PASS
Runtime Validator AutoScale conflict / invalid Scale / axle mismatch = PASS
UseProfilePolicy/AutoScale legacy behavior protected by focused regression
```

P0-04 검증 중 발견된 실제 integration gap도 함께 교정했다.

- SocketScale Resolver가 optional FR/RL/RR Wheel Mesh 미지정 시 Runtime/Builder와 달리 raw slot을 요구하던 문제를 교정해 `WheelMeshFL`을 deterministic fallback으로 재사용한다.
- 전체 Resolver Preview가 invalid Socket Scale 때문에 Blocked여도 이번 refresh의 fresh Resolve read/AssetSnapshot 자체는 Step 2~4 진단에 사용할 수 있도록 Builder diagnostic freshness를 Apply authority와 분리했다.
- 따라서 USER는 X/Z mismatch에서 generic Stale/Locked가 아니라 Step 3의 실제 Socket Scale blocker를 직접 확인한다.

WSA-P0-04는 **Technical PASS**로 닫는다.

### WSA-P0-05 — Legacy Migration

상태: **Technical PASS**

목표:

- DA_TestSedan 등 현재 AutoScale 사용 자산 전환 여부 판단
- 신규 Wagon E2E candidate에 새 mode 적용
- 기존 SUV/legacy path 보존

USER가 실제 Socket Scale을 작성하기 전 임의 migration 금지.

최종 migration 판정:

```text
Wagon
= Category A / USER-authored Socket Scale evidence 있음
= Recipe WheelMeshFL → /Game/CarFight/Vehicles/Shared/Tire/Wheel_FL
= Recipe WheelMeshFR/RL/RR → null, FL fallback 사용
= Recipe WheelVisualIntent → SocketScaleFromChassis
= Front/Rear Radius/Width Socket-derived measurement adoption 4건 → persisted
= Target VehicleData Apply → 아직 하지 않음 / Builder Step 7까지 defer

DA_TestSedan
= bAutoScaleWheelMeshToRadius=true
= independent Sedan wheel geometry + USER Socket Scale migration evidence 없음
= 자동 migration 금지 / Legacy 유지

DA_Veh_HeavyFinite / DA_TestSedan_DRAP50
= AutoScale=true test/historical fixture
= 유지

SUV 계열 / PoliceCar legacy
= 기존 manual/legacy path 유지
```

실제 Wagon E2E는 현재 Step 2~4 상태다. fresh Preview에서 WSA 자체 Resolver blocker는 없고 다음 **후속 Builder Step 미완성**만 확인됐다.

```text
Recipe Blocked 4
= Vehicle Base / Drivetrain / Handling / Performance Profile Snapshot 미지정

Definition Error 2
= FrontWheelClass / RearWheelClass 미지정

Definition Warning
= FR/RL/RR WheelMesh null — WSA FL fallback 의도와 정상 양립
= DriveState override off
```

따라서 WSA migration이 Profile/WheelClass를 임의 생성하거나 Validator를 우회해 Target Apply를 강행하지 않는다. 기존 Builder 계약대로 **Step 2~4의 Recipe authoring truth만 저장하고 Target Definition은 Step 7 explicit Apply까지 미변경**으로 유지한다.

persisted readback:

```text
DA_Recipe_Wagon
WheelVisualIntent.Mode = SocketScaleFromChassis
WheelMeshFL = /Game/CarFight/Vehicles/Shared/Tire/Wheel_FL.Wheel_FL
WheelMeshFR/RL/RR = 별도 persisted reference 없음
AssetAdoption FrontRadius/RearRadius/FrontWidth/RearWidth = true
ProfileBindings = 전부 null 유지
Dataset = adset_v1_482c6abcabd817f213a80f7ff87366c8.e617e1a0b1c00032ba8f1845

DA_Vehicle_Wagon
VehicleVisualConfig WheelMeshFL/FR/RL/RR = null
bUseWheelSocketScale = false
bAutoScaleWheelMeshToRadius = false
bUseLayoutOverrides = false
WheelAnchor RelativeScale = OneVector baseline
Front/Rear Radius = 30/30, Width = 기존 6/6 baseline 유지
Dataset = adset_v1_57d31c4a91091b31e288d80608c701dc.a6136e3eebf35bbcaf001532
```

P0-05 actual migration 과정에서 발견된 공용 integration gap도 함께 교정했다.

- Recipe `TSoftObjectPtr<UStaticMesh>` → Target hard Object leaf projection이 source soft path text를 그대로 복사해 R15 Definition hash readback mismatch를 만들던 문제를 `CFVehicleResolver.cpp v1.5.2`에서 target class-qualified canonical reference로 정규화했다. live UObject load는 추가하지 않아 snapshot-only Resolver 계약을 유지한다.
- `ObjectReferenceRoundTrip` focused regression을 추가했고 WSA affected exact 9 tests가 모두 PASS했다.
- migration commandlet은 canonical Wheel 실제 99.9990×24.9992×99.9990cm의 sub-mm 오차를 공용 0.01cm tolerance로 판정하고, Save 실패 rollback과 structured validation diagnostic을 보유한다.
- 실제 Wagon이 아직 Step 2~4이면 exact expected blocker signature에서 Recipe 1개만 저장하고 Target Apply를 defer한다. 다른 Error/Blocked가 섞이면 fail-closed한다.

최종 validation checkpoint:

```text
Official Build = 05115a84e3c34c4c80efe1695f998174 / Succeeded / Exit 0
WSA Focused Process = 798f21a197254d82932a5ab0b8f31868 / Exit 0
Exact affected tests = 9/9 PASS
ObjectReferenceRoundTrip = PASS
Wagon migration process = c66f7579d3114ba3afb29502f7f5dd6c / Exit 0
Migration marker = CF_WSA_MIGRATE_RECIPE_ONLY_PASS
saved_packages = 1
Target Apply = deferred_to_builder_step7
Legacy/Test/SUV VehicleData mutation = 0
```

WSA-P0-05는 **Technical PASS**로 닫는다. 다음 Gate는 `WSA-P0-06 Technical Validation`이며, P0-05에서 이미 fresh 수행한 Build/focused tests/persisted readback은 새 failure 없이 반복하지 않는다.

### WSA-P0-06 — Technical Validation

상태: **Technical PASS**

순서:

```text
Official Editor Build
→ Wheel size focused tests
→ VehicleData Validator tests
→ Builder Step/Resolver tests
→ WheelSync affected regression
→ persisted AssetDump readback
→ fresh Runtime technical check
```

P0-06 post-review에서 deferred Apply fail-closed 조건과 negative regression을 보강한 뒤 current final source를 다시 검증했다. Wagon Product Asset migration/Save는 재실행하지 않았다.

최종 validation checkpoint:

```text
Official Editor Build
= e2cfa435b5764dc282ec2b2e973175b6
= Succeeded / Exit 0

WSA focused affected regression
= d63625118ad94599a56e4151bb9b6dc6
= exact 10/10 PASS / Exit 0

Wheel size / schema / Resolver
= Registry132 PASS
= SchemaUtilityLegacy PASS
= SocketScaleDerived PASS
= ObjectReferenceRoundTrip PASS
= DeferredApplyGuard PASS

Runtime / WheelSync
= RuntimeVisualFallback PASS
= transient World + actual ACFVehiclePawn spawn
= shared Wheel FL fallback 4 wheels PASS
= Wheel_Mesh authored Scale exact PASS
= Wheel_Anchor parent scale 비소유 PASS
= steering / suspension / spin WheelSync PASS
= Wheel_Mesh Socket Scale survives WheelSync PASS

Builder / Validator
= BuilderShell PASS
= GameplayGuidance PASS
= BuilderProfileCommit PASS
= ValidatorContract PASS

Persisted AssetDump
= DA_Recipe_Wagon WSA Recipe migration persisted PASS
= DA_Vehicle_Wagon unapplied baseline preserved PASS
```

`RuntimeVisualFallback`은 단순 helper 계산이 아니라 transient Editor World에 `ACFVehiclePawn`을 생성하고 `ApplyVehicleWheelVisualConfig`, `ApplyVehicleLayoutConfig`, `UCFWheelSyncComp::ApplyWheelVisualInputsPhase2`를 실제 호출하는 runtime technical regression이다. 따라서 P0-06의 fresh Runtime technical check를 충족한다.

post-review에서 `IsExpectedDeferredApplyValidation()`은 Vehicle Base / Drivetrain / Handling / Performance Profile 누락과 Front/Rear WheelClass 누락의 **exact signature**에서만 defer를 허용하도록 강화했다. DriveState 대체, duplicate Profile/WheelClass, existing Profile binding, 예상 밖 Resolver/Definition blocker는 모두 fail-closed한다. `DeferredApplyGuard`가 이 negative contract를 검증한다.

검증 중 `RunDataAuthoringTests.ps1`의 `Start-Process -Wait`가 Automation `TestExit` 뒤 descendant process를 계속 기다려 상위 runner가 terminal로 회수되지 않는 문제를 확인했다. 기존 `RunBuilderBench.ps1`의 accepted pattern과 동일하게 exact UnrealEditor process handle `WaitForExit()`로 교정했고, 재실행한 focused suite가 10개 test를 연속 terminal 회수해 PASS했다.

다만 이 Technical PASS는 **실제 Wagon Product Target의 시각/주행 PASS가 아니다.** actual `DA_Vehicle_Wagon`은 P0-05 계약대로 아직 Step 7 Definition Apply 전 상태이며, 실제 Wagon의 Socket Scale `(0.8,1.0,0.8)` 시각 지면 접촉/휠하우스 비율/주행감은 P0-07 USER Acceptance에서만 판정한다.

WSA-P0-06은 **Technical PASS**로 닫는다. 다음 Gate는 `WSA-P0-07 USER Acceptance`이며 actual Wagon의 후속 Builder Profile/WheelClass 구성 → Final Apply → PIE가 선행된다.

### WSA-P0-07 — USER Acceptance

상태: **USER PASS / Complete**

actual Wagon current checkpoint:

```text
Builder Step 7 Apply / Save = 완료
Builder Step 8 PIE runtime 적용 = 정상 확인
Wheel Socket Scale FL/FR/RL/RR = (0.8,1.0,0.8)
Derived Front/Rear Wheel Radius = 약 40cm
Derived Front/Rear Wheel Width = 약 25cm
bUseWheelSocketScale = true
bAutoScaleWheelMeshToRadius = false
Shared Wheel FL + FR/RL/RR fallback = persisted
```

fresh persisted `DA_Vehicle_Wagon` AssetDump에서도 4개 WheelAnchor RelativeScale `(0.8,1.0,0.8)`, SocketScale authority true, AutoScale false, Front/Rear Radius `39.9996cm` 계열, shared `Wheel_FL` binding, Front/Rear WheelClass와 Layout override 적용을 확인했다.

남은 USER Gate는 WSA 자체에 한정한다.

```text
1. 타이어가 지면에 자연스럽게 닿는가 — 과도한 파묻힘/뜸 없음
2. 80cm 직경 타이어가 Wagon 휠하우스와 시각적으로 적절한 비율인가
3. 실제 주행에서 Wheel Size 불일치로 보이는 비정상 접지/휠 시각-물리 괴리가 없는가
```

위 3개를 USER가 PASS하기 전에는 WSA-P0-07을 닫지 않는다. WSA와 무관한 Drivetrain/변속 tuning 이슈는 이 Gate의 blocker로 포함하지 않는다.

USER PIE에서 **FL-only shared Wheel fallback의 Right side orientation 결함**을 발견했다. pre-remediation runtime은 FR/RL/RR null일 때 FL StaticMesh reference만 재사용했고 Right side(FR/RR) orientation compensation과 per-wheel spin handedness가 없었다. 기존 `RuntimeVisualFallback`도 shared FL mesh reference, Socket Scale, WheelSync transform preservation만 검증해 Right orientation/spin direction을 놓쳤다.

이 결함은 WSA의 `공용 Wheel Mesh 1개 재사용` 핵심 계약 blocker로 분류했고, 아래 WSA-P0-07A remediation을 구현/검증해 **Technical blocker를 해제했다.** P0-07 자체는 actual Wagon USER PIE 재확인 전까지 닫지 않는다.

#### WSA-P0-07A — Right Fallback Orientation + Spin Remediation

상태: **Technical PASS / USER PIE Recheck Ready**

##### 1. Root Cause Audit

pre-remediation runtime fallback은 `ACFVehiclePawn::ApplyVehicleWheelVisualConfig()`에서 다음처럼 Mesh reference만 resolve했다.

```text
FL = WheelMeshFL
FR = WheelMeshFR가 null이면 FL
RL = WheelMeshRL가 null이면 FL
RR = WheelMeshRR가 null이면 FL
```

이 경로에는 `FR/RR가 FL fallback인지`에 따른 orientation compensation이 없다. 따라서 좌측용 canonical `Wheel_FL`을 Right slot에 그대로 사용하면 outside/inside 방향이 뒤집힌다.

pre-remediation `UCFWheelSyncComp`는 spin에 `WheelSpinVisualSign * WheelSpinMeshAxisSign` global sign만 사용했고 per-wheel handedness가 없었다. remediation 이후 Pawn의 source-aware fallback resolve가 handedness를 전달하고 WheelSync가 실제 적용 단계에서 소비한다.

기존 `RuntimeVisualFallback`은 Mesh reference fallback, Socket Scale, Steering/Suspension/Spin 적용 여부와 Scale 보존만 검증했다. **Right orientation / right spin handedness / explicit Right Mesh 예외 / repeated runtime re-init idempotency**는 검증하지 않았다.

##### 2. Ownership Contract

수정 소유권은 다음으로 고정한다.

```text
VehicleData / Recipe / Builder / Resolver
= 기존 FL-only fallback schema 유지
= 신규 persistent field 없음

ACFVehiclePawn
= current Wheel Visual runtime integration owner 유지
= 각 slot이 explicit Mesh인지 FL fallback인지 source-aware resolve
= Right fallback Mesh orientation compensation과 authored base full relative transform cache를 Wheel Visual 전용 private seam에 한정
= WheelSync에 per-wheel spin handedness 전달
= 이번 작업에서 신규 Component/대규모 Pawn decomposition은 시작하지 않음

UCFWheelSyncComp
= Pawn이 전달한 per-wheel spin handedness를 visual spin sign에 결합
= steering / suspension / runtime spin 기존 소유권 유지

Future extraction guard
= Wheel fallback/orientation 관련 새 로직을 Pawn 여러 함수에 분산하지 않음
= Wheel Visual 전용 private helper/state로 응집해 향후 별도 owner로 기계적 이동 가능하게 유지
= 신규 persistent schema나 Blueprint 수동 보정으로 책임을 우회하지 않음

BP_CFVehiclePawn
= hierarchy 유지
= FR/RR에 고정 180도 보정값을 Asset로 박지 않음
= 수동 per-vehicle 보정 작업 없음
```

BP에 FR/RR 고정 회전을 저장하지 않는 이유는 explicit `WheelMeshFR/RR`가 이미 Right용으로 제작된 Mesh일 수 있기 때문이다. fallback source를 모르는 BP default는 source-aware 판단을 할 수 없다.

##### 3. Right Orientation Compensation

CarFight canonical Wheel 좌표 계약은:

```text
Vehicle Forward = +X
Wheel Axle = +Y
Up = +Z
Wheel spin = local Pitch / Y axis
```

FL Mesh가 Left에서 정상 방향이라는 USER PIE evidence를 기준으로 Right fallback은 **Mesh child local Roll(X) 180°** compensation을 사용한다.

```text
Left FL/RL fallback
= authored base orientation 그대로

Right FR/RR + FL fallback
= authored base orientation × Local Roll 180°
```

Roll 180을 선택하는 이유:

- +X vehicle-forward 의미를 보존한다.
- 좌우 방향인 local Y를 반전해 Left용 side face를 Right side로 돌린다.
- negative Y scale을 사용하지 않아 normal/tangent/culling/negative-transform handedness 부작용을 피한다.
- Wheel_Anchor를 돌리지 않아 steering yaw와 suspension/layout 축을 건드리지 않는다.

보정은 `Wheel_Anchor_*`가 아니라 child `Wheel_Mesh_*`에만 적용한다.

##### 4. Authored Base Full Transform Preservation / Idempotency

runtime re-initialize에서 이전 VehicleData가 변경한 `Wheel_Mesh_*`의 Rotation/Location/Scale이 다음 차량에 남으면 안 된다. 특히 Legacy AutoScale/AutoCenter는 RelativeScale과 RelativeLocation을 수정하고, SocketScale/Manual 경로는 그 모든 값을 항상 다시 쓰지 않으므로 **Rotation-only cache로는 hot-reinit determinism이 성립하지 않는다.**

`ACFVehiclePawn`은 Pawn instance lifetime에서 최초 Wheel Visual mutation 전에 각 `Wheel_Mesh_*`의 **BP-authored base relative transform(Location/Rotation/Scale)** 전체를 Wheel Visual 전용 private state에 한 번 캡처한다.

current private cache 개념:

```text
WheelVisualAuthoredBaseTransforms[FL/FR/RL/RR]
```

매 Apply 시 먼저 authored base full transform을 복원하고 그 위에 현재 VehicleData만 적용한다.

```text
Authored Base Transform restore
→ optional Right fallback Roll180
→ current Scale authority(Socket / Legacy AutoScale / Manual)
→ optional Legacy AutoCenter
```

Right orientation 합성은 Euler 단순 덧셈이 아니라 Quaternion local composition을 사용한다.

그 결과:

```text
Fallback Apply 반복 → 같은 orientation
VehicleData hot re-init → 이전 Location/Scale 잔류 0
Legacy AutoScale/AutoCenter → SocketScale → authored location 복원 + current SocketScale 적용
SocketScale → Manual → authored scale/location 복원
Fallback → explicit FR/RR 전환 → authored base transform 복원
explicit → fallback 전환 → 정확히 1회 Right compensation
```

`OnConstruction()`은 `Super::OnConstruction()` 뒤 기존 Wheel Visual authored cache를 invalidate한다. 실제 `BP_CFVehiclePawn` Wheel components가 SCS-owned이므로 Construction rerun에서는 현재 SCS authored transform을 fresh recapture하고, runtime VehicleData hot-reinit에서는 같은 authored base cache를 유지한다.

##### 5. Source-aware Rule

Right compensation은 pointer equality가 아니라 **source 상태**로 결정한다.

```text
WheelMeshFR == null && WheelMeshFL != null
→ FR uses FL fallback
→ Right compensation ON
→ Right spin handedness -1

WheelMeshRR == null && WheelMeshFL != null
→ RR uses FL fallback
→ Right compensation ON
→ Right spin handedness -1

WheelMeshFR explicit non-null
→ compensation OFF
→ spin handedness +1

WheelMeshRR explicit non-null
→ compensation OFF
→ spin handedness +1
```

explicit FR/RR가 우연히 FL과 같은 StaticMesh pointer를 가리켜도 **explicit source이면 자동 반전하지 않는다.** P0에서 explicit Mesh의 authored orientation은 해당 slot authoring 책임으로 둔다.

RL은 Left slot이므로 FL fallback이어도 compensation이 없다.

##### 6. Per-wheel Spin Handedness

Right fallback Roll 180은 local Y spin axle 방향도 반전한다. 따라서 visual spin은 global sign에 per-wheel handedness를 추가한다.

```text
GlobalSpinSign
= WheelSpinVisualSign
  × WheelSpinMeshAxisSign

PerWheelVisualSpinSign
= GlobalSpinSign
  × WheelSpinHandedness[WheelIndex]
```

기본값:

```text
FL = +1
FR = -1 only when FL fallback compensation is active
RL = +1
RR = -1 only when FL fallback compensation is active
```

이 multiplier는 persistent VehicleData가 아니라 **runtime resolved visual state**다. `ACFVehiclePawn`의 Wheel Visual 전용 resolve 경계가 fallback source를 판정한 직후 `UCFWheelSyncComp`에 전달한다.

WheelSync가 StaticMesh pointer equality로 fallback 여부를 재추론하지 않는다. explicit same-pointer Mesh와 null fallback을 구분할 수 없기 때문이다.

##### 7. Spin Rotation Composition

production runtime의 `AddLocalRotation(SpinPitchDelta)`는 local spin 축을 사용하므로 유지한다. 다만 compensated base rotation에서도 absolute/debug spin path가 Euler `Pitch +=`에 의존하지 않도록 다음 계약으로 정리한다.

```text
AbsoluteTargetRotation
= BaseWheelMeshRotationQuat
  × LocalPitchSpinQuat
```

즉 base Roll 180과 local Pitch spin을 Quaternion으로 합성한다. production delta path와 absolute/debug path가 같은 local-axis 의미를 갖게 한다.

##### 8. 변경 범위

구현 시 예상 파일 범위:

```text
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFWheelSyncComp.cpp
UE/Source/CarFight_Re/Public/CFWheelSyncComp.h
UE/Source/CarFight_Re/Private/CFWheelSizeTests.cpp
Tools/RunWheelSizeTests.ps1
```

Pawn 수정은 이번 WSA 기능에 필요한 최소 범위만 허용한다. Right fallback source 판정, authored base full-transform cache, Construction invalidation, orientation compensation을 Wheel Visual 전용 private helper/state로 응집하고 다른 Pawn 도메인과 섞지 않는다. 목적은 지금 Pawn 축소 작업을 시작하는 것이 아니라, 향후 Pawn 축소 시 이 Wheel Visual 블록을 별도 owner로 옮기기 쉽게 만드는 것이다.

변경하지 않는 범위:

```text
VehicleData schema
Recipe schema
Field Registry / Resolver
Builder Step contract
Wagon Socket Scale / Radius / Width
BP_CFVehiclePawn asset hierarchy/default transform
Wheel_FL StaticMesh asset
Chaos Wheel physics setup
```

##### 9. Focused Regression Design

신규 exact test 권장명:

```text
CarFight.Vehicle.WheelSize.WSA_P0_07.RightFallbackOrientationSpin
```

필수 검증:

```text
A. FL-only fallback
- FL/RL authored base orientation 유지
- FR/RR = authored base × Roll180
- 4개 Mesh reference는 FL shared mesh
- Socket Scale exact 유지

B. idempotency
- ApplyVehicleWheelVisualConfig 두 번 실행
- FR/RR orientation이 180→360으로 누적되지 않음

C. explicit Right override
- FR/RR explicit non-null
- 자동 Right compensation 없음
- explicit가 FL과 같은 pointer여도 source가 explicit이면 compensation 없음

D. spin handedness
- FL/RL multiplier +1
- fallback FR/RR multiplier -1
- 같은 forward spin 입력에서 차량 기준 실제 굴림 방향이 좌우 일치

E. WheelSync ownership
- steering yaw 정상
- suspension 정상
- local spin 정상
- Right compensation 유지
- Socket Scale 유지

F. re-init transition
- fallback → explicit → fallback 재초기화
- authored base full transform이 정확히 복원/재적용
- Legacy AutoScale/AutoCenter → SocketScale에서 stale center location 0
- SocketScale → Manual에서 stale scale/location 0
- Construction cache invalidation 뒤 fresh authored transform 재캡처
- FR/RR absolute + production delta spin handedness 직접 assertion
```

현재 WSA focused suite 10개에 신규 exact test 1개를 추가해 **11 exact tests**로 affected regression을 구성한다. broad DataAuthoring replay는 새 failure가 없으면 수행하지 않는다.

##### 10. Acceptance Gate

Technical PASS 조건:

```text
Official Editor Build PASS
신규 RightFallbackOrientationSpin PASS
기존 WSA affected exact 10 tests PASS
Wagon persisted size authority 변경 0
Wagon/Blueprint/StaticMesh Asset mutation 0
```

그 뒤 USER PIE에서 actual Wagon으로:

```text
FR/RR 외측 방향 정상
전진 시 FL/FR/RL/RR가 시각적으로 같은 굴림 방향
후진 시 네 바퀴 모두 반전 정상
조향 중 FR 방향/스핀 정상
Socket Scale 0.8/1.0/0.8 및 지면 접촉 유지
```

를 USER PIE에서 확인하면 WSA-P0-07을 USER PASS로 닫는다.

2026-09-01 actual Wagon USER 재확인 결과:

```text
Wheel 위치/장착 상태 = 좋음
Right side 방향 = 정상
주행 시 굴림 방향 = 정상
Wheel 회전 = 정상
USER 판정 = PASS
```

따라서 Right fallback orientation/spin blocker는 USER 기준으로도 해제됐고 WSA-P0-07은 최종 PASS다.

Current technical remediation evidence after post-review correction:

```text
Official Editor Build
= d29ed62e7e0940bbb2e02da41e37f64f
= Succeeded / Exit 0

WSA focused affected regression
= e3c3575fc6fc42159690082857705a49
= exact 11/11 PASS / Exit 0

RightFallbackOrientationSpin
= PASS
= Right Roll180 / repeated Apply idempotency / explicit same-pointer override
= authored Location/Rotation/Scale full-transform restoration PASS
= Legacy AutoCenter → SocketScale stale location removal PASS
= SocketScale → Manual stale scale/location removal PASS
= Construction authored-cache fresh recapture PASS
= FR/RR absolute spin handedness PASS
= FR/RR production delta AddLocalRotation handedness PASS
= fallback → explicit → fallback PASS
```

이 evidence는 v0.1.17의 Rotation-only post-remediation evidence를 supersede한다.

구현은 `CFVehiclePawn.cpp/.h`의 Wheel Visual 전용 private seam과 `CFWheelSyncComp.cpp/.h`의 runtime handedness 계약으로 제한했다. VehicleData/Recipe/Builder schema, Wagon Asset, `BP_CFVehiclePawn`, `Wheel_FL` StaticMesh는 이 remediation에서 변경하지 않았다.

첫 affected-suite 실행 `be8bbde85391494199d6a389b5e0b527`은 `BuilderShell` 실행 중 UE Experimental `ModelContextProtocol` 플러그인의 비동기 `server/discover` Error 로그가 현재 Automation에 귀속돼 환경성 Fail이 발생했다. Product/WSA Source에는 해당 호출이 없음을 확인했고 `RunDataAuthoringTests.ps1`에 기본 false인 opt-in log suppression을 추가해 WSA runner에서만 사용했다. 재실행한 exact 11 tests는 모두 PASS했다.

---

## 19. 완료 조건

이 Plan의 P0 완료는 다음을 모두 만족해야 한다.

```text
공용 Wheel Mesh 재사용 가능
차량별 Wheel Mesh 크기 복사본 불필요
USER Socket Scale이 명시 Authority
StaticMesh Bounds read 성공
Visual Wheel Scale exact 적용
Physics Radius/Width 동일 source derived
좌우 axle consistency 검증
AutoScale 이중 적용 0
WheelSync 회귀 0
Builder Step 2~5/7 source contract 일치
실제 신규 차량 USER Visual + Driving PASS
```

2026-09-01 WSA-P0-07 USER PASS 뒤 현재 구현 계약을 `Document/Systems/Vehicles/VehicleData.md`, `VehicleRuntime.md`, `WheelSync.md`에 승격했다. 이 Plan은 이후 **Historical evidence owner**로 사용하며, 새 WSA 결함이 없는 한 완료된 Build/Automation/USER evidence를 반복하지 않는다.

---

## 20. Changelog / Migration

### v0.1.19 - 2026-09-01

- actual Wagon USER PIE에서 `Wheel 위치 좋음 / Right 방향 정상 / 굴림 방향 정상 / 회전 정상`을 확인하고 USER가 명시적으로 PASS했다.
- WSA-P0-07을 USER PASS로 닫고 Wheel Size Authority P0를 Complete로 전환했다. Right-side FL fallback orientation/spin blocker는 Technical + USER 양쪽에서 해제됐다.
- current 계약을 `Document/Systems/Vehicles/VehicleData.md`, `VehicleRuntime.md`, `WheelSync.md`에 승격했다. 본 Plan은 Historical evidence owner로 유지한다.
- CF-FQ-040의 다음 작업은 WSA가 아니라 Vehicle Builder lane이다. 기존 Resolver WSA adoption fingerprint mismatch는 완료된 WSA evidence를 기준으로 fresh refresh/re-resolve한 뒤 actual Wagon apply를 재개해야 하며, drivetrain/Transmission 작업과 WSA 완료 상태를 다시 혼합하지 않는다.

### v0.1.18 - 2026-09-01

- post-review에서 Wheel Visual hot-reinit determinism gap을 발견해 authored cache를 Rotation-only에서 **Full RelativeTransform(Location/Rotation/Scale)**으로 확장했다.
- 매 `ApplyVehicleWheelVisualConfig()`에서 authored full transform을 먼저 복원한 뒤 Right fallback orientation → current scale authority → Legacy AutoCenter 순으로 적용해 `Legacy→Socket`, `Socket→Manual`, fallback/explicit 전환에서 이전 차량 transform 잔류를 제거했다.
- 실제 `BP_CFVehiclePawn`의 `Wheel_Mesh_*`가 SCS-owned임을 확인해 `OnConstruction()`에서 cache invalidate → current SCS authored transform fresh recapture 계약을 추가했다. runtime hot-reinit은 같은 authored base를 유지한다.
- `RightFallbackOrientationSpin`에 authored Location/Rotation/Scale, Legacy→Socket→Manual transition, Construction recapture, RR absolute/delta handedness assertion을 추가했다.
- Official Build `d29ed62e7e0940bbb2e02da41e37f64f` Succeeded/Exit0, WSA focused process `e3c3575fc6fc42159690082857705a49` exact 11/11 PASS/Exit0이다. 이 evidence가 v0.1.17의 이전 Technical evidence를 supersede한다.
- `RunDataAuthoringTests.ps1 v1.2.1`은 opt-in MCP log suppression 사용 여부를 process output marker로 명시해 focused evidence 실행 조건을 추적 가능하게 했다. Product 설정/WSA Runtime 동작은 변경하지 않는다.
- WSA-P0-07은 계속 USER Reacceptance Ready이며 다음 Gate는 actual Wagon PIE의 FR/RR 방향, 전진/후진 spin, 조향, Socket Scale/지면 접촉 확인이다. Drivetrain/변속 tuning은 계속 별도 lane이다.

### v0.1.17 - 2026-08-31

- WSA-P0-07A Right-side FL fallback orientation/spin remediation을 구현하고 Technical PASS로 닫았다. FR/RR null fallback만 authored base 기준 local Roll180을 적용하고 explicit Right Mesh는 같은 pointer여도 자동 반전하지 않는다.
- `ACFVehiclePawn`은 Wheel Visual 전용 private seam에서 authored base rotation을 1회 캡처하고 repeated Apply/re-init 시 deterministic하게 orientation을 재구성한다. Legacy AutoCenter도 최종 Right orientation 확정 뒤 계산되도록 적용 순서를 보강했다.
- `UCFWheelSyncComp`에 persistent schema가 아닌 runtime per-wheel handedness를 추가하고 absolute/debug spin은 BaseQuat × LocalPitchQuat, production delta spin은 handedness 보정 local AddLocalRotation을 사용한다.
- 신규 `CarFight.Vehicle.WheelSize.WSA_P0_07.RightFallbackOrientationSpin`은 Right orientation, idempotency, explicit same-pointer override, absolute/delta spin handedness, fallback↔explicit 전환을 PASS했다.
- Official Build `2387faff9fec4f39841b59387000f2fe` Succeeded/Exit0, WSA focused process `ca30b1afe8534e869085fa88ba1f2a34` exact 11/11 PASS/Exit0이다.
- 첫 suite의 `BuilderShell` 환경성 Fail은 UE Experimental ModelContextProtocol 비동기 Error 로그 오염으로 분리했다. generic runner에는 기본 false opt-in suppression만 추가하고 WSA suite에서만 사용했다.
- WSA-P0-07은 기술 blocker가 해제되어 USER Reacceptance Ready로 전환한다. 다음은 actual Wagon PIE에서 FR/RR 방향, 전진/후진 spin, 조향 중 FR, Socket Scale/지면 접촉 USER 확인이다. Drivetrain/변속 tuning은 계속 별도 lane이다.

### v0.1.16 - 2026-08-31

- 사용자 의도를 재확인해 별도 Vehicle Pawn decomposition 착수와 신규 `UCFVehicleWheelVisualComp` 즉시 신설 전제를 철회했다.
- 이번 WSA remediation은 current `ACFVehiclePawn` integration을 유지하되 Right fallback source, authored base rotation cache, orientation compensation을 Wheel Visual 전용 private seam에 응집해 향후 Pawn 축소 시 기계적으로 추출하기 쉬운 구조로 구현한다.
- WSA 범위에서는 신규 persistent schema/BP 수동 보정/대규모 Pawn 재구성을 하지 않고, `UCFWheelSyncComp`에는 필요한 per-wheel handedness 계약만 추가한다.

### v0.1.15 - 2026-08-31

- `CFVehiclePawn.cpp` 6,686줄 / `CFVehiclePawn.h` 1,935줄의 비대화를 별도 구조 문제로 감사하고 `Document/Plan/Architecture/VehiclePawnDecompositionPlan.md v0.1.0`을 상위 decomposition owner로 추가했다.
- WSA-P0-07A 구현 소유권을 `ACFVehiclePawn` 직접 구현에서 신규 `UCFVehicleWheelVisualComp`로 교정했다. WheelVisualComp가 fallback source, Mesh/Scale apply, authored base rotation, Right compensation, per-wheel handedness를 소유하고 Pawn은 runtime orchestration/delegate만 유지한다.
- persistent VehicleData/Recipe/Builder/Resolver/BP Asset 계약은 변경하지 않는다. 이번 WSA remediation을 VPD-P0-01 Wheel Visual First Extraction의 첫 실제 ownership reduction으로 사용한다.

### v0.1.14 - 2026-08-31

- WSA-P0-07 Right-side FL fallback orientation/spin blocker의 source/runtime ownership을 재감사하고 Remediation Design PASS로 확정했다.
- persistent schema/Builder/Resolver/BP Asset 변경 없이 `ACFVehiclePawn`이 null fallback source를 판정하고 Right fallback Mesh child에 local Roll(X) 180° compensation을 적용하며, `UCFWheelSyncComp`에 per-wheel spin handedness를 전달하는 구조로 고정했다.
- BP-authored base Mesh rotation을 최초 mutation 전에 캡처하고 Quaternion 합성으로 매 Apply를 deterministic하게 재구성해 repeated Apply와 VehicleData re-init에서 orientation 누적을 금지한다.
- explicit FR/RR non-null은 pointer가 FL과 같더라도 source-aware explicit로 취급해 자동 반전을 적용하지 않는다. fallback FR/RR만 handedness -1, FL/RL 및 explicit Right는 +1을 사용한다.
- production delta spin은 local rotation을 유지하고 absolute/debug spin도 BaseQuat × LocalPitchQuat으로 정합화하는 설계를 추가했다.
- 신규 focused regression `WSA_P0_07.RightFallbackOrientationSpin`을 추가해 fallback orientation, idempotency, explicit override, per-wheel spin, WheelSync ownership, fallback↔explicit re-init을 검증하도록 설계했다. 구현 전 상태는 `Design PASS / Implementation Ready`다.

### v0.1.13 - 2026-08-31

- WSA-P0-07 USER PIE에서 FL-only shared Wheel fallback의 Right side orientation 결함을 발견했다. current `ApplyVehicleWheelVisualConfig()`은 FR/RL/RR null일 때 FL Mesh reference만 그대로 재사용하고 Right side orientation compensation을 하지 않는다.
- current `UCFWheelSyncComp`의 visual spin sign은 `WheelSpinVisualSign * WheelSpinMeshAxisSign` global 값뿐이라 Right side/fallback orientation에 따른 per-wheel spin handedness 보정이 없다.
- 기존 `RuntimeVisualFallback` regression은 fallback Mesh/Scale/WheelSync preservation만 검증하고 Right orientation/spin direction을 커버하지 않아 regression gap으로 기록했다.
- 본 결함은 shared Wheel 1개 재사용이라는 WSA 핵심 계약의 실제 USER blocker이므로 WSA-P0-07을 Blocked로 전환했다. unrelated Drivetrain/변속 tuning은 계속 WSA blocker에서 제외한다.

### v0.1.12 - 2026-08-31

- actual Wagon Builder Step 7 Apply/Save 완료와 Step 8 PIE runtime 적용 정상 확인을 WSA-P0-07 current checkpoint로 반영했다.
- fresh persisted `DA_Recipe_Wagon`은 AppliedRecipeRevision 10, `SocketScaleFromChassis`, shared `Wheel_FL`, Radius/Width 4건 adoption과 private Profile 4종 binding을 보유한다. fresh persisted `DA_Vehicle_Wagon`은 4개 WheelAnchor Scale `(0.8,1.0,0.8)`, `bUseWheelSocketScale=true`, `bAutoScaleWheelMeshToRadius=false`, Front/Rear Radius `39.9996cm` 계열, Layout override와 Front/Rear WheelClass 적용 상태를 확인했다.
- P0-07 남은 Gate를 USER Visual + Driving acceptance로 한정했다. 지면 접촉, 휠하우스 비율, Wheel Size로 인한 시각/물리 주행 괴리만 WSA blocker로 판정하며 다른 차량 tuning 문제는 WSA Gate에 혼합하지 않는다.
- WSA-P0-07은 USER 판정 전까지 In Progress를 유지한다.

### v0.1.11 - 2026-08-28

- WSA-P0-05/06 post-review P2 교정을 적용했다. deferred Apply는 Vehicle Base/Drivetrain/Handling/Performance 4개 exact Profile 누락 + Front/Rear WheelClass 누락에서만 허용하며 DriveState 대체, duplicate, existing Profile binding, 예상 밖 blocker는 fail-closed한다.
- `CarFight.DataAuthoring.CF_FQ_040.WSA_P0_05.Migration.DeferredApplyGuard` negative regression을 추가하고 WSA focused suite를 10 exact tests로 확장했다.
- `CFWSAMigrateCmdlet.h/.cpp` 실행 계약 설명을 current Recipe-only Step 2~4 defer / Apply-ready Definition Apply 경계와 동기화했다.
- `RunDataAuthoringTests.ps1 v1.1.0`에서 `Start-Process -Wait` descendant-process 대기를 제거하고 exact UnrealEditor process `WaitForExit()`를 사용하도록 교정했다. 새 focused process `d63625118ad94599a56e4151bb9b6dc6`가 10/10 PASS / Exit0으로 정상 terminal 회수됐다.
- current final C++ source Official Build `e2cfa435b5764dc282ec2b2e973175b6`가 Succeeded / Exit0이다. Wagon migration/Asset Save는 post-review에서 재실행하지 않았으며 P0-06 Technical PASS / P0-07 USER Acceptance Ready 상태를 유지한다.

### v0.1.10 - 2026-08-28

- WSA-P0-06 Technical Validation을 PASS로 닫았다. P0-05 final source 직후 수행한 Official Build `05115a84e3c34c4c80efe1695f998174` Exit0과 focused process `798f21a197254d82932a5ab0b8f31868` exact 9/9 PASS를 중복 재실행 없이 사용했다.
- focused suite는 Registry/Legacy/Socket Resolver/Object reference roundtrip/Runtime visual fallback/Builder/Gameplay/ProfileCommit/Validator를 모두 포함한다.
- `RuntimeVisualFallback`이 transient World의 실제 `ACFVehiclePawn`에서 shared FL fallback, exact Wheel_Mesh Scale, Wheel_Anchor scale 비소유, WheelSync steering/suspension/spin 및 Scale 보존을 실행하므로 fresh Runtime technical check를 충족한다고 판정했다.
- persisted AssetDump의 Wagon Recipe migration truth와 Target unapplied baseline을 함께 보존했다. 실제 Wagon Product Target의 Visual/Driving USER PASS로 확대하지 않는다.
- 다음 Gate는 WSA-P0-07 USER Acceptance이며 actual Wagon 후속 Builder Profile/WheelClass 구성과 Step 7 Definition Apply 뒤 PIE에서 USER가 지면 접촉/휠하우스 비율/주행을 확인한다.

### v0.1.9 - 2026-08-28

- WSA-P0-05 Legacy Migration을 Technical PASS로 닫았다. AutoScale=true 3개 자산은 USER Socket Scale evidence가 없거나 test/historical fixture라 자동 전환하지 않고, SUV/Police legacy path도 보존한다.
- actual Wagon은 USER-authored Socket Scale `(0.8,1.0,0.8)`과 canonical shared Wheel evidence가 있어 Category A로 확정했다. `DA_Recipe_Wagon`에 shared Wheel FL, `SocketScaleFromChassis`, Socket-derived Radius/Width 4건의 reviewed adoption을 persisted 저장했다.
- actual Wagon은 아직 Builder Step 2~4 상태라 VehicleBase/Drivetrain/Handling/Performance Profile과 Front/Rear Wheel Class가 미지정이다. 이 후속-Step blocker를 WSA가 임의 채우거나 Validator 우회하지 않고 `DA_Vehicle_Wagon` Definition Apply를 정상 Step 7까지 defer했다.
- Recipe SoftObject→Target hard Object projection의 canonical text 불일치로 R15 Definition hash mismatch가 발생하던 공용 Resolver gap을 `CFVehicleResolver.cpp v1.5.2`에서 교정하고 `ObjectReferenceRoundTrip` regression을 추가했다.
- Official Build `05115a84e3c34c4c80efe1695f998174` Exit0, focused process `798f21a197254d82932a5ab0b8f31868` exact 9/9 PASS, Wagon migration `c66f7579d3114ba3afb29502f7f5dd6c` Exit0을 확인했다.
- persisted AssetDump에서 Recipe mode/binding/adoption 저장과 Target `bUseWheelSocketScale=false / Radius30 / Width6 / LayoutOverride=false` 미적용 상태를 각각 확인했다. 다음 Gate는 WSA-P0-06 Technical Validation이다.

### v0.1.8 - 2026-08-28

- WSA-P0-03 Runtime Visual / Mesh Fallback을 Technical PASS로 정합화했다. USER fresh Build PASS와 focused process `304cd2dbfaed46cda7a1db7c0da9feb9`에서 exact 4 tests PASS, Wheel_Mesh exact Socket Scale/FL fallback/WheelSync scale 보존을 확인했다.
- WSA-P0-04 Validator / Builder Integration을 구현했다. canonical 100x25x100 Bounds, Socket Scale/axle validation, Step4 Scale stale, mode-aware Gameplay Guidance, Socket-owned wheel geometry Builder guard, Runtime Validator authority conflict를 추가했다.
- P0-04 검증 중 Resolver가 FL-only shared Wheel 계약을 따르지 않는 실제 integration gap을 발견해 FR/RL/RR missing 시 FL Snapshot fallback으로 교정하고 Resolver focused regression을 추가했다.
- invalid Socket Scale에서 전체 Preview Blocked 때문에 Step2=Stale/Step3=Locked로 실제 원인이 가려지던 Builder UX를 교정했다. fresh Resolve read 자체의 currentness를 Apply freshness와 분리해 Step2~4만 최신 Asset/Socket truth를 진단한다.
- Official Build `6215416c66304a048ba83cbe1d5a8ca2` Exit0, focused process `30add52f8ba94f4b8f206356e7c33faa`에서 WSA + affected exact 8 tests 모두 PASS했다.
- WSA-P0-04 Technical PASS, next WSA-P0-05 Legacy Migration. USER 실제 Socket Scale 없는 차량의 임의 migration은 계속 금지한다.

### v0.1.7 - 2026-08-28

- WSA-P0-02를 구현했다. DataAsset/Pawn legacy capture가 StaticMeshSocket RelativeScale을 보존하고, R5가 4개 RelativeScale을 AssetDerived candidate로 올린다.
- `SocketScaleFromChassis` R3 semantic은 `bUseWheelSocketScale=true / bAutoScaleWheelMeshToRadius=false`를 명시한다.
- R6는 Socket mode에서 `CFWheelSizeUtils`를 재사용해 Wheel Bounds + USER Socket Scale로 Front/Rear Radius/Width를 derive한다. Legacy WheelMeshRadiusMeasureMode는 이 branch에서 사용하지 않는다.
- Wheel Size adoption fingerprint를 effective Wheel MeasureFingerprint + Socket names + Socket Scale만으로 좁혀 Location/Rotation 변경과 Size stale을 분리했다.
- 좌우 axle derived size 불일치, X/Z mismatch/invalid scale은 Blocked로 fail-closed한다. 기존 AssetAdoption lane과 `Measurement.WheelSocketScale.*.v1` source를 재사용한다.
- Resolver semantic 변화에 맞춰 ContractRevision을 3→4로 전진시켜 schema-only revision3 approval/receipt를 stale 처리한다.
- Official Build `1ebcc7850e3e4c83adbae3de3590d0ec` PASS, focused process `7be4965e5c804f79ae9b87355701d860`에서 P0-01 보호 2건 + P0-02 SocketScaleDerived 1건 모두 PASS했다.
- WSA-P0-02 Technical PASS, next WSA-P0-03 Runtime Visual / Mesh Fallback.

### v0.1.6 - 2026-08-28

- USER가 Editor를 안전 종료한 뒤 fresh Official Build `c74519c538404520b31896c77cfceba4`를 실행해 Runtime/Editor DLL Link 포함 완전 PASS를 확인했다.
- `Tools/RunWheelSizeTests.ps1` focused wrapper를 추가해 기존 `RunDataAuthoringTests.ps1`를 exact filter 두 건에만 재사용했다. broad DataAuthoring replay는 하지 않았다.
- `Registry132` 1/1, `SchemaUtilityLegacy` 1/1 PASS. legacy DA_TestSUV actual load에서 RelativeScale=OneVector, bUseWheelSocketScale=false, 기존 false path 보존을 확인했다.
- interactive editor.start의 UE MCP readiness 실패는 WSA 기능 검증과 분리하고 재시도 중단했다.
- WSA-P0-01 Technical PASS, next WSA-P0-02 Socket Resolver & Derived Physics.

### v0.1.5 - 2026-08-28

- WSA-P0-01 schema/helper 구현을 적용했다: WheelAnchor RelativeScale, bUseWheelSocketScale, append-only SocketScaleFromChassis, Registry 132, CFWheelSizeUtils와 focused Runtime test.
- 127→132 field/hash schema 변화가 기존 approval/receipt에 보이지 않는 호환성 문제를 재감사에서 발견해 ResolverContractRevision을 2→3으로 올렸다. revision 2 approval은 stale 처리한다.
- Official Build `6918bebc22074c33842a3f8701c0a851`에서 UHT PASS, WSA Runtime/Editor source compile PASS를 확인했다. 최종 Runtime/Editor DLL Link는 실행 중 UnrealEditor.exe 점유로 LNK1104, Exit 6이다.
- 기존 Editor는 Project Runtime reuse 상태이며 활성 다른 작업의 unsaved package 0을 전역 증명하지 못했으므로 자동 save0 종료하지 않았다.
- WSA-P0-01은 Technical PASS가 아니라 `Implementation Applied / Link + Automation Pending`으로 유지한다. Editor 안전 종료 뒤 fresh Build와 exact focused tests만 이어가며 Step1~8 broad replay는 하지 않는다.

### v0.1.4 - 2026-08-28

- 최종 Source 감사에서 `WheelVisualConfig.bUseWheelSocketScale`의 owner를 VehicleBase Profile이 아니라 `ProjectCompatibilityDefault=false + Recipe.WheelVisualIntent explicit`로 고정했다. path-specific dependency는 `Project.CompatibilityDefaults + Recipe.WheelVisualIntent`만 사용한다.
- current `ReadBuilderGameplayGuidance()`가 Base Profile AutoScale clamp를 mode와 무관하게 검사할 수 있음을 확인해, Socket/Manual mode에서는 Legacy clamp/MeasureMode/AutoCenter를 blocker로 사용하지 않는 mode-aware validation 계약을 추가했다.
- current Step 5 Physics Draft가 private VehicleBase `Data` 전체를 교체할 수 있음을 확인했다. `SocketScaleFromChassis`에서는 `bUseReferenceWheelGeometry=false`와 Front/Rear Radius/Width 4값을 baseline-preserve하고 AI Draft 변경을 fail-closed하도록 계약했다.
- 이로써 Wheel Size Authority의 USER Socket Scale → Asset-derived Radius/Width 경로와 AI Reference/Profile authoring 경로 사이의 shadow authority 충돌을 제거했다.
- Canonical shared Wheel `100×25×100`, narrow WheelSizeSourceFingerprint, Step4 Scale equality, enum append-only, Legacy option 분리 계약을 유지한다.
- 설계 감사 결과를 `Design Re-Audited PASS / Implementation Ready`로 전진했다. C++ 구현은 아직 시작하지 않았다.

### v0.1.3 - 2026-08-28

- USER가 실제 shared `Wheel_FL`로 교체/이동한 뒤 live `StaticMeshTools.get_bounds`를 재실행해 `99.9990 × 24.9992 × 99.9990 cm`, center≈0을 확인했다. Canonical Default Wheel `100 × 25 × 100` asset sub-gate를 PASS로 닫았다.
- fresh Git에서 canonical owner `UE/Content/CarFight/Vehicles/Shared/Tire/Wheel_FL.uasset`가 modified임을 확인했다. 별도 untracked Wagon/Wheel_FL은 잘못 작업된 비-owner asset으로 보존하며 자동 정리하지 않는다.
- `ECFWheelVisualIntentMode::SocketScaleFromChassis`는 기존 enum 값 뒤 append-only로 추가하도록 migration contract를 보강했다.
- Socket mode는 vehicle-specific Chassis Socket truth를 요구하므로 Base Profile에 별도 SocketScale policy를 추가하지 않고 Recipe `WheelVisualIntent`를 semantic owner로 고정했다.
- 기존 `bUseWheelVisualOverrides` DerivedGate를 재사용하고 Socket mode 전용 중복 gate는 만들지 않는다.
- v0.1.2 설계 감사 교정 사항인 narrow `WheelSizeSourceFingerprint`, Step 4 Scale equality, Legacy AutoScale/AutoCenter/Clamp 분리를 유지한다.
- C++/Resolver 구현은 아직 시작하지 않았다.

### v0.1.2 - 2026-08-28

- USER 결정에 맞춰 canonical Wheel source dimensions를 `100 × 25 × 100 cm`로 확정하고, `SocketScaleFromChassis`에 참여하는 모든 Wheel Mesh가 이 규격을 만족하도록 계약을 강화했다.
- USER가 Wheel_FL 크기 조정을 완료했다고 보고했지만 fresh live `StaticMeshTools.get_bounds`는 아직 old `79.358 × 35.480 × 79.358 cm`를 반환하고 Git에도 Wheel_FL persisted 변경이 관측되지 않아 canonical Asset verification을 Pending으로 유지했다.
- 전체 `ChassisLayoutFingerprint`를 Radius/Width measurement fingerprint에 넣으면 unrelated Hardpoint 변경도 Wheel Size를 Stale 처리하는 과잉 dependency를 발견했다. per-wheel `WheelSizeSourceFingerprint = effective Wheel MeasureFingerprint + Wheel Socket name + RelativeScale + rule revision`으로 축소했다.
- Layout stale과 Wheel Size stale을 분리해 Wheel Socket Location/Rotation 변경은 Layout만 Stale, Scale/Mesh 변경은 Wheel Size adoption도 Stale하도록 설계를 교정했다.
- current Builder Step 4 direct equality가 Location/Rotation만 비교하는 Source gap을 확인해 `RelativeScale` exact equality를 구현 의무로 추가했다.
- Runtime call order가 Wheel Visual Scale → Layout → WheelSync base capture 순이고 WheelSync가 Scale을 소유하지 않음을 확인해 Scale ownership 충돌이 없음을 감사했다.
- Socket mode에서는 Legacy `WheelMeshRadiusMeasureMode`, AutoCenter, ScaleClamp를 사용하지 않고 canonical centered Wheel Mesh + exact Socket Scale만 사용하도록 고정했다.
- C++/UE Asset 자동 mutation은 수행하지 않았다.

### v0.1.1 - 2026-08-28

- 신규 Socket mode의 radial axis를 X/Z로 고정하고 Legacy `WheelMeshRadiusMeasureMode`를 새 계산에서 제외했다. X/Z circular tolerance PASS 뒤 두 radial extent 평균으로 BaseRadius를 만들도록 계산 계약을 명확히 했다.
- current Field Registry/Reflection exact 127 leaf를 확인했고 새 RelativeScale 4 + bUseWheelSocketScale 1로 예상 132 leaf가 되는 schema 영향 범위를 WSA-P0-01 검증 항목으로 추가했다.
- Source 재감사로 `FCFVehicleAssetReader`가 이미 `FCFVehicleSocketSnapshot.RelativeScale`을 읽고 `ChassisLayoutFingerprint`에 Scale을 포함하며 Wheel Bounds/MeasureFingerprint도 보유함을 확인했다.
- 새 Scale snapshot/scanner/adoption store를 만들지 않고 existing AssetSnapshot + Resolver R6 Measurement Proposal + `FCFVehicleAssetAdoption`을 확장하는 설계로 축소했다.
- current `WheelBounds.Radius.v1 / WidthAxisY.v1` Legacy rule은 보존하고 Socket mode만 `WheelSocketScale.Radius.v1 / WidthAxisY.v1` combined fingerprint를 사용하도록 분기했다.
- current Resolver가 Layout Location/Rotation만 AssetDerived로 내보내는 gap을 확인해 `FCFWheelAnchorPose.RelativeScale` Registry leaf + Resolver candidate를 구현 의무로 추가했다.
- Editor semantic `ECFWheelVisualIntentMode::SocketScaleFromChassis` → Runtime `bUseWheelSocketScale=true / bAutoScaleWheelMeshToRadius=false` mapping을 정식 설계로 추가했다.
- Guided Builder persist는 direct raw capture가 아니라 existing Resolver → Diff → Step 7 DefinitionApply 단일 writer lane을 재사용하도록 명확히 했다.
- C++/UE Asset/Runtime mutation은 여전히 0이다.

### v0.1.0 - 2026-08-28

- USER 승인으로 Wheel Size Authority를 CF-FQ-040의 정식 하위 Plan으로 승격했다.
- 타이어 크기 디자인 Authority를 USER-authored Wheel Socket Scale로 고정했다.
- StaticMesh Bounds는 자동 디자인 입력이 아니라 실제 원본 치수 Source로 한정했다.
- 초기안에서는 공용 Default Wheel Mesh X/Z radial size를 100 cm canonical target으로 고정하고 Y는 실제 base width로 두었다. 이 Y 정책은 v0.1.2에서 USER 결정에 따라 25 cm canonical width로 대체됐다.
- Socket Scale에서 Visual Scale과 Physics Radius/Width를 같은 결과로 파생하는 계약을 정의했다.
- `FCFWheelAnchorPose.RelativeScale`, `bUseWheelSocketScale`, FL default mesh fallback, axle consistency, stale/validator 계약을 설계했다.
- 기존 `bAutoScaleWheelMeshToRadius`는 신규 정상 제작 경로에서 사용하지 않고 Legacy compatibility로 유지하도록 결정했다.
- Reference Tire Size는 Socket mode에서 USER 작성 sanity reference로 재분류하고 AI가 derived Radius/Width를 덮어쓰지 않도록 계약했다.
- WSA-P0-00을 Design PASS로 닫고 다음 Gate를 `WSA-P0-01 Data Schema & Shared Size Utility`로 고정했다.
- 이번 Design 작업의 C++/UE Asset/Runtime mutation은 0이다.
