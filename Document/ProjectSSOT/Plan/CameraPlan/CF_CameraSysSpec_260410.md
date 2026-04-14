# CarFight 차량 카메라 시스템 세부 설계안

- Version: 0.1.0
- Date: 2026-04-10
- Status: Draft
- Scope: 차량 카메라 기획 문서를 구현 가능한 시스템 구조로 구체화한 세부 설계 문서
- Base Document: `Document/ProjectSSOT/Plan/CameraPlan/CF_CameraPlan_260410.md`

---

## 1. 문서 목적

이 문서는 차량 카메라 기획 회의 문서에서 합의된 방향을 실제 구현 가능한 시스템 설계 수준으로 구체화하기 위한 문서다.

이번 문서에서 다루는 목표는 다음과 같다.

1. 차량 카메라의 책임과 범위를 명확히 정의한다.
2. Pawn, Camera Component, DataAsset, UI, Weapon 시스템 사이의 역할을 분리한다.
3. 카메라 입력 처리, 상태 전이, 회전 제한, 충돌 보정, 조준점 계산 흐름을 고정한다.
4. 이후 C++ 구현과 Blueprint 조립 단계에서 기준이 흔들리지 않도록 세부 구조를 정리한다.

---

## 2. 상위 설계 원칙

이 문서는 기존 기획 문서의 다음 원칙을 그대로 따른다.

1. CarFight의 차량 카메라는 일반 레이싱용 chase camera가 아니라 차량 기반 3인칭 슈팅 카메라다.
2. 차량 이동과 카메라 회전은 분리한다.
3. 카메라 회전의 주 목적은 운전 보조보다 조준이다.
4. 카메라 회전 범위는 현재 활성 무기 그룹의 조준 가능 범위에 의해 제한된다.
5. 전방 시야 손실은 제거 대상이 아니라 의도된 리스크이자 플레이 포인트다.
6. 카메라는 월드 오브젝트와 겹치면 안 된다.
7. 충돌 처리에서는 관통 방지뿐 아니라 조준 감각과 가시성 유지도 중요하다.
8. 카메라의 공간상 위치와 회전 피벗은 분리해서 생각한다.
9. 카메라 피벗은 수평적으로 차량 중심에 가깝게 둔다.
10. 카메라는 공간적으로 뒤/위/옆에 있을 수 있지만, 회전 감각은 차량 중심 기준 오비트에 가깝다.

---

## 3. 시스템 목표

이번 카메라 시스템의 1차 목표는 다음과 같다.

### 3.1 플레이 목표

- 플레이어가 차량을 운전하면서 자유 조준할 수 있어야 한다.
- 카메라가 차량과 분리되어 회전하더라도 내 차량이 화면에서 안정적으로 보여야 한다.
- 무기별 조준 가능 범위 차이가 카메라 체감으로 자연스럽게 드러나야 한다.
- 전방 시야 손실이 플레이 리스크로 작동해야 한다.
- 벽 근처, 좁은 지형, 차량 충돌 상황에서도 조준 감각이 급격히 무너지지 않아야 한다.

### 3.2 구현 목표

- 현재 `ACFVehiclePawn` 구조에 큰 충돌 없이 들어갈 수 있어야 한다.
- Thin BP 원칙을 유지해야 한다.
- DataAsset + C++ 중심 구조를 유지해야 한다.
- 무기 미확정 상태에서도 공통 카메라 뼈대를 먼저 만들 수 있어야 한다.
- 나중에 무기 시스템이 들어와도 카메라를 뜯어고치지 않고 프로필만 바꾸면 되도록 설계해야 한다.

---

## 4. 시스템 개요

차량 카메라 시스템은 아래 5개 층으로 나눈다.

### 4.1 Vehicle Layer

차량 자체의 이동, 조향, 속도, 공중 상태, 드리프트 상태를 제공한다.

주요 소유자:
- `ACFVehiclePawn`
- `UCFVehicleDriveComp`

### 4.2 Camera Anchor Layer

차량 기준 좌표계에서 카메라 피벗과 카메라 기준점을 제공한다.

주요 소유자:
- `ACFVehiclePawn`
- Scene Component 계층

### 4.3 Camera Logic Layer

카메라 모드 계산, 회전 제한, 충돌 보정, 보간, FOV, 조준 계산을 담당한다.

주요 소유자:
- `UCFVehicleCameraComp` 신규 도입 권장

