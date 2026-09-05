// Copyright (c) CarFight. All Rights Reserved.
// File: CFVehicleBuilderTab.cpp
// Version: v1.38.0
// Date: 2026-09-05
// Description: CF-FQ-046 사용자 정보 구조 + CF-FQ-047 Step 7 durable final commit / Step 8 Progress·Driving Apply·Explicit Recipe Save UI입니다.
// Changelog:
// - v1.38.0: CF-FQ-047 최종 재감사 교정. guarded Undo도 reverted VehicleData→Recipe exact pair durable 저장까지 완료하는 P0-07H 계약으로 USER 확인/성공/partial-failure 문구를 교정하고 Save All/자동 retry 금지를 명시.
// - v1.37.0: VBHAI-P0-07H에서 Step 7 primary action을 `최종 적용 및 저장`으로 전환하고, fresh durable action에 따라 VehicleData Apply/AppliedState repair/exact Target→Recipe 저장 범위를 USER 확인창에 명확히 표시. 공통 Page Shell/scroll 구조는 변경하지 않음.
// - v1.36.1: P0-07E Source 중간검수 교정. progress bar를 현재 phase index가 아니라 완료된 coarse 단계 수 기준으로 계산해 `7/7 결과 정리`가 terminal 성공 전 100%로 보이지 않게 함.
// - v1.36.0: VBHAI-P0-07E에서 existing PowerShell benchmark child에 exact RunId 7단계 progress/elapsed 표시를 연결하고, persistent USER Driving receipt가 fresh Target에 exact binding된 current Recipe package 하나를 USER click으로 명시 저장하는 UX를 추가. Save All/다른 Asset Save/자동 retry는 추가하지 않음.
// - v1.35.0: VBHAI-P0-07B에서 Step 8 PIE Apply 버튼 아래에 stable typed blocker/준비 완료 이유를 항상 표시하고, 버튼 enable과 클릭 시 stable guard가 동일 VM preflight를 사용하도록 통합. benchmark-running/wrong-step은 Tab-local presentation으로 유지.
// - v1.34.0: CF-FQ-047 post-P0-06 중간검수 P1 교정. successful Recipe/Profile/receipt mutation 뒤 refresh warning을 failure가 아니라 USER 복구 안내 + 진단 정보로 표시.
// - v1.33.1: CF-FQ-046×047 통합 UX 교정. 047의 Step 3 unbound Socket/Step 5 Physics receipt 재검증 USER 문구에서 내부 용어를 제거하고 raw identity/provenance는 진단 정보에만 유지.
// - v1.33.0: CF-FQ-047 P0-06. Hardpoint/Mount downstream 변경으로 Physics receipt만 stale한 경우 private Profile unchanged를 preview로 증명하고 USER 승인 뒤 receipt-only revalidation하는 Step 5 action을 추가.
// - v1.32.0: CF-FQ-047 VBHAI-P0-02. Step 3에 Resolver-independent unbound HP_* inventory와 UseHardpoints-only exact existing Socket adoption UX를 추가.
// - v1.31.1: VBIUX-P0-04 마감. Step 7/8 버튼·확인창·상태 문구의 DefinitionApply/ProposalHash/RunId/DefinitionHash/Runtime Catalog 개발자 용어를 기본 UI에서 제거하고 진단 정보로 격리. Step 8 측정/체크리스트 본문에 bounded scroll을 추가.
// - v1.31.0: VBIUX-P0-04. Step 7 Final Review를 USER field descriptor/Before→After/grouped structural diff로 전환하고 Step 8 기술 측정·직접 주행 checklist·데모 차량 목록 상태를 사용자 용어로 정리. raw identity/path/hash는 진단 정보로 격리.
// - v1.30.1: Step 3 Socket 진단 정보 SVerticalBox Slot의 누락된 닫힘 bracket 1개를 복구해 Slate Construct compile 오류를 교정.
// - v1.30.0: VBIUX-P0-03. Step 5에서 AI 제안/승인 제작 프로필 source authority를 분리하고 수치의 뜻·주행 영향을 추가. Step 6 장착 규칙을 Guidance authority와 사용자 용어 중심으로 정리하고 raw backend 상세는 진단 정보로 격리.
// - v1.29.0: VBIUX-P0-02. Step 1~4 상단 목적/행동과 Reference/Mesh/Socket/Layout Level 1을 사용자 문장으로 전환하고, Step 4 전용 배치 패널과 공통 접힌 진단 정보를 추가. raw VM/backend truth와 mutation semantics는 보존.
// - v1.28.0: USER 피드백 반영. Step 5의 상세 영역에서 Evidence hash/Claim/파일 경로/raw AI 문장을 제거하고 엔진·변속기·구동·브레이크·질량의 실제 의미와 수치만 구조화해 표시. raw BuildPhysicsProposalSummary는 backend diagnostic으로만 보존.
// - v1.27.0: USER 피드백 반영. Step 5/6 내부 용어 중심 본문을 기본 화면에서 제거하고 상단 Step 설명/지금 할 일을 초보자용 문장으로 교체. 8개 Step 표시명과 상태 표기를 사용자 용어로 정리하고 정밀 정보는 접힌 기술 상세로 분리. Step 6 전체를 remaining-height SScrollBox로 전환하고 Mount Type/Size/Preset 라벨을 한국어 중심으로 교정.
// - v1.26.0: VRCP-P0-03. USER Driving PASS persistent receipt 성공 뒤 Default Runtime Catalog promotion을 시도하고, fresh membership 상태와 explicit retry를 Step 8에 추가. Catalog 실패는 USER PASS를 rollback하지 않으며 BuilderVM Catalog state를 추가하지 않음.
// - v1.25.1: P0-07 USER UAT 인식 개선. Hardpoint row의 generic `삭제`를 `장착 위치만 삭제`로 명확화하고, Recipe row만 제거되며 Chassis StaticMesh Socket은 유지된다는 안내를 표/tooltip에 직접 노출. 삭제 로직은 변경하지 않음.
// - v1.25.0: P0-07 USER UAT 2차 피드백. Wheel/Hardpoint `추가 후 편집`이 Socket 생성 직후 cached snapshot 때문에 stale 활성화되던 문제를 live UStaticMesh::FindSocket truth로 교정. 상단 Target/Step/State/Summary/지금 할 일을 한 고정 프레임으로 묶고 Step 3 Socket Preparation을 remaining-height scroll panel로 전환.
// - v1.24.1: P0-07 durable USER Driving PASS가 Recipe non-semantic receipt에 기록되며 auto-save하지 않는 실제 경계를 confirmation/success 문구에 명시. DefinitionHash가 acceptance authority이고 RunId는 diagnostic-only임을 표시.
// - v1.24.0: P0-07 UAT 피드백 반영. Wheel/Hardpoint Chassis 편집 버튼을 하나로 통합하고 missing Wheel/Hardpoint Socket을 exact 이름으로 원점 생성→편집하는 explicit no-auto-save UX를 추가. Hardpoint 이름을 read-only copy slot으로 바꾸고 `기타 조건부 Socket`을 실제 의미인 `파괴 FX Socket`으로 명확화.
// - v1.23.1: VMG-P0-04 코드감사에서 Step 6 scope 주석을 교정. 8영역 Guidance/Socket만 read-only이며 Standard Mount 패널은 explicit typed Recipe write/remove를 소유한다는 실제 구현 경계와 일치시킴.
// - v1.23.0: CF-FQ-043 VMG-P0-04 Step 6 Hardpoint별 Standard Mount transient draft(Type/Size/Preset), explicit commit/remove UI와 Recipe refresh sync를 추가. 기존 8영역 Guidance는 read-only diagnosis authority로 유지.
// - v1.22.2: VMG-P0-03 Step 2 pending Chassis/Wheel Mesh open 버튼, optional FR/RL/RR→pending FL fallback, shared Chassis asset edit warning을 추가하고 Step 2/3 StaticMesh editor backend를 공통화.
// - v1.22.1: VMG-P0-03 Legacy/custom SocketName=None row에서 HP_* 이름을 임의 합성하지 않고 stored truth/LocalTransform 보존 상태를 표시하며 삭제 안내도 실제 Socket binding 유무를 구분.
// - v1.22.0: CF-FQ-043 VMG-P0-03 Step 3에 explicit Hardpoint Plan Mode, Standard category add/remove row, exact Socket status/copy, contextual Wheel/Hardpoint editor entry와 shared Chassis warning을 연결.
// - v1.21.0: Explicit New Vehicle mode에서 Browser refresh가 stale current row를 Slate selection으로 복원하지 않도록 row highlight 복원 조건을 방어적으로 제한.
// - v1.20.0: Step 1 일반 naming을 Vehicle ID 한 칸으로 전환하고 deterministic default identity, 실시간 validation, 접힌 Advanced Asset 경로 override와 Mesh Candidate Quick Start label을 연결.
// - v1.19.0: Explicit New Vehicle Step 1에 Blank/optional StaticMesh picker와 공통 Preview→승인→Commit→exact Browser row highlight/adoption reporting을 연결.
// - v1.18.0: 좌측 작업 대상에 '+ 새 차량 만들기'를 추가하고 BuilderVM transient 신규 제작 상태와 연결. pre-refresh Stable Step 8개를 최초 Slate tree에서 즉시 표시.
// - v1.17.0: Step 1에 Existing Reference Evidence complete replacement R1 전용 검토 버튼/dialog을 추가하고 fingerprint/count/mutation boundary/receipt stale 영향을 USER 승인 전에 표시.
// - v1.16.0: Builder-wide Transmission diagnostic에 상향변속 RPM retention을 함께 표시해 기어 간 spacing sanity review를 명확화.
// - v1.15.0: Step 5 Physics Proposal USER review에 TransmissionProposalHash와 기어별 ChangeUpRPM 예상 차속/post-shift RPM diagnostic을 표시. 값은 runtime shift trigger가 아님을 명시.
// - v1.14.0: Resolver fail-closed로 SelectVehicle가 false를 반환해도 이미 전환된 current selection/Recipe 기준으로 Step 2 pending picker를 재동기화해 이전 차량 표시가 남지 않도록 보강.
// - v1.13.0: 차량 선택/refresh/commit 뒤 Step 2 ObjectPicker subtree를 current Recipe pending state로 재생성해 이전 차량 Chassis/Wheel 표시가 남는 stale presentation을 교정.
// - v1.12.0: actual Wagon Final Review에서 28건 Field Diff가 AutoHeight TextBlock으로 무한 확장되어 하단이 잘리는 문제를 교정. Final Review 상세 Diff 영역만 고정 높이 ScrollBox로 제한하고 Apply/Undo/경계 안내는 항상 노출.
// - v1.11.0: Step 4 Ready/NotCaptured를 Step 7 deferred Apply 정상 상태로 표시하고 Next 버튼이 ViewModel의 단일 forward-progress contract를 사용하도록 교정.
// - v1.10.0: USER 피드백에 따라 작업 대상 색상을 identity hash 팔레트에서 관리 상태 의미 기반 고정색으로 전환하고 같은 분류끼리 모아 정렬. 제목은 중립색, 분류 배지만 의미색을 사용.
// - v1.9.0: E2E에서 발견된 Step 2 Wheel Mesh 지정 UX 공백을 StaticMesh object picker + explicit typed AssetIntent Recipe-only 반영 UI로 교정.
// - v1.8.0: USER 피드백에 따라 작업 대상 row를 identity 기반 고정 accent 색+관리 상태 배지로 구분하고 Step 3을 필수·선택 Socket 이름/복사/차체 메시 열기까지 포함하는 실제 소켓 준비 단계로 강화.
// - v1.7.0: VB-P0-09 Step 8 existing VB-P0-08 benchmark를 non-blocking child process로 실행/회수하고 active PIE transient test-drive + exact USER Driving PASS UX를 연결.
// - v1.6.0: VB-P0-09 Step 7 existing ReadBuilderFinalReview R0 전체 Diff review, explicit DefinitionApply, exact guarded Undo UX를 Guided Shell에 연결.
// - v1.5.0: VB-P0-09 Step 6 existing ReadBuilderGameplayGuidance R0 결과를 8영역 completeness/USER Socket guidance/pending diff 요약으로 Guided Shell에 노출.
// - v1.4.0: VB-P0-09 Step 5 Physics Proposal summary, AI PhysicsDraft load, private 4 Profile mutation0 review→explicit USER commit UX를 추가.
// - v1.3.0: VB-P0-09 Step 1 Reference summary, AI ResearchDraft load, Companion mutation0 review→explicit USER commit, local Reference review token UX를 추가.
// - v1.2.0: navigation을 current StepViews.Num() 기반으로 만들고 Step 1 전용 UI를 fixed index가 아닌 stable StepId로 판정.
// - v1.1.0: Step 1 Mesh-only 후보에 기존 two-record Preview→explicit USER approval→commit UI를 연결. Profile/차급/물리 추론과 Save는 계속 0.
// - v1.0.0: 별도 Guided Shell에서 차량 선택 → 8 Step 상태 확인 → Back/Refresh/Next를 제공하고 Advanced Workspace는 보존.
// Migration:
// - v1.38.0부터 Step 7 guarded Undo 확인은 되돌린 VehicleData와 Recipe의 exact pair 저장까지 포함합니다. 저장 partial failure는 이미 성공한 Undo를 숨기거나 자동 rollback/retry하지 않고 USER에게 현재 상태 확인/후속 복구를 안내합니다.
// - v1.37.0 Step 7 버튼은 exact current VehicleData/Recipe 두 package의 durable commit을 명시적으로 승인하며 Save All/다른 Asset Save/자동 retry를 추가하지 않습니다. CF-FQ-046 common Page Shell/scroll owner와 레이아웃은 그대로 유지합니다.
// - v1.36.1 progress bar는 완료된 coarse 단계 비율만 표시합니다. terminal 100% 성공 표시는 기존 result/exit authority가 계속 소유하며 progress sidecar는 성공 판정 authority가 아닙니다.
// - v1.36.0 Step 8 progress는 PowerShell process가 살아 있을 때만 0.5초 bounded polling하며 final result JSON+exit code가 terminal authority입니다. Recipe Save는 current Recipe package 1개 전체의 현재 미저장 변경을 USER 승인 뒤 저장하며 CF-FQ-046 공통 Page Shell/scroll owner는 변경하지 않습니다.
// - v1.35.0 Step 8 readiness는 local Step 8 presentation만 확장합니다. CF-FQ-046 공통 Page Shell/scroll owner, PIE lifecycle, VehicleData/Recipe Save authority는 변경하지 않습니다.
// - v1.33.1은 USER presentation 문자열만 교정하며 CF-FQ-047 Hardpoint integrity, Physics receipt validation/mutation authority와 저장 경계는 변경하지 않습니다.
// - v1.31.0부터 Step 7/8 기본 화면은 typed FinalReview/Benchmark 결과의 USER projection을 사용하고 ProposalHash/RunId/DefinitionHash/raw path는 `진단 정보`에만 남깁니다. Apply/Undo/USER PASS/Catalog authority는 변경하지 않습니다.
// - v1.30.0부터 Step 5는 loaded AI Draft와 current 승인 Profile을 구분해 표시하며 Step 6 요약은 GameplayGuidanceResult authority를 사용합니다. 기존 Profile commit/Mount write/VehicleData Apply authority는 변경하지 않습니다.
// - v1.29.0부터 Step 1~4 기본 화면은 Editor-private presentation helper의 typed truth projection을 사용하고 raw VM Summary/Resolution과 path/hash는 접힌 `진단 정보`로 이동합니다. Step state/Recipe/Apply/Save authority는 변경하지 않습니다.
// - v1.28.0부터 Step 5 기본 상세은 loaded AI Draft의 사용자 판단용 물리값만 표시하고 raw BuildPhysicsProposalSummary는 backend diagnostic으로 보존합니다.
// - v1.27.0부터 Step 5/6의 정확한 진단 정보는 `기술 상세 보기 (진단용)`에 접혀 표시됩니다. 기존 backend/Recipe/VehicleData mutation 계약은 변경하지 않습니다.
// - v1.26.0부터 Step 8 USER Driving PASS 승인 한 번으로 persistent Receipt 기록 후 Runtime Demo Catalog 등록도 시도합니다. Recipe/Catalog는 모두 자동 저장하지 않으며 Catalog 실패가 USER PASS를 취소하지 않습니다.
// - v1.21.0부터 Explicit New Vehicle mode에서는 Browser refresh가 기존 row selection/highlight를 자동 복원하지 않습니다. 새 record adoption 성공 뒤에는 기존 exact row 복원 경로를 그대로 사용합니다.
// - v1.20.0부터 일반 Guided creation은 Vehicle ID가 기본 입력이며 Definition/Recipe package/object 네 칸은 접힌 `고급 Asset 경로 설정` override로 이동합니다. invalid ID는 sanitize하지 않고 fail-closed하며 최종 collision/path/type는 existing Preview authority가 재검증합니다.
// - v1.19.0 Explicit New Vehicle은 P0-02에서 기존 4개 package/name 입력을 임시 재사용하며 Vehicle ID 단일 naming은 P0-03으로 남깁니다. optional Chassis는 사용 중 Mesh도 허용하고 Recipe AssetIntent에만 기록하며 VehicleData Apply/Save는 하지 않습니다.
// - v1.18.0 신규 차량 entry는 UI/BuilderVM transient state만 전환하며 Asset 생성/Save/VehicleData Apply는 수행하지 않습니다. Blank/ChassisMesh 생성 입력은 후속 VBCUX Gate에서 연결합니다.
// - Step 8 benchmark는 existing RunBuilderBench.ps1/VB-P0-08 authority만 사용하며 saved Target을 요구합니다. USER test-drive는 active PIE transient duplicate만 적용하고 Product Asset 자동 Save/Reference threshold 자동 판정은 하지 않습니다.
// - Step 7 Target mutation은 existing ReadBuilderFinalReview → explicit DefinitionApply → ApplyBuilderFinalReview R3만 사용합니다. Undo는 Apply가 발급한 exact guarded token만 사용하며 auto Save/retry는 없습니다.
// - Step 6은 existing ReadBuilderGameplayGuidance R0를 read-only diagnosis로 소비하고 Standard Mount 패널의 explicit typed Recipe write/remove만 추가로 허용합니다. Step 3의 USER explicit `소켓 추가`만 StaticMesh에 exact-name Socket을 원점 생성할 수 있으며 자동 배치/자동 저장은 하지 않습니다. Target VehicleData Apply는 Step 7이 소유합니다.
// - Step 5 persistent mutation은 existing CommitBuilderProfiles typed facade의 explicit USER-approved R1 transaction만 사용합니다. Target VehicleData Apply는 Step 7에 남기고 raw/shared Profile write와 auto Save는 수행하지 않습니다.
// - Step 1 persistent mutation은 existing CreateBuilderCompanions typed facade의 explicit USER-approved R2 transaction만 사용합니다. raw Profile/Evidence write, VehicleData Apply와 auto Save는 수행하지 않습니다.

#include "DataAuthoring/CFVehicleBuilderTab.h"

#include "DataAuthoring/CFVehicleBuilderVM.h"
#include "DataAuthoring/CFVehicleBuilderHardpointIntegrity.h"
#include "CFVehicleBuilderPresent.h"
#include "DataAuthoring/CFVehicleCatalogPromoService.h"
#include "DataAuthoring/CFVehicleRecipeData.h"
#include "DataAuthoring/CFVehicleRefEvidence.h"

#include "CFEquipmentPresetData.h"
#include "AssetRegistry/AssetData.h"
#include "Editor.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshSocket.h"
#include "Framework/Docking/TabManager.h"
#include "HAL/PlatformApplicationMisc.h"
#include "HAL/PlatformTime.h"
#include "Misc/MessageDialog.h"
#include "PropertyCustomizationHelpers.h"
#include "ScopedTransaction.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Notifications/SProgressBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "SCFVehicleBuilderTab"

namespace
{
	// Vehicle Browser row의 관리 유형을 한눈에 구분할 짧은 배지 문구를 반환합니다.
	FString VehicleRowCategoryText(const FCFVehicleListEntry& Entry)
	{
		if (Entry.bMeshOnlyCandidate)
		{
			return TEXT("메시 후보");
		}

		switch (Entry.ManageState)
		{
		case ECFVehicleManageState::Managed: return TEXT("관리 DA");
		case ECFVehicleManageState::PartiallyManaged: return TEXT("부분관리 DA");
		case ECFVehicleManageState::LegacyImported: return TEXT("레거시 DA");
		case ECFVehicleManageState::Unmanaged: return TEXT("미관리 DA");
		default: return TEXT("기타");
		}
	}

	// Vehicle Browser row의 분류가 항상 같은 의미색을 사용하도록 category 기반 accent를 반환합니다.
	FLinearColor VehicleRowAccentColor(const FCFVehicleListEntry& Entry)
	{
		if (Entry.bMeshOnlyCandidate)
		{
			return FLinearColor(0.32f, 0.72f, 1.00f, 1.0f); // 파랑: 아직 VehicleData가 없는 Mesh 후보
		}

		switch (Entry.ManageState)
		{
		case ECFVehicleManageState::Managed:
			return FLinearColor(0.38f, 0.86f, 0.48f, 1.0f); // 초록: 정상 관리 DA
		case ECFVehicleManageState::PartiallyManaged:
			return FLinearColor(0.95f, 0.78f, 0.28f, 1.0f); // 노랑: 일부 관리 계약만 갖춘 DA
		case ECFVehicleManageState::LegacyImported:
			return FLinearColor(1.00f, 0.56f, 0.24f, 1.0f); // 주황: 레거시 import DA
		case ECFVehicleManageState::Unmanaged:
			return FLinearColor(0.96f, 0.38f, 0.34f, 1.0f); // 빨강: 아직 Recipe 관리 밖의 DA
		default:
			return FLinearColor(0.70f, 0.70f, 0.74f, 1.0f); // 회색: 기타/알 수 없음
		}
	}

	// 작업 대상 목록을 관리 상태별로 모으기 위한 stable category rank입니다.
	int32 VehicleRowCategorySortRank(const FCFVehicleListEntry& Entry)
	{
		if (Entry.bMeshOnlyCandidate)
		{
			return 40;
		}

		switch (Entry.ManageState)
		{
		case ECFVehicleManageState::Managed: return 0;
		case ECFVehicleManageState::PartiallyManaged: return 10;
		case ECFVehicleManageState::LegacyImported: return 20;
		case ECFVehicleManageState::Unmanaged: return 30;
		default: return 50;
		}
	}

	// Vehicle Browser row의 가장 읽기 쉬운 primary identity를 반환합니다.
	FString VehicleRowTitle(const FCFVehicleListEntry& Entry)
	{
		if (Entry.bMeshOnlyCandidate)
		{
			return Entry.ChassisMeshPath.GetAssetName();
		}

		return Entry.DefinitionPath.IsValid()
			? Entry.DefinitionPath.GetAssetName()
			: TEXT("<VehicleData 없음>");
	}

	// Vehicle Browser row의 보조 상태를 반환합니다.
	FString VehicleRowSubtitle(const FCFVehicleListEntry& Entry)
	{
		if (Entry.bMeshOnlyCandidate)
		{
			return Entry.ChassisMeshPath.ToString();
		}

		return Entry.RecipePath.IsValid()
			? FString::Printf(TEXT("Recipe | %s"), *Entry.RecipePath.ToString())
			: TEXT("Recipe 없음 | 미관리 VehicleData");
	}
}

// Widget이 닫힐 때 benchmark child process handle만 해제하고 process 자체는 강제 종료하지 않습니다.
SCFVehicleBuilderTab::~SCFVehicleBuilderTab()
{
	if (DrivingBenchmarkProcess.IsValid())
	{
		FPlatformProcess::CloseProc(DrivingBenchmarkProcess);
		DrivingBenchmarkProcess = FProcHandle();
	}
}

// Non-blocking 기술 주행 측정 child process terminal 상태를 polling하고 current Step 8 결과를 fresh 회수합니다.
void SCFVehicleBuilderTab::Tick(
	const FGeometry& AllottedGeometry,
	const double InCurrentTime,
	const float InDeltaTime)
{
	SCompoundWidget::Tick(AllottedGeometry, InCurrentTime, InDeltaTime);

	if (!bDrivingBenchmarkRunning || !DrivingBenchmarkProcess.IsValid())
	{
		return;
	}
	if (FPlatformProcess::IsProcRunning(DrivingBenchmarkProcess))
	{
		PollDrivingBenchmarkProgress(InCurrentTime);
		return;
	}

	// 종료된 exact benchmark run identity를 running state 초기화 전에 보존합니다.
	const FString FinishedRunId = RunningDrivingBenchmarkRunId;
	// PowerShell terminal이 관측된 즉시 running progress/timer authority를 먼저 종료합니다.
	bDrivingBenchmarkRunning = false;
	ResetDrivingBenchmarkProgressState();

	// Terminal child process exit code입니다.
	int32 ExitCode = INDEX_NONE;
	// OS가 exact exit code를 회수했는지 여부입니다.
	const bool bHasExitCode = FPlatformProcess::GetProcReturnCode(DrivingBenchmarkProcess, &ExitCode);
	FPlatformProcess::CloseProc(DrivingBenchmarkProcess);
	DrivingBenchmarkProcess = FProcHandle();
	RunningDrivingBenchmarkRunId.Reset();

	if (!ViewModel.IsValid())
	{
		LastDiagnosticText = FString::Printf(
			TEXT("Benchmark process finished. ExitCode=%d | RunId=%s | ViewModel unavailable"),
			bHasExitCode ? ExitCode : INDEX_NONE,
			*FinishedRunId);
		LastStatusText = LOCTEXT(
			"DrivingBenchmarkNoViewModel",
			"기술 주행 측정은 종료됐지만 결과를 화면에 불러오지 못했습니다. '진단 정보'를 확인하세요.");
		return;
	}

	// Fresh result/current Target binding diagnostic입니다.
	FString RefreshError;
	// Finished run이 current Target의 accepted benchmark로 다시 읽혔는지 여부입니다.
	const bool bRefreshSucceeded = ViewModel->RefreshCurrentState(RefreshError);
	const bool bExactRunMatched = bRefreshSucceeded
		&& ViewModel->HasDrivingBenchmarkResult()
		&& ViewModel->GetDrivingBenchmarkResult().RunId == FinishedRunId;

	if (bHasExitCode && ExitCode == 0 && bExactRunMatched)
	{
		LastDiagnosticText = FString::Printf(TEXT("Benchmark RunId=%s | ExitCode=0 | current binding matched"), *FinishedRunId);
		LastStatusText = LOCTEXT(
			"DrivingBenchmarkCompleteUser",
			"기술 주행 측정이 완료되었습니다. 위 측정값을 확인한 뒤 PIE에서 선택 차량을 직접 주행하세요.");
		return;
	}

	LastDiagnosticText = FString::Printf(
		TEXT("Benchmark result rejected. ExitCode=%d | RunId=%s | Refresh=%s"),
		bHasExitCode ? ExitCode : INDEX_NONE,
		*FinishedRunId,
		RefreshError.IsEmpty() ? TEXT("<no refresh error>") : *RefreshError);
	LastStatusText = LOCTEXT(
		"DrivingBenchmarkRejectedUser",
		"기술 주행 측정 결과를 현재 차량 결과로 인정하지 못했습니다. 현재 상태를 다시 확인하고 '진단 정보'를 확인하세요.");
}

