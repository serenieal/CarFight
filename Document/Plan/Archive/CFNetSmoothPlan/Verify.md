# CFNetSmooth 검증 계획

- 문서 버전: v0.15.0
- 작성일: 2026-06-18
- 대상 프로젝트: CarFight
- 작업 유형: 신규
- 상태: Draft

---

## 1. 목적

이 문서는 `CFNetSmooth`가 실제 차량 코드에 연결되기 전에 반드시 통과해야 하는 검증 절차를 정의한다.

검증의 핵심은 “부드러워 보이는가”보다 “서버 권위 기준을 깨지 않는가”이다.

---

## 2. 기본 검증 환경

### 2.1 엔진/실행 규칙

프로젝트 규칙에 따라 아래 도구를 우선 사용한다.

```text
빌드: Tools/BuildEditor.bat
에디터 실행: Tools/RunEditor.bat
네트워크 테스트: Tools/RunNetFalse.bat 또는 Tools/RunNetTrue.bat
CFNetSmooth Dedicated 자동 검증: Tools/RunCFNetSmoothDedicated.bat
```

직접 명령을 써야 할 경우 기준 엔진은 아래 경로다.

```text
D:\UnrealEngine_Source
```

`Tools/RunCFNetSmoothDedicated.bat`는 CFNetSmooth 플러그인 테스트 Actor 검증 전용이다.

기본 사용법:

```text
Tools\RunCFNetSmoothDedicated.bat
```

인자를 지정하는 사용법:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2
```

Teleport 테스트 모드 사용법:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 teleport 3.0
```

패킷 손실 테스트 모드 사용법:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 loss 0.0 20
```

Teleport + 패킷 손실 테스트 모드 사용법:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 teleportloss 3.0 20
```

의미:

```text
첫 번째 인자: 서버와 클라이언트를 유지할 시간(초)
두 번째 인자: 접속할 클라이언트 수
세 번째 인자: 선택 테스트 모드. teleport, loss, teleportloss를 사용할 수 있다.
네 번째 인자: teleport 또는 teleportloss 모드에서 사용할 Teleport Interval 값
다섯 번째 인자: loss 또는 teleportloss 모드에서 사용할 패킷 손실률(%)
여섯 번째 인자: loss 또는 teleportloss 모드에서 사용할 패킷 지연(ms)
```

출력:

```text
RuntimeLogs\CFNetSmoothDedicated\<실행시각>\Server.log
RuntimeLogs\CFNetSmoothDedicated\<실행시각>\Client1.log
RuntimeLogs\CFNetSmoothDedicated\<실행시각>\Client2.log
RuntimeLogs\CFNetSmoothDedicated\<실행시각>\Summary.txt
```

주의:

- 이 도구는 차량 RepMove 실험용이 아니다.
- 이 도구는 차량 Pawn, 차량 BP, RepMove, AutoPhysics 설정을 바꾸지 않는다.
- 화면 체감 검증이 필요할 때는 `-NullRHI` 자동 클라이언트가 아니라 가시 클라이언트로 별도 확인한다.
- `teleport` 모드는 저장된 맵/Actor 값을 바꾸지 않고 서버 실행 명령줄에만 `-CFNetSmoothTeleportTest`를 전달한다.
- `loss`, `teleportloss` 모드는 저장된 맵/Actor 값을 바꾸지 않고 서버 실행 명령줄에 `-PktLoss`, 필요 시 `-PktLag`만 전달한다.
- Summary에서 Send/Receive가 0으로 나오면 같은 실행을 반복하지 말고 TestMap 또는 `CF Net Smooth` 컴포넌트의 Enable Debug Log 조건을 먼저 확인한다.

### 2.2 기준선 보호

검증 시작 전 아래 상태를 확인한다.

```text
PhysicsPrediction=false
PhysicsReplicationMode=Default
RepMove=true
AutoPhysics=true
```

