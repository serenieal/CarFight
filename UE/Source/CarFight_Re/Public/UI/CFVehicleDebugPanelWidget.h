// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.34.1
// Date: 2026-09-02
// Description: VehicleDebug Panel용 C++ 부모 위젯 클래스 / RTA Runtime Apply 전용 Navigation child integration
// Changelog:
// - v1.34.1: P0에 불필요한 RuntimeApplyWidgetClass reflected override를 제거하고 C++ 기본 UCFRuntimeApplyWidget으로 고정해 UHT dependency와 설정 표면을 최소화.
// - v1.34.0: RuntimeApply Navigation 섹션과 전용 UCFRuntimeApplyWidget child slot을 추가하고 기존 Generic Section ViewData는 read-only 구조로 유지.
// - v1.33.0: 현재 선택 대상의 표시 정보, 추적 상태, Shield·6방향 Armor·재생 상태와 Integrity를 보여주는 Target Navigation 섹션을 추가.
// - v1.25.0: Weapon 섹션에 EquipmentPresetData 지정 여부 / ID / 호환성 / 요약 표시를 추가.
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
// - v1.11.0: Weapon 섹션에 활성 WeaponData 기반 MaxRange / Cooldown 런타임 표시를 추가.
// - v1.10.0: Weapon 섹션에 활성 WeaponData ID, 호환성, 요약 표시를 추가.
// - v1.9.0: VehicleDebug Weapon 카테고리를 Navigation 섹션으로 추가해 WeaponComp 런타임과 FireOrigin 상태를 표시.
// Migration:
// - v1.34.1 RuntimeApply child는 P0에서 C++ 기본 클래스로 고정합니다. WBP 스타일 override가 실제 필요해질 때 별도 UI 작업으로 추가합니다.
// - v1.34.0 RuntimeApply는 기존 VerticalBox_SelectedSectionHost에 전용 C++ child를 교체 표시하므로 WBP_VehicleDebugPanel Asset 수정이 필요 없다. Vehicle/Equipment mutation은 child가 Runtime Apply service에 위임한다.
// - Target 섹션은 기존 동적 Section 레이아웃에 C++ ViewData로 추가되므로 WBP_VehicleDebugPanel의 위젯 트리나 Blueprint 그래프를 수정할 필요가 없다.
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
// - 기존 WeaponData 표시 구조는 유지하고 발사 제한 원본값은 분당 발사속도로 표시한다.
// - 기존 Weapon 섹션 ID와 필드는 유지하고 WeaponData 하위 섹션만 추가한다.
// - 기존 WBP 바인딩 위젯은 유지하고, 동적 Section 레이아웃에서 Weapon 섹션을 추가로 표시한다.
// Scope: VehicleDebug Overview / Drive / Input / Camera / Aim / Target / Weapon / Runtime / RuntimeApply Navigation을 지원하며 RuntimeApply만 전용 interactive child를 사용합니다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/CFDebugPanelViewData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleDebugPanelWidget.generated.h"

class UTextBlock;
class UWidget;
class UVerticalBox;
class APlayerController;
class UCFRuntimeApplyWidget;
class UCFVehicleDebugNavItemWidget;
class UCFVehicleDebugSectionWidget;

