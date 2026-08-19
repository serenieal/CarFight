// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.33.0
// Date: 2026-08-05
// Description: VehicleDebug Panel용 C++ 부모 위젯 클래스 구현입니다.
// Changelog:
// - v1.33.0: 동적 Navigation에 선택 대상 전용 섹션을 추가해 TargetSelect 표시 정보와 대상 방어·내구도 상태를 매 프레임 갱신.
// - v1.32.0: Weapon 섹션에 VehicleDefenseComp 준비·Fallback·현재 Shield·6방향 Armor 상태와 마지막 전체 피해 결과를 추가.
// - v1.31.0: 탄종 독립 터렛 레티클 유효성, 월드 위치와 비교 거리를 표시하고 기존 Weapon Preview 행을 Legacy로 구분.
// - v1.30.0: Weapon Aim Solution 하위 섹션에 Weapon Reticle Mode 표시 행을 추가.
// - v1.29.0: Weapon Aim Solution 하위 섹션에 직선 Weapon Preview 유효성, Blocking Hit, 월드 위치와 거리를 추가.
// - v1.28.0: Damage HitContext 섹션에 직접 피해 적용 결과, 체력 변화와 파괴 전환 요약을 추가.
// - v1.27.0: Damage HitContext 섹션에 피격 컴포넌트 이름을 독립 항목으로 표시.
// - v1.26.0: Aim 섹션에 Weapon Aim Solution 표시를 추가하고 신규 발사 거부 사유 표시명을 한국어로 변환.
// - v1.25.0: Weapon 섹션에 EquipmentPresetData 지정 여부 / ID / 호환성 / 요약 하위 섹션을 추가.
// - v1.24.0: Weapon 섹션에 마지막 Damage HitContext Debug 표시를 추가.
// - v1.23.0: DamageData 표시 설명을 ProjectileData 단일 소유 정책에 맞게 정리.
// - v1.22.0: Weapon 섹션에 DamageData 지정 여부 / 피해 ID / 해석 경로 / 요약 표시를 추가.
// - v1.21.0: Weapon 섹션에 터렛 조준 현재/목표 Yaw/Pitch와 안정화 상태 표시를 추가.
// - v1.20.0: Weapon 섹션의 터렛 시각 하위 섹션에 Base 메쉬 표시를 추가.
// - v1.19.0: Weapon 섹션의 터렛 시각 하위 섹션에 TurretMountData 지정 / ID / 요약 표시를 추가.
// - v1.18.0: Weapon 섹션에 터렛 시각 장착 하위 섹션을 추가.
// - v1.17.0: Weapon 섹션의 Projectile Pool 하위 섹션에 마지막 반환 요약 표시를 추가.
// - v1.16.0: Weapon 섹션의 발사 쿨다운 초 표시를 분당 발사속도 표시로 전환.
// - v1.15.0: Weapon 섹션에 Projectile Pool 전체 / 활성 / 비활성 수 표시를 추가.
// - v1.14.0: Projectile 준비 표시를 실제 Pool Acquire 경로 기준으로 갱신.
// - v1.13.0: Weapon 섹션의 ProjectileData 표시를 Projectile Actor 스폰 준비 상태와 Dummy HitScan 유지 기준으로 확장.
// - v1.12.0: Weapon 섹션에 활성 ProjectileData 지정 여부, ID, 요약 표시를 추가.
// - v1.11.0: Weapon 섹션에 활성 WeaponData 기반 Trace 사거리와 쿨다운 런타임 표시를 추가.
// - v1.10.0: Weapon 섹션에 활성 WeaponData ID, 호환성, 요약 하위 섹션을 추가.
// - v1.9.0: Weapon 섹션을 추가해 WeaponComp 런타임 준비, 활성 프로파일, 마지막 FireOrigin을 표시.
// - v1.8.5: ThrottleScale 런타임 요약 키를 패널 표시용 한국어 문구로 변환.
// - v1.8.4: VehicleMovement 런타임 setter 요약 키를 패널 표시용 한국어 문구로 변환.
// - v1.8.3: Aim Debug ViewData의 발사 검증/시각 상태 참조명을 FireValidationState / AimVisualState로 교체.
// - v1.8.2: Aim 검증 하위 섹션 표시명을 서버 조준에서 로컬 발사 검증 기준으로 변경.
// - v1.8.1: 싱글플레이 기준에 맞춰 Aim 디버그 패널의 복제 시각 표시 문구를 Aim 시각 표시로 변경.
// Migration:
// - Target 섹션은 기존 동적 Section 렌더링 경로를 사용하므로 WBP 에셋 수정 없이 새 Navigation 항목과 하위 방어·내구도 정보를 표시한다.
// - 터렛 레티클 검증은 신규 Turret Reticle 세 행을 사용하며 기존 Weapon Preview 행은 과거 구현 확인용 Legacy Debug로만 해석한다.
// - Weapon Reticle Mode 표시는 Aim Solution 결과만 읽으며 Debug Panel에서 무기 모드 판정이나 Trace를 다시 수행하지 않는다.
// - Weapon Preview 표시는 AimComp에 캐시된 Weapon Aim Solution 값을 읽는 Debug 전용 표시이며 Trace를 다시 계산하지 않는다.
// - 피격 컴포넌트 이름 표시는 기존 DamageHitContext 값을 별도 행으로 노출할 뿐 충돌 판정과 피해 처리는 변경하지 않는다.
// - Weapon Aim Solution 표시는 Debug 전용이며 조준/발사 계산은 변경하지 않는다.
// - EquipmentPresetData 표시는 Debug 전용이며 FireOrigin / Projectile / Damage 판정을 변경하지 않는다.
// - Base 메쉬 표시는 Debug 전용이며 FireOrigin / Projectile / Damage 판정을 변경하지 않는다.
// - TurretMountData 표시는 Debug 전용이며 FireOrigin / Projectile / Damage 판정을 변경하지 않는다.
// - 터렛 시각 장착 상태는 Debug 표시 전용이며 FireOrigin / Projectile / Damage 판정을 변경하지 않는다.
// - 터렛 조준 상태 표시는 Debug 전용이며 FireOrigin / Projectile / Damage 판정을 변경하지 않는다.
// - DamageData 표시는 Debug 전용이며 실제 HP / 모듈 피해 적용은 수행하지 않는다.
// - DamageData 미지정은 ProjectileData.DamageProfileId fallback 또는 ProjectileData 미연결 상태로 표시하며 발사 가능 여부를 판정하지 않는다.
// - Damage HitContext 표시는 마지막 HitScan / Projectile 결과 확인 전용이며 실제 HP / 모듈 피해 적용은 수행하지 않는다.
// - ProjectileData 미지정 또는 ProjectileActorClass 미지정은 Dummy HitScan fallback 상태로 표시하고 경고 상태로 승격하지 않는다.
// - ProjectileActorClass 지정과 FireMode=Projectile 조건이 맞으면 Projectile Pool 확보 경로 준비 상태로 표시한다.
// - Projectile Pool 카운트는 Pool 재사용 여부를 확인하기 위한 표시 전용 값이며 발사 가능 여부를 판정하지 않는다.
// - Projectile Pool 마지막 반환 요약은 충돌 / 수명 / 반환 검증 표시 전용 값이며 Damage를 적용하지 않는다.
// - 기존 WeaponData 하위 섹션은 유지하고 발사 제한 원본값은 분당 발사속도로 표시한다.
// - 기존 Weapon 섹션 ID와 FireOrigin 하위 섹션은 유지하고 WeaponData 하위 섹션만 추가한다.
// - 기존 Overview / Drive / Input / Camera / Aim / Runtime 섹션은 유지하고 Weapon 섹션만 Navigation에 추가한다.
// - Aim 검증/시각 상태 FieldId는 aim_validation / aim_visual 접두어를 기준으로 사용한다.
// Scope: VehicleDebug Overview / Drive / Input / Camera / Aim / Target / Weapon / Runtime 카테고리를 읽어 Navigation + Selected Section 기반 표시와 기존 fallback 표시를 안정적으로 지원합니다.

#include "UI/CFVehicleDebugPanelWidget.h"

#include "CFVehicleAimComp.h"
#include "Blueprint/WidgetTree.h"
#include "UI/CFVehicleDebugNavItemWidget.h"
#include "UI/CFVehicleDebugSectionWidget.h"

#include "Input/Reply.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Widget.h"
#include "GameFramework/PlayerController.h"

// [v1.0.0] 기본 `라벨: 값` 필드 ViewData를 생성합니다.
FCFVehicleDebugFieldViewData FCFVehicleDebugFieldViewData::MakeLabelValueField(
	const FString& InFieldId,
	const FString& InLabelText,
	const FString& InValueText,
	bool bInIsImportant,
	const FString& InTooltipText)
{
	// [v1.0.0] 생성할 필드 ViewData 결과입니다.
	FCFVehicleDebugFieldViewData FieldViewData;
	FieldViewData.FieldId = InFieldId;
	FieldViewData.LabelText = InLabelText;
	FieldViewData.ValueText = InValueText;
	FieldViewData.TooltipText = InTooltipText;
	FieldViewData.FieldStyle = bInIsImportant ? ECFVehicleDebugFieldStyle::Important : ECFVehicleDebugFieldStyle::Default;
	FieldViewData.bIsImportant = bInIsImportant;
	FieldViewData.bIsVisible = true;
	return FieldViewData;
}

// [v1.0.0] 여러 줄 값 표시용 필드 ViewData를 생성합니다.
FCFVehicleDebugFieldViewData FCFVehicleDebugFieldViewData::MakeMultilineField(
	const FString& InFieldId,
	const FString& InLabelText,
	const FString& InValueText,
	const FString& InTooltipText)
{
	// [v1.0.0] 생성할 여러 줄 필드 ViewData 결과입니다.
	FCFVehicleDebugFieldViewData FieldViewData;
	FieldViewData.FieldId = InFieldId;
	FieldViewData.LabelText = InLabelText;
	FieldViewData.ValueText = InValueText;
	FieldViewData.TooltipText = InTooltipText;
	FieldViewData.FieldStyle = ECFVehicleDebugFieldStyle::Multiline;
	FieldViewData.bIsImportant = false;
	FieldViewData.bIsVisible = true;
	return FieldViewData;
}

// [v1.0.0] 새로운 섹션 ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> FCFVehicleDebugSectionViewData::MakeSection(
	const FString& InSectionId,
	const FString& InTitleText,
	ECFVehicleDebugSectionKind InSectionKind,
	bool bInDefaultExpanded)
{
	// [v1.0.0] 생성할 섹션 ViewData 결과입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> SectionViewData = MakeShared<FCFVehicleDebugSectionViewData>();
	SectionViewData->SectionId = InSectionId;
	SectionViewData->TitleText = InTitleText;
	SectionViewData->SectionKind = InSectionKind;
	SectionViewData->bDefaultExpanded = bInDefaultExpanded;
	SectionViewData->bIsVisible = true;
	return SectionViewData;
}

// [v1.0.0] 현재 섹션에 필드 하나를 추가합니다.
void FCFVehicleDebugSectionViewData::AddField(const FCFVehicleDebugFieldViewData& InFieldViewData)
{
	FieldArray.Add(InFieldViewData);
}

// [v1.0.0] 현재 섹션에 하위 섹션 하나를 추가합니다.
void FCFVehicleDebugSectionViewData::AddChildSection(const TSharedPtr<FCFVehicleDebugSectionViewData>& InChildSectionViewData)
{
	if (InChildSectionViewData.IsValid())
	{
		ChildSectionArray.Add(InChildSectionViewData);
	}
}

// [v1.0.0] 현재 Panel ViewData를 초기화합니다.
void FCFVehicleDebugPanelViewData::Reset()
{
	PanelId.Reset();
	PanelTitleText.Reset();
	GeneratedFrameNumber = 0;
	GeneratedTimeSeconds = 0.0;
	TopLevelSectionArray.Reset();
}

// [v1.0.0] 최상위 섹션 하나를 추가합니다.
void FCFVehicleDebugPanelViewData::AddTopLevelSection(const TSharedPtr<FCFVehicleDebugSectionViewData>& InSectionViewData)
{
	if (InSectionViewData.IsValid())
	{
		TopLevelSectionArray.Add(InSectionViewData);
	}
}

void UCFVehicleDebugPanelWidget::SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef)
{
	// [v1.0.0] Panel이 읽을 차량 Pawn 참조를 저장합니다.
	VehiclePawnRef = InVehiclePawnRef;

	RefreshFromPawn();
	UpdatePanelVisibility();
}

void UCFVehicleDebugPanelWidget::RefreshFromPawn()
{
	// [v1.0.0] 유효한 Pawn이 없으면 Panel을 숨기고 갱신을 중단합니다.
	if (!IsValid(VehiclePawnRef))
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// [v1.33.0] 같은 프레임의 모든 카테고리를 일관되게 표시하고 대상 컴포넌트 검색을 반복하지 않도록 전체 Snapshot을 한 번만 읽습니다.
	const FCFVehicleDebugSnapshot LatestDebugSnapshot = VehiclePawnRef->GetVehicleDebugSnapshot();

	// [v1.33.0] 한 번 읽은 Snapshot의 각 카테고리를 현재 Panel 캐시에 저장합니다.
	CachedOverview = LatestDebugSnapshot.Overview;
	CachedDrive = LatestDebugSnapshot.Drive;
	CachedInput = LatestDebugSnapshot.Input;
	CachedCamera = LatestDebugSnapshot.Camera;
	CachedAim = LatestDebugSnapshot.Aim;
	CachedTarget = LatestDebugSnapshot.Target;
	CachedWeapon = LatestDebugSnapshot.Weapon;
	CachedRuntime = LatestDebugSnapshot.Runtime;

	// [v1.5.0] 현재 Snapshot 기준 최신 Panel ViewData를 생성해 캐시에 저장합니다.
	CachedPanelViewData = BuildVehicleDebugPanelViewData();

	// [v1.7.0] 현재 선택된 SectionId를 유효한 최상위 Section 기준으로 보정합니다.
	ResolveSelectedSectionId();

	// [v1.7.0] Navigation 레이아웃이 준비된 경우 Navigation 항목 표시를 갱신합니다.
	RefreshNavigationItems();

	// [v1.7.0] 선택 Section 레이아웃이 준비된 경우 선택된 Section 하나만 갱신합니다.
	if (IsSelectedSectionLayoutReady())
	{
		RefreshSelectedSectionWidget();
	}
	else if (ShouldUseLegacyFullSectionRendering())
	{
		RefreshDynamicSectionWidgets();
	}
	else
	{
		ApplyVehicleDebugOverview(CachedOverview);
		ApplyVehicleDebugDrive(CachedDrive);
		ApplyVehicleDebugInput(CachedInput);
		ApplyVehicleDebugRuntime(CachedRuntime);
		UpdateCategoryHeaderTexts();
	}
}

void UCFVehicleDebugPanelWidget::SelectSectionById(const FString& InSectionId)
{
	// [v1.7.0] 빈 SectionId는 선택 요청으로 처리하지 않습니다.
	if (InSectionId.IsEmpty())
	{
		return;
	}

	SelectedSectionId = InSectionId;
	ResolveSelectedSectionId();
	RefreshNavigationItems();

	if (IsSelectedSectionLayoutReady())
	{
		RefreshSelectedSectionWidget();
	}
}

FString UCFVehicleDebugPanelWidget::GetSelectedSectionId() const
{
	return SelectedSectionId;
}

void UCFVehicleDebugPanelWidget::ApplyVehicleDebugOverview(const FCFVehicleDebugOverview& InOverview)
{
	// [v1.0.0] Overview 섹션 본문 문자열입니다.
	const FString OverviewBodyText = BuildOverviewBodyText(InOverview);

	// [v1.4.0] Overview Last Transition 하위 섹션 본문 문자열입니다.
	const FString OverviewLastTransitionBodyText = BuildOverviewLastTransitionBodyText(InOverview);

	// [v1.0.0] Overview 섹션 본문 텍스트를 반영합니다.
	if (Text_OverviewBody)
	{
		Text_OverviewBody->SetText(FText::FromString(OverviewBodyText));
	}

	// [v1.4.0] Overview Last Transition 본문 텍스트를 반영합니다.
	if (Text_OverviewLastTransitionBody)
	{
		Text_OverviewLastTransitionBody->SetText(FText::FromString(OverviewLastTransitionBodyText));
	}
}

