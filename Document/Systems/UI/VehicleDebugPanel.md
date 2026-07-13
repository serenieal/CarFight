# VehicleDebugPanel

- Version: 1.1.0
- Date: 2026-07-09
- Status: Active
- Scope: 현재 VehicleDebug Panel의 Navigation + Selected Section 기반 구조, Camera / Aim / Weapon Debug 편입 상태, WeaponFire / FireFeedback 참조 기준 기록

---

## 1. 문서 목적

이 문서는 현재 CarFight 프로젝트에서 사용하는 최신 `VehicleDebug Panel` 구조를 기록한다.

기존 `Document/Systems/UI/VehicleDebug.md`는 레거시 `WBP_VehicleDebug` 기준선 문서로 유지한다.
현재 패널 구조, Navigation 방식, Selected Section 표시 방식, Camera / Aim / Weapon Debug 편입 상태는 이 문서를 기준으로 본다.

현재 CarFight의 전투 구현 기준은 **싱글플레이 로컬 차량 전투**다.
따라서 이 문서의 Weapon Debug는 서버 판정 UI가 아니라, 로컬 `WeaponFire` / `Projectile` / `DamageHitContext` / `FireFeedback` 작업을 검증하기 위한 상태 표시 섹션으로 본다.

---

## 2. 현재 패널 구조 요약

현재 `VehicleDebug Panel`은 모든 디버그 섹션을 한 화면에 세로로 전부 나열하는 구조가 아니다.
현재 기본 구조는 아래와 같다.

```text
FCFVehicleDebugPanelViewData
  -> TopLevelSectionArray
  -> Navigation Item 자동 생성
  -> SelectedSectionId 선택
  -> SelectedSectionHost에 선택된 Section 하나 표시
```

즉 왼쪽 Navigation에서 카테고리를 선택하고, 오른쪽 Selected Section 영역에 선택된 카테고리 하나만 표시하는 방식이다.

---

## 3. 현재 주요 클래스

현재 패널 구조에서 중요한 C++ 클래스는 아래와 같다.

```text
UCFVehicleDebugPanelWidget
UCFVehicleDebugSectionWidget
UCFVehicleDebugFieldRowWidget
UCFVehicleDebugNavItemWidget
FCFVehicleDebugPanelViewData
FCFVehicleDebugSectionViewData
FCFVehicleDebugFieldViewData
FCFVehicleDebugNavItemViewData
```

---

## 4. 현재 주요 WBP 자산

현재 패널 구조에서 중요한 Widget Blueprint는 아래와 같다.

```text
WBP_VehicleDebugPanel
WBP_VehicleDebugSection
WBP_VehicleDebugFieldRow
WBP_VehicleDebugNavItem
```

현재 구조에서 특정 카테고리를 추가할 때는 루트 Panel WBP에 버튼을 수동으로 추가하지 않는다.
`TopLevelSectionArray`에 Section ViewData를 추가하면 Navigation Item이 자동 생성되는 구조를 따른다.

---

## 5. 표시 흐름

현재 표시 흐름은 아래와 같다.

```text
ACFVehiclePawn
  -> FCFVehicleDebugSnapshot
  -> UCFVehicleDebugPanelWidget::RefreshFromPawn()
  -> CachedOverview / CachedDrive / CachedInput / CachedCamera / CachedAim / CachedWeapon / CachedRuntime
  -> BuildVehicleDebugPanelViewData()
  -> TopLevelSectionArray
  -> RefreshNavigationItems()
  -> ResolveSelectedSectionId()
  -> RefreshSelectedSectionWidget()
```

즉 Panel은 Pawn 또는 Component를 직접 여기저기 읽지 않고, `ACFVehiclePawn`이 제공하는 VehicleDebug Snapshot 계층을 통해 카테고리별 데이터를 읽는다.

---

## 6. 현재 TopLevel Section

현재 코드 기준 `BuildVehicleDebugPanelViewData()`는 아래 TopLevel Section을 추가한다.

```text
Overview
Drive
Input
Camera
Aim
Weapon
Runtime
```

화면 표시 제목은 `DisplayTextPolicy`에 따라 한국어로 표시한다.

```text
Overview -> 개요
Drive    -> 주행
Input    -> 입력
Camera   -> 카메라
Aim      -> 조준
Weapon   -> 무기
Runtime  -> 런타임
```

