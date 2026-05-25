v: 1
k: generic_plan_compile
g: CarFight 프로젝트의 Aim 시스템에 다른 클라이언트가 볼 수 있는 최소 조준 시각 상태 복제 기반을 추가한다
tf:
  - UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
  - UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
  - UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
  - UE/Source/CarFight_Re/Public/CFVehiclePawn.h
  - UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
  - UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
scope:
  in:
    - 이번 작업에 포함한다
    - UCFVehicleAimComp를 replication 가능한 컴포넌트로 설정한다
    - RepAimVisualState를 ReplicatedUsing=OnRep_RepAimVisualState로 설정한다
    - "UCFVehicleAimComp::GetLifetimeReplicatedProps()를 추가한다"
    - 필요한 include로 Net/UnrealNetwork.h를 cpp에 추가한다
    - OnRep_RepAimVisualState() 함수를 추가한다
    - 서버에서 RepAimVisualState를 갱신하는 함수 UpdateRepAimVisualFromFireResult() 또는 동등한 함수를 추가한다
    - 서버 발사 검증 성공/실패 결과를 바탕으로 RepAimVisualState를 최소 갱신한다
    - 승인된 FireResult에서는 다음 값을 반영한다
    - "RepAimDirection: FireRequest 또는 FireResult 기준 조준 방향"
    - "RepAimTargetLocation: ServerAimTargetLocation 또는 ServerHitLocation"
    - "bIsFiringVisual: true로 설정 가능"
    - "RepWeaponVisualMode: FireRequest.WeaponGroupId 또는 DefaultAimProfile.ProfileName"
    - 거부된 FireResult에서는 bIsFiringVisual = false로 유지 또는 갱신한다
    - "ACFVehiclePawn::ServerRequestFire_Implementation()에서 서버 결과가 나온 후 AimComp의 RepAimVisual 갱신 함수를 호출한다"
    - VehicleDebug Aim에서 RepAimVisual 값이 갱신된 상태를 확인할 수 있게 기존 getter 경로를 유지한다
  out:
    - 이번 작업에서 하지 않는다
    - Reticle UI 생성 금지
    - WBP 에셋 수정 금지
    - BP 에셋 수정 금지
    - Multicast Fire FX 구현 금지
    - 실제 총구 이펙트 재생 금지
    - Damage 적용 금지
    - Projectile 구현 금지
    - UCFVehicleWeaponComp 생성 금지
    - Ammo / Cooldown / Reload 구현 금지
    - Lock-On 구현 금지
    - AI Combat 구현 금지
    - 복잡한 네트워크 압축 최적화 금지
    - Pawn 전체 네트워크 구조를 대규모로 재설계 금지
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - 파일명은 32자를 넘기지 않는다
  - FCFVehicleRepAimVisualState의 기존 필드명을 변경하지 않는다
  - 기존 Local Aim 계산을 변경하지 않는다
  - 기존 Server Fire 검증 순서를 변경하지 않는다
  - 기존 Server HitScan Trace 흐름을 변경하지 않는다
  - RepAimVisual은 전투 판정용으로 사용하지 않는다
  - "Damage, Projectile, Multicast FX, Reticle UI를 구현하지 않는다"
  - UCFVehicleAimComp가 Owner Pawn을 찾지 못해도 크래시하지 않는다
  - Replication 관련 include 순환을 만들지 않는다
  - 빌드 실패 상태로 끝내지 않는다
acc:
  - 검증 기준은 다음과 같다
  - CarFight_ReEditor / Win64 / Development 빌드가 성공해야 한다
  - "수정 파일은 기본적으로 CFVehicleAimComp.h/.cpp, CFVehiclePawn.cpp여야 한다"
  - CFVehicleAimTypes.h의 기존 필드명을 변경하지 않아야 한다
  - Single Player PIE에서 크래시 없이 시작되어야 한다
  - Listen Server + 1 Client에서 Fire Action이 연결된 경우 서버 FireResult 이후 RepAimVisualState가 갱신될 수 있어야 한다
  - VehicleDebug Aim에서 RepAimVisual 값을 확인할 수 있어야 한다
  - Damage / Projectile / Multicast FX / Reticle UI 코드가 없음을 확인한다
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
  source: Document/ProjectSSOT/Plan/AimPlan/Generated/CF_AimCodexSource_R8.md
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
