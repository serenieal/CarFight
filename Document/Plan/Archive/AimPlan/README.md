# CarFight — AimPlan

> 역할: CarFight 싱글플레이 조준 시스템의 기획, 명세, 설계, 작업, 검증 기준을 묶어서 관리한다.
> 문서 버전: v0.4.0
> 마지막 정리(Asia/Seoul): 2026-06-19
> 상태: Draft / Single Player Planning

---

## 1. AimPlan의 목적

AimPlan은 CarFight의 조준 시스템을 단독 UI 기능이 아니라, 아래 시스템 사이의 전투 준비 계층으로 정의한다.

```text
Camera
  -> Aim
  -> Weapon
  -> Projectile / HitScan
  -> Damage
  -> Vehicle Combat
  -> Destruction / Part Damage
```

조준 시스템은 현재 구현된 `UCFVehicleCameraComp`와 미래의 `UCFVehicleWeaponComp`, Damage, Lock-On, AI Combat, Destruction 시스템 사이에 위치한다.

2026-06-19 기준 프로젝트 방향은 싱글플레이다. 따라서 이 폴더의 최신 문서는 로컬 조준, 로컬 발사 판정, 로컬 피드백, 싱글플레이 검증을 우선한다.

---

## 2. 상위 방향

CarFight의 프로젝트 방향은 다음 문서를 기준으로 한다.

| 기준 문서 | 역할 |
|---|---|
| `Document/ProjectSSOT/03_VisionAlign.md` | 프로젝트 최종 방향 / 현재 기준선 / 구조 갭 |
| `Document/ProjectSSOT/00_Handover.md` | 현재 실제 구현 기준선 |
| `Document/ProjectSSOT/01_Roadmap.md` | 현재 작업 우선순위와 완료 조건 |
| `Document/ProjectSSOT/16_CPP_DecisionLog.md` | C++ 중심 구조 결정 근거 |
| `Document/ProjectSSOT/Plan/CameraPlan/` | 차량 기반 3인칭 카메라 기준선 |
| `Document/ProjectSSOT/Plan/CameraDebugPlan/` | Camera Debug를 VehicleDebug에 연결한 방식 |
| `Document/ProjectSSOT/Plan/VehicleDebugPlan/` | Snapshot / PanelViewData / Navigation 구조 |
| `Document/ProjectSSOT/Plan/DataPlan/` | VehicleData 중심 데이터 분리 방향 |
| `Document/ProjectSSOT/Plan/SteeringPlan/` | 차량 조작과 카메라/조준 입력 분리 기준 |

---

## 3. 문서 구성

| 문서 | 역할 |
|---|---|
| `README.md` | AimPlan 문서 묶음 안내 |
| `CF_AimContext.md` | 기존 기능 / 미래 기능 / 싱글플레이 전투 흐름 관계 |
| `CF_AimNet.md` | Deprecated: 과거 멀티플레이 네트워크 기준 기록 |
| `CF_AimSpec.md` | 조준 시스템 기능 명세 |
| `CF_AimDesign.md` | C++ / BP 구조 설계 |
| `CF_AimTasks.md` | 구현 작업 순서 |
| `CF_AimVerify.md` | PIE Single Player / Standalone 검증 기준 |
| `CF_AimNameAudit.md` | 싱글플레이 전환 후 서버/RPC/복제 계열 코드명 감사 및 적용 결과 |
| `Generated/README.md` | 과거 자동 생성 문서 사용 금지/참고 기준 |

---

## 4. 핵심 정의

CarFight의 조준 시스템은 차량 기반 카메라 조준과 미래 무기 시스템 사이에 위치하는 싱글플레이 전투 준비 계층이다.

이 시스템은 플레이어 입력에 즉시 반응하는 조준감, Reticle 상태, 로컬 발사 가능 여부, 로컬 HitScan/Projectile 연결 준비를 제공한다.

멀티플레이용 RPC, 복제, Dedicated Server 검증은 현재 구현 목표에서 제외한다. 관련 문서는 과거 기록 또는 미래 온라인 전환 후보로만 취급한다.

---

## 5. 1차 구현 목표

1차 구현의 목적은 완성형 무기 시스템이 아니라, 싱글플레이에서 바로 검증 가능한 조준 코어를 여는 것이다.

포함한다.

- `UCFVehicleAimComp` 초안
- Local Aim State 계산
- Reticle State 계산
- 로컬 Fire Command / Fire Result 구조
- 로컬 조준각 검증
- 로컬 HitScan Trace 더미
- VehicleDebug Aim 표시

제외한다.

- 완성형 Damage System
- 완성형 Projectile 예측
- Lock-On
- AI Combat
- 부품 파괴 연동
- 멀티플레이 RPC / 복제 / Dedicated Server 검증

---

## 6. 설계 원칙 요약

- 판단 / 상태 / 규칙은 C++에 둔다.
- BP는 Thin BP와 시각 표현 레이어로 유지한다.
- CameraComp와 AimComp의 책임을 분리한다.
- 로컬 조준 계산과 로컬 발사 판정을 분리한다.
- 전투 결과는 싱글플레이 런타임의 로컬 전투 시스템이 확정한다.
- Aim Runtime State를 네트워크 전송 전제로 설계하지 않는다.
- Debug 없이 조준 시스템을 구현하지 않는다.
- 현재 `DA_PoliceCar` 기준으로 테스트하되, 단일 차량 전용 구조로 만들지 않는다.
- 장기적인 CMVS / Cluster Union / Geometry Collection 전환 가능성을 막지 않는다.

---

## ChangeLog

- v0.3.0 / 2026-06-19
  - 프로젝트 방향을 멀티플레이 대응에서 싱글플레이 기준으로 전환했다.
  - RPC, 복제, Dedicated Server 검증을 현재 구현 목표에서 제외했다.
  - `CF_AimNet.md`는 과거 네트워크 기준 기록으로 격하했다.
- v0.4.0 / 2026-06-19
  - Pawn 단위 서버/RPC Fire 경로 정리 완료 상태를 반영했다.
  - 남은 `BuildFireRequest` / `FCFVehicleFireRequest` 이름은 로컬 Fire Command 의미로 해석하도록 정리했다.

## 마이그레이션 지침

- 신규 Aim 작업은 `CF_AimSpec.md`, `CF_AimDesign.md`, `CF_AimTasks.md`, `CF_AimVerify.md`의 싱글플레이 기준을 따른다.
- `ServerRequestFire`, `ClientReceiveFireResult`, `RepAimVisual` 계열 Pawn/Aim 상태 이름은 현재 코드 기준에서 제거 또는 로컬 명칭으로 정리됐다.
- 남은 `BuildFireRequest` / `FCFVehicleFireRequest` 이름은 네트워크 요청이 아니라 로컬 Fire Command 데이터 의미로 해석한다.
- 추가 이름 정리는 별도 리팩터링 작업으로 분리하고, 먼저 에디터 저장 BP와 디버그 UI 이상 여부를 확인한다.

## Change Note

- 2026-06-19: v0.3.0의 “기존 명칭이 코드에 남아 있더라도 즉시 리네이밍하지 않는다” 지침을 현재 적용 완료 상태에 맞게 축소했다.
