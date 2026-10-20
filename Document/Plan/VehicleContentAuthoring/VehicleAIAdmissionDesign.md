# CF-FQ-059 — VCCAS-P0-03C Trusted AI R2 Admission 상세 설계

- Version: v0.1.14
- Date: 2026-10-20
- Status: **G1 Registry/Schema + focused1/1 PASS / Company G2 managed trusted R2 single-pair Create + persisted Recipe→Definition PASS / R0 VehicleAIPreview focused1/1 PASS / G3 generic ue.write idempotent receipt PARTIAL / Core6 Preview6/6, actual Core6 Create0/6, P0-04 production SAVE NOT ACCEPTED**
- Owner: CarFight main_game / VehicleContentAuthoringPlan.md v0.1.54
- Scope: P0-03C의 **단일 실행 단위는** 새 Definition+Recipe 1쌍 R2 Create. 사용자가 승인한 차종별 N대 의도는 bounded N개의 단일 실행으로 분해할 수 있다. P0-04 영구 Save, Workbook, Catalog, Publish, P0-02 Core6 OEM Evidence는 제외.

## 1. 사용자 경험·권한 정책 (P0-01 교정)

**사용자가 "SUV 한 대 만들어"라고 지시한 작업을 동일한 이유로 반복 승인받는 UI를 만들지 않는다.** 한 사용자 지시의 의미는 작업 목적과 범위를 정하며, 기술적 쓰기 권한은 **현재 인증된 관리형 호출 주체의 실제 operation 권한**이 소유한다. 사용자 텍스트, AI 작성 JSON, CallerKind/Boolean/Hash 중 어떤 것도 단독 인증 증거는 아니다.

- 요청이 "SUV 한 대"이면 **신규 Definition+Recipe 1쌍만** 허용한다. 기존 자산 덮어쓰기·삭제·리네임·다른 프로젝트·임의 Save/Publish는 허용하지 않는다. 요청이 "차종별 한 대씩"이면 이미 확정된 차종 명부에서 대상 N개를 산출하고 **단일 쌍 N개의 별도 operation**으로 분해한다. 명부·N·대상/차종이 명확하지 않다면 R0 조사를 계속하고 최종 R2에만 명확한 실행 범위가 필요하다.
- 생성할 정확한 Mesh/프로필/차량 ID/패키지는 AI가 조사해 구체화한다. **최종 ProposalHash를 사용자 발화 시점에 이미 승인했다고 강제하지 않는다.** R0 Preview 후 Host가 허용한 operation 범위 안에서 CarFight가 fresh ProposalHash와 대상·참조를 검증한다.
- 기존 관리형 Host에 **실제 사용자 인증을 통과한 Project-scoped R2 호출 권한이 있다면 그것을 재사용**한다. 별도 "사용자 발화 원문 인증서", 메시 ID, 서명 서버, 독립 승인 DB 또는 반복 확인 창을 선결조건으로 만들지 않는다. 반대로 운영 권한이 없는 Host 경로를 발화 텍스트만으로 합법화하지 않는다.
- 범위 밖 기존 Asset 변경이 필요한 경우에만 원래 지시를 확장하는 사용자 결정이 필요하다. P0-03C에는 이러한 overwrite/delete route 자체를 추가하지 않는다.
- 단계 구분: R0 생성안은 누구에게나 허용된 기존 읽기 경로; R2 신규 1쌍 생성은 관리형 쓰기 범위가 실제로 검증될 때만 수행. P0-03C 결과는 **미저장·미게시**이며 P0-04 저장/Workbook/게시와 분리한다.

## 2. 현행 소스 계약

| 기존 구성 | 확인된 현재 동작 | 유지할 계약 |
| --- | --- | --- |
| `FCFVehicleAuthoringService::PreviewAIVehicleRecords`, `CFVehicleUXOps.cpp v1.7.0` | AI Caller로 신규 두 record의 path/mesh/collision/proposal hash 읽기 전용 검사; UE5.8 Builder 빌드 PASS | mutation0, 이것만으로 생성 승인 아님 |
| `PreviewVehicleRecords/CreateVehicleRecords` | SlateUI/Automation caller만 허용. R2 Create는 fresh preview, `bOwnershipWriteApproved`, `ApprovalScopeHash` 검사 후 Definition/Recipe를 한 transaction으로 생성 | 기존 UI/Automation 실행·호출자 제한 유지; AI를 Automation으로 위장 금지 |
| `FCFVehicleRecordCreateRequest` | 정확한 Definition/Recipe package/object, optional Chassis, Profile, Transmission/Hardpoint 옵션. operation ID 없음 | 기존 request 시그니처/호출자 유지. AI 전용 호출 컨텍스트를 별도 설계 |
| `FCFAuthoringCallContext` | ClientOperationId·ApprovalClass·ScopeHash는 있지만 외부 사용자 인증 증거는 아님 | 필요한 진단 값 참고만 하고 신뢰 인증이라고 주장하지 않음 |
| `FCFVehicleBuilderVM::BuildDefaultVehicleRecordIdentity` | canonical Authoring root와 DA_Vehicle_/DA_Recipe_ 작명 | 명명·VehicleId 검증 중복 구현 금지 |
| `FCFVehicleCCASProvider` | P0-03A/B read/diff/readback, reviewed mutation·apply는 차단 | 이 Provider를 새로운 원시 UObject writer로 만들지 않음 |

기존 생성 이름 규칙(소스 확인):
- Definition = `/Game/CarFight/Data/Authoring/DA_Vehicle_<VehicleId>`
- Recipe = `/Game/CarFight/Data/Authoring/DA_Recipe_<VehicleId>`
- C-1 구현부터 VehicleId 검증·기본 4-field identity의 단일 소유자는 `FCFVehicleRecordIdentity`다. 기존 `FCFVehicleBuilderVM::ValidateVehicleCreationId` 및 `BuildDefaultVehicleRecordIdentity`는 같은 helper를 위임하는 호환 wrapper다. `PreviewAIVehicleRecords`는 **AI일 때 canonical 4-field exact 경로를 Preview 전에 검사**한다. SlateUI/Automation의 기존 Advanced 경로는 제한하지 않는다. 허용 root는 `/Game/CarFight/Data/Authoring/`이며 실제 R2 Host admission은 아직 닫혀 있다.

## 3. 신뢰 경계·진입점 계약 (P0-01/P0-02/P0-03 교정)

```text
사용자: "SUV 한 대 만들어"                 사용자 작업 의도
           ↓
현재 인증된 관리형 Project/UE 호출 주체   ← 기술적 쓰기 권한 소유
  - 현재 caller의 허용 operation (R2 새 차량 1쌍)
  - carfight / main_game / 현재 exact Checkout / Ready Editor lifetime
  - 허용 Asset root 및 대상 수량·operation 식별
           ↓  [검증된 내부 invocation만; AI 입력으로 재구성 금지]
CarFight Vehicle AI Domain Gate
  - AI 작성 exact VehicleCreateInput
  - 기존 PreviewAIVehicleRecords / canonical VehicleId 규칙
  - target path · collision · 참조/타입 검증 · fresh proposal hash
           ↓
기존 Vehicle Authoring의 단일 Definition+Recipe writer
           ↓
신규 1쌍 / unsaved / unpublished / 정확한 readback
```

### 3.1 외부 구현 계약을 추측하지 않는다

**실제로 확인된 사실**: CarFight `.gopymcp/project.json`의 `unreal` capability는 `editor.build`, `editor.start/stop`, `ue.read`, PIE lifecycle을 등록한다. 이 매니페스트에는 P0-03C R2 vehicle-create operation이 없다. **그 사실만으로 모든 UE write endpoint가 부재하다고 단정하지 않는다.** 현행 GoPyMCP/UE Bridge 공개 write와 서버 내부 operation dispatch의 실제 경계는 후속 읽기 전용 조사 대상이다.

**필수 확인 목록(새 API 설계보다 우선)**: 실제 caller가 R2 쓰기에 어떤 방식으로 권한을 얻는가; Project/Checkout/Runtime은 caller 지정값과 서버 확인값 중 무엇인가; 공개 write tool이 어떠한 operation을 실행할 수 있는가; `CreateVehicleRecords`를 `CallerKind=SlateUI/Automation` 및 Boolean/Hash 위조로 간접 호출할 수 있는가; 실행 결과 중복/응답 분실 복구 contract는 무엇인가. GoPyMCP 장애/코드 수리는 별도 세션.

**권한 최소 계약**: 실제 관리형 진입점이 verified current caller, `carfight/main_game`, exact Checkout + Runtime lifetime, `CreateNewVehicleRecords` / `OwnershipWrite` / R2, 신규 package root, 최대 생성 쌍 수(단일 실행 1), 실행 식별을 서버측으로 검증한다. 불가하면 **AI R2 실행 차단**하고 구체적 계약 gap만 기록한다. 구현자가 `trusted=true`, 임의 토큰, `apply_approved=true`, 사용자 텍스트를 독립적인 authority로 취급하지 않는다.

### 3.2 CarFight / Host 소유 경계

| 소유자 | 책임 | 하지 않는 일 |
| --- | --- | --- |
| 기존 Host/관리형 실행 | 인증된 실행 주체 및 실제 호출 권한, 정확한 Project/Checkout/Runtime 선택, P0-03C R2 operation 노출/차단, 동일 실행의 상태·중복/응답 분실 handling | VehicleData 필드/메시/프로필 의미를 결정하지 않음 |
| CarFight | 승인받은 신규 차량 업무 범위(1쌍/root/ID), 타입/참조/path/중복 검증, R0 Preview와 생성 직전 해시 재검사, 기존 단일 writer·생성 결과 검사 | 새 인증 서버 구축, Host owner 행세, 새로운 raw writer/Save 구현 |
| 실제 사용자 제작 지시 | 만들 차종/대수와 허용한 목표 범위를 결정 | 기술적 서명·정확한 ProposalHash를 미리 제공해야 하는 필수 절차 아님 |

**공개 우회 차단**: 기존 `CreateVehicleRecords`의 `SlateUI/Automation` 및 `bOwnershipWriteApproved` 경로는 C++ 내부 호환 경로다. `CallerKind`는 진단용이므로 외부 caller가 이 값을 제공해 그 함수를 호출할 수 있다면 R2 admission은 안전하지 않다. 실제 외부 dispatcher에서 내부 Legacy 경로를 공개하지 않아야 하며, 신규 AI R2 호출은 **검증된 Host 내부 경계에서만** 연결한다. 형식만 갖춘 C++ request를 받는 public "trusted" overload 금지.

## 4. 입출력 및 스코프 검증 (P1-01/P1-03 교정)

다음 이름은 설명용 **논리 계약**이며 현재 공개 MCP tool이나 C++ 타입으로 실재한다고 주장하지 않는다. 실제 인자는 C-0에서 확인한 Host/Bridge 계약을 기준으로 정한다.

### 4.1 AI 작성 데이터: `VehicleCreateInput` (비신뢰)

- `VehicleId`(기존 ASCII 영문·숫자·underscore만 허용), `DefinitionPackageName/AssetName`, `RecipePackageName/AssetName`, optional `ChassisMesh`, existing five `ProfileBindings`, `bRequireVehicleSpecificTransmission`, `bRequireExplicitHardpointPlan`.
- **C-1 현재 구현**: `FCFVehicleRecordIdentity::ValidateVehicleId/BuildDefaultIdentity/ValidateCanonicalRequest`가 실제 Builder 기본 이름 규칙을 단일 소유한다. Builder의 기존 두 인스턴스 메서드는 이 helper를 호출하는 wrapper가 되었으며, AI Preview는 `ValidateCanonicalRequest`로 정확한 경로·Definition/Recipe ID 결속을 검사한다. 기존 UI Advanced explicit path override는 변경하지 않았다. C++ helper가 생겼다고 R2 Host 인증이 허용되는 것은 아니다.
- 정상 AI R2 root는 `/Game/CarFight/Data/Authoring/` 하위이고 package 이름의 최종 component = object asset name이어야 한다. `/Game/` 전체 허용은 **기존 일반 Preview의 검증 의미**이지 새로운 AI R2 허용 범위가 아니다.
- AI가 보내는 `CallerKind`, `ApprovalScopeHash`, `bOwnershipWriteApproved`, `AuthorizedUserIntentRef`, 자유 기입 `ProjectId/CheckoutId/RuntimeInstanceId`는 **승인 판정의 근거에서 제외**. 현재 AI caller 진단은 신뢰된 adapter에서 AI로 강제하며 실제 authority는 관리형 dispatcher가 결정.

