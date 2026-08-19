# CarFight Code Work Gate

- 문서 버전: v2.7

- 작성일: 2026-07-16
- 최근 갱신일: 2026-08-19

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
+ AssetDump persisted/snapshot 조사
+ GoPyMCP UE MCP live Editor 조사와 Accepted-scope 기술 검증
+ 명시적으로 승인된 bounded UE mutation의 readback / rollback / cleanup
+ 빌드와 자동 테스트
+ Git diff 검수
+ 관련 문서 동기화

사용자
= 사람의 눈·손이 필요한 PIE 시각·UX·조작감·주행감 검증
+ 아직 Accepted되지 않았거나 현재 approval 경계를 넘는 UE 작업
+ user-owned/unknown Editor에서 자동화할 수 없는 수동 작업
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
Blueprint, DataAsset, Niagara, StaticMesh와 기타 바이너리 에셋은 더 이상 사용자 수동 편집만을 기본 경로로 두지 않는다. 저장된 상태 조사에는 AssetDump, current/unsaved/runtime truth와 Accepted scope의 기술 검증에는 GoPyMCP UE MCP를 우선 검토하고, 현재 공개 capability·approval·ownership 경계를 벗어나거나 사람 판단이 필요한 항목만 사용자 Editor 작업으로 남긴다.

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
8. 필요한 AssetDump/UE MCP 기술 검증을 먼저 수행한다. PIE-runtime 기술 사실은 Accepted `GoPyMCP.RuntimeRead`로 직접 관측 가능한지 우선 판정하고, 사람의 시각·감각 판단이나 현재 capability 밖의 사실만 사용자 PIE 또는 Editor 작업으로 남긴다.
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

### 7.1.1 Editor runtime ↔ build interference

공식 Editor build 또는 현재 Consumer가 로드한 plugin/module DLL의 link·replace가 필요한 경우, Editor process 존재만으로 종료를 결정하지 않고 **실제 build target 점유 여부**를 먼저 판정한다.

```text
Editor가 꺼져 있음 또는 target binary 충돌 없음
→ lifecycle 변경 없이 build

Editor가 target binary를 점유
→ lifetime ownership 판정

AI-owned
+ 사용자 작업 없음
+ AI mutation rollback/cleanup 완료
→ canonical save0 stop으로 conflict 해소 가능
→ build

user-owned / unknown / already_running
→ 자동 force/discard 금지
→ Blocked — User Editor Must Close
→ 사용자 종료 후 build 재개

build 뒤 새 binary의 live/PIE/UE MCP 검증 필요
→ 이전 volatile Editor evidence 폐기
→ fresh Editor start
→ Ready 확인
→ 별도 runtime/live validation
```

고정 규칙:

- Build가 Consumer-loaded binary와 독립된 경우에는 Editor를 편의상 종료하지 않는다.
- Build 성공은 새 Editor process나 live integration 성공을 의미하지 않으며, runtime/live PASS도 build PASS를 대신하지 않는다.
- Editor 종료가 build file lock 해소 목적이어도 §7.6의 ownership, save0, user-owned/unknown force 금지 규칙을 그대로 적용한다.
- Build 때문에 Editor lifetime이 바뀌면 이전 dirty, selection, PIE, open-asset, generation evidence를 새 lifetime으로 승계하지 않는다.

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

### 7.4 UE Asset 기술 검증과 사용자 검증 분리

UE 관련 검증은 **관측 가능한 기술 사실**과 **사람의 감각·시각 판단**을 분리한다.

기본 evidence routing:

```text
Persisted / Saved / Package / Snapshot truth
→ AssetDump 우선

Current Editor / World / Unsaved / PIE-runtime truth
→ GoPyMCP UE MCP 우선
→ World/LocalPlayer/Pawn/Component/property/snapshot/log 등 RuntimeRead 범위의 기술 사실은 Accepted `GoPyMCP.RuntimeRead`로 AI 직접 관측·판정

Approved bounded mutation
→ UE MCP write
→ immediate readback
→ rollback / cleanup / 필요한 persistent verification

시각 품질 / 조작감 / 주행감 / UX / 연출 감각
→ USER PIE / manual validation
```

AI Technical Validation으로 닫을 수 있는 대표 항목:

```text
- asset 존재와 class identity
- DataAsset/property 존재·형식·저장 상태
- Blueprint graph/variable 구조와 기술적 연결
- Actor/Component current state
- PIE의 current World/LocalPlayer/Pawn, component binding, runtime property/snapshot/log와 이들로 판정 가능한 상태 전이·데이터 전달·runtime invariant
- MaterialInstance / StaticMesh / Niagara의 허용된 기술 상태
- mutation readback, rollback, cleanup과 persistent contamination 여부
- runtime generation/session consistency와 bounded call outcome
```

사용자 검증으로 남겨야 하는 대표 항목:

```text
- 화면에서 자연스럽고 읽기 쉬운가
- 차량 조작감·주행감이 원하는가
- 조준감·타격감·사운드·연출이 적절한가
- 실제 플레이 UX가 의도와 맞는가
```

사용자 검증이 필요하면 다음을 제공한다.

```text
- 정확한 에셋과 메뉴 경로
- 한국어 UI명(English)
- 입력할 값
- PIE 절차
- PASS 기준
- 실패 결과별 점검 분기
```

기술적으로 확인 가능한 항목을 단순히 UE binary asset이라는 이유로 `User Validation Pending`으로 넘기지 않는다. PIE-runtime 기술 사실도 Accepted `GoPyMCP.RuntimeRead`로 관측 가능한 경우 USER에게 화면이나 값을 대신 읽어달라고 요청하지 않고 AI가 먼저 검증한다. 반대로 AI technical PASS를 시각·감각 품질 PASS로 확대하지 않는다.

RuntimeRead 사용 시 다음 경계를 유지한다.

```text
- RuntimeRead = read-only technical observation
- Editor/PIE lifecycle = 별도 lifecycle control plane
- write/destructive = 기존 approval 경계 유지
- 단순 관측을 위한 Product Debug HUD / Print / UE_LOG / 전용 getter / Consumer별 MCP operation 추가를 기본 경로로 삼지 않음
- unsupported fact가 있으면 임의 getter/function 실행이나 blind retry로 우회하지 않고 실제 관측 공백으로 기록
- 기존 Accepted RuntimeRead 정상 사용은 RR-00~10 replay 사유가 아님
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

### 7.6 Browser-managed Unreal Editor lifecycle

CarFight live Editor가 실제 prerequisite인 작업은 사용자에게 수동 실행을 요구하기 전에 Browser가 canonical lifecycle entry 사용 가능 여부를 확인한다.

```text
Editor 불필요
→ 시작하지 않음

Editor 필요 + 꺼져 있음 + lifecycle entry 사용 가능
→ Browser fixed start selector
→ Tools/RunEditor.ps1 → canonical Tools/RunEditor.bat 호출
→ exact CarFight_Re process 확인
→ exact process가 local 8100 listener를 직접 소유할 때까지 Ready 대기
→ `started + editor_ready=true + port_8100_owned_by_editor=true` terminal PASS

Editor가 작업 시작 전부터 실행 중 또는 start가 already_running
→ user-owned/unknown lifetime
→ 필요한 live 작업에는 사용 가능
→ Browser 자동 force/discard 종료 금지

