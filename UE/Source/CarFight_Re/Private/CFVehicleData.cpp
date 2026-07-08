// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.19.0
// Date: 2026-06-25
// Description: VehicleData 전투 장착 프로파일 보강과 레거시 VehicleMovement 실험값 자동 보정
// Scope: CFVehicleData 에디터 캡처, PostLoad 기반 마이그레이션, 프로젝트 기준 기본값 정렬, 에디터 저장 유도
// Changelog:
// - v1.19.0: Top_01 하드포인트가 있는 기존 자산에 RoofTurret_MediumOrLarge MountProfile을 1회 보강.
// - v1.18.0: 차체 StaticMesh 소켓에서 HardpointSlots의 선택 캡처 소켓을 LocalTransform으로 기록.
// - v1.17.0: 프로젝트 기준 VehicleMovement 기본값에 ThrottleInputScale=1.0을 포함.
// - v1.16.0: ChassisMesh 소켓에서 VehicleLayoutConfig 휠 앵커 값을 직접 캡처하는 DataAsset 에디터 버튼 추가.
// - v1.13.0: 레거시 VehicleMovement 실험값을 현재 프로젝트 차량 기준값으로 자동 보정.
// Migration:
// - 기존 HardpointSlots를 유지하고, MountProfiles는 LocationSlotRef로 하드포인트 슬롯을 참조한다.
// - Top_01 하드포인트가 없거나 RoofTurret_MediumOrLarge 프로파일이 이미 있으면 MountProfiles를 변경하지 않는다.
// - HardpointSlots가 비어 있으면 휠 레이아웃만 캡처하며 오류로 처리하지 않는다.
// - HardpointSlots의 SocketName이 비어 있으면 해당 슬롯의 기존 LocalTransform을 유지한다.
// - 기존 자산은 ThrottleInputScale 기본값 1.0으로 기존 입력 체감을 유지한다.
// - spawned-only 차량은 레벨 액터 대신 DA_* 상세 패널에서 차체 소켓 캡처 버튼을 사용한다.
// - 기존 PostLoad 마이그레이션 조건과 보정값은 변경하지 않는다.

#include "CFVehicleData.h"

#include "Engine/Engine.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Framework/Notifications/NotificationManager.h"
#include "UObject/Package.h"
#include "Widgets/Notifications/SNotificationList.h"

namespace CFVehicleDataMigration
{
	// 부동소수 비교에서 허용할 오차 범위입니다.
	constexpr float FloatTolerance = KINDA_SMALL_NUMBER;

	// 두 부동소수 값이 사실상 같은지 판정합니다.
	bool IsNearlyEqualFloat(const float LeftValue, const float RightValue)
	{
		return FMath::IsNearlyEqual(LeftValue, RightValue, FloatTolerance);
	}

