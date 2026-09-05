// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVehicleResolver.cpp
// Version: v1.7.0
// Date: 2026-09-03
// Description: DAUTH-P0-08E/F Frozen R0~R16 Pure Resolver + CF-FQ-047 stable Mount legacy passthrough integrity 구현입니다.
// Scope: Snapshot-only source candidate/precedence와 R15 transient Materializer/Validator orchestration을 제공합니다.
// Changelog:
// - v1.7.0: CF-FQ-047 P0-06. active Recipe Mount row의 hidden legacy leaf가 explicit stored override가 없을 때 current same-ID serialized value를 보존하고, 신규 row는 Project Compatibility wildcard struct default를 exact selector fallback으로 materialize해 Resolver/full Target hash 일치를 복원.
// - v1.6.0: Performance Profile의 bUseEngineTorqueCurve/atomic EngineTorqueCurve를 R2에 연결하고 opt-in payload를 공통 Runtime validator로 fail-closed 검증.
// - v1.5.3: SoftClass Profile → hard TSubclassOf Definition 변환에서 /Game Blueprint Generated Class를 UClass qualifier로 고정하던 오류를 교정. /Script native class는 Class, content generated class는 BlueprintGeneratedClass canonical qualifier를 사용해 R15 hash readback을 일치시킴.
// - v1.5.2: Recipe SoftObject reference를 Target hard Object leaf로 encode할 때 target class-qualified canonical text로 정규화해 R15 hash roundtrip을 복원.
// - v1.5.1: R15 Definition hash readback mismatch에 첫 불일치 leaf의 path/type/value 진단을 추가해 원인 추적 가능하게 교정.
// - v1.5.0: WSA-P0-04 공용 Wheel 계약에 맞춰 SocketScaleFromChassis R6에서 FR/RL/RR 미지정 시 FL Wheel Snapshot을 deterministic fallback으로 재사용.
// - v1.4.0: WSA-P0-02 SocketScaleFromChassis semantic flag, R5 RelativeScale candidate, R6 Bounds+SocketScale derived Radius/Width, narrow fingerprint와 axle consistency validation 추가.
// - v1.3.0: 설계 검수 교정으로 TransmissionRatios를 Forward/Reverse 분리 leaf가 아닌 atomic typed ratio-set으로 복원하고 Shift RPM integer semantic을 fail-closed 검증.
// - v1.2.0: VB-P0-05 VehicleBase Reference wheel fallback opt-in, Drivetrain Transmission complete payload opt-in, nested ratio-set mapping과 positive reverse-ratio fail-closed validation 추가.
// - v1.1.0: DAUTH-P0-08F R15을 Completed stage로 구현하고 Definition Snapshot 공용 hash authority와 materialized readback consistency를 연결.
// - v1.0.0: Section 22.22~22.39 immutable Resolver input/output과 deterministic candidate stack 최초 구현.
// Migration:
// - Resolver contract revision 2부터 기존 Base/Drivetrain Profile은 opt-in bool이 false이면 새 wheel/Transmission candidate를 만들지 않습니다.
// - ReverseGearRatios의 0/음수 값은 abs 보정하지 않고 Block합니다. setup array는 positive magnitude storage만 허용합니다.
// - R0~R14 Source 계산은 계속 live UObject/StaticMesh/Slate를 재조회하지 않습니다.
// - R15는 Frozen 계약대로 transient UCFVehicleData에만 materialize하여 기존 UCFVDAValidator를 읽기 전용 실행합니다.
// - Runtime schema/Content Asset을 수정하지 않으며 Apply/UI/CSV는 구현하지 않습니다.

#include "DataAuthoring/CFVehicleResolver.h"

#include "CFVehicleData.h"
#include "CFVehicleEngineCurveUtils.h"
#include "CFWheelSizeUtils.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFVehicleFieldCodec.h"
#include "DataAuthoring/CFVehicleFieldRegistry.h"
#include "DataAuthoring/CFVehicleMaterializer.h"
#include "DataAuthoring/CFVehicleSnapshotBuilder.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Misc/SecureHash.h"
#include "UObject/UnrealType.h"

namespace CFVehicleResolverPrivate
{
	// Resolver hash canonical payload format revision입니다.
	static constexpr int32 ResolverHashFormatRevision = 1;

	// Field 하나에 쌓인 immutable candidate layer입니다.
	struct FVehicleFieldCandidate
	{
		// Candidate가 대상으로 하는 exact stable field path입니다.
		FCFVehicleFieldPath FieldPath;

		// Candidate canonical typed value입니다.
		FCFVehicleFieldValue Value;

		// Candidate source 종류입니다.
		ECFVehicleSourceType SourceType = ECFVehicleSourceType::ProjectCompatibilityDefault;

		// Source asset/path/rule identity입니다.
		FString SourceId;

		// 사람이 읽는 diagnostic revision입니다.
		int32 SourceRevision = 0;

		// Resolver가 실제 소비한 source payload fingerprint입니다.
		FString SourceFingerprint;

		// Frozen precedence rank입니다.
		int32 Precedence = 0;

		// 이 field의 Frozen Registry descriptor입니다.
		const FCFVehicleFieldDescriptor* Descriptor = nullptr;
	};

	// Resolve 한 번의 mutable working state이며 public output에는 UObject pointer가 남지 않습니다.
	struct FResolverContext
	{
		// Immutable request reference입니다.
		const FCFVehicleResolveRequest& Request;

		// 최종 public result입니다.
		FCFVehicleResolveResult& Result;

		// Canonical exact path별 candidate stack입니다.
		TMap<FString, TArray<FVehicleFieldCandidate>> CandidateStacks;

		// 내부 pipeline error가 발생했는지 여부입니다.
		bool bHasError = false;

		// Validation이 Resolve를 Blocked로 만들었는지 여부입니다.
		bool bHasBlocked = false;
	};

	// UTF-8 canonical payload를 lowercase MD5 hexadecimal digest로 변환합니다.
	FString HashUtf8Payload(const FString& Payload)
	{
		// TCHAR 표현과 분리된 canonical UTF-8 payload입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// UTF-8 bytes의 deterministic digest를 계산할 MD5 state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());
		// MD5 128-bit 결과 bytes입니다.
		uint8 Digest[16];
		Md5.Final(Digest);

		// Platform-independent lowercase hexadecimal 결과 문자열입니다.
		FString HexResult;
		HexResult.Reserve(32);
		// 한 nibble을 lowercase hex로 바꾸는 고정 문자표입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			HexResult.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			HexResult.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return HexResult;
	}

