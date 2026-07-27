// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.1
// Date: 2026-07-24
// Description: TS-P0-07 선택 대상 장비 조회, 이동 중 사거리 갱신과 직접 조준 비침범 자동화 테스트

#include "CFEquipmentPresetData.h"
#include "CFTargetSelectComp.h"
#include "CFTargetSelectContractTestTypes.h"
#include "CFVehicleData.h"
#include "CFVehiclePawn.h"
#include "CFVehicleWeaponComp.h"
#include "CFWeaponData.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Tests/AutomationEditorCommon.h"

namespace
{
	ACFTargetSelectContractActor* SpawnEquipmentTarget(
		UWorld* TestWorld,
		const FName ActorName,
		const FVector& ActorLocation,
		const ECFTargetCategory TargetCategory,
		const ECFTargetRelation TargetRelation,
		const TArray<FName>& AttributeTags)
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
			TargetActor->DisplayInfo.TargetCategory = TargetCategory;
			TargetActor->DisplayInfo.Relation = TargetRelation;
			TargetActor->DisplayInfo.InformationLevel = ECFTargetInfoLevel::Identified;
			TargetActor->DisplayInfo.AttributeTags = AttributeTags;
			TargetActor->TrackState = ECFTargetTrackState::Visible;
		}
		return TargetActor;
	}

	void ConfigureEquipmentRuntimeData(
		UCFVehicleData* VehicleData,
		UCFEquipmentPresetData* EquipmentPresetData,
		UCFWeaponData* WeaponData)
	{
		if (!VehicleData || !EquipmentPresetData || !WeaponData)
		{
			return;
		}

		WeaponData->WeaponId = TEXT("TS_P0_07_TargetWeapon");
		WeaponData->WeaponSize = ECFVehicleWeaponSize::Large;
		WeaponData->CompatibleMountTypes.Reset();
		WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Turret);
		WeaponData->MaxRange = 5000.0f;
		WeaponData->TargetUsePolicy.AllowedCategories = {ECFTargetCategory::Vehicle};
		WeaponData->TargetUsePolicy.AllowedRelations = {ECFTargetRelation::Hostile};
		WeaponData->TargetUsePolicy.RequiredAttributeTags = {TEXT("Lockable")};
		WeaponData->TargetUsePolicy.ExcludedAttributeTags = {TEXT("Jammed")};
		WeaponData->TargetUsePolicy.AllowedTrackStates = {ECFTargetTrackState::Visible};

		EquipmentPresetData->EquipmentId = TEXT("TS_P0_07_WeaponPreset");
		EquipmentPresetData->RequiredMountType = ECFVehicleMountType::Turret;
		EquipmentPresetData->RequiredWeaponSize = ECFVehicleWeaponSize::Large;
		EquipmentPresetData->DefaultWeaponData = WeaponData;

		FCFVehicleHardpointSlot HardpointSlot;
		HardpointSlot.LocationSlotId = TEXT("Top_01");
		HardpointSlot.LocationCategory = TEXT("Top");
		HardpointSlot.LocalLocation = FVector(0.0f, 0.0f, 100.0f);
		VehicleData->HardpointSlots = {HardpointSlot};

		FCFVehicleMountProfile MountProfile;
		MountProfile.MountProfileId = TEXT("RoofTurret_MediumOrLarge");
		MountProfile.LocationSlotRef = HardpointSlot.LocationSlotId;
		MountProfile.MountType = ECFVehicleMountType::Turret;
		MountProfile.SizeLimit = ECFVehicleWeaponSize::Large;
		MountProfile.DefaultEquipmentPresetData = EquipmentPresetData;
		VehicleData->MountProfiles = {MountProfile};
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FCFTargetEquipmentQueryTest,
	"CarFight.TargetSelect.TS_P0_07.EquipmentQuery",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFTargetEquipmentQueryTest::RunTest(const FString& Parameters)
{
	(void)Parameters;

	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull(TEXT("장비 조회 테스트 월드 생성"), TestWorld);
	if (!TestWorld)
	{
		return false;
	}

	AddExpectedError(
		TEXT("VehicleVisualHitCollision: SM_Body component missing on TargetUseVehiclePawn."),
		EAutomationExpectedErrorFlags::Contains,
		1);

	FActorSpawnParameters PawnSpawnParameters;
	PawnSpawnParameters.Name = TEXT("TargetUseVehiclePawn");
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(
		ACFVehiclePawn::StaticClass(),
		FVector::ZeroVector,
		FRotator::ZeroRotator,
		PawnSpawnParameters);
	TestNotNull(TEXT("장비 조회 차량 Pawn 생성"), VehiclePawn);
	if (!VehiclePawn)
	{
		return false;
	}

	UCFTargetSelectComp* TargetSelectComp = VehiclePawn->GetTargetSelectComp();
	UCFVehicleWeaponComp* VehicleWeaponComp = VehiclePawn->GetVehicleWeaponComp();
	TestNotNull(TEXT("TargetSelectComp 존재"), TargetSelectComp);
	TestNotNull(TEXT("VehicleWeaponComp 존재"), VehicleWeaponComp);
	if (!TargetSelectComp || !VehicleWeaponComp)
	{
		return false;
	}
	TargetSelectComp->bAutoRefreshCandidate = false;

	UCFVehicleData* VehicleData = NewObject<UCFVehicleData>(GetTransientPackage());
	UCFEquipmentPresetData* EquipmentPresetData = NewObject<UCFEquipmentPresetData>(GetTransientPackage());
	UCFWeaponData* WeaponData = NewObject<UCFWeaponData>(GetTransientPackage());
	TestNotNull(TEXT("테스트 VehicleData 생성"), VehicleData);
	TestNotNull(TEXT("테스트 EquipmentPresetData 생성"), EquipmentPresetData);
	TestNotNull(TEXT("테스트 WeaponData 생성"), WeaponData);
	if (!VehicleData || !EquipmentPresetData || !WeaponData)
	{
		return false;
	}
	ConfigureEquipmentRuntimeData(VehicleData, EquipmentPresetData, WeaponData);
	TestTrue(TEXT("활성 무기 런타임 초기화"), VehicleWeaponComp->InitializeWeaponRuntime(VehiclePawn, VehicleData));

	FCFTargetUseResult TargetUseResult = VehicleWeaponComp->GetLastActiveWeaponTargetUseResult();
	TestEqual(TEXT("선택 없음 실패 사유"), TargetUseResult.FailureReason, ECFTargetUseFailureReason::NoSelectedTarget);
	TestFalse(TEXT("선택 없음 사용 불가"), TargetUseResult.bCanUseTarget);

	const TArray<FName> LockableTags = {TEXT("Lockable")};
	ACFTargetSelectContractActor* CompatibleTarget = SpawnEquipmentTarget(
		TestWorld, TEXT("CompatibleTarget"), FVector(3000.0f, 0.0f, 0.0f),
		ECFTargetCategory::Vehicle, ECFTargetRelation::Hostile, LockableTags);
	ACFTargetSelectContractActor* FriendlyTarget = SpawnEquipmentTarget(
		TestWorld, TEXT("FriendlyTarget"), FVector(2500.0f, 0.0f, 0.0f),
		ECFTargetCategory::Vehicle, ECFTargetRelation::Friendly, LockableTags);
	ACFTargetSelectContractActor* DeviceTarget = SpawnEquipmentTarget(
		TestWorld, TEXT("DeviceTarget"), FVector(2500.0f, 0.0f, 0.0f),
		ECFTargetCategory::Device, ECFTargetRelation::Hostile, LockableTags);
	ACFTargetSelectContractActor* MissingTagTarget = SpawnEquipmentTarget(
		TestWorld, TEXT("MissingTagTarget"), FVector(2500.0f, 0.0f, 0.0f),
		ECFTargetCategory::Vehicle, ECFTargetRelation::Hostile, {});
	ACFTargetSelectContractActor* ExcludedTagTarget = SpawnEquipmentTarget(
		TestWorld, TEXT("ExcludedTagTarget"), FVector(2500.0f, 0.0f, 0.0f),
		ECFTargetCategory::Vehicle, ECFTargetRelation::Hostile, {TEXT("Lockable"), TEXT("Jammed")});
	ACFTargetSelectContractActor* FarTarget = SpawnEquipmentTarget(
		TestWorld, TEXT("FarTarget"), FVector(7000.0f, 0.0f, 0.0f),
		ECFTargetCategory::Vehicle, ECFTargetRelation::Hostile, LockableTags);
	ACFTargetSelectContractActor* DestroyedTarget = SpawnEquipmentTarget(
		TestWorld, TEXT("DestroyedTarget"), FVector(2000.0f, 0.0f, 0.0f),
		ECFTargetCategory::Vehicle, ECFTargetRelation::Hostile, LockableTags);
	TestNotNull(TEXT("호환 대상 생성"), CompatibleTarget);
	TestNotNull(TEXT("아군 대상 생성"), FriendlyTarget);
	TestNotNull(TEXT("장치 대상 생성"), DeviceTarget);
	TestNotNull(TEXT("필수 태그 누락 대상 생성"), MissingTagTarget);
	TestNotNull(TEXT("제외 태그 대상 생성"), ExcludedTagTarget);
	TestNotNull(TEXT("거리 초과 대상 생성"), FarTarget);
	TestNotNull(TEXT("파괴 검증 대상 생성"), DestroyedTarget);
	if (!CompatibleTarget || !FriendlyTarget || !DeviceTarget || !MissingTagTarget || !ExcludedTagTarget || !FarTarget || !DestroyedTarget)
	{
		return false;
	}

	const FCFTargetSelectionContext SelectionContext = TargetSelectComp->GetDefaultSelectionContext();

	FCFVehicleFireOrigin DirectAimFireOrigin;
	DirectAimFireOrigin.bResolved = true;
	DirectAimFireOrigin.WorldFireLocation = FVector(125.0f, 25.0f, 80.0f);
	DirectAimFireOrigin.WorldFireDirection = FVector(0.25f, 0.95f, 0.15f).GetSafeNormal();
	VehicleWeaponComp->RecordResolvedFireOrigin(DirectAimFireOrigin, TEXT("TS-P0-07 DirectAimGuard"));

	TestTrue(TEXT("호환 대상 선택"), TargetSelectComp->SetSelectedTarget(CompatibleTarget, SelectionContext));
	TargetUseResult = VehicleWeaponComp->GetLastActiveWeaponTargetUseResult();
	TestEqual(TEXT("선택 변경 이벤트로 캐시 대상 갱신"), TargetUseResult.TargetActor.Get(), static_cast<AActor*>(CompatibleTarget));
	TestTrue(TEXT("호환 대상 사용 가능"), TargetUseResult.bCanUseTarget);
	TestTrue(TEXT("호환 대상 선택 유효"), TargetUseResult.bSelectedTargetValid);
	TestTrue(TEXT("호환 대상 정책 통과"), TargetUseResult.bTargetCompatible);
	TestTrue(TEXT("호환 대상 거리 통과"), TargetUseResult.bWithinUseDistance);
	TestEqual(TEXT("호환 대상 실패 사유 없음"), TargetUseResult.FailureReason, ECFTargetUseFailureReason::None);
	TestTrue(TEXT("호환 대상 거리 약 3000cm"), FMath::IsNearlyEqual(TargetUseResult.DistanceCm, 3000.0f, 1.0f));

	const FCFVehicleFireOrigin FireOriginAfterCompatibleSelection = VehicleWeaponComp->GetLastFireOrigin();
	TestTrue(TEXT("선택 평가가 직접 조준 발사 위치를 변경하지 않음"), FireOriginAfterCompatibleSelection.WorldFireLocation.Equals(DirectAimFireOrigin.WorldFireLocation, KINDA_SMALL_NUMBER));
		TestTrue(TEXT("선택 평가가 직접 조준 발사 방향을 변경하지 않음"), FireOriginAfterCompatibleSelection.WorldFireDirection.Equals(DirectAimFireOrigin.WorldFireDirection, KINDA_SMALL_NUMBER));

	CompatibleTarget->SetActorLocation(FVector(7000.0f, 0.0f, 0.0f));
	VehicleWeaponComp->TickComponent(0.11f, LEVELTICK_All, nullptr);
	TargetUseResult = VehicleWeaponComp->GetLastActiveWeaponTargetUseResult();
	TestFalse(TEXT("대상 이동 뒤 자동 거리 초과 갱신"), TargetUseResult.bWithinUseDistance);
	TestEqual(TEXT("대상 이동 뒤 자동 거리 초과 사유"), TargetUseResult.FailureReason, ECFTargetUseFailureReason::OutOfRange);
	TestTrue(TEXT("대상 이동 뒤 최신 거리 약 7000cm"), FMath::IsNearlyEqual(TargetUseResult.DistanceCm, 7000.0f, 1.0f));

	CompatibleTarget->SetActorLocation(FVector(3000.0f, 0.0f, 0.0f));
	VehicleWeaponComp->TickComponent(0.11f, LEVELTICK_All, nullptr);
	TargetUseResult = VehicleWeaponComp->GetLastActiveWeaponTargetUseResult();
	TestTrue(TEXT("대상 복귀 뒤 자동 사용 가능 갱신"), TargetUseResult.bCanUseTarget);
	TestEqual(TEXT("대상 복귀 뒤 실패 사유 없음"), TargetUseResult.FailureReason, ECFTargetUseFailureReason::None);

	TestTrue(TEXT("아군 대상 선택 가능"), TargetSelectComp->SetSelectedTarget(FriendlyTarget, SelectionContext));
	TargetUseResult = VehicleWeaponComp->GetLastActiveWeaponTargetUseResult();
	TestEqual(TEXT("아군 장비 비호환 사유"), TargetUseResult.FailureReason, ECFTargetUseFailureReason::RelationNotAllowed);
	TestTrue(TEXT("아군도 선택 상태는 유지"), TargetUseResult.bHasSelectedTarget);
	TestFalse(TEXT("아군에는 무기 사용 불가"), TargetUseResult.bCanUseTarget);

	TestTrue(TEXT("장치 대상 선택 가능"), TargetSelectComp->SetSelectedTarget(DeviceTarget, SelectionContext));
	TargetUseResult = VehicleWeaponComp->GetLastActiveWeaponTargetUseResult();
	TestEqual(TEXT("장치 분류 비호환 사유"), TargetUseResult.FailureReason, ECFTargetUseFailureReason::CategoryNotAllowed);

	TestTrue(TEXT("필수 태그 누락 대상 선택"), TargetSelectComp->SetSelectedTarget(MissingTagTarget, SelectionContext));
	TargetUseResult = VehicleWeaponComp->GetLastActiveWeaponTargetUseResult();
	TestEqual(TEXT("필수 태그 누락 사유"), TargetUseResult.FailureReason, ECFTargetUseFailureReason::MissingRequiredTag);
	TestEqual(TEXT("누락 태그 식별"), TargetUseResult.FailureAttributeTag, FName(TEXT("Lockable")));

	TestTrue(TEXT("제외 태그 대상 선택"), TargetSelectComp->SetSelectedTarget(ExcludedTagTarget, SelectionContext));
	TargetUseResult = VehicleWeaponComp->GetLastActiveWeaponTargetUseResult();
	TestEqual(TEXT("제외 태그 사유"), TargetUseResult.FailureReason, ECFTargetUseFailureReason::ExcludedByTag);
	TestEqual(TEXT("제외 태그 식별"), TargetUseResult.FailureAttributeTag, FName(TEXT("Jammed")));

	TestTrue(TEXT("거리 초과 대상 선택"), TargetSelectComp->SetSelectedTarget(FarTarget, SelectionContext));
	TargetUseResult = VehicleWeaponComp->GetLastActiveWeaponTargetUseResult();
	TestTrue(TEXT("거리 초과 대상 정책은 호환"), TargetUseResult.bTargetCompatible);
	TestFalse(TEXT("거리 초과 판정"), TargetUseResult.bWithinUseDistance);
	TestEqual(TEXT("거리 초과 사유"), TargetUseResult.FailureReason, ECFTargetUseFailureReason::OutOfRange);

	TestTrue(TEXT("추적 상태 검증용 호환 대상 재선택"), TargetSelectComp->SetSelectedTarget(CompatibleTarget, SelectionContext));
	TestTrue(TEXT("선택 대상 가림 상태 변경"), TargetSelectComp->SetSelectedTargetTrackState(CompatibleTarget, ECFTargetTrackState::Occluded));
	TargetUseResult = VehicleWeaponComp->GetLastActiveWeaponTargetUseResult();
	TestEqual(TEXT("가림 추적 상태 비호환 사유"), TargetUseResult.FailureReason, ECFTargetUseFailureReason::TrackStateNotAllowed);
	TestEqual(TEXT("가림 추적 상태 캐시"), TargetUseResult.TrackState, ECFTargetTrackState::Occluded);

	TestTrue(TEXT("파괴 대상 선택"), TargetSelectComp->SetSelectedTarget(DestroyedTarget, SelectionContext));
	TestTrue(TEXT("파괴 전 장비 사용 가능"), VehicleWeaponComp->GetLastActiveWeaponTargetUseResult().bCanUseTarget);
	DestroyedTarget->Destroy();
	TargetUseResult = VehicleWeaponComp->GetLastActiveWeaponTargetUseResult();
	TestFalse(TEXT("대상 파괴 뒤 선택 기록 없음"), TargetUseResult.bHasSelectedTarget);
	TestFalse(TEXT("대상 파괴 뒤 장비 사용 불가"), TargetUseResult.bCanUseTarget);
	TestEqual(TEXT("대상 파괴 뒤 안전 실패"), TargetUseResult.FailureReason, ECFTargetUseFailureReason::NoSelectedTarget);

	TestTrue(TEXT("선택 변경 검증용 호환 대상 선택"), TargetSelectComp->SetSelectedTarget(CompatibleTarget, SelectionContext));
	TestEqual(TEXT("호환 대상 캐시"), VehicleWeaponComp->GetLastActiveWeaponTargetUseResult().TargetActor.Get(), static_cast<AActor*>(CompatibleTarget));
	TestTrue(TEXT("선택 변경 검증용 원거리 대상 선택"), TargetSelectComp->SetSelectedTarget(FarTarget, SelectionContext));
	TestEqual(TEXT("장비가 이전 대상 획득 상태를 소유하지 않고 새 선택을 평가"), VehicleWeaponComp->GetLastActiveWeaponTargetUseResult().TargetActor.Get(), static_cast<AActor*>(FarTarget));

	const FCFVehicleFireOrigin FireOriginAfterMultipleQueries = VehicleWeaponComp->GetLastFireOrigin();
	TestTrue(TEXT("다수 선택 변경 뒤 발사 위치 보존"), FireOriginAfterMultipleQueries.WorldFireLocation.Equals(DirectAimFireOrigin.WorldFireLocation, KINDA_SMALL_NUMBER));
	TestTrue(TEXT("다수 선택 변경 뒤 발사 방향 보존"), FireOriginAfterMultipleQueries.WorldFireDirection.Equals(DirectAimFireOrigin.WorldFireDirection, KINDA_SMALL_NUMBER));

	TestTrue(TEXT("장비 비호환 검증 전 호환 대상 선택"), TargetSelectComp->SetSelectedTarget(CompatibleTarget, SelectionContext));
	WeaponData->CompatibleMountTypes.Reset();
	WeaponData->CompatibleMountTypes.Add(ECFVehicleMountType::Utility);
	TestTrue(TEXT("장착 구조 자체는 유지한 채 무기 비호환 런타임 초기화"), VehicleWeaponComp->InitializeWeaponRuntime(VehiclePawn, VehicleData));
	TargetUseResult = VehicleWeaponComp->GetLastActiveWeaponTargetUseResult();
	TestTrue(TEXT("장비 비호환이어도 선택 기록 존재"), TargetUseResult.bHasSelectedTarget);
	TestTrue(TEXT("장비 비호환이어도 선택 대상 유효"), TargetUseResult.bSelectedTargetValid);
	TestFalse(TEXT("무기 비호환 장비 사용 불가"), TargetUseResult.bCanUseTarget);
	TestEqual(TEXT("무기 비호환 실패 사유"), TargetUseResult.FailureReason, ECFTargetUseFailureReason::EquipmentUnavailable);
	TestEqual(TEXT("장비 평가 실패가 공용 선택을 해제하지 않음"), TargetSelectComp->GetSelectedTargetActor(), static_cast<AActor*>(CompatibleTarget));

	CompatibleTarget->Destroy();
	FriendlyTarget->Destroy();
	DeviceTarget->Destroy();
	MissingTagTarget->Destroy();
	ExcludedTagTarget->Destroy();
	FarTarget->Destroy();
	VehiclePawn->Destroy();
	return true;
}

#endif
