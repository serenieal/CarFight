// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderWagonReviewCmdlet.h
// Version: v1.1.0
// Date: 2026-09-01
// Description: CF-FQ-040 actual Wagon Step 7 Final Review R0와 explicit USER-approved DefinitionApply R3를 하나의 bounded commandlet로 제공합니다.
// Changelog:
// - v1.1.0: review-only 설명을 current dual-mode R0 Review / exact approval-bound R3 Apply 계약에 맞게 교정.
// - v1.0.0: exact Wagon selection, fresh Final Review R0, Target/Recipe invariant, apply readiness/diff JSON export를 최초 구현.
// Migration:
// - 승인 hash가 없으면 Target/Recipe/Profile/Evidence를 수정하지 않는 R0 Review입니다.
// - 승인 hash가 있으면 commandlet cpp의 exact proposal/diff/pre/post guard를 모두 통과한 경우에만 existing Builder R3 Apply와 Target+Recipe save를 수행합니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFBuilderWagonReviewCmdlet.generated.h"

/** Actual Wagon의 fresh Step 7 Final Review를 mutation0으로 읽는 Editor-only commandlet입니다. */
UCLASS()
class CARFIGHT_REEDITOR_API UCFBuilderWagonReviewCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFBuilderWagonReviewCommandlet();

	// Current Builder authority에서 exact Wagon Final Review를 읽고 Saved diagnostic JSON만 기록합니다.
	virtual int32 Main(const FString& Params) override;
};