void UCFVehicleDebugPanelWidget::ApplyVehicleDebugDrive(const FCFVehicleDebugDrive& InDrive)
{
	// [v1.0.0] Drive 섹션 본문 문자열입니다.
	const FString DriveBodyText = BuildDriveBodyText(InDrive);

	// [v1.4.0] Drive Transition 하위 섹션 본문 문자열입니다.
	const FString DriveTransitionBodyText = BuildDriveTransitionBodyText(InDrive);

	// [v1.0.0] Drive 섹션 본문 텍스트를 반영합니다.
	if (Text_DriveBody)
	{
		Text_DriveBody->SetText(FText::FromString(DriveBodyText));
	}

	// [v1.4.0] Drive Transition 본문 텍스트를 반영합니다.
	if (Text_DriveTransitionBody)
	{
		Text_DriveTransitionBody->SetText(FText::FromString(DriveTransitionBodyText));
	}
}

void UCFVehicleDebugPanelWidget::ApplyVehicleDebugInput(const FCFVehicleDebugInput& InInput)
{
	// [v1.0.0] Input 섹션 본문 문자열입니다.
	const FString InputBodyText = BuildInputBodyText(InInput);

	// [v1.0.0] Input 섹션 본문 텍스트를 반영합니다.
	if (Text_InputBody)
	{
		Text_InputBody->SetText(FText::FromString(InputBodyText));
	}
}

void UCFVehicleDebugPanelWidget::ApplyVehicleDebugRuntime(const FCFVehicleDebugRuntime& InRuntime)
{
	// [v1.0.0] Runtime 섹션 본문 문자열입니다.
	const FString RuntimeBodyText = BuildRuntimeBodyText(InRuntime);

	// [v1.3.0] Runtime Summary 하위 섹션 본문 문자열입니다.
	const FString RuntimeSummaryBodyText = BuildRuntimeSummaryBodyText(InRuntime);

	// [v1.3.0] Last Init 하위 섹션 본문 문자열입니다.
	const FString RuntimeLastInitBodyText = BuildRuntimeLastInitBodyText(InRuntime);

	// [v1.3.0] Last Validation 하위 섹션 본문 문자열입니다.
	const FString RuntimeLastValidationBodyText = BuildRuntimeLastValidationBodyText(InRuntime);

	// [v1.0.0] Runtime 섹션 본문 텍스트를 반영합니다.
	if (Text_RuntimeBody)
	{
		Text_RuntimeBody->SetText(FText::FromString(RuntimeBodyText));
	}

	// [v1.3.0] Runtime Summary 본문 텍스트를 반영합니다.
	if (Text_RuntimeSummaryBody)
	{
		Text_RuntimeSummaryBody->SetText(FText::FromString(RuntimeSummaryBodyText));
	}

	// [v1.3.0] Last Init 본문 텍스트를 반영합니다.
	if (Text_RuntimeLastInitBody)
	{
		Text_RuntimeLastInitBody->SetText(FText::FromString(RuntimeLastInitBodyText));
	}

	// [v1.3.0] Last Validation 본문 텍스트를 반영합니다.
	if (Text_RuntimeLastValidationBody)
	{
		Text_RuntimeLastValidationBody->SetText(FText::FromString(RuntimeLastValidationBodyText));
	}
}

void UCFVehicleDebugPanelWidget::UpdatePanelVisibility()
{
	// [v1.0.0] 유효한 Pawn이 없으면 Panel을 숨깁니다.
	if (!IsValid(VehiclePawnRef))
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	// [v1.0.0] Pawn의 Panel 표시 정책을 계산한 결과입니다.
	const bool bShouldShowPanel = VehiclePawnRef->ShouldShowVehicleDebugPanel();

	// [v1.3.3] Panel 루트가 NativeOnPreviewMouseButtonDown에서 바깥 클릭을 감지할 수 있도록 Visible을 사용합니다.
	SetVisibility(bShouldShowPanel ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);

	// [v1.2.0] Panel이 표시될 때 자동 상호작용 진입 옵션이 켜져 있으면 상호작용 모드를 켭니다.
	if (bShouldShowPanel && bAutoEnterInteractionModeWhenVisible && !bIsPanelInteractionModeActive)
	{
		EnterPanelInteractionMode();
	}

	// [v1.2.0] Panel이 숨겨지면 상호작용 모드도 함께 종료합니다.
	if (!bShouldShowPanel && bIsPanelInteractionModeActive)
	{
		ExitPanelInteractionMode();
	}
}

void UCFVehicleDebugPanelWidget::ToggleOverviewExpanded()
{
	// [v1.1.0] Overview 카테고리 펼침 상태를 반전합니다.
	bIsOverviewExpanded = !bIsOverviewExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleOverviewLastTransitionExpanded()
{
	// [v1.4.0] Overview Last Transition 하위 섹션 펼침 상태를 반전합니다.
	bIsOverviewLastTransitionExpanded = !bIsOverviewLastTransitionExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleDriveExpanded()
{
	// [v1.1.0] Drive 카테고리 펼침 상태를 반전합니다.
	bIsDriveExpanded = !bIsDriveExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleDriveTransitionExpanded()
{
	// [v1.4.0] Drive Transition 하위 섹션 펼침 상태를 반전합니다.
	bIsDriveTransitionExpanded = !bIsDriveTransitionExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleInputExpanded()
{
	// [v1.1.0] Input 카테고리 펼침 상태를 반전합니다.
	bIsInputExpanded = !bIsInputExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleRuntimeExpanded()
{
	// [v1.1.0] Runtime 카테고리 펼침 상태를 반전합니다.
	bIsRuntimeExpanded = !bIsRuntimeExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleRuntimeSummaryExpanded()
{
	// [v1.3.0] Runtime Summary 하위 섹션 펼침 상태를 반전합니다.
	bIsRuntimeSummaryExpanded = !bIsRuntimeSummaryExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleRuntimeLastInitExpanded()
{
	// [v1.3.0] Last Init 하위 섹션 펼침 상태를 반전합니다.
	bIsRuntimeLastInitExpanded = !bIsRuntimeLastInitExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::ToggleRuntimeLastValidationExpanded()
{
	// [v1.3.0] Last Validation 하위 섹션 펼침 상태를 반전합니다.
	bIsRuntimeLastValidationExpanded = !bIsRuntimeLastValidationExpanded;

	UpdateCategoryBodyVisibility();
	UpdateCategoryHeaderTexts();
}

void UCFVehicleDebugPanelWidget::EnterPanelInteractionMode()
{
	// [v1.2.0] Panel 상호작용 모드를 활성화합니다.
	bIsPanelInteractionModeActive = true;

	// [v1.2.0] 상호작용 입력을 적용할 PlayerController입니다.
	APlayerController* PlayerController = ResolveOwningPlayerController();

	if (!PlayerController)
	{
		return;
	}

	// [v1.2.0] Panel 위젯에 포커스를 주는 Game and UI 입력 모드입니다.
	FInputModeGameAndUI GameAndUiInputMode;
	GameAndUiInputMode.SetWidgetToFocus(TakeWidget());
	GameAndUiInputMode.SetHideCursorDuringCapture(false);
	GameAndUiInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetShowMouseCursor(true);
	PlayerController->SetInputMode(GameAndUiInputMode);
}

void UCFVehicleDebugPanelWidget::ExitPanelInteractionMode()
{
	// [v1.2.0] Panel 상호작용 모드를 비활성화합니다.
	bIsPanelInteractionModeActive = false;

	// [v1.2.0] 상호작용 입력을 적용할 PlayerController입니다.
	APlayerController* PlayerController = ResolveOwningPlayerController();

	if (!PlayerController)
	{
		return;
	}

	// [v1.2.0] 게임 입력 전용 입력 모드입니다.
	FInputModeGameOnly GameOnlyInputMode;

	PlayerController->SetShowMouseCursor(false);
	PlayerController->SetInputMode(GameOnlyInputMode);
}

// [v1.5.0] 현재 Snapshot 기준 Panel ViewData 캐시를 반환합니다.
const FCFVehicleDebugPanelViewData& UCFVehicleDebugPanelWidget::GetCachedPanelViewData() const
{
	return CachedPanelViewData;
}

void UCFVehicleDebugPanelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// [v1.2.3] 디자이너에서 변수 체크가 빠졌더라도 이름 기준으로 위젯 참조를 다시 찾습니다.
	ResolveWidgetReferences();

	// [v1.2.6] 카테고리 헤더 버튼 입력은 WBP 이벤트 그래프에서 Toggle 함수 호출로 연결하는 것을 기본안으로 사용합니다.

	RefreshFromPawn();
	if (!IsDynamicSectionLayoutReady())
	{
		UpdateCategoryBodyVisibility();
	}
	UpdateCategoryHeaderTexts();
	UpdatePanelVisibility();
}

void UCFVehicleDebugPanelWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	// [v1.0.0] 자동 갱신이 꺼져 있으면 매 프레임 갱신을 건너뜁니다.
	if (!bAutoRefreshEveryTick)
	{
		return;
	}

	RefreshFromPawn();
	if (!IsDynamicSectionLayoutReady())
	{
		UpdateCategoryBodyVisibility();
	}
	UpdatePanelVisibility();
}

FReply UCFVehicleDebugPanelWidget::NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// [v1.3.2] 현재 상호작용 모드가 아니면 기본 입력 흐름을 그대로 사용합니다.
	if (!bIsPanelInteractionModeActive)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	// [v1.3.2] 실제 Panel 루트 위젯이 없으면 기본 입력 흐름을 그대로 사용합니다.
	if (!Border_RootPanel)
	{
		return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
	}

	// [v1.3.2] 현재 마우스 클릭 위치의 스크린 좌표입니다.
	const FVector2D ScreenSpacePosition = InMouseEvent.GetScreenSpacePosition();

	// [v1.3.2] 현재 클릭이 실제 Panel 본체 내부인지 여부입니다.
	const bool bIsClickInsidePanel = Border_RootPanel->GetCachedGeometry().IsUnderLocation(ScreenSpacePosition);

	// [v1.3.2] Panel 바깥 클릭이면 상호작용 모드를 종료하고 이번 클릭을 처리 완료로 반환합니다.
	if (!bIsClickInsidePanel)
	{
		ExitPanelInteractionMode();
		return FReply::Handled();
	}

	return Super::NativeOnPreviewMouseButtonDown(InGeometry, InMouseEvent);
}

FString UCFVehicleDebugPanelWidget::ConvertEnumValueToDisplayString(const TCHAR* EnumValueString) const
{
	// [v1.0.0] Panel에 표시할 enum 문자열 원본입니다.
	FString DisplayString = EnumValueString ? EnumValueString : TEXT("");

	// [v1.0.0] 마지막 `::` 뒤 값만 남기기 위한 구분 인덱스입니다.
	int32 ValueSeparatorIndex = INDEX_NONE;

	// [v1.0.0] enum 풀네임에서 실제 값 이름 시작 위치를 찾습니다.
	if (DisplayString.FindLastChar(TEXT(':'), ValueSeparatorIndex) && ValueSeparatorIndex + 1 < DisplayString.Len())
	{
		DisplayString = DisplayString.Mid(ValueSeparatorIndex + 1);
	}

	if (DisplayString == TEXT("TurretAligning"))
	{
		return TEXT("터렛 정렬 중");
	}

	if (DisplayString == TEXT("WeaponNotAligned"))
	{
		return TEXT("무기 정렬 실패");
	}

	if (DisplayString == TEXT("MuzzleBlocked"))
	{
		return TEXT("총구 막힘");
	}

	return DisplayString;
}

FString UCFVehicleDebugPanelWidget::BuildOverviewBodyText(const FCFVehicleDebugOverview& InOverview) const
{
	// [v1.0.0] Runtime 준비 상태 표시 문자열입니다.
	const FString RuntimeReadyText = InOverview.bRuntimeReady ? TEXT("예") : TEXT("아니오");

	// [v1.0.0] Overview 본문 문자열입니다.
	const FString OverviewBodyText = FString::Printf(
		TEXT("준비: %s\n상태: %s\n속도: %.1f km/h\n전진 속도: %.1f km/h\n입력 모드: %s\n입력 소유자: %s"),
		*RuntimeReadyText,
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.CurrentDriveState)),
		InOverview.SpeedKmh,
		InOverview.ForwardSpeedKmh,
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.DeviceMode)),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.InputOwner)));

	return OverviewBodyText;
}

FString UCFVehicleDebugPanelWidget::BuildOverviewLastTransitionBodyText(const FCFVehicleDebugOverview& InOverview) const
{
	// [v1.4.0] Panel 표시용 Overview Last Transition 문자열입니다.
	const FString OverviewLastTransitionPanelText = FormatLongSummaryForPanel(InOverview.LastTransitionShortText, TEXT("주행 상태 전이: "));

		return OverviewLastTransitionPanelText;
}

FString UCFVehicleDebugPanelWidget::BuildDriveBodyText(const FCFVehicleDebugDrive& InDrive) const
{
	// [v1.0.0] Drive 상태 변경 여부 표시 문자열입니다.
	const FString DriveChangedText = InDrive.bDriveStateChangedThisFrame ? TEXT("예") : TEXT("아니오");

	// [v1.0.0] Handbrake 표시 문자열입니다.
	const FString HandbrakeText = InDrive.bHandbrake ? TEXT("켜짐") : TEXT("꺼짐");

	// [v1.0.0] Drive 본문 문자열입니다.
	const FString DriveBodyText = FString::Printf(
		TEXT("현재 상태: %s\n이전 상태: %s\n이번 프레임 변경: %s\n속도: %.1f km/h\n전진 속도: %.1f km/h\n스로틀: %.2f\n브레이크: %.2f\n조향: %.2f\n핸드브레이크: %s"),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InDrive.CurrentDriveState)),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InDrive.PreviousDriveState)),
		*DriveChangedText,
		InDrive.SpeedKmh,
		InDrive.ForwardSpeedKmh,
		InDrive.Throttle,
		InDrive.Brake,
		InDrive.Steering,
		*HandbrakeText);

	return DriveBodyText;
}

FString UCFVehicleDebugPanelWidget::BuildDriveTransitionBodyText(const FCFVehicleDebugDrive& InDrive) const
{
	// [v1.4.0] Panel 표시용 Drive Transition 문자열입니다.
	const FString DriveTransitionPanelText = FormatLongSummaryForPanel(InDrive.DriveStateTransitionSummary, TEXT("주행 상태 전이: "));

	return DriveTransitionPanelText;
}

FString UCFVehicleDebugPanelWidget::BuildInputBodyText(const FCFVehicleDebugInput& InInput) const
{
	// [v1.0.0] 검은 영역 유지 정책 사용 여부 표시 문자열입니다.
	const FString BlackZoneHoldText = InInput.bUsedBlackZoneHold ? TEXT("예") : TEXT("아니오");

	// [v1.0.0] Input 본문 문자열입니다.
	const FString InputBodyText = FString::Printf(
		TEXT("장치 모드: %s\n입력 소유자: %s\n이동 영역: %s\n이동 의도: %s\n이동 원본값: (%.2f, %.2f)\n이동 크기: %.2f\n이동 각도: %.1f\n블랙존 유지: %s"),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.DeviceMode)),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.InputOwner)),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.MoveZone)),
		*ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.MoveIntent)),
		InInput.MoveRaw.X,
		InInput.MoveRaw.Y,
		InInput.MoveMagnitude,
		InInput.MoveAngle,
		*BlackZoneHoldText);

	return InputBodyText;
}

