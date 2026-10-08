# CFVNetDbgFix Codex 작업지시서

- 문서 버전: v0.1.0
- 작성일: 2026-06-05
- 대상 프로젝트: CarFight
- 대상 UE 프로젝트: UE/CarFight_Re.uproject
- 기준 폴더: Document/Plan/ServerUpgradePlan/
- 작업 유형: 차량 네트워크 물리 진단 보강 패치
- 상태: ReadyForCodex

---

## 1. 작업 목적

`CFVNetDbg` 1차 진단 패치의 런타임 로그에서 확인된 문제를 보강한다.

이번 작업은 차량 네트워크 이동의 최종 해결 패치가 아니다. 목적은 다음 테스트 로그가 신뢰 가능한 진단값을 갖도록 만드는 것이다.

```text
- SampleAge 음수 문제 수정
- 서버/클라이언트 Role/소유/복제 상태 로그 보강
- AngularVelocity 진단값 추가
- 동일 PC 2클라 게임패드 입력 오염 조건을 구분할 수 있는 최소 입력 로그 추가
```

---

## 2. 현재 관찰 결과

### 2.1 RepMove=true 로그

`RuntimeLogs/VehicleNet/A_RepMoveTrue`에서 큰 오차가 관찰되었다.

예시:

```text
RepMove=true
LocErr=1355.87cm
RotErr=45.40deg
```

또 다른 예시:

```text
RepMove=true
LocErr=440.12cm
RotErr=3.45deg
VelErr=1035.48cm/s
SpeedErr=37.27km/h
```

판단:

```text
bReplicateMovement=true 상태에서도 서버/클라 위치, 회전, 속도 오차가 순간적으로 크게 발생한다.
```

### 2.2 RepMove=false 로그

`RuntimeLogs/VehicleNet/B_RepMoveFalse`에서 더 큰 장기 오차가 관찰되었다.

예시:

```text
RepMove=false
LocErr=1280.60cm
RotErr=88.02deg
```

또 다른 예시:

```text
RepMove=false
LocErr=2421.02cm
RotErr=135.83deg
```

판단:

```text
bReplicateMovement=false 상태에서는 시각적 끊김은 줄어도 서버/클라 위치와 회전이 크게 갈라진다.
```

### 2.3 SampleAge 문제

현재 로그의 `SampleAge`가 다음처럼 음수로 출력된다.

```text
SampleAge=-12.821s
SampleAge=-34.372s
SampleAge=-96.006s
```

원인 추정:

```text
서버 프로세스의 GetWorld()->GetTimeSeconds()와 클라이언트 프로세스의 GetWorld()->GetTimeSeconds()를 직접 비교하고 있다.
Dedicated Server와 Client는 월드 시작 시각이 다르므로 직접 비교하면 음수가 될 수 있다.
```

수정 방향:

```text
GameState->GetServerWorldTimeSeconds() 기준으로 서버 샘플 시간과 클라이언트 현재 서버 시간을 맞춘다.
```

### 2.4 게임패드 입력 오염 조건

동일 PC 2클라 테스트에서 Client 1의 게임패드 스틱 입력이 Client 2에도 적용되는 현상이 확인되었다.

판단:

```text
이 현상은 네트워크 복제 문제가 아니라 같은 물리 게임패드를 두 UE 프로세스가 동시에 읽는 로컬 입력 폴링 문제일 가능성이 높다.
```

이번 작업에서 입력 시스템을 완전히 고치지는 않는다. 다만 로그 해석을 위해 최소한 아래 항목을 로그에 남긴다.

```text
- IsLocallyControlled
- OwnerController
- LocalRole / RemoteRole
- 가능하면 현재 Drive 입력값 또는 최근 입력 디바이스 모드
```

---

## 3. scope.in

- `CFVehiclePawn.h` / `CFVehiclePawn.cpp`의 `VehicleNetDebug` 진단 로그 보강
- `SampleAge` 계산을 서버 동기 시간 기준으로 수정
- 서버 샘플 구조에 `ServerAngularVelocityDeg` 추가
- 로그에 `AngularVelocityErrorDegPerSec` 추가
- 로그에 `NetMode`, `HasAuthority`, `IsLocallyControlled`, `bReplicates`, `OwnerController` 추가
- 로그에 `TimeSource` 또는 서버 시간 유효 여부 추가
- 가능하면 현재 입력 관련 최소 정보 추가
  - `InputDeviceMode`
  - 현재 Throttle/Brake/Steer/Handbrake 값에 접근 가능한 경우만 추가
