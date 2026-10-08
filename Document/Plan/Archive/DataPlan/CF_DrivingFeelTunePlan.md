# CarFight 차종별 주행감 튜닝 툴 계획서 v0.1.3

- 작성일: 2026-06-23
- 대상 프로젝트: `UE/CarFight_Re`
- 대상 툴: `Vehicle DA Wizard`
- 대상 데이터: `UCFVehicleData.VehicleMovementConfig`
- 문서 상태: Deferred / Not Current Priority
- 상위 계획서: `Document/Plan/DataPlan/CF_DAFillToolPlan.md`

---

## 1. 목적

이 문서는 새 차량 DA의 수치를 단순히 빠르게 입력하는 것이 아니라, **차종마다 다른 주행감**을 만들기 위한 튜닝 툴 방향을 고정한다.

핵심 목표는 아래와 같다.

- 세단, SUV, 스포츠카, 무거운 차량처럼 차종별 조작감을 빠르게 만들 수 있게 한다.
- 사용자가 원시 수치를 모두 외우지 않아도 “가속감”, “조향 민첩성”, “접지감”, “서스펜션 단단함” 같은 체감 언어로 조절하게 한다.
- 슬라이더 변경값은 즉시 DA에 쓰지 않고, 변경 예정값을 확인한 뒤 버튼으로 적용한다.
- 적용 후 Validator 재검사와 PIE 테스트 기준으로 결과를 확인한다.

---

## 2. 범위

### 2.1 1차 구현 범위

`Vehicle DA Wizard`에 `Driving Feel Quick Tune` 섹션을 추가한다.

1차 슬라이더:

- `가속감`
- `조향 민첩성`
- `접지감`
- `서스펜션 단단함`

1차 버튼:

- `세단 프리셋`
- `SUV 프리셋`
- `스포츠 프리셋`
- `무거운 차량 프리셋`
- `DA에 주행감 적용`

1차 적용 대상:

- `EngineMaxTorque`
- `EngineMaxRPM`
- `FrontWheelMaxSteerAngle`
- `SteeringAngleRatio`
- `FrontWheelFrictionForceMultiplier`
- `RearWheelFrictionForceMultiplier`
- `FrontWheelCorneringStiffness`
- `RearWheelCorneringStiffness`
- `FrontWheelSpringRate`
- `RearWheelSpringRate`
- `FrontWheelSpringPreload`
- `RearWheelSpringPreload`

### 2.2 보류 범위

아래는 1차에서 보류한다.

- 실시간 PIE 중 물리값 핫리로드
- 모든 Movement 필드 슬라이더화
- `CenterOfMassOverride` 자동 조정
- `DifferentialType`, `SteeringType` 자동 변경
- DriveState 자동 튜닝
- AI 기반 최적값 추천

---

## 3. 핵심 결정

### 3.1 원시 수치보다 체감 슬라이더를 우선한다

사용자가 기대하는 것은 “숫자를 빨리 넣는 것”보다 “차종마다 다르게 느껴지는 조작감”이다.

따라서 기본 화면은 원시 필드명 중심이 아니라 아래 체감 축 중심으로 구성한다.

| 체감 축 | 사용자가 기대하는 변화 | 내부 변경 대상 |
|---|---|---|
| 가속감 | 출발과 직선 가속이 강해진다 | 토크, RPM |
| 조향 민첩성 | 핸들 반응과 회전성이 커진다 | 조향각, 조향 비율 |
| 접지감 | 미끄러짐이 줄고 코너 유지력이 커진다 | 마찰 배수, 코너링 강성 |
| 서스펜션 단단함 | 출렁임이 줄고 반응이 단단해진다 | 스프링 강성, 프리로드 |

### 3.2 슬라이더 값은 임시값으로 유지한다

슬라이더를 움직이는 즉시 DA 원본에 쓰지 않는다.

안전한 흐름:

```text
Target DA 현재값
-> Wizard 임시 슬라이더 값
-> 변경 예정 수치 표시
-> DA에 적용 버튼
-> FScopedTransaction
-> MarkPackageDirty
-> Validator 재검사
```

### 3.3 위험값은 고급 단계로 분리한다

`CenterOfMassOverride`, `DifferentialType`, `FrontRearSplit`, `DriveStateConfig`는 차량 거동을 크게 흔들 수 있다.

1차에서는 자동 적용하지 않고, 검증 리포트와 수동 조정 대상으로 남긴다.

---

## 4. 1차 수치 매핑

슬라이더 값은 `0.0 ~ 1.0` 범위다.

### 4.1 가속감

| 슬라이더 | 필드 | 낮음 | 높음 |
|---|---|---:|---:|
| 가속감 | `EngineMaxTorque` | 450 | 1200 |
| 가속감 | `EngineMaxRPM` | 4500 | 8500 |

기대 효과:
- 낮으면 무겁고 둔한 차량처럼 느껴진다.
- 높으면 스포츠카처럼 출발과 직선 가속이 강해진다.

검증:
- 직선 5초 가속
- 출발 시 좌우 흔들림
- 휠스핀 과다 여부

### 4.2 조향 민첩성

| 슬라이더 | 필드 | 낮음 | 높음 |
|---|---|---:|---:|
| 조향 민첩성 | `FrontWheelMaxSteerAngle` | 26 | 48 |
| 조향 민첩성 | `SteeringAngleRatio` | 0.45 | 0.90 |

기대 효과:
- 낮으면 SUV나 무거운 차량처럼 둔하게 돈다.
- 높으면 스포츠카처럼 빠르게 방향을 바꾼다.

검증:
- 저속 U턴
- 중속 슬라럼
- 고속 조향 불안정 여부

### 4.3 접지감

