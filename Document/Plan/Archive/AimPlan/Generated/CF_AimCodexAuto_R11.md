v: 1
k: generic_plan_compile
g: CarFight 프로젝트에서 지금까지 C++로 준비한 Aim Reticle과 Fire Request 흐름을 실제 에셋에 연결한다
tf:
  - Document/ProjectSSOT/Plan/AimPlan/CF_AimRoadmap.md
scope:
  in:
    - 이번 작업에 포함한다
    - /Game/CarFight/UI/WBP_AimReticle 위젯 블루프린트를 생성한다
    - WBP_AimReticle의 부모 클래스를 UCFAimReticleWidget으로 설정한다
    - WBP_AimReticle에 최소 표시용 TextBlock 3개를 배치한다
    - Text_ReticleState
    - Text_CanFire
    - Text_ReticleHint
    - 세 TextBlock은 C++의 BindWidgetOptional 이름과 정확히 맞춘다
    - /Game/CarFight/Input/IA_Fire Input Action을 생성한다
    - IA_Fire는 버튼/Bool 계열 입력으로 설정한다
    - /Game/CarFight/Input/IMC_Vehicle_Default에 Fire 입력 매핑을 추가한다
    - Fire 입력 기본 키 후보를 하나 이상 지정한다
    - Mouse Left Button
    - Gamepad Right Trigger 또는 Gamepad Face Button Right 후보
    - /Game/CarFight/Vehicles/BP_CFVehiclePawn의 InputAction_Fire에 IA_Fire를 지정한다
    - /Game/CarFight/Vehicles/BP_CFVehiclePawn의 AimReticleWidgetClass에 WBP_AimReticle을 지정한다
    - 저장 후 에셋 목록에 생성 여부가 확인되어야 한다
  out:
    - 이번 작업에서 하지 않는다
    - C++ 코드 수정 금지
    - Damage 적용 금지
    - Projectile 구현 금지
    - Multicast Fire FX 구현 금지
    - WeaponComp 생성 금지
    - Reticle 이미지/머티리얼/복잡한 애니메이션 구현 금지
    - Lock-On 구현 금지
    - AI Combat 구현 금지
    - VehicleDebug WBP 수정 금지
    - 기존 주행 입력 매핑 삭제 금지
    - 기존 Look/Throttle/Brake/Steering 매핑 변경 금지
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - C++ 파일을 수정하지 않는다
  - 기존 Input Mapping Context의 기존 매핑을 삭제하지 않는다
  - 기존 VehicleDebug UI 에셋을 수정하지 않는다
acc:
  - 검증 기준은 다음과 같다
  - 에셋 목록에서 WBP_AimReticle이 확인되어야 한다
  - 에셋 목록에서 IA_Fire가 확인되어야 한다
  - BP_CFVehiclePawn Details에서 AimReticleWidgetClass가 지정되어 있어야 한다
  - BP_CFVehiclePawn Details에서 InputAction_Fire가 지정되어 있어야 한다
  - Single Player PIE에서 로컬 Pawn이 생성될 때 Reticle Widget이 화면에 나타나야 한다
  - Look 입력에 따라 Reticle 상태 텍스트가 갱신될 수 있어야 한다
  - Fire 입력을 눌렀을 때 크래시하지 않아야 한다
  - VehicleDebug Aim에서 Last Fire Request / Server State가 갱신될 수 있어야 한다
  - 기존 차량 이동/카메라 입력이 유지되어야 한다
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
  source: Document/ProjectSSOT/Plan/AimPlan/Generated/CF_AimCodexSource_R11.md
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
