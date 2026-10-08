# CarFight Plan 작업 규칙

- 문서 버전: v1.7
- 최근 갱신일: 2026-10-08
- 문서 상태: Current
- 적용 범위: `main_game`의 `Document/Plan/` 이하 계획 문서와 체크포인트

---

## 1. 저장소 대문

이 파일은 `main_game` 내부 `Document/Plan/`의 가장 가까운 Plan 전용 작업 대문이다.
Plan은 더 이상 별도 configured repository가 아니므로 CarFight 루트 `AGENTS.md`와 상위 `Document/AGENTS.md`를 자동 상속하고, 이 파일은 Plan 경로에 필요한 차이만 추가한다.

이 파일은 해당 경로에서 필요한 Plan 규칙만 제공한다.
`repository_instructions`는 정책 확인을 돕는 소프트 게이트이며 별도의 서버 측 pre-write 하드 게이트를 요구하지 않는다.
하위 `AGENTS.md`는 특정 Plan 폴더에 실제 차별 규칙이 있을 때만 추가한다.

```text
Git 저장소 경계: main_game
논리적 소유권: CarFight 기능별 Plan과 검증 체크포인트
현재 Work lifecycle: main_game의 Document/UDS/records/**
세션 복원 view: main_game의 Document/UDS/derived/Current.md (authority0)
Planning / 프로젝트 기준선: main_game의 ProjectSSOT
현재 구현: main_game의 실제 코드·에셋과 Systems
Legacy ActiveWork: pre-cutover retained frozen authority0
```

---

## 2. 작업 시작 순서

```text
1. main_game Git 브랜치와 미커밋 변경 확인
2. 이 AGENTS.md 확인
3. 세션 복원·전환이면 main_game의 Document/UDS/derived/Current.md에서 Work 선택
4. 선택한 main_game canonical UDS Work record 확인
5. Document/Plan/README.md에서 해당 representative Plan 경로 선택
6. 우선순위·착수 판단은 main_game의 ProjectSSOT 확인
7. 현재 구현은 main_game의 Systems와 실제 코드·에셋 확인
8. 선택한 representative Plan과 필요한 세부 문서만 읽기
```

Plan 루트 전체를 먼저 재귀 탐색하지 않는다.
명시적 요청이나 직접 작업 대상이 아니면 Archive, Generated와 ConceptArt 이미지를 기본 검색에서 제외한다.
TaskSource, WorkOrder, Codex와 YAML 산출물은 대표 Plan을 먼저 확인한 뒤 필요할 때만 읽는다.

---

## 3. Plan의 역할

Plan은 앞으로 구현하거나 검증할 범위, 단계, 보호 조건과 체크포인트를 관리한다.
완료된 현재 구현은 실제 코드·에셋과 Systems가 소유한다.
Plan이 더 최근에 수정되었다는 이유만으로 Current System을 대체하지 않는다.

현재 구현 판단은 다음 순서를 사용한다.

```text
실제 코드·에셋
→ Systems
→ ProjectSSOT/01_ProjectState.md baseline
→ canonical UDS Work lifecycle
→ 대표 Plan detailed checkpoint
```

다음 작업과 우선순위는 Roadmap의 sequence, FeatureQueue의 후보·우선순위·착수 판단, canonical UDS Work의 current lifecycle/next, representative Plan의 detailed checkpoint를 순서대로 교차검증한다.

Plan 작업이 `main_game` 코드·설정·스크립트 변경으로 이어지면 `Document/CodeWorkGate.md`를 확인한다.
현재 AI 세션의 직접 구현, diff 검수와 가능한 빌드·테스트가 기본 경로다.
TaskSource와 WorkOrder는 선택 참고·실행 문서이며 최종 Codex YAML이나 외부 Codex는 필수 착수 조건이 아니다.
별도 Codex 위임은 사용자가 명시적으로 요청한 경우에만 선택한다.

---

## 4. 문서와 Git 규칙

새 문서 생성과 공식 Plan 등록을 분리한다.
Draft, Working과 Notes는 색인 갱신을 요구하지 않는다.
새 Plan이 공식 착수 대상으로 승격될 때만 `Document/Plan/README.md`를 갱신한다.
Plan이 Current System으로 승격되면 main_game의 `Document/Systems/SystemIndex.md`가 현재 구현을 소유한다.

