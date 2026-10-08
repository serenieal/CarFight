# 16_WeaponArtRules.md

문서 버전: v0.8  
작성일: 2026-07-07  
최종 수정일: 2026-07-08  
상태: Draft  
관련 범위: 차량용 무기 원화 기준, P0 루프 터렛 원화, Tripo 메시 생성 기준, UE 터렛 시각 컴포넌트 적용 기준

---

## 0. 문서 목적

이 문서는 CarFight의 P0 차량용 무기 원화를 만들기 위한 기준 문서다.

현재 범위는 모든 무기군이 아니라, P0 첫 제작 대상인 `P0 Roof Turret Cannon`에 한정한다.

이 문서의 목적은 다음과 같다.

1. Tripo로 차량용 루프 터렛 메시를 만들기 전에 원화가 지켜야 할 기준을 정한다.
2. 터렛 런타임 MeshComponent는 `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh` 3단 구조를 유지한다.
3. 컨셉이미지에는 `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh`가 조립된 완성 터렛 전체를 표시한다.
4. Tripo 입력용 이미지는 `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh`를 각각 별도의 쿼터뷰 + 4뷰 단일 이미지 시트로 준비한다.
5. 각 이미지 시트는 한 장 안에 쿼터뷰 1개와 Front / Back / Left / Right 4개 뷰를 함께 배치한다.
6. `TurretBaseMesh`는 무기별로 매번 새로 만드는 본체가 아니라, 별도 입력 이미지로 제작한 뒤 공용 표준 베이스로 재사용할 수 있는 파츠로 취급한다.
7. Blender 후처리와 Unreal Engine 적용 시 필요한 `YawPivot`, `PitchPivot`, `Muzzle` 소켓 기준을 정한다.
8. 차량보다 무기가 먼저 읽히거나, 전차/APC/군용 무장 플랫폼처럼 보이는 방향을 방지한다.
9. 현재 `BP_CFVehiclePawn` 터렛 시각 컴포넌트 구조와 맞는 원화 기준을 제공한다.
10. 사용자가 Blender 수동 모델링을 하지 않는다는 전제를 반영한다.
11. Blender는 수동 수정 도구가 아니라, 제공된 스크립트를 실행하는 보조 도구로만 사용한다.
12. Tripo 입력용 이미지는 `17_TripoInputImageRules.md`의 범용 입력 규칙을 따른다.

이 문서는 감상용 일러스트 기준이 아니다.

```text
원화
→ Tripo 입력용 이미지 제작
→ Tripo 3D 생성
→ Blender에서 스크립트 실행
→ Unreal Engine 터렛 시각 컴포넌트 적용
```

위 제작 흐름을 위한 실무 기준이다.

---

## 1. 상위 기준 문서

이 문서는 아래 문서를 기준으로 작성한다.

| 문서 | 사용 기준 |
|---|---|
| `Document/Plan/ConceptArt/00_ArtDirection.md` | 단순화된 근미래 민간 차량 개조 전투차 방향 |
| `Document/Plan/ConceptArt/01_DecisionChecklist.md` | 원화 제작 전 결정 순서 |
| `Document/Plan/ConceptArt/02_ComplexityBudget.md` | 1인 개발 가능 복잡도, C1 Simple Game Asset 기준 |
| `Document/Plan/ConceptArt/08_HardpointRules.md` | 루프 중앙 하드포인트, 차량보다 무기가 먼저 읽히면 안 된다는 기준 |
| `Document/Plan/ConceptArt/10_ConceptSheetFormat.md` | Side / Front / Rear / Top View 정사영 시트 기준 |
| `Document/Plan/ConceptArt/11_AIPromptRules.md` | AI 원화 프롬프트 기본 구조 |
| `Document/Plan/ConceptArt/15_LoadoutBalance.md` | 차량 본체와 장비 장착 기준 분리 |
| `Document/ProjectSSOT/CombatPlan/06_WeaponTypes.md` | 대구경포 역할, 느린 회전, 강한 한 방 기준 |
| `Document/ProjectSSOT/CombatPlan/12_Hardpoints.md` | 루프 터렛 하드포인트, 크기 제한, 조준 방식 기준 |
| `Document/Plan/TurretPlan/CF_TurretPlan_v0_1.md` | P0 터렛 런타임/시각 구조 방향 |

---

## 2. 현재 문서 범위

### 2.1 포함 범위

이 문서는 다음 대상만 다룬다.

```text
P0 Roof Turret Cannon
P0 루프 장착형 대구경 저속 터렛포
```

이 무기는 차량 루프 중앙 `Top_01` 하드포인트에 장착되는 차량용 포탑 무기다.

### 2.2 제외 범위

이 문서에서는 아래 항목을 확정하지 않는다.

1. 기관총 원화 기준
2. 오토캐논 원화 기준
3. 미사일/로켓 런처 원화 기준
4. EMP/전자전 장비 원화 기준
5. 실제 Damage 수치
6. 실제 Projectile 밸런스
7. 최종 무기 장착 비용
8. 멀티플레이 판정 구조
9. 모든 차급별 최종 무기 밸런스

위 항목은 별도 무기 기준 문서 또는 CombatPlan 문서에서 확장한다.

---

## 3. P0 Roof Turret Cannon 정의

### 3.1 기본 명칭

| 항목 | 값 |
|---|---|
| 영문 짧은 이름 | `P0 Roof Turret Cannon` |
| 영문 정식 이름 | `P0 Roof-Mounted Large-Caliber Slow Turret Cannon` |
| 한국어 이름 | `P0 루프 장착형 대구경 저속 터렛포` |
| 기준 하드포인트 | `Top_01` |
| 기준 Mount Profile | `RoofTurret_MediumOrLarge` |
| 기준 장착 타입 | `Turret` |
| 기준 무기 감각 | 강한 한 방, 느린 회전, 느린 안정화 |

### 3.2 무기 정체성

P0 Roof Turret Cannon은 사람이 손에 들고 사용하는 총기가 아니다.

이 무기는 차량 루프에 장착되는 차량용 포탑 무기다.

기본 정체성은 다음과 같다.

```text
민간 차량 위에 개조 장비처럼 장착된 근미래 대구경 루프 터렛.
강한 한 방을 가진 대신, 무겁고 느리며 안정화 시간이 필요한 차량용 포탑 무기.
```

### 3.3 전투 역할

이 무기는 다음 역할을 가진다.

1. 중장갑 차량 대응
2. 선제타격
3. 길목 장악
4. 공간 통제
5. 느린 터렛 각속도 시스템 검증
6. 플레이어 조준점과 실제 무기 조준점이 다르게 움직이는 구조 검증

### 3.4 전투 감각

이 무기는 빠르게 적을 따라붙어 계속 맞히는 무기가 아니다.

이 무기는 다음 감각을 목표로 한다.

```text
미리 방향을 잡고,
터렛이 천천히 따라가고,
안정화된 순간 강한 한 발을 쏘는 무기.
```

---

## 4. 크기 등급 기준

### 4.1 기본 크기 등급

P0 Roof Turret Cannon의 원화 크기 등급은 다음으로 둔다.

```text
Medium-Large
```

이 등급은 중형보다 크지만, 차량보다 먼저 읽힐 정도로 거대한 무기는 아니다.

### 4.2 크기 방향

P0 Roof Turret Cannon은 대구경 무기처럼 보여야 한다.

하지만 다음처럼 보이면 안 된다.

1. 전차 주포
2. 전차 포탑
3. APC/IFV 원격무장장치
4. 군용 장갑차 상부 무장 플랫폼
5. 차량 루프 전체를 덮는 대형 무기 플랫폼

### 4.3 기준 차량

원화 검수 기준 차량은 다음으로 둔다.

| 용도 | 차급 |
|---|---|
| 기본 기준 차량 | Compact급 민간 차량 |
| 보조 검수 차량 | Sedan |
| 보조 검수 차량 | SUV |

기본 원화는 Compact급 민간 차량에 올렸을 때 과대해 보이지 않는 크기를 우선한다.

Sedan에서는 루프라인이 낮아 보일 수 있으므로 높이를 더 엄격히 본다.

SUV에서는 루프 면적이 넓어 무기가 작아 보일 수 있으므로, 차량보다 먼저 읽히지 않는지와 동시에 무기 정체성이 유지되는지 확인한다.

