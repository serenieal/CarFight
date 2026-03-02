# P0-002 Wheel Visual Sync (BP_ModularVehicle)

## 목표(DoD)
- 전륜 조향: 조향 시 FL/FR 바퀴가 좌우로 자연스럽게 돌아간다.
- 구동 회전: 가속 시 바퀴가 굴러가는(스핀) 모션이 보인다.
- 서스펜션: 노면/가속/감속에 따라 바퀴 위치가 위아래로 움직인다.
- 우측 바퀴 미러 이슈 없음: Wheel_Anchor는 0 유지, Wheel_Mesh만 Yaw 180 규칙을 지킨다.

---

## 전제(이미 확정된 규칙)
- 휠 인덱스: 0=FL, 1=FR, 2=RL, 3=RR
- 컴포넌트: Wheel_Anchor_* (Scene) 아래 Wheel_Mesh_* (StaticMesh)
- 우측(1,3)은 Mesh만 Yaw 180, Anchor는 0 유지
- C++ 유틸: CarFightVehicleUtils::GetRealWheelTransform(VehicleMovement, WheelIndex)

---

## 1) BP_ModularVehicle에서 Tick 그룹 설정
1. BP_ModularVehicle 열기
2. Class Defaults
3. Actor Tick
   - Start with Tick Enabled = true
   - Tick Group = Post Physics (권장)

---

## 2) 변수 추가 (이름/타입/툴팁)
> 변수 툴팁은 BP 변수 상세에서 Tooltip에 그대로 넣기

### 토글
- bEnableWheelVisualSync (bool, default true)
  - Tooltip: "휠 시각 동기화(Tick)를 켜고 끕니다."
- bDebugWheelVisualSync (bool, default false)
  - Tooltip: "휠 동기화 디버그 출력(PrintString)을 켭니다."

### 상수
- RightWheelYawDeg (float, default 180.0)
  - Tooltip: "우측 바퀴(Wheel_Mesh_FR/RR)에 적용할 Yaw 오프셋 각도(기본 180)."

### 참조(권장: 인스턴스 편집 가능)
- VehicleMovementRef (ChaosWheeledVehicleMovementComponent 참조)
  - Tooltip: "Chaos VehicleMovement 참조. GetRealWheelTransform 입력으로 사용."

### 컴포넌트 배열(길이 4 고정)
- WheelAnchorComponents (SceneComponent Array)
  - Tooltip: "[0 FL,1 FR,2 RL,3 RR] Wheel_Anchor_* 컴포넌트 배열."
- WheelMeshComponents (StaticMeshComponent Array)
  - Tooltip: "[0 FL,1 FR,2 RL,3 RR] Wheel_Mesh_* 컴포넌트 배열."

---

## 3) 배열 채우기(가장 안전한 방법: 디테일 패널에서 직접 지정)
1. BP_ModularVehicle 인스턴스를 맵에 배치
2. 인스턴스 디테일에서 WheelAnchorComponents에 다음 순서로 넣기
   - [0] Wheel_Anchor_FL
   - [1] Wheel_Anchor_FR
   - [2] Wheel_Anchor_RL
   - [3] Wheel_Anchor_RR
3. WheelMeshComponents도 동일 순서
   - [0] Wheel_Mesh_FL
   - [1] Wheel_Mesh_FR
   - [2] Wheel_Mesh_RL
   - [3] Wheel_Mesh_RR
4. VehicleMovementRef는 BP 내부 VehicleMovementComponent를 지정

> 주의: 배열이 비거나 순서가 틀리면 바퀴가 뒤섞여 보임.

---

## 4) 함수 추가: UpdateWheelVisuals (BP 함수)
- 함수명: UpdateWheelVisuals
- 입력: 없음
- 출력: 없음

### 함수 내부 로직(휠 0~3 반복)
1) ForLoop (FirstIndex=0, LastIndex=3)
2) 각 Index에서:
   A. GetRealWheelTransform(VehicleMovementRef, Index)
   B. Break Transform
      - Location = WheelLoc
      - Rotation = WheelRot
   C. Anchor 대상: WheelAnchorComponents[Index]
      - SetRelativeLocation(WheelLoc)
      - AnchorYawOnly = MakeRotator(0, WheelRot.Yaw, 0)
      - SetRelativeRotation(AnchorYawOnly)
   D. Mesh 대상: WheelMeshComponents[Index]
      - RightYaw = (Index==1 or Index==3) ? RightWheelYawDeg : 0
      - MeshRot = MakeRotator(WheelRot.Pitch, RightYaw, 0)
      - SetRelativeRotation(MeshRot)

### 디버그(선택)
- if bDebugWheelVisualSync:
  - PrintString: "W{Index} LocZ={WheelLoc.Z} Yaw={WheelRot.Yaw} Pitch={WheelRot.Pitch}"

> 만약 바퀴 스핀이 반대로 보이면: MeshRot의 Pitch에 -1을 곱해 반전한다.

---

## 5) Event Tick 연결
Event Tick:
- Branch (Condition = bEnableWheelVisualSync)
  - True: Call UpdateWheelVisuals

---

## 6) 체크리스트(DoD 테스트)
1) 정지 상태에서 조향 입력
   - FL/FR 바퀴가 좌우로 돌아간다(AnchorYaw 적용 확인)
2) 전진 가속
   - 바퀴가 굴러간다(SpinPitch 적용 확인)
3) 턱/경사 통과
   - Wheel_Anchor가 상하로 움직인다(서스펜션 위치 동기화)
4) 우측 바퀴 꼬임 여부
   - Wheel_Anchor는 0 유지
   - Wheel_Mesh_FR/RR만 Yaw 180 규칙 유지

---

## 7) 흔한 실패와 해결
- 바퀴가 서로 바뀌어 보임: 배열 순서(0 FL,1 FR,2 RL,3 RR) 재확인
- 우측 바퀴가 비틀림: Anchor에 Yaw 180을 주지 말 것(우측은 Mesh만)
- 덜컹거림/한 프레임 늦음: Tick Group을 Post Physics로 변경