FString UCFVehicleDebugPanelWidget::BuildRuntimeBodyText(const FCFVehicleDebugRuntime& InRuntime) const
{
	// [v1.0.0] Runtime 준비 상태 표시 문자열입니다.
	const FString RuntimeReadyText = InRuntime.bRuntimeReady ? TEXT("예") : TEXT("아니오");

	// [v1.0.0] Drive 컴포넌트 존재 여부 표시 문자열입니다.
	const FString HasDriveComponentText = InRuntime.bHasDriveComponent ? TEXT("예") : TEXT("아니오");

	// [v1.0.0] WheelSync 컴포넌트 존재 여부 표시 문자열입니다.
	const FString HasWheelSyncComponentText = InRuntime.bHasWheelSyncComponent ? TEXT("예") : TEXT("아니오");

	// [v1.0.0] Runtime 본문 문자열입니다.
	const FString RuntimeBodyText = FString::Printf(
		TEXT("준비: %s\n주행 컴포넌트: %s\n휠 동기화 컴포넌트: %s"),
		*RuntimeReadyText,
		*HasDriveComponentText,
		*HasWheelSyncComponentText);

	return RuntimeBodyText;
}

FString UCFVehicleDebugPanelWidget::BuildRuntimeSummaryBodyText(const FCFVehicleDebugRuntime& InRuntime) const
{
	// [v1.3.0] Panel 표시용 Runtime Summary 문자열입니다.
	const FString RuntimeSummaryPanelText = FormatLongSummaryForPanel(InRuntime.RuntimeSummary, TEXT("차량 런타임: "));

	return RuntimeSummaryPanelText;
}

FString UCFVehicleDebugPanelWidget::BuildRuntimeLastInitBodyText(const FCFVehicleDebugRuntime& InRuntime) const
{
	// [v1.3.0] Panel 표시용 Last Init 문자열입니다.
	const FString RuntimeLastInitPanelText = FormatLongSummaryForPanel(InRuntime.LastInitAttemptSummary, TEXT("차량 런타임: "));

	return RuntimeLastInitPanelText;
}

FString UCFVehicleDebugPanelWidget::BuildRuntimeLastValidationBodyText(const FCFVehicleDebugRuntime& InRuntime) const
{
	// [v1.3.0] Panel 표시용 Last Validation 문자열입니다.
	const FString RuntimeLastValidationPanelText = FormatLongSummaryForPanel(InRuntime.LastValidationSummary, TEXT("차량 런타임: "));

	return RuntimeLastValidationPanelText;
}

