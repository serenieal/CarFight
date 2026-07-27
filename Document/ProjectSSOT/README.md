# ProjectSSOT 운영 가이드 (CarFight)

> 문서 버전: v2.8.0
> 마지막 정리(Asia/Seoul): 2026-07-27
> 문서 상태: Active
> 역할: `Document/ProjectSSOT/`의 읽기 순서, 문서 역할, 생명주기를 고정한다.

---

## 1. 목적

`Document/ProjectSSOT/`는 CarFight의 **프로젝트 전용 판단 기준**을 유지하는 SSOT다.

이 폴더의 목적은 문서를 많이 모으는 것이 아니라,
아래 질문에 흔들리지 않고 답할 수 있게 하는 것이다.

```text
- 이 프로젝트가 최종적으로 어디로 가는가?
- 지금 실제로 어디까지 구현되어 있는가?
- 다음에 무엇을 해야 하는가?
- 어떤 기능을 언제 착수할 것인가?
- 왜 그런 결정을 했는가?
- 완료된 기능을 어떻게 회귀 테스트할 것인가?
```

공통 규칙 원본은 `Document/SSOT/`에 둔다.
CarFight에만 필요한 판단은 `Document/ProjectSSOT/`에 둔다.

---

## 2. Document 폴더 역할 분리

CarFight 문서 구조는 아래처럼 분리한다.

| 위치 | 역할 |
|---|---|
| `Document/ActiveWork.md` | 현재 활성 작업, 마지막 작업 초점과 대표 Plan 연결 |
| `Document/ProjectSSOT/` | 프로젝트 현재 판단 기준 |
| `Document/Plan/` | 착수된 기능의 상세 구현·검증 계획과 작업 체크포인트 |
| `Document/Systems/` | 검증된 현재 구현 구조와 책임 |
| `Document/DesignSource/` | 원본 기획과 장기 북극성 |
| `Document/SSOT/` | 여러 프로젝트에 공통 적용되는 기준 |
| `Document/ProjectSSOT/Archive/` | 비활성 프로젝트 판단과 역사 기록 |
| `Document/Plan/Archive/` | 완료·보류·대체된 과거 Plan |

중요 원칙:

```text
ProjectSSOT = 무엇을 왜 해야 하는지 판단한다.
ActiveWork = 지금 어떤 활성 작업을 복원하거나 전환할지 선택한다.
Plan = 선택된 작업을 어떻게 구현하고 검증할지 정의한다.
Systems = 현재 실제로 어떻게 구현되어 있는지 설명한다.
DesignSource = 장기적으로 어디로 갈지 보여준다.
Archive = 현재 판단 또는 착수 기준에서 내려온 기록을 보존한다.
```

`ActiveWork.md`는 ProjectSSOT를 대체하지 않는다.
프로젝트 우선순위와 착수 승인은 Roadmap과 FeatureQueue가 결정하고, ActiveWork는 그중 현재 실제로 진행 중인 작업과 대표 Plan을 연결한다.
대표 Plan은 상세 체크포인트를 관리하며, 현재 구현 판정은 실제 코드·에셋과 Systems를 우선한다.

---

## 2-1. 2026-06-18 현재 전환 기준

현재 활성 개발 기준은 **멀티플레이 / Dedicated Server 계획에서 싱글 플레이 1대 차량 고도화로 전환**한다.

이번 전환의 목적은 아래와 같다.

```text
1. 서버, 멀티플레이, 2클라 검증, 서버 런처, 세션/로비 계획을 현재 일정에서 제외한다.
2. 현재 구현된 차량 1대를 클라이언트 사이드에서 더 완성도 있게 만든다.
3. 개발 일정을 약 2~3개월 단축한다.
4. 서버/멀티 문서는 삭제하지 않고 역사 기록 또는 Deferred 기준으로 남긴다.
```

