v: 1
k: generic_plan_compile
g: CarFight 프로젝트에 Aim Reticle UI용 C++ 부모 위젯을 추가한다
tf:
  - UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
  - UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
  - UE/Source/CarFight_Re/Public/UI/CFVehicleDebugHudWidget.h
  - UE/Source/CarFight_Re/Private/UI/CFVehicleDebugHudWidget.cpp
  - UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
  - UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
  - UE/Source/CarFight_Re/Public/CFVehiclePawn.h
scope:
  in:
    - 이번 작업에 포함한다
    - CFAimReticleWidget.h 생성
    - CFAimReticleWidget.cpp 생성
    - "UCFAimReticleWidget: UUserWidget 클래스 생성"
    - "SetVehiclePawnRef(ACFVehiclePawn* InVehiclePawnRef) 추가"
    - RefreshFromPawn() 추가
    - ApplyReticleState(ECFVehicleReticleState InReticleState) 추가
    - UpdateReticleVisibility() 추가
    - GetReticleStateDisplayText() 또는 동등한 BlueprintPure getter 추가
    - GetCanFireDisplayText() 또는 동등한 BlueprintPure getter 추가
    - NativeConstruct()에서 초기 갱신 수행
    - NativeTick()에서 bAutoRefreshEveryTick가 true일 때 자동 갱신
    - BindWidgetOptional TextBlock 후보를 추가한다
    - AimComp가 없거나 Pawn이 없을 때 크래시하지 않고 Hidden 상태로 처리한다
    - Reticle State별 텍스트와 가시성을 최소 구분한다
  out:
    - 이번 작업에서 하지 않는다
    - WBP_AimReticle 에셋 생성 금지
    - WBP Designer 수정 금지
    - HUD 또는 Viewport에 Reticle 자동 생성/배치 금지
    - ACFVehiclePawn에 ReticleWidgetClass 추가 금지
    - PlayerController/HUD 클래스 생성 금지
    - Reticle 이미지/머티리얼/애니메이션 구현 금지
    - Fire Input / RPC / Server Validation 수정 금지
    - Damage 적용 금지
    - Projectile 구현 금지
    - Multicast Fire FX 구현 금지
    - RepAimVisual 로직 변경 금지
    - WeaponComp 생성 금지
    - BP 에셋 수정 금지
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - 파일명은 32자를 넘기지 않는다
  - Reticle UI는 직접 Trace를 수행하지 않는다
  - Reticle UI는 Fire RPC를 호출하지 않는다
  - Reticle UI는 Damage/Weapon/Projectile을 처리하지 않는다
  - 빌드 실패 상태로 끝내지 않는다
acc:
  - 검증 기준은 다음과 같다
  - CarFight_ReEditor / Win64 / Development 빌드가 성공해야 한다
  - 신규 파일 2개만 생성되는 것이 기본이다
  - "기존 CFVehiclePawn, CFVehicleAimComp, VehicleDebug 파일이 변경되지 않아야 한다"
  - UCFAimReticleWidget이 Blueprintable 위젯 부모로 노출되어야 한다
  - WBP 자식 없이도 C++ 컴파일이 성공해야 한다
  - TextBlock이 BindWidgetOptional이므로 WBP에 해당 위젯이 없어도 크래시하지 않아야 한다
  - Reticle UI가 Trace/RPC/Damage/Projectile을 호출하지 않는지 확인한다
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
  source: Document/ProjectSSOT/Plan/AimPlan/Generated/CF_AimCodexSource_R9.md
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
