# 에디터 작업지시서: TestMap 2클라 서버 테스트 정리

- 문서 버전: v0.1.0
- 작성일: 2026-06-01
- 대상 프로젝트: CarFight
- 작업 유형: UE 에디터 수동 작업
- 대상 맵: `/Game/Maps/TestMap.TestMap`
- 상태: Ready

---

## 1. 목적

`ACFMPGameMode`가 서버에서 차량을 Spawn/Possess하는 구조로 바뀌었기 때문에, `TestMap`을 멀티플레이 테스트에 맞게 정리한다.

현재 확인된 상태:

```text
- PlayerStart_0 1개
- BP_CFVehiclePawn_C_1 1대 배치됨
```

목표 상태:

```text
- PlayerStart 최소 2개
- 맵에 배치된 BP_CFVehiclePawn 제거 또는 서버 테스트에서 사용하지 않도록 분리
- 차량은 ACFMPGameMode가 서버에서 Spawn
```

---

## 2. 작업 전 주의

이 작업은 코드 작업이 아니다.

Codex 작업 대상이 아니라 UE 에디터에서 직접 수행하는 작업이다.

작업 전 아래 상태를 확인한다.

```text
- UE 에디터가 열려 있다면 저장 전 변경 내용을 확인한다.
- TestMap을 수정하기 전에 현재 맵을 열어 상태를 확인한다.
- 작업 후 반드시 저장한다.
```

---

## 3. 작업 절차

### 3.1 TestMap 열기

UE 에디터에서 아래 맵을 연다.

```text
/Content/Maps/TestMap
```

메뉴 경로:

```text
콘텐츠 브라우저(Content Browser) > Content > Maps > TestMap 더블클릭
```

---

### 3.2 PlayerStart 추가

현재 `PlayerStart_0` 1개만 있으므로, 2클라 테스트를 위해 PlayerStart를 하나 더 추가한다.

방법:

```text
배치 액터(Place Actors) > 기본(Basic) > Player Start
```

또는 검색창에서 `Player Start`를 검색해서 뷰포트에 배치한다.

권장 위치:

```text
PlayerStart_0: 기존 위치 유지
PlayerStart_1: 기존 PlayerStart에서 X 또는 Y 방향으로 800~1200 Unreal Unit 이상 떨어진 위치
```

주의:

```text
- 두 PlayerStart가 같은 위치에 겹치면 안 된다.
- 바닥 위에 있어야 한다.
- 차량이 Spawn될 때 바닥이나 벽에 너무 깊게 겹치지 않게 한다.
```

---

### 3.3 배치 차량 처리

현재 맵에는 `BP_CFVehiclePawn_C_1`이 1대 배치되어 있다.

권장 처리:

```text
BP_CFVehiclePawn_C_1 삭제
```

이유:

```text
- 현재 차량은 GameMode가 서버에서 Spawn한다.
- 맵 배치 차량은 서버 Spawn 차량과 혼동될 수 있다.
- BP_CFVehiclePawn의 AI Auto Possess가 PlacedInWorld라서 배치 차량에 AIController가 붙을 수 있다.
```

삭제 방법:

```text
월드 아웃라이너(World Outliner)에서 BP_CFVehiclePawn_C_1 선택 > Delete
```

대안:

```text
싱글 테스트용으로 남겨야 한다면 TestMap을 복사해 서버 테스트 전용 맵을 따로 만든다.
예: TestMap_MP
```

현재 권장 방향은 기존 TestMap에서 배치 차량을 제거하는 것이다.

---

### 3.4 맵 저장

작업 후 맵을 저장한다.

메뉴 경로:

```text
파일(File) > 모두 저장(Save All)
```

또는 콘텐츠 브라우저(Content Browser)에서 TestMap 저장 상태를 확인한다.

---

## 4. 완료 기준

작업 완료 후 상태는 다음이어야 한다.

```text
- TestMap에 PlayerStart가 최소 2개 있다.
- TestMap에 BP_CFVehiclePawn_C_1 같은 배치 차량이 없다.
- 바닥과 기본 테스트 환경은 유지되어 있다.
- GameMode는 ACFMPGameMode를 사용한다.
- 차량은 맵 배치가 아니라 서버 Spawn으로 생성된다.
```

---

## 5. 작업 후 검증

에디터 작업 후 MCP 또는 UE 덤프로 다시 확인한다.

검증 항목:

```text
- Actor 목록에 PlayerStart가 2개 이상 있는지
- Actor 목록에 BP_CFVehiclePawn_C_*가 남아 있지 않은지
- Actor Count가 예상 범위인지
```

---

## 6. 다음 단계

이 작업이 끝나면 다음 테스트를 진행한다.

```text
1. 1클라 Dedicated 접속 테스트
2. 2클라 Dedicated 접속 테스트
3. 입력 분리 확인
4. 이동 복제 확인
```

---

## 7. 변경 기록

### v0.1.0

```text
- TestMap 2클라 서버 테스트용 에디터 작업지시서 최초 작성
- PlayerStart 추가, 배치 차량 제거, 저장/검증 절차 작성
```
