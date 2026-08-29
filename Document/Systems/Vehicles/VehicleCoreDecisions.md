# CarFight — 16_CoreDecisionLog

> 역할: CarFight 현재 기준 차량 코어의 **유지 결정 / 교체 결정 / 임시 운영 판단**을 기록한다.
> 상위 방향 문서: `00_Vision.md`
> 문서 버전: v1.4.2
> 마지막 정리(Asia/Seoul): 2026-08-28


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

### CF-DL-0056 — AutoFit은 런타임/데이터 기준에서 제거한다
- 판정: 확정
- 결정:
  - `AutoFit`은 현재 기준 런타임에서 사용하지 않으며, `CFVehicleData`에서도 제거한다.
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

### CF-DL-0065 — AutoFit 레거시 구조 정리는 하드 삭제로 닫는다
- 판정: 확정
- 결정:
  - `CFVehicleData` 내부의 `VehicleLayoutConfig / WheelLayout / AutoFit` 필드는 하드 삭제로 정리한다.
  - 현재 운영 기준은 `Wheel_Anchor_*` 수동 배치와 `WheelRadius` 조정만 남긴다.
- 이유:
  - 현재 런타임은 이미 AutoFit을 사용하지 않으므로 기능상 보존 가치가 없다.
  - 활성 SSOT와 실제 코드 기준선을 다시 일치시키는 편이 이후 작업 혼선을 줄인다.

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
- 새 UE 작업은 먼저 `02_Roadmap.md`의 단계 위치를 확인한 뒤 시작한다.
- 새 결정이 생기면 이 문서에 먼저 남기고 구현에 들어간다.

---

### CF-DL-0067 — 차량 카메라 코어는 독립 CameraComp로 유지한다
- 판정: 확정
- 결정:
  - 현재 차량 카메라 기준선은 `UCFVehicleCameraComp`를 독립 코어로 두는 방향으로 유지한다.
  - 카메라 계산 책임은 Pawn 본문이나 BP 그래프에 흩뿌리지 않는다.
- 이유:
  - 카메라 입력 누적, 제한각, FOV, Aim Trace, 충돌 처리를 한곳에 모아야 이후 무기 연동과 튜닝이 쉬워진다.

---

### CF-DL-0068 — BP_CFVehiclePawn의 카메라 계층도 Thin BP 원칙으로 유지한다
- 판정: 확정
- 결정:
  - `BP_CFVehiclePawn`의 카메라 관련 책임은 `CameraPivotRoot / CameraAimPivot / CameraBoom / FollowCamera` 조립과 배치 수준으로 유지한다.
  - 카메라 상태 판단과 회전 계산은 BP에 키우지 않는다.
- 이유:
  - 기존 Pawn 운영 원칙과 같은 방식으로 카메라 쪽도 Thin BP를 유지해야 구조 전환과 디버깅 비용이 낮다.

---

### CF-DL-0069 — 카메라 입력 자산은 BP 지정값 우선, 코드 fallback 후순위로 본다
- 판정: 확정
- 결정:
  - `DefaultInputMappingContext`와 `InputAction_Look` 등 입력 자산은 BP/파생 클래스 지정값을 우선한다.
  - 코드 경로 하드코딩은 비어 있을 때만 채우는 fallback로 제한한다.
- 이유:
  - 현재 프로젝트에서 입력 자산 이름과 구조는 계속 바뀔 수 있으므로, 코드가 자산명에 과도하게 고정되는 것을 피해야 한다.

---

### CF-DL-0070 — 현재 Look 입력 기준은 Axis2D로 고정한다
- 판정: 확정
- 결정:
  - 현재 Look 입력 기준선은 `IA_LookAround`의 `Axis2D` 입력으로 본다.
  - 마우스 / 게임패드 모두 최종적으로 `FVector2D` Look 값이 CameraComp에 들어오는 구조를 기준으로 유지한다.
- 이유:
  - 현재 CameraComp 구현은 Yaw / Pitch를 2D 벡터 기반으로 해석하며, 실제 동작 확인도 이 기준에서 PASS가 났다.

