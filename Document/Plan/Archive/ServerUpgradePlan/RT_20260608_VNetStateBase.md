# Runtime Test 2026-06-08 - VehicleNetStateBase

- 문서 버전: v0.2.0
- 작성일: 2026-06-08
- 대상 프로젝트: CarFight
- 테스트 유형: CFVNetStateBase 적용 후 Dedicated Server 2클라 로그 확인
- 로그 경로: `RuntimeLogs/VehicleNet/`
- 관련 문서:
  - `VehicleNetStatePlan.md`
  - `VNetStateBase_LogGuide.md`
  - `CodexTasks/CFVNetStateBase_Review.md`

---

## 1. 목적

이 문서는 `CFVNetStateBase` 적용 후 `VehicleNetStateBase:` 로그가 실제 런타임에서 출력되는지 확인한 결과를 기록한다.

핵심 확인 대상:

```text
- VehicleNetStateBase 로그 출력 여부
- Seq 증가 여부
- BufferCount 증가 및 최대값 제한 여부
- ReceivedLocalTime 기록 여부
- 기존 VehicleNetDebug 로그 유지 여부
```

---

## 2. v0.1.0 테스트 결과 요약

1차 테스트에서는 `VehicleNetStateBase` 추출 파일이 생성되었지만 내용은 비어 있었다.

```text
A_RepMoveTrue/Client1_VehicleNetStateBase.txt: 0 bytes
A_RepMoveTrue/Client2_VehicleNetStateBase.txt: 0 bytes
```

원본 클라이언트 로그에서도 `VehicleNetStateBase` 문자열이 0건이었다.

판정:

```text
1차 테스트에서는 VehicleNetStateBase 런타임 검증 실패.
가장 가능성 높은 원인은 BP의 bLogVehicleNetStateBase 설정 미반영이었다.
```

---

## 3. v0.2.0 재테스트 결과

재테스트에서는 `VehicleNetStateBase:` 로그가 정상 출력되었다.

분석 대상 파일:

```text
RuntimeLogs/VehicleNet/A_RepMoveTrue/Client1_VehicleNetStateBase.txt
RuntimeLogs/VehicleNet/A_RepMoveTrue/Client2_VehicleNetStateBase.txt
RuntimeLogs/VehicleNet/B_RepMoveFalse/Client1_VehicleNetStateBase.txt
RuntimeLogs/VehicleNet/B_RepMoveFalse/Client2_VehicleNetStateBase.txt
```

---

## 4. VehicleNetStateBase 동작 확인

### 4.1 RepMove=true / Client1

대표 로그:

```text
VehicleNetStateBase: Pawn=BP_CFVehiclePawn_C_0 Role=ROLE_AutonomousProxy IsLocal=true Seq=1 BufferCount=1 ServerTimeSeconds=13.814 ReceivedLocalTimeSeconds=0.130 RepMove=true bReplicates=true
VehicleNetStateBase: Pawn=BP_CFVehiclePawn_C_0 Role=ROLE_AutonomousProxy IsLocal=true Seq=16 BufferCount=12 ServerTimeSeconds=14.987 ReceivedLocalTimeSeconds=1.147 RepMove=true bReplicates=true
VehicleNetStateBase: Pawn=BP_CFVehiclePawn_C_1 Role=ROLE_SimulatedProxy IsLocal=false Seq=1 BufferCount=1 ServerTimeSeconds=19.213 ReceivedLocalTimeSeconds=5.389 RepMove=true bReplicates=true
VehicleNetStateBase: Pawn=BP_CFVehiclePawn_C_1 Role=ROLE_SimulatedProxy IsLocal=false Seq=14 BufferCount=12 ServerTimeSeconds=20.344 ReceivedLocalTimeSeconds=6.459 RepMove=true bReplicates=true
```

판정:

```text
- AutonomousProxy와 SimulatedProxy 모두 NetState 수신 로그가 출력된다.
- Seq가 증가한다.
- BufferCount가 1에서 12까지 증가 후 12로 유지된다.
- ReceivedLocalTimeSeconds가 증가한다.
- RepMove=true가 테스트 설정과 일치한다.
```

### 4.2 RepMove=false / Client1

대표 로그:

```text
VehicleNetStateBase: Pawn=BP_CFVehiclePawn_C_0 Role=ROLE_AutonomousProxy IsLocal=true Seq=1 BufferCount=1 ServerTimeSeconds=14.515 ReceivedLocalTimeSeconds=0.165 RepMove=false bReplicates=true
VehicleNetStateBase: Pawn=BP_CFVehiclePawn_C_0 Role=ROLE_AutonomousProxy IsLocal=true Seq=14 BufferCount=12 ServerTimeSeconds=15.523 ReceivedLocalTimeSeconds=1.167 RepMove=false bReplicates=true
VehicleNetStateBase: Pawn=BP_CFVehiclePawn_C_1 Role=ROLE_SimulatedProxy IsLocal=false Seq=1 BufferCount=1 ServerTimeSeconds=19.052 ReceivedLocalTimeSeconds=4.700 RepMove=false bReplicates=true
VehicleNetStateBase: Pawn=BP_CFVehiclePawn_C_1 Role=ROLE_SimulatedProxy IsLocal=false Seq=16 BufferCount=12 ServerTimeSeconds=20.130 ReceivedLocalTimeSeconds=5.777 RepMove=false bReplicates=true
```

