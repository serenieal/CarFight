# CarFight — 16_CoreDecisionLog

> 역할: CarFight 현재 기준 차량 코어의 **유지 결정 / 교체 결정 / 임시 운영 판단**을 기록한다.  
> 상위 방향 문서: `03_VisionAlign.md`  
> 마지막 정리(Asia/Seoul): 2026-04-01

---

## 현재 기준 메모
- 현재 기준 플레이 차량: `BP_CFVehiclePawn`
- 현재 기준 Native Pawn: `ACFVehiclePawn`
- 현재 기준 주행 코어: `UCFVehicleDriveComp`
- 현재 기준 휠 시각 동기화 코어: `UCFWheelSyncComp`
- 현재 기준 데이터 축: `UCFVehicleData`
- 현재 기준 테스트 자산: `DA_PoliceCar`

---

## 유지 결정
### CF-DL-0030 — 현재 구현 조합을 기준선으로 유지
- 판정: 확정
- 결정:
  - 현재 구현 조합은 `ACFVehiclePawn / BP_CFVehiclePawn / UCFVehicleDriveComp / UCFWheelSyncComp / UCFVehicleData` 기준선으로 유지한다.
- 이유:
  - 실제 구현 기준선과 문서 기준선을 일치시켜야 이후 전환 설계가 명확해진다.

---

### CF-DL-0031 — DriveState는 유지 코어로 본다
- 판정: 확정
- 결정:
  - `UCFVehicleDriveComp`는 단순 입력 전달자가 아니라 공통 상태 판단 코어로 유지한다.
- 이유:
  - 최종 구조가 바뀌더라도 상태 판단 규칙은 재사용 가능한 코어로 남길 가치가 있다.

---

### CF-DL-0032 — WheelSync는 유지 코어로 본다
- 판정: 확정
- 결정:
  - `UCFWheelSyncComp`는 특정 차량 전용 임시 처리보다 공통 시각 동기화 코어로 유지한다.
- 이유:
  - 구조 전환 이후에도 바퀴 시각 동기화 문제는 계속 존재하므로 독립 코어로 유지하는 편이 안전하다.

---

### CF-DL-0033 — VehicleData는 유지 코어로 본다
- 판정: 확정
- 결정:
  - `UCFVehicleData`는 공통 코어와 차량별 차이를 연결하는 데이터 축으로 유지한다.
- 이유:
  - 다차종 확장성과 구조 전환 모두에서 데이터 분리 축은 계속 필요하다.

---

### CF-DL-0034 — BP_CFVehiclePawn은 Thin BP 유지
- 판정: 확정
- 결정:
  - `BP_CFVehiclePawn`는 표현 / 조립 / 반응 중심 Thin BP로 유지한다.
  - 새 상태 판정 / 복잡 분기 / 중복 규칙을 BP에 다시 키우지 않는다.
- 이유:
  - 구조 전환 전후를 막론하고 책임 분리를 유지해야 이후 교체 작업이 쉬워진다.

---

## 교체 결정
### CF-DL-0035 — 현재 vehicle root 구조는 최종 구조가 아니다
- 판정: 확정
- 결정:
  - 현재 `ChaosWheeledVehicleMovementComponent` 중심 vehicle root 구조는 최종 구조로 확정하지 않는다.
- 이유:
  - 상위 방향은 `CMVS / Cluster Union / Geometry Collection` 기반 구조를 목표로 한다.

---

### CF-DL-0036 — 차량 조립 방식은 교체 후보로 둔다
- 판정: 확정
- 결정:
  - 현재 `ApplyVehicleVisualConfig / ApplyVehicleLayoutConfig` 중심 구성 방식은 현 기준선으로 유지하되, 최종 조립 방식으로 고정하지 않는다.
- 이유:
  - 상위 방향의 에디터 조립형 / 데이터 기반 재구성 흐름과 아직 차이가 있다.

---

