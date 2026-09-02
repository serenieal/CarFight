// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderEngineUtil.h
// Version: v1.0.0
// Date: 2026-09-01
// Description: CF-FQ-040 ESH-02 vehicle-specific Engine Curve review/hash/fidelity diagnostic 공용 private utility입니다.
// Scope: Step 5 Profile preview와 Step 7 Final Review가 동일한 Engine Curve provenance 및 payload 계약을 사용합니다.
// Changelog:
// - v1.0.0: Baseline/FACT/DERIVED/GAME_BIAS review validation, deterministic proposal hash, sparse-anchor diagnostic을 최초 구현.
// Migration:
// - Runtime Engine Curve 값 owner는 ESH-01의 FCFPerformanceProfileData/FCFVehicleMovementConfig이며 이 utility는 Editor-only provenance만 소유합니다.
// - Existing Profile의 bUseEngineTorqueCurve=false + BaselineInherited review는 허용하고 Performance.EngineCurveFidelityPartial Warning으로 유지합니다.

#pragma once

#include "CFVehicleEngineCurveUtils.h"
#include "Containers/StringConv.h"
#include "DataAuthoring/CFVehicleAIContract.h"
#include "DataAuthoring/CFVehicleProfileTypes.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"
#include "Misc/SecureHash.h"

namespace CFBuilderEngineUtil
{
	// Current canonical research vocabulary의 최대 토크 값 FactKey입니다.
	inline const FName MaxTorqueFactKey(TEXT("Engine.MaxTorque"));

	// Actual Wagon historical Evidence가 사용한 최대 토크 값 legacy FactKey입니다.
	inline const FName LegacyMaxTorqueFactKey(TEXT("Engine.PeakTorqueNm"));

	// Current canonical research vocabulary의 최대 토크 RPM FactKey입니다.
	inline const FName MaxTorqueRpmFactKey(TEXT("Engine.MaxTorqueRpm"));

	// Current canonical research vocabulary의 최대 토크 band FactKey입니다.
	inline const FName TorqueBandFactKey(TEXT("Engine.TorqueBand"));

	// Actual Wagon historical Evidence가 사용한 최대 토크 band legacy FactKey입니다.
	inline const FName LegacyTorqueBandFactKey(TEXT("Engine.PeakTorqueRpmRange"));

	// Current canonical research vocabulary의 최대 출력 FactKey입니다.
	inline const FName MaxPowerFactKey(TEXT("Engine.MaxPower"));

	// Actual Wagon historical Evidence가 사용한 PS 단위 최대 출력 legacy FactKey입니다.
	inline const FName LegacyMaxPowerPsFactKey(TEXT("Engine.MaxPowerPs"));

	// Current canonical research vocabulary의 최대 출력 RPM FactKey입니다.
	inline const FName MaxPowerRpmFactKey(TEXT("Engine.MaxPowerRpm"));

	// Current canonical research vocabulary의 최대 출력 band FactKey입니다.
	inline const FName PowerBandFactKey(TEXT("Engine.PowerBand"));

	// Actual Wagon historical Evidence가 사용한 최대 출력 band legacy FactKey입니다.
	inline const FName LegacyPowerBandFactKey(TEXT("Engine.MaxPowerRpmRange"));

	// 실제 RPM별 torque/dyno point의 canonical research FactKey입니다.
	inline const FName TorqueCurvePointFactKey(TEXT("Engine.TorqueCurvePoint"));

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
		const FTCHARToUTF8 Utf8Payload(*Payload);
		FMD5 Md5;
		Md5.Update(reinterpret_cast<const uint8*>(Utf8Payload.Get()), Utf8Payload.Length());

		uint8 Digest[16];
		Md5.Final(Digest);