CFNetSmooth v0.1/v0.2 테스트 Actor 검증은 기존 차량 기준선을 변경하지 않는다.
실제 차량에 CFNetSmooth를 붙여 Replicate Movement 역할을 대신하게 만드는 적용 검증은 `VehiclePrep.md` 기준으로 RepMove=false에서 별도로 수행한다.

---

## 3. v0.1 테스트 Actor 검증

### 3.1 테스트 목적

차량이 아닌 단순 Actor에서 서버 권위 State 보간이 작동하는지 확인한다.

### 3.2 절차

```text
1. Tools/BuildEditor.bat 실행
2. 테스트 맵에서 `CF Net Smooth Test Actor` 배치
3. Actor Replicates=true 확인
4. Actor Replicate Movement=false 확인
5. CFNetSmooth 컴포넌트의 Enable Debug Log=true 설정
6. CFNetSmooth 컴포넌트의 Apply Received State To Owner=true 확인
7. PIE 2인 Listen Server 실행
8. 출력 로그(Output Log)에서 `[CFNetSmooth][Send]`와 `[CFNetSmooth][Receive]` 확인
9. `Loc=` 값이 시간에 따라 변하는지 확인
10. 클라이언트 화면에서 테스트 Actor가 서버 이동을 따라 움직이는지 확인
11. Dedicated Server 방식으로 같은 테스트 반복
```

### 3.3 예상 결과

- 서버 송신 로그의 `Loc=` 값이 계속 변한다.
- 클라이언트 수신 로그의 `Loc=` 값도 같은 순서로 변한다.
- `Buffer=64`를 넘지 않는다.
- 클라이언트 화면의 테스트 Actor 위치가 서버 이동을 부드럽게 따라간다.
- 현재 단계에서는 위치 보간만 기대한다.
- 회전 보간, 외삽, Snap, Teleport 즉시 적용은 다음 단계 검증 대상이다.

### 3.4 실패 체크포인트

- `[Send]`만 있고 `[Receive]`가 없으면 Actor Replicates와 NetMulticast 조건을 먼저 확인한다.
- `Loc=` 값이 변하지 않으면 `CF Net Smooth Test Actor`의 Enable Server Motion 값을 확인한다.
- `Buffer`가 64를 넘으면 수신 버퍼 제한을 확인한다.
- 수신 로그는 있는데 클라이언트 화면 Actor가 움직이지 않으면 Apply Received State To Owner 값을 먼저 확인한다.
- 클라이언트 위치가 끊겨 보이면 Interp Back Time, Send Rate, 네트워크 지연 조건을 함께 확인한다.

---

## 4. 네트워크 조건 검증

### 4.1 지연 테스트

목표:

- 100ms 지연에서 보간이 유지되는지 확인한다.

체크:

- Snap 횟수
- 평균 Buffer 크기
- 외삽 진입 횟수
- 표시 위치와 서버 위치 차이

### 4.2 손실 테스트

목표:

- 일부 State가 누락되어도 과도한 워프가 발생하지 않는지 확인한다.

체크:

- Unreliable RPC 손실 후 복구 시간
- 외삽 지속 시간
- Teleport State 누락 여부

