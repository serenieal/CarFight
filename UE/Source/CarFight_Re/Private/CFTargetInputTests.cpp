// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-24
// Description: TS-P0-05 Enhanced Input 에셋, Pawn 후보 선택 확정과 Manual 해제 자동화 테스트

#include "CFVehiclePawn.h"
#include "CFTargetSelectComp.h"
#include "CFTargetSelectContractTestTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetRegistryModule.h"
#include "EnhancedActionKeyMapping.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "InputTriggers.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

namespace
{
	const TCHAR* SelectTargetPackageName = TEXT("/Game/CarFight/Input/IA_SelectTarget");
	const TCHAR* SelectTargetAssetName = TEXT("IA_SelectTarget");
	const TCHAR* ClearTargetPackageName = TEXT("/Game/CarFight/Input/IA_ClearTarget");
	const TCHAR* ClearTargetAssetName = TEXT("IA_ClearTarget");
	const TCHAR* DefaultMappingContextPath = TEXT("/Game/CarFight/Input/IMC_Vehicle_Default.IMC_Vehicle_Default");

	bool SaveInputAsset(UObject* AssetObject)
	{
		if (!AssetObject)
		{
			return false;
		}

		UPackage* AssetPackage = AssetObject->GetOutermost();
		if (!AssetPackage)
		{
			return false;
		}

		const FString PackageFilename = FPackageName::LongPackageNameToFilename(
			AssetPackage->GetName(),
			FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SavePackageArgs;
		SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SavePackageArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(AssetPackage, AssetObject, *PackageFilename, SavePackageArgs);
	}

	UInputAction* EnsureBooleanInputAction(const TCHAR* PackageName, const TCHAR* AssetName)
	{
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), PackageName, AssetName);
		UInputAction* InputAction = LoadObject<UInputAction>(nullptr, *ObjectPath);
		bool bCreatedAsset = false;
		bool bChangedAsset = false;

		if (!InputAction)
		{
			UPackage* InputActionPackage = CreatePackage(PackageName);
			if (!InputActionPackage)
			{
				return nullptr;
			}

			InputAction = NewObject<UInputAction>(
				InputActionPackage,
				FName(AssetName),
				RF_Public | RF_Standalone | RF_Transactional);
			if (!InputAction)
			{
				return nullptr;
			}

			FAssetRegistryModule::AssetCreated(InputAction);
			bCreatedAsset = true;
			bChangedAsset = true;
		}

		if (InputAction->ValueType != EInputActionValueType::Boolean)
		{
			InputAction->ValueType = EInputActionValueType::Boolean;
			bChangedAsset = true;
		}
		if (!InputAction->bConsumeInput)
		{
			InputAction->bConsumeInput = true;
			bChangedAsset = true;
		}

		bool bHasPressedTrigger = false;
		for (const UInputTrigger* InputTrigger : InputAction->Triggers)
		{
			if (InputTrigger && InputTrigger->IsA<UInputTriggerPressed>())
			{
				bHasPressedTrigger = true;
				break;
			}
		}
		if (!bHasPressedTrigger)
		{
			InputAction->Triggers.Add(NewObject<UInputTriggerPressed>(InputAction));
			bChangedAsset = true;
		}

		if (bChangedAsset)
		{
			InputAction->MarkPackageDirty();
			if (!SaveInputAsset(InputAction))
			{
				return nullptr;
			}
		}

