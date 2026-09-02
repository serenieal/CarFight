// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-08-31
// Description: CarFight 타겟 선택 시스템 공용 타입·TS-P0-08 Registry 검색 진단·LOS 사전필터 정의
// Scope: 선택 상태, 관계, 분류, 정보 단계, 추적 상태, 선택 컨텍스트, 후보 평가값, 검색 View와 결과 계약을 제공합니다.
// Changelog:
// - v1.4.0: 후보 공급을 Target Registry로 전환하고 RuntimeRegistryTargetableCount 진단을 추가. Legacy RuntimeWorldActorScanCount는 반복 월드 스캔 제거 증거로 0을 유지.
// - v1.3.0: 비-Direct Targetable Actor가 proximity 거리·반각 밖이면 LOS Trace를 생략한 수를 RuntimeVisibilityPrefilterSkipCount로 진단.
// - v1.2.0: TS-P0-08 런타임 후보 검색의 월드 Actor 스캔 수, Visibility/전체 Trace 수와 경과 ms를 읽기 전용 진단값으로 추가.
// - v1.1.0: TS-P0-03 후보 평가용 SearchView와 결정적 정렬 결과 구조를 추가.
// - v1.0.0: TS-P0-01용 최소 타겟 선택 enum, 표시 정보, 선택 컨텍스트, 후보 데이터와 설정 구조체를 추가.
// Migration:
// - v1.4.0 런타임 후보 갱신은 Target Registry snapshot을 사용합니다. RuntimeWorldActorScanCount는 호환 필드로 유지하지만 반복 월드 순회를 수행하지 않으므로 0입니다.
// - v1.3.0 LOS 사전필터는 기존 EvaluateCandidateActors가 반드시 탈락시키는 비-Direct 거리·반각 밖 대상의 Visibility Trace만 생략한다. OutCandidateActors와 정렬·히스테리시스 입력은 유지한다.
// - v1.2.0 진단값은 RefreshCurrentCandidate 런타임 검색에서만 채워지며 순수 EvaluateCandidateActors 호출에서는 0을 유지한다. 후보 판정·정렬·튜닝에는 사용하지 않는다.
// - 기존 Aim, Weapon, Health 계약은 변경하지 않는다.
// - 후보 검색, 정렬, Trace와 장비 락온 상태는 후속 Task에서 별도 구현한다.

#pragma once

#include "CoreMinimal.h"
#include "CFTargetSelectTypes.generated.h"

class AActor;