내부 `SectionId`는 영문을 유지한다.

현재 코드 기준 TopLevel Section 생성 흐름:

```text
BuildVehicleDebugPanelViewData()
  -> BuildOverviewSectionViewData(CachedOverview)
  -> BuildDriveSectionViewData(CachedDrive)
  -> BuildInputSectionViewData(CachedInput)
  -> BuildCameraSectionViewData(CachedCamera)
  -> BuildAimSectionViewData(CachedAim)
  -> BuildWeaponSectionViewData(CachedWeapon)
  -> BuildRuntimeSectionViewData(CachedRuntime)
```

---

## 7. Camera Debug 편입 상태

Camera Debug는 현재 `VehicleDebug Panel`의 정식 TopLevel Section이다.

Camera Debug 연결 흐름은 아래와 같다.

```text
UCFVehicleCameraComp
  -> FCFVehicleCameraRuntimeState
  -> FCFVehicleDebugCamera
  -> FCFVehicleDebugSnapshot.Camera
  -> ACFVehiclePawn::GetVehicleDebugCamera()
  -> UCFVehicleDebugPanelWidget::CachedCamera
  -> BuildCameraSectionViewData()
  -> Camera Navigation Item
  -> Selected Camera Section
```

Camera 탭은 WBP에 수동으로 추가한 버튼이 아니다.
`BuildVehicleDebugPanelViewData()`에서 `BuildCameraSectionViewData(CachedCamera)`를 TopLevel Section으로 추가하면 Navigation Item이 자동 생성된다.

---

## 8. Camera Section 표시 항목

Camera Section은 상위 요약 필드와 하위 Section으로 구성된다.

### 8.1 상위 요약 필드

```text
- 상태 요약
- 카메라 컴포넌트
- 현재 모드
- 조준 프로필
- 조준 막힘
- 발사 가능
- 압축 비율
- 현재 FOV
```

### 8.2 조준 Section

```text
- 누적 Yaw
- 누적 Pitch
- 제한 적용 Yaw
- 제한 적용 Pitch
- Yaw 제한
- Pitch 제한
```

### 8.3 시야 Section

```text
- 목표 거리
- 현재 거리
- 실제 적용 거리
- 목표 FOV
- 현재 FOV
- 압축 비율
```

### 8.4 트레이스 Section

```text
- 조준 막힘
- 발사 가능
- 트레이스 거리
- 명중 위치
```

### 8.5 모드 Section

```text
- 현재
- 이전
- 이번 프레임 변경
- 조준 프로필
```

---

## 9. Aim Debug 편입 상태

Aim Debug는 현재 `VehicleDebug Panel`의 정식 TopLevel Section이다.

Aim Debug는 `VehicleAim`이 제공하는 로컬 조준 상태, 로컬 발사 검증 상태, 로컬 시각화 상태를 표시하기 위한 섹션이다.
상세 의미 기준은 `Document/Systems/Vehicles/VehicleAim.md`를 우선 참조한다.

현재 Aim Debug에서 중요하게 봐야 할 기준은 아래와 같다.

```text
- Local Aim 상태는 즉시 표시용 예측 상태다.
- FireValidationState는 실제 Fire Command 검증 결과 기록 상태다.
- AimVisualState는 UI / Debug / 후속 이펙트가 읽는 로컬 시각화 상태다.
- bLocalWithinWeaponArc와 OutOfArc는 현재 P0 싱글플레이 기준에서 단독 발사 차단 조건이 아니라 표시/디버그 상태다.
```

---

## 10. Weapon Debug 편입 상태

Weapon Debug는 현재 `VehicleDebug Panel`의 정식 TopLevel Section이다.

Weapon Debug 연결 흐름은 아래와 같다.

```text
UCFVehicleWeaponComp / ACFVehiclePawn WeaponFire 흐름
  -> FCFVehicleDebugWeapon
  -> FCFVehicleDebugSnapshot.Weapon
  -> UCFVehicleDebugPanelWidget::CachedWeapon
  -> BuildWeaponSectionViewData(CachedWeapon)
  -> Weapon Navigation Item
  -> Selected Weapon Section
```