FString UCFVehicleDebugPanelWidget::FormatLongSummaryForPanel(const FString& InSummaryText, const FString& InKnownPrefix) const
{
	// [v1.1.0] Panel 표시용으로 정리할 원본 요약 문자열입니다.
	FString FormattedSummaryText = InSummaryText;

	// [v1.8.1] 기존 C++ 원본 요약 접두어와 한국어 접두어를 모두 패널에서는 제거합니다.
	const TArray<FString> KnownPrefixArray = {
		InKnownPrefix,
		TEXT("VehicleRuntime: "),
		TEXT("VehicleLayout: "),
		TEXT("DriveStateTransition: ")
	};

	for (const FString& KnownPrefix : KnownPrefixArray)
	{
		if (!KnownPrefix.IsEmpty() && FormattedSummaryText.StartsWith(KnownPrefix))
		{
			FormattedSummaryText.RightChopInline(KnownPrefix.Len(), EAllowShrinking::No);
			break;
		}
	}

	// [v1.8.1] 원본 디버그 키는 유지하되, Panel 표시용 문자열만 한국어로 변환합니다.
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("Data="), TEXT("데이터="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("Drive="), TEXT("주행="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("WheelSync="), TEXT("휠 동기화="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("Ready="), TEXT("준비="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("MovementProfile="), TEXT("이동 프로필="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("MaxTorque="), TEXT("최대 토크="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("RuntimeTorque="), TEXT("런타임 토크="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("MaxRPM="), TEXT("최대 RPM="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("ConfigMaxRPM="), TEXT("설정 최대 RPM="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("ThrottleScale="), TEXT("스로틀 배율="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("Drag="), TEXT("공기 저항="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("Downforce="), TEXT("다운포스="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("Differential="), TEXT("디퍼렌셜="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("SteeringType="), TEXT("조향 타입="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("RuntimeSetters="), TEXT("런타임 적용="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("WheelPhysicsOverrides="), TEXT("휠 물리 오버라이드="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("RuntimeWheelApply="), TEXT("런타임 휠 적용="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("FrontWheelClass="), TEXT("전륜 클래스="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("RearWheelClass="), TEXT("후륜 클래스="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("FrontSteer="), TEXT("전륜 조향각="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("FrictionF/R="), TEXT("마찰 전/후="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("SpringF/R="), TEXT("스프링 전/후="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("FrontOffset="), TEXT("전륜 오프셋="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("RearOffset="), TEXT("후륜 오프셋="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("WheelVisual ExpectedWheelCount="), TEXT("휠 비주얼 예상 휠 수="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("ExpectedWheelCount="), TEXT("예상 휠 수="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("FrontWheelCount="), TEXT("전륜 수="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("FrontWheelCountForSteering="), TEXT("조향 전륜 수="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("WheelSyncBuild="), TEXT("휠 동기화 빌드="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("WheelSyncRuntime="), TEXT("휠 동기화 런타임="));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("WheelSyncValidation:"), TEXT("휠 동기화 검증:"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("WheelSyncInput:"), TEXT("휠 동기화 입력:"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("ManualAnchorLayout=Required"), TEXT("수동 앵커 배치 필요"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("VehicleData is null"), TEXT("VehicleData 없음"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("VehicleDriveComp is null"), TEXT("VehicleDriveComp 없음"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("WheelSyncComp is null"), TEXT("WheelSyncComp 없음"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("VehicleMovementComponent is null"), TEXT("VehicleMovementComponent 없음"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("DriveComp cache failed"), TEXT("DriveComp 캐시 실패"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("Wheel visual update failed"), TEXT("휠 비주얼 갱신 실패"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("InitializeStarted"), TEXT("초기화 시작"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("Present"), TEXT("있음"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("MissingWheelSyncComp"), TEXT("WheelSyncComp 없음"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("Missing"), TEXT("없음"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("True"), TEXT("예"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("False"), TEXT("아니오"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("None"), TEXT("없음"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("during ApplyVehicleMovementConfig."), TEXT("ApplyVehicleMovementConfig 중"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("during ApplyVehicleWheelPhysicsConfig."), TEXT("ApplyVehicleWheelPhysicsConfig 중"));
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT("during UpdateVehicleWheelVisuals."), TEXT("UpdateVehicleWheelVisuals 중"));

	// [v1.1.0] 파이프 구분자를 패널용 줄바꿈으로 바꿉니다.
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT(" | "), TEXT("\n  - "));

	// [v1.1.0] 쉼표 구분자를 패널용 줄바꿈으로 바꿉니다.
	FormattedSummaryText = FormattedSummaryText.Replace(TEXT(", "), TEXT("\n  - "));

	// [v1.1.0] 첫 줄 앞에도 동일한 들여쓰기 스타일을 맞춥니다.
	FormattedSummaryText = FString::Printf(TEXT("  - %s"), *FormattedSummaryText);

	return FormattedSummaryText;
}

void UCFVehicleDebugPanelWidget::ResolveWidgetReferences()
{
	// [v1.2.3] WidgetTree가 없으면 참조 보강을 수행할 수 없습니다.
	if (!WidgetTree)
	{
		return;
	}

	// [v1.2.3] Overview 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_OverviewBody)
	{
		Container_OverviewBody = WidgetTree->FindWidget(TEXT("Container_OverviewBody"));
	}

	// [v1.6.0] 동적 Section 호스트 참조를 이름 기준으로 보강합니다.
	if (!VerticalBox_DynamicSectionHost)
	{
		VerticalBox_DynamicSectionHost = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("VerticalBox_DynamicSectionHost")));
	}

	// [v1.7.0] Navigation 항목 호스트 참조를 이름 기준으로 보강합니다.
	if (!VerticalBox_NavHost)
	{
		VerticalBox_NavHost = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("VerticalBox_NavHost")));
	}

	// [v1.7.0] 선택 Section 호스트 참조를 이름 기준으로 보강합니다.
	if (!VerticalBox_SelectedSectionHost)
	{
		VerticalBox_SelectedSectionHost = Cast<UVerticalBox>(WidgetTree->FindWidget(TEXT("VerticalBox_SelectedSectionHost")));
	}

	// [v1.3.2] 실제 Panel 본체 루트 위젯 참조를 이름 기준으로 보강합니다.
	if (!Border_RootPanel)
	{
		Border_RootPanel = WidgetTree->FindWidget(TEXT("Border_RootPanel"));
	}

	// [v1.2.3] Overview 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_OverviewTitle)
	{
		Text_OverviewTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_OverviewTitle")));
	}

	// [v1.2.3] Overview 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_OverviewBody)
	{
		Text_OverviewBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_OverviewBody")));
	}

	// [v1.4.0] Overview Last Transition 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_OverviewLastTransitionBody)
	{
		Container_OverviewLastTransitionBody = WidgetTree->FindWidget(TEXT("Container_OverviewLastTransitionBody"));
	}

	// [v1.4.0] Overview Last Transition 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_OverviewLastTransitionTitle)
	{
		Text_OverviewLastTransitionTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_OverviewLastTransitionTitle")));
	}

	// [v1.4.0] Overview Last Transition 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_OverviewLastTransitionBody)
	{
		Text_OverviewLastTransitionBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_OverviewLastTransitionBody")));
	}

	// [v1.2.3] Drive 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_DriveBody)
	{
		Container_DriveBody = WidgetTree->FindWidget(TEXT("Container_DriveBody"));
	}

	// [v1.2.3] Drive 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_DriveTitle)
	{
		Text_DriveTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_DriveTitle")));
	}

	// [v1.2.3] Drive 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_DriveBody)
	{
		Text_DriveBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_DriveBody")));
	}

	// [v1.4.0] Drive Transition 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_DriveTransitionBody)
	{
		Container_DriveTransitionBody = WidgetTree->FindWidget(TEXT("Container_DriveTransitionBody"));
	}

	// [v1.4.0] Drive Transition 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_DriveTransitionTitle)
	{
		Text_DriveTransitionTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_DriveTransitionTitle")));
	}

	// [v1.4.0] Drive Transition 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_DriveTransitionBody)
	{
		Text_DriveTransitionBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_DriveTransitionBody")));
	}

	// [v1.2.3] Input 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_InputBody)
	{
		Container_InputBody = WidgetTree->FindWidget(TEXT("Container_InputBody"));
	}

	// [v1.2.3] Input 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_InputTitle)
	{
		Text_InputTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_InputTitle")));
	}

	// [v1.2.3] Input 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_InputBody)
	{
		Text_InputBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_InputBody")));
	}

	// [v1.2.3] Runtime 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_RuntimeBody)
	{
		Container_RuntimeBody = WidgetTree->FindWidget(TEXT("Container_RuntimeBody"));
	}

	// [v1.2.3] Runtime 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeTitle)
	{
		Text_RuntimeTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeTitle")));
	}

	// [v1.2.3] Runtime 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeBody)
	{
		Text_RuntimeBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeBody")));
	}

	// [v1.3.0] Runtime Summary 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_RuntimeSummaryBody)
	{
		Container_RuntimeSummaryBody = WidgetTree->FindWidget(TEXT("Container_RuntimeSummaryBody"));
	}

	// [v1.3.0] Runtime Summary 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeSummaryTitle)
	{
		Text_RuntimeSummaryTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeSummaryTitle")));
	}

	// [v1.3.0] Runtime Summary 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeSummaryBody)
	{
		Text_RuntimeSummaryBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeSummaryBody")));
	}

	// [v1.3.0] Last Init 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_RuntimeLastInitBody)
	{
		Container_RuntimeLastInitBody = WidgetTree->FindWidget(TEXT("Container_RuntimeLastInitBody"));
	}

	// [v1.3.0] Last Init 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeLastInitTitle)
	{
		Text_RuntimeLastInitTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeLastInitTitle")));
	}

	// [v1.3.0] Last Init 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeLastInitBody)
	{
		Text_RuntimeLastInitBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeLastInitBody")));
	}

	// [v1.3.0] Last Validation 본문 컨테이너 참조를 이름 기준으로 보강합니다.
	if (!Container_RuntimeLastValidationBody)
	{
		Container_RuntimeLastValidationBody = WidgetTree->FindWidget(TEXT("Container_RuntimeLastValidationBody"));
	}

	// [v1.3.0] Last Validation 제목 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeLastValidationTitle)
	{
		Text_RuntimeLastValidationTitle = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeLastValidationTitle")));
	}

	// [v1.3.0] Last Validation 본문 텍스트 참조를 이름 기준으로 보강합니다.
	if (!Text_RuntimeLastValidationBody)
	{
		Text_RuntimeLastValidationBody = Cast<UTextBlock>(WidgetTree->FindWidget(TEXT("Text_RuntimeLastValidationBody")));
	}
}

void UCFVehicleDebugPanelWidget::UpdateCategoryBodyVisibility()
{
	// [v1.3.5] 현재 WBP 구조에서는 본문 텍스트가 컨테이너 안에 있으므로 컨테이너 가시성만 제어합니다.
	if (Container_OverviewBody)
	{
		Container_OverviewBody->SetVisibility(bIsOverviewExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.3.5] Drive 본문 컨테이너가 있으면 펼침 상태를 반영합니다.
	if (Container_DriveBody)
	{
		Container_DriveBody->SetVisibility(bIsDriveExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.4.0] Overview Last Transition 하위 본문 컨테이너는 Overview와 자신 둘 다 펼쳐졌을 때만 보입니다.
	if (Container_OverviewLastTransitionBody)
	{
		Container_OverviewLastTransitionBody->SetVisibility((bIsOverviewExpanded && bIsOverviewLastTransitionExpanded) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.4.0] Drive Transition 하위 본문 컨테이너는 Drive와 자신 둘 다 펼쳐졌을 때만 보입니다.
	if (Container_DriveTransitionBody)
	{
		Container_DriveTransitionBody->SetVisibility((bIsDriveExpanded && bIsDriveTransitionExpanded) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.3.5] Input 본문 컨테이너가 있으면 펼침 상태를 반영합니다.
	if (Container_InputBody)
	{
		Container_InputBody->SetVisibility(bIsInputExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.3.5] Runtime 본문 컨테이너가 있으면 펼침 상태를 반영합니다.
	if (Container_RuntimeBody)
	{
		Container_RuntimeBody->SetVisibility(bIsRuntimeExpanded ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.3.5] Runtime Summary 하위 본문 컨테이너는 Runtime과 자신 둘 다 펼쳐졌을 때만 보입니다.
	if (Container_RuntimeSummaryBody)
	{
		Container_RuntimeSummaryBody->SetVisibility((bIsRuntimeExpanded && bIsRuntimeSummaryExpanded) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.3.5] Last Init 하위 본문 컨테이너는 Runtime과 자신 둘 다 펼쳐졌을 때만 보입니다.
	if (Container_RuntimeLastInitBody)
	{
		Container_RuntimeLastInitBody->SetVisibility((bIsRuntimeExpanded && bIsRuntimeLastInitExpanded) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}

	// [v1.3.5] Last Validation 하위 본문 컨테이너는 Runtime과 자신 둘 다 펼쳐졌을 때만 보입니다.
	if (Container_RuntimeLastValidationBody)
	{
		Container_RuntimeLastValidationBody->SetVisibility((bIsRuntimeExpanded && bIsRuntimeLastValidationExpanded) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UCFVehicleDebugPanelWidget::UpdateCategoryHeaderTexts()
{
	// [v1.2.4] Overview 펼침 상태를 제목 문자열에 반영합니다.
	const FString OverviewHeaderText = FString::Printf(TEXT("%s 개요"), bIsOverviewExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.2.4] Drive 펼침 상태를 제목 문자열에 반영합니다.
	const FString DriveHeaderText = FString::Printf(TEXT("%s 주행"), bIsDriveExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.4.0] Overview Last Transition 펼침 상태를 제목 문자열에 반영합니다.
	const FString OverviewLastTransitionHeaderText = FString::Printf(TEXT("%s 최근 전이"), bIsOverviewLastTransitionExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.4.0] Drive Transition 펼침 상태를 제목 문자열에 반영합니다.
	const FString DriveTransitionHeaderText = FString::Printf(TEXT("%s 전이"), bIsDriveTransitionExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.2.4] Input 펼침 상태를 제목 문자열에 반영합니다.
	const FString InputHeaderText = FString::Printf(TEXT("%s 입력"), bIsInputExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.2.4] Runtime 펼침 상태를 제목 문자열에 반영합니다.
	const FString RuntimeHeaderText = FString::Printf(TEXT("%s 런타임"), bIsRuntimeExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.3.0] Runtime Summary 펼침 상태를 제목 문자열에 반영합니다.
	const FString RuntimeSummaryHeaderText = FString::Printf(TEXT("%s 런타임 요약"), bIsRuntimeSummaryExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.3.0] Last Init 펼침 상태를 제목 문자열에 반영합니다.
	const FString RuntimeLastInitHeaderText = FString::Printf(TEXT("%s 최근 초기화"), bIsRuntimeLastInitExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.3.0] Last Validation 펼침 상태를 제목 문자열에 반영합니다.
	const FString RuntimeLastValidationHeaderText = FString::Printf(TEXT("%s 최근 검증"), bIsRuntimeLastValidationExpanded ? TEXT("[-]") : TEXT("[+]"));

	// [v1.2.4] Overview 제목 텍스트를 갱신합니다.
	if (Text_OverviewTitle)
	{
		Text_OverviewTitle->SetText(FText::FromString(OverviewHeaderText));
	}

	// [v1.4.0] Overview Last Transition 제목 텍스트를 갱신합니다.
	if (Text_OverviewLastTransitionTitle)
	{
		Text_OverviewLastTransitionTitle->SetText(FText::FromString(OverviewLastTransitionHeaderText));
	}

	// [v1.2.4] Drive 제목 텍스트를 갱신합니다.
	if (Text_DriveTitle)
	{
		Text_DriveTitle->SetText(FText::FromString(DriveHeaderText));
	}

	// [v1.4.0] Drive Transition 제목 텍스트를 갱신합니다.
	if (Text_DriveTransitionTitle)
	{
		Text_DriveTransitionTitle->SetText(FText::FromString(DriveTransitionHeaderText));
	}

	// [v1.2.4] Input 제목 텍스트를 갱신합니다.
	if (Text_InputTitle)
	{
		Text_InputTitle->SetText(FText::FromString(InputHeaderText));
	}

	// [v1.2.4] Runtime 제목 텍스트를 갱신합니다.
	if (Text_RuntimeTitle)
	{
		Text_RuntimeTitle->SetText(FText::FromString(RuntimeHeaderText));
	}

	// [v1.3.0] Runtime Summary 제목 텍스트를 갱신합니다.
	if (Text_RuntimeSummaryTitle)
	{
		Text_RuntimeSummaryTitle->SetText(FText::FromString(RuntimeSummaryHeaderText));
	}

	// [v1.3.0] Last Init 제목 텍스트를 갱신합니다.
	if (Text_RuntimeLastInitTitle)
	{
		Text_RuntimeLastInitTitle->SetText(FText::FromString(RuntimeLastInitHeaderText));
	}

	// [v1.3.0] Last Validation 제목 텍스트를 갱신합니다.
	if (Text_RuntimeLastValidationTitle)
	{
		Text_RuntimeLastValidationTitle->SetText(FText::FromString(RuntimeLastValidationHeaderText));
	}
}

APlayerController* UCFVehicleDebugPanelWidget::ResolveOwningPlayerController() const
{
	// [v1.2.0] 위젯 소유 플레이어를 우선 사용합니다.
	APlayerController* PlayerController = GetOwningPlayer();

	if (PlayerController)
	{
		return PlayerController;
	}

	// [v1.2.0] 위젯 소유 플레이어가 없으면 Pawn의 컨트롤러를 fallback으로 사용합니다.
	return VehiclePawnRef ? Cast<APlayerController>(VehiclePawnRef->GetController()) : nullptr;
}

bool UCFVehicleDebugPanelWidget::IsDynamicSectionLayoutReady() const
{
	return VerticalBox_DynamicSectionHost != nullptr && DynamicSectionWidgetClass != nullptr;
}

bool UCFVehicleDebugPanelWidget::IsNavigationLayoutReady() const
{
	return VerticalBox_NavHost != nullptr && NavItemWidgetClass != nullptr;
}

bool UCFVehicleDebugPanelWidget::IsSelectedSectionLayoutReady() const
{
	return VerticalBox_SelectedSectionHost != nullptr && DynamicSectionWidgetClass != nullptr;
}

void UCFVehicleDebugPanelWidget::EnsureDynamicSectionWidgets()
{
	// [v1.6.0] 동적 Section 레이아웃 준비가 안 되었으면 생성을 중단합니다.
	if (!IsDynamicSectionLayoutReady())
	{
		return;
	}

	// [v1.6.0] 현재 필요한 최상위 섹션 개수입니다.
	const int32 RequiredSectionCount = CachedPanelViewData.TopLevelSectionArray.Num();

	if (DynamicSectionWidgetArray.Num() == RequiredSectionCount)
	{
		return;
	}

	VerticalBox_DynamicSectionHost->ClearChildren();
	DynamicSectionWidgetArray.Reset();

	for (int32 SectionIndex = 0; SectionIndex < RequiredSectionCount; ++SectionIndex)
	{
		// [v1.6.0] 새로 생성할 동적 Section 위젯 인스턴스입니다.
		UCFVehicleDebugSectionWidget* DynamicSectionWidget = nullptr;

		if (APlayerController* OwningPlayerController = GetOwningPlayer())
		{
			DynamicSectionWidget = CreateWidget<UCFVehicleDebugSectionWidget>(OwningPlayerController, DynamicSectionWidgetClass);
		}
		else if (WidgetTree)
		{
			DynamicSectionWidget = CreateWidget<UCFVehicleDebugSectionWidget>(WidgetTree, DynamicSectionWidgetClass);
		}

		if (!DynamicSectionWidget)
		{
			continue;
		}

		DynamicSectionWidgetArray.Add(DynamicSectionWidget);
		VerticalBox_DynamicSectionHost->AddChildToVerticalBox(DynamicSectionWidget);
	}
}

void UCFVehicleDebugPanelWidget::RefreshDynamicSectionWidgets()
{
	// [v1.6.0] 동적 Section 레이아웃 준비가 안 되었으면 갱신을 중단합니다.
	if (!IsDynamicSectionLayoutReady())
	{
		return;
	}

	if (VerticalBox_DynamicSectionHost)
	{
		VerticalBox_DynamicSectionHost->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (VerticalBox_SelectedSectionHost)
	{
		VerticalBox_SelectedSectionHost->SetVisibility(ESlateVisibility::Collapsed);
	}

	EnsureDynamicSectionWidgets();

	for (int32 SectionIndex = 0; SectionIndex < DynamicSectionWidgetArray.Num(); ++SectionIndex)
	{
		// [v1.6.0] 현재 갱신할 동적 Section 위젯 인스턴스입니다.
		UCFVehicleDebugSectionWidget* DynamicSectionWidget = DynamicSectionWidgetArray[SectionIndex];

		// [v1.6.0] 현재 위젯에 대응하는 최상위 섹션 ViewData입니다.
		const TSharedPtr<FCFVehicleDebugSectionViewData> TopLevelSectionViewData =
			CachedPanelViewData.TopLevelSectionArray.IsValidIndex(SectionIndex)
			? CachedPanelViewData.TopLevelSectionArray[SectionIndex]
			: nullptr;

		if (!DynamicSectionWidget || !TopLevelSectionViewData.IsValid())
		{
			continue;
		}

		DynamicSectionWidget->SetSectionViewData(*TopLevelSectionViewData);
	}
}

TArray<FCFVehicleDebugNavItemViewData> UCFVehicleDebugPanelWidget::BuildNavigationItemViewDataArray() const
{
	// [v1.7.0] 생성할 Navigation Item ViewData 배열입니다.
	TArray<FCFVehicleDebugNavItemViewData> NavItemViewDataArray;

	for (const TSharedPtr<FCFVehicleDebugSectionViewData>& TopLevelSectionViewData : CachedPanelViewData.TopLevelSectionArray)
	{
		if (!TopLevelSectionViewData.IsValid() || !TopLevelSectionViewData->bIsVisible || !TopLevelSectionViewData->bShowInNavigation)
		{
			continue;
		}

		// [v1.7.0] 현재 최상위 Section에서 만든 Navigation 항목입니다.
		FCFVehicleDebugNavItemViewData NavItemViewData;
		NavItemViewData.SectionId = TopLevelSectionViewData->SectionId;
		NavItemViewData.DisplayText = TopLevelSectionViewData->TitleText;
		NavItemViewData.NavigationGroup = TopLevelSectionViewData->NavigationGroup;
		NavItemViewData.NavigationOrder = TopLevelSectionViewData->NavigationOrder;
		NavItemViewData.BadgeText = TopLevelSectionViewData->BadgeText;
		NavItemViewData.bIsVisible = true;
		NavItemViewData.bIsSelected = TopLevelSectionViewData->SectionId == SelectedSectionId;
		NavItemViewDataArray.Add(NavItemViewData);
	}

	NavItemViewDataArray.Sort([](const FCFVehicleDebugNavItemViewData& Left, const FCFVehicleDebugNavItemViewData& Right)
	{
		if (Left.NavigationGroup != Right.NavigationGroup)
		{
			return static_cast<uint8>(Left.NavigationGroup) < static_cast<uint8>(Right.NavigationGroup);
		}

		if (Left.NavigationOrder != Right.NavigationOrder)
		{
			return Left.NavigationOrder < Right.NavigationOrder;
		}

		return Left.DisplayText < Right.DisplayText;
	});

	return NavItemViewDataArray;
}

void UCFVehicleDebugPanelWidget::ResolveSelectedSectionId()
{
	if (CachedPanelViewData.TopLevelSectionArray.IsEmpty())
	{
		SelectedSectionId.Reset();
		return;
	}

	for (const TSharedPtr<FCFVehicleDebugSectionViewData>& TopLevelSectionViewData : CachedPanelViewData.TopLevelSectionArray)
	{
		if (TopLevelSectionViewData.IsValid() &&
			TopLevelSectionViewData->bIsVisible &&
			TopLevelSectionViewData->SectionId == SelectedSectionId)
		{
			return;
		}
	}

	for (const TSharedPtr<FCFVehicleDebugSectionViewData>& TopLevelSectionViewData : CachedPanelViewData.TopLevelSectionArray)
	{
		if (TopLevelSectionViewData.IsValid() && TopLevelSectionViewData->bIsVisible)
		{
			SelectedSectionId = TopLevelSectionViewData->SectionId;
			return;
		}
	}

	SelectedSectionId.Reset();
}

TSharedPtr<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::FindSelectedSectionViewData() const
{
	for (const TSharedPtr<FCFVehicleDebugSectionViewData>& TopLevelSectionViewData : CachedPanelViewData.TopLevelSectionArray)
	{
		if (TopLevelSectionViewData.IsValid() &&
			TopLevelSectionViewData->bIsVisible &&
			TopLevelSectionViewData->SectionId == SelectedSectionId)
		{
			return TopLevelSectionViewData;
		}
	}

	return nullptr;
}

void UCFVehicleDebugPanelWidget::EnsureNavigationItemWidgets(const TArray<FCFVehicleDebugNavItemViewData>& InNavigationItemViewDataArray)
{
	// [v1.7.0] Navigation 레이아웃 준비가 안 되었으면 생성을 중단합니다.
	if (!IsNavigationLayoutReady())
	{
		return;
	}

	// [v1.7.0] 기존 Navigation Item 캐시에 비어 있는 항목이 있는지 확인합니다.
	bool bHasInvalidCachedNavItem = false;
	for (const TObjectPtr<UCFVehicleDebugNavItemWidget>& NavigationItemWidget : NavigationItemWidgetArray)
	{
		if (!NavigationItemWidget)
		{
			bHasInvalidCachedNavItem = true;
			break;
		}
	}

	// [v1.7.0] 개수가 같고 캐시가 유효하면 기존 위젯을 재사용합니다.
	if (NavigationItemWidgetArray.Num() == InNavigationItemViewDataArray.Num() && !bHasInvalidCachedNavItem)
	{
		return;
	}

	VerticalBox_NavHost->ClearChildren();
	NavigationItemWidgetArray.Reset();

	for (int32 NavItemIndex = 0; NavItemIndex < InNavigationItemViewDataArray.Num(); ++NavItemIndex)
	{
		// [v1.7.0] 새로 생성할 Navigation Item 위젯 인스턴스입니다.
		UCFVehicleDebugNavItemWidget* NavigationItemWidget = nullptr;

		if (APlayerController* OwningPlayerController = GetOwningPlayer())
		{
			NavigationItemWidget = CreateWidget<UCFVehicleDebugNavItemWidget>(OwningPlayerController, NavItemWidgetClass);
		}
		else if (WidgetTree)
		{
			NavigationItemWidget = CreateWidget<UCFVehicleDebugNavItemWidget>(WidgetTree, NavItemWidgetClass);
		}

		if (!NavigationItemWidget)
		{
			continue;
		}

		NavigationItemWidget->SetOwningPanelWidget(this);
		NavigationItemWidgetArray.Add(NavigationItemWidget);
		VerticalBox_NavHost->AddChildToVerticalBox(NavigationItemWidget);
	}
}

void UCFVehicleDebugPanelWidget::RefreshNavigationItems()
{
	if (!IsNavigationLayoutReady())
	{
		return;
	}

	// [v1.7.0] 현재 Panel ViewData 기준으로 생성한 Navigation 항목 배열입니다.
	const TArray<FCFVehicleDebugNavItemViewData> NavItemViewDataArray = BuildNavigationItemViewDataArray();

	EnsureNavigationItemWidgets(NavItemViewDataArray);

	for (int32 NavItemIndex = 0; NavItemIndex < NavigationItemWidgetArray.Num(); ++NavItemIndex)
	{
		// [v1.7.0] 현재 갱신할 Navigation Item 위젯입니다.
		UCFVehicleDebugNavItemWidget* NavigationItemWidget = NavigationItemWidgetArray[NavItemIndex];

		if (!NavigationItemWidget || !NavItemViewDataArray.IsValidIndex(NavItemIndex))
		{
			continue;
		}

		NavigationItemWidget->SetOwningPanelWidget(this);
		NavigationItemWidget->SetNavItemViewData(NavItemViewDataArray[NavItemIndex]);
	}
}

void UCFVehicleDebugPanelWidget::RefreshSelectedSectionWidget()
{
	// [v1.7.0] 선택 Section 레이아웃 준비가 안 되었으면 갱신을 중단합니다.
	if (!IsSelectedSectionLayoutReady())
	{
		return;
	}

	if (VerticalBox_SelectedSectionHost)
	{
		VerticalBox_SelectedSectionHost->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	if (VerticalBox_DynamicSectionHost)
	{
		VerticalBox_DynamicSectionHost->SetVisibility(ESlateVisibility::Collapsed);
	}

	// [v1.7.0] 현재 선택된 Section ViewData입니다.
	const TSharedPtr<FCFVehicleDebugSectionViewData> SelectedSectionViewData = FindSelectedSectionViewData();

	if (!SelectedSectionViewData.IsValid())
	{
		VerticalBox_SelectedSectionHost->ClearChildren();
		SelectedSectionWidget = nullptr;
		return;
	}

	if (!SelectedSectionWidget)
	{
		if (APlayerController* OwningPlayerController = GetOwningPlayer())
		{
			SelectedSectionWidget = CreateWidget<UCFVehicleDebugSectionWidget>(OwningPlayerController, DynamicSectionWidgetClass);
		}
		else if (WidgetTree)
		{
			SelectedSectionWidget = CreateWidget<UCFVehicleDebugSectionWidget>(WidgetTree, DynamicSectionWidgetClass);
		}

		if (SelectedSectionWidget)
		{
			VerticalBox_SelectedSectionHost->ClearChildren();
			VerticalBox_SelectedSectionHost->AddChildToVerticalBox(SelectedSectionWidget);
		}
	}

	if (SelectedSectionWidget)
	{
		SelectedSectionWidget->SetSectionViewData(*SelectedSectionViewData);
	}
}

bool UCFVehicleDebugPanelWidget::ShouldUseLegacyFullSectionRendering() const
{
	return !IsSelectedSectionLayoutReady() && IsDynamicSectionLayoutReady();
}

// [v1.5.0] 현재 Snapshot 기준 최신 Panel ViewData를 생성합니다.
FCFVehicleDebugPanelViewData UCFVehicleDebugPanelWidget::BuildVehicleDebugPanelViewData() const
{
	// [v1.5.0] 생성할 Panel ViewData 결과입니다.
	FCFVehicleDebugPanelViewData PanelViewData;
	PanelViewData.PanelId = TEXT("VehicleDebugPanel");
	PanelViewData.PanelTitleText = TEXT("차량 디버그 패널");
	PanelViewData.GeneratedFrameNumber = GFrameCounter;
	PanelViewData.GeneratedTimeSeconds = FPlatformTime::Seconds();
	PanelViewData.AddTopLevelSection(BuildOverviewSectionViewData(CachedOverview));
	PanelViewData.AddTopLevelSection(BuildDriveSectionViewData(CachedDrive));
	PanelViewData.AddTopLevelSection(BuildInputSectionViewData(CachedInput));
	PanelViewData.AddTopLevelSection(BuildCameraSectionViewData(CachedCamera));
	PanelViewData.AddTopLevelSection(BuildAimSectionViewData(CachedAim));
	PanelViewData.AddTopLevelSection(BuildTargetSectionViewData(CachedTarget));
	PanelViewData.AddTopLevelSection(BuildWeaponSectionViewData(CachedWeapon));
	PanelViewData.AddTopLevelSection(BuildRuntimeSectionViewData(CachedRuntime));
	return PanelViewData;
}

// [v1.5.0] Overview 카테고리용 Section ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildOverviewSectionViewData(const FCFVehicleDebugOverview& InOverview) const
{
	// [v1.5.0] 생성할 Overview 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> OverviewSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("Overview"), TEXT("개요"), ECFVehicleDebugSectionKind::Category, bIsOverviewExpanded);
	OverviewSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Core;
	OverviewSectionViewData->NavigationOrder = 10;
	OverviewSectionViewData->bShowInNavigation = true;

	OverviewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("overview_runtime_ready"), TEXT("준비"), InOverview.bRuntimeReady ? TEXT("예") : TEXT("아니오"), true));
	OverviewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("overview_state"), TEXT("상태"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.CurrentDriveState))));
	OverviewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("overview_speed"), TEXT("속도"), FString::Printf(TEXT("%.1f km/h"), InOverview.SpeedKmh)));
	OverviewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("overview_forward_speed"), TEXT("전진 속도"), FString::Printf(TEXT("%.1f km/h"), InOverview.ForwardSpeedKmh)));
	OverviewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("overview_device_mode"), TEXT("입력 모드"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.DeviceMode))));
	OverviewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("overview_input_owner"), TEXT("입력 소유자"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InOverview.InputOwner))));

	// [v1.5.0] Overview Last Transition 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> OverviewLastTransitionSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("OverviewLastTransition"), TEXT("최근 전이"), ECFVehicleDebugSectionKind::Subsection, bIsOverviewLastTransitionExpanded);
	OverviewLastTransitionSectionViewData->BodyText = BuildOverviewLastTransitionBodyText(InOverview);
	OverviewSectionViewData->AddChildSection(OverviewLastTransitionSectionViewData);

	return OverviewSectionViewData;
}

// [v1.5.0] Drive 카테고리용 Section ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildDriveSectionViewData(const FCFVehicleDebugDrive& InDrive) const
{
	// [v1.5.0] 생성할 Drive 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> DriveSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("Drive"), TEXT("주행"), ECFVehicleDebugSectionKind::Category, bIsDriveExpanded);
	DriveSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Core;
	DriveSectionViewData->NavigationOrder = 20;
	DriveSectionViewData->bShowInNavigation = true;

	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_current_state"), TEXT("현재 상태"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InDrive.CurrentDriveState))));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_previous_state"), TEXT("이전 상태"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InDrive.PreviousDriveState))));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_changed_this_frame"), TEXT("이번 프레임 변경"), InDrive.bDriveStateChangedThisFrame ? TEXT("예") : TEXT("아니오")));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_speed"), TEXT("속도"), FString::Printf(TEXT("%.1f km/h"), InDrive.SpeedKmh)));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_forward_speed"), TEXT("전진 속도"), FString::Printf(TEXT("%.1f km/h"), InDrive.ForwardSpeedKmh)));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_throttle"), TEXT("스로틀"), FString::Printf(TEXT("%.2f"), InDrive.Throttle)));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_brake"), TEXT("브레이크"), FString::Printf(TEXT("%.2f"), InDrive.Brake)));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_steering"), TEXT("조향"), FString::Printf(TEXT("%.2f"), InDrive.Steering)));
	DriveSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("drive_handbrake"), TEXT("핸드브레이크"), InDrive.bHandbrake ? TEXT("켜짐") : TEXT("꺼짐")));

	// [v1.5.0] Drive Transition 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> DriveTransitionSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("DriveTransition"), TEXT("전이"), ECFVehicleDebugSectionKind::Subsection, bIsDriveTransitionExpanded);
	DriveTransitionSectionViewData->BodyText = BuildDriveTransitionBodyText(InDrive);
	DriveSectionViewData->AddChildSection(DriveTransitionSectionViewData);

	return DriveSectionViewData;
}

// [v1.5.0] Input 카테고리용 Section ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildInputSectionViewData(const FCFVehicleDebugInput& InInput) const
{
	// [v1.5.0] 생성할 Input 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> InputSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("Input"), TEXT("입력"), ECFVehicleDebugSectionKind::Category, bIsInputExpanded);
	InputSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Core;
	InputSectionViewData->NavigationOrder = 30;
	InputSectionViewData->bShowInNavigation = true;

	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_device_mode"), TEXT("장치 모드"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.DeviceMode))));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_input_owner"), TEXT("입력 소유자"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.InputOwner))));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_move_zone"), TEXT("이동 영역"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.MoveZone))));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_move_intent"), TEXT("이동 의도"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InInput.MoveIntent))));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_move_raw"), TEXT("이동 원본값"), FString::Printf(TEXT("(%.2f, %.2f)"), InInput.MoveRaw.X, InInput.MoveRaw.Y)));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_move_magnitude"), TEXT("이동 크기"), FString::Printf(TEXT("%.2f"), InInput.MoveMagnitude)));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_move_angle"), TEXT("이동 각도"), FString::Printf(TEXT("%.1f"), InInput.MoveAngle)));
	InputSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("input_black_hold"), TEXT("블랙존 유지"), InInput.bUsedBlackZoneHold ? TEXT("예") : TEXT("아니오")));

	return InputSectionViewData;
}

// [v1.8.0] Camera category section ViewData.
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildCameraSectionViewData(const FCFVehicleDebugCamera& InCamera) const
{
	const FCFVehicleCameraRuntimeState& CameraState = InCamera.CameraRuntimeState;
	const bool bIsAimLimited = CameraState.bAimAtYawLimit || CameraState.bAimAtPitchLimit;
	const FString CameraStatusText = CameraState.bAimBlocked
		? TEXT("조준 막힘")
		: (InCamera.bCameraCompressedByCollision
			? TEXT("카메라 압축")
			: (bIsAimLimited ? TEXT("조준 제한") : TEXT("정상")));

	TSharedRef<FCFVehicleDebugSectionViewData> CameraSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("Camera"), TEXT("카메라"), ECFVehicleDebugSectionKind::Category, true);
	CameraSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Vehicle;
	CameraSectionViewData->NavigationOrder = 30;
	CameraSectionViewData->bShowInNavigation = true;

	CameraSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_status_summary"), TEXT("상태 요약"), CameraStatusText, CameraState.bAimBlocked || InCamera.bCameraCompressedByCollision || bIsAimLimited));
	CameraSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_has_comp"), TEXT("카메라 컴포넌트"), InCamera.bHasVehicleCameraComponent ? TEXT("있음") : TEXT("없음"), !InCamera.bHasVehicleCameraComponent));
	CameraSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_mode"), TEXT("현재 모드"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(CameraState.CurrentCameraMode))));
	CameraSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_profile"), TEXT("조준 프로필"), CameraState.ActiveAimProfileName.ToString()));
	CameraSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_aim_blocked"), TEXT("조준 막힘"), CameraState.bAimBlocked ? TEXT("예") : TEXT("아니오"), CameraState.bAimBlocked));
	CameraSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_aim_trace_blocking_hit"), TEXT("조준 Trace Blocking Hit"), CameraState.bAimTraceHasBlockingHit ? TEXT("예") : TEXT("아니오"), !CameraState.bAimTraceHasBlockingHit));
	CameraSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_can_fire"), TEXT("발사 가능"), CameraState.bWeaponCanFireAtCurrentAim ? TEXT("예") : TEXT("아니오"), !CameraState.bWeaponCanFireAtCurrentAim));
	CameraSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_compression"), TEXT("압축 비율"), FString::Printf(TEXT("%.2f"), InCamera.CollisionCompressionRatio), InCamera.bCameraCompressedByCollision));
	CameraSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_current_fov"), TEXT("현재 FOV"), FString::Printf(TEXT("%.1f"), CameraState.CurrentFOV)));

	TSharedRef<FCFVehicleDebugSectionViewData> AimSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("CameraAim"), TEXT("조준"), ECFVehicleDebugSectionKind::Subsection, true);
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_aim_accum_yaw"), TEXT("누적 Yaw"), FString::Printf(TEXT("%.1f"), CameraState.AccumulatedAimYaw)));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_aim_accum_pitch"), TEXT("누적 Pitch"), FString::Printf(TEXT("%.1f"), CameraState.AccumulatedAimPitch)));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_aim_clamped_yaw"), TEXT("제한 적용 Yaw"), FString::Printf(TEXT("%.1f"), CameraState.ClampedAimYaw)));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_aim_clamped_pitch"), TEXT("제한 적용 Pitch"), FString::Printf(TEXT("%.1f"), CameraState.ClampedAimPitch)));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_aim_yaw_limit"), TEXT("Yaw 제한"), CameraState.bAimAtYawLimit ? TEXT("걸림") : TEXT("아님"), CameraState.bAimAtYawLimit));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_aim_pitch_limit"), TEXT("Pitch 제한"), CameraState.bAimAtPitchLimit ? TEXT("걸림") : TEXT("아님"), CameraState.bAimAtPitchLimit));
	CameraSectionViewData->AddChildSection(AimSectionViewData);

	TSharedRef<FCFVehicleDebugSectionViewData> ViewSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("CameraView"), TEXT("시야"), ECFVehicleDebugSectionKind::Subsection, true);
	ViewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_view_desired_arm"), TEXT("목표 거리"), FString::Printf(TEXT("%.1f"), CameraState.DesiredArmLength)));
	ViewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_view_current_arm"), TEXT("현재 거리"), FString::Printf(TEXT("%.1f"), CameraState.CurrentArmLength)));
	ViewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_view_solved_arm"), TEXT("실제 적용 거리"), FString::Printf(TEXT("%.1f"), CameraState.SolvedArmLength), InCamera.bCameraCompressedByCollision));
	ViewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_view_desired_fov"), TEXT("목표 FOV"), FString::Printf(TEXT("%.1f"), CameraState.DesiredFOV)));
	ViewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_view_current_fov"), TEXT("현재 FOV"), FString::Printf(TEXT("%.1f"), CameraState.CurrentFOV)));
	ViewSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_view_compression"), TEXT("압축 비율"), FString::Printf(TEXT("%.2f"), InCamera.CollisionCompressionRatio), InCamera.bCameraCompressedByCollision));
	CameraSectionViewData->AddChildSection(ViewSectionViewData);

	TSharedRef<FCFVehicleDebugSectionViewData> TraceSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("CameraTrace"), TEXT("트레이스"), ECFVehicleDebugSectionKind::Subsection, true);
	TraceSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_trace_blocked"), TEXT("조준 막힘"), CameraState.bAimBlocked ? TEXT("예") : TEXT("아니오"), CameraState.bAimBlocked));
	TraceSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_trace_blocking_hit"), TEXT("Blocking Hit"), CameraState.bAimTraceHasBlockingHit ? TEXT("예") : TEXT("아니오"), !CameraState.bAimTraceHasBlockingHit));
	TraceSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_trace_can_fire"), TEXT("발사 가능"), CameraState.bWeaponCanFireAtCurrentAim ? TEXT("예") : TEXT("아니오"), !CameraState.bWeaponCanFireAtCurrentAim));
	TraceSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_trace_distance"), TEXT("트레이스 거리"), FString::Printf(TEXT("%.1f"), CameraState.AimTraceDistance)));
	TraceSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_trace_hit"), TEXT("명중 위치"), FString::Printf(TEXT("(%.1f, %.1f, %.1f)"), CameraState.AimHitLocation.X, CameraState.AimHitLocation.Y, CameraState.AimHitLocation.Z)));
	CameraSectionViewData->AddChildSection(TraceSectionViewData);

	TSharedRef<FCFVehicleDebugSectionViewData> ModeSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("CameraMode"), TEXT("모드"), ECFVehicleDebugSectionKind::Subsection, true);
	ModeSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_mode_current"), TEXT("현재"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(CameraState.CurrentCameraMode))));
	ModeSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_mode_previous"), TEXT("이전"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(CameraState.PreviousCameraMode))));
	ModeSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_mode_changed"), TEXT("이번 프레임 변경"), CameraState.bCameraModeChangedThisFrame ? TEXT("예") : TEXT("아니오"), CameraState.bCameraModeChangedThisFrame));
	ModeSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("camera_mode_profile"), TEXT("조준 프로필"), CameraState.ActiveAimProfileName.ToString()));
	CameraSectionViewData->AddChildSection(ModeSectionViewData);

	return CameraSectionViewData;
}

