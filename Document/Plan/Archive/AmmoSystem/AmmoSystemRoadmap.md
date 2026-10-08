# Vehicle Ammo System Roadmap

- Version: 0.1.0
- Date: 2026-07-29
- Status: Ready / Documentation Complete / Implementation Not Started
- Feature: `CF-FQ-031` 차량 탄약·재장전 런타임
- Representative Historical Plan: `Document/Plan/Archive/AmmoSystemPlan.md`
- Design: `Document/Plan/AmmoSystemDesign.md`

---

## 1. 목표

탄약 시스템을 데이터, 차량 Runtime, 발사 트랜잭션, 런처 시퀀스, 재장전, UI와 중량으로 분리하여 기존 전투 루프를 깨지 않는 작은 빌드 가능 단계로 구현한다.

```text
Ammo Data
→ Vehicle Ammo Runtime
→ Single Fire Transaction
→ Launcher Reservation / Lock
→ Reload
→ HUD / Debug
→ Fitting / Mass
→ Integration Verification
```

---

## 2. 현재 단계

```text
M0 요구사항·구조 조사와 문서 준비: Done
M1 Ammo Data Contract: Ready
M2 Vehicle Runtime: Not Started
M3 Fire·Launcher Integration: Not Started
M4 Reload·HUD: Not Started
M5 Fitting·Mass: Not Started
M6 Integrated Verification: Not Started
```

현재 단일 Active 기능은 계속 `CF-FQ-029`다.
이 로드맵을 생성한 사실만으로 `CF-FQ-031` 구현을 자동 시작하지 않는다.

---

## 3. 단계 요약

| 단계 | 포함 Task | 목표 | 종료 기준 |
|---|---|---|---|
| M0 | AMMO-P0-00 | 결정과 책임 경계 잠금 | 구현 문서 준비 |
| M1 | AMMO-P0-01 | AmmoData와 정적 Weapon 설정 | Build·Contract PASS |
| M2 | AMMO-P0-02 | 차량 탄약 Runtime과 Snapshot | 수량 보존·독립 상태 PASS |
| M3 | AMMO-P0-03~04 | 단발 소비와 런처 예약·잠금 | Fire·Launcher 자동화 PASS |
| M4 | AMMO-P0-05~06 | 재장전과 인게임 표시 | Reload·HUD PIE PASS |
| M5 | AMMO-P0-07 | 피팅 한도와 탄약 중량 | 차량 중량 회귀 PASS |
| M6 | AMMO-P0-08 | 전체 통합 검증 | 사용자 PIE·Systems 승격 |

---

## 4. M0 — 요구사항과 구조 잠금

### 상태

```text
Done / 2026-07-29
```

### 확정 결정

```text
- 인게임은 현재 출격 차량의 현재 사용 가능 탄약을 표시한다.
- 피팅 최대 적재량은 피팅 화면에서만 표시한다.
- 장전량은 WeaponInstanceId별, 예비량은 탄종별 차량 공유다.
- UCFVehicleAmmoComp가 런타임 단일 소유자다.
- 발사는 Reserve / Execute / Commit / Rollback을 사용한다.
- Launcher Sequence Active 중 재장전과 교환을 금지한다.
- 거부된 교환 요청이 시퀀스를 취소하지 않는다.
```

### 산출물

```text
AmmoSystemDesign.md
AmmoSystemPlan.md
AmmoSystemRoadmap.md
AmmoTaskSource.md
AmmoWorkOrder.md
```

---

## 5. M1 — Ammo Data Contract

### 포함

```text
AMMO-P0-01
```

### 목표

기존 발사 동작을 변경하지 않는 정적 탄약 데이터 계약을 추가한다.

### 주요 산출물

```text
CFAmmoTypes.h
CFAmmoData.h / .cpp
CFAmmoContractTests.cpp
CFWeaponData 탄약 설정 확장
```

### 잠금 기준

```text
- MagazineSize 필드 유지
- ReloadTimeSeconds 필드 유지
- AmmoTypeId 레거시 유지
- DefaultAmmoData 미지정 기존 에셋은 발사 결과 변화 없음
- 실제 수량 소비는 아직 연결하지 않음
```

### 종료 기준

```text
- UHT PASS
- 공식 Editor 빌드 PASS
- Contract 테스트 PASS 또는 Runner 상태 명시
- 기존 WeaponData 에셋 호환
```

---

## 6. M2 — Vehicle Ammo Runtime

### 포함

```text
AMMO-P0-02
```

### 목표

차량이 출격 시 적재한 탄약을 실제 Runtime 상태로 생성하고 안전하게 조회·수정한다.

### 주요 산출물

```text
CFVehicleAmmoComp.h / .cpp
CFAmmoRuntimeTests.cpp
ACFVehiclePawn 기본 서브오브젝트 연결
```

### 종료 기준

```text
- WeaponInstanceId별 장전 상태 독립
- AmmoData별 Reserve 공유
- 예약 수량과 자유 사용 수량 분리
- 현재 사용 가능량과 현재 차량 보유량 계산 정확
- 초기화·Reset·EndPlay 안전
```

---

## 7. M3 — Fire와 Launcher 통합

### 포함

```text
AMMO-P0-03
AMMO-P0-04
```

### 목표