### 4.2 Host 내부에서 검증되어야 할 실행 범위(미확정 인터페이스)

- **실제 서버가 검증한** Project=`carfight`, Role=`main_game`, exact registered Checkout, Ready Editor runtime lifetime 및 해당 managed caller의 쓰기 capability.
- `Operation=CreateNewVehicleRecords`, `Risk=R2`, `ApprovalClass=OwnershipWrite`, `AllowedRoot=/Game/CarFight/Data/Authoring/`, `MaximumNewPairCount=1`(호출당), `ClientOperationId`의 재호출 정책.
- 사용자 지시·차종/대수의 범위는 상위 작업 맥락으로만 사용한다. **최종 hash 자체에 사용자 지시 원문 또는 Host가 새로 발행할 UserIntentRef가 반드시 포함되어야 한다는 강제조건은 폐기**. 대신 실행에 허용된 operation·자산 범위가 Host의 실제 인가 정책에 부합해야 한다.
- Host가 `ProposalHash`를 직접 전달/결속할 수 있다면 그대로 사용하고, 아니면 허용된 operation 안에서 CarFight가 **Preview → mutation 직전 fresh Preview** 결과를 동일 요청으로 비교한다. 사용자/AI 자가 선언 해시만으로 R2 허용 불가. Host가 승인 시점에 최종 해시를 반드시 미리 알아야 한다고 요구하지 않는다.
- 운영 중 caller scope가 실제로 제공되지 않으면 **Host 계약 조사에 BLOCKED**. CarFight가 신뢰 증거 객체를 새로 발급하지 않는다.

### 4.3 ProposalHash가 증명하는 것과 증명하지 않는 것

`CFVehicleUXOps.cpp::BuildRecordCreateHash`에서 현재 hash는 Definition/Recipe object path, Chassis soft object path, 5 Profile soft object path, 두 creation 정책 flag와 Resolver contract revision으로 구성된다. **Mesh/Profile 내용 및 패키지의 dirty/version/semantic fingerprint는 포함되지 않는다.**

- P0-03C는 두 record의 신규 package identity와 **명시적 soft reference binding**을 만든다. AI Preview와 commit 사이 exact 경로·옵션의 변화는 거부해야 한다. 두 참조의 타입·존재/로드 유효성은 실제 사용되는 범위에서 기존 작성·Resolver 계약을 확인한 뒤 실행 직전에 검사한다. **지정한 Profile의 내용까지 고정/승인된 것처럼 주장하지 않는다.**
- 참조 자산이 Editor에서 변경되어도 동일 path hash가 유지될 수 있다. 따라서 내용·성능·OEM 수치·사용자 Socket 변경의 **완전한 stale 검증 및 실차 검수는 P0-04의 fresh source fingerprints/authoring review**에서 수행한다. P0-03C의 미저장 후보 생성 성공을 P0-04 실제 차량 품질/정확성 PASS로 확대 금지.
- 기존 파일·메모리 패키지가 있거나 다른 세션의 unsaved target을 가리키면 R2 생성은 거부한다. 기존 Chassis/Profile을 **참조만 한다는 이유로 자동 편집·저장하거나 user-owned dirty를 정리하지 않는다.** 의존 자산의 unsaved 상태가 P0-04 source readback에 영향을 주면 그 단계에서 별도로 차단/검토한다.

### 4.4 결과 상태

기존 `FCFVehicleRecordCreateResult` / `FCFAuthoringOpResult`를 우선 재사용한다. 성공하면 exact Definition/Recipe object paths, 새로 생성된 `RecipeId`와 Target binding, changed package 2, `bCreatedAssets=true`, `bPackageDirty=true`, `bSavePerformed=false`, `bAutomaticRetryPerformed=false`를 실제 readback으로 확인한다.

실패 시 현재 실제 enum `UnsupportedOperation`, `ApprovalRequired`, `ApprovalScopeMismatch`, `StateChanged`, `OperationIdConflict`, `InvalidSemanticInput`, `FailedUnknownState`(`Status`) 등의 **실존하는 오류/상태만** 사용하거나 Host 오류와 구분한다. 임의 `RecoveryRequired`를 기존 `ECFAuthoringErrorCode`의 값이라고 주장하지 않는다. `vehicle_<RecipeId>` ContentKey는 **성공 후 RecipeId readback**으로만 확정하며 P0-04 durable authoring은 별도다.

## 5. 실행 단위·수정 순서 (P0-02/P1-02 교정)

**구현 순서는 실제 Host/UE 쓰기 진입점 확인이 먼저다.** 아직 외부 계약이 미확정인데 `CFVehicleUXOps.cpp` writer를 미리 리팩터링할 필요는 없다. 불필요한 영구 ID/토큰 저장소·신규 범용 Tool 등록·권한 관리 UI를 만들지 않는다.

| 순서 | 작업 | 관측 가능한 완료 조건 |
| --- | --- | --- |
| **C-0 신뢰 진입점 조사** | 회사 `CarFightMCP_Admin`의 exact Project/Checkout/Runtime 및 기존 관리형 UE write/Project operation의 **실제 exposed schema·인가·실행 결과·재시도 규칙**을 읽기 전용 확인. caller spoof 레거시 우회 가능성도 확인. 독립 GoPyMCP 장애 수리 X | 가능한 bounded R2 entry 또는 구체적 missing contract(소유 GoPyMCP) 확정. 아직 없다면 R2 구현 Gate HOLD |
| **C-1 도메인 범위 확정** | 공통 helper `FCFVehicleRecordIdentity` 구현 및 Builder 기존 함수 위임, AI canonical 생성안 R0 검사 추가. AI 신규 pair는 정확한 root/4-field identity, 원본 참조 변경0, 기존 자산 collision0. 실제 Host R2 수량 인증은 C-0 의존 | **source + official UE5.8 Build PASS**; focused Automation NOT RUN이므로 전체 기술 Acceptance 보류 |
| **C-2 안전한 기존 writer 연결** | 실제 C-0에서 안전한 호출 경계가 확인된 뒤에만 필요 최소 범위로 기존 `CreateVehicleRecords` 본문을 단일 private 생성 primitive로 추출. UI/Automation의 현재 check 순서·승인/해시·결과/Undo 의미는 그대로 두고, 별도 trusted AI route는 **Host가 이미 검증한 내부 실행**만 사용. 기존 `NewObject` 코드 복제 X | AI를 Automation으로 위장하지 않고 생성 primitive가 하나, 외부 legacy spoof 차단 |
| **C-3 실행 식별/실패 복구** | 동일 `ClientOperationId`에 대한 retry/조회는 **Host 관리형 실행 기록/응답 계약**을 우선 사용. CarFight는 exact package collision/re-preview와 실제 성공·부분실패 readback을 보조. Host가 관련 기록을 보장하지 않으면 무작정 자동 재호출/중복 실행 금지 | 응답 분실/중복/동시성/불명확 상태를 실제 Host·도메인 상태와 일치하게 표현 |
| **C-4 집중 검증** | Build, 기존 Builder/CCAS 회귀 및 아래 T-01~T-14 중 실제 구현 범위 테스트. 증거는 current source+terminal output+JSON readback으로 수집 | source test vs actual trusted Host R2 통합 PASS를 분리 기록 |

**C-0 미확정에서 가능한 작업**: R0 `VehicleAIPreview` focused Automation, 기존 identity·기존 UI 회귀 조사, 읽기 전용 typed domain contract 점검. **C-2 R2 writer 분리·AI 실제 Create 개방은 유보**한다. C-0 확인 후 실제 단순한 기존 path가 있다면 adapter 없이 최소 범위로 연결할 수 있는지 먼저 검토한다.

### 정상 R2 기술 알고리즘 (Host gate 확인 이후)

1. AI 생성안의 exact VehicleId/패키지/optional Chassis/5 Profile/두 정책을 R0 `PreviewAIVehicleRecords`로 검사한다.
2. 관리형 호출 주체가 현재 caller에 허용된 프로젝트·Checkout·Runtime·R2 `CreateNewVehicleRecords` 및 root/수량을 **자체 인증 자료와 운영 정책**으로 확인한다. 사용자에게 같은 승인 버튼을 재요구하지 않는다.
3. CarFight가 AI 진단 identity를 AI로 고정하고, canonical path·기존 객체 없음·참조 타입·대상 1쌍을 검사한다. `CallerKind=SlateUI/Automation`, 승인 Boolean/Hash 등 외부 입력으로는 admission이 열리지 않는다.
4. 동일 request로 쓰기 직전 fresh R0 Preview를 재실행해 **현재 path/옵션 hash가 초기 preview와 exact 동일**한지 검사한다. 내용까지 불변이라고 해석하지 않는다. 대상 패키지 충돌 재검사.
5. 실제 Host 권한이 확인된 내부 경계만 공통 writer를 호출한다. 기존 UI/Automation public facade는 종전 조건을 먼저 거친다. **Editor 게임 스레드에서 한 번** 실행하며 package-scope 경합을 허용하지 않는다.
6. 생성 직후 Definition/Recipe class/object path/Target binding/새 RecipeId/Dirty/Save0/다른 패키지 변경0을 읽는다. 새로운 Product 등록·저장은 없다.
7. Host가 terminal execution 결과를 관리한다. 동일 OperationId 재요청 때 이미 확정된 결과는 읽어 재전송하고 **새 mutation을 재실행하지 않는다**. 불명확하면 현재 Editor 상태를 먼저 읽어 출처를 확인하며, runtime이 소실된 경우 저장 안 된 객체가 살아 있다고 가정하지 않는다.

**부분 실패**: `FScopedTransaction::Cancel`이 UObject 생성을 모두 원자적으로 되돌린다고 가정하지 않는다. 신규 패키지 일부가 메모리에 남은 경우 정확한 상태를 `FailedUnknownState`와 실제 mutation footprint로 보고하고 동일 경로 자동 Undo/delete/save/retry 금지. 더 강한 원자적 rollback은 실제 실험/테스트로 보장되는 경우에만 별도 판단한다.

## 6. 테스트 및 판정 매트릭스 (P0/P1 교정 반영)

