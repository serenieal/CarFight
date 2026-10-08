# CFNetSmooth 결정 기록

- 문서 버전: v0.24.0
- 작성일: 2026-06-18
- 대상 프로젝트: CarFight
- 작업 유형: 신규
- 상태: Draft

---

## 1. 목적

이 문서는 `CFNetSmooth` 설계와 구현 과정에서 확정한 결정, 미결 사항, 재검토 조건을 기록한다.

다음 세션이나 다른 AI가 읽어도 같은 방향으로 판단할 수 있게 유지한다.

---

## 2. 확정 결정

### D-001. 외부 Smooth Sync 원본 복제 금지

- 결정: 외부 Smooth Sync 문서의 개념은 참고하되 원본 구조/API를 그대로 복제하지 않는다.
- 사유: 라이선스/구현 의존을 피하고 서버 권위 네트워크 게임에서 필요한 최소 기능만 안전하게 만들기 위함이다.
- 영향: 문서와 코드 이름은 `CFNetSmooth` 자체 이름을 사용하고, 특정 게임 프로젝트 전용 이름을 플러그인 Runtime API에 넣지 않는다.

### D-001A. CFNetSmooth는 범용 UE 플러그인으로 유지

- 결정: `CFNetSmooth`는 CarFight 전용 플러그인이 아니라 다른 Unreal Engine 프로젝트에서도 재사용 가능한 Runtime 플러그인으로 유지한다.
- 사유: 사용자 요구사항상 이 플러그인은 현재 프로젝트에만 쓰이는 물건이 아니며, 특정 게임 타입에 묶이면 이후 재사용과 검증 범위가 깨지기 때문이다.
- 영향: `UE/Plugins/CFNetSmooth` 내부 Runtime 코드는 `CarFight_Re`, `ACFVehiclePawn`, `/Game/Maps/TestMap` 같은 호스트 프로젝트 전용 모듈, 타입, 경로에 의존하지 않는다. CarFight는 현재 검증 호스트와 적용 예시로만 취급한다.

### D-002. v0.1은 서버 권위만 지원

- 결정: v0.1에서는 서버가 Transform State를 보내고 클라이언트는 표시 보간만 한다.
- 사유: 차량 전투에서 클라이언트 권위 Transform은 치트와 판정 불일치 위험이 크다.
- 영향: Owner Client가 Transform을 직접 보내는 구조는 보류한다.

### D-003. 기존 차량 기준선 보호

- 결정: CFNetSmooth v0.1/v0.2는 기존 `ACFVehiclePawn`, `BP_CFVehiclePawn`, RepMove, AutoPhysics 설정을 변경하지 않는다.
- 사유: 현재 차량 네트워크 문제 추적 기준선을 잃지 않기 위함이다.
- 영향: 테스트 Actor에서 먼저 검증한다.

### D-004. 실제 차량 적용 전 Visual/Shell 분리 검토

- 결정: 실제 차량 적용 시 Actor Transform 직접 보간보다 Visual/Shell 보간을 우선 검토한다.
- 사유: 서버 권위 판정 위치와 클라이언트 표시 위치를 분리해야 안전하다.
- 영향: v0.3에서만 차량 표시 연결을 검토한다.

### D-005. Transform State 전송은 Unreliable 우선

- 결정: 일반 Transform State는 Unreliable 전송을 우선 검토한다.
- 사유: Transform State는 최신 값이 중요하며 오래된 Reliable 큐 적체가 더 위험할 수 있다.
- 영향: Teleport State 누락 문제가 확인되면 별도 Reliable 보조 경로를 검토한다.

### D-006. v0.2 수신 로그 단계는 Unreliable NetMulticast 사용

