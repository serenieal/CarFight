# CFNetSmooth 테스트 결과 기록

- 문서 버전: v0.15.1
- 작성일: 2026-06-18
- 대상 프로젝트: CarFight
- 작업 유형: 신규
- 상태: Draft

---

## 1. 목적

이 문서는 `CFNetSmoothTestActor` 기준으로 실제 확인된 검증 결과를 기록한다.

목표는 다음이다.

```text
이미 확인된 항목과 아직 남은 항목을 분리한다.
다음 세션이나 다른 AI가 같은 기준으로 이어서 판단할 수 있게 한다.
실제 차량 적용 전 필요한 안전 조건을 명확히 남긴다.
```

---

## 2. 검증 범위

### 2.1 포함 범위

이 문서는 아래 범위만 다룬다.

```text
CFNetSmooth 플러그인
CFNetSmoothTestActor
CF Net Smooth 컴포넌트
PIE 기반 테스트 Actor 로그 검증
Dedicated Server 기본 반복 로그 검증
```

### 2.2 제외 범위

아래 항목은 아직 검증 완료로 보지 않는다.

```text
실제 차량 Pawn 적용
차량 Visual/Shell 분리 적용
Shipping 패키징
실제 차량 적용 후 Visual/Shell 검증
사용자 육안 기준 최종 체감 판정
```

---

## 3. 기준 빌드 상태

### 3.1 빌드 명령

```text
Tools\BuildEditor.bat
```

### 3.2 기준 엔진

```text
D:\UnrealEngine_Source
```

### 3.3 마지막 확인 결과

```text
Result: Succeeded
UnrealEditor-CFNetSmooth.dll 생성 확인
```

판정:

- 에디터용 모듈 빌드는 통과했다.
- `D:\UE_5.7` 경로는 사용하지 않았다.

---

## 4. 기능별 결과 요약

| 항목 | 결과 | 근거 | 비고 |
|---|---|---|---|
| 컴포넌트 표시 | 통과 | 에디터에서 `CF Net Smooth` 컴포넌트 확인 | 사용자 확인 |
| Send/Receive | 통과 | `[Send]`, `[Receive]` 로그 확인 | Sequence 증가 |
| Buffer 제한 | 통과 | `Buffer=64` 유지 로그 확인 | 최대 유지 개수 동작 |
| 위치 이동 State | 통과 | `Loc` 값 변화 확인 | 테스트 Actor 이동 |
| Teleport | 통과 | `[Teleport] Applied=true` 로그 확인 | Buffer reset 경로 확인 |
| 위치 Snap | 통과 | `[Snap] Error > Threshold` 로그 확인 | 낮은 임계값 스트레스 |
| 제한 외삽 | 통과 | `[Extrapolate] Time < Max` 로그 확인 | MaxExtrapTime 경로 확인 |
| 회전 송수신 | 통과 | `Rot` 값 변화 확인 | 360도 래핑 정상 |
| 회전 보간 | 부분 통과 | 회전 송수신과 보간 코드 빌드 확인 | 화면 체감은 별도 반복 권장 |
| 회전 Snap | 통과 | `[RotSnap] Error > Threshold` 로그 확인 | 비활성화/기본값도 확인 |
| Dedicated Server 기본 반복 | 통과 | 서버 2회, 클라이언트 2개씩 로그 수집 | Fatal/Error/Timeout 0 |
| Dedicated 자동 실행 스크립트 | 통과 | `RunCFNetSmoothDedicated.bat 10 1` 실행 | Summary.txt 생성 |
| Dedicated 자동 반복 45초 | 통과 | `RunCFNetSmoothDedicated.bat 45 2` 3회 반복 | Fatal/Error/Timeout 0 |
| Dedicated Teleport 모드 | 통과 | `RunCFNetSmoothDedicated.bat 45 2 teleport 3.0` 실행 | Teleport 수신, Fatal/Error/Timeout 0 |
| Dedicated Teleport 손실 모드 | 통과 | `RunCFNetSmoothDedicated.bat 45 2 teleportloss 3.0 20` 실행 | Teleport Reliable 보강 후 Fatal/Error/Timeout 0 |
| Dedicated Teleport 손실+지연 모드 | 통과 | `RunCFNetSmoothDedicated.bat 45 2 teleportloss 3.0 20 100` 실행 | 지연 도착 Teleport 보강 후 Sequence 누락 0 |
| Dedicated Teleport 손실+지연 반복 | 통과 | `RunCFNetSmoothDedicated.bat 45 2 teleportloss 3.0 20 100` 3회 기준 | 각 클라이언트 첫 Teleport 이후 Sequence 누락 0 |
| Dedicated 일반 Transform 손실+지연 모드 | 통과 | `RunCFNetSmoothDedicated.bat 45 2 loss 0.0 20 100` 실행 | Snap 0, 최대 외삽 0.001초 |
| Dedicated 일반 Transform 지연 단독 모드 | 통과 | `RunCFNetSmoothDedicated.bat 45 2 loss 0.0 0 100` 실행 | Snap 0, 최대 외삽 0.006초 |
| Dedicated 일반 Transform 손실 단독 모드 | 통과 | `RunCFNetSmoothDedicated.bat 45 2 loss 0.0 20 0` 실행 | Snap 0, 최대 외삽 0.007초 |
| Win64 Development 패키징 | 통과 | UAT BuildCookRun `-build -cook -stage -pak -archive` 실행 | BuildCookRun 성공, 패키지 스모크 실행 ExitCode 0 |
| Win64 Development Server 패키징 | 통과 | UAT BuildCookRun `-server -noclient -serverplatform=Win64` 실행 | Server BuildCookRun 성공, 서버 스모크 실행 ExitCode 0 |
| Win64 Development 패키지 Server-Client 접속 | 통과 | 패키지 서버 1개 + 패키지 클라이언트 2개 접속 실행 | 서버 Send 1355, 클라이언트 Receive 1118/1097, Fatal/Error/Timeout 0 |
| Win64 Development 패키지 가시 클라이언트 | 부분 통과 | NullRHI 없이 패키지 클라이언트 1개 창 실행 | 렌더링/접속/수신/오류 0 확인, 미세 체감은 사용자 직접 확인 필요 |
| 차량 Visual/Shell용 읽기 API | 빌드 통과 | `CFNetSmooth 0.12.0` 적용 후 `Tools\BuildEditor.bat` 성공 | Owner를 움직이지 않는 `TryGetSmoothedTransform` 계열 API 추가 |
| 범용 플러그인 메타데이터 | 빌드 통과 | `CFNetSmooth 0.12.1` 적용 후 `Tools\BuildEditor.bat` 성공 | `.uplugin`, `Build.cs`에서 CarFight 전용 표현 제거 |
| 0.12.0 이후 Dedicated 로그 스모크 | 판정 보류 | `RunCFNetSmoothDedicated.bat 10 1`, `20 1` 실행 | 접속/맵 로드는 정상이나 CFNetSmooth 로그 0. DebugLog 조건 확인 필요 |

