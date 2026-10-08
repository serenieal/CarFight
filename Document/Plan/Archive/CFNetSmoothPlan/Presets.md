# CFNetSmooth 설정 프리셋

- 문서 버전: v0.1.0
- 작성일: 2026-06-17
- 대상 프로젝트: CarFight
- 작업 유형: 신규
- 상태: Draft

---

## 1. 목적

이 문서는 `CFNetSmooth` 테스트 중 사용하는 설정값을 프리셋 단위로 분리한다.

목표는 다음이다.

```text
기본값과 스트레스 테스트값을 섞지 않는다.
테스트 후 어떤 값을 되돌려야 하는지 명확히 남긴다.
다른 세션이나 다른 AI가 같은 조건으로 검증을 반복할 수 있게 한다.
```

---

## 2. 적용 범위

이 문서는 아래 대상에만 적용한다.

```text
CF Net Smooth Test Actor
CF Net Smooth 컴포넌트
```

현재 단계에서는 기존 차량 Pawn, 차량 BP, RepMove, AutoPhysics 설정을 변경하지 않는다.

---

## 3. 공통 전제

### 3.1 Actor 기본 조건

테스트 Actor는 아래 조건을 유지한다.

| 항목 | 값 | 이유 |
|---|---:|---|
| Actor Replicates | true | 서버 State를 클라이언트에 전파하기 위함 |
| Replicate Movement | false | UE 기본 이동 복제와 CFNetSmooth 적용을 섞지 않기 위함 |
| Always Relevant | true | 테스트 Actor 수신 조건을 단순화하기 위함 |
| Apply Received State To Owner | true | 테스트 Actor에서만 수신 Transform 적용을 확인하기 위함 |

주의:

- 일반 Actor나 차량 Pawn에 `Apply Received State To Owner=true`를 바로 적용하지 않는다.
- 차량 연결은 별도 Visual/Shell 설계와 롤백 절차를 만든 뒤 진행한다.

### 3.2 에디터 위치

설정은 테스트 Actor 선택 후 아래 패널에서 확인한다.

```text
세부 정보(Details) 패널
CF Net Smooth 컴포넌트
CF Net Smooth|Send
CF Net Smooth|Smoothing
CF Net Smooth|Snap
CF Net Smooth|Debug
CF Net Smooth|Test Motion
CF Net Smooth|Test Teleport
```

---

## 4. 프리셋 P-001: 기본 안전값

### 4.1 목적

일반 이동/회전에서 과도한 Snap 없이 보간이 유지되는지 확인한다.

### 4.2 값

| 영역 | 항목 | 값 |
|---|---|---:|
| Send | Send Rate | 20.0 |
| Smoothing | Interp Back Time | 0.10 |
| Smoothing | Max Extrap Time | 0.20 |
| Snap | Pos Snap Dist | 500.0 |
| Snap | Rot Snap Deg | 90.0 |
| Debug | Enable Debug Log | false |
| Test Motion | Enable Server Motion | true |
| Test Motion | Motion Distance | 300.0 |
| Test Motion | Motion Speed | 120.0 |
| Test Motion | Rotate During Motion | false |
| Test Motion | Rotation Speed Deg | 30.0 |
| Test Teleport | Enable Teleport Test | false |
| Test Teleport | Teleport Interval | 5.0 |
| Test Teleport | Teleport Offset | X=0, Y=600, Z=0 |

### 4.3 예상 결과

- 일반 상태에서 `[CFNetSmooth][Snap]` 로그가 거의 나오지 않는다.
- 일반 상태에서 `[CFNetSmooth][RotSnap]` 로그가 거의 나오지 않는다.
- `Enable Debug Log=false`이면 CFNetSmooth 디버그 로그가 나오지 않는다.

---

## 5. 프리셋 P-002: 송수신 로그 확인

### 5.1 목적

서버가 State를 보내고 클라이언트가 받는지 확인한다.

### 5.2 값

| 영역 | 항목 | 값 |
|---|---|---:|
| Debug | Enable Debug Log | true |
| Send | Send Rate | 20.0 |
| Smoothing | Interp Back Time | 0.10 |
| Snap | Pos Snap Dist | 500.0 |
| Snap | Rot Snap Deg | 90.0 |
| Test Motion | Enable Server Motion | true |
| Test Motion | Motion Speed | 120.0 |

### 5.3 예상 결과

```text
[CFNetSmooth][Send]
[CFNetSmooth][Receive]
```

