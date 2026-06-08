# CarFight — 05_TestChecklist

> 문서 버전: v1.0.0  
> 작성일(Asia/Seoul): 2026-06-02  
> 문서 상태: Active  
> 역할: CarFight의 **완료된 Systems 기준 최소 회귀 테스트**를 관리한다.

---

## 1. 목적

이 문서는 CarFight에서 이미 구현되어 `Document/Systems/`에 기록된 기능들의 최소 회귀 테스트를 관리한다.

이 문서는 진행 중 기능의 상세 테스트 계획이 아니다.
진행 중 기능의 테스트 계획은 `Document/Plan/<기능명>/TestPlan.md`에서 관리한다.

이 문서의 목적은 아래와 같다.

```text
- 완료된 기능이 이후 작업으로 깨졌는지 빠르게 확인한다.
- 기능별 테스트 기준 문서 위치를 한 곳에서 찾는다.
- 새로운 기능 완료 후 회귀 테스트 목록에 추가한다.
- AI가 수정 작업 전후로 확인할 최소 검증 범위를 알 수 있게 한다.
```

---

## 2. 문서 사용 기준

문서 역할은 아래처럼 분리한다.

| 문서 위치 | 역할 |
|---|---|
| `Document/Plan/<기능명>/TestPlan.md` | 진행 중 기능의 상세 테스트 계획 |
| `Document/ProjectSSOT/05_TestChecklist.md` | 현재 기준선/P0 검증 기록 |
| `Document/ProjectSSOT/05_TestChecklist.md` | 완료된 Systems 기준 최소 회귀 테스트 인덱스 |
| `Document/Systems/<분류>/<기능명>.md` | 완료 기능의 현재 구현 기준 |

---

## 3. 테스트 상태 표기

| 상태 | 의미 |
|---|---|
| `PASS` | 통과 |
| `FAIL` | 실패 |
| `PARTIAL` | 일부 통과 / 일부 미확인 |
| `N/A` | 현재 조건에서 해당 없음 |
| `TODO` | 아직 테스트 항목만 있고 수행 전 |

---

## 4. 공통 테스트 원칙

```text
1. 현재 구현 기준은 Document/Systems/를 우선한다.
2. 미래 계획은 테스트 PASS 기준으로 쓰지 않는다.
3. 실패 시 원인을 코어 문제 / 수치 문제 / 에디터 설정 문제 / 테스트 환경 문제로 분리한다.
4. 테스트 결과가 기능 책임을 바꾸면 해당 Systems 문서도 갱신한다.
5. 반복 테스트가 3회 이상 수동 절차를 요구하면 02_FeatureQueue.md의 관리툴 후보 큐에 등록한다.
```

---

## 5. 전체 최소 회귀 테스트 세트

아래 테스트는 큰 구조 수정 후 최소 1회 확인한다.

