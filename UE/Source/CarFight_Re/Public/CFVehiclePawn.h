// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 2.152.0
// Date: 2026-08-19
// Description: CarFight 싱글플레이 차량 Pawn 기준 클래스 / CF-FQ-032 UI-P0-06 Player-facing Weapon Selection 직접 순번 입력 통합
// Changelog:
// - v2.152.0: 실제 SelectableWeapons의 1-based 순번을 받는 Axis1D InputAction 슬롯과 Started handler를 추가. 숫자키 ordinal 값만 0-based RequestSelectWeaponIndex로 변환하며 새 WeaponGroup ID, cycle state, 내부 MountProfileId 입력 의미를 만들지 않음.
// - v2.151.0: Applied Fitting 고정 표시 순서의 Weapon Selection 요청 API를 추가. Launcher active 시 WeaponChanged 정상 취소 후 WeaponComp를 전환하고 기존 단일 활성 Turret Visual을 같은 선택으로 재구성.
// - v2.150.0: Weapon Heat accepted-fire 통합 Automation이 실제 ExecuteAcceptedFireCommand→ApplyFireResult 경계를 검증할 수 있도록 전용 friend만 추가. Runtime public API 확대 없음.
// - v2.149.0: TargetSelect Marker 자동 CreateWidget/AddToViewport 소유권을 Pawn에서 제거하고 기존 Class/Instance/API는 UISubsystemOwned 전환용 직렬화·호환 경계로 유지.
// - v2.148.0: AimReticle 자동 CreateWidget/AddToViewport 소유권을 Pawn에서 제거하고 기존 Class/Instance/API는 UISubsystemOwned 전환용 직렬화·호환 경계로 유지.
// - v2.147.0: P0-03 실제 입력 의미를 1회 입력→ActiveScanDurationSec 자동 실행으로 확정. InputAction_StartActiveScan은 IA_ActiveScan 기본 자산을 사용하고 별도 Stop 입력 자산은 만들지 않음.
// - v2.146.0: 실제 키/트리거 의미를 선결하지 않는 optional Active Scan Start/Stop InputAction 슬롯과 Pawn Gameplay command wrapper를 추가. Sensor Component는 입력을 직접 Bind하지 않으며 Fitting/Config를 재적용하지 않음.
// - v2.145.0: UCFVehicleSensorComp 기본 서브오브젝트와 Blueprint getter를 추가해 TargetSelect와 독립된 Sensor Contact/Knowledge Runtime 소유권을 준비. Sensor 준비 여부는 기존 CoreReady/CombatReady를 차단하지 않음.
// - v2.144.0: CF-FQ-015 VD-P0-03 Automation이 protected ApplyVehicleDataConfig를 실제 호출해 VehicleData→Movement/DriveState 전달을 검증할 수 있도록 테스트 전용 friend 경계를 추가. 런타임 동작·Blueprint API 변경 없음.
// - v2.143.0: CF-FQ-031 AMMO-P0-07 Applied Fitting Snapshot의 명시적 출격 탄약과 finite WeaponInstance를 VehicleAmmoComp로 초기화하는 Combat Runtime 연결을 추가.
// - v2.142.0: CF-FQ-031 AMMO-P0-05 현재 활성 무기의 FullMagazine 재장전을 요청하는 BlueprintCallable 명령 진입점을 추가.
// - v2.140.0: CF-FQ-031 AMMO-P0-03 SingleCycle 발사 Transaction 자동화가 실제 ExecuteAcceptedFireCommand를 검증할 수 있도록 전용 테스트 friend 경계를 추가.
// - v2.139.0: CF-FQ-031 AMMO-P0-02 VehicleAmmoComp 기본 서브오브젝트와 공개 Getter·재초기화/EndPlay Reset 경계를 추가. 기존 Fire에는 아직 연결하지 않음.
// - v2.138.0: VehicleDebug Snapshot에 현재 선택 대상의 표시 정보, 추적 상태, 방어·내구도 상태와 마지막 방어 피해 결과를 추가.
// - v2.136.0: 첫 발과 Ripple·Salvo 후속 발사가 같은 Command Target 위치·Guidance Target Actor Snapshot을 Launch Context에 사용하도록 통합.
// - v2.135.0: Launch Context에 발사 순간 선택 Target Actor Snapshot을 복사해 이미 발사된 미사일 목표를 차량 선택 상태와 분리.
// - v2.134.0: 차량 코어 Runtime 준비와 전투 Runtime 준비 상태를 분리하고 기존 bVehicleRuntimeReady를 코어 호환 상태로 유지.
// - v2.133.0: FIT-P0-05 PreRegister Initial Mass 적용, BeginPlay 실제 VehicleMesh 질량 검증과 Cached Snapshot Commit 순서를 추가.
// - v2.132.0: FIT-P0-04 VehicleFittingComp, 선택적 VehicleFittingData와 출격 Weapon·Defense Snapshot 적용 순서를 추가.
// - v2.131.0: UI-P0-02 Pause 진입 전 차량 이동·조향·브레이크·핸드브레이크·Look 입력 잔류를 중립화하는 ClearGameplayInputForPause를 추가.
// - v2.130.0: VehicleDebug Weapon 카테고리에 VehicleDefenseComp 준비·Fallback·현재 상태와 마지막 전체 피해 결과 요약을 추가.
// - v2.129.0: VehicleDefenseComp 기본 서브오브젝트·초기화와 HitScan 정식 방어 피해 진입점을 추가하고 기존 Health Debug 호환을 유지.
// - v2.128.0: Hitscan이 같은 차량 Projectile을 Trace에서 제외하고 다른 차량의 요격 가능 Projectile 적중을 Intercepted 경로로 전달.
// - v2.127.0: LM-P0-04 Direct·Angled·Vertical Launch Context, 차량 속도 상속과 실제 사출 방향 안전 검사 계약을 추가.
// - v2.126.0: LauncherComp 기본 서브오브젝트와 LM-P0-03B Ripple·Salvo 예약 발사, 고정 Command Target, 내부 쿨다운 우회 실행 계약을 추가.
// - v2.125.0: 가변 Muzzle 선택, FireRequest·LaunchContext 스냅샷과 승인 발사 기반 SingleCycle 진행을 연결.
// - v2.124.0: Fire Command에서 Legacy Direct Launch Context를 생성해 ProjectilePool에 전달하는 LM-P0-01 기반 추가.
// - v2.123.0: Combat FX가 차량별 파괴 소켓을 해석할 수 있도록 표준 SM_Body 컴포넌트 getter를 추가.
// - v2.121.0: TS-P0-06 TargetSelect 전용 HUD 클래스·인스턴스·표시 토글·Viewport 수명 API를 추가.
// - v2.120.0: TS-P0-05 타겟 선택·수동 해제 Enhanced Input Action과 Pawn 명령 API를 추가.
// - v2.119.0: TargetPoint 기본 서브오브젝트와 Blueprint 조정 getter를 추가하고 선택 위치를 공용 TargetPoint Fallback으로 전환.
// - v2.118.0: TargetSelectComp 기본 서브오브젝트와 ICFTargetSelectable 차량 기본 계약을 추가.
// - v2.111.0: VehicleHealthComp를 추가하고 HitScan/Projectile DamageHitContext를 BaseDamage 체력 감소와 파괴 상태 결과에 연결.
// - v2.109.0: Muzzle 기준 Weapon Aim Solution 계산 경로를 추가하고 FireCommand/터렛 추적/검증이 같은 AimOrigin/AimDirection/Target을 사용하도록 준비.
// - v2.108.0: SM_Body 전용 WeaponHit / Projectile 피격 표면 구성 함수와 Damage HitContext 피격 컴포넌트 기록을 추가.
// - v2.102.0: Reticle / FireFeedback UI가 읽을 Pawn 측 FireFeedback ViewData 설정과 생성 함수를 추가.
// - v2.101.0: VehicleDebug EquipmentPresetData 기준 설명을 legacy 직접 fallback 제거 정책에 맞게 갱신.
// - v2.100.0: VehicleDebug Weapon 카테고리에 활성 EquipmentPresetData 상태를 추가하고 터렛 시각 장착도 EquipmentPresetData 우선 해석으로 전환.
// - v2.99.0: ActiveTurretMountData Debug 툴팁을 MountProfile inline fallback 제거 정책에 맞게 정리.
// - v2.98.0: MountProfile inline 터렛 시각 fallback을 런타임에서 제거하고 TurretMountData 전용 경로로 전환.
// - v2.97.0: Dummy HitScan과 Projectile Actor 충돌 결과를 같은 Damage HitContext Debug로 기록.
// - v2.96.0: VehicleDebug DamageData 표시 설명을 ProjectileData 단일 소유 정책에 맞게 정리.
// - v2.95.0: VehicleDebug Weapon 카테고리에 활성 DamageData 참조, ID, 요약, 해석 경로를 추가.
// - v2.94.0: 터렛 안정화 전 발사 정책에 맞춰 조준각 초과를 기본 발사 거부 조건에서 제외.
// - v2.93.0: Turret Pitch 메쉬의 Muzzle 소켓이 유효하면 최종 FireOrigin을 총구 기준으로 보정.
// - v2.92.0: 터렛 하드포인트 / Yaw / Pitch 필수 소켓 누락을 VehicleDebug 요약에서 MissingRequiredSocket으로 구분.
// - v2.91.0: 하드포인트 / Yaw / Pitch 소켓 부착을 SnapToTarget 기준으로 명확히 하고 소켓-루트 위치 차이 Debug를 추가.
// - v2.90.0: 터렛 장착 루트가 HardpointSlot.SocketName을 실제 차체 소켓으로 우선 사용하도록 수정.
// - v2.89.0: WeaponComp가 계산한 터렛 Yaw / Pitch 추적 각도를 시각 피벗 컴포넌트에 적용하고 Debug Snapshot에 표시.
// - v2.88.0: 터렛 시각 장착을 BaseMesh + YawPivot + PitchPivot 3단 소켓 계층으로 확장.
// - v2.87.0: 터렛 시각 장착이 TurretMountData를 우선 사용하도록 Debug Snapshot 필드를 추가.
// - v2.86.1: 터렛 시각 장착 빌드 안정화를 위해 하드포인트 슬롯 타입 전방 선언을 추가.
// - v2.86.0: 하드포인트 기반 P0 터렛 시각 장착 컴포넌트와 VehicleDebug 터렛 시각 요약을 추가.
// - v2.85.0: VehicleDebug Weapon 카테고리에 Projectile Pool 마지막 반환 요약을 추가.
// - v2.84.0: VehicleDebug Weapon 카테고리에 활성 무기 분당 발사속도를 추가하고 기존 쿨다운 초는 환산 발사 간격으로 유지.
// - v2.83.0: VehicleDebug Weapon 카테고리에 Projectile Pool 보유 여부와 전체 / 활성 / 비활성 수를 추가.
// - v2.82.0: ProjectilePoolComp를 추가하고 Projectile Actor 스폰 경로를 Pool Acquire 기반으로 전환.
// - v2.81.0: Projectile FireMode에서 공통 Projectile Actor 스폰 경로를 추가하고 Dummy HitScan fallback을 유지.
// - v2.80.0: VehicleDebug Weapon 카테고리에 Projectile Actor 스폰 준비 상태와 실행 요약을 추가.
// - v2.79.0: VehicleDebug Weapon 카테고리에 활성 ProjectileData 참조, ID, 요약을 추가.
// - v2.78.0: 활성 WeaponData의 MaxRange / CooldownSeconds를 Fire 검증과 VehicleDebug Weapon 카테고리에 연결.
// - v2.77.0: VehicleDebug Weapon 카테고리에 활성 WeaponData 참조, ID, 호환성, 요약을 추가.
// - v2.76.0: VehicleDebug Snapshot에 Weapon 카테고리를 추가해 WeaponComp 런타임과 FireOrigin 상태를 패널에서 읽을 수 있게 함.
// - v2.75.0: VehicleWeaponComp를 추가해 VehicleData MountProfiles 기반 FireOrigin을 로컬 Fire Command에 반영.
// - v2.74.0: 차체 메시 소켓에서 휠 앵커와 선택 하드포인트 위치를 함께 캡처하는 차량 레이아웃 버튼으로 확장.
// - v2.69.0: 차체 메시 소켓에서 VehicleData 휠 레이아웃을 캡처하는 에디터 전용 버튼을 추가.
// - v2.67.0: Aim Trace 디버그 변수명을 bDrawLocalAimTraceDebug / LocalAimTraceDebugDuration으로 교체.
// - v2.66.0: Aim Debug Snapshot의 ServerAimState / RepAimVisualState 명칭을 FireValidationState / AimVisualState로 교체.
// - v2.65.0: 참조가 없는 ACFVehiclePawn Fire 레거시 wrapper와 RPC 선언을 제거해 싱글플레이 Fire 경로를 단일화.
// - v2.64.0: Aim Debug 표시명과 입력/결과 툴팁을 싱글플레이 로컬 Fire Command 기준으로 정리.
// - v2.63.0: Aim Fire 입력이 싱글플레이 로컬 Fire Command 경로를 먼저 사용하도록 로컬 함수와 레거시 RPC wrapper를 분리.
// - v2.62.0: 싱글플레이 기준선에서 차량 네트워크 진단 샘플/RepMove 수신 로그/복제 등록 경로를 제거하고 Aim 디버그 표시 문구를 정리.
// - v2.61.0: 싱글플레이 전환에 맞춰 C++ 기준선에서 Actor 복제와 Replicate Movement 강제 활성화를 중단.
// - v2.60.0: 싱글플레이 전환에 맞춰 상단 기준 설명에서 CFNetSmooth 적용 전 문구를 제거.
// - v2.59.0: CFNetSmooth 적용 전 기준선 정리를 위해 차량 NetDebug/OwnerVisual/OwnerBodyVisual 실험 플래그 기본값을 False로 통일.
// Migration:
// - v2.152.0부터 InputAction_SelectWeapon은 `/Game/CarFight/Input/IA_SelectWeapon` Axis1D를 기본 로드한다. P0 키보드 매핑은 숫자 1~9가 각각 실제 1-based selectable weapon 순번 1~9를 전달한다. Mouse Wheel은 Radar Range/Zoom 예약을 보존하고 게임패드 키는 이번 slice에서 임의 지정하지 않는다.
// - v2.152.0 입력 값은 ordinal→index 변환 외 Gameplay 의미를 갖지 않으며 범위 밖 순번은 기존 RequestSelectWeaponIndex fail-closed 검증에 맡긴다. 새 WeaponGroup ID나 cycle cursor를 만들지 않는다.
// - v2.151.0 Weapon Selection은 Applied Fitting이 실제 제공한 고정 순번만 받으며 별도 입력 키나 WeaponGroup ID를 만들지 않는다. 진행 중 Launcher는 기존 WeaponChanged cancel contract로 종료하고 Ammo 예약/Action Lock 정리를 재사용한다.
// - v2.150.0 추가 friend는 WITH_DEV_AUTOMATION_TESTS에서 Heat accepted-fire 통합을 검증하기 위한 C++ 테스트 경계이며 Blueprint/런타임 공개 API를 변경하지 않는다.
// - v2.149.0부터 BeginPlay/SetupPlayerInputComponent는 TargetSelect Marker를 직접 생성하지 않는다. bShowTargetSelectHud/ShouldShowTargetSelectHud은 UISubsystem이 현재 Pawn 표시 정책으로 계속 사용하며 Legacy Class/Instance/ZOrder 필드는 자동 생성 소유권으로 사용하지 않는다.
// - v2.148.0부터 BeginPlay/SetupPlayerInputComponent는 AimReticle을 직접 생성하지 않는다. bShowAimReticle/ShouldShowAimReticle은 UISubsystem이 현재 Pawn 표시 정책으로 계속 사용하며 Legacy Class/Instance/ZOrder 필드는 자동 생성 소유권으로 사용하지 않는다.
// - v2.147.0부터 InputAction_StartActiveScan은 `/Game/CarFight/Input/IA_ActiveScan`을 기본 로드한다. P0 기본 매핑은 `V` 1개이며 Boolean + Pressed 의미다. 누르면 장비의 ActiveScanDurationSec 동안 실행되고 Sensor Runtime이 자동 종료한다.
// - InputAction_StopActiveScan은 기본 null로 유지하며 P0에서 별도 Stop 키를 만들지 않는다. RequestStopActiveScan은 장비 교체·상태 전환·향후 명시적 취소 경로에서 사용할 Gameplay command로 유지한다.
// - v2.146.0의 RequestStartActiveScan / RequestStopActiveScan은 VehicleSensorComp의 기존 StartActiveScan / StopActiveScan에만 위임하며 SensorData 적용, Runtime 초기화, Contact/Knowledge Reset을 수행하지 않는다.
// - v2.145.0부터 기존 BP_CFVehiclePawn 계열은 VehicleSensorComp를 기본 서브오브젝트로 자동 상속하며 Blueprint/Content Asset 수정이 필요하지 않다. Sensor Foundation 초기화 실패도 기존 차량 CoreReady/CombatReady 의미를 변경하지 않는다.
// - v2.144.0 추가 friend는 WITH_DEV_AUTOMATION_TESTS의 VD-P0-03 테스트 접근만 위한 C++ 경계이며 VehicleData 적용 순서, Blueprint 노출, 런타임 동작을 변경하지 않는다.
// - 선택 대상 VehicleDebug는 TargetSelectComp와 대상 Actor의 방어·내구도 상태를 읽기만 하며 선택, 추적, 피해와 재생 계산을 변경하지 않는다. 기존 Blueprint와 WBP에는 추가 작업이 필요하지 않다.
// - bVehicleRuntimeReady는 기존 Tick·Debug·Blueprint 호환을 위해 bVehicleCoreRuntimeReady와 같은 값을 유지한다. 전투 HUD와 전투 명령은 bVehicleCombatRuntimeReady를 별도로 확인한다.
// - finite Ammo는 유효 Applied Fitting Snapshot의 InitialSortieAmmoLoads에서만 초기화하며 MaximumLoadableAmmoCount를 현재 탄약으로 자동 대입하지 않는다.
// - 기존 무한탄 WeaponData 또는 Ammo 선택이 없는 Legacy Fitting은 InfiniteCompatibility로 유지하고 차량 CoreReady를 막지 않는다.
// - 유효 피팅 Snapshot 질량은 PreRegisterAllComponents의 Super 호출 전에 Movement Mass에 1회 기록하고 BeginPlay에서 실제 VehicleMesh 질량을 검증한다.
// - FittingData 미지정·초기 Invalid Snapshot은 기존 Chaos 질량과 VehicleData Weapon·Defense Legacy 경로를 유지한다.
// - Physics State 생성 뒤 다른 Target Mass는 거부하며 SetMassOverrideInKg와 Hot Recreate를 호출하지 않는다.
// - ClearGameplayInputForPause는 차량 입력과 입력 소유권만 중립화하며 진행 중 Launcher Ripple·Salvo 시퀀스를 취소하지 않는다.
// - Pause 해제 뒤에는 새 Enhanced Input 이벤트부터 다시 Drive 입력을 적용한다.
// - 신규 VehicleDefense Debug 필드는 VehicleDefenseComp 상태를 읽기만 하며 HitScan·Projectile·Launcher·Pool 피해 실행 경로를 변경하지 않는다.
// - 기존 WeaponData는 Direct / CarrierVelocityRatio 0 기본값으로 기존 AimDirection·ProjectileData.InitialSpeed 결과를 유지한다.
// - Angled·Vertical Ejection은 실제 선택 Muzzle Transform과 차량 GetVelocity 스냅샷만 발사 순간 복사하며 이후 런처 Transform을 조회하지 않는다.
// - 기존 BP_CFVehiclePawn은 LauncherComp 기본 서브오브젝트를 자동 상속하며 WeaponData 기본값 SingleCycle / 1발에서는 기존 발사 흐름과 동일하게 동작한다.
// - Ripple·Salvo 후속 발사는 첫 입력 순간 Command Target을 고정하고 각 발사 시점의 다음 유효 Muzzle에서 기존 검증·Projectile·FX 경로를 재사용한다.
// - 파괴 FX 위치 해석은 GetVehicleBodyMeshComponent가 반환하는 SM_Body의 FX_Destroyed 소켓을 우선 사용한다.
// - IA_SelectTarget은 현재 후보가 있을 때만 선택을 확정하며 후보가 없으면 기존 선택을 유지한다.
// - IA_ClearTarget은 Manual 사유로 현재 선택만 해제하고 현재 후보를 유지하며 자동 다음 타겟을 선택하지 않는다.
// - 기존 BP_CFVehiclePawn 계열은 TargetPoint, TargetSelectComp와 VehicleHealthComp 기본 서브오브젝트를 자동 상속하며 BP에 수동 컴포넌트 추가가 필요하지 않다.
// - TargetPoint는 기본 비활성이라 기존 차량은 Actor Bounds 중심을 유지하며, 차량별 BP에서 위치를 조정하고 bUseAsTargetPoint를 켠 경우에만 우선 사용한다.
// - ACFVehiclePawn은 기본적으로 Vehicle 분류, Unknown 관계, Identified 정보 단계의 ICFTargetSelectable 계약을 제공하며 파괴된 차량은 선택 불가다.
// - HitScan과 Projectile은 같은 DamageHitContext 피해 적용 경로를 사용하며, 파괴 상태가 되어도 이번 P0에서는 입력과 물리를 자동 중지하지 않는다.
// - 기존 VehicleData와 DamageData 자산은 저장하지 않아도 C++ 기본값 MaxHealth=100, BaseDamage=25, bCanDamageSelf=false를 사용한다.
// - Muzzle 소켓 또는 Reticle 목표점이 유효하지 않으면 fallback 방향으로 발사하지 않고 발사 검증에서 안전하게 거부한다.
// - WeaponNotAligned는 LastFireResult에는 기록하지만 FireFeedback의 일반 FireRejected 빨간 오버라이드로 표시하지 않는다.
// - MountProfile.DefaultEquipmentPresetData가 있으면 터렛 시각 장착과 Weapon Debug는 EquipmentPresetData를 단일 소스로 사용한다.
// - EquipmentPresetData가 없거나 내부 TurretMountData / WeaponData 참조가 비어 있으면 해당 Debug는 Missing 상태로 표시하고 MountProfile 직접 fallback은 사용하지 않는다.
// - OutOfWeaponArc는 호환용 enum 값으로 남지만, P0 터렛 발사 정책에서는 조준각 초과만으로 발사를 막지 않는다.
// - 소켓 이름이 지정되어 있는데 실제 메쉬에 없으면 기존 fallback은 유지하지만, VehicleDebug Panel 터렛 시각 요약에 MissingRequiredSocket 상태가 표시된다.
// - TurretMountData의 TurretBaseMesh가 비어 있으면 기존처럼 하드포인트 루트 기준 Yaw / Pitch 장착을 유지한다.
// - YawPivotSocketName 또는 PitchPivotSocketName이 없으면 해당 Pivot은 부모 컴포넌트 원점 기준으로 fallback된다.
// - EquipmentPresetData 내부 TurretMountData가 있으면 터렛 시각 메쉬 / 피벗 소켓 원본으로 사용하고, 비어 있으면 MountProfile inline 터렛 시각 fallback은 더 이상 사용하지 않는다.
// - 터렛 시각 메쉬가 비어 있으면 표시만 생략하고 기존 FireOrigin / Projectile / Cooldown 검증은 유지한다.
// - 터렛 회전 상태는 WeaponComp가 소유하고, Pawn은 계산된 Yaw / Pitch 값을 TurretYawPivot / TurretPitchPivot 시각 컴포넌트에 적용만 한다.
// - HardpointSlot.SocketName이 차체 소켓에 있으면 터렛 장착 루트는 소켓에 직접 붙고, 없으면 기존 LocalTransform fallback을 사용한다.
// - VehicleDebug Panel의 터렛 시각 요약에서 하드포인트 소켓 위치와 터렛 루트 위치 차이를 확인할 수 있다.
// - MuzzleSocketName이 Pitch 메쉬에 존재하면 최종 FireOrigin 위치와 방향은 해당 소켓을 우선 사용한다.
// - Muzzle 소켓이 없거나 Pitch 메쉬가 없으면 기존 하드포인트 FireOrigin fallback을 유지한다.
// - DamageData는 ProjectileData.DefaultDamageData만 직접 참조하며, HitScan / Laser도 가상 ProjectileData로 연결한다.
// - DamageData가 비어 있으면 기존 발사 / Projectile / Dummy HitScan 결과 기록은 유지하지만 VehicleHealthComp 피해 적용은 MissingDamageData로 거부한다.
// - Damage HitContext는 Debug 기록과 최소 BaseDamage 체력 적용 입력으로 사용하며 장갑 / 모듈 손상은 아직 수행하지 않는다.
// - ProjectileData가 없어도 기존 Dummy HitScan / FireOrigin / 발사 간격 검증은 유지한다.
// - Projectile Actor 스폰은 WeaponData.FireMode가 Projectile이고 ProjectileData / ProjectileActorClass가 모두 유효할 때만 실행한다.
// - Projectile Pool 확보 조건이 맞지 않거나 Pool 확보 실패 시 기존 Dummy HitScan fallback을 유지한다.
// - 기존 Fire Command 필드는 유지하고 Projectile Actor 발사 시 Direct Launch Context로 값 복사해 전달한다.
// - LM-P0-01에서는 차량 Velocity 상속, Multi-Muzzle, Ripple·Salvo와 Angled·Vertical Ejection을 적용하지 않는다.
// - WeaponData가 없으면 기존 Aim Profile MaxAimDistance와 즉시 발사 흐름을 유지한다.
// - 기존 Weapon Debug 필드는 유지하고 WeaponData 관련 필드만 뒤에 추가한다.
// - 기존 VehicleDebug Overview / Drive / Input / Camera / Aim / Runtime 카테고리는 유지하고 Weapon 카테고리만 추가한다.
// - 기존 Aim 기반 발사 흐름은 유지하고, MountProfile 해석에 성공한 경우에만 AimOrigin/AimDirection/WeaponGroupId를 덮어쓴다.
// - MountProfiles가 비어 있거나 Top_01 하드포인트가 없으면 기존 AimComp 발사 원점으로 안전하게 fallback한다.
// - Mesh_TestSUV 같은 차체 메시 소켓 기반 차량은 에디터에서 Capture Vehicle Layout From Body Sockets를 실행해 DA 값을 생성한다.
// - 하드포인트 SocketName이 비어 있거나 누락되어도 휠 레이아웃 캡처 성공 자체는 유지된다.
// - 신규 Fire 흐름은 BuildFireCommand / ValidateFireCommand / RunLocalDummyHitScan / ApplyFireResult를 기준으로 사용한다.
// - bDrawServerAimTraceDebug는 bDrawLocalAimTraceDebug로, ServerAimTraceDebugDuration은 LocalAimTraceDebugDuration으로 교체한다.
// - BP 저장값 보존을 위해 DefaultEngine.ini CoreRedirects의 PropertyRedirects를 유지한다.
// - FCFVehicleDebugAim의 ServerAimState는 FireValidationState로, RepAimVisualState는 AimVisualState로 교체한다.
// - ACFVehiclePawn의 BuildFireRequest / ValidateFireRequestOnServer / RunServerDummyHitScan / ServerRequestFire / ClientReceiveFireResult 호출은 제거하고 로컬 Fire 함수로 교체한다.
// - BP_CFVehiclePawn의 Actor Replicates/Replicate Movement도 False로 저장해 C++ 기본값과 맞춘다.
// - 멀티플레이 진단이 다시 필요하면 별도 멀티플레이 브랜치/문서에서 복구한다.
// Scope: DriveComp / WheelSyncComp / VehicleCameraComp / VehicleAimComp / VehicleWeaponComp / LauncherComp / ProjectilePoolComp / VehicleHealthComp / TargetPoint / TargetSelectComp를 소유하고 차량 런타임, 입력, 카메라 디버그 스냅샷, 로컬 Fire Command·Ripple·Salvo와 타겟 선택 위치·상태 계약을 함께 다룹니다.

#pragma once

#include "CoreMinimal.h"
#include "CFAmmoTypes.h"
#include "CFVehicleDriveComp.h"
#include "CFVehicleAimTypes.h"
#include "CFVehicleCameraTypes.h"
#include "CFDamageTypes.h"
#include "CFVehicleFireFeedbackTypes.h"
#include "CFVehicleWeaponTypes.h"
#include "CFTargetPointComp.h"
#include "CFTargetSelectable.h"
#include "Components/SlateWrapperTypes.h"
#include "Engine/EngineTypes.h"
#include "WheeledVehiclePawn.h"
#include "CFVehiclePawn.generated.h"

class UCFVehicleData;
class UCFVehicleCameraComp;
class UCFVehicleAimComp;
class UCFVehicleWeaponComp;
class UCFVehicleAmmoComp;
class UCFLauncherComp;
class UCFProjectilePoolComp;
class UCFCombatFxComp;
class UCFVehicleHealthComp;
class UCFVehicleDefenseComp;
class UCFVehicleFittingComp;
class UCFVehicleFittingData;
class UCFTargetSelectComp;
class UCFVehicleSensorComp;
class UCFEquipmentPresetData;
class UCFProjectileData;
class UCFDamageData;
class UCFTurretMountData;
class UCFWeaponData;
class UCFAimReticleWidget;
class UCFTargetSelectWidget;
class UCFWheelSyncComp;
class ACFProjectileActor;
class UChaosWheeledVehicleMovementComponent;
class UInputAction;
class UInputComponent;
class UInputMappingContext;
class USceneComponent;
class UStaticMeshComponent;
struct FCFVehicleHardpointSlot;
struct FCFProjectileLaunchContext;
struct FInputActionValue;
struct FKey;

/**
 * 차량 입력에 사용할 장치 모드입니다.
 * - Auto: 키보드/마우스와 게임패드를 모두 허용합니다.
 * - KeyboardMouseOnly: 키보드/마우스 입력만 허용합니다.
 * - GamepadOnly: 게임패드 입력만 허용합니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleInputDeviceMode : uint8
{
	Auto UMETA(DisplayName="Auto"),
	KeyboardMouseOnly UMETA(DisplayName="KeyboardMouseOnly"),
	GamepadOnly UMETA(DisplayName="GamepadOnly")
};

/**
 * 화면 디버그에 표시할 Drive 상태 문자열 포맷 모드입니다.
 * - Off: 화면 디버그를 표시하지 않습니다.
 * - SingleLine: 한 줄 요약 문자열을 표시합니다.
 * - MultiLine: 줄바꿈이 포함된 멀티라인 문자열을 표시합니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleDebugDisplayMode : uint8
{
	Off UMETA(DisplayName="Off"),
	SingleLine UMETA(DisplayName="SingleLine"),
	MultiLine UMETA(DisplayName="MultiLine")
};

/**
 * 차량 2D 이동 입력에서 마지막으로 유효했던 진행 방향 의도입니다.
 * - None: 아직 유효한 진행 방향 의도가 없습니다.
 * - Forward: 최근 유효 입력이 전진 의도였습니다.
 * - Reverse: 최근 유효 입력이 후진 의도였습니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleMoveDirectionIntent : uint8
{
	None UMETA(DisplayName="None"),
	Forward UMETA(DisplayName="Forward"),
	Reverse UMETA(DisplayName="Reverse")
};

/**
 * 차량 2D 이동 입력이 해석된 영역입니다.
 * - None: 아직 어떤 영역으로도 해석되지 않았습니다.
 * - Throttle: 전진 쓰로틀 영역입니다.
 * - Reverse: 후진/브레이크 해석 영역입니다.
 * - Black: 방향 유지 완충용 검은 영역입니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleMoveZone : uint8
{
	None UMETA(DisplayName="None"),
	Throttle UMETA(DisplayName="Throttle"),
	Reverse UMETA(DisplayName="Reverse"),
	Black UMETA(DisplayName="Black")
};

/**
 * 현재 프레임 기준 차량 입력 적용의 주도권을 가진 경로입니다.
 * - None: 아직 어떤 입력 경로도 주도권을 가지지 않습니다.
 * - VehicleMove2D: `IA_VehicleMove` 기반 2D 신규 조작이 주도권을 가집니다.
 * - LegacyAxis: 기존 `Throttle / Brake / Steering` 축 입력이 주도권을 가집니다.
 */
UENUM(BlueprintType)
enum class ECFVehicleInputOwnership : uint8
{
	None UMETA(DisplayName="None"),
	VehicleMove2D UMETA(DisplayName="VehicleMove2D"),
	LegacyAxis UMETA(DisplayName="LegacyAxis")
};

/**
 * 차량 2D 이동 입력 해석에 사용할 각도 설정입니다.
 */

