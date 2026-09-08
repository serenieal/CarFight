// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-09-08
// Description: CF-FQ-030 미사일 유도 체감 튜닝용 Guidance Preset DataAsset
// Scope: MissileGuideConfig 한 덩어리를 저장하는 passive data container로서 Editor/PIE 체감 시험과 후속 미사일 정의에서 코드 재빌드 없이 유도 성능을 조정할 수 있게 합니다.
// Changelog:
// - v1.2.0: PostInitProperties와 자산 이름 기반 Low/Normal/High seed를 제거해 로드 시 사용자 저장 튜닝값에 개입하지 않는 passive data container로 교정. 신규 Preset 초기 seed 책임은 CarFight_ReEditor authoring 경로로 이동.
// - v1.1.0: WriteProbe의 일반 Object property write 제한을 우회하지 않고, Low/Normal/High 신규 DA가 처음 생성될 때 자산 이름 기반 초기값을 1회 제공하는 생성 기본값 로직 추가. v1.2.0에서 수명주기 경계 문제로 폐기.
// - v1.0.0: MG-P0-12 USER Feel 반복 튜닝의 C++ 하드코딩 제거를 위해 Guidance Preset DataAsset 타입 최초 추가.
// Migration:
// - 기존 UCFProjectileData.MissileGuideConfig 소유권은 변경하지 않습니다.
// - 이 DataAsset은 시험/저작용 Guidance 설정 묶음이며 Projectile 속도·피해·Collision·FX를 소유하지 않습니다.
// - 기존 MissileDirectTest 저장 ProjectileData는 그대로 기준으로 사용하고 Feel 명령은 선택 Preset의 MissileGuideConfig만 transient Projectile 복제본에 덮어씁니다.
// - v1.2.0부터 이 클래스는 Low/Normal/High 이름이나 기본값을 알지 않으며 저장 Asset 로드 중 어떤 튜닝값도 재주입하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "CFMissileGuideTypes.h"
#include "Engine/DataAsset.h"
#include "CFMissileGuidePresetData.generated.h"

/**
 * 미사일 유도 성능과 추적 성격을 재빌드 없이 반복 조정하기 위한 Guidance Preset DataAsset입니다.
 */
UCLASS(BlueprintType)
class CARFIGHT_RE_API UCFMissileGuidePresetData : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// [v1.0.0] Preset을 로그·자동화·저작 도구에서 안정적으로 식별할 ID입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Missile Guide Preset|Identity", meta=(DisplayName="프리셋 ID (PresetId)", ToolTip="이 미사일 유도 프리셋을 안정적으로 식별할 이름입니다. 예: MissileFeel_Custom"))
	FName PresetId = NAME_None;

	// [v1.0.0] 에디터와 시험 안내에서 사용자에게 표시할 프리셋 이름입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Missile Guide Preset|Identity", meta=(DisplayName="프리셋 표시 이름 (PresetDisplayName)", ToolTip="에디터와 미사일 체감 시험 안내에서 표시할 읽기 쉬운 이름입니다."))
	FText PresetDisplayName;

	// [v1.0.0] 이 프리셋의 의도와 대표 체감을 기록할 저작용 설명입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Missile Guide Preset|Identity", meta=(MultiLine="true", DisplayName="프리셋 설명 (PresetDescription)", ToolTip="이 프리셋이 어떤 유도 성격을 목표로 하는지 설명합니다. Runtime 판정에는 사용하지 않습니다."))
	FText PresetDescription;

	// [v1.0.0] 실제 Missile Guide Component에 적용할 전체 유도 설정입니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="CarFight|Missile Guide Preset", meta=(DisplayName="미사일 유도 설정 (MissileGuideConfig)", ToolTip="Guidance Law, 유도 시작 조건, Seeker 각도, 관측, 선회율, 횡가속, 재포착 등 미사일 유도 체감을 결정하는 전체 설정입니다."))
	FCFMissileGuideConfig MissileGuideConfig;
};
