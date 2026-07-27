# CarFight — 04_ProjectDecisions

> 문서 버전: v1.7.0
> 작성일(Asia/Seoul): 2026-07-24
> 문서 상태: Active
> 역할: CarFight 프로젝트 전체의 **구조 / 서버 / 관리툴 / 운영 / 데이터 흐름 결정**을 기록한다.

---

## 0. 현재 우선순위 결정 — 전투 FX 활성화

### 결정

```text
- CF-FQ-024 전투 FX를 현재 단일 Active 작업으로 유지한다.
- 현재 단계는 Phase 0 FAB FX 자산 반입과 후보 선별이다.
- CF-FQ-026 타겟 선택 시스템은 TS-P0-00~07 Done / TS-P0-08 Paused 상태로 보존한다.
- CF-FQ-024는 Document/Plan/CombatFxAudio/ImplementationDesign.md와 AssetPreparationChecklist.md를 대표 체크포인트로 사용한다.
```

### 영향

```text
- ActiveWork와 ProjectSSOT는 CF-FQ-024를 우선 복원한다.
- 첫 작업은 반입된 FAB 콘텐츠의 NiagaraSystem 목록과 의존성을 확인하고 Muzzle·Impact·Destroyed 후보를 확정하는 것이다.
- 기존 직접 조준, 발사, 피해, 이중 레티클과 TargetSelect 판정을 FX 연출이 다시 계산하거나 변경하지 않는다.
- CF-FQ-026의 코드·에셋·Automation 결과와 TS-P0-08 결함 체크포인트는 그대로 유지한다.
```

---

## 0-1. CF-PDL-0009 — CarFight는 게임 사운드를 지원하지 않는다

- 상태: `Accepted`
- 결정일: 2026-07-24
- 대체 범위: `CF-PDL-0008`의 Audio·공간 사운드·Sound 자산 계약

### 결정

```text
- CarFight 제품 범위에서 게임 사운드를 제외한다.
- CF-FQ-024는 전투 FX 전용 기능으로 변경한다.
- SoundWave, SoundCue, MetaSound Source와 Sound Attenuation 자산을 제작하거나 도입하지 않는다.
- USoundBase, UAudioComponent와 사운드 재생 API를 게임 런타임에 추가하지 않는다.
- AudioMixer, Audio Modulation과 별도 오디오 플러그인 의존성을 추가하지 않는다.
- 엔진음, 타이어음, 충돌음, 무기음, 피격음, 파괴음, UI음과 BGM을 완료 조건으로 요구하지 않는다.
```

### 이유

```text
- 제품 방향상 사운드가 필요하지 않다는 사용자 결정이 확정됐다.
- 현재 저장소에는 게임 사운드 에셋과 런타임이 없어 제거 마이그레이션 비용이 없다.
- 구현 전에 범위를 닫으면 불필요한 DataAsset 필드, AudioComponent 생명주기, 믹싱과 콘텐츠 제작 비용을 예방할 수 있다.
```

### 영향

```text
- 기존 CF-PDL-0008의 판정과 연출 분리 원칙 및 Niagara 데이터 기반 구조는 유지한다.
- CF-PDL-0008의 Audio 관련 필드, 클래스, 에셋과 테스트 계약은 이 결정으로 대체한다.
- Unreal 기본 플랫폼 오디오 설정은 엔진 생성 설정으로 유지할 수 있지만 게임 기능으로 간주하지 않는다.
- 향후 AI와 개발 작업은 사운드 기능을 Candidate, 품질 후속 또는 누락 기능으로 자동 제안하지 않는다.
```

### 변경 조건

```text
제품 방향이 다시 변경되고 사용자가 게임 사운드 지원을 명시적으로 재승인한 경우에만 재검토한다.
단순 품질 개선, 외부 사운드 자산 확보 또는 엔진 기본 기능 존재만으로 이 결정을 변경하지 않는다.
```

---

## 0-2. CF-PDL-0010 — 세미리얼 FAB 우선 FX 제작과 위치·생명주기 규칙을 사용한다