---

## 5. 상세 결과

### 5.1 컴포넌트 표시

확인:

```text
에디터에서 CF Net Smooth 컴포넌트가 보임
```

판정:

- 플러그인 모듈 로드와 컴포넌트 BP 노출은 정상이다.

---

### 5.2 Send/Receive

확인 로그 유형:

```text
[CFNetSmooth][Send]
[CFNetSmooth][Receive]
```

확인된 내용:

- `Seq`가 순서대로 증가했다.
- `Time`이 증가했다.
- `Loc` 값이 움직임에 따라 변했다.
- `Receive` 로그가 클라이언트에서 찍혔다.
- `Buffer=64`를 넘지 않았다.

판정:

- 서버 State 생성과 NetMulticast 수신은 정상이다.

---

### 5.3 Teleport

확인 로그 유형:

```text
[CFNetSmooth][Teleport] Actor=CFNetSmoothTestActor_1 ... Applied=true
```

확인된 내용:

- Teleport State가 주기적으로 수신됐다.
- `Applied=true`로 테스트 Actor 즉시 적용 경로가 동작했다.
- Y축 기준 위치가 양쪽 오프셋으로 전환됐다.

판정:

- Teleport 수신 시 즉시 적용 경로는 정상이다.
- Buffer Clear 이후 잔상 여부는 화면 반복 확인을 계속 권장한다.

---

### 5.4 위치 Snap

확인 로그 유형:

```text
[CFNetSmooth][Snap] Actor=CFNetSmoothTestActor_1 Error=... Threshold=1.00 Loc=...
```

확인된 내용:

- `Pos Snap Dist=1.0` 테스트에서 Snap 로그가 발생했다.
- `Error`가 `Threshold`보다 큰 상황에서만 Snap 로그가 찍혔다.

판정:

- `PosSnapDist` 기반 위치 Snap 판정과 적용 경로는 정상이다.

---

### 5.5 제한 외삽

확인 로그 유형:

```text
[CFNetSmooth][Extrapolate] Actor=CFNetSmoothTestActor_1 Time=... Max=0.200 Loc=...
```

확인된 내용:

- `Interp Back Time=0.0` 조건에서 외삽 로그가 발생했다.
- `Time` 값이 `Max=0.200` 이하인 범위에서 기록됐다.

판정:

- 최신 State 이후 제한 외삽 진입 경로는 정상이다.
- 장시간 손실 상황에서 Max 이후 멈춤 체감은 추가 검증 대상이다.

---

### 5.6 회전 송수신과 래핑

확인 로그 예:

```text
Send Rot=R(Y=-169.71)
Receive Rot=R(Y=190.28)
```

판정:

- `-169.71 + 360 = 190.29`이므로 같은 방향이다.
- `FRotator` 표시가 180도 경계를 넘으며 음수/양수로 래핑되는 것은 정상이다.
- 회전 데이터가 깨진 것으로 보지 않는다.

---

### 5.7 회전 보간

확인된 내용:

- `FQuat::Slerp` 기반 회전 보간 코드가 빌드 통과했다.
- `Send`와 `Receive`의 `Rot` 값이 시간에 따라 변했다.

판정:

- 코드 경로와 송수신 값은 정상이다.
- 테스트 Actor 기준 회전 송수신과 보간 경로는 통과로 본다.
- 사용자 육안 기준 미세 체감은 실제 차량 Visual/Shell 적용 단계에서 확인한다.

추가 확인 권장:

```text
Rotate During Motion=true
Rotation Speed Deg=90.0
Rot Snap Deg=90.0
Pos Snap Dist=500.0
```

---

### 5.8 회전 Snap

확인 로그 유형:

```text
[CFNetSmooth][RotSnap] Actor=CFNetSmoothTestActor_1 Error=... Threshold=1.00 Rot=...
```

확인된 내용:

- `Rot Snap Deg=1.0`, `Rotation Speed Deg=720.0` 조건에서 RotSnap 로그가 발생했다.
- `Error`가 `Threshold=1.00`보다 큰 상황에서 로그가 찍혔다.
- `Rot Snap Deg=0.0`으로 바꾸면 `[RotSnap]` 로그가 없었다.
- 기본값 상태 일반 이동/회전에서도 `[RotSnap]` 로그가 없었다.

판정:

- 회전 Snap 활성화 조건 정상
- 회전 Snap 비활성화 조건 정상
- 기본값 과민 반응 없음

---

### 5.9 Dedicated Server 기본 반복

실행 조건:

```text
서버: UnrealEditor.exe CarFight_Re.uproject /Game/Maps/TestMap -server -log -unattended -NoSound
클라이언트: UnrealEditor.exe CarFight_Re.uproject 127.0.0.1 -game -NullRHI -log -unattended -NoSound
반복: 2회
클라이언트 수: 각 회차 2개
```

로그 폴더:

```text
RuntimeLogs/CFNetSmoothDedicated/20260617_142147
RuntimeLogs/CFNetSmoothDedicated/20260617_142400
```

1회차 결과:

| 파일 | Send | Receive | Extrapolate | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|
| Server.log | 778 | 0 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 637 | 4 | 0 | 0 | 0 |
| Client2.log | 0 | 562 | 5 | 0 | 0 | 0 |

2회차 결과:

| 파일 | Send | Receive | Extrapolate | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|
| Server.log | 640 | 0 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 477 | 1 | 0 | 0 | 0 |
| Client2.log | 0 | 393 | 0 | 0 | 0 | 0 |

판정:

- Dedicated Server에서 서버 `[CFNetSmooth][Send]`가 반복적으로 발생했다.
- 각 회차의 Client1, Client2에서 `[CFNetSmooth][Receive]`가 반복적으로 발생했다.
- Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0건이었다.
- 기본값 상태에서 Snap, RotSnap, Teleport 로그가 없는 것은 정상이다.
- 짧은 자동 실행 조건에서 기본 Dedicated 송수신 안정성은 통과로 본다.

주의:

- 이 결과는 장시간 지연/손실 테스트가 아니다.
- `-NullRHI` 클라이언트로 실행했기 때문에 화면 체감 검증은 포함하지 않는다.
- Dedicated 환경에서 화면 회전 보간 체감은 별도 가시 클라이언트 테스트로 확인한다.

---

### 5.10 Dedicated 자동 실행 스크립트

실행 명령:

```text
Tools\RunCFNetSmoothDedicated.bat 10 1
```

로그 폴더:

```text
RuntimeLogs/CFNetSmoothDedicated/20260617_142751
```

결과:

| 파일 | Send | Receive | Extrapolate | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|
| Server.log | 181 | 0 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 47 | 0 | 0 | 0 | 0 |

추가 확인:

```text
Summary.txt 생성 확인
```

판정:

- 전용 실행 스크립트가 CarFight 환경 가드를 통과했다.
- Dedicated Server와 자동 클라이언트 실행, 종료, 로그 요약 생성이 정상 동작했다.
- 짧은 스모크 테스트 기준 Fatal, Error, Timeout은 0건이었다.

---

### 5.11 Dedicated 자동 반복 45초

실행 명령:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2
```

반복:

```text
3회
```

로그 폴더:

```text
RuntimeLogs/CFNetSmoothDedicated/20260617_143028
RuntimeLogs/CFNetSmoothDedicated/20260617_143151
RuntimeLogs/CFNetSmoothDedicated/20260617_143313
```

1회차 결과:

| 파일 | Send | Receive | Extrapolate | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|
| Server.log | 786 | 0 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 645 | 5 | 0 | 0 | 0 |
| Client2.log | 0 | 565 | 2 | 0 | 0 | 0 |

2회차 결과:

| 파일 | Send | Receive | Extrapolate | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|
| Server.log | 783 | 0 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 642 | 5 | 0 | 0 | 0 |
| Client2.log | 0 | 553 | 2 | 0 | 0 | 0 |

3회차 결과:

| 파일 | Send | Receive | Extrapolate | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|
| Server.log | 790 | 0 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 645 | 3 | 0 | 0 | 0 |
| Client2.log | 0 | 567 | 1 | 0 | 0 | 0 |

판정:

- 3회 모두 서버 `[CFNetSmooth][Send]`가 780회 이상 발생했다.
- 3회 모두 Client1, Client2에서 `[CFNetSmooth][Receive]`가 반복적으로 발생했다.
- 3회 모두 Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0건이었다.
- 기본 프리셋 상태에서 Teleport, Snap, RotSnap 로그가 0인 것은 정상이다.
- 기본 45초/2클라이언트 Dedicated 자동 반복 검증은 통과로 본다.

주의:

- 이 결과는 아직 네트워크 지연/손실 조건 검증이 아니다.
- `-NullRHI` 자동 클라이언트 기준이므로 화면 체감 검증은 포함하지 않는다.

---

### 5.12 Dedicated Teleport 모드

실행 명령:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 teleport 3.0
```

명령줄 override:

```text
-CFNetSmoothTeleportTest
-CFNetSmoothTeleportInterval=3.0
```

로그 폴더:

```text
RuntimeLogs/CFNetSmoothDedicated/20260617_144231
```

결과:

| 파일 | Send | Receive | Teleport | Snap | Extrapolate | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| Server.log | 795 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 650 | 14 | 1 | 8 | 0 | 0 | 0 |
| Client2.log | 0 | 560 | 12 | 1 | 4 | 0 | 0 | 0 |

확인된 Teleport 로그 예:

```text
[CFNetSmooth][Teleport] Actor=CFNetSmoothTestActor_1 Seq=178 Time=12.103 Loc=X=1430.000 Y=-620.000 Z=0.000 Rot=R(0) Applied=true
[CFNetSmooth][Teleport] Actor=CFNetSmoothTestActor_1 Seq=775 Time=51.406 Loc=X=1430.000 Y=-20.000 Z=0.000 Rot=R(0) Applied=true
```

판정:

- Dedicated Server에서 Teleport 테스트 명령줄 override가 동작했다.
- Client1, Client2 모두 `[CFNetSmooth][Teleport]`를 반복 수신했다.
- Teleport 로그는 모두 `Applied=true`였다.
- Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0건이었다.
- 각 클라이언트에서 Snap 1회가 발생했는데, 접속 직후 이미 이동된 서버 위치를 따라잡는 초기 보정으로 본다.

주의:

- 이 결과는 패킷 손실 조건 검증이 아니다.
- Teleport State가 Unreliable 손실에서 누락될 수 있어 v0.4.0에서 Reliable 보조 경로를 추가했다.

---

### 5.13 Dedicated Teleport 손실 모드

선행 손실 확인:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 teleportloss 3.0 20
```

Reliable 보강 전 로그 폴더:

```text
RuntimeLogs/CFNetSmoothDedicated/20260617_144715
```

Reliable 보강 전 확인:

| 파일 | Receive | Teleport | Snap | Extrapolate | SendTeleportTrue | ReceiveTeleportTrue | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Server.log | 0 | 0 | 0 | 0 | 17 | 0 | 0 | 0 | 0 |
| Client1.log | 506 | 12 | 3 | 116 | 0 | 12 | 0 | 0 | 0 |
| Client2.log | 448 | 11 | 0 | 99 | 0 | 11 | 0 | 0 | 0 |

판정:

- 일반 Transform과 Teleport를 모두 Unreliable로 보낼 경우 손실 조건에서 Teleport State가 일부 누락될 수 있다.
- Teleport는 오래된 State보다 이벤트 누락 자체가 더 위험하므로 일반 Transform과 분리한 Reliable 경로가 필요하다.

Reliable 보강 후 실행 명령:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 teleportloss 3.0 20
```

Reliable 보강 후 로그 폴더:

```text
RuntimeLogs/CFNetSmoothDedicated/20260617_145350
```

Reliable 보강 후 결과:

| 파일 | Send | Receive | Teleport | Snap | Extrapolate | SendTeleportTrue | ReceiveTeleportTrue | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Server.log | 796 | 0 | 0 | 0 | 0 | 17 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 515 | 14 | 5 | 101 | 0 | 14 | 0 | 0 | 0 |
| Client2.log | 0 | 443 | 13 | 3 | 92 | 0 | 13 | 0 | 0 | 0 |

판정:

- Teleport State만 Reliable NetMulticast로 분리한 뒤에도 일반 Receive는 손실 조건에서 계속 들어왔다.
- Client1, Client2 모두 `ReceiveTeleportTrue`와 `[Teleport]` 적용 수가 동일했다.
- 클라이언트별 Teleport 수가 서버의 17회와 같지 않은 것은 클라이언트가 서버 시작 후 순차 접속하기 때문이다.
- Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0건이었다.
- 20% 손실 + Teleport 조건의 전용 자동 검증은 통과로 본다.

---

### 5.14 Dedicated Teleport 손실+지연 모드

