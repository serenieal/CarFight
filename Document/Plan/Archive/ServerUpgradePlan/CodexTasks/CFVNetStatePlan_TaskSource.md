# CFVNetStatePlan Codex 작업지시서

- 문서 버전: v0.1.0
- 작성일: 2026-06-05
- 대상 프로젝트: CarFight
- 대상 UE 프로젝트: UE/CarFight_Re.uproject
- 기준 폴더: Document/Plan/ServerUpgradePlan/
- 작업 유형: 차량 전용 NetState / 원격 차량 보간 설계 준비
- 상태: ReadyForCodex

---

## 1. 작업 목적

차량 네트워크 물리 흔들림 문제를 해결하기 위한 다음 단계 설계를 준비한다.

이번 작업은 원격 차량 보간이나 소유 차량 예측을 실제 구현하는 단계가 아니다. 다음 구현 단계로 넘어가기 전에 차량 전용 서버 권위 NetState 구조와 SimulatedProxy 원격 차량 보간 정책을 명확히 정의하고, 현재 진단 로그의 시간값을 보강한다.

---

## 2. 근거 요약

`RuntimeTest_20260605_NetPhysics.md` 기준 결론:

```text
1. bReplicateMovement=true는 서버 보정이 들어오지만 위치/속도 오차가 순간적으로 발생한다.
2. bReplicateMovement=false는 시각적 끊김을 줄일 수 있지만 서버/클라 위치, 회전, 속도 상태가 빠르게 갈라진다.
3. UE 기본 Actor Movement Replication만으로 CarFight 차량 네트워크 물리를 안정화하기 어렵다.
4. 다음 구조는 차량 전용 서버 권위 NetState 복제 + 원격 차량 보간이다.
5. 소유 차량 예측/서버 보정은 아직 구현하지 않고, 먼저 SimulatedProxy 원격 차량 보간부터 분리한다.
```

남은 진단 문제:

```text
현재 SampleAge는 GameState->GetServerWorldTimeSeconds() 기준으로 개선됐지만 일부 클라이언트/일부 Pawn에서 여전히 음수가 발생한다.
따라서 SampleAge를 ServerTimeDelta와 ReceivedAge로 분리해야 한다.
```

---

## 3. scope.in

- `FCFVehicleNetState` 구조 설계안을 문서화한다.
- `SimulatedProxy` 원격 차량 보간 정책을 문서화한다.
- `AutonomousProxy` 소유 차량 예측/서버 보정은 후속 단계로 분리한다.
- 현재 `VehicleNetDebug` 진단 로그의 시간값을 보강한다.
- `SampleAge`를 아래 두 값으로 분리한다.
  - `ServerTimeDelta`: 클라이언트 추정 서버 시간 - 서버 샘플 시간
  - `ReceivedAge`: OnRep 수신 이후 클라이언트 로컬 시간 기준 경과 시간
- OnRep에서 최신 서버 샘플을 받은 클라이언트 로컬 시간을 기록한다.
- 로그에 `ServerTimeDelta`, `ReceivedAge`, `LastReceivedSeq`를 추가한다.
- 문서에 bReplicateMovement 정책 전환 조건을 정리한다.
- 코드 변경이 필요하다면 진단 필드 추가까지만 수행한다.

---

## 4. scope.out

- 원격 차량 보간 실제 구현 금지
- 소유 차량 예측 실제 구현 금지
- 서버 보정 / Reconciliation 실제 구현 금지
- 차량 Transform을 직접 움직이는 보정 코드 추가 금지
- `bReplicateMovement` 기본값 강제 변경 금지
- 무기/발사/대미지 서버 권한 구조 작업 금지
- BP_CFVehiclePawn 에셋 자동 수정 금지

---

## 5. constraints

- 모든 신규 변수/함수 위에 한 줄 한국어 주석 작성
- 모든 UPROPERTY/UFUNCTION에는 한국어 `DisplayName`과 `ToolTip` 작성
- 파일명 32자 이하 유지
- 기존 `VehicleNetDebug:` 로그 접두어 유지
- 기존 핵심 로그 필드 제거 금지
- `SetActorLocation`, `SetActorRotation`, `TeleportTo` 추가 금지
- 버전은 `ACFVehiclePawn` 기준 v2.24.0으로 표기
- Changelog와 Migration 메모 작성

---

## 6. 대상 파일

### 코드 수정 후보

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

### 문서 신규/갱신

```text
Document/Plan/ServerUpgradePlan/VehicleNetStateDesign.md
Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetStatePlan_Review.md
Document/Plan/ServerUpgradePlan/VehicleNetPhysicsJitter.md
```

---

## 7. 설계 문서 요구사항

`VehicleNetStateDesign.md`를 신규 작성한다.

문서에는 최소 아래 내용을 포함한다.

### 7.1 FCFVehicleNetState 초안

후보 필드:

```text
bValid
SequenceId
ServerTimeSeconds
ServerTransform
LinearVelocity
AngularVelocityDeg
ForwardSpeedKmh
Throttle
Brake
Steering
bHandbrake
bGrounded
```

필드별로 아래를 설명한다.

```text
- 서버에서 캡처하는 값인지
- 클라이언트 표시용 값인지
- 보간 대상인지
- 소유 차량 보정에서 사용할 후보인지
```

### 7.2 역할별 정책

```text
Authority / Dedicated Server:
- 차량 물리 권위 보유
- NetState 샘플 생성
- 충돌/겹침/판정 기준 유지

AutonomousProxy:
- 현재 단계에서는 기본 조작감 유지
- 예측/서버 보정은 후속 단계
- 현재는 진단 로그만 유지

SimulatedProxy:
- 원격 차량 표시 대상
- 서버 NetState를 직접 스냅하지 않고 보간 대상으로 사용
- 작은 오차는 보간, 큰 오차는 제한적 스냅 후보
```

