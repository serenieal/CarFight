# CFNetSmooth 구현 체크리스트

- 문서 버전: v0.9.1
- 작성일: 2026-06-17
- 대상 프로젝트: CarFight
- 작업 유형: 신규
- 상태: Draft

---

## 1. 목적

이 문서는 `CFNetSmooth` v0.1 플러그인 뼈대 생성과 첫 빌드 검증을 위한 실행 체크리스트다.

목표는 다음 작업자가 문서 해석 없이 순서대로 따라가며 “어디까지 했고, 어디서 멈춰야 하는지” 판단할 수 있게 만드는 것이다.

---

## 2. 시작 전 확인

작업 시작 전 아래 항목을 먼저 확인한다.

- [ ] `Document/Plan/CFNetSmoothPlan/README.md`를 읽었다.
- [ ] `Document/Plan/CFNetSmoothPlan/Design.md`를 읽었다.
- [ ] `Document/Plan/CFNetSmoothPlan/Api.md`를 읽었다.
- [ ] `Document/Plan/CFNetSmoothPlan/Plan.md`의 v0.1 금지 작업을 확인했다.
- [ ] 현재 작업이 기존 `ACFVehiclePawn` 직접 수정이 아님을 확인했다.
- [ ] 현재 작업이 `BP_CFVehiclePawn` 기본값 변경이 아님을 확인했다.
- [ ] 현재 작업이 RepMove/AutoPhysics 설정 변경이 아님을 확인했다.

---

## 3. 기준선 확인

CFNetSmooth 플러그인 제작/테스트 Actor 작업 전 기준선은 다음이어야 한다.

```text
PhysicsPrediction=false
PhysicsReplicationMode=Default
RepMove=true
AutoPhysics=true
```

실제 차량에 CFNetSmooth를 붙여 Replicate Movement 역할을 대신하게 만드는 적용 단계에서는 기준이 달라진다.

```text
RepMove=false
CFNetSmooth가 서버 권위 Transform State 송수신과 원격 표시 보간을 담당
```

확인 방법:

- [ ] 기존 차량 네트워크 기준선이 필요한 경우 `Document/Plan/VehicleNetSyncPlan/README.md`를 확인한다.
- [ ] Resimulation 실험 상태가 켜져 있지 않은지 확인한다.
- [ ] 차량 기준선이 불명확하면 플러그인 구현을 시작하지 않고 먼저 기준선 복구 여부를 확인한다.

---

## 4. v0.1 신규 파일 계획

v0.1에서 새로 만들 파일 후보는 다음이다.

```text
UE/Plugins/CFNetSmooth/CFNetSmooth.uplugin
UE/Plugins/CFNetSmooth/Source/CFNetSmooth/CFNetSmooth.Build.cs
UE/Plugins/CFNetSmooth/Source/CFNetSmooth/Public/CFNetSmoothState.h
UE/Plugins/CFNetSmooth/Source/CFNetSmooth/Public/CFNetSmoothComp.h
UE/Plugins/CFNetSmooth/Source/CFNetSmooth/Public/CFNetSmoothTestActor.h
UE/Plugins/CFNetSmooth/Source/CFNetSmooth/Private/CFNetSmoothComp.cpp
UE/Plugins/CFNetSmooth/Source/CFNetSmooth/Private/CFNetSmoothTestActor.cpp
```

주의:

- [ ] 파일명은 32자 이하를 유지한다.
- [ ] `Public`에는 외부에서 include할 구조체/컴포넌트 선언만 둔다.
- [ ] `Private`에는 구현 코드를 둔다.
- [ ] 기존 프로젝트 클래스의 파일명/경로/함수 시그니처를 말 없이 바꾸지 않는다.

---

## 5. 구현 1단계: 플러그인 뼈대

목표:

- 빈 플러그인과 빈 모듈이 프로젝트에서 빌드되는지 확인한다.

작업:

- [ ] `CFNetSmooth.uplugin` 생성
- [ ] `CFNetSmooth.Build.cs` 생성
- [ ] 최소 모듈 클래스 파일 필요 여부 확인
- [ ] 프로젝트가 플러그인을 인식하는지 확인

완료 조건:

- [ ] `Tools/BuildEditor.bat` 빌드가 통과한다.
- [ ] 에디터 실행 전 `UE/Plugins/CFNetSmooth/Binaries/Win64/UnrealEditor-CFNetSmooth.dll`이 존재한다.
- [ ] 에디터 실행 시 플러그인 로드 오류가 없다.

중단 조건:

- [ ] 플러그인 로드 실패가 발생하면 State/Component 구현으로 넘어가지 않는다.
- [ ] 모듈 의존성 오류가 발생하면 Build.cs만 먼저 수정한다.
- [ ] 빌드 성공 후 에디터 실행 전 플러그인 `Binaries`를 삭제하지 않는다.

---

## 6. 구현 2단계: State 구조체

목표:

- 네트워크 송수신에 사용할 최소 State 구조체를 만든다.

작업:

- [ ] `FCFNetSmoothState` 선언
- [ ] `ServerTime` 추가
- [ ] `Location` 추가
- [ ] `Rotation` 추가
- [ ] `LinearVelocity` 추가
- [ ] `AngularVelocity` 추가
- [ ] `bTeleport` 추가
- [ ] `Sequence` 추가

완료 조건:

- [ ] 구조체가 컴파일된다.
- [ ] RPC 파라미터로 사용할 수 있는 타입인지 확인한다.

중단 조건:

- [ ] 직렬화 오류가 나면 보간 로직으로 넘어가지 않고 타입 선택부터 재검토한다.

---

## 7. 구현 3단계: Component 빈 동작

목표:

- `UCFNetSmoothComp`가 Actor에 붙고 Tick/Replication 기본 설정이 동작하는지 확인한다.

작업:

- [ ] `UCFNetSmoothComp` 선언
- [ ] `UActorComponent` 상속
- [ ] Tick 사용 여부 결정
- [ ] 서버/클라이언트 Role 판정 헬퍼 추가
- [ ] 설정값 기본 변수 추가
- [ ] BP 노출 변수에 한국어 툴팁 추가

완료 조건:

- [ ] 컴포넌트가 에디터에서 추가 가능하다.
- [ ] PIE에서 서버/클라이언트 로그가 구분되어 찍힌다.

중단 조건:

- [ ] 컴포넌트가 에디터에 보이지 않으면 UCLASS/모듈 로드부터 확인한다.
- [ ] Tick이 과도하게 로그를 찍으면 디버그 플래그를 먼저 정리한다.

---

## 8. 구현 4단계: 서버 송신 로그

목표:

- 실제 State 송신 전, 서버가 송신할 조건과 값을 올바르게 계산하는지 확인한다.

작업:

- [ ] `SendRate` 누적 시간 계산
- [ ] 서버 Transform 읽기
- [ ] 속도/각속도 후보 계산
- [ ] `Sequence` 증가 처리
- [ ] `bEnableDebugLog`가 켜진 경우에만 로그 출력

완료 조건:

- [ ] 서버에서 `SendRate` 간격에 맞춰 로그가 출력된다.
- [ ] 정지 상태에서 불필요한 로그 폭주가 없다.

중단 조건:

- [ ] 서버가 아닌 클라이언트에서 송신 로그가 찍히면 Role 조건을 먼저 수정한다.

---

## 9. 구현 5단계: 클라이언트 수신 로그

목표:

- 클라이언트가 서버 State를 실제로 받는지 확인한다.

작업:

- [ ] `MulticastReceiveState` Unreliable NetMulticast RPC 추가
- [ ] State 수신 로그 추가
- [ ] 오래된 `Sequence` 무시 조건 추가
- [ ] Teleport State 수신 여부 로그 추가
- [ ] `CF Net Smooth Test Actor`에서 움직이는 `Loc=` 값 송수신 확인

완료 조건:

- [ ] 클라이언트에서 서버 State 수신 로그가 찍힌다.
- [ ] Dedicated Server에서도 수신 로그가 나온다.

중단 조건:

- [ ] Listen Server에서는 되지만 Dedicated Server에서 안 되면 Actor Replicates, 컴포넌트 Replication, NetMulticast 조건을 먼저 확인한다.

---

## 10. 구현 6단계: Buffer와 보간

목표:

- 수신 State를 Buffer에 넣고 목표 시점 기준으로 위치/회전을 보간한다.

작업:

- [x] State Buffer 배열 추가
- [x] ServerTime 기준 삽입/정렬
- [x] Buffer 최대 크기 제한
- [x] `InterpBackTime` 기준 목표 시간 계산
- [x] 위치 보간
- [x] 수신 State를 Owner에 적용할지 정하는 기본 false 옵션 추가
- [x] 회전 보간
- [x] 적용 대상 Transform 결정

완료 조건:

- [ ] 클라이언트 원격 표시가 서버 이동을 부드럽게 따라간다.
- [ ] 클라이언트 원격 표시가 서버 회전을 부드럽게 따라간다.
- [ ] Buffer 크기가 제한 이상 커지지 않는다.
- [ ] 기본 컴포넌트만 붙인 Actor는 자동으로 위치가 덮어써지지 않는다.

중단 조건:

- [ ] 위치가 역주행하면 시간 정렬과 목표 시간 계산을 먼저 확인한다.
- [ ] 회전이 긴 경로로 돌면 `FRotator` 대신 `FQuat` 재검토를 `Decisions.md`에 기록한다.
- [ ] 차량 Pawn이 의도치 않게 움직이면 Apply Received State To Owner 기본값과 테스트 Actor 전용 활성화 경로를 먼저 확인한다.

---

## 11. 구현 7단계: 외삽, Snap, Teleport

목표:

- State 누락, 큰 오차, 순간이동을 안전하게 처리한다.

작업:

- [x] `MaxExtrapTime` 제한 적용
- [x] 정지 상태에서 외삽 중단
- [x] `PosSnapDist` 적용
- [x] `RotSnapDeg` 적용
- [x] `MarkTeleport` 구현
- [x] Teleport State 수신 시 Buffer Clear 구현
- [x] Teleport State 수신 시 즉시 위치/회전 적용 구현
- [x] `ClearStateBuffer` 구현
- [x] `ForceSendNextState` 구현

완료 조건:

- [ ] State가 잠시 끊겨도 짧게만 예측한다.
- [ ] `MaxExtrapTime` 이후 최신 State 위치에서 멈춘다.
- [ ] 큰 위치 오차는 Snap으로 복구한다.
- [ ] 큰 회전 오차는 Snap으로 복구한다.
- [ ] Teleport는 중간 경로 없이 즉시 반영된다.
- [ ] Teleport 수신 직후 Buffer가 새 State 1개로 초기화된다.

중단 조건:

- [ ] Teleport가 누락되면 일반 Unreliable 경로 유지 여부를 재검토한다.
- [ ] Snap이 과도하면 `InterpBackTime`, `SendRate`, Snap 임계값을 같이 본다.

---

## 12. 구현 후 필수 검증

구현 후 아래 순서로 검증한다.

```text
1. Tools/BuildEditor.bat
2. Tools/RunEditor.bat
3. PIE 2인 Listen Server 테스트
4. Dedicated Server 테스트
5. 지연 100ms 테스트
6. 손실 테스트
7. Teleport 테스트
```

완료 조건:

- [ ] 빌드 성공
- [ ] 에디터 로드 성공
- [ ] PIE 2인 원격 표시 보간 확인
- [ ] Dedicated Server 원격 표시 보간 확인
- [ ] 지연 100ms에서 과도한 Snap 없음
- [ ] Teleport 보간 잔상 없음

---

## 13. 작업 완료 보고 양식

작업 완료 시 아래 항목을 반드시 보고한다.

```text
완료 작업:
- 

검증:
- 

남은 작업:
- 

다음 작업 추천:
- 
```

---

## 14. Changelog

### v0.9.1

```text
- 플러그인 제작/테스트 Actor 기준선과 실제 차량 CFNetSmooth 적용 기준선을 분리
- 실제 차량 적용 단계에서는 RepMove=false가 필요함을 명시
```

### v0.9.0

```text
- RotSnapDeg 기반 회전 Snap 구현 완료 항목 반영
- 큰 회전 오차 복구 완료 조건과 검증 필요 상태 유지
```

### v0.8.0

```text
- State 사이 회전 보간 완료 항목 반영
- 적용 대상 Transform 결정 완료 항목 반영
- 회전 보간 완료 조건 추가
```

### v0.7.0

```text
- MaxExtrapTime 기반 제한 외삽 완료 항목 반영
- 제한 시간 이후 최신 State 위치 유지 완료 조건 추가
```

### v0.6.0

```text
- PosSnapDist 기반 위치 Snap 완료 항목 반영
- 위치 Snap과 회전 Snap 완료 기준 분리
```

### v0.5.0

```text
- Teleport 수신 Buffer Clear와 즉시 적용 완료 항목 반영
- Teleport 수신 직후 Buffer=1 완료 조건 추가
- 외삽과 Snap은 후속 작업으로 유지
```

### v0.4.0

```text
- Buffer/위치 보간 구현 완료 항목 반영
- 수신 State Owner 적용 기본 false 옵션과 테스트 Actor 전용 활성화 체크포인트 추가
- 회전 보간, 외삽, Snap, Teleport를 후속 작업으로 분리
```

### v0.3.0

```text
- CFNetSmoothTestActor 신규 파일 계획과 움직이는 Loc 송수신 확인 항목 추가
```

### v0.2.0

```text
- 클라이언트 수신 로그 단계의 RPC 후보를 ClientReceiveState에서 MulticastReceiveState NetMulticast로 갱신
- Dedicated Server 실패 체크포인트를 NetMulticast 조건 기준으로 수정
```

### v0.1.1

```text
- 빌드 성공 후 에디터 실행 전 플러그인 Binaries를 삭제하지 말라는 체크포인트 추가
- CFNetSmooth DLL 존재 확인 항목 추가
```

### v0.1.0

```text
- CFNetSmooth v0.1 구현 체크리스트 최초 작성
- 플러그인 뼈대, State, Component, 송신/수신, Buffer, 보간, 외삽, Snap, Teleport 단계 정의
- 각 단계별 완료 조건과 중단 조건 추가
- 작업 완료 보고 양식에 남은 작업/다음 작업 추천 포함
```

---

## 15. 마이그레이션 지침

- 이 체크리스트는 기존 차량 코드 변경 절차가 아니다.
- v0.1 구현은 별도 플러그인과 테스트 Actor에서만 시작한다.
- 차량 Visual/Shell 연결은 v0.1/v0.2 검증 통과 후 별도 문서와 롤백 지점을 만든 뒤 진행한다.
