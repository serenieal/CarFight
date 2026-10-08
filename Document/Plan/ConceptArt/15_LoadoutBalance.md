# CarFight Concept Art - Loadout Balance

문서 버전: v0.3  
작성일: 2026-07-06  
문서 위치: `Document/Plan/ConceptArt/15_LoadoutBalance.md`  
상위 기준 문서: `Document/Plan/ConceptArt/00_ArtDirection.md`  
관련 문서: `Document/Plan/ConceptArt/04_VehicleClasses.md`, `Document/Plan/ConceptArt/08_HardpointRules.md`, `Document/ProjectSSOT/CombatPlan/12_Hardpoints.md`, `Document/Plan/VehiclePlatformPlan/CF_HardpointMatrix.md`, `Document/Plan/VehiclePlatformPlan/CF_HardpointAuthoring.md`, `Document/Plan/TurretPlan/CF_TurretPlan_v0_1.md`  
관련 범위: 차급별 재미 방향, 허용 장비, 하드포인트 수위, AI 원화/AI 모델링용 장착 기준

---

## 1. 문서 목적

이 문서는 CarFight 차량 차급별로 **어떤 장비를 장착할 수 있어야 재미가 살아나는지**를 정리한다.

이 문서는 최종 전투 밸런스 문서가 아니다.  
이 문서는 공격력, 내구도, 속도, 재장전 시간 같은 수치 밸런스를 정하지 않는다.

이 문서의 목적은 다음과 같다.

- 차급별 장착 가능 장비의 기본 방향을 정한다.
- 작은 차, 중간 차, 큰 차의 재미가 서로 겹치지 않게 한다.
- AI 원화와 AI 모델링 결과가 과무장 차량, 군용 장갑차, APC, 탱크처럼 흐르지 않게 한다.
- 하드포인트가 차량 실루엣보다 먼저 읽히는 결과를 줄인다.
- 1인 개발자가 한 번에 C1 Simple Game Asset 후보로 검수할 수 있는 장비 구성을 정한다.

핵심 문장:

```text
차량의 재미는 장비를 많이 다는 것이 아니라, 차급별 강점과 약점이 멀리서도 읽히는 것이다.
```

---

## 2. 현재 확정 기준

이 문서는 아래 기준을 따른다.

| 항목 | 기준 |
|---|---|
| 기본 스타일 | Simplified Near-Future Civilian Combat Car |
| 기본 제작 목표 | C1 Simple Game Asset 최종 후보 |
| 기본 카메라 기준 | R1 Gameplay Mid |
| 기본 차급 | Microcar, City Car, Subcompact, Compact, Sedan, Coupe, Wagon, SUV, Van, Pickup |
| 기본 하드포인트 원화 기준 | 루프 중앙 1개 |
| P0 구현 검증축 | `Front_01 + Fixed`, `Top_01 + Turret` |
| 확장 후보 | `Front_02 + Gimbal`, `Top_02` 또는 `Back_01 + Launcher`, `UtilitySlot_01` |
| 장비 디자인 기준 | 차량 실루엣을 압도하지 않는 단순한 부착형 장비 |
| AI 모델링 전제 | 생성 결과를 혼자 검수하고 제한적으로 수정할 수 있어야 함 |

이 문서는 `08_HardpointRules.md`의 C1 기준과 충돌하지 않는다.  
따라서 C1 기본 원화는 여전히 **루프 중앙 하드포인트 1개**를 기준으로 검수한다.

다만 구현 검증 문서에서 이미 `Front_01 + Fixed`와 `Top_01 + Turret`이 P0 검증축으로 존재하므로, 이 문서는 원화/모델링 단계에서 어떤 차급이 어떤 장비를 허용할지 정리한다.

---

## 3. 장비 분류

CarFight의 차급별 장비 판단은 아래 단순 분류를 사용한다.

| 장비 분류 | 연결 하드포인트 | 한국어 표시 | 기본 재미 |
|---|---|---|---|
| Fixed Gun | `Front_01 + Fixed` | 전방 고정포 | 운전 방향과 사격 방향이 연결되는 재미 |
| Light Gimbal | `Front_02 + Gimbal` | 소형 짐벌 | 빠른 근거리 대응, 경량 차량 압박 |
| Roof Turret | `Top_01 + Turret` | 루프 터렛 | 차체 방향과 무기 방향이 분리되는 재미 |
| Light Launcher | `Top_02` 또는 `Back_01 + Launcher` | 경량 런처 | 락온, 회피, 대응 장비의 재미 |
| Utility | `UtilitySlot_01` | 유틸리티 장비 | 연막, 플레어, 센서, 냉각, 수리 보조 |