선행 복합 조건 확인:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 teleportloss 3.0 20 100
```

지연 도착 Teleport 보강 전 로그 폴더:

```text
RuntimeLogs/CFNetSmoothDedicated/20260617_145832
```

보강 전 결과:

| 파일 | Receive | Teleport | Snap | Extrapolate | SendTeleportTrue | ReceiveTeleportTrue | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Server.log | 0 | 0 | 0 | 0 | 17 | 0 | 0 | 0 | 0 |
| Client1.log | 504 | 13 | 5 | 143 | 0 | 13 | 0 | 0 | 0 |
| Client2.log | 446 | 10 | 4 | 117 | 0 | 10 | 0 | 0 | 0 |

보강 전 Sequence 비교:

```text
Server Teleport Seq: 44, 89, 134, 177, 222, 268, 314, 360, 405, 450, 496, 542, 588, 634, 680, 726, 772
Client1 missing from first client teleport onward: 222
Client2 missing from first client teleport onward: 360, 496
```

판정:

- Reliable Teleport RPC가 지연되어 최신 일반 State보다 늦게 도착하면 기존 `ServerTime` 폐기 조건에 걸릴 수 있었다.
- 이 문제는 RPC 신뢰성 문제가 아니라 클라이언트 수신 처리 순서 문제다.
- Teleport State는 오래된 일반 State와 다르게 늦게 도착해도 Buffer 정리 이벤트로 처리해야 한다.

지연 도착 Teleport 보강 후 실행 명령:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 teleportloss 3.0 20 100
```

지연 도착 Teleport 보강 후 로그 폴더:

```text
RuntimeLogs/CFNetSmoothDedicated/20260617_150128
```

보강 후 결과:

| 파일 | Send | Receive | Teleport | Snap | Extrapolate | SendTeleportTrue | ReceiveTeleportTrue | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Server.log | 785 | 0 | 0 | 0 | 0 | 17 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 506 | 14 | 3 | 124 | 0 | 14 | 0 | 0 | 0 |
| Client2.log | 0 | 444 | 12 | 2 | 92 | 0 | 12 | 0 | 0 | 0 |

보강 후 Sequence 비교:

```text
Client1 missing from first client teleport onward:
Client2 missing from first client teleport onward:
```

판정:

- 각 클라이언트의 첫 Teleport 이후 중간 Teleport Sequence 누락은 0건이었다.
- `ReceiveTeleportTrue`와 `[Teleport]` 적용 수가 일치했다.
- Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0건이었다.
- 20% 손실 + 100ms 지연 + Teleport 복합 조건 자동 검증은 통과로 본다.

---

### 5.15 Dedicated 일반 Transform 손실+지연 모드

실행 명령:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 loss 0.0 20 100
```

로그 폴더:

```text
RuntimeLogs/CFNetSmoothDedicated/20260617_151132
```

결과:

| 파일 | Send | Receive | Teleport | Snap | Extrapolate | RotSnap | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Server.log | 795 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 510 | 0 | 0 | 116 | 0 | 0 | 0 | 0 |
| Client2.log | 0 | 442 | 0 | 0 | 98 | 0 | 0 | 0 | 0 |

외삽 시간 확인:

| 파일 | Extrapolate Count | Max Observed Time | MaxExtrapTime |
|---|---:|---:|---:|
| Client1.log | 116 | 0.001 | 0.200 |
| Client2.log | 98 | 0.001 | 0.200 |

판정:

- 일반 Transform State는 Unreliable이므로 손실 조건에서 Receive 수가 Send 수보다 적은 것이 정상이다.
- 20% 손실 + 100ms 지연에서도 Snap, RotSnap, Teleport는 0회였다.
- Extrapolate 로그는 발생했지만 실제 관측 시간이 `0.001초`로 `MaxExtrapTime=0.200`보다 충분히 낮았다.
- Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0건이었다.
- 현재 기본값 `InterpBackTime=0.10`, `MaxExtrapTime=0.20`, `PosSnapDist=500`, `RotSnapDeg=90`은 테스트 Actor 기준 유지해도 된다.

---

### 5.16 Dedicated 일반 Transform 지연 단독 모드

실행 명령:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 loss 0.0 0 100
```

로그 폴더:

```text
RuntimeLogs/CFNetSmoothDedicated/20260617_151436
```

결과:

| 파일 | Send | Receive | Teleport | Snap | Extrapolate | RotSnap | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Server.log | 798 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 647 | 0 | 0 | 66 | 0 | 0 | 0 | 0 |
| Client2.log | 0 | 570 | 0 | 0 | 56 | 0 | 0 | 0 | 0 |

외삽 시간 확인:

| 파일 | Extrapolate Count | Max Observed Time | MaxExtrapTime |
|---|---:|---:|---:|
| Client1.log | 66 | 0.006 | 0.200 |
| Client2.log | 56 | 0.001 | 0.200 |

판정:

- 100ms 지연 단독 조건에서도 Snap, RotSnap, Teleport는 0회였다.
- Extrapolate 로그는 발생했지만 최대 관측 시간이 `0.006초`로 `MaxExtrapTime=0.200`보다 충분히 낮았다.
- Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0건이었다.
- 현재 기본값은 테스트 Actor 기준 100ms 지연 단독 조건에서도 유지해도 된다.

---

### 5.17 Dedicated 일반 Transform 손실 단독 모드