### CF-DL-0037 — 파괴 / 분리 구조는 후속 설계 대상으로 둔다
- 판정: 확정
- 결정:
  - 차량 파괴 / 분리 / 무게 중심 변화는 아직 현 구조에 직접 반영하지 않았으며, 최종 구조 전환 설계의 핵심 항목으로 남긴다.
- 이유:
  - 현재 코어는 주행 기준선 확보 단계이고, 최종 전투 물리 구조는 아직 설계 입력값 정리가 먼저다.

---

## 운영 판단
### CF-DL-0038 — GamepadOnly는 임시 운영 편차로 본다
- 판정: 확정
- 결정:
  - 현재 게임패드 중심 테스트 운용은 입력 전환 이슈를 피하기 위한 임시 편차로 본다.
- 이유:
  - 이 문제는 현재 구조 갭과 별개로 다뤄야 우선순위가 흐려지지 않는다.

---

### CF-DL-0039 — BP_ModularVehicle 삭제는 역사 기록으로 본다
- 판정: 확정
- 결정:
  - `BP_ModularVehicle`의 부재 자체를 현재 방향 이탈로 해석하지 않는다.
- 이유:
  - 핵심은 해당 BP의 존재 여부가 아니라, 현재 구조가 최종 방향을 향한 중간 기반인지 여부다.

---

### CF-DL-0040 — 테스트 차량은 기준선이지 종착점이 아니다
- 판정: 확정
- 결정:
  - `DA_PoliceCar`는 현재 첫 기준 차량이며, 최종 목표 차량처럼 취급하지 않는다.
- 이유:
  - 공통 코어가 특정 차량만을 위한 구조로 굳는 것을 막아야 한다.

---

### CF-DL-0041 — 에디터 비가용 기간에는 문서 작업을 우선한다
- 판정: 확정
- 결정:
  - 현재 `ue-assetdump` 플러그인 개선으로 Unreal Editor 실행이 불가능한 동안에는 문서 정렬, 실측 반영, 검증 절차 고정 작업을 우선한다.
- 이유:
  - 런타임 검증이 불가능한 상태에서 추정으로 구현을 밀어붙이면 문서와 실제가 다시 어긋날 수 있다.

---

### CF-DL-0042 — P0 런타임 PASS/FAIL은 에디터 복구 후 재개한다
- 판정: 확정
- 결정:
  - `P0-001 / P0-002 / P0-003`의 최종 PASS / FAIL 판정은 에디터 복구 후 PIE 재검증으로 마무리한다.
- 이유:
  - 현재 단계에서 가능한 것은 자산 / 코드 / 문서 기준 실측까지이며, 런타임 판정은 대체할 수 없다.

---

### CF-DL-0043 — 에디터 복구 후 첫 실행 우선순위는 P0 런타임 검증이다
- 판정: 확정
- 결정:
  - 2026-03-30 기준 에디터 복구가 확인된 이후 첫 실행 작업은 `P0-001 / P0-002 / P0-003` 런타임 검증으로 시작한다.
- 이유:
  - 문서와 자산 기준 정리는 이미 끝났고, 지금 가장 부족한 것은 실제 PIE 기준 PASS / FAIL 데이터다.

---

### CF-DL-0044 — WheelSync 수정은 기준점 정렬을 먼저 본다
- 판정: 확정
- 결정:
  - `WheelSync` 수정은 먼저 `Wheel_Anchor` 기준점과 `Wheel_Mesh` 재배치 축 정렬부터 확인한다.
- 이유:
  - 현재 관찰된 초기 배치 FAIL과 조향 피벗 FAIL은 같은 축에서 설명될 가능성이 높다.

---

### CF-DL-0045 — 바퀴 스핀 미적용은 별도 2순위로 본다
- 판정: 확정
- 결정:
  - 바퀴 스핀 시각 반응은 `Anchor/Layout` 정렬 다음 우선순위로 본다.
