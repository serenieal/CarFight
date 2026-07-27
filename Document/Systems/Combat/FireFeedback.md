# FireFeedback

- Version: 1.4.0
- Date: 2026-07-24
- Status: Current / P0 FireFeedback User PIE Verified
- Scope: 싱글플레이 로컬 발사 결과와 AimFireAlignment 거부 사유를 FireFeedback ViewData로 변환하고 WBP Reticle의 텍스트와 색상으로 표시하는 현재 기준 문서

---

## 1. 문서 목적

이 문서는 CarFight의 현재 전투 구현에서 `FireFeedback`이 어떤 책임을 가져야 하는지 기록한다.

현재 CarFight의 전투 구현 기준은 **싱글플레이 로컬 차량 전투**다.
따라서 이 문서에서 `FireFeedback`은 서버 응답 상태를 표시하는 기능이 아니라, 로컬 `WeaponFire` 결과를 플레이어가 즉시 이해할 수 있는 화면·UI·시각 피드백으로 변환하는 기능으로 본다.

이 문서는 최종 연출 품질 문서가 아니다.
현재 구현된 Reticle/UI 피드백 기준과 후속 VFX 확장 경계를 기록하는 현재 상태 문서다.

프로젝트 결정 `CF-PDL-0009`에 따라 게임 사운드는 지원하지 않는다. 이 문서 후반의 과거 `SFX`, `소리`, `사운드` 후보 표현은 역사 기록이며 신규 구현 범위나 완료 조건으로 사용하지 않는다.

---

## 2. 현재 기준

현재 기준은 아래와 같다.

```text
- 서버 권한 발사, 복제, 2클라 검증, 서버 대기 UI는 현재 구현 범위로 보지 않는다.
- 과거 멀티플레이 기준 용어가 남아 있는 경우에는 Legacy / Deferred 흔적으로 본다.
- 현재 문서 기준에서는 로컬 발사 검증과 로컬 피드백 용어를 우선 사용한다.
```

용어 기준:

```text
WaitingServer   -> FirePending
ServerRejected  -> FireRejected
서버 대기        -> 발사 처리 중
서버 거부        -> 발사 거부 또는 발사 조건 미충족
ServerAimState  -> FireValidationState
RepAimVisualState -> AimVisualState
NoAuthority     -> InvalidLocalState
```

`FirePending`은 서버 응답 대기 상태가 아니라, 로컬 Fire 입력 이후 짧게 표시할 수 있는 발사 처리 피드백 상태다.
`FireRejected`는 서버 거부 상태가 아니라, 로컬 발사 검증에서 조건을 만족하지 못했음을 표시하는 상태다.

---

## 3. 문서 범위

이 문서에서 말하는 `FireFeedback`은 아래 요소를 묶어서 본다.

```text
- WeaponFire 결과를 읽는 기준
- Reticle에 표시할 발사 상태 기준
- 발사 성공 / 실패 / 쿨다운 / 무기 없음 상태 표시 기준
- 최소 P0 VFX 호출 기준
- Debug와 플레이어 표시의 책임 분리
- 피해 판정과 피드백의 책임 분리
```

현재 직접 참조하는 기준 문서:

```text
- Document/Systems/Combat/WeaponFire.md
- Document/Systems/UI/AimReticle.md
- Document/Systems/Vehicles/VehicleAim.md
- Document/Systems/UI/DisplayTextPolicy.md
```

---

## 4. 현재 실제 역할

현재 기준 `FireFeedback`의 핵심 역할은 **`WeaponFire`가 기록한 로컬 발사 결과를 플레이어가 읽을 수 있는 피드백으로 변환하는 것**이다.

현재 구현 기준에서 `FireFeedback`은 아래 일을 맡는다.

