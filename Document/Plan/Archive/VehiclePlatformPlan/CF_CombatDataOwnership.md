# CarFight - 전투 데이터 소유권 분리

> 역할: 차량 / 장착 프로파일 / 터렛 마운트 / 무기 / 탄환 / 피해 / UI 상태 데이터의 소유권을 구현 전에 고정한다.
> 문서 버전: v0.41
> 마지막 정리(Asia/Seoul): 2026-07-03
> 상태: Draft / Combat Data Ownership / EquipmentPreset Legacy Slots Removed

---

## 1. 목적

이 문서는 전투 구현 전에 데이터가 어디에 살아야 하는지 정리한다.

현재 혼선의 핵심은 아래 데이터가 한 덩어리처럼 보인다는 점이다.

```text
차량 위치 데이터
하드포인트 장착 규칙
터렛 회전 상태
무기 발사 규칙
탄환 / Projectile 규칙
피해 / 장갑 / 모듈 규칙
UI / Debug 표시 상태
```

이 문서의 목표는 위 데이터를 서로 다른 책임으로 나누고, P0에서 어디까지 구현할지 고정하는 것이다.

---

## 2. 현재 확인된 기준

| 항목 | 현재 상태 |
|---|---|
| 현재 기본 차량 DataAsset | `DA_TestSedan` |
| 대조 테스트 DataAsset | `DA_TestSUV`, 사용자가 수동 전환 |
| 폐기된 현재 기준 | `DA_PoliceCar`는 현재 작업에 사용하지 않음 |
| 이미 있는 차량 하드포인트 데이터 | `UCFVehicleData::HardpointSlots` |
| 이미 있는 Fire 상태 타입 | `FCFVehicleFireRequest`, `FCFVehicleFireResult`, `ECFVehicleFireRejectReason` |
| 구현된 P0 전투 데이터 | 장착 프로파일, 터렛 상태 타입, FireOrigin 타입, WeaponComp 기본 해석, 최소 `UCFWeaponData`, 최소 `UCFProjectileData`, 최소 `UCFTurretMountData`, 최소 `UCFDamageData`, 최소 `UCFEquipmentPresetData`, `FCFDamageHitContext`, 공통 `ACFProjectileActor`, `UCFProjectilePoolComp`, WeaponData MaxRange / FireRatePerMinute 연결, `ProjectileData.DefaultDamageData` 단일 DamageData 참조 해석, HitScan / Laser용 가상 ProjectileData 정책, `FireMode=Projectile` Pool Acquire 실행 경로, `MountProfile.DefaultEquipmentPresetData` 전용 해석, `MountProfile.DefaultTurretMountData` / `DefaultWeaponData` legacy 직접 슬롯 삭제, MountProfile inline 터렛 fallback 런타임 제거, MountProfile 각도 제한 legacy 전환, MountProfile 각도 제한 필드 삭제 빌드 / PIE 검증, 하드포인트 기반 터렛 시각 장착 컴포넌트, Base / Yaw / Pitch 3단 소켓 장착 계층, TurretMountData 단독 터렛 Yaw / Pitch 제한, 터렛 Yaw / Pitch 시각 추적 회전, Muzzle 소켓 FireOrigin 보정, 터렛 안정화 전 발사 허용 정책, EquipmentPreset / Weapon / Projectile / Damage / Damage HitContext / Turret Visual / Turret Aim Debug 표시, Projectile Pool 마지막 반환 요약 표시, 빠른 연속 Fire 쿨다운 PIE 검증, ProjectileData 연결 전/후 PIE 검증, Projectile Actor 비행 PIE 검증, Projectile Pool PIE 검증, Projectile Pool 마지막 반환 요약 PIE 검증, Muzzle 소켓 FireOrigin PIE 검증, DamageData 단일 소유 빌드 / PIE 회귀 검증, Damage HitContext Debug 빌드 / PIE 검증, MountProfile 각도 제한 legacy 전환 빌드 / PIE 검증 |
| 현재 설계 결정 | `TurretMountData`는 무기 거치대 / 회전 플랫폼이고, `WeaponData`는 거치대 위에 얹는 실제 발사 장치다. 둘의 유효 조합은 `EquipmentPresetData`가 묶는다. |
| 아직 없는 전투 데이터 | 실제 Damage 적용, 차체 HP 차감, 장갑 방향 판정, 모듈 손상, 탄약 / 열 / 재장전 런타임 |
| 현재 우선순위 | legacy 직접 슬롯 제거 후 `EquipmentPresetData` 전용 경로 에디터 종료 후 재빌드 / PIE 회귀 검증 |

---

## 3. 최상위 분리 원칙

```text
차량 = 어디에 달 수 있는가
장착 프로파일 = 그 차량 위치가 어떤 타입 / 크기 / 구조 제한을 허용하는가
터렛 마운트 = 무기를 얹고 회전시키는 거치대 / 조준 기계장치
무기 = 거치대 위에 얹는 실제 발사 장치
탄환 = 발사된 뒤 무엇이 날아가고 어떤 피해를 주는가
런타임 상태 = 지금 쿨다운인지, 탄이 남았는지, 터렛이 따라왔는지
UI = 위 상태를 읽어서 보여주되, 유저에게는 장비 프리셋 단위로 단순화함
```

핵심 규칙:

| 계층 | 해야 할 일 | 하지 말아야 할 일 |
|---|---|---|
| DataAsset | 값을 저장 | 런타임 판단 실행 |
| C++ Component | 값을 해석하고 상태를 갱신 | 에디터 배치값을 하드코딩 |
| Blueprint | 시각 표현과 에셋 연결 | 발사 가능 여부 최종 판단 |
| UI / Debug | 계산된 상태 표시 | 규칙 재판정 |

---

## 4. 차량에 종속되는 데이터

차량 DataAsset은 차체와 차종의 기준값을 소유한다.

P0 기준 저장 위치:

```text
UCFVehicleData
```

차량에 남길 데이터:

| 데이터 | 설명 |
|---|---|
| `HardpointSlots` | 차량별 실제 장착 위치 목록 |
| `LocationSlotId` | `Front_01`, `Top_01` 같은 실제 장착점 |
| `LocationCategory` | `Front`, `Top` 같은 위치 분류 |
| `LocalLocation` / `LocalRotation` | 차체 기준 발사 / 장착 위치 |
| `SocketName` | 선택 캡처 입력. 런타임 필수 아님 |
| 차체 HP | 차량 파괴 기준 |
| 방향별 장갑 계수 | 정면 / 측면 / 후면 피해 차이 |
| 모듈 후보 | 바퀴 / 엔진 / 터렛 같은 손상 후보 |
| 기본 중량 | 총중량 계산 시작값 |
| 장착 가능 프로파일 목록 | 이 차량이 어떤 장착 규칙을 허용하는지 |

차량에 넣지 않을 데이터:

| 넣지 않을 데이터 | 이유 |
|---|---|
| 현재 탄약 수 | 런타임 상태이므로 WeaponComp 또는 CombatComp 소유 |
| 현재 터렛 회전값 | 매 프레임 변하는 상태이므로 WeaponComp 소유 |
| 탄속 / 탄 데미지 | 탄환 또는 무기 데이터 소유 |
| 무기 쿨다운 | 무기 또는 무기 런타임 상태 소유 |
| 발사 이펙트 | 무기 / 표현 계층 소유 |
| Damage 적용 결과 | Damage 런타임 처리 소유 |

---

## 5. 장착 프로파일 데이터

장착 프로파일은 차량 위치 슬롯과 전투 장비 사이의 연결 규칙이다.

P0 기본 저장 위치:

```text
UCFVehicleData 안에 작은 배열로 추가
```

나중에 분리할 조건:

```text
무기 종류가 늘고,
차량별 피팅 조합이 많아지고,
차량 DataAsset이 전투 장비 표로 비대해지면
별도 Combat 또는 Loadout DataAsset으로 분리한다.
```

P0 장착 프로파일 필드 구분:

| 구분 | 필드 | 상태 | 의미 |
|---|---|---|---|
| 슬롯 규칙 | `MountProfileId` | 유지 | 장착 프로파일 식별자. 예: `RoofTurret_MediumOrLarge` |
| 슬롯 규칙 | `LocationSlotRef` | 유지 | 참조할 차량 위치 슬롯. 예: `Top_01` |
| 슬롯 규칙 | `MountType` | 유지 | 고정 / 짐벌 / 터렛 / 런처 / 유틸리티 구분 |
| 슬롯 규칙 | `SizeLimit` | 유지 | 장착 가능한 최대 무기 크기 |
| 슬롯 규칙 | `bExposedModule` | 유지 | 외부 노출 / 손상 후보 여부 |
| 장비 기본값 | `DefaultEquipmentPresetData` | 코드 반영 / 재빌드 필요 | 이 슬롯에 기본 장착할 완성 장비 패키지 |
| Legacy 직접 연결 | `DefaultTurretMountData` | 삭제 완료 | 사용자가 legacy 슬롯을 비우고 PIE 정상작동을 확인했으므로 MountProfile에서 제거 |
| Legacy 직접 연결 | `DefaultWeaponData` | 삭제 완료 | 사용자가 legacy 슬롯을 비우고 PIE 정상작동을 확인했으므로 MountProfile에서 제거 |

MountProfile에 더 이상 늘리지 않을 데이터:

| 데이터 | 새 소유 위치 | 이유 |
|---|---|---|
| Yaw / Pitch 시각 메쉬 | `UCFTurretMountData` | 터렛 마운트의 생김새는 차량이 아니라 거치대 고유값 |
| Pitch 피벗 소켓 / Muzzle 소켓 | `UCFTurretMountData` | 회전축과 총구 위치는 거치대 구조에 종속 |
| Yaw / Pitch 회전 속도 | `UCFTurretMountData` | 얼마나 빨리 도는지는 거치대 기계 성능 |
| 차량 슬롯 기준 Min/Max Yaw/Pitch | 삭제 완료. 필요 시 별도 `HardpointArcConstraint` 후보 | 장착 규칙과 터렛 기계 한계를 섞지 않기 위해 MountProfile에서 제거 |
| 안정화 허용각 / 안정화 시간 | `UCFTurretMountData` | 조준 장치 성능 |
| 터렛 마운트 중량 | `UCFTurretMountData` | 거치대 자체 무게 |
| 발사속도 / 사거리 / Projectile | `UCFWeaponData` | 실제 발사 장치 성능 |
| 완성 장비 조합 | `UCFEquipmentPresetData` | `HeavyCannonMount + SmallWeapon` 같은 비의도 조합을 차량 DA에서 직접 만들지 않기 위함 |

마이그레이션 원칙:

```text
DefaultEquipmentPresetData가 있으면 장비 조합 원본으로 우선 사용한다.
DefaultEquipmentPresetData가 비어 있거나 내부 TurretMountData / WeaponData 참조가 비어 있으면 해당 장비 데이터는 Missing 상태로 표시한다.
MountProfile.DefaultTurretMountData / DefaultWeaponData 직접 연결은 삭제했으며 fallback으로 사용하지 않는다.
TurretMountData가 비어 있으면 MountProfile inline 터렛 시각 / 회전 fallback은 더 이상 런타임에서 사용하지 않는다.
MountProfile의 Min/Max Yaw/Pitch는 DataAsset 재저장 확인 뒤 C++ 필드에서 삭제했다.
남은 legacy inline 시각 / 기계 필드는 기존 DataAsset 직렬화 호환을 위해 잠시 보존하되 에디터 편집 노출은 중단한다.
```

P0 필수 프로파일:

| MountProfileId | LocationSlotRef | MountType | 목적 |
|---|---|---|---|
| `RoofTurret_MediumOrLarge` | `Top_01` | `Turret` | 루프 대구경 저속 터렛 검증 |

P0 이후 후보:

| MountProfileId | LocationSlotRef | MountType | 목적 |
|---|---|---|---|
| `FrontFixed_SmallOrMedium` | `Front_01` | `Fixed` | 차체 방향과 사격 방향 연결 |
| `FrontGimbal_Small` | `Front_02` | `Gimbal` | 전방 제한 짐벌 검증 |
| `RoofOrRearLauncher_Small` | `Top_02` 또는 `Back_01` | `Launcher` | 락온 / 미사일 루프 |

---

## 6. 터렛 마운트에 종속되는 데이터

터렛 마운트는 무기가 아니라 "무기를 얹고 회전시키는 거치대 / 조준 기계장치"로 본다.
기존 대화에서 말한 `TurretData`는 앞으로 `TurretMountData`로 부른다.

용어 고정:

| 용어 | 의미 |
|---|---|
| `TurretMountData` | 무기 거치대, 회전 플랫폼, 조준 기계장치 |
| `WeaponData` | 거치대 위에 얹는 실제 발사 장치 |
| `ProjectileData` | 발사된 뒤 날아가는 탄 |
| `DamageData` | 맞았을 때 적용되는 피해 규칙 |

정적 튜닝값:

| 데이터 | 소유 위치 |
|---|---|
| 터렛 자체 회전 제한각 | `UCFTurretMountData` |
| 차량 구조상 회전 제한각 | P0 미사용. 필요하면 후속 별도 제약 데이터로 재도입 |
| 최종 유효 회전 제한각 | P0는 `UCFTurretMountData` 단독 |
| 회전 속도 | `UCFTurretMountData` |
| 안정화 기준 | `UCFTurretMountData` |
| Base / Yaw / Pitch 터렛 시각 메쉬 | `UCFTurretMountData` |
| YawPivot / PitchPivot / Muzzle 소켓 | `UCFTurretMountData` |

런타임 상태:

| 데이터 | 소유 위치 |
|---|---|
| `CurrentYawDeg` | `UCFVehicleWeaponComp` |
| `CurrentPitchDeg` | `UCFVehicleWeaponComp` |
| `TargetYawDeg` | `UCFVehicleWeaponComp` |
| `TargetPitchDeg` | `UCFVehicleWeaponComp` |
| `bTurretSettled` | `UCFVehicleWeaponComp` |
| `FireOrigin` | `UCFVehicleWeaponComp` 계산 결과 |
| 터렛 Visual 컴포넌트 참조 | P0는 Pawn 보조 컴포넌트, 회전 상태 계산은 `UCFVehicleWeaponComp` |

터렛 BP의 역할:

```text
BP_CFTurret_Proto는 시각 표현 전용이다.
실제 회전 계산, 발사 가능 여부, Damage, 탄약은 BP에서 판단하지 않는다.
```

P0 터렛 시각 장착 v0.2:

```text
SM_Body
└─ Turret_MountRoot = HardpointSlots[Top_01] LocalLocation / LocalRotation
   └─ Turret_BaseMesh = TurretMountData.TurretBaseMesh + TurretBaseRelativeTransform, 선택값 / 고정
      └─ Turret_YawPivot = YawPivotSocketName 소켓 또는 Turret_MountRoot fallback
         └─ Turret_YawMesh = TurretMountData.TurretYawMesh + TurretYawRelativeTransform
            └─ Turret_PitchPivot = PitchPivotSocketName 소켓 또는 YawPivot fallback
               └─ Turret_PitchMesh = TurretMountData.TurretPitchMesh + TurretPitchRelativeTransform
                  └─ MuzzleSocketName = 최종 FireOrigin 위치 / 방향 우선 기준, 소켓 X축이 발사 방향
```

현재 구현 경계:

| 항목 | 현재 결정 |
|---|---|
| 터렛 시각 저장 위치 | `UCFTurretMountData`만 런타임 소스로 사용한다. 기존 `FCFVehicleMountProfile` inline 필드는 직렬화 호환용 legacy 값 |
| 장착 기준 위치 | `LocationSlotRef`가 가리키는 `HardpointSlots`의 `LocalLocation` / `LocalRotation` |
| 차체 부모 컴포넌트 | 우선 `SM_Body`, 없으면 Pawn Root fallback |
| 하드포인트 소켓 우선순위 | `HardpointSlots.SocketName`이 있고 `SM_Body`에 해당 소켓이 있으면 소켓 Transform을 우선 사용. 없으면 `LocalLocation` / `LocalRotation` fallback |
| 하드포인트 위치 진단 | VehicleDebug Panel `무기 > 터렛 시각 > 요약`의 `RootDelta`가 0에 가까우면 터렛 루트는 하드포인트 소켓에 맞고, 보이는 메쉬 오차는 Base/Yaw/Pitch 메쉬 피벗 또는 상대 Transform 문제로 본다 |
| 필수 소켓 누락 진단 | 하드포인트 / YawPivot / PitchPivot / Muzzle 소켓 이름이 지정되어 있는데 실제 메쉬에 없으면 기존 fallback은 유지하되 `SocketValidation=MissingRequiredSocket`, `MissingRequiredSockets=...`로 표시 |
| Base 부착 기준 | Base 메쉬는 하드포인트 루트에 고정. 비어 있으면 표시를 생략 |
| Yaw 부착 기준 | Base 메쉬의 `YawPivotSocketName` 소켓이 있으면 소켓, 없으면 하드포인트 루트 또는 Base 메쉬 원점 fallback |
| Pitch 부착 기준 | Yaw 메쉬의 `PitchPivotSocketName` 소켓이 있으면 소켓, 없으면 YawPivot 또는 Yaw 메쉬 원점 fallback |
| Muzzle FireOrigin 기준 | Pitch 메쉬의 `MuzzleSocketName` 소켓이 있으면 `ACFVehiclePawn`이 최종 FireOrigin 위치와 방향을 해당 소켓 기준으로 보정. 방향은 소켓 X축을 우선 사용 |
| 메시 미지정 시 | 터렛 시각 표시만 생략. FireOrigin / Projectile / Cooldown은 유지 |
| 터렛 회전 계산 | `UCFVehicleWeaponComp::UpdateTurretState`가 조준 방향을 터렛 루트 로컬 Yaw/Pitch로 바꾸고, EquipmentPresetData 내부 TurretMountData의 회전 제한 / 회전 속도를 적용. TurretMountData가 없으면 터렛 조준 추적만 건너뛰고 FireOrigin / Projectile / Cooldown은 유지 |
| 터렛 회전 적용 | `ACFVehiclePawn::UpdateVehicleTurretAimVisuals`가 WeaponComp의 `FCFVehicleTurretState`를 읽어 `Turret_YawPivot`, `Turret_PitchPivot` 상대 회전에 적용 |
| 안정화 전 발사 정책 | 터렛이 목표각까지 돌아가는 중이어도 발사 입력은 처리한다. `bTurretSettled`는 Debug / 후속 정확도 판단용이며 P0 기본 발사 금지 조건이 아니다 |
| 조준각 초과 처리 | 조준 방향이 유효 회전 제한 밖이어도 발사는 막지 않는다. 대신 터렛 목표각 / 현재각 / Muzzle FireOrigin은 TurretMountData 제한각 안에 고정한다 |
| 탄퍼짐 / 명중률 | 현재 단계에서 적용하지 않는다. 후속 사격 시스템에서 WeaponData / ProjectileData / DamageData와 별도 런타임 규칙으로 설계한다 |
| 이번 단계 제외 | Damage 적용 |

