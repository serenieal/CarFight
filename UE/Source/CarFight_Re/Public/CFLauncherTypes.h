// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.3.0
// Date: 2026-08-02
// Description: CarFight 모듈형 런처 발사 패턴 공용 타입
// Scope: SingleCycle·Ripple·Salvo 발사 패턴, Volley 명령 목표 Snapshot, Direct·Angled·Vertical Release 설정, 차량 속도 상속과 결정적 시퀀스 상태를 제공합니다.
// Changelog:
// - v1.3.0: 한 Volley의 CommandTargetLocation과 GuidanceTargetActor를 같은 첫 발사 순간에 보존하는 FCFLauncherCommandTargetSnapshot을 추가.
// - v1.2.0: Direct·AngledEjection·VerticalEjection 방향·속도·차량 속도 상속·안전 검사 거리 설정을 추가.
// - v1.1.0: 발사 시퀀스 상태·취소 사유와 결정적 Dispatch·결과 기록 Runtime을 추가.
// - v1.0.0: 발사 패턴, 실패 정책, 쿨다운 시작 정책과 FCFLauncherFirePatternConfig 최초 추가.
// Migration:
// - GuidanceTargetActor는 약한 참조로 보존하므로 목표 Actor가 파괴되면 후속 발사는 None을 전달하고 미사일의 목표 소실 정책을 사용합니다.
// - 기존 WeaponData는 LauncherReleaseConfig 기본값 Direct / CarrierVelocityRatio 0을 사용해 기존 발사 방향·속도를 유지합니다.
// - AngledEjection은 Muzzle Transform 기준 LocalEjectionDirection을 사용하고 VerticalEjection은 Muzzle X축을 사용합니다.
// - 기존 WeaponData는 LauncherFirePatternConfig 기본값 SingleCycle / 1발을 사용하므로 시퀀스 Runtime이 즉시 Completed가 되고 기존 단발 발사 결과를 유지합니다.
// - Ripple은 프레임당 최대 1발을 Dispatch하며, Salvo는 첫 발을 포함한 MaximumSimultaneousLaunchCount를 첫 처리 묶음 한도로 사용합니다.
// - 잘못된 음수·0 입력은 유효값 Getter에서 안전하게 보정하며 원본 DataAsset 값은 자동 재저장하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFProjectileLaunchTypes.h"
#include "CFLauncherTypes.generated.h"

/**
 * 한 번의 플레이어 발사 입력을 런처가 여러 Projectile로 해석하는 방식입니다.
 */
UENUM(BlueprintType)
enum class ECFLauncherFirePattern : uint8
{
	SingleCycle UMETA(DisplayName="단발 순환 (Single Cycle)"),
	Ripple UMETA(DisplayName="순차 연속 발사 (Ripple)"),
	Salvo UMETA(DisplayName="동시 일제 발사 (Salvo)")
};

/**
 * Ripple 또는 Salvo 도중 한 Projectile 발사가 실패했을 때 남은 시퀀스를 처리하는 방식입니다.
 */
UENUM(BlueprintType)
enum class ECFLauncherSequenceFailurePolicy : uint8
{
	ContinueRemaining UMETA(DisplayName="남은 발사 계속 (Continue Remaining)"),
	StopSequence UMETA(DisplayName="시퀀스 중단 (Stop Sequence)")
};

/**
 * 여러 Projectile로 구성된 발사 시퀀스에서 무기 쿨다운을 시작하는 시점입니다.
 */
UENUM(BlueprintType)
enum class ECFLauncherCooldownStartPolicy : uint8
{
	FirstAcceptedProjectile UMETA(DisplayName="첫 승인 발사 시점 (First Accepted Projectile)"),
	SequenceCompleted UMETA(DisplayName="시퀀스 완료 시점 (Sequence Completed)")
};

/**
 * 런처 발사 시퀀스의 현재 실행 상태입니다.
 */
UENUM(BlueprintType)
enum class ECFLauncherSequenceState : uint8
{
	Idle UMETA(DisplayName="대기 (Idle)"),
	Active UMETA(DisplayName="발사 진행 중 (Active)"),
	Completed UMETA(DisplayName="발사 완료 (Completed)"),
	Cancelled UMETA(DisplayName="발사 취소 (Cancelled)")
};

/**
 * 진행 중인 발사 시퀀스가 종료되기 전에 취소된 원인입니다.
 */
