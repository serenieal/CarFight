// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 2.169.1
// Date: 2026-09-06
// Description: CarFight 싱글플레이 차량 Pawn 구현 / VPS-P0-02 Fire behavior extraction / RTA Fitting-dependent Runtime refresh seam
// Changelog:
// - v2.169.1: VPS-P0-02 Authority 교정으로 FireRequest ID/시간 할당과 입력 순간 LastFireRequest commit을 Pawn에 명시적으로 복귀. FireComp는 전달받은 요청의 계산·검증·실행만 수행.
// - v2.169.0: CF-FQ-048 VPS-P0-02로 Fire Command 생성·검증·Muzzle/Aim glue·HitScan/Projectile 실행·Launcher 후속 발사와 Fire side effect를 UCFVehicleFireComp로 위임. Pawn은 Fire observable/Damage Debug Authority와 기존 Launcher/Automation compatibility facade를 유지.
// - v2.168.0: CF-FQ-048 VPS-P0-01로 차체·휠·레이아웃·터렛·Owner 표시 행동과 순수 시각 캐시를 UCFVehicleVisualComp에 위임. Pawn은 기존 lifecycle 순서, private Automation wrapper, Runtime/Turret observable state Authority와 SM_Body gameplay hit collision을 유지.
// - v2.167.0: RTA-P0-03 장비 hot apply/복구가 전체 차량 재초기화 없이 현재 Applied Fitting의 Ammo·TurretVisual·Launcher를 다시 구성하고 CombatReady를 readback하는 C++ refresh seam을 추가.
// - v2.166.0: VehicleData opt-in EngineTorqueCurve를 공용 validator/mapper로 Chaos EngineSetup에 적용하고, opt-out hot-apply는 Movement archetype의 authored Curve로 복원해 이전 차량 Curve 잔류를 방지.
// - v2.165.0: Wheel Visual authored base를 Location/Rotation/Scale 전체로 캡처해 매 Apply 시작 시 복원하고, OnConstruction은 fresh SCS authored transform을 재캡처하도록 cache invalidation 추가. Legacy/Socket/Manual hot-reinit 잔류 transform을 제거.
// - v2.164.1: Right fallback orientation을 Mesh scale/Legacy AutoCenter보다 먼저 확정해 center correction도 최종 Right orientation 기준으로 계산되게 순서 교정.
// - v2.164.0: FL-only shared Wheel fallback에서 FR/RR Wheel_Mesh에 authored base 기준 local Roll180을 source-aware 적용하고 per-wheel spin handedness를 WheelSync에 전달. 반복 Apply/re-init 누적을 막기 위해 authored base rotation을 1회 캡처.
// - v2.163.0: Guided Builder Step 8처럼 VehicleData를 runtime에서 교체한 뒤 InitializeVehicleRuntime()을 재호출할 때 SM_Body ChassisMesh가 이전 차량으로 남던 문제를 교정. 재초기화 초기에 ApplyVehicleVisualConfig를 명시 호출해 새 VehicleData의 차체 Visual을 Layout/WheelSync보다 먼저 확정.
// - v2.162.0: Legacy TargetSelectWidgetClass의 constructor LoadClass hard-load를 제거해 Reticle/TargetSelect Class 해석을 CFUISubsystem Config Soft Class 단일 소유권으로 정리.
// - v2.161.0: 제거된 WBP_VehicleDebug Text API와 Pawn 직접 소유 Reticle/TargetSelect 호환 wrapper·영구 null instance cache 정리 경로를 제거. HUD/Panel 및 UISubsystem 소유 UI 경로는 변경 없음.
// - v2.160.0: WSA-P0-03 Socket mode에서 Wheel_Mesh_*에 USER RelativeScale을 exact set하고 FR/RL/RR null mesh는 FL을 fallback. Legacy AutoScale과 이중 적용하지 않음.
// - v2.159.0: WSA-P0-02 Pawn legacy socket capture가 component/world scale 대신 underlying UStaticMeshSocket::RelativeScale을 FCFWheelAnchorPose에 보존.
// - v2.158.0: VB-P0-05 설계 검수 교정으로 invalid Chassis/Transmission을 다른 Movement 값보다 먼저 거부해 partial runtime mutation을 제거하고 Shift RPM integer semantic을 runtime에서도 검증.
// - v2.157.0: VehicleMovementConfig의 ChassisWidth와 complete TransmissionSetup을 적용하고 setup-time 값이 live physics와 달라질 때만 선/각속도를 보존한 PhysicsState 재생성을 수행. ReverseGearRatios는 positive magnitude를 그대로 전달하며 음수/0 ratio는 runtime에서도 fail-closed로 거부.
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
// - v2.169.1 FireRequest ID/시간과 LastFireRequest 입력 commit은 다시 Pawn Authority가 직접 수행합니다. Blueprint/Product Asset과 기존 Public/BP/Automation 호출 계약에는 변경이 없습니다.
// - v2.169.0 기존 BP_CFVehiclePawn 계열은 VehicleFireComp 기본 서브오브젝트를 자동 상속합니다. Launcher는 기존 Pawn callback을 계속 사용하고 Fire observable/Damage Debug state는 Pawn에 유지되며 Product Asset 수동 추가/저장은 필요하지 않습니다.
// - v2.168.0 기존 BP_CFVehiclePawn 계열은 VehicleVisualComp 기본 서브오브젝트를 자동 상속합니다. Product Asset 수동 추가/저장은 필요하지 않으며 기존 Public/BP 함수·프로퍼티와 Runtime/Turret observable state는 Pawn에 그대로 유지됩니다.
// - v2.167.0 RefreshFittingDependentRuntime은 현재 Applied Fitting을 이미 Commit한 뒤에만 사용합니다. Drive/Wheel/Health/Fitting을 재초기화하지 않으며 Ammo는 Snapshot의 InitialSortieAmmoLoads 기준으로 재구성하고 임의 탄약 수량을 만들지 않습니다.
// - v2.162.0 AimReticleWidgetClass/TargetSelectWidgetClass와 Legacy ZOrder UPROPERTY는 저장 직렬화 호환을 위해 유지하지만 Pawn runtime은 Class를 hard-load하거나 Legacy ZOrder를 소비하지 않습니다. UI Class와 Layer ZOrder는 CFUISubsystem이 소유합니다.
// - v2.158.0부터 ChassisWidth/ChassisHeight 또는 complete Transmission payload가 invalid이면 ApplyVehicleMovementConfig는 어떤 Movement mutation도 수행하지 않는다. Shift RPM은 Chaos 내부 uint32 의미에 맞는 비음수 정수값이어야 한다.
// - v2.157.0부터 ChassisWidth/ChassisHeight 또는 Transmission setup이 실제 live setup과 달라 PhysicsState 재생성이 필요하면 현재 chassis 선속도/각속도를 복원한다. 새 public hot setter를 만들지 않으며 기존 ApplyVehicleMovementConfig lifecycle 안에서만 수행한다.
// - v2.157.0 ReverseGearRatios는 UE 5.8 Source 계약대로 positive magnitude만 허용한다. invalid 배열은 abs/자동 보정하지 않고 Transmission 적용을 건너뛰며 diagnostic summary를 남긴다.
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
#include "CFVehicleEngineCurveUtils.h"
#include "CFVehicleAimComp.h"
#include "CFVehicleCameraComp.h"
#include "CFVehicleVisualComp.h"
#include "CFVehicleFireComp.h"
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
#include "UI/CFUISubsystem.h"