```text
1. 발사 성공 여부를 플레이어가 즉시 알 수 있게 한다.
2. 발사 실패 이유를 최소한의 UI 상태로 구분한다.
3. 쿨다운 중인지 플레이어가 알 수 있게 한다.
4. 무기가 없거나 무기 데이터가 잘못된 상태를 구분한다.
5. 조준이 막힌 상태와 발사 조건 미충족 상태를 구분한다.
6. 판정 흐름과 연출 흐름을 분리한다.
```

즉 `FireFeedback`은 발사 판정자가 아니다.
발사 판정은 `WeaponFire`가 수행하고, `FireFeedback`은 그 결과를 읽어 화면·UI·시각 표시로 바꾼다.

### 4.1 현재 구현 구조

현재 P0에서는 별도 FireFeedback 컴포넌트를 만들지 않고 아래 구조로 구현했다.

```text
ACFVehiclePawn
  -> LastFireResult / RejectReason 보관
  -> LastFireFeedbackStartTimeSeconds 보관
  -> VehicleWeaponComp에서 전체/남은 쿨다운 읽기
  -> BuildFireFeedbackViewData()로 FCFVehicleFireFeedbackViewData 생성

UCFAimReticleWidget
  -> FireFeedbackViewData 캐시
  -> 기본 ReticleState와 피드백 상태 결합
  -> FireFeedback State/Hint/Cooldown 텍스트 갱신
  -> Reticle 이미지 5개와 텍스트의 상태별 색상 갱신

WBP_AimReticle
  -> 이미지/텍스트 배치
  -> C++ Optional 바인딩 수신
```

현재 관련 구현 버전:

```text
CFVehiclePawn.cpp        v2.107.0
CFAimReticleWidget.cpp   v1.6.0
```

---

## 5. 책임 분리

### 5.1 WeaponFire 책임

`WeaponFire`의 책임은 아래와 같다.

```text
- Fire 입력을 로컬 발사 명령으로 변환한다.
- 발사 가능 여부를 검증한다.
- 발사 원점과 방향을 결정한다.
- Projectile Actor 또는 Dummy HitScan 실행 경로를 결정한다.
- LastFireResult와 RejectReason을 기록한다.
- FireValidationState와 AimVisualState를 갱신한다.
- 쿨다운 기준 시간을 기록한다.
```

### 5.2 AimReticle 책임

`AimReticle`의 책임은 아래와 같다.

```text
- VehicleAimComp의 Local Aim 상태를 표시한다.
- 현재 Reticle 상태를 한국어 텍스트 또는 WBP 시각 상태로 변환한다.
- FireFeedback ViewData를 최종 Reticle 상태, 한국어 텍스트, 쿨다운 시간, 상태별 색상으로 표시한다.
```

### 5.3 FireFeedback 책임

`FireFeedback`의 책임은 아래와 같다.

```text
- WeaponFire 결과를 플레이어 피드백 상태로 변환한다.
- Reticle, 간단한 HUD 문구와 VFX 호출 기준을 정한다.
- 발사 성공 / 발사 불가 / 쿨다운 / 무기 없음 상태를 읽히게 만든다.
- 판정 결과를 임의로 바꾸지 않는다.
```

### 5.4 피해 판정 책임

피해 판정과 피해 적용은 `FireFeedback`의 책임이 아니다.

```text
- 빗나감 / 피격 판정
- 피해량 계산
- HP 차감
- 차량 파괴
- 장갑 / 모듈 피해
```

위 항목은 `HitDamage`, `DamageHitContext`, 후속 Combat 문서의 책임으로 본다.

---

## 6. 현재 입력 데이터

`ACFVehiclePawn::BuildFireFeedbackViewData()`가 실제로 읽는 핵심 데이터는 아래와 같다.

```text
- LastFireResult.bAccepted
- LastFireResult.RejectReason
- LastFireFeedbackStartTimeSeconds
- FireSuccessFeedbackDurationSeconds
- FireRejectedFeedbackDurationSeconds
- VehicleWeaponComp.GetActiveWeaponCooldownSeconds()
- VehicleWeaponComp.GetRemainingCooldownSeconds(CurrentTimeSeconds)
- VehicleAimComp.GetLocalAimState().bLocalWithinWeaponArc
```