주의:

- 이 분류는 장비의 모양과 장착 허용 범위를 정하기 위한 것이다.
- 실제 무기 성능 수치, 탄종, 쿨다운, 피해량은 이 문서에서 정하지 않는다.
- 한 차량에 많은 장비를 달아 강하게 만드는 방향은 기본값이 아니다.

---

## 4. 차급별 재미 밸런스 원칙

차급별 재미는 아래 방향으로 나눈다.

| 차급 묶음 | 재미 방향 | 기본 약점 | 장비 방향 |
|---|---|---|---|
| 작은 차 | 회피, 빠른 위치 변경, 성가신 압박 | 낮은 장비 수용량, 낮은 존재감 | 가벼운 전방 장비와 유틸리티 |
| 중간 차 | 가장 이해하기 쉬운 기준형 전투 | 뚜렷한 극단성이 적음 | 전방 고정포와 작은/중간 루프 장비 |
| 낮은 차 | 빠른 진입, 정면 조준, 스킬샷 | 루프 장비가 실루엣을 망치기 쉬움 | 전방 고정포, 낮은 보조 장비 |
| 긴 차 | 안정감, 보조 장비, 팀 지원 느낌 | 차체가 길어 피격 면적이 큼 | 유틸리티, 센서, 제한적 런처 |
| 큰 차 | 튼튼함, 장비 운반, 압박감 | 큰 표적, 둔해 보이는 실루엣 | 루프 터렛, 유틸리티, 제한적 런처 |

기준 문장:

```text
작은 차는 많이 달 수 없어서 재미있고, 큰 차는 많이 달 수 있지만 둔하고 크게 보여야 재미있다.
```

---

## 5. 장비 허용 등급

이 문서의 표기는 아래 의미로 사용한다.

| 표기 | 의미 |
|---|---|
| 권장 | 해당 차급의 기본 재미와 잘 맞음 |
| 조건부 | 변주로 허용 가능하지만 C1 기본 후보에서는 신중히 검수 |
| 비권장 | 차급 실루엣 또는 제작 난이도를 해치기 쉬움 |
| 금지 | CarFight 방향과 충돌하거나 군용 차량처럼 보일 위험이 큼 |

조건부 장비는 아래 조건을 통과해야 한다.

- R1 Gameplay Mid 거리에서 차급 실루엣이 먼저 읽힌다.
- 하드포인트가 차량보다 먼저 읽히지 않는다.
- AI 모델링 결과에서 작은 부품 덩어리로 깨지지 않는다.
- 한 번의 생성 결과를 긴 수작업 리파인 없이 C1 후보로 검수할 수 있다.

---

## 6. 공통 차량 본체 기본 세팅

공통 차량 본체 기본 세팅은 `15_LoadoutBalance_SUVHoodMountConcept_v3.png`에서 확인한 제작 방향을 모든 차급에 적용하기 위한 기준이다.

이 기준은 SUV 전용 규칙이 아니다.  
차급별 실루엣과 비례는 다르게 유지하되, **무기 미설치 본체, 빈 장착부, 낮은 모델링 복잡도, 분리 가능한 파츠 구조**는 공통으로 적용한다.