#include "ChaosVehicleWheel.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
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

		// WSA 타이어 크기 Authority는 Body component/world scale이 아니라 StaticMesh asset에 USER가 작성한 socket local scale입니다.
		const UStaticMeshSocket* BodyMeshSocket = BodyMeshComponent->GetStaticMesh()->FindSocket(BodySocketName);
		if (!BodyMeshSocket)
		{
			AppendWheelLayoutCaptureFailure(InOutFailureSummary, FString::Printf(TEXT("SocketObjectMissing=%s"), *BodySocketName.ToString()));
			return false;
		}
		OutWheelAnchorPose.RelativeScale = BodyMeshSocket->RelativeScale;
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
	// [v2.168.0] 순수 시각 행동과 순수 시각 캐시를 소유하는 내부 coordinator 기본 서브오브젝트입니다.
	VehicleVisualComp = CreateDefaultSubobject<UCFVehicleVisualComp>(TEXT("VehicleVisualComp"));
	// [v2.169.0] Fire 계산/검증/실행 행동만 소유하고 Pawn observable state를 복제하지 않는 내부 coordinator 기본 서브오브젝트입니다.
	VehicleFireComp = CreateDefaultSubobject<UCFVehicleFireComp>(TEXT("VehicleFireComp"));
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
	bShowVehicleDebugEvents = false;
	DriveStateDebugMessageDuration = 0.0f;
	bShowAimReticle = true;

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
	// [v2.165.0] SCS component authored transform이 Construction에서 갱신될 수 있으므로 이전 Wheel Visual cache를 폐기하고 현재 authored 값을 다시 기준으로 삼습니다.
	InvalidateWheelVisualAuthoredBaseTransforms();
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