Plan Index는 Work lifecycle owner가 아니라 **authority0 representative Plan navigation projection**이다. 대표 Plan이 상세 checkpoint/evidence를 소유하고, Active/Ready/Paused 등 promoted Work의 current lifecycle state/phase/next는 main_game `Document/UDS/records/**`가 canonical하게 소유한다. `Document/Plan/README.md`는 canonical UDS에서 선택한 Work를 representative Plan 경로로 연결하는 데 사용한다.

Current Plan은 `Document/Plan/<Feature>/` 기능 폴더 단위로 배치한다. `Document/Plan/` 루트에는 `AGENTS.md`, `README.md` 같은 저장소 대문만 두고, 서로 같은 Feature를 소유하는 Plan/Roadmap/Design/Spec은 같은 기능 폴더에 모은다. 완료·대체·독립 저장소 이관 문서는 `Archive/`에서 기능 묶음으로 보존한다.

대표 Plan은 상세 evidence를 보존하되 문서 안에 여러 세대의 상태를 기록할 때 **현재 checkpoint와 당시 Historical checkpoint를 명확히 구분**한다. Feature의 USER/Technical detailed checkpoint가 전진하면 representative Plan 상단 Current checkpoint와 해당 Plan 내부 현재 표현을 갱신하고, 과거 상태는 날짜·버전·당시 상태라는 표식을 유지한다. Work lifecycle state/phase/next가 바뀌는 경우 main_game UDS immutable successor가 canonical write이며 Plan Index에 별도 lifecycle truth를 dual-write하지 않는다. 완료 Feature의 현재 구현 owner 버전이 post-closure remediation으로 올라가더라도 당시 Systems Promotion evidence 자체는 Historical로 보존할 수 있으나, Plan Index·Archive Index와 문서 상단의 Current owner 포인터는 main_game 최신 Systems와 동기화한다.

대규모 정리는 주기적으로 강제하지 않는다. Feature 종료 또는 상태 전진 시 작은 projection cleanup을 같이 수행하고, stale current checkpoint·owner pointer 충돌이 발견될 때만 Health Check 범위를 넓힌다.

완료 후 Historical 전환은 다음 Gate를 사용한다.

```text
G0 실제 구현 + 필요한 Build/Automation/USER PIE 완료
G1 Current Knowledge가 Systems와 필요한 ProjectSSOT로 승격
G2 main_game UDS Work를 immutable successor로 closed/retired 수렴 + authority0 Current/head 재생성 + Plan Index stale navigation route 정리
G3 대표 Plan/Result 탐색 경로 보존
G4 semantic Historical
G5 optional physical move
```

`G4 Historical`은 파일을 `Archive/` 폴더로 실제 이동해야만 성립하는 상태가 아니다. 기존 경로를 유지하는 `Historical + Retained Path`를 허용하며, 물리 move/rename은 링크·dirty work·rollback 안전성을 별도 확인하는 maintenance다.

대표 Plan에는 현재 상태, 완료·미검증 범위, 빌드, 자동 테스트, PIE, 다음 작업, 관련 경로와 보호 범위를 필요한 만큼 기록한다.
사용자가 확인하지 않은 PIE 결과를 `PASS`나 `Completed`로 기록하지 않는다.

검증 상태는 실제 결과, 증거, 실행 수단과 공용 재실행 진입점의 존재 여부를 분리한다.
작업 전용 단발성 스크립트나 임시 실행 경로는 해당 체크포인트의 실행 수단일 뿐 공용 도구 또는 저장소 계약으로 자동 승격하지 않는다.
검증 경로가 없었던 경우에는 `Runner Unavailable`처럼 전역 상태로 쓰지 않고 `해당 날짜·체크포인트에서 승인된 실행 경로 미확보`처럼 범위를 명시한다.
과거 Plan의 Not Run 기록은 역사 상태이며 main_game의 최신 코드·Systems·ProjectSSOT와 검증 증거를 덮어쓰지 않는다.
현재 가능 여부를 판단할 때는 현재 도구 표면, 실제 저장소 상태, 작업 허용 범위와 최신 증거를 다시 확인한다.
작업을 완료한 뒤 실제 다음 단계나 이어서 수행할 작업이 남아 있으면 최종 보고에 사용자가 바로 붙여넣을 수 있는 짧은 추천 프롬프트 1개를 제공한다. 후속 작업이 없거나 단순한 가능성만 있는 경우에는 억지로 제안하지 않는다.

