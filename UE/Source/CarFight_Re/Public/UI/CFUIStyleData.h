// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-10
// Description: CarFight 공통 UI Style Token과 DataAsset 타입
// Scope: D1-08 Style Token, D1-09 Font/Icon Asset Binding과 의미 기반 시각 자산 해석 계약을 제공합니다.
// Changelog:
// - v1.2.0: D1-11 Production UI Rework를 위해 Semantic ID→Soft Icon Asset Resolver와 Icon Set 중복/무효 매핑 검증 계약을 추가.
// - v1.1.0: UI/Numeric UFont Binding, Font Family Role, Weight→Typeface 해석과 FSlateFontInfo 생성 계약을 추가.
// - v1.0.0: UI Style Token 구조체, Density·Typography·Color 의미 enum, UCFUIStyleData와 Native Fallback 검증 계약을 최초 추가.
// Migration:
// - DA_CFUIStyle이 로드되면 Font_CFUI·Font_CFNumeric을 Hard Reference로 보유하고 Widget은 콘텐츠 경로를 직접 Load하지 않습니다.
// - D1-09B Font Asset이 아직 없으면 Font Object는 Null이어도 Native Style Fallback 계약은 유지됩니다.
// - Semantic Icon은 Soft Reference를 반환할 뿐 이 타입에서 동기 Load하지 않습니다. 실제 로드 수명은 UI Root/Presenter 또는 Editor Assetization 단계가 소유합니다.
// - 기존 AimReticle·TargetSelect·Pause Widget 동작은 이 타입 확장만으로 변경되지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Fonts/SlateFontInfo.h"
#include "CFUIStyleData.generated.h"

class UFont;

/**
 * Style Data에서 공통으로 참조할 의미 기반 색상 Token입니다.
 */
UENUM(BlueprintType)
enum class ECFUIColorToken : uint8
{
	SurfaceBase UMETA(DisplayName="기본 표면 (Surface Base)"),
	SurfaceRaised UMETA(DisplayName="상위 표면 (Surface Raised)"),
	SurfaceOverlay UMETA(DisplayName="오버레이 표면 (Surface Overlay)"),
	SurfaceSoft UMETA(DisplayName="소프트 표면 (Surface Soft)"),
	DimOverlay UMETA(DisplayName="화면 어둡게 (Dim Overlay)"),
	LineSubtle UMETA(DisplayName="보조 선 (Line Subtle)"),
	LineDefault UMETA(DisplayName="기본 선 (Line Default)"),
	LineStrong UMETA(DisplayName="강조 선 (Line Strong)"),
	TextPrimary UMETA(DisplayName="주 텍스트 (Text Primary)"),
	TextSecondary UMETA(DisplayName="보조 텍스트 (Text Secondary)"),
	TextMuted UMETA(DisplayName="약한 텍스트 (Text Muted)"),
	TextDisabled UMETA(DisplayName="비활성 텍스트 (Text Disabled)"),
	TextOnAccent UMETA(DisplayName="강조면 텍스트 (Text On Accent)"),
	AccentTactical UMETA(DisplayName="전술 강조 (Accent Tactical)"),
	AccentHover UMETA(DisplayName="호버 강조 (Accent Hover)"),
	AccentPressed UMETA(DisplayName="눌림 강조 (Accent Pressed)"),
	StateNotice UMETA(DisplayName="알림 상태 (State Notice)"),
	StateCaution UMETA(DisplayName="주의 상태 (State Caution)"),
	StateDanger UMETA(DisplayName="위험 상태 (State Danger)"),
	StateCritical UMETA(DisplayName="치명 상태 (State Critical)"),
	StateDisabled UMETA(DisplayName="비활성 상태 (State Disabled)"),
	StateSuccess UMETA(DisplayName="성공 상태 (State Success)"),
	Shield UMETA(DisplayName="쉴드 (Shield)"),
	Armor UMETA(DisplayName="장갑 (Armor)"),
	Integrity UMETA(DisplayName="차량 내구도 (Integrity)"),
	Friendly UMETA(DisplayName="아군 관계 (Friendly)"),
	Neutral UMETA(DisplayName="중립 관계 (Neutral)"),
	Hostile UMETA(DisplayName="적대 관계 (Hostile)"),
	Unknown UMETA(DisplayName="미식별 (Unknown)"),
	Estimated UMETA(DisplayName="추정 정보 (Estimated)")
};

/**
 * Typography Role이 사용할 논리적 Font Weight입니다.
 */
UENUM(BlueprintType)
enum class ECFUIFontWeight : uint8
{
	Regular UMETA(DisplayName="일반 (Regular)"),
	Medium UMETA(DisplayName="중간 (Medium)"),
	SemiBold UMETA(DisplayName="세미볼드 (SemiBold)"),
	Bold UMETA(DisplayName="볼드 (Bold)")
};

/**
 * Widget이 실제 Font Asset 경로 대신 요청할 Font Family 역할입니다.
 */
UENUM(BlueprintType)
enum class ECFUIFontFamilyRole : uint8
{
	UI UMETA(DisplayName="UI 폰트 (UI Font)"),
	Numeric UMETA(DisplayName="숫자 폰트 (Numeric Font)")
};

/**
 * Widget이 실제 Font 파일 대신 요청할 Typography 의미 Role입니다.
 */
UENUM(BlueprintType)
enum class ECFUITypographyRole : uint8
{
	DisplayXL UMETA(DisplayName="최상위 숫자 (Display XL)"),
	DisplayL UMETA(DisplayName="대형 숫자 (Display L)"),
	ValueM UMETA(DisplayName="중형 값 (Value M)"),
	ValueS UMETA(DisplayName="소형 값 (Value S)"),
	HeadingL UMETA(DisplayName="대형 제목 (Heading L)"),
	HeadingM UMETA(DisplayName="중형 제목 (Heading M)"),
	Body UMETA(DisplayName="본문 (Body)"),
	Label UMETA(DisplayName="레이블 (Label)"),
	Caption UMETA(DisplayName="캡션 (Caption)")
};

/**
 * UI Motion Token에서 사용하는 제한된 Easing 종류입니다.
 */
UENUM(BlueprintType)
enum class ECFUIEasing : uint8
{
	EaseOutCubic UMETA(DisplayName="Ease Out Cubic"),
	EaseOutQuad UMETA(DisplayName="Ease Out Quad"),
	EaseInOutQuad UMETA(DisplayName="Ease In Out Quad"),
	EaseInOutSine UMETA(DisplayName="Ease In Out Sine")
};

/**
 * 전체 UI와 Panel이 선택할 표시 밀도 Preset입니다.
 */
UENUM(BlueprintType)
enum class ECFUIDensityPreset : uint8
{
	Compact UMETA(DisplayName="컴팩트 (Compact)"),
	Standard UMETA(DisplayName="표준 (Standard)"),
	Expanded UMETA(DisplayName="확장 (Expanded)"),
	Custom UMETA(DisplayName="사용자 지정 (Custom)")
};