// Slate widget tree와 transient Builder ViewModel을 초기화합니다.
void SCFVehicleBuilderTab::Construct(const FArguments& InArgs)
{
	ViewModel = MakeShared<FCFVehicleBuilderVM>();
	// Step 2 FL/FR/RL/RR pending Wheel Mesh picker storage를 고정 4-role로 초기화합니다.
	PendingWheelMeshPaths.SetNum(4);
	LastStatusText = LOCTEXT("InitialStatus", "대상 목록을 읽는 중입니다.");

	ChildSlot
	[
		SNew(SBorder)
		.Padding(12.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 10.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("Title", "CarFight 차량 제작 가이드"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 0.0f, 0.0f, 12.0f)
			[
				SNew(STextBlock)
				.Text(LOCTEXT(
					"Subtitle",
					"차량 기준 정보 → Mesh → Socket → 차량 배치 → 물리 설정 → 게임플레이 설정 → 최종 검토 → 주행 테스트 순서로 진행합니다. "
					"Socket 위치는 사용자가 직접 확인하며 Builder는 자동 저장하지 않습니다."))
				.AutoWrapText(true)
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			[
				SNew(SSplitter)

				+ SSplitter::Slot()
				.Value(0.27f)
				[
					SNew(SVerticalBox)

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 6.0f, 6.0f)
					[
						SNew(STextBlock)
							.Text(LOCTEXT("VehicleListHeader", "1. 작업 대상"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 6.0f, 8.0f)
					[
						SNew(SButton)
							.Text(LOCTEXT("BeginNewVehicle", "+ 새 차량 만들기"))
							.ToolTipText(LOCTEXT("BeginNewVehicleTooltip", "기존 목록 선택과 독립적인 신규 차량 제작 모드로 들어갑니다. 이 버튼만으로 Asset을 생성하거나 저장/적용하지 않습니다."))
							.OnClicked(this, &SCFVehicleBuilderTab::HandleBeginNewVehicleEntry)
					]

					+ SVerticalBox::Slot()
					.FillHeight(0.45f)
					.Padding(0.0f, 0.0f, 6.0f, 10.0f)
					[
						SAssignNew(VehicleListView, SListView<FVehicleRowPtr>)
							.ListItemsSource(&VehicleRows)
							.OnGenerateRow(this, &SCFVehicleBuilderTab::HandleGenerateVehicleRow)
							.OnSelectionChanged(this, &SCFVehicleBuilderTab::HandleVehicleSelectionChanged)
							.SelectionMode(ESelectionMode::Single)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 6.0f, 10.0f)
					[
						SNew(SButton)
							.Text(LOCTEXT("RefreshVehicles", "대상 목록 새로고침"))
							.OnClicked(this, &SCFVehicleBuilderTab::HandleRefreshVehicles)
					]

					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(0.0f, 0.0f, 6.0f, 6.0f)
					[
						SNew(STextBlock)
							.Text(LOCTEXT("StepHeader", "2. 제작 단계"))
							.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
					]

					+ SVerticalBox::Slot()
					.FillHeight(0.55f)
					.Padding(0.0f, 0.0f, 6.0f, 0.0f)
					[
						BuildStepNavigation()
					]
				]

				+ SSplitter::Slot()
				.Value(0.73f)
				[
					SNew(SBorder)
					.Padding(14.0f)
					[
						SNew(SVerticalBox)

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 0.0f, 0.0f, 10.0f)
						[
							// USER가 표시한 빨간 영역: Target / 현재 Step / State / Summary / 지금 할 일을 한 프레임으로 고정합니다.
							SNew(SBorder)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 4.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetSelectionText)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetCurrentStepTitle)
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 16))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetCurrentStepStateText)
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetCurrentStepSummaryText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SBorder)
									.Padding(10.0f)
									[
										SNew(SVerticalBox)

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(STextBlock)
												.Text(LOCTEXT("NextActionHeader", "지금 할 일"))
												.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(0.0f, 6.0f, 0.0f, 0.0f)
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetCurrentStepResolutionText)
												.AutoWrapText(true)
										]
									]
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 12.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetMeshCreationVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("MeshCreateHeader", "새 차량 데이터 만들기"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(SBorder)
										.Visibility(this, &SCFVehicleBuilderTab::GetNewVehicleCreationOptionsVisibility)
										.Padding(10.0f)
										[
											SNew(SVerticalBox)

											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(0.0f, 0.0f, 0.0f, 6.0f)
											[
												SNew(STextBlock)
													.Text(LOCTEXT("NewVehicleStartModeHeader", "신규 차량 시작 방식"))
													.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
											]

											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(0.0f, 0.0f, 0.0f, 8.0f)
											[
												SNew(STextBlock)
													.Text(LOCTEXT("NewVehicleStartModeIntro", "차체 Mesh 없이 시작하거나, 처음부터 사용할 Chassis StaticMesh를 선택할 수 있습니다. 이미 다른 차량이 사용하는 Mesh도 선택할 수 있으며 실제 VehicleData 반영은 뒤 단계에서 다시 검토합니다."))
													.AutoWrapText(true)
											]

											+ SVerticalBox::Slot()
											.AutoHeight()
											.Padding(0.0f, 0.0f, 0.0f, 6.0f)
											[
												SNew(SObjectPropertyEntryBox)
													.AllowedClass(UStaticMesh::StaticClass())
													.ObjectPath(this, &SCFVehicleBuilderTab::GetNewVehicleChassisMeshPath)
													.OnObjectChanged(this, &SCFVehicleBuilderTab::HandleNewVehicleChassisMeshChanged)
													.AllowClear(true)
											]

											+ SVerticalBox::Slot()
											.AutoHeight()
											[
												SNew(STextBlock)
													.Text_Lambda([this]()
													{
														if (!ViewModel.IsValid() || !ViewModel->GetNewVehicleChassisMeshPath().IsValid())
														{
															return LOCTEXT("NewVehicleBlankMode", "현재 시작 방식: Chassis Mesh 없이 시작");
														}
														return FText::FromString(FString::Printf(TEXT("현재 시작 방식: Chassis Mesh 사용 — %s"), *ViewModel->GetNewVehicleChassisMeshPath().GetAssetName()));
													})
													.AutoWrapText(true)
											]
										]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f, 0.0f, 4.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("VehicleIdLabel", "Vehicle ID"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f)
								[
									SAssignNew(NewVehicleIdTextBox, SEditableTextBox)
										.HintText(LOCTEXT("VehicleIdHint", "예: WagonPolice, Wagon_Police, SUV01"))
										.ToolTipText(LOCTEXT("VehicleIdTooltip", "Vehicle ID는 새 차량 데이터의 기본 이름으로 사용됩니다. 영문자, 숫자, _만 사용할 수 있으며 Asset 경로와 이름은 자동으로 제안됩니다."))
										.OnTextChanged(this, &SCFVehicleBuilderTab::HandleVehicleIdTextChanged)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetVehicleIdValidationText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f, 0.0f, 8.0f)
								[
									SNew(SExpandableArea)
										.InitiallyCollapsed(true)
										.AreaTitle(LOCTEXT("AdvancedCreateIdentityTitle", "고급 Asset 경로 설정"))
										.BodyContent()
										[
											SNew(SVerticalBox)
											+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
											[
												SAssignNew(NewDefinitionPackageTextBox, SEditableTextBox)
													.HintText(LOCTEXT("DefinitionPackageHint", "VehicleData package: /Game/.../DA_Vehicle_Name"))
											]
											+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
											[
												SAssignNew(NewDefinitionNameTextBox, SEditableTextBox)
													.HintText(LOCTEXT("DefinitionNameHint", "VehicleData Asset 이름"))
											]
											+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
											[
												SAssignNew(NewRecipePackageTextBox, SEditableTextBox)
													.HintText(LOCTEXT("RecipePackageHint", "Recipe package: /Game/.../DA_Recipe_Name"))
											]
											+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f)
											[
												SAssignNew(NewRecipeNameTextBox, SEditableTextBox)
													.HintText(LOCTEXT("RecipeNameHint", "Recipe Asset 이름"))
											]
										]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SButton)
										.Text(this, &SCFVehicleBuilderTab::GetCreateVehicleButtonText)
										.ToolTipText(LOCTEXT("CreateFromMeshTooltip", "생성할 차량 데이터를 먼저 확인창에서 보여주고, 승인할 때만 새 차량용 Asset을 만듭니다. 자동 저장하지 않습니다."))
										.OnClicked(this, &SCFVehicleBuilderTab::HandleCreateVehicleFromMesh)
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetReferenceFlowVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("ReferenceFlowHeader", "차량 기준 정보 확인"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetReferenceSummaryText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SHorizontalBox)

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("LoadResearchDraft", "AI 차량 정보 불러오기"))
											.ToolTipText(LOCTEXT("LoadResearchDraftTooltip", "AI가 조사한 차량 정보를 불러와 현재 선택한 차량에 맞는 자료인지 확인합니다. 이 버튼만으로 Asset이나 VehicleData는 변경되지 않습니다."))
											.OnClicked(this, &SCFVehicleBuilderTab::HandleLoadResearchDraft)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("ReviewResearchCompanion", "필요한 제작 데이터 생성 검토"))
											.ToolTipText(LOCTEXT("ReviewResearchCompanionTooltip", "차량 기준 자료와 차량 전용 제작 프로필 중 필요한 항목을 먼저 확인창에서 보여줍니다. 승인할 때만 필요한 제작용 Asset을 만들며 VehicleData에는 적용하지 않고 자동 저장하지 않습니다."))
											.OnClicked(this, &SCFVehicleBuilderTab::HandleResearchCompanionReview)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(SButton)
											.Text(LOCTEXT("AcceptReferenceSet", "이 기준 정보로 진행"))
											.ToolTipText(LOCTEXT("AcceptReferenceSetTooltip", "현재 차량 기준 정보가 맞다는 사용자 확인을 기록하고 다음 제작 단계로 진행할 수 있게 합니다. 이 버튼은 차량 물리값이나 VehicleData를 변경하거나 저장하지 않습니다."))
											.OnClicked(this, &SCFVehicleBuilderTab::HandleAcceptReferenceSet)
									]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 8.0f, 0.0f, 0.0f)
								[
									SNew(SButton)
										.Text(LOCTEXT("RefreshReferenceEvidence", "기준 정보 갱신 검토"))
										.ToolTipText(LOCTEXT("RefreshReferenceEvidenceTooltip", "이미 준비된 기준 자료를 새 AI 조사 결과로 갱신할 내용을 먼저 보여줍니다. 승인할 때만 기준 자료 내용이 바뀌며 차량 물리값과 VehicleData는 직접 변경하지 않고 자동 저장하지 않습니다."))
										.OnClicked(this, &SCFVehicleBuilderTab::HandleReferenceEvidenceRefresh)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 10.0f, 0.0f, 0.0f)
								[
									SNew(SExpandableArea)
										.InitiallyCollapsed(true)
										.AreaTitle(LOCTEXT("ReferenceDiagnosticTitle", "진단 정보"))
										.BodyContent()
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetCurrentStepDiagnosticText)
												.AutoWrapText(true)
										]
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetMeshPreparationVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("MeshPreparationHeader", "차체와 바퀴 Mesh 준비"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"MeshPreparationIntro",
											"차체 Mesh와 바퀴 Mesh를 선택합니다. 앞왼쪽 Wheel Mesh는 필수이며, 앞오른쪽/뒤쪽 Mesh를 비워두면 앞왼쪽 Mesh를 재사용합니다. 바퀴의 실제 위치는 다음 Socket 단계에서 직접 맞춥니다."))
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetMeshPreparationSummaryText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 2.0f, 0.0f, 6.0f)
								[
									SAssignNew(MeshPreparationPickerHost, SBox)
									[
										BuildMeshPreparationPickerFields()
									]
								]





								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 10.0f, 0.0f, 8.0f)
								[
									SNew(SButton)
										.Text(LOCTEXT("CommitMeshPreparation", "Mesh 설정 반영"))
										.ToolTipText(LOCTEXT("CommitMeshPreparationTooltip", "현재 선택한 차체와 바퀴 Mesh를 이 차량의 제작 기록에 반영합니다. 실제 VehicleData에는 아직 적용하지 않고 자동 저장하지 않습니다."))
										.OnClicked(this, &SCFVehicleBuilderTab::HandleCommitMeshPreparation)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"MeshPreparationBoundary",
											"반영한 뒤 현재 Mesh가 실제로 읽히는지 다시 확인합니다. 앞오른쪽/뒤쪽 Wheel Mesh를 따로 지정하지 않은 경우에는 앞왼쪽 Wheel Mesh를 계속 재사용합니다."))
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 10.0f, 0.0f, 0.0f)
								[
									SNew(SExpandableArea)
										.InitiallyCollapsed(true)
										.AreaTitle(LOCTEXT("MeshDiagnosticTitle", "진단 정보"))
										.BodyContent()
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetCurrentStepDiagnosticText)
												.AutoWrapText(true)
										]
								]
							]
						]

						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetSocketPreparationVisibility)
							.Padding(12.0f)
							[
								SNew(SScrollBox)

								+ SScrollBox::Slot()
								[
									SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("SocketPreparationHeader", "바퀴와 장비 장착 위치 설정"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"SocketPreparationIntro",
											"Wheel과 장비 장착 위치는 Chassis Socket 이름으로 찾습니다. 아래 이름은 정확히 일치해야 합니다. 누락된 Socket은 '추가 후 편집'으로 만든 뒤 Static Mesh Editor에서 위치·회전·스케일을 직접 맞추고 저장하세요. Builder는 위치를 자동 배치하거나 저장하지 않습니다."))
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetSocketPreparationSummaryText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 6.0f)
								[
									SNew(SButton)
										.Text(LOCTEXT("OpenChassisSocketEditor", "Chassis Socket 편집하기"))
										.ToolTipText(LOCTEXT("OpenChassisSocketEditorTooltip", "Wheel과 Hardpoint 모두 같은 current Chassis StaticMesh의 Socket을 사용합니다. 메시를 열어 위치·회전·Scale을 직접 편집하고 저장하세요. +X Forward / +Y Right / +Z Up 기준입니다."))
										.OnClicked(this, &SCFVehicleBuilderTab::HandleOpenChassisSocketEditor)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("SharedChassisSocketWarning", "주의: Chassis Socket은 차량별 데이터가 아니라 StaticMesh 자산에 저장됩니다. 같은 Chassis Mesh를 사용하는 다른 차량도 이 Socket 위치를 공유합니다."))
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 6.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("RequiredSocketHeader", "필수 Wheel Socket — 4개 모두 필요"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									BuildRequiredWheelSocketRow(0, LOCTEXT("WheelRoleFL", "FL / 앞왼쪽"))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									BuildRequiredWheelSocketRow(1, LOCTEXT("WheelRoleFR", "FR / 앞오른쪽"))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									BuildRequiredWheelSocketRow(2, LOCTEXT("WheelRoleRL", "RL / 뒤왼쪽"))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									BuildRequiredWheelSocketRow(3, LOCTEXT("WheelRoleRR", "RR / 뒤오른쪽"))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 14.0f, 0.0f, 6.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("HardpointPlanHeader", "장비 장착 위치 (Hardpoint)"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SAssignNew(HardpointPlanningHost, SBox)
									[
										BuildHardpointPlanningPanel()
									]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("HardpointAxisGuide", "장비 장착 Socket 축 기준: +X(빨강)=차량 앞/기본 발사 방향, +Y(초록)=오른쪽, +Z(파랑)=위입니다. 장비 방향은 Socket 회전값으로 직접 맞춥니다."))
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 12.0f, 0.0f, 6.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("OptionalSocketHeader", "파괴 FX Socket (선택)"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetOptionalSocketGuideText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SButton)
										.Text(LOCTEXT("CopyOptionalSocketNames", "파괴 FX Socket 이름 복사"))
										.ToolTipText(LOCTEXT("CopyOptionalSocketNamesTooltip", "현재 차량이 사용하는 파괴 FX Socket 이름을 복사합니다. 차량 파괴 효과가 별도 Socket 위치를 사용할 때만 필요합니다."))
										.IsEnabled(this, &SCFVehicleBuilderTab::CanCopyOptionalSocketNames)
										.OnClicked(this, &SCFVehicleBuilderTab::HandleCopyOptionalSocketNames)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"SocketPreparationBoundary",
											"Socket 편집을 마치고 Static Mesh를 직접 저장한 뒤 아래 '현재 상태 다시 확인'을 누르세요. 파괴 FX Socket은 차량이 파괴될 때 효과를 붙일 별도 위치가 필요한 경우에만 사용하면 됩니다."))
										.AutoWrapText(true)
									]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 10.0f, 0.0f, 0.0f)
								[
									SNew(SExpandableArea)
										.InitiallyCollapsed(true)
										.AreaTitle(LOCTEXT("SocketDiagnosticTitle", "진단 정보"))
										.BodyContent()
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetCurrentStepDiagnosticText)
												.AutoWrapText(true)
										]
								]
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetLayoutReviewVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("LayoutReviewHeader", "차량 배치 확인"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetLayoutSummaryText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 4.0f, 0.0f, 0.0f)
								[
									SNew(SExpandableArea)
										.InitiallyCollapsed(true)
										.AreaTitle(LOCTEXT("LayoutDiagnosticTitle", "진단 정보"))
										.BodyContent()
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetCurrentStepDiagnosticText)
												.AutoWrapText(true)
										]
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetPhysicsProposalVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("PhysicsProposalHeader", "AI 물리 설정 검토"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SExpandableArea)
										.InitiallyCollapsed(true)
										.AreaTitle(LOCTEXT("PhysicsProposalTechnicalDetails", "물리 설정 상세 보기"))
										.BodyContent()
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetPhysicsProposalSummaryText)
												.AutoWrapText(true)
										]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SExpandableArea)
										.InitiallyCollapsed(true)
										.AreaTitle(LOCTEXT("PhysicsProposalDiagnosticDetails", "진단 정보"))
										.BodyContent()
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetCurrentStepDiagnosticText)
												.AutoWrapText(true)
										]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(SHorizontalBox)

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("LoadPhysicsProposalDraft", "AI 물리 설정 불러오기"))
											.ToolTipText(LOCTEXT("LoadPhysicsProposalDraftTooltip", "AI가 작성한 물리 설정 초안을 불러와 현재 차량과 맞는지 확인합니다. 이 버튼만으로 차량 데이터는 변경되지 않습니다."))
											.OnClicked(this, &SCFVehicleBuilderTab::HandleLoadPhysicsProposalDraft)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("ReviewPhysicsProposal", "AI 물리 설정 검토 후 반영"))
											.ToolTipText(LOCTEXT("ReviewPhysicsProposalTooltip", "불러온 물리 설정의 변경 내용을 확인창에서 보여줍니다. 승인할 때만 제작용 물리 프로필에 반영되며 실제 VehicleData 반영은 7단계에서 합니다. 자동 저장하지 않습니다."))
											.OnClicked(this, &SCFVehicleBuilderTab::HandlePhysicsProposalReview)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(SButton)
											.Text(LOCTEXT("RefreshPhysicsReceipt", "현재 물리 설정 재검증"))
											.ToolTipText(LOCTEXT("RefreshPhysicsReceiptTooltip", "장비 장착 설정이 바뀌어 차량 전체 상태가 달라졌지만 기존에 승인한 물리값 자체는 변하지 않았을 때 사용합니다. 물리값은 그대로 유지하고 현재 차량 상태에 맞춰 승인 상태만 다시 확인합니다. VehicleData와 물리값은 변경하지 않으며 자동 저장하지 않습니다."))
											.OnClicked(this, &SCFVehicleBuilderTab::HandlePhysicsReceiptRefresh)
									]
								]
							]
						]

						+ SVerticalBox::Slot()
						.FillHeight(1.0f)
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetGameplaySetupVisibility)
							.Padding(12.0f)
							[
								SNew(SScrollBox)

								+ SScrollBox::Slot()
								[
									SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("GameplaySetupHeader", "게임플레이 설정 확인"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SAssignNew(MountPlanningHost, SBox)
									[
										BuildMountPlanningPanel()
									]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetGameplayMountSemanticSummaryText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SExpandableArea)
										.InitiallyCollapsed(true)
										.AreaTitle(LOCTEXT("GameplaySetupTechnicalDetails", "진단 정보"))
										.BodyContent()
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetCurrentStepDiagnosticText)
												.AutoWrapText(true)
										]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"GameplaySetupBoundary",
											"위 장착 규칙은 이 차량의 제작 기록에만 반영됩니다. 실제 VehicleData에 무엇이 바뀌는지는 7단계 '최종 검토'에서 다시 확인한 뒤 적용합니다. 자동 저장하지 않습니다."))
										.AutoWrapText(true)
								]
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetFinalReviewVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("FinalReviewHeader", "최종 변경 확인"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									// Final Review는 수십 개 Field Diff를 표시할 수 있으므로 상세 본문만 bounded scroll 영역으로 제한합니다.
									SNew(SBox)
									.HeightOverride(430.0f)
									[
										SNew(SScrollBox)

										+ SScrollBox::Slot()
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetFinalReviewSummaryText)
												.AutoWrapText(true)
										]
									]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SHorizontalBox)

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("ApplyFinalReview", "최종 적용 및 저장"))
											.ToolTipText(LOCTEXT(
												"ApplyFinalReviewTooltip",
												"현재 최종 검토 결과와 저장 상태를 다시 확인한 뒤 필요한 작업만 수행합니다. 차량 데이터 변경이 있으면 적용하고, 적용 상태 기록이 오래됐으면 복구하며, 현재 선택 차량의 VehicleData와 Recipe만 필요한 범위로 VehicleData → Recipe 순서로 저장합니다. Save All이나 자동 재시도는 하지 않습니다."))
											.IsEnabled(this, &SCFVehicleBuilderTab::CanApplyFinalReview)
											.OnClicked(this, &SCFVehicleBuilderTab::HandleFinalReviewApply)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(SButton)
											.Text(LOCTEXT("UndoFinalReview", "방금 적용한 변경 되돌리기"))
											.ToolTipText(LOCTEXT(
												"UndoFinalReviewTooltip",
												"이 차량 제작 가이드에서 방금 적용한 변경이 아직 가장 최근 작업이고 이후 VehicleData가 다른 경로에서 바뀌지 않았을 때만 되돌립니다. 다른 Unreal 작업을 임의로 되돌리지 않습니다."))
											.IsEnabled(this, &SCFVehicleBuilderTab::CanUndoFinalReview)
											.OnClicked(this, &SCFVehicleBuilderTab::HandleFinalReviewUndo)
									]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SExpandableArea)
										.InitiallyCollapsed(true)
										.AreaTitle(LOCTEXT("FinalReviewDiagnosticTitle", "진단 정보"))
										.BodyContent()
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetCurrentStepDiagnosticText)
												.AutoWrapText(true)
										]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"FinalReviewBoundary",
											"'최종 적용 및 저장'은 현재 선택 차량의 최종 검토 결과를 다시 확인한 뒤 필요한 변경과 저장만 수행합니다. 저장 대상은 현재 VehicleData와 Recipe 두 파일로 제한되며 둘 다 필요하면 VehicleData → Recipe 순서로 저장합니다. Save All과 자동 재시도는 하지 않습니다. 되돌리기는 이 가이드가 방금 만든 적용 transaction에만 한정되며 이미 저장된 파일을 자동으로 과거 상태로 되돌리지는 않습니다."))
										.AutoWrapText(true)
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 8.0f)
						[
							SNew(SBorder)
							.Visibility(this, &SCFVehicleBuilderTab::GetDrivingTestVisibility)
							.Padding(12.0f)
							[
								SNew(SVerticalBox)

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(LOCTEXT("DrivingTestHeader", "주행 테스트"))
										.Font(FCoreStyle::GetDefaultFontStyle("Bold", 11))
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 10.0f)
								[
									SNew(SBox)
									.HeightOverride(380.0f)
									[
										SNew(SScrollBox)
										+ SScrollBox::Slot()
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetDrivingTestSummaryText)
												.AutoWrapText(true)
										]
									]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetRuntimeCatalogPromotionStatusText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SHorizontalBox)

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("RunDrivingBenchmark", "기술 주행 측정 실행"))
											.ToolTipText(LOCTEXT(
												"RunDrivingBenchmarkTooltip",
												"현재 저장된 VehicleData로 가속, 최고속도, 제동, 회전반경 같은 기술 측정값을 자동 계측합니다. 저장되지 않은 변경이 있으면 실행하지 않으며 측정 자체가 차량 데이터를 수정하거나 저장하지 않습니다."))
											.IsEnabled(this, &SCFVehicleBuilderTab::CanRunDrivingBenchmark)
											.OnClicked(this, &SCFVehicleBuilderTab::HandleRunDrivingBenchmark)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									.Padding(0.0f, 0.0f, 6.0f, 0.0f)
									[
										SNew(SButton)
											.Text(LOCTEXT("ApplyDrivingTargetToPIE", "현재 PIE에 선택 차량 적용"))
											.ToolTipText(LOCTEXT(
												"ApplyDrivingTargetToPIETooltip",
												"현재 기술 측정에 사용한 저장된 VehicleData를 플레이 중인 차량에 임시 적용합니다. 원본 VehicleData Asset은 수정하거나 저장하지 않습니다."))
											.IsEnabled(this, &SCFVehicleBuilderTab::CanApplyDrivingTargetToPIE)
											.OnClicked(this, &SCFVehicleBuilderTab::HandleApplyDrivingTargetToPIE)
									]

									+ SHorizontalBox::Slot()
									.AutoWidth()
									[
										SNew(SButton)
											.Text(LOCTEXT("AcceptUserDriving", "주행 테스트 통과"))
											.ToolTipText(LOCTEXT(
												"AcceptUserDrivingTooltip",
												"선택 차량을 PIE에서 직접 주행하고 체크리스트에 이상이 없다고 판단했을 때 현재 차량 상태를 주행 확인 완료로 기록합니다. 완료 뒤 데모 차량 목록 등록도 시도하며 관련 Asset은 자동 저장하지 않습니다."))
											.IsEnabled(this, &SCFVehicleBuilderTab::CanAcceptUserDriving)
											.OnClicked(this, &SCFVehicleBuilderTab::HandleAcceptUserDriving)
									]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SVerticalBox)
										.Visibility(this, &SCFVehicleBuilderTab::GetDrivingBenchmarkProgressVisibility)

										+ SVerticalBox::Slot()
										.AutoHeight()
										.Padding(0.0f, 0.0f, 0.0f, 4.0f)
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetDrivingBenchmarkProgressText)
												.AutoWrapText(true)
										]

										+ SVerticalBox::Slot()
										.AutoHeight()
										[
											SNew(SProgressBar)
												.Percent(this, &SCFVehicleBuilderTab::GetDrivingBenchmarkProgressPercent)
										]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetDrivingApplyReadinessText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 6.0f)
								[
									SNew(STextBlock)
										.Text(this, &SCFVehicleBuilderTab::GetDrivingRecipeSaveStatusText)
										.AutoWrapText(true)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SButton)
										.Text(LOCTEXT("SaveDrivingRecipe", "차량 제작 기록 저장"))
										.ToolTipText(LOCTEXT(
											"SaveDrivingRecipeTooltip",
											"현재 차량의 Recipe Asset 1개에 있는 미저장 변경 전체를 저장합니다. VehicleData, StaticMesh, 데모 차량 목록과 다른 차량 Recipe는 저장하지 않으며 Save All이나 자동 재시도를 하지 않습니다."))
										.IsEnabled(this, &SCFVehicleBuilderTab::CanSaveCurrentRecipe)
										.OnClicked(this, &SCFVehicleBuilderTab::HandleSaveCurrentRecipe)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SButton)
										.Text(LOCTEXT("RetryRuntimeCatalogPromotion", "데모 차량 목록 등록 재시도"))
										.ToolTipText(LOCTEXT(
											"RetryRuntimeCatalogPromotionTooltip",
											"현재 주행 테스트 통과 기록이 여전히 유효할 때만 데모 차량 목록 등록을 다시 시도합니다. 주행 테스트 결과를 다시 기록하거나 취소하지 않으며 목록 Asset은 자동 저장하지 않습니다."))
										.IsEnabled(this, &SCFVehicleBuilderTab::CanRetryRuntimeCatalogPromotion)
										.OnClicked(this, &SCFVehicleBuilderTab::HandleRetryRuntimeCatalogPromotion)
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								.Padding(0.0f, 0.0f, 0.0f, 8.0f)
								[
									SNew(SExpandableArea)
										.InitiallyCollapsed(true)
										.AreaTitle(LOCTEXT("DrivingDiagnosticTitle", "진단 정보"))
										.BodyContent()
										[
											SNew(STextBlock)
												.Text(this, &SCFVehicleBuilderTab::GetCurrentStepDiagnosticText)
												.AutoWrapText(true)
										]
								]

								+ SVerticalBox::Slot()
								.AutoHeight()
								[
									SNew(STextBlock)
										.Text(LOCTEXT(
											"DrivingTestBoundary",
											"기술 주행 측정은 수치를 관측할 뿐 차량의 주행감을 자동으로 합격/불합격 판정하지 않습니다. 최종 통과 여부는 직접 주행한 사용자가 결정합니다. 데모 차량 목록 등록 실패가 주행 테스트 통과 기록을 취소하지 않으며 관련 Asset은 자동 저장하지 않습니다."))
										.AutoWrapText(true)
								]
							]
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						.Padding(0.0f, 4.0f, 0.0f, 6.0f)
						[
							SNew(STextBlock)
								.Text_Lambda([this]() { return LastStatusText; })
								.AutoWrapText(true)
						]

						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(SHorizontalBox)

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
									.Text(LOCTEXT("Previous", "이전"))
									.IsEnabled(this, &SCFVehicleBuilderTab::CanMovePrevious)
									.OnClicked(this, &SCFVehicleBuilderTab::HandlePreviousStep)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
									.Text(LOCTEXT("RefreshCurrent", "현재 상태 다시 확인"))
									.OnClicked(this, &SCFVehicleBuilderTab::HandleRefreshCurrentStep)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							.Padding(0.0f, 0.0f, 6.0f, 0.0f)
							[
								SNew(SButton)
									.Text(LOCTEXT("Next", "다음"))
									.IsEnabled(this, &SCFVehicleBuilderTab::CanMoveNext)
									.OnClicked(this, &SCFVehicleBuilderTab::HandleNextStep)
							]

							+ SHorizontalBox::Slot()
							.FillWidth(1.0f)
							[
								SNew(SSpacer)
							]

							+ SHorizontalBox::Slot()
							.AutoWidth()
							[
								SNew(SButton)
									.Text(LOCTEXT("OpenAdvanced", "고급 차량 데이터 제작 열기"))
									.ToolTipText(LOCTEXT("OpenAdvancedTooltip", "기존 Advanced Workspace를 별도 탭으로 엽니다. Guided Builder 상태를 우회해서 자동 적용하지 않습니다."))
									.OnClicked(this, &SCFVehicleBuilderTab::HandleOpenAdvancedWorkspace)
							]
						]
					]
				]
			]
		]
	];

	HandleRefreshVehicles();
}

// Vehicle Browser cache를 fresh read하고 list row를 다시 만듭니다.
FReply SCFVehicleBuilderTab::HandleRefreshVehicles()
{
	if (!ViewModel.IsValid())
	{
		LastStatusText = LOCTEXT("MissingVM", "Builder ViewModel이 없습니다.");
		return FReply::Handled();
	}

	FString Error;
	if (!ViewModel->RefreshVehicles(Error))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("대상 목록 읽기 실패: %s"), *Error));
		return FReply::Handled();
	}

	VehicleRows.Reset();
	for (const FCFVehicleListEntry& Entry : ViewModel->GetVehicleEntries())
	{
		VehicleRows.Add(MakeShared<FCFVehicleListEntry>(Entry));
	}

	// 같은 관리 분류끼리 모으고, 같은 분류 안에서는 표시 이름으로 정렬해 색상의 의미와 목록 구조를 일치시킵니다.
	VehicleRows.Sort([](const FVehicleRowPtr& Left, const FVehicleRowPtr& Right)
	{
		if (!Left.IsValid() || !Right.IsValid())
		{
			return Left.IsValid();
		}
		const int32 LeftRank = VehicleRowCategorySortRank(*Left);
		const int32 RightRank = VehicleRowCategorySortRank(*Right);
		if (LeftRank != RightRank)
		{
			return LeftRank < RightRank;
		}
		return VehicleRowTitle(*Left) < VehicleRowTitle(*Right);
	});

	if (VehicleListView.IsValid())
	{
		VehicleListView->RequestListRefresh();

		// ViewModel current selection과 같은 fresh Browser row를 다시 찾아 좌측 highlight도 exact identity로 맞춥니다.
		if (!ViewModel->IsNewVehicleEntryActive() && ViewModel->HasSelection())
		{
			// 현재 Builder가 authoritative하게 선택한 exact target입니다.
			const FCFVehicleListEntry& CurrentEntry = ViewModel->GetSelectedEntry();
			// fresh Browser row 중 current target과 exact identity가 같은 row입니다.
			FVehicleRowPtr CurrentRow;
			for (const FVehicleRowPtr& Row : VehicleRows)
			{
				if (!Row.IsValid())
				{
					continue;
				}

				// Managed row는 Definition+Recipe exact pair가 모두 같아야 합니다.
				const bool bSameManagedIdentity = CurrentEntry.DefinitionPath.IsValid()
					&& Row->DefinitionPath == CurrentEntry.DefinitionPath
					&& Row->RecipePath == CurrentEntry.RecipePath;
				// Mesh-only row는 후보 flag와 Chassis exact path가 모두 같아야 합니다.
				const bool bSameMeshCandidate = CurrentEntry.bMeshOnlyCandidate
					&& Row->bMeshOnlyCandidate
					&& Row->ChassisMeshPath == CurrentEntry.ChassisMeshPath;
				if (bSameManagedIdentity || bSameMeshCandidate)
				{
					CurrentRow = Row;
					break;
				}
			}

			if (CurrentRow.IsValid())
			{
				VehicleListView->SetSelection(CurrentRow, ESelectInfo::Direct);
				VehicleListView->RequestScrollIntoView(CurrentRow);
			}
		}
	}

	LastStatusText = FText::FromString(FString::Printf(
		TEXT("대상 %d개를 fresh read했습니다. Product Asset/VehicleData는 변경하지 않았습니다."),
		VehicleRows.Num()));
	return FReply::Handled();
}