// [v2.167.0] 현재 Applied Fitting 기준으로 장비 의존 Ammo·TurretVisual·Launcher와 CombatReady만 다시 구성합니다.
bool ACFVehiclePawn::RefreshFittingDependentRuntime()
{
	// [v2.167.0] Fitting Commit이 완료되어 장비 Runtime 입력을 readback할 수 있는지 여부입니다.
	const bool bFittingRuntimeApplied = VehicleFittingComp && VehicleFittingComp->HasAppliedRuntimeInput();
	if (!bFittingRuntimeApplied)
	{
		bVehicleCombatRuntimeReady = false;
		LastVehicleRuntimeSummary = TEXT("FittingDependentRuntime: Failed, AppliedFittingRuntimeMissing");
		return false;
	}

	// [v2.167.0] 이전 장전·예비·예약·Reload 상태가 새 Snapshot에 잔류하지 않게 비울 Ammo Runtime입니다.
	if (VehicleAmmoComp)
	{
		VehicleAmmoComp->ResetAmmoRuntime();
	}

	// [v2.167.0] Ammo 기본 서브오브젝트 존재와 finite Snapshot 초기화 결과를 합친 탄약 준비 상태입니다.
	bool bAmmoReady = VehicleAmmoComp != nullptr;

	// [v2.167.0] 현재 Applied Snapshot에 실제 finite Ammo Runtime이 필요한 무기가 하나라도 있는지 여부입니다.
	bool bFiniteAmmoRuntimeRequired = false;

	// [v2.167.0] 현재 Applied Runtime이 Snapshot 모드인지 여부입니다.
	const bool bHasAppliedFittingSnapshot = VehicleFittingComp->HasAppliedFittingSnapshot();
	if (bHasAppliedFittingSnapshot)
	{
		// [v2.167.0] 새 Ammo/Turret/Launcher를 구성할 현재 Applied Fitting Snapshot입니다.
		const FCFVehicleFittingSnapshot AppliedFittingSnapshot = VehicleFittingComp->GetAppliedFittingSnapshot();

		// [v2.167.0] WeaponInstanceId별 독립 장전 상태를 만들 finite 무기 초기화 입력입니다.
		TArray<FCFWeaponAmmoInitialization> WeaponAmmoInitializations;
		for (const FCFResolvedFittingMount& ResolvedMount : AppliedFittingSnapshot.ResolvedMounts)
		{
			// [v2.167.0] 이 Mount에 Snapshot이 실제 해결한 WeaponData입니다.
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

			// [v2.167.0] 같은 WeaponData를 여러 Mount에 장착해도 Loaded 상태를 독립 소유할 초기화 입력입니다.
			FCFWeaponAmmoInitialization WeaponAmmoInitialization;
			WeaponAmmoInitialization.WeaponInstanceId = ResolvedMount.MountProfileId;
			WeaponAmmoInitialization.WeaponData = ResolvedWeaponData;
			WeaponAmmoInitialization.InitialLoadedAmmoCountOverride = INDEX_NONE;
			WeaponAmmoInitializations.Add(WeaponAmmoInitialization);
		}

		// [v2.167.0] 명시적 출격 탄약 또는 finite WeaponInstance 때문에 실제 Ammo Runtime 구성이 필요한지 여부입니다.
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

	// [v2.167.0] Snapshot 또는 Legacy 복구 후 현재 Weapon Runtime Source에 맞게 단일 활성 Turret Visual을 재구성합니다.
	ApplyVehicleTurretVisualConfig();

	// [v2.167.0] 최종 Weapon Runtime에 Launcher를 다시 연결한 결과입니다.
	const bool bLauncherReady = LauncherComp
		? LauncherComp->InitializeLauncherRuntime(this, VehicleWeaponComp)
		: false;

	// [v2.167.0] 장비 교체 뒤에도 기존 Aim Runtime이 준비 상태인지 readback합니다.
	const bool bAimReady = VehicleAimComp && VehicleAimComp->IsAimRuntimeReady();

	// [v2.167.0] Fitting Commit 결과 Weapon Runtime이 준비 상태인지 readback합니다.
	const bool bWeaponReady = VehicleWeaponComp && VehicleWeaponComp->IsWeaponRuntimeReady();

	// [v2.167.0] 기존 전투 입력 계약에 필요한 TargetSelectComp가 존재하는지 여부입니다.
	const bool bTargetSelectReady = TargetSelectComp != nullptr;

	bVehicleCombatRuntimeReady = bVehicleCoreRuntimeReady
		&& bAimReady
		&& bWeaponReady
		&& bAmmoReady
		&& bLauncherReady
		&& bTargetSelectReady;

	// [v2.167.0] 기존 호환 RuntimeReady는 장비 hot apply에서도 CoreReady와 동일 의미를 유지합니다.
	bVehicleRuntimeReady = bVehicleCoreRuntimeReady;

	// [v2.167.0] 현재 Ammo 상태를 finite 준비/무한탄 호환/실패로 구분한 bounded readback입니다.
	const TCHAR* AmmoRuntimeState = !VehicleAmmoComp
		? TEXT("Missing")
		: (!bAmmoReady
			? TEXT("Failed")
			: (VehicleAmmoComp->IsAmmoRuntimeInitialized() ? TEXT("Ready") : TEXT("InfiniteCompatibility")));

	LastVehicleRuntimeSummary = FString::Printf(
		TEXT("FittingDependentRuntime: Fitting=%s, Aim=%s, Weapon=%s, Ammo=%s, Launcher=%s, TargetSelect=%s, CoreReady=%s, CombatReady=%s | %s | %s"),
		bHasAppliedFittingSnapshot ? TEXT("Snapshot") : TEXT("Legacy"),
		bAimReady ? TEXT("Ready") : TEXT("Missing"),
		bWeaponReady ? TEXT("Ready") : TEXT("Missing"),
		AmmoRuntimeState,
		bLauncherReady ? TEXT("Ready") : TEXT("Missing"),
		bTargetSelectReady ? TEXT("Ready") : TEXT("Missing"),
		bVehicleCoreRuntimeReady ? TEXT("True") : TEXT("False"),
		bVehicleCombatRuntimeReady ? TEXT("True") : TEXT("False"),
		VehicleFittingComp ? *VehicleFittingComp->GetLastFittingRuntimeSummary() : TEXT("FittingRuntime: ComponentMissing"),
		*LastTurretVisualSummary);
	return bVehicleCombatRuntimeReady;
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

	// [v2.163.0] VehicleData 자체가 runtime에서 교체될 수 있으므로 새 ChassisMesh를 Layout/WheelSync보다 먼저 SM_Body에 재적용합니다.
	ApplyVehicleVisualConfig();
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

bool ACFVehiclePawn::ShouldShowTargetSelectHud() const
{
	return bShowTargetSelectHud && GetNetMode() != NM_DedicatedServer && IsLocallyControlled();
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

// [v2.168.0] 차체 mesh 시각 행동은 VisualComp로 위임하고 gameplay hit collision과 TargetPoint 정렬은 Pawn 책임으로 유지합니다.
void ACFVehiclePawn::ApplyVehicleVisualConfig()
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->ApplyVehicleVisualConfig();
	}

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

// [v2.168.0] 터렛 시각 Reset 행동은 VisualComp로 위임하지만 observable state Authority는 Pawn 필드에 유지됩니다.
void ACFVehiclePawn::ResetTurretVisualComponents()
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->ResetTurretVisualComponents();
	}
}

// [v2.168.0] 기존 Pawn private seam을 유지하면서 활성 터렛 MountProfile 탐색을 VisualComp로 위임합니다.
const FCFVehicleMountProfile* ACFVehiclePawn::FindActiveTurretMountProfile() const
{
	return VehicleVisualComp ? VehicleVisualComp->FindActiveTurretMountProfile() : nullptr;
}

// [v2.168.0] 기존 Pawn private seam을 유지하면서 터렛 하드포인트 슬롯 탐색을 VisualComp로 위임합니다.
const FCFVehicleHardpointSlot* ACFVehiclePawn::FindTurretHardpointSlot(const FName LocationSlotId) const
{
	return VehicleVisualComp ? VehicleVisualComp->FindTurretHardpointSlot(LocationSlotId) : nullptr;
}

// [v2.168.0] 기존 Pawn seam과 observable state Authority를 유지하면서 터렛 시각 구성 행동을 VisualComp로 위임합니다.
void ACFVehiclePawn::ApplyVehicleTurretVisualConfig()
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->ApplyVehicleTurretVisualConfig();
	}
}

// [v2.168.0] 기존 Pawn seam을 유지하면서 터렛 요구 조준 월드 방향 계산을 VisualComp로 위임합니다.
FVector ACFVehiclePawn::ResolveTurretAimWorldDirection() const
{
	if (VehicleVisualComp)
	{
		return VehicleVisualComp->ResolveTurretAimWorldDirection();
	}

	// VisualComp가 비정상적으로 없는 경우에도 기존 Actor 정면 fallback을 보존합니다.
	const FVector ActorForwardDirection = GetActorForwardVector().GetSafeNormal();
	return ActorForwardDirection.IsNearlyZero() ? FVector::ForwardVector : ActorForwardDirection;
}

// [v2.168.0] 기존 Tick 호출 순서를 유지하면서 터렛 Yaw/Pitch 시각 갱신을 VisualComp로 위임합니다.
void ACFVehiclePawn::UpdateVehicleTurretAimVisuals(const float DeltaSeconds)
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->UpdateVehicleTurretAimVisuals(DeltaSeconds);
	}
}

