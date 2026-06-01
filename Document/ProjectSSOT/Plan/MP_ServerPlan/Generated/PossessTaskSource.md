# Minimal Spawn Possess Codex Source

- 문서 버전: 0.1.0
- 작성일: 2026-05-26
- 문서 상태: Codex Source
- 대상 프로젝트: CarFight

## Goal

CarFight의 현재 구현된 BP_CFVehiclePawn을 사용해서 Dedicated Server와 PIE 멀티플레이에서 플레이어가 차량을 소유할 수 있는 최소 Spawn/Possess 구조를 만든다.

이번 작업의 목표는 2명 클라이언트가 각각 자기 차량을 소유하고 입력할 수 있는 최소 테스트 기반이다.

이번 작업은 전투 시스템 구현이 아니다.

## Current Findings

1. UE 5.7 Source Build 전환은 완료됐다.
2. Dedicated Server 빌드와 실행은 성공했다.
3. Dedicated Server는 /Game/Maps/TestMap을 로드하고 7777 포트에서 리슨할 수 있다.
4. Play As Listen Server에서는 차량 빙의가 된다.
5. Play As Client에서는 빙의가 되지 않는다.
6. Dedicated Server에 open 명령으로 접속하면 접속은 되지만 화면이 검정색이다.
7. TestMap에는 PlayerStart 1개와 BP_CFVehiclePawn 1대가 있다.
8. Config에는 GlobalDefaultGameMode 또는 GlobalDefaultServerGameMode가 없다.
9. 커스텀 GameMode C++ 클래스는 확인되지 않았다.
10. 커스텀 PlayerController C++ 클래스는 확인되지 않았다.
11. BP_CFVehiclePawn은 bReplicates = true, bReplicateMovement = true로 확인됐다.
12. C++의 ACFVehiclePawn은 AutoPossessPlayer = Disabled 상태다.

## In Scope

1. 최소 GameMode 또는 GameModeBase C++ 클래스 생성.
2. 접속한 PlayerController마다 차량 Pawn을 서버에서 Spawn.
3. Spawn된 차량 Pawn을 해당 PlayerController가 Possess.
4. PlayerStart를 기준으로 Spawn 위치를 선택.
5. BP_CFVehiclePawn을 Spawn할 Pawn Class로 지정할 수 있는 설정 제공.
6. PIE 2인과 Dedicated Server 클라이언트 접속 테스트가 가능하도록 기본 흐름 구성.
7. 필요한 경우 DefaultEngine.ini 또는 프로젝트 설정에서 GameMode를 지정하는 최소 설정 반영.
8. 초보자가 이해할 수 있도록 주요 변수와 함수에 명확한 주석과 Tooltip 작성.

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
14. 차량 선택 시스템 구현.
15. 팀 시스템 구현.
16. 장기적인 Spawn Manager 또는 Match Flow 시스템 설계.
17. TestMap.umap 직접 수정.
18. uasset 직접 수정.
19. BP_CFVehiclePawn Blueprint 그래프 직접 수정.
20. 차량 이동 네트워크 예측 구조 신규 설계.

## Constraints

1. 현재 구현 기능 안정화와 직접 관련 없는 리팩터링은 하지 않는다.
2. 파일명은 32자를 넘기지 않는다.
3. 새로 만드는 클래스명은 직관적으로 한다.
4. Blueprint에서 설정할 변수는 Tooltip을 작성한다.
5. C++ 변수와 함수 위에는 초보자가 이해할 수 있는 주석을 작성한다.
6. Dedicated Server에서만 권한 있는 Spawn/Possess가 실행되도록 한다.
7. 클라이언트가 직접 Pawn을 Spawn하거나 Possess하지 않게 한다.
8. TestMap을 직접 수정하지 않는다. PlayerStart가 1개뿐인 문제는 로그 또는 문서로 보고한다.
9. BP_CFVehiclePawn을 직접 수정하지 않는다.
10. 빌드 실행이 불가능하면 실행하지 못한 명령과 이유를 기록한다.

## Target Files

우선 검토할 파일:

1. UE/Source/CarFight_Re/CarFight_Re.Build.cs
2. UE/Source/CarFight_Re/Public/CFVehiclePawn.h
3. UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
4. UE/Config/DefaultEngine.ini
5. UE/Config/DefaultGame.ini
6. UE/CarFight_Re.uproject

허용 가능한 신규 파일 후보:

1. UE/Source/CarFight_Re/Public/CFGameMode.h
2. UE/Source/CarFight_Re/Private/CFGameMode.cpp

가능하면 PlayerController 신규 클래스는 만들지 않는다.

PlayerController 신규 클래스가 반드시 필요하다고 판단되면 작업을 멈추고 보고한다.

## Required Work

### Task 1. 최소 GameMode 클래스 생성

새 클래스 후보:

- ACFGameMode

상속 후보:

- AGameModeBase 또는 AGameMode

구현 방향:

1. 서버가 플레이어 접속 시 Pawn을 Spawn/Possess할 수 있는 최소 GameMode를 만든다.
2. 복잡한 매치 상태, 스코어, 리스폰은 만들지 않는다.
3. Spawn할 차량 Pawn Class를 Blueprint에서 설정할 수 있게 한다.
4. 기본값은 가능하면 ACFVehiclePawn 또는 BP 설정에 의존하게 하되, BP_CFVehiclePawn 지정이 필요한 경우 설정 방법을 명확히 남긴다.

### Task 2. PlayerStart 기반 Spawn 위치 사용

구현 방향:

1. GameMode의 기존 PlayerStart 선택 흐름을 최대한 활용한다.
2. PlayerStart가 부족하면 fallback Transform을 사용하되, 경고 로그를 남긴다.
3. TestMap에 PlayerStart가 1개뿐이라는 점을 코드 주석 또는 로그로 고려한다.
4. PlayerStart를 추가로 배치하는 맵 수정은 하지 않는다.

### Task 3. 서버 권한 Spawn/Possess 처리

구현 방향:

1. 서버에서만 차량 Pawn을 Spawn한다.
2. Spawn 성공 후 PlayerController가 해당 차량 Pawn을 Possess한다.
3. Spawn 실패 시 명확한 Warning 로그를 남긴다.
4. PlayerController가 null인 경우 안전하게 종료한다.
5. 이미 Pawn이 존재하는 경우 중복 Spawn하지 않도록 방어한다.

### Task 4. GameMode 적용 방식 정리

구현 방향:

1. C++ GameMode를 프로젝트 기본 GameMode로 적용할지 검토한다.
2. Config에 GlobalDefaultGameMode 또는 GlobalDefaultServerGameMode를 추가하는 경우 변경 이유를 명확히 기록한다.
3. uasset이나 umap을 직접 수정하지 않는다.
4. BP 설정이 필요한 경우 후속 수동 작업으로 보고한다.

### Task 5. 빌드 및 최소 검증

검증 후보:

1. Development Editor 빌드.
2. Development Server 빌드.
3. PIE Play As Listen Server 빙의 확인.
4. PIE Play As Client 빙의 확인.
5. Dedicated Server + open 127.0.0.1:7777 접속 시 검정 화면 해소 여부 확인.

빌드나 실행이 불가능하면 명령과 이유를 남긴다.

## Acceptance Criteria

1. 최소 GameMode C++ 클래스가 추가되어 있다.
2. 서버에서 접속한 PlayerController마다 차량 Pawn을 Spawn/Possess하는 흐름이 있다.
3. 클라이언트가 직접 Spawn/Possess하지 않는다.
4. Spawn할 차량 Pawn Class를 에디터나 Blueprint에서 설정할 수 있다.
5. 무기, 체력, 대미지, 스코어, 리스폰 코드는 추가되지 않았다.
6. TestMap.umap과 BP_CFVehiclePawn.uasset은 직접 수정되지 않았다.
7. Play As Client 또는 Dedicated 접속 검정 화면 문제를 해결하기 위한 최소 구조가 마련되어 있다.
8. 빌드 결과와 남은 수동 작업이 보고되어 있다.

## Verification

가능한 검증 순서:

1. CarFight_ReEditor 빌드.
2. CarFight_ReServer 빌드.
3. PIE Number of Players 2, Play As Listen Server 테스트.
4. PIE Play As Client 테스트.
5. Staged Dedicated Server 실행.
6. Client 1에서 open 127.0.0.1:7777.
7. Client 2에서 open 127.0.0.1:7777.
8. 각 클라이언트가 자기 차량을 소유하는지 확인.
9. 입력이 자기 차량에만 적용되는지 확인.
10. 서로의 차량 이동이 보이는지 확인.

## Reporting Requirements

Codex는 작업 후 아래 내용을 요약한다.

1. 변경한 파일 목록.
2. 새로 추가한 클래스와 역할.
3. Spawn/Possess 흐름 설명.
4. GameMode 적용 방식.
5. 빌드 결과.
6. PIE 테스트 결과.
7. Dedicated Server 접속 테스트 결과.
8. 남은 수동 작업.
9. TestMap PlayerStart 부족 문제가 남아 있는지 여부.

## Stop Conditions

아래 상황에서는 임의로 구현을 계속하지 말고 중단 후 보고한다.

1. PlayerController 신규 클래스가 반드시 필요하다고 판단되는 경우.
2. TestMap.umap 수정이 반드시 필요하다고 판단되는 경우.
3. BP_CFVehiclePawn Blueprint 그래프 수정이 필요하다고 판단되는 경우.
4. 차량 선택 시스템이 필요하다고 판단되는 경우.
5. 리스폰 시스템이 필요하다고 판단되는 경우.
6. 차량 이동 네트워크 예측 구조를 새로 설계해야 한다고 판단되는 경우.
7. 무기, 체력, 스코어 같은 제외 범위를 건드려야 한다고 판단되는 경우.

## Unresolved

1. TestMap에는 PlayerStart가 1개뿐이므로 2클라 Spawn 위치가 겹칠 수 있다.
2. BP_CFVehiclePawn을 Spawn Class로 지정하는 가장 안전한 방식은 코드 작업 중 확인이 필요하다.
3. Config로 GameMode를 지정할지, 사용자가 에디터에서 수동 지정할지는 작업 중 판단이 필요하다.
4. Dedicated Server에서 검정 화면이 GameMode 부족 때문인지 ViewTarget 설정 때문인지는 Spawn/Possess 구현 후 다시 확인해야 한다.

## Source References

1. Document/ProjectSSOT/Plan/MP_ServerPlan/Scope.md
2. Document/ProjectSSOT/Plan/MP_ServerPlan/FeatureList.md
3. Document/ProjectSSOT/Plan/MP_ServerPlan/NetAudit.md
4. Document/ProjectSSOT/Plan/MP_ServerPlan/RiskSearch.md
5. Document/ProjectSSOT/Plan/MP_ServerPlan/MapModeCheck.md
6. Document/ProjectSSOT/Plan/MP_ServerPlan/PawnOwnCheck.md
7. Document/ProjectSSOT/Plan/MP_ServerPlan/PlayTestLog.md
