# 타겟 선택 시스템 기존 구조 조사

- 문서 버전: v1.0.0
- 상태: TS-P0-00 완료
- 작성일: 2026-07-23
- 기능 ID: `CF-FQ-026`
- Task ID: `TS-P0-00`
- 상위 기획: `../Design/TargetSelect.md`
- 상세 작업계획: `TargetSelectPlan.md`
- 로드맵: `TargetSelectRoadmap.md`

---

## 1. 조사 목적

이 문서는 타겟 선택 시스템을 CarFight의 실제 C++ 클래스, Blueprint 에셋, Enhanced Input, HUD, 피해·파괴 상태와 충돌 구조에 연결하기 위한 구현 매핑 결과다.

조사 Task에서는 코드를 구현하거나 Blueprint를 수정하지 않는다. 이미 저장소에 존재하는 `CFTargetSelect` 계약을 유지한 상태에서 실제 소유 클래스, 재사용 경로, 신규 타입, C++과 Blueprint 책임, 충돌 전략과 P0 범위를 확정한다.

---

## 2. 조사 방법과 증거

다음 자료를 교차 확인했다.

### C++ 및 설정

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h
UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h
UE/Source/CarFight_Re/Public/CFVehicleHealthComp.h
UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
UE/Source/CarFight_Re/Public/CFCollisionChannels.h
UE/Config/DefaultEngine.ini
UE/Source/CarFight_Re/Public/CFTargetSelectTypes.h
UE/Source/CarFight_Re/Public/CFTargetSelectable.h
UE/Source/CarFight_Re/Public/CFTargetSelectData.h
UE/Source/CarFight_Re/Public/CFTargetSelectComp.h
UE/Source/CarFight_Re/Private/CFTargetSelectable.cpp
UE/Source/CarFight_Re/Private/CFTargetSelectData.cpp
UE/Source/CarFight_Re/Private/CFTargetSelectComp.cpp
```

### Unreal 에셋

```text
/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn
/Game/CarFight/Input/IMC_Vehicle_Default
/Game/CarFight/Input/IA_Fire
/Game/CarFight/Input/IA_LookAround
/Game/CarFight/UI/WBP_AimReticle
```

### 에셋 덤프 확인 결과

```text
- BP_CFVehiclePawn 부모 클래스: /Script/CarFight_Re.CFVehiclePawn
- BP_CFVehiclePawn은 비-DataOnly Blueprint
- BP_CFVehiclePawn 컴포넌트 수: 30
- BP_CFVehiclePawn 구현 인터페이스 수: 0
- WBP_AimReticle에 Image_CenterDot 존재
- WBP_AimReticle에 Image_WeaponReticle 존재
- IMC_Vehicle_Default에 기존 차량 및 전투 입력 존재
- IA_SelectTarget 에셋 없음
- IA_ClearTarget 에셋 없음
- TargetSelectComp 또는 TargetPoint 컴포넌트가 현재 BP에 존재하지 않음
```

차량 Blueprint 범위의 안전 덤프는 대상 3개를 모두 추출했다. Unreal commandlet 종료 로그의 MCP HTTP Listener 포트 바인딩 오류는 별도 도구 서비스 오류이며 조사 대상 에셋 덤프 결과에는 실패가 없었다.

---

## 3. 현재 차량 런타임 구조

`ACFVehiclePawn`은 현재 플레이어 차량 기능의 C++ 조립 루트다.

기본 서브오브젝트로 다음 컴포넌트를 생성한다.

```text
VehicleDriveComp
WheelSyncComp
VehicleCameraComp
VehicleAimComp
VehicleWeaponComp
ProjectilePoolComp
VehicleHealthComp
OwnerVisualRootComp
터렛 시각 컴포넌트 계층
```

입력, 런타임 초기화, Aim Reticle 생성과 차량 컴포넌트 접근 함수도 `ACFVehiclePawn`이 소유한다.

### 결정 TS-D00-01 — 선택 시스템 소유

```text
UCFTargetSelectComp는 ACFVehiclePawn의 C++ 기본 서브오브젝트로 추가한다.
```

근거:

- 현재 로컬 입력 소유 위치가 `ACFVehiclePawn`이다.
- `VehicleCameraComp`, `VehicleAimComp`, `VehicleWeaponComp`, `VehicleHealthComp` 접근이 가능하다.
- Pawn 교체와 `EndPlay` 시 선택 상태 정리 위치가 명확하다.
- 별도 PlayerController 또는 전역 Subsystem을 추가할 P0 요구가 없다.
- P0는 로컬 플레이어 차량 하나의 단일 선택 상태만 필요하다.

권장 이름:

```cpp
TObjectPtr<UCFTargetSelectComp> TargetSelectComp = nullptr;
```

권장 getter:

```cpp
UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|TargetSelect")
UCFTargetSelectComp* GetTargetSelectComp() const;
```

---

## 4. 선택 가능 대상 구현 위치

현재 `BP_CFVehiclePawn`은 구현 인터페이스가 없으며 `ICFTargetSelectable`도 연결되지 않았다.

### 결정 TS-D00-02 — 차량 선택 가능 계약

```text
ACFVehiclePawn이 ICFTargetSelectable을 C++에서 직접 구현한다.
BP_CFVehiclePawn에 같은 인터페이스를 중복 추가하지 않는다.
```

권장 클래스 선언:

```cpp
class CARFIGHT_RE_API ACFVehiclePawn
    : public AWheeledVehiclePawn
    , public ICFTargetSelectable