USTRUCT(BlueprintType)
struct FCFVehicleMoveInputConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", ClampMax="360.0", DisplayName="쓰로틀 시작 각도 (ThrottleStartAngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 쓰로틀 영역 시작 각도입니다."))
	float ThrottleStartAngleDeg = 260.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", ClampMax="360.0", DisplayName="쓰로틀 종료 각도 (ThrottleEndAngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 쓰로틀 영역 종료 각도입니다."))
	float ThrottleEndAngleDeg = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", ClampMax="360.0", DisplayName="후진 시작 각도 (ReverseStartAngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 후진 영역 시작 각도입니다."))
	float ReverseStartAngleDeg = 135.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", ClampMax="360.0", DisplayName="후진 종료 각도 (ReverseEndAngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 후진 영역 종료 각도입니다."))
	float ReverseEndAngleDeg = 225.0f;
};

/**
 * 차량 2D 이동 입력의 최신 해석 결과입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleMoveInputResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="원본 이동 입력 (RawMoveInput)", ToolTip="차량 이동 Input Action에서 들어온 원본 2D 입력 벡터입니다."))
	FVector2D RawMoveInput = FVector2D::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="입력 강도 (Magnitude)", ToolTip="차량 이동 입력 벡터의 반지름 기반 강도입니다."))
	float Magnitude = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="입력 각도 (AngleDeg)", ToolTip="위가 0도, 시계 방향 증가 기준의 차량 이동 입력 각도입니다."))
	float AngleDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="해석 영역 (ResolvedZone)", ToolTip="현재 차량 이동 입력이 해석된 영역입니다."))
	ECFVehicleMoveZone ResolvedZone = ECFVehicleMoveZone::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="해석 방향 의도 (ResolvedDirectionIntent)", ToolTip="현재 차량 이동 입력이 해석한 진행 방향 의도입니다."))
	ECFVehicleMoveDirectionIntent ResolvedDirectionIntent = ECFVehicleMoveDirectionIntent::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="조향 출력값 (SteeringValue)", ToolTip="현재 프레임에 DriveComp로 전달할 조향 출력값입니다."))
	float SteeringValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="스로틀 출력값 (ThrottleValue)", ToolTip="현재 프레임에 DriveComp로 전달할 스로틀 출력값입니다."))
	float ThrottleValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="브레이크 출력값 (BrakeValue)", ToolTip="현재 프레임에 DriveComp로 전달할 브레이크 출력값입니다."))
	float BrakeValue = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="검은 영역 유지 사용 여부 (bUsedBlackZoneHold)", ToolTip="현재 프레임에 검은 영역 방향 유지 정책이 적용되었는지 여부입니다."))
	bool bUsedBlackZoneHold = false;
};

/**
 * VehicleDebug HUD용 핵심 요약 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugOverview
{
	GENERATED_BODY()

	// [v2.14.1] 현재 Pawn 런타임 초기화 완료 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="런타임 준비 완료 여부 (bRuntimeReady)", ToolTip="현재 Pawn 런타임 초기화가 완료되었는지 여부입니다."))
	bool bRuntimeReady = false;

	// [v2.14.1] HUD 요약용 현재 Drive 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="현재 Drive 상태 (CurrentDriveState)", ToolTip="HUD 요약에 사용할 현재 VehicleDriveComp 기준 Drive 상태입니다."))
	ECFVehicleDriveState CurrentDriveState = ECFVehicleDriveState::Disabled;

	// [v2.14.1] HUD 요약용 현재 전체 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="현재 속도 km/h (SpeedKmh)", ToolTip="HUD 요약에 사용할 현재 차량 전체 속도 km/h 값입니다."))
	float SpeedKmh = 0.0f;

	// [v2.14.1] HUD 요약용 현재 전후 방향 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="전후 방향 속도 km/h (ForwardSpeedKmh)", ToolTip="HUD 요약에 사용할 현재 차량 전후 방향 속도 km/h 값입니다."))
	float ForwardSpeedKmh = 0.0f;

	// [v2.14.1] HUD 요약용 현재 입력 장치 모드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="입력 장치 모드 (DeviceMode)", ToolTip="HUD 요약에 사용할 현재 차량 입력 장치 모드입니다."))
	ECFVehicleInputDeviceMode DeviceMode = ECFVehicleInputDeviceMode::Auto;

	// [v2.14.1] HUD 요약용 현재 입력 주도권입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="입력 주도권 (InputOwner)", ToolTip="HUD 요약에 사용할 현재 차량 입력 주도권입니다."))
	ECFVehicleInputOwnership InputOwner = ECFVehicleInputOwnership::None;

	// [v2.14.1] HUD 요약용 마지막 상태 전이 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Overview", meta=(DisplayName="마지막 전이 요약 (LastTransitionShortText)", ToolTip="HUD 요약에 사용할 마지막 Drive 상태 전이 요약 문자열입니다."))
	FString LastTransitionShortText = TEXT("DriveStateTransition: None");
};

/**
 * VehicleDebug 주행 상세 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugDrive
{
	GENERATED_BODY()

	// [v2.14.1] 현재 Drive 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="현재 Drive 상태 (CurrentDriveState)", ToolTip="현재 VehicleDriveComp 기준 Drive 상태입니다."))
	ECFVehicleDriveState CurrentDriveState = ECFVehicleDriveState::Disabled;

	// [v2.14.1] 이전 프레임 Drive 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="이전 Drive 상태 (PreviousDriveState)", ToolTip="이전 프레임 기준 VehicleDriveComp의 Drive 상태입니다."))
	ECFVehicleDriveState PreviousDriveState = ECFVehicleDriveState::Disabled;

	// [v2.14.1] 이번 프레임 상태 전이 발생 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="이번 프레임 상태 변경 여부 (bDriveStateChangedThisFrame)", ToolTip="이번 프레임에 Drive 상태가 변경되었는지 여부입니다."))
	bool bDriveStateChangedThisFrame = false;

	// [v2.14.1] 현재 전체 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="현재 속도 km/h (SpeedKmh)", ToolTip="현재 차량 전체 속도 km/h 값입니다."))
	float SpeedKmh = 0.0f;

	// [v2.14.1] 현재 전후 방향 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="전후 방향 속도 km/h (ForwardSpeedKmh)", ToolTip="현재 차량 전후 방향 속도 km/h 값입니다."))
	float ForwardSpeedKmh = 0.0f;

	// [v2.14.1] 현재 스로틀 입력값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="스로틀 입력 (Throttle)", ToolTip="현재 VehicleDriveComp에 적용된 스로틀 입력값입니다."))
	float Throttle = 0.0f;

	// [v2.14.1] 현재 브레이크 입력값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="브레이크 입력 (Brake)", ToolTip="현재 VehicleDriveComp에 적용된 브레이크 입력값입니다."))
	float Brake = 0.0f;

	// [v2.14.1] 현재 조향 입력값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="조향 입력 (Steering)", ToolTip="현재 VehicleDriveComp에 적용된 조향 입력값입니다."))
	float Steering = 0.0f;

	// [v2.14.1] 현재 핸드브레이크 입력 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="핸드브레이크 입력 (bHandbrake)", ToolTip="현재 VehicleDriveComp에 적용된 핸드브레이크 입력 상태입니다."))
	bool bHandbrake = false;

	// [v2.14.1] 마지막 Drive 상태 전이 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="상태 전이 요약 (DriveStateTransitionSummary)", ToolTip="마지막 Drive 상태 전이 요약 문자열입니다."))
	FString DriveStateTransitionSummary = TEXT("DriveStateTransition: None");

	// [v2.14.1] Drive 컴포넌트 최신 스냅샷입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Drive", meta=(DisplayName="Drive 상태 스냅샷 (DriveStateSnapshot)", ToolTip="VehicleDriveComp가 계산한 최신 Drive 상태 스냅샷입니다."))
	FCFVehicleDriveStateSnapshot DriveStateSnapshot;
};

/**
 * VehicleDebug 입력 해석 상세 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugInput
{
	GENERATED_BODY()

	// [v2.14.1] 현재 입력 장치 모드입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="입력 장치 모드 (DeviceMode)", ToolTip="현재 차량 입력 장치 모드입니다."))
	ECFVehicleInputDeviceMode DeviceMode = ECFVehicleInputDeviceMode::Auto;

	// [v2.14.1] 현재 입력 주도권입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="입력 주도권 (InputOwner)", ToolTip="현재 차량 입력 적용의 주도권을 가진 경로입니다."))
	ECFVehicleInputOwnership InputOwner = ECFVehicleInputOwnership::None;

	// [v2.14.1] 현재 해석된 이동 입력 영역입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="해석 영역 (MoveZone)", ToolTip="현재 차량 2D 이동 입력이 해석된 영역입니다."))
	ECFVehicleMoveZone MoveZone = ECFVehicleMoveZone::None;

	// [v2.14.1] 현재 해석된 진행 방향 의도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="진행 방향 의도 (MoveIntent)", ToolTip="현재 차량 2D 이동 입력이 해석한 진행 방향 의도입니다."))
	ECFVehicleMoveDirectionIntent MoveIntent = ECFVehicleMoveDirectionIntent::None;

	// [v2.14.1] 원본 2D 이동 입력값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="원본 이동 입력 (MoveRaw)", ToolTip="차량 이동 Input Action에서 들어온 최신 원본 2D 입력 벡터입니다."))
	FVector2D MoveRaw = FVector2D::ZeroVector;

	// [v2.14.1] 현재 이동 입력 강도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="입력 강도 (MoveMagnitude)", ToolTip="차량 이동 입력 벡터의 최신 강도입니다."))
	float MoveMagnitude = 0.0f;

	// [v2.14.1] 현재 이동 입력 각도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="입력 각도 (MoveAngle)", ToolTip="위가 0도, 시계 방향 증가 기준의 차량 이동 입력 각도입니다."))
	float MoveAngle = 0.0f;

	// [v2.14.1] 검은 영역 방향 유지 정책 적용 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input", meta=(DisplayName="검은 영역 유지 사용 여부 (bUsedBlackZoneHold)", ToolTip="현재 프레임에 검은 영역 방향 유지 정책이 적용되었는지 여부입니다."))
		bool bUsedBlackZoneHold = false;

	// [v2.8.0] 게임패드 2D 이동 입력 방향에서 계산한 목표 조향값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input")
	float TargetSteeringInput = 0.0f;

	// [v2.8.0] 실제 DriveComp에 전달 중인 제한 속도 적용 조향값입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input")
	float CurrentSteeringInput = 0.0f;

	// [v2.8.0] 목표 조향을 따라갈 때 사용한 마지막 조향 변화 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input")
	float LastSteeringTurnRate = 0.0f;

	// [v2.8.0] 중립 복귀 중 사용한 마지막 속도 기반 조향 복귀 속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input")
	float LastSteeringReturnRate = 0.0f;

	// [v2.8.0] 현재 실제 조향값이 중립 0을 향해 복귀 중인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Input")
	bool bSteeringReturningToCenter = false;
};

/**
 * VehicleDebug 런타임 진단 카테고리입니다.
 */

USTRUCT(BlueprintType)
struct FCFVehicleDebugRuntime
{
	GENERATED_BODY()

	// [v2.14.1] 현재 Pawn 런타임 초기화 완료 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Runtime", meta=(DisplayName="런타임 준비 완료 여부 (bRuntimeReady)", ToolTip="현재 Pawn 런타임 초기화가 완료되었는지 여부입니다."))
	bool bRuntimeReady = false;

	// [v2.14.1] Drive 컴포넌트 보유 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Runtime", meta=(DisplayName="Drive 컴포넌트 보유 여부 (bHasDriveComponent)", ToolTip="현재 VehicleDriveComp가 유효한지 여부입니다."))
	bool bHasDriveComponent = false;

	// [v2.14.1] WheelSync 컴포넌트 보유 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Runtime", meta=(DisplayName="WheelSync 컴포넌트 보유 여부 (bHasWheelSyncComponent)", ToolTip="현재 WheelSyncComp가 유효한지 여부입니다."))
	bool bHasWheelSyncComponent = false;

	// [v2.14.1] 마지막 런타임 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Runtime", meta=(DisplayName="런타임 요약 (RuntimeSummary)", ToolTip="마지막 차량 런타임 초기화/적용 요약 문자열입니다."))
	FString RuntimeSummary = TEXT("NotInitialized");

	// [v2.14.1] 마지막 초기화 시도 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Runtime", meta=(DisplayName="마지막 초기화 시도 요약 (LastInitAttemptSummary)", ToolTip="최근 차량 런타임 초기화 시도 기준 요약 문자열입니다."))
	FString LastInitAttemptSummary = TEXT("NotInitialized");

	// [v2.14.1] 마지막 검증 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Runtime", meta=(DisplayName="마지막 검증 요약 (LastValidationSummary)", ToolTip="최근 차량 런타임 검증 기준 요약 문자열입니다."))
	FString LastValidationSummary = TEXT("NotInitialized");
};

/**
 * VehicleDebug 카메라 상세 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugCamera
{
	GENERATED_BODY()

	// [v2.7.0] VehicleCameraComp 보유 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Camera", meta=(DisplayName="VehicleCameraComp 보유 여부 (bHasVehicleCameraComponent)", ToolTip="현재 Pawn이 VehicleCameraComp를 보유하고 있는지 여부입니다."))
	bool bHasVehicleCameraComponent = false;

	// [v2.7.0] VehicleCameraComp가 제공하는 원본 카메라 런타임 스냅샷입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Camera", meta=(DisplayName="카메라 런타임 상태 (CameraRuntimeState)", ToolTip="VehicleCameraComp가 계산한 현재 카메라 런타임 스냅샷입니다."))
	FCFVehicleCameraRuntimeState CameraRuntimeState;

	// [v2.7.0] 목표 Arm 길이 대비 실제 해결 Arm 길이 비율입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Camera", meta=(DisplayName="충돌 압축 비율 (CollisionCompressionRatio)", ToolTip="DesiredArmLength 대비 SolvedArmLength 비율입니다. 1에 가까울수록 압축이 적고, 0에 가까울수록 크게 눌린 상태입니다."))
	float CollisionCompressionRatio = 1.0f;

	// [v2.7.0] 카메라가 충돌 등으로 의미 있게 압축된 상태인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Camera", meta=(DisplayName="충돌 압축 상태 (bCameraCompressedByCollision)", ToolTip="True이면 현재 카메라가 목표 Arm 길이보다 의미 있게 짧아진 상태입니다."))
	bool bCameraCompressedByCollision = false;
};

/**
 * VehicleDebug 조준 상세 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugAim
{
	GENERATED_BODY()

	// [v2.16.0] VehicleAimComp 보유 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="VehicleAimComp 보유 여부 (bHasVehicleAimComponent)", ToolTip="현재 Pawn이 VehicleAimComp를 보유하고 있는지 여부입니다."))
	bool bHasVehicleAimComponent = false;

	// [v2.16.0] Aim 런타임 준비 완료 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="Aim 런타임 준비 완료 여부 (bAimRuntimeReady)", ToolTip="VehicleAimComp가 Owner Pawn과 VehicleCameraComp 참조를 준비했는지 여부입니다."))
	bool bAimRuntimeReady = false;

	// [v2.64.0] 로컬 플레이어 기준 Aim 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="로컬 Aim 상태 (LocalAimState)", ToolTip="VehicleAimComp가 계산한 로컬 플레이어 기준 Aim 상태입니다."))
	FCFVehicleLocalAimState LocalAimState;

	// [v2.66.0] 로컬 발사 검증 기준 Aim 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="발사 검증 상태 (FireValidationState)", ToolTip="VehicleAimComp가 보유한 로컬 발사 검증 기준 Aim 상태입니다."))
	FCFVehicleFireValidationState FireValidationState;

	// [v2.66.0] 로컬 디버그와 발사 결과 표시용 Aim 시각 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="Aim 시각 상태 (AimVisualState)", ToolTip="VehicleAimComp가 보유한 로컬 디버그와 발사 결과 표시용 Aim 시각 상태입니다."))
	FCFVehicleAimVisualState AimVisualState;

	// [v2.16.0] 로컬 Reticle 표시 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="Reticle 상태 (ReticleState)", ToolTip="VehicleAimComp가 계산한 현재 로컬 Reticle 표시 상태입니다."))
	ECFVehicleReticleState ReticleState = ECFVehicleReticleState::Hidden;

	// [v2.16.0] Aim 런타임 초기화 또는 갱신 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="Aim 런타임 요약 (AimRuntimeSummary)", ToolTip="VehicleAimComp의 마지막 런타임 초기화 또는 갱신 요약 문자열입니다."))
	FString AimRuntimeSummary = TEXT("AimRuntime: Missing");

	// [v2.64.0] 마지막 발사 명령 디버그 캐시입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="마지막 발사 명령 (LastFireRequest)", ToolTip="Pawn이 마지막으로 생성하거나 레거시 wrapper에서 받은 로컬 발사 명령 디버그 캐시입니다."))
	FCFVehicleFireRequest LastFireRequest;

	// [v2.64.0] 마지막 발사 결과 디버그 캐시입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Aim", meta=(DisplayName="마지막 발사 결과 (LastFireResult)", ToolTip="로컬 발사 검증 뒤 Pawn이 마지막으로 적용한 발사 결과 디버그 캐시입니다."))
	FCFVehicleFireResult LastFireResult;
};

/**
 * VehicleDebug 선택 대상 상세 카테고리입니다.
 * 선택된 Actor의 TargetSelect 표시 정보와 방어·내구도 컴포넌트 상태를 읽기 전용으로 보관합니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugTarget
{
	GENERATED_BODY()

	// [v2.138.0] 현재 Pawn이 TargetSelectComp를 보유하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="TargetSelectComp 보유 여부 (bHasTargetSelectComponent)", ToolTip="현재 Pawn이 선택 대상을 관리하는 TargetSelectComp를 보유하는지 표시합니다."))
	bool bHasTargetSelectComponent = false;

	// [v2.138.0] TargetSelectComp에 선택 대상 기록이 존재하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="선택 대상 존재 여부 (bHasSelectedTarget)", ToolTip="현재 TargetSelectComp에 선택 대상 기록이 존재하는지 표시합니다."))
	bool bHasSelectedTarget = false;

	// [v2.138.0] 현재 선택 대상 Actor가 런타임에서 유효한지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="선택 대상 유효 여부 (bSelectedTargetValid)", ToolTip="현재 선택 대상 Actor가 제거되지 않았고 선택 가능한 유효 상태인지 표시합니다."))
	bool bSelectedTargetValid = false;

	// [v2.138.0] 현재 선택 대상 Actor의 런타임 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="선택 대상 Actor 이름 (SelectedTargetActorName)", ToolTip="현재 선택 대상 Actor 인스턴스의 런타임 이름입니다."))
	FString SelectedTargetActorName = TEXT("None");

	// [v2.138.0] 현재 선택 대상의 안정 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="선택 대상 ID (SelectedTargetId)", ToolTip="TargetSelect 표시 정보에서 읽은 현재 선택 대상의 안정 식별자입니다."))
	FName SelectedTargetId = NAME_None;

	// [v2.138.0] 현재 선택 대상의 사용자 표시 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="선택 대상 표시 이름 (SelectedTargetDisplayName)", ToolTip="TargetSelect 표시 정보에서 읽은 현재 선택 대상의 사용자 표시 이름입니다."))
	FText SelectedTargetDisplayName;

	// [v2.138.0] 현재 선택 대상의 추적 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="선택 대상 추적 상태 (SelectedTargetTrackState)", ToolTip="현재 선택 대상이 가시, 가림, 추정 추적 또는 신호 손실 중인지 표시합니다."))
	ECFTargetTrackState SelectedTargetTrackState = ECFTargetTrackState::Invalid;

	// [v2.138.0] 선택 대상이 VehicleDefenseComp를 보유하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 VehicleDefenseComp 보유 여부 (bHasSelectedTargetDefenseComponent)", ToolTip="현재 선택 대상 Actor가 Shield와 방향별 Armor를 관리하는 VehicleDefenseComp를 보유하는지 표시합니다."))
	bool bHasSelectedTargetDefenseComponent = false;

	// [v2.138.0] 선택 대상의 DefenseData 초기화 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 방어 초기화 여부 (bSelectedTargetDefenseInitialized)", ToolTip="현재 선택 대상의 VehicleDefenseComp가 유효한 DefenseData로 초기화됐는지 표시합니다."))
	bool bSelectedTargetDefenseInitialized = false;

	// [v2.138.0] 선택 대상이 Legacy Integrity 직접 피해 경로를 사용하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 Legacy 방어 Fallback 여부 (bSelectedTargetUsingLegacyDefenseFallback)", ToolTip="True이면 선택 대상이 DefenseData 없이 기존 Integrity 직접 피해 경로를 사용합니다."))
	bool bSelectedTargetUsingLegacyDefenseFallback = false;

	// [v2.138.0] 선택 대상의 현재 Shield, 6방향 Armor와 재생 상태 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 방어 상태 요약 (SelectedTargetDefenseSummary)", ToolTip="선택 대상 VehicleDefenseComp의 현재 Shield, 방향별 Armor, 재생 여부와 남은 재생 지연을 읽기 전용으로 표시합니다."))
	FString SelectedTargetDefenseSummary = TEXT("선택 대상 VehicleDefenseComp 없음");

	// [v2.138.0] 선택 대상에 마지막 전체 방어 피해 결과가 존재하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 마지막 방어 결과 존재 여부 (bHasSelectedTargetLastDamageResult)", ToolTip="선택 대상 VehicleDefenseComp에 마지막 Shield, Armor, 관통과 Integrity 피해 결과가 기록됐는지 표시합니다."))
	bool bHasSelectedTargetLastDamageResult = false;

	// [v2.138.0] 선택 대상의 마지막 전체 방어 피해 결과 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 마지막 방어 결과 요약 (SelectedTargetLastDamageResultSummary)", ToolTip="선택 대상에 마지막으로 적용된 방향, Shield 흡수, Armor 흡수·관통과 Integrity 결과를 표시합니다."))
	FString SelectedTargetLastDamageResultSummary = TEXT("선택 대상 방어 피해 기록 없음");

	// [v2.138.0] 선택 대상이 VehicleHealthComp를 보유하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 VehicleHealthComp 보유 여부 (bHasSelectedTargetHealthComponent)", ToolTip="현재 선택 대상 Actor가 차량 내구도를 관리하는 VehicleHealthComp를 보유하는지 표시합니다."))
	bool bHasSelectedTargetHealthComponent = false;

	// [v2.138.0] 선택 대상의 VehicleHealthComp 초기화 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 내구도 초기화 여부 (bSelectedTargetHealthInitialized)", ToolTip="현재 선택 대상의 최대·현재 Integrity가 초기화됐는지 표시합니다."))
	bool bSelectedTargetHealthInitialized = false;

	// [v2.138.0] 선택 대상의 현재 차량 내구도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 현재 Integrity (SelectedTargetCurrentIntegrity)", ToolTip="현재 선택 대상 VehicleHealthComp의 남은 Vehicle Integrity입니다."))
	float SelectedTargetCurrentIntegrity = 0.0f;

	// [v2.138.0] 선택 대상의 최대 차량 내구도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 최대 Integrity (SelectedTargetMaximumIntegrity)", ToolTip="현재 선택 대상 VehicleHealthComp의 최대 Vehicle Integrity입니다."))
	float SelectedTargetMaximumIntegrity = 0.0f;

	// [v2.138.0] 선택 대상이 파괴 상태인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 파괴 여부 (bSelectedTargetDestroyed)", ToolTip="현재 선택 대상 VehicleHealthComp가 파괴 상태로 전환됐는지 표시합니다."))
	bool bSelectedTargetDestroyed = false;

	// [v2.138.0] 선택 대상 약한 참조와 해제·추적 상태 수명 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Target", meta=(DisplayName="대상 수명 요약 (SelectedTargetLifetimeSummary)", ToolTip="TargetSelectComp가 기록한 선택 대상 유효성, 추적 상태와 마지막 해제 사유 요약입니다."))
	FString SelectedTargetLifetimeSummary = TEXT("SelectedTarget: None");
};

/**
 * VehicleDebug 무기 상세 카테고리입니다.
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugWeapon
{
	GENERATED_BODY()

	// [v2.76.0] VehicleWeaponComp 보유 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="VehicleWeaponComp 보유 여부 (bHasVehicleWeaponComponent)", ToolTip="현재 Pawn이 VehicleWeaponComp를 보유하고 있는지 여부입니다."))
	bool bHasVehicleWeaponComponent = false;

	// [v2.76.0] Weapon 런타임 준비 완료 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="Weapon 런타임 준비 완료 여부 (bWeaponRuntimeReady)", ToolTip="VehicleWeaponComp가 활성 장착 프로파일과 하드포인트 슬롯을 찾았는지 여부입니다."))
	bool bWeaponRuntimeReady = false;

	// [v2.76.0] WeaponComp가 우선 사용할 장착 프로파일 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 장착 프로파일 ID (ActiveMountProfileId)", ToolTip="VehicleWeaponComp가 현재 우선 사용하는 장착 프로파일 ID입니다."))
	FName ActiveMountProfileId = NAME_None;

	// [v2.76.0] 마지막으로 계산된 실제 발사 원점입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 발사 원점 (LastFireOrigin)", ToolTip="VehicleWeaponComp가 마지막으로 계산한 실제 발사 위치와 방향입니다."))
	FCFVehicleFireOrigin LastFireOrigin;

	// [v2.76.0] WeaponComp의 마지막 런타임 초기화 또는 FireOrigin 계산 요약 문자열입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="Weapon 런타임 요약 (LastWeaponRuntimeSummary)", ToolTip="VehicleWeaponComp의 마지막 런타임 초기화 또는 FireOrigin 계산 결과 요약 문자열입니다."))
	FString LastWeaponRuntimeSummary = TEXT("WeaponRuntime: Missing");

	// [v2.101.0] 현재 활성 장착 프로파일에서 해석한 EquipmentPresetData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 EquipmentPresetData (ActiveEquipmentPresetData)", ToolTip="현재 활성 장착 프로파일에서 해석한 장비 프리셋 DataAsset입니다. 비어 있으면 장비 데이터는 Missing 상태입니다."))
	TObjectPtr<UCFEquipmentPresetData> ActiveEquipmentPresetData = nullptr;

	// [v2.100.0] 현재 활성 EquipmentPresetData가 지정되어 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 EquipmentPresetData 지정 여부 (bActiveEquipmentPresetDataAssigned)", ToolTip="현재 활성 장착 프로파일에 장비 프리셋 DataAsset이 지정되어 있는지 여부입니다."))
	bool bActiveEquipmentPresetDataAssigned = false;

	// [v2.100.0] 현재 활성 EquipmentPresetData가 장착 프로파일과 호환되는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 EquipmentPresetData 호환 여부 (bActiveEquipmentPresetDataCompatible)", ToolTip="현재 활성 EquipmentPresetData가 장착 타입과 크기 제한을 통과했는지 여부입니다."))
	bool bActiveEquipmentPresetDataCompatible = false;

	// [v2.100.0] 현재 활성 EquipmentPresetData의 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 장비 프리셋 ID (ActiveEquipmentPresetId)", ToolTip="현재 활성 EquipmentPresetData의 EquipmentId입니다. EquipmentPresetData가 비어 있으면 None입니다."))
	FName ActiveEquipmentPresetId = NAME_None;

	// [v2.100.0] 현재 활성 EquipmentPresetData 핵심 값 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 EquipmentPresetData 요약 (ActiveEquipmentPresetSummary)", ToolTip="현재 활성 EquipmentPresetData의 장비 조합 요약 문자열입니다."))
	FString ActiveEquipmentPresetSummary = TEXT("EquipmentPresetData: MissingOptional");

	// [v2.101.0] 현재 활성 EquipmentPresetData에서 해석한 TurretMountData이며, 비어 있으면 Missing 상태로 표시합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 TurretMountData (ActiveTurretMountData)", ToolTip="현재 활성 EquipmentPresetData에서 해석한 터렛 마운트 DataAsset입니다. 비어 있으면 터렛 시각 / 조준 추적은 Missing으로 표시되고 MountProfile 직접 fallback은 사용하지 않습니다."))
	TObjectPtr<UCFTurretMountData> ActiveTurretMountData = nullptr;

	// [v2.101.0] 현재 활성 TurretMountData가 EquipmentPresetData에서 해석되었는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 TurretMountData 지정 여부 (bActiveTurretMountDataAssigned)", ToolTip="현재 활성 장착 프로파일에서 터렛 마운트 DataAsset이 EquipmentPresetData로 해석되었는지 여부입니다."))
	bool bActiveTurretMountDataAssigned = false;

	// [v2.87.0] 현재 활성 TurretMountData의 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 터렛 마운트 ID (ActiveTurretMountId)", ToolTip="현재 활성 TurretMountData의 TurretMountId입니다. TurretMountData가 비어 있으면 None입니다."))
	FName ActiveTurretMountId = NAME_None;

	// [v2.87.0] 현재 활성 TurretMountData 핵심 값 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 터렛 마운트 요약 (ActiveTurretMountSummary)", ToolTip="현재 활성 TurretMountData의 시각 메쉬, 피벗 소켓, 회전 한계, 회전 속도 요약입니다."))
	FString ActiveTurretMountSummary = TEXT("TurretMountData: NotInitialized");

	// [v2.86.0] 현재 활성 장착 프로파일의 터렛 시각 메쉬가 차량에 붙었는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 시각 장착 여부 (bTurretVisualAttached)", ToolTip="현재 활성 장착 프로파일의 터렛 시각 메쉬가 하드포인트 기준으로 차량에 붙었는지 여부입니다."))
	bool bTurretVisualAttached = false;

	// [v2.88.0] 현재 터렛 시각 장착 상태 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 시각 요약 (TurretVisualSummary)", ToolTip="터렛 Base/Yaw/Pitch 메쉬 연결, 하드포인트, 피벗 소켓 적용 상태를 요약한 문자열입니다."))
	FString TurretVisualSummary = TEXT("TurretVisual: NotInitialized");

	// [v2.89.0] 현재 터렛 조준 추적 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 조준 상태 (TurretState)", ToolTip="WeaponComp가 계산한 터렛 목표 Yaw/Pitch와 현재 추적 Yaw/Pitch 상태입니다."))
	FCFVehicleTurretState TurretState;

	// [v2.89.0] 현재 터렛 조준 추적 계산 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 런타임 요약 (TurretRuntimeSummary)", ToolTip="WeaponComp가 마지막으로 계산한 터렛 조준 추적 상태 요약입니다."))
	FString TurretRuntimeSummary = TEXT("TurretRuntime: NotInitialized");

	// [v2.88.0] 현재 터렛 Base 메쉬 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 Base 메쉬 이름 (TurretBaseMeshName)", ToolTip="현재 하드포인트에 고정된 터렛 받침/Base 메쉬 이름입니다. 비어 있으면 None입니다."))
	FName TurretBaseMeshName = NAME_None;

	// [v2.88.0] 현재 터렛 Yaw 메쉬 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 Yaw 메쉬 이름 (TurretYawMeshName)", ToolTip="현재 YawPivot 아래에 붙은 터렛 회전부/Yaw 메쉬 이름입니다. 비어 있으면 None입니다."))
	FName TurretYawMeshName = NAME_None;

	// [v2.88.0] 현재 터렛 Pitch 메쉬 이름입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="터렛 Pitch 메쉬 이름 (TurretPitchMeshName)", ToolTip="현재 PitchPivot 아래에 붙은 터렛 상부/포신/Pitch 메쉬 이름입니다. 비어 있으면 None입니다."))
	FName TurretPitchMeshName = NAME_None;

	// [v2.77.0] 현재 활성 장착 프로파일에 연결된 WeaponData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 WeaponData (ActiveWeaponData)", ToolTip="현재 활성 장착 프로파일에 연결된 WeaponData입니다."))
	TObjectPtr<UCFWeaponData> ActiveWeaponData = nullptr;

	// [v2.77.0] 현재 활성 WeaponData의 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 무기 ID (ActiveWeaponId)", ToolTip="현재 활성 WeaponData의 WeaponId입니다. WeaponData가 비어 있으면 None입니다."))
	FName ActiveWeaponId = NAME_None;

	// [v2.77.0] 현재 활성 WeaponData가 장착 프로파일과 호환되는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 WeaponData 호환 여부 (bActiveWeaponDataCompatible)", ToolTip="현재 활성 WeaponData가 장착 타입과 크기 제한을 통과했는지 여부입니다."))
	bool bActiveWeaponDataCompatible = false;

	// [v2.77.0] 현재 활성 WeaponData 핵심 값 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 WeaponData 요약 (ActiveWeaponSummary)", ToolTip="현재 활성 WeaponData의 핵심 전투 데이터 요약 문자열입니다."))
	FString ActiveWeaponSummary = TEXT("WeaponData: MissingOptional");

	// [v2.79.0] 현재 활성 WeaponData에 연결된 ProjectileData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 ProjectileData (ActiveProjectileData)", ToolTip="현재 활성 WeaponData에 연결된 ProjectileData입니다. 비어 있으면 Dummy HitScan fallback을 유지합니다."))
	TObjectPtr<UCFProjectileData> ActiveProjectileData = nullptr;

	// [v2.79.0] 현재 활성 ProjectileData의 식별자입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 발사체 ID (ActiveProjectileId)", ToolTip="현재 활성 ProjectileData의 ProjectileId입니다. ProjectileData가 비어 있으면 None입니다."))
	FName ActiveProjectileId = NAME_None;

	// [v2.79.0] 현재 활성 ProjectileData 핵심 값 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 ProjectileData 요약 (ActiveProjectileSummary)", ToolTip="현재 활성 ProjectileData의 핵심 발사체 데이터 요약 문자열입니다."))
	FString ActiveProjectileSummary = TEXT("ProjectileData: MissingOptional");

	// [v2.80.0] 현재 활성 ProjectileData가 Projectile Actor 전환 후보인지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 Projectile 스폰 준비 여부 (bActiveProjectileSpawnReady)", ToolTip="활성 ProjectileData와 ProjectileActorClass가 모두 유효해 Projectile Actor Pool 확보 경로를 사용할 수 있는지 여부입니다."))
	bool bActiveProjectileSpawnReady = false;

	// [v2.80.0] 현재 활성 Projectile 실행 경로 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 Projectile 실행 요약 (ActiveProjectileExecutionSummary)", ToolTip="Dummy HitScan 유지 또는 Projectile 전환 준비 상태를 설명하는 요약 문자열입니다."))
	FString ActiveProjectileExecutionSummary = TEXT("ProjectileExecution: DummyHitScanFallback");

	// [v2.96.0] 현재 활성 ProjectileData에서 해석한 DamageData입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 DamageData (ActiveDamageData)", ToolTip="현재 활성 ProjectileData에서 해석한 DamageData입니다. 비어 있으면 ProjectileData.DamageProfileId fallback 또는 미지정 상태만 표시합니다."))
	TObjectPtr<UCFDamageData> ActiveDamageData = nullptr;

	// [v2.96.0] 현재 활성 DamageData의 DamageId 또는 ProjectileData fallback DamageProfileId입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 피해 ID (ActiveDamageId)", ToolTip="현재 활성 DamageData의 DamageId 또는 fallback DamageProfileId입니다."))
	FName ActiveDamageId = NAME_None;

	// [v2.96.0] 현재 활성 DamageData 핵심 값 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 DamageData 요약 (ActiveDamageSummary)", ToolTip="현재 활성 DamageData 또는 fallback DamageProfileId의 요약 문자열입니다."))
	FString ActiveDamageSummary = TEXT("DamageData: MissingOptional");

	// [v2.95.0] 현재 활성 DamageData가 어떤 경로로 해석되었는지 설명하는 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 DamageData 해석 요약 (ActiveDamageResolutionSummary)", ToolTip="ProjectileData.DefaultDamageData 또는 ProjectileData.DamageProfileId fallback 중 어떤 경로가 사용됐는지 설명합니다."))
	FString ActiveDamageResolutionSummary = TEXT("DamageResolution: MissingOptional");

	// [v2.97.0] 마지막 Damage HitContext 기록이 존재하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 HitContext 존재 여부 (bHasLastDamageHitContext)", ToolTip="Dummy HitScan 또는 Projectile Actor 충돌로 마지막 Damage HitContext Debug가 기록됐는지 여부입니다."))
	bool bHasLastDamageHitContext = false;

	// [v2.97.0] 마지막 Dummy HitScan 또는 Projectile Actor 충돌 Damage HitContext입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 Damage HitContext (LastDamageHitContext)", ToolTip="마지막 Dummy HitScan 결과 또는 Projectile Actor 충돌 결과를 같은 형식으로 기록한 Debug 컨텍스트입니다. 실제 HP 차감에는 사용하지 않습니다."))
	FCFDamageHitContext LastDamageHitContext;

	// [v2.97.0] 마지막 Damage HitContext 표시 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 Damage HitContext 요약 (LastDamageHitContextSummary)", ToolTip="VehicleDebug Panel에 표시할 마지막 Damage HitContext 요약 문자열입니다."))
	FString LastDamageHitContextSummary = TEXT("DamageHitContext: None");

	// [v2.111.0] 마지막 피해 적용 결과가 존재하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 피해 적용 결과 존재 여부 (bHasLastDamageApplyResult)", ToolTip="마지막 HitScan 또는 Projectile 명중의 피해 적용 결과가 기록됐는지 여부입니다."))
	bool bHasLastDamageApplyResult = false;

	// [v2.111.0] 마지막 직접 피해 적용과 체력 변화 결과입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 피해 적용 결과 (LastDamageApplyResult)", ToolTip="피해 적용 여부, 거부 사유, 적용량, 체력 변화와 파괴 전환을 기록합니다."))
	FCFDamageApplyResult LastDamageApplyResult;

		// [v2.111.0] 마지막 피해 적용 결과의 한글 표시 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 피해 적용 결과 요약 (LastDamageApplyResultSummary)", ToolTip="VehicleDebug Panel에 표시할 피해 적용 결과 요약입니다."))
	FString LastDamageApplyResultSummary = TEXT("피해 적용 기록 없음");

	// [v2.130.0] 현재 Pawn이 VehicleDefenseComp를 보유하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="VehicleDefenseComp 보유 여부 (bHasVehicleDefenseComponent)", ToolTip="현재 Pawn이 Shield, 6방향 Armor와 Integrity 분배를 관리하는 VehicleDefenseComp를 보유하는지 표시합니다."))
	bool bHasVehicleDefenseComponent = false;

	// [v2.130.0] VehicleDefenseComp가 유효한 DefenseData로 초기화됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="VehicleDefense 초기화 여부 (bVehicleDefenseInitialized)", ToolTip="True이면 유효한 VehicleDefenseData로 Shield와 6방향 Armor가 초기화된 상태입니다."))
	bool bVehicleDefenseInitialized = false;

	// [v2.130.0] DefenseData 없이 기존 Integrity 직접 피해 호환 경로를 사용하는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="Legacy 방어 Fallback 여부 (bUsingLegacyDefenseFallback)", ToolTip="True이면 VehicleDefenseData가 없어 기존 VehicleHealthComp 직접 Integrity 피해 호환 경로를 사용합니다."))
	bool bUsingLegacyDefenseFallback = false;

	// [v2.130.0] 현재 Shield, 방향별 Armor, 재생 상태와 초기화 상태 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="차량 방어 상태 요약 (VehicleDefenseSummary)", ToolTip="VehicleDefenseComp가 계산한 현재 Shield, 6방향 Armor와 재생 상태의 읽기 전용 요약입니다."))
	FString VehicleDefenseSummary = TEXT("VehicleDefenseComp 없음");

	// [v2.130.0] 현재 초기화 이후 마지막 전체 방어 피해 결과가 저장됐는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 차량 방어 결과 존재 여부 (bHasLastVehicleDamageResult)", ToolTip="마지막 유효 피해의 Shield, Armor, 관통과 Integrity 전체 결과가 저장됐는지 표시합니다."))
	bool bHasLastVehicleDamageResult = false;

	// [v2.130.0] 마지막 전체 방어 피해 결과의 Panel 표시용 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 차량 방어 결과 요약 (LastVehicleDamageResultSummary)", ToolTip="마지막 유효 피해의 방향, Shield 흡수, Armor 흡수·관통과 Integrity 적용 결과를 표시합니다."))
	FString LastVehicleDamageResultSummary = TEXT("차량 방어 피해 기록 없음");

	// [v2.83.0] 현재 Pawn이 ProjectilePoolComp를 보유하고 있는지 여부입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="ProjectilePoolComp 보유 여부 (bHasProjectilePoolComponent)", ToolTip="현재 Pawn이 발사체 재사용 Pool 컴포넌트를 보유하고 있는지 여부입니다."))
	bool bHasProjectilePoolComponent = false;

	// [v2.83.0] Pool이 추적 중인 전체 Projectile Actor 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="전체 Pool 발사체 수 (TotalPooledProjectileCount)", ToolTip="현재 Projectile Pool이 추적 중인 전체 발사체 Actor 수입니다. 활성 / 비활성 Actor를 모두 포함합니다."))
	int32 TotalPooledProjectileCount = 0;

	// [v2.83.0] Pool이 추적 중이고 현재 이동 중인 활성 Projectile Actor 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 Pool 발사체 수 (ActivePooledProjectileCount)", ToolTip="현재 Projectile Pool이 추적 중이고 발사되어 이동 중인 발사체 Actor 수입니다."))
	int32 ActivePooledProjectileCount = 0;

	// [v2.83.0] Pool에서 재사용 대기 중인 비활성 Projectile Actor 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="비활성 Pool 발사체 수 (InactivePooledProjectileCount)", ToolTip="현재 Projectile Pool에서 다음 발사에 재사용할 수 있는 비활성 발사체 Actor 수입니다."))
	int32 InactivePooledProjectileCount = 0;

	// [v2.85.0] Projectile Pool에 마지막으로 반환된 발사체 이벤트 요약입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 Projectile Pool 반환 요약 (LastProjectileReleaseSummary)", ToolTip="마지막으로 Pool에 반환된 발사체의 ID, 비활성화 사유, 충돌 대상, 비행 시간을 요약한 문자열입니다."))
	FString LastProjectileReleaseSummary = TEXT("ProjectileRelease: None");

	// [v2.78.0] 현재 활성 무기에서 실제 Trace에 사용할 최대 사거리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 무기 최대 사거리 (ActiveWeaponMaxRange)", ToolTip="현재 활성 WeaponData가 유효하면 WeaponData.MaxRange, 아니면 Aim Profile MaxAimDistance입니다."))
	float ActiveWeaponMaxRange = 0.0f;

	// [v2.84.0] 현재 활성 무기의 분당 발사속도입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 무기 분당 발사속도 (ActiveWeaponFireRatePerMinute)", ToolTip="현재 활성 WeaponData가 유효하면 WeaponData.FireRatePerMinute, 아니면 0입니다. 60이면 1초마다 1발입니다."))
	float ActiveWeaponFireRatePerMinute = 0.0f;

	// [v2.84.0] 현재 활성 무기의 분당 발사속도에서 환산한 발사 간격입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="활성 무기 발사 간격 초 (ActiveWeaponCooldownSeconds)", ToolTip="현재 활성 WeaponData의 FireRatePerMinute를 초 단위 발사 간격으로 환산한 값입니다. 기존 디버그/검증 호환용입니다."))
	float ActiveWeaponCooldownSeconds = 0.0f;

	// [v2.78.0] 현재 시간 기준 남은 무기 쿨다운 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="남은 무기 쿨다운 초 (ActiveWeaponRemainingCooldownSeconds)", ToolTip="현재 월드 시간 기준 활성 무기의 남은 쿨다운 시간입니다."))
	float ActiveWeaponRemainingCooldownSeconds = 0.0f;

	// [v2.78.0] 마지막으로 승인된 발사 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Weapon", meta=(DisplayName="마지막 승인 발사 시간 (LastAcceptedWeaponFireTimeSeconds)", ToolTip="VehicleWeaponComp가 마지막으로 기록한 승인 발사 시간입니다. 아직 없으면 음수입니다."))
	float LastAcceptedWeaponFireTimeSeconds = -1.0f;
};

/**
 * Pawn 레벨에서 바로 확인할 수 있는 차량 디버그 스냅샷입니다.
 * - 런타임 준비 상태와 요약 문자열
 * - 현재/이전 Drive 상태와 마지막 전이 요약
 * - DriveComp가 계산한 최신 주행 상태 스냅샷
 */
USTRUCT(BlueprintType)
struct FCFVehicleDebugSnapshot
{
	GENERATED_BODY()

	// [v2.14.1] HUD 요약용 Overview 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Overview 카테고리 (Overview)", ToolTip="HUD 요약에 사용할 VehicleDebug Overview 카테고리입니다."))
	FCFVehicleDebugOverview Overview;

	// [v2.14.1] 주행 상세 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Drive 카테고리 (Drive)", ToolTip="주행 상태와 입력 적용값을 담는 VehicleDebug Drive 카테고리입니다."))
	FCFVehicleDebugDrive Drive;

	// [v2.14.1] 입력 해석 상세 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Input 카테고리 (Input)", ToolTip="입력 장치, 주도권, 2D 이동 해석 결과를 담는 VehicleDebug Input 카테고리입니다."))
									FCFVehicleDebugInput Input;

	// [v2.7.0] 카메라 상세 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug")
	FCFVehicleDebugCamera Camera;

	// [v2.16.0] 조준 상세 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Aim 카테고리 (Aim)", ToolTip="AimComp의 로컬/검증/표시용 조준 상태를 담는 VehicleDebug Aim 카테고리입니다."))
		FCFVehicleDebugAim Aim;

	// [v2.138.0] 선택 대상 상세 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Target 카테고리 (Target)", ToolTip="TargetSelectComp의 현재 선택 대상 표시 정보와 대상 Actor의 방어·내구도 상태를 담는 VehicleDebug Target 카테고리입니다."))
	FCFVehicleDebugTarget Target;

	// [v2.77.0] 무기 상세 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Weapon 카테고리 (Weapon)", ToolTip="WeaponComp의 런타임 준비 상태, WeaponData 상태, FireOrigin 결과를 담는 VehicleDebug Weapon 카테고리입니다."))
	FCFVehicleDebugWeapon Weapon;

	// [v2.14.1] 런타임 진단 카테고리입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Runtime 카테고리 (Runtime)", ToolTip="런타임 준비 상태와 요약 문자열을 담는 VehicleDebug Runtime 카테고리입니다."))
	FCFVehicleDebugRuntime Runtime;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="런타임 준비 완료 여부 (bRuntimeReady)", ToolTip="현재 Pawn 런타임 초기화가 완료되었는지 여부입니다."))
	bool bRuntimeReady = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="런타임 요약 (RuntimeSummary)", ToolTip="마지막 차량 런타임 초기화/적용 요약 문자열입니다."))
	FString RuntimeSummary = TEXT("NotInitialized");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Drive 컴포넌트 보유 여부 (bHasDriveComponent)", ToolTip="현재 VehicleDriveComp가 유효한지 여부입니다."))
	bool bHasDriveComponent = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="WheelSync 컴포넌트 보유 여부 (bHasWheelSyncComponent)", ToolTip="현재 WheelSyncComp가 유효한지 여부입니다."))
	bool bHasWheelSyncComponent = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="현재 Drive 상태 (CurrentDriveState)", ToolTip="현재 VehicleDriveComp 기준 Drive 상태입니다."))
	ECFVehicleDriveState CurrentDriveState = ECFVehicleDriveState::Disabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="이전 Drive 상태 (PreviousDriveState)", ToolTip="이전 프레임 기준 VehicleDriveComp의 Drive 상태입니다."))
	ECFVehicleDriveState PreviousDriveState = ECFVehicleDriveState::Disabled;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="이번 프레임 상태 변경 여부 (bDriveStateChangedThisFrame)", ToolTip="이번 프레임에 Drive 상태가 변경되었는지 여부입니다."))
	bool bDriveStateChangedThisFrame = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="상태 전이 요약 (DriveStateTransitionSummary)", ToolTip="마지막 Drive 상태 전이 요약 문자열입니다."))
	FString DriveStateTransitionSummary = TEXT("DriveStateTransition: None");

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="Drive 상태 스냅샷 (DriveStateSnapshot)", ToolTip="VehicleDriveComp가 계산한 최신 Drive 상태 스냅샷입니다."))
	FCFVehicleDriveStateSnapshot DriveStateSnapshot;
};

