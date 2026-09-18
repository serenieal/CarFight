# 09. 락온 / 센서 / 전자전

- 문서 버전: v0.4.5
- 작성일: 2026-06-08
- 최근 갱신일: 2026-09-18
- 프로젝트: CarFight
- 저장 위치: Document/Plan/CombatPlan/09_락온_센서_전자전.md
- 상세 구조 설계: `Document/ProjectSSOT/CombatPlan/09A_TargetingSensorArch.md`

---

## 1. 핵심 정의

CarFight에서 락온은 자동 명중 시스템이 아니라, 센서, 배터리, 시야, 엄폐, 목표 기동, 방어장비가 얽힌 전투 압박 시스템이다.

CarFight는 EVE Online식 정보전과 락온 구조를 참고하지만, 우주함선식 `타겟 락 → 자동 교전` 구조를 그대로 사용하지 않는다. 자동차 전투답게 아래 요소로 재해석한다.

```text
시야
센서
소음
엄폐
연막
플레어
배터리
락온 안정도
목표의 속도와 횡방향 움직임
전자전 / EMP
```

---

## 2. Detection / Selection / Lock / Scan 역할 구분

CarFight는 EVE Online의 정보전 구조에서 **역할을 분리하는 원칙**을 참고하되, 자동차 액션 게임의 직접 조준 감각을 유지한다.

| 구분 | 역할 | 소유하지 않는 것 |
|---|---|---|
| Sensor Detection | 월드 대상을 감지하고 Contact를 생성·유지 | 플레이어 선택, 락온, 상세 Scan 진행 |
| Contact | 현재 센서가 관측·기억하는 대상의 추적 기록 | 플레이어 의도, 장비 준비 상태 |
| Selection | 플레이어가 현재 조작·정보 확인 대상으로 지정한 대상 | 센서 탐지, 락온, Knowledge 승격 |
| Target Lock | 차량 타겟팅 시스템이 특정 Contact를 안정적으로 추적하는 상태 | 자동 명중, Scan Knowledge 자체 |
| Scan / Analysis | 특정 Contact의 추가 정보를 획득하는 별도 작업 | 주변 광역 탐지, Target Lock 자체 |
| Knowledge | 플레이어가 대상에 대해 이미 획득한 정보 | 현재 Contact 수명, 현재 Scan 진행률 |
| Equipment | Selection / Lock / Knowledge를 읽어 장비별 사용 가능 여부를 판정 | 공용 선택·락 상태의 Authority |
| EW | Detection / Lock / Scan / 전자장비 품질을 방해 | 직접 피해를 주력으로 삼는 것 |

핵심 불변식은 다음과 같다.

```text
Selected != Contact
Contact != Locked
Locked != Scanning
Scanning != Knowledge
Knowledge != 현재 Contact 수명
```

한 시스템의 상태 전이가 다른 상태를 **암묵적으로 함께 켜거나 완료시키지 않는다.** 장비가 두 기능을 동시에 제공해야 한다면 상위 Gameplay Policy에서 두 명령을 명시적으로 조합한다.

### 2.1 타겟 선택과 차량 공용 Target Lock

타겟 선택과 Target Lock은 완전히 다른 상태다.

```text
Selection
= 플레이어가 지금 보고 조작하려는 대상
= UI / 명령의 기준
= 선택만으로 Sensor, Lock, Scan을 시작하지 않음

Vehicle Target Lock
= 차량의 Sensor / Targeting Suite가 특정 Sensor Contact 하나를 추적 대상으로 확보한 상태
= Guided Weapon, EW, Lock 보조 장비가 공용으로 소비
= Lock 자체는 자동 명중을 의미하지 않음
```

P0 기본 계약은 **단일 Vehicle Target Lock**이다. Selection과 Lock은 서로 독립이므로 다른 대상을 선택해 정보를 확인해도 기존 Lock은 자동으로 바뀌거나 해제되지 않는다. Lock 교체·해제는 명시적인 Target Lock 명령 또는 유지 조건 붕괴로만 일어난다.

락온 입력은 Selection과 별도 Gameplay Command로 설계한다. 실제 키 배치는 Input UX 단계에서 정하지만, Selection 입력이 Lock을 암묵적으로 시작하거나 Scan 입력이 Lock을 대신 만들지 않는다.

### 2.2 레이더 접촉

레이더는 월드에 존재하는 모든 Actor를 UI가 직접 검색해 표시하는 미니맵이 아니다.

```text
월드 대상
→ 센서 탐지 판정
→ Sensor Contact 생성
→ 레이더와 타겟 정보 UI가 Contact를 소비
```

Sensor Contact는 최소한 다음 정보를 제공할 수 있어야 한다.

```text
ContactId
WorldLocation
RelativeDirection
Distance
AltitudeDifference
DetectionConfidence
LastDetectedGameTime
IdentificationState
AffiliationState
bIsSelectedTarget
```

접촉이 레이더에 존재한다는 사실과 대상의 이름·진영·방어 상태가 공개됐다는 사실은 별개다.

### 2.3 가시 식별과 Target Scan

가시 식별과 Target Scan은 서로 다른 정보 획득 경로다.

| 경로 | 공개 가능한 정보 예 |
|---|---|
| 가시 식별 | 외형, 크기, 차량 유형, 외부 표식, 노출 무장, 외부 손상 |
| 센서 스캔 | 정확한 식별명, 진영, 쉴드·장갑·차량 상태, 내부 부품, 전자 신호 |

멀리 있는 대상은 `???` 또는 `UNKNOWN CONTACT`로 시작할 수 있다. 가까워지거나 충분히 관측하면 가시 정보 일부가 공개되고, 스캔이 진행되면 기술 정보가 추가로 공개된다.

하나의 전역 식별 단계로 모든 필드 공개를 강제하지 않는다.

```text
IdentityKnowledge
AffiliationKnowledge
DefenseKnowledge
WeaponKnowledge
ComponentKnowledge
ThreatKnowledge
```

P0에서는 단순 단계형으로 시작할 수 있지만, 데이터 계약은 분야별 공개 상태로 확장 가능해야 한다.

### 2.4 Active Detection과 Target Scan 분리