판정:

```text
- RepMove=false에서도 NetState 복제와 수신 버퍼 저장은 정상 동작한다.
- RepMove=false 값도 로그에 정확히 반영된다.
- SimulatedProxy 원격 차량도 NetState 버퍼를 가진다.
```

---

## 5. BufferCount 제한 확인

네 개 `VehicleNetStateBase` 추출 파일에서 다음 검색을 수행했다.

```text
BufferCount=1[3-9]
```

결과:

```text
A_RepMoveTrue/Client1: 0건
A_RepMoveTrue/Client2: 0건
B_RepMoveFalse/Client1: 0건
B_RepMoveFalse/Client2: 0건
```

판정:

```text
VehicleNetStateMaxBufferSamples = 12 제한은 정상 동작한다.
버퍼가 12를 초과해 증가하지 않는다.
```

---

## 6. VehicleNetDebug 동시 확인

기존 `VehicleNetDebug`는 계속 정상 출력되었다.

`ReceivedAge=-` 검색 결과:

```text
A_RepMoveTrue/Client1_VehicleNetDebug.txt: 0건
A_RepMoveTrue/Client2_VehicleNetDebug.txt: 0건
B_RepMoveFalse/Client1_VehicleNetDebug.txt: 0건
B_RepMoveFalse/Client2_VehicleNetDebug.txt: 0건
```

판정:

```text
AgeSplit 이후 ReceivedAge는 계속 안정적이다.
```

---

## 7. RepMove=true 관찰

`RepMove=true`에서도 기존 문제는 남아 있다.

대표 패턴:

```text
Client1 / AutonomousProxy:
- LocErr 117cm, 391cm, 311cm 등
- VelErr 349cm/s, 920cm/s, 956cm/s 등
- ServerTimeDelta가 -0.7초 ~ -3초대까지 밀리는 구간 있음
- ReceivedAge는 0.1~0.4초대 구간 확인

Client2 / SimulatedProxy 포함:
- LocErr 148cm, 152cm 등
- 원격 차량에서도 수백 cm/s 속도 오차 확인
```

해석:

```text
RepMove=true는 장기 발산을 억제하지만, 순간 위치/속도 오차와 회전 오차가 계속 발생한다.
```

---

## 8. RepMove=false 관찰

`RepMove=false`에서는 기존 결론처럼 크게 발산한다.

대표 패턴:

```text
Client1:
- LocErr 1000cm 이상 구간 다수

Client2:
- LocErr 1000cm 이상 구간 다수
- RotErr가 크게 벌어지는 구간 확인
```

해석:

```text
bReplicateMovement=false 고정은 여전히 최종 해법이 아니다.
서버/클라 차량 상태가 수 초 안에 수 m~수십 m까지 갈라질 수 있다.
```

---

## 9. 현재 결론

```text
1. CFVNetStateBase는 런타임 검증 기준 통과했다.
2. 서버 NetState 복제와 클라이언트 수신 버퍼 저장이 동작한다.
3. AutonomousProxy와 SimulatedProxy 모두 수신 버퍼를 가진다.
4. BufferCount 최대 12 제한이 동작한다.
5. ReceivedLocalTimeSeconds가 안정적으로 기록된다.
6. 기존 VehicleNetDebug 로그와 ReceivedAge도 정상 유지된다.
7. 다음 단계는 원격 차량 SimulatedProxy 전용 보간 구조 설계/작업지시서다.
```

---

## 10. 다음 코드 작업 후보

다음 코드 작업은 `CFVRemoteInterp`가 적절하다.

범위:

```text
- ROLE_SimulatedProxy 차량만 대상
- IsLocallyControlled=true 차량 제외
- HasAuthority=true 차량 제외
- VehicleNetStateBuffer의 ReceivedLocalTimeSeconds 기준으로 보간 후보 상태 계산
- 첫 구현에서는 보간 결과를 바로 강하게 스냅하지 않음
- 가능하면 시각 보간 컴포넌트/표시 계층을 우선 검토
```

주의:

```text
CFVRemoteInterp는 Transform 적용 가능성이 있는 첫 단계이므로, 작업지시서에서 매우 엄격한 제한이 필요하다.
소유 차량 예측/서버 보정은 아직 하지 않는다.
```

---

## 11. 변경 기록

### v0.2.0

```text
- VehicleNetStateBase 재테스트 성공 결과 추가
- Seq 증가, BufferCount 최대 12 제한, ReceivedLocalTimeSeconds 기록 확인
- RepMove=true / false 설정 일치 확인
- 다음 작업 후보를 CFVRemoteInterp로 갱신
```

### v0.1.0

```text
- 1차 테스트에서 VehicleNetStateBase 로그 0건 문제 기록
- BP 설정 재확인 후 재테스트 필요 기록
```
