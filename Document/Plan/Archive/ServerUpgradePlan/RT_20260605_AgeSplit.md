# Runtime Test 2026-06-05 - VehicleNet AgeSplit

- 문서 버전: v0.1.0
- 작성일: 2026-06-05
- 대상 프로젝트: CarFight
- 테스트 유형: CFVNetAgeSplit 적용 후 Dedicated Server 2클라 차량 네트워크 물리 진단
- 입력 조건: 같은 PC 2클라, 키보드 입력 기준
- 로그 경로: `RuntimeLogs/VehicleNet/`
- 관련 문서:
  - `VehicleNetPhysicsJitter.md`
  - `RT_20260605_VNetKey.md`
  - `VehicleNetStatePlan.md`
  - `CodexTasks/CFVNetAgeSplit_Review.md`

---

## 1. 목적

이 문서는 `CFVNetAgeSplit` 적용 후 새로 추가된 `ServerTimeDelta`와 `ReceivedAge` 로그를 확인하고, 다음 단계인 차량 NetState/원격 차량 보간 설계에 반영할 결론을 정리한다.

---

## 2. 테스트 세트

```text
A_RepMoveTrue
- BP_CFVehiclePawn Replicate Movement = true
- Client 1 키보드 조작

B_RepMoveFalse
- BP_CFVehiclePawn Replicate Movement = false
- Client 1 키보드 조작
```

분석 대상 파일:

```text
RuntimeLogs/VehicleNet/A_RepMoveTrue/Client1_VehicleNetDebug.txt
RuntimeLogs/VehicleNet/A_RepMoveTrue/Client2_VehicleNetDebug.txt
RuntimeLogs/VehicleNet/B_RepMoveFalse/Client1_VehicleNetDebug.txt
RuntimeLogs/VehicleNet/B_RepMoveFalse/Client2_VehicleNetDebug.txt
```

---

## 3. AgeSplit 로그 확인

`VehicleNetDebug:` 로그에 다음 필드가 정상 추가되었다.

```text
ServerTimeDelta
ReceivedAge
```

기존 `SampleAge`는 호환용으로 유지되며, 현재는 `ServerTimeDelta`와 같은 의미로 출력된다.

---

## 4. ReceivedAge 판정

네 개 추출 로그 모두에서 다음 검색 결과가 확인되었다.

```text
ReceivedAge=- : 0건
```

판정:

```text
ReceivedAge는 CFVNetAgeSplit 이후 음수로 깨지지 않았다.
OnRep 수신 로컬 시간 기반 값은 원격 차량 보간 버퍼의 1차 진단값으로 사용할 수 있다.
```

주의:

```text
ReceivedAge가 항상 0에 가까워야 하는 것은 아니다.
OnRep 직후 로그가 출력되면 0에 가깝고, Tick 로그 주기 때문에 나중에 출력되면 0.1~0.4초대 값이 나올 수 있다.
```

---

## 5. ServerTimeDelta 판정

`ServerTimeDelta`는 여전히 음수로 나오는 구간이 있다.

대표 패턴:

```text
RepMove=true / Client1:
- RuntimeReady 이전 첫 줄은 CurrentServerTimeValid=false로 ServerTimeDelta=-1.0이 정상 fallback이다.
- 이후 일부 구간에서 -0.01초대의 작은 음수가 발생한다.

RepMove=true / Client2 원격 차량:
- ServerTimeDelta가 -1초~-6초 이상으로 크게 음수가 되는 구간이 있다.

RepMove=false:
- ServerTimeDelta가 -0.9초, -1.4초, -7초, -10초 이상으로 커지는 구간이 있다.
```

판정:

```text
GameState->GetServerWorldTimeSeconds() 기반 현재 서버 시간 추정값은 진단 참고값으로는 유용하지만,
현재 단계에서 원격 차량 보간의 단독 시간축으로 쓰기에는 불안정하다.
```

---

## 6. RepMove=true 결과

### 6.1 소유 차량

Client 1 소유 차량에서는 `RepMove=true` 상태에서 다음과 같은 오차가 관찰된다.

```text
LocErr: 약 149~173cm 수준 구간 확인
VelErr: 약 598~678cm/s 수준 확인
SpeedErr: 약 21~24km/h 수준 확인
RotErr: 대체로 1도 미만
```

해석:

```text
RepMove=true는 서버 보정이 작동해 장기 발산은 제한되지만,
조작 중 위치/속도 오차가 1~2m급으로 튀는 구간이 있다.
```

### 6.2 원격 차량

Client 2 원격 차량에서는 더 큰 오차가 관찰된다.

```text
LocErr: 199cm, 269cm, 587cm, 816cm 등
RotErr: 61도 이상 구간 확인
VelErr: 1000cm/s 이상 구간 확인
AngVelErr: 49~66deg/s 구간 확인
```

해석:

```text
원격 차량은 서버 갱신을 그대로 표시하거나 기존 RepMove 보정에 의존하면 순간이동/뚝뚝 끊김이 보일 수 있는 수치 조건이 충분하다.
```

---

## 7. RepMove=false 결과

`RepMove=false`에서는 기존 결론처럼 위치/회전 발산이 매우 커진다.

대표 관찰:

```text
Client1:
- LocErr=416cm
- LocErr=921cm
- LocErr=1216cm
- LocErr=1652cm
- LocErr=1813cm
- RotErr=91도, 133도 수준까지 증가

Client2:
- LocErr=1260cm
- LocErr=1702cm
- LocErr=1595cm
- LocErr=1001cm
- RotErr=45도, 94도, 134도 수준 확인
```

판정:

```text
bReplicateMovement=false는 최종 해법이 아니다.
서버/클라 차량 상태가 수 초 안에 수 m~수십 m까지 갈라질 수 있다.
```

---

## 8. 현재 결론

```text
1. CFVNetAgeSplit은 성공했다.
2. ReceivedAge는 안정적인 로컬 수신 경과 시간으로 사용할 수 있다.
3. ServerTimeDelta는 여전히 음수/큰 오차가 있어 단독 보간 시간축으로 쓰면 위험하다.
4. 원격 차량 보간은 GameState 서버 시간 기준보다 ReceivedLocalTime 기반 버퍼로 먼저 설계하는 것이 안전하다.
5. RepMove=true는 순간 위치/속도 오차가 있고, RepMove=false는 장기 위치/회전 발산이 매우 크다.
```

---

## 9. 다음 코드 작업 제안

다음 코드 작업은 `CFVNetStateBase`가 적절하다.

목표:

```text
- FCFVehicleNetState 추가
- 서버에서 차량 권위 NetState 생성/복제
- OnRep에서 수신 샘플을 버퍼에 저장
- 각 버퍼 항목에 ReceivedLocalTimeSeconds 저장
- 아직 Transform 적용/보간은 하지 않음
```

중요 설계 반영:

```text
- 버퍼 시간축은 우선 ReceivedLocalTimeSeconds를 기준으로 한다.
- ServerTimeSeconds는 함께 저장하되, 보간 단독 기준으로 확정하지 않는다.
- 다음 단계 CFVRemoteInterp에서 원격 차량 보간을 적용한다.
```

---

## 10. 변경 기록

### v0.1.0

```text
- CFVNetAgeSplit 적용 후 로그 결과 정리
- ReceivedAge 안정성 확인
- ServerTimeDelta 잔여 불안정성 기록
- RepMove=true / false 오차 패턴 갱신
- 다음 코드 작업을 CFVNetStateBase로 제안
```
