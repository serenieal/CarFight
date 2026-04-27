# VehicleDebug v2 작업 목록

- Version: 0.2.1
- Date: 2026-04-27
- Status: Panel 1차 마감
- Scope: VehicleDebug v2를 실제 작업 단위로 나눈 실행 목록

---

## 1. 사용 목적

이 문서는 설계 문서를 실제 작업 항목으로 쪼개서,

- 무엇부터 시작할지,
- 어떤 순서로 진행할지,
- 어떤 파일/영역을 건드리게 될지

실무 기준으로 확인하기 위한 작업 목록이다.

---

## 2. 1차 목표

이번 1차 목표는 아래로 잡는다.

1. Snapshot 중심 구조 도입
2. 레거시 `WBP_VehicleDebug` 제거
3. Compact HUD 추가
4. Overview / Drive / Input / Runtime Panel 추가

Recent Events와 UI 소유권 이동은 2차 목표로 미뤄도 된다.

---

## 3. 선행 작업

### 3.1 기준선 정리
- [ ] 현재 `VehicleDebug.md` 다시 확인
- [ ] 현재 `GetDebugTextSingleLine()` 출력 샘플 저장
- [ ] 현재 `GetDebugTextMultiLine()` 출력 샘플 저장
- [ ] 현재 위젯 생성 흐름 확인
- [ ] 현재 표시 조건 확인

참고 문서:
- `CF_VehicleDebugBaseline.md`
- 코드 기준선과 PIE 실측 샘플을 같은 문서에 누적 관리한다.

### 3.2 영향 범위 확인
- [ ] `ACFVehiclePawn` 관련 공개 API 재확인
- [ ] 기존 `WBP_VehicleDebug` 바인딩 포인트 재확인
- [ ] `BP_CFVehiclePawn` 이벤트 그래프 영향 범위 정리

---

## 4. C++ 작업

## 4.1 Snapshot 구조 재편
- [ ] `FCFVehicleDebugSnapshot` 현재 필드 정리
- [ ] `Overview` 하위 구조 설계
- [ ] `Drive` 하위 구조 설계
- [ ] `Input` 하위 구조 설계
- [ ] `Runtime` 하위 구조 설계
- [ ] 필요 시 최근 이벤트 구조 초안 추가

## 4.2 데이터 채우기 함수 정리
- [ ] Pawn에서 Overview 데이터 채우기
- [ ] DriveComp 스냅샷과 Drive 카테고리 연결
- [ ] 입력 해석 결과와 Input 카테고리 연결
- [ ] 런타임 요약과 Runtime 카테고리 연결

## 4.3 레거시 텍스트 정리
- [ ] 레거시 `WBP_VehicleDebug`를 제거 대상으로 분리
- [ ] `ShouldShowDebugWidget()`가 레거시 숨김 고정 정책으로 바뀌었는지 확인
- [ ] `GetDebugWidgetVisibility()`가 `Collapsed` 고정인지 확인
- [ ] HUD/Panel 표시 조건이 레거시 Text 정책과 분리되었는지 확인

---

## 5. UMG / BP 작업

## 5.1 HUD 위젯
- [ ] `CFVehicleDebugHudWidget` C++ 부모 클래스 생성
- [ ] `WBP_VehicleDebugHud` 생성
- [ ] `WBP_VehicleDebugHud` 부모 클래스를 `CFVehicleDebugHudWidget`로 지정
- [ ] HUD 표시 필드 확정
- [ ] State / Speed / Input / RuntimeReady 배치
- [ ] HUD 표시 토글 추가

참고 문서:
- `CF_VehicleDebugHudGuide.md`
- HUD 1차는 `Overview` 카테고리만 읽는 것을 기본으로 한다.
- HUD 로직은 순수 BP 함수 그래프보다 C++ 부모 위젯에 두는 것을 기본안으로 한다.

## 5.2 Panel 위젯
- [x] `CFVehicleDebugPanelWidget` C++ 부모 클래스 생성
- [x] `WBP_VehicleDebugPanel` 생성
- [x] `WBP_VehicleDebugPanel` 부모 클래스를 `CFVehicleDebugPanelWidget`로 지정
- [x] 공통 Section 위젯 생성
- [x] 필드 Row 공통 위젯 생성
- [x] `Overview` 섹션 동적 표시 확인
- [x] `Drive` 섹션 동적 표시 확인
- [x] `Input` 섹션 동적 표시 확인
- [x] `Runtime` 섹션 동적 표시 확인

