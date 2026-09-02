// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderWagonReconcileCmdlet.cpp
// Version: v1.0.0
// Date: 2026-09-01
// Description: CF-FQ-040 actual Wagon의 WSA USER-approved SocketScale truth와 Vehicle Builder AssetAdoption을 post-closure 정합화하는 bounded one-shot 구현입니다.
// Changelog:
// - v1.0.0: exact Target USER PASS preflight, stale Chassis SocketScale 0.63→0.80 제한 복구, R2 Measurement 4건 fresh adoption, Resolver Success, Target hash invariant, Chassis+Recipe pair-save rollback을 최초 구현.
// Migration:
// - WSA 기능/정책/Runtime 코드는 변경하지 않습니다.
// - Wagon Chassis와 Recipe만 저장하고 Target VehicleData는 mutation/save하지 않습니다.
// - Generic Builder Measurement Preview/OwnershipWrite Commit 경계를 그대로 재사용합니다.
// - 실패 시 Save 전 종료하거나 저장 직전 Chassis+Recipe 바이트로 두 package를 함께 복원합니다.

#include "CFBuilderWagonReconcileCmdlet.h"

#include "CFVehicleData.h"
#include "CFWheelSizeUtils.h"
#include "DataAuthoring/CFVehicleAuthoringTypes.h"
#include "DataAuthoring/CFVehicleAuthoringVM.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Misc/FileHelper.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

DEFINE_LOG_CATEGORY_STATIC(LogCFBuilderWagonReconcile, Log, All);

namespace CFBuilderWagonReconcile
{
	// USER-approved actual Wagon Recipe exact object path입니다.
	const TCHAR* WagonRecipeObjectPath = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_Wagon.DA_Recipe_Wagon");

	// USER-approved actual Wagon runtime canonical VehicleData exact object path입니다.
	const TCHAR* WagonTargetObjectPath = TEXT("/Game/CarFight/Data/Authoring/DA_Vehicle_Wagon.DA_Vehicle_Wagon");

	// USER가 Wheel Socket을 authoring한 Wagon Chassis exact object path입니다.
	const TCHAR* WagonChassisObjectPath = TEXT("/Game/CarFight/Vehicles/Meshes/Wagon/Wagon.Wagon");

	// WSA canonical source로 사용하는 shared FL Wheel exact object path입니다.
	const TCHAR* SharedWheelObjectPath = TEXT("/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL.Wheel_FL");

	// WSA-P0-07 USER PASS가 확정한 actual Wagon Socket Scale입니다.
	const FVector ExpectedSocketScale(0.8, 1.0, 0.8);

	// Fresh audit에서 발견한 post-closure stale source Socket Scale입니다. 다른 값은 자동 교정하지 않습니다.
	const FVector KnownStaleSocketScale(0.63, 1.0, 0.63);

	// Canonical 100x25x100 Wheel + USER 0.80 scale의 기대 Radius입니다.
	const float ExpectedWheelRadiusCm = 40.0f;

	// Canonical 100x25x100 Wheel + USER Y=1.0 scale의 기대 Width입니다.
	const float ExpectedWheelWidthCm = 25.0f;

	// Commandlet 단계별 실패를 프로세스 종료 코드로 구분합니다.
	enum class EExitCode : int32
	{
		Success = 0,
		AssetLoadFailed = 61,
		PreflightFailed = 62,
		SocketRepairFailed = 63,
		SelectionFailed = 64,
		MeasurementFailed = 65,
		ResolveFailed = 66,
		VerificationFailed = 67,
		SaveFailed = 68
	};

	// USER-approved Wagon Wheel Socket의 exact location contract입니다.
	struct FExpectedWheelSocket
	{
		// StaticMesh Socket exact name입니다.
		const TCHAR* SocketName;

		// USER-approved local location입니다.
		FVector RelativeLocation;
	};