- 결정: 서버 송신/클라이언트 수신 로그 단계에서는 `Unreliable NetMulticast`를 사용한다.
- 사유: Owner가 없는 테스트 Actor와 원격 클라이언트 수신 여부를 먼저 확인해야 하므로, 소유 클라이언트 중심의 Client RPC보다 NetMulticast가 검증 범위에 맞다.
- 영향: 실제 보간 단계에서도 모든 원격 클라이언트 표시를 목표로 할 경우 NetMulticast 유지 여부를 우선 검토한다.

### D-007. 수신 State Owner 적용은 기본 false

- 결정: `CFNetSmooth` 컴포넌트가 수신한 State를 Owner Transform에 적용하는 기능은 기본값을 false로 둔다.
- 사유: 기존 차량 Pawn, 물리 Actor, 다른 복제 Actor의 이동 로직을 컴포넌트 부착만으로 덮어쓰지 않기 위함이다.
- 영향: `CFNetSmoothTestActor`에서만 명시적으로 true로 켜서 위치 보간을 검증한다.

### D-008. Teleport State는 Buffer를 버리고 즉시 적용

- 결정: Teleport State를 수신하면 기존 수신 Buffer를 비우고 Teleport State 1개만 남긴다.
- 사유: 순간이동 직전의 오래된 State가 다시 보간에 사용되면 중간 경로 잔상이 생기기 때문이다.
- 영향: `bApplyReceivedStateToOwner=true`인 테스트 Actor는 즉시 위치/회전을 맞추고, 기본 컴포넌트는 Buffer만 초기화한다.

### D-009. 위치 Snap은 Buffer를 유지하고 표시 위치만 즉시 보정

- 결정: Teleport가 아닌 위치 오차 Snap은 수신 Buffer를 비우지 않고 Owner 표시 위치만 목표 위치로 즉시 맞춘다.
- 사유: 일반 네트워크 흔들림에서 Buffer를 계속 비우면 보간 기준이 불안정해질 수 있기 때문이다.
- 영향: `PosSnapDist`가 0 이하이면 위치 Snap을 끄고, 기본값 500에서는 큰 오차에만 보정한다.

### D-010. 외삽은 최신 State 속도로 MaxExtrapTime까지만 허용

- 결정: 목표 시간이 최신 State보다 미래이면 최신 State의 `LinearVelocity`로 위치를 예측하되 `MaxExtrapTime`까지만 허용한다.
- 사유: State 누락 시 짧은 끊김은 완충하되, 오래된 속도로 무한히 미끄러지는 문제를 막기 위함이다.
- 영향: `MaxExtrapTime`이 0 이하이면 외삽하지 않고 최신 State 위치에서 멈춘다.

### D-011. 회전은 저장은 FRotator, 보간은 FQuat Slerp

- 결정: State에는 기존처럼 `FRotator`를 저장하되, State 사이 회전 보간은 `FQuat::Slerp`로 수행한다.
- 사유: 저장 구조를 크게 바꾸지 않으면서도 긴 회전 경로와 보간 튐을 줄이기 위함이다.
- 영향: 회전 외삽은 아직 하지 않으며, 최신 State 이후 구간에서는 최신 회전을 유지한다.

### D-012. 회전 Snap은 Buffer를 유지하고 표시 Transform만 즉시 보정

- 결정: Teleport가 아닌 회전 오차 Snap은 수신 Buffer를 비우지 않고 Owner 표시 Transform만 목표 Transform으로 즉시 맞춘다.
- 사유: 일반 회전 오차에서 Buffer를 비우면 시간 기준이 흔들릴 수 있고, 위치 Snap과 동일하게 표시 보정만 하는 편이 안정적이다.
- 영향: `RotSnapDeg`가 0 이하이면 회전 Snap을 끄고, 기본값 90에서는 큰 회전 오차에만 보정한다.

### D-013. 이 세션은 플러그인 완성에 집중하고 차량 적용은 별도 세션에서 진행