- 참고 문서:
- `CF_VehicleDebugPanelGuide.md`
- Panel 1차 마감 기준은 `VerticalBox_DynamicSectionHost` + `WBP_VehicleDebugSection` + `WBP_VehicleDebugFieldRow` 구조다.
- Panel 로직은 C++ 부모 위젯에 두고, Header 입력은 공통 Section WBP 이벤트 그래프에서 처리한다.

## 5.2.1 Panel 구조 전환(다음 단계)
- [x] `CF_VehicleDebugPanelSchema.md` 기준으로 ViewData 구조 설계 고정
- [x] `FieldViewData` 타입 초안 작성
- [x] `SectionViewData` 타입 초안 작성
- [x] `PanelViewData` 타입 초안 작성
- [x] Snapshot -> ViewData 빌더 함수 초안 작성
- [x] 공통 `SectionWidget` C++ 부모 클래스 초안 작성
- [x] `WBP_VehicleDebugSection` 공통 자식 위젯 초안 작성
- [x] 기존 정적 `WBP_VehicleDebugPanel`과 병행 가능한 전환 경로 검토
- [x] `VerticalBox_DynamicSectionHost` 기반 동적 Section 표시 PIE 확인
- [x] 기존 정적 `VerticalBox_CategoryList` 제거 방향 확정
- [x] `CFVehicleDebugFieldRowWidget` C++ 부모 클래스 추가
- [x] `WBP_VehicleDebugFieldRow` 공통 자식 위젯 생성
- [x] `WBP_VehicleDebugSection`에 `VerticalBox_FieldHost` 추가
- [x] `WBP_VehicleDebugSection`의 `FieldRowWidgetClass`를 `WBP_VehicleDebugFieldRow`로 지정
- [x] Field Row 기반 표시 PIE 확인
- [x] 하위 Section 들여쓰기 적용 및 PIE 확인
- [x] `CarFight_ReEditor Win64 Development` 빌드 성공 확인

참고 문서:
- `CF_VehicleDebugPanelSchema.md`
- Panel 2차부터는 섹션을 WBP에 계속 수동 추가하는 방식이 아니라 공통 Section/Field 구조 재사용을 기준으로 한다.
- 새 섹션은 루트 WBP가 아니라 ViewData 빌더에 추가한다.

## 5.3 기존 텍스트 위젯 정리
- [ ] `WBP_VehicleDebug` 제거 결정 반영
- [ ] `BP_CFVehiclePawn`에서 `WBP_VehicleDebug` 생성/보관 노드 제거
- [ ] 삭제 전 회귀 비교용 기준선 문서 보존

---

## 6. 표시 정책 작업

- [ ] `bShowDebugHud` 도입 여부 결정
- [ ] `bShowDebugPanel` 도입 여부 결정
- [ ] `bShowDebugTextView` 도입 여부 결정
- [ ] 카테고리별 표시 토글 도입 여부 결정
- [ ] 기존 `DriveStateDebugDisplayMode` 역할 재정의 여부 결정

---

## 7. 이벤트 작업(2차 권장)

- [ ] 최근 이벤트 구조 정의
- [ ] Drive 상태 전이 이벤트 Push
- [ ] Runtime 재초기화 이벤트 Push
- [ ] InputOwner 변경 이벤트 Push
- [ ] 이벤트 최대 보관 개수 제한
- [ ] `WBP_VehicleDebugEvents` 생성

---

## 8. 검증 작업

## 8.1 기능 검증
- [ ] HUD가 정상 표시되는지 확인
- [x] Panel이 정상 표시되는지 확인
- [x] 카테고리 값 누락이 없는지 확인
- [ ] 로컬 제어 차량에서만 표시되는지 확인
- [x] 기존 `WBP_VehicleDebug`가 더 이상 표시되지 않는지 확인

## 8.2 사용성 검증
- [ ] HUD만 봐도 현재 핵심 상태를 이해할 수 있는지 확인
- [x] Panel이 텍스트 덩어리보다 읽기 쉬운지 확인
- [x] Runtime 문제 원인을 찾기 쉬운지 확인
- [x] Drive와 Input 문제를 분리해서 읽기 쉬운지 확인