---

### CF-DL-0071 — 현재 카메라 작업은 기본 기준선 확보 단계로 본다
- 판정: 확정
- 결정:
  - 현재 카메라 작업은 최종 카메라/HUD 확장 완료 단계가 아니라, 기본 카메라 시스템 기준선 확보 단계로 본다.
  - HUD / 조준점 / 무기별 Aim Profile 실제 연동 / Reverse / Airborne / Destroyed 확장은 후속 작업으로 분리한다.
- 이유:
  - 현재 프로젝트 상태에서는 기본 입력, 회전, 피벗, FOV, Aim Trace를 먼저 안정화하는 것이 전체 작업 순서상 안전하다.

---

### CF-DL-0072 — WheelRadius 기준 휠 메시 크기 보정은 VehicleData 옵션으로 둔다
- 판정: **Superseded / Legacy Compatibility** — 신규 정상 차량 제작 정책은 CF-DL-0076이 우선한다. 기존 `bAutoScaleWheelMeshToRadius` 코드/자산 호환은 migration 전까지 유지할 수 있다.
- 결정:
  - 휠 StaticMesh 표시 크기를 `FrontWheelRadius` / `RearWheelRadius`에 맞추는 기능은 `WheelSync` Tick 보정이 아니라 `VehicleData.WheelVisualConfig`의 명시 옵션으로 둔다.
  - 기본값은 비활성화하고, DA에서 `bAutoScaleWheelMeshToRadius`를 켠 차량에만 `Wheel_Mesh_*` Uniform Scale을 적용한다.
  - 측정 축이 다른 에셋을 위해 `AutoMaxXZ`, `AxisX`, `AxisY`, `AxisZ` 측정 모드를 제공한다.
  - 메시 피벗이 실제 휠 중심이 아닐 수 있으므로, 자동 스케일 사용 시 StaticMesh 바운드 중심을 `Wheel_Mesh_*` 원점에 맞추는 중심 보정을 기본으로 둔다.
- 이유:
  - 기존 BP 수동 스케일과 기존 차량 외형을 말 없이 바꾸면 회귀 위험이 크다.
  - 휠 반지름 불일치는 공통 WheelSync 회전/서스펜션 로직보다 VehicleData 해석 단계에서 정렬하는 편이 안전하다.
  - 자동 스케일을 WheelSync 기준 캡처 전에 적용하면 이후 스핀/조향/서스펜션 Tick과 책임이 섞이지 않는다.
        - `WheelRadius`가 물리와 시각에 모두 반영되어도 StaticMesh 바운드 중심이 원점에서 어긋나면 바퀴가 반지름 변경량만큼 계속 파묻혀 보일 수 있다.
- 대체 관계:
  - 이 결정의 구현은 기존 차량 호환용 Legacy path로만 남길 수 있다.
  - 신규 정상 제작에서는 Physics WheelRadius가 Visual Scale을 결정하지 않는다.
  - `bUseWheelSocketScale`와 `bAutoScaleWheelMeshToRadius`의 동시 사용/배율 곱셈은 금지한다.
  - 상세 successor는 CF-DL-0076 및 `Document/Plan/WheelSizeAuthorityPlan.md`다.

---

### CF-DL-0073 — 차량 무기 피격 형상은 시각 차체를 기준으로 분리한다
- 판정: 확정
- 결정:
  - 차량 주행 물리를 담당하는 `VehicleMesh` Physics Asset을 최종 무기 피격 형상으로 사용하지 않는다.
  - `VehicleMesh`는 Chaos Vehicle 물리, 지면/벽/차량 물리 충돌을 담당한다.
  - 화면에 보이는 `SM_Body` StaticMesh는 Query 전용 차체 무기 피격 표면을 담당한다.
  - HitScan과 Projectile은 무기 피격 Query에서 `VehicleMesh`를 무시하고 `SM_Body` 또는 승인된 시각 피격 컴포넌트에서 명중해야 한다.
  - P0에서는 휠과 터렛 시각 메시의 독립 피해를 제외하고 차체 `SM_Body`만 피격 대상으로 본다.
