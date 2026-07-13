# SystemIndex

- Version: 1.3.0
- Date: 2026-07-09
- Status: Active
- Scope: `Document/Systems/` 하위 문서 위치 안내 색인

---

## 1. 문서 목적

이 문서는 `Document/Systems/` 폴더 안에 있는 시스템 문서들이 각각 **어떤 기능을 설명하는 문서인지** 빠르게 찾기 위한 색인이다.

이 문서는 설계 로드맵이나 개발 순서표가 아니다.
문서를 열기 전에 아래 표에서 필요한 기능을 찾고, 해당 경로의 문서를 확인하면 된다.

---

## 2. 최상위 문서

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/SystemIndex.md` | `Document/Systems/` 하위 문서들이 어디에 있고, 어떤 기능을 다루는지 정리한 색인 문서다. |
| `Document/Systems/SystemTemplate.md` | 새 시스템 문서를 작성할 때 사용하는 기본 템플릿이다. 문서 목적, 범위, 현재 역할, 책임, 비책임, 갱신 조건, 버전 관리를 어떤 형식으로 적을지 정의한다. |

---

## 3. Combat 폴더

현재 Combat 폴더 문서는 **현재 구현된 전투 런타임과 싱글플레이 로컬 전투 피드백 기준**을 기록한다.
실제 HP 차감, 차량 파괴, 장갑/모듈 피해, 서버 권한 전투처럼 아직 구현 기준으로 확인되지 않은 내용은 현재 Systems 문서 범위에 넣지 않는다.

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Combat/WeaponFire.md` | 싱글플레이 로컬 차량 Pawn에서 Fire 입력을 발사 명령으로 만들고, 무기/장착/발사체 데이터를 해석해 Projectile Actor 또는 Dummy HitScan 경로로 넘기며, 발사 결과와 거부 사유를 Aim / Debug / 후속 UI 피드백이 읽을 수 있게 남기는 현재 발사 기능 문서다. |
| `Document/Systems/Combat/FireFeedback.md` | `WeaponFire`가 남긴 로컬 발사 성공/실패/쿨다운/무기 없음 결과를 Reticle, 간단한 HUD, VFX, SFX 피드백으로 표시하기 위한 현재 기준 문서다. |
| `Document/Systems/Combat/Projectile.md` | `ProjectileData`, 공통 `CFProjectileActor`, `ProjectilePoolComp`를 통해 발사체를 활성화, 이동, 충돌/수명 종료, Pool 반환 Debug까지 처리하는 현재 발사체 기능 문서다. |
| `Document/Systems/Combat/DamageHitContext.md` | Dummy HitScan과 Projectile Actor 충돌 결과를 같은 `FCFDamageHitContext` 형식으로 기록하고 VehicleDebug에서 읽을 수 있게 하는 현재 Debug 중심 피해 컨텍스트 문서다. |

---

## 4. Config 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Config/ProjectRuntimeConfig.md` | 프로젝트 시작 맵, 렌더링 기술, 하드웨어 타깃, 입력 백엔드, 축 기본값 같은 런타임 환경 설정을 설명하는 문서다. |

---

## 5. Input 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Input/Input.md` | 차량 입력 자산, 입력 매핑 컨텍스트, 장치 모드, 2D 입력, Legacy 입력 충돌 제어, 최종 차량 주행/카메라 입력 전달 흐름을 설명하는 문서다. |

---

## 6. Maps 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Maps/.gitkeep` | 현재 `Maps` 폴더 유지를 위한 빈 파일이다. 기능 설명 문서는 아직 없다. |

---

## 7. Network 폴더

