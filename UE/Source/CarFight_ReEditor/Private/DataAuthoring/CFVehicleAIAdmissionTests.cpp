// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAIAdmissionTests.cpp
// Version: v1.2.1
// Date: 2026-10-20
// Description: CF-FQ-059 P0-03C AI mutation0 신규 Vehicle R2 Preview 및 기존 create guard 회귀 검증.
// Changelog:
// - v1.2.1: 추상 UObject를 직접 생성해 UE5.8 Ensure를 발생시키던 WrongChassis fixture를 정상 생성 가능한 transient UCFVehicleRecipeData로 교체. WrongAssetType 음성 검증의 의미와 비저장 정책은 유지.
// - v1.2.0: 실제 누락된 미저장 대상 패키지 충돌, 잘못된 Chassis UObject 타입, Hardpoint policy hash 변화를 읽기 전용 검증. 테스트 전용 transient fixture는 Save 없이 garbage 처리.
// - v1.1.0: C-1 canonical identity 공통 생성, AI 비정상 package/object/VehicleId 거부, 기존 UI Advanced Preview 호환성 검증.
// - v1.0.0: UI/AI 동일 scope hash, AI 직접 Create 거부, caller 종류 거부, /Game 외부 경로 및 충돌 경로 거부, 신규 패키지 생성/저장 0 검사.
// Migration:
// - v1.2.1: 테스트 Fixture만 변경. 실제 Authoring Writer/레시피 생성/저장 로직과 Automation 선택자에는 변화 없음.
// - v1.2.0: 기존 VehicleAIPreview Automation exact name 유지. 새 fixture는 고유 메모리 package/transient UObject만 사용하고 지속 저장하지 않습니다.
// - 신뢰된 AI R2 실행 권한은 발급하지 않으며 기존 SlateUI/Automation Create 계약과 Product Asset을 변경하지 않습니다.

#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "DataAuthoring/CFVehicleRecordIdentity.h"
#include "DataAuthoring/CFVehicleRecipeData.h"

#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "Misc/PackageName.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleAIRecordPreviewTest,
	"CarFight.CCAS.CF_FQ_059.P0_03C.VehicleAIPreview",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// AI Preview가 기존 R2 생성안과 동일하면서도 승인되지 않은 Create·잘못된 경로를 거부하는지 검증합니다.