기존 dirty 파일은 현재 내용을 읽고 최소 범위만 수정한다.
사용자의 미커밋 변경과 신규 산출물을 임의로 정리하거나 되돌리지 않는다.
`main_game` Git 상태와 diff에서 `Document/Plan/**` 직접 변경 범위를 분리해 검수하고, 다른 작업의 unrelated dirty는 그대로 보호한다.

사용자의 명시적 요청 없이는 commit, push, reset, checkout, stash, rebase, merge와 clean을 수행하지 않는다.

---

## 5. Changelog

### v1.7 - 2026-10-08

- `Document/Plan`을 별도 `plan_repo`에서 `main_game` 일반 디렉터리로 통합한 현재 Git 경계를 반영했다.
- Plan 작업이 CarFight 루트 `AGENTS.md`와 `Document/AGENTS.md`를 자동 상속하도록 작업 대문과 시작 순서를 정렬했다.
- 현재 Git 검수는 `main_game` 단일 저장소에서 `Document/Plan/**` 범위를 분리해 수행하며, 기존 unrelated dirty 보호 원칙을 유지한다.
- 과거 `plan_repo` 경계와 당시 dirty를 언급한 날짜·버전 기록은 Historical evidence로 보존한다.

### v1.6 - 2026-09-16

- UDS-08 MIG-05 permanent adoption을 반영해 promoted Work의 current lifecycle authority를 main_game `Document/UDS/records/**`로 전환했다.
- 세션 복원은 main_game authority0 UDS Current → canonical Work record → Plan Index representative path → representative Plan 순서를 사용한다.
- Plan Index는 lifecycle second-authority가 아닌 authority0 navigation projection으로 재분류하고, Plan은 detailed checkpoint/evidence owner 역할을 유지한다.
- Work lifecycle 변화는 main_game UDS immutable successor가 canonical write이며 Plan Index/Legacy ActiveWork dual-write를 금지한다.

### v1.5 - 2026-08-29

- Plan 루트를 대문 중심으로 유지하고 Current/Paused/Ready 문서를 `Document/Plan/<Feature>/` 기능 폴더 단위로 묶는 물리 배치 규칙을 추가했다.
- 완료·대체·외부 저장소 이관 문서는 `Archive/` 기능 묶음으로 보존하고 Generated Intermediate·중복 예시만 Trash 격리하도록 정리했다.
- 2026-08-29 물리 정리에서 사용자 요청에 따라 기존 root-level Current 문서 경로를 기능 폴더로 이동했으므로 이후에는 새 경로를 공식 진입으로 사용한다.

### v1.4 - 2026-08-22

- 대표 Plan의 상세 evidence 보존과 Current checkpoint projection을 분리하는 Lifecycle 규칙을 추가했다.
- 상태 전진 시 상단 Current checkpoint와 현재 표현만 동기화하고, 날짜·버전이 붙은 Historical evidence는 보존하도록 했다.
- Plan/Archive Index의 Current owner 포인터는 main_game 최신 Systems와 동기화하고 당시 Systems Promotion evidence는 Historical로 유지하도록 경계를 명시했다.
- 정기 대청소 대신 Feature 종료·상태 전진 시 소규모 projection cleanup과 stale 신호 기반 Health Check를 사용하도록 했다.

### v1.3 - 2026-08-13

- 대표 Plan을 detailed status owner, Plan Index를 Current projection으로 명시했다.
- 완료 Plan의 Historical 전환에 Evidence → Current Knowledge Promotion → Current Route Cleanup → Reference Preservation Gate를 추가했다.
- semantic Historical과 physical Archive placement를 분리하고 `Historical + Retained Path`를 정식 허용했다.