| 항목 | 공통 기본값 |
|---|---|
| 기준 사례 이미지 | `Document/Plan/ConceptArt/15_LoadoutBalance_SUVHoodMountConcept_v3.png` |
| 제작 목표 | C1 Simple Game Asset 후보 |
| 장비 상태 | 기본 본체에는 무기/터렛/런처 미설치 |
| 하드포인트 표현 | 실제 장비가 아니라 빈 기계식 장착 인터페이스로 표현 |
| `Front_01` 기본 방향 | 전방 범퍼보다 본네트/전방 상판 위 빈 마운트를 우선 검토 |
| `Top_01` 기본 방향 | 루프 중앙 빈 회전 링/베이스. 실제 터렛 없음 |
| `Top_02` 기본 방향 | 후방 루프 또는 후방 상판의 작은 유틸리티/경량 런처 후보 패드 |
| 전방 범퍼 | 보강 범퍼, 견인 고리, 충돌 방지판 중심. 무기 소켓 기본 제외 |
| 측면 장비 | C1 기본 본체에서는 제외 |
| 색상/재질 | 무광 다크 그레이 차체, 저채도 블루그레이 장갑판, 검은 타이어, 어두운 유리, 주황색 소형 래치 포인트 |
| 작은 디테일 | 볼트, 얇은 홈, 경고 마킹, 작은 패널 라인은 텍스처/노멀 우선 |
| 파츠 구조 | 차체, 휠, 유리, 장갑판, 빈 하드포인트 마운트를 분리 가능하게 유지 |
| 폴리 목표 | 일반 차량 본체 LOD0 기준 20k~30k tris |
| 큰 차급 폴리 목표 | SUV/Van/Pickup 본체 LOD0 기준 25k~30k tris |
| 폴리 상한 | 기본 본체 LOD0 기준 40k tris |
| Reject 기준 | 60k tris 이상이거나 작은 디테일이 메쉬로 과생성된 결과 |

공통 판단 문장:

```text
CarFight 기본 차량 본체는 장비가 설치된 완성 무장 차량이 아니라, 장비를 얹을 빈 하드포인트가 준비된 민간 차량 개조 차체다.
```

공통 AI 원화/AI 모델링 프롬프트 기준:

```text
no installed weapon,
no installed turret,
no installed launcher,
empty mechanical hardpoint mounts only,
hood or front upper body empty Front_01 mount,
empty Top_01 roof center turret ring when the class allows roof mounting,
small empty Top_02 rear utility pad only when the class allows it,
reinforced bumper without weapon socket,
large readable wheels,
large simple armor panels,
low modeling complexity,
target around 20k to 30k triangles for LOD0,
small bolts and seams as texture or normal details,
no APC,
no tank,
no military armored vehicle
```

---

## 7. 차급별 장비 매트릭스

| 차급 | 전방 고정포 | 소형 짐벌 | 루프 터렛 | 경량 런처 | 유틸리티 |
|---|---|---|---|---|---|
| Microcar | 조건부 | 비권장 | 금지 | 금지 | 권장 |
| City Car | 권장 | 조건부 | 비권장 | 금지 | 권장 |
| Subcompact | 권장 | 조건부 | 조건부 | 비권장 | 권장 |
| Compact | 권장 | 조건부 | 권장 | 조건부 | 권장 |
| Sedan | 권장 | 조건부 | 조건부 | 비권장 | 권장 |
| Coupe | 권장 | 조건부 | 비권장 | 금지 | 조건부 |
| Wagon | 권장 | 조건부 | 조건부 | 조건부 | 권장 |
| SUV | 권장 | 조건부 | 권장 | 조건부 | 권장 |
| Van | 조건부 | 비권장 | 조건부 | 조건부 | 권장 |
| Pickup | 권장 | 조건부 | 권장 | 조건부 | 권장 |

매트릭스 해석:

- `권장`은 해당 차급의 기본 재미를 해치지 않는 장비다.
- `조건부`는 변주나 확장 후보로 쓸 수 있지만, AI 원화와 AI 모델링 검수가 필요하다.
- `비권장`은 초기 기본 차량에는 넣지 않는다.
- `금지`는 차급 정체성 또는 CarFight 아트 방향과 충돌하므로 기본 문서 기준에서 제외한다.

---

## 8. 차급별 세부 기준

### 8.1 Microcar

| 항목 | 기준 |
|---|---|
| 한국어 표시명 | 초소형차 |
| 재미 방향 | 작고 빠르게 피하며 성가시게 압박하는 차량 |
| 기본 장비 | 유틸리티 중심 |
| 허용 장비 | 아주 작은 전방 고정포, 연막/플레어 |
| 피해야 할 장비 | 루프 터렛, 런처, 대형 장갑, 큰 휠 아치 |
| 가독성 기준 | 작은 차체, 짧은 길이, 낮은 장비량이 먼저 읽혀야 함 |
| AI 모델링 주의 | 무기가 차체보다 커지면 즉시 실패로 본다 |