- 상태: `Accepted`
- 결정일: 2026-07-24
- 적용 기능: `CF-FQ-024`

### 결정

```text
- CarFight 전투 FX의 최종 미술 방향은 세미리얼로 한다.
- 1인 개발 일정에 맞춰 직접 신규 제작보다 FAB FX 팩 활용을 기본으로 한다.
- FAB 원본 자산은 직접 수정하지 않고, 필요한 경우 /Game/CarFight/FX/Adapted에 복제한다.
- P0는 Muzzle, Impact와 Destroyed의 기능적 가독성을 우선하며 고급 미술 통일은 후속으로 둔다.
- 메시에 고정된 위치는 Mesh Socket, 충돌로 결정되는 위치는 World Transform, 런타임 피벗은 SceneComponent를 사용한다.
- CarFight의 Muzzle과 FX_Exhaust 소켓 +X축은 FX 방출 방향으로 통일한다.
- FAB Niagara의 축 차이는 소켓을 바꾸지 않고 FX DataAsset RotationOffset으로 보정한다.
- 소켓이 없으면 ProjectileData의 Fallback Relative Transform을 사용한다.
- Muzzle, Impact와 Destroyed 일회성 FX는 UCFCombatFxComp가 요청한다.
- Projectile 추진 화염과 Trail 지속형 FX는 ACFProjectileActor가 직접 소유하고 Pool 반환 전에 정지·초기화한다.
- P0에서는 Projectile Trail 자연 소멸, 별도 FX Pool, 다중 노즐과 표면별 Impact를 구현하지 않는다.
```

### 이유

```text
- 전담 FX 아티스트가 없는 1인 개발 환경에서 Niagara를 처음부터 제작하면 일정 위험이 크다.
- 외부 완성 자산을 DataAsset으로 교체 가능하게 연결하면 코드 재작업 없이 후보를 바꿀 수 있다.
- 위치 유형을 구분하지 않으면 무기·발사체마다 소켓, 컴포넌트와 월드 좌표 예외가 늘어난다.
- Projectile Actor는 이미 Pool 생명주기를 소유하므로 지속형 FX 초기화도 같은 주체가 담당하는 것이 안전하다.
- P0에서 자연 소멸과 별도 FX Pool까지 구현하면 시각 품질보다 생명주기 복잡도가 먼저 증가한다.
```

### 영향

```text
- 대표 설계는 Document/Plan/CombatFxAudio/ImplementationDesign.md v0.3.0 이상을 사용한다.
- FAB 자산 반입과 후보 선별은 Document/Plan/CombatFxAudio/AssetPreparationChecklist.md를 사용한다.
- UCFCombatFxData는 Niagara 선택과 Scale / RotationOffset을 소유한다.
- UCFProjectileData는 TrailAttachSocketName과 TrailFallbackRelativeTransform 같은 메시 구조 정보를 소유한다.
- ACFProjectileActor에는 지속형 NiagaraComponent와 Activate / Deactivate / Reset 수명이 추가될 수 있다.
- CarFight_Re C++에서 Niagara 타입을 사용하기 전 Build.cs에 Niagara 모듈 의존성을 추가한다.
- FX 자산 또는 소켓 누락은 발사, 이동, 충돌, 피해와 파괴 판정을 실패시키지 않는다.
```

### 변경 조건

```text
전담 FX 인력이 추가되거나, 현재 FAB 자산으로 제품 품질 목표를 달성할 수 없다는 반복 검증 결과가 있을 때 제작 정책을 재검토한다.
멀티플레이 원격 FX, 대규모 동시 전투 또는 자연 소멸 Trail이 실제 플레이 가독성에 필수라고 확인되기 전에는 P0 생명주기 규칙을 확장하지 않는다.
```

---

## 1. 목적

이 문서는 CarFight 프로젝트 전체에서 확정한 중요한 결정을 기록한다.

기존 `VehicleCoreDecisions.md`는 차량 C++ 코어 중심 결정 로그로 유지한다.
이 문서는 그보다 넓은 범위를 다룬다.

