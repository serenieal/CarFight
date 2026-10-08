# Task Source — CarFight Aim Commit H

- 문서 버전: v1.0
- 문서 상태: Ready
- 작업 유형: Aim Reticle / Fire Input 에셋 연결
- 기준 로드맵: `Document/ProjectSSOT/Plan/AimPlan/CF_AimRoadmap.md`
- 이전 완료 단계: R1/R2, R3, R4, R5/R6, R7, R8, R9, R10
- 이번 단계: R11 — Reticle / Fire Input Asset 연결

## Goal

CarFight 프로젝트에서 지금까지 C++로 준비한 Aim Reticle과 Fire Request 흐름을 실제 에셋에 연결한다.

이번 작업의 목표는 다음 두 가지다.

1. `UCFAimReticleWidget`을 부모로 하는 `WBP_AimReticle`을 생성하고 최소 TextBlock을 배치한다.
2. `InputAction_Fire`에 연결할 `IA_Fire`를 생성하고 `IMC_Vehicle_Default`에 Fire 입력을 추가한다.

이번 작업은 C++ 기능 추가가 아니라 에셋 연결 단계다.

## Current Baseline

현재 기준은 다음과 같다.

- `UCFAimReticleWidget` C++ 부모가 존재한다.
- `ACFVehiclePawn`은 `AimReticleWidgetClass`를 가지고 있다.
- `ACFVehiclePawn`은 로컬 Pawn에서만 Reticle Widget을 생성한다.
- `ACFVehiclePawn`은 `InputAction_Fire`를 가지고 있다.
- `InputAction_Fire`가 지정되면 `HandleFireStarted()`에 바인딩된다.
- `/Game/CarFight/UI`에는 VehicleDebug WBP들은 있으나 `WBP_AimReticle`은 아직 없다.
- `/Game/CarFight/Input`에는 여러 IA가 있으나 `IA_Fire`는 아직 없다.
- `/Game/CarFight/Input/IMC_Vehicle_Default`가 존재한다.
- `/Game/CarFight/Vehicles/BP_CFVehiclePawn`이 존재한다.
- R10 빌드 `CarFight_ReEditor / Win64 / Development`는 성공했다.

## In Scope

이번 작업에 포함한다.

1. `/Game/CarFight/UI/WBP_AimReticle` 위젯 블루프린트를 생성한다.
2. `WBP_AimReticle`의 부모 클래스를 `UCFAimReticleWidget`으로 설정한다.
3. `WBP_AimReticle`에 최소 표시용 TextBlock 3개를 배치한다.
   - `Text_ReticleState`
   - `Text_CanFire`
   - `Text_ReticleHint`
4. 세 TextBlock은 C++의 `BindWidgetOptional` 이름과 정확히 맞춘다.
5. `/Game/CarFight/Input/IA_Fire` Input Action을 생성한다.
6. `IA_Fire`는 버튼/Bool 계열 입력으로 설정한다.
7. `/Game/CarFight/Input/IMC_Vehicle_Default`에 Fire 입력 매핑을 추가한다.
8. Fire 입력 기본 키 후보를 하나 이상 지정한다.
   - Mouse Left Button
   - Gamepad Right Trigger 또는 Gamepad Face Button Right 후보
9. `/Game/CarFight/Vehicles/BP_CFVehiclePawn`의 `InputAction_Fire`에 `IA_Fire`를 지정한다.
10. `/Game/CarFight/Vehicles/BP_CFVehiclePawn`의 `AimReticleWidgetClass`에 `WBP_AimReticle`을 지정한다.
11. 저장 후 에셋 목록에 생성 여부가 확인되어야 한다.

## Out of Scope

이번 작업에서 하지 않는다.

- C++ 코드 수정 금지
- Damage 적용 금지
- Projectile 구현 금지
- Multicast Fire FX 구현 금지
- WeaponComp 생성 금지
- Reticle 이미지/머티리얼/복잡한 애니메이션 구현 금지
- Lock-On 구현 금지
- AI Combat 구현 금지
- VehicleDebug WBP 수정 금지
- 기존 주행 입력 매핑 삭제 금지
- 기존 Look/Throttle/Brake/Steering 매핑 변경 금지

## Target Assets

생성 에셋:

- `/Game/CarFight/UI/WBP_AimReticle`
- `/Game/CarFight/Input/IA_Fire`

수정 에셋:

- `/Game/CarFight/Input/IMC_Vehicle_Default`
- `/Game/CarFight/Vehicles/BP_CFVehiclePawn`

참고 에셋:

- `/Game/CarFight/UI/WBP_VehicleDebugHud`
- `/Game/CarFight/UI/WBP_VehicleDebugPanel`
- `/Game/CarFight/Input/IA_LookAround`
- `/Game/CarFight/Input/IA_VehicleMove`

