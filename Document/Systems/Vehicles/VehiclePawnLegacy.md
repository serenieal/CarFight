# VehiclePawnLegacy

## 문서 목적
이 문서는 현재 프로젝트에 남아 있는 `CFModVehiclePawn / BP_ModularVehicle` 계열의 레거시 상태를 정리한다.
이 문서는 기존 레거시 코드를 즉시 삭제하기 위한 문서가 아니라, **현재 주력 차량 Pawn 계열과 레거시 차량 Pawn 계열을 혼동하지 않기 위한 SSOT 정리 문서**다.

## 문서 범위
이 문서에서 다루는 대상은 아래와 같다.

- 레거시 C++ 클래스: `ACFModVehiclePawn`
- 레거시 의도 대상: `BP_ModularVehicle`
- 현재 주력 C++ 클래스: `ACFVehiclePawn`
- 현재 주력 BP 자산: `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
- 관련 컴포넌트: `UCFWheelSyncComp`

이 문서는 `ACFVehiclePawn / BP_CFVehiclePawn`의 상세 기능 문서가 아니다.
현재 목적은 **`ACFModVehiclePawn / BP_ModularVehicle`을 앞으로 현역 기준으로 볼지, 레거시 기준으로 볼지 명확히 하는 것**이다.

## 현재 확인된 결론
현재 기준 결론은 아래와 같다.

> `CFModVehiclePawn / BP_ModularVehicle` 계열은 현재 주력 차량 Pawn 계열이 아니라, BP 중심 `BP_ModularVehicle`을 C++ 부모 클래스로 점진 이전하려던 Step-1 레거시 스켈레톤으로 본다.

현재 주력 차량 흐름은 아래 계열로 본다.

```text
ACFVehiclePawn
  -> BP_CFVehiclePawn
```

현재 레거시/보존 계열은 아래 계열로 본다.

```text
ACFModVehiclePawn
  -> BP_ModularVehicle 예정 또는 과거 대상
```

## 현재 주력 Pawn 계열
현재 주력 Pawn 계열은 `ACFVehiclePawn / BP_CFVehiclePawn`이다.

현재 확인 근거:
- `/Game/CarFight/Vehicles/BP_CFVehiclePawn.uasset` 자산이 존재한다.
- `ACFVehiclePawn`은 `VehicleDriveComp`, `WheelSyncComp`, `VehicleCameraComp`, `VehicleAimComp`를 보유하는 기준 클래스다.
- `BP_CFVehiclePawn`에는 `VehicleData`, 입력 액션, Aim Reticle, VehicleDebug 관련 기본값이 들어가 있다.
- Dedicated Server 테스트용 `ACFMPGameMode`도 기본 차량 BP fallback 경로로 `/Game/CarFight/Vehicles/BP_CFVehiclePawn`을 사용한다.

따라서 현재 개발 기준에서 차량 기능을 추가하거나 문서화할 때는 특별한 이유가 없는 한 `ACFVehiclePawn / BP_CFVehiclePawn`을 기준으로 한다.

## 현재 레거시 Pawn 계열
현재 레거시 Pawn 계열은 `ACFModVehiclePawn / BP_ModularVehicle`이다.

`ACFModVehiclePawn` 헤더 주석에는 다음 의도가 명시되어 있다.

- `BP_ModularVehicle -> C++ base class (skeleton)`
- 목표: BP를 나중에 얇게 만들기 위한 안정적인 C++ 부모 제공
- Step-1 범위: Tick/Input 로직은 아직 BP에 남겨두고, C++로 옮기지 않음

현재 `ACFModVehiclePawn`은 `AWheeledVehiclePawn`을 상속한다.
현재 클래스가 직접 제공하는 것은 사실상 아래 정도다.

- `WheelSyncComp` 캐시 포인터
- `GetWheelSyncComp()` 접근자
- `RefreshWheelSyncComp()` 함수
- `BeginPlay()`에서 `RefreshWheelSyncComp()` 호출

현재 `ACFModVehiclePawn` 생성자는 `WheelSyncComp`를 `CreateDefaultSubobject`로 만들지 않는다.
주석상 이유는 아래와 같다.

- `BP_ModularVehicle`이 이미 `WheelSyncComp`라는 컴포넌트 변수를 가지고 있을 수 있음
- C++에서 또 생성하면 중복 컴포넌트 문제가 날 수 있음
- 따라서 C++에서는 런타임에 `FindComponentByClass<UCFWheelSyncComp>()`로 캐시만 한다.

## 현재 레거시 판정 이유
`ACFModVehiclePawn / BP_ModularVehicle`을 현재 레거시로 보는 이유는 아래와 같다.

### 1. 코드 범위가 Step-1 스켈레톤에 머물러 있다
`ACFModVehiclePawn`은 현재 Tick, 입력, 카메라, Aim, 데이터, 서버 발사 요청, 디버그 UI 흐름을 소유하지 않는다.
현재 클래스의 실질적 책임은 `WheelSyncComp` 캐시 정도다.

### 2. 현재 주력 기능은 `ACFVehiclePawn`에 집중되어 있다
현재 실제 주행/카메라/조준/Reticle/서버 발사 요청 흐름은 `ACFVehiclePawn` 계열에 들어가 있다.

현재 `ACFVehiclePawn` 계열에서 확인되는 주요 기능:
- VehicleData 연결
- Enhanced Input 연결
- VehicleDriveComp
- WheelSyncComp
- VehicleCameraComp
- VehicleAimComp
- Aim Reticle 위젯 생성
- ServerRequestFire / FireResult 처리
- VehicleDebug HUD/Panel

따라서 새 기능의 기준 부모 클래스로 `ACFModVehiclePawn`을 쓰면 현재 주력 시스템과 분리될 위험이 있다.

### 3. `BP_ModularVehicle` 명칭은 현재 소스 검색상 레거시 주석에서만 확인된다
현재 텍스트 검색 기준으로 `BP_ModularVehicle` 명칭은 `CFModVehiclePawn.h/.cpp`의 주석에서만 확인된다.
반면 `/Game/CarFight/Vehicles/BP_CFVehiclePawn.uasset`은 실제 Content 자산으로 존재한다.

따라서 현재 문서 기준으로는 `BP_ModularVehicle`을 현역 기준 BP로 단정하지 않는다.

## 현재 기준 사용 원칙
앞으로 차량 관련 기능을 다룰 때의 기준은 아래와 같다.

### 새 기능 구현 기준
새로운 차량 기능은 기본적으로 아래 계열에 붙인다.

```text
ACFVehiclePawn / BP_CFVehiclePawn
```

예:
- 무기 장착
- 터렛/하드포인트
- 에너지/열 관리
- 서버 권한 발사 처리
- 차량 HUD
- CameraData 연결
- VehicleData 확장
- 복제 정책

### 레거시 계열 수정 기준
`ACFModVehiclePawn / BP_ModularVehicle`은 아래 경우에만 수정한다.

- 실제로 `BP_ModularVehicle` 자산이 필요하다는 별도 확인이 있을 때
- 과거 BP 기능을 마이그레이션하거나 비교해야 할 때
- `ACFModVehiclePawn`을 삭제하기 전 안전하게 의존성을 확인할 때
- 레거시 유지/폐기 여부를 결정하기 위한 조사 작업을 할 때

### 혼용 금지 원칙
같은 기능을 `ACFVehiclePawn`과 `ACFModVehiclePawn` 양쪽에 동시에 추가하지 않는다.

이유:
- 차량 기준 클래스가 둘로 갈라진다.
- BP 설정과 C++ 기본값이 충돌할 수 있다.
- 문서와 실제 구현이 서로 다른 Pawn 계열을 가리키게 된다.
- 1인 개발 조건에서 유지보수 부담이 커진다.

## 현재 레거시 클래스 동작 방식
현재 `ACFModVehiclePawn`의 동작은 매우 단순하다.

```text
ACFModVehiclePawn 생성
  -> WheelSyncComp를 직접 만들지 않음