Microcar는 무장을 많이 달 수 없어서 재미있는 차급이다.  
이 차급에 강한 루프 무장을 허용하면 초소형차의 장점과 약점이 모두 사라진다.

---

### 8.2 City Car

| 항목 | 기준 |
|---|---|
| 한국어 표시명 | 도심형 경차 |
| 재미 방향 | 짧은 차체로 빠르게 꺾고, 좁은 공간에서 살아남는 차량 |
| 기본 장비 | 전방 고정포 또는 유틸리티 |
| 허용 장비 | 소형 전방 고정포, 소형 짐벌, 연막/플레어 |
| 피해야 할 장비 | 중형 루프 터렛, 런처, 복잡한 외부 장비 랙 |
| 가독성 기준 | 짧고 둥근 민간 경차 실루엣이 유지되어야 함 |
| AI 모델링 주의 | 장비를 크게 붙이면 SUV처럼 보이기 쉽다 |

City Car는 민간 차량성이 강해야 한다.  
전투 개조는 차체 위에 덧붙인 작은 장비 수준으로 제한한다.

---

### 8.3 Subcompact

| 항목 | 기준 |
|---|---|
| 한국어 표시명 | 소형차 |
| 재미 방향 | 작지만 가장 다루기 쉬운 기본형 공격 차량 |
| 기본 장비 | 전방 고정포 |
| 허용 장비 | 소형 짐벌, 소형 루프 장비, 유틸리티 |
| 피해야 할 장비 | 중형 이상 루프 터렛, 런처 기본 장착, 과한 장갑판 |
| 가독성 기준 | 작은 해치백 차체와 단순한 장비 1개가 읽혀야 함 |
| AI 모델링 주의 | 루프 장비를 넣더라도 H0~H1 수준으로 제한한다 |

Subcompact는 초반 반복 제작에 적합한 차급이다.  
장비 구성도 단순해야 AI 모델링 결과를 빠르게 검수할 수 있다.

---

### 8.4 Compact

| 항목 | 기준 |
|---|---|
| 한국어 표시명 | 준중형차 |
| 재미 방향 | CarFight의 가장 표준적인 장비 밸런스 기준 |
| 기본 장비 | 전방 고정포, 작은/중간 루프 터렛 |
| 허용 장비 | 소형 짐벌, 유틸리티, 조건부 경량 런처 |
| 피해야 할 장비 | 대형 루프 터렛, 터렛과 런처 동시 과장, 복잡한 측면 장비 |
| 가독성 기준 | 균형 잡힌 차체 위에 장비 1~2개가 정리되어 보여야 함 |
| AI 모델링 주의 | 기본 프롬프트 기준 차급으로 쓰기 좋으므로 과한 변주를 피한다 |

Compact는 장비 규칙의 기준점이다.  
다른 차급은 Compact보다 더 가볍거나, 더 낮거나, 더 길거나, 더 큰 방향으로 차이를 만든다.

---

### 8.5 Sedan

| 항목 | 기준 |
|---|---|
| 한국어 표시명 | 세단 |
| 재미 방향 | 낮고 안정적인 정면 조준, 운전 방향과 사격 방향을 맞추는 차량 |
| 기본 장비 | 전방 고정포 |
| 허용 장비 | 낮은 루프 터렛, 소형 짐벌, 센서/유틸리티 |
| 피해야 할 장비 | 높은 루프 터렛, 상부 런처 묶음, 리무진처럼 긴 차체 |
| 가독성 기준 | 보닛-캐빈-트렁크 3박스가 장비보다 먼저 읽혀야 함 |
| AI 모델링 주의 | 루프 장비가 높아지면 세단의 낮은 실루엣이 깨진다 |

Sedan은 큰 무기를 올려 강하게 보이는 차급이 아니다.  
낮은 자세와 정면 조준의 안정감이 재미의 중심이다.

---

### 8.6 Coupe

| 항목 | 기준 |
|---|---|
| 한국어 표시명 | 쿠페 |
| 재미 방향 | 빠르게 들어가 정면으로 찌르는 낮은 차량 |
| 기본 장비 | 전방 고정포 |
| 허용 장비 | 작은 짐벌, 작은 센서 |
| 피해야 할 장비 | 루프 터렛, 런처, 큰 장갑 박스, 높은 장비 플랫폼 |
| 가독성 기준 | 낮은 루프 라인과 짧은 후방이 유지되어야 함 |
| AI 모델링 주의 | 멋있는 스포츠카 디테일보다 단순한 낮은 실루엣을 우선한다 |