### 4.4 권장 비율

아래 값은 절대 수치가 아니라 원화 검수용 비율 기준이다.

| 항목 | 권장 기준 |
|---|---:|
| 터렛 전체 폭 | 차량 차폭의 약 35~45% |
| 터렛 베이스 폭 | 차량 차폭의 약 25~35% |
| 포신 길이 | 차량 전체 길이의 약 18~25% 우선 |
| 터렛 전체 높이 | 차량 전체 높이의 약 15~25% |
| 장착 중심 | 차량 좌우 중심선 위 |
| 장착 위치 | 루프 앞뒤 중앙 또는 약간 후방 |

포신 길이는 전방향 방위각 조준 시 차체 외곽과 루프 파츠 간섭을 고려해 제한한다.

---

## 5. 회전/조준 기준

### 5.1 핵심 기준

P0 Roof Turret Cannon은 전방향 방위각 조준이 가능한 루프 터렛이다.

```text
Full Azimuth Coverage
전방향 방위각 조준 가능
```

이 말은 터렛이 반드시 360도까지만 회전해야 한다는 뜻이 아니다.

핵심은 다음이다.

```text
차량 전방, 좌측, 우측, 후방을 모두 조준할 수 있어야 한다.
```

### 5.2 구현 해석

문서 기준은 특정 Yaw 값 표현 방식 하나를 강제하지 않는다.

가능한 구현 방식은 다음과 같다.

| 방식 | 설명 |
|---|---|
| `-180 ~ 180` | 전방향 범위를 signed angle로 표현 |
| `0 ~ 360` | 전방향 범위를 unsigned angle로 표현 |
| 연속 Yaw 회전 | 제한 없이 누적 회전 후 내부에서 래핑 또는 정규화 |
| 별도 플래그 | `bAllowFullYawTraverse` 같은 필드로 전방향 조준 가능 여부 표현 |

다만 현재 P0 문서 기준에서는 전방 제한각 무기처럼 좁게 제한하지 않는다.

### 5.3 현재 데이터 주의사항

현재 확인된 `DA_CannonBody` 값은 다음과 같다.

| 항목 | 현재 값 |
|---|---:|
| `MinYawDeg` | `-30` |
| `MaxYawDeg` | `30` |

이 값은 전방 제한각 무기에 가깝다.

P0 Roof Turret Cannon 기준에서는 전방향 방위각 조준이 가능하도록 데이터 또는 런타임 해석을 수정해야 한다.

이 문서는 `-180 ~ 180`만을 유일한 정답으로 강제하지 않는다.

그러나 현재 `-30 ~ 30` 제한은 P0 루프 터렛 원화/기획 기준과 맞지 않는 것으로 본다.

### 5.4 Top View 검수 기준

원화의 Top View에서는 다음을 확인할 수 있어야 한다.

1. 터렛 중심 위치
2. 회전 반경
3. 전방 조준 시 포신 간섭 여부
4. 좌측 조준 시 포신 간섭 여부
5. 우측 조준 시 포신 간섭 여부
6. 후방 조준 시 포신 간섭 여부
7. 루프 장갑판과 간섭 여부
8. 센서 박스와 간섭 여부
9. 후방 파츠와 간섭 여부
10. 차량 외곽을 과하게 벗어나는지 여부

---

## 6. 현재 UE 적용 구조

### 6.1 현재 프로젝트 기준

현재 프로젝트의 P0 터렛 시각 구조는 별도 터렛 Actor BP가 아니라 `BP_CFVehiclePawn` 내부 컴포넌트 체인이다.

따라서 이 문서에서는 `BP_CFTurret_Proto` 같은 별도 Actor Blueprint를 기준으로 삼지 않는다.

### 6.2 현재 컴포넌트 구조

현재 기준 구조는 다음과 같다.

```text
BP_CFVehiclePawn / ACFVehiclePawn
  OwnerVisualRoot
    Turret_MountRoot
      Turret_BaseMesh
        Turret_YawPivot
          Turret_YawMesh
            Turret_PitchPivot
              Turret_PitchMesh
```

### 6.3 컴포넌트 역할

| 컴포넌트 | 역할 |
|---|---|
| `OwnerVisualRoot` | 차량 외형 표시 루트 |
| `Turret_MountRoot` | 하드포인트 위치에 배치되는 터렛 전체 장착 루트 |
| `Turret_BaseMesh` | 차량에 고정되는 터렛 하부 베이스 메시 |
| `Turret_YawPivot` | 좌우 Yaw 회전을 적용할 가상 피벗 |
| `Turret_YawMesh` | Yaw 회전하는 터렛 몸체 메시 |
| `Turret_PitchPivot` | 포신 Pitch 회전을 적용할 가상 피벗 |
| `Turret_PitchMesh` | Pitch 회전하는 포신 또는 포신 포함 상부 메시 |

### 6.4 현재 데이터 흐름

현재 터렛 시각 메시 적용 흐름은 다음과 같다.

```text
HeavyCannon
  → DefaultTurretMountData = DA_CannonBody
    → TurretBaseMesh = Base
    → TurretYawMesh = CannonBody
    → TurretPitchMesh = HeavyBarrel
```

확인된 현재 에셋은 다음과 같다.

| 데이터 | 현재 값 |
|---|---|
| 장비 프리셋 | `/Game/CarFight/Weapons/HeavyCannon.HeavyCannon` |
| 터렛 마운트 데이터 | `/Game/CarFight/Weapons/Data/DA_CannonBody.DA_CannonBody` |
| Base 메시 | `/Game/CarFight/Weapons/Turrets/Base.Base` |
| Yaw 메시 | `/Game/CarFight/Weapons/Turrets/CannonBody.CannonBody` |
| Pitch 메시 | `/Game/CarFight/Weapons/Turrets/HeavyBarrel.HeavyBarrel` |

### 6.5 공용 베이스 데이터 해석

현재 `DA_CannonBody` 안에 `TurretBaseMesh`가 들어가 있더라도, P0 원화 기준에서는 이를 무기별 신규 제작 메시가 아니라 공용 베이스 참조로 해석한다.

권장 장기 구조는 다음과 같다.

```text
TurretMountProfile 또는 TurretBaseProfile
  → DefaultBaseMesh = TurretBase_Standard
  → YawPivotSocketName = YawPivot

Weapon 또는 TurretVisualData
  → TurretBaseMeshOverride = 선택값
  → TurretYawMesh = 무기별 메시
  → TurretPitchMesh = 무기별 메시
```

`TurretBaseMeshOverride`가 비어 있으면 공용 `TurretBase_Standard`를 사용한다.

---

## 7. 원화 파츠 분리 기준

### 7.1 런타임 필수 MeshComponent 구성

P0 Roof Turret Cannon의 런타임 시각 구조는 3개 MeshComponent를 기본으로 한다.

```text
TurretBaseMesh
TurretYawMesh
TurretPitchMesh
```

이 3단 구조는 UE 적용, 피벗 구성, 회전 적용의 기본 단위다.

단, 이 말은 무기마다 3개 메시를 전부 새로 제작해야 한다는 뜻이 아니다.

### 7.2 컨셉이미지와 Tripo 입력 이미지의 파츠 표시 기준

P0 Roof Turret Cannon은 컨셉이미지와 Tripo 입력 이미지를 서로 다른 목적으로 제작한다.

| 이미지 종류 | 표시 대상 | 목적 |
|---|---|---|
| 컨셉이미지 시트 | 완성 터렛 전체의 쿼터뷰 + Front / Back / Left / Right 4뷰 | 사람이 전체 실루엣, 입체감, 비례, 조립 상태, 차량 장착 감각을 검토 |
| Tripo 입력용 쿼터뷰 + 4뷰 시트 | `TurretBaseMesh` 단독 쿼터뷰 + 4뷰 | 공용 표준 베이스 메시 생성 또는 재생성 |
| Tripo 입력용 쿼터뷰 + 4뷰 시트 | `TurretYawMesh` 단독 쿼터뷰 + 4뷰 | Yaw 회전 몸체 메시 생성 |
| Tripo 입력용 쿼터뷰 + 4뷰 시트 | `TurretPitchMesh` 단독 쿼터뷰 + 4뷰 | Pitch 회전 포신 메시 생성 |

컨셉이미지는 분해도가 아니다.