이 값으로 아래 표시 데이터를 만든다.

```text
- FeedbackState
- LastRejectReason
- bFeedbackActive
- bOverrideReticleState
- bShowCooldown
- RemainingCooldownSeconds
- TotalCooldownSeconds
- CooldownRatio
- bShowOutOfArcWarning
- FeedbackDisplayKey
```

`FireValidationState`와 `AimVisualState`는 WeaponFire/Debug 추적에는 사용되지만, 현재 `BuildFireFeedbackViewData()`의 직접 표시 입력은 아니다.

---

## 7. FireFeedback 표시 상태

현재 P0 기준에서 필요한 최소 표시 상태는 아래와 같다.

| 표시 상태 | 의미 | 대표 입력 |
| --- | --- | --- |
| `FireSuccess` | 발사 성공 | `LastFireResult.bAccepted == true` |
| `FireRejected` | 발사 조건 미충족 | 거부 사유가 일반 실패 계열 |
| `Cooldown` | 무기 쿨다운 중 | `RejectReason == WeaponCooldown` 또는 남은 쿨다운 > 0 |
| `NoWeapon` | 무기 없음 / 비호환 | `RejectReason == NoWeapon` |
| `AimBlocked` | 조준선 막힘 | `RejectReason == AimBlocked` |
| `OutOfArcWarning` | 조준각 거부 피드백 | `RejectReason == OutOfWeaponArc` |
| `OutOfArcWarning` + `FeedbackDisplayKey=TurretAligning` | 터렛 정렬 중 보조 표시 | `RejectReason == TurretAligning` |
| `FirePending` | 발사 처리 중 후보 | 현재 활성화 경로 없음 |

`OutOfArcWarning`은 현재 P0 싱글플레이 기준에서 주 Reticle을 덮어쓰는 단독 발사 차단 상태가 아니다.
`FeedbackState == OutOfArcWarning`은 `OutOfWeaponArc` 거부 결과에서 생성되며, 별도의 `bShowOutOfArcWarning` 보조 표시 플래그는 Local Aim이 무기 각도 밖인 동안에도 true가 될 수 있다.

---

## 8. RejectReason -> FireFeedback 매핑

현재 `BuildFireFeedbackViewData()`가 `WeaponFire` 결과를 `FireFeedback` 표시로 바꾸는 매핑은 아래와 같다.

| RejectReason / 결과 | FireFeedback 표시 | Reticle 처리 | 현재 의미 |
| --- | --- | --- | --- |
| `None` + `bAccepted == true` | `FireSuccess` | `Ready` + 성공 플래시 | 정상 발사 |
| `InvalidLocalState` | `FireRejected` | `FireRejected` | 로컬 상태가 발사 처리에 부적합 |
| `InvalidOwner` | `FireRejected` | `FireRejected` | Pawn / Controller 상태 문제 |
| `VehicleDisabled` | `FireRejected` | `FireRejected` | 차량 또는 Aim 런타임 준비 안 됨 |
| `NoWeapon` | `NoWeapon` | `NoWeapon` | 무기 없음 또는 무기 데이터 비호환 |
| `WeaponCooldown` | `Cooldown` | `Cooldown` | 연사 제한 / 쿨다운 |
| `NoAmmo` | `FireRejected` | `FireRejected` | 현재 탄약/재장전 시스템 미구현이므로 일반 거부로 처리 |
| `OutOfWeaponArc` | `OutOfArcWarning` | BaseReticleState 유지 + 전용 보조 경고 | 주 상태/색상 덮어쓰기 없음 |
| `AimBlocked` | `AimBlocked` | `Blocked` | 조준선 막힘 |
| `TurretAligning` | `OutOfArcWarning` + `FeedbackDisplayKey=TurretAligning` | BaseReticleState 유지 + `정렬 중` 보조 경고 | 주 상태/색상 덮어쓰기 없음 |
| `WeaponNotAligned` | 없음 또는 Debug 전용 | BaseReticleState 유지 | 빨간 FireRejected로 주 Reticle 덮어쓰기 없음 |
| `MuzzleBlocked` | `AimBlocked` | `Blocked` | 총구 앞 WeaponHit 경로 막힘 |
| `InvalidAimOrigin` | `FireRejected` | `FireRejected` | 발사 원점 비정상 |
| `InvalidAimDirection` | `FireRejected` | `FireRejected` | 발사 방향 비정상 |
| `TraceMiss` | `FireRejected` | `FireRejected` | 현재 별도 MissFeedback 상태가 없어 기본 거부로 매핑 |