### v1.2 - 2026-08-02

- Plan과 WorkOrder의 검증 기록에서 실제 결과, 증거, 실행 수단과 공용 재실행 진입점을 분리하도록 했다.
- 작업 전용 단발성 스크립트나 임시 경로를 공용 저장소 도구로 자동 승격하지 않도록 명시했다.
- 과거 Runner Unavailable과 Not Run을 특정 날짜·체크포인트의 역사 상태로 제한하고 현재 실행 가능 여부를 대신하지 않도록 했다.
- 현재 상태 판단은 main_game의 실제 코드, Systems, ProjectSSOT와 최신 검증 증거를 우선하도록 보강했다.

### v1.1 - 2026-07-30

- 실제 후속 단계나 재개 작업이 남아 있을 때 최종 보고에 복사 가능한 짧은 추천 프롬프트 1개를 제공하도록 종료 방침을 추가했다.
- 후속 작업이 없거나 단순 가능성만 있는 경우에는 불필요한 추천 프롬프트를 만들지 않도록 제한했다.

### v1.0 - 2026-07-30

- `plan_repo` 루트의 Browser MCP 작업 대문을 신규 생성했다.
- 상위 CarFight AGENTS가 자동 적용되지 않는 configured repository 경계를 명시했다.
- Plan의 논리적 CarFight 소유권과 별도 Git 경계를 분리했다.
- ActiveWork, ProjectSSOT, Systems와 실제 구현의 권한 관계를 정의했다.
- 현재 AI 직접 구현, 선택적 TaskSource·WorkOrder와 외부 Codex 기준을 Current 방침에 맞췄다.
- 상세 절차를 Plan 색인과 main_game Current 문서로 연결하고 대문 역할 중심으로 유지했다.

---

## 6. Migration

- 2026-10-08부터 `Document/Plan`은 `main_game`이 직접 추적하며 별도 `plan_repo` Git authority를 사용하지 않는다. 과거 체크포인트의 `plan_repo` 표기는 당시 구조를 설명하는 Historical evidence로 유지한다.
- v1.4부터 대표 Plan 내부의 과거 USER/Technical 상태는 날짜·당시 상태가 명확하면 삭제하지 않는다. 다만 `현재`, `Current`, `Next exact gate`로 표시된 projection은 상단 checkpoint와 모순되지 않게 유지한다.
- v1.3부터 완료 Plan을 Current에서 내릴 때 물리 이동을 먼저 요구하지 않는다. Systems/ProjectSSOT 승격과 Current route cleanup이 완료되면 `Archive/README.md`에 Historical + Retained Path로 등록할 수 있다.
- `Document/Plan/README.md`는 detailed evidence나 promoted Work lifecycle truth를 복제하지 않고 canonical UDS에서 선택한 Work를 representative Plan 경로로 연결하는 authority0 navigation projection으로 사용한다.
- v1.1부터 Plan 작업 완료 후 실제 후속 작업이 남아 있으면 최종 보고에 짧은 추천 프롬프트 1개를 포함한다.
- v1.5부터 Current/Paused/Ready Plan은 기능 폴더 경로를 공식 진입으로 사용한다. 2026-08-29 이전 root-level 경로는 Historical 문맥 외에는 새 경로로 교정한다.
- 새 Plan 작업은 이 파일과 `Document/Plan/README.md`에서 시작한다.
- current Work lifecycle이 필요하면 main_game `Document/UDS/derived/Current.md`와 canonical `Document/UDS/records/**`를 확인하고, 현재 구현 판단은 Systems와 실제 저장소를 교차검증한다.
- 별도 규칙이 실제로 필요한 하위 폴더에만 추가 `AGENTS.md`를 만든다.
- 기존 미커밋 Plan 문서와 신규 산출물은 자동으로 정리하거나 되돌리지 않는다.
- 과거 검증 경로 미확보 기록은 당시 체크포인트 이력으로만 읽고 현재 검증 가능 여부는 다시 확인한다.
- 작업 전용 임시 실행 경로는 명시적인 공용화 결정 없이 공식 도구나 Git 등록 대상으로 해석하지 않는다.