| ID | 검증 | 기대 / 실행 구간 |
| --- | --- | --- |
| T-01 | 같은 요청 AI vs UI Preview | Path/ProposalHash 같고 생성·Save 0; **현재 R0** |
| T-02 | AI Preview에 Automation Caller | 거부; **현재 R0** |
| T-03 | AI 직접 기존 Create에 Boolean/Hash 채움; 또는 외부 exposed tool의 SlateUI/Automation 위조 | AI direct Create 거부, Host 공개 경로 spoof 차단. Host 실제 노출 조사 전 PASS 불가 |
| T-04 | Host 권한 부족·인가 값 자가 주장 | R2 거부, 생성 0; **C-0/C-2** |
| T-05 | 다른 Project/Checkout/Runtime 또는 종료된 Editor lifetime | R2 거부; **C-0/C-2** |
| T-06 | AI canonical root 밖·잘못된 VehicleId·N쌍 초과·같은 package | 거부, mutation0; **C-1/C-2** |
| T-07 | R0 뒤 경로/Chassis reference/Profile reference/정책 변경 | fresh hash mismatch로 R2 차단; **C-2** |
| T-08 | 기존 memory/disk package·다른 작업 소유 Dirty target collision | overwrite 0; 참조 Mesh/Profile 자체는 mutation0; **C-2** |
| T-09 | 같은 operation ID 재전송/동시 경합/응답 분실 | writer 재실행 0, Host receipt 혹은 unknown/blocked. **C-0/C-3** |
| T-10 | 실제 인증된 관리형 Host에서 신규 1쌍 | Definition+Recipe 1쌍, RecipeId/Target binding readback; **외부 통합 Gate** |
| T-11 | 정상 R2 직후 persistence·비대상 검사 | Save0/Workbook0/Catalog0/기존 Asset 변경0. Profile payload OEM 값 적합성은 P0-04 |
| T-12 | 생성 중 실패, 1개 package만 남음, runtime 소실 | FailedUnknownState/실제 footprint, 자동 delete/retry0; **C-2/C-3** |
| T-13 | 기존 SlateUI/Automation MeshCreate | 현재 Approval/Hash/Undo/출력 동작 회귀 없음 |
| T-14 | VehicleSchema/VehicleReadDiff/WagonPersistedReadback 등 직접 영향 CCAS | 기존 P0-03A/B PASS baseline 보존; 실제 새 영향 발생 시 집중 회귀 |

**테스트 방식**: 기존 `Tools/RunDataAuthoringTests.ps1 -ExactTestNames`를 사용하며 증거 JSON과 이전 세션 테스트 기록의 overwrite 영향은 확인한다. UE Editor 실험은 테스트 전용 독립 경로·현재 runtime ownership을 확인한 후에만 수행한다. `VehicleAIPreview` 이전 빌드 PASS는 **Automation PASS가 아니다**. T-10/11은 synthetic authority를 임의로 꾸며 PASS로 판정하지 않는다.

## 7. 재검수·완료 판정 (v0.1.1)

**2026-10-16 실제 C++/현재 매니페스트·문서 재검수: CarFight 내부 설계 및 실행 순서 PASS, 외부 Host R2 permission/operation 계약은 미확인으로 C-0 HOLD.** P0-01/02/03, P1-01/02/03 교정 사항 6/6 문서 반영과 T-01~14 매핑을 readback으로 확인했다. 설계 검수 PASS를 신규 C++ compile/UE Automation/실제 R2 Create PASS로 해석하지 않는다.

- **현재 설계 검수 범위**: CarFight P0-03C 도메인 구조·기존 C++·관리형 진입 계약에 맞는 후속 구현 순서와 안전한 차단 기준이 모순 없는지. GoPyMCP가 실제 지원한다고 확인되지 않은 필드를 의무 API로 발명하지 않는다.
- **설계 수락의 경계**: P0-03C CarFight 내부 설계는 source-grounded 범위로 수락할 수 있으나, 정상 **AI R2 실행 PASS는 C-0에서 기존 Host bounded write/authorization 계약을 검증하고 T-10/T-11을 실행한 뒤에만** 판정한다.
- **C-1 이행·검증 기록**: `FCFVehicleRecordIdentity.h/.cpp v1.0.0` 신규, `CFVehicleBuilderVM.cpp v1.39.3` wrapper, `CFVehicleUXOps.cpp v1.7.0` AI canonical R0 guard, `CFVehicleAIAdmissionTests.cpp v1.1.0` negative/compatibility tests. 공식 UE 5.8 Source Editor Build `c35c72f769de422d81e8bb86fdab4148` **exit0 PASS**. 2026-10-17 관리형 `process.run`의 **Legacy repository_id=`main_game`** 계약을 재사용해 `RunDataAuthoringTests.ps1 v1.3.2 -EvidenceId`로 `VehicleAIPreview`/`VBCUX_P0_03.Naming` 각각 **exit0, 성공 1/1 + 1/1, 실패 0, 격리 JSON readback 성공**. Job IDs `a76cd6ac18e74f5db09afd1a06785355`, `bd6fce11d6db4e5ab91d464d0a5c8de4`; JSON SHA256 `ae83f147819cd22ea5790214f9b4943da4b06928f9fe3fb7bd7f603117a013f5`, `bb7067e8be346cbafbe99dec883c29626efa83ffa4d685db1116baaa4abccb07`. 이전 2026-10-16 `carfight_repo` 사용 실패는 Legacy 실행 ID를 Project Registry repository ID와 혼동한 호출 오류이며, **GoPyMCP 실행 기능 고장을 증명하는 결과가 아니었음**. R2 Host authority/C-2/3 통합 검증은 여전히 HOLD.
- **독립 진행 가능**: 기존 증거를 안전하게 보존할 수 있는 분리된 집중 Automation 및 Builder identity 회귀 readback, 현재 code review. 실제 Host contract 미확정이면 C-2 R2 writer 연결을 앞당기지 않는다.
- **외부 의존 1개**: 기존 관리형 tool의 정확한 AI write caller/operation authority·bound invocation/receipt·spoof 차단 여부. 이 질문은 GoPyMCP owner 영역의 계약 확인 항목이며, 이번 CarFight 세션에서는 장애 수리·인증 새 기능 강요·GoPyMCP 소스 수정 없음.
- **전체 CF-FQ-059 분리(2026-10-16 당시 검수)**: P0-02 Core6 OEM Evidence HOLD, P0-03A/B 기존 Technical PASS, P0-03C AI R0 source+Build PASS와 당시 신규 Automation NOT RUN, P0-04 durable Save/Workbook/Catalog/Publish NOT OPENED 보존. 2026-10-17 이후의 C-1 focused 2/2 PASS 및 C-0 공개 계약 BLOCKED는 아래 §7.1과 최신 대표 Plan이 우선한다.

### 7.1 C-0 실제 Host 공개 계약 판정 (2026-10-17)

- Company `CarFightMCP_Admin`에서 `carfight/main_game` exact Checkout을 확인했으며, 현재 `.gopymcp/project.json`에는 관리형 차량 생성 R2 operation이 등록되지 않았다. UE Bridge 자체는 connected이나 프로젝트·Checkout·Ready Runtime lifetime과 결속된 `OwnershipWrite` 인가를 증명한 것은 아니다.
- 공개 `CarFightMCP_UE_ReadOnly`/`CarFightMCP_UE_WriteProbe`의 toolset **각 53개** 확인 결과 일반 `DataAssetTools.create` 및 `ue_call_write`는 있어도 `FCFVehicleAuthoringService::CreateVehicleRecords` 전용 신뢰 dispatcher는 확인되지 않았다. 단순 `apply_approved` Boolean, 외부 작성 `CallerKind=SlateUI/Automation`, `ProposalHash`는 신뢰된 호출 권한이 아니다.
- 실제 외부 dispatcher에서 spoof가 성공한다는 실측 증거는 없다. 반대로 arbitrary 호출이 모든 경로에서 차단된다는 검증도 없다. 범용 UE `ue_call_write`에는 `ClientOperationId` 기반 차량 Create terminal receipt/응답분실 재조회·부분실패 계약이 공개되지 않았다. `process.run` dedupe는 별도 계약이므로 R2 보증으로 전용하지 않는다.
- **C-0 조사 완료 / 후속 Gate BLOCKED:** GoPyMCP owner가 현재 Host Project-scoped trusted R2 operation·caller authority·exact runtime·단일 1쌍/root·spoof 제한·receipt 계약을 증명하거나, 정말 없다면 그 부분만 별도 작업에서 구현해야 한다. CarFight C-2/3 소스 mutation, T-10/11 실제 생성, 추가 승인 UI·토큰·승인 DB 신규 구현은 보류한다. P0-03A/B 및 C-1 과거 PASS 유지. 상세 읽기 증거는 대표 Plan v0.1.36 §13.12.16에서 소유한다.

### 7.2 C-2/3 CarFight 기존 코드·호출 경계 검수 및 독립 작업 분리 (2026-10-17)

**판정: PRE-IMPLEMENTATION READ-ONLY REVIEW COMPLETE / C-2/3 R2 구현 BLOCKED.** 현재 C++ 구현·실제 호출자·기존 집중 테스트를 재확인한 결과 아래와 같이 분리한다.

| 현행 경계·증거 | 현재 코드 확인 | 실행 판단 |
| --- | --- | --- |
| `CFVehicleRecordIdentity.cpp` | VehicleId와 Definition/Recipe 기본 4-field identity를 단일 구현, AI root `/Game/CarFight/Data/Authoring/` canonical 검사 | C-1 구현 보존, 중복 helper 불필요 |
| `CFVehicleUXOps.cpp::BuildVehicleRecordPreview` | 신규 package의 memory/disk 충돌, Definition/Recipe package 분리, 명시 Chassis StaticMesh 로드, path·reference·policy ProposalHash 검사. Profile 5종은 hash에 soft path로 결속되나 이 자리에서 전체 Profile 타입/내용을 검증하지 않음 | R0 typed/domain 음성 테스트 독립 보강 후보. P0-04 OEM/reference fingerprint 검사와 혼합하지 않음 |
| `PreviewAIVehicleRecords` | `CallerKind=AI` 검사 → canonical identity 검사 → 공통 mutation0 preview | 독립적으로 검수·음성 회귀 가능. Host 인증 증거는 아님 |
| `PreviewVehicleRecords` / `CreateVehicleRecords` | SlateUI/Automation만 허용. Create가 다시 `PreviewVehicleRecords` 호출 → `bOwnershipWriteApproved`/fresh `ApprovalScopeHash` 검사 → `FScopedTransaction` 안에서 정확히 Definition·Recipe `NewObject` | 기존 함수의 caller/approval/hash 순서 보존. Host 계약 전에 AI 허용 또는 private writer 추출 금지 |
| `CFVehicleP11VM.cpp`, `CFVehicleBuilderVM.cpp` | UI VM이 승인 전 request를 준비하고 사용 시 SlateUI, approval/hash를 설정. Builder는 VM의 기존 생성 경로를 재사용하며 생성 뒤 별도 Browser adoption 수행 | UI/Automation 호환 회귀 대상. AI 실행 권한의 대체 경로가 아님 |
| `CFVehicleUXTypes.h` | Record Create request에는 `ClientOperationId` 없음; `CFVehicleAIContract.h`의 공통 `FCFAuthoringCallContext`에는 ID·상태/재생 필드가 있지만 이 Create request의 실행 영속·중복 증빙은 아님 | R2 도메인/Host operation contract 확정 뒤 정확한 경계만 추가. 기존 request 전체에 임의 idempotency 저장소 선도입 금지 |
| `CFVehicleAIAdmissionTests.cpp` | AI vs UI Preview Hash, 위조된 AI 직접 Create 차단, canonical root/identity 실패, 생성·저장 0 중심 | C-1 집중 2/2 과거 PASS 보존; T-03 외부 spoof/T-10·T-11 실제 Host 수락까지 확대 금지 |
| `CFVehicleAuthoringVMTests.cpp` | Automation의 실제 기존 writer 성공, Target binding, Basic Sensor·profile inference0·Dirty·Save0 검사 | 기존 UI/Automation 단일 writer 회귀를 독립 검증할 수 있음. 테스트 자체의 에셋 mutation/cleanup을 감안해 격리 evidence로만 실행 |

