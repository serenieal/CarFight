# 타겟 선택 Plan 작업지시 규격

- 문서 버전: v0.1.0
- 상태: Plan 입력 준비
- 작성일: 2026-07-22
- 기준 기획: `../Design/TargetSelect.md`
- 상세 작업: `TargetSelectPlan.md`
- 로드맵: `TargetSelectRoadmap.md`

---

## 1. 목적

이 문서는 Codex 또는 Plan 기능에 전달할 타겟 선택 시스템 작업지시서의 고정 형식을 정의한다.

Plan은 이 문서와 기준 기획을 읽고 작업 범위를 구체화해야 한다. 구현자는 기획 결정을 재해석하거나 여러 Task를 임의로 묶어서는 안 된다.

---

## 2. 작업지시 작성 원칙

1. 작업지시서 하나는 Task ID 하나만 수행한다.
2. 실제 파일과 에셋 경로를 확인한 후 Target Files를 잠근다.
3. Locked Decisions는 구현 중 변경하지 않는다.
4. 조정 가능한 값은 Constraints 또는 Tunable Parameters로 분리한다.
5. Acceptance Criteria는 관찰 가능하고 검증 가능하게 작성한다.
6. Verification은 실행 순서와 기대 결과를 포함한다.
7. Out of Scope를 명시하여 리팩터링 범위 팽창을 막는다.
8. Stop Conditions를 명시해 근거 없는 추측 구현을 방지한다.
9. 기존 미커밋 변경을 보존한다.
10. 코드·Blueprint·문서 산출물을 구분한다.
11. 모든 파일명은 32자 이내로 유지한다.
12. Blueprint 공개 변수와 함수에는 한국어 또는 명확한 영어 툴팁을 작성한다.
13. 코드의 변수와 함수 위에는 역할을 설명하는 주석을 작성한다.
14. 변경 문서에는 버전, Changelog, Migration을 기록한다.

---

## 3. 필수 슬롯

모든 작업지시서는 아래 슬롯을 포함해야 한다.

### Task ID

`TS-[단계]-[번호]` 형식을 사용한다.

예:

- `TS-P0-00`
- `TS-P0-01`
- `TS-P1-02`

### Goal

이번 Task가 완료되면 어떤 상태가 되는지 한 문단으로 작성한다.

### Context

관련 기획, 기존 구현, 선행 Task 결과를 기록한다.

### Locked Decisions

이번 Task에서 바꿀 수 없는 기획 결정을 기록한다.

### Constraints

엔진 버전, 파일명, C++/BP 역할, 성능, 네트워크, 기존 변경 보호 등의 제약을 기록한다.

### Tunable Parameters

구현은 필요하지만 데이터로 조정 가능한 값을 기록한다.

### Dependencies

선행 Task, 필요한 클래스, 필요한 에셋, 외부 시스템 계약을 기록한다.

### Target Files

읽기·수정·생성 대상을 실제 경로로 기록한다.

### Implementation Steps

작업 순서를 번호로 작성한다.

### Deliverables

코드, Blueprint, 데이터, UI, 문서 산출물을 구분한다.

### Acceptance Criteria

완료를 판정할 수 있는 조건을 기록한다.

### Verification

컴파일, PIE, 디버그 출력, 테스트 장면과 기대 결과를 기록한다.

### Out of Scope

이번 Task에서 하지 않을 일을 기록한다.

### Stop Conditions

추측 구현을 중단하고 보고해야 하는 조건을 기록한다.

### Documentation Updates

완료 후 갱신할 문서와 상태를 기록한다.

---

## 4. 복사용 작업지시 템플릿