`UCFTurretMountData` 최소 설계 v0.2:

| 필드 | 설명 |
|---|---|
| `TurretMountId` | 터렛 마운트 식별자 |
| `TurretBaseMesh` | 하드포인트에 고정되는 받침 메쉬. 선택값 |
| `TurretBaseRelativeTransform` | 하드포인트 루트 기준 Base 메쉬 보정 |
| `YawPivotSocketName` | Base 메쉬에서 Yaw 회전부가 얹히는 소켓 |
| `TurretYawMesh` | YawPivot 아래에서 좌우 회전할 상부 / 회전부 메쉬 |
| `TurretYawRelativeTransform` | YawPivot 기준 Yaw 메쉬 보정 |
| `PitchPivotSocketName` | Yaw 메쉬에서 Pitch 회전부가 얹히는 소켓 |
| `TurretPitchMesh` | PitchPivot 아래에서 Yaw를 따라 좌우 회전하고 Pitch로 상하 회전할 포신 / 발사부 메쉬 |
| `TurretPitchRelativeTransform` | PitchPivot 기준 Pitch 메쉬 보정 |
| `MuzzleSocketName` | Pitch 메쉬에서 최종 FireOrigin 위치와 방향을 찾을 총구 소켓. 소켓 X축이 발사 방향 |
| `MinYawDeg` / `MaxYawDeg` | 터렛 마운트 자체 좌우 기계 한계 |
| `MinPitchDeg` / `MaxPitchDeg` | 터렛 마운트 자체 상하 기계 한계 |
| `YawTurnRateDegPerSec` | 터렛 마운트 좌우 회전 속도 |
| `PitchTurnRateDegPerSec` | 터렛 마운트 상하 회전 속도 |
| `StabilizationToleranceDeg` | 안정화로 볼 각도 오차 |
| `AimSettleTimeSeconds` | 안정화 유지 시간 |
| `TurretMountWeightKg` | 거치대 자체 중량 |

최종 회전 제한 계산 방향:

```text
유효 Yaw 최소 = TurretMountData.MinYawDeg
유효 Yaw 최대 = TurretMountData.MaxYawDeg
유효 Pitch 최소 = TurretMountData.MinPitchDeg
유효 Pitch 최대 = TurretMountData.MaxPitchDeg
```

MountProfile.Min/Max Yaw/Pitch는 legacy 직렬화 호환값이며 위 계산에 참여하지 않는다.
현재 코드는 위 계산을 `UCFVehicleWeaponComp` 런타임 상태에 연결했다.
`CurrentYawDeg`, `CurrentPitchDeg`, `TargetYawDeg`, `TargetPitchDeg`, `bTurretSettled`는 WeaponComp가 소유한다.
Pawn은 계산 결과를 시각 피벗 컴포넌트에 적용만 한다.
Fire 입력은 필수 거부 조건이 없으면 처리하고, 조준각 초과 또는 안정화 미완료만으로 `OutOfWeaponArc` 거부를 만들지 않는다.

장비 패키지 결정:

```text
내부 구현은 MountProfile / TurretMountData / WeaponData / ProjectileData / DamageData를 분리한다.
유저와 에셋 제작자는 EquipmentPresetData를 완성 장비 단위로 다룬다.
MountType은 유저 선택지가 아니라 내부 호환성 판정 태그다.
WeaponData가 TurretMountData를 직접 소유하지 않는다.
EquipmentPresetData가 TurretMountData와 WeaponData의 유효 조합을 소유한다.
```

`WeaponData`가 `TurretMountData`를 직접 들지 않는 이유:

| 이유 | 설명 |
|---|---|
| 무기 스탯 중복 방지 | 같은 자동포를 고정형 / 짐벌형 / 터렛형으로 쓸 때 발사속도, 사거리, 탄창, 열 값을 복제하지 않기 위함 |
| 거치대 성능 분리 | 회전 제한, 회전 속도, 피벗, Muzzle 소켓, 중량은 거치대 구조의 값이다 |
| 조합 검증 위치 명확화 | `HeavyCannonMount + SmallWeapon` 같은 비의도 조합은 EquipmentPresetData 또는 장착 검증에서 막는다 |
| 유저 노출 단순화 | 유저에게는 `대구경 저속 회전포탑` 하나로 보이고, 내부만 Mount / Weapon / Projectile / Damage로 분리한다 |

`UCFEquipmentPresetData` 최소 코드 설계 v1.0:

| 타입 | 역할 |
|---|---|
| `EquipmentId` | 장비 패키지 식별자. 예: `EQP_HeavyCannonTurret` |
| `DisplayName` | 유저에게 보여줄 장비 이름. 예: `대구경 저속 회전포탑` |
| `RequiredMountType` | 내부 호환성 필터. 예: `Turret` |
| `RequiredWeaponSize` | 필요한 장착 크기. 예: `Large` |
| `DefaultTurretMountData` | 이 장비가 사용하는 거치대 / 회전 플랫폼 |
| `DefaultWeaponData` | 이 장비가 사용하는 실제 발사 장치 |

P0에서는 EquipmentPresetData가 ProjectileData를 직접 소유하지 않는다.
기본 탄 / 발사체는 기존처럼 `WeaponData.DefaultProjectileData`를 우선 사용한다.
탄종 선택, 탄수, 장전, 교체 탄종 목록은 후속 `AmmoData` 또는 `WeaponRuntime` 단계에서 분리한다.

에디터 연결 기준:

1. 현재 코드에서는 `MountProfiles > RoofTurret_MediumOrLarge`가 `기본 장비 프리셋 데이터 (DefaultEquipmentPresetData)`를 우선 해석한다.
2. `DefaultEquipmentPresetData` 내부의 `DefaultTurretMountData`와 `DefaultWeaponData`가 실제 터렛 시각 장착과 WeaponData 해석의 단일 소스다.
3. `DefaultEquipmentPresetData`가 비어 있거나 내부 참조가 비어 있으면 기존 직접 연결 fallback 없이 해당 장비 데이터는 Missing 상태가 된다.
4. 터렛 마운트 DataAsset에서 `터렛 Base 메쉬 (TurretBaseMesh)` / `Yaw 피벗 소켓 이름 (YawPivotSocketName)` / `터렛 Yaw 메쉬 (TurretYawMesh)` / `Pitch 피벗 소켓 이름 (PitchPivotSocketName)` / `터렛 Pitch 메쉬 (TurretPitchMesh)` / `Muzzle 소켓 이름 (MuzzleSocketName)`을 지정한다.
5. `DefaultEquipmentPresetData` 내부 `DefaultTurretMountData`가 비어 있으면 legacy inline 터렛 필드는 사용하지 않고 터렛 시각 장착을 생략한다. VehicleDebug Panel 요약은 `Source=MissingEquipmentPresetTurretMountData` 또는 `TurretMountData=Missing` 계열로 확인한다.
6. PIE에서 VehicleDebug Panel `무기 > 장비 프리셋`, `무기 > 터렛 시각`, `무기 > 무기 데이터`, `무기 > 터렛 조준`이 같은 EquipmentPreset에서 해석됐는지 확인한다.
7. 빠른 연속 발사 시 기존 남은 쿨다운 감소와 `WeaponCooldown` 거부 사유는 유지되어야 한다.

---

## 7. 무기에 종속되는 데이터

무기는 "언제 어떤 방식으로 발사하는가"를 소유한다.

P0 v0.4 기준으로 최소 `UCFWeaponData` 타입은 구현됐다.
단, 이 단계의 WeaponData는 런타임 발사 판정을 완성하지 않고 장착 데이터 연결, 호환성 검증, 디버그 표시까지만 담당한다.

`UCFWeaponData` 현재 필드:

| 필드 | 설명 |
|---|---|
| `WeaponId` | 무기 식별자 |
| `WeaponSize` | Small / Medium / Large |
| `CompatibleMountTypes` | Fixed / Gimbal / Turret 등 |
| `FireMode` | HitScan / Projectile |
| `FireRatePerMinute` | 분당 발사속도. 실제 발사 간격은 `60 / FireRatePerMinute`로 환산 |
| `MaxRange` | Trace 또는 Projectile 기준 최대 사거리 |
| `ReloadTimeSeconds` | 재장전 시간 |
| `MagazineSize` | 탄창 크기 |
| `AmmoTypeId` | 사용할 탄종 |
| `BaseDamage` | 레거시 확인용 피해량. 실제 DamageData 해석에는 사용하지 않음 |
| `DefaultProjectileData` | 이 무기가 기본으로 사용할 발사체 DataAsset |
| `ProjectileDataId` | ProjectileData 분리 전 식별자 |
| `DamageProfileId` | 레거시 확인용 피해 프로파일 ID. 실제 DamageData 해석에는 사용하지 않음 |
| `SpreadDeg` | 탄 퍼짐 |
| `HeatPerShot` | 발사 열 |
| `MaxHeat` | 과열 한계 |

ProjectileData 참조 결정:

```text
현재 코드에서는 WeaponData가 기본 ProjectileData를 직접 참조한다.
기존 ProjectileDataId는 마이그레이션 / 로그 / 임시 표시 용도이며, 실제 데이터 해석 원본은 되지 않는다.
```

현재 필드:

| 필드 | 소유 위치 | 이유 |
|---|---|---|
| `DefaultProjectileData` | `UCFWeaponData` | 이 무기가 기본으로 발사할 탄환 DataAsset 직접 참조 |
| `ProjectileDataId` | `UCFWeaponData` 임시 / 마이그레이션 | 기존 필드 보존, 에셋 연결 전 로그 확인용 |
| DamageData 직접 참조 | 없음 | DamageData 참조 슬롯은 `UCFProjectileData.DefaultDamageData` 하나만 허용 |

P0에서 기본으로 만들 무기 성격:

```text
대구경 저속 터렛포
MountType=Turret, WeaponSize=Large 호환
FireRatePerMinute / MaxRange는 Debug와 P0 Fire 검증에 연결된 기준값
BaseDamage / DamageProfileId는 기존 에셋 확인용 레거시 값
탄약 / 열 / 재장전 런타임은 P0 1차 비범위
```

---

## 8. 탄환 / Projectile에 종속되는 데이터

탄환은 무기에서 분리한다.

이유:

```text
같은 무기라도 탄종이 바뀌면 피해량, 속도, 관통력, 폭발 여부가 달라질 수 있다.
```

P0 현재 상태에서는 최소 `UCFProjectileData` 타입, Debug 표시, `ProjectileActorClass` 기반 스폰 준비 판정, 공통 `ACFProjectileActor`, `UCFProjectilePoolComp` 기반 Pool Acquire 경로, VehicleDebug Panel Pool 카운트 표시, 마지막 Pool 반환 요약 표시를 구현했다.
실제 Projectile Actor 확보 / 활성화는 `WeaponData.FireMode=Projectile`, `DefaultProjectileData` 유효, `ProjectileActorClass` 유효 조건을 모두 만족할 때만 실행한다.
조건이 맞지 않거나 Pool 확보 / 활성화에 실패하면 기존 Dummy HitScan fallback을 유지한다.
2026-07-01 PIE 기준으로 사용자가 FireOrigin에서 Projectile Actor가 생성되어 날아가는 것을 확인 완료했다.
2026-07-01 PIE 기준으로 사용자가 Projectile Pool 발사 경로의 정상 동작을 확인 완료했다.
2026-07-01 코드 기준으로 `Tools\BuildEditor.bat`는 Projectile Pool 마지막 반환 요약 추가 후 통과했다.
2026-07-01 PIE 기준으로 사용자가 VehicleDebug Panel `무기 > 발사체 Pool > 마지막 반환 요약` 표시를 확인 완료했다.

`UCFProjectileData` 최소 설계 v0.1:

| 필드 | 설명 |
|---|---|
| `ProjectileId` | 탄환 / 발사체 식별자 |
| `InitialSpeed` | 발사체 초기 속도 |
| `bAffectedByGravity` | 중력 영향 여부 |
| `GravityScale` | 중력 보정값. `bAffectedByGravity=false`이면 무시 |
| `LifeTimeSeconds` | 발사체 수명. Projectile 전환 전에는 최대 비행 시간 기준값 |
| `CollisionRadius` | 발사체 충돌 판정 반경 |
| `ProjectileActorClass` | 실제 Projectile Actor 클래스. `ACFProjectileActor` 기반 Blueprint를 지정 |
| `ProjectileStaticMesh` | 공통 Projectile Actor가 표시할 발사체 StaticMesh |
| `ProjectileMeshRelativeRotation` | 메시 앞 방향 보정용 상대 회전 |
| `ProjectileMeshRelativeScale` | 메시 표시 크기. 충돌 판정은 `CollisionRadius`가 소유 |
| `ImpactEffectId` | 피격 표현 후보 ID. 실제 VFX / SFX는 후속 표현 계층에서 연결 |
| `DefaultDamageData` | 이 발사체 또는 가상 HitScan / Laser 발사 데이터가 직접 사용할 DamageData. DamageData 직접 참조의 단일 슬롯 |
| `DamageProfileId` | DamageData 미연결 상태를 Debug에 남기기 위한 fallback 피해 프로파일 ID |

HitScan / Laser 가상 ProjectileData 기준:

| 필드 | 설명 |
|---|---|
| `ProjectileActorClass` | 비워 둔다. 실제 Actor를 스폰하지 않는다는 뜻 |
| `ProjectileId` | `Proto_HitScanInstant`, `Proto_LaserBeam`처럼 발사 결과를 구분할 ID |
| `DefaultDamageData` | HitScan / Laser가 사용할 DamageData 직접 참조 |
| `DamageProfileId` | DamageData 미연결 상태의 Debug fallback ID |
| `ImpactEffectId` | HitScan / Laser 피격 표현 후보 |

Dummy HitScan fallback의 의미:

```text
Dummy HitScan fallback은 데이터 미연결 / 스폰 미준비 상태에서도 FireOrigin, FireRatePerMinute, 입력 검증을 유지하기 위한 안전 경로다.
피해 데이터를 실제로 해석해야 하는 무기는 HitScan / Laser라도 WeaponData.DefaultProjectileData에 가상 ProjectileData를 연결한다.
```

Dummy HitScan 유지 조건:

| 조건 | 이유 |
|---|---|
| `WeaponData.FireMode`가 `HitScan`이다 | 현재 검증된 P0 발사 흐름을 유지 |
| `DefaultProjectileData`가 비어 있다 | 데이터 미연결 상태에서도 FireOrigin / Cooldown 검증을 계속하기 위함 |
| `ProjectileActorClass`가 비어 있다 | 실제 발사체 스폰 경로가 아직 없으면 Dummy Trace로 fallback |
| Projectile Actor 스폰이 실패한다 | 잘못된 클래스 / 월드 없음 / 방향 오류 시 발사 입력이 먹통처럼 보이지 않게 하기 위함 |
| 목표가 사거리 / 쿨다운 / Debug 표시 검증이다 | Projectile 물리보다 데이터 연결 안정성이 우선 |

Projectile 스폰 게이트 상태:

| 상태 | 표시 | 실행 경로 |
|---|---|---|
| `DefaultProjectileData` 없음 | `지정 여부=미지정`, `Fallback=Dummy HitScan 유지` | 기존 Dummy HitScan |
| `DefaultProjectileData` 있음, `ProjectileActorClass` 없음 | `지정 여부=지정`, `스폰 준비=미준비`, `Fallback=Dummy HitScan 유지` | 기존 Dummy HitScan |
| `DefaultProjectileData` 있음, `ProjectileActorClass` 있음, `FireMode=HitScan` | `스폰 준비=미준비`, `Fallback=Dummy HitScan 유지` | 기존 Dummy HitScan |
| `DefaultProjectileData` 있음, `ProjectileActorClass` 있음, `FireMode=Projectile` | `스폰 준비=준비됨`, `Fallback=Projectile Pool 준비됨`, `전환 요약=ReadyToPoolAcquire` | Projectile Pool Acquire |

Projectile Pool Debug 표시:

| 표시 위치 | 필드 | 의미 |
|---|---|---|
| `무기 > 발사체 Pool` | `Pool 컴포넌트` | 현재 Pawn이 `UCFProjectilePoolComp`를 보유하는지 표시 |
| `무기 > 발사체 Pool` | `전체 Pool 수` | Pool이 추적 중인 전체 유효 Projectile Actor 수 |
| `무기 > 발사체 Pool` | `활성 Pool 수` | 현재 발사되어 이동 중인 Projectile Actor 수 |
| `무기 > 발사체 Pool` | `비활성 Pool 수` | 수명 종료 또는 충돌 후 재사용 대기 중인 Projectile Actor 수 |
| `무기 > 발사체 Pool` | `마지막 반환 요약` | 마지막으로 Pool에 반환된 Projectile의 ID, 반환 사유, 충돌 대상, 비행 시간 |

PIE 해석 기준:

| 상황 | 기대 표시 |
|---|---|
| 첫 Projectile 발사 직후 | `전체 Pool 수 >= 1`, 비행 중이면 `활성 Pool 수 > 0` |
| 수명 종료 또는 충돌 후 | `활성 Pool 수` 감소, `비활성 Pool 수` 증가 |
| 비활성 Actor가 있는 상태에서 재발사 | `전체 Pool 수`가 불필요하게 계속 증가하지 않고 기존 Actor 재사용 |
| `비활성 Pool 수 > 0`인데 매 발사마다 `전체 Pool 수`가 증가 | Pool 재사용 의심 상태. Acquire / Release 경로 재점검 필요 |
| 수명 종료로 반환 | `마지막 반환 요약`에 `Reason=LifeExpired`, `Flight`가 `LifeTimeSeconds` 근처로 표시 |
| 충돌로 반환 | `마지막 반환 요약`에 `Reason=Hit`, `HitActor`가 충돌 대상 이름으로 표시 |

Projectile 전환 조건:

| 조건 | 완료 기준 |
|---|---|
| `UCFProjectileData` DataAsset 타입 구현 | 완료. 에디터에서 ProjectileData 에셋 생성 가능 |
| `WeaponData.DefaultProjectileData` 연결 | 완료. VehicleDebug Panel에서 지정 여부와 `ProjectileId` 확인 |
| `ProjectileActorClass` 스폰 준비 게이트 구현 | 완료. `FireMode=Projectile`까지 맞으면 준비 상태 표시 |
| 실제 Projectile Actor 스폰 경로 구현 | 완료. FireOrigin 위치 / 방향에서 공통 Projectile Actor 생성, PIE 비행 확인 완료 |
| Projectile Pool Acquire 경로 구현 | 완료. `UCFProjectilePoolComp`가 클래스별 Pool 버킷을 관리하고, 한도 초과 / 확보 실패 시 Dummy HitScan fallback |
| Projectile Pool Debug 표시 구현 | 완료. VehicleDebug Panel `무기 > 발사체 Pool`에서 컴포넌트 보유 여부, 전체 / 활성 / 비활성 수, 마지막 반환 요약 표시 |
| Projectile Pool PIE 검증 | 완료. 사용자가 발사체 확인과 정상 동작 확인을 완료 |
| Projectile Pool 마지막 반환 요약 PIE 검증 | 완료. 사용자가 VehicleDebug Panel에서 마지막 반환 요약 표시를 확인 완료 |
| 충돌 / 수명 / 중력 적용 검증 | 코드 구현 완료. DamageData 전환 전 `LifeTimeSeconds`, `bAffectedByGravity`, `GravityScale`, `Reason=LifeExpired`, `Reason=Hit`, `HitActor`를 회귀 체크 기준으로 유지 |
| Dummy HitScan fallback 유지 | ProjectileData 누락 시 기존 FireOrigin / Cooldown 검증이 깨지지 않음 |