/**
 * CarFight 신규 차량 Pawn 기준 클래스
 * - 입력은 DriveComp로 위임합니다.
 * - 휠 시각 갱신은 WheelSyncComp로 위임합니다.
 * - 카메라 자유 조준 입력은 VehicleCameraComp로 위임합니다.
 * - BP는 얇은 조립/표현 레이어로 유지하는 것을 목표로 합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API ACFVehiclePawn : public AWheeledVehiclePawn, public ICFTargetSelectable
{
	GENERATED_BODY()

						friend class UCFLauncherComp;
	friend class FCFAmmoFireTransactionTest;
	// [v2.150.0] UI-P0-06 Heat Automation이 기존 private accepted-fire/apply 경계를 public API로 열지 않고 실제 호출하도록 허용합니다.
	friend class FCFHUDP006HeatResourceTest;
	// [v2.144.0] VD-P0-03 Automation이 프로덕션 공개 API를 늘리지 않고 실제 VehicleData 적용 경로를 검증할 테스트 전용 접근 경계입니다.
	friend class FCFVDATuningRuntimeApplyTest;

public:
	// [v1.1.0] 기본 생성자
	ACFVehiclePawn();

protected:
		// [v1.4.0] Construction 시점에 VehicleVisualConfig를 적용해 에디터 미리보기를 갱신합니다.
	virtual void OnConstruction(const FTransform& Transform) override;

	// [v2.133.0] 게임 World의 물리 컴포넌트 등록 전에 초기 피팅 Snapshot 질량을 Movement Mass에 1회 기록합니다.
	virtual void PreRegisterAllComponents() override;

	// [v1.1.0] BeginPlay에서 런타임 초기화와 입력 매핑 등록을 시도합니다.
	virtual void BeginPlay() override;

	// [v2.20.0] EndPlay에서 생성된 Aim Reticle 위젯을 정리합니다.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// [v1.1.0] Tick에서 휠 시각 갱신을 수행합니다.
	virtual void Tick(float DeltaSeconds) override;

	// [v1.1.0] PlayerInputComponent에 Enhanced Input Action을 바인딩합니다.
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn", meta=(DisplayName="차량 데이터 (VehicleData)", ToolTip="차량 루트 DataAsset 참조입니다. 현재 단계에서는 공식 타입을 UCFVehicleData로 고정하고, 세부 VehicleMovement 및 WheelVisual 해석은 후속 단계로 확장합니다."))
		TObjectPtr<UCFVehicleData> VehicleData = nullptr;

	// [v2.132.0] 출격 초기화에서 VehicleData 기본값을 덮어쓸 선택적 피팅 DataAsset입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|VehiclePawn|Fitting", meta=(DisplayName="차량 피팅 데이터 (VehicleFittingData)", ToolTip="지정하면 유효 Snapshot의 Weapon·Defense 입력을 원자 적용합니다. 비어 있으면 기존 VehicleData 기본 경로를 유지합니다."))
	TObjectPtr<UCFVehicleFittingData> VehicleFittingData = nullptr;

		// [v2.134.0] 현재 차량 Gameplay 입력을 소유하는 Pawn 기본 Mapping Context입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="기본 입력 매핑 컨텍스트 (DefaultInputMappingContext)", ToolTip="현재 차량 Gameplay 입력의 소유 Context입니다. BeginPlay와 SetupPlayerInputComponent에서 등록을 시도하며, Controller로 입력 소유권을 일괄 이전하기 전에는 Controller Gameplay Context와 중복 지정하지 않습니다."))
	TObjectPtr<UInputMappingContext> DefaultInputMappingContext = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="차량 이동 입력 액션 (InputAction_VehicleMove)", ToolTip="차량 전진/후진/브레이크/조향을 함께 해석할 2D 이동 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_VehicleMove = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="스로틀 입력 액션 (InputAction_Throttle)", ToolTip="Throttle 축 입력에 사용할 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Throttle = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="조향 입력 액션 (InputAction_Steering)", ToolTip="Steering 축 입력에 사용할 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Steering = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="브레이크 입력 액션 (InputAction_Brake)", ToolTip="Brake 축 입력에 사용할 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Brake = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="핸드브레이크 입력 액션 (InputAction_Handbrake)", ToolTip="Handbrake 토글 입력에 사용할 Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Handbrake = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="시점 입력 액션 (InputAction_Look)", ToolTip="차량 카메라 자유 조준 입력에 사용할 2D Look Input Action 입니다."))
	TObjectPtr<UInputAction> InputAction_Look = nullptr;

	// [v2.64.0] 로컬 발사 명령을 시작할 입력 액션입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="발사 입력 액션 (InputAction_Fire)", ToolTip="Aim 기반 로컬 발사 명령을 시작할 Input Action입니다. 비어 있으면 발사 입력 바인딩을 건너뜁니다."))
				TObjectPtr<UInputAction> InputAction_Fire = nullptr;

	// 현재 후보를 지속 선택 대상으로 확정할 입력 액션입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="타겟 선택 입력 액션 (InputAction_SelectTarget)", ToolTip="현재 TargetSelectComp 후보를 지속 선택 대상으로 확정하는 Boolean Input Action입니다. 후보가 없으면 기존 선택을 유지합니다."))
	TObjectPtr<UInputAction> InputAction_SelectTarget = nullptr;

				// 현재 지속 선택 대상만 수동 해제할 입력 액션입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="타겟 해제 입력 액션 (InputAction_ClearTarget)", ToolTip="현재 선택 대상을 Manual 사유로 해제하는 Boolean Input Action입니다. 현재 후보는 유지하며 자동 다음 타겟을 선택하지 않습니다."))
	TObjectPtr<UInputAction> InputAction_ClearTarget = nullptr;

	// [v2.152.0] 실제 SelectableWeapons의 1-based 순번을 직접 선택할 Axis1D Input Action입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input|Weapon", meta=(DisplayName="무기 순번 선택 입력 액션 (InputAction_SelectWeapon)", ToolTip="숫자키가 전달한 1~9 값을 실제 Applied Fitting SelectableWeapons의 1-based 순번으로 해석합니다. 내부 MountProfileId나 WeaponGroup ID를 입력 의미로 사용하지 않습니다."))
	TObjectPtr<UInputAction> InputAction_SelectWeapon = nullptr;

		// [v2.147.0] P0 단발 Active Scan을 시작할 기본 Input Action입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input|Sensor", meta=(DisplayName="Active Scan 입력 액션 (InputAction_StartActiveScan)", ToolTip="P0 기본 IA_ActiveScan Boolean + Pressed 입력입니다. 한 번 누르면 VehicleSensorComp가 ActiveScanDurationSec 동안 Active Scan을 실행하고 자동 종료합니다. 기본 키는 V입니다."))
	TObjectPtr<UInputAction> InputAction_StartActiveScan = nullptr;

	// [v2.147.0] P0에서는 키에 연결하지 않는 시스템용 Active Scan 중단 의미 슬롯입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Input|Sensor", meta=(DisplayName="Active Scan 중단 입력 액션 (InputAction_StopActiveScan)", ToolTip="P0에서는 기본 자산과 키를 지정하지 않습니다. 장비 교체·상태 전환 또는 향후 명시적 취소 입력이 필요할 때 VehicleSensorComp 중단 명령에 연결할 수 있습니다."))
	TObjectPtr<UInputAction> InputAction_StopActiveScan = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="입력 장치 모드 (InputDeviceMode)", ToolTip="차량 입력을 어떤 장치로 받을지 고정합니다. Auto는 키보드/마우스와 게임패드를 모두 허용하고, KeyboardMouseOnly는 키보드/마우스만, GamepadOnly는 게임패드만 허용합니다."))
	ECFVehicleInputDeviceMode InputDeviceMode = ECFVehicleInputDeviceMode::Auto;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Input", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="입력 장치 아날로그 임계값 (InputDeviceAnalogThreshold)", ToolTip="고정 입력 장치 모드에서 아날로그 키를 활성 입력으로 판정할 최소 절대값입니다. 스틱 드리프트가 있으면 값을 조금 올려서 사용합니다."))
	float InputDeviceAnalogThreshold = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Input", meta=(ClampMin="0", DisplayName="입력 매핑 우선순위 (InputMappingPriority)", ToolTip="기본 Input Mapping Context 등록 우선순위입니다."))
	int32 InputMappingPriority = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="입력 매핑 자동 등록 (bAutoRegisterInputMappingContext)", ToolTip="True이면 BeginPlay / SetupPlayerInputComponent에서 기본 Input Mapping Context 등록을 자동 시도합니다."))
	bool bAutoRegisterInputMappingContext = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="차량 이동 입력 설정 (VehicleMoveInputConfig)", ToolTip="차량 2D 이동 입력의 각도 해석 기본 설정입니다."))
	FCFVehicleMoveInputConfig VehicleMoveInputConfig;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="마지막 진행 방향 의도 (LastMoveDirectionIntent)", ToolTip="검은 영역 진입 시 유지할 마지막 유효 진행 방향입니다."))
	ECFVehicleMoveDirectionIntent LastMoveDirectionIntent = ECFVehicleMoveDirectionIntent::None;

		UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="마지막 차량 이동 입력 결과 (LastVehicleMoveInputResult)", ToolTip="현재 프레임 기준 차량 2D 이동 입력 해석 결과입니다."))
	FCFVehicleMoveInputResult LastVehicleMoveInputResult;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="현재 입력 소유권 (CurrentInputOwnership)", ToolTip="현재 프레임 기준 차량 입력 적용의 주도권을 가진 입력 경로입니다."))
	ECFVehicleInputOwnership CurrentInputOwnership = ECFVehicleInputOwnership::None;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", DisplayName="입력 소유권 유지 시간 (InputOwnershipHoldTimeSec)", ToolTip="입력 경로 주도권 전환 시 흔들림을 줄이기 위해 현재 소유권을 유지할 최소 시간(초)입니다."))
	float InputOwnershipHoldTimeSec = 0.15f;

	// [v2.8.0] 조향 방향 계산을 허용할 최소 2D 스틱 입력 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="조향 방향 최소 입력 (SteeringDirectionMinMagnitude)", ToolTip="이 값보다 작은 2D 이동 입력은 조향 방향 계산에서 무시하고 중립 복귀 대상으로 처리합니다."))
	float SteeringDirectionMinMagnitude = 0.10f;

	// [v2.8.0] 왼쪽 최대 조향에서 오른쪽 최대 조향까지 이동하는 데 걸리는 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.01", DisplayName="락 투 락 시간 (SteeringLockToLockTimeSec)", ToolTip="실제 조향값이 -1에서 +1까지 제한 속도로 이동하는 데 걸리는 시간입니다."))
	float SteeringLockToLockTimeSec = 0.45f;

	// [v2.44.0] 기존 축 조향 입력도 제한 속도 보간을 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="레거시 조향 smoothing 사용 (bSmoothLegacySteeringInput)", ToolTip="키보드/축 조향 입력을 즉시 DriveComp에 넣지 않고 CurrentSteeringInput 보간 경로로 전달합니다. 좌우 조향 끊김을 분리 확인할 때 사용합니다."))
	bool bSmoothLegacySteeringInput = true;

	// [v2.55.0] 차량 속도가 올라갈수록 Chaos Vehicle에 전달하는 실제 조향 입력을 줄일지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="속도 기반 조향 제한 사용 (bEnableSpeedSteeringLimit)", ToolTip="True이면 고속 좌우 조향 시 차량 Yaw 변화가 과격해지는 현상을 줄이기 위해 실제 조향 입력을 속도에 따라 낮춥니다."))
	bool bEnableSpeedSteeringLimit = false;

	// [v2.55.0] 속도 기반 조향 제한이 시작되는 차량 속도(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", DisplayName="조향 제한 시작 속도 km/h (SpeedSteeringLimitStartSpeedKmh)", ToolTip="이 속도부터 Chaos Vehicle에 전달하는 실제 조향 입력을 서서히 줄이기 시작합니다."))
	float SpeedSteeringLimitStartSpeedKmh = 10.0f;

	// [v2.55.0] 속도 기반 조향 제한이 최대치에 도달하는 차량 속도(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", DisplayName="조향 제한 최대 속도 km/h (SpeedSteeringLimitFullSpeedKmh)", ToolTip="이 속도 이상에서는 SpeedSteeringLimitMinScale 값을 적용해 실제 조향 입력을 제한합니다."))
	float SpeedSteeringLimitFullSpeedKmh = 28.0f;

	// [v2.55.0] 고속 구간에서 허용할 최소 실제 조향 입력 배율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.05", ClampMax="1.0", DisplayName="고속 최소 조향 배율 (SpeedSteeringLimitMinScale)", ToolTip="조향 제한 최대 속도 이상에서 원본 조향 입력에 곱할 최소 배율입니다. 0.45이면 풀 조향 입력 1.0이 실제 0.45로 전달됩니다."))
	float SpeedSteeringLimitMinScale = 0.45f;

	// [v2.8.0] 중립 복귀 속도 보간 시작 차량 속도(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", DisplayName="조향 복귀 최소 속도 km/h (SteeringReturnMinSpeedKmh)", ToolTip="중립 복귀 속도 보간을 시작할 차량 속도입니다."))
		float SteeringReturnMinSpeedKmh = 3.0f;


	// [v2.8.0] 중립 복귀 속도 보간 완료 차량 속도(km/h)입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", DisplayName="조향 복귀 최대 속도 km/h (SteeringReturnMaxSpeedKmh)", ToolTip="이 속도 이상에서는 최대 중립 복귀 속도를 사용합니다."))
	float SteeringReturnMaxSpeedKmh = 80.0f;

	// [v2.8.0] 극저속에서 사용할 중립 복귀 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", DisplayName="최소 조향 복귀 속도 (SteeringReturnMinRate)", ToolTip="차량이 정지 또는 극저속일 때 사용하는 중립 복귀 속도입니다."))
		float SteeringReturnMinRate = 0.0f;


	// [v2.8.0] 고속에서 사용할 중립 복귀 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|MoveInput", meta=(ClampMin="0.0", DisplayName="최대 조향 복귀 속도 (SteeringReturnMaxRate)", ToolTip="차량이 SteeringReturnMaxSpeedKmh 이상일 때 사용하는 중립 복귀 속도입니다."))
	float SteeringReturnMaxRate = 6.0f;

	// [v2.8.0] 게임패드 2D 이동 입력 방향에서 계산한 목표 조향값입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="목표 조향값 (TargetSteeringInput)", ToolTip="스틱 방향에서 계산된 목표 조향값입니다."))
	float TargetSteeringInput = 0.0f;

	// [v2.44.0] 기존 축 조향 이벤트에서 받은 목표 조향값입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="레거시 목표 조향값 (LegacyTargetSteeringInput)", ToolTip="키보드/축 조향 입력에서 받은 목표 조향값입니다. bSmoothLegacySteeringInput이 켜져 있을 때 CurrentSteeringInput이 이 값을 제한 속도로 따라갑니다."))
	float LegacyTargetSteeringInput = 0.0f;

	// [v2.8.0] 실제 DriveComp에 전달 중인 제한 속도 적용 조향값입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="현재 조향값 (CurrentSteeringInput)", ToolTip="목표 조향값을 제한 속도로 따라가며 실제 DriveComp에 전달 중인 조향값입니다."))
	float CurrentSteeringInput = 0.0f;

	// [v2.8.0] 마지막으로 계산된 조향 진입 속도입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="마지막 조향 진입 속도 (LastSteeringTurnRate)", ToolTip="목표 조향을 따라갈 때 사용한 마지막 조향 변화 속도입니다."))
	float LastSteeringTurnRate = 0.0f;

	// [v2.8.0] 마지막으로 계산된 중립 복귀 속도입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="마지막 조향 복귀 속도 (LastSteeringReturnRate)", ToolTip="중립 복귀 중 사용한 마지막 속도 기반 조향 복귀 속도입니다."))
	float LastSteeringReturnRate = 0.0f;

	// [v2.8.0] 현재 조향이 중립 복귀 중인지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="조향 중립 복귀 중 여부 (bSteeringReturningToCenter)", ToolTip="현재 실제 조향값이 중립 0을 향해 복귀 중인지 여부입니다."))
	bool bSteeringReturningToCenter = false;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="마지막 VehicleMove 입력 시각 (LastVehicleMoveInputTimeSec)", ToolTip="VehicleMove 2D 입력이 마지막으로 유효하게 들어온 월드 시각(초)입니다."))

	float LastVehicleMoveInputTimeSec = -1.0f;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|MoveInput", meta=(DisplayName="마지막 LegacyAxis 입력 시각 (LastLegacyAxisInputTimeSec)", ToolTip="기존 Throttle/Brake/Steering 축 입력이 마지막으로 유효하게 들어온 월드 시각(초)입니다."))
	float LastLegacyAxisInputTimeSec = -1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="Drive 컴포넌트 (VehicleDriveComp)", ToolTip="입력 전달 전용 Drive 컴포넌트입니다."))

	TObjectPtr<UCFVehicleDriveComp> VehicleDriveComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="WheelSync 컴포넌트 (WheelSyncComp)", ToolTip="휠 시각 동기화 전용 WheelSync 컴포넌트입니다."))
	TObjectPtr<UCFWheelSyncComp> WheelSyncComp = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="VehicleCamera 컴포넌트 (VehicleCameraComp)", ToolTip="차량 중심 피벗 기반 자유 조준과 Aim Trace를 계산하는 차량 카메라 컴포넌트입니다."))
	TObjectPtr<UCFVehicleCameraComp> VehicleCameraComp = nullptr;

	// [v2.15.0] 카메라 조준과 향후 무기 발사 사이의 Aim 해석 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="VehicleAim 컴포넌트 (VehicleAimComp)", ToolTip="카메라 조준과 향후 무기 발사 사이의 Aim 해석 컴포넌트입니다."))
	TObjectPtr<UCFVehicleAimComp> VehicleAimComp = nullptr;

	// [v2.75.0] 차량 장착 프로파일과 실제 발사 원점을 해석하는 Weapon 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="VehicleWeapon 컴포넌트 (VehicleWeaponComp)", ToolTip="차량 하드포인트와 장착 프로파일을 읽어 실제 발사 원점을 계산하는 컴포넌트입니다."))
		TObjectPtr<UCFVehicleWeaponComp> VehicleWeaponComp = nullptr;

		// [v2.132.0] 출격 피팅 Snapshot 준비·Commit·Rollback과 AppliedFittingSnapshot을 소유하는 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="차량 피팅 컴포넌트 (VehicleFittingComp)"))
	TObjectPtr<UCFVehicleFittingComp> VehicleFittingComp = nullptr;

	// [v2.139.0] 탄종별 공유 예비량과 WeaponInstanceId별 독립 장전 상태를 단일 소유하는 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="차량 탄약 컴포넌트 (VehicleAmmoComp)", ToolTip="현재 출격의 실제 장전 탄약, 차량 예비 탄약, Launcher 예약과 재장전 상태를 단일 소유합니다."))
	TObjectPtr<UCFVehicleAmmoComp> VehicleAmmoComp = nullptr;

		// [v2.126.0] Ripple·Salvo 예약 발사, 취소와 Volley 단위 쿨다운을 관리하는 런처 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="런처 컴포넌트 (LauncherComp)", ToolTip="첫 승인 발사 이후 남은 Ripple·Salvo 발사를 예약하고 장비 변경·차량 파괴·실패 정책에 따라 시퀀스를 완료하거나 취소합니다."))
	TObjectPtr<UCFLauncherComp> LauncherComp = nullptr;

	// [v2.82.0] 반복 발사되는 Projectile Actor를 재사용하는 Pool 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="ProjectilePool 컴포넌트 (ProjectilePoolComp)", ToolTip="반복 발사되는 Projectile Actor를 재사용해 Spawn / Destroy 부담을 줄이는 Pool 컴포넌트입니다."))
	TObjectPtr<UCFProjectilePoolComp> ProjectilePoolComp = nullptr;

		// [v2.111.0] 차량 최대/현재 내구도와 파괴 상태를 관리하는 런타임 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="차량 내구도 컴포넌트 (VehicleHealthComp)", ToolTip="VehicleData 최대 내구도, 현재 내구도와 파괴 상태를 관리합니다. 기존 Blueprint 호환을 위해 컴포넌트 이름은 유지합니다."))
	TObjectPtr<UCFVehicleHealthComp> VehicleHealthComp = nullptr;

	// [v2.129.0] 쉴드와 6방향 장갑의 정식 피해 분배를 관리하는 런타임 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="차량 방어 컴포넌트 (VehicleDefenseComp)", ToolTip="VehicleData.DefaultDefenseData를 읽어 쉴드, 6방향 장갑, 관통과 차량 내구도 피해 분배를 관리합니다. 방어 데이터가 없으면 기존 직접 내구도 피해로 Fallback합니다."))
	TObjectPtr<UCFVehicleDefenseComp> VehicleDefenseComp = nullptr;

	// 확정된 발사, Impact와 최초 파괴 결과를 Niagara 시각 연출로 변환하는 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="전투 FX 컴포넌트", ToolTip="전투 판정을 변경하지 않고 데이터 기반 Niagara FX를 재생합니다."))
	TObjectPtr<UCFCombatFxComp> CombatFxComp = nullptr;

	// 차량별 선택·표시 대표 위치를 제공하는 타겟 포인트 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="타겟 포인트 컴포넌트 (TargetPointComp)", ToolTip="차량별 선택 대표 위치를 제공합니다. 기본적으로 비활성 상태이며 Blueprint에서 위치를 조정하고 bUseAsTargetPoint를 켜면 Bounds보다 우선 사용합니다."))
	TObjectPtr<UCFTargetPointComp> TargetPointComp = nullptr;

				// 현재 후보와 지속 선택 대상을 소유하는 타겟 선택 상태 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="타겟 선택 컴포넌트 (TargetSelectComp)", ToolTip="후보, 선택 대상, 표시 정보, 유효성과 추적 상태의 최소 계약을 관리합니다. 후보 탐색, 입력과 HUD는 후속 단계에서 연결합니다."))
	TObjectPtr<UCFTargetSelectComp> TargetSelectComp = nullptr;

	// [v2.145.0] TargetSelect와 독립적으로 Sensor Contact와 Player Knowledge Snapshot을 소유할 런타임 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="차량 센서 컴포넌트 (VehicleSensorComp)", ToolTip="TargetSelect 선택 상태와 독립적으로 Sensor Contact, 플레이어가 획득한 지식과 Actor-free Snapshot을 소유합니다."))
	TObjectPtr<UCFVehicleSensorComp> VehicleSensorComp = nullptr;

	// [v2.86.0] P0 터렛 시각 장착 위치를 잡는 루트 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components|Turret", meta=(AllowPrivateAccess="true", DisplayName="터렛 장착 루트 (TurretMountRootComp)", ToolTip="현재 활성 하드포인트 위치에 배치되는 터렛 시각 장착 루트입니다."))
	TObjectPtr<USceneComponent> TurretMountRootComp = nullptr;

	// [v2.88.0] 하드포인트에 고정되는 터렛 받침 메쉬 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components|Turret", meta=(AllowPrivateAccess="true", DisplayName="터렛 Base 메쉬 컴포넌트 (TurretBaseMeshComp)", ToolTip="하드포인트에 고정되는 터렛 받침/Base 메쉬 컴포넌트입니다."))
	TObjectPtr<UStaticMeshComponent> TurretBaseMeshComp = nullptr;

	// [v2.88.0] 좌우 Yaw 회전을 적용할 가상 피벗 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components|Turret", meta=(AllowPrivateAccess="true", DisplayName="터렛 Yaw 피벗 컴포넌트 (TurretYawPivotComp)", ToolTip="Base 메쉬의 YawPivot 소켓 또는 하드포인트 루트에 붙어 좌우 회전을 담당할 가상 피벗 컴포넌트입니다."))
	TObjectPtr<USceneComponent> TurretYawPivotComp = nullptr;

	// [v2.88.0] YawPivot 아래에서 좌우 Yaw 회전 기준으로 사용할 터렛 상부 메쉬 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components|Turret", meta=(AllowPrivateAccess="true", DisplayName="터렛 Yaw 메쉬 컴포넌트 (TurretYawMeshComp)", ToolTip="YawPivot 아래에 붙어 좌우 회전할 터렛 회전부 메쉬 컴포넌트입니다."))
	TObjectPtr<UStaticMeshComponent> TurretYawMeshComp = nullptr;

	// [v2.88.0] 상하 Pitch 회전을 적용할 가상 피벗 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components|Turret", meta=(AllowPrivateAccess="true", DisplayName="터렛 Pitch 피벗 컴포넌트 (TurretPitchPivotComp)", ToolTip="Yaw 메쉬의 PitchPivot 소켓 또는 Yaw 피벗에 붙어 상하 회전을 담당할 가상 피벗 컴포넌트입니다."))
	TObjectPtr<USceneComponent> TurretPitchPivotComp = nullptr;

	// [v2.88.0] PitchPivot 아래에서 상하 Pitch 회전 후보로 사용할 터렛 상부 또는 포신 메쉬 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components|Turret", meta=(AllowPrivateAccess="true", DisplayName="터렛 Pitch 메쉬 컴포넌트 (TurretPitchMeshComp)", ToolTip="PitchPivot 아래에 붙어 Yaw를 따라 좌우 회전하고 Pitch로 상하 회전할 터렛 상부/포신 메쉬 컴포넌트입니다."))
	TObjectPtr<UStaticMeshComponent> TurretPitchMeshComp = nullptr;

	// [v2.48.0] 로컬 Owner 표시 안정화에 사용할 차체/휠 표시 전용 루트 컴포넌트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Components", meta=(AllowPrivateAccess="true", DisplayName="Owner 표시 루트 (OwnerVisualRootComp)", ToolTip="로컬 조작 차량에서 차체와 휠 표시를 물리 루트 흔들림과 분리하기 위한 표시 전용 루트입니다."))
	TObjectPtr<USceneComponent> OwnerVisualRootComp = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Runtime", meta=(DisplayName="BeginPlay 자동 초기화 (bAutoInitializeOnBeginPlay)", ToolTip="True이면 BeginPlay에서 Drive / WheelSync 캐시와 준비를 자동 시도합니다."))
	bool bAutoInitializeOnBeginPlay = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Runtime", meta=(DisplayName="휠 시각 Tick 사용 (bEnableWheelVisualTick)", ToolTip="True이면 Tick에서 WheelSync의 휠 시각 갱신을 자동 호출합니다."))
	bool bEnableWheelVisualTick = true;

		// [v2.134.0] 차량 주행·물리·내구도·피팅을 사용할 수 있는 코어 Runtime 준비 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Runtime", meta=(DisplayName="차량 코어 런타임 준비 여부 (bVehicleCoreRuntimeReady)", ToolTip="Drive, WheelSync, Health, 정식 Defense 진입점과 Fitting 적용이 준비되어 차량 기본 주행과 피격 처리를 수행할 수 있으면 True입니다. Aim, Weapon과 Launcher 준비 여부는 포함하지 않습니다."))
	bool bVehicleCoreRuntimeReady = false;

	// [v2.134.0] 차량 코어와 조준·무기·런처·타겟 선택을 모두 사용할 수 있는 전투 Runtime 준비 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Runtime", meta=(DisplayName="차량 전투 런타임 준비 여부 (bVehicleCombatRuntimeReady)", ToolTip="차량 코어 Runtime에 더해 Aim, Weapon, Launcher와 TargetSelect가 준비되어 전투 HUD와 전투 명령이 사용할 수 있으면 True입니다."))
	bool bVehicleCombatRuntimeReady = false;

	// [v2.134.0] 기존 Blueprint·Tick·Debug 호환을 위한 차량 코어 Runtime 준비 별칭입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Runtime", meta=(DisplayName="차량 런타임 준비 완료 여부 (bVehicleRuntimeReady)", ToolTip="기존 호환용 값이며 bVehicleCoreRuntimeReady와 같습니다. 전투 준비 여부가 필요하면 bVehicleCombatRuntimeReady를 사용합니다."))
	bool bVehicleRuntimeReady = false;

	// [v2.134.0] 마지막 차량 코어·전투 Runtime 초기화 결과 요약 문자열입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Runtime", meta=(DisplayName="런타임 결과 요약 (LastVehicleRuntimeSummary)", ToolTip="마지막 차량 코어와 전투 Runtime 초기화 결과를 함께 요약한 문자열입니다."))
	FString LastVehicleRuntimeSummary = TEXT("NotInitialized");

	// [v2.17.0] 다음 발사 요청에 사용할 증가형 요청 ID입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Aim", meta=(DisplayName="다음 발사 요청 ID (NextFireRequestId)", ToolTip="다음 Aim 기반 발사 요청에 사용할 증가형 요청 ID입니다."))
	int32 NextFireRequestId = 1;

	// [v2.63.0] 마지막으로 생성하거나 레거시 wrapper에서 받은 발사 명령입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Aim", meta=(DisplayName="마지막 발사 명령 (LastFireRequest)", ToolTip="마지막으로 생성하거나 레거시 wrapper에서 받은 Aim 기반 로컬 발사 명령 디버그 캐시입니다. 변수명은 기존 BP 호환을 위해 유지합니다."))
	FCFVehicleFireRequest LastFireRequest;

	// [v2.63.0] 마지막 로컬 발사 검증 결과입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Aim", meta=(DisplayName="마지막 발사 결과 (LastFireResult)", ToolTip="마지막 로컬 발사 검증 결과 디버그 캐시입니다."))
	FCFVehicleFireResult LastFireResult;

	// [v2.102.0] 마지막 로컬 발사 피드백이 시작된 월드 시간입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|FireFeedback", meta=(DisplayName="마지막 FireFeedback 시작 시간 (LastFireFeedbackStartTimeSeconds)", ToolTip="Reticle / FireFeedback UI가 최근 발사 성공 또는 실패 피드백 표시 시간을 계산할 때 사용하는 월드 시간입니다."))
	double LastFireFeedbackStartTimeSeconds = -1.0;

	// [v2.102.0] 발사 성공 피드백을 화면에 유지할 시간입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|FireFeedback", meta=(ClampMin="0.0", DisplayName="발사 성공 피드백 유지 시간 (FireSuccessFeedbackDurationSeconds)", ToolTip="발사 성공 피드백을 Reticle UI에 짧게 표시할 시간입니다."))
	float FireSuccessFeedbackDurationSeconds = 0.12f;

	// [v2.102.0] 발사 실패 피드백을 화면에 유지할 시간입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|FireFeedback", meta=(ClampMin="0.0", DisplayName="발사 실패 피드백 유지 시간 (FireRejectedFeedbackDurationSeconds)", ToolTip="발사 실패 또는 조건 미충족 피드백을 Reticle UI에 표시할 시간입니다."))
	float FireRejectedFeedbackDurationSeconds = 0.35f;

	// [v2.97.0] 마지막 Damage HitContext 기록이 존재하는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Damage", meta=(DisplayName="마지막 Damage HitContext 존재 여부 (bHasLastDamageHitContext)", ToolTip="Dummy HitScan 또는 Projectile Actor 충돌로 마지막 Damage HitContext Debug가 기록됐는지 여부입니다."))
	bool bHasLastDamageHitContext = false;

	// [v2.97.0] 마지막 Dummy HitScan 또는 Projectile Actor 충돌 Damage HitContext입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Damage", meta=(DisplayName="마지막 Damage HitContext (LastDamageHitContext)", ToolTip="마지막 Dummy HitScan 결과 또는 Projectile Actor 충돌 결과를 같은 형식으로 기록한 Debug 컨텍스트입니다. 실제 HP 차감에는 사용하지 않습니다."))
	FCFDamageHitContext LastDamageHitContext;

	// [v2.97.0] 마지막 Damage HitContext 표시 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Damage", meta=(DisplayName="마지막 Damage HitContext 요약 (LastDamageHitContextSummary)", ToolTip="VehicleDebug Panel에 표시할 마지막 Damage HitContext 요약 문자열입니다."))
	FString LastDamageHitContextSummary = TEXT("DamageHitContext: None");

	// [v2.111.0] 마지막 피해 적용 결과가 존재하는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Damage", meta=(DisplayName="마지막 피해 적용 결과 존재 여부 (bHasLastDamageApplyResult)", ToolTip="마지막 HitScan 또는 Projectile 명중의 피해 적용 결과가 기록됐는지 여부입니다."))
	bool bHasLastDamageApplyResult = false;

	// [v2.111.0] 마지막 직접 피해 적용과 체력 변화 결과입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Damage", meta=(DisplayName="마지막 피해 적용 결과 (LastDamageApplyResult)", ToolTip="피해 적용 여부, 거부 사유, 적용량, 체력 변화와 파괴 전환을 기록합니다."))
	FCFDamageApplyResult LastDamageApplyResult;

	// [v2.111.0] 마지막 피해 적용 결과의 한글 표시 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Debug|Damage", meta=(DisplayName="마지막 피해 적용 결과 요약 (LastDamageApplyResultSummary)", ToolTip="VehicleDebug Panel에 표시할 피해 적용 결과 요약입니다."))
	FString LastDamageApplyResultSummary = TEXT("피해 적용 기록 없음");

	// [v2.67.0] 로컬 HitScan 더미 Trace 디버그 라인 표시 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Aim", meta=(DisplayName="로컬 Aim Trace 디버그 표시 (bDrawLocalAimTraceDebug)", ToolTip="True이면 로컬 HitScan 더미 Trace를 디버그 라인으로 표시합니다."))
	bool bDrawLocalAimTraceDebug = false;

	// [v2.67.0] 로컬 HitScan 더미 Trace 디버그 라인 표시 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Aim", meta=(ClampMin="0.0", DisplayName="로컬 Aim Trace 디버그 시간 (LocalAimTraceDebugDuration)", ToolTip="로컬 HitScan 더미 Trace 디버그 라인을 표시할 시간입니다."))
	float LocalAimTraceDebugDuration = 2.0f;

		// [v2.148.0] UI-P0-04 이전 Pawn 소유 Reticle Class 직렬화 호환을 위해 유지하는 Legacy 설정입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Aim|Reticle", meta=(DisplayName="Aim Reticle 위젯 클래스 (AimReticleWidgetClass)", ToolTip="Legacy 직렬화 호환용 Reticle Class입니다. UI-P0-04 이후 자동 생성 소유권은 CFUISubsystem의 DefaultAimReticleWidgetClass가 담당합니다."))
	TSubclassOf<UCFAimReticleWidget> AimReticleWidgetClass = nullptr;

	// [v2.148.0] UI-P0-04 이전 Pawn 직접 생성 인스턴스가 남아 있을 때만 정리하는 Legacy 캐시입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|Aim|Reticle", meta=(DisplayName="Aim Reticle 위젯 인스턴스 (AimReticleWidgetInstance)", ToolTip="Legacy Pawn 직접 생성 인스턴스 정리용 캐시입니다. 새 자동 수명은 CFUISubsystem이 소유하므로 정상 UI-P0-04 경로에서는 Null입니다."))
	TObjectPtr<UCFAimReticleWidget> AimReticleWidgetInstance = nullptr;

	// [v2.20.0] Aim Reticle 위젯 표시를 허용하는 토글입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Aim|Reticle", meta=(DisplayName="Aim Reticle 표시 여부 (bShowAimReticle)", ToolTip="True이면 로컬 제어 Pawn에서 Aim Reticle 위젯 표시를 허용합니다. VehicleDebug UI 토글과는 별도입니다."))
	bool bShowAimReticle = true;

		// [v2.148.0] UI-P0-04 이전 Pawn 직접 AddToViewport 순서를 보존하는 Legacy ZOrder입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Aim|Reticle", meta=(ClampMin="0", DisplayName="Aim Reticle ZOrder (AimReticleZOrder)", ToolTip="Legacy Pawn 직접 Viewport 경로의 호환값입니다. UI-P0-04 자동 경로는 CFUISubsystem의 HUD Layer 내부 ZOrder를 사용합니다."))
	int32 AimReticleZOrder = 10;

		// [v2.149.0] UI-P0-05 이전 Pawn 소유 TargetSelect Class 직렬화 호환을 위해 유지하는 Legacy 설정입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|TargetSelect|HUD", meta=(DisplayName="타겟 선택 HUD 위젯 클래스 (TargetSelectWidgetClass)", ToolTip="Legacy 직렬화 호환용 TargetSelect Class입니다. UI-P0-05 이후 자동 생성 소유권은 CFUISubsystem의 DefaultTargetSelectWidgetClass가 담당합니다."))
	TSubclassOf<UCFTargetSelectWidget> TargetSelectWidgetClass = nullptr;

	// [v2.149.0] UI-P0-05 이전 Pawn 직접 생성 인스턴스가 남아 있을 때만 정리하는 Legacy 캐시입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|VehiclePawn|TargetSelect|HUD", meta=(DisplayName="타겟 선택 HUD 위젯 인스턴스 (TargetSelectWidgetInstance)", ToolTip="Legacy Pawn 직접 생성 인스턴스 정리용 캐시입니다. 새 자동 수명은 CFUISubsystem이 소유하므로 정상 UI-P0-05 경로에서는 Null입니다."))
	TObjectPtr<UCFTargetSelectWidget> TargetSelectWidgetInstance = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|TargetSelect|HUD", meta=(DisplayName="타겟 선택 HUD 표시 여부 (bShowTargetSelectHud)", ToolTip="True이면 로컬 제어 Pawn에서 후보와 선택 타겟 HUD 표시를 허용합니다."))
	bool bShowTargetSelectHud = true;

		// [v2.149.0] UI-P0-05 이전 Pawn 직접 AddToViewport 순서를 보존하는 Legacy ZOrder입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|TargetSelect|HUD", meta=(ClampMin="0", DisplayName="타겟 선택 HUD ZOrder (TargetSelectHudZOrder)", ToolTip="Legacy Pawn 직접 Viewport 경로의 호환값입니다. UI-P0-05 자동 경로는 CFUISubsystem의 Game Layer 내부 ZOrder를 사용합니다."))
	int32 TargetSelectHudZOrder = 20;

	// [v2.14.4] VehicleDebug HUD/Panel UI 표시를 허용하는 메인 토글입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="VehicleDebug UI 사용 (bEnableDriveStateOnScreenDebug)", ToolTip="True이면 PIE 중 VehicleDebug HUD/Panel UI 표시를 허용합니다. 개발용 온스크린 문자열 출력과는 별도입니다."))
	bool bEnableDriveStateOnScreenDebug = false;

	// [v2.14.4] 개발용 온스크린 문자열 디버그 메시지 출력을 허용하는 토글입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="온스크린 디버그 메시지 사용 (bEnableVehicleDebugOnScreenMessage)", ToolTip="True이면 PIE 중 GEngine AddOnScreenDebugMessage 기반의 개발용 문자열 디버그를 화면에 표시합니다. HUD/Panel UI 표시와는 별도입니다."))
	bool bEnableVehicleDebugOnScreenMessage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="DriveState 디버그 표시 모드 (DriveStateDebugDisplayMode)", ToolTip="PIE 중 화면에 표시할 Drive 상태 디버그 문자열 포맷을 선택합니다. Off는 비표시, SingleLine은 한 줄 요약, MultiLine은 줄바꿈 상세 표시입니다."))
	ECFVehicleDebugDisplayMode DriveStateDebugDisplayMode = ECFVehicleDebugDisplayMode::SingleLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="상태 전이 요약 표시 (bShowDriveStateTransitionSummary)", ToolTip="True이면 현재 Drive 상태와 함께 마지막 상태 전이 요약 문자열도 화면에 표시합니다."))
	bool bShowDriveStateTransitionSummary = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="VehicleDebug HUD 표시 여부 (bShowVehicleDebugHud)", ToolTip="True이면 VehicleDebug HUD 표시를 허용합니다. HUD 위젯에서 이 값을 읽어 표시 여부를 결정할 수 있습니다."))
	bool bShowVehicleDebugHud = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="VehicleDebug Panel 표시 여부 (bShowVehicleDebugPanel)", ToolTip="True이면 VehicleDebug Panel 표시를 허용합니다. 상세 패널 위젯에서 이 값을 읽어 표시 여부를 결정할 수 있습니다."))
	bool bShowVehicleDebugPanel = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="VehicleDebug Text 표시 여부 (bShowVehicleDebugText)", ToolTip="레거시 WBP_VehicleDebug 제거 전환을 위해 기본적으로 비활성 상태로 유지하는 예약용 토글입니다."))
	bool bShowVehicleDebugText = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(DisplayName="VehicleDebug 이벤트 표시 여부 (bShowVehicleDebugEvents)", ToolTip="True이면 향후 Recent Events 표시를 허용합니다. 현재 단계에서는 예약용 토글입니다."))
	bool bShowVehicleDebugEvents = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|Debug", meta=(ClampMin="0.0", DisplayName="디버그 메시지 유지 시간 (DriveStateDebugMessageDuration)", ToolTip="화면에 표시하는 Drive 상태 디버그 메시지 유지 시간(초)입니다."))
	float DriveStateDebugMessageDuration = 0.0f;

	// [v2.48.2] 로컬 조작 차량의 표시 루트 회전 안정화를 사용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|OwnerVisual", meta=(DisplayName="Owner 표시 안정화 사용 (bEnableOwnerVisualStabilization)", ToolTip="True이면 로컬 조작 차량에서 물리 루트는 그대로 두고 차체/휠 표시 루트만 부드럽게 따라가게 합니다. 물리 판정에는 영향을 주지 않습니다."))
	bool bEnableOwnerVisualStabilization = false;

	// [v2.48.0] Owner 표시 루트가 Actor 회전을 따라가는 보간 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|OwnerVisual", meta=(ClampMin="0.1", DisplayName="Owner 표시 안정화 속도 (OwnerVisualStabilizationInterpSpeed)", ToolTip="값이 클수록 차체/휠 표시가 물리 Actor 회전을 빠르게 따라갑니다. 너무 낮으면 차량 표시가 조작보다 늦게 보일 수 있습니다."))
	float OwnerVisualStabilizationInterpSpeed = 4.0f;

	// [v2.48.0] Actor 회전과 Owner 표시 루트 회전 사이에 허용할 최대 지연각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|OwnerVisual", meta=(ClampMin="0.0", DisplayName="Owner 표시 최대 지연각 (OwnerVisualStabilizationMaxLagDeg)", ToolTip="차체/휠 표시 루트가 Actor 회전에서 너무 멀리 떨어지지 않도록 축별 지연각을 제한합니다."))
	float OwnerVisualStabilizationMaxLagDeg = 30.0f;

	// [v2.48.0] Owner 표시 안정화가 Yaw 축에도 적용되는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|OwnerVisual", meta=(DisplayName="Owner 표시 Yaw 안정화 (bOwnerVisualStabilizeYaw)", ToolTip="True이면 좌우 조향 중 보이는 차체/휠 Yaw 흔들림도 표시 루트에서 완충합니다."))
	bool bOwnerVisualStabilizeYaw = false;

	// [v2.48.0] Owner 표시 안정화가 Pitch/Roll 축에 적용되는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|OwnerVisual", meta=(DisplayName="Owner 표시 Pitch/Roll 안정화 (bOwnerVisualStabilizePitchRoll)", ToolTip="True이면 과속방지턱, 착지, 차체 기울어짐 중 보이는 Pitch/Roll 흔들림을 표시 루트에서 완충합니다."))
	bool bOwnerVisualStabilizePitchRoll = false;

	// [v2.48.0] Owner 표시 안정화 중 물리 루트 SkeletalMesh 렌더링을 숨길지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|OwnerVisual", meta=(DisplayName="Owner 물리 메쉬 숨김 (bHideOwnerPhysicsMeshWhenStabilized)", ToolTip="True이면 로컬 조작 차량에서 흔들리는 물리 루트 VehicleMesh 렌더링을 숨기고 안정화된 SM_Body/휠 표시를 사용합니다. 물리 시뮬레이션은 유지됩니다."))
	bool bHideOwnerPhysicsMeshWhenStabilized = false;

	// [v2.53.0] 로컬 조작 차량의 SM_Body만 별도로 부드럽게 표시할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|OwnerBodyVisual", meta=(DisplayName="Owner 차체 표시 안정화 사용 (bEnableOwnerBodyVisualStabilization)", ToolTip="True이면 로컬 조작 차량에서 SM_Body 표시 회전만 부드럽게 따라가게 합니다. 물리와 충돌에는 영향을 주지 않습니다."))
	bool bEnableOwnerBodyVisualStabilization = false;

	// [v2.53.1] SM_Body 표시 회전이 Actor 회전을 따라가는 보간 속도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|OwnerBodyVisual", meta=(ClampMin="0.1", DisplayName="Owner 차체 표시 안정화 속도 (OwnerBodyVisualInterpSpeed)", ToolTip="값이 클수록 로컬 차체 표시가 물리 Actor 회전을 빠르게 따라갑니다. 너무 낮으면 차체만 늦게 움직여 보일 수 있습니다."))
	float OwnerBodyVisualInterpSpeed = 15.0f;

	// [v2.53.1] SM_Body 표시 회전이 Actor 회전에서 벗어날 수 있는 최대 지연각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|OwnerBodyVisual", meta=(ClampMin="0.0", DisplayName="Owner 차체 표시 최대 지연각 (OwnerBodyVisualMaxLagDeg)", ToolTip="차체 표시가 물리 Actor 회전에서 너무 멀리 떨어지지 않도록 축별 지연각을 제한합니다."))
	float OwnerBodyVisualMaxLagDeg = 5.0f;

	// [v2.53.2] SM_Body 표시 안정화를 Yaw 축에도 적용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|OwnerBodyVisual", meta=(DisplayName="Owner 차체 Yaw 안정화 (bOwnerBodyVisualStabilizeYaw)", ToolTip="True이면 좌우 조향 중 보이는 차체 Yaw 흔들림을 SM_Body 표시 회전에서 완충합니다."))
	bool bOwnerBodyVisualStabilizeYaw = false;

	// [v2.53.0] SM_Body 표시 안정화를 Pitch/Roll 축에 적용할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehiclePawn|OwnerBodyVisual", meta=(DisplayName="Owner 차체 Pitch/Roll 안정화 (bOwnerBodyVisualStabilizePitchRoll)", ToolTip="True이면 방지턱, 착지, 차체 기울어짐 중 보이는 Pitch/Roll 흔들림을 SM_Body 표시 회전에서 완충합니다."))
	bool bOwnerBodyVisualStabilizePitchRoll = false;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(ToolTip="차량 입력 전달 전용 DriveComp를 반환합니다."))
	UCFVehicleDriveComp* GetVehicleDriveComp() const { return VehicleDriveComp; }

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(ToolTip="차량 휠 시각 동기화 전용 WheelSyncComp를 반환합니다."))
	UCFWheelSyncComp* GetWheelSyncComp() const { return WheelSyncComp; }

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(ToolTip="차량 중심 피벗 기반 자유 조준을 계산하는 VehicleCameraComp를 반환합니다."))
	UCFVehicleCameraComp* GetVehicleCameraComp() const { return VehicleCameraComp; }

	// [v2.15.0] 차량 Aim 해석 컴포넌트를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(ToolTip="카메라 조준과 향후 무기 발사 사이의 Aim 해석 컴포넌트를 반환합니다."))
	UCFVehicleAimComp* GetVehicleAimComp() const { return VehicleAimComp; }

	// [v2.75.0] 차량 Weapon 해석 컴포넌트를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(ToolTip="차량 하드포인트와 장착 프로파일을 읽어 실제 발사 원점을 계산하는 Weapon 컴포넌트를 반환합니다."))
		UCFVehicleWeaponComp* GetVehicleWeaponComp() const { return VehicleWeaponComp; }

		// [v2.132.0] 출격 피팅 Runtime 상태와 Applied Snapshot을 관리하는 컴포넌트를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(DisplayName="차량 피팅 컴포넌트 반환"))
	UCFVehicleFittingComp* GetVehicleFittingComp() const { return VehicleFittingComp; }

		// [v2.139.0] 현재 출격 장전·예비·예약 탄약 상태를 관리하는 VehicleAmmoComp를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(DisplayName="차량 탄약 컴포넌트 반환", ToolTip="현재 출격의 실제 장전량, 차량 예비량, Launcher 예약과 재장전 상태를 소유하는 VehicleAmmoComp를 반환합니다."))
	UCFVehicleAmmoComp* GetVehicleAmmoComp() const { return VehicleAmmoComp; }

		// [v2.142.0] 현재 활성 WeaponInstance의 FullMagazine 재장전을 Gameplay 명령으로 요청합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Ammo", meta=(DisplayName="현재 무기 재장전 요청", ToolTip="현재 활성 MountProfileId의 유한탄 무기에 FullMagazine 재장전을 요청합니다. Launcher Sequence나 다른 Action Lock 중에는 거부되며 입력 에셋과는 독립된 Gameplay 명령입니다."))
	ECFAmmoTransactionResult RequestReloadCurrentWeapon();

	// [v2.151.0] Applied Fitting의 Player-facing 고정 표시 순서에서 지정 무기를 선택하고 실제 활성 Weapon Runtime과 단일 Turret Visual을 함께 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Weapon", meta=(DisplayName="무기 선택 순번 요청", ToolTip="Applied Fitting의 실제 고정 표시 순서에서 0-based 무기 순번을 선택합니다. 진행 중 Launcher는 WeaponChanged로 정상 취소한 뒤 전환하며 내부 MountProfileId를 UI 그룹 이름으로 사용하지 않습니다."))
	bool RequestSelectWeaponIndex(int32 NewWeaponIndex);

		// [v2.126.0] 차량 Ripple·Salvo 발사 시퀀스를 관리하는 LauncherComp를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(DisplayName="런처 컴포넌트 반환 (Get Launcher Component)", ToolTip="현재 차량의 Ripple·Salvo 예약 발사, 취소와 Volley Debug를 관리하는 LauncherComp를 반환합니다."))
	UCFLauncherComp* GetLauncherComp() const { return LauncherComp; }

	// [v2.82.0] 차량 Projectile Pool 컴포넌트를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(ToolTip="반복 발사되는 Projectile Actor를 재사용하는 Pool 컴포넌트를 반환합니다."))
	UCFProjectilePoolComp* GetProjectilePoolComp() const { return ProjectilePoolComp; }

			// [v2.111.0] 차량 내구도 컴포넌트를 기존 이름으로 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(DisplayName="차량 내구도 컴포넌트 반환", ToolTip="현재 차량의 최대 내구도, 현재 내구도와 파괴 상태를 관리하는 VehicleHealthComp를 반환합니다."))
	UCFVehicleHealthComp* GetVehicleHealthComp() const { return VehicleHealthComp; }

	// [v2.129.0] 차량 쉴드와 방향 장갑을 관리하는 방어 컴포넌트를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(DisplayName="차량 방어 컴포넌트 반환", ToolTip="현재 차량의 쉴드, 6방향 장갑, 관통과 차량 내구도 피해 분배를 관리하는 VehicleDefenseComp를 반환합니다."))
	UCFVehicleDefenseComp* GetVehicleDefenseComp() const { return VehicleDefenseComp; }

	// 표준 차체 시각·피격 컴포넌트인 SM_Body를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn", meta=(DisplayName="차량 차체 메쉬 컴포넌트 반환", ToolTip="이름이 SM_Body인 표준 차체 StaticMeshComponent를 반환합니다. 파괴 FX 소켓과 차체 Bounds 해석에 사용합니다."))
	UStaticMeshComponent* GetVehicleBodyMeshComponent() const;

	// 차량별 선택 대표 위치를 제공하는 타겟 포인트 컴포넌트를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|TargetSelect", meta=(DisplayName="타겟 포인트 컴포넌트 반환", ToolTip="차량별 선택 대표 위치를 제공하는 TargetPointComp를 반환합니다."))
	UCFTargetPointComp* GetTargetPointComp() const { return TargetPointComp; }

				// 차량의 공용 타겟 선택 상태 컴포넌트를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|TargetSelect", meta=(DisplayName="타겟 선택 컴포넌트 반환", ToolTip="현재 후보와 지속 선택 대상을 관리하는 TargetSelectComp를 반환합니다."))
				UCFTargetSelectComp* GetTargetSelectComp() const { return TargetSelectComp; }

		// [v2.145.0] 차량의 독립 Sensor Contact/Knowledge Runtime 컴포넌트를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Sensor", meta=(DisplayName="차량 센서 컴포넌트 반환", ToolTip="TargetSelect와 독립적으로 Sensor Contact, Player Knowledge와 Actor-free Snapshot을 소유하는 VehicleSensorComp를 반환합니다."))
	UCFVehicleSensorComp* GetVehicleSensorComp() const { return VehicleSensorComp; }

	// [v2.146.0] Pawn 입력 계층에서 VehicleSensorComp의 Active Scan 시작 명령을 요청합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Sensor|Input", meta=(DisplayName="Active Scan 시작 요청", ToolTip="현재 VehicleSensorComp에 Active Scan 시작을 요청합니다. Sensor Runtime이 준비되지 않았거나 Active Scan을 지원하지 않거나 이미 실행 중이면 False를 반환합니다. SensorData 적용이나 Runtime 초기화는 수행하지 않습니다."))
	bool RequestStartActiveScan();

	// [v2.146.0] Pawn 입력 계층에서 VehicleSensorComp의 Active Scan 중단 명령을 요청합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Sensor|Input", meta=(DisplayName="Active Scan 중단 요청", ToolTip="현재 VehicleSensorComp에 실행 중 Active Scan 중단을 요청합니다. 실행 중인 Scan이 없으면 False를 반환하며 SensorData, Contact와 획득 Knowledge를 초기화하지 않습니다."))
	bool RequestStopActiveScan();

	// 현재 유효 후보를 지속 선택 대상으로 확정합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|TargetSelect", meta=(DisplayName="현재 후보 선택 확정 (Confirm Current Target Candidate)", ToolTip="현재 TargetSelectComp 후보가 유효할 때만 지속 선택 대상으로 설정합니다. 후보가 없거나 무효하면 False를 반환하고 기존 선택을 유지합니다."))
	bool ConfirmCurrentTargetCandidate();

	// 현재 지속 선택 대상만 Manual 사유로 해제합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|TargetSelect", meta=(DisplayName="선택 타겟 수동 해제 (Clear Selected Target Manually)", ToolTip="현재 선택 대상만 Manual 사유로 해제하고 현재 후보는 유지합니다. 선택 대상이 없으면 False를 반환합니다."))
	bool ClearSelectedTargetManually();

	// [v2.118.0] 현재 차량이 주어진 컨텍스트에서 선택 가능한지 반환합니다.
	virtual bool IsTargetSelectable_Implementation(const FCFTargetSelectionContext& SelectionContext) const override;

	// [v2.118.0] 타겟 HUD와 장비가 사용할 차량 기본 표시 정보를 반환합니다.
	virtual FCFTargetDisplayInfo GetTargetDisplayInfo_Implementation() const override;

	// [v2.118.0] 별도 TargetPoint가 연결되기 전 사용할 차량 Bounds 중심을 반환합니다.
	virtual FVector GetTargetSelectionLocation_Implementation() const override;

	// [v2.118.0] 파괴되지 않은 차량의 P0 기본 추적 상태를 반환합니다.
	virtual ECFTargetTrackState GetTargetTrackState_Implementation() const override;

	// [v2.97.0] Projectile Pool에서 반환된 Hit 발사체를 Damage HitContext Debug로 기록합니다.
	void RecordProjectileDamageHitContextFromPool(const ACFProjectileActor* InProjectileActor);

	// [v2.102.0] Reticle / FireFeedback UI가 읽을 현재 로컬 발사 피드백 표시 데이터를 만듭니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|FireFeedback", meta=(DisplayName="FireFeedback 표시 데이터 만들기 (BuildFireFeedbackViewData)", ToolTip="마지막 로컬 발사 결과, 무기 쿨다운, 로컬 조준 상태를 Reticle / FireFeedback UI 표시 데이터로 변환합니다."))
	FCFVehicleFireFeedbackViewData BuildFireFeedbackViewData() const;

	// [v2.86.0] 현재 터렛 시각 장착 요약 문자열을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Turret", meta=(ToolTip="현재 활성 장착 프로파일의 터렛 시각 메쉬 장착 상태 요약을 반환합니다."))
	FString GetLastTurretVisualSummary() const { return LastTurretVisualSummary; }

	// [v2.20.0] 현재 Pawn에서 Aim Reticle 위젯을 표시할 수 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Aim|Reticle", meta=(ToolTip="현재 Pawn에서 Aim Reticle 위젯을 표시할 수 있는지 반환합니다. 로컬 제어 Pawn에서만 True가 될 수 있습니다."))
	bool ShouldShowAimReticle() const;

		// [v2.148.0] 현재 Pawn이 UISubsystem의 Current Pawn일 때 UISubsystem 소유 Aim Reticle을 호환 반환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Aim|Reticle", meta=(ToolTip="UI-P0-04 호환 API입니다. Pawn이 직접 위젯을 만들지 않고 현재 UISubsystem 소유 Aim Reticle을 반환하며, 현재 Possessed Pawn이 아니면 Null입니다."))
	UCFAimReticleWidget* CreateAimReticleWidget();

	// [v2.148.0] UI-P0-04 이전 Pawn 직접 생성 Reticle 인스턴스만 안전하게 정리합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Aim|Reticle", meta=(ToolTip="Legacy Pawn 직접 생성 Reticle 인스턴스가 남아 있을 때만 제거합니다. UISubsystem 소유 Reticle 수명은 변경하지 않습니다."))
	void DestroyAimReticleWidget();

	// [v2.148.0] 현재 Pawn에 연결된 UISubsystem 소유 Reticle의 표시 상태를 호환 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Aim|Reticle", meta=(ToolTip="현재 Pawn이 UISubsystem의 Current Pawn이면 UISubsystem 소유 Reticle의 Pawn 참조와 표시 상태를 갱신합니다."))
	void RefreshAimReticleWidget();

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|TargetSelect|HUD", meta=(DisplayName="타겟 선택 HUD 표시 가능 여부 반환"))
	bool ShouldShowTargetSelectHud() const;

		// [v2.149.0] 현재 Pawn이 UISubsystem Current Pawn일 때 UISubsystem 소유 TargetSelect Marker를 호환 반환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|TargetSelect|HUD", meta=(DisplayName="타겟 선택 HUD 생성", ToolTip="UI-P0-05 호환 API입니다. Pawn이 직접 위젯을 만들지 않고 현재 UISubsystem 소유 Target Marker를 반환하며, 현재 Possessed Pawn이 아니면 Null입니다."))
	UCFTargetSelectWidget* CreateTargetSelectWidget();

	// [v2.149.0] UI-P0-05 이전 Pawn 직접 생성 TargetSelect 인스턴스만 안전하게 정리합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|TargetSelect|HUD", meta=(DisplayName="타겟 선택 HUD 제거", ToolTip="Legacy Pawn 직접 생성 TargetSelect 인스턴스가 남아 있을 때만 제거합니다. UISubsystem 소유 Marker 수명은 변경하지 않습니다."))
	void DestroyTargetSelectWidget();

	// [v2.149.0] 현재 Pawn에 연결된 UISubsystem 소유 Target Marker의 표시 상태를 호환 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|TargetSelect|HUD", meta=(DisplayName="타겟 선택 HUD 갱신", ToolTip="현재 Pawn이 UISubsystem Current Pawn이면 UISubsystem 소유 Target Marker의 Pawn 참조와 표시 상태를 갱신합니다."))
	void RefreshTargetSelectWidget();

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Input", meta=(ToolTip="현재 플레이어 컨트롤러 기준으로 기본 Input Mapping Context 등록을 시도합니다."))
	bool RegisterDefaultInputMappingContext();

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn", meta=(ToolTip="Drive / WheelSync 캐시와 준비를 다시 시도합니다."))
	bool InitializeVehicleRuntime();

#if WITH_EDITOR
	// [v2.74.0] SM_Body 차체 메시 소켓에서 휠 앵커와 선택 하드포인트 위치를 캡처해 VehicleData에 기록합니다.
	UFUNCTION(CallInEditor, BlueprintCallable, Category="CarFight|VehiclePawn|Editor", meta=(DisplayName="메시 소켓에서 차량 레이아웃 캡처 (Capture Vehicle Layout From Body Sockets)", ToolTip="SM_Body에 적용된 차체 메시의 휠 소켓과 VehicleData.HardpointSlots의 SocketName을 읽어 차량 레이아웃 값을 기록합니다. 휠 소켓이나 앵커가 하나라도 없으면 DataAsset을 수정하지 않지만, 하드포인트 SocketName이 비어 있거나 누락된 경우는 경고만 표시합니다."))
	void CaptureWheelLayoutFromBodySockets();

	// [v2.69.0] VehicleData에 저장된 휠 앵커 레이아웃을 에디터 프리뷰 Pawn에 다시 적용합니다.
	UFUNCTION(CallInEditor, BlueprintCallable, Category="CarFight|VehiclePawn|Editor", meta=(DisplayName="데이터에서 휠 레이아웃 적용 (Apply Vehicle Layout From Data)", ToolTip="현재 VehicleData.VehicleLayoutConfig 값을 BP의 Wheel_Anchor_* 컴포넌트에 다시 적용합니다. 캡처 후 결과 확인용입니다."))
	void ApplyVehicleLayoutFromDataInEditor();
#endif

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn", meta=(ToolTip="WheelSync의 캐시와 준비를 다시 시도합니다."))
	bool PrepareWheelSync();

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn", meta=(ToolTip="WheelSync를 사용해 휠 시각 갱신을 한 번 실행합니다."))
	bool UpdateVehicleWheelVisuals(float DeltaSeconds);

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Input", meta=(ToolTip="Throttle 입력을 DriveComp로 전달합니다."))
	void SetVehicleThrottleInput(float InThrottleValue);

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Input", meta=(ToolTip="Steering 입력을 DriveComp로 전달합니다."))
	void SetVehicleSteeringInput(float InSteeringValue);

	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Input", meta=(ToolTip="Brake 입력을 DriveComp로 전달합니다."))
	void SetVehicleBrakeInput(float InBrakeValue);

		UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Input", meta=(ToolTip="Handbrake 입력을 DriveComp로 전달합니다."))
	void SetVehicleHandbrakeInput(bool bInHandbrakePressed);

	// [v2.131.0] Pause 진입 전 모든 차량 Gameplay 입력과 입력 소유권을 안전한 중립 상태로 초기화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehiclePawn|Input", meta=(DisplayName="Pause용 Gameplay 입력 초기화 (Clear Gameplay Input For Pause)", ToolTip="Pause 진입 전에 이동, 조향, 브레이크, 핸드브레이크와 Look 입력을 중립화하고 입력 소유권을 해제합니다. 진행 중 Launcher 시퀀스는 취소하지 않습니다."))
	void ClearGameplayInputForPause();

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Drive", meta=(ToolTip="VehicleDriveComp 기준 현재 차량 전체 속도(km/h)를 반환합니다."))
	float GetVehicleSpeed() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Drive", meta=(ToolTip="VehicleDriveComp 기준 현재 차량 Drive 상태를 반환합니다."))
	ECFVehicleDriveState GetDriveState() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Drive", meta=(ToolTip="VehicleDriveComp 기준 현재 차량 Drive 상태 스냅샷을 반환합니다."))
	FCFVehicleDriveStateSnapshot GetDriveStateSnapshot() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="Pawn 기준 런타임 준비 상태, Drive 상태, 마지막 전이 요약을 하나로 묶은 디버그 스냅샷을 반환합니다."))
	FCFVehicleDebugSnapshot GetVehicleDebugSnapshot() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="HUD 표시용 VehicleDebug Overview 카테고리를 반환합니다."))
	FCFVehicleDebugOverview GetVehicleDebugOverview() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="상세 패널 표시용 VehicleDebug Drive 카테고리를 반환합니다."))
	FCFVehicleDebugDrive GetVehicleDebugDrive() const;

		UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="상세 패널 표시용 VehicleDebug Input 카테고리를 반환합니다."))
	FCFVehicleDebugInput GetVehicleDebugInput() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="상세 패널 표시용 VehicleDebug Camera 카테고리를 반환합니다."))
	FCFVehicleDebugCamera GetVehicleDebugCamera() const;

	// [v2.16.0] 상세 패널 표시용 VehicleDebug Aim 카테고리를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="상세 패널 표시용 VehicleDebug Aim 카테고리를 반환합니다."))
		FCFVehicleDebugAim GetVehicleDebugAim() const;

	// [v2.138.0] 상세 패널 표시용 VehicleDebug Target 카테고리를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="현재 선택 대상의 표시 정보, 추적 상태, 방어와 내구도 상태를 담은 VehicleDebug Target 카테고리를 반환합니다."))
	FCFVehicleDebugTarget GetVehicleDebugTarget() const;

	// [v2.76.0] 상세 패널 표시용 VehicleDebug Weapon 카테고리를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="상세 패널 표시용 VehicleDebug Weapon 카테고리를 반환합니다."))
	FCFVehicleDebugWeapon GetVehicleDebugWeapon() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="상세 패널 표시용 VehicleDebug Runtime 카테고리를 반환합니다."))
	FCFVehicleDebugRuntime GetVehicleDebugRuntime() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="Debug Widget의 단일 라인 Text 바인딩에 바로 사용할 수 있는 Pawn 디버그 문자열을 반환합니다."))
	FText GetDebugTextSingleLine() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="Debug Widget의 멀티라인 Text 바인딩에 바로 사용할 수 있는 Pawn 디버그 문자열을 반환합니다."))
	FText GetDebugTextMultiLine() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="현재 DriveStateDebugDisplayMode 설정을 따라 Debug Widget Text 바인딩에 바로 사용할 수 있는 Pawn 디버그 문자열을 반환합니다. Off이면 빈 텍스트를 반환합니다."))
	FText GetDebugTextByDisplayMode() const;

	// [v2.14.3] HUD/Panel 공통 기준이 되는 VehicleDebug UI 표시 가능 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="현재 VehicleDebug HUD/Panel 공통 기준이 되는 표시 가능 여부를 반환합니다. 기본 디버그 스위치가 꺼져 있으면 False를 반환합니다."))
	bool ShouldShowVehicleDebugUi() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="레거시 WBP_VehicleDebug 제거 전환을 위해 현재 Debug Widget을 표시하지 않도록 고정된 False를 반환합니다."))
	bool ShouldShowDebugWidget() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="현재 VehicleDebug HUD를 표시해야 하는지 여부를 반환합니다. 로컬 차량 디버그 표시 조건과 HUD 토글을 함께 검사합니다."))
	bool ShouldShowVehicleDebugHud() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="현재 VehicleDebug Panel을 표시해야 하는지 여부를 반환합니다. 로컬 차량 디버그 표시 조건과 Panel 토글을 함께 검사합니다."))
	bool ShouldShowVehicleDebugPanel() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="레거시 WBP_VehicleDebug 제거 전환을 위해 현재 VehicleDebug Legacy Text View를 표시하지 않도록 고정된 False를 반환합니다."))
	bool ShouldShowVehicleDebugText() const;

	UFUNCTION(BlueprintPure, Category="CarFight|VehiclePawn|Debug", meta=(ToolTip="레거시 WBP_VehicleDebug 제거 전환을 위해 항상 Collapsed를 반환합니다."))
	ESlateVisibility GetDebugWidgetVisibility() const;

protected:
	// [v2.61.0] 차량 Pawn의 싱글플레이 기본 복제 상태를 적용합니다.
	void ApplyVehicleSinglePlayerBaseline();

	// [v2.48.0] 로컬 Owner 표시 안정화용 차체/휠 표시 계층을 준비합니다.
	bool PrepareOwnerVisualStabilization();

	// [v2.48.0] 지정한 표시 컴포넌트를 Owner 표시 루트 아래로 안전하게 이동합니다.
	bool AttachOwnerVisualComponent(USceneComponent* VisualComponent);

	// [v2.48.0] 로컬 Owner 표시 루트 회전을 현재 Actor 회전에 부드럽게 맞춥니다.
	void UpdateOwnerVisualStabilization(float DeltaSeconds);

	// [v2.48.0] Owner 표시 안정화 상태를 기본 회전으로 되돌립니다.
	void ResetOwnerVisualStabilization();

	// [v2.48.0] Actor 회전과 표시 회전 사이의 지연각을 설정 한도 안으로 제한합니다.
	FRotator ClampOwnerVisualStabilizedRotation(const FRotator& CurrentActorRotation, const FRotator& DesiredVisualRotation) const;

	// [v2.53.0] 로컬 조작 차량의 SM_Body 표시 회전을 부드럽게 안정화합니다.
	void UpdateOwnerBodyVisualStabilization(float DeltaSeconds);

	// [v2.53.0] 로컬 조작 차량의 SM_Body 표시 안정화 상태를 기본 상태로 되돌립니다.
	void ResetOwnerBodyVisualStabilization();

	// [v2.53.0] Actor 회전과 SM_Body 표시 회전 사이의 지연각을 설정 한도 안으로 제한합니다.
	FRotator ClampOwnerBodyVisualRotation(const FRotator& CurrentActorRotation, const FRotator& DesiredVisualRotation) const;

		// [v1.5.0] DriveComp 캐시를 재사용해 VehicleMovement 컴포넌트를 안전하게 가져옵니다.
	UChaosWheeledVehicleMovementComponent* ResolveVehicleMovementComponent(const TCHAR* CacheFailureSummary, const TCHAR* MissingComponentSummary);

	// [v2.133.0] PreRegister에서 Cached Snapshot Target을 Movement Mass에 적용하거나 실패 시 Legacy 입력으로 복원합니다.
	bool PrepareInitialSortieRuntimeMass();

	// [v2.133.0] BeginPlay에서 Movement 설정 질량과 VehicleMesh 실제 질량·PhysicsAsset 계약을 검증합니다.
	bool VerifyInitialSortieRuntimeMass();

	// [v2.14.1] Snapshot 기반 단일 라인 차량 디버그 문자열을 생성합니다.
	FString BuildVehicleDebugTextSingleLine(const FCFVehicleDebugSnapshot& VehicleDebugSnapshot, bool bIncludeRuntimeSummary, bool bIncludeTransitionSummary, bool bIncludeInputState) const;

	// [v2.14.1] Snapshot 기반 멀티라인 차량 디버그 문자열을 생성합니다.
	FString BuildVehicleDebugTextMultiLine(const FCFVehicleDebugSnapshot& VehicleDebugSnapshot, bool bIncludeRuntimeSummary, bool bIncludeTransitionSummary, bool bIncludeInputState) const;

	// [v2.14.1] 현재 설정에 맞는 차량 디버그 요약 문자열을 Snapshot 기반으로 생성합니다.
	FString BuildVehicleDebugSummary(bool bUseMultilineFormat, bool bIncludeRuntimeSummary, bool bIncludeTransitionSummary, bool bIncludeInputState) const;

	// [v1.5.0] WheelSync 실행 결과를 기존 런타임 요약 뒤에 덧붙입니다.
	void AppendWheelSyncRuntimeSummary();

	// [v1.5.0] 축 입력 액션값을 읽어 허용 여부를 검사한 뒤 지정 setter로 전달합니다.
	void ApplyAxisInputFromAction(const UInputAction* SourceInputAction, const FInputActionValue& InputActionValue, void (ACFVehiclePawn::*AxisInputSetter)(float));

	// [v1.5.0] 지정된 축 입력 setter를 0으로 초기화합니다.
	void ResetAxisInput(void (ACFVehiclePawn::*AxisInputSetter)(float));

	// [v1.6.0] 2D 이동 입력 벡터를 위=0도, 시계 방향 증가 각도로 변환합니다.
	float ConvertMoveInputToAngleDeg(const FVector2D& MoveInputVector) const;

	// [v1.6.0] 주어진 각도가 시작/종료 각도 포함 범위 안에 들어오는지 검사합니다.
	bool IsAngleWithinRange(float InAngleDeg, float StartAngleDeg, float EndAngleDeg) const;

	// [v1.6.0] 현재 속도 기준으로 마지막 진행 방향 의도의 fallback 값을 계산합니다.
	ECFVehicleMoveDirectionIntent ResolveDirectionIntentFallback() const;

	// [v1.6.0] 2D 이동 입력 벡터를 차량 이동 해석 결과로 변환합니다.
	FCFVehicleMoveInputResult ResolveVehicleMoveInput(const FVector2D& MoveInputVector) const;

	// [v1.6.0] 해석된 차량 이동 입력 결과를 DriveComp 입력으로 적용합니다.
	void ApplyResolvedVehicleMoveInput(const FCFVehicleMoveInputResult& ResolvedMoveInput);

		// [v2.63.0] 현재 Aim 상태를 기준으로 로컬 발사 명령 데이터를 생성합니다.
	FCFVehicleFireRequest BuildFireCommand();

	// [v2.126.0] 현재 Muzzle을 다시 해결하면서 첫 입력 순간 Command Target을 유지할 후속 발사 명령을 생성합니다.
	FCFVehicleFireRequest BuildFireCommandForTarget(const FVector& OverrideCommandTargetLocation, bool bUseOverrideTarget);

	// [v2.63.0] 싱글플레이 로컬 발사 명령을 최소 검증하고 결과를 채웁니다.
	bool ValidateFireCommand(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& OutFireResult);

	// [v2.126.0] 후속 Ripple·Salvo 발사에서 입력 단위 쿨다운만 선택적으로 우회해 나머지 발사 조건을 동일하게 검증합니다.
	bool ValidateFireCommandInternal(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& OutFireResult, bool bIgnoreWeaponCooldown);

	// [v2.97.0] 싱글플레이 로컬 더미 HitScan Trace를 실행하고 FireResult와 Damage HitContext Debug에 결과를 채웁니다.
	bool RunLocalDummyHitScan(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& InOutFireResult);

	// [v2.97.0] Dummy HitScan 결과를 Damage HitContext Debug로 기록합니다.
	void RecordDummyHitScanDamageHitContext(const FCFVehicleFireRequest& FireCommand, const FCFVehicleFireResult& FireResult, const FHitResult* HitResult, bool bBlockingHit);

	// [v2.97.0] 마지막 Damage HitContext Debug와 표시 요약을 저장합니다.
		void StoreLastDamageHitContext(const FCFDamageHitContext& InDamageHitContext);

	// [v2.111.0] 마지막 피해 적용 결과와 표시 요약을 저장합니다.
	void StoreLastDamageApplyResult(FCFDamageApplyResult InDamageApplyResult);

	// [v2.97.0] VehicleDebug Panel에 표시할 Damage HitContext 요약 문자열을 생성합니다.
	FString BuildDamageHitContextSummary(const FCFDamageHitContext& InDamageHitContext) const;

	// [v2.93.0] Turret Pitch 메쉬의 Muzzle 소켓으로 최종 FireOrigin을 보정합니다.
	bool TryBuildMuzzleFireOrigin(FCFVehicleFireOrigin& InOutFireOrigin, FString& OutFireOriginSummary) const;

		// [v2.126.0] Muzzle 위치에서 현재 Reticle 또는 명시적 Volley 목표점으로 향하는 공통 Weapon Aim Solution을 계산합니다.
	bool BuildWeaponAimSolution(FCFVehicleWeaponAimSolution& OutWeaponAimSolution, FCFVehicleFireOrigin* OutFireOrigin = nullptr, FString* OutFireOriginSummary = nullptr, const FVector* OverrideAimTargetLocation = nullptr) const;

	// [v2.109.0] 현재 Weapon Aim Solution을 다시 계산해 AimComp에 저장합니다.
	void RefreshWeaponAimSolution();

	// [v2.81.0] 현재 활성 무기가 Projectile Actor 스폰 경로를 사용할 수 있는지 반환합니다.
	bool ShouldUseProjectileActorFire() const;

				// [v2.136.0] 현재 WeaponData Release 설정, 실제 Muzzle Transform과 명시적 첫 발사 목표 Actor Snapshot으로 Launch Context를 생성합니다.
	bool BuildDirectProjectileLaunchContext(const FCFVehicleFireRequest& FireCommand, const UCFProjectileData& InProjectileData, AActor* GuidanceTargetActorSnapshot, FCFProjectileLaunchContext& OutLaunchContext) const;

	// [v2.136.0] 현재 활성 ProjectileData와 현재 선택 목표 Snapshot을 사용해 Projectile Actor를 Pool로 확보하는 호환 경로입니다.
	bool TrySpawnProjectileActorFromFireCommand(const FCFVehicleFireRequest& FireCommand);

	// [v2.136.0] 검증 승인된 명령을 명시적 목표 Actor Snapshot과 함께 Projectile 또는 HitScan 실행 경로로 처리합니다.
	bool ExecuteAcceptedFireCommand(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& InOutFireResult, AActor* GuidanceTargetActorSnapshot, bool bAllowProjectileFallback);

	// [v2.136.0] LauncherComp가 예약한 후속 발사를 첫 발사 순간 위치·Actor Snapshot과 다음 Muzzle로 실행합니다.
	bool ExecuteScheduledLauncherShot(int32 VolleyId, int32 SequenceShotIndex, const FVector& CommandTargetLocation, AActor* GuidanceTargetActorSnapshot);

	// [v2.17.0] 클라이언트 또는 서버 로컬 입력에서 발사 요청을 시작합니다.
				void HandleFireStarted(const FInputActionValue& InputActionValue);

	// 현재 후보 선택 입력을 처리합니다.
	void HandleSelectTargetStarted(const FInputActionValue& InputActionValue);

				// 현재 선택 대상 수동 해제 입력을 처리합니다.
	void HandleClearTargetStarted(const FInputActionValue& InputActionValue);

	// [v2.152.0] Axis1D의 1-based weapon ordinal 입력을 기존 0-based Weapon Selection Gameplay command로 변환합니다.
	void HandleSelectWeaponStarted(const FInputActionValue& InputActionValue);

	// [v2.146.0] optional Active Scan 시작 InputAction을 Pawn Gameplay command로 변환합니다.
	void HandleStartActiveScanStarted(const FInputActionValue& InputActionValue);

	// [v2.146.0] optional Active Scan 중단 InputAction을 Pawn Gameplay command로 변환합니다.
	void HandleStopActiveScanStarted(const FInputActionValue& InputActionValue);

		// [v2.63.0] 로컬 발사 결과를 Pawn과 AimComp 상태에 반영합니다.
	void ApplyFireResult(const FCFVehicleFireRequest& FireCommand, const FCFVehicleFireResult& FireResult);

	// [v2.126.0] 승인 발사 결과의 Muzzle·FX·피드백은 유지하면서 쿨다운 기록 여부만 호출자가 선택하게 합니다.
	void ApplyFireResultInternal(const FCFVehicleFireRequest& FireCommand, const FCFVehicleFireResult& FireResult, bool bRecordCooldown);

	void ApplyVehicleDataConfig();
	void ApplyVehicleVisualConfig();
	// [v2.108.0] VehicleMesh는 무기 채널을 무시하고 SM_Body만 시각 피격 표면으로 구성합니다.
	void ConfigureVehicleVisualHitCollision();
	// [v2.86.0] VehicleData MountProfile의 터렛 시각 메쉬를 하드포인트 위치에 붙입니다.
	void ApplyVehicleTurretVisualConfig();
	// [v2.89.0] 현재 Aim 상태에서 터렛이 바라볼 월드 방향을 계산합니다.
	FVector ResolveTurretAimWorldDirection() const;
	// [v2.89.0] WeaponComp가 계산한 터렛 Yaw / Pitch 각도를 시각 피벗 컴포넌트에 적용합니다.
	void UpdateVehicleTurretAimVisuals(float DeltaSeconds);
	void ApplyVehicleLayoutConfig();
	void ApplyVehicleMovementConfig();
	// [v1.2.0] VehicleMovementConfig 중 런타임 setter가 가능한 휠 물리 값을 차량 인스턴스 기준으로 적용합니다.
	void ApplyVehicleWheelPhysicsConfig();
	void ApplyVehicleWheelVisualConfig();
	void ApplyVehicleReferenceConfig();
		void DisplayDriveStateOnScreenDebug() const;
	bool ShouldAcceptActionInput(const UInputAction* SourceInputAction, float CurrentInputValue) const;
	bool HasActiveMappedKeyForDevice(const UInputAction* SourceInputAction, bool bRequireGamepadKey) const;
	bool IsMappedKeyCurrentlyActive(const FKey& MappingKey) const;

	// [v2.6.1] P1 입력 충돌 방지: 현재 입력값이 소유권 판단에 사용할 만큼 유효한지 검사합니다.
	bool IsMeaningfulInputValue(float CurrentInputValue) const;

	// [v2.6.1] P1 입력 충돌 방지: 2D VehicleMove 입력이 현재 입력 소유권을 획득할 수 있는지 검사합니다.
	bool CanProcessVehicleMoveInput(float MoveInputMagnitude) const;

	// [v2.6.1] P1 입력 충돌 방지: 기존 축 입력이 현재 입력 소유권을 획득할 수 있는지 검사합니다.
	bool CanProcessLegacyAxisInput(float AxisValue) const;

	// [v2.6.1] P1 입력 충돌 방지: VehicleMove 입력 기준으로 입력 소유권 상태를 갱신합니다.
	void UpdateInputOwnershipFromVehicleMove(float MoveInputMagnitude);

	// [v2.6.1] P1 입력 충돌 방지: LegacyAxis 입력 기준으로 입력 소유권 상태를 갱신합니다.
	void UpdateInputOwnershipFromLegacyAxis(float AxisValue);

	// [v2.6.1] P1 입력 충돌 방지: 현재 유효 입력이 없으면 입력 소유권을 해제합니다.
		void ReleaseInputOwnershipIfIdle();

	// [v2.8.0] VehicleMove 2D 입력 방향에서 목표 조향값을 계산합니다.
	float CalculateVehicleMoveTargetSteering(const FVector2D& MoveInputVector, float MoveInputMagnitude) const;

	// [v2.8.0] 현재 차량 속도 기준 중립 복귀 속도를 계산합니다.
	float CalculateSteeringReturnRateKmh(float SpeedKmh) const;

	// [v2.55.0] 현재 차량 속도 기준으로 실제 Chaos Vehicle에 전달할 조향 입력을 계산합니다.
	float CalculateSpeedLimitedSteeringInput(float RawSteeringInput) const;

	// [v2.44.0] 현재 입력 경로의 목표 조향값을 제한 속도로 추적해 실제 조향 입력으로 적용합니다.
	void UpdateVehicleMoveSteeringInput(float DeltaSeconds);

	// [v1.6.0] 차량 이동용 2D 입력 액션값을 읽어 해석 결과를 Drive 입력으로 전달합니다.


	void HandleVehicleMoveInput(const FInputActionValue& InputActionValue);

	// [v1.6.0] 차량 이동용 2D 입력 액션이 해제됐을 때 이동 입력 상태를 초기화합니다.
	void HandleVehicleMoveReleased(const FInputActionValue& InputActionValue);

	void HandleThrottleInput(const FInputActionValue& InputActionValue);
	void HandleThrottleReleased(const FInputActionValue& InputActionValue);
	void HandleSteeringInput(const FInputActionValue& InputActionValue);
	void HandleSteeringReleased(const FInputActionValue& InputActionValue);
	void HandleBrakeInput(const FInputActionValue& InputActionValue);
	void HandleBrakeReleased(const FInputActionValue& InputActionValue);
	void HandleLookInput(const FInputActionValue& InputActionValue);
	void HandleLookReleased(const FInputActionValue& InputActionValue);
	void HandleHandbrakeStarted(const FInputActionValue& InputActionValue);
	void HandleHandbrakeCompleted(const FInputActionValue& InputActionValue);

private:
	// [v2.87.0] 마지막 터렛 시각 장착에 사용한 TurretMountData입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFTurretMountData> LastTurretMountData = nullptr;

	// [v2.101.0] 마지막 터렛 시각 장착에서 EquipmentPresetData 기반 TurretMountData가 해석되어 있었는지 여부입니다.
	bool bLastTurretMountDataAssigned = false;

	// [v2.87.0] 마지막 터렛 시각 장착에 사용한 TurretMountData ID입니다.
	FName LastTurretMountId = NAME_None;

	// [v2.87.0] 마지막 터렛 시각 장착에 사용한 TurretMountData 요약입니다.
	FString LastTurretMountSummary = TEXT("TurretMountData: NotInitialized");

	// [v2.86.0] 현재 활성 장착 프로파일 기준 터렛 시각 메쉬가 붙었는지 여부입니다.
	bool bLastTurretVisualAttached = false;

	// [v2.86.0] 현재 활성 장착 프로파일 기준 터렛 시각 장착 요약입니다.
	FString LastTurretVisualSummary = TEXT("TurretVisual: NotInitialized");

	// [v2.88.0] 마지막으로 적용한 터렛 Base 메쉬 이름입니다.
	FName LastTurretBaseMeshName = NAME_None;

	// [v2.88.0] 마지막으로 적용한 터렛 Yaw 메쉬 이름입니다.
	FName LastTurretYawMeshName = NAME_None;

	// [v2.88.0] 마지막으로 적용한 터렛 Pitch 메쉬 이름입니다.
	FName LastTurretPitchMeshName = NAME_None;

	// [v2.86.0] 터렛 시각 장착에 사용할 활성 MountProfile을 찾습니다.
	const FCFVehicleMountProfile* FindActiveTurretMountProfile() const;

	// [v2.86.0] 터렛 시각 장착에 사용할 하드포인트 슬롯을 찾습니다.
	const FCFVehicleHardpointSlot* FindTurretHardpointSlot(FName LocationSlotId) const;

	// [v2.86.0] 터렛 시각 컴포넌트를 기본 숨김 상태로 되돌립니다.
	void ResetTurretVisualComponents();

	// [v2.48.0] Owner 표시 안정화 계층 준비가 완료됐는지 여부입니다.
	bool bOwnerVisualStabilizationReady = false;

	// [v2.48.0] Owner 표시 안정화용 이전 프레임 표시 회전값입니다.
	FRotator SmoothedOwnerVisualRotation = FRotator::ZeroRotator;

	// [v2.48.0] Owner 표시 안정화 회전 기준값이 유효한지 여부입니다.
	bool bHasSmoothedOwnerVisualRotation = false;

	// [v2.48.0] Owner 표시 루트 아래로 이동한 표시 컴포넌트 목록입니다.
	TArray<TObjectPtr<USceneComponent>> OwnerVisualStabilizedComponents;

	// [v2.48.0] Owner 표시 안정화 때문에 물리 루트 VehicleMesh 렌더링을 숨겼는지 여부입니다.
	bool bOwnerVisualPhysicsMeshHidden = false;

	// [v2.53.0] Owner 차체 표시 안정화가 현재 적용 가능한 상태인지 여부입니다.
	bool bOwnerBodyVisualStabilizationReady = false;

	// [v2.53.0] Owner 차체 표시 안정화용 이전 프레임 표시 회전값입니다.
	FRotator SmoothedOwnerBodyVisualRotation = FRotator::ZeroRotator;

	// [v2.53.0] Owner 차체 표시 안정화 회전 기준값이 유효한지 여부입니다.
	bool bHasSmoothedOwnerBodyVisualRotation = false;

	// [v2.53.0] Owner 차체 표시 안정화 전 SM_Body의 기본 상대 회전입니다.
	FRotator OriginalOwnerBodyVisualRelativeRotation = FRotator::ZeroRotator;

	// [v2.53.0] Owner 차체 표시 안정화 전 SM_Body 상대 회전을 저장했는지 여부입니다.
	bool bHasOriginalOwnerBodyVisualRelativeRotation = false;

};