	// 현재 설정이 과거 임시 실험값 세트와 같은지 판정합니다.
	bool IsLegacyPrototypeMovementConfig(const FCFVehicleMovementConfig& VehicleMovementConfig)
	{
		return VehicleMovementConfig.bUseMovementOverrides
			&& VehicleMovementConfig.MovementProfileName.IsNone()
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelMaxSteerAngle, 40.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelMaxBrakeTorque, 4500.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelMaxBrakeTorque, 4500.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelMaxHandBrakeTorque, 6000.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelRadius, 39.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelRadius, 39.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelWidth, 35.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelWidth, 35.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelFrictionForceMultiplier, 3.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelFrictionForceMultiplier, 3.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelCorneringStiffness, 750.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelCorneringStiffness, 750.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelLoadRatio, 1.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelLoadRatio, 1.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelSpringRate, 100.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelSpringRate, 100.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelSpringPreload, 100.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelSpringPreload, 100.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelSuspensionMaxRaise, 20.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelSuspensionMaxRaise, 20.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontWheelSuspensionMaxDrop, 20.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.RearWheelSuspensionMaxDrop, 20.0f)
			&& VehicleMovementConfig.bFrontWheelAffectedByEngine
			&& !VehicleMovementConfig.bRearWheelAffectedByEngine
			&& VehicleMovementConfig.FrontWheelSweepShape == ESweepShape::Shapecast
			&& VehicleMovementConfig.RearWheelSweepShape == ESweepShape::Shapecast
			&& IsNearlyEqualFloat(VehicleMovementConfig.ChassisHeight, 160.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.DragCoefficient, 0.1f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.DownforceCoefficient, 0.1f)
			&& !VehicleMovementConfig.bEnableCenterOfMassOverride
			&& VehicleMovementConfig.CenterOfMassOverride.IsZero()
			&& IsNearlyEqualFloat(VehicleMovementConfig.EngineMaxTorque, 500.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.EngineMaxRPM, 4500.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.EngineIdleRPM, 1200.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.EngineBrakeEffect, 0.05f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.EngineRevUpMOI, 5.0f)
			&& IsNearlyEqualFloat(VehicleMovementConfig.EngineRevDownRate, 600.0f)
			&& VehicleMovementConfig.DifferentialType == EVehicleDifferential::RearWheelDrive
			&& IsNearlyEqualFloat(VehicleMovementConfig.FrontRearSplit, 0.5f)
			&& VehicleMovementConfig.SteeringType == ESteeringType::AngleRatio
			&& IsNearlyEqualFloat(VehicleMovementConfig.SteeringAngleRatio, 0.7f)
			&& VehicleMovementConfig.bLegacyWheelFrictionPosition
			&& VehicleMovementConfig.FrontWheelAdditionalOffset.IsZero()
			&& VehicleMovementConfig.RearWheelAdditionalOffset.IsZero();
	}

	// 현재 프로젝트 WheelClass와 BP_CFVehiclePawn 기준값을 VehicleMovement 설정에 채워 넣습니다.
	void ApplyProjectBaselineMovementConfig(FCFVehicleMovementConfig& VehicleMovementConfig)
	{
		VehicleMovementConfig.FrontWheelMaxSteerAngle = 35.0f;
		VehicleMovementConfig.ThrottleInputScale = 1.0f;
		VehicleMovementConfig.FrontWheelMaxBrakeTorque = 1500.0f;
		VehicleMovementConfig.RearWheelMaxBrakeTorque = 1500.0f;
		VehicleMovementConfig.RearWheelMaxHandBrakeTorque = 3000.0f;
		VehicleMovementConfig.FrontWheelRadius = 30.0f;
		VehicleMovementConfig.RearWheelRadius = 30.0f;
		VehicleMovementConfig.FrontWheelWidth = 6.0f;
		VehicleMovementConfig.RearWheelWidth = 6.0f;
		VehicleMovementConfig.FrontWheelFrictionForceMultiplier = 2.0f;
		VehicleMovementConfig.RearWheelFrictionForceMultiplier = 2.0f;
		VehicleMovementConfig.FrontWheelCorneringStiffness = 1000.0f;
		VehicleMovementConfig.RearWheelCorneringStiffness = 1000.0f;
		VehicleMovementConfig.FrontWheelLoadRatio = 0.5f;
		VehicleMovementConfig.RearWheelLoadRatio = 0.5f;
		VehicleMovementConfig.FrontWheelSpringRate = 250.0f;
		VehicleMovementConfig.RearWheelSpringRate = 250.0f;
		VehicleMovementConfig.FrontWheelSpringPreload = 50.0f;
		VehicleMovementConfig.RearWheelSpringPreload = 50.0f;
		VehicleMovementConfig.FrontWheelSuspensionMaxRaise = 8.0f;
		VehicleMovementConfig.RearWheelSuspensionMaxRaise = 10.0f;
		VehicleMovementConfig.FrontWheelSuspensionMaxDrop = 10.0f;
		VehicleMovementConfig.RearWheelSuspensionMaxDrop = 10.0f;
		VehicleMovementConfig.bFrontWheelAffectedByEngine = false;
		VehicleMovementConfig.bRearWheelAffectedByEngine = true;
		VehicleMovementConfig.FrontWheelSweepShape = ESweepShape::Raycast;
		VehicleMovementConfig.RearWheelSweepShape = ESweepShape::Raycast;
		VehicleMovementConfig.ChassisHeight = 140.0f;
		VehicleMovementConfig.DragCoefficient = 0.3f;
		VehicleMovementConfig.DownforceCoefficient = 0.3f;
		VehicleMovementConfig.bEnableCenterOfMassOverride = false;
		VehicleMovementConfig.CenterOfMassOverride = FVector::ZeroVector;
		VehicleMovementConfig.EngineMaxTorque = 750.0f;
		VehicleMovementConfig.EngineMaxRPM = 7000.0f;
		VehicleMovementConfig.EngineIdleRPM = 900.0f;
		VehicleMovementConfig.EngineBrakeEffect = 0.2f;
		VehicleMovementConfig.EngineRevUpMOI = 5.0f;
		VehicleMovementConfig.EngineRevDownRate = 600.0f;
		VehicleMovementConfig.DifferentialType = EVehicleDifferential::RearWheelDrive;
		VehicleMovementConfig.FrontRearSplit = 0.5f;
		VehicleMovementConfig.SteeringType = ESteeringType::AngleRatio;
		VehicleMovementConfig.SteeringAngleRatio = 0.7f;
		VehicleMovementConfig.bLegacyWheelFrictionPosition = true;
		VehicleMovementConfig.FrontWheelAdditionalOffset = FVector::ZeroVector;
		VehicleMovementConfig.RearWheelAdditionalOffset = FVector::ZeroVector;
	}

	// 지정한 하드포인트 위치 슬롯 ID가 VehicleData에 선언되어 있는지 확인합니다.
	bool HasHardpointSlotId(const TArray<FCFVehicleHardpointSlot>& HardpointSlots, const FName LocationSlotId)
	{
		// 검사할 하드포인트 슬롯입니다.
		for (const FCFVehicleHardpointSlot& HardpointSlot : HardpointSlots)
		{
			if (HardpointSlot.LocationSlotId == LocationSlotId)
			{
				return true;
			}
		}

		return false;
	}

	// 지정한 전투 장착 프로파일 ID가 VehicleData에 이미 선언되어 있는지 확인합니다.
	bool HasMountProfileId(const TArray<FCFVehicleMountProfile>& MountProfiles, const FName MountProfileId)
	{
		// 검사할 전투 장착 프로파일입니다.
		for (const FCFVehicleMountProfile& MountProfile : MountProfiles)
		{
			if (MountProfile.MountProfileId == MountProfileId)
			{
				return true;
			}
		}

		return false;
	}

	// P0 기본 지붕 터렛 프로파일을 기존 VehicleData에 보강해야 하는지 판단합니다.
	bool ShouldSeedP0RoofTurretProfile(const TArray<FCFVehicleHardpointSlot>& HardpointSlots, const TArray<FCFVehicleMountProfile>& MountProfiles)
	{
		// P0 기본 터렛 프로파일이 참조할 하드포인트 슬롯 ID입니다.
		const FName P0LocationSlotId = TEXT("Top_01");

		// P0 기본 터렛 프로파일을 식별하는 ID입니다.
		const FName P0MountProfileId = TEXT("RoofTurret_MediumOrLarge");

		return HasHardpointSlotId(HardpointSlots, P0LocationSlotId) && !HasMountProfileId(MountProfiles, P0MountProfileId);
	}
}

