# VNetStateBase Log Guide

- 문서 버전: v0.1.0
- 작성일: 2026-06-08
- 대상 프로젝트: CarFight
- 관련 작업: CFVNetStateBase
- 목적: VehicleNetStateBase 런타임 로그 수집 절차 정리

---

## 1. 목적

`CFVNetStateBase`는 서버 권위 차량 NetState를 복제하고, 클라이언트가 이를 `VehicleNetStateBuffer`에 저장하는 기초 파이프라인이다.

이번 단계의 런타임 테스트 목표는 다음이다.

```text
- VehicleNetStateBase 로그가 실제로 출력되는지 확인
- Seq가 증가하는지 확인
- BufferCount가 증가하되 최대값을 넘지 않는지 확인
- ReceivedLocalTimeSeconds가 기록되는지 확인
- 기존 VehicleNetDebug 로그가 계속 출력되는지 확인
- 차량 Transform 보정/보간 없이 기존 움직임이 유지되는지 확인
```

---

## 2. BP 설정

`VehicleNetStateBase:` 로그는 기본값이 꺼져 있다.

테스트 전 `BP_CFVehiclePawn`에서 다음 설정을 켠다.

```text
콘텐츠 브라우저(Content Browser)
→ /Game/CarFight/Vehicles/BP_CFVehiclePawn 열기
→ 클래스 기본값(Class Defaults)
→ CarFight|VehiclePawn|NetState
→ bLogVehicleNetStateBase = true
→ Compile
→ Save
```

권장 기본값:

```text
bEnableVehicleNetStateBase = true
VehicleNetStateSampleIntervalSec = 0.0667
VehicleNetStateMaxBufferSamples = 12
bLogVehicleNetStateBase = true
VehicleNetStateBaseLogIntervalSec = 1.0
```

`VehicleNetStateBaseLogIntervalSec`가 1.0이면 로그는 약 1초 단위로 제한된다.

---

## 3. 테스트 실행

기존 절차와 동일하다.

```text
1. BP_CFVehiclePawn → Replicate Movement = true
2. Tools/RunNetTrue.bat 실행
3. Client 1 키보드 주행
4. 서버/클라 종료

5. BP_CFVehiclePawn → Replicate Movement = false
6. Tools/RunNetFalse.bat 실행
7. Client 1 키보드 주행
8. 서버/클라 종료

9. Tools/ExtractNetLog.bat 실행
```

---

## 4. ExtractNetLog.bat v0.2.0 출력 파일

`Tools/ExtractNetLog.bat` v0.2.0은 각 테스트 폴더에 다음 파일을 생성한다.

```text
Client1_VehicleNetDebug.txt
Client2_VehicleNetDebug.txt
Client1_VehicleNetStateBase.txt
Client2_VehicleNetStateBase.txt
Client1_VehicleNetAll.txt
Client2_VehicleNetAll.txt
```

테스트 폴더:

```text
RuntimeLogs/VehicleNet/A_RepMoveTrue/
RuntimeLogs/VehicleNet/B_RepMoveFalse/
```

---

## 5. 분석 기준

### 5.1 VehicleNetStateBase 로그

확인할 필드:

```text
Seq
BufferCount
ServerTime
ReceivedLocalTime
RepMove
bReplicates
Role
IsLocal
```

기대 결과:

```text
- Seq가 시간이 지나며 증가한다.
- BufferCount가 1부터 증가한다.
- BufferCount가 VehicleNetStateMaxBufferSamples를 넘지 않는다.
- ReceivedLocalTime이 기록된다.
- RepMove=true / false가 테스트 설정과 일치한다.
```

### 5.2 VehicleNetDebug 로그

기존처럼 다음 값을 함께 확인한다.

```text
LocErr
RotErr
VelErr
SpeedErr
AngVelErr
ServerTimeDelta
ReceivedAge
```

---

## 6. 실패 패턴

### 6.1 VehicleNetStateBase 로그가 없다

가능한 원인:

```text
- BP_CFVehiclePawn에서 bLogVehicleNetStateBase가 false다.
- 실제 Spawn되는 차량 BP가 BP_CFVehiclePawn이 아니다.
- 서버에서 ReplicatedVehicleNetState가 갱신되지 않는다.
- ExtractNetLog.bat 실행 전 테스트 로그가 생성되지 않았다.
```

### 6.2 BufferCount가 증가하지 않는다

가능한 원인:

```text
- OnRep_VehicleNetState가 호출되지 않는다.
- ReplicatedVehicleNetState가 복제되지 않는다.
- bValid=false 상태로 복제되고 있다.
- ServerSequenceId가 중복 또는 역행으로 필터링되고 있다.
```

### 6.3 BufferCount가 최대값을 넘는다

가능한 원인:

```text
- VehicleNetStateMaxBufferSamples trim 로직이 동작하지 않는다.
```

---

## 7. 이번 단계에서 하지 않을 것

```text
- 원격 차량 보간 구현 금지
- Transform 보정 금지
- SetActorLocation / SetActorRotation / TeleportTo 사용 금지
- 소유 차량 예측/서버 보정 금지
- bReplicateMovement 최종 정책 확정 금지
```

---

## 8. 다음 단계

`VehicleNetStateBase` 로그가 정상 수집되면 다음 코드 작업은 `CFVRemoteInterp` 설계/작업지시서 작성이다.

다만 `CFVRemoteInterp`는 실제 Transform 적용을 포함할 수 있으므로, 적용 대상은 처음부터 다음처럼 제한해야 한다.

```text
- ROLE_SimulatedProxy만 대상
- IsLocallyControlled=true 차량 제외
- HasAuthority=true 서버 차량 제외
- 첫 구현에서는 스냅 금지 또는 매우 제한
```

---

## 9. 변경 기록

### v0.1.0

```text
- VehicleNetStateBase 로그 수집 절차 작성
- ExtractNetLog.bat v0.2.0 출력 파일 정리
- BP 설정 및 실패 패턴 정리
```
