# CarFight — AimPlan

> 역할: CarFight 조준 시스템의 기획, 네트워크 원칙, 명세, 설계, 작업, 검증 기준을 묶어서 관리한다.  
> 문서 버전: v0.2.0  
> 마지막 정리(Asia/Seoul): 2026-05-21  
> 상태: Draft / Planning

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
  -> Multiplayer Replication
```

조준 시스템은 현재 구현된 `UCFVehicleCameraComp`와 미래의 `UCFVehicleWeaponComp`, Damage, Lock-On, AI Combat, Destruction 시스템 사이에 위치한다.

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
| `CF_AimContext.md` | 기존 기능 / 미래 기능 / 빅픽쳐 / 멀티플레이 관계 |
| `CF_AimNet.md` | 서버 권한, RPC, 복제, 예측, 검증 기준 |
| `CF_AimSpec.md` | 조준 시스템 기능 명세 |
| `CF_AimDesign.md` | C++ / BP 구조 설계 |
| `CF_AimTasks.md` | 구현 작업 순서 |
| `CF_AimVerify.md` | PIE / Listen Server / Client 검증 기준 |

---

## 4. 핵심 정의

CarFight의 조준 시스템은 차량 기반 카메라 조준과 미래 무기 시스템 사이에 위치하는 멀티플레이 대응 전투 준비 계층이다.

이 시스템은 로컬 클라이언트에서 빠른 조준감과 Reticle 예측을 제공하고, 서버에서는 발사 요청을 검증해 실제 전투 결과를 확정한다.

또한 다른 클라이언트에게 필요한 최소 조준 시각 상태를 복제하여, 무기 방향, 발사 이펙트, 피격 결과가 네트워크 환경에서도 일관되게 보이도록 한다.

---

## 5. 1차 구현 목표

1차 구현의 목적은 완성형 무기 시스템이 아니라, 멀티플레이까지 확장 가능한 조준 코어를 여는 것이다.

포함한다.

- `UCFVehicleAimComp` 초안
- Local Aim State 계산
- Server Aim State 구조 준비
- Replicated Aim Visual State 구조 준비
- Reticle State 계산
- Server Fire Request 더미
- 서버 조준각 검증
- 서버 HitScan Trace 더미
- VehicleDebug Aim / Network 표시

제외한다.

- 완성형 Damage System
- 완성형 Projectile 예측
- Lock-On
- AI Combat
- 부품 파괴 연동
- 대규모 네트워크 최적화

---

## 6. 설계 원칙 요약

- 판단 / 상태 / 규칙은 C++에 둔다.
- BP는 Thin BP와 시각 표현 레이어로 유지한다.
- CameraComp와 AimComp의 책임을 분리한다.
- 로컬 조준감과 서버 권한 판정을 분리한다.
- 전투 결과는 서버가 확정한다.
- Aim Runtime State 전체를 무조건 복제하지 않는다.
- Debug 없이 조준 시스템을 구현하지 않는다.
- 현재 `DA_PoliceCar` 기준으로 테스트하되, 단일 차량 전용 구조로 만들지 않는다.
- 장기적인 CMVS / Cluster Union / Geometry Collection 전환 가능성을 막지 않는다.
