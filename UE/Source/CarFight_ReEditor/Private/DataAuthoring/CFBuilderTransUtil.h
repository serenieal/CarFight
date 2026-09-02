// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderTransUtil.h
// Version: v1.3.1
// Date: 2026-09-01
// Description: CF-FQ-040 Guided Vehicle Builder의 Transmission review/hash/fixed-shift + ESH-03 WheelTorqueCrossoverShift diagnostic 공용 private utility입니다.
// Scope: Builder Step 5 Profile preview와 Step 7 Final Review의 기존 fixed-shift 계약을 보존하면서, ESH-03 diagnostic은 explicit gate가 활성화된 호출에서만 실행합니다.
// Changelog:
// - v1.3.1: ESH-03를 pre-DefinitionApply production path에서 비활성화하고, enabled diagnostic의 nested warning/blocker를 parent로 전파하며 ratio/radius/post-shift RPM 입력을 fail-closed하고 poor fixed-common fit 경고를 추가.
// - v1.3.0: vehicle-specific Engine TorqueCurve를 이용해 adjacent gear crossover와 least-squares fixed common ChangeUpRPM recommendation을 계산하는 WheelTorqueCrossoverShift diagnostic 추가.
// - v1.2.0: Resolver가 Success가 아닌 상태에서 compatibility/default Wheel Radius를 WSA authority처럼 소비하지 않도록 fixed-shift speed diagnostic을 fail-closed. Ratio/PostShift RPM은 유지하되 Wheel Radius 기반 ShiftSpeed는 숨기고 vehicle-specific blocker를 추가.
// - v1.1.0: field-level Claim FactKey/value semantic binding, DERIVED fresh method/input validation, GAME_BIAS semantic input 확인, gear-count/positive ratio/final/shift payload structural fail-closed를 추가.
// - v1.0.0: field-level FACT/DERIVED/GAME_BIAS validation, deterministic proposal hash, expected shift speed/post-shift RPM diagnostic을 최초 구현.
// Migration:
// - Generic Resolver와 Project Compatibility Default는 변경하지 않습니다.
// - LegacyCompatible Recipe는 기존 bUseTransmissionConfig=false 동작을 계속 허용합니다.
// - WSA authority는 수정하지 않고 current resolved Wheel Radius를 read-only diagnostic input으로만 사용합니다.

#pragma once

#include "CFVehicleData.h"
#include "CFVehicleEngineCurveUtils.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFVehicleAIContract.h"
#include "DataAuthoring/CFVehicleProfileTypes.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "DataAuthoring/CFVehicleResolverTypes.h"
#include "Misc/SecureHash.h"

namespace CFBuilderTransUtil
{
	// 차량별 Transmission review에서 요구하는 stable semantic key입니다.
	inline const FName AutomaticGearsKey(TEXT("bUseAutomaticGears"));

	// 차량별 Transmission review에서 요구하는 stable semantic key입니다.
	inline const FName AutoReverseKey(TEXT("bUseAutoReverse"));

	// 차량별 Transmission review에서 요구하는 stable semantic key입니다.
	inline const FName ForwardRatiosKey(TEXT("ForwardGearRatios"));

	// 차량별 Transmission review에서 요구하는 stable semantic key입니다.
	inline const FName ReverseRatiosKey(TEXT("ReverseGearRatios"));

	// 차량별 Transmission review에서 요구하는 stable semantic key입니다.
	inline const FName FinalRatioKey(TEXT("FinalRatio"));

	// 차량별 Transmission review에서 요구하는 stable semantic key입니다.
	inline const FName ChangeUpRpmKey(TEXT("ChangeUpRPM"));

	// 차량별 Transmission review에서 요구하는 stable semantic key입니다.
	inline const FName ChangeDownRpmKey(TEXT("ChangeDownRPM"));

	// P0에서 BaselineInherited를 허용하는 auxiliary Transmission semantic key입니다.
	inline const FName GearChangeTimeKey(TEXT("GearChangeTime"));

	// P0에서 BaselineInherited를 허용하는 auxiliary Transmission semantic key입니다.
	inline const FName TransmissionEfficiencyKey(TEXT("TransmissionEfficiency"));

	// 실차가 automatic transmission인지 직접/파생 표현하는 research FactKey입니다.
	inline const FName AutomaticGearsFactKey(TEXT("Drivetrain.AutomaticGears"));

	// Transmission control 방식의 원본 research FactKey입니다.
	inline const FName TransmissionTypeFactKey(TEXT("Drivetrain.TransmissionType"));

	// Forward gear count research FactKey입니다.
	inline const FName GearCountFactKey(TEXT("Drivetrain.GearCount"));

	// Ordered forward gear ratio set research FactKey입니다.
	inline const FName GearRatiosFactKey(TEXT("Drivetrain.GearRatios"));

	// Reverse gear ratio set research FactKey입니다.
	inline const FName ReverseGearRatioFactKey(TEXT("Drivetrain.ReverseGearRatio"));

	// Final drive ratio research FactKey입니다.
	inline const FName FinalDriveRatioFactKey(TEXT("Drivetrain.FinalDriveRatio"));

	// Upshift condition/RPM research FactKey입니다.
	inline const FName UpshiftConditionFactKey(TEXT("Drivetrain.UpshiftCondition"));

	// Downshift condition/RPM research FactKey입니다.
	inline const FName DownshiftConditionFactKey(TEXT("Drivetrain.DownshiftCondition"));

	// Delimiter-safe deterministic payload token을 추가합니다.
	inline void AppendToken(FString& OutPayload, const TCHAR* Label, const FString& Value)
	{
		OutPayload += Label;
		OutPayload += TEXT(":");
		OutPayload += FString::FromInt(Value.Len());
		OutPayload += TEXT(":");
		OutPayload += Value;
		OutPayload += TEXT("\n");
	}

	// Canonical UTF-8 payload를 lowercase MD5 digest로 변환합니다.
	inline FString HashUtf8Payload(const FString& Payload)
	{
		// Platform-independent UTF-8 byte sequence입니다.
		const FTCHARToUTF8 Utf8Payload(*Payload);
		// Digest 누적 state입니다.
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());

		// 최종 MD5 byte 배열입니다.
		uint8 Digest[16];
		Md5.Final(Digest);

