// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.7.0
// Date: 2026-08-31
// Description: CarFight 타겟 선택 상태, Registry 후보 탐색, 선택 수명·장비 조회와 TS-P0-08 LOS 사전필터·검색 진단 컴포넌트
// Scope: 후보 결정적 정렬, 선택 대상 수명, 장비별 읽기 전용 사용 가능 평가와 후보 디버그 표시 계약을 제공합니다.
// Changelog:
// - v1.7.0: 반복 TActorIterator 후보 수집을 UCFTargetRegistrySubsystem snapshot으로 교체하고 Registry 대상 수 진단을 추가. 직접 조준·LOS·정렬·히스테리시스 계약은 유지.
// - v1.6.0: 비-Direct Targetable Actor가 기존 proximity 거리·반각 밖이면 LOS Trace만 생략하고 후보 입력 배열은 유지하는 의미 보존 사전필터를 추가.
// - v1.5.0: TS-P0-08 런타임 후보 검색의 월드 Actor 스캔 수·Trace 수·경과시간을 LastCandidateSearchResult와 DebugSummary에 기록하도록 진단 계약 확장.
// - v1.4.0: TS-P0-08에서 자동 후보 디버그 표시를 후보 갱신 간격 동안 유지하고 Sphere 반경을 데이터 기반 Debug 값으로 분리해 한 프레임 하드코딩을 제거.
// - v1.3.0: TS-P0-07 장비별 선택 대상 호환성·거리·실패 사유 평가 API를 추가.
// - v1.2.1: Actor Destroy 전용 OnDestroyed 구독을 추가해 BeginPlay 이전 Actor 파괴도 즉시 선택 해제하도록 보강.
// - v1.2.0: TS-P0-04 선택 대상 수명 구독, 가림 유예, 거리 이탈, 컴포넌트 비활성화와 자동 해제 계약을 추가.
// - v1.1.0: TS-P0-03 직접 조준 우선, 화면 근접도 차선, 결정적 정렬, 자동 갱신과 디버그 검색 결과를 추가.
// - v1.0.1: Native C++ TargetSelectable과 실제 Blueprint 재정의를 구분하는 안전 인터페이스 디스패치 정책을 반영.
// - v1.0.0: TS-P0-01용 설정 해석, 대상 필터, 후보/선택 상태 API와 BlueprintAssignable 이벤트를 추가.
// Migration:
// - v1.7.0 TargetSelect는 Sensor Snapshot에 의존하지 않으며 월드 Target Registry를 후보 공급원으로 사용합니다. 직접 Trace 적중 Actor는 Registry 반영 시점과 무관하게 안전 보강합니다.
// - v1.6.0 LOS 사전필터는 `!Direct && (Distance > ProximityMax || Angle > ProximityHalfAngle)`인 대상의 Visibility Trace만 생략한다. CandidateActors, EvaluateCandidateActors와 안정화 규칙은 유지한다.
// - v1.5.0 검색 진단값은 관측 전용이며 후보 판정·정렬·Refresh 주기 결정에 피드백하지 않는다. 순수 EvaluateCandidateActors는 기존처럼 월드 Trace를 수행하지 않는다.
// - TS-P0-08 자동 후보 디버그 Sphere는 CandidateRefreshIntervalSec 동안 유지되며 CandidateSearchDebugSphereRadiusCm으로 크기만 조절한다. 후보 판정 거리·반각·히스테리시스 값은 변경하지 않는다.
// - ACFVehiclePawn 부착은 TS-P0-01에서 완료했으며 Enhanced Input, HUD와 장비 소비 연결은 후속 Task에서 수행한다.
// - 장기 Actor 참조는 약한 참조로 유지하며 검색 결과 구조체의 Actor는 컴포넌트 캐시에 강하게 보관하지 않는다.
// - 선택 대상은 OnDestroyed, OnEndPlay와 VehicleHealth 파괴 이벤트를 자동 구독하고 변경·해제 시 기존 구독을 제거한다.
// - 화면 밖 여부는 선택 수명에 사용하지 않으며 월드 가림은 설정된 유예 시간 동안 Occluded 상태로 유지한다.