```text
04_ProjectDecisions.md
= 프로젝트 전체 결정 로그

VehicleCoreDecisions.md
= 차량 C++ 코어 결정 로그
```

이 문서의 목적은 아래와 같다.

```text
- 같은 논의를 반복하지 않는다.
- 왜 특정 방향을 선택했는지 남긴다.
- 나중에 결정을 바꿀 수 있는 조건을 명확히 한다.
- AI가 다음 작업을 시작할 때 현재 프로젝트 판단 기준을 빠르게 읽게 한다.
```

---

## 2. 결정 상태 표기

| 상태 | 의미 |
|---|---|
| `Proposed` | 제안됨. 아직 확정 아님 |
| `Accepted` | 확정됨 |
| `Deferred` | 보류됨 |
| `Rejected` | 기각됨 |
| `Replaced` | 다른 결정으로 대체됨 |

---

## 3. 결정 범위

이 문서는 아래 범위의 결정을 기록한다.

```text
- ProjectSSOT / Plan / Systems 문서 생명주기
- 기능 개발 착수 기준
- 클라이언트 / 서버 / 관리툴 개발 타이밍
- 싱글 플레이 우선 전환과 서버/멀티 보류 기준
- 싱글 로컬 전투 루프 구현/검증 순서
- Dedicated Server 이후 멀티플레이 구조 방향이 재개될 조건
- 데이터 저장 / 로드아웃 / 인벤토리 방향
- 운영 도구 / 관리툴 도입 기준
- 완료 기능의 Systems 승격 기준
```

아래 범위는 기존 문서에 우선 기록한다.

```text
- 차량 C++ 코어 구조 유지/교체 결정: VehicleCoreDecisions.md
- 특정 진행 중 기능의 세부 결정: Document/Plan/<기능명>/DecisionLog.md
- 완료된 기능의 현재 구현 설명: Document/Systems/<분류>/<기능명>.md
```

---

## 4. 결정 목록

## CF-PDL-0001 — 문서 생명주기는 ProjectSSOT / Plan / Systems로 분리한다

- 상태: `Accepted`
- 결정일: 2026-06-02

### 결정

```text
Document/ProjectSSOT/
= 프로젝트 현재 판단 기준

Document/Plan/
= 앞으로 개발할 기능의 상세 계획

Document/Systems/
= 개발 완료된 기능의 현재 구현 기준

Document/ProjectSSOT/Archive/
= 비활성/역사 기록
```

### 이유

```text
Plan과 Systems가 섞이면 AI가 현재 구현과 미래 계획을 혼동한다.
ProjectSSOT가 상세 계획을 많이 품으면 현재 상태 판단 기준이 흐려진다.
따라서 프로젝트 판단 기준, 진행 중 계획, 완료 시스템 기록을 분리한다.
```

### 영향

```text
- 새로운 기능 후보는 03_FeatureQueue.md에 먼저 등록한다.
- 상세 계획이 필요할 때 Document/Plan/<기능명>/를 만든다.
- 구현 완료 후 현재 구현 기준은 Document/Systems/<분류>/<기능명>.md로 승격한다.
```

### 변경 조건

```text
문서 수가 과도하게 늘어나거나, Plan과 Systems 사이의 이동 비용이 개발 속도를 크게 방해할 때 재검토한다.
```

---

## CF-PDL-0002 — 기능 개발은 기술 영역이 아니라 기능 단위로 자른다

- 상태: `Accepted`
- 결정일: 2026-06-02

### 결정

```text
클라이언트 기간 / 서버 기간 / 관리툴 기간으로 나누지 않는다.
기능 하나를 기준으로 클라이언트 / 서버 / 관리툴 / 데이터 / 테스트 필요성을 함께 판단한다.
```

### 이유

```text
1인 개발에서 클라이언트만 오래 만들면 나중에 서버 권한 구조와 충돌할 수 있다.
반대로 서버를 기능과 분리해서 크게 만들면 실제 게임 기능과 맞지 않는 인프라가 될 수 있다.
따라서 기능 단위 수직 절단 방식이 적합하다.
```

### 영향

