// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFWSAMigrateCmdlet.cpp
// Version: v1.1.1
// Date: 2026-08-28
// Description: WSA-P0-05 Wagon 전용 SocketScale migration의 bounded one-shot 구현입니다.
// Changelog:
// - v1.1.1: deferred Apply 허용 조건을 exact 4개 core Profile 누락 + Front/Rear WheelClass 누락으로 강화하고 duplicate/DriveState/기존 binding/예상 밖 blocker negative regression을 추가.
// - v1.1.0: 실제 Wagon E2E가 아직 Step 2~4 상태일 때 Profile/WheelClass 같은 후속 Step blocker를 우회하지 않고, WSA Recipe intent/adoption만 저장하고 Target Apply를 정상 Step 7까지 defer하는 경계를 추가.
// - v1.0.3: Preview validation 실패 시 Recipe/Resolver/Definition structured issue의 layer/severity/code/path/message를 모두 진단 로그에 보존.
// - v1.0.2: canonical Wheel의 99.999/24.9992cm 실제 bounds에서 생기는 sub-mm 파생 오차를 공용 0.01cm size compatibility tolerance로 판정하도록 교정.
// - v1.0.1: 두 Wagon package Save 중간 실패 시 양쪽 파일을 migration 직전 바이트로 복원하는 pair rollback을 추가.
// - v1.0.0: USER-authored Wagon Socket Scale (0.8,1.0,0.8) preflight, shared canonical Wheel FL binding,
//   SocketScaleFromChassis typed commit, exact Wheel measurement 4건 reviewed adoption, normal Apply와 exact 2-package Save를 추가.
// Migration:
// - 기존 Resolver/Validator/Apply writer를 재사용하며 raw VehicleData/Recipe property write를 하지 않습니다.
// - WSA 자체 검증/Resolver 실패는 Save 전에 종료해 persistent partial migration을 남기지 않습니다.
// - 실제 Wagon이 아직 Builder Step 2~4이고 후속 Profile/WheelClass만 미완성이면 Recipe의 WSA intent/adoption만 저장하고 Target Apply는 Step 7까지 defer합니다.
// - Preview가 완전 Apply-ready인 경우에만 기존 DefinitionApply를 실행해 DA_Vehicle_Wagon + DA_Recipe_Wagon 두 package를 저장합니다. 다른 package는 저장하지 않습니다.

#include "CFWSAMigrateCmdlet.h"

#include "CFVehicleData.h"
#include "CFWheelSizeUtils.h"
#include "DataAuthoring/CFVehicleAuthoringTypes.h"
#include "DataAuthoring/CFVehicleAuthoringVM.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#endif
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFWSAMigrate, Log, All);

namespace CFWSAMigrate
{
	// USER가 승인한 Wagon Recipe exact object path입니다.
	const TCHAR* WagonRecipeObjectPath = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_Wagon.DA_Recipe_Wagon");

	// USER가 승인한 Wagon Target VehicleData exact object path입니다.
	const TCHAR* WagonTargetObjectPath = TEXT("/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon");

	// USER가 직접 Wheel Socket을 배치/스케일한 Wagon Chassis exact object path입니다.
	const TCHAR* WagonChassisObjectPath = TEXT("/Game/CarFight/Vehicles/Meshes/Wagon/Wagon.Wagon");

	// WSA canonical 100x25x100 source Wheel exact object path입니다.
	const TCHAR* SharedWheelObjectPath = TEXT("/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL.Wheel_FL");

	// USER 승인 audit에서 확인된 Wagon Wheel Socket Scale입니다.
	const FVector ExpectedSocketScale(0.8, 1.0, 0.8);

	// Canonical Wheel 100x25x100에 0.8/1.0/0.8 Scale을 적용한 기대 Chaos Radius입니다.
	const float ExpectedWheelRadiusCm = 40.0f;

	// Canonical Wheel 100x25x100에 0.8/1.0/0.8 Scale을 적용한 기대 Chaos Width입니다.
	const float ExpectedWheelWidthCm = 25.0f;

	// Commandlet 단계별 실패를 프로세스 종료 코드로 구분합니다.
	enum class EExitCode : int32
	{
		Success = 0,
		AssetLoadFailed = 41,
		PreflightFailed = 42,
		SelectionFailed = 43,
		AssetIntentCommitFailed = 44,
		WheelIntentCommitFailed = 45,
		MeasurementFailed = 46,
		PreviewValidationFailed = 47,
		ApplyFailed = 48,
		PostApplyVerificationFailed = 49,
		SaveFailed = 50
	};

	// USER 승인 당시 Wheel Socket의 exact current truth입니다.
	struct FExpectedWheelSocket
	{
		// StaticMesh Socket의 exact 이름입니다.
		const TCHAR* SocketName;

		// USER가 배치한 exact local 위치입니다.
		FVector RelativeLocation;
	};

	// USER 승인 audit에서 확인된 FL/FR/RL/RR Socket 위치입니다.
	const FExpectedWheelSocket ExpectedWheelSockets[] =
	{
		{TEXT("Wheel_Anchor_FL"), FVector(152.0, -84.0, 17.0)},
		{TEXT("Wheel_Anchor_FR"), FVector(152.0, 84.0, 17.0)},
		{TEXT("Wheel_Anchor_RL"), FVector(-128.0, -84.0, 17.0)},
		{TEXT("Wheel_Anchor_RR"), FVector(-128.0, 84.0, 17.0)}
	};

	// Migration이 채택해야 하는 Socket-derived Wheel measurement 하나의 기대 계약입니다.
	struct FExpectedMeasurement
	{
		// Resolver의 canonical target field path입니다.
		const TCHAR* FieldPath;

		// USER Socket Scale에서 파생되어야 하는 exact 수치입니다.
		float ExpectedValue;

		// 해당 field가 사용해야 하는 WSA measurement rule입니다.
		FName ExpectedRuleId;
	};

	// Front/Rear Radius/Width 네 건의 USER 승인 derived 결과입니다.
	const FExpectedMeasurement ExpectedMeasurements[] =
	{
		{TEXT("VehicleMovementConfig.FrontWheelRadius"), ExpectedWheelRadiusCm, TEXT("WheelSocketScale.Radius.v1")},
		{TEXT("VehicleMovementConfig.RearWheelRadius"), ExpectedWheelRadiusCm, TEXT("WheelSocketScale.Radius.v1")},
		{TEXT("VehicleMovementConfig.FrontWheelWidth"), ExpectedWheelWidthCm, TEXT("WheelSocketScale.Width.v1")},
		{TEXT("VehicleMovementConfig.RearWheelWidth"), ExpectedWheelWidthCm, TEXT("WheelSocketScale.Width.v1")}
	};

	// Preview validation bucket 하나의 structured issue를 손실 없이 로그에 출력합니다.
	void LogValidationBucket(const TCHAR* LayerName, const TArray<FCFVehicleValidationIssue>& Issues)
	{
		for (const FCFVehicleValidationIssue& Issue : Issues)
		{
			// Structured field path가 존재할 때 사용할 canonical path입니다.
			const FString CanonicalFieldPath = Issue.FieldPath.ToCanonicalString(true);

			UE_LOG(
				LogCFWSAMigrate,
				Warning,
				TEXT("CF_WSA_MIGRATE_VALIDATION layer=%s severity=%d code=%s field=%s validator_field=%s message=%s"),
				LayerName,
				static_cast<int32>(Issue.Severity),
				*Issue.IssueCode.ToString(),
				CanonicalFieldPath.IsEmpty() ? TEXT("<none>") : *CanonicalFieldPath,
				Issue.ValidatorFieldPath.IsEmpty() ? TEXT("<none>") : *Issue.ValidatorFieldPath,
				*Issue.Message);
		}
	}