UENUM(BlueprintType)
enum class ECFLauncherSequenceCancelReason : uint8
{
	None UMETA(DisplayName="없음 (None)"),
	Manual UMETA(DisplayName="수동 취소 (Manual)"),
	OwnerInvalid UMETA(DisplayName="소유 차량 무효 (Owner Invalid)"),
	OwnerDestroyed UMETA(DisplayName="소유 차량 파괴 (Owner Destroyed)"),
	WeaponChanged UMETA(DisplayName="무기 변경 (Weapon Changed)"),
	TurretMountChanged UMETA(DisplayName="터렛 마운트 변경 (Turret Mount Changed)"),
	ShotFailed UMETA(DisplayName="발사 실패 정책 중단 (Shot Failed)"),
	RuntimeUnavailable UMETA(DisplayName="런타임 실행 불가 (Runtime Unavailable)")
};

/**
 * 한 Volley가 첫 발사 순간 함께 고정하는 명령 목표 위치와 유도 목표 Actor Snapshot입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFLauncherCommandTargetSnapshot
{
	GENERATED_BODY()

	// [v1.3.0] 첫 발사 순간 고정한 Volley 공통 Command Target 월드 위치입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|CommandTarget", meta=(DisplayName="명령 목표 위치 (CommandTargetLocation)", ToolTip="Ripple·Salvo의 첫 발과 모든 후속 발사가 공통으로 사용하는 첫 발사 순간 Command Target 월드 위치입니다."))
	FVector CommandTargetLocation = FVector::ZeroVector;

	// [v1.3.0] 첫 발사 순간 선택되어 같은 Volley의 모든 미사일이 공유할 약한 유도 목표 Actor 참조입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|CommandTarget", meta=(DisplayName="유도 목표 Actor (GuidanceTargetActor)", ToolTip="Ripple·Salvo 첫 발사 순간 선택된 유도 목표 Actor입니다. 이후 차량 선택 대상이 바뀌어도 같은 Volley는 이 Actor를 유지하며, Actor가 파괴되면 None이 됩니다."))
	TWeakObjectPtr<AActor> GuidanceTargetActor;

	// [v1.3.0] 첫 발사 순간의 명령 목표 위치와 유도 목표 Actor를 하나의 Snapshot으로 함께 저장합니다.
	void Capture(const FVector& InCommandTargetLocation, AActor* InGuidanceTargetActor)
	{
		CommandTargetLocation = InCommandTargetLocation;
		GuidanceTargetActor = InGuidanceTargetActor;
	}

	// [v1.3.0] 이전 Volley의 명령 목표 위치와 유도 목표 Actor 참조를 모두 초기화합니다.
	void Reset()
	{
		CommandTargetLocation = FVector::ZeroVector;
		GuidanceTargetActor.Reset();
	}

	// [v1.3.0] 저장된 명령 목표 위치가 후속 발사에 사용할 수 있는 값인지 반환합니다.
	bool HasValidCommandTargetLocation() const
	{
		return !CommandTargetLocation.ContainsNaN();
	}

	// [v1.3.0] 아직 유효한 첫 발사 순간 유도 목표 Actor를 반환합니다.
	AActor* GetGuidanceTargetActor() const
	{
		return GuidanceTargetActor.Get();
	}
};

/**
 * WeaponData가 소유하고 런처 시퀀스 스케줄러가 읽을 발사 패턴 설정입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFLauncherFirePatternConfig
{
	GENERATED_BODY()

	// [v1.0.0] 플레이어 발사 입력 한 번을 단발·순차·동시 발사 중 어떤 방식으로 처리할지 결정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|FirePattern", meta=(DisplayName="런처 발사 패턴 (FirePattern)", ToolTip="SingleCycle은 입력당 1발, Ripple은 지정 간격 순차 발사, Salvo는 허용된 동시 처리 한도까지 한 번에 발사합니다."))
	ECFLauncherFirePattern FirePattern = ECFLauncherFirePattern::SingleCycle;

	// [v1.0.0] Ripple 또는 Salvo 입력 한 번에 요청할 Projectile 총수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|FirePattern", meta=(ClampMin="1", ClampMax="32", UIMin="1", UIMax="16", DisplayName="입력당 발사체 수 (ProjectileCountPerTrigger)", ToolTip="Ripple 또는 Salvo 입력 한 번에 요청할 Projectile 총수입니다. SingleCycle은 이 값과 무관하게 항상 1발입니다."))
	int32 ProjectileCountPerTrigger = 1;

	// [v1.0.0] Ripple에서 인접 Projectile 발사 요청 사이에 둘 시간 간격입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|FirePattern", meta=(ClampMin="0.0", ClampMax="10.0", UIMin="0.0", UIMax="1.0", Units="s", DisplayName="Ripple 발사 간격 초 (InterMuzzleDelaySeconds)", ToolTip="Ripple 패턴에서 다음 Muzzle 발사 요청까지 기다릴 시간입니다. SingleCycle과 Salvo에서는 0으로 해석합니다."))
	float InterMuzzleDelaySeconds = 0.10f;

	// [v1.0.0] Salvo가 같은 스케줄러 단계에서 동시에 처리할 수 있는 최대 Projectile 수입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|FirePattern", meta=(ClampMin="1", ClampMax="32", UIMin="1", UIMax="16", DisplayName="Salvo 최대 동시 발사 수 (MaximumSimultaneousLaunchCount)", ToolTip="Salvo에서 같은 스케줄러 단계에 동시에 요청할 Projectile 최대 수입니다. 실제 값은 입력당 발사체 수를 초과하지 않습니다."))
	int32 MaximumSimultaneousLaunchCount = 1;

	// [v1.0.0] 시퀀스 중 한 Projectile 발사가 거부되거나 Pool 확보에 실패했을 때의 처리 정책입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|FirePattern", meta=(DisplayName="발사 실패 처리 정책 (SequenceFailurePolicy)", ToolTip="ContinueRemaining은 실패한 발만 건너뛰고 남은 발사를 계속하며, StopSequence는 첫 실패에서 현재 시퀀스를 취소합니다."))
	ECFLauncherSequenceFailurePolicy SequenceFailurePolicy = ECFLauncherSequenceFailurePolicy::ContinueRemaining;

	// [v1.0.0] 여러 Projectile 발사 시퀀스에서 기존 WeaponData 발사 간격 쿨다운을 시작할 시점입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|FirePattern", meta=(DisplayName="쿨다운 시작 정책 (CooldownStartPolicy)", ToolTip="FirstAcceptedProjectile은 첫 성공 발사에서 쿨다운을 시작하고, SequenceCompleted는 마지막 발사 처리 뒤 시작합니다."))
	ECFLauncherCooldownStartPolicy CooldownStartPolicy = ECFLauncherCooldownStartPolicy::FirstAcceptedProjectile;

	// [v1.0.0] 현재 패턴에 맞게 1~32 범위로 보정된 입력당 실제 발사체 수를 반환합니다.
	int32 GetEffectiveProjectileCount() const
	{
		return FirePattern == ECFLauncherFirePattern::SingleCycle
			? 1
			: FMath::Clamp(ProjectileCountPerTrigger, 1, 32);
	}

	// [v1.0.0] Ripple일 때만 음수를 제거한 실제 발사 간격을 반환하고 나머지 패턴은 0을 반환합니다.
	float GetEffectiveInterMuzzleDelaySeconds() const
	{
		return FirePattern == ECFLauncherFirePattern::Ripple
			? FMath::Clamp(InterMuzzleDelaySeconds, 0.0f, 10.0f)
			: 0.0f;
	}

			// [v1.0.0] Salvo일 때만 실제 발사 수 이내로 보정된 동시 처리 한도를 반환하고 나머지 패턴은 1을 반환합니다.
	int32 GetEffectiveMaximumSimultaneousLaunchCount() const
	{
		return FirePattern == ECFLauncherFirePattern::Salvo
			? FMath::Clamp(MaximumSimultaneousLaunchCount, 1, GetEffectiveProjectileCount())
			: 1;
	}
};

/**
 * WeaponData가 소유하고 Launch Context 생성기가 읽을 Projectile 분리 방식과 초기 사출 설정입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFLauncherReleaseConfig
{
	GENERATED_BODY()

	// [v1.2.0] Projectile이 Muzzle에서 분리되는 방식을 결정합니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Release", meta=(DisplayName="발사 분리 모드 (ReleaseMode)", ToolTip="Direct는 기존 조준 방향 직사, AngledEjection은 Muzzle 로컬 사출 방향, VerticalEjection은 Muzzle X축 방향을 사용합니다."))
	ECFProjectileReleaseMode ReleaseMode = ECFProjectileReleaseMode::Direct;

	// [v1.2.0] AngledEjection에서 Muzzle Transform 기준으로 사용할 로컬 사출 방향입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Release", meta=(DisplayName="로컬 사출 방향 (LocalEjectionDirection)", ToolTip="AngledEjection에서 Muzzle Transform으로 월드 방향으로 변환할 로컬 벡터입니다. 0 또는 NaN이면 로컬 +X로 복구합니다."))
	FVector LocalEjectionDirection = FVector(1.0f, 0.0f, 1.0f);

	// [v1.2.0] AngledEjection 또는 VerticalEjection의 런처 분리 속력입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Release", meta=(ClampMin="0.0", ClampMax="100000.0", UIMin="0.0", UIMax="20000.0", Units="cm/s", DisplayName="사출 속력 (EjectionSpeed)", ToolTip="AngledEjection 또는 VerticalEjection 초기 속력입니다. 0이면 ProjectileData.InitialSpeed를 안전한 fallback으로 사용합니다. Direct는 항상 ProjectileData.InitialSpeed를 사용합니다."))
	float EjectionSpeed = 0.0f;

	// [v1.2.0] 발사 차량의 현재 월드 Velocity를 초기 Projectile Velocity에 더할 비율입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Release", meta=(ClampMin="0.0", ClampMax="2.0", UIMin="0.0", UIMax="1.0", DisplayName="차량 속도 상속 비율 (CarrierVelocityRatio)", ToolTip="0이면 차량 속도를 상속하지 않고, 1이면 발사 순간 차량 월드 Velocity를 그대로 더합니다. 기존 Direct 무기 기본값은 0입니다."))
	float CarrierVelocityRatio = 0.0f;

	// [v1.2.0] 비Direct 사출 방향 앞쪽에서 즉시 막힘을 검사할 거리입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Release", meta=(ClampMin="0.0", ClampMax="10000.0", UIMin="0.0", UIMax="1000.0", Units="cm", DisplayName="런처 사출 안전 검사 거리 (LauncherClearanceTraceDistanceCm)", ToolTip="AngledEjection과 VerticalEjection이 실제 InitialLaunchDirection 앞쪽의 비피해 장애물을 검사할 거리입니다. 0이면 추가 사출 안전 검사를 끕니다."))
	float LauncherClearanceTraceDistanceCm = 150.0f;

	// [v1.2.0] AngledEjection에 사용할 안전한 정규화 로컬 사출 방향을 반환합니다.
	FVector GetEffectiveLocalEjectionDirection() const
	{
		FVector SafeDirection = LocalEjectionDirection.GetSafeNormal();
		if (SafeDirection.ContainsNaN() || SafeDirection.IsNearlyZero())
		{
			SafeDirection = FVector::ForwardVector;
		}
		return SafeDirection;
	}

	// [v1.2.0] Direct는 ProjectileData.InitialSpeed를 유지하고 Ejection은 설정값 또는 동일 fallback을 사용합니다.
	float GetEffectiveReleaseSpeed(const float FallbackInitialSpeed) const
	{
		const float SafeFallbackInitialSpeed = FMath::Max(FallbackInitialSpeed, 1.0f);
		if (ReleaseMode == ECFProjectileReleaseMode::Direct)
		{
			return SafeFallbackInitialSpeed;
		}

		return FMath::IsFinite(EjectionSpeed) && EjectionSpeed > KINDA_SMALL_NUMBER
			? EjectionSpeed
			: SafeFallbackInitialSpeed;
	}

	// [v1.2.0] 음수·비정상 입력을 제거한 차량 속도 상속 비율을 반환합니다.
	float GetEffectiveCarrierVelocityRatio() const
	{
		return FMath::IsFinite(CarrierVelocityRatio)
			? FMath::Clamp(CarrierVelocityRatio, 0.0f, 2.0f)
			: 0.0f;
	}

	// [v1.2.0] 비정상 입력을 제거한 실제 사출 안전 검사 거리를 반환합니다.
	float GetEffectiveLauncherClearanceTraceDistanceCm() const
	{
		return FMath::IsFinite(LauncherClearanceTraceDistanceCm)
			? FMath::Clamp(LauncherClearanceTraceDistanceCm, 0.0f, 10000.0f)
			: 0.0f;
	}

	// [v1.2.0] 현재 Release Mode에 맞는 정규화 월드 초기 발사 방향을 계산합니다.
	FVector ResolveInitialLaunchDirection(
		const FTransform& MuzzleWorldTransform,
		const FVector& DirectAimDirection) const
	{
		FVector ResolvedDirection = FVector::ZeroVector;
		switch (ReleaseMode)
		{
		case ECFProjectileReleaseMode::AngledEjection:
			ResolvedDirection = MuzzleWorldTransform.TransformVectorNoScale(GetEffectiveLocalEjectionDirection()).GetSafeNormal();
			break;

		case ECFProjectileReleaseMode::VerticalEjection:
			ResolvedDirection = MuzzleWorldTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
			break;

		case ECFProjectileReleaseMode::Direct:
		default:
			ResolvedDirection = DirectAimDirection.GetSafeNormal();
			break;
		}

		if (ResolvedDirection.ContainsNaN() || ResolvedDirection.IsNearlyZero())
		{
			ResolvedDirection = DirectAimDirection.GetSafeNormal();
		}
		if (ResolvedDirection.ContainsNaN() || ResolvedDirection.IsNearlyZero())
		{
			ResolvedDirection = MuzzleWorldTransform.GetUnitAxis(EAxis::X).GetSafeNormal();
		}
		if (ResolvedDirection.ContainsNaN() || ResolvedDirection.IsNearlyZero())
		{
			ResolvedDirection = FVector::ForwardVector;
		}
		return ResolvedDirection;
	}

	// [v1.2.0] 발사 플랫폼 Velocity에서 실제로 상속할 월드 성분을 계산합니다.
	FVector ResolveInheritedCarrierVelocity(const FVector& CarrierWorldVelocity) const
	{
		if (CarrierWorldVelocity.ContainsNaN())
		{
			return FVector::ZeroVector;
		}
		return CarrierWorldVelocity * GetEffectiveCarrierVelocityRatio();
	}

	// [v1.2.0] 사출 Velocity와 차량 속도 상속을 합친 초기 월드 Velocity를 계산합니다.
	FVector ResolveInitialLaunchVelocity(
		const FVector& InitialLaunchDirection,
		const float FallbackInitialSpeed,
		const FVector& CarrierWorldVelocity) const
	{
		FVector SafeInitialLaunchDirection = InitialLaunchDirection.GetSafeNormal();
		if (SafeInitialLaunchDirection.ContainsNaN() || SafeInitialLaunchDirection.IsNearlyZero())
		{
			SafeInitialLaunchDirection = FVector::ForwardVector;
		}

		return SafeInitialLaunchDirection * GetEffectiveReleaseSpeed(FallbackInitialSpeed)
			+ ResolveInheritedCarrierVelocity(CarrierWorldVelocity);
	}
};

/**
 * 첫 승인 발사 이후 남은 Ripple·Salvo 발사를 결정적으로 예약하고 결과를 집계하는 순수 런타임 상태입니다.
 */