- `Seq`가 증가한다.
- `Time`이 증가한다.
- `Loc`이 이동 테스트에 맞게 변한다.
- `Buffer`가 64를 넘지 않는다.

### 5.4 복구값

테스트 후 `Enable Debug Log=false`로 되돌린다.

---

## 6. 프리셋 P-003: 위치 Snap 스트레스 테스트

### 6.1 목적

`Pos Snap Dist`가 낮을 때 위치 Snap이 발생하는지 확인한다.

### 6.2 값

| 영역 | 항목 | 값 |
|---|---|---:|
| Debug | Enable Debug Log | true |
| Snap | Pos Snap Dist | 1.0 |
| Snap | Rot Snap Deg | 90.0 |
| Test Motion | Motion Speed | 600.0 |
| Test Teleport | Enable Teleport Test | false |

### 6.3 예상 결과

```text
[CFNetSmooth][Snap]
```

- `Error`가 `Threshold=1.00`보다 클 때 로그가 찍힌다.
- 위치 Snap은 Buffer를 비우지 않고 표시 위치만 즉시 보정한다.

### 6.4 복구값

```text
Pos Snap Dist=500.0
Motion Speed=120.0
Enable Debug Log=false
```

---

## 7. 프리셋 P-004: 제한 외삽 테스트

### 7.1 목적

State가 부족할 때 `Max Extrap Time` 안에서만 위치 예측하는지 확인한다.

### 7.2 값

| 영역 | 항목 | 값 |
|---|---|---:|
| Debug | Enable Debug Log | true |
| Smoothing | Interp Back Time | 0.0 |
| Smoothing | Max Extrap Time | 0.20 |
| Snap | Pos Snap Dist | 0.0 |
| Test Motion | Motion Speed | 120.0 |
| Test Teleport | Enable Teleport Test | false |

### 7.3 예상 결과

```text
[CFNetSmooth][Extrapolate]
```

- `Time`이 `Max=0.200` 이하일 때만 외삽 로그가 찍힌다.
- `Max Extrap Time=0.0`으로 바꾸면 외삽 로그가 멈춘다.

### 7.4 복구값

```text
Interp Back Time=0.10
Max Extrap Time=0.20
Pos Snap Dist=500.0
Enable Debug Log=false
```

---

## 8. 프리셋 P-005: Teleport 테스트

### 8.1 목적

Teleport State 수신 시 Buffer Clear와 즉시 적용이 되는지 확인한다.

### 8.2 값

| 영역 | 항목 | 값 |
|---|---|---:|
| Debug | Enable Debug Log | true |
| Snap | Pos Snap Dist | 500.0 |
| Snap | Rot Snap Deg | 90.0 |
| Test Teleport | Enable Teleport Test | true |
| Test Teleport | Teleport Interval | 3.0 |
| Test Teleport | Teleport Offset | X=0, Y=600, Z=0 |

### 8.3 예상 결과

```text
Teleport=true
[CFNetSmooth][Teleport]
Buffer=1
```

- 순간이동 직전 State를 중간 경로로 보간하지 않는다.
- Teleport 이후 일반 보간으로 복귀한다.

### 8.4 복구값

```text
Enable Teleport Test=false
Teleport Interval=5.0
Enable Debug Log=false
```

---

## 9. 프리셋 P-006: 회전 보간 테스트

### 9.1 목적

회전값이 송수신되고 State 사이에서 부드럽게 보간되는지 확인한다.

### 9.2 값

| 영역 | 항목 | 값 |
|---|---|---:|
| Debug | Enable Debug Log | true |
| Smoothing | Interp Back Time | 0.10 |
| Snap | Pos Snap Dist | 500.0 |
| Snap | Rot Snap Deg | 90.0 |
| Test Motion | Rotate During Motion | true |
| Test Motion | Rotation Speed Deg | 90.0 |
| Test Teleport | Enable Teleport Test | false |

### 9.3 예상 결과

```text
[CFNetSmooth][Send] ... Rot=...
[CFNetSmooth][Receive] ... Rot=...
```

- `Rot` 값이 시간에 따라 변한다.
- `-170도`와 `190도`처럼 다르게 보이는 값은 360도 기준 같은 방향일 수 있다.
- 기본값에서는 `[CFNetSmooth][RotSnap]` 로그가 거의 나오지 않는다.

### 9.4 복구값

```text
Rotate During Motion=false
Rotation Speed Deg=30.0
Enable Debug Log=false
```