현재 읽기 기준:
- 활성 판단은 `Document/ProjectSSOT/`를 우선한다.
- 완료된 실제 구현은 `Document/Systems/`를 우선한다.
- `Document/Plan/Archive/MP_ServerPlan`, `Document/Plan/Archive/ServerUpgradePlan`, `Document/Plan/Archive/VehicleNetSyncPlan`, `Document/Plan/Archive/CFNetSmoothPlan`은 **현재 착수 계획이 아니라 보류된 서버/네트워크 계획**으로 본다.
- Plan 활성/보관 기준은 `Document/Plan/README.md`와 `Document/Plan/Archive/README.md`를 함께 확인한다.

현재 세션의 전환 범위:
- 목표는 프로젝트 진행 선로를 멀티플레이에서 싱글플레이로 바꾸는 것이다.
- 기능 추가는 이번 전환 범위가 아니다.
- 기본 실행 경로, Aim/Fire 코드, 설정, 현재 기준 문서가 싱글플레이 기준과 충돌하지 않게 정리한다.
- 서버 Target, CFMPGameMode, CFNetSmooth, 보관된 서버 문서는 삭제하지 않고 현재 착수 기준에서 참고/보류 대상으로만 읽는다.
- 다음 개발 착수 판단은 싱글 1대 차량 기준으로만 한다.

---

## 2-2. 2026-07-27 현재 기능 개발 기준

싱글 실행, 조준·발사, Reticle/UI, 피격·피해, 조준 정렬, 고속 Projectile 신뢰성, 이중 레티클과 전투 FX는 완료 기반으로 본다.
현재 자동 선택된 전역 Active 작업은 없다. `CF-FQ-019 주행/전투 반복 테스트`가 다음 착수 Candidate이며, `CF-FQ-026 타겟 선택 시스템`은 `TS-P0-00~07 Done / TS-P0-08 Paused` 체크포인트를 보존한다.

현재 개발 순서:

```text
완료: CF-FQ-016 차량 무기 조준 및 발사
완료: CF-FQ-017 Reticle / FireFeedback UI
완료: CF-FQ-018 피격 판정 및 피해 처리
완료: CF-FQ-022 조준점·터렛·총구 정렬
완료: CF-FQ-023 고속 Projectile 연속 충돌
완료: CF-FQ-025 이중 레티클 및 터렛방향 시각화
완료: CF-FQ-024 전투 FX 구현 / User PIE PASS / CF-TC-021 PASS
다음 Candidate: CF-FQ-019 주행/전투 반복 테스트
일시중지: CF-FQ-026 타겟 선택 시스템 / TS-P0-08
후속 Candidate: CF-FQ-020 조작감/전투 템포/피드백 개선
후속 Candidate: CF-FQ-021 핵심 게임 루프 검증
```

프로젝트 결정 `CF-PDL-0009`에 따라 CarFight는 게임 사운드를 지원하지 않는다.
`CF-FQ-024`는 발사·Impact·최초 파괴의 Niagara 기반 시각 연출을 완료했으며 현재 구현 기준은 `Document/Systems/Combat/CombatFx.md`다.
완료 Plan 경로는 `Document/Plan/CombatFxAudio/ImplementationDesign.md`를 유지하지만 `CombatFxAudio`는 레거시 디렉터리명이며 Audio 구현을 뜻하지 않는다.
전체 순서는 `Document/ProjectSSOT/02_Roadmap.md`, 착수 상태는 `03_FeatureQueue.md`, 현재 작업 선택은 `Document/ActiveWork.md`를 우선한다.

---

## 3. 현재 활성 문서 최소 집합

`Document/ProjectSSOT/` 루트에서 바로 보는 활성 문서는 아래 6개와 `README.md`로 제한한다.

| 순서 | 문서 | 역할 |
|---:|---|---|
| 0 | `00_Vision.md` | 최종 방향 / 현재 구조 갭 / 유지·교체 원칙 |
| 1 | `01_ProjectState.md` | 현재 실제 기준선 / 임시 운영 편차 / 현재 리스크 |
| 2 | `02_Roadmap.md` | 큰 진행 순서 / 우선순위 / 완료 조건 |
| 3 | `03_FeatureQueue.md` | 기능 후보 / 착수 판단 / 클라·서버·관리툴 필요성 |
| 4 | `04_ProjectDecisions.md` | 프로젝트 전체 결정 로그 |
| 5 | `05_TestChecklist.md` | 완료된 Systems 기준 최소 회귀 테스트 |

