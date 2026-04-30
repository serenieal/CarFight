# CameraDebugPlan

- Version: 0.1.0
- Date: 2026-04-30
- Status: Draft
- Scope: 현재 VehicleDebug Panel 구조에 Camera 디버그 카테고리를 정식 편입하기 위한 문서 묶음 안내

---

## 1. 문서 묶음 목적

이 폴더는 CarFight의 차량 카메라 디버그 정보를 기존 VehicleDebug 시스템에 맞춰 편입하기 위한 설계와 작업 지침을 모아 둔다.

현재 VehicleDebug Panel은 예전처럼 모든 최상위 Section을 한 ScrollBox에 세로로 나열하는 구조가 아니다.
현재 기준은 아래 구조다.

```text
ACFVehiclePawn
  -> FCFVehicleDebugSnapshot
  -> FCFVehicleDebugPanelViewData
  -> FCFVehicleDebugSectionViewData
  -> Navigation Item
  -> Selected Section Content
```

따라서 Camera 디버그도 별도 HUD나 별도 WBP를 새로 만드는 방식이 아니라,
기존 Snapshot / PanelViewData / Navigation / SelectedSection 구조에 새 카테고리로 들어가야 한다.

---

## 2. 현재 기준 요약

### 현재 VehicleDebug Panel 운영 구조

- `UCFVehicleDebugPanelWidget`이 Pawn에서 디버그 카테고리를 읽는다.
- `BuildVehicleDebugPanelViewData()`가 최상위 Section 배열을 만든다.
- `VerticalBox_NavHost`가 최상위 Section의 Navigation 항목을 표시한다.
- `SelectedSectionId`가 현재 선택된 최상위 Section을 가리킨다.
- `VerticalBox_SelectedSectionHost`가 선택된 Section 하나만 표시한다.
- `VerticalBox_DynamicSectionHost`는 선택 Section 구조가 준비되지 않았을 때 사용하는 legacy fallback이다.

### 현재 Camera 쪽 준비 상태

- `UCFVehicleCameraComp`에 `GetCameraRuntimeState()`가 있다.
- `FCFVehicleCameraRuntimeState` 안에 Aim, ArmLength, FOV, Trace 상태가 이미 들어 있다.
- `ACFVehiclePawn`은 `VehicleCameraComp`를 보유하고 있고 `GetVehicleCameraComp()`를 제공한다.

---

## 3. 이 문서 묶음의 결론

Camera 디버그 편입의 정답 흐름은 아래다.

```text
UCFVehicleCameraComp
  -> FCFVehicleCameraRuntimeState
  -> FCFVehicleDebugCamera
  -> FCFVehicleDebugSnapshot.Camera
  -> ACFVehiclePawn::GetVehicleDebugCamera()
  -> UCFVehicleDebugPanelWidget::CachedCamera
  -> BuildCameraSectionViewData()
  -> Camera Nav Item
  -> Selected Camera Section
```

즉, Camera 디버그는 `VehicleDebug Panel`의 새 최상위 카테고리로 들어간다.

---

## 4. 문서 목록

### 4.1 `CD_DebugDesign.md`

Camera 디버그 카테고리의 구조 설계 문서다.

주요 내용:
- 현재 VehicleDebug 구조 재확인
- Camera 카테고리 책임
- Snapshot / PanelViewData / Navigation 연결 방식
- Camera Section과 하위 Section 구성
- HUD로 승격할 수 있는 후보값

### 4.2 `CD_FileTasks.md`

파일 단위 작업 지시서다.

주요 내용:
- `CFVehiclePawn.h` 작업
- `CFVehiclePawn.cpp` 작업
- `CFVehicleDebugPanelWidget.h` 작업
- `CFVehicleDebugPanelWidget.cpp` 작업
- C++ / BP 책임 분리
- 실제 구현 순서

### 4.3 `CD_VerifyGuide.md`

검증 가이드다.

주요 내용:
- 빌드 검증
- PIE 검증
- Navigation 검증
- Selected Section 검증
- Camera 값 검증
- 실패 시 확인 순서

### 4.4 `CD_Checklist.md`

작업 체크리스트다.

주요 내용:
- 데이터 계층 완료 기준
- Panel 계층 완료 기준
- UI/BP 수동 확인 항목
- 문서 반영 항목

---

## 5. 추천 읽기 순서

1. `README.md`
2. `CD_DebugDesign.md`
3. `CD_FileTasks.md`
4. `CD_VerifyGuide.md`
5. `CD_Checklist.md`

---

## 6. 구현 원칙

1. 새 Camera WBP를 만들지 않는다.
2. 루트 Panel WBP에 Camera 버튼을 수동 추가하지 않는다.
3. Camera 값 문자열을 BP에서 직접 조립하지 않는다.
4. `Camera`는 `PanelViewData.TopLevelSectionArray`에 들어가는 새 Section이어야 한다.
5. Navigation 항목은 `SectionViewData`의 메타데이터에서 자동 생성되어야 한다.
6. 선택된 Camera Section은 `VerticalBox_SelectedSectionHost`에 표시되어야 한다.
7. `VerticalBox_DynamicSectionHost`는 fallback으로만 본다.
8. HUD는 1차 대상이 아니며, Panel 검증 후 Overview에 소수 핵심값만 승격한다.

---

## 7. Changelog

### v0.1.0 - 2026-04-30
- 현재 Navigation + Selected Section 기반 VehicleDebug Panel 구조에 맞춘 CameraDebugPlan 문서 묶음을 신규 작성했다.
- Camera 디버그를 별도 HUD가 아니라 VehicleDebug Panel의 새 최상위 카테고리로 편입하는 방향을 고정했다.