// [v2.168.0] 기존 private Automation seam과 Pawn summary Authority를 유지하면서 Wheel Anchor 위치/회전 적용을 VisualComp로 위임합니다.
void ACFVehiclePawn::ApplyVehicleLayoutConfig()
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->ApplyVehicleLayoutConfig();
	}
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
	// [v2.157.0] VehicleData에서 읽은 current complete movement setup입니다.
	const FCFVehicleMovementConfig& VehicleMovementConfig = VehicleData->VehicleMovementConfig;

	// [v2.157.0] Transmission ratio 배열이 positive finite magnitude만 포함하는지 검사합니다.
	const auto AreTransmissionRatiosValid = [](const TArray<float>& Ratios)
	{
		if (Ratios.IsEmpty())
		{
			return false;
		}
		for (const float Ratio : Ratios)
		{
			if (!FMath::IsFinite(Ratio) || Ratio <= 0.0f)
			{
				return false;
			}
		}
		return true;
	};

	// [v2.158.0] Shift RPM이 Chaos 내부 uint32 의미와 같은 비음수 정수값인지 검사합니다.
	const auto IsNonNegativeIntegerRpm = [](const float RpmValue)
	{
		return FMath::IsFinite(RpmValue)
			&& RpmValue >= 0.0f
			&& FMath::IsNearlyEqual(RpmValue, FMath::RoundToFloat(RpmValue));
	};

	// [v2.158.0] Chassis DragArea setup에 필요한 폭/높이가 유효한지 여부입니다.
	const bool bChassisGeometryValid = FMath::IsFinite(VehicleMovementConfig.ChassisWidth)
		&& VehicleMovementConfig.ChassisWidth > 0.0f
		&& FMath::IsFinite(VehicleMovementConfig.ChassisHeight)
		&& VehicleMovementConfig.ChassisHeight > 0.0f;

	// [v2.158.0] VehicleData Transmission complete payload가 UE 5.8 setup 계약을 만족하는지 여부입니다.
	const bool bTransmissionConfigValid = AreTransmissionRatiosValid(VehicleMovementConfig.TransmissionRatios.ForwardGearRatios)
		&& AreTransmissionRatiosValid(VehicleMovementConfig.TransmissionRatios.ReverseGearRatios)
		&& FMath::IsFinite(VehicleMovementConfig.FinalRatio) && VehicleMovementConfig.FinalRatio > 0.0f
		&& IsNonNegativeIntegerRpm(VehicleMovementConfig.ChangeUpRPM)
		&& IsNonNegativeIntegerRpm(VehicleMovementConfig.ChangeDownRPM)
		&& FMath::IsFinite(VehicleMovementConfig.GearChangeTime) && VehicleMovementConfig.GearChangeTime >= 0.0f
		&& FMath::IsFinite(VehicleMovementConfig.TransmissionEfficiency)
		&& VehicleMovementConfig.TransmissionEfficiency >= 0.0f
		&& VehicleMovementConfig.TransmissionEfficiency <= 1.0f
		&& (!VehicleMovementConfig.bUseAutomaticGears || VehicleMovementConfig.ChangeDownRPM <= VehicleMovementConfig.ChangeUpRPM);

	// [v2.166.0] vehicle-specific Engine Torque Curve opt-in payload의 구조 검증 결과입니다.
	FString EngineTorqueCurveError;
	const bool bEngineTorqueCurveValid = !VehicleMovementConfig.bUseEngineTorqueCurve
		|| FCFVehicleEngineCurveUtils::ValidateCurve(VehicleMovementConfig.EngineTorqueCurve, EngineTorqueCurveError);

	// [v2.166.0] complete setup validation 실패 시 Engine/Differential/Steering을 포함해 어떤 Movement 값도 부분 적용하지 않습니다.
	if (!bChassisGeometryValid || !bTransmissionConfigValid || !bEngineTorqueCurveValid)
	{
		LastVehicleRuntimeSummary = bEngineTorqueCurveValid
			? TEXT("VehicleRuntime: MovementConfig invalid; runtime movement mutation skipped. Chassis dimensions must be positive finite, ratios positive finite, Shift RPM non-negative integers, FinalRatio>0, time>=0, Efficiency=0..1.")
			: FString::Printf(TEXT("VehicleRuntime: EngineTorqueCurve invalid; runtime movement mutation skipped. %s"), *EngineTorqueCurveError);
		return;
	}

	// [v2.166.0] 현재 live setup을 복사한 뒤 VehicleData가 소유하는 Engine scalar와 Curve만 prospective하게 덮습니다.
	FVehicleEngineConfig ProposedEngineSetup = ResolvedVehicleMovementComponent->EngineSetup;
	ProposedEngineSetup.MaxTorque = VehicleMovementConfig.EngineMaxTorque;
	ProposedEngineSetup.MaxRPM = VehicleMovementConfig.EngineMaxRPM;
	ProposedEngineSetup.EngineIdleRPM = VehicleMovementConfig.EngineIdleRPM;
	ProposedEngineSetup.EngineBrakeEffect = VehicleMovementConfig.EngineBrakeEffect;
	ProposedEngineSetup.EngineRevUpMOI = VehicleMovementConfig.EngineRevUpMOI;
	ProposedEngineSetup.EngineRevDownRate = VehicleMovementConfig.EngineRevDownRate;

	if (VehicleMovementConfig.bUseEngineTorqueCurve)
	{
		if (!FCFVehicleEngineCurveUtils::ApplyCurveToChaos(VehicleMovementConfig.EngineTorqueCurve, ProposedEngineSetup, EngineTorqueCurveError))
		{
			LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: EngineTorqueCurve apply preparation failed; runtime movement mutation skipped. %s"), *EngineTorqueCurveError);
			return;
		}
	}
	else if (const UChaosWheeledVehicleMovementComponent* AuthoredMovementArchetype = Cast<UChaosWheeledVehicleMovementComponent>(ResolvedVehicleMovementComponent->GetArchetype()))
	{
		// [v2.166.0] 같은 Pawn에 vehicle-specific Curve 차량을 적용했다가 legacy/opt-out 차량으로 교체해도 이전 Curve가 남지 않도록 BP/SCS authored archetype Curve로 복원합니다.
		ProposedEngineSetup.TorqueCurve = AuthoredMovementArchetype->EngineSetup.TorqueCurve;
	}

	// [v2.166.0] 실제 Curve shape가 달라졌을 때만 setup-time PhysicsState 재생성 조건에 포함합니다.
	const bool bEngineTorqueCurveChanged = !FCFVehicleEngineCurveUtils::AreTorqueCurvesEquivalent(ResolvedVehicleMovementComponent->EngineSetup, ProposedEngineSetup);

	// [v2.157.0] Width/Height는 DragArea setup-time derived 값이고 Transmission/Engine Curve는 simulation setup-time 값이므로 live state 재생성 필요 여부를 assignment 전에 계산합니다.
	const bool bSetupRequiresPhysicsRecreate = !FMath::IsNearlyEqual(ResolvedVehicleMovementComponent->ChassisWidth, VehicleMovementConfig.ChassisWidth)
		|| !FMath::IsNearlyEqual(ResolvedVehicleMovementComponent->ChassisHeight, VehicleMovementConfig.ChassisHeight)
		|| ResolvedVehicleMovementComponent->TransmissionSetup.bUseAutomaticGears != VehicleMovementConfig.bUseAutomaticGears
		|| ResolvedVehicleMovementComponent->TransmissionSetup.bUseAutoReverse != VehicleMovementConfig.bUseAutoReverse
		|| ResolvedVehicleMovementComponent->TransmissionSetup.ForwardGearRatios != VehicleMovementConfig.TransmissionRatios.ForwardGearRatios
		|| ResolvedVehicleMovementComponent->TransmissionSetup.ReverseGearRatios != VehicleMovementConfig.TransmissionRatios.ReverseGearRatios
		|| !FMath::IsNearlyEqual(ResolvedVehicleMovementComponent->TransmissionSetup.FinalRatio, VehicleMovementConfig.FinalRatio)
		|| !FMath::IsNearlyEqual(ResolvedVehicleMovementComponent->TransmissionSetup.ChangeUpRPM, VehicleMovementConfig.ChangeUpRPM)
		|| !FMath::IsNearlyEqual(ResolvedVehicleMovementComponent->TransmissionSetup.ChangeDownRPM, VehicleMovementConfig.ChangeDownRPM)
		|| !FMath::IsNearlyEqual(ResolvedVehicleMovementComponent->TransmissionSetup.GearChangeTime, VehicleMovementConfig.GearChangeTime)
		|| !FMath::IsNearlyEqual(ResolvedVehicleMovementComponent->TransmissionSetup.TransmissionEfficiency, VehicleMovementConfig.TransmissionEfficiency)
		|| bEngineTorqueCurveChanged;

	// [v2.157.0] PhysicsState 재생성 전 chassis 속도를 보존할 inherited vehicle mesh입니다.
	USkeletalMeshComponent* VehicleMeshComponent = GetMesh();
	// [v2.157.0] 이미 live PhysicsState가 존재해 setup 재생성이 실제 필요한지 여부입니다.
	const bool bCanRecreateLivePhysics = bSetupRequiresPhysicsRecreate
		&& ResolvedVehicleMovementComponent->HasValidPhysicsState()
		&& VehicleMeshComponent
		&& VehicleMeshComponent->IsPhysicsStateCreated()
		&& VehicleMeshComponent->IsSimulatingPhysics();
	// [v2.157.0] live 재생성 때 복원할 기존 선속도입니다.
	const FVector PreviousLinearVelocity = bCanRecreateLivePhysics ? VehicleMeshComponent->GetPhysicsLinearVelocity() : FVector::ZeroVector;
	// [v2.157.0] live 재생성 때 복원할 기존 각속도입니다.
	const FVector PreviousAngularVelocityDegrees = bCanRecreateLivePhysics ? VehicleMeshComponent->GetPhysicsAngularVelocityInDegrees() : FVector::ZeroVector;

	ResolvedVehicleMovementComponent->ChassisWidth = VehicleMovementConfig.ChassisWidth;
	ResolvedVehicleMovementComponent->ChassisHeight = VehicleMovementConfig.ChassisHeight;
	ResolvedVehicleMovementComponent->DragCoefficient = VehicleMovementConfig.DragCoefficient;
	ResolvedVehicleMovementComponent->DownforceCoefficient = VehicleMovementConfig.DownforceCoefficient;
	ResolvedVehicleMovementComponent->bEnableCenterOfMassOverride = VehicleMovementConfig.bEnableCenterOfMassOverride;
	ResolvedVehicleMovementComponent->CenterOfMassOverride = VehicleMovementConfig.CenterOfMassOverride;
	// [v2.166.0] scalar + optional vehicle-specific/opt-out-restored Curve를 한 번에 적용해 EngineSetup partial mutation을 피합니다.
	ResolvedVehicleMovementComponent->EngineSetup = ProposedEngineSetup;
	ResolvedVehicleMovementComponent->DifferentialSetup.DifferentialType = VehicleMovementConfig.DifferentialType;
	ResolvedVehicleMovementComponent->DifferentialSetup.FrontRearSplit = VehicleMovementConfig.FrontRearSplit;
	ResolvedVehicleMovementComponent->TransmissionSetup.bUseAutomaticGears = VehicleMovementConfig.bUseAutomaticGears;
	ResolvedVehicleMovementComponent->TransmissionSetup.bUseAutoReverse = VehicleMovementConfig.bUseAutoReverse;
	ResolvedVehicleMovementComponent->TransmissionSetup.ForwardGearRatios = VehicleMovementConfig.TransmissionRatios.ForwardGearRatios;
	ResolvedVehicleMovementComponent->TransmissionSetup.ReverseGearRatios = VehicleMovementConfig.TransmissionRatios.ReverseGearRatios;
	ResolvedVehicleMovementComponent->TransmissionSetup.FinalRatio = VehicleMovementConfig.FinalRatio;
	ResolvedVehicleMovementComponent->TransmissionSetup.ChangeUpRPM = VehicleMovementConfig.ChangeUpRPM;
	ResolvedVehicleMovementComponent->TransmissionSetup.ChangeDownRPM = VehicleMovementConfig.ChangeDownRPM;
	ResolvedVehicleMovementComponent->TransmissionSetup.GearChangeTime = VehicleMovementConfig.GearChangeTime;
	ResolvedVehicleMovementComponent->TransmissionSetup.TransmissionEfficiency = VehicleMovementConfig.TransmissionEfficiency;
	ResolvedVehicleMovementComponent->SteeringSetup.SteeringType = VehicleMovementConfig.SteeringType;
	ResolvedVehicleMovementComponent->SteeringSetup.AngleRatio = VehicleMovementConfig.SteeringAngleRatio;
	ResolvedVehicleMovementComponent->bLegacyWheelFrictionPosition = VehicleMovementConfig.bLegacyWheelFrictionPosition;

	ResolvedVehicleMovementComponent->SetMaxEngineTorque(VehicleMovementConfig.EngineMaxTorque);
	ResolvedVehicleMovementComponent->SetDragCoefficient(VehicleMovementConfig.DragCoefficient);
	ResolvedVehicleMovementComponent->SetDownforceCoefficient(VehicleMovementConfig.DownforceCoefficient);
	ResolvedVehicleMovementComponent->SetDifferentialFrontRearSplit(VehicleMovementConfig.FrontRearSplit);

	if (bCanRecreateLivePhysics)
	{
		ResolvedVehicleMovementComponent->RecreatePhysicsState();
		if (ResolvedVehicleMovementComponent->HasValidPhysicsState() && VehicleMeshComponent->IsPhysicsStateCreated())
		{
			VehicleMeshComponent->SetPhysicsLinearVelocity(PreviousLinearVelocity, false);
			VehicleMeshComponent->SetPhysicsAngularVelocityInDegrees(PreviousAngularVelocityDegrees, false);
		}
		else
		{
			LastVehicleRuntimeSummary = TEXT("VehicleRuntime: Movement setup PhysicsState recreate failed.");
			return;
		}
	}

	LastVehicleRuntimeSummary = FString::Printf(TEXT("VehicleRuntime: MovementProfile=%s, RuntimeTorque=%.1f, ConfigMaxRPM=%.1f, EngineCurve=%s(%d), ThrottleScale=%.2f, Chassis=%.1fx%.1f, Drag=%.2f, Downforce=%.2f, Differential=%s, ForwardGears=%d, ReverseGears=%d, FinalRatio=%.3f, PhysicsRecreate=%s, SteeringType=%s, RuntimeSetters=EngineTorque/Drag/Downforce/DiffSplit"), *VehicleMovementConfig.MovementProfileName.ToString(), VehicleMovementConfig.EngineMaxTorque, VehicleMovementConfig.EngineMaxRPM, VehicleMovementConfig.bUseEngineTorqueCurve ? TEXT("VehicleSpecific") : TEXT("AuthoredArchetype"), VehicleMovementConfig.bUseEngineTorqueCurve ? VehicleMovementConfig.EngineTorqueCurve.Points.Num() : 0, VehicleMovementConfig.ThrottleInputScale, VehicleMovementConfig.ChassisWidth, VehicleMovementConfig.ChassisHeight, VehicleMovementConfig.DragCoefficient, VehicleMovementConfig.DownforceCoefficient, *UEnum::GetValueAsString(VehicleMovementConfig.DifferentialType), VehicleMovementConfig.TransmissionRatios.ForwardGearRatios.Num(), VehicleMovementConfig.TransmissionRatios.ReverseGearRatios.Num(), VehicleMovementConfig.FinalRatio, bCanRecreateLivePhysics ? TEXT("True") : TEXT("False"), *UEnum::GetValueAsString(VehicleMovementConfig.SteeringType));
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