Coupe는 장비가 적어야 차급 재미가 산다.  
루프 위에 큰 장비를 올리면 Coupe가 아니라 낮은 장갑차처럼 보일 수 있다.

---

### 8.7 Wagon

| 항목 | 기준 |
|---|---|
| 한국어 표시명 | 왜건 |
| 재미 방향 | 긴 루프와 적재 공간을 활용한 보조/지원형 차량 |
| 기본 장비 | 전방 고정포 또는 유틸리티 |
| 허용 장비 | 센서, 연막/플레어, 소형 루프 장비, 조건부 경량 런처 |
| 피해야 할 장비 | 대형 루프 터렛, 밴처럼 높은 박스형 차체, 군용 수송차 느낌 |
| 가독성 기준 | 긴 루프와 후방 적재 공간이 읽혀야 함 |
| AI 모델링 주의 | 장비 박스를 많이 붙이면 캠핑카나 군용 수송차처럼 흐른다 |

Wagon은 장비를 많이 싣는 느낌을 낼 수 있지만, 실제 하드포인트 수를 늘리는 차급은 아니다.  
작은 센서/유틸리티 장비를 큰 덩어리 몇 개로 정리한다.

---

### 8.8 SUV

| 항목 | 기준 |
|---|---|
| 한국어 표시명 | SUV |
| 재미 방향 | 큰 차체와 높은 자세로 압박하는 다목적 전투 개조 차량 |
| 기본 장비 | 무기 미설치. 본네트 전방 마운트, 루프 중앙 마운트, 후방 루프 패드 준비 |
| 허용 장비 | 중형 루프 터렛, 유틸리티, 조건부 경량 런처 |
| 피해야 할 장비 | APC 형태, 군용 장갑차 형태, 과한 장비 플랫폼, 다중 터렛 |
| 가독성 기준 | 높은 민간 SUV 차체와 큰 휠이 먼저 읽혀야 함 |
| AI 모델링 주의 | 장갑판과 무기가 많아지면 바로 군용 차량처럼 보인다 |

SUV는 CarFight 방향과 잘 맞지만 과무장 위험이 가장 큰 차급이다.  
`Top_02` 같은 확장 슬롯은 두 번째 터렛이 아니라 유틸리티나 경량 런처 후보로 제한한다.

현재 P0 후보인 `DA_TestSUV`는 `Front_01`, `Top_01`, `Top_02`를 가진 비교 차량으로 볼 수 있다.  
다만 ConceptArt 기준에서는 `Top_02`를 기본 강제 장비로 보지 않는다.

---

### 8.8.1 공통 기본 세팅의 SUV 적용 예

SUV는 공통 차량 본체 기본 세팅을 적용한 첫 기준 사례다.  
`15_LoadoutBalance_SUVHoodMountConcept_v3.png`는 SUV 전용 규칙이 아니라 공통 세팅을 SUV 차급에 적용한 예시 이미지로 본다.

| 항목 | 기본값 |
|---|---|
| 기준 이미지 | `Document/Plan/ConceptArt/15_LoadoutBalance_SUVHoodMountConcept_v3.png` |
| 제작 목표 | C1 Simple Game Asset 후보 |
| 장비 상태 | 무기/터렛/런처 미설치 |
| 차체 | 높은 민간 SUV 차체, 큰 휠, 높은 지상고, 굵은 캐빈 |
| `Front_01` 위치 | 전방 범퍼가 아니라 본네트 위 |
| `Front_01` 형태 | 낮은 빈 기계식 마운트. 원형/팔각형 베이스, 볼트 패턴, 클램프, 케이블 포트 |
| `Top_01` 위치 | 루프 중앙 |
| `Top_01` 형태 | 빈 터렛 장착용 회전 링/베이스. 실제 터렛 없음 |
| `Top_02` 위치 | 후방 루프 |
| `Top_02` 형태 | 작은 유틸리티/경량 런처 후보용 빈 장착 패드 |
| 전방 범퍼 | 보강 범퍼와 견인 고리만 허용. 무기 소켓 기본 제외 |
| 측면 장비 | 기본 제외 |
| 색상/재질 | 무광 다크 그레이 차체, 저채도 블루그레이 장갑판, 검은 타이어, 어두운 유리, 주황색 소형 래치 포인트 |
| 폴리 목표 | 큰 차급 공통 기준에 따라 LOD0 기준 25k~30k tris |
| 폴리 상한 | 공통 기준에 따라 LOD0 기준 40k tris |
| Reject 기준 | 공통 기준에 따라 60k tris 이상이거나 작은 디테일이 메쉬로 과생성된 결과 |