// 기존 Browser selection과 독립적으로 신규 차량 제작 진입 상태를 시작합니다.
FReply SCFVehicleBuilderTab::HandleBeginNewVehicleEntry()
{
	if (!ViewModel.IsValid())
	{
		LastStatusText = LOCTEXT("BeginNewVehicleMissingVM", "Builder ViewModel이 없습니다.");
		return FReply::Handled();
	}

	ViewModel->BeginNewVehicleEntry();
	ViewModel->SetNewVehicleBlankStart();
	SyncCreationFieldsFromSelection();
	if (VehicleListView.IsValid())
	{
		VehicleListView->ClearSelection();
	}

	LastStatusText = LOCTEXT(
		"BeginNewVehicleReady",
		"새 차량 제작 모드로 들어왔습니다. 아직 Asset 생성/Save/VehicleData Apply는 수행하지 않았습니다.");
	return FReply::Handled();
}

// 선택 row를 current Builder target으로 전환합니다.
void SCFVehicleBuilderTab::HandleVehicleSelectionChanged(FVehicleRowPtr SelectedItem, const ESelectInfo::Type SelectInfo)
{
	if (!SelectedItem.IsValid() || !ViewModel.IsValid())
	{
		return;
	}

	FString Error;
	if (!ViewModel->SelectVehicle(*SelectedItem, Error))
	{
		// Resolver fail-closed여도 selection/Recipe binding은 current target으로 전환됐을 수 있으므로 이전 차량 picker 표시를 남기지 않습니다.
		SyncCreationFieldsFromSelection();
		SyncMeshPreparationFieldsFromRecipe();
		RefreshMeshPreparationPickerPresentation();
		RefreshHardpointPlanningPresentation();
		SyncMountPlanningDraftsFromRecipe();
		RefreshMountPlanningPresentation();
		LastStatusText = FText::FromString(FString::Printf(TEXT("대상 선택 실패: %s"), *Error));
		return;
	}

	SyncCreationFieldsFromSelection();
	SyncMeshPreparationFieldsFromRecipe();
	RefreshMeshPreparationPickerPresentation();
	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("선택: %s | 현재 Step 상태를 authoritative truth에서 다시 계산했습니다."),
		*VehicleRowTitle(*SelectedItem)));
}

// Browser row 하나의 Slate 표현을 생성합니다.
TSharedRef<ITableRow> SCFVehicleBuilderTab::HandleGenerateVehicleRow(FVehicleRowPtr Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	// 현재 row의 management/candidate 유형을 구분하는 accent 색상입니다.
	const FLinearColor AccentColor = Item.IsValid()
		? VehicleRowAccentColor(*Item)
		: FLinearColor::White;
	// 현재 row 유형을 짧게 표시하는 badge text입니다.
	const FString CategoryText = Item.IsValid()
		? VehicleRowCategoryText(*Item)
		: TEXT("INVALID");

	return SNew(STableRow<FVehicleRowPtr>, OwnerTable)
		.Padding(5.0f)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.Padding(0.0f, 0.0f, 8.0f, 0.0f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(CategoryText))
						.ColorAndOpacity(AccentColor)
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(Item.IsValid() ? VehicleRowTitle(*Item) : TEXT("<invalid>")))
						.ColorAndOpacity(FLinearColor(0.92f, 0.92f, 0.94f, 1.0f))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0.0f, 2.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
					.Text(FText::FromString(Item.IsValid() ? VehicleRowSubtitle(*Item) : FString()))
					.ColorAndOpacity(FLinearColor(0.72f, 0.72f, 0.76f, 1.0f))
					.AutoWrapText(true)
			]
		];
}

// current Step Definition projection을 그대로 사용하는 navigation UI를 생성합니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildStepNavigation()
{
	SAssignNew(StepNavigationBox, SVerticalBox);

	// 현재 ViewModel definition에서 만들어진 Step 개수입니다.
	const int32 StepCount = ViewModel.IsValid() ? ViewModel->GetStepViews().Num() : 0;
	for (int32 StepIndex = 0; StepIndex < StepCount; ++StepIndex)
	{
		StepNavigationBox->AddSlot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(SButton)
				.Text(this, &SCFVehicleBuilderTab::GetStepButtonText, StepIndex)
				.IsEnabled(this, &SCFVehicleBuilderTab::IsStepButtonEnabled, StepIndex)
				.OnClicked(this, &SCFVehicleBuilderTab::HandleSelectStep, StepIndex)
		];
	}

	return StepNavigationBox.ToSharedRef();
}

// Step button을 눌렀을 때 허용된 visible page로 이동합니다.
FReply SCFVehicleBuilderTab::HandleSelectStep(const int32 StepIndex)
{
	if (ViewModel.IsValid() && ViewModel->SelectVisibleStep(StepIndex))
	{
		LastStatusText = FText::FromString(FString::Printf(TEXT("%d단계 화면을 열었습니다."), StepIndex + 1));
	}
	return FReply::Handled();
}

// 이전 Step으로 이동합니다.
FReply SCFVehicleBuilderTab::HandlePreviousStep()
{
	if (ViewModel.IsValid())
	{
		ViewModel->MovePreviousStep();
	}
	return FReply::Handled();
}

// 현재 authoritative truth를 다시 읽고 page 상태를 갱신합니다.
FReply SCFVehicleBuilderTab::HandleRefreshCurrentStep()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	FString Error;
	if (!ViewModel->RefreshCurrentState(Error))
	{
		// Resolve fail-closed여도 current Recipe 자체는 유효할 수 있으므로 stale ObjectPicker 표시를 남기지 않습니다.
		SyncMeshPreparationFieldsFromRecipe();
		RefreshMeshPreparationPickerPresentation();
		RefreshHardpointPlanningPresentation();
		SyncMountPlanningDraftsFromRecipe();
		RefreshMountPlanningPresentation();
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("RefreshFailedUser", "현재 차량 상태를 다시 확인하지 못했습니다. 작업 차량과 Asset 상태를 확인한 뒤 다시 시도하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	SyncMeshPreparationFieldsFromRecipe();
	RefreshMeshPreparationPickerPresentation();
	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastDiagnosticText.Reset();
	LastStatusText = LOCTEXT("RefreshPass", "현재 차량 상태를 다시 확인했습니다. 자동 저장하거나 적용한 내용은 없습니다.");
	return FReply::Handled();
}

// ViewModel forward-progress contract를 만족하는 현재 Step에서 다음 Step으로 이동합니다.
FReply SCFVehicleBuilderTab::HandleNextStep()
{
	if (ViewModel.IsValid() && ViewModel->MoveNextStep())
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("MovedNext", "현재 단계의 필수 조건을 확인하고 다음 단계로 이동했습니다.");
	}
	return FReply::Handled();
}