---

## 9. 피해 / 장갑 / 모듈 데이터

Damage Runtime은 차량 데이터와 ProjectileData / DamageData를 함께 읽는다.
단, 수정이 쉬운 구조를 위해 "피해 규칙", "비행 / 충돌", "피격 대상 상태", "명중 이벤트"를 분리한다.

P0 설계 결론:

```text
DamageData = 맞았을 때 어떤 피해 규칙을 적용할지 정의
ProjectileData = 무엇이 어떻게 날아가고 충돌하는지 정의
VehicleData = 어떤 대상이 얼마나 버티고, 어느 방향 장갑을 갖는지 정의
HitContext = 이번 명중에서 실제로 무엇을 맞췄는지 기록
```

수정 용이성 원칙:

| 원칙 | 이유 |
|---|---|
| DamageData 직접 참조는 ProjectileData에만 둔다 | WeaponData와 ProjectileData 양쪽에 피해 슬롯이 생기는 혼선을 막기 위함 |
| ProjectileData.DamageProfileId fallback은 Debug 전용으로 둔다 | DataAsset 연결 안정화 전에도 미연결 상태와 의도한 피해 프로파일을 추적하기 위함 |
| DamageData는 계산식을 최소로 갖고 값만 소유한다 | 관통식 / 폭발식 / 모듈식 변경 시 런타임 계산 계층만 바꾸기 위함 |
| HitContext는 런타임 구조체로 분리한다 | Projectile / HitScan 양쪽에서 같은 Debug와 후속 Damage 적용 경로를 쓰기 위함 |
| P0에서는 HP 차감을 하지 않는다 | 데이터 연결과 명중 이벤트 검증을 먼저 닫기 위함 |

차량 쪽 데이터:

| 데이터 | 설명 |
|---|---|
| 차체 HP | 차량 생존 기준 |
| 방향별 장갑 계수 | 정면 / 측면 / 후면 피해 차이 |
| 모듈 목록 | 바퀴 / 엔진 / 터렛 등 |
| 하드포인트 노출 여부 | 외부 장비 피격 가능성 |

ProjectileData가 소유할 값:

| 데이터 | 설명 |
|---|---|
| Projectile 이동 방식 | 속도, 중력, 수명 |
| Projectile 충돌 판정 | 충돌 반경, Actor Class |
| 피격 표현 후보 | ImpactEffectId, Decal / Sound 후보 |
| DamageData 참조 | `DefaultDamageData` 단일 직접 참조와 `DamageProfileId` Debug fallback |

`UCFDamageData` 최소 설계 v0.1:

| 필드 | 설명 |
|---|---|
| `DamageId` | 피해 프로파일 식별자. 예: `ProtoDirectHit` |
| `DamageType` | 피해 종류. P0 후보는 `Kinetic`, `Explosive`, `Energy` |
| `BaseDamage` | 기본 직접 피해량 |
| `ArmorPenetration` | 장갑 관통 기준값 |
| `bUseRadialDamage` | 폭발 / 범위 피해 사용 여부 |
| `ExplosionRadius` | 폭발 피해 반경 |
| `ExplosionInnerRadius` | 최대 폭발 피해가 유지되는 내부 반경 |
| `ExplosionDamage` | 폭발 중심 피해량 |
| `MinExplosionDamageScale` | 폭발 반경 가장자리 최소 피해 비율 |
| `ModuleDamageScale` | 모듈 피해 배율. P0에서는 기록만 하고 실제 모듈 손상은 후순위 |
| `ImpulseStrength` | 피격 물리 반응 후보값. 실제 물리 적용은 후순위 |

DamageData 참조 우선순위:

```text
1. ProjectileData.DefaultDamageData
2. ProjectileData.DamageProfileId fallback = Debug 표시 전용
3. ProjectileData 없음 = DamageData 없음, Debug에 가상 ProjectileData 필요 표시
4. DamageData 없음 = Damage 적용 안 함, Debug에 Missing 표시
```

이 우선순위를 쓰는 이유:

| 경로 | 이유 |
|---|---|
| ProjectileData 우선 | 같은 무기라도 탄종이 바뀌면 피해 / 관통 / 폭발 규칙이 달라질 수 있음 |
| HitScan / Laser 가상 ProjectileData | 실제 Actor가 없어도 발사 결과 / 피해 / 피격 표현 데이터를 같은 경로로 해석하기 위함 |
| ProjectileData.DamageProfileId fallback | DamageData 미연결 상태를 Debug에서 추적하기 위한 임시 표시값 |
| Missing 허용 | DamageData가 없어도 FireOrigin / Projectile / Cooldown 검증을 유지하기 위함 |

`FCFDamageHitContext` 최소 설계 v0.1:

| 필드 | 설명 |
|---|---|
| `DamageData` | 이번 명중에서 적용 후보로 해석된 DamageData |
| `DamageId` | DamageData가 없을 때도 로그에 남길 피해 프로파일 ID |
| `WeaponId` | 발사한 무기 ID |
| `ProjectileId` | 발사체 ID. Dummy HitScan이면 `WeaponData.ProjectileDataId` 또는 None |
| `HitActor` | 맞은 Actor |
| `ImpactLocation` | 피격 월드 위치 |
| `ImpactNormal` | 피격 표면 노멀 |
| `IncomingDirection` | 탄이 들어온 월드 방향 |
| `InstigatorActor` | 발사 주체 |
| `FlightDurationSeconds` | Projectile Actor 비행 시간. HitScan이면 0 |
| `bFromProjectileActor` | Projectile Actor 충돌인지 Dummy HitScan 결과인지 구분 |

HitContext가 소유하지 않는 값:

| 값 | 소유 위치 |
|---|---|
| 최종 차체 HP | 후속 Damage Runtime |
| 최종 모듈 손상 단계 | 후속 Module Runtime |
| 장갑 방향 계수 원본 | VehicleData 또는 후속 ArmorData |
| 폭발 범위 내 전체 대상 목록 | 후속 Damage Runtime 계산 결과 |

런타임 Damage 상태:

| 데이터 | 소유 위치 |
|---|---|
| 현재 차체 HP | Damage 또는 Combat 런타임 계층 |
| 현재 모듈 손상 단계 | Damage 또는 Module 런타임 계층 |
| 마지막 피격 방향 | Damage 런타임 상태 |
| 마지막 피해 원인 | Damage 런타임 상태 |

P0에서는 Damage를 바로 완성하지 않는다.

P0 최소:

```text
UCFDamageData DataAsset 타입을 만든다.
ProjectileData.DefaultDamageData에서만 DamageData 직접 참조를 갖는다.
VehicleDebug Panel에서 DamageData 지정 여부, DamageId, 해석 경로, 요약을 볼 수 있게 한다.
Projectile 충돌과 Dummy HitScan 결과를 같은 HitContext 요약으로 기록한다.
실제 HP 차감과 모듈 손상은 DamageData 연결 / 명중 이벤트 검증 이후로 미룬다.
```

P0 비범위:

| 비범위 | 이유 |
|---|---|
| 실제 HP 차감 | 데이터 연결과 HitContext 검증이 먼저 |
| 모듈 손상 | DamageData 이후 ModuleData / ModuleRuntime 분리 필요 |
| 장갑 방향 판정 완성 | VehicleData / ArmorData 경계 확정 이후 |
| 폭발 범위 검색 | RadialDamage 값 연결 후 별도 검증 필요 |
| 탄퍼짐 / 명중률 | 후속 사격 시스템 범위 |
| 물리 충격 적용 | ImpulseStrength 값만 먼저 보존 |

---

## 10. 런타임 상태 데이터

런타임 상태는 DataAsset에 저장하지 않는다.

런타임 상태 후보:

| 상태 | 소유 위치 |
|---|---|
| 활성 장착 프로파일 | `UCFVehicleWeaponComp` |
| 활성 무기 | `UCFVehicleWeaponComp`, P0 이후 Weapon Runtime |
| 현재 터렛 Yaw / Pitch | `UCFVehicleWeaponComp` |
| 쿨다운 남은 시간 | P0 로컬 검증은 `UCFVehicleWeaponComp`, 완성형은 P0 이후 Weapon Runtime |
| 현재 탄약 수 | P0 이후 Weapon Runtime |
| 현재 열 | P0 이후 Weapon Runtime |
| 마지막 발사 시간 | Weapon Runtime |
| 마지막 발사 거부 사유 | Aim / Weapon 상태 |
| 현재 Fire 가능 여부 | C++ 검증 결과 |
| 실제 발사 위치 / 방향 | `FCFVehicleFireOrigin` |
| 조준각 내부 여부 | Aim / Debug 상태. 단독 발사 금지 조건 아님 |
| 터렛 안정화 여부 | WeaponComp Debug 상태. P0 기본 발사 금지 조건 아님 |
| 마지막 Damage Hit Context | P0는 Debug 상태. 실제 HP 차감은 후속 Damage Runtime |

이미 있는 상태 타입:

| 타입 | 역할 |
|---|---|
| `FCFVehicleFireRequest` | 발사 입력 데이터 |
| `FCFVehicleFireResult` | 발사 검증 결과 |
| `ECFVehicleFireRejectReason` | 발사 거부 사유 |
| `ECFVehicleReticleState` | Reticle 표시 상태 |
| `FCFDamageHitContext` | 구현 완료. Projectile / HitScan 명중 Debug 공통 구조 |

---

## 11. UI / Debug 데이터

UI와 Debug는 상태를 계산하지 않고 표시한다.

UI / Reticle이 읽을 값:

| 표시 값 | 원본 |
|---|---|
| 발사 가능 여부 | Fire 검증 결과 |
| 발사 불가 사유 | `ECFVehicleFireRejectReason` |
| 플레이어 조준점 | AimComp |
| 무기 실제 조준점 | WeaponComp |
| 터렛 안정화 여부 | WeaponComp |
| DamageData 지정 여부 | WeaponComp 또는 후속 Damage 해석 계층 |
| 마지막 명중 요약 | 후속 Damage Hit Context Debug |
| 탄약 / 열 | P0 이후 Weapon Runtime |
| 손상 상태 | P0 이후 Damage Runtime |

Debug 우선 표시값:

| 값 | 이유 |
|---|---|
| ActiveMountProfileId | 어떤 장착 규칙을 쓰는지 확인 |
| LocationSlotId | 어느 위치에서 쏘는지 확인 |
| MountType | Fixed / Turret 등 동작 확인 |
| FireOrigin | 차량 중심이 아닌지 확인 |
| FireDirection | 터렛 실제 방향인지 확인 |
| bTurretSettled | 터렛이 목표각을 따라잡았는지 확인. P0 기본 발사 차단 사유 아님 |
| FireRejectReason | UI 불신 방지 |
| DamageId | 어떤 피해 규칙 후보를 해석했는지 확인 |
| LastDamageHitContext | Projectile / Dummy HitScan 명중 이벤트가 같은 형식으로 기록되는지 확인 |

---

## 12. P0 구현 범위

P0 1차에서 할 것:

1. `FCFVehicleMountProfile` 구조를 정의한다.
2. `UCFVehicleData`에 P0 장착 프로파일 배열을 작게 추가한다.
3. `RoofTurret_MediumOrLarge`가 `Top_01`을 참조하게 한다.
4. `UCFVehicleWeaponComp` 골격을 만든다.
5. WeaponComp가 `Top_01` 위치 슬롯을 찾는다.
6. WeaponComp가 `FCFVehicleFireOrigin`을 계산한다.
7. Fire 입력 시 차량 Actor 위치가 아니라 터렛 발사 원점 후보를 우선 사용한다.
8. WeaponComp 디버그 캐시에 활성 프로파일 / 슬롯 / 발사 원점을 남긴다.
9. `UCFWeaponData` 최소 DataAsset 타입을 정의한다.
10. `FCFVehicleMountProfile.DefaultEquipmentPresetData` 내부 `DefaultWeaponData`에서 선택 무기를 참조한다.
11. WeaponComp / VehicleDebug Panel에서 WeaponData ID, 호환성, 요약을 확인한다.
12. `UCFTurretMountData`에 터렛 Base / Yaw / Pitch 시각 메쉬와 피벗 / Muzzle 소켓 이름을 저장한다. `FCFVehicleMountProfile`의 inline 터렛 필드는 legacy 직렬화 호환용으로만 남긴다.
13. Pawn이 `Top_01` 하드포인트 기준으로 터렛 시각 컴포넌트를 장착한다.
14. VehicleDebug Panel에서 터렛 시각 장착 여부, Yaw / Pitch 메쉬, 요약을 확인한다.
15. WeaponComp가 터렛 목표 / 현재 Yaw/Pitch와 안정화 상태를 계산하고, Pawn이 시각 피벗 회전에 적용한다.

P0 1차에서 하지 않을 것:

| 비범위 | 이유 |
|---|---|
| 완성형 WeaponRuntime 탄약 / 열 / 재장전 | 로컬 쿨다운 검증은 P0 완료, 탄약 / 열 / 재장전은 후속 Runtime 상태로 분리 |
| 완성형 Projectile Actor / Pool 튜닝 | 최소 Pool 구현과 Build 통과는 완료. PIE 재사용 확인 후 최적화 |
| 탄약 / 열 / 재장전 | Weapon Runtime 확장 단계에서 처리 |
| 실제 Damage 적용 | Dummy HitScan과 Debug 확인 이후 |
| Muzzle 탄종별 오프셋 / 다중 총구 전환 | 기본 Muzzle 소켓 FireOrigin 보정은 완료. 탄종별 변형은 후속 |
| 터렛 파괴 / 모듈 손상 | Damage / Module 단계에서 처리 |
| AI 전투 운용 | 플레이어 터렛 감각 검증 이후 |
| 차량 종류 추가 | P0 전투 루프 검증 전 확장 금지 |

---

## 13. 코드 구현 상태와 다음 후보

기존 코드 파일 완료 상태:

| 파일 | 작업 유형 | 역할 |
|---|---|---|
| `UE/Source/CarFight_Re/Public/CFTurretMountData.h` | 신규 / v1.4.0 컴파일 통과 / 링크 재시도 필요 | 최소 TurretMountData DataAsset 선언, EquipmentPresetData.DefaultTurretMountData 연결 기준 |
| `UE/Source/CarFight_Re/Private/CFTurretMountData.cpp` | 신규 / v1.4.0 컴파일 통과 / 링크 재시도 필요 | TurretMountData 기본값 / 시각 메쉬 보유 여부 / Debug 요약 구현 |
| `UE/Source/CarFight_Re/Public/CFVehicleWeaponTypes.h` | 수정 / v1.8.0 컴파일 통과 / 링크 재시도 필요 | 장착 프로파일, 터렛 상태, 발사 원점 타입, `DefaultEquipmentPresetData` 전용 참조, DefaultWeaponData / DefaultTurretMountData legacy 직접 슬롯 삭제, MountProfile legacy Min/Max Yaw/Pitch 필드 삭제 |
| `UE/Source/CarFight_Re/Public/CFEquipmentPresetData.h` | 신규 / v1.1.0 컴파일 통과 / 링크 재시도 필요 | 최소 EquipmentPresetData DataAsset 선언, EquipmentId / DisplayName / RequiredMountType / RequiredWeaponSize / DefaultTurretMountData / DefaultWeaponData 필드, 내부 참조 Missing 기준 |
| `UE/Source/CarFight_Re/Private/CFEquipmentPresetData.cpp` | 신규 / v1.1.0 컴파일 통과 / 링크 재시도 필요 | EquipmentPresetData 기본값 / 완성 여부 / 장착 호환성 / Debug 요약 구현 |
| `UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h` | 수정 / v1.17.0 컴파일 통과 / 링크 재시도 필요 | WeaponComp 선언, EquipmentPreset / WeaponData / ProjectileData / DamageData getter, 터렛 Yaw/Pitch 상태 getter와 갱신 함수, EquipmentPreset 전용 해석 기준 |
| `UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp` | 수정 / v1.17.0 컴파일 통과 / 링크 재시도 필요 | WeaponComp 구현, EquipmentPreset 전용 해석, legacy 직접 참조 fallback 제거, WeaponData 호환성 검증, ProjectileData 단일 DamageData 해석, TurretMountData 기반 터렛 목표 / 현재 Yaw/Pitch 추적 계산 |
| `UE/Source/CarFight_Re/Public/CFWeaponData.h` | 신규 / v1.5.0 컴파일 통과 / 링크 재시도 필요 | 최소 WeaponData DataAsset 선언, EquipmentPresetData.DefaultWeaponData 연결 기준, DefaultProjectileData 참조, WeaponData DamageData 직접 참조 제거 |
| `UE/Source/CarFight_Re/Private/CFWeaponData.cpp` | 신규 / v1.4.0 완료 | WeaponData 호환성 검증 / 요약 구현, DamageData는 ProjectileData 단일 경로로 안내 |
| `UE/Source/CarFight_Re/Public/CFVehicleData.h` | 수정 / v1.19.0 완료 | MountProfiles 배열 추가 |
| `UE/Source/CarFight_Re/Private/CFVehicleData.cpp` | 수정 / v1.19.0 완료 | 마이그레이션 / 에디터 보조 반영 |
| `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` | 수정 / v2.101.0 컴파일 통과 / 링크 재시도 필요 | WeaponComp / ProjectilePoolComp / 터렛 시각 컴포넌트 보유, VehicleDebug EquipmentPreset / WeaponData / Projectile / DamageData / Damage HitContext / TurretMountData / Turret Visual / Turret Aim 필드 추가, EquipmentPreset 전용 터렛 데이터 기준 |
| `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp` | 수정 / v2.100.0 컴파일 통과 / 링크 재시도 필요 | 초기화 / FireOrigin 우선 흐름 / Projectile Pool Acquire / EquipmentPreset 전용 터렛 시각 장착 / 터렛 Yaw/Pitch 시각 회전 적용 / Dummy HitScan과 Projectile Actor 충돌 HitContext 기록 |
| `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h` | 수정 / v1.25.0 빌드 통과 / PIE 검증 대기 | Weapon Panel 캐시와 EquipmentPreset / DamageData / Damage HitContext / TurretMountData / Turret Aim 표시 기준 선언 |
| `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp` | 수정 / v1.25.0 빌드 통과 / PIE 검증 대기 | Weapon Navigation 섹션과 EquipmentPreset / WeaponData / Projectile / DamageData / Damage HitContext / TurretMountData / Turret Visual / Turret Aim 표시 |

P0 타입 후보:

| 타입 | 역할 |
|---|---|
| `ECFVehicleMountType` | Fixed / Gimbal / Turret / Launcher / Utility 구분 |
| `ECFVehicleWeaponSize` | Small / Medium / Large 구분 |
| `FCFVehicleMountProfile` | 장착 프로파일 저장값 |
| `FCFVehicleTurretState` | 터렛 현재 / 목표 각도와 안정화 상태 |
| `FCFVehicleFireOrigin` | 실제 발사 위치 / 방향 계산 결과 |
| `ECFWeaponFireMode` | HitScan / Projectile 발사 방식 구분 |
| `UCFWeaponData` | 무기 ID / 호환 장착 타입 / 발사 기본값 저장 |
| `UCFEquipmentPresetData` | TurretMountData와 WeaponData의 유효 조합 저장 |
| `UCFVehicleWeaponComp` | 장착 프로파일 해석과 FireOrigin 계산 |

현재 구현 상태:

| 항목 | 상태 |
|---|---|
| `CFTurretMountData.h/.cpp` | v1.4.0 최소 TurretMountData DataAsset, EquipmentPresetData.DefaultTurretMountData 연결 기준, 시각 메쉬 / 피벗 소켓 / Muzzle FireOrigin 소켓 / 회전 한계 / 회전 속도 / 안정화 / 중량 필드 구현 |
| `CFVehicleWeaponTypes.h` | v1.8.0 `DefaultEquipmentPresetData` 전용 참조 유지, `DefaultWeaponData` / `DefaultTurretMountData` legacy 직접 슬롯 삭제, MountProfile legacy Min/Max Yaw/Pitch 필드 삭제, 남은 inline 터렛 시각 / 기계값은 직렬화 호환용으로 보존하고 에디터 노출 중단 |
| `CFEquipmentPresetData.h/.cpp` | v1.1.0 최소 EquipmentPresetData DataAsset, TurretMountData / WeaponData 유효 조합, 장착 호환성, Debug 요약 함수, 내부 참조 Missing 기준 구현 |
| `CFWeaponData.h/.cpp` | v1.5.0 최소 WeaponData DataAsset 구현, EquipmentPresetData.DefaultWeaponData 연결 기준, `DefaultProjectileData` 직접 참조, `DefaultDamageData` 제거, `FireRatePerMinute` 기반 발사 간격 환산 추가 |
| `CFProjectileActor.h/.cpp` | v1.3.0 공통 Projectile Actor와 Pool 반환 / Destroy 정책 생명주기, 마지막 충돌 Transform / ProjectileData / 발사 주체 Debug 값 구현 완료 |
| `CFProjectileData.h/.cpp` | v1.3.0 최소 ProjectileData DataAsset, 메시 설정, ProjectileActorClass 지정 여부 판정, `DefaultDamageData` 직접 참조 구현 완료 |
| `CFDamageTypes.h` | v1.1.0 `ECFDamageType`, `FCFDamageHitContext`, Dummy miss와 Projectile hit 구분용 `bBlockingHit` 구현 완료 |
| `CFDamageData.h/.cpp` | v1.0.0 최소 DamageData DataAsset, DamageType, 직접 피해 / 관통 / 폭발 / 모듈 피해 배율 / 물리 반응 후보값, Debug 요약 함수 구현 완료 |
| `CFProjectilePoolComp.h/.cpp` | v1.3.0 Projectile Actor 클래스별 Pool 버킷, Acquire / Release, 클래스별 최대 생성 수 제한, 전체 / 활성 / 비활성 Debug 카운트, 마지막 반환 요약, Hit 반환 시 Pawn HitContext 기록 전달 구현 완료 |
| `CFVehicleAimTypes.h` / `CFVehicleAimComp.h/.cpp` | v1.5.0 / v1.9.0 조준각 내부 여부를 표시 / 디버그 상태로 유지하고, 로컬 발사 가능 예측에서 조준각 초과를 단독 차단 조건에서 제외 |
| `CFVehicleWeaponComp.h/.cpp` | v1.17.0 EquipmentPresetData 전용 해석, legacy 직접 참조 fallback 제거, 활성 WeaponData MaxRange / FireRatePerMinute, FireMode 기반 Projectile Pool 준비 getter, ProjectileData 단일 DamageData 해석, `ReadyToPoolAcquire` 요약, TurretMountData 필수 소스 기반 터렛 Yaw/Pitch 추적 상태 계산, FireOrigin 하드포인트 소켓 우선 계산, Pawn 최종 FireOrigin 기록 함수 추가, 터렛 보간 후 CurrentYaw/Pitch 유효 제한각 재고정 |
| `CFVehicleData.h/.cpp` | v1.19.0 `MountProfiles`와 `Top_01` 기본 프로파일 보강 완료 |
| `CFVehiclePawn.h/.cpp` | v2.101.0 / v2.100.0 EquipmentPresetData Debug Snapshot, EquipmentPreset 전용 터렛 시각 장착, WeaponData MaxRange / FireRatePerMinute, Projectile Pool Acquire, DamageData Snapshot, Damage HitContext 기록, Pool Debug, SnapToTarget 하드포인트 소켓 우선 장착, RootDelta 진단, 필수 소켓 누락 진단, Muzzle 소켓 FireOrigin 보정, Turret Visual Snapshot, Turret Aim 시각 회전 적용, 조준각 초과 기본 발사 거부 제거 |
| `CFVehicleDebugPanelWidget.h/.cpp` | v1.25.0 `무기(Weapon)` 섹션에 EquipmentPresetData 지정 / ID / 호환 / 요약, WeaponData 분당 발사속도, ProjectileData Pool 준비, DamageData 지정 / ID / 해석 경로 / 요약, Damage HitContext 기록 / Source / Hit / Actor / Transform 요약, Projectile Pool Debug, TurretMountData / Turret Visual / Turret Aim 표시 추가 |
| `CFDamageTypes.h` | 신규 / v1.1.0 완료 | `ECFDamageType`, `FCFDamageHitContext`, `bBlockingHit` 같은 Damage 공용 타입 선언. 파일명은 32자 이하 유지 |
| `CFDamageData.h` | 신규 / v1.0.0 완료 | 최소 DamageData DataAsset 선언, DamageType, 직접 피해 / 관통 / 폭발 / 모듈 배율 / 물리 반응 후보값 선언 |
| `CFDamageData.cpp` | 신규 / v1.0.0 완료 | DamageData 기본값 / Debug 요약 / 범위 피해 게이트 구현 |
| 빌드 | legacy 직접 슬롯 제거 후 UHT / 컴파일 통과. 실행 중인 `UnrealEditor.exe`가 DLL을 잠가 링크 실패했으므로 에디터 종료 후 `Tools\BuildEditor.bat` 재실행 필요 |
| 에디터 검증 | 사용자가 `DA_TestSedan` / `DA_TestSUV` FireOrigin 테스트, `무기 > 무기 데이터` 표시, 빠른 연속 Fire 쿨다운 감소 확인 완료 |
| 쿨다운 검증 | Aim 쪽 마지막 발사 거부 사유가 `WeaponCooldown`으로 표시되는 것 확인 완료 |
| ProjectileData 검증 | 연결 전 `미지정 / Dummy HitScan 유지`, 연결 후 `지정 / 발사체 ID / 요약 표시`, 빠른 연속 Fire 쿨다운 유지 확인 완료 |
| Projectile Actor 검증 | `FireMode=Projectile` 설정 후 PIE에서 Projectile Actor가 생성되어 날아가는 것 확인 완료 |
| Projectile Pool 검증 | Pool 경로 정상 동작 확인 완료. 마지막 반환 요약의 `LifeExpired` / `Hit` 표시는 다음 PIE 세부 검증 항목 |
| Muzzle FireOrigin 검증 | 사용자가 PIE에서 Muzzle 소켓 기준 발사가 정상 동작함을 확인 완료 |
| Damage HitContext Debug | Dummy HitScan 결과와 Projectile Actor `Hit` 반환을 같은 `FCFDamageHitContext` 요약으로 기록 / 표시하는 코드 추가 후 사용자가 빌드와 PIE 정상작동 확인 완료 |
| 터렛 시각 장착 | `EquipmentPresetData.DefaultTurretMountData` 전용 경로로 전환. MountProfile 직접 fallback과 inline fallback 런타임 사용은 제거했고 에디터 종료 후 링크 재검증 필요 |
| 터렛 조준 회전 | WeaponComp 터렛 상태 계산과 Pawn Yaw/Pitch 피벗 적용 구현 완료. VehicleDebug Panel `무기 > 터렛 조준` 표시 추가, `Tools\BuildEditor.bat` 통과, PIE 회전 감각 확인 대기 |
| 하드포인트 소켓 우선 장착 | `HardpointSlots.SocketName`이 `SM_Body`에 있으면 터렛 시각 루트와 FireOrigin 모두 소켓 Transform을 우선 사용. 소켓이 없으면 기존 LocalTransform fallback |

완료된 코드 작업:

| 파일 | 작업 유형 | 역할 |
|---|---|---|
| `UE/Source/CarFight_Re/Public/CFTurretMountData.h` | 신규 / v1.0.0 완료 | 최소 TurretMountData DataAsset 선언 |
| `UE/Source/CarFight_Re/Private/CFTurretMountData.cpp` | 신규 / v1.0.0 완료 | TurretMountData 기본값, 시각 메쉬 보유 여부, Debug 요약 구현 |
| `UE/Source/CarFight_Re/Public/CFProjectileActor.h` | 신규 / v1.2.0 완료 | 공통 Projectile Actor 선언, Pool 전환용 생명주기 API, 비활성화 사유 Debug getter |
| `UE/Source/CarFight_Re/Private/CFProjectileActor.cpp` | 신규 / v1.2.0 완료 | ProjectileData 기반 메시 / 충돌 / 이동 / 수명 / Pool 반환 처리, 반환 사유 / 비행 시간 기록 |
| `UE/Source/CarFight_Re/Public/CFProjectilePoolComp.h` | 신규 / v1.2.0 완료 | Projectile Actor 클래스별 Pool 버킷, Acquire / Release API, Debug 카운트 getter, 마지막 반환 요약 getter |
| `UE/Source/CarFight_Re/Private/CFProjectilePoolComp.cpp` | 신규 / v1.2.0 완료 | Pool 확보, 반환, 한도 초과 fallback 신호, EndPlay 정리, 유효 Actor 기준 카운트 계산, 마지막 반환 요약 생성 구현 |
| `UE/Source/CarFight_Re/Public/CFProjectileData.h` | 신규 / v1.2.0 완료 | 최소 ProjectileData DataAsset 선언, 메시 설정, `ProjectileActorClass` 지정 여부 getter |
| `UE/Source/CarFight_Re/Private/CFProjectileData.cpp` | 신규 / v1.2.0 완료 | ProjectileData 기본값 / Debug 요약 / 스폰 준비 판정 구현 |
| `UE/Source/CarFight_Re/Public/CFWeaponData.h` | 수정 / v1.2.0 완료 | `DefaultProjectileData` 참조, `FireRatePerMinute` 입력, 기존 `CooldownSeconds` 숨김 마이그레이션 값 보존 |
| `UE/Source/CarFight_Re/Private/CFWeaponData.cpp` | 수정 / v1.2.0 완료 | Weapon 요약에 ProjectileData 지정 여부와 FireRate / 환산 발사 간격 추가 |
| `UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h` | 수정 / v1.7.0 완료 | 활성 ProjectileData getter / FireRatePerMinute getter / FireMode 기반 Pool 준비 getter / Debug 캐시 추가 |
| `UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp` | 수정 / v1.7.0 완료 | WeaponData에서 ProjectileData와 FireRatePerMinute를 읽고 `FireMode=Projectile` 기준 `ReadyToPoolAcquire` 상태 캐시 |
| `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` | 수정 / v2.85.0 완료 | ProjectilePoolComp 보유와 Projectile Pool Fire 경로 함수, FireRate / Pool Debug Snapshot / 마지막 반환 요약 필드 선언 |
| `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp` | 수정 / v2.85.0 완료 | Fire 검증 통과 후 Projectile Pool Acquire 실행, 실패 시 Dummy HitScan fallback, FireRate / Pool Debug Snapshot / 마지막 반환 요약 채우기 |
| `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h` | 수정 / v1.17.0 완료 | Weapon Panel ProjectileData Pool 준비 표시 기준, 분당 발사속도, Pool 카운트 / 마지막 반환 요약 표시 추가 |
| `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp` | 수정 / v1.17.0 완료 | `무기 > 무기 데이터`의 쿨다운 표시를 분당 발사속도로 전환하고 Pool 카운트 / 마지막 반환 요약 표시 유지 |
| `UE/Source/CarFight_Re/Public/CFVehicleWeaponTypes.h` | 수정 / v1.6.0 빌드 / PIE 확인 완료 | `FCFVehicleMountProfile`에 `DefaultTurretMountData` 참조와 `bExposedModule` 노출은 유지하고, legacy Min/Max Yaw/Pitch 필드는 DataAsset 재저장 확인 뒤 실제 삭제 |
| `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` | 수정 / v2.87.0 완료 | 터렛 시각 컴포넌트 보유와 VehicleDebug TurretMountData / Turret Visual Snapshot 필드 선언 |
| `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` | 수정 / v2.99.0 빌드 / PIE 확인 완료 | ActiveTurretMountData Debug 툴팁을 MountProfile inline fallback 제거 정책에 맞게 정리 |
| `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp` | 수정 / v2.98.0 빌드 / PIE 확인 완료 | `Top_01` 하드포인트 기준 TurretMountData 전용 터렛 Base / Yaw / Pitch 메쉬 장착, inline fallback 제거, Turret Visual Snapshot 채우기 |
| `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h` | 수정 / v1.19.0 완료 | Weapon Panel TurretMountData / Turret Visual 하위 섹션 표시 기준 추가 |
| `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp` | 수정 / v1.19.0 완료 | `무기 > 터렛 시각` 마운트 데이터 / 장착 여부 / Yaw 메쉬 / Pitch 메쉬 / 요약 표시 추가 |
| `UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h` | 수정 / v1.15.0 빌드 / PIE 확인 완료 | 터렛 조준 추적 상태 getter, 런타임 요약 getter, `UpdateTurretState` / `ResetTurretState` 선언, TurretMountData 단독 회전 제한 기준 추가 |
| `UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp` | 수정 / v1.15.0 빌드 / PIE 확인 완료 | MountProfile 각도 교집합을 제거하고 TurretMountData 회전 제한 / 회전 속도 / 안정화 기준만 사용해 터렛 Yaw/Pitch 상태 갱신 |
| `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` | 수정 / v2.89.0 완료 | VehicleDebug Turret Aim 필드와 터렛 조준 방향 / 시각 회전 적용 함수 선언 |
| `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp` | 수정 / v2.89.0 완료 | AimComp / CameraComp 조준 방향을 WeaponComp에 전달하고 YawPivot / PitchPivot 상대 회전에 적용 |
| `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h` | 수정 / v1.21.0 완료 | Weapon Panel Turret Aim 표시 기준 추가 |
| `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp` | 수정 / v1.21.0 완료 | `무기 > 터렛 조준` 현재/목표 Yaw/Pitch, 안정화, 요약 표시 추가 |
| `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h` | 수정 / v1.5.0 완료 | 조준각 내부 여부를 표시 / 디버그 상태로 정리하고 `OutOfWeaponArc`를 호환용 enum으로 유지 |
| `UE/Source/CarFight_Re/Public/CFVehicleAimComp.h` | 수정 / v1.9.0 완료 | 로컬 발사 가능 예측에서 조준각 초과를 단독 차단 조건에서 제외 |
| `UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp` | 수정 / v1.9.0 완료 | `bLocalCanFire`와 Reticle 상태 계산에서 조준각 초과를 기본 발사 차단으로 보지 않도록 수정 |
| `UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h` | 수정 / v1.11.0 완료 / v1.15.0에서 기준 갱신 | 터렛 회전값은 발사 정책과 분리하고 TurretMountData 최종 유효 제한각 안에 고정한다는 마이그레이션 기준으로 갱신 |
| `UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp` | 수정 / v1.11.0 완료 / v1.15.0에서 기준 갱신 | 보간 이후 `CurrentYawDeg` / `CurrentPitchDeg`를 TurretMountData 제한각으로 재고정 |
| `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` | 수정 / v2.94.0 완료 | 터렛 안정화 전 발사 정책 마이그레이션 기준 추가 |
| `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp` | 수정 / v2.94.0 완료 | 로컬 Fire 검증에서 조준각 초과만으로 `OutOfWeaponArc` 거부를 만들던 경로 제거 |
| `UE/Source/CarFight_Re/Public/CFProjectileData.h` | 수정 / v1.4.0 완료 | `DefaultDamageData` 단일 직접 참조와 `DamageProfileId` Debug fallback 유지 |
| `UE/Source/CarFight_Re/Private/CFProjectileData.cpp` | 수정 / v1.4.0 완료 | ProjectileData 요약에 DamageData 지정 / ID / 에셋 / 요약 포함 |
| `UE/Source/CarFight_Re/Public/CFWeaponData.h` | 수정 / v1.4.0 완료 | `DefaultDamageData` fallback 참조 제거, BaseDamage / DamageProfileId는 레거시 확인용으로 유지 |
| `UE/Source/CarFight_Re/Private/CFWeaponData.cpp` | 수정 / v1.4.0 완료 | WeaponData 요약에서 DamageData 직접 참조 제거, ProjectileData 연결과 레거시 피해값만 표시 |
| `UE/Source/CarFight_Re/Public/CFVehicleWeaponComp.h` | 수정 / v1.13.0 완료 | 활성 DamageData / DamageId / 요약 / 해석 경로 getter 유지, ProjectileData 단일 소유 정책 반영 |
| `UE/Source/CarFight_Re/Private/CFVehicleWeaponComp.cpp` | 수정 / v1.13.0 완료 | `ProjectileData.DefaultDamageData`만 직접 참조하고, `ProjectileData.DamageProfileId`는 Debug fallback으로만 캐시 |
| `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` | 수정 / v2.96.0 완료 | VehicleDebug Weapon Snapshot의 DamageData 설명을 ProjectileData 단일 소유 정책에 맞게 정리 |
| `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp` | 수정 / v2.96.0 완료 | WeaponComp의 ProjectileData 기준 DamageData 해석 결과를 VehicleDebug Snapshot에 채움 |
| `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h` | 수정 / v1.23.0 완료 | Weapon Panel DamageData 표시 기준을 ProjectileData 단일 소유 정책에 맞게 정리 |
| `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp` | 수정 / v1.23.0 완료 | `무기 > 피해 데이터` 지정 여부 / 피해 ID / 해석 경로 / 요약 표시 유지 |
| `UE/Source/CarFight_Re/Public/CFDamageTypes.h` | 수정 / v1.1.0 완료 | `FCFDamageHitContext.bBlockingHit` 추가로 Dummy HitScan miss와 Projectile hit 구분 |
| `UE/Source/CarFight_Re/Public/CFProjectileActor.h` | 수정 / v1.3.0 완료 | 마지막 충돌 Actor / 위치 / 노멀 / 입사 방향 / ProjectileData / 발사 주체 getter 추가 |
| `UE/Source/CarFight_Re/Private/CFProjectileActor.cpp` | 수정 / v1.3.0 완료 | 충돌 또는 비충돌 비활성화 시 HitContext Debug에 필요한 마지막 Transform 기록 |
| `UE/Source/CarFight_Re/Public/CFProjectilePoolComp.h` | 수정 / v1.3.0 완료 | Hit 반환 Projectile Actor를 Pawn HitContext 기록 경로로 넘기는 책임 추가 |
| `UE/Source/CarFight_Re/Private/CFProjectilePoolComp.cpp` | 수정 / v1.3.0 완료 | `Reason=Hit` 반환 시 소유 Pawn의 `RecordProjectileDamageHitContextFromPool` 호출 |
| `UE/Source/CarFight_Re/Public/CFVehiclePawn.h` | 수정 / v2.97.0 완료 | 마지막 Damage HitContext 캐시, Snapshot 필드, Dummy / Projectile 기록 함수 선언 |
| `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp` | 수정 / v2.97.0 완료 | Dummy HitScan 결과와 Projectile Actor hit 반환을 `FCFDamageHitContext`로 저장하고 요약 생성 |
| `UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h` | 수정 / v1.24.0 완료 | Weapon Panel Damage HitContext 표시 기준 반영 |
| `UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp` | 수정 / v1.24.0 완료 | `무기 > 피해 HitContext` 하위 섹션 추가 |