/**
 * 타겟 선택 시스템이 외부에 공개하는 핵심 상태입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="타겟 핵심 상태 (Target Core State)", ToolTip="후보와 선택 대상의 존재 및 유효성에 따라 계산되는 타겟 선택 시스템의 핵심 상태입니다."))
enum class ECFTargetCoreState : uint8
{
	// [v1.0.0] 유효한 후보와 선택 대상이 모두 없는 상태입니다.
	NoCandidate UMETA(DisplayName="후보 없음 (No Candidate)"),

	// [v1.0.0] 선택 대상은 없고 유효한 후보만 있는 상태입니다.
	CandidateAvailable UMETA(DisplayName="후보 있음 (Candidate Available)"),

	// [v1.0.0] 유효한 선택 대상이 있는 상태입니다.
	TargetSelected UMETA(DisplayName="타겟 선택됨 (Target Selected)"),

	// [v1.0.0] 선택 기록은 남아 있지만 선택 대상이 무효화된 상태입니다.
	SelectedTargetInvalid UMETA(DisplayName="선택 타겟 무효 (Selected Target Invalid)")
};

/**
 * 선택 대상과 플레이어 사이의 관계 분류입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="타겟 관계 (Target Relation)", ToolTip="선택 대상이 아군, 중립, 적대 또는 미확인인지 나타내는 최소 관계 계약입니다."))
enum class ECFTargetRelation : uint8
{
	// [v1.0.0] 관계를 아직 판정하지 못한 상태입니다.
	Unknown UMETA(DisplayName="미확인 (Unknown)"),

	// [v1.0.0] 플레이어와 우호적인 대상입니다.
	Friendly UMETA(DisplayName="아군 (Friendly)"),

	// [v1.0.0] 현재 우호 또는 적대로 판정되지 않은 대상입니다.
	Neutral UMETA(DisplayName="중립 (Neutral)"),

	// [v1.0.0] 플레이어와 적대적인 대상입니다.
	Hostile UMETA(DisplayName="적대 (Hostile)")
};

/**
 * 선택 대상의 큰 기능 분류입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="타겟 분류 (Target Category)", ToolTip="차량, 장치, 환경 오브젝트처럼 선택 대상의 큰 기능 분류를 나타냅니다."))
enum class ECFTargetCategory : uint8
{
	// [v1.0.0] 분류를 아직 알 수 없는 대상입니다.
	Unknown UMETA(DisplayName="미분류 (Unknown)"),

	// [v1.0.0] 전투 또는 이동 차량 대상입니다.
	Vehicle UMETA(DisplayName="차량 (Vehicle)"),

	// [v1.0.0] 상호작용 가능한 장치 또는 설비 대상입니다.
	Device UMETA(DisplayName="장치 (Device)"),

	// [v1.0.0] 환경 오브젝트 또는 구조물 대상입니다.
	Environment UMETA(DisplayName="환경 (Environment)")
};

/**
 * 센서 또는 스캔이 공개할 수 있는 정보 상세 단계입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="타겟 정보 단계 (Target Information Level)", ToolTip="선택 대상에 대해 현재 공개 가능한 정보의 상세 단계를 나타냅니다."))
enum class ECFTargetInfoLevel : uint8
{
	// [v1.0.0] 대상 정보를 표시할 수 없는 상태입니다.
	None UMETA(DisplayName="정보 없음 (None)"),

	// [v1.0.0] 대상 존재만 감지된 상태입니다.
	Detected UMETA(DisplayName="감지됨 (Detected)"),

	// [v1.0.0] 대상의 기본 정체가 식별된 상태입니다.
	Identified UMETA(DisplayName="식별됨 (Identified)"),

	// [v1.0.0] 상세 스캔 정보까지 공개 가능한 상태입니다.
	DetailedScan UMETA(DisplayName="상세 스캔 (Detailed Scan)")
};

/**
 * 선택 대상의 현재 추적 품질 상태입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="타겟 추적 상태 (Target Track State)", ToolTip="선택 대상이 시야에 보이는지, 가려졌는지, 추정 위치로 추적되는지 나타냅니다."))
enum class ECFTargetTrackState : uint8
{
	// [v1.0.0] 추적 상태가 유효하지 않습니다.
	Invalid UMETA(DisplayName="무효 (Invalid)"),

	// [v1.0.0] 대상이 현재 직접 관측 가능한 상태입니다.
	Visible UMETA(DisplayName="가시 (Visible)"),

	// [v1.0.0] 대상이 장애물 등에 가려진 상태입니다.
	Occluded UMETA(DisplayName="가림 (Occluded)"),

	// [v1.0.0] 마지막 관측값을 기반으로 위치를 추정하는 상태입니다.
	Estimated UMETA(DisplayName="추정 추적 (Estimated)"),

	// [v1.0.0] 대상 추적 신호를 잃은 상태입니다.
	SignalLost UMETA(DisplayName="신호 손실 (Signal Lost)")
};

/**
 * 선택 대상을 명시적으로 해제한 사유입니다.
 */
UENUM(BlueprintType, meta=(DisplayName="타겟 해제 사유 (Target Clear Reason)", ToolTip="선택 대상을 해제한 시스템 또는 게임플레이 사유를 나타냅니다."))
enum class ECFTargetClearReason : uint8
{
	// [v1.0.0] 사용자 또는 호출자가 명시적으로 해제했습니다.
	Manual UMETA(DisplayName="수동 해제 (Manual)"),

	// [v1.0.0] 대상이 선택 계약을 더 이상 만족하지 않습니다.
	InvalidTarget UMETA(DisplayName="무효 대상 (Invalid Target)"),