SUV 적용 판단 문장:

```text
SUV 기본 본체는 공통 차량 본체 세팅을 높은 민간 SUV 실루엣에 적용한 사례다.
```

AI 원화/AI 모델링 프롬프트에서 SUV 기본 본체를 만들 때는 아래 기준을 우선한다.

```text
tall civilian SUV body,
no installed weapon,
no installed turret,
hood-mounted empty Front_01 mechanical mount,
empty Top_01 roof center turret ring,
small empty Top_02 rear roof utility pad,
reinforced bumper without weapon socket,
large readable wheels,
large simple armor panels,
low modeling complexity,
target around 25k to 30k triangles for LOD0,
small bolts and seams as texture or normal details,
no APC,
no tank,
no military armored vehicle
```

---

### 8.9 Van

| 항목 | 기준 |
|---|---|
| 한국어 표시명 | 밴 |
| 재미 방향 | 높은 박스형 차체로 장비를 싣는 지원형 차량 |
| 기본 장비 | 유틸리티 |
| 허용 장비 | 센서, 연막/플레어, 소형 루프 터렛, 조건부 후방/상부 런처 |
| 피해야 할 장비 | 군용 병력수송차, APC, 대형 루프포, 복잡한 외부 랙 |
| 가독성 기준 | 높은 박스형 민간 밴 실루엣이 먼저 읽혀야 함 |
| AI 모델링 주의 | 평평한 큰 면은 유지하되 작은 문/손잡이/경첩 디테일은 줄인다 |

Van은 장비를 많이 달 수 있어 보이지만, 초기 C1 후보에서는 유틸리티 중심이 안전하다.  
공격 장비를 크게 올리면 군용 수송차처럼 보이기 쉽다.

---

### 8.10 Pickup

| 항목 | 기준 |
|---|---|
| 한국어 표시명 | 픽업 |
| 재미 방향 | 짐칸을 활용한 유연한 장비 운반 차량 |
| 기본 장비 | 전방 고정포, 짐칸/상부 장비 1개 |
| 허용 장비 | 루프/짐칸 터렛, 유틸리티 박스, 조건부 경량 런처 |
| 피해야 할 장비 | 군용 테크니컬 트럭, 다중 무장 플랫폼, 노출된 거대 포대 |
| 가독성 기준 | 캐빈과 뒤쪽 짐칸 분리가 먼저 읽혀야 함 |
| AI 모델링 주의 | 짐칸 장비는 큰 박스 또는 단순 마운트 1개로 제한한다 |

Pickup은 무장과 잘 어울리지만 가장 쉽게 군용 테크니컬 트럭처럼 보인다.  
장비는 짐칸의 정체성을 보여주는 정도로 제한하고, 차량보다 무기가 먼저 읽히지 않게 한다.

---

## 9. 차급별 기본 추천 장비

초기 AI 원화와 AI 모델링 후보를 만들 때는 아래 조합을 우선한다.

| 차급 | 추천 기본 조합 | 목적 |
|---|---|---|
| Microcar | Utility only 또는 아주 작은 Front Fixed | 초소형 회피 차량 |
| City Car | Front Fixed + Utility | 짧은 경량 전투 개조 |
| Subcompact | Front Fixed + Utility | 가장 쉬운 소형 기본형 |
| Compact | Front Fixed + Top Turret | 표준 전투 차량 기준 |
| Sedan | Front Fixed + Low Top Turret | 낮은 정면 조준 차량 |
| Coupe | Front Fixed only | 빠른 정면 공격 차량 |
| Wagon | Front Fixed + Utility/Sensor | 지원형 긴 차체 |
| SUV | Hood Front Mount + Empty Top Mount + Rear Utility Pad | 다목적 중형 압박 차량 본체 |
| Van | Utility/Sensor + optional Front Fixed | 장비 운반 지원 차량 |
| Pickup | Front Fixed + Bed/Top Utility or Turret | 짐칸 활용 차량 |