- 결정: 현재 세션에서는 `CFNetSmooth` 플러그인 자체 완성, 테스트 Actor 검증, 문서 정리에 집중한다.
- 사유: 플러그인 구현 안정화와 실제 차량 통합을 한 세션에서 섞으면 기준선, 원인 추적, 롤백 범위가 불명확해질 수 있기 때문이다.
- 영향: 차량 Pawn, 차량 BP, RepMove, AutoPhysics, 차량 Visual/Shell 연결은 이 세션의 직접 작업 범위에서 제외한다. 완성된 플러그인을 다른 세션에서 이 프로젝트 차량 구조에 적용한다.

### D-014. Teleport State만 Reliable NetMulticast로 전송

- 결정: 일반 Transform State는 `Unreliable NetMulticast`를 유지하고, `bTeleport=true` State만 `Reliable NetMulticast`로 분리한다.
- 사유: 20% 패킷 손실 테스트에서 Unreliable Teleport State 누락이 확인됐고, Teleport는 최신 일반 State보다 이벤트 누락 방지가 더 중요하기 때문이다.
- 영향: 평상시 위치/회전 State는 큐 적체 위험을 줄이기 위해 기존 경로를 유지한다. 리스폰/강제 복구/순간이동 State는 별도 Reliable 경로로 전달한다.

### D-015. 지연 도착 Teleport State는 오래된 일반 State처럼 폐기하지 않음

- 결정: `bTeleport=true` State는 현재 수신 Anchor보다 오래된 `ServerTime`으로 도착해도 폐기하지 않는다.
- 사유: 20% 손실 + 100ms 지연 조건에서 Reliable Teleport가 최신 일반 State 뒤에 도착해 기존 시간 폐기 조건에 걸리는 문제가 확인됐기 때문이다.
- 영향: Teleport State 수신 시 Teleport 이전 Buffer State만 제거하고, 최신 시간 Anchor는 더 오래된 Teleport 시간으로 되돌리지 않는다.
- 반복 근거: `teleportloss 3.0 20 100` 조건 3회에서 각 클라이언트 첫 Teleport 이후 Sequence 누락 0건을 확인했다.

### D-016. 테스트 Actor 기준 기본 보간값은 일반 Transform 악조건에서도 유지

- 결정: 테스트 Actor 기준 `InterpBackTime=0.10`, `MaxExtrapTime=0.20`, `PosSnapDist=500`, `RotSnapDeg=90` 기본값을 유지한다.
- 사유: 일반 Transform 100ms 지연 단독, 20% 손실 단독, 20% 손실 + 100ms 지연 복합 조건에서 Snap/RotSnap/Teleport가 0회였고, 최대 외삽 시간이 제한값 0.200초보다 충분히 낮았기 때문이다.
- 영향: 차량 적용 전까지 플러그인 기본값을 조정하지 않는다. 차량 적용 세션에서는 차량 속도와 시각 Shell 구조 기준으로 별도 튜닝한다.

### D-017. CFNetSmooth는 Win64 Development Game 패키징을 통과

- 결정: `CFNetSmooth`는 TestMap 기준 Win64 Development Game 패키징 통과 상태로 본다.
- 사유: UAT BuildCookRun `-build -cook -stage -pak -archive`가 ExitCode 0으로 끝났고, 패키징된 실행 파일이 `CFNetSmooth` 프로젝트 플러그인을 Mount한 뒤 Engine 초기화와 정상 종료까지 통과했기 때문이다.
- 영향: Editor 빌드 전용 플러그인 문제는 현재 확인되지 않았다. Server 패키징과 패키지 접속 검증은 D-018, D-019에서 별도로 통과 처리했으며, Shipping 패키징은 아직 별도 검증으로 남긴다.

### D-018. CFNetSmooth는 Win64 Development Server 패키징을 통과