	// Actual Wagon role order의 USER-approved Socket location입니다.
	const FExpectedWheelSocket ExpectedWheelSockets[] =
	{
		{TEXT("Wheel_Anchor_FL"), FVector(152.0, -84.0, 17.0)},
		{TEXT("Wheel_Anchor_FR"), FVector(152.0, 84.0, 17.0)},
		{TEXT("Wheel_Anchor_RL"), FVector(-128.0, -84.0, 17.0)},
		{TEXT("Wheel_Anchor_RR"), FVector(-128.0, 84.0, 17.0)}
	};

	// Fresh R6 measurement 하나의 expected field/value/rule contract입니다.
	struct FExpectedMeasurement
	{
		// Resolver canonical field path입니다.
		const TCHAR* FieldPath;

		// USER-approved Socket truth에서 나와야 하는 numeric value입니다.
		float ExpectedValue;

		// WSA measurement rule입니다.
		FName ExpectedRuleId;
	};

	// Radius/Width 4개 exact measurement입니다.
	const FExpectedMeasurement ExpectedMeasurements[] =
	{
		{TEXT("VehicleMovementConfig.FrontWheelRadius"), ExpectedWheelRadiusCm, TEXT("WheelSocketScale.Radius.v1")},
		{TEXT("VehicleMovementConfig.RearWheelRadius"), ExpectedWheelRadiusCm, TEXT("WheelSocketScale.Radius.v1")},
		{TEXT("VehicleMovementConfig.FrontWheelWidth"), ExpectedWheelWidthCm, TEXT("WheelSocketScale.Width.v1")},
		{TEXT("VehicleMovementConfig.RearWheelWidth"), ExpectedWheelWidthCm, TEXT("WheelSocketScale.Width.v1")}
	};

	// Browser에서 exact actual Wagon managed row를 찾습니다.
	const FCFVehicleListEntry* FindExactWagonEntry(const TArray<FCFVehicleListEntry>& Entries)
	{
		return Entries.FindByPredicate([](const FCFVehicleListEntry& Entry)
		{
			return Entry.DefinitionPath.ToString() == WagonTargetObjectPath
				&& Entry.RecipePath.ToString() == WagonRecipeObjectPath;
		});
	}

	// Current measurement result에서 exact field proposal을 찾습니다.
	const FCFVehicleMeasurementProposal* FindMeasurementProposal(
		const FCFVehicleMeasurementReadResult& MeasurementResult,
		const TCHAR* CanonicalFieldPath)
	{
		return MeasurementResult.Proposals.FindByPredicate([CanonicalFieldPath](const FCFVehicleMeasurementProposal& Proposal)
		{
			return Proposal.FieldPath.ToCanonicalString(true) == CanonicalFieldPath;
		});
	}

	// UObject package를 disk bytes로 backup합니다.
	bool BackupExistingPackage(UObject* AssetObject, FString& OutFilename, TArray<uint8>& OutBytes, FString& OutFailureReason)
	{
		if (!AssetObject || !AssetObject->GetOutermost())
		{
			OutFailureReason = TEXT("Package backup 대상 Asset 또는 package가 null입니다.");
			return false;
		}

		OutFilename = FPackageName::LongPackageNameToFilename(
			AssetObject->GetOutermost()->GetName(),
			FPackageName::GetAssetPackageExtension());

		if (!FFileHelper::LoadFileToArray(OutBytes, *OutFilename))
		{
			OutFailureReason = FString::Printf(TEXT("기존 package backup read 실패: %s"), *OutFilename);
			return false;
		}

		return true;
	}

	// Existing asset 하나를 자기 package exact .uasset에 저장합니다.
	bool SaveExistingAsset(UObject* AssetObject, FString& OutFailureReason)
	{
		if (!AssetObject || !AssetObject->GetOutermost())
		{
			OutFailureReason = TEXT("저장할 Asset 또는 package가 null입니다.");
			return false;
		}

		UPackage* Package = AssetObject->GetOutermost();
		const FString Filename = FPackageName::LongPackageNameToFilename(
			Package->GetName(),
			FPackageName::GetAssetPackageExtension());

		FSavePackageArgs SaveArgs;
		SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SaveArgs.SaveFlags = SAVE_NoError;

		AssetObject->MarkPackageDirty();
		if (!UPackage::SavePackage(Package, AssetObject, *Filename, SaveArgs))
		{
			OutFailureReason = FString::Printf(TEXT("Package save 실패: %s"), *Filename);
			return false;
		}

		return true;
	}

