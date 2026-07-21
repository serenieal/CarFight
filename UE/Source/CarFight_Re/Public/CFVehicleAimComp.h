// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.10.1
// Date: 2026-07-13
// Description: CarFight 싱글플레이 차량 Aim 시스템 기준 클래스
// Changelog:
// - v1.10.1: Weapon Aim Solution 정렬 대기 상태를 Reticle TurretAligning 상태로 분리.
// - v1.10.0: Weapon Aim Solution 저장/조회 API를 추가하고 LocalAimState 반영 기준으로 연결.
// - v1.9.0: 터렛 안정화 전 발사 정책에 맞춰 로컬 발사 가능 예측에서 조준각 초과를 단독 차단 조건에서 제외.
// - v1.8.0: ServerAimState / RepAimVisualState 계열 API를 FireValidationState / AimVisualState 명칭으로 교체.
// - v1.7.0: BP에 보이는 Aim 상태 getter 설명을 싱글플레이 로컬 플레이어/검증 상태 기준으로 정리.
// - v1.6.0: 로컬 Fire Command 전환에 맞춰 발사 요청/결과 함수 설명을 싱글플레이 의미로 정리.
// - v1.5.0: 싱글플레이 기준선에서 Aim 시각 상태의 UE 복제 등록과 OnRep 경로를 제거.
// - v1.4.0: 싱글플레이 전환에 맞춰 AimComp 기본 컴포넌트 복제를 비활성화.
// Migration:
// - BuildLocalReticleState 호출자는 정렬 대기 여부를 bWeaponIsAligning으로 전달해야 한다.
// - Pawn은 Muzzle 기준 AimOrigin/AimDirection/Target을 계산한 뒤 SetWeaponAimSolution으로 AimComp에 전달한다.
// - Camera Trace Hit은 bLocalAimTraceHasBlockingHit으로만 표시하고, 발사 차단은 Weapon Aim Solution의 MuzzleBlocked를 사용한다.
// - 조준각 내부 여부는 LocalAimState / FireValidationState 디버그 상태로 유지하되 기본 발사 거부 조건으로 쓰지 않는다.
// - GetServerAimState는 GetFireValidationState로 교체한다.
// - GetRepAimVisualState는 GetAimVisualState로 교체한다.
// - ApplyServerFireResult / BuildServerAimStateFromFireRequest / UpdateRepAimVisualFromFireResult는 FireValidation/AimVisual 명칭 함수로 교체한다.
// - 멀티플레이 Aim 시각 상태 복제가 다시 필요하면 별도 멀티플레이 브랜치에서 복제 경로를 복구한다.
// Scope: Owner Pawn과 VehicleCameraComp 참조를 안전하게 캐시하고 Tick에서 Local Aim 상태를 갱신하며 로컬 발사 검증/시각 상태를 저장합니다.

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