namespace CFVehicleDataSocketCapture
{
	// 하드포인트 선택 캡처 결과 카운트와 경고 요약입니다.
	struct FCFHardpointCaptureStats
	{
		// 소켓을 찾아 LocalTransform을 갱신한 하드포인트 슬롯 수입니다.
		int32 CapturedCount = 0;

		// SocketName이 비어 있어 캡처를 건너뛴 하드포인트 슬롯 수입니다.
		int32 SkippedCount = 0;

		// SocketName은 있지만 차체 메시에서 찾지 못한 하드포인트 슬롯 수입니다.
		int32 MissingCount = 0;

		// LocationSlotId가 비어 있어 식별이 불완전한 하드포인트 슬롯 수입니다.
		int32 InvalidCount = 0;

		// 하드포인트 선택 캡처 중 발생한 경고 요약입니다.
		FString WarningSummary;
	};

	// DataAsset 소켓 이름이 비어 있을 때 사용할 프로젝트 표준 이름을 반환합니다.
	FName ResolveSocketName(const FName ConfiguredSocketName, const FName DefaultSocketName)
	{
		return ConfiguredSocketName.IsNone() ? DefaultSocketName : ConfiguredSocketName;
	}

	// 캡처 실패 요약에 항목을 쉼표로 이어 붙입니다.
	void AppendCaptureFailure(FString& InOutFailureSummary, const FString& FailureText)
	{
		if (!InOutFailureSummary.IsEmpty())
		{
			InOutFailureSummary += TEXT(", ");
		}
		InOutFailureSummary += FailureText;
	}

	// 하드포인트 캡처 경고 요약에 항목을 쉼표로 이어 붙입니다.
	void AppendHardpointWarning(FCFHardpointCaptureStats& InOutStats, const FString& WarningText)
	{
		if (!InOutStats.WarningSummary.IsEmpty())
		{
			InOutStats.WarningSummary += TEXT(", ");
		}
		InOutStats.WarningSummary += WarningText;
	}