```

### `IsTargetSelectable` P0 규칙

```text
- Actor가 유효해야 한다.
- VehicleHealthComp가 존재해야 한다.
- VehicleHealthComp.IsDestroyed()가 false여야 한다.
- SelectionContext의 자기 선택 허용 정책을 따른다.
- 관계 및 분류 필터는 기존 UCFTargetSelectComp 규칙을 따른다.
```

### `GetTargetDisplayInfo` P0 규칙

```text
TargetCategory = Vehicle
Relation = Unknown
InformationLevel = Detected 또는 Identified
DisplayName = VehicleData에서 표시 이름을 제공하면 사용
FallbackDisplayName = Actor Label 또는 Actor Name
TargetId = Actor FName
```

현재 공용 진영·팀·관계 시스템은 확인되지 않았다. 타겟 선택 P0 안에서 임시 Faction 시스템을 새로 만들지 않는다.

### `GetTargetSelectionLocation` P0 규칙

```text
1. 대표 TargetPointComp가 있으면 해당 월드 위치
2. TargetPointComp가 없으면 Actor Bounds 중심
3. Bounds를 얻지 못하면 Actor 위치
```

### `GetTargetTrackState` P0 규칙

```text
기본값 = Visible
후속 가림 판정에서 Occluded 사용
Estimated와 SignalLost는 센서 단계에서 사용
```

---

## 5. 직접 조준과 무기 조준 경계

현재 프로젝트에는 서로 다른 두 조준 정보가 존재한다.

### `UCFVehicleCameraComp`

```cpp
FRotator GetCurrentAimRotation() const;
FVector GetCurrentAimDirection() const;
FVector GetCurrentAimHitLocation() const;
```

의미:

```text
플레이어 카메라와 Image_CenterDot이 가리키는 직접 조준 기준
```

### `UCFVehicleAimComp`

```cpp
FCFVehicleWeaponAimSolution GetWeaponAimSolution() const;
```

의미:

```text
Muzzle, 터렛 방향, 발사 원점과 무기 발사용 목표점을 포함한 실제 무기 조준 해
```

### 결정 TS-D00-03 — 직접 선택 기준

```text
직접 조준 후보 탐색은 UCFVehicleCameraComp의 카메라 Aim을 사용한다.
WeaponAimSolution과 CurrentMuzzleDirection은 직접 선택 Trace 기준으로 사용하지 않는다.
```

근거:

- 타겟 선택의 1순위는 크로스헤어에 조준된 대상이다.
- 터렛이 아직 정렬 중이어도 플레이어의 선택 의도는 중앙 크로스헤어를 따른다.
- 타겟 선택이 무기 발사 방향을 바꾸거나 무기 방향의 영향을 받으면 선택과 조준의 책임 경계가 무너진다.

### 후속 구현 세부 항목 TS-U00-01

현재 공개 API에는 Aim 방향과 적중 위치가 있지만 직접 Trace 시작 위치 전용 getter가 없다.

권장 보정:

```cpp
UFUNCTION(BlueprintPure, Category="CarFight|Vehicle Camera|Aim")
FVector GetCurrentAimTraceStartLocation() const;
```

이 함수는 `UCFVehicleCameraComp` 내부에서 현재 FollowCamera 또는 실제 Aim Trace 시작 위치를 반환한다. `UCFTargetSelectComp`가 Pawn의 카메라 컴포넌트 이름을 직접 검색하지 않도록 한다.

이 항목은 `TS-P0-03` 후보 탐색 구현 전까지 확정하면 되며 `TS-P0-01` 계약 검증을 차단하지 않는다.

---

## 6. 입력 구조

입력은 다음 함수에서 C++로 바인딩된다.

```cpp
ACFVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
```

Mapping Context:

```text
/Game/CarFight/Input/IMC_Vehicle_Default
```

런타임 등록:

```cpp
ACFVehiclePawn::RegisterDefaultInputMappingContext()
```

이 함수는 로컬 PlayerController와 `UEnhancedInputLocalPlayerSubsystem`을 안전하게 확인한 뒤 Mapping Context를 등록한다.

### 결정 TS-D00-04 — 입력 소유와 에셋

```text
- 선택 및 해제 입력 바인딩은 ACFVehiclePawn::SetupPlayerInputComponent에 추가한다.
- 기존 IMC_Vehicle_Default를 재사용한다.
- 별도 Target 전용 Mapping Context는 P0에서 만들지 않는다.
- IA_SelectTarget과 IA_ClearTarget을 신규 생성한다.
```

권장 C++ 속성:

```cpp
TObjectPtr<UInputAction> InputAction_SelectTarget = nullptr;
TObjectPtr<UInputAction> InputAction_ClearTarget = nullptr;
```

권장 핸들러:

```cpp
void HandleSelectTargetStarted(const FInputActionValue& InputActionValue);
void HandleClearTargetStarted(const FInputActionValue& InputActionValue);
```

### 후속 구현 세부 항목 TS-U00-02

키보드와 게임패드의 최종 키는 `IMC_Vehicle_Default` 전체 충돌표와 실제 조작 테스트 후 확정한다.

초기 후보:

```text
키보드/마우스 선택: MiddleMouseButton 또는 별도 키
키보드/마우스 해제: X 또는 별도 키
게임패드 선택: RightThumbstick 또는 사용하지 않는 FaceButton
게임패드 해제: 사용하지 않는 FaceButton 또는 길게 누르기
```

이 값은 기획 잠금값이 아니며 TS-P0-05에서 사용자 조작 기준으로 확정한다.

---

## 7. HUD 구조

현재 조준 UI는 다음 자산과 C++ 부모를 사용한다.

```text
/Game/CarFight/UI/WBP_AimReticle
UCFAimReticleWidget
```

확인된 주요 요소:

```text
Image_CenterDot
Image_WeaponReticle
조준 브래킷
FireFeedback
Cooldown
OutOfArc 경고
```

현재 UI 생성 방식은 HUD Manager나 Primary Layout이 아니라 `ACFVehiclePawn`이 로컬 Viewport에 직접 추가하는 방식이다.

```cpp
CreateWidget<UCFAimReticleWidget>()
AddToViewport(AimReticleZOrder)
```

### 결정 TS-D00-05 — 타겟 HUD 분리

```text
WBP_AimReticle에 후보와 선택 표시를 합치지 않는다.
별도 UCFTargetSelectWidget과 WBP_TargetSelect를 추가한다.
```

역할 분리:

```text
Image_CenterDot = 플레이어 직접 조준 위치
Image_WeaponReticle = 현재 터렛 방향
Target Candidate Marker = 선택 입력 시 선택될 현재 후보
Selected Target Marker = 이미 선택된 지속 대상
```

권장 Pawn 속성:

```cpp
TSubclassOf<UCFTargetSelectWidget> TargetSelectWidgetClass = nullptr;
TObjectPtr<UCFTargetSelectWidget> TargetSelectWidgetInstance = nullptr;
int32 TargetSelectWidgetZOrder = 11;
```

P0에서는 기존 Aim Reticle 생성 방식과 동일하게 로컬 Pawn이 별도 Target UI를 생성한다. 향후 공통 HUD Layer가 도입되면 두 위젯을 함께 이전한다.

### 후속 구현 세부 항목 TS-U00-03

`BP_CFVehiclePawn`의 `AimReticleWidgetClass` 실제 저장값은 현재 덤프 범위에서 확정되지 않았다. 다만 `WBP_AimReticle` 자산과 Pawn의 위젯 생성 API가 실제로 존재하므로 Target UI의 구현 경로에는 차단 요소가 아니다.

---

## 8. 파괴와 수명 관리

현재 차량 파괴 상태는 `UCFVehicleHealthComp`가 소유한다.

```cpp
bool IsDestroyed() const;
FCFVehicleDestroyedSignature OnVehicleDestroyed;
```

`OnVehicleDestroyed`는 체력이 처음 0 이하로 내려갈 때 한 번 발생한다.

### 결정 TS-D00-06 — 선택 대상 무효화

```text
- 차량 선택 시 대상의 VehicleHealthComp.OnVehicleDestroyed를 구독한다.
- 선택 변경 또는 해제 시 기존 구독을 제거한다.
- 파괴 이벤트를 받으면 ValidityChanged(false)를 발생시키고 Destroyed 사유로 해제한다.
- Actor EndPlay와 TWeakObjectPtr 무효화도 별도로 처리한다.
```

권장 흐름:

```text
SetSelectedTarget
→ 대상 수명 이벤트 구독
→ 대상 파괴 또는 EndPlay
→ 선택 유효성 false 전이
→ ClearSelectedTarget
→ 수명 이벤트 구독 해제
```

선택 대상이 무효화됐다고 자동으로 다음 후보를 선택하지 않는다.

---

## 9. 충돌 구조

현재 공용 충돌 채널:

```cpp
WeaponHit = ECC_GameTraceChannel1
Projectile = ECC_GameTraceChannel2
```

`WeaponHit`은 차량 `SM_Body`의 실제 피격 표면을 찾기 위한 무기 전용 Trace다.

### 결정 TS-D00-07 — 타겟 선택 전용 Trace

```text
TargetSelect Trace Channel을 신규 추가한다.
WeaponHit을 타겟 선택의 장기 기준으로 재사용하지 않는다.
```

권장 C++:

```cpp
static constexpr ECollisionChannel TargetSelect = ECC_GameTraceChannel3;
```

권장 `DefaultEngine.ini`:

```ini
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel3,DefaultResponse=ECR_Ignore,bTraceType=True,bStaticObject=False,Name="TargetSelect")
```

P0 차량의 선택 표면은 다음 응답을 가진다.

```text
WeaponHit = Block
Projectile = Block
TargetSelect = Block
```

분리 근거:

- 선택 가능하지만 공격 불가능한 대상이 존재할 수 있다.
- 무기 피격 정책 변경이 선택 정책을 변경해서는 안 된다.
- 장치, 임무 대상과 유틸리티 오브젝트가 무기 피격 표면 없이 선택될 수 있어야 한다.
- 후보 탐색과 실제 피해 판정의 디버그를 분리할 수 있다.

P0 임시 구현에서 `WeaponHit`을 재사용해야 한다면 명시적 임시 단계로 기록하고 `TS-P0-08` 완료 전에 `TargetSelect` 채널로 이전한다.

---

## 10. 관계와 진영

현재 조사 범위에서 공용 Team, Faction, Friendly 또는 Hostile 런타임 시스템은 발견되지 않았다.

### 결정 TS-D00-08 — P0 관계 처리

```text
- 기본 관계는 ECFTargetRelation::Unknown으로 한다.
- 아군과 적군을 모두 선택 가능하게 유지한다.
- 관계 필터는 현재 데이터 계약만 유지한다.
- 실제 Faction 시스템은 별도 기능에서 도입한다.
```

P0 타겟 선택 Task 안에서 임시 팀 시스템을 만들지 않는다.

---

## 11. 장비 연동

현재 `UCFVehicleWeaponComp`는 차량 장착 프로파일, 하드포인트, WeaponData, 터렛과 발사 원점을 해석한다. 일반 스캐너, 수리, 견인과 해킹 장비가 공통으로 사용하는 Target Consumer 계약은 현재 없다.

### 결정 TS-D00-09 — 장비 소비 경계

```text
- UCFTargetSelectComp는 중립적인 선택 대상 조회와 변경 이벤트만 제공한다.
- VehicleWeaponComp에 선택 상태를 소유시키지 않는다.
- 선택 가능 여부와 장비 사용 가능 여부를 분리한다.
- 장비별 Lock, Scan, Hack, Repair, Tow 획득 상태는 장비가 소유한다.
```

기존 조회 API:

```cpp
bool HasSelectedTarget() const;
AActor* GetSelectedTargetActor() const;
bool IsSelectedTargetValid() const;
FCFTargetDisplayInfo GetSelectedTargetDisplayInfo() const;
```

`TS-P0-07`은 정식 유틸리티 장비 전체를 만들지 않고 최소 테스트 소비자 하나로 조회, 변경 이벤트와 비호환 실패를 검증한다.

---

## 12. 설정 데이터 저장

현재 `UCFTargetSelectData`가 `UPrimaryDataAsset` 기반 설정을 제공하며 `UCFTargetSelectComp`는 DataAsset이 없거나 유효하지 않을 때 Fallback 설정을 사용할 수 있다.

### 결정 TS-D00-10 — 설정 소유

```text
- 런타임 튜닝값은 UCFTargetSelectData를 사용한다.
- 컴포넌트의 Fallback 설정은 안전 기본값으로 유지한다.
- 최종 콘텐츠 자산은 /Game/CarFight/Targeting/Data 아래에 둔다.
```

권장 에셋:

```text
/Game/CarFight/Targeting/Data/DA_TargetSelectP0
```

`Targeting` 폴더가 현재 없으면 TS-P0-01 또는 TS-P0-03 에셋 작업에서 생성한다.

---

## 13. 타겟 포인트

현재 `BP_CFVehiclePawn`에는 TargetPoint 컴포넌트가 없다. `ICFTargetSelectable` 기본 구현은 Actor Bounds 중심을 반환할 수 있지만 차량별 시각 중심을 명시적으로 조절할 수 없다.

### 결정 TS-D00-11 — 대표 타겟 포인트

```text
TS-P0-02에서 UCFTargetPointComp를 추가한다.
ACFVehiclePawn 기본 서브오브젝트가 아니라 BP 또는 대상별 구성에서 배치 가능하게 한다.
```

권장 책임:

```text
- 대표 선택 위치 반환
- 활성 여부
- 대표 포인트 여부
- Blueprint 위치 조정
- 디버그 표시
- 포인트가 없을 때 Bounds 중심 Fallback
```

차량 기본 Blueprint에는 다음 이름을 권장한다.

```text
TargetPoint_Primary
```

---

## 14. C++과 Blueprint 책임 배분

### C++ 책임

```text
- ACFVehiclePawn의 TargetSelectComp 생성과 getter
- ACFVehiclePawn의 ICFTargetSelectable 기본 구현
- 선택 가능성, 파괴 여부와 안전한 Fallback 위치 판정
- 후보 수집, 사전 필터와 계층형 정렬
- 선택 상태와 수명 이벤트 관리
- 직접 Trace와 근접 후보 평가
- Enhanced Input 바인딩 함수
- TargetSelect 전용 Collision Channel 상수
- 장비 조회 API와 상태 이벤트
- 디버그 스냅샷
```

### Blueprint 및 에셋 책임

```text
- IA_SelectTarget과 IA_ClearTarget 생성
- IMC_Vehicle_Default 키 매핑
- BP_CFVehiclePawn의 InputAction과 UI 클래스 참조 설정
- TargetPoint_Primary 위치 배치
- WBP_TargetSelect 배치와 스타일
- 후보와 선택 마커 애니메이션
- DA_TargetSelectP0 생성과 튜닝값 설정
- 테스트 차량별 표시 이름 또는 관계 Override
- 사용자 PIE 검증
```

---

## 15. P0 네트워크와 권한 범위

현재 `ACFVehiclePawn`은 싱글플레이 기준으로 다음 상태를 적용한다.

```cpp
bReplicates = false;
SetReplicateMovement(false);
```

### 결정 TS-D00-12 — P0 권한

```text
- TargetSelectComp는 복제하지 않는다.
- 후보, 선택 대상과 HUD 상태는 로컬 플레이어 전용이다.
- Dedicated Server와 2클라이언트 검증은 P0 범위에서 제외한다.
- 선택 대상이 무기 발사 검증이나 피해 권한의 근거가 되지 않는다.
```

멀티플레이가 다시 활성 범위가 되면 선택 요청, 서버 검증 필요 여부와 정보 공개 범위를 별도 설계한다.

---

## 16. 실제 대상 파일 목록

### 기존 수정 후보

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h
UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp
UE/Source/CarFight_Re/Public/CFCollisionChannels.h
UE/Config/DefaultEngine.ini
UE/Source/CarFight_Re/Public/CFTargetSelectComp.h
UE/Source/CarFight_Re/Private/CFTargetSelectComp.cpp
UE/Source/CarFight_Re/Public/CFTargetSelectable.h
UE/Source/CarFight_Re/Private/CFTargetSelectable.cpp
```

