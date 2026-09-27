// Copyright (c) CarFight. All Rights Reserved.
// File: CFWeaponGuideTab.h
// Version: v1.2.1
// Date: 2026-09-19
// Description: CF-FQ-055 WEA-P0-04 Guided Weapon Equipment Authoring completion summary와 fail-closed navigation-only handoff UX입니다.
// Changelog:
// - v1.2.1: Mid-review P2 교정으로 버튼 enabled 판정을 lightweight bComplete로 제한하고 integrity 실패 시 stale technical detail을 숨깁니다.
// - v1.2.0: USER completion summary와 context injection 없는 Equipment Builder navigation-only Action을 추가했습니다.
// - v1.1.0: Review 단계 explicit 무장 생성 Action과 durable completion Bundle 표시를 추가했습니다.
// - v1.0.1: Damage exact payload의 inner radius/minimum damage scale과 Ammo Texture2D icon 입력을 normal authoring Page에 추가했습니다.
// - v1.0.0: Stable Step rail, single-scroll Guided Draft pages, Damage/Ammo existing provider preview, Previous/Next validation shell을 추가했습니다.
// Migration:
// - WEA-P0-03 Review의 [무장 생성]을 명시적으로 실행할 때만 신규 child를 durable 저장하며 partial success는 자동 rollback/delete하지 않습니다.
// - EquipmentPreset 조립/저장은 CF-FQ-054 Equipment Builder 책임으로 유지되며 이 탭이 CF-FQ-054를 재개하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "WeaponAuthoring/CFWeaponGuideTypes.h"
#include "Widgets/SCompoundWidget.h"

class FCFWeaponGuideVM;
class SBox;
class SScrollBox;
struct FAssetData;

// 무장 제작 가이드의 Editor-only Native Slate 진입점입니다.
class CARFIGHT_REEDITOR_API SCFWeaponGuideTab : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCFWeaponGuideTab) {}
	SLATE_END_ARGS()

	// Slate shell과 session-local Weapon Guide ViewModel을 초기화합니다.
	void Construct(const FArguments& InArgs);

