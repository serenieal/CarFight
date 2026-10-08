# CFNetSmooth 설계서

- 문서 버전: v0.3.0
- 작성일: 2026-06-18
- 대상: 범용 Unreal Engine 프로젝트, 현재 검증 호스트 CarFight
- 작업 유형: 신규
- 상태: Draft

---

## 1. 목적

`CFNetSmooth`는 여러 Unreal Engine 프로젝트에서 원격 Actor 또는 표시 대상의 Transform을 부드럽게 표시하기 위한 자체 Unreal Engine 런타임 플러그인이다.

목표는 외부 Smooth Sync와 같은 범용 플러그인을 그대로 재현하는 것이 아니라, 서버 권위 네트워크 게임에서 재사용 가능한 최소 Transform 표시 보간 기능을 안전하게 구현하는 것이다.

CarFight는 현재 검증 호스트 프로젝트다.
플러그인 설계와 구현은 CarFight 전용 타입, 맵, Pawn, GameMode, Build Target에 의존하지 않아야 한다.

---

## 2. 범위

### 2.1 v0.1 포함 범위

- 서버 권위 Transform State 송신
- 클라이언트 수신 State Buffer
- 위치 보간
- 회전 보간
- 제한 외삽
- Snap Threshold
- Teleport State
- Buffer Clear
- 디버그 로그

### 2.2 v0.1 제외 범위

- 클라이언트 권위 Transform 송신
- 차량 물리 예측
- Network Physics Prediction 대체
- CharacterMovementComponent 대체
- Chaos Vehicle Movement 직접 수정
- Half-float 압축
- 축별 SyncMode
- 다중 SceneComponent 동기화
- 안티치트 검증
- 롤백/리플레이

---

## 3. 핵심 설계 원칙

### 3.1 프로젝트 독립성 우선

`CFNetSmooth`는 CarFight 전용 플러그인이 아니다.

허용되는 기준:

```text
Unreal Engine Runtime 모듈 의존성만 사용
AActor 또는 외부 표시 대상 Transform 기준 API 제공
게임 프로젝트 모듈 의존성 금지
CarFight 전용 클래스명, 맵 경로, Pawn 구조를 플러그인 코드에 직접 참조하지 않음
CarFight는 검증 호스트와 예시 적용 프로젝트로만 사용
```

금지되는 기준:

```text
CarFight_Re 모듈을 Build.cs 의존성에 추가
ACFVehiclePawn 같은 게임 전용 타입을 플러그인 Public API에 노출
/Game/Maps/TestMap 같은 호스트 프로젝트 경로를 플러그인 내부 기본값으로 고정
차량 전용 물리 정책을 플러그인 기본 동작으로 강제
```

문서에서 CarFight를 언급하는 경우는 현재 검증 결과, 적용 예시, 차량 적용 계획을 설명할 때로 제한한다.

### 3.2 서버 권위 우선

기본 설계는 서버가 최종 위치, 회전, 충돌 판정의 기준이 되는 서버 권위 구조를 우선한다.

CarFight처럼 차량 전투와 충돌 판정이 중요한 프로젝트에서는 이 원칙이 특히 중요하다.

따라서 v0.1에서는 다음 구조만 허용한다.

```text
Server Authority Actor
-> Transform State 생성
-> Client Simulated Proxy 수신
-> Client 표시 보간
```

클라이언트가 자기 Transform을 서버로 보내고 다른 클라이언트가 그대로 신뢰하는 구조는 v0.1에서 금지한다.

### 3.3 실제 판정 위치와 표시 위치 분리

차량 전투에서는 표시 부드러움보다 판정 일관성이 우선이다.

권장 구조는 다음이다.

```text
Actor Transform:
- 서버 권위 판정 기준
- 충돌/대미지/리스폰 기준

Visual Transform:
- 원격 클라이언트 표시용
- 보간/외삽 적용 가능
```

v0.1 테스트에서는 Actor 자체를 직접 움직일 수 있지만, 실제 차량 적용 단계에서는 Visual/Shell 분리를 우선 검토한다.

### 3.4 검증 호스트 기준선과 CFNetSmooth 적용 모드 분리

CFNetSmooth 자체는 범용 플러그인이지만, 현재 검증 호스트인 CarFight에서는 차량 네트워크 기준선을 아래 상태로 둔다.

```text
RepMove=true
AutoPhysics=true
PhysicsPrediction=false
PhysicsReplicationMode=Default
```

`CFNetSmooth` 문서나 실험은 이 기준선을 말 없이 바꾸지 않는다.

다만 CFNetSmooth가 실제 차량 Transform 복제 역할을 맡는 적용 단계에서는 아래 전환이 필수다.

```text
RepMove=false
CFNetSmooth가 서버 권위 Transform State 송수신과 원격 표시 보간을 담당
```

즉 `RepMove=true`는 플러그인 연결 전 기준선 또는 롤백 기준선이고, `RepMove=false`는 CFNetSmooth가 Replicate Movement 역할을 대신하는 적용 기준이다.

---

## 4. 구성 요소

### 4.1 플러그인

예정 경로:

```text
UE/Plugins/CFNetSmooth
```

예정 모듈:

```text
CFNetSmooth
```

역할:

- 네트워크 Transform State 구조체 제공
- ActorComponent 기반 보간 컴포넌트 제공
- 테스트와 디버그 API 제공

### 4.2 State 구조체

