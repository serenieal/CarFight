// Copyright (c) CarFight. All Rights Reserved.
// File: CFDADurableCore.cpp
// Version: v1.0.0
// Date: 2026-09-10
// Description: CF-FQ-051 DAO-P0-03 provider-neutral durable transaction non-template implementation입니다.
// Changelog:
// - v1.0.0: generic Create cleanup과 shared durable fault-injection authority를 추가.
// Migration:
// - test fault는 exact target path 기반이며 WITH_DEV_AUTOMATION_TESTS 외 production binary에서는 항상 비활성입니다.

#include "CFDADurableCore.h"

namespace CFDADurableCorePrivate
{
#if WITH_DEV_AUTOMATION_TESTS
	// SavePackage 호출 지점에서 uncertainty를 강제할 exact test target path입니다.
	FString GForceSaveFailureTargetPath;

	// SavePackage 성공 뒤 durable confirmation failure를 강제할 exact test target path입니다.
	FString GForceConfirmationFailureTargetPath;
#endif
}

// Create target의 pre-save failure에서 operation-created UObject/registry/dirty state를 best-effort로 정리합니다.
bool CFDADurableCore::CleanupCreatedAsset(
	UObject* Asset,
	UPackage* Package,
	const bool bRegistryNotified)
{
	if (Asset == nullptr || Package == nullptr)
	{
		return false;
	}

	// Create 이전에는 존재하지 않았어야 하는 exact package long name입니다.
	const FString CreatedPackageName = Package->GetName();
	if (bRegistryNotified)
	{
		FAssetRegistryModule::AssetDeleted(Asset);
	}
	Asset->ClearFlags(RF_Public | RF_Standalone);
	// Operation-created object를 transient package로 옮겨 original target path ownership을 해제한 결과입니다.
	const bool bRenamed = Asset->Rename(
		nullptr,
		GetTransientPackage(),
		REN_DontCreateRedirectors | REN_NonTransactional);
	Asset->MarkAsGarbage();
	Package->SetDirtyFlag(false);

	// Package UObject 자체가 memory에 남지 않아 original package absence가 복원됐는지 나타냅니다.
	const bool bPackageAbsenceRestored = FindPackage(nullptr, *CreatedPackageName) == nullptr;
	return bRenamed && !Package->IsDirty() && bPackageAbsenceRestored;
}

// 현재 exact target이 test-only forced SavePackage uncertainty 대상인지 확인합니다.
bool CFDADurableCore::ShouldForceSaveFailure(const FString& TargetObjectPath)
{
#if WITH_DEV_AUTOMATION_TESTS
	return !CFDADurableCorePrivate::GForceSaveFailureTargetPath.IsEmpty()
		&& CFDADurableCorePrivate::GForceSaveFailureTargetPath.Equals(TargetObjectPath, ESearchCase::CaseSensitive);
#else
	(void)TargetObjectPath;
	return false;
#endif
}

// 현재 exact target이 test-only forced post-save confirmation uncertainty 대상인지 확인합니다.
bool CFDADurableCore::ShouldForceConfirmationFailure(const FString& TargetObjectPath)
{
#if WITH_DEV_AUTOMATION_TESTS
	return !CFDADurableCorePrivate::GForceConfirmationFailureTargetPath.IsEmpty()
		&& CFDADurableCorePrivate::GForceConfirmationFailureTargetPath.Equals(TargetObjectPath, ESearchCase::CaseSensitive);
#else
	(void)TargetObjectPath;
	return false;
#endif
}

#if WITH_DEV_AUTOMATION_TESTS
// 모든 shared durable test fault state를 초기화합니다.
void CFDADurableCore::ResetTestFaults()
{
	CFDADurableCorePrivate::GForceSaveFailureTargetPath.Reset();
	CFDADurableCorePrivate::GForceConfirmationFailureTargetPath.Reset();
}

// exact target의 SavePackage 호출 지점에서 outcome-unknown failure를 강제합니다.
void CFDADurableCore::SetForceSaveFailureTarget(const FString& TargetObjectPath)
{
	CFDADurableCorePrivate::GForceSaveFailureTargetPath = TargetObjectPath;
}

// exact target의 SavePackage 성공 뒤 persisted confirmation uncertainty를 강제합니다.
void CFDADurableCore::SetForceConfirmationFailureTarget(const FString& TargetObjectPath)
{
	CFDADurableCorePrivate::GForceConfirmationFailureTargetPath = TargetObjectPath;
}
#endif