| ID | 영역 | 테스트 | PASS 기준 | 관련 Systems 문서 | 상태 |
|---|---|---|---|---|---|
| `CF-TC-001` | Vehicle | 기본 차량 조작 | 전진/후진/조향/브레이크/핸드브레이크가 동작 | `Document/Systems/Vehicles/VehicleDrive.md` | `TODO` |
| `CF-TC-002` | Vehicle | 휠 시각 동기화 | 휠 위치/조향/스핀 시각 반응이 정상 | `Document/Systems/Vehicles/WheelSync.md` | `TODO` |
| `CF-TC-003` | Vehicle | 카메라 | 차량 기준 카메라가 정상 추적/회전 | `Document/Systems/Vehicles/VehicleCamera.md` | `TODO` |
| `CF-TC-004` | Input | 기본 입력 등록 | Enhanced Input Mapping Context 등록 성공 | `Document/Systems/Input/Input.md` | `TODO` |
| `CF-TC-005` | UI | 차량 디버그 표시 | 디버그 HUD/Panel이 필요한 조건에서 표시 | `Document/Systems/UI/VehicleDebug.md`, `Document/Systems/UI/VehicleDebugPanel.md` | `TODO` |
| `CF-TC-006` | UI | 조준 Reticle | 조준 Reticle 표시/갱신이 정상 | `Document/Systems/UI/AimReticle.md` | `TODO` |
| `CF-TC-007` | Network | Dedicated Server 실행 | 서버 타깃 실행 및 맵 로드 성공 | `Document/Systems/Network/ServerSpawn.md` | `TODO` |
| `CF-TC-008` | Network | 1클라 Spawn/Possess | 클라이언트 1명이 자기 차량을 점유 | `Document/Systems/Network/ServerSpawn.md` | `TODO` |
| `CF-TC-009` | Network | 2클라 Spawn/Possess | 클라이언트 2명이 각자 다른 차량을 점유 | `Document/Systems/Network/ServerSpawn.md` | `TODO` |
| `CF-TC-010` | Network | 입력 분리 | 각 클라이언트 입력이 자기 차량에만 적용 | `Document/Systems/Network/ServerSpawn.md`, `Document/Systems/Input/Input.md` | `TODO` |
| `CF-TC-011` | Network | 이동 복제 | 상대 차량 위치/회전이 양쪽 클라에서 보임 | `Document/Systems/Network/ServerSpawn.md` | `TODO` |
| `CF-TC-012` | Config | 런타임 설정 | 현재 Config 기준 경로/모드가 깨지지 않음 | `Document/Systems/Config/ProjectRuntimeConfig.md` | `TODO` |

---

## 6. Vehicles 테스트

## 6.1 VehicleDrive

- 관련 문서: `Document/Systems/Vehicles/VehicleDrive.md`

### 확인 항목

```text
- Throttle 입력이 차량 전진에 적용된다.
- Brake 입력이 감속/정지에 적용된다.
- Steering 입력이 좌우 조향에 적용된다.
- Handbrake 입력이 핸드브레이크 동작에 적용된다.
- DriveState가 Idle / Accelerating / Braking / Reversing / Coasting / Airborne / Disabled 범위에서 의미 있게 변한다.
- VehicleMovement를 찾지 못하는 경우 Disabled로 안전하게 떨어지는지 확인한다.
```

### PASS 기준

```text
- 입력 적용이 끊기지 않는다.
- DriveState가 명백히 틀린 상태로 고정되지 않는다.
- 입력/상태 디버그가 확인 가능하다.
```

---

## 6.2 WheelSync

- 관련 문서: `Document/Systems/Vehicles/WheelSync.md`

### 확인 항목

```text
- Wheel_Anchor_* 위치가 기준 차량 휠 중심과 맞는다.
- 조향 시 앞바퀴 피벗이 자연스럽다.
- 전진/후진 시 휠 스핀 방향이 정상으로 보인다.
- 심한 떨림 또는 이중 적용이 없다.
- 고속 휠 시각 품질 이슈가 기능 FAIL인지, 품질 후속인지 분리된다.
```

### PASS 기준

```text
- 기본 주행 중 휠 위치/조향/스핀 시각 동기화가 기능적으로 정상이다.
- 품질 이슈가 있어도 기능 판정과 분리해서 기록 가능하다.
```

---

## 6.3 VehicleCamera

- 관련 문서: `Document/Systems/Vehicles/VehicleCamera.md`

### 확인 항목

```text
- 로컬 플레이어 차량 기준 카메라가 정상 생성/활성화된다.
- Look 입력이 카메라에 반영된다.
- Dedicated Server에서 카메라 전용 Tick/LocalPlayer 접근 오류가 없다.
- 비소유 Pawn에서 로컬 카메라/입력 처리가 실행되지 않는다.
```

### PASS 기준

```text
- 로컬 클라이언트 카메라가 정상 동작한다.
- Dedicated Server 로그에 카메라/LocalPlayer 관련 치명 오류가 없다.
```

---

## 6.4 VehicleAim

- 관련 문서: `Document/Systems/Vehicles/VehicleAim.md`

### 확인 항목

