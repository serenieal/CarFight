# 서버 구조 설계

- 문서 버전: v0.1.0
- 작성일: 2026-06-01
- 대상 프로젝트: CarFight
- 상위 문서: `README.md`
- 상태: Active

---

## 1. 목적

이 문서는 CarFight의 현재 차량 클라이언트 기능을 Dedicated Server에서 검증 가능하게 만들기 위한 최소 서버 구조를 설계한다.

현재 설계의 핵심은 다음이다.

```text
서버가 차량을 Spawn하고, PlayerController가 그 차량을 Possess한다.
```

---

## 2. 설계 원칙

```text
- 서버가 권한을 가진다.
- 클라이언트는 자기 입력과 자기 화면 표현만 담당한다.
- 맵에 배치된 차량에 의존하지 않는다.
- AutoPossessPlayer에 의존하지 않는다.
- 접속한 PlayerController마다 서버가 차량을 Spawn한다.
- Possess는 서버에서만 수행한다.
- UI, 카메라, 로컬 사운드는 Dedicated Server에서 실행하지 않는다.
```

---

## 3. 최소 클래스 구성

### 3.1 1차 필수 클래스

| 파일 | 클래스 | 역할 |
|---|---|---|
| `CFMPGameMode.h` | `ACFMPGameMode` | 서버 Spawn/Possess 담당 |
| `CFMPGameMode.cpp` | `ACFMPGameMode` | PostLogin 기반 차량 생성/빙의 구현 |

파일명은 32자를 넘지 않는다.

### 3.2 2차 후보 클래스

| 파일 | 클래스 | 역할 | 도입 조건 |
|---|---|---|---|
| `CFPlayerController.h` | `ACFPlayerController` | 입력/소유권/클라이언트 초기화 분리 | GameMode만으로 부족할 때 |
| `CFPlayerState.h` | `ACFPlayerState` | 플레이어별 상태 복제 | 스코어/팀/리스폰 단계 이후 |
| `CFGameState.h` | `ACFGameState` | 매치 상태 복제 | 매치 구조 도입 이후 |

이번 1차 단계에서는 `ACFMPGameMode`만 필수로 본다.

---

## 4. ACFMPGameMode 책임

`ACFMPGameMode`는 아래 책임만 가진다.

```text
1. 플레이어 접속 감지
2. PlayerStart 선택
3. 차량 Pawn 클래스 결정
4. 차량 Pawn Spawn
5. PlayerController Possess
6. 실패 시 명확한 로그 출력
```

하지 않는 것:

```text
- 무기 지급
- 체력 초기화
- 스코어 초기화
- 팀 배정
- 리스폰 규칙
- 매치 시작/종료
- 로비 처리
```

---

## 5. 주요 함수 설계

### 5.1 `PostLogin`

역할:

```text
서버에 PlayerController가 접속했을 때 차량 Spawn/Possess를 시작한다.
```

처리 순서:

```text
1. Super::PostLogin 호출
2. NewPlayer 유효성 확인
3. 서버 권한 확인
4. SpawnVehicleForController 호출
5. 성공/실패 로그 출력
```

### 5.2 `SpawnVehicleForController`

역할:

```text
PlayerController 하나에 대해 차량 Pawn을 생성하고 Possess한다.
```

처리 순서:

```text
1. Controller 유효성 확인
2. 이미 Pawn을 가지고 있으면 기존 Pawn 사용 여부 판단
3. 차량 Pawn 클래스 결정
4. Spawn Transform 결정
5. 서버에서 SpawnActor 실행
6. Spawn된 Pawn의 Replicate 설정 확인
7. Controller->Possess 실행
8. Possess 결과 로그 출력
```

### 5.3 `FindVehicleSpawnTransform`

역할:

```text
PlayerStart 또는 fallback 위치를 기준으로 차량 Spawn Transform을 찾는다.
```

처리 순서:

```text
1. ChoosePlayerStart 사용 가능 여부 확인
2. PlayerStart Actor 위치 사용
3. PlayerStart가 없으면 GameMode Actor 위치 또는 원점 fallback 사용
4. 차량 간 겹침 방지를 위해 접속 순번 기반 offset 적용 후보
```

### 5.4 `ResolveVehiclePawnClass`

역할:

```text
서버가 Spawn할 차량 Pawn 클래스를 결정한다.
```

1차 기준:

```text
UPROPERTY(EditDefaultsOnly) TSubclassOf<APawn> VehiclePawnClass
```

권장 기본값:

```text
/Game/CarFight/Vehicles/BP_CFVehiclePawn
```

C++ 생성자에서 BP 경로를 강제 로드할지 여부는 구현 단계에서 판단한다.

---

## 6. Config 연결 설계

`DefaultEngine.ini`에는 서버용 GameMode 기준을 연결한다.

후보:

```ini
[/Script/EngineSettings.GameMapsSettings]
GlobalDefaultGameMode=/Script/CarFight_Re.CFMPGameMode
GlobalDefaultServerGameMode=/Script/CarFight_Re.CFMPGameMode
```

주의:

```text
Config로 연결하기 전에 C++ 클래스 빌드가 먼저 성공해야 한다.
```

---

## 7. TestMap 설계 기준

현재 TestMap은 2클라 검증에 부족할 수 있다.

권장 기준:

```text
- PlayerStart 최소 2개
- PlayerStart 간 충분한 거리 확보
- 맵에 배치된 싱글용 BP_CFVehiclePawn 제거 또는 비활성화
- 차량은 GameMode가 서버에서 Spawn
- 바닥과 충돌 테스트 가능 영역 유지
```

---

## 8. ACFVehiclePawn 쪽 요구사항

`ACFVehiclePawn`은 서버 GameMode가 Spawn/Possess할 수 있어야 한다.

필요 조건:

```text
- bReplicates = true
- bReplicateMovement = true
- AutoPossessPlayer = Disabled 권장
- 입력 바인딩은 Dedicated Server에서 실행되지 않아야 함
- 입력 바인딩은 IsLocallyControlled 기준으로 실행되어야 함
- 카메라/UI는 소유 클라이언트 전용이어야 함
```

---

## 9. Dedicated Server 안전 guard 기준

Dedicated Server에서 실행되면 안 되는 코드는 아래 guard 중 하나를 가져야 한다.

```cpp
GetNetMode() != NM_DedicatedServer
```

또는

```cpp
IsLocallyControlled()
```

또는 둘 다 필요하다.

적용 대상:

```text
- Reticle 생성
- Debug HUD 생성
- Debug Panel 생성
- Camera Tick
- LocalPlayer 접근
- Enhanced Input Mapping Context 등록
- 사운드 재생
- 로컬 이펙트 생성
```

---

## 10. 1차 성공 시퀀스

목표 시퀀스:

```text
1. Dedicated Server 실행
2. Client A 접속
3. Server PostLogin(Client A)
4. Server Spawn BP_CFVehiclePawn_A
5. Server Possess Client A -> BP_CFVehiclePawn_A
6. Client A 화면에서 차량 카메라 표시
7. Client B 접속
8. Server PostLogin(Client B)
9. Server Spawn BP_CFVehiclePawn_B
10. Server Possess Client B -> BP_CFVehiclePawn_B
11. Client A/B가 각자 자기 차량 조작
12. 서로의 차량 이동 관찰
```

---

## 11. 변경 기록

### v0.1.0

```text
- 최소 서버 구조를 ACFMPGameMode 중심으로 설계
- GameMode 책임 범위 정의
- 주요 함수 후보와 처리 순서 정의
- Config 연결 후보 정의
- TestMap 정리 기준 정의
- Dedicated Server guard 기준 정의
```