BeginPlay
  -> RefreshWheelSyncComp
      -> FindComponentByClass<UCFWheelSyncComp>()
      -> WheelSyncComp 캐시
```

현재 의미:
- `ACFModVehiclePawn`은 컴포넌트를 생성하는 주체가 아니다.
- BP가 이미 가지고 있는 컴포넌트를 찾아 캐시하는 보조 부모 클래스다.
- 현재 구조에서는 BP 로직을 C++로 옮기는 작업이 아직 진행되지 않았다.

## 현재 기준 비책임 항목
현재 `ACFModVehiclePawn`의 직접 책임이 아닌 것은 아래와 같다.

- Enhanced Input 처리
- 차량 이동 입력 해석
- DriveState 계산
- WheelSync 전체 준비/갱신 운영
- VehicleData 적용
- VehicleCamera 계산
- VehicleAim 계산
- Aim Reticle 생성
- ServerRequestFire 처리
- Dedicated Server 스폰 기준 Pawn 역할
- VehicleDebug HUD/Panel 기준 데이터 제공

위 항목은 현재 주력 계열인 `ACFVehiclePawn / BP_CFVehiclePawn` 기준으로 봐야 한다.

## 현재 상태 표
| 항목 | 현재 상태 | 기준 판단 |
|---|---|---|
| `ACFVehiclePawn` | 현역 C++ 기준 차량 Pawn | 주력 |
| `BP_CFVehiclePawn` | 실제 Content 차량 BP 자산 존재 | 주력 |
| `ACFModVehiclePawn` | Step-1 스켈레톤 C++ 부모 | 레거시/보존 |
| `BP_ModularVehicle` | 현재 소스상 레거시 주석에서 확인 | 현역 여부 미확정 |
| `UCFWheelSyncComp` | 주력/레거시 양쪽에서 언급 가능 | 현재 주력은 `ACFVehiclePawn` 기준 |

## 권장 후속 정리
현재 레거시 계열은 바로 삭제하지 않고 아래 순서로 정리하는 것을 권장한다.

### 1단계: 의존성 확인
확인할 것:
- 실제 Content에 `BP_ModularVehicle` 자산이 존재하는지
- 맵이나 테스트 레벨에서 `BP_ModularVehicle`을 배치하고 있는지
- 어떤 BP가 `ACFModVehiclePawn`을 부모로 삼고 있는지
- `CFModVehiclePawn.h/.cpp`가 빌드 외에 실제 런타임에서 참조되는지

### 2단계: 유지/폐기 결정
의존성이 없다면:
- `ACFModVehiclePawn`을 Deprecated 후보로 둔다.
- 삭제 전 커밋 단위로 분리한다.
- 문서에서 폐기 결정 버전을 기록한다.

의존성이 있다면:
- `BP_ModularVehicle`을 현역으로 복귀시킬지 결정한다.
- 복귀한다면 `ACFVehiclePawn`과 역할을 다시 나눠야 한다.
- 특별한 이유가 없다면 `BP_ModularVehicle` 기능을 `BP_CFVehiclePawn` 계열로 마이그레이션한다.

### 3단계: 삭제 또는 보관
삭제할 경우:
- `CFModVehiclePawn.h`
- `CFModVehiclePawn.cpp`
- Build 참조 확인
- BP 부모 참조 확인
- 문서 체인지로그 기록

보관할 경우:
- 파일 상단에 `Legacy` 또는 `Deprecated Candidate` 주석을 추가한다.
- 새 기능 추가 금지 원칙을 문서에 유지한다.

## 현재 문서 기준의 핵심 결론
현재 `CFModVehiclePawn / BP_ModularVehicle`은,

**과거 BP 중심 차량을 C++ 부모 클래스로 옮기기 위한 Step-1 스켈레톤이며, 현재 주력 차량 기능 기준은 아니다.**

이 문서에서 가장 중요하게 봐야 할 현재 역할은 다음 한 줄로 요약할 수 있다.

> 앞으로 새 차량 기능은 `ACFVehiclePawn / BP_CFVehiclePawn` 기준으로 추가하고, `ACFModVehiclePawn / BP_ModularVehicle`은 의존성 확인 전까지 레거시/보존 대상으로만 다룬다.

## 현재 문서에서 미확인인 항목
아래는 아직 이 문서에서 확정하지 않은 내용이다.

- 실제 Content 폴더 전체에서 `BP_ModularVehicle` 자산이 존재하는지 최종 확인
- 맵/레벨에서 `BP_ModularVehicle`을 참조하는지 여부
- `ACFModVehiclePawn`을 부모로 사용하는 BP가 남아 있는지 여부
- `ACFModVehiclePawn`을 삭제해도 빌드/에디터 로드에 문제가 없는지 여부
- 레거시 계열을 삭제할지, Deprecated 상태로 보관할지 최종 결정

## 문서 갱신 조건
아래 변경이 생기면 이 문서를 함께 갱신한다.

- `BP_ModularVehicle` 자산 존재 여부가 확정될 때
- `ACFModVehiclePawn`을 삭제하거나 Deprecated 처리할 때
- `ACFModVehiclePawn`을 다시 현역 계열로 복귀시키기로 결정할 때
- `BP_CFVehiclePawn`에서 `BP_ModularVehicle`로 기능 기준이 바뀔 때
- `CFModVehiclePawn.h/.cpp`에 새 기능이 추가될 때

## 문서 버전 관리
- 현재 문서 버전: `1.0.0`
- 문서 상태: `Initial`
- 관리 원칙:
  - 이 문서는 레거시 계열을 혼동하지 않기 위한 기준 문서다.
  - 레거시 유지/폐기 결정이 바뀌면 반드시 갱신한다.
  - 구현 변경 없이 표현만 다듬은 경우와, 기준 판단이 바뀐 경우를 구분한다.

### 버전 증가 기준
- `Major`
  - `ACFModVehiclePawn / BP_ModularVehicle`을 다시 현역 기준으로 복귀시킬 때
  - 레거시 계열을 삭제하고 문서 목적을 폐기 기록으로 바꿀 때
- `Minor`
  - 자산 의존성 확인 결과가 추가될 때
  - Deprecated 정책이나 마이그레이션 경로가 추가될 때
- `Patch`
  - 오탈자 수정
  - 표현 명확화
  - 근거 보강
  - 본문 의미는 유지한 채 설명 정밀도만 올라갈 때

## 체인지로그
### v1.0.0 - 2026-06-02
- `CFModVehiclePawn / BP_ModularVehicle` 레거시 상태 정리 문서 최초 작성
- 현재 주력 계열을 `ACFVehiclePawn / BP_CFVehiclePawn`으로 명시
- `ACFModVehiclePawn`을 Step-1 스켈레톤 레거시/보존 대상으로 분류
- 새 기능은 `ACFVehiclePawn / BP_CFVehiclePawn` 기준으로 추가한다는 원칙 기록

## 마지막 확인 기준
- 확인 일시: 2026-06-02
- 확인 근거:
  - `UE/Source/CarFight_Re/Public/CFModVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFModVehiclePawn.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `/Game/CarFight/Vehicles/BP_CFVehiclePawn`
  - `UE/Content/CarFight/Vehicles/BP_CFVehiclePawn.uasset`