```markdown
# [Task ID] [작업명]

## Goal
[완료 상태를 구체적으로 작성]

## Context
- 기준 기획: Document/Design/TargetSelect.md
- 작업계획: Document/Plan/TargetSelect/TargetSelectPlan.md
- 선행 Task:
- 현재 구현 상태:

## Locked Decisions
- 직접 조준 대상이 최우선이다.
- 직접 대상이 없으면 크로스헤어 근접 후보를 사용한다.
- 선택과 조준을 분리한다.
- 선택과 장비 락온을 분리한다.
- 선택 대상은 화면 밖에서도 유지한다.
- 자동 다음 타겟은 사용하지 않는다.
- [해당 Task의 추가 잠금 결정]

## Constraints
- 최신 Unreal Engine 프로젝트 규칙을 따른다.
- 반복 검색과 상태 수명은 C++을 우선한다.
- 시각 표현과 콘텐츠 설정은 Blueprint/Data를 우선한다.
- 기존 사용자 변경사항을 덮어쓰지 않는다.
- 실제 경로를 확인하지 못한 파일은 수정하지 않는다.
- 파일명은 32자 이내다.
- Blueprint 공개 변수와 함수에 툴팁을 작성한다.
- 코드 변수와 함수 위에 역할 주석을 작성한다.
- 범위 밖 리팩터링을 하지 않는다.

## Tunable Parameters
- [설정 이름]: [초기값 / 저장 위치]
- [설정 이름]: [초기값 / 저장 위치]

## Dependencies
- [선행 Task]
- [필요 시스템]
- [필요 에셋]

## Target Files

### Read
- [실제 경로]

### Modify
- [실제 경로]

### Create
- [실제 경로]

## Implementation Steps
1. [구조 확인]
2. [최소 구현]
3. [Blueprint 또는 UI 연결]
4. [오류 및 수명 처리]
5. [디버그 관찰 수단]
6. [검증]
7. [문서 갱신]

## Deliverables

### C++
- [산출물]

### Blueprint
- [산출물]

### Data
- [산출물]

### UI
- [산출물]

### Documentation
- [산출물]

## Acceptance Criteria
- [관찰 가능한 완료 조건]
- [오류 처리 조건]
- [성능 또는 결정성 조건]

## Verification
1. [실행 절차]
   - 기대 결과: [결과]
2. [실행 절차]
   - 기대 결과: [결과]
3. 최종 diff를 검토한다.
   - 기대 결과: 범위 밖 변경이 없다.

## Out of Scope
- [후속 Task 기능]
- [관련 없는 리팩터링]

## Stop Conditions
- 대상 파일 또는 에셋 경로가 확인되지 않음
- 기존 시스템과 책임이 중복되지만 통합 기준이 없음
- 기존 미커밋 변경과 충돌
- 기획의 잠금 결정을 변경해야만 구현 가능
- 검증 환경이 없어 완료 조건을 확인할 수 없음

## Documentation Updates
- Document/Design/TargetSelect.md
- Document/Plan/TargetSelect/TargetSelectPlan.md
- 해당 Task 결과 문서
```

---

## 5. 최초 실행 권장 작업지시

첫 구현 작업은 코드를 바로 작성하는 `TS-P0-01`이 아니라, 실제 프로젝트 구조를 확정하는 `TS-P0-00`으로 시작한다.

### Task ID

`TS-P0-00`

### Goal

CarFight 저장소와 Unreal 에셋을 조사하여 타겟 선택 시스템을 연결할 실제 클래스, 입력 자산, HUD, 관계 시스템, 장비 시스템, 충돌 채널을 확인하고 후속 구현 Task가 추측 없이 Target Files를 잠글 수 있는 조사 결과를 작성한다.

### Locked Decisions

- 기획의 확정 결정은 변경하지 않는다.
- 기존 기능을 제거하거나 수정하지 않는다.
- 실제 존재가 확인된 파일과 에셋만 결과에 기록한다.
- C++/Blueprint 역할은 현재 구조를 존중하되 반복 핵심 로직은 C++을 우선한다.
- 조사 Task에서는 구현하지 않는다.

### Constraints