컨셉이미지에서 `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh`를 서로 떨어뜨려 보여주면 전체 터렛 실루엣, 높이, 회전 반경, 포신 위치를 검수하기 어렵다.

따라서 컨셉이미지는 반드시 아래처럼 완성형 조립 상태를 기준으로 한다.

```text
TurretBaseMesh
  + TurretYawMesh
    + TurretPitchMesh
= 완성 루프 터렛 전체
```

반대로 Tripo 입력용 이미지는 완성 터렛 전체가 아니라, 각 메시 파츠를 따로 뽑는다.

토큰/이미지 생성 비용을 줄이되 입체감도 확인하기 위해, 모든 이미지 시트는 쿼터뷰 1개와 Front / Back / Left / Right 4개 뷰를 한 장에 함께 배치한다.

```text
이미지 시트 세트:
1. CompleteTurret_ConceptSheet 1장
2. TurretBaseMesh_InputSheet 1장
3. TurretYawMesh_InputSheet 1장
4. TurretPitchMesh_InputSheet 1장
```

모든 이미지 시트의 기본 배치 기준은 다음과 같다.

```text
Left Large Area: Quarter View
Right Top Left: Front View
Right Top Right: Back View
Right Bottom Left: Left View
Right Bottom Right: Right View
```

`TurretBaseMesh`는 무기별 본체가 아니라 공용 표준 베이스로 재사용할 수 있지만, 입력 이미지 기준에서는 별도 단일 오브젝트 쿼터뷰 + 4뷰 시트로 제작한다.

### 7.3 메시별 역할

| 구분 | 실제 메시 | 역할 | 현재 UE 대응 |
|---|---|---|---|
| 공용 메시 | `TurretBaseMesh` | 차량 루프에 고정되는 표준 터렛 베이스. 회전하지 않는다. | `Turret_BaseMesh` |
| 무기별 메시 | `TurretYawMesh` | YawPivot 기준으로 전방향 방위각 조준을 담당하는 회전 몸체 메시. | `Turret_YawMesh` |
| 무기별 메시 | `TurretPitchMesh` | PitchPivot 기준으로 상하 회전하는 포신 메시. 포구 형상까지 포함한다. | `Turret_PitchMesh` |

### 7.4 공용 베이스 기준

P0 기준 공용 터렛 베이스는 다음 기준을 따른다.

```text
기본 공용 베이스:
TurretBase_Standard
```

공용 베이스는 아래 역할을 가진다.

1. `Top_01` 하드포인트와 터렛 본체를 연결한다.
2. 차량 루프에 고정된다.
3. `YawPivot` 기준 위치를 안정적으로 제공한다.
4. 무기별 디자인보다 하드포인트 규격을 먼저 보여준다.
5. 차량보다 먼저 읽히지 않는 낮고 단순한 형태를 유지한다.

장기적으로는 아래 등급을 둘 수 있다.

```text
TurretBase_Small
TurretBase_Standard
TurretBase_Heavy
```

하지만 P0에서는 `TurretBase_Standard` 1종을 기본값으로 둔다.

### 7.5 베이스 예외 처리 기준

특수 무기나 크기 등급이 다른 무기를 위해 베이스 Override 가능성은 열어둔다.

```text
기본:
TurretBaseMesh = 공용 TurretBase_Standard 사용

예외:
무기 데이터가 TurretBaseMeshOverride를 가지면 해당 무기 전용 베이스 사용 가능
```

단, P0 Roof Turret Cannon 원화에서는 전용 베이스를 새로 만드는 것을 기본 목표로 삼지 않는다.

### 7.6 형태 요소와 실제 메시의 관계

원화에서는 아래 형태 요소가 읽혀야 하지만, 이 요소들이 모두 별도 메시일 필요는 없다.

| 형태 요소 | 실제 처리 기준 |
|---|---|
| `MountFootprint` | 공용 `TurretBaseMesh` 안에 포함되는 장착 바닥면 형태 |
| `TurretBase` | 공용 `TurretBaseMesh`의 본체 |
| `YawHousing` | 무기별 `TurretYawMesh`의 본체 |
| `BarrelHinge` | `TurretYawMesh`와 `TurretPitchMesh` 사이의 힌지/축 형태 |
| `Barrel` | 무기별 `TurretPitchMesh`의 본체 |
| `Muzzle` | `TurretPitchMesh` 끝단의 포구 형상과 `Muzzle` 소켓 위치 |

### 7.7 선택 형태 요소

아래 요소는 원화에서 표현할 수 있지만, 가능하면 2개 무기별 메시 안에 포함한다.

| 형태 요소 | 처리 기준 |
|---|---|
| `RearBlock` | `TurretYawMesh`에 포함 가능. 후방 장전부/기계부 덩어리. |
| `SensorBlock` | `TurretYawMesh`에 포함 가능. 작은 조준 센서 또는 상태등. |
| `MuzzleBrake` | `TurretPitchMesh` 끝단 형상으로 포함 가능. 과도하게 복잡하면 금지. |
| `MountBolts` | 공용 베이스의 텍스처/노멀 우선. 무기별 메시로 만들지 않는다. |

### 7.8 파츠별 회전 기준

| 구분 | 실제 메시/기준점 | 움직임 |
|---|---|---|
| 고정부 | 공용 `TurretBaseMesh` | 차량에 고정 |
| Yaw 회전부 | 무기별 `TurretYawMesh` | 전방향 방위각 조준 가능 |
| Pitch 회전부 | 무기별 `TurretPitchMesh` | 포신 상하 회전 |
| 기준점 | `YawPivot`, `PitchPivot`, `Muzzle` 소켓 | 회전/발사 기준 |

### 7.9 수동 Blender 모델링 비전제

이 문서는 사용자가 Blender에서 직접 모델링하거나 메시를 수동으로 자르는 작업을 하지 않는다는 전제를 따른다.

즉, 아래 작업은 기본 파이프라인에 포함하지 않는다.

1. 수동 메시 분리
2. 수동 리토폴로지
3. 수동 버텍스 이동
4. 수동 면 정리
5. 수동 피벗 재설정
6. 수동 브래킷 모델링
7. 수동 디테일 추가

Blender는 다음 용도로만 사용한다.

1. Tripo 결과 메시 로드
2. 제공된 스크립트 실행
3. 자동 정리 결과 확인
4. FBX 또는 필요한 포맷으로 Export

### 7.10 Tripo 입력용 완성 터렛 이미지 금지

P0 루프 터렛 메시 생성에서는 완성 터렛 전체를 하나의 Tripo 입력 세트로 넣어 한 번에 생성하는 방식을 기본으로 사용하지 않는다.

금지 예시는 다음과 같다.

```text
Base + Yaw Body + Pitch Body가 모두 결합된 완성 터렛 전체를
Tripo 입력용 이미지로 사용해 한 번에 3D 생성하는 방식
```

이 방식은 현재 작업 환경에서 아래 문제가 있다.

1. Base / Yaw / Pitch 분리가 어려워진다.
2. 사용자가 Blender에서 수동 분리할 수 없다.
3. 공용 베이스 재사용 구조와 충돌한다.
4. 피벗/소켓 기준 정렬이 어려워진다.

단, 위 금지는 Tripo 입력용 이미지에만 적용된다.

컨셉이미지는 오히려 완성 터렛 전체를 조립 상태로 보여줘야 한다.

```text
컨셉이미지:
- 완성 터렛 전체 표시
- Base + YawMesh + PitchMesh 조립 상태 표시

Tripo 입력용 이미지:
- Base 쿼터뷰 + 4뷰 단일 이미지 시트 1장
- YawMesh 쿼터뷰 + 4뷰 단일 이미지 시트 1장
- PitchMesh 쿼터뷰 + 4뷰 단일 이미지 시트 1장
```

### 7.11 Tripo 생성 단위

P0 루프 터렛 기준에서 Tripo의 실제 생성 단위는 다음과 같다.

| 구분 | 생성 방식 | 재사용 해석 |
|---|---|---|
| `TurretBaseMesh` | Tripo에서 별도 생성 | 공용 `TurretBase_Standard` 후보로 재사용 가능 |
| `TurretYawMesh` | Tripo에서 별도 생성 | P0 Roof Turret Cannon 전용 회전 몸체 |
| `TurretPitchMesh` | Tripo에서 별도 생성 | P0 Roof Turret Cannon 전용 포신 |