		// Lowercase hexadecimal 결과입니다.
		FString HexResult;
		HexResult.Reserve(32);
		// Stable lowercase nibble 표입니다.
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			HexResult.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			HexResult.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return HexResult;
	}

	// FName 목록을 lexical order로 복사합니다.
	inline TArray<FName> BuildSortedNames(const TArray<FName>& Names)
	{
		// Caller order와 무관한 deterministic copy입니다.
		TArray<FName> SortedNames = Names;
		SortedNames.Sort([](const FName Left, const FName Right)
		{
			return Left.LexicalLess(Right);
		});
		return SortedNames;
	}

	// FName 목록을 deterministic proposal payload에 추가합니다.
	inline void AppendNameArray(FString& OutPayload, const TCHAR* Label, const TArray<FName>& Names)
	{
		for (const FName Name : BuildSortedNames(Names))
		{
			AppendToken(OutPayload, Label, Name.ToString());
		}
	}

	// float 배열을 순서 보존 canonical payload에 추가합니다.
	inline void AppendFloatArray(FString& OutPayload, const TCHAR* Label, const TArray<float>& Values)
	{
		for (const float Value : Values)
		{
			AppendToken(OutPayload, Label, LexToString(Value));
		}
	}

	// Review component map을 key lexical order로 proposal payload에 추가합니다.
	inline void AppendMethodParameters(FString& OutPayload, const TMap<FName, FString>& Parameters)
	{
		// Deterministic map key 목록입니다.
		TArray<FName> ParameterKeys;
		Parameters.GetKeys(ParameterKeys);
		ParameterKeys.Sort([](const FName Left, const FName Right)
		{
			return Left.LexicalLess(Right);
		});

		for (const FName ParameterKey : ParameterKeys)
		{
			AppendToken(OutPayload, TEXT("MethodParameterKey"), ParameterKey.ToString());
			AppendToken(OutPayload, TEXT("MethodParameterValue"), Parameters.FindRef(ParameterKey));
		}
	}

	// Complete Drivetrain Transmission payload와 field-level review를 하나의 deterministic hash로 묶습니다.
	inline FString BuildTransmissionProposalHash(
		const FCFBuilderTransmissionReview& Review,
		const FCFDrivetrainProfileData& DrivetrainData)
	{
		if (!Review.IsPresent())
		{
			return FString();
		}

		// Hash가 반드시 binding해야 하는 complete Transmission payload입니다.
		FString Payload;
		AppendToken(Payload, TEXT("SchemaRevision"), FString::FromInt(Review.SchemaRevision));
		AppendToken(Payload, TEXT("ExpectedForwardGearCount"), FString::FromInt(Review.ExpectedForwardGearCount));
		AppendToken(Payload, TEXT("bUseTransmissionConfig"), DrivetrainData.bUseTransmissionConfig ? TEXT("1") : TEXT("0"));
		AppendToken(Payload, TEXT("bUseAutomaticGears"), DrivetrainData.bUseAutomaticGears ? TEXT("1") : TEXT("0"));
		AppendToken(Payload, TEXT("bUseAutoReverse"), DrivetrainData.bUseAutoReverse ? TEXT("1") : TEXT("0"));
		AppendFloatArray(Payload, TEXT("ForwardGearRatio"), DrivetrainData.TransmissionRatios.ForwardGearRatios);
		AppendFloatArray(Payload, TEXT("ReverseGearRatio"), DrivetrainData.TransmissionRatios.ReverseGearRatios);
		AppendToken(Payload, TEXT("FinalRatio"), LexToString(DrivetrainData.FinalRatio));
		AppendToken(Payload, TEXT("ChangeUpRPM"), LexToString(DrivetrainData.ChangeUpRPM));
		AppendToken(Payload, TEXT("ChangeDownRPM"), LexToString(DrivetrainData.ChangeDownRPM));
		AppendToken(Payload, TEXT("GearChangeTime"), LexToString(DrivetrainData.GearChangeTime));
		AppendToken(Payload, TEXT("TransmissionEfficiency"), LexToString(DrivetrainData.TransmissionEfficiency));

		// SemanticKey 순서와 caller JSON 배열 순서를 분리하는 deterministic component pointer 목록입니다.
		TArray<const FCFBuilderTransmissionComponentReview*> SortedComponents;
		SortedComponents.Reserve(Review.Components.Num());
		for (const FCFBuilderTransmissionComponentReview& Component : Review.Components)
		{
			SortedComponents.Add(&Component);
		}
		SortedComponents.Sort([](const FCFBuilderTransmissionComponentReview& Left, const FCFBuilderTransmissionComponentReview& Right)
		{
			return Left.SemanticKey.LexicalLess(Right.SemanticKey);
		});

		for (const FCFBuilderTransmissionComponentReview* Component : SortedComponents)
		{
			if (!Component)
			{
				continue;
			}
			AppendToken(Payload, TEXT("SemanticKey"), Component->SemanticKey.ToString());
			AppendToken(Payload, TEXT("Disposition"), FString::FromInt(static_cast<int32>(Component->Disposition)));
			AppendNameArray(Payload, TEXT("EvidenceClaimId"), Component->EvidenceClaimIds);
			AppendNameArray(Payload, TEXT("UnknownFactId"), Component->UnknownFactIds);
			AppendToken(Payload, TEXT("MethodId"), Component->MethodId.ToString());
			AppendToken(Payload, TEXT("MethodRevision"), FString::FromInt(Component->MethodRevision));
			AppendMethodParameters(Payload, Component->MethodParameters);
		}

		return HashUtf8Payload(Payload);
	}

	// Transmission Core semantic이 직접/파생 output으로 요구하는 exact research FactKey를 반환합니다.
	inline FName GetOutputFactKeyForSemantic(const FName SemanticKey)
	{
		if (SemanticKey == AutomaticGearsKey)
		{
			return AutomaticGearsFactKey;
		}
		if (SemanticKey == ForwardRatiosKey)
		{
			return GearRatiosFactKey;
		}
		if (SemanticKey == ReverseRatiosKey)
		{
			return ReverseGearRatioFactKey;
		}
		if (SemanticKey == FinalRatioKey)
		{
			return FinalDriveRatioFactKey;
		}
		if (SemanticKey == ChangeUpRpmKey)
		{
			return UpshiftConditionFactKey;
		}
		if (SemanticKey == ChangeDownRpmKey)
		{
			return DownshiftConditionFactKey;
		}
		return NAME_None;
	}

	// GAME_BIAS가 해당 Core semantic을 설계할 때 최소한 연결해야 하는 research FactKey인지 반환합니다.
	inline bool IsSupportingFactKeyForSemantic(const FName SemanticKey, const FName FactKey)
	{
		if (SemanticKey == AutomaticGearsKey || SemanticKey == AutoReverseKey)
		{
			return FactKey == AutomaticGearsFactKey || FactKey == TransmissionTypeFactKey;
		}
		return FactKey == GetOutputFactKeyForSemantic(SemanticKey);
	}

	// 비교 대상 규모에 비례하는 작은 tolerance로 proposal float와 Evidence 값을 비교합니다.
	inline bool IsTransmissionValueNearlyEqual(const float Left, const double Right)
	{
		// 두 값의 절대 규모 중 큰 값입니다.
		const double MaxMagnitude = FMath::Max(FMath::Abs(static_cast<double>(Left)), FMath::Abs(Right));
		// canonical numeric roundtrip 오차만 허용하는 상대 tolerance입니다.
		const double Tolerance = FMath::Max(1.0e-5, MaxMagnitude * 1.0e-5);
		return FMath::Abs(static_cast<double>(Left) - Right) <= Tolerance;
	}

	// CanonicalText comma-separated ratio list를 finite float 배열로 파싱합니다.
	inline bool ParseCanonicalRatioList(const FString& TextValue, TArray<float>& OutRatios)
	{
		OutRatios.Reset();
		// Comma 기준으로 분리한 canonical ratio token 목록입니다.
		TArray<FString> RatioTokens;
		TextValue.ParseIntoArray(RatioTokens, TEXT(","), true);
		if (RatioTokens.IsEmpty())
		{
			return false;
		}
		for (FString& RatioToken : RatioTokens)
		{
			RatioToken.TrimStartAndEndInline();
			// 현재 token에서 파싱한 ratio 값입니다.
			float RatioValue = 0.0f;
			if (!LexTryParseString(RatioValue, *RatioToken) || !FMath::IsFinite(RatioValue))
			{
				OutRatios.Reset();
				return false;
			}
			OutRatios.Add(RatioValue);
		}
		return true;
	}

	// Evidence Claim의 normalized 값이 current proposed Transmission field와 exact semantic으로 일치하는지 검사합니다.
	inline bool DoesClaimValueMatchPayload(
		const FName SemanticKey,
		const FCFRefClaim& Claim,
		const FCFDrivetrainProfileData& DrivetrainData)
	{
		if (SemanticKey == AutomaticGearsKey)
		{
			return Claim.ValueKind == ECFRefValueKind::Boolean
				&& Claim.BooleanValue == DrivetrainData.bUseAutomaticGears;
		}
		if (SemanticKey == ForwardRatiosKey || SemanticKey == ReverseRatiosKey)
		{
			// Current payload에서 비교할 ordered ratio set입니다.
			const TArray<float>& ExpectedRatios = SemanticKey == ForwardRatiosKey
				? DrivetrainData.TransmissionRatios.ForwardGearRatios
				: DrivetrainData.TransmissionRatios.ReverseGearRatios;
			// Evidence Claim에서 읽은 ordered ratio set입니다.
			TArray<float> EvidenceRatios;
			if (Claim.ValueKind == ECFRefValueKind::Number && ExpectedRatios.Num() == 1)
			{
				EvidenceRatios.Add(static_cast<float>(Claim.NumberValue));
			}
			else if (Claim.ValueKind != ECFRefValueKind::CanonicalText || !ParseCanonicalRatioList(Claim.TextValue, EvidenceRatios))
			{
				return false;
			}
			if (EvidenceRatios.Num() != ExpectedRatios.Num())
			{
				return false;
			}
			for (int32 RatioIndex = 0; RatioIndex < ExpectedRatios.Num(); ++RatioIndex)
			{
				if (!IsTransmissionValueNearlyEqual(ExpectedRatios[RatioIndex], EvidenceRatios[RatioIndex]))
				{
					return false;
				}
			}
			return true;
		}
		if (SemanticKey == FinalRatioKey || SemanticKey == ChangeUpRpmKey || SemanticKey == ChangeDownRpmKey)
		{
			if (Claim.ValueKind != ECFRefValueKind::Number)
			{
				return false;
			}
			// 비교할 current proposed scalar입니다.
			const float ExpectedValue = SemanticKey == FinalRatioKey
				? DrivetrainData.FinalRatio
				: (SemanticKey == ChangeUpRpmKey ? DrivetrainData.ChangeUpRPM : DrivetrainData.ChangeDownRPM);
			return IsTransmissionValueNearlyEqual(ExpectedValue, Claim.NumberValue);
		}
		return false;
	}

	// Ratio set이 non-empty, finite, positive인지 검사합니다.
	inline bool IsValidPositiveRatioSet(const TArray<float>& Ratios)
	{
		if (Ratios.IsEmpty())
		{
			return false;
		}
		for (const float Ratio : Ratios)
		{
			if (!FMath::IsFinite(Ratio) || Ratio <= KINDA_SMALL_NUMBER)
			{
				return false;
			}
		}
		return true;
	}

	// Current Evidence에서 exact canonical Claim을 찾습니다.
	inline const FCFRefClaim* FindCanonicalClaim(const UCFVehicleRefEvidence& Evidence, const FName ClaimId)
	{
		return Evidence.Claims.FindByPredicate([ClaimId](const FCFRefClaim& Claim)
		{
			return Claim.ClaimId == ClaimId && Claim.ResolutionState == ECFRefClaimResolution::Canonical;
		});
	}

	// Current Evidence에서 exact Unknown Fact를 찾습니다.
	inline const FCFRefUnknownFact* FindUnknownFact(const UCFVehicleRefEvidence& Evidence, const FName UnknownFactId)
	{
		return Evidence.UnknownFacts.FindByPredicate([UnknownFactId](const FCFRefUnknownFact& UnknownFact)
		{
			return UnknownFact.UnknownFactId == UnknownFactId;
		});
	}

	// Required semantic component가 정확히 하나 존재하는지 반환합니다.
	inline const FCFBuilderTransmissionComponentReview* FindUniqueComponent(
		const FCFBuilderTransmissionReview& Review,
		const FName SemanticKey,
		FString& OutError)
	{
		// 동일 semantic key 개수입니다.
		int32 MatchCount = 0;
		// 유일한 matching component입니다.
		const FCFBuilderTransmissionComponentReview* Match = nullptr;
		for (const FCFBuilderTransmissionComponentReview& Component : Review.Components)
		{
			if (Component.SemanticKey == SemanticKey)
			{
				++MatchCount;
				Match = &Component;
			}
		}

		if (MatchCount != 1)
		{
			OutError = FString::Printf(
				TEXT("Transmission.CoreProposalMissing: '%s' component는 정확히 1개 필요하지만 %d개입니다."),
				*SemanticKey.ToString(),
				MatchCount);
			return nullptr;
		}
		return Match;
	}

	// Component provenance가 current accepted Evidence/Unknown/consumed Claim set과 current payload semantic에 exact binding되는지 검사합니다.
	inline bool ValidateComponentProvenance(
		const FCFBuilderTransmissionComponentReview& Component,
		const FCFDrivetrainProfileData& DrivetrainData,
		const UCFVehicleRefEvidence& Evidence,
		const TSet<FName>& ConsumedClaimSet,
		FString& OutError)
	{
		if (Component.Disposition == ECFBuilderTransmissionDisposition::BaselineInherited)
		{
			OutError = FString::Printf(
				TEXT("Transmission.CoreProposalMissing: '%s'가 BaselineInherited 상태입니다."),
				*Component.SemanticKey.ToString());
			return false;
		}

		// GAME_BIAS가 target semantic과 실제로 관련된 Evidence/Unknown input을 하나 이상 가졌는지 여부입니다.
		bool bHasSemanticSupportingInput = false;
		// Component가 참조하는 Evidence Claim ID의 중복 검사용 set입니다.
		TSet<FName> ComponentClaimSet;
		for (const FName ClaimId : Component.EvidenceClaimIds)
		{
			if (ClaimId.IsNone() || ComponentClaimSet.Contains(ClaimId) || !ConsumedClaimSet.Contains(ClaimId))
			{
				OutError = FString::Printf(
					TEXT("Transmission.ProvenanceUnbound: '%s'의 EvidenceClaimIds가 current consumed canonical Claim set에 binding되지 않습니다."),
					*Component.SemanticKey.ToString());
				return false;
			}
			ComponentClaimSet.Add(ClaimId);

			// Current Evidence의 canonical Claim입니다.
			const FCFRefClaim* Claim = FindCanonicalClaim(Evidence, ClaimId);
			if (!Claim)
			{
				OutError = FString::Printf(
					TEXT("Transmission.ProvenanceUnbound: '%s'가 참조하는 Claim '%s'이 current Evidence에서 Canonical이 아닙니다."),
					*Component.SemanticKey.ToString(),
					*ClaimId.ToString());
				return false;
			}

			if (Component.Disposition == ECFBuilderTransmissionDisposition::EvidenceDirect
				&& Claim->Provenance != ECFRefProvenance::FACT)
			{
				OutError = FString::Printf(TEXT("Transmission.ProvenanceUnbound: '%s' EvidenceDirect는 FACT Claim만 사용할 수 있습니다."), *Component.SemanticKey.ToString());
				return false;
			}
			if (Component.Disposition == ECFBuilderTransmissionDisposition::EvidenceDerived
				&& Claim->Provenance != ECFRefProvenance::DERIVED)
			{
				OutError = FString::Printf(TEXT("Transmission.ProvenanceUnbound: '%s' EvidenceDerived는 DERIVED Claim만 사용할 수 있습니다."), *Component.SemanticKey.ToString());
				return false;
			}

			if (Component.Disposition == ECFBuilderTransmissionDisposition::EvidenceDirect
				|| Component.Disposition == ECFBuilderTransmissionDisposition::EvidenceDerived)
			{
				// Direct/Derived output이 반드시 가져야 하는 research semantic입니다.
				const FName ExpectedFactKey = GetOutputFactKeyForSemantic(Component.SemanticKey);
				if (ExpectedFactKey.IsNone() || Claim->FactKey != ExpectedFactKey)
				{
					OutError = FString::Printf(
						TEXT("Transmission.ProvenanceSemanticMismatch: '%s'는 FactKey '%s'로 증명할 수 없습니다."),
						*Component.SemanticKey.ToString(),
						*Claim->FactKey.ToString());
					return false;
				}
				if (!DoesClaimValueMatchPayload(Component.SemanticKey, *Claim, DrivetrainData))
				{
					OutError = FString::Printf(
						TEXT("Transmission.ProvenanceValueMismatch: '%s'의 Evidence 값과 proposed payload가 일치하지 않습니다."),
						*Component.SemanticKey.ToString());
					return false;
				}
			}

			if (Component.Disposition == ECFBuilderTransmissionDisposition::EvidenceDerived)
			{
				if (Claim->InputClaimIds.IsEmpty() || Claim->MethodId.IsNone() || Claim->MethodRevision <= 0
					|| Component.MethodId != Claim->MethodId || Component.MethodRevision != Claim->MethodRevision)
				{
					OutError = FString::Printf(
						TEXT("Transmission.ProvenanceUnbound: '%s' DERIVED review와 Claim의 Method/Input binding이 일치하지 않습니다."),
						*Component.SemanticKey.ToString());
					return false;
				}
				for (const FName InputClaimId : Claim->InputClaimIds)
				{
					// DERIVED output을 실제로 계산한 canonical FACT input입니다.
					const FCFRefClaim* InputClaim = FindCanonicalClaim(Evidence, InputClaimId);
					if (!InputClaim || InputClaim->Provenance != ECFRefProvenance::FACT || !ConsumedClaimSet.Contains(InputClaimId))
					{
						OutError = FString::Printf(
							TEXT("Transmission.ProvenanceUnbound: '%s' DERIVED input '%s'가 consumed canonical FACT가 아닙니다."),
							*Component.SemanticKey.ToString(),
							*InputClaimId.ToString());
						return false;
					}
				}
			}

			if (Component.Disposition == ECFBuilderTransmissionDisposition::GameBias
				&& IsSupportingFactKeyForSemantic(Component.SemanticKey, Claim->FactKey))
			{
				bHasSemanticSupportingInput = true;
			}
		}

		if ((Component.Disposition == ECFBuilderTransmissionDisposition::EvidenceDirect
				|| Component.Disposition == ECFBuilderTransmissionDisposition::EvidenceDerived)
			&& Component.EvidenceClaimIds.IsEmpty())
		{
			OutError = FString::Printf(TEXT("Transmission.ProvenanceUnbound: '%s'의 Evidence Claim 근거가 비어 있습니다."), *Component.SemanticKey.ToString());
			return false;
		}

		if (Component.Disposition == ECFBuilderTransmissionDisposition::GameBias)
		{
			if (Component.MethodId.IsNone() || Component.MethodRevision <= 0 || Component.MethodParameters.IsEmpty())
			{
				OutError = FString::Printf(TEXT("Transmission.ProvenanceUnbound: '%s' GAME_BIAS에는 MethodId/Revision/Parameters가 필요합니다."), *Component.SemanticKey.ToString());
				return false;
			}
			if (Component.EvidenceClaimIds.IsEmpty() && Component.UnknownFactIds.IsEmpty())
			{
				OutError = FString::Printf(TEXT("Transmission.ProvenanceUnbound: '%s' GAME_BIAS에는 Evidence Claim 또는 Unknown Fact 입력이 필요합니다."), *Component.SemanticKey.ToString());
				return false;
			}

			// Component가 참조하는 Unknown Fact ID의 중복 검사용 set입니다.
			TSet<FName> UnknownFactSet;
			for (const FName UnknownFactId : Component.UnknownFactIds)
			{
				// Current Evidence의 exact Unknown Fact입니다.
				const FCFRefUnknownFact* UnknownFact = FindUnknownFact(Evidence, UnknownFactId);
				if (UnknownFactId.IsNone() || UnknownFactSet.Contains(UnknownFactId) || !UnknownFact)
				{
					OutError = FString::Printf(
						TEXT("Transmission.ProvenanceUnbound: '%s'가 참조하는 Unknown Fact '%s'을 current Evidence에서 찾을 수 없습니다."),
						*Component.SemanticKey.ToString(),
						*UnknownFactId.ToString());
					return false;
				}
				UnknownFactSet.Add(UnknownFactId);
				if (IsSupportingFactKeyForSemantic(Component.SemanticKey, UnknownFact->FactKey))
				{
					bHasSemanticSupportingInput = true;
				}
			}
			if (!bHasSemanticSupportingInput)
			{
				OutError = FString::Printf(
					TEXT("Transmission.ProvenanceSemanticMismatch: '%s' GAME_BIAS input이 해당 Transmission semantic과 연결되지 않습니다."),
					*Component.SemanticKey.ToString());
				return false;
			}
		}

		OutError.Reset();
		return true;
	}

	// VehicleSpecificRequired Recipe의 complete Transmission Core review가 current Evidence와 payload를 만족하는지 검사합니다.
	inline bool ValidateTransmissionReview(
		const ECFBuilderTransmissionPolicy Policy,
		const FCFBuilderTransmissionReview& Review,
		const FCFDrivetrainProfileData& DrivetrainData,
		const UCFVehicleRefEvidence& Evidence,
		const TArray<FName>& ConsumedClaimIds,
		FString& OutError)
	{
		if (Policy == ECFBuilderTransmissionPolicy::LegacyCompatible)
		{
			OutError.Reset();
			return true;
		}

		if (!DrivetrainData.bUseTransmissionConfig)
		{
			OutError = TEXT("Transmission.VehicleSpecificRequired: 신규 Guided Vehicle은 bUseTransmissionConfig=true인 차량별 Transmission이 필요합니다.");
			return false;
		}
		if (!Review.IsPresent() || Review.SchemaRevision != 1)
		{
			OutError = TEXT("Transmission.ProvenanceUnbound: current field-level Transmission review가 없거나 지원하지 않는 revision입니다.");
			return false;
		}
		if (!DrivetrainData.bUseAutomaticGears || !DrivetrainData.bUseAutoReverse)
		{
			OutError = TEXT("Transmission.CoreProposalMissing: 현재 CarFight P0 Builder 조작계는 Automatic Gears + Auto Reverse proposal을 요구합니다.");
			return false;
		}
		if (!IsValidPositiveRatioSet(DrivetrainData.TransmissionRatios.ForwardGearRatios)
			|| !IsValidPositiveRatioSet(DrivetrainData.TransmissionRatios.ReverseGearRatios)
			|| !FMath::IsFinite(DrivetrainData.FinalRatio) || DrivetrainData.FinalRatio <= KINDA_SMALL_NUMBER)
		{
			OutError = TEXT("Transmission.InvalidRatioPayload: Forward/Reverse ratio set은 비어 있지 않은 positive finite 값이어야 하고 FinalRatio도 positive finite 값이어야 합니다.");
			return false;
		}
		if (!FMath::IsFinite(DrivetrainData.ChangeUpRPM) || !FMath::IsFinite(DrivetrainData.ChangeDownRPM)
			|| DrivetrainData.ChangeUpRPM <= 0.0f || DrivetrainData.ChangeDownRPM < 0.0f
			|| DrivetrainData.ChangeDownRPM >= DrivetrainData.ChangeUpRPM)
		{
			OutError = TEXT("Transmission.InvalidShiftRPM: Vehicle-specific Transmission은 finite ChangeUpRPM > 0, ChangeDownRPM >= 0, ChangeDownRPM < ChangeUpRPM이어야 합니다.");
			return false;
		}
		if (Review.ExpectedForwardGearCount <= 0)
		{
			OutError = TEXT("Transmission.GearCountConflict: Vehicle-specific Transmission은 reviewed ExpectedForwardGearCount가 필요합니다.");
			return false;
		}
		if (Review.ExpectedForwardGearCount != DrivetrainData.TransmissionRatios.ForwardGearRatios.Num())
		{
			OutError = FString::Printf(
				TEXT("Transmission.GearCountConflict: Evidence expected %d단과 proposed %d단이 일치하지 않습니다."),
				Review.ExpectedForwardGearCount,
				DrivetrainData.TransmissionRatios.ForwardGearRatios.Num());
			return false;
		}

		for (int32 GearIndex = 1; GearIndex < DrivetrainData.TransmissionRatios.ForwardGearRatios.Num(); ++GearIndex)
		{
			if (DrivetrainData.TransmissionRatios.ForwardGearRatios[GearIndex - 1]
				<= DrivetrainData.TransmissionRatios.ForwardGearRatios[GearIndex])
			{
				OutError = FString::Printf(
					TEXT("Transmission.RatioOrderInvalid: %d단→%d단 forward ratio가 낮아지는 순서를 만족하지 않습니다."),
					GearIndex,
					GearIndex + 1);
				return false;
			}
		}

		// Proposal 전체가 이미 commit receipt에 소비했다고 주장하는 canonical Claim set입니다.
		TSet<FName> ConsumedClaimSet;
		for (const FName ClaimId : ConsumedClaimIds)
		{
			if (!ClaimId.IsNone())
			{
				ConsumedClaimSet.Add(ClaimId);
			}
		}

		// Vehicle-specific completion에 필요한 7개 Core semantic key입니다.
		const FName RequiredCoreKeys[] =
		{
			AutomaticGearsKey,
			AutoReverseKey,
			ForwardRatiosKey,
			ReverseRatiosKey,
			FinalRatioKey,
			ChangeUpRpmKey,
			ChangeDownRpmKey
		};

		for (const FName RequiredKey : RequiredCoreKeys)
		{
			// Required semantic을 정확히 하나 소유하는 review component입니다.
			const FCFBuilderTransmissionComponentReview* Component = FindUniqueComponent(Review, RequiredKey, OutError);
			if (!Component || !ValidateComponentProvenance(*Component, DrivetrainData, Evidence, ConsumedClaimSet, OutError))
			{
				return false;
			}
		}

		// GAME_BIAS forward ratio set은 ratio 자체가 Unknown이어도 gear-count origin을 별도로 명시해야 합니다.
		FString ForwardRatioComponentError;
		const FCFBuilderTransmissionComponentReview* ForwardRatioComponent = FindUniqueComponent(Review, ForwardRatiosKey, ForwardRatioComponentError);
		if (ForwardRatioComponent && ForwardRatioComponent->Disposition == ECFBuilderTransmissionDisposition::GameBias)
		{
			// Forward ratio GAME_BIAS가 실제 GearCount FACT/Unknown을 참조하는지 여부입니다.
			bool bHasGearCountOrigin = false;
			for (const FName ClaimId : ForwardRatioComponent->EvidenceClaimIds)
			{
				// GearCount origin 후보 canonical Claim입니다.
				const FCFRefClaim* Claim = FindCanonicalClaim(Evidence, ClaimId);
				bHasGearCountOrigin = bHasGearCountOrigin || (Claim && Claim->FactKey == GearCountFactKey);
			}
			for (const FName UnknownFactId : ForwardRatioComponent->UnknownFactIds)
			{
				// GearCount origin 후보 Unknown Fact입니다.
				const FCFRefUnknownFact* UnknownFact = FindUnknownFact(Evidence, UnknownFactId);
				bHasGearCountOrigin = bHasGearCountOrigin || (UnknownFact && UnknownFact->FactKey == GearCountFactKey);
			}
			if (!bHasGearCountOrigin)
			{
				OutError = TEXT("Transmission.ProvenanceSemanticMismatch: GAME_BIAS ForwardGearRatios에는 Drivetrain.GearCount FACT/Unknown origin이 필요합니다.");
				return false;
			}
		}

		OutError.Reset();
		return true;
	}

	// ResolveResult의 exact canonical field에서 float 값을 읽습니다.
	inline bool ReadResolvedFloat(
		const FCFVehicleResolveResult& ResolveResult,
		const TCHAR* CanonicalPath,
		float& OutValue)
	{
		// 요청한 path의 effective resolved field입니다.
		const FCFVehicleResolvedField* Field = ResolveResult.SortedResolvedFields.FindByPredicate([CanonicalPath](const FCFVehicleResolvedField& Candidate)
		{
			return Candidate.FieldPath.ToCanonicalString(true) == CanonicalPath;
		});
		return Field && LexTryParseString(OutValue, *Field->Value.CanonicalValueText) && FMath::IsFinite(OutValue);
	}

	// Current drivetrain 구동축에 해당하는 resolved Wheel Radius의 min/max를 구합니다.
	inline bool ResolvePoweredWheelRadiusRange(
		const FCFDrivetrainProfileData& DrivetrainData,
		const FCFVehicleResolveResult& ResolveResult,
		float& OutMinRadiusCm,
		float& OutMaxRadiusCm)
	{
		// Current resolved front wheel radius입니다.
		float FrontRadiusCm = 0.0f;
		// Current resolved rear wheel radius입니다.
		float RearRadiusCm = 0.0f;
		// Front radius가 valid하게 관측됐는지 여부입니다.
		const bool bHasFrontRadius = ReadResolvedFloat(ResolveResult, TEXT("VehicleMovementConfig.FrontWheelRadius"), FrontRadiusCm) && FrontRadiusCm > 0.0f;
		// Rear radius가 valid하게 관측됐는지 여부입니다.
		const bool bHasRearRadius = ReadResolvedFloat(ResolveResult, TEXT("VehicleMovementConfig.RearWheelRadius"), RearRadiusCm) && RearRadiusCm > 0.0f;

		// Profile의 explicit powered-wheel flags입니다.
		bool bUseFrontRadius = DrivetrainData.bFrontWheelAffectedByEngine;
		// Profile의 explicit powered-wheel flags입니다.
		bool bUseRearRadius = DrivetrainData.bRearWheelAffectedByEngine;
		if (!bUseFrontRadius && !bUseRearRadius)
		{
			bUseFrontRadius = DrivetrainData.DifferentialType != EVehicleDifferential::RearWheelDrive;
			bUseRearRadius = DrivetrainData.DifferentialType != EVehicleDifferential::FrontWheelDrive;
		}

		// Powered axle에서 실제로 사용할 수 있는 radius 목록입니다.
		TArray<float> PoweredRadii;
		if (bUseFrontRadius && bHasFrontRadius)
		{
			PoweredRadii.Add(FrontRadiusCm);
		}
		if (bUseRearRadius && bHasRearRadius)
		{
			PoweredRadii.Add(RearRadiusCm);
		}
		if (PoweredRadii.IsEmpty())
		{
			return false;
		}

		OutMinRadiusCm = PoweredRadii[0];
		OutMaxRadiusCm = PoweredRadii[0];
		for (const float RadiusCm : PoweredRadii)
		{
			OutMinRadiusCm = FMath::Min(OutMinRadiusCm, RadiusCm);
			OutMaxRadiusCm = FMath::Max(OutMaxRadiusCm, RadiusCm);
		}
		return true;
	}

	// CarFight normalized Engine Torque Curve를 actual RPM에서 linear interpolation/clamp 평가합니다.
	inline float EvaluateEngineTorqueMultiplier(const FCFPerformanceProfileData& PerformanceData, const float EngineRpm)
	{
		const TArray<FCFVehicleEngineTorquePoint>& Points = PerformanceData.EngineTorqueCurve.Points;
		if (Points.IsEmpty())
		{
			return 0.0f;
		}
		if (EngineRpm <= Points[0].EngineRPM)
		{
			return Points[0].TorqueMultiplier;
		}
		if (EngineRpm >= Points.Last().EngineRPM)
		{
			return Points.Last().TorqueMultiplier;
		}

		for (int32 PointIndex = 0; PointIndex + 1 < Points.Num(); ++PointIndex)
		{
			const FCFVehicleEngineTorquePoint& Left = Points[PointIndex];
			const FCFVehicleEngineTorquePoint& Right = Points[PointIndex + 1];
			if (EngineRpm < Left.EngineRPM || EngineRpm > Right.EngineRPM)
			{
				continue;
			}
			const float Range = FMath::Max(Right.EngineRPM - Left.EngineRPM, KINDA_SMALL_NUMBER);
			const float Alpha = FMath::Clamp((EngineRpm - Left.EngineRPM) / Range, 0.0f, 1.0f);
			return FMath::Lerp(Left.TorqueMultiplier, Right.TorqueMultiplier, Alpha);
		}
		return Points.Last().TorqueMultiplier;
	}

	// Engine Curve + ratio/final/efficiency를 theoretical wheel torque Nm로 변환합니다.
	inline float CalculateWheelTorqueNm(
		const FCFPerformanceProfileData& PerformanceData,
		const FCFDrivetrainProfileData& DrivetrainData,
		const float EngineRpm,
		const float GearRatio)
	{
		const float EngineTorqueNm = PerformanceData.EngineMaxTorqueByFeel.NeutralValue
			* EvaluateEngineTorqueMultiplier(PerformanceData, EngineRpm);
		return EngineTorqueNm
			* GearRatio
			* DrivetrainData.FinalRatio
			* DrivetrainData.TransmissionEfficiency;
	}

	// 두 gear 선택 간 torque 차이를 더 큰 wheel torque 대비 0..1 relative gap으로 계산합니다.
	inline float CalculateRelativeTorqueGap(const float CurrentWheelTorqueNm, const float NextWheelTorqueNm)
	{
		const float LargerTorqueNm = FMath::Max(CurrentWheelTorqueNm, NextWheelTorqueNm);
		if (LargerTorqueNm <= KINDA_SMALL_NUMBER)
		{
			return 0.0f;
		}
		return FMath::Abs(CurrentWheelTorqueNm - NextWheelTorqueNm) / LargerTorqueNm;
	}

	// Vehicle-specific Engine Curve + gear ratios로 ideal pair crossover와 P0 single common ChangeUpRPM recommendation을 계산합니다.
	inline void BuildWheelTorqueShiftDiagnostic(
		const FCFPerformanceProfileData& PerformanceData,
		const FCFDrivetrainProfileData& DrivetrainData,
		const bool bHasWheelRadius,
		const float MinWheelRadiusCm,
		const float MaxWheelRadiusCm,
		FCFBuilderWheelTorqueShiftDiagnostic& OutDiagnostic)
	{
		OutDiagnostic = FCFBuilderWheelTorqueShiftDiagnostic();
		OutDiagnostic.bEvaluated = true;
		OutDiagnostic.MethodId = TEXT("WheelTorqueCrossoverShift");
		OutDiagnostic.MethodRevision = 1;

		if (!PerformanceData.bUseEngineTorqueCurve)
		{
			OutDiagnostic.Warnings.Add(TEXT("Transmission.EngineCurveUnavailable: vehicle-specific Engine TorqueCurve가 비활성화되어 Wheel-Torque crossover recommendation을 계산하지 않습니다."));
			return;
		}

		FString CurveValidationError;
		if (!FCFVehicleEngineCurveUtils::ValidateCurve(PerformanceData.EngineTorqueCurve, CurveValidationError))
		{
			OutDiagnostic.Blockers.Add(FString::Printf(TEXT("Transmission.EngineCurveInvalid: %s"), *CurveValidationError));
			return;
		}
		// Wheel-torque crossover 계산에 사용할 ordered forward gear ratio set입니다.
		const TArray<float>& Ratios = DrivetrainData.TransmissionRatios.ForwardGearRatios;
		if (Ratios.Num() < 2
			|| !IsValidPositiveRatioSet(Ratios)
			|| !FMath::IsFinite(DrivetrainData.FinalRatio)
			|| DrivetrainData.FinalRatio <= KINDA_SMALL_NUMBER
			|| !FMath::IsFinite(DrivetrainData.TransmissionEfficiency)
			|| DrivetrainData.TransmissionEfficiency <= KINDA_SMALL_NUMBER)
		{
			OutDiagnostic.Blockers.Add(TEXT("Transmission.WheelTorqueCrossoverUnavailable: 최소 2개의 positive finite forward gear와 positive finite FinalRatio/TransmissionEfficiency가 필요합니다."));
			return;
		}

		for (int32 GearIndex = 1; GearIndex < Ratios.Num(); ++GearIndex)
		{
			if (Ratios[GearIndex - 1] <= Ratios[GearIndex])
			{
				OutDiagnostic.Blockers.Add(FString::Printf(
					TEXT("Transmission.WheelTorqueRatioOrderInvalid: %d단→%d단 forward ratio가 엄격히 감소하지 않습니다."),
					GearIndex,
					GearIndex + 1));
				return;
			}
		}

		if (bHasWheelRadius
			&& (!FMath::IsFinite(MinWheelRadiusCm)
				|| !FMath::IsFinite(MaxWheelRadiusCm)
				|| MinWheelRadiusCm <= KINDA_SMALL_NUMBER
				|| MaxWheelRadiusCm < MinWheelRadiusCm))
		{
			OutDiagnostic.Blockers.Add(TEXT("Transmission.WheelTorqueRadiusInvalid: speed diagnostic을 요청한 경우 Wheel Radius 범위는 positive finite이며 Min<=Max여야 합니다."));
			return;
		}

		const float EngineMaxRpmFloat = PerformanceData.EngineMaxRPMByFeel.NeutralValue;
		if (!FMath::IsFinite(EngineMaxRpmFloat) || EngineMaxRpmFloat <= 0.0f)
		{
			OutDiagnostic.Blockers.Add(TEXT("Transmission.WheelTorqueCrossoverUnavailable: positive finite EngineMaxRPM Neutral이 필요합니다."));
			return;
		}

		OutDiagnostic.bVehicleSpecificEngineCurveAvailable = true;
		OutDiagnostic.SearchStartRPM = FMath::Max(
			0,
			FMath::CeilToInt(FMath::Max(PerformanceData.EngineIdleRPM, PerformanceData.EngineTorqueCurve.Points[0].EngineRPM)));
		// Clamp extrapolation을 피하기 위해 shift 탐색 authority로 사용할 마지막 curve RPM입니다.
		const float LastCurveRpm = PerformanceData.EngineTorqueCurve.Points.Last().EngineRPM;
		OutDiagnostic.SearchEndRPM = FMath::FloorToInt(FMath::Min(EngineMaxRpmFloat, LastCurveRpm));
		if (LastCurveRpm + 1.0f < EngineMaxRpmFloat)
		{
			OutDiagnostic.Warnings.Add(TEXT("Transmission.EngineCurveCoveragePartial: Engine Curve 마지막 point가 EngineMaxRPM보다 낮아 Wheel-Torque shift 탐색 상한을 마지막 curve RPM으로 제한합니다."));
		}
		if (OutDiagnostic.SearchEndRPM <= OutDiagnostic.SearchStartRPM)
		{
			OutDiagnostic.Blockers.Add(TEXT("Transmission.WheelTorqueCrossoverUnavailable: Engine RPM search range가 비어 있습니다."));
			return;
		}

		for (int32 GearIndex = 0; GearIndex + 1 < Ratios.Num(); ++GearIndex)
		{
			// Current search upper bound에서 다음 gear로 변속한 직후의 최대 가능한 RPM입니다.
			const float MaxPostShiftRpm = static_cast<float>(OutDiagnostic.SearchEndRPM) * Ratios[GearIndex + 1] / Ratios[GearIndex];
			if (!FMath::IsFinite(MaxPostShiftRpm) || MaxPostShiftRpm + KINDA_SMALL_NUMBER < static_cast<float>(OutDiagnostic.SearchStartRPM))
			{
				OutDiagnostic.Blockers.Add(FString::Printf(
					TEXT("Transmission.PostShiftRPMOutsideCurve: %d→%d는 탐색 상한에서도 post-shift RPM %.1f이 usable curve 시작 RPM %d보다 낮습니다."),
					GearIndex + 1,
					GearIndex + 2,
					MaxPostShiftRpm,
					OutDiagnostic.SearchStartRPM));
				return;
			}
		}
		for (int32 GearIndex = 0; GearIndex + 1 < Ratios.Num(); ++GearIndex)
		{
			const float CurrentGearRatio = Ratios[GearIndex];
			const float NextGearRatio = Ratios[GearIndex + 1];
			FCFBuilderWheelTorquePairDiagnostic& Pair = OutDiagnostic.Pairs.AddDefaulted_GetRef();
			Pair.FromGearNumber = GearIndex + 1;
			Pair.ToGearNumber = GearIndex + 2;

			int32 CrossoverRpm = OutDiagnostic.SearchEndRPM;
			bool bFoundCrossover = false;
			for (int32 CandidateRpm = OutDiagnostic.SearchStartRPM; CandidateRpm <= OutDiagnostic.SearchEndRPM; ++CandidateRpm)
			{
				const float PostShiftRpm = static_cast<float>(CandidateRpm) * NextGearRatio / CurrentGearRatio;
				if (PostShiftRpm + KINDA_SMALL_NUMBER < static_cast<float>(OutDiagnostic.SearchStartRPM))
				{
					continue;
				}
				const float CurrentWheelTorqueNm = CalculateWheelTorqueNm(PerformanceData, DrivetrainData, static_cast<float>(CandidateRpm), CurrentGearRatio);
				const float NextWheelTorqueNm = CalculateWheelTorqueNm(PerformanceData, DrivetrainData, PostShiftRpm, NextGearRatio);
				if (NextWheelTorqueNm + KINDA_SMALL_NUMBER >= CurrentWheelTorqueNm)
				{
					CrossoverRpm = CandidateRpm;
					bFoundCrossover = true;
					break;
				}
			}

			Pair.bCrossoverFound = bFoundCrossover;
			Pair.bLimitedByEngineMaxRPM = !bFoundCrossover;
			Pair.CrossoverRPM = static_cast<float>(CrossoverRpm);
			Pair.CrossoverPostShiftRPM = Pair.CrossoverRPM * NextGearRatio / CurrentGearRatio;
			Pair.CurrentWheelTorqueNm = CalculateWheelTorqueNm(PerformanceData, DrivetrainData, Pair.CrossoverRPM, CurrentGearRatio);
			Pair.NextWheelTorqueNm = CalculateWheelTorqueNm(PerformanceData, DrivetrainData, Pair.CrossoverPostShiftRPM, NextGearRatio);

			if (bHasWheelRadius)
			{
				Pair.bCrossoverSpeedAvailable = true;
				const float OverallRatio = CurrentGearRatio * DrivetrainData.FinalRatio;
				Pair.CrossoverSpeedMinKmh = Pair.CrossoverRPM * 2.0f * PI * MinWheelRadiusCm * 60.0f / (OverallRatio * 100000.0f);
				Pair.CrossoverSpeedMaxKmh = Pair.CrossoverRPM * 2.0f * PI * MaxWheelRadiusCm * 60.0f / (OverallRatio * 100000.0f);
			}
		}

		// P0 single threshold는 adjacent pair의 relative wheel-torque gap squared mean이 최소가 되는 integer RPM을 선택합니다.
		// 지금까지 찾은 공통 RPM 후보의 mean squared relative torque gap입니다.
		float BestMeanSquaredGap = TNumericLimits<float>::Max();
		// 지금까지 찾은 공통 RPM 후보의 최대 adjacent relative torque gap입니다.
		float BestMaxGap = TNumericLimits<float>::Max();
		// 현재 가장 좋은 단일 공통 ChangeUpRPM 후보입니다.
		int32 BestRpm = OutDiagnostic.SearchStartRPM;
		// 모든 gear pair에서 usable post-shift RPM을 만족하는 후보를 하나 이상 찾았는지 여부입니다.
		bool bFoundUsableCommonRpm = false;
		for (int32 CandidateRpm = OutDiagnostic.SearchStartRPM; CandidateRpm <= OutDiagnostic.SearchEndRPM; ++CandidateRpm)
		{
			// 현재 공통 RPM 후보의 adjacent relative torque gap 제곱 합입니다.
			float SumSquaredGap = 0.0f;
			// 현재 공통 RPM 후보에서 가장 큰 adjacent relative torque gap입니다.
			float MaxGap = 0.0f;
			// 현재 후보가 모든 gear pair에서 usable post-shift RPM을 유지하는지 여부입니다.
			bool bCandidateUsable = true;
			for (int32 GearIndex = 0; GearIndex + 1 < Ratios.Num(); ++GearIndex)
			{
				const float CurrentGearRatio = Ratios[GearIndex];
				const float NextGearRatio = Ratios[GearIndex + 1];
				const float PostShiftRpm = static_cast<float>(CandidateRpm) * NextGearRatio / CurrentGearRatio;
				if (PostShiftRpm + KINDA_SMALL_NUMBER < static_cast<float>(OutDiagnostic.SearchStartRPM))
				{
					bCandidateUsable = false;
					break;
				}
				const float CurrentWheelTorqueNm = CalculateWheelTorqueNm(PerformanceData, DrivetrainData, static_cast<float>(CandidateRpm), CurrentGearRatio);
				const float NextWheelTorqueNm = CalculateWheelTorqueNm(PerformanceData, DrivetrainData, PostShiftRpm, NextGearRatio);
				const float RelativeGap = CalculateRelativeTorqueGap(CurrentWheelTorqueNm, NextWheelTorqueNm);
				SumSquaredGap += RelativeGap * RelativeGap;
				MaxGap = FMath::Max(MaxGap, RelativeGap);
			}
			if (!bCandidateUsable)
			{
				continue;
			}

			const float MeanSquaredGap = SumSquaredGap / static_cast<float>(Ratios.Num() - 1);
			if (!bFoundUsableCommonRpm
				|| MeanSquaredGap + SMALL_NUMBER < BestMeanSquaredGap
				|| (FMath::IsNearlyEqual(MeanSquaredGap, BestMeanSquaredGap, SMALL_NUMBER) && CandidateRpm < BestRpm))
			{
				bFoundUsableCommonRpm = true;
				BestMeanSquaredGap = MeanSquaredGap;
				BestMaxGap = MaxGap;
				BestRpm = CandidateRpm;
			}
		}
		if (!bFoundUsableCommonRpm)
		{
			OutDiagnostic.Blockers.Add(TEXT("Transmission.PostShiftRPMOutsideCurve: 모든 adjacent gear에서 usable curve 범위를 유지하는 공통 ChangeUpRPM을 찾을 수 없습니다."));
			return;
		}

		OutDiagnostic.RecommendedChangeUpRPM = BestRpm;
		OutDiagnostic.MeanSquaredRelativeTorqueGap = BestMeanSquaredGap;
		OutDiagnostic.MaxRelativeTorqueGap = BestMaxGap;

		for (int32 PairIndex = 0; PairIndex < OutDiagnostic.Pairs.Num(); ++PairIndex)
		{
			FCFBuilderWheelTorquePairDiagnostic& Pair = OutDiagnostic.Pairs[PairIndex];
			const float CurrentGearRatio = Ratios[PairIndex];
			const float NextGearRatio = Ratios[PairIndex + 1];
			Pair.RecommendedPostShiftRPM = static_cast<float>(BestRpm) * NextGearRatio / CurrentGearRatio;
			Pair.RecommendedCurrentWheelTorqueNm = CalculateWheelTorqueNm(PerformanceData, DrivetrainData, static_cast<float>(BestRpm), CurrentGearRatio);
			Pair.RecommendedNextWheelTorqueNm = CalculateWheelTorqueNm(PerformanceData, DrivetrainData, Pair.RecommendedPostShiftRPM, NextGearRatio);
			Pair.RecommendedRelativeTorqueGap = CalculateRelativeTorqueGap(Pair.RecommendedCurrentWheelTorqueNm, Pair.RecommendedNextWheelTorqueNm);
			if (bHasWheelRadius)
			{
				Pair.bRecommendedSpeedAvailable = true;
				const float OverallRatio = CurrentGearRatio * DrivetrainData.FinalRatio;
				Pair.RecommendedSpeedMinKmh = static_cast<float>(BestRpm) * 2.0f * PI * MinWheelRadiusCm * 60.0f / (OverallRatio * 100000.0f);
				Pair.RecommendedSpeedMaxKmh = static_cast<float>(BestRpm) * 2.0f * PI * MaxWheelRadiusCm * 60.0f / (OverallRatio * 100000.0f);
			}
		}

		if (OutDiagnostic.Pairs.ContainsByPredicate([](const FCFBuilderWheelTorquePairDiagnostic& Pair)
		{
			return Pair.bLimitedByEngineMaxRPM;
		}))
		{
			OutDiagnostic.Warnings.Add(TEXT("Transmission.WheelTorqueCrossoverRedlineLimited: 하나 이상의 gear pair는 EngineMaxRPM 이내에 crossover가 없어 EngineMaxRPM을 pair shift limit으로 사용합니다."));
		}
		OutDiagnostic.Warnings.Add(TEXT("Transmission.FixedCommonShiftApproximation: P0 Runtime은 gear별 shift map이 아니라 하나의 ChangeUpRPM만 지원하므로 recommendation은 adjacent gear 전체의 least-squares compromise입니다."));
		// P0 single-threshold approximation에서 USER review를 강하게 요구할 최대 adjacent torque-gap 경고 기준입니다.
		constexpr float PoorFitRelativeTorqueGapWarning = 0.15f;
		if (OutDiagnostic.MaxRelativeTorqueGap > PoorFitRelativeTorqueGapWarning)
		{
			OutDiagnostic.Warnings.Add(FString::Printf(
				TEXT("Transmission.FixedCommonShiftFitPoor: 공통 ChangeUpRPM에서 최대 adjacent wheel-torque gap이 %.1f%%로 15%%를 초과합니다. 자동 채택하지 말고 GAME_BIAS USER Review가 필요합니다."),
				OutDiagnostic.MaxRelativeTorqueGap * 100.0f));
		}
	}

	// Vehicle-specific Transmission의 fixed ChangeUp/Down RPM kinematic sanity diagnostic을 계산합니다.
	inline void BuildTransmissionDiagnostic(
		const ECFBuilderTransmissionPolicy Policy,
		const FCFBuilderTransmissionReview& Review,
		const FString& TransmissionProposalHash,
		const FCFDrivetrainProfileData& DrivetrainData,
		const FCFPerformanceProfileData& PerformanceData,
		const FCFVehicleResolveResult& ResolveResult,
		FCFBuilderTransmissionDiagnostic& OutDiagnostic,
		const bool bEnableWheelTorqueShiftDiagnostic = false)
	{
		OutDiagnostic = FCFBuilderTransmissionDiagnostic();
		OutDiagnostic.bEvaluated = true;
		OutDiagnostic.bVehicleSpecificRequired = Policy == ECFBuilderTransmissionPolicy::VehicleSpecificRequired;
		OutDiagnostic.TransmissionProposalHash = TransmissionProposalHash;

		if (!DrivetrainData.bUseTransmissionConfig)
		{
			if (OutDiagnostic.bVehicleSpecificRequired)
			{
				OutDiagnostic.Blockers.Add(TEXT("Transmission.VehicleSpecificRequired: 차량별 Transmission config가 활성화되지 않았습니다."));
			}
			return;
		}

		// Shift-speed/EngineMaxRPM diagnostic이 신뢰할 수 있는 authoritative ResolveResult인지 여부입니다.
		const bool bResolveAuthorityAvailable = ResolveResult.ResolveStatus == ECFVehicleResolveStatus::Success;
		if (!bResolveAuthorityAvailable)
		{
			const FString AuthorityMessage = TEXT("Transmission.WheelRadiusAuthorityUnavailable: current Resolver가 Success가 아니므로 WSA-authoritative powered-wheel radius 기반 ShiftSpeed를 계산하지 않습니다.");
			if (OutDiagnostic.bVehicleSpecificRequired)
			{
				OutDiagnostic.Blockers.Add(AuthorityMessage);
			}
			else
			{
				OutDiagnostic.Warnings.Add(AuthorityMessage);
			}
		}

		// Current effective powered-wheel radius 최소값입니다.
		float MinWheelRadiusCm = 0.0f;
		// Current effective powered-wheel radius 최대값입니다.
		float MaxWheelRadiusCm = 0.0f;
		// WSA-owned current resolved radius로 speed diagnostic을 계산할 수 있는지 여부입니다. Blocked/Error Resolver의 fallback winner는 authority로 사용하지 않습니다.
		const bool bHasWheelRadius = bResolveAuthorityAvailable
			&& ResolvePoweredWheelRadiusRange(DrivetrainData, ResolveResult, MinWheelRadiusCm, MaxWheelRadiusCm);
		if (bResolveAuthorityAvailable && !bHasWheelRadius)
		{
			const FString RadiusMessage = TEXT("Transmission.WheelRadiusAuthorityUnavailable: successful Resolver에서 powered-wheel radius를 확인할 수 없습니다.");
			if (OutDiagnostic.bVehicleSpecificRequired)
			{
				OutDiagnostic.Blockers.Add(RadiusMessage);
			}
			else
			{
				OutDiagnostic.Warnings.Add(RadiusMessage);
			}
		}

		// Current effective EngineMaxRPM입니다.
		float EngineMaxRpm = 0.0f;
		// EngineMaxRPM이 authoritative resolved field에 존재하는지 여부입니다.
		const bool bHasEngineMaxRpm = bResolveAuthorityAvailable
			&& ReadResolvedFloat(ResolveResult, TEXT("VehicleMovementConfig.EngineMaxRPM"), EngineMaxRpm)
			&& EngineMaxRpm > 0.0f;
		if (bHasEngineMaxRpm && DrivetrainData.ChangeUpRPM > EngineMaxRpm)
		{
			OutDiagnostic.Blockers.Add(FString::Printf(
				TEXT("Transmission.ShiftThresholdUnreachable: ChangeUpRPM %.0f가 EngineMaxRPM %.0f보다 높습니다."),
				DrivetrainData.ChangeUpRPM,
				EngineMaxRpm));
		}
		else if (bHasEngineMaxRpm && DrivetrainData.ChangeUpRPM >= EngineMaxRpm * 0.98f)
		{
			OutDiagnostic.Warnings.Add(FString::Printf(
				TEXT("Transmission.ShiftThresholdNearLimit: ChangeUpRPM %.0f가 EngineMaxRPM %.0f에 매우 가깝습니다."),
				DrivetrainData.ChangeUpRPM,
				EngineMaxRpm));
		}

		// ESH-03는 Target DefinitionApply 이후 별도 Gate에서만 활성화합니다. pre-gate Step 5/Final Review는 기존 fixed-shift diagnostic만 유지합니다.
		if (bEnableWheelTorqueShiftDiagnostic)
		{
			BuildWheelTorqueShiftDiagnostic(
				PerformanceData,
				DrivetrainData,
				bHasWheelRadius,
				MinWheelRadiusCm,
				MaxWheelRadiusCm,
				OutDiagnostic.WheelTorqueShift);

			// Nested ESH-03 warning/blocker를 parent diagnostic으로 승격해 Step 5/Final Review의 기존 fail-closed 집계가 놓치지 않게 합니다.
			OutDiagnostic.Warnings.Append(OutDiagnostic.WheelTorqueShift.Warnings);
			OutDiagnostic.Blockers.Append(OutDiagnostic.WheelTorqueShift.Blockers);
		}

		if (Review.IsPresent())
		{
			// Auxiliary GearChangeTime provenance입니다.
			const FCFBuilderTransmissionComponentReview* GearTimeReview = Review.Components.FindByPredicate([](const FCFBuilderTransmissionComponentReview& Component)
			{
				return Component.SemanticKey == GearChangeTimeKey;
			});
			// Auxiliary TransmissionEfficiency provenance입니다.
			const FCFBuilderTransmissionComponentReview* EfficiencyReview = Review.Components.FindByPredicate([](const FCFBuilderTransmissionComponentReview& Component)
			{
				return Component.SemanticKey == TransmissionEfficiencyKey;
			});
			if (!GearTimeReview || GearTimeReview->Disposition == ECFBuilderTransmissionDisposition::BaselineInherited
				|| !EfficiencyReview || EfficiencyReview->Disposition == ECFBuilderTransmissionDisposition::BaselineInherited)
			{
				OutDiagnostic.Warnings.Add(TEXT("Transmission.AuxBaselineInherited: GearChangeTime 또는 TransmissionEfficiency가 compatibility/baseline 값을 유지합니다."));
			}

			if (Review.Components.ContainsByPredicate([](const FCFBuilderTransmissionComponentReview& Component)
			{
				return Component.Disposition == ECFBuilderTransmissionDisposition::GameBias;
			}))
			{
				OutDiagnostic.Warnings.Add(TEXT("Transmission.ReferenceFidelityPartial: 하나 이상의 Transmission 값이 USER-reviewed GAME_BIAS입니다."));
			}
		}

		for (int32 GearIndex = 0; GearIndex < DrivetrainData.TransmissionRatios.ForwardGearRatios.Num(); ++GearIndex)
		{
			// Current forward gear ratio입니다.
			const float GearRatio = DrivetrainData.TransmissionRatios.ForwardGearRatios[GearIndex];
			// USER-facing gear diagnostic row입니다.
			FCFBuilderTransmissionGearDiagnostic& GearDiagnostic = OutDiagnostic.Gears.AddDefaulted_GetRef();
			GearDiagnostic.GearNumber = GearIndex + 1;
			GearDiagnostic.GearRatio = GearRatio;
			GearDiagnostic.OverallRatio = GearRatio * DrivetrainData.FinalRatio;

			if (bHasWheelRadius && GearDiagnostic.OverallRatio > KINDA_SMALL_NUMBER)
			{
				GearDiagnostic.bShiftSpeedAvailable = true;
				// Smaller powered wheel produces the lower theoretical road speed.
				GearDiagnostic.ShiftSpeedMinKmh =
					DrivetrainData.ChangeUpRPM * 2.0f * PI * MinWheelRadiusCm * 60.0f
					/ (GearDiagnostic.OverallRatio * 100000.0f);
				// Larger powered wheel produces the higher theoretical road speed.
				GearDiagnostic.ShiftSpeedMaxKmh =
					DrivetrainData.ChangeUpRPM * 2.0f * PI * MaxWheelRadiusCm * 60.0f
					/ (GearDiagnostic.OverallRatio * 100000.0f);
			}

			if (GearIndex + 1 < DrivetrainData.TransmissionRatios.ForwardGearRatios.Num())
			{
				// Next forward gear ratio입니다.
				const float NextGearRatio = DrivetrainData.TransmissionRatios.ForwardGearRatios[GearIndex + 1];
				GearDiagnostic.bPostShiftAvailable = true;
				GearDiagnostic.PostShiftRPM = DrivetrainData.ChangeUpRPM * NextGearRatio / GearRatio;
				GearDiagnostic.RpmRetention = NextGearRatio / GearRatio;
				GearDiagnostic.DownshiftMarginRPM = GearDiagnostic.PostShiftRPM - DrivetrainData.ChangeDownRPM;

				if (GearDiagnostic.PostShiftRPM <= DrivetrainData.ChangeDownRPM)
				{
					OutDiagnostic.Blockers.Add(FString::Printf(
						TEXT("Transmission.PostShiftBelowDownRPM: %d→%d 변속 직후 %.0fRPM이 ChangeDownRPM %.0f 이하입니다."),
						GearIndex + 1,
						GearIndex + 2,
						GearDiagnostic.PostShiftRPM,
						DrivetrainData.ChangeDownRPM));
				}
				else
				{
					// Up/Down threshold band입니다.
					const float ShiftBandRpm = FMath::Max(1.0f, DrivetrainData.ChangeUpRPM - DrivetrainData.ChangeDownRPM);
					if (GearDiagnostic.DownshiftMarginRPM <= ShiftBandRpm * 0.1f)
					{
						OutDiagnostic.Warnings.Add(FString::Printf(
							TEXT("Transmission.PostShiftNearDownRPM: %d→%d 변속 후 Downshift margin이 %.0fRPM입니다."),
							GearIndex + 1,
							GearIndex + 2,
							GearDiagnostic.DownshiftMarginRPM));
					}
				}
			}
		}
	}
}
