# VehicleDebugPanel

- Version: 1.0.0
- Date: 2026-05-07
- Status: Active
- Scope: 현재 VehicleDebug Panel의 Navigation + Selected Section 기반 구조와 Camera Debug 편입 상태 기록

---

## 1. 문서 목적

이 문서는 현재 CarFight 프로젝트에서 사용하는 최신 `VehicleDebug Panel` 구조를 기록한다.

기존 `Document/ProjectSSOT/Systems/UI/VehicleDebug.md`는 레거시 `WBP_VehicleDebug` 기준선 문서로 유지한다.
현재 패널 구조, Navigation 방식, Selected Section 표시 방식, Camera Debug 편입 상태는 이 문서를 기준으로 본다.

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

- `UCFVehicleDebugPanelWidget`
- `UCFVehicleDebugSectionWidget`
- `UCFVehicleDebugFieldRowWidget`
- `UCFVehicleDebugNavItemWidget`
- `FCFVehicleDebugPanelViewData`
- `FCFVehicleDebugSectionViewData`
- `FCFVehicleDebugFieldViewData`
- `FCFVehicleDebugNavItemViewData`

---

## 4. 현재 주요 WBP 자산

현재 패널 구조에서 중요한 Widget Blueprint는 아래와 같다.

- `WBP_VehicleDebugPanel`
- `WBP_VehicleDebugSection`
- `WBP_VehicleDebugFieldRow`
- `WBP_VehicleDebugNavItem`

현재 구조에서 특정 카테고리를 추가할 때는 루트 Panel WBP에 버튼을 수동으로 추가하지 않는다.
`TopLevelSectionArray`에 Section ViewData를 추가하면 Navigation Item이 자동 생성되는 구조를 따른다.

---

## 5. 표시 흐름

현재 표시 흐름은 아래와 같다.

```text
ACFVehiclePawn
  -> FCFVehicleDebugSnapshot
  -> UCFVehicleDebugPanelWidget::RefreshFromPawn()
  -> CachedOverview / CachedDrive / CachedInput / CachedCamera / CachedRuntime
  -> BuildVehicleDebugPanelViewData()
  -> TopLevelSectionArray
  -> RefreshNavigationItems()
  -> ResolveSelectedSectionId()
  -> RefreshSelectedSectionWidget()
```

즉 Panel은 Pawn 또는 Component를 직접 여기저기 읽지 않고, `ACFVehiclePawn`이 제공하는 VehicleDebug Snapshot 계층을 통해 카테고리별 데이터를 읽는다.

---

## 6. 현재 TopLevel Section

현재 패널의 주요 TopLevel Section은 아래와 같다.

```text
Overview
Drive
Input
Camera
Runtime
```

화면 표시 제목은 `DisplayTextPolicy`에 따라 한국어로 표시한다.

```text
Overview -> 개요
Drive -> 주행
Input -> 입력
Camera -> 카메라
Runtime -> 런타임
```

내부 `SectionId`는 영문을 유지한다.

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

- 상태 요약
- 카메라 컴포넌트
- 현재 모드
- 조준 프로필
- 조준 막힘
- 발사 가능
- 압축 비율
- 현재 FOV

### 8.2 조준 Section

- 누적 Yaw
- 누적 Pitch
- 제한 적용 Yaw
- 제한 적용 Pitch
- Yaw 제한
- Pitch 제한

### 8.3 시야 Section

- 목표 거리
- 현재 거리
- 실제 적용 거리
- 목표 FOV
- 현재 FOV
- 압축 비율

### 8.4 트레이스 Section

- 조준 막힘
- 발사 가능
- 트레이스 거리
- 명중 위치

### 8.5 모드 Section

- 현재
- 이전
- 이번 프레임 변경
- 조준 프로필

---

## 9. Camera 상태 표시 정책

Camera 탭 제목 안에는 상태 문구를 표시하지 않는다.

금지 예시:

```text
Camera Compressed
Camera Blocked
Camera Limit
```

이유:

- Navigation은 섹션 선택 역할만 맡는다.
- 상태 문구가 탭 제목 안에 들어가면 탭 목록이 복잡해진다.
- 상태 진단은 선택된 Camera Section 내부에서 보여주는 것이 읽기 쉽다.

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

## 10. 표시 언어 정책

