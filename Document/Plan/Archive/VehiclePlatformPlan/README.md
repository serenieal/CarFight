# CarFight - VehiclePlatformPlan

> 역할: 전투기능이 붙을 수 있는 최소 차량 플랫폼 규격과 작업 계획을 관리한다.
> 문서 버전: v0.67
> 마지막 정리(Asia/Seoul): 2026-07-03
> 상태: Draft / Single Player Planning / EquipmentPreset Legacy Slots Removed

---

## 1. 목적

`VehiclePlatformPlan`은 차량 종류 확장 자체가 아니라, 전투기능을 붙일 수 있는 최소 차량 플랫폼 규격을 정리하는 작업 문서 묶음이다.

이 폴더의 목표는 아래 질문에 답하는 것이다.

1. P0 전투기능을 검증하려면 차량이 최소한 어떤 데이터를 가져야 하는가?
2. 무기와 하드포인트는 차량 플랫폼 안에서 어떤 방식으로 연결되어야 하는가?
3. 조준, 사격, 피해, 모듈 손상, 중량, UI는 차량 플랫폼의 어떤 정보를 참조해야 하는가?
4. 차량 종류 확장은 어느 조건을 만족한 뒤 진행해야 하는가?

---

## 2. 문서 경계

기획 원본과 작업 계획을 분리한다.

| 경로 | 역할 | 작성 기준 |
|---|---|---|
| `Document/ProjectSSOT/CombatPlan/` | 전투 기획 SSOT | 확정된 기획 원칙과 결정만 저장 |
| `Document/Plan/VehiclePlatformPlan/` | 작업 계획 / 구현 준비 | 현재 세션의 실행 순서, 규격 초안, 체크리스트 저장 |

중요 원칙:

```text
ProjectSSOT는 기준 문서다.
Plan은 작업 문서다.
작업 중 나온 판단은 먼저 Plan에 기록하고,
확정이 필요할 때만 ProjectSSOT 갱신 후보로 올린다.
```

---

## 3. 상위 기준

이 폴더는 아래 CombatPlan SSOT를 따른다.

| 기준 문서 | 반영 기준 |
|---|---|
| `04_Positioning.md` | 경량 / 중형 / 중량 차량의 전투 역할 |
| `05_AimingSystem.md` | 플레이어 조준점, 무기 실제 조준점, 조준 방식 |
| `06_WeaponTypes.md` | P0 무기 4종과 대응 장비 |
| `08_DefenseArmor.md` | 장갑 방향, 차체 HP, 모듈 손상 |
| `12_Hardpoints.md` | P0 하드포인트 5종 |
| `13_Fitting.md` | 차량은 클래스가 아니라 플랫폼이라는 원칙 |
| `14_CombatUI.md` | 조준 / 자원 / 손상 UI가 참조할 정보 |
| `15_CombatAI.md` | P0 AI 역할 4종 |
| `16_P0Prototype.md` | P0는 차량 1~2종과 피팅 차이로 검증 |
| `18_Balancing.md` | 수치보다 역할 차이 우선 |

---

## 4. 문서 구성

| 문서 | 역할 |
|---|---|
| `README.md` | 이 폴더의 목적과 문서 경계 |
| `CF_MinVehiclePlatform.md` | 최소 차량 플랫폼 규격 |
| `CF_PlatformBaseline.md` | 현재 차량 / 조준 / UI / 디버그 기준선 |
| `CF_EditorAssetAudit.md` | 기준 BP / DataAsset / Reticle / Mesh 자산 감사 |
| `CF_AssetDumpResult.md` | AssetDump 기반 자동 자산 확인 결과 |
| `CF_MeshSocketAudit.md` | P0 후보 StaticMesh Socket 감사 결과 |
| `CF_EditorCheckGuide.md` | 사용자가 에디터에서 직접 확인할 단계별 체크 가이드 |
| `CF_CombatContract.md` | 전투기능이 차량 플랫폼에서 읽어야 할 계약 |
| `CF_CombatDataOwnership.md` | 차량 / 장착 프로파일 / 터렛 / 무기 / 탄환 / Damage / UI 상태 데이터 소유권 |
| `CF_HardpointMatrix.md` | P0 하드포인트별 Aim / Fire / UI 연결 기준 |
| `CF_HardpointAuthoring.md` | 하드포인트 위치 작성 방식 비교와 P0 선택안 |
| `CF_CodeStartGate.md` | 코드 작업 착수 전 확인 게이트 |
| `CF_PlatformTasks.md` | 구현 전 작업 순서 |
| `CF_PlatformChecklist.md` | 검증 체크리스트 |
| `CF_VehicleWheelSocketPlan_v0_1.md` | 차체 메시 소켓에서 차량별 휠 / 하드포인트 레이아웃을 통합 캡처하는 작업 계획 |
| `CF_SocketCaptureCodePlan.md` | 통합 캡처 버튼 코드 작업 계획 |

---

## 5. 추천 읽기 순서

처음 읽는 AI 세션은 아래 순서로 읽는다.

```text
01. README.md
02. CF_MinVehiclePlatform.md
03. CF_PlatformBaseline.md
04. CF_EditorAssetAudit.md
05. CF_AssetDumpResult.md
06. CF_MeshSocketAudit.md
07. CF_EditorCheckGuide.md
08. CF_CombatContract.md
09. CF_CombatDataOwnership.md
10. CF_HardpointMatrix.md
11. CF_HardpointAuthoring.md
12. CF_CodeStartGate.md
13. CF_PlatformTasks.md
14. CF_PlatformChecklist.md
15. CF_VehicleWheelSocketPlan_v0_1.md
16. CF_SocketCaptureCodePlan.md
```

이 순서를 권장하는 이유:

| 순서 | 이유 |
|---|---|
| 목표 확인 | 세션 목적이 차량 증식이 아니라 플랫폼 규격 정의임을 고정 |
| 기준선 확인 | 현재 코드가 이미 가진 것과 없는 것을 분리 |
| 자동 자산 확인 | AssetDump로 확인된 BP / DataAsset / Reticle 근거와 P0 메시 Socket 감사 결과를 분리 |
| 계약 확인 | Aim / Weapon / Damage / UI / AI가 읽을 정보를 분리 |
| 데이터 소유권 확인 | 차량 / 장착 프로파일 / 터렛 / 무기 / 탄환 / Damage / UI 상태를 어디에 둘지 분리 |
| 하드포인트 작성 방식 확인 | 4번 방식, Socket / BP Preview 배치 후 바퀴 위치 캡처 버튼에 통합된 DataAsset 캡처로 선택 완료 |
| 착수 게이트 확인 | 코드 작업 전에 남은 확인 사항을 분리 |
| 작업 순서 확인 | 실제 구현 준비 순서를 확정 |
| 코드 작업 계획 확인 | 통합 캡처 버튼 구현 대상 파일과 실패 기준을 확인 |

---

## 6. 핵심 방향

현재 세션의 목적은 차량 종류를 늘리는 것이 아니다.

목적은 아래 문장으로 고정한다.

> 전투기능이 붙을 수 있는 최소 차량 플랫폼 규격을 먼저 정하고, 그 규격 위에서 P0 전투기능을 검증한 뒤 차량 종류를 확장한다.

