// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-10
// Description: CarFight 공통 Base Widget의 외부 Visual Context Bridge
// Scope: D1-10A Style·Density·Geometry/Typography Scale·Typography Floor를 외부에서 주입받아 Blueprint Visual Layer에 전달합니다.
// Changelog:
// - v1.0.0: 외부 Visual Context 저장, 중복 적용 방지, Geometry/Typography 해석과 Blueprint Refresh Event를 최초 추가.
// Migration:
// - Widget은 Gameplay Actor를 탐색하거나 DataAsset 경로를 직접 Load하지 않습니다.
// - Visual Context가 Null이면 Native CDO Fallback을 사용하며 D1-09B Config Asset 해석 책임은 UCFUISubsystem에 유지합니다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/CFHUDLayoutData.h"
#include "UI/CFUIDensityData.h"
#include "UI/CFUIStyleData.h"
#include "CFStyledWidgetBase.generated.h"

/**
 * CarFight Base Widget이 공통으로 소비하는 외부 Style·Density·Scale Context입니다.
 */
UCLASS(Blueprintable)
class CARFIGHT_RE_API UCFStyledWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	// [v1.0.0] 외부 UI Presenter/Root가 해석한 Style·Density·Scale Context를 한 번에 적용하고 실제 변경이 있을 때만 Visual Refresh를 요청합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Visual", meta=(DisplayName="UI 시각 Context 설정 (Set UI Visual Context)", ToolTip="외부에서 해석한 Style Data, Density Data, Geometry/Typography Scale과 Typography Floor를 적용합니다. 같은 Context를 다시 넣으면 불필요한 Refresh를 발생시키지 않습니다."))
	void SetUIVisualContext(UCFUIStyleData* InStyleData, UCFUIDensityData* InDensityData, float InGeometryScale, float InTypographyScale, const FCFUITypographyFloor& InTypographyFloor);

	// [v1.0.0] 현재 Visual Context를 사용해 Blueprint Visual Layer 갱신 Event를 다시 호출합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Visual", meta=(DisplayName="UI 시각 Style 새로고침 (Refresh UI Visual Style)", ToolTip="현재 저장된 Style·Density·Scale Context로 Blueprint Visual Refresh Event를 다시 호출합니다."))
	void RefreshUIVisualStyle();

	// [v1.0.0] 현재 적용된 Style Data를 반환하며 외부 Context가 Null이면 Native CDO Fallback을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Visual", meta=(DisplayName="UI Style Data 가져오기 (Get UI Style Data)", ToolTip="현재 Widget이 사용하는 Style Data를 반환합니다. 외부 Style이 없으면 Native Safe Fallback을 반환합니다."))
	const UCFUIStyleData* GetUIStyleData() const;

	// [v1.0.0] 현재 적용된 Density Data를 반환하며 외부 Context가 Null이면 Native Standard CDO Fallback을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Visual", meta=(DisplayName="UI Density Data 가져오기 (Get UI Density Data)", ToolTip="현재 Widget이 사용하는 Density Data를 반환합니다. 외부 Density가 없으면 Native Standard Safe Fallback을 반환합니다."))
	const UCFUIDensityData* GetUIDensityData() const;

	// [v1.0.0] Panel 내부 Design Token에 적용할 유효 Geometry Scale을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Visual", meta=(DisplayName="Geometry 스케일 가져오기 (Get Resolved Geometry Scale)", ToolTip="Panel 내부 Padding, Gap, Icon과 Outline에 적용할 Geometry Scale을 반환합니다. 0 이하 값은 안전하게 1로 처리합니다."))
	float GetResolvedGeometryScale() const;

	// [v1.0.0] Design Unit 간격을 현재 Geometry Scale에 맞춘 실효 값으로 변환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Visual", meta=(DisplayName="간격 스케일 해석 (Resolve Scaled Spacing)", ToolTip="Padding, Gap, Icon Size처럼 스케일 대상인 Design Unit 값을 현재 Geometry Scale에 맞춰 반환합니다. MinimumHitSize에는 사용하지 않습니다."))
	float ResolveScaledSpacing(float DesignUnits) const;

	// [v1.0.0] Font Family·Typography Role을 현재 Typography Scale과 Role별 안전 하한이 적용된 Slate Font로 해석합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Visual", meta=(DisplayName="Typography Font 해석 (Resolve Typography Font)", ToolTip="Style Data의 Composite Font와 Typeface를 현재 Typography Scale 및 Role별 최소 실효 크기로 해석합니다."))
	FSlateFontInfo ResolveTypographyFont(ECFUIFontFamilyRole FontFamilyRole, ECFUITypographyRole TypographyRole) const;

	// [v1.0.0] 동일 Context 중복 적용 방지와 테스트 Evidence에 사용할 Visual Context Revision을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Visual", meta=(DisplayName="시각 Context Revision 가져오기 (Get Visual Context Revision)", ToolTip="실제 Visual Refresh가 발생할 때마다 증가하는 Revision입니다. 동일 Context 재적용 여부를 진단하는 데 사용합니다."))
	int32 GetVisualContextRevision() const;

protected:
	// [v1.0.0] Visual Context가 갱신되거나 명시적 Refresh가 요청됐을 때 Blueprint Visual Layer가 Style을 다시 적용하는 Event입니다.
	UFUNCTION(BlueprintImplementableEvent, Category="CarFight|UI|Visual", meta=(DisplayName="UI 시각 Style 변경됨 (On UI Visual Style Changed)", ToolTip="C++ Bridge가 현재 Visual Context를 갱신한 직후 호출됩니다. Gameplay 조회 없이 Widget Tree의 Style·Density 표현만 갱신하세요."))
	void OnUIVisualStyleChanged();

private:
	// [v1.0.0] 두 Typography Floor 구조가 모든 Role에서 같은지 비교합니다.
	bool IsSameTypographyFloor(const FCFUITypographyFloor& Left, const FCFUITypographyFloor& Right) const;

	// [v1.0.0] 지정 Typography Role의 현재 최소 실효 Font Size를 반환합니다.
	int32 ResolveMinimumTypographySize(ECFUITypographyRole TypographyRole) const;

	// [v1.0.0] 외부에서 주입받은 현재 Style Data 참조입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFUIStyleData> StyleData = nullptr;

	// [v1.0.0] 외부에서 주입받은 현재 Density Data 참조입니다.
	UPROPERTY(Transient)
	TObjectPtr<UCFUIDensityData> DensityData = nullptr;

	// [v1.0.0] Panel 내부 Design Token에 적용할 현재 Geometry Scale입니다.
	UPROPERTY(Transient)
	float GeometryScale = 1.0f;

	// [v1.0.0] Typography Token에 적용할 현재 Typography Scale입니다.
	UPROPERTY(Transient)
	float TypographyScale = 1.0f;

	// [v1.0.0] Typography Scale 적용 후에도 지켜야 하는 Role별 최소 실효 Font Size입니다.
	UPROPERTY(Transient)
	FCFUITypographyFloor TypographyFloor;

	// [v1.0.0] 실제 Visual Refresh가 수행된 횟수를 나타내는 진단 Revision입니다.
	UPROPERTY(Transient)
	int32 VisualContextRevision = 0;
};
