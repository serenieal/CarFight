# CFNetSmooth API 계약

- 문서 버전: v0.14.0
- 작성일: 2026-06-18
- 대상: 범용 Unreal Engine 프로젝트, 현재 검증 호스트 CarFight
- 작업 유형: 신규
- 상태: Draft

---

## 1. 목적

이 문서는 `CFNetSmooth` v0.1 구현 시 사용할 예정 클래스, 구조체, 설정값, BP 노출 API의 계약을 정의한다.

이 문서는 실제 코드가 아니라 구현 기준이다.

`CFNetSmooth`는 CarFight 전용 플러그인이 아니다.
API 계약은 여러 Unreal Engine 프로젝트에서 재사용될 수 있도록 특정 게임 프로젝트 타입, 맵, Build Target을 요구하지 않는다.

---

## 2. 예정 파일 구조

```text
UE/Plugins/CFNetSmooth/CFNetSmooth.uplugin
UE/Plugins/CFNetSmooth/Source/CFNetSmooth/CFNetSmooth.Build.cs
UE/Plugins/CFNetSmooth/Source/CFNetSmooth/Public/CFNetSmoothState.h
UE/Plugins/CFNetSmooth/Source/CFNetSmooth/Public/CFNetSmoothComp.h
UE/Plugins/CFNetSmooth/Source/CFNetSmooth/Public/CFNetSmoothTestActor.h
UE/Plugins/CFNetSmooth/Source/CFNetSmooth/Private/CFNetSmoothComp.cpp
UE/Plugins/CFNetSmooth/Source/CFNetSmooth/Private/CFNetSmoothTestActor.cpp
```

파일명은 모두 32자 이하로 유지한다.

---

## 3. 구조체 계약

### 3.1 FCFNetSmoothState

역할:

- 서버에서 생성한 Transform Snapshot을 담는다.
- 클라이언트 Buffer에서 보간/외삽 기준으로 사용한다.

필수 필드:

| 필드 | 타입 후보 | 역할 |
|---|---|---|
| `ServerTime` | `double` | 서버 기준 State 생성 시간 |
| `Location` | `FVector_NetQuantize100` 또는 `FVector` | 서버 위치 |
| `Rotation` | `FRotator` 또는 `FQuat` | 서버 회전 |
| `LinearVelocity` | `FVector_NetQuantize100` 또는 `FVector` | 제한 외삽용 선형 속도 |
| `AngularVelocity` | `FVector_NetQuantize100` 또는 `FVector` | 제한 외삽용 각속도 |
| `bTeleport` | `bool` | 보간 없이 즉시 적용해야 하는 State 여부 |
| `Sequence` | `uint16` | 오래된 State 폐기와 순서 비교용 번호. UHT 호환을 위해 BP 노출 대상에서 제외 |

v0.1 기본안:

- 위치는 `FVector_NetQuantize100` 후보를 우선 검토한다.
- 회전은 구현 단순성을 위해 `FRotator` 후보를 우선 검토한다.
- 정밀도 문제가 보이면 v0.2에서 `FQuat`로 재검토한다.

---

## 4. 컴포넌트 계약

### 4.1 UCFNetSmoothComp

역할:

- 서버 권위 Actor에서 State를 만든다.
- 원격 클라이언트에서 State Buffer를 유지한다.
- 원격 표시 Transform을 보간한다.
- 외부 차량 Visual/Shell이 Owner를 움직이지 않고 보간 Transform을 읽어갈 수 있게 한다.
- 위치는 State 사이 보간과 최신 State 이후 제한 외삽을 지원한다.
- 회전은 State 사이에서 `FQuat::Slerp`로 보간한다.
- 최신 State 이후 제한 외삽 구간에서는 회전 외삽을 하지 않고 최신 State 회전을 유지한다.
- 위치 또는 회전 오차가 Snap 기준 이상이면 Buffer를 유지한 채 목표 Transform으로 즉시 보정한다.

기본 적용 대상:

```text
AActor
```

v0.1에서는 `SceneComponent` 직접 지정 기능을 보류한다.
v0.12에서는 `SceneComponent` 직접 지정 적용 대신, 외부 코드가 보간 Transform을 읽어 별도 Visual/Shell에 적용할 수 있는 API를 제공한다.

---

## 4.2 테스트 Actor 계약

### 4.2.1 ACFNetSmoothTestActor

역할:

- 서버 권위에서 자동 왕복 이동한다.
- 기본 `CFNetSmooth` 컴포넌트를 가진다.
- 움직이는 `Loc` 값이 `[CFNetSmooth][Send]`, `[CFNetSmooth][Receive]` 로그에 반영되는지 검증한다.

기본 설정:

```text
Replicates=true
Replicate Movement=false
Always Relevant=true
Received State Apply=true
MotionDistance=300
MotionSpeed=120
EnableTeleportTest=false
TeleportInterval=5
TeleportOffset=(0,600,0)
```

주의:

- 이 Actor는 테스트 전용이다.
- 실제 차량 Pawn이나 차량 네트워크 기준선에는 연결하지 않는다.
- Teleport 테스트 옵션은 기본 false이며, 켰을 때만 주기적으로 `MarkTeleport`를 호출한다.
- Dedicated 자동 검증에서는 저장된 맵 값을 바꾸지 않고 `-CFNetSmoothTeleportTest`, `-CFNetSmoothTeleportInterval=3.0` 명령줄 옵션으로 Teleport 테스트를 켤 수 있다.

---

## 5. 설정값 계약

### 5.1 SendRate

역할:

- 서버가 초당 State를 보내는 목표 횟수다.

기본값 후보:

```text
20.0
```

사유:

- 외부 Smooth Sync 문서는 30을 기본값으로 쓰지만, 차량 전투에서 대역폭과 안정성을 먼저 보기 위해 v0.1은 낮게 시작한다.

툴팁 제안:

```text
서버가 초당 Transform State를 보내는 목표 횟수입니다. 높을수록 부드럽지만 네트워크 사용량이 증가합니다.
```

### 5.2 InterpBackTime

역할:

- 클라이언트가 현재보다 약간 과거 시점을 표시해 State 누락을 완충한다.

기본값 후보:

```text
0.10
```

툴팁 제안:

```text
원격 표시를 몇 초 과거 기준으로 보간할지 정합니다. 값이 클수록 안정적이지만 반응이 늦어집니다.
```

### 5.3 MaxExtrapTime

역할:

- State가 부족할 때 최신 속도로 얼마나 미래까지 예측할지 제한한다.
- 값이 0 이하이면 외삽하지 않고 최신 State 위치에서 멈춘다.

기본값 후보:

```text
0.20
```

툴팁 제안:

```text
새 State가 늦게 도착했을 때 속도 기반 예측을 허용하는 최대 시간입니다. 0이면 외삽하지 않고 최신 State 위치에서 멈춥니다.
```

### 5.4 PosSnapDist

역할:

- 클라이언트 표시 위치와 보간 목표 위치의 차이가 이 값을 넘으면 보간하지 않고 즉시 맞춘다.
- 값이 0 이하이면 위치 Snap을 사용하지 않는다.

기본값 후보:

```text
500.0
```

툴팁 제안:

```text
원격 표시 위치가 목표 위치와 이 거리 이상 벌어지면 즉시 목표 위치로 이동합니다. 0이면 위치 Snap을 사용하지 않습니다.
```

### 5.5 RotSnapDeg

역할:

- 원격 표시 회전과 목표 회전의 각도 차이가 이 값을 넘으면 보간하지 않고 즉시 맞춘다.
- 값이 0 이하이면 회전 Snap을 사용하지 않는다.

기본값 후보:

```text
90.0
```

툴팁 제안:

```text
원격 표시 회전이 목표 회전과 이 각도 이상 벌어지면 즉시 목표 회전으로 맞춥니다. 0이면 회전 Snap을 사용하지 않습니다.
```

### 5.6 bEnableDebugLog

역할:

- 전송/수신/보간/Snap/Teleport 로그를 출력한다.

기본값 후보:

```text
false
```

툴팁 제안:

```text
CFNetSmooth의 송신, 수신, 보간, 스냅 로그를 출력합니다. 테스트가 끝나면 끄는 것을 권장합니다.
```

### 5.7 bApplyReceivedStateToOwner

역할:

- 클라이언트가 수신한 서버 State를 이 컴포넌트의 Owner 위치에 실제로 적용할지 정한다.

기본값 후보:

```text
false
```

사유:

- 기존 차량 Pawn이나 다른 Actor에 컴포넌트가 붙어도 자동으로 이동 로직을 덮어쓰지 않기 위함이다.
- `CFNetSmoothTestActor`에서만 명시적으로 켜서 보간 적용을 검증한다.