VehicleDebug Panel은 `Document/ProjectSSOT/Systems/UI/DisplayTextPolicy.md`를 따른다.

기준:

- 내부 식별자, `SectionId`, `FieldId`, enum, Snapshot 필드는 영문 유지
- 화면에 보이는 탭 제목, 섹션 제목, 필드 라벨, 상태값은 한국어 표시
- 원본 런타임/디버그 요약 문자열은 내부 추적성을 위해 영문 키 구조를 유지할 수 있음
- 원본 요약 문자열이 Panel에 표시될 때는 표시 직전 단계에서 한국어로 변환
- 클래스명, 컴포넌트명, 에셋명, enum 원문 값은 추적을 위해 영문으로 남아 있어도 정상

예시:

```text
SectionId = Camera            // 내부 식별자, 영문 유지
TitleText = 카메라            // 화면 표시, 한국어
FieldId = camera_status_summary
LabelText = 상태 요약
ValueText = 카메라 압축
```

---

## 11. Legacy fallback

현재 최신 구조는 `Navigation + Selected Section`이 기본이다.

다만 기존 WBP 연결 상태나 중간 개발 상태에 따라 `VerticalBox_DynamicSectionHost` 기반 legacy full section rendering fallback 경로가 존재할 수 있다.

현재 기준:

- `VerticalBox_SelectedSectionHost`가 있으면 Selected Section 구조 사용
- `VerticalBox_DynamicSectionHost`는 fallback으로만 본다
- 신규 카테고리 추가 기준은 항상 `TopLevelSectionArray`와 Navigation 구조를 우선한다

---

## 12. 현재 기능 책임

현재 `VehicleDebug Panel`의 책임은 아래와 같다.

- VehicleDebug Snapshot 카테고리를 표시용 ViewData로 변환한다.
- TopLevel Section을 Navigation Item으로 표시한다.
- 선택된 Section 하나를 Selected Section 영역에 표시한다.
- 각 Section의 FieldRow와 ChildSection을 동적으로 구성한다.
- 사용자가 읽는 디버그 표시 텍스트를 한국어로 제공한다.
- 원본 디버그 요약 문자열을 Panel 표시 직전 한국어로 변환한다.

---

## 13. 현재 기준 비책임 항목

현재 `VehicleDebug Panel`의 직접 책임으로 보지 않는 항목은 아래와 같다.

- Drive 상태 계산 자체
- Input 해석 자체
- VehicleCamera 런타임 계산 자체
- VehicleRuntime 초기화/검증 로직 자체
- 무기 시스템의 실제 발사 가능 정책 결정
- 카메라 Trace 규칙 자체
- 원본 로그 문자열 형식의 전역 변경

즉 Panel은 계산기가 아니라, 이미 계산된 디버그 Snapshot을 읽기 좋게 표시하는 UI 계층이다.

---

## 14. 문서 갱신 조건

아래 변경이 생기면 이 문서를 함께 갱신한다.

- `FCFVehicleDebugPanelViewData` 구조 변경
- Navigation 생성 방식 변경
- Selected Section 표시 방식 변경
- `BuildVehicleDebugPanelViewData()` TopLevel Section 변경
- Camera Debug Section 표시 항목 변경
- 상태 요약 우선순위 변경
- DisplayTextPolicy 적용 기준 변경
- WBP 구조가 `Navigation + Selected Section`에서 다른 구조로 변경될 때

---

## 15. 관련 문서

- `Document/ProjectSSOT/Systems/UI/DisplayTextPolicy.md`
- `Document/ProjectSSOT/Systems/UI/VehicleDebug.md`
- `Document/ProjectSSOT/Systems/Vehicles/VehicleCamera.md`
- `Document/ProjectSSOT/Plan/CameraDebugPlan/CD_DebugDesign.md`
- `Document/ProjectSSOT/Plan/CameraDebugPlan/CD_VerifyGuide.md`

---

## 16. 체인지로그

### v1.0.0 - 2026-05-07
- 최신 VehicleDebug Panel 구조 문서 최초 작성
- Navigation + Selected Section 기반 구조 정리
- Camera Debug TopLevel Section 편입 상태 기록
- Camera 상태 요약 표시 정책 기록
- DisplayTextPolicy 적용 기준 기록