루트에 새 문서를 추가하기 전에는 반드시 아래를 먼저 검토한다.

```text
- 기존 00~05 문서에 흡수 가능한가?
- 진행 중 기능 계획이면 Document/Plan/에 둬야 하는가?
- 완료된 기능 기록이면 Document/Systems/에 둬야 하는가?
- 역사 기록이면 Archive로 보내야 하는가?
```

---

## 4. 먼저 읽는 순서

AI나 사용자가 CarFight 현재 상태를 파악할 때는 아래 순서로 읽는다.

```text
1. Document/ProjectSSOT/README.md
2. Document/ProjectSSOT/00_Vision.md
3. Document/ProjectSSOT/01_ProjectState.md
4. Document/ProjectSSOT/02_Roadmap.md
5. Document/ProjectSSOT/03_FeatureQueue.md
6. Document/ProjectSSOT/04_ProjectDecisions.md
7. Document/ProjectSSOT/05_TestChecklist.md
```

세부 구현 기준이 필요하면 그 다음에 `Document/Systems/`를 본다.
진행 중 기능의 상세 계획이 필요하면 그때만 `Document/Plan/`을 본다.

---

## 5. 핵심 기준 문장

아래 문장은 활성 문서들에서 서로 충돌하면 안 된다.

### 5.1 최종 방향

```text
CarFight는 차량 전투와 물리 상호작용을 중심으로,
장기적으로 CMVS / Cluster Union / Geometry Collection 기반의
다차종 차량 전투 구조를 목표로 한다.
```

### 5.2 현재 기준선

```text
현재 실제 구현 기준선은
ACFVehiclePawn + UCFVehicleDriveComp + UCFWheelSyncComp + UCFVehicleData + DA_PoliceCar 조합이다.
```

현재 차량 구조는 최종 구조가 아니라,
`ChaosWheeledVehicle` 기반 하이브리드 구조다.

### 5.3 유지 예정 코어

```text
- VehicleData
- DriveState
- WheelSync
- Thin BP 원칙
- VehicleCamera
- Local Aim / Reticle 표시
- Local Fire 골격
```

### 5.4 교체 / 확장 후보

```text
- Vehicle root 구조
- 차량 조립 방식
- 파괴 / 분리 표현 구조
- 서버 권한 전투 처리: 현재 일정 제외 / 장기 보류
- 로드아웃 / 인벤토리 / 보상 저장 구조: 현재 일정 제외
- 관리툴 / 운영 도구: 현재 일정 제외
```

---

## 6. 문서 생명주기

## 6.1 기능 후보 단계

기능 후보는 `03_FeatureQueue.md`에 등록한다.

```text
기능 후보
→ Document/ProjectSSOT/03_FeatureQueue.md
```

이 단계에서는 상세 설계를 하지 않는다.
클라이언트 / 서버 / 관리툴 / 데이터 / 테스트 필요성만 판단한다.

---

## 6.2 개발 착수 단계

착수하기로 결정한 기능은 `Document/Plan/<기능명>/`에 상세 계획을 만든다.

```text
개발 착수
→ Document/Plan/<기능명>/
```

Plan 문서는 앞으로 개발할 기능의 계획만 담는다.
완료된 구현의 기준 문서로 쓰지 않는다.

---

## 6.3 개발 완료 단계

기능 개발이 완료되면 현재 구현 기준을 `Document/Systems/<분류>/<기능명>.md`에 기록한다.

```text
개발 완료
→ Document/Systems/<분류>/<기능명>.md
```

완료된 Plan 문서는 필요하면 `Document/Plan/Archive/`로 이동한다.

---

## 6.4 역사 기록 단계

더 이상 현재 판단 기준이 아닌 문서는 Archive로 보낸다.

```text
비활성 / 역사 기록
→ Document/ProjectSSOT/Archive/
```

Archive 문서는 삭제가 아니라 보존이다.
다만 현재 판단 기준으로 읽지 않는다.

---

## 7. Archive 운영 원칙

`Archive/`는 역사 기록 보존 구역이다.

