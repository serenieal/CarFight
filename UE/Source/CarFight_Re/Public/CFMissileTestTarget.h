// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.1
// Date: 2026-08-02
// Description: CF-FQ-030 Direct 미사일 PIE 전용 선택 가능 테스트 타겟
// Scope: 정지, 측면 왕복, 자동 파괴와 오버슈트 관찰용 경량 TargetSelectable Actor를 제공합니다.
// Changelog:
// - v1.0.1: 수동 PIE 준비 시간을 확보하도록 기본 자동 파괴 지연을 20초로 조정.
// - v1.0.0: 미사일 전용 테스트 타겟 Actor 최초 추가.
// Migration:
// - 게임 본편 차량·AI·스폰 흐름에서는 자동 생성하지 않으며 MissileDirectTest 맵에서만 명시적으로 배치합니다.

#pragma once

#include "CoreMinimal.h"
#include "CFTargetSelectable.h"
#include "GameFramework/Actor.h"
#include "CFMissileTestTarget.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 * 미사일 PIE 테스트 타겟의 이동 방식입니다.
 */
UENUM(BlueprintType)
enum class ECFMissileTestMoveMode : uint8
{
	Stationary UMETA(DisplayName="정지 (Stationary)"),
	LateralPingPong UMETA(DisplayName="측면 왕복 (Lateral Ping Pong)")
};

