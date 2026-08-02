// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-08-01
// Description: CarFight 공통 UI 레이어·화면·입력 모드 타입
// Scope: UI-P0-01A~01B의 PlayerController와 LocalPlayer UI Root가 공유할 최소 타입 계약을 제공합니다.
// Changelog:
// - v1.0.0: UI Layer, Screen State와 Input Mode enum을 최초 추가.
// Migration:
// - 기존 Pawn 소유 AimReticle·TargetSelect HUD와 Gameplay 입력은 이 타입 추가로 변경되지 않는다.
// - 피팅 Loadout·Draft·Snapshot 타입은 이 파일에 추가하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "CFUITypes.generated.h"

/**
 * UI Root 안에서 Widget을 배치할 표준 레이어입니다.
 */
UENUM(BlueprintType)
enum class ECFUILayer : uint8
{
	Game UMETA(DisplayName="월드 표시 (Game)"),
	HUD UMETA(DisplayName="인게임 HUD (HUD)"),
	Screen UMETA(DisplayName="전체 화면 (Screen)"),
	Panel UMETA(DisplayName="상세 패널 (Panel)"),
	Menu UMETA(DisplayName="메뉴 (Menu)"),
	Modal UMETA(DisplayName="모달 (Modal)"),
	System UMETA(DisplayName="시스템 (System)"),
	Debug UMETA(DisplayName="디버그 (Debug)")
};

/**
 * LocalPlayer UI가 현재 어떤 화면 흐름에 있는지 나타냅니다.
 */
UENUM(BlueprintType)
enum class ECFUIScreenState : uint8
{
	None UMETA(DisplayName="없음 (None)"),
	InGame UMETA(DisplayName="인게임 (In Game)"),
	FullScreen UMETA(DisplayName="전체 화면 (Full Screen)"),
	Paused UMETA(DisplayName="일시정지 (Paused)"),
	Modal UMETA(DisplayName="모달 (Modal)"),
	Transition UMETA(DisplayName="전환 중 (Transition)")
};

/**
 * PlayerController가 적용할 게임·UI 입력 모드입니다.
 */
UENUM(BlueprintType)
enum class ECFUIInputMode : uint8
{
	GameOnly UMETA(DisplayName="게임 전용 (Game Only)"),
	GameAndUI UMETA(DisplayName="게임과 UI (Game And UI)"),
	UIOnly UMETA(DisplayName="UI 전용 (UI Only)")
};