```text
- 기능 후보는 03_FeatureQueue.md에서 관리한다.
- 기능 착수 시 Plan에는 클라이언트 / 서버 / 관리툴 / 데이터 / 테스트 범위를 함께 적는다.
- 완료 조건도 기능 단위로 정의한다.
```

### 변경 조건

```text
팀원이 늘어나서 클라이언트/서버/운영 역할을 분리할 수 있게 되면 재검토한다.
```

---

## CF-PDL-0003 — Plan은 진행 중 기능 계획이며 완료 후 Systems로 승격한다

- 상태: `Accepted`
- 결정일: 2026-06-02

### 결정

```text
Document/Plan/은 앞으로 개발할 기능의 상세 계획만 담는다.
기능 개발이 완료되면 핵심 현재 구현 기준은 Document/Systems/로 옮긴다.
완료된 Plan 문서는 Document/Plan/Archive/로 이동한다.
```

### 이유

```text
완료된 기능의 기준 문서가 Plan에 남아 있으면, 현재 구현과 과거 계획이 섞인다.
AI가 기능을 수정할 때도 Plan이 아니라 Systems를 기준으로 읽어야 한다.
```

### 영향

```text
- 완료 기능을 질문할 때는 Systems를 우선 확인한다.
- 진행 중 기능을 질문할 때만 Plan을 확인한다.
- Plan은 ProjectSSOT 비교 대상에서 제외한다.
```

### 변경 조건

```text
특정 기능이 아직 완료되지 않았지만 현재 구현 기준으로 임시 고정해야 할 경우, Systems가 아니라 Plan 안의 CurrentState로만 관리한다.
```

---

## CF-PDL-0004 — 관리툴은 독립 제품처럼 먼저 만들지 않는다

- 상태: `Accepted`
- 결정일: 2026-06-02

### 결정

```text
관리툴은 처음부터 웹 대시보드로 만들지 않는다.
반복 작업이 3회 이상 발생할 후보를 03_FeatureQueue.md의 관리툴 후보 큐에 먼저 등록한다.
초기에는 콘솔 명령 / CLI / 임시 서버 명령으로 대체한다.
```

### 이유

```text
초기 데이터 구조가 불안정한 상태에서 정교한 관리툴을 만들면 계속 갈아엎게 된다.
1인 개발에서는 운영 반복 비용이 실제로 발생한 뒤 얇게 자동화하는 것이 안전하다.
```

### 영향

```text
- 테스트 계정 초기화, 유저 상태 조회, 보상 지급/회수, 전투 로그 조회는 후보로만 관리한다.
- 정식 관리툴 기능은 계정/인벤토리/보상/로그 데이터 구조가 안정화된 뒤 착수한다.
```

### 변경 조건

```text
서버 테스트 반복 비용이 급격히 커지거나, 수동 DB/로그 확인으로 테스트가 막히면 우선순위를 올린다.
```

---

## CF-PDL-0005 — 서버는 게임 규칙 집행자이며 클라이언트 감각과 분리한다

- 상태: `Replaced`
- 결정일: 2026-06-02
- 대체 결정: `CF-PDL-0006`

### 결정

```text
서버는 Spawn, Possess, 소유권, 권한 있는 게임 상태, 전투 결과 검증을 담당한다.
클라이언트는 입력, 카메라, UI, 사운드, 이펙트, 조준 감각을 담당한다.
```

### 이유

```text
CarFight는 직접 조준 감각과 차량/터렛/락온/자원 운용이 함께 중요한 게임이다.
따라서 클라이언트 조작감은 가볍게 보면 안 되지만, 보상/전투 결과/소유권 같은 권한 판단은 서버가 담당해야 한다.
```

### 영향

```text
- 발사 요청은 클라이언트 입력과 서버 권한 처리를 분리해서 설계한다.
- 피드백은 로컬 예측/시각 효과와 서버 판정 결과를 구분한다.
- Dedicated Server에서 UI, 카메라, LocalPlayer, 로컬 사운드/이펙트 코드는 실행하지 않는다.
```

### 변경 조건

```text
싱글플레이 전용 프로젝트로 방향이 바뀌거나, 서버 권한 구조를 포기하는 경우 재검토한다.
```

### 현재 상태

```text
2026-06-18 기준으로 이 결정은 현재 활성 개발 기준이 아니다.
서버 권한 구조 자체를 영구 폐기한 것은 아니지만, 현재 일정에서는 싱글 플레이 1대 차량 고도화 결정이 우선한다.
```

---

## CF-PDL-0006 — 현재 개발 기준은 싱글 플레이 1대 차량 고도화로 전환한다

- 상태: `Accepted`
- 결정일: 2026-06-18

### 결정

```text
현재 개발 일정에서는 서버 / 멀티플레이 / Dedicated Server 검증 / 2클라 테스트 / 서버 런처 / 세션/로비를 제외한다.
우선순위는 클라이언트 사이드에서 1대 차량의 주행감, 카메라, 로컬 조준, Reticle, WheelSync 시각 품질을 고도화하는 것으로 전환한다.
```

### 이유

```text
외부 피드백에서 서버, 멀티플레이 부분을 삭제하고 1대 차량 구현을 고도화하라는 방향이 제시됐다.
현재 서버/멀티 계획을 계속 진행하면 2~3개월 규모의 추가 검증과 도구 작업이 발생한다.
반대로 싱글 차량 고도화에 집중하면 현재 구현된 VehicleData / DriveState / WheelSync / VehicleCamera / Local Aim 코어를 살리면서 체감 결과물을 빠르게 만들 수 있다.
```

### 영향

```text
- 03_FeatureQueue.md의 P0는 싱글 실행 기준선, 1대 차량 주행감, 카메라/로컬 조준 고도화로 변경한다.
- 02_Roadmap.md는 해당 결정 당시 싱글 전환 흐름을 우선했다.
- 2026-06-19 이후 신규 개발 순서는 CF-PDL-0007의 C1~C6 전투 루프 흐름을 우선한다.
- 05_TestChecklist.md의 Network 항목은 현재 사이클에서 N/A 또는 Deferred로 내린다.
- CFMPGameMode, CarFight_ReServer.Target.cs, CFNetSmooth, CFServerLauncher, RunNet*.bat는 이번 문서 전환 세션에서 삭제하거나 분류하지 않는다.
- Document/Plan의 서버/네트워크 계획 문서는 현재 착수 계획이 아니라 역사/보류 계획으로 해석한다.
- 이번 결정의 핵심은 멀티플레이 진행 선로를 싱글플레이 진행 선로로 바꾸는 것이며, 코드 정리는 별도 작업으로 분리한다.
```

### 변경 조건

```text
싱글 차량 데모 기준선이 닫힌 뒤 다시 멀티플레이가 필요하다는 제품 방향이 확정되면 재검토한다.
그 전까지는 서버/멀티 기능을 P0/P1 작업으로 올리지 않는다.
```

---

## CF-PDL-0007 — 다음 개발 기준은 싱글 로컬 전투 루프 검증으로 둔다

- 상태: `Accepted`
- 결정일: 2026-06-19

### 결정

```text
싱글 실행 기준선과 로컬 Aim / Fire / Reticle 골격을 선행 기반으로 본다.
다음 개발은 차량 무기 조준/발사, 발사 피드백, 피격/피해, 주행/전투 반복 테스트, 조작감/전투 템포/피드백 개선, 핵심 게임 루프 검증 순서로 진행한다.
```

### 이유

```text
현재 기준 차량 조작과 로컬 조준/발사 골격은 싱글플레이 기준에서 확인됐다.
다음 체감 성과는 조준 자체가 아니라, 발사 입력이 피격과 피해 결과로 이어지고 플레이어가 그 결과를 즉시 이해하는 데서 나온다.
따라서 서버 권한 구조나 무기 종류 확장을 먼저 열지 않고, 기준 차량 1대의 로컬 전투 루프를 먼저 닫는다.
```

### 영향