### 4.4 Weapon Aim Constraint Layer

현재 활성 무기 그룹의 조준 가능 범위를 정의한다.

주요 소유자:
- 무기 시스템
- Aim Profile DataAsset

### 4.5 HUD / Feedback Layer

조준점, 사격 가능 여부, 조준 한계, 진행 방향, 가림 상태를 화면에 표시한다.

주요 소유자:
- UI Widget
- Camera Component에서 제공하는 읽기 전용 상태값

---

## 5. 권장 클래스 / 파일 구조

파일 이름은 32자를 넘기지 않도록 다음 이름을 권장한다.

### 5.1 신규 권장 파일

- `CFVehicleCameraComp.h`
- `CFVehicleCameraComp.cpp`
- `CFVehicleCameraTypes.h`
- `CFVehicleCameraData.h`
- `CFWeaponAimProfile.h`
- `CFWeaponAimProfile.cpp` 필요 시

### 5.2 기존 파일과의 연결 지점

- `CFVehiclePawn.h`
- `CFVehiclePawn.cpp`
- `CFVehicleData.h`

### 5.3 역할 요약

#### `ACFVehiclePawn`

역할:
- 차량 카메라 관련 Scene Component 소유
- 입력 액션 바인딩
- Vehicle runtime 상태 제공
- CameraComp가 참조할 차량 상태 제공

#### `UCFVehicleCameraComp`

역할:
- 카메라 모드 결정
- 자유 조준 각도 누적
- 무기 제한각 적용
- 최종 카메라 위치/회전 계산
- 충돌 보정
- 조준점 계산
- HUD용 상태 산출

#### `FCFVehicleCameraConfig` 또는 `UCFVehicleCameraData`

역할:
- 차량 공통 카메라 튜닝 데이터 제공
- 기본 거리, 높이, FOV, 충돌 값, 보간 속도 보유

#### `UCFWeaponAimProfile`

역할:
- 현재 무기 그룹 기준 조준 가능 범위 정의
- Yaw/Pitch 제한
- 무기별 화면 프레이밍 보정
- 무기별 FOV/거리 Modifier 제공

---

## 6. Pawn 구성 설계

현재 `ACFVehiclePawn`에는 DriveComp, WheelSyncComp, VehicleData, 입력 액션 구조가 이미 존재한다. 여기에 카메라 관련 요소를 아래처럼 추가하는 방향을 권장한다.

### 6.1 신규 컴포넌트 권장 구성

`ACFVehiclePawn` 소유 기준:

- `CameraPivotRoot`
- `CameraAimPivot`
- `CameraBoom`
- `FollowCamera`
- `VehicleCameraComp`

### 6.2 각 컴포넌트 역할

#### `CameraPivotRoot`

역할:
- 차량 중심 수평 피벗 기준점
- 카메라 회전의 1차 기준점

설계 원칙:
- XY는 차량 중심에 가깝게 둔다.
- Z는 시각 테스트 전까지 임시값으로 두되, 차체가 잘 보이도록 약간 위에 둔다.
- 차량 메쉬 기준 하드코딩 대신 BP 또는 DataAsset 튜닝 가능 구조를 권장한다.

#### `CameraAimPivot`

역할:
- 실제 자유 조준 회전이 적용되는 피벗
- Yaw/Pitch 누적 적용 지점

설계 원칙:
- `CameraPivotRoot`의 자식으로 둔다.
- 무기 제한각이 적용되는 축이다.
- 회전값은 VehicleCameraComp가 소유하고, Scene Component에는 결과만 반영한다.

#### `CameraBoom`

역할:
- 카메라 거리 유지
- 충돌 시 거리 축소
- 3인칭 외부 시점 형성

설계 원칙:
- Spring Arm 기반 출발 가능
- 다만 기본 동작만 사용하지 말고 CameraComp가 길이/보간/충돌 결과를 적극 제어한다.

#### `FollowCamera`

역할:
- 실제 플레이어가 보는 카메라
- 최종 위치/회전/FOV 적용 대상

#### `VehicleCameraComp`

역할:
- 카메라의 두뇌 역할
- 컴포넌트 위치 계산과 모드 전이를 소유

### 6.3 Pawn이 제공해야 하는 읽기 전용 인터페이스

Pawn은 CameraComp에 다음 정보를 제공할 수 있어야 한다.