#pragma once

#include "CoreMinimal.h"
#include "CFDamageTypes.h"
#include "CFTargetSelectTypes.h"
#include "CFTargetUseTypes.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "CFTargetSelectComp.generated.h"

class AActor;
class UCFTargetSelectData;
class UCFVehicleHealthComp;

// [v1.0.0] 현재 타겟 후보 Actor가 변경됐을 때 이전 후보, 새 후보와 새 후보 데이터를 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCFTargetCandidateChangedSignature, AActor*, PreviousCandidate, AActor*, NewCandidate, FCFTargetCandidate, CandidateData);

// [v1.0.0] 선택 대상이 변경됐을 때 이전 대상, 새 대상과 새 표시 정보를 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCFSelectedTargetChangedSignature, AActor*, PreviousTarget, AActor*, NewTarget, FCFTargetDisplayInfo, DisplayInfo);

// [v1.0.0] 선택 대상이 명시적으로 해제됐을 때 대상과 해제 사유를 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCFSelectedTargetClearedSignature, AActor*, ClearedTarget, ECFTargetClearReason, ClearReason);

// [v1.0.0] 선택 대상의 논리 유효성이 변경됐을 때 대상과 새 유효성 값을 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCFSelectedTargetValidityChangedSignature, AActor*, TargetActor, bool, bIsValidTarget);

// [v1.0.0] 선택 대상의 추적 상태가 변경됐을 때 대상과 이전/새 상태를 전달하는 BP 이벤트입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FCFSelectedTargetTrackChangedSignature, AActor*, TargetActor, ECFTargetTrackState, PreviousState, ECFTargetTrackState, NewState);

