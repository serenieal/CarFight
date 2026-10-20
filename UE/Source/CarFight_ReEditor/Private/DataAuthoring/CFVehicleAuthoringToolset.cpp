// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleAuthoringToolset.cpp
// Version: v1.1.0
// Date: 2026-10-20
// Description: UE5.8 Toolset Registry CarFight 차량 identity R0와 검증된 관리형 AI R2 생성 메서드.
// Changelog:
// - v1.1.0: Consume Bridge-issued one-use execution proof before calling the existing private AI R2 writer.
// - v1.0.0: 기존 FCFVehicleRecordIdentity helper를 그대로 호출해 새 차량의 예상 경로만 반환.
// Migration:
// - Toolset Registry 자동 발견을 유지하고, 신규 R2 쓰기는 GoPyMCP에서 exact allowlist/attested opt-in 후에만 승인. 기본 운영 policy는 변경하지 않음.
// - 기존 CreateVehicleRecords/PreviewVehicleRecords 및 UE 자산은 변경하지 않음.

#include "DataAuthoring/CFVehicleAuthoringToolset.h"

#include "DataAuthoring/CFVehicleRecordIdentity.h"
#include "DataAuthoring/CFVehicleAuthoringService.h"
#include "CFVehicleData.h"
#include "Containers/StringConv.h"
#include "HAL/PlatformMisc.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Misc/Guid.h"
#include "UObject/Package.h"
#include "ToolsetRegistry/UToolsetRegistry.h"
#endif

// 입력 VehicleId에 대한 기존 canonical 4-field 이름을 조회하며 패키지나 에셋을 생성하지 않습니다.
FCFVehicleIdentityToolResult UCFVehicleAuthoringToolset::PreviewVehicleRecordIdentity(const FString& VehicleId)
{
	// 기존 UI/AI와 같은 규칙으로 계산되는 순수 읽기 전용 결과입니다.
	FCFVehicleIdentityToolResult Result;
	Result.bValid = FCFVehicleRecordIdentity::BuildDefaultIdentity(
		VehicleId,
		Result.DefinitionPackageName,
		Result.DefinitionAssetName,
		Result.RecipePackageName,
		Result.RecipeAssetName,
		Result.ErrorMessage);
	return Result;
}

// GoPyMCP 내부 Claim은 Toolset 실행 전에 발급된 단일 관리형 쓰기 증명만 1회 소비합니다.
namespace CFVehicleProof
{
	// 신뢰할 수 있는 Host-local Bridge 주소만 사용하고 AI 입력에서 검증 서버를 선택하지 않습니다.
	bool IsLoopbackClaim(const FString& URL)
	{
		const FString Prefix(TEXT("http://127.0.0.1:"));
		const FString Suffix(TEXT("/internal/ue-execution-proof/claim"));
		if (!URL.StartsWith(Prefix) || !URL.EndsWith(Suffix))
		{
			return false;
		}
		const FString Port = URL.Mid(Prefix.Len(), URL.Len() - Prefix.Len() - Suffix.Len());
		return !Port.IsEmpty() && Port.Len() <= 5 && Port.IsNumeric()
			&& FCString::Atoi(*Port) > 0 && FCString::Atoi(*Port) <= 65535;
	}

	// Bridge의 증명은 사용자 JSON으로 만들 수 없고 성공 응답까지 확인한 뒤 소비된 것으로 인정합니다.
	bool Consume(const FString& RequestJson, const FString& Proof)
	{
		if (Proof.Len() < 32 || Proof.Len() > 128 || RequestJson.IsEmpty()
			|| FTCHARToUTF8(*RequestJson).Length() > 32768)
		{
			return false;
		}
		FString URL = FPlatformMisc::GetEnvironmentVariable(TEXT("GOPYMCP_UE_PROOF_CLAIM_URL"));
		if (URL.IsEmpty())
		{
			URL = TEXT("http://127.0.0.1:8020/internal/ue-execution-proof/claim");
		}
		if (!IsLoopbackClaim(URL))
		{
			return false;
		}
		TSharedRef<FJsonObject> Payload = MakeShared<FJsonObject>();
		Payload->SetStringField(TEXT("ticket"), Proof);
		Payload->SetStringField(TEXT("request_json"), RequestJson);
		FString Body;
		const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Body);
		if (!FJsonSerializer::Serialize(Payload, Writer))
		{
			return false;
		}
		// Unreal 5.8 API는 게임 스레드에서도 HTTP 스레드를 통해 bounded blocking 호출을 완료합니다.
		const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
		HttpRequest->SetURL(URL);
		HttpRequest->SetVerb(TEXT("POST"));
		HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
		HttpRequest->SetTimeout(2.0f);
		HttpRequest->SetContentAsString(Body);
		HttpRequest->ProcessRequestUntilComplete();
		const FHttpResponsePtr Response = HttpRequest->GetResponse();
		if (!Response.IsValid() || Response->GetResponseCode() != 200)
		{
			return false;
		}
		TSharedPtr<FJsonObject> Result;
		if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Response->GetContentAsString()), Result)
			|| !Result.IsValid())
		{
			return false;
		}
		bool bAllowed = false;
		return Result->TryGetBoolField(TEXT("ok"), bAllowed) && bAllowed;
	}
}