추가 타입 완료 상태:

| 타입 | 역할 |
|---|---|
| `ACFProjectileActor` | ProjectileData를 읽어 메시 / 충돌 / 이동 / 수명 종료를 처리하는 공통 Actor |
| `UCFProjectilePoolComp` | Projectile Actor 클래스별 재사용 Pool을 관리하고 Acquire / Release 생명주기를 처리하는 ActorComponent |
| `UCFProjectileData` | 발사체 이동 / 충돌 / DamageData 참조 후보를 저장하는 DataAsset |
| `UCFEquipmentPresetData` | 유저-facing 장비 패키지 단위로 TurretMountData와 WeaponData 조합을 저장하는 DataAsset |
| `UCFDamageData` | 맞았을 때의 피해 / 관통 / 폭발 / 모듈 피해 후보값을 저장하는 DataAsset |
| `FCFDamageHitContext` | Projectile / HitScan 명중 이벤트를 같은 형식으로 기록할 런타임 구조체 |
| `UCFTurretMountData` | 무기 거치대 / 회전 플랫폼 / 조준 기계장치의 시각 메쉬, 피벗 소켓, 회전 한계, 회전 속도, 안정화 기준을 저장하는 DataAsset |
| `FCFVehicleDebugWeapon` 확장 | 활성 ProjectileData 지정 여부, 스폰 준비 상태, 요약 표시 |
| `Turret_MountRoot` / `Turret_YawPivot` / `Turret_PitchPivot` | 하드포인트 기준 터렛 시각 장착과 Yaw/Pitch 회전 적용용 Pawn 컴포넌트 |

---

## 14. 검증 기준

문서 검증:

1. 차량 / 장착 프로파일 / 터렛 / 무기 / 탄환 / Damage의 소유권이 분리되어 있다.
2. P0 구현 범위가 `MountProfile + WeaponComp + FireOrigin + Projectile Pool + Turret Visual + Turret Aim Visual`의 안전한 검증 단위로 제한되어 있다.
3. Damage 적용과 완성형 Weapon Runtime이 후순위로 분리되어 있다.
4. BP가 전투 규칙 원본이 되지 않는다고 명시되어 있다.

코드 검증:

1. `D:\Work\CarFight_git\Tools\BuildEditor.bat` 빌드가 통과한다.
2. `DA_TestSedan`에서 `Top_01` 위치 슬롯을 찾을 수 있다.
3. `RoofTurret_MediumOrLarge`가 `Top_01`을 참조한다.
4. Fire 입력 시 Debug에서 차량 중심이 아닌 `Top_01` 기준 발사 원점이 보인다.
5. 기존 Aim / Reticle / Dummy HitScan 흐름이 깨지지 않는다.
6. VehicleDebug Panel Navigation에 `무기` 섹션이 보인다.
7. Fire 후 `무기 > 발사 원점`에서 `Top_01`, `RoofTurret_MediumOrLarge`, 월드 위치 / 방향을 볼 수 있다.
8. `무기 > 무기 데이터`에서 WeaponData 지정 여부, WeaponId, 호환성이 보인다.
9. EquipmentPresetData 내부 WeaponData가 비어 있어도 기존 FireOrigin 계산은 유지된다.
10. `DefaultEquipmentPresetData`가 비어 있으면 기존 직접 참조 fallback 없이 EquipmentPreset / TurretMountData / WeaponData는 Missing 상태로 표시된다. FireOrigin / Dummy HitScan / 즉시 발사 fallback은 유지된다.
11. `DefaultEquipmentPresetData`가 지정되면 VehicleDebug Panel `무기 > 장비 프리셋`에서 지정 여부, 프리셋 ID, 장착 호환, 요약이 표시된다.
12. `DefaultEquipmentPresetData` 내부 `DefaultTurretMountData`가 있으면 터렛 시각 장착과 터렛 조준은 프리셋 내부 TurretMountData를 우선 사용한다.
13. `DefaultEquipmentPresetData` 내부 `DefaultWeaponData`가 있으면 WeaponData / ProjectileData / DamageData 해석은 프리셋 내부 WeaponData를 우선 사용한다.
14. EquipmentPresetData를 연결해도 빠른 연속 발사 시 기존 남은 쿨다운 감소와 `WeaponCooldown` 거부 사유는 유지된다.
15. WeaponData가 유효하면 Dummy HitScan Trace 거리는 `WeaponData.MaxRange`를 사용한다.
16. WeaponData가 유효하면 `FireRatePerMinute`은 `60 / FireRatePerMinute` 발사 간격으로 환산되어 Fire 검증에 사용된다.
17. WeaponData가 유효하고 쿨다운 중이면 FireResult RejectReason이 `WeaponCooldown`이 된다.
18. VehicleDebug Panel `무기 > 무기 데이터`에서 `분당 발사속도`가 표시된다. 예: 60이면 `60`으로 표시한다.
19. PIE에서 빠른 연속 Fire 시 VehicleDebug Panel의 `남은 쿨다운`이 감소하고 Aim 쪽 마지막 거부 사유가 `WeaponCooldown`으로 표시된다.
15. Projectile Actor 스폰은 `WeaponData.FireMode=Projectile`, 활성 WeaponData 호환 통과, `DefaultProjectileData` 유효, `ProjectileActorClass` 유효 조건에서만 실행된다.
16. ProjectileData가 비어 있거나 FireMode가 HitScan이면 기존 Dummy HitScan, FireOrigin, 발사 간격 검증은 유지된다.
17. VehicleDebug Panel `무기 > 발사체 데이터`에서 지정 여부, 발사체 ID, fallback 상태, 요약이 표시된다.
18. `DefaultProjectileData`가 비어 있으면 `발사체 데이터`는 미지정이고 fallback은 `Dummy HitScan 유지`로 표시된다.
19. `DefaultProjectileData` 연결 후 `발사체 데이터`는 지정 상태, 발사체 ID, 요약을 표시한다.
20. ProjectileData 연결 전/후 모두 빠른 연속 Fire 시 남은 쿨다운 감소와 `WeaponCooldown` 거부 사유가 유지된다.
21. `ProjectileActorClass`가 비어 있으면 `스폰 준비=미준비`, `Fallback=Dummy HitScan 유지`로 표시된다.
22. `ProjectileActorClass`가 지정되고 `FireMode=Projectile`이면 `스폰 준비=준비됨`, `Fallback=Projectile Pool 준비됨`, `전환 요약=ReadyToPoolAcquire` 계열로 표시된다.
23. Projectile Pool 준비 상태에서 Fire 입력이 승인되면 FireOrigin 위치 / 방향에서 공통 Projectile Actor가 Pool에서 확보되고 활성화된다.
24. Projectile Pool 확보 / 활성화 실패 시 같은 Fire 입력은 Dummy HitScan fallback으로 처리된다.
25. Projectile Actor는 `ProjectileStaticMesh`, `CollisionRadius`, `InitialSpeed`, `LifeTimeSeconds`, `bAffectedByGravity`, `GravityScale`을 ProjectileData에서 읽는다.
26. Pool 소유 Projectile Actor는 충돌 또는 수명 종료 시 `DeactivateProjectile` 경로를 통해 Pool에 반환된다. Pool 소유자가 없으면 기존 Destroy 정책을 유지한다.
27. PIE에서 `FireMode=Projectile` 설정 후 Projectile Actor가 FireOrigin 기준으로 날아가는 것을 확인했다.
28. VehicleDebug Panel `무기 > 발사체 Pool`에서 `Pool 컴포넌트`, `전체 Pool 수`, `활성 Pool 수`, `비활성 Pool 수`, `마지막 반환 요약`이 표시된다.
29. `Tools\BuildEditor.bat`는 Projectile Pool 마지막 반환 요약 추가 후 통과했다.
30. 사용자는 FireRatePerMinute 전환 후에도 빠른 연속 발사, 남은 쿨다운 감소, `WeaponCooldown` 거부 사유가 정상 동작함을 확인했다.
31. 사용자는 Projectile Pool 발사 경로가 정상 동작함을 확인했다.
32. 사용자는 VehicleDebug Panel `무기 > 발사체 Pool > 마지막 반환 요약` 표시를 확인했다.
33. DamageData 전환 전 회귀 체크에서는 수명 종료 반환 시 `마지막 반환 요약`이 `Reason=LifeExpired`와 비행 시간을 표시하는지 확인한다.
34. DamageData 전환 전 회귀 체크에서는 충돌 반환 시 `마지막 반환 요약`이 `Reason=Hit`와 `HitActor`를 표시하는지 확인한다.
35. DamageData 전환 전 회귀 체크에서는 중력 켜기 / 끄기와 `GravityScale` 변경 시 비행 궤적이 바뀌고, 기존 Pool 반환 요약과 쿨다운 표시가 유지되는지 확인한다.
36. Pool 확보 실패 시 Dummy HitScan fallback 유지 여부를 확인한다.
37. `EquipmentPresetData.DefaultTurretMountData`가 지정되면 Pawn의 터렛 시각 컴포넌트가 `Top_01` 하드포인트에 붙고, Base / YawPivot / Yaw / PitchPivot / Pitch 계층을 구성한다.
38. `EquipmentPresetData.DefaultTurretMountData`가 비어 있으면 기존 inline `MountProfile.TurretYawMesh` 또는 `TurretPitchMesh`가 지정되어 있어도 터렛 시각 컴포넌트는 붙지 않는다.
39. VehicleDebug Panel `무기 > 터렛 시각`에서 마운트 데이터 미지정, `Source=MissingEquipmentPresetTurretMountData`, `InlineFallback=Removed` 계열 요약을 확인한다.
40. `EquipmentPresetData.DefaultTurretMountData`가 지정되고 메쉬가 유효하면 VehicleDebug Panel `무기 > 터렛 시각`에서 마운트 데이터 지정 여부, `장착 여부=예`, Base / Yaw / Pitch 메쉬 이름, `TurretVisual: Attached` 요약이 표시된다.
41. 터렛 시각 메쉬가 비어 있으면 `무기 > 터렛 시각`은 미장착 또는 선택 누락 상태로 표시하되 FireOrigin / Projectile / Cooldown 검증은 유지된다.
42. VehicleDebug Panel `무기 > 터렛 조준`에서 `EquipmentPresetData.DefaultTurretMountData` 지정 시 현재/목표 Yaw, 현재/목표 Pitch, 안정화 여부, `TurretRuntime: Updated` 요약이 표시된다. 미지정 시 터렛 조준 추적은 `TurretMountData=Missing` 계열로 건너뛴다.
43. 조준 방향을 좌우 / 상하로 바꾸면 `Turret_YawPivot`, `Turret_PitchPivot`이 회전하고 현재 각도가 목표 각도를 제한 속도로 따라간다.
44. Pitch 메쉬에 `MuzzleSocketName` 소켓이 있으면 `ACFVehiclePawn::TryBuildMuzzleFireOrigin`이 최종 FireOrigin 위치와 방향을 Muzzle 소켓 기준으로 보정한다.
45. Muzzle 소켓이 없거나 Pitch 메쉬가 없으면 기존 하드포인트 FireOrigin fallback을 유지한다.
46. VehicleDebug Panel `무기 > 터렛 시각 > 요약`에서 `MuzzleStatus=Resolved`이면 Muzzle FireOrigin 전환 후보가 정상이고, 누락 시 `MissingRequiredSockets=Muzzle:...`가 표시된다.
47. `Tools\BuildEditor.bat`는 inline fallback 런타임 제거 후 재실행해야 한다.
48. `HardpointSlots.SocketName`이 차체 `SM_Body` 소켓에 있으면 터렛 시각 루트와 하드포인트 fallback FireOrigin은 해당 소켓 Transform을 우선 사용한다.
49. VehicleDebug Panel `무기 > 터렛 시각 > 요약`에 `HardpointSocketResolved=Yes`가 표시되면 하드포인트 소켓 부착 경로로 해석한다.
50. `RootDelta`가 0에 가까운데 보이는 터렛 메쉬가 어긋나면 `TurretBaseRelativeTransform`, `TurretYawRelativeTransform`, `TurretPitchRelativeTransform`, 또는 각 메쉬 피벗 / 소켓 위치를 조정한다.
51. 조준점이 터렛 유효 각도 밖에 있어도 발사 입력은 `OutOfWeaponArc`로 거부되지 않는다.
52. 조준점이 터렛 유효 각도 밖에 있을 때 VehicleDebug Panel `무기 > 터렛 조준`의 목표 / 현재 Yaw/Pitch는 TurretMountData 제한각 안에 머문다.
53. 터렛이 목표각까지 아직 회전 중이어도 필수 거부 조건이 없으면 Projectile 또는 Dummy HitScan 발사는 진행된다.
54. 빠른 연속 발사 시 기존처럼 남은 쿨다운 감소와 `WeaponCooldown` 거부 사유는 유지된다.
55. 탄퍼짐 / 명중률 보정은 아직 적용하지 않는다. 같은 입력에서 추가 랜덤 편차가 생기면 회귀로 본다.
56. DamageData가 비어 있어도 기존 Projectile / Dummy HitScan / FireRatePerMinute 쿨다운 검증은 유지된다.
57. DamageData 직접 참조 슬롯은 `ProjectileData.DefaultDamageData` 하나만 허용한다.
58. VehicleDebug Panel `무기 > 피해 데이터`에서 DamageData 지정 여부, DamageId, 해석 경로, 요약을 확인할 수 있어야 한다.
59. DamageData 연결 단계에서는 실제 HP 차감이 발생하지 않는다.
60. VehicleDebug Panel `무기 > 피해 HitContext`에서 기록 여부, Source, Hit, DamageId, WeaponId, ProjectileId, 피격 Actor, 발사 주체, 위치, 노멀, 입사 방향, 요약을 확인할 수 있어야 한다.
61. 사용자는 차량 DA의 MountProfiles 상세에서 `MinYawDeg`, `MaxYawDeg`, `MinPitchDeg`, `MaxPitchDeg` 미노출 상태가 정상 동작함을 확인했다.
62. 사용자는 차량 DA의 MountProfiles 상세에서 `bExposedModule`이 기존처럼 유지되는 상태가 정상 동작함을 확인했다.
63. 사용자는 VehicleDebug Panel `무기 > 터렛 조준 > 요약`이 MountProfile 각도 fallback 없이 TurretMountData 단독 제한 기준으로 정상 동작함을 확인했다.
64. Dummy HitScan 경로는 FireResult의 LocalHitLocation / LocalHitNormal을 `FCFDamageHitContext`에 저장하고, Source는 `Dummy HitScan`, `bFromProjectileActor=false`로 표시한다.
65. Dummy HitScan miss 경로는 `Hit=아니오`, `bBlockingHit=false`, 피격 Actor `None`으로 표시하되 FireOrigin / 쿨다운 / 기존 발사 결과를 바꾸지 않는다.
66. Projectile Actor 충돌 경로는 Pool 반환 사유가 `Hit`일 때 마지막 충돌 Actor / 위치 / 노멀 / 입사 방향 / 비행 시간을 `FCFDamageHitContext`에 저장하고, Source는 `Projectile Actor`, `bFromProjectileActor=true`, `bBlockingHit=true`로 표시한다.
67. Projectile Actor `LifeExpired` / 수동 비활성화는 Damage HitContext를 새로 만들지 않고, 기존 `무기 > 발사체 Pool > 마지막 반환 요약` 회귀 기준만 유지한다.
68. Damage HitContext Debug 추가 후에도 빠른 연속 Fire 시 남은 쿨다운 감소와 `WeaponCooldown` 거부 사유는 유지되어야 한다.
66. Damage HitContext Debug 추가 단계에서는 실제 HP 차감 / 모듈 손상 / 장갑 방향 판정 / 폭발 범위 검색이 발생하지 않아야 한다.

에디터 검증:

1. 콘텐츠 브라우저(`Content Browser`)에서 `DA_TestSedan`을 연다.
2. 세부 정보(`Details`)에서 `HardpointSlots`에 `Top_01`이 있는지 확인한다.
3. `Top_01.LocalLocation`, `Top_01.LocalRotation`이 0에 가깝지 않은지 확인한다.
4. 콘텐츠 브라우저(`Content Browser`)에서 터렛 마운트 DataAsset을 만든다.
5. 터렛 마운트 DataAsset에서 Base / Yaw / Pitch 메쉬, YawPivot / PitchPivot / Muzzle 소켓을 지정한다. Muzzle 소켓의 X축은 발사 방향으로 둔다.
6. `DA_TestSedan > MountProfiles > RoofTurret_MediumOrLarge`에서 `기본 장비 프리셋 데이터 (DefaultEquipmentPresetData)`를 지정한다.
7. 지정한 EquipmentPresetData 내부에서 `기본 터렛 마운트 데이터 (DefaultTurretMountData)`와 `기본 무기 데이터 (DefaultWeaponData)`를 지정한다.
8. PIE에서 터렛 시각이 차체 `Top_01` 위치에 붙는지 확인한다.
9. VehicleDebug Panel `무기 > 터렛 시각`에서 마운트 데이터 지정 여부, 장착 여부, Base / Yaw / Pitch 메쉬 이름, 요약을 확인한다.
10. VehicleDebug Panel `무기 > 터렛 조준`에서 현재/목표 Yaw, 현재/목표 Pitch, 안정화 여부가 조준 입력에 따라 갱신되는지 확인한다.
11. VehicleDebug Panel `무기 > 터렛 시각 > 요약`에서 `MuzzleStatus=Resolved`와 `SocketValidation=OK`를 확인한다.
12. VehicleDebug Panel `무기 > 발사 원점`과 Projectile 비행 시작 위치가 Pitch 메쉬 Muzzle 소켓 위치와 일치하는지 확인한다.
13. 콘텐츠 브라우저(`Content Browser`)에서 DamageData DataAsset을 만들거나 기존 DamageData를 연다. 에디터 표시 타입은 `CFDamageData`다.
14. `DA_TestSedan`의 WeaponData에 HitScan / Laser용 가상 ProjectileData 또는 실제 ProjectileData를 연결하고, 해당 ProjectileData에 `기본 피해 데이터 (DefaultDamageData)`를 연결한다.
15. PIE에서 VehicleDebug Panel `무기 > 피해 데이터`의 `지정 여부`, `피해 ID`, `해석 경로`, `요약`을 확인한다.
16. ProjectileData를 제거하면 `해석 경로=MissingProjectileData` 계열이 표시되고, 발사 / 쿨다운 / Dummy HitScan fallback은 유지되는지 확인한다.
17. Dummy HitScan 발사 후 VehicleDebug Panel `무기 > 피해 HitContext`에서 `Source=Dummy HitScan`, `Hit` 상태, `피해 ID`, `무기 ID`, `발사체 ID`, 위치 / 방향 요약을 확인한다.
18. Projectile Actor가 충돌한 뒤 VehicleDebug Panel `무기 > 피해 HitContext`에서 `Source=Projectile Actor`, `Hit=예`, `피격 Actor`, 비행 시간이 갱신되는지 확인한다.
19. Projectile Actor가 수명 종료만 된 경우 `무기 > 피해 HitContext`가 이전 hit 기록을 무리하게 덮어쓰지 않고, `무기 > 발사체 Pool > 마지막 반환 요약`만 `LifeExpired`로 갱신되는지 확인한다.
20. `DA_TestSUV`는 같은 방식으로 수동 대조한다.

---

## 15. 결정 사항

1. P0 1차는 `UCFVehicleData`에 장착 프로파일을 작게 추가한다.
2. 장착 프로파일이 커지면 별도 Combat 또는 Loadout DataAsset으로 분리한다.
3. P0 첫 대상은 `Top_01 + Turret + RoofTurret_MediumOrLarge`다.
4. 터렛 회전 / 안정화 / 실제 발사 원점은 C++ `UCFVehicleWeaponComp`가 계산한다.
5. 터렛 BP는 시각 표현 전용이다.
6. WeaponData는 최소 DataAsset으로 분리하고, `MaxRange` / `FireRatePerMinute`의 P0 로컬 검증 연결은 완료했다.
7. 탄약 / 열 / 재장전은 후속 WeaponRuntime으로 미룬다.
8. Projectile Actor / Pool 경로와 Muzzle 소켓 FireOrigin 보정은 구현과 PIE 확인을 완료했고, DamageData 참조 연결과 Debug 표시는 구현 / 빌드 완료, 실제 Damage 적용은 후속 HitContext 검증 이후로 미룬다.
9. 현재 코드에서 WeaponData는 기본 ProjectileData를 직접 참조한다.
10. ProjectileData는 이동 / 충돌 / 표현 후보 / DamageData 참조를 소유하고, 실제 피해량 / 관통 / 피해 타입은 DamageData가 소유한다.
11. P0 터렛 시각 메쉬와 회전 기계값은 `UCFTurretMountData`에 저장하고, `FCFVehicleMountProfile`은 차량 슬롯 허용 조건과 기본 참조만 가진다.
12. 유저에게는 장기적으로 `EquipmentPreset` 단위로 보여주고, MountType / TurretMountData / WeaponData 분리는 내부 구현과 디버그 기준으로 유지한다.
13. UI와 Debug는 C++ 계산 결과를 표시만 한다.
14. P0 터렛은 목표각까지 안정화되기 전에도 발사한다. 안정화 여부는 Debug / 후속 정확도 시스템용 상태이며 기본 발사 금지 조건이 아니다.
15. 터렛 회전은 DA에 지정한 최소 / 최대 각도 안에서만 움직인다. 조준점이 제한각 밖이어도 발사는 막지 않고, 실제 발사 방향은 현재 Muzzle / 터렛 제한각 결과를 따른다.
16. 탄퍼짐 / 명중률은 이번 단계에서 적용하지 않는다. 후속 사격 시스템에서 별도 설계한다.
17. DamageData 최소 설계는 확정한다. DamageData 직접 참조는 `ProjectileData.DefaultDamageData` 하나만 허용하고, `ProjectileData.DamageProfileId`는 Debug fallback으로만 사용한다.
18. DamageData 연결 단계에서는 실제 HP 차감 / 모듈 손상을 하지 않고, 공통 HitContext Debug부터 검증한다.

---

## 16. Open Questions

| 항목 | 현재 기본안 | 재검토 시점 |
|---|---|---|
| MountProfiles를 계속 `UCFVehicleData`에 둘지 | P0는 차량 DA에 작게 추가 | 프로파일이 차량별 피팅표처럼 커질 때 |
| 터렛 마운트 데이터 저장 위치 | P0는 `EquipmentPresetData.DefaultTurretMountData` 직접 참조. 기존 `FCFVehicleMountProfile` inline 필드는 직렬화 호환용 legacy 값이며 런타임 fallback은 제거 | 남은 inline 직렬화 호환 필드 삭제 판단 시점 |
| 터렛 Yaw / Pitch 회전 감각 | Base는 고정, YawPivot은 `YawPivotSocketName` 소켓 또는 fallback 원점 기준, PitchPivot은 `PitchPivotSocketName` 소켓 또는 fallback 원점 기준으로 구현 | PIE에서 축 방향 / 부호 / 회전 속도 체감 확인 직후 |
| Muzzle 소켓 FireOrigin 회귀 | 코드 구현과 PIE 정상작동 확인 완료. `MuzzleStatus=Resolved`일 때 Pitch 메쉬 Muzzle 소켓 위치와 X축 방향을 최종 FireOrigin으로 사용 | 총구 축 기준을 탄종별로 달리해야 할 때 |
| 유저 노출 장비 단위 | 최소 코드 반영. `EquipmentPresetData`가 TurretMountData와 WeaponData의 유효 조합을 소유하고 MountProfile의 legacy 직접 슬롯은 삭제했다 | 에디터 종료 후 재빌드 / PIE 회귀 확인 |
| 터렛 안정화 전 발사 정책 | 확정. 안정화 전에도 필수 거부 조건이 없으면 발사한다 | 후속 사격 정확도 / 탄퍼짐 시스템 설계 시 |
| 탄퍼짐 / 명중률 적용 시점 | 현재 미적용. 후속 사격 시스템에서 별도 설계 | DamageData 이후 무기별 체감 튜닝 단계 |
| 완성형 WeaponRuntime 적용 시점 | WeaponData 연결 검증 이후 | 탄약 / 열 / 재장전이 필요해질 때 |
| Projectile Actor 전환 시점 | 최소 DataAsset / Debug 연결, 실제 비행, Pool Acquire 빌드 통과 완료 | Damage 적용 전에 Pool PIE 재사용 검증 |
| `ProjectileDataId` 유지 여부 | 현재 v1.1 코드에서는 마이그레이션 / 로그용으로 보존 | 직접 참조가 안정화된 뒤 |
| Damage 적용 시점 | DamageData 연결과 HitContext Debug 검증 이후 | 명중 방향 / ArmorProfile이 필요해질 때 |
| DamageData 참조 위치 | `ProjectileData.DefaultDamageData` 단일 직접 참조. HitScan / Laser는 가상 ProjectileData 사용 | `ProjectileData`라는 이름이 Laser / HitScan에 지나치게 어색해져 `ShotData` 등으로 이름 재검토할 때 |

---

## 17. Changelog

### v0.1

- 전투 데이터 소유권 문서를 신규 작성했다.
- 차량 / 장착 프로파일 / 터렛 / 무기 / 탄환 / Damage / UI 상태의 책임 경계를 분리했다.
- P0 1차 구현 범위를 `MountProfile + UCFVehicleWeaponComp + FireOrigin`으로 제한했다.
- WeaponData / ProjectileData / Damage 적용은 P0 1차 이후로 미뤘다.

### v0.2

- P0 코드 구현 결과를 문서에 반영했다.
- `CFVehicleWeaponTypes`, `CFVehicleWeaponComp`, `UCFVehicleData::MountProfiles`, `ACFVehiclePawn::VehicleWeaponComp` 구현 상태를 추가했다.
- Debug Panel 확장은 아직 후속 검증 항목으로 남겼다.

### v0.3

- 사용자의 FireOrigin 테스트 완료 상태를 반영했다.
- `CFVehiclePawn` v2.76.0의 VehicleDebug Weapon Snapshot 추가를 반영했다.
- `CFVehicleDebugPanelWidget` v1.9.0의 `무기` Navigation 섹션과 FireOrigin 표시 구현을 반영했다.

### v0.4

- `UCFWeaponData` v1.0.0 최소 DataAsset 구현 상태를 반영했다.
- `FCFVehicleMountProfile.DefaultWeaponData` 선택 참조와 WeaponComp 호환성 검증을 반영했다.
- ProjectileData / DamageData는 아직 미구현이며, WeaponData의 ID 필드로만 선점한다고 정리했다.
- `Tools\BuildEditor.bat` 통과 상태를 반영했다.
- Change Note: 오래된 "P0에서는 WeaponData를 만들지 않는다" 설명을 최신 구현 상태로 교체했다. FireOrigin 우선 검증, Dummy HitScan 유지, Damage 후순위 원칙은 보존했다.

### v0.5

- 사용자의 PIE 검증 완료 상태를 반영했다.
- `무기 > 무기 데이터`에서 WeaponData 지정 여부, WeaponId, 호환성 표시 확인 완료를 기록했다.
- `UCFVehicleWeaponComp` v1.2.0의 WeaponData MaxRange / Cooldown 런타임 연결을 반영했다.
- `ACFVehiclePawn` v2.78.0의 Dummy HitScan Trace 거리와 `WeaponCooldown` 검증 연결을 반영했다.
- `CFVehicleDebugPanelWidget` v1.11.0의 Trace 사거리 / 쿨다운 표시를 반영했다.
- `Tools\BuildEditor.bat` 통과 상태를 반영했다.
- Change Note: WeaponData가 없을 때 기존 Aim Profile 거리와 즉시 발사 흐름을 유지한다는 fallback 조건을 보존했다.

### v0.6

- 빠른 연속 Fire PIE 검증 완료 상태를 반영했다.
- VehicleDebug Panel `무기 > 무기 데이터 > 남은 쿨다운` 감소 확인과 Aim 쪽 마지막 발사 거부 사유 `WeaponCooldown` 확인을 기록했다.
- `UCFProjectileData` 최소 설계 v0.1을 추가했다.
- WeaponData가 ProjectileData를 직접 참조하는 후속 기본안을 결정했다.
- Dummy HitScan 유지 조건과 Projectile 전환 조건을 분리했다.
- ProjectileData가 소유할 값과 DamageData로 넘길 값을 분리했다.
- 후속 코드 작업 후보 파일 / 타입 / 검증 기준을 추가했다.
- Change Note: 기존 WeaponData MaxRange / Cooldown 검증 흐름은 완료 상태로 승격하고, 실제 Projectile Actor 스폰은 사용자 승인 후 작업으로 분리했다.

### v0.7

- `UCFProjectileData` v1.0.0 최소 DataAsset 구현 상태를 반영했다.
- `UCFWeaponData` v1.1.0의 `DefaultProjectileData` 직접 참조 추가를 반영했다.
- `UCFVehicleWeaponComp` v1.3.0의 활성 ProjectileData 캐시 / getter를 반영했다.
- `ACFVehiclePawn` v2.79.0의 VehicleDebug Snapshot ProjectileData 필드 추가를 반영했다.
- `CFVehicleDebugPanelWidget` v1.12.0의 `무기 > 발사체 데이터` 표시를 반영했다.
- Dummy HitScan / FireOrigin / Cooldown 흐름은 유지하고 실제 Projectile Actor 스폰은 아직 미구현으로 분리했다.
- `Tools\BuildEditor.bat` 통과 상태를 반영했다.
- Change Note: v0.6의 코드 작업 후보를 완료 상태로 승격하고, 남은 작업을 에디터 에셋 연결 검증과 Projectile Actor 전환 판단으로 좁혔다.

### v0.8

- 사용자의 ProjectileData PIE 검증 완료 상태를 반영했다.
- 연결 전 `지정 여부=미지정`, `Fallback=Dummy HitScan 유지` 표시 정상 확인을 기록했다.
- 연결 후 `지정 여부=지정`, 발사체 ID와 요약 표시 정상 확인을 기록했다.
- 빠른 연속 Fire 시 기존 남은 쿨다운 감소와 `WeaponCooldown` 거부 사유 유지 확인을 기록했다.
- 다음 우선순위를 Projectile Actor 전환 범위 판단과 DamageData 분리 순서 재검토로 넘겼다.
- Change Note: v0.7의 남은 에디터 검증 항목을 완료 상태로 승격했다. Dummy HitScan fallback 유지 조건과 실제 Projectile Actor 스폰 미구현 경계는 보존했다.

### v0.9

- `ProjectileActorClass` 기반 Projectile 스폰 준비 게이트 구현 상태를 반영했다.
- `UCFProjectileData` v1.1.0, `UCFVehicleWeaponComp` v1.4.0, `ACFVehiclePawn` v2.80.0, `CFVehicleDebugPanelWidget` v1.13.0 상태를 반영했다.
- VehicleDebug Panel `무기 > 발사체 데이터`의 `스폰 준비`, `Fallback`, `전환 요약` 표시 기준을 추가했다.
- 실제 Projectile Actor 스폰은 아직 하지 않고, 준비 상태가 `준비됨`이어도 Dummy HitScan Trace를 유지한다고 명시했다.
- Change Note: v0.8의 ProjectileData 연결 검증 완료 상태 위에 스폰 준비 판정만 추가했다. 쿨다운, FireOrigin, Dummy HitScan 실행 경로는 유지한다.

### v0.10

- 공통 `ACFProjectileActor` v1.0.0 추가 상태를 반영했다.
- `UCFProjectileData` v1.2.0의 `ProjectileStaticMesh`, `ProjectileMeshRelativeRotation`, `ProjectileMeshRelativeScale`, `ACFProjectileActor` 기반 `ProjectileActorClass` 제한을 반영했다.
- `UCFVehicleWeaponComp` v1.5.0의 `FireMode=Projectile` 기반 스폰 준비 판정과 `ReadyToSpawn` 전환 요약을 반영했다.
- `ACFVehiclePawn` v2.81.0의 실제 Projectile Actor 스폰 경로와 스폰 실패 시 Dummy HitScan fallback을 반영했다.
- Pool 대비는 `ACFProjectileActor::ActivateProjectile` / `DeactivateProjectile` 생명주기와 `bDestroyWhenDeactivated` 정책으로 1차 반영하고, 실제 Pool 매니저는 후속 작업으로 분리했다.
- Change Note: v0.9의 "준비 상태만 표시" 단계를 실제 P0 스폰 구현 단계로 승격했다. Damage 적용, Pool 매니저, 터렛 메시 장착은 후순위로 유지한다.

### v0.11

- 사용자의 Projectile Actor PIE 비행 확인 완료 상태를 반영했다.
- `FireMode=Projectile` 설정 뒤 FireOrigin 기준으로 Projectile Actor가 생성되어 날아가는 것을 확인 완료로 승격했다.
- 다음 우선순위를 Projectile Pool 구조 분리, 충돌 / 수명 / 중력 세부 검증, DamageData 분리로 좁혔다.
- Change Note: v0.10의 코드 구현 완료 상태에 사용자 PIE 검증 결과를 추가했다. Damage 적용, Pool 매니저, 터렛 메시 장착은 아직 후순위로 유지한다.

### v0.12

- `UCFProjectilePoolComp` v1.0.0 추가와 `ACFProjectileActor` v1.1.0 Pool 반환 정책을 반영했다.
- `ACFVehiclePawn` v2.82.0의 Projectile Pool Acquire 실행 경로와 Pool 확보 실패 시 Dummy HitScan fallback 유지를 반영했다.
- `UCFVehicleWeaponComp` v1.6.0의 `ReadyToPoolAcquire` 실행 요약과 `CFVehicleDebugPanelWidget` v1.14.0의 `Projectile Pool 준비됨` 표시를 반영했다.
- `Tools\BuildEditor.bat` Pool 전환 후 통과 상태를 기록했다.
- Change Note: v0.11의 Pool Deferred 상태를 Pool 구현 / 빌드 통과 / PIE 재사용 검증 대기로 승격했다. DamageData, Damage 적용, 터렛 메시 장착은 아직 후순위로 유지한다.

### v0.13

- `UCFProjectilePoolComp` v1.1.0의 전체 / 활성 / 비활성 Pool 카운트 getter를 반영했다.
- `ACFVehiclePawn` v2.83.0의 VehicleDebug Weapon Snapshot Pool 카운트 필드를 반영했다.
- `CFVehicleDebugPanelWidget` v1.15.0의 `무기 > 발사체 Pool` 표시를 반영했다.
- `Tools\BuildEditor.bat` Pool 카운트 Debug 표시 추가 후 통과 상태를 기록했다.
- Change Note: v0.12의 Pool 재사용 검증 대기 상태는 유지하되, 다음 PIE 검증 기준을 Actor 수 추정이 아니라 Debug Panel 카운트 변화 확인으로 구체화했다.

### v0.14

- `UCFWeaponData` v1.2.0의 `FireRatePerMinute` 입력과 기존 `CooldownSeconds` 숨김 마이그레이션을 반영했다.
- `UCFVehicleWeaponComp` v1.7.0의 FireRate getter와 `60 / FireRatePerMinute` 발사 간격 검증 연결을 반영했다.
- `ACFVehiclePawn` v2.84.0과 `CFVehicleDebugPanelWidget` v1.16.0의 `무기 > 무기 데이터 > 분당 발사속도` 표시를 반영했다.
- `Tools\BuildEditor.bat` FireRatePerMinute 전환 후 통과 상태를 기록했다.
- Change Note: 사용자에게 노출되는 발사 제한 원본값은 `CooldownSeconds`가 아니라 `FireRatePerMinute`다. 기존 `WeaponCooldown` 거부 사유와 `남은 쿨다운` 실시간 표시는 유지한다.

### v0.15

- 사용자의 FireRatePerMinute 전환 후 정상 동작 확인 상태를 반영했다.
- 사용자의 Projectile Pool 발사 경로 정상 동작 확인 상태를 반영했다.
- `ACFProjectileActor` v1.2.0의 비활성화 사유, 마지막 반환 ProjectileId, 충돌 대상, 비행 시간 Debug 기록을 반영했다.
- `UCFProjectilePoolComp` v1.2.0의 마지막 Projectile 반환 요약 getter를 반영했다.
- `ACFVehiclePawn` v2.85.0과 `CFVehicleDebugPanelWidget` v1.17.0의 VehicleDebug Panel `무기 > 발사체 Pool > 마지막 반환 요약` 표시를 반영했다.
- `Tools\BuildEditor.bat` Projectile Pool 마지막 반환 요약 추가 후 통과 상태를 기록했다.
- Change Note: 마지막 반환 요약은 충돌 / 수명 / Pool 반환 검증용 표시값이다. DamageData와 실제 피해 적용은 아직 실행하지 않는다.