자동 실행 예:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 loss 0.0 20
```

판정:

- Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure가 0이어야 한다.
- `[CFNetSmooth][Extrapolate]`가 발생할 수 있으나 `Max Extrap Time` 안에서만 발생해야 한다.
- 일반 Transform State는 Unreliable이므로 Receive 수가 Send 수보다 적은 것이 정상이다.

### 4.3 지연+손실 테스트

목표:

- 실제 네트워크 악조건에서 v0.1 기본값이 너무 공격적인지 확인한다.

판정:

- Snap이 자주 발생하면 `InterpBackTime`을 늘린다.
- 반응이 너무 늦으면 `InterpBackTime`을 줄인다.
- 미끄러짐이 길면 `MaxExtrapTime`을 줄인다.

---

## 5. Teleport 검증

### 5.1 절차

```text
1. Tools/BuildEditor.bat 빌드 후 에디터를 실행한다.
2. 테스트 맵에서 `CF Net Smooth Test Actor`를 선택한다.
3. CFNetSmooth 컴포넌트의 Enable Debug Log=true를 확인한다.
4. Test Teleport 항목에서 Enable Teleport Test=true로 설정한다.
5. Teleport Interval=3.0 정도로 낮춰 테스트 시간을 줄인다.
6. Teleport Offset=(0,600,0) 또는 맵에서 확인하기 쉬운 값으로 둔다.
7. PIE 2인 Listen Server를 실행한다.
8. 출력 로그(Output Log)에서 Teleport=true 수신 로그와 `[CFNetSmooth][Teleport]` 로그를 확인한다.
9. Teleport State 수신 직후 `Buffer=1`로 줄어드는지 확인한다.
10. 클라이언트 표시가 중간 경로 없이 즉시 이동하는지 확인한다.
11. Teleport 이후 일반 위치 보간으로 복귀하는지 확인한다.
```

### 5.2 예상 결과

- 순간이동 시 중간 경로를 보간하지 않는다.
- 이전 Buffer State가 다시 적용되지 않는다.
- Teleport 이후 새 State부터 보간한다.
- Teleport 수신 로그는 `Teleport=true`를 표시한다.
- Teleport 수신 직후 Buffer는 새 State 1개만 가진다.
- `[CFNetSmooth][Teleport]` 로그의 `Applied=true`는 테스트 Actor에서만 기대한다.

### 5.3 Teleport 손실 검증

절차:

```text
1. Tools/BuildEditor.bat 빌드가 성공했는지 확인한다.
2. 아래 자동 검증 명령을 실행한다.
3. Summary.txt에서 SendTeleportTrue, ReceiveTeleportTrue, Teleport, FatalEnsure, Errors, Timeouts 값을 확인한다.
```

자동 실행 명령:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 teleportloss 3.0 20
```

예상 결과:

- Server.log에는 `SendTeleportTrue`가 0보다 크게 나온다.
- 각 Client 로그에서 `ReceiveTeleportTrue`와 `Teleport` 값이 서로 같아야 한다.
- Client별 Teleport 수가 Server의 `SendTeleportTrue`와 반드시 같을 필요는 없다. 클라이언트가 서버 시작 후 순차 접속하기 때문이다.
- Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0이어야 한다.
- 손실+지연 조건에서는 각 클라이언트의 첫 Teleport 이후 서버 Teleport Sequence 누락이 없어야 한다.

실패 체크포인트:

- `ReceiveTeleportTrue`가 있는데 `Teleport`가 적으면 수신 후 적용 경로를 확인한다.
- Client 간 Teleport 수 차이가 과도하면 접속 시각, 네트워크 손실률, Reliable RPC 로그를 같이 확인한다.
- 첫 Teleport 이후 중간 Sequence가 빠지면 지연 도착 Teleport가 오래된 일반 State 폐기 조건에 걸렸는지 확인한다.
- Timeout이나 NetworkFailure가 나오면 플러그인 로직보다 테스트 네트워크 세션 안정성을 먼저 확인한다.

---

## 6. 위치 Snap 검증

### 6.1 절차

```text
1. Tools/BuildEditor.bat 빌드 후 에디터를 실행한다.
2. 테스트 맵에서 `CF Net Smooth Test Actor`를 선택한다.
3. CFNetSmooth 컴포넌트의 Enable Debug Log=true를 확인한다.
4. Test Teleport 항목에서 Enable Teleport Test=false로 둔다.
5. Snap 항목에서 Pos Snap Dist=1.0처럼 낮은 값으로 임시 설정한다.
6. Test Motion 항목에서 Motion Speed=600.0처럼 높은 값으로 임시 설정한다.
7. PIE 2인 Listen Server를 실행한다.
8. 출력 로그(Output Log)에서 `[CFNetSmooth][Snap]` 로그가 찍히는지 확인한다.
9. 테스트 후 Pos Snap Dist=500.0, Motion Speed=120.0으로 되돌린다.
```