Browser가 이번 작업에서 직접 시작한 것이 연속 evidence로 증명됨
→ AI-owned lifetime
→ 사용자 작업 없음
→ AI mutation rollback/cleanup 완료
→ save 0
→ 검증된 fixed AI-owned no-save stop을 direct 정상 종료 경로로 사용
→ known-unreliable 30초 graceful 대기를 의무 선행하지 않음
```

고정 규칙:

- Editor 시작 자체는 UE asset write, destructive action, save 또는 별도 live acceptance approval을 대신하지 않는다. 각 작업의 기존 승인 Gate는 그대로 적용한다.
- `UnrealEditor.exe` process 출현은 Ready evidence가 아니다. Browser fixed start는 exact project process의 8100 listener ownership까지 terminal success 조건으로 사용한다.
- lifecycle 자동화는 `Save`, `Save All` 또는 종료 직전 자동 저장을 수행하지 않는다. 자산 저장이 실제 작업 범위라면 lifecycle과 분리된 명시적 작업으로 다룬다.
- 사용자 작업이 존재하거나 lifetime ownership이 불명확하면 `DiscardUnsaved`, `Stop-Process -Force`, `taskkill /F`와 동등한 폐기 종료를 사용하지 않는다.
- Browser가 시작한 Editor라도 사용자 작업이 개입한 순간 ownership을 user-owned/unknown으로 승격하고 자동 폐기 권한을 잃는다.
- stop/restart/project reopen/level·asset reload 뒤에는 이전 dirty, selection, PIE, open-asset과 generation 기반 evidence를 현재 lifetime으로 승계하지 않는다.
- lifecycle actual은 최소 `started/already_running/stopped/stopped_discarded_unsaved`, 대상 project identity, save 요청 여부와 force 여부를 구분해 기록한다.

---

## 8. 완료 판정


직접 코드 작업은 다음 조건을 충족해야 완료로 기록한다.

```text
- 요청한 구현 범위가 실제 파일에 반영됨
- Git diff에서 변경 범위와 보호 범위 준수 확인
- 필수 빌드 또는 자동 테스트 결과 확인
- 실패한 검증과 실행하지 못한 검증을 구분
- AssetDump/UE MCP로 닫을 수 있는 technical validation은 먼저 수행하며, PIE-runtime은 Accepted `GoPyMCP.RuntimeRead`로 관측 가능한 기술 사실을 AI가 직접 닫은 뒤 실제 사람 판단이 필요할 때만 User Validation Pending으로 분리
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
UE Asset/Runtime Technical Validation: PASS / FAIL / Not Run / Not Applicable
PIE/User Validation: PASS / FAIL / User Validation Pending / Not Applicable
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
Blocked — User Editor Must Close

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

### v2.7 - 2026-08-19

- Accepted `GoPyMCP.RuntimeRead`를 CarFight PIE-runtime 기술 검증의 기본 관측 경로로 승격했다.
- World/LocalPlayer/Pawn/Component/property/snapshot/log와 이들로 판정 가능한 상태 전이·바인딩·데이터 전달은 USER에게 대신 읽어달라고 요청하기 전에 AI가 직접 검증하도록 했다.
- RuntimeRead technical PASS와 USER Visual/Feel PASS를 분리하고, 단순 관측 목적의 임시 Product debug surface·전용 getter·Consumer별 MCP operation 추가를 기본 경로에서 제외했다.
- RuntimeRead read-only, Editor/PIE lifecycle 분리, 기존 write/destructive approval, no blind retry와 Accepted RR replay 금지 경계를 고정했다.
- Migration: 기존 `USER PIE Pending`은 사람 판단이 필요한 항목인지 RuntimeRead로 닫을 수 있는 technical fact인지 먼저 재분류한다. 과거 USER PASS 기록 자체는 재해석하거나 삭제하지 않는다.

### v2.6 - 2026-08-17

- TC-00~09와 post-closure final verification으로 Accepted된 GoPyMCP UE MCP를 CarFight의 정식 기술 검증 수단으로 반영했다.
- `사용자 = Blueprint/DataAsset/Niagara/바이너리 에셋 작업`이라는 기존 기본 역할 구분을 제거하고, `Persisted→AssetDump / Live·Unsaved·Runtime→UE MCP / 시각·감각→USER` evidence routing으로 전환했다.
- Accepted scope의 bounded UE mutation은 기존 explicit approval/no-retry/rollback/cleanup 경계 안에서 AI가 기술적으로 수행·검증할 수 있게 하되 USER PIE의 시각·UX·조작감 판정과 분리했다.
- Migration: UE binary asset이라는 이유만으로 자동으로 사용자 작업으로 넘기지 않는다. 반대로 새로운 UE capability나 위험 write를 기존 Acceptance 범위로 추정하지 않으며, GoPyMCP current policy가 요구하는 새 validation/approval을 따른다.

### v2.5 - 2026-08-15

- §7.1.1에 Editor runtime ↔ build interference 판정을 추가해 process 존재와 실제 DLL/module file-lock을 분리했다.
- 충돌 없는 build는 Editor lifecycle을 바꾸지 않고, proven AI-owned 충돌만 기존 save0/cleanup 조건에서 정리할 수 있도록 했다.
- user-owned/unknown Editor가 required build target을 점유하면 `Blocked — User Editor Must Close`로 분리하고 자동 force/discard를 금지했다.
- Build 뒤 새 binary의 live/PIE/UE MCP 검증은 fresh start → Ready → 별도 runtime evidence로 수행하도록 했다.
- Migration: 기존 §7.6 readiness/ownership 규칙은 그대로 유지하며 build file-lock 해소도 같은 lifecycle 보호를 적용한다.