		FString HexResult;
		HexResult.Reserve(32);
		static constexpr TCHAR HexDigits[] = TEXT("0123456789abcdef");
		for (const uint8 ByteValue : Digest)
		{
			HexResult.AppendChar(HexDigits[(ByteValue >> 4) & 0x0F]);
			HexResult.AppendChar(HexDigits[ByteValue & 0x0F]);
		}
		return HexResult;
	}

	// FName 목록을 caller order와 무관한 deterministic lexical order로 반환합니다.
	inline TArray<FName> BuildSortedNames(const TArray<FName>& Names)
	{
		TArray<FName> SortedNames = Names;
		SortedNames.Sort([](const FName Left, const FName Right)
		{
			return Left.LexicalLess(Right);
		});
		return SortedNames;
	}

	// FName 목록을 deterministic hash payload에 추가합니다.
	inline void AppendNameArray(FString& OutPayload, const TCHAR* Label, const TArray<FName>& Names)
	{
		for (const FName Name : BuildSortedNames(Names))
		{
			AppendToken(OutPayload, Label, Name.ToString());
		}
	}

	// MethodParameters map을 key lexical order로 deterministic hash payload에 추가합니다.
	inline void AppendMethodParameters(FString& OutPayload, const TMap<FName, FString>& Parameters)
	{
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

	// FactKey가 Engine Curve proposal의 의미 있는 입력인지 반환합니다.
	inline bool IsEngineCurveSupportingFactKey(const FName FactKey)
	{
		return FactKey == MaxTorqueFactKey
			|| FactKey == LegacyMaxTorqueFactKey
			|| FactKey == MaxTorqueRpmFactKey
			|| FactKey == TorqueBandFactKey
			|| FactKey == LegacyTorqueBandFactKey
			|| FactKey == MaxPowerFactKey
			|| FactKey == LegacyMaxPowerPsFactKey
			|| FactKey == MaxPowerRpmFactKey
			|| FactKey == PowerBandFactKey
			|| FactKey == LegacyPowerBandFactKey
			|| FactKey == TorqueCurvePointFactKey;
	}

	// FactKey가 최대 토크 magnitude anchor인지 반환합니다.
	inline bool IsTorqueMagnitudeFactKey(const FName FactKey)
	{
		return FactKey == MaxTorqueFactKey || FactKey == LegacyMaxTorqueFactKey;
	}

	// FactKey가 최대 토크 RPM/band anchor인지 반환합니다.
	inline bool IsTorqueRpmFactKey(const FName FactKey)
	{
		return FactKey == MaxTorqueRpmFactKey
			|| FactKey == TorqueBandFactKey
			|| FactKey == LegacyTorqueBandFactKey;
	}

	// FactKey가 최대 출력 magnitude anchor인지 반환합니다.
	inline bool IsPowerMagnitudeFactKey(const FName FactKey)
	{
		return FactKey == MaxPowerFactKey || FactKey == LegacyMaxPowerPsFactKey;
	}

	// FactKey가 최대 출력 RPM/band anchor인지 반환합니다.
	inline bool IsPowerRpmFactKey(const FName FactKey)
	{
		return FactKey == MaxPowerRpmFactKey
			|| FactKey == PowerBandFactKey
			|| FactKey == LegacyPowerBandFactKey;
	}

	// CanonicalText TorqueCurvePoint의 "RPM=<number>;TorqueNm=<number>" 형식을 읽습니다.
	inline bool ParseDirectTorqueCurvePoint(const FString& TextValue, float& OutRpm, float& OutTorqueNm)
	{
		OutRpm = 0.0f;
		OutTorqueNm = 0.0f;

		FString RpmText;
		FString TorqueText;
		TArray<FString> Tokens;
		TextValue.ParseIntoArray(Tokens, TEXT(";"), true);
		for (const FString& Token : Tokens)
		{
			FString Key;
			FString Value;
			if (!Token.Split(TEXT("="), &Key, &Value))
			{
				continue;
			}
			Key.TrimStartAndEndInline();
			Value.TrimStartAndEndInline();
			if (Key.Equals(TEXT("RPM"), ESearchCase::IgnoreCase))
			{
				RpmText = Value;
			}
			else if (Key.Equals(TEXT("TorqueNm"), ESearchCase::IgnoreCase))
			{
				TorqueText = Value;
			}
		}

		if (RpmText.IsEmpty() || TorqueText.IsEmpty())
		{
			return false;
		}

		OutRpm = FCString::Atof(*RpmText);
		OutTorqueNm = FCString::Atof(*TorqueText);
		return FMath::IsFinite(OutRpm)
			&& OutRpm >= 0.0f
			&& FMath::IsFinite(OutTorqueNm)
			&& OutTorqueNm >= 0.0f;
	}

	// Reviewed Engine Curve metadata와 complete Performance payload를 하나의 deterministic hash로 묶습니다.
	inline FString BuildEngineCurveProposalHash(
		const FCFBuilderEngineCurveReview& Review,
		const FCFPerformanceProfileData& PerformanceData)
	{
		if (!PerformanceData.bUseEngineTorqueCurve || !Review.IsVehicleSpecificReview())
		{
			return FString();
		}

		FString Payload;
		AppendToken(Payload, TEXT("SchemaRevision"), FString::FromInt(Review.SchemaRevision));
		AppendToken(Payload, TEXT("Disposition"), FString::FromInt(static_cast<int32>(Review.Disposition)));
		AppendToken(Payload, TEXT("bUseEngineTorqueCurve"), PerformanceData.bUseEngineTorqueCurve ? TEXT("1") : TEXT("0"));
		AppendToken(Payload, TEXT("EngineMaxTorqueNeutral"), LexToString(PerformanceData.EngineMaxTorqueByFeel.NeutralValue));
		AppendToken(Payload, TEXT("EngineMaxRPMNeutral"), LexToString(PerformanceData.EngineMaxRPMByFeel.NeutralValue));
		AppendToken(Payload, TEXT("EngineIdleRPM"), LexToString(PerformanceData.EngineIdleRPM));

		// Curve point order는 RPM ordering 자체가 semantic이므로 배열 순서를 보존합니다.
		for (const FCFVehicleEngineTorquePoint& Point : PerformanceData.EngineTorqueCurve.Points)
		{
			AppendToken(Payload, TEXT("CurveRPM"), LexToString(Point.EngineRPM));
			AppendToken(Payload, TEXT("CurveTorqueMultiplier"), LexToString(Point.TorqueMultiplier));
		}

		AppendNameArray(Payload, TEXT("EvidenceClaimId"), Review.EvidenceClaimIds);
		AppendNameArray(Payload, TEXT("UnknownFactId"), Review.UnknownFactIds);
		AppendToken(Payload, TEXT("MethodId"), Review.MethodId.ToString());
		AppendToken(Payload, TEXT("MethodRevision"), FString::FromInt(Review.MethodRevision));
		AppendMethodParameters(Payload, Review.MethodParameters);
		return HashUtf8Payload(Payload);
	}

	// Direct FACT curve claim set이 proposed normalized Curve와 exact semantic으로 대응되는지 검사합니다.
	inline bool ValidateDirectCurvePoints(
		const FCFBuilderEngineCurveReview& Review,
		const FCFPerformanceProfileData& PerformanceData,
		const UCFVehicleRefEvidence& Evidence,
		FString& OutError)
	{
		const float MaxTorqueNm = PerformanceData.EngineMaxTorqueByFeel.NeutralValue;
		if (!FMath::IsFinite(MaxTorqueNm) || MaxTorqueNm <= KINDA_SMALL_NUMBER)
		{
			OutError = TEXT("Performance.EngineCurveValueMismatch: EvidenceDirect curve에는 positive EngineMaxTorque Neutral이 필요합니다.");
			return false;
		}

		TArray<FCFVehicleEngineTorquePoint> EvidencePoints;
		for (const FName ClaimId : Review.EvidenceClaimIds)
		{
			const FCFRefClaim* Claim = FindCanonicalClaim(Evidence, ClaimId);
			if (!Claim
				|| Claim->Provenance != ECFRefProvenance::FACT
				|| Claim->FactKey != TorqueCurvePointFactKey
				|| Claim->ValueKind != ECFRefValueKind::CanonicalText)
			{
				OutError = FString::Printf(
					TEXT("Performance.EngineCurveProvenanceUnbound: EvidenceDirect Claim '%s'은 canonical FACT Engine.TorqueCurvePoint CanonicalText여야 합니다."),
					*ClaimId.ToString());
				return false;
			}

			float PointRpm = 0.0f;
			float PointTorqueNm = 0.0f;
			if (!ParseDirectTorqueCurvePoint(Claim->TextValue, PointRpm, PointTorqueNm))
			{
				OutError = FString::Printf(
					TEXT("Performance.EngineCurveValueMismatch: Claim '%s'의 Engine.TorqueCurvePoint는 'RPM=<number>;TorqueNm=<number>' 형식이어야 합니다."),
					*ClaimId.ToString());
				return false;
			}

			FCFVehicleEngineTorquePoint& EvidencePoint = EvidencePoints.AddDefaulted_GetRef();
			EvidencePoint.EngineRPM = PointRpm;
			EvidencePoint.TorqueMultiplier = PointTorqueNm / MaxTorqueNm;
		}

		EvidencePoints.Sort([](const FCFVehicleEngineTorquePoint& Left, const FCFVehicleEngineTorquePoint& Right)
		{
			return Left.EngineRPM < Right.EngineRPM;
		});

		if (EvidencePoints.Num() != PerformanceData.EngineTorqueCurve.Points.Num())
		{
			OutError = TEXT("Performance.EngineCurveValueMismatch: EvidenceDirect curve point 수와 proposed curve point 수가 다릅니다.");
			return false;
		}

		constexpr float CurveValueTolerance = 0.0001f;
		for (int32 PointIndex = 0; PointIndex < EvidencePoints.Num(); ++PointIndex)
		{
			const FCFVehicleEngineTorquePoint& EvidencePoint = EvidencePoints[PointIndex];
			const FCFVehicleEngineTorquePoint& ProposedPoint = PerformanceData.EngineTorqueCurve.Points[PointIndex];
			if (!FMath::IsNearlyEqual(EvidencePoint.EngineRPM, ProposedPoint.EngineRPM, CurveValueTolerance)
				|| !FMath::IsNearlyEqual(EvidencePoint.TorqueMultiplier, ProposedPoint.TorqueMultiplier, CurveValueTolerance))
			{
				OutError = FString::Printf(
					TEXT("Performance.EngineCurveValueMismatch: EvidenceDirect curve와 proposed curve가 point %d에서 일치하지 않습니다."),
					PointIndex);
				return false;
			}
		}

		return true;
	}

	// Engine Curve review가 current Evidence/consumed Claim set과 complete Performance payload에 fresh binding되는지 검사합니다.
	inline bool ValidateEngineCurveReview(
		const FCFBuilderEngineCurveReview& Review,
		const FCFPerformanceProfileData& PerformanceData,
		const UCFVehicleRefEvidence& Evidence,
		const TArray<FName>& ConsumedClaimIds,
		FString& OutError)
	{
		if (!PerformanceData.bUseEngineTorqueCurve)
		{
			if (Review.IsVehicleSpecificReview())
			{
				OutError = TEXT("Performance.EngineCurveReviewMismatch: vehicle-specific review가 존재하지만 bUseEngineTorqueCurve=false입니다.");
				return false;
			}
			OutError.Reset();
			return true;
		}

		if (Review.SchemaRevision != 1)
		{
			OutError = FString::Printf(TEXT("Performance.EngineCurveReviewMismatch: 지원하지 않는 Engine Curve review schema revision입니다: %d"), Review.SchemaRevision);
			return false;
		}
		if (!Review.IsVehicleSpecificReview())
		{
			OutError = TEXT("Performance.EngineCurveFidelityPartial: bUseEngineTorqueCurve=true인데 reviewed Engine Curve provenance가 BaselineInherited입니다.");
			return false;
		}

		FString CurveValidationError;
		if (!FCFVehicleEngineCurveUtils::ValidateCurve(PerformanceData.EngineTorqueCurve, CurveValidationError))
		{
			OutError = FString::Printf(TEXT("Performance.EngineCurveInvalid: %s"), *CurveValidationError);
			return false;
		}

		const float MaxTorqueNm = PerformanceData.EngineMaxTorqueByFeel.NeutralValue;
		if (!FMath::IsFinite(MaxTorqueNm) || MaxTorqueNm <= KINDA_SMALL_NUMBER)
		{
			OutError = TEXT("Performance.EngineCurveInvalid: vehicle-specific curve에는 positive finite EngineMaxTorque Neutral이 필요합니다.");
			return false;
		}

		float PeakMultiplier = 0.0f;
		for (const FCFVehicleEngineTorquePoint& Point : PerformanceData.EngineTorqueCurve.Points)
		{
			PeakMultiplier = FMath::Max(PeakMultiplier, Point.TorqueMultiplier);
		}
		if (PeakMultiplier < 0.99f)
		{
			OutError = TEXT("Performance.EngineCurveValueMismatch: normalized curve peak가 1.0에 도달하지 않아 EngineMaxTorque scalar semantic과 맞지 않습니다.");
			return false;
		}

		const float EngineMaxRpm = PerformanceData.EngineMaxRPMByFeel.NeutralValue;
		if (FMath::IsFinite(EngineMaxRpm) && EngineMaxRpm > 0.0f
			&& PerformanceData.EngineTorqueCurve.Points.Last().EngineRPM > EngineMaxRpm + KINDA_SMALL_NUMBER)
		{
			OutError = TEXT("Performance.EngineCurveInvalid: 마지막 curve RPM이 EngineMaxRPM Neutral을 초과합니다.");
			return false;
		}

		TSet<FName> ConsumedClaimSet;
		for (const FName ClaimId : ConsumedClaimIds)
		{
			if (ClaimId.IsNone() || ConsumedClaimSet.Contains(ClaimId))
			{
				OutError = TEXT("Performance.EngineCurveProvenanceUnbound: consumed Claim ID 목록에 invalid/duplicate 항목이 있습니다.");
				return false;
			}
			ConsumedClaimSet.Add(ClaimId);
		}

		bool bHasTorqueMagnitudeAnchor = false;
		bool bHasTorqueRpmAnchor = false;
		bool bHasPowerMagnitudeAnchor = false;
		bool bHasPowerRpmAnchor = false;
		bool bHasEngineSupportingInput = false;
		TSet<FName> ReviewClaimSet;
		for (const FName ClaimId : Review.EvidenceClaimIds)
		{
			if (ClaimId.IsNone() || ReviewClaimSet.Contains(ClaimId) || !ConsumedClaimSet.Contains(ClaimId))
			{
				OutError = TEXT("Performance.EngineCurveProvenanceUnbound: Engine Curve EvidenceClaimIds가 current consumed canonical Claim set에 binding되지 않습니다.");
				return false;
			}
			ReviewClaimSet.Add(ClaimId);

			const FCFRefClaim* Claim = FindCanonicalClaim(Evidence, ClaimId);
			if (!Claim)
			{
				OutError = FString::Printf(TEXT("Performance.EngineCurveProvenanceUnbound: Claim '%s'이 current Evidence에서 Canonical이 아닙니다."), *ClaimId.ToString());
				return false;
			}
			if (!IsEngineCurveSupportingFactKey(Claim->FactKey))
			{
				OutError = FString::Printf(TEXT("Performance.EngineCurveSemanticMismatch: Claim '%s'의 FactKey '%s'는 Engine Curve 입력이 아닙니다."), *ClaimId.ToString(), *Claim->FactKey.ToString());
				return false;
			}
			if (Review.Disposition != ECFBuilderEngineCurveDisposition::EvidenceDirect
				&& Claim->Provenance == ECFRefProvenance::GAME_BIAS)
			{
				OutError = TEXT("Performance.EngineCurveProvenanceUnbound: DERIVED/GAME_BIAS Engine Curve review는 Evidence의 GAME_BIAS Claim을 기반 FACT anchor처럼 사용할 수 없습니다.");
				return false;
			}

			bHasEngineSupportingInput = true;
			bHasTorqueMagnitudeAnchor |= IsTorqueMagnitudeFactKey(Claim->FactKey);
			bHasTorqueRpmAnchor |= IsTorqueRpmFactKey(Claim->FactKey);
			bHasPowerMagnitudeAnchor |= IsPowerMagnitudeFactKey(Claim->FactKey);
			bHasPowerRpmAnchor |= IsPowerRpmFactKey(Claim->FactKey);
		}

		TSet<FName> ReviewUnknownSet;
		for (const FName UnknownFactId : Review.UnknownFactIds)
		{
			const FCFRefUnknownFact* UnknownFact = FindUnknownFact(Evidence, UnknownFactId);
			if (UnknownFactId.IsNone() || ReviewUnknownSet.Contains(UnknownFactId) || !UnknownFact)
			{
				OutError = FString::Printf(TEXT("Performance.EngineCurveProvenanceUnbound: Unknown Fact '%s'을 current Evidence에서 찾을 수 없습니다."), *UnknownFactId.ToString());
				return false;
			}
			ReviewUnknownSet.Add(UnknownFactId);
			if (IsEngineCurveSupportingFactKey(UnknownFact->FactKey))
			{
				bHasEngineSupportingInput = true;
			}
		}

		switch (Review.Disposition)
		{
		case ECFBuilderEngineCurveDisposition::EvidenceDirect:
			if (Review.EvidenceClaimIds.IsEmpty() || !Review.UnknownFactIds.IsEmpty()
				|| !Review.MethodId.IsNone() || Review.MethodRevision != 0 || !Review.MethodParameters.IsEmpty())
			{
				OutError = TEXT("Performance.EngineCurveProvenanceUnbound: EvidenceDirect curve는 direct FACT point만 사용하며 Method/Unknown metadata를 가질 수 없습니다.");
				return false;
			}
			if (!ValidateDirectCurvePoints(Review, PerformanceData, Evidence, OutError))
			{
				return false;
			}
			break;

		case ECFBuilderEngineCurveDisposition::EvidenceDerived:
			if (Review.MethodId.IsNone() || Review.MethodRevision <= 0 || Review.MethodParameters.IsEmpty())
			{
				OutError = TEXT("Performance.EngineCurveProvenanceUnbound: EvidenceDerived curve에는 MethodId/Revision/Parameters가 필요합니다.");
				return false;
			}
			if (!bHasTorqueMagnitudeAnchor || !bHasTorqueRpmAnchor || !bHasPowerMagnitudeAnchor || !bHasPowerRpmAnchor)
			{
				OutError = TEXT("Performance.EngineCurveAnchorInsufficient: DERIVED curve에는 Torque magnitude+RPM/band와 Power magnitude+RPM/band FACT anchor가 모두 필요합니다.");
				return false;
			}
			for (const FName ClaimId : Review.EvidenceClaimIds)
			{
				const FCFRefClaim* Claim = FindCanonicalClaim(Evidence, ClaimId);
				if (!Claim || Claim->Provenance != ECFRefProvenance::FACT)
				{
					OutError = TEXT("Performance.EngineCurveProvenanceUnbound: EvidenceDerived curve input은 current canonical FACT Claim이어야 합니다.");
					return false;
				}
			}
			break;

		case ECFBuilderEngineCurveDisposition::GameBias:
			if (Review.MethodId.IsNone() || Review.MethodRevision <= 0 || Review.MethodParameters.IsEmpty())
			{
				OutError = TEXT("Performance.EngineCurveProvenanceUnbound: GAME_BIAS curve에는 MethodId/Revision/Parameters가 필요합니다.");
				return false;
			}
			if (!bHasEngineSupportingInput)
			{
				OutError = TEXT("Performance.EngineCurveAnchorInsufficient: GAME_BIAS curve에도 최소 1개의 Engine Evidence/Unknown input이 필요합니다.");
				return false;
			}
			break;

		case ECFBuilderEngineCurveDisposition::BaselineInherited:
		default:
			OutError = TEXT("Performance.EngineCurveFidelityPartial: vehicle-specific curve가 enabled 상태인데 BaselineInherited review입니다.");
			return false;
		}

		OutError.Reset();
		return true;
	}

	// Current reviewed Engine Curve payload의 USER-facing warning/blocker projection을 만듭니다.
	inline void BuildEngineCurveDiagnostic(
		const FCFBuilderEngineCurveReview& Review,
		const FString& EngineCurveProposalHash,
		const FCFPerformanceProfileData& PerformanceData,
		FCFBuilderEngineCurveDiagnostic& OutDiagnostic)
	{
		OutDiagnostic = FCFBuilderEngineCurveDiagnostic();
		OutDiagnostic.bEvaluated = true;
		OutDiagnostic.bVehicleSpecificCurveEnabled = PerformanceData.bUseEngineTorqueCurve;
		OutDiagnostic.EngineCurveProposalHash = EngineCurveProposalHash;

		if (!PerformanceData.bUseEngineTorqueCurve)
		{
			OutDiagnostic.Warnings.Add(TEXT("Performance.EngineCurveFidelityPartial: MaxTorque/MaxRPM은 차량별이어도 Engine TorqueCurve가 BaselineInherited라 vehicle-specific Engine Complete가 아닙니다."));
			return;
		}

		if (EngineCurveProposalHash.IsEmpty() || !Review.IsVehicleSpecificReview())
		{
			OutDiagnostic.Blockers.Add(TEXT("Performance.EngineCurveProvenanceUnbound: vehicle-specific Engine Curve review/hash가 current payload에 binding되지 않았습니다."));
			return;
		}

		const float EngineIdleRpm = PerformanceData.EngineIdleRPM;
		const float FirstCurveRpm = PerformanceData.EngineTorqueCurve.Points.IsEmpty()
			? 0.0f
			: PerformanceData.EngineTorqueCurve.Points[0].EngineRPM;
		if (FMath::IsFinite(EngineIdleRpm) && EngineIdleRpm > 0.0f && FirstCurveRpm > EngineIdleRpm + 1.0f)
		{
			OutDiagnostic.Warnings.Add(TEXT("Performance.EngineCurveCoveragePartial: 첫 Engine Curve point가 EngineIdleRPM보다 높아 저회전 영역은 첫 key clamp에 의존합니다."));
		}

		const float EngineMaxRpm = PerformanceData.EngineMaxRPMByFeel.NeutralValue;
		const float LastCurveRpm = PerformanceData.EngineTorqueCurve.Points.IsEmpty()
			? 0.0f
			: PerformanceData.EngineTorqueCurve.Points.Last().EngineRPM;
		if (FMath::IsFinite(EngineMaxRpm) && EngineMaxRpm > 0.0f && LastCurveRpm + 1.0f < EngineMaxRpm)
		{
			OutDiagnostic.Warnings.Add(TEXT("Performance.EngineCurveCoveragePartial: 마지막 Engine Curve point가 EngineMaxRPM보다 낮아 고회전 tail은 마지막 key clamp에 의존합니다."));
		}

		if (Review.Disposition == ECFBuilderEngineCurveDisposition::GameBias)
		{
			OutDiagnostic.Warnings.Add(TEXT("Performance.EngineCurveGameBias: 현재 Engine Curve는 실차 원본 Curve FACT가 아니라 USER review가 필요한 CarFight 근사 Curve입니다."));
		}
		else if (Review.Disposition == ECFBuilderEngineCurveDisposition::EvidenceDerived)
		{
			OutDiagnostic.Warnings.Add(TEXT("Performance.EngineCurveDerived: 현재 Engine Curve는 sparse FACT anchor를 만족하도록 계산한 DERIVED 근사치이며 제조사 원본 Curve가 아닙니다."));
		}
	}
}