실행 명령:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 loss 0.0 20 0
```

로그 폴더:

```text
RuntimeLogs/CFNetSmoothDedicated/20260617_151740
```

결과:

| 파일 | Send | Receive | Teleport | Snap | Extrapolate | RotSnap | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Server.log | 798 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 530 | 0 | 0 | 108 | 0 | 0 | 0 | 0 |
| Client2.log | 0 | 457 | 0 | 0 | 88 | 0 | 0 | 0 | 0 |

외삽 시간 확인:

| 파일 | Extrapolate Count | Max Observed Time | MaxExtrapTime |
|---|---:|---:|---:|
| Client1.log | 108 | 0.007 | 0.200 |
| Client2.log | 88 | 0.001 | 0.200 |

판정:

- 20% 손실 단독 조건에서도 Snap, RotSnap, Teleport는 0회였다.
- Extrapolate 로그는 발생했지만 최대 관측 시간이 `0.007초`로 `MaxExtrapTime=0.200`보다 충분히 낮았다.
- Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0건이었다.
- 현재 기본값은 테스트 Actor 기준 20% 손실 단독 조건에서도 유지해도 된다.

---

### 5.18 Dedicated Teleport 손실+지연 반복성

실행 명령:

```text
Tools\RunCFNetSmoothDedicated.bat 45 2 teleportloss 3.0 20 100
```

반복 기준:

```text
보강 직후 1회 + 추가 반복 2회 = 총 3회
```

로그 폴더:

```text
RuntimeLogs/CFNetSmoothDedicated/20260617_150128
RuntimeLogs/CFNetSmoothDedicated/20260617_152133
RuntimeLogs/CFNetSmoothDedicated/20260617_152316
```

결과:

| 실행 | Server Teleport | Client1 Teleport | Client2 Teleport | Client1 Missing After First Teleport | Client2 Missing After First Teleport | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| 20260617_150128 | 17 | 14 | 12 | 0 | 0 | 0 | 0 | 0 |
| 20260617_152133 | 17 | 14 | 12 | 0 | 0 | 0 | 0 | 0 |
| 20260617_152316 | 17 | 14 | 12 | 0 | 0 | 0 | 0 | 0 |

판정:

- 3회 모두 Client1과 Client2의 첫 Teleport 이후 중간 Teleport Sequence 누락은 0건이었다.
- Client별 Teleport 수가 서버보다 적은 것은 서버 시작 후 클라이언트가 순차 접속하기 때문에 정상이다.
- 3회 모두 Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0건이었다.
- 지연 도착 Teleport 수신 보강은 20% 손실 + 100ms 지연 조건에서 반복성 기준을 통과했다.

---

### 5.19 Win64 Development 패키징

실행 명령:

```text
D:\UnrealEngine_Source\Engine\Build\BatchFiles\RunUAT.bat BuildCookRun -NoP4 -project=D:\Work\CarFight_git\UE\CarFight_Re.uproject -target=CarFight_Re -platform=Win64 -clientconfig=Development -build -cook -stage -pak -archive -archivedirectory=D:\Work\CarFight_git\RuntimeLogs\CFNetSmoothPackage\20260617_152707 -map=/Game/Maps/TestMap -unattended -utf8output
```

패키징 결과 폴더:

```text
RuntimeLogs/CFNetSmoothPackage/20260617_152707
```

주요 결과:

| 항목 | 결과 |
|---|---|
| BuildCookRun | 성공 |
| AutomationTool ExitCode | 0 |
| Cook | Success - 0 error(s), 0 warning(s) |
| Stage | 완료 |
| Archive | 완료 |
| 패키지 실행 스모크 | ExitCode 0 |

산출물 확인:

| 파일 | 크기 |
|---|---:|
| `Windows/CarFight_Re.exe` | 167424 bytes |
| `Windows/CarFight_Re/Binaries/Win64/CarFight_Re.exe` | 292767232 bytes |
| `Windows/CarFight_Re/Content/Paks/CarFight_Re-Windows.pak` | 11034252 bytes |
| `Windows/CarFight_Re/Content/Paks/CarFight_Re-Windows.ucas` | 295166016 bytes |
| `Windows/CarFight_Re/Content/Paks/CarFight_Re-Windows.utoc` | 226710 bytes |

패키지 스모크 실행:

```text
RuntimeLogs/CFNetSmoothPackage/20260617_152707/Windows/CarFight_Re.exe -NullRHI -unattended -NoSound -log -Abslog=RuntimeLogs/CFNetSmoothPackage/20260617_152707/PackageSmoke.log -ExecCmds=Quit
```

스모크 확인:

```text
PackageSmokeExitCode=0
LogPluginManager: Mounting Project plugin CFNetSmooth
LogInit: Display: Engine is initialized. Leaving FEngineLoop::Init()
LogExit: Exiting.
```

판정:

- `CFNetSmooth`는 Editor 전용이 아니라 Win64 Development Game 패키징에서도 컴파일됐다.
- 패키징된 실행 파일이 `CFNetSmooth` 프로젝트 플러그인을 Mount하고 Engine 초기화 후 정상 종료했다.
- TestMap 최소 패키징 기준으로 플러그인 구조, Runtime 모듈, pak/stage/archive 경로는 통과로 본다.
- Shipping 패키징은 아직 별도 검증 대상이며, 서버 패키징 결과는 5.20장에 별도로 기록한다.

---

### 5.20 Win64 Development Server 패키징

실행 명령:

```text
D:\UnrealEngine_Source\Engine\Build\BatchFiles\RunUAT.bat BuildCookRun -NoP4 -project=D:\Work\CarFight_git\UE\CarFight_Re.uproject -server -noclient -serverplatform=Win64 -serverconfig=Development -servertarget=CarFight_ReServer -build -cook -stage -pak -archive -archivedirectory=D:\Work\CarFight_git\RuntimeLogs\CFNetSmoothServerPackage\20260617_153553 -map=/Game/Maps/TestMap -unattended -utf8output
```

서버 패키징 결과 폴더:

```text
RuntimeLogs/CFNetSmoothServerPackage/20260617_153553
```

주요 결과:

| 항목 | 결과 |
|---|---|
| Server BuildCookRun | 성공 |
| AutomationTool ExitCode | 0 |
| Cook TargetPlatform | WindowsServer |
| Cook | Success - 0 error(s), 0 warning(s) |
| Stage | 완료 |
| Archive | 완료 |
| 서버 패키지 실행 스모크 | ExitCode 0 |

산출물 확인:

| 파일 | 크기 |
|---|---:|
| `WindowsServer/CarFight_ReServer.exe` | 167424 bytes |
| `WindowsServer/CarFight_Re/Binaries/Win64/CarFight_ReServer.exe` | 270866944 bytes |
| `WindowsServer/CarFight_Re/Content/Paks/CarFight_Re-WindowsServer.pak` | 9942833 bytes |
| `WindowsServer/CarFight_Re/Content/Paks/CarFight_Re-WindowsServer.ucas` | 107233056 bytes |
| `WindowsServer/CarFight_Re/Content/Paks/CarFight_Re-WindowsServer.utoc` | 79018 bytes |

서버 패키지 스모크 실행:

```text
RuntimeLogs/CFNetSmoothServerPackage/20260617_153553/WindowsServer/CarFight_ReServer.exe /Game/Maps/TestMap -unattended -NoSound -log -Abslog=RuntimeLogs/CFNetSmoothServerPackage/20260617_153553/ServerPackageSmoke.log -ExecCmds=Quit
```

스모크 확인:

```text
ServerPackageSmokeExitCode=0
LogPluginManager: Mounting Project plugin CFNetSmooth
LogLoad: Game class is 'CFMPGameMode'
LogNet: IpNetDriver listening on port 7777
LogWorld: Bringing World /Game/Maps/TestMap.TestMap up for play
LogExit: Exiting.
```

판정:

- `CFNetSmooth`는 Dedicated Server 타깃에서도 컴파일됐다.
- 패키징된 서버 실행 파일이 `CFNetSmooth` 프로젝트 플러그인을 Mount하고 TestMap을 서버 월드로 열었다.
- `GameNetDriver`가 7777 포트로 열렸고, `-ExecCmds=Quit`로 정상 종료했다.
- TestMap 최소 서버 패키징 기준으로 Runtime 모듈, Server 타깃, pak/stage/archive 경로는 통과로 본다.
- Shipping 패키징은 아직 별도 검증 대상이다.

---

### 5.21 Win64 Development 패키지 Server-Client 접속 검증

실행 대상:

```text
Server: RuntimeLogs/CFNetSmoothServerPackage/20260617_153553/WindowsServer/CarFight_ReServer.exe
Client: RuntimeLogs/CFNetSmoothPackage/20260617_152707/Windows/CarFight_Re.exe
```

실행 조건:

```text
서버: /Game/Maps/TestMap -unattended -NoSound -log
클라이언트: 127.0.0.1 -NullRHI -unattended -NoSound -log
클라이언트 수: 2개
실행 시간: 약 45초
```

로그 폴더:

```text
RuntimeLogs/CFNetSmoothPackageNet/20260617_154137
```

결과:

| 파일 | Send | Receive | Teleport | Snap | Extrapolate | RotSnap | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Server.log | 1355 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| Client1.log | 0 | 1118 | 0 | 0 | 7 | 0 | 0 | 0 | 0 |
| Client2.log | 0 | 1097 | 0 | 0 | 4 | 0 | 0 | 0 | 0 |

접속 확인:

```text
Server: LogPluginManager: Mounting Project plugin CFNetSmooth
Server: LogNet: IpNetDriver listening on port 7777
Client1: Welcomed by server
Client1: LoadMap /Game/Maps/TestMap
Client2: Welcomed by server
Client2: LoadMap /Game/Maps/TestMap
```

CFNetSmooth 로그 범위:

| 대상 | 첫 CFNetSmooth 로그 | 마지막 CFNetSmooth 로그 |
|---|---|---|
| Server | Seq=0 Time=0.100 | Seq=1285 Time=85.598 |
| Client1 | Seq=169 Time=11.295 | Seq=1287 Time=85.733 |
| Client2 | Seq=238 Time=15.895 | Seq=1289 Time=85.865 |

판정:

- 패키징된 Dedicated Server와 패키징된 Development Client 사이에서 실제 접속이 성립했다.
- 서버는 `[CFNetSmooth][Send]`를 반복 출력했고, 두 클라이언트는 `[CFNetSmooth][Receive]`를 반복 수신했다.
- Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0건이었다.
- `-NullRHI` 클라이언트 기준이므로 화면 체감 검증은 포함하지 않는다.
- Shipping 패키징과 가시 클라이언트 화면 체감은 아직 별도 검증 대상이다.

---

### 5.22 Win64 Development 패키지 가시 클라이언트 검증

실행 대상:

```text
Server: RuntimeLogs/CFNetSmoothServerPackage/20260617_153553/WindowsServer/CarFight_ReServer.exe
Client: RuntimeLogs/CFNetSmoothPackage/20260617_152707/Windows/CarFight_Re.exe
```

실행 조건:

```text
서버: /Game/Maps/TestMap -unattended -NoSound -log
클라이언트: 127.0.0.1 -windowed -ResX=960 -ResY=540 -NoSound -log
클라이언트 수: 1개
렌더링 조건: NullRHI 사용 안 함
최종 로그 범위: 약 130초
```

로그 폴더:

```text
RuntimeLogs/CFNetSmoothVisualClient/20260617_154800
```

화면 캡처:

```text
RuntimeLogs/CFNetSmoothVisualClient/20260617_154800/VisualClient_10s.png
RuntimeLogs/CFNetSmoothVisualClient/20260617_154800/VisualClient_25s.png
RuntimeLogs/CFNetSmoothVisualClient/20260617_154800/VisualClient_40s.png
```

결과:

| 파일 | Send | Receive | Teleport | Snap | Extrapolate | Max Extrapolate Time | RotSnap | Fatal/Ensure | Error | Timeout |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| Server.log | 1673 | 0 | 0 | 0 | 0 | 0.000 | 0 | 0 | 0 | 0 |
| ClientVisible.log | 0 | 1482 | 0 | 0 | 230 | 0.199 | 0 | 0 | 0 | 0 |

접속 확인:

```text
ClientVisible.log: Welcomed by server
ClientVisible.log: LoadMap /Game/Maps/TestMap
Server.log: IpNetDriver listening on port 7777
```

CFNetSmooth 로그 범위:

| 대상 | 첫 CFNetSmooth 로그 | 마지막 CFNetSmooth 로그 |
|---|---|---|
| Server | Seq=0 Time=0.101 | Seq=1672 Time=130.122 |
| ClientVisible | Seq=190 Time=12.823 | Seq=1671 Time=130.055 |

판정:

- NullRHI 없이 패키지 클라이언트 창이 열리고 TestMap이 렌더링됐다.
- 화면 캡처에서 차량, TestMap 바닥, 테스트 구조물이 보였다.
- 서버 Send와 클라이언트 Receive가 긴 구간 동안 이어졌다.
- Snap, RotSnap, Teleport는 0회였다.
- Extrapolate는 발생했지만 최대 관측 시간이 `0.199초`로 `MaxExtrapTime=0.200` 제한 안에 있었다.
- Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0건이었다.
- 자동 캡처는 미세한 부드러움까지 판정하지 못하므로, 실제 플레이어 눈 기준 체감 평가는 별도로 남긴다.

주의:

- 이번 검증은 일반 이동/회전 가시 클라이언트 최소 검증이다.
- Teleport 가시 체감, MaxExtrapTime 이후 정지 체감, 사용자 직접 육안 평가는 별도 검증 대상이다.

---

### 5.23 CFNetSmooth 0.12.0 차량 적용 준비 API 검증

변경 목적:

```text
차량 적용 세션에서 실제 차량 Actor를 직접 움직이지 않고
원격 Visual/Shell 표시 대상에 보간 Transform을 적용할 수 있게 한다.
```

추가된 대표 API:

```text
TryGetSmoothedTransform
HasReceivedState
GetBufferedStateCount
GetLastReceivedSequence
IsUsingExtrapolation
WasLastReceivedStateTeleport
```

빌드 명령:

```text
Tools\BuildEditor.bat
```

빌드 결과:

```text
Result: Succeeded
UnrealEditor-CFNetSmooth.dll 링크 완료
CFNetSmoothComp.cpp, CFNetSmoothTestActor.cpp, Module.CFNetSmooth.cpp 컴파일 완료
```

판정:

- 공개 UFUNCTION 추가 후 UHT와 C++ 컴파일을 통과했다.
- 차량 Visual/Shell 적용 세션은 `bApplyReceivedStateToOwner=false`를 유지하고 `TryGetSmoothedTransform` 결과를 별도 표시 대상에 적용할 수 있다.
- 이 API는 Owner Actor를 직접 움직이지 않는다.
- SceneComponent를 플러그인이 직접 움직이는 API와 delegate는 아직 보류 상태다.

추가 Dedicated 스모크 실행:

```text
Tools\RunCFNetSmoothDedicated.bat 10 1
Tools\RunCFNetSmoothDedicated.bat 20 1
```

로그 폴더:

```text
RuntimeLogs\CFNetSmoothDedicated\20260618_085442
RuntimeLogs\CFNetSmoothDedicated\20260618_085541
```

확인 결과:

- `CFNetSmooth` 프로젝트 플러그인 Mount는 정상이다.
- 서버 `TestMap` 로드와 `GameNetDriver` 7777 포트 Listen은 정상이다.
- 클라이언트 접속과 `TestMap` 로드는 정상이다.
- Fatal, Ensure, Assertion, Error, Timeout, NetworkFailure는 0건이다.
- 그러나 `[CFNetSmooth][Send]`, `[CFNetSmooth][Receive]` 로그가 0건이라 송수신 동작 판정에는 사용하지 않는다.

보류 사유:

```text
현재 TestMap 또는 CFNetSmoothTestActor 저장값에서 Enable Debug Log가 꺼져 있거나,
자동 Dedicated 실행 조건에서 CFNetSmooth 로그가 출력되지 않는 상태로 추정한다.
같은 10초/20초 스모크를 반복하지 말고 DebugLog 조건을 먼저 확인해야 한다.
```

후속 선택지:

```text
1. 현재 0.12.0 API 작업은 BuildEditor 성공으로 통과 처리한다.
2. 자동 로그 검증 안정화가 필요하면 SetDebugLogEnabled API 또는 -CFNetSmoothDebugLog 명령줄 override를 별도 작업으로 추가한다.
```

---

### 5.24 CFNetSmooth 0.12.1 범용 플러그인 메타데이터 검증

변경 목적:

```text
CFNetSmooth를 CarFight 전용 플러그인이 아니라 여러 UE 프로젝트에서 재사용 가능한 Runtime 플러그인으로 유지한다.
```

변경 내용:

```text
CFNetSmooth.uplugin VersionName=0.12.1
Description에서 CarFight 전용 표현 제거
CreatedBy를 CFNetSmooth로 정리
CFNetSmooth.Build.cs에서 CarFight_Re 모듈 의존성 표현 제거
```

확인 명령:

```text
rg --line-number --fixed-strings "CarFight" UE/Plugins/CFNetSmooth
Tools\BuildEditor.bat
```

확인 결과:

```text
UE/Plugins/CFNetSmooth 내부 CarFight 문자열 0건
Result: Succeeded
Target is up to date
```

판정:

- 플러그인 Runtime 폴더 안에서 CarFight 전용 문자열은 제거됐다.
- `.uplugin` JSON 파싱과 `VersionName=0.12.1` 확인이 정상이다.
- `Tools\BuildEditor.bat`가 성공했으므로 manifest/Build.cs 변경은 에디터 빌드 기준 통과다.
- CarFight 검증 스크립트, RuntimeLogs, TestMap 검증 기록은 플러그인 필수 구성물이 아니라 현재 호스트 프로젝트 검증 자료로 취급한다.

---

## 6. 현재 통과 기준

테스트 Actor 기준으로 아래 항목은 통과로 본다.

```text
플러그인 에디터 빌드
컴포넌트 에디터 표시
서버 State 송신
클라이언트 State 수신
State Buffer 제한
위치 보간 입력 State 확보
Teleport 즉시 적용
위치 Snap
제한 외삽 진입
회전 송수신
회전 Snap
Dedicated Server 기본 반복 송수신
Dedicated 자동 실행 스크립트 스모크 테스트
Dedicated 자동 반복 45초/2클라이언트
Dedicated Teleport 모드 기본 검증
Dedicated Teleport 손실 모드 20%
Dedicated Teleport 손실 20% + 지연 100ms 복합 모드
Dedicated Teleport 손실 20% + 지연 100ms 반복 3회
Dedicated 일반 Transform 손실 20% + 지연 100ms 복합 모드
Dedicated 일반 Transform 지연 100ms 단독 모드
Dedicated 일반 Transform 손실 20% 단독 모드
Win64 Development Game 패키징과 패키지 스모크 실행
Win64 Development Server 패키징과 서버 스모크 실행
Win64 Development 패키지 서버 1개 + 패키지 클라이언트 2개 접속 송수신
Win64 Development 패키지 가시 클라이언트 렌더링/접속/송수신 최소 검증
테스트 Actor 기준 v1.0 Release Candidate 판정
차량 Visual/Shell용 읽기 전용 보간 Transform API 에디터 빌드
범용 플러그인 메타데이터 에디터 빌드
```

---

## 7. 아직 남은 검증

아래 항목은 릴리즈 후보 판정을 막는 필수 검증이 아니라, 환경이나 적용 범위가 바뀔 때 수행할 후속 검증이다.

```text
Shipping 패키징 확인
실제 차량 Visual/Shell 적용 검증
패키지 가시 클라이언트 사용자 직접 체감 확인
Teleport 가시 클라이언트 체감 확인
MaxExtrapTime 이후 정지 체감 확인
회전 보간 화면 체감 반복 확인
코드/기본값/환경 변경 후 최소 재검증
0.12.0 이후 Dedicated 로그 스모크를 재사용하려면 DebugLog 조건 또는 명령줄 override 확인
```

---

## 8. 차량 적용 전 조건

차량 Pawn에 연결하기 전 아래 조건을 만족해야 한다.

- 테스트 Actor 기준 검증 결과가 현재 문서에 기록되어 있어야 한다.
- `Presets.md`의 기본 안전값과 복구값을 확인해야 한다.
- CFNetSmooth 연결 전에는 기존 차량 기준선인 `Default + RepMove=true + AutoPhysics=true`를 유지해야 한다.
- CFNetSmooth가 실제 차량 Transform 복제 역할을 맡는 적용 단계에서는 `RepMove=false` 전환을 별도 패치와 별도 검증으로 수행해야 한다.
- Visual/Shell 적용 설계서를 먼저 작성해야 한다.
- 롤백 절차를 작성해야 한다.
- 기존 `ACFVehiclePawn` Transform, 물리, RepMove 흐름을 말 없이 바꾸지 않아야 한다.

---

## 9. 다음 권장 작업

다음 작업은 같은 조건의 반복 검증이 아니라, 릴리즈 후보 상태를 유지한 채 다음 적용 범위로 넘어가는 것이다.

권장 순서:

```text
1. ReleaseCandidate.md를 기준 문서로 삼고 CFNetSmooth 기능을 동결한다.
2. 코드/기본값/환경 변경이 없으면 같은 조건의 Dedicated/패키지 검증을 반복하지 않는다.
3. 차량 적용 세션은 VehiclePrep.md를 먼저 읽고 Visual/Shell 설계를 진행한다.
4. 배포 준비가 필요할 때만 Win64 Shipping 패키징을 별도 작업으로 진행한다.
5. Dedicated 로그 Summary가 0으로 나오면 같은 실행을 반복하지 말고 Enable Debug Log 조건부터 확인한다.
```

---

## 10. Changelog

### v0.15.1

```text
- CFNetSmooth 0.12.1 범용 플러그인 메타데이터 검증 결과 추가
- UE/Plugins/CFNetSmooth 내부 CarFight 문자열 0건과 Tools/BuildEditor.bat 성공 기록
- CarFight 검증 스크립트와 RuntimeLogs는 플러그인 필수 구성물이 아니라 호스트 프로젝트 검증 자료로 분리
```

### v0.15.0

```text
- CFNetSmooth 0.12.0 차량 Visual/Shell용 읽기 전용 Transform API 빌드 성공 결과 추가
- 10초/20초 Dedicated 스모크에서 접속/맵 로드는 정상이나 CFNetSmooth 로그 0건이라 송수신 판정을 보류한 사실 기록
- 같은 Dedicated 스모크 반복 대신 DebugLog 조건 또는 명령줄 override를 먼저 확인하도록 후속 기준 추가
```

### v0.14.0

```text
- 테스트 Actor 기준 v1.0 Release Candidate 판정을 현재 통과 기준에 추가
- 남은 검증을 릴리즈 후보 필수 조건이 아니라 적용 범위/환경 변경 시 후속 검증으로 재분류
- 다음 권장 작업을 반복 검증 중단과 ReleaseCandidate.md 기준 기능 동결로 갱신
```

### v0.13.1

```text
- 차량 적용 전 조건에서 RepMove=true 기준선을 CFNetSmooth 연결 전 기준선으로 한정
- 실제 차량 CFNetSmooth 적용 단계에서는 RepMove=false 전환이 필요함을 명시
```

### v0.13.0

```text
- Win64 Development 패키지 가시 클라이언트 검증 결과 추가
- NullRHI 없이 패키지 클라이언트 창 실행, 화면 캡처 3장 생성, 서버 Send 1673회와 클라이언트 Receive 1482회 확인
- Snap/RotSnap/Fatal/Error/Timeout 0건을 기록하고, 미세한 화면 부드러움 평가는 사용자 직접 체감 검증으로 분리
```

### v0.12.0

```text
- Win64 Development 패키지 Server-Client 접속 검증 결과 추가
- 패키지 서버 1개와 패키지 클라이언트 2개 사이에서 CFNetSmooth Send/Receive 반복 수신을 확인
- Fatal/Error/Timeout 0건 기준으로 패키지 네트워크 경로를 통과 처리하고, 화면 체감/Shipping 검증은 별도 남은 작업으로 분리
```

### v0.11.0

```text
- Win64 Development Server 패키징 결과 추가
- CarFight_ReServer BuildCookRun 성공, WindowsServer 산출물, 서버 패키지 스모크 실행 ExitCode 0 기록
- 서버 스모크에서 CFNetSmooth 플러그인 Mount, TestMap 로드, GameNetDriver 7777 포트 시작 확인
```

### v0.10.0

```text
- Win64 Development Game 패키징 결과 추가
- BuildCookRun 성공, pak/stage/archive 산출물, 패키지 스모크 실행 ExitCode 0 기록
- 패키징 빌드 검증을 완료 항목으로 이동하고 Shipping/Server 패키징은 별도 남은 검증으로 분리
```

### v0.9.0

```text
- Teleport 손실 20% + 지연 100ms 조건 3회 반복 결과 추가
- 각 클라이언트 첫 Teleport 이후 Sequence 누락 0건 반복 판정 기록
- 남은 검증 항목에서 Teleport 손실+지연 반복 확인을 장시간 확인으로 조정
```

### v0.8.0

```text
- 일반 Transform 20% 손실 단독 검증 결과 추가
- Snap 0회, 최대 외삽 0.007초, Fatal/Error/Timeout 0건 기준으로 기본 보간값 유지 판정 보강
- 남은 검증 항목에서 일반 Transform 손실 단독 조건을 완료 항목으로 이동
```

### v0.7.0

```text
- 일반 Transform 100ms 지연 단독 검증 결과 추가
- Snap 0회, 최대 외삽 0.006초, Fatal/Error/Timeout 0건 기준으로 기본 보간값 유지 판정 보강
- 남은 검증 항목에서 100ms 지연 단독 조건을 완료 항목으로 이동
```

### v0.6.0

```text
- 일반 Transform 20% 손실 + 100ms 지연 복합 검증 결과 추가
- Snap 0회, 최대 외삽 0.001초, Fatal/Error/Timeout 0건 기준으로 기본 보간값 유지 판정 기록
- 남은 검증 항목에서 일반 Transform 복합 조건을 단독 loss/lag 반복 확인으로 조정
```

### v0.5.0

```text
- 20% 손실 + 100ms 지연 + Teleport 복합 조건 검증 결과 추가
- 지연 도착한 Reliable Teleport State가 최신 일반 State 뒤에서 폐기되던 문제와 수정 후 Sequence 누락 0건 결과 기록
- Dedicated Teleport 손실+지연 모드를 통과 기준에 추가
```

### v0.4.0

```text
- Teleport 손실 모드 검증 결과 추가
- Unreliable Teleport 누락 가능성과 Reliable Teleport 보강 후 재검증 결과를 분리 기록
- Tools/RunCFNetSmoothDedicated.bat 45 2 teleportloss 3.0 20 결과와 SendTeleportTrue/ReceiveTeleportTrue 기준 추가
```

### v0.3.0

```text
- Dedicated Teleport 모드 검증 결과 추가
- RunCFNetSmoothDedicated.bat 45 2 teleport 3.0 결과와 Teleport 수신 횟수 기록
- 접속 직후 Snap 1회는 초기 보정으로 해석하고, 손실 조건 Teleport 보강 여부는 미결로 유지
```

### v0.2.2

```text
- Tools/RunCFNetSmoothDedicated.bat 45 2 기준 3회 반복 검증 결과 추가
- 서버 Send, 클라이언트 Receive, Fatal/Error/Timeout 0건 반복 판정 기록
- 기본 프리셋 상태에서 Teleport/Snap/RotSnap 로그 0건이 정상이라는 해석 추가
```

### v0.2.1

```text
- Tools/RunCFNetSmoothDedicated.bat 스모크 테스트 결과 추가
- Summary.txt 생성 확인과 자동 실행 스크립트 통과 판정 추가
- 다음 권장 작업을 차량 적용 설계가 아니라 플러그인 자체 검증 확장으로 갱신
```

### v0.2.0

```text
- Dedicated Server 기본 반복 검증 결과 추가
- 서버 2회, 클라이언트 2개씩 CFNetSmooth Send/Receive 로그 수집 결과 기록
- Fatal/Error/Timeout 0건 기준으로 Dedicated 기본 송수신 통과 판정 추가
- 남은 검증 항목을 Dedicated 장시간/가시 클라이언트/지연/손실 검증으로 구체화
```

### v0.1.0

```text
- CFNetSmoothTestActor 기준 전체 검증 결과 문서 최초 작성
- Send/Receive, Teleport, 위치 Snap, 제한 외삽, 회전 송수신, 회전 Snap 결과 기록
- 아직 남은 Dedicated Server, 지연/손실, 차량 Visual/Shell 검증 범위 분리
```

---

## 11. 마이그레이션 지침

- 이 문서는 테스트 결과 기록이며, 차량 적용 지시서가 아니다.
- 차량 적용은 이 문서만 보고 진행하지 않는다.
- 차량 적용 전 Visual/Shell 설계서와 롤백 절차를 별도로 작성한다.
- 이후 검증 결과가 추가되면 기존 결과를 삭제하지 말고 새 버전 Changelog를 추가한다.
