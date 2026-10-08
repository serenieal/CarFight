# CFVNetDbg Task Source

- 문서 버전: v0.1.0
- 작성일: 2026-06-05
- 대상 프로젝트: CarFight
- 대상 UE 프로젝트: UE/CarFight_Re.uproject
- 작업 유형: 최소 진단 패치 설계
- 상태: ReadyForCodex

---

## 1. 작업 목적

Dedicated Server 2클라 차량 주행 중 발생하는 차량 네트워크 물리 흔들림, 원격 차량 순간이동, 장기 위치/충돌 불일치 문제를 수치로 진단한다.

이번 작업은 최종 이동 시스템 구현이 아니다. `bReplicateMovement=true/false`를 비교할 수 있도록 서버 권위 샘플과 클라이언트 로컬 샘플을 로그로 남기는 진단 패치만 추가한다.

---

## 2. 현재 결론

- `bReplicateMovement=true`는 서버 Actor Movement 자동 복제가 Chaos Vehicle 물리 표시/보정과 충돌해 시각적 끊김을 만들 가능성이 높다.
- `bReplicateMovement=false`는 시각적 끊김은 줄지만 장기적으로 서버/클라 위치와 충돌 판정이 갈라진다.
- 단순 ON/OFF는 최종 해법이 아니다.
- 최종 방향은 차량 전용 서버 권위 NetState, 원격 차량 보간, 소유 차량 예측/서버 보정이다.

---

## 3. 이번 Codex 작업 범위

### scope.in

- `ACFVehiclePawn`에 네트워크 물리 진단 로그 옵션 추가
- 서버에서 주기적으로 차량 권위 상태를 샘플링해 복제
- 클라이언트에서 복제된 서버 샘플과 자기 로컬 차량 상태의 위치/회전/속도 오차 로그 출력
- `bReplicateMovement=true`와 `false` 양쪽 테스트에 사용할 수 있게 유지

### scope.out

- `bReplicateMovement` 기본값 강제 변경 금지
- 원격 차량 보간 구현 금지
- 소유 차량 예측/서버 보정 구현 금지
- 무기/발사/대미지 서버 권한 구조 작업 금지
- BP_CFVehiclePawn 에셋 자동 수정 금지

### constraints

- 진단 패치 외 이동 보정/보간/예측 구현 금지
- `SetActorLocation`, `SetActorRotation`, `TeleportTo` 기반 보정 금지
- 신규 파일명은 32자 이하 유지
- 모든 신규 변수/함수 위에 한 줄 한국어 주석 작성
- 모든 UPROPERTY/UFUNCTION에 한국어 DisplayName과 ToolTip 작성


---

## 4. 대상 파일

- 수정: `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
- 수정: `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`

필요 시 `UE/Config/DefaultEngine.ini`는 읽기만 하고 수정하지 않는다.

---

## 5. 구현 지시

### 5.1 `CFVehiclePawn.h`

1. `FCFVehicleNetDebugState` USTRUCT를 추가한다.
2. 구조체 필드는 최소 아래를 포함한다.
   - `FVector ServerLocation`
   - `FRotator ServerRotation`
   - `FVector ServerLinearVelocity`
   - `FVector ServerAngularVelocityDeg`
   - `float ServerTimeSeconds`
   - `int32 ServerSampleIndex`
3. 모든 UPROPERTY에는 한국어 `DisplayName`과 `ToolTip`을 작성한다.
4. `ACFVehiclePawn`에 아래 멤버를 추가한다.
   - `bEnableNetPhysicsDebug`
   - `NetPhysicsDebugIntervalSeconds`
   - `ReplicatedNetDebugState`
   - `NetPhysicsDebugElapsedSeconds`
5. `ReplicatedNetDebugState`는 `ReplicatedUsing=OnRep_NetDebugState`로 선언한다.
6. 아래 함수를 선언한다.
   - `virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;`
   - `void UpdateVehicleNetPhysicsDebug(float DeltaSeconds);`
   - `void SampleVehicleNetDebugState(FCFVehicleNetDebugState& OutDebugState) const;`
   - `void LogVehicleNetPhysicsState(const TCHAR* LogReason) const;`
   - `UFUNCTION() void OnRep_NetDebugState();`
7. 함수 선언 바로 위에 한 줄 한국어 주석을 작성한다.

### 5.2 `CFVehiclePawn.cpp`

1. `#include "Net/UnrealNetwork.h"`를 추가한다.
2. 파일 상단 익명 namespace 또는 정적 로그 카테고리로 차량 네트워크 물리 로그를 분리한다.
3. `ACFVehiclePawn::Tick`에서 `bVehicleRuntimeReady` 확인 이후 `UpdateVehicleNetPhysicsDebug(DeltaSeconds)`를 호출한다.
4. `UpdateVehicleNetPhysicsDebug`는 `bEnableNetPhysicsDebug=false`이면 즉시 반환한다.
5. 서버 권한(`HasAuthority`)에서는 주기마다 `ReplicatedNetDebugState`를 갱신한다.
6. 서버와 클라이언트 모두 주기마다 로컬 상태 로그를 남긴다.
7. 클라이언트는 `ReplicatedNetDebugState`가 유효하면 로컬 상태와 서버 샘플 사이의 값을 같이 출력한다.
8. 로그 필드는 최소 아래를 포함한다.
   - NetMode
   - LocalRole
   - RemoteRole
   - HasAuthority
   - IsLocallyControlled
   - bReplicates
   - GetReplicateMovement 값
   - ActorLocation
   - ActorRotation
   - LocalLinearVelocity
   - LocalAngularVelocityDeg
   - ServerLocation
   - ServerRotation
   - LocationErrorCm
   - RotationErrorDeg
   - VelocityErrorCmPerSec
   - Owner Controller 이름
