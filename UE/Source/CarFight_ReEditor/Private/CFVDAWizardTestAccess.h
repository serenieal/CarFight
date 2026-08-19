// Copyright (c) CarFight. All Rights Reserved.
//
// File: CFVDAWizardTestAccess.h
// Version: v1.0.0
// Date: 2026-08-18
// Description: DAUTH-P0-10 Automation에서 legacy Wizard managed-target guard를 production API 확장 없이 검증하는 test-only access입니다.
// Changelog:
// - v1.0.0: Target 주입, managed Recipe 조회, legacy write-button enable state read helper 최초 구현.
// Migration:
// - WITH_DEV_AUTOMATION_TESTS 전용이며 shipping/editor production API를 추가하지 않습니다.

#pragma once

#include "CFVDAWizardTab.h"

#if WITH_DEV_AUTOMATION_TESTS

class UCFVehicleData;

/** Legacy Wizard private guard를 P0-10 Automation에서만 읽는 test-only bridge입니다. */
class FCFVDAWizardTestAccess
{
public:
	// Test fixture Target을 legacy Wizard current weak target에 연결합니다.
	static void SetTarget(SCFVDAWizardTab& Wizard, UCFVehicleData* Target)
	{
		Wizard.TargetVehicleData = Target;
	}

	// Common Authoring facade 기반 managed Recipe guard 결과를 반환합니다.
	static bool HasManagedRecipe(const SCFVDAWizardTab& Wizard)
	{
		return Wizard.HasManagedAuthoringRecipe();
	}

	// Legacy Layout 변경 버튼 enable 조건을 반환합니다.
	static bool CanCaptureLayout(const SCFVDAWizardTab& Wizard)
	{
		return Wizard.CanCaptureLayout();
	}

	// Legacy Driving Feel 변경 버튼 enable 조건을 반환합니다.
	static bool CanApplyDrivingFeel(const SCFVDAWizardTab& Wizard)
	{
		return Wizard.CanApplyDrivingFeel();
	}

	// Legacy memory Revert 버튼 enable 조건을 반환합니다.
	static bool CanRevertDrivingFeel(const SCFVDAWizardTab& Wizard)
	{
		return Wizard.CanRevertDrivingFeel();
	}
};

#endif
