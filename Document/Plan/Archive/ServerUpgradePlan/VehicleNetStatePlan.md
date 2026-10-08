# Vehicle NetState / Remote Interpolation Plan

- 문서 버전: v0.2.0
- 작성일: 2026-06-05
- 대상 프로젝트: CarFight
- 관련 영역: Dedicated Server 차량 이동 복제 / Chaos Vehicle / 원격 차량 보간
- 상태: Draft
- 선행 문서:
  - `VehicleNetPhysicsJitter.md`
  - `RT_20260605_VNetKey.md`

---

## 1. 목적

이 문서는 CarFight 차량 네트워크 물리 흔들림 문제를 해결하기 위한 중기 구조를 설계한다.

이번 문서는 코드 작업이 아니라 설계 문서다.

핵심 방향:

```text
UE Actor Movement Replication에만 의존하지 않는다.
서버 권위 차량 상태를 별도 NetState로 복제한다.
원격 차량은 서버 상태 버퍼를 기준으로 보간 표시한다.
소유 차량 예측/서버 보정은 원격 차량 보간 이후 단계로 미룬다.
```

---

## 2. 선행 진단 요약

### 2.1 bReplicateMovement=true

관찰:

```text
- 서버 Transform 보정은 들어온다.
- 하지만 조작 차량과 원격 차량에서 위치/속도 오차가 순간적으로 튄다.
- 원격 차량이 뚝뚝 끊겨 보일 수 있다.
```

해석:

```text
Actor Movement Replication이 Chaos Vehicle 물리 상태를 주기적으로 덮어쓰거나 보정하면서 시각적 끊김을 만든다.
```

### 2.2 bReplicateMovement=false

관찰:

```text
- 시각적 끊김은 줄어들 수 있다.
- 하지만 서버/클라 위치, 회전, 각속도 오차가 빠르게 누적된다.
- 장기 충돌 판정이 클라이언트마다 갈라질 수 있다.
```

해석:

```text
Actor Movement 자동 보정을 끄는 것은 흔들림 원인 분리에는 유효하지만, 서버 권위 상태 동기화가 사라지므로 최종 해법이 아니다.
```

---

## 3. 목표 네트워크 정책

### 3.1 공통 원칙

```text
- 서버가 차량 권위 상태를 가진다.
- 클라이언트는 시각적 표시를 부드럽게 만든다.
- 충돌/판정/최종 위치는 서버 기준으로 유지한다.
- Actor Movement 자동 복제는 최종적으로 차량 전용 복제와 역할이 겹치지 않게 줄인다.
```

### 3.2 단계별 목표

```text
1단계: 진단값 안정화
2단계: 원격 차량용 서버 NetState 복제
3단계: 원격 차량 보간 표시
4단계: 큰 오차 보정 정책 추가
5단계: 소유 차량 예측/서버 보정
```

현재 다음 작업은 1단계 마무리와 2~3단계 설계다.

---

## 4. 대상 역할 분리

### 4.1 Authority / Server

서버 책임:

```text
- 차량 물리 권위 상태 계산
- 차량 위치/회전/속도/각속도 샘플 생성
- NetState 복제
- 충돌/겹침/게임 판정 기준 유지
```

### 4.2 AutonomousProxy / 소유 클라이언트

소유 클라이언트 책임:

```text
- 입력 즉시 반응하는 조작감 유지
- 초기 단계에서는 기존 Chaos Vehicle 입력 흐름 유지
- 서버 권위 상태와의 오차는 진단만 수행
```

초기 중기 단계에서는 소유 차량 보정까지 들어가지 않는다.

이유:

```text
소유 차량 예측/보정은 입력 번호, 재시뮬레이션, 서버 보정 스무딩이 필요하므로 작업 범위가 크다.
원격 차량 보간보다 먼저 구현하면 원인 분리가 어려워진다.
```

### 4.3 SimulatedProxy / 원격 차량

원격 차량 책임:

```text
- 서버에서 복제된 NetState를 직접 순간 적용하지 않는다.
- NetState 샘플을 버퍼에 저장한다.
- 일정 시간 지연된 목표 시각을 기준으로 두 샘플 사이를 보간한다.
- 큰 오차는 별도 정책으로 빠른 보정 또는 제한적 스냅 처리한다.
```