	// Delimiter 충돌 없이 label/문자수/value token을 canonical payload에 추가합니다.
	void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("\n");
	}

	// Frozen general precedence를 stage order와 독립된 rank로 반환합니다.
	int32 GetSourcePrecedence(const ECFVehicleSourceType SourceType)
	{
		switch (SourceType)
		{
		case ECFVehicleSourceType::ProjectCompatibilityDefault:
			return 10;
		case ECFVehicleSourceType::VehicleBaseProfile:
		case ECFVehicleSourceType::DrivetrainProfile:
		case ECFVehicleSourceType::HandlingProfile:
		case ECFVehicleSourceType::PerformanceProfile:
		case ECFVehicleSourceType::DriveStateProfile:
			return 20;
		case ECFVehicleSourceType::RuleDerived:
		case ECFVehicleSourceType::AssetDerived:
			return 30;
		case ECFVehicleSourceType::RecipeExplicitSemanticInput:
			return 40;
		case ECFVehicleSourceType::LegacyImportedPinnedBaseline:
			return 50;
		case ECFVehicleSourceType::LegacySerializedPassthrough:
			return 55;
		case ECFVehicleSourceType::AdvancedLeafOverride:
			return 60;
		default:
			return INDEX_NONE;
		}
	}

	// Source type 하나를 Registry AllowedSourceMask bit로 변환합니다.
	uint64 SourceBit(const ECFVehicleSourceType SourceType)
	{
		return 1ull << static_cast<uint8>(SourceType);
	}

	// Exact array selector를 wildcard로 바꾼 Registry canonical pattern을 반환합니다.
	FString GetRegistryPatternForExactPath(const FCFVehicleFieldPath& ExactPath)
	{
		// Exact selector만 제거한 Registry pattern path입니다.
		FCFVehicleFieldPath PatternPath = ExactPath;
		if (!PatternPath.CollectionPropertyName.IsNone())
		{
			PatternPath.SelectorKeyValue = NAME_None;
		}
		return PatternPath.ToCanonicalString(true);
	}

	// Exact stable path에 해당하는 Frozen Registry descriptor를 찾습니다.
	const FCFVehicleFieldDescriptor* FindDescriptor(const FCFVehicleFieldPath& ExactPath)
	{
		// Exact path에서 만든 wildcard Registry identity입니다.
		const FString Pattern = GetRegistryPatternForExactPath(ExactPath);
		return FCFVehicleFieldRegistry::GetDescriptors().FindByPredicate([&Pattern](const FCFVehicleFieldDescriptor& Descriptor)
		{
			return Descriptor.GetCanonicalPattern() == Pattern;
		});
	}

	// Registry path가 가리키는 Runtime Definition leaf FProperty를 schema에서만 찾습니다.
	const FProperty* FindTargetLeafProperty(const FCFVehicleFieldPath& FieldPath)
	{
		// Property traversal을 시작할 current struct/class입니다.
		const UStruct* CurrentStruct = UCFVehicleData::StaticClass();

		if (!FieldPath.CollectionPropertyName.IsNone())
		{
			// Top-level stable collection property입니다.
			const FArrayProperty* ArrayProperty = FindFProperty<FArrayProperty>(UCFVehicleData::StaticClass(), FieldPath.CollectionPropertyName);
			if (!ArrayProperty)
			{
				return nullptr;
			}

			// Stable collection의 struct element property입니다.
			const FStructProperty* InnerStructProperty = CastField<FStructProperty>(ArrayProperty->Inner);
			if (!InnerStructProperty || !InnerStructProperty->Struct)
			{
				return nullptr;
			}
			CurrentStruct = InnerStructProperty->Struct;
		}

		for (int32 PropertyIndex = 0; PropertyIndex < FieldPath.PropertyChain.Num(); ++PropertyIndex)
		{
			// 현재 traversal step의 property 이름입니다.
			const FName PropertyName = FieldPath.PropertyChain[PropertyIndex];
			// Current struct에서 찾은 reflected property입니다.
			const FProperty* Property = FindFProperty<FProperty>(CurrentStruct, PropertyName);
			if (!Property)
			{
				return nullptr;
			}

			if (PropertyIndex == FieldPath.PropertyChain.Num() - 1)
			{
				return Property;
			}

			// 다음 nested step으로 진입하기 위한 struct property입니다.
			const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (!StructProperty || !StructProperty->Struct)
			{
				return nullptr;
			}
			CurrentStruct = StructProperty->Struct;
		}

		return nullptr;
	}

	// Source property text를 Target Definition leaf type signature에 맞춘 canonical value로 복사합니다.
	bool EncodeSourcePropertyAsTarget(
		const FCFVehicleFieldPath& TargetPath,
		const FProperty& SourceProperty,
		const void* SourceValueAddress,
		FCFVehicleFieldValue& OutValue,
		FString& OutError)
	{
		// Target Definition schema leaf property입니다.
		const FProperty* TargetProperty = FindTargetLeafProperty(TargetPath);
		if (!TargetProperty)
		{
			OutError = FString::Printf(TEXT("Target Definition property를 찾을 수 없습니다: %s"), *TargetPath.ToCanonicalString(true));
			return false;
		}

		// Source property의 Unreal canonical export 결과입니다.
		FCFVehicleFieldValue SourceValue;
		if (!FCFVehicleFieldCodec::ExportValue(SourceProperty, SourceValueAddress, SourceValue, OutError))
		{
			return false;
		}

		// Target Definition leaf의 exact reflection type signature입니다.
		OutValue.PropertyTypeSignature = FCFVehicleFieldCodec::BuildTypeSignature(*TargetProperty);

		// Builder Drivetrain Profile의 SoftClass를 Target TSubclassOf hard Class leaf로 투영하는 source/target property입니다.
		const FSoftClassProperty* SourceSoftClassProperty = CastField<FSoftClassProperty>(&SourceProperty);
		const FClassProperty* TargetClassProperty = CastField<FClassProperty>(TargetProperty);
		if (SourceSoftClassProperty && TargetClassProperty)
		{
			if (!SourceSoftClassProperty->MetaClass
				|| !TargetClassProperty->MetaClass
				|| !SourceSoftClassProperty->MetaClass->IsChildOf(TargetClassProperty->MetaClass))
			{
				OutError = FString::Printf(
					TEXT("SoftClass source MetaClass와 Target Class MetaClass가 호환되지 않습니다: Source=%s Target=%s"),
					*GetPathNameSafe(SourceSoftClassProperty->MetaClass),
					*GetPathNameSafe(TargetClassProperty->MetaClass));
				return false;
			}

			const FString SourceClassPathText = SourceValue.CanonicalValueText.TrimStartAndEnd();
			if (SourceClassPathText.IsEmpty() || SourceClassPathText == TEXT("None"))
			{
				OutValue.CanonicalValueText = TEXT("None");
				OutError.Reset();
				return true;
			}

			const FSoftObjectPath SourceClassPath(SourceClassPathText);
			if (!SourceClassPath.IsValid())
			{
				OutError = FString::Printf(TEXT("SoftClass canonical path가 유효하지 않습니다: %s"), *SourceClassPathText);
				return false;
			}

			// Native /Script class object은 UClass, Content Blueprint generated class object은 UBlueprintGeneratedClass로 ExportText됩니다.
			// Asset을 live load하지 않고 snapshot class path 종류만으로 동일 canonical qualifier를 선택합니다.
			const UClass* ClassObjectType = SourceClassPath.ToString().StartsWith(TEXT("/Script/"))
				? UClass::StaticClass()
				: UBlueprintGeneratedClass::StaticClass();
			OutValue.CanonicalValueText = FString::Printf(
				TEXT("%s'%s'"),
				*GetPathNameSafe(ClassObjectType),
				*SourceClassPath.ToString());
			OutError.Reset();
			return true;
		}

		// Recipe AssetIntent처럼 SoftObject reference를 hard Object Definition leaf로 투영하는 source property입니다.
		const FSoftObjectProperty* SourceSoftObjectProperty = CastField<FSoftObjectProperty>(&SourceProperty);
		// Target VehicleData의 hard Object reference property입니다.
		const FObjectPropertyBase* TargetObjectProperty = CastField<FObjectPropertyBase>(TargetProperty);
		if (SourceSoftObjectProperty && TargetObjectProperty)
		{
			// Source/Target object class가 reflection 상 호환되는지 먼저 확인합니다.
			if (!SourceSoftObjectProperty->PropertyClass
				|| !TargetObjectProperty->PropertyClass
				|| !SourceSoftObjectProperty->PropertyClass->IsChildOf(TargetObjectProperty->PropertyClass))
			{
				OutError = FString::Printf(
					TEXT("SoftObject source class와 Target Object class가 호환되지 않습니다: Source=%s Target=%s"),
					*GetPathNameSafe(SourceSoftObjectProperty->PropertyClass),
					*GetPathNameSafe(TargetObjectProperty->PropertyClass));
				return false;
			}

			// SoftObject export text의 공백을 제거한 source object path text입니다.
			const FString SourceObjectPathText = SourceValue.CanonicalValueText.TrimStartAndEnd();
			if (SourceObjectPathText.IsEmpty() || SourceObjectPathText == TEXT("None"))
			{
				OutValue.CanonicalValueText = TEXT("None");
				OutError.Reset();
				return true;
			}

			// Snapshot text만 사용해 복원한 soft object path이며 live UObject load를 수행하지 않습니다.
			const FSoftObjectPath SourceObjectPath(SourceObjectPathText);
			if (!SourceObjectPath.IsValid())
			{
				OutError = FString::Printf(TEXT("SoftObject canonical path가 유효하지 않습니다: %s"), *SourceObjectPathText);
				return false;
			}

			// Target hard Object property의 ExportText 형식과 동일한 class-qualified canonical reference입니다.
			OutValue.CanonicalValueText = FString::Printf(
				TEXT("%s'%s'"),
				*GetPathNameSafe(TargetObjectProperty->PropertyClass),
				*SourceObjectPath.ToString());
			OutError.Reset();
			return true;
		}

		// Source/Target이 별도 reference adaptation을 요구하지 않는 기존 canonical text 경로입니다.
		OutValue.CanonicalValueText = MoveTemp(SourceValue.CanonicalValueText);
		OutError.Reset();
		return true;
	}

	// Target Definition leaf property 자체를 사용해 compatible native value를 canonical encode합니다.
	bool EncodeNativeTargetValue(
		const FCFVehicleFieldPath& TargetPath,
		const void* ValueAddress,
		FCFVehicleFieldValue& OutValue,
		FString& OutError)
	{
		// Native storage layout과 호환되는 target Definition leaf property입니다.
		const FProperty* TargetProperty = FindTargetLeafProperty(TargetPath);
		if (!TargetProperty)
		{
			OutError = FString::Printf(TEXT("Target Definition property를 찾을 수 없습니다: %s"), *TargetPath.ToCanonicalString(true));
			return false;
		}
		return FCFVehicleFieldCodec::ExportValue(*TargetProperty, ValueAddress, OutValue, OutError);
	}

	// Target object/class leaf의 explicit None canonical value를 만듭니다.
	bool EncodeTargetNone(
		const FCFVehicleFieldPath& TargetPath,
		FCFVehicleFieldValue& OutValue,
		FString& OutError)
	{
		// None value의 type authority가 되는 target property입니다.
		const FProperty* TargetProperty = FindTargetLeafProperty(TargetPath);
		if (!TargetProperty)
		{
			OutError = FString::Printf(TEXT("Target Definition property를 찾을 수 없습니다: %s"), *TargetPath.ToCanonicalString(true));
			return false;
		}
		OutValue.PropertyTypeSignature = FCFVehicleFieldCodec::BuildTypeSignature(*TargetProperty);
		OutValue.CanonicalValueText = TEXT("None");
		OutError.Reset();
		return true;
	}

	// Resolver issue를 지정 validation bucket에 추가하고 aggregate state를 갱신합니다.
	void AddIssue(
		FResolverContext& Context,
		TArray<FCFVehicleValidationIssue>& IssueBucket,
		const ECFVehicleValidationSeverity Severity,
		const FName IssueCode,
		const FString& Message,
		const FCFVehicleFieldPath* FieldPath = nullptr)
	{
		// 새 validation issue row입니다.
		FCFVehicleValidationIssue& Issue = IssueBucket.AddDefaulted_GetRef();
		Issue.Severity = Severity;
		Issue.IssueCode = IssueCode;
		Issue.Message = Message;
		if (FieldPath)
		{
			Issue.FieldPath = *FieldPath;
		}

		if (Severity == ECFVehicleValidationSeverity::Error)
		{
			Context.bHasError = true;
		}
		else if (Severity == ECFVehicleValidationSeverity::Blocked)
		{
			Context.bHasBlocked = true;
		}
	}

	// Field candidate를 Registry mask와 same-precedence conflict rule을 검사한 뒤 stack에 추가합니다.
	bool AddCandidate(
		FResolverContext& Context,
		const FCFVehicleFieldPath& FieldPath,
		const FCFVehicleFieldValue& Value,
		const ECFVehicleSourceType SourceType,
		const FString& SourceId,
		const int32 SourceRevision,
		const FString& SourceFingerprint)
	{
		// Candidate field의 Frozen Registry descriptor입니다.
		const FCFVehicleFieldDescriptor* Descriptor = FindDescriptor(FieldPath);
		if (!Descriptor)
		{
			AddIssue(
				Context,
				Context.Result.ResolverValidation,
				ECFVehicleValidationSeverity::Error,
				TEXT("RegistryDescriptorMissing"),
				FString::Printf(TEXT("Resolver candidate에 대응하는 Registry descriptor가 없습니다: %s"), *FieldPath.ToCanonicalString(true)),
				&FieldPath);
			return false;
		}

		if ((Descriptor->AllowedSourceMask & SourceBit(SourceType)) == 0)
		{
			AddIssue(
				Context,
				Context.Result.ResolverValidation,
				ECFVehicleValidationSeverity::Error,
				TEXT("SourceNotAllowed"),
				FString::Printf(TEXT("Frozen Registry가 허용하지 않는 Source candidate입니다: %s / Source=%d"), *FieldPath.ToCanonicalString(true), static_cast<int32>(SourceType)),
				&FieldPath);
			return false;
		}

		// Target schema가 요구하는 exact type signature입니다.
		const FProperty* TargetProperty = FindTargetLeafProperty(FieldPath);
		if (!TargetProperty)
		{
			AddIssue(
				Context,
				Context.Result.ResolverValidation,
				ECFVehicleValidationSeverity::Error,
				TEXT("TargetPropertyMissing"),
				FString::Printf(TEXT("Target Definition schema leaf를 찾을 수 없습니다: %s"), *FieldPath.ToCanonicalString(true)),
				&FieldPath);
			return false;
		}

		// Candidate value가 Target leaf와 type-compatible한지 확인합니다.
		const FString ExpectedTypeSignature = FCFVehicleFieldCodec::BuildTypeSignature(*TargetProperty);
		if (Value.PropertyTypeSignature != ExpectedTypeSignature)
		{
			AddIssue(
				Context,
				Context.Result.ResolverValidation,
				ECFVehicleValidationSeverity::Error,
				TEXT("CandidateTypeMismatch"),
				FString::Printf(TEXT("Candidate type signature가 Target field와 다릅니다: %s"), *FieldPath.ToCanonicalString(true)),
				&FieldPath);
			return false;
		}

		// Frozen precedence rank입니다.
		const int32 Precedence = GetSourcePrecedence(SourceType);
		if (Precedence == INDEX_NONE)
		{
			AddIssue(
				Context,
				Context.Result.ResolverValidation,
				ECFVehicleValidationSeverity::Error,
				TEXT("UnknownSourcePrecedence"),
				TEXT("등록되지 않은 Resolver Source precedence입니다."),
				&FieldPath);
			return false;
		}

		// Candidate stack identity인 exact canonical path입니다.
		const FString CanonicalPath = FieldPath.ToCanonicalString(true);
		// Existing candidate stack입니다.
		TArray<FVehicleFieldCandidate>& Stack = Context.CandidateStacks.FindOrAdd(CanonicalPath);
		if (Stack.ContainsByPredicate([Precedence](const FVehicleFieldCandidate& ExistingCandidate)
		{
			return ExistingCandidate.Precedence == Precedence;
		}))
		{
			AddIssue(
				Context,
				Context.Result.ResolverValidation,
				ECFVehicleValidationSeverity::Error,
				TEXT("SamePrecedenceConflict"),
				FString::Printf(TEXT("동일 precedence의 두 Source가 같은 field를 동시에 소유하려 했습니다: %s / Rank=%d"), *CanonicalPath, Precedence),
				&FieldPath);
			return false;
		}

		// Stack에 추가할 immutable candidate입니다.
		FVehicleFieldCandidate Candidate;
		Candidate.FieldPath = FieldPath;
		Candidate.Value = Value;
		Candidate.SourceType = SourceType;
		Candidate.SourceId = SourceId;
		Candidate.SourceRevision = SourceRevision;
		Candidate.SourceFingerprint = SourceFingerprint;
		Candidate.Precedence = Precedence;
		Candidate.Descriptor = Descriptor;
		Stack.Add(MoveTemp(Candidate));
		return true;
	}

	// 현재까지 쌓인 candidate 중 Frozen precedence winner를 반환합니다.
	const FVehicleFieldCandidate* FindCurrentWinner(const FResolverContext& Context, const FString& CanonicalPath)
	{
		// 해당 exact field의 candidate stack입니다.
		const TArray<FVehicleFieldCandidate>* Stack = Context.CandidateStacks.Find(CanonicalPath);
		if (!Stack || Stack->IsEmpty())
		{
			return nullptr;
		}

		// 가장 높은 precedence candidate입니다.
		const FVehicleFieldCandidate* Winner = &(*Stack)[0];
		for (const FVehicleFieldCandidate& Candidate : *Stack)
		{
			if (Candidate.Precedence > Winner->Precedence)
			{
				Winner = &Candidate;
			}
		}
		return Winner;
	}

	// Canonical scalar path를 structured exact field path로 찾습니다.
	FCFVehicleFieldPath FindScalarPath(const TCHAR* CanonicalPath)
	{
		// Registry에서 scalar canonical path와 일치하는 descriptor입니다.
		const FCFVehicleFieldDescriptor* Descriptor = FCFVehicleFieldRegistry::GetDescriptors().FindByPredicate([CanonicalPath](const FCFVehicleFieldDescriptor& Candidate)
		{
			return Candidate.GetCanonicalPattern() == CanonicalPath;
		});
		return Descriptor ? Descriptor->StablePathPattern : FCFVehicleFieldPath();
	}

	// Stable collection descriptor pattern에 selector value를 넣어 exact field path를 만듭니다.
	FCFVehicleFieldPath MakeExactCollectionPath(const TCHAR* CanonicalPattern, const FName SelectorValue)
	{
		// Frozen wildcard descriptor입니다.
		const FCFVehicleFieldDescriptor* Descriptor = FCFVehicleFieldRegistry::GetDescriptors().FindByPredicate([CanonicalPattern](const FCFVehicleFieldDescriptor& Candidate)
		{
			return Candidate.GetCanonicalPattern() == CanonicalPattern;
		});
		// Exact selector를 채운 result path입니다.
		FCFVehicleFieldPath Result = Descriptor ? Descriptor->StablePathPattern : FCFVehicleFieldPath();
		Result.SelectorKeyValue = SelectorValue;
		return Result;
	}

	// Reflected source struct의 property 하나를 target field candidate로 추가합니다.
	bool AddStructPropertyCandidate(
		FResolverContext& Context,
		const FCFVehicleFieldPath& TargetPath,
		const UScriptStruct& SourceStruct,
		const void* SourceStructAddress,
		const FName SourcePropertyName,
		const ECFVehicleSourceType SourceType,
		const FString& SourceId,
		const int32 SourceRevision,
		const FString& SourceFingerprint)
	{
		// Typed source payload property입니다.
		const FProperty* SourceProperty = FindFProperty<FProperty>(&SourceStruct, SourcePropertyName);
		if (!SourceProperty)
		{
			AddIssue(
				Context,
				Context.Result.ResolverValidation,
				ECFVehicleValidationSeverity::Error,
				TEXT("SourcePropertyMissing"),
				FString::Printf(TEXT("Typed source property를 찾을 수 없습니다: %s.%s"), *SourceStruct.GetName(), *SourcePropertyName.ToString()),
				&TargetPath);
			return false;
		}

		// Typed source struct 안의 실제 value address입니다.
		const void* SourceValueAddress = SourceProperty->ContainerPtrToValuePtr<void>(SourceStructAddress);
		// Target signature를 적용한 canonical candidate value입니다.
		FCFVehicleFieldValue EncodedValue;
		// Reflection export 실패 이유입니다.
		FString EncodeError;
		if (!EncodeSourcePropertyAsTarget(TargetPath, *SourceProperty, SourceValueAddress, EncodedValue, EncodeError))
		{
			AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Error, TEXT("SourceEncodeFailed"), EncodeError, &TargetPath);
			return false;
		}
		return AddCandidate(Context, TargetPath, EncodedValue, SourceType, SourceId, SourceRevision, SourceFingerprint);
	}

	// Native float candidate를 target float field에 추가합니다.
	bool AddFloatCandidate(
		FResolverContext& Context,
		const FCFVehicleFieldPath& TargetPath,
		const float Value,
		const ECFVehicleSourceType SourceType,
		const FString& SourceId,
		const int32 SourceRevision,
		const FString& SourceFingerprint)
	{
		// Target property codec으로 만든 typed float value입니다.
		FCFVehicleFieldValue EncodedValue;
		// Encoding 실패 이유입니다.
		FString EncodeError;
		if (!EncodeNativeTargetValue(TargetPath, &Value, EncodedValue, EncodeError))
		{
			AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Error, TEXT("FloatEncodeFailed"), EncodeError, &TargetPath);
			return false;
		}
		return AddCandidate(Context, TargetPath, EncodedValue, SourceType, SourceId, SourceRevision, SourceFingerprint);
	}

	// Native bool candidate를 target bool field에 추가합니다.
	bool AddBoolCandidate(
		FResolverContext& Context,
		const FCFVehicleFieldPath& TargetPath,
		const bool Value,
		const ECFVehicleSourceType SourceType,
		const FString& SourceId,
		const int32 SourceRevision,
		const FString& SourceFingerprint)
	{
		// Target property codec으로 만든 typed bool value입니다.
		FCFVehicleFieldValue EncodedValue;
		// Encoding 실패 이유입니다.
		FString EncodeError;
		if (!EncodeNativeTargetValue(TargetPath, &Value, EncodedValue, EncodeError))
		{
			AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Error, TEXT("BoolEncodeFailed"), EncodeError, &TargetPath);
			return false;
		}
		return AddCandidate(Context, TargetPath, EncodedValue, SourceType, SourceId, SourceRevision, SourceFingerprint);
	}

	// Native FVector candidate를 target vector field에 추가합니다.
	bool AddVectorCandidate(
		FResolverContext& Context,
		const FCFVehicleFieldPath& TargetPath,
		const FVector& Value,
		const ECFVehicleSourceType SourceType,
		const FString& SourceId,
		const int32 SourceRevision,
		const FString& SourceFingerprint)
	{
		// Target property codec으로 만든 typed vector value입니다.
		FCFVehicleFieldValue EncodedValue;
		// Encoding 실패 이유입니다.
		FString EncodeError;
		if (!EncodeNativeTargetValue(TargetPath, &Value, EncodedValue, EncodeError))
		{
			AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Error, TEXT("VectorEncodeFailed"), EncodeError, &TargetPath);
			return false;
		}
		return AddCandidate(Context, TargetPath, EncodedValue, SourceType, SourceId, SourceRevision, SourceFingerprint);
	}

	// Native FRotator candidate를 target rotator field에 추가합니다.
	bool AddRotatorCandidate(
		FResolverContext& Context,
		const FCFVehicleFieldPath& TargetPath,
		const FRotator& Value,
		const ECFVehicleSourceType SourceType,
		const FString& SourceId,
		const int32 SourceRevision,
		const FString& SourceFingerprint)
	{
		// Target property codec으로 만든 typed rotator value입니다.
		FCFVehicleFieldValue EncodedValue;
		// Encoding 실패 이유입니다.
		FString EncodeError;
		if (!EncodeNativeTargetValue(TargetPath, &Value, EncodedValue, EncodeError))
		{
			AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Error, TEXT("RotatorEncodeFailed"), EncodeError, &TargetPath);
			return false;
		}
		return AddCandidate(Context, TargetPath, EncodedValue, SourceType, SourceId, SourceRevision, SourceFingerprint);
	}

	// Resolver stage record를 Frozen 실행 순서대로 추가합니다.
	void RecordStage(FResolverContext& Context, const ECFVehicleResolverStage Stage, const ECFVehicleResolverStageStatus Status = ECFVehicleResolverStageStatus::Completed)
	{
		// Stage execution record입니다.
		FCFVehicleResolverStageRecord& Record = Context.Result.StageRecords.AddDefaulted_GetRef();
		Record.Stage = Stage;
		Record.Status = Status;
	}

	// Profile binding path와 실제 Snapshot source path가 일치하는지 검사합니다.
	void ValidateProfileBinding(
		FResolverContext& Context,
		const TCHAR* Label,
		const FSoftObjectPath& BoundPath,
		const FCFVehicleProfileSource& Source,
		const bool bRequired)
	{
		if (bRequired && !Source.IsPresent())
		{
			AddIssue(
				Context,
				Context.Result.RecipeValidation,
				ECFVehicleValidationSeverity::Blocked,
				TEXT("RequiredProfileMissing"),
				FString::Printf(TEXT("Managed Resolve에 필요한 %s Profile Snapshot이 없습니다."), Label));
			return;
		}

		if (BoundPath.IsValid() && Source.IsPresent() && BoundPath != Source.SourceObjectPath)
		{
			AddIssue(
				Context,
				Context.Result.RecipeValidation,
				ECFVehicleValidationSeverity::Error,
				TEXT("ProfileBindingMismatch"),
				FString::Printf(TEXT("Recipe binding과 %s Profile Snapshot identity가 다릅니다."), Label));
		}
	}

	// Override-like array에서 duplicate exact path를 검사합니다.
	void ValidateOverrideDuplicates(
		FResolverContext& Context,
		const TArray<FCFVehicleFieldOverride>& Overrides,
		const TCHAR* Label)
	{
		// Duplicate detection용 canonical exact path 집합입니다.
		TSet<FString> SeenPaths;
		for (const FCFVehicleFieldOverride& Override : Overrides)
		{
			// Override exact canonical path입니다.
			const FString CanonicalPath = Override.FieldPath.ToCanonicalString(true);
			if (SeenPaths.Contains(CanonicalPath))
			{
				AddIssue(
					Context,
					Context.Result.RecipeValidation,
					ECFVehicleValidationSeverity::Error,
					TEXT("DuplicateOverridePath"),
					FString::Printf(TEXT("%s에 duplicate field path가 있습니다: %s"), Label, *CanonicalPath),
					&Override.FieldPath);
			}
			SeenPaths.Add(CanonicalPath);
		}
	}

	// R0 Request / Recipe / Binding validation을 수행합니다.
	void RunR0RequestValidation(FResolverContext& Context)
	{
		if (Context.Request.ResolverContractRevision != FCFVehicleResolver::CurrentResolverContractRevision)
		{
			AddIssue(
				Context,
				Context.Result.RecipeValidation,
				ECFVehicleValidationSeverity::Blocked,
				TEXT("ResolverContractRevisionMismatch"),
				FString::Printf(TEXT("Resolver Contract Revision이 current contract와 다릅니다. Request=%d Current=%d"), Context.Request.ResolverContractRevision, FCFVehicleResolver::CurrentResolverContractRevision));
		}

		// Fully managed Recipe인지 여부입니다.
		const bool bManaged = Context.Request.Recipe.ImportState.ManageState == ECFVehicleManageState::Managed;
		ValidateProfileBinding(Context, TEXT("Vehicle Base"), Context.Request.Recipe.ProfileBindings.VehicleBaseProfile.ToSoftObjectPath(), Context.Request.Profiles.BaseSource, bManaged);
		ValidateProfileBinding(Context, TEXT("Drivetrain"), Context.Request.Recipe.ProfileBindings.DrivetrainProfile.ToSoftObjectPath(), Context.Request.Profiles.DrivetrainSource, bManaged);
		ValidateProfileBinding(Context, TEXT("Handling"), Context.Request.Recipe.ProfileBindings.HandlingProfile.ToSoftObjectPath(), Context.Request.Profiles.HandlingSource, bManaged);
		ValidateProfileBinding(Context, TEXT("Performance"), Context.Request.Recipe.ProfileBindings.PerformanceProfile.ToSoftObjectPath(), Context.Request.Profiles.PerformanceSource, bManaged);

		// Vehicle-specific DriveState가 실제 profile source를 요구하는지 여부입니다.
		const bool bDriveStateProfileRequired = Context.Request.Recipe.DriveStateMode == ECFVehicleDriveStateMode::VehicleSpecific;
		ValidateProfileBinding(Context, TEXT("DriveState"), Context.Request.Recipe.ProfileBindings.DriveStateProfile.ToSoftObjectPath(), Context.Request.Profiles.DriveStateSource, bDriveStateProfileRequired);

		// Recipe Hardpoint stable ID uniqueness 검사 집합입니다.
		TSet<FName> HardpointIds;
		for (const FCFHardpointIntent& HardpointIntent : Context.Request.Recipe.HardpointIntents)
		{
			if (HardpointIntent.LocationSlotId.IsNone())
			{
				AddIssue(Context, Context.Result.RecipeValidation, ECFVehicleValidationSeverity::Error, TEXT("HardpointIdMissing"), TEXT("Hardpoint Intent의 LocationSlotId가 None입니다."));
				continue;
			}
			if (HardpointIds.Contains(HardpointIntent.LocationSlotId))
			{
				AddIssue(Context, Context.Result.RecipeValidation, ECFVehicleValidationSeverity::Error, TEXT("DuplicateHardpointId"), FString::Printf(TEXT("Duplicate Hardpoint LocationSlotId입니다: %s"), *HardpointIntent.LocationSlotId.ToString()));
			}
			HardpointIds.Add(HardpointIntent.LocationSlotId);
		}

		// Recipe Mount stable ID uniqueness 검사 집합입니다.
		TSet<FName> MountIds;
		for (const FCFMountIntent& MountIntent : Context.Request.Recipe.MountIntents)
		{
			if (MountIntent.MountProfileId.IsNone())
			{
				AddIssue(Context, Context.Result.RecipeValidation, ECFVehicleValidationSeverity::Error, TEXT("MountIdMissing"), TEXT("Mount Intent의 MountProfileId가 None입니다."));
				continue;
			}
			if (MountIds.Contains(MountIntent.MountProfileId))
			{
				AddIssue(Context, Context.Result.RecipeValidation, ECFVehicleValidationSeverity::Error, TEXT("DuplicateMountId"), FString::Printf(TEXT("Duplicate MountProfileId입니다: %s"), *MountIntent.MountProfileId.ToString()));
			}
			if (!MountIntent.LocationSlotRef.IsNone() && !HardpointIds.Contains(MountIntent.LocationSlotRef))
			{
				AddIssue(Context, Context.Result.RecipeValidation, ECFVehicleValidationSeverity::Blocked, TEXT("MountHardpointMissing"), FString::Printf(TEXT("Mount가 존재하지 않는 Hardpoint를 참조합니다: %s -> %s"), *MountIntent.MountProfileId.ToString(), *MountIntent.LocationSlotRef.ToString()));
			}
			MountIds.Add(MountIntent.MountProfileId);
		}

		ValidateOverrideDuplicates(Context, Context.Request.Recipe.AdvancedOverrides, TEXT("AdvancedOverrides"));
		ValidateOverrideDuplicates(Context, Context.Request.Recipe.ImportState.LegacyPinnedFields, TEXT("LegacyPinnedFields"));
		ValidateOverrideDuplicates(Context, Context.Request.Recipe.ImportState.LegacySerializedFields, TEXT("LegacySerializedFields"));

		for (const FCFVehicleFieldOverride& Override : Context.Request.Recipe.AdvancedOverrides)
		{
			// Advanced Override 대상 Registry descriptor입니다.
			const FCFVehicleFieldDescriptor* Descriptor = FindDescriptor(Override.FieldPath);
			if (!Descriptor || !Descriptor->bAdvancedOverrideAllowed || Descriptor->bIdentityField || Descriptor->bLegacySerialized)
			{
				AddIssue(Context, Context.Result.RecipeValidation, ECFVehicleValidationSeverity::Error, TEXT("AdvancedOverrideNotAllowed"), FString::Printf(TEXT("Advanced Override가 허용되지 않는 field입니다: %s"), *Override.FieldPath.ToCanonicalString(true)), &Override.FieldPath);
				continue;
			}

			// Advanced Override target leaf schema입니다.
			const FProperty* TargetProperty = FindTargetLeafProperty(Override.FieldPath);
			if (!TargetProperty || Override.OverrideValue.PropertyTypeSignature != FCFVehicleFieldCodec::BuildTypeSignature(*TargetProperty))
			{
				AddIssue(Context, Context.Result.RecipeValidation, ECFVehicleValidationSeverity::Error, TEXT("AdvancedOverrideTypeMismatch"), FString::Printf(TEXT("Advanced Override type signature가 target field와 다릅니다: %s"), *Override.FieldPath.ToCanonicalString(true)), &Override.FieldPath);
			}
		}
	}

	// R1 Project Compatibility Default baseline을 scalar/nested field에 적용합니다.
	void RunR1ProjectDefaults(FResolverContext& Context)
	{
		for (const FCFVehicleFieldEntry& Entry : Context.Request.ProjectDefaults.SortedFields)
		{
			// Wildcard collection default는 실제 stable element가 아니므로 candidate stack을 만들지 않습니다.
			if (!Entry.FieldPath.CollectionPropertyName.IsNone() && Entry.FieldPath.SelectorKeyValue.IsNone())
			{
				continue;
			}

			AddCandidate(
				Context,
				Entry.FieldPath,
				Entry.Value,
				ECFVehicleSourceType::ProjectCompatibilityDefault,
				TEXT("Project.CppDefaults"),
				0,
				FCFVehicleFieldCodec::HashValue(Entry.Value));
		}
	}

	// Profile Domain에 대응하는 public SourceType을 반환합니다.
	ECFVehicleSourceType GetProfileSourceType(const ECFVehicleProfileDomain Domain)
	{
		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			return ECFVehicleSourceType::VehicleBaseProfile;
		case ECFVehicleProfileDomain::Drivetrain:
			return ECFVehicleSourceType::DrivetrainProfile;
		case ECFVehicleProfileDomain::Handling:
			return ECFVehicleSourceType::HandlingProfile;
		case ECFVehicleProfileDomain::Performance:
			return ECFVehicleSourceType::PerformanceProfile;
		case ECFVehicleProfileDomain::DriveState:
			return ECFVehicleSourceType::DriveStateProfile;
		default:
			return ECFVehicleSourceType::ProjectCompatibilityDefault;
		}
	}

	// Profile Domain별 source metadata를 반환합니다.
	const FCFVehicleProfileSource* GetProfileSource(const FCFVehicleProfileSnapshotSet& Profiles, const ECFVehicleProfileDomain Domain)
	{
		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			return &Profiles.BaseSource;
		case ECFVehicleProfileDomain::Drivetrain:
			return &Profiles.DrivetrainSource;
		case ECFVehicleProfileDomain::Handling:
			return &Profiles.HandlingSource;
		case ECFVehicleProfileDomain::Performance:
			return &Profiles.PerformanceSource;
		case ECFVehicleProfileDomain::DriveState:
			return &Profiles.DriveStateSource;
		default:
			return nullptr;
		}
	}

	// Profile Domain별 typed payload struct/address를 반환합니다.
	bool GetProfilePayload(
		const FCFVehicleProfileSnapshotSet& Profiles,
		const ECFVehicleProfileDomain Domain,
		const UScriptStruct*& OutStruct,
		const void*& OutAddress)
	{
		OutStruct = nullptr;
		OutAddress = nullptr;
		switch (Domain)
		{
		case ECFVehicleProfileDomain::VehicleBase:
			OutStruct = FCFVehicleBaseProfileData::StaticStruct();
			OutAddress = &Profiles.BaseData;
			return true;
		case ECFVehicleProfileDomain::Drivetrain:
			OutStruct = FCFDrivetrainProfileData::StaticStruct();
			OutAddress = &Profiles.DrivetrainData;
			return true;
		case ECFVehicleProfileDomain::Handling:
			OutStruct = FCFHandlingProfileData::StaticStruct();
			OutAddress = &Profiles.HandlingData;
			return true;
		case ECFVehicleProfileDomain::Performance:
			OutStruct = FCFPerformanceProfileData::StaticStruct();
			OutAddress = &Profiles.PerformanceData;
			return true;
		case ECFVehicleProfileDomain::DriveState:
			OutStruct = FCFDriveStateProfileData::StaticStruct();
			OutAddress = &Profiles.DriveStateData;
			return true;
		default:
			return false;
		}
	}

	// Feel-derived target leaf에 대응하는 FCFFeelResponse property name을 반환합니다.
	FName GetFeelResponsePropertyName(const FString& CanonicalPattern)
	{
		if (CanonicalPattern == TEXT("VehicleMovementConfig.EngineMaxTorque")) return TEXT("EngineMaxTorqueByFeel");
		if (CanonicalPattern == TEXT("VehicleMovementConfig.EngineMaxRPM")) return TEXT("EngineMaxRPMByFeel");
		if (CanonicalPattern == TEXT("VehicleMovementConfig.ThrottleInputScale")) return TEXT("ThrottleInputScaleByFeel");
		if (CanonicalPattern == TEXT("VehicleMovementConfig.FrontWheelMaxSteerAngle")) return TEXT("FrontWheelMaxSteerAngleByFeel");
		if (CanonicalPattern == TEXT("VehicleMovementConfig.SteeringAngleRatio")) return TEXT("SteeringAngleRatioByFeel");
		if (CanonicalPattern == TEXT("VehicleMovementConfig.FrontWheelFrictionForceMultiplier")) return TEXT("FrontWheelFrictionByFeel");
		if (CanonicalPattern == TEXT("VehicleMovementConfig.RearWheelFrictionForceMultiplier")) return TEXT("RearWheelFrictionByFeel");
		if (CanonicalPattern == TEXT("VehicleMovementConfig.FrontWheelCorneringStiffness")) return TEXT("FrontCorneringByFeel");
		if (CanonicalPattern == TEXT("VehicleMovementConfig.RearWheelCorneringStiffness")) return TEXT("RearCorneringByFeel");
		if (CanonicalPattern == TEXT("VehicleMovementConfig.FrontWheelSpringRate")) return TEXT("FrontSpringRateByFeel");
		if (CanonicalPattern == TEXT("VehicleMovementConfig.RearWheelSpringRate")) return TEXT("RearSpringRateByFeel");
		if (CanonicalPattern == TEXT("VehicleMovementConfig.FrontWheelSpringPreload")) return TEXT("FrontSpringPreloadByFeel");
		if (CanonicalPattern == TEXT("VehicleMovementConfig.RearWheelSpringPreload")) return TEXT("RearSpringPreloadByFeel");
		return NAME_None;
	}

	// Profile payload에서 FCFFeelResponse를 찾아 neutral baseline candidate를 반환합니다.
	const FCFFeelResponse* FindFeelResponse(
		const UScriptStruct& PayloadStruct,
		const void* PayloadAddress,
		const FName ResponsePropertyName)
	{
		// Feel response struct property입니다.
		const FStructProperty* ResponseProperty = FindFProperty<FStructProperty>(&PayloadStruct, ResponsePropertyName);
		if (!ResponseProperty || ResponseProperty->Struct != FCFFeelResponse::StaticStruct())
		{
			return nullptr;
		}
		return ResponseProperty->ContainerPtrToValuePtr<FCFFeelResponse>(PayloadAddress);
	}

	// Transmission ratio 배열 하나가 UE 5.8 positive-magnitude 저장 계약을 만족하는지 검사합니다.
	bool ArePositiveFiniteRatios(const TArray<float>& Ratios)
	{
		for (const float Ratio : Ratios)
		{
			if (!FMath::IsFinite(Ratio) || Ratio <= 0.0f)
			{
				return false;
			}
		}
		return true;
	}

	// Shift RPM 값이 Chaos 내부 uint32 의미와 동일한 비음수 정수값인지 반환합니다.
	bool IsNonNegativeIntegerRpm(const float RpmValue)
	{
		return FMath::IsFinite(RpmValue)
			&& RpmValue >= 0.0f
			&& FMath::IsNearlyEqual(RpmValue, FMath::RoundToFloat(RpmValue));
	}

	// Drivetrain complete Transmission payload를 fail-closed 검증합니다.
	bool ValidateTransmissionProfile(FResolverContext& Context)
	{
		// 검증할 Drivetrain Profile payload입니다.
		const FCFDrivetrainProfileData& Data = Context.Request.Profiles.DrivetrainData;
		if (!Data.bUseTransmissionConfig)
		{
			return true;
		}

		// atomic Transmission ratio-set을 validation issue의 대표 위치로 사용합니다.
		const FCFVehicleFieldPath ValidationPath = FindScalarPath(TEXT("VehicleMovementConfig.TransmissionRatios"));
		if (Data.TransmissionRatios.ForwardGearRatios.IsEmpty()
			|| Data.TransmissionRatios.ReverseGearRatios.IsEmpty()
			|| !ArePositiveFiniteRatios(Data.TransmissionRatios.ForwardGearRatios)
			|| !ArePositiveFiniteRatios(Data.TransmissionRatios.ReverseGearRatios)
			|| !FMath::IsFinite(Data.FinalRatio) || Data.FinalRatio <= 0.0f
			|| !IsNonNegativeIntegerRpm(Data.ChangeUpRPM)
			|| !IsNonNegativeIntegerRpm(Data.ChangeDownRPM)
			|| !FMath::IsFinite(Data.GearChangeTime) || Data.GearChangeTime < 0.0f
			|| !FMath::IsFinite(Data.TransmissionEfficiency) || Data.TransmissionEfficiency < 0.0f || Data.TransmissionEfficiency > 1.0f
			|| (Data.bUseAutomaticGears && Data.ChangeDownRPM > Data.ChangeUpRPM))
		{
			AddIssue(
				Context,
				Context.Result.ResolverValidation,
				ECFVehicleValidationSeverity::Blocked,
				TEXT("DrivetrainTransmissionInvalid"),
				TEXT("차량별 Transmission payload가 유효하지 않습니다. 기어비 배열은 비어 있지 않은 양수 finite magnitude만 허용하며, FinalRatio>0, Shift RPM은 비음수 정수, 변속 시간>=0, Efficiency=0..1이어야 합니다. 자동 변속에서는 ChangeDownRPM<=ChangeUpRPM이어야 합니다."),
				&ValidationPath);
			return false;
		}
		return true;
	}

	// Performance Profile의 vehicle-specific Engine Torque Curve payload를 fail-closed 검증합니다.
	bool ValidateEngineTorqueCurveProfile(FResolverContext& Context)
	{
		// 검증할 Performance Profile payload입니다.
		const FCFPerformanceProfileData& Data = Context.Request.Profiles.PerformanceData;
		if (!Data.bUseEngineTorqueCurve)
		{
			return true;
		}

		// EngineTorqueCurve atomic field를 validation issue의 대표 위치로 사용합니다.
		const FCFVehicleFieldPath ValidationPath = FindScalarPath(TEXT("VehicleMovementConfig.EngineTorqueCurve"));
		FString CurveError;
		if (!FCFVehicleEngineCurveUtils::ValidateCurve(Data.EngineTorqueCurve, CurveError))
		{
			AddIssue(
				Context,
				Context.Result.ResolverValidation,
				ECFVehicleValidationSeverity::Blocked,
				TEXT("PerformanceEngineTorqueCurveInvalid"),
				CurveError,
				&ValidationPath);
			return false;
		}
		return true;
	}

	// Target field가 vehicle-specific Engine Torque Curve atomic payload인지 반환합니다.
	bool IsEngineTorqueCurvePayloadField(const FString& CanonicalPattern)
	{
		return CanonicalPattern == TEXT("VehicleMovementConfig.EngineTorqueCurve");
	}

	// Target field가 VehicleBase Reference wheel geometry opt-in 대상인지 반환합니다.
	bool IsReferenceWheelGeometryField(const FString& CanonicalPattern)
	{
		return CanonicalPattern == TEXT("VehicleMovementConfig.FrontWheelRadius")
			|| CanonicalPattern == TEXT("VehicleMovementConfig.RearWheelRadius")
			|| CanonicalPattern == TEXT("VehicleMovementConfig.FrontWheelWidth")
			|| CanonicalPattern == TEXT("VehicleMovementConfig.RearWheelWidth");
	}

	// Target field가 Drivetrain complete Transmission opt-in 대상인지 반환합니다.
	bool IsTransmissionField(const FString& CanonicalPattern)
	{
		return CanonicalPattern == TEXT("VehicleMovementConfig.TransmissionRatios")
			|| CanonicalPattern == TEXT("VehicleMovementConfig.bUseAutomaticGears")
			|| CanonicalPattern == TEXT("VehicleMovementConfig.bUseAutoReverse")
			|| CanonicalPattern == TEXT("VehicleMovementConfig.FinalRatio")
			|| CanonicalPattern == TEXT("VehicleMovementConfig.ChangeUpRPM")
			|| CanonicalPattern == TEXT("VehicleMovementConfig.ChangeDownRPM")
			|| CanonicalPattern == TEXT("VehicleMovementConfig.GearChangeTime")
			|| CanonicalPattern == TEXT("VehicleMovementConfig.TransmissionEfficiency");
	}

	// R2 Frozen 5 Profile Snapshot을 primary owner field에 적용합니다.
	void RunR2Profiles(FResolverContext& Context)
	{
		// Enabled complete Transmission payload가 잘못되면 Profile stage 자체를 fail-closed로 유지합니다.
		const bool bTransmissionPayloadValid = ValidateTransmissionProfile(Context);
		// Enabled vehicle-specific Engine Torque Curve가 잘못되면 Profile stage 자체를 fail-closed로 유지합니다.
		const bool bEngineTorqueCurveValid = ValidateEngineTorqueCurveProfile(Context);
		for (const FCFVehicleFieldDescriptor& Descriptor : FCFVehicleFieldRegistry::GetDescriptors())
		{
			if (Descriptor.PrimaryProfileDomain == ECFVehicleProfileDomain::None || !Descriptor.StablePathPattern.CollectionPropertyName.IsNone())
			{
				continue;
			}
			if (Descriptor.PrimaryProfileDomain == ECFVehicleProfileDomain::DriveState
				&& Context.Request.Recipe.DriveStateMode != ECFVehicleDriveStateMode::VehicleSpecific)
			{
				continue;
			}

			// Domain source identity/revision/fingerprint metadata입니다.
			const FCFVehicleProfileSource* ProfileSource = GetProfileSource(Context.Request.Profiles, Descriptor.PrimaryProfileDomain);
			if (!ProfileSource || !ProfileSource->IsPresent())
			{
				continue;
			}

			// Domain typed payload reflection struct/address입니다.
			const UScriptStruct* PayloadStruct = nullptr;
			const void* PayloadAddress = nullptr;
			if (!GetProfilePayload(Context.Request.Profiles, Descriptor.PrimaryProfileDomain, PayloadStruct, PayloadAddress) || !PayloadStruct || !PayloadAddress)
			{
				continue;
			}

			// Current descriptor의 canonical Registry pattern입니다.
			const FString CanonicalPattern = Descriptor.GetCanonicalPattern();
			if (IsReferenceWheelGeometryField(CanonicalPattern)
				&& !Context.Request.Profiles.BaseData.bUseReferenceWheelGeometry)
			{
				continue;
			}
			if (IsTransmissionField(CanonicalPattern)
				&& (!Context.Request.Profiles.DrivetrainData.bUseTransmissionConfig || !bTransmissionPayloadValid))
			{
				continue;
			}
			// Opt-in이 꺼져 있으면 Curve payload 자체는 후보를 만들지 않습니다. bUseEngineTorqueCurve=false flag는 정상 candidate로 남겨 hot-apply 시 이전 vehicle-specific Curve를 해제할 수 있게 합니다.
			if (IsEngineTorqueCurvePayloadField(CanonicalPattern)
				&& (!Context.Request.Profiles.PerformanceData.bUseEngineTorqueCurve || !bEngineTorqueCurveValid))
			{
				continue;
			}

			// Profile layer가 제공할 exact scalar target path입니다.
			const FCFVehicleFieldPath& TargetPath = Descriptor.StablePathPattern;
			// Feel-derived field의 profile response property 이름입니다.
			const FName FeelResponseName = GetFeelResponsePropertyName(Descriptor.GetCanonicalPattern());
			if (!FeelResponseName.IsNone())
			{
				// Profile response의 neutral baseline입니다.
				const FCFFeelResponse* Response = FindFeelResponse(*PayloadStruct, PayloadAddress, FeelResponseName);
				if (Response)
				{
					AddFloatCandidate(
						Context,
						TargetPath,
						Response->NeutralValue,
						GetProfileSourceType(Descriptor.PrimaryProfileDomain),
						ProfileSource->SourceObjectPath.ToString(),
						ProfileSource->AuthoringRevision,
						ProfileSource->ProfileFingerprint);
				}
				continue;
			}

			// Direct Profile field는 target leaf name과 typed payload property name이 동일합니다. Atomic TransmissionRatios/EngineTorqueCurve도 payload의 동일 이름 FStructProperty로 통째로 codec 처리합니다.
			const FName TargetLeafName = TargetPath.PropertyChain.IsEmpty() ? NAME_None : TargetPath.PropertyChain.Last();
			if (TargetLeafName.IsNone() || !FindFProperty<FProperty>(PayloadStruct, TargetLeafName))
			{
				continue;
			}

			AddStructPropertyCandidate(
				Context,
				TargetPath,
				*PayloadStruct,
				PayloadAddress,
				TargetLeafName,
				GetProfileSourceType(Descriptor.PrimaryProfileDomain),
				ProfileSource->SourceObjectPath.ToString(),
				ProfileSource->AuthoringRevision,
				ProfileSource->ProfileFingerprint);
		}
	}

	// Recipe semantic source identity 문자열을 반환합니다.
	FString GetRecipeSourceId(const FCFVehicleRecipeSnapshot& Recipe)
	{
		return Recipe.RecipeId.IsValid() ? Recipe.RecipeId.ToString(EGuidFormats::DigitsWithHyphensLower) : TEXT("Recipe.Transient");
	}

	// R3 Recipe asset refs/bindings/semantic collections/explicit values를 candidate stack에 추가합니다.
	void RunR3RecipeSemantic(FResolverContext& Context)
	{
		// Recipe source identity입니다.
		const FString RecipeSourceId = GetRecipeSourceId(Context.Request.Recipe);
		// Recipe source fingerprint입니다.
		const FString& RecipeFingerprint = Context.Request.Recipe.RecipeFingerprint;
		// Recipe diagnostic revision입니다.
		const int32 RecipeRevision = Context.Request.Recipe.AuthoringRevision;

		static const FName VisualAssetFields[] = {TEXT("ChassisMesh"), TEXT("WheelMeshFL"), TEXT("WheelMeshFR"), TEXT("WheelMeshRL"), TEXT("WheelMeshRR")};
		for (const FName FieldName : VisualAssetFields)
		{
			// VehicleVisualConfig.<FieldName> target path입니다.
			const FCFVehicleFieldPath TargetPath = FindScalarPath(*FString::Printf(TEXT("VehicleVisualConfig.%s"), *FieldName.ToString()));
			AddStructPropertyCandidate(Context, TargetPath, *FCFVehicleAssetIntent::StaticStruct(), &Context.Request.Recipe.AssetIntent, FieldName, ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
		}

		static const FName WheelSocketFields[] = {TEXT("BodyWheelSocketFL"), TEXT("BodyWheelSocketFR"), TEXT("BodyWheelSocketRL"), TEXT("BodyWheelSocketRR")};
		for (const FName FieldName : WheelSocketFields)
		{
			// Recipe binding의 현재 FName 값 property입니다.
			const FProperty* SourceProperty = FindFProperty<FProperty>(FCFVehicleAssetIntent::StaticStruct(), FieldName);
			if (!SourceProperty)
			{
				continue;
			}
			// Recipe binding source value address입니다.
			const void* SourceValueAddress = SourceProperty->ContainerPtrToValuePtr<void>(&Context.Request.Recipe.AssetIntent);
			// None binding은 Project Compatibility Default fallback을 그대로 effective로 둡니다.
			const FName* SocketName = static_cast<const FName*>(SourceValueAddress);
			if (SocketName && SocketName->IsNone())
			{
				continue;
			}

			// VehicleLayoutConfig.<FieldName> target path입니다.
			const FCFVehicleFieldPath TargetPath = FindScalarPath(*FString::Printf(TEXT("VehicleLayoutConfig.%s"), *FieldName.ToString()));
			AddStructPropertyCandidate(Context, TargetPath, *FCFVehicleAssetIntent::StaticStruct(), &Context.Request.Recipe.AssetIntent, FieldName, ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
		}

		for (const FCFHardpointIntent& HardpointIntent : Context.Request.Recipe.HardpointIntents)
		{
			if (HardpointIntent.LocationSlotId.IsNone())
			{
				continue;
			}

			static const FName HardpointSemanticFields[] = {TEXT("LocationSlotId"), TEXT("LocationCategory"), TEXT("SocketName")};
			for (const FName FieldName : HardpointSemanticFields)
			{
				// Hardpoint stable selector까지 확정된 exact target path입니다.
				const FString Pattern = FString::Printf(TEXT("HardpointSlots[LocationSlotId=*].%s"), *FieldName.ToString());
				const FCFVehicleFieldPath TargetPath = MakeExactCollectionPath(*Pattern, HardpointIntent.LocationSlotId);
				AddStructPropertyCandidate(Context, TargetPath, *FCFHardpointIntent::StaticStruct(), &HardpointIntent, FieldName, ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
			}
		}

		for (const FCFMountIntent& MountIntent : Context.Request.Recipe.MountIntents)
		{
			if (MountIntent.MountProfileId.IsNone())
			{
				continue;
			}

			static const FName MountSemanticFields[] = {TEXT("MountProfileId"), TEXT("LocationSlotRef"), TEXT("MountType"), TEXT("SizeLimit"), TEXT("DefaultEquipmentPresetData"), TEXT("bExposedModule")};
			for (const FName FieldName : MountSemanticFields)
			{
				// Mount stable selector까지 확정된 exact target path입니다.
				const FString Pattern = FString::Printf(TEXT("MountProfiles[MountProfileId=*].%s"), *FieldName.ToString());
				const FCFVehicleFieldPath TargetPath = MakeExactCollectionPath(*Pattern, MountIntent.MountProfileId);
				AddStructPropertyCandidate(Context, TargetPath, *FCFMountIntent::StaticStruct(), &MountIntent, FieldName, ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
			}
		}

		if (Context.Request.Recipe.MassIntent.BaseMassMode == ECFAuthoringInputMode::ExplicitValue)
		{
			AddFloatCandidate(Context, FindScalarPath(TEXT("BaseVehicleMassKg")), Context.Request.Recipe.MassIntent.ExplicitBaseMassKg, ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
		}
		if (Context.Request.Recipe.MassIntent.GrossMassMode == ECFAuthoringInputMode::ExplicitValue)
		{
			AddFloatCandidate(Context, FindScalarPath(TEXT("MaximumGrossMassKg")), Context.Request.Recipe.MassIntent.ExplicitGrossMassKg, ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
		}
		if (Context.Request.Recipe.DurabilityIntent.MaxHealthMode == ECFAuthoringInputMode::ExplicitValue)
		{
			AddFloatCandidate(Context, FindScalarPath(TEXT("VehicleDurabilityConfig.MaxHealth")), Context.Request.Recipe.DurabilityIntent.ExplicitMaxHealth, ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
		}

		// Default Defense semantic intent 처리 target path입니다.
		const FCFVehicleFieldPath DefensePath = FindScalarPath(TEXT("DefaultDefenseData"));
		if (Context.Request.Recipe.DefaultDataIntent.DefenseMode == ECFAssetIntentMode::ExplicitAsset)
		{
			AddStructPropertyCandidate(Context, DefensePath, *FCFVehicleDefaultIntent::StaticStruct(), &Context.Request.Recipe.DefaultDataIntent, TEXT("DefaultDefenseData"), ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
		}
		else if (Context.Request.Recipe.DefaultDataIntent.DefenseMode == ECFAssetIntentMode::ExplicitNone)
		{
			// ExplicitNone canonical target value입니다.
			FCFVehicleFieldValue NoneValue;
			// None encode 실패 이유입니다.
			FString EncodeError;
			if (EncodeTargetNone(DefensePath, NoneValue, EncodeError))
			{
				AddCandidate(Context, DefensePath, NoneValue, ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
			}
			else
			{
				AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Error, TEXT("DefenseNoneEncodeFailed"), EncodeError, &DefensePath);
			}
		}

		// Default Destroyed FX semantic intent 처리 target path입니다.
		const FCFVehicleFieldPath DestroyedFxPath = FindScalarPath(TEXT("DefaultDestroyedFxData"));
		if (Context.Request.Recipe.DefaultDataIntent.DestroyedFxMode == ECFAssetIntentMode::ExplicitAsset)
		{
			AddStructPropertyCandidate(Context, DestroyedFxPath, *FCFVehicleDefaultIntent::StaticStruct(), &Context.Request.Recipe.DefaultDataIntent, TEXT("DefaultDestroyedFxData"), ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
		}
		else if (Context.Request.Recipe.DefaultDataIntent.DestroyedFxMode == ECFAssetIntentMode::ExplicitNone)
		{
			// ExplicitNone canonical target value입니다.
			FCFVehicleFieldValue NoneValue;
			// None encode 실패 이유입니다.
			FString EncodeError;
			if (EncodeTargetNone(DestroyedFxPath, NoneValue, EncodeError))
			{
				AddCandidate(Context, DestroyedFxPath, NoneValue, ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
			}
			else
			{
				AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Error, TEXT("DestroyedFxNoneEncodeFailed"), EncodeError, &DestroyedFxPath);
			}
		}

		AddStructPropertyCandidate(Context, FindScalarPath(TEXT("DestroyedFxSocketName")), *FCFVehicleDefaultIntent::StaticStruct(), &Context.Request.Recipe.DefaultDataIntent, TEXT("DestroyedFxSocketName"), ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);

		if (Context.Request.Recipe.WheelVisualIntent.Mode != ECFWheelVisualIntentMode::UseProfilePolicy)
		{
			// Recipe WheelVisual intent가 요구하는 legacy auto-scale bool입니다.
			const bool bAutoScale = Context.Request.Recipe.WheelVisualIntent.Mode == ECFWheelVisualIntentMode::AutoScaleToPhysicsRadius;
			// USER Chassis Socket Scale을 Wheel Size Authority로 사용하는 신규 mode 여부입니다.
			const bool bUseWheelSocketScale = Context.Request.Recipe.WheelVisualIntent.Mode == ECFWheelVisualIntentMode::SocketScaleFromChassis;
			AddBoolCandidate(Context, FindScalarPath(TEXT("WheelVisualConfig.bAutoScaleWheelMeshToRadius")), bAutoScale, ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
			AddBoolCandidate(Context, FindScalarPath(TEXT("WheelVisualConfig.bUseWheelSocketScale")), bUseWheelSocketScale, ECFVehicleSourceType::RecipeExplicitSemanticInput, RecipeSourceId, RecipeRevision, RecipeFingerprint);
		}
	}

	// Frozen piecewise-linear Feel Response를 계산합니다.
	float ResolveFeelResponse(const FCFFeelResponse& Response, const float Intent)
	{
		// Frozen 0..1 범위로 제한한 semantic feel입니다.
		const float ClampedFeel = FMath::Clamp(Intent, 0.0f, 1.0f);
		if (ClampedFeel <= 0.5f)
		{
			return FMath::Lerp(Response.LowValue, Response.NeutralValue, ClampedFeel * 2.0f);
		}
		return FMath::Lerp(Response.NeutralValue, Response.HighValue, (ClampedFeel - 0.5f) * 2.0f);
	}

	// Current candidate stack에서 effective BaseVehicleMassKg float context를 읽습니다.
	bool ReadEffectiveBaseMass(const FResolverContext& Context, float& OutBaseMassKg)
	{
		// BaseVehicleMassKg current winner입니다.
		const FVehicleFieldCandidate* Winner = FindCurrentWinner(Context, TEXT("BaseVehicleMassKg"));
		if (!Winner)
		{
			return false;
		}
		OutBaseMassKg = FCString::Atof(*Winner->Value.CanonicalValueText);
		return true;
	}

	// Frozen opt-in MassScale rule을 계산하고 invalid context를 Block합니다.
	bool ResolveMassScale(
		FResolverContext& Context,
		const FCFMassScaleRule& Rule,
		const FCFVehicleFieldPath& TargetPath,
		float& OutScale)
	{
		OutScale = 1.0f;
		if (!Rule.bEnabled)
		{
			return true;
		}

		// Current effective BaseVehicleMassKg입니다.
		float BaseMassKg = 0.0f;
		if (!ReadEffectiveBaseMass(Context, BaseMassKg) || BaseMassKg <= 0.0f || Rule.ReferenceMassKg <= 0.0f)
		{
			AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Blocked, TEXT("MassScaleContextMissing"), TEXT("MassScale이 활성화됐지만 유효한 BaseVehicleMassKg 또는 ReferenceMassKg가 없습니다."), &TargetPath);
			return false;
		}

		OutScale = FMath::Pow(BaseMassKg / Rule.ReferenceMassKg, Rule.MassExponent);
		return true;
	}

	// R4 Driving Feel Rule Derived candidate를 생성합니다.
	void RunR4DrivingFeel(FResolverContext& Context)
	{
		// Performance-derived target/response/intent mapping입니다.
		struct FFeelRuleEntry
		{
			// Target canonical path입니다.
			const TCHAR* TargetPath;
			// Profile response property name입니다.
			const TCHAR* ResponseProperty;
			// Recipe feel intent입니다.
			float Intent;
			// Engine torque mass scale을 적용할지 여부입니다.
			bool bTorqueMassScale;
			// Suspension mass scale을 적용할지 여부입니다.
			bool bSuspensionMassScale;
		};

		// Frozen 13개 Feel-derived field rule 목록입니다.
		const FFeelRuleEntry FeelRules[] =
		{
			{TEXT("VehicleMovementConfig.EngineMaxTorque"), TEXT("EngineMaxTorqueByFeel"), Context.Request.Recipe.DrivingFeelIntent.AccelerationFeel, true, false},
			{TEXT("VehicleMovementConfig.EngineMaxRPM"), TEXT("EngineMaxRPMByFeel"), Context.Request.Recipe.DrivingFeelIntent.AccelerationFeel, false, false},
			{TEXT("VehicleMovementConfig.ThrottleInputScale"), TEXT("ThrottleInputScaleByFeel"), Context.Request.Recipe.DrivingFeelIntent.AccelerationFeel, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelMaxSteerAngle"), TEXT("FrontWheelMaxSteerAngleByFeel"), Context.Request.Recipe.DrivingFeelIntent.SteeringAgility, false, false},
			{TEXT("VehicleMovementConfig.SteeringAngleRatio"), TEXT("SteeringAngleRatioByFeel"), Context.Request.Recipe.DrivingFeelIntent.SteeringAgility, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelFrictionForceMultiplier"), TEXT("FrontWheelFrictionByFeel"), Context.Request.Recipe.DrivingFeelIntent.GripFeel, false, false},
			{TEXT("VehicleMovementConfig.RearWheelFrictionForceMultiplier"), TEXT("RearWheelFrictionByFeel"), Context.Request.Recipe.DrivingFeelIntent.GripFeel, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelCorneringStiffness"), TEXT("FrontCorneringByFeel"), Context.Request.Recipe.DrivingFeelIntent.GripFeel, false, false},
			{TEXT("VehicleMovementConfig.RearWheelCorneringStiffness"), TEXT("RearCorneringByFeel"), Context.Request.Recipe.DrivingFeelIntent.GripFeel, false, false},
			{TEXT("VehicleMovementConfig.FrontWheelSpringRate"), TEXT("FrontSpringRateByFeel"), Context.Request.Recipe.DrivingFeelIntent.SuspensionFirmness, false, true},
			{TEXT("VehicleMovementConfig.RearWheelSpringRate"), TEXT("RearSpringRateByFeel"), Context.Request.Recipe.DrivingFeelIntent.SuspensionFirmness, false, true},
			{TEXT("VehicleMovementConfig.FrontWheelSpringPreload"), TEXT("FrontSpringPreloadByFeel"), Context.Request.Recipe.DrivingFeelIntent.SuspensionFirmness, false, true},
			{TEXT("VehicleMovementConfig.RearWheelSpringPreload"), TEXT("RearSpringPreloadByFeel"), Context.Request.Recipe.DrivingFeelIntent.SuspensionFirmness, false, true}
		};

		for (const FFeelRuleEntry& RuleEntry : FeelRules)
		{
			// Target field path입니다.
			const FCFVehicleFieldPath TargetPath = FindScalarPath(RuleEntry.TargetPath);
			// Target descriptor로부터 primary Profile Domain을 확인합니다.
			const FCFVehicleFieldDescriptor* Descriptor = FindDescriptor(TargetPath);
			if (!Descriptor)
			{
				continue;
			}

			// 해당 domain profile source입니다.
			const FCFVehicleProfileSource* ProfileSource = GetProfileSource(Context.Request.Profiles, Descriptor->PrimaryProfileDomain);
			if (!ProfileSource || !ProfileSource->IsPresent())
			{
				continue;
			}

			// 해당 domain typed payload입니다.
			const UScriptStruct* PayloadStruct = nullptr;
			const void* PayloadAddress = nullptr;
			if (!GetProfilePayload(Context.Request.Profiles, Descriptor->PrimaryProfileDomain, PayloadStruct, PayloadAddress) || !PayloadStruct || !PayloadAddress)
			{
				continue;
			}

			// Profile-authored Feel response입니다.
			const FCFFeelResponse* Response = FindFeelResponse(*PayloadStruct, PayloadAddress, FName(RuleEntry.ResponseProperty));
			if (!Response)
			{
				AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Error, TEXT("FeelResponseMissing"), FString::Printf(TEXT("Feel response가 없습니다: %s"), RuleEntry.ResponseProperty), &TargetPath);
				continue;
			}

			// Frozen piecewise-linear response raw value입니다.
			float ResolvedValue = ResolveFeelResponse(*Response, RuleEntry.Intent);
			// Optional mass context scale입니다.
			float MassScale = 1.0f;
			if (RuleEntry.bTorqueMassScale)
			{
				if (!ResolveMassScale(Context, Context.Request.Profiles.PerformanceData.TorqueMassScale, TargetPath, MassScale))
				{
					continue;
				}
			}
			else if (RuleEntry.bSuspensionMassScale)
			{
				if (!ResolveMassScale(Context, Context.Request.Profiles.HandlingData.SuspensionMassScale, TargetPath, MassScale))
				{
					continue;
				}
			}
			ResolvedValue *= MassScale;

			// Rule source fingerprint는 Profile payload + Recipe intent + effective mass context를 결합합니다.
			FString RulePayload;
			AppendToken(RulePayload, TEXT("Profile"), ProfileSource->ProfileFingerprint);
			AppendToken(RulePayload, TEXT("Recipe"), Context.Request.Recipe.RecipeFingerprint);
			AppendToken(RulePayload, TEXT("MassScale"), FString::SanitizeFloat(MassScale));
			AddFloatCandidate(Context, TargetPath, ResolvedValue, ECFVehicleSourceType::RuleDerived, FString::Printf(TEXT("Rule.%s"), RuleEntry.TargetPath), 0, HashUtf8Payload(RulePayload));
		}
	}

	// Recipe Wheel socket binding에 기존 capture fallback을 적용합니다.
	FName ResolveWheelSocketName(const FName ConfiguredSocketName, const TCHAR* DefaultSocketName)
	{
		return ConfiguredSocketName.IsNone() ? FName(DefaultSocketName) : ConfiguredSocketName;
	}

	// R5 Chassis socket facts를 authoritative Asset Derived pose candidate로 변환합니다.
	void RunR5AssetSockets(FResolverContext& Context)
	{
		// Wheel anchor target name/socket mapping입니다.
		struct FWheelSocketRule
		{
			// Wheel anchor target prefix입니다.
			const TCHAR* AnchorName;
			// Effective socket name입니다.
			FName SocketName;
		};

		// Frozen 4-wheel anchor mapping입니다.
		const FWheelSocketRule WheelRules[] =
		{
			{TEXT("WheelAnchorFL"), ResolveWheelSocketName(Context.Request.Recipe.AssetIntent.BodyWheelSocketFL, TEXT("Wheel_Anchor_FL"))},
			{TEXT("WheelAnchorFR"), ResolveWheelSocketName(Context.Request.Recipe.AssetIntent.BodyWheelSocketFR, TEXT("Wheel_Anchor_FR"))},
			{TEXT("WheelAnchorRL"), ResolveWheelSocketName(Context.Request.Recipe.AssetIntent.BodyWheelSocketRL, TEXT("Wheel_Anchor_RL"))},
			{TEXT("WheelAnchorRR"), ResolveWheelSocketName(Context.Request.Recipe.AssetIntent.BodyWheelSocketRR, TEXT("Wheel_Anchor_RR"))}
		};

		for (const FWheelSocketRule& Rule : WheelRules)
		{
			// Asset Snapshot에서 찾은 requested socket fact입니다.
			const FCFVehicleSocketSnapshot* SocketSnapshot = Context.Request.Assets.FindChassisSocket(Rule.SocketName);
			if (!SocketSnapshot || !SocketSnapshot->bFound)
			{
				AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Blocked, TEXT("RequiredWheelSocketMissing"), FString::Printf(TEXT("필수 Wheel socket fact가 없습니다: %s"), *Rule.SocketName.ToString()));
				continue;
			}

			// Wheel anchor location target path입니다.
			const FCFVehicleFieldPath LocationPath = FindScalarPath(*FString::Printf(TEXT("VehicleLayoutConfig.%s.RelativeLocation"), Rule.AnchorName));
			// Wheel anchor rotation target path입니다.
			const FCFVehicleFieldPath RotationPath = FindScalarPath(*FString::Printf(TEXT("VehicleLayoutConfig.%s.RelativeRotation"), Rule.AnchorName));
			// Wheel anchor authored size scale target path입니다.
			const FCFVehicleFieldPath ScalePath = FindScalarPath(*FString::Printf(TEXT("VehicleLayoutConfig.%s.RelativeScale"), Rule.AnchorName));
			AddVectorCandidate(Context, LocationPath, SocketSnapshot->RelativeLocation, ECFVehicleSourceType::AssetDerived, Context.Request.Assets.ChassisObjectPath.ToString(), 0, Context.Request.Assets.ChassisLayoutFingerprint);
			AddRotatorCandidate(Context, RotationPath, SocketSnapshot->RelativeRotation, ECFVehicleSourceType::AssetDerived, Context.Request.Assets.ChassisObjectPath.ToString(), 0, Context.Request.Assets.ChassisLayoutFingerprint);
			AddVectorCandidate(Context, ScalePath, SocketSnapshot->RelativeScale, ECFVehicleSourceType::AssetDerived, Context.Request.Assets.ChassisObjectPath.ToString(), 0, Context.Request.Assets.ChassisLayoutFingerprint);
		}

		for (const FCFHardpointIntent& HardpointIntent : Context.Request.Recipe.HardpointIntents)
		{
			if (HardpointIntent.LocationSlotId.IsNone() || HardpointIntent.SocketName.IsNone())
			{
				continue;
			}

			// Hardpoint binding socket fact입니다.
			const FCFVehicleSocketSnapshot* SocketSnapshot = Context.Request.Assets.FindChassisSocket(HardpointIntent.SocketName);
			if (!SocketSnapshot || !SocketSnapshot->bFound)
			{
				AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Blocked, TEXT("HardpointSocketMissing"), FString::Printf(TEXT("Hardpoint가 요청한 Chassis socket fact가 없습니다: %s / %s"), *HardpointIntent.LocationSlotId.ToString(), *HardpointIntent.SocketName.ToString()));
				continue;
			}

			// Hardpoint exact local location path입니다.
			const FCFVehicleFieldPath LocationPath = MakeExactCollectionPath(TEXT("HardpointSlots[LocationSlotId=*].LocalLocation"), HardpointIntent.LocationSlotId);
			// Hardpoint exact local rotation path입니다.
			const FCFVehicleFieldPath RotationPath = MakeExactCollectionPath(TEXT("HardpointSlots[LocationSlotId=*].LocalRotation"), HardpointIntent.LocationSlotId);
			AddVectorCandidate(Context, LocationPath, SocketSnapshot->RelativeLocation, ECFVehicleSourceType::AssetDerived, Context.Request.Assets.ChassisObjectPath.ToString(), 0, Context.Request.Assets.ChassisLayoutFingerprint);
			AddRotatorCandidate(Context, RotationPath, SocketSnapshot->RelativeRotation, ECFVehicleSourceType::AssetDerived, Context.Request.Assets.ChassisObjectPath.ToString(), 0, Context.Request.Assets.ChassisLayoutFingerprint);
		}
	}

	// 현재 effective Wheel radius measure mode를 Project/Base layer에서 읽습니다.
	ECFWheelMeshRadiusMeasureMode ReadCurrentMeasureMode(const FResolverContext& Context)
	{
		// Measure mode field current winner입니다.
		const FVehicleFieldCandidate* Winner = FindCurrentWinner(Context, TEXT("WheelVisualConfig.WheelMeshRadiusMeasureMode"));
		if (!Winner)
		{
			return ECFWheelMeshRadiusMeasureMode::AutoMaxXZ;
		}

		// Enum canonical text에서 known enumerator name을 비교합니다.
		const FString& Text = Winner->Value.CanonicalValueText;
		if (Text.Contains(TEXT("AxisX"))) return ECFWheelMeshRadiusMeasureMode::AxisX;
		if (Text.Contains(TEXT("AxisY"))) return ECFWheelMeshRadiusMeasureMode::AxisY;
		if (Text.Contains(TEXT("AxisZ"))) return ECFWheelMeshRadiusMeasureMode::AxisZ;
		return ECFWheelMeshRadiusMeasureMode::AutoMaxXZ;
	}

	// Wheel bounds extent에서 Frozen/current measure mode 반지름을 계산합니다.
	float MeasureWheelRadius(const FVector& BoundsExtent, const ECFWheelMeshRadiusMeasureMode MeasureMode)
	{
		switch (MeasureMode)
		{
		case ECFWheelMeshRadiusMeasureMode::AxisX:
			return FMath::Abs(BoundsExtent.X);
		case ECFWheelMeshRadiusMeasureMode::AxisY:
			return FMath::Abs(BoundsExtent.Y);
		case ECFWheelMeshRadiusMeasureMode::AxisZ:
			return FMath::Abs(BoundsExtent.Z);
		case ECFWheelMeshRadiusMeasureMode::AutoMaxXZ:
		default:
			return FMath::Max(FMath::Abs(BoundsExtent.X), FMath::Abs(BoundsExtent.Z));
		}
	}

	// 같은 axle에서 left 우선/right fallback으로 측정 source Wheel Snapshot을 선택합니다.
	const FCFVehicleWheelAssetSnapshot* SelectWheelSnapshot(
		const FCFVehicleWheelAssetSnapshot& LeftWheel,
		const FCFVehicleWheelAssetSnapshot& RightWheel)
	{
		if (LeftWheel.bAssetLoaded)
		{
			return &LeftWheel;
		}
		if (RightWheel.bAssetLoaded)
		{
			return &RightWheel;
		}
		return nullptr;
	}

	// Axle의 양쪽 Wheel resolver-relevant fingerprint를 deterministic하게 결합합니다.
	FString BuildAxleFingerprint(const FCFVehicleWheelAssetSnapshot& LeftWheel, const FCFVehicleWheelAssetSnapshot& RightWheel)
	{
		// Left/Right identity를 보존한 axle fingerprint payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("Left"), LeftWheel.MeasureFingerprint);
		AppendToken(Payload, TEXT("Right"), RightWheel.MeasureFingerprint);
		return HashUtf8Payload(Payload);
	}

	// Wheel bounds에서 radius/width proposal 하나를 만듭니다.
	void AddMeasurementProposal(
		FResolverContext& Context,
		const FCFVehicleFieldPath& TargetPath,
		const float MeasuredValue,
		const FString& AssetFingerprint,
		const FName RuleId)
	{
		// Target typed candidate value입니다.
		FCFVehicleFieldValue EncodedValue;
		// Encoding 실패 이유입니다.
		FString EncodeError;
		if (!EncodeNativeTargetValue(TargetPath, &MeasuredValue, EncodedValue, EncodeError))
		{
			AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Error, TEXT("MeasurementEncodeFailed"), EncodeError, &TargetPath);
			return;
		}

		// Adoption 전 effective stack과 분리된 proposal입니다.
		FCFVehicleMeasurementProposal& Proposal = Context.Result.MeasurementProposals.AddDefaulted_GetRef();
		Proposal.FieldPath = TargetPath;
		Proposal.MeasuredCandidateValue = MoveTemp(EncodedValue);
		Proposal.AssetFingerprint = AssetFingerprint;
		Proposal.MeasurementRuleId = RuleId;
	}

	// Socket Scale derived size fingerprint에 scale 한 축을 deterministic canonical token으로 추가합니다.
	void AppendScaleFingerprintTokens(FString& Payload, const TCHAR* Prefix, const FVector& Scale)
	{
		AppendToken(Payload, *FString::Printf(TEXT("%sX"), Prefix), FString::SanitizeFloat(Scale.X));
		AppendToken(Payload, *FString::Printf(TEXT("%sY"), Prefix), FString::SanitizeFloat(Scale.Y));
		AppendToken(Payload, *FString::Printf(TEXT("%sZ"), Prefix), FString::SanitizeFloat(Scale.Z));
	}

	// Wheel Bounds와 두 authored Socket Scale만 포함하는 narrow axle-size fingerprint입니다.
	FString BuildSocketScaleAxleFingerprint(
		const FCFVehicleWheelAssetSnapshot& LeftWheelSource,
		const FCFVehicleWheelAssetSnapshot& RightWheelSource,
		const FCFVehicleSocketSnapshot& LeftSocket,
		const FCFVehicleSocketSnapshot& RightSocket)
	{
		FString Payload;
		AppendToken(Payload, TEXT("Kind"), TEXT("WheelSocketScaleAxleSize.v1"));
		AppendToken(Payload, TEXT("LeftWheel"), LeftWheelSource.MeasureFingerprint);
		AppendToken(Payload, TEXT("RightWheel"), RightWheelSource.MeasureFingerprint);
		AppendToken(Payload, TEXT("LeftSocket"), LeftSocket.SocketName.ToString());
		AppendToken(Payload, TEXT("RightSocket"), RightSocket.SocketName.ToString());
		AppendScaleFingerprintTokens(Payload, TEXT("LeftScale"), LeftSocket.RelativeScale);
		AppendScaleFingerprintTokens(Payload, TEXT("RightScale"), RightSocket.RelativeScale);
		return HashUtf8Payload(Payload);
	}

	// 한 Wheel의 Bounds + authored Socket Scale을 공용 Runtime helper로 파생합니다.
	bool TryDeriveSocketScaleWheelSize(
		FResolverContext& Context,
		const TCHAR* WheelLabel,
		const TCHAR* AnchorName,
		const FCFVehicleWheelAssetSnapshot* WheelSource,
		const FCFVehicleSocketSnapshot* SocketSnapshot,
		FCFDerivedWheelSize& OutWheelSize)
	{
		if (!WheelSource || !WheelSource->bAssetLoaded)
		{
			AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Blocked, TEXT("WheelSocketSizeWheelMissing"), FString::Printf(TEXT("Socket Scale Wheel Size source mesh가 없습니다: %s"), WheelLabel));
			return false;
		}
		if (!SocketSnapshot || !SocketSnapshot->bFound)
		{
			return false;
		}

		FString DeriveError;
		if (!FCFWheelSizeUtils::DeriveWheelSizeFromBoundsAndScale(WheelSource->BoundsExtent, SocketSnapshot->RelativeScale, OutWheelSize, DeriveError))
		{
			const FCFVehicleFieldPath ScalePath = FindScalarPath(*FString::Printf(TEXT("VehicleLayoutConfig.%s.RelativeScale"), AnchorName));
			AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Blocked, TEXT("WheelSocketScaleInvalid"), FString::Printf(TEXT("%s Wheel Size 계산 실패: %s"), WheelLabel, *DeriveError), &ScalePath);
			return false;
		}
		return true;
	}

	// Socket Scale mode에서 axle 두 Wheel을 검증하고 Radius/Width proposal을 생성합니다.
	void AddSocketScaleAxleProposals(
		FResolverContext& Context,
		const TCHAR* AxleLabel,
		const TCHAR* RadiusPath,
		const TCHAR* WidthPath,
		const FCFVehicleWheelAssetSnapshot& LeftWheel,
		const FCFVehicleWheelAssetSnapshot& RightWheel,
		const FCFVehicleSocketSnapshot* LeftSocket,
		const FCFVehicleSocketSnapshot* RightSocket,
		const TCHAR* LeftAnchorName,
		const TCHAR* RightAnchorName)
	{
		// 각 side는 자기 Wheel을 우선하고 같은 axle 반대편 Wheel을 bounds fallback으로 사용합니다.
		const FCFVehicleWheelAssetSnapshot* LeftWheelSource = SelectWheelSnapshot(LeftWheel, RightWheel);
		const FCFVehicleWheelAssetSnapshot* RightWheelSource = SelectWheelSnapshot(RightWheel, LeftWheel);

		FCFDerivedWheelSize LeftSize;
		FCFDerivedWheelSize RightSize;
		const bool bLeftValid = TryDeriveSocketScaleWheelSize(Context, *FString::Printf(TEXT("%s.Left"), AxleLabel), LeftAnchorName, LeftWheelSource, LeftSocket, LeftSize);
		const bool bRightValid = TryDeriveSocketScaleWheelSize(Context, *FString::Printf(TEXT("%s.Right"), AxleLabel), RightAnchorName, RightWheelSource, RightSocket, RightSize);
		if (!bLeftValid || !bRightValid || !LeftWheelSource || !RightWheelSource || !LeftSocket || !RightSocket)
		{
			return;
		}

		if (!FCFWheelSizeUtils::AreWheelSizesCompatible(LeftSize, RightSize))
		{
			AddIssue(
				Context,
				Context.Result.ResolverValidation,
				ECFVehicleValidationSeverity::Blocked,
				TEXT("WheelSocketAxleSizeMismatch"),
				FString::Printf(
					TEXT("%s 좌/우 Socket-derived Wheel Size가 일치하지 않습니다. Left(R=%.4f W=%.4f) Right(R=%.4f W=%.4f)"),
					AxleLabel,
					LeftSize.RadiusCm,
					LeftSize.WidthCm,
					RightSize.RadiusCm,
					RightSize.WidthCm));
			return;
		}

		// tolerance 안에서 좌우 편향을 만들지 않도록 axle 최종값은 양쪽 derived 결과의 평균을 사용합니다.
		const float AxleRadiusCm = (LeftSize.RadiusCm + RightSize.RadiusCm) * 0.5f;
		const float AxleWidthCm = (LeftSize.WidthCm + RightSize.WidthCm) * 0.5f;
		const FString AxleFingerprint = BuildSocketScaleAxleFingerprint(*LeftWheelSource, *RightWheelSource, *LeftSocket, *RightSocket);
		AddMeasurementProposal(Context, FindScalarPath(RadiusPath), AxleRadiusCm, AxleFingerprint, TEXT("WheelSocketScale.Radius.v1"));
		AddMeasurementProposal(Context, FindScalarPath(WidthPath), AxleWidthCm, AxleFingerprint, TEXT("WheelSocketScale.Width.v1"));
	}

	// Socket Scale mode에서 optional FR/RL/RR Wheel Mesh가 비어 있으면 canonical FL Wheel을 재사용합니다.
	const FCFVehicleWheelAssetSnapshot& ResolveSocketScaleWheelSnapshot(
		const FCFVehicleWheelAssetSnapshot& RequestedWheel,
		const FCFVehicleWheelAssetSnapshot& FallbackWheelFL)
	{
		return RequestedWheel.bAssetLoaded ? RequestedWheel : FallbackWheelFL;
	}

	// R6 Wheel measurement proposal을 Legacy Bounds mode 또는 USER Socket Scale mode로 분기해 생성합니다.
	void RunR6MeasurementProposals(FResolverContext& Context)
	{
		const bool bSocketScaleMode = Context.Request.Recipe.WheelVisualIntent.Mode == ECFWheelVisualIntentMode::SocketScaleFromChassis;

		if (bSocketScaleMode)
		{
			const FCFVehicleSocketSnapshot* SocketFL = Context.Request.Assets.FindChassisSocket(ResolveWheelSocketName(Context.Request.Recipe.AssetIntent.BodyWheelSocketFL, TEXT("Wheel_Anchor_FL")));
			const FCFVehicleSocketSnapshot* SocketFR = Context.Request.Assets.FindChassisSocket(ResolveWheelSocketName(Context.Request.Recipe.AssetIntent.BodyWheelSocketFR, TEXT("Wheel_Anchor_FR")));
			const FCFVehicleSocketSnapshot* SocketRL = Context.Request.Assets.FindChassisSocket(ResolveWheelSocketName(Context.Request.Recipe.AssetIntent.BodyWheelSocketRL, TEXT("Wheel_Anchor_RL")));
			const FCFVehicleSocketSnapshot* SocketRR = Context.Request.Assets.FindChassisSocket(ResolveWheelSocketName(Context.Request.Recipe.AssetIntent.BodyWheelSocketRR, TEXT("Wheel_Anchor_RR")));

			const FCFVehicleWheelAssetSnapshot& EffectiveWheelFL = Context.Request.Assets.WheelFL;
			const FCFVehicleWheelAssetSnapshot& EffectiveWheelFR = ResolveSocketScaleWheelSnapshot(Context.Request.Assets.WheelFR, EffectiveWheelFL);
			const FCFVehicleWheelAssetSnapshot& EffectiveWheelRL = ResolveSocketScaleWheelSnapshot(Context.Request.Assets.WheelRL, EffectiveWheelFL);
			const FCFVehicleWheelAssetSnapshot& EffectiveWheelRR = ResolveSocketScaleWheelSnapshot(Context.Request.Assets.WheelRR, EffectiveWheelFL);

			AddSocketScaleAxleProposals(Context, TEXT("Front"), TEXT("VehicleMovementConfig.FrontWheelRadius"), TEXT("VehicleMovementConfig.FrontWheelWidth"), EffectiveWheelFL, EffectiveWheelFR, SocketFL, SocketFR, TEXT("WheelAnchorFL"), TEXT("WheelAnchorFR"));
			AddSocketScaleAxleProposals(Context, TEXT("Rear"), TEXT("VehicleMovementConfig.RearWheelRadius"), TEXT("VehicleMovementConfig.RearWheelWidth"), EffectiveWheelRL, EffectiveWheelRR, SocketRL, SocketRR, TEXT("WheelAnchorRL"), TEXT("WheelAnchorRR"));
		}
		else
		{
			// Legacy/current Base/Profile/default radius measure mode입니다.
			const ECFWheelMeshRadiusMeasureMode MeasureMode = ReadCurrentMeasureMode(Context);
			const FCFVehicleWheelAssetSnapshot* FrontWheel = SelectWheelSnapshot(Context.Request.Assets.WheelFL, Context.Request.Assets.WheelFR);
			const FCFVehicleWheelAssetSnapshot* RearWheel = SelectWheelSnapshot(Context.Request.Assets.WheelRL, Context.Request.Assets.WheelRR);

			if (FrontWheel)
			{
				const FString FrontFingerprint = BuildAxleFingerprint(Context.Request.Assets.WheelFL, Context.Request.Assets.WheelFR);
				AddMeasurementProposal(Context, FindScalarPath(TEXT("VehicleMovementConfig.FrontWheelRadius")), MeasureWheelRadius(FrontWheel->BoundsExtent, MeasureMode), FrontFingerprint, TEXT("WheelBounds.Radius.v1"));
				AddMeasurementProposal(Context, FindScalarPath(TEXT("VehicleMovementConfig.FrontWheelWidth")), FMath::Abs(FrontWheel->BoundsExtent.Y) * 2.0f, FrontFingerprint, TEXT("WheelBounds.WidthAxisY.v1"));
			}

			if (RearWheel)
			{
				const FString RearFingerprint = BuildAxleFingerprint(Context.Request.Assets.WheelRL, Context.Request.Assets.WheelRR);
				AddMeasurementProposal(Context, FindScalarPath(TEXT("VehicleMovementConfig.RearWheelRadius")), MeasureWheelRadius(RearWheel->BoundsExtent, MeasureMode), RearFingerprint, TEXT("WheelBounds.Radius.v1"));
				AddMeasurementProposal(Context, FindScalarPath(TEXT("VehicleMovementConfig.RearWheelWidth")), FMath::Abs(RearWheel->BoundsExtent.Y) * 2.0f, RearFingerprint, TEXT("WheelBounds.WidthAxisY.v1"));
			}

			if (Context.Request.Recipe.AssetAdoption.bUseSuggestedRadiusMeasureMode)
			{
				AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Blocked, TEXT("RadiusModeSuggestionNotFrozen"), TEXT("bUseSuggestedRadiusMeasureMode가 활성화됐지만 P0-08E에는 bounds에서 radius mode를 자동 선택하는 Frozen rule이 없습니다. 임의 추정하지 않습니다."));
			}
		}

		Context.Result.MeasurementProposals.Sort([](const FCFVehicleMeasurementProposal& Left, const FCFVehicleMeasurementProposal& Right)
		{
			return Left.FieldPath.ToCanonicalString(true) < Right.FieldPath.ToCanonicalString(true);
		});
	}

	// 특정 measurement field의 proposal을 찾습니다.
	const FCFVehicleMeasurementProposal* FindMeasurementProposal(const FResolverContext& Context, const TCHAR* CanonicalPath)
	{
		return Context.Result.MeasurementProposals.FindByPredicate([CanonicalPath](const FCFVehicleMeasurementProposal& Proposal)
		{
			return Proposal.FieldPath.ToCanonicalString(true) == CanonicalPath;
		});
	}

	// Accepted measurement 하나가 stored fingerprint와 일치할 때만 AssetDerived effective candidate를 추가합니다.
	void ApplyAcceptedMeasurement(
		FResolverContext& Context,
		const bool bAccepted,
		const TCHAR* CanonicalPath,
		const FString& AcceptedFingerprint)
	{
		if (!bAccepted)
		{
			return;
		}

		// R6에서 생성된 exact measurement proposal입니다.
		const FCFVehicleMeasurementProposal* Proposal = FindMeasurementProposal(Context, CanonicalPath);
		if (!Proposal)
		{
			AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Blocked, TEXT("AcceptedMeasurementMissing"), FString::Printf(TEXT("채택된 measurement의 현재 proposal이 없습니다: %s"), CanonicalPath));
			return;
		}
		if (AcceptedFingerprint.IsEmpty() || AcceptedFingerprint != Proposal->AssetFingerprint)
		{
			AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Blocked, TEXT("AcceptedMeasurementFingerprintMismatch"), FString::Printf(TEXT("채택 당시 Asset fingerprint와 현재 measurement fingerprint가 다릅니다: %s"), CanonicalPath), &Proposal->FieldPath);
			return;
		}

		AddCandidate(Context, Proposal->FieldPath, Proposal->MeasuredCandidateValue, ECFVehicleSourceType::AssetDerived, FString::Printf(TEXT("Measurement.%s"), *Proposal->MeasurementRuleId.ToString()), 0, Proposal->AssetFingerprint);
	}

	// R7 Explicit accepted measurement만 effective candidate로 적용합니다.
	void RunR7AcceptedMeasurements(FResolverContext& Context)
	{
		const FCFVehicleAssetAdoption& Adoption = Context.Request.Recipe.AssetAdoption;
		ApplyAcceptedMeasurement(Context, Adoption.bUseMeasuredFrontRadius, TEXT("VehicleMovementConfig.FrontWheelRadius"), Adoption.FrontRadiusAssetFingerprint);
		ApplyAcceptedMeasurement(Context, Adoption.bUseMeasuredRearRadius, TEXT("VehicleMovementConfig.RearWheelRadius"), Adoption.RearRadiusAssetFingerprint);
		ApplyAcceptedMeasurement(Context, Adoption.bUseMeasuredFrontWidth, TEXT("VehicleMovementConfig.FrontWheelWidth"), Adoption.FrontWidthAssetFingerprint);
		ApplyAcceptedMeasurement(Context, Adoption.bUseMeasuredRearWidth, TEXT("VehicleMovementConfig.RearWheelWidth"), Adoption.RearWidthAssetFingerprint);
	}

	// Generic persisted field override value를 지정 source precedence layer로 적용합니다.
	void ApplyStoredOverrideLayer(
		FResolverContext& Context,
		const TArray<FCFVehicleFieldOverride>& Overrides,
		const ECFVehicleSourceType SourceType,
		const FString& SourceId,
		const FString& SourceFingerprint)
	{
		for (const FCFVehicleFieldOverride& Override : Overrides)
		{
			AddCandidate(Context, Override.FieldPath, Override.OverrideValue, SourceType, SourceId, Context.Request.Recipe.AuthoringRevision, SourceFingerprint);
		}
	}

	// R8 Existing Definition Legacy Pin을 Frozen higher precedence layer로 적용합니다.
	void RunR8LegacyPins(FResolverContext& Context)
	{
		ApplyStoredOverrideLayer(Context, Context.Request.Recipe.ImportState.LegacyPinnedFields, ECFVehicleSourceType::LegacyImportedPinnedBaseline, TEXT("Recipe.ImportState.LegacyPins"), Context.Request.Recipe.RecipeFingerprint);
	}

	// Active Mount row의 hidden legacy leaf를 current serialized value 또는 C++ struct default로 deterministic 보존합니다.
	void ApplyActiveMountLegacySerializedFallbacks(FResolverContext& Context)
	{
		for (const FCFMountIntent& MountIntent : Context.Request.Recipe.MountIntents)
		{
			if (MountIntent.MountProfileId.IsNone())
			{
				continue;
			}

			for (const FCFVehicleFieldDescriptor& Descriptor : FCFVehicleFieldRegistry::GetDescriptors())
			{
				if (!Descriptor.bLegacySerialized
					|| Descriptor.StablePathPattern.CollectionPropertyName != TEXT("MountProfiles"))
				{
					continue;
				}

				const FCFVehicleFieldPath ExactPath = MakeExactCollectionPath(*Descriptor.GetCanonicalPattern(), MountIntent.MountProfileId);
				const FString ExactCanonicalPath = ExactPath.ToCanonicalString(true);

				const bool bHasStoredLegacyOverride = Context.Request.Recipe.ImportState.LegacySerializedFields.ContainsByPredicate(
					[&ExactCanonicalPath](const FCFVehicleFieldOverride& Override)
					{
						return Override.FieldPath.ToCanonicalString(true) == ExactCanonicalPath;
					});
				if (bHasStoredLegacyOverride)
				{
					continue;
				}

				const FCFVehicleFieldEntry* SourceEntry = nullptr;
				FString SourceId;
				if (Context.Request.bHasCurrentDefinition)
				{
					SourceEntry = Context.Request.CurrentDefinition.SortedFields.FindByPredicate(
						[&ExactCanonicalPath](const FCFVehicleFieldEntry& Entry)
						{
							return Entry.FieldPath.ToCanonicalString(true) == ExactCanonicalPath;
						});
					if (SourceEntry)
					{
						SourceId = TEXT("CurrentDefinition.LegacySerialized");
					}
				}

				if (!SourceEntry)
				{
					const FString DefaultCanonicalPattern = Descriptor.GetCanonicalPattern();
					SourceEntry = Context.Request.ProjectDefaults.SortedFields.FindByPredicate(
						[&DefaultCanonicalPattern](const FCFVehicleFieldEntry& Entry)
						{
							return Entry.FieldPath.ToCanonicalString(true) == DefaultCanonicalPattern;
						});
					if (SourceEntry)
					{
						SourceId = TEXT("Project.CppDefaultStableMountLegacy");
					}
				}

				if (!SourceEntry)
				{
					AddIssue(
						Context,
						Context.Result.ResolverValidation,
						ECFVehicleValidationSeverity::Error,
						TEXT("MountLegacyPassthroughBaselineMissing"),
						FString::Printf(TEXT("Active Mount legacy passthrough baseline을 찾을 수 없습니다: %s"), *ExactCanonicalPath),
						&ExactPath);
					continue;
				}

				AddCandidate(
					Context,
					ExactPath,
					SourceEntry->Value,
					ECFVehicleSourceType::LegacySerializedPassthrough,
					SourceId,
					Context.Request.Recipe.AuthoringRevision,
					FCFVehicleFieldCodec::HashValue(SourceEntry->Value));
			}
		}
	}

	// R9 Hidden legacy Mount serialized field를 passthrough layer로 적용합니다.
	void RunR9LegacySerialized(FResolverContext& Context)
	{
		ApplyActiveMountLegacySerializedFallbacks(Context);
		ApplyStoredOverrideLayer(Context, Context.Request.Recipe.ImportState.LegacySerializedFields, ECFVehicleSourceType::LegacySerializedPassthrough, TEXT("Recipe.ImportState.LegacySerialized"), Context.Request.Recipe.RecipeFingerprint);
	}

	// R10 Advanced Leaf Override를 최상위 precedence layer로 적용합니다.
	void RunR10AdvancedOverrides(FResolverContext& Context)
	{
		ApplyStoredOverrideLayer(Context, Context.Request.Recipe.AdvancedOverrides, ECFVehicleSourceType::AdvancedLeafOverride, TEXT("Recipe.AdvancedOverride"), Context.Request.Recipe.RecipeFingerprint);
	}

	// Current winner가 Project Compatibility Default가 아닌 authored source인지 확인합니다.
	bool HasAuthoredWinner(const FResolverContext& Context, const TCHAR* CanonicalPath)
	{
		// 현재 field winner입니다.
		const FVehicleFieldCandidate* Winner = FindCurrentWinner(Context, CanonicalPath);
		return Winner && Winner->SourceType != ECFVehicleSourceType::ProjectCompatibilityDefault;
	}

	// R11 Derived Gate / Readiness flag candidate를 낮은 Derived precedence로 계산합니다.
	void RunR11DerivedGates(FResolverContext& Context)
	{
		static const TCHAR* LayoutPoseFields[] =
		{
			TEXT("VehicleLayoutConfig.WheelAnchorFL.RelativeLocation"), TEXT("VehicleLayoutConfig.WheelAnchorFL.RelativeRotation"), TEXT("VehicleLayoutConfig.WheelAnchorFL.RelativeScale"),
			TEXT("VehicleLayoutConfig.WheelAnchorFR.RelativeLocation"), TEXT("VehicleLayoutConfig.WheelAnchorFR.RelativeRotation"), TEXT("VehicleLayoutConfig.WheelAnchorFR.RelativeScale"),
			TEXT("VehicleLayoutConfig.WheelAnchorRL.RelativeLocation"), TEXT("VehicleLayoutConfig.WheelAnchorRL.RelativeRotation"), TEXT("VehicleLayoutConfig.WheelAnchorRL.RelativeScale"),
			TEXT("VehicleLayoutConfig.WheelAnchorRR.RelativeLocation"), TEXT("VehicleLayoutConfig.WheelAnchorRR.RelativeRotation"), TEXT("VehicleLayoutConfig.WheelAnchorRR.RelativeScale")
		};
		// 12개 Wheel Anchor leaf가 모두 non-default source인지 여부입니다.
		bool bLayoutReady = true;
		for (const TCHAR* FieldPath : LayoutPoseFields)
		{
			bLayoutReady &= HasAuthoredWinner(Context, FieldPath);
		}
		AddBoolCandidate(Context, FindScalarPath(TEXT("VehicleLayoutConfig.bUseLayoutOverrides")), bLayoutReady, ECFVehicleSourceType::RuleDerived, TEXT("Rule.DerivedGate.Layout"), 0, Context.Request.Assets.ChassisLayoutFingerprint);

		static const TCHAR* MovementDetailFields[] =
		{
			TEXT("VehicleMovementConfig.ThrottleInputScale"),
			TEXT("VehicleMovementConfig.FrontWheelMaxSteerAngle"),
			TEXT("VehicleMovementConfig.FrontWheelMaxBrakeTorque"),
			TEXT("VehicleMovementConfig.RearWheelMaxBrakeTorque"),
			TEXT("VehicleMovementConfig.RearWheelMaxHandBrakeTorque"),
			TEXT("VehicleMovementConfig.FrontWheelRadius"), TEXT("VehicleMovementConfig.RearWheelRadius"),
			TEXT("VehicleMovementConfig.FrontWheelWidth"), TEXT("VehicleMovementConfig.RearWheelWidth"),
			TEXT("VehicleMovementConfig.FrontWheelFrictionForceMultiplier"), TEXT("VehicleMovementConfig.RearWheelFrictionForceMultiplier"),
			TEXT("VehicleMovementConfig.FrontWheelCorneringStiffness"), TEXT("VehicleMovementConfig.RearWheelCorneringStiffness"),
			TEXT("VehicleMovementConfig.FrontWheelLoadRatio"), TEXT("VehicleMovementConfig.RearWheelLoadRatio"),
			TEXT("VehicleMovementConfig.FrontWheelSpringRate"), TEXT("VehicleMovementConfig.RearWheelSpringRate"),
			TEXT("VehicleMovementConfig.FrontWheelSpringPreload"), TEXT("VehicleMovementConfig.RearWheelSpringPreload"),
			TEXT("VehicleMovementConfig.FrontWheelSuspensionMaxRaise"), TEXT("VehicleMovementConfig.RearWheelSuspensionMaxRaise"),
			TEXT("VehicleMovementConfig.FrontWheelSuspensionMaxDrop"), TEXT("VehicleMovementConfig.RearWheelSuspensionMaxDrop"),
			TEXT("VehicleMovementConfig.bFrontWheelAffectedByEngine"), TEXT("VehicleMovementConfig.bRearWheelAffectedByEngine"),
			TEXT("VehicleMovementConfig.FrontWheelSweepShape"), TEXT("VehicleMovementConfig.RearWheelSweepShape")
		};
		// Detailed Wheel/Throttle authored path가 하나라도 effective인지 여부입니다.
		bool bMovementOverridesReady = false;
		for (const TCHAR* FieldPath : MovementDetailFields)
		{
			bMovementOverridesReady |= HasAuthoredWinner(Context, FieldPath);
		}
		AddBoolCandidate(Context, FindScalarPath(TEXT("VehicleMovementConfig.bUseMovementOverrides")), bMovementOverridesReady, ECFVehicleSourceType::RuleDerived, TEXT("Rule.DerivedGate.Movement"), 0, Context.Request.Recipe.RecipeFingerprint);

		// COM vector가 Project Default 이외 source에 의해 소유되는지 여부입니다.
		const bool bCenterOfMassOverrideReady = HasAuthoredWinner(Context, TEXT("VehicleMovementConfig.CenterOfMassOverride"));
		AddBoolCandidate(Context, FindScalarPath(TEXT("VehicleMovementConfig.bEnableCenterOfMassOverride")), bCenterOfMassOverrideReady, ECFVehicleSourceType::RuleDerived, TEXT("Rule.DerivedGate.CenterOfMass"), 0, Context.Request.Recipe.RecipeFingerprint);

		// Managed 또는 Legacy Pin에서 WheelVisual group을 Adopt한 Recipe의 readiness입니다.
		const bool bWheelVisualReady = Context.Request.Recipe.ImportState.ManageState == ECFVehicleManageState::Managed
			|| Context.Request.Recipe.ImportState.AdoptedGroups.Contains(ECFVehicleAdoptGroup::WheelVisual);
		AddBoolCandidate(Context, FindScalarPath(TEXT("WheelVisualConfig.bUseWheelVisualOverrides")), bWheelVisualReady, ECFVehicleSourceType::RuleDerived, TEXT("Rule.DerivedGate.WheelVisual"), 0, Context.Request.Recipe.RecipeFingerprint);

		// VehicleSpecific + valid Profile일 때만 DriveState runtime gate를 true로 계산합니다.
		const bool bDriveStateReady = Context.Request.Recipe.DriveStateMode == ECFVehicleDriveStateMode::VehicleSpecific && Context.Request.Profiles.DriveStateSource.IsPresent();
		AddBoolCandidate(Context, FindScalarPath(TEXT("DriveStateConfig.bUseDriveStateOverrides")), bDriveStateReady, ECFVehicleSourceType::RuleDerived, TEXT("Rule.DerivedGate.DriveState"), 0, Context.Request.Recipe.RecipeFingerprint);
	}

	// Recipe semantic stable collection에서 반드시 존재해야 하는 local field candidate를 검사합니다.
	void ValidateCollectionCompleteness(FResolverContext& Context)
	{
		for (const FCFHardpointIntent& HardpointIntent : Context.Request.Recipe.HardpointIntents)
		{
			if (HardpointIntent.LocationSlotId.IsNone())
			{
				continue;
			}
			static const TCHAR* RequiredHardpointLeaves[] = {TEXT("LocationSlotId"), TEXT("LocationCategory"), TEXT("SocketName"), TEXT("LocalLocation"), TEXT("LocalRotation")};
			for (const TCHAR* LeafName : RequiredHardpointLeaves)
			{
				// Hardpoint exact leaf path입니다.
				const FString CanonicalPath = FString::Printf(TEXT("HardpointSlots[LocationSlotId=%s].%s"), *HardpointIntent.LocationSlotId.ToString(), LeafName);
				if (!FindCurrentWinner(Context, CanonicalPath))
				{
					// Manual SocketName=None local pose는 Advanced Override 또는 Legacy Pin이 없으면 incomplete입니다.
					const FCFVehicleFieldPath ExactPath = MakeExactCollectionPath(*FString::Printf(TEXT("HardpointSlots[LocationSlotId=*].%s"), LeafName), HardpointIntent.LocationSlotId);
					AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Blocked, TEXT("HardpointResolvedFieldMissing"), FString::Printf(TEXT("Hardpoint resolved leaf가 없습니다: %s"), *CanonicalPath), &ExactPath);
				}
			}
		}

		for (const FCFMountIntent& MountIntent : Context.Request.Recipe.MountIntents)
		{
			if (MountIntent.MountProfileId.IsNone())
			{
				continue;
			}
			static const TCHAR* RequiredMountLeaves[] = {TEXT("MountProfileId"), TEXT("LocationSlotRef"), TEXT("MountType"), TEXT("SizeLimit"), TEXT("DefaultEquipmentPresetData"), TEXT("bExposedModule")};
			for (const TCHAR* LeafName : RequiredMountLeaves)
			{
				// Mount exact active leaf path입니다.
				const FString CanonicalPath = FString::Printf(TEXT("MountProfiles[MountProfileId=%s].%s"), *MountIntent.MountProfileId.ToString(), LeafName);
				if (!FindCurrentWinner(Context, CanonicalPath))
				{
					const FCFVehicleFieldPath ExactPath = MakeExactCollectionPath(*FString::Printf(TEXT("MountProfiles[MountProfileId=*].%s"), LeafName), MountIntent.MountProfileId);
					AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Blocked, TEXT("MountResolvedFieldMissing"), FString::Printf(TEXT("Mount active resolved leaf가 없습니다: %s"), *CanonicalPath), &ExactPath);
				}
			}
		}
	}

	// R12 Cross-field Resolver Validation Foundation을 수행합니다.
	void RunR12CrossFieldValidation(FResolverContext& Context)
	{
		ValidateCollectionCompleteness(Context);

		// Managed Wheel geometry는 proposal만으로 effective source가 되지 않으므로 explicit adoption/override/pin을 요구합니다.
		if (Context.Request.Recipe.ImportState.ManageState == ECFVehicleManageState::Managed)
		{
			static const TCHAR* WheelGeometryFields[] =
			{
				TEXT("VehicleMovementConfig.FrontWheelRadius"), TEXT("VehicleMovementConfig.RearWheelRadius"),
				TEXT("VehicleMovementConfig.FrontWheelWidth"), TEXT("VehicleMovementConfig.RearWheelWidth")
			};
			for (const TCHAR* CanonicalPath : WheelGeometryFields)
			{
				// Project Default만 winner인 상태는 explicit geometry choice가 아직 없다는 뜻입니다.
				const FVehicleFieldCandidate* Winner = FindCurrentWinner(Context, CanonicalPath);
				if (!Winner || Winner->SourceType == ECFVehicleSourceType::ProjectCompatibilityDefault)
				{
					const FCFVehicleFieldPath FieldPath = FindScalarPath(CanonicalPath);
					AddIssue(Context, Context.Result.ResolverValidation, ECFVehicleValidationSeverity::Blocked, TEXT("WheelMeasurementNotAdopted"), FString::Printf(TEXT("Managed Vehicle의 Wheel geometry가 아직 명시적으로 채택/override되지 않았습니다: %s"), CanonicalPath), &FieldPath);
				}
			}
		}
	}

	// Descriptor dependency key를 현재 immutable request/candidate facts의 fingerprint로 변환합니다.
	FString BuildDependencyFingerprint(
		const FResolverContext& Context,
		const FCFVehicleFieldDescriptor& Descriptor)
	{
		// Dependency key 순서대로 만드는 canonical payload입니다.
		FString Payload;
		for (const FName Dependency : Descriptor.RequiredDependencies)
		{
			// Frozen stable dependency key 문자열입니다.
			const FString Key = Dependency.ToString();
			FString Value;
			if (Key == TEXT("Project.CompatibilityDefaults")) Value = Context.Request.ProjectDefaults.DefinitionHash;
			else if (Key == TEXT("Profile.VehicleBase")) Value = Context.Request.Profiles.BaseSource.ProfileFingerprint;
			else if (Key == TEXT("Profile.Drivetrain")) Value = Context.Request.Profiles.DrivetrainSource.ProfileFingerprint;
			else if (Key == TEXT("Profile.Handling")) Value = Context.Request.Profiles.HandlingSource.ProfileFingerprint;
			else if (Key == TEXT("Profile.Performance")) Value = Context.Request.Profiles.PerformanceSource.ProfileFingerprint;
			else if (Key == TEXT("Profile.DriveState")) Value = Context.Request.Profiles.DriveStateSource.ProfileFingerprint;
			else if (Key.StartsWith(TEXT("Recipe."))) Value = Context.Request.Recipe.RecipeFingerprint;
			else if (Key == TEXT("Asset.ChassisSockets")) Value = Context.Request.Assets.ChassisLayoutFingerprint;
			else if (Key == TEXT("Asset.WheelBounds"))
			{
				AppendToken(Value, TEXT("FL"), Context.Request.Assets.WheelFL.MeasureFingerprint);
				AppendToken(Value, TEXT("FR"), Context.Request.Assets.WheelFR.MeasureFingerprint);
				AppendToken(Value, TEXT("RL"), Context.Request.Assets.WheelRL.MeasureFingerprint);
				AppendToken(Value, TEXT("RR"), Context.Request.Assets.WheelRR.MeasureFingerprint);
				Value = HashUtf8Payload(Value);
			}
			else if (Key.StartsWith(TEXT("Resolved.")))
			{
				// Derived dependency는 현재 candidate winner path/value 집합으로 versioned 됩니다.
				TArray<FString> StackKeys;
				Context.CandidateStacks.GetKeys(StackKeys);
				StackKeys.Sort();
				FString ResolvedPayload;
				for (const FString& StackKey : StackKeys)
				{
					const FVehicleFieldCandidate* Winner = FindCurrentWinner(Context, StackKey);
					if (Winner)
					{
						AppendToken(ResolvedPayload, TEXT("Path"), StackKey);
						AppendToken(ResolvedPayload, TEXT("Value"), FCFVehicleFieldCodec::HashValue(Winner->Value));
					}
				}
				Value = HashUtf8Payload(ResolvedPayload);
			}
			else if (Key == TEXT("Resolver.OptionalBaseMassContext"))
			{
				const FVehicleFieldCandidate* MassWinner = FindCurrentWinner(Context, TEXT("BaseVehicleMassKg"));
				Value = MassWinner ? FCFVehicleFieldCodec::HashValue(MassWinner->Value) : TEXT("None");
			}
			AppendToken(Payload, *Key, Value);
		}
		return HashUtf8Payload(Payload);
	}

	// Candidate 하나의 field-level source signature를 Frozen Section 22.27 input으로 생성합니다.
	FString BuildCandidateSignature(
		const FResolverContext& Context,
		const FVehicleFieldCandidate& Candidate)
	{
		// Stable path/source/rule/dependency/contract canonical payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("HashFormatRevision"), FString::FromInt(ResolverHashFormatRevision));
		AppendToken(Payload, TEXT("FieldPath"), Candidate.FieldPath.ToCanonicalString(true));
		AppendToken(Payload, TEXT("SourceType"), FString::FromInt(static_cast<int32>(Candidate.SourceType)));
		AppendToken(Payload, TEXT("SourceId"), Candidate.SourceId);
		AppendToken(Payload, TEXT("SourcePayloadFingerprint"), Candidate.SourceFingerprint);
		AppendToken(Payload, TEXT("ResolveRule"), Candidate.Descriptor ? FString::FromInt(static_cast<int32>(Candidate.Descriptor->ResolveRule)) : TEXT("-1"));
		AppendToken(Payload, TEXT("ResolverContractRevision"), FString::FromInt(Context.Request.ResolverContractRevision));
		AppendToken(Payload, TEXT("DependencyFingerprint"), Candidate.Descriptor ? BuildDependencyFingerprint(Context, *Candidate.Descriptor) : TEXT("None"));
		return HashUtf8Payload(Payload);
	}

	// R13 Source Trace / Resolved Hash를 canonical order로 생성합니다.
	void RunR13SourceTraceAndHash(FResolverContext& Context)
	{
		Context.Result.SortedResolvedFields.Reset();
		Context.Result.PreviewSourceTrace.Reset();

		// Canonical exact path sorting용 stack key 목록입니다.
		TArray<FString> CanonicalPaths;
		Context.CandidateStacks.GetKeys(CanonicalPaths);
		CanonicalPaths.Sort();

				// Global source signature canonical payload입니다.
		FString SourceSetPayload;
		// Existing Definition Snapshot hash authority에 전달할 Resolver-owned exact field entries입니다.
		TArray<FCFVehicleFieldEntry> DefinitionHashFields;
		DefinitionHashFields.Reserve(CanonicalPaths.Num());

		for (const FString& CanonicalPath : CanonicalPaths)
		{
			// Current exact field candidate stack copy입니다.
			TArray<FVehicleFieldCandidate> Stack = Context.CandidateStacks.FindChecked(CanonicalPath);
			Stack.Sort([](const FVehicleFieldCandidate& Left, const FVehicleFieldCandidate& Right)
			{
				if (Left.Precedence != Right.Precedence) return Left.Precedence < Right.Precedence;
				if (Left.SourceType != Right.SourceType) return static_cast<uint8>(Left.SourceType) < static_cast<uint8>(Right.SourceType);
				if (Left.SourceId != Right.SourceId) return Left.SourceId < Right.SourceId;
				return FCFVehicleFieldCodec::HashValue(Left.Value) < FCFVehicleFieldCodec::HashValue(Right.Value);
			});
			if (Stack.IsEmpty())
			{
				continue;
			}

			// Highest precedence winner index입니다.
			const int32 EffectiveIndex = Stack.Num() - 1;
			// Public source trace row입니다.
			FCFVehicleSourceTrace& Trace = Context.Result.PreviewSourceTrace.AddDefaulted_GetRef();
			Trace.FieldPath = Stack[EffectiveIndex].FieldPath;
			Trace.EffectiveLayerIndex = EffectiveIndex;

			// Shadow signature canonical payload입니다.
			FString ShadowPayload;
			for (int32 CandidateIndex = 0; CandidateIndex < Stack.Num(); ++CandidateIndex)
			{
				// Current candidate입니다.
				const FVehicleFieldCandidate& Candidate = Stack[CandidateIndex];
				// Public preview layer입니다.
				FCFVehicleSourceLayer& Layer = Trace.Layers.AddDefaulted_GetRef();
				Layer.SourceType = Candidate.SourceType;
				Layer.SourceId = Candidate.SourceId;
				Layer.SourceRevision = Candidate.SourceRevision;
				Layer.SourceFingerprint = Candidate.SourceFingerprint;
				Layer.ValueHash = FCFVehicleFieldCodec::HashValue(Candidate.Value);
				Layer.SourceSignature = BuildCandidateSignature(Context, Candidate);
				Layer.bEffective = CandidateIndex == EffectiveIndex;
				if (Layer.bEffective)
				{
					Trace.EffectiveSourceSignature = Layer.SourceSignature;
				}
				else
				{
					AppendToken(ShadowPayload, TEXT("Shadow"), Layer.SourceSignature);
				}
			}
			Trace.ShadowSourceSignature = HashUtf8Payload(ShadowPayload);

			// Effective resolved field row입니다.
			FCFVehicleResolvedField& ResolvedField = Context.Result.SortedResolvedFields.AddDefaulted_GetRef();
			ResolvedField.FieldPath = Stack[EffectiveIndex].FieldPath;
			ResolvedField.Value = Stack[EffectiveIndex].Value;
			ResolvedField.SourceTraceIndex = Context.Result.PreviewSourceTrace.Num() - 1;

						AppendToken(SourceSetPayload, TEXT("Path"), CanonicalPath);
			AppendToken(SourceSetPayload, TEXT("EffectiveSource"), Trace.EffectiveSourceSignature);

			// Resolver leaf set을 Definition Snapshot과 동일 canonical path/type/value hash format으로 전달할 entry입니다.
			FCFVehicleFieldEntry& DefinitionEntry = DefinitionHashFields.AddDefaulted_GetRef();
			DefinitionEntry.FieldPath = ResolvedField.FieldPath;
			DefinitionEntry.Value = ResolvedField.Value;
		}

		Context.Result.SourceSignature = HashUtf8Payload(SourceSetPayload);

		// SnapshotBuilder v1.0부터 사용한 Definition hash format을 Resolver에서도 유일한 authority로 재사용합니다.
		FString DefinitionHashError;
		if (!FCFVehicleSnapshotBuilder::BuildDefinitionHashFromFields(DefinitionHashFields, Context.Result.ResolvedDefinitionHash, DefinitionHashError))
		{
			AddIssue(
				Context,
				Context.Result.ResolverValidation,
				ECFVehicleValidationSeverity::Error,
				TEXT("ResolvedDefinitionHashFailed"),
				DefinitionHashError);
				}
	}

	// Definition Snapshot exact field를 canonical path로 찾습니다.
	const FCFVehicleFieldEntry* FindDefinitionField(const FCFVehicleDefinitionSnapshot& Snapshot, const FString& CanonicalPath)
	{
		return Snapshot.SortedFields.FindByPredicate([&CanonicalPath](const FCFVehicleFieldEntry& Entry)
		{
			return Entry.FieldPath.ToCanonicalString(true) == CanonicalPath;
		});
	}

	// R14 Current Definition과 deterministic leaf diff foundation을 생성합니다.
	void RunR14FieldDiff(FResolverContext& Context)
	{
		Context.Result.FieldDiff.Reset();
		if (!Context.Request.bHasCurrentDefinition)
		{
			return;
		}

		for (const FCFVehicleResolvedField& ResolvedField : Context.Result.SortedResolvedFields)
		{
			// Resolved exact canonical path입니다.
			const FString CanonicalPath = ResolvedField.FieldPath.ToCanonicalString(true);
			// Current Definition matching exact leaf입니다.
			const FCFVehicleFieldEntry* CurrentField = FindDefinitionField(Context.Request.CurrentDefinition, CanonicalPath);
			if (CurrentField && CurrentField->Value.PropertyTypeSignature == ResolvedField.Value.PropertyTypeSignature && CurrentField->Value.CanonicalValueText == ResolvedField.Value.CanonicalValueText)
			{
				continue;
			}

			// Leaf-level deterministic diff foundation row입니다.
			FCFVehicleFieldDiff& Diff = Context.Result.FieldDiff.AddDefaulted_GetRef();
			Diff.FieldPath = ResolvedField.FieldPath;
			Diff.bHasAfterValue = true;
			Diff.AfterValue = ResolvedField.Value;
			Diff.SourceTraceIndex = ResolvedField.SourceTraceIndex;
			if (CurrentField)
			{
				Diff.Operation = ECFVehicleDiffOp::SetLeaf;
				Diff.bHasBeforeValue = true;
				Diff.BeforeValue = CurrentField->Value;
			}
			else
			{
				// Stable collection identity leaf가 신규라면 AddArrayElement marker를 사용하고 나머지 leaf는 before 없는 SetLeaf로 둡니다.
				const FCFVehicleFieldDescriptor* Descriptor = FindDescriptor(ResolvedField.FieldPath);
				Diff.Operation = Descriptor && Descriptor->bIdentityField ? ECFVehicleDiffOp::AddArrayElement : ECFVehicleDiffOp::SetLeaf;
			}
		}

		Context.Result.FieldDiff.Sort([](const FCFVehicleFieldDiff& Left, const FCFVehicleFieldDiff& Right)
		{
			return Left.FieldPath.ToCanonicalString(true) < Right.FieldPath.ToCanonicalString(true);
		});
	}

		// R15 Resolver leaf set을 transient Definition으로 materialize하고 기존 UCFVDAValidator 및 hash readback을 실행합니다.
	ECFVehicleResolverStageStatus RunR15DefinitionMaterialization(FResolverContext& Context)
	{
		if (Context.bHasError)
		{
			return ECFVehicleResolverStageStatus::Failed;
		}

		// Transient candidate lifetime 밖으로 보존할 R15 value-copy 결과입니다.
		FCFVehicleMaterializationResult MaterializationResult;
		// Codec/materialization/readback 자체 실패 이유입니다.
		FString MaterializationError;
		if (!FCFVehicleMaterializer::MaterializeAndValidate(Context.Result.SortedResolvedFields, MaterializationResult, MaterializationError))
		{
			AddIssue(
				Context,
				Context.Result.DefinitionValidation,
				ECFVehicleValidationSeverity::Error,
				TEXT("DefinitionMaterializationFailed"),
				MaterializationError);
			return ECFVehicleResolverStageStatus::Failed;
		}

		Context.Result.DefinitionValidation = MoveTemp(MaterializationResult.DefinitionValidation);

		if (MaterializationResult.ResolvedReadbackSnapshot.DefinitionHash != Context.Result.ResolvedDefinitionHash)
		{
			// Hash mismatch의 첫 exact leaf 차이를 사람이 바로 확인할 수 있는 진단 문자열입니다.
			FString FirstMismatchDetail = TEXT("<leaf mismatch not found>");

			for (const FCFVehicleResolvedField& ResolvedField : Context.Result.SortedResolvedFields)
			{
				// 현재 Resolver leaf의 exact canonical path입니다.
				const FString CanonicalPath = ResolvedField.FieldPath.ToCanonicalString(true);

				// Materialized readback에서 같은 exact path를 가진 leaf입니다.
				const FCFVehicleFieldEntry* ReadbackField = MaterializationResult.ResolvedReadbackSnapshot.SortedFields.FindByPredicate([&CanonicalPath](const FCFVehicleFieldEntry& Entry)
				{
					return Entry.FieldPath.ToCanonicalString(true) == CanonicalPath;
				});

				if (!ReadbackField)
				{
					FirstMismatchDetail = FString::Printf(TEXT("Path=%s Readback=<missing>"), *CanonicalPath);
					break;
				}

				if (ReadbackField->Value.PropertyTypeSignature != ResolvedField.Value.PropertyTypeSignature
					|| ReadbackField->Value.CanonicalValueText != ResolvedField.Value.CanonicalValueText)
				{
					FirstMismatchDetail = FString::Printf(
						TEXT("Path=%s ResolverType=%s ReadbackType=%s ResolverValue=%s ReadbackValue=%s"),
						*CanonicalPath,
						*ResolvedField.Value.PropertyTypeSignature,
						*ReadbackField->Value.PropertyTypeSignature,
						*ResolvedField.Value.CanonicalValueText,
						*ReadbackField->Value.CanonicalValueText);
					break;
				}
			}

			AddIssue(
				Context,
				Context.Result.DefinitionValidation,
				ECFVehicleValidationSeverity::Error,
				TEXT("DefinitionHashReadbackMismatch"),
				FString::Printf(
					TEXT("Transient materialized readback hash가 Resolver hash와 다릅니다. Resolver=%s Readback=%s FirstMismatch=%s"),
					*Context.Result.ResolvedDefinitionHash,
					*MaterializationResult.ResolvedReadbackSnapshot.DefinitionHash,
					*FirstMismatchDetail));
			return ECFVehicleResolverStageStatus::Failed;
		}

		// 기존 UCFVDAValidator의 Definition Error/Blocked는 계산 엔진 실패가 아니라 Apply eligibility 차단으로 분리합니다.
		if (MaterializationResult.bHasBlockingValidation)
		{
			Context.bHasBlocked = true;
		}
		return ECFVehicleResolverStageStatus::Completed;
	}

	// R16 AppliedState와 current trace/target snapshot으로 Effective/Shadow/External Drift foundation을 생성합니다.
	void RunR16StaleDrift(FResolverContext& Context)
	{
		Context.Result.StaleReport = FCFVehicleStaleReport();
		for (const FCFVehicleAppliedTrace& AppliedTrace : Context.Request.Recipe.AppliedState.FieldTraces)
		{
			// Applied exact canonical field path입니다.
			const FString CanonicalPath = AppliedTrace.FieldPath.ToCanonicalString(true);
			// Current Resolver source trace입니다.
			const FCFVehicleSourceTrace* CurrentTrace = Context.Result.PreviewSourceTrace.FindByPredicate([&CanonicalPath](const FCFVehicleSourceTrace& Trace)
			{
				return Trace.FieldPath.ToCanonicalString(true) == CanonicalPath;
			});
			// Current Target exact field입니다.
			const FCFVehicleFieldEntry* CurrentTarget = Context.Request.bHasCurrentDefinition ? FindDefinitionField(Context.Request.CurrentDefinition, CanonicalPath) : nullptr;

			// Effective signature가 달라졌는지 여부입니다.
			const bool bEffectiveChanged = !CurrentTrace || CurrentTrace->EffectiveSourceSignature != AppliedTrace.EffectiveSourceSignature;
			// Shadow signature가 달라졌는지 여부입니다.
			const bool bShadowChanged = CurrentTrace && CurrentTrace->ShadowSourceSignature != AppliedTrace.ShadowSourceSignature;
			// Current Target value hash가 last applied value와 달라졌는지 여부입니다.
			const FString CurrentTargetHash = CurrentTarget ? FCFVehicleFieldCodec::HashValue(CurrentTarget->Value) : FString();
			const bool bExternalDrift = CurrentTarget && !AppliedTrace.LastAppliedValueHash.IsEmpty() && CurrentTargetHash != AppliedTrace.LastAppliedValueHash;
			if (!bEffectiveChanged && !bShadowChanged && !bExternalDrift)
			{
				continue;
			}

			// Public stale field row입니다.
			FCFVehicleStaleField& StaleField = Context.Result.StaleReport.Fields.AddDefaulted_GetRef();
			StaleField.FieldPath = AppliedTrace.FieldPath;
			StaleField.bEffectiveSourceChanged = bEffectiveChanged;
			StaleField.bShadowSourceChanged = bShadowChanged;
			StaleField.bExternalDrift = bExternalDrift;
			StaleField.LastAppliedSourceSignature = AppliedTrace.EffectiveSourceSignature;
			StaleField.CurrentSourceSignature = CurrentTrace ? CurrentTrace->EffectiveSourceSignature : FString();
			StaleField.LastAppliedValueHash = AppliedTrace.LastAppliedValueHash;
			StaleField.CurrentTargetValueHash = CurrentTargetHash;
			Context.Result.StaleReport.bHasEffectiveStale |= bEffectiveChanged;
			Context.Result.StaleReport.bHasShadowSourceChange |= bShadowChanged;
			Context.Result.StaleReport.bHasExternalDrift |= bExternalDrift;
		}

		Context.Result.StaleReport.Fields.Sort([](const FCFVehicleStaleField& Left, const FCFVehicleStaleField& Right)
		{
			return Left.FieldPath.ToCanonicalString(true) < Right.FieldPath.ToCanonicalString(true);
		});
	}
}