툴팁 제안:

```text
수신한 서버 State를 이 컴포넌트의 Owner 위치에 적용할지 정합니다. 기본값은 꺼짐이며 테스트 Actor에서만 켜는 것을 권장합니다.
```

---

## 6. 함수 계약

### 6.1 ForceSendNextState

역할:

- 다음 송신 주기에 변화량과 상관없이 State를 보낸다.

사용 시점:

- 리스폰 직후
- 차량 활성화 직후
- 큰 상태 전환 직후

툴팁 제안:

```text
다음 송신 주기에 Transform State를 강제로 전송합니다.
```

### 6.2 MarkTeleport

역할:

- 다음 State를 Teleport State로 표시한다.
- 클라이언트는 Teleport State 수신 시 기존 수신 Buffer를 비우고 새 State를 즉시 적용한다.

사용 시점:

- 리스폰
- 강제 위치 복구
- 레벨 경계 이탈 후 복귀

툴팁 제안:

```text
다음 Transform State를 보간 없이 즉시 적용해야 하는 Teleport State로 표시합니다.
```

### 6.3 ClearStateBuffer

역할:

- 클라이언트 수신 State Buffer를 비운다.
- 수신 여부, 마지막 수신 Sequence, 서버/로컬 시간 기준값도 함께 초기화한다.

사용 시점:

- 소유권 변경
- Actor 재활성화
- 큰 시간 역전 감지
- Teleport 적용 직후

툴팁 제안:

```text
수신된 Transform State 목록을 모두 지웁니다. 소유권 변경이나 리스폰 후 사용합니다.
```

### 6.4 SetSmoothingEnabled

역할:

- 보간 적용을 켜거나 끈다.

툴팁 제안:

```text
원격 Transform 보간 적용 여부를 변경합니다. 끄면 수신 State를 즉시 적용하는 방식으로 동작합니다.
```

### 6.5 SetReceivedStateApplyEnabled

역할:

- 수신한 서버 State를 Owner 위치에 실제로 적용할지 켜거나 끈다.

사용 시점:

- 테스트 Actor에서 위치 보간 적용을 검증할 때 켠다.
- 기존 이동/물리/차량 로직이 있는 Actor에는 기본적으로 켜지 않는다.

툴팁 제안:

```text
수신한 서버 Transform State를 이 Actor 위치에 실제로 적용할지 변경합니다. 기존 이동 로직이 있는 Actor에서는 신중하게 켜야 합니다.
```

### 6.6 TryGetSmoothedTransform

역할:

- 수신 Buffer를 기준으로 현재 표시해야 할 Transform을 계산해서 반환한다.
- Owner Actor를 직접 움직이지 않는다.
- 차량 적용 세션에서 원격 Visual/Shell에 Transform을 적용할 때 사용한다.

반환:

| 값 | 의미 |
|---|---|
| `true` | 보간 또는 제한 외삽으로 표시 Transform을 계산했다. |
| `false` | 아직 수신 State가 없거나 시간 기준이 없어 표시 Transform을 계산할 수 없다. |

출력:

| 출력값 | 역할 |
|---|---|
| `OutSmoothedTransform` | 외부 표시 대상에 적용할 위치/회전 Transform |
| `bOutUsedExtrapolation` | 이번 계산이 제한 외삽을 사용했는지 여부 |

툴팁 제안:

```text
Owner를 직접 움직이지 않고 현재 수신 Buffer 기준 보간 Transform을 반환합니다. 차량 Visual/Shell 적용 시 사용합니다.
```

### 6.7 HasReceivedState

역할:

- 클라이언트가 서버 Transform State를 하나 이상 수신했는지 반환한다.

툴팁 제안:

```text
클라이언트가 서버 Transform State를 하나 이상 수신했는지 반환합니다.
```

### 6.8 GetBufferedStateCount

역할:

- 현재 수신 Buffer에 저장된 State 개수를 반환한다.

툴팁 제안:

```text
현재 수신 Buffer에 저장된 Transform State 개수를 반환합니다.
```

### 6.9 GetLastReceivedSequence

역할:

- 마지막으로 수신한 State의 Sequence 번호를 반환한다.
- BP 호환을 위해 반환 타입은 `int32`를 사용한다.

툴팁 제안:

```text
마지막으로 수신한 Transform State의 Sequence 번호를 반환합니다.
```