```text
- 02_Roadmap.md의 신규 진행 순서는 C1~C6 전투 루프 단계로 재정렬한다.
- 03_FeatureQueue.md의 최우선 후보는 CF-FQ-016~CF-FQ-021로 이동한다.
- 05_TestChecklist.md에는 Combat / Feedback / CoreLoop 검증 항목을 추가한다.
- 기존 주행감, WheelSync, 카메라 품질 항목은 전투 반복 테스트와 피드백 개선 단계에서 함께 관찰한다.
- 서버 권한 발사, 2클라 판정, PvP 판정, 세션/로비, 보상/경제는 계속 Deferred로 둔다.
```

### 변경 조건

```text
싱글 로컬 전투 루프가 PASS로 닫힌 뒤, 멀티플레이 또는 AI 교전이 제품 방향상 필요하다고 확정되면 재검토한다.
그 전까지는 서버/멀티 전투 구현을 P0/P1로 올리지 않는다.
```

---

## CF-PDL-0008 — Reticle/UI 피드백과 실제 전투 FX를 분리하고 데이터 기반 연출 계층으로 구현한다

- 상태: `Accepted`
- 결정일: 2026-07-15

### 결정

```text
CF-FQ-017은 Reticle 색상, 상태 문구와 쿨다운 UI를 소유하는 완료 기능으로 유지한다.
실제 발사, Impact와 차량 파괴 Niagara는 CF-FQ-024가 별도 기능으로 소유한다.
발사 승인, 첫 Blocking Hit과 최초 Destroyed 전환은 기존 전투 C++ 판정이 소유하고, 연출 계층은 확정된 결과를 다시 계산하지 않는다.
FX 자산은 WeaponData, ProjectileData, VehicleData가 참조하는 공용 연출 DataAsset에서 선택하며 소스 코드에 자산 경로를 하드코딩하지 않는다.
```

### 이유

```text
UI 피드백과 월드 연출을 한 기능으로 묶으면 완료된 Reticle 상태를 다시 열게 되고, 전투 판정과 자산 제작의 생명주기가 섞인다.
현재 발사·피격·피해·파괴 판정은 검증됐으므로 연출은 그 결과를 소비하는 독립 계층이어야 한다.
DataAsset 기반 참조를 사용하면 1인 개발 환경에서 C++ 재빌드 없이 Niagara 자산과 시각 파라미터를 교체·튜닝할 수 있다.
```

### 영향

```text
- CF-FQ-024를 P0 Active 기능으로 추가한다.
- 대표 Plan은 Document/Plan/CombatFxAudio/ImplementationDesign.md를 사용한다.
- C++는 호출 시점, 요청 데이터, null 안전성, 중복 방지와 Projectile Pool 초기화를 담당한다.
- Blueprint와 Unreal Editor 자산은 Niagara, Material, Decal과 DataAsset 연결을 담당한다.
- 연출 자산 누락 또는 재생 실패는 발사·피해·파괴 결과를 취소하지 않는다.
- P0는 발사, 첫 Impact와 최초 파괴의 1회성 연출까지만 포함하며 고급 믹싱·표면 분기·Geometry Collection 파괴는 후속으로 둔다.
- 완료 검증은 CF-TC-021이 소유한다.
```

### 변경 조건

```text
멀티플레이 원격 연출 복제, 물리 표면별 Impact 또는 대규모 Presentation Subsystem이 필요해지면 데이터 소유와 호출 주체를 재검토한다.
그 전까지 판정과 연출 분리, 데이터 기반 자산 참조 원칙을 유지한다.
```

---

## 5. 후속 결정 후보

아래 항목은 아직 확정하지 않았다.

```text
- 싱글용 GameMode를 새로 만들지, 기존 기본 실행 흐름을 조정할지
- `CFMPGameMode`와 `CarFight_ReServer.Target.cs`를 장기 보존할지 별도 브랜치/Archive 기준으로 분리할지
- CFNetSmooth 서브모듈을 현재 프로젝트에서 언제 다시 볼지
- FAB 자산 조사 후 Vehicle Impact와 World Impact를 P0에서 분리할지
- Projectile Trail 자연 소멸과 별도 FX Pool을 언제 도입할지
- 다중 미사일 노즐과 다중 FX 소켓 배열을 언제 지원할지
- 물리 표면별 Impact FX 분기를 언제 도입할지
- 파괴 후 차량에 부착되는 지속 연기와 화염을 언제 도입할지
```