즉, P0 루프 터렛의 Tripo 입력 작업은 기본적으로 3개 파츠 기준이다.

```text
1회차: TurretBaseMesh 생성
2회차: TurretYawMesh 생성
3회차: TurretPitchMesh 생성
```

`Muzzle`은 별도 메시가 아니라 `TurretPitchMesh`의 소켓 기준점으로 처리한다.

---

## 8. 피벗/소켓 기준

### 8.1 필수 소켓 구성

P0 Roof Turret Cannon의 필수 소켓은 3개다.

```text
YawPivot
PitchPivot
Muzzle
```

이 소켓은 별도 메시가 아니라, 3개 필수 메시 사이의 회전/발사 기준점이다.

### 8.2 피벗/소켓 기준

| 기준 | 배치 대상 | 위치 의미 |
|---|---|---|
| `YawPivot` | `TurretBaseMesh` | `TurretYawMesh`가 좌우 회전할 중심 |
| `PitchPivot` | `TurretYawMesh` | `TurretPitchMesh`가 상하 회전할 중심 |
| `Muzzle` | `TurretPitchMesh` | 실제 발사 위치와 발사 방향 기준 |

### 8.3 현재 소켓 기준

현재 `DA_CannonBody` 기준 소켓 이름은 다음과 같다.

| 소켓 | 현재 이름 | 위치 의미 |
|---|---|---|
| Yaw 피벗 소켓 | `YawPivot` | Base 메시 위의 좌우 회전 중심 |
| Pitch 피벗 소켓 | `PitchPivot` | Yaw 메시에서 포신이 상하 회전하는 중심 |
| Muzzle 소켓 | `Muzzle` | Pitch 메시 끝 발사 위치 |

### 8.4 Blender 정리 기준

Tripo로 생성한 메시를 Blender에서 정리할 때는 아래 기준을 따른다.

1. 차량 Forward는 `+X`로 본다.
2. 차축 방향은 `+Y`로 본다.
3. 위쪽은 `+Z`로 본다.
4. `TurretBaseMesh`의 바닥면은 차량 루프에 닿는 평면으로 정리한다.
5. `YawPivot`은 `TurretBaseMesh` 중심 위에 둔다.
6. `PitchPivot`은 `TurretYawMesh` 전방의 포신 힌지 중심에 둔다.
7. `Muzzle`은 `TurretPitchMesh` 끝 중앙에 둔다.
8. 작은 볼트, 얇은 케이블, 미세 패널선은 가능하면 텍스처/노멀로 넘긴다.

---

## 9. 실루엣 기준

### 9.1 권장 실루엣

P0 Roof Turret Cannon은 다음 실루엣을 우선한다.

```text
낮은 회전 베이스
+ 단순한 장갑 하우징
+ 단일 대구경 포신
+ 명확한 포구
```

### 9.2 형태 방향

권장 형태는 다음과 같다.

1. 낮고 넓은 하부 베이스
2. 회전할 것처럼 보이는 상부 몸체
3. 단일 포신
4. 포신 힌지가 읽히는 구조
5. 포구가 명확한 구조
6. 후방 장전부가 너무 길지 않은 구조
7. 25% 축소 상태에서도 루프 터렛임을 알 수 있는 구조

### 9.3 피해야 할 실루엣

아래 형태는 금지한다.

1. 사람이 들 수 있을 것 같은 총기 형태
2. 전차 포탑처럼 큰 쐐기형 장갑 포탑
3. 전차 주포처럼 과대하게 긴 포신
4. APC/IFV 원격무장장치처럼 실제 군용 장비에 가까운 형태
5. 미사일 포드처럼 보이는 다연장 박스 구조
6. 루프 전체를 덮는 무기 플랫폼
7. 차량보다 먼저 읽히는 대형 터렛
8. 복잡한 노출 기어/케이블이 전체 인상을 지배하는 형태

---

## 10. 컨셉이미지 형식

### 10.1 컨셉이미지의 표시 대상

P0 Roof Turret Cannon 컨셉이미지는 완성 터렛 전체를 표시한다.

컨셉이미지에 표시되어야 하는 대상은 다음이다.

```text
TurretBaseMesh + TurretYawMesh + TurretPitchMesh
```

즉, 컨셉이미지는 아래 세 파츠가 실제 장착 상태처럼 조립된 하나의 터렛을 보여줘야 한다.

1. 차량 루프에 고정되는 `TurretBaseMesh`
2. 베이스 위에서 Yaw 회전하는 `TurretYawMesh`
3. YawMesh 전방 힌지에 붙어 Pitch 회전하는 `TurretPitchMesh`

컨셉이미지는 Tripo 입력용 분리 이미지가 아니다.

따라서 컨셉이미지에서 세 파츠를 따로 떨어뜨린 분해도처럼 보여주는 것은 기본 기준이 아니다.

### 10.2 필수 구성

P0 Roof Turret Cannon 컨셉이미지는 한 장 안에 아래 구성을 모두 포함한다.

| 구성 | 필수 여부 | 목적 |
|---|---|---|
| Quarter View | 필수 | 완성 터렛의 입체감, 상부 인상, 전체 볼륨 확인 |
| Front View | 필수 | 완성 터렛의 전체 폭, 포구, 좌우 대칭 확인 |
| Back View | 필수 | 완성 터렛의 후방 장전부/기계부 부피 확인 |
| Left View | 필수 | 포신 길이, 전체 높이, Pitch 구조 확인 |
| Right View | 필수 | 좌우 측면 실루엣 일관성 확인 |

### 10.3 필수 레이아웃

컨셉이미지 시트의 기본 레이아웃은 다음으로 통일한다.

```text
Left Large Area: Quarter View
Right Top Left: Front View
Right Top Right: Back View
Right Bottom Left: Left View
Right Bottom Right: Right View
```

### 10.4 선택 요소

아래 요소는 선택이다.

| 요소 | 용도 |
|---|---|
| Detail Callout | 소켓/피벗/파츠 경계 설명. 단, 본체는 조립 상태 유지 |
| Vehicle Mount Preview | Compact 차량 루프 위 장착 검수 |
| Top View Reference | 전방향 조준 회전 반경과 루프 간섭 추가 검수 |
| Exploded Reference | 필요 시 보조 자료로만 사용. 기본 컨셉이미지로 사용하지 않음 |

### 10.5 Top View Reference 표시 기준

Top View Reference는 필수 기본 구성은 아니지만, 전방향 조준 회전 반경과 루프 간섭을 추가 검수할 때 사용할 수 있다.

Top View Reference에는 다음 정보가 읽혀야 한다.

1. 터렛 중심점
2. Base 외곽
3. Yaw Body 외곽
4. 포신 방향
5. 포신 회전 반경
6. 전방/측면/후방 조준 가능성
7. 차량 루프/장갑/센서와의 간섭 가능성

Top View Reference를 사용할 경우 다음 표기 문구를 넣을 수 있다.

```text
Full Azimuth Coverage
Clear roof rotation space
No major roof interference
```

---

## 11. Tripo 입력 기준

### 11.1 상위 입력 이미지 규칙

P0 Roof Turret Cannon의 Tripo 입력용 이미지는 `Document/Plan/ConceptArt/17_TripoInputImageRules.md`의 범용 규칙을 따른다.

이 문서에서는 터렛 전용 예외와 적용 방식을 보강한다.

### 11.2 Tripo 입력용 이미지의 표시 대상

Tripo 입력용 이미지는 완성 터렛 전체가 아니라, 실제 생성할 메시 파츠 하나만 표시한다.

P0 Roof Turret Cannon 기준 Tripo 입력용 이미지 세트는 다음 3종이다.

| 입력 이미지 세트 | 표시 대상 | 표시 방식 |
|---|---|---|
| `TurretBaseMesh_Input` | `TurretBaseMesh`만 표시 | 베이스 단독 오브젝트 |
| `TurretYawMesh_Input` | `TurretYawMesh`만 표시 | Yaw 회전 몸체 단독 오브젝트 |
| `TurretPitchMesh_Input` | `TurretPitchMesh`만 표시 | Pitch 포신 단독 오브젝트 |

각 입력 이미지 세트는 쿼터뷰 1개와 Front / Back / Left / Right 4뷰를 한 장 안에 넣은 단일 이미지 시트로 준비한다.