현재 P0 기준에서는 UI 상태를 지나치게 세분화하지 않는다.
`bAllowFireWhileAligning=true`인 정렬 중 발사는 승인될 수 있으므로 거부 피드백을 만들지 않지만, 기본 Reticle은 계속 `TurretAligning` amber 상태를 유지한다. 정책 false일 때만 기존 `TurretAligning` / `WeaponNotAligned` 거부 매핑을 사용한다.
먼저 플레이어가 아래 네 가지를 구분할 수 있으면 충분하다.

```text
- 발사됨
- 쿨다운 중
- 무기 없음
- 발사 조건 미충족
```

---

## 9. Cooldown 표시 기준

현재 쿨다운 표시는 `VehicleWeaponComp`의 아래 Getter를 사용한다.

```text
GetActiveWeaponCooldownSeconds()
GetRemainingCooldownSeconds(CurrentTimeSeconds)
```

계산과 표시:

```text
bShowCooldown = TotalCooldownSeconds > 0 && RemainingCooldownSeconds > 0
CooldownRatio = Clamp(RemainingCooldownSeconds / TotalCooldownSeconds, 0, 1)
Text_Cooldown = "%.2f초"
```

처리 규칙:

```text
- 전체 시간 또는 남은 시간이 0 이하면 쿨다운 텍스트를 표시하지 않는다.
- FireSuccess 유지 시간 동안에도 Text_Cooldown 숫자는 표시할 수 있다.
- 성공 표시가 끝난 뒤 남은 쿨다운이 있으면 Cooldown이 주 피드백 상태가 된다.
- 쿨다운이 끝나면 이전 WeaponCooldown 결과가 남아 있어도 FeedbackActive와 DisplayKey를 제거한다.
```

---

## 10. Reticle과 FireFeedback 현재 표시 우선순위

현재 `BuildFireFeedbackViewData()`의 우선순위는 아래와 같다.

```text
1. 마지막 발사가 성공했고 성공 표시 유지 시간 안이면 FireSuccess
2. 성공 표시가 끝났고 남은 쿨다운이 있으면 Cooldown
3. 마지막 발사가 성공했고 쿨다운도 끝났으면 None
4. 마지막 발사가 실패했으면 RejectReason별 피드백을 실패 유지 시간 동안 표시
```

실제 화면 흐름:

```text
정상 발사
  -> FireSuccess: 밝은 녹색 Reticle / 발사 문구
  -> Cooldown: 파란색 Reticle / 재사용 대기 / 남은 시간
  -> None: 일반 Aim 상태로 복귀, FireFeedback 텍스트 제거
```

실패 매핑:

```text
NoWeapon        -> 회색, 주 Reticle 상태 덮어쓰기
AimBlocked      -> 주황색, Blocked 상태로 덮어쓰기
FireRejected    -> 빨간색, FireRejected 상태로 덮어쓰기
OutOfArcWarning -> 노란색 전용 보조 경고, 주 상태/색상 덮어쓰기 없음
TurretAligning  -> amber 전용 보조 경고, 주 상태/색상 덮어쓰기 없음
MuzzleBlocked   -> 주황색 Blocked 표시
WeaponNotAligned -> 주 Reticle 빨간 덮어쓰기 없음
```