현재 싱글 플레이 1대 차량 고도화 기준에서 Network 폴더 문서는 활성 구현 지시가 아니라 보류된 서버/멀티 기록으로 본다.

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Network/ServerSpawn.md` | 현재는 Deferred 기록이다. Dedicated Server 테스트 환경에서 플레이어 로그인 후 서버가 기본 차량 Pawn을 스폰하고 PlayerController가 Possess하도록 만드는 최소 멀티플레이 진입 기능을 설명한다. |

---

## 8. UI 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/UI/AimReticle.md` | 로컬 차량 Pawn의 `VehicleAimComp`에서 Reticle 상태를 읽어 화면 표시 텍스트와 위젯 가시성으로 변환하고, 후속 weapon fire UI 단계에서 로컬 발사 결과 피드백까지 표시할 수 있는 조준 UI 기능 문서다. |
| `Document/Systems/UI/DisplayTextPolicy.md` | 내부 식별자는 영문으로 유지하고, 화면에 보이는 UI/Debug UI 텍스트는 한국어로 표시한다는 표시 텍스트 정책 문서다. |
| `Document/Systems/UI/VehicleDebug.md` | 차량 Pawn의 런타임 준비 상태, Drive 상태, 입력 상태, 마지막 상태 전이, 런타임 요약을 문자열로 조합해 로컬 플레이어 위젯으로 표시하는 텍스트 기반 차량 진단 기능 문서다. |
| `Document/Systems/UI/VehicleDebugPanel.md` | `VehicleDebug Panel`의 Navigation + Selected Section 구조, TopLevel Section, Camera Debug 편입 상태, 표시 언어 정책을 설명하는 문서다. |

---

## 9. Vehicles 폴더

| 경로 | 문서 내용 |
| --- | --- |
| `Document/Systems/Vehicles/VehicleAim.md` | `VehicleCamera`가 만든 조준 결과를 Local 표시, 로컬 발사 검증 상태, 로컬 시각화 상태로 나누어 관리하는 차량 조준 중간 계층 문서다. |
| `Document/Systems/Vehicles/VehicleCamera.md` | Look 입력을 차량 기준 누적 조준 상태로 변환하고, 카메라 모드, Aim Profile, 속도, 충돌 상태를 반영해 SpringArm, FOV, AimTrace를 계산/적용하는 차량 카메라 기능 문서다. |
| `Document/Systems/Vehicles/VehicleCoreDecisions.md` | 현재 차량 코어의 유지 결정, 교체 결정, 임시 운영 판단을 기록하는 결정 로그 문서다. 차량 코어 변경 전 확인해야 하는 기준 문서다. |
| `Document/Systems/Vehicles/VehicleData.md` | 차량 하나의 외형, 주행 성격, 휠 물리, 휠 시각 구성, Wheel Class 참조, Drive 상태 판정 기준을 하나의 DataAsset으로 묶어 공급하는 차량 구성 데이터 문서다. |
| `Document/Systems/Vehicles/VehicleDrive.md` | 차량 입력을 Chaos Vehicle Movement에 적용하고, 속도/방향/접지/입력 상태를 바탕으로 DriveState를 계산/유지하는 주행 상태 기능 문서다. |
| `Document/Systems/Vehicles/VehiclePawnLegacy.md` | `CFModVehiclePawn / BP_ModularVehicle` 계열을 현재 주력 차량 Pawn이 아닌 레거시 계열로 정리하는 문서다. |
| `Document/Systems/Vehicles/VehicleRuntime.md` | 차량 Pawn이 BeginPlay 시점에 VehicleData를 실제 주행/휠/Drive 설정에 반영하고, Drive/WheelSync 준비를 검증한 뒤 런타임 Ready 상태를 관리하는 문서다. |
| `Document/Systems/Vehicles/VehicleSteering.md` | 게임패드 VehicleMove 2D 입력 방향을 목표 조향으로 해석하고, 제한 속도와 차량 속도 기반 중립 복귀 규칙을 거쳐 실제 조향값을 적용하는 문서다. |
| `Document/Systems/Vehicles/WheelSync.md` | 실제 Movement와 휠 회전 상태를 읽어 각 휠의 조향, 서스펜션, 스핀 시각 입력을 만들고 Anchor/Mesh 컴포넌트에 적용하는 휠 시각 동기화 문서다. |

---

## 10. 기능별로 찾기

