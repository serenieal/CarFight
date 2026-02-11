# P0-001 Enhanced Input Migration (Developers -> /Game)

## 목표(DoD)
- IA/IMC 에셋이 Content/Developers 경로가 아니라 /Game/CarFight/Input 아래에 존재
- 플레이 시 IMC_Vehicle_Default가 정상 주입(Add Mapping Context)되고 차량 입력이 동작
- Redirector 정리(Fix Up Redirectors) 완료

---

## 0) 현재 상태(확정)
- IA/IMC: Content/Developers/chaeksong/Input 아래 존재
- BP_ModularVehicle: Content/Developers/chaeksong/Blueprints/BP_ModularVehicle.uasset
- DefaultInput.ini에서 EnhancedPlayerInput/EnhancedInputComponent 설정은 이미 되어있음

---

## 1) 에셋 이관(UE Editor)
1. UE Editor 열기
2. Content Browser에서 Content/Developers/chaeksong/Input 폴더 선택
3. IA_*, IMC_Vehicle_Default 모두 선택
4. 우클릭 -> Asset Actions -> Move To...
5. 대상 폴더 생성/선택: /Game/CarFight/Input
6. 이동 완료 후 Save All

---

## 2) Redirector 정리(UE Editor)
1. Content Browser에서 /Game/CarFight 폴더 우클릭
2. Fix Up Redirectors in Folder
3. Save All

---

## 3) 주입 지점 단일화(UE Editor: Find in Blueprints)
1. Tools -> Find in Blueprints
2. 검색어: Add Mapping Context
3. 검색 결과에서 IMC_Vehicle_Default를 주입하는 블루프린트(대개 PlayerController 또는 Pawn)를 찾는다
4. ‘주입 SSOT(단일 진실 소스)’로 하나만 남긴다
   - 권장 SSOT: PlayerController(Possessed) 또는 Pawn(BeginPlay + LocalPlayer Subsystem)
5. 중복 주입이 있으면 한 곳만 유지하고 나머지는 제거

---

## 4) 런타임 검증(DoD 테스트)
1. PIE 시작
2. 입력 확인
   - Throttle/Steer/Brake/Handbrake가 VehicleMovement에 적용되어 차량이 움직임
3. 주입 확인(디버그)
   - 주입 블루프린트에 PrintString으로 "IMC Added" 한 번만 찍히게 확인

---

## 5) 커밋 단위(권장)
- Commit A: "docs: add input migration guide" (이 문서)
- Commit B: "content: migrate enhanced input assets" (에셋 이동 + redirector 정리)
- Commit C: "bp: unify mapping context injection" (주입 지점 단일화)