	// Chassis+Recipe save 실패 시 두 파일을 모두 pre-reconcile bytes로 복원합니다.
	bool RestorePackagePair(
		const FString& ChassisFilename,
		const TArray<uint8>& ChassisBytes,
		const FString& RecipeFilename,
		const TArray<uint8>& RecipeBytes,
		FString& OutFailureReason)
	{
		const bool bChassisRestored = FFileHelper::SaveArrayToFile(ChassisBytes, *ChassisFilename);
		const bool bRecipeRestored = FFileHelper::SaveArrayToFile(RecipeBytes, *RecipeFilename);
		if (!bChassisRestored || !bRecipeRestored)
		{
			OutFailureReason = FString::Printf(
				TEXT("Reconcile rollback 실패: chassis=%s recipe=%s"),
				bChassisRestored ? TEXT("restored") : TEXT("FAILED"),
				bRecipeRestored ? TEXT("restored") : TEXT("FAILED"));
			return false;
		}
		return true;
	}

	// Chassis+Recipe 두 package를 모두 저장하거나 둘 중 하나라도 실패하면 둘 다 복원합니다.
	bool SavePackagePair(
		UStaticMesh* Chassis,
		UCFVehicleRecipeData* Recipe,
		const FString& ChassisFilename,
		const TArray<uint8>& ChassisBytes,
		const FString& RecipeFilename,
		const TArray<uint8>& RecipeBytes,
		FString& OutFailureReason)
	{
		FString SaveFailure;
		if (SaveExistingAsset(Chassis, SaveFailure) && SaveExistingAsset(Recipe, SaveFailure))
		{
			return true;
		}

		FString RollbackFailure;
		if (!RestorePackagePair(ChassisFilename, ChassisBytes, RecipeFilename, RecipeBytes, RollbackFailure))
		{
			OutFailureReason = FString::Printf(TEXT("%s / %s"), *SaveFailure, *RollbackFailure);
			return false;
		}

		OutFailureReason = FString::Printf(TEXT("%s / Chassis+Recipe 원상복구 완료"), *SaveFailure);
		return false;
	}