/**
 * 후보 검색 결과와 선택 대상 상태를 저장하고 다른 시스템에 전달하는 최소 런타임 컴포넌트입니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFTargetSelectComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] 후보 검색과 선택 대상 수명 검사를 위한 Tick 기본값을 초기화합니다.
					UCFTargetSelectComp();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Deactivate() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// 유효한 DataAsset 설정 또는 컴포넌트 Fallback 설정을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Config", meta=(DisplayName="해석된 타겟 선택 설정 반환 (Get Resolved Target Select Config)", ToolTip="TargetSelectData가 존재하고 유효하면 해당 설정을 반환하고, 아니면 FallbackTargetSelectConfig를 반환합니다."))
	FCFTargetSelectConfig GetResolvedTargetSelectConfig() const;

	// [v1.0.0] 대상 Actor가 인터페이스와 선택 컨텍스트 필터를 모두 만족하는지 반환합니다.
				UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Selection", meta=(DisplayName="Actor 타겟 사용 가능 여부 (Can Use Actor As Target)", ToolTip="Actor 유효성, 자기 선택 정책, TargetSelectable 인터페이스, 차량 파괴 상태, 분류, 관계와 속성 태그 필터를 검사합니다. 시야 Trace는 수행하지 않습니다."))
	bool CanUseActorAsTarget(AActor* TargetActor, const FCFTargetSelectionContext& SelectionContext) const;

	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|Search", meta=(DisplayName="현재 타겟 후보 갱신 (Refresh Current Candidate)", ToolTip="Owner 차량의 카메라 Aim, TargetSelect Trace와 현재 월드 Target Registry의 선택 가능 Actor를 사용해 최종 후보를 다시 계산합니다."))
	bool RefreshCurrentCandidate();

	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|Search", meta=(DisplayName="후보 Actor 목록 평가 (Evaluate Candidate Actors)", ToolTip="주어진 Actor 목록을 직접 조준, 화면 근접도, 월드 거리와 안정 정렬 키 순으로 평가합니다. 월드 Trace는 수행하지 않습니다."))
	FCFTargetSearchResult EvaluateCandidateActors(const TArray<AActor*>& CandidateActors, const FCFTargetSearchView& SearchView, const FCFTargetSelectionContext& SelectionContext) const;

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Search", meta=(DisplayName="정규화 화면 거리 계산 (Calculate Normalized Screen Distance)", ToolTip="카메라 View, 수직 FOV와 화면비를 기준으로 화면 중심에서 월드 위치까지의 정규화 거리를 계산합니다."))
	static float CalculateNormalizedScreenDistance(const FCFTargetSearchView& SearchView, const FVector& TargetWorldLocation);

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Search", meta=(DisplayName="마지막 후보 검색 결과 반환 (Get Last Candidate Search Result)", ToolTip="마지막 런타임 후보 갱신에서 계산된 정렬 후보와 최종 후보 결과를 반환합니다."))
	FCFTargetSearchResult GetLastCandidateSearchResult() const;

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Debug", meta=(DisplayName="후보 검색 디버그 요약 생성 (Build Candidate Search Debug Summary)", ToolTip="마지막 후보 검색의 입력·허용·직접 후보 수, Registry 대상 수·Trace·검색 시간, 최종 후보와 안정화 여부를 한 줄 문자열로 생성합니다."))
	FString BuildCandidateSearchDebugSummary() const;

	UFUNCTION(BlueprintCallable, CallInEditor, Category="CarFight|TargetSelect|Debug", meta=(DisplayName="후보 검색 디버그 그리기 (Draw Candidate Search Debug)", ToolTip="마지막 후보 검색 목록의 위치와 최종 후보를 월드 디버그 도형으로 표시합니다."))
	void DrawCandidateSearchDebug(float DurationSeconds = 2.0f, float SphereRadius = 20.0f) const;

	// 검색 단계가 제공한 현재 후보를 저장하고 후보 변경 이벤트를 발생시킵니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|Candidate", meta=(DisplayName="현재 타겟 후보 설정 (Set Current Candidate)", ToolTip="유효한 후보 Actor와 후보 데이터를 저장합니다. 후보 변경은 선택 대상을 자동으로 변경하거나 해제하지 않습니다."))
	bool SetCurrentCandidate(const FCFTargetCandidate& Candidate, const FCFTargetSelectionContext& SelectionContext);

	// [v1.0.0] 현재 후보만 명시적으로 비우고 선택 대상은 유지합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|Candidate", meta=(DisplayName="현재 타겟 후보 해제 (Clear Current Candidate)", ToolTip="현재 후보 데이터와 약한 Actor 참조만 비웁니다. 기존 선택 대상은 유지합니다."))
	void ClearCurrentCandidate();

	// [v1.0.0] 유효한 Actor를 현재 선택 대상으로 설정하고 선택 변경 이벤트를 발생시킵니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|Selection", meta=(DisplayName="선택 타겟 설정 (Set Selected Target)", ToolTip="선택 컨텍스트를 만족하는 Actor를 선택 대상으로 설정합니다. 무효 입력은 False를 반환하고 기존 선택을 유지하며, 같은 대상 재선택은 이벤트 없이 True를 반환합니다."))
	bool SetSelectedTarget(AActor* TargetActor, const FCFTargetSelectionContext& SelectionContext);

	// [v1.0.0] 선택 대상을 명시적인 사유와 함께 해제하고 후보는 유지합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|Selection", meta=(DisplayName="선택 타겟 해제 (Clear Selected Target)", ToolTip="현재 선택 기록과 표시 정보를 명시적인 해제 사유와 함께 비웁니다. 현재 후보는 유지합니다."))
	void ClearSelectedTarget(ECFTargetClearReason ClearReason);

	// [v1.0.0] 외부 수명 시스템이 선택 대상의 논리 유효성 변경을 통지합니다.
				UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|Lifetime", meta=(DisplayName="선택 타겟 유효성 변경 통지 (Notify Selected Target Validity Changed)", ToolTip="Health, Destroy, EndPlay 또는 게임플레이 상태 시스템이 현재 선택 대상의 유효성 변경을 통지합니다."))
	bool NotifySelectedTargetValidityChanged(AActor* TargetActor, bool bIsValidTarget);

	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|Lifetime", meta=(DisplayName="선택 타겟 수명 갱신 (Refresh Selected Target Lifetime)", ToolTip="현재 선택 대상의 참조, 선택 가능 상태, 추적 거리와 월드 시야를 검사하고 필요한 경우 유효성 전이와 자동 해제를 수행합니다."))
	bool RefreshSelectedTargetLifetime(float DeltaTime);

	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|Lifetime", meta=(DisplayName="선택 타겟 가시성 갱신 (Update Selected Target Visibility)", ToolTip="현재 선택 대상의 월드 가시성 결과와 경과 시간을 반영합니다. 화면 밖 여부는 선택 해제 조건으로 사용하지 않습니다."))
	bool UpdateSelectedTargetVisibility(bool bIsVisible, float DeltaTime);

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Lifetime", meta=(DisplayName="선택 타겟 가림 경과 시간 반환", ToolTip="현재 선택 대상이 연속으로 가려진 시간을 초 단위로 반환합니다."))
	float GetSelectedTargetOcclusionElapsedSeconds() const { return SelectedTargetOcclusionElapsedSeconds; }

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Lifetime", meta=(DisplayName="마지막 선택 해제 사유 반환", ToolTip="가장 최근에 선택 대상이 해제된 사유를 반환합니다."))
	ECFTargetClearReason GetLastSelectedTargetClearReason() const { return LastSelectedTargetClearReason; }

	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Debug", meta=(DisplayName="선택 수명 디버그 요약 생성", ToolTip="선택 대상, 유효성, 추적 상태, 가림 시간과 마지막 해제 사유를 한 줄 문자열로 생성합니다."))
	FString BuildSelectedTargetLifetimeDebugSummary() const;

	// 외부 추적 시스템이 현재 선택 대상의 추적 상태를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|TargetSelect|Tracking", meta=(DisplayName="선택 타겟 추적 상태 설정 (Set Selected Target Track State)", ToolTip="현재 선택 대상의 추적 상태를 갱신하고 실제 상태 변경 때만 이벤트를 발생시킵니다."))
	bool SetSelectedTargetTrackState(AActor* TargetActor, ECFTargetTrackState NewTrackState);

	// [v1.0.0] 현재 기본 선택 컨텍스트 사본을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Config", meta=(DisplayName="기본 선택 컨텍스트 반환 (Get Default Selection Context)", ToolTip="컴포넌트에 설정된 기본 타겟 선택 컨텍스트 사본을 반환합니다."))
	FCFTargetSelectionContext GetDefaultSelectionContext() const { return DefaultSelectionContext; }

	// [v1.0.0] 현재 유효한 후보 Actor를 반환하며 무효 약한 참조는 Null을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Candidate", meta=(DisplayName="현재 타겟 후보 Actor 반환 (Get Current Candidate Actor)", ToolTip="현재 유효한 후보 Actor를 반환합니다. 후보가 없거나 약한 참조가 무효하면 Null을 반환합니다."))
	AActor* GetCurrentCandidateActor() const;

	// [v1.0.0] 현재 유효한 후보 데이터 사본을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Candidate", meta=(DisplayName="현재 타겟 후보 데이터 반환 (Get Current Candidate Data)", ToolTip="현재 유효한 후보 데이터 사본을 반환하고 TargetActor를 안전하게 다시 주입합니다. 후보가 무효하면 기본값을 반환합니다."))
	FCFTargetCandidate GetCurrentCandidateData() const;

	// [v1.0.0] 현재 유효한 후보 Actor가 있는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Candidate", meta=(DisplayName="현재 타겟 후보 존재 여부 (Has Current Candidate)", ToolTip="현재 후보 기록과 약한 Actor 참조가 모두 유효한지 반환합니다."))
	bool HasCurrentCandidate() const;

	// [v1.0.0] 현재 유효한 선택 대상 Actor를 반환하며 무효 약한 참조는 Null을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Selection", meta=(DisplayName="선택 타겟 Actor 반환 (Get Selected Target Actor)", ToolTip="현재 유효한 선택 대상 Actor를 반환합니다. 선택 기록이 없거나 약한 참조가 무효하면 Null을 반환합니다."))
	AActor* GetSelectedTargetActor() const;

	// [v1.0.0] 현재 선택 대상의 캐시된 표시 정보를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Selection", meta=(DisplayName="선택 타겟 표시 정보 반환 (Get Selected Target Display Info)", ToolTip="현재 선택 대상에서 선택 시점에 읽은 표시 정보 사본을 반환합니다. 선택 기록이 없으면 기본값을 반환합니다."))
	FCFTargetDisplayInfo GetSelectedTargetDisplayInfo() const;

	// [v1.0.0] 유효성 여부와 관계없이 현재 선택 기록이 존재하는지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Selection", meta=(DisplayName="선택 타겟 기록 존재 여부 (Has Selected Target)", ToolTip="대상이 나중에 무효화됐더라도 명시적으로 해제되지 않은 선택 기록이 존재하는지 반환합니다."))
	bool HasSelectedTarget() const { return bHasSelectedTarget; }

	// [v1.0.0] 현재 선택 기록과 약한 Actor 참조가 모두 유효한지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Selection", meta=(DisplayName="선택 타겟 유효 여부 (Is Selected Target Valid)", ToolTip="선택 기록이 존재하고 외부 유효성 상태와 약한 Actor 참조가 모두 유효한지 반환합니다."))
	bool IsSelectedTargetValid() const;

	// [v1.0.0] 후보와 선택 상태를 기준으로 계산한 현재 핵심 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|State", meta=(DisplayName="타겟 핵심 상태 반환 (Get Target Core State)", ToolTip="선택 기록을 후보보다 우선해 NoCandidate, CandidateAvailable, TargetSelected 또는 SelectedTargetInvalid 상태를 반환합니다."))
	ECFTargetCoreState GetTargetCoreState() const;

	// [v1.0.0] 현재 선택 대상의 추적 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Tracking", meta=(DisplayName="선택 타겟 추적 상태 반환 (Get Selected Target Track State)", ToolTip="유효한 선택 대상의 현재 추적 상태를 반환합니다. 선택 대상이 없거나 무효하면 Invalid를 반환합니다."))
			ECFTargetTrackState GetSelectedTargetTrackState() const;

	// [v1.3.0] 장비 요청 조건으로 현재 선택 대상을 읽기 전용 평가합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Equipment", meta=(DisplayName="선택 타겟 장비 사용 평가", ToolTip="현재 선택 상태를 변경하지 않고 장비 준비 여부, 대상 호환성, 추적 상태와 사용 거리를 평가합니다."))
	FCFTargetUseResult EvaluateSelectedTargetForUse(const FCFTargetUseRequest& UseRequest) const;

	// [v1.3.0] 장비 사용 평가 결과를 한 줄 디버그 문자열로 생성합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|TargetSelect|Equipment", meta=(DisplayName="타겟 사용 평가 요약 생성", ToolTip="장비 ID, 선택 대상, 호환성, 거리와 실패 사유를 한 줄 문자열로 생성합니다."))
		FString BuildTargetUseDebugSummary(const FCFTargetUseResult& UseResult) const;

	// [v1.0.0] 현재 후보 Actor가 변경될 때 호출되는 BP 할당 가능 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|TargetSelect|Events", meta=(DisplayName="타겟 후보 변경 이벤트 (OnTargetCandidateChanged)", ToolTip="현재 후보 Actor가 실제로 변경되거나 명시적으로 해제될 때 호출됩니다."))
	FCFTargetCandidateChangedSignature OnTargetCandidateChanged;

	// [v1.0.0] 현재 선택 대상이 다른 Actor로 변경될 때 호출되는 BP 할당 가능 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|TargetSelect|Events", meta=(DisplayName="선택 타겟 변경 이벤트 (OnSelectedTargetChanged)", ToolTip="현재 선택 대상이 다른 유효 Actor로 변경될 때 이전 대상, 새 대상과 표시 정보를 전달합니다."))
	FCFSelectedTargetChangedSignature OnSelectedTargetChanged;

	// [v1.0.0] 현재 선택 대상이 명시적으로 해제될 때 호출되는 BP 할당 가능 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|TargetSelect|Events", meta=(DisplayName="선택 타겟 해제 이벤트 (OnSelectedTargetCleared)", ToolTip="현재 선택 기록이 명시적인 사유와 함께 해제될 때 호출됩니다."))
	FCFSelectedTargetClearedSignature OnSelectedTargetCleared;

	// [v1.0.0] 현재 선택 대상의 논리 유효성이 변경될 때 호출되는 BP 할당 가능 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|TargetSelect|Events", meta=(DisplayName="선택 타겟 유효성 변경 이벤트 (OnSelectedTargetValidityChanged)", ToolTip="외부 수명 시스템 통지로 현재 선택 대상의 논리 유효성이 실제로 변경될 때 호출됩니다."))
	FCFSelectedTargetValidityChangedSignature OnSelectedTargetValidityChanged;

	// [v1.0.0] 현재 선택 대상의 추적 상태가 변경될 때 호출되는 BP 할당 가능 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|TargetSelect|Events", meta=(DisplayName="선택 타겟 추적 상태 변경 이벤트 (OnSelectedTargetTrackStateChanged)", ToolTip="외부 추적 시스템이 현재 선택 대상의 추적 상태를 실제로 변경할 때 호출됩니다."))
	FCFSelectedTargetTrackChangedSignature OnSelectedTargetTrackStateChanged;

	// [v1.0.0] 우선 사용할 타겟 선택 설정 DataAsset입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Config", meta=(DisplayName="타겟 선택 데이터 (TargetSelectData)", ToolTip="유효하면 Fallback 설정보다 우선 사용할 타겟 선택 설정 DataAsset입니다."))
	TObjectPtr<UCFTargetSelectData> TargetSelectData = nullptr;

	// [v1.0.0] DataAsset이 없거나 유효하지 않을 때 사용할 안전 기본 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Config", meta=(DisplayName="기본 타겟 선택 설정 (FallbackTargetSelectConfig)", ToolTip="TargetSelectData가 없거나 유효하지 않을 때 사용할 컴포넌트 내장 설정입니다."))
	FCFTargetSelectConfig FallbackTargetSelectConfig;

	// [v1.0.0] 일반 타겟 선택 요청에 사용할 기본 필터 컨텍스트입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|TargetSelect|Config", meta=(DisplayName="기본 선택 컨텍스트 (DefaultSelectionContext)", ToolTip="일반 플레이어 선택 요청에서 사용할 기본 자기 선택, 분류, 관계와 속성 태그 필터입니다."))
					FCFTargetSelectionContext DefaultSelectionContext;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Search", meta=(DisplayName="후보 자동 갱신 (bAutoRefreshCandidate)", ToolTip="True이면 로컬 플레이어 차량에서 CandidateRefreshIntervalSec 간격으로 현재 후보를 자동 갱신합니다."))
	bool bAutoRefreshCandidate = true;

				// [v1.0.0] 자동 후보 갱신 직후 마지막 검색 후보와 최종 후보의 디버그 Sphere를 표시할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Debug", meta=(DisplayName="후보 검색 디버그 자동 표시 (bDrawCandidateSearchDebug)", ToolTip="True이면 자동 후보 갱신 직후 마지막 검색 후보와 최종 후보 Sphere를 다음 후보 갱신 시점까지 유지해 표시합니다."))
	bool bDrawCandidateSearchDebug = false;

	// [v1.4.0] 자동 후보 검색 디버그 Sphere의 월드 반경입니다. 게임플레이 후보 판정 범위에는 영향을 주지 않습니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|TargetSelect|Debug", meta=(ClampMin="1.0", Units="cm", DisplayName="후보 디버그 Sphere 반경 (CandidateSearchDebugSphereRadiusCm)", ToolTip="자동 후보 디버그 Sphere의 월드 반경입니다. 디버그 가독성만 조절하며 선택 거리, 반각과 후보 전환 판정에는 영향을 주지 않습니다."))
	float CandidateSearchDebugSphereRadiusCm = 20.0f;

private:
	// [v1.7.0] 실제 런타임 카메라 View와 Registry 후보 Actor를 수집하고 Registry 대상 수·LOS Trace·사전필터 진단값을 반환합니다.
	bool BuildRuntimeSearchView(FCFTargetSearchView& OutSearchView, TArray<AActor*>& OutCandidateActors, int32& OutRegistryTargetableCount, int32& OutVisibilityTraceCount, int32& OutVisibilityPrefilterSkipCount, int32& OutTotalTraceCount) const;
	void CacheLastCandidateSearchResult(const FCFTargetSearchResult& SearchResult);
	bool ShouldKeepCurrentCandidateForStability(const FCFTargetCandidate& RawBestCandidate, const TArray<FCFTargetCandidate>& SortedCandidates, const FCFTargetSelectConfig& Config, FCFTargetCandidate& OutCurrentCandidate) const;
	FVector ResolveTargetSelectionLocation(AActor* TargetActor) const;
	bool IsSelectedTargetVisibleInWorld(AActor* TargetActor) const;
	void BindSelectedTargetLifetimeEvents(AActor* TargetActor);
	void UnbindSelectedTargetLifetimeEvents();

			UFUNCTION()
	void HandleSelectedTargetDestroyed(AActor* DestroyedActor);

	UFUNCTION()
	void HandleSelectedTargetEndPlay(AActor* EndedActor, EEndPlayReason::Type EndPlayReason);

	UFUNCTION()
	void HandleSelectedVehicleDestroyed(FCFDamageHitContext DamageHitContext);

	// 표시 정보가 선택 컨텍스트의 분류, 관계와 속성 태그 필터를 만족하는지 반환합니다.
	bool DoesDisplayInfoMatchContext(const FCFTargetDisplayInfo& DisplayInfo, const FCFTargetSelectionContext& SelectionContext) const;

	// [v1.0.0] 현재 후보 Actor를 GC 강한 참조 없이 보관하는 약한 참조입니다.
	TWeakObjectPtr<AActor> CurrentCandidateActor;

	// [v1.0.0] 현재 선택 대상 Actor를 GC 강한 참조 없이 보관하는 약한 참조입니다.
	TWeakObjectPtr<AActor> SelectedTargetActor;

	// [v1.0.0] 후보 검색 결과에서 Actor 강한 참조를 제거한 캐시 데이터입니다.
	FCFTargetCandidate CurrentCandidateData;

	// [v1.0.0] 현재 선택 대상에서 선택 시점에 읽은 표시 정보입니다.
	FCFTargetDisplayInfo SelectedTargetDisplayInfo;

	// [v1.0.0] 현재 선택 대상의 논리 유효성 상태입니다.
	bool bSelectedTargetValid = false;

	// [v1.0.0] 무효 약한 참조와 구분하기 위한 후보 기록 존재 여부입니다.
	bool bHasCurrentCandidate = false;

	// [v1.0.0] 무효 약한 참조와 구분하기 위한 선택 기록 존재 여부입니다.
	bool bHasSelectedTarget = false;

	// [v1.0.0] 외부 추적 시스템이 마지막으로 저장한 선택 대상 추적 상태입니다.
					ECFTargetTrackState SelectedTargetTrackState = ECFTargetTrackState::Invalid;

	float CandidateRefreshElapsedSeconds = 0.0f;
	float SelectedTargetLifetimeCheckElapsedSeconds = 0.0f;
	float SelectedTargetOcclusionElapsedSeconds = 0.0f;
	ECFTargetClearReason LastSelectedTargetClearReason = ECFTargetClearReason::Manual;
	TWeakObjectPtr<AActor> BoundSelectedTargetActor;
	TWeakObjectPtr<UCFVehicleHealthComp> BoundSelectedVehicleHealthComp;
	FCFTargetSearchResult LastCandidateSearchResult;
	TArray<TWeakObjectPtr<AActor>> LastCandidateActors;
	TWeakObjectPtr<AActor> LastBestCandidateActor;
};
