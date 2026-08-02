// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.7.0
// Date: 2026-07-28
// Description: CarFight 터렛 마운트 DataAsset 구현
// Scope: 터렛 마운트 데이터 기본값, Base / Yaw / Pitch 시각 메쉬 보유 여부, 디버그 요약 생성을 제공합니다.
// Changelog:
// - v1.7.0: 터렛 마운트 요약에 가변 Muzzle 배열, 전체 필수 정책과 Legacy 단일 Muzzle fallback을 표시.
// - v1.6.0: 터렛 마운트 요약에 총구 안전 검사 거리를 표시.
// - v1.5.0: 터렛 마운트 요약에 정렬 중 발사 허용 정책을 표시.
// - v1.4.0: MountProfile legacy 직접 TurretMountData 슬롯 제거에 맞춰 Missing 기준을 EquipmentPresetData 내부 참조로 갱신.
// - v1.3.0: 터렛 회전 제한이 MountProfile 교집합 없이 TurretMountData 단독 기준임을 명시.
// - v1.2.0: MuzzleSocketName이 FireOrigin 전환에 쓰이는 총구 소켓임을 문서화.
// - v1.1.0: Base 메쉬와 YawPivot 소켓을 요약과 시각 메쉬 보유 판정에 포함.
// - v1.0.0: 최소 TurretMountData DataAsset 구현을 추가.
// Migration:
// - MuzzleSocketNames 배열이 비어 있으면 기존 MuzzleSocketName 하나를 사용하는 Legacy Single 경로로 표시한다.
// - 기존 TurretMountData 인스턴스는 MuzzleClearanceDistanceCm=150cm 기본값을 사용하며, 0이면 총구 가림 사전 검사를 비활성화한다.
// - 기존 TurretMountData 인스턴스는 bAllowFireWhileAligning=true 기본값을 사용하며, false로 설정하면 정렬 완료 전 발사를 거부한다.
// - EquipmentPresetData 내부 TurretMountData가 비어 있으면 MountProfile inline 터렛 fallback은 사용하지 않고 Missing 상태로 건너뛴다.
// - TurretBaseMesh가 비어 있어도 YawPivot은 하드포인트 루트 기준으로 동작한다.
// - MuzzleSocketName이 Pitch 메쉬에 존재하면 최종 FireOrigin은 해당 소켓 기준으로 보정된다.
// - Min/Max Yaw/Pitch는 MountProfile 교집합 없이 런타임 터렛 회전 제한의 단일 기준이다.

#include "CFTurretMountData.h"

#include "Engine/StaticMesh.h"

// [v1.0.0] 기본 터렛 마운트 데이터 값을 초기화합니다.
UCFTurretMountData::UCFTurretMountData()
{
}

// [v1.1.0] 표시 가능한 터렛 시각 메쉬가 하나라도 지정되어 있는지 반환합니다.
bool UCFTurretMountData::HasTurretVisualMeshes() const
{
	return TurretBaseMesh != nullptr || TurretYawMesh != nullptr || TurretPitchMesh != nullptr;
}

// [v1.7.0] 디버그 패널에 가변 Muzzle, 발사 정책과 총구 안전 거리를 포함한 터렛 마운트 데이터 요약 문자열을 생성합니다.
FString UCFTurretMountData::BuildTurretMountSummary() const
{
	// [v1.7.0] MuzzleSocketNames 배열에 지정된 유효 이름을 순서대로 표시할 문자열 목록입니다.
	TArray<FString> MuzzleSocketNameTexts;
	for (const FName MuzzleSocketNameEntry : MuzzleSocketNames)
	{
		if (!MuzzleSocketNameEntry.IsNone())
		{
			MuzzleSocketNameTexts.Add(MuzzleSocketNameEntry.ToString());
		}
	}

	// [v1.7.0] 배열이 비어 있을 때 기존 단일 Muzzle fallback을 명확히 표시하는 총구 목록 문자열입니다.
	const FString MuzzleListText = MuzzleSocketNameTexts.IsEmpty()
		? FString::Printf(TEXT("Legacy:%s"), *MuzzleSocketName.ToString())
		: FString::Join(MuzzleSocketNameTexts, TEXT("|"));

	// [v1.1.0] 디버그에 표시할 Base 메쉬 이름입니다.
	const FString BaseMeshText = TurretBaseMesh ? TurretBaseMesh->GetName() : TEXT("MissingOptional");

	// [v1.0.0] 디버그에 표시할 Yaw 메쉬 이름입니다.
	const FString YawMeshText = TurretYawMesh ? TurretYawMesh->GetName() : TEXT("MissingOptional");

	// [v1.0.0] 디버그에 표시할 Pitch 메쉬 이름입니다.
	const FString PitchMeshText = TurretPitchMesh ? TurretPitchMesh->GetName() : TEXT("MissingOptional");

		return FString::Printf(
		TEXT("TurretMountData: Id=%s, BaseMesh=%s, YawMesh=%s, PitchMesh=%s, YawPivot=%s, PitchPivot=%s, Muzzles=%s, RequireAllMuzzles=%s, Yaw=%.1f..%.1f, Pitch=%.1f..%.1f, TurnRate=%.1f/%.1f, Settle=%.2fs, AllowFireWhileAligning=%s, MuzzleClearance=%.1fcm, Weight=%.1fkg"),
		*TurretMountId.ToString(),
		*BaseMeshText,
		*YawMeshText,
		*PitchMeshText,
		*YawPivotSocketName.ToString(),
		*PitchPivotSocketName.ToString(),
		*MuzzleListText,
		bRequireAllMuzzles ? TEXT("True") : TEXT("False"),
		MinYawDeg,
		MaxYawDeg,
		MinPitchDeg,
		MaxPitchDeg,
		YawTurnRateDegPerSec,
		PitchTurnRateDegPerSec,
		AimSettleTimeSeconds,
		bAllowFireWhileAligning ? TEXT("True") : TEXT("False"),
		MuzzleClearanceDistanceCm,
		TurretMountWeightKg);
}