### 6.10 IsUsingExtrapolation

역할:

- 최근 표시 Transform 계산이 제한 외삽 구간을 사용했는지 반환한다.

툴팁 제안:

```text
최근 표시 Transform 계산이 제한 외삽 구간을 사용 중인지 반환합니다.
```

### 6.11 WasLastReceivedStateTeleport

역할:

- 마지막으로 수신 처리한 State가 Teleport State였는지 반환한다.

툴팁 제안:

```text
마지막으로 수신 처리한 Transform State가 순간이동 State였는지 반환합니다.
```

---

## 7. RPC 계약

### 7.1 MulticastReceiveState

역할:

- 서버가 모든 클라이언트로 일반 Transform State를 전파한다.

현재 기준:

```text
Unreliable NetMulticast RPC
```

사유:

- Transform State는 최신 값이 중요하다.
- 오래된 Reliable 큐 적체가 더 위험할 수 있다.
- Owner가 없는 테스트 Actor와 원격 클라이언트 수신 검증까지 포함하려면 Client RPC보다 NetMulticast가 적합하다.

제외:

- `bTeleport=true` State는 `MulticastReceiveTeleportState`로 보낸다.

### 7.2 MulticastReceiveTeleportState

역할:

- 서버가 모든 클라이언트로 Teleport Transform State를 신뢰성 있게 전파한다.

현재 기준:

```text
Reliable NetMulticast RPC
```

사유:

- 20% 패킷 손실 조건에서 Unreliable Teleport State 누락이 확인됐다.
- Teleport는 최신 일반 State로 자연 복구되기보다 중간 경로 보간 잔상 방지가 더 중요하다.
- 일반 Transform State 전체를 Reliable로 바꾸면 큐 적체 위험이 있으므로 Teleport만 예외로 둔다.

### 7.3 HandleReceivedState

역할:

- Unreliable 일반 State와 Reliable Teleport State가 공유하는 클라이언트 수신 처리 함수다.
- 중복 Sequence 폐기, 오래된 ServerTime 폐기, Buffer 정렬, Buffer 최대 개수 제한, Teleport 즉시 적용을 한 곳에서 처리한다.
- 오래된 일반 State는 폐기하지만, 지연 도착한 Teleport State는 폐기하지 않는다.
- Teleport State 수신 시 Teleport 이전 Buffer State만 제거한다.
- 지연 도착 Teleport State를 처리할 때는 최신 시간 Anchor를 더 오래된 시간으로 되돌리지 않는다.

주의:

- 이 함수는 내부 C++ 구현 함수이며 BP 노출 대상이 아니다.
- RPC 진입점은 일반 State와 Teleport State를 분리하지만 수신 후 적용 규칙은 동일한 기준을 사용한다.

---

## 8. BP 노출 원칙

- BP는 설정값 튜닝과 테스트 호출만 담당한다.
- 핵심 보간, State 정렬, Snap 판정은 C++에 둔다.
- BP 노출 함수에는 한국어 툴팁을 반드시 작성한다.
- BP에서 서버 권위 판정을 우회하는 API는 만들지 않는다.
- Public API와 BP 노출 API에는 CarFight 전용 Pawn, VehicleMesh, GameMode, 맵 경로를 요구하지 않는다.
- 프로젝트별 적용 코드는 호스트 프로젝트의 Actor, Component, Blueprint에서 `CFNetSmooth` API를 호출하는 방식으로 분리한다.

---

## 9. Changelog

### v0.14.0

```text
- CFNetSmooth API 계약이 CarFight 전용이 아니라 여러 UE 프로젝트에서 재사용 가능한 범용 플러그인 기준임을 명시
- Public/BP API에 호스트 프로젝트 전용 타입, 맵, Build Target을 요구하지 않는 원칙 추가
```

### v0.13.0

```text
- 차량 Visual/Shell 적용을 위한 읽기 전용 공개 API 계약 추가
- TryGetSmoothedTransform, HasReceivedState, GetBufferedStateCount, GetLastReceivedSequence, IsUsingExtrapolation, WasLastReceivedStateTeleport 역할과 툴팁 추가
- SceneComponent 직접 지정 적용은 계속 보류하고, 외부 Visual/Shell 적용은 Transform getter 방식으로 진행하도록 기준 정리
```

### v0.12.0