**외부 계약 없이 안전하게 가능한 다음 범위**: (1) 현재 R0 입력 검사의 부족한 음성 케이스를 기존 `VehicleAIPreview` 테스트에 additive 보강할지 실제 기존 coverage와 대조, (2) 기존 UI/Automation two-record 생성 회귀의 정확한 test selector와 에셋 격리/clean-up 경계를 확인, (3) C-2/3의 필요한 readback fields와 T-01/02/06/07/08/13 테스트 매핑을 문서·테스트 소스 수준에서 확정. 이 세 범위에서 새로운 AI Create, 신규 에셋 생성, 기존 로직 리팩터링은 요구하지 않는다. 기존 C-1 focused 2/2는 이미 PASS이므로 동등 테스트 반복은 하지 않는다.

**Host 계약 이후에만 개방**: `CreateVehicleRecords`를 재사용할 신뢰된 내부 호출 경계, non-spoofable AI caller·`OwnershipWrite` operation, exact Project/Checkout/Ready Runtime, 허용 root/단일 신규 쌍, same OperationId terminal receipt/unknown handling, 호출·실행 단위의 동시성 제어, 정상 Host T-10/11. Host 외부 입력에서 임의 `CallerKind=Automation/SlateUI`와 Boolean/Hash를 채워 현재 Create를 호출하는 방안은 금지한다.

**정확한 실패 한계**: `FScopedTransaction::Cancel`은 두 package/object 생성의 원자적 Rollback을 증명하지 않는다. Create 코드에는 성공 후 별도의 두-object class/path/RecipeId/readback terminal receipt도 없다. 두 객체 중 일부만 생성된 뒤 실패했다면 실제 footprint를 읽어 unknown/partial로 판정하고 자동 Undo/delete/Save/retry는 금지한다. `FCFAuthoringOpResult`의 dedupe 관련 필드 존재만으로 이 Create의 operation replay가 구현됐다고 주장하지 않는다.

**검증/변경 범위**: CarFight actual source + 기존 Automation test + 대표 Plan/Systems + UDS head 검수만 수행. 새 UE5.8 build·Automation·실제 R2 생성 NOT RUN. P0-03A/B 및 C-1 과거 PASS 보존; C-2/3 C++ 변경0, UE Asset/Workbook/Save/Catalog/Publish0, GoPyMCP 수정0. UDS state=`Ready`/phase=`VCCAS-P0-03C` 유지하므로 lifecycle record/projection 변경하지 않음.

### 7.3 C-2/3 Host-independent R0 음성 회귀 최소 보강 (2026-10-18)

**변경:** `UE/Source/CarFight_ReEditor/Private/DataAuthoring/CFVehicleAIAdmissionTests.cpp v1.2.0`의 기존 exact `CarFight.CCAS.CF_FQ_059.P0_03C.VehicleAIPreview` 테스트 하나에만 아래 누락 사례를 추가했다. Production Service, Request schema, UI/Builder, Runtime·GoPyMCP, UE Content Asset 및 데이터 파일은 변경하지 않았다.

- **Unsaved collision:** 고유 VehicleId로 canonical 신규 대상 Definition 이름을 만들고 Save하지 않은 빈 `UPackage`를 테스트 메모리에만 생성한다. 기존 Preview가 `StateChanged`로 생성안을 차단하고 Recipe를 만들지 않는지 확인한 후 test-only package dirty flag를 false로 복원하고 garbage 처리한다. 영구 Asset 생성/Save 0.
- **Wrong Chassis type:** `GetTransientPackage()`에 만든 비-StaticMesh `UObject`를 Chassis soft object path로 지정해 기존 Preview가 `WrongAssetType`으로 거부하는지 검사하며 fixture는 garbage 처리한다. 임의 기존 Mesh/Profile을 수정하지 않는다.
- **Hardpoint intent hash:** 신규 차량 요청의 `bRequireExplicitHardpointPlan` 한 필드만 true로 변경해 새 Preview의 ProposalHash가 원본과 달라지는지 검사한다. 기존 Transmission policy hash 테스트와 구분된다.

**빌드 실측:** Company `CarFightMCP_Admin build.run` legacy `repository_id=main_game`, preset `carfight.editor.development`, build job `41bacc7d43b3476d8f083f6ef25db4b8`. 공식 UE5.8 Source Build에서 `[1/5] CFVehicleAIAdmissionTests.cpp`와 `[2/5] CFVehicleBuilderVM.cpp` 컴파일 진행, 이후 Link 시 실행 중인 `D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe`가 `UE/Binaries/Win64/UnrealEditor-CarFight_ReEditor.dll` 파일을 점유. UBA 및 NoUba 재시도 모두 DLL 기록 실패(LNK1104), job **FAILED exit code 6**. 테스트 코드 자체의 C++ compiler error는 관측되지 않았으나 **완전 빌드 PASS 아님**. 이 DLL의 owner/unsaved 상태 미확인 상태에서는 Editor 자동 종료·강제 중지를 금지한다.

**Automation:** 변경된 DLL 로드 여부를 보장할 수 없으므로 v1.2.0 focused test는 **NOT RUN**, 신규 regression PASS를 선언하지 않는다. 기존 2026-10-17 C-1 v1.1.0 2/2 PASS는 historical evidence로 유지하며 v1.2.0 증거로 대체하지 않는다. 추후 current Editor의 안전한 DLL release가 확인된 뒤 **동일 기존 공식 빌드 완료 → `Tools/RunDataAuthoringTests.ps1 -ExactTestNames CarFight.CCAS.CF_FQ_059.P0_03C.VehicleAIPreview -SuppressModelContextProtocolLog -EvidenceId <new_unique_id>` 1건만** 격리 실행하고 terminal JSON 읽기로 성공/실패/누락/중복을 판정한다. 이미 존재하는 EvidenceId로 재실행하지 않는다.

**보호·의존성:** Trusted AI Create와 Host 인증·OperationId·R2 writer는 여전히 BLOCKED, UI/Automation Create 변경0. UDS Work `Ready`/`VCCAS-P0-03C` 유지; 별도 lifecycle mutation 없음. Git stage/commit/push 없음.

### 7.4 T-13 기존 차량 생성 회귀의 실제 테스트 선택자·격리 계약 검사 (2026-10-19)

**읽기 전용 코드 검수 완료.** C-2/3에서 기존 `CreateVehicleRecords`를 공유할 때 보호해야 할 CarFight UI/Builder의 현재 회귀 소유자를 `CFVehicleAuthoringVMTests.cpp` 등록 선언과 `RunTest` 본문에서 직접 확인했다. 이번 단계에서 빌드·Editor 종료·Automation 실행은 하지 않았다.

| 정확한 기존 Automation 선택자 | 현재 확인된 검사 | 미검증 범위 |
| --- | --- | --- |
| `CarFight.DataAuthoring.DAUTH_P0_11.FrozenUX.MeshCreate` (`FCFVehicleP11MeshCreateTest`) | SlateUI/Automation과 공통 Preview→승인 Hash→`CreateVehicleRecords` 단일 writer; 생성 Definition·Recipe/Target 결속, Chassis 참조, Basic Sensor, Profile inference0, Transmission·Hardpoint Legacy policy, package Dirty, Save0, 자동 Retry0; 이어서 Guided Builder 선택·고유 identity·정책 검사 | trusted AI Host 호출 권한·OperationId·실제 외부 replay 아님. 이 테스트의 모든 중간 실패 경로에 대한 완전한 package cleanup 보증 및 Unreal Undo 실제 재생 검증을 의미하지 않음 |
| `CarFight.DataAuthoring.CF_FQ_042.VBCUX_P0_02.RecordCreation` (`FCFVBCUXP002RecordCreationTest`) | Blank Start와 이미 사용 중인 Chassis Mesh 재사용으로 두 차량 생성, `ExecutePreparedNewVehicleRecordCreate`·Builder adoption·fresh Browser row, 원본 VehicleData의 Chassis 참조 보존, VehicleSpecificRequired Transmission·Unspecified Hardpoint, Save0 | production Core6 신규 차량 저작/영구 Save/Workbook/Publish 아님. 입력 출처·정책 승인 Runtime 범위 아님 |

**격리 관련 중요한 사실:** 두 테스트는 `FGuid` 기반 고유 `/Game/CarFight/Tests/...` 패키지에 Editor 메모리 UObject를 만들고 `CleanupRegisteredAsset`으로 `AssetDeleted`/`RF_Public|RF_Standalone` 제거/`MarkAsGarbage`를 호출한다. 그러나 해당 공통 cleanup helper 자체에는 **UPackage dirty flag를 false로 되돌리거나 package를 메모리에서 완전히 제거하는 보장이 없다**. `CreateVehicleRecords`는 실제로 신규 두 package를 Dirty로 표시한다. 따라서 이 테스트들의 소스가 Save0를 단언한다는 이유만으로 사용자 편집 중인 **공유 Editor에서 안전하게 재생할 수 있다**고 판정하지 않는다. 검증은 기존 격리된 `RunDataAuthoringTests.ps1 -ExactTestNames`의 fresh unattended Editor process에서만 수행하고 테스트 증거 출력은 새 `-EvidenceId`로 분리한다. 기존 실행 기록을 덮어쓰거나 user-owned Editor를 테스트 때문에 자동 종료하지 않는다.

**확정된 최소 선택:** C-2/3 AI R2 writer 코드가 아직 바뀌지 않았으므로 기존 두 regression 테스트를 지금 즉시 재실행하지 않는다. 신뢰된 내부 호출 경계가 실제로 구현되어 기존 writer/UI/Builder에 영향이 생길 때 T-13 focused set으로 두 테스트를 선택하고, 새 `VehicleAIPreview v1.2.0` 음성 회귀와 별도로 증거를 수집한다. 현재 소스 검수를 실행 PASS 또는 T-13 전체 수락으로 간주하지 않는다. 코드·Asset·Workbook 변경 없음, UDS Ready/P0-03C 유지.

### 7.5 C-0 UE MCP 프로젝트 Toolset 등록·관리형 R2 호출 권한 집중 실측 (2026-10-19)

**판정: UE 등록·발견 경로 PARTIAL PASS / 현재 CarFight Create operation NONE / 관리형 새 R2 Toolset 동적 허용·trusted caller 전달·receipt 계약 UNVERIFIED.** CarFight가 Generic Host를 프로젝트별 로직으로 교체해야 한다는 결론은 아니다. 본 절의 사실은 Company CarFightMCP_Admin + exact Ready Project Runtime `project_id=carfight`, `capability_id=unreal`, `runtime_instance_id=runtime_50d8c839234241c0b9ed3d4d3ebc59e0` 에서 읽기 전용으로 실측했다.