| 위치 | 역할 |
|---|---|
| `Archive/LegacyBP/` | 구형 BP 기준 절차 / 핫픽스 / 조사 기록 |
| `Archive/CPP/` | 완료된 C++ 전환 단계 문서 / 과도기 비교 기록 |
| `Archive/Checkpoints/` | 일자별 체크포인트 / 기준선 판정 기록 |
| `Archive/Ops/` | 문서 정리 / 이동 실행 기록 |
| `Archive/Merged/` | 활성 문서에 흡수된 문서 |

Archive를 읽을 때는 다음 원칙을 따른다.

```text
- 현재 기준은 루트 00~05 문서를 우선한다.
- Archive 문서는 과거 판단의 근거로만 본다.
- Archive 내용이 현재 Systems 문서와 충돌하면 Systems 문서를 우선한다.
```

---

## 8. Systems 사용 원칙

`Document/Systems/`는 완료된 기능의 현재 구현 기준이다.

예:

```text
Document/Systems/Vehicles/VehicleDrive.md
Document/Systems/Vehicles/WheelSync.md
Document/Systems/Vehicles/VehicleCoreDecisions.md
Document/Systems/Network/ServerSpawn.md
Document/Systems/Input/Input.md
Document/Systems/UI/AimReticle.md
```

기능 구현을 수정할 때는 관련 `Systems` 문서를 먼저 확인한다.
수정 후 기능의 책임이나 동작 방식이 바뀌면 해당 `Systems` 문서도 갱신한다.

---

## 9. Plan 사용 원칙

`Document/Plan/`은 앞으로 개발할 기능의 상세 계획 구역이다.

Plan은 현재 구현 기준이 아니다.

```text
Plan에 있는 내용 = 앞으로 할 계획
Systems에 있는 내용 = 현재 완료된 구현
ProjectSSOT에 있는 내용 = 프로젝트 판단 기준
```

AI가 현재 구현을 확인할 때 Plan을 기준으로 단정하면 안 된다.

Plan의 공식 진입과 활성·보관 분류는 다음 색인을 사용한다.

```text
Document/Plan/README.md
Document/Plan/Archive/README.md
```

Plan 루트 색인은 폴더와 대표 진입 문서 단위로만 관리한다.
Plan 하위에 Draft, Working, Notes 또는 세부 작업 파일이 추가될 때마다 이 ProjectSSOT README를 갱신하지 않는다.
새 Plan 폴더가 공식 착수 대상으로 승격될 때만 `Document/Plan/README.md`를 갱신한다.

---

## 10. 문서 정리 결과

2026-06-02 정리 결과:

```text
03_VisionAlign.md        → 00_Vision.md
00_Handover.md           → 01_ProjectState.md
01_Roadmap.md            → 02_Roadmap.md
02_FeatureQueue.md       → 03_FeatureQueue.md
03_ProjectDecisionLog.md → 04_ProjectDecisions.md
04_TestChecklist.md      → 05_TestChecklist.md
08_P0_Verification.md    → Archive/Checkpoints/P0_Verification_20260401.md
16_CPP_DecisionLog.md    → Document/Systems/Vehicles/VehicleCoreDecisions.md
```

삭제한 문서는 없다.
루트 SSOT 역할을 벗어난 문서는 보존 위치로 이동했다.

---

## 11. 변경 이력

### v2.8.0 - 2026-07-27

```text
- CF-FQ-024 전투 FX의 Done / User PIE PASS / CF-TC-021 PASS를 현재 완료 기준선에 반영했다.
- 현재 자동 선택된 Active 작업이 없고 CF-FQ-019가 다음 착수 Candidate임을 반영했다.
- CF-FQ-026 타겟 선택 시스템을 TS-P0-08 Paused 체크포인트로 동기화했다.
- 전투 FX 현재 구현 기준을 Document/Systems/Combat/CombatFx.md로 연결했다.
```

### v2.7.0 - 2026-07-24