기존 구현처럼 Target Scan을 시작하면서 광역 Active Detection이 함께 켜지는 암묵적 결합은 새 설계의 기준이 아니다.

```text
Active Detection Pulse
= 일정 시간 Sensor Detection 능력을 강화하거나 탐지 범위를 확장
= 주변 Contact 발견·갱신 목적
= Target Knowledge를 직접 올리지 않음

Target Scan / Analysis
= 선택한 Sensor Contact 하나의 Knowledge를 분석
= 기본형은 Selection + 유효 Contact를 요구
= Target Lock을 자동 생성하지 않음
= 광역 Active Detection을 자동 시작하지 않음
```

특정 Scanner 장비가 두 기능을 모두 제공하는 것은 허용한다. 다만 그 경우에도 내부적으로는 다음처럼 명시적으로 조합한다.

```text
Scanner Capability
→ RequestActiveDetectionPulse()
+ RequestTargetScan(SelectedContact)
```

두 기능이 하나의 `bActiveScanRunning` 상태를 공유하던 구조는 2026-09-17 Phase 1에서 제거했다. Phase 2에서는 Contact/HUD/Radar의 legacy `AnalysisProgress01` 경로를 제거하고 Sensor canonical command를 `StartActiveDetectionPulse / StartTargetScan / CancelSensorOperations`로 정리했다. 구형 `StartActiveScan / StartTargetedScan / StopActiveScan` 계열은 저장된 Blueprint·기존 C++ caller 보호용 Deprecated compatibility wrapper로만 유지한다. Vehicle Target Lock과 Knowledge Entity Identity는 후속 Migration으로 남는다.

### 2.5 Knowledge 수명과 정보 Freshness

Contact 수명과 획득 Knowledge 수명을 분리한다.

```text
Contact
= 현재 탐지/추적 정보
= Live / LastKnown / Lost / DestroyedHold

Persistent Knowledge
= 한 번 확인하면 대상 Identity 기준으로 장기 보존 가능한 정보
= 예: 차량 종류, 진영, 기본 장비 분류

Dynamic Knowledge
= 시간이 지나면 낡을 수 있는 정보
= 예: 현재 체력, 현재 장착 상태, 현재 모듈 손상
= LastUpdatedTime / Freshness를 별도로 가짐
```

같은 대상의 안정적인 `TargetEntityId`를 다시 확인할 수 있으면 새 Contact를 획득해도 Persistent Knowledge를 재연결할 수 있다. `ContactId`는 Sensor 추적 기록, `TargetEntityId`는 실제 대상 개체, `TargetTypeId / ArchetypeId`는 차량 종류를 식별한다. 현재 VehicleData PrimaryAssetId 기반 `TargetId`는 차량 종류 성격이므로 실제 개체 `TargetEntityId`로 재사용하지 않는다. Identity를 증명할 수 없는 새 Contact에 과거 Knowledge를 추측으로 복사하지 않는다.

현재 `Detected / Identified / DetailedScan` 단계형 계약은 P0 호환 Projection으로 유지할 수 있지만, 장기 구조에서는 분야별 Knowledge와 Freshness가 실제 Authority가 된다.

---

## 3. 센서가 제공해야 하는 정보

| 정보 | 설명 | 전투 영향 |
|---|---|---|
| 적 위치 | 대략적 또는 정확한 위치 | 접근 / 엄폐 판단 |
| 거리 | 현재 목표까지 거리 | 무기 유효거리 판단 |
| 상대 속도 | 적의 이동 속도 | 추격 / 이탈 판단 |
| 상대 횡방향 속도 | 내 기준 좌우로 얼마나 빠르게 움직이는지 | 무기 각속도 / 락온 안정도 판단 |
| 장갑 방향 | 정면 / 측면 / 후면 노출 여부 | 탄종 / 공격 방향 판단 |
| 락온 가능 여부 | 현재 락온 가능한 상태인지 | 미사일 / 센서 보조 사용 |
| 모듈 상태 | 바퀴 / 엔진 / 터렛 손상 여부 | 약점 공격 판단 |
| 전자장비 신호 | 배터리 / 센서 / 락온 사용 흔적 | 전자전 판단 |

### P0 센서 정보

```text
01. Sensor Contact 식별자
02. 목표 위치와 상대 방향
03. 목표 거리
04. 목표 속도
05. 탐지 신뢰도와 마지막 탐지 시간
06. 현재 선택 대상 여부
07. 현재 공개 가능한 정보 상태
08. 현재 Contact가 Vehicle Target Lock 후보가 될 수 있는지에 필요한 Sensor 관측 정보
```

`LockState / LockProgress / LockQuality / LockedContactId`는 Sensor 원본 정보나 개별 무기 상태가 아니라 차량 공용 `Vehicle Targeting Runtime`이 소유한다. Sensor는 거리·Contact 상태·관측 품질처럼 Lock 판정에 필요한 입력을 제공하고, 장비는 공용 Locked Target을 자신의 사용 조건에 따라 소비한다. 정확한 방어 수치와 내부 부품 상태는 가시 식별만으로 자동 공개하지 않고 Target Scan 또는 별도 정보 조건을 요구한다.

---

## 4. Target Lock 기본 정의

Target Lock은 선택 UI나 개별 무기의 내부 상태가 아니라 **차량 공용 Targeting Runtime**이다.

차량은 유효한 Sensor Contact 하나에 대해 Lock 획득을 요청하고, Sensor / 거리 / 가시성 / 상대 기동 / 전자전 조건을 만족하는 동안 Lock Progress와 Lock Quality를 유지한다. Lock이 완료되면 Guided Weapon, EW, 추적 보조 장비가 동일한 Locked Target을 읽을 수 있다.

```text
Target Lock Authority
= Vehicle Targeting Runtime

Target Lock Consumer
= Missile / Guided Weapon / EW / Targeting Assist / 일부 Scanner
```

개별 장비는 공용 Lock을 소유하지 않고 `이 장비가 현재 Locked Target을 사용할 수 있는가`를 판정한다. Target Homing Missile은 발사 승인 순간 Vehicle Locked Target을 Launch Context로 Snapshot하고, 발사 후 미사일 Seeker 추적은 별도 Projectile/Seeker Runtime 책임이다. 발사 뒤 Selection이나 Vehicle Lock이 바뀌어도 이미 발사된 P0 Missile 목표는 자동 변경하지 않는다.