// [v2.168.0] 기존 private Automation seam을 유지하면서 Wheel Visual authored cache 소유권을 VisualComp로 위임합니다.
void ACFVehiclePawn::CaptureWheelVisualAuthoredBaseTransformsIfNeeded()
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->CaptureWheelVisualAuthoredBaseTransformsIfNeeded();
	}
}

// [v2.168.0] 기존 private Automation seam을 유지하면서 Wheel Visual authored cache 폐기를 VisualComp로 위임합니다.
void ACFVehiclePawn::InvalidateWheelVisualAuthoredBaseTransforms()
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->InvalidateWheelVisualAuthoredBaseTransforms();
	}
}

// [v2.168.0] 기존 private Automation seam을 유지하면서 Wheel Visual base 복원과 Right fallback orientation을 VisualComp로 위임합니다.
void ACFVehiclePawn::PrepareWheelVisualComponentForApply(UStaticMeshComponent* WheelMeshComponent, const int32 WheelIndex, const bool bUseRightFallbackCompensation)
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->PrepareWheelVisualComponentForApply(WheelMeshComponent, WheelIndex, bUseRightFallbackCompensation);
	}
}

// [v2.168.0] 기존 private Automation seam을 유지하면서 Wheel Visual 행동과 순수 시각 캐시를 VisualComp로 위임합니다.
void ACFVehiclePawn::ApplyVehicleWheelVisualConfig()
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->ApplyVehicleWheelVisualConfig();
	}
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