### 7.3 원격 차량 보간 정책 초안

```text
- 보간 대상은 SimulatedProxy부터 시작한다.
- 서버 NetState를 RingBuffer 또는 소형 배열로 저장한다.
- 표시 시간은 현재 서버 시간보다 InterpolationDelay만큼 과거로 둔다.
- 기본 후보 InterpolationDelay: 100ms ~ 200ms
- 작은 위치 오차는 보간
- 큰 위치 오차는 빠른 보정 또는 제한적 스냅
- 회전은 Slerp 또는 RInterp 계열 후보
- 선형 속도와 각속도는 예측/외삽 후보지만 1차 구현에서는 신중히 제한한다.
```

### 7.4 bReplicateMovement 정책 전환 조건

```text
단기:
- Replicates=true 유지
- bReplicateMovement는 실험 변수 유지

중기 후보:
- SimulatedProxy 원격 차량 표시를 VehicleNetState 보간으로 대체할 수 있게 되면 bReplicateMovement=false 전환 검토

전환 전 필수 조건:
- 서버 NetState가 안정적으로 복제됨
- 원격 차량 표시가 보간으로 안정화됨
- 큰 오차 처리 기준이 있음
- 충돌/권위 판정은 서버 기준으로 유지됨
```

---

## 8. 진단 코드 보강 요구사항

현재 `SampleAge`는 이름을 그대로 유지해도 되지만, 로그에는 아래 값을 추가한다.

```text
ServerTimeDelta
ReceivedAge
LastReceivedSeq
```

### 8.1 OnRep 수신 시간 기록

`OnRep_VehicleNetDebugServerSample`에서 다음 값을 기록한다.

```text
LastVehicleNetDebugReceivedLocalTimeSec
LastVehicleNetDebugReceivedSequenceId
```

필요한 멤버 후보:

```cpp
// [v2.24.0] 마지막 VehicleNetDebug 샘플을 수신한 클라이언트 로컬 시간입니다.
float LastVehicleNetDebugReceivedLocalTimeSec = -1.0f;

// [v2.24.0] 마지막으로 수신한 VehicleNetDebug 샘플 번호입니다.
int32 LastVehicleNetDebugReceivedSequenceId = INDEX_NONE;
```

### 8.2 로그 계산

```text
ServerTimeDelta = CurrentServerTimeSeconds - VehicleNetDebugServerSample.ServerWorldTimeSeconds
ReceivedAge = CurrentWorldTimeSec - LastVehicleNetDebugReceivedLocalTimeSec
```

주의:

```text
ServerTimeDelta는 음수가 나올 수 있는 진단값으로 인정한다.
ReceivedAge는 OnRep 수신 후 경과 시간이므로 원격 보간 진단에 더 안정적인 값이다.
```

### 8.3 로그 유지

기존 `SampleAge`는 당장 제거하지 않는다.

권장:

```text
- SampleAge는 ServerTimeDelta와 같은 값으로 유지하거나 deprecated 성격으로 둔다.
- 새 로그에는 ServerTimeDelta와 ReceivedAge를 반드시 추가한다.
```

---

## 9. acceptance

작업 완료 판정 기준:

```text
- VehicleNetStateDesign.md가 생성된다.
- 문서에 FCFVehicleNetState 초안이 포함된다.
- 문서에 SimulatedProxy 원격 차량 보간 정책이 포함된다.
- 문서에 bReplicateMovement 전환 조건이 포함된다.
- VehicleNetDebug 로그에 ServerTimeDelta와 ReceivedAge가 추가된다.
- OnRep 수신 로컬 시간과 수신 Seq가 기록된다.
- 기존 LocErr, RotErr, VelErr, SpeedErr, AngVelErr, RepMove, Seq 로그는 유지된다.
- 원격 차량 보간 실제 구현은 없다.
- 소유 차량 예측/서버 보정 실제 구현은 없다.
- bReplicateMovement 기본 정책 변경은 없다.
- Editor / Server 빌드가 성공한다.
```

---

## 10. verification

검증 절차:

```text
1. 코드에서 SetActorLocation, SetActorRotation, TeleportTo가 새로 추가되지 않았는지 확인한다.
2. CarFight_ReEditor Win64 Development 빌드.
3. CarFight_ReServer Win64 Development 빌드.
4. Dedicated Server 2클라 실행.
5. bReplicateMovement=true 상태에서 키보드 입력으로 로그 수집.
6. bReplicateMovement=false 상태에서 키보드 입력으로 로그 수집.
7. Tools/ExtractNetLog.bat 실행.
8. 로그에 ServerTimeDelta / ReceivedAge / LastReceivedSeq가 나오는지 확인.
9. ReceivedAge가 음수로 지속 출력되지 않는지 확인.
```

---

## 11. 산출물

Codex는 작업 완료 후 아래 문서를 남긴다.

```text
Document/Plan/ServerUpgradePlan/VehicleNetStateDesign.md
Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetStatePlan_Review.md
```

리뷰 문서에는 아래를 포함한다.

```text
- 수정 파일 목록
- 추가 로그 필드
- ServerTimeDelta / ReceivedAge 분리 방식
- NetState 설계 요약
- 원격 차량 보간 정책 요약
- Editor / Server 빌드 결과
- Changelog
- Migration notes
```

---

## 12. Changelog

### v0.1.0

```text
- CFVNetStatePlan Codex 작업지시서 작성
- NetState 설계 문서 요구사항 정의
- SimulatedProxy 원격 차량 보간 정책 요구사항 정의
- SampleAge를 ServerTimeDelta / ReceivedAge로 분리하는 진단 보강 정의
```