따라서 P0에서는 차량 종류를 많이 만들지 않는다.

권장 흐름:

```text
01. 최소 차량 플랫폼 규격 정의
02. P0 테스트 차량 1~2종 선정
03. P0 하드포인트 규격 정의
04. 조준 / 사격 / 피해 / 모듈 / 중량 / UI 연동
05. P0 전투기능 검증
06. 검증된 역할만 기준으로 차량 종류 확장
```

`DA_PoliceCar`는 더 이상 현재 작업에 사용하지 않는다.
P0 기본 차형 후보는 세단(Sedan)과 SUV로 둔다.
P0 후보 DataAsset은 기본 테스트 기준 `DA_TestSedan`과 대조 테스트 후보 `DA_TestSUV`이며, 메시 후보는 `Mesh_TestSedan`, `Mesh_TestSUV`다.
현재 `DA_TestSedan`, `DA_TestSUV`는 WheelClass / Movement / WheelVisual / DriveState / Layout Override와 하드포인트 `HardpointSlots`가 저장되어 있다.
현재 `BP_CFVehiclePawn.VehicleData` 기본 연결은 `DA_TestSedan`으로 저장 완료됐다.
`DA_TestSedan` 기준 기본 주행 검증은 완료됐다.
`DA_TestSUV` 대조 테스트는 별도 자동화 없이 사용자가 수동으로 VehicleData를 전환해 확인한다.
주행감은 현재 충분히 나눠진 상태로 보고, 당장 중요한 작업에서 제외한다.
다음 우선순위는 legacy 직접 슬롯 제거 후 `EquipmentPresetData` 전용 경로 재빌드와 PIE 회귀 확인이다.
전투 데이터 소유권은 `CF_CombatDataOwnership.md`를 기준으로 하며, `MountProfile + UCFVehicleWeaponComp + FireOrigin` 기본 연결은 구현되어 기존 빌드와 사용자 테스트를 통과했다. 최신 legacy 직접 슬롯 삭제 변경은 UHT / 컴파일은 통과했지만 실행 중인 에디터 DLL 잠금으로 링크 재시도가 필요하다.
P0 현재 기준으로 최소 `UCFWeaponData` 타입, 최소 `UCFProjectileData` 타입, 최소 `UCFTurretMountData` 타입, 최소 `UCFDamageData` 타입, 최소 `UCFEquipmentPresetData` 타입, `FCFDamageHitContext` 타입, 공통 `ACFProjectileActor` 타입, `UCFProjectilePoolComp` 타입, `MountProfile.DefaultEquipmentPresetData` 전용 해석, `MountProfile.DefaultWeaponData` / `DefaultTurretMountData` legacy 직접 슬롯 삭제, WeaponData `DefaultProjectileData` 참조, ProjectileData `DefaultDamageData` 단일 DamageData 참조, HitScan / Laser용 가상 ProjectileData 정책, WeaponData `MaxRange` / `FireRatePerMinute` Fire 검증 연결, `MountProfile` legacy inline 터렛 필드 직렬화 호환 보존 / 런타임 fallback 제거, MountProfile 각도 제한 legacy 전환 빌드 / PIE 검증, MountProfile legacy Min/Max Yaw/Pitch 필드 삭제 빌드 / PIE 검증, `TurretMountData` Base / Yaw / Pitch 3단 소켓 시각 계층, TurretMountData 단독 터렛 Yaw / Pitch 제한, 터렛 Yaw / Pitch 시각 추적 회전, Muzzle 소켓 FireOrigin 보정, 터렛 안정화 전 발사 허용 정책, DamageData 최소 설계, Damage HitContext Debug 기록 / 표시가 추가됐다.
현재 결정은 `TurretMountData = 무기 거치대 / 회전 플랫폼`, `WeaponData = 거치대 위 실제 발사 장치`, `EquipmentPresetData = 둘을 묶는 완성 장비 패키지`로 고정한다.
`WeaponData`가 `TurretMountData`를 직접 소유하지 않는다. 대신 `EquipmentPresetData`가 `HeavyCannonMount + HeavyCannonWeapon`처럼 유효한 조합을 소유하고, 차량 `MountProfile`은 최종적으로 `DefaultEquipmentPresetData` 하나를 기본 장비로 참조한다.
현재 `MountProfile.DefaultTurretMountData` / `DefaultWeaponData` 직접 연결은 사용자가 비운 상태로 PIE 정상작동을 확인했으므로 C++에서 삭제했다. 장비 기본값은 `DefaultEquipmentPresetData` 하나로 고정한다.
VehicleDebug Panel에는 `무기(Weapon)` Navigation 섹션이 추가되어 WeaponComp 런타임 준비, 활성 프로파일, EquipmentPresetData 지정 / ID / 호환성 / 요약, WeaponData 지정 / 호환성, ProjectileData 지정 / ID / 스폰 준비 / fallback, DamageData 지정 / 피해 ID / 해석 경로 / 요약, 마지막 Damage HitContext 기록 여부 / Source / Hit / DamageId / WeaponId / ProjectileId / 피격 Actor / 위치 / 방향 / 요약, Projectile Pool 컴포넌트 / 전체 / 활성 / 비활성 수 / 마지막 반환 요약, 터렛 시각 장착 여부 / Base 메쉬 / Yaw 메쉬 / Pitch 메쉬 / `MuzzleStatus` / 요약, 터렛 조준 현재/목표 Yaw/Pitch와 안정화 상태, Trace 사거리, 분당 발사속도, 남은 쿨다운, 마지막 FireOrigin 위치 / 방향을 확인할 수 있다.
사용자가 PIE에서 `무기 > 무기 데이터`의 지정 여부 / WeaponId / 호환성 표시 확인을 완료했다.
사용자가 PIE에서 빠르게 연속 발사했을 때 `무기 > 무기 데이터 > 남은 쿨다운` 감소와 Aim 쪽 마지막 발사 거부 사유 `WeaponCooldown` 표시를 확인 완료했다.
사용자가 PIE에서 ProjectileData 연결 전 `미지정 / Dummy HitScan 유지`, 연결 후 `지정 / 발사체 ID / 요약 표시`, 빠른 연속 Fire 쿨다운 유지까지 모두 정상 동작 확인을 완료했다.
사용자가 PIE에서 `FireRatePerMinute` 전환 후에도 빠른 연속 발사, 남은 쿨다운 감소, `WeaponCooldown` 거부 사유가 정상 동작함을 확인했다.
현재 ProjectileData 단계는 최소 코드 구현, `ProjectileActorClass` 기반 스폰 준비 판정, 실제 Projectile Actor Pool Acquire 경로, Pool 카운트 / 마지막 반환 요약 Debug 표시 구현 완료, PIE 확인 완료, 빌드 통과 상태다.
ProjectileData가 비어 있거나 `FireMode=HitScan`이면 Dummy HitScan fallback, FireOrigin, 발사 간격 검증은 유지된다.
`ProjectileActorClass`가 비어 있으면 VehicleDebug Panel은 `스폰 준비=미준비`, `Fallback=Dummy HitScan 유지`로 표시하고, `ProjectileActorClass`와 `FireMode=Projectile`이 모두 맞으면 `스폰 준비=준비됨`, `Fallback=Projectile Pool 준비됨`, `전환 요약=ReadyToPoolAcquire`로 표시한다.
Projectile Actor는 `ProjectileStaticMesh`, `InitialSpeed`, `CollisionRadius`, `LifeTimeSeconds`, `bAffectedByGravity`, `GravityScale`을 ProjectileData에서 읽고, Pool 소유자가 있으면 충돌 또는 수명 종료 시 Pool로 반환된다.
사용자가 PIE에서 Projectile Actor가 FireOrigin 기준으로 생성되어 날아가는 것을 확인 완료했다.
사용자가 PIE에서 Projectile Pool 발사 경로의 정상 동작을 확인 완료했다.
사용자가 PIE에서 VehicleDebug Panel `무기 > 발사체 Pool > 마지막 반환 요약` 표시를 확인 완료했다.
사용자가 PIE에서 Muzzle 소켓 FireOrigin 발사가 정상작동함을 확인 완료했다.
터렛 시각 장착 코드는 `Top_01` 하드포인트 기준으로 `DefaultTurretMountData`의 Base / Yaw / Pitch 메쉬를 소켓 계층으로 붙인다. 기존 MountProfile inline Yaw / Pitch fallback은 런타임에서 제거됐다.
`UCFTurretMountData` 최소 구현, `DefaultTurretMountData` 우선 연결, Base / Yaw / Pitch 3단 소켓 계층, 터렛 Yaw / Pitch 시각 추적 회전, 하드포인트 소켓 우선 장착, SnapToTarget 소켓 부착, `RootDelta` 진단, Muzzle 소켓 FireOrigin 보정, MountProfile 각도 제한 legacy 전환은 `Tools\BuildEditor.bat`와 PIE 정상작동 확인을 완료했다. MountProfile legacy Min/Max Yaw/Pitch 필드 실제 삭제도 `Tools\BuildEditor.bat`와 PIE 정상작동 확인을 완료했다.
하드포인트 / YawPivot / PitchPivot / Muzzle 소켓 이름이 지정되어 있는데 실제 메쉬에 없으면 기존 fallback은 유지하지만 VehicleDebug Panel `무기 > 터렛 시각 > 요약`에 `SocketValidation=MissingRequiredSocket`, `MissingRequiredSockets=...`가 표시된다.
터렛 안정화 전에도 필수 거부 조건이 없으면 발사한다. 조준각 초과는 기본 `OutOfWeaponArc` 거부 사유가 아니며, 터렛 목표 / 현재 회전값과 Muzzle FireOrigin은 TurretMountData 제한각 안에 고정한다.
탄퍼짐 / 명중률은 현재 단계에서 적용하지 않고, 후속 사격 시스템에서 별도 설계한다.
DamageData 최소 설계는 `ProjectileData.DefaultDamageData` 단일 직접 참조로 확정한다. `ProjectileData.DamageProfileId`는 Debug fallback이고, `WeaponData.BaseDamage` / `WeaponData.DamageProfileId`는 레거시 확인용이다.
`UCFDamageData`와 `FCFDamageHitContext` 타입은 코드 추가와 `Tools\BuildEditor.bat` 통과를 완료했다.
`ProjectileData.DefaultDamageData` 단일 DamageData 참조와 VehicleDebug Panel `무기 > 피해 데이터` 표시는 빌드와 PIE 정상작동 확인을 완료했다.
DamageData 연결 단계에서는 실제 HP 차감 / 모듈 손상 / 장갑 방향 판정 / 폭발 범위 검색을 하지 않는다. Projectile Actor 충돌과 Dummy HitScan 결과를 같은 HitContext Debug 형식으로 기록 / 표시하는 코드는 추가됐고, 사용자가 빌드와 PIE 정상작동을 확인했다.
DamageData 전환 전 회귀 체크 기준으로 `Reason=LifeExpired`, `Reason=Hit`, `HitActor`, 비행 시간 표시는 계속 유지한다.
하드포인트 위치 작성 방식은 4번 방식, 즉 Socket 또는 BP Preview로 위치를 잡고 최종 수치는 바퀴 위치 캡처 버튼에 통합된 차량 레이아웃 캡처로 DataAsset에 기록하는 흐름으로 확정한다.
하드포인트 명명은 위치 카테고리, 위치 슬롯 인스턴스, 장착 타입을 분리한다.
`Front`, `Top`은 위치 카테고리이고, 실제 장착점은 `Front_01`, `Front_02`, `Top_01`, `Top_02` 같은 슬롯 인스턴스로 기록한다.
Mesh Socket / Preview 이름은 `HP_Front_01`, `HP_Top_01`처럼 위치 슬롯 인스턴스만 표현하고, Fixed / Gimbal / Turret 같은 장착 타입은 DA 장착 / 터렛 섹션의 `MountType`으로 기록한다.
같은 위치 카테고리에 여러 터렛 / 무장이 달릴 수 있다.
2026-06-25 현재 저장 기준 `Mesh_TestSedan`에는 `HP_Front_01`, `HP_Top_01` 소켓이 있고, `Mesh_TestSUV`에는 `HP_Front_01`, `HP_Top_01`, `HP_Top_02` 소켓이 있다.
이 소켓명은 `DA_TestSedan`, `DA_TestSUV`의 `HardpointSlots.SocketName`에 선택 캡처 입력으로 저장되어 있다.
단, 모든 차량에 `HP_Front_01`, `HP_Top_01` 소켓이 필수로 있어야 하는 것은 아니다.
하드포인트 소켓은 차량별 설정에 따라 있을 수도 있고 없을 수도 있으며, 런타임 기준은 DataAsset LocalTransform이다.

