v: 1
k: generic_plan_compile
g: CarFight 프로젝트에 Aim 시스템의 1차 C++ 기반을 추가한다
tf:
  - UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
  - UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
  - UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp
  - UE/Source/CarFight_Re/Public/CFVehiclePawn.h
  - UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
  - UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h
  - UE/Source/CarFight_Re/Public/CFVehicleCameraTypes.h
  - UE/Source/CarFight_Re/Public/CFVehicleCameraData.h
scope:
  in:
    - 이번 작업에 포함한다
    - CFVehicleAimTypes.h 생성
    - UCFVehicleAimComp 헤더 / cpp 생성
    - ACFVehiclePawn에 AimComp 멤버 추가
    - ACFVehiclePawn 생성자에서 AimComp 생성
    - GetVehicleAimComp() getter 추가
    - AimComp 초기화 함수 추가
    - AimComp가 Owner Pawn과 CameraComp를 안전하게 찾을 수 있는 기본 구조 추가
    - C++ 컴파일 가능한 상태 유지
    - CFVehicleAimTypes.h에는 다음 타입을 정의한다
    - ECFVehicleReticleState
    - ECFVehicleFireRejectReason
    - FCFVehicleAimProfile
    - FCFVehicleLocalAimState
    - FCFVehicleServerAimState
    - FCFVehicleRepAimVisualState
    - FCFVehicleFireRequest
    - FCFVehicleFireResult
    - UCFVehicleAimComp에는 다음 기본 멤버를 둔다
    - OwnerVehiclePawn
    - CachedVehicleCameraComp
    - DefaultAimProfile
    - LocalAimState
    - ServerAimState
    - RepAimVisualState
    - bAimRuntimeReady
    - LastAimRuntimeSummary
    - UCFVehicleAimComp에는 다음 기본 함수를 둔다
    - InitializeAimRuntime()
    - RefreshAimRuntimeReferences()
    - GetLocalAimState()
    - GetServerAimState()
    - GetRepAimVisualState()
    - GetDefaultAimProfile()
    - IsAimRuntimeReady()
    - GetLastAimRuntimeSummary()
    - ResolveOwnerVehiclePawn()
    - ResolveVehicleCameraComp()
  out:
    - 이번 작업에서 하지 않는다
    - ServerRequestFire() 구현 금지
    - InputAction_Fire 생성 금지
    - Reticle UI 생성 금지
    - VehicleDebug Aim 카테고리 생성 금지
    - Local Aim 계산 로직 완성 금지
    - Fire RPC 구현 금지
    - 서버 발사 검증 구현 금지
    - HitScan 구현 금지
    - Damage 구현 금지
    - Projectile 구현 금지
    - UCFVehicleWeaponComp 생성 금지
    - BP 에셋 수정 금지
    - PoliceCar 전용 하드코딩 금지
    - CameraComp 안에 무기 발사 판정 추가 금지
ban:
  - do_not_expand_scope
  - do_not_create_unrequested_files
  - do_not_change_unlisted_public_contracts
  - do_not_treat_review_targets_as_change_targets
  - do_not_mark_success_with_missing_acceptance
  - 파일명은 32자를 넘기지 않는다
  - UCFVehicleAimComp는 이번 작업에서 Tick을 사용하지 않는다. PrimaryComponentTick.bCanEverTick = false를 기본으로 한다
  - 기존 UCFVehicleCameraComp 책임을 AimComp로 옮기지 않는다
  - 기존 VehicleDebug / CameraDebug 코드를 이번 작업에서 변경하지 않는다
  - "기존 주행, 조향, 카메라 입력 동작을 변경하지 않는다"
  - include 순환을 만들지 않는다. 가능한 forward declaration을 사용한다
  - 빌드 실패 상태로 끝내지 않는다
acc:
  - 검증 기준은 다음과 같다
  - CarFight_ReEditor / Win64 / Development 빌드가 성공해야 한다
  - 신규 파일 3개가 올바른 경로에 생성되어야 한다
  - CFVehiclePawn.h와 CFVehiclePawn.cpp만 기존 파일 중 수정되어야 한다
  - BP_CFVehiclePawn에서 AimComp가 컴포넌트로 보일 수 있어야 한다
  - BeginPlay 또는 기존 런타임 초기화 흐름에서 AimComp 초기화가 호출되어야 한다
  - CameraComp가 없는 경우에도 에디터 또는 PIE가 크래시하지 않아야 한다
  - "기존 Look 입력, CameraComp, VehicleDriveComp, WheelSyncComp 동작에 회귀가 없어야 한다"
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
  source: Document/ProjectSSOT/Plan/AimPlan/Generated/CF_AimCodexSource_R1R2.md
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
