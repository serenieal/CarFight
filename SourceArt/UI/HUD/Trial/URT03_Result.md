# UI Resource Handoff Trial 03

- 문서 버전: v1.0
- 최근 갱신일: 2026-08-27
- 상태: Pre-USER End-to-End Review
- 범위: 완결된 Outer Frame Housing 1개 모듈의 Concept → Source → Layer → Composite → 부분 수정 → Runtime Packing
- Production/UE 적용: 없음

## 1. 목적

Trial 02가 좌상단 코너의 좌표 정밀도에 과도하게 수렴한 문제를 교정하고, 실제로 필요한 UI Resource Handoff 전체 흐름을 검증한다.

이번 Trial의 핵심 질문은 선을 몇 px 정확히 땄는가가 아니다.

우선 평가 축:
- Structural Fidelity
- Visual Identity Fidelity
- Editability
- Layer Economy
- Runtime Suitability

공용 Gate는 Document/SSOT/Shared/UI_Resource_Gate.md v0.4에서 이 평가 순서를 소유한다.

## 2. Trial 02에서 이어받은 것

Trial 02는 Deterministic Geometry를 반복 가능한 좌표/패스/마스크 Source로 표현할 수 있다는 Capability Evidence만 유지한다.
한 코너의 좌표 정밀도를 계속 올리는 대신, Trial 03에서 완결된 리소스 모듈 전체 흐름으로 확대한다.

## 3. Trial 03 Resource Contract

Authoring Layer:
- FrameBase
- FrameEdge
- FrameAccent
- FrameGlow

Bounded Edit Demonstration:
- 가정 요청: Accent 선 두께만 줄여라.
- 변경 허용: FrameAccent, FrameGlow
- 변경 금지: FrameBase, FrameEdge
- 전체 Resource 재생성 금지

Runtime Packing:
- FrameRuntimeBase = Base + Edge + Accent
- FrameRuntimeGlow = Glow
- Runtime Texture 후보 수: 2

## 4. Review Evidence

A. Concept / Visual Target
B. Authoring Layers
C. Final Composite
D. Target + Composite Overlay
E. Bounded Edit Variant
F. Runtime Pack

## 5. USER Review 질문

Structural Fidelity:
- 전체 Frame 실루엣과 큰 비례가 Target의 디자인 언어를 유지하는가?
- Outer / Inner 구조가 과도하게 단순해지지 않았는가?

Visual Identity Fidelity:
- 최종 Composite가 같은 UI 계열로 느껴지는가?
- 별개의 SF Frame으로 재디자인된 느낌이 강하지 않은가?

Editability:
- Accent만 수정한 Variant에서 Base/Edge가 흔들리지 않았는가?
- 향후 여기만 바꿔 요청을 Layer ownership으로 처리할 수 있어 보이는가?

Layer Economy:
- Base / Edge / Accent / Glow 4 Authoring Layer가 과도하지 않은가?
- 더 합쳐도 수정성이 유지되는 Layer가 있는가?

Runtime Suitability:
- Base + Glow 2 Runtime Texture packing이 합리적인가?
- Runtime 제어가 필요한 Glow를 별도 소유하는 것이 맞는가?

## 6. 현재 판정

- End-to-End flow: Built
- USER review evidence: Built
- Production import: 0
- UE Asset mutation: 0
- Technique promotion: Pending

USER Review 후 결과는 PASS / REWORK / FAIL 중 하나로 닫는다.

## 7. Changelog

### v1.0 - 2026-08-27

- 미시적 Geometry Trial에서 End-to-End Resource Handoff Trial로 범위를 확대했다.
- 평가 축을 Structural Fidelity / Visual Identity / Editability / Layer Economy / Runtime Suitability로 전환했다.
- 4 Authoring Layer와 2 Runtime Texture packing 후보를 정의했다.
- 부분 수정 Demo를 포함해 전체 재생성 없이 특정 책임만 수정 가능한가를 검증 범위에 추가했다.