Weapon 탭은 WBP에 수동으로 추가한 버튼이 아니다.
`BuildVehicleDebugPanelViewData()`에서 `BuildWeaponSectionViewData(CachedWeapon)`를 TopLevel Section으로 추가하면 Navigation Item이 자동 생성된다.

Weapon Debug는 발사 정책을 결정하는 계산기가 아니다.
Weapon Debug는 이미 계산되거나 기록된 `WeaponFire`, `VehicleWeaponComp`, `Projectile`, `DamageHitContext` 상태를 화면에서 확인하기 위한 UI 계층이다.

---

## 11. Weapon Section 표시 항목

Weapon Section은 상위 요약 필드와 하위 Section으로 구성된다.
실제 상세 필드의 의미 기준은 `Document/Systems/Combat/WeaponFire.md`를 우선 참조한다.

### 11.1 상위 요약 필드

현재 Weapon Section의 상위 요약 필드는 아래 성격의 정보를 표시한다.

```text
- 상태 요약
- Weapon 컴포넌트 존재 여부
- Weapon 런타임 준비 여부
- 활성 MountProfile
- 장비 프리셋 ID
- 무기 데이터 ID
- 무기 데이터 호환 여부
- 런타임 요약
```

상태 요약은 대략 아래 우선순위로 결정된다.

```text
WeaponComp 없음
런타임 미준비
WeaponData 미지정
WeaponData 호환 불가
쿨다운
Projectile Pool 준비됨
FireOrigin 해결
FireOrigin 대기
```

Navigation 배지는 아래와 같은 짧은 상태를 표시할 수 있다.

```text
없음
미준비
데이터없음
불일치
쿨다운
발사체준비
해결
대기
```

### 11.2 EquipmentPresetData Section

```text
- 지정 여부
- 프리셋 ID
- 장착 호환
- 요약
```

### 11.3 TurretVisual Section

```text
- 마운트 데이터
- 마운트 ID
- 장착 여부
- Base 메쉬
- Yaw 메쉬
- Pitch 메쉬
- 마운트 요약
- 터렛 시각 요약
```

### 11.4 TurretAim Section

```text
- 현재 Yaw
- 목표 Yaw
- 현재 Pitch
- 목표 Pitch
- 안정화
- 터렛 Aim 요약
```

### 11.5 WeaponData Section

```text
- 지정 여부
- 무기 ID
- 장착 호환
- Trace 사거리
- 분당 발사속도
- 남은 쿨다운
- 마지막 승인 발사 시간
- 요약
```

이 섹션의 `남은 쿨다운`, `마지막 승인 발사 시간`은 Reticle / FireFeedback 구현 전에 쿨다운 상태를 확인하는 기준으로 유용하다.
다만 표시 UI의 최종 기준은 `Document/Systems/Combat/FireFeedback.md`에서 정한다.

### 11.6 ProjectileData Section

```text
- 지정 여부
- 발사체 ID
- 스폰 준비
- Fallback
- 전환 요약
- 요약
```

### 11.7 DamageData Section

```text
- 지정 여부
- 피해 ID
- 해석 경로
- 요약
```

### 11.8 DamageHitContext Section

```text
- 기록 여부
- Source
- Hit
- 피해 ID
- 무기 ID
- 발사체 ID
- 피격 Actor
- 발사 주체
- 피격 위치
- 피격 노멀
- 입사 방향
- 비행 시간
- 요약
```

### 11.9 ProjectilePool Section

```text
- Pool 컴포넌트
- 전체 Pool 수
- 활성 Pool 수
- 비활성 Pool 수
- 마지막 반환 요약
```

### 11.10 WeaponFireOrigin Section

```text
- 해결 여부
- 계산 프로파일
- 위치 슬롯
- 장착 타입
- 월드 위치
- 월드 방향
```

---

## 12. Weapon Debug 참조 기준

Weapon Debug의 상세 필드 의미는 아래 문서를 우선 참조한다.

```text
- Document/Systems/Combat/WeaponFire.md
- Document/Systems/Combat/Projectile.md
- Document/Systems/Combat/DamageHitContext.md
- Document/Systems/Combat/FireFeedback.md
- Document/Systems/Vehicles/VehicleAim.md
```

참조 우선순위:

```text
WeaponFire 판정/기록 기준      -> Combat/WeaponFire.md
발사 성공/실패/쿨다운 표시 기준 -> Combat/FireFeedback.md
Projectile Actor / Pool 기준    -> Combat/Projectile.md
DamageHitContext 기록 기준      -> Combat/DamageHitContext.md
조준각 / OutOfArc 기준          -> Vehicles/VehicleAim.md
```

Weapon Debug는 위 문서들의 상태를 보여주는 표시 UI다.
따라서 Debug Panel의 표시가 전투 판정 기준을 새로 정의하지 않는다.

---

## 13. Camera 상태 표시 정책

Camera 탭 제목 안에는 상태 문구를 표시하지 않는다.

금지 예시:

```text
Camera Compressed
Camera Blocked
Camera Limit
```

이유:

```text
- Navigation은 섹션 선택 역할만 맡는다.
- 상태 문구가 탭 제목 안에 들어가면 탭 목록이 복잡해진다.
- 상태 진단은 선택된 Camera Section 내부에서 보여주는 것이 읽기 쉽다.
```

현재 정책:

```text
Camera 탭: 카메라
Camera Section 내부 필드: 상태 요약
```

상태 요약 우선순위:

```text
조준 막힘
카메라 압축
조준 제한
정상
```

---

## 14. 표시 언어 정책

VehicleDebug Panel은 `Document/Systems/UI/DisplayTextPolicy.md`를 따른다.

기준:

```text
- 내부 식별자, SectionId, FieldId, enum, Snapshot 필드는 영문 유지
- 화면에 보이는 탭 제목, 섹션 제목, 필드 라벨, 상태값은 한국어 표시
- 원본 런타임/디버그 요약 문자열은 내부 추적성을 위해 영문 키 구조를 유지할 수 있음
- 원본 요약 문자열이 Panel에 표시될 때는 표시 직전 단계에서 한국어로 변환
- 클래스명, 컴포넌트명, 에셋명, enum 원문 값은 추적을 위해 영문으로 남아 있어도 정상
```

예시:

```text
SectionId = Camera            // 내부 식별자, 영문 유지
TitleText = 카메라            // 화면 표시, 한국어
FieldId = camera_status_summary
LabelText = 상태 요약
ValueText = 카메라 압축
```

---

## 15. Legacy fallback

현재 최신 구조는 `Navigation + Selected Section`이 기본이다.

다만 기존 WBP 연결 상태나 중간 개발 상태에 따라 `VerticalBox_DynamicSectionHost` 기반 legacy full section rendering fallback 경로가 존재할 수 있다.

현재 기준:

```text
- VerticalBox_SelectedSectionHost가 있으면 Selected Section 구조 사용
- VerticalBox_DynamicSectionHost는 fallback으로만 본다
- 신규 카테고리 추가 기준은 항상 TopLevelSectionArray와 Navigation 구조를 우선한다
```

---

## 16. 현재 기능 책임

현재 `VehicleDebug Panel`의 책임은 아래와 같다.

```text
- VehicleDebug Snapshot 카테고리를 표시용 ViewData로 변환한다.
- TopLevel Section을 Navigation Item으로 표시한다.
- 선택된 Section 하나를 Selected Section 영역에 표시한다.
- 각 Section의 FieldRow와 ChildSection을 동적으로 구성한다.
- 사용자가 읽는 디버그 표시 텍스트를 한국어로 제공한다.
- 원본 디버그 요약 문자열을 Panel 표시 직전 한국어로 변환한다.
- WeaponFire / Projectile / DamageHitContext / FireFeedback 검증에 필요한 현재 상태를 표시한다.
```

---

## 17. 현재 기준 비책임 항목

현재 `VehicleDebug Panel`의 직접 책임으로 보지 않는 항목은 아래와 같다.

```text
- Drive 상태 계산 자체
- Input 해석 자체
- VehicleCamera 런타임 계산 자체
- VehicleAim 런타임 계산 자체
- VehicleRuntime 초기화/검증 로직 자체
- WeaponFire 실제 발사 가능 정책 결정
- Projectile Actor 생성/반환 로직 자체
- DamageHitContext 생성 로직 자체
- FireFeedback 표시 정책 결정 자체
- 카메라 Trace 규칙 자체
- 원본 로그 문자열 형식의 전역 변경
```