### 6.2 예상 결과

- `[CFNetSmooth][Snap]` 로그에 `Error`와 `Threshold`가 표시된다.
- Snap 발생 시 클라이언트 표시 위치가 목표 위치로 즉시 보정된다.
- Teleport 테스트를 끈 상태이므로 `[CFNetSmooth][Teleport]` 로그 없이 Snap만 확인할 수 있다.
- `Pos Snap Dist=0.0`이면 Snap 로그가 찍히지 않는다.

### 6.3 실패 체크포인트

- Snap 로그가 안 나오면 `Pos Snap Dist`가 너무 크거나 이동 속도가 낮은지 확인한다.
- Snap 로그가 매 프레임 과도하게 나오면 테스트용으로 낮춘 `Pos Snap Dist`를 기본값으로 되돌린다.
- Teleport 로그가 같이 나오면 `Enable Teleport Test=false`인지 확인한다.

---

## 7. 제한 외삽 검증

### 7.1 절차

```text
1. Tools/BuildEditor.bat 빌드 후 에디터를 실행한다.
2. 테스트 맵에서 `CF Net Smooth Test Actor`를 선택한다.
3. CFNetSmooth 컴포넌트의 Enable Debug Log=true를 확인한다.
4. Test Teleport 항목에서 Enable Teleport Test=false로 둔다.
5. Snap 항목에서 Pos Snap Dist=0.0으로 설정해 Snap을 임시 비활성화한다.
6. Smoothing 항목에서 Interp Back Time=0.0으로 설정한다.
7. Smoothing 항목에서 Max Extrap Time=0.20으로 설정한다.
8. Test Motion 항목에서 Motion Speed=120.0으로 둔다.
9. PIE 2인 Listen Server를 실행한다.
10. 출력 로그(Output Log)에서 `[CFNetSmooth][Extrapolate]` 로그가 찍히는지 확인한다.
11. Max Extrap Time=0.0으로 바꿨을 때 `[CFNetSmooth][Extrapolate]` 로그가 멈추는지 확인한다.
12. 테스트 후 Interp Back Time=0.10, Max Extrap Time=0.20, Pos Snap Dist=500.0으로 되돌린다.
```

### 7.2 예상 결과

- `Interp Back Time=0.0`에서는 목표 시간이 최신 State보다 미래가 되기 쉬워 외삽 로그가 나온다.
- `[CFNetSmooth][Extrapolate]` 로그에 `Time`과 `Max`가 표시된다.
- `Time`은 `Max Extrap Time` 이하일 때만 외삽 위치를 만든다.
- `Max Extrap Time=0.0`이면 외삽 로그가 나오지 않고 최신 State 위치에서 멈춘다.

### 7.3 실패 체크포인트

- 외삽 로그가 안 나오면 `Interp Back Time=0.0`, `Max Extrap Time>0`, `Enable Debug Log=true`인지 확인한다.
- Snap 로그가 같이 나오면 `Pos Snap Dist=0.0`인지 확인한다.
- 움직임이 너무 튀면 테스트 후 기본값으로 되돌렸는지 확인한다.

---

## 8. 회전 보간 검증

### 8.1 절차

```text
1. Tools/BuildEditor.bat 빌드 후 에디터를 실행한다.
2. 테스트 맵에서 `CF Net Smooth Test Actor`를 선택한다.
3. CFNetSmooth 컴포넌트의 Enable Debug Log=true를 확인한다.
4. Test Teleport 항목에서 Enable Teleport Test=false로 둔다.
5. Test Motion 항목에서 Rotate During Motion=true로 설정한다.
6. Test Motion 항목에서 Rotation Speed Deg=90.0 정도로 설정한다.
7. Smoothing 항목에서 Interp Back Time=0.10으로 둔다.
8. Snap 항목에서 Pos Snap Dist=500.0으로 둔다.
9. PIE 2인 Listen Server를 실행한다.
10. 출력 로그(Output Log)에서 `[CFNetSmooth][Send]`, `[CFNetSmooth][Receive]`의 `Rot=` 값이 변하는지 확인한다.
11. 클라이언트 화면에서 테스트 Actor 회전이 튀지 않고 따라오는지 확인한다.
12. 테스트 후 Rotate During Motion=false 또는 Rotation Speed Deg=30.0으로 되돌린다.
```