// [v1.8.0] Aim 카테고리용 Section ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildAimSectionViewData(const FCFVehicleDebugAim& InAim) const
{
	// [v1.8.0] Aim Section에서 사용할 Local Aim 상태입니다.
	const FCFVehicleLocalAimState& LocalAimState = InAim.LocalAimState;

	// [v1.8.3] Aim Section에서 사용할 발사 검증 상태입니다.
	const FCFVehicleFireValidationState& FireValidationState = InAim.FireValidationState;

	// [v1.8.3] Aim Section에서 사용할 표시용 Aim 시각 상태입니다.
	const FCFVehicleAimVisualState& AimVisualState = InAim.AimVisualState;

	// [v1.26.0] Weapon Aim Solution을 읽어올 Aim 컴포넌트입니다.
	const UCFVehicleAimComp* VehicleAimComp = IsValid(VehiclePawnRef) ? VehiclePawnRef->GetVehicleAimComp() : nullptr;

	// [v1.26.0] AimComp가 계산해 캐시한 무기 조준 해입니다.
	const FCFVehicleWeaponAimSolution WeaponAimSolution = IsValid(VehicleAimComp) ? VehicleAimComp->GetWeaponAimSolution() : FCFVehicleWeaponAimSolution();

	// [v1.8.0] Navigation 배지와 주요 필드에 표시할 Reticle 상태 문자열입니다.
	const FString ReticleStateText = ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InAim.ReticleState));

	// [v1.8.0] AimComp 상태를 한 줄로 요약한 문자열입니다.
	const FString AimStatusText = !InAim.bHasVehicleAimComponent
		? TEXT("AimComp 없음")
		: (!InAim.bAimRuntimeReady
			? TEXT("런타임 미준비")
			: ReticleStateText);

	// [v1.8.0] 생성할 Aim 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> AimSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("Aim"), TEXT("조준"), ECFVehicleDebugSectionKind::Category, true);
	AimSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Vehicle;
	AimSectionViewData->NavigationOrder = 45;
	AimSectionViewData->BadgeText = ReticleStateText;
	AimSectionViewData->bShowInNavigation = true;

	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_status_summary"), TEXT("상태 요약"), AimStatusText, !InAim.bHasVehicleAimComponent || !InAim.bAimRuntimeReady || InAim.ReticleState != ECFVehicleReticleState::Ready));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_has_comp"), TEXT("Aim 컴포넌트"), InAim.bHasVehicleAimComponent ? TEXT("있음") : TEXT("없음"), !InAim.bHasVehicleAimComponent));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_runtime_ready"), TEXT("런타임 준비"), InAim.bAimRuntimeReady ? TEXT("예") : TEXT("아니오"), !InAim.bAimRuntimeReady));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_reticle_state"), TEXT("Reticle 상태"), ReticleStateText, InAim.ReticleState != ECFVehicleReticleState::Ready));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_local_can_fire"), TEXT("로컬 발사 가능"), LocalAimState.bLocalCanFire ? TEXT("예") : TEXT("아니오"), !LocalAimState.bLocalCanFire));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_within_arc"), TEXT("무기 조준각 내부"), LocalAimState.bLocalWithinWeaponArc ? TEXT("예") : TEXT("아니오"), !LocalAimState.bLocalWithinWeaponArc));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_blocked"), TEXT("조준 막힘"), LocalAimState.bLocalAimBlocked ? TEXT("예") : TEXT("아니오"), LocalAimState.bLocalAimBlocked));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_weapon_solution_valid"), TEXT("무기 조준 해"), WeaponAimSolution.bHasValidSolution ? TEXT("유효") : TEXT("무효"), !WeaponAimSolution.bHasValidSolution));
	AimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("aim_runtime_summary"), TEXT("런타임 요약"), InAim.AimRuntimeSummary));

	// [v1.8.0] Local Aim 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> LocalAimSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("AimLocal"), TEXT("로컬 조준"), ECFVehicleDebugSectionKind::Subsection, true);
	LocalAimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_local_target"), TEXT("목표 위치"), FString::Printf(TEXT("(%.1f, %.1f, %.1f)"), LocalAimState.LocalAimTargetLocation.X, LocalAimState.LocalAimTargetLocation.Y, LocalAimState.LocalAimTargetLocation.Z)));
	LocalAimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_local_direction"), TEXT("조준 방향"), FString::Printf(TEXT("(%.2f, %.2f, %.2f)"), LocalAimState.LocalAimDirection.X, LocalAimState.LocalAimDirection.Y, LocalAimState.LocalAimDirection.Z)));
	AimSectionViewData->AddChildSection(LocalAimSectionViewData);

	// [v1.26.0] Weapon Aim Solution 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> WeaponAimSolutionSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("WeaponAimSolution"), TEXT("무기 조준 해"), ECFVehicleDebugSectionKind::Subsection, true);
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_valid"), TEXT("유효 여부"), WeaponAimSolution.bHasValidSolution ? TEXT("예") : TEXT("아니오"), !WeaponAimSolution.bHasValidSolution));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_trace_hit"), TEXT("Camera Aim Trace Hit"), WeaponAimSolution.bAimTraceHasBlockingHit ? TEXT("예") : TEXT("아니오"), !WeaponAimSolution.bAimTraceHasBlockingHit));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_turret_aligning"), TEXT("터렛 정렬 중"), WeaponAimSolution.bTurretAligning ? TEXT("예") : TEXT("아니오"), WeaponAimSolution.bTurretAligning));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_weapon_not_aligned"), TEXT("무기 정렬 실패"), WeaponAimSolution.bWeaponNotAligned ? TEXT("예") : TEXT("아니오"), WeaponAimSolution.bWeaponNotAligned));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_muzzle_blocked"), TEXT("총구 막힘"), WeaponAimSolution.bMuzzleBlocked ? TEXT("예") : TEXT("아니오"), WeaponAimSolution.bMuzzleBlocked));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_turret_reticle_valid"), TEXT("터렛 레티클 유효 여부"), WeaponAimSolution.bHasValidTurretReticlePoint ? TEXT("예") : TEXT("아니오"), !WeaponAimSolution.bHasValidTurretReticlePoint));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_turret_reticle_location"), TEXT("터렛 레티클 월드 위치"), FString::Printf(TEXT("(%.1f, %.1f, %.1f)"), WeaponAimSolution.TurretReticleWorldLocation.X, WeaponAimSolution.TurretReticleWorldLocation.Y, WeaponAimSolution.TurretReticleWorldLocation.Z), !WeaponAimSolution.bHasValidTurretReticlePoint));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_turret_reticle_distance"), TEXT("터렛 레티클 비교 거리"), FString::Printf(TEXT("%.1f"), WeaponAimSolution.TurretReticleDistance), !WeaponAimSolution.bHasValidTurretReticlePoint));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_reticle_mode"), TEXT("Legacy Weapon Preview Mode"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(WeaponAimSolution.WeaponReticleMode)), WeaponAimSolution.WeaponReticleMode == ECFWeaponReticleMode::Hidden));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_preview_valid"), TEXT("Legacy Weapon Preview 유효 여부"), WeaponAimSolution.bHasValidWeaponPreview ? TEXT("예") : TEXT("아니오"), !WeaponAimSolution.bHasValidWeaponPreview));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_preview_hit"), TEXT("Legacy Weapon Preview Blocking Hit"), WeaponAimSolution.bWeaponPreviewHasBlockingHit ? TEXT("예") : TEXT("아니오"), WeaponAimSolution.bHasValidWeaponPreview && !WeaponAimSolution.bWeaponPreviewHasBlockingHit));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_preview_location"), TEXT("Legacy Weapon Preview 월드 위치"), FString::Printf(TEXT("(%.1f, %.1f, %.1f)"), WeaponAimSolution.WeaponPreviewWorldLocation.X, WeaponAimSolution.WeaponPreviewWorldLocation.Y, WeaponAimSolution.WeaponPreviewWorldLocation.Z), !WeaponAimSolution.bHasValidWeaponPreview));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_preview_distance"), TEXT("Legacy Weapon Preview 거리"), FString::Printf(TEXT("%.1f"), WeaponAimSolution.WeaponPreviewDistance), !WeaponAimSolution.bHasValidWeaponPreview));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_origin"), TEXT("발사 시작 위치"), FString::Printf(TEXT("(%.1f, %.1f, %.1f)"), WeaponAimSolution.AimOrigin.X, WeaponAimSolution.AimOrigin.Y, WeaponAimSolution.AimOrigin.Z), !WeaponAimSolution.bHasValidSolution));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_direction"), TEXT("발사 방향"), FString::Printf(TEXT("(%.2f, %.2f, %.2f)"), WeaponAimSolution.AimDirection.X, WeaponAimSolution.AimDirection.Y, WeaponAimSolution.AimDirection.Z), !WeaponAimSolution.bHasValidSolution));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_target"), TEXT("목표 위치"), FString::Printf(TEXT("(%.1f, %.1f, %.1f)"), WeaponAimSolution.AimTargetLocation.X, WeaponAimSolution.AimTargetLocation.Y, WeaponAimSolution.AimTargetLocation.Z), !WeaponAimSolution.bHasValidSolution));
	WeaponAimSolutionSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_aim_solution_alignment_error"), TEXT("정렬 오차"), FString::Printf(TEXT("%.2f 도"), WeaponAimSolution.WeaponAlignmentErrorDeg), WeaponAimSolution.bWeaponNotAligned));
	AimSectionViewData->AddChildSection(WeaponAimSolutionSectionViewData);

	// [v1.8.2] 발사 검증 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> FireValidationSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("AimFireValidation"), TEXT("발사 검증"), ECFVehicleDebugSectionKind::Subsection, true);
	FireValidationSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_validation_can_fire"), TEXT("검증 발사 가능"), FireValidationState.bValidationCanFire ? TEXT("예") : TEXT("아니오"), !FireValidationState.bValidationCanFire));
	FireValidationSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_validation_within_arc"), TEXT("검증 조준각 내부"), FireValidationState.bValidationWithinWeaponArc ? TEXT("예") : TEXT("아니오"), !FireValidationState.bValidationWithinWeaponArc));
	FireValidationSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_validation_reject_reason"), TEXT("마지막 발사 거부 사유"), ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(FireValidationState.LastValidationRejectReason)), FireValidationState.LastValidationRejectReason != ECFVehicleFireRejectReason::None));
	FireValidationSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_validation_last_accepted_id"), TEXT("마지막 승인 명령 ID"), FString::FromInt(FireValidationState.LastAcceptedFireRequestId)));
	FireValidationSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_validation_last_rejected_id"), TEXT("마지막 거부 명령 ID"), FString::FromInt(FireValidationState.LastRejectedFireRequestId)));
	AimSectionViewData->AddChildSection(FireValidationSectionViewData);

	// [v1.8.3] Aim Visual 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> AimVisualSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("AimVisual"), TEXT("Aim 시각"), ECFVehicleDebugSectionKind::Subsection, true);
	AimVisualSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_visual_direction"), TEXT("표시 조준 방향"), FString::Printf(TEXT("(%.2f, %.2f, %.2f)"), AimVisualState.VisualAimDirection.X, AimVisualState.VisualAimDirection.Y, AimVisualState.VisualAimDirection.Z)));
	AimVisualSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_visual_target"), TEXT("표시 목표 위치"), FString::Printf(TEXT("(%.1f, %.1f, %.1f)"), AimVisualState.VisualAimTargetLocation.X, AimVisualState.VisualAimTargetLocation.Y, AimVisualState.VisualAimTargetLocation.Z)));
	AimVisualSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_visual_firing"), TEXT("발사 시각화"), AimVisualState.bIsFiringVisual ? TEXT("예") : TEXT("아니오"), AimVisualState.bIsFiringVisual));
	AimVisualSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("aim_visual_weapon_mode"), TEXT("무기 시각 모드"), AimVisualState.WeaponVisualMode.ToString()));
	AimSectionViewData->AddChildSection(AimVisualSectionViewData);

	return AimSectionViewData;
}

