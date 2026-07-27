// Copyright (c) CarFight. All Rights Reserved.
//
// Version: 1.0.0
// Date: 2026-07-24
// Description: TS-P0-06 TargetSelect HUD 에셋과 상태 표시 자동화 테스트

#include "UI/CFTargetSelectWidget.h"
#include "CFVehiclePawn.h"
#include "CFTargetSelectComp.h"
#include "CFTargetSelectContractTestTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/AutomationTest.h"
#include "Misc/PackageName.h"
#include "Tests/AutomationEditorCommon.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"

namespace
{
	const TCHAR* TargetSelectWidgetPackageName = TEXT("/Game/CarFight/UI/WBP_TargetSelect");
	const TCHAR* TargetSelectWidgetAssetName = TEXT("WBP_TargetSelect");

	bool SaveTargetHudAsset(UObject* AssetObject)
	{
		if (!AssetObject || !AssetObject->GetOutermost())
		{
			return false;
		}
		UPackage* AssetPackage = AssetObject->GetOutermost();
		const FString PackageFilename = FPackageName::LongPackageNameToFilename(AssetPackage->GetName(), FPackageName::GetAssetPackageExtension());
		FSavePackageArgs SavePackageArgs;
		SavePackageArgs.TopLevelFlags = RF_Public | RF_Standalone;
		SavePackageArgs.SaveFlags = SAVE_NoError;
		return UPackage::SavePackage(AssetPackage, AssetObject, *PackageFilename, SavePackageArgs);
	}