// Explicit New Vehicle 또는 Mesh-only Quick Start의 VehicleData+Recipe proposal을 검토하고 explicit 승인 뒤 공통 commit/adoption 경로를 실행합니다.
FReply SCFVehicleBuilderTab::HandleCreateVehicleFromMesh()
{
	if (!ViewModel.IsValid()
		|| (!ViewModel->IsNewVehicleEntryActive() && !ViewModel->IsMeshOnlyCandidate())
		|| !NewVehicleIdTextBox.IsValid()
		|| !NewDefinitionPackageTextBox.IsValid() || !NewDefinitionNameTextBox.IsValid()
		|| !NewRecipePackageTextBox.IsValid() || !NewRecipeNameTextBox.IsValid())
	{
		return FReply::Handled();
	}

	// Explicit New Vehicle와 기존 Mesh-only Quick Start를 구분하는 현재 진입 상태입니다.
	const bool bExplicitNewVehicle = ViewModel->IsNewVehicleEntryActive();
	// Preview/build diagnostic입니다.
	FString Error;
	if (!ViewModel->SetVehicleCreationId(NewVehicleIdTextBox->GetText().ToString(), Error))
	{
		VehicleIdValidationText = FText::FromString(Error);
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("VehicleIdInvalidUser", "Vehicle ID를 사용할 수 없습니다. 입력 규칙을 확인한 뒤 다시 시도하세요.");
		return FReply::Handled();
	}

	// Mutation0 two-record creation preview입니다.
	FCFVehicleRecordCreatePreview Preview;
	// 현재 진입 방식에 맞는 canonical Guided create request를 준비했는지 여부입니다.
	const bool bPreviewReady = bExplicitNewVehicle
		? ViewModel->PrepareNewVehicleRecordCreate(
			NewDefinitionPackageTextBox->GetText().ToString(),
			NewDefinitionNameTextBox->GetText().ToString(),
			NewRecipePackageTextBox->GetText().ToString(),
			NewRecipeNameTextBox->GetText().ToString(),
			Preview,
			Error)
		: ViewModel->PrepareSelectedMeshRecordCreate(
			NewDefinitionPackageTextBox->GetText().ToString(),
			NewDefinitionNameTextBox->GetText().ToString(),
			NewRecipePackageTextBox->GetText().ToString(),
			NewRecipeNameTextBox->GetText().ToString(),
			Preview,
			Error);
	if (!bPreviewReady)
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("VehicleCreatePreviewFailedUser", "새 차량 생성 내용을 검토할 수 없습니다. 입력한 Vehicle ID와 Asset 설정을 확인한 뒤 다시 시도하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	// Review dialog에 표시할 optional Chassis Mesh입니다. 비어 있으면 Chassis Mesh 없이 시작합니다.
	const FSoftObjectPath ReviewChassisMeshPath = bExplicitNewVehicle
		? ViewModel->GetNewVehicleChassisMeshPath()
		: ViewModel->GetSelectedEntry().ChassisMeshPath;
	// USER가 실제 생성 범위와 아직 하지 않는 작업만 읽고 승인할 review 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT("새 차량 데이터 생성 검토\n\nVehicle ID: %s\n시작 방식: %s\n차체 Mesh: %s\n\n지금 만드는 것:\n- 차량 기본 데이터\n- 차량 제작 기록\n\n지금 하지 않는 것:\n- 실존 차량 기준 정보 확정\n- AI 물리 설정 생성/반영\n- VehicleData에 Chassis/주행 설정 적용\n- 자동 저장\n\n계속하시겠습니까?"),
		*NewVehicleIdTextBox->GetText().ToString(),
		ReviewChassisMeshPath.IsValid() ? TEXT("Chassis Mesh 사용") : TEXT("Chassis Mesh 없이 시작"),
		ReviewChassisMeshPath.IsValid() ? *ReviewChassisMeshPath.GetAssetName() : TEXT("없음")));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("CreateCancelled", "새 차량 생성을 취소했습니다. 변경된 Asset은 없습니다.");
		return FReply::Handled();
	}

	// Explicit USER approval이 붙은 exact two-record terminal result입니다.
	FCFVehicleRecordCreateResult Result;
	// Record creation 이후 exact Browser/Builder adoption 성공 여부입니다.
	bool bBuilderAdopted = false;
	// Record creation과 분리해서 보고할 post-create adoption diagnostic입니다.
	FString AdoptionError;
	if (!ViewModel->ExecutePreparedNewVehicleRecordCreate(Result, bBuilderAdopted, AdoptionError, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("VehicleCreateCommitFailedUser", "새 차량 데이터를 만들지 못했습니다. 현재 입력과 Asset 상태를 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	// 생성된 Asset은 이미 존재합니다. Browser row copy/highlight를 fresh read하되 adoption 실패를 생성 실패로 오인하지 않습니다.
	HandleRefreshVehicles();
	if (!bBuilderAdopted)
	{
		// 생성 성공 뒤 rollback하지 않고 exact Created record와 adoption 오류를 분리해 표시합니다.
		const FString CreatedDefinitionPath = Result.CreatedDefinition ? FSoftObjectPath(Result.CreatedDefinition).ToString() : TEXT("<null>");
		// 생성 성공 뒤 rollback하지 않고 exact Created Recipe를 표시합니다.
		const FString CreatedRecipePath = Result.CreatedRecipe ? FSoftObjectPath(Result.CreatedRecipe).ToString() : TEXT("<null>");
		LastDiagnosticText = FString::Printf(
			TEXT("%s\nDefinition=%s\nRecipe=%s"),
			*AdoptionError,
			*CreatedDefinitionPath,
			*CreatedRecipePath);
		LastStatusText = LOCTEXT(
			"VehicleCreateAdoptionFailedUser",
			"새 차량 Asset은 만들어졌지만 차량 제작 가이드가 새 차량으로 자동 전환되지 못했습니다. Asset은 삭제하지 않았습니다. 차량 목록을 새로 고친 뒤 생성한 차량을 선택하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	SyncCreationFieldsFromSelection();
	SyncMeshPreparationFieldsFromRecipe();
	RefreshMeshPreparationPickerPresentation();
	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastDiagnosticText = Result.Operation.Message;
	LastStatusText = LOCTEXT(
		"VehicleCreatePassUser",
		"새 차량 데이터를 만들고 차량 제작 가이드의 작업 대상으로 선택했습니다. 자동 저장하거나 VehicleData에 Chassis/주행 설정을 적용하지 않았습니다. 이제 1단계에서 차량 기준 정보를 준비하세요.");
	return FReply::Handled();
}

// Step 1 current Recipe에 exact binding된 AI Research Draft를 Project Saved에서 읽습니다.
FReply SCFVehicleBuilderTab::HandleLoadResearchDraft()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Draft load/binding validation diagnostic입니다.
	FString Error;
	if (!ViewModel->LoadResearchDraft(Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("ResearchDraftLoadFailedUser", "AI 차량 정보를 불러오지 못했습니다. 현재 상태를 다시 확인한 뒤 재시도하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	LastDiagnosticText.Reset();
	LastStatusText = LOCTEXT(
		"ResearchDraftLoaded",
		"AI 차량 정보를 불러왔습니다. 아직 차량 제작 데이터나 VehicleData는 변경하지 않았습니다. 아래 내용을 확인하세요.");
	return FReply::Handled();
}

// Step 1 Evidence + Missing private Profiles의 R2 proposal을 검토하고 explicit USER 승인 뒤 commit합니다.
FReply SCFVehicleBuilderTab::HandleResearchCompanionReview()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Mutation0 Companion proposal입니다.
	FCFBuilderCompanionPreview Preview;
	// Preview/commit diagnostic입니다.
	FString Error;
	if (!ViewModel->PrepareResearchCompanions(Preview, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("ResearchCompanionPreviewFailedUser", "필요한 제작 데이터를 검토할 수 없습니다. 현재 상태를 다시 확인한 뒤 재시도하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	// Preview 시점에 Evidence가 이미 persistent하게 존재했는지 여부입니다.
	const bool bEvidenceAlreadyExists = ViewModel->GetCurrentReferenceEvidencePath().IsValid();
	// Current Recipe의 persistent private Profile binding입니다.
	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	// USER review에 표시할 VehicleBase create 여부입니다.
	const bool bCreateVehicleBase = Recipe && Recipe->ProfileBindings.VehicleBaseProfile.IsNull();
	// USER review에 표시할 Drivetrain create 여부입니다.
	const bool bCreateDrivetrain = Recipe && Recipe->ProfileBindings.DrivetrainProfile.IsNull();
	// USER review에 표시할 Handling create 여부입니다.
	const bool bCreateHandling = Recipe && Recipe->ProfileBindings.HandlingProfile.IsNull();
	// USER review에 표시할 Performance create 여부입니다.
	const bool bCreatePerformance = Recipe && Recipe->ProfileBindings.PerformanceProfile.IsNull();

	// 기존 데이터 보존/신규 생성 여부만 USER에게 보여주는 표시 helper입니다.
	const auto BuildCompanionLine = [](const TCHAR* Label, const bool bCreate)
	{
		return FString::Printf(
			TEXT("%s: %s"),
			Label,
			bCreate ? TEXT("새로 준비") : TEXT("기존 정보 유지"));
	};

	// USER가 실제 생성 범위와 비변경 범위만 이해하고 승인할 review 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT(
			"차량 제작 데이터 생성 검토\n\n"
			"%s\n%s\n%s\n%s\n%s\n\n"
			"승인하면 아직 없는 차량 제작용 데이터만 준비합니다.\n"
			"이미 준비된 데이터는 덮어쓰지 않습니다.\n"
			"VehicleData에는 아직 적용하지 않습니다.\n"
			"자동 저장하지 않습니다.\n\n"
			"계속하시겠습니까?"),
		*BuildCompanionLine(TEXT("차량 기준 자료"), !bEvidenceAlreadyExists),
		*BuildCompanionLine(TEXT("차량 기본 설정"), bCreateVehicleBase),
		*BuildCompanionLine(TEXT("구동계 설정"), bCreateDrivetrain),
		*BuildCompanionLine(TEXT("핸들링 설정"), bCreateHandling),
		*BuildCompanionLine(TEXT("성능 설정"), bCreatePerformance)));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("ResearchCompanionCancelled", "제작 데이터 생성을 취소했습니다. 변경된 내용은 없습니다.");
		return FReply::Handled();
	}

	// Explicit USER-approved R2 terminal result입니다.
	FCFBuilderCompanionResult Result;
	if (!ViewModel->ExecutePreparedResearchCompanions(Result, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("ResearchCompanionCommitFailedUser", "필요한 제작 데이터를 생성하지 못했습니다. 현재 상태를 다시 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	LastDiagnosticText = Result.Operation.Message;
	LastStatusText = LOCTEXT("ResearchCompanionCommitPassUser", "필요한 차량 기준 자료와 차량 전용 제작 프로필 준비가 완료됐습니다. VehicleData에는 아직 반영하지 않았고 자동 저장하지 않았습니다.");
	return FReply::Handled();
}

// Step 1 loaded ResearchDraft로 Existing Reference Evidence complete replacement R1을 검토하고 explicit USER 승인 뒤 commit합니다.
FReply SCFVehicleBuilderTab::HandleReferenceEvidenceRefresh()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Current Evidence + loaded ResearchDraft로 만든 mutation0 R1 preview입니다.
	FCFBuilderEvidenceRefreshPreview Preview;
	// Preview/commit diagnostic입니다.
	FString Error;
	if (!ViewModel->PrepareReferenceEvidenceRefresh(Preview, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("ReferenceRefreshPreviewFailedUser", "기준 정보 갱신 내용을 검토할 수 없습니다. 현재 상태를 다시 확인한 뒤 재시도하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	if (Preview.Operation.Status == ECFAuthoringOpStatus::NoChange)
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("ReferenceEvidenceRefreshNoChange", "불러온 AI 차량 정보와 현재 기준 정보가 같습니다. 갱신할 내용이 없습니다.");
		return FReply::Handled();
	}

	// USER가 기준 정보 갱신의 실제 범위와 후속 행동만 이해하고 승인할 review 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT(
			"차량 기준 정보 갱신 검토\n\n"
			"확정된 기준 항목: %d → %d\n"
			"아직 확인되지 않은 정보: %d → %d\n\n"
			"갱신하면 현재 차량 기준 자료의 조사 내용만 새 정보로 교체됩니다.\n"
			"차량 전용 제작 프로필과 VehicleData는 직접 변경하지 않습니다.\n"
			"기준 정보가 바뀌므로 이 단계의 확인과 이후 AI 물리 설정을 다시 검토해야 합니다.\n"
			"자동 저장하지 않습니다.\n\n"
			"새 AI 차량 정보로 기준 자료를 갱신하시겠습니까?"),
		Preview.CurrentClaimCount,
		Preview.ProspectiveClaimCount,
		Preview.CurrentUnknownFactCount,
		Preview.ProspectiveUnknownFactCount));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("ReferenceEvidenceRefreshCancelled", "기준 정보 갱신을 취소했습니다. 변경된 내용은 없습니다.");
		return FReply::Handled();
	}

	// Explicit USER-approved Evidence Refresh terminal result입니다.
	FCFBuilderEvidenceRefreshResult Result;
	if (!ViewModel->ExecutePreparedEvidenceRefresh(Result, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("ReferenceRefreshCommitFailedUser", "기준 정보를 갱신하지 못했습니다. 현재 상태를 다시 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	LastDiagnosticText = Result.Operation.Message;
	LastStatusText = LOCTEXT("ReferenceRefreshCommitPassUser", "차량 기준 정보를 갱신했습니다. 기준 정보가 바뀌었으므로 이 단계에서 다시 확인한 뒤 새 AI 물리 설정을 검토하세요. VehicleData는 변경하지 않았고 자동 저장하지 않았습니다.");
	return FReply::Handled();
}

// Step 1 current Evidence summary/fingerprint를 USER가 확인한 뒤 local Reference review token을 갱신합니다.
FReply SCFVehicleBuilderTab::HandleAcceptReferenceSet()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	if (!ViewModel->GetCurrentReferenceEvidence())
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("ReferenceAcceptMissingEvidence", "확인할 차량 기준 정보가 아직 없습니다. 먼저 AI 차량 정보를 불러오고 필요한 제작 데이터를 준비하세요.");
		return FReply::Handled();
	}

	// USER가 현재 기준 정보를 실제 차량 제작 기준으로 사용할지 판단할 review 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT(
			"차량 기준 정보 확인\n\n%s\n\n"
			"이 정보가 이 차량을 제작할 기준으로 맞는지 확인해 주세요.\n"
			"이 확인 자체는 차량 설정이나 VehicleData를 변경하지 않고 자동 저장도 하지 않습니다.\n\n"
			"이 기준 정보로 진행하시겠습니까?"),
		*GetReferenceSummaryText().ToString()));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("ReferenceAcceptCancelled", "기준 정보 확인을 취소했습니다. 기존 확인 상태는 변경하지 않았습니다.");
		return FReply::Handled();
	}

	// Local Reference review token update diagnostic입니다.
	FString Error;
	if (!ViewModel->AcceptCurrentReferenceSet(Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("ReferenceAcceptFailedUser", "기준 정보 확인을 완료하지 못했습니다. 현재 상태를 다시 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	LastDiagnosticText.Reset();
	LastStatusText = LOCTEXT(
		"ReferenceAcceptPass",
		"현재 차량 기준 정보를 확인했습니다. 차량 물리값이나 VehicleData를 변경하거나 저장하지 않았습니다. 다음 단계로 진행할 수 있습니다.");
	return FReply::Handled();
}


// Step 2 current Recipe의 Chassis/Wheel Mesh pending 선택값을 typed AssetIntent Recipe-only commit으로 반영합니다.
FReply SCFVehicleBuilderTab::HandleCommitMeshPreparation()
{
	if (!ViewModel.IsValid() || !ViewModel->GetRecipe())
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("MeshPrepMissingRecipe", "현재 차량의 제작 기록을 찾을 수 없습니다. 작업 차량을 다시 선택한 뒤 현재 상태를 다시 확인하세요.");
		return FReply::Handled();
	}

	if (!PendingChassisMeshPath.IsValid())
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("MeshPrepMissingChassis", "차체 Mesh가 필요합니다. 차체 Mesh를 선택한 뒤 다시 반영하세요.");
		return FReply::Handled();
	}
	if (!PendingWheelMeshPaths.IsValidIndex(0) || !PendingWheelMeshPaths[0].IsValid())
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("MeshPrepMissingWheelFL", "앞왼쪽 Wheel Mesh는 필수입니다. 앞왼쪽 Wheel Mesh를 선택한 뒤 다시 반영하세요.");
		return FReply::Handled();
	}

	// Current Recipe의 Socket binding 등 Step 2가 소유하지 않는 AssetIntent 값을 보존한 채 Mesh reference만 교체합니다.
	FCFVehicleAssetIntent AssetIntent = ViewModel->GetRecipe()->AssetIntent;
	AssetIntent.ChassisMesh = TSoftObjectPtr<UStaticMesh>(PendingChassisMeshPath);
	AssetIntent.WheelMeshFL = TSoftObjectPtr<UStaticMesh>(PendingWheelMeshPaths[0]);
	AssetIntent.WheelMeshFR = PendingWheelMeshPaths.IsValidIndex(1)
		? TSoftObjectPtr<UStaticMesh>(PendingWheelMeshPaths[1])
		: TSoftObjectPtr<UStaticMesh>();
	AssetIntent.WheelMeshRL = PendingWheelMeshPaths.IsValidIndex(2)
		? TSoftObjectPtr<UStaticMesh>(PendingWheelMeshPaths[2])
		: TSoftObjectPtr<UStaticMesh>();
	AssetIntent.WheelMeshRR = PendingWheelMeshPaths.IsValidIndex(3)
		? TSoftObjectPtr<UStaticMesh>(PendingWheelMeshPaths[3])
		: TSoftObjectPtr<UStaticMesh>();

	// Existing typed Authoring facade의 Recipe-only terminal result입니다.
	FCFAuthoringOpResult CommitResult;
	// Commit/fresh re-evaluation diagnostic입니다.
	FString Error;
	if (!ViewModel->CommitMeshPreparation(AssetIntent, CommitResult, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("MeshPrepCommitFailedUser", "Mesh 설정을 반영하지 못했습니다. 현재 상태와 선택한 Mesh를 다시 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	SyncMeshPreparationFieldsFromRecipe();
	RefreshMeshPreparationPickerPresentation();
	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastDiagnosticText = CommitResult.Message;
	LastStatusText = LOCTEXT("MeshPrepCommitPassUser", "차체와 바퀴 Mesh 설정을 차량 제작 기록에 반영했습니다. 실제 VehicleData에는 아직 적용하지 않았고 자동 저장하지 않았습니다.");
	return FReply::Handled();
}

// Explicit New Vehicle의 optional Chassis StaticMesh picker 변경을 BuilderVM canonical creation state에 반영합니다.
void SCFVehicleBuilderTab::HandleNewVehicleChassisMeshChanged(const FAssetData& AssetData)
{
	if (!ViewModel.IsValid() || !ViewModel->IsNewVehicleEntryActive())
	{
		return;
	}

	ViewModel->SetNewVehicleChassisMeshPath(
		AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath());
}

// Step 2 Chassis StaticMesh object picker 변경을 pending 값에만 반영합니다.
void SCFVehicleBuilderTab::HandleChassisMeshChanged(const FAssetData& AssetData)
{
	PendingChassisMeshPath = AssetData.IsValid()
		? AssetData.GetSoftObjectPath()
		: FSoftObjectPath();
}

// Step 2 Wheel StaticMesh object picker 변경을 role별 pending 값에만 반영합니다.
void SCFVehicleBuilderTab::HandleWheelMeshChanged(const FAssetData& AssetData, const int32 WheelRoleIndex)
{
	if (!PendingWheelMeshPaths.IsValidIndex(WheelRoleIndex))
	{
		return;
	}

	PendingWheelMeshPaths[WheelRoleIndex] = AssetData.IsValid()
		? AssetData.GetSoftObjectPath()
		: FSoftObjectPath();
}

// Step 2에서 explicit commit 전 pending Chassis StaticMesh를 Asset Editor에서 직접 엽니다.
FReply SCFVehicleBuilderTab::HandleOpenPendingChassisMesh()
{
	return OpenStaticMeshAssetPath(
		PendingChassisMeshPath,
		TEXT("Step 2 pending Chassis Mesh"),
		TEXT("아직 'Mesh 설정 반영' 전의 선택한 Chassis Mesh를 열었습니다. 이 StaticMesh를 직접 편집하면 같은 Mesh를 사용하는 다른 차량에도 영향을 줄 수 있습니다."));
}

// Step 2에서 role별 pending Wheel StaticMesh를 열고 optional role이 비었으면 pending FL fallback을 엽니다.
FReply SCFVehicleBuilderTab::HandleOpenPendingWheelMesh(const int32 WheelRoleIndex)
{
	const FSoftObjectPath EffectiveMeshPath = GetEffectivePendingWheelMeshPath(WheelRoleIndex);
	const bool bUsesFLFallback = WheelRoleIndex > 0
		&& PendingWheelMeshPaths.IsValidIndex(WheelRoleIndex)
		&& !PendingWheelMeshPaths[WheelRoleIndex].IsValid()
		&& PendingWheelMeshPaths.IsValidIndex(0)
		&& PendingWheelMeshPaths[0].IsValid();

	const TCHAR* RoleLabels[] = {TEXT("FL"), TEXT("FR"), TEXT("RL"), TEXT("RR")};
	const FString RoleLabel = WheelRoleIndex >= 0 && WheelRoleIndex < UE_ARRAY_COUNT(RoleLabels)
		? FString(RoleLabels[WheelRoleIndex])
		: FString::Printf(TEXT("Role%d"), WheelRoleIndex);
	const FString CompletionNote = bUsesFLFallback
		? FString::Printf(TEXT("%s Wheel Mesh가 비어 있어 앞왼쪽 Wheel Mesh를 대신 열었습니다. 아직 'Mesh 설정 반영'은 하지 않았습니다."), *RoleLabel)
		: FString::Printf(TEXT("%s Wheel Mesh를 열었습니다. 아직 'Mesh 설정 반영'은 하지 않았습니다."), *RoleLabel);

	return OpenStaticMeshAssetPath(
		EffectiveMeshPath,
		FString::Printf(TEXT("%s Wheel Mesh"), *RoleLabel),
		CompletionNote);
}

// Step 2 pending Chassis Mesh open 버튼 활성 여부입니다.
bool SCFVehicleBuilderTab::CanOpenPendingChassisMesh() const
{
	return PendingChassisMeshPath.IsValid();
}

// Step 2 role별 effective pending Wheel Mesh open 버튼 활성 여부입니다.
bool SCFVehicleBuilderTab::CanOpenPendingWheelMesh(const int32 WheelRoleIndex) const
{
	return GetEffectivePendingWheelMeshPath(WheelRoleIndex).IsValid();
}

// Step 2 optional Wheel fallback 여부를 포함하는 동적 open tooltip입니다.
FText SCFVehicleBuilderTab::GetPendingWheelMeshOpenTooltip(const int32 WheelRoleIndex) const
{
	if (!PendingWheelMeshPaths.IsValidIndex(WheelRoleIndex))
	{
		return LOCTEXT("PendingWheelOpenInvalidRole", "확인할 수 없는 Wheel 위치입니다.");
	}
	if (PendingWheelMeshPaths[WheelRoleIndex].IsValid())
	{
		return LOCTEXT("PendingWheelOpenExact", "현재 선택한 Wheel StaticMesh를 직접 엽니다. 'Mesh 설정 반영' 전이어도 선택한 Mesh를 확인할 수 있습니다.");
	}
	if (WheelRoleIndex > 0 && PendingWheelMeshPaths.IsValidIndex(0) && PendingWheelMeshPaths[0].IsValid())
	{
		return LOCTEXT("PendingWheelOpenFallback", "이 위치의 Wheel Mesh가 비어 있으므로 실제로 재사용될 앞왼쪽 Wheel Mesh를 엽니다.");
	}
	return LOCTEXT("PendingWheelOpenMissing", "열 수 있는 Wheel StaticMesh가 없습니다.");
}

// Step 2 direct role path 또는 optional role의 pending FL fallback path를 반환합니다.
FSoftObjectPath SCFVehicleBuilderTab::GetEffectivePendingWheelMeshPath(const int32 WheelRoleIndex) const
{
	if (!PendingWheelMeshPaths.IsValidIndex(WheelRoleIndex))
	{
		return FSoftObjectPath();
	}
	if (PendingWheelMeshPaths[WheelRoleIndex].IsValid())
	{
		return PendingWheelMeshPaths[WheelRoleIndex];
	}
	if (WheelRoleIndex > 0 && PendingWheelMeshPaths.IsValidIndex(0))
	{
		return PendingWheelMeshPaths[0];
	}
	return FSoftObjectPath();
}

// Step 2 Chassis + FL/FR/RL/RR StaticMesh picker 묶음을 current pending state로 새로 만듭니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildMeshPreparationPickerFields()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(0.0f, 0.0f, 0.0f, 4.0f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox)
				.WidthOverride(150.0f)
				[
					SNew(STextBlock)
						.Text(LOCTEXT("ChassisMeshPickerLabel", "차체 메시 [필수]"))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SObjectPropertyEntryBox)
					.AllowedClass(UStaticMesh::StaticClass())
					.ObjectPath(this, &SCFVehicleBuilderTab::GetPendingChassisMeshPath)
					.OnObjectChanged(this, &SCFVehicleBuilderTab::HandleChassisMeshChanged)
					.AllowClear(false)
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			[
				SNew(SButton)
					.Text(LOCTEXT("OpenPendingChassisMesh", "열기"))
					.ToolTipText(LOCTEXT("OpenPendingChassisMeshTooltip", "현재 선택한 Chassis StaticMesh를 직접 엽니다. 'Mesh 설정 반영' 전이어도 선택한 Mesh를 확인할 수 있습니다."))
					.IsEnabled(this, &SCFVehicleBuilderTab::CanOpenPendingChassisMesh)
					.OnClicked(this, &SCFVehicleBuilderTab::HandleOpenPendingChassisMesh)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.Padding(150.0f, 0.0f, 0.0f, 6.0f)
		[
			SNew(STextBlock)
				.Text(LOCTEXT("Step2SharedChassisWarning", "주의: Chassis StaticMesh 자산을 직접 편집하면 같은 Chassis Mesh를 사용하는 다른 차량에도 Mesh/Socket 변경이 공유됩니다. Builder는 차량별 복사본을 자동 생성하지 않습니다."))
				.AutoWrapText(true)
		]

		+ SVerticalBox::Slot().AutoHeight()[BuildWheelMeshPickerRow(0, LOCTEXT("WheelMeshRoleFL", "FL / 앞왼쪽"), true)]
		+ SVerticalBox::Slot().AutoHeight()[BuildWheelMeshPickerRow(1, LOCTEXT("WheelMeshRoleFR", "FR / 앞오른쪽"), false)]
		+ SVerticalBox::Slot().AutoHeight()[BuildWheelMeshPickerRow(2, LOCTEXT("WheelMeshRoleRL", "RL / 뒤왼쪽"), false)]
		+ SVerticalBox::Slot().AutoHeight()[BuildWheelMeshPickerRow(3, LOCTEXT("WheelMeshRoleRR", "RR / 뒤오른쪽"), false)];
}

// 차량 선택/refresh/commit 뒤 ObjectPicker presentation을 current Recipe pending state로 재생성합니다.
void SCFVehicleBuilderTab::RefreshMeshPreparationPickerPresentation()
{
	if (MeshPreparationPickerHost.IsValid())
	{
		MeshPreparationPickerHost->SetContent(BuildMeshPreparationPickerFields());
	}
}

// Step 2 Wheel Mesh role 한 행의 StaticMesh picker UI를 만듭니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildWheelMeshPickerRow(
	const int32 WheelRoleIndex,
	const FText& RoleLabel,
	const bool bRequired)
{
	// USER-facing 필수/선택 구분을 role label에 붙인 문구입니다.
	const FText RequirementLabel = FText::Format(
		bRequired
			? LOCTEXT("WheelMeshRequiredFormat", "{0} [필수]")
			: LOCTEXT("WheelMeshOptionalFormat", "{0} [선택]"),
		RoleLabel);

	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 2.0f, 8.0f, 2.0f)
		[
			SNew(SBox)
			.WidthOverride(150.0f)
			[
				SNew(STextBlock)
					.Text(RequirementLabel)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
			]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 2.0f, 6.0f, 2.0f)
		[
			SNew(SObjectPropertyEntryBox)
				.AllowedClass(UStaticMesh::StaticClass())
				.ObjectPath(this, &SCFVehicleBuilderTab::GetPendingWheelMeshPath, WheelRoleIndex)
				.OnObjectChanged(this, &SCFVehicleBuilderTab::HandleWheelMeshChanged, WheelRoleIndex)
				.AllowClear(!bRequired)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 2.0f)
		[
			SNew(SButton)
				.Text(LOCTEXT("OpenPendingWheelMesh", "열기"))
				.ToolTipText(this, &SCFVehicleBuilderTab::GetPendingWheelMeshOpenTooltip, WheelRoleIndex)
				.IsEnabled(this, &SCFVehicleBuilderTab::CanOpenPendingWheelMesh, WheelRoleIndex)
				.OnClicked(this, &SCFVehicleBuilderTab::HandleOpenPendingWheelMesh, WheelRoleIndex)
		];
}

// Step 3 Wheel/Hardpoint가 공유하는 current Chassis StaticMesh를 하나의 Socket 편집 진입으로 엽니다.
FReply SCFVehicleBuilderTab::HandleOpenChassisSocketEditor()
{
	return OpenCurrentChassisMeshForSocketEditing();
}

// Step 3 exact missing SocketName을 current Chassis StaticMesh 원점에 explicit 생성하고 Asset Editor를 엽니다.
FReply SCFVehicleBuilderTab::HandleAddChassisSocket(const FName SocketName)
{
	return AddChassisSocketAtOriginAndOpen(SocketName);
}

// Current Chassis StaticMesh를 공통 Socket 편집 문맥으로 엽니다.
FReply SCFVehicleBuilderTab::OpenCurrentChassisMeshForSocketEditing()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	return OpenStaticMeshAssetPath(
		ViewModel->GetCurrentChassisMeshPath(),
		TEXT("Chassis Socket"),
		TEXT("바퀴와 장비 장착 위치는 같은 Chassis StaticMesh의 Socket을 사용합니다. 같은 Chassis Mesh를 쓰는 다른 차량과 Socket 위치를 공유할 수 있으므로 편집 전에 확인하세요. Builder는 자동 배치하거나 저장하지 않습니다."));
}

// Exact missing Socket을 current Chassis에 transaction으로 추가하고 편집기를 여는 공통 backend입니다.
FReply SCFVehicleBuilderTab::AddChassisSocketAtOriginAndOpen(const FName SocketName)
{
	if (!ViewModel.IsValid() || SocketName.IsNone())
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("AddChassisSocketInvalidName", "추가할 Socket 이름이 없습니다. 장착 위치 설정을 다시 확인하세요.");
		return FReply::Handled();
	}

	const FSoftObjectPath MeshPath = ViewModel->GetCurrentChassisMeshPath();
	UObject* MeshObject = MeshPath.ResolveObject();
	if (!MeshObject)
	{
		MeshObject = MeshPath.TryLoad();
	}
	UStaticMesh* StaticMesh = Cast<UStaticMesh>(MeshObject);
	if (!StaticMesh)
	{
		LastDiagnosticText = MeshPath.ToString();
		LastStatusText = LOCTEXT("AddChassisSocketMissingMeshUser", "Socket을 추가할 Chassis StaticMesh를 열 수 없습니다. 현재 차체 Mesh 설정을 다시 확인하세요. 자세한 경로는 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	// Exact Socket이 이미 있으면 중복 생성하지 않고 편집기만 엽니다.
	if (StaticMesh->FindSocket(SocketName))
	{
		return OpenStaticMeshAssetPath(
			MeshPath,
			TEXT("Chassis Socket"),
			FString::Printf(TEXT("Socket %s는 이미 있습니다. 위치·회전·스케일만 확인하세요."), *SocketName.ToString()));
	}

	FScopedTransaction Transaction(NSLOCTEXT("CarFightDataAuthoring", "AddBuilderChassisSocket", "차량 Builder Chassis Socket 추가"));
	StaticMesh->Modify();
	UStaticMeshSocket* NewSocket = NewObject<UStaticMeshSocket>(StaticMesh, NAME_None, RF_Transactional);
	if (!NewSocket)
	{
		Transaction.Cancel();
		LastDiagnosticText = TEXT("UStaticMeshSocket NewObject failed.");
		LastStatusText = LOCTEXT("AddChassisSocketCreateFailed", "새 Socket을 만들 수 없습니다. 현재 Chassis Mesh 상태를 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	NewSocket->SocketName = SocketName;
	NewSocket->RelativeLocation = FVector::ZeroVector;
	NewSocket->RelativeRotation = FRotator::ZeroRotator;
	NewSocket->RelativeScale = FVector::OneVector;
	StaticMesh->Sockets.Add(NewSocket);
	StaticMesh->MarkPackageDirty();
	StaticMesh->PostEditChange();

	// Socket 생성 뒤 current Builder state를 다시 읽되 refresh 오류는 Socket 생성 자체를 rollback하지 않습니다.
	FString RefreshError;
	ViewModel->RefreshCurrentState(RefreshError);
	RefreshHardpointPlanningPresentation();
	LastDiagnosticText = RefreshError;

	return OpenStaticMeshAssetPath(
		MeshPath,
		TEXT("Chassis Socket"),
		FString::Printf(
			TEXT("Socket %s를 원점에 추가했습니다. 이름은 그대로 유지하고 위치·회전·스케일을 직접 맞춘 뒤 StaticMesh를 저장하세요. 자동 저장하지 않았습니다."),
			*SocketName.ToString()));
}

// Current committed Chassis StaticMesh object의 live FindSocket truth를 반환합니다.
bool SCFVehicleBuilderTab::IsCurrentChassisSocketPresentLive(const FName SocketName) const
{
	if (!ViewModel.IsValid() || SocketName.IsNone())
	{
		return false;
	}

	const FSoftObjectPath MeshPath = ViewModel->GetCurrentChassisMeshPath();
	if (!MeshPath.IsValid())
	{
		return false;
	}

	UObject* MeshObject = MeshPath.ResolveObject();
	if (!MeshObject)
	{
		MeshObject = MeshPath.TryLoad();
	}
	const UStaticMesh* StaticMesh = Cast<UStaticMesh>(MeshObject);
	return StaticMesh && StaticMesh->FindSocket(SocketName) != nullptr;
}

// Exact Socket이 current Chassis에 없을 때만 `추가 후 편집` 버튼을 활성화합니다.
bool SCFVehicleBuilderTab::CanAddCurrentChassisSocket(const FName SocketName) const
{
	return !SocketName.IsNone() && !IsCurrentChassisSocketPresentLive(SocketName);
}

// Step 2/3에서 exact StaticMesh path를 공통 Asset Editor backend로 엽니다.
FReply SCFVehicleBuilderTab::OpenStaticMeshAssetPath(
	const FSoftObjectPath& MeshPath,
	const FString& ContextLabel,
	const FString& CompletionNote)
{
	if (!MeshPath.IsValid())
	{
		LastDiagnosticText = FString::Printf(TEXT("%s: invalid StaticMesh path."), *ContextLabel);
		LastStatusText = FText::FromString(FString::Printf(TEXT("%s를 열 수 없습니다. 현재 Mesh 설정을 다시 확인하세요."), *ContextLabel));
		return FReply::Handled();
	}

	UObject* MeshObject = MeshPath.ResolveObject();
	if (!MeshObject)
	{
		MeshObject = MeshPath.TryLoad();
	}
	UStaticMesh* StaticMesh = Cast<UStaticMesh>(MeshObject);
	if (!StaticMesh)
	{
		LastDiagnosticText = FString::Printf(TEXT("%s | %s"), *ContextLabel, *MeshPath.ToString());
		LastStatusText = FText::FromString(FString::Printf(TEXT("%s StaticMesh를 열 수 없습니다. 현재 Mesh 설정을 다시 확인하세요. 자세한 경로는 '진단 정보'에서 확인할 수 있습니다."), *ContextLabel));
		return FReply::Handled();
	}

	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor
		? GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()
		: nullptr;
	if (!AssetEditorSubsystem)
	{
		LastDiagnosticText = TEXT("UAssetEditorSubsystem unavailable.");
		LastStatusText = LOCTEXT("OpenStaticMeshMissingSubsystem", "Static Mesh Editor를 열 수 없습니다. Editor 상태를 다시 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	AssetEditorSubsystem->OpenEditorForAsset(StaticMesh);
	// 성공 시 USER에게 full object path 대신 Asset 이름만 표시합니다.
	const FString AssetName = MeshPath.GetAssetName();
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("%s를 열었습니다: %s\n%s"),
		*ContextLabel,
		*AssetName,
		*CompletionNote));
	return FReply::Handled();
}

// Step 3 Hardpoint Plan Mode를 explicit USER action으로 기록합니다.
FReply SCFVehicleBuilderTab::HandleSetHardpointPlanMode(const ECFBuilderHardpointPlanMode PlanMode)
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	FCFAuthoringOpResult Result;
	FString Error;
	if (!ViewModel->CommitHardpointPlanMode(PlanMode, Result, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("HardpointPlanChangeFailedUser", "장비 장착 위치 사용 방식을 변경하지 못했습니다. 현재 상태를 다시 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		RefreshHardpointPlanningPresentation();
		return FReply::Handled();
	}

	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastDiagnosticText = Result.Message;
	LastStatusText = LOCTEXT("HardpointPlanChangePassUser", "장비 장착 위치 사용 방식을 차량 제작 기록에 반영했습니다. VehicleData에는 아직 적용하지 않았고 자동 저장하지 않았습니다.");
	return FReply::Handled();
}

// Step 3 Standard physical category에 deterministic stable Hardpoint row를 추가합니다.
FReply SCFVehicleBuilderTab::HandleAddStandardHardpoint(const FName LocationCategory)
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	FCFHardpointIntent CreatedIntent;
	FCFAuthoringOpResult Result;
	FString Error;
	if (!ViewModel->AddStandardHardpoint(LocationCategory, CreatedIntent, Result, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("HardpointAddFailedUser", "장비 장착 위치를 추가하지 못했습니다. 현재 장착 위치 설정을 다시 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		RefreshHardpointPlanningPresentation();
		return FReply::Handled();
	}

	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastDiagnosticText = Result.Message;
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("장비 장착 위치를 추가했습니다. 위치 ID: %s / Socket: %s\nChassis StaticMesh의 Socket은 아직 만들지 않았습니다. 필요하면 '추가 후 편집'을 사용하세요."),
		*CreatedIntent.LocationSlotId.ToString(),
		*CreatedIntent.SocketName.ToString()));
	return FReply::Handled();
}

// Step 3 current Chassis의 canonical existing unbound HP_* Socket을 exact stable identity로 current Recipe Hardpoint에 채택합니다.
FReply SCFVehicleBuilderTab::HandleAdoptExistingHardpointSocket(const FName SocketName)
{
	if (!ViewModel.IsValid() || SocketName.IsNone())
	{
		return FReply::Handled();
	}

	FCFHardpointIntent AdoptedIntent;
	FCFAuthoringOpResult Result;
	FString Error;
	if (!ViewModel->AdoptExistingStandardHardpointSocket(SocketName, AdoptedIntent, Result, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT(
			"HardpointSocketAdoptionFailedUser",
			"기존 Socket을 이 차량의 장비 장착 위치로 연결하지 못했습니다. 현재 장착 위치 사용 방식과 ID 충돌 여부를 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		RefreshHardpointPlanningPresentation();
		return FReply::Handled();
	}

	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	// successful Recipe mutation 뒤 남은 non-fatal refresh warning입니다.
	const FString& PostCommitRefreshWarning = ViewModel->GetLastPostCommitRefreshWarning();
	LastDiagnosticText = PostCommitRefreshWarning.IsEmpty()
		? Result.Message
		: FString::Printf(
			TEXT("%s\n후속 상태 갱신 경고:\n%s"),
			*Result.Message,
			*PostCommitRefreshWarning);
	if (!PostCommitRefreshWarning.IsEmpty())
	{
		LastStatusText = LOCTEXT(
			"HardpointSocketAdoptionRefreshWarningUser",
			"기존 Socket 연결은 차량 제작 기록에 완료됐지만 화면 상태를 다시 읽지 못했습니다. '현재 상태 다시 확인'을 눌러 최신 상태를 확인하세요. VehicleData 적용·자동 저장은 하지 않았습니다.");
		return FReply::Handled();
	}

	LastStatusText = FText::FromString(FString::Printf(
		TEXT("기존 Chassis Socket %s를 이 차량의 장비 장착 위치 %s에 연결했습니다. Socket 위치/회전은 변경하지 않았고 VehicleData 적용·자동 저장도 하지 않았습니다."),
		*AdoptedIntent.SocketName.ToString(),
		*AdoptedIntent.LocationSlotId.ToString()));
	return FReply::Handled();
}

// Step 3 exact Recipe Hardpoint row를 typed no-cascade remove lane으로 제거합니다.
FReply SCFVehicleBuilderTab::HandleRemoveHardpoint(const FName LocationSlotId)
{
	if (!ViewModel.IsValid() || LocationSlotId.IsNone())
	{
		return FReply::Handled();
	}

	// 삭제 전 USER에게 남은 StaticMesh Socket을 정확히 안내할 current Recipe binding입니다.
	FName SocketName = NAME_None;
	if (const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe())
	{
		if (const FCFHardpointIntent* ExistingIntent = Recipe->HardpointIntents.FindByPredicate([LocationSlotId](const FCFHardpointIntent& Intent)
		{
			return Intent.LocationSlotId == LocationSlotId;
		}))
		{
			SocketName = ExistingIntent->SocketName;
		}
	}
	FCFAuthoringOpResult Result;
	FString Error;
	if (!ViewModel->RemoveHardpointIntent(LocationSlotId, Result, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT("HardpointRemoveFailedUser", "장비 장착 위치를 삭제하지 못했습니다. 이 위치를 사용하는 장착 규칙이 남아 있는지 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		RefreshHardpointPlanningPresentation();
		return FReply::Handled();
	}

	RefreshHardpointPlanningPresentation();
	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastDiagnosticText = Result.Message;
	LastStatusText = SocketName.IsNone()
		? FText::FromString(FString::Printf(
			TEXT("장비 장착 위치 %s 설정을 삭제했습니다. Chassis StaticMesh는 변경하지 않았습니다."),
			*LocationSlotId.ToString()))
		: FText::FromString(FString::Printf(
			TEXT("장비 장착 위치 %s 설정을 삭제했습니다. Chassis StaticMesh의 Socket %s는 그대로 유지됩니다."),
			*LocationSlotId.ToString(),
			*SocketName.ToString()));
	return FReply::Handled();
}

// Step 3 Hardpoint row의 exact SocketName을 클립보드에 복사합니다.
FReply SCFVehicleBuilderTab::HandleCopyHardpointSocketName(const FName SocketName)
{
	if (!SocketName.IsNone())
	{
		const FString SocketNameText = SocketName.ToString();
		FPlatformApplicationMisc::ClipboardCopy(*SocketNameText);
		LastStatusText = FText::FromString(FString::Printf(TEXT("장비 장착 Socket 이름을 복사했습니다: %s"), *SocketNameText));
	}
	return FReply::Handled();
}

// Current Recipe/Mode에서 Step 3 Hardpoint planning subtree를 만듭니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildHardpointPlanningPanel()
{
	TSharedRef<SVerticalBox> Panel = SNew(SVerticalBox);
	if (!ViewModel.IsValid() || !ViewModel->GetRecipe())
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock).Text(LOCTEXT("HardpointPlanNoRecipe", "현재 차량의 장비 장착 위치 설정을 읽을 수 없습니다."))
		];
		return Panel;
	}

	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	const ECFBuilderHardpointPlanMode PlanMode = ViewModel->GetHardpointPlanMode();
	const TCHAR* ModeText = TEXT("알 수 없음");
	switch (PlanMode)
	{
	case ECFBuilderHardpointPlanMode::LegacyCompatible: ModeText = TEXT("기존 호환 모드"); break;
	case ECFBuilderHardpointPlanMode::Unspecified: ModeText = TEXT("미결정"); break;
	case ECFBuilderHardpointPlanMode::NoHardpoints: ModeText = TEXT("장착점 없음"); break;
	case ECFBuilderHardpointPlanMode::UseHardpoints: ModeText = TEXT("장착 위치 사용"); break;
	default: break;
	}

	Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
	[
		SNew(STextBlock)
			.Text(FText::FromString(FString::Printf(TEXT("현재 계획: %s | 장비 장착 위치 %d개 / 장착 규칙 %d개"), ModeText, Recipe->HardpointIntents.Num(), Recipe->MountIntents.Num())))
			.AutoWrapText(true)
	];

	if (PlanMode == ECFBuilderHardpointPlanMode::LegacyCompatible)
	{
		Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock)
					.Text(LOCTEXT("LegacyHardpointPlanNotice", "기존 차량은 현재 장비 장착 구조를 그대로 유지합니다. 새 차량 제작 가이드 방식으로 관리하려면 아래 버튼으로 '장착 위치 사용'을 선택하세요."))
					.AutoWrapText(true)
			]
			+ SVerticalBox::Slot().AutoHeight()
			[
				SNew(SButton)
					.Text(LOCTEXT("OptInUseHardpoints", "장착 위치 사용으로 전환"))
					.OnClicked(this, &SCFVehicleBuilderTab::HandleSetHardpointPlanMode, ECFBuilderHardpointPlanMode::UseHardpoints)
			]
		];
	}
	else
	{
		const bool bCanChooseNoHardpoints = Recipe->HardpointIntents.IsEmpty() && Recipe->MountIntents.IsEmpty();
		Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SButton)
					.Text(LOCTEXT("ChooseNoHardpoints", "장착점 없음"))
					.ToolTipText(LOCTEXT("ChooseNoHardpointsTooltip", "장비 장착 위치와 장착 규칙이 모두 비어 있을 때만 이 차량에 장착 위치가 없음을 선택할 수 있습니다. 기존 설정은 자동으로 삭제하지 않습니다."))
					.IsEnabled(bCanChooseNoHardpoints && PlanMode != ECFBuilderHardpointPlanMode::NoHardpoints)
					.OnClicked(this, &SCFVehicleBuilderTab::HandleSetHardpointPlanMode, ECFBuilderHardpointPlanMode::NoHardpoints)
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).Padding(6.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(SButton)
					.Text(LOCTEXT("ChooseUseHardpoints", "장착 위치 사용"))
					.IsEnabled(PlanMode != ECFBuilderHardpointPlanMode::UseHardpoints)
					.OnClicked(this, &SCFVehicleBuilderTab::HandleSetHardpointPlanMode, ECFBuilderHardpointPlanMode::UseHardpoints)
			]
		];
	}

	// Resolver AssetSnapshot과 분리된 current Chassis 전체 Socket inventory에서 USER가 알아야 할 unbound/collision 후보만 표시합니다.
	FCFBuilderChassisSocketInventory SocketInventory;
	FString SocketInventoryError;
	if (ViewModel->ReadCurrentChassisSocketInventory(SocketInventory, SocketInventoryError))
	{
		bool bHasIntegrityRows = false;
		for (const FCFBuilderChassisSocketInventoryEntry& Entry : SocketInventory.Entries)
		{
			if (Entry.Classification == ECFBuilderHardpointSocketClassification::StandardAdoptable
				|| Entry.Classification == ECFBuilderHardpointSocketClassification::NonCanonicalHardpointLike
				|| Entry.Classification == ECFBuilderHardpointSocketClassification::IdentityCollision)
			{
				bHasIntegrityRows = true;
				break;
			}
		}

		if (bHasIntegrityRows)
		{
			Panel->AddSlot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 4.0f)
			[
				SNew(STextBlock)
					.Text(LOCTEXT("UnboundHardpointSocketHeader", "Chassis에는 있지만 현재 차량에 연결되지 않은 장착 Socket"))
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
			];

			Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
			[
				SNew(STextBlock)
					.Text(LOCTEXT(
						"UnboundHardpointSocketGuide",
						"StaticMesh에 Socket이 있는 것만으로는 이 차량에서 장비를 장착할 수 없습니다. 실제 장착 위치로 쓰려면 '장착 위치 사용'을 선택한 뒤 아래 제작 가이드 표준 Socket 이름을 차량의 장비 장착 위치로 연결해야 합니다. 다른 차량이 같은 Chassis를 공유할 수 있으므로 Builder가 자동 연결하지 않습니다."))
					.AutoWrapText(true)
			];

			for (const FCFBuilderChassisSocketInventoryEntry& Entry : SocketInventory.Entries)
			{
				if (Entry.Classification != ECFBuilderHardpointSocketClassification::StandardAdoptable
					&& Entry.Classification != ECFBuilderHardpointSocketClassification::NonCanonicalHardpointLike
					&& Entry.Classification != ECFBuilderHardpointSocketClassification::IdentityCollision)
				{
					continue;
				}

				FString StateText;
				switch (Entry.Classification)
				{
				case ECFBuilderHardpointSocketClassification::StandardAdoptable:
					StateText = PlanMode == ECFBuilderHardpointPlanMode::UseHardpoints
						? FString::Printf(TEXT("현재 미연결 — %s로 연결 가능"), *Entry.ParsedLocationSlotId.ToString())
						: FString::Printf(TEXT("현재 미연결 — 장착 위치 사용으로 전환 후 %s로 연결 가능"), *Entry.ParsedLocationSlotId.ToString());
					break;
				case ECFBuilderHardpointSocketClassification::NonCanonicalHardpointLike:
					StateText = TEXT("HP_ 이름이지만 제작 가이드 표준 이름 형식이 아니어서 자동 연결하지 않습니다.");
					break;
				case ECFBuilderHardpointSocketClassification::IdentityCollision:
					StateText = TEXT("같은 위치 ID가 이미 다른 장착 위치에 사용 중이라 연결할 수 없습니다.");
					break;
				default:
					break;
				}

				TSharedRef<SHorizontalBox> IntegrityRow = SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 1.0f, 8.0f, 1.0f)
					[
						SNew(SBox)
							.WidthOverride(150.0f)
							[
								SNew(STextBlock).Text(FText::FromName(Entry.SocketName))
							]
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(0.0f, 1.0f, 8.0f, 1.0f)
					[
						SNew(STextBlock)
							.Text(FText::FromString(StateText))
							.AutoWrapText(true)
					];

				if (Entry.Classification == ECFBuilderHardpointSocketClassification::StandardAdoptable)
				{
					IntegrityRow->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 1.0f)
					[
						SNew(SButton)
							.Text(LOCTEXT("AdoptExistingHardpointSocket", "이 Socket 사용"))
							.ToolTipText(LOCTEXT("AdoptExistingHardpointSocketTooltip", "현재 Chassis의 기존 Socket 위치/회전을 그대로 두고 이 차량의 장비 장착 위치에 정확한 위치 ID로 연결합니다. StaticMesh/VehicleData를 변경하거나 자동 저장하지 않습니다."))
							.IsEnabled(PlanMode == ECFBuilderHardpointPlanMode::UseHardpoints)
							.OnClicked(this, &SCFVehicleBuilderTab::HandleAdoptExistingHardpointSocket, Entry.SocketName)
					];
				}

				Panel->AddSlot().AutoHeight().Padding(0.0f, 1.0f)[IntegrityRow];
			}
		}
	}
	else if (!SocketInventoryError.IsEmpty())
	{
		Panel->AddSlot().AutoHeight().Padding(0.0f, 6.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock)
				.Text(LOCTEXT("HardpointSocketInventoryUnavailable", "Chassis의 기존 장착 Socket 연결 상태를 확인할 수 없습니다. '현재 상태 다시 확인' 후 다시 확인하세요."))
				.AutoWrapText(true)
		];
	}

	if (PlanMode == ECFBuilderHardpointPlanMode::UseHardpoints)
	{
		Panel->AddSlot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 4.0f)
		[
			SNew(STextBlock)
				.Text(LOCTEXT("StandardHardpointAddGuide", "추가할 장비 위치를 선택하세요. 추가하면 위치 ID와 Socket 이름이 자동으로 정해집니다. 위치 종류를 바꾸려면 해당 장착 위치를 삭제한 뒤 다시 추가합니다."))
				.AutoWrapText(true)
		];

		TSharedRef<SHorizontalBox> CategoryButtons = SNew(SHorizontalBox);
		for (const FName Category : ViewModel->GetStandardHardpointCategories())
		{
			FString Label = Category.ToString();
			if (Category == TEXT("Top")) Label = TEXT("위");
			else if (Category == TEXT("Front")) Label = TEXT("앞");
			else if (Category == TEXT("Back")) Label = TEXT("뒤");
			else if (Category == TEXT("LeftSide")) Label = TEXT("왼쪽");
			else if (Category == TEXT("RightSide")) Label = TEXT("오른쪽");
			else if (Category == TEXT("Bottom")) Label = TEXT("아래");
			else if (Category == TEXT("Internal")) Label = TEXT("내부");

			CategoryButtons->AddSlot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
					.Text(FText::FromString(Label))
					.ToolTipText(FText::FromString(FString::Printf(TEXT("%s 위치에 다음 빈 번호로 장비 장착 위치를 추가합니다."), *Label)))
					.OnClicked(this, &SCFVehicleBuilderTab::HandleAddStandardHardpoint, Category)
			];
		}
		Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)[CategoryButtons];
	}

	if (Recipe->HardpointIntents.IsEmpty())
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
				.Text(PlanMode == ECFBuilderHardpointPlanMode::UseHardpoints
					? LOCTEXT("HardpointRowsEmptyUse", "아직 장착 위치가 없습니다. 하나 이상 추가해야 Step 3을 완료할 수 있습니다.")
					: LOCTEXT("HardpointRowsEmpty", "현재 장비 장착 위치가 없습니다."))
				.AutoWrapText(true)
		];
		return Panel;
	}

	Panel->AddSlot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 2.0f)
	[
		SNew(STextBlock).Text(LOCTEXT("HardpointRowsHeader", "현재 장비 장착 위치"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
	];

	Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
	[
		SNew(STextBlock)
			.Text(LOCTEXT("HardpointDeleteBoundary", "삭제는 이 차량의 장착 위치 설정만 제거합니다. Chassis StaticMesh의 Socket은 그대로 유지됩니다."))
			.AutoWrapText(true)
	];

	for (const FCFHardpointIntent& Intent : Recipe->HardpointIntents)
	{
		// UI는 creation-time suggestion을 다시 추론하지 않고 Recipe에 실제 저장된 SocketName truth만 표시합니다.
		const FName StoredSocketName = Intent.SocketName;
		const FString SocketDisplayText = StoredSocketName.IsNone() ? TEXT("(없음)") : StoredSocketName.ToString();
		const bool bCanDelete = PlanMode != ECFBuilderHardpointPlanMode::LegacyCompatible;

		Panel->AddSlot().AutoHeight().Padding(0.0f, 1.0f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox).WidthOverride(74.0f)[SNew(STextBlock).Text(FText::FromName(Intent.LocationCategory))]
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SBox).WidthOverride(110.0f)[SNew(STextBlock).Text(FText::FromName(Intent.LocationSlotId))]
			]
			+ SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(SEditableTextBox)
					.Text(FText::FromString(SocketDisplayText))
					.IsReadOnly(true)
					.ToolTipText(LOCTEXT("HardpointSocketExactNameTooltip", "이 문자열이 StaticMesh SocketName과 한 글자까지 정확히 일치해야 합니다."))
			]
			+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
			[
				SNew(STextBlock)
					.Text_Lambda([this, StoredSocketName, PlanMode]()
					{
						if (StoredSocketName.IsNone())
						{
							return FText::FromString(PlanMode == ECFBuilderHardpointPlanMode::LegacyCompatible
								? TEXT("기존 저장 위치 사용")
								: TEXT("Socket 이름 없음 — 확인 필요"));
						}
						return IsCurrentChassisSocketPresentLive(StoredSocketName)
							? FText::FromString(TEXT("현재 있음"))
							: FText::FromString(TEXT("없음 — 추가 필요"));
					})
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
					.Text(LOCTEXT("AddHardpointSocketAndEdit", "추가 후 편집"))
					.ToolTipText(LOCTEXT("AddHardpointSocketAndEditTooltip", "현재 Chassis에 이 이름의 Socket이 없으면 원점에 새로 만들고 Static Mesh Editor를 엽니다. 이름은 한 글자까지 정확히 유지하고 위치·회전·스케일은 직접 맞춘 뒤 저장하세요."))
					.IsEnabled_Lambda([this, StoredSocketName]() { return CanAddCurrentChassisSocket(StoredSocketName); })
					.OnClicked(this, &SCFVehicleBuilderTab::HandleAddChassisSocket, StoredSocketName)
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)
			[
				SNew(SButton)
					.Text(LOCTEXT("CopyHardpointSocket", "복사"))
					.IsEnabled(!StoredSocketName.IsNone())
					.OnClicked(this, &SCFVehicleBuilderTab::HandleCopyHardpointSocketName, StoredSocketName)
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton)
					.Text(LOCTEXT("RemoveHardpointRow", "장착 위치만 삭제"))
					.ToolTipText(LOCTEXT("RemoveHardpointRowTooltip", "이 차량의 장비 장착 위치 설정만 삭제합니다. Chassis StaticMesh의 Socket은 그대로 유지됩니다. 이 위치를 사용하는 장착 규칙이 남아 있으면 삭제되지 않습니다."))
					.IsEnabled(bCanDelete && !Intent.LocationSlotId.IsNone())
					.OnClicked(this, &SCFVehicleBuilderTab::HandleRemoveHardpoint, Intent.LocationSlotId)
			]
		];
	}

	return Panel;
}