---

## 6. 문서 갱신 조건

아래 결정이 생기면 이 문서를 갱신한다.

```text
- 문서 구조나 생명주기 변경
- 기능 착수 기준 변경
- 서버/클라이언트 책임 경계 변경
- 싱글 우선 / 서버 보류 기준 변경
- 관리툴 도입 기준 변경
- 데이터 저장 구조의 큰 방향 변경
- Systems 승격 기준 변경
```

---

## 7. 문서 버전 관리

- 현재 문서 버전: `v1.7.0`
- 문서 상태: `Active`

### 버전 증가 기준

| 버전 | 기준 |
|---|---|
| Major | 결정 로그의 역할/범위 자체 변경 |
| Minor | 새 결정 항목 추가 또는 기존 결정의 의미 확장 |
| Patch | 표현 정리, 오탈자 수정, 링크 보강 |

---

## 8. 체인지로그

### v1.7.0 - 2026-07-24

```text
- 현재 우선순위 결정을 CF-FQ-026 TargetSelect에서 CF-FQ-024 전투 FX 활성화로 변경했다.
- CF-FQ-024의 현재 단계를 Phase 0 FAB FX 자산 반입과 후보 선별로 고정했다.
- CF-FQ-026은 TS-P0-00~07 Done / TS-P0-08 Paused로 보존했다.
- ActiveWork와 ProjectSSOT가 CombatFxAudio Plan과 AssetPreparationChecklist를 우선 복원하도록 변경했다.
```

### v1.6.0 - 2026-07-24

```text
- CF-PDL-0010 세미리얼 FAB 우선 FX 제작과 위치·생명주기 결정을 Accepted로 추가했다.
- FAB 원본 보존, 최소 수정, CarFight Adapted 복제 경로와 P0 범위 제한을 고정했다.
- 고정 위치 Socket, 충돌 위치 World Transform, 런타임 피벗 SceneComponent 구분을 공식화했다.
- Muzzle과 FX_Exhaust +X 방출 축, RotationOffset 보정과 Fallback Relative Transform 계약을 추가했다.
- Projectile 지속형 FX를 ACFProjectileActor가 소유하고 Pool 반환 전 즉시 초기화하도록 결정했다.
- CF-PDL-0008에 남아 있던 과거 Audio 표현을 CF-PDL-0009 기준에 맞춰 시각 FX 전용으로 정리했다.
- 후속 결정 후보에서 사운드 관련 항목을 제거하고 실제 FX 확장 후보로 교체했다.
```

### v1.5.0 - 2026-07-24

```text
- CF-PDL-0009 게임 사운드 비지원 결정을 Accepted로 추가했다.
- CF-PDL-0008의 Audio 관련 계약을 CF-PDL-0009로 대체하고 Niagara 기반 시각 연출 원칙만 유지했다.
- CF-FQ-024를 전투 FX 전용 Ready 기능으로 변경했다.
- 향후 AI와 개발 작업이 사운드를 후보나 누락 기능으로 자동 제안하지 않도록 고정했다.
```

### v1.3.0 - 2026-07-15

```text
- CF-PDL-0008 판정과 전투 FX/Audio 분리 및 데이터 기반 연출 계층 결정을 추가했다.
- CF-FQ-017 UI와 CF-FQ-024 Niagara/공간 사운드 기능 경계를 고정했다.
- 완료된 무기·발사·피해 관련 후속 결정 후보를 제거하고 Audio/FX 후속 후보로 갱신했다.
- CF-TC-021을 전투 FX/Audio 완료 검증으로 연결했다.
```

### v1.2.0 - 2026-06-19