### 신규 C++ 후보

```text
UE/Source/CarFight_Re/Public/CFTargetPointComp.h
UE/Source/CarFight_Re/Private/CFTargetPointComp.cpp
UE/Source/CarFight_Re/Public/UI/CFTargetSelectWidget.h
UE/Source/CarFight_Re/Private/UI/CFTargetSelectWidget.cpp
```

### 신규 또는 수정 Unreal 에셋

```text
/Game/CarFight/Input/IA_SelectTarget
/Game/CarFight/Input/IA_ClearTarget
/Game/CarFight/Input/IMC_Vehicle_Default
/Game/CarFight/Vehicles/Blueprints/BP_CFVehiclePawn
/Game/CarFight/UI/WBP_TargetSelect
/Game/CarFight/Targeting/Data/DA_TargetSelectP0
```

### 문서

```text
Document/Design/TargetSelect.md
Document/Plan/TargetSelectInvestigation.md
Document/Plan/TargetSelect/TargetSelectPlan.md
Document/Plan/TargetSelectRoadmap.md
Document/Plan/TargetSelectWorkOrder.md
Document/ActiveWork.md
```

---

## 17. 재사용 타입과 신규 타입

### 재사용

```text
ACFVehiclePawn
UCFVehicleCameraComp
UCFVehicleAimComp
UCFVehicleHealthComp
UCFAimReticleWidget의 별도 Viewport UI 생성 패턴
IMC_Vehicle_Default
CFCollisionChannels namespace
ECFTargetCoreState
ECFTargetRelation
ECFTargetCategory
ECFTargetInfoLevel
ECFTargetTrackState
ECFTargetClearReason
FCFTargetDisplayInfo
FCFTargetSelectionContext
FCFTargetCandidate
FCFTargetSelectConfig
ICFTargetSelectable
UCFTargetSelectData
UCFTargetSelectComp
```