/**
 * 한 Typography Role의 크기·행높이·Weight를 정의합니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUITypographyStyle
{
	GENERATED_BODY()

	FCFUITypographyStyle();

	// [v1.0.0] 2560×1440 기준 기본 글자 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="글자 크기 (Font Size)", ToolTip="2560×1440 기준 Typography Role의 기본 글자 크기입니다. 1080p에서는 별도 안전 하한을 적용합니다."))
	int32 FontSize = 20;

	// [v1.0.0] 한 줄 텍스트의 기본 행 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="행 높이 (Line Height)", ToolTip="Typography Role 한 줄의 기본 행 높이입니다."))
	int32 LineHeight = 26;

	// [v1.0.0] 실제 Font Face 이름과 분리된 논리적 Weight입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="글자 굵기 (Font Weight)", ToolTip="실제 Font Face 경로가 아니라 Style Data가 해석하는 논리적 Font Weight입니다."))
	ECFUIFontWeight Weight = ECFUIFontWeight::Regular;
};

/**
 * CarFight 전역 의미 색상 Token 값입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIColorTokens
{
		GENERATED_BODY()

	// [v1.0.0] 승인된 CarFight 의미 색상 Palette 기본값을 준비합니다.
	FCFUIColorTokens();

	// [v1.0.0] 일반 HUD 패널의 기본 배경색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="기본 표면 (Surface Base)", ToolTip="일반 HUD 패널의 기본 배경색과 Alpha입니다."))
	FLinearColor SurfaceBase;

	// [v1.0.0] 선택 패널·Header 등 한 단계 높은 표면색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="상위 표면 (Surface Raised)", ToolTip="선택 패널, Header와 한 단계 높은 표면에 사용하는 색상입니다."))
	FLinearColor SurfaceRaised;

	// [v1.0.0] Pause·Modal 등 강한 분리 표면색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="오버레이 표면 (Surface Overlay)", ToolTip="Pause와 Modal처럼 월드와 강하게 분리해야 하는 표면색입니다."))
	FLinearColor SurfaceOverlay;

	// [v1.0.0] 가벼운 그룹과 Radar 내부에 사용하는 약한 표면색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="소프트 표면 (Surface Soft)", ToolTip="가벼운 그룹 배경과 Radar 내부에 사용하는 낮은 불투명도의 표면색입니다."))
	FLinearColor SurfaceSoft;

	// [v1.0.0] Pause 배경에서 월드를 어둡게 하는 색상입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="화면 어둡게 (Dim Overlay)", ToolTip="Pause 또는 Modal 뒤의 월드 화면을 어둡게 하는 Overlay 색상입니다."))
	FLinearColor DimOverlay;

	// [v1.0.0] 내부 Grid와 보조 구분선 색상입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="보조 선 (Line Subtle)", ToolTip="내부 Grid와 중요도가 낮은 구분선에 사용하는 색상입니다."))
	FLinearColor LineSubtle;

	// [v1.0.0] 일반 Panel 외곽선 색상입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="기본 선 (Line Default)", ToolTip="일반 Panel과 Control의 기본 외곽선 색상입니다."))
	FLinearColor LineDefault;

	// [v1.0.0] 선택 전 핵심 경계 등에 사용하는 강한 선 색상입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="강조 선 (Line Strong)", ToolTip="핵심 경계와 높은 대비가 필요한 선에 사용하는 색상입니다."))
	FLinearColor LineStrong;

	// [v1.0.0] 핵심 값과 주요 문구의 텍스트 색상입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="주 텍스트 (Text Primary)", ToolTip="핵심 값과 주요 문구에 사용하는 기본 텍스트 색상입니다."))
	FLinearColor TextPrimary;

	// [v1.0.0] 레이블·단위의 보조 텍스트 색상입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="보조 텍스트 (Text Secondary)", ToolTip="레이블, 단위와 보조 정보에 사용하는 텍스트 색상입니다."))
	FLinearColor TextSecondary;

	// [v1.0.0] 중요도가 낮은 상태·설명 텍스트 색상입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="약한 텍스트 (Text Muted)", ToolTip="중요도가 낮은 상태와 설명에 사용하는 텍스트 색상입니다."))
	FLinearColor TextMuted;

	// [v1.0.0] 비활성·Unavailable 텍스트 색상입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="비활성 텍스트 (Text Disabled)", ToolTip="비활성 또는 Unavailable 상태에 사용하는 텍스트 색상입니다."))
	FLinearColor TextDisabled;

	// [v1.0.0] 밝은 Accent 면 위에 사용하는 텍스트 색상입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="강조면 텍스트 (Text On Accent)", ToolTip="밝은 Accent Fill 위에서 가독성을 확보하기 위한 텍스트 색상입니다."))
	FLinearColor TextOnAccent;

	// [v1.0.0] 선택·조준·시스템 기본 강조색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="전술 강조 (Accent Tactical)", ToolTip="선택, 조준과 시스템 기본 강조에 사용하는 Cyan 계열 색상입니다."))
	FLinearColor AccentTactical;

	// [v1.0.0] Hover 상태의 밝은 강조색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="호버 강조 (Accent Hover)", ToolTip="Hover 상태와 밝은 강조에 사용하는 색상입니다."))
	FLinearColor AccentHover;

	// [v1.0.0] Pressed 상태의 강조색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="눌림 강조 (Accent Pressed)", ToolTip="Pressed와 확정 입력 반응에 사용하는 강조색입니다."))
	FLinearColor AccentPressed;

	// [v1.0.0] 정보 공개와 일반 알림 상태색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="알림 상태 (State Notice)", ToolTip="정보 공개와 일반 알림에 사용하는 상태색입니다."))
	FLinearColor StateNotice;

	// [v1.0.0] 진행·불안정·주의 상태색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="주의 상태 (State Caution)", ToolTip="진행 중, 불안정 또는 주의가 필요한 상태에 사용하는 색상입니다."))
	FLinearColor StateCaution;

	// [v1.0.0] 손상·즉각 대응 위험 상태색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="위험 상태 (State Danger)", ToolTip="손상, 실패 위험과 즉각 대응이 필요한 상태에 사용하는 색상입니다."))
	FLinearColor StateDanger;

	// [v1.0.0] 치명·긴급 경고 상태색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="치명 상태 (State Critical)", ToolTip="치명 상태와 긴급 경고에만 사용하는 상태색입니다."))
	FLinearColor StateCritical;

	// [v1.0.0] 파괴·사용 불가 상태색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="비활성 상태 (State Disabled)", ToolTip="파괴, 비활성과 사용 불가 상태에 사용하는 색상입니다."))
	FLinearColor StateDisabled;

	// [v1.0.0] 명시적인 완료·복구 결과에만 사용하는 상태색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="성공 상태 (State Success)", ToolTip="일반 정상 상태가 아니라 명시적인 완료 또는 복구 결과에만 사용하는 색상입니다."))
	FLinearColor StateSuccess;

	// [v1.0.0] Shield 방어 계층 표시색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="쉴드 색상 (Shield Color)", ToolTip="Shield 에너지 계층의 연속 Fill과 Semantic Icon에 사용하는 색상입니다."))
	FLinearColor ShieldColor;

	// [v1.0.0] Armor 방어 계층 표시색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="장갑 색상 (Armor Color)", ToolTip="Armor 판재 계층과 방향별 Armor 표시에 사용하는 색상입니다."))
	FLinearColor ArmorColor;

	// [v1.0.0] Vehicle Integrity 방어 계층 표시색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="차량 내구도 색상 (Integrity Color)", ToolTip="Vehicle Integrity 생존 계층에 사용하는 색상입니다."))
	FLinearColor IntegrityColor;

	// [v1.0.0] Friendly 관계 표시색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="아군 관계색 (Friendly Color)", ToolTip="아군 Contact와 Target Marker에 사용하는 관계색입니다."))
	FLinearColor FriendlyColor;

	// [v1.0.0] Neutral 관계 표시색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="중립 관계색 (Neutral Color)", ToolTip="중립 Contact와 Target Marker에 사용하는 관계색입니다."))
	FLinearColor NeutralColor;

	// [v1.0.0] Hostile 관계 표시색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="적대 관계색 (Hostile Color)", ToolTip="적대 Contact와 Target Marker에 사용하는 관계색입니다."))
	FLinearColor HostileColor;

	// [v1.0.0] Unknown 관계·지식 상태 표시색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="미식별 색상 (Unknown Color)", ToolTip="미식별 Contact와 ??? 지식 상태에 사용하는 색상입니다."))
	FLinearColor UnknownColor;

	// [v1.0.0] Estimated 지식 상태 표시색입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Color", meta=(DisplayName="추정 정보 색상 (Estimated Color)", ToolTip="추정값과 불확실 정보에 사용하는 색상입니다."))
	FLinearColor EstimatedColor;
};

/**
 * Typography Family Role과 각 표시 Role의 기본값을 정의합니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUITypographyTokens
{
	GENERATED_BODY()

	// [v1.0.0] 승인된 Font Family와 Typography Role 기본값을 준비합니다.
	FCFUITypographyTokens();

	// [v1.0.0] 한글·일반 영문 UI에 사용하는 기본 Font Family 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="UI 폰트 패밀리 (UI Font Family)", ToolTip="실제 Font Asset 경로가 아니라 UI용 기본 Font Family Preset 이름입니다. D1-06 기본값은 Pretendard입니다."))
	FName UIFontFamily;

	// [v1.0.0] 속도·탄약·거리 등 고정폭 수치에 사용하는 Font Family 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="숫자 폰트 패밀리 (Numeric Font Family)", ToolTip="실제 Font Asset 경로가 아니라 숫자와 기술 수치용 Font Family Preset 이름입니다. D1-06 기본값은 IBM Plex Mono입니다."))
	FName NumericFontFamily;

	// [v1.0.0] 최상위 숫자 Typography Role입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="최상위 숫자 (Display XL)", ToolTip="속도 등 한 화면에서 가장 강한 숫자에 사용하는 Typography Role입니다."))
	FCFUITypographyStyle DisplayXL;

	// [v1.0.0] 대형 숫자 Typography Role입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="대형 숫자 (Display L)", ToolTip="주 자원과 핵심 방어값에 사용하는 대형 숫자 Typography Role입니다."))
	FCFUITypographyStyle DisplayL;

	// [v1.0.0] 중형 값 Typography Role입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="중형 값 (Value M)", ToolTip="거리, 주 자원과 일반 보조 수치에 사용하는 Typography Role입니다."))
	FCFUITypographyStyle ValueM;

	// [v1.0.0] 소형 값 Typography Role입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="소형 값 (Value S)", ToolTip="Armor 숫자와 작은 자원·보조 상태 값에 사용하는 Typography Role입니다."))
	FCFUITypographyStyle ValueS;

	// [v1.0.0] Pause·주요 Screen의 대형 제목 Role입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="대형 제목 (Heading L)", ToolTip="Pause와 주요 전체 화면의 제목에 사용하는 Typography Role입니다."))
	FCFUITypographyStyle HeadingL;

	// [v1.0.0] HUD Panel 제목 Role입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="중형 제목 (Heading M)", ToolTip="HUD Panel Header와 짧은 패널 제목에 사용하는 Typography Role입니다."))
	FCFUITypographyStyle HeadingM;

	// [v1.0.0] 메뉴와 상태 설명 본문 Role입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="본문 (Body)", ToolTip="메뉴 문구와 짧은 상태 설명에 사용하는 Typography Role입니다."))
	FCFUITypographyStyle Body;

	// [v1.0.0] HUD 레이블과 단위 Role입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="레이블 (Label)", ToolTip="HUD 레이블, 단위와 짧은 상태 이름에 사용하는 Typography Role입니다."))
	FCFUITypographyStyle Label;

	// [v1.0.0] 입력 힌트와 보조 설명 Role입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Typography", meta=(DisplayName="캡션 (Caption)", ToolTip="입력 힌트와 중요도가 낮은 보조 설명에 사용하는 Typography Role입니다."))
		FCFUITypographyStyle Caption;
};

/**
 * UI/Numeric Font Family가 사용할 Composite UFont와 Typeface 이름을 정의합니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIFontAssets
{
	GENERATED_BODY()

	// [v1.1.0] 실제 Font Asset이 없어도 안정적인 Typeface 이름 기본값을 준비합니다.
	FCFUIFontAssets();

	// [v1.1.0] 한글·일반 영문 UI가 사용할 Runtime Composite UFont입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style|Font", meta=(DisplayName="UI 폰트 자산 (UI Font Asset)", ToolTip="Pretendard 기반 Runtime Composite UFont입니다. DA_CFUIStyle이 Hard Reference하며 Widget은 경로를 직접 Load하지 않습니다."))
	TObjectPtr<UFont> UIFontAsset = nullptr;

	// [v1.1.0] 속도·탄약·거리 등 숫자가 사용할 Runtime Composite UFont입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style|Font", meta=(DisplayName="숫자 폰트 자산 (Numeric Font Asset)", ToolTip="IBM Plex Mono 기반 Runtime Composite UFont입니다. DA_CFUIStyle이 Hard Reference하며 Widget은 경로를 직접 Load하지 않습니다."))
	TObjectPtr<UFont> NumericFontAsset = nullptr;

	// [v1.1.0] Regular Weight가 Composite Font에서 찾을 Typeface 이름입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style|Font", meta=(DisplayName="Regular Typeface 이름 (Regular Typeface Name)", ToolTip="ECFUIFontWeight::Regular가 사용할 Composite Font Typeface 이름입니다."))
	FName RegularTypefaceName;

	// [v1.1.0] Medium Weight가 Composite Font에서 찾을 Typeface 이름입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style|Font", meta=(DisplayName="Medium Typeface 이름 (Medium Typeface Name)", ToolTip="ECFUIFontWeight::Medium이 사용할 Composite Font Typeface 이름입니다."))
	FName MediumTypefaceName;

	// [v1.1.0] SemiBold Weight가 Composite Font에서 찾을 Typeface 이름입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style|Font", meta=(DisplayName="SemiBold Typeface 이름 (SemiBold Typeface Name)", ToolTip="ECFUIFontWeight::SemiBold가 사용할 Composite Font Typeface 이름입니다."))
	FName SemiBoldTypefaceName;

	// [v1.1.0] Bold Weight가 Composite Font에서 찾을 Typeface 이름입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style|Font", meta=(DisplayName="Bold Typeface 이름 (Bold Typeface Name)", ToolTip="ECFUIFontWeight::Bold가 사용할 Composite Font Typeface 이름입니다."))
	FName BoldTypefaceName;
};

/**
 * 공통 간격과 안전 크기 Token입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUISpacingTokens
{
	GENERATED_BODY()

	FCFUISpacingTokens();

	// [v1.0.0] 가장 작은 간격 단위입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="간격 XS (Space XS)", ToolTip="가장 작은 UI 간격 단위입니다."))
	float SpaceXS = 4.0f;

	// [v1.0.0] 작은 간격 단위입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="간격 S (Space S)", ToolTip="작은 UI 간격 단위입니다."))
	float SpaceS = 8.0f;

	// [v1.0.0] 중간 간격 단위입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="간격 M (Space M)", ToolTip="중간 UI 간격 단위입니다."))
	float SpaceM = 12.0f;

	// [v1.0.0] 큰 간격 단위입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="간격 L (Space L)", ToolTip="큰 UI 간격 단위입니다."))
	float SpaceL = 16.0f;

	// [v1.0.0] XL 간격 단위입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="간격 XL (Space XL)", ToolTip="Section 또는 큰 내부 여백에 사용하는 간격 단위입니다."))
	float SpaceXL = 24.0f;

	// [v1.0.0] 2XL 간격 단위입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="간격 2XL (Space 2XL)", ToolTip="큰 UI 그룹 사이에 사용하는 간격 단위입니다."))
	float Space2XL = 32.0f;

	// [v1.0.0] 3XL 간격 단위입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="간격 3XL (Space 3XL)", ToolTip="대형 Screen Layout에서 사용하는 간격 단위입니다."))
	float Space3XL = 48.0f;

	// [v1.0.0] 4XL 간격 단위입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="간격 4XL (Space 4XL)", ToolTip="1440p 기본 Safe Margin과 같은 큰 간격에 사용하는 단위입니다."))
	float Space4XL = 64.0f;

	// [v1.0.0] 5XL 간격 단위입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="간격 5XL (Space 5XL)", ToolTip="매우 큰 Screen 구획에 사용하는 간격 단위입니다."))
	float Space5XL = 96.0f;

	// [v1.0.0] 1440p 중앙 HUD Canvas의 기본 외곽 Safe Margin입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="기본 안전 여백 (Safe Margin)", ToolTip="2560×1440 중앙 HUD Canvas에서 사용하는 기본 외곽 Safe Margin입니다."))
	float SafeMargin = 64.0f;

	// [v1.0.0] D1-07에서 승인된 1080p 전용 외곽 Design Unit Margin입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="1080p HUD 외곽 여백 (HUD Outer Margin 1080)", ToolTip="1920×1080 Phase 1 고정 HUD의 외곽 배치 Override입니다. 32 Design Unit은 0.75 Scale에서 실효 약 24px입니다."))
	float HUDOuterMargin1080 = 32.0f;

	// [v1.0.0] 일반 HUD Panel 내부 Padding입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="패널 여백 (Panel Padding)", ToolTip="일반 HUD Panel의 Standard 기본 내부 Padding입니다."))
	float PanelPadding = 24.0f;

	// [v1.0.0] Compact Panel 내부 Padding입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="컴팩트 패널 여백 (Compact Panel Padding)", ToolTip="Compact Density Panel의 기본 내부 Padding입니다."))
	float PanelPaddingCompact = 12.0f;

	// [v1.0.0] 일반 HUD Header 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="헤더 높이 (Header Height)", ToolTip="Standard Density HUD Panel Header의 기본 높이입니다."))
	float HeaderHeight = 48.0f;

	// [v1.0.0] 일반 정보 행 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="정보 행 높이 (Info Row Height)", ToolTip="Standard Density 일반 정보 행의 기본 높이입니다."))
	float InfoRowHeight = 36.0f;

	// [v1.0.0] 중요한 정보 행 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="대형 정보 행 높이 (Large Info Row Height)", ToolTip="Standard Density 중요 정보 행의 기본 높이입니다."))
	float InfoRowHeightLarge = 44.0f;

	// [v1.0.0] Density와 무관하게 유지할 최소 클릭·Focus 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="최소 입력 크기 (Minimum Hit Size)", ToolTip="접근성 안전 하한입니다. Density가 Compact여도 48보다 작게 낮추지 않습니다."))
	float MinimumHitSize = 48.0f;

	// [v1.0.0] 일반 메뉴 버튼 기본 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="메뉴 버튼 높이 (Menu Button Height)", ToolTip="일반 메뉴 버튼의 기본 높이입니다."))
	float MenuButtonHeight = 60.0f;

	// [v1.0.0] Pause Primary Button 기본 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="주 버튼 높이 (Primary Button Height)", ToolTip="Pause 등 Primary Button의 기본 높이입니다."))
	float PrimaryButtonHeight = 68.0f;

	// [v1.0.0] Pause Primary Button 기본 폭입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Spacing", meta=(DisplayName="주 버튼 폭 (Primary Button Width)", ToolTip="Pause 등 Primary Button의 기본 최소 폭입니다."))
	float PrimaryButtonWidth = 360.0f;
};

/**
 * Panel·Button의 형태와 외곽선 두께 Token입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIShapeTokens
{
	GENERATED_BODY()

	FCFUIShapeTokens();

	// [v1.0.0] HUD 소형 Panel과 Chip Radius입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Shape", meta=(DisplayName="작은 모서리 반경 (Radius Small)", ToolTip="HUD 소형 Panel과 Chip의 기본 Corner Radius입니다."))
	float RadiusSmall = 4.0f;

	// [v1.0.0] 일반 Panel과 Button Radius입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Shape", meta=(DisplayName="표준 모서리 반경 (Radius Standard)", ToolTip="일반 Panel과 Button의 기본 Corner Radius입니다."))
	float RadiusStandard = 6.0f;

	// [v1.0.0] Pause·Modal Radius입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Shape", meta=(DisplayName="큰 모서리 반경 (Radius Large)", ToolTip="Pause와 Modal의 기본 Corner Radius입니다."))
	float RadiusLarge = 8.0f;

	// [v1.0.0] 주요 전술 Panel의 선택적 Chamfer 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Shape", meta=(DisplayName="표준 챔퍼 (Chamfer Standard)", ToolTip="주요 전술 Panel 한두 모서리에 선택적으로 적용할 Chamfer 크기입니다."))
	float ChamferStandard = 12.0f;

	// [v1.0.0] 내부 Grid와 장식선의 최소 외곽선 두께입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Shape", meta=(DisplayName="헤어라인 외곽선 (Outline Hairline)", ToolTip="내부 Grid와 장식선의 최소 두께입니다. 화면 실효 1px 아래로 내리지 않습니다."))
	float OutlineHairline = 1.0f;

	// [v1.0.0] 일반 Panel 외곽선 두께입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Shape", meta=(DisplayName="표준 외곽선 (Outline Standard)", ToolTip="일반 Panel과 Button 외곽선의 기본 두께입니다."))
	float OutlineStandard = 2.0f;

	// [v1.0.0] Focus·Selected 상태 외곽선 두께입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Shape", meta=(DisplayName="집중 외곽선 (Outline Focused)", ToolTip="Focus와 Selected 상태에 사용하는 외곽선 두께입니다."))
	float OutlineFocused = 3.0f;

	// [v1.0.0] Critical 상태 외곽선 두께입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Shape", meta=(DisplayName="치명 외곽선 (Outline Critical)", ToolTip="Critical 상태에만 사용하는 강한 외곽선 두께입니다."))
	float OutlineCritical = 4.0f;
};

/**
 * UI 표면과 상태별 공통 Alpha Token입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIOpacityTokens
{
	GENERATED_BODY()

	FCFUIOpacityTokens();

	// [v1.0.0] 일반 HUD Panel Alpha입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Opacity", meta=(DisplayName="HUD 패널 불투명도 (HUD Panel Opacity)", ToolTip="일반 HUD Panel 표면에 사용하는 기본 Alpha입니다."))
	float HUDPanel = 0.80f;

	// [v1.0.0] 가벼운 HUD 배경 Alpha입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Opacity", meta=(DisplayName="HUD 소프트 불투명도 (HUD Soft Opacity)", ToolTip="Radar 내부 등 가벼운 HUD 배경에 사용하는 Alpha입니다."))
	float HUDSoft = 0.56f;

	// [v1.0.0] Pause·Modal Overlay 표면 Alpha입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Opacity", meta=(DisplayName="오버레이 불투명도 (Overlay Opacity)", ToolTip="Pause와 Modal의 강한 분리 표면에 사용하는 Alpha입니다."))
	float Overlay = 0.94f;

	// [v1.0.0] 화면 Dim Overlay Alpha입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Opacity", meta=(DisplayName="화면 어둡기 (Dim Opacity)", ToolTip="Pause 배경 등에서 월드를 어둡게 하는 Overlay Alpha입니다."))
	float Dim = 0.72f;

	// [v1.0.0] 비활성 요소 Alpha입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Opacity", meta=(DisplayName="비활성 불투명도 (Disabled Opacity)", ToolTip="비활성 상태의 보조 시각 요소에 사용하는 Alpha입니다."))
	float Disabled = 0.45f;

	// [v1.0.0] 비필수 장식 요소 Alpha입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Opacity", meta=(DisplayName="장식 불투명도 (Decorative Opacity)", ToolTip="세계관 장식과 비필수 시각 요소에 사용하는 최대 기본 Alpha입니다."))
	float Decorative = 0.60f;
};

/**
 * 하나의 UI Motion Preset을 정의합니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIMotionStyle
{
	GENERATED_BODY()

	FCFUIMotionStyle();

	// [v1.0.0] 애니메이션 지속 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Motion", meta=(DisplayName="지속 시간 초 (Duration Seconds)", ToolTip="해당 UI Motion의 기본 지속 시간입니다."))
	float DurationSeconds = 0.16f;

	// [v1.0.0] Motion 곡선의 논리적 Easing입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Motion", meta=(DisplayName="이징 (Easing)", ToolTip="Blueprint Animation 또는 C++ 보간에서 대응시킬 논리적 Easing 종류입니다."))
	ECFUIEasing Easing = ECFUIEasing::EaseInOutQuad;

	// [v1.0.0] 이 Motion이 반복을 허용하는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Motion", meta=(DisplayName="반복 허용 (Allow Loop)", ToolTip="일반 HUD Motion은 False이며 Critical Pulse만 True를 허용합니다."))
	bool bAllowLoop = false;
};

/**
 * 공통 UI 상태 변화에 사용하는 Motion Token 모음입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIMotionTokens
{
		GENERATED_BODY()

	// [v1.0.0] 승인된 UI Motion Duration·Easing·Loop 기본값을 준비합니다.
	FCFUIMotionTokens();

	// [v1.0.0] Hover·Focus 진입 Motion입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Motion", meta=(DisplayName="호버 모션 (Motion Hover)", ToolTip="Hover와 Focus 진입에 사용하는 기본 Motion입니다."))
	FCFUIMotionStyle Hover;

	// [v1.0.0] Button Pressed 반응 Motion입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Motion", meta=(DisplayName="눌림 모션 (Motion Pressed)", ToolTip="Button Pressed 반응에 사용하는 짧은 Motion입니다."))
	FCFUIMotionStyle Pressed;

	// [v1.0.0] 일반 상태 변경 Motion입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Motion", meta=(DisplayName="상태 모션 (Motion State)", ToolTip="작은 상태 변경에 사용하는 기본 Motion입니다."))
	FCFUIMotionStyle State;

	// [v1.0.0] Panel 등장·퇴장 Motion입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Motion", meta=(DisplayName="패널 모션 (Motion Panel)", ToolTip="Panel 등장과 퇴장에 사용하는 Motion입니다."))
	FCFUIMotionStyle Panel;

	// [v1.0.0] Gauge 값 보간 Motion입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Motion", meta=(DisplayName="게이지 모션 (Motion Gauge)", ToolTip="Status Bar와 Gauge 값 변화 보간에 사용하는 Motion입니다."))
	FCFUIMotionStyle Gauge;

	// [v1.0.0] Alert 1회 강조 Motion입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Motion", meta=(DisplayName="알림 모션 (Motion Alert)", ToolTip="Alert 진입 시 한 번 강조하는 Motion입니다."))
	FCFUIMotionStyle Alert;

	// [v1.0.0] Critical 상태에만 반복을 허용하는 Pulse Motion입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Motion", meta=(DisplayName="치명 펄스 모션 (Motion Critical Pulse)", ToolTip="Critical 상태에만 반복을 허용하는 약 1.4Hz 기본 Pulse Motion입니다."))
	FCFUIMotionStyle CriticalPulse;
};

/**
 * 공통 Panel Base가 사용할 의미 기반 Style입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIPanelStyle
{
	GENERATED_BODY()

	FCFUIPanelStyle();

	// [v1.0.0] Panel 배경에 사용할 의미 색상 Token입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Panel", meta=(DisplayName="배경 색상 Token (Background Color Token)", ToolTip="Panel 배경에 사용할 FCFUIColorTokens의 의미 색상 Token입니다."))
	ECFUIColorToken BackgroundColorToken = ECFUIColorToken::SurfaceBase;

	// [v1.0.0] Panel 외곽선에 사용할 의미 색상 Token입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Panel", meta=(DisplayName="외곽선 색상 Token (Outline Color Token)", ToolTip="Panel 외곽선에 사용할 의미 색상 Token입니다."))
	ECFUIColorToken OutlineColorToken = ECFUIColorToken::LineDefault;

	// [v1.0.0] Header 배경에 사용할 의미 색상 Token입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Panel", meta=(DisplayName="헤더 색상 Token (Header Color Token)", ToolTip="Panel Header 배경에 사용할 의미 색상 Token입니다."))
	ECFUIColorToken HeaderColorToken = ECFUIColorToken::SurfaceRaised;

	// [v1.0.0] Selected 상태에 사용할 의미 강조색 Token입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Panel", meta=(DisplayName="선택 강조 Token (Selected Accent Token)", ToolTip="Selected Panel의 Corner Marker와 일부 외곽선에 사용하는 의미 색상 Token입니다."))
	ECFUIColorToken SelectedAccentToken = ECFUIColorToken::AccentTactical;

	// [v1.0.0] Critical 상태에 사용할 의미 강조색 Token입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Panel", meta=(DisplayName="치명 강조 Token (Critical Accent Token)", ToolTip="Critical Panel의 강한 외곽선과 Pulse에 사용하는 의미 색상 Token입니다."))
	ECFUIColorToken CriticalAccentToken = ECFUIColorToken::StateCritical;
};

/**
 * Button 한 상태의 의미 기반 색상과 외곽선 값을 정의합니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIButtonStateStyle
{
	GENERATED_BODY()

	FCFUIButtonStateStyle();

	// [v1.0.0] Button 상태 배경에 사용할 의미 색상 Token입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Button", meta=(DisplayName="배경 색상 Token (Background Color Token)", ToolTip="Button 상태 배경에 사용할 의미 색상 Token입니다."))
	ECFUIColorToken BackgroundColorToken = ECFUIColorToken::SurfaceRaised;

	// [v1.0.0] Button 상태 외곽선에 사용할 의미 색상 Token입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Button", meta=(DisplayName="외곽선 색상 Token (Outline Color Token)", ToolTip="Button 상태 외곽선에 사용할 의미 색상 Token입니다."))
	ECFUIColorToken OutlineColorToken = ECFUIColorToken::LineDefault;

	// [v1.0.0] Button 상태 텍스트에 사용할 의미 색상 Token입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Button", meta=(DisplayName="텍스트 색상 Token (Text Color Token)", ToolTip="Button 상태 텍스트에 사용할 의미 색상 Token입니다."))
	ECFUIColorToken TextColorToken = ECFUIColorToken::TextPrimary;

	// [v1.0.0] Button 상태 외곽선의 Design Unit 두께입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Button", meta=(DisplayName="외곽선 두께 (Outline Thickness)", ToolTip="Button 상태 외곽선의 Design Unit 두께입니다."))
	float OutlineThickness = 2.0f;
};

/**
 * 공통 Button Base의 상태별 Style입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIButtonStyle
{
		GENERATED_BODY()

	// [v1.0.0] Normal·Hover·Focused·Pressed·Disabled Button 기본값을 준비합니다.
	FCFUIButtonStyle();

	// [v1.0.0] Normal 상태 Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Button", meta=(DisplayName="일반 상태 (Normal)", ToolTip="일반 Button 상태의 기본 Style입니다."))
	FCFUIButtonStateStyle Normal;

	// [v1.0.0] Hover 상태 Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Button", meta=(DisplayName="호버 상태 (Hover)", ToolTip="마우스 Hover 상태의 Style입니다."))
	FCFUIButtonStateStyle Hover;

	// [v1.0.0] Focused 상태 Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Button", meta=(DisplayName="포커스 상태 (Focused)", ToolTip="키보드·게임패드 Focus 상태의 Style입니다. Hover와 별도 상태로 유지합니다."))
	FCFUIButtonStateStyle Focused;

	// [v1.0.0] Pressed 상태 Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Button", meta=(DisplayName="눌림 상태 (Pressed)", ToolTip="Button 입력이 눌린 동안 사용하는 Style입니다."))
	FCFUIButtonStateStyle Pressed;

	// [v1.0.0] Disabled 상태 Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Button", meta=(DisplayName="비활성 상태 (Disabled)", ToolTip="사용 불가 Button 상태의 Style입니다."))
	FCFUIButtonStateStyle Disabled;

	// [v1.0.0] Pressed 상태에서 적용할 기본 Render Scale입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Button", meta=(DisplayName="눌림 스케일 (Pressed Scale)", ToolTip="Pressed 상태에서 짧게 적용할 기본 Render Scale입니다."))
	float PressedScale = 0.98f;

	// [v1.0.0] Primary Button의 Accent Fill Alpha입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Button", meta=(DisplayName="주 버튼 강조 Alpha (Primary Accent Alpha)", ToolTip="Primary Button에서 AccentTactical Fill에 적용할 기본 Alpha입니다."))
	float PrimaryAccentAlpha = 0.18f;
};

/**
 * Status Bar 한 Variant의 Density별 높이와 Fill 의미를 정의합니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIStatusBarVariantStyle
{
	GENERATED_BODY()

	FCFUIStatusBarVariantStyle();

	// [v1.0.0] Compact Density에서의 Bar 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|StatusBar", meta=(DisplayName="컴팩트 높이 (Compact Height)", ToolTip="Compact Density에서 사용하는 Status Bar 높이입니다."))
	float CompactHeight = 8.0f;

	// [v1.0.0] Standard Density에서의 Bar 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|StatusBar", meta=(DisplayName="표준 높이 (Standard Height)", ToolTip="Standard Density에서 사용하는 Status Bar 높이입니다."))
	float StandardHeight = 10.0f;

	// [v1.0.0] Expanded Density에서의 Bar 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|StatusBar", meta=(DisplayName="확장 높이 (Expanded Height)", ToolTip="Expanded Density에서 사용하는 Status Bar 높이입니다."))
	float ExpandedHeight = 12.0f;

	// [v1.0.0] 기본 Fill에 사용할 의미 색상 Token입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|StatusBar", meta=(DisplayName="채움 색상 Token (Fill Color Token)", ToolTip="Status Bar Fill에 사용할 기본 의미 색상 Token입니다."))
	ECFUIColorToken FillColorToken = ECFUIColorToken::AccentTactical;

	// [v1.0.0] 현재 View State가 제공하는 상태색으로 Fill을 Override할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|StatusBar", meta=(DisplayName="상태색 사용 (Use State Color)", ToolTip="일반 Resource처럼 Presenter 상태색을 우선 사용할 Variant이면 True입니다."))
	bool bUseStateColor = false;

	// [v1.0.0] 시각적 분절을 사용할 때의 Segment 개수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|StatusBar", meta=(DisplayName="시각 분절 수 (Visual Segment Count)", ToolTip="0이면 연속 Fill입니다. Armor 기본 10개 분절은 시각 재질 표현이며 6방향 장갑 의미가 아닙니다."))
	int32 VisualSegmentCount = 0;
};

/**
 * Shield·Armor·Integrity·Resource·Progress Bar의 공통 Style입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIStatusBarStyle
{
		GENERATED_BODY()

	// [v1.0.0] Shield·Armor·Integrity·Resource·Progress Bar 기본값을 준비합니다.
	FCFUIStatusBarStyle();

	// [v1.0.0] Shield Bar Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|StatusBar", meta=(DisplayName="쉴드 바 (Shield Bar)", ToolTip="Shield 에너지 계층의 연속 Fill Style입니다."))
	FCFUIStatusBarVariantStyle Shield;

	// [v1.0.0] Armor Bar Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|StatusBar", meta=(DisplayName="장갑 바 (Armor Bar)", ToolTip="Armor 판재 계층의 시각적 분절 Fill Style입니다."))
	FCFUIStatusBarVariantStyle Armor;

	// [v1.0.0] Vehicle Integrity Bar Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|StatusBar", meta=(DisplayName="차량 내구도 바 (Integrity Bar)", ToolTip="Vehicle Integrity 생존 계층의 굵은 연속 Fill Style입니다."))
	FCFUIStatusBarVariantStyle Integrity;

	// [v1.0.0] 일반 무기·차량 Resource Bar Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|StatusBar", meta=(DisplayName="자원 바 (Resource Bar)", ToolTip="Ammo, Charge, Heat 등 상태별 의미색을 사용할 수 있는 일반 Resource Style입니다."))
	FCFUIStatusBarVariantStyle Resource;

	// [v1.0.0] Lock·Scan·Reload 진행 Bar Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|StatusBar", meta=(DisplayName="진행 바 (Progress Bar)", ToolTip="Lock, Scan과 Reload 진행률에 사용하는 얇은 Accent Bar Style입니다."))
	FCFUIStatusBarVariantStyle Progress;
};

/**
 * 공통 Info Row의 Density별 Geometry와 지식 상태 문구입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIInfoRowStyle
{
		GENERATED_BODY()

	// [v1.0.0] Info Row Geometry와 지식 상태 문구 기본값을 준비합니다.
	FCFUIInfoRowStyle();

	// [v1.0.0] Compact Density 레이블 폭입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|InfoRow", meta=(DisplayName="컴팩트 레이블 폭 (Compact Label Width)", ToolTip="Compact Density Info Row의 기본 Label 영역 폭입니다."))
	float CompactLabelWidth = 96.0f;

	// [v1.0.0] Standard Density 레이블 폭입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|InfoRow", meta=(DisplayName="표준 레이블 폭 (Standard Label Width)", ToolTip="Standard Density Info Row의 기본 Label 영역 폭입니다."))
	float StandardLabelWidth = 120.0f;

	// [v1.0.0] Expanded Density 레이블 폭입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|InfoRow", meta=(DisplayName="확장 레이블 폭 (Expanded Label Width)", ToolTip="Expanded Density Info Row의 기본 Label 영역 폭입니다."))
	float ExpandedLabelWidth = 144.0f;

	// [v1.0.0] Compact Density 상태 아이콘 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|InfoRow", meta=(DisplayName="컴팩트 아이콘 크기 (Compact Icon Size)", ToolTip="Compact Density Info Row의 상태 아이콘 기본 크기입니다."))
	float CompactIconSize = 20.0f;

	// [v1.0.0] Standard Density 상태 아이콘 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|InfoRow", meta=(DisplayName="표준 아이콘 크기 (Standard Icon Size)", ToolTip="Standard Density Info Row의 상태 아이콘 기본 크기입니다."))
	float StandardIconSize = 24.0f;

	// [v1.0.0] Expanded Density 상태 아이콘 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|InfoRow", meta=(DisplayName="확장 아이콘 크기 (Expanded Icon Size)", ToolTip="Expanded Density Info Row의 상태 아이콘 기본 크기입니다."))
	float ExpandedIconSize = 32.0f;

	// [v1.0.0] Unknown 지식 상태에 사용할 기본 표시 문구입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|InfoRow", meta=(DisplayName="미식별 문구 (Unknown Text)", ToolTip="정보 자체는 존재하지만 플레이어에게 아직 공개되지 않은 Unknown 상태에 표시할 문구입니다."))
	FText UnknownText;

	// [v1.0.0] Unavailable 상태에 사용할 기본 표시 문구입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|InfoRow", meta=(DisplayName="사용 불가 문구 (Unavailable Text)", ToolTip="Provider 또는 기능이 해당 정보를 제공하지 못하는 Unavailable 상태에 표시할 문구입니다."))
	FText UnavailableText;
};

/**
 * Alert Severity 한 단계의 Geometry·Accent·기본 지속시간을 정의합니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIAlertSeverityStyle
{
	GENERATED_BODY()

	FCFUIAlertSeverityStyle();

	// [v1.0.0] Alert Item의 기본 높이입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Alert", meta=(DisplayName="알림 높이 (Alert Height)", ToolTip="해당 Severity Alert Item의 기본 높이입니다."))
	float Height = 48.0f;

	// [v1.0.0] Alert 강조에 사용할 의미 색상 Token입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Alert", meta=(DisplayName="강조 색상 Token (Accent Color Token)", ToolTip="해당 Severity의 Accent Line과 상태 강조에 사용할 의미 색상 Token입니다."))
	ECFUIColorToken AccentColorToken = ECFUIColorToken::StateNotice;

	// [v1.0.0] 자동 제거형 Alert의 기본 표시 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Alert", meta=(DisplayName="기본 지속 시간 (Default Duration)", ToolTip="0이면 상태 해제 또는 명시 시간까지 유지하는 Persistent 기본 동작으로 해석할 수 있습니다."))
	float DefaultDurationSeconds = 2.0f;

	// [v1.0.0] 명시 시간이 없을 때 기본적으로 Persistent인지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Alert", meta=(DisplayName="기본 지속 상태 (Persistent By Default)", ToolTip="Critical처럼 명시 시간이 없으면 상태 해제까지 유지해야 하는 Severity에서 True입니다."))
	bool bPersistentByDefault = false;
};

/**
 * Notice·Warning·Critical Alert의 공통 표시 Style입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIAlertStyle
{
		GENERATED_BODY()

	// [v1.0.0] Notice·Warning·Critical Alert 기본값을 준비합니다.
	FCFUIAlertStyle();

	// [v1.0.0] Notice Alert Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Alert", meta=(DisplayName="일반 알림 (Notice)", ToolTip="일반 정보성 Alert의 기본 Style입니다."))
	FCFUIAlertSeverityStyle Notice;

	// [v1.0.0] Warning Alert Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Alert", meta=(DisplayName="주의 알림 (Warning)", ToolTip="주의가 필요한 Alert의 기본 Style입니다."))
	FCFUIAlertSeverityStyle Warning;

	// [v1.0.0] Critical Alert Style입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Alert", meta=(DisplayName="치명 알림 (Critical)", ToolTip="치명 상태 Alert의 기본 Style입니다. 반복 Pulse는 Critical에서만 허용합니다."))
	FCFUIAlertSeverityStyle Critical;

	// [v1.0.0] Alert Feed에 동시에 표시할 기본 최대 개수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Alert", meta=(DisplayName="최대 동시 알림 수 (Maximum Visible Alerts)", ToolTip="Alert Feed에서 동시에 표시할 기본 최대 Alert 개수입니다."))
	int32 MaximumVisibleAlerts = 3;
};

/**
 * Semantic Icon ID와 실제 Unreal Asset의 느슨한 연결 한 건입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUISemanticIconEntry
{
	GENERATED_BODY()

	FCFUISemanticIconEntry();

	// [v1.0.0] Gameplay와 Widget이 공유할 안정적인 의미 Icon ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Icon", meta=(DisplayName="의미 아이콘 ID (Semantic Icon ID)", ToolTip="Vehicle, Shield, Armor, Weapon, Target처럼 실제 Texture 경로와 분리된 안정적인 의미 ID입니다."))
	FName SemanticId;

	// [v1.0.0] Texture·Material·Vector 등 실제 표시 자산을 Soft Reference로 연결합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Icon", meta=(DisplayName="아이콘 자산 (Icon Asset)", ToolTip="Semantic ID에 대응하는 실제 Unreal Asset Soft Reference입니다. D1-09 이후 채우며 C++에 경로를 하드코딩하지 않습니다."))
	TSoftObjectPtr<UObject> IconAsset;
};

/**
 * Solid Core + Tactical Cut 기본 규격과 Semantic Icon 매핑을 소유합니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFUIIconSet
{
		GENERATED_BODY()

	// [v1.0.0] 승인된 Solid Core + Tactical Cut Icon Preset 기본값을 준비합니다.
	FCFUIIconSet();

	// [v1.0.0] 현재 Icon Set의 사람이 읽을 수 있는 Preset 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Icon", meta=(DisplayName="아이콘 스타일 이름 (Icon Style Name)", ToolTip="현재 Icon Set Preset의 이름입니다. D1-06 기본값은 Solid Core + Tactical Cut입니다."))
	FName StyleName;

	// [v1.0.0] Semantic Icon 제작 기준 Grid 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Icon", meta=(DisplayName="기본 아이콘 그리드 (Base Icon Grid)", ToolTip="Semantic Icon을 제작·정렬할 때 사용하는 정사각 Base Grid 크기입니다."))
	float BaseGridSize = 24.0f;

	// [v1.0.0] Caption·Tile 보조 아이콘 기본 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Icon", meta=(DisplayName="작은 아이콘 크기 (Icon Small)", ToolTip="Caption과 Compact Tile 보조 아이콘의 기본 크기입니다."))
	float IconSmall = 16.0f;

	// [v1.0.0] 일반 상태 아이콘 기본 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Icon", meta=(DisplayName="중간 아이콘 크기 (Icon Medium)", ToolTip="일반 상태와 Info Row 아이콘의 기본 크기입니다."))
	float IconMedium = 20.0f;

	// [v1.0.0] Panel 핵심 아이콘 기본 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Icon", meta=(DisplayName="큰 아이콘 크기 (Icon Large)", ToolTip="Panel 핵심 상태를 나타내는 큰 Semantic Icon 기본 크기입니다."))
	float IconLarge = 28.0f;

	// [v1.0.0] 비선택 Weapon Compact Tile 아이콘 기본 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Icon", meta=(DisplayName="컴팩트 무기 아이콘 (Weapon Compact Icon)", ToolTip="비선택 Weapon Compact Tile에 사용하는 기본 아이콘 크기입니다."))
	float WeaponCompact = 18.0f;

	// [v1.0.0] 선택 Weapon 아이콘 기본 크기입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Icon", meta=(DisplayName="선택 무기 아이콘 (Weapon Selected Icon)", ToolTip="현재 선택된 Weapon에 사용하는 기본 아이콘 크기입니다."))
	float WeaponSelected = 20.0f;

	// [v1.0.0] Semantic ID와 실제 Icon Asset의 교체 가능한 매핑 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|UI|Style|Icon", meta=(DisplayName="의미 아이콘 목록 (Semantic Icons)", ToolTip="실제 Texture·Material·Vector 경로를 C++에 하드코딩하지 않고 Semantic ID로 연결하는 목록입니다."))
	TArray<FCFUISemanticIconEntry> SemanticIcons;
};

/**
 * CarFight 전체 UI가 공유할 Style Token DataAsset 타입입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFUIStyleData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] Color Token 모음입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="색상 Token (Color Tokens)", ToolTip="CarFight 전체 UI가 공유하는 의미 기반 색상 Token입니다."))
	FCFUIColorTokens Colors;

		// [v1.0.0] Typography Token 모음입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="타이포그래피 Token (Typography Tokens)", ToolTip="Font Family Role과 Typography Role 기본값입니다."))
	FCFUITypographyTokens Typography;

	// [v1.1.0] 실제 Composite UFont와 Weight별 Typeface 이름입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="폰트 자산 (Font Assets)", ToolTip="UI와 Numeric Runtime Composite UFont 및 Weight별 Typeface 이름을 소유합니다. D1-09B에서 실제 Font Asset을 연결합니다."))
	FCFUIFontAssets FontAssets;

	// [v1.0.0] Spacing·안전 크기 Token 모음입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="간격 Token (Spacing Tokens)", ToolTip="Spacing Grid, Safe Margin과 최소 입력 크기 기본값입니다."))
	FCFUISpacingTokens Spacing;

	// [v1.0.0] Shape·Outline Token 모음입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="형태 Token (Shape Tokens)", ToolTip="Corner Radius, Chamfer와 Outline 기본값입니다."))
	FCFUIShapeTokens Shape;

	// [v1.0.0] Opacity Token 모음입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="불투명도 Token (Opacity Tokens)", ToolTip="HUD Panel, Overlay, Disabled와 Decorative Alpha 기본값입니다."))
	FCFUIOpacityTokens Opacity;

	// [v1.0.0] Motion Token 모음입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="모션 Token (Motion Tokens)", ToolTip="Hover, Pressed, State, Panel, Gauge, Alert와 Critical Pulse Motion 기본값입니다."))
	FCFUIMotionTokens Motion;

	// [v1.0.0] Panel Base 의미 Style입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="패널 Style (Panel Style)", ToolTip="공통 Panel Base가 사용할 배경·외곽선·Header·선택·Critical 의미 색상 참조입니다."))
	FCFUIPanelStyle PanelStyle;

	// [v1.0.0] Button Base 상태별 Style입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="버튼 Style (Button Style)", ToolTip="Normal, Hover, Focused, Pressed와 Disabled Button 상태 Style입니다."))
	FCFUIButtonStyle ButtonStyle;

	// [v1.0.0] 공통 Status Bar Style입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="상태 바 Style (Status Bar Style)", ToolTip="Shield, Armor, Integrity, Resource와 Progress Bar의 Density별 기본 Style입니다."))
	FCFUIStatusBarStyle StatusBarStyle;

	// [v1.0.0] 공통 Info Row Style입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="정보 행 Style (Info Row Style)", ToolTip="Target, Weapon과 Vehicle 정보 행의 Density별 Geometry와 Unknown·Unavailable 문구입니다."))
	FCFUIInfoRowStyle InfoRowStyle;

	// [v1.0.0] Alert Feed Item 공통 Style입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="알림 Style (Alert Style)", ToolTip="Notice, Warning과 Critical Alert의 기본 Geometry·색상·지속시간입니다."))
	FCFUIAlertStyle AlertStyle;

	// [v1.0.0] Semantic Icon 기본 규격과 실제 자산 매핑입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|UI|Style", meta=(DisplayName="아이콘 세트 (Icon Set)", ToolTip="Solid Core + Tactical Cut 기본 규격과 Semantic ID 기반 실제 Icon Asset 매핑입니다."))
	FCFUIIconSet IconSet;

	// [v1.0.0] 의미 기반 Color Token을 현재 Style Data의 실제 색으로 해석합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Style", meta=(DisplayName="UI 색상 Token 해석 (Resolve UI Color Token)", ToolTip="의미 기반 Color Token을 현재 Style Data의 FLinearColor 값으로 반환합니다."))
	FLinearColor ResolveColor(ECFUIColorToken ColorToken) const;

		// [v1.0.0] Typography Role을 현재 Style Data의 크기·행높이·Weight로 해석합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Style", meta=(DisplayName="UI 타이포그래피 Role 해석 (Resolve UI Typography Role)", ToolTip="Typography Role을 현재 Style Data의 FCFUITypographyStyle 값으로 반환합니다."))
	FCFUITypographyStyle ResolveTypography(ECFUITypographyRole TypographyRole) const;

	// [v1.1.0] UI 또는 Numeric Family Role에 연결된 Composite UFont를 반환합니다.
	UFont* ResolveFontAsset(ECFUIFontFamilyRole FontFamilyRole) const;

	// [v1.1.0] 논리적 Font Weight를 Composite Font의 Typeface 이름으로 해석합니다.
	FName ResolveTypefaceName(ECFUIFontWeight FontWeight) const;

		// [v1.1.0] Font Family·Typography Role·Scale·안전 하한을 실제 Slate Font 정보로 해석합니다.
	FSlateFontInfo ResolveSlateFontInfo(ECFUIFontFamilyRole FontFamilyRole, ECFUITypographyRole TypographyRole, float TypographyScale = 1.0f, int32 MinimumEffectiveFontSize = 1) const;

	// [v1.2.0] 실제 Texture/Material 경로 대신 Semantic ID로 등록된 Soft Icon Asset을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|UI|Style", meta=(DisplayName="의미 아이콘 해석 (Resolve Semantic Icon)", ToolTip="Vehicle, Armor, Target 같은 Semantic ID에 연결된 Soft Icon Asset을 반환합니다. 이 함수는 Asset을 직접 Load하지 않습니다."))
	TSoftObjectPtr<UObject> ResolveSemanticIconAsset(FName SemanticId) const;

	// [v1.0.0] 현재 Style Data가 최소 안전 하한과 승인 기본 계약을 위반하지 않는지 검증합니다.
	bool ValidateStyleData(FString& OutFailureReason) const;
};
