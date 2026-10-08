# CarFight — CF_AimStatus

> 역할: AimPlan의 현재 완료 상태, 남은 작업, 다음 권장 작업을 한눈에 보기 위해 정리한다.
> 문서 버전: v1.5.0
> 마지막 정리(Asia/Seoul): 2026-06-19
> 상태: Completed / Single Player Aim Core Phase 1 + Local Fire Rename

---

## 1. 현재 상태 요약

Aim 시스템 1차 구현은 완료 상태다.

완료된 범위는 다음이다.

```text
Aim Core
  -> 완료

Reticle C++ / WBP 연결
  -> 완료

Fire Command / Local Validation
  -> 완료

Local Dummy HitScan
  -> 완료

Single Player 기본 확인
  -> 완료

Legacy Network Name Audit
  -> 완료

Legacy Network Name Cleanup
  -> 완료

Aim Net Serialization Cleanup
  -> 완료
```

아직 완료로 보지 않는 범위는 다음이다.

```text
Standalone Game 검증
Fire Visual / FX 연결
Damage 적용
WeaponComp 분리
Projectile
Lock-On
AI Combat
Part Damage / Destruction 연동
```

---

## 2. 완료된 C++ 범위

| 단계 | 내용 | 상태 |
|---:|---|---:|
| R1/R2 | Aim 타입 + AimComp 골격 | 완료 |
| R3 | Local Aim 계산 | 완료 |
| R4 | VehicleDebug Aim | 완료 |
| R5/R6 | Fire Command + 로컬 검증 | 완료 |
| R7 | Local HitScan 더미 | 완료 |
| R8 | Fire Visual 기반 | 완료 |
| R9 | Reticle UI C++ 부모 | 완료 |
| R10 | Reticle Widget Pawn 연결 준비 | 완료 |

---

## 3. 완료된 UE Editor 연결 범위

| 항목 | 상태 |
|---|---:|
| `WBP_AimReticle` 생성 | 완료 |
| `WBP_AimReticle` Parent Class 설정 | 완료 |
| `Text_ReticleState` 배치 | 완료 |
| `Text_CanFire` 배치 | 완료 |
| `Text_ReticleHint` 배치 | 완료 |
| `IA_Fire` 생성 | 완료 |
| `IMC_Vehicle_Default`에 Fire 매핑 | 완료 |
| `BP_CFVehiclePawn.InputAction_Fire` 연결 | 완료 |
| `BP_CFVehiclePawn.AimReticleWidgetClass` 연결 | 완료 |
| Single Player Fire 입력 확인 | 완료 |

---

## 4. 현재 기능 흐름

현재 Aim 시스템 흐름은 다음과 같다.

```text
Look 입력
  -> CameraComp Aim Trace
  -> AimComp Local Aim 계산
  -> ReticleState 계산
  -> WBP_AimReticle 표시

Fire 입력
  -> IA_Fire
  -> HandleFireStarted()
  -> BuildFireCommand()
  -> ValidateFireCommand()
  -> RunLocalDummyHitScan()
  -> FireResult 생성
  -> ApplyFireResult()
  -> FireValidationState / AimVisualState 갱신
  -> VehicleDebug Aim에서 확인
```

주의:

Pawn 단위 서버/RPC wrapper는 제거됐다. `UCFVehicleAimComp::BuildFireRequest()`와 `FCFVehicleFireRequest` 계열 이름은 아직 남아 있지만, 현재 의미는 네트워크 요청이 아니라 로컬 Fire Command 데이터다.

---

## 5. 현재 제약

현재 Fire는 실제 전투 결과를 만들지 않는다.

즉, 아래는 아직 없다.

```text
총구 이펙트
발사 사운드
Hit Marker
실제 Damage
탄약
쿨다운
재장전
무기 그룹
Projectile
```

따라서 현재 Fire는 다음 목적의 더미 기능이다.