	// RequiredProfileMissing issue가 exact core Profile 하나를 가리키는지 판정합니다.
	bool MatchRequiredProfileMissing(const FCFVehicleValidationIssue& Issue, const TCHAR* ProfileLabel)
	{
		return Issue.Severity == ECFVehicleValidationSeverity::Blocked
			&& Issue.IssueCode == TEXT("RequiredProfileMissing")
			&& Issue.Message == FString::Printf(TEXT("Managed Resolve에 필요한 %s Profile Snapshot이 없습니다."), ProfileLabel);
	}

	// 현재 Wagon의 미완성 Builder Step에서만 허용되는 후속-Step validation blocker인지 판정합니다.
	bool IsExpectedDeferredApplyValidation(const UCFVehicleRecipeData* Recipe, const FCFVehicleValidationReadResult& ValidationResult, FString& OutFailureReason)
	{
		if (!Recipe)
		{
			OutFailureReason = TEXT("Deferred Apply 판정 대상 Recipe가 null입니다.");
			return false;
		}

		// Step 2~4 audit baseline에서는 core 4 Profile과 optional DriveState binding이 모두 아직 비어 있어야 합니다.
		if (!Recipe->ProfileBindings.VehicleBaseProfile.IsNull()
			|| !Recipe->ProfileBindings.DrivetrainProfile.IsNull()
			|| !Recipe->ProfileBindings.HandlingProfile.IsNull()
			|| !Recipe->ProfileBindings.PerformanceProfile.IsNull()
			|| !Recipe->ProfileBindings.DriveStateProfile.IsNull())
		{
			OutFailureReason = TEXT("Deferred Apply는 Profile binding이 전부 비어 있는 Wagon Step 2~4 audit baseline에서만 허용됩니다.");
			return false;
		}

		// Vehicle Base Profile 누락 issue 관측 여부입니다.
		bool bVehicleBaseMissing = false;
		// Drivetrain Profile 누락 issue 관측 여부입니다.
		bool bDrivetrainMissing = false;
		// Handling Profile 누락 issue 관측 여부입니다.
		bool bHandlingMissing = false;
		// Performance Profile 누락 issue 관측 여부입니다.
		bool bPerformanceMissing = false;
		for (const FCFVehicleValidationIssue& Issue : ValidationResult.RecipeValidation)
		{
			if (Issue.Severity == ECFVehicleValidationSeverity::Info || Issue.Severity == ECFVehicleValidationSeverity::Warning)
			{
				continue;
			}

			// 현재 RequiredProfileMissing issue가 매칭한 exact core Profile flag입니다.
			bool* MatchedFlag = nullptr;
			if (MatchRequiredProfileMissing(Issue, TEXT("Vehicle Base"))) MatchedFlag = &bVehicleBaseMissing;
			else if (MatchRequiredProfileMissing(Issue, TEXT("Drivetrain"))) MatchedFlag = &bDrivetrainMissing;
			else if (MatchRequiredProfileMissing(Issue, TEXT("Handling"))) MatchedFlag = &bHandlingMissing;
			else if (MatchRequiredProfileMissing(Issue, TEXT("Performance"))) MatchedFlag = &bPerformanceMissing;

			if (MatchedFlag)
			{
				if (*MatchedFlag)
				{
					OutFailureReason = FString::Printf(TEXT("RequiredProfileMissing duplicate issue가 있습니다: %s"), *Issue.Message);
					return false;
				}
				*MatchedFlag = true;
				continue;
			}

			OutFailureReason = FString::Printf(
				TEXT("Recipe에 exact core 4 Profile 누락 외 blocking validation이 있습니다: code=%s message=%s"),
				*Issue.IssueCode.ToString(),
				*Issue.Message);
			return false;
		}

		for (const FCFVehicleValidationIssue& Issue : ValidationResult.ResolverValidation)
		{
			if (Issue.Severity == ECFVehicleValidationSeverity::Error || Issue.Severity == ECFVehicleValidationSeverity::Blocked)
			{
				OutFailureReason = FString::Printf(
					TEXT("Resolver blocking validation은 deferred Apply 허용 대상이 아닙니다: code=%s field=%s message=%s"),
					*Issue.IssueCode.ToString(),
					*Issue.FieldPath.ToCanonicalString(true),
					*Issue.Message);
				return false;
			}
		}

		// Front Wheel Class 누락 issue 관측 여부입니다.
		bool bFrontWheelClassMissing = false;
		// Rear Wheel Class 누락 issue 관측 여부입니다.
		bool bRearWheelClassMissing = false;
		for (const FCFVehicleValidationIssue& Issue : ValidationResult.DefinitionValidation)
		{
			if (Issue.Severity == ECFVehicleValidationSeverity::Info || Issue.Severity == ECFVehicleValidationSeverity::Warning)
			{
				continue;
			}

			if (Issue.Severity == ECFVehicleValidationSeverity::Error
				&& Issue.IssueCode == TEXT("Definition.RequiredRefs")
				&& Issue.ValidatorFieldPath == TEXT("VehicleReferenceConfig.FrontWheelClass"))
			{
				if (bFrontWheelClassMissing)
				{
					OutFailureReason = TEXT("FrontWheelClass RequiredRefs duplicate issue가 있습니다.");
					return false;
				}
				bFrontWheelClassMissing = true;
				continue;
			}

			if (Issue.Severity == ECFVehicleValidationSeverity::Error
				&& Issue.IssueCode == TEXT("Definition.RequiredRefs")
				&& Issue.ValidatorFieldPath == TEXT("VehicleReferenceConfig.RearWheelClass"))
			{
				if (bRearWheelClassMissing)
				{
					OutFailureReason = TEXT("RearWheelClass RequiredRefs duplicate issue가 있습니다.");
					return false;
				}
				bRearWheelClassMissing = true;
				continue;
			}

			OutFailureReason = FString::Printf(
				TEXT("Definition에 승인되지 않은 blocking validation이 있습니다: code=%s validator_field=%s message=%s"),
				*Issue.IssueCode.ToString(),
				*Issue.ValidatorFieldPath,
				*Issue.Message);
			return false;
		}

		if (!bVehicleBaseMissing || !bDrivetrainMissing || !bHandlingMissing || !bPerformanceMissing
			|| !bFrontWheelClassMissing || !bRearWheelClassMissing)
		{
			OutFailureReason = FString::Printf(
				TEXT("Wagon Step 2~4 deferred blocker signature가 audit baseline과 다릅니다: base=%s drivetrain=%s handling=%s performance=%s front_class=%s rear_class=%s"),
				bVehicleBaseMissing ? TEXT("missing") : TEXT("not-missing"),
				bDrivetrainMissing ? TEXT("missing") : TEXT("not-missing"),
				bHandlingMissing ? TEXT("missing") : TEXT("not-missing"),
				bPerformanceMissing ? TEXT("missing") : TEXT("not-missing"),
				bFrontWheelClassMissing ? TEXT("missing") : TEXT("not-missing"),
				bRearWheelClassMissing ? TEXT("missing") : TEXT("not-missing"));
			return false;
		}

		OutFailureReason.Reset();
		return true;
	}