USTRUCT(BlueprintType)
struct CARFIGHT_RE_API FCFLauncherSequenceRuntime
{
	GENERATED_BODY()

	// [v1.1.0] 현재 시퀀스 상태입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Sequence", meta=(DisplayName="런처 시퀀스 상태 (State)", ToolTip="Idle, Active, Completed 또는 Cancelled 중 현재 런처 발사 시퀀스 상태입니다."))
	ECFLauncherSequenceState State = ECFLauncherSequenceState::Idle;

	// [v1.1.0] Cancelled 상태가 된 원인입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Sequence", meta=(DisplayName="런처 시퀀스 취소 사유 (CancelReason)", ToolTip="시퀀스가 완료 전에 취소된 원인입니다. 취소되지 않았으면 None입니다."))
	ECFLauncherSequenceCancelReason CancelReason = ECFLauncherSequenceCancelReason::None;

	// [v1.1.0] 이번 시퀀스가 사용하는 안전 보정된 발사 패턴 설정입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Sequence", meta=(DisplayName="활성 발사 패턴 설정 (ActiveConfig)", ToolTip="이번 시퀀스 시작 시 복사하고 안전하게 보정한 발사 패턴 설정입니다."))
	FCFLauncherFirePatternConfig ActiveConfig;

	// [v1.1.0] 이번 발사 묶음을 식별하는 증가형 ID입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Sequence", meta=(DisplayName="발사 묶음 ID (VolleyId)", ToolTip="한 번의 플레이어 발사 입력에서 생성된 발사 묶음을 식별합니다."))
	int32 VolleyId = 0;