public:
	// [v1.0.0] Owner Pawn과 CameraComp 참조를 갱신하고 Aim 런타임 준비 상태를 계산합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Aim", meta=(ToolTip="Owner 차량 Pawn과 VehicleCameraComp 참조를 갱신하고 Aim 런타임 준비 상태를 계산합니다."))
	bool InitializeAimRuntime();

	// [v1.0.0] Aim 런타임에 필요한 Owner Pawn과 CameraComp 참조만 다시 찾습니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Aim", meta=(ToolTip="Aim 런타임에 필요한 Owner 차량 Pawn과 VehicleCameraComp 참조를 다시 찾습니다."))
	bool RefreshAimRuntimeReferences();

	// [v1.7.0] 현재 로컬 Aim 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="현재 로컬 플레이어 기준 Aim 상태를 반환합니다."))
	FCFVehicleLocalAimState GetLocalAimState() const;

	// [v1.7.0] 현재 로컬 Reticle 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="현재 로컬 플레이어 기준 Reticle 표시 상태를 반환합니다."))
	ECFVehicleReticleState GetReticleState() const;

	// [v1.8.0] 현재 발사 검증 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="현재 로컬 발사 검증 상태를 반환합니다."))
	FCFVehicleFireValidationState GetFireValidationState() const;

	// [v1.8.0] 현재 로컬 디버그/발사 결과 표시용 Aim 시각 상태를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="현재 로컬 디버그와 발사 결과 표시에 사용할 Aim 시각 상태를 반환합니다."))
	FCFVehicleAimVisualState GetAimVisualState() const;

	// [v1.10.0] 현재 무기 발사 기준 Aim Solution을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="현재 Muzzle 기준 발사 원점, 방향, Reticle 목표점, 차단 판정을 묶은 Weapon Aim Solution을 반환합니다."))
	FCFVehicleWeaponAimSolution GetWeaponAimSolution() const;

	// [v1.10.0] Pawn이 계산한 무기 발사 기준 Aim Solution을 저장합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Aim", meta=(ToolTip="Pawn이 계산한 Muzzle 기준 발사 원점, 방향, Reticle 목표점, 차단 판정을 AimComp에 저장합니다."))
	void SetWeaponAimSolution(const FCFVehicleWeaponAimSolution& InWeaponAimSolution);

	// [v1.0.0] 기본 Aim Profile을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="현재 AimComp가 보유한 기본 Aim Profile을 반환합니다."))
	FCFVehicleAimProfile GetDefaultAimProfile() const;

	// [v1.0.0] Aim 런타임 참조 준비 완료 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="AimComp가 Owner 차량 Pawn과 VehicleCameraComp를 모두 찾았는지 여부를 반환합니다."))
	bool IsAimRuntimeReady() const;

	// [v1.0.0] 마지막 Aim 런타임 초기화 요약 문자열을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|Aim", meta=(ToolTip="마지막 Aim 런타임 초기화 또는 참조 갱신 결과 요약 문자열을 반환합니다."))
	FString GetLastAimRuntimeSummary() const;

	// [v1.6.0] 현재 Local Aim 상태를 기준으로 로컬 발사 명령 데이터를 생성합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon", meta=(ToolTip="현재 Local Aim 상태를 기준으로 로컬 발사 명령 데이터를 생성합니다. 함수명은 기존 BP 호환을 위해 유지합니다."))
	FCFVehicleFireRequest BuildFireRequest(int32 FireRequestId, float ClientFireTimeSeconds) const;

	// [v1.8.0] 로컬 발사 처리 결과를 검증 상태에 반영합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon", meta=(ToolTip="로컬 발사 처리 결과를 검증 상태에 반영합니다."))
	void ApplyFireValidationResult(const FCFVehicleFireResult& FireResult);

	// [v1.8.0] 로컬 발사 결과를 로컬 디버그/발사 결과 표시용 Aim 시각 상태에 반영합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon", meta=(ToolTip="로컬 발사 결과를 로컬 디버그와 발사 결과 표시용 Aim 시각 상태에 반영합니다."))
	void UpdateAimVisualFromFireResult(const FCFVehicleFireRequest& FireRequest, const FCFVehicleFireResult& FireResult);

	// [v1.8.0] 로컬 발사 명령과 검증 결과를 기준으로 검증 상태를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|Weapon", meta=(ToolTip="로컬 발사 명령과 검증 결과를 기준으로 검증 상태를 갱신합니다."))
	void BuildFireValidationStateFromFireCommand(const FCFVehicleFireRequest& FireCommand, ECFVehicleFireRejectReason RejectReason, bool bAccepted);

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
	ECFVehicleReticleState BuildLocalReticleState(bool bWithinWeaponArc, bool bAimBlocked, bool bWeaponIsAligning, bool bCanFire) const;

	// [v1.0.0] Owner Actor를 차량 Pawn으로 해석합니다.
	ACFVehiclePawn* ResolveOwnerVehiclePawn() const;

	// [v1.0.0] Owner 차량 Pawn에서 VehicleCameraComp를 해석합니다.
	UCFVehicleCameraComp* ResolveVehicleCameraComp() const;

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

	// [v1.7.0] 로컬 플레이어가 즉시 표시할 Aim 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="로컬 Aim 상태 (LocalAimState)", ToolTip="로컬 플레이어가 즉시 표시할 Aim 상태입니다."))
	FCFVehicleLocalAimState LocalAimState;

	// [v1.8.0] 로컬 발사 검증에 사용할 Aim 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="발사 검증 상태 (FireValidationState)", ToolTip="로컬 발사 검증에 사용할 Aim 상태입니다."))
	FCFVehicleFireValidationState FireValidationState;

	// [v1.8.0] 로컬 디버그와 발사 결과 표시에 사용할 Aim 시각 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="Aim 시각 상태 (AimVisualState)", ToolTip="싱글플레이에서 로컬 디버그와 발사 결과 표시에 사용할 Aim 시각 상태입니다. 전투 판정용 데이터가 아닙니다."))
	FCFVehicleAimVisualState AimVisualState;

	// [v1.10.0] Muzzle 기준 발사 원점, 방향, Reticle 목표점, 차단 판정을 묶은 최신 조준 해석 결과입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="Weapon Aim Solution (WeaponAimSolution)", ToolTip="BuildFireCommand, HitScan, Projectile이 공통으로 사용할 최신 Muzzle 기준 조준 해석 결과입니다."))
	FCFVehicleWeaponAimSolution WeaponAimSolution;

	// [v1.0.0] Owner Pawn과 CameraComp 참조가 모두 준비되었는지 여부입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="Aim 런타임 준비 완료 여부 (bAimRuntimeReady)", ToolTip="Owner 차량 Pawn과 VehicleCameraComp 참조가 모두 준비되었는지 여부입니다."))
	bool bAimRuntimeReady = false;

	// [v1.0.0] 마지막 Aim 런타임 초기화 또는 참조 갱신 결과 요약입니다.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="CarFight|Aim", meta=(AllowPrivateAccess="true", DisplayName="마지막 Aim 런타임 요약 (LastAimRuntimeSummary)", ToolTip="마지막 Aim 런타임 초기화 또는 참조 갱신 결과 요약 문자열입니다."))
	FString LastAimRuntimeSummary = TEXT("NotInitialized");
};
