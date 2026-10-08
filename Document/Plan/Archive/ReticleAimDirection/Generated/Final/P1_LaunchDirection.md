v: 1
artifact_role: final_codex_input
codex_input: true
k: generic_plan_compile
g: 현재 실사용 Heavy Cannon처럼 중력이 적용되는 Projectile에서도 실제 초기 발사 방향을 Weapon Reticle 데이터로 제공한다
tf:
  - UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
  - UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
  - UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
scope:
  in:
    - ECFWeaponReticleMode 신규 enum 추가
    - FCFVehicleWeaponAimSolution에 현재 Reticle Mode 필드 추가
    - 기존 Preview 필드의 의미를 DirectImpact와 LaunchDirection 공용 표시 데이터로 확장
    - BuildWeaponAimSolution()의 모드 판정과 LaunchDirection 월드 위치 계산
    - 기존 DirectImpact Preview와 공유 WeaponHit Trace 회귀 보호
    - VehicleDebug에 Weapon Reticle Mode 표시 추가
    - FireOrigin 요약 문자열에 Mode 표시 추가
    - "허용 파일의 Version, Date, Changelog 정합성 보정"
    - "scoped diff, Tools\\\\BuildEditor.bat와 정적 계약 검증"
  out:
    - 중력 Projectile 예상 탄착 위치 계산
    - "Ballistic Solver, 탄도 궤적 Trace와 BallisticImpact Reticle"
    - World To Screen 투영
    - CFAimReticleWidget 변경
    - WBP_AimReticle 변경
    - "Weapon Reticle 이미지, 색상, Opacity와 애니메이션"
    - Lead Indicator와 이동 목표 예측
    - 화면 가장자리 Clamp
    - "실제 Projectile 중력, 속도 또는 DataAsset 값 변경"
    - 기존 WeaponFire 판정 정책 변경
    - "commit, push 또는 작업 트리 정리"
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - DA_HeavyShell.bAffectedByGravity를 변경하지 않는다
  - "AimTargetLocation, AimDirection, FireRequest.AimDirection과 정렬 중 발사 정책을 변경하지 않는다"
  - unrelated dirty 변경을 되돌리거나 재포맷하지 않는다
  - 사용자 PIE를 PASS로 선언하지 않는다
sc:
  acc:
    - a1
    - a2
    - a3
    - a4
    - a5
    - a6
  verify:
    - v1
acc:
  - "세 허용 파일 외 소스, 설정과 에셋을 수정하지 않는다"
  - Heavy Cannon에서 사용할 LaunchDirection 데이터 계약을 제공한다
  - DirectImpact와 LaunchDirection의 의미가 enum과 Debug에서 구분된다
  - 중력 Projectile 탄착점을 예측한 것처럼 표시하지 않는다
  - 기존 Aim/Fire/MuzzleBlocked 계약을 보존한다
  - Tools\\BuildEditor.bat가 성공한다
verify:
  - 작업 전후 세 허용 파일의 scoped Git diff 확인
fail:
  - acceptance_criteria를 충족하지 못하면 성공이 아니다
  - must_change_targets 검토 없이 수정 파일 0개로 종료하면 재검토가 필요하다
  - 대상 파일 변경 필요성을 확인하지 않고 이미 구현됨으로 종료하면 성공이 아니다
  - failure_reproduction 입력이 남아 있으면 원인 확인 없이 성공 처리하면 안 된다
  - target_files 검토 없이 수정 파일 0개로 종료하면 성공이 아니다
ask_if_missing:
  - "목표, 범위, 검증 조건 중 하나라도 불명확하면 구현하지 말고 missing contract를 보고한다"
ref:
  source: Document/Plan/ReticleAimDirection/TaskSource_LaunchDirection.md
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
    guidance_codes:
      - codex_task_slots_required
      - check_mcp_ssot_before_generation
      - prefer_plan_build_from_folder_request
      - use_plan_build_from_folder_request
      - no_manual_longform_fallback
    warning_codes:
      - project_document_path_ignored_for_mcp_ssot_preflight
