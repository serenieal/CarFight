# CarFight Code Work Gate

- 문서 버전: v2.2
- 작성일: 2026-07-16
- 최근 갱신일: 2026-08-02
- 문서 상태: Current
- 적용 범위: `main_game` CarFight 저장소의 실제 코드·설정·스크립트 작업

---

## 1. 최우선 원칙

이 문서는 CarFight 코드 작업의 최우선 실행 게이트다.
루트 `AGENTS.md`의 일반 코드 작성 규칙, 구현 절차와 개별 Plan보다 먼저 적용한다.

```text
현재 AI 세션
= 저장소 분석
+ 설계와 작업 범위 확정
+ 실제 코드·설정·스크립트 수정
+ 빌드와 자동 테스트
+ Git diff 검수
+ 관련 문서 동기화

사용자
= Unreal Editor에서 필요한 Blueprint·DataAsset·Niagara·바이너리 에셋 작업
+ 사용자 조작이 필요한 PIE 시각·감각 검증
```

CarFight의 코드 작업은 현재 AI 세션이 승인된 저장소 도구를 사용해 직접 수행하는 것을 기본 경로로 한다.
작은 수정, 한 줄 수정, 컴파일 오류 수정, 기능 구현과 후속 보정 모두 동일한 직접 구현 절차를 따른다.

TaskSource, WorkOrder와 Codex YAML은 복잡한 작업을 구조화하거나 외부 실행을 명시적으로 선택할 때 사용할 수 있는 보조 산출물이다.
이 산출물이 없다는 이유만으로 직접 구현을 차단하지 않는다.

별도 Codex 위임은 기본 경로가 아니다.
사용자가 특정 작업을 별도 Codex 환경으로 넘기라고 명시했을 때만 선택적으로 사용한다.

---

## 2. 적용 범위

다음 경로 또는 형식의 변경은 코드 작업으로 분류한다.

```text
UE/Source/**
UE/Config/**
Tools/**
*.Build.cs
*.Target.cs
*.cpp
*.h
*.ini
*.json
*.yaml
*.yml
*.py
*.ps1
*.bat
*.cmd
```

`UE/Plugins/ue-assetdump`와 `GoPyMCP`는 독립 Git 저장소다.
해당 저장소 내부 작업은 그 저장소의 가장 가까운 `AGENTS.md`, 문서 진입점과 자체 작업 방침을 따른다.
CarFight 정책을 독립 저장소 내부에 자동 적용하지 않는다.

문서 전용 수정, 읽기 전용 분석과 코드 리뷰는 코드 빌드가 필수는 아니다.
Blueprint, DataAsset, Niagara, StaticMesh와 기타 바이너리 에셋은 사용자가 Unreal Editor에서 직접 편집할 수 있으며, AI는 정확한 적용 절차와 검증 기준을 제공한다.

---

## 3. 코드 작업 전 필수 확인

코드·설정·스크립트를 수정하기 전에 다음 순서를 수행한다.

```text
1. 작업 소유 저장소 확인
2. Git 브랜치, upstream과 미커밋 변경 확인
3. 루트 또는 가장 가까운 AGENTS.md 확인
4. 이 CodeWorkGate 확인
5. Document/Document_Entry.md 확인
6. ActiveWork와 관련 ProjectSSOT 확인
7. 관련 Systems, 대표 Plan과 실제 코드 확인
8. 이번 작업의 목표와 완료 조건 확정
9. 변경 허용 파일과 보호 범위 확정
10. 빌드·자동 테스트·PIE 검증 방법 확정
```

위 확인 전에는 쓰기 도구를 호출하지 않는다.
이전 대화나 AI 기억보다 현재 Git 상태, 실제 코드와 현재 문서를 우선한다.

---

## 4. 직접 구현 기본 절차

현재 AI 세션은 다음 순서로 작업한다.

```text
1. 현재 구현과 호출 흐름 읽기
2. 변경 영향도와 회귀 위험 분석
3. 수정 파일과 구현 단위를 작게 고정
4. 승인된 저장소 쓰기 도구로 직접 수정
5. 수정 직후 Git diff 확인
6. 필요한 빌드 또는 자동 테스트 실행
7. 실패 시 로그 근거로 같은 범위 안에서 보정
8. 성공 후 사용자 PIE 또는 Editor 작업 절차 제공
9. 대표 Plan, ActiveWork, ProjectSSOT 또는 Systems 중 필요한 문서만 동기화
10. 변경 파일, 검증 결과, 미검증 항목과 다음 단계를 보고
```

구현과 검증은 가능한 한 같은 세션에서 끝낸다.
빌드나 테스트가 가능한 작업을 코드 수정만 하고 완료 처리하지 않는다.

---

## 5. 기존 미커밋 변경 보호