초기 제작 추천 순서:

```text
Compact → SUV → Subcompact → Pickup → Van → Sedan → City Car → Wagon → Coupe → Microcar
```

이 순서는 장비 밸런스 검증과 AI 모델링 난이도를 함께 고려한 순서다.  
이미 Compact와 SUV 원화 기준을 다룬 흐름이 있으므로 두 차급을 먼저 검증하는 것이 좋다.

---

## 10. AI 원화 프롬프트 영향

장비 프롬프트는 아래 순서로 넣는다.

1. 차급을 먼저 지정한다.
2. 차급 실루엣을 지정한다.
3. 허용 장비를 1~2개만 지정한다.
4. 금지 장비를 명확히 제외한다.
5. 하드포인트가 차량보다 먼저 보이지 않도록 제한한다.

기본 장비 프롬프트 구조:

```text
[vehicle class],
[class silhouette],
simplified near-future civilian combat car,
[allowed equipment],
equipment is simple and secondary to the vehicle silhouette,
large readable forms,
low modeling complexity,
no military APC,
no tank,
no mech,
no oversized weapon platform
```

SUV 예시:

```text
tall civilian SUV body,
simplified near-future civilian combat car,
front fixed weapon mount and one simple roof turret,
optional small utility box on rear roof,
equipment is secondary to the SUV silhouette,
large readable forms,
low modeling complexity,
no APC,
no military armored vehicle,
no tank,
no oversized turret,
no multiple weapon platforms
```

---

## 11. AI 모델링 검수 영향

AI 모델링 결과는 아래 순서로 본다.

| 순서 | 체크 | 실패 기준 |
|---|---|---|
| 1 | 차급 실루엣 | 장비 때문에 차급이 안 읽힘 |
| 2 | 장비 개수 | 기본 후보인데 장비가 3개 이상으로 보임 |
| 3 | 하드포인트 위치 | 장비가 차체 중심선에서 어색하게 벗어남 |
| 4 | 파츠 단순성 | 작은 기계 부품이 과하게 많음 |
| 5 | 군용화 위험 | APC, 탱크, 군용 테크니컬처럼 보임 |
| 6 | 수정 가능성 | 혼자 분리/정리하기 어려운 복잡한 결과 |

Reject 기준:

```text
차급보다 무기가 먼저 읽히면 Reject다.
```

Fix 기준:

```text
차급과 큰 장비 위치는 맞지만 장식 디테일만 많은 경우에는 제한적 후처리 후보로 본다.
```

Pass 기준:

```text
차급, 하드포인트, 장비 수, 큰 형태가 R1 Gameplay Mid 거리에서 바로 읽히면 Pass다.
```

---

## 12. 금지 방향

아래 방향은 차급별 장비 밸런스에서 제외한다.

- Microcar에 대형 루프 터렛을 올리는 방향
- City Car를 장갑 SUV처럼 키우는 방향
- Coupe에 높은 루프 무장 플랫폼을 올리는 방향
- Sedan을 다중 런처 차량처럼 만드는 방향
- Wagon과 Van을 군용 수송차처럼 만드는 방향
- SUV를 APC나 군용 장갑차처럼 만드는 방향
- Pickup을 군용 테크니컬 트럭처럼 만드는 방향
- 모든 차급에 동일한 터렛과 동일한 런처를 반복 장착하는 방향
- 측면 하드포인트를 C1 기본값으로 넣는 방향
- 차량보다 무기가 먼저 보이는 방향

---

## 13. 현재 결정 상태

| 항목 | 결정 |
|---|---|
| 차급별 장비 기준 | 차급 재미를 기준으로 허용/조건부/비권장/금지로 분리 |
| 기본 장비 수 | C1 기본 후보는 1개 중심, 확장 후보는 2개까지 신중히 검토 |
| P0 구현 연계 | `Front_01 + Fixed`, `Top_01 + Turret`을 핵심 검증축으로 유지 |
| 확장 장비 | Gimbal, Launcher, Utility는 차급별 조건부 또는 권장으로 제한 |
| 작은 차 방향 | 회피와 유틸리티 중심, 중/대형 무장 금지 |
| 중간 차 방향 | CarFight 기본 전투 장비 검증 |
| 큰 차 방향 | 장비 수용 가능하지만 군용화 금지 |
| 공통 본체 방향 | 무기 미설치 본체, 빈 하드포인트, 분리 가능한 파츠 구조 |
| AI 모델링 기준 | 차급보다 장비가 먼저 읽히면 실패 |