- 이유:
  - 현재 자산 덤프 기준 `VehicleMesh`는 `QueryAndPhysics / Vehicle / Visibility Block`이고 `SM_Body`는 `NoCollision / Visibility Ignore`다.
  - 현재 Dummy HitScan은 `ECC_Visibility`를 사용하고 Projectile Collision은 모든 채널을 Block하므로 실제 피격은 Physics Asset을 기준으로 발생한다.
  - 보이는 차체와 실제 피격 외곽이 다르면 보이지 않는 공간에 맞거나 보이는 표면을 탄이 통과하는 문제가 생긴다.
  - 향후 장갑 패널과 모듈별 피해를 확장하려면 시각 파츠를 기준으로 피격 책임을 분리하는 편이 안전하다.
- 구현 전 미확정:
  - HitScan Trace Channel, Projectile Object Channel, 시각 차체 Collision Profile의 최종 이름과 슬롯은 별도 구현 설계에서 확정한다.
- 관련 문서:
  - `Document/Plan/Archive/HitDamage/ImplementationDesign.md`
  - `Document/Systems/Combat/DamageHitContext.md`

---

### CF-DL-0074 — Reticle 월드 목표점을 터렛과 실제 발사의 단일 조준 기준으로 사용한다
- 판정: 확정
- 결정:
  - 화면 Reticle이 지시하는 월드 위치를 `DesiredAimTargetLocation` 성격의 단일 조준 기준으로 사용한다.
  - 카메라 Aim Trace, 터렛 요구 방향, Muzzle 요구 발사 방향, HitScan 및 Projectile 초기 방향은 같은 Aim Solution에서 파생한다.
  - 중력 없는 직선 무기의 요구 방향은 `MuzzleWorldLocation → DesiredAimTargetLocation`으로 계산한다.
  - 현재 Muzzle 방향과 요구 방향의 정렬 오차가 허용 범위 안에 들어온 뒤 실제 발사를 허용한다.
  - 카메라는 목표를 볼 수 있지만 총구 앞이 막힌 상황을 별도 Muzzle Trace로 검사한다.
  - `OutOfArc`와 `TurretAligning`은 분리한다.
- 이유:
  - 현재 카메라, 차량 중심, 터렛 피벗, Muzzle Socket이 서로 다른 방향 기준을 사용해 Reticle과 탄착이 일치하지 않을 수 있다.
  - 터렛 회전 성능을 전투 요소로 유지하면서도 실제 발사 결과를 플레이어 조준 의도와 일치시키려면 단일 Aim Solution이 필요하다.
  - 정렬되지 않은 총신에서 탄환만 목표점으로 꺾어 발사하면 시각과 판정이 다시 분리된다.
- 후속:
  - 중력 Projectile의 탄도 발사 해와 이동 표적 선행 조준은 별도 확장으로 둔다.
- 관련 문서:
  - `Document/Plan/Archive/AimFireAlignment/ImplementationDesign.md`
  - `Document/Systems/Vehicles/VehicleAim.md`
  - `Document/Systems/UI/AimReticle.md`

---

### CF-DL-0075 — 고속 Projectile은 연속 구간 충돌을 보장한다
- 판정: 확정
- 결정:
  - 실제 Projectile 판정은 `ProjectileMovementComponent`의 Sweep 이동을 명시적으로 사용한다.
  - 고속, 중력, 유도 Projectile은 Sub-stepping을 사용하고 시간 간격과 반복 횟수를 탄종별 데이터로 조정할 수 있게 한다.
  - Sweep/Sub-stepping 후에도 터널링이 남으면 이전 위치부터 현재 위치까지 `CollisionRadius` 기반 보조 Sphere Sweep을 사용한다.
  - `OnComponentHit`과 보조 Sweep은 단일 Impact 처리 함수와 Activation별 1회 처리 플래그를 공유한다.
  - CCD는 보조 안전장치이며 단독 해결책으로 보지 않는다.
  - 비행 시간을 플레이어가 인지하기 어려운 초고속 탄종은 HitScan 판정과 Tracer 시각 표현을 분리할 수 있다.