---

## 7. 레티클 위치

레티클은 이 폴더의 중심 주제가 아니다.

레티클은 아래 성격으로만 다룬다.

| 구분 | 기준 |
|---|---|
| 위치 | 전투 UI 연동 항목 |
| 목적 | 하드포인트와 무기 조준 방식의 상태를 플레이어에게 표시 |
| 의존 대상 | 차량 플랫폼, 하드포인트, 무기 조준 방식 |
| 본 폴더 범위 | 레티클이 참조해야 할 최소 데이터만 기록 |
| 제외 | 상세 디자인, 아이콘 형태, 최종 HUD 배치 |

---

## 8. Changelog

### v0.67

- 사용자가 `MountProfile.DefaultTurretMountData` / `DefaultWeaponData` legacy 직접 슬롯을 비운 상태로 PIE 정상작동을 확인한 뒤, C++에서 두 슬롯을 삭제한 상태를 상위 요약에 반영했다.
- 장비 기본값은 `MountProfile.DefaultEquipmentPresetData` 하나로 고정하고, TurretMountData / WeaponData는 EquipmentPresetData 내부 참조만 해석한다고 정리했다.
- Build Note: `Tools\BuildEditor.bat`에서 UHT / 컴파일은 통과했지만, 실행 중인 `UnrealEditor.exe`가 DLL을 잠가 링크 단계가 실패했다. 에디터 종료 후 같은 스크립트 재실행이 필요하다.
- Change Note: 상세 소유권과 검증 기준은 `CF_CombatDataOwnership.md` v0.41을 따른다.