### 신규 필요

```text
UCFTargetPointComp
UCFTargetSelectWidget
IA_SelectTarget
IA_ClearTarget
WBP_TargetSelect
DA_TargetSelectP0
TargetSelect Trace Channel
```

### P0에서 만들지 않음

```text
별도 Player Targeting Subsystem
공용 Faction 시스템
다중 타겟 컨테이너
서버 복제 계약
장비별 LockOn 상태 머신
센서 정보 모델
부위 선택 모델
```

---

## 18. 미결 항목 해소표

| 기존 미결 항목 | 조사 결과 | 상태 |
|---|---|---|
| 컴포넌트 소유 클래스 | `ACFVehiclePawn` | 확정 |
| 기존 크로스헤어/HUD 경로 | `UCFVehicleCameraComp`, `WBP_AimReticle`, Pawn Viewport 생성 | 확정 |
| 관계/진영 시스템 | 현재 공용 시스템 없음, P0 `Unknown` | 확정 |
| 기존 선택 가능 인터페이스 | 없음, 기존 `ICFTargetSelectable` 사용 | 확정 |
| 입력 Action과 Mapping | `IA_SelectTarget`, `IA_ClearTarget`, `IMC_Vehicle_Default` | 에셋명 확정 |
| 입력 최종 키 | TS-P0-05에서 조작 테스트 후 확정 | 후속 세부 항목 |
| 네트워크 범위 | 로컬 싱글, 복제 없음 | 확정 |
| 충돌 채널 | 신규 `TargetSelect` Trace Channel 권장 | 확정 |
| 설정 저장 형태 | `UCFTargetSelectData` + Fallback | 확정 |
| 직접 Trace 조준 기준 | `VehicleCameraComp` 카메라 Aim | 확정 |
| 직접 Trace 시작점 API | CameraComp getter 추가 권장 | 후속 세부 항목 |
| 파괴 무효화 | `VehicleHealthComp.OnVehicleDestroyed` | 확정 |
| HUD 통합 위치 | Aim Reticle과 분리된 별도 Viewport 위젯 | 확정 |
| 장비 소비 방식 | TargetSelectComp 중립 조회 API | 확정 |