```text
TurretBaseMesh_Input:
- TurretBaseMesh_InputSheet 1장
  - Left Large Area: Quarter View
  - Right Top Left: Front View
  - Right Top Right: Back View
  - Right Bottom Left: Left View
  - Right Bottom Right: Right View

TurretYawMesh_Input:
- TurretYawMesh_InputSheet 1장
  - Left Large Area: Quarter View
  - Right Top Left: Front View
  - Right Top Right: Back View
  - Right Bottom Left: Left View
  - Right Bottom Right: Right View

TurretPitchMesh_Input:
- TurretPitchMesh_InputSheet 1장
  - Left Large Area: Quarter View
  - Right Top Left: Front View
  - Right Top Right: Back View
  - Right Bottom Left: Left View
  - Right Bottom Right: Right View
```

즉, P0 Roof Turret Cannon의 Tripo 입력용 이미지는 총 3장을 기본으로 한다.

```text
1. Base 쿼터뷰 + 4뷰 시트 1장
2. YawMesh 쿼터뷰 + 4뷰 시트 1장
3. PitchMesh 쿼터뷰 + 4뷰 시트 1장
```

### 11.3 Tripo용 원화 목표

Tripo에 입력할 원화는 다음 조건을 만족해야 한다.

1. 배경은 단순해야 한다.
2. 그림자는 완전히 없어야 한다.
3. 접지 그림자, 그라데이션 그림자, 강한 명암 음영을 모두 넣지 않는다.
4. 조명은 shadowless lighting 기준으로 균일해야 한다.
5. 원화는 Tripo가 해석하기 쉬운 단일 오브젝트 쿼터뷰 + 4뷰 이미지 시트여야 한다.
6. 한 장 안의 Quarter / Front / Back / Left / Right 뷰가 같은 오브젝트로 일관되어야 한다.
7. 쿼터뷰는 왼쪽 큰 영역에 배치하고, Front / Back / Left / Right는 오른쪽 2x2 그리드 안에 고정된 순서로 배치한다.
8. `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh`가 각각 독립 오브젝트 쿼터뷰 + 4뷰 시트로 준비되어야 한다.
9. `TurretBaseMesh`는 차량 루프에 닿는 하부 장착면이 읽혀야 한다.
10. `TurretYawMesh`는 공용 베이스 위에 올라갈 하부 연결면과 `PitchPivot` 위치가 읽혀야 한다.
11. `TurretPitchMesh`는 포신 방향, 힌지 연결부, `Muzzle` 위치가 읽혀야 한다.
12. 작은 디테일보다 큰 덩어리와 피벗/소켓 구조가 우선이어야 한다.
13. `Muzzle`은 별도 파츠가 아니라 `TurretPitchMesh` 끝단의 소켓 위치로 읽혀야 한다.
14. `YawPivot`, `PitchPivot`, `Muzzle` 위치가 예상 가능해야 한다.
15. 4뷰 시트 안에는 텍스트 라벨, 치수선, 콜아웃을 넣지 않는 것을 기본으로 한다.

### 11.4 컨셉이미지와 Tripo 입력용 이미지의 구분

이 문서는 컨셉이미지와 Tripo 입력용 이미지를 구분한다.

| 구분 | 목적 | 구성 |
|---|---|---|
| 컨셉이미지 | 사람이 완성 터렛의 구조와 실루엣을 검토 | Base + YawMesh + PitchMesh가 조립된 완성 터렛 전체 |
| Tripo 입력용 이미지 | 실제 Tripo 생성 입력 | Base 쿼터뷰 + 4뷰 시트 1장 / YawMesh 쿼터뷰 + 4뷰 시트 1장 / PitchMesh 쿼터뷰 + 4뷰 시트 1장 |

Tripo 입력 이미지에는 다음을 넣지 않는다.

1. 완성 터렛 전체 조립 상태
2. 콜아웃 텍스트
3. 치수선
4. 설명 문장
5. 파츠 이름 표기
6. 여러 오브젝트 동시 배치
7. 차량과 함께 찍힌 상태
8. 모든 종류의 그림자
9. 접지 그림자
10. 그라데이션 그림자
11. 강한 명암 음영
12. 복잡한 배경

### 11.5 Tripo 생성 후 작업 방식

Tripo 생성 후에는 사용자가 Blender에서 수동 모델링을 하지 않는다.

작업 흐름은 다음과 같다.

```text
1. Tripo 결과 메시 로드
2. 제공된 Blender 스크립트 실행
3. 자동 정렬/원점/스케일/기본 이름 정리
4. Export
```

이 문서는 수동 메시 분리나 수동 리토폴로지를 전제로 하지 않는다.

---

## 12. AI 원화 프롬프트 기준

### 12.1 컨셉이미지 프롬프트 기준

컨셉이미지는 완성 터렛 전체를 조립 상태로 보여주는 이미지다.

컨셉이미지 프롬프트에는 `Base`, `YawMesh`, `PitchMesh`가 서로 떨어진 분해 파츠가 아니라, 하나의 완성된 루프 터렛으로 조립되어 있다는 조건을 명확히 넣는다.

```text
vehicle-mounted roof turret weapon,
assembled complete roof turret,
Base + YawMesh + PitchMesh combined into one complete turret,
large caliber slow turret cannon,
simplified near-future civilian combat car weapon,
P0 roof-mounted turret cannon,
Full Azimuth Coverage,
C1 simple game asset concept,
complete turret concept image sheet,
one large quarter view,
front view, back view, left view, right view,
plain background,
no shadows,
no cast shadow,
no contact shadow,
no ground shadow,
no gradient shadow,
shadowless lighting,
complete assembled turret silhouette,
low fixed turret base attached under yaw housing,
compact armored yaw housing mounted on base,
single heavy barrel attached to front pitch hinge,
clear yaw rotation axis,
clear barrel pitch hinge,
clear muzzle,
large simple hard-surface shapes,
low modeling complexity,
easy pivot and socket planning,
matte dark gunmetal,
matte armor panels,
small limited sensor detail,
not exploded view,
not separated parts layout,
no loose parts,
no handheld firearm,
no tank-style oversized cannon,
no tank turret,
no APC weapon station,
no oversized turret,
no multiple turrets,
no missile pods,
no side weapon pods,
no tiny mechanical parts,
no complex exposed cables,
no detailed mechanical gears,
no real military vehicle,
no logo,
no cinematic render,
no action scene,
no dramatic perspective,
no complex background
```

### 12.2 Tripo 입력용 Base 프롬프트 기준

`TurretBaseMesh` 입력 이미지는 베이스만 단독으로 표시한다.

```text
single object only,
vehicle roof turret base mesh,
standard reusable turret base,
low fixed mechanical base,
flat bottom mounting surface,
wide stable circular or low polygonal base,
clear center yaw pivot location,
simple hard-surface shape,
low modeling complexity,
AI 3D modeling ready,
single image sheet with one large quarter view and one 2x2 orthographic view grid, left large quarter view, right top left front view, right top right back view, right bottom left left view, right bottom right right view,
plain background,
no shadows,
no cast shadow,
no contact shadow,
no ground shadow,
no gradient shadow,
shadowless lighting,
no yaw housing,
no barrel,
no complete turret,
no text,
no callouts,
no dimension lines,
no vehicle,
no complex cables,
no tiny mechanical details
```

### 12.3 Tripo 입력용 YawMesh 프롬프트 기준

`TurretYawMesh` 입력 이미지는 Yaw 회전 몸체만 단독으로 표시한다.

```text
single object only,
vehicle roof turret yaw housing mesh,
compact armored rotating turret body,
mounts on standard turret base,
clear lower connection surface,
clear pitch hinge position at front,
large simple hard-surface shape,
low modeling complexity,
AI 3D modeling ready,
single image sheet with one large quarter view and one 2x2 orthographic view grid, left large quarter view, right top left front view, right top right back view, right bottom left left view, right bottom right right view,
plain background,
no shadows,
no cast shadow,
no contact shadow,
no ground shadow,
no gradient shadow,
shadowless lighting,
no turret base,
no barrel mesh,
no complete turret,
no text,
no callouts,
no dimension lines,
no vehicle,
no complex exposed cables,
no detailed mechanical gears
```

### 12.4 Tripo 입력용 PitchMesh 프롬프트 기준