### v0.66

- `UCFEquipmentPresetData` 최소 코드 반영과 `MountProfile.DefaultEquipmentPresetData` 우선 해석 상태를 상위 요약에 반영했다.
- `MountProfile.DefaultTurretMountData` / `DefaultWeaponData` 직접 참조는 EquipmentPreset 내부 참조가 비었을 때만 legacy fallback으로 남긴다고 정리했다.
- Build Note: `Tools\BuildEditor.bat`가 EquipmentPresetData 최소 코드 반영 후 통과했다.
- 다음 우선순위를 EquipmentPreset 에셋 생성, MountProfile 연결, 기존 발사 / Projectile / 쿨다운 / 터렛 시각 PIE 회귀 확인으로 이동했다.
- Change Note: README는 상세 필드 명세를 반복하지 않고, 구현 기준은 `CF_CombatDataOwnership.md` v0.40을 참조하는 상위 방향만 유지한다.

### v0.65

- 사용자의 구조 문제 제기를 반영해 `EquipmentPresetData`를 유저-facing 장비 카드이자 에셋 제작 단위로 확정했다.
- `WeaponData`가 `TurretMountData`를 직접 소유하지 않고, `EquipmentPresetData`가 둘의 유효 조합을 소유한다고 정리했다.
- 다음 우선순위를 `UCFEquipmentPresetData` 최소 타입과 `MountProfile.DefaultEquipmentPresetData` 연결 코드 작업 범위 확정으로 이동했다.
- Change Note: 현재 `MountProfile.DefaultTurretMountData` / `DefaultWeaponData` 직접 연결은 즉시 삭제하지 않고 legacy fallback으로 유지한다.

### v0.64

- 사용자의 PIE 확인 결과를 반영해 MountProfile legacy Min/Max Yaw/Pitch 필드 실제 삭제를 빌드 / PIE 정상작동 확인 완료 상태로 승격했다.
- 다음 우선순위를 남은 MountProfile inline legacy 시각 / 기계 필드 삭제 범위 결정으로 이동했다.
- Change Note: 이번 단계는 MountProfile 각도 필드 삭제 검증 완료 기록이다. 남은 legacy 시각 / 기계 필드는 별도 안전 삭제 단계에서 판단한다.

### v0.63

- 사용자가 관련 DataAsset 저장을 완료한 뒤, `FCFVehicleMountProfile`의 legacy `MinYawDeg`, `MaxYawDeg`, `MinPitchDeg`, `MaxPitchDeg` 필드를 C++에서 실제 삭제한 상태를 반영했다.
- `CFVehicleWeaponTypes.h` v1.6.0과 `Tools\BuildEditor.bat` 통과 상태를 반영했다.
- 다음 우선순위를 PIE에서 차량 DA 로드 경고 없음, MountProfiles 상세 정상 표시, TurretMountData 단독 제한, 기존 발사 / Projectile / 쿨다운 회귀 확인으로 이동했다.
- Change Note: 차량 구조상 각도 제한이 필요해지면 삭제한 MountProfile 필드를 되살리지 않고 별도 `HardpointArcConstraint` 계열 데이터로 설계한다.

### v0.62

- 사용자의 PIE 확인 결과를 반영해 MountProfile 각도 제한 legacy 전환을 빌드 / PIE 정상작동 확인 완료 상태로 승격했다.
- 당시 다음 우선순위를 MountProfile 각도 제한 PIE 검증에서 DataAsset 재저장 확인으로 이동했다.
- Superseded: v0.63에서 DataAsset 저장 확인 후 legacy Min/Max Yaw/Pitch 필드 삭제를 완료했다.
- Change Note: 실제 C++ legacy 필드 삭제는 기존 DataAsset 재저장 확인 뒤 별도 단계로 진행한다.

### v0.61

- MountProfile의 `MinYawDeg`, `MaxYawDeg`, `MinPitchDeg`, `MaxPitchDeg`를 차량 DA 편집 노출 / 런타임 터렛 제한에서 제거하는 현재 우선순위를 반영했다.
- 터렛 제한각은 P0에서 `UCFTurretMountData` 단독 기준이고, `bExposedModule`은 그대로 MountProfile에 유지한다고 요약했다.
- `CF_CombatDataOwnership.md` v0.35 기준으로 VehicleDebug Panel `무기 > 터렛 조준 > 요약`의 `Source=TurretMountData`, `LimitCorrection=No` 확인을 다음 검증으로 추가했다.
- Change Note: 차량 구조상 회전 제한이 필요해질 때는 MountProfile inline 각도값 재사용이 아니라 별도 제약 데이터로 다시 설계한다.
- Build Note: `Tools\BuildEditor.bat` MountProfile 각도 제한 legacy 전환 후 통과했다.

### v0.60

- Damage Runtime 전에 MountProfile inline 터렛 fallback 제거 단계를 먼저 진행하기로 한 현재 우선순위를 반영했다.
- `CF_CombatDataOwnership.md`를 기준으로 legacy inline 터렛 필드는 직렬화 호환용으로만 보존하고, 에디터 노출 / 런타임 fallback은 중단한다고 요약했다.
- 다음 검증을 `Tools\BuildEditor.bat`와 PIE에서 `DefaultTurretMountData` 미지정 시 터렛 시각 / 조준 Missing 표시, 발사 / Projectile / 쿨다운 유지 확인으로 좁혔다.
- Change Note: 실제 필드 삭제는 이번 단계가 아니라 빌드 / PIE / DataAsset 재저장 확인 뒤 별도 단계로 진행한다.

### v0.54

- DamageData 최소 설계를 상위 요약에 반영했다.
- 수정 용이성을 위해 당시에는 `ProjectileData.DefaultDamageData` 우선, `WeaponData.DefaultDamageData` fallback, 기존 `WeaponData.BaseDamage` 임시 fallback 순서를 기록했다.
- DamageData 연결 단계에서는 실제 HP 차감 없이 HitContext Debug부터 검증한다고 명시했다.
- Superseded: v0.57에서 DamageData 직접 참조는 `ProjectileData.DefaultDamageData` 단일 경로로 정리했다.

### v0.55

- `UCFDamageData`와 `FCFDamageHitContext` 타입 추가 및 `Tools\BuildEditor.bat` 통과 상태를 상위 요약에 반영했다.
- 다음 우선순위를 DamageData 타입 추가에서 DamageData 참조 연결과 VehicleDebug 표시로 이동했다.
- Change Note: 실제 Damage 적용과 HP 차감은 아직 후속 작업이다.

### v0.56

- `ProjectileData.DefaultDamageData` / `WeaponData.DefaultDamageData` 참조 연결 구현과 `Tools\BuildEditor.bat` 통과 상태를 상위 요약에 반영했다.
- VehicleDebug Panel `무기 > 피해 데이터`의 지정 여부 / 피해 ID / 해석 경로 / 요약 표시 추가 상태를 반영했다.
- 다음 우선순위를 DamageData 참조 연결 구현에서 PIE 표시 확인과 HitContext Debug 기록으로 이동했다.
- Change Note: 실제 Damage 적용, HP 차감, 모듈 손상은 아직 후속 작업이다.
- Superseded: v0.57에서 `WeaponData.DefaultDamageData`는 제거 대상으로 전환했다.