- 결정: `CFNetSmooth`는 TestMap 기준 Win64 Development Dedicated Server 패키징 통과 상태로 본다.
- 사유: UAT BuildCookRun `-server -noclient -serverplatform=Win64 -servertarget=CarFight_ReServer`가 ExitCode 0으로 끝났고, 패키징된 서버 실행 파일이 `CFNetSmooth` 프로젝트 플러그인을 Mount한 뒤 TestMap 로드, `GameNetDriver` 7777 포트 시작, 정상 종료까지 통과했기 때문이다.
- 영향: 네트워크 플러그인으로서 Dedicated Server 타깃 컴파일과 최소 서버 기동은 확인됐다. Shipping 패키징과 가시 클라이언트 화면 체감은 별도 검증으로 남긴다.

### D-019. CFNetSmooth는 Win64 Development 패키지 Server-Client 접속을 통과

- 결정: `CFNetSmooth`는 TestMap 기준 패키지 서버 1개 + 패키지 클라이언트 2개 접속 검증을 통과 상태로 본다.
- 사유: 패키징된 Dedicated Server에서 `[CFNetSmooth][Send]`가 1355회 발생했고, 패키징된 Client1/Client2에서 `[CFNetSmooth][Receive]`가 각각 1118회/1097회 발생했으며 Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure가 0건이었기 때문이다.
- 영향: Editor/PIE가 아닌 패키지 환경에서도 CFNetSmooth의 최소 네트워크 송수신 경로는 확인됐다. `-NullRHI` 자동 실행 기준이므로 화면 체감 검증과 Shipping 패키징은 별도 검증으로 남긴다.

### D-020. CFNetSmooth는 Win64 Development 패키지 가시 클라이언트 최소 검증을 통과

- 결정: `CFNetSmooth`는 TestMap 기준 NullRHI 없는 패키지 클라이언트 창 실행, 접속, 렌더링, 송수신 최소 검증을 통과 상태로 본다.
- 사유: 패키징된 Dedicated Server에서 `[CFNetSmooth][Send]`가 1673회 발생했고, NullRHI 없이 실행한 패키지 Client에서 `[CFNetSmooth][Receive]`가 1482회 발생했으며 화면 캡처 3장에서 TestMap과 차량 렌더링이 확인됐고 Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure가 0건이었기 때문이다.
- 영향: 패키지 환경의 실제 렌더링 클라이언트에서도 플러그인 수신 경로와 기본 화면 표시가 동작한다. 다만 자동 캡처는 미세한 부드러움까지 판정하지 못하므로 사용자 직접 체감, Teleport 가시 체감, Shipping 패키징은 별도 검증으로 남긴다.

### D-021. 차량 적용 전 차량 Transform 책임 정리를 먼저 수행

- 결정: `CFNetSmooth`를 실제 차량에 연결하기 전에 `VehiclePrep.md` 기준으로 차량 Actor, VehicleMesh, Wheel, Camera, Visual/Shell의 Transform 책임을 먼저 분리한다.
- 사유: 지금까지 실패한 주요 원인군은 같은 차량 Actor Transform을 Chaos Vehicle, UE RepMove, 수동 보간이 동시에 소유하려는 구조였다. CFNetSmooth를 그대로 붙이면 같은 충돌을 반복할 수 있다.
- 영향: 실제 차량 Actor와 VehicleMesh에는 CFNetSmooth 수신 Transform을 직접 적용하지 않는다. CFNetSmooth는 원격 표시용 Visual/Shell 대상에만 연결하는 방향을 우선한다.

### D-022. CFNetSmooth 플러그인 수정은 제작 세션으로 분리

- 결정: 차량 적용 세션에서는 `UE/Plugins/CFNetSmooth` 플러그인 소스를 수정하지 않는다.
- 사유: 차량 기준선 정리와 플러그인 API 변경을 같은 세션에서 섞으면 원인 추적과 롤백 범위가 불명확해진다.
- 영향: 플러그인 API, 디버그, 보간 정책 수정이 필요하면 `VehiclePrep.md`의 제작 세션 전달 프롬프트로 별도 CFNetSmooth 제작 세션에 요청한다.