// [v1.33.0] 현재 선택 대상 카테고리용 Section ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildTargetSectionViewData(const FCFVehicleDebugTarget& InTarget) const
{
	// [v1.33.0] 선택 대상 상태를 Navigation과 주요 필드에 표시할 요약 문자열입니다.
	FString TargetStatusText = TEXT("선택 대상 없음");
	if (!InTarget.bHasTargetSelectComponent)
	{
		TargetStatusText = TEXT("TargetSelectComp 없음");
	}
	else if (InTarget.bHasSelectedTarget && !InTarget.bSelectedTargetValid)
	{
		TargetStatusText = TEXT("선택 대상 무효");
	}
	else if (InTarget.bSelectedTargetDestroyed)
	{
		TargetStatusText = TEXT("선택 대상 파괴");
	}
	else if (InTarget.bSelectedTargetValid)
	{
		TargetStatusText = TEXT("선택 대상 유효");
	}

	// [v1.33.0] Navigation에 표시할 선택 대상의 짧은 상태 배지입니다.
	const FString TargetBadgeText = !InTarget.bHasTargetSelectComponent
		? TEXT("컴포넌트없음")
		: (!InTarget.bHasSelectedTarget
			? TEXT("없음")
			: (!InTarget.bSelectedTargetValid
				? TEXT("무효")
				: (InTarget.bSelectedTargetDestroyed ? TEXT("파괴") : TEXT("선택됨"))));

	// [v1.33.0] 선택 대상 표시 이름이 비어 있을 때 사용할 안전한 표시 문자열입니다.
	const FString TargetDisplayNameText = InTarget.SelectedTargetDisplayName.IsEmpty()
		? TEXT("None")
		: InTarget.SelectedTargetDisplayName.ToString();

	// [v1.33.0] 선택 대상의 추적 enum을 Panel 표시 문자열로 변환한 값입니다.
	const FString TargetTrackStateText = ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(InTarget.SelectedTargetTrackState));

	// [v1.33.0] 선택 대상 방어 경로를 표시할 문자열입니다.
	const FString TargetDefenseModeText = !InTarget.bHasSelectedTargetDefenseComponent
		? TEXT("방어 컴포넌트 없음")
		: (InTarget.bSelectedTargetUsingLegacyDefenseFallback ? TEXT("Legacy Integrity Fallback") : TEXT("Shield → Armor → Integrity"));

	// [v1.33.0] 생성할 선택 대상 최상위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> TargetSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("Target"), TEXT("선택 대상"), ECFVehicleDebugSectionKind::Category, true);
	TargetSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Vehicle;
	TargetSectionViewData->NavigationOrder = 47;
	TargetSectionViewData->BadgeText = TargetBadgeText;
	TargetSectionViewData->bShowInNavigation = true;

	TargetSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_status_summary"), TEXT("상태 요약"), TargetStatusText, !InTarget.bSelectedTargetValid));
	TargetSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_has_select_comp"), TEXT("TargetSelect 컴포넌트"), InTarget.bHasTargetSelectComponent ? TEXT("있음") : TEXT("없음"), !InTarget.bHasTargetSelectComponent));
	TargetSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_has_selected_record"), TEXT("선택 기록"), InTarget.bHasSelectedTarget ? TEXT("있음") : TEXT("없음"), !InTarget.bHasSelectedTarget));
	TargetSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_valid"), TEXT("대상 유효"), InTarget.bSelectedTargetValid ? TEXT("예") : TEXT("아니오"), InTarget.bHasSelectedTarget && !InTarget.bSelectedTargetValid));
	TargetSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_display_name"), TEXT("표시 이름"), TargetDisplayNameText, InTarget.bHasSelectedTarget && TargetDisplayNameText == TEXT("None")));
	TargetSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_id"), TEXT("대상 ID"), InTarget.SelectedTargetId.ToString(), InTarget.bHasSelectedTarget && InTarget.SelectedTargetId.IsNone()));
	TargetSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_actor_name"), TEXT("Actor"), InTarget.SelectedTargetActorName, InTarget.bHasSelectedTarget && InTarget.SelectedTargetActorName == TEXT("None")));
	TargetSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_track_state"), TEXT("추적 상태"), TargetTrackStateText, InTarget.bHasSelectedTarget && InTarget.SelectedTargetTrackState == ECFTargetTrackState::Invalid));
	TargetSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_destroyed"), TEXT("파괴 상태"), InTarget.bSelectedTargetDestroyed ? TEXT("파괴됨") : TEXT("정상"), InTarget.bSelectedTargetDestroyed));

	// [v1.33.0] 선택 대상의 현재 Shield, 방향별 Armor와 재생 상태를 표시하는 하위 섹션입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> TargetDefenseSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("TargetDefense"), TEXT("대상 방어"), ECFVehicleDebugSectionKind::Subsection, true);
	TargetDefenseSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_defense_component"), TEXT("방어 컴포넌트"), InTarget.bHasSelectedTargetDefenseComponent ? TEXT("있음") : TEXT("없음"), InTarget.bSelectedTargetValid && !InTarget.bHasSelectedTargetDefenseComponent));
	TargetDefenseSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_defense_initialized"), TEXT("DefenseData 초기화"), InTarget.bSelectedTargetDefenseInitialized ? TEXT("예") : TEXT("아니오"), InTarget.bHasSelectedTargetDefenseComponent && !InTarget.bSelectedTargetDefenseInitialized && !InTarget.bSelectedTargetUsingLegacyDefenseFallback));
	TargetDefenseSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_defense_mode"), TEXT("피해 경로"), TargetDefenseModeText, InTarget.bSelectedTargetUsingLegacyDefenseFallback));
	TargetDefenseSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("target_defense_summary"), TEXT("현재 Shield·6방향 Armor·재생 상태"), InTarget.SelectedTargetDefenseSummary));
	TargetDefenseSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_defense_last_result_recorded"), TEXT("전체 피해 결과 기록"), InTarget.bHasSelectedTargetLastDamageResult ? TEXT("있음") : TEXT("없음"), InTarget.bHasSelectedTargetDefenseComponent && !InTarget.bHasSelectedTargetLastDamageResult));
	TargetDefenseSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("target_defense_last_result"), TEXT("마지막 방향·Shield·Armor·Integrity 결과"), InTarget.SelectedTargetLastDamageResultSummary));
	TargetSectionViewData->AddChildSection(TargetDefenseSectionViewData);

	// [v1.33.0] 선택 대상의 현재·최대 Integrity와 파괴 상태를 표시하는 하위 섹션입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> TargetIntegritySectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("TargetIntegrity"), TEXT("대상 내구도"), ECFVehicleDebugSectionKind::Subsection, true);
	TargetIntegritySectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_health_component"), TEXT("내구도 컴포넌트"), InTarget.bHasSelectedTargetHealthComponent ? TEXT("있음") : TEXT("없음"), InTarget.bSelectedTargetValid && !InTarget.bHasSelectedTargetHealthComponent));
	TargetIntegritySectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_health_initialized"), TEXT("Integrity 초기화"), InTarget.bSelectedTargetHealthInitialized ? TEXT("예") : TEXT("아니오"), InTarget.bHasSelectedTargetHealthComponent && !InTarget.bSelectedTargetHealthInitialized));
	TargetIntegritySectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_integrity_current"), TEXT("현재 Integrity"), FString::Printf(TEXT("%.1f"), InTarget.SelectedTargetCurrentIntegrity), InTarget.bHasSelectedTargetHealthComponent && InTarget.SelectedTargetCurrentIntegrity <= 0.0f));
	TargetIntegritySectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_integrity_maximum"), TEXT("최대 Integrity"), FString::Printf(TEXT("%.1f"), InTarget.SelectedTargetMaximumIntegrity), InTarget.bHasSelectedTargetHealthComponent && InTarget.SelectedTargetMaximumIntegrity <= 0.0f));
	TargetIntegritySectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("target_integrity_destroyed"), TEXT("파괴 여부"), InTarget.bSelectedTargetDestroyed ? TEXT("예") : TEXT("아니오"), InTarget.bSelectedTargetDestroyed));
	TargetSectionViewData->AddChildSection(TargetIntegritySectionViewData);

	// [v1.33.0] TargetSelect 약한 참조와 추적 수명 상태를 표시하는 하위 섹션입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> TargetLifetimeSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("TargetLifetime"), TEXT("대상 추적 수명"), ECFVehicleDebugSectionKind::Subsection, false);
	TargetLifetimeSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("target_lifetime_summary"), TEXT("수명 요약"), InTarget.SelectedTargetLifetimeSummary));
	TargetSectionViewData->AddChildSection(TargetLifetimeSectionViewData);

	return TargetSectionViewData;
}