- 현재 차량 속도
- 차량 전진 벡터
- 차량 오른쪽 벡터
- 차량 Up 벡터
- 현재 DriveState
- 드리프트 여부
- 공중 상태 여부
- 후진 여부
- 현재 활성 무기 그룹 ID
- 현재 활성 Aim Profile 참조

---

## 7. 카메라 컴포넌트 설계

## 7.1 컴포넌트 채택 이유

이번 설계에서는 `PlayerCameraManager` 중심보다 `UActorComponent` 중심이 더 적합하다.

이유는 다음과 같다.

1. 카메라 기준점이 차량 Pawn 구조에 강하게 결합된다.
2. 차량별 튜닝과 BP 조립이 쉽다.
3. Thin BP + C++ 중심 구조와 잘 맞는다.
4. 무기/차량별 프로필 결합이 명확하다.

따라서 1차 구현은 `UCFVehicleCameraComp`를 기준으로 설계한다.

## 7.2 `UCFVehicleCameraComp`가 소유할 핵심 런타임 값

### 입력 누적값

- `RawLookInputX`
- `RawLookInputY`
- `AccumulatedAimYaw`
- `AccumulatedAimPitch`

### 제한 적용 후 값

- `ClampedAimYaw`
- `ClampedAimPitch`

### 최종 카메라 상태

- `DesiredCameraRotation`
- `DesiredCameraLocation`
- `SolvedCameraLocation`
- `SolvedCameraRotation`
- `CurrentCameraFOV`
- `CurrentArmLength`

### HUD용 상태

- `bAimBlocked`
- `bAimAtLimitYaw`
- `bAimAtLimitPitch`
- `bWeaponCanFireAtCurrentAim`
- `CurrentAimHitLocation`
- `CurrentAimTraceDistance`

### 모드 상태

- `CurrentCameraMode`
- `PreviousCameraMode`
- `bCameraModeChangedThisFrame`

---

## 8. 카메라 모드 설계

현재 설계에서는 카메라 모드를 아래처럼 나눈다.

### 8.1 `Normal`

역할:
- 기본 주행 상태
- 자유 조준 유지
- 무기별 제한각 적용

설명:
- 이 게임은 기본 상태부터 자유 조준 감각을 갖는 편이 자연스럽다.
- 따라서 `Normal`도 일반 운전용 카메라가 아니라 조준 가능한 차량 TPS 카메라다.

### 8.2 `Combat`

역할:
- 교전 집중 상태
- 사격 중 또는 타겟 유지 상태

설명:
- 기본 조준 규칙은 `Normal`과 공유할 수 있다.
- 차이는 FOV, 거리, 타겟 프레이밍, 반응 속도 같은 Modifier 수준으로 관리한다.

### 8.3 `Reverse`

역할:
- 후진 상태에서 시야 확보

설명:
- 후진 보조는 넣을 수 있지만, 무기 규칙과 충돌하지 않도록 별도 규칙이 필요하다.
- 단순 뒤보기 카메라와 실제 후방 사격용 카메라는 구분 가능성을 열어둔다.

### 8.4 `Airborne`

역할:
- 점프 또는 공중 상태

설명:
- Pitch 반응을 약하게 적용
- 착지 충격을 아주 약하게 반영

### 8.5 `Destroyed`

역할:
- 차량 파괴 또는 행동 불능 상태

설명:
- 연출 우선이 아니라 상황 파악 우선
- 조작 종료와 함께 짧은 전환 처리 가능

### 8.6 `Spectate`

역할:
- 향후 관전자 또는 리스폰 대기 상태

### 8.7 상태 우선순위 권장

1. `Destroyed`
2. `Spectate`
3. `Reverse`
4. `Airborne`
5. `Combat`
6. `Normal`

단, 실제 구현에서는 `Reverse`와 `Combat`가 동시에 켜질 여지가 있으므로 완전 배타 상태머신보다는 BaseMode + Modifier 구조가 더 유연할 수 있다.

권장 1차 방식:
- BaseMode: `Normal / Destroyed / Spectate`
- Modifier: `Combat / Reverse / Airborne`

---

## 9. 입력 처리 설계

## 9.1 입력 철학

차량 입력과 카메라 입력은 분리한다.

- 차량 입력: 스로틀, 조향, 브레이크, 핸드브레이크
- 카메라 입력: Look X, Look Y

카메라가 보는 방향이 차량 이동 방향을 직접 결정하지 않는다.

