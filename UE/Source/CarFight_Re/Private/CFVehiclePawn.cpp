// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 2.156.0
// Date: 2026-08-22
// Description: CarFight 싱글플레이 차량 Pawn 구현 / CF-FQ-032 post-closure Target Identity 안정화
// Changelog:
// - v2.156.0: VehicleData의 유효한 PrimaryAssetId.PrimaryAssetName을 차량 안정 TargetId로 사용. Actor instance GetFName/GetName은 Identity source에서 제외하고 Player-facing DisplayName은 명시 source가 없으면 Empty 유지. VehicleData/PrimaryAssetId가 없으면 TargetId=None으로 fail-closed.
// - v2.155.0: 차량 기본 TargetDisplayInfo에서 런타임 UObject Actor 이름을 안정 TargetId/Player-facing DisplayName으로 공개하던 fallback을 제거. 명시 Identity source가 생기기 전에는 ID/이름을 비워 Sensor/HUD 내부 이름 누출을 차단.
// - v2.154.0: IA_RadarZoom을 기본 로드하고 Started Axis1D의 양수/음수를 현재 LocalPlayer UISubsystem의 Radar Zoom In/Out으로 전달. Pawn은 Sensor Range/Profile 계산을 소유하지 않음.
// - v2.153.0: explicit WeaponCharge Runtime의 충전 부족을 발사 검증에서 별도 차단하고 실제 승인된 각 발사 결과마다 Charge를 정확히 한 번 소비. 기존 Heat/Ammo/Launcher/Cooldown 순서를 보존.
// - v2.152.0: IA_SelectWeapon Axis1D를 기본 로드하고 Started 입력을 1-based ordinal→0-based RequestSelectWeaponIndex로 전달. 숫자키 직접 선택 외 cycle state·WeaponGroup ID·내부 MountProfileId 입력 의미는 추가하지 않음.
// - v2.151.0: Applied Fitting 고정 순번의 Weapon Selection 요청을 구현. 유효 순번 검증 후 Launcher active는 WeaponChanged로 정상 취소하고 WeaponComp 선택·단일 활성 Turret Visual을 같은 순번으로 전환.
// - v2.150.0: explicit Heat Runtime 과열을 발사 검증에서 차단하고 실제 승인된 각 발사 결과에 Heat를 정확히 한 번 누적. 기존 Ammo/Launcher/Cooldown 순서와 계약은 유지.
// - v2.149.0: BeginPlay/SetupPlayerInputComponent의 TargetSelect 직접 생성을 제거하고 Legacy Create/Refresh API를 현재 UISubsystem 소유 Game Layer Marker 조회·갱신 wrapper로 전환.
// - v2.148.0: BeginPlay/SetupPlayerInputComponent의 AimReticle 직접 생성을 제거하고 Legacy Create/Refresh API를 현재 UISubsystem 소유 Reticle 조회·갱신 wrapper로 전환.
// - v2.147.0: `/Game/CarFight/Input/IA_ActiveScan`을 P0 기본 Start Action으로 로드해 V 단발 입력을 기존 StartActiveScan command에 연결. 별도 Stop Action은 기본 로드하지 않음.
// - v2.146.0: optional Active Scan Start/Stop InputAction을 Pawn 입력 계층에서 Sensor Gameplay command로 연결하고 RequestStartActiveScan/RequestStopActiveScan wrapper를 추가. 실제 InputAction 자산·키·트리거는 자동 생성/로드하지 않음.
// - v2.144.0: UCFVehicleSensorComp 기본 서브오브젝트를 생성하고 InitializeVehicleRuntime에서 독립 Sensor Foundation을 초기화. Sensor 결과는 기존 CoreReady/CombatReady 게이트에 포함하지 않음.
// - v2.143.0: CF-FQ-031 AMMO-P0-07 Applied Fitting Snapshot의 InitialSortieAmmoLoads와 finite ResolvedMounts로 VehicleAmmoComp를 초기화하고 Ammo Ready를 CombatReady에 포함.
// - v2.142.0: CF-FQ-031 AMMO-P0-05 현재 활성 WeaponInstance를 VehicleAmmoComp 수동 Reload API로 전달하는 Gameplay 명령을 추가.
// - v2.141.0: CF-FQ-031 AMMO-P0-04 유한탄 Ripple·Salvo 전체 유효 발수를 첫 발 실행 전에 예약하고 첫 발 Commit·실패/시작 실패 Release 및 Launcher Action Lock 거부를 연결.
// - v2.140.0: CF-FQ-031 AMMO-P0-03 SingleCycle을 Validate→Ammo Reserve→실행→Commit/Rollback으로 연결하고 HitScan 실제 실행을 검증 단계에서 Execute 단계로 이동.
// - v2.139.0: CF-FQ-031 AMMO-P0-02 VehicleAmmoComp 기본 서브오브젝트를 만들고 재초기화와 EndPlay에서 출격 탄약 상태를 Reset. 기존 Fire에는 미연결.
// - v2.138.0: VehicleDebug Snapshot이 현재 선택 대상 Actor의 TargetSelect 표시 정보와 방어·내구도 런타임 상태를 읽기 전용으로 수집하도록 연결.
// - v2.137.0: WITH_EDITOR PIE에서 PreRegister·BeginPlay·다음 Tick·1초 후·EndPlay의 피팅·방어 컴포넌트 인스턴스와 런타임 상태를 기록하는 CF-FQ-033 수명 Probe를 추가.
// - v2.136.0: 첫 발사 순간 위치·Actor를 함께 캡처하고 Ripple·Salvo 후속 Launch Context가 현재 TargetSelect를 재조회하지 않도록 수정.
// - v2.135.0: Projectile Launch Context에 발사 순간 선택 Target Actor를 Snapshot으로 복사해 Direct 미사일의 독립 목표 수명을 연결.
// - v2.134.0: 차량 코어 Runtime과 전투 Runtime 준비 상태를 분리하고 기존 bVehicleRuntimeReady를 코어 호환 상태로 유지.
// - v2.133.0: FIT-P0-05 PreRegister Initial Mass 적용, BeginPlay 실제 VehicleMesh 질량 검증과 Cached Snapshot Weapon·Defense Commit을 구현.
// - v2.132.0: FIT-P0-04 VehicleFittingComp 기본 서브오브젝트와 Legacy·Snapshot Weapon·Defense 출격 초기화 순서를 구현.
// - v2.131.0: UI-P0-02 Pause 진입 전 Drive·Look·입력 소유권 잔류를 중립화하고 Launcher 시퀀스는 보존하는 ClearGameplayInputForPause를 구현.
// - v2.130.0: VehicleDebug Snapshot이 VehicleDefenseComp 준비·Fallback·현재 상태와 마지막 전체 피해 결과 요약을 읽도록 연결.
// - v2.129.0: VehicleDefenseComp 기본 서브오브젝트·런타임 초기화와 HitScan 정식 방어 피해 진입점을 구현하고 기존 Health Debug 결과를 유지.
// - v2.128.0: Hitscan 동일 차량 Projectile 제외, 다른 차량 Projectile Intercepted 처리와 요격 FX 정책 연결을 구현.
// - v2.127.0: LM-P0-04 Release 설정 기반 Direct·Angled·Vertical Context, 차량 속도 상속과 비Direct 사출 안전 검사를 구현.
// - v2.126.0: LauncherComp 기본 서브오브젝트, 고정 Command Target 후속 발사, 내부 쿨다운 우회와 Ripple·Salvo 실행 연결을 추가.
// - v2.125.0: 가변 Muzzle 소켓 해결, FireRequest·LaunchContext 스냅샷과 승인 발사 기반 SingleCycle 진행을 구현.
// - v2.124.0: Fire Command 기반 Direct Launch Context 생성과 ProjectilePool Context Acquire 경로를 연결.
// - v2.123.0: 이름이 SM_Body인 표준 차체 StaticMeshComponent를 Combat FX에 제공하는 getter를 구현.
// - v2.122.0: 차량 TargetPoint를 SM_Body Bounds 중심에 자동 정렬해 인스턴스별 후보 대표 위치 불일치를 보정.
// - v2.121.0: TS-P0-06 WBP_TargetSelect 기본 로드와 로컬 Viewport 생성·갱신·정리 수명을 연결.
// - v2.120.0: TS-P0-05 선택·해제 Input Action 로드, Enhanced Input 바인딩과 후보 확정·Manual 해제 명령을 구현.
// - v2.119.0: TargetPoint 기본 서브오브젝트를 VehicleMesh에 연결하고 차량 선택 위치를 공용 TargetPoint Fallback으로 전환.
// - v2.118.0: TargetSelectComp 기본 서브오브젝트를 생성하고 차량의 ICFTargetSelectable 기본 표시·위치·추적 계약을 구현.
// - v2.117.0: MuzzleBlocked를 전체 조준 경로가 아닌 TurretMountData의 총구 안전 거리 안에서만 판정하도록 수정.
// - v2.116.0: 총구 Trace가 VehicleHealthComp를 가진 유효 피해 대상을 먼저 맞으면 MuzzleBlocked로 오판하지 않도록 수정.
// - v2.115.0: CurrentMuzzleDirection을 조준 목표 거리까지 연장한 탄종 독립 터렛 레티클 월드 지점을 Aim Solution에 추가.
// - v2.114.0: LaunchDirection Preview를 Command 목표 깊이로 투영하고 Camera/Muzzle Trace가 같은 Actor를 적중하면 목표 표면으로 허용.
// - v2.113.0: 중력 Projectile을 LaunchDirection Weapon Reticle 모드로 분리해 초기 발사 방향 표식을 제공.
// - v2.112.0: 실제 최종 발사 방향의 WeaponHit Trace를 MuzzleBlocked와 직선 Weapon Preview World 데이터가 공유하도록 확장.
// - v2.111.0: VehicleHealthComp 생성, VehicleData 최대 체력 초기화, HitScan/Projectile 공용 BaseDamage 적용과 Debug 결과 연결.
// - v2.110.0: 터렛별 정렬 중 발사 정책에 따라 실제 Muzzle/요구 방향을 선택하고 검증과 obstruction 경로를 통일.
// - v2.109.0: Muzzle 기준 Weapon Aim Solution을 추가해 FireCommand, HitScan, Projectile, 터렛 추적 방향을 Reticle 목표점 기준으로 통합.
// - v2.108.0: SM_Body를 WeaponHit / Projectile 전용 QueryOnly 피격 표면으로 구성하고 Damage HitContext에 피격 컴포넌트 이름을 기록.
// - v2.107.0: OutOfWeaponArc 경고 표시를 발사 실패 피드백 유지 시간 안에서만 활성화.
// - v2.106.0: 발사 성공 피드백을 쿨다운보다 우선 표시하고 성공 표시 종료 후 쿨다운 상태로 전환.
// - v2.105.1: FireFeedback FName 삼항 연산의 문자열/EName 형식 불일치를 명시적 FName 생성으로 수정.
// - v2.105.0: 연속 발사 후 WeaponCooldown / OutOfWeaponArc FireFeedback이 종료되지 않던 문제를 표시 유지 조건 기준으로 수정.
// - v2.104.0: Pawn에서 Reticle / FireFeedback UI용 ViewData를 생성하고 발사 결과 적용 시 피드백 시작 시간을 기록.
// - v2.103.0: 자동 스케일된 휠 메시의 바운드 중심을 Wheel_Mesh 원점에 맞춰 시각 휠과 물리 휠 중심 불일치를 보정.
// - v2.102.0: VehicleData WheelVisualConfig 옵션이 켜진 경우 WheelRadius 기준으로 Wheel_Mesh_* 표시 스케일을 자동 보정.
// - v2.100.0: 터렛 시각 장착에서 MountProfile legacy 직접 TurretMountData fallback을 제거하고 EquipmentPresetData 전용 경로로 전환.
// - v2.99.0: VehicleDebug Snapshot에 활성 EquipmentPresetData 상태를 채우고 터렛 시각 장착을 EquipmentPresetData 우선 해석으로 전환.
// - v2.98.0: MountProfile inline 터렛 시각 fallback을 런타임에서 제거하고 TurretMountData 전용 경로로 전환.
// - v2.97.0: Dummy HitScan과 Projectile Actor 충돌 결과를 같은 Damage HitContext Debug로 기록.
// - v2.96.0: VehicleDebug DamageData 표시 설명을 ProjectileData 단일 소유 정책에 맞게 정리.
// - v2.95.0: VehicleDebug Snapshot의 Weapon 카테고리에 활성 DamageData 참조, ID, 요약, 해석 경로를 채움.
// - v2.94.0: 터렛 안정화 전 발사 정책에 맞춰 조준각 초과를 기본 발사 거부 조건에서 제외.
// - v2.93.0: Turret Pitch 메쉬의 Muzzle 소켓이 유효하면 최종 FireOrigin을 총구 기준으로 보정.
// - v2.92.0: 터렛 하드포인트 / Yaw / Pitch 필수 소켓 누락을 VehicleDebug 요약에서 MissingRequiredSocket으로 구분.
// - v2.91.0: 하드포인트 / Yaw / Pitch 소켓 부착을 SnapToTarget 기준으로 명확히 하고 소켓-루트 위치 차이 Debug를 추가.
// - v2.90.0: 터렛 장착 루트가 HardpointSlot.SocketName을 실제 차체 소켓으로 우선 사용하도록 수정.
// - v2.89.0: WeaponComp가 계산한 터렛 Yaw / Pitch 추적 각도를 시각 피벗 컴포넌트에 적용하고 Debug Snapshot에 표시.
// - v2.88.0: 터렛 시각 장착을 BaseMesh + YawPivot + PitchPivot 3단 소켓 계층으로 확장.
// - v2.87.0: 터렛 시각 장착이 DefaultTurretMountData를 우선 사용하고 MountProfile inline 필드를 fallback으로 사용하도록 전환.
// - v2.86.1: 터렛 Pitch 메쉬 기본 부모 선택식의 TObjectPtr 모호성을 제거해 빌드를 안정화.
// - v2.86.0: MountProfile 터렛 시각 메쉬를 하드포인트 위치에 붙이고 VehicleDebug Snapshot에 터렛 시각 요약을 채움.
// - v2.85.0: VehicleDebug Snapshot의 Weapon 카테고리에 Projectile Pool 마지막 반환 요약을 채움.
// - v2.84.0: VehicleDebug Snapshot의 Weapon 카테고리에 활성 무기 분당 발사속도와 환산 발사 간격을 채움.
// - v2.83.0: VehicleDebug Snapshot의 Weapon 카테고리에 Projectile Pool 보유 여부와 전체 / 활성 / 비활성 수를 채움.
// - v2.82.0: ProjectilePoolComp를 생성하고 Projectile Actor 스폰 경로를 Pool Acquire 기반으로 전환.
// - v2.81.0: Projectile FireMode에서 공통 Projectile Actor 스폰 경로를 추가하고 Dummy HitScan fallback을 유지.
// - v2.80.0: VehicleDebug Snapshot의 Weapon 카테고리에 Projectile Actor 스폰 준비 상태와 실행 요약을 채움.
// - v2.79.0: VehicleDebug Snapshot의 Weapon 카테고리에 활성 ProjectileData ID/요약을 채움.
// - v2.78.0: 활성 WeaponData의 MaxRange를 Dummy HitScan Trace 거리에 적용하고 CooldownSeconds를 로컬 Fire 검증에 연결.
// - v2.77.0: VehicleDebug Snapshot의 Weapon 카테고리에 활성 WeaponData ID/호환성/요약을 채움.
// - v2.76.0: VehicleDebug Snapshot에 Weapon 카테고리를 채워 패널에서 WeaponComp와 FireOrigin 상태를 확인할 수 있게 함.
// - v2.75.0: VehicleWeaponComp를 생성하고 VehicleData MountProfiles 기반 FireOrigin을 로컬 Fire Command에 반영.
// - v2.74.0: SM_Body 소켓에서 휠 앵커와 선택 하드포인트 위치를 함께 캡처하는 차량 레이아웃 흐름으로 확장.
// - v2.73.0: VehicleMovementConfig.ThrottleInputScale을 실제 스로틀 입력에 적용해 가속감 Quick Tune 체감 차이를 보장.
// - v2.72.1: 전진 구동을 끊을 수 있는 휠 재생성, 구동 휠, 서스펜션 런타임 변경을 안전하게 제외.
// - v2.72.0: VehicleMovementConfig 값을 이미 생성된 Chaos Vehicle 런타임 시뮬레이션에도 즉시 적용하도록 보강.
// - v2.71.0: 에디터 Details 버튼 실행 결과가 PIE 외부에서도 보이도록 Slate 알림을 추가.
// - v2.70.0: 소켓 캡처/레이아웃 적용 에디터 버튼 실행 후 Wheel_Anchor 갱신과 WheelSync 재준비 피드백을 보강.
// - v2.69.0: 차체 메시 소켓에서 VehicleData.VehicleLayoutConfig를 캡처하는 에디터 전용 흐름 추가.
// - v2.68.0: VehicleData의 VehicleLayoutConfig를 Wheel_Anchor_*에 적용하고 WheelSync 준비 전 레이아웃 재적용 순서를 추가.
// - v2.67.0: 로컬 HitScan Trace 디버그 변수명을 LocalAimTraceDebug 기준으로 교체.
// - v2.66.0: AimComp의 FireValidationState / AimVisualState 리네이밍에 맞춰 Debug Snapshot과 Fire Result 연결을 갱신.
// - v2.65.0: 참조가 없는 ACFVehiclePawn Fire 레거시 wrapper와 RPC 구현을 제거해 싱글플레이 Fire 실행 경로를 단일화.
// - v2.63.0: Aim Fire 입력을 BuildFireCommand / ValidateFireCommand / RunLocalDummyHitScan / ApplyFireResult 로컬 경로로 전환하고 기존 RPC 이름 함수는 wrapper로 유지.
// - v2.62.0: 싱글플레이 기준선에서 차량 네트워크 진단 샘플/RepMove 수신 로그/복제 등록 경로를 제거.
// - v2.61.0: 싱글플레이 전환에 맞춰 C++ 기준선에서 Actor 복제와 Replicate Movement 강제 활성화를 중단.
// - v2.60.0: 싱글플레이 전환에 맞춰 상단 기준 설명에서 CFNetSmooth 적용 전 문구를 제거.
// - v2.59.0: CFNetSmooth Visual/Shell 적용 전 기준선을 깨끗하게 만들기 위해 차량 진단 로그와 Owner 표시 안정화 기본값을 False로 통일.
// Migration:
// - v2.156.0부터 차량 TargetDisplayInfo.TargetId는 VehicleData의 유효한 PrimaryAssetId.PrimaryAssetName을 사용한다. VehicleData가 없거나 PrimaryAssetId가 invalid면 None으로 유지하고 Actor GetFName/GetName fallback은 사용하지 않는다. DisplayName은 별도 Player-facing 이름 source가 생기기 전 Empty를 유지한다.
// - v2.155.0의 TargetId=None-only 임시 교정은 Sensor Identified 공개 계약과 충돌할 수 있어 v2.156.0의 VehicleData PrimaryAssetId 기반 안정 ID로 대체한다.
// - v2.154.0부터 `/Game/CarFight/Input/IA_RadarZoom` Axis1D를 기본 로드한다. MouseScrollUp=양수는 Zoom In, MouseScrollDown=음수는 Zoom Out으로 UISubsystem에만 전달하며 Sensor detection range와 Scanner Profile은 변경하지 않는다.
// - v2.152.0부터 `/Game/CarFight/Input/IA_SelectWeapon` Axis1D를 Pawn Gameplay Input으로 사용한다. 숫자키 1~9가 실제 SelectableWeapons의 1-based ordinal을 전달하며 Mouse Wheel은 Radar Range/Zoom 예약을 보존하고 게임패드 선택키는 이번 P0에서 지정하지 않는다.
// - v2.152.0 handler는 정수 1~9 ordinal만 수락해 `RequestSelectWeaponIndex(Ordinal - 1)`에 위임한다. 실제 목록 범위·Launcher cancel·WeaponComp/Turret 전환은 기존 검증된 Gameplay command가 계속 소유한다.
// - v2.148.0부터 AimReticle은 Pawn에서 CreateWidget/AddToViewport하지 않는다. UISubsystem이 HUD Layer 단일 인스턴스를 소유하고 Possess 변경마다 현재 Pawn만 SetVehiclePawnRef로 연결한다.
// - v2.147.0부터 InputAction_StartActiveScan은 IA_ActiveScan을 기본 로드한다. IA_ActiveScan은 Boolean + Pressed이고 IMC_Vehicle_Default의 V 키 한 번으로 ActiveScanDurationSec 실행을 시작한다.
// - InputAction_StopActiveScan은 기본 null이며 P0 키 매핑을 만들지 않는다. RequestStopActiveScan은 시스템/장비 전환용 명시적 중단 command로만 유지한다.
// - v2.146.0의 Pawn wrapper는 VehicleSensorComp Start/Stop 명령에만 위임하며 SensorData Apply, InitializeSensorRuntime, ResetSensorRuntime과 FittingSnapshot 연결을 호출하지 않는다.
// - bVehicleRuntimeReady는 기존 Tick·Debug 호환을 위해 bVehicleCoreRuntimeReady와 같은 값을 유지한다. 전투 HUD와 전투 명령은 bVehicleCombatRuntimeReady를 별도로 사용한다.
// - 유효 피팅 Snapshot 질량은 게임 World의 PreRegisterAllComponents에서 Super 호출 전에 Movement Mass에 1회 기록한다.
// - BeginPlay는 같은 Cached Snapshot의 Configured Mass와 VehicleMesh 실제 질량을 검증한 뒤에만 Weapon·Defense를 Commit한다.
// - FittingData 미지정·초기 Invalid Snapshot은 기존 Chaos 질량과 VehicleData Legacy 입력을 유지한다.
// - Physics State 생성 뒤 다른 Target Mass는 Runtime Ready 실패로 거부하며 SetMassOverrideInKg와 Hot Recreate를 호출하지 않는다.
// - VehicleDefense Debug 연결은 컴포넌트 캐시를 읽기만 하며 CF-FQ-029 Launcher·Pool·추진·요격과 실제 피해 판정을 변경하지 않는다.
// - 기존 WeaponData는 Direct / CarrierVelocityRatio 0 기본값으로 AimDirection·ProjectileData.InitialSpeed 결과를 유지한다.
// - AngledEjection은 실제 Muzzle 로컬 방향을, VerticalEjection은 실제 Muzzle X축을 사용하며 CommandTargetLocation은 별도로 보존한다.
// - 기존 BP_CFVehiclePawn은 LauncherComp 기본 서브오브젝트를 자동 상속하며 WeaponData 기본값 SingleCycle / 1발에서는 기존 발사 흐름과 동일하게 동작한다.
// - Ripple·Salvo는 첫 입력 순간 Command Target을 고정하고 각 후속 발사 시점의 다음 유효 Muzzle에서 기존 검증·Projectile·FX 경로를 재사용한다.
// - 파괴 FX는 GetVehicleBodyMeshComponent로 SM_Body를 찾고 차량별 FX_Destroyed 소켓을 우선 사용한다.
// - 차량 TargetPoint는 SM_Body Bounds 중심에 자동 정렬하며 PreferredBoundsLocalOffset으로 차량별 미세 조정한다.
// - 선택 입력은 현재 후보가 유효할 때만 선택을 변경하고 후보가 없으면 기존 선택을 유지한다.
// - 해제 입력은 Manual 사유로 선택만 해제하며 후보 유지와 자동 다음 타겟 금지 정책을 보존한다.
// - MuzzleBlocked는 MuzzleClearanceDistanceCm 안의 비피해 장애물만 차단하며, 더 먼 충돌은 실제 HitScan/Projectile 적중 처리에 맡긴다.
// - 총구와 Command 목표 사이의 피해 가능한 차량은 정상 적중 대상으로 발사를 허용하며, 비피해 장애물은 기존처럼 MuzzleBlocked로 거부한다.
// - bAllowFireWhileAligning=true인 터렛은 정렬 중 현재 Muzzle 방향으로 발사하며, false인 터렛은 기존처럼 정렬 완료 전 거부한다.
// - 중력 Projectile의 LaunchDirection Preview는 예상 탄착점이 아니라 AimDirection 기준 초기 발사 방향 표식이다.
// - MuzzleBlocked는 정책과 관계없이 실제 최종 발사 경로를 기준으로 항상 발사를 거부한다.
// - Muzzle 소켓 또는 Reticle 목표점이 유효하지 않으면 fallback 방향으로 발사하지 않고 발사 검증에서 안전하게 거부한다.
// - WeaponNotAligned는 LastFireResult에는 기록하지만 FireFeedback에서 일반 FireRejected 빨간 오버라이드를 만들지 않는다.
// - 자동 휠 메시 스케일을 켠 차량은 기본적으로 메시 바운드 중심 보정도 함께 적용된다. 기존 수동 Wheel_Mesh 상대 위치를 유지해야 하면 bAutoCenterWheelMeshBoundsToOrigin=false로 끈다.
// - 기존 차량은 WheelVisualConfig.bAutoScaleWheelMeshToRadius=false 기본값으로 기존 Wheel_Mesh_* 수동 스케일을 유지한다.
// - 자동 휠 메시 스케일은 StaticMesh 로컬 바운드 반지름을 기준으로 하므로 축이 다른 메시에서는 WheelMeshRadiusMeasureMode를 조정한다.
// - MountProfile.DefaultEquipmentPresetData가 있으면 터렛 시각 장착과 Weapon Debug는 EquipmentPresetData를 단일 소스로 사용한다.
// - EquipmentPresetData가 없거나 내부 TurretMountData / WeaponData 참조가 비어 있으면 해당 Debug는 Missing 상태로 표시하고 MountProfile 직접 fallback은 사용하지 않는다.
// - OutOfWeaponArc는 호환용 enum 값으로 남지만, P0 터렛 발사 정책에서는 조준각 초과만으로 발사를 막지 않는다.
// - 소켓 이름이 지정되어 있는데 실제 메쉬에 없으면 기존 fallback은 유지하지만, VehicleDebug Panel 터렛 시각 요약에 MissingRequiredSocket 상태가 표시된다.
// - TurretBaseMesh가 비어 있으면 기존처럼 하드포인트 루트에 YawPivot을 두고 Yaw / Pitch 메쉬를 장착한다.
// - YawPivotSocketName 또는 PitchPivotSocketName이 없으면 해당 Pivot은 부모 원점 기준으로 fallback된다.
// - EquipmentPresetData 내부 TurretMountData가 있으면 TurretMountData의 메쉬 / 소켓 / Transform을 사용한다.
// - EquipmentPresetData 내부 TurretMountData가 비어 있으면 MountProfile inline 터렛 시각 fallback은 더 이상 사용하지 않고 시각 장착을 생략한다.
// - 터렛 시각 메쉬가 비어 있으면 표시만 생략하고 기존 FireOrigin / Projectile / Cooldown 검증은 유지한다.
// - 터렛 회전 상태는 WeaponComp가 소유하고, Pawn은 계산된 Yaw / Pitch 값을 TurretYawPivot / TurretPitchPivot 시각 컴포넌트에 적용만 한다.
// - HardpointSlot.SocketName이 차체 소켓에 있으면 터렛 장착 루트는 소켓에 직접 붙고, 없으면 기존 LocalTransform fallback을 사용한다.
// - VehicleDebug Panel의 터렛 시각 요약에서 하드포인트 소켓 위치와 터렛 루트 위치 차이를 확인할 수 있다.
// - MuzzleSocketName이 Pitch 메쉬에 존재하면 최종 FireOrigin 위치와 방향은 해당 소켓을 우선 사용한다.
// - Muzzle 소켓이 없거나 Pitch 메쉬가 없으면 기존 하드포인트 FireOrigin fallback을 유지한다.
// - DamageData가 비어 있거나 ProjectileData.DamageProfileId fallback만 있으면 기존 Fire / Projectile / Dummy HitScan 흐름은 유지하지만 VehicleHealthComp 피해 적용은 MissingDamageData로 거부한다.
// - Damage HitContext 기록은 Debug와 최소 BaseDamage 체력 적용 입력으로 사용하며 장갑 / 모듈 손상은 수행하지 않는다.
// - Projectile Actor 확보는 WeaponData.FireMode가 Projectile이고 ProjectileData / ProjectileActorClass가 모두 유효할 때만 실행한다.
// - Projectile Pool 확보 조건이 맞지 않거나 Pool 확보 실패 시 기존 Dummy HitScan fallback을 유지한다.
// - Fire Command의 AimOrigin·AimDirection·PredictedAimTargetLocation과 요청 ID를 Direct Launch Context로 복사한다.
// - LM-P0-01 Direct Context는 InitialSpeed 기반 Velocity와 Zero Carrier Velocity를 사용해 기존 발사 결과를 유지한다.
// - WeaponData가 없으면 기존 Aim Profile MaxAimDistance와 즉시 발사 흐름을 유지한다.
// - 기존 FireCommand / FireOrigin 흐름은 유지하고 WeaponData는 우선 디버그와 데이터 연결 확인에만 사용한다.
// - 기존 VehicleDebug 카테고리와 Fire 함수 시그니처는 유지하고 Weapon 카테고리만 추가한다.
// - 기존 Aim 기반 발사 검증과 더미 HitScan은 유지하며, WeaponComp가 FireOrigin 계산에 성공한 경우에만 발사 원점과 무기 그룹을 덮어쓴다.
// - MountProfiles가 비어 있거나 Top_01 하드포인트가 없으면 WeaponComp는 Ready가 아니어도 전체 VehicleRuntime Ready를 막지 않는다.
// - 메시 소켓 캡처 버튼은 VehicleData.HardpointSlots에 선언된 SocketName만 선택적으로 갱신한다.
// - 하드포인트 SocketName이 비어 있거나 누락되어도 휠 레이아웃 캡처 성공 자체는 유지된다.
// - 가속감 Quick Tune은 EngineMaxTorque와 함께 ThrottleInputScale을 저장하므로 0%와 100% 테스트는 스로틀 입력 크기부터 달라진다.
// - DA 주행감 프리셋을 적용한 뒤 PIE를 다시 시작하거나 Initialize Vehicle Runtime을 호출하면 엔진 토크와 안전한 휠 런타임 setter까지 갱신된다.
// - Mesh_TestSUV처럼 Wheel_Anchor_FL/FR/RL/RR 소켓을 가진 차체 메시에서는 에디터 캡처 버튼으로 DA 레이아웃 값을 생성한다.
// - 소켓 캡처 버튼은 선택된 Pawn 인스턴스의 VehicleData를 갱신하므로, 테스트할 때 DA_TestSUV가 할당된 액터를 선택한다.
// - VehicleLayoutConfig 적용 차량은 DA에서 bUseLayoutOverrides를 켜고 네 WheelAnchor 값을 입력한다.
// - 기존 BP 수동 Wheel_Anchor 배치는 bUseLayoutOverrides=false fallback으로 유지한다.
// - 신규 Fire 흐름은 로컬 함수 경로를 기준으로 호출한다.
// - bDrawServerAimTraceDebug / ServerAimTraceDebugDuration 호출은 bDrawLocalAimTraceDebug / LocalAimTraceDebugDuration으로 교체한다.
// - Debug Snapshot의 ServerAimState / RepAimVisualState 접근은 FireValidationState / AimVisualState로 교체한다.
// - ACFVehiclePawn의 BuildFireRequest / ValidateFireRequestOnServer / RunServerDummyHitScan / ServerRequestFire / ClientReceiveFireResult 호출은 제거하고 로컬 Fire 함수로 교체한다.
// - BP_CFVehiclePawn의 Actor Replicates/Replicate Movement도 False로 저장해 C++ 기본값과 맞춘다.
// - 멀티플레이 진단이 다시 필요하면 별도 멀티플레이 브랜치/문서에서 복구한다.

#include "CFVehiclePawn.h"

#include "CFCollisionChannels.h"
#include "CFEquipmentPresetData.h"
#include "CFLauncherComp.h"
#include "CFProjectileData.h"
#include "CFProjectileLaunchTypes.h"
#include "CFProjectileActor.h"
#include "CFProjectilePoolComp.h"
#include "CFDamageData.h"
#include "CFTurretMountData.h"
#include "CFVehicleData.h"
#include "CFVehicleAimComp.h"
#include "CFVehicleCameraComp.h"
#include "CFVehicleDriveComp.h"
#include "CFCombatFxComp.h"
#include "CFVehicleHealthComp.h"
#include "CFVehicleDefenseComp.h"
#include "CFVehicleDefenseData.h"
#include "CFVehicleFittingComp.h"
#include "CFVehicleFittingData.h"
#include "CFVehicleAmmoComp.h"
#include "CFTargetSelectComp.h"
#include "CFVehicleSensorComp.h"
#include "CFWeaponData.h"
#include "CFVehicleWeaponComp.h"
#include "CFWheelSyncComp.h"
#include "CarFightVehicleUtils.h"
#include "UI/CFAimReticleWidget.h"
#include "UI/CFTargetSelectWidget.h"
#include "UI/CFUISubsystem.h"

#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "EnhancedActionKeyMapping.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "PhysicsEngine/BodySetup.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "TimerManager.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace
{
	// ??????????れ삀???筌ｋ〃泥???도 ??ш끽維뽳쭛?????곷츉??繹먮끏?????モ봼????ш끽維???怨뚮옖甕??????怨좊룴??猷?獄??怨뚮옖????筌뤾퍓???
	struct FCFWheelClassRuntimeSnapshot
	{
		float MaxSteerAngle = 0.0f;
		float MaxBrakeTorque = 0.0f;
		float MaxHandBrakeTorque = 0.0f;
		float WheelRadius = 0.0f;
		float WheelWidth = 0.0f;
		float FrictionForceMultiplier = 0.0f;
		float CorneringStiffness = 0.0f;
		float WheelLoadRatio = 0.0f;
		float SpringRate = 0.0f;
		float SpringPreload = 0.0f;
		float SuspensionMaxRaise = 0.0f;
		float SuspensionMaxDrop = 0.0f;
		bool bAffectedByEngine = false;
		ESweepShape SweepShape = ESweepShape::Raycast;
	};

	// 癲ル슣??????????????れ삀???筌ｋ〃泥???틖 ??ш끽維????筌먲퐢六????ㅺ컼??????怨좊룴??猷?筌뤿뱶?????濚왿몾??????덊렡.
	FCFWheelClassRuntimeSnapshot CaptureWheelClassRuntimeSnapshot(const UChaosVehicleWheel& WheelClassDefaultObject)
	{
		FCFWheelClassRuntimeSnapshot Snapshot;
		Snapshot.MaxSteerAngle = WheelClassDefaultObject.MaxSteerAngle;
		Snapshot.MaxBrakeTorque = WheelClassDefaultObject.MaxBrakeTorque;
		Snapshot.MaxHandBrakeTorque = WheelClassDefaultObject.MaxHandBrakeTorque;
		Snapshot.WheelRadius = WheelClassDefaultObject.WheelRadius;
		Snapshot.WheelWidth = WheelClassDefaultObject.WheelWidth;
		Snapshot.FrictionForceMultiplier = WheelClassDefaultObject.FrictionForceMultiplier;
		Snapshot.CorneringStiffness = WheelClassDefaultObject.CorneringStiffness;
		Snapshot.WheelLoadRatio = WheelClassDefaultObject.WheelLoadRatio;
		Snapshot.SpringRate = WheelClassDefaultObject.SpringRate;
		Snapshot.SpringPreload = WheelClassDefaultObject.SpringPreload;
		Snapshot.SuspensionMaxRaise = WheelClassDefaultObject.SuspensionMaxRaise;
		Snapshot.SuspensionMaxDrop = WheelClassDefaultObject.SuspensionMaxDrop;
		Snapshot.bAffectedByEngine = WheelClassDefaultObject.bAffectedByEngine;
		Snapshot.SweepShape = WheelClassDefaultObject.SweepShape;
		return Snapshot;
	}

	// ???怨좊룴??猷멸강?????潁뺛꺈彛???????????れ삀???筌ｋ〃泥???도 ????????ㅺ컼?얜쑚????嚥▲꺃??繹먮끏????
	void RestoreWheelClassRuntimeSnapshot(UChaosVehicleWheel& WheelClassDefaultObject, const FCFWheelClassRuntimeSnapshot& Snapshot)
	{
		WheelClassDefaultObject.MaxSteerAngle = Snapshot.MaxSteerAngle;
		WheelClassDefaultObject.MaxBrakeTorque = Snapshot.MaxBrakeTorque;
		WheelClassDefaultObject.MaxHandBrakeTorque = Snapshot.MaxHandBrakeTorque;
		WheelClassDefaultObject.WheelRadius = Snapshot.WheelRadius;
		WheelClassDefaultObject.WheelWidth = Snapshot.WheelWidth;
		WheelClassDefaultObject.FrictionForceMultiplier = Snapshot.FrictionForceMultiplier;
		WheelClassDefaultObject.CorneringStiffness = Snapshot.CorneringStiffness;
		WheelClassDefaultObject.WheelLoadRatio = Snapshot.WheelLoadRatio;
		WheelClassDefaultObject.SpringRate = Snapshot.SpringRate;
		WheelClassDefaultObject.SpringPreload = Snapshot.SpringPreload;
		WheelClassDefaultObject.SuspensionMaxRaise = Snapshot.SuspensionMaxRaise;
		WheelClassDefaultObject.SuspensionMaxDrop = Snapshot.SuspensionMaxDrop;
		WheelClassDefaultObject.bAffectedByEngine = Snapshot.bAffectedByEngine;
		WheelClassDefaultObject.SweepShape = Snapshot.SweepShape;
	}

	// 癲ル슓堉곁땟???DA????ш끽維????ш끽維???????쒓랜萸????????????れ삀???筌ｋ〃泥???군 ??ш끽維뽳쭛???낆뒩????筌뤾퍓???
	void ApplyVehicleMovementWheelTuningToWheelClass(
		UChaosVehicleWheel& WheelClassDefaultObject,
		const FCFVehicleMovementConfig& VehicleMovementConfig,
		const bool bIsFrontWheel)
	{
		WheelClassDefaultObject.MaxBrakeTorque =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelMaxBrakeTorque
			: VehicleMovementConfig.RearWheelMaxBrakeTorque;
		WheelClassDefaultObject.WheelRadius =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelRadius
			: VehicleMovementConfig.RearWheelRadius;
		WheelClassDefaultObject.WheelWidth =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelWidth
			: VehicleMovementConfig.RearWheelWidth;
		WheelClassDefaultObject.FrictionForceMultiplier =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelFrictionForceMultiplier
			: VehicleMovementConfig.RearWheelFrictionForceMultiplier;
		WheelClassDefaultObject.CorneringStiffness =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelCorneringStiffness
			: VehicleMovementConfig.RearWheelCorneringStiffness;
		WheelClassDefaultObject.WheelLoadRatio =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelLoadRatio
			: VehicleMovementConfig.RearWheelLoadRatio;
		WheelClassDefaultObject.SpringRate =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelSpringRate
			: VehicleMovementConfig.RearWheelSpringRate;
		WheelClassDefaultObject.SpringPreload =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelSpringPreload
			: VehicleMovementConfig.RearWheelSpringPreload;
		WheelClassDefaultObject.SuspensionMaxRaise =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelSuspensionMaxRaise
			: VehicleMovementConfig.RearWheelSuspensionMaxRaise;
		WheelClassDefaultObject.SuspensionMaxDrop =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelSuspensionMaxDrop
			: VehicleMovementConfig.RearWheelSuspensionMaxDrop;
		WheelClassDefaultObject.bAffectedByEngine =
			bIsFrontWheel
			? VehicleMovementConfig.bFrontWheelAffectedByEngine
			: VehicleMovementConfig.bRearWheelAffectedByEngine;
		WheelClassDefaultObject.SweepShape =
			bIsFrontWheel
			? VehicleMovementConfig.FrontWheelSweepShape
			: VehicleMovementConfig.RearWheelSweepShape;

		if (bIsFrontWheel)
		{
			WheelClassDefaultObject.MaxSteerAngle = VehicleMovementConfig.FrontWheelMaxSteerAngle;
			return;
		}

		WheelClassDefaultObject.MaxHandBrakeTorque = VehicleMovementConfig.RearWheelMaxHandBrakeTorque;
	}

	// [v2.72.0] DA 휠 물리값을 이미 생성된 Chaos Vehicle 런타임 시뮬레이션에 즉시 반영합니다.
	void ApplyVehicleMovementWheelTuningToRuntime(
		UChaosWheeledVehicleMovementComponent& VehicleMovementComponent,
		const FCFVehicleMovementConfig& VehicleMovementConfig,
		const int32 WheelIndex,
		const bool bIsFrontWheel)
	{
		// [v2.72.0] 현재 휠에 적용할 최대 브레이크 토크입니다.
		const float WheelMaxBrakeTorque = bIsFrontWheel
			? VehicleMovementConfig.FrontWheelMaxBrakeTorque
			: VehicleMovementConfig.RearWheelMaxBrakeTorque;

		// [v2.72.0] 현재 휠에 적용할 반지름입니다.
		const float WheelRadius = bIsFrontWheel
			? VehicleMovementConfig.FrontWheelRadius
			: VehicleMovementConfig.RearWheelRadius;

		// [v2.72.0] 현재 휠에 적용할 마찰력 배수입니다.
		const float WheelFrictionMultiplier = bIsFrontWheel
			? VehicleMovementConfig.FrontWheelFrictionForceMultiplier
			: VehicleMovementConfig.RearWheelFrictionForceMultiplier;

		VehicleMovementComponent.SetWheelMaxBrakeTorque(WheelIndex, WheelMaxBrakeTorque);
		VehicleMovementComponent.SetWheelRadius(WheelIndex, WheelRadius);
		VehicleMovementComponent.SetWheelFrictionMultiplier(WheelIndex, WheelFrictionMultiplier);

		if (bIsFrontWheel)
		{
			VehicleMovementComponent.SetWheelMaxSteerAngle(WheelIndex, VehicleMovementConfig.FrontWheelMaxSteerAngle);
			return;
		}

		VehicleMovementComponent.SetWheelHandbrakeTorque(WheelIndex, VehicleMovementConfig.RearWheelMaxHandBrakeTorque);
	}

	// ???????⑥??StaticMeshComponent??癲ル슓??젆??눀???癰???????猿?????????⑤베肄????筌뤿걩???筌뤾퍓???
	UStaticMeshComponent* FindStaticMeshComponentByName(const AActor* OwnerActor, const FName ComponentName)
	{
		if (!OwnerActor || ComponentName.IsNone())
		{
			return nullptr;
		}

		TArray<UStaticMeshComponent*> StaticMeshComponents;
		OwnerActor->GetComponents<UStaticMeshComponent>(StaticMeshComponents);
		for (UStaticMeshComponent* StaticMeshComp : StaticMeshComponents)
		{
			if (StaticMeshComp && StaticMeshComp->GetFName() == ComponentName)
			{
				return StaticMeshComp;
			}
		}

		return nullptr;
	}

	// [v2.108.0] StaticMesh가 Query 충돌에 사용할 Simple Collision을 가지고 있는지 검사합니다.
	bool HasStaticMeshSimpleCollision(const UStaticMeshComponent* StaticMeshComponent)
	{
		if (!StaticMeshComponent || !StaticMeshComponent->GetStaticMesh())
		{
			return false;
		}

		// [v2.108.0] Simple Collision 정보를 담고 있는 BodySetup입니다.
		const UBodySetup* BodySetup = StaticMeshComponent->GetStaticMesh()->GetBodySetup();
		if (!BodySetup)
		{
			return false;
		}

		return BodySetup->AggGeom.GetElementCount() > 0;
	}

	// [v2.102.0] 휠 메시 자동 스케일 요약 문자열에 항목을 이어 붙입니다.
	void AppendWheelMeshAutoScaleSummary(FString& InOutScaleSummary, const FString& ItemSummary)
	{
		if (!InOutScaleSummary.IsEmpty())
		{
			InOutScaleSummary += TEXT("; ");
		}

		InOutScaleSummary += ItemSummary;
	}

	// [v2.102.0] StaticMesh 로컬 바운드에서 설정된 측정 모드 기준 휠 반지름(cm)을 계산합니다.
	float MeasureWheelMeshRadiusCm(const UStaticMesh* WheelMesh, const ECFWheelMeshRadiusMeasureMode MeasureMode)
	{
		if (!WheelMesh)
		{
			return 0.0f;
		}

		// [v2.102.0] StaticMesh 에셋 로컬 공간의 원본 바운딩 박스입니다.
		const FBox WheelMeshBoundingBox = WheelMesh->GetBoundingBox();
		if (!WheelMeshBoundingBox.IsValid)
		{
			return 0.0f;
		}

		// [v2.102.0] 바운딩 박스 중심에서 각 축 끝까지의 거리입니다.
		const FVector WheelMeshBoxExtent = WheelMeshBoundingBox.GetExtent();

		switch (MeasureMode)
		{
		case ECFWheelMeshRadiusMeasureMode::AxisX:
			return WheelMeshBoxExtent.X;
		case ECFWheelMeshRadiusMeasureMode::AxisY:
			return WheelMeshBoxExtent.Y;
		case ECFWheelMeshRadiusMeasureMode::AxisZ:
			return WheelMeshBoxExtent.Z;
		case ECFWheelMeshRadiusMeasureMode::AutoMaxXZ:
		default:
			return FMath::Max(WheelMeshBoxExtent.X, WheelMeshBoxExtent.Z);
		}
	}

	// [v2.102.0] 목표 WheelRadius와 측정 반지름을 비교해 안전 범위로 제한된 표시 스케일을 계산합니다.
	bool CalculateWheelMeshScaleToRadius(
		const float TargetWheelRadiusCm,
		const float MeasuredWheelRadiusCm,
		const FCFVehicleWheelVisualConfig& WheelVisualConfig,
		float& OutWheelMeshScale)
	{
		if (TargetWheelRadiusCm <= 0.0f || MeasuredWheelRadiusCm <= KINDA_SMALL_NUMBER)
		{
			OutWheelMeshScale = 1.0f;
			return false;
		}

		// [v2.102.0] DA에서 입력한 최소 스케일을 안전 하한으로 보정한 값입니다.
		const float ConfigScaleClampMin = FMath::Max(0.01f, WheelVisualConfig.WheelMeshScaleClampMin);

		// [v2.102.0] DA에서 입력한 최대 스케일을 안전 하한으로 보정한 값입니다.
		const float ConfigScaleClampMax = FMath::Max(0.01f, WheelVisualConfig.WheelMeshScaleClampMax);

		// [v2.102.0] 최소/최대 입력이 뒤집혀도 실제 Clamp에 사용할 낮은 값입니다.
		const float SafeScaleClampMin = FMath::Min(ConfigScaleClampMin, ConfigScaleClampMax);

		// [v2.102.0] 최소/최대 입력이 뒤집혀도 실제 Clamp에 사용할 높은 값입니다.
		const float SafeScaleClampMax = FMath::Max(ConfigScaleClampMin, ConfigScaleClampMax);

		// [v2.102.0] 목표 반지름을 메시 원본 반지름으로 나눈 원본 스케일 배율입니다.
		const float RawWheelMeshScale = TargetWheelRadiusCm / MeasuredWheelRadiusCm;

		OutWheelMeshScale = FMath::Clamp(RawWheelMeshScale, SafeScaleClampMin, SafeScaleClampMax);
		return true;
	}

	// [v2.102.0] Wheel_Mesh_* 컴포넌트에 메시를 넣고 옵션이 켜진 경우 WheelRadius 기준 표시 스케일을 적용합니다.
	void ApplyWheelMeshVisualConfigToComponent(
		UStaticMeshComponent* WheelMeshComponent,
		UStaticMesh* WheelMesh,
		const float TargetWheelRadiusCm,
		const FCFVehicleWheelVisualConfig& WheelVisualConfig,
		FString& InOutScaleSummary)
	{
		if (!WheelMeshComponent)
		{
			return;
		}

		WheelMeshComponent->SetStaticMesh(WheelMesh);

		if (!WheelVisualConfig.bAutoScaleWheelMeshToRadius)
		{
			return;
		}

		// [v2.102.0] 현재 처리 중인 Wheel_Mesh_* 컴포넌트의 표시 이름입니다.
		const FString WheelMeshComponentName = WheelMeshComponent->GetName();

		if (!WheelMesh)
		{
			AppendWheelMeshAutoScaleSummary(InOutScaleSummary, FString::Printf(TEXT("%s=MeshMissing"), *WheelMeshComponentName));
			return;
		}

		// [v2.103.0] StaticMesh 원본 바운드의 중심 보정에도 사용할 로컬 바운딩 박스입니다.
		const FBox WheelMeshBoundingBox = WheelMesh->GetBoundingBox();

		// [v2.102.0] StaticMesh 원본 바운드에서 계산한 휠 반지름(cm)입니다.
		const float MeasuredWheelRadiusCm = MeasureWheelMeshRadiusCm(WheelMesh, WheelVisualConfig.WheelMeshRadiusMeasureMode);

		// [v2.102.0] WheelRadius 대비 적용할 최종 Uniform Scale 값입니다.
		float FinalWheelMeshScale = 1.0f;
		if (!CalculateWheelMeshScaleToRadius(TargetWheelRadiusCm, MeasuredWheelRadiusCm, WheelVisualConfig, FinalWheelMeshScale))
		{
			AppendWheelMeshAutoScaleSummary(InOutScaleSummary, FString::Printf(TEXT("%s=InvalidRadius(Target=%.2f,Measured=%.2f)"), *WheelMeshComponentName, TargetWheelRadiusCm, MeasuredWheelRadiusCm));
			return;
		}

		WheelMeshComponent->SetRelativeScale3D(FVector(FinalWheelMeshScale));

		// [v2.103.0] 자동 중심 보정 후 Wheel_Mesh 원점에 맞춰질 StaticMesh 바운드 중심입니다.
		FVector WheelMeshBoundsCenter = FVector::ZeroVector;
		if (WheelMeshBoundingBox.IsValid)
		{
			WheelMeshBoundsCenter = WheelMeshBoundingBox.GetCenter();
		}

		if (WheelVisualConfig.bAutoCenterWheelMeshBoundsToOrigin)
		{
			// [v2.103.0] 컴포넌트 회전과 스케일을 반영해 바운드 중심을 부모 공간에서 원점으로 되돌리는 위치 보정값입니다.
			const FVector WheelMeshCenterCorrection = WheelMeshComponent->GetRelativeRotation().RotateVector(-WheelMeshBoundsCenter * FinalWheelMeshScale);
			WheelMeshComponent->SetRelativeLocation(WheelMeshCenterCorrection);
		}

		AppendWheelMeshAutoScaleSummary(InOutScaleSummary, FString::Printf(TEXT("%s=Scale %.3f(Target=%.2f,Measured=%.2f,Center=%s,CenterFix=%s)"), *WheelMeshComponentName, FinalWheelMeshScale, TargetWheelRadiusCm, MeasuredWheelRadiusCm, *WheelMeshBoundsCenter.ToCompactString(), WheelVisualConfig.bAutoCenterWheelMeshBoundsToOrigin ? TEXT("On") : TEXT("Off")));
	}

	// [v2.48.0] 이름이 일치하는 SceneComponent를 Owner에서 찾습니다.
	USceneComponent* FindSceneComponentByName(const AActor* OwnerActor, const FName ComponentName)
	{
		if (!OwnerActor || ComponentName.IsNone())
		{
			return nullptr;
		}

		// [v2.48.0] Owner에 등록된 SceneComponent 후보 목록입니다.
		TArray<USceneComponent*> SceneComponents;
		OwnerActor->GetComponents<USceneComponent>(SceneComponents);
		for (USceneComponent* SceneComponent : SceneComponents)
		{
			if (SceneComponent && SceneComponent->GetFName() == ComponentName)
			{
				return SceneComponent;
			}
		}

		return nullptr;
	}

#if WITH_EDITOR
	// 하드포인트 선택 캡처 결과 카운트와 경고 요약입니다.
	struct FCFHardpointCaptureStats
	{
		// 소켓을 찾아 LocalTransform을 갱신한 하드포인트 슬롯 수입니다.
		int32 CapturedCount = 0;

		// SocketName이 비어 있어 캡처를 건너뛴 하드포인트 슬롯 수입니다.
		int32 SkippedCount = 0;

		// SocketName은 있지만 SM_Body 메시에서 찾지 못한 하드포인트 슬롯 수입니다.
		int32 MissingCount = 0;

		// LocationSlotId가 비어 있어 식별이 불완전한 하드포인트 슬롯 수입니다.
		int32 InvalidCount = 0;

		// 하드포인트 선택 캡처 중 발생한 경고 요약입니다.
		FString WarningSummary;
	};

	// [v2.71.0] 에디터 버튼 실행 결과를 Output Log, 화면 메시지, Slate 알림으로 함께 확인할 수 있게 합니다.
	void ShowVehicleLayoutEditorMessage(const FString& MessageText, const FColor& ScreenColor)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.0f, ScreenColor, MessageText);
		}

		// [v2.71.0] Details 패널 버튼 실행 결과를 PIE 외부에서도 볼 수 있게 띄우는 에디터 알림 정보입니다.
		FNotificationInfo NotificationInfo(FText::FromString(MessageText));
		NotificationInfo.ExpireDuration = 5.0f;
		NotificationInfo.bFireAndForget = true;

		// [v2.71.0] 화면 오른쪽 하단에 표시되는 Slate 알림 항목입니다.
		TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(NotificationInfo);
		if (NotificationItem.IsValid())
		{
			NotificationItem->SetCompletionState(ScreenColor == FColor::Red ? SNotificationItem::CS_Fail : SNotificationItem::CS_Success);
		}
	}

	// [v2.69.0] DataAsset 소켓 이름이 비어 있을 때 사용할 프로젝트 표준 이름을 반환합니다.
	FName ResolveWheelLayoutSocketName(const FName ConfiguredSocketName, const FName DefaultSocketName)
	{
		return ConfiguredSocketName.IsNone() ? DefaultSocketName : ConfiguredSocketName;
	}

	// [v2.69.0] 캡처 실패 요약에 항목을 쉼표로 이어 붙입니다.
	void AppendWheelLayoutCaptureFailure(FString& InOutFailureSummary, const FString& FailureText)
	{
		if (!InOutFailureSummary.IsEmpty())
		{
			InOutFailureSummary += TEXT(", ");
		}
		InOutFailureSummary += FailureText;
	}

	// [v2.74.0] 하드포인트 캡처 경고 요약에 항목을 쉼표로 이어 붙입니다.
	void AppendHardpointCaptureWarning(FCFHardpointCaptureStats& InOutStats, const FString& WarningText)
	{
		if (!InOutStats.WarningSummary.IsEmpty())
		{
			InOutStats.WarningSummary += TEXT(", ");
		}
		InOutStats.WarningSummary += WarningText;
	}

	// [v2.69.0] 차체 메시 소켓 Transform을 Wheel_Anchor_* 부모 기준 상대 포즈로 변환합니다.
	bool BuildWheelAnchorPoseFromBodySocket(
		const AActor* OwnerActor,
		const UStaticMeshComponent* BodyMeshComponent,
		const FName BodySocketName,
		const FName WheelAnchorName,
		FCFWheelAnchorPose& OutWheelAnchorPose,
		FString& InOutFailureSummary)
	{
		if (!OwnerActor)
		{
			AppendWheelLayoutCaptureFailure(InOutFailureSummary, TEXT("OwnerActor=Missing"));
			return false;
		}

		if (!BodyMeshComponent || !BodyMeshComponent->GetStaticMesh())
		{
			AppendWheelLayoutCaptureFailure(InOutFailureSummary, TEXT("SM_Body.StaticMesh=Missing"));
			return false;
		}

		if (BodySocketName.IsNone() || !BodyMeshComponent->DoesSocketExist(BodySocketName))
		{
			AppendWheelLayoutCaptureFailure(InOutFailureSummary, FString::Printf(TEXT("SocketMissing=%s"), *BodySocketName.ToString()));
			return false;
		}

		// [v2.69.0] 소켓 위치를 적용받을 Wheel_Anchor_* 컴포넌트입니다.
		const USceneComponent* WheelAnchorComponent = FindSceneComponentByName(OwnerActor, WheelAnchorName);
		if (!WheelAnchorComponent)
		{
			AppendWheelLayoutCaptureFailure(InOutFailureSummary, FString::Printf(TEXT("AnchorMissing=%s"), *WheelAnchorName.ToString()));
			return false;
		}

		// [v2.69.0] 차체 메시 소켓의 월드 Transform입니다.
		const FTransform SocketWorldTransform = BodyMeshComponent->GetSocketTransform(BodySocketName, RTS_World);

		// [v2.69.0] Wheel_Anchor_*가 상대 Transform을 저장할 기준 부모 Transform입니다.
		const USceneComponent* WheelAnchorParentComponent = WheelAnchorComponent->GetAttachParent();

		// [v2.69.0] 부모가 없을 때는 Actor Transform을 상대 기준으로 사용합니다.
		const FTransform AnchorParentWorldTransform = WheelAnchorParentComponent ? WheelAnchorParentComponent->GetComponentTransform() : OwnerActor->GetActorTransform();

		// [v2.69.0] 최종적으로 DataAsset에 저장할 Wheel_Anchor_* 상대 Transform입니다.
		const FTransform CapturedRelativeTransform = SocketWorldTransform.GetRelativeTransform(AnchorParentWorldTransform);

		OutWheelAnchorPose.RelativeLocation = CapturedRelativeTransform.GetLocation();
		OutWheelAnchorPose.RelativeRotation = CapturedRelativeTransform.Rotator();
		return true;
	}

	// [v2.74.0] SM_Body 소켓 Transform을 차체 기준 하드포인트 슬롯 LocalTransform에 선택적으로 캡처합니다.
	void CaptureHardpointSlotFromBodySocket(
		const UStaticMeshComponent* BodyMeshComponent,
		FCFVehicleHardpointSlot& InOutHardpointSlot,
		FCFHardpointCaptureStats& InOutStats)
	{
		if (InOutHardpointSlot.LocationSlotId.IsNone())
		{
			++InOutStats.InvalidCount;
			AppendHardpointCaptureWarning(InOutStats, TEXT("HardpointInvalidSlotId=None"));
		}

		if (InOutHardpointSlot.SocketName.IsNone())
		{
			++InOutStats.SkippedCount;
			return;
		}

		if (!BodyMeshComponent || !BodyMeshComponent->GetStaticMesh())
		{
			++InOutStats.MissingCount;
			AppendHardpointCaptureWarning(InOutStats, FString::Printf(TEXT("HardpointBodyMissing=%s"), *InOutHardpointSlot.SocketName.ToString()));
			return;
		}

		if (!BodyMeshComponent->DoesSocketExist(InOutHardpointSlot.SocketName))
		{
			++InOutStats.MissingCount;
			AppendHardpointCaptureWarning(InOutStats, FString::Printf(TEXT("HardpointSocketMissing=%s:%s"), *InOutHardpointSlot.LocationSlotId.ToString(), *InOutHardpointSlot.SocketName.ToString()));
			return;
		}

		// [v2.74.0] SM_Body 컴포넌트 기준으로 환산된 하드포인트 소켓 Transform입니다.
		const FTransform SocketComponentTransform = BodyMeshComponent->GetSocketTransform(InOutHardpointSlot.SocketName, RTS_Component);

		InOutHardpointSlot.LocalLocation = SocketComponentTransform.GetLocation();
		InOutHardpointSlot.LocalRotation = SocketComponentTransform.Rotator();
		++InOutStats.CapturedCount;
	}

	// [v2.74.0] 하드포인트 선택 캡처 결과에 경고가 포함되어 있는지 반환합니다.
	bool HasHardpointCaptureWarning(const FCFHardpointCaptureStats& CaptureStats)
	{
		return CaptureStats.MissingCount > 0 || CaptureStats.InvalidCount > 0;
	}
#endif

	// [v2.47.0] 이름이 일치하는 SkeletalMeshComponent를 Owner에서 찾습니다.
	USkeletalMeshComponent* FindSkeletalMeshComponentByName(const AActor* OwnerActor, const FName ComponentName)
	{
		if (!OwnerActor || ComponentName.IsNone())
		{
			return nullptr;
		}

		// [v2.47.0] Owner에 등록된 SkeletalMeshComponent 후보 목록입니다.
		TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
		OwnerActor->GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);
		for (USkeletalMeshComponent* SkeletalMeshComponent : SkeletalMeshComponents)
		{
			if (SkeletalMeshComponent && SkeletalMeshComponent->GetFName() == ComponentName)
			{
				return SkeletalMeshComponent;
			}
		}

		return nullptr;
	}

	// Triggered/Completed ????????????곸죷 ????력??袁⑸즴????獄?獄????살씁??癲ル슪?ｇ몭???筌뤾퍓???
	template<typename TriggeredHandlerType, typename CompletedHandlerType>
	void BindTriggeredCompletedInputAction(
		UEnhancedInputComponent* EnhancedInputComponent,
		UInputAction* SourceInputAction,
		ACFVehiclePawn* VehiclePawn,
		TriggeredHandlerType TriggeredHandler,
		CompletedHandlerType CompletedHandler)
	{
		if (!EnhancedInputComponent || !SourceInputAction || !VehiclePawn)
		{
			return;
		}

		EnhancedInputComponent->BindAction(SourceInputAction, ETriggerEvent::Triggered, VehiclePawn, TriggeredHandler);
		EnhancedInputComponent->BindAction(SourceInputAction, ETriggerEvent::Completed, VehiclePawn, CompletedHandler);
	}

	// Started/Completed ???????????????곸죷 ????력??袁⑸즴????獄?獄????살씁??癲ル슪?ｇ몭???筌뤾퍓???
	template<typename StartedHandlerType, typename CompletedHandlerType>
	void BindStartedCompletedInputAction(
		UEnhancedInputComponent* EnhancedInputComponent,
		UInputAction* SourceInputAction,
		ACFVehiclePawn* VehiclePawn,
		StartedHandlerType StartedHandler,
		CompletedHandlerType CompletedHandler)
	{
		if (!EnhancedInputComponent || !SourceInputAction || !VehiclePawn)
		{
			return;
		}

		EnhancedInputComponent->BindAction(SourceInputAction, ETriggerEvent::Started, VehiclePawn, StartedHandler);
		EnhancedInputComponent->BindAction(SourceInputAction, ETriggerEvent::Completed, VehiclePawn, CompletedHandler);
	}

	// ??れ삀??????????釉먯뒠??????WheelSync ??ш낄援ο쭛??????? ??癰귙끋源????れ삀??????뽮덫????⑤챶援??袁⑸즵????筌뤾퍓???
	FString StripWheelSyncRuntimeSummarySuffix(const FString& RuntimeSummary)
	{
		// ??れ삀??????????釉먯뒠??????WheelSyncBuild ?????? ??筌믨퀣援??嚥▲꺂痢???ш끽維?????낇돲??
		const int32 ExistingWheelSyncBuildIndex = RuntimeSummary.Find(TEXT(" | WheelSyncBuild="));

		// ??れ삀??????????釉먯뒠??????WheelSyncRuntime ?????? ??筌믨퀣援??嚥▲꺂痢???ш끽維?????낇돲??
		const int32 ExistingWheelSyncRuntimeIndex = RuntimeSummary.Find(TEXT(" | WheelSyncRuntime="));

		// ??れ삀???WheelSync ?????濚???좊읈????沃섅굥?? ?????嚥▲꺂痢???筌믨퀣援???ш끽維?????낇돲??
		int32 ExistingWheelSyncSummaryIndex = INDEX_NONE;

		if (ExistingWheelSyncBuildIndex != INDEX_NONE)
		{
			ExistingWheelSyncSummaryIndex = ExistingWheelSyncBuildIndex;
		}
		if (ExistingWheelSyncRuntimeIndex != INDEX_NONE)
		{
			ExistingWheelSyncSummaryIndex = (ExistingWheelSyncSummaryIndex != INDEX_NONE)
				? FMath::Min(ExistingWheelSyncSummaryIndex, ExistingWheelSyncRuntimeIndex)
				: ExistingWheelSyncRuntimeIndex;
		}

		return (ExistingWheelSyncSummaryIndex != INDEX_NONE)
			? RuntimeSummary.Left(ExistingWheelSyncSummaryIndex)
			: RuntimeSummary;
	}

#if WITH_EDITOR
	// [v2.137.0] 한 차량이 실제로 보유한 모든 Fitting 컴포넌트의 인스턴스와 상태를 로그 문자열로 만듭니다.
	FString BuildVehicleFittingComponentProbeSummary(const ACFVehiclePawn* VehiclePawn, int32& OutComponentCount)
	{
		OutComponentCount = 0;
		if (!IsValid(VehiclePawn))
		{
			return TEXT("VehiclePawn=Invalid");
		}

		// [v2.137.0] Actor에 실제로 연결된 모든 VehicleFittingComp 인스턴스입니다.
		TArray<UCFVehicleFittingComp*> VehicleFittingComponents;
		VehiclePawn->GetComponents<UCFVehicleFittingComp>(VehicleFittingComponents);
		OutComponentCount = VehicleFittingComponents.Num();

		// [v2.137.0] 컴포넌트 이름·주소·상태를 순서대로 누적할 최종 문자열입니다.
		FString ComponentProbeSummary;
		for (int32 ComponentIndex = 0; ComponentIndex < VehicleFittingComponents.Num(); ++ComponentIndex)
		{
			// [v2.137.0] 현재 순회 중인 피팅 컴포넌트 인스턴스입니다.
			const UCFVehicleFittingComp* VehicleFittingComponent = VehicleFittingComponents[ComponentIndex];
			if (ComponentIndex > 0)
			{
				ComponentProbeSummary += TEXT("; ");
			}

			if (!IsValid(VehicleFittingComponent))
			{
				ComponentProbeSummary += TEXT("Invalid");
				continue;
			}

			ComponentProbeSummary += FString::Printf(
				TEXT("%s@%p{Initial=%s,Apply=%s,MassSummary=%s,RuntimeSummary=%s}"),
				*VehicleFittingComponent->GetPathName(),
				static_cast<const void*>(VehicleFittingComponent),
				*UEnum::GetValueAsString(VehicleFittingComponent->GetInitialMassState()),
				*UEnum::GetValueAsString(VehicleFittingComponent->GetRuntimeApplyState()),
				*VehicleFittingComponent->GetLastInitialMassSummary(),
				*VehicleFittingComponent->GetLastFittingRuntimeSummary());
		}

		return ComponentProbeSummary.IsEmpty() ? TEXT("None") : ComponentProbeSummary;
	}

	// [v2.137.0] 한 차량이 실제로 보유한 모든 Defense 컴포넌트의 인스턴스와 상태를 로그 문자열로 만듭니다.
	FString BuildVehicleDefenseComponentProbeSummary(const ACFVehiclePawn* VehiclePawn, int32& OutComponentCount)
	{
		OutComponentCount = 0;
		if (!IsValid(VehiclePawn))
		{
			return TEXT("VehiclePawn=Invalid");
		}

		// [v2.137.0] Actor에 실제로 연결된 모든 VehicleDefenseComp 인스턴스입니다.
		TArray<UCFVehicleDefenseComp*> VehicleDefenseComponents;
		VehiclePawn->GetComponents<UCFVehicleDefenseComp>(VehicleDefenseComponents);
		OutComponentCount = VehicleDefenseComponents.Num();

		// [v2.137.0] 컴포넌트 이름·주소·상태를 순서대로 누적할 최종 문자열입니다.
		FString ComponentProbeSummary;
		for (int32 ComponentIndex = 0; ComponentIndex < VehicleDefenseComponents.Num(); ++ComponentIndex)
		{
			// [v2.137.0] 현재 순회 중인 방어 컴포넌트 인스턴스입니다.
			const UCFVehicleDefenseComp* VehicleDefenseComponent = VehicleDefenseComponents[ComponentIndex];
			if (ComponentIndex > 0)
			{
				ComponentProbeSummary += TEXT("; ");
			}

			if (!IsValid(VehicleDefenseComponent))
			{
				ComponentProbeSummary += TEXT("Invalid");
				continue;
			}

			ComponentProbeSummary += FString::Printf(
				TEXT("%s@%p{Ready=%s,Active=%s,Shield=%.3f/%.3f,Armor=%.3f/%.3f/%.3f/%.3f/%.3f/%.3f}"),
				*VehicleDefenseComponent->GetPathName(),
				static_cast<const void*>(VehicleDefenseComponent),
				VehicleDefenseComponent->IsDefenseInitialized() ? TEXT("Yes") : TEXT("No"),
				*GetPathNameSafe(VehicleDefenseComponent->GetActiveDefenseData()),
				VehicleDefenseComponent->GetCurrentShield(),
				VehicleDefenseComponent->GetMaximumShield(),
				VehicleDefenseComponent->GetCurrentArmor(ECFArmorDirection::Front),
				VehicleDefenseComponent->GetCurrentArmor(ECFArmorDirection::Left),
				VehicleDefenseComponent->GetCurrentArmor(ECFArmorDirection::Right),
				VehicleDefenseComponent->GetCurrentArmor(ECFArmorDirection::Rear),
				VehicleDefenseComponent->GetCurrentArmor(ECFArmorDirection::Top),
				VehicleDefenseComponent->GetCurrentArmor(ECFArmorDirection::Bottom));
		}

		return ComponentProbeSummary.IsEmpty() ? TEXT("None") : ComponentProbeSummary;
	}

	// [v2.137.0] 수동 PIE에서 차량별 컴포넌트 인스턴스와 초기화 상태를 한 줄로 기록합니다.
	void LogEditorPIEVehicleRuntimeProbe(const ACFVehiclePawn* VehiclePawn, const TCHAR* ProbePhase)
	{
		if (!IsValid(VehiclePawn))
		{
			return;
		}

		// [v2.137.0] PIE 여부와 World 경로를 판정할 현재 차량 World입니다.
		const UWorld* VehicleWorld = VehiclePawn->GetWorld();
		if (!VehicleWorld || VehicleWorld->WorldType != EWorldType::PIE)
		{
			return;
		}

		// [v2.137.0] Pawn의 C++ 소유 포인터가 가리키는 피팅 컴포넌트입니다.
		const UCFVehicleFittingComp* GetterFittingComponent = VehiclePawn->GetVehicleFittingComp();
		// [v2.137.0] Pawn의 C++ 소유 포인터가 가리키는 방어 컴포넌트입니다.
		const UCFVehicleDefenseComp* GetterDefenseComponent = VehiclePawn->GetVehicleDefenseComp();
		// [v2.137.0] 현재 차량의 Integrity를 제공하는 내구도 컴포넌트입니다.
		const UCFVehicleHealthComp* VehicleHealthComponent = VehiclePawn->GetVehicleHealthComp();
		// [v2.137.0] Actor에 연결된 전체 피팅 컴포넌트 수입니다.
		int32 FittingComponentCount = 0;
		// [v2.137.0] Actor에 연결된 전체 피팅 컴포넌트 인스턴스 상태입니다.
		const FString FittingComponentSummary = BuildVehicleFittingComponentProbeSummary(VehiclePawn, FittingComponentCount);
		// [v2.137.0] Actor에 연결된 전체 방어 컴포넌트 수입니다.
		int32 DefenseComponentCount = 0;
		// [v2.137.0] Actor에 연결된 전체 방어 컴포넌트 인스턴스 상태입니다.
		const FString DefenseComponentSummary = BuildVehicleDefenseComponentProbeSummary(VehiclePawn, DefenseComponentCount);

		UE_LOG(
			LogTemp,
			Display,
			TEXT("CF-FQ-033 ManualPIEProbe | Phase=%s | World=%s | Actor=%s@%p | BegunPlay=%s | Local=%s | Controller=%s | VehicleData=%s | FittingData=%s | FittingGetter=%s@%p | FittingCount=%d | FittingInstances=%s | DefenseGetter=%s@%p | DefenseCount=%d | DefenseInstances=%s | Integrity=%.3f | VehicleSummary=%s"),
			ProbePhase ? ProbePhase : TEXT("Unknown"),
			*VehicleWorld->GetPathName(),
			*VehiclePawn->GetPathName(),
			static_cast<const void*>(VehiclePawn),
			VehiclePawn->HasActorBegunPlay() ? TEXT("Yes") : TEXT("No"),
			VehiclePawn->IsLocallyControlled() ? TEXT("Yes") : TEXT("No"),
			*GetPathNameSafe(VehiclePawn->GetController()),
			*GetPathNameSafe(VehiclePawn->VehicleData.Get()),
			*GetPathNameSafe(VehiclePawn->VehicleFittingData.Get()),
			*GetPathNameSafe(GetterFittingComponent),
			static_cast<const void*>(GetterFittingComponent),
			FittingComponentCount,
			*FittingComponentSummary,
			*GetPathNameSafe(GetterDefenseComponent),
			static_cast<const void*>(GetterDefenseComponent),
			DefenseComponentCount,
			*DefenseComponentSummary,
			VehicleHealthComponent ? VehicleHealthComponent->GetCurrentIntegrity() : 0.0f,
			*VehiclePawn->LastVehicleRuntimeSummary);
	}

	// [v2.137.0] BeginPlay 직후와 1초 뒤 차량 상태를 다시 기록해 초기화 후 상태 교체 여부를 판정합니다.
	void ScheduleEditorPIEVehicleRuntimeProbes(ACFVehiclePawn* VehiclePawn)
	{
		if (!IsValid(VehiclePawn))
		{
			return;
		}

		// [v2.137.0] 지연 Probe 타이머를 소유할 현재 PIE World입니다.
		UWorld* VehicleWorld = VehiclePawn->GetWorld();
		if (!VehicleWorld || VehicleWorld->WorldType != EWorldType::PIE)
		{
			return;
		}

		// [v2.137.0] 타이머 실행 전에 Pawn이 종료될 수 있으므로 사용할 약한 참조입니다.
		const TWeakObjectPtr<ACFVehiclePawn> WeakVehiclePawn(VehiclePawn);
		VehicleWorld->GetTimerManager().SetTimerForNextTick(
			FTimerDelegate::CreateLambda([WeakVehiclePawn]()
			{
				if (WeakVehiclePawn.IsValid())
				{
					LogEditorPIEVehicleRuntimeProbe(WeakVehiclePawn.Get(), TEXT("NextTick"));
				}
			}));

		// [v2.137.0] 초기화 직후와 1초 후 상태를 비교할 일회성 타이머 핸들입니다.
		FTimerHandle OneSecondProbeTimerHandle;
		VehicleWorld->GetTimerManager().SetTimer(
			OneSecondProbeTimerHandle,
			FTimerDelegate::CreateLambda([WeakVehiclePawn]()
			{
				if (WeakVehiclePawn.IsValid())
				{
					LogEditorPIEVehicleRuntimeProbe(WeakVehiclePawn.Get(), TEXT("AfterOneSecond"));
				}
			}),
			1.0f,
			false);
	}
#endif

}

ACFVehiclePawn::ACFVehiclePawn()
{
	PrimaryActorTick.bCanEverTick = true;
	VehicleDriveComp = CreateDefaultSubobject<UCFVehicleDriveComp>(TEXT("VehicleDriveComp"));
	WheelSyncComp = CreateDefaultSubobject<UCFWheelSyncComp>(TEXT("WheelSyncComp"));
	VehicleCameraComp = CreateDefaultSubobject<UCFVehicleCameraComp>(TEXT("VehicleCameraComp"));
		VehicleAimComp = CreateDefaultSubobject<UCFVehicleAimComp>(TEXT("VehicleAimComp"));
		VehicleWeaponComp = CreateDefaultSubobject<UCFVehicleWeaponComp>(TEXT("VehicleWeaponComp"));

		// [v2.132.0] 출격 피팅 Snapshot과 Weapon·Defense 원자 적용 경계를 소유할 기본 서브오브젝트입니다.
	VehicleFittingComp = CreateDefaultSubobject<UCFVehicleFittingComp>(TEXT("VehicleFittingComp"));

	// [v2.139.0] 이번 출격의 실제 장전·예비·예약 탄약 상태를 단일 소유할 기본 서브오브젝트입니다.
	VehicleAmmoComp = CreateDefaultSubobject<UCFVehicleAmmoComp>(TEXT("VehicleAmmoComp"));
	LauncherComp = CreateDefaultSubobject<UCFLauncherComp>(TEXT("LauncherComp"));
	ProjectilePoolComp = CreateDefaultSubobject<UCFProjectilePoolComp>(TEXT("ProjectilePoolComp"));

		// [v2.111.0] 차량 최대/현재 내구도와 파괴 상태를 관리할 기존 이름의 기본 서브오브젝트입니다.
	VehicleHealthComp = CreateDefaultSubobject<UCFVehicleHealthComp>(TEXT("VehicleHealthComp"));

	// [v2.129.0] 쉴드와 6방향 장갑의 정식 피해 분배를 관리할 기본 서브오브젝트입니다.
	VehicleDefenseComp = CreateDefaultSubobject<UCFVehicleDefenseComp>(TEXT("VehicleDefenseComp"));
	CombatFxComp = CreateDefaultSubobject<UCFCombatFxComp>(TEXT("CombatFxComp"));

				// [v2.118.0] 후보와 지속 선택 대상의 최소 상태 계약을 관리할 기본 서브오브젝트입니다.
					TargetSelectComp = CreateDefaultSubobject<UCFTargetSelectComp>(TEXT("TargetSelectComp"));

	// [v2.144.0] TargetSelect와 독립적으로 Sensor Contact/Knowledge Snapshot을 소유할 기본 서브오브젝트입니다.
	VehicleSensorComp = CreateDefaultSubobject<UCFVehicleSensorComp>(TEXT("VehicleSensorComp"));

	// [v2.119.0] Per-vehicle target selection position. Disabled by default for bounds compatibility.
	TargetPointComp = CreateDefaultSubobject<UCFTargetPointComp>(TEXT("TargetPoint"));
	if (TargetPointComp)
	{
				TargetPointComp->SetupAttachment(GetMesh());
		TargetPointComp->bUseAsTargetPoint = true;
		TargetPointComp->bAutoAlignToPreferredBounds = true;
		TargetPointComp->PreferredBoundsComponentName = TEXT("SM_Body");
		TargetPointComp->PreferredBoundsLocalOffset = FVector::ZeroVector;
	}

	OwnerVisualRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("OwnerVisualRoot"));
	if (OwnerVisualRootComp)
	{
		OwnerVisualRootComp->SetupAttachment(GetMesh());
	}
	TurretMountRootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Turret_MountRoot"));
	if (TurretMountRootComp)
	{
		TurretMountRootComp->SetupAttachment(OwnerVisualRootComp ? OwnerVisualRootComp : GetMesh());
	}

	TurretBaseMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Turret_BaseMesh"));
	if (TurretBaseMeshComp)
	{
		TurretBaseMeshComp->SetupAttachment(TurretMountRootComp);
		TurretBaseMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TurretBaseMeshComp->SetGenerateOverlapEvents(false);
		TurretBaseMeshComp->SetCanEverAffectNavigation(false);
	}

	TurretYawPivotComp = CreateDefaultSubobject<USceneComponent>(TEXT("Turret_YawPivot"));
	if (TurretYawPivotComp)
	{
		TurretYawPivotComp->SetupAttachment(TurretBaseMeshComp ? static_cast<USceneComponent*>(TurretBaseMeshComp.Get()) : TurretMountRootComp.Get());
	}

	TurretYawMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Turret_YawMesh"));
	if (TurretYawMeshComp)
	{
		TurretYawMeshComp->SetupAttachment(TurretYawPivotComp ? TurretYawPivotComp.Get() : TurretMountRootComp.Get());
		TurretYawMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TurretYawMeshComp->SetGenerateOverlapEvents(false);
		TurretYawMeshComp->SetCanEverAffectNavigation(false);
	}

	TurretPitchPivotComp = CreateDefaultSubobject<USceneComponent>(TEXT("Turret_PitchPivot"));
	if (TurretPitchPivotComp)
	{
		TurretPitchPivotComp->SetupAttachment(TurretYawMeshComp ? static_cast<USceneComponent*>(TurretYawMeshComp.Get()) : TurretYawPivotComp.Get());
	}

	TurretPitchMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Turret_PitchMesh"));
	if (TurretPitchMeshComp)
	{
		// [v2.86.1] Pitch 메쉬 기본 부모로 사용할 터렛 시각 컴포넌트입니다.
		USceneComponent* PitchParentComponent = TurretPitchPivotComp ? TurretPitchPivotComp.Get() : TurretMountRootComp.Get();
		TurretPitchMeshComp->SetupAttachment(PitchParentComponent);
		TurretPitchMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		TurretPitchMeshComp->SetGenerateOverlapEvents(false);
		TurretPitchMeshComp->SetCanEverAffectNavigation(false);
	}

	// [v2.61.0] C++ 기본 객체 기준으로 싱글플레이 차량 기본값을 먼저 적용합니다.
	ApplyVehicleSinglePlayerBaseline();

	bAutoInitializeOnBeginPlay = true;
	bEnableWheelVisualTick = true;
	bAutoRegisterInputMappingContext = true;
	InputDeviceMode = ECFVehicleInputDeviceMode::Auto;
	InputDeviceAnalogThreshold = 0.1f;
	InputMappingPriority = 0;

	// [v2.44.0] 키보드/축 조향도 보간 경로를 타도록 기본 활성화합니다.
	bSmoothLegacySteeringInput = true;

	// [v2.44.0] LegacyAxis 목표 조향 초기값입니다.
	LegacyTargetSteeringInput = 0.0f;

		// [v2.134.0] 차량 주행·물리·내구도·피팅 코어 Runtime 준비 상태 초기값입니다.
	bVehicleCoreRuntimeReady = false;

	// [v2.134.0] 조준·무기·런처·타겟 선택을 포함한 전투 Runtime 준비 상태 초기값입니다.
	bVehicleCombatRuntimeReady = false;

	// [v2.134.0] 기존 Tick·Debug 호환용 차량 코어 Runtime 준비 상태 초기값입니다.
	bVehicleRuntimeReady = false;
	LastVehicleRuntimeSummary = TEXT("Constructed");
	bEnableDriveStateOnScreenDebug = false;
	bEnableVehicleDebugOnScreenMessage = false;
	DriveStateDebugDisplayMode = ECFVehicleDebugDisplayMode::SingleLine;
	bShowDriveStateTransitionSummary = true;
	bShowVehicleDebugHud = true;
	bShowVehicleDebugPanel = true;
	bShowVehicleDebugText = false;
	bShowVehicleDebugEvents = false;
	DriveStateDebugMessageDuration = 0.0f;
	bShowAimReticle = true;
	AimReticleZOrder = 10;

	// [v2.48.2] Owner 표시 루트 안정화 기본 사용 여부입니다.
	bEnableOwnerVisualStabilization = false;

	// [v2.48.1] Owner 표시 루트 안정화 기본 보간 속도입니다.
	OwnerVisualStabilizationInterpSpeed = 4.0f;

	// [v2.48.1] Owner 표시 루트 안정화 기본 최대 지연각입니다.
	OwnerVisualStabilizationMaxLagDeg = 30.0f;

	// [v2.48.1] Owner 표시 루트 Yaw 안정화 기본 사용 여부입니다.
	bOwnerVisualStabilizeYaw = false;

	// [v2.48.0] Owner 표시 루트 Pitch/Roll 안정화 기본 사용 여부입니다.
	bOwnerVisualStabilizePitchRoll = false;

	// [v2.48.0] Owner 표시 안정화 중 물리 루트 렌더링 숨김 기본 사용 여부입니다.
	bHideOwnerPhysicsMeshWhenStabilized = false;

	// [v2.53.0] Owner 차체 표시 안정화 기본 사용 여부입니다.
	bEnableOwnerBodyVisualStabilization = false;

	// [v2.53.1] Owner 차체 표시 안정화 기본 보간 속도입니다.
	OwnerBodyVisualInterpSpeed = 15.0f;

	// [v2.53.1] Owner 차체 표시 안정화 기본 최대 지연각입니다.
	OwnerBodyVisualMaxLagDeg = 5.0f;

	// [v2.53.2] Owner 차체 표시 Yaw 안정화 기본 사용 여부입니다.
	bOwnerBodyVisualStabilizeYaw = false;

	// [v2.53.0] Owner 차체 표시 Pitch/Roll 안정화 기본 사용 여부입니다.
	bOwnerBodyVisualStabilizePitchRoll = false;

	// [v2.48.0] Owner 표시 안정화 준비 상태 초기값입니다.
	bOwnerVisualStabilizationReady = false;

	// [v2.48.0] Owner 표시 안정화 표시 회전 초기값입니다.
	SmoothedOwnerVisualRotation = FRotator::ZeroRotator;

	// [v2.48.0] Owner 표시 안정화 회전 기준값 유효 여부 초기값입니다.
	bHasSmoothedOwnerVisualRotation = false;

	// [v2.48.0] Owner 표시 안정화로 물리 루트 렌더링을 숨겼는지 여부 초기값입니다.
	bOwnerVisualPhysicsMeshHidden = false;

	// [v2.53.0] Owner 차체 표시 안정화 준비 상태 초기값입니다.
	bOwnerBodyVisualStabilizationReady = false;

	// [v2.53.0] Owner 차체 표시 안정화 표시 회전 초기값입니다.
	SmoothedOwnerBodyVisualRotation = FRotator::ZeroRotator;

	// [v2.53.0] Owner 차체 표시 안정화 회전 기준값 유효 여부 초기값입니다.
	bHasSmoothedOwnerBodyVisualRotation = false;

	// [v2.53.0] Owner 차체 표시 안정화 전 SM_Body 기본 상대 회전 초기값입니다.
	OriginalOwnerBodyVisualRelativeRotation = FRotator::ZeroRotator;

	// [v2.53.0] Owner 차체 표시 안정화 전 SM_Body 상대 회전 저장 여부 초기값입니다.
	bHasOriginalOwnerBodyVisualRelativeRotation = false;

	// [v2.21.0] Dedicated Server 테스트에서 C++ 기본 Pawn이 로컬 Player0을 강제 점유하지 않도록 기본값을 비활성화합니다.
	AutoPossessPlayer = EAutoReceiveInput::Disabled;

	// [v2.14.0] 입력 자산은 BP/파생 클래스에서 지정한 값을 우선 사용하고, 비어 있을 때만 기본 fallback 자산을 로드합니다.
	if (!DefaultInputMappingContext)
	{
		DefaultInputMappingContext = LoadObject<UInputMappingContext>(nullptr, TEXT("/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default"));
	}

	if (!InputAction_Throttle)
	{
		InputAction_Throttle = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_Throttle.IA_Throttle"));
	}

	if (!InputAction_Steering)
	{
		InputAction_Steering = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_Steering.IA_Steering"));
	}

	if (!InputAction_Brake)
	{
		InputAction_Brake = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_Brake.IA_Brake"));
	}

	if (!InputAction_Handbrake)
	{
		InputAction_Handbrake = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_Handbrake.IA_Handbrake"));
	}

				if (!InputAction_Look)
	{
		InputAction_Look = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_LookAround.IA_LookAround"));
	}

		if (!InputAction_SelectTarget)
	{
		InputAction_SelectTarget = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_SelectTarget.IA_SelectTarget"));
	}

												if (!InputAction_ClearTarget)
	{
		InputAction_ClearTarget = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_ClearTarget.IA_ClearTarget"));
	}

	// [v2.152.0] P0 숫자키 1~9가 실제 selectable weapon ordinal을 전달할 기본 Axis1D Input Action입니다.
	if (!InputAction_SelectWeapon)
	{
		InputAction_SelectWeapon = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_SelectWeapon.IA_SelectWeapon"));
	}

	// [v2.154.0] Mouse Scroll Up/Down이 Provider-local Radar Range Preset 변경을 전달할 기본 Axis1D Input Action입니다.
	if (!InputAction_RadarZoom)
	{
		InputAction_RadarZoom = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_RadarZoom.IA_RadarZoom"));
	}

	// [v2.147.0] P0의 1회 입력→ActiveScanDurationSec 자동 실행에 사용할 기본 Active Scan Input Action입니다.
	if (!InputAction_StartActiveScan)
	{
		InputAction_StartActiveScan = LoadObject<UInputAction>(nullptr, TEXT("/Game/CarFight/Input/IA_ActiveScan.IA_ActiveScan"));
	}

	if (!TargetSelectWidgetClass)
	{
		TargetSelectWidgetClass = LoadClass<UCFTargetSelectWidget>(nullptr, TEXT("/Game/CarFight/UI/WBP_TargetSelect.WBP_TargetSelect_C"));
	}
}

// [v2.118.0] 현재 차량이 주어진 컨텍스트에서 선택 가능한지 반환합니다.
bool ACFVehiclePawn::IsTargetSelectable_Implementation(const FCFTargetSelectionContext& SelectionContext) const
{
	// [v2.118.0] 자기 선택 허용 여부는 선택을 수행하는 TargetSelectComp에서 검사합니다.
	(void)SelectionContext;

	return IsValid(this)
		&& IsValid(VehicleHealthComp)
		&& !VehicleHealthComp->IsDestroyed();
}

// [v2.156.0] 타겟 HUD와 Sensor Knowledge가 사용할 차량 표시 정보를 VehicleData의 안정 PrimaryAssetId에서 만들고 내부 Actor 이름은 공개하지 않습니다.
FCFTargetDisplayInfo ACFVehiclePawn::GetTargetDisplayInfo_Implementation() const
{
	// [v2.156.0] Vehicle category/tag와 Player-facing 이름 비공개 계약을 유지하면서 안정 TargetId를 선택적으로 채울 표시 정보입니다.
	FCFTargetDisplayInfo DisplayInfo;
	DisplayInfo.TargetCategory = ECFTargetCategory::Vehicle;
	DisplayInfo.Relation = ECFTargetRelation::Unknown;
	DisplayInfo.InformationLevel = ECFTargetInfoLevel::Identified;
	DisplayInfo.AttributeTags.Add(FName(TEXT("Vehicle")));

	if (IsValid(VehicleData))
	{
		// [v2.156.0] Pawn instance 이름과 무관하게 같은 VehicleData가 같은 TargetId를 제공하도록 사용할 Primary Asset 식별자입니다.
		const FPrimaryAssetId VehiclePrimaryAssetId = VehicleData->GetPrimaryAssetId();
		if (VehiclePrimaryAssetId.IsValid())
		{
			DisplayInfo.TargetId = VehiclePrimaryAssetId.PrimaryAssetName;
		}
	}

	return DisplayInfo;
}

// [v2.118.0] 별도 TargetPoint가 연결되기 전 사용할 차량 Bounds 중심을 반환합니다.
FVector ACFVehiclePawn::GetTargetSelectionLocation_Implementation() const
{
	return UCFTargetPointComp::ResolveTargetPoint(this).WorldLocation;
}

// [v2.118.0] 파괴되지 않은 차량의 P0 기본 추적 상태를 반환합니다.
ECFTargetTrackState ACFVehiclePawn::GetTargetTrackState_Implementation() const
{
	return IsValid(VehicleHealthComp) && !VehicleHealthComp->IsDestroyed()
		? ECFTargetTrackState::Visible
		: ECFTargetTrackState::Invalid;
}

UStaticMeshComponent* ACFVehiclePawn::GetVehicleBodyMeshComponent() const
{
	TArray<UStaticMeshComponent*> StaticMeshComponents;
	GetComponents<UStaticMeshComponent>(StaticMeshComponents);

	for (UStaticMeshComponent* StaticMeshComponent : StaticMeshComponents)
	{
		if (IsValid(StaticMeshComponent) && StaticMeshComponent->GetFName() == FName(TEXT("SM_Body")))
		{
			return StaticMeshComponent;
		}
	}

	return nullptr;
}

bool ACFVehiclePawn::ConfirmCurrentTargetCandidate()
{
	if (!TargetSelectComp)
	{
		return false;
	}

	AActor* CurrentCandidateActor = TargetSelectComp->GetCurrentCandidateActor();
	if (!CurrentCandidateActor)
	{
		return false;
	}

	return TargetSelectComp->SetSelectedTarget(CurrentCandidateActor, TargetSelectComp->GetDefaultSelectionContext());
}

bool ACFVehiclePawn::ClearSelectedTargetManually()
{
	if (!TargetSelectComp || !TargetSelectComp->HasSelectedTarget())
	{
		return false;
	}

	TargetSelectComp->ClearSelectedTarget(ECFTargetClearReason::Manual);
	return true;
}

// [v2.5.2] Construction 시점에 차체뿐 아니라 휠 메시도 기존 Wheel_Mesh_* 컴포넌트에 적용해 에디터 뷰포트 미리보기를 갱신합니다.
void ACFVehiclePawn::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyVehicleVisualConfig();
	ApplyVehicleWheelVisualConfig();
	ApplyVehicleLayoutConfig();
	ApplyVehicleTurretVisualConfig();
}

// [v2.133.0] 게임 World의 물리 컴포넌트 등록 전에 초기 피팅 Snapshot 질량을 Movement Mass에 1회 기록합니다.
void ACFVehiclePawn::PreRegisterAllComponents()
{
#if WITH_EDITOR
	LogEditorPIEVehicleRuntimeProbe(this, TEXT("PreRegister.BeforePrepare"));
#endif

	// [v2.133.0] CDO·Editor Preview·수동 초기화 Pawn에서 초기 출격 질량을 기록하지 않을지 여부입니다.
	const bool bCanPrepareInitialSortieMass = !HasAnyFlags(RF_ClassDefaultObject)
		&& bAutoInitializeOnBeginPlay
		&& GetWorld()
		&& GetWorld()->IsGameWorld();
	if (bCanPrepareInitialSortieMass)
	{
		PrepareInitialSortieRuntimeMass();
	}

#if WITH_EDITOR
	LogEditorPIEVehicleRuntimeProbe(this, TEXT("PreRegister.AfterPrepareBeforeSuper"));
#endif

	Super::PreRegisterAllComponents();

#if WITH_EDITOR
	LogEditorPIEVehicleRuntimeProbe(this, TEXT("PreRegister.AfterSuper"));
#endif
}

void ACFVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

#if WITH_EDITOR
	LogEditorPIEVehicleRuntimeProbe(this, TEXT("BeginPlay.AfterSuperBeforeInitialize"));
#endif

	// [v2.61.0] BP 저장값이 이전 네트워크 테스트 기준으로 남아 있어도 런타임 싱글플레이 기준선을 보장합니다.
	ApplyVehicleSinglePlayerBaseline();

	// [v2.21.0] 로컬 Viewport/입력 UI 처리를 실행할 수 있는 Pawn인지 여부입니다.
	const bool bCanRunLocalPresentation = (GetNetMode() != NM_DedicatedServer) && IsLocallyControlled();
	if (bCanRunLocalPresentation && bAutoRegisterInputMappingContext)
	{
		RegisterDefaultInputMappingContext();
	}
	if (bAutoInitializeOnBeginPlay)
	{
		InitializeVehicleRuntime();
	}

#if WITH_EDITOR
	LogEditorPIEVehicleRuntimeProbe(this, TEXT("BeginPlay.AfterInitialize"));
	ScheduleEditorPIEVehicleRuntimeProbes(this);
#endif

			// [v2.149.0] AimReticle과 TargetSelect Marker의 자동 생성·Rebind 수명은 UCFUISubsystem이 소유하므로 Pawn BeginPlay에서는 UI Widget을 직접 만들지 않습니다.
	(void)bCanRunLocalPresentation;
}

void ACFVehiclePawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
#if WITH_EDITOR
	LogEditorPIEVehicleRuntimeProbe(this, TEXT("EndPlay.BeforeReset"));
#endif

		// [v2.132.0] 이전 출격의 Prepared·Applied Snapshot이 다음 수명으로 남지 않게 정리합니다.
	if (VehicleFittingComp)
	{
		VehicleFittingComp->ResetFittingRuntimeState();
	}

	// [v2.139.0] 이전 출격의 장전·예비·예약 탄약과 통계가 다음 Pawn 수명으로 남지 않게 정리합니다.
	if (VehicleAmmoComp)
	{
		VehicleAmmoComp->ResetAmmoRuntime();
	}

#if WITH_EDITOR
	LogEditorPIEVehicleRuntimeProbe(this, TEXT("EndPlay.AfterReset"));
#endif

	DestroyTargetSelectWidget();
	DestroyAimReticleWidget();

	Super::EndPlay(EndPlayReason);
}

void ACFVehiclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bVehicleRuntimeReady)
	{
				DisplayDriveStateOnScreenDebug();
		return;

	}

	// [v2.8.0] VehicleMove 기반 조향은 입력 이벤트가 없는 동안에도 중립 복귀가 필요하므로 Tick에서 계속 갱신합니다.
	if ((GetNetMode() != NM_DedicatedServer) && IsLocallyControlled())
	{
		UpdateVehicleMoveSteeringInput(DeltaSeconds);
	}

	// [v2.37.0] 정리 기준선에서 원격 Visual/Shell 분기 없이 WheelSync 시각 갱신을 실행할 수 있는지 여부입니다.
	const bool bCanUpdateVehicleWheelVisuals = (GetNetMode() != NM_DedicatedServer)
		&& bEnableWheelVisualTick;
	if (bCanUpdateVehicleWheelVisuals)
	{
		UpdateVehicleWheelVisuals(DeltaSeconds);
	}
	UpdateOwnerVisualStabilization(DeltaSeconds);
	UpdateOwnerBodyVisualStabilization(DeltaSeconds);
	UpdateVehicleTurretAimVisuals(DeltaSeconds);
	DisplayDriveStateOnScreenDebug();
}

// [v2.61.0] 차량 Pawn의 싱글플레이 기본 복제 상태를 적용합니다.
void ACFVehiclePawn::ApplyVehicleSinglePlayerBaseline()
{
	// [v2.61.0] 싱글플레이 차량 Actor는 네트워크 복제 대상이 아닙니다.
	bReplicates = false;

	// [v2.61.0] 싱글플레이 차량 이동은 로컬 물리와 입력만 사용하므로 Actor Movement Replication을 끕니다.
	SetReplicateMovement(false);
}

void ACFVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled())
	{
		return;
	}

	if (bAutoRegisterInputMappingContext)
	{
		RegisterDefaultInputMappingContext();
	}

	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EnhancedInputComponent)
	{
		return;
	}

	BindTriggeredCompletedInputAction(
		EnhancedInputComponent,
		InputAction_VehicleMove,
		this,
		&ACFVehiclePawn::HandleVehicleMoveInput,
		&ACFVehiclePawn::HandleVehicleMoveReleased);
	BindTriggeredCompletedInputAction(
		EnhancedInputComponent,
		InputAction_Throttle,
		this,
		&ACFVehiclePawn::HandleThrottleInput,
		&ACFVehiclePawn::HandleThrottleReleased);
	BindTriggeredCompletedInputAction(
		EnhancedInputComponent,
		InputAction_Steering,
		this,
		&ACFVehiclePawn::HandleSteeringInput,
		&ACFVehiclePawn::HandleSteeringReleased);
	BindTriggeredCompletedInputAction(
		EnhancedInputComponent,
		InputAction_Brake,
		this,
		&ACFVehiclePawn::HandleBrakeInput,
		&ACFVehiclePawn::HandleBrakeReleased);
	BindTriggeredCompletedInputAction(
		EnhancedInputComponent,
		InputAction_Look,
		this,
		&ACFVehiclePawn::HandleLookInput,
		&ACFVehiclePawn::HandleLookReleased);
	BindStartedCompletedInputAction(
		EnhancedInputComponent,
		InputAction_Handbrake,
		this,
		&ACFVehiclePawn::HandleHandbrakeStarted,
		&ACFVehiclePawn::HandleHandbrakeCompleted);
				if (InputAction_Fire)
	{
		EnhancedInputComponent->BindAction(InputAction_Fire, ETriggerEvent::Started, this, &ACFVehiclePawn::HandleFireStarted);
	}
		if (InputAction_SelectTarget)
	{
		EnhancedInputComponent->BindAction(InputAction_SelectTarget, ETriggerEvent::Started, this, &ACFVehiclePawn::HandleSelectTargetStarted);
	}
				if (InputAction_ClearTarget)
	{
		EnhancedInputComponent->BindAction(InputAction_ClearTarget, ETriggerEvent::Started, this, &ACFVehiclePawn::HandleClearTargetStarted);
	}
	// [v2.152.0] 숫자키 ordinal은 Started 1회만 Gameplay selection command로 전달합니다.
	if (InputAction_SelectWeapon)
	{
		EnhancedInputComponent->BindAction(InputAction_SelectWeapon, ETriggerEvent::Started, this, &ACFVehiclePawn::HandleSelectWeaponStarted);
	}
	// [v2.154.0] Mouse Scroll의 1회 디지털 입력값을 Radar 표시 Range 단계 변경으로 전달합니다.
	if (InputAction_RadarZoom)
	{
		EnhancedInputComponent->BindAction(InputAction_RadarZoom, ETriggerEvent::Started, this, &ACFVehiclePawn::HandleRadarZoomStarted);
	}
	if (InputAction_StartActiveScan)
	{
		EnhancedInputComponent->BindAction(InputAction_StartActiveScan, ETriggerEvent::Started, this, &ACFVehiclePawn::HandleStartActiveScanStarted);
	}
	if (InputAction_StopActiveScan)
	{
		EnhancedInputComponent->BindAction(InputAction_StopActiveScan, ETriggerEvent::Started, this, &ACFVehiclePawn::HandleStopActiveScanStarted);
	}

			// [v2.149.0] AimReticle과 TargetSelect Marker는 UISubsystem이 Current Pawn에 Rebind하므로 Pawn 입력 준비 단계에서 Widget을 직접 생성하지 않습니다.
}

// [v2.142.0] 현재 활성 WeaponInstance의 FullMagazine 재장전을 Gameplay 명령으로 요청합니다.
ECFAmmoTransactionResult ACFVehiclePawn::RequestReloadCurrentWeapon()
{
	if (!VehicleAmmoComp || !VehicleWeaponComp)
	{
		return ECFAmmoTransactionResult::MissingWeaponRuntime;
	}

	return VehicleAmmoComp->RequestActiveWeaponReload(VehicleWeaponComp);
}

// [v2.151.0] Applied Fitting의 실제 고정 표시 순서에서 무기를 선택하고 Launcher·Weapon Runtime·단일 활성 Turret Visual을 안전한 순서로 전환합니다.
bool ACFVehiclePawn::RequestSelectWeaponIndex(const int32 NewWeaponIndex)
{
	if (!VehicleWeaponComp
		|| !VehicleWeaponComp->HasWeaponSelectionRuntime()
		|| NewWeaponIndex < 0
		|| NewWeaponIndex >= VehicleWeaponComp->GetSelectableWeaponCount())
	{
		return false;
	}

	if (NewWeaponIndex == VehicleWeaponComp->GetSelectedWeaponIndex())
	{
		return true;
	}

	// [v2.151.0] 이미 예약된 Ripple·Salvo 후속 발사가 이전 무기 상태로 계속 실행되지 않도록 기존 정상 취소 계약을 먼저 사용합니다.
	if (LauncherComp && LauncherComp->IsFireSequenceActive())
	{
		LauncherComp->CancelFireSequence(ECFLauncherSequenceCancelReason::WeaponChanged);
	}

	if (!VehicleWeaponComp->ApplySelectedWeaponIndex(NewWeaponIndex))
	{
		return false;
	}

	// [v2.151.0] 현재 Pawn의 단일 활성 터렛 시각을 새 WeaponComp active mount/preset과 같은 source로 다시 구성합니다.
	ApplyVehicleTurretVisualConfig();
	return true;
}

// [v2.146.0] Pawn 입력 계층에서 VehicleSensorComp의 기존 Active Scan 시작 명령만 호출합니다.
bool ACFVehiclePawn::RequestStartActiveScan()
{
	return IsValid(VehicleSensorComp)
		&& VehicleSensorComp->StartActiveScan();
}

// [v2.146.0] Pawn 입력 계층에서 VehicleSensorComp의 기존 Active Scan 중단 명령만 호출합니다.
bool ACFVehiclePawn::RequestStopActiveScan()
{
	return IsValid(VehicleSensorComp)
		&& VehicleSensorComp->StopActiveScan();
}

bool ACFVehiclePawn::RegisterDefaultInputMappingContext()
{
	if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled())
	{
		return false;
	}

	// [v2.21.0] Enhanced Input 매핑을 등록할 로컬 플레이어 컨트롤러입니다.
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return false;
	}
	// [v2.21.0] Enhanced Input Subsystem을 소유한 로컬 플레이어입니다.
	ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return false;
	}
	// [v2.21.0] 실제 Input Mapping Context를 추가할 Enhanced Input Subsystem입니다.
	UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	if (!EnhancedInputSubsystem || !DefaultInputMappingContext)
	{
		return false;
	}
	EnhancedInputSubsystem->AddMappingContext(DefaultInputMappingContext, InputMappingPriority);
	return true;
}

// [v2.68.0] VehicleData와 표시 계층을 준비한 뒤 WheelSync가 최종 앵커 기준을 캡처할 수 있게 런타임을 초기화합니다.
bool ACFVehiclePawn::InitializeVehicleRuntime()
{
	// [v2.134.0] 새 초기화 시도에서 차량 코어 Runtime 준비 상태를 먼저 초기화합니다.
	bVehicleCoreRuntimeReady = false;

	// [v2.134.0] 새 초기화 시도에서 전투 Runtime 준비 상태를 먼저 초기화합니다.
	bVehicleCombatRuntimeReady = false;

	// [v2.134.0] 기존 호환 준비 상태도 차량 코어 상태와 함께 초기화합니다.
		bVehicleRuntimeReady = false;
	LastVehicleRuntimeSummary = TEXT("VehicleRuntime: InitializeStarted");

	// [v2.139.0] 명시 재초기화에서 이전 출격 탄약 상태가 재사용되지 않도록 P0-02 Runtime을 먼저 비웁니다.
	if (VehicleAmmoComp)
	{
		VehicleAmmoComp->ResetAmmoRuntime();
	}
	ApplyVehicleDataConfig();

	// [v2.68.0] VehicleData 기반 공통 설정 적용 직후의 요약 문자열입니다.
	const FString DataConfigSummary = LastVehicleRuntimeSummary;

	// [v2.48.0] 로컬 Owner 표시 안정화 계층 준비 결과입니다.
	const bool bOwnerVisualReady = PrepareOwnerVisualStabilization();

	// [v2.68.0] Owner 표시 루트 재부착 이후 최종 부모 기준으로 레이아웃을 다시 적용합니다.
	ApplyVehicleLayoutConfig();

	// [v2.86.0] Owner 표시 루트 재부착 이후 최종 부모 기준으로 터렛 시각 장착을 다시 적용합니다.
	ApplyVehicleTurretVisualConfig();

	// [v2.68.0] WheelSync 캡처 직전에 확정된 레이아웃 적용 요약 문자열입니다.
	const FString LayoutConfigSummary = LastVehicleRuntimeSummary;

	// [v2.86.0] VehicleRuntime 요약에 함께 남길 최신 터렛 시각 장착 요약입니다.
	const FString TurretVisualConfigSummary = LastTurretVisualSummary;

	// [v2.68.0] 차량 입력/물리 Drive 컴포넌트 캐시 준비 결과입니다.
	const bool bDriveReady = (VehicleDriveComp != nullptr) && VehicleDriveComp->CacheVehicleMovementComponent();

	// [v2.68.0] DataAsset 레이아웃 적용 이후 WheelSync 준비가 성공했는지 여부입니다.
	const bool bWheelSyncReady = PrepareWheelSync();

		// [v2.15.0] AimComp가 Owner Pawn과 VehicleCameraComp를 안전하게 찾았는지 여부입니다.
	const bool bAimReady = VehicleAimComp ? VehicleAimComp->InitializeAimRuntime() : false;

	// [v2.111.0] VehicleData 최대 내구도 또는 안전 기본값으로 차량 내구도가 준비됐는지 여부입니다.
	const bool bHealthReady = VehicleHealthComp ? VehicleHealthComp->InitializeFromVehicleData(VehicleData) : false;

	// [v2.132.0] 기존 활성 프로파일을 Snapshot Weapon 적용 대상으로 유지할 ID입니다.
	const FName RequestedActiveMountProfileId = VehicleWeaponComp
		? VehicleWeaponComp->GetActiveMountProfileId()
		: NAME_None;

		// [v2.133.0] 첫 BeginPlay는 PreRegister의 Cached 입력을 사용하고, 명시 재초기화는 같은 질량인지 검증한 새 입력만 준비합니다.
	const bool bFittingPrepared = VehicleFittingComp
		? (VehicleFittingComp->HasPreparedRuntimeInput()
			? true
			: VehicleFittingComp->PrepareInitialSortieFitting(VehicleFittingData, VehicleData, RequestedActiveMountProfileId))
		: false;

	// [v2.133.0] Snapshot 경로는 Configured Mass와 VehicleMesh 실제 질량 검증을 통과해야 하위 Runtime Commit을 허용합니다.
	const bool bInitialMassReady = bFittingPrepared && VerifyInitialSortieRuntimeMass();

	// [v2.133.0] 질량 검증을 통과한 같은 Cached Snapshot의 Weapon·Defense 입력만 한 트랜잭션으로 Commit합니다.
	const bool bFittingApplied = bInitialMassReady && VehicleFittingComp
		? VehicleFittingComp->CommitPreparedSortieFittingToVehicle(this, VehicleWeaponComp, VehicleDefenseComp)
		: false;

		// [v2.132.0] Commit 후 실제 Weapon Runtime 준비 상태입니다.
	const bool bWeaponReady = VehicleWeaponComp && VehicleWeaponComp->IsWeaponRuntimeReady();

	// [v2.143.0] Ammo 기본 서브오브젝트 존재와 finite Snapshot 초기화 결과를 합친 전투 탄약 준비 상태입니다.
	bool bAmmoReady = VehicleAmmoComp != nullptr;

	// [v2.143.0] 이번 Applied Snapshot에서 실제 finite Ammo Runtime 초기화가 필요한 무기가 하나라도 있는지 여부입니다.
	bool bFiniteAmmoRuntimeRequired = false;

	if (bFittingApplied && VehicleFittingComp && VehicleFittingComp->HasAppliedFittingSnapshot())
	{
		// [v2.143.0] Initial Mass와 Weapon·Defense Commit에 사용한 바로 그 Applied Fitting Snapshot 복사본입니다.
		const FCFVehicleFittingSnapshot AppliedFittingSnapshot = VehicleFittingComp->GetAppliedFittingSnapshot();

		// [v2.143.0] WeaponInstanceId별 독립 장전 상태를 만들 finite 무기 초기화 입력 목록입니다.
		TArray<FCFWeaponAmmoInitialization> WeaponAmmoInitializations;
		for (const FCFResolvedFittingMount& ResolvedMount : AppliedFittingSnapshot.ResolvedMounts)
		{
			UCFWeaponData* ResolvedWeaponData = ResolvedMount.WeaponData;
			if (!IsValid(ResolvedWeaponData) || ResolvedWeaponData->bUseInfiniteAmmoForDebug)
			{
				continue;
			}

			bFiniteAmmoRuntimeRequired = true;
			if (!ResolvedWeaponData->UsesFiniteAmmoRuntime() || ResolvedMount.MountProfileId.IsNone())
			{
				bAmmoReady = false;
				continue;
			}

			// [v2.143.0] 같은 WeaponData를 여러 Mount에 장착해도 Loaded를 독립 소유하게 할 WeaponInstance 초기화 입력입니다.
			FCFWeaponAmmoInitialization WeaponAmmoInitialization;
			WeaponAmmoInitialization.WeaponInstanceId = ResolvedMount.MountProfileId;
			WeaponAmmoInitialization.WeaponData = ResolvedWeaponData;
			WeaponAmmoInitialization.InitialLoadedAmmoCountOverride = INDEX_NONE;
			WeaponAmmoInitializations.Add(WeaponAmmoInitialization);
		}

		// [v2.143.0] 명시적 출격 탄약 또는 finite WeaponInstance가 있으면 VehicleAmmoComp에 실제 Runtime을 구성해야 하는지 여부입니다.
		const bool bShouldInitializeAmmoRuntime = !AppliedFittingSnapshot.InitialSortieAmmoLoads.IsEmpty()
			|| !WeaponAmmoInitializations.IsEmpty();
		if (bAmmoReady && bShouldInitializeAmmoRuntime)
		{
			bAmmoReady = VehicleAmmoComp
				&& VehicleAmmoComp->InitializeAmmoRuntime(
					this,
					AppliedFittingSnapshot.InitialSortieAmmoLoads,
					WeaponAmmoInitializations);
		}
		else if (bFiniteAmmoRuntimeRequired)
		{
			bAmmoReady = false;
		}
	}

	// [v2.132.0] Snapshot 장비가 반영된 최종 Weapon 캐시로 터렛 시각화를 다시 적용합니다.
	if (bFittingApplied)
	{
		ApplyVehicleTurretVisualConfig();
	}

	// [v2.126.0] LauncherComp를 최종 Weapon Runtime에 연결합니다.
	const bool bLauncherReady = LauncherComp ? LauncherComp->InitializeLauncherRuntime(this, VehicleWeaponComp) : false;

	// [v2.132.0] Commit 후 실제 DefenseData 초기화 여부입니다.
	const bool bDefenseDataReady = VehicleDefenseComp && VehicleDefenseComp->IsDefenseInitialized();

	// [v2.129.0] Legacy Fallback 상태여도 정식 방어 진입점을 제공할 컴포넌트 존재 여부입니다.
	const bool bDefenseComponentReady = VehicleDefenseComp != nullptr;

				if (CombatFxComp)
	{
		CombatFxComp->InitializeCombatFxRuntime(this, VehicleData, VehicleHealthComp);
	}

	// [v2.144.0] Sensor Foundation은 TargetSelect와 독립 초기화하며 아직 기존 CoreReady/CombatReady의 필수 조건으로 사용하지 않습니다.
	if (VehicleSensorComp)
	{
		VehicleSensorComp->InitializeSensorRuntime();
	}

						// [v2.134.0] 기본 주행·물리·내구도·피팅을 사용할 수 있는 차량 코어 Runtime 준비 상태입니다.
	bVehicleCoreRuntimeReady = bDriveReady
		&& bWheelSyncReady
		&& bHealthReady
		&& bDefenseComponentReady
		&& bFittingApplied;

	// [v2.134.0] 전투 Runtime 준비 판정에 포함할 TargetSelectComp 존재 여부입니다.
	const bool bTargetSelectReady = TargetSelectComp != nullptr;

		// [v2.143.0] 차량 코어에 Aim·Weapon·Ammo·Launcher·TargetSelect가 모두 연결된 전투 Runtime 준비 상태입니다.
	bVehicleCombatRuntimeReady = bVehicleCoreRuntimeReady
		&& bAimReady
		&& bWeaponReady
		&& bAmmoReady
		&& bLauncherReady
		&& bTargetSelectReady;

	// [v2.134.0] 기존 Tick·Debug·Blueprint 호환 값은 차량 코어 Runtime 준비 상태와 동일하게 유지합니다.
	bVehicleRuntimeReady = bVehicleCoreRuntimeReady;

		// [v2.143.0] 실제 finite Ammo Runtime, 기존 무한탄 호환 또는 초기화 실패를 구분해 표시할 탄약 런타임 상태입니다.
	const TCHAR* AmmoRuntimeState = !VehicleAmmoComp
		? TEXT("Missing")
		: (!bAmmoReady
			? TEXT("Failed")
			: (VehicleAmmoComp->IsAmmoRuntimeInitialized() ? TEXT("Ready") : TEXT("InfiniteCompatibility")));

	// [v2.129.0] 실제 DefenseData 초기화 또는 Legacy Fallback 상태를 구분해 표시할 방어 런타임 상태입니다.
	const TCHAR* DefenseRuntimeState = !bDefenseComponentReady
		? TEXT("Missing")
		: (bDefenseDataReady ? TEXT("Ready") : TEXT("LegacyFallback"));

				// [v2.132.0] VehicleFittingComp가 기록한 Legacy·Snapshot 적용 결과입니다.
	const FString FittingRuntimeSummary = VehicleFittingComp
		? VehicleFittingComp->GetLastFittingRuntimeSummary()
		: TEXT("FittingRuntime: ComponentMissing");

	// [v2.133.0] Initial Mass Prepare·실제 VehicleMesh 검증 결과입니다.
	const FString InitialMassSummary = VehicleFittingComp
		? VehicleFittingComp->GetLastInitialMassSummary()
		: TEXT("InitialMass: ComponentMissing");

			LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: Data=%s, Fitting=%s, Mass=%s, Drive=%s, WheelSync=%s, Aim=%s, Weapon=%s, Ammo=%s, Launcher=%s, Health=%s, Defense=%s, TargetSelect=%s, OwnerVisual=%s, CoreReady=%s, CombatReady=%s | %s | %s | %s | %s | %s"), VehicleData ? TEXT("Present") : TEXT("Missing"), bFittingApplied ? TEXT("Applied") : TEXT("Failed"), bInitialMassReady ? TEXT("Ready") : TEXT("Failed"), bDriveReady ? TEXT("Ready") : TEXT("Missing"), bWheelSyncReady ? TEXT("Ready") : TEXT("Missing"), bAimReady ? TEXT("Ready") : TEXT("Missing"), bWeaponReady ? TEXT("Ready") : TEXT("Missing"), AmmoRuntimeState, bLauncherReady ? TEXT("Ready") : TEXT("Missing"), bHealthReady ? TEXT("Ready") : TEXT("Missing"), DefenseRuntimeState, bTargetSelectReady ? TEXT("Ready") : TEXT("Missing"), bOwnerVisualReady ? TEXT("Ready") : TEXT("Skipped"), bVehicleCoreRuntimeReady ? TEXT("True") : TEXT("False"), bVehicleCombatRuntimeReady ? TEXT("True") : TEXT("False"), *FittingRuntimeSummary, *InitialMassSummary, *DataConfigSummary, *LayoutConfigSummary, *LastTurretVisualSummary);
	return bVehicleRuntimeReady;
}

#if WITH_EDITOR
// [v2.74.0] SM_Body 차체 메시 소켓에서 휠 앵커와 선택 하드포인트 위치를 캡처해 VehicleData에 기록합니다.
void ACFVehiclePawn::CaptureWheelLayoutFromBodySockets()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleLayoutSocketCapture: VehicleData=Missing");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LastVehicleRuntimeSummary);
		ShowVehicleLayoutEditorMessage(LastVehicleRuntimeSummary, FColor::Red);
		return;
	}

	ApplyVehicleVisualConfig();

	// [v2.69.0] 차체 메시가 적용되는 표준 StaticMeshComponent입니다.
	UStaticMeshComponent* BodyMeshComponent = FindStaticMeshComponentByName(this, TEXT("SM_Body"));
	if (!BodyMeshComponent || !BodyMeshComponent->GetStaticMesh())
	{
		LastVehicleRuntimeSummary = TEXT("VehicleLayoutSocketCapture: SM_Body.StaticMesh=Missing");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LastVehicleRuntimeSummary);
		ShowVehicleLayoutEditorMessage(LastVehicleRuntimeSummary, FColor::Red);
		return;
	}

	// [v2.69.0] 현재 DataAsset에 저장된 레이아웃 설정입니다.
	const FCFVehicleLayoutConfig& ExistingLayoutConfig = VehicleData->VehicleLayoutConfig;

	// [v2.69.0] 앞왼쪽 바퀴 중심을 읽을 차체 소켓 이름입니다.
	const FName BodySocketFL = ResolveWheelLayoutSocketName(ExistingLayoutConfig.BodyWheelSocketFL, FName(TEXT("Wheel_Anchor_FL")));

	// [v2.69.0] 앞오른쪽 바퀴 중심을 읽을 차체 소켓 이름입니다.
	const FName BodySocketFR = ResolveWheelLayoutSocketName(ExistingLayoutConfig.BodyWheelSocketFR, FName(TEXT("Wheel_Anchor_FR")));

	// [v2.69.0] 뒤왼쪽 바퀴 중심을 읽을 차체 소켓 이름입니다.
	const FName BodySocketRL = ResolveWheelLayoutSocketName(ExistingLayoutConfig.BodyWheelSocketRL, FName(TEXT("Wheel_Anchor_RL")));

	// [v2.69.0] 뒤오른쪽 바퀴 중심을 읽을 차체 소켓 이름입니다.
	const FName BodySocketRR = ResolveWheelLayoutSocketName(ExistingLayoutConfig.BodyWheelSocketRR, FName(TEXT("Wheel_Anchor_RR")));

	// [v2.69.0] 앞왼쪽 바퀴 앵커에 저장할 캡처 포즈입니다.
	FCFWheelAnchorPose CapturedWheelAnchorFL;

	// [v2.69.0] 앞오른쪽 바퀴 앵커에 저장할 캡처 포즈입니다.
	FCFWheelAnchorPose CapturedWheelAnchorFR;

	// [v2.69.0] 뒤왼쪽 바퀴 앵커에 저장할 캡처 포즈입니다.
	FCFWheelAnchorPose CapturedWheelAnchorRL;

	// [v2.69.0] 뒤오른쪽 바퀴 앵커에 저장할 캡처 포즈입니다.
	FCFWheelAnchorPose CapturedWheelAnchorRR;

	// [v2.69.0] 캡처 실패 원인을 모두 모아 표시할 요약 문자열입니다.
	FString FailureSummary;

	// [v2.69.0] 앞왼쪽 소켓 캡처 성공 여부입니다.
	const bool bCapturedFL = BuildWheelAnchorPoseFromBodySocket(this, BodyMeshComponent, BodySocketFL, FName(TEXT("Wheel_Anchor_FL")), CapturedWheelAnchorFL, FailureSummary);

	// [v2.69.0] 앞오른쪽 소켓 캡처 성공 여부입니다.
	const bool bCapturedFR = BuildWheelAnchorPoseFromBodySocket(this, BodyMeshComponent, BodySocketFR, FName(TEXT("Wheel_Anchor_FR")), CapturedWheelAnchorFR, FailureSummary);

	// [v2.69.0] 뒤왼쪽 소켓 캡처 성공 여부입니다.
	const bool bCapturedRL = BuildWheelAnchorPoseFromBodySocket(this, BodyMeshComponent, BodySocketRL, FName(TEXT("Wheel_Anchor_RL")), CapturedWheelAnchorRL, FailureSummary);

	// [v2.69.0] 뒤오른쪽 소켓 캡처 성공 여부입니다.
	const bool bCapturedRR = BuildWheelAnchorPoseFromBodySocket(this, BodyMeshComponent, BodySocketRR, FName(TEXT("Wheel_Anchor_RR")), CapturedWheelAnchorRR, FailureSummary);

	if (!bCapturedFL || !bCapturedFR || !bCapturedRL || !bCapturedRR)
	{
		LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleLayoutSocketCapture: Failed, %s"), *FailureSummary);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LastVehicleRuntimeSummary);
		ShowVehicleLayoutEditorMessage(LastVehicleRuntimeSummary, FColor::Red);
		return;
	}

	Modify();
	VehicleData->Modify();

	// [v2.69.0] 실제로 수정할 VehicleData 레이아웃 설정입니다.
	FCFVehicleLayoutConfig& MutableLayoutConfig = VehicleData->VehicleLayoutConfig;
	MutableLayoutConfig.bUseLayoutOverrides = true;
	MutableLayoutConfig.BodyWheelSocketFL = BodySocketFL;
	MutableLayoutConfig.BodyWheelSocketFR = BodySocketFR;
	MutableLayoutConfig.BodyWheelSocketRL = BodySocketRL;
	MutableLayoutConfig.BodyWheelSocketRR = BodySocketRR;
	MutableLayoutConfig.WheelAnchorFL = CapturedWheelAnchorFL;
	MutableLayoutConfig.WheelAnchorFR = CapturedWheelAnchorFR;
	MutableLayoutConfig.WheelAnchorRL = CapturedWheelAnchorRL;
	MutableLayoutConfig.WheelAnchorRR = CapturedWheelAnchorRR;

	// [v2.74.0] 하드포인트 선택 캡처 결과 카운트입니다.
	FCFHardpointCaptureStats HardpointCaptureStats;

	// [v2.74.0] 현재 선택 캡처를 시도할 하드포인트 슬롯입니다.
	for (FCFVehicleHardpointSlot& HardpointSlot : VehicleData->HardpointSlots)
	{
		CaptureHardpointSlotFromBodySocket(BodyMeshComponent, HardpointSlot, HardpointCaptureStats);
	}

	VehicleData->MarkPackageDirty();
	ApplyVehicleLayoutConfig();

	// [v2.70.0] 캡처 직후 WheelSync의 기준 위치 캐시도 새 앵커 레이아웃으로 다시 준비합니다.
	const bool bWheelSyncReadyAfterCapture = WheelSyncComp ? WheelSyncComp->TryPrepareWheelSync() : false;

	// [v2.74.0] 하드포인트 선택 캡처 중 경고가 있었는지 여부입니다.
	const bool bHasHardpointWarning = HasHardpointCaptureWarning(HardpointCaptureStats);

	// [v2.74.0] 캡처 결과 상태 이름입니다.
	const TCHAR* CaptureStateText = bHasHardpointWarning ? TEXT("AppliedWithWarning") : TEXT("Applied");

	// [v2.74.0] 하드포인트 경고 요약 표시 문자열입니다.
	const FString HardpointWarningSuffix = HardpointCaptureStats.WarningSummary.IsEmpty() ? FString() : FString::Printf(TEXT(", Warnings=%s"), *HardpointCaptureStats.WarningSummary);

	LastVehicleRuntimeSummary = FString::Printf(
		TEXT("VehicleLayoutSocketCapture: %s, SourceMesh=%s, Wheels=4/4, Sockets=%s/%s/%s/%s, WheelSync=%s, Hardpoints=Captured=%d, Skipped=%d, Missing=%d, Invalid=%d%s"),
		CaptureStateText,
		*BodyMeshComponent->GetStaticMesh()->GetName(),
		*BodySocketFL.ToString(),
		*BodySocketFR.ToString(),
		*BodySocketRL.ToString(),
		*BodySocketRR.ToString(),
		bWheelSyncReadyAfterCapture ? TEXT("Ready") : TEXT("SkippedOrFailed"),
		HardpointCaptureStats.CapturedCount,
		HardpointCaptureStats.SkippedCount,
		HardpointCaptureStats.MissingCount,
		HardpointCaptureStats.InvalidCount,
		*HardpointWarningSuffix);
	UE_LOG(LogTemp, Display, TEXT("%s"), *LastVehicleRuntimeSummary);
	ShowVehicleLayoutEditorMessage(LastVehicleRuntimeSummary, (bWheelSyncReadyAfterCapture && !bHasHardpointWarning) ? FColor::Green : FColor::Yellow);
}

// [v2.69.0] VehicleData에 저장된 휠 앵커 레이아웃을 에디터 프리뷰 Pawn에 다시 적용합니다.
void ACFVehiclePawn::ApplyVehicleLayoutFromDataInEditor()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleLayoutEditorApply: VehicleData=Missing");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LastVehicleRuntimeSummary);
		ShowVehicleLayoutEditorMessage(LastVehicleRuntimeSummary, FColor::Red);
		return;
	}

	Modify();
	ApplyVehicleVisualConfig();
	ApplyVehicleLayoutConfig();

	if (!VehicleData->VehicleLayoutConfig.bUseLayoutOverrides)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"), *LastVehicleRuntimeSummary);
		ShowVehicleLayoutEditorMessage(LastVehicleRuntimeSummary, FColor::Yellow);
		return;
	}

	// [v2.70.0] 수동 적용 버튼에서도 WheelSync 기준 위치 캐시를 새 레이아웃으로 다시 준비합니다.
	const bool bWheelSyncReadyAfterApply = WheelSyncComp ? WheelSyncComp->TryPrepareWheelSync() : false;
	LastVehicleRuntimeSummary = FString::Printf(TEXT("%s, WheelSync=%s"), *LastVehicleRuntimeSummary, bWheelSyncReadyAfterApply ? TEXT("Ready") : TEXT("SkippedOrFailed"));
	UE_LOG(LogTemp, Display, TEXT("%s"), *LastVehicleRuntimeSummary);
	ShowVehicleLayoutEditorMessage(LastVehicleRuntimeSummary, bWheelSyncReadyAfterApply ? FColor::Green : FColor::Yellow);
}
#endif

bool ACFVehiclePawn::ShouldShowAimReticle() const
{
	// [v2.20.0] Dedicated Server에서는 Viewport UI를 생성하지 않기 위한 네트워크 모드 조건입니다.
	const bool bHasViewportContext = GetNetMode() != NM_DedicatedServer;

	return bShowAimReticle && bHasViewportContext && IsLocallyControlled();
}

// [v2.148.0] 현재 Pawn이 UISubsystem Current Pawn일 때 UISubsystem 소유 Aim Reticle을 호환 반환합니다.
UCFAimReticleWidget* ACFVehiclePawn::CreateAimReticleWidget()
{
	if (AimReticleWidgetInstance)
	{
		DestroyAimReticleWidget();
	}

	// [v2.148.0] 현재 Pawn의 LocalPlayer를 확인할 소유 PlayerController입니다.
	APlayerController* OwningPlayerController = Cast<APlayerController>(GetController());
	if (!OwningPlayerController)
	{
		return nullptr;
	}

	// [v2.148.0] UISubsystem을 소유하는 현재 LocalPlayer입니다.
	ULocalPlayer* LocalPlayer = OwningPlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	// [v2.148.0] UI-P0-04 AimReticle 단일 수명을 소유하는 LocalPlayer UISubsystem입니다.
	UCFUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UCFUISubsystem>();
	if (!UISubsystem || UISubsystem->GetCurrentPawn() != this)
	{
		return nullptr;
	}

	// [v2.148.0] 현재 HUD Layer에 UISubsystem이 소유하고 있는 단일 Aim Reticle 인스턴스입니다.
	UCFAimReticleWidget* SubsystemAimReticleWidget = UISubsystem->GetAimReticleWidget();
	if (!SubsystemAimReticleWidget)
	{
		return nullptr;
	}

	SubsystemAimReticleWidget->SetVehiclePawnRef(this);
	SubsystemAimReticleWidget->SetVisibility(ShouldShowAimReticle() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	return SubsystemAimReticleWidget;
}

// [v2.148.0] UI-P0-04 이전 Pawn 직접 생성 Reticle 인스턴스만 안전하게 정리합니다.
void ACFVehiclePawn::DestroyAimReticleWidget()
{
	if (!AimReticleWidgetInstance)
	{
		return;
	}

	AimReticleWidgetInstance->SetVehiclePawnRef(nullptr);
	AimReticleWidgetInstance->RemoveFromParent();
	AimReticleWidgetInstance = nullptr;
}

// [v2.148.0] 현재 Pawn에 연결된 UISubsystem 소유 Reticle의 표시 상태를 호환 갱신합니다.
void ACFVehiclePawn::RefreshAimReticleWidget()
{
	(void)CreateAimReticleWidget();
}

bool ACFVehiclePawn::ShouldShowTargetSelectHud() const
{
	return bShowTargetSelectHud && GetNetMode() != NM_DedicatedServer && IsLocallyControlled();
}

// [v2.149.0] 현재 Pawn이 UISubsystem Current Pawn일 때 UISubsystem 소유 TargetSelect Marker를 호환 반환합니다.
UCFTargetSelectWidget* ACFVehiclePawn::CreateTargetSelectWidget()
{
	if (TargetSelectWidgetInstance)
	{
		DestroyTargetSelectWidget();
	}

	// [v2.149.0] 현재 Pawn의 LocalPlayer를 확인할 소유 PlayerController입니다.
	APlayerController* OwningPlayerController = Cast<APlayerController>(GetController());
	if (!OwningPlayerController)
	{
		return nullptr;
	}

	// [v2.149.0] UISubsystem을 소유하는 현재 LocalPlayer입니다.
	ULocalPlayer* LocalPlayer = OwningPlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return nullptr;
	}

	// [v2.149.0] UI-P0-05 TargetSelect Marker 단일 수명을 소유하는 LocalPlayer UISubsystem입니다.
	UCFUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UCFUISubsystem>();
	if (!UISubsystem || UISubsystem->GetCurrentPawn() != this)
	{
		return nullptr;
	}

	// [v2.149.0] 현재 Game Layer에 UISubsystem이 소유하고 있는 단일 TargetSelect Marker 인스턴스입니다.
	UCFTargetSelectWidget* SubsystemTargetSelectWidget = UISubsystem->GetTargetSelectWidget();
	if (!SubsystemTargetSelectWidget)
	{
		return nullptr;
	}

	SubsystemTargetSelectWidget->SetVehiclePawnRef(this);
	SubsystemTargetSelectWidget->SetVisibility(ShouldShowTargetSelectHud() ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	SubsystemTargetSelectWidget->RefreshFromTargetSelect();
	return SubsystemTargetSelectWidget;
}

void ACFVehiclePawn::DestroyTargetSelectWidget()
{
	if (!TargetSelectWidgetInstance)
	{
		return;
	}
	TargetSelectWidgetInstance->SetVehiclePawnRef(nullptr);
	TargetSelectWidgetInstance->RemoveFromParent();
	TargetSelectWidgetInstance = nullptr;
}

// [v2.149.0] 현재 Pawn에 연결된 UISubsystem 소유 Target Marker의 표시 상태를 호환 갱신합니다.
void ACFVehiclePawn::RefreshTargetSelectWidget()
{
	(void)CreateTargetSelectWidget();
}


bool ACFVehiclePawn::PrepareWheelSync()
{
	if (!WheelSyncComp)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleRuntime: WheelSyncComp is null.");
		return false;
	}
	return WheelSyncComp->TryPrepareWheelSync();
}

bool ACFVehiclePawn::UpdateVehicleWheelVisuals(float DeltaSeconds)
{
	if (!WheelSyncComp)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleRuntime: WheelSyncComp is null during UpdateVehicleWheelVisuals.");
		return false;
	}
	const bool bUpdated = WheelSyncComp->UpdateWheelVisualsPhase2(DeltaSeconds);
	if (!bUpdated)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleRuntime: Wheel visual update failed.");
	}
	else
	{
		AppendWheelSyncRuntimeSummary();
	}
	return bUpdated;
}

UChaosWheeledVehicleMovementComponent* ACFVehiclePawn::ResolveVehicleMovementComponent(const TCHAR* CacheFailureSummary, const TCHAR* MissingComponentSummary)
{
	if (!VehicleDriveComp)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleRuntime: VehicleDriveComp is null.");
		return nullptr;
	}
	if (!VehicleDriveComp->CacheVehicleMovementComponent())
	{
		LastVehicleRuntimeSummary = CacheFailureSummary;
		return nullptr;
	}
	UChaosWheeledVehicleMovementComponent* ResolvedVehicleMovementComponent = VehicleDriveComp->GetVehicleMovementComponent();
	if (!ResolvedVehicleMovementComponent)
	{
		LastVehicleRuntimeSummary = MissingComponentSummary;
		return nullptr;
	}
	return ResolvedVehicleMovementComponent;
}

// [v2.133.0] PreRegister에서 Cached Snapshot Target을 Movement Mass에 적용하거나 실패 시 Legacy 입력으로 복원합니다.
bool ACFVehiclePawn::PrepareInitialSortieRuntimeMass()
{
	if (!VehicleFittingComp)
	{
		LastVehicleRuntimeSummary = TEXT("InitialMass: VehicleFittingCompMissing");
		return false;
	}

	// [v2.133.0] PreRegister Snapshot의 활성 장착 입력으로 유지할 프로파일 ID입니다.
	const FName RequestedActiveMountProfileId = VehicleWeaponComp
		? VehicleWeaponComp->GetActiveMountProfileId()
		: NAME_None;
	if (!VehicleFittingComp->PrepareInitialSortieFitting(VehicleFittingData, VehicleData, RequestedActiveMountProfileId))
	{
		LastVehicleRuntimeSummary = VehicleFittingComp->GetLastInitialMassSummary();
		return false;
	}

	if (!VehicleFittingComp->ShouldApplyPreparedInitialMass())
	{
		LastVehicleRuntimeSummary = VehicleFittingComp->GetLastInitialMassSummary();
		return true;
	}

	// [v2.133.0] 물리 생성 전에 Target Mass를 기록할 실제 Chaos Wheeled Movement 컴포넌트입니다.
	UChaosWheeledVehicleMovementComponent* ResolvedVehicleMovementComponent = FindComponentByClass<UChaosWheeledVehicleMovementComponent>();
	if (!ResolvedVehicleMovementComponent)
	{
		const bool bFallbackPrepared = VehicleFittingComp->FallbackPreparedInitialMassToLegacy(VehicleData, RequestedActiveMountProfileId, TEXT("VehicleMovementMissingBeforePhysics"));
		LastVehicleRuntimeSummary = VehicleFittingComp->GetLastInitialMassSummary();
		return bFallbackPrepared;
	}

	// [v2.133.0] 실패 시 Super 호출 전에 복원할 기존 Chaos Movement Mass입니다.
	const float PreviousMovementMassKg = ResolvedVehicleMovementComponent->Mass;
	// [v2.133.0] Cached Snapshot에서 읽은 초기 출격 Target Mass입니다.
	const float TargetMovementMassKg = VehicleFittingComp->GetPreparedInitialMassKg();
	ResolvedVehicleMovementComponent->Mass = TargetMovementMassKg;

	if (!VehicleFittingComp->RecordInitialMassBeforePhysics(PreviousMovementMassKg, ResolvedVehicleMovementComponent->Mass))
	{
		ResolvedVehicleMovementComponent->Mass = PreviousMovementMassKg;
		const bool bFallbackPrepared = VehicleFittingComp->FallbackPreparedInitialMassToLegacy(VehicleData, RequestedActiveMountProfileId, TEXT("MovementMassRecordFailed"));
		LastVehicleRuntimeSummary = VehicleFittingComp->GetLastInitialMassSummary();
		return bFallbackPrepared;
	}

	LastVehicleRuntimeSummary = VehicleFittingComp->GetLastInitialMassSummary();
	return true;
}

// [v2.133.0] BeginPlay에서 Movement 설정 질량과 VehicleMesh 실제 질량·PhysicsAsset 계약을 검증합니다.
bool ACFVehiclePawn::VerifyInitialSortieRuntimeMass()
{
	if (!VehicleFittingComp)
	{
		return false;
	}

	if (VehicleFittingComp->UsesLegacyInitialMass())
	{
		return VehicleFittingComp->VerifyInitialMassAfterPhysics(0.0f, 0.0f, false, false, false);
	}

	// [v2.133.0] Snapshot Target과 비교할 현재 Chaos Movement 설정값입니다.
	UChaosWheeledVehicleMovementComponent* ResolvedVehicleMovementComponent = ResolveVehicleMovementComponent(TEXT("InitialMass: DriveCompCacheFailed"), TEXT("InitialMass: VehicleMovementMissing"));
	// [v2.133.0] 실제 Physics State·PhysicsAsset·Body Mass를 제공할 상속 VehicleMesh입니다.
	USkeletalMeshComponent* VehicleMeshComponent = GetMesh();
	if (!ResolvedVehicleMovementComponent || !VehicleMeshComponent)
	{
		return VehicleFittingComp->VerifyInitialMassAfterPhysics(0.0f, 0.0f, false, false, false);
	}

	// [v2.133.0] Movement Component에 현재 설정된 Chaos 차량 질량입니다.
	const float ConfiguredMovementMassKg = ResolvedVehicleMovementComponent->Mass;
	// [v2.133.0] Physics State 생성 뒤 VehicleMesh BodyInstance가 보고하는 실제 총질량입니다.
	const float ActualVehicleMeshMassKg = VehicleMeshComponent->GetMass();
	// [v2.133.0] VehicleMesh가 실제 Physics State를 생성했는지 여부입니다.
	const bool bHasVehiclePhysicsState = VehicleMeshComponent->IsPhysicsStateCreated();
	// [v2.133.0] VehicleMesh Root Body가 Chaos 물리 시뮬레이션 중인지 여부입니다.
	const bool bVehicleSimulatesPhysics = VehicleMeshComponent->IsSimulatingPhysics();
	// [v2.133.0] VehicleMesh가 실제 충돌·관성 원본 PhysicsAsset을 해석했는지 여부입니다.
	const bool bHasVehiclePhysicsAsset = VehicleMeshComponent->GetPhysicsAsset() != nullptr;

	return VehicleFittingComp->VerifyInitialMassAfterPhysics(
		ConfiguredMovementMassKg,
		ActualVehicleMeshMassKg,
		bHasVehiclePhysicsState,
		bVehicleSimulatesPhysics,
		bHasVehiclePhysicsAsset);
}

FString ACFVehiclePawn::BuildVehicleDebugTextSingleLine(const FCFVehicleDebugSnapshot& VehicleDebugSnapshot, bool bIncludeRuntimeSummary, bool bIncludeTransitionSummary, bool bIncludeInputState) const
{
	// [v2.14.1] 기존 SingleLine 의미를 유지하기 위해 구분자는 ` | `를 그대로 사용합니다.
	const FString SegmentSeparator = TEXT(" | ");

	// [v2.14.1] 기존 출력 의미를 유지할 핵심 문자열 세그먼트 목록입니다.
	TArray<FString> DebugSegments;
	DebugSegments.Reserve(16);
	DebugSegments.Add(FString::Printf(TEXT("Ready=%s"), VehicleDebugSnapshot.Runtime.bRuntimeReady ? TEXT("True") : TEXT("False")));

	if (VehicleDebugSnapshot.Runtime.bHasDriveComponent)
	{
		DebugSegments.Add(FString::Printf(TEXT("State=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Drive.CurrentDriveState)));
		DebugSegments.Add(FString::Printf(TEXT("Speed=%.1f km/h"), VehicleDebugSnapshot.Drive.SpeedKmh));
		DebugSegments.Add(FString::Printf(TEXT("ForwardSpeed=%.1f km/h"), VehicleDebugSnapshot.Drive.ForwardSpeedKmh));
		DebugSegments.Add(FString::Printf(TEXT("Throttle=%.2f"), VehicleDebugSnapshot.Drive.Throttle));
		DebugSegments.Add(FString::Printf(TEXT("Brake=%.2f"), VehicleDebugSnapshot.Drive.Brake));
		DebugSegments.Add(FString::Printf(TEXT("Steering=%.2f"), VehicleDebugSnapshot.Drive.Steering));
		DebugSegments.Add(FString::Printf(TEXT("Handbrake=%s"), VehicleDebugSnapshot.Drive.bHandbrake ? TEXT("On") : TEXT("Off")));
	}
	else
	{
		DebugSegments.Add(TEXT("State=DriveCompMissing"));
	}

	if (bIncludeInputState)
	{
		DebugSegments.Add(FString::Printf(TEXT("DeviceMode=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.DeviceMode)));
		DebugSegments.Add(FString::Printf(TEXT("InputOwner=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.InputOwner)));
		DebugSegments.Add(FString::Printf(TEXT("MoveZone=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.MoveZone)));
		DebugSegments.Add(FString::Printf(TEXT("MoveIntent=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.MoveIntent)));
		DebugSegments.Add(FString::Printf(TEXT("MoveRaw=(%.2f, %.2f)"), VehicleDebugSnapshot.Input.MoveRaw.X, VehicleDebugSnapshot.Input.MoveRaw.Y));
		DebugSegments.Add(FString::Printf(TEXT("MoveMag=%.2f"), VehicleDebugSnapshot.Input.MoveMagnitude));
		DebugSegments.Add(FString::Printf(TEXT("MoveAngle=%.1f"), VehicleDebugSnapshot.Input.MoveAngle));
		DebugSegments.Add(FString::Printf(TEXT("BlackHold=%s"), VehicleDebugSnapshot.Input.bUsedBlackZoneHold ? TEXT("True") : TEXT("False")));
	}

	if (bIncludeTransitionSummary)
	{
		if (VehicleDebugSnapshot.Runtime.bHasDriveComponent)
		{
			DebugSegments.Add(VehicleDebugSnapshot.Drive.DriveStateTransitionSummary);
		}
		else
		{
			DebugSegments.Add(TEXT("DriveStateTransition: DriveCompMissing"));
		}
	}

	if (bIncludeRuntimeSummary)
	{
		DebugSegments.Add(VehicleDebugSnapshot.Runtime.RuntimeSummary);
	}

	return FString::Join(DebugSegments, *SegmentSeparator);
}

FString ACFVehiclePawn::BuildVehicleDebugTextMultiLine(const FCFVehicleDebugSnapshot& VehicleDebugSnapshot, bool bIncludeRuntimeSummary, bool bIncludeTransitionSummary, bool bIncludeInputState) const
{
	// [v2.14.1] 기존 MultiLine 의미를 유지하기 위해 구분자만 줄바꿈으로 바꿉니다.
	const FString SegmentSeparator = TEXT("\n");

	// [v2.14.1] MultiLine도 기존 핵심 항목 순서를 유지합니다.
	TArray<FString> DebugSegments;
	DebugSegments.Reserve(16);
	DebugSegments.Add(FString::Printf(TEXT("Ready=%s"), VehicleDebugSnapshot.Runtime.bRuntimeReady ? TEXT("True") : TEXT("False")));

	if (VehicleDebugSnapshot.Runtime.bHasDriveComponent)
	{
		DebugSegments.Add(FString::Printf(TEXT("State=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Drive.CurrentDriveState)));
		DebugSegments.Add(FString::Printf(TEXT("Speed=%.1f km/h"), VehicleDebugSnapshot.Drive.SpeedKmh));
		DebugSegments.Add(FString::Printf(TEXT("ForwardSpeed=%.1f km/h"), VehicleDebugSnapshot.Drive.ForwardSpeedKmh));
		DebugSegments.Add(FString::Printf(TEXT("Throttle=%.2f"), VehicleDebugSnapshot.Drive.Throttle));
		DebugSegments.Add(FString::Printf(TEXT("Brake=%.2f"), VehicleDebugSnapshot.Drive.Brake));
		DebugSegments.Add(FString::Printf(TEXT("Steering=%.2f"), VehicleDebugSnapshot.Drive.Steering));
		DebugSegments.Add(FString::Printf(TEXT("Handbrake=%s"), VehicleDebugSnapshot.Drive.bHandbrake ? TEXT("On") : TEXT("Off")));
	}
	else
	{
		DebugSegments.Add(TEXT("State=DriveCompMissing"));
	}

	if (bIncludeInputState)
	{
		DebugSegments.Add(FString::Printf(TEXT("DeviceMode=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.DeviceMode)));
		DebugSegments.Add(FString::Printf(TEXT("InputOwner=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.InputOwner)));
		DebugSegments.Add(FString::Printf(TEXT("MoveZone=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.MoveZone)));
		DebugSegments.Add(FString::Printf(TEXT("MoveIntent=%s"), *UEnum::GetValueAsString(VehicleDebugSnapshot.Input.MoveIntent)));
		DebugSegments.Add(FString::Printf(TEXT("MoveRaw=(%.2f, %.2f)"), VehicleDebugSnapshot.Input.MoveRaw.X, VehicleDebugSnapshot.Input.MoveRaw.Y));
		DebugSegments.Add(FString::Printf(TEXT("MoveMag=%.2f"), VehicleDebugSnapshot.Input.MoveMagnitude));
		DebugSegments.Add(FString::Printf(TEXT("MoveAngle=%.1f"), VehicleDebugSnapshot.Input.MoveAngle));
		DebugSegments.Add(FString::Printf(TEXT("BlackHold=%s"), VehicleDebugSnapshot.Input.bUsedBlackZoneHold ? TEXT("True") : TEXT("False")));
	}

	if (bIncludeTransitionSummary)
	{
		if (VehicleDebugSnapshot.Runtime.bHasDriveComponent)
		{
			DebugSegments.Add(VehicleDebugSnapshot.Drive.DriveStateTransitionSummary);
		}
		else
		{
			DebugSegments.Add(TEXT("DriveStateTransition: DriveCompMissing"));
		}
	}

	if (bIncludeRuntimeSummary)
	{
		DebugSegments.Add(VehicleDebugSnapshot.Runtime.RuntimeSummary);
	}

	return FString::Join(DebugSegments, *SegmentSeparator);
}

FString ACFVehiclePawn::BuildVehicleDebugSummary(bool bUseMultilineFormat, bool bIncludeRuntimeSummary, bool bIncludeTransitionSummary, bool bIncludeInputState) const
{
	// [v2.14.1] 텍스트 출력도 동일한 Snapshot 원본을 공유하도록 먼저 현재 스냅샷을 확보합니다.
	const FCFVehicleDebugSnapshot VehicleDebugSnapshot = GetVehicleDebugSnapshot();

	return bUseMultilineFormat
		? BuildVehicleDebugTextMultiLine(VehicleDebugSnapshot, bIncludeRuntimeSummary, bIncludeTransitionSummary, bIncludeInputState)
		: BuildVehicleDebugTextSingleLine(VehicleDebugSnapshot, bIncludeRuntimeSummary, bIncludeTransitionSummary, bIncludeInputState);
}

void ACFVehiclePawn::AppendWheelSyncRuntimeSummary()
{
	const FString BaseRuntimeSummary = StripWheelSyncRuntimeSummarySuffix(LastVehicleRuntimeSummary);
	const FString WheelSyncBuildSummary = WheelSyncComp ? WheelSyncComp->LastValidationSummary : TEXT("WheelSyncBuild=MissingWheelSyncComp");
	const FString WheelSyncRuntimeSummary = WheelSyncComp ? WheelSyncComp->LastInputBuildSummary : TEXT("WheelSyncRuntime=MissingWheelSyncComp");
	LastVehicleRuntimeSummary = FString::Printf(TEXT("%s | WheelSyncBuild=%s | WheelSyncRuntime=%s"), *BaseRuntimeSummary, *WheelSyncBuildSummary, *WheelSyncRuntimeSummary);
}

void ACFVehiclePawn::ApplyAxisInputFromAction(const UInputAction* SourceInputAction, const FInputActionValue& InputActionValue, void (ACFVehiclePawn::*AxisInputSetter)(float))
{
	const float AxisValue = InputActionValue.Get<float>();
	if (!ShouldAcceptActionInput(SourceInputAction, AxisValue))
	{
		if (CurrentInputOwnership != ECFVehicleInputOwnership::VehicleMove2D)
		{
			(this->*AxisInputSetter)(0.0f);
			ReleaseInputOwnershipIfIdle();
		}
		return;
	}
	if (!CanProcessLegacyAxisInput(AxisValue))
	{
		return;
	}
	UpdateInputOwnershipFromLegacyAxis(AxisValue);
	(this->*AxisInputSetter)(AxisValue);
}


void ACFVehiclePawn::ResetAxisInput(void (ACFVehiclePawn::*AxisInputSetter)(float))
{
	(this->*AxisInputSetter)(0.0f);
}

float ACFVehiclePawn::ConvertMoveInputToAngleDeg(const FVector2D& MoveInputVector) const
{
	const float RawAngleDeg = FMath::RadiansToDegrees(FMath::Atan2(MoveInputVector.X, MoveInputVector.Y));
	return FMath::Fmod(RawAngleDeg + 360.0f, 360.0f);
}

bool ACFVehiclePawn::IsAngleWithinRange(const float InAngleDeg, const float StartAngleDeg, const float EndAngleDeg) const
{
	if (StartAngleDeg <= EndAngleDeg)
	{
		return (InAngleDeg >= StartAngleDeg) && (InAngleDeg <= EndAngleDeg);
	}

	return (InAngleDeg >= StartAngleDeg) || (InAngleDeg <= EndAngleDeg);
}

ECFVehicleMoveDirectionIntent ACFVehiclePawn::ResolveDirectionIntentFallback() const
{
	const FCFVehicleDriveStateSnapshot DriveStateSnapshot = GetDriveStateSnapshot();
	if (DriveStateSnapshot.ForwardSpeedKmh > 0.0f)
	{
		return ECFVehicleMoveDirectionIntent::Forward;
	}
	if (DriveStateSnapshot.ForwardSpeedKmh < 0.0f)
	{
		return ECFVehicleMoveDirectionIntent::Reverse;
	}
	return ECFVehicleMoveDirectionIntent::None;
}

FCFVehicleMoveInputResult ACFVehiclePawn::ResolveVehicleMoveInput(const FVector2D& MoveInputVector) const
{
	FCFVehicleMoveInputResult ResolvedMoveInput;
	ResolvedMoveInput.RawMoveInput = MoveInputVector;
	ResolvedMoveInput.Magnitude = FMath::Clamp(MoveInputVector.Length(), 0.0f, 1.0f);
	ResolvedMoveInput.SteeringValue = CalculateVehicleMoveTargetSteering(MoveInputVector, ResolvedMoveInput.Magnitude);

	if (ResolvedMoveInput.Magnitude <= KINDA_SMALL_NUMBER)
	{
		ResolvedMoveInput.ResolvedDirectionIntent = ResolveDirectionIntentFallback();
		return ResolvedMoveInput;
	}

	ResolvedMoveInput.AngleDeg = ConvertMoveInputToAngleDeg(MoveInputVector);

	const bool bInThrottleZone = IsAngleWithinRange(
		ResolvedMoveInput.AngleDeg,
		VehicleMoveInputConfig.ThrottleStartAngleDeg,
		VehicleMoveInputConfig.ThrottleEndAngleDeg);
	const bool bInReverseZone = IsAngleWithinRange(
		ResolvedMoveInput.AngleDeg,
		VehicleMoveInputConfig.ReverseStartAngleDeg,
		VehicleMoveInputConfig.ReverseEndAngleDeg);


	// [v2.6.1] 차량의 최신 Drive 상태 스냅샷을 가져옵니다.
	const FCFVehicleDriveStateSnapshot DriveStateSnapshot = GetDriveStateSnapshot();

	// [v2.6.1] 후진 전환 시 너무 엄격한 완전 정지 판정 대신,
	// DriveState가 Idle이거나 전방 기준 속도가 아주 낮아진 상태를 후진 허용 구간으로 봅니다.
	const float ReverseBrakeHoldSpeedThresholdKmh = 0.75f;

	// [v2.6.1] 아직 전방으로 의미 있는 속도가 남아 있으면 뒤 입력을 브레이크로 유지합니다.
	const bool bShouldHoldBrakeForForwardMotion =
		(DriveStateSnapshot.CurrentDriveState != ECFVehicleDriveState::Idle)
		&& (DriveStateSnapshot.ForwardSpeedKmh > ReverseBrakeHoldSpeedThresholdKmh);

	// [v2.6.1] 검은 영역 유지 시 사용할 fallback 진행 방향 의도입니다.
	const ECFVehicleMoveDirectionIntent FallbackIntent =
		(LastMoveDirectionIntent != ECFVehicleMoveDirectionIntent::None)
			? LastMoveDirectionIntent
			: ResolveDirectionIntentFallback();

	if (bInThrottleZone)
	{
		ResolvedMoveInput.ResolvedZone = ECFVehicleMoveZone::Throttle;
		ResolvedMoveInput.ResolvedDirectionIntent = ECFVehicleMoveDirectionIntent::Forward;
		ResolvedMoveInput.ThrottleValue = ResolvedMoveInput.Magnitude;
		return ResolvedMoveInput;
	}

		if (bInReverseZone)
	{
		ResolvedMoveInput.ResolvedZone = ECFVehicleMoveZone::Reverse;
		if (bShouldHoldBrakeForForwardMotion)
		{
			ResolvedMoveInput.ResolvedDirectionIntent = ECFVehicleMoveDirectionIntent::Forward;
			ResolvedMoveInput.BrakeValue = ResolvedMoveInput.Magnitude;
		}
		else
		{
			// [v2.6.3] A안: Chaos Vehicle의 bUseAutoReverse에 후진 전환을 맡기기 위해,
			// 후진 의도도 음수 스로틀 대신 브레이크 입력으로 전달합니다.
			ResolvedMoveInput.ResolvedDirectionIntent = ECFVehicleMoveDirectionIntent::Reverse;
			ResolvedMoveInput.BrakeValue = ResolvedMoveInput.Magnitude;
		}
		return ResolvedMoveInput;
	}


	ResolvedMoveInput.ResolvedZone = ECFVehicleMoveZone::Black;
	ResolvedMoveInput.ResolvedDirectionIntent = FallbackIntent;
	ResolvedMoveInput.bUsedBlackZoneHold = true;
	if (FallbackIntent == ECFVehicleMoveDirectionIntent::Forward)
	{
		ResolvedMoveInput.ThrottleValue = ResolvedMoveInput.Magnitude;
	}
	else if (FallbackIntent == ECFVehicleMoveDirectionIntent::Reverse)
	{
		// [v2.6.3] A안: 검은 영역에서도 직전 후진 의도는 브레이크 입력 유지로 전달합니다.
		ResolvedMoveInput.BrakeValue = ResolvedMoveInput.Magnitude;
	}


	return ResolvedMoveInput;
}

void ACFVehiclePawn::ApplyResolvedVehicleMoveInput(const FCFVehicleMoveInputResult& ResolvedMoveInput)
{
	// [v2.8.0] VehicleMove 조향은 목표값만 갱신하고, 실제 적용값은 Tick의 UpdateVehicleMoveSteeringInput에서 제한 속도로 추적합니다.
	TargetSteeringInput = ResolvedMoveInput.SteeringValue;
	SetVehicleBrakeInput(ResolvedMoveInput.BrakeValue);

	// [v2.6.3] A안: 수동 기어 강제를 제거하고 Chaos Vehicle의 bUseAutoReverse가
	// 브레이크 -> 후진 전환을 직접 처리하도록 둡니다.
	SetVehicleThrottleInput(ResolvedMoveInput.ThrottleValue);


	if (ResolvedMoveInput.ResolvedDirectionIntent != ECFVehicleMoveDirectionIntent::None)
	{
		LastMoveDirectionIntent = ResolvedMoveInput.ResolvedDirectionIntent;
	}
}



void ACFVehiclePawn::ApplyVehicleDataConfig()
{
	ApplyVehicleMovementConfig();
	ApplyVehicleReferenceConfig();
	ApplyVehicleWheelPhysicsConfig();
	ApplyVehicleWheelVisualConfig();
	ApplyVehicleTurretVisualConfig();
	if (VehicleDriveComp && VehicleData)
	{
		VehicleDriveComp->ApplyDriveStateConfig(VehicleData->DriveStateConfig);
	}
}

void ACFVehiclePawn::ApplyVehicleVisualConfig()
{
	if (!VehicleData)
	{
				ConfigureVehicleVisualHitCollision();
		if (TargetPointComp)
		{
			TargetPointComp->AlignToPreferredBoundsComponent();
		}
		return;
	}
	UStaticMeshComponent* ChassisStaticMeshComp = FindStaticMeshComponentByName(this, TEXT("SM_Body"));
	if (ChassisStaticMeshComp && VehicleData->VehicleVisualConfig.ChassisMesh)
	{
		ChassisStaticMeshComp->SetStaticMesh(VehicleData->VehicleVisualConfig.ChassisMesh);
	}
	// 현재 WheelSync 컴포넌트에는 휠 메쉬 자산 적용 전용 API가 없습니다.
	// 휠 시각 메쉬 교체는 별도 구현 전까지 여기서 수행하지 않습니다.

		ConfigureVehicleVisualHitCollision();
	if (TargetPointComp)
	{
		TargetPointComp->AlignToPreferredBoundsComponent();
	}
}

// [v2.108.0] VehicleMesh는 무기 채널을 무시하고 SM_Body만 시각 피격 표면으로 구성합니다.
void ACFVehiclePawn::ConfigureVehicleVisualHitCollision()
{
	TArray<UPrimitiveComponent*> PrimitiveComponents;
	GetComponents<UPrimitiveComponent>(PrimitiveComponents);

	for (UPrimitiveComponent* PrimitiveComponent : PrimitiveComponents)
	{
		if (!PrimitiveComponent)
		{
			continue;
		}

		PrimitiveComponent->SetCollisionResponseToChannel(CFCollisionChannels::WeaponHit, ECR_Ignore);
		PrimitiveComponent->SetCollisionResponseToChannel(CFCollisionChannels::Projectile, ECR_Ignore);
		PrimitiveComponent->SetCollisionResponseToChannel(CFCollisionChannels::TargetSelect, ECR_Ignore);
	}

	UStaticMeshComponent* BodyMeshComponent = FindStaticMeshComponentByName(this, TEXT("SM_Body"));
	if (!BodyMeshComponent)
	{
		UE_LOG(LogTemp, Warning, TEXT("VehicleVisualHitCollision: SM_Body component missing on %s."), *GetName());
		return;
	}

	BodyMeshComponent->SetCollisionProfileName(CFCollisionChannels::VehicleVisualHitProfileName, false);
	BodyMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BodyMeshComponent->SetCollisionObjectType(ECC_WorldDynamic);
	BodyMeshComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	BodyMeshComponent->SetCollisionResponseToChannel(CFCollisionChannels::WeaponHit, ECR_Block);
	BodyMeshComponent->SetCollisionResponseToChannel(CFCollisionChannels::Projectile, ECR_Block);
	BodyMeshComponent->SetCollisionResponseToChannel(CFCollisionChannels::TargetSelect, ECR_Block);
	BodyMeshComponent->SetGenerateOverlapEvents(false);

	if (!HasStaticMeshSimpleCollision(BodyMeshComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("VehicleVisualHitCollision: SM_Body StaticMesh has no Simple Collision. WeaponHit, Projectile or TargetSelect may not hit %s."), *GetName());
	}
}

// [v2.86.0] 터렛 시각 컴포넌트를 기본 숨김 상태로 되돌립니다.
void ACFVehiclePawn::ResetTurretVisualComponents()
{
	LastTurretMountData = nullptr;
	bLastTurretMountDataAssigned = false;
	LastTurretMountId = NAME_None;
	LastTurretMountSummary = TEXT("TurretMountData: Reset");
	bLastTurretVisualAttached = false;
	LastTurretVisualSummary = TEXT("TurretVisual: Reset");
	LastTurretBaseMeshName = NAME_None;
	LastTurretYawMeshName = NAME_None;
	LastTurretPitchMeshName = NAME_None;

	if (TurretMountRootComp)
	{
		TurretMountRootComp->SetRelativeTransform(FTransform::Identity);
		TurretMountRootComp->SetVisibility(false, true);
		TurretMountRootComp->SetHiddenInGame(true, true);
	}

	if (TurretBaseMeshComp)
	{
		TurretBaseMeshComp->AttachToComponent(TurretMountRootComp, FAttachmentTransformRules::KeepRelativeTransform);
		TurretBaseMeshComp->SetStaticMesh(nullptr);
		TurretBaseMeshComp->SetRelativeTransform(FTransform::Identity);
		TurretBaseMeshComp->SetVisibility(false, true);
		TurretBaseMeshComp->SetHiddenInGame(true, true);
	}

	if (TurretYawPivotComp)
	{
		TurretYawPivotComp->AttachToComponent(TurretMountRootComp, FAttachmentTransformRules::KeepRelativeTransform);
		TurretYawPivotComp->SetRelativeTransform(FTransform::Identity);
	}

	if (TurretYawMeshComp)
	{
		TurretYawMeshComp->AttachToComponent(TurretYawPivotComp ? TurretYawPivotComp.Get() : TurretMountRootComp.Get(), FAttachmentTransformRules::KeepRelativeTransform);
		TurretYawMeshComp->SetStaticMesh(nullptr);
		TurretYawMeshComp->SetRelativeTransform(FTransform::Identity);
		TurretYawMeshComp->SetVisibility(false, true);
		TurretYawMeshComp->SetHiddenInGame(true, true);
	}

	if (TurretPitchPivotComp)
	{
		TurretPitchPivotComp->AttachToComponent(TurretYawMeshComp ? static_cast<USceneComponent*>(TurretYawMeshComp.Get()) : TurretYawPivotComp.Get(), FAttachmentTransformRules::KeepRelativeTransform);
		TurretPitchPivotComp->SetRelativeTransform(FTransform::Identity);
	}

	if (TurretPitchMeshComp)
	{
		TurretPitchMeshComp->AttachToComponent(TurretPitchPivotComp ? TurretPitchPivotComp.Get() : TurretMountRootComp.Get(), FAttachmentTransformRules::KeepRelativeTransform);
		TurretPitchMeshComp->SetStaticMesh(nullptr);
		TurretPitchMeshComp->SetRelativeTransform(FTransform::Identity);
		TurretPitchMeshComp->SetVisibility(false, true);
		TurretPitchMeshComp->SetHiddenInGame(true, true);
	}
}

// [v2.86.0] 터렛 시각 장착에 사용할 활성 MountProfile을 찾습니다.
const FCFVehicleMountProfile* ACFVehiclePawn::FindActiveTurretMountProfile() const
{
	if (!VehicleData)
	{
		return nullptr;
	}

	// [v2.86.0] VehicleWeaponComp가 우선 사용하는 활성 장착 프로파일 ID입니다.
	const FName RequestedMountProfileId = VehicleWeaponComp ? VehicleWeaponComp->GetActiveMountProfileId() : FName(TEXT("RoofTurret_MediumOrLarge"));

	// [v2.86.0] 활성 ID와 비교할 차량 장착 프로파일입니다.
	for (const FCFVehicleMountProfile& MountProfile : VehicleData->MountProfiles)
	{
		if (MountProfile.MountProfileId == RequestedMountProfileId)
		{
			return &MountProfile;
		}
	}

	if (RequestedMountProfileId.IsNone() && !VehicleData->MountProfiles.IsEmpty())
	{
		return &VehicleData->MountProfiles[0];
	}

	return nullptr;
}

// [v2.86.0] 터렛 시각 장착에 사용할 하드포인트 슬롯을 찾습니다.
const FCFVehicleHardpointSlot* ACFVehiclePawn::FindTurretHardpointSlot(const FName LocationSlotId) const
{
	if (!VehicleData || LocationSlotId.IsNone())
	{
		return nullptr;
	}

	// [v2.86.0] 위치 슬롯 ID와 비교할 차량 하드포인트 슬롯입니다.
	for (const FCFVehicleHardpointSlot& HardpointSlot : VehicleData->HardpointSlots)
	{
		if (HardpointSlot.LocationSlotId == LocationSlotId)
		{
			return &HardpointSlot;
		}
	}

	return nullptr;
}

// [v2.86.0] VehicleData MountProfile의 터렛 시각 메쉬를 하드포인트 위치에 붙입니다.
void ACFVehiclePawn::ApplyVehicleTurretVisualConfig()
{
	ResetTurretVisualComponents();

	if (!VehicleData)
	{
		LastTurretVisualSummary = TEXT("TurretVisual: VehicleData=Missing");
		return;
	}

	if (!TurretMountRootComp || !TurretBaseMeshComp || !TurretYawPivotComp || !TurretYawMeshComp || !TurretPitchPivotComp || !TurretPitchMeshComp)
	{
		LastTurretVisualSummary = TEXT("TurretVisual: Components=Missing");
		return;
	}

	// [v2.86.0] 현재 터렛 시각 표시를 적용할 활성 장착 프로파일입니다.
	const FCFVehicleMountProfile* ActiveMountProfile = FindActiveTurretMountProfile();
	if (!ActiveMountProfile)
	{
		LastTurretVisualSummary = TEXT("TurretVisual: MountProfile=Missing");
		return;
	}

	if (ActiveMountProfile->MountType != ECFVehicleMountType::Turret)
	{
		LastTurretVisualSummary = FString::Printf(TEXT("TurretVisual: SkippedNonTurret, Profile=%s"), *ActiveMountProfile->MountProfileId.ToString());
		return;
	}

	// [v2.86.0] 활성 장착 프로파일이 참조하는 하드포인트 슬롯입니다.
	const FCFVehicleHardpointSlot* HardpointSlot = FindTurretHardpointSlot(ActiveMountProfile->LocationSlotRef);
	if (!HardpointSlot)
	{
		LastTurretVisualSummary = FString::Printf(TEXT("TurretVisual: HardpointSlot=Missing, LocationSlotRef=%s"), *ActiveMountProfile->LocationSlotRef.ToString());
		return;
	}

			// [v2.132.0] Weapon Runtime 준비 후에는 Commit된 Snapshot EquipmentPresetData를 우선합니다.
	UCFEquipmentPresetData* ResolvedEquipmentPresetData = VehicleWeaponComp
		&& VehicleWeaponComp->IsWeaponRuntimeReady()
		&& VehicleWeaponComp->GetActiveMountProfileId() == ActiveMountProfile->MountProfileId
		? VehicleWeaponComp->GetActiveEquipmentPresetData()
		: ActiveMountProfile->DefaultEquipmentPresetData.Get();

	// [v2.132.0] 최종 EquipmentPresetData에서 TurretMountData를 해석했는지 여부입니다.
	const bool bUsingEquipmentPresetTurretMountData = ResolvedEquipmentPresetData
		&& ResolvedEquipmentPresetData->DefaultTurretMountData;

	// [v2.132.0] Legacy 기본값 또는 Snapshot Override의 활성 TurretMountData입니다.
	UCFTurretMountData* ActiveTurretMountData = bUsingEquipmentPresetTurretMountData
		? ResolvedEquipmentPresetData->DefaultTurretMountData.Get()
		: nullptr;

	LastTurretMountData = ActiveTurretMountData;
	bLastTurretMountDataAssigned = (ActiveTurretMountData != nullptr);
	LastTurretMountId = ActiveTurretMountData ? ActiveTurretMountData->TurretMountId : NAME_None;
	LastTurretMountSummary = ActiveTurretMountData ? ActiveTurretMountData->BuildTurretMountSummary() : TEXT("TurretMountData: Missing, Source=EquipmentPresetDataRequired, InlineFallback=Removed");

	// [v2.100.0] 터렛 시각 값을 가져온 원본을 표시할 문자열입니다.
			const FString TurretVisualSourceText = bUsingEquipmentPresetTurretMountData
		? (VehicleWeaponComp && VehicleWeaponComp->IsUsingRuntimeEquipmentPresetOverride()
			? TEXT("FittingSnapshotEquipmentPresetData")
			: TEXT("VehicleDefaultEquipmentPresetData"))
		: TEXT("MissingEquipmentPresetTurretMountData");

	// [v2.88.0] 현재 적용할 Base 메쉬입니다.
	UStaticMesh* ResolvedTurretBaseMesh = ActiveTurretMountData ? ActiveTurretMountData->TurretBaseMesh.Get() : nullptr;

	// [v2.87.0] 현재 적용할 Yaw 메쉬입니다.
	UStaticMesh* ResolvedTurretYawMesh = ActiveTurretMountData ? ActiveTurretMountData->TurretYawMesh.Get() : nullptr;

	// [v2.87.0] 현재 적용할 Pitch 메쉬입니다.
	UStaticMesh* ResolvedTurretPitchMesh = ActiveTurretMountData ? ActiveTurretMountData->TurretPitchMesh.Get() : nullptr;

	// [v2.88.0] 현재 적용할 Base 메쉬 상대 Transform입니다.
	const FTransform ResolvedTurretBaseRelativeTransform = ActiveTurretMountData ? ActiveTurretMountData->TurretBaseRelativeTransform : FTransform::Identity;

	// [v2.88.0] 현재 적용할 Yaw 피벗 소켓 이름입니다.
	const FName ResolvedYawPivotSocketName = ActiveTurretMountData ? ActiveTurretMountData->YawPivotSocketName : NAME_None;

	// [v2.87.0] 현재 적용할 Yaw 메쉬 상대 Transform입니다.
	const FTransform ResolvedTurretYawRelativeTransform = ActiveTurretMountData ? ActiveTurretMountData->TurretYawRelativeTransform : FTransform::Identity;

	// [v2.87.0] 현재 적용할 Pitch 메쉬 상대 Transform입니다.
	const FTransform ResolvedTurretPitchRelativeTransform = ActiveTurretMountData ? ActiveTurretMountData->TurretPitchRelativeTransform : FTransform::Identity;

	// [v2.87.0] 현재 적용할 Pitch 피벗 소켓 이름입니다.
	const FName ResolvedPitchPivotSocketName = ActiveTurretMountData ? ActiveTurretMountData->PitchPivotSocketName : NAME_None;

	// [v2.93.0] Muzzle FireOrigin 전환에서 사용할 소켓 이름입니다.
	const FName ResolvedMuzzleSocketName = ActiveTurretMountData ? ActiveTurretMountData->MuzzleSocketName : NAME_None;

	// [v2.88.0] 활성 터렛 마운트 소스에 Base 메쉬가 지정되어 있는지 여부입니다.
	const bool bHasBaseMesh = ResolvedTurretBaseMesh != nullptr;

	// [v2.87.0] 활성 터렛 마운트 소스에 Yaw 메쉬가 지정되어 있는지 여부입니다.
	const bool bHasYawMesh = ResolvedTurretYawMesh != nullptr;

	// [v2.87.0] 활성 터렛 마운트 소스에 Pitch 메쉬가 지정되어 있는지 여부입니다.
	const bool bHasPitchMesh = ResolvedTurretPitchMesh != nullptr;

	// [v2.92.0] 이름이 지정됐지만 실제 메쉬에서 찾지 못한 필수 소켓 목록입니다.
	TArray<FString> MissingRequiredSocketDescriptions;

	if (!bHasBaseMesh && !bHasYawMesh && !bHasPitchMesh)
	{
		LastTurretVisualSummary = FString::Printf(
			TEXT("TurretVisual: MeshMissingOptional, Profile=%s, Slot=%s, MountData=%s, Source=%s"),
			*ActiveMountProfile->MountProfileId.ToString(),
			*HardpointSlot->LocationSlotId.ToString(),
			*LastTurretMountId.ToString(),
			*TurretVisualSourceText);
		return;
	}

	// [v2.86.0] 터렛 장착 위치의 부모가 될 차체 표시 컴포넌트입니다.
	UStaticMeshComponent* BodyMeshComponent = FindStaticMeshComponentByName(this, TEXT("SM_Body"));

	// [v2.86.0] 차체 표시 컴포넌트가 없을 때 사용할 fallback 부모 컴포넌트입니다.
	USceneComponent* MountParentComponent = BodyMeshComponent ? Cast<USceneComponent>(BodyMeshComponent) : GetRootComponent();
	if (!MountParentComponent)
	{
		LastTurretVisualSummary = TEXT("TurretVisual: MountParent=Missing");
		return;
	}

	// [v2.91.0] 하드포인트 슬롯에 저장된 차량/차체 기준 상대 Transform입니다.
	const FTransform HardpointLocalTransform(HardpointSlot->LocalRotation, HardpointSlot->LocalLocation);

	// [v2.91.0] 터렛 루트가 맞아야 하는 하드포인트 월드 위치입니다.
	FVector ExpectedHardpointWorldLocation = (HardpointLocalTransform * MountParentComponent->GetComponentTransform()).GetLocation();

	// [v2.90.0] 하드포인트 소켓을 실제 차체 소켓으로 해결했는지 여부입니다.
	bool bHardpointSocketResolved = false;

	// [v2.92.0] 하드포인트 슬롯에 소켓 이름이 명시되어 있는지 여부입니다.
	const bool bHardpointSocketNameConfigured = !HardpointSlot->SocketName.IsNone();

	if (BodyMeshComponent && !HardpointSlot->SocketName.IsNone() && BodyMeshComponent->DoesSocketExist(HardpointSlot->SocketName))
	{
		// [v2.91.0] 실제 차체 소켓의 월드 Transform입니다.
		const FTransform HardpointSocketWorldTransform = BodyMeshComponent->GetSocketTransform(HardpointSlot->SocketName, RTS_World);

		ExpectedHardpointWorldLocation = HardpointSocketWorldTransform.GetLocation();
		TurretMountRootComp->AttachToComponent(BodyMeshComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, HardpointSlot->SocketName);
		TurretMountRootComp->SetRelativeTransform(FTransform::Identity);
		bHardpointSocketResolved = true;
	}
	else
	{
		TurretMountRootComp->AttachToComponent(MountParentComponent, FAttachmentTransformRules::KeepRelativeTransform);
		TurretMountRootComp->SetRelativeTransform(HardpointLocalTransform);
	}

	if (bHardpointSocketNameConfigured && !bHardpointSocketResolved)
	{
		MissingRequiredSocketDescriptions.Add(FString::Printf(TEXT("Hardpoint:%s on SM_Body"), *HardpointSlot->SocketName.ToString()));
	}

	TurretMountRootComp->UpdateComponentToWorld();

	// [v2.91.0] 실제 터렛 루트 월드 위치입니다.
	const FVector TurretRootWorldLocation = TurretMountRootComp->GetComponentLocation();

	// [v2.91.0] 기대 하드포인트 위치와 실제 터렛 루트 위치 사이의 거리입니다.
	const float TurretRootToHardpointDistance = FVector::Dist(TurretRootWorldLocation, ExpectedHardpointWorldLocation);

	TurretMountRootComp->SetVisibility(true, true);
	TurretMountRootComp->SetHiddenInGame(false, true);

	// [v2.88.0] YawPivot이 Base 메쉬의 YawPivot 소켓에 붙었는지 여부입니다.
	bool bYawPivotSocketResolved = false;

	// [v2.92.0] Base 메쉬 아래에 Yaw 또는 Pitch 시각 메쉬를 붙여야 해서 YawPivot 소켓이 필요한지 여부입니다.
	const bool bYawPivotSocketRequired = bHasBaseMesh && (bHasYawMesh || bHasPitchMesh) && !ResolvedYawPivotSocketName.IsNone();

	if (bHasBaseMesh)
	{
		TurretBaseMeshComp->AttachToComponent(TurretMountRootComp, FAttachmentTransformRules::KeepRelativeTransform);
		TurretBaseMeshComp->SetStaticMesh(ResolvedTurretBaseMesh);
		TurretBaseMeshComp->SetRelativeTransform(ResolvedTurretBaseRelativeTransform);
		TurretBaseMeshComp->SetVisibility(true, true);
		TurretBaseMeshComp->SetHiddenInGame(false, true);
		LastTurretBaseMeshName = ResolvedTurretBaseMesh->GetFName();

		if (!ResolvedYawPivotSocketName.IsNone() && TurretBaseMeshComp->DoesSocketExist(ResolvedYawPivotSocketName))
		{
			TurretYawPivotComp->AttachToComponent(TurretBaseMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ResolvedYawPivotSocketName);
			bYawPivotSocketResolved = true;
		}
		else
		{
			TurretYawPivotComp->AttachToComponent(TurretBaseMeshComp, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
	else
	{
		TurretYawPivotComp->AttachToComponent(TurretMountRootComp, FAttachmentTransformRules::KeepRelativeTransform);
	}

	TurretYawPivotComp->SetRelativeTransform(FTransform::Identity);

	if (bYawPivotSocketRequired && !bYawPivotSocketResolved)
	{
		MissingRequiredSocketDescriptions.Add(FString::Printf(TEXT("YawPivot:%s on %s"), *ResolvedYawPivotSocketName.ToString(), *LastTurretBaseMeshName.ToString()));
	}

	if (bHasYawMesh)
	{
		TurretYawMeshComp->AttachToComponent(TurretYawPivotComp, FAttachmentTransformRules::KeepRelativeTransform);
		TurretYawMeshComp->SetStaticMesh(ResolvedTurretYawMesh);
		TurretYawMeshComp->SetRelativeTransform(ResolvedTurretYawRelativeTransform);
		TurretYawMeshComp->SetVisibility(true, true);
		TurretYawMeshComp->SetHiddenInGame(false, true);
		LastTurretYawMeshName = ResolvedTurretYawMesh->GetFName();
	}

	// [v2.86.0] Pitch 메쉬가 Yaw 메쉬의 피벗 소켓에 붙었는지 여부입니다.
	bool bPitchSocketResolved = false;

	// [v2.92.0] Yaw 메쉬 아래에 Pitch 메쉬를 붙여야 해서 PitchPivot 소켓이 필요한지 여부입니다.
	const bool bPitchPivotSocketRequired = bHasYawMesh && bHasPitchMesh && !ResolvedPitchPivotSocketName.IsNone();

	// [v2.88.0] PitchPivot을 붙일 기본 부모 컴포넌트입니다.
	USceneComponent* PitchPivotParentComponent = bHasYawMesh ? static_cast<USceneComponent*>(TurretYawMeshComp.Get()) : TurretYawPivotComp.Get();

	if (bHasYawMesh && !ResolvedPitchPivotSocketName.IsNone() && TurretYawMeshComp->DoesSocketExist(ResolvedPitchPivotSocketName))
	{
		TurretPitchPivotComp->AttachToComponent(TurretYawMeshComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale, ResolvedPitchPivotSocketName);
		bPitchSocketResolved = true;
	}
	else
	{
		TurretPitchPivotComp->AttachToComponent(PitchPivotParentComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}

	TurretPitchPivotComp->SetRelativeTransform(FTransform::Identity);

	if (bPitchPivotSocketRequired && !bPitchSocketResolved)
	{
		MissingRequiredSocketDescriptions.Add(FString::Printf(TEXT("PitchPivot:%s on %s"), *ResolvedPitchPivotSocketName.ToString(), *LastTurretYawMeshName.ToString()));
	}

	if (bHasPitchMesh)
	{
		TurretPitchMeshComp->AttachToComponent(TurretPitchPivotComp, FAttachmentTransformRules::KeepRelativeTransform);
		TurretPitchMeshComp->SetStaticMesh(ResolvedTurretPitchMesh);
		TurretPitchMeshComp->SetRelativeTransform(ResolvedTurretPitchRelativeTransform);
		TurretPitchMeshComp->SetVisibility(true, true);
		TurretPitchMeshComp->SetHiddenInGame(false, true);
		LastTurretPitchMeshName = ResolvedTurretPitchMesh->GetFName();
	}

	// [v2.93.0] Pitch 메쉬에 Muzzle 소켓이 실제로 존재하는지 여부입니다.
	bool bMuzzleSocketResolved = false;

	// [v2.93.0] Muzzle FireOrigin 전환을 위해 Muzzle 소켓을 필수로 볼 수 있는지 여부입니다.
	const bool bMuzzleSocketRequired = bHasPitchMesh && !ResolvedMuzzleSocketName.IsNone();

	if (bMuzzleSocketRequired && TurretPitchMeshComp->DoesSocketExist(ResolvedMuzzleSocketName))
	{
		bMuzzleSocketResolved = true;
	}

	if (bMuzzleSocketRequired && !bMuzzleSocketResolved)
	{
		MissingRequiredSocketDescriptions.Add(FString::Printf(TEXT("Muzzle:%s on %s"), *ResolvedMuzzleSocketName.ToString(), *LastTurretPitchMeshName.ToString()));
	}

	// [v2.92.0] 하드포인트 소켓의 최종 부착 상태를 디버그에 표시할 문자열입니다.
	FString HardpointSocketStatusText = TEXT("MissingRequiredSocketFallback");
	if (HardpointSlot->SocketName.IsNone())
	{
		HardpointSocketStatusText = TEXT("SkippedNoSocketName");
	}
	else if (bHardpointSocketResolved)
	{
		HardpointSocketStatusText = TEXT("Resolved");
	}

	// [v2.92.0] YawPivot 소켓의 최종 부착 상태를 디버그에 표시할 문자열입니다.
	FString YawPivotStatusText = TEXT("MissingRequiredSocketFallback");
	if (!bHasBaseMesh)
	{
		YawPivotStatusText = TEXT("FallbackNoBaseMesh");
	}
	else if (!(bHasYawMesh || bHasPitchMesh))
	{
		YawPivotStatusText = TEXT("SkippedNoChildMesh");
	}
	else if (ResolvedYawPivotSocketName.IsNone())
	{
		YawPivotStatusText = TEXT("FallbackNoSocketName");
	}
	else if (bYawPivotSocketResolved)
	{
		YawPivotStatusText = TEXT("Resolved");
	}

	// [v2.92.0] PitchPivot 소켓의 최종 부착 상태를 디버그에 표시할 문자열입니다.
	FString PitchPivotStatusText = TEXT("MissingRequiredSocketFallback");
	if (!bHasPitchMesh)
	{
		PitchPivotStatusText = TEXT("SkippedNoPitchMesh");
	}
	else if (!bHasYawMesh)
	{
		PitchPivotStatusText = TEXT("FallbackNoYawMesh");
	}
	else if (ResolvedPitchPivotSocketName.IsNone())
	{
		PitchPivotStatusText = TEXT("FallbackNoSocketName");
	}
	else if (bPitchSocketResolved)
	{
		PitchPivotStatusText = TEXT("Resolved");
	}

	// [v2.93.0] Muzzle 소켓의 최종 FireOrigin 전환 상태를 디버그에 표시할 문자열입니다.
	FString MuzzleSocketStatusText = TEXT("MissingRequiredSocketFallback");
	if (!bHasPitchMesh)
	{
		MuzzleSocketStatusText = TEXT("FallbackNoPitchMesh");
	}
	else if (ResolvedMuzzleSocketName.IsNone())
	{
		MuzzleSocketStatusText = TEXT("FallbackNoSocketName");
	}
	else if (bMuzzleSocketResolved)
	{
		MuzzleSocketStatusText = TEXT("Resolved");
	}

	// [v2.92.0] 필수 소켓 검증의 전체 상태를 디버그에 표시할 문자열입니다.
	const FString SocketValidationStatusText = MissingRequiredSocketDescriptions.IsEmpty() ? TEXT("OK") : TEXT("MissingRequiredSocket");

	// [v2.92.0] 누락된 필수 소켓 목록을 한 줄 요약으로 묶은 문자열입니다.
	const FString MissingRequiredSocketSummary = MissingRequiredSocketDescriptions.IsEmpty() ? TEXT("None") : FString::Join(MissingRequiredSocketDescriptions, TEXT(" | "));

	if (!MissingRequiredSocketDescriptions.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("TurretVisual missing required socket(s): %s"), *MissingRequiredSocketSummary);
	}

	bLastTurretVisualAttached = true;
	LastTurretVisualSummary = FString::Printf(
		TEXT("TurretVisual: Attached, Profile=%s, Slot=%s, HardpointSocket=%s, HardpointSocketResolved=%s, HardpointSocketStatus=%s, SocketValidation=%s, MissingRequiredSockets=%s, RootWorld=(%.1f, %.1f, %.1f), ExpectedWorld=(%.1f, %.1f, %.1f), RootDelta=%.2f, Parent=%s, MountData=%s, Source=%s, BaseMesh=%s, YawMesh=%s, PitchMesh=%s, YawPivot=%s, PitchPivot=%s, MuzzleSocket=%s, MuzzleStatus=%s"),
		*ActiveMountProfile->MountProfileId.ToString(),
		*HardpointSlot->LocationSlotId.ToString(),
		*HardpointSlot->SocketName.ToString(),
		bHardpointSocketResolved ? TEXT("Yes") : TEXT("No"),
		*HardpointSocketStatusText,
		*SocketValidationStatusText,
		*MissingRequiredSocketSummary,
		TurretRootWorldLocation.X,
		TurretRootWorldLocation.Y,
		TurretRootWorldLocation.Z,
		ExpectedHardpointWorldLocation.X,
		ExpectedHardpointWorldLocation.Y,
		ExpectedHardpointWorldLocation.Z,
		TurretRootToHardpointDistance,
		*MountParentComponent->GetName(),
		*LastTurretMountId.ToString(),
		*TurretVisualSourceText,
		*LastTurretBaseMeshName.ToString(),
		*LastTurretYawMeshName.ToString(),
		*LastTurretPitchMeshName.ToString(),
		*YawPivotStatusText,
		*PitchPivotStatusText,
		*ResolvedMuzzleSocketName.ToString(),
		*MuzzleSocketStatusText);
}

// [v2.110.0] 최종 발사 방향과 분리된 요구 조준 방향으로 터렛이 바라볼 월드 방향을 계산합니다.
FVector ACFVehiclePawn::ResolveTurretAimWorldDirection() const
{
	// [v2.126.0] Ripple·Salvo 진행 중에는 사용자가 새로 이동한 Reticle이 아니라 첫 입력 순간 고정 Command Target을 터렛이 계속 추적합니다.
	FVector ActiveLauncherCommandTargetLocation;
	if (LauncherComp
		&& LauncherComp->TryGetActiveCommandTargetLocation(ActiveLauncherCommandTargetLocation)
		&& TurretYawPivotComp)
	{
		const FVector DirectionToLauncherCommandTarget = ActiveLauncherCommandTargetLocation - TurretYawPivotComp->GetComponentLocation();
		if (!DirectionToLauncherCommandTarget.ContainsNaN() && !DirectionToLauncherCommandTarget.IsNearlyZero())
		{
			return DirectionToLauncherCommandTarget.GetSafeNormal();
		}
	}

	if (VehicleAimComp)
	{
		// [v2.109.0] AimComp에 저장된 최신 Weapon Aim Solution입니다.
		const FCFVehicleWeaponAimSolution WeaponAimSolution = VehicleAimComp->GetWeaponAimSolution();
		if (WeaponAimSolution.bHasValidSolution && !WeaponAimSolution.DesiredAimDirection.IsNearlyZero())
		{
			return WeaponAimSolution.DesiredAimDirection.GetSafeNormal();
		}

		// [v2.89.0] AimComp가 보유한 현재 로컬 조준 상태입니다.
		const FCFVehicleLocalAimState LocalAimState = VehicleAimComp->GetLocalAimState();

		if (TurretYawPivotComp && !LocalAimState.LocalAimTargetLocation.IsNearlyZero())
		{
			// [v2.89.0] 터렛 Yaw 피벗 위치에서 조준 목표까지의 월드 방향입니다.
			const FVector DirectionToAimTarget = LocalAimState.LocalAimTargetLocation - TurretYawPivotComp->GetComponentLocation();
			if (!DirectionToAimTarget.IsNearlyZero())
			{
				return DirectionToAimTarget.GetSafeNormal();
			}
		}

		if (!LocalAimState.LocalAimDirection.IsNearlyZero())
		{
			return LocalAimState.LocalAimDirection.GetSafeNormal();
		}
	}

	if (VehicleCameraComp)
	{
		// [v2.89.0] 카메라 컴포넌트가 직접 제공하는 현재 조준 방향입니다.
		const FVector CameraAimDirection = VehicleCameraComp->GetCurrentAimDirection();
		if (!CameraAimDirection.IsNearlyZero())
		{
			return CameraAimDirection.GetSafeNormal();
		}
	}

	// [v2.89.0] AimComp와 CameraComp가 유효하지 않을 때 사용할 Actor 정면 방향입니다.
	const FVector ActorForwardDirection = GetActorForwardVector().GetSafeNormal();
	if (!ActorForwardDirection.IsNearlyZero())
	{
		return ActorForwardDirection;
	}

	return FVector::ForwardVector;
}

// [v2.89.0] WeaponComp가 계산한 터렛 Yaw / Pitch 각도를 시각 피벗 컴포넌트에 적용합니다.
void ACFVehiclePawn::UpdateVehicleTurretAimVisuals(const float DeltaSeconds)
{
	if (!VehicleWeaponComp)
	{
		if (VehicleAimComp)
		{
			VehicleAimComp->SetWeaponAimSolution(FCFVehicleWeaponAimSolution());
		}
		return;
	}

	if (!TurretMountRootComp || !TurretYawPivotComp || !TurretPitchPivotComp)
	{
		VehicleWeaponComp->ResetTurretState();
		if (VehicleAimComp)
		{
			VehicleAimComp->SetWeaponAimSolution(FCFVehicleWeaponAimSolution());
		}
		return;
	}

	if (!bLastTurretVisualAttached)
	{
		VehicleWeaponComp->UpdateTurretState(DeltaSeconds, GetActorForwardVector(), FTransform::Identity, false);
		TurretYawPivotComp->SetRelativeRotation(FRotator::ZeroRotator);
		TurretPitchPivotComp->SetRelativeRotation(FRotator::ZeroRotator);
		if (VehicleAimComp)
		{
			VehicleAimComp->SetWeaponAimSolution(FCFVehicleWeaponAimSolution());
		}
		return;
	}

	// [v2.89.0] 터렛 조준 계산에 사용할 월드 방향입니다.
	const FVector TurretAimWorldDirection = ResolveTurretAimWorldDirection();

	// [v2.89.0] 터렛 로컬 각도 계산의 기준이 되는 터렛 장착 루트 Transform입니다.
	const FTransform TurretReferenceTransform = TurretMountRootComp->GetComponentTransform();

	// [v2.89.0] WeaponComp가 터렛 상태 갱신에 성공했는지 여부입니다.
	const bool bTurretStateUpdated = VehicleWeaponComp->UpdateTurretState(DeltaSeconds, TurretAimWorldDirection, TurretReferenceTransform, bLastTurretVisualAttached);
	if (!bTurretStateUpdated)
	{
		return;
	}

	// [v2.89.0] 시각 피벗에 적용할 최신 터렛 조준 추적 상태입니다.
	const FCFVehicleTurretState TurretState = VehicleWeaponComp->GetTurretState();

	// [v2.89.0] Yaw 피벗에 적용할 상대 회전입니다.
	const FRotator YawPivotRelativeRotation(0.0f, TurretState.CurrentYawDeg, 0.0f);

	// [v2.89.0] Pitch 피벗에 적용할 상대 회전입니다.
	const FRotator PitchPivotRelativeRotation(TurretState.CurrentPitchDeg, 0.0f, 0.0f);

	TurretYawPivotComp->SetRelativeRotation(YawPivotRelativeRotation);
	TurretPitchPivotComp->SetRelativeRotation(PitchPivotRelativeRotation);

	RefreshWeaponAimSolution();
}

// [v2.68.0] VehicleData의 바퀴 앵커 레이아웃 오버라이드를 BP Wheel_Anchor_* 컴포넌트에 적용합니다.
void ACFVehiclePawn::ApplyVehicleLayoutConfig()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleLayout: VehicleData=Missing, ManualAnchorLayout=Fallback");
		return;
	}

	// [v2.68.0] VehicleData에서 읽은 차량 레이아웃 설정입니다.
	const FCFVehicleLayoutConfig& VehicleLayoutConfig = VehicleData->VehicleLayoutConfig;
	if (!VehicleLayoutConfig.bUseLayoutOverrides)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleLayout: ManualAnchorLayout=Fallback");
		return;
	}

	// [v2.68.0] 레이아웃을 적용하지 못한 Wheel_Anchor_* 컴포넌트 이름 목록입니다.
	FString MissingWheelAnchorNames;

	// [v2.68.0] DataAsset 레이아웃이 적용된 바퀴 앵커 개수입니다.
	int32 AppliedWheelAnchorCount = 0;

	// [v2.68.0] 단일 WheelAnchor 포즈를 같은 이름의 SceneComponent에 적용하는 로컬 함수입니다.
	const auto ApplyWheelAnchorPose = [this, &MissingWheelAnchorNames, &AppliedWheelAnchorCount](const FName WheelAnchorName, const FCFWheelAnchorPose& WheelAnchorPose)
	{
		// [v2.68.0] 이름으로 찾은 바퀴 앵커 SceneComponent입니다.
		USceneComponent* WheelAnchorComponent = FindSceneComponentByName(this, WheelAnchorName);
		if (!WheelAnchorComponent)
		{
			if (!MissingWheelAnchorNames.IsEmpty())
			{
				MissingWheelAnchorNames += TEXT(", ");
			}
			MissingWheelAnchorNames += WheelAnchorName.ToString();
			return;
		}

		#if WITH_EDITOR
		if (GIsEditor)
		{
			WheelAnchorComponent->Modify();
		}
		#endif

		WheelAnchorComponent->SetRelativeLocationAndRotation(WheelAnchorPose.RelativeLocation, WheelAnchorPose.RelativeRotation, false, nullptr, ETeleportType::TeleportPhysics);
		WheelAnchorComponent->UpdateComponentToWorld();
		++AppliedWheelAnchorCount;
	};

	ApplyWheelAnchorPose(TEXT("Wheel_Anchor_FL"), VehicleLayoutConfig.WheelAnchorFL);
	ApplyWheelAnchorPose(TEXT("Wheel_Anchor_FR"), VehicleLayoutConfig.WheelAnchorFR);
	ApplyWheelAnchorPose(TEXT("Wheel_Anchor_RL"), VehicleLayoutConfig.WheelAnchorRL);
	ApplyWheelAnchorPose(TEXT("Wheel_Anchor_RR"), VehicleLayoutConfig.WheelAnchorRR);

	if (!MissingWheelAnchorNames.IsEmpty())
	{
		LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleLayout: LayoutOverride=Partial, Applied=%d, Missing=%s"), AppliedWheelAnchorCount, *MissingWheelAnchorNames);
		return;
	}

	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleLayout: LayoutOverride=Applied, Applied=%d"), AppliedWheelAnchorCount);
}

void ACFVehiclePawn::ApplyVehicleMovementConfig()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleRuntime: VehicleData is null during ApplyVehicleMovementConfig.");
		return;
	}
	UChaosWheeledVehicleMovementComponent* ResolvedVehicleMovementComponent = ResolveVehicleMovementComponent(TEXT("VehicleRuntime: DriveComp cache failed during ApplyVehicleMovementConfig."), TEXT("VehicleRuntime: VehicleMovementComponent is null during ApplyVehicleMovementConfig."));
	if (!ResolvedVehicleMovementComponent)
	{
		return;
	}
	const FCFVehicleMovementConfig& VehicleMovementConfig = VehicleData->VehicleMovementConfig;
	ResolvedVehicleMovementComponent->ChassisHeight = VehicleMovementConfig.ChassisHeight;
	ResolvedVehicleMovementComponent->DragCoefficient = VehicleMovementConfig.DragCoefficient;
	ResolvedVehicleMovementComponent->DownforceCoefficient = VehicleMovementConfig.DownforceCoefficient;
	ResolvedVehicleMovementComponent->bEnableCenterOfMassOverride = VehicleMovementConfig.bEnableCenterOfMassOverride;
	ResolvedVehicleMovementComponent->CenterOfMassOverride = VehicleMovementConfig.CenterOfMassOverride;
	ResolvedVehicleMovementComponent->EngineSetup.MaxTorque = VehicleMovementConfig.EngineMaxTorque;
	ResolvedVehicleMovementComponent->EngineSetup.MaxRPM = VehicleMovementConfig.EngineMaxRPM;
	ResolvedVehicleMovementComponent->EngineSetup.EngineIdleRPM = VehicleMovementConfig.EngineIdleRPM;
	ResolvedVehicleMovementComponent->EngineSetup.EngineBrakeEffect = VehicleMovementConfig.EngineBrakeEffect;
	ResolvedVehicleMovementComponent->EngineSetup.EngineRevUpMOI = VehicleMovementConfig.EngineRevUpMOI;
	ResolvedVehicleMovementComponent->EngineSetup.EngineRevDownRate = VehicleMovementConfig.EngineRevDownRate;
	ResolvedVehicleMovementComponent->DifferentialSetup.DifferentialType = VehicleMovementConfig.DifferentialType;
	ResolvedVehicleMovementComponent->DifferentialSetup.FrontRearSplit = VehicleMovementConfig.FrontRearSplit;
	ResolvedVehicleMovementComponent->SteeringSetup.SteeringType = VehicleMovementConfig.SteeringType;
	ResolvedVehicleMovementComponent->SteeringSetup.AngleRatio = VehicleMovementConfig.SteeringAngleRatio;
	ResolvedVehicleMovementComponent->bLegacyWheelFrictionPosition = VehicleMovementConfig.bLegacyWheelFrictionPosition;

	ResolvedVehicleMovementComponent->SetMaxEngineTorque(VehicleMovementConfig.EngineMaxTorque);
	ResolvedVehicleMovementComponent->SetDragCoefficient(VehicleMovementConfig.DragCoefficient);
	ResolvedVehicleMovementComponent->SetDownforceCoefficient(VehicleMovementConfig.DownforceCoefficient);
	ResolvedVehicleMovementComponent->SetDifferentialFrontRearSplit(VehicleMovementConfig.FrontRearSplit);

	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: MovementProfile=%s, RuntimeTorque=%.1f, ConfigMaxRPM=%.1f, ThrottleScale=%.2f, Drag=%.2f, Downforce=%.2f, Differential=%s, SteeringType=%s, RuntimeSetters=EngineTorque/Drag/Downforce/DiffSplit"), *VehicleMovementConfig.MovementProfileName.ToString(), VehicleMovementConfig.EngineMaxTorque, VehicleMovementConfig.EngineMaxRPM, VehicleMovementConfig.ThrottleInputScale, VehicleMovementConfig.DragCoefficient, VehicleMovementConfig.DownforceCoefficient, *UEnum::GetValueAsString(VehicleMovementConfig.DifferentialType), *UEnum::GetValueAsString(VehicleMovementConfig.SteeringType));
}

void ACFVehiclePawn::ApplyVehicleWheelPhysicsConfig()
{
	if (!VehicleData)
	{
		LastVehicleRuntimeSummary = TEXT("VehicleRuntime: VehicleData is null during ApplyVehicleWheelPhysicsConfig.");
		return;
	}

	// [v2.56.1] 실제 Chaos Vehicle Movement 컴포넌트입니다.
	UChaosWheeledVehicleMovementComponent* ResolvedVehicleMovementComponent = ResolveVehicleMovementComponent(TEXT("VehicleRuntime: DriveComp cache failed during ApplyVehicleWheelPhysicsConfig."), TEXT("VehicleRuntime: VehicleMovementComponent is null during ApplyVehicleWheelPhysicsConfig."));
	if (!ResolvedVehicleMovementComponent)
	{
		return;
	}

	// [v2.56.1] VehicleData에서 읽은 차량 물리 설정입니다.
	const FCFVehicleMovementConfig& VehicleMovementConfig = VehicleData->VehicleMovementConfig;

	// [v2.56.1] VehicleData에서 읽은 차량 참조 설정입니다.
	const FCFVehicleReferenceConfig& VehicleReferenceConfig = VehicleData->VehicleReferenceConfig;

	// [v2.56.1] 런타임 휠 물리 덮어쓰기를 사용할지 여부입니다.
	const bool bUseRuntimeWheelPhysicsOverrides = VehicleMovementConfig.bUseMovementOverrides;

	// [v2.72.0] 실제 런타임 휠 인스턴스까지 갱신 요청한 개수입니다.
	int32 RuntimeWheelApplyCount = 0;

	// [v2.56.1] 휠 setup에 클래스/오프셋을 적용합니다.
	const auto ConfigureWheelSetup = [&](FChaosWheelSetup& WheelSetup, const int32 WheelIndex, const TSubclassOf<UChaosVehicleWheel> WheelClass, const bool bIsFrontWheel)
	{
		WheelSetup.WheelClass = WheelClass;
		WheelSetup.AdditionalOffset = bIsFrontWheel ? VehicleMovementConfig.FrontWheelAdditionalOffset : VehicleMovementConfig.RearWheelAdditionalOffset;
		if (!WheelClass)
		{
			return;
		}

		// [v2.56.1] 휠 클래스의 기본 오브젝트입니다.
		UChaosVehicleWheel* WheelClassDefaultObject = WheelClass->GetDefaultObject<UChaosVehicleWheel>();
		if (!WheelClassDefaultObject)
		{
			return;
		}

		if (!bUseRuntimeWheelPhysicsOverrides)
		{
			return;
		}

		// [v2.56.1] 클래스 기본값 임시 변경 전 복구용 스냅샷입니다.
		const FCFWheelClassRuntimeSnapshot WheelClassRuntimeSnapshot = CaptureWheelClassRuntimeSnapshot(*WheelClassDefaultObject);

		ApplyVehicleMovementWheelTuningToWheelClass(*WheelClassDefaultObject, VehicleMovementConfig, bIsFrontWheel);
		WheelSetup.WheelClass = WheelClass;

		if (ResolvedVehicleMovementComponent->Wheels.IsValidIndex(WheelIndex))
		{
			ApplyVehicleMovementWheelTuningToRuntime(*ResolvedVehicleMovementComponent, VehicleMovementConfig, WheelIndex, bIsFrontWheel);
			++RuntimeWheelApplyCount;
		}

		RestoreWheelClassRuntimeSnapshot(*WheelClassDefaultObject, WheelClassRuntimeSnapshot);
	};

	for (int32 WheelIndex = 0; WheelIndex < ResolvedVehicleMovementComponent->WheelSetups.Num(); ++WheelIndex)
	{
		// [v2.56.1] 현재 순회 중인 Chaos 휠 setup입니다.
		FChaosWheelSetup& WheelSetup = ResolvedVehicleMovementComponent->WheelSetups[WheelIndex];

		// [v2.56.1] 휠 본 이름 문자열입니다.
		const FString BoneNameString = WheelSetup.BoneName.ToString();

		// [v2.56.1] 현재 휠을 앞바퀴로 볼지 여부입니다.
		const bool bIsFrontWheel = BoneNameString.Contains(TEXT("F"));

		// [v2.56.1] 현재 휠에 적용할 휠 클래스입니다.
		const TSubclassOf<UChaosVehicleWheel> WheelClass = bIsFrontWheel
			? VehicleReferenceConfig.FrontWheelClass
			: VehicleReferenceConfig.RearWheelClass;

		ConfigureWheelSetup(WheelSetup, WheelIndex, WheelClass, bIsFrontWheel);
	}

	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: WheelPhysicsOverrides=%s, RuntimeWheelApply=%d/%d, FrontWheelClass=%s, RearWheelClass=%s, FrontSteer=%.1f, FrictionF/R=%.2f/%.2f, FrontOffset=%s, RearOffset=%s"),
		bUseRuntimeWheelPhysicsOverrides ? TEXT("True") : TEXT("False"),
		RuntimeWheelApplyCount,
		ResolvedVehicleMovementComponent->WheelSetups.Num(),
		VehicleReferenceConfig.FrontWheelClass ? *VehicleReferenceConfig.FrontWheelClass->GetName() : TEXT("None"),
		VehicleReferenceConfig.RearWheelClass ? *VehicleReferenceConfig.RearWheelClass->GetName() : TEXT("None"),
		VehicleMovementConfig.FrontWheelMaxSteerAngle,
		VehicleMovementConfig.FrontWheelFrictionForceMultiplier,
		VehicleMovementConfig.RearWheelFrictionForceMultiplier,
		*VehicleMovementConfig.FrontWheelAdditionalOffset.ToCompactString(),
		*VehicleMovementConfig.RearWheelAdditionalOffset.ToCompactString());
}

// [v2.5.2] VehicleData의 휠 메시 자산을 기존 Wheel_Mesh_* 컴포넌트에 적용하고 WheelSync 기본 시각 설정을 함께 갱신합니다.
void ACFVehiclePawn::ApplyVehicleWheelVisualConfig()
{
	if (!WheelSyncComp || !VehicleData)
	{
		return;
	}

	// [v2.102.0] VehicleData 기준 휠 시각 설정입니다.
	const FCFVehicleWheelVisualConfig& WheelVisualConfig = VehicleData->WheelVisualConfig;

	// [v2.102.0] VehicleData 기준 휠 메시 참조 설정입니다.
	const FCFVehicleVisualConfig& VehicleVisualConfig = VehicleData->VehicleVisualConfig;

	// [v2.102.0] 휠 메시 자동 스케일 목표 반지름을 제공하는 이동 설정입니다.
	const FCFVehicleMovementConfig& VehicleMovementConfig = VehicleData->VehicleMovementConfig;

	// VehicleData 기준 WheelSync 기본 설정값을 반영합니다.
	WheelSyncComp->ExpectedWheelCount = WheelVisualConfig.ExpectedWheelCount;
	WheelSyncComp->FrontWheelCountForSteering = WheelVisualConfig.FrontWheelCountForSteering;

	// [v2.102.0] 런타임 요약에 남길 휠 메시 자동 스케일 적용 결과입니다.
	FString WheelMeshAutoScaleSummary = WheelVisualConfig.bAutoScaleWheelMeshToRadius ? TEXT("AutoScale=On") : TEXT("AutoScale=Off");

	// 앞왼쪽 휠 메시 컴포넌트에 VehicleData의 FL 휠 메시를 적용합니다.
	if (UStaticMeshComponent* WheelMeshFLComp = FindStaticMeshComponentByName(this, TEXT("Wheel_Mesh_FL")))
	{
		ApplyWheelMeshVisualConfigToComponent(WheelMeshFLComp, VehicleVisualConfig.WheelMeshFL, VehicleMovementConfig.FrontWheelRadius, WheelVisualConfig, WheelMeshAutoScaleSummary);
	}

	// 앞오른쪽 휠 메시 컴포넌트에 VehicleData의 FR 휠 메시를 적용합니다.
	if (UStaticMeshComponent* WheelMeshFRComp = FindStaticMeshComponentByName(this, TEXT("Wheel_Mesh_FR")))
	{
		ApplyWheelMeshVisualConfigToComponent(WheelMeshFRComp, VehicleVisualConfig.WheelMeshFR, VehicleMovementConfig.FrontWheelRadius, WheelVisualConfig, WheelMeshAutoScaleSummary);
	}

	// 뒤왼쪽 휠 메시 컴포넌트에 VehicleData의 RL 휠 메시를 적용합니다.
	if (UStaticMeshComponent* WheelMeshRLComp = FindStaticMeshComponentByName(this, TEXT("Wheel_Mesh_RL")))
	{
		ApplyWheelMeshVisualConfigToComponent(WheelMeshRLComp, VehicleVisualConfig.WheelMeshRL, VehicleMovementConfig.RearWheelRadius, WheelVisualConfig, WheelMeshAutoScaleSummary);
	}

	// 뒤오른쪽 휠 메시 컴포넌트에 VehicleData의 RR 휠 메시를 적용합니다.
	if (UStaticMeshComponent* WheelMeshRRComp = FindStaticMeshComponentByName(this, TEXT("Wheel_Mesh_RR")))
	{
		ApplyWheelMeshVisualConfigToComponent(WheelMeshRRComp, VehicleVisualConfig.WheelMeshRR, VehicleMovementConfig.RearWheelRadius, WheelVisualConfig, WheelMeshAutoScaleSummary);
	}

	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: WheelVisual ExpectedWheelCount=%d, FrontWheelCount=%d, %s"), WheelSyncComp->ExpectedWheelCount, WheelSyncComp->FrontWheelCountForSteering, *WheelMeshAutoScaleSummary);
}

void ACFVehiclePawn::ApplyVehicleReferenceConfig()
{
	if (!VehicleData)
	{
		return;
	}
	const FCFVehicleReferenceConfig& VehicleReferenceConfig = VehicleData->VehicleReferenceConfig;
	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: FrontWheelClass=%s, RearWheelClass=%s"), VehicleReferenceConfig.FrontWheelClass ? *VehicleReferenceConfig.FrontWheelClass->GetName() : TEXT("None"), VehicleReferenceConfig.RearWheelClass ? *VehicleReferenceConfig.RearWheelClass->GetName() : TEXT("None"));
}

void ACFVehiclePawn::SetVehicleThrottleInput(const float InThrottleValue)
{
	if (VehicleDriveComp)
	{
		// [v2.73.0] VehicleData에서 읽은 스로틀 입력 배율입니다.
		const float ThrottleInputScale = (VehicleData && VehicleData->VehicleMovementConfig.bUseMovementOverrides)
			? FMath::Clamp(VehicleData->VehicleMovementConfig.ThrottleInputScale, 0.0f, 1.0f)
			: 1.0f;

		// [v2.73.0] 최종적으로 DriveComp에 전달할 보정 스로틀 입력입니다.
		const float ScaledThrottleInput = FMath::Clamp(InThrottleValue * ThrottleInputScale, -1.0f, 1.0f);

		VehicleDriveComp->ApplyThrottleInput(ScaledThrottleInput);
	}
}

void ACFVehiclePawn::SetVehicleSteeringInput(const float InSteeringValue)
{
	if (VehicleDriveComp)
	{
		// [v2.55.0] 현재 속도 기준 조향 제한을 적용한 실제 Chaos Vehicle 입력값입니다.
		const float SpeedLimitedSteeringValue = CalculateSpeedLimitedSteeringInput(InSteeringValue);

		VehicleDriveComp->ApplySteeringInput(SpeedLimitedSteeringValue);
	}
}

void ACFVehiclePawn::SetVehicleBrakeInput(const float InBrakeValue)
{
	if (VehicleDriveComp)
	{
		VehicleDriveComp->ApplyBrakeInput(InBrakeValue);
	}
}

void ACFVehiclePawn::SetVehicleHandbrakeInput(const bool bInHandbrakePressed)
{
	if (VehicleDriveComp)
	{
		VehicleDriveComp->ApplyHandbrakeInput(bInHandbrakePressed);
	}
}

// [v2.131.0] Pause 진입 전 모든 차량 Gameplay 입력과 입력 소유권을 안전한 중립 상태로 초기화합니다.
void ACFVehiclePawn::ClearGameplayInputForPause()
{
	LastVehicleMoveInputResult = FCFVehicleMoveInputResult();
	LastMoveDirectionIntent = ECFVehicleMoveDirectionIntent::None;
	CurrentInputOwnership = ECFVehicleInputOwnership::None;
	LastVehicleMoveInputTimeSec = -1.0f;
	LastLegacyAxisInputTimeSec = -1.0f;

	TargetSteeringInput = 0.0f;
	LegacyTargetSteeringInput = 0.0f;
	CurrentSteeringInput = 0.0f;
	LastSteeringTurnRate = 0.0f;
	LastSteeringReturnRate = 0.0f;
	bSteeringReturningToCenter = false;

	if (VehicleDriveComp)
	{
		VehicleDriveComp->ClearDriveInputs();
	}

	if (VehicleCameraComp)
	{
		VehicleCameraComp->ClearLookInput();
	}

	// [v2.131.0] 진행 중 Ripple·Salvo는 취소하지 않는다. World Pause가 Tick과 게임 시간만 정지시키고 해제 후 동일 상태에서 재개한다.
}

float ACFVehiclePawn::GetVehicleSpeed() const
{
	return VehicleDriveComp ? VehicleDriveComp->GetCurrentSpeedKmh() : 0.0f;
}

ECFVehicleDriveState ACFVehiclePawn::GetDriveState() const
{
	return VehicleDriveComp ? VehicleDriveComp->GetDriveState() : ECFVehicleDriveState::Disabled;
}

FCFVehicleDriveStateSnapshot ACFVehiclePawn::GetDriveStateSnapshot() const
{
	return VehicleDriveComp ? VehicleDriveComp->GetDriveStateSnapshot() : FCFVehicleDriveStateSnapshot();
}

FCFVehicleDebugSnapshot ACFVehiclePawn::GetVehicleDebugSnapshot() const
{
	// [v2.14.1] VehicleDebug v2 Phase 1: 기존 필드를 유지하면서 카테고리형 Snapshot을 함께 채웁니다.
	FCFVehicleDebugSnapshot DebugSnapshot;

	// [v2.14.1] 현재 Drive 컴포넌트 존재 여부를 먼저 고정합니다.
	const bool bHasDriveComponent = (VehicleDriveComp != nullptr);

	// [v2.14.1] 현재 WheelSync 컴포넌트 존재 여부를 먼저 고정합니다.
	const bool bHasWheelSyncComponent = (WheelSyncComp != nullptr);

	// [v2.76.0] 현재 Weapon 컴포넌트 존재 여부를 먼저 고정합니다.
	const bool bHasVehicleWeaponComponent = (VehicleWeaponComp != nullptr);

	// [v2.14.1] Drive 카테고리 채우기에 사용할 최신 Drive 상태 스냅샷입니다.
	FCFVehicleDriveStateSnapshot CurrentDriveStateSnapshot;

	DebugSnapshot.bRuntimeReady = bVehicleRuntimeReady;
	DebugSnapshot.RuntimeSummary = LastVehicleRuntimeSummary;
	DebugSnapshot.bHasDriveComponent = bHasDriveComponent;
	DebugSnapshot.bHasWheelSyncComponent = bHasWheelSyncComponent;

	DebugSnapshot.Runtime.bRuntimeReady = bVehicleRuntimeReady;
	DebugSnapshot.Runtime.bHasDriveComponent = bHasDriveComponent;
	DebugSnapshot.Runtime.bHasWheelSyncComponent = bHasWheelSyncComponent;
	DebugSnapshot.Runtime.RuntimeSummary = LastVehicleRuntimeSummary;
	DebugSnapshot.Runtime.LastInitAttemptSummary = LastVehicleRuntimeSummary;
	DebugSnapshot.Runtime.LastValidationSummary = LastVehicleRuntimeSummary;

		DebugSnapshot.Input.DeviceMode = InputDeviceMode;
	DebugSnapshot.Input.InputOwner = CurrentInputOwnership;
	DebugSnapshot.Input.MoveZone = LastVehicleMoveInputResult.ResolvedZone;
	DebugSnapshot.Input.MoveIntent = LastMoveDirectionIntent;
	DebugSnapshot.Input.MoveRaw = LastVehicleMoveInputResult.RawMoveInput;
	DebugSnapshot.Input.MoveMagnitude = LastVehicleMoveInputResult.Magnitude;
	DebugSnapshot.Input.MoveAngle = LastVehicleMoveInputResult.AngleDeg;
	DebugSnapshot.Input.bUsedBlackZoneHold = LastVehicleMoveInputResult.bUsedBlackZoneHold;
	DebugSnapshot.Input.TargetSteeringInput = TargetSteeringInput;
	DebugSnapshot.Input.CurrentSteeringInput = CurrentSteeringInput;
	DebugSnapshot.Input.LastSteeringTurnRate = LastSteeringTurnRate;
	DebugSnapshot.Input.LastSteeringReturnRate = LastSteeringReturnRate;
	DebugSnapshot.Input.bSteeringReturningToCenter = bSteeringReturningToCenter;


	// [v2.7.0] Camera 카테고리는 VehicleCameraComp가 제공하는 원본 런타임 스냅샷과 표시용 압축 비율을 함께 담습니다.
	DebugSnapshot.Camera.bHasVehicleCameraComponent = (VehicleCameraComp != nullptr);
	if (DebugSnapshot.Camera.bHasVehicleCameraComponent)
	{
		DebugSnapshot.Camera.CameraRuntimeState = VehicleCameraComp->GetCameraRuntimeState();

		const float DesiredArmLength = DebugSnapshot.Camera.CameraRuntimeState.DesiredArmLength;
		const float SolvedArmLength = DebugSnapshot.Camera.CameraRuntimeState.SolvedArmLength;

		if (DesiredArmLength > KINDA_SMALL_NUMBER)
		{
			DebugSnapshot.Camera.CollisionCompressionRatio = FMath::Clamp(SolvedArmLength / DesiredArmLength, 0.0f, 1.0f);
		}
		else
		{
			DebugSnapshot.Camera.CollisionCompressionRatio = 1.0f;
		}

		DebugSnapshot.Camera.bCameraCompressedByCollision = DebugSnapshot.Camera.CollisionCompressionRatio < 0.90f;
	}

	// [v2.16.0] Aim 카테고리는 VehicleAimComp가 제공하는 현재 상태를 표시용으로만 읽습니다.
	DebugSnapshot.Aim.bHasVehicleAimComponent = (VehicleAimComp != nullptr);
	if (DebugSnapshot.Aim.bHasVehicleAimComponent)
	{
		DebugSnapshot.Aim.bAimRuntimeReady = VehicleAimComp->IsAimRuntimeReady();
		DebugSnapshot.Aim.LocalAimState = VehicleAimComp->GetLocalAimState();
		DebugSnapshot.Aim.FireValidationState = VehicleAimComp->GetFireValidationState();
		DebugSnapshot.Aim.AimVisualState = VehicleAimComp->GetAimVisualState();
		DebugSnapshot.Aim.ReticleState = VehicleAimComp->GetReticleState();
		DebugSnapshot.Aim.AimRuntimeSummary = VehicleAimComp->GetLastAimRuntimeSummary();
		DebugSnapshot.Aim.LastFireRequest = LastFireRequest;
		DebugSnapshot.Aim.LastFireResult = LastFireResult;
		}

	// [v2.138.0] Target 카테고리는 현재 TargetSelect 선택 기록과 선택 Actor의 컴포넌트 상태만 읽으며 선택·피해 계산은 수행하지 않습니다.
	DebugSnapshot.Target.bHasTargetSelectComponent = (TargetSelectComp != nullptr);
	if (TargetSelectComp)
	{
		DebugSnapshot.Target.bHasSelectedTarget = TargetSelectComp->HasSelectedTarget();
		DebugSnapshot.Target.bSelectedTargetValid = TargetSelectComp->IsSelectedTargetValid();
		DebugSnapshot.Target.SelectedTargetTrackState = TargetSelectComp->GetSelectedTargetTrackState();
		DebugSnapshot.Target.SelectedTargetLifetimeSummary = TargetSelectComp->BuildSelectedTargetLifetimeDebugSummary();

		// [v2.138.0] TargetSelect가 선택 시점에 캐시한 표시 정보입니다.
		const FCFTargetDisplayInfo SelectedTargetDisplayInfo = TargetSelectComp->GetSelectedTargetDisplayInfo();
		DebugSnapshot.Target.SelectedTargetId = SelectedTargetDisplayInfo.TargetId;
		DebugSnapshot.Target.SelectedTargetDisplayName = SelectedTargetDisplayInfo.DisplayName;

		// [v2.138.0] 방어와 내구도 상태를 읽어올 현재 선택 대상 Actor입니다.
		AActor* SelectedTargetActor = TargetSelectComp->GetSelectedTargetActor();
		if (IsValid(SelectedTargetActor))
		{
			DebugSnapshot.Target.SelectedTargetActorName = SelectedTargetActor->GetName();

			// [v2.138.0] 선택 대상의 Shield, 방향별 Armor와 재생 상태를 제공하는 컴포넌트입니다.
			UCFVehicleDefenseComp* SelectedTargetDefenseComp = SelectedTargetActor->FindComponentByClass<UCFVehicleDefenseComp>();
			DebugSnapshot.Target.bHasSelectedTargetDefenseComponent = (SelectedTargetDefenseComp != nullptr);
			if (SelectedTargetDefenseComp)
			{
				DebugSnapshot.Target.bSelectedTargetDefenseInitialized = SelectedTargetDefenseComp->IsDefenseInitialized();
				DebugSnapshot.Target.bSelectedTargetUsingLegacyDefenseFallback = !SelectedTargetDefenseComp->IsDefenseInitialized()
					|| SelectedTargetDefenseComp->GetActiveDefenseData() == nullptr;
				DebugSnapshot.Target.SelectedTargetDefenseSummary = SelectedTargetDefenseComp->BuildVehicleDefenseSummary();
				DebugSnapshot.Target.bHasSelectedTargetLastDamageResult = SelectedTargetDefenseComp->HasLastVehicleDamageResult();
				DebugSnapshot.Target.SelectedTargetLastDamageResultSummary = SelectedTargetDefenseComp->GetLastVehicleDamageResultSummary();
			}

			// [v2.138.0] 선택 대상의 현재·최대 Integrity와 파괴 상태를 제공하는 컴포넌트입니다.
			UCFVehicleHealthComp* SelectedTargetHealthComp = SelectedTargetActor->FindComponentByClass<UCFVehicleHealthComp>();
			DebugSnapshot.Target.bHasSelectedTargetHealthComponent = (SelectedTargetHealthComp != nullptr);
			if (SelectedTargetHealthComp)
			{
				DebugSnapshot.Target.bSelectedTargetHealthInitialized = SelectedTargetHealthComp->IsHealthInitialized();
				DebugSnapshot.Target.SelectedTargetCurrentIntegrity = SelectedTargetHealthComp->GetCurrentIntegrity();
				DebugSnapshot.Target.SelectedTargetMaximumIntegrity = SelectedTargetHealthComp->GetMaximumIntegrity();
				DebugSnapshot.Target.bSelectedTargetDestroyed = SelectedTargetHealthComp->IsDestroyed();
			}
		}
	}

	// [v2.76.0] Weapon 카테고리는 VehicleWeaponComp가 제공하는 런타임과 FireOrigin 상태를 표시용으로만 읽습니다.
	DebugSnapshot.Weapon.bHasVehicleWeaponComponent = bHasVehicleWeaponComponent;
	if (DebugSnapshot.Weapon.bHasVehicleWeaponComponent)
	{
		// [v2.78.0] Weapon Debug에서 fallback으로 표시할 Aim Profile 최대 거리입니다.
		const float FallbackWeaponRange = VehicleAimComp ? VehicleAimComp->GetDefaultAimProfile().MaxAimDistance : 0.0f;

		// [v2.78.0] Weapon Debug에서 쿨다운 계산에 사용할 현재 월드 시간입니다.
		const float CurrentWeaponTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

		DebugSnapshot.Weapon.bWeaponRuntimeReady = VehicleWeaponComp->IsWeaponRuntimeReady();
		DebugSnapshot.Weapon.ActiveMountProfileId = VehicleWeaponComp->GetActiveMountProfileId();
		DebugSnapshot.Weapon.LastFireOrigin = VehicleWeaponComp->GetLastFireOrigin();
		DebugSnapshot.Weapon.LastWeaponRuntimeSummary = VehicleWeaponComp->GetLastWeaponRuntimeSummary();

		// [v2.99.0] 현재 활성 장착 프로파일에서 우선 해석한 EquipmentPresetData입니다.
		UCFEquipmentPresetData* ActiveEquipmentPresetData = VehicleWeaponComp->GetActiveEquipmentPresetData();

		DebugSnapshot.Weapon.ActiveEquipmentPresetData = ActiveEquipmentPresetData;
		DebugSnapshot.Weapon.bActiveEquipmentPresetDataAssigned = (ActiveEquipmentPresetData != nullptr);
		DebugSnapshot.Weapon.bActiveEquipmentPresetDataCompatible = VehicleWeaponComp->IsActiveEquipmentPresetCompatible();
		DebugSnapshot.Weapon.ActiveEquipmentPresetSummary = VehicleWeaponComp->GetActiveEquipmentPresetSummary();

		if (ActiveEquipmentPresetData)
		{
			DebugSnapshot.Weapon.ActiveEquipmentPresetId = ActiveEquipmentPresetData->EquipmentId;
		}

		// [v2.77.0] 현재 활성 장착 프로파일에 연결된 WeaponData입니다.
		UCFWeaponData* ActiveWeaponData = VehicleWeaponComp->GetActiveWeaponData();

		DebugSnapshot.Weapon.ActiveWeaponData = ActiveWeaponData;
		DebugSnapshot.Weapon.bActiveWeaponDataCompatible = VehicleWeaponComp->IsActiveWeaponDataCompatible();
		DebugSnapshot.Weapon.ActiveWeaponSummary = VehicleWeaponComp->GetActiveWeaponSummary();
		DebugSnapshot.Weapon.ActiveProjectileData = VehicleWeaponComp->GetActiveProjectileData();
		DebugSnapshot.Weapon.ActiveProjectileSummary = VehicleWeaponComp->GetActiveProjectileSummary();
		DebugSnapshot.Weapon.bActiveProjectileSpawnReady = VehicleWeaponComp->IsActiveProjectileSpawnReady();
		DebugSnapshot.Weapon.ActiveProjectileExecutionSummary = VehicleWeaponComp->GetActiveProjectileExecutionSummary();
		DebugSnapshot.Weapon.ActiveDamageData = VehicleWeaponComp->GetActiveDamageData();
		DebugSnapshot.Weapon.ActiveDamageId = VehicleWeaponComp->GetActiveDamageId();
		DebugSnapshot.Weapon.ActiveDamageSummary = VehicleWeaponComp->GetActiveDamageSummary();
		DebugSnapshot.Weapon.ActiveDamageResolutionSummary = VehicleWeaponComp->GetActiveDamageResolutionSummary();
		DebugSnapshot.Weapon.bHasLastDamageHitContext = bHasLastDamageHitContext;
		DebugSnapshot.Weapon.LastDamageHitContext = LastDamageHitContext;
			DebugSnapshot.Weapon.LastDamageHitContextSummary = LastDamageHitContextSummary;
	DebugSnapshot.Weapon.bHasLastDamageApplyResult = bHasLastDamageApplyResult;
	DebugSnapshot.Weapon.LastDamageApplyResult = LastDamageApplyResult;
	DebugSnapshot.Weapon.LastDamageApplyResultSummary = LastDamageApplyResultSummary;
		DebugSnapshot.Weapon.ActiveWeaponMaxRange = VehicleWeaponComp->GetActiveWeaponMaxRange(FallbackWeaponRange);
		DebugSnapshot.Weapon.ActiveWeaponFireRatePerMinute = VehicleWeaponComp->GetActiveWeaponFireRatePerMinute();
		DebugSnapshot.Weapon.ActiveWeaponCooldownSeconds = VehicleWeaponComp->GetActiveWeaponCooldownSeconds();
		DebugSnapshot.Weapon.ActiveWeaponRemainingCooldownSeconds = VehicleWeaponComp->GetRemainingCooldownSeconds(CurrentWeaponTimeSeconds);
		DebugSnapshot.Weapon.LastAcceptedWeaponFireTimeSeconds = VehicleWeaponComp->GetLastAcceptedFireTimeSeconds();
		DebugSnapshot.Weapon.TurretState = VehicleWeaponComp->GetTurretState();
		DebugSnapshot.Weapon.TurretRuntimeSummary = VehicleWeaponComp->GetLastTurretRuntimeSummary();

		if (ActiveWeaponData)
		{
			DebugSnapshot.Weapon.ActiveWeaponId = ActiveWeaponData->WeaponId;
		}

		if (DebugSnapshot.Weapon.ActiveProjectileData)
		{
			DebugSnapshot.Weapon.ActiveProjectileId = DebugSnapshot.Weapon.ActiveProjectileData->ProjectileId;
		}
	}

		// [v2.130.0] 방어 Debug는 실제 피해를 재계산하지 않고 VehicleDefenseComp의 현재 상태와 마지막 결과 캐시만 읽습니다.
	DebugSnapshot.Weapon.bHasVehicleDefenseComponent = VehicleDefenseComp != nullptr;
	if (VehicleDefenseComp)
	{
		DebugSnapshot.Weapon.bVehicleDefenseInitialized = VehicleDefenseComp->IsDefenseInitialized();
		DebugSnapshot.Weapon.bUsingLegacyDefenseFallback = !VehicleDefenseComp->IsDefenseInitialized()
			|| VehicleDefenseComp->GetActiveDefenseData() == nullptr;
		DebugSnapshot.Weapon.VehicleDefenseSummary = VehicleDefenseComp->BuildVehicleDefenseSummary();
		DebugSnapshot.Weapon.bHasLastVehicleDamageResult = VehicleDefenseComp->HasLastVehicleDamageResult();
		DebugSnapshot.Weapon.LastVehicleDamageResultSummary = VehicleDefenseComp->GetLastVehicleDamageResultSummary();
	}

	// [v2.87.0] 터렛 마운트 / 시각 장착 상태는 WeaponComp 런타임과 별개로 표시용 캐시에서 읽습니다.
	DebugSnapshot.Weapon.ActiveTurretMountData = LastTurretMountData;
	DebugSnapshot.Weapon.bActiveTurretMountDataAssigned = bLastTurretMountDataAssigned;
	DebugSnapshot.Weapon.ActiveTurretMountId = LastTurretMountId;
	DebugSnapshot.Weapon.ActiveTurretMountSummary = LastTurretMountSummary;
	DebugSnapshot.Weapon.bTurretVisualAttached = bLastTurretVisualAttached;
	DebugSnapshot.Weapon.TurretVisualSummary = LastTurretVisualSummary;
	DebugSnapshot.Weapon.TurretBaseMeshName = LastTurretBaseMeshName;
	DebugSnapshot.Weapon.TurretYawMeshName = LastTurretYawMeshName;
	DebugSnapshot.Weapon.TurretPitchMeshName = LastTurretPitchMeshName;

	// [v2.83.0] Projectile Pool 디버그 카운트는 WeaponComp 상태와 별개로 표시용으로만 읽습니다.
	DebugSnapshot.Weapon.bHasProjectilePoolComponent = (ProjectilePoolComp != nullptr);
	if (DebugSnapshot.Weapon.bHasProjectilePoolComponent)
	{
		DebugSnapshot.Weapon.TotalPooledProjectileCount = ProjectilePoolComp->GetTotalPooledProjectileCount();
		DebugSnapshot.Weapon.ActivePooledProjectileCount = ProjectilePoolComp->GetActivePooledProjectileCount();
		DebugSnapshot.Weapon.InactivePooledProjectileCount = ProjectilePoolComp->GetInactivePooledProjectileCount();
		DebugSnapshot.Weapon.LastProjectileReleaseSummary = ProjectilePoolComp->GetLastProjectileReleaseSummary();
	}

	DebugSnapshot.Overview.bRuntimeReady = bVehicleRuntimeReady;
	DebugSnapshot.Overview.DeviceMode = InputDeviceMode;
	DebugSnapshot.Overview.InputOwner = CurrentInputOwnership;

	if (bHasDriveComponent)
	{
		CurrentDriveStateSnapshot = VehicleDriveComp->GetDriveStateSnapshot();
		DebugSnapshot.CurrentDriveState = VehicleDriveComp->GetDriveState();
		DebugSnapshot.PreviousDriveState = VehicleDriveComp->GetPreviousDriveState();
		DebugSnapshot.bDriveStateChangedThisFrame = VehicleDriveComp->HasDriveStateChangedThisFrame();
		DebugSnapshot.DriveStateTransitionSummary = VehicleDriveComp->GetLastDriveStateTransitionSummary();
		DebugSnapshot.DriveStateSnapshot = CurrentDriveStateSnapshot;

		DebugSnapshot.Drive.CurrentDriveState = DebugSnapshot.CurrentDriveState;
		DebugSnapshot.Drive.PreviousDriveState = DebugSnapshot.PreviousDriveState;
		DebugSnapshot.Drive.bDriveStateChangedThisFrame = DebugSnapshot.bDriveStateChangedThisFrame;
		DebugSnapshot.Drive.SpeedKmh = CurrentDriveStateSnapshot.CurrentSpeedKmh;
		DebugSnapshot.Drive.ForwardSpeedKmh = CurrentDriveStateSnapshot.ForwardSpeedKmh;
		DebugSnapshot.Drive.Throttle = CurrentDriveStateSnapshot.CurrentInputState.ThrottleInput;
		DebugSnapshot.Drive.Brake = CurrentDriveStateSnapshot.CurrentInputState.BrakeInput;
		DebugSnapshot.Drive.Steering = CurrentDriveStateSnapshot.CurrentInputState.SteeringInput;
		DebugSnapshot.Drive.bHandbrake = CurrentDriveStateSnapshot.CurrentInputState.bHandbrakePressed;
		DebugSnapshot.Drive.DriveStateTransitionSummary = DebugSnapshot.DriveStateTransitionSummary;
		DebugSnapshot.Drive.DriveStateSnapshot = CurrentDriveStateSnapshot;

		DebugSnapshot.Overview.CurrentDriveState = DebugSnapshot.CurrentDriveState;
		DebugSnapshot.Overview.SpeedKmh = CurrentDriveStateSnapshot.CurrentSpeedKmh;
		DebugSnapshot.Overview.ForwardSpeedKmh = CurrentDriveStateSnapshot.ForwardSpeedKmh;
		DebugSnapshot.Overview.LastTransitionShortText = DebugSnapshot.DriveStateTransitionSummary;
	}

	return DebugSnapshot;
}

FCFVehicleDebugOverview ACFVehiclePawn::GetVehicleDebugOverview() const
{
	// [v2.14.2] HUD 위젯이 필요한 카테고리만 직접 읽을 수 있도록 Overview를 반환합니다.
	return GetVehicleDebugSnapshot().Overview;
}

FCFVehicleDebugDrive ACFVehiclePawn::GetVehicleDebugDrive() const
{
	// [v2.14.2] 상세 패널이 필요한 Drive 카테고리만 직접 읽을 수 있도록 반환합니다.
	return GetVehicleDebugSnapshot().Drive;
}

FCFVehicleDebugInput ACFVehiclePawn::GetVehicleDebugInput() const
{
	// [v2.14.2] 상세 패널이 필요한 Input 카테고리만 직접 읽을 수 있도록 반환합니다.
	return GetVehicleDebugSnapshot().Input;
}

FCFVehicleDebugCamera ACFVehiclePawn::GetVehicleDebugCamera() const
{
	// [v2.7.0] 상세 패널이 필요한 Camera 카테고리만 직접 읽을 수 있도록 반환합니다.
	return GetVehicleDebugSnapshot().Camera;
}

FCFVehicleDebugAim ACFVehiclePawn::GetVehicleDebugAim() const
{
	// [v2.16.0] 상세 패널이 필요한 Aim 카테고리만 직접 읽을 수 있도록 반환합니다.
	return GetVehicleDebugSnapshot().Aim;
}

// [v2.138.0] 상세 패널이 필요한 현재 선택 대상 카테고리만 직접 읽을 수 있도록 반환합니다.
FCFVehicleDebugTarget ACFVehiclePawn::GetVehicleDebugTarget() const
{
	return GetVehicleDebugSnapshot().Target;
}

// [v2.76.0] 상세 패널이 필요한 Weapon 카테고리만 직접 읽을 수 있도록 반환합니다.
FCFVehicleDebugWeapon ACFVehiclePawn::GetVehicleDebugWeapon() const
{
	return GetVehicleDebugSnapshot().Weapon;
}

FCFVehicleDebugRuntime ACFVehiclePawn::GetVehicleDebugRuntime() const
{
	// [v2.14.2] 상세 패널이 필요한 Runtime 카테고리만 직접 읽을 수 있도록 반환합니다.
	return GetVehicleDebugSnapshot().Runtime;
}

FText ACFVehiclePawn::GetDebugTextSingleLine() const
{
	return FText::FromString(BuildVehicleDebugSummary(false, true, bShowDriveStateTransitionSummary, true));
}

FText ACFVehiclePawn::GetDebugTextMultiLine() const
{
	return FText::FromString(BuildVehicleDebugSummary(true, true, bShowDriveStateTransitionSummary, true));
}

FText ACFVehiclePawn::GetDebugTextByDisplayMode() const
{
	if (DriveStateDebugDisplayMode == ECFVehicleDebugDisplayMode::Off)
	{
		return FText::GetEmpty();
	}
	if (DriveStateDebugDisplayMode == ECFVehicleDebugDisplayMode::MultiLine)
	{
		return GetDebugTextMultiLine();
	}
	return GetDebugTextSingleLine();
}

bool ACFVehiclePawn::ShouldShowVehicleDebugUi() const
{
	// [v2.21.0] VehicleDebug HUD/Panel은 Viewport가 있는 로컬 제어 Pawn에서만 표시합니다.
	return bEnableDriveStateOnScreenDebug && (GetNetMode() != NM_DedicatedServer) && IsLocallyControlled();
}

bool ACFVehiclePawn::ShouldShowDebugWidget() const
{
	// [v2.14.3] 레거시 WBP_VehicleDebug 제거 전환을 위해 기존 Text Widget 표시는 항상 비활성화합니다.
	return false;
}

bool ACFVehiclePawn::ShouldShowVehicleDebugHud() const
{
	// [v2.14.3] HUD는 레거시 Text Widget과 분리된 공통 UI 표시 조건과 HUD 전용 토글을 함께 만족할 때만 표시합니다.
	return ShouldShowVehicleDebugUi() && bShowVehicleDebugHud;
}

bool ACFVehiclePawn::ShouldShowVehicleDebugPanel() const
{
	// [v2.14.3] 상세 패널은 레거시 Text Widget과 분리된 공통 UI 표시 조건과 Panel 전용 토글을 함께 만족할 때만 표시합니다.
	return ShouldShowVehicleDebugUi() && bShowVehicleDebugPanel;
}

bool ACFVehiclePawn::ShouldShowVehicleDebugText() const
{
	// [v2.14.3] 레거시 WBP_VehicleDebug 제거 전환을 위해 Legacy Text View 표시는 항상 비활성화합니다.
	return false;
}

ESlateVisibility ACFVehiclePawn::GetDebugWidgetVisibility() const
{
	// [v2.14.3] 레거시 WBP_VehicleDebug 제거 전환을 위해 Visibility는 항상 Collapsed를 반환합니다.
	return ESlateVisibility::Collapsed;
}

// [v2.48.0] 로컬 Owner 표시 안정화용 차체/휠 표시 계층을 준비합니다.
bool ACFVehiclePawn::PrepareOwnerVisualStabilization()
{
	bOwnerVisualStabilizationReady = false;
	OwnerVisualStabilizedComponents.Reset();

	if (!bEnableOwnerVisualStabilization)
	{
		ResetOwnerVisualStabilization();
		return false;
	}

	if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled())
	{
		ResetOwnerVisualStabilization();
		return false;
	}

	if (!OwnerVisualRootComp)
	{
		return false;
	}

	// [v2.48.0] Owner 표시 루트의 부모가 될 차량 물리 루트 SkeletalMeshComponent입니다.
	USkeletalMeshComponent* VehicleMeshComponent = FindSkeletalMeshComponentByName(this, TEXT("VehicleMesh"));
	if (!VehicleMeshComponent)
	{
		VehicleMeshComponent = GetMesh();
	}

	if (!VehicleMeshComponent)
	{
		return false;
	}

	if (OwnerVisualRootComp->GetAttachParent() != VehicleMeshComponent)
	{
		OwnerVisualRootComp->AttachToComponent(VehicleMeshComponent, FAttachmentTransformRules::KeepRelativeTransform);
	}

	OwnerVisualRootComp->SetRelativeLocation(FVector::ZeroVector);
	OwnerVisualRootComp->SetRelativeRotation(FRotator::ZeroRotator);
	OwnerVisualRootComp->SetRelativeScale3D(FVector::OneVector);

	// [v2.48.0] Owner 표시 루트 아래로 묶을 차체/휠 표시 컴포넌트 이름 목록입니다.
	const TArray<FName> OwnerVisualComponentNames =
	{
		TEXT("SM_Body"),
		TEXT("Wheel_Anchor_FL"),
		TEXT("Wheel_Anchor_FR"),
		TEXT("Wheel_Anchor_RL"),
		TEXT("Wheel_Anchor_RR"),
		TEXT("Wheel_Mesh_FL"),
		TEXT("Wheel_Mesh_FR"),
		TEXT("Wheel_Mesh_RL"),
		TEXT("Wheel_Mesh_RR")
	};

	// [v2.48.0] Owner 표시 루트 아래로 이동한 컴포넌트 개수입니다.
	int32 AttachedVisualComponentCount = 0;
	for (const FName& OwnerVisualComponentName : OwnerVisualComponentNames)
	{
		// [v2.48.0] 이름으로 찾은 표시 대상 SceneComponent입니다.
		USceneComponent* VisualComponent = FindSceneComponentByName(this, OwnerVisualComponentName);
		if (AttachOwnerVisualComponent(VisualComponent))
		{
			++AttachedVisualComponentCount;
		}
	}

	if (VehicleMeshComponent && bHideOwnerPhysicsMeshWhenStabilized)
	{
		VehicleMeshComponent->SetVisibility(false, false);
		VehicleMeshComponent->SetHiddenInGame(true, false);
		bOwnerVisualPhysicsMeshHidden = true;
	}
	else if (VehicleMeshComponent && bOwnerVisualPhysicsMeshHidden)
	{
		VehicleMeshComponent->SetVisibility(true, false);
		VehicleMeshComponent->SetHiddenInGame(false, false);
		bOwnerVisualPhysicsMeshHidden = false;
	}

	SmoothedOwnerVisualRotation = GetActorRotation();
	bHasSmoothedOwnerVisualRotation = true;
	bOwnerVisualStabilizationReady = AttachedVisualComponentCount > 0;

	return bOwnerVisualStabilizationReady;
}

// [v2.48.0] 지정한 표시 컴포넌트를 Owner 표시 루트 아래로 안전하게 이동합니다.
bool ACFVehiclePawn::AttachOwnerVisualComponent(USceneComponent* VisualComponent)
{
	if (!VisualComponent || !OwnerVisualRootComp)
	{
		return false;
	}

	if ((VisualComponent == OwnerVisualRootComp) || (VisualComponent == GetRootComponent()) || (VisualComponent == GetMesh()))
	{
		return false;
	}

	if (!VisualComponent->IsRegistered())
	{
		return false;
	}

	if (!VisualComponent->IsAttachedTo(OwnerVisualRootComp))
	{
		VisualComponent->AttachToComponent(OwnerVisualRootComp, FAttachmentTransformRules::KeepWorldTransform);
	}

	OwnerVisualStabilizedComponents.AddUnique(VisualComponent);
	return true;
}

// [v2.48.0] 로컬 Owner 표시 루트 회전을 현재 Actor 회전에 부드럽게 맞춥니다.
void ACFVehiclePawn::UpdateOwnerVisualStabilization(const float DeltaSeconds)
{
	if (!bEnableOwnerVisualStabilization)
	{
		ResetOwnerVisualStabilization();
		return;
	}

	if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled())
	{
		ResetOwnerVisualStabilization();
		return;
	}

	if (!OwnerVisualRootComp)
	{
		return;
	}

	if (!bOwnerVisualStabilizationReady)
	{
		// [v2.48.0] 이번 Tick에서 Owner 표시 안정화 계층 준비에 성공했는지 여부입니다.
		const bool bPreparedOwnerVisualThisFrame = PrepareOwnerVisualStabilization();
		if (bPreparedOwnerVisualThisFrame && WheelSyncComp && bVehicleRuntimeReady)
		{
			PrepareWheelSync();
		}
	}

	if (!bOwnerVisualStabilizationReady)
	{
		return;
	}

	// [v2.48.0] 현재 물리 Actor 회전입니다.
	const FRotator CurrentActorRotation = GetActorRotation();

	if (!bHasSmoothedOwnerVisualRotation)
	{
		SmoothedOwnerVisualRotation = CurrentActorRotation;
		bHasSmoothedOwnerVisualRotation = true;
	}

	// [v2.48.0] 음수 Tick 간격을 방지한 표시 안정화 DeltaSeconds입니다.
	const float SafeDeltaSeconds = FMath::Max(DeltaSeconds, 0.0f);

	// [v2.48.0] 표시 루트 회전 보간에 사용할 안전한 보간 속도입니다.
	const float SafeInterpSpeed = FMath::Max(OwnerVisualStabilizationInterpSpeed, 0.1f);

	// [v2.48.0] 현재 보간 속도로 Actor 회전을 따라간 후보 표시 회전입니다.
	const FRotator InterpolatedVisualRotation = FMath::RInterpTo(SmoothedOwnerVisualRotation, CurrentActorRotation, SafeDeltaSeconds, SafeInterpSpeed);

	// [v2.48.0] 축별 사용 여부를 반영한 표시 회전 후보입니다.
	FRotator DesiredVisualRotation = CurrentActorRotation;
	if (bOwnerVisualStabilizePitchRoll)
	{
		DesiredVisualRotation.Pitch = InterpolatedVisualRotation.Pitch;
		DesiredVisualRotation.Roll = InterpolatedVisualRotation.Roll;
	}
	if (bOwnerVisualStabilizeYaw)
	{
		DesiredVisualRotation.Yaw = InterpolatedVisualRotation.Yaw;
	}

	SmoothedOwnerVisualRotation = ClampOwnerVisualStabilizedRotation(CurrentActorRotation, DesiredVisualRotation);
	OwnerVisualRootComp->SetWorldRotation(SmoothedOwnerVisualRotation, false, nullptr, ETeleportType::TeleportPhysics);
}

// [v2.48.0] Owner 표시 안정화 상태를 기본 회전으로 되돌립니다.
void ACFVehiclePawn::ResetOwnerVisualStabilization()
{
	if (OwnerVisualRootComp)
	{
		OwnerVisualRootComp->SetRelativeRotation(FRotator::ZeroRotator);
	}

	SmoothedOwnerVisualRotation = GetActorRotation();
	bHasSmoothedOwnerVisualRotation = false;
	bOwnerVisualStabilizationReady = false;
	OwnerVisualStabilizedComponents.Reset();

	// [v2.48.0] 물리 루트 렌더링 복구 대상 SkeletalMeshComponent입니다.
	USkeletalMeshComponent* VehicleMeshComponent = FindSkeletalMeshComponentByName(this, TEXT("VehicleMesh"));
	if (!VehicleMeshComponent)
	{
		VehicleMeshComponent = GetMesh();
	}

	if (VehicleMeshComponent && bOwnerVisualPhysicsMeshHidden)
	{
		VehicleMeshComponent->SetVisibility(true, false);
		VehicleMeshComponent->SetHiddenInGame(false, false);
		bOwnerVisualPhysicsMeshHidden = false;
	}
}

// [v2.48.0] Actor 회전과 표시 회전 사이의 지연각을 설정 한도 안으로 제한합니다.
FRotator ACFVehiclePawn::ClampOwnerVisualStabilizedRotation(const FRotator& CurrentActorRotation, const FRotator& DesiredVisualRotation) const
{
	// [v2.48.0] 표시 루트가 Actor 회전에서 벗어날 수 있는 최대 축별 각도입니다.
	const float SafeMaxLagDeg = FMath::Max(OwnerVisualStabilizationMaxLagDeg, 0.0f);

	// [v2.48.0] Actor Pitch에서 표시 Pitch까지의 지연각입니다.
	const float PitchLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Pitch, DesiredVisualRotation.Pitch);

	// [v2.48.0] Actor Yaw에서 표시 Yaw까지의 지연각입니다.
	const float YawLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Yaw, DesiredVisualRotation.Yaw);

	// [v2.48.0] Actor Roll에서 표시 Roll까지의 지연각입니다.
	const float RollLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Roll, DesiredVisualRotation.Roll);

	// [v2.48.0] 최대 지연각으로 제한한 표시 Pitch입니다.
	const float ClampedPitchDeg = CurrentActorRotation.Pitch + FMath::Clamp(PitchLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// [v2.48.0] 최대 지연각으로 제한한 표시 Yaw입니다.
	const float ClampedYawDeg = CurrentActorRotation.Yaw + FMath::Clamp(YawLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// [v2.48.0] 최대 지연각으로 제한한 표시 Roll입니다.
	const float ClampedRollDeg = CurrentActorRotation.Roll + FMath::Clamp(RollLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// [v2.48.0] 정규화 전 최종 표시 회전입니다.
	FRotator ClampedVisualRotation(ClampedPitchDeg, ClampedYawDeg, ClampedRollDeg);
	ClampedVisualRotation.Normalize();

	return ClampedVisualRotation;
}

// [v2.53.0] 로컬 조작 차량의 SM_Body 표시 회전을 부드럽게 안정화합니다.
void ACFVehiclePawn::UpdateOwnerBodyVisualStabilization(const float DeltaSeconds)
{
	if (!bEnableOwnerBodyVisualStabilization || bEnableOwnerVisualStabilization)
	{
		ResetOwnerBodyVisualStabilization();
		return;
	}

	if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled())
	{
		ResetOwnerBodyVisualStabilization();
		return;
	}

	// [v2.53.0] 안정화 대상 차체 표시 컴포넌트입니다.
	UStaticMeshComponent* BodyMeshComponent = FindStaticMeshComponentByName(this, TEXT("SM_Body"));
	if (!BodyMeshComponent || !BodyMeshComponent->IsRegistered() || BodyMeshComponent->IsSimulatingPhysics())
	{
		bOwnerBodyVisualStabilizationReady = false;
		return;
	}

	if (!bHasOriginalOwnerBodyVisualRelativeRotation)
	{
		// [v2.53.0] 안정화 전 차체 표시 기본 상대 회전입니다.
		const FRotator CurrentBodyRelativeRotation = BodyMeshComponent->GetRelativeRotation();

		OriginalOwnerBodyVisualRelativeRotation = CurrentBodyRelativeRotation;
		bHasOriginalOwnerBodyVisualRelativeRotation = true;
	}

	// [v2.53.0] 현재 물리 Actor 회전입니다.
	const FRotator CurrentActorRotation = GetActorRotation();

	if (!bHasSmoothedOwnerBodyVisualRotation)
	{
		SmoothedOwnerBodyVisualRotation = CurrentActorRotation;
		bHasSmoothedOwnerBodyVisualRotation = true;
	}

	// [v2.53.0] 음수 Tick 간격을 방지한 차체 표시 안정화 DeltaSeconds입니다.
	const float SafeDeltaSeconds = FMath::Max(DeltaSeconds, 0.0f);

	// [v2.53.0] 차체 표시 회전 보간에 사용할 안전한 보간 속도입니다.
	const float SafeInterpSpeed = FMath::Max(OwnerBodyVisualInterpSpeed, 0.1f);

	// [v2.53.0] 현재 보간 속도로 Actor 회전을 따라간 후보 차체 표시 회전입니다.
	const FRotator InterpolatedBodyRotation = FMath::RInterpTo(SmoothedOwnerBodyVisualRotation, CurrentActorRotation, SafeDeltaSeconds, SafeInterpSpeed);

	// [v2.53.0] 축별 사용 여부를 반영한 차체 표시 회전 후보입니다.
	FRotator DesiredBodyRotation = CurrentActorRotation;
	if (bOwnerBodyVisualStabilizePitchRoll)
	{
		DesiredBodyRotation.Pitch = InterpolatedBodyRotation.Pitch;
		DesiredBodyRotation.Roll = InterpolatedBodyRotation.Roll;
	}
	if (bOwnerBodyVisualStabilizeYaw)
	{
		DesiredBodyRotation.Yaw = InterpolatedBodyRotation.Yaw;
	}

	SmoothedOwnerBodyVisualRotation = ClampOwnerBodyVisualRotation(CurrentActorRotation, DesiredBodyRotation);
	BodyMeshComponent->SetWorldRotation(SmoothedOwnerBodyVisualRotation, false, nullptr, ETeleportType::TeleportPhysics);
	bOwnerBodyVisualStabilizationReady = true;
}

// [v2.53.0] 로컬 조작 차량의 SM_Body 표시 안정화 상태를 기본 상태로 되돌립니다.
void ACFVehiclePawn::ResetOwnerBodyVisualStabilization()
{
	// [v2.53.0] 리셋 대상 차체 표시 컴포넌트입니다.
	UStaticMeshComponent* BodyMeshComponent = FindStaticMeshComponentByName(this, TEXT("SM_Body"));
	if (BodyMeshComponent && BodyMeshComponent->IsRegistered() && bHasOriginalOwnerBodyVisualRelativeRotation)
	{
		BodyMeshComponent->SetRelativeRotation(OriginalOwnerBodyVisualRelativeRotation);
	}

	SmoothedOwnerBodyVisualRotation = GetActorRotation();
	bHasSmoothedOwnerBodyVisualRotation = false;
	bOwnerBodyVisualStabilizationReady = false;
}

// [v2.53.0] Actor 회전과 SM_Body 표시 회전 사이의 지연각을 설정 한도 안으로 제한합니다.
FRotator ACFVehiclePawn::ClampOwnerBodyVisualRotation(const FRotator& CurrentActorRotation, const FRotator& DesiredVisualRotation) const
{
	// [v2.53.0] 차체 표시가 Actor 회전에서 벗어날 수 있는 최대 축별 각도입니다.
	const float SafeMaxLagDeg = FMath::Max(OwnerBodyVisualMaxLagDeg, 0.0f);

	// [v2.53.0] Actor Pitch에서 차체 표시 Pitch까지의 지연각입니다.
	const float PitchLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Pitch, DesiredVisualRotation.Pitch);

	// [v2.53.0] Actor Yaw에서 차체 표시 Yaw까지의 지연각입니다.
	const float YawLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Yaw, DesiredVisualRotation.Yaw);

	// [v2.53.0] Actor Roll에서 차체 표시 Roll까지의 지연각입니다.
	const float RollLagDeg = FMath::FindDeltaAngleDegrees(CurrentActorRotation.Roll, DesiredVisualRotation.Roll);

	// [v2.53.0] 최대 지연각으로 제한한 차체 표시 Pitch입니다.
	const float ClampedPitchDeg = CurrentActorRotation.Pitch + FMath::Clamp(PitchLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// [v2.53.0] 최대 지연각으로 제한한 차체 표시 Yaw입니다.
	const float ClampedYawDeg = CurrentActorRotation.Yaw + FMath::Clamp(YawLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// [v2.53.0] 최대 지연각으로 제한한 차체 표시 Roll입니다.
	const float ClampedRollDeg = CurrentActorRotation.Roll + FMath::Clamp(RollLagDeg, -SafeMaxLagDeg, SafeMaxLagDeg);

	// [v2.53.0] 정규화 전 최종 차체 표시 회전입니다.
	FRotator ClampedBodyRotation(ClampedPitchDeg, ClampedYawDeg, ClampedRollDeg);
	ClampedBodyRotation.Normalize();

	return ClampedBodyRotation;
}

void ACFVehiclePawn::DisplayDriveStateOnScreenDebug() const
{
	if ((GetNetMode() == NM_DedicatedServer) || !IsLocallyControlled() || !bEnableVehicleDebugOnScreenMessage || !GEngine)
	{
		return;
	}

		const FString DebugSummary = BuildVehicleDebugSummary(
		DriveStateDebugDisplayMode == ECFVehicleDebugDisplayMode::MultiLine,

		true,
		bShowDriveStateTransitionSummary,
		true);

	GEngine->AddOnScreenDebugMessage(
		reinterpret_cast<uint64>(this),
		DriveStateDebugMessageDuration,
				FColor::Cyan,
		DebugSummary);
}

bool ACFVehiclePawn::ShouldAcceptActionInput(const UInputAction* SourceInputAction, const float CurrentInputValue) const
{
	if (InputDeviceMode == ECFVehicleInputDeviceMode::Auto)
	{
		return true;
	}
	const bool bRequireGamepadKey = (InputDeviceMode == ECFVehicleInputDeviceMode::GamepadOnly);
	if (FMath::Abs(CurrentInputValue) < InputDeviceAnalogThreshold)
	{
		return false;
	}
	return HasActiveMappedKeyForDevice(SourceInputAction, bRequireGamepadKey);
}

bool ACFVehiclePawn::HasActiveMappedKeyForDevice(const UInputAction* SourceInputAction, const bool bRequireGamepadKey) const
{
	if (!SourceInputAction || !DefaultInputMappingContext)
	{
		return false;
	}
	for (const FEnhancedActionKeyMapping& ActionKeyMapping : DefaultInputMappingContext->GetMappings())
	{
		if (ActionKeyMapping.Action != SourceInputAction)
		{
			continue;
		}
		if (ActionKeyMapping.Key.IsGamepadKey() != bRequireGamepadKey)
		{
			continue;
		}
		if (IsMappedKeyCurrentlyActive(ActionKeyMapping.Key))
		{
			return true;
		}
	}
	return false;
}

bool ACFVehiclePawn::IsMappedKeyCurrentlyActive(const FKey& MappingKey) const
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		return false;
	}
	if (PlayerController->IsInputKeyDown(MappingKey))
	{
		return true;
	}
	const float AnalogValue = PlayerController->GetInputAnalogKeyState(MappingKey);
	return FMath::Abs(AnalogValue) >= InputDeviceAnalogThreshold;
}

bool ACFVehiclePawn::IsMeaningfulInputValue(const float CurrentInputValue) const
{
	return FMath::Abs(CurrentInputValue) >= InputDeviceAnalogThreshold;
}

bool ACFVehiclePawn::CanProcessVehicleMoveInput(const float MoveInputMagnitude) const
{
	if (!IsMeaningfulInputValue(MoveInputMagnitude))
	{
		return false;
	}
	if (CurrentInputOwnership != ECFVehicleInputOwnership::LegacyAxis)
	{
		return true;
	}
	if (!GetWorld())
	{
		return true;
	}
	return (GetWorld()->GetTimeSeconds() - LastLegacyAxisInputTimeSec) >= InputOwnershipHoldTimeSec;
}

bool ACFVehiclePawn::CanProcessLegacyAxisInput(const float AxisValue) const
{
	if (!IsMeaningfulInputValue(AxisValue))
	{
		return false;
	}
	if (CurrentInputOwnership != ECFVehicleInputOwnership::VehicleMove2D)
	{
		return true;
	}
	if (!GetWorld())
	{
		return true;
	}
	return (GetWorld()->GetTimeSeconds() - LastVehicleMoveInputTimeSec) >= InputOwnershipHoldTimeSec;
}

void ACFVehiclePawn::UpdateInputOwnershipFromVehicleMove(const float MoveInputMagnitude)
{
	if (!IsMeaningfulInputValue(MoveInputMagnitude))
	{
		return;
	}
	LastVehicleMoveInputTimeSec = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	CurrentInputOwnership = ECFVehicleInputOwnership::VehicleMove2D;
}

void ACFVehiclePawn::UpdateInputOwnershipFromLegacyAxis(const float AxisValue)
{
	if (!IsMeaningfulInputValue(AxisValue))
	{
		return;
	}
	LastLegacyAxisInputTimeSec = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	CurrentInputOwnership = ECFVehicleInputOwnership::LegacyAxis;
}

void ACFVehiclePawn::ReleaseInputOwnershipIfIdle()
{
	if (!GetWorld())
	{
		return;
	}
	const float CurrentTimeSec = GetWorld()->GetTimeSeconds();
	const bool bVehicleMoveExpired = (LastVehicleMoveInputTimeSec < 0.0f) || ((CurrentTimeSec - LastVehicleMoveInputTimeSec) >= InputOwnershipHoldTimeSec);
	const bool bLegacyAxisExpired = (LastLegacyAxisInputTimeSec < 0.0f) || ((CurrentTimeSec - LastLegacyAxisInputTimeSec) >= InputOwnershipHoldTimeSec);
	if (bVehicleMoveExpired && bLegacyAxisExpired)
	{
		CurrentInputOwnership = ECFVehicleInputOwnership::None;
	}
}

float ACFVehiclePawn::CalculateVehicleMoveTargetSteering(const FVector2D& MoveInputVector, const float MoveInputMagnitude) const
{
	if (MoveInputMagnitude < SteeringDirectionMinMagnitude)
	{
		return 0.0f;
	}
	const FVector2D SteeringDirection = MoveInputVector / MoveInputMagnitude;
	return FMath::Clamp(SteeringDirection.X, -1.0f, 1.0f);
}

float ACFVehiclePawn::CalculateSteeringReturnRateKmh(const float SpeedKmh) const
{
	const float AbsoluteSpeedKmh = FMath::Abs(SpeedKmh);
	if (AbsoluteSpeedKmh <= SteeringReturnMinSpeedKmh)
	{
		return 0.0f;
	}

	const float ReturnSpeedRangeKmh = FMath::Max(SteeringReturnMaxSpeedKmh - SteeringReturnMinSpeedKmh, 1.0f);
	const float SpeedAlpha = FMath::Clamp((AbsoluteSpeedKmh - SteeringReturnMinSpeedKmh) / ReturnSpeedRangeKmh, 0.0f, 1.0f);
	return FMath::Lerp(SteeringReturnMinRate, SteeringReturnMaxRate, SpeedAlpha);
}

// [v2.55.0] 속도에 따라 실제 Chaos Vehicle로 전달할 조향 입력을 제한합니다.
float ACFVehiclePawn::CalculateSpeedLimitedSteeringInput(const float RawSteeringInput) const
{
	// [v2.55.0] 입력 경로에서 들어온 원본 조향값을 Chaos 입력 허용 범위로 고정한 값입니다.
	const float ClampedRawSteeringInput = FMath::Clamp(RawSteeringInput, -1.0f, 1.0f);

	if (!bEnableSpeedSteeringLimit || FMath::Abs(ClampedRawSteeringInput) <= KINDA_SMALL_NUMBER)
	{
		return ClampedRawSteeringInput;
	}

	// [v2.55.0] 현재 차량 속도를 얻기 위한 Drive 상태 스냅샷입니다.
	const FCFVehicleDriveStateSnapshot DriveStateSnapshot = GetDriveStateSnapshot();

	// [v2.55.0] 전진과 후진 모두 같은 제한을 적용하기 위한 절대 속도(km/h)입니다.
	const float AbsoluteSpeedKmh = FMath::Abs(DriveStateSnapshot.CurrentSpeedKmh);

	// [v2.55.0] 제한 시작 속도를 음수가 되지 않도록 보정한 값입니다.
	const float LimitStartSpeedKmh = FMath::Max(SpeedSteeringLimitStartSpeedKmh, 0.0f);

	// [v2.55.0] 제한 최대 속도가 시작 속도보다 낮게 설정되지 않도록 보정한 값입니다.
	const float LimitFullSpeedKmh = FMath::Max(SpeedSteeringLimitFullSpeedKmh, LimitStartSpeedKmh + 1.0f);

	if (AbsoluteSpeedKmh <= LimitStartSpeedKmh)
	{
		return ClampedRawSteeringInput;
	}

	// [v2.55.0] 현재 속도가 제한 시작과 최대 제한 사이에서 어느 정도 진행됐는지 나타내는 비율입니다.
	const float SpeedLimitAlpha = FMath::Clamp((AbsoluteSpeedKmh - LimitStartSpeedKmh) / (LimitFullSpeedKmh - LimitStartSpeedKmh), 0.0f, 1.0f);

	// [v2.55.0] 고속 구간에서 허용할 최소 조향 배율입니다.
	const float MinimumSteeringScale = FMath::Clamp(SpeedSteeringLimitMinScale, 0.05f, 1.0f);

	// [v2.55.0] 현재 속도 기준으로 원본 조향 입력에 곱할 최종 배율입니다.
	const float SteeringScale = FMath::Lerp(1.0f, MinimumSteeringScale, SpeedLimitAlpha);

	return FMath::Clamp(ClampedRawSteeringInput * SteeringScale, -1.0f, 1.0f);
}

// [v2.44.0] 현재 입력 경로의 목표 조향값을 제한 속도로 추적해 DriveComp에 적용합니다.
void ACFVehiclePawn::UpdateVehicleMoveSteeringInput(const float DeltaSeconds)
{
	// [v2.44.0] 현재 LegacyAxis 조향을 보간 경로로 처리해야 하는지 여부입니다.
	const bool bUseSmoothedLegacySteering = bSmoothLegacySteeringInput && CurrentInputOwnership == ECFVehicleInputOwnership::LegacyAxis;
	if (CurrentInputOwnership == ECFVehicleInputOwnership::LegacyAxis && !bUseSmoothedLegacySteering)
	{
		CurrentSteeringInput = 0.0f;
		TargetSteeringInput = 0.0f;
		LegacyTargetSteeringInput = 0.0f;
		LastSteeringTurnRate = 0.0f;
		LastSteeringReturnRate = 0.0f;
		bSteeringReturningToCenter = false;
		return;
	}

	// [v2.44.0] 음수 Tick 간격이 들어오지 않도록 보정한 DeltaSeconds입니다.
	const float SafeDeltaSeconds = FMath::Max(DeltaSeconds, 0.0f);

	// [v2.44.0] 현재 입력 경로에서 실제 조향 보간이 따라갈 목표값입니다.
	const float ActiveTargetSteeringInput = bUseSmoothedLegacySteering ? LegacyTargetSteeringInput : TargetSteeringInput;

	// [v2.44.0] 중립이 아닌 조향 목표가 있는지 여부입니다.
	const bool bHasSteeringIntent = FMath::Abs(ActiveTargetSteeringInput) > KINDA_SMALL_NUMBER;
	if (bHasSteeringIntent)
	{
		LastSteeringTurnRate = 2.0f / FMath::Max(SteeringLockToLockTimeSec, 0.01f);
		LastSteeringReturnRate = 0.0f;
		bSteeringReturningToCenter = false;
		CurrentSteeringInput = FMath::FInterpConstantTo(CurrentSteeringInput, ActiveTargetSteeringInput, SafeDeltaSeconds, LastSteeringTurnRate);
	}
	else
	{
		const FCFVehicleDriveStateSnapshot DriveStateSnapshot = GetDriveStateSnapshot();
		LastSteeringTurnRate = 0.0f;
		LastSteeringReturnRate = CalculateSteeringReturnRateKmh(DriveStateSnapshot.CurrentSpeedKmh);
		bSteeringReturningToCenter = true;
		CurrentSteeringInput = FMath::FInterpConstantTo(CurrentSteeringInput, 0.0f, SafeDeltaSeconds, LastSteeringReturnRate);
	}

	CurrentSteeringInput = FMath::Clamp(CurrentSteeringInput, -1.0f, 1.0f);
	SetVehicleSteeringInput(CurrentSteeringInput);
}

// [v1.6.0] 차량 이동용 2D 입력 액션값을 읽어 해석 결과를 Drive 입력으로 전달합니다.
void ACFVehiclePawn::HandleVehicleMoveInput(const FInputActionValue& InputActionValue)
{
	const FVector2D MoveInputVector = InputActionValue.Get<FVector2D>();
	const float MoveInputMagnitude = FMath::Clamp(MoveInputVector.Length(), 0.0f, 1.0f);
	if (!ShouldAcceptActionInput(InputAction_VehicleMove, MoveInputMagnitude) || !CanProcessVehicleMoveInput(MoveInputMagnitude))
	{
		LastVehicleMoveInputResult = FCFVehicleMoveInputResult();
		LastVehicleMoveInputResult.RawMoveInput = MoveInputVector;
		LastVehicleMoveInputResult.Magnitude = MoveInputMagnitude;
		// [v2.8.0] 입력이 최소 기준 아래로 내려가면 목표 조향을 0으로 두고 Tick의 중립 복귀에 맡깁니다.
		TargetSteeringInput = 0.0f;
		ReleaseInputOwnershipIfIdle();
		return;
	}

	UpdateInputOwnershipFromVehicleMove(MoveInputMagnitude);
	LegacyTargetSteeringInput = 0.0f;
	LastVehicleMoveInputResult = ResolveVehicleMoveInput(MoveInputVector);
	ApplyResolvedVehicleMoveInput(LastVehicleMoveInputResult);
}



void ACFVehiclePawn::HandleVehicleMoveReleased(const FInputActionValue&)
{
	LastVehicleMoveInputResult = FCFVehicleMoveInputResult();
	TargetSteeringInput = 0.0f;
	if (CurrentInputOwnership == ECFVehicleInputOwnership::VehicleMove2D)
	{
		ResetAxisInput(&ACFVehiclePawn::SetVehicleThrottleInput);
		ResetAxisInput(&ACFVehiclePawn::SetVehicleBrakeInput);
		// [v2.8.0] 조향은 즉시 0으로 리셋하지 않고 Tick의 속도 기반 중립 복귀에 맡깁니다.
		CurrentInputOwnership = ECFVehicleInputOwnership::None;
	}
	ReleaseInputOwnershipIfIdle();
}


void ACFVehiclePawn::HandleThrottleInput(const FInputActionValue& InputActionValue)
{
	ApplyAxisInputFromAction(InputAction_Throttle, InputActionValue, &ACFVehiclePawn::SetVehicleThrottleInput);
}

void ACFVehiclePawn::HandleThrottleReleased(const FInputActionValue&)
{
	// [v2.44.1] VehicleMove2D가 소유 중이 아닐 때는 소유권이 None이어도 남은 LegacyAxis 스로틀을 반드시 정리합니다.
	const bool bShouldResetLegacyThrottleOnRelease = CurrentInputOwnership != ECFVehicleInputOwnership::VehicleMove2D;
	if (bShouldResetLegacyThrottleOnRelease)
	{
		ResetAxisInput(&ACFVehiclePawn::SetVehicleThrottleInput);
	}
	ReleaseInputOwnershipIfIdle();
}

// [v2.44.0] LegacyAxis 조향 입력을 직접 적용 또는 smoothing 목표값으로 갱신합니다.
void ACFVehiclePawn::HandleSteeringInput(const FInputActionValue& InputActionValue)
{
	// [v2.44.0] 현재 입력 액션에서 읽은 원본 LegacyAxis 조향값입니다.
	const float SteeringAxisValue = InputActionValue.Get<float>();
	if (!ShouldAcceptActionInput(InputAction_Steering, SteeringAxisValue))
	{
		if (CurrentInputOwnership != ECFVehicleInputOwnership::VehicleMove2D)
		{
			if (bSmoothLegacySteeringInput)
			{
				LegacyTargetSteeringInput = 0.0f;
			}
			else
			{
				ResetAxisInput(&ACFVehiclePawn::SetVehicleSteeringInput);
			}
			ReleaseInputOwnershipIfIdle();
		}
		return;
	}
	if (!CanProcessLegacyAxisInput(SteeringAxisValue))
	{
		return;
	}
	UpdateInputOwnershipFromLegacyAxis(SteeringAxisValue);
	if (bSmoothLegacySteeringInput)
	{
		LegacyTargetSteeringInput = FMath::Clamp(SteeringAxisValue, -1.0f, 1.0f);
		return;
	}

	LegacyTargetSteeringInput = 0.0f;
	SetVehicleSteeringInput(SteeringAxisValue);
}

// [v2.44.0] LegacyAxis 조향 해제 시 직접 조향 또는 smoothing 목표값을 중립으로 되돌립니다.
void ACFVehiclePawn::HandleSteeringReleased(const FInputActionValue&)
{
	// [v2.44.1] VehicleMove2D가 소유 중이 아닐 때는 소유권이 None이어도 남은 LegacyAxis 조향을 반드시 정리합니다.
	const bool bShouldResetLegacySteeringOnRelease = CurrentInputOwnership != ECFVehicleInputOwnership::VehicleMove2D;
	if (bShouldResetLegacySteeringOnRelease)
	{
		if (bSmoothLegacySteeringInput)
		{
			LegacyTargetSteeringInput = 0.0f;
		}
		else
		{
			ResetAxisInput(&ACFVehiclePawn::SetVehicleSteeringInput);
		}
	}
	ReleaseInputOwnershipIfIdle();
}

void ACFVehiclePawn::HandleBrakeInput(const FInputActionValue& InputActionValue)
{
	ApplyAxisInputFromAction(InputAction_Brake, InputActionValue, &ACFVehiclePawn::SetVehicleBrakeInput);
}

void ACFVehiclePawn::HandleBrakeReleased(const FInputActionValue&)
{
	// [v2.44.1] VehicleMove2D가 소유 중이 아닐 때는 소유권이 None이어도 남은 LegacyAxis 브레이크/후진 입력을 반드시 정리합니다.
	const bool bShouldResetLegacyBrakeOnRelease = CurrentInputOwnership != ECFVehicleInputOwnership::VehicleMove2D;
	if (bShouldResetLegacyBrakeOnRelease)
	{
		ResetAxisInput(&ACFVehiclePawn::SetVehicleBrakeInput);
	}
	ReleaseInputOwnershipIfIdle();
}


void ACFVehiclePawn::HandleLookInput(const FInputActionValue& InputActionValue)
{
	if (!VehicleCameraComp)
	{
		return;
	}

	const FVector2D LookInputValue = InputActionValue.Get<FVector2D>();
	VehicleCameraComp->SetLookInput(LookInputValue);
}

void ACFVehiclePawn::HandleLookReleased(const FInputActionValue&)
{
	if (!VehicleCameraComp)
	{
		return;
	}

	VehicleCameraComp->ClearLookInput();
}

// [v2.126.0] 현재 Aim 상태를 사용하는 기존 발사 명령 생성 경로를 유지합니다.
FCFVehicleFireRequest ACFVehiclePawn::BuildFireCommand()
{
	return BuildFireCommandForTarget(FVector::ZeroVector, false);
}

// [v2.126.0] 현재 Muzzle을 다시 해결하면서 첫 입력 순간 Command Target을 유지할 후속 발사 명령을 생성합니다.
FCFVehicleFireRequest ACFVehiclePawn::BuildFireCommandForTarget(
	const FVector& OverrideCommandTargetLocation,
	const bool bUseOverrideTarget)
{
	// [v2.63.0] 현재 월드 시간 또는 fallback 0초입니다.
	const float ClientFireTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// [v2.63.0] 이번 로컬 발사 명령에 사용할 요청 ID입니다.
	const int32 FireRequestId = NextFireRequestId++;

	// [v2.63.0] AimComp가 없을 때도 크래시 없이 반환할 fallback 발사 명령입니다.
	FCFVehicleFireRequest FireRequest;
	if (VehicleAimComp)
	{
		FireRequest = VehicleAimComp->BuildFireRequest(FireRequestId, ClientFireTimeSeconds);
	}
	else
	{
		FireRequest.FireRequestId = FireRequestId;
		FireRequest.ClientFireTimeSeconds = ClientFireTimeSeconds;
		FireRequest.AimOrigin = GetActorLocation();
		FireRequest.AimDirection = GetActorForwardVector();
		FireRequest.PredictedAimTargetLocation = GetActorLocation() + GetActorForwardVector() * 1000.0f;
	}

		if (bUseOverrideTarget && !OverrideCommandTargetLocation.ContainsNaN())
	{
		FireRequest.PredictedAimTargetLocation = OverrideCommandTargetLocation;
	}

	if (VehicleWeaponComp)
	{
		// [v2.109.0] Muzzle 기준으로 계산한 공통 Weapon Aim Solution입니다.
		FCFVehicleWeaponAimSolution WeaponAimSolution;

		// [v2.109.0] Weapon Aim Solution과 같은 기준으로 기록할 FireOrigin입니다.
		FCFVehicleFireOrigin FinalFireOrigin;

				// [v2.109.0] Weapon Aim Solution 계산 결과를 WeaponComp 디버그에 남길 요약입니다.
		FString WeaponAimSolutionSummary;

		// [v2.126.0] 후속 Ripple·Salvo 발사에서만 사용할 첫 입력 순간 고정 Command Target 포인터입니다.
		const FVector* OverrideTargetLocation = bUseOverrideTarget
			? &OverrideCommandTargetLocation
			: nullptr;

		if (BuildWeaponAimSolution(WeaponAimSolution, &FinalFireOrigin, &WeaponAimSolutionSummary, OverrideTargetLocation))
		{
			VehicleWeaponComp->RecordResolvedFireOrigin(FinalFireOrigin, WeaponAimSolutionSummary);
			if (VehicleAimComp && !bUseOverrideTarget)
			{
				VehicleAimComp->SetWeaponAimSolution(WeaponAimSolution);
			}

			FireRequest.AimOrigin = WeaponAimSolution.AimOrigin;
			FireRequest.AimDirection = WeaponAimSolution.AimDirection;
						FireRequest.PredictedAimTargetLocation = WeaponAimSolution.AimTargetLocation;
			FireRequest.MuzzleSocketName = FinalFireOrigin.MuzzleSocketName;
			FireRequest.MuzzleSocketIndex = FinalFireOrigin.MuzzleSocketIndex;
			FireRequest.MuzzleSocketCount = FinalFireOrigin.MuzzleSocketCount;
			FireRequest.WeaponGroupId = FinalFireOrigin.MountProfileId;
		}
				else
		{
			if (VehicleAimComp && !bUseOverrideTarget)
			{
				VehicleAimComp->SetWeaponAimSolution(WeaponAimSolution);
			}
			FireRequest.AimDirection = FVector::ZeroVector;
		}
	}

	return FireRequest;
}

// [v2.140.0] 현재 활성 무기가 AMMO-P0-03 SingleCycle 유한탄 Transaction 대상인지 반환합니다.
static bool ShouldUseSingleFireAmmoTransaction(const ACFVehiclePawn* VehiclePawn)
{
	if (!VehiclePawn)
	{
		return false;
	}

	// [v2.140.0] 현재 활성 WeaponData와 Launcher Pattern을 제공하는 Weapon 컴포넌트입니다.
	const UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();
	if (!VehicleWeaponComp)
	{
		return false;
	}

	// [v2.140.0] 유한탄 사용 여부를 판정할 현재 활성 WeaponData입니다.
	const UCFWeaponData* ActiveWeaponData = VehicleWeaponComp->GetActiveWeaponData();
	if (!ActiveWeaponData || !ActiveWeaponData->UsesFiniteAmmoRuntime())
	{
		return false;
	}

	// [v2.140.0] AMMO-P0-03과 AMMO-P0-04의 책임 경계를 구분할 현재 발사 패턴입니다.
	const FCFLauncherFirePatternConfig FirePatternConfig = VehicleWeaponComp->GetActiveLauncherFirePatternConfig();
	return FirePatternConfig.FirePattern == ECFLauncherFirePattern::SingleCycle;
}

// [v2.140.0] 현재 활성 MountProfileId를 Ammo Runtime의 WeaponInstanceId로 반환합니다.
static FName ResolveActiveAmmoWeaponInstanceId(const ACFVehiclePawn* VehiclePawn)
{
	// [v2.140.0] WeaponInstanceId 원본을 제공하는 현재 Weapon 컴포넌트입니다.
	const UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn ? VehiclePawn->GetVehicleWeaponComp() : nullptr;
	return VehicleWeaponComp ? VehicleWeaponComp->GetActiveMountProfileId() : NAME_None;
}

// [v2.140.0] Ammo Transaction 결과를 플레이어 발사 결과의 기존 거부 사유로 변환합니다.
static ECFVehicleFireRejectReason ResolveAmmoTransactionFireRejectReason(const ECFAmmoTransactionResult AmmoTransactionResult)
{
	switch (AmmoTransactionResult)
	{
	case ECFAmmoTransactionResult::Accepted:
		return ECFVehicleFireRejectReason::None;
	case ECFAmmoTransactionResult::Reloading:
		return ECFVehicleFireRejectReason::Reloading;
	case ECFAmmoTransactionResult::SequenceAlreadyActive:
	case ECFAmmoTransactionResult::ActionLocked:
		return ECFVehicleFireRejectReason::WeaponActionLocked;
	case ECFAmmoTransactionResult::ExecutionFailed:
		return ECFVehicleFireRejectReason::InvalidLocalState;
	case ECFAmmoTransactionResult::MissingAmmoData:
	case ECFAmmoTransactionResult::MissingWeaponRuntime:
	case ECFAmmoTransactionResult::InvalidAmmoAmount:
	case ECFAmmoTransactionResult::NotEnoughLoadedAmmo:
	default:
		return ECFVehicleFireRejectReason::NoAmmo;
	}
}

// [v2.140.0] 실패한 SingleCycle 실행의 Ammo 예약을 안전하게 반환합니다.
static void RollbackSingleFireAmmoReservation(ACFVehiclePawn* VehiclePawn, const FName WeaponInstanceId, const bool bAmmoReservationActive)
{
	if (!bAmmoReservationActive || !VehiclePawn)
	{
		return;
	}

	// [v2.140.0] 현재 SingleCycle 예약을 소유하는 VehicleAmmoComp입니다.
	UCFVehicleAmmoComp* VehicleAmmoComp = VehiclePawn->GetVehicleAmmoComp();
	if (VehicleAmmoComp)
	{
		VehicleAmmoComp->RollbackSingleFireAmmo(WeaponInstanceId);
	}
}

// [v2.140.0] 성공한 SingleCycle 실행의 Ammo 예약을 실제 소비로 확정합니다.
static bool CommitSingleFireAmmoReservation(ACFVehiclePawn* VehiclePawn, const FName WeaponInstanceId, const bool bAmmoReservationActive)
{
	if (!bAmmoReservationActive)
	{
		return true;
	}
	if (!VehiclePawn)
	{
		return false;
	}

	// [v2.140.0] 현재 SingleCycle 예약을 실제 소비로 확정할 VehicleAmmoComp입니다.
	UCFVehicleAmmoComp* VehicleAmmoComp = VehiclePawn->GetVehicleAmmoComp();
	return VehicleAmmoComp
		&& VehicleAmmoComp->CommitSingleFireAmmo(WeaponInstanceId) == ECFAmmoTransactionResult::Accepted;
}

// [v2.126.0] 플레이어 입력 발사는 기존 쿨다운을 포함한 전체 검증을 수행합니다.
bool ACFVehiclePawn::ValidateFireCommand(
	const FCFVehicleFireRequest& FireCommand,
	FCFVehicleFireResult& OutFireResult)
{
	return ValidateFireCommandInternal(FireCommand, OutFireResult, false);
}

// [v2.126.0] 후속 Ripple·Salvo 발사에서는 입력 단위 쿨다운만 선택적으로 우회하고 나머지 조건을 동일하게 검증합니다.
bool ACFVehiclePawn::ValidateFireCommandInternal(
	const FCFVehicleFireRequest& FireCommand,
	FCFVehicleFireResult& OutFireResult,
	const bool bIgnoreWeaponCooldown)
{
	OutFireResult = FCFVehicleFireResult();
	OutFireResult.FireRequestId = FireCommand.FireRequestId;
	OutFireResult.ValidationAimTargetLocation = FireCommand.PredictedAimTargetLocation;
	OutFireResult.LocalHitLocation = FireCommand.PredictedAimTargetLocation;

	if (!GetController())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidOwner;
		return false;
	}

	if (!VehicleAimComp)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::NoWeapon;
		return false;
	}

	if (!VehicleWeaponComp)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::NoWeapon;
		return false;
	}

	if (!VehicleAimComp->IsAimRuntimeReady())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::VehicleDisabled;
		return false;
	}

		// [v2.126.0] 일반 입력은 AimComp 캐시를, 후속 시퀀스는 고정 Command Target으로 다시 계산한 최신 Weapon Aim Solution을 사용합니다.
	FCFVehicleWeaponAimSolution WeaponAimSolution;
	if (bIgnoreWeaponCooldown)
	{
		const FVector ScheduledCommandTargetLocation = FVector(FireCommand.PredictedAimTargetLocation);
		if (!BuildWeaponAimSolution(WeaponAimSolution, nullptr, nullptr, &ScheduledCommandTargetLocation))
		{
			OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimDirection;
			return false;
		}
	}
	else
	{
		WeaponAimSolution = VehicleAimComp->GetWeaponAimSolution();
	}

	if (!WeaponAimSolution.bHasValidSolution)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimDirection;
		return false;
	}

	// [v2.63.0] 로컬 검증에 사용할 발사 명령 조준 방향입니다.
	const FVector AimDirection = FVector(FireCommand.AimDirection);
	if (AimDirection.ContainsNaN() || AimDirection.IsNearlyZero())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimDirection;
		return false;
	}

	// [v2.63.0] 로컬 검증에 사용할 발사 명령 조준 시작 위치입니다.
	const FVector AimOrigin = FVector(FireCommand.AimOrigin);
	if (AimOrigin.ContainsNaN())
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimOrigin;
		return false;
	}

	// [v2.63.0] Pawn 위치와 발사 명령 조준 시작점 사이 거리입니다.
	const float AimOriginDistance = FVector::Dist(AimOrigin, GetActorLocation());

	// [v2.63.0] 기본 Aim Profile에서 허용하는 최대 거리입니다.
	const float MaxAimDistance = VehicleAimComp->GetDefaultAimProfile().MaxAimDistance;
	if (AimOriginDistance > MaxAimDistance)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimOrigin;
		return false;
	}

		// [v2.127.0] Direct는 기존 Command Target 방향의 MuzzleBlocked 결과를 사용하고 비Direct는 실제 사출 방향 검사를 Projectile 실행 직전에 수행합니다.
	const FCFLauncherReleaseConfig ActiveReleaseConfig = VehicleWeaponComp->GetActiveLauncherReleaseConfig();
	if (ActiveReleaseConfig.ReleaseMode == ECFProjectileReleaseMode::Direct
		&& WeaponAimSolution.bMuzzleBlocked)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::MuzzleBlocked;
		return false;
	}

	if (!WeaponAimSolution.bAllowFireWhileAligning && WeaponAimSolution.bTurretAligning)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::TurretAligning;
		return false;
	}

	if (!WeaponAimSolution.bAllowFireWhileAligning && WeaponAimSolution.bWeaponNotAligned)
	{
		OutFireResult.RejectReason = ECFVehicleFireRejectReason::WeaponNotAligned;
		return false;
	}

	if (VehicleWeaponComp && VehicleWeaponComp->GetActiveWeaponData())
	{
		// [v2.78.0] 활성 WeaponData가 현재 MountProfile과 호환되는지 여부입니다.
		const bool bActiveWeaponDataCompatible = VehicleWeaponComp->IsActiveWeaponDataCompatible();
		if (!bActiveWeaponDataCompatible)
		{
			OutFireResult.RejectReason = ECFVehicleFireRejectReason::NoWeapon;
			return false;
		}

								// [v2.150.0] 실제 Heat Runtime이 MaxHeat 도달 후 다음 표준 한 발을 위한 여유가 생기기 전인지 여부입니다.
		if (!VehicleWeaponComp->CanActiveWeaponAcceptHeatShot())
		{
			OutFireResult.RejectReason = ECFVehicleFireRejectReason::WeaponOverheated;
			return false;
		}

		// [v2.153.0] 실제 WeaponCharge Runtime이 다음 표준 한 발의 명시 소비량을 감당할 수 있는지 확인합니다.
		if (!VehicleWeaponComp->CanActiveWeaponAcceptChargeShot())
		{
			OutFireResult.RejectReason = ECFVehicleFireRejectReason::WeaponChargeInsufficient;
			return false;
		}

		// [v2.78.0] 이번 발사 명령의 월드 시간입니다.
		const float CurrentFireTimeSeconds = FireCommand.ClientFireTimeSeconds;

		// [v2.78.0] 현재 활성 무기가 아직 쿨다운 중인지 여부입니다.
				const bool bActiveWeaponOnCooldown = !bIgnoreWeaponCooldown
			&& VehicleWeaponComp->IsActiveWeaponOnCooldown(CurrentFireTimeSeconds);
				if (bActiveWeaponOnCooldown)
		{
			OutFireResult.RejectReason = ECFVehicleFireRejectReason::WeaponCooldown;
			return false;
		}
	}

	if (ShouldUseSingleFireAmmoTransaction(this))
	{
		if (!VehicleAmmoComp)
		{
			OutFireResult.RejectReason = ECFVehicleFireRejectReason::NoAmmo;
			return false;
		}

		// [v2.140.0] 현재 MountProfile에 대응하는 Ammo Runtime WeaponInstanceId입니다.
		const FName AmmoWeaponInstanceId = ResolveActiveAmmoWeaponInstanceId(this);

		// [v2.140.0] 실제 발사 실행 전에 장전량·Reload·Action Lock을 확인한 결과입니다.
		const ECFAmmoTransactionResult AmmoValidationResult = VehicleAmmoComp->ValidateSingleFireAmmo(AmmoWeaponInstanceId);
		if (AmmoValidationResult != ECFAmmoTransactionResult::Accepted)
		{
			OutFireResult.RejectReason = ResolveAmmoTransactionFireRejectReason(AmmoValidationResult);
			return false;
		}
	}

	OutFireResult.bAccepted = true;
	OutFireResult.RejectReason = ECFVehicleFireRejectReason::None;
	if (ShouldUseProjectileActorFire())
	{
		// [v2.81.0] Projectile 모드에서 예측 표시용으로 사용할 최대 비행 거리입니다.
		const float ProjectilePredictionDistance = VehicleWeaponComp->GetActiveWeaponMaxRange(MaxAimDistance);

		// [v2.81.0] Projectile 모드에서 즉시 HitScan 없이 표시할 예측 위치입니다.
		const FVector ProjectilePredictionLocation = AimOrigin + AimDirection.GetSafeNormal() * ProjectilePredictionDistance;

		OutFireResult.ValidationAimTargetLocation = ProjectilePredictionLocation;
		OutFireResult.LocalHitLocation = ProjectilePredictionLocation;
		OutFireResult.LocalHitNormal = FVector::UpVector;
		return true;
	}

		// [v2.140.0] HitScan도 검증 단계에서는 실제 Trace·Damage를 실행하지 않고 ExecuteAcceptedFireCommand에서만 실행합니다.
	return true;
}

// [v2.97.0] 싱글플레이 로컬 더미 HitScan Trace를 실행하고 FireResult와 Damage HitContext Debug에 결과를 채웁니다.
bool ACFVehiclePawn::RunLocalDummyHitScan(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& InOutFireResult)
{
	if (!VehicleAimComp)
	{
		return false;
	}

	// [v2.63.0] 로컬 Trace에 사용할 월드입니다.
	UWorld* World = GetWorld();

	// [v2.63.0] 로컬 Trace 시작 위치입니다.
	const FVector TraceStart = FVector(FireCommand.AimOrigin);

	// [v2.63.0] 로컬 Trace 방향입니다.
	const FVector TraceDirection = FVector(FireCommand.AimDirection).GetSafeNormal();

	// [v2.78.0] WeaponData가 없을 때 사용할 기존 Aim Profile 최대 거리입니다.
	const float FallbackTraceDistance = VehicleAimComp->GetDefaultAimProfile().MaxAimDistance;

	// [v2.78.0] 로컬 Trace에 사용할 최종 최대 거리입니다.
	const float TraceDistance = VehicleWeaponComp ? VehicleWeaponComp->GetActiveWeaponMaxRange(FallbackTraceDistance) : FallbackTraceDistance;

	// [v2.63.0] 로컬 Trace 종료 위치입니다.
	const FVector TraceEnd = TraceStart + TraceDirection * TraceDistance;

	if (!World || TraceDirection.IsNearlyZero())
	{
		InOutFireResult.ValidationAimTargetLocation = TraceEnd;
		InOutFireResult.LocalHitLocation = TraceEnd;
		InOutFireResult.LocalHitNormal = FVector::UpVector;
		RecordDummyHitScanDamageHitContext(FireCommand, InOutFireResult, nullptr, false);
		return false;
	}

	// [v2.63.0] 로컬 Trace 적중 결과입니다.
	FHitResult HitResult;

	// [v2.63.0] 로컬 Trace에서 Owner Pawn을 무시하기 위한 쿼리 설정입니다.
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CarFightLocalAimTrace), false);
	QueryParams.AddIgnoredActor(this);
	if (ProjectilePoolComp)
	{
		ProjectilePoolComp->AddActiveSourceProjectilesToQueryParams(this, QueryParams);
	}

	// [v2.108.0] 로컬 Trace가 무기 피격 전용 채널에 적중했는지 여부입니다.
	const bool bHit = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, CFCollisionChannels::WeaponHit, QueryParams);

	if (bHit)
	{
		InOutFireResult.ValidationAimTargetLocation = HitResult.ImpactPoint;
		InOutFireResult.LocalHitLocation = HitResult.ImpactPoint;
		InOutFireResult.LocalHitNormal = HitResult.ImpactNormal;
	}
	else
	{
		InOutFireResult.ValidationAimTargetLocation = TraceEnd;
		InOutFireResult.LocalHitLocation = TraceEnd;
		InOutFireResult.LocalHitNormal = FVector::UpVector;
	}

	RecordDummyHitScanDamageHitContext(FireCommand, InOutFireResult, bHit ? &HitResult : nullptr, bHit);

	if (bDrawLocalAimTraceDebug)
	{
		// [v2.63.0] 로컬 Trace 디버그 라인 색상입니다.
		const FColor TraceColor = bHit ? FColor::Green : FColor::Red;

		DrawDebugLine(World, TraceStart, bHit ? HitResult.ImpactPoint : TraceEnd, TraceColor, false, LocalAimTraceDebugDuration, 0, 2.0f);
		if (bHit)
		{
			DrawDebugSphere(World, HitResult.ImpactPoint, 24.0f, 12, FColor::Yellow, false, LocalAimTraceDebugDuration);
		}
	}

	return bHit;
}

// [v2.97.0] Dummy HitScan 결과를 Damage HitContext Debug로 기록합니다.
void ACFVehiclePawn::RecordDummyHitScanDamageHitContext(
	const FCFVehicleFireRequest& FireCommand,
	const FCFVehicleFireResult& FireResult,
	const FHitResult* HitResult,
	const bool bBlockingHit)
{
	// [v2.97.0] 현재 활성 DamageData입니다.
	UCFDamageData* ActiveDamageData = VehicleWeaponComp ? VehicleWeaponComp->GetActiveDamageData() : nullptr;

	// [v2.97.0] 현재 활성 DamageId 또는 fallback DamageProfileId입니다.
	const FName ActiveDamageId = VehicleWeaponComp ? VehicleWeaponComp->GetActiveDamageId() : NAME_None;

	// [v2.97.0] 현재 활성 WeaponData입니다.
	UCFWeaponData* ActiveWeaponData = VehicleWeaponComp ? VehicleWeaponComp->GetActiveWeaponData() : nullptr;

	// [v2.97.0] 현재 활성 ProjectileData입니다.
	UCFProjectileData* ActiveProjectileData = VehicleWeaponComp ? VehicleWeaponComp->GetActiveProjectileData() : nullptr;

	// [v2.97.0] 기록할 Damage HitContext입니다.
	FCFDamageHitContext DamageHitContext;
	DamageHitContext.DamageData = ActiveDamageData;
	DamageHitContext.DamageId = ActiveDamageData ? ActiveDamageData->DamageId : ActiveDamageId;
	DamageHitContext.WeaponId = ActiveWeaponData ? ActiveWeaponData->WeaponId : FireCommand.WeaponGroupId;
	DamageHitContext.ProjectileId = ActiveProjectileData ? ActiveProjectileData->ProjectileId : (ActiveWeaponData ? ActiveWeaponData->ProjectileDataId : NAME_None);
	DamageHitContext.HitActor = (bBlockingHit && HitResult) ? HitResult->GetActor() : nullptr;
	DamageHitContext.HitComponentName = (bBlockingHit && HitResult && HitResult->GetComponent()) ? HitResult->GetComponent()->GetFName() : NAME_None;
	DamageHitContext.ImpactLocation = FireResult.LocalHitLocation;
	DamageHitContext.ImpactNormal = FireResult.LocalHitNormal.GetSafeNormal();
	if (DamageHitContext.ImpactNormal.IsNearlyZero())
	{
		DamageHitContext.ImpactNormal = FVector::UpVector;
	}
	DamageHitContext.IncomingDirection = FVector(FireCommand.AimDirection).GetSafeNormal();
	if (DamageHitContext.IncomingDirection.IsNearlyZero())
	{
		DamageHitContext.IncomingDirection = GetActorForwardVector().GetSafeNormal();
	}
	if (DamageHitContext.IncomingDirection.IsNearlyZero())
	{
		DamageHitContext.IncomingDirection = FVector::ForwardVector;
	}
	DamageHitContext.InstigatorActor = this;
	DamageHitContext.FlightDurationSeconds = 0.0f;
		DamageHitContext.bFromProjectileActor = false;
	DamageHitContext.bBlockingHit = bBlockingHit;

			// [v2.129.0] HitScan DamageHitContext를 정식 방어층에 적용한 전체 결과입니다.
	FCFVehicleDamageResult VehicleDamageResult;

	// [v2.111.0] 기존 VehicleHealth Debug와 Pool 호환에 사용할 차량 내구도 적용 결과입니다.
	FCFDamageApplyResult DamageApplyResult;

	// [v2.128.0] 다른 차량 Projectile 적중을 차량 피해가 아닌 Projectile 요격 경로로 처리했는지 여부입니다.
	bool bProjectileIntercepted = false;
	if (bBlockingHit && HitResult)
	{
		if (ACFProjectileActor* HitProjectileActor = Cast<ACFProjectileActor>(HitResult->GetActor()))
		{
			bProjectileIntercepted = HitProjectileActor->TryResolveProjectileInterception(this, this, *HitResult);
		}
	}

	if (CombatFxComp && ActiveProjectileData)
	{
		CombatFxComp->PlayImpactFx(ActiveProjectileData->DefaultImpactFxData, DamageHitContext);
	}

		if (!bProjectileIntercepted)
	{
		UCFVehicleDefenseComp::TryApplyDamageToActor(DamageHitContext, VehicleDamageResult);
		DamageApplyResult = UCFVehicleDefenseComp::BuildIntegrityCompatibilityResult(VehicleDamageResult);
	}

	StoreLastDamageHitContext(DamageHitContext);
	StoreLastDamageApplyResult(DamageApplyResult);
}

// [v2.97.0] Projectile Pool에서 반환된 Hit 발사체를 Damage HitContext Debug로 기록합니다.
void ACFVehiclePawn::RecordProjectileDamageHitContextFromPool(const ACFProjectileActor* InProjectileActor)
{
	if (!InProjectileActor || !InProjectileActor->HasLastDamageHitContext())
	{
		return;
	}

	// [v2.111.0] Projectile Actor의 첫 유효 Impact에서 이미 생성된 Damage HitContext 복사본입니다.
	FCFDamageHitContext DamageHitContext = InProjectileActor->GetLastDamageHitContext();

		// [v2.128.0] Projectile의 실제 비활성화 사유입니다.
	const ECFProjectileDeactivateReason ProjectileDeactivateReason = InProjectileActor->GetLastDeactivateReason();

	// [v2.111.0] 일반 World·차량 Hit에서만 누락된 WeaponId를 현재 활성 WeaponData로 보완합니다.
	UCFWeaponData* ActiveWeaponData = VehicleWeaponComp ? VehicleWeaponComp->GetActiveWeaponData() : nullptr;
	if (ProjectileDeactivateReason == ECFProjectileDeactivateReason::Hit
		&& DamageHitContext.WeaponId.IsNone()
		&& ActiveWeaponData)
	{
		DamageHitContext.WeaponId = ActiveWeaponData->WeaponId;
	}

	// [v2.111.0] Projectile Actor가 첫 Impact에서 이미 실행한 직접 피해 적용 결과 복사본입니다.
	FCFDamageApplyResult DamageApplyResult = InProjectileActor->GetLastDamageApplyResult();

	UCFProjectileData* ImpactProjectileData = InProjectileActor->GetLastDeactivatedProjectileData();
	const bool bShouldPlayProjectileImpactFx = ImpactProjectileData
		&& (ProjectileDeactivateReason == ECFProjectileDeactivateReason::Hit
			|| (ProjectileDeactivateReason == ECFProjectileDeactivateReason::Intercepted
				&& ImpactProjectileData->bDetonateWhenIntercepted));
	if (CombatFxComp && bShouldPlayProjectileImpactFx)
	{
		CombatFxComp->PlayImpactFx(ImpactProjectileData->DefaultImpactFxData, DamageHitContext);
	}

	StoreLastDamageHitContext(DamageHitContext);
	StoreLastDamageApplyResult(DamageApplyResult);
}

// [v2.97.0] 마지막 Damage HitContext Debug와 표시 요약을 저장합니다.
void ACFVehiclePawn::StoreLastDamageHitContext(const FCFDamageHitContext& InDamageHitContext)
{
	LastDamageHitContext = InDamageHitContext;
	bHasLastDamageHitContext = true;
	LastDamageHitContextSummary = BuildDamageHitContextSummary(LastDamageHitContext);
}

// [v2.111.0] 마지막 피해 적용 결과와 표시 요약을 저장합니다.
void ACFVehiclePawn::StoreLastDamageApplyResult(FCFDamageApplyResult InDamageApplyResult)
{
	LastDamageApplyResult = MoveTemp(InDamageApplyResult);
	bHasLastDamageApplyResult = true;
	LastDamageApplyResultSummary = UCFVehicleHealthComp::BuildDamageApplyResultSummary(LastDamageApplyResult);
}

// [v2.97.0] VehicleDebug Panel에 표시할 Damage HitContext 요약 문자열을 생성합니다.
FString ACFVehiclePawn::BuildDamageHitContextSummary(const FCFDamageHitContext& InDamageHitContext) const
{
	// [v2.97.0] HitContext 소스 경로를 표시할 문자열입니다.
	const FString SourceText = InDamageHitContext.bFromProjectileActor ? TEXT("ProjectileActor") : TEXT("DummyHitScan");

	// [v2.97.0] 실제 적중 여부를 표시할 문자열입니다.
	const FString HitText = InDamageHitContext.bBlockingHit ? TEXT("Yes") : TEXT("No");

	// [v2.97.0] 맞은 Actor 이름입니다.
	const FString HitActorName = InDamageHitContext.HitActor ? InDamageHitContext.HitActor->GetName() : TEXT("None");

	// [v2.108.0] 실제 피격 컴포넌트 이름입니다.
	const FString HitComponentName = InDamageHitContext.HitComponentName.IsNone() ? TEXT("None") : InDamageHitContext.HitComponentName.ToString();

	// [v2.97.0] 발사 주체 Actor 이름입니다.
	const FString InstigatorActorName = InDamageHitContext.InstigatorActor ? InDamageHitContext.InstigatorActor->GetName() : TEXT("None");

	return FString::Printf(
		TEXT("DamageHitContext: Source=%s, Hit=%s, DamageId=%s, Weapon=%s, Projectile=%s, HitActor=%s, HitComponent=%s, Instigator=%s, Location=(%.1f, %.1f, %.1f), Normal=(%.2f, %.2f, %.2f), Incoming=(%.2f, %.2f, %.2f), Flight=%.2fs"),
		*SourceText,
		*HitText,
		*InDamageHitContext.DamageId.ToString(),
		*InDamageHitContext.WeaponId.ToString(),
		*InDamageHitContext.ProjectileId.ToString(),
		*HitActorName,
		*HitComponentName,
		*InstigatorActorName,
		InDamageHitContext.ImpactLocation.X,
		InDamageHitContext.ImpactLocation.Y,
		InDamageHitContext.ImpactLocation.Z,
		InDamageHitContext.ImpactNormal.X,
		InDamageHitContext.ImpactNormal.Y,
		InDamageHitContext.ImpactNormal.Z,
		InDamageHitContext.IncomingDirection.X,
		InDamageHitContext.IncomingDirection.Y,
		InDamageHitContext.IncomingDirection.Z,
		InDamageHitContext.FlightDurationSeconds);
}

// [v2.125.0] Turret Pitch 메쉬에서 현재 SingleCycle 순서에 맞는 Muzzle 소켓으로 최종 FireOrigin을 보정합니다.
bool ACFVehiclePawn::TryBuildMuzzleFireOrigin(FCFVehicleFireOrigin& InOutFireOrigin, FString& OutFireOriginSummary) const
{
	OutFireOriginSummary = FString();

	if (!LastTurretMountData || !TurretPitchMeshComp || !TurretPitchMeshComp->GetStaticMesh())
	{
		return false;
	}

	// [v2.125.0] 신규 가변 Muzzle 배열입니다. 비어 있으면 기존 단일 MuzzleSocketName을 사용합니다.
	const TArray<FName>& ConfiguredMuzzleSocketNames = LastTurretMountData->MuzzleSocketNames;
	const bool bUsingLegacySingleMuzzle = ConfiguredMuzzleSocketNames.IsEmpty();
	const int32 MuzzleSocketCount = bUsingLegacySingleMuzzle ? 1 : ConfiguredMuzzleSocketNames.Num();
	if (MuzzleSocketCount <= 0)
	{
		return false;
	}

	if (!bUsingLegacySingleMuzzle && LastTurretMountData->bRequireAllMuzzles)
	{
		for (const FName RequiredMuzzleSocketName : ConfiguredMuzzleSocketNames)
		{
			if (RequiredMuzzleSocketName.IsNone() || !TurretPitchMeshComp->DoesSocketExist(RequiredMuzzleSocketName))
			{
				return false;
			}
		}
	}

	// [v2.125.0] 다음 승인 발사 전까지 유지되는 SingleCycle 검색 시작 인덱스입니다.
	const int32 RequestedStartIndex = VehicleWeaponComp
		? VehicleWeaponComp->GetNextMuzzleSocketIndex()
		: 0;
	const int32 SafeStartIndex = FMath::Clamp(RequestedStartIndex, 0, MuzzleSocketCount - 1);

	// [v2.125.0] 실제 Pitch 메쉬에 존재해 이번 FireOrigin에 사용할 Muzzle 소켓입니다.
	FName SelectedMuzzleSocketName = NAME_None;
	int32 SelectedMuzzleSocketIndex = INDEX_NONE;
	for (int32 SearchOffset = 0; SearchOffset < MuzzleSocketCount; ++SearchOffset)
	{
		const int32 CandidateMuzzleSocketIndex = (SafeStartIndex + SearchOffset) % MuzzleSocketCount;
		const FName CandidateMuzzleSocketName = bUsingLegacySingleMuzzle
			? LastTurretMountData->MuzzleSocketName
			: ConfiguredMuzzleSocketNames[CandidateMuzzleSocketIndex];

		if (!CandidateMuzzleSocketName.IsNone() && TurretPitchMeshComp->DoesSocketExist(CandidateMuzzleSocketName))
		{
			SelectedMuzzleSocketName = CandidateMuzzleSocketName;
			SelectedMuzzleSocketIndex = CandidateMuzzleSocketIndex;
			break;
		}
	}

	if (SelectedMuzzleSocketName.IsNone() || SelectedMuzzleSocketIndex == INDEX_NONE)
	{
		return false;
	}

	// [v2.125.0] 선택된 Pitch 메쉬 Muzzle 소켓의 월드 Transform입니다.
	const FTransform MuzzleSocketWorldTransform = TurretPitchMeshComp->GetSocketTransform(SelectedMuzzleSocketName, RTS_World);

	// [v2.125.0] 최종 FireOrigin에 사용할 Muzzle 소켓 월드 위치입니다.
	const FVector MuzzleSocketWorldLocation = MuzzleSocketWorldTransform.GetLocation();
	if (MuzzleSocketWorldLocation.ContainsNaN())
	{
		return false;
	}

	// [v2.125.0] 최종 발사 방향을 어느 기준에서 얻었는지 표시할 디버그 문자열입니다.
	FString DirectionSourceText = TEXT("MuzzleSocketX");

	// [v2.125.0] 선택된 Muzzle 소켓의 X축을 기준으로 계산한 월드 발사 방향입니다.
	FVector MuzzleSocketForwardDirection = MuzzleSocketWorldTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
	if (MuzzleSocketForwardDirection.ContainsNaN() || MuzzleSocketForwardDirection.IsNearlyZero())
	{
		MuzzleSocketForwardDirection = ResolveTurretAimWorldDirection().GetSafeNormal();
		DirectionSourceText = TEXT("TurretAimFallback");
	}

	if (MuzzleSocketForwardDirection.ContainsNaN() || MuzzleSocketForwardDirection.IsNearlyZero())
	{
		MuzzleSocketForwardDirection = InOutFireOrigin.WorldFireDirection.GetSafeNormal();
		DirectionSourceText = TEXT("PreviousFireOriginFallback");
	}

	if (MuzzleSocketForwardDirection.ContainsNaN() || MuzzleSocketForwardDirection.IsNearlyZero())
	{
		return false;
	}

	InOutFireOrigin.bResolved = true;
	InOutFireOrigin.WorldFireLocation = MuzzleSocketWorldLocation;
	InOutFireOrigin.WorldFireDirection = MuzzleSocketForwardDirection;
	InOutFireOrigin.MuzzleSocketName = SelectedMuzzleSocketName;
	InOutFireOrigin.MuzzleSocketIndex = SelectedMuzzleSocketIndex;
	InOutFireOrigin.MuzzleSocketCount = MuzzleSocketCount;

	if (VehicleWeaponComp)
	{
		VehicleWeaponComp->RecordResolvedMuzzleSelection(
			SelectedMuzzleSocketName,
			SelectedMuzzleSocketIndex,
			MuzzleSocketCount);
	}

	OutFireOriginSummary = FString::Printf(
		TEXT("VehicleWeaponFireOrigin: Resolved, Source=MuzzleSocket, Mode=%s, Profile=%s, Slot=%s, Muzzle=%s, MuzzleIndex=%d, MuzzleCount=%d, PitchMesh=%s, DirectionSource=%s, Location=(%.1f, %.1f, %.1f), Direction=(%.3f, %.3f, %.3f)"),
		bUsingLegacySingleMuzzle ? TEXT("LegacySingle") : TEXT("ConfiguredArray"),
		*InOutFireOrigin.MountProfileId.ToString(),
		*InOutFireOrigin.LocationSlotId.ToString(),
		*SelectedMuzzleSocketName.ToString(),
		SelectedMuzzleSocketIndex,
		MuzzleSocketCount,
		*TurretPitchMeshComp->GetStaticMesh()->GetName(),
		*DirectionSourceText,
		InOutFireOrigin.WorldFireLocation.X,
		InOutFireOrigin.WorldFireLocation.Y,
		InOutFireOrigin.WorldFireLocation.Z,
		InOutFireOrigin.WorldFireDirection.X,
		InOutFireOrigin.WorldFireDirection.Y,
		InOutFireOrigin.WorldFireDirection.Z);

	return true;
}

// [v2.114.0] 터렛 정책의 최종 발사 방향과 목표 깊이 기준 Weapon Preview World 데이터를 포함한 Weapon Aim Solution을 계산합니다.
bool ACFVehiclePawn::BuildWeaponAimSolution(
	FCFVehicleWeaponAimSolution& OutWeaponAimSolution,
	FCFVehicleFireOrigin* OutFireOrigin,
	FString* OutFireOriginSummary,
	const FVector* OverrideAimTargetLocation) const
{
	OutWeaponAimSolution = FCFVehicleWeaponAimSolution();

	if (OutFireOrigin)
	{
		*OutFireOrigin = FCFVehicleFireOrigin();
	}

	if (OutFireOriginSummary)
	{
		*OutFireOriginSummary = FString();
	}

	if (!VehicleAimComp || !VehicleWeaponComp)
	{
		return false;
	}

	// [v2.109.0] AimComp가 보유한 현재 로컬 조준 상태입니다.
	const FCFVehicleLocalAimState LocalAimState = VehicleAimComp->GetLocalAimState();

	// [v2.114.0] Camera Runtime에서 읽은 현재 Aim Trace 상태입니다.
	const FCFVehicleCameraRuntimeState CameraRuntimeState = VehicleCameraComp
		? VehicleCameraComp->GetCameraRuntimeState()
		: FCFVehicleCameraRuntimeState();

		// [v2.126.0] 고정 Volley 목표에서는 현재 Camera Trace를 다른 목표의 표면 일치 근거로 사용하지 않습니다.
	const bool bAimTraceHasBlockingHit = OverrideAimTargetLocation
		? false
		: (VehicleCameraComp ? CameraRuntimeState.bAimTraceHasBlockingHit : LocalAimState.bLocalAimTraceHasBlockingHit);
	OutWeaponAimSolution.bAimTraceHasBlockingHit = bAimTraceHasBlockingHit;

	// [v2.126.0] 일반 조준에서만 Camera Aim Trace가 선택한 Actor를 Command 목표 표면으로 사용합니다.
	AActor* AimTraceHitActor = OverrideAimTargetLocation
		? nullptr
		: (VehicleCameraComp ? CameraRuntimeState.AimTraceHitActor.Get() : nullptr);

	// [v2.126.0] 일반 발사는 현재 Reticle 목표를, 후속 Ripple·Salvo는 첫 입력 순간 고정 목표를 사용합니다.
	const FVector AimTargetLocation = OverrideAimTargetLocation
		? *OverrideAimTargetLocation
		: LocalAimState.LocalAimTargetLocation;
	if (AimTargetLocation.ContainsNaN() || AimTargetLocation.IsNearlyZero())
	{
		return false;
	}

	// [v2.109.0] Muzzle 계산 전 VehicleWeaponComp가 만든 기본 FireOrigin입니다.
	FCFVehicleFireOrigin ResolvedFireOrigin;
	if (!VehicleWeaponComp->BuildFireOrigin(LocalAimState.LocalAimDirection, ResolvedFireOrigin))
	{
		return false;
	}

	// [v2.109.0] Muzzle 소켓 위치를 반영한 최종 FireOrigin입니다.
	FCFVehicleFireOrigin FinalFireOrigin = ResolvedFireOrigin;

	// [v2.109.0] Muzzle FireOrigin 계산 결과 요약입니다.
	FString MuzzleFireOriginSummary;
	if (!TryBuildMuzzleFireOrigin(FinalFireOrigin, MuzzleFireOriginSummary))
	{
		return false;
	}

	// [v2.109.0] Muzzle에서 Reticle 목표점까지의 원본 방향 벡터입니다.
	const FVector RawDesiredLaunchDirection = AimTargetLocation - FinalFireOrigin.WorldFireLocation;
	if (RawDesiredLaunchDirection.ContainsNaN() || RawDesiredLaunchDirection.IsNearlyZero())
	{
		return false;
	}

	// [v2.110.0] Muzzle에서 Reticle 목표점으로 향하는 요구 발사 방향입니다.
	const FVector DesiredAimDirection = RawDesiredLaunchDirection.GetSafeNormal();

	// [v2.109.0] Muzzle 소켓 X축 기준 현재 포신 방향입니다.
	const FVector CurrentMuzzleDirection = FinalFireOrigin.WorldFireDirection.GetSafeNormal();
	if (CurrentMuzzleDirection.ContainsNaN() || CurrentMuzzleDirection.IsNearlyZero())
	{
		return false;
	}

	// [v2.109.0] 총구 X축과 요구 발사 방향의 내적입니다.
	const float MuzzleAlignmentDot = FMath::Clamp(FVector::DotProduct(CurrentMuzzleDirection, DesiredAimDirection), -1.0f, 1.0f);

	// [v2.109.0] 총구 X축과 요구 발사 방향 사이의 각도 오차입니다.
	const float WeaponAlignmentErrorDeg = FMath::RadiansToDegrees(FMath::Acos(MuzzleAlignmentDot));

	// [v2.109.0] TurretMountData에서 재사용할 안정화 허용 오차입니다.
	const float StabilizationToleranceDeg = LastTurretMountData
		? FMath::Max(0.0f, LastTurretMountData->StabilizationToleranceDeg)
		: 0.0f;

	// [v2.109.0] 현재 터렛 추적 상태입니다.
	const FCFVehicleTurretState CurrentTurretState = VehicleWeaponComp->GetTurretState();

	// [v2.109.0] 터렛이 안정화 대기 중인지 여부입니다.
	const bool bTurretAligning = LastTurretMountData && !CurrentTurretState.bTurretSettled;

	// [v2.109.0] 총구 방향이 요구 발사 방향 허용 오차 밖인지 여부입니다.
	const bool bWeaponNotAligned = WeaponAlignmentErrorDeg > StabilizationToleranceDeg;

	// [v2.110.0] 활성 TurretMountData가 정렬 중 발사를 허용하는지 여부입니다.
	const bool bAllowFireWhileAligning = LastTurretMountData && LastTurretMountData->bAllowFireWhileAligning;

	// [v2.110.0] 터렛 또는 총구가 아직 요구 방향에 정렬 중인지 여부입니다.
	const bool bWeaponIsAligning = bTurretAligning || bWeaponNotAligned;

	// [v2.110.0] BuildFireCommand, HitScan, Projectile이 공통으로 사용할 실제 최종 발사 방향입니다.
	const FVector FinalFireDirection = (bWeaponIsAligning && bAllowFireWhileAligning)
		? CurrentMuzzleDirection
		: DesiredAimDirection;

	// [v2.112.0] 터렛 Reticle과 Weapon Preview가 사용할 명령 목표까지의 거리입니다.
	const float CommandPathDistance = RawDesiredLaunchDirection.Size();

	// [v2.117.0] TurretMountData가 없을 때 사용할 총구 안전 검사 기본 거리입니다.
	const float DefaultMuzzleClearanceDistanceCm = 150.0f;

	// [v2.117.0] 총구 바로 앞에서 발사체가 안전하게 빠져나갈 수 있는지 검사할 최대 거리입니다.
	const float MuzzleClearanceDistanceCm = LastTurretMountData
		? FMath::Max(0.0f, LastTurretMountData->MuzzleClearanceDistanceCm)
		: DefaultMuzzleClearanceDistanceCm;

	// [v2.117.0] Command 목표가 총구 안전 거리보다 가까울 때 목표를 넘겨 검사하지 않도록 제한한 거리입니다.
	const float MuzzleBlockTraceDistance = FMath::Min(CommandPathDistance, MuzzleClearanceDistanceCm);

	// [v2.115.0] 사용자 조준점과 같은 깊이에서 터렛 방향을 비교할 수 있는 거리인지 여부입니다.
	const bool bHasValidTurretReticleDistance = FMath::IsFinite(CommandPathDistance) && CommandPathDistance > UE_KINDA_SMALL_NUMBER;

	// [v2.115.0] CurrentMuzzleDirection을 사용자 조준점과 같은 거리까지 연장한 터렛 조준 월드 지점입니다.
	const FVector TurretReticleWorldLocation = bHasValidTurretReticleDistance
		? FinalFireOrigin.WorldFireLocation + CurrentMuzzleDirection * CommandPathDistance
		: FVector::ZeroVector;

	// [v2.115.0] 터렛 레티클 월드 지점을 UI에 제공할 수 있는지 여부입니다.
	const bool bHasValidTurretReticlePoint = bHasValidTurretReticleDistance && !TurretReticleWorldLocation.ContainsNaN();

	// [v2.112.0] Preview 거리 fallback에 사용할 기본 AimProfile 최대 거리입니다.
	const float FallbackPreviewDistance = FMath::Max(0.0f, VehicleAimComp->GetDefaultAimProfile().MaxAimDistance);

	// [v2.112.0] 현재 활성 WeaponData입니다.
	UCFWeaponData* ActiveWeaponData = VehicleWeaponComp->GetActiveWeaponData();

	// [v2.112.0] 현재 활성 ProjectileData입니다.
	UCFProjectileData* ActiveProjectileData = VehicleWeaponComp->GetActiveProjectileData();

	// [v2.112.0] 활성 WeaponData가 현재 MountProfile에서 호환되는지 여부입니다.
	const bool bActiveWeaponCompatible = ActiveWeaponData && VehicleWeaponComp->IsActiveWeaponDataCompatible();

	// [v2.113.0] 현재 활성 무기가 제공할 Weapon Reticle 월드 데이터의 의미입니다.
	ECFWeaponReticleMode WeaponReticleMode = ECFWeaponReticleMode::Hidden;
	if (bActiveWeaponCompatible)
	{
		if (ActiveWeaponData->FireMode == ECFWeaponFireMode::HitScan)
		{
			WeaponReticleMode = ECFWeaponReticleMode::DirectImpact;
		}
		else if (ActiveWeaponData->FireMode == ECFWeaponFireMode::Projectile && ActiveProjectileData)
		{
			WeaponReticleMode = ActiveProjectileData->bAffectedByGravity
				? ECFWeaponReticleMode::LaunchDirection
				: ECFWeaponReticleMode::DirectImpact;
		}
	}

	// [v2.113.0] 활성 무기 또는 AimProfile fallback에서 얻은 Weapon Reticle Preview 최대 거리입니다.
	const float PreviewMaxRange = WeaponReticleMode != ECFWeaponReticleMode::Hidden
		? VehicleWeaponComp->GetActiveWeaponMaxRange(FallbackPreviewDistance)
		: 0.0f;

	// [v2.112.0] Preview 계산에 사용할 최대 거리가 안전한지 여부입니다.
	const bool bHasValidPreviewRange = FMath::IsFinite(PreviewMaxRange) && PreviewMaxRange > 0.0f;

	// [v2.113.0] DirectImpact만 최대 사거리 Preview를 위해 Trace를 확장합니다.
	const bool bShouldTracePreviewMaxRange = WeaponReticleMode == ECFWeaponReticleMode::DirectImpact && bHasValidPreviewRange;

	// [v2.113.0] MuzzleBlocked와 DirectImpact Preview가 공유할 WeaponHit Trace 거리입니다.
	const float SharedTraceDistance = bShouldTracePreviewMaxRange
		? FMath::Max(CommandPathDistance, PreviewMaxRange)
		: CommandPathDistance;

	// [v2.112.0] 실제 최종 발사 방향을 따라 계산한 공유 WeaponHit Trace 종료 위치입니다.
	const FVector SharedTraceEnd = FinalFireOrigin.WorldFireLocation + FinalFireDirection * SharedTraceDistance;

	// [v2.112.0] MuzzleBlocked와 Preview가 함께 해석할 공유 WeaponHit Trace 결과입니다.
	FHitResult SharedWeaponHitResult;

	// [v2.109.0] Muzzle 앞 장애물 Trace에서 현재 Pawn을 무시하기 위한 쿼리 설정입니다.
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CarFightMuzzleBlockTrace), false);
	QueryParams.AddIgnoredActor(this);

	// [v2.109.0] Muzzle 앞 장애물 Trace에 사용할 월드입니다.
	UWorld* World = GetWorld();

	// [v2.112.0] 공유 WeaponHit Trace가 Blocking Hit을 얻었는지 여부입니다.
	bool bHasSharedWeaponHit = false;

	// [v2.110.0] 실제 최종 발사 경로에서 검사 거리보다 앞에 WeaponHit Blocking Hit이 발생했는지 여부입니다.
	bool bMuzzleBlocked = false;
	if (World && FMath::IsFinite(SharedTraceDistance) && SharedTraceDistance > 0.0f && !SharedTraceEnd.ContainsNaN())
	{
		bHasSharedWeaponHit = World->LineTraceSingleByChannel(
			SharedWeaponHitResult,
			FinalFireOrigin.WorldFireLocation,
			SharedTraceEnd,
			CFCollisionChannels::WeaponHit,
			QueryParams);

		// [v2.114.0] 총구 Trace가 Camera Aim Trace와 같은 목표 Actor를 적중했는지 여부입니다.
		const bool bSharedWeaponHitMatchesAimTarget = bAimTraceHasBlockingHit
			&& IsValid(AimTraceHitActor)
			&& SharedWeaponHitResult.GetActor() == AimTraceHitActor;

		// [v2.116.0] 총구 Trace가 가장 먼저 적중한 Actor입니다.
		AActor* SharedWeaponHitActor = SharedWeaponHitResult.GetActor();

		// [v2.116.0] 첫 적중 Actor가 실제 발사로 피해를 받을 수 있는 차량 표적인지 여부입니다.
		const bool bSharedWeaponHitIsDamageableVehicle = IsValid(SharedWeaponHitActor)
			&& IsValid(SharedWeaponHitActor->FindComponentByClass<UCFVehicleHealthComp>());

		// [v2.117.0] 첫 적중이 총구 안전 검사 거리 안에 있는지 여부입니다.
		const bool bSharedWeaponHitWithinMuzzleClearance = FMath::IsFinite(SharedWeaponHitResult.Distance)
			&& SharedWeaponHitResult.Distance < MuzzleBlockTraceDistance;

		bMuzzleBlocked = bHasSharedWeaponHit
			&& !bSharedWeaponHitMatchesAimTarget
			&& !bSharedWeaponHitIsDamageableVehicle
			&& bSharedWeaponHitWithinMuzzleClearance;
	}

	if (!bHasValidPreviewRange)
	{
		WeaponReticleMode = ECFWeaponReticleMode::Hidden;
	}

	// [v2.113.0] Weapon Reticle Preview가 유효한지 여부입니다.
	bool bHasValidWeaponPreview = WeaponReticleMode != ECFWeaponReticleMode::Hidden && bHasValidPreviewRange;

	// [v2.112.0] Preview 결과가 최대 거리 안의 Blocking Hit인지 여부입니다.
	bool bWeaponPreviewHasBlockingHit = false;

	// [v2.112.0] Preview가 표시할 월드 위치입니다.
	FVector WeaponPreviewWorldLocation = FVector::ZeroVector;

	// [v2.112.0] Preview 시작점에서 결과 위치까지의 거리입니다.
	float WeaponPreviewDistance = 0.0f;

	if (bHasValidWeaponPreview)
	{
		if (WeaponReticleMode == ECFWeaponReticleMode::DirectImpact
			&& bHasSharedWeaponHit
			&& FMath::IsFinite(SharedWeaponHitResult.Distance)
			&& SharedWeaponHitResult.Distance <= PreviewMaxRange)
		{
			bWeaponPreviewHasBlockingHit = true;
			WeaponPreviewWorldLocation = SharedWeaponHitResult.ImpactPoint;
			WeaponPreviewDistance = FMath::Max(0.0f, SharedWeaponHitResult.Distance);
		}
		else if (WeaponReticleMode == ECFWeaponReticleMode::LaunchDirection)
		{
			// [v2.114.0] 카메라와 총구 시차를 제거하기 위해 실제 발사 방향을 Command 목표와 같은 깊이까지 연장한 위치입니다.
			WeaponPreviewWorldLocation = FinalFireOrigin.WorldFireLocation + FinalFireDirection * CommandPathDistance;
			WeaponPreviewDistance = CommandPathDistance;
			bWeaponPreviewHasBlockingHit = false;
		}
		else
		{
			WeaponPreviewWorldLocation = FinalFireOrigin.WorldFireLocation + FinalFireDirection * PreviewMaxRange;
			WeaponPreviewDistance = PreviewMaxRange;
		}

		if (WeaponPreviewWorldLocation.ContainsNaN() || !FMath::IsFinite(WeaponPreviewDistance) || WeaponPreviewDistance <= 0.0f)
		{
			WeaponReticleMode = ECFWeaponReticleMode::Hidden;
			bHasValidWeaponPreview = false;
			bWeaponPreviewHasBlockingHit = false;
			WeaponPreviewWorldLocation = FVector::ZeroVector;
			WeaponPreviewDistance = 0.0f;
		}
	}

	FinalFireOrigin.WorldFireDirection = FinalFireDirection;

	OutWeaponAimSolution.bHasValidSolution = true;
	OutWeaponAimSolution.bTurretAligning = bTurretAligning;
	OutWeaponAimSolution.bWeaponNotAligned = bWeaponNotAligned;
	OutWeaponAimSolution.bAllowFireWhileAligning = bAllowFireWhileAligning;
	OutWeaponAimSolution.bMuzzleBlocked = bMuzzleBlocked;
	OutWeaponAimSolution.bHasValidTurretReticlePoint = bHasValidTurretReticlePoint;
	OutWeaponAimSolution.TurretReticleWorldLocation = bHasValidTurretReticlePoint ? TurretReticleWorldLocation : FVector::ZeroVector;
	OutWeaponAimSolution.TurretReticleDistance = bHasValidTurretReticlePoint ? CommandPathDistance : 0.0f;
	OutWeaponAimSolution.WeaponReticleMode = WeaponReticleMode;
	OutWeaponAimSolution.bHasValidWeaponPreview = bHasValidWeaponPreview;
	OutWeaponAimSolution.bWeaponPreviewHasBlockingHit = bWeaponPreviewHasBlockingHit;
	OutWeaponAimSolution.WeaponPreviewWorldLocation = WeaponPreviewWorldLocation;
	OutWeaponAimSolution.WeaponPreviewDistance = WeaponPreviewDistance;
	OutWeaponAimSolution.AimOrigin = FinalFireOrigin.WorldFireLocation;
	OutWeaponAimSolution.AimDirection = FinalFireDirection;
	OutWeaponAimSolution.DesiredAimDirection = DesiredAimDirection;
	OutWeaponAimSolution.CurrentMuzzleDirection = CurrentMuzzleDirection;
	OutWeaponAimSolution.AimTargetLocation = AimTargetLocation;
	OutWeaponAimSolution.WeaponAlignmentErrorDeg = WeaponAlignmentErrorDeg;

	if (OutFireOrigin)
	{
		*OutFireOrigin = FinalFireOrigin;
	}

	if (OutFireOriginSummary)
	{
		// [v2.113.0] FireOrigin 요약에 기록할 Weapon Reticle 모드 문자열입니다.
		const TCHAR* WeaponReticleModeText = TEXT("Hidden");
		switch (WeaponReticleMode)
		{
		case ECFWeaponReticleMode::DirectImpact:
			WeaponReticleModeText = TEXT("DirectImpact");
			break;
		case ECFWeaponReticleMode::LaunchDirection:
			WeaponReticleModeText = TEXT("LaunchDirection");
			break;
		case ECFWeaponReticleMode::Hidden:
		default:
			WeaponReticleModeText = TEXT("Hidden");
			break;
		}

		*OutFireOriginSummary = FString::Printf(
			TEXT("%s, AllowFireWhileAligning=%s, DesiredAimDirection=(%.3f, %.3f, %.3f), CurrentMuzzleDirection=(%.3f, %.3f, %.3f), FinalAimDirection=(%.3f, %.3f, %.3f), Target=(%.1f, %.1f, %.1f), AlignmentErrorDeg=%.2f, TurretAligning=%s, WeaponNotAligned=%s, MuzzleBlocked=%s, WeaponReticleMode=%s, WeaponPreviewValid=%s, WeaponPreviewHit=%s, WeaponPreviewLocation=(%.1f, %.1f, %.1f), WeaponPreviewDistance=%.1f"),
			*MuzzleFireOriginSummary,
			bAllowFireWhileAligning ? TEXT("True") : TEXT("False"),
			DesiredAimDirection.X,
			DesiredAimDirection.Y,
			DesiredAimDirection.Z,
			CurrentMuzzleDirection.X,
			CurrentMuzzleDirection.Y,
			CurrentMuzzleDirection.Z,
			FinalFireDirection.X,
			FinalFireDirection.Y,
			FinalFireDirection.Z,
			AimTargetLocation.X,
			AimTargetLocation.Y,
			AimTargetLocation.Z,
			WeaponAlignmentErrorDeg,
			bTurretAligning ? TEXT("Yes") : TEXT("No"),
			bWeaponNotAligned ? TEXT("Yes") : TEXT("No"),
			bMuzzleBlocked ? TEXT("Yes") : TEXT("No"),
			WeaponReticleModeText,
			bHasValidWeaponPreview ? TEXT("Yes") : TEXT("No"),
			bWeaponPreviewHasBlockingHit ? TEXT("Yes") : TEXT("No"),
			WeaponPreviewWorldLocation.X,
			WeaponPreviewWorldLocation.Y,
			WeaponPreviewWorldLocation.Z,
			WeaponPreviewDistance);
	}

	return true;
}

// [v2.109.0] 현재 Weapon Aim Solution을 다시 계산해 AimComp에 저장합니다.
void ACFVehiclePawn::RefreshWeaponAimSolution()
{
	if (!VehicleAimComp)
	{
		return;
	}

	// [v2.109.0] AimComp에 저장할 최신 Weapon Aim Solution입니다.
	FCFVehicleWeaponAimSolution WeaponAimSolution;
	BuildWeaponAimSolution(WeaponAimSolution);
	VehicleAimComp->SetWeaponAimSolution(WeaponAimSolution);
}

// [v2.81.0] 현재 활성 무기가 Projectile Actor 스폰 경로를 사용할 수 있는지 반환합니다.
bool ACFVehiclePawn::ShouldUseProjectileActorFire() const
{
	if (!VehicleWeaponComp)
	{
		return false;
	}

	return VehicleWeaponComp->IsActiveProjectileSpawnReady();
}

// [v2.136.0] WeaponData Release 설정과 명시적 첫 발사 Guidance Target Actor Snapshot으로 Direct·Angled·Vertical Launch Context를 생성합니다.
bool ACFVehiclePawn::BuildDirectProjectileLaunchContext(
	const FCFVehicleFireRequest& FireCommand,
	const UCFProjectileData& InProjectileData,
	AActor* GuidanceTargetActorSnapshot,
	FCFProjectileLaunchContext& OutLaunchContext) const
{
	OutLaunchContext = FCFProjectileLaunchContext();

	// [v2.127.0] Direct 호환 방향과 Muzzle Transform fallback에 사용할 정규화 조준 방향입니다.
	const FVector DirectAimDirection = FVector(FireCommand.AimDirection).GetSafeNormal();
	if (DirectAimDirection.ContainsNaN() || DirectAimDirection.IsNearlyZero())
	{
		return false;
	}

	// [v2.127.0] 실제 선택 Muzzle의 월드 위치입니다.
	const FVector LaunchLocation = FVector(FireCommand.AimOrigin);
	if (LaunchLocation.ContainsNaN())
	{
		return false;
	}

	// [v2.127.0] 실제 Muzzle 소켓 Transform을 우선하고 없으면 Direct Aim 기준 Transform을 사용합니다.
	FTransform MuzzleWorldTransform(DirectAimDirection.Rotation(), LaunchLocation);
	if (TurretPitchMeshComp
		&& !FireCommand.MuzzleSocketName.IsNone()
		&& TurretPitchMeshComp->DoesSocketExist(FireCommand.MuzzleSocketName))
	{
		const FTransform ResolvedMuzzleTransform = TurretPitchMeshComp->GetSocketTransform(FireCommand.MuzzleSocketName, RTS_World);
		if (!ResolvedMuzzleTransform.ContainsNaN())
		{
			MuzzleWorldTransform = ResolvedMuzzleTransform;
			MuzzleWorldTransform.SetLocation(LaunchLocation);
		}
	}

	// [v2.127.0] 활성 WeaponData가 없거나 호환되지 않을 때 Direct 기본값을 유지할 Release 설정입니다.
	const FCFLauncherReleaseConfig ReleaseConfig = VehicleWeaponComp
		? VehicleWeaponComp->GetActiveLauncherReleaseConfig()
		: FCFLauncherReleaseConfig();

	// [v2.127.0] Release Mode에 따라 조준 방향, Muzzle 로컬 방향 또는 Muzzle X축에서 계산한 초기 분리 방향입니다.
	const FVector InitialLaunchDirection = ReleaseConfig.ResolveInitialLaunchDirection(MuzzleWorldTransform, DirectAimDirection);
	if (InitialLaunchDirection.ContainsNaN() || InitialLaunchDirection.IsNearlyZero())
	{
		return false;
	}

	// [v2.127.0] 발사 순간 차량의 월드 Velocity 스냅샷입니다.
	FVector CarrierWorldVelocity = GetVelocity();
	if (CarrierWorldVelocity.ContainsNaN())
	{
		CarrierWorldVelocity = FVector::ZeroVector;
	}

	// [v2.127.0] Launch Context Debug와 후속 비행 계산에 별도로 보존할 차량 속도 상속 성분입니다.
	const FVector InheritedCarrierVelocity = ReleaseConfig.ResolveInheritedCarrierVelocity(CarrierWorldVelocity);

	// [v2.127.0] 사출 속도와 차량 속도 상속을 합친 실제 초기 월드 Velocity입니다.
	const FVector InitialLaunchVelocity = ReleaseConfig.ResolveInitialLaunchVelocity(
		InitialLaunchDirection,
		InProjectileData.InitialSpeed,
		CarrierWorldVelocity);
	if (InitialLaunchVelocity.ContainsNaN() || InitialLaunchVelocity.IsNearlyZero())
	{
		return false;
	}

	// [v2.127.0] 발사 명령 목표가 NaN일 때 초기 방향 앞쪽으로 복구한 안전한 명령 목표입니다.
	FVector SafeCommandTargetLocation = FVector(FireCommand.PredictedAimTargetLocation);
	if (SafeCommandTargetLocation.ContainsNaN())
	{
		SafeCommandTargetLocation = LaunchLocation + InitialLaunchDirection * FMath::Max(InProjectileData.InitialSpeed, 1.0f);
	}

	OutLaunchContext.LaunchTransform = FTransform(InitialLaunchDirection.Rotation(), LaunchLocation);
	OutLaunchContext.InitialLaunchDirection = InitialLaunchDirection;
	OutLaunchContext.InitialLaunchVelocity = InitialLaunchVelocity;
	OutLaunchContext.InheritedCarrierVelocity = InheritedCarrierVelocity;
	OutLaunchContext.CommandTargetLocation = SafeCommandTargetLocation;
	OutLaunchContext.GuidanceTargetActor = GuidanceTargetActorSnapshot;
	OutLaunchContext.MuzzleSocketName = FireCommand.MuzzleSocketName;
	OutLaunchContext.MuzzleSocketIndex = FireCommand.MuzzleSocketIndex;
	OutLaunchContext.MuzzleSocketCount = FireCommand.MuzzleSocketCount;
	OutLaunchContext.ReleaseMode = ReleaseConfig.ReleaseMode;
	OutLaunchContext.FireRequestId = FireCommand.FireRequestId;
	OutLaunchContext.WeaponGroupId = FireCommand.WeaponGroupId;
	return true;
}

// [v2.136.0] 현재 활성 ProjectileData와 현재 선택 목표 Snapshot을 사용해 Projectile Actor를 Pool로 확보하는 호환 경로입니다.
bool ACFVehiclePawn::TrySpawnProjectileActorFromFireCommand(const FCFVehicleFireRequest& FireCommand)
{
	if (!VehicleWeaponComp)
	{
		return false;
	}

	if (!ProjectilePoolComp)
	{
		return false;
	}

	// [v2.81.0] Projectile Actor 스폰에 사용할 활성 ProjectileData입니다.
	UCFProjectileData* ActiveProjectileData = VehicleWeaponComp->GetActiveProjectileData();
	if (!ActiveProjectileData || !ActiveProjectileData->ProjectileActorClass)
	{
		return false;
	}

	// [v2.136.0] 이 호환 단발 호출이 Launch Context에 전달할 현재 선택 목표 Actor Snapshot입니다.
	AActor* GuidanceTargetActorSnapshot = TargetSelectComp
		? TargetSelectComp->GetSelectedTargetActor()
		: nullptr;

	// [v2.136.0] 현재 Release 설정의 방향·속도·차량 속도와 목표 Actor Snapshot을 보존하는 Launch Context입니다.
	FCFProjectileLaunchContext LaunchContext;
	if (!BuildDirectProjectileLaunchContext(FireCommand, *ActiveProjectileData, GuidanceTargetActorSnapshot, LaunchContext))
	{
		return false;
	}

	// [v2.127.0] Pool에서 재사용하거나 새로 확보해 Release-aware Launch Context로 활성화한 공통 Projectile Actor입니다.
	ACFProjectileActor* AcquiredProjectileActor = ProjectilePoolComp->AcquireProjectileWithContext(
		ActiveProjectileData,
		LaunchContext,
		this);

	return AcquiredProjectileActor != nullptr;
}

// [v2.140.0] 검증 승인된 명령을 SingleCycle Ammo Transaction과 함께 Projectile 또는 HitScan 실행 경로로 처리합니다.
bool ACFVehiclePawn::ExecuteAcceptedFireCommand(
	const FCFVehicleFireRequest& FireCommand,
	FCFVehicleFireResult& InOutFireResult,
	AActor* GuidanceTargetActorSnapshot,
	const bool bAllowProjectileFallback)
{
	if (!InOutFireResult.bAccepted)
	{
		return false;
	}

	// [v2.140.0] 이번 실행이 AMMO-P0-03 SingleCycle 유한탄 Transaction을 사용할지 여부입니다.
	const bool bUseSingleFireAmmoTransaction = ShouldUseSingleFireAmmoTransaction(this);

	// [v2.140.0] Ammo Runtime에서 이번 발사 무기를 식별할 현재 MountProfileId입니다.
	const FName AmmoWeaponInstanceId = ResolveActiveAmmoWeaponInstanceId(this);

	// [v2.140.0] 실제 실행 직전 탄약 예약이 성공해 Commit 또는 Rollback이 필요한지 여부입니다.
	bool bAmmoReservationActive = false;
	if (bUseSingleFireAmmoTransaction)
	{
		if (!VehicleAmmoComp)
		{
			InOutFireResult.bAccepted = false;
			InOutFireResult.RejectReason = ECFVehicleFireRejectReason::NoAmmo;
			return false;
		}

		// [v2.140.0] 검증 이후 실행 직전에 다시 원자적으로 확보한 SingleCycle 탄약 예약 결과입니다.
		const ECFAmmoTransactionResult AmmoReservationResult = VehicleAmmoComp->ReserveSingleFireAmmo(AmmoWeaponInstanceId);
		if (AmmoReservationResult != ECFAmmoTransactionResult::Accepted)
		{
			InOutFireResult.bAccepted = false;
			InOutFireResult.RejectReason = ResolveAmmoTransactionFireRejectReason(AmmoReservationResult);
			return false;
		}
		bAmmoReservationActive = true;
	}

	if (!ShouldUseProjectileActorFire())
	{
		// [v2.140.0] HitScan의 실제 Trace·Damage는 검증 이후 정확히 이 실행 단계에서 한 번만 수행합니다.
		RunLocalDummyHitScan(FireCommand, InOutFireResult);

		// [v2.140.0] HitScan은 Blocking Hit 여부와 무관하게 실제 한 발을 발사했으므로 Trace Miss도 탄약을 소비합니다.
		const bool bAmmoCommitted = CommitSingleFireAmmoReservation(this, AmmoWeaponInstanceId, bAmmoReservationActive);
		if (!bAmmoCommitted)
		{
			InOutFireResult.bAccepted = false;
			InOutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
			return false;
		}
		return true;
	}

	// [v2.127.0] 현재 발사 실행에 사용할 활성 ProjectileData입니다.
	UCFProjectileData* ActiveProjectileData = VehicleWeaponComp
		? VehicleWeaponComp->GetActiveProjectileData()
		: nullptr;
	if (!ActiveProjectileData || !ActiveProjectileData->ProjectileActorClass)
	{
		RollbackSingleFireAmmoReservation(this, AmmoWeaponInstanceId, bAmmoReservationActive);
		InOutFireResult.bAccepted = false;
		InOutFireResult.RejectReason = ECFVehicleFireRejectReason::NoWeapon;
		return false;
	}

	// [v2.136.0] Pool Acquire와 사출 안전 검사가 함께 사용할 위치·Actor 목표 Snapshot 포함 Launch Context입니다.
	FCFProjectileLaunchContext LaunchContext;
	if (!BuildDirectProjectileLaunchContext(FireCommand, *ActiveProjectileData, GuidanceTargetActorSnapshot, LaunchContext))
	{
		RollbackSingleFireAmmoReservation(this, AmmoWeaponInstanceId, bAmmoReservationActive);
		InOutFireResult.bAccepted = false;
		InOutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidAimDirection;
		return false;
	}

	// [v2.127.0] Angled·Vertical은 Command Target이 아니라 실제 InitialLaunchDirection 앞쪽의 즉시 장애물만 검사합니다.
	const FCFLauncherReleaseConfig ReleaseConfig = VehicleWeaponComp->GetActiveLauncherReleaseConfig();

	// [v2.127.0] 비Direct 실제 사출 방향의 안전 검사 거리입니다.
	const float ClearanceTraceDistanceCm = ReleaseConfig.GetEffectiveLauncherClearanceTraceDistanceCm();
	if (LaunchContext.ReleaseMode != ECFProjectileReleaseMode::Direct
		&& ClearanceTraceDistanceCm > KINDA_SMALL_NUMBER)
	{
		// [v2.127.0] 비Direct 사출 안전 검사를 시작할 실제 Muzzle 위치입니다.
		const FVector TraceStart = LaunchContext.LaunchTransform.GetLocation();

		// [v2.127.0] 비Direct 사출 안전 검사가 사용할 정규화 초기 분리 방향입니다.
		const FVector TraceDirection = LaunchContext.InitialLaunchDirection.GetSafeNormal();

		// [v2.127.0] 사출 안전 Trace를 실행할 현재 World입니다.
		UWorld* World = GetWorld();
		if (World && !TraceStart.ContainsNaN() && !TraceDirection.ContainsNaN() && !TraceDirection.IsNearlyZero())
		{
			// [v2.127.0] 설정된 안전 검사 거리 앞쪽의 Trace 종료점입니다.
			const FVector TraceEnd = TraceStart + TraceDirection * ClearanceTraceDistanceCm;

			// [v2.127.0] 사출 경로의 첫 Blocking Hit 결과입니다.
			FHitResult ReleaseHitResult;

			// [v2.127.0] 발사 차량 자신을 제외한 사출 경로 Query 설정입니다.
			FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(CarFightLauncherReleaseTrace), false);
			QueryParams.AddIgnoredActor(this);

			// [v2.127.0] 실제 초기 사출 방향 앞쪽이 Blocking Hit으로 막혔는지 여부입니다.
			const bool bReleasePathBlocked = World->LineTraceSingleByChannel(
				ReleaseHitResult,
				TraceStart,
				TraceEnd,
				CFCollisionChannels::WeaponHit,
				QueryParams);
			if (bReleasePathBlocked)
			{
				// [v2.127.0] 사출 경로에서 처음 적중한 Actor입니다.
				AActor* HitActor = ReleaseHitResult.GetActor();

				// [v2.127.0] 적중 Actor가 기존 정책상 정상 타격 대상으로 허용할 피해 가능 차량인지 여부입니다.
				const bool bHitDamageableVehicle = IsValid(HitActor)
					&& IsValid(HitActor->FindComponentByClass<UCFVehicleHealthComp>());
				if (!bHitDamageableVehicle)
				{
					RollbackSingleFireAmmoReservation(this, AmmoWeaponInstanceId, bAmmoReservationActive);
					InOutFireResult.bAccepted = false;
					InOutFireResult.RejectReason = ECFVehicleFireRejectReason::MuzzleBlocked;
					InOutFireResult.ValidationAimTargetLocation = ReleaseHitResult.ImpactPoint;
					InOutFireResult.LocalHitLocation = ReleaseHitResult.ImpactPoint;
					InOutFireResult.LocalHitNormal = ReleaseHitResult.ImpactNormal;
					return false;
				}
			}
		}
	}

	// [v2.127.0] 안전 검사에 사용한 같은 Launch Context 인스턴스를 Pool 활성화에 전달해 방향·목표·차량 속도 Snapshot 불일치를 방지합니다.
	ACFProjectileActor* AcquiredProjectileActor = ProjectilePoolComp
		? ProjectilePoolComp->AcquireProjectileWithContext(ActiveProjectileData, LaunchContext, this)
		: nullptr;
	if (AcquiredProjectileActor)
	{
		// [v2.140.0] Pool에서 실제 Projectile을 확보한 경우에만 예약 탄약을 소비로 확정합니다.
		const bool bAmmoCommitted = CommitSingleFireAmmoReservation(this, AmmoWeaponInstanceId, bAmmoReservationActive);
		if (!bAmmoCommitted)
		{
			InOutFireResult.bAccepted = false;
			InOutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
			return false;
		}
		return true;
	}

	// [v2.140.0] 기존 Direct 첫 발 Pool 실패 fallback은 실제 HitScan 한 발로 성립하므로 성공 시 예약 탄약을 소비합니다.
	if (bAllowProjectileFallback && LaunchContext.ReleaseMode == ECFProjectileReleaseMode::Direct)
	{
		RunLocalDummyHitScan(FireCommand, InOutFireResult);

		// [v2.140.0] Direct fallback HitScan의 실제 발사 성공을 탄약 소비로 확정한 결과입니다.
		const bool bAmmoCommitted = CommitSingleFireAmmoReservation(this, AmmoWeaponInstanceId, bAmmoReservationActive);
		if (!bAmmoCommitted)
		{
			InOutFireResult.bAccepted = false;
			InOutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
			return false;
		}
		return true;
	}

	RollbackSingleFireAmmoReservation(this, AmmoWeaponInstanceId, bAmmoReservationActive);
	InOutFireResult.bAccepted = false;
	InOutFireResult.RejectReason = ECFVehicleFireRejectReason::NoWeapon;
	return false;
}

// [v2.136.0] LauncherComp가 예약한 후속 발사를 첫 발사 순간 위치·Actor Snapshot과 다음 Muzzle로 실행합니다.
bool ACFVehiclePawn::ExecuteScheduledLauncherShot(
	const int32 VolleyId,
	const int32 SequenceShotIndex,
	const FVector& CommandTargetLocation,
	AActor* GuidanceTargetActorSnapshot)
{
	(void)VolleyId;
	(void)SequenceShotIndex;

	// [v2.126.0] 첫 발사 순간 Command Target 위치를 유지하면서 현재 Muzzle을 다시 해결한 후속 발사 요청입니다.
	FCFVehicleFireRequest ScheduledFireRequest = BuildFireCommandForTarget(CommandTargetLocation, true);

	// [v2.126.0] 후속 발사의 검증과 실행 결과입니다.
	FCFVehicleFireResult ScheduledFireResult;

	// [v2.126.0] 후속 발사에서는 입력 단위 Weapon 쿨다운만 우회하고 나머지 조건을 동일하게 검증한 결과입니다.
	const bool bFireCommandAccepted = ValidateFireCommandInternal(ScheduledFireRequest, ScheduledFireResult, true);

	// [v2.136.0] 첫 발사 순간 Guidance Target Actor Snapshot을 현재 TargetSelect 재조회 없이 Launch Context에 전달한 실행 결과입니다.
	const bool bFireCommandExecuted = bFireCommandAccepted
		&& ExecuteAcceptedFireCommand(ScheduledFireRequest, ScheduledFireResult, GuidanceTargetActorSnapshot, false);

	ApplyFireResultInternal(ScheduledFireRequest, ScheduledFireResult, false);
	return bFireCommandExecuted && ScheduledFireResult.bAccepted;
}

// [v2.63.0] 현재 후보 선택 입력을 처리합니다.
void ACFVehiclePawn::HandleSelectTargetStarted(const FInputActionValue&)
{
	ConfirmCurrentTargetCandidate();
}

// [v2.120.0] 현재 선택 대상을 Manual 사유로 해제합니다.
void ACFVehiclePawn::HandleClearTargetStarted(const FInputActionValue&)
{
	ClearSelectedTargetManually();
}

// [v2.152.0] Axis1D의 1-based weapon ordinal 입력을 기존 0-based Weapon Selection Gameplay command로 변환합니다.
void ACFVehiclePawn::HandleSelectWeaponStarted(const FInputActionValue& InputActionValue)
{
	// [v2.152.0] Enhanced Input mapping이 전달한 실제 1-based weapon ordinal 값입니다.
	const float WeaponOrdinalValue = InputActionValue.Get<float>();
	if (!FMath::IsFinite(WeaponOrdinalValue))
	{
		return;
	}

	// [v2.152.0] 정수 ordinal 검증과 RequestSelectWeaponIndex 전달에 사용할 반올림 결과입니다.
	const int32 RequestedWeaponOrdinal = FMath::RoundToInt(WeaponOrdinalValue);
	if (RequestedWeaponOrdinal < 1
		|| RequestedWeaponOrdinal > 9
		|| !FMath::IsNearlyEqual(WeaponOrdinalValue, static_cast<float>(RequestedWeaponOrdinal), KINDA_SMALL_NUMBER))
	{
		return;
	}

	RequestSelectWeaponIndex(RequestedWeaponOrdinal - 1);
}

// [v2.154.0] Axis1D Radar Zoom 부호를 현재 LocalPlayer UISubsystem의 Provider-local Range 요청으로 변환합니다.
void ACFVehiclePawn::HandleRadarZoomStarted(const FInputActionValue& InputActionValue)
{
	// [v2.154.0] Mouse Scroll mapping이 전달한 명시적 +1/-1 Radar Zoom 방향 값입니다.
	const float RadarZoomDirection = InputActionValue.Get<float>();
	if (!FMath::IsFinite(RadarZoomDirection) || FMath::IsNearlyZero(RadarZoomDirection, KINDA_SMALL_NUMBER))
	{
		return;
	}

	// [v2.154.0] 현재 입력 Pawn을 실제로 소유하는 LocalPlayer를 확인할 PlayerController입니다.
	APlayerController* OwningPlayerController = Cast<APlayerController>(GetController());
	if (!OwningPlayerController)
	{
		return;
	}

	// [v2.154.0] Radar Zoom UI state를 소유하는 UISubsystem의 LocalPlayer입니다.
	ULocalPlayer* LocalPlayer = OwningPlayerController->GetLocalPlayer();
	if (!LocalPlayer)
	{
		return;
	}

	// [v2.154.0] 현재 Pawn과 HUDDataProvider 수명을 연결하는 LocalPlayer UI Subsystem입니다.
	UCFUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UCFUISubsystem>();
	if (!UISubsystem || UISubsystem->GetCurrentPawn() != this)
	{
		return;
	}

	if (RadarZoomDirection > 0.0f)
	{
		UISubsystem->RequestRadarZoomIn();
		return;
	}

	UISubsystem->RequestRadarZoomOut();
}

// [v2.146.0] optional Start Active Scan InputAction을 Sensor Gameplay command로 변환합니다.
void ACFVehiclePawn::HandleStartActiveScanStarted(const FInputActionValue&)
{
	RequestStartActiveScan();
}

// [v2.146.0] optional Stop Active Scan InputAction을 Sensor Gameplay command로 변환합니다.
void ACFVehiclePawn::HandleStopActiveScanStarted(const FInputActionValue&)
{
	RequestStopActiveScan();
}

// [v2.141.0] Fire 입력 순간 위치·Actor 목표 Snapshot을 캡처하고 유한탄 Ripple·Salvo는 첫 발 전에 전체 유효 발수를 예약합니다.
void ACFVehiclePawn::HandleFireStarted(const FInputActionValue&)
{
	LastFireRequest = BuildFireCommand();

	// [v2.126.0] 진행 중인 Ripple·Salvo 시퀀스가 새 입력으로 교체되지 않도록 사용할 발사 결과입니다.
	FCFVehicleFireResult FireResult;
	if (LauncherComp && LauncherComp->IsFireSequenceActive())
	{
		FireResult.FireRequestId = LastFireRequest.FireRequestId;
		FireResult.ValidationAimTargetLocation = LastFireRequest.PredictedAimTargetLocation;
		FireResult.LocalHitLocation = LastFireRequest.PredictedAimTargetLocation;
		FireResult.bAccepted = false;

		// [v2.141.0] 유한탄 Sequence Action Lock이 실제로 활성 상태인지 확인할 현재 Ammo WeaponInstanceId입니다.
		const FName ActiveAmmoWeaponInstanceId = ResolveActiveAmmoWeaponInstanceId(this);
		FireResult.RejectReason = VehicleAmmoComp
			&& VehicleAmmoComp->HasActiveLauncherSequenceReservation(ActiveAmmoWeaponInstanceId)
			? ECFVehicleFireRejectReason::WeaponActionLocked
			: ECFVehicleFireRejectReason::WeaponCooldown;
		ApplyFireResultInternal(LastFireRequest, FireResult, false);
		return;
	}

	// [v2.136.0] 첫 Projectile과 같은 Volley의 모든 후속 Projectile이 공유할 발사 순간 선택 목표 Actor Snapshot입니다.
	AActor* GuidanceTargetActorSnapshot = TargetSelectComp
		? TargetSelectComp->GetSelectedTargetActor()
		: nullptr;

	// [v2.141.0] 첫 발 실행 전에 유한탄 예약 수량에 맞게 축소될 수 있는 현재 Launcher 발사 패턴 설정입니다.
	FCFLauncherFirePatternConfig FirePatternConfig = VehicleWeaponComp
		? VehicleWeaponComp->GetActiveLauncherFirePatternConfig()
		: FCFLauncherFirePatternConfig();

	// [v2.141.0] 이번 입력의 유한탄 여부와 부분 시퀀스 정책을 제공할 현재 활성 WeaponData입니다.
	UCFWeaponData* ActiveWeaponData = VehicleWeaponComp
		? VehicleWeaponComp->GetActiveWeaponData()
		: nullptr;

	// [v2.141.0] SingleCycle이 아닌 Ripple·Salvo에서 전체 발수 예약을 사용해야 하는지 여부입니다.
	const bool bUseLauncherAmmoReservation = ActiveWeaponData
		&& ActiveWeaponData->UsesFiniteAmmoRuntime()
		&& FirePatternConfig.FirePattern != ECFLauncherFirePattern::SingleCycle;

	// [v2.141.0] Launcher 예약이 활성화되면 Ammo Runtime에서 현재 무기를 식별할 WeaponInstanceId입니다.
	const FName LauncherAmmoWeaponInstanceId = bUseLauncherAmmoReservation
		? ResolveActiveAmmoWeaponInstanceId(this)
		: NAME_None;

	// [v2.141.0] 첫 발 실행 전에 전체 시퀀스 예약이 성공해 이후 Commit·Release가 필요한지 여부입니다.
	bool bLauncherAmmoReservationActive = false;

	// [v2.126.0] 첫 Projectile은 기존 쿨다운을 포함한 전체 조준·Muzzle·장비 검증을 먼저 수행합니다.
	const bool bFireCommandAccepted = ValidateFireCommand(LastFireRequest, FireResult);
	if (bFireCommandAccepted && bUseLauncherAmmoReservation)
	{
		if (!LauncherComp || !VehicleAmmoComp)
		{
			FireResult.bAccepted = false;
			FireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
		}
		else
		{
			// [v2.141.0] 현재 장전량과 부분 시퀀스 정책으로 실제 실행할 수 있도록 예약된 Launcher 총 발수입니다.
			int32 ReservedLauncherShotCount = 0;

			// [v2.141.0] 첫 발을 포함한 전체 유효 발수의 사전 예약 결과입니다.
			const ECFAmmoTransactionResult LauncherReservationResult = VehicleAmmoComp->ReserveLauncherSequenceAmmo(
				LauncherAmmoWeaponInstanceId,
				FirePatternConfig.GetEffectiveProjectileCount(),
				ActiveWeaponData->bAllowPartialSequence,
				ReservedLauncherShotCount);
			if (LauncherReservationResult == ECFAmmoTransactionResult::Accepted && ReservedLauncherShotCount > 0)
			{
				FirePatternConfig.ProjectileCountPerTrigger = ReservedLauncherShotCount;
				bLauncherAmmoReservationActive = true;
			}
			else
			{
				FireResult.bAccepted = false;
				FireResult.RejectReason = ResolveAmmoTransactionFireRejectReason(LauncherReservationResult);
			}
		}
	}

	if (FireResult.bAccepted)
	{
		ExecuteAcceptedFireCommand(LastFireRequest, FireResult, GuidanceTargetActorSnapshot, true);
	}

	if (bLauncherAmmoReservationActive)
	{
		if (FireResult.bAccepted)
		{
			// [v2.141.0] 실제 성공한 첫 Launcher 발사를 사전 예약량에서 정확히 한 발 소비로 확정한 결과입니다.
			const ECFAmmoTransactionResult FirstShotCommitResult = VehicleAmmoComp->CommitReservedLauncherShot(LauncherAmmoWeaponInstanceId);
			if (FirstShotCommitResult != ECFAmmoTransactionResult::Accepted)
			{
				VehicleAmmoComp->ReleaseLauncherSequenceReservation(LauncherAmmoWeaponInstanceId);
				bLauncherAmmoReservationActive = false;
				FireResult.bAccepted = false;
				FireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
			}
		}
		else
		{
			// [v2.141.0] 첫 발이 실제 실행되지 못했으므로 첫 발 포함 전체 Sequence 예약을 소비 없이 반환합니다.
			VehicleAmmoComp->ReleaseLauncherSequenceReservation(LauncherAmmoWeaponInstanceId);
			bLauncherAmmoReservationActive = false;
		}
	}

	// [v2.126.0] 첫 승인 시점에 쿨다운을 시작하는 정책인지 여부입니다.
	const bool bRecordCooldownOnFirstAcceptedProjectile = !LauncherComp
		|| FirePatternConfig.CooldownStartPolicy == ECFLauncherCooldownStartPolicy::FirstAcceptedProjectile;
	ApplyFireResultInternal(LastFireRequest, FireResult, bRecordCooldownOnFirstAcceptedProjectile);

	if (FireResult.bAccepted && LauncherComp)
	{
		// [v2.141.0] 첫 발사 순간 목표 Snapshot과 남은 발사 및 선택적 유한탄 예약을 LauncherComp에 인계한 결과입니다.
		const bool bSequenceStarted = LauncherComp->StartFireSequenceAfterFirstAcceptedShot(
			FirePatternConfig,
			FVector(LastFireRequest.PredictedAimTargetLocation),
			GuidanceTargetActorSnapshot,
			LastFireRequest.ClientFireTimeSeconds,
			bLauncherAmmoReservationActive ? LauncherAmmoWeaponInstanceId : NAME_None);

		if (!bSequenceStarted && bLauncherAmmoReservationActive && VehicleAmmoComp)
		{
			// [v2.141.0] 첫 발은 이미 소비됐지만 Scheduler가 시작되지 못한 경우 남은 미실행 예약과 Action Lock만 반환합니다.
			VehicleAmmoComp->ReleaseLauncherSequenceReservation(LauncherAmmoWeaponInstanceId);
			bLauncherAmmoReservationActive = false;
		}

		if (!bSequenceStarted
			&& !bRecordCooldownOnFirstAcceptedProjectile
			&& VehicleWeaponComp)
		{
			// 시퀀스 시작 실패에서도 첫 발의 쿨다운이 누락되어 반복 발사가 열리지 않게 보호합니다.
			VehicleWeaponComp->RecordAcceptedFire(LastFireRequest.ClientFireTimeSeconds);
		}
	}
}

// [v2.63.0] 기존 단발 호출자는 승인 발사 시 쿨다운을 즉시 기록하는 호환 경로를 유지합니다.
void ACFVehiclePawn::ApplyFireResult(const FCFVehicleFireRequest& FireCommand, const FCFVehicleFireResult& FireResult)
{
	ApplyFireResultInternal(FireCommand, FireResult, true);
}

// [v2.126.0] 승인 발사 결과의 Muzzle·FX·피드백은 유지하면서 쿨다운 기록 여부만 호출자가 선택하게 합니다.
void ACFVehiclePawn::ApplyFireResultInternal(
	const FCFVehicleFireRequest& FireCommand,
	const FCFVehicleFireResult& FireResult,
	const bool bRecordCooldown)
{
	LastFireRequest = FireCommand;
	LastFireResult = FireResult;

	// [v2.104.0] 마지막 로컬 FireFeedback 표시 시작 시간을 기록합니다.
	LastFireFeedbackStartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0;

		if (FireResult.bAccepted && VehicleWeaponComp)
	{
				// [v2.150.0] SingleCycle과 Ripple·Salvo 후속 발사를 구분하지 않고 실제 승인된 한 발마다 Heat를 정확히 한 번 누적합니다.
		VehicleWeaponComp->RecordAcceptedWeaponShotHeat();

		// [v2.153.0] 실제 승인된 한 발마다 현재 선택 무기의 내부 Charge를 정확히 한 번 소비합니다.
		VehicleWeaponComp->RecordAcceptedWeaponShotCharge();

		if (bRecordCooldown)
		{
			VehicleWeaponComp->RecordAcceptedFire(FireCommand.ClientFireTimeSeconds);
		}
		VehicleWeaponComp->AdvanceMuzzleSequenceAfterAcceptedFire(
			FireCommand.MuzzleSocketName,
			FireCommand.MuzzleSocketIndex,
			FireCommand.MuzzleSocketCount);

		UCFWeaponData* ActiveWeaponData = VehicleWeaponComp->GetActiveWeaponData();
		if (CombatFxComp && ActiveWeaponData)
		{
			CombatFxComp->PlayFireFx(ActiveWeaponData->DefaultFireFxData, FVector(FireCommand.AimOrigin), FVector(FireCommand.AimDirection));
		}
	}

	if (VehicleAimComp)
	{
		VehicleAimComp->BuildFireValidationStateFromFireCommand(FireCommand, FireResult.RejectReason, FireResult.bAccepted);
		VehicleAimComp->ApplyFireValidationResult(FireResult);
		VehicleAimComp->UpdateAimVisualFromFireResult(FireCommand, FireResult);
	}
}

// [v2.104.0] Reticle / FireFeedback UI가 읽을 현재 로컬 발사 피드백 표시 데이터를 만듭니다.
FCFVehicleFireFeedbackViewData ACFVehiclePawn::BuildFireFeedbackViewData() const
{
	// [v2.104.0] Reticle / FireFeedback UI에 반환할 표시 데이터입니다.
	FCFVehicleFireFeedbackViewData ViewData;
	ViewData.LastRejectReason = LastFireResult.RejectReason;

	// [v2.104.0] 현재 월드 시간 또는 fallback 0초입니다.
	const double CurrentTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0;

	// [v2.104.0] 유효한 피드백 시작 시간이 있는지 여부입니다.
	const bool bHasValidFeedbackStartTime = LastFireFeedbackStartTimeSeconds >= 0.0;

	// [v2.104.0] 마지막 피드백 시작 이후 지난 시간입니다.
	const double FeedbackAgeSeconds = bHasValidFeedbackStartTime
		? CurrentTimeSeconds - LastFireFeedbackStartTimeSeconds
		: 1000000.0;

	// [v2.104.0] 현재 활성 무기의 전체 쿨다운 시간입니다.
	const float TotalCooldownSeconds = VehicleWeaponComp
		? VehicleWeaponComp->GetActiveWeaponCooldownSeconds()
		: 0.0f;

	// [v2.104.0] 현재 활성 무기의 남은 쿨다운 시간입니다.
	const float RemainingCooldownSeconds = VehicleWeaponComp
		? VehicleWeaponComp->GetRemainingCooldownSeconds(static_cast<float>(CurrentTimeSeconds))
		: 0.0f;

	ViewData.TotalCooldownSeconds = TotalCooldownSeconds;
	ViewData.RemainingCooldownSeconds = RemainingCooldownSeconds;
	ViewData.bShowCooldown = TotalCooldownSeconds > 0.0f && RemainingCooldownSeconds > 0.0f;
	ViewData.CooldownRatio = ViewData.bShowCooldown
		? FMath::Clamp(RemainingCooldownSeconds / TotalCooldownSeconds, 0.0f, 1.0f)
		: 0.0f;

	if (VehicleAimComp)
	{
		// [v2.104.0] 로컬 플레이어 기준 현재 Aim 상태입니다.
		const FCFVehicleLocalAimState LocalAimState = VehicleAimComp->GetLocalAimState();
		ViewData.bShowOutOfArcWarning = !LocalAimState.bLocalWithinWeaponArc;
	}

	if (LastFireResult.bAccepted)
	{
		// [v2.106.0] 발사 성공 직후에는 쿨다운보다 성공 피드백을 먼저 표시할지 판단합니다.
		const bool bWithinSuccessFeedbackTime = FeedbackAgeSeconds <= FireSuccessFeedbackDurationSeconds;
		if (bWithinSuccessFeedbackTime)
		{
			ViewData.FeedbackState = ECFVehicleFireFeedbackState::FireSuccess;
			ViewData.bFeedbackActive = true;
			ViewData.bOverrideReticleState = false;
			ViewData.FeedbackDisplayKey = TEXT("FireSuccess");
			return ViewData;
		}
	}

	if (ViewData.bShowCooldown)
	{
		// [v2.106.0] 성공 피드백 유지 시간이 끝난 뒤 남은 쿨다운을 주 상태로 표시합니다.
		ViewData.FeedbackState = ECFVehicleFireFeedbackState::Cooldown;
		ViewData.bFeedbackActive = true;
		ViewData.bOverrideReticleState = true;
		ViewData.FeedbackDisplayKey = TEXT("Cooldown");
		return ViewData;
	}

	if (LastFireResult.bAccepted)
	{
		// [v2.106.0] 성공 피드백과 쿨다운이 모두 끝나면 이전 성공 결과를 화면에 남기지 않습니다.
		ViewData.FeedbackState = ECFVehicleFireFeedbackState::None;
		ViewData.bFeedbackActive = false;
		ViewData.bOverrideReticleState = false;
		ViewData.FeedbackDisplayKey = NAME_None;
		return ViewData;
	}

	// [v2.104.0] 발사 실패 피드백 유지 시간 안에 있는지 여부입니다.
	const bool bWithinRejectedFeedbackTime = FeedbackAgeSeconds <= FireRejectedFeedbackDurationSeconds;

	switch (LastFireResult.RejectReason)
	{
	case ECFVehicleFireRejectReason::NoWeapon:
		ViewData.FeedbackState = ECFVehicleFireFeedbackState::NoWeapon;
		ViewData.bFeedbackActive = bWithinRejectedFeedbackTime;
		ViewData.bOverrideReticleState = true;
		ViewData.FeedbackDisplayKey = TEXT("NoWeapon");
		break;

	case ECFVehicleFireRejectReason::WeaponCooldown:
		// [v2.105.1] 마지막 거부 사유가 남아 있어도 실제 쿨다운이 끝나면 피드백을 비활성화합니다.
		ViewData.FeedbackState = ECFVehicleFireFeedbackState::Cooldown;
		ViewData.bFeedbackActive = ViewData.bShowCooldown;
		ViewData.bOverrideReticleState = ViewData.bShowCooldown;
		ViewData.FeedbackDisplayKey = ViewData.bShowCooldown ? FName(TEXT("Cooldown")) : NAME_None;
		break;

	case ECFVehicleFireRejectReason::AimBlocked:
	case ECFVehicleFireRejectReason::MuzzleBlocked:
		ViewData.FeedbackState = ECFVehicleFireFeedbackState::AimBlocked;
		ViewData.bFeedbackActive = bWithinRejectedFeedbackTime;
		ViewData.bOverrideReticleState = true;
		ViewData.FeedbackDisplayKey = TEXT("AimBlocked");
		break;

	case ECFVehicleFireRejectReason::TurretAligning:
		ViewData.FeedbackState = ECFVehicleFireFeedbackState::OutOfArcWarning;
		ViewData.bFeedbackActive = bWithinRejectedFeedbackTime;
		ViewData.bOverrideReticleState = false;
		ViewData.bShowOutOfArcWarning = bWithinRejectedFeedbackTime;
		ViewData.FeedbackDisplayKey = bWithinRejectedFeedbackTime ? FName(TEXT("TurretAligning")) : NAME_None;
		break;

	case ECFVehicleFireRejectReason::WeaponNotAligned:
		ViewData.FeedbackState = ECFVehicleFireFeedbackState::None;
		ViewData.bFeedbackActive = false;
		ViewData.bOverrideReticleState = false;
		ViewData.FeedbackDisplayKey = NAME_None;
		break;

	case ECFVehicleFireRejectReason::OutOfWeaponArc:
		// [v2.107.0] 조준각 경고 텍스트는 발사 실패 피드백 유지 시간 안에서만 활성화합니다.
		ViewData.FeedbackState = ECFVehicleFireFeedbackState::OutOfArcWarning;
		ViewData.bFeedbackActive = bWithinRejectedFeedbackTime;
		ViewData.bOverrideReticleState = false;
		ViewData.bShowOutOfArcWarning = bWithinRejectedFeedbackTime;
		ViewData.FeedbackDisplayKey = bWithinRejectedFeedbackTime ? FName(TEXT("OutOfArcWarning")) : NAME_None;
		break;

	case ECFVehicleFireRejectReason::None:
		ViewData.FeedbackState = ECFVehicleFireFeedbackState::None;
		ViewData.bFeedbackActive = false;
		ViewData.bOverrideReticleState = false;
		ViewData.FeedbackDisplayKey = NAME_None;
		break;

	default:
		ViewData.FeedbackState = ECFVehicleFireFeedbackState::FireRejected;
		ViewData.bFeedbackActive = bWithinRejectedFeedbackTime;
		ViewData.bOverrideReticleState = true;
		ViewData.FeedbackDisplayKey = TEXT("FireRejected");
		break;
	}

	return ViewData;
}

void ACFVehiclePawn::HandleHandbrakeStarted(const FInputActionValue&)
{
	if (!ShouldAcceptActionInput(InputAction_Handbrake, 1.0f))
	{
		SetVehicleHandbrakeInput(false);
		return;
	}
	SetVehicleHandbrakeInput(true);
}

void ACFVehiclePawn::HandleHandbrakeCompleted(const FInputActionValue&)
{
	SetVehicleHandbrakeInput(false);
}