- 이유:
  - 현재 설정상 `bUseWheelSpinPitchDebugPipe = false`, `bApplySpinPitchInCpp = false`라 별도 경로 문제로 분리해 보는 편이 명확하다.

---

### CF-DL-0046 — DriveState는 수정 우선순위에서 제외한다
- 판정: 확정
- 결정:
  - `DriveState` 코어는 현재 기준선에서 PASS이므로, WheelSync 수정 단계의 직접 대상에서 제외한다.
- 이유:
  - 현재 실패는 WheelSync 쪽에 집중되어 있고, DriveState는 디버그 UX 개선 정도만 후속 과제로 남아 있다.

---

### CF-DL-0047 — WheelSync 수정 전에는 Anchor 중심 기준을 먼저 검토한다
- 판정: 잠정 확정
- 결정:
  - WheelSync 직접 수정 전에 `Wheel_Anchor_*`를 바퀴 중심점 / 조향 피벗의 우선 기준으로 두는 구조를 먼저 검토한다.
- 이유:
  - 현재 확인된 코드 경로상 조향 회전 기준은 Anchor 쪽에 있고, 초기 배치 FAIL과 조향 피벗 FAIL도 같은 축에서 설명될 가능성이 높다.

---

### CF-DL-0048 — Wheel_Mesh는 시각 표현 전용 책임으로 좁히는 방향을 우선 검토한다
- 판정: 잠정 확정
- 결정:
  - `Wheel_Mesh_*`는 Anchor 자식 기준의 시각 표현 전용 책임으로 두고, 조향 피벗 기준은 맡기지 않는 방향을 우선 검토한다.
- 이유:
  - 현재는 Layout이 Mesh를 이동시키고 WheelSync는 Anchor를 회전시키는 구조라 책임 경계가 흐려져 있다.

---

### CF-DL-0049 — 스핀 복구는 배치 기준 정리 다음 순서로 둔다
- 판정: 확정
- 결정:
  - 바퀴 스핀 시각 반응 복구는 `Anchor / Mesh` 기준 정리 이후에 진행한다.
- 이유:
  - 현재 스핀 미적용은 설정상 설명이 가능하지만, 배치 기준이 먼저 정리되지 않으면 스핀을 살려도 결과 해석이 다시 흐려질 수 있다.

---

### CF-DL-0050 — 실제 수정 전에는 메시 피벗과 배치 대상 컴포넌트를 먼저 확인한다
- 판정: 확정
- 결정:
  - 코드 수정 전에 `Wheel_Mesh` 원본 피벗과 `WheelLayout`의 최종 배치 대상 컴포넌트를 먼저 확인한다.
- 이유:
  - 코드 문제와 메시 자산 문제를 섞어서 수정하면 원인 분리가 다시 무너질 수 있다.

---

### CF-DL-0051 — WheelLayout의 기본 배치 대상은 Anchor로 본다
- 판정: 확정
- 결정:
  - 현재 기준선에서는 `WheelLayout` / AutoFit의 기본 배치 대상을 `Wheel_Anchor_*`로 본다.
- 이유:
  - 사용자 확인 결과 `Wheel_Mesh_*` 상대 위치/회전은 모두 0이고, `WheelLayout = 0`일 때 Anchor가 차량 중앙에 몰린다.
  - 따라서 바퀴 중심점 배치 책임은 Mesh보다 Anchor가 맡는 해석이 더 자연스럽다.

---

### CF-DL-0052 — Wheel_Mesh의 0 기준은 정상 상태로 본다
- 판정: 확정
- 결정:
  - `Wheel_Mesh_*`의 상대 위치/회전이 0인 상태를 비정상으로 보지 않고, Anchor 자식 기준의 정상 기본값으로 본다.
- 이유:
  - 메시 피벗도 정상으로 확인됐으므로, 현재 문제를 Mesh 기본값보다는 Layout 대상 선정 문제로 보는 편이 타당하다.

