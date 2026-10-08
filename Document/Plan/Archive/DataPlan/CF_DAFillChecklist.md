# CarFight Vehicle DA 입력 실무 체크리스트 v0.1.1

- 작성일: 2026-06-23
- 대상 프로젝트: `UE/CarFight_Re`
- 대상 작업: 새 차량 추가 시 `CFVehicleData` 기반 `DA_*` 입력과 1차 검증
- 문서 상태: Draft
- 기준 기능개발계획서: `Document/Plan/DataPlan/CF_DAFillToolPlan.md`
- 기준 차량 DA: `/Game/CarFight/Vehicles/Data/Cars/DA_TestSedan`
- 기준 차량 Pawn: `/Game/CarFight/Vehicles/BP_CFVehiclePawn`

---

## 1. 사용 목적

이 문서는 새 차량 `DA_*`를 만들 때 툴이 없어도 같은 순서로 빠짐없이 입력하고 검증하기 위한 실무 체크리스트다.

목표는 좋은 차량 튜닝값을 한 번에 찾는 것이 아니다. 목표는 아래 3가지다.

1. 필수 참조 누락을 막는다.
2. 레이아웃과 Movement 수치를 한 번에 섞어 바꾸지 않는다.
3. PIE 검증 결과를 남겨 다음 튜닝 판단을 이어갈 수 있게 한다.

---

## 2. 작업 전 원칙

- [ ] 기준 DA는 기본적으로 `DA_TestSedan`을 사용한다.
- [ ] 새 차량 DA는 빈 자산으로 시작하지 않고 기준 DA를 복제해서 시작한다.
- [ ] 한 번에 여러 수치 그룹을 바꾸지 않는다.
- [ ] 차체/휠 참조와 레이아웃이 안정되기 전에는 Movement 감각 튜닝을 하지 않는다.
- [ ] `CenterOfMassOverride`는 마지막 단계 전까지 건드리지 않는다.
- [ ] 테스트 결과를 저장하지 않은 상태에서 다음 수치 그룹으로 넘어가지 않는다.

---

## 3. 새 DA 생성 체크

### 3.1 콘텐츠 브라우저 작업

에디터 경로:
- 콘텐츠 브라우저(`Content Browser`)
- `/Game/CarFight/Vehicles/Data/Cars`

작업:
- [ ] `DA_TestSedan`을 찾았다.
- [ ] `DA_TestSedan`을 우클릭했다.
- [ ] 복제(`Duplicate`)를 선택했다.
- [ ] 새 이름을 `DA_차량명` 형식으로 정했다.
- [ ] 새 DA가 `/Game/CarFight/Vehicles/Data/Cars` 아래에 생성되었다.

권장 이름:
- `DA_TestSedan`
- `DA_TestSUV`
- `DA_Sports`
- `DA_Truck`

금지:
- [ ] 임시 이름 `NewDataAsset`, `DataAsset1`, `Copy` 상태로 남기지 않았다.
- [ ] 기준 DA인 `DA_TestSedan`을 직접 수정하지 않았다.

---

## 4. 필수 참조 입력 체크

새 DA를 열고 세부 정보(`Details`) 패널에서 아래 값을 확인한다.

### 4.1 VehicleVisualConfig

- [ ] `차체 메쉬 (ChassisMesh)`가 새 차량 차체 Static Mesh로 지정되었다.
- [ ] `앞왼쪽 휠 메쉬 (WheelMeshFL)`이 지정되었다.
- [ ] `앞오른쪽 휠 메쉬 (WheelMeshFR)`이 지정되었다.
- [ ] `뒤왼쪽 휠 메쉬 (WheelMeshRL)`이 지정되었다.
- [ ] `뒤오른쪽 휠 메쉬 (WheelMeshRR)`이 지정되었다.

허용 조건:
- [ ] FR/RL/RR 휠 메쉬가 아직 없다면, 임시로 FL과 같은 휠 메쉬를 쓰는지 명확히 기록했다.

실패 시 예상 결과:
- 차체가 보이지 않는다.
- 일부 바퀴가 보이지 않는다.
- WheelSync 시각 검증이 어렵다.

### 4.2 VehicleReferenceConfig

- [ ] `전륜 Wheel Class (FrontWheelClass)`가 지정되었다.
- [ ] `후륜 Wheel Class (RearWheelClass)`가 지정되었다.