	// DataAsset 버튼 실행 결과를 Output Log, 화면 메시지, Slate 알림으로 함께 확인할 수 있게 합니다.
	void ShowCaptureMessage(const FString& MessageText, const FColor& ScreenColor)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.0f, ScreenColor, MessageText);
		}

		// Details 패널 버튼 실행 결과를 PIE 외부에서도 볼 수 있게 띄우는 에디터 알림 정보입니다.
		FNotificationInfo NotificationInfo(FText::FromString(MessageText));
		NotificationInfo.ExpireDuration = 5.0f;
		NotificationInfo.bFireAndForget = true;

		// 화면 오른쪽 하단에 표시되는 Slate 알림 항목입니다.
		TSharedPtr<SNotificationItem> NotificationItem = FSlateNotificationManager::Get().AddNotification(NotificationInfo);
		if (NotificationItem.IsValid())
		{
			NotificationItem->SetCompletionState(ScreenColor == FColor::Red ? SNotificationItem::CS_Fail : SNotificationItem::CS_Success);
		}
	}

	// StaticMesh 소켓의 로컬 Transform을 VehicleLayoutConfig에 저장할 휠 앵커 포즈로 변환합니다.
	bool BuildWheelAnchorPoseFromMeshSocket(
		const UStaticMesh* ChassisMesh,
		const FName SocketName,
		FCFWheelAnchorPose& OutWheelAnchorPose,
		FString& InOutFailureSummary)
	{
		if (!ChassisMesh)
		{
			AppendCaptureFailure(InOutFailureSummary, TEXT("ChassisMesh=Missing"));
			return false;
		}

		// 차체 StaticMesh에서 찾은 소켓 객체입니다.
		const UStaticMeshSocket* MeshSocket = ChassisMesh->FindSocket(SocketName);
		if (!MeshSocket)
		{
			AppendCaptureFailure(InOutFailureSummary, FString::Printf(TEXT("SocketMissing=%s"), *SocketName.ToString()));
			return false;
		}

		OutWheelAnchorPose.RelativeLocation = MeshSocket->RelativeLocation;
		OutWheelAnchorPose.RelativeRotation = MeshSocket->RelativeRotation;
		return true;
	}

	// 차체 StaticMesh 소켓 Transform을 하드포인트 슬롯 LocalTransform에 선택적으로 캡처합니다.
	void CaptureHardpointSlotFromMeshSocket(
		const UStaticMesh* ChassisMesh,
		FCFVehicleHardpointSlot& InOutHardpointSlot,
		FCFHardpointCaptureStats& InOutStats)
	{
		if (InOutHardpointSlot.LocationSlotId.IsNone())
		{
			++InOutStats.InvalidCount;
			AppendHardpointWarning(InOutStats, TEXT("HardpointInvalidSlotId=None"));
		}

		if (InOutHardpointSlot.SocketName.IsNone())
		{
			++InOutStats.SkippedCount;
			return;
		}

		if (!ChassisMesh)
		{
			++InOutStats.MissingCount;
			AppendHardpointWarning(InOutStats, FString::Printf(TEXT("HardpointChassisMissing=%s"), *InOutHardpointSlot.SocketName.ToString()));
			return;
		}

		// 차체 StaticMesh에서 찾은 하드포인트 소켓 객체입니다.
		const UStaticMeshSocket* MeshSocket = ChassisMesh->FindSocket(InOutHardpointSlot.SocketName);
		if (!MeshSocket)
		{
			++InOutStats.MissingCount;
			AppendHardpointWarning(InOutStats, FString::Printf(TEXT("HardpointSocketMissing=%s:%s"), *InOutHardpointSlot.LocationSlotId.ToString(), *InOutHardpointSlot.SocketName.ToString()));
			return;
		}

		InOutHardpointSlot.LocalLocation = MeshSocket->RelativeLocation;
		InOutHardpointSlot.LocalRotation = MeshSocket->RelativeRotation;
		++InOutStats.CapturedCount;
	}

	// 하드포인트 선택 캡처 결과에 경고가 포함되어 있는지 반환합니다.
	bool HasHardpointCaptureWarning(const FCFHardpointCaptureStats& CaptureStats)
	{
		return CaptureStats.MissingCount > 0 || CaptureStats.InvalidCount > 0;
	}
}