### v0.57

- DamageData 직접 참조 슬롯을 `ProjectileData.DefaultDamageData` 하나로 정리했다.
- HitScan / Laser도 실제 Actor를 스폰하지 않는 가상 ProjectileData를 통해 DamageData를 참조한다고 명시했다.
- `WeaponData.BaseDamage` / `WeaponData.DamageProfileId`는 레거시 확인용이며 실제 DamageData 해석 경로가 아니라고 정리했다.
- 사용자가 빌드와 PIE 정상작동을 확인했다.
- Change Note: 실제 Damage 적용과 HitContext Debug 기록은 아직 후속 작업이다.

### v0.58

- 다음 우선순위를 Damage HitContext Debug 빌드 / PIE 검증과 이후 Damage Runtime 범위 결정으로 갱신했다.
- VehicleDebug Panel `무기 > 피해 HitContext`에서 기록 여부, Source, Hit, DamageId, WeaponId, ProjectileId, 피격 Actor, 발사 주체, 위치 / 방향 / 요약을 확인하는 기준을 추가했다.
- Dummy HitScan 결과와 Projectile Actor 충돌 결과를 같은 `FCFDamageHitContext` 형식으로 기록 / 표시하는 코드 추가 상태를 상위 요약에 반영했다.
- Change Note: 실제 HP 차감, 모듈 손상, 장갑 방향 판정, 폭발 범위 검색은 아직 후속 Damage Runtime 작업이다.

### v0.59

- Damage HitContext Debug 추가 후 사용자가 빌드와 PIE 정상작동을 확인한 상태를 상위 요약에 반영했다.
- 다음 우선순위를 실제 Damage Runtime 작업 범위 결정으로 이동했다.
- Change Note: Damage HitContext는 Debug 표시 전용으로 유지한다. 실제 HP 차감, 모듈 손상, 장갑 방향 판정, 폭발 범위 검색은 아직 구현하지 않는다.

### v0.53

- 터렛 안정화 전 발사 정책을 상위 요약에 반영했다.
- 조준각 초과는 기본 발사 거부 조건이 아니며, 터렛 회전값과 Muzzle FireOrigin만 DA 제한각 안에 고정한다고 명시했다.
- 탄퍼짐 / 명중률은 이번 단계에서 적용하지 않고 후속 사격 시스템에서 다룬다고 기록했다.
- Build Note: 첫 빌드는 남아 있던 CarFight `UnrealEditor.exe` DLL 잠금으로 실패했고, 해당 프로세스 종료 후 `Tools\BuildEditor.bat` 재실행은 통과했다.

### v0.52

- 사용자의 PIE 확인 결과를 반영해 Muzzle 소켓 FireOrigin 보정을 정상작동 확인 완료 상태로 승격했다.
- 다음 우선순위를 Muzzle PIE 확인에서 DamageData 전 정책 결정으로 이동했다.
- Change Note: Muzzle 소켓 X축 발사 방향 기준, 하드포인트 fallback, Dummy HitScan / Projectile Pool / 쿨다운 회귀 기준은 유지한다.

### v0.51

- Muzzle 소켓 FireOrigin 보정 구현과 `Tools\BuildEditor.bat` 통과 상태를 상위 요약에 반영했다.
- VehicleDebug Panel `무기 > 터렛 시각 > 요약`에서 `MuzzleStatus=Resolved/MissingRequiredSocketFallback`을 확인하는 기준을 추가했다.
- Change Note: Muzzle 소켓이 없거나 Pitch 메쉬가 없으면 기존 하드포인트 FireOrigin fallback, Dummy HitScan, Projectile Pool, 쿨다운 검증은 유지한다.
- Build Note: `Tools\BuildEditor.bat` Muzzle 소켓 FireOrigin 보정 추가 후 통과했다.

### v0.50

- 터렛 하드포인트 / Yaw / Pitch 필수 소켓 누락 진단을 상위 요약에 반영했다.
- VehicleDebug Panel `무기 > 터렛 시각 > 요약`에서 `SocketValidation=OK/MissingRequiredSocket`, `MissingRequiredSockets=...`로 소켓명 오타를 확인하는 기준을 추가했다.
- Change Note: 필수 소켓이 누락되어도 기존 fallback은 유지한다. 단, 의도적 fallback과 소켓명 오타를 구분하기 위해 Debug 요약과 로그에 누락 상태를 남긴다.
- Build Note: 에디터 종료 후 `Tools\BuildEditor.bat` 재실행 결과 v2.92.0 MissingRequiredSocket 진단 추가 상태로 통과했다.

### v0.49

- 터렛 하드포인트 / Yaw / Pitch 소켓 부착을 SnapToTarget 기준으로 명확히 한 상태를 상위 요약에 반영했다.
- VehicleDebug Panel `무기 > 터렛 시각 > 요약`의 `RootDelta`로 소켓 루트 부착 문제와 메쉬 피벗 / 상대 Transform 문제를 구분하는 기준을 추가했다.
- `Tools\BuildEditor.bat` SnapToTarget 소켓 부착과 RootDelta 진단 추가 후 통과 상태를 기록했다.
- Change Note: `RootDelta`가 0에 가까우면 하드포인트 소켓 부착은 정상으로 보고, 이후 오차는 Base / Yaw / Pitch 메쉬 피벗, 소켓, RelativeTransform 조정 문제로 분리한다.

### v0.48

- 터렛 시각 장착과 FireOrigin 계산에서 `HardpointSlots.SocketName`을 실제 `SM_Body` 소켓으로 우선 사용하는 수정과 빌드 통과 상태를 상위 요약에 반영했다.
- VehicleDebug Panel `무기 > 터렛 시각 > 요약`에서 `HardpointSocketResolved=Yes/No`로 소켓 부착 여부를 확인하는 기준을 추가했다.
- 다음 PIE 확인 기준을 “바닥 fallback 여부”가 아니라 `HardpointSocketResolved=Yes`와 실제 소켓 위치 부착 확인으로 좁혔다.
- Change Note: `SocketName`이 비어 있거나 차체 소켓이 없으면 기존 `LocalLocation` / `LocalRotation` fallback을 유지한다. Dummy HitScan / Projectile / Cooldown 검증 기준은 유지한다.

### v0.47

- 터렛 Yaw / Pitch 시각 추적 회전 구현과 `Tools\BuildEditor.bat` 통과 상태를 상위 요약에 반영했다.
- VehicleDebug Panel `무기 > 터렛 조준`에서 현재/목표 Yaw/Pitch와 안정화 상태를 확인하는 기준을 추가했다.
- 다음 작업을 PIE 회전 감각 확인과 Muzzle 소켓 FireOrigin 전환 조건 확정으로 좁혔다.
- Change Note: 기존 Yaw / Pitch fallback과 Dummy HitScan / Projectile / Cooldown 검증 기준은 유지한다. DamageData와 Muzzle FireOrigin 전환은 아직 후속 작업이다.

### v0.46