// 1회용 실행 증명을 소비한 뒤에만 기존 신규 차량 two-record Writer를 실행합니다.
FCFVehicleCreateToolResult UCFVehicleAuthoringToolset::CreateNewVehicleRecords(
	const FString& request_json, const FString& gopymcp_execution_proof)
{
	FCFVehicleCreateToolResult Result;
	Result.ErrorCode = TEXT("InvalidRequest");
	TSharedPtr<FJsonObject> Input;
	if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(request_json), Input)
		|| !Input.IsValid() || Input->Values.Num() != 1)
	{
		return Result;
	}
	FString VehicleId;
	if (!Input->TryGetStringField(TEXT("VehicleId"), VehicleId))
	{
		return Result;
	}
	// 기존 canonical identity 검증과 기본 패키지 경로를 직접 재사용합니다.
	FCFVehicleRecordCreateRequest Request;
	Request.CallerKind = ECFAuthoringCallerKind::AI;
	FString IdentityError;
	FString DefinitionAssetName;
	FString RecipeAssetName;
	if (!FCFVehicleRecordIdentity::BuildDefaultIdentity(
		VehicleId, Request.DefinitionPackageName, DefinitionAssetName,
		Request.RecipePackageName, RecipeAssetName, IdentityError))
	{
		return Result;
	}
	Request.DefinitionAssetName = FName(*DefinitionAssetName);
	Request.RecipeAssetName = FName(*RecipeAssetName);
	Result.ErrorCode = TEXT("ExecutionProofRejected");
	if (!CFVehicleProof::Consume(request_json, gopymcp_execution_proof))
	{
		return Result;
	}
	// Fresh AI Preview가 생성 경로·충돌·R2 승인 범위를 결정합니다.
	FCFVehicleRecordCreatePreview Preview;
	Result.ErrorCode = TEXT("PreviewRejected");
	if (!FCFVehicleAuthoringService::PreviewAIVehicleRecords(Request, Preview))
	{
		return Result;
	}
	Request.bOwnershipWriteApproved = true;
	Request.ApprovalScopeHash = Preview.Proposal.ProposalHash;
	FCFVehicleRecordCreateResult Created;
	Result.ErrorCode = TEXT("CreateRejected");
	if (!FCFVehicleAuthoringService::CreateTrustedAIVehicleRecords(Request, Created))
	{
		return Result;
	}
	Result.ErrorCode = TEXT("CreateResultIncomplete");
	if (!Created.CreatedDefinition || !Created.CreatedRecipe)
	{
		return Result;
	}
	Result.bSucceeded = true;
	Result.ErrorCode.Empty();
	Result.DefinitionPath = Created.CreatedDefinition->GetPathName();
	Result.RecipePath = Created.CreatedRecipe->GetPathName();
	Result.bSavePerformed = Created.Operation.Mutation.bSavePerformed;
	return Result;
}

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFVehicleToolsetG1Test,
	"CarFight.CCAS.CF_FQ_059.P0_03C.G1ToolsetRegistration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

