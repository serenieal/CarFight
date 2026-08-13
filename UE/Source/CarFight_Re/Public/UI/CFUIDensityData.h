// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-10
// Description: CarFight UI Density Profile DataAsset 타입
// Scope: D1-09A Compact·Standard·Expanded의 Padding·행·아이콘·텍스트 비율과 Caption 정책을 C++ 타입으로 제공합니다.
// Changelog:
// - v1.0.0: Caption Policy, Density Token과 UCFUIDensityData Native Standard Fallback을 최초 추가.
// Migration:
// - Density는 정보 의미를 삭제하지 않으며 MinimumHitSize·Critical 상태·Knowledge State를 소유하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "UI/CFUIStyleData.h"
#include "CFUIDensityData.generated.h"

/**
 * Density Profile이 보조 Caption을 어느 수준까지 표시할지 정의합니다.
 */
UENUM(BlueprintType)
enum class ECFUICaptionPolicy : uint8
{
	CoreOnly UMETA(DisplayName="핵심만 (Core Only)"),
	Contextual UMETA(DisplayName="상황별 (Contextual)"),
	Expanded UMETA(DisplayName="확장 표시 (Expanded)")
};

/**
 * 하나의 Density Profile이 공통 Widget에 전달할 Geometry와 표시 비율 Token입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIDensityTokens
{
	GENERATED_BODY()

	// [v1.0.0] Standard Density 기본값을 Native Fallback으로 준비합니다.
	FCFUIDensityTokens();

	// [v1.0.0] Panel 가장자리에서 내부 Content까지의 기본 Padding입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="패널 Padding (Panel Padding)"))
	float PanelPadding = 24.0f;

	// [v1.0.0] 같은 Panel 안의 직접 인접 요소 사이 기본 간격입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="패널 내부 간격 (Panel Inner Gap)"))
	float PanelInnerGap = 12.0f;

	// [v1.0.0] 서로 다른 정보 Section 사이 기본 간격입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="섹션 간격 (Section Gap)"))
	float SectionGap = 16.0f;

	// [v1.0.0] 연속 InfoRow 사이 기본 간격입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="정보 행 간격 (Info Row Gap)"))
	float InfoRowGap = 8.0f;

	// [v1.0.0] Panel Header 기본 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="헤더 높이 (Header Height)"))
	float HeaderHeight = 48.0f;

	// [v1.0.0] 일반 InfoRow 기본 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="정보 행 높이 (Info Row Height)"))
	float InfoRowHeight = 36.0f;

	// [v1.0.0] 핵심 값이 포함된 큰 InfoRow 기본 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="대형 정보 행 높이 (Large Info Row Height)"))
	float InfoRowHeightLarge = 44.0f;

	// [v1.0.0] 짧은 상태 Chip의 기본 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="Chip 높이 (Chip Height)"))
	float ChipHeight = 28.0f;

	// [v1.0.0] 작은 Semantic Icon 기본 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="작은 아이콘 크기 (Icon Small)"))
	float IconSmall = 20.0f;

	// [v1.0.0] 중간 Semantic Icon 기본 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="중간 아이콘 크기 (Icon Medium)"))
	float IconMedium = 24.0f;

	// [v1.0.0] 큰 Semantic Icon 기본 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="큰 아이콘 크기 (Icon Large)"))
	float IconLarge = 32.0f;

	// [v1.0.0] 핵심 Value Typography에 추가로 적용할 Density 비율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="주요 값 비율 (Primary Value Scale)"))
	float PrimaryValueScale = 1.0f;

	// [v1.0.0] 보조 텍스트에 추가로 적용할 Density 비율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="보조 텍스트 비율 (Secondary Text Scale)"))
	float SecondaryTextScale = 1.0f;

	// [v1.0.0] 공통 Outline Token에 추가로 적용할 Density 비율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="외곽선 비율 (Outline Scale)"))
	float OutlineScale = 1.0f;

	// [v1.0.0] 보조 Caption을 표시할 기본 정책입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="Caption 정책 (Caption Policy)"))
	ECFUICaptionPolicy CaptionPolicy = ECFUICaptionPolicy::Contextual;
};

/**
 * 공통 Widget이 소비할 Density Preset 한 개를 소유하는 DataAsset입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFUIDensityData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] Standard Density를 Native Safe Fallback으로 준비합니다.
	UCFUIDensityData();

	// [v1.0.0] 이 DataAsset의 논리 Density Preset입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="Density Preset", ToolTip="Compact, Standard, Expanded 또는 명시적 Custom 중 하나입니다."))
	ECFUIDensityPreset Preset = ECFUIDensityPreset::Standard;

	// [v1.0.0] Widget Geometry와 표시 비율에 적용할 실제 Density Token입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Density", meta=(DisplayName="Density Token", ToolTip="Padding, Gap, Row, Icon, Text Scale과 Caption Policy를 소유합니다."))
	FCFUIDensityTokens Tokens;

	// [v1.0.0] 지정 Preset의 승인 기본값을 현재 Tokens에 적용합니다.
	void ApplyPresetDefaults(ECFUIDensityPreset NewPreset);

	// [v1.0.0] Density Token이 음수 Geometry나 0 이하 Scale을 포함하지 않는지 검증합니다.
	bool ValidateDensityData(FString& OutFailureReason) const;
};