- 터렛 마운트 시각 계층을 Base / Yaw / Pitch 3단 소켓 구조로 상위 요약에 반영했다.
- VehicleDebug Panel `무기 > 터렛 시각`에서 Base / Yaw / Pitch 메쉬 이름을 확인하는 기준을 반영했다.
- 다음 작업을 터렛 마운트 DataAsset의 Base / Yaw / Pitch 소켓 값 보강과 PIE 장착 확인으로 좁혔다.
- `Tools\BuildEditor.bat` Base / Yaw / Pitch 3단 소켓 계층 변경 후 통과 상태를 기록했다.
- Change Note: 기존 Yaw / Pitch fallback과 Dummy HitScan / Projectile / Cooldown 검증 기준은 유지한다. 실제 Yaw / Pitch 추적 회전과 Muzzle 소켓 FireOrigin 전환은 아직 후속 작업이다.

### v0.45

- `UCFTurretMountData` 최소 DataAsset 구현과 `MountProfile.DefaultTurretMountData` 연결 추가 상태를 상위 요약에 반영했다.
- VehicleDebug Panel `무기 > 터렛 시각`의 마운트 데이터 지정 / ID / 요약 표시 추가를 반영했다.
- `Tools\BuildEditor.bat` TurretMountData 최소 구현 후 통과 상태를 기록했다.
- 다음 작업을 터렛 마운트 DataAsset 생성과 `DA_TestSedan` / `DA_TestSUV` 에셋 연결 PIE 확인으로 좁혔다.
- Change Note: 실제 Yaw / Pitch 추적 회전과 Muzzle 소켓 FireOrigin 전환은 아직 후속 작업이다.

### v0.44

- `TurretMountData = 무기 거치대 / 회전 플랫폼`, `WeaponData = 실제 발사 장치` 개념 결정을 상위 요약에 반영했다.
- 유저에게는 내부 Mount / Weapon / Projectile 분리를 그대로 보여주지 않고, 장기적으로 `EquipmentPreset` 장비 카드 단위로 단순화한다고 기록했다.
- 다음 작업을 `UCFTurretMountData` 최소 타입 추가와 `MountProfile.DefaultTurretMountData` 연결로 좁혔다.
- Change Note: 기존 `MountProfile` inline 터렛 시각 필드는 기존 에셋 보호용 fallback으로 유지하고, 새 데이터 원본은 `TurretMountData`로 이동한다.

### v0.43

- DamageData 작업 전에 터렛 장착 / 회전 / Muzzle 기준을 먼저 닫기로 한 현재 결정을 상위 요약에 반영했다.
- `MountProfile` 터렛 Yaw / Pitch 시각 메쉬 필드와 VehicleDebug Panel `무기 > 터렛 시각` 표시 추가 상태를 반영했다.
- 터렛 시각 장착 추가 후 `Tools\BuildEditor.bat` 통과 상태를 기록했다.
- 다음 작업을 터렛 시각 장착 PIE 확인, Yaw / Pitch 회전 정책 구현, Muzzle 소켓 FireOrigin 전환으로 좁혔다.
- Change Note: DamageData와 실제 피해 적용은 터렛 기준 확인 이후로 유지한다. Projectile Pool 반환 요약은 Damage 전 회귀 기준으로 계속 보존한다.

### v0.42

- 사용자의 PIE `무기 > 발사체 Pool > 마지막 반환 요약` 확인 완료 상태를 상위 요약에 반영했다.
- Projectile Pool 마지막 반환 요약 검증을 완료 상태로 승격했다.
- 다음 작업을 DamageData 최소 설계와 Projectile 충돌 / 수명 / 중력 회귀 기준 정리로 옮겼다.
- Change Note: 마지막 반환 요약은 Debug 표시 전용이며, DamageData 전환 전 회귀 체크 기준으로 유지한다.

### v0.41

- 사용자의 FireRatePerMinute 전환 후 정상 동작 확인 상태를 상위 요약에 반영했다.
- 사용자의 Projectile Pool 발사 경로 정상 동작 확인 상태를 상위 요약에 반영했다.
- VehicleDebug Panel `무기 > 발사체 Pool > 마지막 반환 요약` 추가와 `Tools\BuildEditor.bat` 통과 상태를 반영했다.
- 다음 검증 기준을 `LifeExpired` / `Hit` / `HitActor` / 비행 시간 표시 확인으로 좁혔다.
- Change Note: 마지막 반환 요약은 Debug 표시 전용이며, DamageData와 실제 피해 적용은 아직 후속 작업이다.

### v0.40

- `UCFWeaponData` 입력 기준을 `CooldownSeconds`에서 `FireRatePerMinute`로 전환한 상태를 상위 요약에 반영했다.
- VehicleDebug Panel `무기 > 무기 데이터`의 발사 제한 표시 기준을 `분당 발사속도`로 갱신했다.
- 기존 `WeaponCooldown` 거부 사유와 `남은 쿨다운` 실시간 표시는 유지한다고 명시했다.
- Change Note: 기존 에셋의 `CooldownSeconds`는 숨김 마이그레이션 값으로 유지하고, 새 입력 / 표시 기준은 `분당 발사속도`다.

### v0.39

- Projectile Pool Debug 카운트 표시 추가와 빌드 통과 상태를 상위 요약에 반영했다.
- VehicleDebug Panel `무기 > 발사체 Pool`에서 Pool 컴포넌트 / 전체 Pool 수 / 활성 Pool 수 / 비활성 Pool 수를 확인한다고 명시했다.
- 다음 PIE 검증 기준을 Actor 수 추정이 아니라 Debug Panel 카운트 변화 확인으로 구체화했다.
- Change Note: v0.38의 Pool 구현 완료 / PIE 검증 대기 상태는 유지하고, 검증 관측 지표만 구체화했다.

### v0.38

- `UCFProjectilePoolComp` 구현과 Projectile Pool Acquire 빌드 통과 상태를 상위 요약에 반영했다.
- VehicleDebug Panel 표시 기준을 `Projectile Pool 준비됨` / `ReadyToPoolAcquire`로 갱신했다.
- 다음 작업을 Projectile Pool PIE 재사용 검증, 충돌 / 수명 / 중력 세부 검증, DamageData 분리 순서 재검토로 좁혔다.
- Change Note: v0.37의 Pool Deferred 상태를 구현 완료 / 빌드 통과 / PIE 검증 대기로 승격했다. Damage 적용과 터렛 메시 장착은 후순위로 유지한다.

### v0.37

- 사용자의 Projectile Actor PIE 비행 확인 완료 상태를 상위 요약에 반영했다.
- `FireMode=Projectile` 설정 뒤 FireOrigin 기준으로 Projectile Actor가 생성되어 날아가는 것을 확인 완료로 승격했다.
- 다음 작업을 Projectile Pool 구조 분리, 충돌 / 수명 / 중력 세부 검증, DamageData 분리로 좁혔다.
- Change Note: v0.36의 코드 구현 완료 상태에 사용자 PIE 검증 결과를 추가했다. Damage 적용과 터렛 메시 장착은 후순위로 유지한다.

### v0.36

