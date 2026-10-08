v: 1
k: generic_plan_compile
g: CarFight 프로젝트에 Aim 기반 발사 요청의 1차 서버 검증 흐름을 추가한다
tf:
  - UE/Source/CarFight_Re/Public/CFVehiclePawn.h
  - UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
  - UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
  - UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
  - UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
  - UE/Source/CarFight_Re/Public/UI/CFVehicleDebugPanelWidget.h
  - UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
scope:
  in:
    - 이번 작업에 포함한다
    - ACFVehiclePawn에 InputAction_Fire UPROPERTY를 추가한다
    - SetupPlayerInputComponent()에서 InputAction_Fire가 유효할 때 Started 입력을 HandleFireStarted()에 바인딩한다
    - "HandleFireStarted(const FInputActionValue& InputActionValue)를 추가한다"
    - BuildFireRequest() 함수를 추가한다
    - NextFireRequestId 또는 동등한 요청 ID 카운터를 추가한다
    - LastFireRequest와 LastFireResult 디버그용 캐시를 추가한다
    - "ServerRequestFire(const FCFVehicleFireRequest& FireRequest) RPC를 추가한다"
    - "ClientReceiveFireResult(const FCFVehicleFireResult& FireResult) RPC를 추가한다"
    - 서버 검증 함수 ValidateFireRequestOnServer() 또는 동등한 helper를 추가한다
    - 서버 검증 성공 시 FCFVehicleFireResult.bAccepted = true를 반환한다
    - 서버 검증 실패 시 적절한 ECFVehicleFireRejectReason을 반환한다
    - 서버 검증 결과를 VehicleAimComp 또는 Pawn의 Debug 상태에 반영할 최소 경로를 추가한다
    - FCFVehicleDebugAim 또는 VehicleDebug Aim 표시에서 Last Fire Request / Result / Reject Reason을 확인할 수 있게 한다
    - 서버 검증은 최소한 다음을 검사한다
    - 요청 Pawn 소유권 또는 Controller 유효성
    - AimComp 유효성
    - Aim runtime ready
    - AimDirection 유효성
    - AimOrigin 유효성
    - 로컬/기본 AimProfile 기준 Weapon Arc 유효성
    - Aim blocked 여부
  out:
    - 이번 작업에서 하지 않는다
    - IA_Fire InputAction 에셋 생성 금지
    - Input Mapping Context 에셋 수정 금지
    - BP 에셋 수정 금지
    - Reticle UI 생성 금지
    - HitScan Trace 구현 금지
    - Projectile 구현 금지
    - Damage 구현 금지
    - UCFVehicleWeaponComp 생성 금지
    - Ammo / Cooldown / Reload 완성 구현 금지
    - Multicast Fire FX 구현 금지
    - RepAimVisual 복제 구현 금지
    - Lock-On 구현 금지
    - AI Combat 구현 금지
    - VehicleDebug Panel WBP Designer 수정 금지
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - 파일명은 32자를 넘기지 않는다
  - 기존 주행/조향/카메라 입력 동작을 변경하지 않는다
  - InputAction_Fire가 null이면 아무 입력 바인딩도 하지 않고 크래시하지 않는다
  - 이번 작업에서 Fire Input Action 에셋을 생성하지 않는다
  - 클라이언트가 보낸 Fire Request를 그대로 신뢰하지 않는다
  - HitScan/Damage/Projectile 로직을 넣지 않는다
  - UCFVehicleAimComp 안에 WeaponComp 완성 책임을 넣지 않는다
  - 빌드 실패 상태로 끝내지 않는다
acc:
  - 검증 기준은 다음과 같다
  - CarFight_ReEditor / Win64 / Development 빌드가 성공해야 한다
  - 수정 파일은 기본적으로 Target Files 4개여야 한다
  - CFVehicleDebugPanelWidget.h/.cpp는 가능하면 변경하지 않아야 한다
  - InputAction_Fire가 비어 있어도 기존 입력 바인딩이 유지되어야 한다
  - Single Player PIE에서 크래시 없이 시작되어야 한다
  - Listen Server + 1 Client에서 Fire Action이 연결된 경우 ServerRequestFire가 호출될 수 있어야 한다
  - 서버 검증 결과가 FCFVehicleServerAimState에 반영되어야 한다
  - VehicleDebug Aim에서 Last Reject Reason / Last Accepted ID / Last Rejected ID가 확인 가능해야 한다
  - HitScan/Damage/Projectile/Multicast FX 코드가 없음을 확인한다
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
  source: Document/ProjectSSOT/Plan/AimPlan/Generated/CF_AimCodexSource_R5R6.md
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
