// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFBuilderWagonEvidenceCmdlet.h
// Version: v1.0.0
// Date: 2026-09-01
// Description: CF-FQ-040 actual Wagon의 canonical Saved ResearchDraft를 existing Reference Evidence Refresh R1 경로로 persistent 적용하는 bounded one-shot commandlet입니다.
// Changelog:
// - v1.0.0: actual Wagon ResearchDraft load, mutation0 preview, explicit AuthoringWrite commit, Evidence-only save/rollback, Recipe/Target/Profile invariant 검증을 최초 구현.
// Migration:
// - raw Evidence field write를 하지 않고 FCFVehicleAuthoringService::Preview/CommitBuilderEvidenceRefresh를 그대로 재사용합니다.
// - exact existing DA_Ref_Wagon 한 package만 저장하며 Recipe/Target/Profile은 저장하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "CFBuilderWagonEvidenceCmdlet.generated.h"

/** Actual Wagon canonical ResearchDraft를 existing Reference Evidence에 안전하게 refresh하는 Editor-only one-shot commandlet입니다. */
UCLASS()
class CARFIGHT_REEDITOR_API UCFBuilderWagonEvidenceCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	// Headless Editor commandlet 실행 속성을 준비합니다.
	UCFBuilderWagonEvidenceCommandlet();

	// Canonical Saved ResearchDraft를 reviewed R1 Evidence Refresh로 commit하고 Evidence package 하나만 저장합니다.
	virtual int32 Main(const FString& Params) override;
};