// [v1.9.0] Weapon 카테고리용 Section ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildWeaponSectionViewData(const FCFVehicleDebugWeapon& InWeapon) const
{
	// [v1.9.0] Weapon Section에서 사용할 마지막 FireOrigin 상태입니다.
	const FCFVehicleFireOrigin& LastFireOrigin = InWeapon.LastFireOrigin;

	// [v1.9.0] MountType enum 값을 Panel 표시용 문자열로 변환한 값입니다.
	const FString MountTypeText = ConvertEnumValueToDisplayString(*UEnum::GetValueAsString(LastFireOrigin.MountType));

	// [v1.25.0] 활성 EquipmentPresetData가 실제로 연결되어 있는지 여부입니다.
	const bool bHasActiveEquipmentPresetData = InWeapon.ActiveEquipmentPresetData != nullptr;

	// [v1.10.0] 활성 WeaponData가 실제로 연결되어 있는지 여부입니다.
	const bool bHasActiveWeaponData = InWeapon.ActiveWeaponData != nullptr;

	// [v1.12.0] 활성 ProjectileData가 실제로 연결되어 있는지 여부입니다.
	const bool bHasActiveProjectileData = InWeapon.ActiveProjectileData != nullptr;

	// [v1.22.0] 활성 DamageData가 실제로 연결되어 있는지 여부입니다.
	const bool bHasActiveDamageData = InWeapon.ActiveDamageData != nullptr;

	// [v1.10.0] WeaponData 미지정을 패널에서 경고로 표시해야 하는지 여부입니다.
	const bool bShouldWarnMissingWeaponData = InWeapon.bHasVehicleWeaponComponent && InWeapon.bWeaponRuntimeReady && !bHasActiveWeaponData;

	// [v1.10.0] WeaponData 호환 실패를 패널에서 경고로 표시해야 하는지 여부입니다.
	const bool bShouldWarnIncompatibleWeaponData = bHasActiveWeaponData && !InWeapon.bActiveWeaponDataCompatible;

	// [v1.11.0] 현재 활성 무기가 쿨다운 중인지 여부입니다.
	const bool bIsWeaponOnCooldown = InWeapon.ActiveWeaponRemainingCooldownSeconds > KINDA_SMALL_NUMBER;

	// [v1.10.0] 활성 WeaponData 지정 여부를 표시할 문자열입니다.
	const FString WeaponDataAssignedText = bHasActiveWeaponData ? TEXT("지정") : TEXT("미지정");

	// [v1.12.0] 활성 ProjectileData 지정 여부를 표시할 문자열입니다.
	const FString ProjectileDataAssignedText = bHasActiveProjectileData ? TEXT("지정") : TEXT("미지정");

	// [v1.22.0] 활성 DamageData 지정 여부를 표시할 문자열입니다.
	const FString DamageDataAssignedText = bHasActiveDamageData ? TEXT("지정") : TEXT("미지정");

	// [v1.25.0] 활성 EquipmentPresetData 지정 여부를 표시할 문자열입니다.
	const FString EquipmentPresetDataAssignedText = bHasActiveEquipmentPresetData ? TEXT("지정") : TEXT("미지정");

	// [v1.25.0] 활성 EquipmentPresetData 호환 여부를 표시할 문자열입니다.
	const FString EquipmentPresetDataCompatibleText = bHasActiveEquipmentPresetData
		? (InWeapon.bActiveEquipmentPresetDataCompatible ? TEXT("예") : TEXT("아니오"))
		: TEXT("미지정");

	// [v1.25.0] EquipmentPresetData 호환 실패를 패널에서 경고로 표시해야 하는지 여부입니다.
	const bool bShouldWarnIncompatibleEquipmentPresetData = bHasActiveEquipmentPresetData && !InWeapon.bActiveEquipmentPresetDataCompatible;

	// [v1.24.0] 마지막 Damage HitContext입니다.
	const FCFDamageHitContext& LastDamageHitContext = InWeapon.LastDamageHitContext;

	// [v1.24.0] 마지막 Damage HitContext 기록 여부를 표시할 문자열입니다.
	const FString DamageHitContextAssignedText = InWeapon.bHasLastDamageHitContext ? TEXT("있음") : TEXT("없음");

	// [v1.24.0] 마지막 Damage HitContext 소스 경로를 표시할 문자열입니다.
	const FString DamageHitContextSourceText = LastDamageHitContext.bFromProjectileActor ? TEXT("Projectile Actor") : TEXT("Dummy HitScan");

	// [v1.24.0] 마지막 Damage HitContext 실제 적중 여부를 표시할 문자열입니다.
	const FString DamageHitContextHitText = LastDamageHitContext.bBlockingHit ? TEXT("예") : TEXT("아니오");

	// [v1.24.0] 마지막 Damage HitContext 피격 Actor 이름입니다.
	const FString DamageHitContextActorText = LastDamageHitContext.HitActor ? LastDamageHitContext.HitActor->GetName() : TEXT("None");

	// [v1.24.0] 마지막 Damage HitContext 발사 주체 Actor 이름입니다.
	const FString DamageHitContextInstigatorText = LastDamageHitContext.InstigatorActor ? LastDamageHitContext.InstigatorActor->GetName() : TEXT("None");

	// [v1.13.0] 활성 ProjectileData가 Projectile Actor 전환 후보인지 표시할 문자열입니다.
	const FString ProjectileSpawnReadyText = InWeapon.bActiveProjectileSpawnReady ? TEXT("준비됨") : TEXT("미준비");

	// [v1.14.0] 실제 발사 실행 경로가 Dummy HitScan 유지인지 Projectile Pool 준비 상태인지 표시할 문자열입니다.
	const FString ProjectileFallbackText = InWeapon.bActiveProjectileSpawnReady ? TEXT("Projectile Pool 준비됨") : TEXT("Dummy HitScan 유지");

	// [v1.15.0] Projectile Pool 컴포넌트 보유 여부를 표시할 문자열입니다.
	const FString ProjectilePoolComponentText = InWeapon.bHasProjectilePoolComponent ? TEXT("예") : TEXT("아니오");

	// [v1.15.0] Projectile 경로가 준비됐지만 Pool 컴포넌트가 없을 때 표시할 경고 여부입니다.
	const bool bShouldWarnMissingProjectilePool = InWeapon.bActiveProjectileSpawnReady && !InWeapon.bHasProjectilePoolComponent;

	// [v1.16.0] 활성 WeaponData의 분당 발사속도를 표시할 문자열입니다.
	const FString WeaponFireRateText = FString::Printf(TEXT("%.0f"), InWeapon.ActiveWeaponFireRatePerMinute);

	// [v1.18.0] 터렛 시각 메쉬가 실제로 붙었는지 표시할 문자열입니다.
	const FString TurretVisualAttachedText = InWeapon.bTurretVisualAttached ? TEXT("예") : TEXT("아니오");

	// [v1.19.0] 활성 TurretMountData가 직접 지정되어 있는지 표시할 문자열입니다.
	const FString TurretMountDataAssignedText = InWeapon.bActiveTurretMountDataAssigned ? TEXT("지정") : TEXT("미지정");

	// [v1.21.0] 현재 터렛 조준 추적 상태입니다.
	const FCFVehicleTurretState& TurretState = InWeapon.TurretState;

	// [v1.21.0] 터렛 안정화 여부를 표시할 문자열입니다.
	const FString TurretSettledText = TurretState.bTurretSettled ? TEXT("예") : TEXT("아니오");

	// [v1.10.0] 활성 WeaponData 호환 여부를 표시할 문자열입니다.
	const FString WeaponDataCompatibleText = bHasActiveWeaponData
		? (InWeapon.bActiveWeaponDataCompatible ? TEXT("예") : TEXT("아니오"))
		: TEXT("미지정");

	// [v1.9.0] Navigation 배지와 주요 필드에 사용할 Weapon 상태 요약입니다.
	FString WeaponStatusText = TEXT("FireOrigin 대기");
	if (!InWeapon.bHasVehicleWeaponComponent)
	{
		WeaponStatusText = TEXT("WeaponComp 없음");
	}
	else if (!InWeapon.bWeaponRuntimeReady)
	{
		WeaponStatusText = TEXT("런타임 미준비");
	}
	else if (!bHasActiveWeaponData)
	{
		WeaponStatusText = TEXT("WeaponData 미지정");
	}
	else if (!InWeapon.bActiveWeaponDataCompatible)
	{
		WeaponStatusText = TEXT("WeaponData 호환 불가");
	}
	else if (bIsWeaponOnCooldown)
	{
		WeaponStatusText = TEXT("쿨다운");
	}
	else if (InWeapon.bActiveProjectileSpawnReady)
	{
		WeaponStatusText = TEXT("Projectile Pool 준비됨");
	}
	else if (LastFireOrigin.bResolved)
	{
		WeaponStatusText = TEXT("FireOrigin 해결");
	}

	// [v1.9.0] Navigation에 표시할 짧은 Weapon 상태 배지입니다.
	const FString WeaponBadgeText = !InWeapon.bHasVehicleWeaponComponent
		? TEXT("없음")
		: (!InWeapon.bWeaponRuntimeReady
			? TEXT("미준비")
			: (!bHasActiveWeaponData
				? TEXT("데이터없음")
				: (!InWeapon.bActiveWeaponDataCompatible
					? TEXT("불일치")
					: (bIsWeaponOnCooldown
						? TEXT("쿨다운")
						: (InWeapon.bActiveProjectileSpawnReady
							? TEXT("발사체준비")
							: (LastFireOrigin.bResolved ? TEXT("해결") : TEXT("대기")))))));

	// [v1.9.0] 생성할 Weapon 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> WeaponSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("Weapon"), TEXT("무기"), ECFVehicleDebugSectionKind::Category, true);
	WeaponSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Vehicle;
	WeaponSectionViewData->NavigationOrder = 50;
	WeaponSectionViewData->BadgeText = WeaponBadgeText;
	WeaponSectionViewData->bShowInNavigation = true;

	WeaponSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_status_summary"), TEXT("상태 요약"), WeaponStatusText, !InWeapon.bHasVehicleWeaponComponent || !InWeapon.bWeaponRuntimeReady || !LastFireOrigin.bResolved || bShouldWarnMissingWeaponData || bShouldWarnIncompatibleWeaponData || bIsWeaponOnCooldown));
	WeaponSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_has_comp"), TEXT("Weapon 컴포넌트"), InWeapon.bHasVehicleWeaponComponent ? TEXT("있음") : TEXT("없음"), !InWeapon.bHasVehicleWeaponComponent));
	WeaponSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_runtime_ready"), TEXT("런타임 준비"), InWeapon.bWeaponRuntimeReady ? TEXT("예") : TEXT("아니오"), !InWeapon.bWeaponRuntimeReady));
	WeaponSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_active_profile"), TEXT("활성 프로파일"), InWeapon.ActiveMountProfileId.ToString(), InWeapon.ActiveMountProfileId.IsNone()));
	WeaponSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_equipment_preset_id"), TEXT("장비 프리셋 ID"), InWeapon.ActiveEquipmentPresetId.ToString(), false));
	WeaponSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_data_id"), TEXT("무기 데이터 ID"), InWeapon.ActiveWeaponId.ToString(), bShouldWarnMissingWeaponData));
	WeaponSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_data_compatible"), TEXT("무기 데이터 호환"), WeaponDataCompatibleText, bShouldWarnMissingWeaponData || bShouldWarnIncompatibleWeaponData));
	WeaponSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("weapon_runtime_summary"), TEXT("런타임 요약"), InWeapon.LastWeaponRuntimeSummary));

	// [v1.25.0] 활성 EquipmentPresetData 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> EquipmentPresetDataSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("EquipmentPresetData"), TEXT("장비 프리셋"), ECFVehicleDebugSectionKind::Subsection, true);
	EquipmentPresetDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("equipment_preset_data_assigned"), TEXT("지정 여부"), EquipmentPresetDataAssignedText, false));
	EquipmentPresetDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("equipment_preset_data_active_id"), TEXT("프리셋 ID"), InWeapon.ActiveEquipmentPresetId.ToString(), false));
	EquipmentPresetDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("equipment_preset_data_compatible"), TEXT("장착 호환"), EquipmentPresetDataCompatibleText, bShouldWarnIncompatibleEquipmentPresetData));
	EquipmentPresetDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("equipment_preset_data_summary"), TEXT("요약"), InWeapon.ActiveEquipmentPresetSummary));
	WeaponSectionViewData->AddChildSection(EquipmentPresetDataSectionViewData);

	// [v1.18.0] 터렛 시각 장착 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> TurretVisualSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("TurretVisual"), TEXT("터렛 시각"), ECFVehicleDebugSectionKind::Subsection, true);
	TurretVisualSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("turret_mount_data_assigned"), TEXT("마운트 데이터"), TurretMountDataAssignedText, !InWeapon.bActiveTurretMountDataAssigned));
	TurretVisualSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("turret_mount_data_id"), TEXT("마운트 ID"), InWeapon.ActiveTurretMountId.ToString(), InWeapon.ActiveTurretMountId.IsNone()));
	TurretVisualSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("turret_visual_attached"), TEXT("장착 여부"), TurretVisualAttachedText, !InWeapon.bTurretVisualAttached));
	TurretVisualSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("turret_visual_base_mesh"), TEXT("Base 메쉬"), InWeapon.TurretBaseMeshName.ToString(), InWeapon.TurretBaseMeshName.IsNone()));
	TurretVisualSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("turret_visual_yaw_mesh"), TEXT("Yaw 메쉬"), InWeapon.TurretYawMeshName.ToString(), InWeapon.TurretYawMeshName.IsNone()));
	TurretVisualSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("turret_visual_pitch_mesh"), TEXT("Pitch 메쉬"), InWeapon.TurretPitchMeshName.ToString(), InWeapon.TurretPitchMeshName.IsNone()));
	TurretVisualSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("turret_mount_data_summary"), TEXT("마운트 요약"), InWeapon.ActiveTurretMountSummary));
	TurretVisualSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("turret_visual_summary"), TEXT("요약"), InWeapon.TurretVisualSummary));
	WeaponSectionViewData->AddChildSection(TurretVisualSectionViewData);

	// [v1.21.0] 터렛 조준 추적 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> TurretAimSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("TurretAim"), TEXT("터렛 조준"), ECFVehicleDebugSectionKind::Subsection, true);
	TurretAimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("turret_aim_current_yaw"), TEXT("현재 Yaw"), FString::Printf(TEXT("%.1f 도"), TurretState.CurrentYawDeg), !InWeapon.bTurretVisualAttached));
	TurretAimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("turret_aim_target_yaw"), TEXT("목표 Yaw"), FString::Printf(TEXT("%.1f 도"), TurretState.TargetYawDeg), !InWeapon.bTurretVisualAttached));
	TurretAimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("turret_aim_current_pitch"), TEXT("현재 Pitch"), FString::Printf(TEXT("%.1f 도"), TurretState.CurrentPitchDeg), !InWeapon.bTurretVisualAttached));
	TurretAimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("turret_aim_target_pitch"), TEXT("목표 Pitch"), FString::Printf(TEXT("%.1f 도"), TurretState.TargetPitchDeg), !InWeapon.bTurretVisualAttached));
	TurretAimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("turret_aim_settled"), TEXT("안정화"), TurretSettledText, InWeapon.bTurretVisualAttached && !TurretState.bTurretSettled));
	TurretAimSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("turret_aim_runtime_summary"), TEXT("요약"), InWeapon.TurretRuntimeSummary));
	WeaponSectionViewData->AddChildSection(TurretAimSectionViewData);

	// [v1.10.0] 활성 WeaponData 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> WeaponDataSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("WeaponData"), TEXT("무기 데이터"), ECFVehicleDebugSectionKind::Subsection, true);
	WeaponDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_data_assigned"), TEXT("지정 여부"), WeaponDataAssignedText, bShouldWarnMissingWeaponData));
	WeaponDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_data_active_id"), TEXT("무기 ID"), InWeapon.ActiveWeaponId.ToString(), bShouldWarnMissingWeaponData));
	WeaponDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_data_mount_compatible"), TEXT("장착 호환"), WeaponDataCompatibleText, bShouldWarnMissingWeaponData || bShouldWarnIncompatibleWeaponData));
	WeaponDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_data_range"), TEXT("Trace 사거리"), FString::Printf(TEXT("%.1f"), InWeapon.ActiveWeaponMaxRange), InWeapon.ActiveWeaponMaxRange <= 0.0f));
	WeaponDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_data_fire_rate"), TEXT("분당 발사속도"), WeaponFireRateText, InWeapon.ActiveWeaponFireRatePerMinute <= 0.0f));
	WeaponDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_data_remaining_cooldown"), TEXT("남은 쿨다운"), FString::Printf(TEXT("%.2f 초"), InWeapon.ActiveWeaponRemainingCooldownSeconds), bIsWeaponOnCooldown));
	WeaponDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_data_last_fire_time"), TEXT("마지막 승인 발사 시간"), FString::Printf(TEXT("%.2f"), InWeapon.LastAcceptedWeaponFireTimeSeconds), InWeapon.LastAcceptedWeaponFireTimeSeconds < 0.0f));
	WeaponDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("weapon_data_summary"), TEXT("요약"), InWeapon.ActiveWeaponSummary));
	WeaponSectionViewData->AddChildSection(WeaponDataSectionViewData);

	// [v1.12.0] 활성 ProjectileData 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> ProjectileDataSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("ProjectileData"), TEXT("발사체 데이터"), ECFVehicleDebugSectionKind::Subsection, true);
	ProjectileDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("projectile_data_assigned"), TEXT("지정 여부"), ProjectileDataAssignedText));
	ProjectileDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("projectile_data_active_id"), TEXT("발사체 ID"), InWeapon.ActiveProjectileId.ToString(), false));
	ProjectileDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("projectile_data_spawn_ready"), TEXT("스폰 준비"), ProjectileSpawnReadyText, false));
	ProjectileDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("projectile_data_fallback"), TEXT("Fallback"), ProjectileFallbackText, false));
	ProjectileDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("projectile_data_execution_summary"), TEXT("전환 요약"), InWeapon.ActiveProjectileExecutionSummary));
	ProjectileDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("projectile_data_summary"), TEXT("요약"), InWeapon.ActiveProjectileSummary));
	WeaponSectionViewData->AddChildSection(ProjectileDataSectionViewData);

	// [v1.22.0] 활성 DamageData 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> DamageDataSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("DamageData"), TEXT("피해 데이터"), ECFVehicleDebugSectionKind::Subsection, true);
	DamageDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_data_assigned"), TEXT("지정 여부"), DamageDataAssignedText, false));
	DamageDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_data_active_id"), TEXT("피해 ID"), InWeapon.ActiveDamageId.ToString(), false));
	DamageDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("damage_data_resolution_summary"), TEXT("해석 경로"), InWeapon.ActiveDamageResolutionSummary));
	DamageDataSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("damage_data_summary"), TEXT("요약"), InWeapon.ActiveDamageSummary));
	WeaponSectionViewData->AddChildSection(DamageDataSectionViewData);

	// [v1.24.0] 마지막 Damage HitContext 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> DamageHitContextSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("DamageHitContext"), TEXT("피해 HitContext"), ECFVehicleDebugSectionKind::Subsection, true);
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_assigned"), TEXT("기록 여부"), DamageHitContextAssignedText, !InWeapon.bHasLastDamageHitContext));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_source"), TEXT("Source"), DamageHitContextSourceText, false));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_hit"), TEXT("Hit"), DamageHitContextHitText, InWeapon.bHasLastDamageHitContext && !LastDamageHitContext.bBlockingHit));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_damage_id"), TEXT("피해 ID"), LastDamageHitContext.DamageId.ToString(), LastDamageHitContext.DamageId.IsNone()));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_weapon_id"), TEXT("무기 ID"), LastDamageHitContext.WeaponId.ToString(), LastDamageHitContext.WeaponId.IsNone()));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_projectile_id"), TEXT("발사체 ID"), LastDamageHitContext.ProjectileId.ToString(), LastDamageHitContext.ProjectileId.IsNone()));
				DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_actor"), TEXT("피격 Actor"), DamageHitContextActorText, InWeapon.bHasLastDamageHitContext && LastDamageHitContext.bBlockingHit && !LastDamageHitContext.HitActor));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_component"), TEXT("피격 컴포넌트"), LastDamageHitContext.HitComponentName.ToString(), InWeapon.bHasLastDamageHitContext && LastDamageHitContext.bBlockingHit && LastDamageHitContext.HitComponentName.IsNone()));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_instigator"), TEXT("발사 주체"), DamageHitContextInstigatorText, InWeapon.bHasLastDamageHitContext && !LastDamageHitContext.InstigatorActor));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_location"), TEXT("피격 위치"), FString::Printf(TEXT("(%.1f, %.1f, %.1f)"), LastDamageHitContext.ImpactLocation.X, LastDamageHitContext.ImpactLocation.Y, LastDamageHitContext.ImpactLocation.Z), false));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_normal"), TEXT("피격 노멀"), FString::Printf(TEXT("(%.2f, %.2f, %.2f)"), LastDamageHitContext.ImpactNormal.X, LastDamageHitContext.ImpactNormal.Y, LastDamageHitContext.ImpactNormal.Z), false));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_incoming"), TEXT("입사 방향"), FString::Printf(TEXT("(%.2f, %.2f, %.2f)"), LastDamageHitContext.IncomingDirection.X, LastDamageHitContext.IncomingDirection.Y, LastDamageHitContext.IncomingDirection.Z), false));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_hit_context_flight"), TEXT("비행 시간"), FString::Printf(TEXT("%.2f 초"), LastDamageHitContext.FlightDurationSeconds), false));
		DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("damage_hit_context_summary"), TEXT("HitContext 요약"), InWeapon.LastDamageHitContextSummary));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("damage_apply_recorded"), TEXT("피해 적용 기록"), InWeapon.bHasLastDamageApplyResult ? TEXT("있음") : TEXT("없음"), !InWeapon.bHasLastDamageApplyResult));
	DamageHitContextSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("damage_apply_summary"), TEXT("피해 적용 결과"), InWeapon.LastDamageApplyResultSummary.IsEmpty() ? TEXT("피해 적용 기록 없음") : InWeapon.LastDamageApplyResultSummary));
		// [v1.32.0] VehicleDefenseComp의 현재 상태와 마지막 전체 피해 결과를 계산 없이 표시하는 방어 하위 섹션입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> VehicleDefenseSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("VehicleDefense"), TEXT("차량 방어"), ECFVehicleDebugSectionKind::Subsection, true);
	VehicleDefenseSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("vehicle_defense_component"), TEXT("방어 컴포넌트"), InWeapon.bHasVehicleDefenseComponent ? TEXT("있음") : TEXT("없음"), !InWeapon.bHasVehicleDefenseComponent));
	VehicleDefenseSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("vehicle_defense_initialized"), TEXT("DefenseData 초기화"), InWeapon.bVehicleDefenseInitialized ? TEXT("예") : TEXT("아니오"), !InWeapon.bVehicleDefenseInitialized && !InWeapon.bUsingLegacyDefenseFallback));
	VehicleDefenseSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("vehicle_defense_mode"), TEXT("피해 경로"), InWeapon.bUsingLegacyDefenseFallback ? TEXT("Legacy Integrity Fallback") : TEXT("Shield → Armor → Integrity"), InWeapon.bUsingLegacyDefenseFallback));
	VehicleDefenseSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("vehicle_defense_state_summary"), TEXT("현재 Shield·6방향 Armor 상태"), InWeapon.VehicleDefenseSummary));
	VehicleDefenseSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("vehicle_defense_last_result_recorded"), TEXT("전체 피해 결과 기록"), InWeapon.bHasLastVehicleDamageResult ? TEXT("있음") : TEXT("없음"), !InWeapon.bHasLastVehicleDamageResult));
	VehicleDefenseSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("vehicle_defense_last_result_summary"), TEXT("마지막 방향·Shield·Armor·Integrity 결과"), InWeapon.LastVehicleDamageResultSummary.IsEmpty() ? TEXT("차량 방어 피해 기록 없음") : InWeapon.LastVehicleDamageResultSummary));
	WeaponSectionViewData->AddChildSection(VehicleDefenseSectionViewData);

	WeaponSectionViewData->AddChildSection(DamageHitContextSectionViewData);

	// [v1.15.0] Projectile Pool 카운트 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> ProjectilePoolSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("ProjectilePool"), TEXT("발사체 Pool"), ECFVehicleDebugSectionKind::Subsection, true);
	ProjectilePoolSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("projectile_pool_component"), TEXT("Pool 컴포넌트"), ProjectilePoolComponentText, bShouldWarnMissingProjectilePool));
	ProjectilePoolSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("projectile_pool_total"), TEXT("전체 Pool 수"), FString::FromInt(InWeapon.TotalPooledProjectileCount), false));
	ProjectilePoolSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("projectile_pool_active"), TEXT("활성 Pool 수"), FString::FromInt(InWeapon.ActivePooledProjectileCount), false));
	ProjectilePoolSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("projectile_pool_inactive"), TEXT("비활성 Pool 수"), FString::FromInt(InWeapon.InactivePooledProjectileCount), false));
	ProjectilePoolSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeMultilineField(TEXT("projectile_pool_last_release"), TEXT("마지막 반환 요약"), InWeapon.LastProjectileReleaseSummary));
	WeaponSectionViewData->AddChildSection(ProjectilePoolSectionViewData);

	// [v1.9.0] 마지막 FireOrigin 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> FireOriginSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("WeaponFireOrigin"), TEXT("발사 원점"), ECFVehicleDebugSectionKind::Subsection, true);
	FireOriginSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_fire_resolved"), TEXT("해결 여부"), LastFireOrigin.bResolved ? TEXT("예") : TEXT("아니오"), !LastFireOrigin.bResolved));
	FireOriginSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_fire_profile"), TEXT("계산 프로파일"), LastFireOrigin.MountProfileId.ToString(), LastFireOrigin.MountProfileId.IsNone()));
	FireOriginSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_fire_slot"), TEXT("위치 슬롯"), LastFireOrigin.LocationSlotId.ToString(), LastFireOrigin.LocationSlotId.IsNone()));
	FireOriginSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_fire_mount_type"), TEXT("장착 타입"), MountTypeText, LastFireOrigin.MountType == ECFVehicleMountType::None));
	FireOriginSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_fire_location"), TEXT("월드 위치"), FString::Printf(TEXT("(%.1f, %.1f, %.1f)"), LastFireOrigin.WorldFireLocation.X, LastFireOrigin.WorldFireLocation.Y, LastFireOrigin.WorldFireLocation.Z), !LastFireOrigin.bResolved));
	FireOriginSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("weapon_fire_direction"), TEXT("월드 방향"), FString::Printf(TEXT("(%.2f, %.2f, %.2f)"), LastFireOrigin.WorldFireDirection.X, LastFireOrigin.WorldFireDirection.Y, LastFireOrigin.WorldFireDirection.Z), !LastFireOrigin.bResolved));
	WeaponSectionViewData->AddChildSection(FireOriginSectionViewData);

	return WeaponSectionViewData;
}

