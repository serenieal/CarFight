# CFVNetDbgFix 리뷰

- 문서 버전: v0.1.0
- 작성일: 2026-06-05
- 대상 프로젝트: CarFight
- 작업 유형: 차량 네트워크 물리 진단 보강
- 상태: Build Passed

---

## 1. Modified files

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
Document/Plan/ServerUpgradePlan/VehicleNetPhysicsJitter.md
Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetDbgFix_Review.md
```

---

## 2. Added log fields

기존 `VehicleNetDebug:` 접두사와 핵심 필드는 유지했다.

유지 필드:

```text
RepMove
Seq
SampleAge
LocErr
RotErr
VelErr
SpeedErr
ServerLoc
LocalLoc
ServerVel
LocalVel
```

추가 필드:

```text
NetMode
HasAuthority
IsLocal
bReplicates
OwnerController
RuntimeReady
ServerTimeValid
CurrentServerTimeValid
AngVelErr
ServerAngVel
LocalAngVel
```

서버 샘플 구조체 추가 필드:

```text
bServerTimeValid
ServerAngularVelocityDeg
```

---

## 3. SampleAge fix method

기존 문제:

```text
서버 프로세스의 GetWorld()->GetTimeSeconds()와 클라이언트 프로세스의 GetWorld()->GetTimeSeconds()를 직접 비교해서 SampleAge가 -12s, -34s, -96s처럼 크게 음수로 나올 수 있었다.
```

수정 방식:

```text
서버 샘플 생성 시 GameState->GetServerWorldTimeSeconds()를 사용한다.
클라이언트 로그 계산 시에도 GameState->GetServerWorldTimeSeconds()를 사용한다.
양쪽 모두 GameState 서버 시간이 유효할 때만 SampleAge를 계산한다.
시간 기준이 유효하지 않으면 SampleAge=-1.0으로 남기고 ServerTimeValid / CurrentServerTimeValid를 false로 기록한다.
```

이 방식은 Dedicated Server와 클라이언트가 서로 다른 프로세스에서 실행될 때도 같은 서버 시간 기준으로 SampleAge를 해석하게 한다.

---

## 4. Same-PC gamepad input contamination warning

같은 PC에서 Dedicated Server 2클라 테스트를 할 때 하나의 물리 게임패드 입력이 두 클라이언트 프로세스에 동시에 읽히는 현상이 관측되었다.

판정:

```text
현재 단계에서는 네트워크 복제 결함으로 단정하지 않는다.
로컬 입력 폴링 오염 조건으로 분리해서 본다.
```

재검증 권장:

```text
Client 1은 키보드 입력으로 테스트한다.
Client 2는 입력을 주지 않거나 별도 입력 장치 격리가 확인된 상태에서 테스트한다.
VehicleNetDebug 로그를 Tools/ExtractNetLog.bat로 추출해 RepMove true/false를 비교한다.
```

---

## 5. Editor and Server build results

```text
CarFight_ReEditor Win64 Development: Succeeded
CarFight_ReServer Win64 Development: Succeeded
```

금지 호출 검색:

```text
SetActorLocation: not found in modified diagnostic code
SetActorRotation: not found in modified diagnostic code
TeleportTo: not found in modified diagnostic code
```

주의:

```text
이번 작업은 진단 보강이며 차량 Transform 보정, 원격 차량 보간, 소유 차량 예측/서버 보정은 구현하지 않았다.
bReplicateMovement 기본 정책도 변경하지 않았다.
```

---

## 6. Changelog

### v0.1.0

```text
- FCFVehicleNetDebugSample에 bServerTimeValid 추가
- FCFVehicleNetDebugSample에 ServerAngularVelocityDeg 추가
- SampleAge 계산 기준을 GameState 서버 시간으로 보강
- 서버 시간 기준이 없을 때 SampleAge=-1.0과 유효성 false 로그를 남기도록 변경
- NetMode, HasAuthority, IsLocal, bReplicates, OwnerController, RuntimeReady 로그 추가
- ServerAngVel, LocalAngVel, AngVelErr 로그 추가
- Editor / Server 빌드 성공 확인
```

---

## 7. Migration notes

에디터에서 별도 BP 에셋을 자동 수정하지 않았다.

테스트 절차:

```text
1. BP_CFVehiclePawn 또는 파생 Pawn에서 VehiclePawn|NetDebug 옵션을 확인한다.
2. bReplicateMovement=true로 Dedicated Server 2클라 주행 테스트를 실행한다.
3. Client 1은 키보드 입력을 사용해 같은 PC 게임패드 입력 오염을 피한다.
4. Tools/ExtractNetLog.bat로 VehicleNetDebug 로그를 추출한다.
5. bReplicateMovement=false로 같은 테스트를 반복한다.
6. LocErr, RotErr, VelErr, SpeedErr, AngVelErr, ServerTimeValid, CurrentServerTimeValid, SampleAge를 비교한다.
```

예상 결과:

```text
GameState 서버 시간이 준비된 정상 Dedicated Server 테스트에서는 SampleAge가 지속적인 큰 음수로 나오지 않는다.
시간 기준이 아직 준비되지 않은 초기 구간은 SampleAge=-1.0으로 명시된다.
```