---

### CF-DL-0053 — 직접 수정 1순위는 ApplyVehicleLayoutConfig의 배치 대상 교정이다
- 판정: 확정
- 결정:
  - 첫 코드 수정은 `ACFVehiclePawn::ApplyVehicleLayoutConfig()`가 `Wheel_Mesh_*`가 아니라 `Wheel_Anchor_*`를 배치하도록 교정하는 것으로 시작한다.
- 이유:
  - 초기 배치 FAIL과 조향 피벗 FAIL을 동시에 설명하는 가장 강한 원인 축이기 때문이다.

---

### CF-DL-0054 — 스핀 복구는 배치 교정 검증 후 진행한다
- 판정: 확정
- 결정:
  - 바퀴 스핀 시각 복구는 Anchor 배치 교정 후 PIE 재검증이 끝난 다음 단계로 진행한다.
- 이유:
  - 현재 스핀 미적용 원인은 비교적 명확하지만, 배치 기준을 먼저 바로잡아야 스핀 결과도 안정적으로 해석할 수 있다.

---

### CF-DL-0055 — PoliceCar는 수동 Anchor 기준선을 우선 기준으로 고정한다
- 판정: 확정
- 결정:
  - `PoliceCar`의 현재 기준선은 `Wheel_Anchor_*` 수동 배치 좌표를 우선 기준으로 고정한다.
- 이유:
  - 수동 배치 기준에서는 바퀴 위치와 조향 피벗이 모두 PASS였다.
  - 즉 현재 실패 본체는 WheelSync 철학보다 AutoFit 경로 쪽에 더 가깝다.

---

### CF-DL-0056 — AutoFit은 레거시 필드로만 남긴다
- 판정: 확정
- 결정:
  - `AutoFit`은 현재 기준 런타임에서 사용하지 않으며, 레거시 필드로만 남긴다.
- 이유:
  - 현재 단계에서 AutoFit은 기준선보다 불안정했고, 수동 Anchor 기준선은 이미 실제 PASS 결과를 제공했다.
  - 프로젝트 방향상 앞으로도 자동 / 반자동 배치를 기본안으로 채택하지 않는다.

---

### CF-DL-0057 — 휠하우스 자동 검출은 런타임 기본안으로 채택하지 않는다
- 판정: 확정
- 결정:
  - 휠하우스 형상 기반 자동 검출은 현재 프로젝트 계획에서 제외하며, 런타임 기본안으로 채택하지 않는다.
- 이유:
  - 이미지에서 사람이 휠하우스를 읽는 것과, UE Static Mesh 기하 정보만으로 안정적으로 바퀴 중심을 추론하는 것은 다른 문제다.
  - 현재 프로젝트 단계에서는 마커 / 소켓 / 수동 기준선 데이터화가 더 안전하다.

---

### CF-DL-0058 — Wheel spin 방향은 raw 물리 부호를 그대로 믿지 않는다
- 판정: 확정
- 결정:
  - 휠 시각 회전 방향은 raw `WheelAngularVelocity` 부호를 그대로 적용하지 않는다.
  - 현재 기준선에서는 `ForwardSpeedKmh`를 우선 방향 안정화 기준으로 보고, 휠 각속도는 회전 크기와 강한 슬립 상황 판단에 사용한다.
  - 정지 근처 / 저속 구간에서는 dead zone과 sign hold를 둬서 방향이 프레임 단위로 뒤집히지 않게 한다.
- 이유:
  - PIE 기준 `Speed=0`, `Forward=0` 근처에서도 `WheelAngularVelocity`는 작은 음수 / 양수 노이즈를 냈다.
  - 이 raw 부호를 그대로 적분한 결과, 전진 / 후진 반복 이후 바퀴가 정방향과 역방향으로 번갈아 보이는 문제가 확인됐다.

---