### 8.2 예상 결과

- 서버 송신 로그와 클라이언트 수신 로그의 `Rot=` 값이 시간에 따라 변한다.
- 클라이언트 화면의 테스트 Actor 회전이 부드럽게 따라온다.
- 현재 단계에서는 State 사이 회전 보간만 기대한다.
- 최신 State 이후 외삽 구간에서는 회전이 최신 State 회전에 머무르는 것이 정상이다.
- 큰 회전 오차를 즉시 맞추는 `RotSnapDeg` 처리는 9장 회전 Snap 검증에서 별도로 확인한다.

### 8.3 실패 체크포인트

- `Rot=` 값이 계속 `R(0)`이면 `Rotate During Motion=true`와 `Rotation Speed Deg` 값을 확인한다.
- 회전이 긴 경로로 도는 것처럼 보이면 `FRotator` 저장 방식과 `FQuat` 보간 경로를 재검토한다.
- 위치 Snap 로그가 과도하게 나오면 `Pos Snap Dist`가 기본값인지 확인한다.

---

## 9. 회전 Snap 검증

### 9.1 절차

```text
1. Tools/BuildEditor.bat 빌드 후 에디터를 실행한다.
2. 테스트 맵에서 `CF Net Smooth Test Actor`를 선택한다.
3. CFNetSmooth 컴포넌트의 Enable Debug Log=true를 확인한다.
4. Test Teleport 항목에서 Enable Teleport Test=false로 둔다.
5. Snap 항목에서 Pos Snap Dist=0.0으로 설정해 위치 Snap을 임시 비활성화한다.
6. Snap 항목에서 Rot Snap Deg=1.0처럼 낮은 값으로 임시 설정한다.
7. Test Motion 항목에서 Rotate During Motion=true로 설정한다.
8. Test Motion 항목에서 Rotation Speed Deg=720.0처럼 높은 값으로 임시 설정한다.
9. PIE 2인 Listen Server를 실행한다.
10. 출력 로그(Output Log)에서 `[CFNetSmooth][RotSnap]` 로그가 찍히는지 확인한다.
11. Rot Snap Deg=0.0으로 바꿨을 때 `[CFNetSmooth][RotSnap]` 로그가 멈추는지 확인한다.
12. 테스트 후 Rot Snap Deg=90.0, Pos Snap Dist=500.0, Rotation Speed Deg=30.0 또는 Rotate During Motion=false로 되돌린다.
```

### 9.2 예상 결과

- `[CFNetSmooth][RotSnap]` 로그에 `Error`, `Threshold`, `Rot`가 표시된다.
- Rot Snap 발생 시 클라이언트 표시 회전이 목표 회전으로 즉시 보정된다.
- 위치 Snap을 끈 상태이므로 `[CFNetSmooth][Snap]` 로그 없이 회전 Snap만 확인할 수 있다.
- `Rot Snap Deg=0.0`이면 회전 Snap 로그가 찍히지 않는다.

### 9.3 실패 체크포인트

- RotSnap 로그가 안 나오면 `Rot Snap Deg`가 너무 크거나 `Rotation Speed Deg`가 낮은지 확인한다.
- `Rot=` 값이 변하지 않으면 `Rotate During Motion=true`인지 확인한다.
- Snap 로그가 같이 나오면 `Pos Snap Dist=0.0`인지 확인한다.
- RotSnap 로그가 매 프레임 과도하게 나오면 테스트용으로 낮춘 `Rot Snap Deg`를 기본값으로 되돌린다.

---