```text
Aim Target 확인
발사 명령 확인
로컬 검증 확인
로컬 Trace 확인
Debug 상태 확인
```

---

## 6. 다음 권장 작업

바로 기능을 추가하기보다 다음 순서를 권장한다.

```text
1. 싱글플레이 전환 완료 판정 기록
2. 필요 시 Standalone Game 후속 검증
```

이번 세션 제외:

```text
Fire FX / 사운드 연결
HitScan Damage 더미
WeaponComp 설계
Projectile
Lock-On
AI Combat
```

다음 문서 후보:

```text
CF_AimVerify.md
CF_AimNameAudit.md
```

이 문서는 싱글플레이 검증 절차를 다룬다.

---

## 7. 현재 판정

```text
AimPlan Phase 1:
  완료

Aim Multiplayer Verification:
  Deprecated

Weapon / Damage Integration:
  미착수
```

---

## ChangeLog

- v1.1.0 / 2026-06-19
  - 현재 상태를 싱글플레이 Aim Core 완료 기준으로 전환했다.
  - 다음 권장 작업을 Listen Server 검증에서 Standalone / Local HitScan / Fire FX 검증으로 변경했다.
  - 네트워크 검증은 Deprecated 상태로 표시했다.
- v1.2.0 / 2026-06-19
  - 실제 C++ 코드에 남은 `BuildFireRequest`, `ServerRequestFire`, `ServerAimState`, `RepAimVisualState`가 레거시 이름임을 명시했다.
  - `CF_AimNameAudit.md` 완료 상태를 추가했다.
  - 다음 권장 작업을 Debug 표시명 / Tooltip 정리로 조정했다.
- v1.3.0 / 2026-06-19
  - Pawn 단위 서버/RPC Fire 경로 제거 완료 상태를 반영했다.
  - `FireValidationState`, `AimVisualState`, `FirePending`, `FireRejected`, 로컬 Trace Debug 명칭 정리 완료 상태를 반영했다.
  - Aim 타입의 `FVector_NetQuantize` 계열 제거는 빌드 완료 / 에디터 재확인 필요 상태로 분리했다.
- v1.4.0 / 2026-06-19
  - 이번 세션 목표를 기능 추가가 아니라 싱글플레이 전환 완료로 고정했다.
  - Fire FX, Damage, WeaponComp 등 기능 추가 후보를 이번 세션 제외 항목으로 분리했다.
- v1.5.0 / 2026-06-19
  - 사용자 에디터 정상작동 확인을 반영해 Aim Net Serialization 제거 후 확인 상태를 완료로 변경했다.
  - 이번 세션의 남은 작업을 기능 추가가 아닌 완료 판정 기록으로 축소했다.

## 마이그레이션 지침

- 신규 상태 기록은 `CF_AimVerify.md`의 싱글플레이 검증 결과를 기준으로 갱신한다.
- Pawn 단위 RPC 제거는 완료됐으므로 새 작업에서 `ServerRequestFire` / `ClientReceiveFireResult`를 다시 도입하지 않는다.
- 남은 `BuildFireRequest` / `FCFVehicleFireRequest` 이름은 현재 로컬 Fire Command 의미로 해석하고, 필요할 때 별도 리네이밍 작업으로 분리한다.
- `FVector_NetQuantize` 제거 후 저장된 BP 값은 에디터에서 열어 저장하며 이상 여부를 확인한다.

## Change Note

- 2026-06-19: 이전 v1.2.0의 “서버/RPC 이름이 실제 코드에 남아 있다” 설명을 현재 코드 기준으로 축소했다.
- 문서상 과거 멀티플레이 감사 기록은 `CF_AimNameAudit.md`에 남기고, 이 문서는 현재 상태 요약만 유지한다.
- 2026-06-19: 사용자 에디터 확인으로 `빌드 완료 / 에디터 재확인 필요` 상태를 `완료`로 축소했다. Standalone Game 검증은 이번 세션 완료 조건이 아니라 후속 검증 후보로 남긴다.