즉 Panel은 계산기가 아니라, 이미 계산된 디버그 Snapshot을 읽기 좋게 표시하는 UI 계층이다.

---

## 18. 문서 갱신 조건

아래 변경이 생기면 이 문서를 함께 갱신한다.

```text
- FCFVehicleDebugPanelViewData 구조 변경
- Navigation 생성 방식 변경
- Selected Section 표시 방식 변경
- BuildVehicleDebugPanelViewData() TopLevel Section 변경
- Camera Debug Section 표시 항목 변경
- Aim Debug Section 표시 항목 변경
- Weapon Debug Section 표시 항목 변경
- Weapon Debug 상태 요약 우선순위 변경
- 상태 요약 우선순위 변경
- DisplayTextPolicy 적용 기준 변경
- WBP 구조가 Navigation + Selected Section에서 다른 구조로 변경될 때
```

---

## 19. 관련 문서

```text
- Document/Systems/UI/DisplayTextPolicy.md
- Document/Systems/UI/VehicleDebug.md
- Document/Systems/UI/AimReticle.md
- Document/Systems/Vehicles/VehicleCamera.md
- Document/Systems/Vehicles/VehicleAim.md
- Document/Systems/Combat/WeaponFire.md
- Document/Systems/Combat/FireFeedback.md
- Document/Systems/Combat/Projectile.md
- Document/Systems/Combat/DamageHitContext.md
- Document/Plan/CameraDebugPlan/CD_DebugDesign.md
- Document/Plan/CameraDebugPlan/CD_VerifyGuide.md
```

---

## 20. 문서 버전 관리

- 현재 문서 버전: `1.1.0`
- 문서 상태: `Active`
- 관리 원칙:
  - 이 문서는 한 번 작성하고 끝내는 문서가 아니라, 기능의 현재 상태가 바뀌면 함께 갱신한다.
  - 기능 설명 본문이 바뀌면 체인지로그도 같이 갱신한다.
  - 구현 변경 없이 표현만 다듬은 경우와, 기능 이해에 영향을 주는 내용 변경을 구분해서 기록한다.

---

## 21. Migration

### v1.0.0 -> v1.1.0

```text
- TopLevel Section 목록에 Aim / Weapon을 현재 코드 기준으로 추가한다.
- Weapon Section은 실제 코드에 존재하는 BuildWeaponSectionViewData(CachedWeapon) 기준으로 설명한다.
- Weapon Debug 상세 필드 의미는 WeaponFire.md를 우선 참조한다.
- 발사 피드백 표시 기준은 FireFeedback.md를 우선 참조한다.
- Debug Panel은 전투 판정 계산기가 아니라 이미 계산된 Snapshot 표시 UI로 해석한다.
```

---

## 22. Changelog

### v1.1.0 - 2026-07-09

```text
- 현재 코드 기준 TopLevel Section 목록에 Aim / Weapon 추가
- Weapon Debug 편입 상태와 연결 흐름 추가
- Weapon Section 상위 필드와 하위 Section 목록 정리
- Weapon Debug 상세 기준은 WeaponFire / Projectile / DamageHitContext / FireFeedback 문서를 우선 참조한다고 명시
- VehicleDebug Panel의 비책임 항목에 WeaponFire 정책 결정, Projectile 생성, DamageHitContext 생성, FireFeedback 정책 결정을 추가
- 관련 문서에 AimReticle, VehicleAim, WeaponFire, FireFeedback, Projectile, DamageHitContext 추가
```

### v1.0.0 - 2026-05-07

```text
- 최신 VehicleDebug Panel 구조 문서 최초 작성
- Navigation + Selected Section 기반 구조 정리
- Camera Debug TopLevel Section 편입 상태 기록
- Camera 상태 요약 표시 정책 기록
- DisplayTextPolicy 적용 기준 기록
```

---

## 23. 변경 이력

### 2026-06-19 - 링크 경로 정정

```text
- 이전 ProjectSSOT Plan/Systems 참조를 현재 Document/Plan 및 Document/Systems 경로로 정정했다.
- 구버전 ProjectSSOT 파일명 참조를 현재 00_Vision ~ 05_TestChecklist 기준 또는 Archive 경로로 정정했다.
```