	// Recipe-only migration 뒤 WSA authoring truth가 정확히 저장 가능한 상태인지 검증합니다.
	bool VerifyMigratedRecipe(const UCFVehicleRecipeData* Recipe, const UStaticMesh* SharedWheelMesh, FString& OutFailureReason)
	{
		if (!Recipe || !SharedWheelMesh)
		{
			OutFailureReason = TEXT("Recipe-only verification 대상이 null입니다.");
			return false;
		}

		if (Recipe->AssetIntent.WheelMeshFL.Get() != SharedWheelMesh
			|| !Recipe->AssetIntent.WheelMeshFR.IsNull()
			|| !Recipe->AssetIntent.WheelMeshRL.IsNull()
			|| !Recipe->AssetIntent.WheelMeshRR.IsNull())
		{
			OutFailureReason = TEXT("Recipe Wheel binding이 FL shared-wheel + FR/RL/RR null fallback 계약과 다릅니다.");
			return false;
		}

		if (Recipe->WheelVisualIntent.Mode != ECFWheelVisualIntentMode::SocketScaleFromChassis)
		{
			OutFailureReason = TEXT("Recipe WheelVisualIntent가 SocketScaleFromChassis가 아닙니다.");
			return false;
		}

		const FCFVehicleAssetAdoption& Adoption = Recipe->AssetAdoption;
		if (!Adoption.bUseMeasuredFrontRadius
			|| !Adoption.bUseMeasuredRearRadius
			|| !Adoption.bUseMeasuredFrontWidth
			|| !Adoption.bUseMeasuredRearWidth
			|| Adoption.FrontRadiusAssetFingerprint.IsEmpty()
			|| Adoption.RearRadiusAssetFingerprint.IsEmpty()
			|| Adoption.FrontWidthAssetFingerprint.IsEmpty()
			|| Adoption.RearWidthAssetFingerprint.IsEmpty())
		{
			OutFailureReason = TEXT("Recipe에 Socket-derived Radius/Width 4건의 reviewed adoption/fingerprint가 모두 저장되지 않았습니다.");
			return false;
		}

		OutFailureReason.Reset();
		return true;
	}

	// Recipe-only migration에서 Target VehicleData가 Apply되지 않은 audit baseline을 유지하는지 확인합니다.
	bool VerifyTargetStillUnapplied(const UCFVehicleData* TargetVehicleData, FString& OutFailureReason)
	{
		if (!TargetVehicleData)
		{
			OutFailureReason = TEXT("Unapplied Target verification 대상이 null입니다.");
			return false;
		}

		if (TargetVehicleData->WheelVisualConfig.bUseWheelSocketScale
			|| TargetVehicleData->WheelVisualConfig.bAutoScaleWheelMeshToRadius
			|| TargetVehicleData->VehicleLayoutConfig.bUseLayoutOverrides)
		{
			OutFailureReason = TEXT("Recipe-only migration 중 Target Wheel/Layout authority가 예상 밖으로 변경되었습니다.");
			return false;
		}

		if (TargetVehicleData->VehicleVisualConfig.WheelMeshFL
			|| TargetVehicleData->VehicleVisualConfig.WheelMeshFR
			|| TargetVehicleData->VehicleVisualConfig.WheelMeshRL
			|| TargetVehicleData->VehicleVisualConfig.WheelMeshRR)
		{
			OutFailureReason = TEXT("Recipe-only migration 중 Target Wheel Mesh reference가 예상 밖으로 변경되었습니다.");
			return false;
		}

		const FCFVehicleMovementConfig& Movement = TargetVehicleData->VehicleMovementConfig;
		if (!FMath::IsNearlyEqual(Movement.FrontWheelRadius, 30.0f, KINDA_SMALL_NUMBER)
			|| !FMath::IsNearlyEqual(Movement.RearWheelRadius, 30.0f, KINDA_SMALL_NUMBER)
			|| !FMath::IsNearlyEqual(Movement.FrontWheelWidth, 6.0f, KINDA_SMALL_NUMBER)
			|| !FMath::IsNearlyEqual(Movement.RearWheelWidth, 6.0f, KINDA_SMALL_NUMBER))
		{
			OutFailureReason = TEXT("Recipe-only migration 중 Target Wheel physics baseline(30/30/6/6)이 예상 밖으로 변경되었습니다.");
			return false;
		}

		OutFailureReason.Reset();
		return true;
	}

	// 두 UObject path가 문자열 변형 없이 같은 exact object를 가리키는지 확인합니다.
	bool IsExactObjectPath(const FSoftObjectPath& ObjectPath, const TCHAR* ExpectedObjectPath)
	{
		return ObjectPath.ToString() == ExpectedObjectPath;
	}

	// USER audit 당시 Socket location/rotation/scale truth가 현재도 그대로인지 fail-closed 검증합니다.
	bool ValidateExpectedSocket(
		const UStaticMesh* ChassisMesh,
		const FExpectedWheelSocket& ExpectedSocket,
		FCFDerivedWheelSize& OutDerivedSize,
		const FVector& SharedWheelBoundsExtent,
		FString& OutFailureReason)
	{
		if (!ChassisMesh)
		{
			OutFailureReason = TEXT("Wagon Chassis가 null입니다.");
			return false;
		}

		// 현재 Chassis에서 exact name으로 찾은 Wheel Socket입니다.
		const UStaticMeshSocket* MeshSocket = ChassisMesh->FindSocket(FName(ExpectedSocket.SocketName));
		if (!MeshSocket)
		{
			OutFailureReason = FString::Printf(TEXT("필수 Wagon Wheel Socket이 없습니다: %s"), ExpectedSocket.SocketName);
			return false;
		}

		if (!MeshSocket->RelativeLocation.Equals(ExpectedSocket.RelativeLocation, KINDA_SMALL_NUMBER))
		{
			OutFailureReason = FString::Printf(
				TEXT("USER 승인 뒤 Wagon Socket 위치가 변경되었습니다: %s current=%s expected=%s"),
				ExpectedSocket.SocketName,
				*MeshSocket->RelativeLocation.ToString(),
				*ExpectedSocket.RelativeLocation.ToString());
			return false;
		}

		if (!MeshSocket->RelativeRotation.Equals(FRotator::ZeroRotator, KINDA_SMALL_NUMBER))
		{
			OutFailureReason = FString::Printf(
				TEXT("USER 승인 뒤 Wagon Socket 회전이 변경되었습니다: %s current=%s"),
				ExpectedSocket.SocketName,
				*MeshSocket->RelativeRotation.ToString());
			return false;
		}

		if (!MeshSocket->RelativeScale.Equals(ExpectedSocketScale, KINDA_SMALL_NUMBER))
		{
			OutFailureReason = FString::Printf(
				TEXT("USER 승인 뒤 Wagon Socket Scale이 변경되었습니다: %s current=%s expected=%s"),
				ExpectedSocket.SocketName,
				*MeshSocket->RelativeScale.ToString(),
				*ExpectedSocketScale.ToString());
			return false;
		}

		// 공용 WSA helper가 현재 Socket Scale을 유효한 USER authority로 인정하는지 확인하는 사유입니다.
		FString ScaleError;
		if (!FCFWheelSizeUtils::ValidateWheelSocketScale(MeshSocket->RelativeScale, ScaleError))
		{
			OutFailureReason = FString::Printf(TEXT("%s Socket Scale invalid: %s"), ExpectedSocket.SocketName, *ScaleError);
			return false;
		}

		// 현재 canonical Wheel bounds + USER Socket Scale에서 파생한 per-wheel size입니다.
		FString DeriveError;
		if (!FCFWheelSizeUtils::DeriveWheelSizeFromBoundsAndScale(
			SharedWheelBoundsExtent,
			MeshSocket->RelativeScale,
			OutDerivedSize,
			DeriveError))
		{
			OutFailureReason = FString::Printf(TEXT("%s Socket-derived Wheel Size 계산 실패: %s"), ExpectedSocket.SocketName, *DeriveError);
			return false;
		}

		if (!FMath::IsNearlyEqual(OutDerivedSize.RadiusCm, ExpectedWheelRadiusCm, FCFWheelSizeUtils::DefaultSizeCompatibilityToleranceCm)
			|| !FMath::IsNearlyEqual(OutDerivedSize.WidthCm, ExpectedWheelWidthCm, FCFWheelSizeUtils::DefaultSizeCompatibilityToleranceCm))
		{
			OutFailureReason = FString::Printf(
				TEXT("%s derived size가 USER 승인값과 다릅니다: R=%.4f W=%.4f"),
				ExpectedSocket.SocketName,
				OutDerivedSize.RadiusCm,
				OutDerivedSize.WidthCm);
			return false;
		}

		return true;
	}