### v0.16

- 사용자의 PIE `무기 > 발사체 Pool > 마지막 반환 요약` 확인 완료 상태를 반영했다.
- Projectile Pool 마지막 반환 요약 검증을 완료 상태로 승격했다.
- 다음 우선순위를 DamageData 최소 설계와 Projectile 충돌 / 수명 / 중력 회귀 기준 정리로 옮겼다.
- Change Note: 마지막 반환 요약은 계속 Debug 표시 전용이다. DamageData 전환 전 `LifeExpired` / `Hit` / `HitActor` / 비행 시간 표시는 회귀 체크 기준으로 유지한다.

### v0.17

- DamageData 작업 전에 터렛 장착 / 회전 / Muzzle 기준을 먼저 닫기로 한 결정을 반영했다.
- `FCFVehicleMountProfile`의 터렛 Yaw / Pitch 메쉬, 상대 Transform, Pitch 피벗 소켓 필드를 문서화했다.
- Pawn의 하드포인트 기반 터렛 시각 장착 구현과 VehicleDebug Panel `무기 > 터렛 시각` 표시를 완료 상태로 반영했다.
- `Tools\BuildEditor.bat` 터렛 시각 장착 추가 후 통과 상태를 기록했다.
- Change Note: 이번 단계는 시각 장착까지이며, 실제 Yaw / Pitch 추적 회전과 Muzzle 소켓 FireOrigin 전환은 후속 작업이다. 기존 Projectile / Cooldown / Dummy HitScan fallback 기준은 유지한다.

### v0.18

- `TurretData` 표현을 `TurretMountData`로 고정하고, 의미를 "무기 거치대 / 회전 플랫폼 / 조준 기계장치"로 정리했다.
- `WeaponData`는 거치대 위에 얹는 실제 발사 장치로 정의하고, 발사속도 / 사거리 / Projectile 참조를 계속 소유한다고 명시했다.
- `MountProfile`은 차량 슬롯의 타입 / 크기 / 차량 구조상 각도 제한과 기본 DataAsset 참조만 소유하는 방향으로 정리했다.
- 유저에게는 내부 분리 구조를 그대로 노출하지 않고, 장기적으로 `EquipmentPreset` 장비 카드 단위로 단순화한다고 기록했다.
- Change Note: 기존 `FCFVehicleMountProfile` inline 터렛 시각 필드는 기존 에셋 보호용 fallback으로 남긴다. 새 구현은 `DefaultTurretMountData`를 우선 사용한다.
- Superseded: v0.34에서 inline 터렛 fallback 런타임 사용은 제거하고, 해당 필드는 직렬화 호환용 legacy 값으로만 보존한다.
- Superseded: v0.37에서 MountProfile 차량 구조상 Min/Max Yaw/Pitch 필드는 C++에서 삭제하고, 필요 시 별도 `HardpointArcConstraint` 후보로 재설계한다.

### v0.19

- `UCFTurretMountData` v1.0.0 최소 DataAsset 구현 상태를 반영했다.
- `FCFVehicleMountProfile.DefaultTurretMountData` v1.3.0 참조 추가와 기존 inline 터렛 시각 필드 fallback 정책을 반영했다.
- Superseded: v0.34에서 inline 터렛 fallback 런타임 사용은 제거하고, 해당 필드는 직렬화 호환용 legacy 값으로만 보존한다.
- `ACFVehiclePawn` v2.87.0의 TurretMountData 우선 터렛 시각 장착과 VehicleDebug Snapshot 확장을 반영했다.
- `CFVehicleDebugPanelWidget` v1.19.0의 `무기 > 터렛 시각` 마운트 데이터 지정 / ID / 요약 표시를 반영했다.
- `Tools\BuildEditor.bat` TurretMountData 최소 구현 후 통과 상태를 기록했다.
- Change Note: 이번 단계는 데이터 원본 분리와 시각 장착 연결까지만 수행한다. 실제 Yaw / Pitch 추적 회전과 Muzzle 소켓 FireOrigin 전환은 후속 작업이다.

### v0.20

- 터렛 시각 계층을 Base / Yaw / Pitch 3단 소켓 구조로 확장했다.
- `TurretBaseMesh`, `TurretBaseRelativeTransform`, `YawPivotSocketName`을 `UCFTurretMountData` 최소 설계 v0.2에 추가했다.
- Base는 고정, YawPivot은 Base의 `YawPivot` 소켓 또는 하드포인트 fallback, PitchPivot은 Yaw 메쉬의 `PitchPivot` 소켓 또는 YawPivot fallback으로 정리했다.
- VehicleDebug Panel `무기 > 터렛 시각`에서 Base / Yaw / Pitch 메쉬 이름을 확인하는 기준을 추가했다.
- `Tools\BuildEditor.bat` Base / Yaw / Pitch 3단 소켓 계층 변경 후 통과 상태를 기록했다.
- Change Note: 기존 `DefaultTurretMountData` 미지정 시 MountProfile inline Yaw / Pitch fallback은 유지한다. 실제 Yaw / Pitch 추적 회전과 Muzzle 소켓 FireOrigin 전환은 아직 후속 작업이다.
- Superseded: v0.34에서 inline 터렛 fallback 런타임 사용은 제거하고, 해당 필드는 직렬화 호환용 legacy 값으로만 보존한다.

### v0.21

- `UCFVehicleWeaponComp` v1.8.0의 터렛 Yaw/Pitch 조준 추적 상태 계산을 반영했다.
- `ACFVehiclePawn` v2.89.0의 `Turret_YawPivot` / `Turret_PitchPivot` 시각 회전 적용을 반영했다.
- `CFVehicleDebugPanelWidget` v1.21.0의 VehicleDebug Panel `무기 > 터렛 조준` 현재/목표 Yaw/Pitch, 안정화 상태 표시를 반영했다.
- `Tools\BuildEditor.bat` 터렛 Yaw/Pitch 시각 추적 회전 추가 후 통과 상태를 기록했다.
- Change Note: 터렛 회전 상태는 WeaponComp가 소유하고 Pawn은 시각 피벗 회전에만 적용한다. Muzzle 소켓 FireOrigin 전환, DamageData, Damage 적용은 아직 후속 작업이다.

### v0.22

- 터렛 시각 장착과 FireOrigin 계산에서 `HardpointSlots.SocketName`을 실제 `SM_Body` 소켓으로 우선 사용하는 정책을 반영했다.
- `ACFVehiclePawn` v2.90.0의 터렛 장착 루트 소켓 우선 부착과 `UCFVehicleWeaponComp` v1.9.0의 FireOrigin 소켓 우선 계산을 반영했다.
- VehicleDebug Panel `무기 > 터렛 시각 > 요약`에서 `HardpointSocketResolved=Yes/No`로 소켓 부착 여부를 확인하는 기준을 추가했다.
- `Tools\BuildEditor.bat` 하드포인트 소켓 우선 장착 수정 후 통과 상태를 기록했다.
- Change Note: 소켓이 없거나 이름이 맞지 않으면 기존 `LocalLocation` / `LocalRotation` fallback을 유지한다. Dummy HitScan / Projectile / Cooldown 검증 경로는 변경하지 않는다.

### v0.23

- `ACFVehiclePawn` v2.91.0의 SnapToTarget 소켓 부착과 `RootDelta` 위치 진단을 반영했다.
- VehicleDebug Panel `무기 > 터렛 시각 > 요약`에서 하드포인트 기대 위치와 터렛 루트 위치 차이를 확인하는 기준을 추가했다.
- `Tools\BuildEditor.bat` SnapToTarget 소켓 부착과 RootDelta 진단 추가 후 통과 상태를 기록했다.
- Change Note: `RootDelta`가 0에 가까우면 하드포인트 부착은 정상으로 보고, 이후 오차는 터렛 메쉬 피벗 / 소켓 / RelativeTransform 조정 문제로 분리한다.

### v0.24

- `ACFVehiclePawn` v2.92.0의 필수 소켓 누락 진단을 반영했다.
- VehicleDebug Panel `무기 > 터렛 시각 > 요약`에서 `SocketValidation=OK/MissingRequiredSocket`, `MissingRequiredSockets=...`로 소켓명 오타와 의도적 fallback을 구분한다.
- Change Note: 소켓 이름이 지정되어 있는데 실제 메쉬에 없으면 기존 fallback은 유지한다. 다만 Debug 요약과 로그에 누락 상태를 남겨 에셋 오타를 즉시 확인한다.
- Build Note: 에디터 종료 후 `Tools\BuildEditor.bat` 재실행 결과 v2.92.0 MissingRequiredSocket 진단 추가 상태로 통과했다.

### v0.25

- `ACFVehiclePawn` v2.93.0의 Muzzle 소켓 FireOrigin 보정과 `UCFVehicleWeaponComp` v1.10.0의 최종 FireOrigin 기록 함수를 반영했다.
- `UCFTurretMountData` v1.2.0 기준으로 `MuzzleSocketName`은 Pitch 메쉬 총구 소켓이며, 소켓 X축이 발사 방향이다.
- VehicleDebug Panel `무기 > 터렛 시각 > 요약`에서 `MuzzleStatus=Resolved/MissingRequiredSocketFallback`을 확인하는 기준을 추가했다.
- Change Note: Muzzle 소켓이 없거나 Pitch 메쉬가 없으면 기존 하드포인트 FireOrigin fallback, Dummy HitScan, Projectile Pool, 쿨다운 검증은 유지한다.
- Build Note: `Tools\BuildEditor.bat` Muzzle 소켓 FireOrigin 보정 추가 후 통과했다.

### v0.26

- 사용자의 PIE 확인 결과를 반영해 Muzzle 소켓 FireOrigin 보정을 정상작동 확인 완료 상태로 승격했다.
- 다음 우선순위를 Muzzle 완료 상태에서 DamageData 전 정책 결정으로 옮겼다.
- Change Note: Muzzle 소켓 X축 발사 방향 기준, 하드포인트 fallback, Dummy HitScan / Projectile Pool / 쿨다운 회귀 기준은 유지한다.

### v0.27

- 터렛 안정화 전 발사 정책을 확정했다. 터렛이 목표각까지 돌아가는 중이어도 필수 거부 조건이 없으면 발사한다.
- 조준각 초과를 기본 발사 거부 사유에서 제외하고, 당시 기준으로 터렛 목표 / 현재 회전값을 MountProfile과 TurretMountData 교집합 제한각 안에 고정한다고 명시했다.
- Superseded: v0.35에서 MountProfile 각도 교집합은 제거되고 TurretMountData 단독 제한으로 갱신됐다.
- `CFVehicleAimTypes` v1.5.0, `CFVehicleAimComp` v1.9.0, `CFVehicleWeaponComp` v1.11.0, `CFVehiclePawn` v2.94.0 코드 작업 후보 / 완료 기준을 추가했다.
- Change Note: 탄퍼짐 / 명중률은 이번 단계에 적용하지 않는다. 기존 Projectile / Dummy HitScan / FireRatePerMinute 쿨다운 검증 기준은 유지한다.
- Build Note: 첫 빌드는 남아 있던 `UnrealEditor.exe`의 DLL 잠금으로 링크 실패했고, 해당 CarFight 에디터 프로세스 종료 후 `Tools\BuildEditor.bat` 재실행은 통과했다.

### v0.28

- DamageData 최소 설계를 확정했다. `UCFDamageData`는 피해 / 관통 / 폭발 / 모듈 피해 배율 / 물리 반응 후보값만 소유한다.
- 수정 용이성을 위해 당시에는 `ProjectileData.DefaultDamageData` 우선, `WeaponData.DefaultDamageData` fallback, 기존 `WeaponData.BaseDamage` 임시 fallback 순서를 채택했다.
- Projectile Actor 충돌과 Dummy HitScan 결과를 같은 `FCFDamageHitContext` 요약으로 기록하는 방향을 확정했다.
- Change Note: 실제 HP 차감, 모듈 손상, 장갑 방향 판정, 폭발 범위 검색, 탄퍼짐 / 명중률, 물리 충격 적용은 후속 단계로 유지한다.
- Superseded: v0.31에서 `WeaponData.DefaultDamageData` fallback은 제거하고 DamageData 직접 참조를 ProjectileData 단일 경로로 정리했다.

### v0.29

- `CFDamageTypes.h` v1.0.0과 `CFDamageData.h/.cpp` v1.0.0 코드 추가 상태를 반영했다.
- `UCFDamageData`와 `FCFDamageHitContext` 타입 추가 후 `Tools\BuildEditor.bat` 통과 상태를 기록했다.
- Change Note: 아직 DamageData 직접 참조와 VehicleDebug Damage 표시, 실제 Damage 적용은 연결하지 않았다.
- Superseded: v0.31 기준 DamageData 직접 참조 슬롯은 `ProjectileData.DefaultDamageData`만 허용한다.

### v0.30

- `UCFProjectileData` v1.3.0과 `UCFWeaponData` v1.3.0의 `DefaultDamageData` 참조 연결 상태를 반영했다.
- `UCFVehicleWeaponComp` v1.12.0의 DamageData 우선순위 해석과 `ACFVehiclePawn` v2.95.0의 VehicleDebug Snapshot 연결을 반영했다.
- `CFVehicleDebugPanelWidget` v1.22.0의 VehicleDebug Panel `무기 > 피해 데이터` 지정 여부 / 피해 ID / 해석 경로 / 요약 표시를 반영했다.
- `Tools\BuildEditor.bat` DamageData 참조 연결과 Debug 표시 추가 후 통과 상태를 기록했다.
- Change Note: 실제 HP 차감, 모듈 손상, 장갑 방향 판정, 마지막 HitContext Debug 기록은 아직 후속 작업이다.
- Superseded: v0.31에서 `WeaponData.DefaultDamageData`와 WeaponData 피해 fallback 해석은 제거했다.

### v0.31

- DamageData 직접 참조 슬롯을 `ProjectileData.DefaultDamageData` 하나로 확정했다.
- HitScan / Laser처럼 실제 Actor를 스폰하지 않는 무기도 가상 ProjectileData를 `WeaponData.DefaultProjectileData`에 연결하는 정책으로 정리했다.
- `UCFWeaponData` v1.4.0에서 `DefaultDamageData` 직접 참조를 제거하고, `BaseDamage` / `DamageProfileId`는 레거시 확인용으로만 유지한다고 명시했다.
- `UCFVehicleWeaponComp` v1.13.0에서 `WeaponData.DefaultDamageData`, `WeaponData.DamageProfileId`, `WeaponData.BaseDamage` 기반 DamageData fallback 해석을 제거했다.
- `ACFVehiclePawn` v2.96.0과 `CFVehicleDebugPanelWidget` v1.23.0의 설명을 ProjectileData 단일 소유 정책에 맞게 정리했다.
- 사용자가 `Tools\BuildEditor.bat` 빌드와 PIE 정상작동을 확인했다.
- Change Note: 실제 HP 차감, 모듈 손상, 장갑 방향 판정, 마지막 HitContext Debug 기록은 아직 후속 작업이다. 이번 변경은 DamageData 소유권과 Debug 해석 경로 정리까지만 포함한다.

### v0.32

- Dummy HitScan 결과와 Projectile Actor 충돌 결과를 같은 `FCFDamageHitContext` 형식으로 기록 / 표시하는 코드 작업 상태를 반영했다.
- `CFDamageTypes.h` v1.1.0의 `bBlockingHit` 추가로 Dummy HitScan miss와 실제 hit을 구분한다고 명시했다.
- `CFProjectileActor` v1.3.0이 마지막 충돌 Actor / 위치 / 노멀 / 입사 방향 / ProjectileData / 발사 주체를 보존한다고 정리했다.
- `CFProjectilePoolComp` v1.3.0이 `Reason=Hit` 반환 시 소유 Pawn으로 HitContext 기록을 전달한다고 정리했다.
- `ACFVehiclePawn` v2.97.0과 `CFVehicleDebugPanelWidget` v1.24.0의 `무기 > 피해 HitContext` Debug 표시 기준을 추가했다.
- Change Note: 이번 변경은 Debug 기록 / 표시까지만 포함한다. 실제 HP 차감, 모듈 손상, 장갑 방향 판정, 폭발 범위 검색은 아직 후속 Damage Runtime 작업이다.

### v0.33

- Damage HitContext Debug 추가 후 사용자가 `Tools\BuildEditor.bat` 빌드와 PIE 정상작동을 확인한 상태를 반영했다.
- `CFDamageTypes.h` v1.1.0, `CFProjectileActor` v1.3.0, `CFProjectilePoolComp` v1.3.0, `ACFVehiclePawn` v2.97.0, `CFVehicleDebugPanelWidget` v1.24.0을 완료 상태로 승격했다.
- 다음 우선순위를 Damage HitContext Debug 검증에서 실제 Damage Runtime 작업 범위 결정으로 이동했다.
- Change Note: Damage HitContext는 계속 Debug 표시 전용이다. 실제 HP 차감, 모듈 손상, 장갑 방향 판정, 폭발 범위 검색은 아직 구현하지 않는다.

### v0.41

- 사용자가 `MountProfile.DefaultTurretMountData` / `DefaultWeaponData` legacy 직접 슬롯을 비운 상태로 PIE 정상작동을 확인한 뒤, C++에서 두 슬롯을 삭제한 상태를 반영했다.
- `FCFVehicleMountProfile`의 장비 기본값은 `DefaultEquipmentPresetData` 하나로 고정하고, TurretMountData / WeaponData는 EquipmentPresetData 내부 참조만 해석한다고 정리했다.
- `CFVehicleWeaponTypes.h` v1.8.0, `CFVehicleWeaponComp.h/.cpp` v1.17.0, `CFVehiclePawn.h/.cpp` v2.101.0 / v2.100.0, `CFEquipmentPresetData.h/.cpp` v1.1.0, `CFTurretMountData.h/.cpp` v1.4.0, `CFWeaponData.h` v1.5.0 상태를 반영했다.
- Build Note: `Tools\BuildEditor.bat`에서 UHT / 컴파일은 통과했지만, 실행 중인 `UnrealEditor.exe`가 DLL을 잠가 링크 단계가 실패했다. 에디터 종료 후 같은 스크립트 재실행이 필요하다.
- Change Note: 이번 단계는 legacy 직접 fallback 제거다. 남은 MountProfile inline 터렛 필드는 기존 DataAsset 직렬화 호환용으로만 보존한다.

### v0.40