	// [v1.1.0] 이번 시퀀스가 처리해야 하는 전체 Projectile 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Sequence", meta=(DisplayName="전체 발사체 수 (TotalProjectileCount)", ToolTip="SingleCycle, Ripple 또는 Salvo 시퀀스가 처리해야 하는 전체 Projectile 수입니다."))
	int32 TotalProjectileCount = 0;

	// [v1.1.0] 실제 실행을 시도한 Projectile 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Sequence", meta=(DisplayName="시도한 발사체 수 (AttemptedProjectileCount)", ToolTip="첫 승인 발사를 포함해 실제 발사 실행을 시도한 Projectile 수입니다."))
	int32 AttemptedProjectileCount = 0;

	// [v1.1.0] 발사 검증과 실행이 승인된 Projectile 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Sequence", meta=(DisplayName="승인된 발사체 수 (AcceptedProjectileCount)", ToolTip="발사 검증과 실행이 승인된 Projectile 수입니다."))
	int32 AcceptedProjectileCount = 0;

	// [v1.1.0] 검증 또는 실행에 실패한 Projectile 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Sequence", meta=(DisplayName="실패한 발사체 수 (FailedProjectileCount)", ToolTip="검증 또는 실행 실패로 승인되지 않은 Projectile 수입니다."))
	int32 FailedProjectileCount = 0;

