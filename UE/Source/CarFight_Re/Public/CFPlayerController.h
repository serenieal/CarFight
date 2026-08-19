// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.4.0
// Date: 2026-08-18
// Description: CarFight UI 공통 입력·Mapping Context·싱글플레이 Pause PlayerController
// Scope: Pawn 독립 Pause·Back 입력, Controller 소유 Context, 입력 중립화, 실제 World Pause와 UI USER Visual 검증 진입점을 제공합니다.
// Changelog:
// - v1.4.0: M_VehicleDefensePIE 전용 UI-P0-03 USER Visual 검증을 위해 PIE 한정 방어 Pawn 빙의·기준 Pawn 복귀·방어 피해 Exec 진입점을 추가.
// - v1.3.0: Pause 중 Enter·게임패드 확인 버튼으로 Continue를 실행하는 Controller Fallback 입력을 추가.
// - v1.2.0: 차량 Gameplay 입력 소유권을 Pawn DefaultInputMappingContext로 잠그고 Controller Gameplay Context의 원자 이전 조건을 명시.
// - v1.1.0: UI-P0-02 차량 Gameplay 입력 중립화, 키 상태 Flush와 실제 싱글플레이 Pause 적용·해제 API를 추가.
// - v1.0.0: UI-P0-01A 최소 PlayerController와 System·Gameplay·UI Context 수명 계약을 최초 추가.
// Migration:
// - 현재 차량 Gameplay 입력 소유자는 ACFVehiclePawn이며, Pawn 자동 Context 등록을 비활성화하는 원자 이전 작업 전까지 GameplayInputMappingContext는 None으로 유지한다.
// - 기존 ACFVehiclePawn의 DefaultInputMappingContext와 Gameplay Action 바인딩은 유지한다.
// - Controller는 자신이 등록한 Mapping Context만 제거하며 Legacy Pawn Context를 제거하지 않는다.
// - 실제 게임 일시정지와 입력 잔류 초기화는 v1.1.0부터 UISubsystem Pause Menu 수명과 함께 사용한다.
// - Pause 진입은 차량 입력만 중립화하며 Launcher Sequence와 Projectile 상태를 취소하거나 재생성하지 않는다.
// - v1.4.0 UI Visual Exec 진입점은 PIE의 M_VehicleDefensePIE 계열 맵에서만 동작하며 Production Gameplay 입력이나 저장 에셋 계약에는 포함하지 않는다.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "UI/CFUITypes.h"
#include "CFPlayerController.generated.h"

class APawn;
class ACFVehiclePawn;
class UCFUISubsystem;
class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UInputMappingContext;
class UWidget;
struct FInputActionValue;

/**
 * Controller가 Pause 공통 입력을 받았음을 알리는 이벤트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCFPlayerPauseRequestSignature);

/**
 * Controller가 Back 공통 입력을 받았음을 알리는 이벤트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FCFPlayerBackRequestSignature);

/**
 * Controller의 Possessed Pawn이 변경됐음을 알리는 이벤트입니다.
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCFControlledPawnChangedSignature, APawn*, PreviousPawn, APawn*, NewPawn);

/**
 * Pawn 수명과 독립된 UI 공통 입력과 LocalPlayer UI 연결을 소유하는 최소 PlayerController입니다.
 */