발사 성공과 탄약 소비를 원자적으로 연결하고 다연장 시퀀스의 전체 예약과 행동 잠금을 구현한다.

### 단발 종료 기준

```text
- 정상 HitScan·Projectile 소비
- 검증 거부 소비 없음
- 실행 실패 Rollback
- fallback 성공 소비
```

### 런처 종료 기준

```text
- 시퀀스 시작 전 유효 발수 예약
- 부분 Salvo 기본 허용
- 성공 발사만 Commit
- 실패·취소 예약 해제
- Active 중 재장전·무기·탄종·장비 교환 거부
- 거부 요청 후 시퀀스 계속
```

### 선행 게이트

`CF-FQ-029`의 실제 Launcher Sequence 상태와 사용자 PIE 결과를 재확인한다.
런처 결함을 탄약 Task에서 우회 구현하지 않는다.

---

## 8. M4 — Reload와 HUD

### 포함

```text
AMMO-P0-05
AMMO-P0-06
```

### 목표

FullMagazine 재장전과 현재 전투에 필요한 탄약 정보를 명확하게 표시한다.

### 재장전 종료 기준

```text
- 완료 순간에만 수량 이동
- 부분 재장전 지원
- 재장전 중 발사 거부
- 시퀀스 중 재장전 시작 불가
- 시퀀스 완료 후 자동 재장전
```

### HUD 종료 기준

```text
- 일반 상태: ImmediateUsable | CurrentUsable
- 시퀀스 상태: Pattern 진행·남은 발사·자유 사용 가능량
- NoAmmo·Reloading·SequenceActive 구분
- 최대 적재 가능량이 인게임 기본 HUD에 표시되지 않음
- 16:9·32:9 사용자 PIE
```

---

## 9. M5 — Fitting과 Mass

### 포함

```text
AMMO-P0-07
```

### 목표

출격 전 탄종과 탄수가 차량 중량과 피팅 한도에 반영되게 한다.

### 종료 기준

```text
- MaximumLoadableAmmoCount
- InitialSortieAmmoCount
- UnitMassKg
- TotalAmmoMassKg
- 현재 적재 / 최대 적재 UI
- 장전·예비 중복 중량 없음
- 기존 Chaos 차량 질량 회귀 없음
```

런타임 발사마다 질량을 즉시 갱신할지 배치 갱신할지는 실제 차량 Runtime 비용을 측정한 뒤 잠근다.

---

## 10. M6 — 통합 검증과 승격

### 포함

```text
AMMO-P0-08
```

### 필수 시나리오

```text
- Heavy Cannon 장전·발사·재장전
- RocketLauncher Ripple 전체·부분 발사
- 시퀀스 중 재장전·교환 거부
- 시퀀스 완료 후 자동 재장전
- NoAmmo
- Projectile Pool 반복
- 차량 파괴 중 강제 취소
- 현재 사용 가능량·현재 보유량·피팅 최대량 구분
- 탄약 중량에 따른 차량 설정 회귀
```

### 승격 조건

```text
- 공식 Editor 빌드 PASS
- 전체 Ammo Automation PASS
- CF-FQ-029 Launcher 회귀 PASS
- 사용자 PIE PASS
- Ammo.md Current System 작성
- FeatureQueue Done 전환
```

---

## 11. P1 이후 확장

```text
- 탄종 전환과 WeaponAmmoOption
- PerRound Reload
- 탄약 보급
- 탄약 상자와 전장 획득
- 탄약고 손상·유폭
- AI 탄약 보존 판단
- 전투 결과 탄약 효율
- 서버 권한과 복제
```

P1 기능을 위해 P0 Runtime에 개별 탄창 아이템이나 탄약고 위치 구조를 미리 넣지 않는다.

---

## 12. 중단 조건

```text
- CF-FQ-029 Launcher Sequence 계약이 사용자 PIE에서 변경됨
- WeaponInstanceId를 현재 장착 구조에서 안정적으로 만들 수 없음
- 기존 dirty와 핵심 Fire 함수가 충돌하여 최소 패치 불가
- 차량 질량 소유권이 현재 문서와 코드에서 불일치
- UI가 현재 선택 무기 식별자를 제공하지 못함
- 사용자 결정 없이 Reloading 중 무기 교환 정책을 확정해야 함
```

중단 시 범위를 임의 확장하지 않고 대표 Plan에 차단 사유를 기록한다.

---

## 13. Changelog

### v0.1.0 - 2026-07-29

```text
- AMMO-P0-00~08을 M0~M6 단계로 정리했다.
- 데이터 → Runtime → Fire → Launcher → Reload·HUD → Mass → 검증 순서를 확정했다.
- CF-FQ-029 Launcher Sequence 사용자 PIE와 실제 계약을 통합 전 선행 게이트로 지정했다.
```

---

## 14. Migration

```text
- M1은 데이터 계약만 추가하고 현재 발사 흐름을 변경하지 않는다.
- M2까지는 탄약 Runtime을 관찰 가능하게 만들되 발사 소비를 연결하지 않는다.
- M3부터 기존 WeaponFire와 Launcher에 탄약 검증을 단계적으로 연결한다.
- 각 단계는 이전 단계의 공식 빌드와 회귀 검증을 통과한 뒤 진행한다.
