# CFNetSmoothPlan 문서 인덱스

- 문서 버전: v0.3.2
- 작성일: 2026-06-18
- 대상 프로젝트: CarFight
- 관련 영역: 차량 네트워크 Transform 보간 / Smooth Sync류 자체 플러그인 설계
- 작업 유형: 신규
- 상태: Draft

---

## 1. 목적

이 폴더는 외부 Smooth Sync 문서를 참고해 여러 Unreal Engine 프로젝트에서 재사용 가능한 네트워크 Transform 스무딩 플러그인 `CFNetSmooth`를 직접 만들기 위한 설계, 실행 계획, API 계약, 검증 기준을 관리한다.

핵심 방향은 다음과 같다.

```text
외부 Smooth Sync 원본을 그대로 복제하지 않는다.
CarFight는 현재 검증 호스트이며, 플러그인 자체는 CarFight 전용으로 만들지 않는다.
서버 권위 네트워크 Actor 표시 보간에 필요한 최소 기능만 자체 구현한다.
게임 프로젝트 모듈, Pawn, 맵, GameMode에 직접 의존하지 않는다.
```

---

## 2. 배경

외부 Smooth Sync 문서에서 확인한 핵심 개념은 다음이다.

- 송신 측 Transform State 생성
- 수신 측 State Buffer 저장
- `InterpolationBackTime` 기준 과거 시점 보간
- State 부족 시 제한 외삽
- 거리/각도 오차가 크면 Snap
- 리스폰/순간이동은 Teleport State로 처리
- Ownership 변경 시 Buffer Clear 필요

CarFight에서는 차량 전투, 충돌, 서버 권위 싱크가 중요하므로 클라이언트 권위 Transform 전송을 기본안으로 삼지 않는다.
다른 프로젝트에서도 기본 정책은 서버 권위 수신 표시 보간이며, 프로젝트별 적용 방식은 외부 코드나 Blueprint에서 결정한다.

---

## 3. 문서 목록

| 문서 | 역할 |
|---|---|
| `Design.md` | CFNetSmooth의 목적, 범위, 구조, 책임 분리, 금지 범위 |
| `Plan.md` | v0.1부터 v0.4까지 구현 단계와 완료 조건 |
| `ImplementChecklist.md` | v0.1 플러그인 뼈대부터 첫 검증까지의 실행 체크리스트 |
| `Api.md` | 예정 클래스, 구조체, BP 노출 API, 설정값 계약 |
| `Presets.md` | 테스트 Actor용 기본값, 스트레스 테스트값, 복구값 프리셋 |
| `Verify.md` | PIE, Dedicated Server, 네트워크 지연/손실 검증 절차 |
| `TestResults.md` | 테스트 Actor 기준 실제 검증 결과와 남은 검증 항목 |
| `Decisions.md` | 확정 결정, 미결 사항, 재검토 조건 |
| `VehiclePrep.md` | 실제 차량 적용 전 차량 코드 정리, Transform 책임 분리, 플러그인 수정 세션 분리 기준 |
| `ReleaseCandidate.md` | CFNetSmooth v1.0 후보 판정, 반복 검증 중단 기준, 차량 적용 세션 인계 기준 |
| `Repository.md` | NetSmoothSync 독립 레포와 CarFight 서브모듈 등록 기준 |

---

## 4. 권장 읽기 순서

처음 읽는 세션은 아래 순서를 따른다.

```text
1. README.md
2. ReleaseCandidate.md
3. Design.md
4. Api.md
5. Plan.md
6. ImplementChecklist.md
7. Presets.md
8. Verify.md
9. TestResults.md
10. Decisions.md
11. VehiclePrep.md
12. Repository.md
```

기존 차량 네트워크 상태와 맞춰 봐야 할 때는 아래 문서를 추가로 읽는다.

```text
Document/Plan/VehicleNetSyncPlan/README.md
Document/Plan/VehicleNetSyncPlan/VehicleNetSyncRecoveryPlan.md
Document/Plan/CFNetSmoothPlan/VehiclePrep.md
```

---

## 5. 현재 최우선 결론

`CFNetSmooth`는 당장 기존 차량 네트워크 문제의 최종 해결책으로 확정하지 않는다.

우선순위는 다음이다.

```text
1. CFNetSmooth를 차량에 붙이기 전에는 기존 Default + RepMove=true + AutoPhysics=true 기준선을 유지한다.
2. CFNetSmooth가 실제 차량 Transform 복제 역할을 맡는 순간에는 Replicate Movement=false로 전환한다.
3. 이 세션에서는 CFNetSmooth 플러그인 자체 완성에 집중한다.
4. CFNetSmooth는 별도 플러그인/별도 테스트 Actor에서 검증한다.
5. 실제 차량 Pawn, 차량 BP, RepMove, AutoPhysics, Visual/Shell 연결은 별도 차량 적용 세션에서 진행한다.
6. 차량 적용 세션은 `VehiclePrep.md`를 먼저 읽고 차량 Transform 책임 충돌을 제거한 뒤 CFNetSmooth를 연결한다.
7. 플러그인 Runtime 코드는 CarFight 전용 클래스, 맵, Build Target에 의존하지 않는다.
```

