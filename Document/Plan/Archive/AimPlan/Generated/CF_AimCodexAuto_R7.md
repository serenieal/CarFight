v: 1
k: generic_plan_compile
g: CarFight 프로젝트의 Aim 기반 서버 발사 검증 흐름에 Server HitScan 더미 Trace를 추가한다
tf:
  - UE/Source/CarFight_Re/Public/CFVehiclePawn.h
  - UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
  - UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
  - UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
  - UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
scope:
  in:
    - 이번 작업에 포함한다
    - 서버에서만 실행되는 HitScan 더미 Trace helper를 추가한다
    - ValidateFireRequestOnServer()가 발사 요청 승인 조건을 통과한 뒤 서버 Trace를 수행하게 한다
    - Trace 시작점은 FireRequest.AimOrigin을 우선 사용한다
    - Trace 방향은 FireRequest.AimDirection.GetSafeNormal()을 사용한다
    - "Trace 거리는 VehicleAimComp->GetDefaultAimProfile().MaxAimDistance를 사용한다"
    - "Trace 끝점은 AimOrigin + AimDirection * MaxAimDistance로 계산한다"
    - Trace는 서버 World에서 LineTraceSingleByChannel()을 사용한다
    - Collision Channel은 1차 더미이므로 ECC_Visibility를 사용한다
    - Trace Query Params에는 Owner Pawn을 ignored actor로 추가한다
    - Trace가 Hit되면 FCFVehicleFireResult.ServerHitLocation과 ServerHitNormal에 Hit 결과를 넣는다
    - "Trace가 Miss되면 ServerHitLocation은 Trace End로 넣고 ServerHitNormal은 FVector::UpVector 또는 -AimDirection 후보 중 하나로 일관되게 설정한다"
    - "Trace 결과와 무관하게, 이번 단계에서는 유효 발사 요청이면 Damage 없이 승인 상태를 유지한다"
    - 선택적으로 서버 Debug Draw를 켜고 끌 수 있는 C++ bool 설정을 추가한다
    - VehicleDebug Aim에서 ServerAimTargetLocation 또는 ServerHitLocation에 가까운 값을 확인할 수 있도록 기존 FireResult 반영 경로를 유지한다
  out:
    - 이번 작업에서 하지 않는다
    - Damage 적용 금지
    - Projectile 구현 금지
    - Multicast Fire FX 구현 금지
    - RepAimVisual 복제 구현 금지
    - Reticle UI 생성 금지
    - UCFVehicleWeaponComp 생성 금지
    - Ammo / Cooldown / Reload 구현 금지
    - Hit Actor를 기반으로 게임플레이 결과 처리 금지
    - BP 에셋 수정 금지
    - Input Mapping Context 에셋 수정 금지
    - WBP Designer 수정 금지
    - Lock-On 구현 금지
    - AI Combat 구현 금지
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - 파일명은 32자를 넘기지 않는다
  - 요청이 거부된 경우 서버 Trace를 실행하지 않는다
  - 클라이언트가 보낸 Hit 결과를 신뢰하지 않는다. Hit 결과는 서버 Trace로만 만든다
  - "Damage, Projectile, Multicast FX, WeaponComp를 구현하지 않는다"
  - Trace 실패를 곧바로 Fire Reject로 처리하지 않는다. 이번 단계에서는 더미 HitScan 시각/디버그 결과만 만든다
  - 기존 Fire Request / Result / ServerAimState 흐름을 깨지 않는다
  - 기존 주행 / 카메라 / VehicleDebug 기능을 변경하지 않는다
  - 빌드 실패 상태로 끝내지 않는다
acc:
  - 검증 기준은 다음과 같다
  - CarFight_ReEditor / Win64 / Development 빌드가 성공해야 한다
  - 수정 파일은 기본적으로 CFVehiclePawn.h/.cpp여야 한다
  - CFVehicleAimTypes.h의 기존 필드명을 변경하지 않아야 한다
  - Single Player PIE에서 크래시 없이 시작되어야 한다
  - Listen Server + 1 Client에서 Fire Action이 연결된 경우 서버 Trace가 실행될 수 있어야 한다
  - Trace 결과가 FCFVehicleFireResult와 FCFVehicleServerAimState에 반영되어야 한다
  - VehicleDebug Aim에서 Server Aim 위치와 Last Accepted/Rejected ID가 유지되어야 한다
  - Damage / Projectile / Multicast FX 코드가 없음을 확인한다
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
  source: Document/ProjectSSOT/Plan/AimPlan/Generated/CF_AimCodexSource_R7.md
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
