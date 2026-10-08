v: 1
k: generic_plan_compile
g: CarFight 프로젝트의 VehicleDebug 시스템에 Aim 카테고리를 추가한다
tf:
  - UE/Source/CarFight_Re/Public/CFVehiclePawn.h
  - UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
  - UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h
  - UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
  - UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
  - UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
  - UE/Source/CarFight_Re/Public/UI/CFDebugPanelViewData.h
scope:
  in:
    - 이번 작업에 포함한다
    - CFVehiclePawn.h에 FCFVehicleDebugAim 구조체를 추가한다
    - FCFVehicleDebugSnapshot에 Aim 카테고리를 추가한다
    - ACFVehiclePawn에 GetVehicleDebugAim() BlueprintPure getter를 추가한다
    - "ACFVehiclePawn::GetVehicleDebugSnapshot()에서 VehicleAimComp 상태를 읽어 DebugSnapshot.Aim을 채운다"
    - "ACFVehiclePawn::GetVehicleDebugAim()을 구현한다"
    - UCFVehicleDebugPanelWidget에 CachedAim 멤버를 추가한다
    - "RefreshFromPawn()에서 VehiclePawnRef->GetVehicleDebugAim()을 읽어 CachedAim에 저장한다"
    - BuildVehicleDebugPanelViewData()에 Aim Section을 추가한다
    - "BuildAimSectionViewData(const FCFVehicleDebugAim& InAim) 선언과 구현을 추가한다"
    - "Aim Section은 Camera 다음, Runtime 이전에 표시한다"
    - Aim Section Navigation 설정은 다음을 권장한다
    - "SectionId: Aim"
    - "Title: 조준"
    - "NavigationGroup: Vehicle"
    - "NavigationOrder: 45"
    - "BadgeText: Reticle 상태 또는 Ready/Blocked/OutOfArc 요약"
  out:
    - 이번 작업에서 하지 않는다
    - UCFVehicleAimComp Local Aim 계산 로직 변경 금지
    - ServerRequestFire() 구현 금지
    - InputAction_Fire 생성 금지
    - Fire Request 생성 금지
    - Server Fire 검증 금지
    - HitScan 구현 금지
    - Damage 구현 금지
    - Projectile 구현 금지
    - Reticle UI 생성 금지
    - BP 에셋 수정 금지
    - Replication 설정 금지
    - WBP Designer 수정 금지
    - UCFVehicleWeaponComp 생성 금지
    - CameraComp 내부 로직 수정 금지
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - 파일명은 32자를 넘기지 않는다
  - 기존 FCFVehicleDebugSnapshot 구조를 제거하거나 이름 변경하지 않는다
  - "기존 Overview, Drive, Input, Camera, Runtime 카테고리를 제거하지 않는다"
  - 기존 BuildCameraSectionViewData() 동작을 변경하지 않는다
  - ServerAimState와 RepAimVisualState는 현재 기본값을 읽어 표시만 한다. 계산하지 않는다
  - Fire/RPC/HitScan/Damage/UI 생성 로직을 넣지 않는다
  - 빌드 실패 상태로 끝내지 않는다
acc:
  - 검증 기준은 다음과 같다
  - CarFight_ReEditor / Win64 / Development 빌드가 성공해야 한다
  - 수정 파일은 기본적으로 Target Files 4개여야 한다
  - CFVehicleAimComp.h/.cpp가 이번 작업에서 변경되지 않아야 한다
  - CFVehicleCameraComp.h/.cpp가 이번 작업에서 변경되지 않아야 한다
  - VehicleDebug Panel ViewData에 Aim Section이 추가되어야 한다
  - "BuildVehicleDebugPanelViewData()의 Section 순서는 Overview, Drive, Input, Camera, Aim, Runtime이어야 한다"
  - AimComp가 유효하면 LocalAimState 값이 Aim Section에 반영되어야 한다
  - AimComp가 없거나 준비되지 않아도 크래시하지 않아야 한다
  - 기존 Camera Section이 유지되어야 한다
  - 기존 VehicleDebug Navigation 구조가 유지되어야 한다
verify:
  - acc 항목 기준으로 결과를 수동 검증한다
fail:
  - acceptance_criteria를 충족하지 못하면 성공이 아니다
  - must_change_targets 검토 없이 수정 파일 0개로 종료하면 재검토가 필요하다
  - 대상 파일 변경 필요성을 확인하지 않고 이미 구현됨으로 종료하면 성공이 아니다
  - failure_reproduction 입력이 남아 있으면 원인 확인 없이 성공 처리하면 안 된다
  - target_files 검토 없이 수정 파일 0개로 종료하면 성공이 아니다
ask_if_missing:
  - "목표, 범위, 검증 조건 중 하나라도 불명확하면 구현하지 말고 missing contract를 보고한다"
ref:
  source: Document/ProjectSSOT/Plan/AimPlan/Generated/CF_AimCodexSource_R4.md
metadata:
  ssot_preflight:
    checked: true
    profile: codex_task
    status: ok
    blocked: false
    warn_only: true
    documents:
      - GoPyMCP/Workspace/docs/mcp_ssot/MCP_SSOT_Master.md
      - GoPyMCP/Workspace/docs/mcp_ssot/MCP_Golden_Path.md
      - GoPyMCP/Workspace/docs/mcp_ssot/MCP_Error_Guide.md
    applied_rules:
      - "Codex task documents should include goal, scope, forbidden work, and verification criteria."
      - Check current MCP SSOT before generating MCP implementation instructions.
      - Route Codex task requests to compact contract generation before long-form Markdown summaries.
      - Use plan.generate_task_contracts for plan-folder work-order generation before legacy compile_outputs flows.
      - Do not fallback to manual long-form Markdown when the plan automation chain fails.
    warnings:
      - project document path ignored for MCP SSOT preflight