1. **Editor MCP Toolset registry 확인**: `ue_list_toolsets(max_items=100)`에서 53개 이름·설명이 정상 열거된다. 이 중 `SlateInspectorToolset.SlateInspectorToolset` 설명은 **`UToolsetRegistry`에 등록되어 `ModelContextProtocol` 플러그인이 자동으로 탐지**한다고 명시한다. 또한 `ue_describe_toolset`이 각 exposed tool의 상세 schema/Toolset fingerprint/generation을 반환한다. 이는 UE 내부 프로젝트/플러그인 작성 Toolset의 **발견 메커니즘이 존재함**을 직접 지지하지만, 아직 CarFight 사용자 정의 Toolset의 실제 C++ 등록·컴파일 시험, 플러그인 자체 등록 API/모듈 의존성이 readback된 것은 아니다.
2. **현재 CarFight 등록 상태**: `.uproject`에 `ModelContextProtocol`, `MCPClientToolset`, `AllToolsets` enabled. `CarFight_ReEditor.Build.cs`에는 해당 plugin의 direct dependency 없음. UE exposed 53개에 CarFight 전용 `CreateVehicleRecords`/`TrustedVehicleCreate` Toolset은 **없음**. `.gopymcp/project.json`의 CarFight `unreal` Project operations는 editor.build/start/stop, ue.read, PIE start/stop만 등록되어 있으며 전용 vehicle-create R2 operation은 없음. 다만 manifest에 없다는 사실만으로 **모든** UE write endpoint 부재를 주장하지 않는다.
3. **등록 ≠ 서버 호출 허용**: 동일 Ready Runtime에서 허용된 `editor_toolset.toolsets.asset.AssetTools.find_assets` R0를 직접 호출해 기존 Recipe4(Wagon/TestSedan/TestSUV/DefenseSUV)를 정상 재조회했다. 반면 toolset에 명시적으로 존재하는 `PluginToolset.PluginToolset.ListEnabledPlugins` / `ListDiscoveredPlugins`, `ToolsetRegistry.AgentSkillToolset.ListSkills`는 **읽기 이름의 도구인데도 `ERR_UE_TOOL_BLOCKED`**였다. `ue_describe_toolset` 성공/53개 이름에 존재하는 것/읽기 이름은 `ue_call_read` 또는 `ue_call_write` 실행 권한을 보장하지 않는다.
4. **R2 capability 구조 구분**: 외부 `ue_call_write`는 별도의 `apply_approved` 인자와 Project/Capability/Runtime selectors를 가지며, 정책·Runtime 조정·dirty 보호를 서버가 소유하는 기존 generic Host surface이다. 그러나 `apply_approved=true`나 selector를 호출자/CarFight C++ 신원 증거로 재해석할 수 없고, CarFight 전용 신규 Toolset 메서드에 해당 policy가 **동적으로/정적으로 허용**되는지, 실제 authenticated Project/Checkout/Runtime/R2 권한이 Toolset 내부에 어떤 비위조 형태로 전달되는지, 실행 job/동일 OperationId terminal receipt 및 응답 분실 후 readback이 어떻게 보장되는지는 아직 증거가 없다. 임의 Toolset 하나를 추가하고 곧바로 R2 admission PASS라고 주장하지 않는다.
5. **확장 가능성 결론**: `UToolsetRegistry`는 UE side discovery의 유력한 기존 공식 경로이지만, 인증된 managed write dispatch를 담보하지 않는다. `CarFight_ReEditor`와 Engine 플러그인 사이의 실제 registry registration API, build/module dependency, runtime 재발견 시점/Editor reload와 compatibility는 이 저장소의 repo_search(UE/Plugins 하위 UToolsetRegistry/ToolsetRegistry/ModelContextProtocol 등록 소스 **0 hit**)만으로는 확정되지 않았다. 따라서 **Toolset 작성 가능성은 구조적 확인**, 프로젝트 CarFight custom Toolset compile/register 성공 **NOT RUN**, Host write admission **BLOCKED — exact contract missing**으로 분리한다.

**정확한 GoPyMCP Host 소유 확인 3항목 (읽기 전용, GoPyMCP 범용 계약만):**

- **G1. 등록·노출:** 기존 `UToolsetRegistry` 기반 프로젝트 Editor 모듈 Toolset을 추가하면 `ue_list_toolsets/ue_describe_toolset`에 반영되는가? 필요한 plugin module dependency, Server allowlist/refresh/restart owner 절차는 무엇인가? `ModelContextProtocol` 일반 registrar 기능만 검수하며 GoPyMCP에 CarFight 업무 로직을 넣지 않는다.
- **G2. 권한:** 인증된 `project_id=carfight` / `repository_role=main_game` / exact registered Checkout / Ready Runtime을 서버가 검증하고 신규 Toolset의 **하나의 `CreateNewVehicleRecords` R2 Write 메서드**를 `ue_call_write`를 통해 허용하는 실제 policy 확장 계약이 존재하는가? Host는 caller-supplied `trusted`, `CallerKind`, `bOwnershipWriteApproved`, hash를 **인증 신원**으로 취급하지 않는가? Caller 선택 가능한 JSON 인자로는 권한이 위조되지 않아야 한다.
- **G3. 재전송 결과:** 서버가 이미 실행한 단일 Editor write의 invocation/job/request identity를 잃지 않고 조회할 수 있는가? 응답 분실/같은 OperationId/다른 request 재사용/부분 생성 unknown에서 중복 재실행 없이 현재 UE 에셋 두 쌍의 상태를 재조회할 권한/증거가 무엇인가? 기존 generic job/receipt가 재사용 가능하면 새 원장·DB를 추가하지 않는다.

**범위 결정**: G1+G2+G3가 현재 Host 표면으로 충족되면 CarFight는 Editor-only 얇은 Toolset Adapter 하나와 `FCFVehicleAuthoringService`의 기존 단일 two-record 생성 primitive 재사용에만 국소 구현을 한정한다. UI/Automation 경로의 fresh preview, approval, hash, transaction, Save0·Undo 계약을 보존하며 AI를 Automation/SlateUI로 위장하지 않는다. **새 host authorization 서버, 새로운 도메인 Writer, 임의 Python/Automation production mutation, 독자 approval token/중복 실행 DB, GoPyMCP 내부 CarFight 코드 금지.** 반대로 Host 신규 Toolset 정책·receipt가 미제공이면 범용 Host 소유 질문만 별도 전달하고 CarFight C-2/3 구현 선행/CarFight Production Save를 유보한다. P0-04 Core6 R0 6/6 Preview PASS, UE5.8 Build PASS와 이미 준비된 12 asset names는 보존한다.

**이번 작업 증거 범위**: Repo/Plan/manifest/UE runtime `status+list+describe+read call` 실제 read only. 신규 Custom Toolset 컴파일/등록·R2 tool write/Editor 재시작·에셋 Create/Save·GoPyMCP 수정 **NOT RUN**. UDS Ready/VCCAS-P0-03C 유지, 기존 dirty 보호, Git stage/commit/push0.

### 7.6 GoPyMCP G1~G3 독립 조사 수령·CarFight C-0 결정 정합화 (2026-10-20)

**근거 범위:** 별도 GoPyMCP 세션의 읽기 전용 조사 보고서(Company `CarFightMCP_Admin`, GoPyMCP `main`, HEAD `437c08b00ffa760a1d57e6ab85a85c5ddd828a30`, source/policy/runtime 조사, GoPyMCP Git 변경0) 내용을 CarFight 실제 `CFVehicleUXOps.cpp`의 `PreviewAIVehicleRecords`, `CreateVehicleRecords` 구현 및 현재 Plan §7.5와 비교했다. **이 절은 전달받은 Host 조사 결과를 CarFight 계약 의존성으로 기록한 것이며, 이 CarFight 세션에서 GoPyMCP 소스를 직접 재조회하거나 수정했다고 주장하지 않는다.** CarFight 소스에는 변경 없음.

| 구분 | GoPyMCP 실제 보고 판정 | 확인된 기존 재사용 계약 | 잔여 검증 / 최소 변경 소유권 |
| --- | --- | --- | --- |
| G1 — 프로젝트 Toolset 등록/발견 | **PARTIAL** | `ue.list_toolsets → UEBridgeFacade → LiveToolSearchAdapter → UESessionManager.invoke_read`, 현재53개 실측, `describe_toolset` 신규 조회 및 generation-bound page cache. `UToolsetRegistry` 프로젝트 등록은 UE 쪽 발견 원리로만 확인 | **CarFight/UE 플러그인 공급자:** 등록 실제 C++ 헤더/API·Build.cs 모듈·모듈 unload/reload/Editor restart 절차 확인. GoPyMCP 신규 등록 API 도입 증거 없음 |
| G2 — 신규 Toolset R2 쓰기 | **BLOCKED** | Gateway `ue.write` Scope, signed request context, `authorize_ue_tool` exact `(toolset_name,tool_name)` policy + `apply_approved`, ProjectUEWriteRouter Project/Capability/Runtime/Ready Editor/Coordinator Epoch/EditorWrite Lease/fresh runtime 재검증, 단일 `ue.call_write`, audit·AI_PROTECTED_DIRTY | **GoPyMCP:** `Config/mcp_ue_policy.json`에 정확한 등록 메서드의 쓰기 정책 추가가 최소 후보이며 Bridge 정책은 시작 시 로드하므로 반영 시 별도 승인된 갱신 필요. **CarFight/Host:** generic `ue.write`와 도메인 `OwnershipWrite` 간 비위조 신뢰 전달 계약은 **미증명**. 사용자 작성 caller/hash/bool 또는 direct localhost Toolset 호출을 인증으로 간주 금지. 실제 ChatGPT 앱 ue.write Scope 소유 여부도 **미검증** |
| G3 — 실행 상태·중복·응답 유실 | **PARTIAL** | `UEBridgeFacade.run_ue_call_write` audit `started/succeeded/failed/outcome_unknown/blocked`; `UESessionManager.invoke_write` **단일 invoke**, 전송 후 불명확 시 `ERR_UE_CALL_OUTCOME_UNKNOWN, outcome_known=false`; Request ID/Arguments SHA audit. `AssetTools.exists/find_assets` 읽기 사용 가능 | 범용 `ue.call_write`에 caller-stable OperationId, 멱등 replay, 공개 `ue.write.status`, process.status 재사용 계약 **없음/확인되지 않음**. **CarFight:** 대상 두 asset의 deterministic path + exact class/target/recipe identity readback으로 중복·부분·불명확 상태 최소 처리. 동일 command 자동 재전송 금지; 두 asset 존재만으로 요청 성공 주장 금지 |

**기존 Writer 대조:** CarFight `CreateVehicleRecords`는 먼저 `PreviewVehicleRecords`(caller `SlateUI/Automation`)로 fresh candidate를 확인한 뒤 `bOwnershipWriteApproved`, `ApprovalScopeHash` 일치 확인 → `FScopedTransaction` → `UCFVehicleData`와 `UCFVehicleRecipeData` 1쌍 생성 / Recipe Target binding / AssetRegistry 등록 / Dirty / `bSavePerformed=false`. `PreviewAIVehicleRecords`는 AI R0 mutation0이며 AI가 `CallerKind=Automation`을 대신 기입해 Create 진입하는 것은 정상 R2가 아님. 이 기존 사실은 위 보고서와 충돌하지 않는다.

**우선순위 및 인계 범위:**

1. **G1-C++(CarFight/설치된 UE 공급자)**: 현재 UE5.8 `UToolsetRegistry`의 실존 등록 API·필수 모듈·발견/재시작 계약을 최소 조회. 섣불리 별도 plugin 또는 거대한 adapter를 만들지 않음.
2. **G2-POLICY(GoPyMCP 범용)**: 등록할 정확한 `(toolset,tool)`에 필요한 현행 allowlist 분류 및 정책 적용 방식과 ChatGPT 연결의 실제 `ue.write` Scope 검사. 도메인 코드를 GoPyMCP에 넣지 않음.
3. **G2-R2(교차 경계, 가장 중요한 결정)**: UE 내부 Toolset이 서버에서 인증한 실행 요청과 직접 localhost 호출을 **구별할 수 있는지** 기존 범용 계약을 먼저 실측. `apply_approved`, request의 `CallerKind`, approval Boolean/Hash만으로 도메인 R2를 통과시키지 않는다. 기존 증거 전달 계약이 없다면 독자 서버/토큰이 아니라 **최소 범용 Host 신뢰 연결 계약** 필요 여부만 소유분리하여 결정. 신뢰 연결이 확인되기 전 실제 Writer Adapter commit **BLOCKED**.
4. **G3-DOMAIN(기존 CarFight Writer 소비자)**: 부정·성공·unknown 응답에서 deterministic Definition+Recipe path/RecipeId/Target/class를 조회하는 단일 도메인 결과 판정. `none/none`도 `outcome_unknown` 전달 이후에는 결과 미확정일 수 있어 무조건 재호출하지 않는다. `one/both`는 partial/conflict vs already-complete를 구분하고 명시적 사용자 후속 승인/새 원장 인프라 없이 기존 실행 기록·직접 자산 readback을 우선 쓴다.
5. **구현 승격 기준:** C-2 기존 `CreateVehicleRecords` 공통 private 생성 부분 재사용/Toolset Adapter 설계는 G1 실제 API와 G2 신뢰 경계가 확정된 후만. G3는 기존 UE 감사·에셋 조회로 가능한 최소 bounded 처리부터 적용. GoPyMCP에 신규 generic R2 실행 프레임워크, CarFight 전용 GoPyMCP router, 고정 프로덕션 저장/자동 복구 루프, 속임수 Automation mutation을 도입하지 않는다.