	// [v1.0.0] 대상이 파괴됐습니다.
	Destroyed UMETA(DisplayName="파괴됨 (Destroyed)"),

	// [v1.0.0] 대상이 추적 가능 거리 밖으로 벗어났습니다.
	OutOfTrackingRange UMETA(DisplayName="추적 거리 이탈 (Out Of Tracking Range)"),

	// [v1.0.0] 타겟 선택 시스템이 비활성화됐습니다.
	SystemDisabled UMETA(DisplayName="시스템 비활성 (System Disabled)"),

	// [v1.0.0] 타겟 선택 컴포넌트의 소유자가 제거됐습니다.
	OwnerDestroyed UMETA(DisplayName="소유자 제거 (Owner Destroyed)")
};

/**
 * UI와 장비 시스템이 공통으로 읽을 수 있는 선택 대상 표시 정보입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="타겟 표시 정보 (Target Display Info)", ToolTip="타겟 ID, 표시 이름, 분류, 관계, 정보 단계와 속성 태그를 전달합니다."))
struct FCFTargetDisplayInfo
{
	GENERATED_BODY()

	// [v1.0.0] 저장, 로그와 비교에 사용할 안정적인 대상 식별자입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Display", meta=(DisplayName="타겟 ID (TargetId)", ToolTip="저장, 전투 로그, UI 갱신과 안정적인 비교에 사용할 대상 식별자입니다."))
	FName TargetId = NAME_None;

	// [v1.0.0] HUD와 정보 패널에 표시할 대상 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Display", meta=(DisplayName="표시 이름 (DisplayName)", ToolTip="HUD 타겟 마커와 정보 패널에 표시할 대상 이름입니다."))
	FText DisplayName;

	// [v1.0.0] 대상의 큰 기능 분류입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Display", meta=(DisplayName="타겟 분류 (TargetCategory)", ToolTip="대상이 차량, 장치, 환경 또는 미분류인지 나타냅니다."))
	ECFTargetCategory TargetCategory = ECFTargetCategory::Unknown;

	// [v1.0.0] 플레이어와 대상 사이의 관계입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Display", meta=(DisplayName="타겟 관계 (Relation)", ToolTip="대상이 미확인, 아군, 중립 또는 적대인지 나타냅니다."))
	ECFTargetRelation Relation = ECFTargetRelation::Unknown;

	// [v1.0.0] 현재 공개 가능한 대상 정보 단계입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Display", meta=(DisplayName="정보 단계 (InformationLevel)", ToolTip="대상에 대해 현재 감지, 식별 또는 상세 스캔 정보까지 공개 가능한지 나타냅니다."))
	ECFTargetInfoLevel InformationLevel = ECFTargetInfoLevel::Detected;

	// [v1.0.0] 장비와 선택 컨텍스트가 필터링에 사용할 가벼운 속성 태그 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Display", meta=(DisplayName="속성 태그 (AttributeTags)", ToolTip="GameplayTags 의존성 없이 선택 컨텍스트와 장비가 필터링에 사용할 FName 속성 태그 목록입니다."))
	TArray<FName> AttributeTags;
};

/**
 * 타겟 선택 요청이 허용할 대상 범위를 정의하는 컨텍스트입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="타겟 선택 컨텍스트 (Target Selection Context)", ToolTip="자기 선택, 시야 요구, 분류, 관계와 속성 태그 필터를 정의합니다."))
struct FCFTargetSelectionContext
{
	GENERATED_BODY()

	// [v1.0.0] 디버그와 장비별 정책 구분에 사용할 컨텍스트 식별자입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Context", meta=(DisplayName="컨텍스트 ID (ContextId)", ToolTip="기본 선택, 스캔, 수리, 해킹처럼 선택 요청의 목적을 구분하는 식별자입니다."))
	FName ContextId = TEXT("Default");

	// [v1.0.0] 컴포넌트 소유 Actor 자신을 선택할 수 있는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Context", meta=(DisplayName="자기 선택 허용 (bAllowSelfTarget)", ToolTip="True이면 타겟 선택 컴포넌트의 소유 Actor 자신도 선택할 수 있습니다."))
	bool bAllowSelfTarget = false;

	// [v1.0.0] 새 대상을 선택할 때 시야 확인이 필요하다는 정책 표시입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Context", meta=(DisplayName="신규 선택 시 시야 요구 (bRequireLineOfSightForNewSelection)", ToolTip="True이면 후보 검색 단계가 새 대상 선택 전에 시야를 확인해야 합니다. TS-P0-01 컴포넌트는 Trace를 수행하지 않습니다."))
	bool bRequireLineOfSightForNewSelection = true;

	// [v1.0.0] 선택을 허용할 대상 분류 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Context", meta=(DisplayName="허용 분류 (AllowedCategories)", ToolTip="선택을 허용할 대상 분류입니다. 비어 있으면 모든 분류를 허용합니다."))
	TArray<ECFTargetCategory> AllowedCategories;

	// [v1.0.0] 선택을 허용할 관계 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Context", meta=(DisplayName="허용 관계 (AllowedRelations)", ToolTip="선택을 허용할 관계입니다. 비어 있으면 모든 관계를 허용합니다."))
	TArray<ECFTargetRelation> AllowedRelations;

	// [v1.0.0] 대상이 반드시 모두 가져야 하는 속성 태그 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Context", meta=(DisplayName="필수 속성 태그 (RequiredAttributeTags)", ToolTip="대상이 선택 가능하려면 모두 포함해야 하는 속성 태그입니다."))
	TArray<FName> RequiredAttributeTags;

	// [v1.0.0] 대상이 하나라도 가지면 선택을 거부할 속성 태그 목록입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Context", meta=(DisplayName="제외 속성 태그 (ExcludedAttributeTags)", ToolTip="대상이 하나라도 포함하면 선택을 거부할 속성 태그입니다."))
	TArray<FName> ExcludedAttributeTags;
};

/**
 * 후보 검색 단계가 평가하고 선택 컴포넌트에 전달할 후보 데이터입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="타겟 후보 데이터 (Target Candidate)", ToolTip="후보 Actor, 대표 위치, 표시 정보와 화면·거리 평가값을 전달합니다."))
struct FCFTargetCandidate
{
	GENERATED_BODY()

	// [v1.0.0] 후보로 평가된 실제 Actor입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Candidate", meta=(DisplayName="후보 Actor (TargetActor)", ToolTip="후보 검색 단계가 평가한 실제 대상 Actor입니다."))
	TObjectPtr<AActor> TargetActor = nullptr;

	// [v1.0.0] 후보 평가와 UI 투영에 사용할 대표 월드 위치입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Candidate", meta=(DisplayName="타겟 월드 위치 (TargetWorldLocation)", ToolTip="선택 지점 컴포넌트 또는 Actor Bounds 중심에서 얻은 후보 대표 월드 위치입니다."))
	FVector TargetWorldLocation = FVector::ZeroVector;

	// [v1.0.0] 후보 시점에 확인한 대상 표시 정보입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Candidate", meta=(DisplayName="표시 정보 (DisplayInfo)", ToolTip="후보 시점에 선택 가능 인터페이스에서 확인한 대상 표시 정보입니다."))
	FCFTargetDisplayInfo DisplayInfo;

	// [v1.0.0] 크로스헤어 직접 조준 Trace가 이 후보를 맞췄는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Candidate", meta=(DisplayName="직접 조준 명중 (bDirectAimHit)", ToolTip="크로스헤어 직접 조준 Trace가 이 후보를 맞췄는지 여부입니다. TS-P0-01은 Trace를 수행하지 않습니다."))
	bool bDirectAimHit = false;

	// [v1.0.0] 후보 수집 시점에 직접 가시 상태였는지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Candidate", meta=(DisplayName="가시 여부 (bVisible)", ToolTip="후보 수집 시점에 대상이 직접 가시 상태였는지 여부입니다."))
	bool bVisible = false;

	// [v1.0.0] 크로스헤어 방향과 후보 방향 사이의 각도입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Candidate", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg", DisplayName="크로스헤어 각도 (CrosshairAngleDeg)", ToolTip="크로스헤어 방향과 후보 방향 사이의 각도입니다. 단위는 도(deg)입니다."))
	float CrosshairAngleDeg = 180.0f;

	// [v1.0.0] 화면 중심에서 후보까지의 정규화 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Candidate", meta=(ClampMin="0.0", DisplayName="정규화 화면 거리 (NormalizedScreenDistance)", ToolTip="화면 중심에서 후보까지의 정규화 거리입니다. 0에 가까울수록 크로스헤어에 가깝습니다."))
	float NormalizedScreenDistance = 1.0f;

	// [v1.0.0] 선택 원점에서 후보 대표 위치까지의 월드 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Candidate", meta=(ClampMin="0.0", Units="cm", DisplayName="월드 거리 (WorldDistanceCm)", ToolTip="선택 원점에서 후보 대표 위치까지의 거리입니다. 단위는 센티미터(cm)입니다."))
	float WorldDistanceCm = 0.0f;

	// [v1.0.0] 점수가 같은 후보의 결과 순서를 안정화할 정렬 키입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Candidate", meta=(DisplayName="안정 정렬 키 (StableSortKey)", ToolTip="점수가 같은 후보 사이에서 프레임마다 순서가 흔들리지 않도록 사용할 안정적인 정렬 키입니다."))
	FName StableSortKey = NAME_None;
};

/**
 * 후보 평가 함수에 전달할 카메라 View와 가시성 입력입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="타겟 검색 View (Target Search View)", ToolTip="후보 화면 근접도와 직접 조준 여부를 계산할 카메라 View, FOV, 화면비와 가시 대상 목록입니다."))
struct FCFTargetSearchView
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Search", meta=(DisplayName="View 원점 (ViewOrigin)", ToolTip="카메라 Aim Trace와 후보 방향 계산을 시작할 월드 위치입니다."))
	FVector ViewOrigin = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Search", meta=(DisplayName="View 방향 (ViewDirection)", ToolTip="화면 중앙 크로스헤어가 향하는 정규화 월드 방향입니다."))
	FVector ViewDirection = FVector::ForwardVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Search", meta=(DisplayName="View 위쪽 방향 (ViewUpDirection)", ToolTip="화면 상단 방향을 나타내는 월드 벡터입니다. ViewDirection과 직교하도록 내부에서 보정합니다."))
	FVector ViewUpDirection = FVector::UpVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Search", meta=(ClampMin="1.0", ClampMax="179.0", Units="deg", DisplayName="수직 FOV (VerticalFOVDeg)", ToolTip="화면 근접도 투영에 사용할 수직 시야각입니다."))
	float VerticalFOVDeg = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Search", meta=(ClampMin="0.1", DisplayName="Viewport 화면비 (ViewportAspectRatio)", ToolTip="Viewport 가로 길이를 세로 길이로 나눈 화면비입니다."))
	float ViewportAspectRatio = 1.7777778f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Search", meta=(DisplayName="직접 조준 적중 Actor (DirectAimHitActor)", ToolTip="TargetSelect 중앙 Trace가 직접 적중한 Actor입니다. 유효한 선택 대상일 때 모든 근접 후보보다 우선합니다."))
	TObjectPtr<AActor> DirectAimHitActor = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Search", meta=(DisplayName="가시 대상 Actor 목록 (VisibleTargetActors)", ToolTip="시야 요구 컨텍스트에서 현재 직접 관측 가능한 대상으로 판정된 Actor 목록입니다."))
	TArray<TObjectPtr<AActor>> VisibleTargetActors;
};

/**
 * 한 번의 후보 평가가 생성한 정렬 결과와 진단 집계입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="타겟 검색 결과 (Target Search Result)", ToolTip="결정적 순서로 정렬된 후보 목록, 최종 후보와 평가 집계를 제공합니다."))
struct FCFTargetSearchResult
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search", meta=(DisplayName="최종 후보 존재 여부 (bHasBestCandidate)", ToolTip="필터와 거리, 각도 조건을 통과한 최종 후보가 존재하는지 나타냅니다."))
	bool bHasBestCandidate = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search", meta=(DisplayName="안정화를 위해 기존 후보 유지 (bKeptCurrentCandidateForStability)", ToolTip="새 근접 후보의 우위가 부족해 기존 후보를 유지했는지 나타냅니다."))
	bool bKeptCurrentCandidateForStability = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search", meta=(DisplayName="입력 Actor 수 (InputActorCount)", ToolTip="후보 평가 함수에 전달된 전체 Actor 수입니다."))
	int32 InputActorCount = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search", meta=(DisplayName="허용 후보 수 (AcceptedCandidateCount)", ToolTip="선택 필터와 직접 또는 근접 후보 조건을 통과한 Actor 수입니다."))
	int32 AcceptedCandidateCount = 0;

			UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search", meta=(DisplayName="직접 조준 후보 수 (DirectAimCandidateCount)", ToolTip="직접 조준 적중 후보로 인정된 Actor 수입니다."))
	int32 DirectAimCandidateCount = 0;

	// [v1.4.0] Registry 전환 이전 진단 호환 필드이며 반복 월드 Actor 스캔이 제거됐음을 나타내기 위해 런타임에서도 0을 유지합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search|Diagnostics", meta=(DisplayName="레거시 런타임 월드 Actor 스캔 수 (RuntimeWorldActorScanCount)", ToolTip="Target Registry 전환 이후 후보 갱신에서는 전체 월드 Actor를 반복 순회하지 않으므로 0을 유지합니다. 호환 진단 필드입니다."))
	int32 RuntimeWorldActorScanCount = 0;

	// [v1.4.0] 마지막 런타임 후보 갱신에서 Registry가 반환한 유효 TargetSelectable Actor 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search|Diagnostics", meta=(DisplayName="런타임 Registry Targetable 수 (RuntimeRegistryTargetableCount)", ToolTip="마지막 런타임 후보 갱신에서 월드 Target Registry snapshot이 반환한 유효 ICFTargetSelectable Actor 수입니다. 자기 자신이나 컨텍스트 필터 대상도 Registry에는 포함될 수 있습니다."))
	int32 RuntimeRegistryTargetableCount = 0;

		// [v1.2.0] 실제 런타임 검색에서 Targetable Actor의 시야 확인을 위해 수행한 Visibility Trace 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search|Diagnostics", meta=(DisplayName="런타임 Visibility Trace 수 (RuntimeVisibilityTraceCount)", ToolTip="마지막 런타임 후보 갱신에서 직접 조준 Trace 외에 후보 시야 확인을 위해 실제 수행한 Trace 수입니다. 순수 Evaluate Candidate Actors 호출에서는 0입니다."))
	int32 RuntimeVisibilityTraceCount = 0;

	// [v1.3.0] 비-Direct Targetable Actor 중 거리 또는 반각 조건상 proximity 후보가 될 수 없어 Visibility Trace를 생략한 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search|Diagnostics", meta=(DisplayName="Visibility Trace 사전필터 생략 수 (RuntimeVisibilityPrefilterSkipCount)", ToolTip="마지막 런타임 후보 갱신에서 proximity 거리·반각 밖이라 LOS Trace를 실행하지 않은 Targetable Actor 수입니다. 후보 배열 자체에서는 제거하지 않습니다."))
	int32 RuntimeVisibilityPrefilterSkipCount = 0;

	// [v1.2.0] 실제 런타임 검색에서 수행한 직접 조준 Trace와 Visibility Trace를 합친 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search|Diagnostics", meta=(DisplayName="런타임 전체 Trace 수 (RuntimeTotalTraceCount)", ToolTip="마지막 런타임 후보 갱신의 직접 조준 Trace와 Visibility Trace를 합친 수입니다. 검색 View를 만들지 못했거나 순수 평가만 수행하면 0입니다."))
	int32 RuntimeTotalTraceCount = 0;

	// [v1.2.0] 실제 RefreshCurrentCandidate 한 번의 View 생성·월드 수집·후보 평가에 걸린 경과 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search|Diagnostics", meta=(ClampMin="0.0", Units="ms", DisplayName="런타임 검색 시간 ms (RuntimeSearchDurationMs)", ToolTip="마지막 Refresh Current Candidate의 View 생성, Registry 후보 수집, Trace와 후보 평가 전체에 걸린 경과 시간입니다. 성능 관측용이며 후보 판정에 사용하지 않습니다."))
	float RuntimeSearchDurationMs = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search", meta=(DisplayName="최종 후보 (BestCandidate)", ToolTip="직접 조준, 화면 근접도, 월드 거리와 안정 정렬 키 순으로 선택된 최종 후보입니다."))
	FCFTargetCandidate BestCandidate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Search", meta=(DisplayName="정렬 후보 목록 (SortedCandidates)", ToolTip="계층형 규칙에 따라 결정적으로 정렬된 전체 허용 후보 목록입니다."))
	TArray<FCFTargetCandidate> SortedCandidates;
};

/**
 * 타겟 후보 검색과 선택 안정화에 사용할 공통 설정입니다.
 */