미커밋 변경이 존재해도 자동으로 작업을 중단하지 않는다.
다만 다음 원칙을 강제한다.

```text
- 기존 dirty 파일을 작업 대상이라는 이유만으로 전체 교체하지 않는다.
- 사용자가 만든 변경을 임의로 정리하거나 되돌리지 않는다.
- 수정 전 해당 파일의 현재 내용을 읽고 필요한 최소 범위만 변경한다.
- unrelated 파일과 중첩 저장소 변경을 건드리지 않는다.
- 충돌 가능성이 높으면 작업 범위를 더 작게 분리한다.
- 안전한 최소 변경 범위를 확정할 수 없을 때만 Blocked로 판정한다.
```

기존 변경과 새 변경이 같은 파일에 공존하면 Git diff에서 이번 작업의 변경 범위를 구분해 보고한다.

---

### 5.1 공용 통합 파일 단일 소유 — 2026-08-02 추가

여러 기능이 동시에 진행될 때 다음 파일은 기능별 로컬 파일이 아니라 **교차 기능 조립 지점**으로 취급한다.

```text
CFVehiclePawn.h/.cpp
CFVehicleWeaponComp.h/.cpp
CFVehicleData.h/.cpp
CFWeaponData.h/.cpp
CFProjectileActor.h/.cpp
CFPlayerController.h/.cpp
Document/ActiveWork.md
Document/ProjectSSOT/03_FeatureQueue.md
Document/Plan/README.md
```

동시 작업 규칙:

```text
1. 기능별 세션은 자기 기능의 로컬 타입·컴포넌트·테스트 파일을 우선 수정한다.
2. 공용 통합 파일은 한 시점에 하나의 통합 작업만 수정 소유권을 가진다.
3. 다른 기능 세션은 필요한 Hook, 입력·출력 계약과 보호 조건만 대표 Plan에 기록한다.
4. 공용 조립 변경은 실제 연관 기능 상태를 모두 읽은 통합 작업에서 최소 패치로 반영한다.
5. 공용 파일 변경 뒤에는 해당 파일을 공유하는 기능의 필수 회귀를 함께 실행한다.
6. 안전한 최소 병합 범위를 확정할 수 없으면 `Blocked — Unsafe Dirty Overlap`로 중단한다.
```

---

## 6. 필수 구현 품질 기준

코드 변경에는 다음 기준을 적용한다.

```text
- 변수와 함수 이름은 책임이 드러나도록 작성
- 새 파일명과 클래스명은 32자 이하
- 변수와 함수 바로 위에 역할 주석 작성
- Blueprint 노출 필드와 함수에 한국어 DisplayName·ToolTip 제공
- 기존 함수 시그니처, 경로와 Public/Private 구조를 이유 없이 변경하지 않음
- 무분별한 리네이밍 금지
- null, 비활성 데이터와 기존 에셋 호환 경로 유지
- 기능 실패가 unrelated 핵심 로직을 취소하지 않도록 책임 분리
- Tick에서 불필요한 검색, 할당과 자산 재설정 금지
```

구조 변경이 크면 한 번에 완성형으로 바꾸지 않고 빌드 가능한 작은 단계로 나눈다.

---

## 7. 검증 기준

작업 성격에 따라 다음 증거를 확보한다.

### 7.1 C++ 또는 Build.cs 변경

```text
- Git diff 검수
- 공식 CarFight Editor 빌드
- 관련 Automation이 있으면 실행
- 사용자 PIE가 필요한 항목 분리
```

공식 에디터 빌드는 루트 `AGENTS.md`에 지정된 다음 경로를 사용한다.

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

### 7.2 설정·스크립트 변경

```text
- 구문 또는 파서 검증
- 가능한 최소 실행 테스트
- Git diff 검수
- 실패 시 원복이 아니라 수정 근거와 실패 범위 기록
```

### 7.3 문서 전용 변경

```text
- 링크와 경로 확인
- 현재 상태와 역사 기록 구분
- 관련 현재 문서 간 표현 충돌 확인
```

### 7.4 Unreal Editor 사용자 검증

AI가 직접 확인할 수 없는 시각 품질, 조작감과 에셋 연결은 다음을 제공한다.

```text
- 정확한 에셋과 메뉴 경로
- 한국어 UI명(English)
- 입력할 값
- PIE 절차
- PASS 기준
- 실패 결과별 점검 분기
```

### 7.5 검증 결과와 실행 경로 분리

빌드, Automation, AssetDump, PIE와 기타 검증은 다음 항목을 서로 분리해서 기록한다.

