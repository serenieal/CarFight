# P0 Verification (Wheel Sync + Enhanced Input)

## 목적
P0(휠 시각 동기화 + Enhanced Input 세팅)가 "완료"인지 카메라 유무와 관계없이 판정한다.

---

## P0-001 Enhanced Input (완료 조건)
- [ ] IA/IMC 에셋이 /Game/CarFight/Input 아래에 존재한다.
- [ ] IMC_Vehicle_Default가 런타임에 1회만 주입된다(SSOT = BP_ModularVehicle).
- [ ] Throttle / Steer / Brake / Handbrake 입력이 VehicleMovement에 적용되어 차량이 움직인다.

### 확인 메모
- 주입 SSOT: BP_ModularVehicle
- 비고:

---

## P0-002 Wheel Visual Sync (완료 조건 / 카메라 없이도 가능)
### A) 디테일 패널 검증(권장)
PIE 실행 후, World Outliner에서 BP_ModularVehicle 인스턴스 선택:
- [ ] Wheel_Anchor_FL RelativeLocation.Z 값이 노면/가감속에 따라 변한다 (서스펜션/위치 동기화)
- [ ] Wheel_Anchor_FL/FR RelativeRotation.Yaw 값이 조향 입력에 따라 변한다 (조향 동기화)
- [ ] Wheel_Mesh_FL RelativeRotation.Pitch 값이 가속 시 계속 변한다 (스핀 동기화)

### B) 디버그 로그 검증(선택)
- [ ] bDebugWheelVisualSync = true 로 두고, LocZ/Yaw/Pitch 값이 변하는 로그를 확인했다.
  - NOTE: PrintString 노드 사용 시 "Print to Log" 체크 권장

### C) 우측 180 규칙 검증(비틀림 방지)
- [ ] Wheel_Anchor_FR/RR: 회전은 0 유지(특히 Yaw 0 유지)
- [ ] Wheel_Mesh_FR/RR: Yaw 180(또는 RightWheelYawDeg) 적용

### 확인 메모
- 비고:

---

## 카메라 이슈(추적 항목: P0 외)
- [ ] BP_ModularVehicle에 SpringArm+CameraComponent 존재
- [ ] Auto Possess Player = Player 0
- [ ] GameMode DefaultPawnClass = BP_ModularVehicle
- [ ] Possess 이후 ViewTarget 설정 필요 여부 확인

---

## 최종 판정
- P0-001: (PASS / FAIL)
- P0-002: (PASS / FAIL)
- 작성자/날짜:
