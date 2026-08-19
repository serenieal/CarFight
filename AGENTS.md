# 최우선 코드 작업 게이트

- 이 게이트는 이 파일의 모든 일반 코드 작성 규칙보다 우선한다.
- CarFight의 C++, 설정, 스크립트와 빌드 파일 작업은 현재 AI 세션이 저장소 도구를 사용해 직접 구현하는 것을 기본 경로로 한다.
- 코드 작업 요청을 받으면 먼저 현재 Git 상태, 이 파일과 `Document/CodeWorkGate.md`를 확인한 뒤 허용 범위와 보호 범위를 확정한다.
- TaskSource, WorkOrder와 Codex YAML은 복잡한 작업을 구조화할 때 사용할 수 있는 선택 산출물이며, 직접 구현을 시작하기 위한 필수 게이트가 아니다.
- 현재 AI 세션은 분석, 설계, 실제 파일 수정, 빌드·자동 테스트, Git diff 검수와 문서 동기화를 한 작업 흐름에서 수행한다.
- 기존 미커밋 변경과 사용자 자산을 임의로 수정·정리·되돌리지 않는다. 충돌 위험이 있는 파일은 읽고 영향 범위를 분리한 뒤 안전하게 수정한다.
- commit, push, reset, checkout과 stash는 사용자가 명시적으로 요청하지 않으면 수행하지 않는다.
- 아래 코드 작성 규칙은 현재 AI 세션이 직접 구현할 때 그대로 적용한다.



항상 기본적으로 한국어로 답변해. 내가 다른 언어를 명시적으로 요청한 경우에만 해당 언어를 사용해.

내 주요 작업은 언리얼 엔진 개발이다. 답변은 초보자도 따라올 수 있도록 중간 과정을 생략하지 말고 순서대로 자세히 설명해. 설명을 너무 짧게 압축하지 말고, 실제 작업 흐름이 보이도록 단계별로 안내해.

코드를 작성할 때는 다음 규칙을 반드시 지켜라.
- 모든 변수 바로 위에 해당 변수의 역할을 설명하는 한 줄 주석을 작성해라.
- 모든 함수 바로 위에 해당 함수의 역할을 설명하는 한 줄 주석을 작성해라.
- 변수명과 함수명, 파일명, 클래스명은 직관적이고 의미가 분명하게 지어줘.
- 기존에 제안한 코드가 있다면 그 스타일과 구조를 최대한 유지해.
- 코드를 수정하거나 개선할 때는 반드시 버전 표기(v1.0, v1.1 등)를 해줘.
- 코드 변경 시 변경점(Changelog)과 마이그레이션 지침을 함께 제공해줘.
- 기존 코드의 함수 시그니처, 파일 경로, 구조를 말 없이 바꾸지 말아줘.
- 각 코드 블록을 보여주기 전에 정확한 파일 경로와 파일명, 그리고 작업 유형(신규 / 수정 / 교체 / 추가)을 먼저 적어줘.
- 새로 제안하는 파일명과 클래스명은 32자를 넘기지 않게 해줘.

언리얼 엔진 관련 설명에서는 다음 규칙을 지켜줘.
- CarFight의 현재 공식 엔진 기준은 **Unreal Engine 5.8 Source Build**이며 물리 경로는 `D:\UnrealEngine_Source`다.
- CarFight 관련 설명·설계·코드·플러그인 호환성 판단은 별도 엔진 업그레이드 결정이 있기 전까지 UE 5.8을 기준으로 한다.
- 과거 문서, 대화, 설치 경로에 남은 UE 5.7 표기는 Historical/폐기 기준이며 현재 엔진 버전으로 재해석하지 않는다.
- 메뉴 경로, 옵션명, 설정명은 한국어(English) 형식으로 함께 표기해줘.
- 실제로 존재하는 블루프린트 노드 이름만 사용해줘.
- 개념적인 표현을 실제 블루프린트 노드처럼 설명하지 말아줘.
- 특정 노드나 기능의 존재 여부가 확실하지 않으면 추측하지 말고 불확실하다고 말해줘.
- 블루프린트에서 사용하는 변수와 함수에는 이해하기 쉬운 툴팁 문구도 함께 제안해줘.
- 언리얼 에디터에서 타입/함수/클래스를 찾게 안내할 때는 C++ 심볼명 그대로 쓰지 말고, 에디터에 보이는 이름 기준으로 안내해줘. 예: `FCFVehicleDebugOverview`가 아니라 `CFVehicleDebugOverview`.

문제 해결과 구현 안내에서는 다음 원칙을 지켜줘.
- 정보가 부족하면 추측하지 말고 불확실한 부분을 명확히 구분해줘.
- 화려한 방법보다 안전하고 유지보수하기 쉬운 방법을 우선 추천해줘.
- 여러 방법이 있으면 가장 안정적인 기본안을 먼저 제시하고, 그 다음에 대안을 설명해줘.
- 구현 후에는 반드시 검증 방법, 체크포인트, 예상 결과를 함께 알려줘.

답변 형식은 다음 규칙을 지켜줘.
- 모든 답변의 가장 첫 줄에 서울 기준 현재 시각을 표시해줘.
- 답변은 섹션, 단계, 체크리스트 형태로 구조적으로 정리해줘.
- 직설적이고 실용적으로 설명해줘.

문서를 작성할 때는 해당 문서를 어떤 세션 어떤 AI에서 읽어도 같은 방향으로 구현할 수 있게끔 작성해야해.
기본적으로 토큰을 낭비하지 않고 아끼는 방향으로 작업해야해.
주석이나 UI등 내가 읽어야 하는 사항은 한글로 표기해줘.
변경 시 changelog 포함.
무분별한 리네이밍 금지.
Public/Private 경로 규칙 준수.

Windows PowerShell에서 한글/비ASCII가 포함된 파일을 읽을 때는 기본 인코딩을 절대 사용하지 말 것.
항상 UTF-8을 명시해서 읽을 것.

규칙:
- Get-Content 사용 시 항상 -Encoding utf8 지정
- 한글이 포함된 파일 내용 조회는 가능하면 .NET ReadAllText(..., UTF8Encoding) 또는 python -X utf8 사용
- 인코딩 지정 없는 Get-Content, type, cat 별칭 사용 금지
- 코드/주석을 인용하거나 요약하기 전에 UTF-8로 다시 읽어 확인
- Windows PowerShell 5.1에서는 UTF-8 without BOM 파일이 깨질 수 있으므로 기본 인코딩 가정 금지

권장 패턴:
- Get-Content <path> -Encoding utf8
- [System.IO.File]::ReadAllText(<path>, [System.Text.UTF8Encoding]::new($false))
- python -X utf8 -c "..."


# 라우팅 규칙

- 문서 정리, 명세 정제, 설계서 압축, 중복 제거, 구조 개선 요청에는 `doc_curation` skill을 우선 사용한다.
- AGENTS.md는 짧게 유지하고 전체 워크플로는 넣지 않는다.

# 코드 작업 게이트 상세

- 상세 실행 규칙, 사전 확인, 직접 수정, 검증과 상태 기록 기준은 `Document/CodeWorkGate.md`를 따른다.
- 현재 AI 세션은 승인된 저장소 도구로 코드·설정·스크립트를 직접 수정하며, 별도 Codex 위임은 사용자가 명시적으로 요청한 경우에만 선택적으로 사용한다.
- 문서 전용 수정과 읽기 전용 분석·리뷰는 코드 수정 게이트의 빌드 요구를 작업 성격에 맞게 적용한다. Blueprint·DataAsset·Niagara·StaticMesh 등 UE asset 작업은 `Document/CodeWorkGate.md`의 evidence routing에 따라 AssetDump/GoPyMCP UE MCP 기술 검증을 먼저 검토하고, 사람 판단이나 현재 capability 경계를 벗어나는 부분만 사용자 작업으로 남긴다.


# CarFight 작업 시작 및 종료 규칙


- 작업 시작 전에 현재 Git 브랜치와 미커밋 변경을 확인한다.
- `Document/Document_Entry.md`를 읽고 작업에 필요한 ProjectSSOT, Systems, Plan 문서만 선택해서 확인한다.
- 이전 대화나 AI 기억보다 현재 저장소와 ProjectSSOT를 우선한다.
- 기존 미커밋 변경을 임의로 수정, 정리 또는 되돌리지 않는다.
- 작업 상태가 변경된 경우 관련 대표 Plan 또는 현재 구현 문서에 완료 범위, 미검증 항목 및 다음 작업을 기록한다.
- 작업을 완료한 뒤 실제 다음 단계나 이어서 수행할 작업이 남아 있으면 최종 보고에 사용자가 바로 붙여넣을 수 있는 짧은 추천 프롬프트 1개를 제공한다. 후속 작업이 없거나 단순한 가능성만 있는 경우에는 억지로 제안하지 않는다.
- 사용자가 `이전 작업 이어서 진행해줘`라고 요청하면 `Document/ActiveWork.md`에서 마지막 작업 초점을 찾고 대표 Plan, 관련 Systems와 실제 코드를 교차검증한 뒤 복원 결과를 먼저 보고한다.
- 사용자가 특정 작업명 또는 작업 ID로 재개를 요청하면 `Document/ActiveWork.md`의 해당 작업을 마지막 작업 초점보다 우선한다.
- 사용자가 `새 세션 인계 준비해줘`라고 요청하면 상태가 바뀐 활성 작업의 대표 Plan 체크포인트와 `Document/ActiveWork.md`를 갱신하고 새 세션용 짧은 시작 문구만 제공한다.

# 독립 저장소 경계 규칙

- CarFight 문서체계는 `main_game`의 게임 프로젝트 작업만 관리한다.
- `UE/Plugins/ue-assetdump`와 `GoPyMCP`는 각각 별도 Git 저장소이며 자체 `AGENTS.md`, 문서 진입점, ActiveWork와 Plan을 사용한다.
- 독립 저장소의 내부 작업 상태, 릴리스 계획, TaskSource와 체크포인트를 `Document/ActiveWork.md` 또는 `Document/Plan/`에 등록하지 않는다.
- CarFight가 독립 도구의 공개 계약에 의존할 때는 계약명, 요구 버전과 CarFight 사용 위치만 기록한다.
- 독립 저장소 작업을 시작하면 해당 저장소의 Git 상태와 가장 가까운 `AGENTS.md`를 먼저 확인한다.

# CarFight UE 증거·작업 라우팅 규칙

- 저장된 Asset/DataAsset/Blueprint/reference/package 사실은 **AssetDump persisted/snapshot evidence**를 우선한다.
- 현재 Editor/world/unsaved/PIE-runtime 사실과 Accepted scope의 bounded 기술 검증은 **GoPyMCP UE MCP live evidence**를 우선한다.
- PIE-runtime의 World/LocalPlayer/Pawn/Component/property/snapshot/log 등 **Accepted `GoPyMCP.RuntimeRead`로 관측 가능한 기술 사실은 USER에게 대신 읽어달라고 요청하기 전에 AI가 직접 검증**한다.
- RuntimeRead로 판정 가능한 상태 전이, 바인딩, 데이터 전달과 runtime invariant는 AI Technical Validation으로 닫을 수 있다. 반대로 시각 품질, UX, 조작감, 주행감, 조준감과 연출 감각은 사용자 PIE/manual validation으로 남긴다.
- 단순 관측을 위해 Product code에 임시 Debug HUD, Print, UE_LOG, 전용 getter 또는 Consumer별 MCP operation을 먼저 추가하지 않는다. Accepted RuntimeRead로 충분하지 않은 실제 관측 공백이 확인될 때만 별도 capability 필요성을 검토한다.
- RuntimeRead는 read-only 관측 수단으로 취급하며 Editor/PIE lifecycle, write/destructive approval과 분리한다. 관측 편의를 이유로 임의 getter/function 실행, blind retry 또는 기존 PIE ownership adoption을 요구하지 않는다.
- 같은 질문에 AssetDump와 UE MCP를 습관적으로 중복 호출하지 않는다. Fresh AssetDump generation과 live UE MCP가 둘 다 필요하면 isolation이 별도 증명되기 전 기본 직렬화한다.
- 기존 Accepted UE MCP capability의 정상 프로젝트 사용은 완료된 TC-00~09 또는 RuntimeRead RR-00~10을 다시 여는 사유가 아니다. 새 tool identity, write/destructive 종류, getter/function 실행, guard/retry/public-facade/lifetime 의미가 필요할 때만 GoPyMCP의 새 workflow-driven validation을 연다.

- 상세 선택·검증·approval 경계는 `Document/CodeWorkGate.md`와 GoPyMCP Current policy를 따른다.

# CarFight 빌드/실행 강제 규칙

- 현재 공식 엔진은 **Unreal Engine 5.8 Source Build**이며 기준 루트는 `D:\UnrealEngine_Source`이다.
- 에디터 빌드는 반드시 `D:\Work\CarFight_git\Tools\BuildEditor.bat`를 실행한다.
- 에디터 실행은 반드시 `D:\Work\CarFight_git\Tools\RunEditor.bat`를 실행한다. Browser의 자동 lifecycle 시작은 `Tools/RunEditor.ps1`을 사용하되 이 래퍼가 canonical BAT를 호출해야 한다.
- 공식 Editor build 또는 Consumer-loaded plugin/module DLL을 다시 link/replace하기 전에는 현재 Editor가 대상 binary를 실제 점유하는지 먼저 판정한다. 충돌이 없으면 build를 위해 Editor를 불필요하게 종료하지 않는다.
- 대상 binary 충돌이 있으면 아래 lifecycle ownership 규칙을 적용한다. user-owned/unknown Editor는 자동 force/discard하지 않고 사용자 종료가 필요한 blocker로 보고하며, proven AI-owned + 사용자 작업 없음 + cleanup 완료 lifetime만 canonical save0 stop으로 충돌을 해소할 수 있다.


- 빌드/실행/테스트에서 `D:\UE_5.7` 또는 `D:\UE_5.7_Source`를 사용하지 않는다. 이 경로명은 과거 금지 대상일 뿐 현재 엔진 버전을 뜻하지 않는다.
- 오래된 문서의 UE 5.7 및 `D:\UE_5.7` 예시는 Historical/폐기 기록으로 보고 현재 판단이나 새 명령에 사용하지 않는다.
- 직접 명령을 작성해야 하면 먼저 `Tools\CarFightEnv.bat`를 확인하고, 반드시 `D:\UnrealEngine_Source\Engine\Build\BatchFiles\Build.bat`와 `D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor.exe`를 사용한다.

# Unreal Editor lifecycle 자동 운영 규칙

- 작업에 실제 live Editor가 필요하고 canonical lifecycle entry가 사용 가능하면 Browser가 사용자에게 수동 실행을 요구하기 전에 `Tools/RunEditor.ps1`로 CarFight Editor를 직접 시작할 수 있다. Browser fixed start의 terminal PASS는 exact CarFight project process가 local 8100 listener를 직접 소유하는 **Ready 상태**까지 확인한 경우에만 인정하며, process 출현만으로 live Ready를 주장하지 않는다. Editor 시작 자체는 Blueprint/UE write/destructive/save 승인으로 간주하지 않는다.
- Editor가 필요하지 않은 source·문서·offline/build 작업에서는 편의를 이유로 Editor를 자동 시작하지 않는다.
- 현재 작업 시작 전부터 실행 중이던 Editor나 start가 `already_running`으로 관측된 Editor는 사용자 소유 또는 ownership 불명으로 취급한다. prerequisite 확인과 허용된 live 작업에는 사용할 수 있지만 Browser가 자동 종료하지 않는다.
- Browser가 현재 작업에서 직접 시작한 것이 연속 evidence로 확인되고, 그 lifetime에 사용자 작업이 없으며 AI mutation이 rollback/cleanup 완료된 경우에는 known-unreliable graceful 대기를 반복하지 않고 `Tools/StopEditor.ps1 -DiscardUnsaved` 기반의 fixed AI-owned no-save stop을 정상 종료 경로로 사용할 수 있다.
- lifecycle 자동화는 Save/Save All을 수행하지 않는다. 사용자 작업 또는 ownership이 불명확하면 `-DiscardUnsaved`, `Stop-Process -Force`, `taskkill /F`와 동등한 폐기 종료를 사용하지 않고 중단한다.
- Editor stop/restart, project reopen, level/asset reload는 dirty/selection/open-asset/PIE 등 volatile Editor evidence를 무효화한다. 이후 작업은 현재 lifetime에서 prerequisite를 다시 확립한다.
- Build interference 해소를 위해 Editor를 종료한 뒤 새 binary의 live/PIE/UE MCP 검증이 필요하면 build PASS와 runtime PASS를 분리하고 fresh start → Ready 뒤 새 lifetime에서 검증한다.

- 상세 lifecycle·검증 기준은 `Document/CodeWorkGate.md`를 따른다.

# 이성수준 규칙


- 작업 난이도와 위험도에 따라 이성수준을 조절한다.
- 간단한 탐색, 파일 찾기, 짧은 설명은 낮음을 우선한다.
- 일반 구현, 소규모 수정, 테스트 보강은 중간을 기본으로 한다.
- 디버깅, 리뷰, 구조 변경, 다중 파일 영향 분석은 높음을 우선한다.
- 불필요하게 높은 이성수준을 남용하지 않는다.
- 세부 기준은 더 가까운 하위 AGENTS.md를 우선한다.

# Changelog

- 2026-08-19: Accepted `GoPyMCP.RuntimeRead`를 CarFight PIE 기술 검증의 기본 관측 수단으로 반영했다. RuntimeRead로 관측 가능한 runtime fact는 USER 확인보다 AI 직접 검증을 우선하고, USER Gate는 시각·UX·조작감·체감 판단에 유지한다. 관측 목적의 임시 Product debug surface/Consumer별 MCP 추가를 기본 경로에서 제외하고 RuntimeRead read-only/lifecycle·mutation 분리 경계를 고정했다.
- 2026-08-17: TC-00~09/Post-Closure Final Verification이 끝난 GoPyMCP UE MCP를 CarFight의 정식 기술 검증 수단으로 반영했다. `Persisted→AssetDump / Live·Unsaved·Runtime→UE MCP / 시각·감각→USER` 라우팅과 기존 Accepted capability 사용 시 TC replay 금지, 새 capability만 workflow-driven validation을 여는 원칙을 추가했다.
- 2026-08-15: 공식 Editor/plugin binary build 전에 runtime-build interference를 판정하고, 실제 file-lock/loaded-binary 충돌이 있을 때만 ownership-safe lifecycle을 적용하도록 추가했다. Build 뒤 live 검증은 fresh Ready lifetime에서 별도 증거로 수행한다.
- 2026-08-15: Browser fixed Editor start의 terminal PASS를 exact project process + editor-owned 8100 Ready까지 강화했다. Process 출현만으로 Ready를 주장하지 않으며, proven AI-owned + cleanup 완료 + 사용자 작업 부재 lifetime은 검증된 no-save discard stop을 direct 정상 종료 경로로 사용한다. User-owned/unknown force 금지는 유지한다.

- 2026-08-14: Browser-managed CarFight Editor lifecycle을 공통 실행 규칙으로 추가했다. Canonical RunEditor.bat 유지, AI-owned lifetime만 no-save 종료/폐기 허용, pre-existing/user-owned Editor 자동 종료 금지와 volatile evidence invalidation을 고정했다.