락온은 `잡히면 무조건 맞는 상태`가 아니다.

| 기능 | 설명 |
|---|---|
| 미사일 발사 조건 | 락온 완료 후 유도 무기 사용 |
| 목표 추적 보조 | 터렛 / 짐벌 안정화 보정 가능 |
| 예측 조준 보조 | 리드샷 UI 제공 가능 |
| 약점 정보 표시 | 장갑 방향 / 모듈 상태 표시 가능 |
| 전자전 대상 지정 | 재밍 / EMP / 센서 방해 대상 지정 |

P0에서는 `미사일 발사 조건 + 공용 Target Lock 게이지`만 우선 구현한다.

Target Lock 획득 방식은 개별 무기가 아니라 차량의 `Targeting Profile / Targeting Runtime Policy`가 소유한다.

```text
CrosshairHold
TargetCone
SensorTrack
ManualDesignate
```

P0 기본값은 `CrosshairHold` 또는 제한된 `TargetCone`으로 두고, Lock 요청된 Contact를 유효 조준 영역에 유지하는 동안 공용 Lock Progress가 상승하도록 한다.

개별 장비는 획득 방식을 소유하지 않고 P0에서는 실제 의미가 확정된 Lock 요구 정책 exact2만 가진다.

```text
NoLockRequired
VehicleLockRequired
```

`VehicleLockPreferred`는 Lock 유무에 따른 실제 무기 동작 차이가 정의된 Consumer가 생기기 전에는 선행 도입하지 않는다.

`LaserGuide`처럼 공용 Vehicle Lock과 다른 표적 지정 방식이 필요하면 해당 장비의 별도 Guidance / Designation Capability로 추가하며, 공용 Target Lock 획득 모드에 섞지 않는다.

---

## 5. Target Lock 진행 구조

P0 Target Lock 상태는 단일 Locked Target 기준으로 다음처럼 관리한다.

```text
Idle
  ↓ Request Lock(Selected Contact)
Acquiring
  ↓ Progress 1.0
Locked
  ↓ 유지 조건 약화
Locked + Quality 감소
  ↓ Quality / 유지 한계 붕괴
Break Event → Idle
```

Gameplay 흐름:

```text
01. TargetSelect가 플레이어의 Selected Target을 제공한다.
02. Targeting Runtime이 Selected Actor와 Sensor Contact를 연결한다.
03. 유효 Contact가 있으면 명시적인 Lock 요청으로 Acquiring을 시작한다.
04. Sensor 범위, 유효 추적 조건, LOS, 거리, 목표 기동을 평가해 Lock Progress가 상승한다.
05. 완료하면 Locked Target ContactId를 공용으로 공개한다.
06. 엄폐 / 연막 / 빠른 횡이동 / 전자전은 Lock Quality를 감소시킨다.
07. Lock 유지 조건이 완전히 붕괴되면 Lock을 해제한다.
08. Lock이 끊겨도 Selection은 별도 규칙에 따라 유지될 수 있다.
09. Selection을 다른 대상으로 옮겨도 기존 Lock은 자동 교체하지 않는다.
10. 단일 Lock에서 명시적으로 다른 Contact에 Request Lock하면 기존 Lock/Acquire를 종료하고 새 Contact Acquiring을 시작한다.
11. 같은 대상 중복 Lock 요청은 상태를 재시작하지 않고 no-op/fail-visible 처리한다.
12. Scan 시작·완료는 Lock 상태를 자동 생성·해제하지 않는다.
13. Lock 붕괴는 지속 Broken State가 아니라 Break Transition Event로 기록하고 즉시 Idle로 복귀한다.
```

핵심은 락온이 한 번 잡으면 끝나는 상태가 아니라 계속 유지해야 하는 전투 상태이며, Selection / Scan과 독립적으로 존재한다는 점이다.

---

## 6. 락온 안정도

락온에는 완료 / 미완료만 있는 것이 아니라, 중간 상태인 락온 안정도가 있어야 한다.

| 요소 | 영향 |
|---|---|
| 목표와의 거리 | 멀수록 락온 시간 증가 |
| 목표 횡방향 속도 | 빠를수록 락온 안정도 감소 |
| 목표 급가속 / 급제동 | 락온 흔들림 |
| 엄폐물 | 락온 차단 |
| 연막 | 광학 / 열 추적 방해 |
| 플레어 | 미사일 추적 방해 |
| 재밍 | 센서 / 락온 품질 저하 |
| 내 배터리 상태 | 배터리 부족 시 락온 유지력 감소 |
| 센서 성능 | 좋을수록 락온 속도 / 안정도 증가 |

```text
빠른 차량은 단순 회피율이 높은 것이 아니라,
상대의 락온 안정도를 흔드는 방식으로 생존해야 한다.
```

---

## 7. 직접 조준과 락온의 관계

CarFight는 직접 조준 게임이므로, 락온이 직접 조준을 대체하면 안 된다.

| 방식 | 역할 |
|---|---|
| 직접 조준 | 플레이어가 직접 맞히는 주 전투 감각 |
| 짐벌 / 터렛 보정 | 직접 조준을 보조하지만 각속도 제한을 받음 |
| 락온 | 미사일 / 센서 / 전자전 / 보조 조준을 위한 추적 상태 |
| 자동 명중 | 최대한 피해야 함 |

락온 무기가 강해질수록 대응 수단도 반드시 있어야 한다.

```text
락온 무기 강함
→ 배터리 소모
→ 락온 시간 필요
→ 엄폐로 차단 가능
→ 연막 / 플레어 / 재밍으로 대응 가능
→ 탄약 중량 부담
```

---

## 8. 배터리와 센서 / 락온의 관계

배터리는 총을 쏘는 마나가 아니다.

배터리는 센서, 락온, 전자전, 능동 방어, 터렛 안정화 같은 전장 전자장비의 전력 자원이다.

