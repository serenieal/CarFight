# CarFight Code Work Gate

- 문서 버전: v1.1
- 작성일: 2026-07-16
- 최근 갱신일: 2026-07-16
- 문서 상태: Current
- 적용 범위: 웹브라우저 AI가 지휘하는 CarFight, AssetDump, GoPyMCP의 실제 코드·설정·스크립트 작업 준비

---

## 1. 최우선 원칙

이 문서는 웹브라우저 AI의 코드 작업 준비 게이트다.
루트 `AGENTS.md`의 일반 코드 작성 규칙, 구현 절차와 개별 Plan보다 먼저 적용한다.

```text
웹브라우저 AI 세션
= 분석, 설계, TaskSource 작성, Codex용 최종 작업지시서 생성과 전달 담당

별도 Codex 세션 또는 사용자가 선택한 Codex 환경
= 전달받은 최종 작업지시서에 따라 실제 코드 수정과 자동 검증 수행

웹브라우저 AI의 후속 검수 세션
= Codex 적용 후 실제 Git diff, 빌드, 테스트와 PIE 결과 검수 담당
```

웹브라우저 AI는 C++, Go, Python, PowerShell, 배치 파일, 빌드 스크립트와 텍스트 설정을 기본적으로 직접 수정하지 않는다.
작은 수정, 한 줄 수정, 컴파일 오류 수정과 후속 보정도 먼저 Codex용 작업지시서로 만든다.

**웹브라우저 AI 세션에 Codex 실행 도구가 연결되어 있을 필요는 없다.**
`plan.*` 기능은 Codex를 실행하는 기능이 아니라 Codex가 읽을 TaskSource와 최종 YAML 작업지시서를 생성하는 기능이다.
Codex 실행기 미연결은 정상 상태이며 작업지시서 생성을 중단하는 사유가 아니다.

이 규칙은 사용자가 세션 복원 문구를 사용했는지와 관계없이 모든 새 브라우저 세션의 코드 작업 요청에 적용한다.

---

## 2. 코드 작업으로 분류하는 범위

다음 중 하나를 변경하도록 Codex에 지시해야 하는 작업은 코드 작업으로 분류한다.

```text
UE/Source/**
UE/Config/**
Tools/**
UE/Plugins/*/Source/**
UE/Plugins/*/Scripts/**
GoPyMCP/Workspace/core/**
GoPyMCP/Workspace/adapters/**
GoPyMCP/Config/**
*.Build.cs
*.Target.cs
*.cpp
*.h
*.go
*.py
*.ps1
*.bat
*.cmd
*.ini
*.json
*.yaml
*.yml
```

문서 전용 수정, 읽기 전용 분석, 조사, 코드 리뷰와 사용자가 Unreal Editor에서 직접 수행하는 Blueprint·바이너리 에셋 작업은 이 게이트의 코드 수정 대상에서 제외한다.
단, 문서 작업 중 실제 코드 파일 변경이 필요해지면 브라우저 AI는 직접 수정하지 않고 Codex용 작업지시서 생성 단계로 전환한다.

---

## 3. 새 세션 필수 진입

CarFight 관련 코드 작업 요청을 받은 웹브라우저 AI는 사용자의 첫 문구가 무엇이든 다음 순서를 먼저 수행한다.

```text
1. 작업 소유 저장소 판별
2. 소유 저장소 Git 브랜치와 미커밋 변경 확인
3. 루트 또는 가장 가까운 AGENTS.md 확인
4. 이 CodeWorkGate 확인
5. 해당 저장소 Document Entry 확인
6. 관련 ProjectSSOT·Systems·대표 Plan과 실제 코드 확인
7. 작업 범위, 보호 범위, 완료 조건과 검증 방법 확정
```

위 확인 전에는 소스·설정·스크립트 쓰기 도구를 호출하지 않는다.
사용자가 `이전 작업 이어서 진행해줘` 또는 특정 작업 재개 문구를 사용하지 않았다는 이유로 이 절차를 생략하지 않는다.

---

## 4. 브라우저 AI 세션의 필수 산출물

브라우저 AI가 코드 작업 준비를 완료하려면 다음 산출물이 필요하다.

```text
대표 Plan
→ 현재 구현, 남은 작업과 설계 의도 확인

TaskSource
→ 목적, 근거, 허용 경로, 보호 범위, 구현 요구사항, 완료·실패 조건과 검증 방법

최종 Codex 작업지시서
→ Codex가 직접 읽을 최종 YAML 입력
→ 대상 파일, 금지사항, 구현 요구, 빌드·테스트와 결과 보고 형식 포함
```

TaskSource와 최종 Codex 작업지시서는 반드시 작업을 소유한 저장소의 Plan 폴더에 둔다.
CarFight, AssetDump와 GoPyMCP의 내부 작업지시서를 다른 저장소 문서체계에 복사하지 않는다.

### 4.1 Plan 기능의 역할

상황에 따라 다음 기능을 사용한다.