// Mode/add/remove/selection/refresh 뒤 Hardpoint planning subtree를 current truth로 교체합니다.
void SCFVehicleBuilderTab::RefreshHardpointPlanningPresentation()
{
	if (HardpointPlanningHost.IsValid())
	{
		HardpointPlanningHost->SetContent(BuildHardpointPlanningPanel());
	}
}

// Step 3 required Wheel Socket 한 개의 exact current 이름을 클립보드에 복사합니다.
FReply SCFVehicleBuilderTab::HandleCopyRequiredWheelSocketName(const int32 WheelRoleIndex)
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Current Recipe binding을 반영한 required Wheel Socket 4종입니다.
	const TArray<FName> RequiredSocketNames = ViewModel->GetRequiredWheelSocketNames();
	if (!RequiredSocketNames.IsValidIndex(WheelRoleIndex))
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("CopyRequiredSocketInvalidIndex", "복사할 Wheel Socket 위치를 찾을 수 없습니다.");
		return FReply::Handled();
	}

	// Clipboard에 기록할 exact Socket 이름입니다.
	const FString SocketNameText = RequiredSocketNames[WheelRoleIndex].ToString();
	FPlatformApplicationMisc::ClipboardCopy(*SocketNameText);
	LastStatusText = FText::FromString(FString::Printf(TEXT("Socket 이름을 복사했습니다: %s"), *SocketNameText));
	return FReply::Handled();
}

// Step 3 current Recipe의 optional Gameplay Socket 이름 전체를 줄바꿈 목록으로 복사합니다.
FReply SCFVehicleBuilderTab::HandleCopyOptionalSocketNames()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Hardpoint 표와 중복되지 않는 current Recipe의 파괴 FX Socket 이름입니다.
	const TArray<FName> OptionalSocketNames = ViewModel->GetConditionalNonHardpointSocketNames();
	if (OptionalSocketNames.IsEmpty())
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("CopyOptionalSocketEmpty", "현재 차량에는 별도로 준비할 파괴 FX Socket 이름이 없습니다.");
		return FReply::Handled();
	}

	// Clipboard에 복사할 줄바꿈 exact Socket name 목록입니다.
	TArray<FString> OptionalSocketNameTexts;
	OptionalSocketNameTexts.Reserve(OptionalSocketNames.Num());
	for (const FName SocketName : OptionalSocketNames)
	{
		OptionalSocketNameTexts.Add(SocketName.ToString());
	}

	// 여러 이름을 한 번에 붙여넣기 쉬운 줄바꿈 텍스트입니다.
	const FString ClipboardText = FString::Join(OptionalSocketNameTexts, LINE_TERMINATOR);
	FPlatformApplicationMisc::ClipboardCopy(*ClipboardText);
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("파괴 FX Socket 이름 %d개를 줄바꿈 목록으로 복사했습니다."),
		OptionalSocketNames.Num()));
	return FReply::Handled();
}

// Step 3 required Wheel Socket 한 행의 역할/이름/현재 상태/추가/복사 UI를 만듭니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildRequiredWheelSocketRow(const int32 WheelRoleIndex, const FText& RoleLabel)
{
	const TArray<FName> RequiredSocketNames = ViewModel.IsValid() ? ViewModel->GetRequiredWheelSocketNames() : TArray<FName>();
	const FName SocketName = RequiredSocketNames.IsValidIndex(WheelRoleIndex) ? RequiredSocketNames[WheelRoleIndex] : NAME_None;

	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 2.0f, 8.0f, 2.0f)
		[
			SNew(SBox)
			.WidthOverride(100.0f)
			[
				SNew(STextBlock)
					.Text(RoleLabel)
					.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
			]
		]

		+ SHorizontalBox::Slot()
		.FillWidth(1.0f)
		.Padding(0.0f, 2.0f, 8.0f, 2.0f)
		[
			SNew(SEditableTextBox)
				.Text(this, &SCFVehicleBuilderTab::GetRequiredWheelSocketNameText, WheelRoleIndex)
				.IsReadOnly(true)
				.ToolTipText(LOCTEXT("WheelSocketExactNameTooltip", "이 문자열이 StaticMesh SocketName과 한 글자까지 정확히 일치해야 합니다."))
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		.Padding(0.0f, 2.0f, 8.0f, 2.0f)
		[
			SNew(STextBlock)
				.Text(this, &SCFVehicleBuilderTab::GetRequiredWheelSocketStatusText, WheelRoleIndex)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 2.0f, 4.0f, 2.0f)
		[
			SNew(SButton)
				.Text(LOCTEXT("AddWheelSocketAndEdit", "추가 후 편집"))
				.ToolTipText(LOCTEXT("AddWheelSocketAndEditTooltip", "현재 Chassis에 이 이름의 Wheel Socket이 없으면 원점에 만들고 Static Mesh Editor를 엽니다. 이름은 그대로 유지하고 위치·회전·스케일은 직접 맞춘 뒤 저장하세요."))
				.IsEnabled_Lambda([this, SocketName]() { return CanAddCurrentChassisSocket(SocketName); })
				.OnClicked(this, &SCFVehicleBuilderTab::HandleAddChassisSocket, SocketName)
		]

		+ SHorizontalBox::Slot()
		.AutoWidth()
		.Padding(0.0f, 2.0f)
		[
			SNew(SButton)
				.Text(LOCTEXT("CopyRequiredSocketName", "복사"))
				.OnClicked(this, &SCFVehicleBuilderTab::HandleCopyRequiredWheelSocketName, WheelRoleIndex)
		];
}

// Step 3 소켓 준비 전용 UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetSocketPreparationVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::SocketGuide
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 3 required Wheel Socket exact 이름을 표시합니다.
FText SCFVehicleBuilderTab::GetRequiredWheelSocketNameText(const int32 WheelRoleIndex) const
{
	if (!ViewModel.IsValid())
	{
		return FText::GetEmpty();
	}

	// Current Recipe binding을 반영한 required Socket 목록입니다.
	const TArray<FName> RequiredSocketNames = ViewModel->GetRequiredWheelSocketNames();
	return RequiredSocketNames.IsValidIndex(WheelRoleIndex)
		? FText::FromName(RequiredSocketNames[WheelRoleIndex])
		: FText::FromString(TEXT("-"));
}

// Step 3 required Wheel Socket이 current Chassis에 존재하는지 표시합니다.
FText SCFVehicleBuilderTab::GetRequiredWheelSocketStatusText(const int32 WheelRoleIndex) const
{
	if (!ViewModel.IsValid())
	{
		return FText::GetEmpty();
	}

	// Current Recipe binding을 반영한 required Socket 목록입니다.
	const TArray<FName> RequiredSocketNames = ViewModel->GetRequiredWheelSocketNames();
	if (!RequiredSocketNames.IsValidIndex(WheelRoleIndex))
	{
		return FText::FromString(TEXT("확인 불가"));
	}

	return IsCurrentChassisSocketPresentLive(RequiredSocketNames[WheelRoleIndex])
		? FText::FromString(TEXT("현재 있음"))
		: FText::FromString(TEXT("없음 — 추가 필요"));
}

// Step 3 current Recipe가 별도 위치를 요구하는 파괴 FX Socket 상태를 표시합니다.
FText SCFVehicleBuilderTab::GetOptionalSocketGuideText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::GetEmpty();
	}

	// Hardpoint와 중복되지 않는 current Recipe의 non-Hardpoint Socket은 현재 계약상 Destroyed FX Socket입니다.
	const TArray<FName> OptionalSocketNames = ViewModel->GetConditionalNonHardpointSocketNames();
	if (OptionalSocketNames.IsEmpty())
	{
		return FText::FromString(TEXT("현재 차량은 별도 파괴 FX Socket을 요구하지 않습니다. 이 항목은 무시해도 됩니다."));
	}

	TArray<FString> SocketLines;
	SocketLines.Reserve(OptionalSocketNames.Num() + 1);
	for (const FName SocketName : OptionalSocketNames)
	{
		const bool bSocketFound = ViewModel->IsCurrentChassisSocketFound(SocketName);
		SocketLines.Add(FString::Printf(
			TEXT("[파괴 FX] %s | %s"),
			*SocketName.ToString(),
			bSocketFound ? TEXT("현재 있음") : TEXT("현재 없음 — 효과를 이 위치에 붙일 경우 추가 필요")));
	}

	SocketLines.Add(TEXT("이 Socket은 차량이 파괴될 때 폭발/잔해 같은 FX를 차체의 특정 위치에 붙이기 위한 것입니다. 별도 위치가 필요하지 않다면 만들지 않아도 됩니다."));
	return FText::FromString(FString::Join(SocketLines, LINE_TERMINATOR));
}

// Step 3에서 복사할 optional Socket 이름이 하나 이상 있는지 반환합니다.
bool SCFVehicleBuilderTab::CanCopyOptionalSocketNames() const
{
	return ViewModel.IsValid() && !ViewModel->GetConditionalNonHardpointSocketNames().IsEmpty();
}