## 8.3 회귀 검증
- [ ] HUD/Panel 표시 조건이 Legacy Text 제거 후에도 의도대로 유지되는지 확인
- [ ] HUD에 필요한 핵심 항목이 빠지지 않았는지 확인
- [ ] 기존 상태 전이 요약이 HUD 또는 후속 Panel 경로에서 유지되는지 확인

---

## 9. 문서 작업

- [ ] 설계 변경분을 `VehicleDebug.md`에 반영할 시점 판단
- [ ] 새 위젯 이름을 문서에 기록
- [ ] 새 C++ 부모 위젯 이름과 파일 경로를 문서에 기록
- [ ] 새 토글 변수 이름을 문서에 기록
- [ ] `WBP_VehicleDebug` 제거 방향을 문서에 기록
- [ ] 카테고리 구조를 Systems 문서로 승격할지 검토

---

## 10. 추천 작업 순서

### 순서 A - 안전한 시작
1. Snapshot 구조 재편
2. 텍스트 호환 유지
3. HUD 추가
4. Panel 추가
5. 이벤트 추가

### 순서 B - 빠른 가시화 우선
1. HUD 목업 먼저 제작
2. 필요한 필드 역산
3. Snapshot 구조 정리
4. Panel 확장

이번 주제는 구조 개편이 핵심이므로, 기본 추천은 `순서 A`다.

---

## 11. 오늘 바로 시작하기 좋은 최소 작업

당장 첫 커밋 수준으로 시작하기 좋은 최소 묶음은 아래다.

- [ ] `FCFVehicleDebugSnapshot` 카테고리 초안 작성
- [ ] `CFVehicleDebugHudWidget` C++ 부모 클래스 생성
- [ ] `WBP_VehicleDebugHud` 뼈대 생성
- [ ] Overview 핵심값 4~6개 HUD에 표시
- [ ] 레거시 `WBP_VehicleDebug` 표시 제거

이 묶음만 끝내도 구조 개편의 방향이 확정되고, 눈에 보이는 결과가 생긴다.

---

## 12. Changelog

### v0.2.1 - 2026-04-27
- Panel 1차 마감 상태를 작업 목록에 반영했다.
- `WBP_VehicleDebugPanel`, `WBP_VehicleDebugSection`, `WBP_VehicleDebugFieldRow` 기반 운영 구조와 PIE 확인 항목을 완료 처리했다.
- 하위 Section 들여쓰기 적용 및 빌드 성공 확인을 완료 항목에 추가했다.

### v0.2.0 - 2026-04-27
- `CFVehicleDebugFieldRowWidget` 도입과 `WBP_VehicleDebugFieldRow` 생성 작업을 Panel 구조 전환 태스크에 추가했다.
- `VerticalBox_FieldHost`와 `FieldRowWidgetClass` 설정, Field Row PIE 확인 항목을 추가했다.

### v0.1.9 - 2026-04-24
- Panel 구조 전환 태스크를 실제 완료 상태에 맞춰 갱신했다.
- `VerticalBox_DynamicSectionHost` 기반 동적 Section 표시 확인과 `VerticalBox_CategoryList` 제거 방향을 작업 목록에 반영했다.

### v0.1.8 - 2026-04-24
- `CFDebugPanelViewData.h`와 `CFVehicleDebugSectionWidget` 기반 클래스 추가 이후 기준으로 Panel 구조 전환 태스크 상태를 갱신했다.

### v0.1.7 - 2026-04-24
- `CF_VehicleDebugPanelSchema.md` 기준의 Panel 구조 전환 태스크를 추가했다.
- 다음 단계 목표를 “기능 추가”보다 “데이터 기반 Panel 구조 전환” 중심으로 재정리했다.

### v0.1.6 - 2026-04-23
- Panel 1차 완료 기준을 반영해 Runtime 3분할 하위 섹션과 WBP 이벤트 그래프 기반 Header 입력 연결을 현재 태스크 기준으로 정리했다.

### v0.1.5 - 2026-04-23
- Panel 상호작용 모드와 카테고리별 접기/펼치기 확인 항목이 실제 진행 상태에 맞도록 보강되었다.
- 체인지로그가 누락되지 않도록 작업 목록 문서에도 변경 이력을 남기는 규칙을 반영했다.

### v0.1.4 - 2026-04-23
- 레거시 `WBP_VehicleDebug` 제거 작업을 명시적 태스크로 승격했다.
- `CF_VehicleDebugBaseline.md`와 `CF_VehicleDebugHudGuide.md` 참조를 선행 작업 맥락에 반영했다.