후속 세부 항목은 구현 경로를 추측하게 만드는 차단 요소가 아니다. 각 항목의 담당 Task가 확정돼 있으므로 TS-P0-00 종료 조건을 만족한다.

---

## 19. TS-P0-01 현재 코드와 조사 결과 비교

### 일치

```text
- UCFTargetSelectComp를 ActorComponent로 구성
- 컴포넌트 자체 복제 비활성
- 선택 가능 인터페이스 분리
- 관계와 분류를 표시 정보에서 제공
- DataAsset과 Fallback 설정 분리
- 선택과 후보를 약한 Actor 참조로 관리
- BlueprintAssignable 변경 이벤트 제공
- 후보 검색과 수명 자동 처리를 후속 Task로 분리
```

### TS-P0-01 검증 또는 최소 보정 필요

```text
- ACFVehiclePawn에 컴포넌트가 아직 생성되지 않음
- ACFVehiclePawn이 ICFTargetSelectable을 아직 구현하지 않음
- Blueprint에서 모든 enum, struct, 함수와 이벤트 노출 확인 필요
- 같은 후보와 같은 대상 재설정 시 이벤트 중복 여부 확인 필요
- 유효하지 않은 DataAsset의 Fallback 동작 확인 필요
- 선택 대상 파괴 및 EndPlay 구독 API는 TS-P0-04 전에 계약 검토 필요
- GetTargetSelectionLocation의 차량별 TargetPoint 연결은 TS-P0-02 범위
```