// 레거시 VehicleMovement 실험값 자산을 현재 프로젝트 기준값으로 보정합니다.
void UCFVehicleData::PostLoad()
{
	Super::PostLoad();

	// 현재 로드된 VehicleMovement 설정 참조입니다.
	FCFVehicleMovementConfig& MutableVehicleMovementConfig = this->VehicleMovementConfig;

	// PostLoad에서 VehicleData를 실제로 수정했는지 여부입니다.
	bool bModifiedVehicleData = false;

	if (CFVehicleDataMigration::IsLegacyPrototypeMovementConfig(MutableVehicleMovementConfig))
	{
		Modify();
		CFVehicleDataMigration::ApplyProjectBaselineMovementConfig(MutableVehicleMovementConfig);
		bModifiedVehicleData = true;
	}

	if (CFVehicleDataMigration::ShouldSeedP0RoofTurretProfile(HardpointSlots, MountProfiles))
	{
		if (!bModifiedVehicleData)
		{
			Modify();
		}

		// 기존 Top_01 하드포인트에 연결할 P0 기본 지붕 터렛 프로파일입니다.
		const FCFVehicleMountProfile P0RoofTurretProfile;
		MountProfiles.Add(P0RoofTurretProfile);
		bModifiedVehicleData = true;
	}

#if WITH_EDITOR
	if (bModifiedVehicleData)
	{
		MarkPackageDirty();
	}
#endif
}

