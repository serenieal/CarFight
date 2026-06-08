# DisplayTextPolicy

- Version: 1.0.0
- Date: 2026-05-07
- Status: Active
- Scope: CarFight 프로젝트의 UI/Debug UI/에디터용 표시 텍스트 언어 정책

---

## 1. 문서 목적

이 문서는 CarFight 프로젝트에서 **내부 식별자와 화면 표시 텍스트의 언어를 분리하는 공통 원칙**을 정의한다.

프로젝트 코드, 에셋, enum, SectionId, FieldId 같은 내부 식별자는 개발 안정성과 추적성을 위해 영문을 유지한다.
반대로 플레이 중 화면에 보이거나, 디버그 패널에서 사람이 읽는 제목/라벨/상태값/설명문은 한국어로 표시한다.

---

## 2. 핵심 원칙

```text
내부에서 쓰이는 이름은 영문 유지
외부에 시각적으로 보이는 텍스트는 한국어 표시
```

이 원칙은 정식 UI뿐 아니라 개발용 Debug UI에도 적용한다.

---

## 3. 영문 유지 대상

아래 항목은 내부 식별자이므로 영문을 유지한다.

- C++ 클래스명
- C++ 구조체명
- enum 이름과 enum 값 이름
- 변수명
- 함수명
- 파일명
- 에셋명
- Blueprint 변수명
- Blueprint 함수명
- `SectionId`
- `FieldId`
- 내부 Snapshot 키
- 로그 원본 키
- 저장/직렬화용 키
- 코드에서 비교나 검색 기준으로 쓰는 문자열

예시:

```cpp
SectionId = TEXT("RuntimeSummary");
FieldId = TEXT("camera_status_summary");
CurrentCameraMode = ECFVehicleCameraMode::Default;
```

---

## 4. 한국어 표시 대상

아래 항목은 사람이 화면에서 읽는 텍스트이므로 한국어를 우선한다.

- UI 제목
- 탭 제목
- 섹션 제목
- 필드 라벨
- 상태값
- 경고 문구
- 설명 문구
- 버튼 텍스트
- Tooltip 표시 문구
- 디버그 패널에 표시되는 요약 문장
- 개발자가 PIE 중 읽는 화면 표시용 문자열

예시:

```cpp
TitleText = TEXT("런타임 요약");
LabelText = TEXT("상태 요약");
ValueText = TEXT("카메라 압축");
```

---

## 5. Debug UI 적용 원칙

Debug UI에서도 내부 데이터 구조는 영문을 유지한다.

예시:

```text
FCFVehicleDebugRuntime.RuntimeSummary      // 내부 필드, 영문 유지
RuntimeSummary SectionId                   // 내부 SectionId, 영문 유지
"런타임 요약"                              // 화면 표시 제목, 한국어
```

원본 디버그 문자열이 이미 영문 키 기반으로 만들어져 있다면, 원본 생성 지점을 무리하게 바꾸지 않는다.
대신 화면에 표시하기 직전 변환 계층에서 한국어로 바꾼다.

권장 흐름:

```text
원본 디버그 문자열 생성
  -> 내부 키/식별자는 영문 유지
  -> Panel/ViewData/Widget 표시 직전 한국어 변환
  -> 화면에는 한국어 제목/라벨/상태값 표시
```

---

## 6. 원본 문자열 변환 기준

원본 문자열이 로그, 디버그 스냅샷, 검증 요약 등 여러 곳에서 재사용될 수 있으면 원본은 유지한다.

예시:

```text
VehicleRuntime: Data=Present, Drive=Present, Ready=True
```

화면 표시 단계에서는 아래처럼 변환한다.

```text
데이터=있음
주행=있음
준비=예
```

즉, 원본 문자열은 추적/검색/로그 호환성을 위해 영문 구조를 유지하고, UI에서 읽는 최종 표시만 한국어화한다.

---

## 7. 예외 기준

아래 항목은 화면에 보여도 영문 유지가 허용된다.

- 클래스명
- 에셋명
- enum 원문 값
- DataAsset 이름
- Component 이름
- 디버그 식별자
- 정확한 검색/복사가 필요한 내부 키
- 엔진/플러그인에서 제공하는 고유 이름

예시:

```text
BP_CFVehiclePawn
CFVehicleDebugPanelWidget
ECFVehicleCameraMode::Default
VehicleCameraComp
```

이런 값은 한국어로 번역하면 오히려 추적이 어려워질 수 있으므로 원문 유지가 가능하다.

---

## 8. 구현 시 체크리스트

새 UI 또는 Debug UI를 만들 때 아래를 확인한다.

- [ ] 내부 식별자는 영문으로 유지했는가?
- [ ] 화면에 보이는 제목은 한국어인가?
- [ ] 화면에 보이는 라벨은 한국어인가?
- [ ] 화면에 보이는 상태값은 한국어인가?
- [ ] 원본 로그/스냅샷 문자열을 불필요하게 바꾸지 않았는가?
- [ ] 표시 직전 변환 계층이 필요한 경우 별도 함수로 분리했는가?
- [ ] 한국어 표시 때문에 내부 검색성이나 코드 추적성이 떨어지지 않는가?

---

## 9. 관련 문서

- `Document/ProjectSSOT/Systems/UI/VehicleDebug.md`
- `Document/ProjectSSOT/Plan/CameraDebugPlan/CD_DebugDesign.md`
- `Document/ProjectSSOT/Plan/CameraDebugPlan/CD_Checklist.md`
- `Document/ProjectSSOT/Plan/CameraDebugPlan/CD_VerifyGuide.md`

---

## 10. Changelog

### v1.0.0 - 2026-05-07
- 내부 식별자는 영문 유지, 화면 표시 텍스트는 한국어 표시라는 프로젝트 공통 UI 텍스트 정책을 최초 작성했다.