UCLASS(BlueprintType, Blueprintable)
class CARFIGHT_RE_API ACFPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	// [v1.0.0] 게임 입력을 기본 상태로 하고 커서를 숨깁니다.
	ACFPlayerController();

	// [v1.0.0] LocalPlayer UI Subsystem 등록과 Controller 소유 Context를 준비합니다.
	virtual void BeginPlay() override;

	// [v1.0.0] Controller가 등록한 Context와 UI Subsystem 연결을 정리합니다.
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	// [v1.0.0] Pause·Back Enhanced Input 또는 안전 Fallback Key를 바인딩합니다.
	virtual void SetupInputComponent() override;

	// [v1.0.0] 새 Pawn Possession을 UI와 Gameplay Context 수명에 통지합니다.
	virtual void OnPossess(APawn* InPawn) override;

	// [v1.0.0] 기존 Pawn의 입력 억제를 복원하고 Gameplay Context를 제거합니다.
	virtual void OnUnPossess() override;

	// [v1.0.0] System은 유지하고 Gameplay와 UI Context를 현재 입력 상태에 맞게 갱신합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|PlayerController|Input", meta=(DisplayName="입력 Mapping Context 갱신 (Refresh Input Mapping Contexts)", ToolTip="Controller가 소유한 System, Gameplay와 UI Mapping Context를 현재 Pawn과 UI 입력 상태에 맞게 중복 없이 갱신합니다."))
	void RefreshInputMappingContexts();

	// [v1.0.0] UI 전용 상태와 Gameplay 상태 사이에서 Context와 Pawn 입력 억제를 전환합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|PlayerController|Input", meta=(DisplayName="UI 입력 활성화 설정 (Set UI Input Enabled)", ToolTip="True이면 Controller 소유 Gameplay Context를 제거하고 UI Context를 등록하며 기존 Pawn Gameplay 입력을 억제합니다. False이면 반대로 복원합니다."))
	void SetUIInputEnabled(bool bEnabled);

	// [v1.0.0] Unreal Input Mode, Focus, Cursor와 Controller 소유 Context를 함께 적용합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|PlayerController|Input", meta=(DisplayName="UI 입력 모드 적용 (Apply UI Input Mode)", ToolTip="GameOnly, GameAndUI 또는 UIOnly 입력 모드와 Focus Widget, 마우스 커서를 적용하고 Mapping Context 수명을 동기화합니다."))
	void ApplyUIInputMode(ECFUIInputMode InputMode, UWidget* FocusWidget, bool bShowCursor);

		// [v1.1.0] Pause 진입 전 현재 차량 Gameplay 입력과 Controller 키 상태를 중립화합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|PlayerController|Pause", meta=(DisplayName="Pause 전 Gameplay 입력 준비 (Prepare Gameplay Input For Pause)", ToolTip="현재 차량의 이동, 조향, 브레이크, 핸드브레이크와 Look 입력을 중립화하고 Controller의 눌린 키 상태를 비웁니다. Launcher 시퀀스는 취소하지 않습니다."))
	void PrepareGameplayInputForPause();

	// [v1.1.0] 싱글플레이 World Pause를 적용하거나 해제합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|PlayerController|Pause", meta=(DisplayName="싱글플레이 Pause 설정 (Set Single Player Paused)", ToolTip="Pause 진입 시 Gameplay 입력을 먼저 중립화한 뒤 Unreal World Pause를 적용합니다. 해제 시 눌린 키 상태를 비워 입력 잔류를 방지합니다."))
	bool SetSinglePlayerPaused(bool bShouldPause);

	// [v1.1.0] 현재 World가 Pause 상태인지 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|PlayerController|Pause", meta=(DisplayName="싱글플레이 Pause 여부 반환 (Is Single Player Paused)", ToolTip="현재 Controller가 속한 World의 실제 Pause 상태를 반환합니다."))
	bool IsSinglePlayerPaused() const;

	// [v1.0.0] 현재 UI 입력 활성화 여부를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|PlayerController|Input", meta=(DisplayName="UI 입력 활성화 여부 반환 (Is UI Input Enabled)", ToolTip="현재 Controller가 Gameplay보다 UI 입력을 우선하도록 설정됐는지 반환합니다."))
	bool IsUIInputEnabled() const { return bUIInputEnabled; }

	// [v1.0.0] Controller가 실제 등록해 소유 중인 Mapping Context 수를 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|PlayerController|Input", meta=(DisplayName="소유 Mapping Context 수 반환 (Get Owned Mapping Context Count)", ToolTip="Controller가 직접 등록해 제거 책임을 가진 System, Gameplay와 UI Mapping Context 수를 반환합니다."))
	int32 GetOwnedMappingContextCount() const;

			// [v1.0.0] 현재 LocalPlayer의 CarFight UI Subsystem을 반환합니다.
	UFUNCTION(BlueprintPure, Category="CarFight|PlayerController|UI", meta=(DisplayName="UI Subsystem 반환 (Get UI Subsystem)", ToolTip="현재 LocalPlayer가 소유한 CFUISubsystem을 반환합니다."))
	UCFUISubsystem* GetUISubsystem() const;

	// [v1.4.0] UI-P0-03 USER Visual에서 현재 PIE의 정식 Defense Pawn으로 빙의합니다.
	UFUNCTION(Exec, Category="CarFight|PlayerController|UI|Acceptance", meta=(DisplayName="UI Visual 방어 Pawn 빙의", ToolTip="M_VehicleDefensePIE 계열 PIE에서 정식 DefenseData가 초기화된 차량 Pawn으로 빙의합니다. 저장 에셋은 변경하지 않습니다."))
	void CFUIVisualPossessDefensePawn();

	// [v1.4.0] UI-P0-03 Pawn Rebind USER Visual에서 비방어 기준 Pawn으로 돌아갑니다.
	UFUNCTION(Exec, Category="CarFight|PlayerController|UI|Acceptance", meta=(DisplayName="UI Visual 기준 Pawn 빙의", ToolTip="M_VehicleDefensePIE 계열 PIE에서 DefenseData가 없는 기준 차량 Pawn으로 빙의해 HUD Rebind를 확인합니다. 저장 에셋은 변경하지 않습니다."))
	void CFUIVisualPossessBaselinePawn();

	// [v1.4.0] UI-P0-03 Defense USER Visual에서 방어 차량에 Shield·Armor·Integrity가 모두 변하는 정면 피해를 1회 적용합니다.
	UFUNCTION(Exec, Category="CarFight|PlayerController|UI|Acceptance", meta=(DisplayName="UI Visual 방어 피해 적용", ToolTip="M_VehicleDefensePIE 계열 PIE의 정식 Defense 차량에 BaseDamage 200 / ArmorPenetration 50 정면 피해를 적용합니다. Production Damage 경로를 사용하며 저장 에셋은 변경하지 않습니다."))
	void CFUIVisualApplyDefenseDamage();

	// [v1.0.0] Pause 공통 입력 요청 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|PlayerController|Input")
	FCFPlayerPauseRequestSignature OnPauseInputRequested;

	// [v1.0.0] Back 공통 입력 요청 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|PlayerController|Input")
	FCFPlayerBackRequestSignature OnBackInputRequested;

	// [v1.0.0] Possessed Pawn 변경 이벤트입니다.
	UPROPERTY(BlueprintAssignable, Category="CarFight|PlayerController|Lifecycle")
	FCFControlledPawnChangedSignature OnControlledPawnChanged;

