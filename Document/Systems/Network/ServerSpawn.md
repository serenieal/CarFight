# ServerSpawn

## 문서 목적
이 문서는 현재 프로젝트에서 `Dedicated Server / Multiplayer Spawn` 기능이 실제로 어떤 일을 하는지, 그리고 그 기능이 어떤 클래스/설정 구성으로 동작하는지를 기록한다.
이 문서는 미래 설계나 개선 계획이 아니라, **현재 확인된 구현 상태**를 기준으로 작성한다.

## 문서 범위
이 문서에서 말하는 `ServerSpawn` 기능은 아래 요소를 묶어서 본다.

- 서버 빌드 타깃: `CarFight_ReServer.Target.cs`
- 서버 테스트용 GameMode: `ACFMPGameMode`
- 기본 차량 Pawn 클래스 경로: `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
- 로그인 처리 진입점: `ACFMPGameMode::PostLogin()`
- 자동 시작 흐름 차단 지점: `ACFMPGameMode::HandleStartingNewPlayer_Implementation()`
- 차량 스폰 함수: `ACFMPGameMode::SpawnVehicleForController()`
- 스폰 위치 해석 함수: `ACFMPGameMode::FindVehicleSpawnTransform()`
- Pawn 클래스 해석 함수: `ACFMPGameMode::ResolveVehiclePawnClass()`
- 로그아웃 처리: `ACFMPGameMode::Logout()`

즉, 현재 기준 `ServerSpawn`은 **Dedicated Server 전체 운영 시스템이 아니라, 서버 빌드 타깃과 접속한 플레이어에게 차량 Pawn을 생성/점유시키는 최소 멀티플레이 스폰 흐름**으로 본다.

## 이 기능이 현재 실제로 하는 일
현재 구현 기준 `ServerSpawn`의 핵심 역할은 **Dedicated Server 환경에서 플레이어가 로그인했을 때 서버가 차량 Pawn을 스폰하고 해당 PlayerController가 점유하도록 만드는 것**이다.

현재 이 기능은 아래 일을 한다.

### 1. Dedicated Server 빌드 타깃을 제공한다
현재 프로젝트에는 `CarFight_ReServer.Target.cs`가 존재한다.

현재 설정:
- `Type = TargetType.Server`
- `DefaultBuildSettings = BuildSettingsVersion.V6`
- `IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7`
- `bWithLiveCoding = false`
- `ExtraModuleNames.Add("CarFight_Re")`

현재 의미:
- 서버 전용 빌드 타깃이 분리되어 있다.
- Live Coding은 서버 런처 배포/실행 흐름에서 제외된다.
- 현재 서버 타깃은 `CarFight_Re` 모듈을 서버 빌드 대상으로 포함한다.

즉 현재 프로젝트는 **Dedicated Server 빌드를 위한 최소 타깃 정의를 가지고 있다.**

### 2. 기본 Pawn 자동 생성 흐름을 막는다
`ACFMPGameMode` 생성자에서는 `DefaultPawnClass = nullptr`로 설정한다.

현재 의미:
- Unreal 기본 GameMode의 자동 Pawn 생성 흐름에 맡기지 않는다.
- 차량 Pawn 생성은 `PostLogin()` 이후 직접 처리한다.
- `HandleStartingNewPlayer_Implementation()`도 로그만 남기고 기본 시작 처리를 진행하지 않는다.

즉 현재 서버 스폰 구조는 **GameMode의 기본 Pawn 자동 생성이 아니라, PostLogin 기반 수동 차량 스폰 흐름**이다.

### 3. 기본 차량 Pawn BP를 fallback으로 찾는다
`ACFMPGameMode` 생성자에서는 `ConstructorHelpers::FClassFinder<APawn>`으로 아래 BP 클래스를 찾는다.

```text
/Game/CarFight/Vehicles/BP_CFVehiclePawn
```

현재 동작:
- 찾으면 `VehiclePawnClass`에 저장한다.
- 실패하면 `VehiclePawnClass = nullptr`로 두고 경고 로그를 남긴다.

현재 의미:
- 별도 BP GameMode 설정 없이도 첫 서버 테스트에서 기본 차량 BP를 사용할 수 있다.
- 다만 BP 경로가 바뀌면 이 fallback은 실패한다.
- 현재 문서 기준으로 이 경로는 서버 테스트의 기본 차량 클래스 경로다.

### 4. 플레이어 로그인 직후 차량 스폰을 시도한다
`ACFMPGameMode::PostLogin()`은 플레이어가 접속한 직후 호출된다.

현재 처리 조건:
- `NewPlayer`가 유효해야 한다.
- GameMode가 Authority를 가져야 한다.
- `bSpawnVehicleOnPostLogin`이 true여야 한다.

조건을 만족하면:
- `SpawnVehicleForController(NewPlayer)` 호출

현재 의미:
- 서버는 플레이어 접속 후 곧바로 차량 Pawn을 생성하려고 한다.
- `bSpawnVehicleOnPostLogin`으로 이 자동 스폰 흐름을 끌 수 있다.
- Dedicated Server 테스트 기본값은 자동 스폰 활성화다.

### 5. 이미 Pawn을 가진 컨트롤러는 다시 스폰하지 않는다
`SpawnVehicleForController()`는 먼저 `PlayerController->GetPawn()`을 확인한다.

현재 동작:
- 기존 Pawn이 있으면 새 Pawn을 만들지 않는다.
- 기존 Pawn을 그대로 반환한다.
- 로그로 이미 Pawn을 보유하고 있음을 남긴다.

현재 의미:
- 중복 스폰을 방지한다.
- 재로그인/재호출 상황에서 같은 컨트롤러가 이미 Pawn을 가진 경우 안전하게 빠진다.

### 6. 사용할 차량 Pawn 클래스를 해석한다
`ResolveVehiclePawnClass()`는 현재 `VehiclePawnClass`가 유효한지 확인한다.

현재 동작:
- `VehiclePawnClass`가 있으면 반환한다.
- 없으면 경고 로그를 남기고 nullptr 반환한다.

현재 의미:
- 현재 구조에서 스폰 가능한 차량 Pawn 클래스가 없으면 스폰을 진행하지 않는다.
- 별도 차량 선택/로드아웃/계정 기반 Pawn 클래스 선택은 아직 없다.

### 7. PlayerStart를 우선 사용해 스폰 Transform을 찾는다
`FindVehicleSpawnTransform()`은 먼저 `ChoosePlayerStart(PlayerController)`를 호출한다.

현재 동작:
- `PlayerStart`를 찾으면 해당 Actor의 Transform을 사용한다.
- 로그로 어떤 PlayerStart를 사용했는지 남긴다.

현재 의미:
- 맵에 PlayerStart가 있으면 그 위치가 서버 차량 스폰 기준이 된다.
- 맵 구성에서 PlayerStart 배치가 중요하다.

### 8. PlayerStart가 없으면 결정적 fallback 위치를 사용한다
`PlayerStart`를 찾지 못하면 fallback 위치를 계산한다.

현재 fallback 규칙:
- `CurrentSpawnIndex = SpawnIndex`
- `SpawnIndex = SpawnIndex + 1`
- `SpawnOffsetX = CurrentSpawnIndex * SpawnOffsetBetweenPlayers`
- 위치: `(SpawnOffsetX, 0.0, 300.0)`
- 회전: `FRotator::ZeroRotator`

현재 기본값:
- `SpawnIndex = 0`
- `SpawnOffsetBetweenPlayers = 600.0f`

현재 의미:
- PlayerStart가 없어도 서버 접속 테스트는 진행할 수 있다.
- 각 플레이어는 X축 방향으로 600씩 떨어져 스폰된다.
- Z는 300으로 고정된다.
- 이 fallback은 테스트용 안전장치에 가깝다.

### 9. 서버에서 차량 Pawn을 스폰하고 점유시킨다
`SpawnVehicleForController()`는 조건이 맞으면 서버에서 Pawn을 생성한다.

현재 스폰 설정:
- `SpawnParameters.Owner = PlayerController`
- `SpawnParameters.Instigator = PlayerController->GetPawn()`
- `SpawnCollisionHandlingOverride = AdjustIfPossibleButAlwaysSpawn`

현재 스폰 후 처리:
- `World->SpawnActor<APawn>()`
- 성공하면 로그 기록
- `PlayerController->Possess(SpawnedVehiclePawn)` 호출
- 점유 성공 여부를 로그로 확인
- 최종 Pawn 반환

현재 의미:
- 서버가 Pawn을 생성하고 점유까지 수행한다.
- 스폰 충돌이 있어도 가능한 조정 후 항상 스폰하려고 한다.
- 현재는 차량 생성 성공/점유 성공 여부를 로그로 진단한다.

### 10. 로그아웃 시 로그를 남긴다
`ACFMPGameMode::Logout()`은 컨트롤러와 Pawn 이름을 로그로 남긴 뒤 `Super::Logout()`을 호출한다.

현재 의미:
- 로그아웃 시 별도 Pawn 제거 정책은 문서 기준 확인되지 않았다.
- 기본 정리는 Super 흐름에 맡긴다.

## 현재 기준 기능의 성격 정리
현재 구현을 종합하면 `ServerSpawn`은 아래 역할을 가진다.

1. **Dedicated Server 빌드 기준 제공**
   - 서버 타깃을 별도로 정의한다.
   - Live Coding을 서버 타깃에서 제외한다.

2. **서버 접속 후 차량 스폰 기능**
   - `PostLogin()`에서 플레이어 컨트롤러를 받는다.
   - 서버 권한과 자동 스폰 토글을 확인한다.
   - 차량 Pawn을 서버에서 생성한다.

3. **PlayerStart/fallback 스폰 위치 해석 기능**
   - PlayerStart가 있으면 우선 사용한다.
   - 없으면 SpawnIndex 기반 fallback 위치를 사용한다.

4. **Controller-Pawn 점유 연결 기능**
   - 서버에서 만든 차량 Pawn을 PlayerController가 점유한다.
   - 중복 Pawn 스폰을 방지한다.

따라서 현재 `ServerSpawn`은 정식 매치메이킹/세션/관리툴 서버가 아니라,
**Dedicated Server 접속 테스트에서 플레이어가 차량 Pawn을 받아 조작 가능한 상태로 들어가게 하는 최소 서버 스폰 기능**이라고 보는 것이 맞다.

## 현재 동작 방식
현재 `ServerSpawn`은 아래 방식으로 동작한다.

### 1. 서버 타깃 빌드
서버 빌드는 `CarFight_ReServer.Target.cs`를 기준으로 한다.

```text
CarFight_ReServerTarget
  -> TargetType.Server
  -> ExtraModuleNames: CarFight_Re
  -> bWithLiveCoding = false