---

## 14. Changelog

### v0.3

- `15_LoadoutBalance_SUVHoodMountConcept_v3.png`에서 확인한 세팅을 SUV 전용이 아니라 공통 차량 본체 기본 세팅으로 재정의했다.
- 공통 차량 본체 기준에 무기/터렛/런처 미설치, 빈 기계식 하드포인트, 분리 가능한 파츠 구조, 폴리 목표를 추가했다.
- SUV 세팅 섹션을 공통 기본 세팅의 SUV 적용 예로 낮췄다.
- 섹션 번호를 공통 세팅 추가에 맞게 정리했다.

### v0.2

- SUV 기준 이미지인 `15_LoadoutBalance_SUVHoodMountConcept_v3.png`를 문서에 반영했다.
- SUV 기본 상태를 무기/터렛/런처 미설치 본체로 명확히 했다.
- `Front_01` 위치를 전방 범퍼가 아니라 본네트 위 빈 기계식 마운트로 정했다.
- SUV 본체의 `Top_01`, `Top_02`, 색상/재질, 폴리 목표 기준을 추가했다.

### v0.1

- 차급별 재미 밸런스 기준을 신규 작성했다.
- 장비 분류를 Fixed Gun, Light Gimbal, Roof Turret, Light Launcher, Utility로 정리했다.
- 차급별 장비 허용 매트릭스를 추가했다.
- Microcar부터 Pickup까지 차급별 권장/조건부/비권장/금지 장비 기준을 작성했다.
- AI 원화 프롬프트와 AI 모델링 검수 기준에 장비 밸런스 항목을 연결했다.

---

## 15. Migration

### v0.3 적용 시 기존 작업 영향

- 이후 모든 차량 기본 본체 원화와 AI 모델링은 공통 차량 본체 기본 세팅을 먼저 확인한다.
- SUV v3 이미지는 SUV 전용 고정 규칙이 아니라 공통 본체 세팅을 검증한 첫 사례로 본다.
- 차급별 차이는 실루엣, 비례, 허용 장비 수위에서 만들고, 본체 제작 방식은 공통 세팅을 따른다.
- 실제 무기, 터렛, 런처는 기본 본체에 설치하지 않고 별도 에셋으로 제작한다.

### v0.2 적용 시 기존 작업 영향

- SUV 기본 본체 원화와 AI 모델링은 `15_LoadoutBalance_SUVHoodMountConcept_v3.png`의 세팅을 참고한다.
- SUV 본체에는 터렛, 포신, 런처를 설치하지 않고 빈 장착부만 만든다.
- 기존 범퍼 전방 무기 소켓 방식은 SUV 기본값에서 제외한다.
- SUV의 `Front_01`은 본네트 위 빈 기계식 마운트로 해석한다.
- 실제 터렛과 무기는 별도 에셋으로 제작해 `Top_01` 또는 다른 허용 슬롯에 장착한다.

### v0.1 적용 시 기존 작업 영향

이 문서 작성 이후에는 다음 기준을 적용한다.

- 차급별 원화 프롬프트를 만들 때 장비를 무조건 많이 추가하지 않는다.
- `04_VehicleClasses.md`의 차급 실루엣을 먼저 적용한 뒤, 이 문서의 장비 허용 기준을 추가한다.
- `08_HardpointRules.md`의 C1 기본 하드포인트 1개 기준은 유지한다.
- 구현 단계의 `Front_01`, `Top_01`, `Top_02` 같은 슬롯 이름은 `CF_HardpointMatrix.md`와 충돌하지 않게 사용한다.
- `Top_02`와 `Back_01`은 기본 강제 장비가 아니라 확장 후보로 본다.
- AI 모델링 결과에서 장비가 차급 실루엣을 압도하면 오래 고치지 않고 Reject로 돌린다.
