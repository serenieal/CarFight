# CFNetSmooth 구현 계획서

- 문서 버전: v0.1.0
- 작성일: 2026-06-17
- 대상 프로젝트: CarFight
- 작업 유형: 신규
- 상태: Draft

---

## 1. 목적

이 문서는 `CFNetSmooth`를 플러그인으로 직접 구현하기 위한 단계별 작업 계획을 정의한다.

기본 원칙은 “작게 만들고, 독립 검증하고, 차량에 연결하기 전 멈춰서 판단한다”이다.

---

## 2. 구현 단계 요약

```text
v0.1: 테스트 Actor용 서버 권위 Transform 보간
v0.2: Teleport / Buffer Clear / Debug 강화
v0.3: 차량 Visual/Shell 연결 실험
v0.4: 성능/대역폭 튜닝과 축별 동기화 검토
```

---

## 3. v0.1 단계

### 3.1 목표

테스트 Actor에서 서버가 보낸 Transform State를 클라이언트가 부드럽게 표시할 수 있는지 확인한다.

### 3.2 작업 목록

- `CFNetSmooth` 플러그인 뼈대 생성
- `FCFNetSmoothState` 구조체 생성
- `UCFNetSmoothComp` 컴포넌트 생성
- 서버 State 송신 구현
- 클라이언트 State Buffer 구현
- 위치/회전 보간 구현
- 제한 외삽 구현
- Snap Threshold 구현
- 테스트 Actor 또는 기존 단순 Actor에 컴포넌트 부착

### 3.3 금지 작업

- `ACFVehiclePawn` 직접 수정
- `BP_CFVehiclePawn` 기본값 변경
- `RepMove` 또는 `AutoPhysics` 설정 변경
- Chaos Vehicle Movement 수정
- Network Physics Prediction 재시도

### 3.4 완료 기준

- Listen Server PIE 2인에서 테스트 Actor가 부드럽게 보인다.
- Dedicated Server 테스트에서 테스트 Actor가 정지하지 않는다.
- 네트워크 지연 100ms에서 심한 워프 없이 따라온다.
- Snap 발생 로그가 예상 조건에서만 출력된다.

---

## 4. v0.2 단계

### 4.1 목표

리스폰, 순간이동, 소유권 변경 같은 끊김 상황을 안전하게 처리한다.

### 4.2 작업 목록

- `MarkTeleport` 구현
- `ClearStateBuffer` 구현
- `ForceSendNextState` 구현
- Teleport State 수신 시 즉시 적용
- 오래된 Sequence 폐기
- Buffer 크기 제한
- 디버그 로그 포맷 고정

### 4.3 완료 기준

- 리스폰 위치가 보간 잔상 없이 즉시 반영된다.
- Buffer Clear 후 과거 State가 다시 적용되지 않는다.
- Teleport 직후 1초 안에 원격 표시가 정상 보간 상태로 복귀한다.

---

## 5. v0.3 단계

### 5.1 목표

실제 차량 Actor에 바로 붙이지 않고, 차량 Visual/Shell에만 적용 가능한지 검증한다.

### 5.2 작업 목록

- 차량 표시 대상 후보 정리
- Actor Transform과 Visual Transform 책임 분리
- 원격 SimulatedProxy에만 보간 적용 조건 추가
- 로컬 AutonomousProxy에는 적용하지 않는 보호 조건 추가
- WheelSync, 카메라, 차체 표시 안정화와 충돌 여부 확인

### 5.3 금지 작업

- 서버 권위 Actor 위치를 클라이언트 보간 결과로 덮어쓰기
- 로컬 조작 차량의 물리 Transform 보간
- 충돌 판정 기준 위치 변경

### 5.4 완료 기준

- Client2에서 Client1 차량의 표시만 부드러워진다.
- Client1 로컬 조작감은 바뀌지 않는다.
- 서버 충돌/리스폰 기준 위치는 기존과 동일하다.

---

## 6. v0.4 단계

### 6.1 목표

v0.3까지 통과한 경우에만 대역폭, 성능, 설정값을 정리한다.

### 6.2 작업 목록

- `SendRate` A/B 테스트
- `InterpBackTime` A/B 테스트
- `PosSnapDist` / `RotSnapDeg` 튜닝
- Unreliable RPC 손실 시 품질 확인
- 필요 시 위치/회전/속도 축별 동기화 검토
- 필요 시 NetQuantize 정밀도 조정

### 6.3 완료 기준

- 2인 Dedicated Server에서 원격 표시가 안정적이다.
- 지연/손실 환경에서도 스냅이 과도하게 발생하지 않는다.
- 로그로 평균 Buffer 크기와 Snap 횟수를 확인할 수 있다.

---

## 7. 작업 순서

권장 작업 순서는 다음이다.

```text
1. 플러그인 뼈대 생성
2. 빈 컴포넌트 빌드 확인
3. State 구조체 추가
4. 서버 송신 로그만 추가
5. 클라이언트 수신 로그만 추가
6. Buffer 삽입/정렬 구현
7. 보간 적용
8. 외삽 적용
9. Snap 적용
10. Teleport 적용
11. Dedicated Server 검증
12. 차량 Visual/Shell 연결 여부 재판정
```

---

## 8. 위험과 대응

| 위험 | 대응 |
|---|---|
| 기존 차량 기준선 훼손 | v0.1/v0.2에서 차량 파일 수정 금지 |
| 보간이 서버 판정을 덮음 | Actor Transform과 Visual Transform 분리 |
| Reliable RPC 큐 적체 | v0.1은 Unreliable 우선, Teleport만 별도 검토 |
| 과도한 외삽으로 미끄러짐 | `MaxExtrapTime` 제한과 정지 상태 감지 |
| 문서와 실제 코드 불일치 | 구현 후 Changelog와 API 문서 동시 갱신 |

---

## 9. Changelog

### v0.1.0

```text
- CFNetSmooth 단계별 구현 계획 최초 작성
- v0.1~v0.4 범위, 금지 작업, 완료 기준 정의
- 차량 적용 전 독립 테스트 원칙 명시
```

---

## 10. 마이그레이션 지침

- v0.1/v0.2는 기존 차량 코드로 마이그레이션하지 않는다.
- v0.3 진입 전 `VehicleNetSyncPlan`의 현재 기준선과 충돌 여부를 다시 확인한다.
- 실제 차량 적용은 별도 패치 문서와 롤백 지점을 만든 뒤 진행한다.