USTRUCT(BlueprintType, meta=(DisplayName="타겟 선택 설정 (Target Select Config)", ToolTip="직접 선택 거리, 근접 후보 범위, 갱신 주기, 가림 유예와 후보 전환 우위 비율을 정의합니다."))
struct FCFTargetSelectConfig
{
	GENERATED_BODY()

	// [v1.0.0] 직접 조준 명중 대상을 선택할 수 있는 최대 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Config", meta=(ClampMin="0.0", Units="cm", DisplayName="직접 선택 최대 거리 (DirectSelectMaxDistanceCm)", ToolTip="크로스헤어 직접 조준 명중 대상을 선택할 수 있는 최대 거리입니다. 단위는 센티미터(cm)입니다."))
	float DirectSelectMaxDistanceCm = 200000.0f;

	// [v1.0.0] 크로스헤어 근접 후보를 선택할 수 있는 최대 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Config", meta=(ClampMin="0.0", Units="cm", DisplayName="근접 선택 최대 거리 (ProximitySelectMaxDistanceCm)", ToolTip="크로스헤어 근접 후보를 선택할 수 있는 최대 거리입니다. 단위는 센티미터(cm)입니다."))
	float ProximitySelectMaxDistanceCm = 120000.0f;

	// [v1.0.0] 근접 후보 수집에 사용할 크로스헤어 기준 반각입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Config", meta=(ClampMin="0.1", ClampMax="180.0", Units="deg", DisplayName="근접 후보 반각 (ProximityHalfAngleDeg)", ToolTip="크로스헤어 근접 후보를 수집할 원뿔의 반각입니다. 단위는 도(deg)입니다."))
	float ProximityHalfAngleDeg = 7.0f;