#if WITH_EDITOR
// VehicleVisualConfig.ChassisMesh 소켓에서 휠 앵커와 선택 하드포인트 위치 값을 직접 캡처합니다.
void UCFVehicleData::CaptureLayoutFromChassisSockets()
{
	// 소켓을 읽을 차체 StaticMesh 자산입니다.
	UStaticMesh* ChassisMesh = VehicleVisualConfig.ChassisMesh;
	if (!ChassisMesh)
	{
		const FString FailureMessage = TEXT("VehicleDataSocketCapture: ChassisMesh=Missing");
		UE_LOG(LogTemp, Warning, TEXT("%s"), *FailureMessage);
		CFVehicleDataSocketCapture::ShowCaptureMessage(FailureMessage, FColor::Red);
		return;
	}

	// 현재 DataAsset에 저장된 레이아웃 설정입니다.
	const FCFVehicleLayoutConfig& ExistingLayoutConfig = VehicleLayoutConfig;

	// 앞왼쪽 바퀴 중심을 읽을 차체 소켓 이름입니다.
	const FName BodySocketFL = CFVehicleDataSocketCapture::ResolveSocketName(ExistingLayoutConfig.BodyWheelSocketFL, FName(TEXT("Wheel_Anchor_FL")));

	// 앞오른쪽 바퀴 중심을 읽을 차체 소켓 이름입니다.
	const FName BodySocketFR = CFVehicleDataSocketCapture::ResolveSocketName(ExistingLayoutConfig.BodyWheelSocketFR, FName(TEXT("Wheel_Anchor_FR")));

	// 뒤왼쪽 바퀴 중심을 읽을 차체 소켓 이름입니다.
	const FName BodySocketRL = CFVehicleDataSocketCapture::ResolveSocketName(ExistingLayoutConfig.BodyWheelSocketRL, FName(TEXT("Wheel_Anchor_RL")));

	// 뒤오른쪽 바퀴 중심을 읽을 차체 소켓 이름입니다.
	const FName BodySocketRR = CFVehicleDataSocketCapture::ResolveSocketName(ExistingLayoutConfig.BodyWheelSocketRR, FName(TEXT("Wheel_Anchor_RR")));

	// 앞왼쪽 바퀴 앵커에 저장할 캡처 포즈입니다.
	FCFWheelAnchorPose CapturedWheelAnchorFL;

	// 앞오른쪽 바퀴 앵커에 저장할 캡처 포즈입니다.
	FCFWheelAnchorPose CapturedWheelAnchorFR;

	// 뒤왼쪽 바퀴 앵커에 저장할 캡처 포즈입니다.
	FCFWheelAnchorPose CapturedWheelAnchorRL;

	// 뒤오른쪽 바퀴 앵커에 저장할 캡처 포즈입니다.
	FCFWheelAnchorPose CapturedWheelAnchorRR;

	// 캡처 실패 원인을 모두 모아 표시할 요약 문자열입니다.
	FString FailureSummary;

	// 앞왼쪽 소켓 캡처 성공 여부입니다.
	const bool bCapturedFL = CFVehicleDataSocketCapture::BuildWheelAnchorPoseFromMeshSocket(ChassisMesh, BodySocketFL, CapturedWheelAnchorFL, FailureSummary);

	// 앞오른쪽 소켓 캡처 성공 여부입니다.
	const bool bCapturedFR = CFVehicleDataSocketCapture::BuildWheelAnchorPoseFromMeshSocket(ChassisMesh, BodySocketFR, CapturedWheelAnchorFR, FailureSummary);

	// 뒤왼쪽 소켓 캡처 성공 여부입니다.
	const bool bCapturedRL = CFVehicleDataSocketCapture::BuildWheelAnchorPoseFromMeshSocket(ChassisMesh, BodySocketRL, CapturedWheelAnchorRL, FailureSummary);

	// 뒤오른쪽 소켓 캡처 성공 여부입니다.
	const bool bCapturedRR = CFVehicleDataSocketCapture::BuildWheelAnchorPoseFromMeshSocket(ChassisMesh, BodySocketRR, CapturedWheelAnchorRR, FailureSummary);

	if (!bCapturedFL || !bCapturedFR || !bCapturedRL || !bCapturedRR)
	{
		const FString FailureMessage = FString::Printf(TEXT("VehicleDataSocketCapture: Failed, %s"), *FailureSummary);
		UE_LOG(LogTemp, Warning, TEXT("%s"), *FailureMessage);
		CFVehicleDataSocketCapture::ShowCaptureMessage(FailureMessage, FColor::Red);
		return;
	}

	Modify();

	VehicleLayoutConfig.bUseLayoutOverrides = true;
	VehicleLayoutConfig.BodyWheelSocketFL = BodySocketFL;
	VehicleLayoutConfig.BodyWheelSocketFR = BodySocketFR;
	VehicleLayoutConfig.BodyWheelSocketRL = BodySocketRL;
	VehicleLayoutConfig.BodyWheelSocketRR = BodySocketRR;
	VehicleLayoutConfig.WheelAnchorFL = CapturedWheelAnchorFL;
	VehicleLayoutConfig.WheelAnchorFR = CapturedWheelAnchorFR;
	VehicleLayoutConfig.WheelAnchorRL = CapturedWheelAnchorRL;
	VehicleLayoutConfig.WheelAnchorRR = CapturedWheelAnchorRR;

	// 하드포인트 선택 캡처 결과 카운트입니다.
	CFVehicleDataSocketCapture::FCFHardpointCaptureStats HardpointCaptureStats;

	// 현재 선택 캡처를 시도할 하드포인트 슬롯입니다.
	for (FCFVehicleHardpointSlot& HardpointSlot : HardpointSlots)
	{
		CFVehicleDataSocketCapture::CaptureHardpointSlotFromMeshSocket(ChassisMesh, HardpointSlot, HardpointCaptureStats);
	}

	MarkPackageDirty();

	// 하드포인트 선택 캡처 중 경고가 있었는지 여부입니다.
	const bool bHasHardpointWarning = CFVehicleDataSocketCapture::HasHardpointCaptureWarning(HardpointCaptureStats);

	// 캡처 결과 상태 이름입니다.
	const TCHAR* CaptureStateText = bHasHardpointWarning ? TEXT("AppliedWithWarning") : TEXT("Applied");

	// 하드포인트 경고 요약 표시 문자열입니다.
	const FString HardpointWarningSuffix = HardpointCaptureStats.WarningSummary.IsEmpty() ? FString() : FString::Printf(TEXT(", Warnings=%s"), *HardpointCaptureStats.WarningSummary);

	const FString SuccessMessage = FString::Printf(
		TEXT("VehicleDataSocketCapture: %s, ChassisMesh=%s, Wheels=4/4, Sockets=%s/%s/%s/%s, Hardpoints=Captured=%d, Skipped=%d, Missing=%d, Invalid=%d%s"),
		CaptureStateText,
		*ChassisMesh->GetName(),
		*BodySocketFL.ToString(),
		*BodySocketFR.ToString(),
		*BodySocketRL.ToString(),
		*BodySocketRR.ToString(),
		HardpointCaptureStats.CapturedCount,
		HardpointCaptureStats.SkippedCount,
		HardpointCaptureStats.MissingCount,
		HardpointCaptureStats.InvalidCount,
		*HardpointWarningSuffix);
	UE_LOG(LogTemp, Display, TEXT("%s"), *SuccessMessage);
	CFVehicleDataSocketCapture::ShowCaptureMessage(SuccessMessage, bHasHardpointWarning ? FColor::Yellow : FColor::Green);
}
#endif