**현재 수락:** C-0 Host 소스·정책 조사 보고서 **접수/경계 분류 완료**, G1 PARTIAL / G2 BLOCKED / G3 PARTIAL. Core6 기존 6/6 R0 Preview PASS 보존, R2 Create/Save/Companion **0/6**, T-10/11 실제 trusted AI 실행 **NOT RUN**. 이번 CarFight 작업에서는 Code/UE Asset/GoPyMCP/Build/Automation/Git Commit·Push 없음. P0-03C R2 admission **NOT ACCEPTED**.

### 7.7 G1 UE 5.8 프로젝트 Toolset 등록 설계검수 (2026-10-20)

**결론: DESIGN PASS WITH LOCAL API/LIFECYCLE CHECK REQUIRED.** Epic UE 5.8 공식 Unreal MCP 및 ToolsetRegistry API 문서를 직접 확인해, 기존 엔진 Toolset 기능 재사용·CarFight Editor 모듈 소유·R0 발견 검증 우선의 설계를 승인한다. **현 설치된 D:\UnrealEngine_Source 플러그인 실제 소스/빌드/런타임 등록의 확인은 아직 NOT RUN**. G2 관리형 R2 승인/정책과 G3 UE write terminal/retry는 GoPyMCP/공유 경계 별도 소유, 본 G1 설계검수에 끼워넣지 않는다.

**공식 UE 5.8 등록 API 증거:**
- Unreal MCP 공식 문서: https://dev.epicgames.com/documentation/unreal-engine/unreal-mcp-in-unreal-editor?lang=ko (MCP Toolset의 권장 경로는 `UToolsetDefinition` + C++ `UCLASS(BlueprintType, Hidden)`, static `UFUNCTION(meta=(AICallable))`; 등록 후 `ModelContextProtocol.RefreshTools`; **새 UFUNCTION 선언은 Live Coding 반영이 아니라 Editor 재시작**).
- UE5.8 API `UToolsetRegistry`: https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/ToolsetRegistry/UToolsetRegistry — `#include "ToolsetRegistry/UToolsetRegistry.h"`, `IsAvailable`, `IsToolsetClassRegistered`, `RegisterToolsetClass(TSubclassOf<UToolsetDefinition>)`, `UnregisterToolsetClass`; Engine experimental plugin의 **ToolsetRegistry** 모듈.
- UE5.8 API `UToolsetDefinition`: https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/ToolsetRegistry/UToolsetDefinition — `#include "ToolsetRegistry/ToolsetDefinition.h"`, 공통 Toolset UObject 기저, static `AICallable`과 비공개 함수의 `AIIgnore` 계약.
- UE5.8 API `UToolsetRegistrySubsystem`: https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/ToolsetRegistry/UToolsetRegistrySubsystem — `UEditorSubsystem`, 초기화/해제 수명과 `Get` helper의 존재는 확인. 단, **CarFight 설치된 플러그인에서 실제 등록 타이밍·auto discovery 대상 여부·반환 실패 처리는 source 확인 전에는 가정하지 않는다**.

**CarFight 현재 소스 대조:**
- `UE/CarFight_Re.uproject`에 `ModelContextProtocol/MCPClientToolset/AllToolsets` enabled. 의존 ToolsetRegistry가 현재 등록된 53 Toolset과 공식 plugin chain의 등록 원천이지만 이 .uproject에 별도의 `ToolsetRegistry` 항목을 추가해야 한다는 근거는 없다.
- `UE/Source/CarFight_ReEditor/CarFight_ReEditor.Build.cs v1.10.5`에는 `ToolsetRegistry` direct dependency가 아직 없다. G1 C++ 구현이 필요해지면 **Editor-only PrivateDependencyModuleNames**에 `ToolsetRegistry` 1개만 추가하는 것을 1순위로 검토한다. `CarFight_Re` Runtime 모듈 또는 `ModelContextProtocol`에 무의미한 직접 의존성을 더하지 않는다.
- 기존 실제 Editor module owner는 `UE/Source/CarFight_ReEditor/Private/CarFightReEditor.cpp` 의 `FCarFightReEditorModule::StartupModule/ShutdownModule` 및 대응 `Public/CarFightReEditor.h`이다. 탭/메뉴 등록과 기존 cleanup이 이미 있으며 **다른 독립 module/프로젝트 plugin을 새로 만들 필요는 없다**.
- 신규 등록 예정 단일 Editor Toolset은 제안 타입 `UCFVehicleAuthoringToolset` (정확한 C++ 파일/클래스는 설계 후보, 새 구현 없음)이며 `Private/DataAuthoring` 소유. G1 단계의 노출은 **mutation0 R0 조회/등록 검증만**, `CreateVehicleRecords` 실제 R2 메서드 `AICallable` 공개는 G2 신뢰 호출 증명 전 금지.

**수명·등록 경계 교정:**
1. 현 UE5.8 engine 설치 소스에서 `ToolsetRegistrySubsystem::Initialize`, `UToolsetRegistry::RegisterToolsetClass`, `ModelContextProtocolEditor` discovery/refresh 동작을 확인하여 **자동 클래스 발견만으로 충분한지 vs Editor 모듈 명시적 Register가 필요한지** 하나를 선택한다. 공식 가이드의 startup auto discovery 문장과 실제 installed subsystem 초기화 순서가 다를 수 있으므로 양쪽을 동시에 무조건 강제하지 않는다.
2. 명시 등록이 필요하다면 Registry available 이후 1회만 `RegisterToolsetClass`한다. `StartupModule`에서 `IsAvailable`이 false일 때 즉시 실패/무한 polling하거나 억지로 Registry 생성하지 않는다. Engine 초기화 완료 후 안전한 한 번의 callback/등록 경계를 실제 공급자 소스 기준으로 선택한다. 이미 등록된 클래스에 대한 중복 재등록은 방지한다.
3. Editor module 종료 시 명시 등록한 클래스만 `UnregisterToolsetClass`하고 등록을 위한 delegate가 있다면 해제한다. 기존 Slate/Tab lifecycle을 변경하지 않는다. 신규 Toolset `UFUNCTION` 추가 시 공식 UE 문서상 완전한 Editor 재시작 필요. 함수 본문만 변경했다면 Live Coding을 고려할 수 있으나 등록/발견 스키마 갱신은 `ModelContextProtocol.RefreshTools`과 신규 `ue.list_toolsets/describe_toolset`로 실제 readback한다. Editor 재시작은 사용자/Runtime ownership 승인 경계에 따른 별도 실행이다.
4. Unreal MCP 기본 localhost 호출은 서버의 GoPyMCP OAuth/R2 권한을 대체하지 않는다. G1 Toolset에서 향후 쓰기 함수가 `AICallable`로 공개되면 직접 localhost 호출도 가능할 수 있으므로 **G2 Host 인증과 도메인 OwnershipWrite 신뢰 경계가 증명되기 전 쓰기 메서드를 Toolset에 등록하지 않는다**. 단순 Host allowlist 등록을 C++ 내부 호출 권한의 근거로 간주하지 않는다.

**G1 최소 검증 및 완료 기준 (G2 분리):**
- **R1, 공급자 실제 코드:** exact Engine 5.8 설치 소스에서 헤더/모듈/registry 등록·언로드 수명 확인. 문서로만 유추한 자동등록은 DESIGN이지 설치 증거가 아니다.
- **R2, 좁은 빌드:** 기존 Editor 모듈에만 G1 Toolset 읽기 진입·필요 의존성을 추가해 CarFight 공식 `Tools/BuildEditor.bat` 경로로 UE5.8 컴파일/링크. `UFUNCTION(AICallable)`은 기존 읽기 전용 함수/정보만 래핑하고 생성/저장·기존 Writer 리팩터링은 건드리지 않는다.
- **R3, 발견/readback:** 소유권이 확인된 Editor 재시작 또는 Registry refresh 후 `ue.list_toolsets`에서 추가 Toolset 1개 발견, `ue.describe_toolset`에서 **정확히 의도한 R0 메서드**의 명세 확인, Registry 중복 이름 없음. R0 서버 호출 정책으로 실행이 별도 차단될 수 있으며 **도구 목록/스키마 발견 PASS를 G2 call admission으로 승격하지 않는다**.
- **R4, 안정성:** 모듈 재로드·Editor 재실행 시 누락/중복 없이 등록되며 종료 후 stale handler 미존재. 기존 53 Toolset이 사라지지 않고 기존 CarFight Builder UI가 유지됨을 확인. Startup-only 변경의 경우 R2 Create/Save 실제 테스트는 G1 수락에 요구하지 않는다.

**설계 검수 최종 판정:** Registry **공식 API / Toolset 작성 경로 / Editor module owner / 동적 Refresh 절차 확인**. `CarFight_ReEditor` 단일 Editor 모듈 재사용·R0 단위 등록 우선이라는 설계는 **PASS**. **실제 설치 Engine 등록 타이밍 및 구현·빌드·live Discovery 검증 NOT RUN**이므로 G1 구현 기술 PASS는 아직 아니다. G2 POLICY/R2 및 G3 결과 복구는 이 작업에 합치지 않는다. 이번 작업은 문서만 수정하고 Engine/GoPyMCP/CarFight C++/UE asset/Editor runtime/Git staging/commit/push는 변경하지 않았다.

### 7.8 G1 Toolset R0 C++ 구현·실제 빌드 결과 (2026-10-20)

**판정: SOURCE IMPLEMENTED / UHT+C++ COMPILE PASS / FULL DLL LINK FAIL (EDITOR LOCK) / LIVE TOOLSET DISCOVERY & FOCUSED AUTOMATION NOT RUN / G2 R2 UNOPENED.**

**G1 실제 변경:**
- 새 Editor-only `Private/DataAuthoring/CFVehicleAuthoringToolset.h/.cpp v1.0.0`: `UCFVehicleAuthoringToolset : UToolsetDefinition` 및 `static UFUNCTION(meta=(AICallable)) PreviewVehicleRecordIdentity(VehicleId)` 하나. `FCFVehicleIdentityToolResult`가 기존 `FCFVehicleRecordIdentity::BuildDefaultIdentity`를 재사용해 유효성/Definition·Recipe package+object 이름/오류를 반환한다. 신규 에셋·레시피·Package 생성/수정/저장 없음.
- 같은 .cpp에 focused `CarFight.CCAS.CF_FQ_059.P0_03C.G1ToolsetRegistration` 등록: Registry availability, class registration, schema에 R0만 있고 R2 Create/Save가 없는지, 유효·무효 VehicleId 테스트.
- `CarFight_ReEditor.Build.cs v1.10.6`: Private `ToolsetRegistry` dependency 추가. `UE/CarFight_Re.uproject`: UBT 경고에서 direct plugin dependency가 누락되었음을 확인해 `ToolsetRegistry: Enabled=true` 추가(현행 파일 readback PASS).
- 별도 CarFight 플러그인, 모듈 Init/Shutdown 수동 Register/Unregister, 새 공용 registry/권한 시스템을 만들지 않았다. UE5.8 공식 문서의 startup automatic `UToolsetDefinition` class discovery를 우선 사용한다. 실제 등록 실패 때만 현지 Engine lifecycle을 확인하고 수동 등록 필요성을 판단한다.