| 사용처 | 설명 |
|---|---|
| 능동 스캔 | 짧은 시간 탐지 성능 증가 |
| 빠른 락온 | 락온 시간 단축 |
| 락온 유지 보조 | 목표가 빠르게 움직여도 안정도 유지 |
| 터렛 안정화 | 조준 흔들림 감소, 안정화 시간 단축 |
| 짐벌 보정 강화 | 제한 시간 동안 추적 성능 증가 |
| 플레어 / 연막 / 요격장치 | 방어장비 작동 |
| 재밍 | 적 센서 / 락온 방해 |
| EMP 대응 | 전자 피해 저항 또는 복구 |

```text
배터리는 강한 기능을 쓰게 해주지만,
고갈되면 전자장비 의존 전투가 약해져야 한다.
```

배터리가 0이 되었다고 차량이 멈추면 안 된다. 자동차의 기본 주행은 연료 / 엔진 쪽이고, 배터리는 전장 장비 운용에 가깝다.

---

## 9. 락온 대응 수단

락온 무기가 불쾌하지 않으려면 대응 수단이 명확해야 한다.

| 대응 수단 | 역할 |
|---|---|
| 엄폐 | 센서선 / 시야 차단 |
| 연막 | 광학 / 열 추적 방해 |
| 플레어 | 미사일 추적 교란 |
| 고속 횡이동 | 락온 안정도 감소 |
| 급가속 / 급제동 | 추적 예측 흔들기 |
| 재밍 | 적 락온 속도 / 안정도 저하 |
| EMP | 적 센서 / 배터리 / 락온 기능 방해 |
| 센서 차폐 | 락온 저항 증가 |
| 요격장치 | 일부 미사일 / 로켓 격추 |

### P0 대응 수단

```text
01. 엄폐
02. 연막 또는 플레어 1종
03. 목표 횡방향 속도에 따른 락온 안정도 감소
```

---

## 10. 연막과 플레어의 차이

### 연막

| 항목 | 내용 |
|---|---|
| 역할 | 시야 / 센서 / 락온 차단 |
| 장점 | 자동차 전투와 지형전 감각이 잘 맞음 |
| 단점 | 시각 효과와 AI 시야 처리 필요 |
| 어울리는 상황 | 이탈, 재장전, 냉각, 락온 끊기 |

연막은 차량 전투 아이덴티티가 강하다. 도주, 재정비, 엄폐와 잘 맞는다.

### 플레어

| 항목 | 내용 |
|---|---|
| 역할 | 미사일 추적 교란 |
| 장점 | 기능이 명확하고 구현 범위가 좁음 |
| 단점 | 미사일 대응에만 치우침 |
| 어울리는 상황 | 미사일 경고 후 타이밍 대응 |

### P0 추천

P0에서는 플레어보다 연막을 먼저 추천한다.

```text
연막은 미사일뿐 아니라 직접 조준, 센서, 락온, 재정비 흐름과 모두 연결된다.
자동차 전투에서 도주 / 재접근 / 엄폐 플레이를 만들기 좋다.
```

단, 구현을 더 단순하게 하려면 플레어부터 넣어도 된다.

---

## 11. 전자전의 역할

전자전은 P0의 핵심은 아니지만, CarFight의 전략성을 키우는 데 중요하다.

| 대상 | 효과 |
|---|---|
| 센서 | 탐지 거리 감소, 정보 품질 저하 |
| 락온 | 락온 시간 증가, 락온 안정도 감소 |
| 미사일 | 추적 성능 저하 |
| 터렛 안정화 | 각속도 감소, 안정화 시간 증가 |
| 짐벌 보정 | 보정각 / 추적 안정성 감소 |
| 배터리 | 회복 속도 감소, 순간 출력 제한 |
| 능동 방어 | 연막 / 플레어 / 요격장치 사용 지연 |

| 장비 | 설명 |
|---|---|
| 재밍 장치 | 일정 범위 적의 락온 성능 저하 |
| 센서 교란기 | 적 UI 정보 품질 저하 |
| EMP탄 | 명중 시 배터리 / 센서 계통 방해 |
| EMP 미사일 | 락온 기반 전자 피해 |
| 차폐 장비 | EMP / 재밍 저항 |
| 보조 배터리 | 전자전 중에도 장비 유지력 증가 |

전자전은 직접 피해가 낮아야 한다. 대신 상대가 의존하는 시스템을 약하게 만들어야 한다.

```text
전자전은 적을 죽이는 무기가 아니라,
적이 잘 싸우기 위해 필요한 조건을 망가뜨리는 무기다.
```

---

## 12. 전자전 도입 시점

P0에 전자전을 넣으면 복잡도가 너무 커질 수 있다.

전자전은 아래 시스템이 먼저 있어야 의미가 생긴다.

```text
센서 정보
락온 안정도
배터리 자원
터렛 / 짐벌 보정
방어장비
UI 피드백
```

따라서 전자전은 P1 이후에 도입한다.

P0에서는 EMP / 재밍 직접 구현을 제외하고, 락온 안정도와 연막 / 엄폐 대응만 먼저 구현한다.

---

## 13. 센서 등급과 피팅

나중에 피팅 시스템으로 가면 센서도 장비화할 수 있다.

| 센서 타입 | 강점 | 약점 |
|---|---|---|
| 기본 센서 | 균형형 | 특화 없음 |

