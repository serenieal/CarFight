// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 0.1.1
// Date: 2026-07-09
// Description: CarFight Reticle / FireFeedback UI가 읽을 공용 표시 타입 정의
// Changelog:
// - v0.1.1: Step01 범위에 맞춰 ECFVehicleFireFeedbackState와 FCFVehicleFireFeedbackViewData를 신규 추가.
// Migration:
// - 신규 타입 파일이므로 기존 코드 마이그레이션은 없다.
// - 후속 Step에서 Pawn / Reticle Widget이 이 타입을 include하여 표시 데이터만 읽도록 연결한다.
// Scope: 타입 정의 전용. UObject 클래스, .cpp, 서버/RPC/복제 코드는 포함하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFVehicleAimTypes.h"
#include "CFVehicleFireFeedbackTypes.generated.h"

// [v0.1.1] Reticle / UI에 표시할 로컬 발사 피드백 상태입니다.
UENUM(BlueprintType)
enum class ECFVehicleFireFeedbackState : uint8
{
	None,

	FireSuccess,

	FirePending,

	FireRejected,

	Cooldown,

	NoWeapon,

	AimBlocked,

	OutOfArcWarning
};

// [v0.1.1] Reticle / FireFeedback UI가 읽을 로컬 발사 피드백 표시 데이터입니다.
USTRUCT(BlueprintType)
struct FCFVehicleFireFeedbackViewData
{
	GENERATED_BODY()

	// [v0.1.1] 현재 표시할 발사 피드백 상태입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	ECFVehicleFireFeedbackState FeedbackState = ECFVehicleFireFeedbackState::None;

	// [v0.1.1] WeaponFire에서 기록한 마지막 발사 거부 사유입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	ECFVehicleFireRejectReason LastRejectReason = ECFVehicleFireRejectReason::None;

	// [v0.1.1] 현재 피드백을 화면에 표시해야 하는지 여부입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	bool bFeedbackActive = false;

	// [v0.1.1] 기본 Reticle 상태를 피드백 상태로 덮어써야 하는지 여부입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	bool bOverrideReticleState = false;

	// [v0.1.1] 쿨다운 UI를 표시해야 하는지 여부입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	bool bShowCooldown = false;

	// [v0.1.1] 남은 쿨다운 시간입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	float RemainingCooldownSeconds = 0.0f;

	// [v0.1.1] 전체 쿨다운 시간입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	float TotalCooldownSeconds = 0.0f;

	// [v0.1.1] 0.0~1.0 범위의 쿨다운 비율입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	float CooldownRatio = 0.0f;

	// [v0.1.1] 조준각 경고를 보조 표시로 띄워야 하는지 여부입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	bool bShowOutOfArcWarning = false;

	// [v0.1.1] FireFeedback 상태를 한국어 표시로 변환하기 전의 짧은 내부 표시 키입니다.
	UPROPERTY(BlueprintReadOnly, Category="CarFight|Vehicle|FireFeedback")
	FName FeedbackDisplayKey = NAME_None;
};