현재 세션의 완료 목표는 다음이다.

```text
1. 테스트 Actor 기준 기능 검증을 충분히 통과시킨다.
2. 플러그인 설정값, 검증 결과, 미결 사항을 문서화한다.
3. 다른 세션에서 가져다 적용할 수 있는 완성된 Smooth Sync류 플러그인 후보 상태로 만든다.
4. 같은 조건의 반복 검증은 중단하고 ReleaseCandidate.md 기준으로 기능 동결 상태를 유지한다.
```

현재 추가 완료 상태는 다음이다.

```text
일반 Transform State는 Unreliable NetMulticast로 유지한다.
Teleport State는 Reliable NetMulticast로 분리한다.
20% 손실 + Teleport 자동 검증은 통과했다.
20% 손실 + 100ms 지연 + Teleport 자동 검증은 Sequence 누락 0건으로 통과했다.
20% 손실 + 100ms 지연 + Teleport 자동 검증은 총 3회 반복에서도 각 클라이언트 첫 Teleport 이후 Sequence 누락 0건으로 통과했다.
20% 손실 + 100ms 지연 일반 Transform 자동 검증은 Snap 0회, 최대 외삽 0.001초로 통과했다.
100ms 지연 일반 Transform 자동 검증은 Snap 0회, 최대 외삽 0.006초로 통과했다.
20% 손실 일반 Transform 자동 검증은 Snap 0회, 최대 외삽 0.007초로 통과했다.
Win64 Development Game 패키징과 패키지 스모크 실행은 통과했다.
Win64 Development Server 패키징과 서버 패키지 스모크 실행은 통과했다.
Win64 Development 패키지 서버 1개 + 패키지 클라이언트 2개 접속에서 CFNetSmooth 송수신은 통과했다.
Win64 Development 패키지 가시 클라이언트 최소 렌더링/접속/송수신 검증은 통과했다.
CFNetSmooth는 테스트 Actor 기준 v1.0 Release Candidate로 정리했다.
코드/기본값/환경 변경이 없으면 같은 조건의 반복 검증은 진행하지 않는다.
CFNetSmooth 0.12.0에서 차량 Visual/Shell용 읽기 전용 보간 Transform API를 추가했다.
CFNetSmooth 0.12.1에서 플러그인 manifest와 Build.cs 설명을 범용 UE 런타임 플러그인 기준으로 정리했다.
CFNetSmooth는 GitHub `serenieal/NetSmoothSync` 독립 레포로 분리했고, CarFight에는 `UE/Plugins/CFNetSmooth` 서브모듈로 등록했다.
NetSmoothSync `v0.12.1` 태그를 생성했고, 새 클론 기준 서브모듈 초기화 검증을 통과했다.
CFNetSmooth 0.12.0 API 추가 후 BuildEditor는 통과했고, Dedicated 로그 0건은 DebugLog 조건 확인 대상으로 분리했다.
```

---

## 6. Changelog

### v0.3.2

```text
- NetSmoothSync v0.12.1 태그 생성과 새 클론 기준 서브모듈 초기화 검증 통과 상태 반영
```

### v0.3.1

```text
- Repository.md 문서 추가
- NetSmoothSync 독립 GitHub 레포와 CarFight UE/Plugins/CFNetSmooth 서브모듈 등록 상태를 인덱스에 반영
```

### v0.3.0

```text
- CFNetSmooth가 CarFight 전용이 아니라 여러 UE 프로젝트에서 재사용될 범용 플러그인임을 인덱스에 명시
- CarFight는 현재 검증 호스트이며 플러그인 Runtime 코드는 게임 프로젝트 전용 클래스, 맵, Build Target에 의존하지 않는다는 원칙 추가
- CFNetSmooth 0.12.1 manifest/Build.cs 범용화 상태 반영
```

### v0.2.9

```text
- CFNetSmooth 0.12.0 API 추가 후 BuildEditor 통과 상태 추가
- Dedicated 스모크 Send/Receive 0건은 플러그인 실패가 아니라 DebugLog 조건 확인 대상으로 분리한 상태 반영
```

### v0.2.8

```text
- CFNetSmooth 0.12.0 Visual/Shell용 읽기 전용 보간 Transform API 추가 상태 반영
- 차량 세션이 Owner 직접 적용 없이 TryGetSmoothedTransform 결과를 별도 표시 대상에 적용할 수 있는 기준 추가
```

### v0.2.7

```text
- ReleaseCandidate.md 문서 추가
- CFNetSmooth를 테스트 Actor 기준 v1.0 Release Candidate로 정리
- 같은 조건의 반복 검증을 중단하고 코드/기본값/환경 변경 시에만 추가 검증하는 기준을 인덱스에 반영
- 처음 읽는 세션이 ReleaseCandidate.md를 먼저 보도록 읽기 순서 갱신
```

### v0.2.6