`TurretPitchMesh` 입력 이미지는 포신/Pitch 회전 파츠만 단독으로 표시한다.

```text
single object only,
large caliber turret barrel mesh,
single heavy barrel,
clear rear hinge connection,
clear forward muzzle,
short-to-medium heavy cannon barrel,
simple muzzle shape,
large simple hard-surface form,
low modeling complexity,
AI 3D modeling ready,
single image sheet with one large quarter view and one 2x2 orthographic view grid, left large quarter view, right top left front view, right top right back view, right bottom left left view, right bottom right right view,
plain background,
no shadows,
no cast shadow,
no contact shadow,
no ground shadow,
no gradient shadow,
shadowless lighting,
no turret base,
no yaw housing,
no complete turret,
no text,
no callouts,
no dimension lines,
no vehicle,
no multiple barrels,
no missile pod,
no complex exposed cables
```

### 12.5 프롬프트 금지어/주의어

아래 표현은 사용하지 않거나 매우 조심해서 사용한다.

| 표현 | 이유 |
|---|---|
| `tank cannon` | 전차 주포처럼 과대 생성될 가능성이 높음 |
| `military turret` | APC/IFV/전차 무장처럼 생성될 가능성이 높음 |
| `realistic weapon system` | 실제 군용 장비 느낌이 강해질 수 있음 |
| `highly detailed mechanical parts` | Tripo 후처리 난이도가 올라감 |
| `exposed cables` | 작은 케이블 과생성 가능 |
| `multiple barrels` | P0 단일 대구경 포신 기준과 충돌 |
| `missile pod` | P0 Roof Turret Cannon이 런처처럼 보일 수 있음 |
| `exploded view` | 기본 컨셉이미지에서 분해도처럼 생성될 수 있음 |
| `separated parts layout` | 컨셉이미지가 조립 상태가 아니라 분해 파츠처럼 생성될 수 있음 |

---

## 13. 검수 체크리스트

### 13.1 컨셉이미지 1차 검수

컨셉이미지는 아래 질문에 모두 `예`가 나와야 한다.

1. 차량용 루프 터렛처럼 보이는가?
2. `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh`가 조립된 완성 터렛 전체로 보이는가?
3. 분해도처럼 파츠가 서로 떨어져 보이지 않는가?
4. 사람이 들고 쓰는 총기처럼 보이지 않는가?
5. 전차/APC/IFV 무장처럼 보이지 않는가?
6. Base, Yaw Body, Pitch Barrel의 위치 관계가 자연스러운가?
7. `TurretBaseMesh`는 차량 루프에 고정되는 하부 베이스처럼 보이는가?
8. `TurretYawMesh`는 베이스 위에서 전방향 방위각 조준을 담당하는 회전 몸체처럼 보이는가?
9. `TurretPitchMesh`는 상하 회전하는 단일 포신처럼 보이는가?
10. `YawPivot` 위치를 예상할 수 있는가?
11. `PitchPivot` 위치를 예상할 수 있는가?
12. `Muzzle` 소켓을 둘 포구 중심이 명확한가?
13. Top View 검수 기준에서 전방향 방위각 조준 가능성이 읽히는가?
14. 포신이 후방을 향해도 루프 파츠와 심각하게 충돌하지 않는가?
15. 차량보다 무기가 먼저 읽히지 않는가?
16. Compact급 차량에 올렸을 때 과대해 보이지 않는가?
17. 작은 볼트/케이블/패널선이 과도하지 않은가?
18. Tripo가 큰 덩어리 기준으로 메시를 만들 수 있을 만큼 각 파츠의 형태가 단순한가?
19. Blender 수동 수정 없이도 스크립트 기반 정리가 가능할 정도로 구조가 단순한가?

### 13.2 Tripo 입력 이미지 검수

Tripo 입력용 이미지는 아래 질문에 모두 `예`가 나와야 한다.

1. `TurretBaseMesh_Input`에는 베이스만 단독으로 표시되는가?
2. `TurretYawMesh_Input`에는 Yaw 회전 몸체만 단독으로 표시되는가?
3. `TurretPitchMesh_Input`에는 Pitch 포신만 단독으로 표시되는가?
4. 각 입력 이미지 세트가 4장 분리 이미지가 아니라, 쿼터뷰 + 4뷰 단일 이미지 시트 1장으로 준비되었는가?
5. 이미지 시트 안에 Quarter / Front / Back / Left / Right가 모두 들어가 있는가?
6. 시트 배치가 Left Large Area = Quarter, Right Top Left = Front, Right Top Right = Back, Right Bottom Left = Left, Right Bottom Right = Right 순서를 따르는가?
7. P0 기준 Tripo 입력용 이미지가 Base 1장, YawMesh 1장, PitchMesh 1장, 총 3장으로 정리되었는가?
8. Tripo 입력 이미지에 완성 터렛 전체 조립 상태가 들어가지 않았는가?
9. Tripo 입력 이미지에 컨셉이미지용 분해도나 보조 설명 이미지가 들어가지 않았는가?
10. 각 입력 이미지에 텍스트, 치수선, 콜아웃, 파츠 이름 표기가 없는가?
11. 각 입력 이미지에 차량이나 다른 오브젝트가 같이 들어가지 않았는가?
12. 각 입력 이미지의 배경, 스케일, 조명, 중심 위치가 일관되는가?
13. 각 입력 이미지에 그림자가 완전히 없는가?
14. 접지 그림자, 그라데이션 그림자, 강한 명암 음영이 모두 없는가?
15. 조명이 shadowless lighting 기준으로 균일한가?
16. 각 파츠가 단일 오브젝트처럼 명확하게 읽히는가?

### 13.3 Tripo 메시 생성 후 검수

Tripo 결과물은 아래 기준으로 검수한다.

1. `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh` 3개 메시로 정리할 수 있는가?
2. `TurretBaseMesh`는 공용 `TurretBase_Standard` 후보로 사용할 수 있는가?
3. `TurretYawMesh` 하단이 `TurretBaseMesh` 위에 자연스럽게 장착될 수 있는가?
4. `TurretPitchMesh`가 `TurretYawMesh`의 전방 Pitch 힌지에 자연스럽게 장착될 수 있는가?
5. `YawPivot` 중심을 베이스 기준으로 잡을 수 있는가?
6. `PitchPivot` 중심을 잡을 수 있는가?
7. `TurretPitchMesh` 끝에 `Muzzle` 소켓을 둘 수 있는가?
8. 포신이 너무 길거나 무겁게 생성되지 않았는가?
9. 불필요한 케이블/기어/볼트가 많지 않은가?
10. 실제 UE 컴포넌트 `Turret_BaseMesh`, `Turret_YawMesh`, `Turret_PitchMesh`에 대응 가능한가?
11. 수동 면 수정 없이 스크립트 기반 정리만으로 사용할 수 있는가?

### 13.4 UE 적용 전 검수

UE 적용 전에는 아래를 확인한다.

1. `TurretBaseMesh`를 공용 베이스로 재사용할 수 있는가?
2. `TurretYawMesh`가 베이스의 `YawPivot` 기준으로 자연스럽게 회전 가능한가?
3. `TurretPitchMesh`가 `PitchPivot` 기준으로 자연스럽게 회전 가능한가?
4. `YawPivot` 소켓 기준과 `TurretYawMesh` 하단 중심이 맞는가?
5. `PitchPivot` 소켓을 둘 위치가 명확한가?
6. `Muzzle` 소켓을 둘 위치가 명확한가?
7. 차량 Forward `+X`, 차축 `+Y`, Up `+Z` 기준에 맞는가?
8. 전방향 방위각 조준 기준과 충돌하지 않는가?
9. Blender에서 필요한 작업이 스크립트 실행과 Export 수준으로 제한되는가?
10. 컨셉이미지의 조립 상태와 실제 UE 컴포넌트 조립 순서가 일치하는가?

---

## 14. 금지 기준

아래 조건 중 하나라도 강하게 해당하면 P0 Roof Turret Cannon 원화 후보에서 제외한다.