	// 현재 Wagon Recipe/Target/Chassis/Wheel이 USER 승인 audit baseline과 동일한지 검증합니다.
	bool ValidatePreflight(
		const UCFVehicleRecipeData* Recipe,
		const UCFVehicleData* TargetVehicleData,
		const UStaticMesh* ChassisMesh,
		const UStaticMesh* SharedWheelMesh,
		FString& OutFailureReason)
	{
		if (!Recipe || !TargetVehicleData || !ChassisMesh || !SharedWheelMesh)
		{
			OutFailureReason = TEXT("Wagon migration preflight에 필요한 Asset 중 null이 있습니다.");
			return false;
		}

		if (!IsExactObjectPath(Recipe->TargetVehicleData.ToSoftObjectPath(), WagonTargetObjectPath))
		{
			OutFailureReason = FString::Printf(TEXT("Recipe TargetVehicleData가 audit baseline과 다릅니다: %s"), *Recipe->TargetVehicleData.ToSoftObjectPath().ToString());
			return false;
		}

		if (!IsExactObjectPath(Recipe->AssetIntent.ChassisMesh.ToSoftObjectPath(), WagonChassisObjectPath))
		{
			OutFailureReason = FString::Printf(TEXT("Recipe ChassisMesh가 audit baseline과 다릅니다: %s"), *Recipe->AssetIntent.ChassisMesh.ToSoftObjectPath().ToString());
			return false;
		}

		if (!Recipe->AssetIntent.WheelMeshFL.IsNull()
			|| !Recipe->AssetIntent.WheelMeshFR.IsNull()
			|| !Recipe->AssetIntent.WheelMeshRL.IsNull()
			|| !Recipe->AssetIntent.WheelMeshRR.IsNull())
		{
			OutFailureReason = TEXT("Recipe WheelMesh binding이 이미 audit baseline(null/null/null/null)에서 변경되었습니다.");
			return false;
		}

		if (Recipe->WheelVisualIntent.Mode != ECFWheelVisualIntentMode::UseProfilePolicy)
		{
			OutFailureReason = FString::Printf(TEXT("Recipe WheelVisualIntent.Mode가 audit baseline UseProfilePolicy가 아닙니다: %d"), static_cast<int32>(Recipe->WheelVisualIntent.Mode));
			return false;
		}

		if (TargetVehicleData->WheelVisualConfig.bUseWheelSocketScale
			|| TargetVehicleData->WheelVisualConfig.bAutoScaleWheelMeshToRadius
			|| TargetVehicleData->WheelVisualConfig.bUseWheelVisualOverrides)
		{
			OutFailureReason = TEXT("Target WheelVisualConfig가 audit baseline(false/false/false)에서 변경되었습니다.");
			return false;
		}

		if (TargetVehicleData->VehicleLayoutConfig.bUseLayoutOverrides)
		{
			OutFailureReason = TEXT("Target VehicleLayoutConfig가 이미 capture/apply된 상태입니다.");
			return false;
		}

		// canonical shared Wheel local bounds입니다.
		const FBoxSphereBounds WheelBounds = SharedWheelMesh->GetBounds();

		// canonical Wheel validation 실패 사유입니다.
		FString CanonicalWheelError;
		if (!FCFWheelSizeUtils::ValidateCanonicalWheelBounds(WheelBounds.Origin, WheelBounds.BoxExtent, CanonicalWheelError))
		{
			OutFailureReason = FString::Printf(TEXT("Shared Wheel canonical bounds validation 실패: %s"), *CanonicalWheelError);
			return false;
		}

		// FL/FR/RL/RR Socket별 파생 size 검증 결과입니다.
		FCFDerivedWheelSize DerivedWheelSizes[UE_ARRAY_COUNT(ExpectedWheelSockets)];
		for (int32 SocketIndex = 0; SocketIndex < UE_ARRAY_COUNT(ExpectedWheelSockets); ++SocketIndex)
		{
			if (!ValidateExpectedSocket(
				ChassisMesh,
				ExpectedWheelSockets[SocketIndex],
				DerivedWheelSizes[SocketIndex],
				WheelBounds.BoxExtent,
				OutFailureReason))
			{
				return false;
			}
		}

		if (!FCFWheelSizeUtils::AreWheelSizesCompatible(DerivedWheelSizes[0], DerivedWheelSizes[1])
			|| !FCFWheelSizeUtils::AreWheelSizesCompatible(DerivedWheelSizes[2], DerivedWheelSizes[3]))
		{
			OutFailureReason = TEXT("Wagon front/rear axle 좌우 Socket-derived Wheel Size가 일치하지 않습니다.");
			return false;
		}

		return true;
	}

	// Vehicle Browser에서 exact Wagon Recipe/Target row만 찾습니다.
	const FCFVehicleListEntry* FindExactWagonEntry(const TArray<FCFVehicleListEntry>& Entries)
	{
		return Entries.FindByPredicate([](const FCFVehicleListEntry& Entry)
		{
			return Entry.DefinitionPath.ToString() == WagonTargetObjectPath
				&& Entry.RecipePath.ToString() == WagonRecipeObjectPath;
		});
	}

	// 현재 measurement 목록에서 exact canonical field proposal을 찾습니다.
	const FCFVehicleMeasurementProposal* FindMeasurementProposal(
		const FCFVehicleMeasurementReadResult& MeasurementResult,
		const TCHAR* CanonicalFieldPath)
	{
		return MeasurementResult.Proposals.FindByPredicate([CanonicalFieldPath](const FCFVehicleMeasurementProposal& Proposal)
		{
			return Proposal.FieldPath.ToCanonicalString(true) == CanonicalFieldPath;
		});
	}

	// USER가 승인한 Socket-derived Wheel measurement 하나를 exact proposal/fingerprint로 채택합니다.
	bool AdoptExpectedMeasurement(
		FCFVehicleAuthoringVM& ViewModel,
		const FExpectedMeasurement& ExpectedMeasurement,
		FString& OutFailureReason)
	{
		// 이전 Recipe commit 뒤 current asset/socket truth로 다시 만든 measurement 목록입니다.
		FString RefreshError;
		if (!ViewModel.RefreshMeasurementProposals(RefreshError))
		{
			OutFailureReason = FString::Printf(TEXT("Measurement refresh 실패: %s"), *RefreshError);
			return false;
		}

		// exact target field에 대응하는 current Resolver proposal입니다.
		const FCFVehicleMeasurementProposal* Proposal = FindMeasurementProposal(ViewModel.GetMeasurementResult(), ExpectedMeasurement.FieldPath);
		if (!Proposal)
		{
			OutFailureReason = FString::Printf(TEXT("필수 Socket-derived proposal이 없습니다: %s"), ExpectedMeasurement.FieldPath);
			return false;
		}

		if (Proposal->MeasurementRuleId != ExpectedMeasurement.ExpectedRuleId)
		{
			OutFailureReason = FString::Printf(
				TEXT("Measurement rule mismatch: %s current=%s expected=%s"),
				ExpectedMeasurement.FieldPath,
				*Proposal->MeasurementRuleId.ToString(),
				*ExpectedMeasurement.ExpectedRuleId.ToString());
			return false;
		}

		// Resolver canonical text에서 읽은 현재 proposed numeric value입니다.
		const float ProposedValue = FCString::Atof(*Proposal->MeasuredCandidateValue.CanonicalValueText);
		if (!FMath::IsNearlyEqual(ProposedValue, ExpectedMeasurement.ExpectedValue, FCFWheelSizeUtils::DefaultSizeCompatibilityToleranceCm))
		{
			OutFailureReason = FString::Printf(
				TEXT("Measurement value mismatch: %s current=%.4f expected=%.4f"),
				ExpectedMeasurement.FieldPath,
				ProposedValue,
				ExpectedMeasurement.ExpectedValue);
			return false;
		}

		// exact proposal에 OwnershipWrite approval을 붙이기 전 mutation0 preview입니다.
		FCFVehicleMeasurementPreviewResult Preview;
		if (!ViewModel.PrepareMeasurementDecision(*Proposal, ECFVehicleMeasureDecision::AcceptMeasuredValue, Preview))
		{
			OutFailureReason = Preview.Operation.Message.IsEmpty()
				? FString::Printf(TEXT("Measurement preview 실패: %s"), ExpectedMeasurement.FieldPath)
				: Preview.Operation.Message;
			return false;
		}

		// USER가 승인한 exact Socket-derived proposal을 기존 R2 Recipe-only lane으로 commit한 결과입니다.
		FCFAuthoringOpResult CommitResult;
		if (!ViewModel.ExecutePreparedMeasurement(CommitResult))
		{
			OutFailureReason = CommitResult.Message.IsEmpty()
				? FString::Printf(TEXT("Measurement commit 실패: %s"), ExpectedMeasurement.FieldPath)
				: CommitResult.Message;
			return false;
		}

		return true;
	}