## 9.2 Pawn에 추가 권장 입력 액션

- `InputAction_Look`
- `InputAction_CameraReset` 선택
- `InputAction_AimModifier` 선택
- `InputAction_RearView` 선택

### 9.3 입력 처리 순서

1. Player Input 수신
2. Pawn이 Look 입력을 CameraComp로 전달
3. CameraComp가 누적 Yaw/Pitch 갱신
4. 활성 Aim Profile 기준으로 Clamp 적용
5. 최종 카메라 회전 계산
6. 최종 조준점과 무기 사격 가능 상태 계산

### 9.4 Look 입력과 차량 조향의 분리 규칙

- 마우스/스틱 Look 입력은 절대 Steering으로 재해석하지 않는다.
- Steering은 오직 차량 조향 입력이다.
- Look과 Steering을 같은 축으로 쓰더라도 시스템 내부에서는 완전히 분리된 값으로 관리한다.

---

## 10. 카메라 회전 계산 설계

## 10.1 좌표 기준

카메라 제한각은 차량 로컬 기준으로 계산한다.

기준:
- 차량 Forward 기준 0도
- 오른쪽 회전은 +Yaw
- 왼쪽 회전은 -Yaw
- 위쪽은 +Pitch
- 아래쪽은 -Pitch

## 10.2 계산 순서

1. Raw Look 입력을 Delta Yaw/Pitch로 변환
2. 누적 각도 갱신
3. 활성 Aim Profile의 Min/Max Yaw/Pitch 범위로 Clamp
4. Clamp 결과를 `CameraAimPivot`에 반영
5. 피벗 회전에 따라 원하는 카메라 위치를 계산
6. 충돌 보정 후 최종 카메라 Transform 확정

## 10.3 자동 센터링 기본 정책

초기 설계에서는 자동 센터링을 기본 활성화하지 않는다.

이유:
- 이 게임의 카메라는 자유 조준이 핵심이다.
- 플레이어가 옆이나 뒤를 보는 리스크가 게임성이다.
- 자동으로 차량 정면으로 되돌아오면 의도된 플레이 감각이 약해진다.

후속 옵션으로 다음 정도는 검토 가능하다.

- 특정 시간 입력이 없을 때 아주 약한 복귀
- 카메라 리셋 버튼으로 즉시 전방 복귀

그러나 1차 구현 기준은 `자동 복귀 없음`을 권장한다.

---

## 11. 무기 제한각 설계

## 11.1 핵심 규칙

현재 활성 무기 그룹의 조준 가능 범위가 카메라 회전 가능 범위가 된다.

따라서 CameraComp는 매 프레임 활성 Aim Profile을 읽고 Yaw/Pitch Clamp 범위를 갱신해야 한다.

## 11.2 Aim Profile이 가져야 하는 최소 데이터

### 필수 필드

- `MinYawDeg`
- `MaxYawDeg`
- `MinPitchDeg`
- `MaxPitchDeg`

### 권장 필드

- `BaseFOV`
- `BaseArmLength`
- `AimHeightOffset`
- `AimSideOffset`
- `bAllowRearView`
- `bUseTargetFraming`
- `YawSoftLimitZoneDeg`
- `PitchSoftLimitZoneDeg`

## 11.3 예시 프로필 해석

### 전방 협각 무기

- MinYaw: -20
- MaxYaw: 20
- Pitch도 비교적 좁음
- 뒤보기 불가

### 상부 전방향 무기

- MinYaw: -170
- MaxYaw: 170 또는 그 이상
- Pitch 범위도 큼
- 뒤 조준 가능

### 측면 편향 무기

- 예: 우측 편향이면 MinYaw -10, MaxYaw 100 같은 식의 비대칭 범위 가능

---

## 12. 카메라 위치 계산 설계

## 12.1 위치 개념

카메라의 회전 중심은 차량 중심 피벗에 가깝지만, 실제 카메라 위치는 외부 3인칭 위치에 놓인다.

즉,
- 피벗은 중심
- 카메라는 반경 위의 외부 위치

구조다.

## 12.2 위치 계산 요소

최종 카메라 위치는 아래 요소를 합쳐 만든다.

1. Pivot 위치
2. AimPivot 회전
3. 기본 Arm Length
4. 높이 오프셋
5. 좌우 프레이밍 오프셋
6. 속도 기반 거리 Modifier
7. 모드 기반 Modifier
8. 충돌 보정