| 찾고 싶은 내용 | 확인할 문서 |
| --- | --- |
| 현재 로컬 발사 명령, 무기 데이터 해석, 쿨다운, FireOrigin, 발사 결과 기록 | `Combat/WeaponFire.md` |
| 발사 성공/실패/쿨다운/무기 없음 상태를 Reticle, HUD, VFX, SFX로 표시하는 기준 | `Combat/FireFeedback.md` |
| Projectile Actor 활성화, 이동, 충돌, 수명 종료, Pool 반환 | `Combat/Projectile.md` |
| Dummy HitScan / Projectile 충돌 결과 Debug 기록 | `Combat/DamageHitContext.md` |
| 프로젝트 시작 맵, 렌더링, 입력 백엔드 설정 | `Config/ProjectRuntimeConfig.md` |
| 입력 액션, 매핑 컨텍스트, 키보드/게임패드 입력 처리 | `Input/Input.md` |
| 보류된 서버 접속 후 차량 Pawn 생성과 Possess 기록 | `Network/ServerSpawn.md` |
| UI 텍스트를 한국어로 표시하는 기준 | `UI/DisplayTextPolicy.md` |
| 차량 디버그 위젯의 기본 문자열 표시 | `UI/VehicleDebug.md` |
| 차량 디버그 패널의 탭/섹션 구조 | `UI/VehicleDebugPanel.md` |
| 조준점/Reticle 표시, 로컬 발사 결과 피드백 표시 후보 | `UI/AimReticle.md` |
| 차량 코어 변경 전 결정 기준 | `Vehicles/VehicleCoreDecisions.md` |
| 차량 DataAsset 구조 | `Vehicles/VehicleData.md` |
| 차량 BeginPlay 준비와 Ready 판정 | `Vehicles/VehicleRuntime.md` |
| 차량 주행 입력 적용과 DriveState | `Vehicles/VehicleDrive.md` |
| 게임패드 2D 조향 해석과 조향 복귀 | `Vehicles/VehicleSteering.md` |
| 바퀴 위치, 조향 피벗, 스핀, 휠 시각 동기화 | `Vehicles/WheelSync.md` |
| 차량 카메라, FOV, AimTrace | `Vehicles/VehicleCamera.md` |
| 차량 조준 상태, 로컬 발사 검증용 Aim 상태, OutOfArc / 조준각 경고 기준 | `Vehicles/VehicleAim.md` |
| 예전 `BP_ModularVehicle` 계열 정리 | `Vehicles/VehiclePawnLegacy.md` |

---

## 11. 문서 추가 시 갱신 규칙

`Document/Systems/` 아래에 새 시스템 문서를 추가하면 이 문서도 함께 갱신한다.

갱신할 위치:

1. 해당 폴더 섹션의 문서 표
2. 필요한 경우 `기능별로 찾기` 표
3. 아래 Changelog

---

## 12. Changelog

### v1.3.0 - 2026-07-09

- `Document/Systems/Combat/FireFeedback.md` 신규 문서를 Combat 폴더 색인에 추가했다.
- 기능별 찾기 표에 발사 성공/실패/쿨다운/무기 없음 상태를 표시하는 기준 문서로 `Combat/FireFeedback.md`를 추가했다.
- `Vehicles/VehicleAim.md` 설명에서 서버 검증 / 복제 시각화 표현을 제거하고 로컬 발사 검증 상태 / 로컬 시각화 상태 기준으로 정정했다.
- `UI/AimReticle.md`, `Combat/WeaponFire.md` 설명을 Reticle / FireFeedback 교통정리 이후 기준에 맞춰 보강했다.

### v1.2.0 - 2026-07-09

- `Document/Systems/Combat/` 섹션을 추가했다.
- 현재 구현된 전투 기능 문서 `WeaponFire.md`, `Projectile.md`, `DamageHitContext.md`를 색인에 등록했다.
- 아직 구현 기준으로 확인되지 않은 HP 차감, 파괴, 장갑/모듈 피해, 서버 권한 전투는 Combat Systems 현재 범위에 포함하지 않는다고 명시했다.

### v1.1.0 - 2026-06-19

- 싱글 플레이 1대 차량 고도화 전환 기준에 맞춰 Network 폴더를 Deferred 기록으로 표시했다.
- 기능별 찾기 표에서 서버 Spawn/Possess 항목이 현재 활성 작업처럼 보이지 않게 정리했다.

### v1.0.0 - 2026-06-02

- `Document/Systems/` 하위 문서 위치 안내 색인 문서로 최초 작성했다.
- 각 폴더별 문서 경로와 문서가 다루는 기능을 표로 정리했다.
- 기능별로 어떤 문서를 열면 되는지 찾기 표를 추가했다.
