// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-13
// Description: CarFight 공용 Collision Channel 기준
// Scope: 무기 피격 Trace와 발사체 Object Channel, 차량 시각 피격 Profile 이름을 C++에서 공유합니다.
// Changelog:
// - v1.0.0: WeaponHit Trace Channel, Projectile Object Channel, VehicleVisualHit Profile 이름을 추가.
// Migration:
// - DefaultEngine.ini의 CollisionProfile 정의와 이 파일의 채널 값은 같은 순서를 유지한다.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

namespace CFCollisionChannels
{
	// [v1.0.0] HitScan 무기가 차량 시각 피격 표면을 찾을 때 사용하는 Trace Channel입니다.
	static constexpr ECollisionChannel WeaponHit = ECC_GameTraceChannel1;

	// [v1.0.0] Projectile Actor 충돌 컴포넌트가 사용하는 Object Channel입니다.
	static constexpr ECollisionChannel Projectile = ECC_GameTraceChannel2;

	// [v1.0.0] SM_Body에 적용할 차량 시각 피격 Collision Profile 이름입니다.
	static const FName VehicleVisualHitProfileName(TEXT("VehicleVisualHit"));
}