bool FCFVehicleAIRecordPreviewTest::RunTest(const FString& Parameters)
{
	// 테스트 실행마다 다른, 디스크에 존재하지 않는 후보 Asset 이름을 구성합니다.
	const FString UniqueToken = FGuid::NewGuid().ToString(EGuidFormats::Digits);
	// 기존 Builder 기본 신규 Authoring 경로 규칙으로 만든 후보 Definition package입니다.
	const FString DefinitionPackageName = TEXT("/Game/CarFight/Data/Authoring/DA_Vehicle_") + UniqueToken;
	// 기존 Builder 기본 신규 Authoring 경로 규칙으로 만든 후보 Recipe package입니다.
	const FString RecipePackageName = TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_") + UniqueToken;
	// AI의 읽기 전용 생성안 입력입니다.
	FCFVehicleRecordCreateRequest AIRequest;
	AIRequest.DefinitionPackageName = DefinitionPackageName;
	AIRequest.DefinitionAssetName = FName(*FString(TEXT("DA_Vehicle_") + UniqueToken));
	AIRequest.RecipePackageName = RecipePackageName;
	AIRequest.RecipeAssetName = FName(*FString(TEXT("DA_Recipe_") + UniqueToken));
	AIRequest.CallerKind = ECFAuthoringCallerKind::AI;

	TestNull(TEXT("Preview start has no Definition package"), FindPackage(nullptr, *DefinitionPackageName));
	TestNull(TEXT("Preview start has no Recipe package"), FindPackage(nullptr, *RecipePackageName));

	// AI 호출이 만드는 변경 없는 R2 proposal입니다.
	FCFVehicleRecordCreatePreview AIPreview;
	if (!TestTrue(TEXT("AI Preview succeeds without UI caller spoofing"),
		FCFVehicleAuthoringService::PreviewAIVehicleRecords(AIRequest, AIPreview)))
	{
		AddError(AIPreview.Operation.Message);
		return false;
	}
	TestEqual(TEXT("AI Preview has R2 proposal"), AIPreview.Proposal.RiskClass, ECFAuthoringRiskClass::R2_OwnershipExceptionalWrite);
	TestEqual(TEXT("AI Preview requires existing OwnershipWrite"), AIPreview.Proposal.RequiredApprovalClass, ECFAuthoringApprovalClass::OwnershipWrite);
	TestTrue(TEXT("AI Preview returns an exact proposal hash"), !AIPreview.Proposal.ProposalHash.IsEmpty());
	TestFalse(TEXT("AI Preview never saves"), AIPreview.Proposal.bSavePerformed);

	// 기존 UI 경로와 비교할 동일 입력의 합법적 SlateUI Preview입니다.
	FCFVehicleRecordCreateRequest UIRequest = AIRequest;
	UIRequest.CallerKind = ECFAuthoringCallerKind::SlateUI;
	// 기존 UI Preview가 만든 계약 결과입니다.
	FCFVehicleRecordCreatePreview UIPreview;
	if (TestTrue(TEXT("Existing UI Preview still succeeds"), FCFVehicleAuthoringService::PreviewVehicleRecords(UIRequest, UIPreview)))
	{
		TestEqual(TEXT("AI and UI reviewed identical content have exact same proposal hash"),
			AIPreview.Proposal.ProposalHash, UIPreview.Proposal.ProposalHash);
	}

	// AI가 자기 입력에 승인 플래그와 정확한 해시까지 써 넣은 공격성 요청입니다.
	FCFVehicleRecordCreateRequest FakeApprovalRequest = AIRequest;
	FakeApprovalRequest.bOwnershipWriteApproved = true;
	FakeApprovalRequest.ApprovalScopeHash = AIPreview.Proposal.ProposalHash;
	// 실제 R2 생성 API는 여전히 AI 직접 호출을 차단해야 합니다.
	FCFVehicleRecordCreateResult DeniedCreateResult;
	TestFalse(TEXT("AI self-declared approval cannot execute existing Create"),
		FCFVehicleAuthoringService::CreateVehicleRecords(FakeApprovalRequest, DeniedCreateResult));
	TestEqual(TEXT("AI direct Create returns unsupported operation"),
		DeniedCreateResult.Operation.ErrorCode, ECFAuthoringErrorCode::UnsupportedOperation);
	TestNull(TEXT("Denied AI Create did not return Definition"), DeniedCreateResult.CreatedDefinition);
	TestNull(TEXT("Denied AI Create did not return Recipe"), DeniedCreateResult.CreatedRecipe);
	TestFalse(TEXT("Denied AI Create reports no saved packages"), DeniedCreateResult.Operation.Mutation.bSavePerformed);

	// AI Preview를 Automation으로 위장해 실행하려는 잘못된 caller 입력입니다.
	FCFVehicleRecordCreateRequest SpoofedPreviewRequest = AIRequest;
	SpoofedPreviewRequest.CallerKind = ECFAuthoringCallerKind::Automation;
	// AI 전용 Preview 경계에서 종류가 다른 호출을 거부한 결과입니다.
	FCFVehicleRecordCreatePreview SpoofedPreview;
	TestFalse(TEXT("AI Preview never accepts Automation caller identity"),
		FCFVehicleAuthoringService::PreviewAIVehicleRecords(SpoofedPreviewRequest, SpoofedPreview));
	TestEqual(TEXT("Non-AI caller is rejected on AI Preview"),
		SpoofedPreview.Operation.ErrorCode, ECFAuthoringErrorCode::UnsupportedOperation);

	// 다른 Asset이어도 패키지가 같은 경우의 충돌 요청입니다.
	FCFVehicleRecordCreateRequest DuplicatePackageRequest = AIRequest;
	DuplicatePackageRequest.RecipePackageName = AIRequest.DefinitionPackageName;
	// 중복 package를 막는 읽기 전용 결과입니다.
	FCFVehicleRecordCreatePreview DuplicatePackagePreview;
	TestFalse(TEXT("AI Preview refuses same-package Definition and Recipe"),
		FCFVehicleAuthoringService::PreviewAIVehicleRecords(DuplicatePackageRequest, DuplicatePackagePreview));
	TestEqual(TEXT("Same-package request is invalid semantic input"),
		DuplicatePackagePreview.Operation.ErrorCode, ECFAuthoringErrorCode::InvalidSemanticInput);

	// 이전에 Save하지 않았어도 메모리에 존재하는 canonical Definition package 충돌을 검사할 독립 후보입니다.
	const FString OccupiedVehicleId = TEXT("Occupied_") + UniqueToken;
	// 테스트가 소유한 메모리 전용 Definition/Recipe identity 요청입니다.
	FCFVehicleRecordCreateRequest OccupiedRequest = AIRequest;
	OccupiedRequest.DefinitionAssetName = FName(*(TEXT("DA_Vehicle_") + OccupiedVehicleId));
	OccupiedRequest.DefinitionPackageName = TEXT("/Game/CarFight/Data/Authoring/") + OccupiedRequest.DefinitionAssetName.ToString();
	OccupiedRequest.RecipeAssetName = FName(*(TEXT("DA_Recipe_") + OccupiedVehicleId));
	OccupiedRequest.RecipePackageName = TEXT("/Game/CarFight/Data/Authoring/") + OccupiedRequest.RecipeAssetName.ToString();
	// 원본 프로젝트 Asset과 겹치지 않는 새 고유 package만 메모리에 생성합니다. Save는 수행하지 않습니다.
	UPackage* OccupiedUnsavedPackage = CreatePackage(*OccupiedRequest.DefinitionPackageName);
	if (TestNotNull(TEXT("Unsaved collision fixture package exists in memory"), OccupiedUnsavedPackage))
	{
		TestFalse(TEXT("Unsaved collision fixture is not persisted to disk"), FPackageName::DoesPackageExist(OccupiedRequest.DefinitionPackageName));
		// 대상 Definition package가 메모리에 이미 있으므로 AI Preview는 변경 없이 거부해야 합니다.
		FCFVehicleRecordCreatePreview OccupiedPreview;
		TestFalse(TEXT("AI Preview refuses an existing unsaved Definition package"),
			FCFVehicleAuthoringService::PreviewAIVehicleRecords(OccupiedRequest, OccupiedPreview));
		TestEqual(TEXT("Unsaved Definition collision returns StateChanged"),
			OccupiedPreview.Operation.ErrorCode, ECFAuthoringErrorCode::StateChanged);
		TestNull(TEXT("Unsaved collision did not create Recipe package"), FindPackage(nullptr, *OccupiedRequest.RecipePackageName));
		// 테스트 전용으로 만든 빈 package를 garbage 대상으로 정리하며 디스크 저장은 하지 않습니다.
		OccupiedUnsavedPackage->SetDirtyFlag(false);
		OccupiedUnsavedPackage->MarkAsGarbage();
	}

	// StaticMesh가 아닌, 실제 생성 가능한 transient Recipe DataAsset을 Chassis로 참조하는 비정상 입력입니다.
	// 추상 UObject를 NewObject로 생성하면 UE5.8 Ensure가 발생하므로 기존 concrete UCFVehicleRecipeData를 사용합니다.
	UObject* WrongChassisObject = NewObject<UCFVehicleRecipeData>(
		GetTransientPackage(), FName(*(TEXT("CF059WrongChassis_") + UniqueToken)), RF_Transient);
	if (TestNotNull(TEXT("Wrong-type transient Chassis fixture exists"), WrongChassisObject))
	{
		// 실제 UObject는 존재하지만 StaticMesh 타입이 아니어야 합니다.
		FCFVehicleRecordCreateRequest WrongChassisRequest = AIRequest;
		WrongChassisRequest.ChassisMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(WrongChassisObject));
		// Reference의 클래스 검사가 Preview에서 실패하고 새 package는 생성되지 않아야 합니다.
		FCFVehicleRecordCreatePreview WrongChassisPreview;
		TestFalse(TEXT("AI Preview refuses non-StaticMesh Chassis reference"),
			FCFVehicleAuthoringService::PreviewAIVehicleRecords(WrongChassisRequest, WrongChassisPreview));
		TestEqual(TEXT("Non-StaticMesh Chassis returns WrongAssetType"),
			WrongChassisPreview.Operation.ErrorCode, ECFAuthoringErrorCode::WrongAssetType);
		// 테스트 전용 transient UObject만 GC 대상으로 남기며 기존 Asset에는 영향이 없습니다.
		WrongChassisObject->MarkAsGarbage();
	}

	// /Game 외부 패키지에 신규 Vehicle을 만들라는 잘못된 입력입니다.
	FCFVehicleRecordCreateRequest ForeignPackageRequest = AIRequest;
	ForeignPackageRequest.DefinitionPackageName = TEXT("/Engine/Transient/DA_OutsideGame");
	// /Game으로 제한한 생성 범위가 외부 패키지를 차단해야 합니다.
	FCFVehicleRecordCreatePreview ForeignPackagePreview;
	TestFalse(TEXT("AI Preview refuses non-Game asset package"),
		FCFVehicleAuthoringService::PreviewAIVehicleRecords(ForeignPackageRequest, ForeignPackagePreview));
	TestEqual(TEXT("Foreign package scope is blocked by canonical identity"),
		ForeignPackagePreview.Operation.ErrorCode, ECFAuthoringErrorCode::InvalidSemanticInput);

	// 기본 Builder의 4-field naming owner와 UI-less AI의 입력이 실제 동일한지 검사합니다.
	// 동일한 VehicleId에서 도출한 Definition package입니다.
	FString BuiltDefinitionPackage;
	// 동일한 VehicleId에서 도출한 Definition object입니다.
	FString BuiltDefinitionAsset;
	// 동일한 VehicleId에서 도출한 Recipe package입니다.
	FString BuiltRecipePackage;
	// 동일한 VehicleId에서 도출한 Recipe object입니다.
	FString BuiltRecipeAsset;
	// 이름 생성 실패 이유입니다.
	FString IdentityError;
	if (TestTrue(TEXT("Canonical helper builds AI candidate identity"),
		FCFVehicleRecordIdentity::BuildDefaultIdentity(
			UniqueToken, BuiltDefinitionPackage, BuiltDefinitionAsset, BuiltRecipePackage, BuiltRecipeAsset, IdentityError)))
	{
		TestEqual(TEXT("Helper Definition package matches AI request"), BuiltDefinitionPackage, AIRequest.DefinitionPackageName);
		TestEqual(TEXT("Helper Definition name matches AI request"), BuiltDefinitionAsset, AIRequest.DefinitionAssetName.ToString());
		TestEqual(TEXT("Helper Recipe package matches AI request"), BuiltRecipePackage, AIRequest.RecipePackageName);
		TestEqual(TEXT("Helper Recipe name matches AI request"), BuiltRecipeAsset, AIRequest.RecipeAssetName.ToString());
	}

	// AI가 정상적인 /Game 하위이지만 canonical Authoring root 밖의 위치를 요구합니다.
	FCFVehicleRecordCreateRequest OtherGameRootRequest = AIRequest;
	OtherGameRootRequest.DefinitionPackageName = TEXT("/Game/CarFight/Tests/CF059AI/") + AIRequest.DefinitionAssetName.ToString();
	// 허용된 경로의 일치 여부 확인 결과입니다.
	FCFVehicleRecordCreatePreview OtherGameRootPreview;
	TestFalse(TEXT("AI Preview cannot escape canonical Authoring root"),
		FCFVehicleAuthoringService::PreviewAIVehicleRecords(OtherGameRootRequest, OtherGameRootPreview));
	TestEqual(TEXT("Canonical scope mismatch is semantic invalid"),
		OtherGameRootPreview.Operation.ErrorCode, ECFAuthoringErrorCode::InvalidSemanticInput);

	// AI가 동일 Root에서 서로 다른 VehicleId를 가진 Recipe를 제출합니다.
	FCFVehicleRecordCreateRequest MismatchedRecipeRequest = AIRequest;
	MismatchedRecipeRequest.RecipeAssetName = FName(*(TEXT("DA_Recipe_Other") + UniqueToken));
	// Definition과 Recipe VehicleId 불일치 확인 결과입니다.
	FCFVehicleRecordCreatePreview MismatchedRecipePreview;
	TestFalse(TEXT("AI Preview refuses mismatched Definition/Recipe identity"),
		FCFVehicleAuthoringService::PreviewAIVehicleRecords(MismatchedRecipeRequest, MismatchedRecipePreview));
	TestEqual(TEXT("Definition/Recipe identity mismatch is semantic invalid"),
		MismatchedRecipePreview.Operation.ErrorCode, ECFAuthoringErrorCode::InvalidSemanticInput);

	// 공통 VehicleId validator가 ASCII 외 문자와 경로 구분자를 거부하는지 확인합니다.
	TestFalse(TEXT("Canonical VehicleId refuses Unicode"), FCFVehicleRecordIdentity::ValidateVehicleId(TEXT("왜건"), IdentityError));
	TestFalse(TEXT("Canonical VehicleId refuses path separator"), FCFVehicleRecordIdentity::ValidateVehicleId(TEXT("SUV/Other"), IdentityError));
	TestFalse(TEXT("Canonical VehicleId refuses blank"), FCFVehicleRecordIdentity::ValidateVehicleId(TEXT(""), IdentityError));
	// 무효한 ID를 빌드할 때 기존 Builder처럼 출력 4개가 비워져야 합니다.
	FString InvalidDefinitionPackage = TEXT("stale");
	// 무효한 ID를 빌드할 때 사용할 Definition object 결과입니다.
	FString InvalidDefinitionObject = TEXT("stale");
	// 무효한 ID를 빌드할 때 사용할 Recipe package 결과입니다.
	FString InvalidRecipePackage = TEXT("stale");
	// 무효한 ID를 빌드할 때 사용할 Recipe object 결과입니다.
	FString InvalidRecipeObject = TEXT("stale");
	TestFalse(TEXT("Canonical builder fails invalid ID"),
		FCFVehicleRecordIdentity::BuildDefaultIdentity(
			TEXT("Bad/Vehicle"), InvalidDefinitionPackage, InvalidDefinitionObject,
			InvalidRecipePackage, InvalidRecipeObject, IdentityError));
	TestTrue(TEXT("Invalid identity resets all four output names"),
		InvalidDefinitionPackage.IsEmpty() && InvalidDefinitionObject.IsEmpty()
		&& InvalidRecipePackage.IsEmpty() && InvalidRecipeObject.IsEmpty());

	// 요청 중 실제 차량 제작 policy 하나가 변경된 상태입니다.
	FCFVehicleRecordCreateRequest ChangedIntentRequest = AIRequest;
	ChangedIntentRequest.bRequireVehicleSpecificTransmission = true;
	// 변경된 내용의 새로운 생성안입니다.
	FCFVehicleRecordCreatePreview ChangedIntentPreview;
	if (TestTrue(TEXT("Changed transmission request can be previewed"),
		FCFVehicleAuthoringService::PreviewAIVehicleRecords(ChangedIntentRequest, ChangedIntentPreview)))
	{
		TestNotEqual(TEXT("Changed transmission intent changes exact approval hash"),
			ChangedIntentPreview.Proposal.ProposalHash, AIPreview.Proposal.ProposalHash);
	}

	// 동일 차량 요청에서 explicit Hardpoint Plan 정책만 바꾼 새 생성안입니다.
	FCFVehicleRecordCreateRequest ChangedHardpointRequest = AIRequest;
	ChangedHardpointRequest.bRequireExplicitHardpointPlan = true;
	// Hardpoint 정책 변경을 승인 Hash에 결속할 읽기 전용 결과입니다.
	FCFVehicleRecordCreatePreview ChangedHardpointPreview;
	if (TestTrue(TEXT("Changed Hardpoint plan request can be previewed"),
		FCFVehicleAuthoringService::PreviewAIVehicleRecords(ChangedHardpointRequest, ChangedHardpointPreview)))
	{
		TestNotEqual(TEXT("Changed Hardpoint plan changes exact approval hash"),
			ChangedHardpointPreview.Proposal.ProposalHash, AIPreview.Proposal.ProposalHash);
	}

	TestNull(TEXT("No Definition package created by any AI Preview or denied Create"), FindPackage(nullptr, *DefinitionPackageName));
	TestNull(TEXT("No Recipe package created by any AI Preview or denied Create"), FindPackage(nullptr, *RecipePackageName));
	TestFalse(TEXT("No Definition package persisted"), FPackageName::DoesPackageExist(DefinitionPackageName));
	TestFalse(TEXT("No Recipe package persisted"), FPackageName::DoesPackageExist(RecipePackageName));
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