전용 `Text_OutOfArcWarning`이 있을 때 일반 State/Hint를 숨기는 조건은 현재 활성 피드백 자체가 `OutOfArcWarning`인 경우로 제한한다. 조준각 밖이어도 `FireSuccess`, `Cooldown`, `FireRejected`가 현재 피드백이면 해당 일반 문구가 유지된다.

---

## 11. P0 구현 및 검증 상태

구현 완료:

```text
- ECFVehicleFireFeedbackState / FCFVehicleFireFeedbackViewData 정의
- ACFVehiclePawn::BuildFireFeedbackViewData() 구현
- FireSuccess / Cooldown / FireRejected / NoWeapon / AimBlocked / OutOfArcWarning 매핑
- WBP_AimReticle의 State / Hint / Cooldown / 전용 OutOfArc 경고 연결
- Reticle 이미지 5개와 피드백 텍스트의 상태별 색상 적용
- FireSuccess → Cooldown → 종료 흐름
- WeaponCooldown과 OutOfArcWarning 텍스트 잔류 수정
- 전용 OutOfArc 경고 중복 방지와 다른 피드백 비가림 조건 수정
- TurretAligning 정렬 중 문구와 amber 보조 색상 표시
- WeaponNotAligned 빨간 FireRejected 주 상태 덮어쓰기 방지
- MuzzleBlocked AimBlocked / Blocked 표시 연결
- VehicleDebug Panel Weapon Aim Solution 표시
```

PIE 확인 완료:

```text
- Ready 흰색
- FireSuccess 녹색
- Cooldown 파란색
- 연속 입력 후 쿨다운 종료 시 텍스트 제거
- 조준각 밖에서 FireSuccess / Cooldown / FireRejected 일반 문구 유지
- 실제 OutOfArcWarning에서 전용 경고가 일반 문구를 대체
- TurretAligning amber, WeaponNotAligned 비가림, MuzzleBlocked 주황 Blocked 표시
- NoWeapon 회색 Reticle과 "무기 없음" / "사용 가능한 무기 없음" 문구
- AimBlocked 주황 Reticle과 "조준 가림" / "조준선이 막힘" 문구
- NoWeapon / AimBlocked 유지 시간 종료 후 FireFeedback 텍스트 제거
- 장애물 제거 후 정상 Aim 복귀
- Ready → FireSuccess → Cooldown → Ready 정상 발사 회귀
```

현재 판정:

```text
- Unreal Editor 타깃 빌드: PASS
- CF-FQ-017: Done
- CF-TC-014: PASS
```

P0에서 계속 제외하는 항목:

```text
- 무기별 고유 애니메이션 대량 제작
- 복잡한 HUD 전체 구조
- 고품질 VFX 완성
- 피해 숫자, 킬로그, 전투 로그 UI
- 멀티플레이 동기화 피드백
```

---

## 12. VFX 최소 기준

현재 `FireFeedback`은 Reticle과 HUD 표시를 소유하고, 후속 전투 시각 FX가 따라야 할 호출 조건만 제공한다.
게임 사운드는 프로젝트 결정 `CF-PDL-0009`에 따라 지원하지 않는다.

### 12.1 발사 성공

```text
- 실제 발사 실행이 성공한 경우에만 Muzzle 또는 FireOrigin 기준 발사 플래시를 1회 요청할 수 있다.
- Projectile Actor와 Dummy HitScan은 같은 승인 결과 기준을 사용한다.
```

### 12.2 발사 실패

```text
- 발사 실패에서는 실제 총구 플래시를 생성하지 않는다.
- 실패 이유는 Reticle 색상, 상태 문구와 Debug로 표시한다.
```

### 12.3 쿨다운

```text
- 쿨다운은 Reticle, UI 문구와 남은 시간 또는 비율로 표시한다.
- 청각 피드백과 지속 사운드는 사용하지 않는다.
```

---

## 13. 표시 텍스트 정책

화면에 보이는 텍스트는 `DisplayTextPolicy`를 따른다.
내부 enum과 데이터 식별자는 영문을 유지하고, 플레이어에게 보이는 문구는 한국어를 사용한다.