초기 구현 대상은 원격 차량이다.

---

## 5. FCFVehicleNetState 초안

파일 후보:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
```

초기 구조 필드 후보:

```text
FCFVehicleNetState
- ServerSequenceId: 서버가 샘플링한 순번
- ServerTimeSeconds: GameState 서버 시간 기준 샘플 시각
- ServerLocation: 서버 권위 기준 차량 위치
- ServerRotation: 서버 권위 기준 차량 회전
- ServerLinearVelocity: 서버 권위 기준 차량 선형 속도, cm/s
- ServerAngularVelocityDeg: 서버 권위 기준 차량 각속도, deg/s
- ServerForwardSpeed: 차량 전방 벡터 기준 전진 속도, cm/s
```

주의:

```text
- 초안 단계에서는 입력값 Throttle/Brake/Steering을 넣지 않는다.
- 입력값은 소유 차량 예측/서버 보정 단계에서 필요하다.
- 원격 차량 보간만 목표라면 Transform + Velocity + AngularVelocity가 우선이다.
```

---

## 6. Replication 정책 초안

### 6.1 서버 샘플 주기

초기값 후보:

```text
VehicleNetStateSendRate = 15Hz
VehicleNetStateSampleInterval = 0.0667s
```

이유:

```text
- 차량은 Character보다 회전/속도 변화가 커서 너무 낮은 주기는 원격 보간 품질이 나쁘다.
- 30Hz는 품질은 좋지만 초기 단계에서는 네트워크 비용이 커질 수 있다.
- 15Hz + 100ms 보간 지연으로 먼저 품질을 확인한다.
```

### 6.2 복제 대상

```text
ReplicatedVehicleNetState
- ReplicatedUsing 방식으로 수신 이벤트를 받는다.
- 서버에서만 갱신한다.
- 클라이언트에서는 직접 수정하지 않는다.
```

OnRep 책임:

```text
- 수신한 NetState를 원격 보간 버퍼에 추가한다.
- 로컬 수신 시간 ReceivedLocalTimeSeconds를 함께 저장한다.
- 보간 버퍼 크기를 제한한다.
```

---

## 7. 원격 차량 보간 버퍼

클라이언트 로컬 전용 버퍼 항목 후보:

```text
FCFVehicleNetStateBufferItem
- State: 수신한 FCFVehicleNetState
- ReceivedLocalTimeSeconds: 클라이언트 로컬 수신 시간
```

버퍼 정책:

```text
- 최대 10~20개 샘플 유지
- 오래된 샘플 제거
- ServerSequenceId가 역행하면 무시
- 동일 SequenceId는 중복 삽입하지 않음
```

---

## 8. 원격 차량 보간 방식

### 8.1 목표 시각

```text
TargetServerTime = CurrentEstimatedServerTime - InterpDelay
```

초기값 후보:

```text
InterpDelay = 0.10s ~ 0.15s
```

의미:

```text
클라이언트는 가장 최신 서버 상태를 바로 표시하지 않고, 약간 과거의 서버 상태를 두 샘플 사이에서 보간해 표시한다.
이렇게 하면 패킷 도착 간격이 들쭉날쭉해도 원격 차량이 순간이동하듯 보이는 현상을 줄일 수 있다.
```

### 8.2 보간 대상

```text
Location: 선형 보간
Rotation: 쿼터니언 구면 보간
LinearVelocity: 선형 보간
AngularVelocity: 선형 보간
```

### 8.3 보간 적용 대상

초기 정책:

```text
GetLocalRole() == ROLE_SimulatedProxy 인 차량만 적용한다.
IsLocallyControlled() == true 차량에는 적용하지 않는다.
HasAuthority() == true 서버 차량에는 적용하지 않는다.
```

---

## 9. 큰 오차 처리 정책

초기 기준값 후보:

```text
SmallErrorDistance = 100cm
LargeErrorDistance = 500cm
TeleportErrorDistance = 1500cm
```

정책:

```text
- 100cm 이하: 일반 보간
- 100~500cm: 보간 속도 강화
- 500~1500cm: 빠른 보정 또는 짧은 시간 내 따라잡기
- 1500cm 이상: 제한적 스냅 고려
```

주의:

```text
첫 구현에서는 큰 오차 스냅을 바로 넣지 않는다.
먼저 보간 버퍼가 원격 차량 순간이동을 줄이는지 확인한다.
```

---

## 10. bReplicateMovement 정책

초기 실험 정책:

```text
- bReplicateMovement=true 상태에서도 NetState를 복제해 로그/버퍼를 만든다.
- 원격 보간 적용 실험 시에는 bReplicateMovement=false 조합도 테스트한다.
- 최종 정책은 런타임 비교 후 결정한다.
```

예상 최종 방향:

```text
- bReplicates=true 유지
- bReplicateMovement는 차량 전용 NetState 보간과 충돌하지 않도록 false 또는 최소화 검토
- 서버 권위 위치/충돌은 NetState와 서버 물리 기준으로 유지
```

단, 현재 단계에서 기본 정책을 확정하지 않는다.

---

## 11. 구현 단계 제안

### 11.1 CFVNetAgeSplit

목표:

```text
- 현재 SampleAge를 ServerTimeDelta / ReceivedAge로 분리
- OnRep 수신 로컬 시간 기록
- 로그 해석 안정화
```

성격:

```text
진단 보강 코드 작업
Codex 작업지시서 필요
```

### 11.2 CFVNetStateBase

목표:

```text
- FCFVehicleNetState 추가
- 서버에서 ReplicatedVehicleNetState 생성/복제
- OnRep에서 버퍼에 저장
- 아직 Transform 적용은 하지 않음
```

성격:

```text
기초 NetState 코드 작업
Codex 작업지시서 필요
```

### 11.3 CFVRemoteInterp

목표:

```text
- SimulatedProxy 원격 차량에만 보간 표시 적용
- 소유 차량과 서버 차량에는 적용하지 않음
- 디버그 로그로 보간 상태 확인
```

성격:

```text
원격 보간 코드 작업
Codex 작업지시서 필요
```

---

## 12. 이번 단계에서 하지 않을 것

```text
- 소유 차량 예측/서버 보정 구현 금지
- 입력 히스토리/재시뮬레이션 구현 금지
- 무기/발사/대미지 서버 권한 구조 작업 금지
- 차량 피팅/터렛/락온 시스템 작업 금지
- bReplicateMovement 최종 정책 확정 금지
```

---

## 13. CFVNetAgeSplit 이후 설계 반영

`RT_20260605_AgeSplit.md` 기준으로 다음 결정을 반영한다.

```text
- ReceivedAge는 네 개 테스트 추출 로그에서 음수 0건으로 확인되었다.
- ServerTimeDelta는 원격 차량 또는 RepMove=false 상태에서 몇 초 단위 음수가 발생할 수 있다.
- 따라서 원격 차량 보간 버퍼의 1차 시간축은 ServerTimeSeconds가 아니라 ReceivedLocalTimeSeconds로 둔다.
- ServerTimeSeconds는 서버 샘플 시각으로 저장하되, 보간 단독 기준으로 확정하지 않는다.
- CFVNetStateBase는 Transform 적용 없이 서버 NetState 복제와 수신 버퍼 저장만 수행한다.
```

다음 코드 작업:

```text
CFVNetStateBase
- FCFVehicleNetState 추가
- ReplicatedVehicleNetState 복제
- OnRep_VehicleNetState에서 ReceivedLocalTimeSeconds와 함께 버퍼 저장
- 아직 보간/보정/Transform 적용 금지
```

---

## 14. 변경 기록

### v0.1.0

```text
- 차량 전용 NetState 구조 초안 작성
- Authority / AutonomousProxy / SimulatedProxy 책임 분리
- 원격 차량 보간 버퍼와 보간 정책 초안 작성
- bReplicateMovement 정책을 실험 변수로 유지
- 다음 코드 작업 후보를 CFVNetAgeSplit / CFVNetStateBase / CFVRemoteInterp로 분리
```