	// [v1.1.0] 아직 Dispatch하지 않은 Projectile 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Sequence", meta=(DisplayName="남은 발사체 수 (RemainingProjectileCount)", ToolTip="아직 발사 실행 요청을 Dispatch하지 않은 Projectile 수입니다."))
	int32 RemainingProjectileCount = 0;

	// [v1.1.0] Dispatch했지만 아직 결과를 기록하지 않은 Projectile 수입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Sequence", meta=(DisplayName="결과 대기 발사체 수 (PendingResultCount)", ToolTip="Dispatch 이후 RecordShotResult가 아직 호출되지 않은 Projectile 수입니다."))
	int32 PendingResultCount = 0;

	// [v1.1.0] Ripple 다음 발사까지 남은 시간입니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Sequence", meta=(DisplayName="다음 발사까지 남은 시간 (TimeUntilNextDispatchSeconds)", ToolTip="Ripple 패턴에서 다음 Projectile Dispatch까지 남은 초입니다. 다른 패턴은 0입니다."))
	float TimeUntilNextDispatchSeconds = 0.0f;

	// [v1.1.0] 첫 승인 발사가 Salvo 첫 처리 묶음 한도에 이미 포함됐는지 추적합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="CarFight|Launcher|Sequence", meta=(DisplayName="초기 Salvo 묶음 대기 (bInitialSalvoBatchPending)", ToolTip="True이면 첫 승인 발사를 MaximumSimultaneousLaunchCount에 포함해 첫 추가 Dispatch 예산을 계산합니다."))
	bool bInitialSalvoBatchPending = false;