- `VehicleNetDebug` 로그 문자열은 기존 grep 패턴이 깨지지 않도록 접두어 `VehicleNetDebug:` 유지
- 문서에 테스트 주의사항을 추가 또는 갱신

---

## 4. scope.out

- 차량 이동 보정 구현 금지
- 원격 차량 보간 구현 금지
- 소유 차량 예측/서버 보정 구현 금지
- `bReplicateMovement` 기본 정책 변경 금지
- 무기/발사/대미지 서버 권한 구조 작업 금지
- 게임패드 입력 시스템 대수정 금지
- BP_CFVehiclePawn 에셋 자동 수정 금지

---

## 5. constraints

- 파일명 32자 이하 유지
- 모든 신규 변수/함수 위에 한 줄 한국어 주석 작성
- 모든 UPROPERTY/UFUNCTION에는 한국어 `DisplayName`과 `ToolTip` 작성
- 기존 `VehicleNetDebug:` 로그 접두어 유지
- `SetActorLocation`, `SetActorRotation`, `TeleportTo` 호출 금지
- 진단 패치 외 구조 변경 금지
- 버전은 `ACFVehiclePawn` 기준 v2.23.0으로 표기
- Changelog와 Migration 메모 작성

---

## 6. 대상 파일

### 수정

```text
UE/Source/CarFight_Re/Public/CFVehiclePawn.h
UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp
```

### 문서 갱신

```text
Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetDbgFix_Review.md
Document/Plan/ServerUpgradePlan/VehicleNetPhysicsJitter.md
```

문서 갱신은 코드 빌드 성공 이후 수행한다.

---

## 7. 구현 지시

### 7.1 `CFVehiclePawn.h`

`FCFVehicleNetDebugSample`에 아래 필드를 추가한다.

```cpp
// [v2.23.0] 서버 권위 차량 각속도(deg/s)입니다.
UPROPERTY(BlueprintReadOnly, Category="CarFight|VehiclePawn|NetDebug", meta=(DisplayName="서버 각속도 (ServerAngularVelocityDeg)", ToolTip="서버가 캡처한 차량의 물리 각속도(deg/s)입니다."))
FVector ServerAngularVelocityDeg = FVector::ZeroVector;

// [v2.23.0] 서버 샘플 시각이 GameState 서버 시간 기준으로 캡처되었는지 여부입니다.
UPROPERTY(BlueprintReadOnly, Category="CarFight|VehiclePawn|NetDebug", meta=(DisplayName="서버 시간 유효 (bServerTimeValid)", ToolTip="True이면 ServerWorldTimeSeconds가 GameState 서버 시간 기준으로 캡처되었습니다."))
bool bServerTimeValid = false;
```

필요하면 `ACFVehiclePawn`에 아래 헬퍼 함수를 추가한다.

```cpp
// [v2.23.0] 진단 로그용 서버 기준 시간을 반환합니다.
float ResolveVehicleNetDebugServerTimeSeconds(bool& bOutServerTimeValid) const;

// [v2.23.0] 현재 차량의 물리 각속도를 deg/s 단위로 반환합니다.
FVector GetVehicleNetDebugAngularVelocityDeg() const;

// [v2.23.0] 네트워크 모드를 진단 로그용 문자열로 반환합니다.
const TCHAR* GetVehicleNetDebugNetModeName() const;
```

주의:

```text
- 함수명은 32자 제한 파일명과 무관하지만 직관적으로 유지한다.
- 이미 유사 함수가 있으면 중복 생성하지 말고 재사용한다.
```

---

### 7.2 `CFVehiclePawn.cpp` - 서버 시간 수정

현재 코드의 문제 지점:

```cpp
const float SampleAgeSec = CurrentWorldTimeSec - VehicleNetDebugServerSample.ServerWorldTimeSeconds;
```