## 12.3 프레이밍 정책

현재 원칙은 다음과 같다.

- 회전 피벗은 차량 중심 기준
- 프레이밍은 이후 시각 테스트로 조정 가능
- 초기값은 크게 치우치지 않는 중앙형 3인칭 프레이밍 권장

즉, 1차 구현에서는 과한 숄더뷰보다 차량이 안정적으로 보이는 중앙형 구도를 우선한다.

---

## 13. 충돌 처리 설계

## 13.1 목표

- 카메라가 벽, 지형, 구조물과 겹치지 않아야 한다.
- 충돌 시 카메라가 갑자기 튀지 않아야 한다.
- 충돌 중에도 조준 감각을 최대한 유지해야 한다.
- 차량과 조준점의 가시성이 가능한 한 유지되어야 한다.

## 13.2 권장 처리 순서

1. Pivot에서 DesiredCameraLocation까지 Sweep 또는 Spring Arm 충돌 판정
2. 충돌 시 Arm Length를 줄여 안전 위치 산출
3. Arm Length 변화는 부드럽게 보간
4. 충돌 해제 시 원래 길이로 즉시 튀지 않고 천천히 복귀
5. 근접 시 차량이 화면을 너무 가리면 높이/FOV 보조 규칙 적용 가능

## 13.3 충돌 처리 원칙 상세

### 거리 우선 보정

1차 구현은 위치를 완전히 재배치하기보다 거리 축소를 우선 사용한다.

이유:
- 조준 방향 감각 보존이 쉽다.
- 예측 가능성이 높다.
- TPS 카메라 체감이 덜 깨진다.

### 회전은 가능하면 보존

충돌 때문에 카메라 위치는 바뀔 수 있지만, 플레이어가 입력한 Aim 방향 자체는 유지되도록 한다.

### 근접 시야 보정

카메라가 너무 앞으로 당겨져 차체가 화면 대부분을 가리는 경우,
다음 중 하나를 선택적으로 적용할 수 있다.

- FOV 소폭 증가
- 높이 소폭 상승
- 차체 투명화는 후순위

초기 구현에서는 FOV/높이 보정까지만 권장한다.

---

## 14. 속도감 / 반응성 설계

## 14.1 기본 원칙

속도감은 주되, 조준 안정성을 해치지 않아야 한다.

## 14.2 권장 적용 항목

### FOV 변화

- 기본 FOV: 약 80~90
- 고속 FOV: 약 90~100
- 선형 또는 곡선 보간 사용
- 급격한 변화 금지

### 거리 변화

- 속도가 높을수록 카메라를 약간 뒤로 뺀다.
- 과도한 거리 변화는 금지한다.

### 흔들림

- 기본 카메라 쉐이크는 매우 약하게 유지
- 작은 충돌마다 흔들지 않는다.
- 큰 충돌이나 착지에만 짧은 임팩트 사용 가능

## 14.3 롤 처리 정책

차량이 기울더라도 카메라가 같이 심하게 롤하지 않도록 한다.

이유:
- 전투 가독성 유지
- 화면 피로 감소
- 조준 안정성 확보

---

## 15. 조준점 계산 설계

## 15.1 기본 원칙

조준점은 단순 화면 장식이 아니라 실제 조준 상태를 반영해야 한다.

## 15.2 계산 흐름

1. 최종 카메라 Transform 확정
2. 화면 중심 또는 조준점 기준으로 월드 Ray 생성
3. 충돌 판정으로 Aim Hit 계산
4. 현재 Aim Direction이 무기 제한각 안인지 확인
5. 차량 또는 근거리 장애물에 막히는지 확인
6. HUD 상태 갱신

## 15.3 HUD 상태 분류

- `ValidAim`
- `NearYawLimit`
- `NearPitchLimit`
- `AimBlocked`
- `FireInvalid`

## 15.4 조준점 표현 권장

1차 구현에서는 조준점을 화면 중앙에 유지하고, 카메라 자체를 무기 제한각으로 Clamp하는 구조를 권장한다.

이 방식의 장점:
- 구현이 단순하다.
- 조작 이해가 쉽다.
- “카메라가 더 안 돌아가는 것 = 무기 한계”를 바로 체감할 수 있다.

후속 확장으로는 조준점 자체의 소프트 클램프 연출을 넣을 수 있다.