```text
- CFNetSmooth 실제 차량 적용 시 Replicate Movement=false가 필수임을 현재 최우선 결론에 명시
- 기존 RepMove=true 기준선은 CFNetSmooth 연결 전/롤백 기준선임을 분리
```

### v0.2.5

```text
- VehiclePrep.md 문서 추가
- 실제 차량 적용 전 차량 코드 정리와 Transform 책임 분리 문서를 문서 목록/읽기 순서/현재 결론에 반영
- CFNetSmooth 플러그인 수정은 별도 제작 세션으로 넘기는 기준을 인덱스에 반영
```

### v0.2.4

```text
- Win64 Development 패키지 가시 클라이언트 최소 검증 통과 상태 추가
- NullRHI 없는 패키지 클라이언트 창 실행, 화면 캡처, CFNetSmooth Receive 확인을 현재 결론에 반영
```

### v0.2.3

```text
- Win64 Development 패키지 Server-Client 접속 검증 통과 상태 추가
- 패키지 서버 1개와 패키지 클라이언트 2개 환경에서 CFNetSmooth Send/Receive가 동작한 것을 현재 결론에 반영
```

### v0.2.2

```text
- Win64 Development Server 패키징과 서버 패키지 스모크 실행 통과 상태 추가
- CFNetSmooth가 패키징된 Dedicated Server 실행 시 프로젝트 플러그인으로 Mount되고 GameNetDriver가 시작되는 것을 현재 결론에 반영
```

### v0.2.1

```text
- Win64 Development Game 패키징과 패키지 스모크 실행 통과 상태 추가
- CFNetSmooth가 패키지 실행 시 프로젝트 플러그인으로 Mount되는 것을 현재 결론에 반영
```

### v0.2.0

```text
- Teleport 손실 20% + 지연 100ms 자동 검증 3회 반복 통과 상태 추가
- 각 클라이언트 첫 Teleport 이후 Sequence 누락 0건 기준을 현재 인덱스 결론에 반영
```

### v0.1.9

```text
- 일반 Transform 20% 손실 단독 자동 검증 통과 상태 추가
- 테스트 Actor 기준 기본 보간값 유지 근거를 일반 Transform 악조건 전체로 확장
```

### v0.1.8

```text
- 일반 Transform 100ms 지연 단독 자동 검증 통과 상태 추가
- 테스트 Actor 기준 기본 보간값 유지 근거를 지연 단독 조건까지 확장
```

### v0.1.7

```text
- 일반 Transform 20% 손실 + 100ms 지연 자동 검증 통과 상태 추가
- 테스트 Actor 기준 기본 보간값 유지 결정을 인덱스에 반영
```

### v0.1.6

```text
- 20% 손실 + 100ms 지연 + Teleport 복합 검증 통과 상태 추가
- 지연 도착 Teleport State 보강 완료 상태를 인덱스에 반영
```

### v0.1.5

```text
- Teleport State Reliable NetMulticast 분리 완료 상태 추가
- 20% 손실 + Teleport 자동 검증 통과 상태를 인덱스에 반영
```

### v0.1.4

```text
- 현재 세션의 범위를 CFNetSmooth 플러그인 완성으로 고정
- 차량 Pawn, 차량 BP, RepMove, AutoPhysics, Visual/Shell 연결은 별도 차량 적용 세션에서 진행하도록 명시
```

### v0.1.3

```text
- TestResults.md 문서 추가
- 테스트 Actor 기준 실제 검증 결과와 남은 검증 항목을 문서 목록과 읽기 순서에 포함
```

### v0.1.2

```text
- Presets.md 문서 추가
- 테스트 Actor용 기본값, 스트레스 테스트값, 복구값 문서를 문서 목록과 읽기 순서에 포함
```

### v0.1.1

```text
- ImplementChecklist.md 문서 추가
- 구현 직전 실행 체크리스트를 읽기 순서에 포함
- 작업 완료 보고에 남은 작업/다음 작업 추천을 포함하도록 기준 보강
```

### v0.1.0

```text
- CFNetSmoothPlan 폴더 인덱스 최초 작성
- 외부 Smooth Sync 분석 결과를 CarFight 자체 플러그인 계획으로 분리
- 설계/계획/API/검증/결정 문서 구조 정의
```

---

## 7. 마이그레이션 지침

- 기존 `VehicleNetSyncPlan`은 현재 차량 네트워크 문제 추적 문서로 유지한다.
- `CFNetSmoothPlan`은 새 자체 플러그인 후보 설계 문서로만 사용한다.
- 기존 차량 Pawn, BP, RepMove, AutoPhysics 설정을 이 문서만 보고 즉시 변경하지 않는다.
- CFNetSmooth가 실제 차량 Transform 복제 역할을 맡는 적용 단계에서는 `VehiclePrep.md` 기준으로 Replicate Movement=false 전환을 별도 패치로 진행한다.
- 다른 프로젝트로 옮길 때는 플러그인 Runtime 코드와 CarFight 검증 스크립트/로그/맵 의존성을 분리해서 취급한다.