	// Apply 뒤 Target이 WSA-P0-05 기대 상태를 exact하게 materialize했는지 검증합니다.
	bool VerifyAppliedTarget(
		const UCFVehicleData* TargetVehicleData,
		const UStaticMesh* ChassisMesh,
		const UStaticMesh* SharedWheelMesh,
		FString& OutFailureReason)
	{
		if (!TargetVehicleData || !ChassisMesh || !SharedWheelMesh)
		{
			OutFailureReason = TEXT("Post-Apply verification 대상이 null입니다.");
			return false;
		}

		if (TargetVehicleData->VehicleVisualConfig.ChassisMesh != ChassisMesh
			|| TargetVehicleData->VehicleVisualConfig.WheelMeshFL != SharedWheelMesh)
		{
			OutFailureReason = TEXT("Post-Apply Chassis/WheelMeshFL materialization이 기대값과 다릅니다.");
			return false;
		}

		if (TargetVehicleData->VehicleVisualConfig.WheelMeshFR
			|| TargetVehicleData->VehicleVisualConfig.WheelMeshRL
			|| TargetVehicleData->VehicleVisualConfig.WheelMeshRR)
		{
			OutFailureReason = TEXT("Post-Apply optional FR/RL/RR Wheel Mesh가 null fallback 계약을 벗어났습니다.");
			return false;
		}

		if (!TargetVehicleData->WheelVisualConfig.bUseWheelSocketScale
			|| TargetVehicleData->WheelVisualConfig.bAutoScaleWheelMeshToRadius)
		{
			OutFailureReason = TEXT("Post-Apply WheelVisual SocketScale/AutoScale flag가 기대 계약과 다릅니다.");
			return false;
		}

		if (!TargetVehicleData->VehicleLayoutConfig.bUseLayoutOverrides)
		{
			OutFailureReason = TEXT("Post-Apply VehicleLayoutConfig.bUseLayoutOverrides가 true가 아닙니다.");
			return false;
		}

		// Post-Apply persisted Wheel Anchor pose를 role 순서대로 확인할 포인터입니다.
		const FCFWheelAnchorPose* AppliedWheelAnchors[] =
		{
			&TargetVehicleData->VehicleLayoutConfig.WheelAnchorFL,
			&TargetVehicleData->VehicleLayoutConfig.WheelAnchorFR,
			&TargetVehicleData->VehicleLayoutConfig.WheelAnchorRL,
			&TargetVehicleData->VehicleLayoutConfig.WheelAnchorRR
		};

		for (int32 SocketIndex = 0; SocketIndex < UE_ARRAY_COUNT(ExpectedWheelSockets); ++SocketIndex)
		{
			if (!AppliedWheelAnchors[SocketIndex]->RelativeLocation.Equals(ExpectedWheelSockets[SocketIndex].RelativeLocation, KINDA_SMALL_NUMBER)
				|| !AppliedWheelAnchors[SocketIndex]->RelativeRotation.Equals(FRotator::ZeroRotator, KINDA_SMALL_NUMBER)
				|| !AppliedWheelAnchors[SocketIndex]->RelativeScale.Equals(ExpectedSocketScale, KINDA_SMALL_NUMBER))
			{
				OutFailureReason = FString::Printf(TEXT("Post-Apply WheelAnchor pose mismatch: %s"), ExpectedWheelSockets[SocketIndex].SocketName);
				return false;
			}
		}

		const FCFVehicleMovementConfig& Movement = TargetVehicleData->VehicleMovementConfig;
		if (!FMath::IsNearlyEqual(Movement.FrontWheelRadius, ExpectedWheelRadiusCm, FCFWheelSizeUtils::DefaultSizeCompatibilityToleranceCm)
			|| !FMath::IsNearlyEqual(Movement.RearWheelRadius, ExpectedWheelRadiusCm, FCFWheelSizeUtils::DefaultSizeCompatibilityToleranceCm)
			|| !FMath::IsNearlyEqual(Movement.FrontWheelWidth, ExpectedWheelWidthCm, FCFWheelSizeUtils::DefaultSizeCompatibilityToleranceCm)
			|| !FMath::IsNearlyEqual(Movement.RearWheelWidth, ExpectedWheelWidthCm, FCFWheelSizeUtils::DefaultSizeCompatibilityToleranceCm))
		{
			OutFailureReason = FString::Printf(
				TEXT("Post-Apply derived Wheel geometry mismatch: FR=%.4f RR=%.4f FW=%.4f RW=%.4f"),
				Movement.FrontWheelRadius,
				Movement.RearWheelRadius,
				Movement.FrontWheelWidth,
				Movement.RearWheelWidth);
			return false;
		}

		return true;
	}

	// 기존 DataAsset 하나를 자기 package의 exact .uasset 파일에 저장합니다.
	bool SaveExistingAsset(UObject* AssetObject, FString& OutFailureReason)
	{
		if (!AssetObject)
		{
			OutFailureReason = TEXT("저장할 Asset이 null입니다.");
			return false;
		}

		// Asset을 소유한 exact package입니다.
		UPackage* AssetPackage = AssetObject->GetOutermost();
		if (!AssetPackage)
		{
			OutFailureReason = FString::Printf(TEXT("Asset package를 찾을 수 없습니다: %s"), *AssetObject->GetPathName());
			return false;
		}

		// Long package name을 실제 .uasset 경로로 변환합니다.
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			AssetPackage->GetName(),
			FPackageName::GetAssetPackageExtension());

		// Existing public standalone DataAsset의 저장 옵션입니다.
		FSavePackageArgs SavePackageArgs;
		SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SavePackageArgs.SaveFlags = SAVE_NoError;

		AssetObject->MarkPackageDirty();
		if (!UPackage::SavePackage(AssetPackage, AssetObject, *PackageFilename, SavePackageArgs))
		{
			OutFailureReason = FString::Printf(TEXT("Package save 실패: %s"), *PackageFilename);
			return false;
		}

