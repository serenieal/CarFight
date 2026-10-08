# CFVNetStateBase Review

- Version: v2.25.0
- Date: 2026-06-08
- Contract: `Document/Plan/ServerUpgradePlan/Generated/Final/CFVNetStateBase_CodexContract.yaml`
- Scope: 차량 전용 NetState 복제 베이스 추가

## 1. 변경 파일

- `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

## 2. 구현 요약

### 2.1 NetState 구조체 추가

- `FCFVehicleNetState`를 `BlueprintType`으로 추가했다.
- 서버 권위 차량 상태를 아래 필드로 담는다.
  - `bValid`
  - `ServerSequenceId`
  - `ServerTimeSeconds`
  - `ServerLocation`
  - `ServerRotation`
  - `ServerLinearVelocity`
  - `ServerAngularVelocityDeg`
  - `ServerForwardSpeedCmPerSec`
- 위치/속도 계열 필드는 네트워크 복제 비용을 낮추기 위해 `FVector_NetQuantize10`을 사용했다.
- 모든 `UPROPERTY`에는 한국어 `DisplayName`과 `ToolTip`을 추가했다.

### 2.2 클라이언트 수신 버퍼 추가

- `FCFVehicleNetStateBufferItem`을 추가했다.
- 버퍼 항목은 아래 값을 저장한다.
  - `FCFVehicleNetState State`
  - `float ReceivedLocalTimeSeconds`
- `VehicleNetStateBuffer`는 로컬 전용 `TArray<FCFVehicleNetStateBufferItem>`으로 유지한다.
- 이번 단계에서는 버퍼를 이동 보정, 스냅, 보간, 예측에 사용하지 않는다.

### 2.3 복제 파이프라인 추가

- `ReplicatedVehicleNetState`를 `ReplicatedUsing=OnRep_VehicleNetState`로 추가했다.
- `GetLifetimeReplicatedProps`에 `DOREPLIFETIME(ACFVehiclePawn, ReplicatedVehicleNetState)`를 등록했다.
- 서버 권위 Pawn만 `VehicleNetStateSampleIntervalSec` 주기로 NetState를 캡처해 복제한다.
- 기본 샘플 주기는 `0.0667f`로 설정했다.

### 2.4 OnRep 처리

- `OnRep_VehicleNetState`는 유효하지 않은 상태를 무시한다.
- 수신 시 `GetWorld()->GetTimeSeconds()`로 클라이언트 로컬 수신 시각을 기록한다.
- `AddVehicleNetStateBufferItem`은 마지막 버퍼 순번보다 같거나 낮은 중복/역순 샘플을 무시한다.
- `VehicleNetStateMaxBufferSamples`를 초과하면 오래된 샘플부터 제거한다.

### 2.5 진단 로그

- `bLogVehicleNetStateBase`가 true일 때만 `VehicleNetStateBase:` 로그를 출력한다.
- 로그 주기는 `VehicleNetStateBaseLogIntervalSec`로 제한한다.
- 로그에는 아래 항목을 포함한다.
  - Pawn
  - Role
  - IsLocal
  - Seq
  - BufferCount
  - ServerTimeSeconds
  - ReceivedLocalTimeSeconds
  - RepMove
  - bReplicates

## 3. 이동 동작 영향

- 이번 구현은 차량 Transform, 물리 상태, MovementComponent에 보정 값을 적용하지 않는다.
- NetState는 서버 캡처, 복제, 클라이언트 버퍼 저장, 로그 출력까지만 수행한다.
- `SetActorLocation`, `SetActorRotation`, `TeleportTo`를 추가하지 않았다.

## 4. 검증 결과

### 4.1 Editor 빌드

- Command:
  - `D:\UnrealEngine_Source\Engine\Build\BatchFiles\Build.bat CarFight_ReEditor Win64 Development -Project=D:\Work\CarFight_git\UE\CarFight_Re.uproject -WaitMutex -NoHotReloadFromIDE`
- Result:
  - Succeeded

### 4.2 Server 빌드

- Command:
  - `D:\UnrealEngine_Source\Engine\Build\BatchFiles\Build.bat CarFight_ReServer Win64 Development -Project=D:\Work\CarFight_git\UE\CarFight_Re.uproject -WaitMutex -NoHotReloadFromIDE`
- Result:
  - Succeeded

### 4.3 금지 동작 검색

- Command:
  - `Select-String -Path UE\Source\CarFight_Re\Private\CFVehiclePawn.cpp -Encoding utf8 -Pattern 'SetActorLocation|SetActorRotation|TeleportTo'`
- Result:
  - No match

## 5. Changelog

- v2.25.0
  - 차량 전용 `FCFVehicleNetState` 추가.
  - 클라이언트 수신 버퍼 `FCFVehicleNetStateBufferItem` 및 `VehicleNetStateBuffer` 추가.
  - `ReplicatedVehicleNetState` 복제와 `OnRep_VehicleNetState` 수신 경로 추가.
  - 서버 권위 NetState 주기 샘플링 추가.
  - `VehicleNetStateBase:` 진단 로그 추가.
  - 이동 보정/보간/스냅 적용 없이 데이터 파이프라인만 구성.

## 6. 마이그레이션 지침

- 기존 Blueprint 또는 C++ 호출부 변경은 필요 없다.
- 차량 NetState 수신 여부를 확인하려면 `bLogVehicleNetStateBase`를 true로 켠다.
- 로그가 과도하면 `VehicleNetStateBaseLogIntervalSec`를 늘린다.
- 다음 단계에서 보간/예측을 붙일 때는 `VehicleNetStateBuffer`를 읽기 전용 입력으로 먼저 사용하고, Transform 적용 로직은 별도 계약에서 추가해야 한다.
