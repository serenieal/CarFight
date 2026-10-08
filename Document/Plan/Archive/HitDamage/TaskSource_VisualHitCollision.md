# 시각 차체 기반 무기 피격 콜리전 분리 Task Source

- Version: 1.0.0
- Date: 2026-07-13
- Status: Ready for Codex Contract
- Project: CarFight
- Feature: `CF-FQ-018`
- Work Type: C++ / Config collision separation and hit-component debug
- artifact_role: intermediate
- codex_input: false

## Goal

현재 `VehicleMesh` Physics Asset이 받는 HitScan / Projectile 피격을 분리하여, 플레이어에게 보이는 `SM_Body` StaticMesh가 차량 차체의 무기 피격 표면이 되도록 구현한다.

이번 작업은 실제 HP 감소 전에 수행하는 선행 작업이다.

```text
VehicleMesh
- Chaos Vehicle 물리와 일반 월드 물리 충돌 유지
- WeaponHit Trace와 Projectile Object Channel은 Ignore

SM_Body
- 화면에 보이는 차체
- QueryOnly
- WeaponHit Trace와 Projectile Object Channel은 Block

HitScan / Projectile
- 실제 명중 컴포넌트 이름을 DamageHitContext Debug에 기록
```

## Confirmed Current State

### Project Collision Config

`UE/Config/DefaultEngine.ini`에는 현재 `GameTraceChannel` 또는 사용자 Collision Profile 정의가 없다.

따라서 이번 작업에서 아래 슬롯을 사용한다.

```text
ECC_GameTraceChannel1 = WeaponHit Trace Channel
ECC_GameTraceChannel2 = Projectile Object Channel
```

### Vehicle Assets

현재 기준 차량 DataAsset과 차체 StaticMesh 연결:

```text
DA_TestSedan
→ /Game/CarFight/Vehicles/Meshes/Sedan/Sedan.Sedan

DA_TestSUV
→ /Game/CarFight/Vehicles/Meshes/SUV/SUV.SUV
```

현재 `BP_CFVehiclePawn`:

```text
VehicleMesh = QueryAndPhysics / Vehicle Profile / Visibility Block
SM_Body     = NoCollision / NoCollision Profile / Visibility Ignore
```

### Current Weapon Collision

```text
ACFVehiclePawn::RunLocalDummyHitScan()
→ ECC_Visibility

ACFProjectileActor::CollisionComponent
→ ObjectType WorldDynamic
→ All Channels Block
```

따라서 현재 차량 피격은 `SM_Body`가 아니라 `VehicleMesh` Physics Asset 기준이다.

### StaticMesh Collision Limitation

현재 AssetDump는 StaticMesh의 `BodySetup/AggGeom` Simple Collision 개수를 노출하지 않는다.

이번 코드 작업은 `Sedan.Sedan`, `SUV.SUV`의 `.uasset`을 수정하지 않는다.
대신 런타임에서 `SM_Body`의 StaticMesh가 Simple Collision을 갖지 않으면 명확한 Warning 로그를 남긴다.

Projectile 충돌을 위해 기준 차체 StaticMesh에는 최종적으로 Simple Collision 또는 Convex Collision이 필요하다.

## In Scope

1. 커스텀 Collision Channel과 `VehicleVisualHit` Profile을 `DefaultEngine.ini`에 추가한다.
2. C++에서 채널 값을 공유하는 `CFCollisionChannels.h`를 추가한다.
3. 차량의 모든 기존 PrimitiveComponent가 WeaponHit / Projectile을 우선 Ignore하도록 설정한다.
4. `SM_Body`만 QueryOnly 상태로 두고 WeaponHit / Projectile을 Block한다.
5. `VehicleMesh`의 기존 Vehicle Profile과 QueryAndPhysics 상태를 변경하지 않는다.
6. Dummy HitScan을 `ECC_Visibility`에서 WeaponHit 채널로 변경한다.
7. Projectile CollisionComponent의 Object Type을 Projectile 채널로 변경한다.
8. HitScan과 Projectile의 실제 피격 컴포넌트 이름을 `FCFDamageHitContext`와 VehicleDebug 요약에 기록한다.
9. `SM_Body` StaticMesh의 Simple Collision 누락을 런타임 Warning으로 알린다.
10. Unreal Editor 타깃 빌드를 수행한다.

## Target Files