// G1의 현재 Editor에 등록된 R0 Toolset Schema와 실제 canonical identity 반환을 검증합니다.
bool FCFVehicleToolsetG1Test::RunTest(const FString& Parameters)
{
	// Toolset Registry가 현재 Editor에서 사용할 수 있는지 확인합니다.
	if (!TestTrue(TEXT("G1 UE Toolset Registry is available"), UToolsetRegistry::IsAvailable()))
	{
		return false;
	}

	TestTrue(
		TEXT("G1 CarFight Toolset registered once at Editor startup"),
		UToolsetRegistry::IsToolsetClassRegistered(UCFVehicleAuthoringToolset::StaticClass()));

	// UE Toolset Registry가 생성한 공개 툴 스키마입니다.
	const FString RegisteredToolSchema = UToolsetRegistry::GetToolsetJsonSchema(UCFVehicleAuthoringToolset::StaticClass());
	TestTrue(TEXT("G1 Toolset has one read-only identity tool"), RegisteredToolSchema.Contains(TEXT("PreviewVehicleRecordIdentity")));
	TestTrue(TEXT("G2 Toolset exposes separate proof-verified R2 Create"), RegisteredToolSchema.Contains(TEXT("CreateNewVehicleRecords")));
	TestFalse(TEXT("Existing raw CreateVehicleRecords remains private"), RegisteredToolSchema.Contains(TEXT("\"CreateVehicleRecords\"")));
	TestFalse(TEXT("G1 Toolset does not expose Save"), RegisteredToolSchema.Contains(TEXT("SaveVehicle")));

	// Core6 예시의 결정적 경로를 기존 작성 규칙에서 가져옵니다.
	const FCFVehicleIdentityToolResult ValidResult = UCFVehicleAuthoringToolset::PreviewVehicleRecordIdentity(TEXT("Morning27_KR"));
	TestTrue(TEXT("G1 canonical Morning ID accepted"), ValidResult.bValid);
	TestEqual(TEXT("G1 exact Definition object name"), ValidResult.DefinitionAssetName, FString(TEXT("DA_Vehicle_Morning27_KR")));
	TestEqual(TEXT("G1 exact Recipe object name"), ValidResult.RecipeAssetName, FString(TEXT("DA_Recipe_Morning27_KR")));
	TestEqual(TEXT("G1 exact Definition package"), ValidResult.DefinitionPackageName, FString(TEXT("/Game/CarFight/Data/Authoring/DA_Vehicle_Morning27_KR")));
	TestEqual(TEXT("G1 exact Recipe package"), ValidResult.RecipePackageName, FString(TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_Morning27_KR")));

	// 기존 VehicleId가 거절하는 비ASCII 문자가 공개 Toolset에서도 동일하게 거부되는지 확인합니다.
	const FCFVehicleIdentityToolResult InvalidResult = UCFVehicleAuthoringToolset::PreviewVehicleRecordIdentity(TEXT("Morning-27"));
	TestFalse(TEXT("G1 invalid VehicleId rejected"), InvalidResult.bValid);
	TestTrue(TEXT("G1 invalid VehicleId has a diagnostic"), !InvalidResult.ErrorMessage.IsEmpty());
	TestTrue(TEXT("G1 invalid VehicleId never proposes packages"), InvalidResult.DefinitionPackageName.IsEmpty() && InvalidResult.RecipePackageName.IsEmpty());

	// AI 직접 호출은 실행 증명이 없으면 네트워크 조회 이전에 거부하며 패키지를 생성하지 않습니다.
	const FString UniqueId = TEXT("ProofDenied_") + FGuid::NewGuid().ToString(EGuidFormats::Digits);
	const FString Candidate = FString::Printf(TEXT("{\"VehicleId\":\"%s\"}"), *UniqueId);
	const FCFVehicleCreateToolResult Denied = UCFVehicleAuthoringToolset::CreateNewVehicleRecords(Candidate, TEXT(""));
	TestFalse(TEXT("G2 direct UE Toolset cannot create without proof"), Denied.bSucceeded);
	TestEqual(TEXT("G2 direct unapproved call rejected"), Denied.ErrorCode, FString(TEXT("ExecutionProofRejected")));
	TestNull(TEXT("G2 denied call does not create Definition"), FindPackage(nullptr, *(TEXT("/Game/CarFight/Data/Authoring/DA_Vehicle_") + UniqueId)));
	TestNull(TEXT("G2 denied call does not create Recipe"), FindPackage(nullptr, *(TEXT("/Game/CarFight/Data/Authoring/DA_Recipe_") + UniqueId)));
	const FCFVehicleCreateToolResult Invalid = UCFVehicleAuthoringToolset::CreateNewVehicleRecords(TEXT("{\"VehicleId\":\"x\",\"trusted\":true}"), TEXT(""));
	TestFalse(TEXT("G2 caller-authored trusted JSON never grants authority"), Invalid.bSucceeded);
	TestEqual(TEXT("G2 caller-authored privilege key rejected"), Invalid.ErrorCode, FString(TEXT("InvalidRequest")));
	return !HasAnyErrors();
}

#endif // WITH_DEV_AUTOMATION_TESTS