	// [v1.1.0] 첫 승인 발사를 포함한 새 시퀀스를 시작합니다.
	void Start(const FCFLauncherFirePatternConfig& InConfig, const int32 InVolleyId)
	{
		Reset();
		ActiveConfig = InConfig;
		ActiveConfig.ProjectileCountPerTrigger = ActiveConfig.GetEffectiveProjectileCount();
		ActiveConfig.InterMuzzleDelaySeconds = ActiveConfig.GetEffectiveInterMuzzleDelaySeconds();
		ActiveConfig.MaximumSimultaneousLaunchCount = ActiveConfig.GetEffectiveMaximumSimultaneousLaunchCount();
		VolleyId = FMath::Max(InVolleyId, 1);
		TotalProjectileCount = ActiveConfig.GetEffectiveProjectileCount();
		AttemptedProjectileCount = 1;
		AcceptedProjectileCount = 1;
		RemainingProjectileCount = FMath::Max(TotalProjectileCount - 1, 0);
		TimeUntilNextDispatchSeconds = ActiveConfig.GetEffectiveInterMuzzleDelaySeconds();
		bInitialSalvoBatchPending = ActiveConfig.FirePattern == ECFLauncherFirePattern::Salvo;
		State = RemainingProjectileCount > 0
			? ECFLauncherSequenceState::Active
			: ECFLauncherSequenceState::Completed;
	}

