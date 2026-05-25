# Dedicated Server Stabilization Codex Source

- 문서 버전: 0.2.0
- 작성일: 2026-05-22
- 문서 상태: Codex Source
- 대상 프로젝트: CarFight

## Goal

현재 이미 구현되어 있는 차량 중심 기능을 Dedicated Server 환경에서 테스트할 수 있도록 최소 코드 기반을 준비한다.

이번 작업은 신규 게임플레이 시스템 구현이 아니다.

목표는 다음 세 가지다.

1. CarFight_Re Dedicated Server Target을 준비한다.
2. 현재 차량 Pawn의 싱글플레이 자동 빙의 구조가 Dedicated Server 멀티 테스트를 방해하지 않도록 정리한다.
3. Dedicated Server에서 로컬 UI, 카메라, 디버그 흐름이 실행되어도 오류가 나지 않도록 현재 구현 기능만 안전하게 보호한다.

## Current Decisions

1. 현재 작업 범위는 이미 구현된 기능 안정화로 제한한다.
2. 무기, 체력, 대미지, 스코어, 리스폰, 로비, 세션은 구현하지 않는다.
3. BP_CFVehiclePawn의 bReplicates와 bReplicateMovement는 true로 확인됐다.
4. BP_CFVehiclePawn과 ACFVehiclePawn의 AutoPossessPlayer는 Player0로 확인됐다.
5. BP_CFVehiclePawn EventGraph의 Debug UI 생성은 Is Locally Controlled 조건 뒤에서 실행된다.
6. TestMap은 PlayerStart 1개와 BP_CFVehiclePawn 1대만 가지고 있다.
7. CarFight_ReServer.Target.cs는 현재 없는 것으로 확인됐다.

## In Scope

1. Dedicated Server Target 파일 생성 또는 구성.
2. 현재 차량 Pawn의 AutoPossessPlayer Player0 구조 정리.
3. Dedicated Server에서 안전하지 않은 로컬 전용 초기화 보호.
4. 현재 구현된 차량 입력, 카메라, UI, 디버그 흐름의 소유자 조건 확인 및 필요한 최소 방어 코드 추가.
5. 변경 사항에 대한 짧고 명확한 코드 주석 추가.
6. 변경 후 빌드 가능성 확인.

## Out of Scope

1. 무기 시스템 구현.
2. 투사체 구현.
3. 탄약 또는 장전 구현.
4. 체력 시스템 구현.
5. 대미지 시스템 구현.
6. 사망 처리 구현.
7. 스코어 구현.
8. 킬 또는 데스 구현.
9. 리스폰 시스템 구현.
10. 로비 구현.
11. Steam 세션 구현.
12. 매치메이킹 구현.
13. 서버 브라우저 구현.
14. 대규모 네트워크 최적화.
15. uasset 직접 수정.
16. TestMap.umap 직접 수정.
17. BP_CFVehiclePawn Blueprint 그래프 직접 수정.
18. 신규 GameMode 클래스 생성.
19. 신규 PlayerController 클래스 생성.
20. 차량 이동 네트워크 예측 구조 신규 설계.

## Constraints

1. 파일명은 32자를 넘기지 않는다.
2. 기존 C++ 스타일과 Unreal TargetRules 스타일을 최대한 따른다.
3. Blueprint 또는 Map 수정이 필요하면 직접 수정하지 말고 후속 작업으로 보고한다.
4. GameMode 또는 PlayerController 신규 클래스가 필요하다고 판단되면 작업을 중단하고 보고한다.
5. 무기, 체력, 스코어, 리스폰 계열 코드는 추가하지 않는다.
6. 현재 구현 기능 안정화와 직접 관련 없는 리팩터링은 하지 않는다.
7. public Blueprint 변수나 함수가 추가될 경우 주석과 Tooltip을 명확히 작성한다.
8. 빌드 실행이 불가능하면 실행하지 못한 명령과 이유를 남긴다.

## Target Files

1. UE/Source/CarFight_Re.Target.cs
2. UE/Source/CarFight_ReEditor.Target.cs
3. UE/Source/CarFight_ReServer.Target.cs
4. UE/Source/CarFight_Re/CarFight_Re.Build.cs
5. UE/Source/CarFight_Re/Public/CFVehiclePawn.h
6. UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
7. UE/Source/CarFight_Re/Public/CFVehicleAimComp.h
8. UE/Source/CarFight_Re/Private/CFVehicleAimComp.cpp

허용 가능한 신규 파일은 UE/Source/CarFight_ReServer.Target.cs 하나다.

## Task Candidates

### Task 1. Dedicated Server Target 확인 및 생성

확인할 것:

- UE/Source/CarFight_ReServer.Target.cs 존재 여부.
- 기존 Target 파일의 스타일.
- 프로젝트 모듈명 CarFight_Re.

구현 방향:

- 파일이 없으면 UE/Source/CarFight_ReServer.Target.cs를 생성한다.
- TargetType은 Server로 둔다.
- ExtraModuleNames에는 CarFight_Re를 추가한다.
- Unreal Engine 5.7과 기존 TargetRules 스타일에 맞춘다.

### Task 2. ACFVehiclePawn AutoPossess 구조 정리

현재 문제:

- ACFVehiclePawn 생성자와 BP_CFVehiclePawn Class Defaults에서 AutoPossessPlayer = Player0가 확인됐다.

구현 방향:

- C++ 생성자에서 AutoPossessPlayer를 Player0로 강제하지 않도록 정리한다.
- Dedicated Server에서 자동 빙의에 의존하지 않도록 한다.
- 싱글플레이 또는 에디터 테스트 편의가 필요하면 명시적 옵션으로 분리하는 방안을 고려한다.

중요 제한:

- BP Class Defaults 값은 이번 Codex 작업에서 직접 수정하지 않는다.
- BP 값 때문에 문제가 계속 남으면 후속 에디터 설정 작업으로 보고한다.

### Task 3. Dedicated Server 로컬 전용 흐름 보호

검토 대상:

- BeginPlay.
- RegisterDefaultInputMappingContext.
- CreateAimReticleWidget.
- DestroyAimReticleWidget.
- Debug UI 관련 흐름.
- VehicleCameraComp 사용 흐름.

구현 방향:

- Dedicated Server에서는 Viewport, LocalPlayer, Camera, UI 생성에 의존하지 않게 방어한다.
- 이미 GetNetMode() != NM_DedicatedServer와 IsLocallyControlled() 조건이 있는 흐름은 유지한다.
- 부족한 방어 조건이 있으면 최소한으로 추가한다.

### Task 4. 입력과 이동 흐름은 구조 변경 없이 안전성만 점검

현재 상태:

- 차량 입력은 ACFVehiclePawn에서 DriveComp를 통해 Chaos Vehicle Movement에 직접 적용된다.
- BP_CFVehiclePawn의 bReplicates와 bReplicateMovement는 true다.

구현 방향:

- 이번 작업에서는 차량 이동 네트워크 구조를 새로 설계하지 않는다.
- 입력이 비소유 Pawn에서 처리될 위험이 있으면 소유자 조건을 보강한다.
- 대규모 예측 또는 서버 권한 주행 시스템은 만들지 않는다.

## Acceptance Criteria

1. UE/Source/CarFight_ReServer.Target.cs가 존재하거나, 존재하지 않아도 되는 명확한 이유가 기록되어 있다.
2. C++에서 AutoPossessPlayer = Player0 강제 설정이 제거되거나 Dedicated Server에서 무시되도록 안전하게 처리되어 있다.
3. Dedicated Server에서 UI, Viewport, LocalPlayer, Camera 흐름이 오류를 만들 가능성이 줄어들었다.
4. 현재 구현된 차량 기능 외의 신규 게임플레이 시스템이 추가되지 않았다.
5. 무기, 체력, 대미지, 스코어, 리스폰 관련 코드가 새로 추가되지 않았다.
6. 변경 파일은 Target Files 범위를 벗어나지 않는다. 벗어나야 하면 이유를 보고한다.
7. 변경 내용에 초보자가 이해할 수 있는 주석이 필요한 곳에는 짧고 명확한 주석을 추가한다.

## Verification

1. Unreal Build Tool이 CarFight_ReServer Target을 인식하는지 확인한다.
2. Development Editor 빌드가 가능한지 확인한다.
3. Development Server 빌드가 가능한지 확인한다.
4. 가능하면 Dedicated Server 실행으로 TestMap 로드까지 확인한다.
5. 빌드 실행이 불가능하면 실행하지 못한 명령, 이유, 사용자가 직접 실행해야 할 검증 명령을 남긴다.

## Reporting Requirements

Codex는 작업 후 아래 내용을 요약한다.

1. 변경한 파일 목록.
2. 각 파일을 변경한 이유.
3. 추가한 방어 조건.
4. Dedicated Server Target 처리 결과.
5. 빌드 또는 검증 결과.
6. 남은 수동 작업.
7. 후속으로 필요한 Blueprint 또는 맵 설정 작업.

## Stop Conditions

아래 상황에서는 임의로 구현을 계속하지 말고 중단 후 보고한다.

1. GameMode 신규 클래스가 필요하다고 판단되는 경우.
2. PlayerController 신규 클래스가 필요하다고 판단되는 경우.
3. TestMap.umap 수정이 필요하다고 판단되는 경우.
4. BP_CFVehiclePawn Blueprint Class Defaults 수정이 필요하다고 판단되는 경우.
5. 무기, 체력, 스코어, 리스폰 시스템 변경이 필요하다고 판단되는 경우.
6. 차량 이동 네트워크 예측 구조를 새로 설계해야 한다고 판단되는 경우.

## Unresolved

1. BP_CFVehiclePawn Class Defaults의 AutoPossessPlayer = Player0 값은 에디터 설정 수정이 필요할 수 있다.
2. TestMap에는 PlayerStart 1개와 차량 1대만 있으므로 2클라 테스트에는 맵 또는 Spawn/Possess 구조 변경이 필요할 수 있다.
3. GameMode 또는 PlayerController 전용 클래스가 아직 확인되지 않았다.
4. 실제 Dedicated Server 실행 테스트는 아직 완료되지 않았다.

## Source References

1. Document/ProjectSSOT/Plan/MP_ServerPlan/Scope.md
2. Document/ProjectSSOT/Plan/MP_ServerPlan/FeatureList.md
3. Document/ProjectSSOT/Plan/MP_ServerPlan/NetAudit.md
4. Document/ProjectSSOT/Plan/MP_ServerPlan/RiskSearch.md
5. Document/ProjectSSOT/Plan/MP_ServerPlan/DSMinRun.md