### D-023. CFNetSmooth는 테스트 Actor 기준 v1.0 Release Candidate로 기능 동결

- 결정: 현재 `CFNetSmooth`는 테스트 Actor 기준 v1.0 Release Candidate로 보고, 코드/기본값/환경 변화가 없는 같은 조건 반복 검증은 중단한다.
- 사유: 에디터 빌드, Dedicated 자동 검증, 손실/지연/Teleport 악조건, Development Game/Server 패키징, 패키지 서버/클라이언트 접속, 패키지 가시 클라이언트 최소 렌더링 검증을 이미 통과했다. 동일 조건에서 45초와 180초를 반복하는 것은 새 실패 원인을 거의 추가하지 않는다.
- 영향: 이후 작업은 `ReleaseCandidate.md` 기준으로 차량 적용 인계 또는 Shipping 배포 검증으로 넘어간다. 단, 플러그인 코드, 기본값, 테스트 환경, 차량 적용 구조가 바뀌면 해당 변경 범위에 맞는 최소 재검증을 수행한다.

### D-024. 차량 Visual/Shell 연결은 읽기 전용 Transform API로 지원

- 결정: 실제 차량 적용을 위해 `CFNetSmooth`는 Owner Actor를 직접 움직이지 않고 보간 Transform을 반환하는 읽기 전용 API를 제공한다.
- 사유: 차량 적용 세션에서는 실제 차량 Actor와 VehicleMesh에 수신 Transform을 직접 적용하면 안 되고, 원격 Visual/Shell만 별도로 움직여야 하기 때문이다.
- 영향: 차량 세션은 `bApplyReceivedStateToOwner=false`를 유지한 상태에서 `TryGetSmoothedTransform` 결과를 Visual/Shell에 적용할 수 있다. SceneComponent를 플러그인이 직접 소유해 움직이는 API는 아직 만들지 않는다.

### D-025. 0.12.0 이후 Dedicated 로그 0건은 반복 실행보다 DebugLog 조건을 먼저 확인

- 결정: `CFNetSmooth 0.12.0` API 추가 후 `RunCFNetSmoothDedicated.bat 10 1`, `20 1`에서 접속/맵 로드는 정상이나 Send/Receive 로그가 0건이면 송수신 실패로 단정하지 않고 DebugLog 조건 확인 대상으로 분리한다.
- 사유: 해당 실행에서 플러그인 Mount, TestMap 로드, 클라이언트 접속, Fatal/Error/Timeout 0건은 확인됐지만 CFNetSmooth 로그가 없어 로그 기반 송수신 판정 근거가 부족하기 때문이다.
- 영향: 같은 조건의 짧은 Dedicated 스모크를 반복하지 않는다. 필요하면 `SetDebugLogEnabled` API 또는 `-CFNetSmoothDebugLog` 명령줄 override를 별도 작업으로 추가한다.

### D-026. 플러그인 manifest와 Build.cs는 범용 메타데이터를 사용

- 결정: `CFNetSmooth 0.12.1`부터 `.uplugin` 설명과 `Build.cs` 주석에서 CarFight 전용 표현을 제거하고 범용 Runtime 플러그인 기준으로 정리한다.
- 사유: manifest와 모듈 주석도 다른 프로젝트로 옮길 때 첫 기준 문서가 되므로, 여기서부터 CarFight 전용 물건처럼 보이면 안 된다.
- 영향: `.uplugin`의 `Description`은 project-agnostic 표현을 사용하고, `CreatedBy`는 `CFNetSmooth`로 둔다. `Build.cs`는 게임 프로젝트 모듈 의존성을 추가하지 않는다는 마이그레이션 기준을 가진다.

---

## 3. 미결 사항

### U-001. 회전 타입