	// [v1.1.0] 경과 시간을 반영하고 이번 호출에서 Dispatch 가능한 Projectile 수를 반환합니다.
	int32 GetDispatchBudget(const float DeltaSeconds)
	{
		if (State != ECFLauncherSequenceState::Active || RemainingProjectileCount <= 0)
		{
			return 0;
		}

		if (ActiveConfig.FirePattern == ECFLauncherFirePattern::Ripple)
		{
			TimeUntilNextDispatchSeconds -= FMath::Max(DeltaSeconds, 0.0f);
			if (TimeUntilNextDispatchSeconds > KINDA_SMALL_NUMBER)
			{
				return 0;
			}

			TimeUntilNextDispatchSeconds = ActiveConfig.GetEffectiveInterMuzzleDelaySeconds();
			return 1;
		}

		if (ActiveConfig.FirePattern == ECFLauncherFirePattern::Salvo)
		{
			int32 DispatchCapacity = ActiveConfig.GetEffectiveMaximumSimultaneousLaunchCount();
			if (bInitialSalvoBatchPending)
			{
				DispatchCapacity = FMath::Max(DispatchCapacity - 1, 0);
				bInitialSalvoBatchPending = false;
			}
			return FMath::Min(RemainingProjectileCount, DispatchCapacity);
		}

		return 0;
	}

	// [v1.1.0] Dispatch 예산에서 Projectile 한 발을 실제 실행 시도 상태로 이동합니다.
	bool MarkShotDispatched()
	{
		if (State != ECFLauncherSequenceState::Active || RemainingProjectileCount <= 0)
		{
			return false;
		}

		++AttemptedProjectileCount;
		--RemainingProjectileCount;
		++PendingResultCount;
		return true;
	}

	// [v1.1.0] 마지막 Dispatch의 승인 또는 실패 결과를 기록하고 완료·중단 상태를 판정합니다.
	void RecordShotResult(const bool bAccepted)
	{
		if (PendingResultCount <= 0 || State != ECFLauncherSequenceState::Active)
		{
			return;
		}

		--PendingResultCount;
		if (bAccepted)
		{
			++AcceptedProjectileCount;
		}
		else
		{
			++FailedProjectileCount;
			if (ActiveConfig.SequenceFailurePolicy == ECFLauncherSequenceFailurePolicy::StopSequence)
			{
				State = ECFLauncherSequenceState::Cancelled;
				CancelReason = ECFLauncherSequenceCancelReason::ShotFailed;
				return;
			}
		}

		if (RemainingProjectileCount <= 0 && PendingResultCount <= 0)
		{
			State = ECFLauncherSequenceState::Completed;
		}
	}

	// [v1.1.0] 진행 중인 시퀀스를 지정한 사유로 취소합니다.
	void Cancel(const ECFLauncherSequenceCancelReason InCancelReason)
	{
		if (State != ECFLauncherSequenceState::Active)
		{
			return;
		}
		State = ECFLauncherSequenceState::Cancelled;
		CancelReason = InCancelReason == ECFLauncherSequenceCancelReason::None
			? ECFLauncherSequenceCancelReason::Manual
			: InCancelReason;
	}

	// [v1.1.0] 시퀀스 런타임을 신규 입력 대기 상태로 초기화합니다.
	void Reset()
	{
		State = ECFLauncherSequenceState::Idle;
		CancelReason = ECFLauncherSequenceCancelReason::None;
		ActiveConfig = FCFLauncherFirePatternConfig();
		VolleyId = 0;
		TotalProjectileCount = 0;
		AttemptedProjectileCount = 0;
		AcceptedProjectileCount = 0;
		FailedProjectileCount = 0;
		RemainingProjectileCount = 0;
		PendingResultCount = 0;
		TimeUntilNextDispatchSeconds = 0.0f;
		bInitialSalvoBatchPending = false;
	}

	// [v1.1.0] 현재 시퀀스가 후속 Projectile을 Dispatch할 수 있는 Active 상태인지 반환합니다.
	bool IsActive() const
	{
		return State == ECFLauncherSequenceState::Active;
	}
};
