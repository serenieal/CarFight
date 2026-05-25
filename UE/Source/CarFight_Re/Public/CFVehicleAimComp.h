// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-05-21
// Description: CarFight 차량 Aim 시스템의 복제 Aim 시각 상태 추가
// Scope: Owner Pawn과 VehicleCameraComp 참조를 안전하게 캐시하고 Tick에서 Local Aim 상태를 갱신하며 서버 검증/복제 시각 상태를 저장합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleAimTypes.h"
#include "Components/ActorComponent.h"
#include "CFVehicleAimComp.generated.h"

class ACFVehiclePawn;
class UCFVehicleCameraComp;

/**
 * 차량 카메라 조준과 향후 무기 발사 사이의 해석 계층입니다.
 */
UCLASS(ClassGroup=(CarFight), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class CARFIGHT_RE_API UCFVehicleAimComp : public UActorComponent
{
	GENERATED_BODY()

public:
	// [v1.0.0] 기본 생성자입니다.
	UCFVehicleAimComp();

protected:
	// [v1.0.0] BeginPlay에서 Aim 런타임 참조 초기화를 시도합니다.
	virtual void BeginPlay() override;

public:
	// [v1.1.0] Tick에서 Local Aim 상태를 갱신합니다.
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// [v1.3.0] 복제 대상 Aim 시각 상태를 등록합니다.
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// [v1.0.0] Owner Pawn과 CameraComp 참조를 갱신하고 Aim 런타임 준비 상태를 계산합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Aim", meta=(ToolTip="Owner 차량 Pawn과 VehicleCameraComp 참조를 갱신하고 Aim 런타임 준비 상태를 계산합니다."))
	bool InitializeAimRuntime();

	// [v1.0.0] Aim 런타임에 필요한 Owner Pawn과 CameraComp 참조만 다시 찾습니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Aim", meta=(ToolTip="Aim 런타임에 필요한 Owner 차량 Pawn과 VehicleCameraComp 참조를 다시 찾습니다."))
	bool RefreshAimRuntimeReferences();

	// [v1.0.0] 현재 로컬 Aim 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="현재 로컬 클라이언트 기준 Aim 상태를 반환합니다."))
	FCFVehicleLocalAimState GetLocalAimState() const;

	// [v1.1.0] 현재 로컬 Reticle 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="현재 로컬 클라이언트 기준 Reticle 표시 상태를 반환합니다."))
	ECFVehicleReticleState GetReticleState() const;

	// [v1.0.0] 현재 서버 Aim 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="현재 서버 검증 기준 Aim 상태를 반환합니다."))
	FCFVehicleServerAimState GetServerAimState() const;

	// [v1.0.0] 현재 복제 시각화용 Aim 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="현재 다른 클라이언트 표시용 Aim 시각 상태를 반환합니다."))
	FCFVehicleRepAimVisualState GetRepAimVisualState() const;

	// [v1.0.0] 기본 Aim Profile을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="현재 AimComp가 보유한 기본 Aim Profile을 반환합니다."))
	FCFVehicleAimProfile GetDefaultAimProfile() const;

	// [v1.0.0] Aim 런타임 참조 준비 완료 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="AimComp가 Owner 차량 Pawn과 VehicleCameraComp를 모두 찾았는지 여부를 반환합니다."))
	bool IsAimRuntimeReady() const;

	// [v1.0.0] 마지막 Aim 런타임 초기화 요약 문자열을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="마지막 Aim 런타임 초기화 또는 참조 갱신 결과 요약 문자열을 반환합니다."))
	FString GetLastAimRuntimeSummary() const;

	// [v1.2.0] 현재 Local Aim 상태를 기준으로 서버 발사 요청 데이터를 생성합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon", meta=(ToolTip="현재 Local Aim 상태를 기준으로 서버 발사 요청 데이터를 생성합니다."))
	FCFVehicleFireRequest BuildFireRequest(int32 FireRequestId, float ClientFireTimeSeconds) const;

	// [v1.2.0] 서버 발사 처리 결과를 ServerAimState에 반영합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon", meta=(ToolTip="서버 발사 처리 결과를 ServerAimState에 반영합니다."))
	void ApplyServerFireResult(const FCFVehicleFireResult& FireResult);

	// [v1.3.0] 서버 발사 결과를 다른 클라이언트 표시용 복제 Aim 시각 상태에 반영합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon", meta=(ToolTip="서버 발사 결과를 다른 클라이언트 표시용 복제 Aim 시각 상태에 반영합니다. 전투 판정용으로 사용하지 않습니다."))
	void UpdateRepAimVisualFromFireResult(const FCFVehicleFireRequest& FireRequest, const FCFVehicleFireResult& FireResult);

	// [v1.2.0] 서버 발사 요청과 검증 결과를 기준으로 ServerAimState를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon", meta=(ToolTip="서버 발사 요청과 검증 결과를 기준으로 ServerAimState를 갱신합니다."))
	void BuildServerAimStateFromFireRequest(const FCFVehicleFireRequest& FireRequest, ECFVehicleFireRejectReason RejectReason, bool bAccepted);

	// [v1.2.0] 발사 요청의 AimDirection이 DefaultAimProfile 조준각 안에 있는지 검사합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Weapon", meta=(ToolTip="발사 요청의 AimDirection이 DefaultAimProfile 조준각 안에 있는지 검사합니다."))
	bool IsFireRequestWithinDefaultProfile(const FCFVehicleFireRequest& FireRequest) const;

protected:
	// [v1.1.0] CameraComp 런타임 상태를 읽어 Local Aim 상태를 갱신합니다.
	void RefreshLocalAimState(float DeltaSeconds);

	// [v1.1.0] 월드 조준 방향을 차량 로컬 Yaw/Pitch 각도로 변환합니다.
	bool CalculateAimAnglesRelativeToVehicle(const FVector& AimDirection, float& OutYawDeg, float& OutPitchDeg) const;

	// [v1.1.0] 차량 로컬 Yaw/Pitch가 기본 Aim Profile 안에 있는지 검사합니다.
	bool IsAimWithinDefaultProfile(float AimYawDeg, float AimPitchDeg) const;

	// [v1.1.0] Local Aim 계산 결과를 Reticle 상태로 변환합니다.
	ECFVehicleReticleState BuildLocalReticleState(bool bWithinWeaponArc, bool bAimBlocked, bool bCanFire) const;

	// [v1.0.0] Owner Actor를 차량 Pawn으로 해석합니다.
	ACFVehiclePawn* ResolveOwnerVehiclePawn() const;

	// [v1.0.0] Owner 차량 Pawn에서 VehicleCameraComp를 해석합니다.
	UCFVehicleCameraComp* ResolveVehicleCameraComp() const;

	// [v1.3.0] 복제 Aim 시각 상태가 클라이언트에 도착했을 때 표시 캐시만 갱신합니다.
	UFUNCTION()
	void OnRep_RepAimVisualState();

private:
	// [v1.0.0] 이 AimComp를 소유한 차량 Pawn 캐시입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="Owner 차량 Pawn (OwnerVehiclePawn)", ToolTip="이 AimComp를 소유한 차량 Pawn 캐시입니다."))
	TObjectPtr<ACFVehiclePawn> OwnerVehiclePawn = nullptr;

	// [v1.0.0] Owner 차량 Pawn에서 찾은 VehicleCameraComp 캐시입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="캐시된 VehicleCameraComp (CachedVehicleCameraComp)", ToolTip="Owner 차량 Pawn에서 찾은 VehicleCameraComp 캐시입니다."))
	TObjectPtr<UCFVehicleCameraComp> CachedVehicleCameraComp = nullptr;

	// [v1.0.0] Aim 시스템의 기본 조준 가능 범위 프로필입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="기본 Aim Profile (DefaultAimProfile)", ToolTip="Aim 시스템의 기본 조준 가능 범위 프로필입니다. 후속 단계에서 무기/데이터 에셋과 연결할 수 있습니다."))
	FCFVehicleAimProfile DefaultAimProfile;

	// [v1.0.0] 로컬 클라이언트가 즉시 표시할 Aim 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="로컬 Aim 상태 (LocalAimState)", ToolTip="로컬 클라이언트가 즉시 표시할 Aim 상태입니다. 이번 단계에서는 계산 로직 없이 기본값을 유지합니다."))
	FCFVehicleLocalAimState LocalAimState;

	// [v1.0.0] 서버 검증에 사용할 Aim 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="서버 Aim 상태 (ServerAimState)", ToolTip="서버 검증에 사용할 Aim 상태입니다. 이번 단계에서는 검증 로직 없이 기본값을 유지합니다."))
	FCFVehicleServerAimState ServerAimState;

	// [v1.3.0] 다른 클라이언트에게 보여줄 Aim 시각 상태입니다.
	UPROPERTY(ReplicatedUsing=OnRep_RepAimVisualState, VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="복제 Aim 시각 상태 (RepAimVisualState)", ToolTip="다른 클라이언트에게 보여줄 Aim 시각 상태입니다. 전투 판정용 데이터가 아닙니다."))
	FCFVehicleRepAimVisualState RepAimVisualState;

	// [v1.0.0] Owner Pawn과 CameraComp 참조가 모두 준비되었는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="Aim 런타임 준비 완료 여부 (bAimRuntimeReady)", ToolTip="Owner 차량 Pawn과 VehicleCameraComp 참조가 모두 준비되었는지 여부입니다."))
	bool bAimRuntimeReady = false;

	// [v1.0.0] 마지막 Aim 런타임 초기화 또는 참조 갱신 결과 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="마지막 Aim 런타임 요약 (LastAimRuntimeSummary)", ToolTip="마지막 Aim 런타임 초기화 또는 참조 갱신 결과 요약 문자열입니다."))
	FString LastAimRuntimeSummary = TEXT("NotInitialized");
};