예정 이름:

```text
FCFNetSmoothState
```

역할:

- 서버가 보낸 위치/회전/속도/시간 정보를 담는다.
- 텔레포트 여부를 포함한다.
- 수신 측 Buffer에서 보간 기준으로 사용된다.

### 4.3 Component 클래스

예정 이름:

```text
UCFNetSmoothComp
```

역할:

- 서버에서 State 송신 조건을 판단한다.
- 클라이언트에서 State Buffer를 관리한다.
- 클라이언트 Tick에서 보간/외삽/Snap을 적용한다.

파일명/클래스명은 32자 이하 규칙을 지키기 위해 짧게 유지한다.

---

## 5. 데이터 흐름

### 5.1 서버 송신 흐름

```text
1. 서버 Tick에서 SendRate 간격 확인
2. 대상 Transform 읽기
3. 위치/회전/속도/각속도 State 생성
4. 마지막 송신 State와 비교
5. 변화가 있거나 강제 송신이면 Unreliable NetMulticast RPC로 전파
```

### 5.2 클라이언트 수신 흐름

```text
1. NetMulticast RPC로 State 수신
2. Teleport State면 Buffer 정리 후 즉시 적용
3. 일반 State면 시간순 Buffer에 삽입
4. Buffer 크기 제한 초과분 제거
```

### 5.3 클라이언트 표시 흐름

```text
1. 현재 클라이언트 시간에서 InterpBackTime을 뺀 목표 시간을 계산
2. 목표 시간을 감싸는 이전/다음 State를 찾음
3. 두 State가 있으면 위치/회전 보간
4. 최신 State만 있으면 제한 외삽
5. 오차가 SnapThreshold를 넘으면 즉시 Snap
6. 결과 Transform을 표시 대상에 적용
```

---

## 6. 제약

- v0.1은 서버 권위 Actor에만 적용한다.
- v0.1은 원격 표시 품질 검증이 목적이며 차량 물리 해결책이 아니다.
- CharacterMovementComponent, Chaos Vehicle Movement의 내부 네트워크 구조를 대체하지 않는다.
- 클라이언트 예측, 입력 재전송, 서버 보정은 별도 시스템으로 분리한다.
- 기존 차량 Pawn에 바로 적용하지 않고 테스트 전용 Actor부터 시작한다.
- 플러그인 Runtime 모듈은 특정 게임 프로젝트 모듈에 의존하지 않는다.
- CarFight 전용 경로와 클래스는 플러그인 코드가 아니라 검증 스크립트와 적용 문서에서만 다룬다.

---

## 7. 미결 사항

- 실제 차량 적용 시 Actor Transform을 보간할지 Visual/Shell만 보간할지 결정 필요
- Chaos Vehicle 원격 표시와 WheelSync의 책임 분리 필요
- 서버 State를 Reliable RPC로 보낼지 Unreliable RPC로 보낼지 테스트 필요
- SendRate와 NetUpdateFrequency의 관계를 프로젝트 기준으로 측정 필요
- 큰 충돌/리스폰/탑승 전환 시 Buffer Clear 호출 위치 결정 필요

---

## 8. 완료 기준

v0.1 설계 완료 조건은 다음이다.

- 클래스 책임이 문서화되어 있다.
- 금지 범위가 명확하다.
- 테스트 Actor 기준 데이터 흐름이 설명되어 있다.
- 실제 차량 적용 전 필요한 검증 단계가 분리되어 있다.

---

## 9. Changelog

### v0.3.0

```text
- CFNetSmooth가 CarFight 전용이 아니라 여러 UE 프로젝트에서 재사용될 범용 런타임 플러그인임을 목적과 설계 원칙에 명시
- CarFight는 검증 호스트 프로젝트이며 플러그인 코드가 CarFight 전용 타입, 맵, Build Target에 의존하지 않아야 한다는 기준 추가
- 프로젝트 독립성 우선 원칙과 금지 기준을 추가
```

### v0.2.1

```text
- 기존 RepMove=true 기준선과 CFNetSmooth 실제 적용 모드를 분리
- CFNetSmooth가 Replicate Movement 역할을 대신하는 경우 RepMove=false가 필수임을 명시
```

### v0.2.0

```text
- 서버 송신/클라이언트 수신 로그 구현에 맞춰 Client RPC 표현을 Unreliable NetMulticast RPC로 갱신
```

### v0.1.0

```text
- CFNetSmooth 최초 설계서 작성
- 서버 권위 Transform State 보간 구조 정의
- v0.1 포함/제외 범위와 금지 범위 명시
- 외부 Smooth Sync 개념을 CarFight 자체 구현 요구사항으로 변환
```

---

## 10. 마이그레이션 지침

- 이 설계는 기존 차량 네트워크 코드를 즉시 교체하지 않는다.
- 기존 `VehicleNetSyncPlan`의 현재 기준선을 유지한 상태에서 별도 플러그인으로 실험한다.
- 실제 차량 Pawn에 붙이기 전 테스트 Actor에서 Dedicated Server 검증을 완료한다.
- 다른 프로젝트로 옮길 때는 `UE/Plugins/CFNetSmooth` 폴더 단위로 복사하고, 게임 프로젝트 모듈 의존성을 추가하지 않는 것을 기본으로 한다.
- CarFight 전용 검증 스크립트와 RuntimeLogs는 플러그인 필수 구성물로 보지 않는다.