// Step 5 current Recipe/Target/accepted Evidence에 binding된 AI Physics Draft를 Project Saved에서 읽습니다.
FReply SCFVehicleBuilderTab::HandleLoadPhysicsProposalDraft()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Physics Draft load/binding validation diagnostic입니다.
	FString Error;
	if (!ViewModel->LoadPhysicsProposalDraft(Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT(
			"PhysicsDraftLoadFailedUser",
			"AI 물리 설정을 불러오지 못했습니다. 현재 차량 기준 정보와 제작 프로필 상태를 다시 확인한 뒤 재시도하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	LastDiagnosticText.Reset();
	LastStatusText = LOCTEXT(
		"PhysicsDraftLoaded",
		"AI 물리 설정 제안을 불러왔습니다. 아직 차량 전용 제작 프로필이나 VehicleData는 변경하지 않았습니다. '물리 설정 상세 보기'에서 수치와 의미를 확인한 뒤 검토하세요.");
	return FReply::Handled();
}

// Step 5 차량 전용 제작 프로필 변경을 검토하고 명시적 USER 승인 뒤 existing typed commit을 실행합니다.
FReply SCFVehicleBuilderTab::HandlePhysicsProposalReview()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Existing typed facade가 만든 mutation-free Profile 변경 preview입니다.
	FCFBuilderProfileCommitPreview Preview;
	// Preview/commit diagnostic입니다.
	FString Error;
	if (!ViewModel->PreparePhysicsProposal(Preview, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT(
			"PhysicsProposalReviewFailedUser",
			"AI 물리 설정 변경 내용을 검토할 수 없습니다. 현재 상태를 다시 확인한 뒤 재시도하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	// USER에게 보여줄 proposal metadata입니다.
	const FCFBuilderPhysicsDraft& Draft = ViewModel->GetLoadedPhysicsProposalDraft();

	// Profile domain별 actual payload 변화 여부를 USER 문장으로 변환합니다.
	const auto BuildProfileChangeLine = [](const TCHAR* Label, const FString& CurrentFingerprint, const FString& ProspectiveFingerprint)
	{
		return FString::Printf(
			TEXT("%s: %s"),
			Label,
			CurrentFingerprint == ProspectiveFingerprint ? TEXT("현재 값 유지") : TEXT("변경 예정"));
	};

	// USER 화면에는 숨기고 진단 정보에만 남길 exact proposal identity입니다.
	FString PhysicsReviewDiagnostic = FString::Printf(
		TEXT("EvidenceFingerprint=%s\nProspectiveResolvedDefinitionHash=%s\nTransmissionProposalHash=%s"),
		*Preview.EvidenceFingerprint,
		*Preview.ProspectiveResolvedDefinitionHash,
		Preview.TransmissionProposalHash.IsEmpty() ? TEXT("<none>") : *Preview.TransmissionProposalHash);
	for (const FName ClaimId : Draft.ConsumedClaimIds)
	{
		PhysicsReviewDiagnostic += FString::Printf(TEXT("\nConsumedClaim=%s"), *ClaimId.ToString());
	}
	for (const FString& TransmissionWarning : Preview.TransmissionDiagnostic.Warnings)
	{
		PhysicsReviewDiagnostic += FString::Printf(TEXT("\nTransmissionWarning=%s"), *TransmissionWarning);
	}
	LastDiagnosticText = PhysicsReviewDiagnostic;

	// USER가 실제 변경 영역과 mutation boundary만 판단할 review 문구입니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT(
			"AI 물리 설정 반영 검토\n\n"
			"%s\n\n"
			"%s\n\n"
			"변경 범위:\n"
			"- %s\n"
			"- %s\n"
			"- %s\n"
			"- %s\n\n"
			"'물리 설정 상세 보기'의 수치와 설명을 확인했다면 승인하세요.\n\n"
			"승인하면 차량 전용 제작 프로필에만 반영합니다.\n"
			"실제 VehicleData는 아직 변경하지 않으며 7단계 '최종 검토'에서 다시 확인합니다.\n"
			"기존 공유 프로필은 덮어쓰지 않고 자동 저장하지 않습니다.\n\n"
			"계속하시겠습니까?"),
		Draft.ProposalLabel.IsEmpty() ? TEXT("AI 물리 설정") : *Draft.ProposalLabel,
		Draft.UserFacingSummary.IsEmpty() ? TEXT("이 차량의 주행 특성에 맞춘 물리 설정 제안입니다.") : *Draft.UserFacingSummary,
		*BuildProfileChangeLine(TEXT("차량 기본 / 질량"), Preview.CurrentFingerprints.VehicleBaseFingerprint, Preview.ProspectiveFingerprints.VehicleBaseFingerprint),
		*BuildProfileChangeLine(TEXT("구동계 / 변속기"), Preview.CurrentFingerprints.DrivetrainFingerprint, Preview.ProspectiveFingerprints.DrivetrainFingerprint),
		*BuildProfileChangeLine(TEXT("조향 / 제동 / 서스펜션"), Preview.CurrentFingerprints.HandlingFingerprint, Preview.ProspectiveFingerprints.HandlingFingerprint),
		*BuildProfileChangeLine(TEXT("엔진 / 성능"), Preview.CurrentFingerprints.PerformanceFingerprint, Preview.ProspectiveFingerprints.PerformanceFingerprint)));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastStatusText = LOCTEXT(
			"PhysicsProposalCancelled",
			"AI 물리 설정 반영을 취소했습니다. 차량 전용 제작 프로필과 VehicleData는 변경하지 않았습니다.");
		return FReply::Handled();
	}

	// Explicit USER-approved private Profile commit 결과입니다.
	FCFAuthoringOpResult Result;
	if (!ViewModel->ExecutePreparedPhysicsProposal(Result, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT(
			"PhysicsProposalCommitFailedUser",
			"AI 물리 설정을 차량 전용 제작 프로필에 반영하지 못했습니다. 현재 상태를 다시 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	// successful Profile/receipt mutation 뒤 남은 non-fatal refresh warning입니다.
	const FString& PostCommitRefreshWarning = ViewModel->GetLastPostCommitRefreshWarning();
	LastDiagnosticText = PostCommitRefreshWarning.IsEmpty()
		? Result.Message
		: FString::Printf(
			TEXT("%s\n후속 상태 갱신 경고:\n%s"),
			*Result.Message,
			*PostCommitRefreshWarning);
	if (!PostCommitRefreshWarning.IsEmpty())
	{
		LastStatusText = LOCTEXT(
			"PhysicsProposalCommitRefreshWarningUser",
			"AI 물리 설정 반영은 완료됐지만 화면 상태를 다시 읽지 못했습니다. '현재 상태 다시 확인'을 눌러 최신 상태를 확인하세요. 실제 VehicleData는 아직 변경하지 않았고 자동 저장하지 않았습니다.");
		return FReply::Handled();
	}

	LastStatusText = LOCTEXT(
		"PhysicsProposalCommitPassUser",
		"AI 물리 설정을 차량 전용 제작 프로필에 반영했습니다. 실제 VehicleData는 아직 변경하지 않았습니다. 승인된 수치는 '물리 설정 상세 보기'에서 확인할 수 있으며 실제 적용은 7단계에서 다시 검토합니다. 자동 저장하지 않았습니다.");
	return FReply::Handled();
}

// Step 5 private 4 Profile은 unchanged이고 downstream resolved hash만 drift한 경우 persistent Physics receipt만 명시적으로 재검증합니다.
FReply SCFVehicleBuilderTab::HandlePhysicsReceiptRefresh()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Current accepted Physics payload 유지 여부와 receipt-only drift를 검증한 mutation0 preview입니다.
	FCFBuilderProfileCommitPreview Preview;
	// Preview/commit diagnostic입니다.
	FString Error;
	if (!ViewModel->PrepareCurrentPhysicsReceiptRefresh(Preview, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT(
			"PhysicsReceiptRefreshUnavailableUser",
			"현재 물리 설정을 그대로 재확인할 수 없습니다. 기존에 승인한 물리값이나 차량 기준 정보가 실제로 달라졌다면 일반 'AI 물리 설정' 검토가 필요합니다. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	// USER에게 숨기고 진단 정보에만 남길 old/new resolved Definition identity입니다.
	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	const FString PreviousResolvedHash = Recipe ? Recipe->BuilderCommitReceipt.ProspectiveResolvedDefinitionHash : FString();
	LastDiagnosticText = FString::Printf(
		TEXT("Physics receipt-only revalidation\nPreviousResolvedDefinitionHash=%s\nCurrentProspectiveResolvedDefinitionHash=%s\nEvidenceFingerprint=%s"),
		PreviousResolvedHash.IsEmpty() ? TEXT("<none>") : *PreviousResolvedHash,
		Preview.ProspectiveResolvedDefinitionHash.IsEmpty() ? TEXT("<none>") : *Preview.ProspectiveResolvedDefinitionHash,
		Preview.EvidenceFingerprint.IsEmpty() ? TEXT("<none>") : *Preview.EvidenceFingerprint);

	// 이 경로는 Profile payload 4개를 바꾸지 않고 provenance receipt만 current resolved Definition에 다시 binding합니다.
	const FText ReviewText = LOCTEXT(
		"PhysicsReceiptRefreshReview",
		"현재 물리 설정 재검증\n\n장비 장착 설정이 바뀌어 차량 전체 상태가 달라졌지만 기존에 승인한 물리값 자체는 변하지 않았습니다.\n\n승인하면:\n- 기존 물리값을 그대로 유지합니다.\n- VehicleData는 변경하지 않습니다.\n- 현재 차량 상태에 맞춰 물리 설정 승인 상태만 다시 확인합니다.\n- 자동 저장하지 않습니다.\n\n현재 물리 설정을 그대로 유지하고 승인 상태만 다시 확인하시겠습니까?");
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastStatusText = LOCTEXT(
			"PhysicsReceiptRefreshCancelled",
			"현재 물리 설정 재검증을 취소했습니다. 물리값, VehicleData, 차량 제작 정보는 변경하지 않았습니다.");
		return FReply::Handled();
	}

	// Existing CommitBuilderProfiles receipt-only transaction의 terminal 결과입니다.
	FCFAuthoringOpResult Result;
	if (!ViewModel->ExecutePreparedPhysicsProposal(Result, Error))
	{
		LastDiagnosticText += FString::Printf(TEXT("\nReceipt refresh failed: %s"), *Error);
		LastStatusText = LOCTEXT(
			"PhysicsReceiptRefreshFailedUser",
			"현재 물리 설정 재검증을 완료하지 못했습니다. 안전 조건이 달라졌을 수 있으니 현재 상태를 다시 확인하세요.");
		return FReply::Handled();
	}

	if (Result.Mutation.bProfileChanged || Result.Mutation.bTargetChanged || Result.Mutation.bSavePerformed)
	{
		LastDiagnosticText += FString::Printf(
			TEXT("\nUnexpected mutation boundary: ProfileChanged=%s TargetChanged=%s SavePerformed=%s"),
			Result.Mutation.bProfileChanged ? TEXT("true") : TEXT("false"),
			Result.Mutation.bTargetChanged ? TEXT("true") : TEXT("false"),
			Result.Mutation.bSavePerformed ? TEXT("true") : TEXT("false"));
		LastStatusText = LOCTEXT(
			"PhysicsReceiptRefreshBoundaryMismatch",
			"물리 설정 재검증 결과의 변경 범위가 예상과 달라 진단이 필요합니다. 자동 저장은 수행하지 않았습니다.");
		return FReply::Handled();
	}

	LastDiagnosticText += FString::Printf(TEXT("\n%s"), *Result.Message);
	// successful receipt-only mutation 뒤 남은 non-fatal refresh warning입니다.
	const FString& PostCommitRefreshWarning = ViewModel->GetLastPostCommitRefreshWarning();
	if (!PostCommitRefreshWarning.IsEmpty())
	{
		LastDiagnosticText += FString::Printf(TEXT("\n후속 상태 갱신 경고:\n%s"), *PostCommitRefreshWarning);
		LastStatusText = LOCTEXT(
			"PhysicsReceiptRefreshWarningUser",
			"물리 설정 승인 상태 재검증은 기록됐지만 화면 상태를 다시 읽지 못했습니다. '현재 상태 다시 확인'을 눌러 최신 상태를 확인하세요. 물리값과 VehicleData는 변경하지 않았고 자동 저장하지 않았습니다.");
		return FReply::Handled();
	}

	LastStatusText = LOCTEXT(
		"PhysicsReceiptRefreshPassUser",
		"기존 물리값은 그대로 유지한 채 현재 차량 상태에 맞춰 물리 설정 승인 상태만 다시 확인했습니다. VehicleData는 변경하지 않았고 자동 저장하지 않았습니다.");
	return FReply::Handled();
}

// Step 7 fresh semantic review와 exact Target/Recipe 저장 상태를 검토하고 USER 승인 뒤 durable final commit을 실행합니다.
FReply SCFVehicleBuilderTab::HandleFinalReviewApply()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// USER dialog 직전 current semantic + durable action입니다.
	FCFBuilderFinalCommitPreflight Preflight;
	// USER dialog 직전 fresh Final Review입니다.
	FCFBuilderFinalReviewResult Review;
	// Fresh preflight / durable commit diagnostic입니다.
	FString Error;
	if (!ViewModel->PrepareFinalReviewCommit(Preflight, Review, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT(
			"FinalReviewPrepareFailedUser",
			"최종 적용 및 저장 내용을 다시 확인하지 못했습니다. 현재 상태를 다시 확인하고 '진단 정보'를 확인하세요.");
		return FReply::Handled();
	}

	// USER가 이해할 current durable action 설명입니다.
	FString ActionText;
	switch (Preflight.Action)
	{
	case ECFBuilderFinalCommitAction::ApplyAndPersist:
		ActionText = TEXT("차량 데이터에 최종 변경을 적용한 뒤 VehicleData와 Recipe를 필요한 범위로 저장합니다.");
		break;
	case ECFBuilderFinalCommitAction::PersistDirtyPair:
		ActionText = TEXT("최종 계산 결과는 이미 적용되어 있으므로 현재 VehicleData와 Recipe의 미저장 변경만 필요한 범위로 저장합니다.");
		break;
	case ECFBuilderFinalCommitAction::FinalizeAppliedStateAndPersist:
		ActionText = TEXT("VehicleData는 최종 계산 결과와 같으므로 차량 데이터를 다시 쓰지 않고, Recipe의 적용 상태 기록을 현재 값으로 복구한 뒤 필요한 파일을 저장합니다.");
		break;
	default:
		ActionText = TEXT("현재 상태에 필요한 최종 저장 작업을 수행합니다.");
		break;
	}

	// Confirmation에 재사용할 current USER-facing 변경 요약입니다.
	const FString UserReviewSummary = FCFVehicleBuilderPresentation::BuildFinalReviewSummary(Review).ToString();
	// USER에게 기본 화면에서는 숨길 exact approval/action identity입니다.
	LastDiagnosticText = FString::Printf(
		TEXT("FinalCommit Action=%d\nScope=%s\nRecipe=%s\nTarget=%s\n%s"),
		static_cast<int32>(Preflight.Action),
		*Preflight.ApprovalScopeHash,
		*Preflight.SavedHandoff.RecipePath.ToString(),
		*Preflight.SavedHandoff.TargetPath.ToString(),
		*Preflight.Diagnostic);

	// USER는 실제 변경 목록과 exact 두 파일 저장 경계를 함께 확인하고 승인합니다.
	const FText ReviewText = FText::FromString(FString::Printf(
		TEXT(
			"최종 적용 및 저장 확인\n\n"
			"이번 작업:\n%s\n\n"
			"최종 변경 내용:\n%s\n\n"
			"저장 대상:\n"
			"- 차량 데이터: %s\n"
			"- 제작 정보: %s\n\n"
			"두 파일 모두 저장이 필요하면 차량 데이터 → 제작 정보 순서로 저장합니다.\n"
			"현재 두 파일에 함께 들어 있는 다른 미저장 변경도 각 파일을 저장할 때 같이 저장됩니다.\n"
			"다른 에셋은 저장하지 않으며 '모두 저장'도 사용하지 않습니다.\n"
			"실행 직전에 상태를 다시 확인하고, 확인창을 연 뒤 데이터가 달라졌다면 중단합니다. 자동 재시도하지 않습니다.\n\n"
			"진행하시겠습니까?"),
		*ActionText,
		*UserReviewSummary,
		*Preflight.SavedHandoff.TargetPath.ToString(),
		*Preflight.SavedHandoff.RecipePath.ToString()));

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		// USER 취소 시 prepared approval을 남기지 않도록 read-only refresh로 폐기합니다.
		FString RefreshError;
		ViewModel->RefreshCurrentState(RefreshError);
		if (!RefreshError.IsEmpty())
		{
			LastDiagnosticText += FString::Printf(TEXT("\nCancel refresh: %s"), *RefreshError);
		}
		LastStatusText = LOCTEXT(
			"FinalReviewApplyCancelled",
			"최종 적용 및 저장을 취소했습니다. 차량 데이터와 제작 정보는 변경하거나 저장하지 않았습니다.");
		return FReply::Handled();
	}

	// Explicit USER-approved durable final commit terminal result입니다.
	FCFBuilderFinalCommitResult Result;
	if (!ViewModel->ExecutePreparedFinalReviewCommit(Result, Error))
	{
		LastDiagnosticText += FString::Printf(
			TEXT("\nFinal commit failed: Outcome=%d\n%s"),
			static_cast<int32>(Result.Outcome),
			*Error);
		LastStatusText = Result.Outcome == ECFBuilderFinalCommitOutcome::TargetSavedRecipeSaveFailed
			? LOCTEXT(
				"FinalReviewRecipeSaveFailedUser",
				"차량 데이터 저장은 완료됐지만 제작 정보 저장에 실패했습니다. 이미 저장된 차량 데이터를 되돌리거나 다시 저장하지 않습니다. 현재 상태를 다시 확인한 뒤 남은 제작 정보 저장만 복구하세요.")
			: LOCTEXT(
				"FinalReviewApplyFailedUser",
				"최종 적용 및 저장을 완료하지 못했습니다. 이미 성공한 저장 단계가 있다면 그대로 보존하며 자동 재시도하지 않습니다. 현재 상태를 다시 확인하고 '진단 정보'를 확인하세요.");
		return FReply::Handled();
	}

	LastDiagnosticText += FString::Printf(
		TEXT("\nFinal commit result: Outcome=%d | TargetApplied=%s | AppliedStateFinalized=%s | TargetSaved=%s | RecipeSaved=%s | UndoAvailable=%s\n%s"),
		static_cast<int32>(Result.Outcome),
		Result.bTargetApplied ? TEXT("true") : TEXT("false"),
		Result.bAppliedStateFinalized ? TEXT("true") : TEXT("false"),
		Result.bTargetSaved ? TEXT("true") : TEXT("false"),
		Result.bRecipeSaved ? TEXT("true") : TEXT("false"),
		Result.bUndoAvailable ? TEXT("true") : TEXT("false"),
		*Result.Diagnostic);
	LastStatusText = Result.Outcome == ECFBuilderFinalCommitOutcome::CommittedRefreshWarning
		? LOCTEXT(
			"FinalReviewCommittedRefreshWarningUser",
			"차량 데이터와 제작 정보의 최종 적용/저장은 완료됐지만 화면 상태를 다시 읽지 못했습니다. '현재 상태 다시 확인'으로 화면만 갱신하세요. 저장 작업을 다시 실행할 필요는 없습니다.")
		: FText::FromString(FString::Printf(
			TEXT("최종 적용 및 저장이 완료되었습니다. 차량 데이터 적용: %s | 적용 상태 복구: %s | 차량 데이터 저장: %s | 제작 정보 저장: %s."),
			Result.bTargetApplied ? TEXT("예") : TEXT("아니오"),
			Result.bAppliedStateFinalized ? TEXT("예") : TEXT("아니오"),
			Result.bTargetSaved ? TEXT("예") : TEXT("불필요/기존 저장 유지"),
			Result.bRecipeSaved ? TEXT("예") : TEXT("불필요/기존 저장 유지")));
	return FReply::Handled();
}

// Step 7 마지막 successful Builder Apply가 발급한 exact guarded Undo token을 USER 승인 뒤 실행합니다.
FReply SCFVehicleBuilderTab::HandleFinalReviewUndo()
{
	if (!ViewModel.IsValid() || !ViewModel->HasFinalReviewUndoToken())
	{
		LastStatusText = LOCTEXT("FinalReviewUndoMissing", "현재 이 가이드에서 안전하게 되돌릴 직전 변경이 없습니다.");
		return FReply::Handled();
	}

	// USER 화면에는 숨기고 진단용으로만 보존할 exact Undo identity입니다.
	const FCFBuilderUndoToken& UndoToken = ViewModel->GetFinalReviewUndoToken();
	LastDiagnosticText = FString::Printf(
		TEXT("Undo TransactionId=%s\nRecipe=%s\nTarget=%s"),
		*UndoToken.TransactionId.ToString(EGuidFormats::DigitsWithHyphensLower),
		*UndoToken.RecipePath.ToString(),
		*UndoToken.TargetDefinitionPath.ToString());
	// 다른 Unreal 작업을 건드리지 않고 exact reverted pair만 저장하는 guarded Undo 경계를 USER에게 설명합니다.
	const FText ReviewText = LOCTEXT(
		"FinalReviewUndoReviewUser",
		"방금 적용한 변경 되돌리기\n\n이 차량 제작 가이드에서 방금 적용한 변경만 되돌립니다.\n이후에 차량 데이터나 관련 제작 정보가 다른 경로에서 바뀌었거나 다른 Unreal 작업이 더 최근에 있다면 아무 것도 되돌리지 않고 중단합니다.\n되돌리기가 성공하면 이 차량의 차량 데이터와 제작 정보만 차량 데이터 → 제작 정보 순서로 저장합니다. 전체 저장(Save All)이나 다른 에셋 저장은 하지 않으며, 저장 실패를 자동으로 재시도하거나 되돌리기를 다시 뒤집지 않습니다.\n\n방금 적용한 변경을 되돌리고 저장하시겠습니까?");

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastStatusText = LOCTEXT("FinalReviewUndoCancelled", "되돌리기를 취소했습니다. 현재 적용 상태는 그대로 유지합니다.");
		return FReply::Handled();
	}

	// Exact guarded Undo terminal result입니다.
	FCFAuthoringOpResult Result;
	// Guarded Undo diagnostic입니다.
	FString Error;
	if (!ViewModel->ExecuteFinalReviewUndo(Result, Error))
	{
		LastDiagnosticText += FString::Printf(TEXT("\nUndo failed: %s"), *Error);
		if (Result.Mutation.bTargetChanged || Result.Mutation.bRecipeChanged)
		{
			LastStatusText = LOCTEXT(
				"FinalReviewUndoPersistPartialUser",
				"방금 적용한 변경 자체는 되돌렸지만 저장을 끝내지 못했습니다. 자동으로 다시 되돌리거나 저장을 재시도하지 않았습니다. 현재 상태를 다시 확인한 뒤 Step 7의 최종 적용 및 저장으로 복구하세요.");
			return FReply::Handled();
		}
		LastStatusText = LOCTEXT(
			"FinalReviewUndoFailedUser",
			"방금 변경을 안전하게 되돌릴 수 없었습니다. 다른 작업이나 데이터 변경을 임의로 되돌리지는 않았습니다. '진단 정보'를 확인하세요.");
		return FReply::Handled();
	}

	LastDiagnosticText += FString::Printf(TEXT("\nUndo result: %s"), *Result.Message);
	LastStatusText = ViewModel->GetLastPostCommitRefreshWarning().IsEmpty()
		? LOCTEXT(
			"FinalReviewUndoPassUser",
			"이 차량 제작 가이드에서 방금 적용한 변경을 되돌리고 차량 데이터와 제작 정보를 저장했습니다. 이전 적용 전 차이가 다시 생겼다면 Step 7은 완료로 강제되지 않습니다.")
		: LOCTEXT(
			"FinalReviewUndoRefreshWarningUser",
			"변경 되돌리기와 저장은 완료됐지만 화면 상태를 다시 읽지 못했습니다. '현재 상태 다시 확인'으로 화면만 갱신하세요. 저장을 다시 실행할 필요는 없습니다.");
	return FReply::Handled();
}

// Step 8 current saved Target을 existing VB-P0-08 fixed-60Hz benchmark runner로 non-blocking 실행합니다.
FReply SCFVehicleBuilderTab::HandleRunDrivingBenchmark()
{
	if (!ViewModel.IsValid() || bDrivingBenchmarkRunning)
	{
		return FReply::Handled();
	}

	// Existing runner를 시작할 executable입니다.
	FString Executable;
	// Existing runner의 exact command-line arguments입니다.
	FString Arguments;
	// Runner working directory입니다.
	FString WorkingDirectory;
	// 이번 run의 exact result/USER acceptance identity입니다.
	FString RunId;
	// Saved-state / target identity / runner preparation diagnostic입니다.
	FString Error;
	if (!ViewModel->PrepareDrivingBenchmarkLaunch(Executable, Arguments, WorkingDirectory, RunId, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT(
			"DrivingBenchmarkPrepareFailedUser",
			"기술 주행 측정을 시작할 준비가 되지 않았습니다. 저장되지 않은 변경과 현재 차량 상태를 확인하고 '진단 정보'를 확인하세요.");
		return FReply::Handled();
	}

	// UI thread를 막지 않는 detached benchmark child process입니다.
	DrivingBenchmarkProcess = FPlatformProcess::CreateProc(
		*Executable,
		*Arguments,
		true,
		true,
		true,
		nullptr,
		0,
		*WorkingDirectory,
		nullptr,
		nullptr);
	if (!DrivingBenchmarkProcess.IsValid())
	{
		LastDiagnosticText = FString::Printf(TEXT("Benchmark CreateProc failed. RunId=%s"), *RunId);
		LastStatusText = LOCTEXT("DrivingBenchmarkLaunchFailed", "기술 주행 측정을 시작하지 못했습니다. 차량 데이터는 변경하지 않았습니다. '진단 정보'를 확인하세요.");
		return FReply::Handled();
	}

	RunningDrivingBenchmarkRunId = RunId;
	ResetDrivingBenchmarkProgressState();
	// USER elapsed 표시 기준인 monotonic start time입니다.
	DrivingBenchmarkStartedAtSeconds = FPlatformTime::Seconds();
	// 첫 sidecar read는 다음 Tick에서 즉시 허용합니다.
	NextDrivingBenchmarkProgressPollAtSeconds = 0.0;
	bDrivingBenchmarkRunning = true;
	LastDiagnosticText = FString::Printf(TEXT("Benchmark started. RunId=%s"), *RunId);
	LastStatusText = LOCTEXT(
		"DrivingBenchmarkRunningUser",
		"기술 주행 측정을 실행 중입니다. 현재 단계와 경과시간이 아래에 표시되며 차량 데이터를 수정하거나 자동 저장하지 않습니다.");
	return FReply::Handled();
}

// PowerShell child가 실행 중일 때 exact RunId progress sidecar를 bounded 0.5초 cadence로 polling합니다.
void SCFVehicleBuilderTab::PollDrivingBenchmarkProgress(const double CurrentTimeSeconds)
{
	if (!bDrivingBenchmarkRunning
		|| !ViewModel.IsValid()
		|| RunningDrivingBenchmarkRunId.IsEmpty()
		|| CurrentTimeSeconds < NextDrivingBenchmarkProgressPollAtSeconds)
	{
		return;
	}

	// Disk read cadence를 0.5초로 제한하는 다음 허용 시각입니다.
	NextDrivingBenchmarkProgressPollAtSeconds = CurrentTimeSeconds + 0.5;
	// Current sidecar typed snapshot입니다.
	FCFVehicleBuilderBenchmarkProgress Progress;
	// Missing/partial/stale progress는 benchmark 본체 failure가 아닌 diagnostic으로만 보존합니다.
	FString Error;
	if (!ViewModel->ReadDrivingBenchmarkProgress(Progress, Error))
	{
		LastDrivingBenchmarkProgressReadDiagnostic = Error;
		return;
	}

	if (Progress.RunId != RunningDrivingBenchmarkRunId)
	{
		LastDrivingBenchmarkProgressReadDiagnostic = FString::Printf(
			TEXT("Stale benchmark progress ignored. RunningRunId=%s SnapshotRunId=%s"),
			*RunningDrivingBenchmarkRunId,
			*Progress.RunId);
		return;
	}

	DrivingBenchmarkProgress = MoveTemp(Progress);
	bHasDrivingBenchmarkProgress = true;
	LastDrivingBenchmarkProgressReadDiagnostic.Reset();
}

// 새 benchmark 시작/terminal에서 Tab-local progress cache와 elapsed timer를 초기화합니다.
void SCFVehicleBuilderTab::ResetDrivingBenchmarkProgressState()
{
	DrivingBenchmarkProgress = FCFVehicleBuilderBenchmarkProgress();
	bHasDrivingBenchmarkProgress = false;
	DrivingBenchmarkStartedAtSeconds = 0.0;
	NextDrivingBenchmarkProgressPollAtSeconds = 0.0;
	LastDrivingBenchmarkProgressReadDiagnostic.Reset();
}

// Step 8 current selected VehicleData transient duplicate를 active PIE Player VehiclePawn에 적용합니다.
FReply SCFVehicleBuilderTab::HandleApplyDrivingTargetToPIE()
{
	if (!ViewModel.IsValid()
		|| ViewModel->GetStepViews().IsEmpty()
		|| ViewModel->GetCurrentStep().StepId != ECFVehicleBuilderStepId::DrivingTest)
	{
		return FReply::Handled();
	}

	if (bDrivingBenchmarkRunning)
	{
		// Benchmark-running 안내에는 persistent preflight를 다시 평가할 필요가 없어 기본 placeholder를 사용합니다.
		const FCFVehicleDrivingApplyPreflight PlaceholderPreflight;
		LastDiagnosticText.Reset();
		LastStatusText = FCFVehicleBuilderPresentation::BuildDrivingApplyReadiness(PlaceholderPreflight, true);
		return FReply::Handled();
	}

	// Button enable과 production Apply guard가 공유하는 current stable preflight입니다.
	const FCFVehicleDrivingApplyPreflight Preflight = ViewModel->ReadDrivingApplyPreflight();
	if (!Preflight.CanApply())
	{
		LastDiagnosticText = Preflight.Diagnostic;
		LastStatusText = FCFVehicleBuilderPresentation::BuildDrivingApplyReadiness(Preflight, false);
		return FReply::Handled();
	}

	// Stable preflight 통과 뒤의 active PIE/Pawn/runtime initialize diagnostic입니다.
	FString Error;
	if (!ViewModel->ApplySelectedVehicleToActivePIE(Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT(
			"DrivingTargetApplyFailedUser",
			"현재 플레이 차량에 적용하지 못했습니다. Play가 꺼져 있다면 플레이를 시작하세요. Play 중이라면 플레이 차량이 CarFight 차량으로 정상 생성됐는지 확인한 뒤 다시 시도하세요. 자세한 원인은 '진단 정보'에서 확인할 수 있습니다.");
		return FReply::Handled();
	}

	LastDiagnosticText.Reset();
	LastStatusText = LOCTEXT(
		"DrivingTargetAppliedToPIE",
		"현재 PIE의 플레이 차량에 선택 차량을 임시 적용했습니다. 원본 VehicleData Asset은 변경하지 않았습니다. 이제 가속, 변속, 조향, 제동과 바퀴/차체 거동을 직접 확인하세요.");
	return FReply::Handled();
}

// Step 8 exact current benchmark/Target에 USER Driving PASS를 기록하고 성공 뒤 Runtime Catalog promotion을 시도합니다.
FReply SCFVehicleBuilderTab::HandleAcceptUserDriving()
{
	if (!ViewModel.IsValid() || !ViewModel->HasDrivingBenchmarkResult())
	{
		return FReply::Handled();
	}

	// USER 화면에 노출하지 않을 exact benchmark/Target identity는 진단 정보에만 보존합니다.
	LastDiagnosticText = FString::Printf(
		TEXT("Driving acceptance RunId=%s\nTargetDefinitionHash=%s"),
		*ViewModel->GetDrivingBenchmarkResult().RunId,
		*ViewModel->GetDrivingBenchmarkResult().ExpectedTargetDefinitionHash);
	// USER는 실제 주행 체크리스트와 저장 경계만 확인하고 승인합니다.
	const FText ReviewText = LOCTEXT(
		"UserDrivingReviewUser",
		"주행 테스트 통과 확인\n\n현재 PIE에서 선택 차량을 직접 주행했고 아래 항목을 확인했습니까?\n\n- 출발과 가속 반응이 의도한 차량 성격에 어울림\n- 변속 시점과 변속 후 RPM 회복이 자연스러움\n- 조향 반응과 회전반경이 차량 크기와 성격에 어울림\n- 브레이크와 핸드브레이크가 정상적으로 제어됨\n- 고속에서 RPM·변속·차체 반응에 명백한 이상이 없음\n- 바퀴 떨림·과도한 튀김·차체 물리 이상이 플레이를 방해하지 않음\n\n기술 주행 측정 수치만으로 통과시키는 것이 아니라 직접 주행한 사용자의 판단입니다.\n승인하면 현재 차량 상태에 대한 주행 테스트 통과 기록을 남기고 데모 차량 목록 등록도 시도합니다. 목록 등록에 실패해도 주행 테스트 통과 기록은 취소하지 않습니다.\n관련 Asset은 자동 저장하지 않습니다.\n\n현재 차량을 주행 테스트 통과로 기록하시겠습니까?");

	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastStatusText = LOCTEXT("UserDrivingCancelled", "주행 테스트 통과 기록을 취소했습니다. 차량 데이터와 데모 차량 목록은 변경하지 않았습니다.");
		return FReply::Handled();
	}

	// Exact benchmark/Target-bound persistent USER Driving acceptance diagnostic입니다.
	FString Error;
	if (!ViewModel->AcceptCurrentUserDriving(Error))
	{
		LastDiagnosticText += FString::Printf(TEXT("\nDriving acceptance failed: %s"), *Error);
		LastStatusText = LOCTEXT(
			"UserDrivingAcceptFailedUser",
			"주행 테스트 통과 기록을 남기지 못했습니다. 현재 차량 상태를 다시 확인하고 '진단 정보'를 확인하세요.");
		return FReply::Handled();
	}

	// USER Driving acceptance와 독립된 Catalog promotion 결과입니다. 실패해도 이미 기록된 persistent receipt는 유지합니다.
	FString CatalogPromotionStatus;
	// USER status에는 내부 Catalog 결과를 직접 노출하지 않고 진단 정보와 별도 상태 표시에서 확인합니다.
	const bool bCatalogPromotionSucceeded = PromoteCurrentAcceptedVehicleToRuntimeCatalog(CatalogPromotionStatus);
	LastDiagnosticText += FString::Printf(
		TEXT("\nCatalog promotion success=%s\n%s"),
		bCatalogPromotionSucceeded ? TEXT("true") : TEXT("false"),
		*CatalogPromotionStatus);
	LastStatusText = LOCTEXT(
		"UserDrivingAcceptedUser",
		"주행 테스트 통과를 기록했습니다. 데모 차량 목록 등록도 함께 시도했습니다. 차량 제작 기록(Recipe)은 자동 저장하지 않았으므로 아래 '차량 제작 기록 저장'에서 저장 여부를 확인하세요.");
	return FReply::Handled();
}

// Persistent USER Driving receipt가 fresh Target에 exact binding된 current Recipe package 하나를 USER 승인 뒤 명시 저장합니다.
FReply SCFVehicleBuilderTab::HandleSaveCurrentRecipe()
{
	if (!ViewModel.IsValid())
	{
		return FReply::Handled();
	}

	// Button enable과 writer가 공유하는 fresh persistent-receipt preflight입니다.
	const FCFVehicleRecipeSavePreflight Preflight = ViewModel->ReadCurrentRecipeSavePreflight();
	if (!Preflight.CanSave())
	{
		LastDiagnosticText = Preflight.Diagnostic;
		LastStatusText = FCFVehicleBuilderPresentation::BuildDrivingRecipeSaveStatus(Preflight);
		return FReply::Handled();
	}

	// Recipe package 전체의 current 미저장 변경이 저장된다는 실제 writer scope를 USER 승인 전에 명시합니다.
	const FText ReviewText = LOCTEXT(
		"DrivingRecipeSaveReview",
		"차량 제작 기록 저장\n\n현재 차량의 제작 데이터(Recipe) Asset 1개에 있는 미저장 변경 전체를 저장합니다.\n여기에는 방금 기록한 주행 테스트 통과 정보와 같은 Recipe 내부 변경이 함께 포함될 수 있습니다.\n\n저장하지 않는 것:\n- VehicleData\n- Chassis/Wheel StaticMesh\n- 데모 차량 목록\n- 다른 차량 Recipe\n\nSave All은 실행하지 않고 자동 재시도도 하지 않습니다.\n\n현재 차량의 Recipe Asset 1개를 저장하시겠습니까?");
	if (FMessageDialog::Open(EAppMsgType::YesNo, ReviewText) != EAppReturnType::Yes)
	{
		LastDiagnosticText.Reset();
		LastStatusText = LOCTEXT("DrivingRecipeSaveCancelled", "차량 제작 기록 저장을 취소했습니다. 다른 Asset도 저장하지 않았습니다.");
		return FReply::Handled();
	}

	// Exact current Recipe single-package writer terminal result입니다.
	const FCFVehicleRecipeSaveResult Result = ViewModel->SaveCurrentRecipeAfterDrivingAcceptance();
	LastDiagnosticText = Result.Diagnostic;
	LastStatusText = FCFVehicleBuilderPresentation::BuildDrivingRecipeSaveResult(Result);
	return FReply::Handled();
}

// Current persistent USER Driving acceptance를 유지한 채 데모 차량 목록 등록만 explicit 재시도합니다.
FReply SCFVehicleBuilderTab::HandleRetryRuntimeCatalogPromotion()
{
	// Retry는 USER Driving acceptance를 다시 쓰지 않고 current acceptance guard 뒤 Catalog mutation만 수행합니다.
	FString CatalogPromotionStatus;
	// USER 화면에는 success/fail만 표시하고 detailed Catalog message는 진단 정보에 남깁니다.
	const bool bCatalogPromotionSucceeded = PromoteCurrentAcceptedVehicleToRuntimeCatalog(CatalogPromotionStatus);
	LastDiagnosticText = CatalogPromotionStatus;
	LastStatusText = bCatalogPromotionSucceeded
		? LOCTEXT("RuntimeCatalogRetryPassUser", "데모 차량 목록 등록을 다시 시도했고 현재 차량이 목록에 등록된 상태입니다. 목록 Asset은 자동 저장하지 않았습니다.")
		: LOCTEXT("RuntimeCatalogRetryFailUser", "데모 차량 목록 등록을 완료하지 못했습니다. 주행 테스트 통과 기록은 유지됩니다. '진단 정보'를 확인하세요.");
	return FReply::Handled();
}

// Current Recipe가 가리키는 exact persistent VehicleData를 Catalog promotion target으로 검증해 반환합니다.
bool SCFVehicleBuilderTab::TryGetCurrentCatalogPromotionTarget(
	UCFVehicleData*& OutTargetVehicleData,
	FString& OutError) const
{
	OutTargetVehicleData = nullptr;
	if (!ViewModel.IsValid())
	{
		OutError = TEXT("Builder ViewModel이 없어 Runtime Catalog 대상 차량을 확인할 수 없습니다.");
		return false;
	}

	// Current Builder Recipe입니다.
	UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	if (!Recipe)
	{
		OutError = TEXT("Runtime Catalog promotion에 사용할 current Recipe가 없습니다.");
		return false;
	}

	// Recipe의 canonical persistent Runtime VehicleData target입니다.
	UCFVehicleData* TargetVehicleData = Recipe->TargetVehicleData.LoadSynchronous();
	FString TargetValidationError;
	if (!FCFVehicleCatalogPromoService::ValidatePromotionTarget(
		TargetVehicleData,
		TargetValidationError))
	{
		OutError = TargetValidationError;
		return false;
	}

	OutTargetVehicleData = TargetVehicleData;
	OutError.Reset();
	return true;
}

// Current acceptance/Target/Default Catalog를 fresh read해 데모 차량 목록 등록 상태를 USER 용어로 표시합니다.
FText SCFVehicleBuilderTab::GetRuntimeCatalogPromotionStatusText() const
{
	if (!ViewModel.IsValid())
	{
		return LOCTEXT("RuntimeCatalogStatusNoViewModel", "데모 차량 목록: 현재 상태를 읽을 수 없습니다.");
	}

	if (!ViewModel->HasCurrentUserDrivingAcceptance())
	{
		return LOCTEXT("RuntimeCatalogStatusNoAcceptance", "데모 차량 목록: 주행 테스트 통과 후 등록할 수 있습니다.");
	}

	UCFVehicleData* TargetVehicleData = nullptr;
	FString TargetError;
	if (!TryGetCurrentCatalogPromotionTarget(TargetVehicleData, TargetError))
	{
		return LOCTEXT("RuntimeCatalogStatusTargetFailedUser", "데모 차량 목록: 현재 차량을 등록 대상으로 확인하지 못했습니다. '진단 정보'를 확인하세요.");
	}

	// Project Settings의 current Default Runtime Test Catalog입니다.
	UCFRuntimeTestCatalogData* RuntimeCatalog = nullptr;
	FCFVehicleCatalogPromoResult ResolveFailure;
	if (!FCFVehicleCatalogPromoService::ResolveDefaultRuntimeCatalogRaw(
		RuntimeCatalog,
		ResolveFailure))
	{
		return LOCTEXT("RuntimeCatalogStatusResolveFailedUser", "데모 차량 목록: 목록 Asset 상태를 확인하지 못했습니다. 등록 재시도 전에 '진단 정보'를 확인하세요.");
	}

	// Last-message cache가 아닌 current Catalog object에서 읽은 exact membership 상태입니다.
	const FCFVehicleCatalogPromoResult MembershipResult =
		FCFVehicleCatalogPromoService::ReadPromotionMembership(
			TargetVehicleData,
			RuntimeCatalog);

	// Invalid Catalog에 target pointer가 이미 있어도 success로 보이지 않도록 validation outcome을 membership보다 먼저 판정합니다.
	if (MembershipResult.Outcome == ECFVehicleCatalogPromoOutcome::CatalogInvalid)
	{
		return LOCTEXT("RuntimeCatalogStatusInvalidUser", "데모 차량 목록: 목록 Asset에 해결해야 할 문제가 있어 현재 등록 상태를 확정할 수 없습니다.");
	}

	if (MembershipResult.Outcome == ECFVehicleCatalogPromoOutcome::AlreadyRegistered
		&& MembershipResult.bIsMember)
	{
		return FText::FromString(FString::Printf(
			TEXT("데모 차량 목록: 등록됨 | 목록 저장 상태: %s"),
			MembershipResult.bCatalogPackageDirty
				? TEXT("저장되지 않은 변경 있음")
				: TEXT("저장 상태 정상")));
	}

	return LOCTEXT("RuntimeCatalogStatusNotRegisteredUser", "데모 차량 목록: 아직 등록되지 않았습니다. 주행 테스트 통과 기록은 유지됩니다.");
}

// Current USER Driving acceptance와 persistent Target이 유효하고 아직 exact Catalog member가 아닐 때 재시도를 허용합니다.
bool SCFVehicleBuilderTab::CanRetryRuntimeCatalogPromotion() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasCurrentUserDrivingAcceptance())
	{
		return false;
	}

	UCFVehicleData* TargetVehicleData = nullptr;
	FString TargetError;
	if (!TryGetCurrentCatalogPromotionTarget(TargetVehicleData, TargetError))
	{
		return false;
	}

	UCFRuntimeTestCatalogData* RuntimeCatalog = nullptr;
	FCFVehicleCatalogPromoResult ResolveFailure;
	if (!FCFVehicleCatalogPromoService::ResolveDefaultRuntimeCatalogRaw(
		RuntimeCatalog,
		ResolveFailure))
	{
		// Catalog가 현재 unavailable이어도 USER가 설정/Asset을 복구한 뒤 같은 버튼으로 재시도할 수 있게 유지합니다.
		return true;
	}

	const FCFVehicleCatalogPromoResult MembershipResult =
		FCFVehicleCatalogPromoService::ReadPromotionMembership(
			TargetVehicleData,
			RuntimeCatalog);
	return !(MembershipResult.Outcome == ECFVehicleCatalogPromoOutcome::AlreadyRegistered
		&& MembershipResult.bIsMember);
}

// Current USER Driving acceptance를 재확인한 뒤 Promotion Service를 호출하고 fresh readback 상태를 USER-facing 문자열로 반환합니다.
bool SCFVehicleBuilderTab::PromoteCurrentAcceptedVehicleToRuntimeCatalog(FString& OutStatusText)
{
	if (!ViewModel.IsValid() || !ViewModel->HasCurrentUserDrivingAcceptance())
	{
		OutStatusText = TEXT("등록하지 않음 — current USER Driving PASS 재확인에 실패했습니다.");
		return false;
	}

	UCFVehicleData* TargetVehicleData = nullptr;
	FString TargetError;
	if (!TryGetCurrentCatalogPromotionTarget(TargetVehicleData, TargetError))
	{
		OutStatusText = FString::Printf(TEXT("등록 실패 — %s"), *TargetError);
		return false;
	}

	// USER Driving PASS와 분리된 Editor-only Catalog mutation 결과입니다.
	const FCFVehicleCatalogPromoResult PromotionResult =
		FCFVehicleCatalogPromoService::PromoteVehicleToRuntimeCatalog(TargetVehicleData);

	// Promotion 결과 뒤 current Default Catalog/membership을 fresh readback해 stale last-result 표시를 피합니다.
	UCFRuntimeTestCatalogData* RuntimeCatalog = nullptr;
	FCFVehicleCatalogPromoResult ResolveFailure;
	FString FreshStatusText;
	if (FCFVehicleCatalogPromoService::ResolveDefaultRuntimeCatalogRaw(
		RuntimeCatalog,
		ResolveFailure))
	{
		const FCFVehicleCatalogPromoResult FreshMembership =
			FCFVehicleCatalogPromoService::ReadPromotionMembership(
				TargetVehicleData,
				RuntimeCatalog);
		if (FreshMembership.Outcome == ECFVehicleCatalogPromoOutcome::CatalogInvalid)
		{
			FreshStatusText = FString::Printf(
				TEXT("현재 Catalog validation 실패 — %s"),
				*FreshMembership.Message);
		}
		else if (FreshMembership.Outcome == ECFVehicleCatalogPromoOutcome::AlreadyRegistered
			&& FreshMembership.bIsMember)
		{
			FreshStatusText = FString::Printf(
				TEXT("현재 등록됨, Catalog %s"),
				FreshMembership.bCatalogPackageDirty
					? TEXT("미저장 변경 있음")
					: TEXT("package clean"));
		}
		else
		{
			FreshStatusText = FString::Printf(TEXT("현재 미등록 — %s"), *FreshMembership.Message);
		}
	}
	else
	{
		FreshStatusText = FString::Printf(TEXT("현재 상태 확인 실패 — %s"), *ResolveFailure.Message);
	}

	OutStatusText = FString::Printf(
		TEXT("%s | %s | 자동 저장 안 함"),
		*PromotionResult.Message,
		*FreshStatusText);
	return PromotionResult.IsSuccess();
}

// Vehicle ID 입력 변경을 BuilderVM canonical transient state에 반영하고 valid할 때 default package/object identity를 갱신합니다.
void SCFVehicleBuilderTab::HandleVehicleIdTextChanged(const FText& NewText)
{
	if (!ViewModel.IsValid())
	{
		ClearCreationIdentityFields();
		VehicleIdValidationText = LOCTEXT("VehicleIdMissingVM", "Builder ViewModel이 없어 Vehicle ID를 검증할 수 없습니다.");
		return;
	}

	// USER가 입력한 exact Vehicle ID 문자열입니다. Invalid 문자를 임의 sanitize하지 않습니다.
	const FString VehicleId = NewText.ToString();
	// Vehicle ID validation diagnostic입니다.
	FString Error;
	if (!ViewModel->SetVehicleCreationId(VehicleId, Error))
	{
		ClearCreationIdentityFields();
		VehicleIdValidationText = FText::FromString(Error);
		return;
	}

	ApplyDefaultCreationIdentityFromVehicleId();
}

// Current Vehicle ID로 deterministic default Definition/Recipe package/object 네 값을 Advanced 입력란에 채웁니다.
void SCFVehicleBuilderTab::ApplyDefaultCreationIdentityFromVehicleId()
{
	if (!ViewModel.IsValid()
		|| !NewDefinitionPackageTextBox.IsValid() || !NewDefinitionNameTextBox.IsValid()
		|| !NewRecipePackageTextBox.IsValid() || !NewRecipeNameTextBox.IsValid())
	{
		return;
	}

	// Default VehicleData package identity입니다.
	FString DefinitionPackageName;
	// Default VehicleData object name입니다.
	FString DefinitionAssetName;
	// Default Recipe package identity입니다.
	FString RecipePackageName;
	// Default Recipe object name입니다.
	FString RecipeAssetName;
	// Default identity build diagnostic입니다.
	FString Error;
	if (!ViewModel->BuildDefaultVehicleRecordIdentity(
		ViewModel->GetVehicleCreationId(),
		DefinitionPackageName,
		DefinitionAssetName,
		RecipePackageName,
		RecipeAssetName,
		Error))
	{
		ClearCreationIdentityFields();
		VehicleIdValidationText = FText::FromString(Error);
		return;
	}

	NewDefinitionPackageTextBox->SetText(FText::FromString(DefinitionPackageName));
	NewDefinitionNameTextBox->SetText(FText::FromString(DefinitionAssetName));
	NewRecipePackageTextBox->SetText(FText::FromString(RecipePackageName));
	NewRecipeNameTextBox->SetText(FText::FromString(RecipeAssetName));
	VehicleIdValidationText = FText::FromString(FString::Printf(
		TEXT("기본 경로 제안: %s / %s. 충돌과 최종 경로 유효성은 생성 검토 Preview가 다시 확인합니다."),
		*DefinitionPackageName,
		*RecipePackageName));
}

// Advanced Definition/Recipe package/object 입력 네 값을 모두 비웁니다.
void SCFVehicleBuilderTab::ClearCreationIdentityFields()
{
	if (NewDefinitionPackageTextBox.IsValid())
	{
		NewDefinitionPackageTextBox->SetText(FText::GetEmpty());
	}
	if (NewDefinitionNameTextBox.IsValid())
	{
		NewDefinitionNameTextBox->SetText(FText::GetEmpty());
	}
	if (NewRecipePackageTextBox.IsValid())
	{
		NewRecipePackageTextBox->SetText(FText::GetEmpty());
	}
	if (NewRecipeNameTextBox.IsValid())
	{
		NewRecipeNameTextBox->SetText(FText::GetEmpty());
	}
}

// Current Vehicle ID validation 안내를 표시합니다.
FText SCFVehicleBuilderTab::GetVehicleIdValidationText() const
{
	return VehicleIdValidationText;
}

// Explicit New Vehicle과 Mesh Candidate Quick Start를 구분하는 생성 검토 버튼 문구를 반환합니다.
FText SCFVehicleBuilderTab::GetCreateVehicleButtonText() const
{
	return ViewModel.IsValid() && !ViewModel->IsNewVehicleEntryActive() && ViewModel->IsMeshOnlyCandidate()
		? LOCTEXT("CreateFromMeshCandidate", "이 Mesh로 새 차량 만들기 — 검토")
		: LOCTEXT("CreateNewVehicleReview", "생성 내용 검토");
}

// Explicit New Vehicle은 빈 Vehicle ID로 초기화하고 Mesh Candidate는 Mesh stem 기반 Vehicle ID 제안값을 채웁니다.
void SCFVehicleBuilderTab::SyncCreationFieldsFromSelection()
{
	if (!NewVehicleIdTextBox.IsValid())
	{
		return;
	}

	if (!ViewModel.IsValid())
	{
		NewVehicleIdTextBox->SetText(FText::GetEmpty());
		ClearCreationIdentityFields();
		return;
	}

	if (ViewModel->IsNewVehicleEntryActive())
	{
		NewVehicleIdTextBox->SetText(FText::GetEmpty());
		ClearCreationIdentityFields();
		VehicleIdValidationText = LOCTEXT("VehicleIdEmptyGuide", "Vehicle ID를 입력하세요. 영문자, 숫자, _만 사용할 수 있습니다.");
		return;
	}

	if (!ViewModel->IsMeshOnlyCandidate())
	{
		NewVehicleIdTextBox->SetText(FText::GetEmpty());
		ClearCreationIdentityFields();
		VehicleIdValidationText = FText::GetEmpty();
		return;
	}

	// Mesh 이름에서 semantic inference 없이 Vehicle ID 제안용 파일명 stem만 가져옵니다.
	FString CandidateVehicleId = ViewModel->GetSelectedEntry().ChassisMeshPath.GetAssetName();
	CandidateVehicleId.RemoveFromStart(TEXT("SM_"));
	if (CandidateVehicleId.IsEmpty())
	{
		CandidateVehicleId = TEXT("Vehicle");
	}

	NewVehicleIdTextBox->SetText(FText::FromString(CandidateVehicleId));
}



// Current Recipe AssetIntent를 Step 2 pending picker 값으로 동기화합니다.
void SCFVehicleBuilderTab::SyncMeshPreparationFieldsFromRecipe()
{
	PendingChassisMeshPath = FSoftObjectPath();
	PendingWheelMeshPaths.SetNum(4);
	for (FSoftObjectPath& PendingWheelMeshPath : PendingWheelMeshPaths)
	{
		PendingWheelMeshPath = FSoftObjectPath();
	}

	if (!ViewModel.IsValid() || !ViewModel->GetRecipe())
	{
		return;
	}

	// Current persistent Recipe의 typed AssetIntent입니다.
	const FCFVehicleAssetIntent& AssetIntent = ViewModel->GetRecipe()->AssetIntent;
	PendingChassisMeshPath = AssetIntent.ChassisMesh.ToSoftObjectPath();
	PendingWheelMeshPaths[0] = AssetIntent.WheelMeshFL.ToSoftObjectPath();
	PendingWheelMeshPaths[1] = AssetIntent.WheelMeshFR.ToSoftObjectPath();
	PendingWheelMeshPaths[2] = AssetIntent.WheelMeshRL.ToSoftObjectPath();
	PendingWheelMeshPaths[3] = AssetIntent.WheelMeshRR.ToSoftObjectPath();
}

// Step 2 Mesh 준비 전용 UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetMeshPreparationVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::MeshPrep
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 2 pending Chassis StaticMesh object path 문자열을 반환합니다.
FString SCFVehicleBuilderTab::GetPendingChassisMeshPath() const
{
	return PendingChassisMeshPath.IsValid() ? PendingChassisMeshPath.ToString() : FString();
}

// Step 2 pending Wheel StaticMesh role별 object path 문자열을 반환합니다.
FString SCFVehicleBuilderTab::GetPendingWheelMeshPath(const int32 WheelRoleIndex) const
{
	return PendingWheelMeshPaths.IsValidIndex(WheelRoleIndex) && PendingWheelMeshPaths[WheelRoleIndex].IsValid()
		? PendingWheelMeshPaths[WheelRoleIndex].ToString()
		: FString();
}

// Step 1의 Explicit New Vehicle 또는 Mesh-only Quick Start 생성 UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetMeshCreationVisibility() const
{
	return ViewModel.IsValid()
		&& (ViewModel->IsNewVehicleEntryActive() || ViewModel->IsMeshOnlyCandidate())
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::IdentityReference
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Explicit New Vehicle 전용 Blank/optional Chassis picker 영역 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetNewVehicleCreationOptionsVisibility() const
{
	return ViewModel.IsValid() && ViewModel->IsNewVehicleEntryActive()
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Explicit New Vehicle의 current optional Chassis StaticMesh object path 문자열을 반환합니다.
FString SCFVehicleBuilderTab::GetNewVehicleChassisMeshPath() const
{
	return ViewModel.IsValid() && ViewModel->GetNewVehicleChassisMeshPath().IsValid()
		? ViewModel->GetNewVehicleChassisMeshPath().ToString()
		: FString();
}

// Step 1 managed Recipe의 Reference/Companion UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetReferenceFlowVisibility() const
{
	return ViewModel.IsValid()
		&& !ViewModel->IsNewVehicleEntryActive()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::IdentityReference
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 1 persistent Evidence 또는 loaded Draft의 typed truth를 사용자용 기준 정보로 표시합니다.
FText SCFVehicleBuilderTab::GetReferenceSummaryText() const
{
	if (!ViewModel.IsValid())
	{
		return FText::GetEmpty();
	}

	// USER 표시 helper에 넘길 Reference scalar projection입니다.
	FCFVehicleReferencePresentInfo Info;
	// Current persistent Reference Evidence입니다.
	const UCFVehicleRefEvidence* Evidence = ViewModel->GetCurrentReferenceEvidence();
	Info.bHasPersistentEvidence = Evidence != nullptr;
	Info.bHasLoadedDraft = ViewModel->HasLoadedResearchDraft();

	// Step 1 USER review 완료 여부를 persistent/backend state가 아닌 current Step projection에서 읽습니다.
	const FCFVehicleBuilderStepView* ReferenceStep = ViewModel->FindStepView(ECFVehicleBuilderStepId::IdentityReference);
	Info.bUserReviewed = ReferenceStep && ReferenceStep->State == ECFVehicleBuilderStepState::Complete;

	// Reference payload 배열을 USER 표시 scalar로 축약하는 read-only helper입니다.
	auto PopulateReferenceInfo = [&Info](
		const TArray<FCFRefVehicleIdentity>& ReferenceVehicles,
		const TArray<FCFRefSourceCitation>& Sources,
		const TArray<FCFRefConflict>& Conflicts,
		const TArray<FCFRefUnknownFact>& UnknownFacts)
	{
		// Primary role 차량 identity입니다.
		const FCFRefVehicleIdentity* PrimaryVehicle = ReferenceVehicles.FindByPredicate([](const FCFRefVehicleIdentity& Identity)
		{
			return Identity.Role == ECFRefVehicleRole::Primary;
		});
		if (PrimaryVehicle)
		{
			Info.Manufacturer = PrimaryVehicle->Manufacturer;
			Info.Model = PrimaryVehicle->Model;
			Info.ModelYearStart = PrimaryVehicle->ModelYearStart;
			Info.Trim = PrimaryVehicle->Trim;
			Info.Powertrain = PrimaryVehicle->Powertrain;
			Info.Transmission = PrimaryVehicle->Transmission;
		}

		Info.SourceCount = Sources.Num();
		Info.ConflictCount = Conflicts.Num();
		Info.BlockingConflictCount = 0;
		for (const FCFRefConflict& Conflict : Conflicts)
		{
			if (Conflict.Severity == ECFRefConflictSeverity::Block
				&& (Conflict.ResolutionPolicy == ECFRefResolutionPolicy::UnresolvedBlock
					|| Conflict.ResolutionClaimId.IsNone()))
			{
				++Info.BlockingConflictCount;
			}
		}

		Info.UnknownCount = UnknownFacts.Num();
		Info.BlockingUnknownCount = 0;
		for (const FCFRefUnknownFact& UnknownFact : UnknownFacts)
		{
			if (UnknownFact.BlockingUse == ECFRefUnknownBlockingUse::ProposalBlock)
			{
				++Info.BlockingUnknownCount;
			}
		}
	};

	if (Evidence)
	{
		PopulateReferenceInfo(
			Evidence->ReferenceVehicles,
			Evidence->Sources,
			Evidence->Conflicts,
			Evidence->UnknownFacts);
	}
	else if (ViewModel->HasLoadedResearchDraft())
	{
		// Persistent Evidence가 아직 없을 때 보여줄 loaded AI Research Draft payload입니다.
		const FCFVehicleRefEvidencePayload& DraftPayload = ViewModel->GetLoadedResearchDraft().EvidencePayload;
		PopulateReferenceInfo(
			DraftPayload.ReferenceVehicles,
			DraftPayload.Sources,
			DraftPayload.Conflicts,
			DraftPayload.UnknownFacts);
	}

	return FCFVehicleBuilderPresentation::BuildReferenceSummary(Info);
}

// Step 2 current Recipe AssetIntent를 사용자용 Chassis/Wheel Mesh 요약으로 표시합니다.
FText SCFVehicleBuilderTab::GetMeshPreparationSummaryText() const
{
	if (!ViewModel.IsValid() || !ViewModel->GetRecipe())
	{
		return LOCTEXT("MeshSummaryUnavailable", "현재 차량의 Mesh 설정을 읽을 수 없습니다.");
	}

	// Current persistent Recipe의 AssetIntent truth입니다.
	const FCFVehicleAssetIntent& AssetIntent = ViewModel->GetRecipe()->AssetIntent;
	return FCFVehicleBuilderPresentation::BuildMeshSummary(AssetIntent);
}

// Step 3 required Wheel Socket과 current 배치 수치를 사용자용 요약으로 표시합니다.
FText SCFVehicleBuilderTab::GetSocketPreparationSummaryText() const
{
	if (!ViewModel.IsValid())
	{
		return LOCTEXT("SocketSummaryUnavailable", "현재 차량의 Socket 상태를 읽을 수 없습니다.");
	}

	// Step 3 current Recipe binding 기준 effective Wheel Socket 이름입니다.
	FCFVehicleSocketPresentInfo Info;
	Info.WheelSocketNames = ViewModel->GetRequiredWheelSocketNames();
	for (const FName SocketName : Info.WheelSocketNames)
	{
		// Current Chassis에서 exact Socket이 실제 존재하는지 표시합니다.
		Info.WheelSocketFound.Add(ViewModel->IsCurrentChassisSocketFound(SocketName));
	}

	// Step 4 evaluator와 같은 source를 사용하는 read-only Layout fact입니다.
	FString LayoutError;
	ViewModel->ReadCurrentLayoutFacts(Info.LayoutFacts, LayoutError);
	return FCFVehicleBuilderPresentation::BuildSocketSummary(Info);
}

// Step 4 차량 배치 확인 전용 UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetLayoutReviewVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::LayoutCapture
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 4 current Socket/VehicleData 배치 수치를 사용자용 요약으로 표시합니다.
FText SCFVehicleBuilderTab::GetLayoutSummaryText() const
{
	if (!ViewModel.IsValid() || ViewModel->GetStepViews().IsEmpty())
	{
		return LOCTEXT("LayoutSummaryUnavailable", "현재 차량 배치 상태를 읽을 수 없습니다.");
	}

	// Step 4 evaluator와 공유하는 current Socket/Target 배치 사실입니다.
	FCFVehicleBuilderLayoutFacts LayoutFacts;
	// 실패 원문은 Level 1에 노출하지 않고 진단 정보에서만 사용합니다.
	FString LayoutError;
	ViewModel->ReadCurrentLayoutFacts(LayoutFacts, LayoutError);
	return FCFVehicleBuilderPresentation::BuildLayoutSummary(
		LayoutFacts,
		ViewModel->GetCurrentStep().State);
}

// Step 1~8 raw VM/backend 문장과 action diagnostic을 접힌 진단 정보로만 표시합니다.
FText SCFVehicleBuilderTab::GetCurrentStepDiagnosticText() const
{
	if (!ViewModel.IsValid() || ViewModel->GetStepViews().IsEmpty())
	{
		return FText::GetEmpty();
	}

	// Current raw backend Step projection입니다.
	const FCFVehicleBuilderStepView& Step = ViewModel->GetCurrentStep();
	FString DiagnosticText = FString::Printf(
		TEXT("[Backend Step Summary]\n%s\n\n[Backend Step Resolution]\n%s"),
		*Step.Summary.ToString(),
		*Step.Resolution.ToString());

	if (Step.StepId == ECFVehicleBuilderStepId::IdentityReference)
	{
		DiagnosticText += TEXT("\n\n[Reference Detail]\n");
		DiagnosticText += ViewModel->BuildReferenceSummary();
		DiagnosticText += TEXT("\n\n[AI Research Draft Path]\n");
		DiagnosticText += ViewModel->GetResearchDraftPath();
	}
	else if (Step.StepId == ECFVehicleBuilderStepId::LayoutCapture)
	{
		// Level 2에만 추가할 current Layout read diagnostic입니다.
		FCFVehicleBuilderLayoutFacts LayoutFacts;
		FString LayoutError;
		if (!ViewModel->ReadCurrentLayoutFacts(LayoutFacts, LayoutError) && !LayoutError.IsEmpty())
		{
			DiagnosticText += TEXT("\n\n[Layout Read Error]\n");
			DiagnosticText += LayoutError;
		}
	}
	else if (Step.StepId == ECFVehicleBuilderStepId::PhysicsProposal)
	{
		DiagnosticText += TEXT("\n\n[Physics Proposal Detail]\n");
		DiagnosticText += ViewModel->BuildPhysicsProposalSummary();
	}
	else if (Step.StepId == ECFVehicleBuilderStepId::GameplaySetup)
	{
		DiagnosticText += TEXT("\n\n[Gameplay Guidance Detail]\n");
		DiagnosticText += ViewModel->BuildGameplayGuidanceSummary();
	}
	else if (Step.StepId == ECFVehicleBuilderStepId::FinalReview)
	{
		DiagnosticText += TEXT("\n\n[Final Review Detail]\n");
		DiagnosticText += ViewModel->BuildFinalReviewSummary();
	}
	else if (Step.StepId == ECFVehicleBuilderStepId::DrivingTest)
	{
		DiagnosticText += TEXT("\n\n[Driving Test Detail]\n");
		DiagnosticText += ViewModel->BuildDrivingTestSummary();
	}

	if (!LastDiagnosticText.IsEmpty())
	{
		DiagnosticText += TEXT("\n\n[Last Action Diagnostic]\n");
		DiagnosticText += LastDiagnosticText;
	}

	return FText::FromString(DiagnosticText);
}


// Step 5 Physics Proposal 전용 UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetPhysicsProposalVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::PhysicsProposal
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 5에서 초보자가 바로 다음 행동을 이해할 수 있는 짧은 요약을 표시합니다.
FText SCFVehicleBuilderTab::GetPhysicsProposalUserSummaryText() const
{
	if (!ViewModel.IsValid() || ViewModel->GetStepViews().IsEmpty())
	{
		return LOCTEXT("PhysicsProposalUserSummaryUnavailable", "현재 물리 설정 상태를 읽을 수 없습니다. '현재 상태 다시 확인'을 눌러주세요.");
	}

	// 현재 Step 5의 진행 상태입니다.
	const FCFVehicleBuilderStepView& PhysicsStep = ViewModel->GetCurrentStep();
	if (PhysicsStep.State == ECFVehicleBuilderStepState::Complete)
	{
		return LOCTEXT(
			"PhysicsProposalUserSummaryComplete",
			"AI 물리 설정 검토가 완료되었습니다. 이 단계에서 추가로 할 작업이 없습니다. 실제 VehicleData 반영은 7단계 Final Review에서 다시 확인한 뒤 이루어지므로 '다음'으로 진행하세요.");
	}

	if (PhysicsStep.State == ECFVehicleBuilderStepState::Blocked)
	{
		return LOCTEXT(
			"PhysicsProposalUserSummaryBlocked",
			"현재 물리 설정 단계가 막혀 있습니다. 먼저 '현재 상태 다시 확인'을 눌러보고, 계속 막히면 아래 '진단 정보'에서 원인을 확인하세요.");
	}

	if (ViewModel->HasLoadedPhysicsProposalDraft())
	{
		return LOCTEXT(
			"PhysicsProposalUserSummaryLoaded",
			"AI 물리 설정을 불러왔습니다. 'AI 물리 설정 검토 후 반영'을 눌러 변경 내용을 확인한 뒤 승인하세요. 이 단계에서는 VehicleData를 직접 변경하지 않습니다.");
	}

	return LOCTEXT(
		"PhysicsProposalUserSummaryNeedLoad",
		"먼저 'AI 물리 설정 불러오기'를 누르세요. 불러온 뒤 'AI 물리 설정 검토 후 반영'에서 변경 내용을 확인하고 승인하면 됩니다.");
}

// Step 5에서 AI 제안과 승인 완료 제작 프로필을 구분하고 실제 물리값의 의미와 주행 영향을 표시합니다.
FText SCFVehicleBuilderTab::GetPhysicsProposalSummaryText() const
{
	if (!ViewModel.IsValid())
	{
		return LOCTEXT("PhysicsProposalReadableNoViewModel", "현재 물리 설정 상태를 읽을 수 없습니다.");
	}

	if (ViewModel->HasLoadedPhysicsProposalDraft())
	{
		// 아직 승인되지 않은 current AI 물리 설정 제안입니다.
		const FCFBuilderPhysicsDraft& Draft = ViewModel->GetLoadedPhysicsProposalDraft();
		return FCFVehicleBuilderPresentation::BuildPhysicsSummary(
			Draft.ProfilePayload,
			ECFVehiclePhysicsPresentationSource::AiDraft);
	}

	// 승인 완료 제작 프로필을 표시해도 되는지 판정할 current Recipe와 Step입니다.
	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	const FCFVehicleBuilderStepView* PhysicsStep = ViewModel->FindStepView(ECFVehicleBuilderStepId::PhysicsProposal);
	if (Recipe && Recipe->BuilderCommitReceipt.IsValid()
		&& PhysicsStep
		&& PhysicsStep->State == ECFVehicleBuilderStepState::Complete)
	{
		// 승인 완료 Builder-private 4 Profile current typed payload입니다.
		FCFBuilderPrivateProfilePayload CurrentProfilePayload;
		// Read 실패 원문은 Level 1에 노출하지 않고 진단 화면에서 backend summary로 확인합니다.
		FString ProfileReadError;
		if (ViewModel->ReadCurrentPrivateProfilePayload(CurrentProfilePayload, ProfileReadError))
		{
			return FCFVehicleBuilderPresentation::BuildPhysicsSummary(
				CurrentProfilePayload,
				ECFVehiclePhysicsPresentationSource::ApprovedBuilderProfile);
		}

		return LOCTEXT(
			"PhysicsProposalReadableCommittedReadFailed",
			"AI 물리 설정 검토 기록은 있지만 승인된 차량 전용 제작 프로필의 세부 값을 읽을 수 없습니다. '현재 상태 다시 확인'을 누르고 진단 정보를 확인하세요.");
	}

	if (Recipe && Recipe->BuilderCommitReceipt.IsValid())
	{
		return LOCTEXT(
			"PhysicsProposalReadableStale",
			"기존에 승인한 차량 전용 제작 프로필이 있지만 현재 기준 정보와 다시 확인이 필요한 상태입니다. 'AI 물리 설정 불러오기'로 현재 기준의 새 제안을 불러와 검토하세요.");
	}

	return LOCTEXT(
		"PhysicsProposalReadableNotLoaded",
		"아직 AI 물리 설정 제안을 불러오지 않았습니다. 아래 'AI 물리 설정 불러오기'를 눌러 먼저 제안을 확인하세요.");
}

// Step 6 Hardpoint별 Standard MountType transient draft를 변경합니다. Recipe mutation은 하지 않습니다.
FReply SCFVehicleBuilderTab::HandleSelectMountDraftType(
	const FName LocationSlotId,
	const ECFVehicleMountType MountType)
{
	if (!LocationSlotId.IsNone())
	{
		PendingMountTypes.Add(LocationSlotId, MountType);
		RefreshMountPlanningPresentation();
		LastDiagnosticText.Reset();
		LastStatusText = FText::FromString(FString::Printf(
			TEXT("%s 장착 위치의 장착 방식을 선택했습니다. 아직 '장착 규칙 반영' 전입니다."),
			*LocationSlotId.ToString()));
	}
	return FReply::Handled();
}

// Step 6 Hardpoint별 Standard SizeLimit transient draft를 변경합니다. Recipe mutation은 하지 않습니다.
FReply SCFVehicleBuilderTab::HandleSelectMountDraftSize(
	const FName LocationSlotId,
	const ECFVehicleWeaponSize SizeLimit)
{
	if (!LocationSlotId.IsNone())
	{
		PendingMountSizes.Add(LocationSlotId, SizeLimit);
		RefreshMountPlanningPresentation();
		LastDiagnosticText.Reset();
		LastStatusText = FText::FromString(FString::Printf(
			TEXT("%s 장착 위치의 허용 장비 크기를 선택했습니다. 아직 '장착 규칙 반영' 전입니다."),
			*LocationSlotId.ToString()));
	}
	return FReply::Handled();
}

// Step 6 Hardpoint별 optional EquipmentPreset pending picker를 변경합니다. Recipe mutation은 하지 않습니다.
void SCFVehicleBuilderTab::HandleMountPresetChanged(
	const FAssetData& AssetData,
	const FName LocationSlotId)
{
	if (LocationSlotId.IsNone())
	{
		return;
	}

	PendingMountPresetPaths.Add(
		LocationSlotId,
		AssetData.IsValid() ? AssetData.GetSoftObjectPath() : FSoftObjectPath());
	RefreshMountPlanningPresentation();
	LastDiagnosticText.Reset();
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("%s 장착 위치의 기본 장비 프리셋을 선택했습니다. 아직 '장착 규칙 반영' 전입니다."),
		*LocationSlotId.ToString()));
}

// Step 6 Hardpoint 하나의 complete transient draft를 Standard 1:1 typed Recipe write로 반영합니다.
FReply SCFVehicleBuilderTab::HandleCommitStandardMount(const FName LocationSlotId)
{
	if (!ViewModel.IsValid() || LocationSlotId.IsNone())
	{
		return FReply::Handled();
	}

	const ECFVehicleMountType MountType = PendingMountTypes.FindRef(LocationSlotId);
	const ECFVehicleWeaponSize SizeLimit = PendingMountSizes.FindRef(LocationSlotId);
	const FSoftObjectPath PresetPath = PendingMountPresetPaths.FindRef(LocationSlotId);

	FCFMountIntent CommittedIntent;
	FCFAuthoringOpResult Result;
	FString Error;
	if (!ViewModel->CommitStandardMountIntent(
		LocationSlotId,
		MountType,
		SizeLimit,
		PresetPath,
		CommittedIntent,
		Result,
		Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT(
			"MountCommitFailedUser",
			"장착 규칙을 반영하지 못했습니다. 장착 방식과 허용 크기, 현재 장착 위치 상태를 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		RefreshMountPlanningPresentation();
		return FReply::Handled();
	}

	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastDiagnosticText = Result.Message;
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("%s 장착 위치의 장착 규칙을 차량 제작 기록에 반영했습니다. 실제 VehicleData는 변경하지 않았고 자동 저장하지 않았습니다."),
		*CommittedIntent.LocationSlotRef.ToString()));
	return FReply::Handled();
}

// Step 6 exact Recipe MountProfileId 하나를 typed remove lane으로 제거합니다.
FReply SCFVehicleBuilderTab::HandleRemoveMount(const FName MountProfileId)
{
	if (!ViewModel.IsValid() || MountProfileId.IsNone())
	{
		return FReply::Handled();
	}

	FCFAuthoringOpResult Result;
	FString Error;
	if (!ViewModel->RemoveMountIntent(MountProfileId, Result, Error))
	{
		LastDiagnosticText = Error;
		LastStatusText = LOCTEXT(
			"MountRemoveFailedUser",
			"장착 규칙을 삭제하지 못했습니다. 현재 장착 구조를 확인하세요. 자세한 원문은 '진단 정보'에서 확인할 수 있습니다.");
		RefreshMountPlanningPresentation();
		return FReply::Handled();
	}

	SyncMountPlanningDraftsFromRecipe();
	RefreshMountPlanningPresentation();
	LastDiagnosticText = Result.Message;
	LastStatusText = FText::FromString(FString::Printf(
		TEXT("장착 규칙 %s를 삭제했습니다. 장비 장착 위치와 Chassis Socket, VehicleData는 변경하지 않았고 자동 저장하지 않았습니다."),
		*MountProfileId.ToString()));
	return FReply::Handled();
}

// Step 6 current Recipe/HardpointPlanMode에서 Standard Mount planning subtree를 생성합니다.
TSharedRef<SWidget> SCFVehicleBuilderTab::BuildMountPlanningPanel()
{
	TSharedRef<SVerticalBox> Panel = SNew(SVerticalBox);
	if (!ViewModel.IsValid() || !ViewModel->GetRecipe())
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock).Text(LOCTEXT("MountPlanNoRecipe", "현재 차량의 장착 규칙을 읽을 수 없습니다."))
		];
		return Panel;
	}

	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	const ECFBuilderHardpointPlanMode PlanMode = ViewModel->GetHardpointPlanMode();

	Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
	[
		SNew(STextBlock)
			.Text(LOCTEXT("StandardMountHeader", "장착 위치별 장비 규칙"))
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 10))
	];

	if (PlanMode == ECFBuilderHardpointPlanMode::LegacyCompatible)
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
				.Text(FText::FromString(FString::Printf(
					TEXT("기존 차량의 장착 규칙 %d개를 그대로 유지하는 호환 모드입니다. 이 화면에서는 기존의 복합 장착 구조를 자동으로 바꾸지 않습니다. 새 제작 방식으로 편집하려면 3단계에서 '장착 위치 사용'으로 전환하세요."),
					Recipe->MountIntents.Num())))
				.AutoWrapText(true)
		];
		return Panel;
	}

	if (PlanMode == ECFBuilderHardpointPlanMode::Unspecified)
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("MountPlanUnspecified", "3단계에서 장비 장착 위치 사용 여부가 아직 정해지지 않았습니다. 먼저 '장착점 없음' 또는 '장착 위치 사용'을 선택하세요."))
				.AutoWrapText(true)
		];
		return Panel;
	}

	if (PlanMode == ECFBuilderHardpointPlanMode::NoHardpoints)
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
				.Text(Recipe->HardpointIntents.IsEmpty() && Recipe->MountIntents.IsEmpty()
					? LOCTEXT("MountPlanIntentionalZero", "이 차량은 장비 장착 위치를 사용하지 않도록 설정되어 있습니다. 추가 장착 규칙이 없어도 정상입니다.")
					: LOCTEXT("MountPlanNoHardpointConflict", "장착 위치를 사용하지 않는 설정인데 기존 장착 위치나 장착 규칙이 남아 있습니다. 장착 규칙을 먼저 삭제하고 3단계에서 장착 위치를 정리하세요."))
				.AutoWrapText(true)
		];
		return Panel;
	}

	if (Recipe->HardpointIntents.IsEmpty())
	{
		Panel->AddSlot().AutoHeight()
		[
			SNew(STextBlock)
				.Text(LOCTEXT("MountPlanNoHardpointsYet", "장비 장착 위치를 사용하도록 설정했지만 아직 위치가 없습니다. 3단계에서 장착 위치를 하나 이상 추가하세요."))
				.AutoWrapText(true)
		];
		return Panel;
	}

	Panel->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 8.0f)
	[
		SNew(STextBlock)
			.Text(LOCTEXT("MountPlanDraftBoundary", "각 장착 위치에서 어떤 방식과 크기의 장비를 달 수 있는지 정합니다. 아래에서 장착 방식과 허용 크기를 고른 뒤 '장착 규칙 생성/반영'을 누르세요. 기본 장비 프리셋은 필요할 때만 선택하면 됩니다."))
			.AutoWrapText(true)
	];

	for (const FCFHardpointIntent& HardpointIntent : Recipe->HardpointIntents)
	{
		const FName LocationSlotId = HardpointIntent.LocationSlotId;
		if (LocationSlotId.IsNone())
		{
			continue;
		}

		TArray<const FCFMountIntent*> MountsForHardpoint;
		for (const FCFMountIntent& MountIntent : Recipe->MountIntents)
		{
			if (MountIntent.LocationSlotRef == LocationSlotId)
			{
				MountsForHardpoint.Add(&MountIntent);
			}
		}

		// 이 장착 위치에 정확히 하나의 기존 장착 규칙이 있을 때 편집 대상으로 사용합니다.
		const FCFMountIntent* ExistingMount = MountsForHardpoint.Num() == 1 ? MountsForHardpoint[0] : nullptr;
		const ECFVehicleMountType DraftMountType = PendingMountTypes.FindRef(LocationSlotId);
		const ECFVehicleWeaponSize DraftSizeLimit = PendingMountSizes.FindRef(LocationSlotId);
		const bool bDraftRuleComplete = DraftMountType != ECFVehicleMountType::None
			&& (DraftMountType == ECFVehicleMountType::Utility || DraftSizeLimit != ECFVehicleWeaponSize::None);
		const bool bStandardStructureEditable = MountsForHardpoint.Num() <= 1;

		const auto BuildTypeButton = [this, LocationSlotId, DraftMountType](const ECFVehicleMountType Type, const TCHAR* Label, const TCHAR* Tooltip)
		{
			const FString ButtonLabel = DraftMountType == Type
				? FString::Printf(TEXT("[%s]"), Label)
				: FString(Label);
			return SNew(SButton)
				.Text(FText::FromString(ButtonLabel))
				.ToolTipText(FText::FromString(Tooltip))
				.OnClicked(this, &SCFVehicleBuilderTab::HandleSelectMountDraftType, LocationSlotId, Type);
		};
		const auto BuildSizeButton = [this, LocationSlotId, DraftSizeLimit](const ECFVehicleWeaponSize Size, const TCHAR* Label)
		{
			const FString ButtonLabel = DraftSizeLimit == Size
				? FString::Printf(TEXT("[%s]"), Label)
				: FString(Label);
			return SNew(SButton)
				.Text(FText::FromString(ButtonLabel))
				.OnClicked(this, &SCFVehicleBuilderTab::HandleSelectMountDraftSize, LocationSlotId, Size);
		};

		Panel->AddSlot().AutoHeight().Padding(0.0f, 4.0f)
		[
			SNew(SBorder)
			.Padding(8.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 5.0f)
				[
					SNew(STextBlock)
						.Text(FText::FromString(FString::Printf(
							TEXT("장착 위치: %s | 현재 장착 규칙: %s"),
							*LocationSlotId.ToString(),
							MountsForHardpoint.IsEmpty() ? TEXT("없음") : TEXT("있음"))))
						.Font(FCoreStyle::GetDefaultFontStyle("Bold", 9))
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
					SNew(STextBlock)
						.Text(MountsForHardpoint.Num() > 1
							? LOCTEXT("MountMultiConflict", "이 장착 위치에 장착 규칙이 여러 개 연결되어 있어 간단 편집을 사용할 수 없습니다. '고급 차량 데이터 제작'에서 구조를 먼저 정리하세요.")
							: LOCTEXT("MountTypeDraftLabel", "장착 방식 선택"))
						.AutoWrapText(true)
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 5.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildTypeButton(ECFVehicleMountType::Fixed, TEXT("고정(Fixed)"), TEXT("차량과 Socket 방향을 그대로 사용하는 고정형 장착입니다."))]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildTypeButton(ECFVehicleMountType::Gimbal, TEXT("가동(Gimbal)"), TEXT("제한된 범위에서 조준 방향을 움직일 수 있는 가동형 장착입니다."))]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildTypeButton(ECFVehicleMountType::Turret, TEXT("포탑(Turret)"), TEXT("회전 포탑처럼 독립적으로 조준 방향을 바꾸는 장착입니다."))]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildTypeButton(ECFVehicleMountType::Launcher, TEXT("발사기(Launcher)"), TEXT("로켓·미사일 등 발사기 계열 장비에 사용하는 장착입니다."))]
					+ SHorizontalBox::Slot().AutoWidth()[BuildTypeButton(ECFVehicleMountType::Utility, TEXT("유틸리티(Utility)"), TEXT("센서·보조장치처럼 무기형 장비와 다른 규칙을 쓰는 유틸리티 장비용 장착입니다."))]
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
				[
					SNew(STextBlock)
						.Text(LOCTEXT("MountSizeDraftLabel", "허용 장비 크기 — 일반 장비는 소형 이상이 필요하고, 유틸리티는 크기 제한 없음도 선택할 수 있습니다."))
						.AutoWrapText(true)
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildSizeButton(ECFVehicleWeaponSize::None, TEXT("제한 없음"))]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildSizeButton(ECFVehicleWeaponSize::Small, TEXT("소형"))]
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 4.0f, 0.0f)[BuildSizeButton(ECFVehicleWeaponSize::Medium, TEXT("중형"))]
					+ SHorizontalBox::Slot().AutoWidth()[BuildSizeButton(ECFVehicleWeaponSize::Large, TEXT("대형"))]
				]

				+ SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 6.0f)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(0.0f, 0.0f, 8.0f, 0.0f)
					[
						SNew(SBox).WidthOverride(145.0f)
						[
							SNew(STextBlock).Text(LOCTEXT("MountPresetLabel", "기본 장비 프리셋 (선택)"))
						]
					]
					+ SHorizontalBox::Slot().FillWidth(1.0f)
					[
						SNew(SObjectPropertyEntryBox)
							.AllowedClass(UCFEquipmentPresetData::StaticClass())
							.ObjectPath(this, &SCFVehicleBuilderTab::GetPendingMountPresetPath, LocationSlotId)
							.OnObjectChanged(this, &SCFVehicleBuilderTab::HandleMountPresetChanged, LocationSlotId)
							.AllowClear(true)
					]
				]

				+ SVerticalBox::Slot().AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 6.0f, 0.0f)
					[
						SNew(SButton)
							.Text(ExistingMount ? LOCTEXT("UpdateStandardMount", "장착 규칙 반영") : LOCTEXT("CreateStandardMount", "장착 규칙 생성"))
							.ToolTipText(LOCTEXT("CommitStandardMountTooltip", "선택한 장착 방식, 허용 크기, 기본 장비 프리셋을 이 차량의 제작 기록에 반영합니다. 실제 VehicleData와 Static Mesh는 직접 변경하지 않으며 자동 저장하지 않습니다."))
							.IsEnabled(bStandardStructureEditable && bDraftRuleComplete)
							.OnClicked(this, &SCFVehicleBuilderTab::HandleCommitStandardMount, LocationSlotId)
					]
					+ SHorizontalBox::Slot().AutoWidth()
					[
						SNew(SButton)
							.Text(LOCTEXT("RemoveStandardMount", "장착 규칙 삭제"))
							.ToolTipText(LOCTEXT("RemoveStandardMountTooltip", "이 장착 위치의 장착 규칙만 차량 제작 기록에서 삭제합니다. 장착 위치와 Chassis Socket, Static Mesh, VehicleData는 삭제하지 않습니다."))
							.IsEnabled(ExistingMount && !ExistingMount->MountProfileId.IsNone())
							.OnClicked(this, &SCFVehicleBuilderTab::HandleRemoveMount, ExistingMount ? ExistingMount->MountProfileId : NAME_None)
					]
				]
			]
		];
	}

	return Panel;
}