```text
- 현재 AimComp가 의도한 기준으로 조준 방향/타겟 정보를 계산한다.
- UI Reticle과 연결되는 데이터가 유효하다.
- 서버 권한 발사 구조 도입 전까지는 Aim과 Fire 책임을 혼동하지 않는다.
```

### PASS 기준

```text
- 조준 데이터가 UI 또는 후속 Fire 요청에서 읽을 수 있는 형태로 유지된다.
- Aim 기능이 서버 판정 책임까지 임의로 떠안지 않는다.
```

---

## 7. Input 테스트

- 관련 문서: `Document/Systems/Input/Input.md`

### 확인 항목

```text
- DefaultInputMappingContext가 로컬 플레이어에 등록된다.
- Throttle / Brake / Steering / Handbrake 입력이 바인딩된다.
- VehicleMove 2D 입력과 Legacy Axis 입력이 충돌하지 않는다.
- InputDeviceMode가 Auto / KeyboardMouseOnly / GamepadOnly 조건에서 의도대로 필터링된다.
- Look 입력이 VehicleCameraComp로 전달된다.
```

### PASS 기준

```text
- 현재 테스트 장치에서 차량 조작 입력이 정상 동작한다.
- 장치 모드 제한 때문에 입력이 막힌 경우 구조 실패와 설정 문제를 분리해 기록한다.
```

---

## 8. UI 테스트

## 8.1 AimReticle

- 관련 문서: `Document/Systems/UI/AimReticle.md`

### 확인 항목

```text
- Reticle 위젯이 필요한 조건에서 생성된다.
- 조준 상태 변화가 Reticle에 반영된다.
- 서버 또는 비소유 Pawn에서 로컬 UI 생성이 실행되지 않는다.
```

### PASS 기준

```text
- 로컬 클라이언트 화면에서 Reticle이 정상 표시된다.
- Dedicated Server 로그에 UI 생성 오류가 없다.
```

---

## 8.2 VehicleDebug / VehicleDebugPanel

- 관련 문서:
  - `Document/Systems/UI/VehicleDebug.md`
  - `Document/Systems/UI/VehicleDebugPanel.md`

### 확인 항목

```text
- 디버그 UI 생성 조건이 명확하다.
- 표시 텍스트가 현재 Systems 기준 필드와 맞는다.
- Dedicated Server에서 Debug UI 생성이 차단된다.
- 너무 긴 Runtime 문자열이 가독성을 해치지 않는지 확인한다.
```

### PASS 기준

```text
- 디버그 UI가 테스트 중 필요한 정보를 제공한다.
- 서버에서 로컬 UI 관련 오류가 없다.
```

---

## 9. Network 테스트

## 9.1 ServerSpawn

- 관련 문서: `Document/Systems/Network/ServerSpawn.md`

### 확인 항목

```text
- CarFight_ReServer Target 빌드/실행이 가능하다.
- 서버가 TestMap 또는 서버 테스트맵을 로드한다.
- Client 1이 접속한다.
- 서버에서 PlayerController 1개가 확인된다.
- ACFMPGameMode::PostLogin() 흐름이 호출된다.
- 차량 Pawn이 Spawn된다.
- PlayerController가 Spawn된 차량을 Possess한다.
- Client 1 화면이 검정 화면에 고정되지 않는다.
```

### PASS 기준

```text
- 1클라 접속 후 자기 차량을 조작할 수 있다.
- 서버 로그에 Spawn/Possess 성공 로그가 확인된다.
```

---

## 9.2 2클라 소유권 / 입력 분리

- 관련 문서:
  - `Document/Systems/Network/ServerSpawn.md`
  - `Document/Systems/Input/Input.md`

### 확인 항목

```text
- Client 1과 Client 2가 모두 접속한다.
- 차량 Pawn이 2대 생성된다.
- Client 1과 Client 2가 서로 다른 차량을 소유한다.
- Client 1 입력은 Client 1 차량에만 적용된다.
- Client 2 입력은 Client 2 차량에만 적용된다.
- 비소유 차량이 로컬 입력에 반응하지 않는다.
```