		return true;
	}

	// 두 Wagon package의 기존 디스크 바이트를 Save 전에 보존합니다.
	bool BackupExistingPackage(UObject* AssetObject, FString& OutPackageFilename, TArray<uint8>& OutOriginalBytes, FString& OutFailureReason)
	{
		if (!AssetObject || !AssetObject->GetOutermost())
		{
			OutFailureReason = TEXT("Package backup 대상 Asset 또는 package가 null입니다.");
			return false;
		}

		OutPackageFilename = FPackageName::LongPackageNameToFilename(
			AssetObject->GetOutermost()->GetName(),
			FPackageName::GetAssetPackageExtension());

		if (!FFileHelper::LoadFileToArray(OutOriginalBytes, *OutPackageFilename))
		{
			OutFailureReason = FString::Printf(TEXT("기존 package backup read 실패: %s"), *OutPackageFilename);
			return false;
		}

		return true;
	}

	// Recipe-only Save 실패 시 migration 직전 파일 바이트로 복원합니다.
	bool SaveRecipeOnlyWithRollback(UObject* RecipeAsset, FString& OutFailureReason)
	{
		// Recipe package의 migration 직전 파일 경로입니다.
		FString RecipeFilename;
		// Recipe package의 migration 직전 원본 바이트입니다.
		TArray<uint8> RecipeOriginalBytes;
		if (!BackupExistingPackage(RecipeAsset, RecipeFilename, RecipeOriginalBytes, OutFailureReason))
		{
			return false;
		}

		FString SaveFailureReason;
		if (SaveExistingAsset(RecipeAsset, SaveFailureReason))
		{
			return true;
		}

		if (!FFileHelper::SaveArrayToFile(RecipeOriginalBytes, *RecipeFilename))
		{
			OutFailureReason = FString::Printf(TEXT("%s / Recipe rollback 실패: %s"), *SaveFailureReason, *RecipeFilename);
			return false;
		}

		OutFailureReason = FString::Printf(TEXT("%s / Recipe 원상복구 완료"), *SaveFailureReason);
		return false;
	}

	// Save 중간 실패 시 두 Wagon package를 migration 직전 바이트로 복원합니다.
	bool RestorePackagePair(
		const FString& TargetFilename,
		const TArray<uint8>& TargetOriginalBytes,
		const FString& RecipeFilename,
		const TArray<uint8>& RecipeOriginalBytes,
		FString& OutFailureReason)
	{
		const bool bTargetRestored = FFileHelper::SaveArrayToFile(TargetOriginalBytes, *TargetFilename);
		const bool bRecipeRestored = FFileHelper::SaveArrayToFile(RecipeOriginalBytes, *RecipeFilename);

		if (!bTargetRestored || !bRecipeRestored)
		{
			OutFailureReason = FString::Printf(
				TEXT("Migration Save rollback 실패: target=%s recipe=%s"),
				bTargetRestored ? TEXT("restored") : TEXT("FAILED"),
				bRecipeRestored ? TEXT("restored") : TEXT("FAILED"));
			return false;
		}

		return true;
	}

	// 두 Wagon package를 모두 저장하거나, 어느 한쪽이라도 실패하면 두 파일 모두 원상복구합니다.
	bool SaveMigrationPackagePair(UObject* TargetAsset, UObject* RecipeAsset, FString& OutFailureReason)
	{
		// Target package의 migration 직전 파일 경로입니다.
		FString TargetFilename;

		// Target package의 migration 직전 원본 바이트입니다.
		TArray<uint8> TargetOriginalBytes;

		// Recipe package의 migration 직전 파일 경로입니다.
		FString RecipeFilename;

		// Recipe package의 migration 직전 원본 바이트입니다.
		TArray<uint8> RecipeOriginalBytes;

		if (!BackupExistingPackage(TargetAsset, TargetFilename, TargetOriginalBytes, OutFailureReason)
			|| !BackupExistingPackage(RecipeAsset, RecipeFilename, RecipeOriginalBytes, OutFailureReason))
		{
			return false;
		}

		FString SaveFailureReason;
		if (SaveExistingAsset(TargetAsset, SaveFailureReason)
			&& SaveExistingAsset(RecipeAsset, SaveFailureReason))
		{
			return true;
		}

		FString RollbackFailureReason;
		if (!RestorePackagePair(
			TargetFilename,
			TargetOriginalBytes,
			RecipeFilename,
			RecipeOriginalBytes,
			RollbackFailureReason))
		{
			OutFailureReason = FString::Printf(
				TEXT("%s / %s"),
				*SaveFailureReason,
				*RollbackFailureReason);
			return false;
		}

		OutFailureReason = FString::Printf(TEXT("%s / 두 Wagon package 원상복구 완료"), *SaveFailureReason);
		return false;
	}
}

#if WITH_DEV_AUTOMATION_TESTS

// Deferred Apply test에서 exact RequiredProfileMissing issue 하나를 만듭니다.
static FCFVehicleValidationIssue MakeDeferredProfileMissingIssue(const TCHAR* ProfileLabel)
{
	// 반환할 exact Recipe blocker issue입니다.
	FCFVehicleValidationIssue Issue;
	Issue.Severity = ECFVehicleValidationSeverity::Blocked;
	Issue.IssueCode = TEXT("RequiredProfileMissing");
	Issue.Message = FString::Printf(TEXT("Managed Resolve에 필요한 %s Profile Snapshot이 없습니다."), ProfileLabel);
	return Issue;
}

// Deferred Apply test에서 exact WheelClass RequiredRefs issue 하나를 만듭니다.
static FCFVehicleValidationIssue MakeDeferredWheelClassMissingIssue(const TCHAR* ValidatorFieldPath)
{
	// 반환할 exact Definition blocker issue입니다.
	FCFVehicleValidationIssue Issue;
	Issue.Severity = ECFVehicleValidationSeverity::Error;
	Issue.IssueCode = TEXT("Definition.RequiredRefs");
	Issue.ValidatorFieldPath = ValidatorFieldPath;
	Issue.Message = FString::Printf(TEXT("Automation required reference missing: %s"), ValidatorFieldPath);
	return Issue;
}

// Wagon Step 2~4에서 허용되는 exact deferred blocker fixture를 만듭니다.
static FCFVehicleValidationReadResult BuildExpectedDeferredValidationFixture()
{
	// 반환할 validation readback fixture입니다.
	FCFVehicleValidationReadResult ValidationResult;
	ValidationResult.RecipeValidation.Add(MakeDeferredProfileMissingIssue(TEXT("Vehicle Base")));
	ValidationResult.RecipeValidation.Add(MakeDeferredProfileMissingIssue(TEXT("Drivetrain")));
	ValidationResult.RecipeValidation.Add(MakeDeferredProfileMissingIssue(TEXT("Handling")));
	ValidationResult.RecipeValidation.Add(MakeDeferredProfileMissingIssue(TEXT("Performance")));
	ValidationResult.DefinitionValidation.Add(MakeDeferredWheelClassMissingIssue(TEXT("VehicleReferenceConfig.FrontWheelClass")));
	ValidationResult.DefinitionValidation.Add(MakeDeferredWheelClassMissingIssue(TEXT("VehicleReferenceConfig.RearWheelClass")));
	return ValidationResult;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFWSAMigrateDeferredApplyGuardTest,
	"CarFight.DataAuthoring.CF_FQ_040.WSA_P0_05.Migration.DeferredApplyGuard",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// Deferred Apply가 exact Wagon Step 2~4 blocker에서만 열리고 유사/중복/부분 구성은 모두 fail-closed하는지 검증합니다.
bool FCFWSAMigrateDeferredApplyGuardTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	// Profile binding이 모두 비어 있는 transient Wagon Recipe입니다.
	UCFVehicleRecipeData* Recipe = NewObject<UCFVehicleRecipeData>(GetTransientPackage(), NAME_None, RF_Transient);
	if (!TestNotNull(TEXT("Deferred Apply Recipe fixture exists"), Recipe))
	{
		return false;
	}

	// Expected Step 2~4 validation signature입니다.
	const FCFVehicleValidationReadResult ExpectedValidation = BuildExpectedDeferredValidationFixture();
	// Guard가 반환하는 실패 사유입니다.
	FString FailureReason;
	TestTrue(TEXT("Exact four core Profiles plus Front/Rear WheelClass blockers allow defer"), CFWSAMigrate::IsExpectedDeferredApplyValidation(Recipe, ExpectedValidation, FailureReason));

	// Performance 누락 대신 DriveState 누락이 끼어든 잘못된 signature입니다.
	FCFVehicleValidationReadResult DriveStateSubstitution = ExpectedValidation;
	DriveStateSubstitution.RecipeValidation[3] = MakeDeferredProfileMissingIssue(TEXT("DriveState"));
	FailureReason.Reset();
	TestFalse(TEXT("DriveState cannot substitute for Performance blocker"), CFWSAMigrate::IsExpectedDeferredApplyValidation(Recipe, DriveStateSubstitution, FailureReason));

	// Vehicle Base가 중복되고 Performance가 빠진 잘못된 signature입니다.
	FCFVehicleValidationReadResult DuplicateProfile = ExpectedValidation;
	DuplicateProfile.RecipeValidation[3] = MakeDeferredProfileMissingIssue(TEXT("Vehicle Base"));
	FailureReason.Reset();
	TestFalse(TEXT("Duplicate core Profile blocker is rejected"), CFWSAMigrate::IsExpectedDeferredApplyValidation(Recipe, DuplicateProfile, FailureReason));

	// Resolver 자체에 예상 밖 blocker가 추가된 signature입니다.
	FCFVehicleValidationReadResult ResolverBlocked = ExpectedValidation;
	// 예상 밖 Resolver blocker issue입니다.
	FCFVehicleValidationIssue UnexpectedResolverIssue;
	UnexpectedResolverIssue.Severity = ECFVehicleValidationSeverity::Blocked;
	UnexpectedResolverIssue.IssueCode = TEXT("AutomationUnexpectedResolverBlocker");
	UnexpectedResolverIssue.Message = TEXT("Automation unexpected resolver blocker");
	ResolverBlocked.ResolverValidation.Add(UnexpectedResolverIssue);
	FailureReason.Reset();
	TestFalse(TEXT("Unexpected Resolver blocker prevents defer"), CFWSAMigrate::IsExpectedDeferredApplyValidation(Recipe, ResolverBlocked, FailureReason));

	// 이미 core Profile binding 하나가 생긴 부분 구성 Recipe입니다.
	Recipe->ProfileBindings.VehicleBaseProfile = TSoftObjectPtr<UCFVehicleBaseProfile>(FSoftObjectPath(TEXT("/Game/Test/P_Base.P_Base")));
	FailureReason.Reset();
	TestFalse(TEXT("Existing core Profile binding prevents Step 2~4 defer"), CFWSAMigrate::IsExpectedDeferredApplyValidation(Recipe, ExpectedValidation, FailureReason));
	Recipe->ProfileBindings.VehicleBaseProfile.Reset();

	// Front WheelClass RequiredRefs가 중복된 잘못된 Definition signature입니다.
	FCFVehicleValidationReadResult DuplicateWheelClass = ExpectedValidation;
	DuplicateWheelClass.DefinitionValidation.Add(MakeDeferredWheelClassMissingIssue(TEXT("VehicleReferenceConfig.FrontWheelClass")));
	FailureReason.Reset();
	TestFalse(TEXT("Duplicate WheelClass blocker is rejected"), CFWSAMigrate::IsExpectedDeferredApplyValidation(Recipe, DuplicateWheelClass, FailureReason));

	return true;
}

