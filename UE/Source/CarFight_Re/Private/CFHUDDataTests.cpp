// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.35.0
// Date: 2026-08-25
// Description: CF-FQ-032 HUD Runtime Automation + CF-FQ-039 VehiclePanel Production Visual 회귀 검증
// Scope: 실제 Runtime 계약과 저장 Production HUD의 Vehicle Frame·Shield/Integrity·Armor·ViewMode·Alert·Style Context·다중 해상도 Root Layout 의미 Presentation을 검증합니다.
// Changelog:
// - v1.35.0: CF-FQ-039 modular Armor migration 단계에 맞춰 ArmorSectorProductionVisualContract를 common Plate + additive Image_DirectionIcon + Text fallback 계약으로 전환. DirectionIcon Texture가 아직 Source Binding되지 않은 현재 persisted 상태에서는 Icon Collapsed / Text HitTestInvisible을 검증하고 Ratio/Tint 회귀는 그대로 유지.
// - v1.34.2: fresh PIE에서는 Defense Bar가 실제 픽셀로 정상 렌더링되지만 viewport에 붙지 않은 transient Widget의 GetDesiredSize()가 계속 0을 반환하는 test-harness 불일치를 교정. 실제 회귀 원인인 ProgressBar Style Background/Fill/Marquee Brush intrinsic height > 0을 직접 검증하도록 변경.
// - v1.34.1: 실제 PIE pixel review에서 발견된 Defense Bar 높이 0 회귀를 막기 위해 Presenter 적용 후 Layout Prepass를 수행하고 Shield/Integrity ProgressBar DesiredSize.Y가 0보다 큰지 검증.
// - v1.34.0: 저장 VehiclePanel의 Border_Surface가 HUDVisualData VehiclePanelFrame 9-Slice를 실제 소비하는지와 Presenter 적용 후 Shield/Integrity가 custom dark track + 2px inset semantic fill/색/Ratio를 유지하는 VehicleDefenseBarVisualContract 추가.
// - v1.33.1: headless nested UserWidget lifecycle에 의존하지 않고 실제 Presenter 공용 경로와 동일하게 SetArmorPercent를 먼저 호출한 뒤 icon-first Direction Label visibility를 검증하도록 focused test를 교정.
// - v1.33.0: 저장 Production HUD의 6개 재사용 ArmorSector가 방향 Texture 우선/Label fallback 계약을 지키고 0~1 Armor Ratio를 Stable Armor/Caution/Critical 시각 상태와 실제 세로 Bar Percent로 반영하는 ArmorSectorProductionVisualContract 추가.
// - v1.32.0: Vehicle Target Identity 테스트가 Production TargetCandidate resolver와 동일하게 native ICFTargetSelectable의 GetTargetDisplayInfo_Implementation 경로를 사용하도록 fixture를 교정. Product Identity 의미는 변경하지 않음.
// - v1.31.0: Vehicle Target Identity를 VehicleData PrimaryAssetId 기반 안정 TargetId로 검증하고 Actor instance 이름 비노출, DisplayName fail-closed와 Identified Sensor Contact public contract 유효성을 함께 확인.
// - v1.30.0: Vehicle Target Identity fail-closed 회귀와 Critical suppression 중 Warning duration 비소모/첫 표시부터 3초 lifecycle 회귀를 추가.
// - v1.29.0: 저장 WBP_CFInGameHUD의 Designer-owned Root Slot을 현재 UUserInterfaceSettings DPI Scale로 환산한 1920×1080 / 2560×1440 / 3440×1440 / 5120×1440 logical Viewport에 해석해 6 Panel 화면내 유지·상호 비겹침과 ReticleLayer full-stretch를 검증하는 ResolutionLayoutContract 추가.
// - v1.28.0: AlertStyleLifecycleContract와 NestedStyleContextContract를 추가해 Style Alert duration/persistent, non-reset lifecycle, Priority와 Root→중첩 Production Styled Widget Context 전파를 검증.
// - v1.27.0: 저장 WBP_CFInGameHUD의 ReticleLayer Vehicle Semantic Image가 Camera 상대 Yaw를 차체 상대 화면 방향으로 소비하고 ±90도 clamp, Spectate/Destroyed/Unavailable hide, Designer Track ownership을 지키는 ViewModeProductionConsumerContract 추가.
// - v1.26.1: transient Pawn에 Production FollowCamera가 없는 Automation 환경에서는 Camera Runtime 초기화를 강제하지 않고 VehicleCameraComp의 공식 GetCurrentAimDirection fallback 계약을 사용하도록 Fixture를 교정.
// - v1.26.0: 기존 VehicleCameraComp/AimComp source로 Camera Mode·Vehicle Heading·Camera 상대 방향·CurrentMuzzleDirection 기반 Turret 상대 방향을 ViewMode에 전달하고 Null Rebind에서 fail-closed하는 계약을 추가.
// - v1.25.0: RadarContactConsumerContract를 전용 UImage Blip/Brush Template, Display Range Text, Frame/Player, selected range-out 2-Corner Edge 검증으로 교정하고 Text glyph icon 부재를 명시적으로 확인.
// - v1.24.0: RadarContactConsumerContract 초기 기능 slice에서 7개 actual Contact pool, normalized Canvas Anchor, in-range selected bracket과 range-out 4-corner 오용 방지를 검증. Text glyph 기대는 v1.25.0에서 제거.
// - v1.23.0: UI-P0-07 TargetKnowledgePanelContract를 추가해 NO TARGET, Detected Unknown, Identified, Sensor Contact Unknown fail-closed와 Clear 복귀를 저장 Production TargetPanel에서 검증.
// - v1.22.0: RpmGaugeVisualBindingContract를 구형 21 ProgressBar Tick 검증에서 단일 UI Material Image + `RPMRatio` 0/.85/.925/1/reset0 검증으로 전환하고 legacy RPM Tick 부재를 명시적으로 확인.
// - v1.21.0: explicit WeaponCharge의 초기값·승인 한 발 소비·Game-Time 회복·충전 부족 차단 가능 상태, 실제 WeaponComp 연계, HUD Charge Channel과 Compact Primary/Secondary/NO CHARGE 우선순위를 검증하는 WeaponChargeRuntimeResourceContract 추가. Pawn protected fire helper는 테스트 편의로 노출하지 않음.
// - v1.20.0: saved Production WeaponPanel의 truthful Text Rail 구조, selected exclusion, fixed order, DisplayName-only/generic fallback, 2+overflow와 실제 Presenter slot 적용을 검증하는 WeaponRailVisualContract 추가.
// - v1.19.0: Applied Fitting ResolvedMounts 고정 순서→Weapon Selection Runtime→per-weapon Cooldown/Heat 격리→HUD DisplayName/SelectedIndex를 transient-only로 검증하는 WeaponSelectionRuntimeContract 추가. Production Rail/Asset mutation 0.
// - v1.18.1: HeatRuntimeResourceContract에 실제 transient Pawn/WeaponComp ExecuteAcceptedFireCommand→ApplyFireResult 4발 누적과 Component Tick 냉각 검증을 추가. Asset mutation 0.
// - v1.18.0: explicit Heat Runtime의 누적·자연 냉각·과열 회복, actual Heat Resource Channel, Reload/NoAmmo/Cooldown 우선순위 보호와 Launcher+Ammo+Heat Compact Projection을 검증하는 HeatRuntimeResourceContract 추가.
// - v1.17.1: 신규 RPM Visual test helper 인자명을 AutomationTestBase::TestName과 충돌하지 않도록 TickPercentTestLabel로 교정. Production 구현 변경 0.
// - v1.17.0: 저장 Production WBP_CFSpeedGauge의 기존 21 Tick이 explicit RPM ratio를 소비하고 Redline unavailable에서 21 fill을 0으로 reset하는 RpmGaugeVisualBindingContract 추가. Asset mutation 0.
// - v1.16.0: RPM Gauge Runtime Source Contract 추가. Current RPM은 Chaos Movement, Redline/Maximum은 Current Pawn VehicleData explicit source이며 Redline 0에서 fallback 없이 Unavailable인지 검증.
// - v1.15.0: explicit RedlineStartRPM→0.85 / EngineMaxRPM→1.0 piecewise RPM Gauge mapping과 미설정/invalid fail-closed 계약을 검증하는 RpmGaugePresentationContract 추가.
// - v1.14.0: ResourceVisualSlotContract의 표시 상태 expectation을 Presenter 공통 정책인 HitTestInvisible로 교정. Production 구현과 Presentation 값/lifecycle은 변경하지 않음.
// - v1.13.0: Dynamic Resource Visual Stage B. 저장 Production HUD의 새 Compact 의미 슬롯에 한 ViewData 적용당 Projection이 정확히 한 번 소비되는지, Header Reserve·Primary/Secondary/FireState·Launcher terminal 1주기와 legacy 고정 Row 부재를 검증하는 ResourceVisualSlotContract를 추가.
// - v1.12.0: Dynamic Resource Visual Stage A. raw ResourceChannels 직접 렌더링 대신 Primary 1 + Secondary 최대 2 + FireState 1 Projection, Reserve Header 분리와 Launcher Active→Terminal 1주기→Cooldown 전이를 검증하는 ResourcePresentationProjection 회귀를 추가.
// - v1.11.0: legacy Weapon 필드와 충돌하는 ResourceChannels를 구성해 Presenter가 Ammo·Reserve·Reload·NoAmmo·Cooldown·Launcher를 공통 채널에서 우선 해석하는 parity 회귀를 추가.
// - v1.10.0: 호환되는 실제 활성 EquipmentPresetData.DisplayName이 HUD Weapon DisplayName으로 전달되고 빈 이름에서 내부 ID fallback 없이 Unavailable로 복귀하는 focused 회귀를 추가.
// - v1.9.0: 기존 실제 Ammo·Reserve·Cooldown·Reload·LauncherSequence가 additive ResourceChannels에 값 손실 없이 투영되고 Battery·Charge·Heat는 생성되지 않는 UI-P0-06 focused 회귀를 추가.
// - v1.8.0: 실제 ACFVehiclePawn의 Chaos Movement Engine RPM/Current Gear가 Provider Vehicle ViewData와 Gear Text로 동일하게 전달되는 UI-P0-06 focused 회귀를 추가.
// - v1.7.0: UI-P0-06 착수 감사에서 Provider 부재로 확정된 Weapon DisplayName·VehicleBattery·WeaponCharge와 기존 RPM·Gear·Heat가 기본 ViewData에서 Unavailable을 유지하는 회귀를 보강.
// - v1.6.0: 저장된 M_VehicleDefensePIE의 실제 PIE 복제 Defense SUV를 Provider에 바인딩하고 정식 3층 피해가 Shield/Front Armor/Integrity ViewData로 전달되는 Technical PIE 회귀를 추가.
// - v1.5.0: 실제 VehicleDefenseComp 피해·재생으로 Shield/Armor/Integrity ViewData를 검증하고 Provider Rebind 뒤 Old Pawn Defense 이벤트가 새 Pawn ViewData를 갱신하지 않는 구독 해제 회귀를 추가.
// - v1.4.0: Salvo 전용 Hold 회귀를 제거하고 LauncherSequenceRevision 기반 Ripple·Salvo 공통 Active→Terminal→Cooldown/READY lifecycle 및 HeavyCannon SingleCycle 상태 경로를 검증.
// - v1.3.0: [폐기 이력] 같은 프레임에 완료되는 Salvo Hold가 사용할 terminal Snapshot 변환 계약을 추가.
// - v1.2.0: 정상 LauncherSequence를 Alert가 아닌 WeaponPanel 진행 상태로 표현하는 RIPPLE/SALVO 문구·진행률 회귀를 추가하고 구형 AlertSemanticRouting 회귀를 제거.
// - v1.1.0: LauncherSequence AlertKey가 단독/혼합 상태에서 전용 Launcher 슬롯으로 결정되는 semantic routing 회귀를 추가.
// - v1.0.2: Transient ULocalPlayer Outer를 실제 ClassWithin 계약에 맞게 GEngine 아래 생성하도록 교정.
// - v1.0.1: Provider Rebind Fixture의 UCFUISubsystem Outer를 실제 ClassWithin 계약인 ULocalPlayer로 교정.
// - v1.0.0: ViewData Availability와 Provider Rebind Generation/Old Pawn 해제 계약 테스트를 최초 추가.
// Migration:
// - 테스트는 Transient ULocalPlayer/UObject와 Automation World만 사용하며 Unreal Asset을 생성하거나 저장하지 않습니다.
// - Defense 검증은 Production 계산을 복제하지 않고 실제 UCFVehicleDefenseComp::TryApplyDamageToActor와 Shield 재생 Tick을 사용합니다.
// - FirePattern은 RIPPLE/SALVO 문구 선택에만 사용하며 Presentation 수명 전이는 LauncherSequenceRevision과 Active 상태로 검증합니다.
// - v1.13.0 Stage B 테스트는 저장 Production HUD를 읽기만 하며 Asset을 생성·수정·저장하지 않습니다. 새 의미 슬롯 Asset 적용 전에는 의도적으로 통과 조건이 성립하지 않습니다.
// - v1.22.0 RPM Visual Binding 테스트도 저장 Production HUD를 읽기만 하며 대표 VehicleData의 RedlineStartRPM을 작성하거나 Asset을 저장하지 않습니다. UI Material의 Runtime MID 스칼라만 transient Widget에서 검증합니다.
// - v1.18.0 Heat 검증은 transient 순수 Runtime/ViewData만 사용하고 WeaponData/Production Asset을 생성·수정·저장하지 않습니다.
// - v1.19.0 Weapon Selection 검증은 transient Fitting Snapshot·Pawn·DataAsset UObject만 사용하고 실제 Content/Production Rail/Input Asset을 생성·수정·저장하지 않습니다.
// - v1.20.0 Weapon Rail 검증은 저장 Production HUD를 읽기만 하고 Presenter synthetic ViewData를 적용합니다. Runtime ID, weapon icon, 비선택 resource summary를 생성하지 않으며 Asset을 저장하지 않습니다. 내부 MountProfileId는 Fitting→WeaponComp identity 검증에만 사용하고 HUD ViewData에는 노출하지 않습니다.
// - v1.21.0 WeaponCharge 검증은 transient Runtime/Pawn/ViewData만 사용하고 저장 WeaponData/Production Asset을 생성·수정·저장하지 않습니다. VehicleBattery, Heat tuning, Redline authoring은 변경하지 않습니다.
// - v1.23.0 Target Knowledge 검증은 저장 Production HUD를 읽기만 하고 synthetic FCFTargetHUDData를 Presenter에 적용합니다. TargetPanel Asset, Sensor Runtime과 TargetSelect Runtime은 생성·수정·저장하지 않습니다.
// - v1.25.0 Radar Contact Consumer 검증은 B2 targeted migration이 끝난 저장 RadarPanel을 읽고 Presenter가 transient UImage pool만 추가합니다. Frame/Blip/Player/Selection Texture는 저장 Asset Brush를 검증하며 Content Asset을 저장하지 않습니다.
// - v1.26.0 ViewMode 검증은 transient Map/Pawn의 기존 Camera/Aim Runtime만 사용하며 Content/Blueprint/DataAsset을 생성·수정·저장하지 않습니다.
// - v1.28.0 Alert/Style 검증은 Native Style CDO와 저장 Production HUD를 읽기만 하며 Asset을 생성·수정·저장하지 않습니다. Alert 시간은 synthetic Game-Time 인자로 결정론적으로 검증합니다.
// - v1.29.0 Resolution 검증은 저장 Production Root의 기존 UCanvasPanelSlot Anchor/Offset/Alignment와 현재 Project UUserInterfaceSettings의 DPI Scale을 읽어 synthetic logical Viewport에 계산만 하며 Product Layout·Asset·Viewport Runtime을 변경하지 않습니다.
// - v1.32.0 Identity fixture는 Production CFTargetCandidateSearch의 native interface resolver branch와 동일한 GetTargetDisplayInfo_Implementation 호출을 사용합니다. Raw Execute dispatch를 native C++ override 검증 수단으로 해석하지 않습니다.
// - v1.31.0 Identity 검증은 transient UCFVehicleData의 PrimaryAssetId만 사용하며 실제 VehicleData Asset을 생성·수정·저장하지 않습니다. Actor instance 이름을 TargetId/DisplayName fallback으로 사용하지 않고 Identified Sensor Contact 계약까지 확인합니다.
// - v1.30.0 Identity/Alert remediation 검증은 transient VehiclePawn과 synthetic Alert Game-Time만 사용하며 Content Asset을 생성·수정·저장하지 않습니다.
// - v1.33.0 Armor Production Visual 검증은 저장 WBP_CFInGameHUD/WBP_CFArmorBodyMap/WBP_CFArmorSector를 읽기만 하고 transient Widget 인스턴스의 SetArmorPercent만 호출합니다. Content Asset과 Gameplay Defense 상태를 생성·수정·저장하지 않습니다.
// - v1.34.0 Vehicle Frame/Defense Bar Visual 검증은 저장 WBP_CFInGameHUD와 HUDVisualData를 읽고 transient Presenter ViewData만 적용합니다. Widget Tree, Texture, DataAsset과 Designer Layout을 저장하지 않습니다.
// - v1.34.1 Defense Bar 검증은 property 값만 확인하지 않고 Layout Prepass 뒤 DesiredSize.Y > 0까지 확인해 Brush intrinsic size 손실로 인한 실제 픽셀 소실을 차단합니다.
// - v1.34.2 viewport에 붙지 않은 transient UserWidget의 DesiredSize는 실제 PIE pixel 결과와 불일치할 수 있으므로 pixel 가시성 자체는 fresh PIE screenshot evidence가 소유하고, focused test는 regression root인 3개 Style Brush intrinsic height를 직접 고정합니다.




#if WITH_DEV_AUTOMATION_TESTS

#include "CFDamageData.h"
#include "CFEquipmentPresetData.h"
#include "CFSensorTypes.h"
#include "CFTargetSelectable.h"
#include "CFTurretMountData.h"
#include "CFVehicleAimComp.h"
#include "CFVehicleAimTypes.h"
#include "CFVehicleCameraComp.h"
#include "CFVehicleData.h"
#include "CFVehicleDefenseComp.h"
#include "CFVehicleDefenseData.h"
#include "CFVehicleFittingComp.h"
#include "CFVehicleHealthComp.h"
#include "CFVehiclePawn.h"
#include "CFVehicleDriveComp.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"
#include "CFWeaponChargeRuntime.h"
#include "CFWeaponHeatRuntime.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Widget.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Texture2D.h"
#include "Engine/UserInterfaceSettings.h"
#include "EngineUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"
#include "UI/CFArmorSectorWidget.h"
#include "UI/CFHUDDataProvider.h"
#include "UI/CFHUDPresenter.h"
#include "UI/CFHUDVisualData.h"
#include "UI/CFHUDViewData.h"
#include "UI/CFStyledWidgetBase.h"
#include "UI/CFUIStyleData.h"
#include "UI/CFUISubsystem.h"

namespace
{
	// [v1.5.0] HUD Provider가 읽을 실제 차량 방어 런타임 묶음입니다.
	struct FCFHUDDefenseFixture
	{
		// [v1.5.0] Provider Source와 실제 피해 대상 역할을 하는 차량 Pawn입니다.
		ACFVehiclePawn* VehiclePawn = nullptr;

		// [v1.5.0] 차량 Integrity 상태를 소유하는 실제 Health 컴포넌트입니다.
		UCFVehicleHealthComp* HealthComponent = nullptr;

		// [v1.5.0] Shield·6방향 Armor 상태와 피해 분배를 소유하는 실제 Defense 컴포넌트입니다.
		UCFVehicleDefenseComp* DefenseComponent = nullptr;

		// [v1.5.0] 이번 Fixture의 Shield·Armor·재생 설정을 제공하는 Transient 방어 데이터입니다.
		UCFVehicleDefenseData* DefenseData = nullptr;
	};

	// [v1.5.0] 지정 최대 Shield와 공통 100 Armor를 가진 실제 ACFVehiclePawn 방어 Fixture를 준비합니다.
	FCFHUDDefenseFixture CreateHUDDefenseFixture(UWorld* TestWorld, const FName PawnName, const float MaximumShield)
	{
		// [v1.5.0] 생성한 Pawn과 실제 방어 컴포넌트를 반환할 결과입니다.
		FCFHUDDefenseFixture Fixture;
		if (!TestWorld)
		{
			return Fixture;
		}

		// [v1.5.0] 테스트 Pawn에 안정적인 이름을 부여할 Spawn 설정입니다.
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = PawnName;
		Fixture.VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
			ACFVehiclePawn::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			SpawnParameters);
		if (!Fixture.VehiclePawn)
		{
			return Fixture;
		}

		Fixture.HealthComponent = Fixture.VehiclePawn->GetVehicleHealthComp();
		Fixture.DefenseComponent = Fixture.VehiclePawn->GetVehicleDefenseComp();

		// [v1.5.0] Fixture Pawn lifetime에 결합된 Transient VehicleDefenseData 이름입니다.
		const FName DefenseDataName(*FString::Printf(TEXT("%s_DefenseData"), *PawnName.ToString()));
		Fixture.DefenseData = NewObject<UCFVehicleDefenseData>(Fixture.VehiclePawn, DefenseDataName);
		if (!Fixture.HealthComponent || !Fixture.DefenseComponent || !Fixture.DefenseData)
		{
			return Fixture;
		}

		Fixture.DefenseData->bUseShield = true;
		Fixture.DefenseData->MaximumShield = MaximumShield;
		Fixture.DefenseData->ShieldRegenerationDelaySeconds = 5.0f;
		Fixture.DefenseData->ShieldRegenerationPerSecond = 10.0f;
		Fixture.DefenseData->ArmorResistance = 100.0f;
		Fixture.DefenseData->FrontArmorConfig.MaximumArmor = 100.0f;
		Fixture.DefenseData->FrontArmorConfig.DamageMultiplier = 1.0f;
		Fixture.DefenseData->LeftArmorConfig.MaximumArmor = 100.0f;
		Fixture.DefenseData->LeftArmorConfig.DamageMultiplier = 1.0f;
		Fixture.DefenseData->RightArmorConfig.MaximumArmor = 100.0f;
		Fixture.DefenseData->RightArmorConfig.DamageMultiplier = 1.0f;
		Fixture.DefenseData->RearArmorConfig.MaximumArmor = 100.0f;
		Fixture.DefenseData->RearArmorConfig.DamageMultiplier = 1.0f;
		Fixture.DefenseData->TopArmorConfig.MaximumArmor = 100.0f;
		Fixture.DefenseData->TopArmorConfig.DamageMultiplier = 1.0f;
		Fixture.DefenseData->BottomArmorConfig.MaximumArmor = 100.0f;
		Fixture.DefenseData->BottomArmorConfig.DamageMultiplier = 1.0f;

		Fixture.HealthComponent->InitializeFromVehicleData(nullptr);
		Fixture.DefenseComponent->Activate(true);
		Fixture.DefenseComponent->InitializeFromDefenseData(Fixture.DefenseData);
		return Fixture;
	}

		// [v1.5.0] 실제 VehicleDefenseComp 정면 피해 진입점에 전달할 Blocking HitContext를 생성합니다.
	FCFDamageHitContext BuildHUDDefenseHitContext(
		ACFVehiclePawn* TargetPawn,
		AActor* InstigatorActor,
		UCFDamageData* DamageData)
	{
		// [v1.5.0] 정면 +X 위치를 사용하는 완성된 방어 피해 컨텍스트입니다.
		FCFDamageHitContext DamageHitContext;
		if (!TargetPawn)
		{
			return DamageHitContext;
		}

		DamageHitContext.DamageData = DamageData;
		DamageHitContext.HitActor = TargetPawn;
		DamageHitContext.InstigatorActor = InstigatorActor;
		DamageHitContext.ImpactLocation = TargetPawn->GetActorLocation() + FVector(100.0f, 0.0f, 0.0f);
		DamageHitContext.ImpactNormal = FVector::ForwardVector;
		DamageHitContext.IncomingDirection = -FVector::ForwardVector;
		DamageHitContext.bBlockingHit = true;
		return DamageHitContext;
	}

	// [v1.6.0] 저장된 M_VehicleDefensePIE의 실제 PIE Defense SUV와 Provider 3층 ViewData를 검증합니다.
	class FCFVerifyHUDDefensePIECommand final : public IAutomationLatentCommand
	{
	public:
		// [v1.6.0] 결과를 기록할 Automation Test와 PIE 준비 제한 시간 시작점을 보존합니다.
		explicit FCFVerifyHUDDefensePIECommand(FAutomationTestBase* InTest)
			: Test(InTest)
			, StartTimeSeconds(FPlatformTime::Seconds())
		{
		}

		// [v1.6.0] 실제 PIE World의 저장 Defense SUV를 찾아 피해 전후 Provider ViewData를 검증합니다.
		virtual bool Update() override
		{
			// [v1.6.0] 현재 Engine World Context에서 찾은 실제 PIE World입니다.
			UWorld* PIEWorld = nullptr;
			if (GEngine)
			{
				for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
				{
					if (WorldContext.WorldType == EWorldType::PIE && WorldContext.World())
					{
						PIEWorld = WorldContext.World();
						break;
					}
				}
			}

			// [v1.6.0] PIE World 준비를 기다린 누적 시간입니다.
			const double ElapsedSeconds = FPlatformTime::Seconds() - StartTimeSeconds;
			// [v1.6.0] 맵 또는 PIE 시작 실패를 무한 대기하지 않을 제한 시간입니다.
			constexpr double PIEWorldReadyTimeoutSeconds = 30.0;
			if (!PIEWorld || !PIEWorld->AreActorsInitialized())
			{
				if (ElapsedSeconds >= PIEWorldReadyTimeoutSeconds)
				{
					Test->AddError(TEXT("UI-P0-03 DefensePIEViewData: 30초 안에 PIE World 초기화가 완료되지 않았습니다."));
					return true;
				}
				return false;
			}

			// [v1.6.0] 유효한 정식 DefenseData가 준비된 맵 배치 차량 수입니다.
			int32 InitializedDefenseVehicleCount = 0;
			// [v1.6.0] M_VehicleDefensePIE에서 정식 Shield·Armor Runtime을 가진 저장 Defense SUV의 PIE 복제입니다.
			ACFVehiclePawn* DefenseVehiclePawn = nullptr;
			for (TActorIterator<ACFVehiclePawn> VehiclePawnIterator(PIEWorld); VehiclePawnIterator; ++VehiclePawnIterator)
			{
				// [v1.6.0] 현재 순회 중인 실제 PIE 차량 Pawn입니다.
				ACFVehiclePawn* CandidateVehiclePawn = *VehiclePawnIterator;
				if (!IsValid(CandidateVehiclePawn))
				{
					continue;
				}

				// [v1.6.0] 후보 차량의 실제 Shield·Armor Runtime 컴포넌트입니다.
				UCFVehicleDefenseComp* CandidateDefenseComponent = CandidateVehiclePawn->GetVehicleDefenseComp();
				if (CandidateDefenseComponent
					&& CandidateDefenseComponent->IsDefenseInitialized()
					&& CandidateDefenseComponent->GetActiveDefenseData())
				{
					++InitializedDefenseVehicleCount;
					DefenseVehiclePawn = CandidateVehiclePawn;
				}
			}

			Test->TestEqual(TEXT("PIE 정식 Defense 차량 정확히 1대"), InitializedDefenseVehicleCount, 1);
			if (!Test->TestNotNull(TEXT("PIE 저장 Defense SUV 발견"), DefenseVehiclePawn))
			{
				return true;
			}

			// [v1.6.0] 저장 Defense SUV의 실제 Shield·Armor 컴포넌트입니다.
			UCFVehicleDefenseComp* DefenseComponent = DefenseVehiclePawn->GetVehicleDefenseComp();
			// [v1.6.0] 저장 Defense SUV의 실제 Integrity 컴포넌트입니다.
			UCFVehicleHealthComp* HealthComponent = DefenseVehiclePawn->GetVehicleHealthComp();
			if (!Test->TestNotNull(TEXT("PIE Defense Component"), DefenseComponent)
				|| !Test->TestNotNull(TEXT("PIE Health Component"), HealthComponent))
			{
				return true;
			}

			// [v1.6.0] ULocalPlayer/UISubsystem ClassWithin 계약을 만족하는 PIE 기술검증용 Transient LocalPlayer입니다.
			ULocalPlayer* TestLocalPlayer = GEngine ? NewObject<ULocalPlayer>(GEngine) : nullptr;
			// [v1.6.0] OnCurrentPawnChanged를 실제 Provider에 전달할 PIE 기술검증용 Transient UISubsystem입니다.
			UCFUISubsystem* UISubsystem = TestLocalPlayer ? NewObject<UCFUISubsystem>(TestLocalPlayer) : nullptr;
			// [v1.6.0] 실제 PIE Defense/Health Runtime을 ViewData로 변환할 HUD Provider입니다.
			UCFHUDDataProvider* DataProvider = NewObject<UCFHUDDataProvider>(GetTransientPackage());
			if (!Test->TestNotNull(TEXT("PIE HUD LocalPlayer"), TestLocalPlayer)
				|| !Test->TestNotNull(TEXT("PIE HUD UISubsystem"), UISubsystem)
				|| !Test->TestNotNull(TEXT("PIE HUD DataProvider"), DataProvider))
			{
				return true;
			}

			Test->TestTrue(TEXT("PIE HUD Provider 초기화"), DataProvider->InitializeProvider(UISubsystem));
			UISubsystem->OnCurrentPawnChanged.Broadcast(nullptr, DefenseVehiclePawn);

			// [v1.6.0] 저장된 실제 Defense SUV의 피해 전 Provider ViewData입니다.
			const FCFDefenseHUDData InitialDefenseViewData = DataProvider->GetCurrentViewData().Defense;
			Test->TestEqual(TEXT("PIE 초기 Defense Availability Known"), InitialDefenseViewData.Availability, ECFUIViewAvailability::Known);
			Test->TestTrue(TEXT("PIE 초기 Shield 100"), FMath::IsNearlyEqual(InitialDefenseViewData.CurrentShield, 100.0f));
			Test->TestTrue(TEXT("PIE 초기 Front Armor Ratio 1"), FMath::IsNearlyEqual(InitialDefenseViewData.FrontArmorRatio, 1.0f));
			Test->TestTrue(TEXT("PIE 초기 Integrity 100"), FMath::IsNearlyEqual(InitialDefenseViewData.CurrentIntegrity, 100.0f));

			// [v1.6.0] 자기 피해 차단과 독립된 PIE 공격 주체입니다.
			AActor* InstigatorActor = PIEWorld->SpawnActor<AActor>();
			// [v1.6.0] Shield 100 소진 뒤 남은 100을 AP 50으로 Armor 50 / Integrity 50에 분배할 Transient 피해 데이터입니다.
			UCFDamageData* DamageData = NewObject<UCFDamageData>(DefenseVehiclePawn, TEXT("HUDDefensePIEDamage"));
			DamageData->BaseDamage = 200.0f;
			DamageData->ArmorPenetration = 50.0f;
			if (!Test->TestNotNull(TEXT("PIE HUD 공격 주체"), InstigatorActor))
			{
				DataProvider->ShutdownProvider();
				return true;
			}

			// [v1.6.0] 실제 PIE Pawn에 정식 방어 진입점을 적용한 3층 피해 결과입니다.
			FCFVehicleDamageResult DamageResult;
			Test->TestTrue(
				TEXT("PIE 실제 Defense 피해 적용"),
				UCFVehicleDefenseComp::TryApplyDamageToActor(
					BuildHUDDefenseHitContext(DefenseVehiclePawn, InstigatorActor, DamageData),
					DamageResult));
			Test->TestTrue(TEXT("PIE Shield 흡수 100"), FMath::IsNearlyEqual(DamageResult.DamageAbsorbedByShield, 100.0f));
			Test->TestTrue(TEXT("PIE Armor 흡수 50"), FMath::IsNearlyEqual(DamageResult.DamageAbsorbedByArmor, 50.0f));
			Test->TestTrue(TEXT("PIE Integrity 적용 50"), FMath::IsNearlyEqual(DamageResult.DamageAppliedToIntegrity, 50.0f));

			// [v1.6.0] 실제 PIE Shield/Armor/Health 이벤트 직후 Provider가 보존한 손상 ViewData입니다.
			const FCFDefenseHUDData DamagedDefenseViewData = DataProvider->GetCurrentViewData().Defense;
			Test->TestEqual(TEXT("PIE Shield 0 KnownZero"), DamagedDefenseViewData.ShieldAvailability, ECFUIViewAvailability::KnownZero);
			Test->TestTrue(TEXT("PIE 피해 후 Shield Ratio 0"), FMath::IsNearlyZero(DamagedDefenseViewData.ShieldRatio));
			Test->TestTrue(TEXT("PIE 피해 후 Front Armor Ratio 0.5"), FMath::IsNearlyEqual(DamagedDefenseViewData.FrontArmorRatio, 0.5f));
			Test->TestTrue(TEXT("PIE 피해 후 Integrity Ratio 0.5"), FMath::IsNearlyEqual(DamagedDefenseViewData.IntegrityRatio, 0.5f));

			Test->AddInfo(FString::Printf(
				TEXT("UI-P0-03 Defense Technical PIE | Pawn=%s | Shield=%.1f/%.1f | FrontArmorRatio=%.3f | Integrity=%.1f/%.1f | Revision=%d"),
				*DefenseVehiclePawn->GetPathName(),
				DamagedDefenseViewData.CurrentShield,
				DamagedDefenseViewData.MaximumShield,
				DamagedDefenseViewData.FrontArmorRatio,
				DamagedDefenseViewData.CurrentIntegrity,
				DamagedDefenseViewData.MaximumIntegrity,
				DataProvider->GetCurrentViewData().Revision));

			DataProvider->ShutdownProvider();
			InstigatorActor->Destroy();
			return true;
		}

	private:
		// [v1.6.0] Latent PIE 검증 결과를 기록할 현재 Automation Test입니다.
		FAutomationTestBase* Test = nullptr;
		// [v1.6.0] PIE World 준비 제한 시간을 계산할 시작 시각입니다.
		double StartTimeSeconds = 0.0;
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDViewDataAvailabilityTest,
	"CarFight.UI.UI_P0_03.ViewDataAvailability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.0.0] 기본 ViewData가 실제 Provider 없는 채널을 Known 값으로 위조하지 않는지 검증합니다.
bool FCFHUDViewDataAvailabilityTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 기본 생성 상태의 전체 HUD ViewData입니다.
	const FCFInGameUIViewData ViewData;
	TestEqual(TEXT("Vehicle 기본 상태 Unavailable"), ViewData.Vehicle.Availability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Defense 기본 상태 Unavailable"), ViewData.Defense.Availability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Weapon 기본 상태 Unavailable"), ViewData.Weapon.Availability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Target 기본 상태 Unavailable"), ViewData.Target.Availability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Radar 기본 상태 Unavailable"), ViewData.Radar.Availability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Alert 기본 상태 Unavailable"), ViewData.Alerts.Availability, ECFUIViewAvailability::Unavailable);
				TestEqual(TEXT("Engine RPM 기본 상태 Unavailable"), ViewData.Vehicle.EngineRpmAvailability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Gear 기본 상태 Unavailable"), ViewData.Vehicle.GearAvailability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Ammo 기본 상태 Unavailable"), ViewData.Weapon.AmmoAvailability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Heat 기본 상태 Unavailable"), ViewData.Weapon.HeatAvailability, ECFUIViewAvailability::Unavailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006MissingProviderAvailabilityTest,
	"CarFight.UI.UI_P0_06.MissingProviderAvailability",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.7.0] UI-P0-06에서 실제 Provider 부재로 확정한 차량·무기 채널이 정적 설정이나 내부 ID만으로 Known 값으로 승격되지 않는지 검증합니다.
bool FCFHUDP006MissingProviderAvailabilityTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.7.0] 실제 Gameplay Provider가 연결되지 않은 기본 HUD ViewData입니다.
	const FCFInGameUIViewData ViewData;
	TestEqual(TEXT("UI-P0-06 Engine RPM Provider 없음"), ViewData.Vehicle.EngineRpmAvailability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("UI-P0-06 Gear Provider 없음"), ViewData.Vehicle.GearAvailability, ECFUIViewAvailability::Unavailable);
			TestEqual(TEXT("UI-P0-06 기본 Weapon DisplayName Source 없음"), ViewData.Weapon.DisplayNameAvailability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("UI-P0-06 Vehicle Battery Provider 없음"), ViewData.Weapon.VehicleBatteryAvailability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("UI-P0-06 Weapon Charge Provider 없음"), ViewData.Weapon.WeaponChargeAvailability, ECFUIViewAvailability::Unavailable);
				TestEqual(TEXT("UI-P0-06 Heat Runtime Provider 없음"), ViewData.Weapon.HeatAvailability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("UI-P0-06 Provider 없는 기본 Resource Channel 0개"), ViewData.Weapon.ResourceChannels.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006ResourceChannelProjectionTest,
	"CarFight.UI.UI_P0_06.ResourceChannelProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.9.0] 기존 실제 Weapon HUD 필드가 공통 Resource Channel로 값 손실 없이 투영되고 Runtime 없는 채널은 생성되지 않는지 검증합니다.
bool FCFHUDP006ResourceChannelProjectionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.9.0] 실제 finite Ammo·Cooldown·Reload·Launcher 값을 대표하도록 구성한 기존 Weapon ViewData입니다.
	FCFWeaponHUDData WeaponViewData;
	WeaponViewData.Availability = ECFUIViewAvailability::Known;
	WeaponViewData.AmmoAvailability = ECFUIViewAvailability::Known;
	WeaponViewData.LoadedAmmoCount = 5;
	WeaponViewData.MagazineCapacity = 10;
	WeaponViewData.ReserveAmmoCount = 7;
	WeaponViewData.CurrentOnboardAmmoCount = 12;
	WeaponViewData.CooldownAvailability = ECFUIViewAvailability::Known;
	WeaponViewData.CooldownDurationSeconds = 2.0f;
	WeaponViewData.RemainingCooldownSeconds = 0.5f;
	WeaponViewData.ReloadState = ECFWeaponReloadState::Reloading;
	WeaponViewData.ReloadDurationSeconds = 3.0f;
	WeaponViewData.RemainingReloadTimeSeconds = 1.5f;
	WeaponViewData.bWeaponActionLocked = true;
	WeaponViewData.WeaponActionLockReason = ECFWeaponActionLockReason::Reloading;
	WeaponViewData.LauncherAvailability = ECFUIViewAvailability::Known;
	WeaponViewData.bLauncherSequenceActive = true;
	WeaponViewData.LauncherTotalProjectileCount = 4;
	WeaponViewData.LauncherAcceptedProjectileCount = 2;
	WeaponViewData.LauncherRemainingProjectileCount = 2;
	WeaponViewData.RebuildResourceChannelsFromCurrentFields();

	TestEqual(TEXT("UI-P0-06 실제 Source 공통 Resource Channel 5개"), WeaponViewData.ResourceChannels.Num(), 5);
	if (WeaponViewData.ResourceChannels.Num() != 5)
	{
		return false;
	}

	// [v1.9.0] 첫 번째 canonical 채널인 실제 장전/탄창 Ammo입니다.
	const FCFWeaponResourceHUDData& AmmoChannel = WeaponViewData.ResourceChannels[0];
	TestEqual(TEXT("UI-P0-06 Ammo Channel Type"), AmmoChannel.ChannelType, ECFWeaponResourceChannelType::Ammo);
	TestEqual(TEXT("UI-P0-06 Ammo DisplayMode CountPair"), AmmoChannel.DisplayMode, ECFWeaponResourceDisplayMode::CountPair);
	TestTrue(TEXT("UI-P0-06 Ammo Current 5"), FMath::IsNearlyEqual(AmmoChannel.CurrentValue, 5.0f));
	TestTrue(TEXT("UI-P0-06 Ammo Maximum 10"), FMath::IsNearlyEqual(AmmoChannel.MaximumValue, 10.0f));
	TestTrue(TEXT("UI-P0-06 Ammo Ratio 0.5"), FMath::IsNearlyEqual(AmmoChannel.NormalizedValue, 0.5f));
	TestTrue(TEXT("UI-P0-06 Ammo Visible"), AmmoChannel.bIsVisible);

	// [v1.9.0] 두 번째 canonical 채널인 실제 차량 공유 Reserve Ammo입니다.
	const FCFWeaponResourceHUDData& ReserveAmmoChannel = WeaponViewData.ResourceChannels[1];
	TestEqual(TEXT("UI-P0-06 Reserve Channel Type"), ReserveAmmoChannel.ChannelType, ECFWeaponResourceChannelType::ReserveAmmo);
	TestTrue(TEXT("UI-P0-06 Reserve Current 7"), FMath::IsNearlyEqual(ReserveAmmoChannel.CurrentValue, 7.0f));
	TestTrue(TEXT("UI-P0-06 Reserve Visible"), ReserveAmmoChannel.bIsVisible);

	// [v1.9.0] 세 번째 canonical 채널인 실제 Weapon Cooldown입니다.
	const FCFWeaponResourceHUDData& CooldownChannel = WeaponViewData.ResourceChannels[2];
	TestEqual(TEXT("UI-P0-06 Cooldown Channel Type"), CooldownChannel.ChannelType, ECFWeaponResourceChannelType::Cooldown);
	TestTrue(TEXT("UI-P0-06 Cooldown Remaining 0.5"), FMath::IsNearlyEqual(CooldownChannel.RemainingTimeSeconds, 0.5f));
	TestTrue(TEXT("UI-P0-06 Cooldown Progress 0.75"), FMath::IsNearlyEqual(CooldownChannel.NormalizedValue, 0.75f));
	TestTrue(TEXT("UI-P0-06 Cooldown Active"), CooldownChannel.bIsActive);
	TestTrue(TEXT("UI-P0-06 Cooldown Blocks Fire"), CooldownChannel.bBlocksFire);

	// [v1.9.0] 네 번째 canonical 채널인 실제 Ammo Reload 상태입니다.
	const FCFWeaponResourceHUDData& ReloadChannel = WeaponViewData.ResourceChannels[3];
	TestEqual(TEXT("UI-P0-06 Reload Channel Type"), ReloadChannel.ChannelType, ECFWeaponResourceChannelType::Reload);
	TestTrue(TEXT("UI-P0-06 Reload Remaining 1.5"), FMath::IsNearlyEqual(ReloadChannel.RemainingTimeSeconds, 1.5f));
	TestTrue(TEXT("UI-P0-06 Reload Progress 0.5"), FMath::IsNearlyEqual(ReloadChannel.NormalizedValue, 0.5f));
	TestTrue(TEXT("UI-P0-06 Reload Visible"), ReloadChannel.bIsVisible);
	TestTrue(TEXT("UI-P0-06 Reload actual ActionLock blocks fire"), ReloadChannel.bBlocksFire);

	// [v1.9.0] 다섯 번째 canonical 채널인 실제 Launcher Sequence 상태입니다.
	const FCFWeaponResourceHUDData& LauncherSequenceChannel = WeaponViewData.ResourceChannels[4];
	TestEqual(TEXT("UI-P0-06 Launcher Channel Type"), LauncherSequenceChannel.ChannelType, ECFWeaponResourceChannelType::LauncherSequence);
	TestTrue(TEXT("UI-P0-06 Launcher Accepted 2"), FMath::IsNearlyEqual(LauncherSequenceChannel.CurrentValue, 2.0f));
	TestTrue(TEXT("UI-P0-06 Launcher Total 4"), FMath::IsNearlyEqual(LauncherSequenceChannel.MaximumValue, 4.0f));
	TestTrue(TEXT("UI-P0-06 Launcher Progress 0.5"), FMath::IsNearlyEqual(LauncherSequenceChannel.NormalizedValue, 0.5f));
	TestTrue(TEXT("UI-P0-06 Launcher Visible"), LauncherSequenceChannel.bIsVisible);
	TestFalse(TEXT("UI-P0-06 Launcher는 Reload ActionLock을 발사 차단으로 오인하지 않음"), LauncherSequenceChannel.bBlocksFire);

	for (const FCFWeaponResourceHUDData& ResourceChannel : WeaponViewData.ResourceChannels)
	{
		TestTrue(
			TEXT("UI-P0-06 Runtime 없는 Battery/Charge/Heat 채널 생성 금지"),
			ResourceChannel.ChannelType != ECFWeaponResourceChannelType::VehicleBattery
				&& ResourceChannel.ChannelType != ECFWeaponResourceChannelType::WeaponCharge
				&& ResourceChannel.ChannelType != ECFWeaponResourceChannelType::Heat);
	}
			return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006ResourcePresenterParityTest,
	"CarFight.UI.UI_P0_06.ResourcePresenterParity",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.11.0] Production Presenter가 legacy 개별 필드보다 ResourceChannels를 우선 소비하면서 기존 문구·우선순위를 유지하는지 검증합니다.
bool FCFHUDP006ResourcePresenterParityTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.11.0] Resource Channel 우선 해석을 증명하기 위해 legacy 필드는 의도적으로 충돌 값으로 둔 Weapon ViewData입니다.
	FCFWeaponHUDData WeaponViewData;
	WeaponViewData.AmmoAvailability = ECFUIViewAvailability::Unavailable;
	WeaponViewData.LoadedAmmoCount = 99;
	WeaponViewData.MagazineCapacity = 99;
	WeaponViewData.ReserveAmmoCount = 99;
	WeaponViewData.CurrentOnboardAmmoCount = 99;
	WeaponViewData.CooldownAvailability = ECFUIViewAvailability::Unavailable;
	WeaponViewData.CooldownDurationSeconds = 99.0f;
	WeaponViewData.RemainingCooldownSeconds = 99.0f;
		WeaponViewData.ReloadState = ECFWeaponReloadState::Ready;
	WeaponViewData.ReloadDurationSeconds = 99.0f;
	WeaponViewData.RemainingReloadTimeSeconds = 99.0f;
	WeaponViewData.LauncherAvailability = ECFUIViewAvailability::Unavailable;
	WeaponViewData.bLauncherSequenceActive = false;
	WeaponViewData.LauncherPattern = ECFLauncherFirePattern::Salvo;
	WeaponViewData.LauncherTotalProjectileCount = 99;
	WeaponViewData.LauncherAcceptedProjectileCount = 99;

	// [v1.11.0] Presenter가 Primary Ammo `3 / 8`을 만들 실제 공통 Ammo 채널입니다.
	FCFWeaponResourceHUDData AmmoChannel;
	AmmoChannel.ChannelType = ECFWeaponResourceChannelType::Ammo;
	AmmoChannel.DisplayMode = ECFWeaponResourceDisplayMode::CountPair;
	AmmoChannel.Availability = ECFUIViewAvailability::Known;
	AmmoChannel.bHasCurrentValue = true;
	AmmoChannel.CurrentValue = 3.0f;
	AmmoChannel.bHasMaximumValue = true;
	AmmoChannel.MaximumValue = 8.0f;
	AmmoChannel.bIsActive = true;
	AmmoChannel.bIsVisible = true;
	AmmoChannel.bBlocksFire = false;
	WeaponViewData.ResourceChannels.Add(AmmoChannel);

	// [v1.11.0] Presenter가 label-less Reserve `6`을 만들 실제 공통 ReserveAmmo 채널입니다.
	FCFWeaponResourceHUDData ReserveAmmoChannel;
	ReserveAmmoChannel.ChannelType = ECFWeaponResourceChannelType::ReserveAmmo;
	ReserveAmmoChannel.DisplayMode = ECFWeaponResourceDisplayMode::Count;
	ReserveAmmoChannel.Availability = ECFUIViewAvailability::Known;
	ReserveAmmoChannel.bHasCurrentValue = true;
	ReserveAmmoChannel.CurrentValue = 6.0f;
	ReserveAmmoChannel.bIsActive = true;
	ReserveAmmoChannel.bIsVisible = true;
	WeaponViewData.ResourceChannels.Add(ReserveAmmoChannel);

	// [v1.11.0] Presenter 우선순위 최상단 Reload를 검증할 실제 공통 Reload 채널입니다.
	FCFWeaponResourceHUDData ReloadChannel;
	ReloadChannel.ChannelType = ECFWeaponResourceChannelType::Reload;
	ReloadChannel.DisplayMode = ECFWeaponResourceDisplayMode::TimeRemaining;
	ReloadChannel.Availability = ECFUIViewAvailability::Known;
	ReloadChannel.bHasMaximumValue = true;
	ReloadChannel.MaximumValue = 4.0f;
	ReloadChannel.bHasNormalizedValue = true;
	ReloadChannel.NormalizedValue = 0.75f;
	ReloadChannel.bHasRemainingTimeSeconds = true;
	ReloadChannel.RemainingTimeSeconds = 1.0f;
	ReloadChannel.bIsActive = true;
	ReloadChannel.bIsVisible = true;
	ReloadChannel.bBlocksFire = true;
	WeaponViewData.ResourceChannels.Add(ReloadChannel);

	// [v1.11.0] Reload가 끝난 뒤 Cooldown/READY를 검증할 실제 공통 Cooldown 채널입니다.
	FCFWeaponResourceHUDData CooldownChannel;
	CooldownChannel.ChannelType = ECFWeaponResourceChannelType::Cooldown;
	CooldownChannel.DisplayMode = ECFWeaponResourceDisplayMode::TimeRemaining;
	CooldownChannel.Availability = ECFUIViewAvailability::Known;
	CooldownChannel.bHasMaximumValue = true;
	CooldownChannel.MaximumValue = 2.0f;
	CooldownChannel.bHasNormalizedValue = true;
	CooldownChannel.NormalizedValue = 0.75f;
	CooldownChannel.bHasRemainingTimeSeconds = true;
	CooldownChannel.RemainingTimeSeconds = 0.5f;
	CooldownChannel.bIsActive = true;
	CooldownChannel.bIsVisible = true;
	CooldownChannel.bBlocksFire = true;
	WeaponViewData.ResourceChannels.Add(CooldownChannel);

	// [v1.11.0] Launcher 문구와 진행률을 legacy 99/99 대신 2/4로 만들 실제 공통 LauncherSequence 채널입니다.
	FCFWeaponResourceHUDData LauncherSequenceChannel;
	LauncherSequenceChannel.ChannelType = ECFWeaponResourceChannelType::LauncherSequence;
	LauncherSequenceChannel.DisplayMode = ECFWeaponResourceDisplayMode::Sequence;
	LauncherSequenceChannel.Availability = ECFUIViewAvailability::Known;
	LauncherSequenceChannel.bHasCurrentValue = true;
	LauncherSequenceChannel.CurrentValue = 2.0f;
	LauncherSequenceChannel.bHasMaximumValue = true;
	LauncherSequenceChannel.MaximumValue = 4.0f;
	LauncherSequenceChannel.bHasNormalizedValue = true;
	LauncherSequenceChannel.NormalizedValue = 0.5f;
	LauncherSequenceChannel.bIsActive = true;
	LauncherSequenceChannel.bIsVisible = true;
	WeaponViewData.ResourceChannels.Add(LauncherSequenceChannel);

	// [v1.11.0] 공통 Ammo 채널에서 해석된 Primary Ammo 문구입니다.
	FText AmmoText;
	TestTrue(TEXT("UI-P0-06 Presenter Resource Ammo 표시"), UCFHUDPresenter::ResolveAmmoPresentation(WeaponViewData, AmmoText));
	TestEqual(TEXT("UI-P0-06 Presenter Resource Ammo 3 / 8"), AmmoText.ToString(), FString(TEXT("3 / 8")));

	// [v1.11.0] 공통 ReserveAmmo 채널에서 해석된 label-less Reserve 문구입니다.
	FText ReserveAmmoText;
	TestTrue(TEXT("UI-P0-06 Presenter Resource Reserve 표시"), UCFHUDPresenter::ResolveReserveAmmoPresentation(WeaponViewData, ReserveAmmoText));
	TestEqual(TEXT("UI-P0-06 Presenter Resource Reserve 6"), ReserveAmmoText.ToString(), FString(TEXT("6")));

	// [v1.11.0] Reload > NoAmmo > Cooldown 우선순위 첫 단계에서 해석된 상태 문구입니다.
	FText WeaponStatusText;
	// [v1.11.0] Reload Resource Channel이 제공한 실제 0~1 진행률입니다.
	float WeaponStatusProgress = 0.0f;
	TestTrue(TEXT("UI-P0-06 Presenter Resource Reload 표시"), UCFHUDPresenter::ResolveWeaponStatusPresentation(WeaponViewData, false, WeaponStatusText, WeaponStatusProgress));
	TestEqual(TEXT("UI-P0-06 Presenter Resource Reload 문구"), WeaponStatusText.ToString(), FString(TEXT("RELOAD 1.0 / 4.0 s")));
	TestTrue(TEXT("UI-P0-06 Presenter Resource Reload Progress 0.75"), FMath::IsNearlyEqual(WeaponStatusProgress, 0.75f));

	// [v1.11.0] Reload 종료 뒤 NoAmmo를 검증하기 위해 공통 Reload 채널을 비활성화합니다.
	WeaponViewData.ResourceChannels[2].bIsActive = false;
	WeaponViewData.ResourceChannels[2].bIsVisible = false;
	// [v1.11.0] 실제 Ammo Resource의 fire blocker를 켜 NoAmmo 상태를 만듭니다.
	WeaponViewData.ResourceChannels[0].bBlocksFire = true;
	TestTrue(TEXT("UI-P0-06 Presenter Resource NoAmmo 표시"), UCFHUDPresenter::ResolveWeaponStatusPresentation(WeaponViewData, false, WeaponStatusText, WeaponStatusProgress));
	TestEqual(TEXT("UI-P0-06 Presenter Resource NoAmmo 문구"), WeaponStatusText.ToString(), FString(TEXT("NO AMMO")));

	// [v1.11.0] NoAmmo 해제 뒤 Cooldown Resource가 상태를 소유하도록 실제 Ammo blocker를 해제합니다.
	WeaponViewData.ResourceChannels[0].bBlocksFire = false;
	TestTrue(TEXT("UI-P0-06 Presenter Resource Cooldown 표시"), UCFHUDPresenter::ResolveWeaponStatusPresentation(WeaponViewData, false, WeaponStatusText, WeaponStatusProgress));
	TestEqual(TEXT("UI-P0-06 Presenter Resource Cooldown 문구"), WeaponStatusText.ToString(), FString(TEXT("0.5 s")));
	TestTrue(TEXT("UI-P0-06 Presenter Resource Cooldown Progress 0.75"), FMath::IsNearlyEqual(WeaponStatusProgress, 0.75f));

	// [v1.11.0] Resource Launcher current/max와 legacy FirePattern을 조합한 Player-facing Sequence 문구입니다.
	FText LauncherSequenceText;
	// [v1.11.0] Resource Launcher current/max에서 계산한 실제 Sequence 진행률입니다.
	float LauncherSequenceProgress = 0.0f;
	TestTrue(TEXT("UI-P0-06 Presenter Resource Launcher 표시"), UCFHUDPresenter::ResolveLauncherSequencePresentation(WeaponViewData, LauncherSequenceText, LauncherSequenceProgress));
	TestEqual(TEXT("UI-P0-06 Presenter Resource Launcher SALVO 2 / 4"), LauncherSequenceText.ToString(), FString(TEXT("SALVO 2 / 4")));
		TestTrue(TEXT("UI-P0-06 Presenter Resource Launcher Progress 0.5"), FMath::IsNearlyEqual(LauncherSequenceProgress, 0.5f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006ResourcePresentationProjectionTest,
	"CarFight.UI.UI_P0_06.ResourcePresentationProjection",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.12.0] raw ResourceChannels를 직접 Row로 만들지 않고 Compact Primary/Secondary/FireState Projection과 기존 Launcher lifecycle을 보존하는지 검증합니다.
bool FCFHUDP006ResourcePresentationProjectionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.12.0] LauncherSequenceRevision Presentation lifecycle을 실제 Stage A Projection에서 한 번씩 소비할 Presenter입니다.
	UCFHUDPresenter* Presenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-06 Resource Projection Presenter"), Presenter))
	{
		return false;
	}

	// [v1.12.0] Ammo Primary, Header Reserve, Cooldown FireState와 후속 Launcher 전이를 함께 검증할 Weapon ViewData입니다.
	FCFWeaponHUDData WeaponViewData;
	WeaponViewData.Availability = ECFUIViewAvailability::Known;
	WeaponViewData.WeaponId = FName(TEXT("ProjectionWeapon"));
	WeaponViewData.LauncherPattern = ECFLauncherFirePattern::Salvo;

	// [v1.12.0] 평상시 Compact Primary `3 / 8`을 제공하는 실제 공통 Ammo 채널입니다.
	FCFWeaponResourceHUDData AmmoChannel;
	AmmoChannel.ChannelType = ECFWeaponResourceChannelType::Ammo;
	AmmoChannel.DisplayMode = ECFWeaponResourceDisplayMode::CountPair;
	AmmoChannel.Availability = ECFUIViewAvailability::Known;
	AmmoChannel.bHasCurrentValue = true;
	AmmoChannel.CurrentValue = 3.0f;
	AmmoChannel.bHasMaximumValue = true;
	AmmoChannel.MaximumValue = 8.0f;
	AmmoChannel.bIsActive = true;
	AmmoChannel.bIsVisible = true;
	AmmoChannel.bBlocksFire = false;
	WeaponViewData.ResourceChannels.Add(AmmoChannel);

	// [v1.12.0] Presentation Entry가 아니라 기존 Header 우측 label-less owner로 남아야 하는 ReserveAmmo 채널입니다.
	FCFWeaponResourceHUDData ReserveAmmoChannel;
	ReserveAmmoChannel.ChannelType = ECFWeaponResourceChannelType::ReserveAmmo;
	ReserveAmmoChannel.DisplayMode = ECFWeaponResourceDisplayMode::Count;
	ReserveAmmoChannel.Availability = ECFUIViewAvailability::Known;
	ReserveAmmoChannel.bHasCurrentValue = true;
	ReserveAmmoChannel.CurrentValue = 6.0f;
	ReserveAmmoChannel.bIsActive = true;
	ReserveAmmoChannel.bIsVisible = true;
	WeaponViewData.ResourceChannels.Add(ReserveAmmoChannel);

	// [v1.12.0] Launcher가 없을 때 기존 Weapon Status가 `0.5 s` FireState를 만들 Cooldown 채널입니다.
	FCFWeaponResourceHUDData CooldownChannel;
	CooldownChannel.ChannelType = ECFWeaponResourceChannelType::Cooldown;
	CooldownChannel.DisplayMode = ECFWeaponResourceDisplayMode::TimeRemaining;
	CooldownChannel.Availability = ECFUIViewAvailability::Known;
	CooldownChannel.bHasMaximumValue = true;
	CooldownChannel.MaximumValue = 2.0f;
	CooldownChannel.bHasNormalizedValue = true;
	CooldownChannel.NormalizedValue = 0.75f;
	CooldownChannel.bHasRemainingTimeSeconds = true;
	CooldownChannel.RemainingTimeSeconds = 0.5f;
	CooldownChannel.bIsActive = true;
	CooldownChannel.bIsVisible = true;
	CooldownChannel.bBlocksFire = true;
	WeaponViewData.ResourceChannels.Add(CooldownChannel);

	// [v1.12.0] Stage A Presenter가 현재 ViewData에서 만든 Compact Presentation Entry 목록입니다.
	TArray<FCFWeaponResourcePresentationEntry> PresentationEntries;
	Presenter->BuildWeaponResourceEntries(WeaponViewData, PresentationEntries);

	TestEqual(TEXT("UI-P0-06 Projection 평상시 Entry 2개"), PresentationEntries.Num(), 2);
	if (PresentationEntries.Num() != 2)
	{
		return false;
	}

	TestEqual(TEXT("UI-P0-06 Projection 평상시 Primary 역할"), PresentationEntries[0].Role, ECFWeaponResourcePresentationRole::Primary);
	TestEqual(TEXT("UI-P0-06 Projection 평상시 Primary Ammo"), PresentationEntries[0].SourceChannelType, ECFWeaponResourceChannelType::Ammo);
	TestEqual(TEXT("UI-P0-06 Projection 평상시 Ammo 3 / 8"), PresentationEntries[0].DisplayText.ToString(), FString(TEXT("3 / 8")));
	TestFalse(TEXT("UI-P0-06 Projection Ammo Progress 없음"), PresentationEntries[0].bHasProgress);
	TestEqual(TEXT("UI-P0-06 Projection 평상시 FireState 역할"), PresentationEntries[1].Role, ECFWeaponResourcePresentationRole::FireState);
	TestEqual(TEXT("UI-P0-06 Projection 복합 FireState Source None"), PresentationEntries[1].SourceChannelType, ECFWeaponResourceChannelType::None);
	TestEqual(TEXT("UI-P0-06 Projection Cooldown 0.5 s"), PresentationEntries[1].DisplayText.ToString(), FString(TEXT("0.5 s")));
	TestTrue(TEXT("UI-P0-06 Projection Cooldown Progress 0.75"), FMath::IsNearlyEqual(PresentationEntries[1].Progress01, 0.75f));

	// [v1.12.0] Reserve/Battery/Charge/Heat가 Compact Entry로 새지 않았는지 세는 현재 Secondary 개수입니다.
	int32 SecondaryEntryCount = 0;
	for (const FCFWeaponResourcePresentationEntry& PresentationEntry : PresentationEntries)
	{
		if (PresentationEntry.Role == ECFWeaponResourcePresentationRole::Secondary)
		{
			++SecondaryEntryCount;
		}
		TestTrue(
			TEXT("UI-P0-06 Projection Reserve/Battery/Charge/Heat Entry 금지"),
			PresentationEntry.SourceChannelType != ECFWeaponResourceChannelType::ReserveAmmo
				&& PresentationEntry.SourceChannelType != ECFWeaponResourceChannelType::VehicleBattery
				&& PresentationEntry.SourceChannelType != ECFWeaponResourceChannelType::WeaponCharge
				&& PresentationEntry.SourceChannelType != ECFWeaponResourceChannelType::Heat);
	}
	TestTrue(TEXT("UI-P0-06 Projection Secondary 최대 2"), SecondaryEntryCount <= 2);

	// [v1.12.0] Cooldown과 동시에 존재해도 기존 우선순위로 단일 FireState만 소유할 실제 Reload 채널입니다.
	FCFWeaponResourceHUDData ReloadChannel;
	ReloadChannel.ChannelType = ECFWeaponResourceChannelType::Reload;
	ReloadChannel.DisplayMode = ECFWeaponResourceDisplayMode::TimeRemaining;
	ReloadChannel.Availability = ECFUIViewAvailability::Known;
	ReloadChannel.bHasMaximumValue = true;
	ReloadChannel.MaximumValue = 4.0f;
	ReloadChannel.bHasNormalizedValue = true;
	ReloadChannel.NormalizedValue = 0.75f;
	ReloadChannel.bHasRemainingTimeSeconds = true;
	ReloadChannel.RemainingTimeSeconds = 1.0f;
	ReloadChannel.bIsActive = true;
	ReloadChannel.bIsVisible = true;
	ReloadChannel.bBlocksFire = true;
	WeaponViewData.ResourceChannels.Add(ReloadChannel);
	Presenter->BuildWeaponResourceEntries(WeaponViewData, PresentationEntries);

	TestEqual(TEXT("UI-P0-06 Projection Reload 동시 Entry 2개"), PresentationEntries.Num(), 2);
	if (PresentationEntries.Num() != 2)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 Projection Reload 단일 FireState"), PresentationEntries[1].Role, ECFWeaponResourcePresentationRole::FireState);
	TestEqual(TEXT("UI-P0-06 Projection Reload 우선 문구"), PresentationEntries[1].DisplayText.ToString(), FString(TEXT("RELOAD 1.0 / 4.0 s")));
	TestTrue(TEXT("UI-P0-06 Projection Reload Progress 0.75"), FMath::IsNearlyEqual(PresentationEntries[1].Progress01, 0.75f));

	// [v1.12.0] Launcher Active에서 기존 Sequence lifecycle을 Compact Primary로 승격할 실제 LauncherSequence 채널입니다.
	FCFWeaponResourceHUDData LauncherSequenceChannel;
	LauncherSequenceChannel.ChannelType = ECFWeaponResourceChannelType::LauncherSequence;
	LauncherSequenceChannel.DisplayMode = ECFWeaponResourceDisplayMode::Sequence;
	LauncherSequenceChannel.Availability = ECFUIViewAvailability::Known;
	LauncherSequenceChannel.bHasCurrentValue = true;
	LauncherSequenceChannel.CurrentValue = 2.0f;
	LauncherSequenceChannel.bHasMaximumValue = true;
	LauncherSequenceChannel.MaximumValue = 4.0f;
	LauncherSequenceChannel.bHasNormalizedValue = true;
	LauncherSequenceChannel.NormalizedValue = 0.5f;
	LauncherSequenceChannel.bIsActive = true;
	LauncherSequenceChannel.bIsVisible = true;
	LauncherSequenceChannel.bBlocksFire = true;
	WeaponViewData.ResourceChannels.Add(LauncherSequenceChannel);
	WeaponViewData.LauncherSequenceRevision = 10;
	Presenter->BuildWeaponResourceEntries(WeaponViewData, PresentationEntries);

	TestEqual(TEXT("UI-P0-06 Projection Launcher Active Entry 2개"), PresentationEntries.Num(), 2);
	if (PresentationEntries.Num() != 2)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 Projection Launcher Primary"), PresentationEntries[0].Role, ECFWeaponResourcePresentationRole::Primary);
	TestEqual(TEXT("UI-P0-06 Projection Launcher Source"), PresentationEntries[0].SourceChannelType, ECFWeaponResourceChannelType::LauncherSequence);
	TestEqual(TEXT("UI-P0-06 Projection Launcher SALVO 2 / 4"), PresentationEntries[0].DisplayText.ToString(), FString(TEXT("SALVO 2 / 4")));
	TestTrue(TEXT("UI-P0-06 Projection Launcher Progress 0.5"), FMath::IsNearlyEqual(PresentationEntries[0].Progress01, 0.5f));
	TestEqual(TEXT("UI-P0-06 Projection Launcher 중 Ammo Secondary"), PresentationEntries[1].Role, ECFWeaponResourcePresentationRole::Secondary);
	TestEqual(TEXT("UI-P0-06 Projection Launcher 중 Ammo 유지"), PresentationEntries[1].SourceChannelType, ECFWeaponResourceChannelType::Ammo);
	TestEqual(TEXT("UI-P0-06 Projection Launcher 중 Ammo 3 / 8"), PresentationEntries[1].DisplayText.ToString(), FString(TEXT("3 / 8")));

	// [v1.12.0] Active 뒤 실제 terminal Revision에서 정확히 한 ViewData 주기 표시할 Launcher 채널 배열 인덱스입니다.
	const int32 LauncherChannelIndex = WeaponViewData.ResourceChannels.Num() - 1;
	WeaponViewData.ResourceChannels[LauncherChannelIndex].bIsActive = false;
	WeaponViewData.ResourceChannels[LauncherChannelIndex].CurrentValue = 4.0f;
	WeaponViewData.ResourceChannels[LauncherChannelIndex].NormalizedValue = 1.0f;
	WeaponViewData.LauncherSequenceRevision = 11;
	Presenter->BuildWeaponResourceEntries(WeaponViewData, PresentationEntries);

	TestEqual(TEXT("UI-P0-06 Projection Launcher Terminal Entry 2개"), PresentationEntries.Num(), 2);
	if (PresentationEntries.Num() != 2)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 Projection terminal SALVO 4 / 4"), PresentationEntries[0].DisplayText.ToString(), FString(TEXT("SALVO 4 / 4")));
	TestTrue(TEXT("UI-P0-06 Projection terminal Progress 1"), FMath::IsNearlyEqual(PresentationEntries[0].Progress01, 1.0f));
	TestEqual(TEXT("UI-P0-06 Projection terminal에서도 Ammo Secondary"), PresentationEntries[1].SourceChannelType, ECFWeaponResourceChannelType::Ammo);

	// [v1.12.0] Terminal 한 주기 뒤 Cooldown 복귀를 확인하기 위해 Reload를 비활성화할 배열 인덱스입니다.
	const int32 ReloadChannelIndex = LauncherChannelIndex - 1;
	WeaponViewData.ResourceChannels[ReloadChannelIndex].bIsActive = false;
	WeaponViewData.ResourceChannels[ReloadChannelIndex].bIsVisible = false;
	WeaponViewData.ResourceChannels[ReloadChannelIndex].bBlocksFire = false;
	Presenter->BuildWeaponResourceEntries(WeaponViewData, PresentationEntries);

	TestEqual(TEXT("UI-P0-06 Projection terminal 다음 Entry 2개"), PresentationEntries.Num(), 2);
	if (PresentationEntries.Num() != 2)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 Projection terminal 다음 Ammo Primary"), PresentationEntries[0].SourceChannelType, ECFWeaponResourceChannelType::Ammo);
	TestEqual(TEXT("UI-P0-06 Projection terminal 다음 FireState"), PresentationEntries[1].Role, ECFWeaponResourcePresentationRole::FireState);
	TestEqual(TEXT("UI-P0-06 Projection terminal 다음 Cooldown"), PresentationEntries[1].DisplayText.ToString(), FString(TEXT("0.5 s")));
		TestTrue(TEXT("UI-P0-06 Projection terminal 다음 Cooldown Progress"), FMath::IsNearlyEqual(PresentationEntries[1].Progress01, 0.75f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006HeatResourceTest,
	"CarFight.UI.UI_P0_06.HeatRuntimeResourceContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.18.0] explicit Heat Runtime의 누적·냉각·과열과 실제 Resource Channel→Compact Presentation을 기존 Ammo/Reload/Cooldown/Launcher 계약 안에서 검증합니다.
bool FCFHUDP006HeatResourceTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.18.0] 실제 WeaponComp와 같은 순수 상태 전이 계약을 검증할 Heat Runtime입니다.
	FCFWeaponHeatRuntime HeatRuntime;
	HeatRuntime.Configure(25.0f, 100.0f, 10.0f);
	TestTrue(TEXT("UI-P0-06 Heat Runtime explicit config 활성"), HeatRuntime.IsEnabled());
	TestTrue(TEXT("UI-P0-06 Heat Runtime 초기 발사 가능"), HeatRuntime.CanAcceptShot());
	TestTrue(TEXT("UI-P0-06 Heat Runtime recovery threshold 75"), FMath::IsNearlyEqual(HeatRuntime.GetRecoveryHeatThreshold(), 75.0f));

	HeatRuntime.RecordAcceptedShot();
	HeatRuntime.RecordAcceptedShot();
	HeatRuntime.RecordAcceptedShot();
	TestTrue(TEXT("UI-P0-06 Heat 3발 누적 75"), FMath::IsNearlyEqual(HeatRuntime.GetCurrentHeat(), 75.0f));
	TestFalse(TEXT("UI-P0-06 Heat 75는 아직 과열 아님"), HeatRuntime.IsOverheated());

	HeatRuntime.RecordAcceptedShot();
	TestTrue(TEXT("UI-P0-06 Heat 4발 Max 100"), FMath::IsNearlyEqual(HeatRuntime.GetCurrentHeat(), 100.0f));
	TestTrue(TEXT("UI-P0-06 MaxHeat 도달 과열"), HeatRuntime.IsOverheated());
	TestFalse(TEXT("UI-P0-06 과열 중 발사 차단"), HeatRuntime.CanAcceptShot());

	HeatRuntime.RecordAcceptedShot();
	TestTrue(TEXT("UI-P0-06 과열 중 추가 누적 없음"), FMath::IsNearlyEqual(HeatRuntime.GetCurrentHeat(), 100.0f));
	HeatRuntime.AdvanceCooling(2.0f);
	TestTrue(TEXT("UI-P0-06 2초 자연 냉각 80"), FMath::IsNearlyEqual(HeatRuntime.GetCurrentHeat(), 80.0f));
	TestTrue(TEXT("UI-P0-06 recovery threshold 위에서는 과열 유지"), HeatRuntime.IsOverheated());
	HeatRuntime.AdvanceCooling(0.5f);
	TestTrue(TEXT("UI-P0-06 2.5초 자연 냉각 75"), FMath::IsNearlyEqual(HeatRuntime.GetCurrentHeat(), 75.0f));
	TestFalse(TEXT("UI-P0-06 다음 한 발 headroom에서 과열 해제"), HeatRuntime.IsOverheated());
	TestTrue(TEXT("UI-P0-06 냉각 후 재사용 가능"), HeatRuntime.CanAcceptShot());

	// [v1.18.0] 기존 Asset처럼 냉각 입력이 0인 경우 Heat Runtime이 발사 결과를 바꾸지 않는 호환 상태입니다.
	FCFWeaponHeatRuntime DisabledHeatRuntime;
	DisabledHeatRuntime.Configure(25.0f, 100.0f, 0.0f);
		TestFalse(TEXT("UI-P0-06 HeatDissipation 0은 Runtime 비활성"), DisabledHeatRuntime.IsEnabled());
	TestTrue(TEXT("UI-P0-06 Heat Runtime 비활성은 발사 차단 안 함"), DisabledHeatRuntime.CanAcceptShot());

	// [v1.18.1] 실제 ACFVehiclePawn accepted-fire 경로와 UCFVehicleWeaponComp Heat owner를 함께 검증할 transient Automation World입니다.
	UWorld* HeatIntegrationWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-06 Heat Integration World"), HeatIntegrationWorld))
	{
		return false;
	}

	// [v1.18.1] 실제 WeaponComp와 Fire 실행 경로를 기본 서브오브젝트로 소유할 transient 차량입니다.
	ACFVehiclePawn* HeatVehiclePawn = HeatIntegrationWorld->SpawnActor<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("UI-P0-06 Heat Integration Vehicle Pawn"), HeatVehiclePawn))
	{
		return false;
	}

	// [v1.18.1] 실제 per-weapon Heat 상태를 소유할 차량 무기 컴포넌트입니다.
	UCFVehicleWeaponComp* HeatVehicleWeaponComp = HeatVehiclePawn->GetVehicleWeaponComp();
	if (!TestNotNull(TEXT("UI-P0-06 Heat Integration WeaponComp"), HeatVehicleWeaponComp))
	{
		HeatVehiclePawn->Destroy();
		return false;
	}

	// [v1.18.1] 테스트 무기를 활성 MountProfile의 단일 EquipmentPreset source로 연결할 transient Preset입니다.
	UCFEquipmentPresetData* HeatEquipmentPresetData = NewObject<UCFEquipmentPresetData>(GetTransientPackage());
	HeatEquipmentPresetData->EquipmentId = TEXT("UI_P006_HeatPreset");
	HeatEquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
	HeatEquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Large;

	// [v1.18.1] 기존 Asset 값을 건드리지 않고 explicit Heat 25/100/10을 실제 WeaponComp에 공급할 transient WeaponData입니다.
	UCFWeaponData* HeatWeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	HeatWeaponData->WeaponId = TEXT("UI_P006_HeatWeapon");
	HeatWeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
	HeatWeaponData->CompatibleMountTypes.Reset();
	HeatWeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
	HeatWeaponData->FireMode = ECFWeaponFireMode::HitScan;
	HeatWeaponData->FireRatePerMinute = 0.0f;
	HeatWeaponData->bUseInfiniteAmmoForDebug = true;
	HeatWeaponData->HeatPerShot = 25.0f;
	HeatWeaponData->MaxHeat = 100.0f;
	HeatWeaponData->HeatDissipationPerSecond = 10.0f;
	HeatEquipmentPresetData->DefaultWeaponData = HeatWeaponData;

	// [v1.18.1] 활성 MountProfile이 참조할 최소 위치 슬롯과 WeaponData source를 제공할 transient VehicleData입니다.
	UCFVehicleData* HeatVehicleData = NewObject<UCFVehicleData>(GetTransientPackage());
	// [v1.18.1] 테스트 MountProfile이 사용할 최소 하드포인트 슬롯입니다.
	FCFVehicleHardpointSlot HeatHardpointSlot;
	HeatHardpointSlot.LocationSlotId = TEXT("Top_Heat_01");
	HeatHardpointSlot.LocationCategory = TEXT("Top");
	HeatHardpointSlot.LocalLocation = FVector::ZeroVector;
	HeatVehicleData->HardpointSlots.Add(HeatHardpointSlot);

	// [v1.18.1] WeaponComp 기본 활성 ID와 일치해 transient Heat WeaponData를 실제 활성 무기로 만드는 MountProfile입니다.
	FCFVehicleMountProfile HeatMountProfile;
	HeatMountProfile.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
	HeatMountProfile.LocationSlotRef = HeatHardpointSlot.LocationSlotId;
	HeatMountProfile.MountType = ECFVehicleMountType::Turret;
	HeatMountProfile.SizeLimit = ECFVehicleWeaponSize::Large;
	HeatMountProfile.DefaultEquipmentPresetData = HeatEquipmentPresetData;
	HeatVehicleData->MountProfiles.Add(HeatMountProfile);

	TestTrue(TEXT("UI-P0-06 actual WeaponComp Heat Runtime 초기화"), HeatVehicleWeaponComp->InitializeWeaponRuntime(HeatVehiclePawn, HeatVehicleData));
	TestTrue(TEXT("UI-P0-06 actual WeaponComp Heat Runtime 활성"), HeatVehicleWeaponComp->IsActiveWeaponHeatRuntimeEnabled());

	for (int32 AcceptedShotIndex = 0; AcceptedShotIndex < 4; ++AcceptedShotIndex)
	{
		// [v1.18.1] 빈 Automation World에서 실제 HitScan Miss를 발생시키되 승인 발사로 처리할 요청입니다.
		FCFVehicleFireRequest HeatFireRequest;
		HeatFireRequest.FireRequestId = AcceptedShotIndex + 1;
		HeatFireRequest.AimOrigin = FVector(0.0f, 0.0f, 2000.0f);
		HeatFireRequest.AimDirection = FVector::ForwardVector;
		HeatFireRequest.PredictedAimTargetLocation = FVector(5000.0f, 0.0f, 2000.0f);
		HeatFireRequest.ClientFireTimeSeconds = static_cast<float>(AcceptedShotIndex + 1);
		HeatFireRequest.WeaponGroupId = HeatMountProfile.MountProfileId;

		// [v1.18.1] Validate 승인 직후와 같은 상태에서 Execute→Apply 실제 발사 후처리 경계를 검증할 결과입니다.
		FCFVehicleFireResult HeatFireResult;
		HeatFireResult.FireRequestId = HeatFireRequest.FireRequestId;
		HeatFireResult.ValidationAimTargetLocation = HeatFireRequest.PredictedAimTargetLocation;
		HeatFireResult.LocalHitLocation = HeatFireRequest.PredictedAimTargetLocation;
		HeatFireResult.bAccepted = true;
		HeatFireResult.RejectReason = ECFVehicleFireRejectReason::None;

		TestTrue(TEXT("UI-P0-06 actual accepted HitScan 실행"), HeatVehiclePawn->ExecuteAcceptedFireCommand(HeatFireRequest, HeatFireResult, nullptr, true));
		TestTrue(TEXT("UI-P0-06 actual accepted HitScan 결과 유지"), HeatFireResult.bAccepted);
		HeatVehiclePawn->ApplyFireResult(HeatFireRequest, HeatFireResult);
		TestTrue(
			TEXT("UI-P0-06 actual accepted-fire Heat 누적"),
			FMath::IsNearlyEqual(HeatVehicleWeaponComp->GetCurrentWeaponHeat(), 25.0f * static_cast<float>(AcceptedShotIndex + 1)));
	}

	TestTrue(TEXT("UI-P0-06 actual WeaponComp MaxHeat 과열"), HeatVehicleWeaponComp->IsActiveWeaponOverheated());
	TestFalse(TEXT("UI-P0-06 actual WeaponComp 과열 발사 허용 안 함"), HeatVehicleWeaponComp->CanActiveWeaponAcceptHeatShot());
	HeatVehicleWeaponComp->TickComponent(2.5f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("UI-P0-06 actual WeaponComp 2.5초 냉각 75"), FMath::IsNearlyEqual(HeatVehicleWeaponComp->GetCurrentWeaponHeat(), 75.0f));
	TestFalse(TEXT("UI-P0-06 actual WeaponComp 냉각 후 과열 해제"), HeatVehicleWeaponComp->IsActiveWeaponOverheated());
	TestTrue(TEXT("UI-P0-06 actual WeaponComp 냉각 후 재사용 가능"), HeatVehicleWeaponComp->CanActiveWeaponAcceptHeatShot());
	HeatVehiclePawn->Destroy();

	// [v1.18.0] Ammo/Cooldown과 실제 Heat를 동시에 가진 일반 무기의 통합 Weapon ViewData입니다.
	FCFWeaponHUDData WeaponViewData;
	WeaponViewData.Availability = ECFUIViewAvailability::Known;
	WeaponViewData.WeaponId = FName(TEXT("HeatResourceWeapon"));
	WeaponViewData.AmmoAvailability = ECFUIViewAvailability::Known;
	WeaponViewData.LoadedAmmoCount = 3;
	WeaponViewData.MagazineCapacity = 8;
	WeaponViewData.ReserveAmmoCount = 6;
	WeaponViewData.CurrentOnboardAmmoCount = 9;
	WeaponViewData.CooldownAvailability = ECFUIViewAvailability::Known;
	WeaponViewData.CooldownDurationSeconds = 2.0f;
	WeaponViewData.RemainingCooldownSeconds = 0.5f;
	WeaponViewData.HeatAvailability = ECFUIViewAvailability::Known;
	WeaponViewData.CurrentHeat = 50.0f;
	WeaponViewData.MaximumHeat = 100.0f;
	WeaponViewData.HeatRatio = 0.5f;
	WeaponViewData.bWeaponOverheated = false;
	WeaponViewData.RebuildResourceChannelsFromCurrentFields();

	// [v1.18.0] 실제 ResourceChannels에서 semantic Heat 채널을 찾은 포인터입니다.
	const FCFWeaponResourceHUDData* HeatChannel = nullptr;
	for (const FCFWeaponResourceHUDData& ResourceChannel : WeaponViewData.ResourceChannels)
	{
		if (ResourceChannel.ChannelType == ECFWeaponResourceChannelType::Heat)
		{
			HeatChannel = &ResourceChannel;
			break;
		}
	}
	if (!TestNotNull(TEXT("UI-P0-06 actual Heat Resource Channel 생성"), HeatChannel))
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 Heat DisplayMode Percent"), HeatChannel->DisplayMode, ECFWeaponResourceDisplayMode::Percent);
	TestTrue(TEXT("UI-P0-06 Heat current 50"), FMath::IsNearlyEqual(HeatChannel->CurrentValue, 50.0f));
	TestTrue(TEXT("UI-P0-06 Heat max 100"), FMath::IsNearlyEqual(HeatChannel->MaximumValue, 100.0f));
	TestTrue(TEXT("UI-P0-06 Heat ratio 0.5"), FMath::IsNearlyEqual(HeatChannel->NormalizedValue, 0.5f));
	TestFalse(TEXT("UI-P0-06 normal Heat는 발사 비차단"), HeatChannel->bBlocksFire);

	// [v1.18.0] 실제 Compact Projection이 raw 채널 순서가 아니라 Ammo Primary + Heat Secondary + 단일 Cooldown FireState를 만드는 Presenter입니다.
	UCFHUDPresenter* Presenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-06 Heat Projection Presenter"), Presenter))
	{
		return false;
	}
	// [v1.18.0] 일반 Heat 상태에서 생성된 Compact Presentation Entry입니다.
	TArray<FCFWeaponResourcePresentationEntry> PresentationEntries;
	Presenter->BuildWeaponResourceEntries(WeaponViewData, PresentationEntries);
	TestEqual(TEXT("UI-P0-06 Heat 일반 Compact Entry 3개"), PresentationEntries.Num(), 3);
	if (PresentationEntries.Num() != 3)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 Heat 일반 Primary Ammo"), PresentationEntries[0].SourceChannelType, ECFWeaponResourceChannelType::Ammo);
	TestEqual(TEXT("UI-P0-06 Heat 일반 Secondary Heat"), PresentationEntries[1].SourceChannelType, ECFWeaponResourceChannelType::Heat);
	TestEqual(TEXT("UI-P0-06 Heat 일반 문구 50%"), PresentationEntries[1].DisplayText.ToString(), FString(TEXT("HEAT 50%")));
	TestTrue(TEXT("UI-P0-06 Heat 일반 Progress 0.5"), FMath::IsNearlyEqual(PresentationEntries[1].Progress01, 0.5f));
	TestEqual(TEXT("UI-P0-06 Heat 일반 FireState Cooldown"), PresentationEntries[2].DisplayText.ToString(), FString(TEXT("0.5 s")));

	WeaponViewData.CurrentHeat = 100.0f;
	WeaponViewData.HeatRatio = 1.0f;
	WeaponViewData.bWeaponOverheated = true;
	WeaponViewData.RebuildResourceChannelsFromCurrentFields();
	Presenter->BuildWeaponResourceEntries(WeaponViewData, PresentationEntries);
	TestEqual(TEXT("UI-P0-06 과열 FireState 유지 Entry 3개"), PresentationEntries.Num(), 3);
	if (PresentationEntries.Num() != 3)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 과열 Heat Secondary 유지"), PresentationEntries[1].DisplayText.ToString(), FString(TEXT("HEAT 100%")));
	TestEqual(TEXT("UI-P0-06 과열 FireState"), PresentationEntries[2].DisplayText.ToString(), FString(TEXT("OVERHEATED")));
	TestTrue(TEXT("UI-P0-06 과열 FireState Progress 1"), FMath::IsNearlyEqual(PresentationEntries[2].Progress01, 1.0f));

	WeaponViewData.ReloadState = ECFWeaponReloadState::Reloading;
	WeaponViewData.ReloadDurationSeconds = 4.0f;
	WeaponViewData.RemainingReloadTimeSeconds = 1.0f;
	WeaponViewData.bWeaponActionLocked = true;
	WeaponViewData.WeaponActionLockReason = ECFWeaponActionLockReason::Reloading;
	WeaponViewData.RebuildResourceChannelsFromCurrentFields();
	Presenter->BuildWeaponResourceEntries(WeaponViewData, PresentationEntries);
	TestEqual(TEXT("UI-P0-06 Reload가 과열보다 우선"), PresentationEntries.Last().DisplayText.ToString(), FString(TEXT("RELOAD 1.0 / 4.0 s")));

	WeaponViewData.ReloadState = ECFWeaponReloadState::Ready;
	WeaponViewData.ReloadDurationSeconds = 0.0f;
	WeaponViewData.RemainingReloadTimeSeconds = 0.0f;
	WeaponViewData.bWeaponActionLocked = false;
	WeaponViewData.WeaponActionLockReason = ECFWeaponActionLockReason::None;
	WeaponViewData.CurrentHeat = 50.0f;
	WeaponViewData.HeatRatio = 0.5f;
	WeaponViewData.bWeaponOverheated = false;
	WeaponViewData.LauncherAvailability = ECFUIViewAvailability::Known;
	WeaponViewData.bLauncherSequenceActive = true;
	WeaponViewData.LauncherPattern = ECFLauncherFirePattern::Salvo;
	WeaponViewData.LauncherTotalProjectileCount = 4;
	WeaponViewData.LauncherAcceptedProjectileCount = 2;
	WeaponViewData.LauncherRemainingProjectileCount = 2;
	WeaponViewData.LauncherSequenceRevision = 1;
	WeaponViewData.RebuildResourceChannelsFromCurrentFields();
	Presenter->BuildWeaponResourceEntries(WeaponViewData, PresentationEntries);
	TestEqual(TEXT("UI-P0-06 Launcher+Ammo+Heat Compact Entry 3개"), PresentationEntries.Num(), 3);
	if (PresentationEntries.Num() != 3)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 Launcher가 Primary 유지"), PresentationEntries[0].SourceChannelType, ECFWeaponResourceChannelType::LauncherSequence);
	TestEqual(TEXT("UI-P0-06 Launcher 중 Ammo Secondary A"), PresentationEntries[1].SourceChannelType, ECFWeaponResourceChannelType::Ammo);
	TestEqual(TEXT("UI-P0-06 Launcher 중 Heat Secondary B"), PresentationEntries[2].SourceChannelType, ECFWeaponResourceChannelType::Heat);
	TestTrue(TEXT("UI-P0-06 Launcher 중 FireState 없음"), PresentationEntries[0].Role == ECFWeaponResourcePresentationRole::Primary
		&& PresentationEntries[1].Role == ECFWeaponResourcePresentationRole::Secondary
		&& PresentationEntries[2].Role == ECFWeaponResourcePresentationRole::Secondary);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006WeaponChargeResourceTest,
	"CarFight.UI.UI_P0_06.WeaponChargeRuntimeResourceContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.21.0] explicit WeaponCharge의 순수 상태·actual WeaponComp 승인 한 발 소비·회복과 HUD Resource/Compact Projection을 검증합니다.
bool FCFHUDP006WeaponChargeResourceTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.21.0] 무기 자체 내부 Charge의 순수 상태 전이를 검증할 Runtime입니다.
	FCFWeaponChargeRuntime ChargeRuntime;
	ChargeRuntime.Configure(100.0f, 40.0f, 30.0f, 10.0f);
	TestTrue(TEXT("UI-P0-06 Charge Runtime explicit config 활성"), ChargeRuntime.IsEnabled());
	TestTrue(TEXT("UI-P0-06 Charge 초기값 40"), FMath::IsNearlyEqual(ChargeRuntime.GetCurrentCharge(), 40.0f));
	TestTrue(TEXT("UI-P0-06 Charge 40에서 한 발 가능"), ChargeRuntime.CanAcceptShot());
	ChargeRuntime.RecordAcceptedShot();
	TestTrue(TEXT("UI-P0-06 Charge 한 발 소비 후 10"), FMath::IsNearlyEqual(ChargeRuntime.GetCurrentCharge(), 10.0f));
	TestFalse(TEXT("UI-P0-06 Charge 10은 한 발 소비량 30보다 부족"), ChargeRuntime.CanAcceptShot());
	ChargeRuntime.AdvanceRecovery(1.5f);
	TestTrue(TEXT("UI-P0-06 Charge 1.5초 회복 25"), FMath::IsNearlyEqual(ChargeRuntime.GetCurrentCharge(), 25.0f));
	TestFalse(TEXT("UI-P0-06 Charge 25는 아직 발사 불가"), ChargeRuntime.CanAcceptShot());
	ChargeRuntime.AdvanceRecovery(0.5f);
	TestTrue(TEXT("UI-P0-06 Charge 2초 회복 30"), FMath::IsNearlyEqual(ChargeRuntime.GetCurrentCharge(), 30.0f));
	TestTrue(TEXT("UI-P0-06 Charge 30에서 발사 재허용"), ChargeRuntime.CanAcceptShot());
	ChargeRuntime.RecordAcceptedShot();
	TestTrue(TEXT("UI-P0-06 Charge 두 번째 소비 후 0"), FMath::IsNearlyEqual(ChargeRuntime.GetCurrentCharge(), 0.0f));
	ChargeRuntime.AdvanceRecovery(10.0f);
	TestTrue(TEXT("UI-P0-06 Charge 회복은 Max 100 clamp"), FMath::IsNearlyEqual(ChargeRuntime.GetCurrentCharge(), 100.0f));

	// [v1.21.0] 기존 all-zero/incomplete Asset 호환처럼 Recovery 0이면 Charge Runtime이 비활성이고 발사를 막지 않습니다.
	FCFWeaponChargeRuntime DisabledChargeRuntime;
	DisabledChargeRuntime.Configure(100.0f, 40.0f, 30.0f, 0.0f);
	TestFalse(TEXT("UI-P0-06 Charge Recovery 0은 Runtime 비활성"), DisabledChargeRuntime.IsEnabled());
	TestTrue(TEXT("UI-P0-06 Charge Runtime 비활성은 발사 차단 안 함"), DisabledChargeRuntime.CanAcceptShot());

	// [v1.21.0] 실제 ACFVehiclePawn accepted-fire 경로와 UCFVehicleWeaponComp Charge owner를 함께 검증할 transient Automation World입니다.
	UWorld* ChargeIntegrationWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-06 Charge Integration World"), ChargeIntegrationWorld))
	{
		return false;
	}

	// [v1.21.0] 실제 WeaponComp와 Fire 실행 경로를 기본 서브오브젝트로 소유할 transient 차량입니다.
	ACFVehiclePawn* ChargeVehiclePawn = ChargeIntegrationWorld->SpawnActor<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("UI-P0-06 Charge Integration Vehicle Pawn"), ChargeVehiclePawn))
	{
		return false;
	}

	// [v1.21.0] 실제 per-weapon Charge 상태를 소유할 차량 무기 컴포넌트입니다.
	UCFVehicleWeaponComp* ChargeVehicleWeaponComp = ChargeVehiclePawn->GetVehicleWeaponComp();
	if (!TestNotNull(TEXT("UI-P0-06 Charge Integration WeaponComp"), ChargeVehicleWeaponComp))
	{
		ChargeVehiclePawn->Destroy();
		return false;
	}

	// [v1.21.0] 테스트 WeaponData를 활성 MountProfile의 single preset source로 연결할 transient Preset입니다.
	UCFEquipmentPresetData* ChargeEquipmentPresetData = NewObject<UCFEquipmentPresetData>(GetTransientPackage());
	ChargeEquipmentPresetData->EquipmentId = TEXT("UI_P006_ChargePreset");
	ChargeEquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
	ChargeEquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Large;

	// [v1.21.0] 저장 Asset을 건드리지 않고 explicit Charge 100/60/30/10을 실제 WeaponComp에 공급할 transient WeaponData입니다.
	UCFWeaponData* ChargeWeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	ChargeWeaponData->WeaponId = TEXT("UI_P006_ChargeWeapon");
	ChargeWeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
	ChargeWeaponData->CompatibleMountTypes.Reset();
	ChargeWeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
	ChargeWeaponData->FireMode = ECFWeaponFireMode::HitScan;
	ChargeWeaponData->FireRatePerMinute = 0.0f;
	ChargeWeaponData->bUseInfiniteAmmoForDebug = true;
	ChargeWeaponData->MaximumWeaponCharge = 100.0f;
	ChargeWeaponData->InitialWeaponCharge = 60.0f;
	ChargeWeaponData->WeaponChargePerShot = 30.0f;
	ChargeWeaponData->WeaponChargeRecoveryPerSecond = 10.0f;
	ChargeEquipmentPresetData->DefaultWeaponData = ChargeWeaponData;

	// [v1.21.0] 활성 MountProfile이 참조할 최소 위치 슬롯과 Charge WeaponData source를 제공할 transient VehicleData입니다.
	UCFVehicleData* ChargeVehicleData = NewObject<UCFVehicleData>(GetTransientPackage());
	// [v1.21.0] 테스트 MountProfile이 사용할 최소 하드포인트 슬롯입니다.
	FCFVehicleHardpointSlot ChargeHardpointSlot;
	ChargeHardpointSlot.LocationSlotId = TEXT("Top_Charge_01");
	ChargeHardpointSlot.LocationCategory = TEXT("Top");
	ChargeHardpointSlot.LocalLocation = FVector::ZeroVector;
	ChargeVehicleData->HardpointSlots.Add(ChargeHardpointSlot);

	// [v1.21.0] WeaponComp 기본 활성 ID와 일치해 transient Charge WeaponData를 실제 활성 무기로 만드는 MountProfile입니다.
	FCFVehicleMountProfile ChargeMountProfile;
	ChargeMountProfile.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
	ChargeMountProfile.LocationSlotRef = ChargeHardpointSlot.LocationSlotId;
	ChargeMountProfile.MountType = ECFVehicleMountType::Turret;
	ChargeMountProfile.SizeLimit = ECFVehicleWeaponSize::Large;
	ChargeMountProfile.DefaultEquipmentPresetData = ChargeEquipmentPresetData;
	ChargeVehicleData->MountProfiles.Add(ChargeMountProfile);

	TestTrue(TEXT("UI-P0-06 actual WeaponComp Charge Runtime 초기화"), ChargeVehicleWeaponComp->InitializeWeaponRuntime(ChargeVehiclePawn, ChargeVehicleData));
	TestTrue(TEXT("UI-P0-06 actual WeaponComp Charge Runtime 활성"), ChargeVehicleWeaponComp->IsActiveWeaponChargeRuntimeEnabled());
	TestTrue(TEXT("UI-P0-06 actual WeaponComp Charge 초기값 60"), FMath::IsNearlyEqual(ChargeVehicleWeaponComp->GetCurrentWeaponCharge(), 60.0f));

		for (int32 AcceptedShotIndex = 0; AcceptedShotIndex < 2; ++AcceptedShotIndex)
	{
		TestTrue(TEXT("UI-P0-06 actual WeaponComp 승인 전 Charge 충분"), ChargeVehicleWeaponComp->CanActiveWeaponAcceptChargeShot());
		ChargeVehicleWeaponComp->RecordAcceptedWeaponShotCharge();
		TestTrue(
			TEXT("UI-P0-06 actual WeaponComp 승인 한 발 Charge 소비"),
			FMath::IsNearlyEqual(ChargeVehicleWeaponComp->GetCurrentWeaponCharge(), 60.0f - 30.0f * static_cast<float>(AcceptedShotIndex + 1)));
	}


	TestFalse(TEXT("UI-P0-06 actual WeaponComp Charge 0 발사 불가"), ChargeVehicleWeaponComp->CanActiveWeaponAcceptChargeShot());
	ChargeVehicleWeaponComp->TickComponent(3.0f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("UI-P0-06 actual WeaponComp 3초 Charge 회복 30"), FMath::IsNearlyEqual(ChargeVehicleWeaponComp->GetCurrentWeaponCharge(), 30.0f));
	TestTrue(TEXT("UI-P0-06 actual WeaponComp Charge 회복 후 재사용 가능"), ChargeVehicleWeaponComp->CanActiveWeaponAcceptChargeShot());
	ChargeVehiclePawn->Destroy();

	// [v1.21.0] Charge 25/100 부족 상태와 Cooldown을 동시에 가진 synthetic Weapon ViewData입니다.
	FCFWeaponHUDData ChargeWeaponViewData;
	ChargeWeaponViewData.Availability = ECFUIViewAvailability::Known;
	ChargeWeaponViewData.WeaponId = FName(TEXT("ChargeResourceWeapon"));
	ChargeWeaponViewData.WeaponChargeAvailability = ECFUIViewAvailability::Known;
	ChargeWeaponViewData.CurrentWeaponCharge = 25.0f;
	ChargeWeaponViewData.MaximumWeaponCharge = 100.0f;
	ChargeWeaponViewData.WeaponChargeRatio = 0.25f;
	ChargeWeaponViewData.bWeaponChargeInsufficient = true;
	ChargeWeaponViewData.CooldownAvailability = ECFUIViewAvailability::Known;
	ChargeWeaponViewData.CooldownDurationSeconds = 2.0f;
	ChargeWeaponViewData.RemainingCooldownSeconds = 0.5f;
	ChargeWeaponViewData.RebuildResourceChannelsFromCurrentFields();

	// [v1.21.0] 실제 ResourceChannels에서 semantic WeaponCharge 채널을 찾은 포인터입니다.
	const FCFWeaponResourceHUDData* ChargeChannel = nullptr;
	// [v1.21.0] VehicleBattery가 실제 Runtime 없이 실수로 생성되지 않았는지 확인하는 상태입니다.
	bool bVehicleBatteryChannelFound = false;
	for (const FCFWeaponResourceHUDData& ResourceChannel : ChargeWeaponViewData.ResourceChannels)
	{
		if (ResourceChannel.ChannelType == ECFWeaponResourceChannelType::WeaponCharge)
		{
			ChargeChannel = &ResourceChannel;
		}
		else if (ResourceChannel.ChannelType == ECFWeaponResourceChannelType::VehicleBattery)
		{
			bVehicleBatteryChannelFound = true;
		}
	}
	if (!TestNotNull(TEXT("UI-P0-06 actual WeaponCharge Resource Channel 생성"), ChargeChannel))
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 Charge DisplayMode Percent"), ChargeChannel->DisplayMode, ECFWeaponResourceDisplayMode::Percent);
	TestTrue(TEXT("UI-P0-06 Charge current 25"), FMath::IsNearlyEqual(ChargeChannel->CurrentValue, 25.0f));
	TestTrue(TEXT("UI-P0-06 Charge max 100"), FMath::IsNearlyEqual(ChargeChannel->MaximumValue, 100.0f));
	TestTrue(TEXT("UI-P0-06 Charge ratio 0.25"), FMath::IsNearlyEqual(ChargeChannel->NormalizedValue, 0.25f));
	TestTrue(TEXT("UI-P0-06 Charge 부족은 bBlocksFire"), ChargeChannel->bBlocksFire);
	TestFalse(TEXT("UI-P0-06 VehicleBattery 가짜 Resource Channel 없음"), bVehicleBatteryChannelFound);

	// [v1.21.0] 실제 Charge Compact Projection과 NO CHARGE 상태를 검증할 Presenter입니다.
	UCFHUDPresenter* ChargePresenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-06 Charge Projection Presenter"), ChargePresenter))
	{
		return false;
	}

	// [v1.21.0] Ammo/Launcher가 없는 Charge 기반 무기의 Compact Presentation Entry입니다.
	TArray<FCFWeaponResourcePresentationEntry> ChargePresentationEntries;
	ChargePresenter->BuildWeaponResourceEntries(ChargeWeaponViewData, ChargePresentationEntries);
	TestEqual(TEXT("UI-P0-06 Charge-only Compact Entry 2개"), ChargePresentationEntries.Num(), 2);
	if (ChargePresentationEntries.Num() != 2)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 Charge-only Primary"), ChargePresentationEntries[0].SourceChannelType, ECFWeaponResourceChannelType::WeaponCharge);
	TestEqual(TEXT("UI-P0-06 Charge-only Primary 문구"), ChargePresentationEntries[0].DisplayText.ToString(), FString(TEXT("CHARGE 25%")));
	TestEqual(TEXT("UI-P0-06 Charge 부족 FireState"), ChargePresentationEntries[1].DisplayText.ToString(), FString(TEXT("NO CHARGE")));
	TestTrue(TEXT("UI-P0-06 Charge 부족 Progress 0.25"), FMath::IsNearlyEqual(ChargePresentationEntries[1].Progress01, 0.25f));

	// [v1.21.0] Heat와 Charge가 함께 있을 때 Charge Primary + Heat Secondary + NO CHARGE를 확인합니다.
	ChargeWeaponViewData.HeatAvailability = ECFUIViewAvailability::Known;
	ChargeWeaponViewData.CurrentHeat = 50.0f;
	ChargeWeaponViewData.MaximumHeat = 100.0f;
	ChargeWeaponViewData.HeatRatio = 0.5f;
	ChargeWeaponViewData.bWeaponOverheated = true;
	ChargeWeaponViewData.RebuildResourceChannelsFromCurrentFields();
	ChargePresenter->BuildWeaponResourceEntries(ChargeWeaponViewData, ChargePresentationEntries);
	TestEqual(TEXT("UI-P0-06 Charge+Heat Compact Entry 3개"), ChargePresentationEntries.Num(), 3);
	if (ChargePresentationEntries.Num() != 3)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 Charge+Heat Primary Charge"), ChargePresentationEntries[0].SourceChannelType, ECFWeaponResourceChannelType::WeaponCharge);
	TestEqual(TEXT("UI-P0-06 Charge+Heat Secondary Heat"), ChargePresentationEntries[1].SourceChannelType, ECFWeaponResourceChannelType::Heat);
	TestEqual(TEXT("UI-P0-06 NoCharge가 Overheated보다 우선"), ChargePresentationEntries[2].DisplayText.ToString(), FString(TEXT("NO CHARGE")));

	// [v1.21.0] Charge 부족을 해제하면 같은 상태에서 Overheated가 다음 우선순위로 노출됩니다.
	ChargeWeaponViewData.CurrentWeaponCharge = 50.0f;
	ChargeWeaponViewData.WeaponChargeRatio = 0.5f;
	ChargeWeaponViewData.bWeaponChargeInsufficient = false;
	ChargeWeaponViewData.RebuildResourceChannelsFromCurrentFields();
	ChargePresenter->BuildWeaponResourceEntries(ChargeWeaponViewData, ChargePresentationEntries);
	TestEqual(TEXT("UI-P0-06 Charge 충분 시 Overheated fallback"), ChargePresentationEntries.Last().DisplayText.ToString(), FString(TEXT("OVERHEATED")));

	// [v1.21.0] Heat 차단을 해제하고 finite Ammo를 추가해 Ammo Primary + Charge/Heat Secondary 최대 2를 검증합니다.
	ChargeWeaponViewData.bWeaponOverheated = false;
	ChargeWeaponViewData.AmmoAvailability = ECFUIViewAvailability::Known;
	ChargeWeaponViewData.LoadedAmmoCount = 3;
	ChargeWeaponViewData.MagazineCapacity = 8;
	ChargeWeaponViewData.ReserveAmmoCount = 6;
	ChargeWeaponViewData.CurrentOnboardAmmoCount = 9;
	ChargeWeaponViewData.RebuildResourceChannelsFromCurrentFields();
	ChargePresenter->BuildWeaponResourceEntries(ChargeWeaponViewData, ChargePresentationEntries);
	TestEqual(TEXT("UI-P0-06 Ammo+Charge+Heat Compact Entry 4개"), ChargePresentationEntries.Num(), 4);
	if (ChargePresentationEntries.Num() != 4)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 Ammo Primary 유지"), ChargePresentationEntries[0].SourceChannelType, ECFWeaponResourceChannelType::Ammo);
	TestEqual(TEXT("UI-P0-06 Charge Secondary A"), ChargePresentationEntries[1].SourceChannelType, ECFWeaponResourceChannelType::WeaponCharge);
	TestEqual(TEXT("UI-P0-06 Heat Secondary B"), ChargePresentationEntries[2].SourceChannelType, ECFWeaponResourceChannelType::Heat);
	TestEqual(TEXT("UI-P0-06 Charge 충분 시 FireState Cooldown"), ChargePresentationEntries[3].DisplayText.ToString(), FString(TEXT("0.5 s")));

	// [v1.21.0] NoAmmo가 Charge 부족보다 우선하는 상태를 확인합니다.
	ChargeWeaponViewData.LoadedAmmoCount = 0;
	ChargeWeaponViewData.CurrentOnboardAmmoCount = 0;
	ChargeWeaponViewData.CurrentWeaponCharge = 0.0f;
	ChargeWeaponViewData.WeaponChargeRatio = 0.0f;
	ChargeWeaponViewData.bWeaponChargeInsufficient = true;
	ChargeWeaponViewData.RebuildResourceChannelsFromCurrentFields();
	ChargePresenter->BuildWeaponResourceEntries(ChargeWeaponViewData, ChargePresentationEntries);
	TestEqual(TEXT("UI-P0-06 NoAmmo가 NoCharge보다 우선"), ChargePresentationEntries.Last().DisplayText.ToString(), FString(TEXT("NO AMMO")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006WeaponSelectionRuntimeTest,
	"CarFight.UI.UI_P0_06.WeaponSelectionRuntimeContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.19.0] Applied Fitting 고정 순서가 실제 Weapon Selection Runtime과 HUD Player-facing 목록으로 이어지고 무기별 Cooldown/Heat가 섞이지 않는지 검증합니다.
bool FCFHUDP006WeaponSelectionRuntimeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	/**
	 * Fitting Snapshot의 실제 Weapon Runtime 입력만 캡처하고 Defense/Sensor는 이번 UI 계약 밖에서 무변경 성공 처리하는 테스트 Adapter입니다.
	 */
	class FCFWeaponSelectionCaptureAdapter final : public ICFFittingRuntimeApplyAdapter
	{
	public:
		// [v1.19.0] Fitting Commit이 Weapon Runtime 입력을 실제로 호출한 횟수입니다.
		int32 WeaponApplyCallCount = 0;

		// [v1.19.0] Applied Fitting이 생성한 마지막 실제 Weapon Runtime 입력입니다.
		FCFFittingWeaponRuntimeInput LastWeaponInput;

		// [v1.19.0] Fitting의 Weapon Runtime 입력을 캡처하고 실제 선택 목록 또는 legacy/single mount가 있으면 준비 완료로 판정합니다.
		virtual bool ApplyWeaponRuntime(const FCFFittingWeaponRuntimeInput& WeaponInput, bool& bOutWeaponRuntimeReady) override
		{
			++WeaponApplyCallCount;
			LastWeaponInput = WeaponInput;
			bOutWeaponRuntimeReady = WeaponInput.UsesLegacyVehicleConfiguration()
				|| !WeaponInput.SelectableWeapons.IsEmpty()
				|| WeaponInput.bHasResolvedMount;
			return true;
		}

		// [v1.19.0] Weapon Selection source 검증과 무관한 Defense Runtime 적용을 무변경 성공 처리합니다.
		virtual bool ApplyDefenseRuntime(const FCFFittingDefenseRuntimeInput& DefenseInput, bool& bOutDefenseRuntimeReady) override
		{
			(void)DefenseInput;
			bOutDefenseRuntimeReady = false;
			return true;
		}
	};

	// [v1.19.0] 실제 Pawn/WeaponComp 전환과 HUD Provider를 함께 검증할 transient Automation World입니다.
	UWorld* SelectionWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-06 Weapon Selection World"), SelectionWorld))
	{
		return false;
	}

	// [v1.19.0] 실제 WeaponComp·Launcher·Ammo·Provider source를 보유할 transient 차량 Pawn입니다.
	ACFVehiclePawn* SelectionVehiclePawn = SelectionWorld->SpawnActor<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("UI-P0-06 Weapon Selection Vehicle Pawn"), SelectionVehiclePawn))
	{
		return false;
	}

	// [v1.19.0] 실제 선택 Runtime과 per-weapon Cooldown/Heat를 소유할 WeaponComp입니다.
	UCFVehicleWeaponComp* SelectionWeaponComp = SelectionVehiclePawn->GetVehicleWeaponComp();
	if (!TestNotNull(TEXT("UI-P0-06 Weapon Selection WeaponComp"), SelectionWeaponComp))
	{
		SelectionVehiclePawn->Destroy();
		return false;
	}

	// [v1.19.0] 실제 Fitting Snapshot→Weapon Runtime 입력 변환을 수행할 transient FittingComp입니다.
	UCFVehicleFittingComp* SelectionFittingComp = NewObject<UCFVehicleFittingComp>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-06 Weapon Selection FittingComp"), SelectionFittingComp))
	{
		SelectionVehiclePawn->Destroy();
		return false;
	}

	// [v1.19.0] 두 실제 weapon-bearing mount를 정의할 transient VehicleData입니다.
	UCFVehicleData* SelectionVehicleData = NewObject<UCFVehicleData>(GetTransientPackage());
	SelectionVehicleData->BaseVehicleMassKg = 1000.0f;
	SelectionVehicleData->MaximumGrossMassKg = 2500.0f;

	// [v1.19.0] 첫 번째 무기가 사용할 실제 하드포인트 위치 슬롯입니다.
	FCFVehicleHardpointSlot PrimaryHardpointSlot;
	PrimaryHardpointSlot.LocationSlotId = TEXT("Selection_Top_01");
	PrimaryHardpointSlot.LocationCategory = TEXT("Top");
	SelectionVehicleData->HardpointSlots.Add(PrimaryHardpointSlot);

	// [v1.19.0] 두 번째 무기가 사용할 실제 하드포인트 위치 슬롯입니다.
	FCFVehicleHardpointSlot SecondaryHardpointSlot;
	SecondaryHardpointSlot.LocationSlotId = TEXT("Selection_Top_02");
	SecondaryHardpointSlot.LocationCategory = TEXT("Top");
	SelectionVehicleData->HardpointSlots.Add(SecondaryHardpointSlot);

	// [v1.19.0] 첫 번째 실제 무기의 Player-facing Preset source입니다.
	UCFEquipmentPresetData* PrimaryEquipmentPreset = NewObject<UCFEquipmentPresetData>(GetTransientPackage());
	PrimaryEquipmentPreset->EquipmentId = TEXT("Selection_Primary_Preset");
	PrimaryEquipmentPreset->DisplayName = FText::FromString(TEXT("PRIMARY CANNON"));
	PrimaryEquipmentPreset->RequiredMountType = ECFVehicleMountType::Turret;
	PrimaryEquipmentPreset->RequiredWeaponSize = ECFVehicleWeaponSize::Large;

	// [v1.19.0] 첫 번째 Preset이 실제 단일 Turret Visual source로도 사용할 transient TurretMountData입니다.
	UCFTurretMountData* PrimaryTurretMountData = NewObject<UCFTurretMountData>(GetTransientPackage());
	PrimaryTurretMountData->TurretMountId = TEXT("Selection_Primary_MountData");
	PrimaryEquipmentPreset->DefaultTurretMountData = PrimaryTurretMountData;

	// [v1.19.0] 60 RPM, Heat 25/100/10을 가진 첫 번째 실제 WeaponData입니다.
	UCFWeaponData* PrimaryWeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	PrimaryWeaponData->WeaponId = TEXT("Selection_Primary_Weapon");
	PrimaryWeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
	PrimaryWeaponData->CompatibleMountTypes.Reset();
	PrimaryWeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
	PrimaryWeaponData->FireMode = ECFWeaponFireMode::HitScan;
	PrimaryWeaponData->FireRatePerMinute = 60.0f;
	PrimaryWeaponData->bUseInfiniteAmmoForDebug = true;
	PrimaryWeaponData->HeatPerShot = 25.0f;
	PrimaryWeaponData->MaxHeat = 100.0f;
	PrimaryWeaponData->HeatDissipationPerSecond = 10.0f;
	PrimaryEquipmentPreset->DefaultWeaponData = PrimaryWeaponData;

	// [v1.19.0] 두 번째 실제 무기의 Player-facing Preset source입니다.
	UCFEquipmentPresetData* SecondaryEquipmentPreset = NewObject<UCFEquipmentPresetData>(GetTransientPackage());
	SecondaryEquipmentPreset->EquipmentId = TEXT("Selection_Secondary_Preset");
	SecondaryEquipmentPreset->DisplayName = FText::FromString(TEXT("SECONDARY CANNON"));
	SecondaryEquipmentPreset->RequiredMountType = ECFVehicleMountType::Turret;
	SecondaryEquipmentPreset->RequiredWeaponSize = ECFVehicleWeaponSize::Large;

	// [v1.19.0] 두 번째 Preset이 실제 단일 Turret Visual source로도 사용할 transient TurretMountData입니다.
	UCFTurretMountData* SecondaryTurretMountData = NewObject<UCFTurretMountData>(GetTransientPackage());
	SecondaryTurretMountData->TurretMountId = TEXT("Selection_Secondary_MountData");
	SecondaryEquipmentPreset->DefaultTurretMountData = SecondaryTurretMountData;

	// [v1.19.0] 120 RPM, Heat 25/100/10을 가진 두 번째 실제 WeaponData입니다.
	UCFWeaponData* SecondaryWeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	SecondaryWeaponData->WeaponId = TEXT("Selection_Secondary_Weapon");
	SecondaryWeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
	SecondaryWeaponData->CompatibleMountTypes.Reset();
	SecondaryWeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
	SecondaryWeaponData->FireMode = ECFWeaponFireMode::HitScan;
	SecondaryWeaponData->FireRatePerMinute = 120.0f;
	SecondaryWeaponData->bUseInfiniteAmmoForDebug = true;
	SecondaryWeaponData->HeatPerShot = 25.0f;
	SecondaryWeaponData->MaxHeat = 100.0f;
	SecondaryWeaponData->HeatDissipationPerSecond = 10.0f;
	SecondaryEquipmentPreset->DefaultWeaponData = SecondaryWeaponData;

	// [v1.19.0] 첫 번째 내부 mount identity와 실제 Preset을 VehicleData에 연결하는 MountProfile입니다.
	FCFVehicleMountProfile PrimaryMountProfile;
	PrimaryMountProfile.MountProfileId = TEXT("SelectionMount_Primary");
	PrimaryMountProfile.LocationSlotRef = PrimaryHardpointSlot.LocationSlotId;
	PrimaryMountProfile.MountType = ECFVehicleMountType::Turret;
	PrimaryMountProfile.SizeLimit = ECFVehicleWeaponSize::Large;
	PrimaryMountProfile.DefaultEquipmentPresetData = PrimaryEquipmentPreset;
	SelectionVehicleData->MountProfiles.Add(PrimaryMountProfile);

	// [v1.19.0] 두 번째 내부 mount identity와 실제 Preset을 VehicleData에 연결하는 MountProfile입니다.
	FCFVehicleMountProfile SecondaryMountProfile;
	SecondaryMountProfile.MountProfileId = TEXT("SelectionMount_Secondary");
	SecondaryMountProfile.LocationSlotRef = SecondaryHardpointSlot.LocationSlotId;
	SecondaryMountProfile.MountType = ECFVehicleMountType::Turret;
	SecondaryMountProfile.SizeLimit = ECFVehicleWeaponSize::Large;
	SecondaryMountProfile.DefaultEquipmentPresetData = SecondaryEquipmentPreset;
	SelectionVehicleData->MountProfiles.Add(SecondaryMountProfile);

	// [v1.19.0] Applied Fitting source를 재현하는 유효 transient Snapshot입니다.
	FCFVehicleFittingSnapshot SelectionSnapshot;
	SelectionSnapshot.FittingId = TEXT("Selection_Fitting");
	SelectionSnapshot.VehicleData = SelectionVehicleData;
	SelectionSnapshot.ValidationState = ECFFittingValidationState::Valid;
	SelectionSnapshot.BaseVehicleMassKg = 1000.0f;
	SelectionSnapshot.TotalVehicleMassKg = 1500.0f;
	SelectionSnapshot.MaximumGrossMassKg = 2500.0f;

	// [v1.19.0] Fitting 고정 표시 순서 0에 위치할 첫 실제 weapon-bearing mount입니다.
	FCFResolvedFittingMount PrimaryResolvedMount;
	PrimaryResolvedMount.MountProfileId = PrimaryMountProfile.MountProfileId;
	PrimaryResolvedMount.LocationSlotId = PrimaryHardpointSlot.LocationSlotId;
	PrimaryResolvedMount.MountType = ECFVehicleMountType::Turret;
	PrimaryResolvedMount.SizeLimit = ECFVehicleWeaponSize::Large;
	PrimaryResolvedMount.EquipmentPresetData = PrimaryEquipmentPreset;
	PrimaryResolvedMount.TurretMountData = PrimaryTurretMountData;
	PrimaryResolvedMount.WeaponData = PrimaryWeaponData;
	PrimaryResolvedMount.SelectionSource = ECFFittingSelectionSource::FittingOverride;
	SelectionSnapshot.ResolvedMounts.Add(PrimaryResolvedMount);

	// [v1.19.0] Fitting 고정 표시 순서 1에 위치할 두 번째 실제 weapon-bearing mount입니다.
	FCFResolvedFittingMount SecondaryResolvedMount;
	SecondaryResolvedMount.MountProfileId = SecondaryMountProfile.MountProfileId;
	SecondaryResolvedMount.LocationSlotId = SecondaryHardpointSlot.LocationSlotId;
	SecondaryResolvedMount.MountType = ECFVehicleMountType::Turret;
	SecondaryResolvedMount.SizeLimit = ECFVehicleWeaponSize::Large;
	SecondaryResolvedMount.EquipmentPresetData = SecondaryEquipmentPreset;
	SecondaryResolvedMount.TurretMountData = SecondaryTurretMountData;
	SecondaryResolvedMount.WeaponData = SecondaryWeaponData;
	SecondaryResolvedMount.SelectionSource = ECFFittingSelectionSource::FittingOverride;
	SelectionSnapshot.ResolvedMounts.Add(SecondaryResolvedMount);

	TestTrue(
		TEXT("UI-P0-06 Applied Fitting Snapshot Weapon Selection 준비"),
		SelectionFittingComp->PrepareSortieFittingSnapshot(SelectionSnapshot, PrimaryMountProfile.MountProfileId));

	// [v1.19.0] Fitting Commit이 생성한 실제 Weapon Runtime 입력을 수집할 Adapter입니다.
	FCFWeaponSelectionCaptureAdapter SelectionCaptureAdapter;
	TestTrue(TEXT("UI-P0-06 Applied Fitting Weapon Runtime 입력 Commit"), SelectionFittingComp->CommitPreparedSortieFitting(SelectionCaptureAdapter));
	TestEqual(TEXT("UI-P0-06 Fitting Weapon Runtime 적용 1회"), SelectionCaptureAdapter.WeaponApplyCallCount, 1);
	TestEqual(TEXT("UI-P0-06 Fitting selectable weapons 2개"), SelectionCaptureAdapter.LastWeaponInput.SelectableWeapons.Num(), 2);
	TestEqual(TEXT("UI-P0-06 Fitting selected index 0"), SelectionCaptureAdapter.LastWeaponInput.SelectedWeaponIndex, 0);
	if (SelectionCaptureAdapter.LastWeaponInput.SelectableWeapons.Num() != 2)
	{
		SelectionVehiclePawn->Destroy();
		return false;
	}
	TestEqual(
		TEXT("UI-P0-06 Fitting 고정 순서 Primary 내부 identity"),
		SelectionCaptureAdapter.LastWeaponInput.SelectableWeapons[0].InternalMountProfileId,
		PrimaryMountProfile.MountProfileId);
	TestEqual(
		TEXT("UI-P0-06 Fitting 고정 순서 Secondary 내부 identity"),
		SelectionCaptureAdapter.LastWeaponInput.SelectableWeapons[1].InternalMountProfileId,
		SecondaryMountProfile.MountProfileId);

	TestTrue(
		TEXT("UI-P0-06 실제 Weapon Selection Runtime 초기화"),
		SelectionWeaponComp->InitializeWeaponSelectionRuntime(
			SelectionVehiclePawn,
			SelectionVehicleData,
			SelectionCaptureAdapter.LastWeaponInput.SelectableWeapons,
			SelectionCaptureAdapter.LastWeaponInput.SelectedWeaponIndex));
	TestTrue(TEXT("UI-P0-06 Weapon Selection Runtime 활성"), SelectionWeaponComp->HasWeaponSelectionRuntime());
	TestEqual(TEXT("UI-P0-06 Weapon Selection 실제 무기 수"), SelectionWeaponComp->GetSelectableWeaponCount(), 2);
	TestEqual(TEXT("UI-P0-06 초기 Selected index"), SelectionWeaponComp->GetSelectedWeaponIndex(), 0);
	TestEqual(TEXT("UI-P0-06 초기 Active WeaponData"), SelectionWeaponComp->GetActiveWeaponData(), PrimaryWeaponData);
	TestEqual(TEXT("UI-P0-06 Primary Player-facing DisplayName"), SelectionWeaponComp->GetSelectableWeaponDisplayName(0).ToString(), FString(TEXT("PRIMARY CANNON")));
	TestEqual(TEXT("UI-P0-06 Secondary Player-facing DisplayName"), SelectionWeaponComp->GetSelectableWeaponDisplayName(1).ToString(), FString(TEXT("SECONDARY CANNON")));

	SelectionWeaponComp->RecordAcceptedFire(10.0f);
	SelectionWeaponComp->RecordAcceptedWeaponShotHeat();
	SelectionWeaponComp->RecordAcceptedWeaponShotHeat();
	TestTrue(TEXT("UI-P0-06 Primary Heat 50"), FMath::IsNearlyEqual(SelectionWeaponComp->GetCurrentWeaponHeat(), 50.0f));

	TestTrue(TEXT("UI-P0-06 Secondary 선택 요청 성공"), SelectionVehiclePawn->RequestSelectWeaponIndex(1));
	TestEqual(TEXT("UI-P0-06 Secondary Selected index"), SelectionWeaponComp->GetSelectedWeaponIndex(), 1);
	TestEqual(TEXT("UI-P0-06 Secondary Active WeaponData"), SelectionWeaponComp->GetActiveWeaponData(), SecondaryWeaponData);
	TestTrue(TEXT("UI-P0-06 Secondary 초기 last fire 없음"), SelectionWeaponComp->GetLastAcceptedFireTimeSeconds() < 0.0f);
	TestTrue(TEXT("UI-P0-06 Secondary 초기 Heat 0"), FMath::IsNearlyZero(SelectionWeaponComp->GetCurrentWeaponHeat()));

	SelectionWeaponComp->RecordAcceptedFire(20.0f);
	SelectionWeaponComp->RecordAcceptedWeaponShotHeat();
	TestTrue(TEXT("UI-P0-06 Secondary Heat 25"), FMath::IsNearlyEqual(SelectionWeaponComp->GetCurrentWeaponHeat(), 25.0f));

	SelectionWeaponComp->TickComponent(1.0f, LEVELTICK_All, nullptr);
	TestTrue(TEXT("UI-P0-06 Primary 재선택 성공"), SelectionVehiclePawn->RequestSelectWeaponIndex(0));
	TestTrue(TEXT("UI-P0-06 비선택 Primary Heat도 50→40 냉각"), FMath::IsNearlyEqual(SelectionWeaponComp->GetCurrentWeaponHeat(), 40.0f));
	TestTrue(TEXT("UI-P0-06 Primary last fire 10 보존"), FMath::IsNearlyEqual(SelectionWeaponComp->GetLastAcceptedFireTimeSeconds(), 10.0f));
	TestTrue(TEXT("UI-P0-06 Primary 60RPM 독립 Cooldown 0.5"), FMath::IsNearlyEqual(SelectionWeaponComp->GetRemainingCooldownSeconds(10.5f), 0.5f));

	TestTrue(TEXT("UI-P0-06 Secondary 재선택 성공"), SelectionVehiclePawn->RequestSelectWeaponIndex(1));
	TestTrue(TEXT("UI-P0-06 Secondary Heat 25→15 냉각"), FMath::IsNearlyEqual(SelectionWeaponComp->GetCurrentWeaponHeat(), 15.0f));
	TestTrue(TEXT("UI-P0-06 Secondary last fire 20 보존"), FMath::IsNearlyEqual(SelectionWeaponComp->GetLastAcceptedFireTimeSeconds(), 20.0f));
	TestTrue(TEXT("UI-P0-06 Secondary 120RPM 독립 Cooldown 0.25"), FMath::IsNearlyEqual(SelectionWeaponComp->GetRemainingCooldownSeconds(20.25f), 0.25f));
	TestFalse(TEXT("UI-P0-06 범위 밖 무기 선택 거부"), SelectionVehiclePawn->RequestSelectWeaponIndex(2));
	TestEqual(TEXT("UI-P0-06 잘못된 선택 뒤 Secondary 유지"), SelectionWeaponComp->GetSelectedWeaponIndex(), 1);

	// [v1.19.0] 실제 WeaponComp Player-facing selection source가 HUD ViewData로 내부 ID 없이 전달되는지 검증할 Provider입니다.
	UCFHUDDataProvider* SelectionHUDProvider = NewObject<UCFHUDDataProvider>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-06 Weapon Selection HUD Provider"), SelectionHUDProvider))
	{
		SelectionVehiclePawn->Destroy();
		return false;
	}
	SelectionHUDProvider->RebindCurrentPawn(SelectionVehiclePawn);
	SelectionHUDProvider->RefreshViewData();

	// [v1.19.0] Secondary가 선택된 시점의 실제 Player-facing Weapon HUD ViewData입니다.
	FCFWeaponHUDData SelectionWeaponViewData = SelectionHUDProvider->GetCurrentViewData().Weapon;
	TestEqual(TEXT("UI-P0-06 HUD Weapon Selection Known"), SelectionWeaponViewData.WeaponSelectionAvailability, ECFUIViewAvailability::Known);
	TestEqual(TEXT("UI-P0-06 HUD 선택 가능 무기 2개"), SelectionWeaponViewData.SelectableWeapons.Num(), 2);
	TestEqual(TEXT("UI-P0-06 HUD Selected index 1"), SelectionWeaponViewData.SelectedWeaponIndex, 1);
	if (SelectionWeaponViewData.SelectableWeapons.Num() != 2)
	{
		SelectionHUDProvider->ShutdownProvider();
		SelectionVehiclePawn->Destroy();
		return false;
	}
	TestEqual(TEXT("UI-P0-06 HUD Primary DisplayName"), SelectionWeaponViewData.SelectableWeapons[0].DisplayName.ToString(), FString(TEXT("PRIMARY CANNON")));
	TestEqual(TEXT("UI-P0-06 HUD Secondary DisplayName"), SelectionWeaponViewData.SelectableWeapons[1].DisplayName.ToString(), FString(TEXT("SECONDARY CANNON")));
	TestFalse(TEXT("UI-P0-06 HUD Primary 비선택"), SelectionWeaponViewData.SelectableWeapons[0].bSelected);
	TestTrue(TEXT("UI-P0-06 HUD Secondary 선택"), SelectionWeaponViewData.SelectableWeapons[1].bSelected);

	TestTrue(TEXT("UI-P0-06 Provider 관측 중 Primary 선택"), SelectionVehiclePawn->RequestSelectWeaponIndex(0));
	SelectionHUDProvider->RefreshViewData();
	SelectionWeaponViewData = SelectionHUDProvider->GetCurrentViewData().Weapon;
	TestEqual(TEXT("UI-P0-06 HUD Selected index 0 갱신"), SelectionWeaponViewData.SelectedWeaponIndex, 0);
	TestTrue(TEXT("UI-P0-06 HUD Primary 선택 갱신"), SelectionWeaponViewData.SelectableWeapons[0].bSelected);
	TestFalse(TEXT("UI-P0-06 HUD Secondary 비선택 갱신"), SelectionWeaponViewData.SelectableWeapons[1].bSelected);

		SelectionHUDProvider->ShutdownProvider();
	SelectionVehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006WeaponRailVisualTest,
	"CarFight.UI.UI_P0_06.WeaponRailVisualContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.20.0] 저장 Production WeaponPanel이 실제 selection ViewData의 비선택 무기만 이름 기반 Rail로 표시하고 fake icon/selected duplicate를 만들지 않는지 검증합니다.
bool FCFHUDP006WeaponRailVisualTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.20.0] 저장 Production HUD를 실제 인스턴스화해 Rail slot 적용을 확인할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-06 Weapon Rail Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.20.0] DefaultGame.ini와 동일한 저장 Production HUD Generated Class입니다.
	UClass* ProductionHUDClass = LoadClass<UCFStyledWidgetBase>(
		nullptr,
		TEXT("/Game/CarFight/UI/HUD/WBP_CFInGameHUD.WBP_CFInGameHUD_C"));
	if (!TestNotNull(TEXT("UI-P0-06 Weapon Rail Production HUD Class"), ProductionHUDClass))
	{
		return false;
	}

	// [v1.20.0] 저장 Production HUD Class로 만든 테스트 전용 Widget 인스턴스입니다.
	UCFStyledWidgetBase* ProductionHUDWidget = CreateWidget<UCFStyledWidgetBase>(TestWorld, ProductionHUDClass);
	if (!TestNotNull(TEXT("UI-P0-06 Weapon Rail Production HUD Widget"), ProductionHUDWidget))
	{
		return false;
	}

	// [v1.20.0] 저장 Production Root 아래 실제 WeaponPanel 자식 Widget입니다.
	UUserWidget* WeaponPanelWidget = Cast<UUserWidget>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("WBP_CFWeaponPanel"))));
	if (!TestNotNull(TEXT("UI-P0-06 Weapon Rail Production WeaponPanel"), WeaponPanelWidget))
	{
		return false;
	}

	// [v1.20.0] 실제 비선택 무기가 하나 이상일 때 Presenter가 열 Rail 전체 행입니다.
	UWidget* WeaponRail = WeaponPanelWidget->GetWidgetFromName(FName(TEXT("HorizontalBox_WeaponRail")));
	// [v1.20.0] 첫 번째 fixed Rail Text Tile Container입니다.
	UWidget* WeaponRailTile1 = WeaponPanelWidget->GetWidgetFromName(FName(TEXT("SizeBox_WeaponRail1")));
	// [v1.20.0] 두 번째 fixed Rail Text Tile Container입니다.
	UWidget* WeaponRailTile2 = WeaponPanelWidget->GetWidgetFromName(FName(TEXT("SizeBox_WeaponRail2")));
	// [v1.20.0] 세 번째 fixed Rail Text Tile 또는 overflow Container입니다.
	UWidget* WeaponRailTile3 = WeaponPanelWidget->GetWidgetFromName(FName(TEXT("SizeBox_WeaponRail3")));
	// [v1.20.0] 첫 Rail Tile의 Presenter 최종 문자열입니다.
	UTextBlock* WeaponRailText1 = Cast<UTextBlock>(WeaponPanelWidget->GetWidgetFromName(FName(TEXT("Text_WeaponRail1"))));
	// [v1.20.0] 두 번째 Rail Tile의 Presenter 최종 문자열입니다.
	UTextBlock* WeaponRailText2 = Cast<UTextBlock>(WeaponPanelWidget->GetWidgetFromName(FName(TEXT("Text_WeaponRail2"))));
	// [v1.20.0] 세 번째 Rail Tile 또는 +N overflow의 Presenter 최종 문자열입니다.
	UTextBlock* WeaponRailText3 = Cast<UTextBlock>(WeaponPanelWidget->GetWidgetFromName(FName(TEXT("Text_WeaponRail3"))));
	if (!TestNotNull(TEXT("UI-P0-06 Weapon Rail Row"), WeaponRail)
		|| !TestNotNull(TEXT("UI-P0-06 Weapon Rail Tile1"), WeaponRailTile1)
		|| !TestNotNull(TEXT("UI-P0-06 Weapon Rail Tile2"), WeaponRailTile2)
		|| !TestNotNull(TEXT("UI-P0-06 Weapon Rail Tile3"), WeaponRailTile3)
		|| !TestNotNull(TEXT("UI-P0-06 Weapon Rail Text1"), WeaponRailText1)
		|| !TestNotNull(TEXT("UI-P0-06 Weapon Rail Text2"), WeaponRailText2)
		|| !TestNotNull(TEXT("UI-P0-06 Weapon Rail Text3"), WeaponRailText3))
	{
		return false;
	}

	TestEqual(TEXT("UI-P0-06 Weapon Rail Designer default Collapsed"), WeaponRail->GetVisibility(), ESlateVisibility::Collapsed);
	TestNull(TEXT("UI-P0-06 fake Turret Rail Image 제거"), WeaponPanelWidget->GetWidgetFromName(FName(TEXT("Image_WeaponRail1"))));
	TestNull(TEXT("UI-P0-06 fake Ammo Rail Image 제거"), WeaponPanelWidget->GetWidgetFromName(FName(TEXT("Image_WeaponRail2"))));
	TestNull(TEXT("UI-P0-06 fake Reload Rail Image 제거"), WeaponPanelWidget->GetWidgetFromName(FName(TEXT("Image_WeaponRail3"))));

	// [v1.20.0] actual EquipmentPresetData.DisplayName이 알려진 Player-facing selection HUD 항목을 만드는 Helper입니다.
	auto MakeNamedSelectionItem = [](const TCHAR* DisplayName, const bool bSelected)
	{
		// [v1.20.0] 내부 ID 필드 없이 DisplayName과 selected 의미만 가진 결과 항목입니다.
		FCFWeaponSelectionHUDItem SelectionItem;
		SelectionItem.DisplayNameAvailability = ECFUIViewAvailability::Known;
		SelectionItem.DisplayName = FText::FromString(DisplayName);
		SelectionItem.bSelected = bSelected;
		return SelectionItem;
	};

	// [v1.20.0] 세 실제 selectable weapon 중 첫 번째가 선택된 대표 Player-facing ViewData입니다.
	FCFWeaponHUDData ThreeWeaponViewData;
	ThreeWeaponViewData.Availability = ECFUIViewAvailability::Known;
	ThreeWeaponViewData.DisplayNameAvailability = ECFUIViewAvailability::Known;
	ThreeWeaponViewData.DisplayName = FText::FromString(TEXT("PRIMARY CANNON"));
	ThreeWeaponViewData.WeaponSelectionAvailability = ECFUIViewAvailability::Known;
	ThreeWeaponViewData.SelectedWeaponIndex = 0;
	ThreeWeaponViewData.SelectableWeapons.Add(MakeNamedSelectionItem(TEXT("PRIMARY CANNON"), true));
	ThreeWeaponViewData.SelectableWeapons.Add(MakeNamedSelectionItem(TEXT("SECONDARY CANNON"), false));
	ThreeWeaponViewData.SelectableWeapons.Add(MakeNamedSelectionItem(TEXT("MISSILE POD"), false));

	// [v1.20.0] pure projection에서 선택 무기가 제거되고 Provider fixed order가 유지되는지 확인할 Rail 결과입니다.
	TArray<FCFWeaponRailPresentationEntry> RailEntries;
	UCFHUDPresenter::BuildWeaponRailEntries(ThreeWeaponViewData, RailEntries);
	TestEqual(TEXT("UI-P0-06 selected 제외 후 Rail 2개"), RailEntries.Num(), 2);
	if (RailEntries.Num() != 2)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 Rail 첫 비선택 순번+이름"), RailEntries[0].DisplayText.ToString(), FString(TEXT("02  SECONDARY CANNON")));
	TestEqual(TEXT("UI-P0-06 Rail 둘째 비선택 순번+이름"), RailEntries[1].DisplayText.ToString(), FString(TEXT("03  MISSILE POD")));

	// [v1.20.0] 실제 Production Widget에 synthetic ViewData를 적용할 Presenter입니다.
	UCFHUDPresenter* Presenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-06 Weapon Rail Presenter"), Presenter))
	{
		return false;
	}
	Presenter->SetProductionWidget(ProductionHUDWidget);

	// [v1.20.0] private UFUNCTION HandleHUDViewDataChanged를 실제 Provider delegate 경로와 동일하게 호출할 Reflection 함수입니다.
	UFunction* HandleViewDataFunction = Presenter->FindFunction(FName(TEXT("HandleHUDViewDataChanged")));
	if (!TestNotNull(TEXT("UI-P0-06 Weapon Rail HandleHUDViewDataChanged"), HandleViewDataFunction))
	{
		return false;
	}

	// [v1.20.0] Reflection UFUNCTION의 단일 ViewData 인자를 전달할 파라미터 구조입니다.
	struct FHandleHUDViewDataChangedParams
	{
		// [v1.20.0] Presenter에 적용할 전체 HUD ViewData입니다.
		FCFInGameUIViewData ViewData;
	};

	// [v1.20.0] Rail 관련 ViewData를 Production Presenter에 정확히 한 번 적용하는 Helper입니다.
	auto ApplyWeaponViewDataOnce = [Presenter, HandleViewDataFunction](const FCFWeaponHUDData& WeaponViewData)
	{
		// [v1.20.0] 이번 한 번의 Presenter 적용에 사용할 전체 ViewData 파라미터입니다.
		FHandleHUDViewDataChangedParams Params;
		Params.ViewData.Weapon = WeaponViewData;
		Presenter->ProcessEvent(HandleViewDataFunction, &Params);
	};

	ApplyWeaponViewDataOnce(ThreeWeaponViewData);
	TestEqual(TEXT("UI-P0-06 actual Rail visible"), WeaponRail->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-06 actual Rail Tile1 visible"), WeaponRailTile1->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-06 actual Rail Tile2 visible"), WeaponRailTile2->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-06 actual Rail Tile3 collapsed"), WeaponRailTile3->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-06 actual Rail Text1"), WeaponRailText1->GetText().ToString(), FString(TEXT("02  SECONDARY CANNON")));
	TestEqual(TEXT("UI-P0-06 actual Rail Text2"), WeaponRailText2->GetText().ToString(), FString(TEXT("03  MISSILE POD")));

	// [v1.20.0] 선택이 가운데 무기로 바뀌었을 때 기존 selected가 Rail로 복귀하고 새 selected가 Rail에서 빠지는 ViewData입니다.
	ThreeWeaponViewData.SelectedWeaponIndex = 1;
	ThreeWeaponViewData.DisplayName = FText::FromString(TEXT("SECONDARY CANNON"));
	ThreeWeaponViewData.SelectableWeapons[0].bSelected = false;
	ThreeWeaponViewData.SelectableWeapons[1].bSelected = true;
	ApplyWeaponViewDataOnce(ThreeWeaponViewData);
	TestEqual(TEXT("UI-P0-06 selection change Rail Text1"), WeaponRailText1->GetText().ToString(), FString(TEXT("01  PRIMARY CANNON")));
	TestEqual(TEXT("UI-P0-06 selection change Rail Text2"), WeaponRailText2->GetText().ToString(), FString(TEXT("03  MISSILE POD")));

	// [v1.20.0] DisplayName이 없는 실제 selection 항목도 내부 identity 대신 일반 WEAPON으로만 표시할 ViewData입니다.
	ThreeWeaponViewData.SelectableWeapons[2].DisplayNameAvailability = ECFUIViewAvailability::Unavailable;
	ThreeWeaponViewData.SelectableWeapons[2].DisplayName = FText::GetEmpty();
	ApplyWeaponViewDataOnce(ThreeWeaponViewData);
	TestEqual(TEXT("UI-P0-06 missing name generic fallback only"), WeaponRailText2->GetText().ToString(), FString(TEXT("03  WEAPON")));

	// [v1.20.0] 선택 1개 + 비선택 4개에서 앞 2개 + +2 overflow를 검증할 5무기 ViewData입니다.
	FCFWeaponHUDData FiveWeaponViewData;
	FiveWeaponViewData.Availability = ECFUIViewAvailability::Known;
	FiveWeaponViewData.DisplayNameAvailability = ECFUIViewAvailability::Known;
	FiveWeaponViewData.DisplayName = FText::FromString(TEXT("A"));
	FiveWeaponViewData.WeaponSelectionAvailability = ECFUIViewAvailability::Known;
	FiveWeaponViewData.SelectedWeaponIndex = 0;
	FiveWeaponViewData.SelectableWeapons.Add(MakeNamedSelectionItem(TEXT("A"), true));
	FiveWeaponViewData.SelectableWeapons.Add(MakeNamedSelectionItem(TEXT("B"), false));
	FiveWeaponViewData.SelectableWeapons.Add(MakeNamedSelectionItem(TEXT("C"), false));
	FiveWeaponViewData.SelectableWeapons.Add(MakeNamedSelectionItem(TEXT("D"), false));
	FiveWeaponViewData.SelectableWeapons.Add(MakeNamedSelectionItem(TEXT("E"), false));
	UCFHUDPresenter::BuildWeaponRailEntries(FiveWeaponViewData, RailEntries);
	TestEqual(TEXT("UI-P0-06 overflow Rail fixed 3 slots"), RailEntries.Num(), 3);
	if (RailEntries.Num() != 3)
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-06 overflow first fixed order"), RailEntries[0].DisplayText.ToString(), FString(TEXT("02  B")));
	TestEqual(TEXT("UI-P0-06 overflow second fixed order"), RailEntries[1].DisplayText.ToString(), FString(TEXT("03  C")));
	TestEqual(TEXT("UI-P0-06 overflow +2"), RailEntries[2].DisplayText.ToString(), FString(TEXT("+2")));
	TestTrue(TEXT("UI-P0-06 third slot overflow semantic"), RailEntries[2].bOverflow);
	ApplyWeaponViewDataOnce(FiveWeaponViewData);
	TestEqual(TEXT("UI-P0-06 overflow Tile3 visible"), WeaponRailTile3->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-06 overflow Production +2"), WeaponRailText3->GetText().ToString(), FString(TEXT("+2")));

	// [v1.20.0] 실제 선택 가능한 무기가 하나뿐이면 selected duplicate 없이 Rail 전체가 사라져야 하는 ViewData입니다.
	FCFWeaponHUDData SingleWeaponViewData;
	SingleWeaponViewData.Availability = ECFUIViewAvailability::Known;
	SingleWeaponViewData.DisplayNameAvailability = ECFUIViewAvailability::Known;
	SingleWeaponViewData.DisplayName = FText::FromString(TEXT("ONLY WEAPON"));
	SingleWeaponViewData.WeaponSelectionAvailability = ECFUIViewAvailability::Known;
	SingleWeaponViewData.SelectedWeaponIndex = 0;
	SingleWeaponViewData.SelectableWeapons.Add(MakeNamedSelectionItem(TEXT("ONLY WEAPON"), true));
	ApplyWeaponViewDataOnce(SingleWeaponViewData);
	TestEqual(TEXT("UI-P0-06 selected-only Rail collapsed"), WeaponRail->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-06 selected-only Tile1 collapsed"), WeaponRailTile1->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-06 selected-only Tile2 collapsed"), WeaponRailTile2->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-06 selected-only Tile3 collapsed"), WeaponRailTile3->GetVisibility(), ESlateVisibility::Collapsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006RpmGaugePresentationTest,
	"CarFight.UI.UI_P0_06.RpmGaugePresentationContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.15.0] 실제 Chaos Current RPM을 explicit RedlineStartRPM=0.85 / EngineMaxRPM=1.0 화면 위치로 변환하고 fallback 추정을 하지 않는지 검증합니다.
bool FCFHUDP006RpmGaugePresentationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.15.0] 실제 Runtime current RPM과 explicit authored Redline/Maximum 계약을 표현할 Vehicle HUD ViewData입니다.
	FCFVehicleHUDData VehicleViewData;
	VehicleViewData.EngineRpmAvailability = ECFUIViewAvailability::KnownZero;
	VehicleViewData.EngineRedlineStartRpmAvailability = ECFUIViewAvailability::Known;
	VehicleViewData.EngineMaximumRpmAvailability = ECFUIViewAvailability::Known;
	VehicleViewData.EngineRedlineStartRpm = 6000.0f;
	VehicleViewData.EngineMaximumRpm = 7000.0f;

	// [v1.15.0] Presenter가 계산한 승인 Tachometer 0~1 화면 비율입니다.
	float GaugeRatio = -1.0f;
	TestTrue(TEXT("UI-P0-06 RPM 0 mapping available"), UCFHUDPresenter::ResolveEngineRpmGaugePresentation(VehicleViewData, GaugeRatio));
	TestTrue(TEXT("UI-P0-06 RPM 0 -> 0.0"), FMath::IsNearlyEqual(GaugeRatio, 0.0f));

	VehicleViewData.EngineRpmAvailability = ECFUIViewAvailability::Known;
	VehicleViewData.EngineRpm = 3000.0f;
	TestTrue(TEXT("UI-P0-06 RPM half redline mapping available"), UCFHUDPresenter::ResolveEngineRpmGaugePresentation(VehicleViewData, GaugeRatio));
	TestTrue(TEXT("UI-P0-06 RPM 3000 -> 0.425"), FMath::IsNearlyEqual(GaugeRatio, 0.425f));

	VehicleViewData.EngineRpm = 6000.0f;
	TestTrue(TEXT("UI-P0-06 Redline mapping available"), UCFHUDPresenter::ResolveEngineRpmGaugePresentation(VehicleViewData, GaugeRatio));
	TestTrue(TEXT("UI-P0-06 Redline 6000 -> 0.85"), FMath::IsNearlyEqual(GaugeRatio, 0.85f));

	VehicleViewData.EngineRpm = 6500.0f;
	TestTrue(TEXT("UI-P0-06 post-redline mapping available"), UCFHUDPresenter::ResolveEngineRpmGaugePresentation(VehicleViewData, GaugeRatio));
	TestTrue(TEXT("UI-P0-06 RPM 6500 -> 0.925"), FMath::IsNearlyEqual(GaugeRatio, 0.925f));

	VehicleViewData.EngineRpm = 7000.0f;
	TestTrue(TEXT("UI-P0-06 maximum mapping available"), UCFHUDPresenter::ResolveEngineRpmGaugePresentation(VehicleViewData, GaugeRatio));
	TestTrue(TEXT("UI-P0-06 Maximum 7000 -> 1.0"), FMath::IsNearlyEqual(GaugeRatio, 1.0f));

	VehicleViewData.EngineRpm = 8000.0f;
	TestTrue(TEXT("UI-P0-06 overrun mapping available"), UCFHUDPresenter::ResolveEngineRpmGaugePresentation(VehicleViewData, GaugeRatio));
	TestTrue(TEXT("UI-P0-06 overrun clamps -> 1.0"), FMath::IsNearlyEqual(GaugeRatio, 1.0f));

	VehicleViewData.EngineRedlineStartRpmAvailability = ECFUIViewAvailability::Unavailable;
	TestFalse(TEXT("UI-P0-06 Redline 미설정은 EngineMaxRPM fallback 금지"), UCFHUDPresenter::ResolveEngineRpmGaugePresentation(VehicleViewData, GaugeRatio));

	VehicleViewData.EngineRedlineStartRpmAvailability = ECFUIViewAvailability::Known;
	VehicleViewData.EngineRedlineStartRpm = 0.0f;
	TestFalse(TEXT("UI-P0-06 explicit Redline 0은 mapping 불가"), UCFHUDPresenter::ResolveEngineRpmGaugePresentation(VehicleViewData, GaugeRatio));

		VehicleViewData.EngineRedlineStartRpm = 7000.0f;
	TestFalse(TEXT("UI-P0-06 Redline >= Maximum fail-closed"), UCFHUDPresenter::ResolveEngineRpmGaugePresentation(VehicleViewData, GaugeRatio));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006RpmGaugeVisualBindingTest,
	"CarFight.UI.UI_P0_06.RpmGaugeVisualBindingContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.22.0] 저장 Production SpeedGauge의 단일 UI Material이 explicit RPM ratio를 소비하고 unconfigured Redline에서 stale/fallback 값을 남기지 않는지 검증합니다.
bool FCFHUDP006RpmGaugeVisualBindingTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.22.0] 저장 Production HUD Widget을 실제 생성해 RPM Material binding을 검증할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-06 RPM Visual Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.22.0] DefaultGame.ini와 동일한 저장 Production HUD Generated Class입니다.
	UClass* ProductionHUDClass = LoadClass<UCFStyledWidgetBase>(
		nullptr,
		TEXT("/Game/CarFight/UI/HUD/WBP_CFInGameHUD.WBP_CFInGameHUD_C"));
	if (!TestNotNull(TEXT("UI-P0-06 RPM Visual Production HUD Class"), ProductionHUDClass))
	{
		return false;
	}

	// [v1.22.0] 저장 Production HUD Class로 만든 테스트 전용 Widget 인스턴스입니다.
	UCFStyledWidgetBase* ProductionHUDWidget = CreateWidget<UCFStyledWidgetBase>(TestWorld, ProductionHUDClass);
	if (!TestNotNull(TEXT("UI-P0-06 RPM Visual Production HUD Widget"), ProductionHUDWidget))
	{
		return false;
	}

	// [v1.22.0] Production VehiclePanel 자식 Widget 인스턴스입니다.
	UUserWidget* VehiclePanelWidget = Cast<UUserWidget>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("WBP_CFVehiclePanel"))));
	if (!TestNotNull(TEXT("UI-P0-06 RPM Visual VehiclePanel"), VehiclePanelWidget))
	{
		return false;
	}

	// [v1.22.0] 단일 동적 RPM Material Image와 Speed/Gear를 소유하는 저장 Production SpeedGauge입니다.
	UUserWidget* SpeedGaugeWidget = Cast<UUserWidget>(VehiclePanelWidget->GetWidgetFromName(FName(TEXT("WBP_CFSpeedGauge"))));
	if (!TestNotNull(TEXT("UI-P0-06 RPM Visual SpeedGauge"), SpeedGaugeWidget))
	{
		return false;
	}

	// [v1.22.0] Track·21 Tick·Red Zone을 한 Material에서 렌더링하는 실제 Production RPM Gauge Image입니다.
	UImage* RpmGaugeImage = Cast<UImage>(SpeedGaugeWidget->GetWidgetFromName(FName(TEXT("Image_RPMGauge"))));
	if (!TestNotNull(TEXT("UI-P0-06 RPM Visual 단일 Material Image"), RpmGaugeImage))
	{
		return false;
	}

	// [v1.22.0] 구형 UMG 그림 구조의 대표 첫 Tick은 저장 Production Tree에 더 이상 존재하면 안 됩니다.
	TestNull(
		TEXT("UI-P0-06 RPM Visual legacy ProgressBar Tick 제거"),
		SpeedGaugeWidget->GetWidgetFromName(FName(TEXT("ProgressBar_RPMTick00"))));
	// [v1.22.0] 구형 정적 Track 전용 Image도 새 Material Image와 중복하면 안 됩니다.
	TestNull(
		TEXT("UI-P0-06 RPM Visual legacy Track Image 제거"),
		SpeedGaugeWidget->GetWidgetFromName(FName(TEXT("Image_RPMTrackArt"))));

	// [v1.22.0] RPM Gauge Brush Material에서 만들어져 Presenter가 RPMRatio를 갱신할 Widget 전용 MID입니다.
	UMaterialInstanceDynamic* RpmGaugeMaterial = RpmGaugeImage->GetDynamicMaterial();
	if (!TestNotNull(TEXT("UI-P0-06 RPM Visual Dynamic Material"), RpmGaugeMaterial))
	{
		return false;
	}

	// [v1.22.0] 실제 Production Widget에 ViewData를 적용할 Presenter입니다.
	UCFHUDPresenter* Presenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-06 RPM Visual Presenter"), Presenter))
	{
		return false;
	}
	Presenter->SetProductionWidget(ProductionHUDWidget);

	// [v1.22.0] private UFUNCTION HandleHUDViewDataChanged를 실제 delegate와 동일한 경로로 호출할 Reflection 함수입니다.
	UFunction* HandleViewDataFunction = Presenter->FindFunction(FName(TEXT("HandleHUDViewDataChanged")));
	if (!TestNotNull(TEXT("UI-P0-06 RPM Visual HandleHUDViewDataChanged"), HandleViewDataFunction))
	{
		return false;
	}

	// [v1.22.0] Reflection UFUNCTION의 단일 ViewData 인자를 전달할 파라미터 구조입니다.
	struct FHandleHUDViewDataChangedParams
	{
		// [v1.22.0] Presenter에 적용할 전체 HUD ViewData입니다.
		FCFInGameUIViewData ViewData;
	};

	// [v1.22.0] 한 ViewData를 Presenter에 한 번 전달해 Production RPM Material Parameter를 갱신하는 테스트 Helper입니다.
	auto ApplyViewDataOnce = [Presenter, HandleViewDataFunction](const FCFInGameUIViewData& ViewData)
	{
		// [v1.22.0] 현재 한 번 적용할 Reflection 호출 파라미터입니다.
		FHandleHUDViewDataChangedParams Params;
		Params.ViewData = ViewData;
		Presenter->ProcessEvent(HandleViewDataFunction, &Params);
	};

	// [v1.22.0] 현재 MID의 RPMRatio가 기대 0~1 값인지 검증하는 Helper입니다.
	auto TestRpmRatio = [this, RpmGaugeMaterial](const TCHAR* TestLabel, const float ExpectedRatio)
	{
		// [v1.22.0] Presenter가 현재 Widget 전용 MID에 적용한 실제 RPMRatio 값입니다.
		const float ActualRpmRatio = RpmGaugeMaterial->K2_GetScalarParameterValue(FName(TEXT("RPMRatio")));
		TestTrue(TestLabel, FMath::IsNearlyEqual(ActualRpmRatio, ExpectedRatio, 0.0001f));
	};

	// [v1.22.0] 대표 VehicleData처럼 Max는 있지만 Redline은 0/Unavailable인 현재 Production 입력을 재현하는 전체 HUD ViewData입니다.
	FCFInGameUIViewData ViewData;
	ViewData.Vehicle.EngineRpmAvailability = ECFUIViewAvailability::Known;
	ViewData.Vehicle.EngineRpm = 6500.0f;
	ViewData.Vehicle.EngineMaximumRpmAvailability = ECFUIViewAvailability::Known;
	ViewData.Vehicle.EngineMaximumRpm = 6500.0f;
	ViewData.Vehicle.EngineRedlineStartRpmAvailability = ECFUIViewAvailability::Unavailable;
	ViewData.Vehicle.EngineRedlineStartRpm = 0.0f;
	ApplyViewDataOnce(ViewData);
	TestRpmRatio(TEXT("UI-P0-06 Redline Unconfigured는 Max fallback 없이 RPMRatio 0"), 0.0f);

	// [v1.22.0] Visual binding만 검증하기 위한 synthetic explicit Redline 계약이며 저장 VehicleData Asset에는 쓰지 않습니다.
	ViewData.Vehicle.EngineRedlineStartRpmAvailability = ECFUIViewAvailability::Known;
	ViewData.Vehicle.EngineRedlineStartRpm = 6000.0f;
	ViewData.Vehicle.EngineMaximumRpm = 7000.0f;
	ViewData.Vehicle.EngineRpm = 6000.0f;
	ApplyViewDataOnce(ViewData);
	TestRpmRatio(TEXT("UI-P0-06 Redline에서 RPMRatio 0.85"), 0.85f);

	ViewData.Vehicle.EngineRpm = 6500.0f;
	ApplyViewDataOnce(ViewData);
	TestRpmRatio(TEXT("UI-P0-06 Redline-Max 중간에서 RPMRatio 0.925"), 0.925f);

	ViewData.Vehicle.EngineRpm = 7000.0f;
	ApplyViewDataOnce(ViewData);
	TestRpmRatio(TEXT("UI-P0-06 Maximum에서 RPMRatio 1.0"), 1.0f);

	ViewData.Vehicle.EngineRedlineStartRpmAvailability = ECFUIViewAvailability::Unavailable;
	ViewData.Vehicle.EngineRedlineStartRpm = 0.0f;
	ApplyViewDataOnce(ViewData);
	TestRpmRatio(TEXT("UI-P0-06 Redline 제거 후 stale RPMRatio reset"), 0.0f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006ArmorSectorProductionVisualTest,
	"CarFight.UI.UI_P0_06.ArmorSectorProductionVisualContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.35.0] 저장 Production VehiclePanel의 6개 ArmorSector가 modular Direction Icon 슬롯·Texture 미바인딩 fallback과 Ratio 기반 Production 상태색을 실제 공통 Widget에서 일관되게 소비하는지 검증합니다.
bool FCFHUDP006ArmorSectorProductionVisualTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.33.0] 저장 Production HUD Widget을 실제 생성해 ArmorSector Presentation만 검증할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-06 Armor Visual Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.33.0] DefaultGame.ini와 동일한 저장 Production HUD Generated Class입니다.
	UClass* ProductionHUDClass = LoadClass<UCFStyledWidgetBase>(
		nullptr,
		TEXT("/Game/CarFight/UI/HUD/WBP_CFInGameHUD.WBP_CFInGameHUD_C"));
	if (!TestNotNull(TEXT("UI-P0-06 Armor Visual Production HUD Class"), ProductionHUDClass))
	{
		return false;
	}

	// [v1.33.0] 저장 Production HUD Class로 만든 테스트 전용 transient Widget 인스턴스입니다.
	UCFStyledWidgetBase* ProductionHUDWidget = CreateWidget<UCFStyledWidgetBase>(TestWorld, ProductionHUDClass);
	if (!TestNotNull(TEXT("UI-P0-06 Armor Visual Production HUD Widget"), ProductionHUDWidget))
	{
		return false;
	}

	// [v1.33.0] 저장 Production VehiclePanel 자식 Widget입니다.
	UUserWidget* VehiclePanelWidget = Cast<UUserWidget>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("WBP_CFVehiclePanel"))));
	if (!TestNotNull(TEXT("UI-P0-06 Armor Visual VehiclePanel"), VehiclePanelWidget))
	{
		return false;
	}

	// [v1.33.0] 탑다운 VehicleSilhouette와 6개 재사용 Sector를 소유하는 저장 ArmorBodyMap입니다.
	UUserWidget* ArmorBodyMapWidget = Cast<UUserWidget>(VehiclePanelWidget->GetWidgetFromName(FName(TEXT("WBP_CFArmorBodyMap"))));
	if (!TestNotNull(TEXT("UI-P0-06 Armor Visual BodyMap"), ArmorBodyMapWidget))
	{
		return false;
	}

	// [v1.33.0] Runtime Presenter가 안정 이름으로 찾는 여섯 방향 재사용 Sector 이름입니다.
	const FName ArmorSectorNames[] =
	{
		FName(TEXT("WBP_ArmorFront")),
		FName(TEXT("WBP_ArmorRight")),
		FName(TEXT("WBP_ArmorRear")),
		FName(TEXT("WBP_ArmorLeft")),
		FName(TEXT("WBP_ArmorTop")),
		FName(TEXT("WBP_ArmorBottom"))
	};

	// [v1.35.0] 6개 Sector 모두 동일 UCFArmorSectorWidget modular 슬롯/fallback 계약을 유지하는지 순회할 인덱스입니다.
	int32 ArmorSectorIndex = 0;
	for (; ArmorSectorIndex < UE_ARRAY_COUNT(ArmorSectorNames); ++ArmorSectorIndex)
	{
		// [v1.33.0] 현재 방향의 실제 저장 재사용 ArmorSector Widget입니다.
		UCFArmorSectorWidget* ArmorSectorWidget = Cast<UCFArmorSectorWidget>(ArmorBodyMapWidget->GetWidgetFromName(ArmorSectorNames[ArmorSectorIndex]));
		if (!TestNotNull(*FString::Printf(TEXT("UI-P0-06 Armor Visual Sector %s"), *ArmorSectorNames[ArmorSectorIndex].ToString()), ArmorSectorWidget))
		{
			return false;
		}

		// [v1.35.0] 현재 persisted 전이 단계에서 계속 남아 있는 기존 Plate Texture입니다. 새 modular Direction 의미는 이 Texture 존재 여부로 판정하지 않습니다.
		UTexture2D* ArmorPlateTexture = ArmorSectorWidget->GetConfiguredArmorPlateTexture();
		TestNotNull(*FString::Printf(TEXT("UI-P0-06 Armor Visual Plate Texture %s"), *ArmorSectorNames[ArmorSectorIndex].ToString()), ArmorPlateTexture);

		// [v1.35.0] Source Binding 전이므로 아직 연결되지 않아야 하는 별도 modular Direction Icon Texture입니다.
		UTexture2D* DirectionIconTexture = ArmorSectorWidget->GetConfiguredDirectionIconTexture();
		TestNull(*FString::Printf(TEXT("UI-P0-06 Armor Visual Direction Icon Texture pending %s"), *ArmorSectorNames[ArmorSectorIndex].ToString()), DirectionIconTexture);

		// [v1.35.0] WBP migration으로 실제 Overlay_Plate에 additive 저장된 Direction Icon Image입니다.
		UImage* DirectionIconImage = Cast<UImage>(ArmorSectorWidget->GetWidgetFromName(FName(TEXT("Image_DirectionIcon"))));
		if (!TestNotNull(*FString::Printf(TEXT("UI-P0-06 Armor Visual Direction Icon Image %s"), *ArmorSectorNames[ArmorSectorIndex].ToString()), DirectionIconImage))
		{
			return false;
		}

		// [v1.35.0] 실제 Presenter와 같은 공용 API를 호출해 현재 Ratio와 Texture 미바인딩 fallback visibility를 한 번 적용합니다.
		ArmorSectorWidget->SetArmorPercent(1.0f, true);

		// [v1.35.0] Direction Icon Texture가 아직 없을 때만 표시되어야 하는 fallback Direction Text입니다.
		UTextBlock* DirectionText = Cast<UTextBlock>(ArmorSectorWidget->GetWidgetFromName(FName(TEXT("Text_Direction"))));
		if (!TestNotNull(*FString::Printf(TEXT("UI-P0-06 Armor Visual Direction Text %s"), *ArmorSectorNames[ArmorSectorIndex].ToString()), DirectionText))
		{
			return false;
		}
		TestEqual(
			*FString::Printf(TEXT("UI-P0-06 Armor Visual unbound Icon collapsed %s"), *ArmorSectorNames[ArmorSectorIndex].ToString()),
			DirectionIconImage->GetVisibility(),
			ESlateVisibility::Collapsed);
		TestEqual(
			*FString::Printf(TEXT("UI-P0-06 Armor Visual unbound Icon Text fallback visible %s"), *ArmorSectorNames[ArmorSectorIndex].ToString()),
			DirectionText->GetVisibility(),
			ESlateVisibility::HitTestInvisible);
	}

	// [v1.33.0] Ratio 상태색 경계값을 대표로 검증할 Front 재사용 Sector입니다.
	UCFArmorSectorWidget* FrontArmorSector = Cast<UCFArmorSectorWidget>(ArmorBodyMapWidget->GetWidgetFromName(FName(TEXT("WBP_ArmorFront"))));
	if (!TestNotNull(TEXT("UI-P0-06 Armor Visual Front Sector"), FrontArmorSector))
	{
		return false;
	}

	// [v1.33.0] Front 방향 Texture와 상태 Tint를 함께 표시하는 실제 Production Plate Image입니다.
	UImage* FrontArmorPlateImage = Cast<UImage>(FrontArmorSector->GetWidgetFromName(FName(TEXT("Image_ArmorPlate"))));
	// [v1.33.0] Front 실제 Armor Ratio를 아래에서 위로 표시하는 Production 세로 ProgressBar입니다.
	UProgressBar* FrontArmorProgressBar = Cast<UProgressBar>(FrontArmorSector->GetWidgetFromName(FName(TEXT("ProgressBar_Armor"))));
	if (!TestNotNull(TEXT("UI-P0-06 Armor Visual Front Plate Image"), FrontArmorPlateImage)
		|| !TestNotNull(TEXT("UI-P0-06 Armor Visual Front ProgressBar"), FrontArmorProgressBar))
	{
		return false;
	}

	// [v1.33.0] Front Sector가 현재 Visual Context에서 사용하는 실제 UI Style Data입니다.
	const UCFUIStyleData* ArmorStyleData = FrontArmorSector->GetUIStyleData();
	if (!TestNotNull(TEXT("UI-P0-06 Armor Visual Style Data"), ArmorStyleData))
	{
		return false;
	}

	// [v1.33.0] Stable Armor 상태의 기대 의미 색상입니다.
	const FLinearColor StableArmorColor = ArmorStyleData->ResolveColor(ECFUIColorToken::Armor);
	// [v1.33.0] Caution Armor 상태의 기대 의미 색상입니다.
	const FLinearColor CautionArmorColor = ArmorStyleData->ResolveColor(ECFUIColorToken::StateCaution);
	// [v1.33.0] Critical Armor 상태의 기대 의미 색상입니다.
	const FLinearColor CriticalArmorColor = ArmorStyleData->ResolveColor(ECFUIColorToken::StateCritical);

	FrontArmorSector->SetArmorPercent(0.80f, true);
	TestTrue(TEXT("UI-P0-06 Armor Visual 80% Stable tint"), FrontArmorPlateImage->GetColorAndOpacity().Equals(StableArmorColor));
	TestTrue(TEXT("UI-P0-06 Armor Visual 80% Bar ratio"), FMath::IsNearlyEqual(FrontArmorProgressBar->GetPercent(), 0.80f));
	TestEqual(TEXT("UI-P0-06 Armor Visual 80% Bar visible"), FrontArmorProgressBar->GetVisibility(), ESlateVisibility::HitTestInvisible);

	FrontArmorSector->SetArmorPercent(0.50f, true);
	TestTrue(TEXT("UI-P0-06 Armor Visual 50% Caution tint"), FrontArmorPlateImage->GetColorAndOpacity().Equals(CautionArmorColor));
	TestTrue(TEXT("UI-P0-06 Armor Visual 50% Bar ratio"), FMath::IsNearlyEqual(FrontArmorProgressBar->GetPercent(), 0.50f));

	FrontArmorSector->SetArmorPercent(0.20f, true);
	TestTrue(TEXT("UI-P0-06 Armor Visual 20% Critical tint"), FrontArmorPlateImage->GetColorAndOpacity().Equals(CriticalArmorColor));
	TestTrue(TEXT("UI-P0-06 Armor Visual 20% Bar ratio"), FMath::IsNearlyEqual(FrontArmorProgressBar->GetPercent(), 0.20f));

	FrontArmorSector->SetArmorPercent(0.0f, true);
	TestTrue(TEXT("UI-P0-06 Armor Visual 0% Critical tint"), FrontArmorPlateImage->GetColorAndOpacity().Equals(CriticalArmorColor));
	TestTrue(TEXT("UI-P0-06 Armor Visual 0% empty Bar"), FMath::IsNearlyZero(FrontArmorProgressBar->GetPercent()));

	FrontArmorSector->SetArmorPercent(0.80f, false);
	TestEqual(TEXT("UI-P0-06 Armor Visual unavailable Bar collapsed"), FrontArmorProgressBar->GetVisibility(), ESlateVisibility::Collapsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006VehicleDefenseBarVisualTest,
	"CarFight.UI.UI_P0_06.VehicleDefenseBarVisualContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.34.0] 저장 VehiclePanel Frame과 Shield/Integrity ProgressBar가 현재 Production Visual 계약을 실제 Asset/Presenter 경로에서 만족하는지 검증합니다.
bool FCFHUDP006VehicleDefenseBarVisualTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.34.0] 저장 Production HUD를 transient 인스턴스로 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.34.0] DefaultGame.ini와 동일한 저장 Production HUD Generated Class입니다.
	UClass* ProductionHUDClass = LoadClass<UCFStyledWidgetBase>(
		nullptr,
		TEXT("/Game/CarFight/UI/HUD/WBP_CFInGameHUD.WBP_CFInGameHUD_C"));
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual HUD Class"), ProductionHUDClass))
	{
		return false;
	}

	// [v1.34.0] 저장 Production HUD Class로 만든 테스트 전용 transient Widget입니다.
	UCFStyledWidgetBase* ProductionHUDWidget = CreateWidget<UCFStyledWidgetBase>(TestWorld, ProductionHUDClass);
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual HUD Widget"), ProductionHUDWidget))
	{
		return false;
	}

	// [v1.34.0] Frame과 Shield/Integrity를 실제 소유하는 저장 VehiclePanel입니다.
	UCFStyledWidgetBase* VehiclePanelWidget = Cast<UCFStyledWidgetBase>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("WBP_CFVehiclePanel"))));
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual VehiclePanel"), VehiclePanelWidget))
	{
		return false;
	}

	// [v1.34.0] 저장 VehiclePanel의 전체 9-Slice Frame Brush를 소유하는 Border입니다.
	UBorder* VehiclePanelSurface = Cast<UBorder>(VehiclePanelWidget->GetWidgetFromName(FName(TEXT("Border_Surface"))));
	// [v1.34.0] 실제 Runtime Ratio와 Production Style을 받을 Shield ProgressBar입니다.
	UProgressBar* ShieldProgressBar = Cast<UProgressBar>(VehiclePanelWidget->GetWidgetFromName(FName(TEXT("ProgressBar_Shield"))));
	// [v1.34.0] 실제 Runtime Ratio와 Production Style을 받을 Integrity ProgressBar입니다.
	UProgressBar* IntegrityProgressBar = Cast<UProgressBar>(VehiclePanelWidget->GetWidgetFromName(FName(TEXT("ProgressBar_Integrity"))));
	// [v1.34.0] Shield semantic color가 Bar와 함께 적용될 저장 Icon입니다.
	UImage* ShieldImage = Cast<UImage>(VehiclePanelWidget->GetWidgetFromName(FName(TEXT("Image_Shield"))));
	// [v1.34.0] Integrity semantic color가 Bar와 함께 적용될 저장 Icon입니다.
	UImage* IntegrityImage = Cast<UImage>(VehiclePanelWidget->GetWidgetFromName(FName(TEXT("Image_Integrity"))));
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual Surface"), VehiclePanelSurface)
		|| !TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual Shield Bar"), ShieldProgressBar)
		|| !TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual Integrity Bar"), IntegrityProgressBar)
		|| !TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual Shield Icon"), ShieldImage)
		|| !TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual Integrity Icon"), IntegrityImage))
	{
		return false;
	}

	// [v1.34.0] Production VehiclePanel Frame Soft Reference를 소유하는 실제 HUD Visual Data입니다.
	UCFHUDVisualData* HUDVisualData = LoadObject<UCFHUDVisualData>(
		nullptr,
		TEXT("/Game/CarFight/UI/HUD/Visual/DA_CFHUDVisual_Default.DA_CFHUDVisual_Default"));
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual HUD Visual Data"), HUDVisualData))
	{
		return false;
	}

	// [v1.34.0] Visual Data가 지정한 실제 P2 VehiclePanel Frame Texture입니다.
	UTexture2D* ExpectedVehiclePanelFrame = HUDVisualData->VehiclePanelFrame.LoadSynchronous();
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual expected Frame Texture"), ExpectedVehiclePanelFrame))
	{
		return false;
	}

	// [v1.34.0] 저장 Border Surface가 현재 Brush에서 실제 소비하는 Resource UObject입니다.
	UObject* ActualVehiclePanelFrameResource = VehiclePanelSurface->Background.GetResourceObject();
	TestTrue(
		TEXT("UI-P0-06 Vehicle Defense Visual saved Frame resource matches HUDVisualData"),
		ActualVehiclePanelFrameResource == ExpectedVehiclePanelFrame);
	TestEqual(
		TEXT("UI-P0-06 Vehicle Defense Visual Frame uses 9-slice Box"),
		VehiclePanelSurface->Background.DrawAs,
		ESlateBrushDrawType::Box);

	// [v1.34.0] 실제 Production Widget에 synthetic ViewData를 한 번 적용할 Presenter입니다.
	UCFHUDPresenter* Presenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual Presenter"), Presenter))
	{
		return false;
	}
	Presenter->SetProductionWidget(ProductionHUDWidget);

	// [v1.34.0] private UFUNCTION을 Provider delegate와 같은 경로로 호출할 Reflection 함수입니다.
	UFunction* HandleViewDataFunction = Presenter->FindFunction(FName(TEXT("HandleHUDViewDataChanged")));
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual HandleHUDViewDataChanged"), HandleViewDataFunction))
	{
		return false;
	}

	// [v1.34.0] Reflection UFUNCTION의 단일 ViewData 인자를 전달하는 파라미터 구조입니다.
	struct FHandleHUDViewDataChangedParams
	{
		// [v1.34.0] Presenter에 적용할 전체 HUD ViewData입니다.
		FCFInGameUIViewData ViewData;
	};

	// [v1.34.0] Shield 78%, Integrity 42%의 실제 Runtime 의미를 모사할 synthetic HUD ViewData입니다.
	FCFInGameUIViewData ViewData;
	ViewData.Defense.Availability = ECFUIViewAvailability::Known;
	ViewData.Defense.ShieldAvailability = ECFUIViewAvailability::Known;
	ViewData.Defense.CurrentShield = 78.0f;
	ViewData.Defense.MaximumShield = 100.0f;
	ViewData.Defense.ShieldRatio = 0.78f;
	ViewData.Defense.IntegrityAvailability = ECFUIViewAvailability::Known;
	ViewData.Defense.CurrentIntegrity = 42.0f;
	ViewData.Defense.MaximumIntegrity = 100.0f;
	ViewData.Defense.IntegrityRatio = 0.42f;

	// [v1.34.0] synthetic ViewData를 Production Presenter에 정확히 한 번 전달할 Reflection 호출 인자입니다.
	FHandleHUDViewDataChangedParams ApplyParams;
	ApplyParams.ViewData = ViewData;
	Presenter->ProcessEvent(HandleViewDataFunction, &ApplyParams);

	// [v1.34.2] Shield ProgressBar에 Presenter가 실제 적용한 Style이며 intrinsic Brush 크기 회귀를 직접 검사합니다.
	const FProgressBarStyle& ShieldDefenseBarStyle = ShieldProgressBar->GetWidgetStyle();
	// [v1.34.2] Integrity ProgressBar에 Presenter가 실제 적용한 Style이며 intrinsic Brush 크기 회귀를 직접 검사합니다.
	const FProgressBarStyle& IntegrityDefenseBarStyle = IntegrityProgressBar->GetWidgetStyle();

	// [v1.34.0] Production VehiclePanel이 실제 소비하는 현재 Style Context입니다.
	const UCFUIStyleData* VehiclePanelStyleData = VehiclePanelWidget->GetUIStyleData();
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Defense Visual Style Data"), VehiclePanelStyleData))
	{
		return false;
	}

	// [v1.34.0] Shield Bar/Icon이 공유해야 하는 현재 semantic 색상입니다.
	const FLinearColor ExpectedShieldColor = VehiclePanelStyleData->ResolveColor(ECFUIColorToken::Shield);
	// [v1.34.0] Integrity Bar/Icon이 공유해야 하는 현재 semantic 색상입니다.
	const FLinearColor ExpectedIntegrityColor = VehiclePanelStyleData->ResolveColor(ECFUIColorToken::Integrity);
	// [v1.34.0] 기본 ProgressBar primitive와 구분되는 Production inset padding 계약입니다.
	const FVector2D ExpectedDefenseBarPadding(2.0f, 2.0f);

	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Shield Ratio"), FMath::IsNearlyEqual(ShieldProgressBar->GetPercent(), 0.78f));
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Integrity Ratio"), FMath::IsNearlyEqual(IntegrityProgressBar->GetPercent(), 0.42f));
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Shield semantic Fill"), ShieldProgressBar->GetFillColorAndOpacity().Equals(ExpectedShieldColor));
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Integrity semantic Fill"), IntegrityProgressBar->GetFillColorAndOpacity().Equals(ExpectedIntegrityColor));
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Shield Icon semantic color"), ShieldImage->GetColorAndOpacity().Equals(ExpectedShieldColor));
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Integrity Icon semantic color"), IntegrityImage->GetColorAndOpacity().Equals(ExpectedIntegrityColor));
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Shield inset padding"), ShieldProgressBar->GetBorderPadding().Equals(ExpectedDefenseBarPadding));
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Integrity inset padding"), IntegrityProgressBar->GetBorderPadding().Equals(ExpectedDefenseBarPadding));
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Shield Background intrinsic height"), ShieldDefenseBarStyle.BackgroundImage.ImageSize.Y > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Shield Fill intrinsic height"), ShieldDefenseBarStyle.FillImage.ImageSize.Y > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Shield Marquee intrinsic height"), ShieldDefenseBarStyle.MarqueeImage.ImageSize.Y > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Integrity Background intrinsic height"), IntegrityDefenseBarStyle.BackgroundImage.ImageSize.Y > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Integrity Fill intrinsic height"), IntegrityDefenseBarStyle.FillImage.ImageSize.Y > KINDA_SMALL_NUMBER);
	TestTrue(TEXT("UI-P0-06 Vehicle Defense Visual Integrity Marquee intrinsic height"), IntegrityDefenseBarStyle.MarqueeImage.ImageSize.Y > KINDA_SMALL_NUMBER);
	TestEqual(TEXT("UI-P0-06 Vehicle Defense Visual Shield fill direction"), ShieldProgressBar->GetBarFillType(), EProgressBarFillType::LeftToRight);
	TestEqual(TEXT("UI-P0-06 Vehicle Defense Visual Integrity fill direction"), IntegrityProgressBar->GetBarFillType(), EProgressBarFillType::LeftToRight);
	TestEqual(TEXT("UI-P0-06 Vehicle Defense Visual Shield fill style"), ShieldProgressBar->GetBarFillStyle(), EProgressBarFillStyle::Scale);
	TestEqual(TEXT("UI-P0-06 Vehicle Defense Visual Integrity fill style"), IntegrityProgressBar->GetBarFillStyle(), EProgressBarFillStyle::Scale);
	TestEqual(TEXT("UI-P0-06 Vehicle Defense Visual Shield visible"), ShieldProgressBar->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-06 Vehicle Defense Visual Integrity visible"), IntegrityProgressBar->GetVisibility(), ESlateVisibility::HitTestInvisible);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006ResourceVisualSlotContractTest,
	"CarFight.UI.UI_P0_06.ResourceVisualSlotContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.13.0] Stage B Production WeaponPanel이 Compact Presentation을 ViewData 적용당 정확히 한 번 소비하고 legacy 고정 Row와 중복하지 않는지 검증합니다.
bool FCFHUDP006ResourceVisualSlotContractTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.13.0] 저장 Production HUD Widget을 실제 생성해 Presenter 의미 슬롯 적용을 검증할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-06 Stage B Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.13.0] DefaultGame.ini와 동일한 저장 Production HUD Generated Class입니다.
	UClass* ProductionHUDClass = LoadClass<UCFStyledWidgetBase>(
		nullptr,
		TEXT("/Game/CarFight/UI/HUD/WBP_CFInGameHUD.WBP_CFInGameHUD_C"));
	if (!TestNotNull(TEXT("UI-P0-06 Stage B Production HUD Class"), ProductionHUDClass))
	{
		return false;
	}

	// [v1.13.0] 저장 Production HUD Class로 만든 테스트 전용 Widget 인스턴스입니다.
	UCFStyledWidgetBase* ProductionHUDWidget = CreateWidget<UCFStyledWidgetBase>(TestWorld, ProductionHUDClass);
	if (!TestNotNull(TEXT("UI-P0-06 Stage B Production HUD Widget"), ProductionHUDWidget))
	{
		return false;
	}

	// [v1.13.0] 실제 Production WeaponPanel 자식 Widget 인스턴스입니다.
	UUserWidget* WeaponPanelWidget = Cast<UUserWidget>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("WBP_CFWeaponPanel"))));
	if (!TestNotNull(TEXT("UI-P0-06 Stage B Production WeaponPanel"), WeaponPanelWidget))
	{
		return false;
	}

	// [v1.13.0] Stage B 새 Compact Resource Presentation 전체 Container입니다.
	UWidget* ResourcePresentationWidget = WeaponPanelWidget->GetWidgetFromName(FName(TEXT("VerticalBox_ResourcePresentation")));
	// [v1.13.0] Stage B Primary 의미 Row입니다.
	UWidget* PrimaryResourceRow = WeaponPanelWidget->GetWidgetFromName(FName(TEXT("HorizontalBox_PrimaryResource")));
	// [v1.13.0] Stage B Primary Player-facing 문자열 Widget입니다.
	UTextBlock* PrimaryResourceText = Cast<UTextBlock>(WeaponPanelWidget->GetWidgetFromName(FName(TEXT("Text_WeaponPrimaryResource"))));
	// [v1.13.0] Stage B Primary 선택적 진행률 Widget입니다.
	UProgressBar* PrimaryResourceProgress = Cast<UProgressBar>(WeaponPanelWidget->GetWidgetFromName(FName(TEXT("ProgressBar_PrimaryResource"))));
	// [v1.13.0] Stage B Secondary A/B 전체 Row입니다.
	UWidget* SecondaryResourcesRow = WeaponPanelWidget->GetWidgetFromName(FName(TEXT("HorizontalBox_SecondaryResources")));
	// [v1.13.0] Stage B 첫 번째 Secondary Container입니다.
	UWidget* SecondaryResourceA = WeaponPanelWidget->GetWidgetFromName(FName(TEXT("VerticalBox_SecondaryResourceA")));
	// [v1.13.0] Stage B 첫 번째 Secondary 문자열 Widget입니다.
	UTextBlock* SecondaryResourceTextA = Cast<UTextBlock>(WeaponPanelWidget->GetWidgetFromName(FName(TEXT("Text_WeaponSecondaryResourceA"))));
	// [v1.13.0] Stage B 두 번째 Secondary Container입니다.
	UWidget* SecondaryResourceB = WeaponPanelWidget->GetWidgetFromName(FName(TEXT("VerticalBox_SecondaryResourceB")));
	// [v1.13.0] Stage B FireState Row입니다.
	UWidget* FireStateRow = WeaponPanelWidget->GetWidgetFromName(FName(TEXT("HorizontalBox_FireState")));
	// [v1.13.0] Stage B FireState Player-facing 문자열 Widget입니다.
	UTextBlock* FireStateText = Cast<UTextBlock>(WeaponPanelWidget->GetWidgetFromName(FName(TEXT("Text_WeaponFireState"))));
	// [v1.13.0] Stage B FireState 선택적 진행률 Widget입니다.
	UProgressBar* FireStateProgress = Cast<UProgressBar>(WeaponPanelWidget->GetWidgetFromName(FName(TEXT("ProgressBar_FireState"))));
	// [v1.13.0] Header 우측 label-less Reserve 숫자 Widget입니다.
	UTextBlock* ReserveAmmoText = Cast<UTextBlock>(WeaponPanelWidget->GetWidgetFromName(FName(TEXT("Text_WeaponReserveAmmo"))));
	// [v1.13.0] Player-facing WeaponGroup Runtime 전 기본 숨김을 유지할 Compact Rail입니다.
	UWidget* WeaponRail = WeaponPanelWidget->GetWidgetFromName(FName(TEXT("HorizontalBox_WeaponRail")));

	if (!TestNotNull(TEXT("UI-P0-06 Stage B Resource Container"), ResourcePresentationWidget)
		|| !TestNotNull(TEXT("UI-P0-06 Stage B Primary Row"), PrimaryResourceRow)
		|| !TestNotNull(TEXT("UI-P0-06 Stage B Primary Text"), PrimaryResourceText)
		|| !TestNotNull(TEXT("UI-P0-06 Stage B Primary Progress"), PrimaryResourceProgress)
		|| !TestNotNull(TEXT("UI-P0-06 Stage B Secondary Row"), SecondaryResourcesRow)
		|| !TestNotNull(TEXT("UI-P0-06 Stage B Secondary A"), SecondaryResourceA)
		|| !TestNotNull(TEXT("UI-P0-06 Stage B Secondary A Text"), SecondaryResourceTextA)
		|| !TestNotNull(TEXT("UI-P0-06 Stage B Secondary B"), SecondaryResourceB)
		|| !TestNotNull(TEXT("UI-P0-06 Stage B FireState Row"), FireStateRow)
		|| !TestNotNull(TEXT("UI-P0-06 Stage B FireState Text"), FireStateText)
		|| !TestNotNull(TEXT("UI-P0-06 Stage B FireState Progress"), FireStateProgress)
		|| !TestNotNull(TEXT("UI-P0-06 Stage B Header Reserve"), ReserveAmmoText)
		|| !TestNotNull(TEXT("UI-P0-06 Stage B Weapon Rail"), WeaponRail))
	{
		return false;
	}

	// [v1.13.0] Stage B 저장 Asset에서 제거되어 새 의미 슬롯과 동시에 존재하면 안 되는 legacy Launcher Row입니다.
	TestNull(TEXT("UI-P0-06 Stage B legacy Launcher Row 제거"), WeaponPanelWidget->GetWidgetFromName(FName(TEXT("HorizontalBox_LauncherSequence"))));
	// [v1.13.0] Stage B 저장 Asset에서 제거되어야 하는 legacy Ammo Row입니다.
	TestNull(TEXT("UI-P0-06 Stage B legacy Ammo Row 제거"), WeaponPanelWidget->GetWidgetFromName(FName(TEXT("HorizontalBox_Ammo"))));
	// [v1.13.0] 실제 Heat Runtime이 없으므로 Stage B 저장 Asset에서 제거되어야 하는 legacy Heat Row입니다.
	TestNull(TEXT("UI-P0-06 Stage B legacy Heat Row 제거"), WeaponPanelWidget->GetWidgetFromName(FName(TEXT("HorizontalBox_Heat"))));
	// [v1.13.0] Stage B FireState 의미 슬롯으로 치환되어야 하는 legacy Cooldown Row입니다.
	TestNull(TEXT("UI-P0-06 Stage B legacy Cooldown Row 제거"), WeaponPanelWidget->GetWidgetFromName(FName(TEXT("HorizontalBox_Cooldown"))));

	// [v1.13.0] 실제 Production Widget에 ViewData를 적용할 Presenter입니다.
	UCFHUDPresenter* Presenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-06 Stage B Presenter"), Presenter))
	{
		return false;
	}
	Presenter->SetProductionWidget(ProductionHUDWidget);

	// [v1.13.0] private UFUNCTION HandleHUDViewDataChanged를 실제 delegate와 동일한 경로로 호출할 Reflection 함수입니다.
	UFunction* HandleViewDataFunction = Presenter->FindFunction(FName(TEXT("HandleHUDViewDataChanged")));
	if (!TestNotNull(TEXT("UI-P0-06 Stage B HandleHUDViewDataChanged"), HandleViewDataFunction))
	{
		return false;
	}

	// [v1.13.0] Reflection UFUNCTION의 단일 ViewData 인자를 전달할 파라미터 구조입니다.
	struct FHandleHUDViewDataChangedParams
	{
		// [v1.13.0] Presenter에 적용할 전체 HUD ViewData입니다.
		FCFInGameUIViewData ViewData;
	};

	// [v1.13.0] 한 ViewData를 Presenter에 정확히 한 번 전달해 Production 의미 슬롯을 갱신하는 테스트 Helper입니다.
	auto ApplyViewDataOnce = [Presenter, HandleViewDataFunction](const FCFInGameUIViewData& ViewData)
	{
		// [v1.13.0] 현재 한 번 적용할 Reflection 호출 파라미터입니다.
		FHandleHUDViewDataChangedParams Params;
		Params.ViewData = ViewData;
		Presenter->ProcessEvent(HandleViewDataFunction, &Params);
	};

	// [v1.13.0] Ammo Primary, Reserve Header와 Cooldown FireState를 포함하는 평상시 Production HUD 입력입니다.
	FCFInGameUIViewData ViewData;
	ViewData.Weapon.Availability = ECFUIViewAvailability::Known;
	ViewData.Weapon.DisplayNameAvailability = ECFUIViewAvailability::Known;
	ViewData.Weapon.DisplayName = FText::FromString(TEXT("STAGE B TEST WEAPON"));
	ViewData.Weapon.LauncherPattern = ECFLauncherFirePattern::Salvo;

	// [v1.13.0] 평상시 Primary `3 / 8`과 Launcher 중 Secondary를 제공할 실제 Ammo Resource입니다.
	FCFWeaponResourceHUDData AmmoChannel;
	AmmoChannel.ChannelType = ECFWeaponResourceChannelType::Ammo;
	AmmoChannel.DisplayMode = ECFWeaponResourceDisplayMode::CountPair;
	AmmoChannel.Availability = ECFUIViewAvailability::Known;
	AmmoChannel.bHasCurrentValue = true;
	AmmoChannel.CurrentValue = 3.0f;
	AmmoChannel.bHasMaximumValue = true;
	AmmoChannel.MaximumValue = 8.0f;
	AmmoChannel.bIsActive = true;
	AmmoChannel.bIsVisible = true;
	ViewData.Weapon.ResourceChannels.Add(AmmoChannel);

	// [v1.13.0] Resource Container가 아니라 Header 우측 `6`으로 표시할 실제 ReserveAmmo Resource입니다.
	FCFWeaponResourceHUDData ReserveChannel;
	ReserveChannel.ChannelType = ECFWeaponResourceChannelType::ReserveAmmo;
	ReserveChannel.DisplayMode = ECFWeaponResourceDisplayMode::Count;
	ReserveChannel.Availability = ECFUIViewAvailability::Known;
	ReserveChannel.bHasCurrentValue = true;
	ReserveChannel.CurrentValue = 6.0f;
	ReserveChannel.bIsActive = true;
	ReserveChannel.bIsVisible = true;
	ViewData.Weapon.ResourceChannels.Add(ReserveChannel);

	// [v1.13.0] 평상시 FireState `0.5 s`와 0.75 진행률을 제공할 실제 Cooldown Resource입니다.
	FCFWeaponResourceHUDData CooldownChannel;
	CooldownChannel.ChannelType = ECFWeaponResourceChannelType::Cooldown;
	CooldownChannel.DisplayMode = ECFWeaponResourceDisplayMode::TimeRemaining;
	CooldownChannel.Availability = ECFUIViewAvailability::Known;
	CooldownChannel.bHasMaximumValue = true;
	CooldownChannel.MaximumValue = 2.0f;
	CooldownChannel.bHasNormalizedValue = true;
	CooldownChannel.NormalizedValue = 0.75f;
	CooldownChannel.bHasRemainingTimeSeconds = true;
	CooldownChannel.RemainingTimeSeconds = 0.5f;
	CooldownChannel.bIsActive = true;
	CooldownChannel.bIsVisible = true;
	CooldownChannel.bBlocksFire = true;
	ViewData.Weapon.ResourceChannels.Add(CooldownChannel);

	ApplyViewDataOnce(ViewData);
	TestEqual(TEXT("UI-P0-06 Stage B 평상시 Primary Ammo"), PrimaryResourceText->GetText().ToString(), FString(TEXT("3 / 8")));
	TestEqual(TEXT("UI-P0-06 Stage B Header Reserve 6"), ReserveAmmoText->GetText().ToString(), FString(TEXT("6")));
	TestEqual(TEXT("UI-P0-06 Stage B 평상시 FireState Cooldown"), FireStateText->GetText().ToString(), FString(TEXT("0.5 s")));
	TestEqual(TEXT("UI-P0-06 Stage B 평상시 Secondary Row Collapsed"), SecondaryResourcesRow->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-06 Stage B 평상시 Primary Progress Collapsed"), PrimaryResourceProgress->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-06 Stage B 평상시 FireState Progress 표시"), FireStateProgress->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestTrue(TEXT("UI-P0-06 Stage B 평상시 FireState Progress 0.75"), FMath::IsNearlyEqual(FireStateProgress->GetPercent(), 0.75f));
	TestEqual(TEXT("UI-P0-06 Stage B Weapon Rail Collapsed"), WeaponRail->GetVisibility(), ESlateVisibility::Collapsed);

	// [v1.13.0] Launcher Active에서 Primary `SALVO 2 / 4`, Ammo Secondary, FireState 숨김을 만들 실제 Launcher Resource입니다.
	FCFWeaponResourceHUDData LauncherChannel;
	LauncherChannel.ChannelType = ECFWeaponResourceChannelType::LauncherSequence;
	LauncherChannel.DisplayMode = ECFWeaponResourceDisplayMode::Sequence;
	LauncherChannel.Availability = ECFUIViewAvailability::Known;
	LauncherChannel.bHasCurrentValue = true;
	LauncherChannel.CurrentValue = 2.0f;
	LauncherChannel.bHasMaximumValue = true;
	LauncherChannel.MaximumValue = 4.0f;
	LauncherChannel.bHasNormalizedValue = true;
	LauncherChannel.NormalizedValue = 0.5f;
	LauncherChannel.bIsActive = true;
	LauncherChannel.bIsVisible = true;
	ViewData.Weapon.ResourceChannels.Add(LauncherChannel);
	ViewData.Weapon.LauncherSequenceRevision = 10;
	ApplyViewDataOnce(ViewData);

	TestEqual(TEXT("UI-P0-06 Stage B Launcher Primary"), PrimaryResourceText->GetText().ToString(), FString(TEXT("SALVO 2 / 4")));
	TestEqual(TEXT("UI-P0-06 Stage B Launcher Primary Progress 표시"), PrimaryResourceProgress->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestTrue(TEXT("UI-P0-06 Stage B Launcher Primary Progress 0.5"), FMath::IsNearlyEqual(PrimaryResourceProgress->GetPercent(), 0.5f));
	TestEqual(TEXT("UI-P0-06 Stage B Launcher 중 Secondary A Ammo"), SecondaryResourceTextA->GetText().ToString(), FString(TEXT("3 / 8")));
	TestEqual(TEXT("UI-P0-06 Stage B Launcher 중 Secondary Row 표시"), SecondaryResourcesRow->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-06 Stage B Launcher 중 Secondary A 표시"), SecondaryResourceA->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-06 Stage B Launcher 중 Secondary B Collapsed"), SecondaryResourceB->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-06 Stage B Launcher 중 FireState Collapsed"), FireStateRow->GetVisibility(), ESlateVisibility::Collapsed);

	// [v1.13.0] Active 뒤 새 Revision에서 한 ViewData 주기만 보여야 할 terminal Launcher Resource의 배열 인덱스입니다.
	const int32 LauncherChannelIndex = ViewData.Weapon.ResourceChannels.Num() - 1;
	ViewData.Weapon.ResourceChannels[LauncherChannelIndex].bIsActive = false;
	ViewData.Weapon.ResourceChannels[LauncherChannelIndex].CurrentValue = 4.0f;
	ViewData.Weapon.ResourceChannels[LauncherChannelIndex].NormalizedValue = 1.0f;
	ViewData.Weapon.LauncherSequenceRevision = 11;
	ApplyViewDataOnce(ViewData);
	TestEqual(TEXT("UI-P0-06 Stage B terminal 정확히 첫 적용에서 유지"), PrimaryResourceText->GetText().ToString(), FString(TEXT("SALVO 4 / 4")));
	TestTrue(TEXT("UI-P0-06 Stage B terminal Progress 1"), FMath::IsNearlyEqual(PrimaryResourceProgress->GetPercent(), 1.0f));
	TestEqual(TEXT("UI-P0-06 Stage B terminal Ammo Secondary 유지"), SecondaryResourceTextA->GetText().ToString(), FString(TEXT("3 / 8")));

	// [v1.13.0] 같은 terminal Revision의 다음 ViewData 적용에서는 terminal Snapshot이 재소비되지 않고 Ammo/Cooldown으로 복귀해야 합니다.
	ApplyViewDataOnce(ViewData);
	TestEqual(TEXT("UI-P0-06 Stage B terminal 다음 Ammo Primary 복귀"), PrimaryResourceText->GetText().ToString(), FString(TEXT("3 / 8")));
	TestEqual(TEXT("UI-P0-06 Stage B terminal 다음 Secondary Row Collapsed"), SecondaryResourcesRow->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-06 Stage B terminal 다음 Cooldown FireState"), FireStateText->GetText().ToString(), FString(TEXT("0.5 s")));
	TestEqual(TEXT("UI-P0-06 Stage B terminal 다음 FireState 표시"), FireStateRow->GetVisibility(), ESlateVisibility::HitTestInvisible);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006WeaponDisplayNameTest,
	"CarFight.UI.UI_P0_06.WeaponDisplayNameRuntimeViewData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.10.0] 호환되는 실제 활성 EquipmentPresetData.DisplayName만 HUD Weapon DisplayName으로 전달하고 내부 ID fallback을 만들지 않는지 검증합니다.
bool FCFHUDP006WeaponDisplayNameTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.10.0] 실제 VehicleWeaponComp와 HUD Provider를 함께 실행할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-06 Weapon DisplayName Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.10.0] 실제 VehicleWeaponComp 기본 서브오브젝트를 보유한 테스트 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("UI-P0-06 Weapon DisplayName Vehicle Pawn"), VehiclePawn))
	{
		return false;
	}

	// [v1.10.0] 실제 Weapon Runtime이 활성 MountProfile과 Hardpoint를 해석할 Transient VehicleData입니다.
	UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(VehiclePawn, TEXT("HUDDisplayNameVehicleData"));
	// [v1.10.0] Player-facing 표시 이름을 제공할 Transient EquipmentPresetData입니다.
	UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>(VehiclePawn, TEXT("HUDDisplayNameEquipmentPreset"));
	// [v1.10.0] EquipmentPresetData 무장 패키지 완성도를 만족할 실제 Transient TurretMountData입니다.
	UCFTurretMountData* TurretMountData = NewObject<UCFTurretMountData>(VehiclePawn, TEXT("HUDDisplayNameTurretMount"));
	// [v1.10.0] EquipmentPresetData 무장 패키지와 활성 WeaponData를 제공할 실제 Transient WeaponData입니다.
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(VehiclePawn, TEXT("HUDDisplayNameWeaponData"));
	// [v1.10.0] 현재 활성 장비 선택을 소유할 실제 차량 Weapon Runtime 컴포넌트입니다.
	UCFVehicleWeaponComp* WeaponComponent = VehiclePawn->GetVehicleWeaponComp();
	if (!TestNotNull(TEXT("UI-P0-06 VehicleData"), VehicleData)
		|| !TestNotNull(TEXT("UI-P0-06 EquipmentPresetData"), EquipmentPresetData)
		|| !TestNotNull(TEXT("UI-P0-06 TurretMountData"), TurretMountData)
		|| !TestNotNull(TEXT("UI-P0-06 WeaponData"), WeaponData)
		|| !TestNotNull(TEXT("UI-P0-06 VehicleWeaponComp"), WeaponComponent))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.10.0] Weapon Runtime이 참조할 실제 하드포인트 위치 ID입니다.
	const FName HardpointSlotId(TEXT("HUD_DisplayName_Slot"));
	// [v1.10.0] Weapon Runtime이 선택할 실제 MountProfile ID입니다.
	const FName MountProfileId(TEXT("HUD_DisplayName_Profile"));

	// [v1.10.0] MountProfile이 참조할 최소 실제 Hardpoint 계약입니다.
	FCFVehicleHardpointSlot HardpointSlot;
	HardpointSlot.LocationSlotId = HardpointSlotId;
	VehicleData->HardpointSlots.Add(HardpointSlot);

	// [v1.10.0] Transient EquipmentPresetData를 실제 현재 장비로 해석할 MountProfile입니다.
	FCFVehicleMountProfile MountProfile;
	MountProfile.MountProfileId = MountProfileId;
	MountProfile.LocationSlotRef = HardpointSlotId;
	MountProfile.MountType = ECFVehicleMountType::Turret;
	MountProfile.SizeLimit = ECFVehicleWeaponSize::Large;
	VehicleData->MountProfiles.Add(MountProfile);

	EquipmentPresetData->EquipmentId = TEXT("Internal_Equipment_Id_Must_Not_Render");
	EquipmentPresetData->DisplayName = FText::FromString(TEXT("HUD Test Cannon"));
	EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
	EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Large;
	EquipmentPresetData->DefaultTurretMountData = TurretMountData;
	EquipmentPresetData->DefaultWeaponData = WeaponData;
	WeaponData->WeaponId = TEXT("Internal_Weapon_Id_Must_Not_Render");

	TestTrue(
		TEXT("UI-P0-06 실제 Fitting-style EquipmentPreset Runtime 초기화"),
		WeaponComponent->InitializeWeaponRuntimeFromFitting(VehiclePawn, VehicleData, MountProfileId, EquipmentPresetData));
	TestTrue(TEXT("UI-P0-06 활성 EquipmentPreset 호환"), WeaponComponent->IsActiveEquipmentPresetCompatible());
	TestEqual(TEXT("UI-P0-06 활성 EquipmentPreset 일치"), WeaponComponent->GetActiveEquipmentPresetData(), EquipmentPresetData);

	// [v1.10.0] 실제 차량 Weapon Runtime을 Player-facing HUD ViewData로 변환할 Provider입니다.
	UCFHUDDataProvider* DataProvider = NewObject<UCFHUDDataProvider>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-06 Weapon DisplayName HUD Data Provider"), DataProvider))
	{
		VehiclePawn->Destroy();
		return false;
	}
	DataProvider->RebindCurrentPawn(VehiclePawn);

	// [v1.10.0] 호환되는 현재 EquipmentPresetData의 Player-facing 이름이 반영된 첫 ViewData입니다.
	const FCFWeaponHUDData NamedWeaponViewData = DataProvider->GetCurrentViewData().Weapon;
	TestEqual(TEXT("UI-P0-06 Weapon DisplayName Availability Known"), NamedWeaponViewData.DisplayNameAvailability, ECFUIViewAvailability::Known);
	TestEqual(TEXT("UI-P0-06 Weapon DisplayName 실제 EquipmentPreset 값"), NamedWeaponViewData.DisplayName.ToString(), FString(TEXT("HUD Test Cannon")));
	TestNotEqual(TEXT("UI-P0-06 내부 WeaponId 비노출"), NamedWeaponViewData.DisplayName.ToString(), FString(TEXT("Internal_Weapon_Id_Must_Not_Render")));
	TestNotEqual(TEXT("UI-P0-06 내부 EquipmentId 비노출"), NamedWeaponViewData.DisplayName.ToString(), FString(TEXT("Internal_Equipment_Id_Must_Not_Render")));

	EquipmentPresetData->DisplayName = FText::GetEmpty();
	DataProvider->RefreshViewData();

	// [v1.10.0] 공개 이름을 지운 뒤 내부 ID fallback 없이 Unavailable로 복귀한 ViewData입니다.
	const FCFWeaponHUDData EmptyNameWeaponViewData = DataProvider->GetCurrentViewData().Weapon;
	TestEqual(TEXT("UI-P0-06 빈 DisplayName은 Unavailable"), EmptyNameWeaponViewData.DisplayNameAvailability, ECFUIViewAvailability::Unavailable);
	TestTrue(TEXT("UI-P0-06 빈 DisplayName에서 내부 ID fallback 없음"), EmptyNameWeaponViewData.DisplayName.IsEmpty());

	DataProvider->ShutdownProvider();
	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006RpmGaugeRuntimeSourceTest,
	"CarFight.UI.UI_P0_06.RpmGaugeRuntimeSourceContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.16.0] Current RPM은 Chaos Runtime, Redline/Maximum은 Current Pawn VehicleData의 explicit authored 값에서만 오는지 검증합니다.
bool FCFHUDP006RpmGaugeRuntimeSourceTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.16.0] 실제 ACFVehiclePawn과 Chaos Movement를 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-06 RPM Source Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.16.0] Chaos current RPM과 Current VehicleData를 함께 제공할 실제 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("UI-P0-06 RPM Source Vehicle Pawn"), VehiclePawn))
	{
		return false;
	}

	// [v1.16.0] HUD Maximum/Redline source를 명시할 Transient VehicleData입니다.
	UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(VehiclePawn, TEXT("HUDRpmSourceVehicleData"));
	if (!TestNotNull(TEXT("UI-P0-06 RPM Source VehicleData"), VehicleData))
	{
		VehiclePawn->Destroy();
		return false;
	}
	VehicleData->VehicleMovementConfig.EngineIdleRPM = 1000.0f;
	VehicleData->VehicleMovementConfig.RedlineStartRPM = 6000.0f;
	VehicleData->VehicleMovementConfig.EngineMaxRPM = 7000.0f;
	VehiclePawn->VehicleData = VehicleData;

	// [v1.16.0] 차량이 소유한 실제 Drive Runtime입니다.
	UCFVehicleDriveComp* VehicleDriveComponent = VehiclePawn->GetVehicleDriveComp();
	if (!TestNotNull(TEXT("UI-P0-06 RPM Source Drive Component"), VehicleDriveComponent))
	{
		VehiclePawn->Destroy();
		return false;
	}
	VehicleDriveComponent->CacheVehicleMovementComponent();

	// [v1.16.0] Current Engine RPM의 유일한 Runtime source인 실제 UE 5.8 Chaos Movement입니다.
	UChaosWheeledVehicleMovementComponent* VehicleMovementComponent = VehicleDriveComponent->GetVehicleMovementComponent();
	if (!TestNotNull(TEXT("UI-P0-06 RPM Source Chaos Movement"), VehicleMovementComponent))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.16.0] Provider 결과와 비교할 실제 Chaos current Engine RPM입니다.
	const float ExpectedCurrentEngineRpm = FMath::Max(0.0f, VehicleMovementComponent->GetEngineRotationSpeed());
	// [v1.16.0] 실제 Runtime과 authored VehicleData를 HUD ViewData로 분리 투영할 Provider입니다.
	UCFHUDDataProvider* DataProvider = NewObject<UCFHUDDataProvider>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-06 RPM Source HUD Provider"), DataProvider))
	{
		VehiclePawn->Destroy();
		return false;
	}
	DataProvider->RebindCurrentPawn(VehiclePawn);

	// [v1.16.0] explicit Redline이 설정된 첫 Vehicle HUD ViewData입니다.
	const FCFVehicleHUDData ConfiguredViewData = DataProvider->GetCurrentViewData().Vehicle;
	TestTrue(TEXT("UI-P0-06 Current RPM은 실제 Chaos 값"), FMath::IsNearlyEqual(ConfiguredViewData.EngineRpm, ExpectedCurrentEngineRpm));
	TestEqual(
		TEXT("UI-P0-06 Current RPM availability는 Chaos 값 기준"),
		ConfiguredViewData.EngineRpmAvailability,
		ExpectedCurrentEngineRpm <= KINDA_SMALL_NUMBER ? ECFUIViewAvailability::KnownZero : ECFUIViewAvailability::Known);
	TestEqual(TEXT("UI-P0-06 Redline explicit source Known"), ConfiguredViewData.EngineRedlineStartRpmAvailability, ECFUIViewAvailability::Known);
	TestTrue(TEXT("UI-P0-06 Redline VehicleData 6000"), FMath::IsNearlyEqual(ConfiguredViewData.EngineRedlineStartRpm, 6000.0f));
	TestEqual(TEXT("UI-P0-06 Maximum explicit source Known"), ConfiguredViewData.EngineMaximumRpmAvailability, ECFUIViewAvailability::Known);
	TestTrue(TEXT("UI-P0-06 Maximum VehicleData 7000"), FMath::IsNearlyEqual(ConfiguredViewData.EngineMaximumRpm, 7000.0f));

	VehicleData->VehicleMovementConfig.RedlineStartRPM = 0.0f;
	DataProvider->RefreshViewData();

	// [v1.16.0] Redline 미설정으로 전환한 뒤 EngineMaxRPM fallback이 없는 Vehicle HUD ViewData입니다.
	const FCFVehicleHUDData UnconfiguredViewData = DataProvider->GetCurrentViewData().Vehicle;
	TestEqual(TEXT("UI-P0-06 Redline 0은 Unavailable"), UnconfiguredViewData.EngineRedlineStartRpmAvailability, ECFUIViewAvailability::Unavailable);
	TestTrue(TEXT("UI-P0-06 Redline 0 값 유지"), FMath::IsNearlyZero(UnconfiguredViewData.EngineRedlineStartRpm));
	TestEqual(TEXT("UI-P0-06 Redline 미설정이어도 Maximum source 유지"), UnconfiguredViewData.EngineMaximumRpmAvailability, ECFUIViewAvailability::Known);
	TestTrue(TEXT("UI-P0-06 Redline 미설정이어도 Maximum 7000"), FMath::IsNearlyEqual(UnconfiguredViewData.EngineMaximumRpm, 7000.0f));

	DataProvider->ShutdownProvider();
	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP006VehicleDriveRuntimeTest,
	"CarFight.UI.UI_P0_06.VehicleDriveRuntimeViewData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.8.0] 실제 ACFVehiclePawn의 UE 5.8 Chaos Engine RPM과 Current Gear가 추정 없이 HUD Vehicle ViewData에 전달되는지 검증합니다.
bool FCFHUDP006VehicleDriveRuntimeTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.8.0] 실제 ACFVehiclePawn과 Chaos Vehicle Movement를 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Runtime Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.8.0] HUD Provider의 실제 Vehicle Runtime Source로 사용할 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>();
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Pawn"), VehiclePawn))
	{
		return false;
	}

	// [v1.8.0] 차량이 소유한 실제 Drive Runtime입니다.
	UCFVehicleDriveComp* VehicleDriveComponent = VehiclePawn->GetVehicleDriveComp();
	if (!TestNotNull(TEXT("UI-P0-06 Vehicle Drive Component"), VehicleDriveComponent))
	{
		VehiclePawn->Destroy();
		return false;
	}
	VehicleDriveComponent->CacheVehicleMovementComponent();

	// [v1.8.0] 실제 Engine RPM과 Current Gear의 원본인 UE 5.8 Chaos Movement입니다.
	UChaosWheeledVehicleMovementComponent* VehicleMovementComponent = VehicleDriveComponent->GetVehicleMovementComponent();
	if (!TestNotNull(TEXT("UI-P0-06 Chaos Wheeled Vehicle Movement"), VehicleMovementComponent))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.8.0] Provider 결과와 직접 비교할 실제 Chaos Engine RPM입니다.
	const float ExpectedEngineRpm = FMath::Max(0.0f, VehicleMovementComponent->GetEngineRotationSpeed());
	// [v1.8.0] Provider 결과와 직접 비교할 실제 Chaos Current Gear입니다.
	const int32 ExpectedCurrentGear = VehicleMovementComponent->GetCurrentGear();
	// [v1.8.0] Current Gear의 공식 음수/0/양수 의미를 HUD R/N/전진 단수 Text로 변환한 기대값입니다.
	const FString ExpectedGearText = ExpectedCurrentGear < 0
		? TEXT("R")
		: (ExpectedCurrentGear == 0 ? TEXT("N") : FString::FromInt(ExpectedCurrentGear));

	// [v1.8.0] 실제 차량 Runtime을 읽기 전용 HUD ViewData로 변환할 Provider입니다.
	UCFHUDDataProvider* DataProvider = NewObject<UCFHUDDataProvider>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-06 HUD Data Provider"), DataProvider))
	{
		VehiclePawn->Destroy();
		return false;
	}
	DataProvider->RebindCurrentPawn(VehiclePawn);

	// [v1.8.0] 실제 Chaos Vehicle Runtime에서 생성된 Vehicle HUD ViewData입니다.
	const FCFVehicleHUDData VehicleViewData = DataProvider->GetCurrentViewData().Vehicle;
	TestTrue(
		TEXT("UI-P0-06 Engine RPM 실제 Chaos 값 일치"),
		FMath::IsNearlyEqual(VehicleViewData.EngineRpm, ExpectedEngineRpm));
	TestEqual(
		TEXT("UI-P0-06 Engine RPM Availability 실제 Provider 사용"),
		VehicleViewData.EngineRpmAvailability,
		ExpectedEngineRpm <= KINDA_SMALL_NUMBER ? ECFUIViewAvailability::KnownZero : ECFUIViewAvailability::Known);
	TestEqual(TEXT("UI-P0-06 Gear Availability Known"), VehicleViewData.GearAvailability, ECFUIViewAvailability::Known);
	TestEqual(TEXT("UI-P0-06 Gear Text 실제 Current Gear 변환"), VehicleViewData.GearText.ToString(), ExpectedGearText);

	DataProvider->ShutdownProvider();
	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDDefenseRuntimeViewDataTest,
	"CarFight.UI.UI_P0_03.DefenseRuntimeViewData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.5.0] 실제 Shield·Armor·Integrity 피해와 Shield 재생이 Provider Defense ViewData에 즉시 반영되는지 검증합니다.
bool FCFHUDDefenseRuntimeViewDataTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.5.0] 실제 Pawn Defense Runtime과 Provider 이벤트를 연결할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("Defense HUD Runtime Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.5.0] 최대 Shield 40, 6방향 Armor 100, Integrity 100의 실제 차량 Fixture입니다.
	FCFHUDDefenseFixture DefenseFixture = CreateHUDDefenseFixture(TestWorld, TEXT("HUDDefenseRuntimePawn"), 40.0f);
	// [v1.5.0] 자기 피해 차단과 독립된 실제 공격 주체입니다.
	AActor* InstigatorActor = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("Defense HUD 차량 Pawn"), DefenseFixture.VehiclePawn)
		|| !TestNotNull(TEXT("Defense HUD Health Component"), DefenseFixture.HealthComponent)
		|| !TestNotNull(TEXT("Defense HUD Defense Component"), DefenseFixture.DefenseComponent)
		|| !TestNotNull(TEXT("Defense HUD 공격 주체"), InstigatorActor))
	{
		return false;
	}

	// [v1.5.0] ULocalPlayer와 UISubsystem의 ClassWithin 계약을 만족하는 Transient LocalPlayer입니다.
	ULocalPlayer* TestLocalPlayer = GEngine ? NewObject<ULocalPlayer>(GEngine) : nullptr;
	// [v1.5.0] 실제 OnCurrentPawnChanged 경로를 제공할 Transient UI Subsystem입니다.
	UCFUISubsystem* UISubsystem = TestLocalPlayer ? NewObject<UCFUISubsystem>(TestLocalPlayer) : nullptr;
	// [v1.5.0] 실제 VehicleDefense/Health 이벤트를 Defense ViewData로 변환할 Provider입니다.
	UCFHUDDataProvider* DataProvider = NewObject<UCFHUDDataProvider>(GetTransientPackage());
	if (!TestNotNull(TEXT("Defense HUD Transient LocalPlayer"), TestLocalPlayer)
		|| !TestNotNull(TEXT("Defense HUD Transient UISubsystem"), UISubsystem)
		|| !TestNotNull(TEXT("Defense HUD DataProvider"), DataProvider))
	{
		return false;
	}

	TestTrue(TEXT("Defense HUD Provider 초기화"), DataProvider->InitializeProvider(UISubsystem));
	UISubsystem->OnCurrentPawnChanged.Broadcast(nullptr, DefenseFixture.VehiclePawn);

	// [v1.5.0] 실제 피해 전 Provider가 읽은 초기 Defense ViewData입니다.
	const FCFDefenseHUDData InitialDefenseViewData = DataProvider->GetCurrentViewData().Defense;
	TestEqual(TEXT("초기 Defense Availability Known"), InitialDefenseViewData.Availability, ECFUIViewAvailability::Known);
	TestEqual(TEXT("초기 Shield Availability Known"), InitialDefenseViewData.ShieldAvailability, ECFUIViewAvailability::Known);
	TestTrue(TEXT("초기 Shield 40"), FMath::IsNearlyEqual(InitialDefenseViewData.CurrentShield, 40.0f));
	TestTrue(TEXT("초기 Shield Ratio 1"), FMath::IsNearlyEqual(InitialDefenseViewData.ShieldRatio, 1.0f));
	TestTrue(TEXT("초기 Front Armor Ratio 1"), FMath::IsNearlyEqual(InitialDefenseViewData.FrontArmorRatio, 1.0f));
	TestTrue(TEXT("초기 Integrity 100"), FMath::IsNearlyEqual(InitialDefenseViewData.CurrentIntegrity, 100.0f));
	TestTrue(TEXT("초기 Integrity Ratio 1"), FMath::IsNearlyEqual(InitialDefenseViewData.IntegrityRatio, 1.0f));

	// [v1.5.0] Shield 40을 소진한 뒤 남은 40을 AP 50으로 Armor 20 / Integrity 20에 분배할 피해 데이터입니다.
	UCFDamageData* DamageData = NewObject<UCFDamageData>(DefenseFixture.VehiclePawn, TEXT("HUDDefenseRuntimeDamage"));
	DamageData->BaseDamage = 80.0f;
	DamageData->ArmorPenetration = 50.0f;

	// [v1.5.0] Production HitScan/Projectile과 같은 정식 Actor 방어 진입점 결과입니다.
	FCFVehicleDamageResult DamageResult;
	TestTrue(
		TEXT("Defense HUD 실제 방어 피해 적용"),
		UCFVehicleDefenseComp::TryApplyDamageToActor(
			BuildHUDDefenseHitContext(DefenseFixture.VehiclePawn, InstigatorActor, DamageData),
			DamageResult));
	TestTrue(TEXT("실제 피해 Shield 흡수 40"), FMath::IsNearlyEqual(DamageResult.DamageAbsorbedByShield, 40.0f));
	TestTrue(TEXT("실제 피해 Armor 흡수 20"), FMath::IsNearlyEqual(DamageResult.DamageAbsorbedByArmor, 20.0f));
	TestTrue(TEXT("실제 피해 Integrity 적용 20"), FMath::IsNearlyEqual(DamageResult.DamageAppliedToIntegrity, 20.0f));

	// [v1.5.0] 실제 Shield/Armor/Health 이벤트 직후 Provider가 보존한 손상 Defense ViewData입니다.
	const FCFDefenseHUDData DamagedDefenseViewData = DataProvider->GetCurrentViewData().Defense;
	TestEqual(TEXT("Shield 0은 KnownZero"), DamagedDefenseViewData.ShieldAvailability, ECFUIViewAvailability::KnownZero);
	TestTrue(TEXT("피해 후 Shield 0"), FMath::IsNearlyZero(DamagedDefenseViewData.CurrentShield));
	TestTrue(TEXT("피해 후 Shield Ratio 0"), FMath::IsNearlyZero(DamagedDefenseViewData.ShieldRatio));
	TestTrue(TEXT("피해 후 Front Armor Ratio 0.8"), FMath::IsNearlyEqual(DamagedDefenseViewData.FrontArmorRatio, 0.8f));
	TestTrue(TEXT("피해 후 Right Armor Ratio 유지 1"), FMath::IsNearlyEqual(DamagedDefenseViewData.RightArmorRatio, 1.0f));
	TestTrue(TEXT("피해 후 Rear Armor Ratio 유지 1"), FMath::IsNearlyEqual(DamagedDefenseViewData.RearArmorRatio, 1.0f));
	TestTrue(TEXT("피해 후 Left Armor Ratio 유지 1"), FMath::IsNearlyEqual(DamagedDefenseViewData.LeftArmorRatio, 1.0f));
	TestTrue(TEXT("피해 후 Top Armor Ratio 유지 1"), FMath::IsNearlyEqual(DamagedDefenseViewData.TopArmorRatio, 1.0f));
	TestTrue(TEXT("피해 후 Bottom Armor Ratio 유지 1"), FMath::IsNearlyEqual(DamagedDefenseViewData.BottomArmorRatio, 1.0f));
	TestTrue(TEXT("피해 후 Integrity 80"), FMath::IsNearlyEqual(DamagedDefenseViewData.CurrentIntegrity, 80.0f));
	TestTrue(TEXT("피해 후 Integrity Ratio 0.8"), FMath::IsNearlyEqual(DamagedDefenseViewData.IntegrityRatio, 0.8f));

	DefenseFixture.DefenseComponent->TickComponent(6.0f, LEVELTICK_All, nullptr);

	// [v1.5.0] 5초 지연 뒤 1초간 초당 10 Shield가 실제 재생된 Provider ViewData입니다.
	const FCFDefenseHUDData RegeneratingDefenseViewData = DataProvider->GetCurrentViewData().Defense;
	TestEqual(TEXT("재생 Shield Availability Known"), RegeneratingDefenseViewData.ShieldAvailability, ECFUIViewAvailability::Known);
	TestTrue(TEXT("재생 후 Shield 10"), FMath::IsNearlyEqual(RegeneratingDefenseViewData.CurrentShield, 10.0f));
	TestTrue(TEXT("재생 후 Shield Ratio 0.25"), FMath::IsNearlyEqual(RegeneratingDefenseViewData.ShieldRatio, 0.25f));
	TestTrue(TEXT("재생 중 상태 전달"), RegeneratingDefenseViewData.bShieldRegenerating);
	TestTrue(TEXT("재생 중 Armor는 0.8 유지"), FMath::IsNearlyEqual(RegeneratingDefenseViewData.FrontArmorRatio, 0.8f));
	TestTrue(TEXT("재생 중 Integrity는 0.8 유지"), FMath::IsNearlyEqual(RegeneratingDefenseViewData.IntegrityRatio, 0.8f));

	DataProvider->ShutdownProvider();
	DefenseFixture.VehiclePawn->Destroy();
	InstigatorActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDDefensePIEViewDataTest,
	"CarFight.UI.UI_P0_03.DefensePIEViewData",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.6.0] 저장된 M_VehicleDefensePIE를 실제 PIE로 실행해 Defense Runtime→Provider ViewData 기술 경로를 검증합니다.
bool FCFHUDDefensePIEViewDataTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.6.0] CF-FQ-033에서 USER/Automation 방어 fixture로 사용하는 저장 테스트 맵입니다.
	const FString VehicleDefensePIEMapPath = TEXT("/Game/Maps/M_VehicleDefensePIE");
	ADD_LATENT_AUTOMATION_COMMAND(FEditorLoadMap(VehicleDefensePIEMapPath));
	ADD_LATENT_AUTOMATION_COMMAND(FStartPIECommand(false));
	ADD_LATENT_AUTOMATION_COMMAND(FCFVerifyHUDDefensePIECommand(this));
	ADD_LATENT_AUTOMATION_COMMAND(FEndPlayMapCommand());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDProviderPawnRebindTest,
	"CarFight.UI.UI_P0_03.ProviderPawnRebind",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.5.0] UISubsystem OnCurrentPawnChanged가 Provider의 Old Pawn 이벤트를 해제하고 새 Pawn Runtime만 소비하는지 검증합니다.
bool FCFHUDProviderPawnRebindTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.0.0] 서로 다른 두 차량 Pawn을 생성할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("HUD Provider Rebind Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.5.0] 첫 번째 Provider Source로 사용할 Shield 40 차량 Fixture입니다.
	FCFHUDDefenseFixture FirstFixture = CreateHUDDefenseFixture(TestWorld, TEXT("HUDRebindFirstPawn"), 40.0f);
	// [v1.5.0] 두 번째 Provider Source로 사용할 Shield 80 차량 Fixture입니다.
	FCFHUDDefenseFixture SecondFixture = CreateHUDDefenseFixture(TestWorld, TEXT("HUDRebindSecondPawn"), 80.0f);
	// [v1.5.0] 두 Pawn의 실제 방어 이벤트를 발생시킬 독립 공격 Actor입니다.
	AActor* InstigatorActor = TestWorld->SpawnActor<AActor>();
	if (!TestNotNull(TEXT("첫 번째 HUD Source 차량"), FirstFixture.VehiclePawn)
		|| !TestNotNull(TEXT("두 번째 HUD Source 차량"), SecondFixture.VehiclePawn)
		|| !TestNotNull(TEXT("HUD Rebind 공격 주체"), InstigatorActor))
	{
		return false;
	}

	// [v1.0.2] ULocalPlayer의 Engine ClassWithin과 ULocalPlayerSubsystem의 LocalPlayer ClassWithin을 모두 만족하는 테스트 Outer입니다.
	ULocalPlayer* TestLocalPlayer = GEngine ? NewObject<ULocalPlayer>(GEngine) : nullptr;
	// [v1.0.1] Current Pawn 이벤트만 제공할 Transient UISubsystem입니다.
	UCFUISubsystem* UISubsystem = TestLocalPlayer ? NewObject<UCFUISubsystem>(TestLocalPlayer) : nullptr;
	// [v1.0.0] Rebind 수명 계약을 검증할 Transient HUD Provider입니다.
	UCFHUDDataProvider* DataProvider = NewObject<UCFHUDDataProvider>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient LocalPlayer"), TestLocalPlayer)
		|| !TestNotNull(TEXT("Transient UISubsystem"), UISubsystem)
		|| !TestNotNull(TEXT("Transient HUD Data Provider"), DataProvider))
	{
		return false;
	}

	TestTrue(TEXT("HUD Provider 초기화"), DataProvider->InitializeProvider(UISubsystem));
	// [v1.0.0] 초기 Null Source 다음 첫 차량 Rebind 직전 Generation입니다.
	const int32 InitialGeneration = DataProvider->GetBindingGeneration();

	UISubsystem->OnCurrentPawnChanged.Broadcast(nullptr, FirstFixture.VehiclePawn);
	TestTrue(TEXT("첫 차량 Rebind"), DataProvider->GetBoundVehiclePawn() == FirstFixture.VehiclePawn);
	TestTrue(TEXT("첫 차량 Rebind Generation 증가"), DataProvider->GetBindingGeneration() > InitialGeneration);
	TestTrue(TEXT("첫 차량 Shield 40 반영"), FMath::IsNearlyEqual(DataProvider->GetCurrentViewData().Defense.CurrentShield, 40.0f));

	// [v1.0.0] 두 번째 차량 Rebind 직전 Generation입니다.
	const int32 FirstPawnGeneration = DataProvider->GetBindingGeneration();
	UISubsystem->OnCurrentPawnChanged.Broadcast(FirstFixture.VehiclePawn, SecondFixture.VehiclePawn);
	TestTrue(TEXT("두 번째 차량 Rebind"), DataProvider->GetBoundVehiclePawn() == SecondFixture.VehiclePawn);
	TestTrue(TEXT("두 번째 차량 Rebind Generation 증가"), DataProvider->GetBindingGeneration() > FirstPawnGeneration);
	TestTrue(TEXT("두 번째 차량 Shield 80 반영"), FMath::IsNearlyEqual(DataProvider->GetCurrentViewData().Defense.CurrentShield, 80.0f));

	// [v1.5.0] Old Pawn 구독 해제 여부를 검사하기 직전 Provider ViewData Revision입니다.
	const int32 RevisionBeforeOldPawnDamage = DataProvider->GetCurrentViewData().Revision;
	// [v1.5.0] Rebind 후 첫 차량에 실제 Shield 이벤트를 발생시킬 피해 데이터입니다.
	UCFDamageData* FirstPawnDamageData = NewObject<UCFDamageData>(FirstFixture.VehiclePawn, TEXT("HUDRebindOldPawnDamage"));
	FirstPawnDamageData->BaseDamage = 20.0f;
	FirstPawnDamageData->ArmorPenetration = 0.0f;
	// [v1.5.0] Old Pawn에 실제로 적용된 방어 피해 결과입니다.
	FCFVehicleDamageResult FirstPawnDamageResult;
	TestTrue(
		TEXT("Rebind 후 Old Pawn 실제 피해 적용"),
		UCFVehicleDefenseComp::TryApplyDamageToActor(
			BuildHUDDefenseHitContext(FirstFixture.VehiclePawn, InstigatorActor, FirstPawnDamageData),
			FirstPawnDamageResult));
	TestTrue(TEXT("Old Pawn 자체 Shield는 20으로 감소"), FMath::IsNearlyEqual(FirstFixture.DefenseComponent->GetCurrentShield(), 20.0f));
	TestEqual(TEXT("Old Pawn 이벤트로 Provider Revision 증가 없음"), DataProvider->GetCurrentViewData().Revision, RevisionBeforeOldPawnDamage);
	TestTrue(TEXT("Old Pawn 이벤트 후 Provider는 두 번째 Shield 80 유지"), FMath::IsNearlyEqual(DataProvider->GetCurrentViewData().Defense.CurrentShield, 80.0f));

	// [v1.5.0] 현재 Bound Pawn 이벤트가 Provider를 즉시 갱신하는지 검사할 직전 Revision입니다.
	const int32 RevisionBeforeCurrentPawnDamage = DataProvider->GetCurrentViewData().Revision;
	// [v1.5.0] 현재 두 번째 차량에 실제 Shield 이벤트를 발생시킬 피해 데이터입니다.
	UCFDamageData* SecondPawnDamageData = NewObject<UCFDamageData>(SecondFixture.VehiclePawn, TEXT("HUDRebindCurrentPawnDamage"));
	SecondPawnDamageData->BaseDamage = 20.0f;
	SecondPawnDamageData->ArmorPenetration = 0.0f;
	// [v1.5.0] 현재 Bound Pawn에 실제로 적용된 방어 피해 결과입니다.
	FCFVehicleDamageResult SecondPawnDamageResult;
	TestTrue(
		TEXT("Current Pawn 실제 피해 적용"),
		UCFVehicleDefenseComp::TryApplyDamageToActor(
			BuildHUDDefenseHitContext(SecondFixture.VehiclePawn, InstigatorActor, SecondPawnDamageData),
			SecondPawnDamageResult));
	TestTrue(TEXT("Current Pawn 이벤트로 Provider Revision 증가"), DataProvider->GetCurrentViewData().Revision > RevisionBeforeCurrentPawnDamage);
	TestTrue(TEXT("Current Pawn Shield 60 즉시 반영"), FMath::IsNearlyEqual(DataProvider->GetCurrentViewData().Defense.CurrentShield, 60.0f));

	UISubsystem->OnCurrentPawnChanged.Broadcast(SecondFixture.VehiclePawn, nullptr);
	TestNull(TEXT("Pawn 해제 후 Provider Source 없음"), DataProvider->GetBoundVehiclePawn());
	TestEqual(TEXT("Pawn 해제 후 Vehicle ViewData Unavailable"), DataProvider->GetCurrentViewData().Vehicle.Availability, ECFUIViewAvailability::Unavailable);
	TestEqual(TEXT("Pawn 해제 후 Defense ViewData Unavailable"), DataProvider->GetCurrentViewData().Defense.Availability, ECFUIViewAvailability::Unavailable);

	DataProvider->ShutdownProvider();
	FirstFixture.VehiclePawn->Destroy();
	SecondFixture.VehiclePawn->Destroy();
	InstigatorActor->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDWeaponLauncherPresentationTest,
	"CarFight.UI.UI_P0_03.WeaponLauncherSequencePresentation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.4.0] HeavyCannon SingleCycle, Ripple과 Salvo가 무기별 예외 없이 공통 Presentation lifecycle을 사용하는지 검증합니다.
bool FCFHUDWeaponLauncherPresentationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.4.0] 상태를 실제 Presenter 인스턴스처럼 순차적으로 보존할 Transient Presenter입니다.
	UCFHUDPresenter* Presenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("Transient HUD Presenter"), Presenter))
	{
		return false;
	}

	// [v1.4.0] 공통 Launcher Sequence 테스트에 사용할 WeaponId입니다.
	const FName LauncherWeaponId(TEXT("LauncherWeapon"));

	// [v1.4.0] Ripple 4발 중 실제 승인 2발 상태를 재현하는 Active ViewData입니다.
	FCFWeaponHUDData RippleActiveViewData;
	RippleActiveViewData.WeaponId = LauncherWeaponId;
	RippleActiveViewData.LauncherAvailability = ECFUIViewAvailability::Known;
	RippleActiveViewData.LauncherSequenceRevision = 10;
	RippleActiveViewData.bLauncherSequenceActive = true;
	RippleActiveViewData.LauncherPattern = ECFLauncherFirePattern::Ripple;
	RippleActiveViewData.LauncherTotalProjectileCount = 4;
	RippleActiveViewData.LauncherAcceptedProjectileCount = 2;
	RippleActiveViewData.LauncherRemainingProjectileCount = 2;

	// [v1.4.0] Presenter가 만들어야 하는 Player-facing Sequence 문구입니다.
	FText SequenceText;
	// [v1.4.0] Presenter가 만들어야 하는 0~1 진행률입니다.
	float SequenceProgress = 0.0f;
	TestTrue(
		TEXT("Active Ripple은 공통 Launcher lifecycle로 표시"),
		Presenter->ResolveLauncherSequenceDisplay(RippleActiveViewData, SequenceText, SequenceProgress));
	TestEqual(TEXT("Ripple 표시 문구"), SequenceText.ToString(), FString(TEXT("RIPPLE 2 / 4")));
	TestTrue(TEXT("Ripple 진행률 0.5"), FMath::IsNearlyEqual(SequenceProgress, 0.5f));

	// [v1.4.0] Runtime은 terminal이지만 정식 Launcher 이벤트 Revision은 아직 증가하지 않은 Ammo 부수 Refresh입니다.
	FCFWeaponHUDData RipplePreTerminalRefresh = RippleActiveViewData;
	RipplePreTerminalRefresh.LauncherAvailability = ECFUIViewAvailability::KnownZero;
	RipplePreTerminalRefresh.bLauncherSequenceActive = false;
	RipplePreTerminalRefresh.LauncherAcceptedProjectileCount = 4;
	RipplePreTerminalRefresh.LauncherRemainingProjectileCount = 0;
	TestTrue(
		TEXT("Ripple terminal 전 Ammo Refresh는 마지막 Active Presentation 유지"),
		Presenter->ResolveLauncherSequenceDisplay(RipplePreTerminalRefresh, SequenceText, SequenceProgress));
	TestEqual(TEXT("정식 terminal 이벤트 전 Ripple 문구 유지"), SequenceText.ToString(), FString(TEXT("RIPPLE 2 / 4")));

	// [v1.4.0] 실제 Launcher Completed 이벤트가 Revision을 증가시킨 최종 Snapshot입니다.
	FCFWeaponHUDData RippleTerminalViewData = RipplePreTerminalRefresh;
	RippleTerminalViewData.LauncherSequenceRevision = 11;
	TestTrue(
		TEXT("Ripple terminal 이벤트는 최종 Snapshot 1회 표시"),
		Presenter->ResolveLauncherSequenceDisplay(RippleTerminalViewData, SequenceText, SequenceProgress));
	TestEqual(TEXT("Ripple terminal 문구"), SequenceText.ToString(), FString(TEXT("RIPPLE 4 / 4")));
	TestTrue(TEXT("Ripple terminal 진행률 1.0"), FMath::IsNearlyEqual(SequenceProgress, 1.0f));
	TestFalse(
		TEXT("Ripple terminal 다음 ViewData는 Weapon Status로 전환"),
		Presenter->ResolveLauncherSequenceDisplay(RippleTerminalViewData, SequenceText, SequenceProgress));

	// [v1.4.0] 같은 lifecycle을 Salvo에 그대로 적용하고 FirePattern은 문구만 SALVO로 바꿉니다.
	FCFWeaponHUDData SalvoActiveViewData = RippleActiveViewData;
	SalvoActiveViewData.LauncherSequenceRevision = 20;
	SalvoActiveViewData.LauncherPattern = ECFLauncherFirePattern::Salvo;
	SalvoActiveViewData.LauncherAcceptedProjectileCount = 3;
	SalvoActiveViewData.LauncherRemainingProjectileCount = 1;
	TestTrue(
		TEXT("Active Salvo도 Ripple과 동일 lifecycle로 표시"),
		Presenter->ResolveLauncherSequenceDisplay(SalvoActiveViewData, SequenceText, SequenceProgress));
	TestEqual(TEXT("Salvo 표시 문구"), SequenceText.ToString(), FString(TEXT("SALVO 3 / 4")));
	TestTrue(TEXT("Salvo 진행률 0.75"), FMath::IsNearlyEqual(SequenceProgress, 0.75f));

	// [v1.4.0] Salvo도 Revision이 바뀌기 전 부수 Refresh에서는 마지막 Active Presentation을 그대로 유지합니다.
	FCFWeaponHUDData SalvoPreTerminalRefresh = SalvoActiveViewData;
	SalvoPreTerminalRefresh.LauncherAvailability = ECFUIViewAvailability::KnownZero;
	SalvoPreTerminalRefresh.bLauncherSequenceActive = false;
	SalvoPreTerminalRefresh.LauncherAcceptedProjectileCount = 4;
	SalvoPreTerminalRefresh.LauncherRemainingProjectileCount = 0;
	TestTrue(
		TEXT("Salvo terminal 전 Ammo Refresh도 마지막 Active Presentation 유지"),
		Presenter->ResolveLauncherSequenceDisplay(SalvoPreTerminalRefresh, SequenceText, SequenceProgress));
	TestEqual(TEXT("정식 terminal 이벤트 전 Salvo 문구 유지"), SequenceText.ToString(), FString(TEXT("SALVO 3 / 4")));

	FCFWeaponHUDData SalvoTerminalViewData = SalvoPreTerminalRefresh;
	SalvoTerminalViewData.LauncherSequenceRevision = 21;
	TestTrue(
		TEXT("Salvo terminal 이벤트도 공통 최종 Snapshot 1회 표시"),
		Presenter->ResolveLauncherSequenceDisplay(SalvoTerminalViewData, SequenceText, SequenceProgress));
	TestEqual(TEXT("Salvo terminal 문구"), SequenceText.ToString(), FString(TEXT("SALVO 4 / 4")));
	TestTrue(TEXT("Salvo terminal 진행률 1.0"), FMath::IsNearlyEqual(SequenceProgress, 1.0f));
	TestFalse(
		TEXT("Salvo terminal 다음 ViewData도 Weapon Status로 전환"),
		Presenter->ResolveLauncherSequenceDisplay(SalvoTerminalViewData, SequenceText, SequenceProgress));

	// [v1.4.0] HeavyCannon SingleCycle은 Launcher Sequence를 만들지 않고 같은 Weapon Status 경로에서 Cooldown을 표시합니다.
	FCFWeaponHUDData HeavyCannonViewData;
	HeavyCannonViewData.WeaponId = FName(TEXT("HeavyCannon"));
	HeavyCannonViewData.LauncherAvailability = ECFUIViewAvailability::KnownZero;
	HeavyCannonViewData.LauncherPattern = ECFLauncherFirePattern::SingleCycle;
	HeavyCannonViewData.bLauncherSequenceActive = false;
	HeavyCannonViewData.CooldownAvailability = ECFUIViewAvailability::Known;
	HeavyCannonViewData.CooldownDurationSeconds = 2.0f;
	HeavyCannonViewData.RemainingCooldownSeconds = 1.5f;
	TestFalse(
		TEXT("HeavyCannon SingleCycle은 별도 Launcher Presentation 예외를 만들지 않음"),
		Presenter->ResolveLauncherSequenceDisplay(HeavyCannonViewData, SequenceText, SequenceProgress));

	// [v1.4.0] HeavyCannon과 Launcher terminal 이후가 함께 사용하는 공통 Weapon Status 출력입니다.
	FText WeaponStatusText;
	float WeaponStatusProgress = 0.0f;
	TestTrue(
		TEXT("HeavyCannon Cooldown은 공통 Weapon Status로 표시"),
		UCFHUDPresenter::ResolveWeaponStatusPresentation(
			HeavyCannonViewData,
			false,
			WeaponStatusText,
			WeaponStatusProgress));
	TestEqual(TEXT("HeavyCannon Cooldown 문구"), WeaponStatusText.ToString(), FString(TEXT("1.5 s")));
	TestTrue(TEXT("HeavyCannon Cooldown 진행률 0.25"), FMath::IsNearlyEqual(WeaponStatusProgress, 0.25f));

	HeavyCannonViewData.RemainingCooldownSeconds = 0.0f;
	HeavyCannonViewData.CooldownAvailability = ECFUIViewAvailability::KnownZero;
	TestTrue(
		TEXT("HeavyCannon Cooldown 종료는 같은 Weapon Status에서 READY"),
		UCFHUDPresenter::ResolveWeaponStatusPresentation(
			HeavyCannonViewData,
			false,
			WeaponStatusText,
			WeaponStatusProgress));
		TestEqual(TEXT("HeavyCannon READY 문구"), WeaponStatusText.ToString(), FString(TEXT("READY")));
	TestTrue(TEXT("HeavyCannon READY 진행률 1.0"), FMath::IsNearlyEqual(WeaponStatusProgress, 1.0f));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP007TargetIdentityFailClosedTest,
	"CarFight.UI.UI_P0_07.TargetIdentityFailClosedContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.31.0] VehiclePawn Target Identity가 VehicleData PrimaryAssetId를 사용하고 Actor instance 이름과 Player-facing 이름 fallback을 노출하지 않는지 검증합니다.
bool FCFHUDP007TargetIdentityFailClosedTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.31.0] Content Asset 없이 실제 VehiclePawn TargetSelectable override를 검증할 transient Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-07 Identity Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.31.0] Target Identity로 절대 사용되면 안 되는 결정적 내부 Actor instance 이름을 지정할 Spawn 설정입니다.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = TEXT("InternalVehicleActorName");

	// [v1.31.0] 처음에는 VehicleData가 없어 TargetId가 fail-closed여야 하는 실제 기본 VehiclePawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		SpawnParameters);
	if (!TestNotNull(TEXT("UI-P0-07 Identity Vehicle Pawn"), VehiclePawn))
	{
		return false;
	}

		// [v1.32.0] Production TargetCandidate resolver의 native interface branch와 동일하게 사용할 차량 TargetSelectable 구현입니다.
	const ICFTargetSelectable* NativeTargetSelectable = Cast<ICFTargetSelectable>(VehiclePawn);
	if (!TestNotNull(TEXT("UI-P0-07 native Vehicle TargetSelectable"), NativeTargetSelectable))
	{
		VehiclePawn->Destroy();
		return false;
	}

	// [v1.32.0] VehicleData 미지정 상태에서 native Production resolver 경로가 반환한 fail-closed 표시 정보입니다.
	const FCFTargetDisplayInfo MissingVehicleDataDisplayInfo = NativeTargetSelectable->GetTargetDisplayInfo_Implementation();
	TestEqual(TEXT("UI-P0-07 VehicleData 없음 TargetId fail-closed"), MissingVehicleDataDisplayInfo.TargetId, NAME_None);
	TestTrue(TEXT("UI-P0-07 VehicleData 없음 DisplayName fail-closed"), MissingVehicleDataDisplayInfo.DisplayName.IsEmpty());

	// [v1.31.0] 실제 Content Asset을 만들지 않고 안정 PrimaryAssetId source를 제공할 transient VehicleData입니다.
	UCFVehicleData* StableVehicleData = NewObject<UCFVehicleData>(GetTransientPackage(), TEXT("DA_AutomationStableVehicle"));
	if (!TestNotNull(TEXT("UI-P0-07 transient VehicleData"), StableVehicleData))
	{
		VehiclePawn->Destroy();
		return false;
	}
	VehiclePawn->VehicleData = StableVehicleData;

	// [v1.31.0] Pawn instance 수명/이름과 독립된 VehicleData의 안정 Primary Asset 식별자입니다.
	const FPrimaryAssetId StableVehiclePrimaryAssetId = StableVehicleData->GetPrimaryAssetId();
	if (!TestTrue(TEXT("UI-P0-07 VehicleData PrimaryAssetId valid"), StableVehiclePrimaryAssetId.IsValid()))
	{
		VehiclePawn->Destroy();
		return false;
	}

		// [v1.32.0] VehicleData 지정 뒤 native Production resolver 경로가 반환한 안정 Target Identity 표시 정보입니다.
	const FCFTargetDisplayInfo StableDisplayInfo = NativeTargetSelectable->GetTargetDisplayInfo_Implementation();
	TestEqual(TEXT("UI-P0-07 TargetId는 VehicleData PrimaryAssetName"), StableDisplayInfo.TargetId, StableVehiclePrimaryAssetId.PrimaryAssetName);
	TestNotEqual(TEXT("UI-P0-07 TargetId는 Actor instance 이름이 아님"), StableDisplayInfo.TargetId, VehiclePawn->GetFName());
	TestTrue(TEXT("UI-P0-07 Player-facing DisplayName은 명시 source 전까지 Empty"), StableDisplayInfo.DisplayName.IsEmpty());
	TestEqual(TEXT("UI-P0-07 Vehicle Category 유지"), StableDisplayInfo.TargetCategory, ECFTargetCategory::Vehicle);
	TestEqual(TEXT("UI-P0-07 Identified source 의미 유지"), StableDisplayInfo.InformationLevel, ECFTargetInfoLevel::Identified);

	// [v1.31.0] Identified Sensor 공개 계약이 새 안정 TargetId를 정상 수용하는지 검증할 Actor-free Contact입니다.
	FCFSensorContact IdentifiedSensorContact;
	IdentifiedSensorContact.ContactId = TEXT("Contact_IdentityContract");
	IdentifiedSensorContact.KnownTargetId = StableDisplayInfo.TargetId;
	IdentifiedSensorContact.InformationLevel = ECFTargetInfoLevel::Identified;
	IdentifiedSensorContact.ContactState = ECFSensorContactState::Live;
	TestTrue(TEXT("UI-P0-07 안정 TargetId는 Identified Sensor public contract 유효"), IdentifiedSensorContact.IsPublicContractValid());

	VehiclePawn->Destroy();
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP007TargetKnowledgePanelTest,
	"CarFight.UI.UI_P0_07.TargetKnowledgePanelContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.23.0] 저장 Production TargetPanel이 TargetSelect 선택 상태와 Sensor Snapshot Knowledge를 ViewData 경계에서만 소비하는지 검증합니다.
bool FCFHUDP007TargetKnowledgePanelTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.23.0] 저장 Production HUD를 실제 인스턴스화해 TargetPanel Presenter 적용을 확인할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-07 Target Knowledge Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.23.0] DefaultGame.ini와 동일한 저장 Production HUD Generated Class입니다.
	UClass* ProductionHUDClass = LoadClass<UCFStyledWidgetBase>(
		nullptr,
		TEXT("/Game/CarFight/UI/HUD/WBP_CFInGameHUD.WBP_CFInGameHUD_C"));
	if (!TestNotNull(TEXT("UI-P0-07 Production HUD Class"), ProductionHUDClass))
	{
		return false;
	}

	// [v1.23.0] 저장 Production HUD Class로 만든 테스트 전용 Widget 인스턴스입니다.
	UCFStyledWidgetBase* ProductionHUDWidget = CreateWidget<UCFStyledWidgetBase>(TestWorld, ProductionHUDClass);
	if (!TestNotNull(TEXT("UI-P0-07 Production HUD Widget"), ProductionHUDWidget))
	{
		return false;
	}

	// [v1.23.0] 저장 Production Root 아래 Target Knowledge 의미 Widget입니다.
	UUserWidget* TargetPanelWidget = Cast<UUserWidget>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("WBP_CFTargetPanel"))));
	if (!TestNotNull(TEXT("UI-P0-07 TargetPanel"), TargetPanelWidget))
	{
		return false;
	}

	// [v1.23.0] 선택 없음/Unknown/Identified 상태를 표시할 Target 제목입니다.
	UTextBlock* TargetTitleText = Cast<UTextBlock>(TargetPanelWidget->GetWidgetFromName(FName(TEXT("Text_TargetTitle"))));
	// [v1.23.0] Sensor Snapshot 기반 실제 거리 표시입니다.
	UTextBlock* TargetDistanceText = Cast<UTextBlock>(TargetPanelWidget->GetWidgetFromName(FName(TEXT("Text_TargetDistance"))));
	// [v1.23.0] Sensor Knowledge Identity 공개 상태 표시입니다.
	UTextBlock* TargetIdentityText = Cast<UTextBlock>(TargetPanelWidget->GetWidgetFromName(FName(TEXT("Text_TargetIdentity"))));
	// [v1.23.0] 아직 authoritative Target Armor source가 없어 숨김을 유지할 Mock Row입니다.
	UTextBlock* TargetArmorText = Cast<UTextBlock>(TargetPanelWidget->GetWidgetFromName(FName(TEXT("Text_TargetArmor"))));
	// [v1.23.0] Sensor Analysis 진행률의 Player-facing Text입니다.
	UTextBlock* TargetScanText = Cast<UTextBlock>(TargetPanelWidget->GetWidgetFromName(FName(TEXT("Text_TargetScan"))));
	// [v1.23.0] Sensor Analysis 진행률의 0~1 Bar입니다.
	UProgressBar* TargetScanProgressBar = Cast<UProgressBar>(TargetPanelWidget->GetWidgetFromName(FName(TEXT("ProgressBar_TargetScan"))));
	if (!TestNotNull(TEXT("UI-P0-07 Target Title"), TargetTitleText)
		|| !TestNotNull(TEXT("UI-P0-07 Target Distance"), TargetDistanceText)
		|| !TestNotNull(TEXT("UI-P0-07 Target Identity"), TargetIdentityText)
		|| !TestNotNull(TEXT("UI-P0-07 Target Armor"), TargetArmorText)
		|| !TestNotNull(TEXT("UI-P0-07 Target Scan Text"), TargetScanText)
		|| !TestNotNull(TEXT("UI-P0-07 Target Scan Progress"), TargetScanProgressBar))
	{
		return false;
	}

	// [v1.23.0] synthetic Target ViewData를 실제 Production Widget에 적용할 Presenter입니다.
	UCFHUDPresenter* Presenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-07 Presenter"), Presenter))
	{
		return false;
	}
	Presenter->SetProductionWidget(ProductionHUDWidget);

	// [v1.23.0] private UFUNCTION HandleHUDViewDataChanged를 실제 Provider delegate 경로와 동일하게 호출할 Reflection 함수입니다.
	UFunction* HandleViewDataFunction = Presenter->FindFunction(FName(TEXT("HandleHUDViewDataChanged")));
	if (!TestNotNull(TEXT("UI-P0-07 HandleHUDViewDataChanged"), HandleViewDataFunction))
	{
		return false;
	}

	// [v1.23.0] Reflection UFUNCTION의 단일 ViewData 인자를 전달할 파라미터 구조입니다.
	struct FHandleHUDViewDataChangedParams
	{
		// [v1.23.0] Presenter에 적용할 전체 HUD ViewData입니다.
		FCFInGameUIViewData ViewData;
	};

	// [v1.23.0] Target 관련 ViewData를 Production Presenter에 정확히 한 번 적용하는 Helper입니다.
	auto ApplyTargetViewDataOnce = [Presenter, HandleViewDataFunction](const FCFTargetHUDData& TargetViewData)
	{
		// [v1.23.0] 이번 한 번의 Presenter 적용에 사용할 전체 ViewData 파라미터입니다.
		FHandleHUDViewDataChangedParams Params;
		Params.ViewData.Target = TargetViewData;
		Presenter->ProcessEvent(HandleViewDataFunction, &Params);
	};

	// [v1.23.0] 선택 Target이 없는 기본 상태입니다.
	FCFTargetHUDData NoTargetViewData;
	NoTargetViewData.Availability = ECFUIViewAvailability::KnownZero;
	ApplyTargetViewDataOnce(NoTargetViewData);
	TestEqual(TEXT("UI-P0-07 no target title"), TargetTitleText->GetText().ToString(), FString(TEXT("NO TARGET")));
	TestEqual(TEXT("UI-P0-07 no target distance collapsed"), TargetDistanceText->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-07 no target identity collapsed"), TargetIdentityText->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-07 no target armor collapsed"), TargetArmorText->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-07 no target scan text collapsed"), TargetScanText->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-07 no target scan bar collapsed"), TargetScanProgressBar->GetVisibility(), ESlateVisibility::Collapsed);

	// [v1.23.0] Sensor가 Contact와 거리/분석 진행률은 알고 있지만 Identity는 아직 공개하지 않은 Detected 상태입니다.
	FCFTargetHUDData DetectedTargetViewData;
	DetectedTargetViewData.Availability = ECFUIViewAvailability::Known;
	DetectedTargetViewData.bHasSelectedTarget = true;
	DetectedTargetViewData.bSelectedTargetValid = true;
	DetectedTargetViewData.SensorContactAvailability = ECFUIViewAvailability::Known;
	DetectedTargetViewData.IdentityAvailability = ECFUIViewAvailability::Unknown;
	DetectedTargetViewData.InformationLevel = ECFTargetInfoLevel::Detected;
	DetectedTargetViewData.DistanceAvailability = ECFUIViewAvailability::Known;
	DetectedTargetViewData.DistanceMeters = 842.0f;
	DetectedTargetViewData.AnalysisProgress01 = 0.35f;
	ApplyTargetViewDataOnce(DetectedTargetViewData);
	TestEqual(TEXT("UI-P0-07 detected unknown title"), TargetTitleText->GetText().ToString(), FString(TEXT("UNKNOWN CONTACT")));
	TestEqual(TEXT("UI-P0-07 detected identity unknown"), TargetIdentityText->GetText().ToString(), FString(TEXT("식별  ???")));
	TestEqual(TEXT("UI-P0-07 detected distance"), TargetDistanceText->GetText().ToString(), FString(TEXT("거리  842 m")));
	TestEqual(TEXT("UI-P0-07 detected scan text"), TargetScanText->GetText().ToString(), FString(TEXT("스캔  35%")));
	TestTrue(TEXT("UI-P0-07 detected scan progress 0.35"), FMath::IsNearlyEqual(TargetScanProgressBar->GetPercent(), 0.35f));
	TestEqual(TEXT("UI-P0-07 detected distance visible"), TargetDistanceText->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-07 detected identity visible"), TargetIdentityText->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-07 detected scan visible"), TargetScanText->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-07 target armor remains collapsed"), TargetArmorText->GetVisibility(), ESlateVisibility::Collapsed);

	// [v1.23.0] Sensor Knowledge가 Identified로 승격해 Player-facing DisplayName을 공개한 상태입니다.
	FCFTargetHUDData IdentifiedTargetViewData = DetectedTargetViewData;
	IdentifiedTargetViewData.IdentityAvailability = ECFUIViewAvailability::Known;
	IdentifiedTargetViewData.DisplayName = FText::FromString(TEXT("적대 차량"));
	IdentifiedTargetViewData.InformationLevel = ECFTargetInfoLevel::Identified;
	IdentifiedTargetViewData.AnalysisProgress01 = 0.60f;
	ApplyTargetViewDataOnce(IdentifiedTargetViewData);
	TestEqual(TEXT("UI-P0-07 identified title"), TargetTitleText->GetText().ToString(), FString(TEXT("적대 차량")));
	TestEqual(TEXT("UI-P0-07 identified identity"), TargetIdentityText->GetText().ToString(), FString(TEXT("식별  적대 차량")));
	TestEqual(TEXT("UI-P0-07 identified scan text"), TargetScanText->GetText().ToString(), FString(TEXT("스캔  60%")));
	TestTrue(TEXT("UI-P0-07 identified scan progress 0.60"), FMath::IsNearlyEqual(TargetScanProgressBar->GetPercent(), 0.60f));

	// [v1.23.0] 선택 기록은 남아 있어도 현재 Sensor Contact가 Unknown이면 stale 거리/분석 숫자를 노출하지 않는 fail-closed 상태입니다.
	FCFTargetHUDData UnknownContactViewData = DetectedTargetViewData;
	UnknownContactViewData.SensorContactAvailability = ECFUIViewAvailability::Unknown;
	UnknownContactViewData.DistanceMeters = 999.0f;
	UnknownContactViewData.AnalysisProgress01 = 0.95f;
	ApplyTargetViewDataOnce(UnknownContactViewData);
	TestEqual(TEXT("UI-P0-07 unknown contact title"), TargetTitleText->GetText().ToString(), FString(TEXT("UNKNOWN CONTACT")));
	TestEqual(TEXT("UI-P0-07 unknown contact distance collapsed"), TargetDistanceText->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-07 unknown contact scan text collapsed"), TargetScanText->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-07 unknown contact scan bar collapsed"), TargetScanProgressBar->GetVisibility(), ESlateVisibility::Collapsed);

		ApplyTargetViewDataOnce(NoTargetViewData);
	TestEqual(TEXT("UI-P0-07 clear returns NO TARGET"), TargetTitleText->GetText().ToString(), FString(TEXT("NO TARGET")));
	TestEqual(TEXT("UI-P0-07 clear distance collapsed"), TargetDistanceText->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-07 clear identity collapsed"), TargetIdentityText->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-07 clear scan collapsed"), TargetScanText->GetVisibility(), ESlateVisibility::Collapsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP008RadarContactConsumerTest,
	"CarFight.UI.UI_P0_08.RadarContactConsumerContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.25.0] 저장 Production RadarPanel이 전용 Image Template와 FCFRadarHUDData만으로 runtime Contact·Range·selected edge를 확장·배치하는지 검증합니다.
bool FCFHUDP008RadarContactConsumerTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.25.0] 저장 Production HUD 인스턴스에 synthetic Radar ViewData를 적용할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-08B Radar Consumer Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.25.0] DefaultGame.ini와 동일한 저장 Production HUD Generated Class입니다.
	UClass* ProductionHUDClass = LoadClass<UCFStyledWidgetBase>(
		nullptr,
		TEXT("/Game/CarFight/UI/HUD/WBP_CFInGameHUD.WBP_CFInGameHUD_C"));
	if (!TestNotNull(TEXT("UI-P0-08B Production HUD Class"), ProductionHUDClass))
	{
		return false;
	}

	// [v1.25.0] 실제 저장 RadarPanel Tree를 포함하는 transient Production HUD Widget 인스턴스입니다.
	UCFStyledWidgetBase* ProductionHUDWidget = CreateWidget<UCFStyledWidgetBase>(TestWorld, ProductionHUDClass);
	if (!TestNotNull(TEXT("UI-P0-08B Production HUD Widget"), ProductionHUDWidget))
	{
		return false;
	}

	// [v1.25.0] 저장 Production Root 아래 Radar 의미 Widget입니다.
	UUserWidget* RadarPanelWidget = Cast<UUserWidget>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("WBP_CFRadarPanel"))));
	if (!TestNotNull(TEXT("UI-P0-08B RadarPanel"), RadarPanelWidget))
	{
		return false;
	}

	// [v1.25.0] Designer가 크기·위치를 소유하고 runtime Contact만 추가하는 Radar 공간 Canvas입니다.
	UCanvasPanel* RadarCanvas = Cast<UCanvasPanel>(RadarPanelWidget->GetWidgetFromName(FName(TEXT("CanvasPanel_RadarContacts"))));
	// [v1.25.0] Radar Grid·3 Ring·Frame을 표시할 저장 전용 Image입니다.
	UImage* RadarFrameImage = Cast<UImage>(RadarPanelWidget->GetWidgetFromName(FName(TEXT("Image_RadarFrame"))));
	// [v1.25.0] Radar 중앙 Player 상향 삼각형 전용 Image입니다.
	UImage* RadarPlayerImage = Cast<UImage>(RadarPanelWidget->GetWidgetFromName(FName(TEXT("Image_RadarPlayer"))));
	// [v1.25.0] in-range 선택 Contact에 사용할 저장 4-Corner Bracket Image입니다.
	UImage* SelectedBracketImage = Cast<UImage>(RadarPanelWidget->GetWidgetFromName(FName(TEXT("Image_RadarSelected"))));
	// [v1.25.0] range-out 선택 Contact에 사용할 저장 2-Corner Edge Bracket Image입니다.
	UImage* SelectedEdgeImage = Cast<UImage>(RadarPanelWidget->GetWidgetFromName(FName(TEXT("Image_RadarSelectedEdge"))));
	// [v1.25.0] 현재 단계식 Display Range를 표시할 저장 Text 의미 슬롯입니다.
	UTextBlock* RadarRangeText = Cast<UTextBlock>(RadarPanelWidget->GetWidgetFromName(FName(TEXT("Text_RadarRange"))));
	if (!TestNotNull(TEXT("UI-P0-08B Radar Canvas"), RadarCanvas)
		|| !TestNotNull(TEXT("UI-P0-08B Radar Frame"), RadarFrameImage)
		|| !TestNotNull(TEXT("UI-P0-08B Radar Player"), RadarPlayerImage)
		|| !TestNotNull(TEXT("UI-P0-08B Selected Bracket"), SelectedBracketImage)
		|| !TestNotNull(TEXT("UI-P0-08B Selected Edge"), SelectedEdgeImage)
		|| !TestNotNull(TEXT("UI-P0-08B Radar Range Text"), RadarRangeText))
	{
		return false;
	}
	TestNotNull(TEXT("UI-P0-08B Radar Frame dedicated Brush"), RadarFrameImage->GetBrush().GetResourceObject());
	TestNotNull(TEXT("UI-P0-08B Radar Player dedicated Brush"), RadarPlayerImage->GetBrush().GetResourceObject());
	TestNotNull(TEXT("UI-P0-08B Selected 4-corner dedicated Brush"), SelectedBracketImage->GetBrush().GetResourceObject());
	TestNotNull(TEXT("UI-P0-08B Selected Edge dedicated Brush"), SelectedEdgeImage->GetBrush().GetResourceObject());

	// [v1.25.0] Friendly runtime Image의 Brush/색/크기를 소유하는 숨겨진 저장 Template입니다.
	UImage* FriendlyTemplate = Cast<UImage>(RadarPanelWidget->GetWidgetFromName(FName(TEXT("Image_RadarFriendly"))));
	// [v1.25.0] Neutral runtime Image의 Unknown diamond Brush와 Neutral 색/크기를 소유하는 숨겨진 저장 Template입니다.
	UImage* NeutralTemplate = Cast<UImage>(RadarPanelWidget->GetWidgetFromName(FName(TEXT("Image_RadarNeutral"))));
	// [v1.25.0] Hostile runtime Image의 Brush/색/크기를 소유하는 숨겨진 저장 Template입니다.
	UImage* HostileTemplate = Cast<UImage>(RadarPanelWidget->GetWidgetFromName(FName(TEXT("Image_RadarHostile"))));
	// [v1.25.0] Unknown runtime Image의 Brush/색/크기를 소유하는 숨겨진 저장 Template입니다.
	UImage* UnknownTemplate = Cast<UImage>(RadarPanelWidget->GetWidgetFromName(FName(TEXT("Image_RadarUnknown"))));
	if (!TestNotNull(TEXT("UI-P0-08B Friendly Template"), FriendlyTemplate)
		|| !TestNotNull(TEXT("UI-P0-08B Neutral Template"), NeutralTemplate)
		|| !TestNotNull(TEXT("UI-P0-08B Hostile Template"), HostileTemplate)
		|| !TestNotNull(TEXT("UI-P0-08B Unknown Template"), UnknownTemplate))
	{
		return false;
	}
	TestNotNull(TEXT("UI-P0-08B Friendly dedicated Brush"), FriendlyTemplate->GetBrush().GetResourceObject());
	TestNotNull(TEXT("UI-P0-08B Neutral dedicated Brush"), NeutralTemplate->GetBrush().GetResourceObject());
	TestNotNull(TEXT("UI-P0-08B Hostile dedicated Brush"), HostileTemplate->GetBrush().GetResourceObject());
	TestNotNull(TEXT("UI-P0-08B Unknown dedicated Brush"), UnknownTemplate->GetBrush().GetResourceObject());

	// [v1.25.0] synthetic Radar ViewData를 실제 Production Widget에 적용할 Presenter입니다.
	UCFHUDPresenter* Presenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-08B Presenter"), Presenter))
	{
		return false;
	}
	Presenter->SetProductionWidget(ProductionHUDWidget);

	// [v1.25.0] private UFUNCTION HandleHUDViewDataChanged를 실제 Provider delegate와 같은 경로로 호출할 Reflection 함수입니다.
	UFunction* HandleViewDataFunction = Presenter->FindFunction(FName(TEXT("HandleHUDViewDataChanged")));
	if (!TestNotNull(TEXT("UI-P0-08B HandleHUDViewDataChanged"), HandleViewDataFunction))
	{
		return false;
	}

	// [v1.25.0] Reflection UFUNCTION의 단일 ViewData 인자를 전달할 파라미터 구조입니다.
	struct FHandleHUDViewDataChangedParams
	{
		// [v1.25.0] Presenter에 적용할 전체 HUD ViewData입니다.
		FCFInGameUIViewData ViewData;
	};

	// [v1.25.0] Radar 관련 ViewData를 Production Presenter에 정확히 한 번 적용하는 Helper입니다.
	auto ApplyRadarViewDataOnce = [Presenter, HandleViewDataFunction](const FCFRadarHUDData& RadarViewData)
	{
		// [v1.25.0] 이번 한 번의 Presenter 적용에 사용할 전체 ViewData 파라미터입니다.
		FHandleHUDViewDataChangedParams Params;
		Params.ViewData.Radar = RadarViewData;
		Presenter->ProcessEvent(HandleViewDataFunction, &Params);
	};

	// [v1.25.0] runtime pool 이름으로 현재 Canvas에 존재하는 Image Blip을 찾는 Helper입니다.
	auto FindRuntimeBlip = [RadarCanvas](const int32 PoolIndex) -> UImage*
	{
		// [v1.25.0] Presenter와 동일한 안정 runtime Image Blip 이름입니다.
		const FName RuntimeBlipName(*FString::Printf(TEXT("Image_RadarRuntimeContact_%03d"), PoolIndex));
		for (int32 ChildIndex = 0; ChildIndex < RadarCanvas->GetChildrenCount(); ++ChildIndex)
		{
			// [v1.25.0] 현재 Radar Canvas의 한 runtime/static Image 자식입니다.
			UImage* RuntimeBlip = Cast<UImage>(RadarCanvas->GetChildAt(ChildIndex));
			if (RuntimeBlip && RuntimeBlip->GetFName() == RuntimeBlipName)
			{
				return RuntimeBlip;
			}
		}
		return nullptr;
	};

	// [v1.25.0] 5개 Preview보다 많은 실제 Contact도 고정 상한 없이 표시하는 첫 Radar ViewData입니다.
	FCFRadarHUDData InRangeRadarViewData;
	InRangeRadarViewData.Availability = ECFUIViewAvailability::Known;
	InRangeRadarViewData.DisplayRangeAvailability = ECFUIViewAvailability::Known;
	InRangeRadarViewData.DisplayRangeMeters = 50.0f;
	InRangeRadarViewData.MaximumDetectionRangeAvailability = ECFUIViewAvailability::Known;
	InRangeRadarViewData.MaximumDetectionRangeMeters = 100.0f;

	// [v1.25.0] Forward +0.6 / Right +0.2 위치의 Friendly Contact입니다.
	FCFRadarContactHUDData FriendlyContact;
	FriendlyContact.Relation = ECFTargetRelation::Friendly;
	FriendlyContact.NormalizedPositionAvailability = ECFUIViewAvailability::Known;
	FriendlyContact.NormalizedPosition = FVector2D(0.6f, 0.2f);
	FriendlyContact.bInsideDisplayRange = true;
	InRangeRadarViewData.Contacts.Add(FriendlyContact);

	// [v1.25.0] Forward -0.2 / Right +0.5 위치의 선택 Hostile Contact입니다.
	FCFRadarContactHUDData SelectedHostileContact;
	SelectedHostileContact.Relation = ECFTargetRelation::Hostile;
	SelectedHostileContact.NormalizedPositionAvailability = ECFUIViewAvailability::Known;
	SelectedHostileContact.NormalizedPosition = FVector2D(-0.2f, 0.5f);
	SelectedHostileContact.bInsideDisplayRange = true;
	SelectedHostileContact.bSelected = true;
	InRangeRadarViewData.Contacts.Add(SelectedHostileContact);

	// [v1.25.0] Neutral이 Unknown diamond Texture를 관계색만 바꿔 재사용하는 in-range Contact입니다.
	FCFRadarContactHUDData NeutralContact;
	NeutralContact.Relation = ECFTargetRelation::Neutral;
	NeutralContact.NormalizedPositionAvailability = ECFUIViewAvailability::Known;
	NeutralContact.NormalizedPosition = FVector2D(0.1f, -0.3f);
	NeutralContact.bInsideDisplayRange = true;
	InRangeRadarViewData.Contacts.Add(NeutralContact);

	for (int32 ExtraContactIndex = 0; ExtraContactIndex < 4; ++ExtraContactIndex)
	{
		// [v1.25.0] 고정 Preview 5개 상한을 넘기기 위해 추가하는 실제 synthetic Unknown Contact입니다.
		FCFRadarContactHUDData ExtraContact;
		ExtraContact.Relation = ECFTargetRelation::Unknown;
		ExtraContact.NormalizedPositionAvailability = ECFUIViewAvailability::Known;
		ExtraContact.NormalizedPosition = FVector2D(-0.30f + (0.12f * ExtraContactIndex), -0.20f + (0.08f * ExtraContactIndex));
		ExtraContact.bInsideDisplayRange = true;
		InRangeRadarViewData.Contacts.Add(ExtraContact);
	}

	ApplyRadarViewDataOnce(InRangeRadarViewData);
	TestEqual(TEXT("UI-P0-08B Radar Canvas visible"), RadarCanvas->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-08B Radar Frame visible"), RadarFrameImage->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-08B Radar Player visible"), RadarPlayerImage->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-08B Range text"), RadarRangeText->GetText().ToString(), FString(TEXT("RANGE 50 m")));
	TestEqual(TEXT("UI-P0-08B Range text visible"), RadarRangeText->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-08B static Friendly preview hidden"), FriendlyTemplate->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-08B static Neutral preview hidden"), NeutralTemplate->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-08B static Hostile preview hidden"), HostileTemplate->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-08B static Unknown preview hidden"), UnknownTemplate->GetVisibility(), ESlateVisibility::Collapsed);
	TestNull(TEXT("UI-P0-08B legacy Text glyph runtime Blip 없음"), RadarPanelWidget->GetWidgetFromName(FName(TEXT("Text_RadarRuntimeContact_000"))));

	for (int32 ExpectedBlipIndex = 0; ExpectedBlipIndex < 7; ++ExpectedBlipIndex)
	{
		// [v1.25.0] 이번 ViewData의 각 in-range Contact에 대응해야 하는 runtime pooled Image입니다.
		UImage* RuntimeBlip = FindRuntimeBlip(ExpectedBlipIndex);
		TestNotNull(*FString::Printf(TEXT("UI-P0-08B runtime Image Blip %d 존재"), ExpectedBlipIndex), RuntimeBlip);
		if (RuntimeBlip)
		{
			TestEqual(*FString::Printf(TEXT("UI-P0-08B runtime Image Blip %d visible"), ExpectedBlipIndex), RuntimeBlip->GetVisibility(), ESlateVisibility::HitTestInvisible);
			TestNotNull(*FString::Printf(TEXT("UI-P0-08B runtime Image Blip %d Brush"), ExpectedBlipIndex), RuntimeBlip->GetBrush().GetResourceObject());
		}
	}

	// [v1.25.0] 관계별 Brush Template를 확인할 첫 세 runtime Image Blip입니다.
	UImage* FriendlyBlip = FindRuntimeBlip(0);
	UImage* HostileBlip = FindRuntimeBlip(1);
	UImage* NeutralBlip = FindRuntimeBlip(2);
	if (!TestNotNull(TEXT("UI-P0-08B Friendly Blip"), FriendlyBlip)
		|| !TestNotNull(TEXT("UI-P0-08B Hostile Blip"), HostileBlip)
		|| !TestNotNull(TEXT("UI-P0-08B Neutral Blip"), NeutralBlip))
	{
		return false;
	}
	TestTrue(TEXT("UI-P0-08B Friendly Brush Template 일치"), FriendlyBlip->GetBrush().GetResourceObject() == FriendlyTemplate->GetBrush().GetResourceObject());
	TestTrue(TEXT("UI-P0-08B Hostile Brush Template 일치"), HostileBlip->GetBrush().GetResourceObject() == HostileTemplate->GetBrush().GetResourceObject());
	TestTrue(TEXT("UI-P0-08B Neutral diamond Brush Template 일치"), NeutralBlip->GetBrush().GetResourceObject() == NeutralTemplate->GetBrush().GetResourceObject());
	TestTrue(TEXT("UI-P0-08B Friendly relation color 일치"), FriendlyBlip->GetColorAndOpacity().Equals(FriendlyTemplate->GetColorAndOpacity()));
	TestTrue(TEXT("UI-P0-08B Hostile relation color 일치"), HostileBlip->GetColorAndOpacity().Equals(HostileTemplate->GetColorAndOpacity()));
	TestTrue(TEXT("UI-P0-08B Neutral relation color 일치"), NeutralBlip->GetColorAndOpacity().Equals(NeutralTemplate->GetColorAndOpacity()));

	// [v1.25.0] Friendly normalized Forward/Right를 Canvas 0~1 Anchor로 변환한 실제 Slot입니다.
	UCanvasPanelSlot* FriendlyBlipSlot = Cast<UCanvasPanelSlot>(FriendlyBlip->Slot);
	// [v1.25.0] 선택 Hostile normalized 위치를 같은 방식으로 변환한 실제 Slot입니다.
	UCanvasPanelSlot* HostileBlipSlot = Cast<UCanvasPanelSlot>(HostileBlip->Slot);
	// [v1.25.0] Friendly runtime 크기의 Designer owner인 Template Slot입니다.
	UCanvasPanelSlot* FriendlyTemplateSlot = Cast<UCanvasPanelSlot>(FriendlyTemplate->Slot);
	// [v1.25.0] 기존 Designer 4-Corner Bracket의 runtime 위치 Slot입니다.
	UCanvasPanelSlot* SelectedBracketSlot = Cast<UCanvasPanelSlot>(SelectedBracketImage->Slot);
	if (!TestNotNull(TEXT("UI-P0-08B Friendly Canvas Slot"), FriendlyBlipSlot)
		|| !TestNotNull(TEXT("UI-P0-08B Hostile Canvas Slot"), HostileBlipSlot)
		|| !TestNotNull(TEXT("UI-P0-08B Friendly Template Slot"), FriendlyTemplateSlot)
		|| !TestNotNull(TEXT("UI-P0-08B Selected Canvas Slot"), SelectedBracketSlot))
	{
		return false;
	}
	TestTrue(TEXT("UI-P0-08B Friendly anchor X 0.60"), FMath::IsNearlyEqual(FriendlyBlipSlot->GetAnchors().Minimum.X, 0.60f, 0.001f));
	TestTrue(TEXT("UI-P0-08B Friendly anchor Y 0.20"), FMath::IsNearlyEqual(FriendlyBlipSlot->GetAnchors().Minimum.Y, 0.20f, 0.001f));
	TestTrue(TEXT("UI-P0-08B Hostile anchor X 0.75"), FMath::IsNearlyEqual(HostileBlipSlot->GetAnchors().Minimum.X, 0.75f, 0.001f));
	TestTrue(TEXT("UI-P0-08B Hostile anchor Y 0.60"), FMath::IsNearlyEqual(HostileBlipSlot->GetAnchors().Minimum.Y, 0.60f, 0.001f));
	TestTrue(TEXT("UI-P0-08B runtime Blip 크기는 Designer Template 소유"), FriendlyBlipSlot->GetSize().Equals(FriendlyTemplateSlot->GetSize(), 0.001f));
	TestTrue(TEXT("UI-P0-08B selected bracket follows hostile X"), FMath::IsNearlyEqual(SelectedBracketSlot->GetAnchors().Minimum.X, 0.75f, 0.001f));
	TestTrue(TEXT("UI-P0-08B selected bracket follows hostile Y"), FMath::IsNearlyEqual(SelectedBracketSlot->GetAnchors().Minimum.Y, 0.60f, 0.001f));
	TestEqual(TEXT("UI-P0-08B selected in-range bracket visible"), SelectedBracketImage->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-08B selected in-range edge hidden"), SelectedEdgeImage->GetVisibility(), ESlateVisibility::Collapsed);

	// [v1.25.0] 선택 Contact가 표시 범위 밖으로 나갔지만 08A가 Edge Direction을 제공하는 상태입니다.
	FCFRadarHUDData RangeOutRadarViewData = InRangeRadarViewData;
	RangeOutRadarViewData.Contacts.Reset();
	// [v1.25.0] 승인된 2-Corner Edge Bracket이 필요한 range-out selected Contact입니다.
	FCFRadarContactHUDData RangeOutSelectedContact;
	RangeOutSelectedContact.Relation = ECFTargetRelation::Hostile;
	RangeOutSelectedContact.NormalizedPositionAvailability = ECFUIViewAvailability::Known;
	RangeOutSelectedContact.NormalizedPosition = FVector2D(0.948683f, 0.316228f);
	RangeOutSelectedContact.bInsideDisplayRange = false;
	RangeOutSelectedContact.bSelected = true;
	RangeOutSelectedContact.bShowSelectedEdgeMarker = true;
	RangeOutSelectedContact.SelectedEdgeDirection = RangeOutSelectedContact.NormalizedPosition;
	RangeOutRadarViewData.Contacts.Add(RangeOutSelectedContact);
	ApplyRadarViewDataOnce(RangeOutRadarViewData);
	TestEqual(TEXT("UI-P0-08B range-out ordinary Blip hidden"), FriendlyBlip->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-08B range-out에 4-corner bracket 오용 금지"), SelectedBracketImage->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-08B range-out 2-corner edge visible"), SelectedEdgeImage->GetVisibility(), ESlateVisibility::HitTestInvisible);

	// [v1.25.0] selected edge direction을 Radar circumference Anchor로 변환한 실제 Slot입니다.
	UCanvasPanelSlot* SelectedEdgeSlot = Cast<UCanvasPanelSlot>(SelectedEdgeImage->Slot);
	if (!TestNotNull(TEXT("UI-P0-08B Selected Edge Canvas Slot"), SelectedEdgeSlot))
	{
		return false;
	}
	TestTrue(TEXT("UI-P0-08B selected edge anchor X"), FMath::IsNearlyEqual(SelectedEdgeSlot->GetAnchors().Minimum.X, 0.658114f, 0.001f));
	TestTrue(TEXT("UI-P0-08B selected edge anchor Y"), FMath::IsNearlyEqual(SelectedEdgeSlot->GetAnchors().Minimum.Y, 0.025659f, 0.001f));

	// [v1.25.0] Sensor Runtime은 준비됐지만 Contact가 0개인 정상 KnownZero Radar 상태입니다.
	FCFRadarHUDData EmptyRadarViewData;
	EmptyRadarViewData.Availability = ECFUIViewAvailability::KnownZero;
	ApplyRadarViewDataOnce(EmptyRadarViewData);
	TestEqual(TEXT("UI-P0-08B KnownZero Radar Canvas 유지"), RadarCanvas->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-08B KnownZero Frame 유지"), RadarFrameImage->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-08B KnownZero Player 유지"), RadarPlayerImage->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestEqual(TEXT("UI-P0-08B KnownZero Range unknown이면 collapsed"), RadarRangeText->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-08B KnownZero runtime Blip hidden"), FriendlyBlip->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-08B KnownZero selected edge hidden"), SelectedEdgeImage->GetVisibility(), ESlateVisibility::Collapsed);

	// [v1.25.0] Sensor/Radar Provider 자체가 없는 fail-closed 상태입니다.
	FCFRadarHUDData UnavailableRadarViewData;
	UnavailableRadarViewData.Availability = ECFUIViewAvailability::Unavailable;
	ApplyRadarViewDataOnce(UnavailableRadarViewData);
			TestEqual(TEXT("UI-P0-08B unavailable Radar Canvas collapsed"), RadarCanvas->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-08B unavailable Frame collapsed"), RadarFrameImage->GetVisibility(), ESlateVisibility::Collapsed);
	TestEqual(TEXT("UI-P0-08B unavailable Player collapsed"), RadarPlayerImage->GetVisibility(), ESlateVisibility::Collapsed);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP009ViewModeDirectionTest,
	"CarFight.UI.UI_P0_09.ViewModeDirectionFoundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.26.0] 실제 VehicleCameraComp/AimComp의 기존 상태가 차체 기준 ViewMode ViewData로 전달되고 Null Rebind에서 제거되는지 검증합니다.
bool FCFHUDP009ViewModeDirectionTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.26.0] Asset 저장 없이 실제 Vehicle Pawn과 Component 수명을 제공할 transient Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-09A transient World 생성"), TestWorld))
	{
		return false;
	}

	// [v1.26.0] 차체 Heading 30도를 명시해 카메라·터렛 상대각 기준을 고정할 Spawn 설정입니다.
	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Name = FName(TEXT("UI_P0_09_ViewModePawn"));
	// [v1.26.0] 실제 Camera/Aim Component를 기본 소유하는 테스트 차량 Pawn입니다.
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator(0.0f, 30.0f, 0.0f),
		SpawnParameters);
	if (!TestNotNull(TEXT("UI-P0-09A Vehicle Pawn 생성"), VehiclePawn))
	{
		return false;
	}

	// [v1.26.0] 현재 Camera Mode와 실제 시선 방향을 제공할 기존 VehicleCameraComp입니다.
	UCFVehicleCameraComp* VehicleCameraComponent = VehiclePawn->GetVehicleCameraComp();
	// [v1.26.0] CurrentMuzzleDirection과 정렬 상태를 제공할 기존 VehicleAimComp입니다.
	UCFVehicleAimComp* VehicleAimComponent = VehiclePawn->GetVehicleAimComp();
	if (!TestNotNull(TEXT("UI-P0-09A VehicleCameraComp 존재"), VehicleCameraComponent)
		|| !TestNotNull(TEXT("UI-P0-09A VehicleAimComp 존재"), VehicleAimComponent))
	{
		return false;
	}

			// [v1.26.1] Transient Pawn에는 Production FollowCamera 참조가 없으므로 초기화를 강제하지 않고 CameraComp의 공식 fallback 시선 계약을 사용합니다.
	VehicleCameraComponent->ResetAimToVehicleForward();

	// [v1.26.0] 차체 Yaw 30도에서 월드 Yaw 60도·Pitch 10도로 향하도록 만든 실제 Weapon Aim Solution 입력입니다.
	FCFVehicleWeaponAimSolution WeaponAimSolution;
	WeaponAimSolution.bHasValidSolution = true;
	WeaponAimSolution.bTurretAligning = true;
	WeaponAimSolution.CurrentMuzzleDirection = FRotator(10.0f, 60.0f, 0.0f).Vector();
	WeaponAimSolution.AimDirection = WeaponAimSolution.CurrentMuzzleDirection;
	WeaponAimSolution.DesiredAimDirection = WeaponAimSolution.CurrentMuzzleDirection;
	VehicleAimComponent->SetWeaponAimSolution(WeaponAimSolution);

	// [v1.26.0] 실제 Pawn Runtime을 UI ViewData로 변환할 transient HUD Provider입니다.
	UCFHUDDataProvider* DataProvider = NewObject<UCFHUDDataProvider>();
	if (!TestNotNull(TEXT("UI-P0-09A HUD Provider 생성"), DataProvider))
	{
		return false;
	}
	DataProvider->RebindCurrentPawn(VehiclePawn);

	// [v1.26.0] Provider가 Camera/Aim Runtime에서 만든 현재 ViewMode 결과입니다.
	const FCFViewModeHUDData& ViewModeData = DataProvider->GetCurrentViewData().ViewMode;
	TestEqual(TEXT("UI-P0-09A ViewMode Known"), ViewModeData.Availability, ECFUIViewAvailability::Known);
	TestEqual(TEXT("UI-P0-09A 기본 Camera Mode Normal"), ViewModeData.CameraMode, ECFVehicleCameraMode::Normal);
	TestTrue(TEXT("UI-P0-09A Vehicle Heading 30도"), FMath::IsNearlyEqual(ViewModeData.VehicleHeadingDegrees, 30.0f, 0.1f));
	TestTrue(TEXT("UI-P0-09A Camera 상대 Yaw finite"), FMath::IsFinite(ViewModeData.CameraRelativeYawDegrees));
	TestTrue(TEXT("UI-P0-09A Camera 상대 Yaw -180~180"), ViewModeData.CameraRelativeYawDegrees >= -180.0f && ViewModeData.CameraRelativeYawDegrees <= 180.0f);
	TestTrue(TEXT("UI-P0-09A Camera Pitch finite"), FMath::IsFinite(ViewModeData.CameraPitchDegrees));
	TestTrue(TEXT("UI-P0-09A Turret 방향 사용 가능"), ViewModeData.bTurretDirectionAvailable);
	TestTrue(TEXT("UI-P0-09A Turret 상대 Yaw +30도"), FMath::IsNearlyEqual(ViewModeData.TurretRelativeYawDegrees, 30.0f, 0.1f));
	TestTrue(TEXT("UI-P0-09A Turret Pitch +10도"), FMath::IsNearlyEqual(ViewModeData.TurretPitchDegrees, 10.0f, 0.1f));
	TestTrue(TEXT("UI-P0-09A Turret Aligning 원본 전달"), ViewModeData.bTurretAligning);

	DataProvider->RebindCurrentPawn(nullptr);
	// [v1.26.0] Null Rebind 뒤 이전 차량의 ViewMode 방향이 남지 않았는지 확인할 새 ViewData입니다.
	const FCFViewModeHUDData& ClearedViewModeData = DataProvider->GetCurrentViewData().ViewMode;
	TestEqual(TEXT("UI-P0-09A Null Rebind ViewMode Unavailable"), ClearedViewModeData.Availability, ECFUIViewAvailability::Unavailable);
			TestFalse(TEXT("UI-P0-09A Null Rebind Turret 방향 제거"), ClearedViewModeData.bTurretDirectionAvailable);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP009ViewModeConsumerTest,
	"CarFight.UI.UI_P0_09.ViewModeProductionConsumerContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.27.0] 저장 Production Root의 Vehicle Direction Image가 ViewMode ViewData만 소비해 Camera 기준 차체 좌우 방향을 표시하는지 검증합니다.
bool FCFHUDP009ViewModeConsumerTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.27.0] 저장 Production HUD를 실제 인스턴스화해 ViewMode Presenter 적용을 확인할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-09B Production Consumer Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.27.0] DefaultGame.ini와 동일한 저장 Production HUD Generated Class입니다.
	UClass* ProductionHUDClass = LoadClass<UCFStyledWidgetBase>(
		nullptr,
		TEXT("/Game/CarFight/UI/HUD/WBP_CFInGameHUD.WBP_CFInGameHUD_C"));
	if (!TestNotNull(TEXT("UI-P0-09B Production HUD Class"), ProductionHUDClass))
	{
		return false;
	}

	// [v1.27.0] 저장 ReticleLayer와 ViewDirection 의미 Widget을 포함하는 테스트 전용 Production HUD 인스턴스입니다.
	UCFStyledWidgetBase* ProductionHUDWidget = CreateWidget<UCFStyledWidgetBase>(TestWorld, ProductionHUDClass);
	if (!TestNotNull(TEXT("UI-P0-09B Production HUD Widget"), ProductionHUDWidget))
	{
		return false;
	}

	// [v1.27.0] 기존 Production Root의 full-screen Reticle presentation owner입니다.
	UCanvasPanel* ReticleLayer = Cast<UCanvasPanel>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("CanvasPanel_Slot_ReticleLayer"))));
	// [v1.27.0] 위치·크기를 UMG Designer가 소유할 Vehicle Direction Track입니다.
	UCanvasPanel* ViewDirectionCanvas = Cast<UCanvasPanel>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("CanvasPanel_ViewDirection"))));
	// [v1.27.0] Camera 기준 차체 좌우 방향을 표시할 Vehicle Semantic Image입니다.
	UImage* VehicleDirectionImage = Cast<UImage>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("Image_ViewVehicleDirection"))));
	if (!TestNotNull(TEXT("UI-P0-09B ReticleLayer"), ReticleLayer)
		|| !TestNotNull(TEXT("UI-P0-09B ViewDirection Track"), ViewDirectionCanvas)
		|| !TestNotNull(TEXT("UI-P0-09B Vehicle Direction Image"), VehicleDirectionImage))
	{
		return false;
	}
	TestTrue(TEXT("UI-P0-09B Track은 ReticleLayer 직접 자식"), ViewDirectionCanvas->GetParent() == ReticleLayer);
	TestTrue(TEXT("UI-P0-09B Vehicle Image는 Track 직접 자식"), VehicleDirectionImage->GetParent() == ViewDirectionCanvas);
	TestNotNull(TEXT("UI-P0-09B Vehicle Semantic Brush"), VehicleDirectionImage->GetBrush().GetResourceObject());
	TestNull(TEXT("UI-P0-09B Text glyph 방향 표시는 만들지 않음"), ProductionHUDWidget->GetWidgetFromName(FName(TEXT("Text_ViewVehicleDirection"))));

	// [v1.27.0] Presenter가 runtime에는 건드리지 않아야 하는 Designer-owned Track Canvas Slot입니다.
	UCanvasPanelSlot* ViewDirectionTrackSlot = Cast<UCanvasPanelSlot>(ViewDirectionCanvas->Slot);
	// [v1.27.0] Presenter가 ViewData에 따라 X Anchor만 갱신할 Vehicle Image Canvas Slot입니다.
	UCanvasPanelSlot* VehicleDirectionSlot = Cast<UCanvasPanelSlot>(VehicleDirectionImage->Slot);
	if (!TestNotNull(TEXT("UI-P0-09B Track Canvas Slot"), ViewDirectionTrackSlot)
		|| !TestNotNull(TEXT("UI-P0-09B Vehicle Direction Canvas Slot"), VehicleDirectionSlot))
	{
		return false;
	}
	// [v1.27.0] Presenter 적용 전 Designer Track Anchor를 보존 여부 비교용으로 저장합니다.
	const FAnchors InitialTrackAnchors = ViewDirectionTrackSlot->GetAnchors();
	// [v1.27.0] Presenter 적용 전 Designer Track Offset을 보존 여부 비교용으로 저장합니다.
	const FMargin InitialTrackOffsets = ViewDirectionTrackSlot->GetOffsets();

	// [v1.27.0] synthetic ViewMode ViewData를 저장 Production Widget에 적용할 Presenter입니다.
	UCFHUDPresenter* Presenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-09B Presenter"), Presenter))
	{
		return false;
	}
	Presenter->SetProductionWidget(ProductionHUDWidget);

	// [v1.27.0] private UFUNCTION HandleHUDViewDataChanged를 실제 Provider delegate와 같은 경로로 호출할 Reflection 함수입니다.
	UFunction* HandleViewDataFunction = Presenter->FindFunction(FName(TEXT("HandleHUDViewDataChanged")));
	if (!TestNotNull(TEXT("UI-P0-09B HandleHUDViewDataChanged"), HandleViewDataFunction))
	{
		return false;
	}

	// [v1.27.0] Reflection UFUNCTION의 단일 ViewData 인자를 전달할 파라미터 구조입니다.
	struct FHandleHUDViewDataChangedParams
	{
		// [v1.27.0] Presenter에 적용할 전체 HUD ViewData입니다.
		FCFInGameUIViewData ViewData;
	};

	// [v1.27.0] ViewMode ViewData를 Production Presenter에 정확히 한 번 적용하는 Helper입니다.
	auto ApplyViewModeOnce = [Presenter, HandleViewDataFunction](const FCFViewModeHUDData& ViewModeViewData)
	{
		// [v1.27.0] 이번 한 번의 Presenter 적용에 사용할 전체 ViewData 파라미터입니다.
		FHandleHUDViewDataChangedParams Params;
		Params.ViewData.ViewMode = ViewModeViewData;
		Presenter->ProcessEvent(HandleViewDataFunction, &Params);
	};

	// [v1.27.0] Camera와 차체 정면이 일치해 Vehicle Direction이 Track 중앙에 있어야 하는 기본 상태입니다.
	FCFViewModeHUDData CenterViewMode;
	CenterViewMode.Availability = ECFUIViewAvailability::Known;
	CenterViewMode.CameraMode = ECFVehicleCameraMode::Normal;
	CenterViewMode.CameraRelativeYawDegrees = 0.0f;
	ApplyViewModeOnce(CenterViewMode);
	TestEqual(TEXT("UI-P0-09B 중앙 Vehicle Direction visible"), VehicleDirectionImage->GetVisibility(), ESlateVisibility::HitTestInvisible);
	TestTrue(TEXT("UI-P0-09B 중앙 anchor X 0.5"), FMath::IsNearlyEqual(VehicleDirectionSlot->GetAnchors().Minimum.X, 0.5f, 0.001f));

	// [v1.27.0] Camera가 차체보다 오른쪽 45도를 보면 차체 방향은 화면 왼쪽 25% 지점에 있어야 합니다.
	FCFViewModeHUDData CameraRightViewMode = CenterViewMode;
	CameraRightViewMode.CameraRelativeYawDegrees = 45.0f;
	ApplyViewModeOnce(CameraRightViewMode);
	TestTrue(TEXT("UI-P0-09B Camera +45 => Vehicle anchor X 0.25"), FMath::IsNearlyEqual(VehicleDirectionSlot->GetAnchors().Minimum.X, 0.25f, 0.001f));

	// [v1.27.0] Camera가 차체보다 왼쪽 90도를 보면 차체 방향은 Track 오른쪽 끝에 있어야 합니다.
	FCFViewModeHUDData CameraLeftViewMode = CenterViewMode;
	CameraLeftViewMode.CameraRelativeYawDegrees = -90.0f;
	ApplyViewModeOnce(CameraLeftViewMode);
	TestTrue(TEXT("UI-P0-09B Camera -90 => Vehicle anchor X 1.0"), FMath::IsNearlyEqual(VehicleDirectionSlot->GetAnchors().Minimum.X, 1.0f, 0.001f));

	// [v1.27.0] Camera가 차체보다 오른쪽 135도여도 presentation 범위를 넘지 않고 왼쪽 끝으로 clamp되어야 합니다.
	FCFViewModeHUDData CameraFarRightViewMode = CenterViewMode;
	CameraFarRightViewMode.CameraRelativeYawDegrees = 135.0f;
	ApplyViewModeOnce(CameraFarRightViewMode);
	TestTrue(TEXT("UI-P0-09B Camera +135 => Vehicle anchor X clamp 0.0"), FMath::IsNearlyEqual(VehicleDirectionSlot->GetAnchors().Minimum.X, 0.0f, 0.001f));

	// [v1.27.0] Player vehicle 방향을 표현하면 안 되는 Spectate 상태입니다.
	FCFViewModeHUDData SpectateViewMode = CenterViewMode;
	SpectateViewMode.CameraMode = ECFVehicleCameraMode::Spectate;
	ApplyViewModeOnce(SpectateViewMode);
	TestEqual(TEXT("UI-P0-09B Spectate Vehicle Direction hidden"), VehicleDirectionImage->GetVisibility(), ESlateVisibility::Collapsed);

	// [v1.27.0] 파괴 Camera에서 이전 차체 방향을 남기지 않아야 하는 상태입니다.
	FCFViewModeHUDData DestroyedViewMode = CenterViewMode;
	DestroyedViewMode.CameraMode = ECFVehicleCameraMode::Destroyed;
	ApplyViewModeOnce(DestroyedViewMode);
	TestEqual(TEXT("UI-P0-09B Destroyed Vehicle Direction hidden"), VehicleDirectionImage->GetVisibility(), ESlateVisibility::Collapsed);

	// [v1.27.0] Pawn/Rebind source가 없어 ViewMode 자체가 Unavailable인 fail-closed 상태입니다.
	FCFViewModeHUDData UnavailableViewMode;
	UnavailableViewMode.Availability = ECFUIViewAvailability::Unavailable;
	ApplyViewModeOnce(UnavailableViewMode);
	TestEqual(TEXT("UI-P0-09B Unavailable Vehicle Direction hidden"), VehicleDirectionImage->GetVisibility(), ESlateVisibility::Collapsed);

	TestTrue(TEXT("UI-P0-09B Presenter는 Designer Track Anchor를 변경하지 않음"), ViewDirectionTrackSlot->GetAnchors() == InitialTrackAnchors);
		TestTrue(TEXT("UI-P0-09B Presenter는 Designer Track Offset을 변경하지 않음"), ViewDirectionTrackSlot->GetOffsets() == InitialTrackOffsets);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP009AlertStyleTest,
	"CarFight.UI.UI_P0_09.AlertStyleLifecycleContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.30.0] Alert duration이 반복 Refresh에 의해 리셋되지 않고 suppression 시간은 소비하지 않으며 Critical은 상태 해제까지 유지되는지 검증합니다.
bool FCFHUDP009AlertStyleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.28.0] private Alert lifecycle resolver를 직접 검증할 transient Presenter입니다.
	UCFHUDPresenter* Presenter = NewObject<UCFHUDPresenter>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-09C Alert Presenter"), Presenter))
	{
		return false;
	}

	// [v1.28.0] Production Root가 Context 미주입 시에도 사용하는 Native Style CDO입니다.
	const UCFUIStyleData* StyleData = GetDefault<UCFUIStyleData>();
	if (!TestNotNull(TEXT("UI-P0-09C Native Style Data"), StyleData))
	{
		return false;
	}
	TestTrue(TEXT("UI-P0-09C Notice 기본 2초"), FMath::IsNearlyEqual(StyleData->AlertStyle.Notice.DefaultDurationSeconds, 2.0f));
	TestTrue(TEXT("UI-P0-09C Warning 기본 3초"), FMath::IsNearlyEqual(StyleData->AlertStyle.Warning.DefaultDurationSeconds, 3.0f));
	TestTrue(TEXT("UI-P0-09C Critical Persistent"), StyleData->AlertStyle.Critical.bPersistentByDefault);

	// [v1.28.0] 반복 ViewData Refresh에서 최초 시각이 유지되는지 검증할 Warning 상태입니다.
	FCFCombatAlertViewData WarningAlerts;
	WarningAlerts.Availability = ECFUIViewAvailability::Known;
	// [v1.28.0] Caution Priority가 Warning Style 3초 계약을 소비할 실제 테스트 Alert입니다.
	FCFHUDAlertItem WarningAlert;
	WarningAlert.AlertKey = FName(TEXT("Test.Warning"));
	WarningAlert.Priority = ECFHUDAlertPriority::Caution;
	WarningAlert.Title = FText::FromString(TEXT("WARNING"));
	WarningAlerts.ActiveAlerts.Add(WarningAlert);

	// [v1.28.0] Warning 최초 활성 시각 10초에서 선택된 Alert입니다.
	const FCFHUDAlertItem* WarningAtStart = Presenter->ResolveAlertForPresentation(WarningAlerts, *StyleData, 10.0);
	TestNotNull(TEXT("UI-P0-09C Warning 시작 시 표시"), WarningAtStart);
	// [v1.28.0] 같은 AlertKey 반복 Refresh를 일부러 넣어 최초 시각이 리셋되지 않는지 확인할 중간 결과입니다.
	const FCFHUDAlertItem* WarningAtRefresh = Presenter->ResolveAlertForPresentation(WarningAlerts, *StyleData, 11.5);
	TestNotNull(TEXT("UI-P0-09C Warning 1.5초 반복 Refresh 후 표시"), WarningAtRefresh);
	// [v1.28.0] 최초 10초 기준 3초를 넘긴 뒤 같은 상태가 계속 존재해도 다시 보이면 안 되는 결과입니다.
	const FCFHUDAlertItem* WarningAfterExpiry = Presenter->ResolveAlertForPresentation(WarningAlerts, *StyleData, 13.1);
	TestNull(TEXT("UI-P0-09C Warning 반복 Refresh가 3초를 리셋하지 않음"), WarningAfterExpiry);

	// [v1.28.0] 실제 Gameplay 상태에서 Warning AlertKey가 사라진 해제 ViewData입니다.
	FCFCombatAlertViewData ClearedAlerts;
	ClearedAlerts.Availability = ECFUIViewAvailability::KnownZero;
	// [v1.28.0] 상태 해제가 기존 AlertKey lifecycle을 제거하는지 확인할 결과입니다.
	const FCFHUDAlertItem* ClearedPresentation = Presenter->ResolveAlertForPresentation(ClearedAlerts, *StyleData, 14.0);
	TestNull(TEXT("UI-P0-09C 상태 해제 시 Alert 없음"), ClearedPresentation);
		
	TestEqual(TEXT("UI-P0-09C 상태 해제 시 first-presented cache 비움"), Presenter->AlertFirstPresentedGameTimeSeconds.Num(), 0);
	TestEqual(TEXT("UI-P0-09C 상태 해제 시 completed cache 비움"), Presenter->CompletedAlertPresentationKeys.Num(), 0);

	// [v1.28.0] 같은 AlertKey가 실제 상태 해제 뒤 재발생했을 때 새 3초 lifecycle이 시작되는 결과입니다.
	const FCFHUDAlertItem* WarningAfterReentry = Presenter->ResolveAlertForPresentation(WarningAlerts, *StyleData, 20.0);
	TestNotNull(TEXT("UI-P0-09C Warning 재발생 시 다시 표시"), WarningAfterReentry);
	// [v1.28.0] 재발생 기준 2.9초에서는 Warning이 아직 표시되어야 하는 결과입니다.
	const FCFHUDAlertItem* WarningBeforeSecondExpiry = Presenter->ResolveAlertForPresentation(WarningAlerts, *StyleData, 22.9);
	TestNotNull(TEXT("UI-P0-09C 재발생 Warning 2.9초 표시"), WarningBeforeSecondExpiry);
	// [v1.28.0] 재발생 기준 3초를 넘긴 뒤 다시 제거되는 결과입니다.
	const FCFHUDAlertItem* WarningAfterSecondExpiry = Presenter->ResolveAlertForPresentation(WarningAlerts, *StyleData, 23.1);
	TestNull(TEXT("UI-P0-09C 재발생 Warning 3초 후 제거"), WarningAfterSecondExpiry);

	Presenter->ResetAlertPresentationLifecycle();
	// [v1.28.0] 배열 순서와 무관하게 가장 높은 Priority가 선택되는지 검증할 혼합 Alert 상태입니다.
	FCFCombatAlertViewData MixedPriorityAlerts;
	MixedPriorityAlerts.Availability = ECFUIViewAvailability::Known;
	// [v1.28.0] 혼합 상태의 낮은 Info 후보입니다.
	FCFHUDAlertItem InfoAlert;
	InfoAlert.AlertKey = FName(TEXT("Test.Info"));
	InfoAlert.Priority = ECFHUDAlertPriority::Info;
	InfoAlert.Title = FText::FromString(TEXT("INFO"));
	// [v1.28.0] 혼합 상태의 중간 Caution 후보입니다.
	FCFHUDAlertItem CautionAlert;
	CautionAlert.AlertKey = FName(TEXT("Test.Caution"));
	CautionAlert.Priority = ECFHUDAlertPriority::Caution;
	CautionAlert.Title = FText::FromString(TEXT("CAUTION"));
	// [v1.28.0] 혼합 상태의 최고 Critical 후보이며 Persistent 계약을 검증합니다.
	FCFHUDAlertItem CriticalAlert;
	CriticalAlert.AlertKey = FName(TEXT("Test.Critical"));
	CriticalAlert.Priority = ECFHUDAlertPriority::Critical;
	CriticalAlert.Title = FText::FromString(TEXT("CRITICAL"));
	MixedPriorityAlerts.ActiveAlerts.Add(InfoAlert);
	MixedPriorityAlerts.ActiveAlerts.Add(CriticalAlert);
	MixedPriorityAlerts.ActiveAlerts.Add(CautionAlert);

	// [v1.28.0] 정렬되지 않은 혼합 배열에서 실제 최고 Priority로 선택된 첫 결과입니다.
	const FCFHUDAlertItem* HighestPriorityAlert = Presenter->ResolveAlertForPresentation(MixedPriorityAlerts, *StyleData, 30.0);
	if (TestNotNull(TEXT("UI-P0-09C 혼합 Priority Alert 선택"), HighestPriorityAlert))
	{
		TestEqual(TEXT("UI-P0-09C 배열 순서와 무관하게 Critical 우선"), HighestPriorityAlert->AlertKey, CriticalAlert.AlertKey);
	}
	// [v1.28.0] 매우 긴 시간이 지나도 상태가 유지되는 Critical Persistent 결과입니다.
		
	const FCFHUDAlertItem* CriticalAfterLongDuration = Presenter->ResolveAlertForPresentation(MixedPriorityAlerts, *StyleData, 3000.0);
	if (TestNotNull(TEXT("UI-P0-09C Critical 장시간 후에도 Persistent"), CriticalAfterLongDuration))
	{
		TestEqual(TEXT("UI-P0-09C 장시간 후 Critical 유지"), CriticalAfterLongDuration->AlertKey, CriticalAlert.AlertKey);
	}

	Presenter->ResetAlertPresentationLifecycle();
	// [v1.30.0] Critical에 가려진 Caution이 화면에 나오기 전에는 3초 duration을 소비하지 않는지 검증할 혼합 상태입니다.
	FCFCombatAlertViewData SuppressedAlerts;
	SuppressedAlerts.Availability = ECFUIViewAvailability::Known;
	SuppressedAlerts.ActiveAlerts.Add(CautionAlert);
	SuppressedAlerts.ActiveAlerts.Add(CriticalAlert);

	// [v1.30.0] suppression 시작 시 실제 화면을 소유해야 하는 Persistent Critical입니다.
	const FCFHUDAlertItem* SuppressionStartAlert = Presenter->ResolveAlertForPresentation(SuppressedAlerts, *StyleData, 100.0);
	if (TestNotNull(TEXT("UI-P0-09C suppression 시작 Critical 표시"), SuppressionStartAlert))
	{
		TestEqual(TEXT("UI-P0-09C suppression 시작 Critical 우선"), SuppressionStartAlert->AlertKey, CriticalAlert.AlertKey);
	}
	// [v1.30.0] Warning 기본 duration보다 훨씬 긴 10초 suppression 뒤에도 Caution first-presented 시간이 없어야 하는 결과입니다.
	const FCFHUDAlertItem* SuppressionAfterTenSeconds = Presenter->ResolveAlertForPresentation(SuppressedAlerts, *StyleData, 110.0);
	if (TestNotNull(TEXT("UI-P0-09C 10초 suppression 후 Critical 유지"), SuppressionAfterTenSeconds))
	{
		TestEqual(TEXT("UI-P0-09C 10초 suppression 후에도 Critical 우선"), SuppressionAfterTenSeconds->AlertKey, CriticalAlert.AlertKey);
	}
	TestFalse(
		TEXT("UI-P0-09C suppressed Caution은 표시 전 duration 미시작"),
		Presenter->AlertFirstPresentedGameTimeSeconds.Contains(CautionAlert.AlertKey));

	// [v1.30.0] Critical 상태가 해제되고 동일 Caution만 계속 활성인 첫 실제 Presentation 상태입니다.
	FCFCombatAlertViewData CautionOnlyAlerts;
	CautionOnlyAlerts.Availability = ECFUIViewAvailability::Known;
	CautionOnlyAlerts.ActiveAlerts.Add(CautionAlert);
	// [v1.30.0] Critical 해제 직후 Caution이 처음 화면을 소유해야 하는 결과입니다.
	const FCFHUDAlertItem* CautionFirstPresented = Presenter->ResolveAlertForPresentation(CautionOnlyAlerts, *StyleData, 110.1);
	if (TestNotNull(TEXT("UI-P0-09C suppression 해제 후 Caution 첫 표시"), CautionFirstPresented))
	{
		TestEqual(TEXT("UI-P0-09C suppression 해제 후 Caution 선택"), CautionFirstPresented->AlertKey, CautionAlert.AlertKey);
	}
	// [v1.30.0] Caution이 실제로 처음 표시된 Game-Time을 확인할 cache 값입니다.
	const double* CautionFirstPresentedTime = Presenter->AlertFirstPresentedGameTimeSeconds.Find(CautionAlert.AlertKey);
	if (TestNotNull(TEXT("UI-P0-09C Caution first-presented time 기록"), CautionFirstPresentedTime))
	{
		TestTrue(TEXT("UI-P0-09C Caution duration은 110.1부터 시작"), FMath::IsNearlyEqual(*CautionFirstPresentedTime, 110.1));
	}

	// [v1.30.0] 실제 첫 표시 후 2.8초에는 Caution이 아직 표시되어야 하는 결과입니다.
	const FCFHUDAlertItem* CautionBeforeExpiry = Presenter->ResolveAlertForPresentation(CautionOnlyAlerts, *StyleData, 112.9);
	TestNotNull(TEXT("UI-P0-09C Caution 첫 표시 후 2.8초 유지"), CautionBeforeExpiry);
	// [v1.30.0] 실제 첫 표시 후 3초를 넘긴 같은 활성 상태에서는 완료되어 사라져야 하는 결과입니다.
	const FCFHUDAlertItem* CautionAfterExpiry = Presenter->ResolveAlertForPresentation(CautionOnlyAlerts, *StyleData, 113.2);
	TestNull(TEXT("UI-P0-09C Caution 첫 표시 후 3초 초과 제거"), CautionAfterExpiry);
	TestTrue(TEXT("UI-P0-09C 만료 Caution completed 기록"), Presenter->CompletedAlertPresentationKeys.Contains(CautionAlert.AlertKey));

	// [v1.30.0] Gameplay 상태 해제가 first-presented/completed lifecycle을 모두 비우는 최종 상태입니다.
	const FCFHUDAlertItem* CautionCleared = Presenter->ResolveAlertForPresentation(ClearedAlerts, *StyleData, 114.0);
	TestNull(TEXT("UI-P0-09C suppression 회귀 상태 해제 후 Alert 없음"), CautionCleared);
	TestEqual(TEXT("UI-P0-09C suppression 회귀 first-presented cache 비움"), Presenter->AlertFirstPresentedGameTimeSeconds.Num(), 0);
	TestEqual(TEXT("UI-P0-09C suppression 회귀 completed cache 비움"), Presenter->CompletedAlertPresentationKeys.Num(), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP009NestedStyleTest,
	"CarFight.UI.UI_P0_09.NestedStyleContextContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.28.0] Production Root에 주입한 Style·Density·Scale Context가 중첩 Styled Panel까지 동일하게 전파되는지 검증합니다.
bool FCFHUDP009NestedStyleTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.28.0] 저장 Production HUD를 실제 인스턴스화해 중첩 Context 전파를 확인할 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-09D Production Style Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.28.0] DefaultGame.ini와 동일한 저장 Production HUD Generated Class입니다.
	UClass* ProductionHUDClass = LoadClass<UCFStyledWidgetBase>(
		nullptr,
		TEXT("/Game/CarFight/UI/HUD/WBP_CFInGameHUD.WBP_CFInGameHUD_C"));
	if (!TestNotNull(TEXT("UI-P0-09D Production HUD Class"), ProductionHUDClass))
	{
		return false;
	}

	// [v1.28.0] 중첩 Styled Panel을 포함하는 실제 저장 Production HUD 인스턴스입니다.
	UCFStyledWidgetBase* ProductionHUDWidget = CreateWidget<UCFStyledWidgetBase>(TestWorld, ProductionHUDClass);
	if (!TestNotNull(TEXT("UI-P0-09D Production HUD Widget"), ProductionHUDWidget))
	{
		return false;
	}

	// [v1.28.0] Root WidgetTree의 실제 Production VehiclePanel Styled Widget입니다.
	UCFStyledWidgetBase* VehiclePanelWidget = Cast<UCFStyledWidgetBase>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("WBP_CFVehiclePanel"))));
	// [v1.28.0] Root WidgetTree의 실제 Production AlertFeed Styled Widget입니다.
	UCFStyledWidgetBase* AlertFeedWidget = Cast<UCFStyledWidgetBase>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("WBP_CFAlertFeed"))));
	if (!TestNotNull(TEXT("UI-P0-09D VehiclePanel Styled Widget"), VehiclePanelWidget)
		|| !TestNotNull(TEXT("UI-P0-09D AlertFeed Styled Widget"), AlertFeedWidget))
	{
		return false;
	}

	// [v1.28.0] Root가 해석했다고 가정해 주입할 테스트 전용 Style Data입니다.
	UCFUIStyleData* InjectedStyleData = NewObject<UCFUIStyleData>(GetTransientPackage());
	// [v1.28.0] Root가 해석했다고 가정해 주입할 테스트 전용 Density Data입니다.
	UCFUIDensityData* InjectedDensityData = NewObject<UCFUIDensityData>(GetTransientPackage());
	if (!TestNotNull(TEXT("UI-P0-09D Injected Style Data"), InjectedStyleData)
		|| !TestNotNull(TEXT("UI-P0-09D Injected Density Data"), InjectedDensityData))
	{
		return false;
	}

	// [v1.28.0] Typography Scale까지 자식이 동일하게 소비하는지 확인할 기본 Typography Floor입니다.
	FCFUITypographyFloor InjectedTypographyFloor;
	// [v1.28.0] 기본 1.0과 구분되는 Geometry Scale 테스트 값입니다.
	constexpr float InjectedGeometryScale = 1.25f;
	// [v1.28.0] 기본 1.0과 구분되는 Typography Scale 테스트 값입니다.
	constexpr float InjectedTypographyScale = 1.15f;
	ProductionHUDWidget->SetUIVisualContext(
		InjectedStyleData,
		InjectedDensityData,
		InjectedGeometryScale,
		InjectedTypographyScale,
		InjectedTypographyFloor);

	TestTrue(TEXT("UI-P0-09D VehiclePanel Style 동일 참조"), VehiclePanelWidget->GetUIStyleData() == InjectedStyleData);
	TestTrue(TEXT("UI-P0-09D AlertFeed Style 동일 참조"), AlertFeedWidget->GetUIStyleData() == InjectedStyleData);
	TestTrue(TEXT("UI-P0-09D VehiclePanel Density 동일 참조"), VehiclePanelWidget->GetUIDensityData() == InjectedDensityData);
	TestTrue(TEXT("UI-P0-09D AlertFeed Density 동일 참조"), AlertFeedWidget->GetUIDensityData() == InjectedDensityData);
	TestTrue(TEXT("UI-P0-09D VehiclePanel Geometry Scale 전파"), FMath::IsNearlyEqual(VehiclePanelWidget->GetResolvedGeometryScale(), InjectedGeometryScale));
	TestTrue(TEXT("UI-P0-09D AlertFeed Geometry Scale 전파"), FMath::IsNearlyEqual(AlertFeedWidget->GetResolvedGeometryScale(), InjectedGeometryScale));

	// [v1.28.0] 동일 Typography Context가 Root와 VehiclePanel에서 같은 실효 Body Font Size로 해석되는지 비교할 Root Font입니다.
	const FSlateFontInfo RootBodyFont = ProductionHUDWidget->ResolveTypographyFont(ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body);
	// [v1.28.0] Root와 같은 Typography Context를 전달받은 VehiclePanel의 실효 Body Font입니다.
	const FSlateFontInfo VehiclePanelBodyFont = VehiclePanelWidget->ResolveTypographyFont(ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body);
	// [v1.28.0] Root와 같은 Typography Context를 전달받은 AlertFeed의 실효 Body Font입니다.
	const FSlateFontInfo AlertFeedBodyFont = AlertFeedWidget->ResolveTypographyFont(ECFUIFontFamilyRole::UI, ECFUITypographyRole::Body);
	TestEqual(TEXT("UI-P0-09D VehiclePanel Typography Size 전파"), VehiclePanelBodyFont.Size, RootBodyFont.Size);
	TestEqual(TEXT("UI-P0-09D AlertFeed Typography Size 전파"), AlertFeedBodyFont.Size, RootBodyFont.Size);

	// [v1.28.0] 첫 Context 적용 뒤 실제 Refresh가 발생한 VehiclePanel revision입니다.
	const int32 VehiclePanelRevisionAfterFirstApply = VehiclePanelWidget->GetVisualContextRevision();
	// [v1.28.0] 첫 Context 적용 뒤 실제 Refresh가 발생한 AlertFeed revision입니다.
	const int32 AlertFeedRevisionAfterFirstApply = AlertFeedWidget->GetVisualContextRevision();
	TestTrue(TEXT("UI-P0-09D VehiclePanel 첫 Refresh 발생"), VehiclePanelRevisionAfterFirstApply > 0);
	TestTrue(TEXT("UI-P0-09D AlertFeed 첫 Refresh 발생"), AlertFeedRevisionAfterFirstApply > 0);

	ProductionHUDWidget->SetUIVisualContext(
		InjectedStyleData,
		InjectedDensityData,
		InjectedGeometryScale,
		InjectedTypographyScale,
		InjectedTypographyFloor);
	TestEqual(TEXT("UI-P0-09D 동일 Context VehiclePanel 중복 Refresh 없음"), VehiclePanelWidget->GetVisualContextRevision(), VehiclePanelRevisionAfterFirstApply);
		TestEqual(TEXT("UI-P0-09D 동일 Context AlertFeed 중복 Refresh 없음"), AlertFeedWidget->GetVisualContextRevision(), AlertFeedRevisionAfterFirstApply);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFHUDP010ResolutionLayoutTest,
	"CarFight.UI.UI_P0_10.ResolutionLayoutContract",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// [v1.29.0] 저장 Production Root의 Designer Slot이 P0 필수 4개 해상도에서 화면내·비겹침을 유지하고 ReticleLayer가 full-stretch인지 검증합니다.
bool FCFHUDP010ResolutionLayoutTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// [v1.29.0] 저장 Production HUD를 실제 인스턴스화해 Designer Slot 값을 읽을 Automation World입니다.
	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	if (!TestNotNull(TEXT("UI-P0-10 Resolution Automation World"), TestWorld))
	{
		return false;
	}

	// [v1.29.0] 실제 게임에서 사용하는 저장 Production HUD Generated Class입니다.
	UClass* ProductionHUDClass = LoadClass<UCFStyledWidgetBase>(
		nullptr,
		TEXT("/Game/CarFight/UI/HUD/WBP_CFInGameHUD.WBP_CFInGameHUD_C"));
	if (!TestNotNull(TEXT("UI-P0-10 Production HUD Class"), ProductionHUDClass))
	{
		return false;
	}

	// [v1.29.0] 저장 Designer Root와 7 direct child Slot을 보유한 실제 Production HUD 인스턴스입니다.
	UCFStyledWidgetBase* ProductionHUDWidget = CreateWidget<UCFStyledWidgetBase>(TestWorld, ProductionHUDClass);
	if (!TestNotNull(TEXT("UI-P0-10 Production HUD Widget"), ProductionHUDWidget))
	{
		return false;
	}

	// [v1.29.0] Production HUD의 저장 Root Canvas입니다.
	UCanvasPanel* RootCanvas = Cast<UCanvasPanel>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("CanvasPanel_Root"))));
	if (!TestNotNull(TEXT("UI-P0-10 Root Canvas"), RootCanvas))
	{
		return false;
	}
	TestEqual(TEXT("UI-P0-10 Root direct child 7개 유지"), RootCanvas->GetChildrenCount(), 7);

	// [v1.29.0] 여섯 고정 Production Panel의 Root SizeBox 의미 이름입니다.
	const FName ProductionPanelSlotNames[] =
	{
		FName(TEXT("SizeBox_Slot_MissionSummary")),
		FName(TEXT("SizeBox_Slot_AlertFeed")),
		FName(TEXT("SizeBox_Slot_TargetPanel")),
		FName(TEXT("SizeBox_Slot_VehiclePanel")),
		FName(TEXT("SizeBox_Slot_RadarPanel")),
		FName(TEXT("SizeBox_Slot_WeaponPanel"))
	};

		// [v1.29.0] P0 Integration Validation에서 직접 검증할 필수 네 물리 Viewport 해상도입니다.
	const FVector2D IntegrationViewportSizes[] =
	{
		FVector2D(1920.0f, 1080.0f),
		FVector2D(2560.0f, 1440.0f),
		FVector2D(3440.0f, 1440.0f),
		FVector2D(5120.0f, 1440.0f)
	};

	// [v1.29.0] AimReticle·Target Marker·ViewDirection이 공유하는 기존 full-screen Reticle Layer입니다.
	UCanvasPanel* ReticleLayer = Cast<UCanvasPanel>(ProductionHUDWidget->GetWidgetFromName(FName(TEXT("CanvasPanel_Slot_ReticleLayer"))));
	// [v1.29.0] ReticleLayer의 persisted Root Canvas Slot입니다.
	UCanvasPanelSlot* ReticleLayerSlot = ReticleLayer ? Cast<UCanvasPanelSlot>(ReticleLayer->Slot) : nullptr;
	if (!TestNotNull(TEXT("UI-P0-10 Reticle Layer"), ReticleLayer)
		|| !TestNotNull(TEXT("UI-P0-10 Reticle Layer Canvas Slot"), ReticleLayerSlot))
	{
		return false;
	}

	// [v1.29.0] ReticleLayer가 전체 Viewport를 stretch하도록 저장된 Anchor입니다.
	const FAnchors ReticleAnchors = ReticleLayerSlot->GetAnchors();
	// [v1.29.0] ReticleLayer full-stretch에서 추가 여백이 없어야 하는 저장 Offset입니다.
	const FMargin ReticleOffsets = ReticleLayerSlot->GetOffsets();
	TestTrue(TEXT("UI-P0-10 Reticle Anchor Min 0,0"), ReticleAnchors.Minimum.Equals(FVector2D::ZeroVector, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("UI-P0-10 Reticle Anchor Max 1,1"), ReticleAnchors.Maximum.Equals(FVector2D(1.0f, 1.0f), KINDA_SMALL_NUMBER));
	TestTrue(TEXT("UI-P0-10 Reticle full-stretch Offset 0"),
		FMath::IsNearlyZero(ReticleOffsets.Left)
		&& FMath::IsNearlyZero(ReticleOffsets.Top)
		&& FMath::IsNearlyZero(ReticleOffsets.Right)
		&& FMath::IsNearlyZero(ReticleOffsets.Bottom));

		// [v1.29.0] 현재 Project Settings의 실제 DPI Scale 정책을 제공하는 Engine UI Settings입니다.
	const UUserInterfaceSettings* UserInterfaceSettings = GetDefault<UUserInterfaceSettings>();
	if (!TestNotNull(TEXT("UI-P0-10 UserInterfaceSettings"), UserInterfaceSettings))
	{
		return false;
	}

	// [v1.29.0] 네 필수 Viewport를 순회할 인덱스입니다.
	int32 ViewportIndex = 0;
	for (; ViewportIndex < UE_ARRAY_COUNT(IntegrationViewportSizes); ++ViewportIndex)
	{
				// [v1.29.0] 현재 검증 대상의 실제 물리 Viewport 해상도입니다.
		const FVector2D PhysicalViewportSize = IntegrationViewportSizes[ViewportIndex];
		// [v1.29.0] 현재 Project DPI Curve/Rule이 이 물리 해상도에 적용하는 실제 Slate/UMG Scale입니다.
		const float ViewportDpiScale = UserInterfaceSettings->GetDPIScaleBasedOnSize(FIntPoint(
			FMath::RoundToInt(PhysicalViewportSize.X),
			FMath::RoundToInt(PhysicalViewportSize.Y)));
		TestTrue(
			FString::Printf(TEXT("UI-P0-10 %.0fx%.0f DPI Scale 유효"), PhysicalViewportSize.X, PhysicalViewportSize.Y),
			FMath::IsFinite(ViewportDpiScale) && ViewportDpiScale > 0.0f);
		if (!FMath::IsFinite(ViewportDpiScale) || ViewportDpiScale <= 0.0f)
		{
			return false;
		}

		// [v1.29.0] 실제 UMG Root Canvas가 배치 계산에 사용하는 DPI 적용 후 logical Viewport 크기입니다.
		const FVector2D ViewportSize = PhysicalViewportSize / ViewportDpiScale;
		// [v1.29.0] 현재 Viewport에서 계산된 여섯 Panel의 MinX/MinY/MaxX/MaxY Rect 목록입니다.
		TArray<FVector4> ResolvedPanelRects;
		ResolvedPanelRects.Reserve(UE_ARRAY_COUNT(ProductionPanelSlotNames));

		// [v1.29.0] 여섯 Production Panel Slot을 순회할 인덱스입니다.
		int32 PanelSlotIndex = 0;
		for (; PanelSlotIndex < UE_ARRAY_COUNT(ProductionPanelSlotNames); ++PanelSlotIndex)
		{
			// [v1.29.0] 현재 해상도에서 geometry를 검증할 저장 Root SizeBox입니다.
			UWidget* PanelSlotWidget = ProductionHUDWidget->GetWidgetFromName(ProductionPanelSlotNames[PanelSlotIndex]);
			// [v1.29.0] 현재 SizeBox의 Designer-owned Anchor/Offset/Alignment를 제공하는 Canvas Slot입니다.
			UCanvasPanelSlot* PanelCanvasSlot = PanelSlotWidget ? Cast<UCanvasPanelSlot>(PanelSlotWidget->Slot) : nullptr;
			if (!TestNotNull(FString::Printf(TEXT("UI-P0-10 %s Widget"), *ProductionPanelSlotNames[PanelSlotIndex].ToString()), PanelSlotWidget)
				|| !TestNotNull(FString::Printf(TEXT("UI-P0-10 %s Canvas Slot"), *ProductionPanelSlotNames[PanelSlotIndex].ToString()), PanelCanvasSlot))
			{
				return false;
			}

			// [v1.29.0] 현재 Production Panel이 사용 중인 고정 Anchor입니다.
			const FAnchors PanelAnchors = PanelCanvasSlot->GetAnchors();
			TestTrue(
				FString::Printf(TEXT("UI-P0-10 %s fixed-anchor 유지"), *ProductionPanelSlotNames[PanelSlotIndex].ToString()),
				PanelAnchors.Minimum.Equals(PanelAnchors.Maximum, KINDA_SMALL_NUMBER));

			// [v1.29.0] 고정 Anchor 기준 위치와 Width/Height를 담는 persisted Canvas Offset입니다.
			const FMargin PanelOffsets = PanelCanvasSlot->GetOffsets();
			// [v1.29.0] 고정 Anchor 기준점에 적용할 Designer Alignment입니다.
			const FVector2D PanelAlignment = PanelCanvasSlot->GetAlignment();
			// [v1.29.0] 현재 synthetic Viewport에서 Anchor가 가리키는 실제 Pixel 위치입니다.
			const FVector2D AnchorPosition(
				ViewportSize.X * PanelAnchors.Minimum.X,
				ViewportSize.Y * PanelAnchors.Minimum.Y);
			// [v1.29.0] 고정 Canvas Slot에서 Right/Bottom이 소유하는 실제 Panel Width/Height입니다.
			const FVector2D PanelSize(PanelOffsets.Right, PanelOffsets.Bottom);
			TestTrue(
				FString::Printf(TEXT("UI-P0-10 %s positive size"), *ProductionPanelSlotNames[PanelSlotIndex].ToString()),
				PanelSize.X > 0.0f && PanelSize.Y > 0.0f);

			// [v1.29.0] Anchor + PixelOffset - Alignment 보정을 적용한 현재 Panel의 화면 좌상단입니다.
			const FVector2D PanelMinimum = AnchorPosition
				+ FVector2D(PanelOffsets.Left, PanelOffsets.Top)
				- (PanelSize * PanelAlignment);
			// [v1.29.0] 현재 Panel의 화면 우하단입니다.
			const FVector2D PanelMaximum = PanelMinimum + PanelSize;
			// [v1.29.0] 부동소수 계산 오차만 허용하면서 전체 Panel이 현재 Viewport 안에 들어오는지 나타냅니다.
			const bool bPanelInsideViewport = PanelMinimum.X >= -0.5f
				&& PanelMinimum.Y >= -0.5f
				&& PanelMaximum.X <= ViewportSize.X + 0.5f
				&& PanelMaximum.Y <= ViewportSize.Y + 0.5f;
			TestTrue(
				FString::Printf(
										TEXT("UI-P0-10 %s physical %.0fx%.0f / logical %.1fx%.1f 화면 내부"),
					*ProductionPanelSlotNames[PanelSlotIndex].ToString(),
					PhysicalViewportSize.X,
					PhysicalViewportSize.Y,
					ViewportSize.X,
					ViewportSize.Y),
				bPanelInsideViewport);

			ResolvedPanelRects.Add(FVector4(PanelMinimum.X, PanelMinimum.Y, PanelMaximum.X, PanelMaximum.Y));
		}

		// [v1.29.0] 현재 Viewport에서 Panel pairwise overlap을 검사할 첫 번째 Rect 인덱스입니다.
		int32 FirstRectIndex = 0;
		for (; FirstRectIndex < ResolvedPanelRects.Num(); ++FirstRectIndex)
		{
			// [v1.29.0] 현재 첫 번째 Panel과 비교할 뒤쪽 Rect 인덱스입니다.
			int32 SecondRectIndex = FirstRectIndex + 1;
			for (; SecondRectIndex < ResolvedPanelRects.Num(); ++SecondRectIndex)
			{
				// [v1.29.0] pairwise overlap 판정의 첫 번째 Root Panel Rect입니다.
				const FVector4& FirstRect = ResolvedPanelRects[FirstRectIndex];
				// [v1.29.0] pairwise overlap 판정의 두 번째 Root Panel Rect입니다.
				const FVector4& SecondRect = ResolvedPanelRects[SecondRectIndex];
				// [v1.29.0] 경계선 접촉은 허용하고 실제 양의 면적이 겹칠 때만 True가 되는 Root Panel overlap입니다.
				const bool bPanelsOverlap = FirstRect.X < SecondRect.Z
					&& FirstRect.Z > SecondRect.X
					&& FirstRect.Y < SecondRect.W
					&& FirstRect.W > SecondRect.Y;
				TestFalse(
					FString::Printf(
												TEXT("UI-P0-10 physical %.0fx%.0f %s / %s Root Panel 비겹침"),
						PhysicalViewportSize.X,
						PhysicalViewportSize.Y,
						*ProductionPanelSlotNames[FirstRectIndex].ToString(),
						*ProductionPanelSlotNames[SecondRectIndex].ToString()),
					bPanelsOverlap);
			}
		}
	}
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