## 10. 차량 Visual/Shell 검증

이 검증은 v0.3 이후에만 수행한다.

### 10.1 진입 조건

- v0.1 테스트 Actor 검증 통과
- v0.2 Teleport/Buffer Clear 검증 통과
- 기존 차량 기준선 정상 확인

### 10.2 절차

```text
1. Client1에서 차량 조작
2. Client2에서 Client1 차량 원격 표시 확인
3. 로컬 Client1 조작감 변화 여부 확인
4. 서버 로그에서 권위 위치 유지 확인
5. 리스폰/충돌/정지 상황 반복
```

### 10.3 예상 결과

- Client2 원격 표시만 개선된다.
- Client1 로컬 차량 물리와 조작감은 바뀌지 않는다.
- 서버 권위 충돌/리스폰 판정은 기존과 동일하다.

---

## 11. 완료 기준

v0.1 완료:

- 테스트 Actor가 Listen/Dedicated 환경에서 보간된다.
- 지연 100ms 조건에서 큰 워프 없이 이동한다.
- 멈춘 뒤 외삽 미끄러짐이 제한된다.

v0.2 완료:

- Teleport가 보간 잔상 없이 처리된다.
- 20% 손실 조건에서도 Teleport State 수신과 적용이 누락 없이 이어진다.
- 20% 손실 + 100ms 지연 조건에서도 각 클라이언트 첫 Teleport 이후 중간 Teleport Sequence 누락이 없어야 한다.
- Buffer Clear가 소유권/리스폰 상황에서 정상 동작한다.
- 큰 위치 오차가 PosSnapDist 기준으로 즉시 복구된다.
- State가 잠시 끊기면 MaxExtrapTime 안에서만 예측하고 이후 최신 State 위치에서 멈춘다.
- State 사이 회전이 튀지 않고 보간된다.
- 큰 회전 오차가 RotSnapDeg 기준으로 즉시 복구된다.

v0.3 완료:

- 원격 차량 표시 개선이 확인된다.
- 로컬 조작 차량과 서버 권위 기준선이 훼손되지 않는다.

릴리즈 후보 이후 검증 정책:

```text
테스트 Actor 기준 v1.0 Release Candidate 판정 후에는 같은 조건의 반복 검증을 중단한다.
코드가 바뀌지 않았고 기본값이 바뀌지 않았고 실행 환경이 바뀌지 않았다면 45초/180초 반복 검증은 추가하지 않는다.
검증은 변경된 범위에 맞춰 최소로 수행한다.
```

추가 검증이 필요한 조건:

```text
CFNetSmooth 코드 변경
CFNetSmooth 기본값 변경
테스트 Actor 또는 테스트 맵 조건 변경
Development가 아닌 Shipping 패키징
실제 차량 Visual/Shell 연결
실제 차량 Replicate Movement 정책 변경
사용자 직접 체감 문제 보고
```

최소 재검증 기준:

```text
Tools\BuildEditor.bat
Tools\RunCFNetSmoothDedicated.bat 10 1
변경된 기능에 해당하는 단일 악조건 테스트 1회
```

Dedicated 스모크 해석 기준:

```text
접속, 맵 로드, Fatal/Error/Timeout 0건은 실행 안정성 확인으로 본다.
Send/Receive 로그가 0건이면 송수신 검증 통과로 보지 않는다.
이 경우 테스트 시간을 늘리지 말고 Enable Debug Log 저장값 또는 명령줄 로그 override 필요 여부를 먼저 확인한다.
```

---

## 12. Changelog

### v0.15.0

```text
- Dedicated Summary의 Send/Receive가 0일 때 같은 테스트 반복을 중단하고 DebugLog 조건을 먼저 확인하는 기준 추가
- 0.12.0 이후 스모크 테스트 해석 기준을 실행 안정성 확인과 송수신 검증으로 분리
```

### v0.14.0