표시 문구 후보:

| 상태 | 한국어 표시 후보 |
| --- | --- |
| 발사 성공 | `발사` 또는 짧은 플래시만 사용 |
| 발사 조건 미충족 | `발사 불가` |
| 쿨다운 | `재사용 대기` |
| 무기 없음 | `무기 없음` |
| 조준 막힘 | `조준 가림` |
| 조준각 경고 | `각도 경고` |
| 발사 처리 중 | `발사 처리 중` |

P0에서는 텍스트를 너무 많이 보여주기보다, Debug Panel에서 상세 원인을 확인하고 실제 플레이 화면에서는 핵심 상태만 표시하는 쪽을 우선한다.

---

## 14. 현재 기능 책임

현재 `FireFeedback`의 책임은 아래로 제한한다.

```text
- WeaponFire 결과를 표시 상태로 변환한다.
- 발사 성공 / 실패 / 쿨다운 / 무기 없음 상태를 플레이어에게 읽히게 한다.
- Reticle / 간단한 HUD / VFX 호출 기준을 정리한다.
- 판정 결과를 변경하지 않는다.
- 피해 판정을 수행하지 않는다.
```

---

## 15. 현재 기준 비책임 항목

현재 `FireFeedback`은 아래를 직접 수행하지 않는다.

```text
- Fire 입력 처리
- 발사 가능 여부 검증
- FireOrigin 계산
- Projectile Actor 생성
- Dummy HitScan 수행
- 실제 HP 차감
- 차량 파괴 처리
- 장갑 / 모듈 피해 계산
- 쿨다운 수치 계산 자체
- 무기 데이터 호환성 판정
- 멀티플레이 복제
- 서버 응답 대기 표시
```

---

## 16. 현재 문서 기준의 핵심 결론

현재 `FireFeedback`은 **WeaponFire가 만든 로컬 발사 결과를 플레이어가 읽을 수 있는 피드백으로 변환하기 위한 기준 기능**이다.

가장 중요한 현재 역할은 다음 한 줄로 요약할 수 있다.

> `FireFeedback`은 현재 “발사가 되었는가, 왜 안 되었는가, 아직 쿨다운인가”를 플레이어가 즉시 이해하게 만드는 표시 계층이다.

---

## 17. 현재 미확인 항목

아래 항목은 아직 확정하지 않았다.

```text
- 발사 성공 VFX 자산명과 연결 시점
- Cooldown 숫자 표시를 게이지로 확장할지 여부
- FirePending을 비동기/충전 무기에서 활성화할지 여부
- Reloading을 실제 탄약 시스템과 연결할지 여부
```

---

## 18. 문서 갱신 조건

아래 변경이 생기면 이 문서를 갱신한다.

```text
- FireFeedback 전담 C++ 클래스 또는 컴포넌트가 추가될 때
- AimReticle이 LastFireResult / RejectReason / Cooldown 값을 직접 읽도록 확장될 때
- RejectReason -> UI 표시 매핑이 바뀔 때
- Cooldown 표시 방식이 확정될 때
- VFX 자산 연결 경로가 확정될 때
- OutOfArc / OutOfWeaponArc가 실제 발사 거부 조건으로 승격될 때
- 피해 판정 / 피해 처리와 피드백 호출 순서가 바뀔 때
```

---

## 19. 문서 버전 관리

- 현재 문서 버전: `1.4.0`
- 문서 상태: `Current / P0 FireFeedback User PIE Verified`
- 관리 원칙:
  - 이 문서는 한 번 작성하고 끝내는 문서가 아니라, 기능의 현재 상태가 바뀌면 함께 갱신한다.
  - 기능 설명 본문이 바뀌면 체인지로그도 같이 갱신한다.
  - 구현 변경 없이 표현만 다듬은 경우와, 기능 이해에 영향을 주는 내용 변경을 구분해서 기록한다.

### 버전 증가 기준