// [v1.5.0] Runtime 카테고리용 Section ViewData를 생성합니다.
TSharedRef<FCFVehicleDebugSectionViewData> UCFVehicleDebugPanelWidget::BuildRuntimeSectionViewData(const FCFVehicleDebugRuntime& InRuntime) const
{
	// [v1.5.0] 생성할 Runtime 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> RuntimeSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("Runtime"), TEXT("런타임"), ECFVehicleDebugSectionKind::Category, bIsRuntimeExpanded);
	RuntimeSectionViewData->NavigationGroup = ECFVehicleDebugNavGroup::Core;
	RuntimeSectionViewData->NavigationOrder = 40;
	RuntimeSectionViewData->bShowInNavigation = true;

	RuntimeSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("runtime_ready"), TEXT("준비"), InRuntime.bRuntimeReady ? TEXT("예") : TEXT("아니오"), true));
	RuntimeSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("runtime_has_drive_comp"), TEXT("주행 컴포넌트"), InRuntime.bHasDriveComponent ? TEXT("예") : TEXT("아니오")));
	RuntimeSectionViewData->AddField(FCFVehicleDebugFieldViewData::MakeLabelValueField(TEXT("runtime_has_wheelsync_comp"), TEXT("휠 동기화 컴포넌트"), InRuntime.bHasWheelSyncComponent ? TEXT("예") : TEXT("아니오")));

	// [v1.5.0] Runtime Summary 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> RuntimeSummarySectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("RuntimeSummary"), TEXT("런타임 요약"), ECFVehicleDebugSectionKind::Subsection, bIsRuntimeSummaryExpanded);
	RuntimeSummarySectionViewData->BodyText = BuildRuntimeSummaryBodyText(InRuntime);
	RuntimeSectionViewData->AddChildSection(RuntimeSummarySectionViewData);

	// [v1.5.0] Last Init 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> RuntimeLastInitSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("RuntimeLastInit"), TEXT("최근 초기화"), ECFVehicleDebugSectionKind::Subsection, bIsRuntimeLastInitExpanded);
	RuntimeLastInitSectionViewData->BodyText = BuildRuntimeLastInitBodyText(InRuntime);
	RuntimeSectionViewData->AddChildSection(RuntimeLastInitSectionViewData);

	// [v1.5.0] Last Validation 하위 섹션 ViewData입니다.
	TSharedRef<FCFVehicleDebugSectionViewData> RuntimeLastValidationSectionViewData =
		FCFVehicleDebugSectionViewData::MakeSection(TEXT("RuntimeLastValidation"), TEXT("최근 검증"), ECFVehicleDebugSectionKind::Subsection, bIsRuntimeLastValidationExpanded);
	RuntimeLastValidationSectionViewData->BodyText = BuildRuntimeLastValidationBodyText(InRuntime);
	RuntimeSectionViewData->AddChildSection(RuntimeLastValidationSectionViewData);

	return RuntimeSectionViewData;
}
