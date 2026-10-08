# CFVNetAgeSplit 리뷰

- 문서 버전: v0.1.0
- 작성일: 2026-06-05
- 대상 프로젝트: CarFight
- 작업 유형: VehicleNetDebug 시간 진단 분리
- 상태: Build Passed

---

## 1. Modified files

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetAgeSplit_Review.md
```

---

## 2. Added field

```text
LastVehicleNetDebugSampleReceiveLocalTimeSec
```

역할:

```text
클라이언트가 마지막 VehicleNetDebug 서버 샘플을 수신한 로컬 월드 시각을 저장한다.
이 필드는 private 일반 float이며 복제하지 않는다.
초기값은 -1.0f이다.
```

---

## 3. Age split method

기존 호환 필드:

```text
SampleAge
```

이번 작업에서 추가한 분리 필드:

```text
ServerTimeDelta
ReceivedAge
```

계산 방식:

```text
ServerTimeDelta = CurrentServerTimeSeconds - VehicleNetDebugServerSample.ServerWorldTimeSeconds
```

조건:

```text
VehicleNetDebugServerSample.bServerTimeValid == true
CurrentServerTimeValid == true
```

조건을 만족하지 않으면:

```text
ServerTimeDelta = -1.0f
```

수신 후 경과 시간:

```text
ReceivedAge = CurrentWorldTimeSec - LastVehicleNetDebugSampleReceiveLocalTimeSec
```

조건:

```text
LastVehicleNetDebugSampleReceiveLocalTimeSec >= 0.0f
```

조건을 만족하지 않으면:

```text
ReceivedAge = -1.0f
```

호환 정책:

```text
SampleAge는 로그에서 유지한다.
이번 단계에서 SampleAge는 ServerTimeDelta와 같은 값을 출력한다.
```

---

## 4. Preserved log fields

기존 `VehicleNetDebug:` 접두사는 유지했다.

기존 주요 필드도 유지했다.

```text
RepMove
Seq
SampleAge
LocErr
RotErr
VelErr
SpeedErr
```

추가 필드:

```text
ServerTimeDelta
ReceivedAge
```

---

## 5. Build results

```text
CarFight_ReEditor Win64 Development: Succeeded
CarFight_ReServer Win64 Development: Succeeded
```

---

## 6. Forbidden correction check

검색 대상:

```text
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

검색 결과:

```text
SetActorLocation: not found
SetActorRotation: not found
TeleportTo: not found
```

이번 작업은 진단 로그 분리만 수행했으며 런타임 이동, 물리 시뮬레이션, Transform 보정 동작은 변경하지 않았다.

---

## 7. Changelog

### v0.1.0

```text
- LastVehicleNetDebugSampleReceiveLocalTimeSec 추가
- OnRep_VehicleNetDebugServerSample에서 서버 샘플 수신 로컬 시각 저장
- LogVehicleNetDebugClientError에서 ServerTimeDelta 계산 추가
- LogVehicleNetDebugClientError에서 ReceivedAge 계산 추가
- VehicleNetDebug 로그에 ServerTimeDelta / ReceivedAge 추가
- SampleAge는 호환용으로 유지하고 ServerTimeDelta와 같은 값으로 출력
- Editor / Server 빌드 성공 확인
```

---

## 8. Migration notes

별도 BP 에셋 수정은 필요 없다.

로그 해석 기준:

```text
ServerTimeDelta:
- 서버 샘플 시각과 현재 서버 시간의 차이다.
- GameState 서버 시간 기준이 유효하지 않으면 -1.0f이다.

ReceivedAge:
- 클라이언트가 마지막 서버 샘플을 받은 뒤 로컬 시간 기준으로 지난 시간이다.
- 아직 OnRep 수신 시각이 없으면 -1.0f이다.

SampleAge:
- 기존 분석 스크립트 호환을 위해 유지한다.
- 이번 단계에서는 ServerTimeDelta와 같은 값이다.
```
