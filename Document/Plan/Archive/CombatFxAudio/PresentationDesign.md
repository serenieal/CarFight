# Combat FX Presentation Design

- Version: 0.2.0
- Date: 2026-07-24
- Status: Ready / Visual FX Only / Audio Scope Rejected
- Feature ID: `CF-FQ-024`
- Parent Plan: `Document/Plan/CombatFxAudio/ImplementationDesign.md`
- Path Note: `CombatFxAudio`는 레거시 디렉터리명이며 현재 문서는 시각 연출만 다룬다.

---

## 1. 목적

이 문서는 발사, Projectile 비행, 첫 Impact와 최초 차량 전투 불능 결과의 시각 표현 방향을 고정한다.
전투 판정은 기존 WeaponFire, Projectile, DamageHitContext와 HitDamage가 소유하며 이 문서는 판정을 다시 계산하지 않는다.

CarFight는 프로젝트 결정 `CF-PDL-0009`에 따라 게임 사운드를 지원하지 않는다.
SoundWave, SoundCue, MetaSound, Sound Attenuation, USoundBase와 AudioComponent는 이 문서의 범위가 아니다.

---

## 2. 전체 연출 방향

| 항목 | 현재 기준 |
|---|---|
| 전체 장르 | 밀리터리 SF |
| P0 기준 무기 | 대구경 실체 포탄 무기 |
| 기술 인상 | 화약식 대구경 포탄 + 약한 전자기 가속 보조 |
| 발사 표현 | Muzzle Flash, 연기, 미립자, 약한 청백색 방전 |
| Projectile 표현 | 짧은 Tracer, 희미한 연기, 약한 열 왜곡 |
| Impact 표현 | 순간 섬광, 금속 스파크, 작은 파편, 짧은 연기 |
| 차량 파괴 의미 | 차량 소멸이 아닌 전투 불능 |
| 전투 불능 표현 | 순간 파손 FX + 약한 지속 연기 |
| 카메라 반응 | 승인된 발사에만 짧고 약한 Camera Shake |
| 사운드 | 프로젝트 전역 비지원 |

---

## 3. 발사 연출

```text
발사 명령 승인
→ HitScan 또는 Projectile 실행 성공
→ 실제 Muzzle 위치에서 Muzzle FX 1회
→ 로컬 플레이어 소유 차량이면 Camera Shake 1회
→ Projectile이면 Trail 활성화
```

다음 결과에서는 Muzzle FX와 Camera Shake를 생성하지 않는다.

```text
Cooldown
NoWeapon
AimBlocked
MuzzleBlocked
정렬 정책에 따른 발사 거부
Projectile 생성 실패
기타 발사 실행 실패
```

Muzzle FX 기준:

```text
위치: 실제 최종 Muzzle 위치
방향: 실제 최종 AimDirection
주 표현: 주황빛 백색 섬광, 화약 연기, 짧은 미립자
보조 표현: 매우 약한 청백색 전자기 방전
금지 표현: 레이저처럼 보이는 긴 빛줄기
```

---

## 4. Projectile Trail

P0 Trail 후보:

```text
NS_CF_CannonTrail_P0
```

시각 구성:

```text
- 짧은 백색 또는 주황빛 Tracer
- 희미한 회색 연기
- 매우 약한 청백색 전자기 잔광
- 짧고 약한 열 왜곡
```

초기 튜닝 범위:

```text
Trail 길이: 500 ~ 1,500 uu
잔류 시간: 0.05 ~ 0.15초
```

Projectile Pool 재활성화와 반환 시 Trail 상태를 명시적으로 초기화한다.

---

## 5. Impact 연출

첫 Blocking Hit의 `ImpactPoint`와 `ImpactNormal`을 사용한다.

```text
첫 Blocking Hit
→ Impact FX 프로파일 선택
→ ImpactPoint에서 FX 1회
→ ImpactNormal 기준 회전
```

P0에서는 장갑 금속 Impact 한 종류를 Default에도 연결한다.
향후 표면별 시각 분기는 별도 기능 승인을 받은 뒤 확장한다.

P0 시각 구성:

```text
- 충돌 지점의 강한 순간 섬광
- 방사형 금속 스파크
- 짧은 금속 파편
- 소량의 회색 연기
- 매우 약한 청백색 전자기 방전
```

현재 관통과 도탄 판정이 없으므로 관통 구멍, 내부 폭발과 긴 도탄 궤적을 확정 표현으로 사용하지 않는다.

---

## 6. 차량 전투 불능 연출

Destroyed 상태는 차량 증발이나 대형 폭발이 아니라 전투 시스템이 손상된 전투 불능 상태로 표현한다.

순간 연출:

```text
- 강한 순간 스파크
- 내부 장치 파손 섬광
- 작은 금속 파편
- 냉각 또는 압력 계통 분출
- 짧은 전기 아크
```

지속 연출:

```text
NS_CF_DisableSmoke_P0
- 차량에 부착
- 약한 검은 연기 또는 회색 냉각 연기
- 추가 피해에서 중복 생성 금지
- 차량 초기화, 재스폰과 EndPlay에서 제거
```

금지 표현:

```text
- 차량 전체를 덮는 거대한 화염구
- 차체 증발
- 건물 붕괴 수준 충격파
- 완성형 Geometry Collection 분해
- 탄약고 유폭을 확정하는 연쇄 폭발
```

---

## 7. 카메라 셰이크

후보 자산:

```text
CS_CF_CannonFire_P0
```

실행 조건:

```text
발사 차량이 로컬 플레이어 소유
AND 실제 발사 실행 성공
→ Camera Shake 1회
```

초기 기준:

```text
지속 시간: 0.10 ~ 0.20초
강도: Reticle을 놓치지 않을 정도
위치 반응: 짧고 약한 충격
회전 반응: 매우 약하게 사용
감쇠: 빠른 시작과 빠른 종료
```

조준 복원을 요구하는 강한 반동은 별도 무기 반동 시스템으로 분리한다.

---

## 8. 완료 기준

```text
- 승인된 발사에서 Muzzle FX 1회
- 발사 거부에서 Muzzle FX와 Camera Shake 없음
- Projectile Trail 활성화와 Pool 반환 후 잔류 없음
- 첫 Impact에서 FX 1회와 중복 없음
- 최초 전투 불능에서 순간 FX와 지속 연기 각 1회
- 추가 피해에서 파괴 연출 중복 없음
- Sound 자산, Audio 클래스와 오디오 모듈 참조 0개
```

---

## 9. Changelog

### v0.2.0 - 2026-07-24

```text
- 프로젝트 전역 사운드 비지원 결정에 따라 Audio와 MetaSound 설계를 제거했다.
- 발사, Trail, Impact, 전투 불능과 Camera Shake의 시각 연출 기준만 유지했다.
- Sound 관련 완료 조건을 오디오 참조 0개 검증으로 대체했다.
```

### v0.1.0 - 2026-07-16

```text
- 과거 Combat FX / Audio 통합 Presentation Design 최초 작성.
- Audio 범위는 v0.2.0과 CF-PDL-0009로 대체됐다.
```

---

## 10. Migration

### v0.2.0 적용 안내

```text
- 기존 MetaSound, SoundWave, Sound Attenuation 후보를 생성하거나 연결하지 않는다.
- 시각 연출 자산명은 필요에 따라 유지할 수 있다.
- P0 구현 계약은 Parent Plan의 UCFCombatFxData와 UCFCombatFxComp 후보를 따른다.
- 과거 사운드 설계는 역사 기록으로만 해석한다.
```