---

## 16. UI / 피드백 설계

## 16.1 필수 피드백

- 현재 조준 가능 여부
- 현재 사격 가능 여부
- 조준 한계 근접 여부
- 조준 가림 여부
- 차량 진행 방향 힌트

## 16.2 권장 HUD 요소

### 조준점

- 기본 상태 표시
- 한계 근접 시 변화
- 발사 불가 시 잠금 또는 색상/형태 변화

### 진행 방향 힌트

카메라가 옆이나 뒤를 볼 때도 차량이 실제로 어느 방향으로 진행 중인지 알 수 있게 한다.

예:
- 미니 전방 화살표
- 진행 벡터 힌트
- HUD 노즈 방향 표시

### 제한각 피드백

- Yaw 한계 근접 표시
- Pitch 한계 근접 표시
- 실제 발사 불가 상태 표시

---

## 17. DataAsset 설계

현재 구조상 카메라 값도 DataAsset으로 관리하는 것이 맞다.

## 17.1 권장 데이터 분리

### 차량 공통 카메라 데이터

소유 후보:
- `UCFVehicleData` 내부 Struct
- 또는 별도 `UCFVehicleCameraData`

권장 이유:
- 차량 크기와 형태에 따른 기본 카메라 값은 무기와 별도로 관리하는 편이 좋다.

필수 항목 예시:
- 기본 Arm Length
- 기본 Pivot Height Offset
- 기본 FOV
- 속도 기반 FOV Curve 또는 범위
- 속도 기반 거리 Modifier
- 위치 보간 속도
- 회전 보간 속도
- 충돌 최소 거리
- 충돌 복귀 속도

### 무기 조준 프로필 데이터

소유 후보:
- 무기 DataAsset 내부 참조
- 또는 별도 `UCFWeaponAimProfile`

필수 항목 예시:
- MinYawDeg
- MaxYawDeg
- MinPitchDeg
- MaxPitchDeg
- Combat FOV Modifier
- Combat Arm Modifier
- Framing Side Offset
- Framing Height Offset
- RearView 허용 여부

## 17.2 런타임 결합 규칙

최종 카메라 값은 다음 순서로 합성한다.

1. 차량 공통 카메라 데이터
2. 현재 모드 Modifier
3. 현재 활성 무기 Aim Profile
4. 런타임 상태 보정값(속도, 공중, 충돌, 피격 등)

---

## 18. 상태 전이 및 평가 순서

카메라 계산은 Tick 기준으로 아래 순서를 권장한다.

1. 차량 상태 수집
2. 활성 무기 그룹 및 Aim Profile 수집
3. BaseMode / Modifier 결정
4. 입력 누적
5. Aim Yaw/Pitch Clamp
6. 원하는 Pivot 회전 계산
7. 원하는 카메라 위치/FOV 계산
8. 충돌 보정
9. Aim Trace 계산
10. HUD 상태 계산
11. 최종 Camera 적용

---

## 19. 네트워크 / 판정 권장 원칙

## 19.1 카메라 자체는 로컬 처리 우선

- 카메라 Transform 자체는 로컬 클라이언트 책임으로 둔다.
- 서버에 카메라 위치를 그대로 복제할 필요는 없다.

## 19.2 사격 판정은 서버 검증 가능 구조로 둔다

- 클라이언트는 현재 Aim Direction 또는 Aim Hit를 전달할 수 있다.
- 서버는 활성 Aim Profile 기준으로 유효 각도인지 재검증할 수 있어야 한다.
- 즉, 카메라 편의와 판정 권한은 분리한다.

이 항목은 멀티플레이 확정 시 더 상세화한다.

---

## 20. 1차 구현 범위

현재 추천하는 1차 구현 범위는 다음과 같다.

### 20.1 필수 구현

1. `ACFVehiclePawn`에 카메라 피벗/붐/카메라 구성 추가
2. `UCFVehicleCameraComp` 추가
3. Look 입력 수집 및 Yaw/Pitch 누적
4. 활성 Aim Profile 기반 제한각 Clamp
5. 중심 피벗 기반 카메라 위치 계산
6. 충돌 처리
7. 속도 기반 FOV/거리 변화
8. 조준점/사격 가능 여부 HUD 상태 계산

### 20.2 선택 구현

- Camera Reset 입력
- 후진 Modifier
- 공중 Modifier
- 큰 충돌 임팩트

