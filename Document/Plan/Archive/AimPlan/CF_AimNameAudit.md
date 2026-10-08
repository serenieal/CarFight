# CarFight - CF_AimNameAudit

> 역할: 싱글플레이 전환 이후 Aim 코드에 남은 멀티플레이/서버/복제 계열 이름의 정리 우선순위를 고정한다.
> 문서 버전: v1.1.0
> 마지막 정리(Asia/Seoul): 2026-06-19
> 상태: Completed / Rename Applied

---

## 0. 2026-06-19 적용 결과

이 감사 문서의 P0/P1/P2 정리 중 현재 코드에 반영된 항목은 다음이다.

```text
P0 Fire RPC 경로
  -> 완료
  -> ServerRequestFire / ClientReceiveFireResult wrapper 제거
  -> HandleFireStarted -> BuildFireCommand -> ValidateFireCommand -> RunLocalDummyHitScan -> ApplyFireResult

P1 Aim 상태 타입 / API 이름
  -> 완료
  -> FireValidationState / AimVisualState 기준으로 정리

P1 Reticle enum 이름
  -> 완료
  -> FirePending / FireRejected / InvalidLocalState / TraceMiss 기준으로 정리

P1 Trace Debug 표시명
  -> 완료
  -> LocalAimTraceDebug 기준으로 정리

P1 Net Serialization 표현
  -> 빌드 완료 / 에디터 재확인 필요
  -> Aim 타입의 FVector_NetQuantize 계열 제거
```

현재 남은 이름 정리 후보는 다음이다.

```text
UCFVehicleAimComp::BuildFireRequest()
FCFVehicleFireRequest
FCFVehicleFireResult
```

위 이름은 현재 네트워크 요청이 아니라 로컬 Fire Command / Fire Result 데이터로 쓰인다. 추가 리네이밍은 저장된 BP 참조와 에디터 확인 후 별도 작업으로 분리한다.

Change Note:
- 아래 1장 이후 내용은 최초 감사 시점의 원본 판단을 보존한다.
- 현재 작업 상태를 볼 때는 이 0장을 먼저 보고, 상세 근거가 필요할 때만 아래 P0/P1/P2 분석을 참고한다.

---

## 1. 문서 목적

이 문서는 최초 작성 당시에는 코드 리네이밍 작업지시서가 아니었다. 현재는 적용 완료 항목과 남은 후보를 함께 기록하는 감사 결과 문서로 본다.

목적은 최초 감사 당시 코드에 남은 `Server`, `Client`, `Rep`, `Request`, `RPC` 계열 이름을 다음 기준으로 분류하는 것이다.

1. 싱글플레이 전환 후 즉시 수정해야 하는가?
2. BP / UHT / 에디터 참조 때문에 단계적으로 바꿔야 하는가?
3. 방어 코드 또는 과거 기록으로 유지해도 되는가?

현재 기준에서 가장 중요한 원칙은 다음이다.

- 현재 게임 방향은 싱글플레이다.
- 새 구현 목표는 `Local Aim -> Local Fire Validation -> Local Hit Result -> Local Feedback`이다.
- 기존 C++ / BP 참조를 깨뜨릴 수 있는 이름은 말 없이 바꾸지 않는다.
- 먼저 호환 경로를 만들고, 그다음 표시명 / Tooltip / 문서 / API 이름 순서로 정리한다.

---

## 2. 감사 범위

확인한 코드 범위:

```text
D:\Work\CarFight_git\UE\Source\CarFight_Re
```

중점 파일:

```text
Public/CFVehicleAimTypes.h
Public/CFVehicleAimComp.h
Private/CFVehicleAimComp.cpp
Public/CFVehiclePawn.h
Private/CFVehiclePawn.cpp
Private/UI/CFVehicleDebugPanelWidget.cpp
Private/UI/CFAimReticleWidget.cpp
```

확인 키워드:

```text
Server
Client
Rep
Request
RPC
Multicast
Authority
Net
Replication
Replicated
```

---

## 3. 현재 판정 요약

| 등급 | 판정 | 의미 |
|---|---|---|
| P0 | 구조 전환 필요 | 실제 런타임 경로가 아직 서버/RPC 이름과 UFUNCTION 속성에 의존한다. |
| P1 | 단계적 리네이밍 필요 | 타입/필드/API/Debug 표시명이 BP 직렬화와 UHT에 연결될 수 있다. |
| P2 | 보존 가능 | 방어 코드, 과거 기록, 또는 싱글플레이에서도 harmless한 가드다. |

최초 감사 당시에는 바로 코드를 전면 리네이밍하지 않는다고 판단했다. 2026-06-19 적용 결과는 이 문서 0장을 따른다.

이유:

- `UFUNCTION(Server, Reliable)` / `UFUNCTION(Client, Reliable)` 제거는 런타임 동작과 BP 참조에 영향을 줄 수 있다.
- `USTRUCT(BlueprintType)` 필드명 변경은 에디터 표시, 저장된 BP 핀, 디버그 위젯 참조에 영향을 줄 수 있다.
- `FCFVehicleFireRequest`, `FCFVehicleServerAimState`, `FCFVehicleRepAimVisualState`는 여러 파일에서 동시에 쓰인다.

---

## 4. P0 - 구조 전환 필요

### 4.1 Fire 입력이 아직 RPC 경로 이름을 지난다

확인 위치:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

확인 내용:

- `ServerRequestFire(const FCFVehicleFireRequest& FireRequest)`
- `ClientReceiveFireResult(const FCFVehicleFireResult& FireResult)`
- `UFUNCTION(Server, Reliable)`
- `UFUNCTION(Client, Reliable)`
- `HandleFireStarted()`에서 `HasAuthority()` 여부에 따라 RPC 또는 RPC 구현 함수를 호출

현재 문제:

싱글플레이 기준에서는 Fire 입력이 로컬 `Fire Command` 검증으로 끝나야 한다. 하지만 현재 이름과 UFUNCTION 속성은 아직 서버 요청 / 클라이언트 결과 수신 구조를 암시한다.

권장 전환:

```text
1단계:
  HandleFireStarted()
    -> BuildFireCommand()
    -> ValidateFireCommand()
    -> RunLocalDummyHitScan()
    -> ApplyFireResult()

2단계:
  기존 ServerRequestFire / ClientReceiveFireResult는 임시 wrapper로 남긴다.

3단계:
  BP 참조와 빌드 검증 후 RPC UFUNCTION 속성을 제거한다.
```

즉시 리네이밍 금지 이유:

저장된 BP나 입력 연결이 C++ 함수명을 참조하고 있을 가능성이 있다. 먼저 새 로컬 함수 경로를 추가하고, 기존 함수는 Deprecated wrapper로 남기는 편이 안전하다.

---

### 4.2 Server HitScan 이름이 실제 로컬 Trace 역할을 한다

확인 위치:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

확인 내용:

- `ValidateFireRequestOnServer()`
- `RunServerDummyHitScan()`
- `bDrawServerAimTraceDebug`
- `ServerAimTraceDebugDuration`
- `CarFightServerAimTrace`

현재 문제:

싱글플레이 기준에서는 이 로직이 서버 판정이 아니라 로컬 발사 검증과 로컬 Trace다.

권장 전환:

```text
ValidateFireRequestOnServer()
  -> ValidateFireCommand()

RunServerDummyHitScan()
  -> RunLocalDummyHitScan()

bDrawServerAimTraceDebug
  -> bDrawLocalAimTraceDebug

ServerAimTraceDebugDuration
  -> LocalAimTraceDebugDuration
```

마이그레이션 지침:

- 새 이름의 함수와 변수를 먼저 추가한다.
- 기존 이름은 한 버전 동안 wrapper 또는 alias 역할로 유지한다.
- Debug UI 표시명과 Tooltip을 먼저 로컬 기준으로 바꾼다.

---

## 5. P1 - 단계적 리네이밍 필요

### 5.1 Aim 타입 이름

확인 위치:

```text
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
```

현재 이름:

```text
FCFVehicleServerAimState
FCFVehicleRepAimVisualState
FCFVehicleFireRequest
```

권장 새 의미:

```text
FCFVehicleFireValidationState
FCFVehicleFireVisualState
FCFVehicleFireCommand
```

현재 문제:

이 타입들은 `BlueprintType`이므로 에디터 저장 데이터, Debug Snapshot, UI ViewData에 영향을 줄 수 있다.

권장 전환:

1. 새 타입을 바로 만들기 전에 현재 사용처를 빌드 기준으로 모두 확인한다.
2. 먼저 DisplayName / ToolTip을 싱글플레이 의미로 정리한다.
3. 다음 단계에서 새 타입을 추가하고 기존 타입을 호환 wrapper로 유지한다.
4. BP 참조를 교체한 뒤 기존 타입 제거 여부를 결정한다.

---

### 5.2 Reticle / RejectReason enum 이름

확인 위치:

```text
UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h
UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp
```

현재 이름:

```text
WaitingServer
ServerRejected
NoAuthority
InvalidOwner
ServerTraceMiss
```

권장 새 의미:

```text
FirePending 또는 FireBlocked
FireRejected 또는 FireConfirmed
InvalidState
InvalidPawn
TraceMiss
```

현재 문제:

Reticle UI와 Debug 문자열이 이 enum 값을 직접 표시한다. 바로 제거하면 UI 분기가 깨질 수 있다.

권장 전환:

- v1: 기존 enum은 유지하고 표시 텍스트만 싱글플레이 문구로 바꾼다.
- v2: 새 enum 값을 추가하고 기존 값은 Deprecated 후보로 둔다.
- v3: 사용처가 모두 새 값으로 이동하면 기존 값을 제거한다.

---

### 5.3 VehicleDebug Aim 표시명

확인 위치:

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/UI/CFVehicleDebugPanelWidget.cpp
```

현재 표시:

```text
서버 Aim 상태
서버 발사 가능
서버 조준각 내부
마지막 서버 거부 사유
AimRepVisual
```

권장 표시:

```text
발사 검증 상태
로컬 발사 가능
로컬 조준각 내부
마지막 발사 거부 사유
AimFireVisual
```

현재 문제:

표시명은 사용자 혼선을 바로 만들지만, 코드 시그니처보다 변경 위험이 낮다.

권장 전환:

이 항목은 다음 코드 패치에서 우선 수정해도 된다.

단, 내부 타입명까지 동시에 바꾸지는 않는다.

---

## 6. P2 - 보존 가능

### 6.1 Dedicated Server 가드

확인 위치:

```text
UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h
UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

예:

```text
ShouldSkipCameraRuntimeOnDedicatedServer()
GetNetMode() == NM_DedicatedServer
```

판정:

현재 싱글플레이 기준에서 필수는 아니지만, 존재해도 런타임 위험은 낮다. 이 가드는 나중에 별도 청소 작업에서 처리한다.

즉시 제거하지 않는 이유:

- 카메라 / UI / Pawn 초기화 방어 코드로 쓰이고 있다.
- 제거가 실제 기능 개선으로 이어지지 않는다.
- 현재 목표는 Aim 네트워크 명칭 정리다.

---

### 6.2 과거 Changelog / Generated 문서

확인 위치:

```text
AimPlan/Generated
CarFight_Re.Build.cs
CFVehiclePawn 상단 Changelog
```

판정:

과거 기록은 유지한다.

단, 현재 작업 기준 문서로 쓰지 않도록 `AimPlan/Generated/README.md`에서 Historical 상태를 명시했다.

---

## 7. 권장 작업 순서