1. `UE/Config/DefaultEngine.ini`
2. `UE/Source/CarFight_Re/Public/CFCollisionChannels.h` — 신규
3. `UE/Source/CarFight_Re/Public/CFDamageTypes.h`
4. `UE/Source/CarFight_Re/Public/CFProjectileActor.h`
5. `UE/Source/CarFight_Re/Private/CFProjectileActor.cpp`
6. `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
7. `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

## Collision Definitions

### DefaultEngine.ini

`[/Script/Engine.CollisionProfile]` 섹션을 추가하거나 기존 섹션이 생기면 병합한다.

채널 정의:

```ini
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel1,DefaultResponse=ECR_Block,bTraceType=True,bStaticObject=False,Name="WeaponHit")
+DefaultChannelResponses=(Channel=ECC_GameTraceChannel2,DefaultResponse=ECR_Block,bTraceType=False,bStaticObject=False,Name="Projectile")
```

Profile 이름:

```text
VehicleVisualHit
```

Profile 정책:

```text
CollisionEnabled = QueryOnly
ObjectTypeName    = WorldDynamic
WeaponHit         = Block
Projectile        = Block
기존 엔진 채널    = Ignore
```

최종 INI 문법은 UE 최신버전의 `DefaultChannelResponses` / `Profiles` 형식에 맞춰 작성한다.

### CFCollisionChannels.h

직관적인 공용 이름을 제공한다.

```cpp
namespace CFCollisionChannels
{
    inline constexpr ECollisionChannel WeaponHit = ECC_GameTraceChannel1;
    inline constexpr ECollisionChannel Projectile = ECC_GameTraceChannel2;

    // VehicleVisualHit Collision Profile 이름 반환 또는 상수
}
```

`VehicleVisualHit` Profile 이름도 한 곳에서 공유한다.

## Required C++ Changes

### 1. ACFVehiclePawn Visual Hit Policy

`CFVehiclePawn.h/.cpp`에 직관적인 private 함수 하나를 추가한다.

권장 함수명:

```cpp
void ApplyVehicleVisualHitCollision();
```

이 함수는 다음 순서로 동작한다.

```text
1. Pawn의 모든 UPrimitiveComponent를 조회한다.
2. 모든 PrimitiveComponent가 WeaponHit / Projectile에 Ignore하도록 설정한다.
3. 기존 CollisionEnabled, ObjectType, Profile, 일반 채널 응답은 변경하지 않는다.
4. 이름이 정확히 SM_Body인 UStaticMeshComponent를 찾는다.
5. SM_Body에 VehicleVisualHit Profile을 적용한다.
6. SM_Body를 QueryOnly로 설정한다.
7. SM_Body의 SimulatePhysics와 Overlap Event는 사용하지 않는다.
8. SM_Body는 WeaponHit / Projectile만 Block하고 나머지 채널은 Ignore한다.
9. SM_Body가 없거나 StaticMesh가 없으면 Warning을 남기고 안전하게 종료한다.
10. StaticMesh BodySetup의 Simple Collision 개수가 0이면 Runtime GameWorld에서 Warning을 남긴다.
```

중요:

```text
- VehicleMesh의 CollisionEnabled를 끄지 않는다.
- VehicleMesh의 Vehicle Collision Profile을 바꾸지 않는다.
- VehicleMesh의 WorldStatic / WorldDynamic / Vehicle 물리 응답을 바꾸지 않는다.
- 변경하는 것은 WeaponHit / Projectile 응답뿐이다.
```

`ApplyVehicleVisualConfig()`에서 `SM_Body`에 ChassisMesh를 적용한 뒤 `ApplyVehicleVisualHitCollision()`을 호출한다.

`ApplyVehicleVisualConfig()`는 OnConstruction과 런타임 초기화 경로에서 이미 사용되므로 동일 경로를 유지한다.

Simple Collision 검사 시 필요하면 다음 헤더를 사용한다.

```cpp
#include "PhysicsEngine/BodySetup.h"
```

`BodySetup->AggGeom.GetElementCount()` 또는 UE 최신버전에서 동일 의미의 안전한 API를 사용한다.

### 2. Dummy HitScan Channel

`ACFVehiclePawn::RunLocalDummyHitScan()`에서:

```cpp
ECC_Visibility
```

를:

```cpp
CFCollisionChannels::WeaponHit
```

으로 변경한다.

기존 `QueryParams.AddIgnoredActor(this)`는 유지한다.
벽과 지형은 WeaponHit 기본 Block 정책으로 계속 Trace를 차단해야 한다.

### 3. Projectile Object Type

`ACFProjectileActor` 생성자와 `ApplyProjectileCollision()`에서:

```cpp
ECC_WorldDynamic
```

를:

```cpp
CFCollisionChannels::Projectile
```

로 변경한다.

Projectile의 기존 All Channels Block 정책은 유지한다.
대상 측 응답에서 VehicleMesh는 Ignore, SM_Body는 Block한다.

발사 주체의 `IgnoreActorWhenMoving()` 정책은 그대로 유지한다.

### 4. Hit Component Debug

`FCFDamageHitContext`에 다음 의미의 필드를 추가한다.

```cpp
FName HitComponentName = NAME_None;
```

표시명과 ToolTip은 한국어를 사용하고 내부 변수명은 영문을 유지한다.

Dummy HitScan:

```text
Blocking Hit + HitResult 유효
→ HitResult->GetComponent()->GetFName()

Miss 또는 Component 없음
→ NAME_None
```

Projectile:

```text
HandleProjectileHit의 OtherComponent 이름을 LastHitComponentName에 저장
비 Hit 비활성화에서는 NAME_None으로 초기화
GetLastHitComponentName() getter 제공
Pool 반환 뒤 VehiclePawn이 DamageHitContext.HitComponentName에 복사
```

`BuildDamageHitContextSummary()`에 다음 필드를 추가한다.

```text
HitComponent=SM_Body
```

기존 Summary 필드는 삭제하거나 이름을 바꾸지 않는다.

## Versioning

예상 버전:

```text
CFCollisionChannels.h   v1.0.0
CFDamageTypes.h         v1.2.0
CFProjectileActor.h     v1.4.0
CFProjectileActor.cpp   v1.4.0
CFVehiclePawn.h         v2.103.0 이상
CFVehiclePawn.cpp       v2.108.0 이상
```

각 파일의 기존 최신 버전을 기준으로 증가시킨다.

모든 변경 파일 상단에:

```text
Version
Date
Description 또는 Scope
Changelog
Migration
```

을 갱신한다.

신규/수정 함수, UPROPERTY, 핵심 지역 변수 위에는 버전이 포함된 1줄 요약 주석을 둔다.

## Constraints

- 실제 HP 감소, Health Component, 파괴 상태를 구현하지 않는다.
- `UCFDamageData`의 수치나 구조를 변경하지 않는다.
- Damage 계산 공식을 추가하지 않는다.
- `BP_CFVehiclePawn.uasset`을 수정하지 않는다.
- `Sedan.Sedan`, `SUV.SUV` 또는 다른 StaticMesh `.uasset`을 수정하지 않는다.
- 휠과 터렛을 독립 피격 대상으로 만들지 않는다.
- VehicleMesh의 Physics Asset을 수정하지 않는다.
- 기존 Chaos Vehicle 주행 설정을 변경하지 않는다.
- 기존 Projectile 이동 속도, 반경, 중력, 수명, Pool 정책을 변경하지 않는다.
- Existing FireFeedback / Reticle 코드를 수정하지 않는다.
- 서버 권한, 복제, 멀티플레이 코드를 추가하지 않는다.
- Complex Collision을 런타임에 강제로 설정하지 않는다.
- Simple Collision이 없을 때 VehicleMesh로 자동 fallback하지 않는다.
- Git commit 또는 push를 수행하지 않는다.

## Acceptance Criteria

1. `DefaultEngine.ini`에 WeaponHit, Projectile, VehicleVisualHit 정의가 정확히 한 번 존재한다.
2. `CFCollisionChannels.h`가 두 채널과 Profile 이름의 단일 C++ 기준을 제공한다.
3. `VehicleMesh`는 기존 QueryAndPhysics / Vehicle 물리 동작을 유지하면서 WeaponHit / Projectile만 Ignore한다.
4. `SM_Body`는 QueryOnly이며 WeaponHit / Projectile을 Block한다.
5. Pawn의 다른 PrimitiveComponent는 WeaponHit / Projectile을 Ignore한다.
6. Dummy HitScan은 `ECC_Visibility`가 아니라 WeaponHit을 사용한다.
7. Projectile CollisionComponent는 Projectile Object Channel을 사용한다.
8. 발사 주체 자기 차량 Ignore가 유지된다.
9. Dummy HitScan 명중 시 `DamageHitContext.HitComponentName`이 실제 Component 이름을 기록한다.
10. Projectile 명중 시 Pool 반환 뒤에도 `DamageHitContext.HitComponentName`이 유지된다.
11. VehicleDebug DamageHitContext Summary에 `HitComponent=...`이 표시된다.
12. SM_Body StaticMesh에 Simple Collision이 없으면 명확한 Warning이 출력된다.
13. Simple Collision 누락 시 VehicleMesh Physics Asset으로 무기 피격을 되돌리는 fallback은 없다.
14. Unreal Editor 타깃 빌드가 성공한다.
15. 대상 파일 외 코드/Config/Asset은 변경하지 않는다.