bool ACFVehiclePawn::ShouldShowVehicleDebugUi() const
{
	// [v2.21.0] VehicleDebug HUD/Panel은 Viewport가 있는 로컬 제어 Pawn에서만 표시합니다.
	return bEnableDriveStateOnScreenDebug && (GetNetMode() != NM_DedicatedServer) && IsLocallyControlled();
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

// [v2.168.0] 기존 Pawn seam을 유지하면서 Owner 표시 안정화 준비 행동과 순수 시각 캐시를 VisualComp로 위임합니다.
bool ACFVehiclePawn::PrepareOwnerVisualStabilization()
{
	return VehicleVisualComp ? VehicleVisualComp->PrepareOwnerVisualStabilization() : false;
}

// [v2.168.0] 기존 Pawn seam을 유지하면서 Owner 표시 컴포넌트 부착 행동을 VisualComp로 위임합니다.
bool ACFVehiclePawn::AttachOwnerVisualComponent(USceneComponent* VisualComponent)
{
	return VehicleVisualComp ? VehicleVisualComp->AttachOwnerVisualComponent(VisualComponent) : false;
}

// [v2.168.0] 기존 Tick 호출 순서를 유지하면서 Owner 표시 안정화 갱신을 VisualComp로 위임합니다.
void ACFVehiclePawn::UpdateOwnerVisualStabilization(const float DeltaSeconds)
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->UpdateOwnerVisualStabilization(DeltaSeconds);
	}
}