```

### 2. GameMode 생성자 초기화
`ACFMPGameMode::ACFMPGameMode()`에서 수행하는 일:

1. `DefaultPawnClass = nullptr`
2. `/Game/CarFight/Vehicles/BP_CFVehiclePawn` 클래스 검색
3. 찾으면 `VehiclePawnClass`에 저장
4. `bSpawnVehicleOnPostLogin = true`
5. `SpawnIndex = 0`
6. `SpawnOffsetBetweenPlayers = 600.0f`

### 3. 플레이어 접속 흐름
현재 플레이어 접속 흐름은 아래와 같다.

```text
Player Login
  -> ACFMPGameMode::PostLogin
  -> Authority 확인
  -> bSpawnVehicleOnPostLogin 확인
  -> SpawnVehicleForController
  -> FindVehicleSpawnTransform
  -> SpawnActor<APawn>
  -> PlayerController.Possess
```

### 4. 기본 시작 흐름 차단
현재 `HandleStartingNewPlayer_Implementation()`은 로그만 남기고 기본 Pawn 시작 흐름을 진행하지 않는다.

현재 의미:
- 차량 스폰 흐름은 `PostLogin()`으로 단일화한다.
- 기본 GameMode 시작 처리와 중복되는 Pawn 생성을 피한다.

## 현재 표시 조건 / 실행 조건
현재 `ServerSpawn`이 제대로 동작하려면 아래 조건이 중요하다.

- 서버가 `ACFMPGameMode`를 사용해야 한다.
- GameMode가 Authority를 가져야 한다.
- `bSpawnVehicleOnPostLogin`이 true여야 자동 스폰된다.
- `VehiclePawnClass`가 유효해야 한다.
- 기본 fallback을 쓸 경우 `/Game/CarFight/Vehicles/BP_CFVehiclePawn` 경로가 유효해야 한다.
- `World`가 유효해야 한다.
- PlayerController가 유효해야 한다.
- Possess가 성공하려면 생성된 Pawn과 Controller가 서버 권한 흐름에서 정상이어야 한다.

조건 해석:
- Dedicated Server 빌드 타깃이 있어도 GameMode 설정이 다르면 이 스폰 흐름은 자동으로 쓰이지 않는다.
- 맵에 PlayerStart가 없더라도 fallback 위치로 스폰은 가능하다.
- 하지만 fallback은 테스트용에 가깝고, 정식 맵에서는 PlayerStart 또는 별도 스폰 포인트 정책이 필요하다.

## 현재 자산 / 클래스 역할

### `CarFight_ReServer.Target.cs`
- 종류: C# Unreal TargetRules
- 현재 역할: CarFight_Re Dedicated Server 빌드 타깃 정의

### `ACFMPGameMode`
- 종류: C++ GameModeBase
- 현재 역할: 플레이어 로그인 후 차량 Pawn 스폰 및 점유 흐름 담당

### `BP_CFVehiclePawn`
- 종류: Blueprint Pawn
- 현재 역할: 서버 접속 플레이어에게 기본으로 스폰할 차량 Pawn 클래스 fallback

### `PlayerStart`
- 종류: Engine Actor
- 현재 역할: 차량 스폰 위치 우선 후보

### `PlayerController`
- 종류: Engine Controller
- 현재 역할: 로그인 주체이며, 서버가 스폰한 차량 Pawn을 점유하는 주체

## 현재 생성 및 연결 구조
현재 생성 및 연결 구조는 아래와 같다.

```text
Dedicated Server 실행
  -> CarFight_ReServerTarget
  -> ACFMPGameMode
  -> PostLogin(NewPlayer)
  -> SpawnVehicleForController(NewPlayer)
      -> ExistingPawn 확인
      -> ResolveVehiclePawnClass
      -> FindVehicleSpawnTransform
          -> ChoosePlayerStart
          -> fallback Transform
      -> SpawnActor<APawn>
      -> PlayerController.Possess