	// [v1.0.0] 후보 목록을 다시 계산할 기본 시간 간격입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Config", meta=(ClampMin="0.001", Units="s", DisplayName="후보 갱신 간격 (CandidateRefreshIntervalSec)", ToolTip="후보 검색 단계가 후보 목록을 다시 계산할 기본 시간 간격입니다. 단위는 초(s)입니다."))
	float CandidateRefreshIntervalSec = 0.05f;

	// [v1.0.0] 선택 대상이 가려진 뒤 즉시 해제하지 않고 추적을 유지할 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Config", meta=(ClampMin="0.0", Units="s", DisplayName="가림 유예 시간 (OcclusionGracePeriodSec)", ToolTip="선택 대상이 가려진 뒤 추정 추적으로 유지할 수 있는 기본 유예 시간입니다. 단위는 초(s)입니다."))
	float OcclusionGracePeriodSec = 1.5f;

	// [v1.0.0] 현재 후보를 새 후보로 바꾸기 위해 요구할 최소 상대 우위 비율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Config", meta=(ClampMin="0.0", ClampMax="1.0", DisplayName="후보 전환 우위 비율 (CandidateSwitchAdvantageRatio)", ToolTip="후보 흔들림을 줄이기 위해 새 후보가 현재 후보보다 더 좋아야 하는 최소 상대 우위 비율입니다. 0~1 범위입니다."))
	float CandidateSwitchAdvantageRatio = 0.15f;
};