기본 권장:
- 전륜: `BP_Wheel_Front`
- 후륜: `BP_Wheel_Rear`

실패 시 예상 결과:
- Chaos Vehicle 휠 세팅이 적용되지 않는다.
- 차량이 움직이지 않거나 휠 물리 적용이 비정상일 수 있다.

---

## 5. 차체 소켓 체크

차체 Static Mesh를 열고 소켓 매니저(`Socket Manager`)를 확인한다.

에디터 경로:
- 새 DA의 `차체 메쉬 (ChassisMesh)` 더블클릭
- Static Mesh 에디터(`Static Mesh Editor`)
- 소켓 매니저(`Socket Manager`)

필수 소켓:
- [ ] `Wheel_Anchor_FL`이 있다.
- [ ] `Wheel_Anchor_FR`이 있다.
- [ ] `Wheel_Anchor_RL`이 있다.
- [ ] `Wheel_Anchor_RR`이 있다.

위치 기준:
- [ ] `Wheel_Anchor_FL`은 앞왼쪽 바퀴 중심에 있다.
- [ ] `Wheel_Anchor_FR`은 앞오른쪽 바퀴 중심에 있다.
- [ ] `Wheel_Anchor_RL`은 뒤왼쪽 바퀴 중심에 있다.
- [ ] `Wheel_Anchor_RR`은 뒤오른쪽 바퀴 중심에 있다.

예외 처리:
- [ ] 소켓 이름이 다르면 `VehicleLayoutConfig`의 `BodyWheelSocketFL/FR/RL/RR`에 실제 소켓 이름을 적었다.
- [ ] 예외 소켓 이름을 작업 메모에 기록했다.

실패 시 처리:
- 소켓이 없으면 레이아웃 캡처를 하지 않는다.
- 소켓 위치가 틀리면 먼저 Static Mesh 소켓을 수정한다.

---

## 6. 레이아웃 캡처 체크

새 DA를 다시 연다.

에디터 경로:
- 새 차량 DA 열기
- 세부 정보(`Details`)
- `CarFight|Vehicle Data|Editor`

작업:
- [ ] `차체 소켓에서 휠 레이아웃 캡처 (Capture Wheel Layout From Chassis Sockets)` 버튼을 실행했다.
- [ ] 실행 후 에디터 메시지 또는 로그에 실패 메시지가 없는지 확인했다.
- [ ] `레이아웃 덮어쓰기 사용 (bUseLayoutOverrides)`이 켜져 있다.
- [ ] `앞왼쪽 바퀴 앵커 (WheelAnchorFL)` 값이 0만으로 남아 있지 않다.
- [ ] `앞오른쪽 바퀴 앵커 (WheelAnchorFR)` 값이 0만으로 남아 있지 않다.
- [ ] `뒤왼쪽 바퀴 앵커 (WheelAnchorRL)` 값이 0만으로 남아 있지 않다.
- [ ] `뒤오른쪽 바퀴 앵커 (WheelAnchorRR)` 값이 0만으로 남아 있지 않다.

판단:
- [ ] 좌우 X/Y 부호가 차량 좌표계 기준으로 이상하지 않다.
- [ ] 앞/뒤 위치가 뒤바뀌지 않았다.
- [ ] 네 바퀴 높이가 차체 기준으로 크게 어긋나지 않는다.

실패 시 처리:
- 소켓 이름을 다시 확인한다.
- 차체 메쉬 소켓 위치를 다시 확인한다.
- 레이아웃 값을 수동으로 먼저 고치지 말고 캡처 실패 원인을 찾는다.

---

## 7. 기준 복사 유지 체크

새 차량 첫 테스트 전에는 아래 항목을 기준 DA 값에서 크게 바꾸지 않는다.

### 7.1 유지 권장 그룹

- [ ] `DriveStateConfig`는 기준 DA 값을 유지했다.
- [ ] `WheelVisualConfig.ExpectedWheelCount`는 4륜 차량이면 `4`를 유지했다.
- [ ] `WheelVisualConfig.FrontWheelCountForSteering`은 전륜 조향 4륜 차량이면 `2`를 유지했다.
- [ ] 브레이크 토크는 기준 DA 값을 유지했다.
- [ ] 마찰 배수는 기준 DA 값을 유지했다.
- [ ] 서스펜션 강성/프리로드는 기준 DA 값을 유지했다.
- [ ] 중심질량 오버라이드는 꺼진 상태를 유지했다.