| 슬라이더 | 필드 | 낮음 | 높음 |
|---|---|---:|---:|
| 접지감 | `FrontWheelFrictionForceMultiplier` | 1.2 | 3.2 |
| 접지감 | `RearWheelFrictionForceMultiplier` | 1.2 | 3.2 |
| 접지감 | `FrontWheelCorneringStiffness` | 700 | 1500 |
| 접지감 | `RearWheelCorneringStiffness` | 700 | 1500 |

기대 효과:
- 낮으면 더 쉽게 미끄러진다.
- 높으면 코너에서 더 잘 붙지만, 과하면 비현실적으로 붙거나 튐이 생길 수 있다.

검증:
- 원형 회전
- 중속 슬라럼
- 코너 탈출 시 미끄러짐

### 4.4 서스펜션 단단함

| 슬라이더 | 필드 | 낮음 | 높음 |
|---|---|---:|---:|
| 서스펜션 단단함 | `FrontWheelSpringRate` | 160 | 500 |
| 서스펜션 단단함 | `RearWheelSpringRate` | 160 | 500 |
| 서스펜션 단단함 | `FrontWheelSpringPreload` | 35 | 90 |
| 서스펜션 단단함 | `RearWheelSpringPreload` | 35 | 90 |

기대 효과:
- 낮으면 부드럽고 출렁인다.
- 높으면 단단하고 반응이 빠르지만, 과하면 튀거나 접지가 불안정할 수 있다.

검증:
- 정지 차고
- 작은 턱 통과
- 급브레이크 시 차체 쏠림

---

## 5. 차종 프리셋 초안

| 프리셋 | 가속감 | 조향 민첩성 | 접지감 | 서스펜션 단단함 | 의도 |
|---|---:|---:|---:|---:|---|
| 세단 | 0.50 | 0.50 | 0.55 | 0.45 | 기준형, 무난한 조작감 |
| SUV | 0.38 | 0.34 | 0.50 | 0.38 | 둔하지만 안정적인 큰 차 |
| 스포츠 | 0.85 | 0.78 | 0.82 | 0.78 | 빠르고 민첩한 차 |
| 무거운 차량 | 0.28 | 0.25 | 0.45 | 0.55 | 느리고 묵직한 차 |

이 값은 최종 밸런스가 아니라 툴 검증용 시작점이다.

---

## 6. UX 원칙

- 슬라이더 옆에 현재 슬라이더 값과 적용 예정 DA 수치를 같이 보여준다.
- `DA에 적용`은 명확한 변경 버튼으로 표시한다.
- 적용 후 자동으로 전체 검사를 실행한다.
- 원본 DA 수정은 `FScopedTransaction`으로 Undo 가능하게 한다.
- Source DA가 있으면 비교 리포트에 차이를 남긴다.
- 위험 필드는 기본 Quick Tune에서 숨기고 별도 고급 단계로 분리한다.

---

## 7. 검증 기준

### 7.1 툴 검증

- [ ] Target DA가 없으면 적용 버튼이 비활성화된다.
- [ ] 프리셋 버튼을 누르면 4개 슬라이더 값이 바뀐다.
- [ ] 적용 전에는 DA 원본이 바뀌지 않는다.
- [ ] `DA에 주행감 적용` 후 `bUseMovementOverrides`가 켜진다.
- [ ] 적용 후 Validator 리포트가 자동 갱신된다.
- [ ] Undo로 적용 전 상태를 되돌릴 수 있다.

### 7.2 차량 검증

- [ ] 세단 프리셋은 기준형 조작감으로 움직인다.
- [ ] SUV 프리셋은 조향이 둔하고 출렁임이 더 크다.
- [ ] 스포츠 프리셋은 가속과 조향이 빠르다.
- [ ] 무거운 차량 프리셋은 가속과 조향이 둔하다.
- [ ] 어떤 프리셋도 스폰 직후 즉시 뒤집히지 않는다.

---

## 8. 미결 사항

- 실제 프로젝트 기준 차량은 `DA_TestSedan`으로 확정됐고, 기본 주행 검증도 완료됐다. `DA_TestSUV`는 수동 전환 대조 테스트 후보로 유지한다.
- 주행감은 현재 충분히 나눠진 상태로 보고, 가까운 핵심 작업에서 제외한다.
- 슬라이더 범위 정밀화는 하드포인트 / Fire / UI 상태 계약 연결 이후에 다시 본다.
- 고급 물리값을 언제 Quick Tune에 노출할지 결정 필요.
- 테스트 기록을 `.md` 파일로 저장할지, 별도 DataAsset으로 저장할지 결정 필요.

---

## 9. Changelog

### v0.1.3 - 2026-06-25

- 주행감은 현재 충분히 나눠진 상태로 보고 계획 상태를 Deferred로 낮췄다.
- Quick Tune 정밀화는 하드포인트 / Fire / UI 상태 계약 연결 이후로 미뤘다.

### v0.1.2 - 2026-06-25

- `DA_TestSedan` 기준 기본 주행 검증 완료 상태를 반영했다.
- `DA_TestSUV` 대조 테스트 방식은 수동 VehicleData 전환으로 결정했다.

### v0.1.1 - 2026-06-25

- `DA_PoliceCar` 폐기 결정에 맞춰 주행감 튜닝 기준 차량을 `DA_TestSedan`으로 확정했다.
- `DA_TestSUV`는 대조 테스트 후보로 유지했다.

### v0.1.0 - 2026-06-23

- 차종별 주행감 튜닝 툴 방향 최초 작성
- 원시 수치 편집보다 체감 슬라이더 우선 원칙 정의
- 4개 Quick Tune 슬라이더와 4개 차종 프리셋 정의
- 1차 DA 필드 매핑과 검증 기준 정의