// Step 6 persistent Recipe의 existing 1:1 Mount 값을 transient draft로 다시 동기화합니다.
void SCFVehicleBuilderTab::SyncMountPlanningDraftsFromRecipe()
{
	PendingMountTypes.Reset();
	PendingMountSizes.Reset();
	PendingMountPresetPaths.Reset();

	if (!ViewModel.IsValid() || !ViewModel->GetRecipe())
	{
		return;
	}

	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	for (const FCFHardpointIntent& HardpointIntent : Recipe->HardpointIntents)
	{
		if (HardpointIntent.LocationSlotId.IsNone())
		{
			continue;
		}

		TArray<const FCFMountIntent*> MountsForHardpoint;
		for (const FCFMountIntent& MountIntent : Recipe->MountIntents)
		{
			if (MountIntent.LocationSlotRef == HardpointIntent.LocationSlotId)
			{
				MountsForHardpoint.Add(&MountIntent);
			}
		}

		if (MountsForHardpoint.Num() == 1)
		{
			const FCFMountIntent& ExistingMount = *MountsForHardpoint[0];
			PendingMountTypes.Add(HardpointIntent.LocationSlotId, ExistingMount.MountType);
			PendingMountSizes.Add(HardpointIntent.LocationSlotId, ExistingMount.SizeLimit);
			PendingMountPresetPaths.Add(HardpointIntent.LocationSlotId, ExistingMount.DefaultEquipmentPresetData.ToSoftObjectPath());
		}
		else
		{
			PendingMountTypes.Add(HardpointIntent.LocationSlotId, ECFVehicleMountType::None);
			PendingMountSizes.Add(HardpointIntent.LocationSlotId, ECFVehicleWeaponSize::None);
			PendingMountPresetPaths.Add(HardpointIntent.LocationSlotId, FSoftObjectPath());
		}
	}
}