		(void)bCreatedAsset;
		return InputAction;
	}

	bool HasInputMapping(const UInputMappingContext* MappingContext, const UInputAction* InputAction, const FKey& MappingKey)
	{
		if (!MappingContext || !InputAction)
		{
			return false;
		}

		for (const FEnhancedActionKeyMapping& ExistingMapping : MappingContext->GetMappings())
		{
			if (ExistingMapping.Action == InputAction && ExistingMapping.Key == MappingKey)
			{
				return true;
			}
		}
		return false;
	}

	bool EnsureTargetInputMappings(
		UInputMappingContext* MappingContext,
		UInputAction* SelectTargetAction,
		UInputAction* ClearTargetAction)
	{
		if (!MappingContext || !SelectTargetAction || !ClearTargetAction)
		{
			return false;
		}

		struct FRequiredTargetInputMapping
		{
			UInputAction* InputAction = nullptr;
			FKey InputKey;
		};

		const TArray<FRequiredTargetInputMapping> RequiredMappings =
		{
			{SelectTargetAction, EKeys::MiddleMouseButton},
			{SelectTargetAction, EKeys::Gamepad_RightThumbstick},
			{ClearTargetAction, EKeys::RightMouseButton},
			{ClearTargetAction, EKeys::Gamepad_FaceButton_Right}
		};

		bool bChangedMappingContext = false;
		for (const FRequiredTargetInputMapping& RequiredMapping : RequiredMappings)
		{
			for (const FEnhancedActionKeyMapping& ExistingMapping : MappingContext->GetMappings())
			{
				if (ExistingMapping.Key == RequiredMapping.InputKey
					&& ExistingMapping.Action != RequiredMapping.InputAction)
				{
					return false;
				}
			}

			if (!HasInputMapping(MappingContext, RequiredMapping.InputAction, RequiredMapping.InputKey))
			{
				MappingContext->Modify();
				MappingContext->MapKey(RequiredMapping.InputAction, RequiredMapping.InputKey);
				bChangedMappingContext = true;
			}
		}

		if (bChangedMappingContext)
		{
			MappingContext->MarkPackageDirty();
			return SaveInputAsset(MappingContext);
		}
		return true;
	}

	ACFTargetSelectContractActor* SpawnInputTarget(UWorld* TestWorld, const FName ActorName, const FVector& ActorLocation)
	{
		if (!TestWorld)
		{
			return nullptr;
		}

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ActorName;
		ACFTargetSelectContractActor* TargetActor = TestWorld->SpawnActor<ACFTargetSelectContractActor>(
			ACFTargetSelectContractActor::StaticClass(),
			ActorLocation,
			FRotator::ZeroRotator,
			SpawnParameters);
		if (TargetActor)
		{
			TargetActor->DisplayInfo.TargetId = ActorName;
			TargetActor->DisplayInfo.DisplayName = FText::FromName(ActorName);
			TargetActor->DisplayInfo.TargetCategory = ECFTargetCategory::Vehicle;
			TargetActor->DisplayInfo.Relation = ECFTargetRelation::Unknown;
			TargetActor->DisplayInfo.InformationLevel = ECFTargetInfoLevel::Identified;
		}
		return TargetActor;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetInputIntegrationTest,
	"CarFight.TargetSelect.TS_P0_05.InputIntegration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFTargetInputIntegrationTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UInputAction* SelectTargetAction = EnsureBooleanInputAction(SelectTargetPackageName, SelectTargetAssetName);
	UInputAction* ClearTargetAction = EnsureBooleanInputAction(ClearTargetPackageName, ClearTargetAssetName);
	UInputMappingContext* DefaultMappingContext = LoadObject<UInputMappingContext>(nullptr, DefaultMappingContextPath);

	TestNotNull(TEXT("IA_SelectTarget 생성 또는 로드"), SelectTargetAction);
	TestNotNull(TEXT("IA_ClearTarget 생성 또는 로드"), ClearTargetAction);
	TestNotNull(TEXT("IMC_Vehicle_Default 로드"), DefaultMappingContext);
	if (!SelectTargetAction || !ClearTargetAction || !DefaultMappingContext)
	{
		return false;
	}

	TestEqual(TEXT("IA_SelectTarget 값 타입 Boolean"), SelectTargetAction->ValueType, EInputActionValueType::Boolean);
	TestEqual(TEXT("IA_ClearTarget 값 타입 Boolean"), ClearTargetAction->ValueType, EInputActionValueType::Boolean);
	TestTrue(TEXT("TargetSelect 입력 매핑 저장"), EnsureTargetInputMappings(DefaultMappingContext, SelectTargetAction, ClearTargetAction));
	TestTrue(TEXT("선택 키 MiddleMouseButton"), HasInputMapping(DefaultMappingContext, SelectTargetAction, EKeys::MiddleMouseButton));
	TestTrue(TEXT("선택 키 Gamepad RightThumbstick"), HasInputMapping(DefaultMappingContext, SelectTargetAction, EKeys::Gamepad_RightThumbstick));
	TestTrue(TEXT("해제 키 RightMouseButton"), HasInputMapping(DefaultMappingContext, ClearTargetAction, EKeys::RightMouseButton));
	TestTrue(TEXT("해제 키 Gamepad FaceButton Right"), HasInputMapping(DefaultMappingContext, ClearTargetAction, EKeys::Gamepad_FaceButton_Right));

	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull(TEXT("입력 통합 테스트 월드 생성"), TestWorld);
	if (!TestWorld)
	{
		return false;
	}

		// BP 조립 없이 C++ 기본 Pawn만 생성하므로 SM_Body 누락 로그 1회는 이 테스트의 의도된 전제입니다.
	AddExpectedError(
		TEXT("VehicleVisualHitCollision: SM_Body component missing on TargetInputVehiclePawn."),
		EAutomationExpectedErrorFlags::Contains,
		1);

	FActorSpawnParameters PawnSpawnParameters;
	PawnSpawnParameters.Name = TEXT("TargetInputVehiclePawn");
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		PawnSpawnParameters);
	ACFTargetSelectContractActor* TargetActor = SpawnInputTarget(TestWorld, TEXT("InputTarget"), FVector(3000.0f, 0.0f, 0.0f));

	TestNotNull(TEXT("입력 통합 차량 Pawn 생성"), VehiclePawn);
	TestNotNull(TEXT("입력 통합 선택 대상 생성"), TargetActor);
	if (!VehiclePawn || !TargetActor)
	{
		return false;
	}

	VehiclePawn->InputAction_SelectTarget = SelectTargetAction;
	VehiclePawn->InputAction_ClearTarget = ClearTargetAction;
	TestEqual(TEXT("Pawn 선택 Input Action 연결"), VehiclePawn->InputAction_SelectTarget.Get(), SelectTargetAction);
	TestEqual(TEXT("Pawn 해제 Input Action 연결"), VehiclePawn->InputAction_ClearTarget.Get(), ClearTargetAction);

	UCFTargetSelectComp* TargetSelectComp = VehiclePawn->GetTargetSelectComp();
	TestNotNull(TEXT("Pawn TargetSelectComp 존재"), TargetSelectComp);
	if (!TargetSelectComp)
	{
		return false;
	}
	TargetSelectComp->bAutoRefreshCandidate = false;

	FCFTargetCandidate CandidateData;
	CandidateData.TargetActor = TargetActor;
	CandidateData.DisplayInfo = TargetActor->DisplayInfo;
	const FCFTargetSelectionContext SelectionContext = TargetSelectComp->GetDefaultSelectionContext();
	TestTrue(TEXT("입력 검증용 현재 후보 설정"), TargetSelectComp->SetCurrentCandidate(CandidateData, SelectionContext));
	TestTrue(TEXT("현재 후보 선택 확정"), VehiclePawn->ConfirmCurrentTargetCandidate());
	TestEqual(TEXT("확정된 선택 대상"), TargetSelectComp->GetSelectedTargetActor(), static_cast<AActor*>(TargetActor));

	TargetSelectComp->ClearCurrentCandidate();
	TestFalse(TEXT("후보가 없으면 선택 확정 실패"), VehiclePawn->ConfirmCurrentTargetCandidate());
	TestEqual(TEXT("후보 없음 선택 입력은 기존 선택 유지"), TargetSelectComp->GetSelectedTargetActor(), static_cast<AActor*>(TargetActor));

	TestTrue(TEXT("Manual 해제 전 후보 복원"), TargetSelectComp->SetCurrentCandidate(CandidateData, SelectionContext));
	TestTrue(TEXT("선택 타겟 Manual 해제"), VehiclePawn->ClearSelectedTargetManually());
	TestFalse(TEXT("Manual 해제 후 선택 기록 없음"), TargetSelectComp->HasSelectedTarget());
	TestEqual(TEXT("Manual 해제 사유"), TargetSelectComp->GetLastSelectedTargetClearReason(), ECFTargetClearReason::Manual);
	TestEqual(TEXT("Manual 해제 뒤 현재 후보 유지"), TargetSelectComp->GetCurrentCandidateActor(), static_cast<AActor*>(TargetActor));
	TestNull(TEXT("Manual 해제 뒤 후보 자동 선택 금지"), TargetSelectComp->GetSelectedTargetActor());
	TestFalse(TEXT("선택이 없으면 중복 Manual 해제 실패"), VehiclePawn->ClearSelectedTargetManually());

	TargetActor->Destroy();
	VehiclePawn->Destroy();
	return true;
}

#endif