> **P0 Basic Sensor 초기 운용값** — Vehicle Builder가 정상 신규 차량에 공통 기본 센서를 자동 연결하기 위해 `DA_VehicleSensor_Basic`을 사용한다. 현재 운용값은 Passive 20m / Visual 30m / Active 40m, Active Scan 3초, Update 0.2초, Contact Memory 2초, Destroyed Hold 1초, Analysis Gain **0.40/s**, Decay 0.15/s, Radar 10/20/40m(기본 20m)로 둔다. 이 값은 **최종 밸런스 확정값이 아니라 P0 기술·플레이테스트 baseline**이지만, Tactical Analysis가 활성인 Sensor는 정상 범위·직접 가시·유효 Target 조건을 Scan 전체에 유지하면 **Active Scan 1회 안에 DetailedScan까지 완료 가능해야 한다.** 최소 설정 관계는 `AnalysisGainPerSec * max(ActiveScanDurationSec - UpdateIntervalSec, 0) >= DetailedScanThreshold`이며 마지막 만료 update에 의존하지 않도록 Update 1회 안전 여유를 둔다. 의도적인 다회 스캔 장비는 별도 명시 계약이 생기기 전까지 만들지 않는다. Actor 검사 상한과 Identified/Detailed 임계값처럼 Native Sensor 계약 기본값과 동일한 항목은 이 P0 밸런스 baseline에서 중복 소유하지 않는다. 구조 계약은 `Scanner override > Vehicle Basic > zero-range fallback`이고, 의도적인 Sensorless 차량은 별도 명시한다.
| 장거리 센서 | 먼 거리 탐지 / 락온 | 근거리 난전 대응 낮음 |
| 고속 추적 센서 | 빠른 목표 락온 안정 | 탐지 거리 짧음 |
| 장갑 약점 스캐너 | 장갑 방향 / 모듈 정보 표시 | 배터리 소모 큼 |
| 전자전 센서 | 재밍 저항 / EMP 감지 | 직접 전투 보조 낮음 |

센서도 EVE Online식 피팅 전략성을 가지되, 자동차 전투에 맞게 해석한다.

---

## 14. 락온과 무기 타입의 관계

| 무기 타입 | 락온 필요성 | 설명 |
|---|---:|---|
| 기관총 | 낮음 | 직접 조준 / 짐벌 중심 |
| 오토캐논 | 낮음 | 직접 조준 중심 |
| 대구경포 | 낮음 | 직접 조준 / 예측 조준 중심 |
| 로켓 | 낮음~중간 | 기본은 비유도, 일부 보조 가능 |
| 미사일 | 높음 | 락온 핵심 무기 |
| 레일건 / 코일건 | 낮음 | 직접 정밀 조준 중심 |
| EMP 무기 | 중간~높음 | EMP탄은 직접, EMP 미사일은 락온 |
| 방어장비 | 중간 | 위협 감지 / 타이밍 대응 |
| 지뢰 / 설치물 | 낮음 | 지역 통제 중심 |

락온은 모든 무기에 붙는 공통 시스템이 아니라, 미사일 / 전자전 / 센서 보조 무기의 핵심 조건으로 둔다.

---

## 15. 락온 UI 방향

락온 시스템은 UI가 없으면 플레이어가 이해하기 어렵다.

| UI | 역할 |
|---|---|
| 락온 가능 표시 | 현재 목표를 락온할 수 있는지 |
| 락온 진행 게이지 | 완료까지 얼마나 남았는지 |
| 락온 안정도 표시 | 흔들리는지, 끊기기 직전인지 |
| 락온 경고 | 내가 락온당하고 있는지 |
| 미사일 경고 | 발사체가 날아오는지 |
| 연막 / 플레어 준비 상태 | 대응 가능 여부 |
| 배터리 잔량 | 락온 / 센서 / 방어장비 사용 가능 여부 |

### P0 락온 / 센서 UI

```text
01. 선택 타겟 고정 정보 패널
02. 미공개 필드의 ??? 표시
03. Sensor Contact 기반 레이더
04. 목표 락온 게이지
05. 락온 완료와 실패 원인 표시
06. 내가 락온당함 경고
07. 미사일 접근 경고
08. 연막 / 플레어 사용 가능 표시
09. 배터리 잔량
```

---

## 16. P0 최소 Targeting / Sensor / Scan 시스템

```text
01. Sensor Detection / Contact / Selection / Target Lock / Target Scan / Knowledge를 별도 상태로 둔다.
02. TargetSelect는 플레이어 Selection만 소유하며 Lock·Scan을 자동 시작하지 않는다.
03. Vehicle Targeting Runtime은 단일 Target Lock을 소유한다.
04. 명시적 Lock 요청 시 Selected Target의 유효 Sensor Contact를 대상으로 Acquiring을 시작한다.
05. 일정 시간 유효 추적 조건을 유지하면 Locked 상태가 된다.
06. 엄폐 / 거리 / 횡방향 속도 / 연막은 Lock Progress 또는 Lock Quality를 약화시킨다.
07. Guided Missile과 일부 EW/보조 장비만 Vehicle Target Lock을 요구한다.
08. Direct Fire 무기는 Lock 없이 직접 조준·발사할 수 있다.
09. Target Scan은 선택 Contact 하나의 Knowledge 분석 작업이며 Vehicle Target Lock을 자동 생성하지 않는다.
10. Active Detection Pulse는 주변 Contact 탐지 강화 작업이며 Target Scan을 자동 시작하지 않는다.
11. Scan Attempt Progress는 Attempt에 귀속하고 중단·완료 시 0으로 돌아가며, 획득 Knowledge만 별도로 보존한다.
12. Persistent Knowledge와 Dynamic Knowledge Freshness를 분리할 수 있는 데이터 구조를 사용한다.
13. Sensor Contact와 최소 Radar 표시를 제공한다.
14. 선택 타겟의 미공개 정보는 ???로 유지한다.
15. Scan/Lock 실패 사유는 UI가 소비할 수 있는 명시 결과로 제공한다.
```

---

## 17. P0 포함 / 제외 범위

### P0 포함

| 시스템 | 이유 |
|---|---|
| 기본 센서 범위 | 적 탐지 / 락온 조건 |
| 락온 게이지 | 미사일 사용 조건 |
| 배터리 소모 | 자동차식 전자장비 자원 검증 |
| 엄폐로 락온 차단 | 지형 전투 검증 |
| 횡방향 속도에 따른 락온 저하 | 차량 기동 가치 검증 |
| 연막 또는 플레어 1종 | 락온 대응 수단 |

### P0 제외

| 시스템 | 제외 이유 |
|---|---|
| 복잡한 센서 타입 | 피팅 복잡도 증가 |
| 다중 락온 | UI / AI 부담 큼 |
| EMP탄 | 전자 피해 피드백 필요 |
| 재밍 장비 | 락온 / 센서 UI 정리 후 적합 |
| 센서 모듈 손상 | 모듈 UI / 정보 품질 시스템 필요 |
| 배터리 계통 손상 | 배터리 운용이 먼저 안정되어야 함 |
| 약점 스캔 | 장갑 / 모듈 UI가 먼저 필요 |