### v2.4 - 2026-08-15


- Browser lifecycle start의 terminal PASS를 exact CarFight_Re process + editor-owned local 8100 Ready로 강화했다. Process 출현만으로 live Ready를 주장하지 않는다.
- Ready 상태 actual에서도 `CloseMainWindow()` graceful stop이 30초 fail-closed한 evidence를 반영해, proven AI-owned + cleanup 완료 + 사용자 작업 부재 lifetime은 fixed no-save discard stop을 direct 정상 종료 경로로 사용하도록 교정했다.
- `already_running` 또는 pre-existing Editor는 user-owned/unknown으로 유지하며 force/discard 금지를 완화하지 않았다.
- Migration: 수동 `Tools/RunEditor.bat`와 일반 `Tools/RunEditor.ps1` process-only 기본 호출은 유지한다. Browser fixed preset만 readiness-aware semantics를 사용한다.

### v2.3 - 2026-08-14

- §7.6에 Browser-managed Unreal Editor lifecycle을 추가했다.
- live Editor가 필요할 때 `Tools/RunEditor.ps1`이 canonical `Tools/RunEditor.bat`를 호출하는 자동 시작 경로와 exact project/process prerequisite를 고정했다.
- 작업 시작 전부터 실행 중인 Editor를 user-owned/unknown으로 취급해 자동 종료하지 않도록 하고, Browser가 직접 시작한 AI-owned lifetime만 cleanup 완료 뒤 no-save 종료 대상으로 허용했다.
- lifecycle 자동 Save를 금지하고 강제 미저장 폐기는 AI-owned lifetime에만 허용했다.
- Editor stop/restart/reopen/reload 뒤 volatile dirty/selection/PIE/open-asset evidence를 무효화하도록 공통화했다.

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
- CarFight live Editor가 필요한 새 작업은 사용자 수동 실행을 기본 전제로 두지 않는다. canonical Browser lifecycle entry가 있으면 자동 시작을 우선하되 기존 UE write/save/destructive approval은 그대로 유지한다.
- 공식 Editor/plugin binary build는 active Editor가 실제 target binary를 점유하는지 먼저 확인한다. 충돌이 없으면 Editor를 종료하지 않고, 충돌이 있으면 AI-owned만 save0 조건에서 정리하며 user-owned/unknown은 `Blocked — User Editor Must Close`로 둔다.
- Build를 위해 lifetime을 종료한 뒤 새 binary를 live 검증할 때는 이전 volatile evidence를 재사용하지 않고 fresh start → Ready → validation으로 새 evidence를 만든다.

- 작업 시작 전에 이미 켜져 있던 Editor는 자동 종료하지 않는다. Browser가 이번 작업에서 시작한 AI-owned lifetime만 rollback/cleanup 완료와 사용자 작업 부재가 확인될 때 no-save 종료할 수 있다.
- Editor lifecycle 변화 뒤 volatile prerequisite evidence는 재사용하지 않는다.
- 저장된 asset/package 사실은 AssetDump를 기본 evidence source로, current Editor/unsaved/runtime 사실은 GoPyMCP UE MCP를 기본 evidence source로 사용한다. 둘을 습관적으로 중복 호출하지 않는다.
- 기존 Accepted UE MCP capability의 정상 프로젝트 사용은 완료 TC 또는 RuntimeRead RR-00~10을 다시 여는 사유가 아니다. 새 identity/write/destructive/getter-function/guard/retry/public-facade/lifetime 의미가 필요할 때만 GoPyMCP의 새 workflow-driven validation을 요구한다.
- 기존 `USER PIE Pending`은 먼저 `AI Runtime Technical Validation`과 `USER Visual/Feel Validation`으로 분해한다. Accepted RuntimeRead로 관측 가능한 사실은 사용자 미확인만을 이유로 Pending으로 유지하지 않는다.
```