protected:
	// [v1.0.0] Pawn이 없어도 유지할 System Mapping Context입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|PlayerController|Input", meta=(DisplayName="System 입력 Mapping Context (SystemInputMappingContext)", ToolTip="Pause와 공통 Back처럼 Pawn 수명과 독립된 입력을 담는 Mapping Context입니다. 비어 있으면 C++ Fallback Key를 사용할 수 있습니다."))
	TObjectPtr<UInputMappingContext> SystemInputMappingContext = nullptr;

		// [v1.2.0] 차량 입력 소유권을 Controller로 원자 이전하기 전에는 None으로 유지할 Gameplay Mapping Context입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|PlayerController|Input", meta=(DisplayName="Gameplay 입력 Mapping Context (GameplayInputMappingContext)", ToolTip="현재 차량 Gameplay 입력은 Pawn DefaultInputMappingContext가 소유하므로 None을 유지합니다. 향후 Pawn의 자동 Mapping 등록을 비활성화하고 Controller로 전체 입력 소유권을 같은 변경에서 이전할 때만 지정합니다."))
	TObjectPtr<UInputMappingContext> GameplayInputMappingContext = nullptr;

	// [v1.0.0] 메뉴·화면 조작에 사용할 UI Mapping Context입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|PlayerController|Input", meta=(DisplayName="UI 입력 Mapping Context (UIInputMappingContext)", ToolTip="메뉴 이동, 확인, 취소와 탭 전환을 담을 UI Mapping Context입니다."))
	TObjectPtr<UInputMappingContext> UIInputMappingContext = nullptr;

	// [v1.0.0] Pawn 독립 Pause 입력 Action입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|PlayerController|Input", meta=(DisplayName="Pause 입력 Action (InputAction_Pause)", ToolTip="Pawn이 없어도 처리할 Pause Boolean Input Action입니다. 비어 있으면 Escape와 Gamepad Special Right Fallback을 사용합니다."))
	TObjectPtr<UInputAction> InputAction_Pause = nullptr;

	// [v1.0.0] UI 공통 Back 입력 Action입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|PlayerController|Input", meta=(DisplayName="Back 입력 Action (InputAction_Back)", ToolTip="현재 화면 뒤로가기 또는 취소 요청에 사용할 Boolean Input Action입니다. 비어 있으면 BackSpace와 Gamepad FaceButton Right Fallback을 사용합니다."))
	TObjectPtr<UInputAction> InputAction_Back = nullptr;

	// [v1.0.0] System Context 우선순위입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|PlayerController|Input", meta=(DisplayName="System Context 우선순위 (SystemInputPriority)", ToolTip="System Mapping Context 등록 우선순위입니다."))
	int32 SystemInputPriority = 100;

	// [v1.0.0] Gameplay Context 우선순위입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|PlayerController|Input", meta=(DisplayName="Gameplay Context 우선순위 (GameplayInputPriority)", ToolTip="Controller 소유 Gameplay Mapping Context 등록 우선순위입니다."))
	int32 GameplayInputPriority = 0;

	// [v1.0.0] UI Context 우선순위입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|PlayerController|Input", meta=(DisplayName="UI Context 우선순위 (UIInputPriority)", ToolTip="UI Mapping Context 등록 우선순위입니다."))
	int32 UIInputPriority = 200;

	// [v1.0.0] Input Action 에셋이 없을 때 C++ 안전 키를 바인딩할지 여부입니다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="CarFight|PlayerController|Input", meta=(DisplayName="System 입력 Fallback Key 사용 (bBindFallbackSystemKeysWhenActionsMissing)", ToolTip="Pause 또는 Back Input Action이 비어 있을 때 C++ 기본 키를 바인딩합니다. Pause Fallback은 Pause 중에도 실행됩니다."))
	bool bBindFallbackSystemKeysWhenActionsMissing = true;