---

## 18. 설계 원칙

```text
01. 락온은 확정 명중이 아니라 전투 압박 상태다.
02. 락온은 센서 범위, 시야, 거리, 목표 움직임, 배터리 상태에 영향을 받는다.
03. 빠른 차량은 락온 안정도를 흔드는 방식으로 생존할 수 있어야 한다.
04. 엄폐, 연막, 플레어, 재밍은 락온에 대한 명확한 대응 수단이다.
05. 배터리는 총을 쏘는 마나가 아니라 센서 / 락온 / 방어장비 / 전자전의 전력 자원이다.
06. 전자전은 직접 피해보다 적의 정보전, 락온, 배터리, 안정화 능력을 방해하는 역할이다.
07. 락온 무기는 직접 조준 무기를 대체하면 안 된다.
08. 대응 없는 락온은 금지한다.
```

대응 없는 락온은 불쾌하고, 대응 가능한 락온은 전술이 된다.

---

## 19. 결정안

```text
CarFight Targeting / Sensor / Scan / EW v0.4.0

핵심 상태:
Detection / Contact / Selection / Target Lock / Target Scan / Knowledge
는 서로 독립된 상태와 책임을 가진다.

Sensor Detection:
- 주변 대상을 감지한다.
- Contact를 생성하고 Live / LastKnown / Lost 수명을 관리한다.
- Selection, Lock, Scan을 자동 변경하지 않는다.

Selection:
- 플레이어가 현재 보고 조작하려는 대상을 지정한다.
- Selection 자체는 Sensor/Lock/Scan gameplay 상태를 발생시키지 않는다.

Vehicle Target Lock:
- 차량 공용 Targeting Runtime이 단일 Contact를 추적 대상으로 확보한다.
- Guided Weapon / EW / 일부 보조 장비가 공용으로 소비한다.
- 개별 무기는 공용 Lock의 owner가 아니다.
- 직접 조준 무기는 Lock을 요구하지 않는다.

Target Scan / Analysis:
- 선택 Contact 하나의 추가 Knowledge를 획득하는 별도 작업이다.
- Target Lock을 자동 생성하지 않는다.
- Active Detection Pulse를 자동 실행하지 않는다.
- 현재 Attempt progress와 획득 Knowledge는 별도 상태다.

Knowledge:
- Contact 수명과 분리한다.
- Persistent 정보와 시간이 지나면 낡는 Dynamic 정보의 Freshness를 구분할 수 있어야 한다.
- Detected / Identified / DetailedScan은 P0 호환 Projection으로 유지할 수 있다.

Active Detection Pulse:
- 주변 Contact 발견 능력을 강화하는 Sensor 작업이다.
- Target Scan과 내부 상태를 공유하지 않는다.

배터리:
- 센서 강화, Target Lock 유지, 터렛 안정화, 방어장비, 전자전에 사용한다.
- 총을 쏘기 위한 마나가 아니다.

P0 구현 순서:
01. 현재 Sensor/Scan 상태 모델을 새 6축 책임 모델에 맞춰 분리 [Phase 1 완료]
02. Active Detection과 Target Scan의 공유 Runtime state 제거 [Phase 1 완료]
03. Legacy Contact/HUD Scan Progress 제거 + Sensor/Pawn command naming 정리 [Phase 2 Fresh Technical PASS]
04. TargetEntityId Foundation + Persistent Knowledge Store/Reassociation [Phase 3~3B-1 Technical PASS]
05. Vehicle Target Lock Runtime 추가 [Phase 4 Technical PASS]
06. Selection / Lock / Scan 명령 경계를 Vehicle Targeting까지 확장 [Phase 5 Technical PASS]
07. HUD에 Selection / Lock / Scan 상태를 독립 표현 [Phase 6 Technical PASS]
08. Guided Missile이 Vehicle Target Lock을 소비하도록 연결 [Phase 7 Technical PASS]
09. 엄폐 / 횡이동 / 연막 대응 [후속]
10. 실패 사유 UI 계약 고도화 [후속]

P1 이후:
01. 다중 Target Lock
02. 분야별 Knowledge와 Dynamic Freshness 확장
03. EMP탄 / 재밍 장비
04. 센서 모듈 손상
05. 배터리 계통 손상
06. 약점 Scan
07. 센서 타입 피팅 고도화
```

현재 `UCFVehicleSensorComp` / `FCFSensorSnapshot`은 Phase 1 Active Detection Pulse/Target Scan Runtime state 분리와 Phase 2 legacy Scan Progress/API naming 정리를 반영했고, Phase 3~3B-1 TargetEntityId/Persistent Knowledge, Phase 4 Vehicle Target Lock Runtime, Phase 5 Gameplay Command, Phase 6 HUD Presentation, Phase 7 Guided Weapon Locked Target source migration까지 Technical PASS로 반영됐다. `Contact.AnalysisProgress01`은 제거됐고 Current Scan progress는 `Snapshot.ScanAttempt.Progress01`이 소유한다. 전체 v0.4.x 설계에서 아직 선행 구현하지 않은 주요 후속은 실제 요구 발생 시의 Phase 3B-2 Dynamic Knowledge Domain/Freshness/Rescan과 상위 P0/P1 EW·대응수단 확장이다.

---

## 20. Changelog

### v0.4.5 - 2026-09-18

- `09A_TargetingSensorArch.md v0.1.16` 기준으로 Phase 3~7 Current 구현 상태를 상위 CombatPlan에 동기화했다.
- TargetEntityId/Persistent Knowledge, Vehicle Target Lock, Gameplay Command, HUD Presentation, Guided Weapon Locked Target source migration을 모두 Technical PASS 상태로 갱신했다.
- 현재 잔여 후속을 Phase 3B-2 Dynamic Knowledge Domain/Freshness/Rescan과 엄폐·횡이동·연막/EW 대응 확장으로 명확히 분리했다.

### v0.4.4 - 2026-09-17