	UTextBlock* AddTargetHudText(UWidgetTree* WidgetTree, UVerticalBox* ParentBox, const FName WidgetName, const TCHAR* InitialText)
	{
		if (!WidgetTree || !ParentBox)
		{
			return nullptr;
		}
		UTextBlock* TextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), WidgetName);
		if (TextBlock)
		{
			TextBlock->SetText(FText::FromString(InitialText));
			TextBlock->SetVisibility(ESlateVisibility::HitTestInvisible);
			ParentBox->AddChildToVerticalBox(TextBlock);
		}
		return TextBlock;
	}

	bool BuildTargetHudWidgetTree(UWidgetBlueprint* WidgetBlueprint)
	{
		if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
		{
			return false;
		}
		WidgetBlueprint->Modify();
		WidgetBlueprint->WidgetTree->Modify();
		UCanvasPanel* RootCanvas = WidgetBlueprint->WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("CanvasPanel_Root"));
		UVerticalBox* CandidateRoot = WidgetBlueprint->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VerticalBox_CandidateRoot"));
		UVerticalBox* SelectedRoot = WidgetBlueprint->WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("VerticalBox_SelectedRoot"));
		if (!RootCanvas || !CandidateRoot || !SelectedRoot)
		{
			return false;
		}
		WidgetBlueprint->WidgetTree->RootWidget = RootCanvas;
		RootCanvas->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		CandidateRoot->SetVisibility(ESlateVisibility::Collapsed);
		SelectedRoot->SetVisibility(ESlateVisibility::Collapsed);

		if (UCanvasPanelSlot* CandidateSlot = RootCanvas->AddChildToCanvas(CandidateRoot))
		{
			CandidateSlot->SetAnchors(FAnchors(0.0f, 0.0f));
			CandidateSlot->SetAlignment(FVector2D(0.5f, 1.0f));
			CandidateSlot->SetPosition(FVector2D(960.0f, 500.0f));
			CandidateSlot->SetAutoSize(true);
			CandidateSlot->SetZOrder(0);
		}
		if (UCanvasPanelSlot* SelectedSlot = RootCanvas->AddChildToCanvas(SelectedRoot))
		{
			SelectedSlot->SetAnchors(FAnchors(0.0f, 0.0f));
			SelectedSlot->SetAlignment(FVector2D(0.5f, 1.0f));
			SelectedSlot->SetPosition(FVector2D(960.0f, 500.0f));
			SelectedSlot->SetAutoSize(true);
			SelectedSlot->SetZOrder(1);
		}
		return AddTargetHudText(WidgetBlueprint->WidgetTree, CandidateRoot, TEXT("Text_CandidateMarker"), TEXT("◇"))
			&& AddTargetHudText(WidgetBlueprint->WidgetTree, CandidateRoot, TEXT("Text_CandidateInfo"), TEXT("후보"))
			&& AddTargetHudText(WidgetBlueprint->WidgetTree, SelectedRoot, TEXT("Text_SelectedMarker"), TEXT("▣"))
			&& AddTargetHudText(WidgetBlueprint->WidgetTree, SelectedRoot, TEXT("Text_SelectedInfo"), TEXT("선택"))
			&& AddTargetHudText(WidgetBlueprint->WidgetTree, SelectedRoot, TEXT("Text_SelectedTrackState"), TEXT("미확인 · 가시"));
	}

	UWidgetBlueprint* EnsureTargetHudWidgetBlueprint()
	{
		const FString ObjectPath = FString::Printf(TEXT("%s.%s"), TargetSelectWidgetPackageName, TargetSelectWidgetAssetName);
		UWidgetBlueprint* WidgetBlueprint = LoadObject<UWidgetBlueprint>(nullptr, *ObjectPath);
		if (!WidgetBlueprint)
		{
			UPackage* WidgetPackage = CreatePackage(TargetSelectWidgetPackageName);
			if (!WidgetPackage)
			{
				return nullptr;
			}
			WidgetBlueprint = Cast<UWidgetBlueprint>(FKismetEditorUtilities::CreateBlueprint(
				UCFTargetSelectWidget::StaticClass(), WidgetPackage, FName(TargetSelectWidgetAssetName), BPTYPE_Normal,
				UWidgetBlueprint::StaticClass(), UWidgetBlueprintGeneratedClass::StaticClass(), TEXT("TS-P0-06")));
			if (!WidgetBlueprint)
			{
				return nullptr;
			}
			FAssetRegistryModule::AssetCreated(WidgetBlueprint);
		}
		if (!WidgetBlueprint->ParentClass || !WidgetBlueprint->ParentClass->IsChildOf(UCFTargetSelectWidget::StaticClass()) || !BuildTargetHudWidgetTree(WidgetBlueprint))
		{
			return nullptr;
		}
		FKismetEditorUtilities::CompileBlueprint(WidgetBlueprint);
		if (WidgetBlueprint->Status == BS_Error || !WidgetBlueprint->GeneratedClass)
		{
			return nullptr;
		}
		WidgetBlueprint->MarkPackageDirty();
		return SaveTargetHudAsset(WidgetBlueprint) ? WidgetBlueprint : nullptr;
	}

	ACFTargetSelectContractActor* SpawnHudTarget(UWorld* TestWorld, const FName ActorName, const FVector& Location, const ECFTargetRelation Relation)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Name = ActorName;
		ACFTargetSelectContractActor* TargetActor = TestWorld ? TestWorld->SpawnActor<ACFTargetSelectContractActor>(
			ACFTargetSelectContractActor::StaticClass(), Location, FRotator::ZeroRotator, SpawnParameters) : nullptr;
		if (TargetActor)
		{
			TargetActor->DisplayInfo.TargetId = ActorName;
			TargetActor->DisplayInfo.DisplayName = FText::FromName(ActorName);
			TargetActor->DisplayInfo.TargetCategory = ECFTargetCategory::Vehicle;
			TargetActor->DisplayInfo.Relation = Relation;
			TargetActor->DisplayInfo.InformationLevel = ECFTargetInfoLevel::Identified;
		}
		return TargetActor;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FCFTargetHudTest, "CarFight.TargetSelect.TS_P0_06.TargetHud", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FCFTargetHudTest::RunTest(const FString& Parameters)
{
	(void)Parameters;
	UWidgetBlueprint* WidgetBlueprint = EnsureTargetHudWidgetBlueprint();
	TestNotNull(TEXT("WBP_TargetSelect 생성 또는 로드"), WidgetBlueprint);
	if (!WidgetBlueprint || !WidgetBlueprint->WidgetTree)
	{
		return false;
	}
	TestTrue(TEXT("WBP_TargetSelect 부모 클래스"), WidgetBlueprint->GeneratedClass->IsChildOf(UCFTargetSelectWidget::StaticClass()));
	TestNotNull(TEXT("후보 Root"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("VerticalBox_CandidateRoot")));
	TestNotNull(TEXT("후보 형태"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("Text_CandidateMarker")));
	TestNotNull(TEXT("후보 정보"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("Text_CandidateInfo")));
	TestNotNull(TEXT("선택 Root"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("VerticalBox_SelectedRoot")));
	TestNotNull(TEXT("선택 형태"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("Text_SelectedMarker")));
	TestNotNull(TEXT("선택 정보"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("Text_SelectedInfo")));
	TestNotNull(TEXT("선택 추적 상태"), WidgetBlueprint->WidgetTree->FindWidget(TEXT("Text_SelectedTrackState")));

	UWorld* TestWorld = FAutomationEditorCommonUtils::CreateNewMap();
	TestNotNull(TEXT("Target HUD 테스트 월드"), TestWorld);
	if (!TestWorld)
	{
		return false;
	}
	AddExpectedError(TEXT("VehicleVisualHitCollision: SM_Body component missing on TargetHudVehiclePawn."), EAutomationExpectedErrorFlags::Contains, 1);
	FActorSpawnParameters PawnSpawnParameters;
	PawnSpawnParameters.Name = TEXT("TargetHudVehiclePawn");
	ACFVehiclePawn* VehiclePawn = TestWorld->SpawnActor<ACFVehiclePawn>(ACFVehiclePawn::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, PawnSpawnParameters);
	ACFTargetSelectContractActor* TargetA = SpawnHudTarget(TestWorld, TEXT("HudTargetA"), FVector(3000.0f, 0.0f, 100.0f), ECFTargetRelation::Hostile);
	ACFTargetSelectContractActor* TargetB = SpawnHudTarget(TestWorld, TEXT("HudTargetB"), FVector(4500.0f, 200.0f, 100.0f), ECFTargetRelation::Neutral);
	TestNotNull(TEXT("Target HUD 차량 Pawn"), VehiclePawn);
	TestNotNull(TEXT("Target HUD 대상 A"), TargetA);
	TestNotNull(TEXT("Target HUD 대상 B"), TargetB);
		if (!VehiclePawn || !TargetA || !TargetB)
	{
		return false;
	}
	TestEqual(TEXT("Pawn TargetSelect HUD 클래스 기본 연결"), VehiclePawn->TargetSelectWidgetClass.Get(), WidgetBlueprint->GeneratedClass.Get());
	TestTrue(TEXT("Pawn TargetSelect HUD 기본 표시 허용"), VehiclePawn->bShowTargetSelectHud);
	TestEqual(TEXT("Pawn TargetSelect HUD 기본 ZOrder"), VehiclePawn->TargetSelectHudZOrder, 20);

	UCFTargetSelectComp* TargetSelectComp = VehiclePawn->GetTargetSelectComp();
	UCFTargetSelectWidget* TargetWidget = NewObject<UCFTargetSelectWidget>(GetTransientPackage());
	TestNotNull(TEXT("TargetSelectComp"), TargetSelectComp);
	TestNotNull(TEXT("TargetSelectWidget 테스트 인스턴스"), TargetWidget);
	if (!TargetSelectComp || !TargetWidget)
	{
		return false;
	}
	TargetSelectComp->bAutoRefreshCandidate = false;
	TargetWidget->SetVehiclePawnRef(VehiclePawn);
	const FCFTargetSelectionContext SelectionContext = TargetSelectComp->GetDefaultSelectionContext();

	FCFTargetCandidate CandidateA;
	CandidateA.TargetActor = TargetA;
	CandidateA.TargetWorldLocation = TargetA->GetActorLocation();
	CandidateA.DisplayInfo = TargetA->DisplayInfo;
	CandidateA.WorldDistanceCm = 3000.0f;
	TestTrue(TEXT("후보 A 설정"), TargetSelectComp->SetCurrentCandidate(CandidateA, SelectionContext));
	TestEqual(TEXT("후보 A 캐시"), TargetWidget->GetCachedCandidateActor(), static_cast<AActor*>(TargetA));
	TestTrue(TEXT("후보 A 표시"), TargetWidget->IsCandidateMarkerVisible());
	TestTrue(TEXT("후보 이름"), TargetWidget->GetCandidateInfoText().ToString().Contains(TEXT("HudTargetA")));
	TestTrue(TEXT("후보 거리"), TargetWidget->GetCandidateInfoText().ToString().Contains(TEXT("30 m")));

	TestTrue(TEXT("대상 A 선택"), TargetSelectComp->SetSelectedTarget(TargetA, SelectionContext));
	TestEqual(TEXT("선택 A 캐시"), TargetWidget->GetCachedSelectedActor(), static_cast<AActor*>(TargetA));
	TestFalse(TEXT("동일 후보 중복 표시 금지"), TargetWidget->IsCandidateMarkerVisible());
	TestTrue(TEXT("선택 표시"), TargetWidget->IsSelectedMarkerVisible());
	TestTrue(TEXT("선택 이름"), TargetWidget->GetSelectedInfoText().ToString().Contains(TEXT("HudTargetA")));
	TestTrue(TEXT("적대 관계"), TargetWidget->GetSelectedTrackStateText().ToString().Contains(TEXT("적대")));

	FCFTargetCandidate CandidateB;
	CandidateB.TargetActor = TargetB;
	CandidateB.TargetWorldLocation = TargetB->GetActorLocation();
	CandidateB.DisplayInfo = TargetB->DisplayInfo;
	CandidateB.WorldDistanceCm = 4500.0f;
	TestTrue(TEXT("후보 B 설정"), TargetSelectComp->SetCurrentCandidate(CandidateB, SelectionContext));
	TestEqual(TEXT("후보 B 캐시"), TargetWidget->GetCachedCandidateActor(), static_cast<AActor*>(TargetB));
	TestTrue(TEXT("선택과 다른 후보 함께 표시"), TargetWidget->IsCandidateMarkerVisible());

	TestTrue(TEXT("선택 A 가림"), TargetSelectComp->SetSelectedTargetTrackState(TargetA, ECFTargetTrackState::Occluded));
	TestTrue(TEXT("가림 상태 텍스트"), TargetWidget->GetSelectedTrackStateText().ToString().Contains(TEXT("가림")));
	TestEqual(TEXT("가림 형태"), TargetWidget->GetSelectedMarkerGlyphText().ToString(), FString(TEXT("▧")));

	TargetSelectComp->ClearSelectedTarget(ECFTargetClearReason::Manual);
	TestFalse(TEXT("Manual 해제 뒤 선택 숨김"), TargetWidget->IsSelectedMarkerVisible());
	TestTrue(TEXT("Manual 해제 뒤 후보 유지"), TargetWidget->IsCandidateMarkerVisible());
	TestEqual(TEXT("Manual 해제 뒤 후보 B 캐시"), TargetWidget->GetCachedCandidateActor(), static_cast<AActor*>(TargetB));

	TargetB->Destroy();
	TargetSelectComp->ClearCurrentCandidate();
	TargetWidget->RefreshFromTargetSelect();
	TestNull(TEXT("후보 파괴 뒤 캐시 Null"), TargetWidget->GetCachedCandidateActor());
	TestFalse(TEXT("후보 파괴 뒤 숨김"), TargetWidget->IsCandidateMarkerVisible());

	TargetWidget->SetVehiclePawnRef(nullptr);
	TargetA->Destroy();
	VehiclePawn->Destroy();
	return true;
}

#endif
