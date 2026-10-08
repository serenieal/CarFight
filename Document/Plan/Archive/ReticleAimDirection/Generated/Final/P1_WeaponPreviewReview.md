v: 1
artifact_role: final_codex_input
codex_input: true
k: generic_plan_compile
g: 기존 Browser Direct Edit 정책 위반분을 Codex가 검토·보정한다
tf:
  - UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
  - UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
  - UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
scope:
  in:
    - FCFVehicleWeaponAimSolution의 기존 Preview 필드 검토·보정
    - BuildWeaponAimSolution()의 DirectImpact 지원 조건과 Preview 결과 계산
    - MuzzleBlocked와 Preview가 공유하는 단일 WeaponHit Trace
    - 활성 무기 MaxRange와 AimProfile fallback 거리 처리
    - Preview 네 필드의 OutWeaponAimSolution 할당
    - "VehicleDebug의 Preview 유효 여부, Hit 여부, 월드 위치와 거리 표시"
    - "세 허용 파일의 Version, Date, Changelog 정합성 보정"
    - "scoped diff, Tools\\\\BuildEditor.bat와 정적 계약 검증"
  out:
    - World To Screen 투영
    - CFAimReticleWidget 변경
    - WBP_AimReticle 변경
    - Weapon Reticle 이미지 표시와 시각 디자인
    - Ballistic Solver와 중력 Projectile 예상 탄착점
    - "Lead Indicator, 화면 경계 Clamp, 다중 터렛, 네트워크 보정"
    - 기존 WeaponFire 판정 정책 변경
    - "commit, push 또는 작업 트리 정리"
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - 정책 위반 이력을 숨기거나 기존 직접 수정이 Codex 실행 결과였다고 기록하지 않는다
  - 중력 Projectile은 DirectImpact Preview를 제공하지 않는다
  - unrelated dirty 변경을 되돌리거나 재포맷하지 않는다
  - 신규 소스 파일과 UE 에셋을 만들거나 수정하지 않는다
  - 사용자 PIE는 PASS로 선언하지 않는다
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
  - Phase 1 World Preview 데이터와 Debug 표시만 완성한다
  - 기존 직접 수정분을 검토하고 실제 최종 구현과 일치하도록 보정한다
  - Tools\BuildEditor.bat가 성공한다
  - 기존 Aim/Fire/Reticle 계약을 보존한다
  - 정책 위반 이력을 숨기지 않는다
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
  source: Document/Plan/ReticleAimDirection/TaskSource_Phase1.md
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