// Step 6 transient/persistent 변경 뒤 Mount planning subtree를 current truth로 교체합니다.
void SCFVehicleBuilderTab::RefreshMountPlanningPresentation()
{
	if (MountPlanningHost.IsValid())
	{
		MountPlanningHost->SetContent(BuildMountPlanningPanel());
	}
}

// Step 6 Hardpoint별 pending EquipmentPreset object path 문자열을 반환합니다.
FString SCFVehicleBuilderTab::GetPendingMountPresetPath(const FName LocationSlotId) const
{
	const FSoftObjectPath* PresetPath = PendingMountPresetPaths.Find(LocationSlotId);
	return PresetPath && PresetPath->IsValid() ? PresetPath->ToString() : FString();
}

// Step 6 Gameplay Setup 전용 R0 guidance UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetGameplaySetupVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::GameplaySetup
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 6에서 초보자가 남은 작업과 다음 행동을 바로 이해할 수 있는 짧은 요약을 표시합니다.
FText SCFVehicleBuilderTab::GetGameplaySetupUserSummaryText() const
{
	if (!ViewModel.IsValid())
	{
		return LOCTEXT("GameplaySetupUserSummaryUnavailable", "현재 게임플레이 설정 상태를 읽을 수 없습니다. '현재 상태 다시 확인'을 눌러주세요.");
	}

	if (!ViewModel->HasGameplayGuidanceResult())
	{
		return LOCTEXT("GameplaySetupUserSummaryNeedRefresh", "현재 설정을 아직 확인하지 못했습니다. '현재 상태 다시 확인'을 눌러주세요.");
	}

	// 현재 Gameplay Setup의 fresh authoritative guidance 결과입니다.
	const FCFBuilderGameplayGuidanceResult& GuidanceResult = ViewModel->GetGameplayGuidanceResult();
	// Mount 영역의 completeness는 Hardpoint/Mount 개수를 다시 계산하지 않고 existing Guidance authority를 그대로 사용합니다.
	const FCFBuilderGameplayGuidanceItem* MountGuidance = GuidanceResult.Items.FindByPredicate([](const FCFBuilderGameplayGuidanceItem& Item)
	{
		return Item.Area == ECFBuilderGameplayArea::MountProfiles;
	});
	if (MountGuidance
		&& (MountGuidance->State == ECFBuilderGuidanceState::NeedsReview
			|| MountGuidance->State == ECFBuilderGuidanceState::Blocked))
	{
		FString MountMessage = TEXT("장비 장착 규칙을 확인해야 합니다.");
		if (!MountGuidance->Summary.IsEmpty())
		{
			MountMessage += FString::Printf(TEXT("\n%s"), *MountGuidance->Summary);
		}
		if (!MountGuidance->ResolutionText.IsEmpty())
		{
			MountMessage += FString::Printf(TEXT("\n%s"), *MountGuidance->ResolutionText);
		}
		MountMessage += TEXT("\n위 장착 위치별 설정을 정리한 뒤 '현재 상태 다시 확인'을 눌러주세요.");
		return FText::FromString(MountMessage);
	}

	if (GuidanceResult.BlockedCount > 0)
	{
		return FText::FromString(FString::Printf(
			TEXT("진행을 막는 설정 문제가 %d개 있습니다. 먼저 위 장착 규칙을 확인하고, 해결되지 않으면 아래 '진단 정보'에서 막힌 항목의 원인을 확인한 뒤 '현재 상태 다시 확인'을 눌러주세요."),
			GuidanceResult.BlockedCount));
	}

	if (GuidanceResult.NeedsReviewCount > 0)
	{
		return FText::FromString(FString::Printf(
			TEXT("사용자가 확인하거나 수동으로 처리할 항목이 %d개 남아 있습니다. 필요한 설정이나 Socket 위치를 확인한 뒤 '현재 상태 다시 확인'을 눌러주세요. 자세한 원인이 필요할 때만 아래 '진단 정보'를 열면 됩니다."),
			GuidanceResult.NeedsReviewCount));
	}

	if (GuidanceResult.bCanCompleteGameplayStep)
	{
		if (GuidanceResult.PendingGameplayDiffCount > 0)
		{
			return FText::FromString(FString::Printf(
				TEXT("이 단계의 수동 작업은 완료되었습니다. VehicleData에 반영될 변경 예정 항목 %d개는 7단계 Final Review에서 다시 확인합니다. '다음'으로 진행하세요."),
				GuidanceResult.PendingGameplayDiffCount));
		}

		return LOCTEXT(
			"GameplaySetupUserSummaryComplete",
			"이 단계에서 추가로 할 작업이 없습니다. '다음'을 눌러 7단계 Final Review로 진행하세요.");
	}

	return LOCTEXT(
		"GameplaySetupUserSummaryFallback",
		"설정 상태를 다시 확인해야 합니다. '현재 상태 다시 확인'을 누르고, 계속 같은 상태라면 '진단 정보'에서 원인을 확인하세요.");
}

// Step 6의 8영역 completeness/USER Socket/pending diff 정밀 진단 요약을 표시합니다.
FText SCFVehicleBuilderTab::GetGameplaySetupSummaryText() const
{
	return ViewModel.IsValid()
		? FText::FromString(ViewModel->BuildGameplayGuidanceSummary())
		: FText::GetEmpty();
}

// Step 6 Recipe authored Hardpoint↔Mount 관계를 실제 VehicleData 결과와 구분해 표시합니다.
FText SCFVehicleBuilderTab::GetGameplayMountSemanticSummaryText() const
{
	if (!ViewModel.IsValid() || !ViewModel->GetRecipe())
	{
		return FText::GetEmpty();
	}

	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	const UCFVehicleData* TargetVehicleData = Recipe->TargetVehicleData.Get();
	FCFBuilderHardpointSemanticFacts Facts;
	FString Error;
	if (!FCFVehicleBuilderHardpointIntegrity::BuildHardpointSemanticFacts(
		Recipe,
		TargetVehicleData,
		nullptr,
		Facts,
		Error))
	{
		return FText::FromString(TEXT("장비 장착 위치와 장착 규칙의 연결 상태를 읽을 수 없습니다."));
	}
	return FCFVehicleBuilderPresentation::BuildMountSemanticSummary(Facts);
}

// Step 7 Final Review 전용 R0 review/apply/undo UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetFinalReviewVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::FinalReview
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 7 typed Field Diff/apply readiness를 USER label + Before→After + 의미/영향으로 표시합니다.
FText SCFVehicleBuilderTab::GetFinalReviewSummaryText() const
{
	if (!ViewModel.IsValid() || !ViewModel->HasFinalReviewResult())
	{
		return LOCTEXT("FinalReviewSummaryUnavailable", "현재 최종 변경 목록을 읽을 수 없습니다. '현재 상태 다시 확인'을 눌러주세요.");
	}

	const FCFBuilderFinalReviewResult& Review = ViewModel->GetFinalReviewResult();
	FString Combined = FCFVehicleBuilderPresentation::BuildFinalReviewSummary(Review).ToString();

	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	const UCFVehicleData* TargetVehicleData = Recipe ? Recipe->TargetVehicleData.Get() : nullptr;
	FCFBuilderHardpointSemanticFacts Facts;
	FString Error;
	if (Recipe && FCFVehicleBuilderHardpointIntegrity::BuildHardpointSemanticFacts(
		Recipe,
		TargetVehicleData,
		&Review,
		Facts,
		Error))
	{
		Combined += TEXT("\n\n");
		Combined += FCFVehicleBuilderPresentation::BuildFinalReviewHardpointReadback(Facts).ToString();
	}
	return FText::FromString(Combined);
}

// Current Step 7 durable final commit action이 USER 실행 가능한지 반환합니다.
bool SCFVehicleBuilderTab::CanApplyFinalReview() const
{
	return ViewModel.IsValid()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::FinalReview
		&& ViewModel->HasFinalReviewResult()
		&& ViewModel->GetFinalCommitPreflight().CanExecute();
}

// Current Editor lifetime에 exact Builder guarded Undo token이 있는지 반환합니다.
bool SCFVehicleBuilderTab::CanUndoFinalReview() const
{
	return ViewModel.IsValid()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::FinalReview
		&& ViewModel->HasFinalReviewUndoToken();
}

// Step 8 Technical Driving / USER Driving 전용 UI 표시 조건입니다.
EVisibility SCFVehicleBuilderTab::GetDrivingTestVisibility() const
{
	return ViewModel.IsValid()
		&& ViewModel->HasSelection()
		&& !ViewModel->IsMeshOnlyCandidate()
		&& ViewModel->GetRecipe()
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::DrivingTest
		? EVisibility::Visible
		: EVisibility::Collapsed;
}

// Step 8 saved-state/technical metrics/USER checklist를 USER-facing 요약으로 표시합니다.
FText SCFVehicleBuilderTab::GetDrivingTestSummaryText() const
{
	if (!ViewModel.IsValid())
	{
		return LOCTEXT("DrivingSummaryUnavailable", "현재 주행 테스트 상태를 읽을 수 없습니다.");
	}

	// USER 주행 화면에 필요한 current saved-state와 typed benchmark projection입니다.
	FCFVehicleDrivingPresentInfo Info;
	const UCFVehicleRecipeData* Recipe = ViewModel->GetRecipe();
	const UCFVehicleData* TargetVehicleData = Recipe ? Recipe->TargetVehicleData.Get() : nullptr;
	Info.bRecipeDirty = Recipe && Recipe->GetOutermost()->IsDirty();
	Info.bTargetDirty = TargetVehicleData && TargetVehicleData->GetOutermost()->IsDirty();
	Info.bHasBenchmarkResult = ViewModel->HasDrivingBenchmarkResult();
	Info.bUserDrivePrepared = ViewModel->IsUserTestDrivePreparedThisSession();
	Info.bUserDrivingAccepted = ViewModel->HasCurrentUserDrivingAcceptance();
	if (Info.bHasBenchmarkResult)
	{
		Info.BenchmarkResult = ViewModel->GetDrivingBenchmarkResult();
	}

	FString Combined = FCFVehicleBuilderPresentation::BuildDrivingSummary(Info).ToString();
	FCFBuilderHardpointSemanticFacts Facts;
	FString Error;
	if (Recipe && FCFVehicleBuilderHardpointIntegrity::BuildHardpointSemanticFacts(
		Recipe,
		TargetVehicleData,
		nullptr,
		Facts,
		Error))
	{
		Combined += TEXT("\n\n");
		Combined += FCFVehicleBuilderPresentation::BuildDrivingHardpointReadback(Facts).ToString();
	}
	return FText::FromString(Combined);
}

// Step 8 PIE 적용 버튼의 stable blocker 또는 준비 완료 이유를 사용자용 문장으로 표시합니다.
FText SCFVehicleBuilderTab::GetDrivingApplyReadinessText() const
{
	if (!ViewModel.IsValid()
		|| ViewModel->GetStepViews().IsEmpty()
		|| ViewModel->GetCurrentStep().StepId != ECFVehicleBuilderStepId::DrivingTest)
	{
		return FText::GetEmpty();
	}

	if (bDrivingBenchmarkRunning)
	{
		// Benchmark-running 안내에는 persistent preflight를 다시 평가할 필요가 없어 기본 placeholder를 사용합니다.
		const FCFVehicleDrivingApplyPreflight PlaceholderPreflight;
		return FCFVehicleBuilderPresentation::BuildDrivingApplyReadiness(PlaceholderPreflight, true);
	}

	// Current stable persisted/benchmark blocker projection입니다.
	const FCFVehicleDrivingApplyPreflight Preflight = ViewModel->ReadDrivingApplyPreflight();
	return FCFVehicleBuilderPresentation::BuildDrivingApplyReadiness(Preflight, false);
}

// Running benchmark의 cached coarse phase와 monotonic 경과 시간을 USER-facing 문장으로 표시합니다.
FText SCFVehicleBuilderTab::GetDrivingBenchmarkProgressText() const
{
	// Current run 시작 뒤 경과한 monotonic seconds입니다.
	const double ElapsedSeconds = bDrivingBenchmarkRunning && DrivingBenchmarkStartedAtSeconds > 0.0
		? FMath::Max(0.0, FPlatformTime::Seconds() - DrivingBenchmarkStartedAtSeconds)
		: 0.0;
	return FCFVehicleBuilderPresentation::BuildDrivingBenchmarkProgress(
		bDrivingBenchmarkRunning,
		bHasDrivingBenchmarkProgress,
		DrivingBenchmarkProgress,
		ElapsedSeconds);
}

// Running benchmark의 cached coarse phase를 0~1 progress 값으로 표시하고 snapshot 전에는 unset을 반환합니다.
TOptional<float> SCFVehicleBuilderTab::GetDrivingBenchmarkProgressPercent() const
{
	if (!bDrivingBenchmarkRunning
		|| !bHasDrivingBenchmarkProgress
		|| DrivingBenchmarkProgress.PhaseCount <= 0
		|| DrivingBenchmarkProgress.PhaseIndex <= 0)
	{
		return TOptional<float>();
	}

	// 현재 coarse phase에 진입하기 전에 완료된 단계 수입니다. Finalizing 진입만으로 100%가 되지 않게 합니다.
	const int32 CompletedPhaseCount = FMath::Clamp(
		DrivingBenchmarkProgress.PhaseIndex - 1,
		0,
		DrivingBenchmarkProgress.PhaseCount);
	return FMath::Clamp(
		static_cast<float>(CompletedPhaseCount)
			/ static_cast<float>(DrivingBenchmarkProgress.PhaseCount),
		0.0f,
		1.0f);
}

// Benchmark process가 실행 중일 때만 progress 영역을 표시합니다.
EVisibility SCFVehicleBuilderTab::GetDrivingBenchmarkProgressVisibility() const
{
	return bDrivingBenchmarkRunning ? EVisibility::Visible : EVisibility::Collapsed;
}

// Current persistent USER Driving receipt/Recipe dirty 상태를 Save 의미와 함께 USER-facing으로 표시합니다.
FText SCFVehicleBuilderTab::GetDrivingRecipeSaveStatusText() const
{
	if (!ViewModel.IsValid()
		|| ViewModel->GetStepViews().IsEmpty()
		|| ViewModel->GetCurrentStep().StepId != ECFVehicleBuilderStepId::DrivingTest)
	{
		return FText::GetEmpty();
	}

	return FCFVehicleBuilderPresentation::BuildDrivingRecipeSaveStatus(
		ViewModel->ReadCurrentRecipeSavePreflight());
}

// Current Step 8 exact Recipe가 persistent receipt 기준으로 dirty일 때만 explicit Save 버튼을 활성화합니다.
bool SCFVehicleBuilderTab::CanSaveCurrentRecipe() const
{
	if (!ViewModel.IsValid()
		|| bDrivingBenchmarkRunning
		|| ViewModel->GetStepViews().IsEmpty()
		|| ViewModel->GetCurrentStep().StepId != ECFVehicleBuilderStepId::DrivingTest)
	{
		return false;
	}

	return ViewModel->ReadCurrentRecipeSavePreflight().CanSave();
}

// Current Step 8에서 benchmark child process를 새로 시작할 수 있는지 반환합니다.
bool SCFVehicleBuilderTab::CanRunDrivingBenchmark() const
{
	return ViewModel.IsValid()
		&& !bDrivingBenchmarkRunning
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::DrivingTest
		&& ViewModel->GetCurrentStep().State != ECFVehicleBuilderStepState::Locked
		&& ViewModel->GetCurrentStep().State != ECFVehicleBuilderStepState::Blocked;
}

// Current Step 8 stable preflight가 모두 충족됐을 때만 PIE transient 적용 버튼을 활성화합니다.
bool SCFVehicleBuilderTab::CanApplyDrivingTargetToPIE() const
{
	if (!ViewModel.IsValid()
		|| bDrivingBenchmarkRunning
		|| ViewModel->GetStepViews().IsEmpty()
		|| ViewModel->GetCurrentStep().StepId != ECFVehicleBuilderStepId::DrivingTest)
	{
		return false;
	}

	// Production Apply guard와 동일한 current stable preflight입니다.
	const FCFVehicleDrivingApplyPreflight Preflight = ViewModel->ReadDrivingApplyPreflight();
	return Preflight.CanApply();
}

// Current benchmark가 exact current Target에 binding됐을 때 USER explicit PASS 버튼을 활성화합니다.
bool SCFVehicleBuilderTab::CanAcceptUserDriving() const
{
	return ViewModel.IsValid()
		&& !bDrivingBenchmarkRunning
		&& !ViewModel->GetStepViews().IsEmpty()
		&& ViewModel->GetCurrentStep().StepId == ECFVehicleBuilderStepId::DrivingTest
		&& ViewModel->HasDrivingBenchmarkResult()
		&& ViewModel->IsUserTestDrivePreparedThisSession()
		&& !ViewModel->HasCurrentUserDrivingAcceptance();
}

// 기존 Advanced Vehicle Authoring Workspace를 별도 탭으로 엽니다.
FReply SCFVehicleBuilderTab::HandleOpenAdvancedWorkspace()
{
	FGlobalTabmanager::Get()->TryInvokeTab(FName(TEXT("CarFight.VehicleAuthoring")));
	return FReply::Handled();
}

// 현재 선택을 USER-facing 한 줄로 표시합니다.
FText SCFVehicleBuilderTab::GetSelectionText() const
{
	if (ViewModel.IsValid() && ViewModel->IsNewVehicleEntryActive())
	{
		return LOCTEXT("NewVehicleSelection", "대상: 새 차량 만들기 (신규 제작 시작)");
	}

	if (!ViewModel.IsValid() || !ViewModel->HasSelection())
	{
		return LOCTEXT("NoSelection", "대상: 선택 안 됨");
	}

	return FText::FromString(FString::Printf(
		TEXT("대상: %s"),
		*VehicleRowTitle(ViewModel->GetSelectedEntry())));
}

// 현재 Step 제목을 초보자용 화면 명칭으로 표시합니다.
FText SCFVehicleBuilderTab::GetCurrentStepTitle() const
{
	if (!ViewModel.IsValid() || ViewModel->GetStepViews().IsEmpty())
	{
		return LOCTEXT("NoStep", "단계 없음");
	}

	// 현재 표시 중인 Step projection입니다.
	const FCFVehicleBuilderStepView& Step = ViewModel->GetCurrentStep();
	// Stable StepId를 사용자용 명칭으로 변환한 제목입니다.
	const FText DisplayTitle = FCFVehicleBuilderPresentation::GetStepTitle(Step.StepId, Step.Title);
	return FText::FromString(FString::Printf(TEXT("%d. %s"), Step.StepNumber, *DisplayTitle.ToString()));
}

// 현재 Step 상태에서 내부 Provider 표기를 숨기고 사용자가 필요한 진행 상태만 표시합니다.
FText SCFVehicleBuilderTab::GetCurrentStepStateText() const
{
	if (!ViewModel.IsValid() || ViewModel->GetStepViews().IsEmpty())
	{
		return FText::GetEmpty();
	}

	// 현재 표시 중인 Step projection입니다.
	const FCFVehicleBuilderStepView& Step = ViewModel->GetCurrentStep();
	return FText::FromString(FString::Printf(
		TEXT("상태: %s"),
		*FCFVehicleBuilderPresentation::GetStepStateLabel(Step.State)));
}

// 현재 Step 설명을 기본 화면에서 이해하기 쉬운 목적 설명으로 표시합니다.
FText SCFVehicleBuilderTab::GetCurrentStepSummaryText() const
{
	if (!ViewModel.IsValid() || ViewModel->GetStepViews().IsEmpty())
	{
		return FText::GetEmpty();
	}

	// 현재 표시 중인 Step projection입니다.
	const FCFVehicleBuilderStepView& Step = ViewModel->GetCurrentStep();
	switch (Step.StepId)
	{
	case ECFVehicleBuilderStepId::IdentityReference:
	case ECFVehicleBuilderStepId::MeshPrep:
	case ECFVehicleBuilderStepId::SocketGuide:
	case ECFVehicleBuilderStepId::LayoutCapture:
	case ECFVehicleBuilderStepId::FinalReview:
	case ECFVehicleBuilderStepId::DrivingTest:
		return FCFVehicleBuilderPresentation::GetStepPurpose(Step.StepId, Step.Summary);
	case ECFVehicleBuilderStepId::PhysicsProposal:
		return LOCTEXT("PhysicsProposalPlainSummary", "AI가 준비한 차량의 엔진, 변속기, 브레이크 등 주행 물리 설정을 확인하는 단계입니다.");
	case ECFVehicleBuilderStepId::GameplaySetup:
		return LOCTEXT("GameplaySetupPlainSummary", "무기와 장비를 달 장착 위치의 규칙과 차량의 게임플레이 설정을 확인하는 단계입니다.");
	default:
		return Step.Summary;
	}
}

// 현재 Step 해결/다음 행동을 사용자용 구체 행동으로 표시합니다.
FText SCFVehicleBuilderTab::GetCurrentStepResolutionText() const
{
	if (!ViewModel.IsValid() || ViewModel->GetStepViews().IsEmpty())
	{
		return FText::GetEmpty();
	}

	// 현재 표시 중인 Step projection입니다.
	const FCFVehicleBuilderStepView& Step = ViewModel->GetCurrentStep();
	switch (Step.StepId)
	{
	case ECFVehicleBuilderStepId::IdentityReference:
	case ECFVehicleBuilderStepId::MeshPrep:
	case ECFVehicleBuilderStepId::SocketGuide:
	case ECFVehicleBuilderStepId::LayoutCapture:
	case ECFVehicleBuilderStepId::FinalReview:
	case ECFVehicleBuilderStepId::DrivingTest:
		return FCFVehicleBuilderPresentation::GetStepNextAction(Step, Step.Resolution);
	case ECFVehicleBuilderStepId::PhysicsProposal:
		return GetPhysicsProposalUserSummaryText();
	case ECFVehicleBuilderStepId::GameplaySetup:
		return GetGameplaySetupUserSummaryText();
	default:
		return Step.Resolution;
	}
}

// Step button label을 상태와 함께 표시합니다.
FText SCFVehicleBuilderTab::GetStepButtonText(const int32 StepIndex) const
{
	if (!ViewModel.IsValid() || !ViewModel->GetStepViews().IsValidIndex(StepIndex))
	{
		return FText::FromString(FString::Printf(TEXT("%d. -"), StepIndex + 1));
	}

	// 목록에서 표시할 Step projection입니다.
	const FCFVehicleBuilderStepView& Step = ViewModel->GetStepViews()[StepIndex];
	// Stable StepId를 사용자용 명칭으로 변환한 제목입니다.
	const FText DisplayTitle = FCFVehicleBuilderPresentation::GetStepTitle(Step.StepId, Step.Title);
	return FText::FromString(FString::Printf(
		TEXT("%d. %s  [%s]"),
		Step.StepNumber,
		*DisplayTitle.ToString(),
		*FCFVehicleBuilderPresentation::GetStepStateLabel(Step.State)));
}

// Step button을 현재 state에서 클릭할 수 있는지 반환합니다.
bool SCFVehicleBuilderTab::IsStepButtonEnabled(const int32 StepIndex) const
{
	if (!ViewModel.IsValid() || !ViewModel->GetStepViews().IsValidIndex(StepIndex))
	{
		return false;
	}

	const ECFVehicleBuilderStepState State = ViewModel->GetStepViews()[StepIndex].State;
	return State != ECFVehicleBuilderStepState::Locked && State != ECFVehicleBuilderStepState::Unavailable;
}

// Previous 버튼 활성 조건입니다.
bool SCFVehicleBuilderTab::CanMovePrevious() const
{
	return ViewModel.IsValid() && ViewModel->GetCurrentStepIndex() > 0;
}

// Next 버튼 활성 조건입니다.
bool SCFVehicleBuilderTab::CanMoveNext() const
{
	if (!ViewModel.IsValid() || ViewModel->GetStepViews().IsEmpty())
	{
		return false;
	}

	return ViewModel->CanAdvanceFromCurrentStep();
}

#undef LOCTEXT_NAMESPACE