#endif

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFWSAMigrateCommandlet::UCFWSAMigrateCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// Wagon exact current truth를 검증하고 typed migration 뒤 Step 2~4면 Recipe-only 저장, Apply-ready면 Definition Apply + 두 패키지 저장을 수행합니다.
int32 UCFWSAMigrateCommandlet::Main(const FString& Params)
{
	(void)Params;

	// 이번 one-shot에서 실패 원인을 persistent Save 전까지 누적할 문자열입니다.
	FString FailureReason;

	// USER 승인 대상 Wagon Recipe입니다.
	UCFVehicleRecipeData* WagonRecipe = LoadObject<UCFVehicleRecipeData>(nullptr, CFWSAMigrate::WagonRecipeObjectPath);

	// USER 승인 대상 Wagon runtime canonical VehicleData입니다.
	UCFVehicleData* WagonTarget = LoadObject<UCFVehicleData>(nullptr, CFWSAMigrate::WagonTargetObjectPath);

	// USER가 Socket 위치/Scale을 authoring한 Wagon Chassis입니다.
	UStaticMesh* WagonChassis = LoadObject<UStaticMesh>(nullptr, CFWSAMigrate::WagonChassisObjectPath);

	// WSA canonical source로 사용할 shared FL Wheel입니다.
	UStaticMesh* SharedWheel = LoadObject<UStaticMesh>(nullptr, CFWSAMigrate::SharedWheelObjectPath);

	if (!WagonRecipe || !WagonTarget || !WagonChassis || !SharedWheel)
	{
		UE_LOG(
			LogCFWSAMigrate,
			Error,
			TEXT("CF_WSA_MIGRATE_LOAD_FAIL recipe=%p target=%p chassis=%p wheel=%p"),
			WagonRecipe,
			WagonTarget,
			WagonChassis,
			SharedWheel);
		return static_cast<int32>(CFWSAMigrate::EExitCode::AssetLoadFailed);
	}

	if (!CFWSAMigrate::ValidatePreflight(WagonRecipe, WagonTarget, WagonChassis, SharedWheel, FailureReason))
	{
		UE_LOG(LogCFWSAMigrate, Error, TEXT("CF_WSA_MIGRATE_PREFLIGHT_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFWSAMigrate::EExitCode::PreflightFailed);
	}

	// Existing Data Authoring typed facade만 사용할 transient Workspace VM입니다.
	FCFVehicleAuthoringVM ViewModel;

	// Asset Registry 기반 exact Wagon row를 찾기 위한 Browser refresh 실패 사유입니다.
	FString BrowserError;
	if (!ViewModel.RefreshBrowser(TEXT("Wagon"), BrowserError))
	{
		UE_LOG(LogCFWSAMigrate, Error, TEXT("CF_WSA_MIGRATE_BROWSER_FAIL %s"), *BrowserError);
		return static_cast<int32>(CFWSAMigrate::EExitCode::SelectionFailed);
	}

	// Wagon Target+Recipe exact identity를 동시에 만족하는 managed row입니다.
	const FCFVehicleListEntry* WagonEntry = CFWSAMigrate::FindExactWagonEntry(ViewModel.GetBrowserEntries());
	if (!WagonEntry)
	{
		UE_LOG(LogCFWSAMigrate, Error, TEXT("CF_WSA_MIGRATE_SELECTION_FAIL exact Wagon managed row missing"));
		return static_cast<int32>(CFWSAMigrate::EExitCode::SelectionFailed);
	}

	// 현재 baseline은 Wheel geometry 미채택 등으로 Preview가 Blocked일 수 있으므로 selection 자체와 exact UObject binding을 분리해 확인합니다.
	FString SelectionError;
	const bool bInitialPreviewFresh = ViewModel.SelectVehicle(*WagonEntry, SelectionError);
	if (!ViewModel.HasSelection()
		|| ViewModel.GetRecipe() != WagonRecipe
		|| ViewModel.GetTargetVehicleData() != WagonTarget)
	{
		UE_LOG(LogCFWSAMigrate, Error, TEXT("CF_WSA_MIGRATE_SELECTION_FAIL %s"), *SelectionError);
		return static_cast<int32>(CFWSAMigrate::EExitCode::SelectionFailed);
	}

	UE_LOG(
		LogCFWSAMigrate,
		Display,
		TEXT("CF_WSA_MIGRATE_SELECTION_OK initial_preview_fresh=%s"),
		bInitialPreviewFresh ? TEXT("true") : TEXT("false"));

	// Step 2 typed AssetIntent에서 Chassis/socket binding은 보존하고 canonical shared Wheel FL만 지정합니다.
	FCFVehicleAssetIntent MigratedAssetIntent = WagonRecipe->AssetIntent;
	MigratedAssetIntent.WheelMeshFL = SharedWheel;
	MigratedAssetIntent.WheelMeshFR.Reset();
	MigratedAssetIntent.WheelMeshRL.Reset();
	MigratedAssetIntent.WheelMeshRR.Reset();

	// Existing R1 semantic AssetIntent commit 결과입니다.
	FCFAuthoringOpResult AssetIntentResult;
	if (!ViewModel.CommitAssetIntent(MigratedAssetIntent, AssetIntentResult))
	{
		FailureReason = AssetIntentResult.Message.IsEmpty() ? TEXT("typed AssetIntent commit 실패") : AssetIntentResult.Message;
		UE_LOG(LogCFWSAMigrate, Error, TEXT("CF_WSA_MIGRATE_ASSET_INTENT_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFWSAMigrate::EExitCode::AssetIntentCommitFailed);
	}

	// SocketScaleFromChassis를 exact semantic desired state로 표현하는 R1 command입니다.
	FCFVehicleSemanticChange WheelIntentChange;
	WheelIntentChange.Operation = ECFVehicleSemanticOp::SetWheelVisualIntent;
	WheelIntentChange.WheelVisualIntent.Mode = ECFWheelVisualIntentMode::SocketScaleFromChassis;

	// Existing R1 semantic WheelVisualIntent commit 결과입니다.
	FCFAuthoringOpResult WheelIntentResult;
	if (!ViewModel.CommitSemanticChange(WheelIntentChange, WheelIntentResult))
	{
		FailureReason = WheelIntentResult.Message.IsEmpty() ? TEXT("typed WheelVisualIntent commit 실패") : WheelIntentResult.Message;
		UE_LOG(LogCFWSAMigrate, Error, TEXT("CF_WSA_MIGRATE_WHEEL_INTENT_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFWSAMigrate::EExitCode::WheelIntentCommitFailed);
	}

	// Socket-derived Front/Rear Radius/Width 네 exact proposal을 USER 승인값으로 한 건씩 review/commit합니다.
	for (const CFWSAMigrate::FExpectedMeasurement& ExpectedMeasurement : CFWSAMigrate::ExpectedMeasurements)
	{
		if (!CFWSAMigrate::AdoptExpectedMeasurement(ViewModel, ExpectedMeasurement, FailureReason))
		{
			UE_LOG(
				LogCFWSAMigrate,
				Error,
				TEXT("CF_WSA_MIGRATE_MEASUREMENT_FAIL field=%s reason=%s"),
				ExpectedMeasurement.FieldPath,
				*FailureReason);
			return static_cast<int32>(CFWSAMigrate::EExitCode::MeasurementFailed);
		}
	}

	// 네 measurement adoption 뒤 full Resolver/Diff/Trace/Validator를 fresh하게 다시 읽습니다.
	FString PreviewError;
	if (!ViewModel.RefreshPreview(PreviewError))
	{
		// Preview 실패 시점의 complete structured validation readback입니다.
		const FCFVehicleValidationReadResult& ValidationResult = ViewModel.GetValidationResult();
		// 사람이 빠르게 상태를 판정할 aggregate summary입니다.
		const FCFAuthoringValidationSummary& ValidationSummary = ValidationResult.Operation.ValidationSummary;

		CFWSAMigrate::LogValidationBucket(TEXT("Recipe"), ValidationResult.RecipeValidation);
		CFWSAMigrate::LogValidationBucket(TEXT("Resolver"), ValidationResult.ResolverValidation);
		CFWSAMigrate::LogValidationBucket(TEXT("Definition"), ValidationResult.DefinitionValidation);

		// 현재 실제 Wagon E2E가 아직 Step 2~4라 후속 Profile/WheelClass만 미완성인지 fail-closed로 판정합니다.
		FString DeferredApplyReason;
		if (!CFWSAMigrate::IsExpectedDeferredApplyValidation(WagonRecipe, ValidationResult, DeferredApplyReason))
		{
			UE_LOG(
				LogCFWSAMigrate,
				Error,
				TEXT("CF_WSA_MIGRATE_PREVIEW_FAIL error=%s errors=%d blocked=%d warnings=%d reason=%s"),
				*PreviewError,
				ValidationSummary.ErrorCount,
				ValidationSummary.BlockedCount,
				ValidationSummary.WarningCount,
				*DeferredApplyReason);
			return static_cast<int32>(CFWSAMigrate::EExitCode::PreviewValidationFailed);
		}

		if (!CFWSAMigrate::VerifyMigratedRecipe(WagonRecipe, SharedWheel, FailureReason)
			|| !CFWSAMigrate::VerifyTargetStillUnapplied(WagonTarget, FailureReason))
		{
			UE_LOG(LogCFWSAMigrate, Error, TEXT("CF_WSA_MIGRATE_DEFERRED_VERIFY_FAIL %s"), *FailureReason);
			return static_cast<int32>(CFWSAMigrate::EExitCode::PostApplyVerificationFailed);
		}

		// Step 2~4에서 승인된 WSA authoring truth만 Recipe에 저장합니다. Target Definition은 Step 7 전까지 저장하지 않습니다.
		if (!CFWSAMigrate::SaveRecipeOnlyWithRollback(WagonRecipe, FailureReason))
		{
			UE_LOG(LogCFWSAMigrate, Error, TEXT("CF_WSA_MIGRATE_RECIPE_SAVE_FAIL %s"), *FailureReason);
			return static_cast<int32>(CFWSAMigrate::EExitCode::SaveFailed);
		}

		UE_LOG(
			LogCFWSAMigrate,
			Display,
			TEXT("CF_WSA_MIGRATE_RECIPE_ONLY_PASS recipe=%s wheel=%s socket_scale=%s target_apply=deferred_to_builder_step7 saved_packages=1"),
			*WagonRecipe->GetPathName(),
			*SharedWheel->GetPathName(),
			*CFWSAMigrate::ExpectedSocketScale.ToString());
		return static_cast<int32>(CFWSAMigrate::EExitCode::Success);
	}

	// Current fresh exact Diff를 DefinitionApply approval로 묶는 결과입니다.
	FCFAuthoringOpResult PrepareApplyResult;
	if (!ViewModel.PrepareApply(PrepareApplyResult))
	{
		FailureReason = PrepareApplyResult.Message.IsEmpty() ? TEXT("Definition Apply 준비 실패") : PrepareApplyResult.Message;
		UE_LOG(LogCFWSAMigrate, Error, TEXT("CF_WSA_MIGRATE_APPLY_PREPARE_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFWSAMigrate::EExitCode::ApplyFailed);
	}

	// Existing R3 ApplyResolvedVehicle → FCFVehicleApplyService shared writer 결과입니다.
	FCFAuthoringOpResult ApplyResult;
	if (!ViewModel.ExecutePreparedApply(ApplyResult))
	{
		FailureReason = ApplyResult.Message.IsEmpty() ? TEXT("Definition Apply 실행 실패") : ApplyResult.Message;
		UE_LOG(LogCFWSAMigrate, Error, TEXT("CF_WSA_MIGRATE_APPLY_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFWSAMigrate::EExitCode::ApplyFailed);
	}

	if (!CFWSAMigrate::VerifyAppliedTarget(WagonTarget, WagonChassis, SharedWheel, FailureReason))
	{
		UE_LOG(LogCFWSAMigrate, Error, TEXT("CF_WSA_MIGRATE_POST_APPLY_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFWSAMigrate::EExitCode::PostApplyVerificationFailed);
	}

	// Target/Recipe 두 package를 모두 저장하거나 실패 시 둘 다 migration 직전 바이트로 복원합니다.
	if (!CFWSAMigrate::SaveMigrationPackagePair(WagonTarget, WagonRecipe, FailureReason))
	{
		UE_LOG(LogCFWSAMigrate, Error, TEXT("CF_WSA_MIGRATE_SAVE_PAIR_FAIL %s"), *FailureReason);
		return static_cast<int32>(CFWSAMigrate::EExitCode::SaveFailed);
	}

	UE_LOG(
		LogCFWSAMigrate,
		Display,
		TEXT("CF_WSA_MIGRATE_PASS recipe=%s target=%s wheel=%s radius=%.2f width=%.2f socket_scale=%s autoscale=false saved_packages=2"),
		*WagonRecipe->GetPathName(),
		*WagonTarget->GetPathName(),
		*SharedWheel->GetPathName(),
		CFWSAMigrate::ExpectedWheelRadiusCm,
		CFWSAMigrate::ExpectedWheelWidthCm,
		*CFWSAMigrate::ExpectedSocketScale.ToString());

	return static_cast<int32>(CFWSAMigrate::EExitCode::Success);
}