### 7.2 먼저 바꿔도 되는 측정값

차량 크기가 확실히 다를 때만 아래 값을 먼저 조정한다.

- [ ] 전륜 휠 반지름
- [ ] 후륜 휠 반지름
- [ ] 전륜 휠 폭
- [ ] 후륜 휠 폭
- [ ] 차체 높이

주의:
- 측정값을 바꾼 뒤에는 바로 PIE에서 스폰 안정성을 확인한다.

---

## 8. 새 DA 연결 체크

새 DA를 실제 차량 Pawn에 연결한다.

에디터 경로:
- `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
- 클래스 기본값(`Class Defaults`)
- 세부 정보(`Details`)
- `VehicleData`

작업:
- [ ] `BP_CFVehiclePawn`을 열었다.
- [ ] 클래스 기본값(`Class Defaults`)을 열었다.
- [ ] `VehicleData`에 새 `DA_*`를 지정했다.
- [ ] 블루프린트를 컴파일(`Compile`)했다.
- [ ] 블루프린트를 저장(`Save`)했다.

주의:
- 기존 기준 테스트를 유지해야 하는 경우, `BP_CFVehiclePawn` 원본을 바로 바꾸지 말고 테스트용 파생 BP 또는 테스트 맵 배치 액터에서만 바꾼다.

---

## 9. PIE 1차 스폰 검증

에디터 경로:
- 플레이(`Play`) 또는 선택된 뷰포트에서 플레이(`Selected Viewport`)

검증:
- [ ] 차량이 스폰된다.
- [ ] 시작 즉시 뒤집히지 않는다.
- [ ] 시작 즉시 튀어 오르지 않는다.
- [ ] 차체가 보인다.
- [ ] 네 바퀴가 보인다.
- [ ] 네 바퀴가 차체 근처의 올바른 위치에 있다.
- [ ] 오류 로그가 반복 출력되지 않는다.

실패 시 원인 후보:
- 차체/휠 메쉬 누락
- Wheel Class 누락
- 휠 레이아웃 소켓 위치 오류
- 휠 반지름과 실제 휠 메쉬 크기 차이
- 중심질량 또는 서스펜션 값 이상

판정:
- [ ] 스폰 검증이 통과되기 전에는 엔진/조향/브레이크 튜닝으로 넘어가지 않는다.

---

## 10. PIE 2차 기본 주행 검증

검증 순서:
- [ ] 전진 입력을 눌렀을 때 차량이 전진한다.
- [ ] 입력을 놓으면 차량이 급격하게 불안정해지지 않는다.
- [ ] 좌우 조향 입력에 앞바퀴가 반응한다.
- [ ] 차량이 좌우로 회전한다.
- [ ] 브레이크 입력이 속도를 줄인다.
- [ ] 후진 입력 또는 반대 방향 입력이 비정상적으로 씹히지 않는다.
- [ ] 핸드브레이크 입력이 있다면 동작을 확인했다.

기록:
- [ ] 최고속을 아직 판단하지 않았다.
- [ ] 드리프트 감각을 아직 판단하지 않았다.
- [ ] “움직인다 / 멈춘다 / 돈다”만 먼저 확인했다.

실패 시 처리:
- 전진이 안 되면 Wheel Class와 엔진 영향 휠 설정을 확인한다.
- 조향이 안 되면 전륜 Wheel Class와 `FrontWheelCountForSteering`을 확인한다.
- 브레이크가 안 되면 브레이크 토크와 입력 연결을 확인한다.

---

## 11. 그룹별 튜닝 체크

아래는 한 번에 하나의 그룹만 진행한다.

### 11.1 엔진 그룹

대상:
- `EngineMaxTorque`
- `EngineMaxRPM`
- `EngineIdleRPM`
- `EngineBrakeEffect`
- `EngineRevUpMOI`
- `EngineRevDownRate`

작업:
- [ ] 기준 DA 값과 현재 값을 비교했다.
- [ ] `EngineMaxTorque`를 먼저 소폭 조정했다.
- [ ] PIE에서 직선 가속 5초를 확인했다.
- [ ] 휠스핀 또는 급발진 문제가 있는지 기록했다.

판정:
- 토크를 올렸는데 제어가 어려우면 마찰/다운포스를 바로 올리지 말고 토크 조정부터 되돌아본다.

### 11.2 조향 그룹

대상:
- `FrontWheelMaxSteerAngle`
- `SteeringType`
- `SteeringAngleRatio`

작업:
- [ ] 저속 좌회전/우회전을 확인했다.
- [ ] 중속 좌회전/우회전을 확인했다.
- [ ] 고속에서 과도하게 흔들리는지 확인했다.
- [ ] 조향각을 바꾼 뒤 다른 Movement 값을 동시에 바꾸지 않았다.

판정:
- 저속 회전이 둔하면 조향각을 소폭 올린다.
- 고속에서 불안정하면 조향각을 줄이거나 이후 속도 기반 조향 정책을 별도 검토한다.

### 11.3 브레이크 그룹

대상:
- `FrontWheelMaxBrakeTorque`
- `RearWheelMaxBrakeTorque`
- `RearWheelMaxHandBrakeTorque`

작업:
- [ ] 직선 주행 후 브레이크 정지를 확인했다.
- [ ] 급정지 시 차량이 뒤집히거나 튀지 않는지 확인했다.
- [ ] 핸드브레이크 사용 시 회전이 과하지 않은지 확인했다.

판정:
- 제동이 약하면 브레이크 토크를 소폭 올린다.
- 제동 중 차체가 크게 흔들리면 서스펜션/중심질량보다 브레이크 배분을 먼저 의심한다.

### 11.4 접지 그룹

대상:
- `FrontWheelFrictionForceMultiplier`
- `RearWheelFrictionForceMultiplier`
- `FrontWheelCorneringStiffness`
- `RearWheelCorneringStiffness`
- `FrontWheelLoadRatio`
- `RearWheelLoadRatio`

작업:
- [ ] 원형 회전 테스트를 했다.
- [ ] 좌우 슬라럼 테스트를 했다.
- [ ] 접지값을 바꾼 뒤 엔진 토크를 동시에 바꾸지 않았다.

판정:
- 너무 미끄러우면 마찰 배수를 소폭 올린다.
- 너무 붙어서 장난감처럼 보이면 마찰 배수를 낮춘다.
- 코너 중 튐이 생기면 코너링 강성을 의심한다.

### 11.5 서스펜션 그룹

대상:
- `FrontWheelSpringRate`
- `RearWheelSpringRate`
- `FrontWheelSpringPreload`
- `RearWheelSpringPreload`
- `FrontWheelSuspensionMaxRaise`
- `RearWheelSuspensionMaxRaise`
- `FrontWheelSuspensionMaxDrop`
- `RearWheelSuspensionMaxDrop`

작업:
- [ ] 정지 상태 차고를 확인했다.
- [ ] 가속 시 차체가 과하게 들리거나 꺼지는지 확인했다.
- [ ] 작은 턱 또는 경사에서 바퀴가 차체와 어긋나 보이는지 확인했다.

판정:
- 차체가 너무 출렁이면 SpringRate를 소폭 올린다.
- 차체가 너무 딱딱하게 튀면 SpringRate를 낮춘다.
- 기본 차고가 이상하면 SpringPreload와 휠 레이아웃을 함께 확인한다.

### 11.6 고급 물리 그룹

대상:
- `bEnableCenterOfMassOverride`
- `CenterOfMassOverride`
- `DragCoefficient`
- `DownforceCoefficient`
- `DifferentialType`
- `FrontRearSplit`

작업:
- [ ] 앞 단계가 모두 통과된 뒤에만 이 그룹을 만졌다.
- [ ] 중심질량 변경 전 기존 값을 기록했다.
- [ ] 중심질량 변경 후 전복, 롤링, 급제동 반응을 확인했다.
- [ ] 다운포스 변경 후 고속 안정감만 따로 확인했다.

판정:
- 중심질량은 문제 해결의 마지막 카드로 취급한다.
- 고속 불안정은 조향각, 접지, 서스펜션을 먼저 확인한 뒤 다운포스를 조정한다.

---

## 12. DriveState 검증 체크

DriveState는 물리 감각 튜닝이 아니라 상태 판정 튜닝이다.

확인:
- [ ] 정지 상태에서 Idle로 판정된다.
- [ ] 아주 느린 이동 중 Idle이 과하게 튀지 않는다.
- [ ] 후진 입력 시 Reversing으로 판정된다.
- [ ] 전진/후진 전환 중 상태가 프레임마다 튀지 않는다.
- [ ] 점프 또는 낙하 상황에서 Airborne 판정이 과하게 빨리 들어가지 않는다.
- [ ] 착지 후 Airborne 상태가 너무 오래 남지 않는다.

조정 후보:
- Idle이 너무 빨리 들어오면 `IdleEnterSpeedThresholdKmh`를 낮추거나 `IdleExitSpeedThresholdKmh`를 확인한다.
- Reversing이 튀면 `ReverseEnterSpeedThresholdKmh`와 `ReverseExitSpeedThresholdKmh`를 확인한다.
- Airborne이 과하면 `AirborneMinSpeedThresholdKmh`와 `AirborneVerticalSpeedThresholdCmPerSec`를 확인한다.

주의:
- DriveState 문제를 엔진 토크나 브레이크 수치로 해결하려고 하지 않는다.

---

## 13. 테스트 기록 양식

아래 양식을 DA별로 남긴다.

```text
테스트 일시:
대상 DA:
기준 DA:
변경 그룹:
변경 필드:
변경 전 값:
변경 후 값:
테스트 맵:
PIE 결과:
문제 현상:
원인 후보:
다음 작업:
```

기록 예시:

```text
테스트 일시: 2026-06-23 13:00
대상 DA: DA_TestSUV
기준 DA: DA_TestSedan
변경 그룹: 엔진
변경 필드: EngineMaxTorque
변경 전 값: 750
변경 후 값: 850
테스트 맵: TestMap
PIE 결과: 직선 가속은 좋아졌지만 출발 시 휠스핀이 강함
문제 현상: 저속 출발 때 좌우 흔들림 증가
원인 후보: 토크 과다 또는 후륜 접지 부족
다음 작업: 토크를 800으로 낮춘 뒤 재검증
```

---

## 14. 완료 판정 체크

새 차량 DA 1차 입력 완료 조건:

- [ ] 필수 참조가 모두 채워졌다.
- [ ] 차체 소켓 4개가 확인되었다.
- [ ] 휠 레이아웃 캡처가 완료되었다.
- [ ] 새 DA가 차량 Pawn에 연결되었다.
- [ ] PIE에서 스폰 안정성을 통과했다.
- [ ] 전진/조향/브레이크/후진 기본 주행을 통과했다.
- [ ] WheelVisual 위치가 크게 어긋나지 않는다.
- [ ] DriveState가 심하게 튀지 않는다.
- [ ] 테스트 기록을 남겼다.

통과 후 다음 단계:
- 엔진 그룹 튜닝
- 조향 그룹 튜닝
- 브레이크 그룹 튜닝
- 접지 그룹 튜닝
- 서스펜션 그룹 튜닝
- 고급 물리 그룹 튜닝

---

## 15. Changelog

### v0.1.1 - 2026-06-25

- `DA_PoliceCar` 폐기 결정에 맞춰 새 차량 복제 기준 DA를 `DA_TestSedan`으로 변경했다.
- 테스트 기록 예시의 기준 DA도 현재 P0 기본 기준에 맞춰 갱신했다.

### v0.1.0 - 2026-06-23

- 새 차량 `CFVehicleData` DA 입력 실무 체크리스트 최초 작성
- 기준 DA 복제, 필수 참조 입력, 소켓 확인, 레이아웃 캡처, PIE 검증 순서를 고정
- Movement 수치를 엔진, 조향, 브레이크, 접지, 서스펜션, 고급 물리 그룹으로 분리
- DriveState 검증 체크와 테스트 기록 양식을 추가

---

## 16. Migration 메모

- 이 문서는 `CF_DAFillToolPlan.md`의 Phase 1 산출물이다.
- 이후 `EUW_VDAWizard`가 구현되면 이 체크리스트의 각 항목을 위젯 단계와 1:1로 연결한다.
- 체크리스트와 툴의 기준이 달라지면 이 문서를 먼저 갱신한 뒤 툴 표시 문구를 맞춘다.