```text
Validation
= 실제 검증 결과

Evidence
= Job ID, 보고서, 로그, 결과 파일 또는 사용자 확인

Execution Method
= 고정 프리셋, 공용 도구, 작업 전용 임시 경로 또는 사용자 수동 절차

Reusable Entry Point
= 다음 세션에서 그대로 다시 사용할 수 있는 공용 진입점의 존재 여부

Historical Scope
= 현재 상태인지, 특정 날짜와 체크포인트의 당시 상태인지 구분
```

다음 판정 규칙을 적용한다.

```text
- 검증 방식은 작업 성격, 현재 제공되는 도구, 허용 범위와 보호 범위에 맞게 선택한다.
- 작업 전용 단발성 스크립트나 임시 실행 경로는 해당 검증의 실행 수단일 뿐 공용 저장소 도구로 자동 승격하지 않는다.
- 임시 스크립트가 Git 미추적 상태라는 사실만으로 등록·재사용·공용화가 필요하다고 판정하지 않는다.
- 과거 체크포인트의 Runner Unavailable, Tool Unavailable 또는 Not Run 기록을 현재 저장소 전체의 영구적인 실행 불가 계약으로 해석하지 않는다.
- 현재 실행 가능 여부를 판단할 때는 현재 도구 표면, 실제 저장소 상태, 작업 허용 범위와 최신 검증 증거를 다시 확인한다.
- 최신 검증 증거가 있으면 Current 문서는 최신 결과를 반영하고, 과거 Not Run 기록은 날짜가 있는 Changelog나 체크포인트 이력으로만 보존한다.
- 검증을 실행하지 못했으면 도구 부재, 작업 승인 경로 미확보, 환경 실패, 작업 범위 제외 또는 사용자 검증 필요를 구체적으로 구분한다.
```

권장 기록 형식:

```text
Validation: PASS / FAIL / Not Run / Not Applicable
Evidence: <Job, Report, Log, Result File or User Result>
Execution Method: <Preset / Shared Tool / Task-local Temporary Path / User Procedure>
Reusable Entry Point: Available / Not Defined / Not Applicable
Historical Scope: Current / <Date and Checkpoint>
```

---

## 8. 완료 판정

직접 코드 작업은 다음 조건을 충족해야 완료로 기록한다.

```text
- 요청한 구현 범위가 실제 파일에 반영됨
- Git diff에서 변경 범위와 보호 범위 준수 확인
- 필수 빌드 또는 자동 테스트 결과 확인
- 실패한 검증과 실행하지 못한 검증을 구분
- 사용자 PIE가 필요하면 User Validation Pending으로 분리
- 관련 현재 상태 문서가 실제 코드와 일치
- commit·push 등 요청되지 않은 Git 쓰기 작업을 수행하지 않음
```

권장 상태 표기는 다음과 같다.

```text
Implementation Source: Browser AI Direct
Source Changes: Applied / Not Applied / Partial
Diff Review: PASS / FAIL / Pending
Build: PASS / FAIL / Not Run / Not Applicable
Automation: PASS / FAIL / Not Run / Not Applicable
PIE: PASS / FAIL / User Validation Pending / Not Applicable
Evidence: <Job, Report, Log, Result File or User Result>
Execution Method: <Preset / Shared Tool / Task-local Temporary Path / User Procedure>
Reusable Entry Point: Available / Not Defined / Not Applicable
Historical Scope: Current / <Date and Checkpoint>
```

TaskSource와 WorkOrder가 존재하면 구현 근거로 연결할 수 있지만, 존재하지 않아도 정책 위반이 아니다.

---

## 9. 차단 기준

다음 상황에서만 직접 구현을 차단한다.

```text
- 작업 소유 저장소 또는 대상 브랜치를 확인할 수 없음
- 기존 dirty 변경과 충돌해 안전한 최소 변경 범위를 확정할 수 없음
- 사용자 결정 없이는 서로 배타적인 구현 방향을 선택할 수 없음
- 필요한 쓰기·빌드·검증 도구가 반복 실패해 결과 신뢰성을 확보할 수 없음
- 요구사항이 현재 프로젝트 결정 또는 보호 범위와 직접 충돌함
```

차단 상태는 원인에 맞게 구체적으로 기록한다.

```text
Blocked — Unsafe Dirty Overlap
Blocked — User Decision Required
Blocked — Required Tool Unavailable
Blocked — Project Contract Conflict
```

`plan.*` 기능, 최종 Codex YAML 또는 외부 Codex 실행기가 없다는 사실은 차단 사유가 아니다.

---

## 10. 별도 Codex 사용

사용자가 별도 Codex 실행을 명시적으로 요청한 경우에만 다음 산출물을 만들 수 있다.

```text
- TaskSource
- 사람이 읽을 수 있는 WorkOrder
- Codex용 YAML 또는 프롬프트
- 허용 경로와 보호 범위
- 빌드·테스트와 결과 보고 형식
```

