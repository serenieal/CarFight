v: 1
k: generic_plan_compile
g: 현재 이미 구현되어 있는 차량 중심 기능을 Dedicated Server 환경에서 테스트할 수 있도록 최소 코드 기반을 준비한다
tf:
  - UE/Source/CarFight_Re.Target.cs
  - UE/Source/CarFight_ReEditor.Target.cs
  - UE/Source/CarFight_ReServer.Target.cs
  - UE/Source/CarFight_Re/CarFight_Re.Build.cs
  - UE/Source/CarFight_Re/Public/CFVehiclePawn.h
  - UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
  - UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
  - UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
scope:
  in:
    - Dedicated Server Target 파일 생성 또는 구성
    - 현재 차량 Pawn의 AutoPossessPlayer Player0 구조 정리
    - Dedicated Server에서 안전하지 않은 로컬 전용 초기화 보호
    - "현재 구현된 차량 입력, 카메라, UI, 디버그 흐름의 소유자 조건 확인 및 필요한 최소 방어 코드 추가"
    - 변경 사항에 대한 짧고 명확한 코드 주석 추가
    - 변경 후 빌드 가능성 확인
  out:
    - 무기 시스템 구현
    - 투사체 구현
    - 탄약 또는 장전 구현
    - 체력 시스템 구현
    - 대미지 시스템 구현
    - 사망 처리 구현
    - 스코어 구현
    - 킬 또는 데스 구현
    - 리스폰 시스템 구현
    - 로비 구현
    - Steam 세션 구현
    - 매치메이킹 구현
    - 서버 브라우저 구현
    - 대규모 네트워크 최적화
    - uasset 직접 수정
    - TestMap.umap 직접 수정
    - BP_CFVehiclePawn Blueprint 그래프 직접 수정
    - 신규 GameMode 클래스 생성
    - 신규 PlayerController 클래스 생성
    - 차량 이동 네트워크 예측 구조 신규 설계
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - 파일명은 32자를 넘기지 않는다
  - Blueprint 또는 Map 수정이 필요하면 직접 수정하지 말고 후속 작업으로 보고한다
  - "무기, 체력, 스코어, 리스폰 계열 코드는 추가하지 않는다"
  - 현재 구현 기능 안정화와 직접 관련 없는 리팩터링은 하지 않는다
acc:
  - Unreal Build Tool이 CarFight_ReServer Target을 인식하는지 확인한다
  - Development Editor 빌드가 가능한지 확인한다
  - Development Server 빌드가 가능한지 확인한다
  - 가능하면 Dedicated Server 실행으로 TestMap 로드까지 확인한다
  - "빌드 실행이 불가능하면 실행하지 못한 명령, 이유, 사용자가 직접 실행해야 할 검증 명령을 남긴다"
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
  source: Document/ProjectSSOT/Plan/MP_ServerPlan/Generated/DSCodeSource.md
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