- `UCFEquipmentPresetData` v1.0.0 최소 DataAsset 타입과 `FCFVehicleMountProfile.DefaultEquipmentPresetData` 참조 추가 상태를 반영했다.
- `UCFVehicleWeaponComp` v1.16.0 기준 해석 우선순위를 `EquipmentPresetData` 우선, `DefaultTurretMountData` / `DefaultWeaponData` legacy fallback으로 정리했다.
- `ACFVehiclePawn`과 VehicleDebug Panel에 EquipmentPresetData 지정 여부 / ID / 호환성 / 요약 표시를 추가한 상태를 반영했다.
- Build Note: `Tools\BuildEditor.bat`가 EquipmentPresetData 최소 코드 반영 후 통과했다.
- 다음 우선순위를 `DA_TestSedan` / `DA_TestSUV`용 EquipmentPresetData 에셋 생성, MountProfile 연결, legacy fallback 회귀 PIE 확인으로 이동했다.
- Change Note: 이번 단계는 장비 조합의 소유권을 코드에 반영하지만, 기존 직접 참조 필드는 삭제하지 않는다. 삭제는 빌드, PIE, DataAsset 재저장 확인 뒤 별도 판단한다.

### v0.39

- `WeaponData`가 `TurretMountData`를 직접 소유하지 않고, 새 `EquipmentPresetData`가 둘의 유효 조합을 소유하는 방향으로 확정했다.
- `FCFVehicleMountProfile`의 다음 목표 필드를 `DefaultEquipmentPresetData`로 정리하고, 현재 `DefaultTurretMountData` / `DefaultWeaponData` 직접 연결은 legacy fallback으로 잠시 보존한다고 명시했다.
- `UCFEquipmentPresetData` 최소 설계 v0.1을 추가했다.
- Change Note: 이번 변경은 장비 제작 단위의 이질감을 해소하기 위한 소유권 결정 문서화다. 코드와 에셋은 아직 변경하지 않았다.

### v0.38

- 사용자의 PIE 확인 결과를 반영해 MountProfile legacy Min/Max Yaw/Pitch 필드 실제 삭제를 빌드 / PIE 정상작동 확인 완료 상태로 승격했다.
- 차량 DA 로드, MountProfiles 상세, TurretMountData 단독 제한, 기존 발사 / Projectile / 쿨다운 회귀가 정상 동작하는 상태로 기록했다.
- 다음 우선순위를 남은 MountProfile inline legacy 시각 / 기계 필드 삭제 범위 결정으로 이동했다.
- Change Note: 이번 단계는 MountProfile 각도 필드 삭제 검증 완료 기록이다. 남은 legacy 시각 / 기계 필드는 별도 안전 삭제 단계에서 판단한다.

### v0.37

- 사용자가 `DA_TestSedan`, `DA_TestSUV` 등 관련 DataAsset을 저장한 뒤, `FCFVehicleMountProfile`의 legacy `MinYawDeg`, `MaxYawDeg`, `MinPitchDeg`, `MaxPitchDeg` 필드를 C++에서 실제 삭제했다.
- `CFVehicleWeaponTypes.h`를 v1.6.0으로 갱신하고, `Tools\BuildEditor.bat` 통과 상태를 기록했다.
- `bExposedModule`, `DefaultTurretMountData`, `DefaultWeaponData`는 유지한다.
- 다음 검증은 PIE에서 차량 DA 로드 경고 없음, MountProfiles 상세 정상 표시, TurretMountData 단독 제한, 기존 발사 / Projectile / 쿨다운 회귀 확인이다.
- Change Note: 차량 구조상 각도 제한이 다시 필요해지면 삭제한 MountProfile 필드를 되살리지 않고 별도 `HardpointArcConstraint` 계열 데이터로 설계한다.

### v0.36

- 사용자의 PIE 확인 결과를 반영해 MountProfile 각도 제한 legacy 전환을 정상동작 확인 완료 상태로 승격했다.
- 차량 DA MountProfiles에서 `MinYawDeg`, `MaxYawDeg`, `MinPitchDeg`, `MaxPitchDeg` 미노출, `bExposedModule` 유지, TurretMountData 단독 제한 기준 동작을 완료 상태로 기록했다.
- 당시 다음 우선순위를 MountProfile 각도 제한 PIE 검증에서 DataAsset 재저장 확인으로 이동했다.
- Superseded: v0.37에서 DataAsset 저장 확인 후 legacy Min/Max Yaw/Pitch 필드 삭제를 완료했다.
- Change Note: 이번 단계는 런타임 / 에디터 노출 제거 검증 완료 기록이다. 실제 C++ 필드 삭제는 DataAsset 재저장 확인 뒤 별도 단계로 진행한다.

### v0.35

- MountProfile의 `MinYawDeg`, `MaxYawDeg`, `MinPitchDeg`, `MaxPitchDeg`를 P0 런타임 터렛 제한에서 제거하고 legacy 직렬화 호환값으로 전환하기로 정리했다.
- 터렛 목표 / 현재 Yaw/Pitch 제한은 `UCFTurretMountData`의 Min/Max Yaw/Pitch만 사용하며, MountProfile과의 교집합 계산은 더 이상 사용하지 않는다.
- `bExposedModule`은 후속 Damage / Module 후보 판단을 위해 MountProfile 편집 노출 상태로 유지한다.
- VehicleDebug Panel `무기 > 터렛 조준 > 요약`에서 `Source=TurretMountData`, `LimitCorrection=No`를 확인하는 검증 기준을 추가했다.
- Change Note: 차량 구조상 회전 제한이 다시 필요해지면 MountProfile inline 값 재사용이 아니라 별도 `HardpointArcConstraint` 계열 데이터로 재도입한다.
- Build Note: `Tools\BuildEditor.bat` MountProfile 각도 제한 legacy 전환 후 통과했다.

### v0.34

- Damage Runtime 전에 MountProfile inline 터렛 fallback 제거 단계를 먼저 진행하기로 한 사용자의 결정을 반영했다.
- `FCFVehicleMountProfile`의 inline 터렛 시각 / 회전 기계값 필드는 기존 DataAsset 로드 안정성을 위해 직렬화 호환용으로 잠시 보존하되, 에디터 편집 노출과 런타임 사용을 중단한다고 정리했다.
- `DefaultTurretMountData`가 비어 있으면 터렛 시각 장착과 터렛 조준 추적은 Missing 계열 Debug로 건너뛰고, FireOrigin / Projectile / FireRatePerMinute 쿨다운 / Damage HitContext Debug는 유지한다고 명시했다.
- Change Note: 이번 단계는 삭제 전 안전 차단 단계다. 실제 필드 삭제는 `Tools\BuildEditor.bat`, PIE 회귀 확인, 기존 DataAsset 재저장 확인 뒤 별도 단계로 진행한다.

---

## 18. Migration 메모

- 기존 `HardpointSlots`는 유지한다.
- `LocationSlotId`, `LocationCategory`, `SocketName`, `LocalLocation`, `LocalRotation` 의미는 바꾸지 않는다.
- 기존 Fire 함수 시그니처는 P0 1차에서 유지하고, 내부에서 WeaponComp FireOrigin을 우선 사용한다.
- 기존 AimComp의 `DefaultAimProfile`은 fallback으로 유지한다.
- 기존 Reticle / Debug는 새 상태를 읽도록 확장하되, 판단 로직을 소유하지 않는다.
- VehicleDebug Panel의 `무기` 섹션은 C++ 계산 결과를 표시만 하며, 발사 가능 여부를 새로 판정하지 않는다.
- EquipmentPresetData 내부 `DefaultWeaponData`가 비어 있어도 기존 FireOrigin 계산은 유지된다.
- 신규 WeaponData 에셋은 먼저 EquipmentPresetData 내부 `DefaultWeaponData`에 수동 연결해 패널 호환성 표시를 확인한다.
- WeaponData가 유효하고 호환되면 `MaxRange`는 Dummy HitScan Trace 거리로 사용한다.
- WeaponData가 유효하고 호환되면 `FireRatePerMinute`는 `60 / FireRatePerMinute` 발사 간격으로 환산되어 로컬 Fire 검증의 `WeaponCooldown` 거부 사유에 사용된다.
- 기존 에셋에 저장된 `CooldownSeconds` 값은 로드 시 `FireRatePerMinute`로 환산하고, 새 입력 / 표시 기준은 `분당 발사속도`로 통일한다.
- 현재 PIE 기준 빠른 연속 Fire 쿨다운 감소와 `WeaponCooldown` 거부 사유 확인은 완료 상태로 본다.
- 직접 참조가 안정화될 때까지 `ProjectileDataId`는 기존 WeaponData 에셋 호환을 위해 유지한다.
- `DefaultProjectileData` 직접 참조를 추가하더라도 ProjectileData가 비어 있으면 Dummy HitScan fallback을 유지한다.
- `DefaultProjectileData`가 연결되면 VehicleDebug Panel `무기 > 발사체 데이터`에서 지정 여부, 발사체 ID, 요약을 먼저 확인한다.
- 2026-06-30 기준 위 ProjectileData 연결 전/후 패널 표시와 쿨다운 유지 검증은 완료됐다.
- `ProjectileActorClass`가 비어 있거나 `FireMode=HitScan`이면 VehicleDebug Panel은 `스폰 준비=미준비`, `Fallback=Dummy HitScan 유지`로 표시한다.
- `ProjectileActorClass`가 지정되고 `FireMode=Projectile`이면 VehicleDebug Panel은 `스폰 준비=준비됨`, `Fallback=Projectile Pool 준비됨`, `전환 요약=ReadyToPoolAcquire`로 표시한다.
- 실제 Projectile Actor 확보 / 활성화는 Fire 검증 통과 후 `ACFVehiclePawn::TrySpawnProjectileActorFromFireCommand`에서 `UCFProjectilePoolComp`를 통해 실행한다.
- Projectile Actor는 Pool 소유자가 지정된 경우 충돌 또는 수명 종료 시 Pool로 반환된다. Pool 소유자가 없으면 기존 Destroy 정책을 유지한다.
- Pool 한도 초과, 클래스 오류, 월드 없음, 방향 오류 등으로 Pool 확보에 실패하면 같은 Fire 입력은 Dummy HitScan fallback으로 처리한다.
- 2026-07-01 기준 PIE에서 Projectile Actor가 FireOrigin 기준으로 날아가는 것은 확인 완료됐다.
- 2026-07-01 기준 Projectile Pool Debug 표시 추가 후 `Tools\BuildEditor.bat`는 통과했다. 사용자는 Pool 발사 경로 정상 동작을 확인했다.
- 2026-07-01 기준 Projectile Pool 마지막 반환 요약 추가 후 `Tools\BuildEditor.bat`는 통과했다.
- 2026-07-01 기준 사용자는 PIE에서 `무기 > 발사체 Pool > 마지막 반환 요약` 표시를 확인 완료했다.
- DamageData 전환 전에는 `무기 > 발사체 Pool > 마지막 반환 요약`의 `LifeExpired` / `Hit` / `HitActor` / 비행 시간 표시를 회귀 체크 기준으로 유지한다.
- Projectile Pool 마지막 반환 요약은 Debug 표시 전용이다. 이 값으로 Damage 적용, 발사 가능 여부, Pool 확보 여부를 새로 판정하지 않는다.
- `FCFVehicleMountProfile`의 `TurretYawMesh`, `TurretPitchMesh`, `TurretYawRelativeTransform`, `PitchPivotSocketName`, `TurretPitchRelativeTransform`, 회전 속도 / 안정화 / 중량 legacy 필드는 기존 DataAsset 직렬화 호환용으로만 잠시 보존한다.
- `FCFVehicleMountProfile`의 inline 터렛 필드는 새 에셋 작성 / 에디터 편집 / 런타임 fallback 경로로 사용하지 않는다. 새 에셋은 EquipmentPresetData 내부 `DefaultTurretMountData`에 연결한다.
- 터렛 마운트 데이터가 비어 있는 기존 DataAsset은 터렛 시각 / 조준 추적만 Missing으로 표시하며 FireOrigin / Projectile / Cooldown / Dummy HitScan fallback은 그대로 유지한다.
- `UCFTurretMountData.TurretBaseMesh`는 선택값이다. 비어 있으면 YawPivot은 기존 하드포인트 루트 기준으로 붙는다.
- P0 터렛 Yaw 피벗은 Base 메쉬의 `YawPivotSocketName` 소켓을 우선 사용한다. 소켓이 없거나 이름이 다르면 YawPivot은 Base 메쉬 또는 하드포인트 루트 원점으로 fallback된다.
- P0 터렛 Pitch 피벗은 Yaw 메쉬의 `PitchPivotSocketName` 소켓을 우선 사용한다. 소켓이 없거나 이름이 다르면 PitchPivot은 Yaw 메쉬 또는 YawPivot 원점으로 fallback된다.
- 이름이 지정된 하드포인트 / YawPivot / PitchPivot / Muzzle 소켓이 실제 메쉬에 없으면 기존 fallback은 유지하지만, VehicleDebug Panel `무기 > 터렛 시각 > 요약`에 `SocketValidation=MissingRequiredSocket`과 `MissingRequiredSockets=...`가 표시된다.
- `UCFTurretMountData.MuzzleSocketName`은 Pitch 메쉬에서 최종 FireOrigin 위치와 방향을 찾는 총구 소켓이다. 소켓 X축이 발사 방향이어야 한다.
- Muzzle 소켓이 유효하면 `ACFVehiclePawn`은 `BuildFireCommand` 단계에서 WeaponComp 하드포인트 FireOrigin을 Muzzle 소켓 기준으로 보정하고, WeaponComp의 마지막 FireOrigin도 최종값으로 갱신한다.
- Muzzle 소켓이 없거나 Pitch 메쉬가 없으면 기존 하드포인트 FireOrigin fallback을 유지한다.
- 2026-07-01 기준 터렛 시각 장착 추가 후 `Tools\BuildEditor.bat`는 통과했다.
- 2026-07-01 기준 `UCFTurretMountData`와 `DefaultTurretMountData` 연결 추가 후 `Tools\BuildEditor.bat`는 통과했다.
- 2026-07-01 기준 Base / Yaw / Pitch 3단 소켓 계층 변경 후 `Tools\BuildEditor.bat`는 통과했다.
- 2026-07-01 기준 터렛 Yaw/Pitch 시각 추적 회전 추가 후 `Tools\BuildEditor.bat`는 통과했다.
- 2026-07-01 기준 하드포인트 소켓 우선 장착 수정 후 `Tools\BuildEditor.bat`는 통과했다.
- 2026-07-01 기준 SnapToTarget 소켓 부착과 RootDelta 진단 추가 후 `Tools\BuildEditor.bat`는 통과했다.
- 2026-07-02 기준 MissingRequiredSocket 진단 추가 후 에디터 종료 상태에서 `Tools\BuildEditor.bat`는 통과했다.
- 2026-07-02 기준 Muzzle 소켓 FireOrigin 보정 추가 후 `Tools\BuildEditor.bat`는 통과했다.
- 2026-07-02 기준 사용자는 PIE에서 Muzzle 소켓 FireOrigin 발사가 정상작동함을 확인했다.
- 터렛 회전 상태는 `UCFVehicleWeaponComp`가 소유하고, `ACFVehiclePawn`은 계산된 각도를 `Turret_YawPivot` / `Turret_PitchPivot` 시각 회전에 적용만 한다.
- 터렛 안정화 전에도 필수 거부 조건이 없으면 발사한다. `OutOfWeaponArc`는 호환용 enum으로 유지하지만 P0 기본 로컬 Fire 검증의 단독 거부 사유로 사용하지 않는다.
- 터렛 목표 / 현재 Yaw/Pitch는 TurretMountData의 유효 제한각 안에 고정한다.
- 탄퍼짐 / 명중률은 현재 적용하지 않으며, 후속 사격 시스템에서 별도 설계한다.
- 2026-07-02 기준 터렛 안정화 전 발사 정책 코드 반영 후 `Tools\BuildEditor.bat`는 통과했다.
- DamageData 최소 소유값, HitContext Debug 타입, `DefaultDamageData` 참조 연결, VehicleDebug `무기 > 피해 데이터` 표시는 구현됐다.
- DamageData 연결 단계에서는 기존 FireOrigin / Projectile / Dummy HitScan / FireRatePerMinute 쿨다운 검증을 깨지 않아야 하며, 이번 구현도 발사 가능 여부를 바꾸지 않는다.
- 2026-07-02 기준 `UCFDamageData` / `FCFDamageHitContext` 타입 추가 후 `Tools\BuildEditor.bat`는 통과했다.
- 2026-07-02 기준 `ProjectileData.DefaultDamageData` / `WeaponData.DefaultDamageData` 참조 연결과 VehicleDebug Damage 표시 추가 후 `Tools\BuildEditor.bat`는 통과했다. 이후 v0.31에서 `WeaponData.DefaultDamageData`는 제거 대상으로 전환했다.
- 2026-07-02 기준 `WeaponData.DefaultDamageData` 제거와 `ProjectileData.DefaultDamageData` 단일 소유 전환 후 사용자가 빌드와 PIE 정상작동을 확인했다.
- Dummy HitScan과 Projectile Actor 충돌은 같은 `FCFDamageHitContext` 구조체로 기록한다.
- Dummy HitScan miss는 `bBlockingHit=false`이고, Projectile Actor 충돌은 `bBlockingHit=true`로 기록한다.
- Projectile Actor가 `LifeExpired`로 Pool에 반환될 때는 Damage HitContext를 새로 만들지 않는다.
- Damage HitContext Debug는 VehicleDebug Panel 표시 전용이며 실제 Damage 적용, 발사 가능 여부, Pool 확보 여부를 새로 판정하지 않는다.
- 2026-07-02 기준 Damage HitContext Debug 추가 후 사용자가 `Tools\BuildEditor.bat` 빌드와 PIE 정상작동을 확인했다.
- 유저-facing 장비 선택 UI는 장기적으로 `EquipmentPreset` 단위로 만들고, MountType / TurretMountData / WeaponData 같은 내부 용어를 직접 선택지로 노출하지 않는다.
- `UCFEquipmentPresetData`는 유저-facing 장비 패키지이자 에셋 제작 단위로 사용한다.
- `FCFVehicleMountProfile.DefaultEquipmentPresetData`는 TurretMountData / WeaponData의 단일 해석 진입점이다.
- `DefaultEquipmentPresetData`가 비어 있으면 기존 직접 참조 fallback 없이 EquipmentPreset / TurretMountData / WeaponData가 Missing 상태로 표시된다.
- `DefaultEquipmentPresetData` 내부의 TurretMountData 또는 WeaponData 중 한쪽이 비어 있으면 그 한쪽만 Missing 상태로 표시하고 직접 참조 fallback은 사용하지 않는다.
- VehicleDebug Panel `무기 > 장비 프리셋`은 표시 전용이며 발사 가능 여부를 새로 판정하지 않는다.
- `DefaultTurretMountData` / `DefaultWeaponData` 직접 연결 필드는 사용자가 비운 상태로 PIE 정상작동을 확인했으므로 C++에서 삭제했다.
- `Top_01` 하드포인트가 있는 기존 DataAsset은 PostLoad에서 `RoofTurret_MediumOrLarge` 프로파일을 1회 보강한다.
- `DA_PoliceCar`는 현재 작업 기준으로 되살리지 않는다.