- 이유:
  - 사용자 PIE에서 고속 Projectile이 프레임 사이의 충돌체를 통과하는 터널링이 확인됐다.
  - 실제 피해 처리 전에 HitContext가 속도와 프레임률에 관계없이 신뢰 가능해야 한다.
  - 중복 Impact 처리를 막지 않으면 한 발이 피해를 두 번 적용할 수 있다.
- 관련 문서:
  - `Document/Plan/Archive/ProjectileContinuousCollision/ImplementationDesign.md`
  - `Document/Systems/Combat/Projectile.md`
  - `Document/Systems/Combat/DamageHitContext.md`

---

### CF-DL-0076 — 타이어 크기 Authority는 USER Wheel Socket Scale로 둔다
- 판정: 확정 / Design Locked / Implementation Pending
- 결정:
  - 차량별 타이어의 적절한 크기는 AI/코드가 자동 판단하지 않고 USER가 차체 StaticMesh의 `Wheel_Anchor_FL/FR/RL/RR` Socket을 휠하우스에 맞춰 직접 배치·스케일해 결정한다.
  - Wheel StaticMesh Bounds는 원본 메시의 실제 기하학적 치수를 읽는 Source일 뿐, 휠하우스에 적절한 Scale을 자동 선택하는 디자인 Authority가 아니다.
  - `SocketScaleFromChassis`에 참여하는 Wheel StaticMesh는 차축 Y 기준으로 X/Z=직경, Y=폭을 사용하고 Bounds Size `100×25×100cm`를 canonical source dimension으로 둔다.
  - Socket Scale은 X/Z=직경 배율, Y=폭 배율로 해석하며 일반 타이어는 X와 Z가 동일해야 한다.
  - USER Socket Scale과 resolved Wheel Mesh Bounds에서 최종 Visual Wheel Scale과 Chaos `Front/Rear WheelRadius/Width`를 같은 source로 파생한다.
  - Socket mode에서 파생된 Radius/Width는 VehicleData에 명시적으로 저장되는 Runtime 최종값이며 AI Reference Physics Proposal의 독립 덮어쓰기 대상이 아니다.
  - 실차 Tire/Wheel Reference 값은 USER Socket 작성과 결과 sanity comparison에 사용하고 USER의 시각적 결정을 자동 보정하지 않는다.
  - Chassis Socket은 authoring source이며 runtime permanent parent가 아니다. 기존 `Wheel_Anchor_* → Wheel_Mesh_* → UCFWheelSyncComp` transform ownership을 유지하고 Socket Scale은 Wheel_Mesh 시각 크기에 적용한다.
  - 동일 타이어 디자인은 공용 Wheel StaticMesh를 재사용하며 P0에서는 `WheelMeshFL`을 기본 공용 메시로 사용하고 비어 있는 FR/RL/RR은 FL fallback을 허용하는 방향으로 구현한다.
  - 기존 `bAutoScaleWheelMeshToRadius`는 CF-DL-0072 Legacy compatibility로만 남기며 신규 Socket mode와 동시에 적용하지 않는다.
- 이유:
  - 타이어가 휠하우스에 어울리는지는 기하학 수치만으로 결정할 수 없는 시각 디자인 판단이며 USER가 직접 보는 것이 더 신뢰 가능하다.
  - 시각 타이어와 물리 Wheel 크기를 별도 입력으로 유지하면 타이어가 지면에 파묻히거나 떠 보이는 불일치가 생길 수 있다.
  - 공용 Wheel Mesh + USER Socket Scale 구조는 차종별 크기 차이 때문에 Wheel Mesh를 복제하는 반복 작업을 제거하면서도 자동 시각 판단을 요구하지 않는다.
  - VehicleData에 derived 물리값을 저장하면 기존 Resolver/Diff/Validation/Apply/Undo와 Runtime authority를 유지할 수 있다.