### CF-DL-0059 — DriveState 확장보다 WheelSpinRuntimeState를 우선 둔다
- 판정: 확정
- 결정:
  - 이 문제를 해결하기 위해 `DriveState` FSM을 확장하지 않는다.
  - 대신 `UCFWheelSyncComp` 내부에 시각 회전 안정화 전용의 `WheelSpinRuntimeState` 성격 상태를 둔다.
  - 이 상태는 최소한 `stable sign`, `direction change hold`, `accumulated visual spin`을 유지한다.
- 이유:
  - 현재 문제는 차량의 큰 상태보다, 휠 시각 회전 방향 안정화 규칙 부족에서 발생한다.
  - `DriveState`는 상위 주행 상태 표현에는 적합하지만, 휠 시각 회전의 미세 방향 전환을 직접 제어하는 용도로는 부족하다.

---

### CF-DL-0060 — Wheel spin은 absolute pitch보다 delta local rotation을 우선 사용한다
- 판정: 확정
- 결정:
  - `UCFWheelSyncComp`의 wheel spin 시각 적용은 누적 absolute pitch를 매 프레임 강제로 대입하는 방식보다, 프레임별 `delta local rotation` 경로를 우선 사용한다.
  - 디버그와 내부 누적 상태는 계속 유지하되, 실제 mesh 적용은 delta 회전 기준으로 본다.
- 이유:
  - 누적 absolute pitch를 그대로 적용하면 회전 래핑 구간에서 렌더러가 큰 회전 변화처럼 해석할 가능성이 있다.
  - 현재 프로젝트 PoliceCar 기준에서는 delta 회전 경로가 시각적으로 더 안정적이고 디버그 해석도 명확했다.

---

### CF-DL-0061 — PoliceCar wheel mesh 축 보정은 WheelSpinMeshAxisSign으로 관리한다
- 판정: 확정
- 결정:
  - 현재 프로젝트 PoliceCar 기준 wheel mesh의 로컬 Pitch 축은 Chaos 휠 각속도 / Forward 기준과 반대 방향으로 읽힌다.
  - 이 축 차이는 `WheelSpinVisualSign`를 기존 의미와 섞어 해결하지 않고, 별도 보정값 `WheelSpinMeshAxisSign`으로 관리한다.
  - 현재 기준 기본값은 `-1.0`이다.
- 이유:
  - PIE 디버그 기준으로 후진 시 `Forward < 0`, 전진 시 `Forward > 0`인데도 시각 바퀴가 일관되게 반대로 보였다.
  - 이는 sign hold 로직 실패가 아니라 mesh 축 부호 차이 문제였고, 별도 축 보정값으로 분리하는 편이 차종별 예외 관리에도 안전하다.

---

### CF-DL-0062 — 바퀴 파묻힘 1순위는 WheelRadius 기준선으로 본다
- 판정: 확정
- 결정:
  - `PoliceCar` 바퀴 파묻힘 이슈는 `Wheel_Anchor` 시각 기준선보다 `DA_PoliceCar`의 `WheelRadius` 기준선을 먼저 맞추는 문제로 본다.
  - 현재 세션 검증 기준으로 `WheelRadius = 39`는 작고, `WheelRadius = 43`에서 자연스러운 접지 높이를 확인했다.
- 이유:
  - 사용자 PIE 재검증에서 `WheelRadius`만 조정해도 파묻힘 정도가 직접적으로 바뀌었다.
  - 반대로 `WheelSync` 쪽 `Suspension Z` 보정 실험은 원인 해결보다 오판정을 만들었다.

---

### CF-DL-0063 — 바퀴 파묻힘 해결에 WheelSync Suspension Z 실험 경로를 채택하지 않는다
- 판정: 확정
- 결정:
  - 바퀴 파묻힘 해결을 위해 `UCFWheelSyncComp`에 실험성 `Suspension Z` 보정 로직을 운영 기본안으로 넣지 않는다.
  - 현재 운영 기본값은 `bApplySuspensionZInCpp = false`를 유지한다.
  - 이 이슈는 코드 보정보다 `VehicleData` 물리 수치 정렬로 먼저 해결한다.