- `Major`
  - FireFeedback이 전투 HUD 전체 또는 대규모 피드백 시스템으로 확장될 때
  - 서버/멀티플레이 발사 피드백까지 현재 범위로 다시 들어올 때
- `Minor`
  - FireFeedback 전담 클래스, Reticle 연동, VFX 연결, Cooldown 표시 방식이 추가될 때
  - RejectReason 매핑 또는 표시 우선순위가 바뀔 때
- `Patch`
  - 오탈자 수정
  - 표현 명확화
  - 근거 보강
  - 본문 의미는 유지한 채 설명 정밀도만 올라갈 때

---

## 20. Migration

### v1.3.0 -> v1.4.0

```text
- 코드와 WBP 구현은 변경하지 않는다.
- FireFeedback의 현재 표시 계약은 그대로 유지한다.
- 후속 연출은 Niagara 기반 VFX만 사용하며 게임 사운드를 연결하지 않는다.
- 과거 SFX 언급은 v1.4.0 이전 역사 기록으로만 해석한다.
```

### v1.2.1 -> v1.3.0

```text
- NoWeapon와 AimBlocked의 실제 UI 색상, 문구와 피드백 만료를 사용자 PIE 기준으로 확정한다.
- 장애물 제거 후 정상 Aim 복귀와 정상 발사 회귀를 Current 동작으로 사용한다.
- CF-FQ-017 Done / CF-TC-014 PASS로 전환한다.
- 코드와 WBP 자산 마이그레이션은 필요하지 않다.
```

### v1.2.0 -> v1.2.1

```text
- 정렬 중 발사 허용 정책은 발사 승인 여부만 바꾸며 기존 TurretAligning amber 주 상태를 제거하지 않는다.
- 정책 false의 TurretAligning / WeaponNotAligned 거부 표시와 MuzzleBlocked Blocked 표시는 기존 의미를 유지한다.
- WBP 구조와 바인딩은 변경하지 않았고 `Tools/BuildEditor.bat`만 검증했다.
- PIE 표시는 Pending이다.
```

### v1.1.0 -> v1.2.0

```text
- TurretAligning은 FeedbackDisplayKey로 구분하고 `정렬 중` 문구와 TurretAligningReticleColor를 사용한다.
- WeaponNotAligned는 빨간 FireRejected로 주 Reticle을 덮지 않는다.
- MuzzleBlocked는 AimBlocked / Blocked 주황 표시 경로를 사용한다.
- 기존 WBP Optional 바인딩 이름은 변경하지 않는다.
- C++ 빌드는 완료됐지만 PIE 표시 검증은 별도로 수행해야 한다.
```

### v1.0.0 -> v1.1.0

```text
- 별도 FireFeedback 컴포넌트 없이 ACFVehiclePawn::BuildFireFeedbackViewData()와 UCFAimReticleWidget 통합 구조를 현재 기준으로 사용한다.
- FireSuccess는 성공 유지 시간 동안 Cooldown보다 우선 표시하고, 이후 남은 쿨다운 상태로 전환한다.
- WeaponCooldown과 OutOfArcWarning은 실제 표시 조건이 끝나면 활성 상태와 표시 키를 제거한다.
- Text_OutOfArcWarning은 실제 OutOfArcWarning 피드백에서만 일반 State/Hint를 대체한다.
- 기존 WBP Optional 바인딩 구조를 유지하면 데이터 마이그레이션은 필요하지 않다.
```

### 신규 문서 기준

```text
- 서버 대기 / 서버 거부 표현은 현재 FireFeedback 기준에서 사용하지 않는다.
- WaitingServer는 FirePending으로 해석한다.
- ServerRejected는 FireRejected로 해석한다.
- FirePending은 로컬 발사 처리 피드백 상태로 해석한다.
- FireRejected는 로컬 발사 조건 미충족 상태로 해석한다.
- OutOfArcWarning은 현재 P0 싱글플레이 기준에서 조준각 경고/디버그 상태로 해석한다.
- 실제 발사 성공 여부는 WeaponFire의 LastFireResult / ValidateFireCommand 결과를 기준으로 판단한다.
```

---

## 21. Changelog

### v1.4.0 - 2026-07-24

```text
- 프로젝트 전역 사운드 비지원 결정에 맞춰 FireFeedback의 현재 책임에서 SFX와 사운드 후보를 제거했다.
- 발사 성공·실패·쿨다운의 VFX 호출 기준을 시각 연출과 UI 전용으로 정리했다.
- Sound 미확정 항목과 문서 갱신 조건을 제거했다.
```

### v1.3.0 - 2026-07-15

```text
- NoWeapon 회색 표시, AimBlocked 주황 표시와 두 상태의 문구 만료를 사용자 PIE로 확인했다.
- 장애물 제거 후 정상 Aim 복귀와 Ready → FireSuccess → Cooldown → Ready 회귀를 확인했다.
- CF-FQ-017 Done / CF-TC-014 PASS를 Current 상태에 반영했다.
```

### v1.2.1 - 2026-07-14

```text
- Align Fire Policy가 기존 FireFeedback 매핑을 유지하는 조건을 명시했다.
- 정책 true의 정렬 중 승인과 TurretAligning amber 표시가 공존하는 기준을 기록했다.
- C++ 빌드 완료와 PIE Pending을 기록했다.
```

### v1.2.0 - 2026-07-13

```text
- TurretAligning / WeaponNotAligned / MuzzleBlocked FireFeedback 표시 정책 반영
- TurretAligning amber 보조 경고와 WeaponNotAligned 비덮어쓰기 정책 기록
- AimFireAlignment Presentation 빌드 완료와 PIE Pending 상태 분리 기록
```

### v1.1.0 - 2026-07-13

```text
- ACFVehiclePawn::BuildFireFeedbackViewData()와 UCFAimReticleWidget의 실제 구현 구조 반영
- FireSuccess → Cooldown → 종료 표시 우선순위와 상태별 색상 규칙 반영
- WeaponCooldown / OutOfArcWarning 잔류 버그 수정 결과 반영
- 전용 OutOfArc 경고의 중복 방지, fallback, 다른 FireFeedback 비가림 조건 반영
- WBP_AimReticle 실제 자산과 Optional 바인딩 구조 반영
- NoWeapon / AimBlocked 실제 PIE 검증을 남은 완료 조건으로 기록
```

### v1.0.0 - 2026-07-09

```text
- FireFeedback Systems 문서 신규 작성
- 싱글플레이 로컬 발사 피드백 기준 정리
- WeaponFire / AimReticle / FireFeedback 책임 분리 기준 추가
- RejectReason -> FireFeedback / Reticle 표시 후보 매핑 추가
- Cooldown 표시 기준과 P0 최소 구현 범위 정리
- 서버 대기 / 서버 거부 표현을 현재 범위에서 제외
```

---

## 22. 마지막 확인 기준

- 확인 일시: `2026-07-15`
- 확인 근거:
  - `Document/Systems/Combat/WeaponFire.md`
  - `Document/Systems/UI/AimReticle.md`
  - `Document/Systems/Vehicles/VehicleAim.md`
  - `Document/Systems/UI/DisplayTextPolicy.md`
    - `UE/Source/CarFight_Re/Public/CFVehicleAimTypes.h`
  - `UE/Source/CarFight_Re/Public/CFVehiclePawn.h`
  - `UE/Source/CarFight_Re/Private/CFVehiclePawn.cpp`
  - `UE/Source/CarFight_Re/Public/CFVehicleFireFeedbackTypes.h`
  - `UE/Source/CarFight_Re/Public/UI/CFAimReticleWidget.h`
  - `UE/Source/CarFight_Re/Private/UI/CFAimReticleWidget.cpp`
  - `/Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle` 자산 상세 덤프
  - 2026-07-10 ~ 2026-07-13 사용자 빌드/PIE 확인 결과