```text
- 현재 전역 Active를 CF-FQ-026 타겟 선택 시스템으로 동기화했다.
- CF-FQ-024를 사운드 없는 Niagara 기반 전투 FX 전용 Ready 기능으로 정리했다.
- CombatFxAudio 경로가 레거시 디렉터리명이며 Audio 구현을 뜻하지 않는다고 명시했다.
- 프로젝트 전역 사운드 비지원 결정 CF-PDL-0009를 읽기 기준에 반영했다.
```

### v2.6.0 - 2026-07-22

```text
- CF-FQ-025 이중 레티클 및 터렛방향 시각화의 Done / User PIE PASS를 현재 완료 기준선에 반영
- Image_CenterDot 조준 레티클과 CurrentMuzzleDirection 기반 Image_WeaponReticle 터렛 레티클 책임을 완료 계약으로 추가
- CF-FQ-024를 현재 활성 기능과 신규 구현 시작점으로 유지
- 현재 개발 기준 날짜와 문서 버전을 최신 상태로 동기화
```

### v2.5.0 - 2026-07-15

```text
- 완료된 전투 판정 기준선과 현재 CF-FQ-024 전투 FX / Audio 활성 상태를 반영
- CF-FQ-017 Reticle/UI와 CF-FQ-024 Niagara/공간 사운드를 별도 기능 생명주기로 분리
- 현재 진행 순서를 CF-FQ-024 → CF-FQ-019 → CF-FQ-020 → CF-FQ-021로 갱신
- CombatFxAudio 대표 Plan을 현재 활성 상세 설계로 연결
```

### v2.4.0 - 2026-07-14

```text
- Document/ActiveWork.md를 현재 활성 작업과 대표 Plan 연결 계층으로 역할 표에 추가
- ProjectSSOT, ActiveWork, Plan, Systems, DesignSource와 Archive의 책임 경계 명시
- ActiveWork가 Roadmap과 FeatureQueue의 우선순위·착수 판단을 대체하지 않는다고 명시
- 대표 Plan은 상세 체크포인트, 실제 코드·에셋과 Systems는 현재 구현 판정을 담당하도록 정리
```

### v2.3.0 - 2026-07-14

```text
- Plan 루트와 Archive 루트의 폴더 단위 색인 운영 원칙 추가
- Plan 하위 개별 파일 추가 시 ProjectSSOT README를 갱신하지 않는 규칙 추가
- CombatPlan 안내 경로를 실제 위치인 Document/ProjectSSOT/CombatPlan/으로 정정
```

### v2.2.0 - 2026-06-19

```text
- 현재 개발 기준을 싱글 로컬 전투 루프 검증 순서로 갱신
- 차량 무기 조준/발사, 발사 피드백, 피격/피해, 반복 테스트, 템포 개선, 핵심 루프 검증 순서를 ProjectSSOT 읽기 기준에 추가
- Local Fire 골격을 유지 예정 코어에 추가
```

### v2.1.4 - 2026-06-19

```text
- 사용자 에디터 확인 결과를 반영해 싱글플레이 전환 세션을 완료 판정 대상으로 정리
- 기능 추가 후보는 이번 세션 범위에서 제외하고 후속 선택지로만 유지
```

### v2.1.3 - 2026-06-19

```text
- 현재 세션 범위를 기능 추가가 아니라 싱글플레이 전환 정리로 고정
- Aim/Fire 코드와 현재 기준 문서는 싱글플레이 기준으로 맞추되, 서버 Target/보관 문서는 삭제하지 않는 기준으로 조정
```

### v2.1.2 - 2026-06-19

```text
- 현재 세션의 목표를 코드 삭제가 아니라 멀티플레이 진행 선로에서 싱글플레이 진행 선로로 바꾸는 문서 전환으로 명시
- 서버/멀티 관련 코드, Target, 플러그인, 배치 파일은 현상 유지하고 현재 착수 기준에서만 제외하도록 기준 추가
```

### v2.1.1 - 2026-06-19

```text
- Plan 루트/Archive 안내 문서 신설에 맞춰 서버/네트워크 보류 문서 경로를 Archive 기준으로 정정
- 현재 착수 기준 확인 시 Document/Plan/README.md와 Document/Plan/Archive/README.md를 함께 보도록 안내 추가
```

