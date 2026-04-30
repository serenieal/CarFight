// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-04-27
// Description: VehicleDebug Panel의 데이터 기반 렌더링과 Navigation 표시를 위한 공통 ViewData 구조입니다.
// Scope: 정적 WBP 패널을 Section / Field / Navigation 기반 구조로 전환하기 위한 C++ 데이터 모델입니다.

#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"

// VehicleDebug Panel의 공통 필드 표시 형태를 정의합니다.
enum class ECFVehicleDebugFieldStyle : uint8
{
	// 기본 `라벨: 값` 형태입니다.
	Default,

	// 중요한 값을 강조할 때 사용합니다.
	Important,

	// 긴 여러 줄 값을 표시할 때 사용합니다.
	Multiline
};

// VehicleDebug Panel의 공통 섹션 종류를 정의합니다.
enum class ECFVehicleDebugSectionKind : uint8
{
	// 상위 카테고리 섹션입니다.
	Category,

	// 상위 카테고리 내부의 하위 상세 섹션입니다.
	Subsection,

	// 최근 이벤트 같은 리스트형 섹션입니다.
	EventList
};

// VehicleDebug Panel Navigation에서 사용할 카테고리 그룹입니다.
enum class ECFVehicleDebugNavGroup : uint8
{
	// 핵심 주행/입력/런타임 정보를 묶는 기본 그룹입니다.
	Core,

	// 차량 하위 시스템 정보를 묶는 그룹입니다.
	Vehicle,

	// 문제 추적과 검증 정보를 묶는 그룹입니다.
	Diagnostics,

	// 네트워크, 소유권, AI 등 고급 정보를 묶는 그룹입니다.
	Advanced
};

// VehicleDebug Panel의 Navigation 표시용 항목 ViewData입니다.
struct FCFVehicleDebugNavItemViewData
{
	// Navigation 항목과 연결될 Section 고유 ID입니다.
	FString SectionId;

	// Navigation 버튼에 표시할 이름입니다.
	FString DisplayText;

	// Navigation 항목이 속한 그룹입니다.
	ECFVehicleDebugNavGroup NavigationGroup = ECFVehicleDebugNavGroup::Core;

	// 같은 그룹 안에서 표시될 순서입니다.
	int32 NavigationOrder = 0;

	// 버튼 우측에 표시할 짧은 배지 문자열입니다.
	FString BadgeText;

	// 현재 항목을 Navigation에 표시할지 여부입니다.
	bool bIsVisible = true;

	// 현재 선택된 항목인지 여부입니다.
	bool bIsSelected = false;
};

// VehicleDebug Panel의 가장 작은 표시 단위인 필드 ViewData입니다.
struct FCFVehicleDebugFieldViewData
{
	// 필드의 고유 식별자입니다.
	FString FieldId;

	// 필드 좌측에 표시할 라벨 문자열입니다.
	FString LabelText;

	// 필드 우측 또는 본문에 표시할 값 문자열입니다.
	FString ValueText;

	// 필드의 툴팁 문자열입니다.
	FString TooltipText;

	// 필드 표시 스타일입니다.
	ECFVehicleDebugFieldStyle FieldStyle = ECFVehicleDebugFieldStyle::Default;

	// 중요 필드인지 여부입니다.
	bool bIsImportant = false;

	// 현재 필드를 표시할지 여부입니다.
	bool bIsVisible = true;

	// 기본 `라벨: 값` 필드 ViewData를 생성합니다.
	static FCFVehicleDebugFieldViewData MakeLabelValueField(
		const FString& InFieldId,
		const FString& InLabelText,
		const FString& InValueText,
		bool bInIsImportant = false,
		const FString& InTooltipText = FString());

	// 여러 줄 값 표시용 필드 ViewData를 생성합니다.
	static FCFVehicleDebugFieldViewData MakeMultilineField(
		const FString& InFieldId,
		const FString& InLabelText,
		const FString& InValueText,
		const FString& InTooltipText = FString());
};

// VehicleDebug Panel의 공통 섹션 ViewData입니다.
struct FCFVehicleDebugSectionViewData
{
	// 섹션의 고유 식별자입니다.
	FString SectionId;

	// 섹션 헤더에 표시할 제목 문자열입니다.
	FString TitleText;

	// 섹션 본문에 직접 표시할 여러 줄 문자열입니다.
	FString BodyText;

	// 섹션 분류입니다.
	ECFVehicleDebugSectionKind SectionKind = ECFVehicleDebugSectionKind::Category;

	// Navigation에서 사용할 그룹입니다.
	ECFVehicleDebugNavGroup NavigationGroup = ECFVehicleDebugNavGroup::Core;

	// 같은 Navigation 그룹 안에서의 표시 순서입니다.
	int32 NavigationOrder = 0;

	// 이 Section을 Navigation 목록에 표시할지 여부입니다.
	bool bShowInNavigation = true;

	// Navigation 항목 우측에 표시할 짧은 배지 문자열입니다.
	FString BadgeText;

	// 섹션 기본 펼침 상태입니다.
	bool bDefaultExpanded = true;

	// 현재 섹션 표시 여부입니다.
	bool bIsVisible = true;

	// 섹션 내부에 표시할 필드 배열입니다.
	TArray<FCFVehicleDebugFieldViewData> FieldArray;

	// 현재 섹션 아래에 표시할 하위 섹션 배열입니다.
	TArray<TSharedPtr<FCFVehicleDebugSectionViewData>> ChildSectionArray;

	// 새로운 섹션 ViewData를 생성합니다.
	static TSharedRef<FCFVehicleDebugSectionViewData> MakeSection(
		const FString& InSectionId,
		const FString& InTitleText,
		ECFVehicleDebugSectionKind InSectionKind,
		bool bInDefaultExpanded = true);

	// 현재 섹션에 필드 하나를 추가합니다.
	void AddField(const FCFVehicleDebugFieldViewData& InFieldViewData);

	// 현재 섹션에 하위 섹션 하나를 추가합니다.
	void AddChildSection(const TSharedPtr<FCFVehicleDebugSectionViewData>& InChildSectionViewData);
};

// VehicleDebug Panel 전체 렌더링 입력 데이터입니다.
struct FCFVehicleDebugPanelViewData
{
	// Panel 데이터의 고유 식별자입니다.
	FString PanelId;

	// Panel 제목 문자열입니다.
	FString PanelTitleText;

	// ViewData를 생성한 프레임 번호입니다.
	uint64 GeneratedFrameNumber = 0;

	// ViewData를 생성한 시각(초)입니다.
	double GeneratedTimeSeconds = 0.0;

	// Panel 최상위 섹션 배열입니다.
	TArray<TSharedPtr<FCFVehicleDebugSectionViewData>> TopLevelSectionArray;

	// 현재 Panel ViewData를 초기화합니다.
	void Reset();

	// 최상위 섹션 하나를 추가합니다.
	void AddTopLevelSection(const TSharedPtr<FCFVehicleDebugSectionViewData>& InSectionViewData);
};