이 계산은 서버/클라 프로세스의 `GetWorld()->GetTimeSeconds()` 기준이 달라 음수가 된다.

수정 방향:

```cpp
bool bCurrentServerTimeValid = false;
const float CurrentServerTimeSeconds = ResolveVehicleNetDebugServerTimeSeconds(bCurrentServerTimeValid);
const float SampleAgeSec = (bCurrentServerTimeValid && VehicleNetDebugServerSample.bServerTimeValid)
    ? CurrentServerTimeSeconds - VehicleNetDebugServerSample.ServerWorldTimeSeconds
    : -1.0f;
```

`CaptureVehicleNetDebugSample`에서도 서버 샘플 시간을 아래 방식으로 저장한다.

```cpp
bool bSampleServerTimeValid = false;
NewDebugSample.ServerWorldTimeSeconds = ResolveVehicleNetDebugServerTimeSeconds(bSampleServerTimeValid);
NewDebugSample.bServerTimeValid = bSampleServerTimeValid;
```

`ResolveVehicleNetDebugServerTimeSeconds` 구현 기준:

```text
1. GetWorld()가 없으면 0.0f 반환, bOutServerTimeValid=false
2. GameState가 있으면 GameState->GetServerWorldTimeSeconds() 사용, bOutServerTimeValid=true
3. GameState가 없으면 GetWorld()->GetTimeSeconds() fallback, bOutServerTimeValid=false
```

필요 include 후보:

```cpp
#include "GameFramework/GameStateBase.h"
#include "Components/PrimitiveComponent.h"
```

---

### 7.3 `CFVehiclePawn.cpp` - 각속도 추가

서버 샘플 캡처 시:

```cpp
NewDebugSample.ServerAngularVelocityDeg = GetVehicleNetDebugAngularVelocityDeg();
```

클라이언트 로그 시:

```cpp
const FVector LocalAngularVelocityDeg = GetVehicleNetDebugAngularVelocityDeg();
const float AngularVelocityErrorDegPerSec = FVector::Dist(LocalAngularVelocityDeg, VehicleNetDebugServerSample.ServerAngularVelocityDeg);
```

각속도 획득 기준:

```text
1. RootComponent가 UPrimitiveComponent이면 GetPhysicsAngularVelocityInDegrees 사용
2. RootComponent가 적합하지 않으면 GetMesh() 또는 차량 Mesh의 PrimitiveComponent를 사용
3. 모두 실패하면 FVector::ZeroVector 반환
```

---

### 7.4 `CFVehiclePawn.cpp` - 로그 보강

기존 로그 접두어는 유지한다.

```text
VehicleNetDebug:
```

기존 로그 항목 유지:

```text
Pawn
Role
RemoteRole
RepMove
Seq
SampleAge
LocErr
RotErr
VelErr
SpeedErr
ServerLoc
LocalLoc
ServerVel
LocalVel
```

추가 로그 항목:

```text
NetMode
HasAuthority
IsLocalCtrl
bReplicates
OwnerCtrl
ServerTimeValid
CurrentServerTimeValid
ServerAngVel
LocalAngVel
AngVelErr
InputDeviceMode
```

가능하면 추가할 입력값:

```text
Throttle
Brake
Steer
Handbrake
```

단, 현재 DriveComp/Pawn에서 안정적으로 접근 가능한 값이 없으면 무리하게 구조 변경하지 않는다.

---

### 7.5 Tick 위치 검토

현재 `UpdateVehicleNetDebug(DeltaSeconds)`는 `bVehicleRuntimeReady` 검사 이전에 호출된다.

수정 권장:

```text
- 진단 로그가 초기화 전 낙하/스폰 안정화 구간까지 필요하면 유지
- 실제 주행 진단만 필요하면 bVehicleRuntimeReady 이후로 이동
```

이번 작업에서는 아래 절충안을 적용한다.

```text
- 함수는 유지
- 로그 문자열에 RuntimeReady=true/false를 추가
```

즉, Tick 위치를 크게 바꾸지 말고 `RuntimeReady` 값을 로그에 추가한다.

---

## 8. acceptance

작업 완료 판정 기준은 다음과 같다.