```text
- CF-PDL-0007 싱글 로컬 전투 루프 검증 결정을 추가
- 차량 무기 조준/발사, 발사 피드백, 피격/피해, 반복 테스트, 템포 개선, 핵심 루프 검증 순서를 결정 로그에 고정
- 후속 결정 후보를 첫 기준 무기, 발사 방식, 피해 상태 위치, 피드백 최소 범위 중심으로 갱신
```

### v1.1.1 - 2026-06-19

```text
- CF-PDL-0006의 영향을 코드 삭제/분류가 아니라 진행 선로 전환으로 명확히 했다.
- CFMPGameMode, CarFight_ReServer.Target.cs, CFNetSmooth, CFServerLauncher, RunNet*.bat는 이번 문서 전환 세션에서 현상 유지한다고 명시했다.
- 후속 결정 후보에서 즉시 삭제 판단처럼 읽히는 표현을 장기 보존/분리 검토 표현으로 조정했다.
```

### v1.1.0 - 2026-06-18

```text
- CF-PDL-0005 서버 권한 구조 결정을 현재 활성 기준에서 Replaced로 변경
- CF-PDL-0006 싱글 플레이 1대 차량 고도화 전환 결정을 추가
- 서버/멀티/관리툴 관련 후속 결정 후보를 Deferred 판단 항목으로 재정렬
- 이전 FeatureQueue 파일명 참조를 현재 파일명 `03_FeatureQueue.md`로 정정
```

### v1.0.0 - 2026-06-02

```text
- ProjectDecisionLog 문서 최초 작성
- ProjectSSOT / Plan / Systems 문서 생명주기 결정 기록
- 기능 단위 수직 절단 개발 방식 결정 기록
- 관리툴 도입 기준 결정 기록
- 서버/클라이언트 책임 경계 결정 기록
```

---

## 9. Migration

### v1.7.0 적용 안내

```text
- 새 세션은 CF-FQ-024와 CombatFxAudio Plan 문서 2개를 우선 복원한다.
- Phase 0에서는 FAB Niagara 후보 조사와 선별만 수행하고 판정 C++를 변경하지 않는다.
- CF-FQ-026은 TS-P0-08 Paused이며 TargetSelect 변경은 사용자가 다시 주력 작업으로 전환할 때 재개한다.
- 전투 FX 구현은 CF-PDL-0009 사운드 비지원과 CF-PDL-0010 FAB 우선 규칙을 함께 따른다.
```

### v1.6.0 적용 안내

```text
- CF-FQ-024 재개 시 ImplementationDesign.md와 AssetPreparationChecklist.md를 함께 읽는다.
- FAB 팩을 프로젝트에 반입하기 전에는 구체 Niagara 자산명이나 성능을 추정으로 확정하지 않는다.
- 외부 FX 자산은 DataAsset으로 연결하고 원본 폴더를 직접 수정하지 않는다.
- 소켓 누락을 기능 실패로 처리하지 않고 Fallback Transform을 사용한다.
- Projectile 지속형 FX 초기화는 ACFProjectileActor와 Projectile Pool 수명 안에서 처리한다.
- Trail 자연 소멸, 다중 노즐, 표면별 Impact와 지속 파괴 FX는 P0 완료 조건이 아니다.
```

### v1.5.0 적용 안내

```text
- CF-PDL-0009가 CF-PDL-0008의 모든 Audio 계약보다 우선한다.
- CF-FQ-024와 CombatFxAudio 경로는 시각 FX 전용으로 해석한다.
- Sound 자산, Audio 런타임과 오디오 모듈을 구현하거나 테스트하지 않는다.
- 제품 방향이 사용자에 의해 명시적으로 재변경되기 전에는 사운드 기능을 재활성화하지 않는다.
```

### v1.3.0 적용 안내

```text
- CF-FQ-017의 완료 상태와 Systems 문서는 유지한다.
- 실제 전투 FX / Audio 신규 구현은 CF-FQ-024와 CombatFxAudio Plan을 사용한다.
- 연출 자산 미연결 상태를 전투 판정 실패로 처리하지 않는다.
- CF-FQ-024 완료 전에는 신규 CombatFxAudio Systems 문서를 만들거나 Current로 승격하지 않는다.
```
