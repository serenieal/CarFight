# UI Reconstruction Trial 02

- 문서 버전: v1.1
- 최근 갱신일: 2026-08-27
- 상태: Technique Capability Evidence / End-to-End Quality Validation은 Trial 03으로 이동
- 범위: 외부 프레임 좌상단 코너의 Deterministic Geometry Reconstruction
- Production/UE 적용: 없음

## 1. 목적

Trial 01의 실패를 반복하지 않고, Flat Concept의 프레임 구조를 생성형 이미지가 아니라 수정 가능한 결정형 좌표로 재구성할 수 있는지 검증한다.

이번 Trial은 예쁜 최종 리소스를 만드는 실험이 아니다.
다음 네 가지가 핵심 evidence다.

1. Reference와 형상 언어가 충분히 가까운가
2. Outer / Inner Edge / Inner Rail이 서로 독립 구조로 유지되는가
3. 좌표를 수정하면 전체를 재생성하지 않고 부분 수정 가능한가
4. Geometry PASS 전 Glow/Texture/Treatment 없이도 형상을 판단할 수 있는가

## 2. Trial 01에서 반영한 개선

Trial 01은 생성형 이미지로 "재구성된 것처럼 보이는 Frame"을 만들었기 때문에 Geometry Reconstruction 검증으로 유효하지 않았다.

Trial 02에서는 다음을 강제한다.

- 이미지 생성 사용 0
- Geometry를 명시적 좌표로 기록
- Reference / Geometry-only / Overlay / Critical Zoom 비교
- Geometry PASS 전 Treatment 금지
- 사용자 Visual PASS 전 Validated Technique 승격 금지

공용 예방 규칙은 `Document/SSOT/Shared/UI_Resource_Gate.md v0.3`에 반영됐다.

## 3. Source Authority

- Canonical visual target: `SourceArt/UI/HUD/VT/VT_VehPanel01.jpg`
- Size: 1280 x 594
- SHA-256: `cf5b0a57b76f9c41272ee01ed752d263a7b01a78bf02e9cb9dc042d000df6b6b`
- Source type: Flat Raster
- Trial scope: visible upper-left frame structure only

가려진 전체 Frame을 복원했다고 주장하지 않는다.
이번 좌표는 화면에서 실제로 보이는 좌상단 구조를 대상으로 한 Reconstruction Candidate다.

## 4. Deterministic Source

정확한 좌표는 `URT02_Geometry.json`이 소유한다.

현재 분리한 구조:

- Outer Contour
- Inner Edge
- Inner Rail

이 좌표는 생성형 재생성 없이 독립적으로 수정할 수 있다.

## 5. USER Review Gate

현재 결과는 Production Handoff PASS가 아니다. 좌표 기반 Deterministic Geometry를 만들고 부분 수정 가능한 Source로 표현할 수 있다는 Capability Evidence만 유지한다.

USER가 Review 이미지에서 다음을 판정해야 한다.

- 기존 Trial 01처럼 원본과 전혀 다른 새 Frame으로 보이지 않는가
- 외곽 꺾임 위치와 상단 계단 구조가 Reference를 충분히 따라가는가
- Inner Edge와 Inner Rail의 간격이 자연스러운가
- 이 수준의 좌표 기반 수정이 실제 Production Resource 제작의 기반이 될 수 있어 보이는가

### 결과 처리

- PASS: 좌표 수정성 검증 후 Treatment Trial로 진행
- REWORK: Overlay에서 어긋난 좌표만 교정하고 Trial 02 유지
- FAIL: 현재 Polygon/Polyline 방식의 한계를 기록하고 다른 deterministic technique 후보를 연다

## 6. 현재 차단 범위

Blocks:

- Trial 02 Treatment / Glow 단계
- SVG/Vector 계열 Technique의 Validated 승격

Does Not Block:

- 기존 승인 UI Asset 재사용
- 관련 없는 Runtime/UMG 작업
- 다른 CarFight 작업

## 7. Changelog

### v1.1 - 2026-08-27

- USER 피드백에 따라 좌상단 코너의 미세 좌표 최적화를 중단했다.
- Trial 02를 Deterministic Geometry Capability Evidence로 한정하고, 전체 Resource Handoff 품질 검증은 Trial 03으로 이관했다.
- Pixel/좌표 Fidelity를 최종 목표로 확대하지 않도록 범위를 교정했다.

### v1.0 - 2026-08-27

- Trial 01 실패를 입력으로 한 개선형 Geometry Reconstruction Trial을 신규 정의했다.
- 생성형 evidence를 제거하고 explicit coordinate source + overlay review 계약을 적용했다.
- USER Geometry PASS 전 Treatment와 Technique 승격을 차단했다.