```text
- 지연 도착 Teleport State 예외 처리 계약 추가
- Teleport 수신 시 이전 Buffer State 제거와 시간 Anchor 유지 규칙 추가
- 20% 손실 + 100ms 지연 조건에서 필요한 수신 순서 기준 보강
```

### v0.11.0

```text
- 일반 Transform State는 Unreliable NetMulticast, Teleport State는 Reliable NetMulticast로 분리하는 RPC 계약 추가
- MulticastReceiveTeleportState 계약 추가
- 공통 수신 처리 함수 HandleReceivedState 역할 추가
```

### v0.10.0

```text
- ACFNetSmoothTestActor의 Dedicated 자동 검증용 Teleport 명령줄 override 계약 추가
```

### v0.9.0

```text
- RotSnapDeg 기반 회전 Snap 계약 추가
- 회전 Snap은 Buffer를 비우지 않고 목표 Transform으로 즉시 보정한다는 기준 추가
- RotSnapDeg가 0 이하이면 회전 Snap을 비활성화한다는 기준 추가
```

### v0.8.0

```text
- State 사이 회전 보간 계약 추가
- 회전 보간은 FQuat::Slerp를 사용하고, 외삽 구간에서는 최신 회전을 유지한다는 기준 추가
- 회전 Snap과 회전 외삽은 후속 작업으로 분리
```

### v0.7.0

```text
- MaxExtrapTime 기반 제한 외삽 계약 추가
- MaxExtrapTime이 0 이하이면 최신 State 위치에서 멈춘다는 동작 명시
- [CFNetSmooth][Extrapolate] 로그 기준 추가
```

### v0.6.0

```text
- PosSnapDist를 목표 위치 기준 Snap 거리로 명확화
- PosSnapDist가 0 이하이면 위치 Snap을 비활성화한다는 계약 추가
- Snap 로그와 검증 기준을 위한 API 설명 보강
```

### v0.5.0

```text
- Teleport State 수신 시 Buffer Clear와 즉시 적용 계약 추가
- CFNetSmoothTestActor의 선택형 Teleport 테스트 설정값 추가
- ClearStateBuffer가 수신 시간 기준값까지 초기화한다는 계약 명시
```

### v0.4.0

```text
- bApplyReceivedStateToOwner 설정값 계약 추가
- SetReceivedStateApplyEnabled 함수 계약 추가
- 테스트 Actor 기본 설정에 Received State Apply=true 명시
```

### v0.3.0

```text
- CFNetSmoothTestActor 신규 파일 경로와 테스트 Actor 계약 추가
- 움직이는 서버 Transform 송수신 로그 검증용 Actor의 기본 설정 기록
```

### v0.2.0

```text
- Owner 없는 테스트 Actor와 원격 클라이언트 수신 검증을 위해 RPC 계약을 Unreliable Client RPC에서 Unreliable NetMulticast RPC로 변경
- 예정 함수명을 ClientReceiveState에서 MulticastReceiveState로 갱신
```

### v0.1.1

```text
- UHT가 uint16 BlueprintReadWrite 필드를 허용하지 않아 Sequence를 BP 노출 대상에서 제외하도록 계약 보강
```

### v0.1.0

```text
- CFNetSmooth v0.1 API 계약 최초 작성
- 예정 파일 구조, State 필드, 컴포넌트 책임, 설정값, 함수, RPC 후보 정의
- BP 툴팁 문구 초안 추가
```

---

## 10. 마이그레이션 지침

- 이 API 계약은 기존 차량 클래스의 함수 시그니처를 변경하지 않는다.
- 실제 구현 시 `Public/Private` 경로 규칙을 지킨다.
- 기존 `ACFVehiclePawn`에 직접 API를 추가하기 전에 플러그인 테스트 Actor에서 먼저 검증한다.
- 수신 State의 Owner 적용은 기본 false로 유지하고, 검증 목적의 `CFNetSmoothTestActor`에서만 true로 시작한다.
- 차량 Visual/Shell 적용은 `bApplyReceivedStateToOwner=false`를 유지하고 `TryGetSmoothedTransform` 결과를 외부 표시 대상에 적용한다.
- 일반 Transform State를 Reliable로 바꾸지 않는다. Teleport State만 Reliable 예외로 유지한다.
- 회전 Snap 테스트가 끝나면 `RotSnapDeg`를 기본값 90.0으로 되돌린다.
- 다른 프로젝트로 이식할 때도 게임 전용 타입을 CFNetSmooth Public API에 추가하지 않는다.