// [v2.168.0] 기존 Pawn seam을 유지하면서 Owner 표시 안정화 Reset을 VisualComp로 위임합니다.
void ACFVehiclePawn::ResetOwnerVisualStabilization()
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->ResetOwnerVisualStabilization();
	}
}

// [v2.168.0] 기존 Pawn seam을 유지하면서 Owner 표시 회전 제한 계산을 VisualComp로 위임합니다.
FRotator ACFVehiclePawn::ClampOwnerVisualStabilizedRotation(const FRotator& CurrentActorRotation, const FRotator& DesiredVisualRotation) const
{
	return VehicleVisualComp ? VehicleVisualComp->ClampOwnerVisualStabilizedRotation(CurrentActorRotation, DesiredVisualRotation) : DesiredVisualRotation;
}

// [v2.168.0] 기존 Tick 호출 순서를 유지하면서 Owner 차체 표시 안정화 갱신을 VisualComp로 위임합니다.
void ACFVehiclePawn::UpdateOwnerBodyVisualStabilization(const float DeltaSeconds)
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->UpdateOwnerBodyVisualStabilization(DeltaSeconds);
	}
}

// [v2.168.0] 기존 Pawn seam을 유지하면서 Owner 차체 표시 안정화 Reset을 VisualComp로 위임합니다.
void ACFVehiclePawn::ResetOwnerBodyVisualStabilization()
{
	if (VehicleVisualComp)
	{
		VehicleVisualComp->ResetOwnerBodyVisualStabilization();
	}
}