## Verification

### Static Review

```text
- DefaultEngine.ini에 기존 사용자 Collision Channel이 없던 상태에서 GameTraceChannel1/2만 추가됐는지 확인
- VehicleMesh의 Profile / CollisionEnabled 변경 코드가 없는지 확인
- SM_Body만 WeaponHit / Projectile Block인지 확인
- RunLocalDummyHitScan에서 ECC_Visibility가 제거됐는지 확인
- Projectile ObjectType이 CFCollisionChannels::Projectile인지 확인
- HitComponentName이 HitScan / Projectile 두 경로에서 채워지는지 확인
- .uasset 변경이 없는지 확인
```

### Build

Unreal Editor를 종료한 상태에서 실행한다.

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

빌드 실패 시 첫 컴파일 오류를 보고하고 범위를 임의로 확장하지 않는다.

### PIE Manual Check — Before StaticMesh Asset Edit

1. Editor를 완전히 재시작해 Collision Channel Config를 로드한다.
2. `DA_TestSedan` 또는 `DA_TestSUV` 차량을 실행한다.
3. Output Log에서 `SM_Body` Simple Collision 누락 Warning 여부를 확인한다.
4. Warning이 있으면 코드 실패가 아니라 StaticMesh Asset 준비 필요 상태로 기록한다.

### StaticMesh Manual Check

다음 자산을 Static Mesh Editor에서 연다.

```text
/Game/CarFight/Vehicles/Meshes/Sedan/Sedan
/Game/CarFight/Vehicles/Meshes/SUV/SUV
```

확인:

```text
- Collision 표시를 켰을 때 Simple/Convex Collision이 존재하는가
- 차체 외곽과 Collision 형상이 크게 어긋나지 않는가
```

Simple Collision이 없으면 사용자 승인 후 별도 Asset 작업으로 추가한다.
이번 Codex 작업에서 `.uasset`을 자동 수정하지 않는다.

### PIE Manual Check — HitScan

```text
1. Dummy HitScan 모드로 상대 차량의 보이는 차체를 발사한다.
2. DamageHitContext Summary가 HitComponent=SM_Body인지 확인한다.
3. Physics Asset만 돌출된 보이지 않는 공간을 발사했을 때 차량 Hit이 발생하지 않는지 확인한다.
4. 벽과 지형은 계속 HitScan을 차단하는지 확인한다.
```

### PIE Manual Check — Projectile

```text
1. Projectile 모드로 상대 차량의 보이는 차체를 발사한다.
2. Projectile이 VehicleMesh에서 조기 충돌하지 않고 SM_Body에서 충돌하는지 확인한다.
3. Pool 반환 뒤 DamageHitContext Summary가 HitComponent=SM_Body인지 확인한다.
4. 자기 차량 발사체 Ignore가 유지되는지 확인한다.
```

### Vehicle Physics Regression

```text
- 전진 / 후진 / 조향 / 브레이크 / 핸드브레이크 정상
- 지면과 벽 충돌 정상
- 차량이 바닥을 통과하거나 월드 충돌을 잃지 않음
```

## Out of Scope

- StaticMesh Simple Collision 자동 생성
- `.uasset` 수정
- 체력/내구도 컴포넌트
- BaseDamage 적용
- 장갑 관통
- 충격량 적용
- 차량 파괴
- 부위별 피해
- 폭발 범위 피해
- VFX / SFX
- 멀티플레이 및 복제
- 문서 완료 처리
- Git commit / push

## Unresolved

- 없음.

## Manual Follow-up

- `Sedan.Sedan`, `SUV.SUV`의 실제 Simple Collision 존재 여부는 현재 AssetDump로 확인할 수 없다.
- 코드 빌드 후 Runtime Warning과 Static Mesh Editor 수동 확인으로 해소한다.
- Simple Collision이 없다면 별도 Asset 작업 승인을 받은 뒤 추가한다.

## Changelog

- v1.0.0: GameTraceChannel1/2 사용, SM_Body Query 피격, VehicleMesh 무기 Query 제외, HitComponentName Debug 기록을 포함한 Codex 작업 원본 작성.