### Step 1 - Debug 표시명 정리

목표:

사용자가 에디터에서 보는 텍스트부터 싱글플레이 기준으로 맞춘다.

대상:

```text
CFVehicleAimTypes.h
CFVehicleAimComp.h
CFVehiclePawn.h
CFVehicleDebugPanelWidget.cpp
CFAimReticleWidget.cpp
```

작업:

- Tooltip / DisplayName / Debug 라벨의 `서버`, `복제`, `클라이언트` 표현을 로컬 판정 기준으로 수정
- 타입명과 함수명은 유지

검증:

- C++ 빌드
- PIE Single Player
- VehicleDebug Aim 표시 확인

### Step 2 - 로컬 Fire 함수 추가

목표:

실제 Fire 흐름이 RPC 이름을 거치지 않아도 동작하도록 로컬 함수를 만든다.

대상:

```text
CFVehiclePawn.h
CFVehiclePawn.cpp
CFVehicleAimComp.h
CFVehicleAimComp.cpp
```

작업:

- `BuildFireCommand()`
- `ValidateFireCommand()`
- `RunLocalDummyHitScan()`
- `ApplyFireResult()`
- 기존 `ServerRequestFire()` / `ClientReceiveFireResult()`는 wrapper로 유지

검증:

- C++ 빌드
- PIE Single Player Fire 입력
- Standalone Game Fire 입력
- VehicleDebug LastFireResult 갱신 확인

### Step 3 - 타입 리네이밍 설계

목표:

`FCFVehicleServerAimState`, `FCFVehicleRepAimVisualState`, `FCFVehicleFireRequest`를 새 타입으로 바꿀 수 있는지 확인한다.

작업:

- BP 참조 확인
- 저장된 에셋 참조 위험 확인
- 새 타입 도입 시 마이그레이션 경로 작성

검증:

- UHT 빌드
- 에디터 열기
- BP Compile
- WBP / VehicleDebug 표시 확인

---

## 8. Standalone 검증 전 체크포인트

Standalone Game 검증 전에 아래를 확인한다.

```text
1. CF_AimVerify.md 기준 검증 절차를 사용한다.
2. CF_AimNetVerify.md는 Deprecated 문서이므로 사용하지 않는다.
3. Fire 입력은 현재 코드상 RPC 이름을 지나갈 수 있지만, 싱글플레이에서는 로컬 판정 의미로 기록한다.
4. VehicleDebug Aim에서 Server/Rep 라벨이 보이면 실패가 아니라 리네이밍 전 잔여 표시로 기록한다.
5. 실제 크래시, 입력 미동작, Reticle 미표시, HitLocation 미갱신은 기능 이슈로 기록한다.
```

---

## 9. 완료 조건

이 감사 문서는 다음 조건을 만족하면 완료로 본다.

- Aim 코드에 남은 주요 네트워크 계열 이름의 위치가 정리되어 있다.
- 즉시 수정 대상과 단계적 리네이밍 대상을 구분했다.
- BP / UHT / 에디터 참조 위험 때문에 전면 리네이밍을 보류하는 이유가 명시되어 있다.
- 다음 코드 작업 순서가 정리되어 있다.

---

## ChangeLog

- v1.0.0 / 2026-06-19
  - 싱글플레이 전환 이후 Aim 코드에 남은 서버/RPC/복제 계열 이름을 감사했다.
  - P0/P1/P2 우선순위를 정의했다.
  - 전면 리네이밍 대신 단계적 마이그레이션 경로를 제안했다.

## 마이그레이션 지침

- 다음 코드 작업은 표시명/Tooltip 정리부터 시작한다.
- 함수 시그니처와 `BlueprintType` 구조체 이름은 BP 참조를 확인하기 전까지 직접 변경하지 않는다.
- RPC 제거는 새 로컬 Fire 경로를 추가하고 Standalone 검증이 끝난 뒤 진행한다.
