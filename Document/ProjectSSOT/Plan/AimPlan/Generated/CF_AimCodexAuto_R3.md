v: 1
k: generic_plan_compile
g: CarFight 프로젝트의 UCFVehicleAimComp에 Local Aim 계산을 추가한다
tf:
  - UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
  - UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
  - UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
  - UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h
  - UE/Source/CarFight_Re/Public/CFVehicleCameraTypes.h
  - UE/Source/CarFight_Re/Public/CFVehiclePawn.h
scope:
  in:
    - 이번 작업에 포함한다
    - UCFVehicleAimComp에서 Tick을 활성화한다
    - Tick에서 Local Aim State를 갱신한다
    - RefreshLocalAimState(float DeltaSeconds) 함수를 추가한다
    - Camera Runtime State를 읽어 LocalAimState.LocalAimTargetLocation을 갱신한다
    - Owner Pawn 위치 또는 Camera Aim Target을 기준으로 LocalAimState.LocalAimDirection을 계산한다
    - 차량 기준 Aim Yaw / Pitch를 계산하는 helper 함수를 추가한다
    - "DefaultAimProfile의 MinYawDeg, MaxYawDeg, MinPitchDeg, MaxPitchDeg를 이용해 bLocalWithinWeaponArc를 계산한다"
    - Camera Runtime State의 bAimBlocked를 bLocalAimBlocked에 반영한다
    - "bLocalCanFire를 bLocalWithinWeaponArc && !bLocalAimBlocked 기준으로 계산한다"
    - LocalReticleState를 다음 규칙으로 계산한다
    - "Aim 런타임 준비 안 됨: Hidden"
    - "Aim blocked: Blocked"
    - "Weapon Arc 밖: OutOfArc"
    - "발사 가능: Ready"
    - "그 외: Hidden"
    - 계산 결과를 LocalAimState에 저장한다
    - 기존 getter GetLocalAimState()와 GetReticleState() 또는 신규 getter로 상태를 읽을 수 있게 한다
  out:
    - 이번 작업에서 하지 않는다
    - ServerRequestFire() 구현 금지
    - InputAction_Fire 생성 금지
    - Fire Request 생성 금지
    - Server Aim State 계산 금지
    - Server Fire 검증 금지
    - HitScan 구현 금지
    - Damage 구현 금지
    - Projectile 구현 금지
    - UCFVehicleWeaponComp 생성 금지
    - VehicleDebug Aim 카테고리 생성 금지
    - Reticle UI 생성 금지
    - BP 에셋 수정 금지
    - Replication 설정 금지
    - PoliceCar 전용 하드코딩 금지
    - CameraComp 내부 로직 수정 금지
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - 파일명은 32자를 넘기지 않는다
  - 기존 CFVehicleAimTypes.h의 struct 필드명을 바꾸지 않는다
  - 기존 ACFVehiclePawn 생성자와 런타임 초기화 흐름을 변경하지 않는다
  - 기존 UCFVehicleCameraComp 책임을 AimComp로 옮기지 않는다
  - UCFVehicleCameraComp 내부 코드를 수정하지 않는다
  - "Tick에서 서버 RPC, DebugPanel, UI, Damage, HitScan을 호출하지 않는다"
  - CameraComp가 없거나 Aim Runtime이 준비되지 않은 경우 크래시하지 않는다
  - DefaultAimProfile 기본값은 이번 작업에서 임의 변경하지 않는다
  - 빌드 실패 상태로 끝내지 않는다
acc:
  - 검증 기준은 다음과 같다
  - CarFight_ReEditor / Win64 / Development 빌드가 성공해야 한다
  - 수정 파일은 원칙적으로 CFVehicleAimComp.h와 CFVehicleAimComp.cpp여야 한다
  - CFVehiclePawn.h/.cpp가 이번 작업에서 변경되지 않아야 한다
  - CFVehicleCameraComp.h/.cpp가 이번 작업에서 변경되지 않아야 한다
  - Single Player PIE에서 크래시 없이 시작되어야 한다
  - CameraComp가 준비된 경우 LocalAimState가 Tick에서 갱신되어야 한다
  - AimTarget이 Zero에 고정되지 않고 CameraRuntimeState의 AimHitLocation을 따라갈 수 있어야 한다
  - 조준각 밖에서는 OutOfArc가 나올 수 있어야 한다
  - AimBlocked 상태에서는 Blocked가 우선되어야 한다
  - "서버 RPC, DebugPanel, Reticle UI, Damage 관련 코드가 추가되지 않았는지 확인한다"
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
  source: Document/ProjectSSOT/Plan/AimPlan/Generated/CF_AimCodexSource_R3.md
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