1. 사람용 총기처럼 보임
2. 전차 주포처럼 보임
3. 전차 포탑처럼 보임
4. APC/IFV 원격무장장치처럼 보임
5. 미사일 런처처럼 보임
6. 다연장 무기처럼 보임
7. 차량 루프 전체를 덮음
8. 차량보다 무기가 먼저 읽힘
9. 측면 무기 포드가 붙음
10. 다중 터렛 구조임
11. 포신이 지나치게 길어 전방향 조준 시 차량 외곽을 심하게 벗어남
12. 노출 기어/케이블/볼트가 전체 형태를 지배함
13. `TurretBaseMesh`와 `TurretYawMesh` 경계가 보이지 않음
14. `TurretYawMesh`와 `TurretPitchMesh`의 Pitch 힌지 위치가 보이지 않음
15. `TurretPitchMesh` 끝단의 `Muzzle` 소켓 기준 위치가 불명확함
16. Tripo 후처리 시 3개 필수 메시로 정리하기 사실상 불가능함

---

## 15. 현재 구현과 문서 기준의 차이

현재 프로젝트에는 이미 P0 터렛 시각 컴포넌트 구조가 있다.

현재 구조는 다음 점에서 문서 기준과 잘 맞는다.

1. 고정 Base가 있음
2. YawPivot이 있음
3. YawMesh가 있음
4. PitchPivot이 있음
5. PitchMesh가 있음
6. Muzzle 소켓 이름이 있음
7. 메시 데이터가 `DA_CannonBody`로 분리되어 있음

다만 현재 데이터 기준에서 아래 차이가 있다.

```text
현재 DA_CannonBody:
MinYawDeg = -30
MaxYawDeg = 30

문서 목표:
P0 Roof Turret Cannon은 전방향 방위각 조준 가능
```

따라서 추후 구현 단계에서는 `DA_CannonBody` 또는 런타임 Yaw 해석을 전방향 방위각 조준 가능 기준으로 수정해야 한다.

이 문서는 특정 표현 방식 하나를 강제하지 않는다.

가능한 수정 방식은 다음 중 하나다.

1. `MinYawDeg = -180`, `MaxYawDeg = 180`
2. `MinYawDeg = 0`, `MaxYawDeg = 360`
3. Yaw 제한을 사용하지 않는 별도 플래그 추가
4. 내부적으로 연속 Yaw 회전 후 각도 정규화 처리

P0 문서 기준에서 중요한 것은 수치 표현 방식이 아니라, 전방/측면/후방 조준이 모두 가능해야 한다는 점이다.

---

## 16. 결정 사항 요약

현재 확정 기준은 다음과 같다.

1. 문서 범위는 P0 루프 터렛 원화 기준으로 제한한다.
2. 첫 제작 대상은 `P0 Roof Turret Cannon`으로 한다.
3. 한국어 명칭은 `P0 루프 장착형 대구경 저속 터렛포`로 한다.
4. 무기 크기 등급은 `Medium-Large`로 한다.
5. 기준 차량은 Compact급 민간 차량으로 둔다.
6. Sedan과 SUV는 보조 검수 차량으로 둔다.
7. 터렛은 차량보다 먼저 읽히면 안 된다.
8. 루프 터렛은 360도만 회전해야 하는 것이 아니라, 전방향 방위각 조준이 가능해야 한다.
9. Top View에서 전방/측면/후방 조준 시 터렛 회전 반경과 간섭 여부를 확인한다.
10. 현재 UE 기준은 별도 `BP_CFTurret_Proto`가 아니라 `BP_CFVehiclePawn` 내부 터렛 컴포넌트 체인이다.
11. 런타임 MeshComponent 구조는 `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh` 3단으로 유지한다.
12. 컨셉이미지는 `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh`가 조립된 완성 터렛 전체의 쿼터뷰 + Front / Back / Left / Right 4뷰를 표시한다.
13. 컨셉이미지는 기본적으로 분해도가 아니며, 파츠가 떨어진 상태를 기본 이미지로 사용하지 않는다.
14. 분해도는 필요할 때만 보조 `Exploded Reference`로 사용한다.
15. Tripo 입력용 이미지는 `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh`를 각각 단독 오브젝트 쿼터뷰 + 4뷰 이미지 시트로 표시한다.
16. Tripo 입력용 이미지는 기본적으로 Base 쿼터뷰 + 4뷰 시트 1장, YawMesh 쿼터뷰 + 4뷰 시트 1장, PitchMesh 쿼터뷰 + 4뷰 시트 1장, 총 3장으로 준비한다.
17. 각 이미지 시트는 Left Large Area = Quarter, Right Top Left = Front, Right Top Right = Back, Right Bottom Left = Left, Right Bottom Right = Right 순서를 따른다.
18. `TurretBaseMesh`는 별도 Tripo 입력 이미지로 만들 수 있지만, 결과물은 공용 `TurretBase_Standard` 후보로 재사용할 수 있다.
19. `TurretYawMesh`는 P0 Roof Turret Cannon 전용 Yaw 회전 몸체로 제작한다.
20. `TurretPitchMesh`는 P0 Roof Turret Cannon 전용 Pitch 포신으로 제작한다.
21. 완성 터렛 전체를 Tripo 입력용 이미지로 넣어 한 번에 생성하는 방식은 기본 방식으로 사용하지 않는다.
22. `Muzzle`은 별도 메시가 아니라 `TurretPitchMesh`에 배치하는 소켓으로 취급한다.
23. `MountFootprint`, `BarrelHinge`, `Muzzle`은 원화에서 읽혀야 하는 형태/기준점이지만 별도 메시 필수 항목은 아니다.
24. Blender 수동 모델링은 기본 파이프라인에 포함하지 않는다.
25. Blender는 스크립트 실행용 보조 도구로만 사용한다.
26. Tripo 입력용 이미지는 `17_TripoInputImageRules.md`의 범용 입력 이미지 기준을 따른다.
27. Tripo 입력용 이미지에는 텍스트, 치수선, 차량 동반 구성, 복잡한 배경을 넣지 않는다.
28. Tripo 입력용 이미지에는 모든 종류의 그림자, 접지 그림자, 그라데이션 그림자, 강한 명암 음영을 넣지 않는다.
29. Tripo 입력용 이미지는 shadowless lighting 기준의 균일 조명을 사용한다.
30. 전차, APC, IFV, 군용 무장 플랫폼, 사람용 총기, 미사일 포드처럼 보이는 형태는 금지한다.
31. 작은 볼트/케이블/패널선은 텍스처/노멀 우선으로 처리한다.
32. 현재 `DA_CannonBody`의 `-30 ~ 30` Yaw 제한은 문서 목표와 차이가 있으므로 추후 데이터 또는 런타임 해석 수정 대상으로 둔다.

---

## 17. Changelog

### v0.8 - 2026-07-08

- Tripo 입력용 이미지의 그림자 금지 기준을 추가.
- 접지 그림자, 그라데이션 그림자, 강한 명암 음영을 모두 금지.
- AI 원화 프롬프트에 `no shadows`, `no cast shadow`, `no contact shadow`, `no ground shadow`, `no gradient shadow`, `shadowless lighting` 조건 추가.
- Tripo 입력 이미지 검수 기준에 그림자 없음 검수 항목 추가.
- 결정 사항 요약에 shadowless lighting 기준의 균일 조명 사용 원칙 추가.

### v0.7 - 2026-07-08

- 모든 이미지 시트를 쿼터뷰 + Front / Back / Left / Right 4뷰 구성으로 통일.
- 컨셉이미지 시트에 완성 터렛 쿼터뷰와 완성 터렛 4뷰를 함께 포함하도록 수정.
- `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh` 입력 시트도 각각 쿼터뷰 + 4뷰 구성으로 수정.
- 기존 4뷰 단일 이미지 시트 기준을 쿼터뷰 + 4뷰 단일 이미지 시트 기준으로 확장.
- 프롬프트, 검수 체크리스트, Migration 기준을 새 레이아웃으로 갱신.

### v0.6 - 2026-07-08

- Tripo 입력용 이미지를 각 뷰 4장 분리 방식에서 파츠별 4뷰 단일 이미지 시트 방식으로 변경.
- `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh` 각각 1장씩, 총 3장의 Tripo 입력 이미지 기준으로 수정.
- 4뷰 단일 이미지 시트의 배치 순서를 Top Left = Front, Top Right = Back, Bottom Left = Left, Bottom Right = Right로 정의.
- Tripo 입력 프롬프트를 `single 2x2 orthographic view sheet` 기준으로 수정.
- Tripo 입력 이미지 검수 기준을 4장 분리 기준에서 4뷰 단일 이미지 시트 기준으로 수정.