/**
 * Direct TargetActor Guidance를 반복 검증하기 위한 경량 선택 가능 타겟입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API ACFMissileTestTarget : public AActor, public ICFTargetSelectable
{
	GENERATED_BODY()

public:
	// [v1.0.0] 선택·충돌·시각 컴포넌트와 안전한 테스트 기본값을 초기화합니다.
	ACFMissileTestTarget();

	// [v1.0.0] 에디터 프로퍼티가 바뀐 뒤 충돌 응답과 초기 위치를 다시 적용합니다.
	virtual void OnConstruction(const FTransform& Transform) override;

	// [v1.0.0] 측면 왕복 이동과 현재 목표 Velocity를 갱신합니다.
	virtual void Tick(float DeltaSeconds) override;

	// [v1.0.0] Guidance가 사용할 현재 테스트 타겟 Velocity를 반환합니다.
	virtual FVector GetVelocity() const override;

	// [v1.0.0] 테스트 타겟의 선택 가능 상태를 반환합니다.
	virtual bool IsTargetSelectable_Implementation(const FCFTargetSelectionContext& SelectionContext) const override;

	// [v1.0.0] HUD와 TargetSelect가 사용할 테스트 타겟 표시 정보를 반환합니다.
	virtual FCFTargetDisplayInfo GetTargetDisplayInfo_Implementation() const override;

	// [v1.0.0] 타겟 중심의 월드 위치를 선택 대표 위치로 반환합니다.
	virtual FVector GetTargetSelectionLocation_Implementation() const override;

	// [v1.0.0] 파괴 전 테스트 타겟의 기본 추적 상태를 Visible로 반환합니다.
	virtual ECFTargetTrackState GetTargetTrackState_Implementation() const override;

	// [v1.0.0] 자동 파괴 대기 없이 현재 테스트 타겟을 즉시 제거합니다.
	UFUNCTION(BlueprintCallable, CallInEditor, Category="CarFight|MissileTest", meta=(DisplayName="테스트 타겟 즉시 파괴 (Destroy Test Target Now)", ToolTip="목표 파괴 뒤 이미 발사된 미사일의 TargetLost·ContinueStraight 동작을 검증하기 위해 이 Actor를 즉시 Destroy합니다."))
	void DestroyTestTargetNow();

	// [v1.0.0] 현재 위치를 새 이동 중심으로 저장하고 왕복 시간과 Velocity를 초기화합니다.
	UFUNCTION(BlueprintCallable, CallInEditor, Category="CarFight|MissileTest", meta=(DisplayName="테스트 이동 초기화 (Reset Test Motion)", ToolTip="현재 Actor 위치를 측면 왕복 이동의 새 중심으로 저장하고 경과 시간과 현재 Velocity를 0으로 초기화합니다."))
	void ResetTestMotion();

	// [v1.0.0] 에디터·Python 생성 뒤 현재 프로퍼티 기준으로 충돌과 이동 상태를 다시 적용합니다.
	UFUNCTION(BlueprintCallable, CallInEditor, Category="CarFight|MissileTest", meta=(DisplayName="테스트 타겟 설정 갱신 (Refresh Test Target Config)", ToolTip="Projectile 차단 여부와 현재 이동 중심을 다시 적용합니다. 전용 테스트 맵 생성 스크립트가 배치 직후 호출합니다."))
	void RefreshTestTargetConfig();

	// [v1.0.0] Automation에서 월드 Tick 없이 목표 Velocity 추정값을 지정합니다.
	void SetTestVelocityForAutomation(const FVector& InTargetVelocity);

	// [v1.0.0] 테스트와 로그에서 이 타겟을 식별할 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileTest|Identity", meta=(DisplayName="테스트 타겟 ID (TargetId)", ToolTip="TargetSelect HUD, 미사일 검증 로그와 테스트 절차에서 이 타겟을 구분할 안정적인 이름입니다."))
	FName TargetId = TEXT("MissileTestTarget");

	// [v1.0.0] HUD에 표시할 사용자 친화적인 테스트 타겟 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileTest|Identity", meta=(DisplayName="테스트 타겟 표시 이름 (TargetDisplayName)", ToolTip="비어 있으면 Actor Label 또는 Actor 이름을 사용합니다."))
	FText TargetDisplayName;

	// [v1.0.0] 정지 또는 측면 왕복 중 사용할 테스트 이동 방식입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileTest|Motion", meta=(DisplayName="테스트 이동 방식 (MovementMode)", ToolTip="Stationary는 현재 위치를 유지하고 Lateral Ping Pong은 지정 방향과 진폭 안에서 왕복 이동합니다."))
	ECFMissileTestMoveMode MovementMode = ECFMissileTestMoveMode::Stationary;

	// [v1.0.0] 측면 왕복 이동에 사용할 월드 방향입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileTest|Motion", meta=(EditCondition="MovementMode == ECFMissileTestMoveMode::LateralPingPong", DisplayName="측면 이동 방향 (LateralMoveDirection)", ToolTip="측면 왕복 이동에 사용할 월드 방향입니다. 0 벡터이면 월드 Y축을 사용합니다."))
	FVector LateralMoveDirection = FVector::RightVector;

	// [v1.0.0] 이동 중심에서 한쪽 끝까지의 측면 왕복 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileTest|Motion", meta=(ClampMin="0.0", Units="cm", EditCondition="MovementMode == ECFMissileTestMoveMode::LateralPingPong", DisplayName="측면 이동 진폭 cm (LateralAmplitudeCm)", ToolTip="이동 중심에서 양쪽 끝까지 이동할 최대 거리입니다."))
	float LateralAmplitudeCm = 2500.0f;

	// [v1.0.0] 왕복 중심 통과 시 사용할 최대 측면 속력입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileTest|Motion", meta=(ClampMin="0.0", Units="cm/s", EditCondition="MovementMode == ECFMissileTestMoveMode::LateralPingPong", DisplayName="측면 이동 속도 cm/s (LateralSpeedCmPerSec)", ToolTip="사인 왕복 이동이 중심을 통과할 때의 최대 속력입니다. Guidance TargetVelocityEstimate 검증에도 사용됩니다."))
	float LateralSpeedCmPerSec = 1200.0f;

	// [v1.0.0] PIE 시작 뒤 지정 시간에 이 타겟을 자동 파괴할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileTest|Lifetime", meta=(DisplayName="자동 파괴 사용 (bAutoDestroy)", ToolTip="True이면 AutoDestroyDelaySeconds 뒤 Actor를 Destroy해 이미 발사된 미사일의 목표 상실 수명을 검증합니다."))
	bool bAutoDestroy = false;

	// [v1.0.0] PIE 시작 후 자동 파괴까지 기다릴 시간입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileTest|Lifetime", meta=(ClampMin="0.0", Units="s", EditCondition="bAutoDestroy", DisplayName="자동 파괴 지연 초 (AutoDestroyDelaySeconds)", ToolTip="PIE 시작 후 이 시간이 지나면 테스트 타겟을 자동으로 Destroy합니다."))
		float AutoDestroyDelaySeconds = 20.0f;

	// [v1.0.0] 실제 Projectile 충돌을 차단할지 여부입니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="CarFight|MissileTest|Collision", meta=(DisplayName="Projectile 적중 차단 (bBlockProjectileHits)", ToolTip="True이면 미사일이 이 타겟에 실제 충돌할 수 있습니다. 오버슈트 관찰용 타겟은 False로 두어 통과 후 Overshoot 상태를 확인합니다."))
	bool bBlockProjectileHits = true;

protected:
	// [v1.0.0] PIE 시작 위치와 자동 파괴 타이머를 준비합니다.
	virtual void BeginPlay() override;

private:
	// [v1.0.0] Visibility, WeaponHit와 Projectile 채널 응답을 현재 설정에 맞게 적용합니다.
	void ApplyTestCollisionResponses();

	// [v1.0.0] 자동 파괴 타이머가 만료됐을 때 이 테스트 타겟을 제거합니다.
	void HandleAutoDestroyTimer();

	// [v1.0.0] TargetSelect Trace와 Projectile 적중에 사용할 구형 충돌 루트입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileTest|Components", meta=(AllowPrivateAccess="true", DisplayName="테스트 타겟 충돌 (CollisionComponent)", ToolTip="Visibility·WeaponHit Trace와 선택적 Projectile 적중을 받는 구형 충돌 루트입니다."))
	TObjectPtr<USphereComponent> CollisionComponent = nullptr;

	// [v1.0.0] 테스트 맵에서 타겟 위치와 이동을 쉽게 식별할 기본 큐브 메시입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|MissileTest|Components", meta=(AllowPrivateAccess="true", DisplayName="테스트 타겟 메시 (TargetMeshComponent)", ToolTip="Engine 기본 Cube를 사용해 테스트 타겟을 시각적으로 표시하며 충돌 판정은 CollisionComponent가 담당합니다."))
	TObjectPtr<UStaticMeshComponent> TargetMeshComponent = nullptr;

	// [v1.0.0] 측면 왕복 이동의 월드 중심 위치입니다.
	FVector MotionCenterLocation = FVector::ZeroVector;

	// [v1.0.0] 현재 측면 왕복 이동의 누적 시간입니다.
	float MotionElapsedSeconds = 0.0f;

	// [v1.0.0] Guidance가 TargetVelocityEstimate로 읽을 현재 월드 Velocity입니다.
	FVector CurrentTargetVelocity = FVector::ZeroVector;

	// [v1.0.0] 자동 파괴 호출을 관리할 타이머 핸들입니다.
	FTimerHandle AutoDestroyTimerHandle;
};