- 이유:
  - 실제 테스트에서 해당 실험 경로는 바퀴를 덜 묻히게 하기보다 오히려 더 파묻히게 만들었다.
  - `WheelSync`는 공통 시각 동기화 코어로 유지해야 하므로, 특정 차량의 반경 불일치를 보정하는 임시 로직을 넣는 것은 장기적으로 위험하다.

---

### CF-DL-0064 — Pawn 디테일 카테고리 루트는 VehiclePawn으로 분리한다
- 판정: 확정
- 결정:
  - `ACFVehiclePawn` 본체의 UPROPERTY / UFUNCTION 카테고리 루트는 `CarFight|Vehicle` 대신 `CarFight|VehiclePawn`을 사용한다.
  - 목적은 에디터 디테일 패널에서 엔진 기본 `Vehicle` 계열 카테고리와의 시각적 중복을 줄이는 것이다.
- 이유:
  - 실제 프로젝트에서 `Vehicle` 카테고리가 중복으로 보여 읽기 어려운 문제가 있었다.
  - 기능 변경 없이 메타데이터 수준에서 분리하는 방식이 가장 안전한 기본안이었다.

---

### CF-DL-0065 — AutoFit 레거시 구조는 즉시 삭제하지 않고 카테고리 격리부터 한다
- 판정: 확정
- 결정:
  - `CFVehicleData` 내부의 `VehicleLayoutConfig / WheelLayout / AutoFit` 필드는 지금 단계에서 하드 삭제하지 않는다.
  - 대신 `CarFight|Legacy|AutoFit` 카테고리로 격리해 현재 운영 기준과 시각적으로 분리한다.
- 이유:
  - 현재 런타임은 이미 AutoFit을 사용하지 않으므로 기능상 급한 제거가 아니다.
  - `DA_PoliceCar` 등 기존 자산 직렬화 호환성을 보존하면서, 현재 작업자가 레거시 구조를 덜 오해하게 만드는 것이 우선이다.

---

### CF-DL-0066 — 디테일 패널 중복 해소는 Drive와 Components 메타데이터 분리로 닫는다
- 판정: 확정
- 결정:
  - `UCFVehicleDriveComp` 내부 카테고리 루트는 `CarFight|VehiclePawn|Drive` 대신 `CarFight|VehicleDrive`를 사용한다.
  - `ACFVehiclePawn`가 소유한 `VehicleDriveComp`, `WheelSyncComp` 참조 프로퍼티는 `CarFight|VehiclePawn` 대신 `CarFight|Components`로 분리한다.
  - 이번 이슈는 런타임 로직 수정이 아니라 블루프린트 자산 재저장 포함 메타데이터 정리로 마무리한다.
- 이유:
  - `CarFight|VehiclePawn` 루트만 분리한 뒤에도 실제 디테일 패널에서는 `Vehicle Pawn` 묶음이 두 벌로 남아 있었다.
  - 실측 덤프 기준으로 중복의 직접 원인은 Pawn 본체와 `VehicleDriveComp`/컴포넌트 참조가 같은 루트를 공유하던 구조였다.
  - 카테고리 루트만 더 세분화하면 기능 영향 없이 읽기성을 회복할 수 있고, 사용자 요청인 최소 위험 수정 원칙에도 맞는다.

---

## 후속 판단 기준
- 구조 전환 전까지는 유지 코어와 교체 구조를 섞어 수정하지 않는다.
- 새 UE 작업은 먼저 `01_Roadmap.md`의 단계 위치를 확인한 뒤 시작한다.
- 새 결정이 생기면 이 문서에 먼저 남기고 구현에 들어간다.