- 공통 `ACFProjectileActor` v1.0.0 추가와 실제 Projectile Actor 스폰 경로 구현 상태를 상위 요약에 반영했다.
- `UCFProjectileData` v1.2.0의 메시 설정 필드와 `ACFProjectileActor` 기반 `ProjectileActorClass` 제한을 반영했다.
- `UCFVehicleWeaponComp` v1.5.0의 `FireMode=Projectile` 기반 스폰 준비 판정과 `ACFVehiclePawn` v2.81.0의 스폰 실패 시 Dummy HitScan fallback을 반영했다.
- Pool 대비는 Actor 생명주기와 `bDestroyWhenDeactivated` 정책으로 1차 반영하고, 실제 Pool 매니저는 후속 작업으로 분리했다.
- Change Note: v0.35의 "스폰 준비 게이트" 단계를 실제 P0 Projectile Actor 스폰 구현 상태로 승격했다. Damage 적용과 터렛 메시 장착은 후순위로 유지한다.

### v0.35

- `ProjectileActorClass` 기반 Projectile 스폰 준비 게이트 구현 상태를 상위 요약에 반영했다.
- VehicleDebug Panel `무기 > 발사체 데이터`의 `스폰 준비`, `Fallback`, `전환 요약` 표시 기준을 반영했다.
- 실제 Projectile Actor 스폰은 아직 하지 않고, 준비 상태가 `준비됨`이어도 Dummy HitScan Trace를 유지한다고 명시했다.
- 다음 작업을 실제 Projectile Actor 스폰 승인 범위 판단과 DamageData 분리 순서 재검토로 좁혔다.
- Change Note: v0.34의 ProjectileData PIE 검증 완료 상태를 유지하면서, 그 위에 스폰 준비 판정만 추가했다.

### v0.34

- 사용자의 ProjectileData PIE 검증 완료 상태를 상위 요약에 반영했다.
- 연결 전 `미지정 / Dummy HitScan 유지`, 연결 후 `지정 / 발사체 ID / 요약 표시` 정상 확인을 기록했다.
- 빠른 연속 Fire 시 남은 쿨다운 감소와 `WeaponCooldown` 거부 사유 유지 확인을 기록했다.
- 다음 작업을 Projectile Actor 전환 범위 판단과 DamageData 분리 순서 재검토로 좁혔다.
- Change Note: v0.33의 남은 에디터 연결 검증을 완료 상태로 승격하고, 실제 Projectile Actor 스폰은 아직 하지 않는다는 경계를 유지했다.

### v0.33

- `UCFProjectileData` 최소 DataAsset 구현 완료 상태를 상위 요약에 반영했다.
- WeaponData `DefaultProjectileData` 직접 참조와 VehicleDebug Panel `무기 > 발사체 데이터` 표시 추가를 반영했다.
- 다음 작업을 ProjectileData 에셋 생성과 WeaponData 연결 에디터 검증으로 좁혔다.
- 실제 Projectile Actor 스폰은 아직 하지 않고 Dummy HitScan fallback을 유지한다고 명시했다.
- `Tools\BuildEditor.bat` 통과 상태를 반영했다.
- Change Note: v0.32의 구현 전 문서화 상태를 최소 코드 구현 완료 상태로 갱신하고, 남은 작업을 에셋 연결 검증으로 분리했다.

### v0.32

- 빠른 연속 Fire PIE 쿨다운 검증 완료 상태를 상위 요약에 반영했다.
- VehicleDebug Panel `남은 쿨다운` 감소와 Aim 쪽 `WeaponCooldown` 거부 사유 확인을 완료 상태로 기록했다.
- 다음 작업을 ProjectileData / DamageData 분리 범위 문서화와 승인 후 코드 작업으로 좁혔다.
- `CF_CombatDataOwnership.md`가 `UCFProjectileData` 최소 설계와 Dummy HitScan / Projectile 전환 기준을 소유한다고 명시했다.
- Change Note: v0.31의 "남은 확인" 문장을 검증 완료 상태로 교체하고, 코드 구현은 사용자 승인 후 진행한다는 경계를 추가했다.

### v0.31

- 사용자의 PIE 검증 완료 상태를 반영했다.
- `무기 > 무기 데이터`에서 지정 여부 / WeaponId / 호환성 표시 확인 완료를 상위 요약에 추가했다.
- WeaponData `MaxRange` / `CooldownSeconds` Fire 검증 연결을 상위 상태에 반영했다.
- `Tools\BuildEditor.bat` 통과 상태를 반영했다.
- 다음 작업을 연속 Fire 쿨다운 검증 후 ProjectileData / DamageData 분리로 좁혔다.
- Change Note: 기존 FireOrigin / WeaponData 표시 완료 설명은 유지하고, 남은 확인을 쿨다운 런타임 검증으로 교체했다.

### v0.30

- `UCFWeaponData` 최소 DataAsset 타입 구현과 `MountProfile.DefaultWeaponData` 참조 추가 상태를 반영했다.
- VehicleDebug Panel `무기(Weapon)` 섹션의 WeaponData 지정 / 호환성 / 요약 표시를 상위 요약에 추가했다.
- 다음 작업을 WeaponData 에셋 생성과 ProjectileData / DamageData 분리로 좁혔다.
- Change Note: 기존 FireOrigin / Weapon Debug 완료 설명은 유지하고, 오래된 다음 우선순위 문장을 최신 전투 데이터 분리 단계로 교체했다.

### v0.29

- 사용자의 FireOrigin 테스트 완료 상태를 반영했다.
- VehicleDebug Panel `무기(Weapon)` Navigation 섹션 구현 완료와 빌드 통과 상태를 반영했다.
- 다음 작업을 WeaponData / ProjectileData / DamageData 분리 전 기준선 확인으로 좁혔다.

### v0.28

- `MountProfile + UCFVehicleWeaponComp + FireOrigin` P0 코드 구현 완료와 빌드 통과 상태를 반영했다.
- 다음 작업을 `DA_TestSedan` / `DA_TestSUV` 에디터 저장값 및 FireOrigin 위치 검증으로 좁혔다.

### v0.27

- `CF_CombatDataOwnership.md`를 추가해 전투 데이터 소유권 기준 문서로 연결했다.
- P0 1차 구현 범위를 `MountProfile + UCFVehicleWeaponComp + FireOrigin`으로 제한한다는 상위 방향을 추가했다.

### v0.26

- 주행감은 현재 충분히 나눠진 상태로 보고, 가까운 우선순위에서 제외했다.
- 다음 초점을 하드포인트 장착 / 터렛 / Fire / UI 상태 계약 연결로 명확히 했다.

### v0.25

- `DA_TestSedan` 기준 기본 주행 검증 완료 상태를 반영했다.
- `DA_TestSUV` 대조 테스트 방식은 별도 자동화 없이 수동 VehicleData 전환으로 결정했다.

### v0.24

- `DA_PoliceCar`를 현재 작업 기준에서 폐기하고, `BP_CFVehiclePawn.VehicleData` 기본 연결이 `DA_TestSedan`으로 저장된 상태를 반영했다.
- `DA_TestSUV`는 P0 대조 테스트 후보로 유지했다.

### v0.23

