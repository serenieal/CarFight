// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-04-27
// Description: VehicleDebug Panel의 공통 Section 렌더링용 C++ 부모 위젯입니다.
// Scope: Section ViewData를 받아 제목/본문/하위 섹션을 공통 구조로 갱신하기 위한 기반 위젯입니다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/CFDebugPanelViewData.h"
#include "CFVehicleDebugSectionWidget.generated.h"

class UTextBlock;
class UWidget;
class UVerticalBox;
class UCFVehicleDebugFieldRowWidget;

/**
 * VehicleDebug Panel의 공통 섹션 렌더링용 C++ 부모 위젯입니다.
 * - Header 입력 진입점은 WBP에서 연결할 수 있게 열어 둡니다.
 * - C++는 섹션 데이터 적용과 하위 섹션 위젯 생성/갱신을 담당합니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API UCFVehicleDebugSectionWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// [v1.0.0] 현재 섹션 ViewData를 설정하고 즉시 위젯 표시를 갱신합니다.
	void SetSectionViewData(const FCFVehicleDebugSectionViewData& InSectionViewData);

	// [v1.0.0] 현재 캐시된 섹션 ViewData 기준으로 제목/본문/하위 섹션 표시를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Section", meta=(DisplayName="섹션 위젯 갱신 (RefreshFromSectionViewData)", ToolTip="현재 캐시된 섹션 ViewData 기준으로 제목/본문/하위 섹션 표시를 갱신합니다."))
	void RefreshFromSectionViewData();

	// [v1.0.0] 현재 섹션 펼침 상태를 반전하고 표시를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|VehicleDebug|Section", meta=(DisplayName="섹션 접기/펼치기 전환 (ToggleSectionExpanded)", ToolTip="현재 섹션 펼침 상태를 반전하고 제목/본문/하위 섹션 표시를 갱신합니다."))
	void ToggleSectionExpanded();

	// [v1.0.0] 현재 캐시된 섹션 ViewData를 반환합니다.
	const FCFVehicleDebugSectionViewData& GetCachedSectionViewData() const;

protected:
	// [v1.0.0] 위젯 생성 직후 현재 캐시 기준으로 첫 섹션 갱신을 수행합니다.
	virtual void NativeConstruct() override;

	// [v1.0.0] Section 본문 컨테이너 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UWidget> Container_Body = nullptr;

	// [v1.0.0] Section 제목 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Title = nullptr;

	// [v1.0.0] Section 본문 텍스트 위젯 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> Text_Body = nullptr;

	// [v1.1.0] Field Row 위젯을 붙일 Vertical Box 호스트 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalBox_FieldHost = nullptr;

	// [v1.0.0] 하위 Section 위젯을 붙일 Vertical Box 호스트 참조입니다.
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UVerticalBox> VerticalBox_ChildSectionHost = nullptr;

	// [v1.1.0] Field Row 생성에 사용할 위젯 클래스입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Section", meta=(DisplayName="Field Row 위젯 클래스 (FieldRowWidgetClass)", ToolTip="필드 Row 생성 시 사용할 공통 Field Row 위젯 클래스입니다. 보통 WBP_VehicleDebugFieldRow를 지정합니다."))
	TSubclassOf<UCFVehicleDebugFieldRowWidget> FieldRowWidgetClass;

	// [v1.0.0] 하위 Section 생성에 사용할 위젯 클래스입니다. 비어 있으면 현재 Section 위젯 클래스가 fallback으로 사용됩니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Section", meta=(DisplayName="하위 Section 위젯 클래스 (ChildSectionWidgetClass)", ToolTip="하위 섹션 생성 시 사용할 공통 Section 위젯 클래스입니다. 보통 WBP_VehicleDebugSection을 지정합니다."))
	TSubclassOf<UCFVehicleDebugSectionWidget> ChildSectionWidgetClass;

	// [v1.2.0] 하위 Section 위젯을 상위 Section보다 안쪽에 보이게 하는 왼쪽 들여쓰기 값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Section", meta=(DisplayName="하위 Section 왼쪽 들여쓰기 (ChildSectionIndentLeft)", ToolTip="하위 섹션 버튼이 상위 카테고리와 구분되도록 왼쪽에 적용할 여백입니다."))
	float ChildSectionIndentLeft = 14.0f;

	// [v1.2.0] 하위 Section 위젯 사이에 적용할 위쪽 여백 값입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|VehicleDebug|Section", meta=(DisplayName="하위 Section 위쪽 여백 (ChildSectionTopPadding)", ToolTip="하위 섹션들이 서로 붙어 보이지 않도록 위쪽에 적용할 여백입니다."))
	float ChildSectionTopPadding = 2.0f;

	// [v1.0.0] 기본 펼침 여부를 초기화할 때 사용할 현재 섹션 캐시입니다.
	FCFVehicleDebugSectionViewData CachedSectionViewData;

	// [v1.0.0] 현재 섹션의 펼침 상태입니다.
	bool bIsSectionExpanded = true;

	// [v1.0.1] 마지막으로 적용한 섹션 식별자입니다.
	FString LastAppliedSectionId;

	// [v1.0.0] 생성해 둔 하위 Section 위젯 캐시 배열입니다.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCFVehicleDebugSectionWidget>> ChildSectionWidgetArray;

	// [v1.1.0] 생성해 둔 Field Row 위젯 캐시 배열입니다.
	UPROPERTY(Transient)
	TArray<TObjectPtr<UCFVehicleDebugFieldRowWidget>> FieldRowWidgetArray;

private:
	// [v1.0.0] 현재 섹션 본문 문자열을 생성합니다.
	FString BuildSectionBodyText() const;

	// [v1.0.0] 현재 펼침 상태를 제목 문자열에 반영합니다.
	void UpdateSectionHeaderText();

	// [v1.0.0] 현재 펼침 상태를 본문/하위 섹션 가시성에 반영합니다.
	void UpdateSectionBodyVisibility();

	// [v1.1.0] 현재 WBP가 Field Row 렌더링 준비가 되었는지 확인합니다.
	bool IsFieldRowLayoutReady() const;

	// [v1.1.0] 현재 Field ViewData 수에 맞게 Field Row 위젯을 준비합니다.
	void EnsureFieldRowWidgets();

	// [v1.1.0] 현재 캐시 기준으로 Field Row 위젯 데이터를 갱신합니다.
	void RefreshFieldRowWidgets();

	// [v1.1.1] 하위 Section 생성에 사용할 위젯 클래스를 반환합니다.
	TSubclassOf<UCFVehicleDebugSectionWidget> ResolveChildSectionWidgetClass() const;

	// [v1.0.0] 현재 하위 섹션 ViewData 수에 맞게 하위 Section 위젯을 준비합니다.
	void EnsureChildSectionWidgets();

	// [v1.0.0] 현재 캐시 기준으로 하위 Section 위젯 데이터를 갱신합니다.
	void RefreshChildSectionWidgets();
};