### 20.3 후순위 구현

- 락온 프레이밍
- 고급 타겟 유지 카메라
- 제한각 끝에서 차량 자동 보조 회전
- 시네마틱 파괴 카메라

---

## 21. C++ / Blueprint 역할 분리

## 21.1 C++가 담당할 것

- 카메라 모드 판정
- 입력 누적과 제한각 처리
- 충돌 보정
- 최종 Transform 계산
- FOV 계산
- 조준점 상태 계산
- HUD 제공용 읽기 전용 함수

## 21.2 Blueprint가 담당할 것

- Scene Component 배치 미세 조정
- 디버그 표시
- 차종별 피벗 초기 위치 튜닝
- UI Widget 연결

## 21.3 Thin BP 원칙

BP에서는 로직 계산을 최소화하고,
- 배치
- 연결
- 시각 조정
중심으로 유지한다.

---

## 22. 디버그 설계

1차 구현부터 다음 디버그 기능을 권장한다.

### 22.1 On-Screen Debug

- 현재 CameraMode
- 현재 AimProfile 이름
- Accumulated Yaw/Pitch
- Clamped Yaw/Pitch
- 현재 Arm Length
- 현재 FOV
- bAimBlocked
- bWeaponCanFireAtCurrentAim

### 22.2 월드 디버그

- Pivot 위치
- 카메라 Desired 위치
- 카메라 Solved 위치
- Aim Trace 라인
- Yaw/Pitch 제한각 시각화는 후순위

---

## 23. 구현 리스크와 대응

## 23.1 리스크: 카메라가 자유 조준인데 운전이 너무 헷갈릴 수 있음

대응:
- 진행 방향 힌트 제공
- Camera Reset 입력 제공 검토
- 지나친 오프셋 프레이밍 금지

## 23.2 리스크: 좁은 지형에서 충돌 처리 때문에 조준감이 무너질 수 있음

대응:
- 거리 축소 우선
- 회전값 보존
- 복귀 보간 사용

## 23.3 리스크: 무기 시스템이 아직 미확정이라 카메라 설계가 흔들릴 수 있음

대응:
- 무기 구체 종류가 아니라 Aim Profile 인터페이스를 먼저 정의
- 차량 공통 카메라와 무기 제한 프로필을 분리

## 23.4 리스크: 차량별 차체 크기 차이로 같은 카메라 값이 어색할 수 있음

대응:
- VehicleData 또는 CameraData에서 차종별 튜닝 허용
- Pivot 높이/Arm 길이/FOV를 DataAsset으로 관리

---

## 24. 현재 권장 구현 순서

1. `ACFVehiclePawn`에 카메라 Scene Component 뼈대 추가
2. `UCFVehicleCameraComp` 생성
3. Look 입력 액션 추가 및 CameraComp 연결
4. 중심 피벗 회전 + 기본 외부 3인칭 위치 계산 구현
5. 무기 없이도 작동하는 임시 Aim Profile 구현
6. 충돌 처리 구현
7. 속도 기반 FOV/거리 Modifier 구현
8. HUD 상태값 노출
9. 실제 무기 시스템과 Aim Profile 연동
10. 후진/공중/파괴 Modifier 추가

---

## 25. 현재 시점 결론

CarFight 차량 카메라 시스템의 1차 구현 기준은 다음과 같이 정의한다.

> 차량 카메라는 차량 중심 수평 피벗을 기준으로 자유 회전하는 차량 기반 3인칭 슈팅 카메라로 구현한다.
> 차량 이동과 카메라 회전은 분리하며, Look 입력은 차량 조향에 재사용하지 않는다.
> 카메라 회전 범위는 현재 활성 무기 그룹의 Aim Profile이 제공하는 Yaw/Pitch 제한값에 의해 Clamp된다.
> 최종 카메라 계산은 `ACFVehiclePawn`이 제공하는 기준점과 상태를 바탕으로 `UCFVehicleCameraComp`가 담당한다.
> 카메라 충돌 처리에서는 관통 방지뿐 아니라 조준 감각과 가시성 유지까지 함께 고려한다.
> 차량 공통 카메라 값과 무기별 조준 프로필은 DataAsset 계층으로 분리한다.

이 문서는 이후 실제 C++ 클래스 설계, 헤더 작성, 변수명 정의, Blueprint 구성, UI 연결 작업의 기준 문서로 사용한다.