```text
- 릴리즈 후보 이후 같은 조건 반복 검증 중단 정책 추가
- 코드/기본값/환경/차량 적용 범위 변경 시에만 최소 재검증을 수행하도록 기준 추가
- 최소 재검증 명령 기준을 BuildEditor, Dedicated 10초 스모크, 변경 기능 단일 악조건 테스트로 정리
```

### v0.13.1

```text
- 테스트 Actor 검증 기준선과 실제 차량 CFNetSmooth 적용 검증 기준선을 분리
- 실제 차량 적용 검증은 RepMove=false에서 별도로 수행해야 함을 명시
```

### v0.13.0

```text
- Teleport 손실+지연 검증에서 Sequence 누락 확인 기준 추가
- 첫 Teleport 이후 중간 Sequence가 빠질 때의 실패 체크포인트 추가
- v0.2 완료 기준에 20% 손실 + 100ms 지연 Teleport 검증 조건 추가
```

### v0.12.0

```text
- RunCFNetSmoothDedicated.bat loss, teleportloss 모드 사용법 추가
- Teleport 손실 검증 절차와 Summary.txt 판정 기준 추가
- v0.2 완료 기준에 20% 손실 조건 Teleport Reliable 검증 추가
```

### v0.11.0

```text
- Tools/RunCFNetSmoothDedicated.bat teleport 모드 사용법 추가
- Teleport 테스트 모드가 저장된 맵/Actor 값을 바꾸지 않고 명령줄 override만 사용한다는 설명 추가
```

### v0.10.0

```text
- Tools/RunCFNetSmoothDedicated.bat 사용법 추가
- CFNetSmooth Dedicated 자동 검증 로그와 Summary.txt 출력 위치 추가
- 차량 RepMove 실험용 도구가 아니라는 주의 사항 추가
```

### v0.9.0

```text
- 회전 Snap 검증 절차 추가
- Rot Snap Deg 임시 저값 테스트와 비활성화 확인 방법 추가
- 완료 기준에 큰 회전 오차 Snap 복구 조건 추가
```

### v0.8.0

```text
- 회전 보간 검증 절차 추가
- Rotate During Motion 기반 회전 로그/화면 확인 절차 추가
- 회전 외삽과 RotSnapDeg는 후속 작업으로 분리
```

### v0.7.0

```text
- 제한 외삽 검증 절차 추가
- Interp Back Time=0.0 기반 외삽 로그 확인 방법 추가
- Max Extrap Time=0.0 비활성화 확인 방법 추가
```

### v0.6.0

```text
- 위치 Snap 검증 절차 추가
- Pos Snap Dist 임시 저값 테스트와 복구 절차 추가
- 완료 기준에 큰 위치 오차 Snap 복구 조건 추가
```

### v0.5.0

```text
- 선택형 Teleport 테스트 옵션 기준 검증 절차 추가
- Teleport 수신 직후 Buffer=1 확인 기준 추가
- [CFNetSmooth][Teleport] 로그 기대 결과 추가
```

### v0.4.0

```text
- 테스트 Actor 위치 보간 적용 검증 절차 추가
- Apply Received State To Owner 확인 항목 추가
- 아직 회전/외삽/Snap/Teleport는 다음 단계라는 범위 명시
```

### v0.3.0

```text
- CFNetSmoothTestActor 기준의 움직이는 서버 Transform 송수신 로그 검증 절차 추가
- 현재 단계에서는 클라이언트 Transform 보간 적용을 기대하지 않는다는 판정 기준 명시
```

### v0.1.0

```text
- CFNetSmooth 검증 계획 최초 작성
- 테스트 Actor, 네트워크 조건, Teleport, 차량 Visual/Shell 검증 절차 정의
- 기준선 보호와 실패 체크포인트 명시
```

---

## 13. 마이그레이션 지침

- 검증 실패 시 기존 차량 코드에 연결하지 않는다.
- v0.3 차량 연결 전 별도 롤백 절차를 작성한다.
- 검증 결과는 `Decisions.md`에 성공/실패 근거로 기록한다.