// Immutable request 하나를 Frozen R0~R16 순서로 resolve합니다.
bool FCFVehicleResolver::Resolve(const FCFVehicleResolveRequest& Request, FCFVehicleResolveResult& OutResult)
{
	OutResult = FCFVehicleResolveResult();
	OutResult.ResolverContractRevision = CurrentResolverContractRevision;

	// 한 번의 pure resolve working context입니다.
	CFVehicleResolverPrivate::FResolverContext Context{Request, OutResult};

	CFVehicleResolverPrivate::RunR0RequestValidation(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R0_RequestValidation);

	CFVehicleResolverPrivate::RunR1ProjectDefaults(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R1_ProjectDefaults);

	CFVehicleResolverPrivate::RunR2Profiles(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R2_Profiles);

	CFVehicleResolverPrivate::RunR3RecipeSemantic(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R3_RecipeSemantic);

	CFVehicleResolverPrivate::RunR4DrivingFeel(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R4_DrivingFeel);

	CFVehicleResolverPrivate::RunR5AssetSockets(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R5_AssetSockets);

	CFVehicleResolverPrivate::RunR6MeasurementProposals(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R6_MeasurementProposal);

	CFVehicleResolverPrivate::RunR7AcceptedMeasurements(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R7_AcceptedMeasurement);

	CFVehicleResolverPrivate::RunR8LegacyPins(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R8_LegacyPins);

	CFVehicleResolverPrivate::RunR9LegacySerialized(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R9_LegacySerialized);

	CFVehicleResolverPrivate::RunR10AdvancedOverrides(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R10_AdvancedOverride);

	CFVehicleResolverPrivate::RunR11DerivedGates(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R11_DerivedGates);

	CFVehicleResolverPrivate::RunR12CrossFieldValidation(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R12_CrossFieldValidation);

	CFVehicleResolverPrivate::RunR13SourceTraceAndHash(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R13_SourceTraceHash);

	CFVehicleResolverPrivate::RunR14FieldDiff(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R14_FieldDiff);

		// Frozen R15 transient Definition materialization / existing UCFVDAValidator 실행 상태입니다.
	const ECFVehicleResolverStageStatus MaterializationStageStatus = CFVehicleResolverPrivate::RunR15DefinitionMaterialization(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R15_DefinitionMaterialization, MaterializationStageStatus);

	CFVehicleResolverPrivate::RunR16StaleDrift(Context);
	CFVehicleResolverPrivate::RecordStage(Context, ECFVehicleResolverStage::R16_StaleDrift);

	if (Context.bHasError)
	{
		OutResult.ResolveStatus = ECFVehicleResolveStatus::Error;
	}
	else if (Context.bHasBlocked)
	{
		OutResult.ResolveStatus = ECFVehicleResolveStatus::Blocked;
	}
	else
	{
		OutResult.ResolveStatus = ECFVehicleResolveStatus::Success;
	}

	return OutResult.ResolveStatus != ECFVehicleResolveStatus::Error;
}
