v: 1
artifact_role: final_codex_input
codex_input: true
k: generic_plan_compile
g: "text 현재 Command Reticle을 유지하면서, FCFVehicleWeaponAimSolution.AimDirection이 나타내는 실제 최종 사격 방향을 별도의 Weapon Reticle로 정확히 표시한다"
tf:
  - UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
  - UE/Source/CarFight_Re/Public/CFVehiclePawn.h
  - UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
  - UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h
  - UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
  - UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
scope:
  in:
    - 직선 HitScan과 중력 없는 Projectile의 Weapon Preview World 위치 계산
    - Command Reticle과 Weapon Reticle의 이중 Reticle 표시
    - 실제 최종 발사 방향과 동일한 WeaponHit Preview 결과 공유
    - C++ World To Screen 투영과 Optional WBP 바인딩
    - "정렬 중 발사 허용/금지, MuzzleBlocked, 이동 상태 회귀 검증"
    - Ballistic과 Lead Indicator로 확장 가능한 데이터 계약 정의
  out:
    - 중력 Projectile Ballistic Solver
    - 이동 목표 Lead Indicator
    - 자동 락온과 Aim Assist
    - 다중 터렛 동시 Reticle
    - 네트워크 지연 보정
    - 기존 WeaponFire 판정 정책 변경
    - 완성형 전투 HUD 전체 재설계
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - "WBP는 Trace, 방향 계산과 발사 가능 판정을 수행하지 않는다"
  - 기존 WBP 바인딩과 FireFeedback 상태를 깨지 않는다
  - 신규 또는 수정 파일명은 32자를 넘지 않는다
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
    - v2
    - v3
    - v4
acc:
  - text
  - Tools\BuildEditor.bat PASS
  - 정책 true 정렬 중 발사에서 Weapon Reticle과 실제 탄착 일치
  - 정책 false 정렬 중 거부와 정렬 완료 후 일치
  - MuzzleBlocked 발사 차단과 Preview 위치 일치
  - HitScan과 중력 없는 Projectile 첫 충돌 일치
verify:
  - text
  - Tools\BuildEditor.bat PASS
  - 정책 true 정렬 중 발사에서 Weapon Reticle과 실제 탄착 일치
  - 정책 false 정렬 중 거부와 정렬 완료 후 일치
fail:
  - acceptance_criteria를 충족하지 못하면 성공이 아니다
  - must_change_targets 검토 없이 수정 파일 0개로 종료하면 재검토가 필요하다
  - 대상 파일 변경 필요성을 확인하지 않고 이미 구현됨으로 종료하면 성공이 아니다
  - failure_reproduction 입력이 남아 있으면 원인 확인 없이 성공 처리하면 안 된다
  - target_files 검토 없이 수정 파일 0개로 종료하면 성공이 아니다
ask_if_missing:
  - "목표, 범위, 검증 조건 중 하나라도 불명확하면 구현하지 말고 missing contract를 보고한다"
ref:
  source: Document/Plan/ReticleAimDirection/ImplementationDesign.md
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