- 저장소와 UE 에셋은 읽기 전용으로 조사한다.
- 파일명은 32자 이내로 제안한다.
- 클래스·에셋 경로를 추측하지 않는다.
- 미커밋 변경을 수정하지 않는다.
- 조사 결과가 불충분하면 구현 Task를 생성하지 않는다.

### Target Files

조사 시작 시 실제 프로젝트에서 확정한다.

최소 조사 범위:

- 플레이어 차량 Pawn/Controller
- 크로스헤어
- HUD 루트
- Enhanced Input
- 차량 베이스
- 관계·진영
- 상호작용 또는 선택 가능 계약
- 무기와 유틸리티 장비
- 충돌 설정
- 관련 설계 문서

### Acceptance Criteria

- 실제 경로가 있는 조사 대상 목록
- 재사용 가능 타입과 신규 필요 타입 구분
- 타겟 선택 컴포넌트 소유 위치 제안
- 입력 연결 위치 제안
- HUD 연결 위치 제안
- 충돌 판정 전략 제안
- 네트워크 P0 범위 기록
- 모든 미결 항목의 상태 기록
- TS-P0-01 작업지시서의 Target Files 작성 가능

### Verification

- 파일 검색 결과와 클래스 내용을 교차 확인
- Blueprint는 에셋 덤프 또는 그래프 확인
- 프로젝트 설정의 충돌 채널 확인
- 관련 변경이 발생하지 않았는지 Git status/diff 확인

### Out of Scope

- C++ 구현
- Blueprint 수정
- 입력 자산 생성
- HUD 생성
- 프로젝트 설정 변경
- Git commit 또는 push

---

## 6. TaskSource 작성 체크리스트

Plan 기능에 요청하기 전에 다음을 확인한다.

- [ ] Task ID가 하나인가
- [ ] Goal이 완료 상태를 설명하는가
- [ ] Locked Decisions가 기획과 일치하는가
- [ ] 실제 Target Files가 존재 확인됐는가
- [ ] 생성 파일 이름이 32자 이내인가
- [ ] 선행 Task가 완료됐는가
- [ ] Acceptance Criteria가 검증 가능한가
- [ ] Verification에 기대 결과가 있는가
- [ ] Out of Scope가 충분한가
- [ ] Stop Conditions가 있는가
- [ ] 문서 갱신 대상이 있는가
- [ ] 기존 사용자 변경 보호가 명시됐는가

---

## 7. Plan 결과 검토 게이트

Plan이 생성한 작업지시서는 실행 전에 다음을 검토한다.

### 범위 게이트

- Task ID가 하나인지
- 범위 밖 파일이 포함되지 않았는지
- 관련 없는 리팩터링이 없는지

### 기획 게이트

- 직접 조준 우선이 유지되는지
- 선택과 조준이 분리되는지
- 선택과 장비 획득이 분리되는지
- 자동 다음 타겟이 추가되지 않았는지

### 기술 게이트

- 실제 파일 경로인지
- C++/BP 책임이 적절한지
- 수명과 무효 참조 처리가 있는지
- 디버그와 검증 수단이 있는지

### 완료 게이트

- Acceptance Criteria가 명확한지
- Verification이 실행 가능한지
- 문서 갱신과 diff 검토가 포함됐는지

---

## 8. 변경 이력

### v0.1.0 - 2026-07-22

- Plan 작업지시 필수 슬롯 정의
- 복사용 템플릿 작성
- TS-P0-00 최초 조사 작업지시 작성
- TaskSource 및 Plan 검토 체크리스트 추가

---

## 9. 마이그레이션

기존 작업지시 형식이 있다면 다음 필드를 추가한다.

- Locked Decisions
- Tunable Parameters
- Verification의 기대 결과
- Out of Scope
- Stop Conditions
- Documentation Updates

기존 작업지시를 한 번에 모두 변경하지 않고, 타겟 선택 시스템의 신규 Task부터 본 형식을 적용한다.
