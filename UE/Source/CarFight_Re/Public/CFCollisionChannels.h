// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.1.0
// Date: 2026-07-24
// Description: CarFight 공용 Collision Channel 기준
// Scope: 무기 피격 Trace, 발사체 Object Channel, 타겟 선택 Trace와 차량 시각 피격 Profile 이름을 공유합니다.
// Changelog:
// - v1.1.0: TS-P0-03 전용 TargetSelect Trace Channel을 추가.
// - v1.0.0: WeaponHit Trace Channel, Projectile Object Channel, VehicleVisualHit Profile 이름을 추가.
// Migration:
// - DefaultEngine.ini의 CollisionProfile 정의와 이 파일의 채널 값은 같은 순서를 유지한다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

namespace CFCollisionChannels
{
	static constexpr ECollisionChannel WeaponHit = ECC_GameTraceChannel1;
	static constexpr ECollisionChannel Projectile = ECC_GameTraceChannel2;
	static constexpr ECollisionChannel TargetSelect = ECC_GameTraceChannel3;
	static const FName VehicleVisualHitProfileName(TEXT("VehicleVisualHit"));
}