9. `GetLifetimeReplicatedProps`에서 `ReplicatedNetDebugState`를 `DOREPLIFETIME`으로 등록한다.
10. 이 패치는 진단 목적이므로 차량 Transform을 직접 보정하거나 SetActorLocation/SetActorRotation을 호출하지 않는다.

---

## 6. 코드 스타일 규칙

- 파일 경로와 작업 유형을 커밋 메시지 또는 리뷰 문서에 명시한다.
- 모든 신규 변수/함수 위에 한 줄 한국어 주석을 작성한다.
- 모든 UPROPERTY/UFUNCTION에는 한국어 DisplayName과 ToolTip을 작성한다.
- 파일명 32자 제한을 지킨다.
- 변경 버전은 `CFVehiclePawn` 기준 v2.22.0으로 표기한다.
- Changelog와 Migration 메모를 작성한다.

---

## 7. 테스트 절차

1. Editor Target 빌드: `CarFight_ReEditor Win64 Development`
2. Server Target 빌드: `CarFight_ReServer Win64 Development`
3. BP_CFVehiclePawn에서 `bEnableNetPhysicsDebug=true`, `NetPhysicsDebugIntervalSeconds=0.25~0.5`로 설정한다.
4. 테스트 A: `bReplicateMovement=true`로 Dedicated Server + Client 2개 실행
5. 테스트 B: `bReplicateMovement=false`로 같은 테스트 반복
6. 각 테스트에서 저속 가속, 고속 가속, 급조향, 정지, 충돌 근접 통과를 수행한다.
7. 서버 로그와 두 클라이언트 로그를 보관한다.

---

## 8. 성공 기준

- Editor/Server 빌드 성공
- Dedicated Server에서 UI/LocalPlayer 오류 없음
- `bReplicateMovement=true/false` 양쪽에서 같은 로그 항목이 출력됨
- 클라이언트 로그에서 서버 샘플 대비 위치/회전/속도 오차가 수치로 확인됨
- 차량 Transform 보정, 보간, 예측 로직은 추가되지 않음

---

## 9. 다음 단계 메모

이 진단 패치의 로그를 기준으로 다음 결정을 한다.

- 원격 차량 보간용 `FCFVehicleNetState` 설계
- 소유 차량 예측/서버 보정 설계
- `bReplicateMovement`를 끄고 차량 전용 NetState로 대체할지 판단
- 큰 오차 스냅 기준, 작은 오차 보간 기준, 보정 속도 기준 산정

---

## 10. Changelog

### v0.1.0

- 차량 네트워크 물리 흔들림 첫 진단 패치 작업지시서 작성
- 서버 권위 샘플 복제와 클라이언트 오차 로그 범위 정의
- 최종 이동 시스템 구현을 명시적으로 제외