이 경우에도 현재 AI 세션은 실제 Git 상태와 코드 근거를 먼저 확인한다.
외부 결과를 받은 뒤에는 Git diff, 빌드, 테스트와 PIE 증거를 다시 검수한다.

---

## 11. Git 보호 규칙

사용자의 명시적 요청 없이는 다음을 수행하지 않는다.

```text
commit
push
reset
checkout
stash
rebase
merge
clean
```

빌드 산출물, unrelated 파일과 기존 사용자 변경을 정리 목적으로 삭제하지 않는다.

---

## 12. Changelog

### v2.2 - 2026-08-02

- 검증 결과, 증거, 실행 수단, 재실행 가능한 공용 진입점과 역사 범위를 서로 분리하는 기록 기준을 추가했다.
- 작업 전용 단발성 스크립트와 임시 실행 경로를 공용 저장소 도구로 자동 승격하지 않도록 명시했다.
- Git 미추적 임시 스크립트의 존재만으로 등록이나 재사용이 필요하다고 판정하지 않도록 고정했다.
- 과거 Runner Unavailable 또는 Not Run 기록을 현재의 영구적인 실행 불가 계약으로 해석하지 않고 현재 도구·저장소·범위·최신 증거를 다시 확인하도록 했다.
- Current 문서는 최신 검증 결과를 반영하고 과거 상태는 날짜가 있는 체크포인트 이력으로 보존하도록 구분했다.

### v2.1 - 2026-08-02

- `CFVehiclePawn`, VehicleWeaponComp, VehicleData, WeaponData, ProjectileActor, PlayerController와 Current 상태 색인을 교차 기능 공용 통합 파일로 지정했다.
- 기능별 로컬 구현과 공용 조립 변경을 분리하고 공용 통합 파일은 한 시점에 하나의 통합 작업만 수정하도록 고정했다.
- 공용 파일 변경 뒤에는 해당 파일을 공유하는 기능의 필수 회귀를 함께 실행하도록 규정했다.
- 안전한 최소 병합 범위를 확정할 수 없는 동시 dirty 작업은 `Blocked — Unsafe Dirty Overlap`로 중단하도록 명시했다.

### v2.0 - 2026-07-27

- CarFight 코드 작업의 기본 주체를 별도 Codex에서 현재 AI 세션의 직접 구현으로 변경했다.
- TaskSource·최종 Codex YAML 품질 게이트를 필수 착수 조건에서 선택적 보조 산출물로 변경했다.
- 분석, 직접 수정, diff 검수, 빌드·자동 테스트와 문서 동기화를 하나의 기본 작업 흐름으로 통합했다.
- 기존 dirty 변경 보호, 직접 구현 완료 기준과 구체적인 차단 기준을 정의했다.
- 별도 Codex는 사용자가 명시적으로 요청한 경우에만 사용하는 선택 경로로 변경했다.
- 독립 저장소인 AssetDump와 GoPyMCP는 자체 정책을 따르도록 적용 범위를 CarFight `main_game`으로 명확히 했다.

### v1.1 - 2026-07-16

- 브라우저 AI가 TaskSource와 최종 Codex YAML을 생성하고 별도 Codex가 실제 코드를 수정하는 방식을 사용했다.
- v2.0 적용 이후 이 방식은 역사 기록이며 현재 CarFight 기본 경로가 아니다.

---

## 13. Migration

```text
- 새 CarFight 코드 작업은 현재 AI 세션의 직접 구현 방식으로 시작한다.
- 기존 TaskSource와 WorkOrder는 설계 참고 자료로 유지하며 삭제하거나 실행 출처를 바꾸지 않는다.
- 기존 문서의 Ready for External Codex, Final YAML Missing과 Plan Work-Order Generation Blocked는 당시 상태 기록으로 보존할 수 있다.
- 현재 활성 작업에 남은 동일 표현은 직접 구현 가능 상태로 갱신한다.
- 과거 직접 구현을 Policy Violation으로 기록한 이력은 당시 정책 기준의 역사 기록으로 유지한다.
- v2.0 이후 TaskSource 또는 Codex YAML 없이 직접 구현한 것은 정책 위반이 아니다.
- 기존 미커밋 변경은 자동으로 되돌리거나 정리하지 않는다.
- 기존 문서의 Runner Unavailable, Tool Unavailable과 Execution Not Run은 해당 날짜·체크포인트의 역사 기록으로 읽으며 현재 실행 가능 여부를 대신하지 않는다.
- 새 검증 기록은 Validation, Evidence, Execution Method, Reusable Entry Point와 Historical Scope를 필요 범위에서 분리한다.
- 작업 전용 임시 스크립트는 명시적인 공용화 결정 없이 Git 등록이나 표준 실행기로 승격하지 않는다.
```