### v0.5 - 2026-07-08

- 컨셉이미지의 표시 대상을 Base + YawMesh + PitchMesh가 조립된 완성 터렛 전체로 재정의.
- 컨셉이미지는 기본적으로 분해도가 아니며, 분해도는 보조 `Exploded Reference`로만 허용하도록 명시.
- Tripo 입력용 이미지의 표시 대상을 `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh` 각각 단독 오브젝트로 재정의.
- 기존 `TurretYawMesh`, `TurretPitchMesh` 2개 입력 기준을 Base/Yaw/Pitch 3개 입력 기준으로 수정.
- 컨셉이미지 프롬프트와 Tripo 입력용 Base/YawMesh/PitchMesh 파츠별 프롬프트를 분리.
- 검수 체크리스트를 컨셉이미지 검수, Tripo 입력 이미지 검수, Tripo 메시 생성 후 검수, UE 적용 전 검수로 재정리.

### v0.4 - 2026-07-08

- 사용자가 Blender 수동 모델링을 하지 않는다는 전제를 문서에 반영.
- Blender를 스크립트 실행용 보조 도구로만 사용하는 기준 추가.
- 전체 터렛 단일 생성 금지 기준 추가.
- Tripo 생성 대상을 `TurretYawMesh`, `TurretPitchMesh` 2개로 분리하는 기준 명시.
- 문서/검수용 이미지와 Tripo 입력용 이미지를 구분하는 기준 추가.
- Tripo 입력용 이미지는 `17_TripoInputImageRules.md` 범용 규칙을 따르도록 연결.
- 검수 체크리스트에 “수동 Blender 수정 비전제” 항목 추가.

### v0.3 - 2026-07-08

- `TurretBaseMesh`를 무기별 신규 제작 메시가 아니라 공용 베이스 메시로 사용하는 기준을 추가.
- P0 기본 공용 베이스를 `TurretBase_Standard`로 정의.
- 무기별 신규 제작 우선 대상을 `TurretYawMesh`, `TurretPitchMesh` 2개로 재정의.
- `TurretBaseMeshOverride`를 특수 무기/크기 등급 예외 처리용 선택값으로 명시.
- Tripo 입력 기준과 검수 체크리스트를 공용 베이스 + 무기별 2메시 기준으로 수정.

### v0.2 - 2026-07-07

- 실제 필수 메시 구성을 `TurretBaseMesh`, `TurretYawMesh`, `TurretPitchMesh` 3개로 명확히 고정.
- `Muzzle`은 별도 메시가 아니라 `TurretPitchMesh`의 `Muzzle` 소켓으로 사용하는 기준을 명시.
- `MountFootprint`, `BarrelHinge`, `Muzzle`을 별도 메시가 아닌 형태/기준점으로 재정의.
- Tripo 입력 기준, 검수 체크리스트, 결정 사항 요약을 3메시 + 3소켓 기준으로 수정.

### v0.1 - 2026-07-07

- P0 차량용 무기 원화 기준 문서 신규 작성.
- 첫 제작 대상을 `P0 Roof Turret Cannon`으로 정의.
- 루프 터렛의 전방향 방위각 조준 가능 기준 정의.
- 현재 `BP_CFVehiclePawn` 내부 터렛 시각 컴포넌트 구조 반영.
- `BP_CFTurret_Proto` 별도 Actor 기준을 문서 기준에서 제외.
- Base / Yaw Body / Barrel 3분할 원화 기준 정의.
- `YawPivot`, `PitchPivot`, `Muzzle` 소켓 기준 정의.
- Tripo 입력용 AI 프롬프트 초안 추가.
- 현재 `DA_CannonBody`의 `MinYawDeg=-30`, `MaxYawDeg=30` 제한과 문서 목표의 차이 명시.

---

## 18. Migration

### 기존 문서와의 관계

이 문서는 기존 ConceptArt 문서를 교체하지 않는다.

기존 문서의 역할은 그대로 유지한다.

| 기존 문서 | 유지 역할 |
|---|---|
| `00_ArtDirection.md` | 전체 차량/부품/무기 아트 방향 |
| `08_HardpointRules.md` | 차량 하드포인트 위치와 크기 기준 |
| `10_ConceptSheetFormat.md` | 원화 시트 형식 기준 |
| `11_AIPromptRules.md` | 차량 원화 AI 프롬프트 기본 구조 |
| `15_LoadoutBalance.md` | 차량 본체와 장비 장착 기준 분리 |

### 차량 본체 원화 기준 유지

차량 본체 원화는 계속 다음 기준을 따른다.

```text
no installed weapon
no installed turret
empty mechanical hardpoint mounts only
```

즉, 차량 본체 원화에 실제 터렛을 붙이지 않는다.

### 무기 원화 기준 추가

터렛/무기 메시를 만들 때만 이 문서를 사용한다.

```text
차량 본체 원화:
15_LoadoutBalance 기준

터렛 무기 원화:
16_WeaponArtRules 기준
```

### 구현 마이그레이션 주의

현재 구현은 `BP_CFVehiclePawn` 내부 터렛 시각 컴포넌트 구조를 사용한다.

런타임 구조는 계속 아래 3단 MeshComponent 구조를 유지한다.

```text
Turret_BaseMesh
Turret_YawMesh
Turret_PitchMesh
YawPivot socket
PitchPivot socket
Muzzle socket
```

v0.8부터는 Tripo 입력용 이미지에 그림자를 절대 넣지 않는다.

```text
no shadows
no cast shadow
no contact shadow
no ground shadow
no gradient shadow
shadowless lighting
```

그림자는 Tripo가 하부 형상, 접지면, 돌출 파츠, 구멍, 재질 변화로 오해할 수 있으므로 기본 입력 이미지에서는 모든 그림자를 금지한다.

v0.7부터는 이미지 종류별 표시 대상을 아래처럼 분리한다.

```text
컨셉이미지:
- Turret_BaseMesh + Turret_YawMesh + Turret_PitchMesh 조립 상태
- 완성 터렛 전체 쿼터뷰 + Front / Back / Left / Right 4뷰
- 기본적으로 분해도 아님

Tripo 입력용 이미지:
- Turret_BaseMesh 쿼터뷰 + 4뷰 단일 이미지 시트 1장
- Turret_YawMesh 쿼터뷰 + 4뷰 단일 이미지 시트 1장
- Turret_PitchMesh 쿼터뷰 + 4뷰 단일 이미지 시트 1장
- 각 시트는 같은 파츠의 Quarter / Front / Back / Left / Right를 표시
```

Blender/Tripo 작업 전제는 다음과 같다.

```text
- Blender 수동 모델링은 기본 파이프라인에 포함하지 않는다.
- Blender는 스크립트 실행용 보조 도구로만 사용한다.
- 완성 터렛 전체를 Tripo 입력용 이미지로 넣어 한 번에 생성하지 않는다.
- Tripo 입력용 이미지는 Base/Yaw/Pitch 3개 파츠 세트로 준비한다.
- 각 파츠는 4장 분리 이미지가 아니라 쿼터뷰 + 4뷰 단일 이미지 시트 1장으로 준비한다.
- Tripo 입력용 이미지는 17_TripoInputImageRules.md의 범용 규칙을 따른다.
```

공용 구조는 다음과 같다.

```text
공용 재사용 가능:
Turret_BaseMesh = TurretBase_Standard 후보

P0 Roof Turret Cannon 전용:
Turret_YawMesh
Turret_PitchMesh
```

기존 `/Game/CarFight/Weapons/Turrets/Base.Base`는 P0 공용 베이스 후보로 유지하거나, 추후 `TurretBase_Standard` 명명 규칙에 맞춰 재정리할 수 있다.

무기 데이터는 장기적으로 아래 해석을 따른다.

```text
TurretBaseMeshOverride가 비어 있음:
  공용 TurretBase_Standard 사용

TurretBaseMeshOverride가 지정됨:
  해당 무기 전용 베이스 사용
```

별도 터렛 Actor BP를 새로 만드는 방식은 이 문서의 기본 전제가 아니다.

추후 별도 터렛 Actor 구조로 전환할 경우, 이 문서는 `v0.9` 이상에서 Migration 항목을 추가해 갱신해야 한다.