## Constraints

반드시 지킬 제약은 다음과 같다.

1. C++ 파일을 수정하지 않는다.
2. 기존 Input Mapping Context의 기존 매핑을 삭제하지 않는다.
3. 기존 VehicleDebug UI 에셋을 수정하지 않는다.
4. `WBP_AimReticle`의 부모 클래스는 반드시 `UCFAimReticleWidget`이어야 한다.
5. TextBlock 이름은 C++ BindWidgetOptional 이름과 정확히 일치해야 한다.
6. `BP_CFVehiclePawn.AimReticleWidgetClass`에는 `WBP_AimReticle`을 지정한다.
7. `BP_CFVehiclePawn.InputAction_Fire`에는 `IA_Fire`를 지정한다.
8. `IA_Fire`는 버튼 입력으로 동작해야 한다.
9. Fire 입력은 최소 Mouse Left Button에 매핑한다.
10. 가능하면 Gamepad Right Trigger도 함께 매핑한다.
11. 저장 가능한 모든 변경 에셋을 저장한다.
12. 에디터/PIE가 크래시하지 않아야 한다.

## Recommended WBP Layout

`WBP_AimReticle`은 최소 구조로 만든다.

권장 구조:

- Root Canvas Panel
- Center Anchor 기준 Vertical Box 또는 Overlay
- 중앙에 `Text_ReticleState`
- 그 아래 `Text_CanFire`
- 그 아래 `Text_ReticleHint`

권장 초기 텍스트:

- `Text_ReticleState`: `Reticle: Hidden`
- `Text_CanFire`: `CanFire: false`
- `Text_ReticleHint`: `Aim Reticle`

권장 표시 기준:

- 폰트 크기는 너무 크지 않게 16~24 후보
- 중앙 화면에 보이도록 배치
- 색상/애니메이션은 이번 단계에서 중요하지 않음

## Recommended Fire Input Mapping

`IMC_Vehicle_Default`에 다음 매핑을 추가한다.

- Action: `IA_Fire`
- Key: `Left Mouse Button`

가능하면 추가 후보:

- Action: `IA_Fire`
- Key: `Gamepad Right Trigger`

기존 매핑은 유지한다.

## Acceptance Criteria

작업 완료 조건은 다음과 같다.

1. `/Game/CarFight/UI/WBP_AimReticle`이 생성되어 있다.
2. `WBP_AimReticle` 부모 클래스가 `UCFAimReticleWidget`이다.
3. `WBP_AimReticle`에 `Text_ReticleState`, `Text_CanFire`, `Text_ReticleHint`가 존재한다.
4. `/Game/CarFight/Input/IA_Fire`가 생성되어 있다.
5. `IA_Fire`가 버튼/Bool 입력으로 설정되어 있다.
6. `IMC_Vehicle_Default`에 `IA_Fire` 매핑이 추가되어 있다.
7. 기존 입력 매핑이 삭제되지 않았다.
8. `BP_CFVehiclePawn.InputAction_Fire`가 `IA_Fire`를 참조한다.
9. `BP_CFVehiclePawn.AimReticleWidgetClass`가 `WBP_AimReticle`을 참조한다.
10. 에셋 저장이 완료되어 있다.
11. C++ 파일 변경이 없다.

## Verification

검증 기준은 다음과 같다.

1. 에셋 목록에서 `WBP_AimReticle`이 확인되어야 한다.
2. 에셋 목록에서 `IA_Fire`가 확인되어야 한다.
3. `BP_CFVehiclePawn` Details에서 `AimReticleWidgetClass`가 지정되어 있어야 한다.
4. `BP_CFVehiclePawn` Details에서 `InputAction_Fire`가 지정되어 있어야 한다.
5. Single Player PIE에서 로컬 Pawn이 생성될 때 Reticle Widget이 화면에 나타나야 한다.
6. Look 입력에 따라 Reticle 상태 텍스트가 갱신될 수 있어야 한다.
7. Fire 입력을 눌렀을 때 크래시하지 않아야 한다.
8. VehicleDebug Aim에서 Last Fire Request / Server State가 갱신될 수 있어야 한다.
9. 기존 차량 이동/카메라 입력이 유지되어야 한다.

## Notes

이 단계는 에셋 연결 단계다.

작업 후 다음 단계는 `R12 — PIE 검증 / 버그 수정 지시서`로 진행한다.

R12에서는 Single Player, Listen Server + 1 Client, Listen Server + 2 Clients 검증 결과를 바탕으로 C++ 또는 에셋 보정 지시서를 만든다.

## Unresolved

없음.