- 새 요청과 폴더 문맥으로 TaskSource·최종 계약을 만들 때: `plan.build_from_folder_request`
- 기존 문서에서 TaskSource를 합성할 때: `plan.synthesize_task_source`
- TaskSource의 부족 슬롯을 보완할 때: `plan.refine_task_source`
- 큰 작업을 작은 작업 단위로 나눌 때: `plan.decompose_tasks`
- TaskSource에서 Codex 계약들을 생성할 때: `plan.generate_task_contracts`
- 단일 최종 Codex 입력을 만들 때: `plan.build_codex_task`
- 사용자용 문서와 Codex용 YAML을 함께 컴파일할 때: `plan.compile_outputs`

이 기능들은 **Codex 프로세스를 실행하지 않는다.**
최종 YAML을 생성하고 품질·증거 게이트 결과를 반환한다.

### 4.2 기본 생성 순서

```text
1. plan.build_from_folder_request 또는 plan.synthesize_task_source
2. quality_gate와 repair_plan 확인
3. 필요한 경우 plan.refine_task_source 또는 plan.decompose_tasks
4. plan.build_codex_task, plan.generate_task_contracts 또는 plan.compile_outputs
5. final_output_ready == true 확인
6. 최종 Codex YAML 실제 경로 확인
7. 사용자에게 TaskSource와 최종 YAML 경로 전달
```

`final_output_ready == false`이면 원인을 보고하고 repair_plan을 적용해 작업지시서를 보강한다.
Codex 실행 도구가 없다는 이유로 `Blocked` 처리하지 않는다.

---

## 5. 브라우저 AI 세션 완료 판정

브라우저 AI의 **작업지시서 작성 단계**는 다음 조건을 모두 만족하면 완료다.

- TaskSource가 실제 경로에 존재한다.
- 최종 Codex YAML이 실제 경로에 존재한다.
- `quality_gate.passed == true`다.
- `evidence_gate.passed == true`다.
- `final_output_ready == true`다.
- 허용 변경 경로와 보호 범위가 명시되어 있다.
- 완료 조건, 실패 조건과 검증 방법이 명시되어 있다.
- 사용자의 명시적 요청 없는 commit·push·reset·checkout·stash 금지가 포함되어 있다.
- 사용자에게 TaskSource와 최종 YAML 경로를 전달했다.

이 단계의 완료 상태는 다음처럼 기록한다.

```text
Codex Work Order: Ready for External Codex
```

이 상태는 실제 코드 수정 완료를 의미하지 않는다.
별도 Codex 환경에서 작업지시서를 실행하기 전까지 코드 변경 상태는 `Not Started`다.

---

## 6. 별도 Codex 실행과 후속 검수

실제 Codex 실행은 다음 중 하나에서 이루어진다.

- 사용자가 여는 Codex 작업 세션
- 별도로 연결된 Codex 환경
- 향후 Codex 실행 도구가 제공되는 환경

브라우저 AI 세션은 Codex 실행기가 없다는 이유로 작업지시서 생성을 거부하지 않는다.
실행기가 현재 연결되어 있지 않으면 다음 상태로 종료한다.

```text
Codex Work Order: Ready for External Codex
External Codex Execution: Not Run
Source Changes: Not Applied by Browser AI
```

별도 Codex 실행 후 사용자가 검수를 요청하면 브라우저 AI는 다음을 확인한다.

```text
1. 실제 Git diff
2. 작업지시서의 허용 변경 범위 준수
3. 보호 범위 침범 여부
4. Codex가 보고한 빌드·자동 테스트 결과
5. 필요한 추가 빌드·테스트
6. 사용자가 수행해야 하는 PIE 결과
7. 대표 Plan, Systems와 ActiveWork 동기화 필요 여부
```

검수 미달 항목은 새 TaskSource 또는 보정 작업지시서로 만들어 다시 외부 Codex에 전달한다.

---

## 7. 대표 Plan 실행 출처 기록

대표 Plan 체크포인트에는 단계에 맞는 다음 필드를 기록한다.

```text
- 준비 방식: Plan/Codex Work Order / Browser Direct Edit (User Approved Exception) / Policy Violation
- TaskSource: <실제 경로 또는 Missing>
- 최종 Codex 작업지시서: <실제 YAML 경로 또는 Missing>
- 작업지시서 상태: Ready for External Codex / Quality Gate Failed / Missing
- 외부 Codex 실행 상태: Not Run / In Progress / PASS / FAIL / Unknown
- 브라우저 검수: Not Started / PASS / FAIL / Pending
- 빌드 상태: PASS / FAIL / Not Run / Unknown
- PIE 상태: PASS / FAIL / User Validation Pending / Not Applicable / Unknown
```

TaskSource와 최종 YAML 경로가 없으면 `Plan/Codex Work Order`로 기록하지 않는다.
사후에 문서를 만들어 원래 직접 수정이 Codex 작업이었던 것처럼 기록하지 않는다.

---