// [v2.168.0] 기존 Pawn seam을 유지하면서 Owner 차체 표시 회전 제한 계산을 VisualComp로 위임합니다.
FRotator ACFVehiclePawn::ClampOwnerBodyVisualRotation(const FRotator& CurrentActorRotation, const FRotator& DesiredVisualRotation) const
{
	return VehicleVisualComp ? VehicleVisualComp->ClampOwnerBodyVisualRotation(CurrentActorRotation, DesiredVisualRotation) : DesiredVisualRotation;
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

// [v2.169.1] Pawn Authority가 요청 ID/시간을 할당한 뒤 일반 FireRequest 계산을 FireComp로 위임합니다.
FCFVehicleFireRequest ACFVehiclePawn::BuildFireCommand()
{
	// 이번 입력의 Pawn-owned FireRequest 시간입니다.
	const float ClientFireTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// 이번 입력에 Pawn Authority가 할당하는 단조 증가 FireRequest ID입니다.
	const int32 FireRequestId = NextFireRequestId++;

	return VehicleFireComp
		? VehicleFireComp->BuildFireCommand(FireRequestId, ClientFireTimeSeconds)
		: FCFVehicleFireRequest();
}

// [v2.169.1] Pawn Authority가 요청 ID/시간을 할당한 뒤 고정 Command Target FireRequest 계산을 FireComp로 위임합니다.
FCFVehicleFireRequest ACFVehiclePawn::BuildFireCommandForTarget(
	const FVector& OverrideCommandTargetLocation,
	const bool bUseOverrideTarget)
{
	// 이번 후속 발사의 Pawn-owned FireRequest 시간입니다.
	const float ClientFireTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;

	// 이번 후속 발사에 Pawn Authority가 할당하는 단조 증가 FireRequest ID입니다.
	const int32 FireRequestId = NextFireRequestId++;

	return VehicleFireComp
		? VehicleFireComp->BuildFireCommandForTarget(
			FireRequestId,
			ClientFireTimeSeconds,
			OverrideCommandTargetLocation,
			bUseOverrideTarget)
		: FCFVehicleFireRequest();
}

// [v2.169.0] 기존 Pawn private seam을 유지하면서 일반 발사 검증을 FireComp로 위임합니다.
bool ACFVehiclePawn::ValidateFireCommand(
	const FCFVehicleFireRequest& FireCommand,
	FCFVehicleFireResult& OutFireResult)
{
	if (VehicleFireComp)
	{
		return VehicleFireComp->ValidateFireCommand(FireCommand, OutFireResult);
	}

	OutFireResult = FCFVehicleFireResult();
	OutFireResult.FireRequestId = FireCommand.FireRequestId;
	OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
	return false;
}

// [v2.169.0] 기존 Pawn private seam을 유지하면서 Launcher 후속 발사의 선택적 쿨다운 우회 검증을 FireComp로 위임합니다.
bool ACFVehiclePawn::ValidateFireCommandInternal(
	const FCFVehicleFireRequest& FireCommand,
	FCFVehicleFireResult& OutFireResult,
	const bool bIgnoreWeaponCooldown)
{
	if (VehicleFireComp)
	{
		return VehicleFireComp->ValidateFireCommandInternal(FireCommand, OutFireResult, bIgnoreWeaponCooldown);
	}

	OutFireResult = FCFVehicleFireResult();
	OutFireResult.FireRequestId = FireCommand.FireRequestId;
	OutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
	return false;
}

// [v2.169.0] 기존 Pawn private seam을 유지하면서 HitScan 실행을 FireComp로 위임합니다.
bool ACFVehiclePawn::RunLocalDummyHitScan(const FCFVehicleFireRequest& FireCommand, FCFVehicleFireResult& InOutFireResult)
{
	return VehicleFireComp && VehicleFireComp->RunLocalDummyHitScan(FireCommand, InOutFireResult);
}

// [v2.169.0] 기존 Pawn private seam을 유지하면서 HitScan Damage 계산/적용 행동을 FireComp로 위임합니다.
void ACFVehiclePawn::RecordDummyHitScanDamageHitContext(
	const FCFVehicleFireRequest& FireCommand,
	const FCFVehicleFireResult& FireResult,
	const FHitResult* HitResult,
	const bool bBlockingHit)
{
	if (VehicleFireComp)
	{
		VehicleFireComp->RecordDummyHitScanDamageHitContext(FireCommand, FireResult, HitResult, bBlockingHit);
	}
}

// [v2.169.0] Projectile Pool callback용 기존 Pawn public seam을 유지하면서 Damage/ImpactFx 행동을 FireComp로 위임합니다.
void ACFVehiclePawn::RecordProjectileDamageHitContextFromPool(const ACFProjectileActor* InProjectileActor)
{
	if (VehicleFireComp)
	{
		VehicleFireComp->RecordProjectileDamageHitContextFromPool(InProjectileActor);
	}
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

// [v2.169.0] 기존 Pawn private seam을 유지하면서 Muzzle FireOrigin 계산을 FireComp로 위임합니다.
bool ACFVehiclePawn::TryBuildMuzzleFireOrigin(FCFVehicleFireOrigin& InOutFireOrigin, FString& OutFireOriginSummary) const
{
	if (VehicleFireComp)
	{
		return VehicleFireComp->TryBuildMuzzleFireOrigin(InOutFireOrigin, OutFireOriginSummary);
	}

	OutFireOriginSummary = FString();
	return false;
}

// [v2.169.0] 기존 Pawn private seam을 유지하면서 Weapon Aim Solution 계산을 FireComp로 위임합니다.
bool ACFVehiclePawn::BuildWeaponAimSolution(
	FCFVehicleWeaponAimSolution& OutWeaponAimSolution,
	FCFVehicleFireOrigin* OutFireOrigin,
	FString* OutFireOriginSummary,
	const FVector* OverrideAimTargetLocation) const
{
	if (VehicleFireComp)
	{
		return VehicleFireComp->BuildWeaponAimSolution(OutWeaponAimSolution, OutFireOrigin, OutFireOriginSummary, OverrideAimTargetLocation);
	}

	OutWeaponAimSolution = FCFVehicleWeaponAimSolution();
	if (OutFireOrigin)
	{
		*OutFireOrigin = FCFVehicleFireOrigin();
	}
	if (OutFireOriginSummary)
	{
		*OutFireOriginSummary = FString();
	}
	return false;
}

// [v2.169.0] 기존 Pawn seam을 유지하면서 Weapon Aim Solution 갱신을 FireComp로 위임합니다.
void ACFVehiclePawn::RefreshWeaponAimSolution()
{
	if (VehicleFireComp)
	{
		VehicleFireComp->RefreshWeaponAimSolution();
	}
}

// [v2.169.0] 기존 Pawn seam을 유지하면서 Projectile 실행 가능 판정을 FireComp로 위임합니다.
bool ACFVehiclePawn::ShouldUseProjectileActorFire() const
{
	return VehicleFireComp && VehicleFireComp->ShouldUseProjectileActorFire();
}

// [v2.169.0] 기존 Pawn private seam을 유지하면서 Projectile Launch Context 생성을 FireComp로 위임합니다.
bool ACFVehiclePawn::BuildDirectProjectileLaunchContext(
	const FCFVehicleFireRequest& FireCommand,
	const UCFProjectileData& InProjectileData,
	AActor* GuidanceTargetActorSnapshot,
	FCFProjectileLaunchContext& OutLaunchContext) const
{
	if (VehicleFireComp)
	{
		return VehicleFireComp->BuildDirectProjectileLaunchContext(FireCommand, InProjectileData, GuidanceTargetActorSnapshot, OutLaunchContext);
	}

	OutLaunchContext = FCFProjectileLaunchContext();
	return false;
}

// [v2.169.0] 기존 Pawn private seam을 유지하면서 단발 Projectile Pool 실행을 FireComp로 위임합니다.
bool ACFVehiclePawn::TrySpawnProjectileActorFromFireCommand(const FCFVehicleFireRequest& FireCommand)
{
	return VehicleFireComp && VehicleFireComp->TrySpawnProjectileActorFromFireCommand(FireCommand);
}

// [v2.169.0] 기존 Automation private seam을 유지하면서 검증 승인 발사 실행을 FireComp로 위임합니다.
bool ACFVehiclePawn::ExecuteAcceptedFireCommand(
	const FCFVehicleFireRequest& FireCommand,
	FCFVehicleFireResult& InOutFireResult,
	AActor* GuidanceTargetActorSnapshot,
	const bool bAllowProjectileFallback)
{
	if (VehicleFireComp)
	{
		return VehicleFireComp->ExecuteAcceptedFireCommand(FireCommand, InOutFireResult, GuidanceTargetActorSnapshot, bAllowProjectileFallback);
	}

	InOutFireResult.bAccepted = false;
	InOutFireResult.RejectReason = ECFVehicleFireRejectReason::InvalidLocalState;
	return false;
}

// [v2.169.0] LauncherComp가 계속 호출하는 기존 Pawn callback을 유지하면서 후속 발사 행동을 FireComp로 위임합니다.
bool ACFVehiclePawn::ExecuteScheduledLauncherShot(
	const int32 VolleyId,
	const int32 SequenceShotIndex,
	const FVector& CommandTargetLocation,
	AActor* GuidanceTargetActorSnapshot)
{
	return VehicleFireComp
		&& VehicleFireComp->ExecuteScheduledLauncherShot(VolleyId, SequenceShotIndex, CommandTargetLocation, GuidanceTargetActorSnapshot);
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

// [v2.169.1] Enhanced Input binding과 입력 순간 Fire observable commit을 Pawn에 유지하고 이후 발사 행동만 FireComp로 위임합니다.
void ACFVehiclePawn::HandleFireStarted(const FInputActionValue&)
{
	LastFireRequest = BuildFireCommand();

	if (VehicleFireComp)
	{
		VehicleFireComp->HandleFireStarted(LastFireRequest);
	}
}

// [v2.63.0] 기존 단발 호출자는 승인 발사 시 쿨다운을 즉시 기록하는 호환 경로를 유지합니다.
void ACFVehiclePawn::ApplyFireResult(const FCFVehicleFireRequest& FireCommand, const FCFVehicleFireResult& FireResult)
{
	ApplyFireResultInternal(FireCommand, FireResult, true);
}

// [v2.169.0] Pawn-owned Fire observable을 먼저 commit한 뒤 Domain side effect만 FireComp에 위임합니다.
void ACFVehiclePawn::ApplyFireResultInternal(
	const FCFVehicleFireRequest& FireCommand,
	const FCFVehicleFireResult& FireResult,
	const bool bRecordCooldown)
{
	LastFireRequest = FireCommand;
	LastFireResult = FireResult;

	// 마지막 로컬 FireFeedback 표시 시작 시간을 Pawn observable state로 기록합니다.
	LastFireFeedbackStartTimeSeconds = GetWorld() ? GetWorld()->GetTimeSeconds() : -1.0;

	if (VehicleFireComp)
	{
		VehicleFireComp->ApplyFireResultSideEffects(FireCommand, FireResult, bRecordCooldown);
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