```text
- `VehicleNetDebug:` 로그 접두어가 유지된다.
- `SampleAge`가 정상 Dedicated Server 2클라 테스트에서 지속적인 큰 음수로 출력되지 않는다.
- `ServerTimeValid` 또는 동등한 시간 유효성 로그가 출력된다.
- `NetMode`, `HasAuthority`, `IsLocallyControlled`, `bReplicates`, `OwnerController` 또는 동등한 항목이 로그에 추가된다.
- `ServerAngularVelocityDeg`, `LocalAngularVelocityDeg`, `AngularVelocityErrorDegPerSec` 또는 동등한 각속도 항목이 로그에 추가된다.
- 기존 `LocErr`, `RotErr`, `VelErr`, `SpeedErr`, `RepMove`, `Seq` 로그는 제거되지 않는다.
- 차량 Transform 보정, 원격 보간, 소유 차량 예측 코드는 추가되지 않는다.
- `bReplicateMovement` 기본 정책은 변경하지 않는다.
- Editor / Server 빌드가 성공한다.
```

---

## 9. verification

검증 절차는 다음 순서로 수행한다.

```text
1. 코드 리뷰로 `SetActorLocation`, `SetActorRotation`, `TeleportTo`가 진단 패치에 추가되지 않았는지 확인한다.
2. `CarFight_ReEditor Win64 Development` 빌드를 수행한다.
3. `CarFight_ReServer Win64 Development` 빌드를 수행한다.
4. `VehicleNetDebug:` 로그 문자열이 유지되는지 확인한다.
5. Dedicated Server 2클라 테스트에서 `SampleAge`가 큰 음수로 계속 찍히지 않는지 확인한다.
6. `bReplicateMovement=true`와 `false` 각각에서 로그가 출력되는지 확인한다.
7. 게임패드 대신 키보드 입력으로 Client 1만 조작해 입력 오염이 줄어든 조건에서 재수집한다.
```

---

## 10. 빌드 검증

Codex 작업 후 아래 빌드를 수행한다.

```text
CarFight_ReEditor Win64 Development
CarFight_ReServer Win64 Development
```

성공 기준:

```text
- 두 타겟 모두 빌드 성공
- `VehicleNetDebug:` 로그 접두어 유지
- `SampleAge`가 정상 테스트에서 음수로 계속 나오지 않음
- 기존 LocErr/RotErr/VelErr/SpeedErr 로그 유지
- Transform 보정/보간/예측 코드가 추가되지 않음
```

---

## 9. 런타임 재테스트 지시

빌드 성공 후 아래 조건으로 재테스트한다.

### 9.1 게임패드 입력 오염 회피

이번 재테스트는 게임패드를 사용하지 않는다.

```text
- Client 1 창에 포커스
- 키보드로만 조작
- Client 2 창은 관찰용으로 둔다
```

이유:

```text
동일 PC에서 두 UE 클라이언트가 같은 물리 게임패드를 동시에 읽을 수 있어 입력이 오염된다.
```

### 9.2 테스트 세트

```text
A. bReplicateMovement=true
B. bReplicateMovement=false
```

### 9.3 로그 추출

기존 스크립트를 그대로 사용한다.

```text
Tools/RunNetTrue.bat
Tools/RunNetFalse.bat
Tools/ExtractNetLog.bat
```

---

## 10. 산출물

Codex는 작업 완료 후 아래를 남긴다.

```text
Document/Plan/ServerUpgradePlan/CodexTasks/CFVNetDbgFix_Review.md
```

리뷰 문서에 포함할 것:

```text
- 수정 파일 목록
- 추가된 로그 항목
- SampleAge 수정 방식
- 게임패드 입력 오염 주의사항
- Editor / Server 빌드 결과
- Changelog
- Migration
```

---

## 11. Changelog

### v0.1.0

```text
- CFVNetDbgFix Codex 작업지시서 작성
- SampleAge 음수 문제 수정 방향 정의
- 각속도/Role/소유/복제/시간 유효성 로그 보강 범위 정의
- 동일 PC 2클라 게임패드 입력 오염 조건을 재테스트 주의사항으로 명시
```