- `09A_TargetingSensorArch.md v0.1.4`와 Current `SensorContact.md v1.11.0`에 맞춰 Phase 2 legacy Scan Progress 제거와 Sensor/Pawn canonical API naming 구현 사실을 상위 기획에 동기화했다.
- `Contact.AnalysisProgress01`과 Target/Radar HUD legacy progress 경로가 제거됐고 Current Scan progress Authority는 `Snapshot.ScanAttempt.Progress01`임을 명확히 했다.
- canonical command를 `StartActiveDetectionPulse / StartTargetScan / CancelSensorOperations`로 정리하고 구형 ActiveScan/TargetedScan 이름은 compatibility wrapper로만 유지하는 정책을 반영했다.
- 남은 Transitional 범위를 TargetEntityId, Vehicle Target Lock, Guided Weapon source migration과 후속 Lock/HUD/EW 단계로 축소했다.
- Phase 2는 최종 코드 기준 Official UE 5.8 Build `966325479f6e4a05ab75fa350822fa45` PASS, `CarFight.Sensor` 16/16 PASS, `CarFight.UI.UI_P0_07` 2/2 PASS로 Fresh Technical PASS다. UI 회귀에서 발견된 Contact Unknown completion feedback 잔존 결함은 같은 Phase에서 교정 후 재검증했다.

### v0.4.3 - 2026-09-17

- `09A_TargetingSensorArch.md v0.1.3` 및 Current SensorContact v1.10.0에 맞춰 Phase 1 Active Detection / Target Scan Runtime state split 완료 사실을 상위 기획에 동기화했다.
- `bActiveScanRunning` shared-state 결합은 더 이상 Current blocker가 아니며, 당시 남아 있던 legacy Scan progress/naming은 v0.4.4 / Phase 2에서 제거·compatibility migration 완료됐다. 현재 잔여 범위는 TargetEntityId, Vehicle Target Lock과 Guided Weapon source migration이다.

### v0.4.2 - 2026-09-17

- `09A_TargetingSensorArch.md v0.1.1` 설계검수 교정에 맞춰 `ContactId / TargetEntityId / TargetTypeId` 식별자 역할을 상위 기획에도 반영했다.
- 현재 VehicleData PrimaryAssetId 기반 TargetId를 실제 개체 Entity Identity로 간주하지 않도록 명시했다.
- P0 Lock Requirement를 `NoLockRequired / VehicleLockRequired` exact2로 축소하고 의미가 미정인 `VehicleLockPreferred` 선행 도입을 제거했다.
- `Broken`을 지속 Lock State가 아니라 Break Transition Event로 정리하고 단일 Lock의 명시적 대상 교체/중복 요청 규칙을 추가했다.
- Target Homing Missile은 발사 전 Vehicle Lock을 소비하고 발사 순간 Locked Target을 Snapshot한 뒤 발사 후 Seeker가 독립적으로 목표를 소유하도록 상위 기획을 동기화했다.

### v0.4.1 - 2026-09-17

- 재설계의 상세 Runtime 책임, 상태 모델, C++/DataAsset/Blueprint 역할, Migration 순서와 Technical Validation 기준을 `09A_TargetingSensorArch.md v0.1.0`으로 분리해 공식 상세 설계 문서로 연결했다.
- `09_LockSensorEW.md`는 전투 기획과 상위 원칙을 소유하고, `09A_TargetingSensorArch.md`는 해당 원칙의 구현 전 Architecture Baseline을 소유하도록 문서 책임을 분리했다.

### v0.4.0 - 2026-09-17

- EVE Online의 역할 분리 원칙을 CarFight 액션 전투에 맞게 재해석해 `Detection / Contact / Selection / Target Lock / Target Scan / Knowledge` 6축 책임 모델을 확정했다.
- 기존 장비별 Lock ownership을 차량 공용 `Vehicle Target Lock` Runtime으로 재설계하고 Guided Weapon / EW / 보조 장비는 이를 소비하도록 방향을 변경했다.
- Selection은 UI/명령 의도만 소유하며 Sensor, Lock, Scan을 암묵적으로 시작하지 않는다고 고정했다.
- `Active Detection Pulse`와 `Target Scan / Analysis`를 별도 Gameplay Operation으로 분리하고 동일 `bActiveScanRunning` 상태 공유를 Migration 대상 구조로 판정했다.
- Scan Attempt 진행률과 영구 Knowledge를 분리하고 Contact 수명과 Knowledge 수명도 별도 축으로 고정했다.
- Persistent Knowledge와 Dynamic Knowledge Freshness 확장 방향을 추가했다.
- Direct Fire는 Lock 불필요, Guided Weapon / 일부 EW·보조 장비만 Lock 요구라는 CarFight 액션성 보존 원칙을 명시했다.
- 당시 v1.9~v1.10 Sensor/Targeted Scan Source를 Transitional Implementation으로 판정했다. 이후 2026-09-17 Phase 1에서 Active Detection / Target Scan Runtime state split은 구현 완료됐고 나머지 migration만 잔여한다.

### v0.3.2 - 2026-09-16

- USER PIE에서 Basic Sensor Active Scan이 약 72~75%까지만 상승한 뒤 decay하는 문제를 확인했다. 원인은 `3초 × 0.25/s < DetailedScanThreshold 1.0`인 상호모순 설정이었다.
- Basic Sensor Analysis Gain을 `0.40/s`로 교정해 정상 조건에서 약 2.5초에 100%에 도달하고 3초 Scan 만료 전에 DetailedScan을 완료하도록 했다.
- 단일 필드 유효성만으로 Sensor 성립성을 판정하지 않고, Tactical Analysis가 활성인 Sensor는 `Gain × max(Duration - UpdateInterval, 0) >= DetailedThreshold`를 만족해야 한다는 cross-field gameplay invariant를 ProjectSSOT로 고정했다.
- 의도적인 multi-pass 분석은 우연한 수치 조합으로 만들지 않고 별도 명시 계약이 생긴 경우에만 허용한다.

### v0.3.1 - 2026-09-16