private:
	// 현재 visible Stable Step 진행 상황을 왼쪽 rail로 구성합니다.
	TSharedRef<SWidget> BuildStepRail() const;

	// 현재 Stable Step에 대응하는 Guided Draft Page를 구성합니다.
	TSharedRef<SWidget> BuildCurrentPage();

	// 기본 이름, 표시 이름, 목적, Template 선택 Page를 구성합니다.
	TSharedRef<SWidget> BuildIdentityPage();

	// 장착 타입과 무기 크기 Page를 구성합니다.
	TSharedRef<SWidget> BuildMountCompatibilityPage();

	// TurretMountData create/reuse와 핵심 기하 Page를 구성합니다.
	TSharedRef<SWidget> BuildMountGeometryPage();

	// WeaponData create/reuse와 발사/탄약/선택 Capability Page를 구성합니다.
	TSharedRef<SWidget> BuildFireBehaviorPage();

	// ProjectileData create/reuse와 핵심 비행값 Page를 구성합니다.
	TSharedRef<SWidget> BuildProjectilePage();

	// DamageData create/reuse와 existing provider exact12 Draft Page를 구성합니다.
	TSharedRef<SWidget> BuildDamagePage();

	// AmmoData create/reuse와 existing provider exact8 Draft Page를 구성합니다.
	TSharedRef<SWidget> BuildAmmoPage();

	// Launcher conditional 설정 Page를 구성합니다.
	TSharedRef<SWidget> BuildLauncherPage();

	// Propulsion conditional 설정 Page를 구성합니다.
	TSharedRef<SWidget> BuildPropulsionPage();

	// Missile Flight conditional 설정 Page를 구성합니다.
	TSharedRef<SWidget> BuildMissileFlightPage();

	// Guidance conditional 설정 Page를 구성합니다.
	TSharedRef<SWidget> BuildGuidancePage();

	// Heat conditional 설정 Page를 구성합니다.
	TSharedRef<SWidget> BuildHeatPage();

	// Charge conditional 설정 Page를 구성합니다.
	TSharedRef<SWidget> BuildChargePage();

	// 전체 Draft validation과 provider mutation0 preview Page를 구성합니다.
	TSharedRef<SWidget> BuildReviewPage();

	// P0-04 completion USER summary와 navigation-only handoff Action을 구성합니다.
	TSharedRef<SWidget> BuildCompletePage();

	// 현재 ViewModel 상태를 반영해 Step rail과 Page host를 재구성합니다.
	void RefreshView();

	// 이전 visible Step으로 이동합니다.
	FReply HandlePrevious();

	// 현재 Step을 검증하고 다음 visible Step으로 이동하거나 Review에서 explicit durable Create를 실행합니다.
	FReply HandleNext();

	// 현재 Step만 fresh 검증해 사용자 상태 메시지를 갱신합니다.
	FReply HandleValidateCurrent();

	// 모든 transient Draft를 새 세션 기본값으로 되돌립니다.
	FReply HandleResetDraft();

	// completion Bundle을 다시 검증한 뒤 Equipment Builder Nomad Tab만 열고 context는 전달하지 않습니다.
	FReply HandleOpenEquipmentBuilder();

	// completion Bundle에서 navigation-only handoff 계약을 만들 수 있는지 반환합니다.
	bool CanOpenEquipmentBuilder() const;

	// Existing TurretMountData picker 선택을 ViewModel에 반영합니다.
	void HandleTurretChanged(const FAssetData& AssetData);

	// Existing WeaponData picker 선택을 ViewModel에 반영합니다.
	void HandleWeaponChanged(const FAssetData& AssetData);

	// Existing ProjectileData picker 선택을 ViewModel에 반영합니다.
	void HandleProjectileChanged(const FAssetData& AssetData);

	// Existing DamageData picker 선택을 ViewModel에 반영합니다.
	void HandleDamageChanged(const FAssetData& AssetData);

	// Existing AmmoData picker 선택을 ViewModel에 반영합니다.
	void HandleAmmoChanged(const FAssetData& AssetData);

	// Turret picker가 표시할 exact object path를 반환합니다.
	FString GetTurretObjectPath() const;

	// Weapon picker가 표시할 exact object path를 반환합니다.
	FString GetWeaponObjectPath() const;

	// Projectile picker가 표시할 exact object path를 반환합니다.
	FString GetProjectileObjectPath() const;

	// Damage picker가 표시할 exact object path를 반환합니다.
	FString GetDamageObjectPath() const;

	// Ammo picker가 표시할 exact object path를 반환합니다.
	FString GetAmmoObjectPath() const;

	// Ammo Icon picker가 표시할 Texture2D object path를 반환합니다.
	FString GetAmmoIconObjectPath() const;

	// Ammo Icon picker 선택을 신규 Ammo Draft에 반영합니다.
	void HandleAmmoIconChanged(const FAssetData& AssetData);

	// 현재 Step과 visible index를 사용자용 문자열로 반환합니다.
	FText GetCurrentStepText() const;

	// ViewModel 또는 UI action의 마지막 사용자 상태를 반환합니다.
	FText GetStatusText() const;

	// 현재 Step이 P0-03 UI에서 다음 또는 explicit Create를 실행할 수 있는지 반환합니다.
	bool CanUseNext() const;

	// Child mode 버튼의 current 선택 상태를 포함한 label을 반환합니다.
	static FText GetChildModeLabel(ECFWeaponGuideChildMode CurrentMode, ECFWeaponGuideChildMode ButtonMode);

	// Template enum을 사용자용 한글 label로 변환합니다.
	static FText GetTemplateLabel(ECFWeaponGuideTemplate Template);

	// MountType enum을 사용자용 한글/영문 label로 변환합니다.
	static FText GetMountTypeLabel(ECFVehicleMountType MountType);

	// WeaponSize enum을 사용자용 label로 변환합니다.
	static FText GetWeaponSizeLabel(ECFVehicleWeaponSize WeaponSize);

	// Weapon Guide session-local ViewModel입니다.
	TSharedPtr<FCFWeaponGuideVM> ViewModel;

	// 동적으로 재구성되는 Stable Step rail host입니다.
	TSharedPtr<SBox> StepRailHost;

	// 동적으로 재구성되는 current Page host입니다.
	TSharedPtr<SBox> PageHost;

	// Current Page 공통 세로 스크롤입니다.
	TSharedPtr<SScrollBox> PageScrollBox;

	// Complete Page 진입 시 1회 authoritative integrity 검증 결과를 캐시합니다. 버튼 IsEnabled는 이 lightweight 값만 읽습니다.
	bool bCompletionHandoffReady = false;

	// UI action에서만 생성된 마지막 상태 메시지입니다.
	FString UiStatusMessage;
};