- 선택지: `FRotator` 또는 `FQuat`
- 현재 결정: 저장은 `FRotator`, 보간 계산은 `FQuat::Slerp`
- 재검토 조건: 회전 보간에서 짐벌/긴 회전 경로 문제가 계속 보이면 State 저장 타입을 `FQuat`로 변경한다.

### U-002. 위치 정밀도

- 선택지: `FVector`, `FVector_NetQuantize100`, `FVector_NetQuantize10`
- v0.1 후보: `FVector_NetQuantize100`
- 재검토 조건: 고속 차량에서 정밀도 부족이 보이면 더 높은 정밀도로 변경한다.

### U-003. Teleport 전송 신뢰성

- 선택지: 일반 State와 같은 Unreliable, Teleport만 Reliable, Teleport 반복 송신
- 현재 결정: Teleport만 Reliable NetMulticast
- 재검토 조건: Reliable Teleport가 대량 발생해 큐 적체나 지연을 만들면 Teleport 반복 송신 또는 별도 압축 정책을 재검토한다.

### U-004. 차량 적용 대상

- 선택지: Actor Transform, Visual Root, Body Mesh, 별도 Shell Actor
- 현재 결정: 실제 차량 Actor Transform 직접 적용은 금지하고, 원격 표시용 Visual/Shell을 우선 후보로 둔다.
- 재검토 조건: `VehiclePrep.md` 기준 차량 사전 정리 후 Visual/Shell 후보가 실제 차량 물리와 완전히 분리되는지 확인한다.

### U-005. SendRate 기본값

- 선택지: 15, 20, 30
- v0.1 후보: 20
- 재검토 조건: 지연/손실 테스트에서 끊김 또는 대역폭 문제가 확인되면 조정한다.

### U-006. Visual/Shell 적용을 위한 플러그인 API 보강 필요 여부

- 선택지: 기존 Owner 적용 API 유지, SceneComponent 대상 적용 API 추가, 수신 Transform getter/delegate 추가
- 현재 결정: 수신 Transform getter 방식은 D-024로 추가 완료. SceneComponent 대상 직접 적용 API와 delegate는 아직 보류.
- 재검토 조건: 차량 Visual/Shell 적용 중 Tick 호출 방식만으로 부족하거나, Teleport/리스폰 이벤트를 즉시 전달해야 하면 delegate 또는 SceneComponent 대상 적용 API를 재검토한다.

---

## 4. 재검토 조건

다음 중 하나라도 발생하면 설계를 재검토한다.

- Dedicated Server에서 State 수신이 불안정하다.
- 지연 100ms에서 Snap이 과도하게 발생한다.
- 회전 Snap 로그가 과도하게 발생한다.
- 멈춘 Actor가 외삽으로 계속 미끄러진다.
- Teleport State가 누락되어 중간 경로가 보간된다.
- 지연 도착한 Teleport State가 최신 일반 State와 충돌해 표시 위치가 되돌아간다.
- 차량 연결 시 로컬 조작감이 바뀐다.
- 서버 권위 위치와 클라이언트 표시 위치가 혼동된다.
- CFNetSmooth를 실제 차량 Actor에 직접 붙여야만 하는 구조가 된다.
- Visual/Shell 적용에 필요한 API가 없어 플러그인 수정이 필요하다.
- Dedicated Summary에서 Send/Receive가 0건인데 같은 실행을 반복하고 있다.
- 플러그인 Runtime 코드나 Public API에 특정 게임 프로젝트 타입, 맵 경로, Build Target 의존성이 들어간다.

---

## 5. 참고 출처

- Noble Whale Smooth Sync Unreal 문서: `https://noblewhale.com/smooth_sync_unreal/index.html`
- Smooth Sync `USmoothSync` 클래스 문서: `https://noblewhale.com/smooth_sync_unreal/class_u_smooth_sync.html`
- Smooth Sync `SmoothState` 클래스 문서: `https://noblewhale.com/smooth_sync_unreal/class_smooth_state.html`
- 프로젝트 기준 문서: `Document/Plan/VehicleNetSyncPlan/README.md`
- 프로젝트 기준 문서: `Document/Plan/VehicleNetSyncPlan/VehicleNetSyncRecoveryPlan.md`
- 차량 적용 전 준비 문서: `Document/Plan/CFNetSmoothPlan/VehiclePrep.md`
- 릴리즈 후보 정리 문서: `Document/Plan/CFNetSmoothPlan/ReleaseCandidate.md`