**공식 첫 빌드:** `carfight.editor.development` job `47a7a0a43f004f9b82d676e0b92955ad`, **FAILED exit6**, UHT 238.005초/3 generated files, [1/42] 새 Toolset.cpp 컴파일 PASS 및 40/42 static LIB link 도달. DLL link 단계에서 실행 중인 UnrealEditor.exe가 `UnrealEditor-CarFight_ReEditor.dll`을 점유해 UBA code9001 → NoUba LNK1104. 현재 빌드 로그에 G1 C++ compile error는 없음. `.uproject` 직접 의존성 교정은 이 첫 빌드 이후 적용했으므로 **교정 후 재빌드 NOT RUN**. OpenXLSX C4996/다른 플러그인 경고는 G1 범위 밖으로 보존.

**Runtime 소유권:** default UE Bridge connected만 확인. 이전 exact CarFight Runtime 조회는 `runtime_provenance_unknown` / `ERR_UE_PROJECT_RUNTIME_INVALID`; 연결된 Editor가 올바른 소유/Checkout인지 미확인이다. 현재 Editor 강제 종료/기존 Runtime 탈취/자동 재빌드는 금지, 다른 세션 Dirty 보호.

**정확한 검증 잔여:** 관리형 Editor의 실제 소유권을 확인한 뒤 기존 공식 종료 절차로 DLL lock을 해제할 수 있을 때 → 현 코드 + uproject 직접 plugin 선언으로 공식 Build exit0 → fresh isolated G1 focused Automation exact1/1 + JSON readback → 등록된 실제 CarFight Toolset wire 이름 및 R0 method를 exact Project Runtime에서 `ue.list_toolsets/ue.describe_toolset`으로 실측. Registry/schema 발견과 G2 `ue.call_read/write` 서버 정책 허용은 다른 계약이다. 실제 `CreateVehicleRecords` Writer, G2 R2 caller/OwnershipWrite 및 G3 receipt 모두 기존 소유권 그대로이며 R2 생성0/6·Save0/6.

**금지:** AI를 Automation caller로 위장하거나 localhost Toolset 발견을 Host 인증 근거로 간주하지 않는다. 코드 외 UE assets/GoPyMCP/Git Commit·Push는 변경하지 않았다.

### 7.9 G1 사용자 공식 빌드 성공 보고 및 공유 소스 재기준 확인 (2026-10-20)

**최신 빌드 증거 분리:** USER가 2026-10-20 `CarFight_ReEditor` 빌드를 직접 완료하고 **성공**했다고 보고했다. 직전 관리형 빌드 job `47a7a0a43f004f9b82d676e0b92955ad` exit6 DLL lock은 역사적 실패로 유지하고, 사용자 보고를 **USER BUILD PASS (로그 독립 재조회 미실시)** 로 기록한다. 이것만으로 G1 Toolset의 런타임 등록·작동 또는 G2 Trusted R2 권한을 수락하지 않는다.

**최신 실제 소스 readback:** `CFVehicleAuthoringToolset.h/.cpp`는 이전 G1 R0-only v1.0.0에서 **v1.1.0**으로 변경된 상태다. 새 `CreateNewVehicleRecords(request_json,gopymcp_execution_proof)` R2 메서드와 Bridge one-use proof claim 호출, `CreateTrustedAIVehicleRecords` 위임이 추가됐으며 기존 G1 Automation도 현재는 R0와 별도의 R2 Schema 존재를 검증하도록 변경됐다. 이는 이전 G1 읽기 전용 구현 이후 다른 작업에서 생긴 변경으로 확인되며, 현 G1 작업은 **해당 변경을 작성·수정·되돌리거나 G2 합격으로 해석하지 않는다**. 코드/증명·정책의 최종 소유 세션과 실제 성공 여부는 G2에서 독립 검수한다. 사용자 빌드가 v1.1.0 변경 이전/이후 어느 정확한 바이트를 포함했는지는 이번 보고만으로 확정되지 않는다.

**실시간 G1 검증:** 이번에 `ue.status` 및 `ue.list_toolsets(max_items=120)`를 재호출했으나 모두 **`ERR_UE_MCP_UNAVAILABLE`**. 따라서 현재 R0/R2 Toolset의 실제 이름·스키마·generation·기존 53개 Toolset 보존은 **NOT VERIFIED**. 관리형 UE Runtime을 임의 생성·다른 Editor를 중지/탈취하지 않았다. focused Automation `CarFight.CCAS.CF_FQ_059.P0_03C.G1ToolsetRegistration` 역시 **NOT RUN**; 현재 테스트가 G2 R2 노출 상태를 기대하는 최신 소스임을 먼저 확인했다.

**남은 일:** 현재 최신 바이트로 빌드된 Editor가 정상 열리고 안전한 관리형 UE 연결이 확보되면 `ue.list_toolsets` → `ue.describe_toolset`에서 정확한 클래스·wire name/함수 스키마를 실측하고, G1 focused Automation은 격리된 공식 runner에서 현재 소스/바이너리 일치 확인 뒤 수행한다. G2 신뢰된 write admission 및 G3 복구 실증은 다른 소유 작업으로 분리한다.

### 7.10 G1 두 독립 검증 실행 — 실제 Registry 등록 실패 확정 (2026-10-20)

**판정: G1 LIVE DISCOVERY FAIL / FOCUSED AUTOMATION 0/1 FAIL — ROOT SYMPTOM: `UToolsetRegistry::IsToolsetClassRegistered(UCFVehicleAuthoringToolset::StaticClass()) == false`.** 이번 실행은 CarFight Editor의 정상적인 신규 관리형 세션과 별도의 Automation 프로세스에서 순차 수행했다. G2 Host write policy/admission이나 새 R2 Create/Save 동작은 검증·변경하지 않았다.

**① 관리형 Live Toolset 조회:** Project `carfight`/Capability `unreal`/Checkout `checkout_eefe2944e0954bed9270908eb95d8fd0`에서 공식 `editor.start` process job `c87bbef577264a54af3543b75757c492` **PASS exit0**. Runtime `runtime_9573e55ffd894df2a4fe003de9ca6e0a`, Editor PID 24032, 8100 port owned by Editor, `project_runtime_ready=true`, `protocol_ready=true`. Exact runtime `ue.status=connected` PASS. `ue.list_toolsets(max_items=120)` 결과 **53개**, `CarFight` / `UCFVehicleAuthoringToolset` matching toolset **0**. 따라서 CarFight 전용 `ue.describe_toolset`은 등록 이름이 없어 호출 대상 부재. 이번 세션이 시작한 Editor는 exact Runtime의 `editor.stop` job `3e8e882b66904d58a5c1f206d0be2de7` **exit0**, `save_requested=false`, `force_kill_used=false`, `project_runtime_stopped=true`로 종료. 타 세션 및 user-owned Editor 종료/저장 없음.

**② 집중 Automation:** 공식 `Tools/RunDataAuthoringTests.ps1`를 `repository_id=main_game`의 승인된 repository script로 한 번 실행. `-ExactTestNames CarFight.CCAS.CF_FQ_059.P0_03C.G1ToolsetRegistration -EvidenceId CF059_G1_Check_20261020_01 -SuppressModelContextProtocolLog`. Job `5a4a3b7405354b2ea3184cba4221ed53` terminal **failed exit1**. 생성된 분리 JSON `UE/Saved/CarFight/DataAuthoringAutomationResult_CF059_G1_Check_20261020_01.json` 직접 readback: `status=failed`, `execution_mode=exact_list_same_process`, `requested_test_count=1`, `engine_exit_code=0`, `success_count=0`, `failure_count=1`, `missing_count=0`, `unexpected_count=0`, `duplicate_terminal_count=0`. UE Automation 정확한 오류: `CFVehicleAuthoringToolset.cpp(200): Expected 'G1 CarFight Toolset registered once at Editor startup' to be true`. 동일 테스트에서 Registry `IsAvailable` 검사 자체는 실패로 기록되지 않았다. 이는 현재 Editor 프로세스에서 Registry는 존재하지만 CarFight 클래스가 **registered가 아닌 상태**임을 좁힌다.