## 8. 작업지시서 생성 차단 기준

다음은 브라우저 AI의 작업지시서 생성이 실제로 차단되는 경우다.

- `plan.*` 기능 자체가 연결되어 있지 않거나 반복 실패한다.
- TaskSource의 필수 범위·대상 파일·완료 조건·검증 방법을 확정할 근거가 없다.
- 품질 게이트가 실패했고 repair_plan을 수행할 근거를 확보할 수 없다.
- 기존 dirty 변경과 충돌하여 안전한 대상 범위를 확정할 수 없다.
- 사용자 결정 없이는 구현 방향을 하나로 확정할 수 없다.

차단 상태는 다음처럼 기록한다.

```text
Blocked — Plan Work-Order Generation Unavailable
```

**Codex 실행기가 연결되어 있지 않은 상태는 차단 기준이 아니다.**

---

## 9. 직접 수정 예외

브라우저 AI의 직접 코드 수정은 기본 경로가 아니다.
다음 순서를 모두 만족해야 예외가 열린다.

```text
1. 브라우저 AI가 plan.* 작업지시서 생성이 불가능한 원인과 차단 범위를 사용자에게 보고
2. 직접 수정 시 위험, 대상 파일과 검증 방법을 설명
3. 사용자가 그 보고 이후 웹브라우저 AI의 직접 수정을 명시적으로 승인
4. 대표 Plan에 Browser Direct Edit (User Approved Exception)으로 실행 출처 기록
```

사용자의 일반적인 `수정해줘`, `구현해줘`, `계속해줘`는 직접 수정 승인으로 해석하지 않는다.
Codex 실행기 미연결만으로 직접 수정 예외를 요청하지 않는다.

---

## 10. 정책 위반 발견 시 처리

최종 Codex 작업지시서 없이 웹브라우저 AI가 코드를 직접 수정한 사실을 발견하면 다음 순서로 처리한다.

```text
1. 추가 직접 수정을 즉시 중단
2. 기존 변경을 임의로 되돌리거나 정리하지 않음
3. 대표 Plan에 Policy Violation과 Missing 작업지시서를 사실대로 기록
4. 실제 Git diff와 빌드·테스트·PIE 증거를 별도로 보존
5. 필요한 경우 현재 구현 검토·보정용 TaskSource와 최종 Codex YAML 생성
6. 새 작업지시서는 기존 직접 수정의 사후 출처가 아니라 별도 검토·보정 작업으로 기록
```

기능이 빌드와 PIE를 통과했더라도 실행 출처 위반은 별도의 `Workflow Compliance FAIL`로 기록한다.
기능 검증 PASS를 자동으로 취소하지 않는다.

---

## 11. Changelog

### v1.1 - 2026-07-16

- `plan.*` 기능을 Codex 실행기가 아니라 Codex용 TaskSource·최종 YAML 생성기로 명확히 정의.
- 브라우저 세션 완료 조건을 실제 코드 수정에서 `Ready for External Codex` 작업지시서 생성으로 변경.
- Codex 실행기 미연결을 정상 상태로 정의하고 차단 기준에서 제거.
- 작업지시서 생성 단계, 별도 Codex 실행 단계와 브라우저 후속 검수 단계를 분리.
- 대표 Plan 필드를 작업지시서 상태와 외부 Codex 실행 상태로 분리.
- 실제 차단 기준을 `plan.*` 사용 불가, 품질 게이트 미해결과 범위 확정 불가로 한정.

### v1.0 - 2026-07-16

- 웹브라우저 AI 코드 작업의 최우선 실행 게이트 최초 정의.
- 모든 새 세션에서 사용자 트리거 문구와 무관하게 적용하도록 명시.
- TaskSource와 Codex 계약을 코드 변경 전 필수 산출물로 지정.
- 대표 Plan에 실행 방식과 계약·검수 출처 필드 추가.
- Plan/Codex 사용 불가를 자동 직접 수정 예외에서 제거.
- 정책 위반 발견 시 중단, 사실 기록과 Codex 검토·보정 절차 추가.

---

## 12. Migration

- 기존 문서의 `Codex 실행 계약`은 `최종 Codex 작업지시서` 또는 `Codex 입력 YAML`로 해석한다.
- 기존 문서의 `Codex 실행`은 브라우저 세션 내부 필수 단계가 아니라 별도 Codex 환경의 후속 단계로 해석한다.
- 브라우저 세션은 최종 YAML 생성과 경로 전달 후 `Ready for External Codex`로 종료할 수 있다.
- Codex 실행기가 연결되지 않았다는 이유로 작업지시서 생성을 중단하지 않는다.
- 기존 코드와 미커밋 변경은 자동으로 되돌리지 않는다.
- 기존 구현 중 TaskSource와 최종 Codex 작업지시서가 없는 변경은 사후 위조하지 않고 `Policy Violation` 또는 `Provenance Missing`으로 기록한다.
- 문서 전용 변경은 기존 방식으로 계속 수행할 수 있다.