```

## 현재 기능 책임
현재 `ServerSpawn`의 책임은 아래와 같다.

- 서버 빌드 타깃을 제공한다.
- Dedicated Server 테스트용 GameMode를 제공한다.
- 기본 Pawn 자동 생성 흐름을 피한다.
- 플레이어 로그인 후 차량 Pawn 자동 스폰을 수행한다.
- 이미 Pawn을 가진 컨트롤러의 중복 스폰을 방지한다.
- PlayerStart 또는 fallback 위치로 스폰 Transform을 만든다.
- 서버에서 Pawn을 생성하고 Controller에 점유시킨다.
- 스폰/점유 흐름의 주요 실패 원인을 로그로 남긴다.

## 현재 기준 비책임 항목
현재 구현상 `ServerSpawn`의 직접 책임이 아닌 것은 아래와 같다.

- 계정 로그인/인증
- 세션 브라우저
- 매치메이킹
- 서버 목록 관리
- 방 생성/입장 UI
- 관리툴 연동
- DB 저장
- 플레이어 데이터 로드
- 차량 선택/로드아웃 선택
- 리스폰 시스템
- 팀 스폰/진영 스폰
- 스폰 보호
- 스폰 포인트 점유/예약
- 로그아웃 시 Pawn 제거 정책 확정
- Dedicated Server 운영 자동화 스크립트

현재 `ServerSpawn`은 **멀티플레이 전체 인프라가 아니라, 서버 접속 후 차량 Pawn을 하나 생성해 조작 가능한 상태로 만드는 최소 스폰 계층**이다.

## 현재 문서 기준의 핵심 결론
현재 `ServerSpawn` 기능은,

**Dedicated Server 테스트 환경에서 플레이어가 로그인하면 서버가 차량 Pawn을 스폰하고 해당 PlayerController가 점유하도록 만드는 최소 멀티플레이 진입 기능**이다.

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> `ServerSpawn`은 현재 서버 접속 플레이어에게 기본 차량 Pawn을 생성해 주고, 그 Pawn을 컨트롤러에 점유시켜 멀티플레이 차량 조작 테스트를 가능하게 해주는 현재 상태 기능이다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- 현재 기본 맵/GameMode 설정에서 `ACFMPGameMode`가 실제로 적용되는 최종 경로
- Dedicated Server 실행 스크립트 또는 배치 파일 존재 여부
- 서버 접속 테스트용 맵 이름과 PlayerStart 배치 상태
- 로그아웃 시 서버가 Pawn을 명시적으로 제거할지 여부
- 리스폰/재접속/관전자 처리 정책
- 차량 선택/로드아웃 선택이 들어올 때 `VehiclePawnClass`를 어떻게 바꿀지
- 서버 목록/관리툴/DB와 이 스폰 흐름이 연결될 계획
- 클라이언트 이동/주행 상태 복제 정책과 `VehicleDrive`의 관계

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `CarFight_ReServer.Target.cs` 설정 변경
- `ACFMPGameMode`의 로그인/스폰/점유 흐름 변경
- 기본 차량 Pawn 경로 변경
- `bSpawnVehicleOnPostLogin` 기본값 변경
- PlayerStart/fallback 스폰 위치 정책 변경
- 리스폰 시스템 추가
- 차량 선택/로드아웃 기반 Pawn 클래스 선택이 추가될 때
- 관리툴/DB/세션 시스템과 연결될 때
- Dedicated Server 실행 방식 또는 운영 문서가 추가될 때

## 문서 버전 관리
- 현재 문서 버전: `1.0.0`
- 문서 상태: `Initial`
- 관리 원칙:
  - 이 문서는 한 번 작성하고 끝내는 문서가 아니라, 기능의 현재 상태가 바뀌면 함께 갱신한다.
  - 기능 설명 본문이 바뀌면 체인지로그도 같이 갱신한다.
  - 구현 변경 없이 표현만 다듬은 경우와, 기능 이해에 영향을 주는 내용 변경을 구분해서 기록한다.

### 버전 증가 기준
- `Major`
  - 스폰 구조가 단순 PostLogin 스폰에서 매치/세션/리스폰 시스템으로 재정의될 때
  - 서버 운영 문서와 통합되어 범위가 크게 확장될 때
- `Minor`
  - 리스폰, 차량 선택, 스폰 포인트 정책, 서버 실행 경로가 추가될 때
  - GameMode/맵 적용 경로가 확정될 때
- `Patch`
  - 오탈자 수정
  - 표현 명확화
  - 근거 보강
  - 본문 의미는 유지한 채 설명 정밀도만 올라갈 때

## 체인지로그
### v1.0.0 - 2026-06-02
- `Dedicated Server / Multiplayer Spawn` 문서 최초 작성
- `CarFight_ReServer.Target.cs`와 `ACFMPGameMode` 기준 현재 서버 스폰 흐름 정리
- PostLogin 차량 스폰, PlayerStart/fallback Transform, Possess 흐름 기록
- 현재 비책임 항목과 미확인 항목 분리

## 마지막 확인 기준
- 확인 일시: `2026-06-02`
- 확인 근거:
  - `UE/Source/CarFight_ReServer.Target.cs`
  - `UE/Source/CarFight_Re/Public/CFMPGameMode.h`
  - `UE/Source/CarFight_Re/Private/CFMPGameMode.cpp`