### PASS 기준

```text
- 각 클라이언트가 자기 차량만 조작한다.
- 입력 교차 오염이 없다.
```

---

## 9.3 이동 복제

- 관련 문서: `Document/Systems/Network/ServerSpawn.md`

### 확인 항목

```text
- Client 1 차량 이동이 Client 2 화면에서 보인다.
- Client 2 차량 이동이 Client 1 화면에서 보인다.
- 위치 복제가 확인된다.
- 회전 복제가 확인된다.
- 정지 상태도 동기화된다.
- 급가속/급조향 시 품질 문제를 기능 FAIL과 분리한다.
```

### PASS 기준

```text
- 상대 차량 위치/회전이 갱신된다.
- 상대 차량이 전혀 안 보이거나 완전히 다른 위치로 분리되지 않는다.
```

---

## 10. Config 테스트

- 관련 문서: `Document/Systems/Config/ProjectRuntimeConfig.md`

### 확인 항목

```text
- DefaultEngine.ini의 주요 GameMode/ServerGameMode 연결이 의도와 맞는다.
- 입력/맵/런타임 설정 경로가 현재 Systems 문서와 충돌하지 않는다.
- 오래된 ProjectSSOT/Plan 경로가 남아 있으면 실제 구조에 맞게 수정 후보로 기록한다.
```

### PASS 기준

```text
- 현재 실행 경로와 문서 경로가 서로 충돌하지 않는다.
- 런타임 설정 때문에 기본 테스트가 막히지 않는다.
```

---

## 11. 테스트 기록 양식

각 테스트 후 아래 형식으로 기록한다.

```text
테스트 일시:
테스트 환경:
엔진 버전:
실행 방식:
대상 맵:
대상 기능:
관련 Systems 문서:
결과: PASS / FAIL / PARTIAL / N/A
관찰 내용:
로그 요약:
수정 필요 항목:
다음 액션:
```

---

## 12. 실패 분류 기준

실패 시 아래 중 하나로 분류한다.

| 분류 | 의미 |
|---|---|
| `Core` | 코드 구조 또는 핵심 로직 문제 |
| `Data` | DataAsset / Config / 값 문제 |
| `Asset` | BP / 맵 / 메시 / 위젯 자산 문제 |
| `Server` | 권한 / 복제 / Dedicated Server 문제 |
| `Input` | 입력 매핑 / 장치 필터링 문제 |
| `UI` | 표시 / 위젯 생성 / 로컬 전용 처리 문제 |
| `Quality` | 기능은 되지만 품질 개선 필요 |
| `DocMismatch` | 문서와 실제 구현이 불일치 |

---

## 13. 문서 갱신 조건

아래 상황이 발생하면 이 문서를 갱신한다.

```text
- Document/Systems/에 새 완료 기능 문서가 추가됨
- 기존 Systems 문서의 기능 책임이 바뀜
- 회귀 테스트 항목이 늘거나 줄어듦
- 테스트 기준이 PASS/FAIL 판정에 영향을 줄 만큼 바뀜
- 반복 테스트 자동화 또는 관리툴 후보가 생김
```

---

## 14. 문서 버전 관리

- 현재 문서 버전: `v1.0.0`
- 문서 상태: `Active`

### 버전 증가 기준

| 버전 | 기준 |
|---|---|
| Major | 테스트 문서 운영 방식 자체 변경 |
| Minor | 새 시스템 테스트 섹션 추가 또는 PASS 기준 확장 |
| Patch | 표현 정리, 오탈자 수정, 링크 보강 |

---

## 15. 체인지로그

### v1.0.0 - 2026-06-02

```text
- TestChecklist 문서 최초 작성
- Systems 기준 최소 회귀 테스트 인덱스 작성
- Vehicles / Input / UI / Network / Config 테스트 섹션 추가
- 테스트 기록 양식과 실패 분류 기준 추가
```