- 현재 evidence:
  - shared `/Game/CarFight/Vehicles/Shared/Tire/Wheel_FL` live Bounds는 `99.9990×24.9992×99.9990cm`, center≈0으로 canonical `100×25×100` PASS다.
  - Wheel Size Authority 재감사에서 전체 ChassisLayoutFingerprint 대신 narrow WheelSizeSourceFingerprint, Step4 RelativeScale equality, enum append-only, Legacy AutoScale/AutoCenter/Clamp 비사용을 확정했다.
- 관련 문서:
  - `Document/Plan/WheelSizeAuthorityPlan.md v0.1.4`
  - `Document/Plan/VehicleBuilderPlan.md v0.1.23`
  - `Document/Plan/VehicleBuilderRoadmap.md v0.1.23`

---

## 변경 이력
- v1.4.2 (2026-08-28)
  - CF-DL-0076 final Source audit에서 Socket mode field source를 ProjectDefault+Recipe explicit로 한정하고, Legacy WheelVisual validation과 AI private Profile wheel geometry가 새 Authority에 침범하지 않도록 설계 guard를 추가했다.
  - 상세 owner를 WheelSizeAuthorityPlan v0.1.4 / VehicleBuilderPlan-Roadmap v0.1.23으로 갱신했다.
- v1.4.1 (2026-08-28)
  - CF-DL-0076 canonical Wheel source dimension을 `100×25×100cm`로 확정하고 actual shared Wheel_FL live Bounds/center PASS evidence를 반영했다.
  - Wheel Size stale fingerprint 범위와 Legacy visual option 분리를 재감사 설계에 맞게 명확히 했다. Runtime 구현은 여전히 Pending이다.
- v1.4.0 (2026-08-28)
  - CF-DL-0072의 WheelRadius→Visual AutoScale 정책을 신규 정상 제작 기준에서는 Superseded / Legacy Compatibility로 재분류했다.
  - CF-DL-0076으로 USER Wheel Socket Scale을 타이어 크기 Authority로 고정하고, 공용 Wheel Bounds + Socket Scale에서 Visual/Physics 크기를 함께 파생하는 설계를 확정했다.
  - 실제 Runtime 구현은 아직 변경하지 않았으며 상세 구현 Gate는 `WheelSizeAuthorityPlan.md`가 소유한다.
- v1.3.0 (2026-07-13)
  - Reticle 월드 목표점을 터렛/Muzzle/실제 발사의 단일 조준 기준으로 사용하는 결정을 CF-DL-0074로 추가했다.
  - 고속 Projectile의 Sweep/Sub-stepping/보조 Sphere Sweep과 단일 Impact 처리 결정을 CF-DL-0075로 추가했다.
- v1.2.0 (2026-07-13)
  - 차량 무기 피격 형상을 VehicleMesh Physics Asset이 아니라 SM_Body 시각 차체 기준으로 분리하는 결정을 CF-DL-0073으로 추가했다.
  - Collision Channel/Profile의 구체 이름과 슬롯은 구현 설계에서 확정하는 미확정 항목으로 분리했다.
- v1.1.1 (2026-07-08)
  - WheelRadius 기준 휠 메시 자동 스케일과 메시 바운드 중심 보정 정책을 CF-DL-0072로 추가했다.
- v1.1.0 (2026-04-15)
  - 문서 버전 / 마지막 정리 날짜를 갱신했다.
  - 차량 카메라 코어, Thin BP 카메라 계층, 입력 자산 우선순위, Axis2D 입력 기준, 카메라 기준선 단계 판단을 결정 로그에 추가했다.
---

## 변경 이력
### 2026-06-19 - 링크 경로 정정
- 이전 ProjectSSOT Plan/Systems 참조를 현재 Document/Plan 및 Document/Systems 경로로 정정했다.
- 구버전 ProjectSSOT 파일명 참조를 현재 `00_Vision` ~ `05_TestChecklist` 기준 또는 Archive 경로로 정정했다.