- Vehicle Builder 정상 신규 차량용 공통 `DA_VehicleSensor_Basic`의 P0 기술·플레이테스트 baseline을 확정했다.
- 초기 운용값을 Passive 20m / Visual 30m / Active 40m, Active Scan 3초, Update 0.2초, Contact Memory 2초, Destroyed Hold 1초, Analysis Gain 0.25/s, Decay 0.15/s, Radar 10/20/40m(기본 20m)로 명시했다.
- 이 수치를 최종 밸런스와 분리하고 `Scanner override > Vehicle Basic > zero-range fallback` source priority 및 explicit Sensorless 예외를 고정했다.
- Native Sensor 계약 기본값과 동일한 Actor 검사 상한/정보 임계값은 P0 밸런스 owner에서 중복 소유하지 않는다.

### v0.3.0 - 2026-07-30

- 타겟 선택과 장비 락온의 상태·소유권을 분리했다.
- 기본 락온을 선택 타겟 조준 영역 유지형으로 확정하고 무기별 획득 방식 확장을 추가했다.
- 미니맵이 아닌 Sensor Contact 기반 레이더 계약을 추가했다.
- 가시 식별과 센서 스캔을 별도 정보 획득 경로로 분리했다.
- 선택 타겟 패널의 미공개 필드 `???`와 분야별 Knowledge 확장 원칙을 추가했다.
- P0 센서, UI와 락온 범위를 새 계약에 맞춰 갱신했다.

### v0.2

- 저장 위치를 `Document/Plan/CombatPlan` 바로 아래의 단일 문서 파일로 정정했다.
- 파일명을 `09_락온_센서_전자전.md`로 정리했다.
- Windows 경로에서 `/`가 경로 구분자로 해석되는 문제를 피하기 위해 문서 제목과 파일명을 분리했다.

### v0.1

- 락온을 자동 명중이 아닌 전투 압박 상태로 정의했다.
- 센서, 락온, 전자전의 역할을 분리했다.
- 배터리를 총기 발사용 마나가 아니라 전장 전자장비 전력 자원으로 정의했다.
- P0 최소 구현 범위를 `기본 센서`, `락온 게이지`, `배터리 소모`, `엄폐`, `횡방향 속도`, `연막 또는 플레어`로 제한했다.
- EMP / 재밍 / 센서 손상 / 배터리 계통 손상 / 약점 스캔은 P1 이후로 분리했다.

---

## 21. Migration

- v0.4.0부터 `TargetSelect`, `Vehicle Target Lock`, `Target Scan`, `Active Detection Pulse`는 서로 다른 Gameplay State/Operation이다. 하나의 명령이 다른 상태를 암묵적으로 시작하는 기존 결합은 새 구현에서 제거한다.
- v0.4.0부터 Target Lock의 Authority는 개별 Weapon/Equipment가 아니라 차량 공용 Targeting Runtime이다. 장비별 Lock 방식은 공용 Locked Target을 사용할 수 있는 조건 또는 후속 Seeker 정책으로 표현한다.
- v0.4.2부터 현재 VehicleData PrimaryAssetId 기반 TargetId를 Persistent Knowledge의 실제 개체 Identity로 사용하지 않는다. Contact lifetime 밖 Knowledge 재연결은 안정 `TargetEntityId`가 있을 때만 허용한다.
- v0.4.2부터 P0 Lock Requirement는 `NoLockRequired / VehicleLockRequired` exact2다. `VehicleLockPreferred`는 구체 Consumer 정책이 생기기 전에는 사용하지 않는다.
- v0.4.2부터 Lock 붕괴는 지속 Broken State가 아니라 Break Transition Event이며, Target Homing Missile의 발사 후 목표 ownership은 Vehicle Lock이 아니라 Missile Seeker가 소유한다.
- v0.4.3 Current 구현에서는 `StartTargetedScan()`이 `bActiveScanRunning`을 켜지 않는다. broad Active Detection과 Target Scan은 독립 Operation으로 동시 실행 가능하며 각 만료가 상대 Operation을 자동 종료하지 않는다. 둘을 함께 수행해야 하는 장비는 상위 Policy가 두 Operation을 명시적으로 요청한다.
- v0.4.4 / Phase 2부터 `FCFSensorContact.AnalysisProgress01`은 제거됐다. Scan 진행은 `Snapshot.ScanAttempt.Progress01`이 소유하고, Knowledge는 `InformationLevel / AnalysisCompletionRevision`이 소유한다.
- v0.4.0 설계 반영만으로 기존 Systems 문서나 실제 Source가 자동 변경된 것으로 간주하지 않는다. 구현과 Technical Validation이 끝난 뒤 Systems를 Current 계약으로 갱신한다.
- v0.3.2부터 `DA_VehicleSensor_Basic.AnalysisGainPerSec`는 0.25/s가 아니라 0.40/s다. 0.25/s는 3초 Scan으로 DetailedScan 완료가 불가능한 폐기 설정이다.
- v0.3.2부터 Tactical Analysis가 활성인 Sensor tuning은 개별 숫자 범위뿐 아니라 단발 Scan 완료 cross-field invariant까지 만족해야 한다. multi-pass가 필요하면 별도 모드/계약을 먼저 정의한다.
- v0.3.1의 `DA_VehicleSensor_Basic` 수치는 최종 밸런스 확정이 아니라 신규 정상 차량이 scan-capable 상태로 출고되기 위한 P0 기술·플레이테스트 baseline이다. 이후 조정은 DataAsset 값으로 수행하며 Scanner/Vehicle Basic/Fallback source priority를 바꾸지 않는다.
- 기존 차량의 `DefaultSensorData=None`은 자동으로 Basic Sensor를 지급하지 않고 explicit Sensorless 호환 상태로 보존한다.
- 기존 하위 폴더형 저장 방식은 사용하지 않는다.
- 전투 기획 항목 문서는 `Document/Plan/CombatPlan` 바로 아래에 번호가 붙은 단일 Markdown 파일로 둔다.
- 기존 기획 문서에서 `열/에너지`로 표현된 전투 자원은 이 문서 기준으로 `배터리/연료` 체계와 분리한다.
- 배터리는 센서, 락온, 방어장비, 전자전에 사용한다.
- 무기 과열은 차량 자원이 아니라 무기별 자원으로 유지한다.
- 락온 무기는 반드시 대응 수단과 함께 설계한다.
- P0 구현 시 락온 미사일을 넣는다면 연막 또는 플레어 1종을 함께 넣는다.
