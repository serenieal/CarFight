// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.2.0
// Date: 2026-08-10
// Description: D1-10B Base Widget Blueprint의 Designer WidgetTree를 자동 생성·검증하는 Editor Bridge
// Scope: Python에 노출되지 않은 UWidgetBlueprint::WidgetTree 접근만 보완하며 런타임 UI 또는 Gameplay 책임을 추가하지 않습니다.
// Changelog:
// - v1.2.0: UE 5.8 Python에서 Out FString이 ReturnValue를 가리는 마샬링을 우회하는 bool-only Build/Validate 래퍼를 추가.
// - v1.1.0: 정확한 Native Parent와 WBP_CFButtonBase의 Button_Interaction BindWidget 메타데이터 검증을 추가.
// - v1.0.0: Button/Panel/StatusBar/InfoRow/AlertItem 5종의 정확한 WidgetTree 생성과 구조·Graph 안전성 검증을 최초 추가.
// Migration:
// - 이 클래스는 D1-10B Asset 제작 도구용입니다. Gameplay Runtime, Pause, DataAsset Load와 무관합니다.
// - D1-10B 이후 제품 Widget은 생성된 Blueprint Asset을 사용하며 이 Bridge를 런타임에서 호출하지 않습니다.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CFUIBaseEditorBridge.generated.h"

/**
 * D1-10B Base Widget Blueprint 제작 과정에서만 사용하는 최소 Editor Utility Bridge입니다.
 */
UCLASS()
class CARFIGHT_RE_API UCFUIBaseEditorBridge : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// [v1.0.0] 지정 Widget Blueprint의 Designer Tree를 D1-10B Template ID에 맞는 정확한 구조로 생성합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Base Widget Tree 생성 (Build Base Widget Tree)", ToolTip="D1-10B Widget Blueprint의 Designer Tree를 Button, Panel, StatusBar, InfoRow, AlertItem Template 규격으로 생성합니다. Editor 제작 단계에서만 사용하세요."))
		static bool BuildBaseWidgetTree(UObject* WidgetBlueprintObject, FName TemplateId, FString& OutFailureReason);

	// [v1.2.0] UE Python 자동화가 FString Out Parameter 마샬링과 무관하게 정확한 Bool Build 결과를 받을 수 있는 전용 래퍼입니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Base Widget Tree 생성 결과 (Build Base Widget Tree Result)", ToolTip="D1-10B Python 자동화용 Bool-only Tree 생성 래퍼입니다. 기존 실패 사유는 Editor Log에 기록됩니다."))
	static bool BuildBaseWidgetTreeResult(UObject* WidgetBlueprintObject, FName TemplateId);

	// [v1.1.0] 지정 Widget Blueprint가 D1-10B Native Parent, Tree 구조, Button BindWidget, Spacer Visibility, Cast 0, CallFunction 0 계약을 만족하는지 읽기 전용으로 검증합니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Base Widget Tree 검증 (Validate Base Widget Tree)", ToolTip="D1-10B Widget Blueprint의 정확한 Native Parent·Tree 구조·Button BindWidget과 Gameplay Cast/CallFunction 부재, Spacer 비Collapsed 계약을 검증합니다."))
	static bool ValidateBaseWidgetTree(UObject* WidgetBlueprintObject, FName TemplateId, FString& OutFailureReason);

	// [v1.2.0] UE Python 자동화가 FString Out Parameter 마샬링과 무관하게 정확한 Bool Validate 결과를 받을 수 있는 전용 래퍼입니다.
	UFUNCTION(BlueprintCallable, Category="CarFight|UI|Editor", meta=(DisplayName="Base Widget Tree 검증 결과 (Validate Base Widget Tree Result)", ToolTip="D1-10B Python 자동화용 Bool-only Tree 검증 래퍼입니다. 기존 실패 사유는 Editor Log에 기록됩니다."))
	static bool ValidateBaseWidgetTreeResult(UObject* WidgetBlueprintObject, FName TemplateId);
};