private:
	// [v1.4.0] UI Visual Exec 진입점이 허용되는 M_VehicleDefensePIE 계열 PIE World인지 확인합니다.
	bool IsUIVisualAcceptancePIE() const;

	// [v1.4.0] 현재 PIE World에서 정식 DefenseData가 초기화된 차량 Pawn을 찾습니다.
	ACFVehiclePawn* FindUIVisualDefensePawn() const;

	// [v1.4.0] 현재 PIE World에서 DefenseData가 없는 비방어 기준 차량 Pawn을 찾습니다.
	ACFVehiclePawn* FindUIVisualBaselinePawn() const;

	// [v1.0.0] Enhanced Input Pause Action 입력을 공통 요청으로 변환합니다.
	void HandlePauseInputAction(const FInputActionValue& InputActionValue);

	// [v1.0.0] Enhanced Input Back Action 입력을 공통 요청으로 변환합니다.
	void HandleBackInputAction(const FInputActionValue& InputActionValue);

	// [v1.0.0] C++ Fallback Pause Key 입력을 공통 요청으로 변환합니다.
	void HandlePauseFallbackKey();

		// [v1.0.0] C++ Fallback Back Key 입력을 공통 요청으로 변환합니다.
	void HandleBackFallbackKey();

	// [v1.3.0] Pause 중 Enter·게임패드 확인 입력을 Continue 요청으로 변환합니다.
	void HandlePauseConfirmFallbackKey();

	// [v1.0.0] Pause 요청을 Controller와 UI Subsystem에 한 번씩 전달합니다.
	void BroadcastPauseInputRequest();

	// [v1.0.0] Back 요청을 Controller와 UI Subsystem에 한 번씩 전달합니다.
	void BroadcastBackInputRequest();

	// [v1.0.0] Possession 변경을 Controller와 UI Subsystem에 전달합니다.
	void NotifyControlledPawnChanged(APawn* PreviousPawn, APawn* NewPawn);

	// [v1.0.0] 현재 LocalPlayer의 Enhanced Input Subsystem을 반환합니다.
	UEnhancedInputLocalPlayerSubsystem* GetEnhancedInputSubsystem() const;

	// [v1.0.0] 지정 Context를 Controller 소유로 중복 없이 등록합니다.
	bool RegisterOwnedMappingContext(UInputMappingContext* MappingContext, int32 Priority, TWeakObjectPtr<const UInputMappingContext>& RegisteredContext);

	// [v1.0.0] Controller가 등록한 지정 Context만 제거합니다.
	void RemoveOwnedMappingContext(TWeakObjectPtr<const UInputMappingContext>& RegisteredContext);

	// [v1.0.0] Controller가 등록한 모든 Context를 제거합니다.
	void RemoveAllOwnedMappingContexts();

	// [v1.0.0] Legacy Pawn Gameplay 입력을 삭제하지 않고 현재 UI 상태에 따라 억제·복원합니다.
	void ApplyGameplayInputSuppression(bool bSuppressGameplayInput);

	// [v1.0.0] 현재 UI 입력이 Gameplay보다 우선하는지 여부입니다.
	bool bUIInputEnabled = false;

	// [v1.0.0] 현재 Possessed Pawn의 Gameplay 입력이 일시 억제됐는지 여부입니다.
	bool bGameplayInputSuppressed = false;

	// [v1.0.0] Controller가 등록해 제거 책임을 가진 실제 System Context입니다.
	TWeakObjectPtr<const UInputMappingContext> RegisteredSystemContext;

	// [v1.0.0] Controller가 등록해 제거 책임을 가진 실제 Gameplay Context입니다.
	TWeakObjectPtr<const UInputMappingContext> RegisteredGameplayContext;

	// [v1.0.0] Controller가 등록해 제거 책임을 가진 실제 UI Context입니다.
	TWeakObjectPtr<const UInputMappingContext> RegisteredUIContext;
};