**교정 책임/경계:** `CarFightReEditor.cpp`의 현행 `FCarFightReEditorModule::StartupModule/ShutdownModule`에는 `UCFVehicleAuthoringToolset` 등록·해제 코드가 없음(현행 Source 확인). 기존 설계가 기대한 startup auto discovery가 이 실행에서 이루어지지 않았다. UE5.8 공식 `UToolsetRegistry::RegisterToolsetClass / UnregisterToolsetClass / IsToolsetClassRegistered` API는 존재한다(https://dev.epicgames.com/documentation/unreal-engine/API/Plugins/ToolsetRegistry/UToolsetRegistry). 따라서 현 Editor module의 Registry 유효 시점(필요하면 `OnPostEngineInit`)에 **한 번만 명시적으로 등록**, 정상 shutdown 때 자신이 소유한 등록만 해제하는 최소 교정이 후속 후보이다. 설치된 공급자 초기화 순서와 등록 시점을 검수 후 구현하고 반복 polling/별도 Registry는 만들지 않는다. **중요:** 현 `CFVehicleAuthoringToolset.h/.cpp v1.1.0`에는 G2 작업의 `CreateNewVehicleRecords` R2 `AICallable` 메서드가 함께 존재한다. 클래스 등록은 이 메서드도 UE MCP localhost의 discovered callable로 공개할 수 있으므로 **G2 소유자의 proof claim 신뢰 경계 검수 및 정상 정책 적용과 동기화하지 않고 클래스 등록 교정을 강행하지 않는다**. G1 검증 실패를 G2 OAuth/Policy 차단으로 잘못 분류하지 않는다.

**이번 작업 자산·코드 경계:** Repo source C++/Build.cs/uproject/GoPyMCP 설정 **변경0**, Core6 Create/Save0, 기존 dirty 보존. 테스트 결과/로그와 이 문서 체크포인트 기록만 새로 생성. G1 최종 기술 PASS는 등록 수정 + 재빌드 + exact Test 1/1 + Live list/describe PASS 후에만 수락.

### 7.11 G1/G2 신뢰 실행 및 CarFight 독립 마감검수 (2026-10-20)

**최신 판정: G1 LIVE REGISTRY / SCHEMA PASS + FOCUSED AUTOMATION 1/1 PASS; G2 COMPANY MANAGED R2 SINGLE-PAIR LIVE CREATE + DIRECT PROOF NEGATIVE + PERSISTED RECIPE BINDING PASS; R0 AI PREVIEW FOCUSED 1/1 PASS AFTER TEST-FIXTURE CORRECTION; G3 GENERIC UE-WRITE RECEIPT/REPLAY STILL PARTIAL; CORE6 ACTUAL PRODUCTION CREATE 0/6.** 이전 §7.1~7.10의 G2 BLOCKED/등록 FAIL은 해당 시점의 Historical 근거로 유지하고 본 절만 현재 수락 근거로 사용한다.

**G1 및 G2 연결:** CarFight `CarFightReEditor.cpp`는 `OnPostEngineInit` 준비 후 `UToolsetRegistry::RegisterToolsetClass` 1회/모듈 종료 시 소유 등록 해제로 교정됐다(현재 다른 세션 구현). 관리형 UE ReadOnly가 실측한 toolset은 `CarFight_ReEditor.CFVehicleAuthoringToolset`; 목록 전체 54개, `ue.describe_toolset`은 `PreviewVehicleRecordIdentity`와 증명 소비형 `CreateNewVehicleRecords` 정확히 2메서드. GoPyMCP는 기존 Project `ue.call_write`/Gateway OAuth `ue.write`/exact policy/Ready Runtime/EditorWrite Lease를 통과한 opt-in method에만 1회용 증명을 발행하는 공용 계약을 구현했다. 외부 사용자 주입 Proof는 `ERR_UE_INPUT_INVALID`, 승인 없는 write는 `ERR_UE_APPROVAL_REQUIRED`로 거부. GoPyMCP 원본 계약 `Workspace/docs/contracts/UEExecutionProof_Contract.md v0.1.2`; 집중 회귀 **198 PASS(별도 작업 증거)**. CarFight `Document/Systems/Vehicles/VehicleR2Proof.md v0.1.5`의 근거를 상세로 재사용.

**실제 생성/저장 구분:** Company 관리형 R2 테스트가 `VehicleId=G2R2Smoke20261020A`의 Definition+Recipe 1쌍 **Create PASS**(Toolset 반환 `bSucceeded=true`, 호출 자체 `bSavePerformed=false`). User가 **해당 두 에셋을 직접 Save**했고 종료 후 persisted UObject를 AssetDump로 재조회한 결과 `CFVehicleRecipeData.TargetVehicleData` soft reference가 `/Game/CarFight/Data/Authoring/DA_Vehicle_G2R2Smoke20261020A.DA_Vehicle_G2R2Smoke20261020A`와 정확히 같았고 참조 대상은 `CFVehicleData`. 이번 세션에서 fresh Recipe index **5/5** + Definition index **2/2**를 독립 실행·증거 readback. 이는 단일 Smoke 성공이지 Core6 생산6/6·P0-04 Save writer 자동화·G3 receipt 실증이 아니다.

**CarFight 독립 focused 시험:**
- G1 `CarFight.CCAS.CF_FQ_059.P0_03C.G1ToolsetRegistration`, Admin process job `5644a800538e4bcf82cde8c04a69fe91` **exit0 PASS 1/1**. 격리 JSON `UE/Saved/CarFight/DataAuthoringAutomationResult_CF059_G1G2_Closeout_20261020_01.json` status=success, engine_exit0, failure/missing/unexpected/duplicate0. 실제 Registry 등록·R0/R2 schema·잘못된 proof와 사용자 JSON 권한 위조 거부·새 package 생성0 확인.
- AI R0 `CarFight.CCAS.CF_FQ_059.P0_03C.VehicleAIPreview` initial job `c7ce29eefcdd4ce1923f7112febee782` fail0/1: 시험 Fixture에서 `NewObject<UObject>` 추상 객체를 생성해 UE5.8 `EnsureFailed` 발생; `UObjectGlobals.cpp:3292`→test `CFVehicleAIAdmissionTests.cpp:135` 증명. G2/domain writer failure 아님. `CFVehicleAIAdmissionTests.cpp v1.2.1` **test-only** 변경으로 메모리/비저장 `UCFVehicleRecipeData` concrete 객체를 Chassis wrong-type 증거로 사용해 재현 정책은 유지. 공식 UE5.8 Editor build `bdffe5bcbc1247ae868c9ad51b016fe9` **exit0**, 해당 test TU compile + Editor DLL link PASS. 동일 exact R0 재실행 `3fb3ceae95874e9e90206b8006c7f32d` **exit0 PASS 1/1**. JSON `UE/Saved/CarFight/DataAuthoringAutomationResult_CF059_R0Regression_20261020_02.json` readback engine_exit0, failure/missing/unexpected/duplicate0.

**경고·잔여 판단:** 정규 build `4cd1156e9fbb4c26923098af8cf7f928`은 exit0이지만 "Target is up to date"로 **compile 0**; 후속 `bdffe5...`는 시험 TU만 새로 컴파일. 다른 세션이 현 `CarFightReEditor.cpp v1.10.3`를 annotation-only compile trigger로 관리 중이므로 그 파일을 임의 수정하거나 C4996 소멸을 검증 PASS로 추정하지 않는다. 해당 TU의 실제 재컴파일 로그에서 `OnPostEngineInit` C4996 부재를 확인하는 작업은 소유 세션에 유지한다. 다른 외부 플러그인의 Interchange 의존성 경고는 본 범위와 무관하다.

**최종 단계 경계:** G1 기술 Registry·Schema·G1 focused PASS, Company 관리형 **G2 실호출 one-pair 기술 PASS**. 그러나 **G2 GoPyMCP 자체 Git·배포 마감 및 다른 호스트/클라이언트 OAuth는 별도**, 일반 UE write G3 OperationId/idempotency receipt **UNVERIFIED**, P0-03C 전체 Core6 Create0/6·Save0/6 및 P0-04/Recipe/Evidence Production acceptance **미완료**. 본 회차는 CarFight 시험 Fixture v1.2.1 1건과 문서 정합화만 수행하고 다른 세션 G2 source·runtime·user-saved binary assets·GoPyMCP configuration·Git Commit/Push는 건드리지 않았다.

## Changelog

- **v0.1.14 / 2026-10-20 — G1/G2 post-live closure:** verified G1 Automation exact1/1, refreshed saved Recipe soft reference+Definition class, corrected abstract test-only fixture R0 regression, official compiled test TU/link exit0 plus R0 exact1/1. Company G2 smoke accepted within exact tested scope, G3 and Core6/P0-04 pending. Other session compile-warning TU/GoPyMCP ownership preserved.
- **v0.1.13 / 2026-10-20 — G1 2단계 실제 검증:** ready Project Editor 시작/조회 53 Toolsets 중 custom0, 관리형 정상 stop, focused exact1 Automation 실패0/1(Registry class not registered), isolated result JSON readback. G1 explicit lifecycle registration 후속 교정 필요, 동시 G2 R2 메서드 노출 주의, R2/Asset/Code mutation0.
- **v0.1.12 / 2026-10-20 — G1 user build PASS handoff:** USER build 성공 보고를 앞선 관리형 DLL lock exit6과 별도 기록. 새 소스 v1.1.0에 외부 R2 연결 변경 반영을 읽기 전용 확인했으나 소유 세션의 변경으로 보존. UE Bridge unavailable로 live list/describe, focused Automation 미실행. 실제 R2 Create/Save 0.
- **v0.1.11 / 2026-10-20 — G1 C++ Toolset Source implemented:** Editor-only read-only canonical identity tool and registry-focused test, Build.cs/.uproject ToolsetRegistry dependency. UHT/C++ PASS but Editor DLL lock build exit6; live admission/test pending. R2 mutation0.
- **v0.1.10 / 2026-10-20 — G1 등록 설계검수:** Epic UE5.8 공식 `UToolsetDefinition/UToolsetRegistry` C++ API와 ModelContextProtocol RefreshTools/new UFUNCTION restart contract 확인. 현 `CarFightReEditor` module lifecycle와 Build.cs를 비교해 단일 Editor 모듈 최소 통합 및 R0만 먼저 노출, 실제 공급자 initialization/등록 방식은 현지 소스 확인 후 선택, G2/G3 분리·localhost write 주의 기록. C++/UE asset/runtime 변경 없음.
- **v0.1.9 / 2026-10-20 — GoPyMCP G1~G3 외부 조사 수렴:** GoPyMCP 보고서의 G1 PARTIAL/G2 BLOCKED/G3 PARTIAL, 정확한 Host policy, `ue.write` vs domain R2 신뢰 경계, 단일 invoke outcome_unknown/no idempotent job을 CarFight 현 Writer 계약과 대조해 반영. 외부 보고 내용과 CarFight에서 실제 확인된 코드를 구분. C-2/3 수정·저작 실행 없음.
- **v0.1.8 / 2026-10-19 — C-0 실제 Registry·서버 R2 허용 계약 집중 검증:** connected UE53 Toolsets, `UToolsetRegistry` 등록/자동탐지 설명, custom CarFight R2 노출0, allowlisted AssetTools R0 success vs existing ListEnabledPlugins/ListDiscoveredPlugins/ListSkills `ERR_UE_TOOL_BLOCKED` 3종으로 dynamic registration과 server dispatch 권한 분리 확인. G1~G3 exact Host 계약 질문 및 최소 CarFight Adapter/불필요한 신규 시스템 금지 결정. Code/UE asset mutation none.
- **v0.1.7 / 2026-10-19 — T-13 회귀 선택자·격리 검수:** 기존 MeshCreate/RecordCreation 두 Automation의 실제 이름·검증 범위·test-only 메모리 객체 정리 경계를 확인. 테스트용 Dirty package의 완전한 cleanup은 source만으로 증명되지 않으므로 별도 unattended Editor 회귀로만 실행하도록 정리. 이번 변경은 문서만; 구현·Build·Automation 없음.
- **v0.1.6 / 2026-10-18 — R0 음성 회귀 최소 보강 및 Link 차단 기록:** 메모리 전용 생성 대상 충돌·잘못된 Chassis 타입·Hardpoint Hash 3개를 기존 test에만 추가. 테스트 소스 컴파일은 진행됐으나 Editor가 module DLL을 점유해 공식 전체 빌드 exit6; 신규 Automation NOT RUN. C-2/3 AI R2 Create 개방 없음.
- **v0.1.5 / 2026-10-17 — CarFight C-2/3 실행 경계 사전검수:** 현재 C++ writer·UI/Automation·AI Preview·테스트를 실제 대조하고 Host-independent 회귀·도메인 점검과 Host-dependent admission/receipt를 분리. 현재 차단 조건과 C-2/3 변경0 유지.
- **v0.1.4 / 2026-10-17 — C-0 공개 Host 계약 검사 결과 반영:** 등록 Project operations/UE 공개 53개 toolsets·현재 Create gate·관리형 권한/receipt 미증명 범위를 기록하고 GoPyMCP owner blocker를 특정했다. 신규 Vehicle R2 쓰기 허용·C-2/3 코드 작업은 하지 않았다. 대표 Plan pointer v0.1.36으로 갱신, 기존 C-1 회귀 PASS 및 디자인 계약을 보존했다.
- **v0.1.3 / 2026-10-17 — PER 경로 재확인 및 C-1 focused 2/2 PASS:** legacy `repository_id=main_game`으로 동일 보존형 runner를 실제 호출하여 VehicleAIPreview, Builder Naming 각각 1/1 PASS; JSON 각각 readback. `carfight_repo`는 Project Registry repository ID이며 legacy process selector가 아님을 교정. 실제 AI R2 Create 및 C-0 trust contract는 미수락. 이력 v0.1.2의 당시 NOT RUN은 과거 사실로 유지.
- **v0.1.2 / 2026-10-16 — C-1 구현·문서 동기화:** 공통 `FCFVehicleRecordIdentity`로 기존 Builder VehicleId/default package 4-field 생성 규칙을 직접 재사용하고 AI R0 Preview에 canonical root/이름 강제. SlateUI/Automation 고급 경로 유지, 기존 R2 Create 경로 유지. UE5.8 managed Editor Build `c35c72f769de422d81e8bb86fdab4148` exit0 PASS; focused Automation은 기존 evidence overwrite 위험으로 NOT RUN. C-0 Host R2 계약 HOLD, R2 실제 생성 미구현.
- **v0.1.1 / 2026-10-16 — 교정·재검수:** P0-01~03, P1-01~03 교정 및 CarFight 내부 설계 정합성 6/6 재검수 PASS. C-0 외부 Host 신뢰 R2 admission actual contract HOLD.  실제 Host operation 권한을 인증 근거로 사용하고 사용자 지시→최종 Hash 선승인 요구를 제거했다. 미확인 `UserIntentRef`/토큰 발급/추가 승인 UI는 채택하지 않음. C-0 실제 entry 조사 → 최소 bounded 연결 순서, UI 기존 경로 spoof 금지, reference path hash 한계·P0-04 단계 구분, Builder naming 공유, Host 중복/불확실 결과 소유, 실제 enum/테스트 항목 및 phase gate를 정합화. 문서 교정만 수행.
- **v0.1.0 / 2026-10-16 — 최초 설계:** 사용자 지시 범위·Host/CarFight 경계·기존 writer 재사용·T-01~14 수락 매트릭스 수립. 이력으로 보존.