	// USER PASS된 Target이 0.80 SocketScale/40cm authority를 실제로 materialize한 상태인지 read-only 검증합니다.
	bool ValidateTargetUserPass(
		const UCFVehicleData* Target,
		const UStaticMesh* Chassis,
		const UStaticMesh* SharedWheel,
		FString& OutFailureReason)
	{
		if (!Target || !Chassis || !SharedWheel)
		{
			OutFailureReason = TEXT("Target preflight Asset이 null입니다.");
			return false;
		}

		if (Target->VehicleVisualConfig.ChassisMesh != Chassis
			|| Target->VehicleVisualConfig.WheelMeshFL != SharedWheel
			|| Target->VehicleVisualConfig.WheelMeshFR
			|| Target->VehicleVisualConfig.WheelMeshRL
			|| Target->VehicleVisualConfig.WheelMeshRR)
		{
			OutFailureReason = TEXT("Target Chassis/shared Wheel fallback binding이 WSA USER PASS 상태와 다릅니다.");
			return false;
		}

		if (!Target->WheelVisualConfig.bUseWheelSocketScale
			|| Target->WheelVisualConfig.bAutoScaleWheelMeshToRadius
			|| !Target->VehicleLayoutConfig.bUseLayoutOverrides)
		{
			OutFailureReason = TEXT("Target WheelVisual/Layout authority flag가 WSA USER PASS 상태와 다릅니다.");
			return false;
		}

		const FCFWheelAnchorPose* Anchors[] =
		{
			&Target->VehicleLayoutConfig.WheelAnchorFL,
			&Target->VehicleLayoutConfig.WheelAnchorFR,
			&Target->VehicleLayoutConfig.WheelAnchorRL,
			&Target->VehicleLayoutConfig.WheelAnchorRR
		};

		for (int32 Index = 0; Index < UE_ARRAY_COUNT(ExpectedWheelSockets); ++Index)
		{
			if (!Anchors[Index]->RelativeLocation.Equals(ExpectedWheelSockets[Index].RelativeLocation, KINDA_SMALL_NUMBER)
				|| !Anchors[Index]->RelativeRotation.Equals(FRotator::ZeroRotator, KINDA_SMALL_NUMBER)
				|| !Anchors[Index]->RelativeScale.Equals(ExpectedSocketScale, KINDA_SMALL_NUMBER))
			{
				OutFailureReason = FString::Printf(
					TEXT("Target WheelAnchor가 USER PASS 상태와 다릅니다: %s"),
					ExpectedWheelSockets[Index].SocketName);
				return false;
			}
		}

		const FCFVehicleMovementConfig& Movement = Target->VehicleMovementConfig;
		if (!FMath::IsNearlyEqual(Movement.FrontWheelRadius, ExpectedWheelRadiusCm, FCFWheelSizeUtils::DefaultSizeCompatibilityToleranceCm)
			|| !FMath::IsNearlyEqual(Movement.RearWheelRadius, ExpectedWheelRadiusCm, FCFWheelSizeUtils::DefaultSizeCompatibilityToleranceCm)
			|| !FMath::IsNearlyEqual(Movement.FrontWheelWidth, ExpectedWheelWidthCm, FCFWheelSizeUtils::DefaultSizeCompatibilityToleranceCm)
			|| !FMath::IsNearlyEqual(Movement.RearWheelWidth, ExpectedWheelWidthCm, FCFWheelSizeUtils::DefaultSizeCompatibilityToleranceCm))
		{
			OutFailureReason = FString::Printf(
				TEXT("Target Wheel geometry가 USER PASS 40/40/25/25와 다릅니다: FR=%.4f RR=%.4f FW=%.4f RW=%.4f"),
				Movement.FrontWheelRadius,
				Movement.RearWheelRadius,
				Movement.FrontWheelWidth,
				Movement.RearWheelWidth);
			return false;
		}

		return true;
	}

	// Chassis Socket location/rotation은 USER-approved 값을 보존하고 Scale drift만 exact known stale/current 상태로 판정합니다.
	bool InspectChassisSocketState(UStaticMesh* Chassis, bool& OutNeedsRepair, FString& OutFailureReason)
	{
		if (!Chassis)
		{
			OutFailureReason = TEXT("Chassis가 null입니다.");
			return false;
		}

		int32 ExpectedScaleCount = 0;
		int32 StaleScaleCount = 0;
		for (const FExpectedWheelSocket& ExpectedSocket : ExpectedWheelSockets)
		{
			UStaticMeshSocket* Socket = Chassis->FindSocket(FName(ExpectedSocket.SocketName));
			if (!Socket)
			{
				OutFailureReason = FString::Printf(TEXT("필수 Wheel Socket이 없습니다: %s"), ExpectedSocket.SocketName);
				return false;
			}

			if (!Socket->RelativeLocation.Equals(ExpectedSocket.RelativeLocation, KINDA_SMALL_NUMBER)
				|| !Socket->RelativeRotation.Equals(FRotator::ZeroRotator, KINDA_SMALL_NUMBER))
			{
				OutFailureReason = FString::Printf(
					TEXT("Socket location/rotation drift는 자동 복구하지 않습니다: %s"),
					ExpectedSocket.SocketName);
				return false;
			}

			if (Socket->RelativeScale.Equals(ExpectedSocketScale, KINDA_SMALL_NUMBER))
			{
				++ExpectedScaleCount;
			}
			else if (Socket->RelativeScale.Equals(KnownStaleSocketScale, KINDA_SMALL_NUMBER))
			{
				++StaleScaleCount;
			}
			else
			{
				OutFailureReason = FString::Printf(
					TEXT("알 수 없는 USER Socket Scale drift는 자동 복구하지 않습니다: %s current=%s"),
					ExpectedSocket.SocketName,
					*Socket->RelativeScale.ToString());
				return false;
			}
		}

		if (ExpectedScaleCount != UE_ARRAY_COUNT(ExpectedWheelSockets)
			&& StaleScaleCount != UE_ARRAY_COUNT(ExpectedWheelSockets))
		{
			OutFailureReason = TEXT("Wheel Socket Scale이 0.63/0.80 mixed 상태입니다. 자동 복구하지 않습니다.");
			return false;
		}

		OutNeedsRepair = StaleScaleCount == UE_ARRAY_COUNT(ExpectedWheelSockets);
		return true;
	}

