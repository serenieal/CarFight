# CarFight — CF_AimEditorChecklist

> 역할: Aim 시스템 1차 구현 이후 UE Editor에서 수동으로 연결해야 하는 에셋과 설정을 기록한다.  
> 문서 버전: v1.0.0  
> 마지막 정리(Asia/Seoul): 2026-05-21  
> 상태: Completed / Manual Editor Setup

---

## 1. 문서 목적

이 문서는 Codex가 수행한 C++ 작업 이후, UE Editor에서 사람이 직접 연결해야 하는 작업을 기록한다.

중요한 구분은 다음과 같다.

```text
Codex 작업 대상:
  C++ 타입 / 컴포넌트 / RPC / Debug / Widget 부모 클래스

UE Editor 수동 작업 대상:
  WBP 생성
  InputAction 에셋 생성
  InputMappingContext 수정
  BP_CFVehiclePawn Details 값 지정
  PIE 실행 확인
```

따라서 이 문서는 Codex 작업지시서가 아니라, 에디터 수동 연결 체크리스트다.

---

## 2. 현재 수동 연결 완료 상태

사용자 확인 기준으로 다음 항목은 완료되었다.

```text
WBP_AimReticle 생성 완료
IA_Fire 생성 완료
IMC_Vehicle_Default에 IA_Fire 매핑 완료
BP_CFVehiclePawn에 InputAction_Fire 연결 완료
BP_CFVehiclePawn에 AimReticleWidgetClass 연결 완료
Single Player PIE에서 Reticle 표시 확인 완료
Fire 입력 확인 완료
```

MCP 확인 기준으로 다음 항목도 확인되었다.

```text
/Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle 존재 확인
WBP_AimReticle class_name = WidgetBlueprint 확인
WBP_AimReticle resolved_class 확인
Text Reticle State / Text Can Fire / Text Reticle Hint 관련 바인딩 후보 확인
```

---

## 3. WBP_AimReticle 연결 체크리스트

대상 경로:

```text
/Game/CarFight/UI/WBP_AimReticle
```

체크리스트:

- [x] `WBP_AimReticle` 생성
- [x] Parent Class를 `UCFAimReticleWidget` 또는 에디터 표시명 `CFAimReticleWidget`으로 설정
- [x] `Text_ReticleState` TextBlock 배치
- [x] `Text_CanFire` TextBlock 배치
- [x] `Text_ReticleHint` TextBlock 배치
- [x] 세 TextBlock 이름이 C++ `BindWidgetOptional` 이름과 일치
- [x] Reticle Widget이 화면에서 보임
- [x] Reticle Widget이 입력을 가로막지 않도록 설정 확인

### 3.1 Visibility 명칭 정정

UE Editor의 Visibility 드롭다운에는 예전 명칭인 `Hit Test Invisible`이 그대로 표시되지 않을 수 있다.

현재 에디터에서 보이는 올바른 선택지는 다음이다.

```text
Not Hit-Testable (Self & All Children)
Not Hit-Testable (Self Only)
```

Reticle UI에는 다음 값이 적합하다.

```text
Visibility = Not Hit-Testable (Self & All Children)
```

이 값은 Reticle과 그 자식 위젯이 마우스 입력 판정에 참여하지 않게 하므로, 조준 / 발사 입력을 막지 않는다.

---

## 4. IA_Fire 연결 체크리스트

대상 경로:

```text
/Game/CarFight/Input/IA_Fire
```

체크리스트:

- [x] `IA_Fire` 생성
- [x] 입력 타입을 버튼 / Boolean 계열로 설정
- [x] `IMC_Vehicle_Default`에 `IA_Fire` 매핑 추가
- [x] `Left Mouse Button` 매핑 추가
- [ ] Gamepad Fire 입력 추가 여부는 후속 확인 대상
- [x] 기존 차량 이동 / 조향 / Look 입력 매핑을 삭제하지 않음

---

## 5. BP_CFVehiclePawn 연결 체크리스트

대상 경로:

```text
/Game/CarFight/Vehicles/BP_CFVehiclePawn
```

체크리스트:

- [x] `InputAction_Fire`에 `/Game/CarFight/Input/IA_Fire` 지정
- [x] `AimReticleWidgetClass`에 `/Game/CarFight/UI/WBP_AimReticle` 지정
- [x] `bShowAimReticle` 활성 상태 확인
- [x] Compile 수행
- [x] Save 수행

---

## 6. PIE 수동 확인 체크리스트

### 6.1 Single Player PIE

- [x] PIE 시작 시 크래시 없음
- [x] Reticle Widget 표시 확인
- [x] Reticle State 텍스트 표시 확인
- [x] CanFire 텍스트 표시 확인
- [x] 차량 이동 유지 확인
- [x] 카메라 Look 입력 유지 확인
- [x] Fire 입력 시 크래시 없음
- [x] Fire 입력 흐름 확인

### 6.2 VehicleDebug 확인

- [x] Aim 카테고리 존재
- [x] Local Aim State 확인 가능
- [x] Server Aim State 확인 가능
- [x] Rep Aim Visual State 확인 가능
- [x] Fire 입력 후 관련 값 갱신 확인 가능

### 6.3 Multiplayer 확인

아래 항목은 아직 완료 기록 없음.

- [ ] Listen Server + 1 Client에서 Fire Request 확인
- [ ] Listen Server + 1 Client에서 Server RejectReason 확인
- [ ] Listen Server + 2 Clients에서 RepAimVisual 확인
- [ ] Dedicated Server 후보 검증

---

## 7. 현재 문서 판정

Aim 시스템 1차 구현과 에디터 수동 연결은 완료 상태로 본다.

다만 멀티플레이 검증은 별도 검증 단계로 남긴다.

```text
완료:
  C++ Aim Core
  Local Aim
  VehicleDebug Aim
  Fire Request
  Server Validation
  Server Dummy HitScan
  RepAimVisual 기반
  Reticle C++ 부모
  Reticle Pawn 연결
  WBP_AimReticle 수동 연결
  IA_Fire 수동 연결
  Single Player Fire 확인

남음:
  Listen Server 검증
  RepAimVisual 원격 표시 검증
  Fire FX Multicast
  Damage / WeaponComp 후속 설계
```
