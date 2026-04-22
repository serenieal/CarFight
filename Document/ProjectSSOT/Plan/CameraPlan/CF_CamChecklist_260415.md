# CarFight Vehicle Camera Checklist

현재시각: 2026-04-21 14:21 (Asia/Seoul)

이 문서는 차량 카메라 작업의 구현 상태와 남은 작업을 점검하기 위한 체크리스트다.
현재 문서는 이전 패치 과정에서 손상된 내용을 복구하면서, 실제 코드 반영 상태 기준으로 다시 정리했다.

---

## 1. 문서 목적

- 차량 카메라 작업의 현재 구현 범위를 빠르게 파악한다.
- 남은 작업을 우선순위 단위로 정리한다.
- 실제 코드에 반영된 항목과 아직 기획/설계 단계인 항목을 분리한다.

---

## 2. 현재 구현 상태 요약

현재까지 확인된 구현 상태는 다음과 같다.

- [x] `UCFVehicleCameraComp` 기반 카메라 뼈대 구현
- [x] `ACFVehiclePawn`에서 Look 입력을 `VehicleCameraComp`로 전달
- [x] 자유 조준용 Yaw / Pitch 누적 구조 구현
- [x] Aim Profile 기반 Yaw / Pitch Clamp 구조 구현
- [x] SpringArm + FollowCamera 연동 구현
- [x] 기본 Arm Length / FOV / Height / Side Offset 계산 구현
- [x] 속도 기반 Arm Length / FOV 보정 구조 구현
- [x] 카메라 런타임 상태 구조체(`FCFVehicleCameraRuntimeState`) 구현
- [x] Aim Trace 계산 및 차단 여부(`bAimBlocked`) 계산 구현
- [x] 빌드 성공 확인

---

## 3. 구현 단계별 체크리스트

### 3.1 1단계 - 구조 준비

- [x] `UCFVehicleCameraComp` 클래스 생성
- [x] Tick 기반 갱신 루프 구성
- [x] `ACFVehiclePawn` 소유자 해석 구조 구현
- [x] `CameraPivotRoot` / `CameraAimPivot` / `CameraBoom` / `FollowCamera` 자동 참조 구조 구현
- [x] BeginPlay 자동 초기화 구조 구현

### 3.2 2단계 - Aim / 회전 구조

- [x] 현재 프레임 Look 입력 누적 구조 구현
- [x] DeltaTime 스케일 적용 옵션 구현
- [x] 누적 Aim Yaw / Pitch 상태 유지
- [x] Aim Profile 기반 Clamp 구조 구현
- [x] Soft Limit 접근 상태 계산 (`bAimAtYawLimit`, `bAimAtPitchLimit`)
- [x] ResetAimToVehicleForward 구현

### 3.3 3단계 - 기본 카메라 계산

- [x] Base Arm Length 계산
- [x] Base FOV 계산
- [x] Base Height Offset 계산
- [x] Base Side Offset 계산
- [x] Combat / Reverse / Airborne 모드별 오프셋 반영 구조 구현
- [x] 속도 기반 Arm Length 보정 구조 구현
- [x] 속도 기반 FOV 보정 구조 구현
- [x] Aim Profile의 Arm/FOV/Height/Side 보정 반영 구조 구현
- [x] `CurrentArmLength` / `CurrentFOV` 보간 적용
- [x] `CameraAimPivot` 월드 위치/회전 반영
- [x] `CameraBoom->TargetArmLength` / `SocketOffset` 반영
- [x] `FollowCamera->FieldOfView` 반영
- [x] `SolvedArmLength` 계산

### 3.4 3단계 보조 - DataAsset / 설정 분리

- [x] `UCFVehicleCameraData` 생성
- [x] `FCFVehicleCameraTuningConfig` 정의
- [x] 기본 튜닝값을 DataAsset/구조체 기준으로 읽는 경로 구현
- [x] 기본 Aim Profile 제공 구조 구현

---

## 3.5 4단계 - 1차 실전 사용 가능 수준 만들기

이 단계는 단순히 작동하는 수준에서 플레이 가능한 수준으로 끌어올리는 작업이다.

### 3.5.1 카메라 감각 튜닝

- [x] Yaw 감도 튜닝
- [x] Pitch 감도 튜닝
- [ ] Pitch 최소 / 최대 제한각 튜닝
- [x] 기본 Arm Length 튜닝
- [x] 기본 FOV 튜닝
- [ ] PivotRoot 높이(Z) 튜닝
- [ ] SocketOffset 튜닝
- [x] 속도에 따른 FOV 증가량 튜닝
- [x] 속도에 따른 거리 증가량 튜닝
- [ ] 충돌 시 카메라가 너무 당겨지지 않도록 추가 조정

적용값 메모 (2026-04-17)
- `LookYawSpeedDegPerSec`: `120.0 -> 150.0`
- `LookPitchSpeedDegPerSec`: `90.0 -> 75.0`
- `BaseArmLength`: `520.0 -> 560.0`
- `BaseHeightOffset`: `70.0 -> 80.0`
- `BaseFOV`: `85.0 -> 87.0`
- `MaxSpeedFOVBonus`: `8.0 -> 6.0`
- `MaxSpeedArmLengthBonus`: `70.0 -> 45.0`
- 적용 파일: `UE/Source/CarFight_Re/Public/CFVehicleCameraData.h`

### 3.5.2 카메라 충돌 감각 튜닝

- [x] SpringArm 기반 충돌 처리 출발점 확보
- [x] 충돌 복귀 튐 완화 1차 패치 적용
- [x] 충돌 시 높이 / FOV 보조 1차 구조 추가
- [ ] 좁은 공간에서 카메라가 튀는지 테스트
- [ ] 충돌 해제 후 복귀 감각 테스트
- [ ] 차체가 화면을 과하게 가리는지 테스트
- [ ] 필요 시 FOV 보조 추가 조정
- [ ] 필요 시 높이 보조 추가 조정
- [ ] 충돌 상황에서 조준 방향 감각 유지 확인