- 차량 레이아웃 통합 캡처 코드 구현과 `DA_TestSedan` / `DA_TestSUV` 하드포인트 슬롯 저장 상태를 상위 요약에 반영했다.
- 테스트 메시의 현재 하드포인트 소켓명을 `HP_` prefix 표준명 기준으로 갱신했다.
- 남은 확인을 `BP_CFVehiclePawn.VehicleData` 테스트 전환 방식으로 좁혔다.

### v0.22

- `CF_SocketCaptureCodePlan.md`를 추가해 통합 캡처 버튼의 코드 작업 범위를 분리했다.
- 하드포인트 선택 캡처 정책을 실제 구현 대상 파일과 검증 기준으로 연결했다.

### v0.21

- 하드포인트 소켓이 모든 차량에 필수로 존재해야 한다는 오해를 제거했다.
- 소켓은 차량별 설정에 따라 있을 수도 있고 없을 수도 있는 캡처 입력으로 정리했다.
- `HP_Front_02`는 하드포인트 증가 가능성 예시로만 다루도록 상위 방향을 조정했다.

### v0.20

- 실제 StaticMesh SocketName 정책을 `HP_` prefix 리네임으로 확정했다.
- 하드포인트 LocalTransform 캡처 방식을 바퀴 위치 캡처 버튼에 통합하는 자동 캡처 방식으로 확정했다.
- `CF_VehicleWheelSocketPlan_v0_1.md`의 역할 설명을 휠 / 하드포인트 통합 캡처 계획으로 갱신했다.

### v0.19

- `CF_MeshSocketAudit.md`를 추가해 P0 후보 메시의 실제 Socket 목록과 Transform 후보를 별도 근거로 분리했다.
- `Mesh_TestSedan`, `Mesh_TestSUV`에는 작성 보조용 하드포인트 Socket 후보가 있음을 핵심 방향에 반영했다.
- 현재 실제 SocketName이 문서 권장 예시의 `HP_` prefix와 다르므로 코드 착수 전 명명 결정을 남겼다.

### v0.18

- 핵심 방향에 같은 위치 카테고리의 복수 하드포인트 지원 원칙을 추가했다.
- `Front`, `Top`을 위치 카테고리로 정리하고 실제 장착점은 `Front_01`, `Top_02` 같은 슬롯 인스턴스로 기록한다고 명시했다.

### v0.17

- 핵심 방향에 하드포인트 위치 슬롯과 장착 타입 분리 원칙을 추가했다.
- Mesh Socket / Preview 이름은 위치 중심, DA 장착 / 터렛 섹션은 `MountType` 중심으로 기록한다는 기준을 요약했다.

### v0.16

- 하드포인트 위치 작성 방식을 4번 방식, Socket / BP Preview 배치 후 DataAsset 캡처로 확정했다.
- `CF_HardpointAuthoring.md`의 역할을 P0 권장안에서 P0 선택안으로 갱신했다.

### v0.15

- `CF_HardpointAuthoring.md`를 추가해 하드포인트 위치 작성 방식 비교와 P0 권장안을 분리했다.
- 추천 읽기 순서에 하드포인트 작성 방식 문서를 추가했다.

### v0.14

- `DA_TestSedan`, `DA_TestSUV` AssetDump 확인 결과를 핵심 방향에 반영했다.
- P0 후보 메시 경로가 `/Game/CarFight/Vehicles/TestSedan`, `/Game/CarFight/Vehicles/TestSUV` 기준임을 관련 문서에서 정정했다.
- `BP_CFVehiclePawn.VehicleData` 전환 방식과 `DA_TestSedan` 설정 보강 판단이 남은 작업임을 명시했다.

### v0.13

- `CF_VehicleWheelSocketPlan_v0_1.md`를 v0.4로 갱신해 spawned-only 차량 기준의 `DA_*` 직접 소켓 캡처 절차를 기본 경로로 정리했다.

### v0.12

- `CF_VehicleWheelSocketPlan_v0_1.md`를 v0.3으로 갱신해 PIE 외부 에디터 알림과 빌드 후 에디터 재시작 체크포인트를 추가했다.

### v0.11

- `CF_VehicleWheelSocketPlan_v0_1.md`를 v0.2로 갱신해 버튼 실행 후 에디터 갱신, WheelSync 재준비, 무반응 진단 체크포인트를 추가했다.

### v0.10

- `Mesh_TestSUV`의 `Wheel_Anchor_FL/FR/RL/RR` 소켓을 기준으로 휠 레이아웃을 DataAsset에 캡처하는 계획 문서 `CF_VehicleWheelSocketPlan_v0_1.md`를 추가했다.

### v0.9

- 임시 P0 세단 / SUV 메시 후보로 `Mesh_TestSedan`, `Mesh_TestSUV`를 기록했다.
- 차량 DataAsset 지정은 아직 남은 작업으로 분리했다.

### v0.8

- P0 테스트 차량의 기본 차형 후보를 세단(Sedan)과 SUV로 기록했다.
- 실제 메시 / DataAsset 경로는 아직 별도 지정 대상으로 남겼다.

### v0.7

- `DA_PoliceCar`를 P0 최종 대상 차량이 아니라 임시 감사 기준으로 정리했다.
- P0 테스트 차량 1~2종을 별도 지정해야 한다는 흐름을 핵심 방향에 추가했다.

### v0.6

- `Combined_Body` Socket Manager 0 소켓 확인 결과를 문서 묶음 방향에 반영했다.
- P0 하드포인트 기준이 DataAsset 로컬 Transform임을 읽기 흐름에 맞췄다.

### v0.5

- AssetDump 확인 결과 문서 `CF_AssetDumpResult.md`를 추가했다.
- 읽기 순서에 자동 자산 확인 단계를 추가했다.

### v0.4

- 에디터 확인 가이드 `CF_EditorCheckGuide.md`를 추가했다.
- 사용자가 직접 확인해야 하는 BP / DataAsset / Mesh Socket / PIE 체크 흐름을 읽기 순서에 추가했다.

### v0.3

- 에디터 자산 감사 문서 `CF_EditorAssetAudit.md`를 추가했다.
- 기준 BP / DataAsset / Reticle / Mesh 확인 흐름을 읽기 순서에 추가했다.

### v0.2

- 현재 기준선 문서 `CF_PlatformBaseline.md`를 추가했다.
- 전투 플랫폼 계약 문서 `CF_CombatContract.md`를 추가했다.
- P0 하드포인트 매트릭스 `CF_HardpointMatrix.md`를 추가했다.
- 코드 착수 게이트 `CF_CodeStartGate.md`를 추가했다.
- 새 문서 읽기 순서를 추가했다.

### v0.1

- `VehiclePlatformPlan` 작업 폴더 생성.
- 기획 SSOT와 작업 문서의 저장 위치를 분리.
- 세션 목적을 `전투기능이 붙을 수 있는 최소 차량 플랫폼 규격 정의`로 고정.
- 레티클을 중심 주제가 아니라 전투 UI 연동 항목으로 위치 지정.

---

## 9. Migration 메모

- `Document/ProjectSSOT/CombatPlan/`에는 이번 작업의 초안이나 구현 체크리스트를 저장하지 않는다.
- 확정 전 판단은 이 폴더에 먼저 기록한다.
- CombatPlan SSOT 갱신이 필요해지면 별도 변경안으로 분리한다.