	// Exact known stale 0.63 Socket Scale만 USER-approved 0.80으로 in-memory 복구합니다.
	bool RepairChassisSocketScale(UStaticMesh* Chassis, FString& OutFailureReason)
	{
		if (!Chassis)
		{
			OutFailureReason = TEXT("Socket repair Chassis가 null입니다.");
			return false;
		}

		Chassis->Modify();
		for (const FExpectedWheelSocket& ExpectedSocket : ExpectedWheelSockets)
		{
			UStaticMeshSocket* Socket = Chassis->FindSocket(FName(ExpectedSocket.SocketName));
			if (!Socket || !Socket->RelativeScale.Equals(KnownStaleSocketScale, KINDA_SMALL_NUMBER))
			{
				OutFailureReason = FString::Printf(
					TEXT("Socket repair 직전 known stale scale이 아닙니다: %s"),
					ExpectedSocket.SocketName);
				return false;
			}

			Socket->Modify();
			Socket->RelativeScale = ExpectedSocketScale;
		}

		Chassis->MarkPackageDirty();
		Chassis->PostEditChange();

		for (const FExpectedWheelSocket& ExpectedSocket : ExpectedWheelSockets)
		{
			const UStaticMeshSocket* Socket = Chassis->FindSocket(FName(ExpectedSocket.SocketName));
			if (!Socket || !Socket->RelativeScale.Equals(ExpectedSocketScale, KINDA_SMALL_NUMBER))
			{
				OutFailureReason = FString::Printf(
					TEXT("Socket repair readback 실패: %s"),
					ExpectedSocket.SocketName);
				return false;
			}
		}

		return true;
	}

	// Current fresh proposal 하나를 expected value/rule로 검증하고 기존 R2 reviewed measurement lane으로 commit합니다.
	bool AdoptExpectedMeasurement(
		FCFVehicleAuthoringVM& ViewModel,
		const FExpectedMeasurement& ExpectedMeasurement,
		FString& OutFailureReason)
	{
		FString RefreshError;
		if (!ViewModel.RefreshMeasurementProposals(RefreshError))
		{
			OutFailureReason = FString::Printf(TEXT("Measurement refresh 실패: %s"), *RefreshError);
			return false;
		}

		const FCFVehicleMeasurementProposal* Proposal = FindMeasurementProposal(
			ViewModel.GetMeasurementResult(),
			ExpectedMeasurement.FieldPath);
		if (!Proposal)
		{
			OutFailureReason = FString::Printf(TEXT("필수 measurement proposal이 없습니다: %s"), ExpectedMeasurement.FieldPath);
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

		FCFVehicleMeasurementPreviewResult Preview;
		if (!ViewModel.PrepareMeasurementDecision(*Proposal, ECFVehicleMeasureDecision::AcceptMeasuredValue, Preview))
		{
			OutFailureReason = Preview.Operation.Message.IsEmpty()
				? FString::Printf(TEXT("Measurement preview 실패: %s"), ExpectedMeasurement.FieldPath)
				: Preview.Operation.Message;
			return false;
		}

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
}

// Headless Editor commandlet 실행 속성을 준비합니다.
UCFBuilderWagonReconcileCommandlet::UCFBuilderWagonReconcileCommandlet()
{
	IsClient = false;
	IsServer = false;
	IsEditor = true;
	LogToConsole = true;
	ShowErrorCount = true;
}

// USER-approved 0.80 Socket truth를 Chassis와 Recipe adoption에 재정합화하고 Target mutation 없이 Resolver Success를 검증합니다.
int32 UCFBuilderWagonReconcileCommandlet::Main(const FString& Params)
{
	(void)Params;

	using namespace CFBuilderWagonReconcile;

	FString FailureReason;

	UCFVehicleRecipeData* Recipe = LoadObject<UCFVehicleRecipeData>(nullptr, WagonRecipeObjectPath);
	UCFVehicleData* Target = LoadObject<UCFVehicleData>(nullptr, WagonTargetObjectPath);
	UStaticMesh* Chassis = LoadObject<UStaticMesh>(nullptr, WagonChassisObjectPath);
	UStaticMesh* SharedWheel = LoadObject<UStaticMesh>(nullptr, SharedWheelObjectPath);

	if (!Recipe || !Target || !Chassis || !SharedWheel)
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_LOAD_FAIL"));
		return static_cast<int32>(EExitCode::AssetLoadFailed);
	}

	if (Recipe->WheelVisualIntent.Mode != ECFWheelVisualIntentMode::SocketScaleFromChassis)
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_PREFLIGHT_FAIL Recipe WheelVisualIntent is not SocketScaleFromChassis"));
		return static_cast<int32>(EExitCode::PreflightFailed);
	}