/**
 * VehicleDebug Panel용 C++ 부모 위젯 클래스입니다.
 * - 카테고리 데이터 읽기와 패널 본문 구성은 C++가 담당합니다.
 * - 레이아웃과 스타일은 WBP 자식이 담당합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API UCFVehicleDebugPanelWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// [v1.0.0] Panel이 읽을 차량 Pawn 참조를 설정하고 즉시 패널을 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="차량 Pawn 참조 설정 (SetVehiclePawnRef)", ToolTip="Panel이 디버그 정보를 읽어올 차량 Pawn 참조를 설정하고 즉시 갱신을 수행합니다."))
	void SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef);

	// [v1.0.0] 현재 Pawn에서 최신 VehicleDebug 카테고리를 읽어 패널을 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Pawn에서 Panel 갱신 (RefreshFromPawn)", ToolTip="현재 차량 Pawn에서 최신 VehicleDebug 카테고리를 읽어 Panel 표시값을 갱신합니다."))
	void RefreshFromPawn();

	// [v1.0.0] 전달받은 Overview 카테고리를 패널 텍스트 위젯에 반영합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Overview 적용 (ApplyVehicleDebugOverview)", ToolTip="전달받은 VehicleDebug Overview 카테고리를 Panel 텍스트 위젯에 반영합니다."))
	void ApplyVehicleDebugOverview(const FCFVehicleDebugOverview& InOverview);

	// [v1.0.0] 전달받은 Drive 카테고리를 패널 텍스트 위젯에 반영합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Drive 적용 (ApplyVehicleDebugDrive)", ToolTip="전달받은 VehicleDebug Drive 카테고리를 Panel 텍스트 위젯에 반영합니다."))
	void ApplyVehicleDebugDrive(const FCFVehicleDebugDrive& InDrive);

	// [v1.0.0] 전달받은 Input 카테고리를 패널 텍스트 위젯에 반영합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Input 적용 (ApplyVehicleDebugInput)", ToolTip="전달받은 VehicleDebug Input 카테고리를 Panel 텍스트 위젯에 반영합니다."))
	void ApplyVehicleDebugInput(const FCFVehicleDebugInput& InInput);

	// [v1.0.0] 전달받은 Runtime 카테고리를 패널 텍스트 위젯에 반영합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Runtime 적용 (ApplyVehicleDebugRuntime)", ToolTip="전달받은 VehicleDebug Runtime 카테고리를 Panel 텍스트 위젯에 반영합니다."))
	void ApplyVehicleDebugRuntime(const FCFVehicleDebugRuntime& InRuntime);

	// [v1.0.0] 현재 Pawn의 Panel 표시 조건에 따라 위젯 가시성을 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Panel 가시성 갱신 (UpdatePanelVisibility)", ToolTip="현재 차량 Pawn의 Panel 표시 조건에 따라 Panel 위젯 가시성을 갱신합니다."))
	void UpdatePanelVisibility();

	// [v1.1.0] Overview 카테고리 펼침/접힘 상태를 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Overview 접기/펼치기 전환 (ToggleOverviewExpanded)", ToolTip="Overview 카테고리의 펼침/접힘 상태를 전환합니다."))
	void ToggleOverviewExpanded();

	// [v1.4.0] Overview Last Transition 하위 섹션 펼침/접힘 상태를 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Overview Last Transition 접기/펼치기 전환 (ToggleOverviewLastTransitionExpanded)", ToolTip="Overview Last Transition 하위 섹션의 펼침/접힘 상태를 전환합니다."))
	void ToggleOverviewLastTransitionExpanded();

	// [v1.1.0] Drive 카테고리 펼침/접힘 상태를 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Drive 접기/펼치기 전환 (ToggleDriveExpanded)", ToolTip="Drive 카테고리의 펼침/접힘 상태를 전환합니다."))
	void ToggleDriveExpanded();

	// [v1.4.0] Drive Transition 하위 섹션 펼침/접힘 상태를 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Drive Transition 접기/펼치기 전환 (ToggleDriveTransitionExpanded)", ToolTip="Drive Transition 하위 섹션의 펼침/접힘 상태를 전환합니다."))
	void ToggleDriveTransitionExpanded();

	// [v1.1.0] Input 카테고리 펼침/접힘 상태를 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Input 접기/펼치기 전환 (ToggleInputExpanded)", ToolTip="Input 카테고리의 펼침/접힘 상태를 전환합니다."))
	void ToggleInputExpanded();

	// [v1.1.0] Runtime 카테고리 펼침/접힘 상태를 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Runtime 접기/펼치기 전환 (ToggleRuntimeExpanded)", ToolTip="Runtime 카테고리의 펼침/접힘 상태를 전환합니다."))
	void ToggleRuntimeExpanded();

	// [v1.3.0] Runtime Summary 하위 섹션 펼침/접힘 상태를 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Runtime Summary 접기/펼치기 전환 (ToggleRuntimeSummaryExpanded)", ToolTip="Runtime Summary 하위 섹션의 펼침/접힘 상태를 전환합니다."))
	void ToggleRuntimeSummaryExpanded();

	// [v1.3.0] Last Init 하위 섹션 펼침/접힘 상태를 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Last Init 접기/펼치기 전환 (ToggleRuntimeLastInitExpanded)", ToolTip="Last Init 하위 섹션의 펼침/접힘 상태를 전환합니다."))
	void ToggleRuntimeLastInitExpanded();

	// [v1.3.0] Last Validation 하위 섹션 펼침/접힘 상태를 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Last Validation 접기/펼치기 전환 (ToggleRuntimeLastValidationExpanded)", ToolTip="Last Validation 하위 섹션의 펼침/접힘 상태를 전환합니다."))
	void ToggleRuntimeLastValidationExpanded();

	// [v1.2.0] Panel 상호작용 모드를 활성화하고 마우스 커서를 표시합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Panel 상호작용 모드 진입 (EnterPanelInteractionMode)", ToolTip="Panel 상호작용 모드를 활성화하고 마우스 커서를 표시하며 입력 모드를 Game and UI로 전환합니다."))
	void EnterPanelInteractionMode();

	// [v1.2.0] Panel 상호작용 모드를 종료하고 마우스 커서를 숨깁니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Panel 상호작용 모드 종료 (ExitPanelInteractionMode)", ToolTip="Panel 상호작용 모드를 종료하고 마우스 커서를 숨기며 입력 모드를 Game Only로 전환합니다."))
	void ExitPanelInteractionMode();

	// [v1.5.0] 현재 Snapshot 기준 Panel ViewData를 생성해 반환합니다.
	const FCFVehicleDebugPanelViewData& GetCachedPanelViewData() const;

	// [v1.7.0] 표시할 최상위 Section을 SectionId 기준으로 선택하고 Panel을 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Section 선택 (SelectSectionById)", ToolTip="Navigation 또는 Blueprint에서 호출해 현재 Panel Content 영역에 표시할 최상위 Section을 선택합니다."))
	void SelectSectionById(const FString& InSectionId);

	// [v1.7.0] 현재 선택된 SectionId를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="선택된 Section ID 반환 (GetSelectedSectionId)", ToolTip="현재 Panel Content 영역에 표시 중인 최상위 SectionId를 반환합니다."))
	FString GetSelectedSectionId() const;

protected:
	// [v1.0.0] 위젯 생성 직후 현재 참조 기준으로 첫 Panel 갱신을 수행합니다.
	virtual void NativeConstruct() override;

	// [v1.0.0] 옵션이 켜져 있으면 매 프레임 Pawn 기준 Panel 갱신을 수행합니다.
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	// [v1.3.2] Panel 바깥 클릭을 감지해 상호작용 모드를 종료할지 판단합니다.
	virtual FReply NativeOnPreviewMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

	// [v1.3.2] 실제 Panel 본체 루트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> Border_RootPanel = nullptr;

	// [v1.6.0] 동적 Section 렌더링을 붙일 Vertical Box 호스트입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalBox_DynamicSectionHost = nullptr;

	// [v1.7.0] Navigation 항목을 붙일 Vertical Box 호스트입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalBox_NavHost = nullptr;

	// [v1.7.0] 선택된 최상위 Section 하나를 붙일 Vertical Box 호스트입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalBox_SelectedSectionHost = nullptr;

	// [v1.1.0] Overview 본문 컨테이너 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> Container_OverviewBody = nullptr;

	// [v1.0.0] Overview 섹션 제목 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_OverviewTitle = nullptr;

	// [v1.0.0] Overview 섹션 본문 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_OverviewBody = nullptr;

	// [v1.4.0] Overview Last Transition 하위 본문 컨테이너 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> Container_OverviewLastTransitionBody = nullptr;

	// [v1.4.0] Overview Last Transition 제목 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_OverviewLastTransitionTitle = nullptr;

	// [v1.4.0] Overview Last Transition 본문 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_OverviewLastTransitionBody = nullptr;

	// [v1.1.0] Drive 본문 컨테이너 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> Container_DriveBody = nullptr;

	// [v1.0.0] Drive 섹션 제목 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_DriveTitle = nullptr;

	// [v1.0.0] Drive 섹션 본문 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_DriveBody = nullptr;

	// [v1.4.0] Drive Transition 하위 본문 컨테이너 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> Container_DriveTransitionBody = nullptr;

	// [v1.4.0] Drive Transition 제목 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_DriveTransitionTitle = nullptr;

	// [v1.4.0] Drive Transition 본문 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_DriveTransitionBody = nullptr;

	// [v1.1.0] Input 본문 컨테이너 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> Container_InputBody = nullptr;

	// [v1.0.0] Input 섹션 제목 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_InputTitle = nullptr;

	// [v1.0.0] Input 섹션 본문 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_InputBody = nullptr;

	// [v1.1.0] Runtime 본문 컨테이너 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> Container_RuntimeBody = nullptr;

	// [v1.0.0] Runtime 섹션 제목 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RuntimeTitle = nullptr;

	// [v1.0.0] Runtime 섹션 본문 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RuntimeBody = nullptr;

	// [v1.3.0] Runtime Summary 본문 컨테이너 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> Container_RuntimeSummaryBody = nullptr;

	// [v1.3.0] Runtime Summary 제목 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RuntimeSummaryTitle = nullptr;

	// [v1.3.0] Runtime Summary 본문 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RuntimeSummaryBody = nullptr;

	// [v1.3.0] Last Init 본문 컨테이너 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> Container_RuntimeLastInitBody = nullptr;

	// [v1.3.0] Last Init 제목 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RuntimeLastInitTitle = nullptr;

	// [v1.3.0] Last Init 본문 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RuntimeLastInitBody = nullptr;

	// [v1.3.0] Last Validation 본문 컨테이너 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> Container_RuntimeLastValidationBody = nullptr;

	// [v1.3.0] Last Validation 제목 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RuntimeLastValidationTitle = nullptr;

	// [v1.3.0] Last Validation 본문 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_RuntimeLastValidationBody = nullptr;

	// [v1.0.0] Panel이 디버그 정보를 읽어올 차량 Pawn 참조입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="차량 Pawn 참조 (VehiclePawnRef)", ToolTip="현재 Panel이 디버그 정보를 읽어올 차량 Pawn 참조입니다."))
	TObjectPtr<ACFVehiclePawn> VehiclePawnRef = nullptr;

	// [v1.0.0] 현재 Panel에 적용 중인 최신 Overview 캐시입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Overview 캐시 (CachedOverview)", ToolTip="현재 Panel에 표시 중인 최신 VehicleDebug Overview 캐시입니다."))
	FCFVehicleDebugOverview CachedOverview;

	// [v1.0.0] 현재 Panel에 적용 중인 최신 Drive 캐시입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Drive 캐시 (CachedDrive)", ToolTip="현재 Panel에 표시 중인 최신 VehicleDebug Drive 캐시입니다."))
	FCFVehicleDebugDrive CachedDrive;

	// [v1.0.0] 현재 Panel에 적용 중인 최신 Input 캐시입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Input 캐시 (CachedInput)", ToolTip="현재 Panel에 표시 중인 최신 VehicleDebug Input 캐시입니다."))
		FCFVehicleDebugInput CachedInput;

	// [v1.8.0] Cached Camera debug category.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleDebug|Panel")
	FCFVehicleDebugCamera CachedCamera;

	// [v1.8.0] 현재 Panel에 적용 중인 최신 Aim 캐시입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Aim 캐시 (CachedAim)", ToolTip="현재 Panel에 표시 중인 최신 VehicleDebug Aim 캐시입니다."))
		FCFVehicleDebugAim CachedAim;

	// [v1.33.0] 현재 Panel에 적용 중인 최신 선택 대상 캐시입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Target 캐시 (CachedTarget)", ToolTip="현재 Panel에 표시 중인 선택 대상의 표시 정보, 추적 상태, 방어와 내구도 캐시입니다."))
	FCFVehicleDebugTarget CachedTarget;

	// [v1.9.0] 현재 Panel에 적용 중인 최신 Weapon 캐시입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Weapon 캐시 (CachedWeapon)", ToolTip="현재 Panel에 표시 중인 최신 VehicleDebug Weapon 캐시입니다."))
	FCFVehicleDebugWeapon CachedWeapon;

	// [v1.0.0] 현재 Panel에 적용 중인 최신 Runtime 캐시입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Runtime 캐시 (CachedRuntime)", ToolTip="현재 Panel에 표시 중인 최신 VehicleDebug Runtime 캐시입니다."))
	FCFVehicleDebugRuntime CachedRuntime;

	// [v1.0.0] True이면 NativeTick에서 Panel을 자동 갱신합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="매 프레임 자동 갱신 (bAutoRefreshEveryTick)", ToolTip="True이면 NativeTick에서 현재 Pawn 기준 Panel을 자동 갱신합니다."))
	bool bAutoRefreshEveryTick = true;

	// [v1.6.0] 동적 Section 생성에 사용할 공통 위젯 클래스입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="동적 Section 위젯 클래스 (DynamicSectionWidgetClass)", ToolTip="동적 Section 렌더링에 사용할 공통 Section 위젯 클래스입니다. 보통 WBP_VehicleDebugSection을 지정합니다."))
	TSubclassOf<UCFVehicleDebugSectionWidget> DynamicSectionWidgetClass;

	// [v1.7.0] Navigation 항목 생성에 사용할 위젯 클래스입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Navigation Item 위젯 클래스 (NavItemWidgetClass)", ToolTip="Navigation 항목 생성 시 사용할 공통 Nav Item 위젯 클래스입니다. 보통 WBP_VehicleDebugNavItem을 지정합니다."))
	TSubclassOf<UCFVehicleDebugNavItemWidget> NavItemWidgetClass;

	// [v1.7.0] 현재 Panel Content 영역에 표시할 선택된 최상위 SectionId입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="선택된 Section ID (SelectedSectionId)", ToolTip="현재 Panel Content 영역에 표시할 최상위 SectionId입니다. 비어 있거나 유효하지 않으면 첫 번째 표시 가능한 Section으로 대체됩니다."))
	FString SelectedSectionId = TEXT("Overview");

	// [v1.2.0] True이면 Panel이 보일 때 상호작용 모드를 자동으로 활성화합니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="표시 시 상호작용 자동 진입 (bAutoEnterInteractionModeWhenVisible)", ToolTip="True이면 Panel이 보일 때 마우스 커서를 표시하고 입력 모드를 Game and UI로 자동 전환합니다."))
	bool bAutoEnterInteractionModeWhenVisible = true;

	// [v1.1.0] Overview 카테고리 펼침 상태입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Overview 펼침 상태 (bIsOverviewExpanded)", ToolTip="True이면 Overview 카테고리 본문을 펼친 상태로 유지합니다."))
	bool bIsOverviewExpanded = true;

	// [v1.4.0] Overview Last Transition 하위 섹션 펼침 상태입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Overview Last Transition 펼침 상태 (bIsOverviewLastTransitionExpanded)", ToolTip="True이면 Overview Last Transition 하위 섹션 본문을 펼친 상태로 유지합니다."))
	bool bIsOverviewLastTransitionExpanded = false;

	// [v1.1.0] Drive 카테고리 펼침 상태입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Drive 펼침 상태 (bIsDriveExpanded)", ToolTip="True이면 Drive 카테고리 본문을 펼친 상태로 유지합니다."))
	bool bIsDriveExpanded = true;

	// [v1.4.0] Drive Transition 하위 섹션 펼침 상태입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Drive Transition 펼침 상태 (bIsDriveTransitionExpanded)", ToolTip="True이면 Drive Transition 하위 섹션 본문을 펼친 상태로 유지합니다."))
	bool bIsDriveTransitionExpanded = false;

	// [v1.3.5] Input 카테고리 펼침 상태입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Input 펼침 상태 (bIsInputExpanded)", ToolTip="True이면 Input 카테고리 본문을 펼친 상태로 유지합니다."))
	bool bIsInputExpanded = true;

	// [v1.3.5] Runtime 카테고리 펼침 상태입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Runtime 펼침 상태 (bIsRuntimeExpanded)", ToolTip="True이면 Runtime 카테고리 본문을 펼친 상태로 유지합니다."))
	bool bIsRuntimeExpanded = true;

	// [v1.2.0] 현재 Panel 상호작용 모드 활성 여부입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Panel 상호작용 활성 여부 (bIsPanelInteractionModeActive)", ToolTip="True이면 Panel 상호작용 모드가 활성화되어 마우스 커서와 UI 입력 모드가 유지됩니다."))
	bool bIsPanelInteractionModeActive = false;

	// [v1.3.0] Runtime Summary 하위 섹션 펼침 상태입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Runtime Summary 펼침 상태 (bIsRuntimeSummaryExpanded)", ToolTip="True이면 Runtime Summary 하위 섹션 본문을 펼친 상태로 유지합니다."))
	bool bIsRuntimeSummaryExpanded = false;

	// [v1.3.0] Last Init 하위 섹션 펼침 상태입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Last Init 펼침 상태 (bIsRuntimeLastInitExpanded)", ToolTip="True이면 Last Init 하위 섹션 본문을 펼친 상태로 유지합니다."))
	bool bIsRuntimeLastInitExpanded = false;

	// [v1.3.0] Last Validation 하위 섹션 펼침 상태입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Panel", meta=(DisplayName="Last Validation 펼침 상태 (bIsRuntimeLastValidationExpanded)", ToolTip="True이면 Last Validation 하위 섹션 본문을 펼친 상태로 유지합니다."))
	bool bIsRuntimeLastValidationExpanded = false;

private:
	// [v1.5.0] 현재 Snapshot 기준으로 생성된 최신 Panel ViewData 캐시입니다.
	FCFVehicleDebugPanelViewData CachedPanelViewData;

	// [v1.6.0] 생성해 둔 동적 최상위 Section 위젯 캐시 배열입니다.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCFVehicleDebugSectionWidget>> DynamicSectionWidgetArray;

	// [v1.7.0] 생성해 둔 Navigation Item 위젯 캐시 배열입니다.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCFVehicleDebugNavItemWidget>> NavigationItemWidgetArray;

	// [v1.7.0] 선택된 Generic Section 표시용 위젯 캐시입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFVehicleDebugSectionWidget> SelectedSectionWidget = nullptr;

	// [v1.34.0] RuntimeApply Section 선택 시 재사용할 전용 interactive child 위젯 캐시입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFRuntimeApplyWidget> RuntimeApplyWidget = nullptr;

	// [v1.2.3] WBP에서 이름이 맞는 위젯들을 찾아 C++ 멤버 참조를 보강합니다.
	void ResolveWidgetReferences();

	// [v1.2.4] 현재 펼침 상태를 제목 텍스트에 반영합니다.
	void UpdateCategoryHeaderTexts();

	// [v1.0.0] enum 값을 Panel 표시용 문자열로 변환합니다.
	FString ConvertEnumValueToDisplayString(const TCHAR* EnumValueString) const;

	// [v1.0.0] Overview 카테고리를 패널 본문 문자열로 변환합니다.
	FString BuildOverviewBodyText(const FCFVehicleDebugOverview& InOverview) const;

	// [v1.4.0] Overview Last Transition 하위 섹션 본문 문자열을 생성합니다.
	FString BuildOverviewLastTransitionBodyText(const FCFVehicleDebugOverview& InOverview) const;

	// [v1.0.0] Drive 카테고리를 패널 본문 문자열로 변환합니다.
	FString BuildDriveBodyText(const FCFVehicleDebugDrive& InDrive) const;

	// [v1.4.0] Drive Transition 하위 섹션 본문 문자열을 생성합니다.
	FString BuildDriveTransitionBodyText(const FCFVehicleDebugDrive& InDrive) const;

	// [v1.0.0] Input 카테고리를 패널 본문 문자열로 변환합니다.
	FString BuildInputBodyText(const FCFVehicleDebugInput& InInput) const;

	// [v1.0.0] Runtime 카테고리를 패널 본문 문자열로 변환합니다.
	FString BuildRuntimeBodyText(const FCFVehicleDebugRuntime& InRuntime) const;

	// [v1.3.0] Runtime Summary 하위 섹션 본문 문자열을 생성합니다.
	FString BuildRuntimeSummaryBodyText(const FCFVehicleDebugRuntime& InRuntime) const;

	// [v1.3.0] Last Init 하위 섹션 본문 문자열을 생성합니다.
	FString BuildRuntimeLastInitBodyText(const FCFVehicleDebugRuntime& InRuntime) const;

	// [v1.3.0] Last Validation 하위 섹션 본문 문자열을 생성합니다.
	FString BuildRuntimeLastValidationBodyText(const FCFVehicleDebugRuntime& InRuntime) const;

	// [v1.1.0] 긴 요약 문자열을 Panel 가독성에 맞는 멀티라인 문자열로 변환합니다.
	FString FormatLongSummaryForPanel(const FString& InSummaryText, const FString& InKnownPrefix) const;

	// [v1.1.0] 카테고리 본문 컨테이너 가시성을 펼침 상태에 맞게 갱신합니다.
	void UpdateCategoryBodyVisibility();

	// [v1.2.0] 현재 Pawn 기준으로 상호작용 입력을 적용할 PlayerController를 찾습니다.
	APlayerController* ResolveOwningPlayerController() const;

	// [v1.6.0] 현재 WBP가 동적 Section 렌더링 준비가 되었는지 확인합니다.
	bool IsDynamicSectionLayoutReady() const;

	// [v1.7.0] 현재 WBP가 Navigation 렌더링 준비가 되었는지 확인합니다.
	bool IsNavigationLayoutReady() const;

	// [v1.7.0] 현재 WBP가 선택 Section 렌더링 준비가 되었는지 확인합니다.
	bool IsSelectedSectionLayoutReady() const;

	// [v1.6.0] 현재 최상위 섹션 개수에 맞게 동적 Section 위젯을 준비합니다.
	void EnsureDynamicSectionWidgets();

	// [v1.6.0] 현재 CachedPanelViewData 기준으로 동적 Section 위젯을 갱신합니다.
	void RefreshDynamicSectionWidgets();

	// [v1.7.0] 현재 CachedPanelViewData에서 Navigation 항목 배열을 생성합니다.
	TArray<FCFVehicleDebugNavItemViewData> BuildNavigationItemViewDataArray() const;

	// [v1.7.0] 현재 SelectedSectionId가 유효한지 확인하고 필요하면 fallback SectionId로 보정합니다.
	void ResolveSelectedSectionId();

	// [v1.7.0] 현재 SelectedSectionId에 해당하는 Section ViewData를 찾습니다.
	TSharedPtr<FCFVehicleDebugSectionViewData> FindSelectedSectionViewData() const;

	// [v1.7.0] Navigation Item 위젯 개수를 현재 Navigation 항목 수에 맞게 준비합니다.
	void EnsureNavigationItemWidgets(const TArray<FCFVehicleDebugNavItemViewData>& InNavigationItemViewDataArray);

	// [v1.7.0] 현재 Navigation 항목 ViewData 기준으로 Navigation Item 위젯을 갱신합니다.
	void RefreshNavigationItems();

	// [v1.7.0] 선택된 Section 하나를 Content 영역에 갱신합니다.
	void RefreshSelectedSectionWidget();

	// [v1.7.0] 기존 전체 Section 렌더링 fallback을 사용할지 여부를 반환합니다.
	bool ShouldUseLegacyFullSectionRendering() const;

	// [v1.5.0] 현재 Snapshot 기준 최신 Panel ViewData를 생성합니다.
	FCFVehicleDebugPanelViewData BuildVehicleDebugPanelViewData() const;

	// [v1.5.0] Overview 카테고리용 Section ViewData를 생성합니다.
	TSharedRef<FCFVehicleDebugSectionViewData> BuildOverviewSectionViewData(const FCFVehicleDebugOverview& InOverview) const;

	// [v1.5.0] Drive 카테고리용 Section ViewData를 생성합니다.
	TSharedRef<FCFVehicleDebugSectionViewData> BuildDriveSectionViewData(const FCFVehicleDebugDrive& InDrive) const;

	// [v1.5.0] Input 카테고리용 Section ViewData를 생성합니다.
		TSharedRef<FCFVehicleDebugSectionViewData> BuildInputSectionViewData(const FCFVehicleDebugInput& InInput) const;

	// [v1.8.0] Camera category section ViewData.
	TSharedRef<FCFVehicleDebugSectionViewData> BuildCameraSectionViewData(const FCFVehicleDebugCamera& InCamera) const;

	// [v1.8.0] Aim 카테고리용 Section ViewData를 생성합니다.
		TSharedRef<FCFVehicleDebugSectionViewData> BuildAimSectionViewData(const FCFVehicleDebugAim& InAim) const;

	// [v1.33.0] 현재 선택 대상 카테고리용 Section ViewData를 생성합니다.
	TSharedRef<FCFVehicleDebugSectionViewData> BuildTargetSectionViewData(const FCFVehicleDebugTarget& InTarget) const;

	// [v1.9.0] Weapon 카테고리용 Section ViewData를 생성합니다.
	TSharedRef<FCFVehicleDebugSectionViewData> BuildWeaponSectionViewData(const FCFVehicleDebugWeapon& InWeapon) const;

	// [v1.34.0] 전용 interactive child를 선택할 RuntimeApply Navigation Section ViewData를 생성합니다.
	TSharedRef<FCFVehicleDebugSectionViewData> BuildRuntimeApplySectionViewData() const;

	// [v1.5.0] Runtime 카테고리용 Section ViewData를 생성합니다.
	TSharedRef<FCFVehicleDebugSectionViewData> BuildRuntimeSectionViewData(const FCFVehicleDebugRuntime& InRuntime) const;
};