---

## 10. 프리셋 P-007: 회전 Snap 스트레스 테스트

### 10.1 목적

`Rot Snap Deg`가 낮을 때 회전 Snap이 발생하고, 0일 때 비활성화되는지 확인한다.

### 10.2 값

| 영역 | 항목 | 값 |
|---|---|---:|
| Debug | Enable Debug Log | true |
| Snap | Pos Snap Dist | 0.0 |
| Snap | Rot Snap Deg | 1.0 |
| Test Motion | Rotate During Motion | true |
| Test Motion | Rotation Speed Deg | 720.0 |
| Test Teleport | Enable Teleport Test | false |

### 10.3 예상 결과

```text
[CFNetSmooth][RotSnap]
```

- `Error`가 `Threshold=1.00`보다 클 때 로그가 찍힌다.
- `Rot Snap Deg=0.0`으로 바꾸면 `[CFNetSmooth][RotSnap]` 로그가 멈춘다.
- 기본값 `Rot Snap Deg=90.0`에서는 일반 이동/회전 중 로그가 거의 나오지 않는다.

### 10.4 2026-06-17 확인 결과

```text
Rot Snap Deg=0.0: [RotSnap] 로그 없음
기본값 일반 이동/회전: [RotSnap] 로그 없음
```

판정:

- 회전 Snap 비활성화 조건 정상
- 기본 임계값 과민 반응 없음

### 10.5 복구값

```text
Rot Snap Deg=90.0
Pos Snap Dist=500.0
Rotate During Motion=false
Rotation Speed Deg=30.0
Enable Debug Log=false
```

---

## 11. 프리셋 P-008: 악조건 네트워크 관찰

### 11.1 목적

지연이나 손실이 있는 환경에서 기본값이 너무 공격적인지 확인한다.

### 11.2 값

| 영역 | 항목 | 값 |
|---|---|---:|
| Debug | Enable Debug Log | true |
| Send | Send Rate | 20.0 |
| Smoothing | Interp Back Time | 0.15 |
| Smoothing | Max Extrap Time | 0.20 |
| Snap | Pos Snap Dist | 500.0 |
| Snap | Rot Snap Deg | 90.0 |
| Test Motion | Motion Speed | 120.0 |
| Test Motion | Rotate During Motion | true |
| Test Motion | Rotation Speed Deg | 90.0 |

### 11.3 판정 기준

- Snap 로그가 자주 나오면 `Interp Back Time`을 0.20까지 늘려 본다.
- 반응이 너무 늦으면 `Interp Back Time`을 0.10으로 되돌린다.
- 외삽 로그가 너무 자주 나오면 `Send Rate`, `Interp Back Time`, 네트워크 지연 조건을 같이 본다.
- `Max Extrap Time`을 크게 늘려서 오래 미끄러지게 만들지 않는다.

### 11.4 복구값

```text
Interp Back Time=0.10
Max Extrap Time=0.20
Enable Debug Log=false
Rotate During Motion=false
Rotation Speed Deg=30.0
```

---

## 12. 테스트 후 복구 체크리스트

테스트를 끝낼 때 아래 값을 확인한다.

```text
Enable Debug Log=false
Send Rate=20.0
Interp Back Time=0.10
Max Extrap Time=0.20
Pos Snap Dist=500.0
Rot Snap Deg=90.0
Enable Server Motion=true
Motion Distance=300.0
Motion Speed=120.0
Rotate During Motion=false
Rotation Speed Deg=30.0
Enable Teleport Test=false
Teleport Interval=5.0
Teleport Offset=(0,600,0)
```

---

## 13. Changelog

### v0.1.0

```text
- CFNetSmooth 테스트용 설정 프리셋 문서 최초 작성
- 기본 안전값, 송수신, 위치 Snap, 제한 외삽, Teleport, 회전 보간, 회전 Snap, 악조건 관찰 프리셋 추가
- RotSnapDeg 비활성화와 기본값 일반 이동/회전 확인 결과 기록
```

---

## 14. 마이그레이션 지침

- 이 문서는 테스트 Actor 검증용 프리셋이다.
- 차량 Pawn이나 차량 BP 기본값을 이 문서만 보고 변경하지 않는다.
- 스트레스 테스트값을 저장된 BP 기본값으로 남기지 않는다.
- 차량 적용 단계에서는 별도 차량용 프리셋과 롤백 절차를 새로 작성한다.