---

## 6. Changelog

### v0.24.0

```text
- D-001A CFNetSmooth를 CarFight 전용이 아닌 범용 UE Runtime 플러그인으로 유지하는 결정 추가
- D-026 플러그인 manifest와 Build.cs를 범용 메타데이터로 유지하는 결정 추가
- D-001의 사유와 영향을 특정 프로젝트 전용 구현이 아니라 CFNetSmooth 자체 이름/API 기준으로 갱신
```

### v0.23.0

```text
- D-025 0.12.0 이후 Dedicated 스모크 Send/Receive 0건을 DebugLog 조건 확인 대상으로 분리하는 결정 추가
- 같은 Dedicated 스모크 반복 대신 SetDebugLogEnabled API 또는 명령줄 override를 별도 작업으로 검토하도록 기준 추가
```

### v0.22.0

```text
- D-024 차량 Visual/Shell 연결을 읽기 전용 Transform API로 지원하는 결정 추가
- U-006을 수신 Transform getter 추가 완료, SceneComponent 직접 적용/delegate 보류 상태로 갱신
- 차량 세션은 bApplyReceivedStateToOwner=false를 유지하고 TryGetSmoothedTransform 결과를 Visual/Shell에 적용하도록 기준 추가
```

### v0.21.0

```text
- D-023 CFNetSmooth 테스트 Actor 기준 v1.0 Release Candidate 기능 동결 결정 추가
- 코드/기본값/환경 변화 없는 같은 조건 반복 검증을 중단하는 기준 추가
- ReleaseCandidate.md를 참고 출처에 추가
```

### v0.20.0

```text
- D-021 차량 적용 전 차량 Transform 책임 정리 우선 결정 추가
- D-022 CFNetSmooth 플러그인 수정은 제작 세션으로 분리하는 결정 추가
- U-004 차량 적용 대상을 Visual/Shell 우선 후보로 갱신
- U-006 Visual/Shell 적용을 위한 플러그인 API 보강 필요 여부 추가
- VehiclePrep.md를 참고 출처에 추가
```

### v0.19.0

```text
- Win64 Development 패키지 가시 클라이언트 최소 검증 통과 결정 D-020 추가
- NullRHI 없는 패키지 클라이언트 렌더링, 화면 캡처, Receive 1482회, Fatal/Error/Timeout 0건 근거 기록
- 미세한 화면 부드러움, Teleport 가시 체감, Shipping 패키징은 별도 검증으로 분리
```

### v0.18.0

```text
- Win64 Development 패키지 Server-Client 접속 통과 결정 D-019 추가
- 패키지 서버 Send 1355회, 패키지 클라이언트 Receive 1118회/1097회, Fatal/Error/Timeout 0건 근거 기록
- D-017, D-018의 남은 검증 표현을 현재 통과 상태와 가시 화면 체감/Shipping 패키징 기준으로 갱신
```

### v0.17.0

```text
- Win64 Development Dedicated Server 패키징 통과 결정 D-018 추가
- 서버 패키지 스모크에서 CFNetSmooth 플러그인 Mount, TestMap 로드, GameNetDriver 7777 포트 시작을 확인한 근거 추가
- 실제 패키지 클라이언트 접속 검증은 별도 검증으로 분리
```

### v0.16.0

```text
- Win64 Development Game 패키징 통과 결정 D-017 추가
- 패키징 스모크 실행에서 CFNetSmooth 플러그인 Mount와 Engine 초기화 후 정상 종료를 확인한 근거 추가
- Shipping/Server 패키징은 별도 검증으로 분리
```