`ACFVehiclePawn` 연결은 원래 `TS-P0-05`에 포함돼 있었지만 컴포넌트 기본 서브오브젝트와 차량 인터페이스 구현은 계약 검증에 필요한 최소 통합이다. 다음 작업지시에서는 입력 바인딩과 HUD를 제외하고 이 최소 통합을 `TS-P0-01`에 포함할지 명시해야 한다.

권장 결정:

```text
- TargetSelectComp 기본 서브오브젝트 생성과 getter: TS-P0-01 최소 통합
- ACFVehiclePawn의 ICFTargetSelectable 기본 구현: TS-P0-01 최소 통합
- 입력 바인딩: TS-P0-05 유지
- Target UI 생성: TS-P0-06 유지
```

---

## 20. TS-P0-00 완료 판정

### 완료 조건 확인

```text
[PASS] 실제 Pawn, 입력, HUD, 관계, 장비, 파괴, 충돌 경로 기록
[PASS] 재사용 타입과 신규 타입 분리
[PASS] C++과 Blueprint 책임 확정
[PASS] P0 로컬 싱글 범위 확정
[PASS] 추측 기반 클래스 또는 에셋 경로 제거
[PASS] TS-P0-01 작업 대상과 검증 범위 작성 가능
[PASS] 저장소 검색과 Unreal 에셋 덤프 교차 확인
```

