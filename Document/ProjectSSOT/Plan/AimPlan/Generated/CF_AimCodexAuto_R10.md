v: 1
k: generic_plan_compile
g: CarFight 프로젝트에서 R9에 추가된 UCFAimReticleWidget을 ACFVehiclePawn이 선택적으로 생성하고 Viewport에 추가할 수 있는 C++ 연결 기반을 추가한다
tf:
  - UE/Source/CarFight_Re/Public/CFVehiclePawn.h
  - UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
  - UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
  - UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
scope:
  in:
    - 이번 작업에 포함한다
    - ACFVehiclePawn에 AimReticleWidgetClass UPROPERTY를 추가한다
    - ACFVehiclePawn에 AimReticleWidgetInstance 런타임 캐시를 추가한다
    - ACFVehiclePawn에 bShowAimReticle 토글을 추가한다
    - ACFVehiclePawn에 AimReticleZOrder 설정값을 추가한다
    - ShouldShowAimReticle() BlueprintPure 함수를 추가한다
    - CreateAimReticleWidget() BlueprintCallable 함수를 추가한다
    - DestroyAimReticleWidget() BlueprintCallable 함수를 추가한다
    - RefreshAimReticleWidget() BlueprintCallable 함수를 추가한다
    - BeginPlay() 또는 기존 런타임 초기화 흐름 이후에 CreateAimReticleWidget()을 호출한다
    - EndPlay() override를 추가해 DestroyAimReticleWidget()을 호출한다
    - Reticle Widget은 로컬 제어 Pawn에서만 생성한다
    - AimReticleWidgetClass == nullptr이면 아무것도 생성하지 않고 안전하게 종료한다
    - 생성된 Widget에는 SetVehiclePawnRef(this)를 호출한다
    - "기존 주행, 입력, AimComp, VehicleDebug 흐름을 유지한다"
  out:
    - 이번 작업에서 하지 않는다
    - WBP_AimReticle 에셋 생성 금지
    - WBP Designer 수정 금지
    - Reticle 이미지/머티리얼/애니메이션 구현 금지
    - PlayerController/HUD 클래스 생성 금지
    - Fire Input / RPC / Server Validation 수정 금지
    - HitScan / Damage / Projectile 수정 금지
    - RepAimVisual 로직 수정 금지
    - WeaponComp 생성 금지
    - Input Mapping Context 수정 금지
    - BP 에셋 값 지정 금지
    - Multicast Fire FX 구현 금지
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - 파일명은 32자를 넘기지 않는다
  - 서버 전용 또는 Simulated Proxy에서 Widget을 생성하지 않는다
  - AimReticleWidgetClass가 null이면 크래시하지 않는다
  - 이미 Widget Instance가 있으면 중복 생성하지 않는다
  - UI 연결은 표시 전용이다. Trace/RPC/Damage/Weapon 로직을 호출하지 않는다
  - 기존 VehicleDebug UI 토글과 Aim Reticle 토글을 섞지 않는다
  - 기존 bEnableDriveStateOnScreenDebug는 Reticle 표시 조건으로 사용하지 않는다
  - 빌드 실패 상태로 끝내지 않는다
acc:
  - 검증 기준은 다음과 같다
  - CarFight_ReEditor / Win64 / Development 빌드가 성공해야 한다
  - 수정 파일은 CFVehiclePawn.h/.cpp여야 한다
  - CFAimReticleWidget.h/.cpp는 변경하지 않는 것이 기본이다
  - AimReticleWidgetClass가 null인 상태에서도 PIE 시작 시 크래시하지 않아야 한다
  - BP에서 AimReticleWidgetClass에 WBP 자식을 지정할 수 있어야 한다
  - WBP가 지정된 로컬 Pawn에서만 Widget이 생성될 수 있어야 한다
  - Simulated Proxy나 서버 전용 경로에서 Widget이 생성되지 않아야 한다
  - 기존 VehicleDebug HUD/Panel 동작이 변경되지 않아야 한다
  - Reticle 연결 코드가 Trace/RPC/Damage/Projectile을 호출하지 않는지 확인한다
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
  source: Document/ProjectSSOT/Plan/AimPlan/Generated/CF_AimCodexSource_R10.md
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
