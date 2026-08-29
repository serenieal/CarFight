# ProjectSSOT Archive 운영 가이드

- 문서 버전: v1.0
- 최근 갱신일: 2026-08-22
- 문서 상태: Current Archive Index

## 1. 목적

`Document/ProjectSSOT/Archive/`는 CarFight 프로젝트의 **과거 프로젝트 기준, 완료된 과도기 작업, 체크포인트, 운영 기록과 Current Systems에서 내려온 Historical 기록**을 보관한다.

Archive는 현재 상태·우선순위·현재 구현의 owner가 아니다.

```text
현재 프로젝트 판단 = Document/ProjectSSOT/README.md
현재 상태 = Document/ProjectSSOT/01_ProjectState.md
현재 순서 = Document/ProjectSSOT/02_Roadmap.md
기능 상태 = Document/ProjectSSOT/03_FeatureQueue.md
현재 구현 = Document/Systems/SystemIndex.md + 실제 Source/Asset
현재 작업 복원 = Document/ActiveWork.md
Historical Plan = Document/Plan/Archive/README.md
```

---

## 2. 하위 분류

### `Archive/LegacyBP/`
- 구형 `BP_ModularVehicle` 기준 절차
- 구형 BP 기반 핫픽스·조사 기록

### `Archive/CPP/`
- 완료된 C++ 전환 단계 문서
- helper/debug pipe 비교 기록
- 단일 축 전환 테스트 기록

### `Archive/Checkpoints/`
- 일자별 ProjectSSOT/P0 체크포인트 로그

### `Archive/Ops/`
- 과거 문서 정리·이동·운영 기록

### `Archive/Systems/`
- 한때 Systems에 있었지만 현재 구현 owner가 아니게 된 Historical 시스템 기록
- `Systems/Network/ServerSpawn.md`에서 내려온 Dedicated Server / Multiplayer Spawn 기록
- `Systems/UI/VehicleDebug.md`에서 내려온 구형 `WBP_VehicleDebug` 기준선 기록

---

## 3. 사용 원칙

- Archive 문서를 Current 구현 지시로 사용하지 않는다.
- Archive 내부의 `현재`, `다음`, `Pending`, `Active` 표현은 해당 문서가 작성된 당시 Historical 상태로 읽는다.
- 현재 상태·우선순위·검증 판정은 위 Current owner 문서를 먼저 확인한다.
- 과거 Acceptance, RCA, migration 이유가 필요할 때만 필요한 Archive 문서를 선택해서 읽는다.
- Current Systems에서 내려온 문서는 삭제하지 않고 당시 구현·회귀 비교 기준선으로 보존한다.
- Plan Historical evidence는 이 폴더가 아니라 별도 `Document/Plan/Archive/`가 소유한다.

---

## 4. 2026-08-22 Current Systems 하향 이동

| Archived Path | 이전 위치 | 분류 | 현재 판단 |
| --- | --- | --- | --- |
| `Systems/Network/ServerSpawn.md` | `Document/Systems/Network/ServerSpawn.md` | Decommissioned / Historical | 서버/멀티 작업 재개 결정 전까지 Current Systems가 아님 |
| `Systems/UI/VehicleDebug.md` | `Document/Systems/UI/VehicleDebug.md` | Legacy Baseline / Historical | 현재 디버그 UI는 `Document/Systems/UI/VehicleDebugPanel.md` 중심으로 판단 |

이 이동은 구현 파일 삭제나 기능 상태 변경이 아니다. 문서 역할을 실제 현재 상태에 맞춰 교정한 physical curation이다.

---

## 5. Changelog / Migration

### v1.0 - 2026-08-22

- 2026-03 기준으로 고정돼 있던 stale 활성 문서 목록을 제거하고 현재 ProjectSSOT/Systems/ActiveWork 진입 경로로 정상화했다.
- `ServerSpawn.md`와 구형 `VehicleDebug.md`를 `Archive/Systems/` Historical 기록으로 등록했다.
- ProjectSSOT Archive와 Plan Archive의 책임을 분리했다.

Migration: 과거 README의 `00_Handover.md`, `01_Roadmap.md`, `02_Conventions.md`, `08_P0_Verification.md`, `16_CPP_DecisionLog.md`, `18_DriveState_Tuning.md`, `19_DriveState_CoreChecklist.md` 활성 목록은 폐기한다. 현재 작업은 `ProjectSSOT/README.md`, `01_ProjectState.md`, `02_Roadmap.md`, `03_FeatureQueue.md`, `Systems/SystemIndex.md`, `ActiveWork.md`에서 시작한다.