	if (!ValidateTargetUserPass(Target, Chassis, SharedWheel, FailureReason))
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_TARGET_FAIL %s"), *FailureReason);
		return static_cast<int32>(EExitCode::PreflightFailed);
	}

	const FBoxSphereBounds SharedWheelBounds = SharedWheel->GetBounds();
	FString WheelBoundsError;
	if (!FCFWheelSizeUtils::ValidateCanonicalWheelBounds(SharedWheelBounds.Origin, SharedWheelBounds.BoxExtent, WheelBoundsError))
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_WHEEL_FAIL %s"), *WheelBoundsError);
		return static_cast<int32>(EExitCode::PreflightFailed);
	}

	bool bNeedsSocketRepair = false;
	if (!InspectChassisSocketState(Chassis, bNeedsSocketRepair, FailureReason))
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_SOCKET_PREFLIGHT_FAIL %s"), *FailureReason);
		return static_cast<int32>(EExitCode::PreflightFailed);
	}

	// Reconcile 전 Target semantic hash입니다. Target은 끝까지 불변이어야 합니다.
	FCFVehicleDefinitionSnapshot TargetBefore;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetBefore, FailureReason))
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_TARGET_SNAPSHOT_FAIL %s"), *FailureReason);
		return static_cast<int32>(EExitCode::PreflightFailed);
	}

	// Persistent save 직전 rollback baseline이 될 Chassis package bytes입니다.
	FString ChassisFilename;
	TArray<uint8> ChassisBytes;
	// Persistent save 직전 rollback baseline이 될 Recipe package bytes입니다.
	FString RecipeFilename;
	TArray<uint8> RecipeBytes;
	if (!BackupExistingPackage(Chassis, ChassisFilename, ChassisBytes, FailureReason)
		|| !BackupExistingPackage(Recipe, RecipeFilename, RecipeBytes, FailureReason))
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_BACKUP_FAIL %s"), *FailureReason);
		return static_cast<int32>(EExitCode::PreflightFailed);
	}

	if (bNeedsSocketRepair && !RepairChassisSocketScale(Chassis, FailureReason))
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_SOCKET_REPAIR_FAIL %s"), *FailureReason);
		return static_cast<int32>(EExitCode::SocketRepairFailed);
	}

	// Existing Authoring facade를 통해 actual Wagon을 current selection으로 만든다.
	FCFVehicleAuthoringVM ViewModel;
	FString BrowserError;
	if (!ViewModel.RefreshBrowser(TEXT("Wagon"), BrowserError))
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_BROWSER_FAIL %s"), *BrowserError);
		return static_cast<int32>(EExitCode::SelectionFailed);
	}

	const FCFVehicleListEntry* WagonEntry = FindExactWagonEntry(ViewModel.GetBrowserEntries());
	if (!WagonEntry)
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_SELECTION_FAIL exact managed row missing"));
		return static_cast<int32>(EExitCode::SelectionFailed);
	}

	FString SelectionError;
	ViewModel.SelectVehicle(*WagonEntry, SelectionError);
	if (!ViewModel.HasSelection() || ViewModel.GetRecipe() != Recipe || ViewModel.GetTargetVehicleData() != Target)
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_SELECTION_FAIL %s"), *SelectionError);
		return static_cast<int32>(EExitCode::SelectionFailed);
	}

	// USER-approved current 0.80 source에서 나온 Radius/Width 4건을 fresh fingerprint로 한 건씩 재채택합니다.
	for (const FExpectedMeasurement& ExpectedMeasurement : ExpectedMeasurements)
	{
		if (!AdoptExpectedMeasurement(ViewModel, ExpectedMeasurement, FailureReason))
		{
			UE_LOG(
				LogCFBuilderWagonReconcile,
				Error,
				TEXT("CF_BUILDER_WAGON_RECONCILE_MEASUREMENT_FAIL field=%s reason=%s"),
				ExpectedMeasurement.FieldPath,
				*FailureReason);
			return static_cast<int32>(EExitCode::MeasurementFailed);
		}
	}

	// Adoption 이후 current Resolver/Diff/Validation을 fresh 재계산합니다.
	FString PreviewError;
	if (!ViewModel.RefreshPreview(PreviewError))
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_RESOLVE_FAIL %s"), *PreviewError);
		return static_cast<int32>(EExitCode::ResolveFailed);
	}

	const FCFVehicleResolveReadResult& ResolveRead = ViewModel.GetResolveResult();
	if (ResolveRead.ResolveResult.ResolveStatus != ECFVehicleResolveStatus::Success)
	{
		UE_LOG(
			LogCFBuilderWagonReconcile,
			Error,
			TEXT("CF_BUILDER_WAGON_RECONCILE_RESOLVE_NOT_SUCCESS status=%d"),
			static_cast<int32>(ResolveRead.ResolveResult.ResolveStatus));
		return static_cast<int32>(EExitCode::ResolveFailed);
	}

	// Reconcile 동안 Target Definition이 어떤 방식으로도 바뀌지 않았는지 검증합니다.
	FCFVehicleDefinitionSnapshot TargetAfter;
	if (!FCFVehicleSnapshotBuilder::BuildDefinitionSnapshot(*Target, TargetAfter, FailureReason)
		|| TargetAfter.DefinitionHash != TargetBefore.DefinitionHash)
	{
		UE_LOG(
			LogCFBuilderWagonReconcile,
			Error,
			TEXT("CF_BUILDER_WAGON_RECONCILE_TARGET_MUTATION_FAIL before=%s after=%s reason=%s"),
			*TargetBefore.DefinitionHash,
			*TargetAfter.DefinitionHash,
			*FailureReason);
		return static_cast<int32>(EExitCode::VerificationFailed);
	}

	// 최종 Chassis Socket readback을 exact USER-approved 0.80으로 확인합니다.
	bool bStillNeedsRepair = false;
	if (!InspectChassisSocketState(Chassis, bStillNeedsRepair, FailureReason) || bStillNeedsRepair)
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_SOCKET_VERIFY_FAIL %s"), *FailureReason);
		return static_cast<int32>(EExitCode::VerificationFailed);
	}

	// Chassis + Recipe만 atomically 저장하고 Target은 저장하지 않습니다.
	if (!SavePackagePair(
		Chassis,
		Recipe,
		ChassisFilename,
		ChassisBytes,
		RecipeFilename,
		RecipeBytes,
		FailureReason))
	{
		UE_LOG(LogCFBuilderWagonReconcile, Error, TEXT("CF_BUILDER_WAGON_RECONCILE_SAVE_FAIL %s"), *FailureReason);
		return static_cast<int32>(EExitCode::SaveFailed);
	}

	UE_LOG(
		LogCFBuilderWagonReconcile,
		Display,
		TEXT("CF_BUILDER_WAGON_RECONCILE_PASS socket_repaired=%s target_hash=%s resolved_hash=%s saved_packages=2 target_saved=false"),
		bNeedsSocketRepair ? TEXT("true") : TEXT("false"),
		*TargetAfter.DefinitionHash,
		*ResolveRead.ResolveResult.ResolvedDefinitionHash);

	return static_cast<int32>(EExitCode::Success);
}