적용값 메모 (2026-04-21)
- `bUseCollisionViewAssist = true`
- `CollisionViewAssistStartRatio = 0.82`
- `MaxCollisionHeightAssist = 18.0`
- `MaxCollisionFOVAssist = 3.0`
- SpringArm 실제 해결 거리(`SolvedArmLength`)를 내부 `CurrentArmLength`에 반영하도록 수정
- 충돌 해제 후 즉시 튀지 않고 코드 보간으로 복귀하도록 조정
- 적용 파일:
  - `UE/Source/CarFight_Re/Public/CFVehicleCameraData.h`
  - `UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp`

### 3.5.3 디버그 기능

- [x] 카메라 런타임 상태 구조체 작성
- [x] Aim Trace 디버그 옵션 구조 존재
- [ ] 화면 디버그 위젯 연결
- [ ] 현재 CameraMode 표시
- [ ] 현재 AimProfile 이름 표시
- [ ] Accumulated / Clamped Yaw/Pitch 표시
- [ ] Current / Desired / Solved ArmLength 표시
- [ ] Current / Desired FOV 표시
- [ ] `bAimBlocked` / `bWeaponCanFireAtCurrentAim` 표시
- [ ] 월드 디버그 라인 정리

### 현재 상태 메모

기본 카메라 계산 로직은 구현되었다. 다만 무기 실데이터 연동, 실전 테스트, 디버그 HUD 연결은 아직 남아 있다.
현재 단계에서 가장 큰 불확실성은 튜닝값 그 자체보다, 복잡한 테스트 조건에서 어떤 상태가 발생하는지 관측 수단이 부족하다는 점이다.

---

## 3.6 5단계 - 무기 시스템 연동

이 단계는 카메라 시스템을 실제 게임 시스템과 결합하는 핵심 단계다.

### 3.6.1 Aim Profile 구조 확정

- [x] 기본 Aim Profile 구조체 정의
- [x] Min / Max Yaw / Pitch 필드 정의
- [x] Soft Limit Zone 필드 정의
- [x] FOV / Arm / Height / Side Offset 필드 정의
- [ ] 무기 시스템 전용 `CFWeaponAimProfile` 파일 분리
- [ ] 무기 DataAsset과 Aim Profile 연결
- [ ] 현재 활성 무기 그룹이 Aim Profile을 공급하도록 연결

### 3.6.2 무기별 제한각 반영

- [ ] 전방 협각 무기용 Profile 구현
- [ ] 전방 확장형 Profile 구현
- [ ] 측면 편향형 Profile 구현
- [ ] 전방향 터렛형 Profile 구현
- [ ] 후방 사격 가능형 Profile 구현

### 3.6.3 카메라와 실제 발사 가능 범위 일치 검증

- [ ] 카메라 회전 가능 범위와 실제 발사 가능 범위 일치 확인
- [ ] 조준 가능하지만 발사 불가한 상태 처리 규칙 정리
- [ ] HUD 경고 규칙 정리

---

## 3.7 6단계 - 테스트 / 검증

### 3.7.1 기본 조작 테스트

- [x] Look 입력 전달 확인
- [x] 기본 Yaw / Pitch 회전 확인
- [x] 기본 Arm Length / FOV 변화 확인
- [ ] Pitch 제한각 동작 확인
- [ ] Aim Profile별 제한각 차이 확인

### 3.7.2 충돌 / 공간 테스트

- [ ] 단일 벽 접근 테스트
- [ ] 좁은 골목 테스트
- [ ] 낮은 천장 테스트
- [ ] 코너에서 비스듬히 붙는 상황 테스트
- [ ] 기둥/장애물 스치기 테스트
- [ ] 후진 상태 충돌 테스트

### 3.7.3 실전 플레이 테스트

- [ ] 주행 중 조준 테스트
- [ ] 고속 진입 후 회전 테스트
- [ ] 조준하면서 벽 따라 주행 테스트
- [ ] 충돌 중 적 추적 가능 여부 확인
- [ ] 차체 가시성 / 조준점 가시성 확인

---

## 3.8 7단계 - 모드 확장

### 3.8.1 Camera Mode

- [x] Normal 모드 구조
- [x] Combat 모드 구조
- [x] Reverse 모드 구조
- [x] Airborne 모드 구조
- [x] Destroyed / Spectate 열거 및 구조 준비
- [ ] 실제 게임 상태와 Reverse 연동
- [ ] 실제 게임 상태와 Airborne 연동
- [ ] 실제 게임 상태와 Destroyed 연동
- [ ] Spectate 진입 규칙 확정

### 3.8.2 입력 / 보조 기능

- [ ] Camera Reset 입력 연결
- [ ] Rear View 입력 연결
- [ ] Aim Modifier 입력 연결

---

## 4. 다음 우선 작업 추천

현재 기준 다음 우선 작업은 아래 순서가 적절하다.

1. 디버그 HUD 최소 연결
2. 충돌 / 좁은 지형 테스트 조건 강화
3. Pitch 제한각 / Pivot / SocketOffset 2차 튜닝
4. 무기 시스템과 Aim Profile 실제 연동

---

## 5. 참고 구현 파일

- `UE/Source/CarFight_Re/Public/CFVehicleCameraData.h`
- `UE/Source/CarFight_Re/Public/CFVehicleCameraTypes.h`
- `UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h`
- `UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp`
- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
- `/Game/CarFight/Vehicles/BP_CFVehiclePawn.BP_CFVehiclePawn`