판정:

```text
TS-P0-00 기존 구조 조사 = Done
현재 Task = TS-P0-01 계약 및 데이터 설계 검증
```

---

## 21. 다음 작업

### 즉시 작업

```text
TS-P0-01 현재 C++ 계약 검증과 최소 통합 보정
```

검증 범위:

```text
1. Blueprint enum과 struct 노출
2. ICFTargetSelectable BlueprintNativeEvent 노출
3. UCFTargetSelectData 생성 및 Fallback 검증
4. UCFTargetSelectComp 함수와 이벤트 노출
5. 같은 후보와 같은 선택 대상 재설정 시 이벤트 무동작
6. null, 자기 자신, 인터페이스 미구현 Actor 거부
7. 관계, 분류, 필수 태그와 제외 태그 필터
8. 약한 참조 무효화 안전성
9. ACFVehiclePawn TargetSelectComp 기본 서브오브젝트와 getter
10. ACFVehiclePawn ICFTargetSelectable 최소 구현
```

제외 범위:

```text
- 후보 Trace
- TargetPointComp
- 입력 에셋과 키 매핑
- HUD
- 파괴 이벤트 자동 구독
- 장비 소비자
```

### TS-P0-01 이후

```text
TS-P0-02 대표 타겟 포인트
→ TS-P0-03 직접 조준 및 근접 후보 탐색
→ TS-P0-04 선택 상태 수명
→ TS-P0-05 입력 연결
→ TS-P0-06 HUD
→ TS-P0-07 장비 조회
→ TS-P0-08 통합 검증
```

---

## 22. 변경 이력

### v1.0.0 - 2026-07-23

- CarFight 실제 Pawn, Aim, 입력, HUD, 파괴, 장비와 충돌 구조를 조사했다.
- `ACFVehiclePawn`을 선택 컴포넌트 소유 클래스와 차량 선택 가능 구현체로 확정했다.
- 직접 선택 기준을 `UCFVehicleCameraComp`의 카메라 Aim으로 고정했다.
- `IA_SelectTarget`, `IA_ClearTarget`, 별도 Target UI와 `TargetSelect` Trace Channel 필요성을 확정했다.
- 관계 시스템 부재, 로컬 싱글 권한과 `VehicleHealthComp.OnVehicleDestroyed` 재사용을 기록했다.
- TS-P0-00 완료와 TS-P0-01 검증 범위를 확정했다.

---

## 23. Migration

신규 조사 문서다.

적용 규칙:

1. 현재 `CFTargetSelect` 소스를 제거하거나 처음부터 다시 작성하지 않는다.
2. 신규 구현은 이 문서의 실제 경로와 책임 배분을 따른다.
3. 직접 선택 Trace에 `WeaponAimSolution` 또는 터렛 방향을 사용하지 않는다.
4. 입력과 HUD를 TS-P0-01 검증에 섞지 않는다.
5. `TS-P0-01` 검증에서 계약 문제가 발견되면 최소 범위만 보정하고 결과를 `TargetSelectPlan.md`에 기록한다.