### v2.1.0 - 2026-06-18

```text
- 활성 개발 기준을 싱글 플레이 1대 차량 고도화로 전환
- 서버/멀티/2클라/서버 런처/세션 계획을 현재 일정 제외 항목으로 명시
- 유지 예정 코어에서 서버 Spawn/Possess를 제거하고 VehicleCamera / Local Aim / Reticle을 추가
- 서버 관련 Plan 문서군을 현재 착수 계획이 아닌 보류된 역사/계획 자료로 해석하도록 기준 추가
```

### v2.0.0 - 2026-06-02

```text
- ProjectSSOT 루트 활성 문서를 00~05 체계로 재정렬
- ProjectSSOT / Plan / Systems / Archive 생명주기 명확화
- P0 상세 검증 문서를 Archive/Checkpoints로 이동한 사실 기록
- 차량 C++ 코어 결정 로그를 Systems/Vehicles로 이관한 사실 기록
- 기존 ProjectSSOT/Plan 경로 설명 제거
```

### v1.1.0 - 2026-04-15

```text
- 문서 버전 / 마지막 정리 날짜 추가
- 당시 CameraPlan 보조 실행 계획 문서 연결
```

---

## 12. Migration

### v2.8.0 적용 안내

```text
- 새 세션에서 자동 Active 작업을 가정하지 않는다.
- 다음 신규 착수 후보는 CF-FQ-019이며 사용자가 선택한 뒤 Active로 전환한다.
- CF-FQ-026 재개 시 Document/Plan/TargetSelectPlan.md의 TS-P0-08 체크포인트에서 시작한다.
- CF-FQ-024 현재 구현 판단은 Document/Systems/Combat/CombatFx.md를 우선한다.
```

### v2.7.0 적용 안내

```text
- 새 세션은 CF-FQ-026과 TargetSelect 대표 Plan을 먼저 복원한다.
- CF-FQ-024 재개 시 CombatFxAudio 경로의 ImplementationDesign을 사용하되 시각 FX 전용으로 해석한다.
- SoundWave, SoundCue, MetaSound, Sound Attenuation, USoundBase, UAudioComponent와 오디오 모듈을 추가하지 않는다.
- 과거 Audio 관련 Changelog와 Migration은 역사 기록으로만 읽는다.
```

### v2.6.0 적용 안내

```text
- CF-FQ-025는 완료된 Systems 기준으로 읽고 ReticleAimDirection Plan을 활성 구현 계획으로 해석하지 않는다.
- 현재 신규 구현은 CF-FQ-024와 Document/Plan/CombatFxAudio/ImplementationDesign.md에서 시작한다.
- 투사체 착탄 위치의 월드 공간 3D 표시는 별도 Feature 결정 전까지 현재 착수 대상으로 해석하지 않는다.
```

### v2.5.0 적용 안내

```text
- CF-FQ-017은 Reticle / FireFeedback UI 완료 기능으로 유지한다.
- 실제 발사·Impact·파괴 Niagara와 공간 사운드는 CF-FQ-024가 소유한다.
- 현재 활성 Plan은 Document/Plan/CombatFxAudio/ImplementationDesign.md다.
- CombatFxAudio는 구현·빌드·사용자 PIE 완료 전까지 Systems 현재 기능으로 읽지 않는다.
- 다음 반복 테스트 CF-FQ-019는 CF-FQ-024 완료 후 착수한다.
```

### v2.4.0 적용 안내

```text
- 기존 ProjectSSOT 루트 00~05 문서와 읽기 순서는 변경하지 않는다.
- ActiveWork는 프로젝트 판단 문서가 아니라 현재 활성 작업과 대표 Plan을 연결하는 보조 계층으로 사용한다.
- 프로젝트 우선순위와 착수 여부는 계속 Roadmap과 FeatureQueue가 결정한다.
- 대표 Plan은 상세 작업 체크포인트를 관리하며, 현재 구현 판정은 실제 코드·에셋과 Systems를 우선한다.
- 전체 문서 권한 우선순위와 상태 모델은 Document/Document_Entry.md v2.4를 기준으로 한다.
```
