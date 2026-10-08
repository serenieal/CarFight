v: 1
k: generic_plan_compile
g: CarFight의 현재 구현된 BP_CFVehiclePawn을 사용해서 Dedicated Server와 PIE 멀티플레이에서 플레이어가 차량을 소유할 수 있는 최소 Spawn/Possess 구조를 만든다
tf:
  - UE/Source/CarFight_Re/CarFight_Re.Build.cs
  - UE/Source/CarFight_Re/Public/CFVehiclePawn.h
  - UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
  - UE/Config/DefaultEngine.ini
  - UE/Config/DefaultGame.ini
  - UE/CarFight_Re.uproject
  - UE/Source/CarFight_Re/Public/CFGameMode.h
  - UE/Source/CarFight_Re/Private/CFGameMode.cpp
scope:
  in:
    - 최소 GameMode 또는 GameModeBase C++ 클래스 생성
    - 접속한 PlayerController마다 차량 Pawn을 서버에서 Spawn
    - Spawn된 차량 Pawn을 해당 PlayerController가 Possess
    - PlayerStart를 기준으로 Spawn 위치를 선택
    - BP_CFVehiclePawn을 Spawn할 Pawn Class로 지정할 수 있는 설정 제공
    - PIE 2인과 Dedicated Server 클라이언트 접속 테스트가 가능하도록 기본 흐름 구성
    - 필요한 경우 DefaultEngine.ini 또는 프로젝트 설정에서 GameMode를 지정하는 최소 설정 반영
    - 초보자가 이해할 수 있도록 주요 변수와 함수에 명확한 주석과 Tooltip 작성
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
    - 차량 선택 시스템 구현
    - 팀 시스템 구현
    - 장기적인 Spawn Manager 또는 Match Flow 시스템 설계
    - TestMap.umap 직접 수정
    - uasset 직접 수정
    - BP_CFVehiclePawn Blueprint 그래프 직접 수정
    - 차량 이동 네트워크 예측 구조 신규 설계
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - 현재 구현 기능 안정화와 직접 관련 없는 리팩터링은 하지 않는다
  - 파일명은 32자를 넘기지 않는다
  - TestMap을 직접 수정하지 않는다. PlayerStart가 1개뿐인 문제는 로그 또는 문서로 보고한다
  - BP_CFVehiclePawn을 직접 수정하지 않는다
acc:
  - CarFight_ReEditor 빌드
  - CarFight_ReServer 빌드
  - "PIE Number of Players 2, Play As Listen Server 테스트"
  - PIE Play As Client 테스트
  - Staged Dedicated Server 실행
  - "Client 1에서 open 127.0.0.1:7777"
  - "Client 2에서 open 127.0.0.1:7777"
  - 각 클라이언트가 자기 차량을 소유하는지 확인
  - 입력이 자기 차량에만 적용되는지 확인
  - 서로의 차량 이동이 보이는지 확인
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
  source: Document/ProjectSSOT/Plan/MP_ServerPlan/Generated/PossessTaskSource.md
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
