# 서버 고도화 결정 로그

- 문서 버전: v0.2.0
- 작성일: 2026-06-01
- 대상 프로젝트: CarFight
- 상위 문서: `README.md`
- 상태: Active

---

## 1. 목적

이 문서는 서버 고도화 작업 중 확정한 결정을 기록한다.

결정 로그의 목적은 같은 논의를 반복하지 않고, 왜 특정 구조를 선택했는지 남기는 것이다.

---

## 2. 결정 상태 표기

```text
[Proposed] 제안됨
[Accepted] 확정됨
[Deferred] 보류됨
[Rejected] 기각됨
[Replaced] 다른 결정으로 대체됨
```

---

## 3. 결정 목록

## D-001. 서버 고도화 작업은 새 폴더로 분리한다

- 상태: [Accepted]
- 결정일: 2026-06-01

### 결정

```text
Document/ProjectSSOT/Plan/ServerUpgradePlan/ 폴더를 새로 만들고,
서버 고도화 실행 계획은 이 폴더에서 관리한다.
```

### 이유

```text
기존 MP_ServerPlan은 Dedicated Server 최소 실행선과 초기 조사 기록의 성격이 강하다.
이번 작업은 그 다음 단계인 실제 서버 구조 고도화이므로 별도 폴더로 분리한다.
```

### 영향

```text
- MP_ServerPlan은 보존한다.
- ServerUpgradePlan은 현재 서버 고도화의 Active 계획 폴더가 된다.
```

---

## D-002. 현재 1차 목표는 신규 gameplay가 아니라 서버 안정화다

- 상태: [Accepted]
- 결정일: 2026-06-01

### 결정

```text
이번 단계에서는 무기, 체력, 대미지, 스코어, 리스폰을 새로 만들지 않는다.
현재 구현된 차량 클라이언트 기능을 Dedicated Server에서 안정화하는 데 집중한다.
```

### 이유

```text
서버가 아직 2클라 접속, Spawn, Possess, 입력 분리, 이동 복제를 검증하지 못했다.
이 상태에서 gameplay 시스템을 추가하면 원인 분리가 어려워진다.
```

### 영향

```text
- 무기/체력/대미지는 Phase 8 이후 확장 단계에서 다시 계획한다.
- 현재 작업은 차량 Spawn/Possess/입력/복제/서버 안전성 중심으로 제한한다.
```

---

## D-003. 첫 코드 작업 후보는 ACFMPGameMode다

- 상태: [Accepted]
- 결정일: 2026-06-01

### 결정

```text
첫 서버 코드 작업은 ACFMPGameMode 기반 최소 Spawn/Possess 구조로 잡는다.
```

### 이유

```text
현재 관찰된 핵심 증상은 Play As Client 빙의 실패와 Dedicated Server 접속 후 검정 화면이다.
이는 차량 기능 자체보다 서버가 PlayerController마다 차량을 Spawn/Possess하지 않는 구조 문제로 본다.
```

### 영향

```text
- CFMPGameMode.h / CFMPGameMode.cpp 생성 후보가 된다.
- GameMode는 PlayerController 접속 시 차량 생성과 Possess만 담당한다.
- 로비, 리스폰, 팀, 스코어는 포함하지 않는다.
```

---

## D-004. 맵 배치 차량 의존을 줄인다

- 상태: [Accepted]
- 결정일: 2026-06-01
- 확정일: 2026-06-01

### 결정

```text
멀티플레이 테스트에서는 맵에 배치된 차량에 의존하지 않고,
서버 GameMode가 PlayerStart 기준으로 차량을 Spawn한다.
```

### 이유

```text
맵 배치 차량과 AutoPossessPlayer는 싱글/Listen 테스트에서는 편하지만,
Dedicated Server 2클라 테스트에서는 소유권 충돌과 검정 화면 원인이 될 수 있다.
```

### 결과

```text
TestMap에는 PlayerStart 2개를 배치했고, 기존 배치 차량은 제거했다.
ACFMPGameMode가 접속자별 차량을 Spawn/Possess하는 구조로 2클라 테스트를 통과했다.
```

---

## D-005. PlayerController 전용 클래스는 1차 필수에서 제외한다

- 상태: [Accepted]
- 결정일: 2026-06-01

### 결정

```text
1차 안정화에서는 GameMode만 필수로 보고,
전용 PlayerController는 GameMode만으로 부족할 때 도입한다.
```

### 이유

```text
현재 가장 큰 병목은 PlayerController 클래스 부재 자체가 아니라 Spawn/Possess 흐름 부재다.
불필요한 클래스를 먼저 늘리면 테스트 범위가 커진다.
```

### 영향

```text
- 1차 코드 작업은 CFMPGameMode에 집중한다.
- 입력 매핑, 클라이언트 초기화, HUD 관리가 복잡해지면 CFPlayerController 도입을 재검토한다.
```

---

## D-006. Dedicated Server 금지 코드는 별도 Phase로 정리한다

- 상태: [Accepted]
- 결정일: 2026-06-01

### 결정

```text
UI, 카메라, LocalPlayer, 로컬 사운드, 로컬 이펙트 guard 정리는 Phase 7로 분리한다.
```

### 이유

```text
Spawn/Possess가 되기 전에는 어떤 로컬 전용 코드가 실제 서버에서 문제를 일으키는지 충분히 관찰하기 어렵다.
먼저 1클라/2클라 접속을 통과한 뒤 로그 기준으로 정리하는 것이 안전하다.
```

### 영향

```text
- ACFVehiclePawn의 UI guard는 유지한다.
- UCFVehicleCameraComp 등은 접속 테스트 이후 우선순위를 정한다.
```

---

## 4. 후속 결정 후보

아래 항목은 아직 확정하지 않았다.

```text
- CFPlayerController를 언제 도입할지
- 무기/발사 요청을 어떤 RPC/서버 권한 구조로 확장할지
- 체력/대미지/리스폰을 어떤 순서로 도입할지
- 장기적으로 차량 이동 품질 문제가 발생할 경우 Chaos Vehicle 기본 복제를 유지할지, 별도 서버 입력/RPC 구조를 도입할지
```

---

## 5. 변경 기록

### v0.1.0

```text
- 서버 고도화 결정 로그 최초 작성
- D-001부터 D-006까지 초기 결정 기록
- 후속 결정 후보 목록 작성
```