### v0.15.0

```text
- D-015에 Teleport 손실 20% + 지연 100ms 조건 3회 반복 근거 추가
- 각 클라이언트 첫 Teleport 이후 Sequence 누락 0건 기준을 반복성 근거로 기록
```

### v0.14.0

```text
- 일반 Transform 20% 손실 단독 검증 결과를 D-016 기본값 유지 근거에 추가
- D-016 제목과 사유를 일반 Transform 악조건 전체 기준으로 정리
```

### v0.13.0

```text
- 일반 Transform 100ms 지연 단독 검증 결과를 D-016 기본값 유지 근거에 추가
- 최대 외삽 시간이 제한값보다 낮았다는 기준을 단독/복합 조건 공통 판정으로 정리
```

### v0.12.0

```text
- 일반 Transform 20% 손실 + 100ms 지연 결과를 바탕으로 테스트 Actor 기본 보간값 유지 결정 D-016 추가
- 차량 적용 전까지 플러그인 기본값을 조정하지 않는 기준 추가
```

### v0.11.0

```text
- 지연 도착 Teleport State를 오래된 일반 State처럼 폐기하지 않는 결정 D-015 추가
- Teleport 수신 시 이전 Buffer만 제거하고 최신 시간 Anchor를 되돌리지 않는 기준 추가
- 20% 손실 + 100ms 지연 조건에서 확인된 Teleport Sequence 누락 원인 기록
```

### v0.10.0

```text
- Teleport State만 Reliable NetMulticast로 전송하는 결정 D-014 추가
- U-003 Teleport 전송 신뢰성을 미결 후보에서 현재 결정으로 갱신
- 20% 손실 테스트에서 Unreliable Teleport 누락이 확인됐다는 근거 추가
```

### v0.9.0

```text
- 현재 세션은 CFNetSmooth 플러그인 완성에 집중하고 차량 적용은 별도 세션에서 진행하는 결정 D-013 추가
- U-004 차량 적용 대상 재검토 조건을 별도 차량 적용 세션 기준으로 갱신
```

### v0.8.0

```text
- 회전 Snap은 Buffer를 유지하고 표시 Transform만 즉시 보정하는 결정 D-012 추가
- 회전 Snap 로그 과다 발생 시 재검토 조건 추가
```

### v0.7.0

```text
- 회전 저장은 FRotator, 보간은 FQuat Slerp로 수행하는 결정 D-011 추가
- U-001 회전 타입 미결 사항을 현재 결정 기준으로 갱신
```

### v0.6.0

```text
- 외삽을 최신 State 속도로 MaxExtrapTime까지만 허용하는 결정 D-010 추가
```

### v0.5.0

```text
- 위치 Snap은 Buffer를 유지하고 표시 위치만 즉시 보정하는 결정 D-009 추가
```

### v0.4.0

```text
- Teleport State 수신 시 Buffer를 비우고 즉시 적용하는 결정 D-008 추가
```

### v0.3.0

```text
- 수신 State Owner 적용 기본값을 false로 유지하는 결정 D-007 추가
```

### v0.2.0

```text
- 수신 로그 단계에서 Unreliable NetMulticast를 사용하기로 한 결정 D-006 추가
```

### v0.1.0

```text
- CFNetSmooth 결정 기록 최초 작성
- 확정 결정 D-001~D-005 추가
- 미결 사항 U-001~U-005 추가
- 재검토 조건과 참고 출처 추가
```

---

## 7. 마이그레이션 지침

- 결정이 바뀌면 해당 결정 번호를 유지하고 새 버전 항목을 추가한다.
- 미결 사항이 확정되면 `확정 결정`으로 승격하고 Changelog에 기록한다.
- 차량 코드 적용이 시작되면 관련 결정마다 실제 파일 경로와 롤백 지점을 추가한다.
