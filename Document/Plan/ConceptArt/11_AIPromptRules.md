# CarFight Concept Art - AI Prompt Rules

문서 버전: v0.3  
작성일: 2026-07-06  
문서 위치: `Document/Plan/ConceptArt/11_AIPromptRules.md`  
상위 기준 문서: `Document/Plan/ConceptArt/00_ArtDirection.md`  
관련 문서: `Document/Plan/ConceptArt/01_DecisionChecklist.md`, `Document/Plan/ConceptArt/02_ComplexityBudget.md`, `Document/Plan/ConceptArt/03_CameraReadability.md`, `Document/Plan/ConceptArt/04_VehicleClasses.md`, `Document/Plan/ConceptArt/05_SilhouetteRules.md`, `Document/Plan/ConceptArt/06_ProportionRules.md`, `Document/Plan/ConceptArt/07_ArmorPanelRules.md`, `Document/Plan/ConceptArt/08_HardpointRules.md`, `Document/Plan/ConceptArt/09_MaterialColorRules.md`, `Document/Plan/ConceptArt/10_ConceptSheetFormat.md`  
관련 범위: AI 이미지 생성 프롬프트 구조, 차급별 삽입 문구, 필수/금지 문구, 원화 시트 출력 형식, AI 생성 결과 검수 기준

---

## 1. 문서 목적

이 문서는 CarFight 차량 원화를 AI 이미지 생성으로 만들 때 사용할 **프롬프트 작성 규칙**을 정의한다.

CarFight의 AI 원화는 감상용 이미지가 아니라 C1 Simple Game Asset 최종 후보를 만들기 위한 제작 기준선이다.  
따라서 프롬프트는 멋진 렌더보다 차급 실루엣, 낮은 모델링 난이도, 큰 장갑판, 루프 하드포인트, 정사영 4뷰 시트를 우선해야 한다.

이 문서의 목적은 다음과 같다.

- 앞선 ConceptArt 기준을 AI 프롬프트 구조로 묶는다.
- 차량 차급별로 넣을 실루엣/비례 문구를 정한다.
- 반드시 포함할 문구와 금지할 문구를 분리한다.
- AI 결과가 탱크, APC, 군용 장갑차, 실제 브랜드 차량, 과도한 네온 디자인으로 흐르지 않게 한다.
- 한 번의 생성 결과가 C1 최종 후보에 가까워지도록 제약을 명확히 한다.

---

## 2. 현재 확정 기준

이 문서는 아래 기준을 따른다.

| 항목 | 기준 |
|---|---|
| 기본 스타일 | Simplified Near-Future Civilian Combat Car |
| 기본 제작 목표 | C1 Simple Game Asset 최종 후보 |
| 기본 카메라 기준 | R1 Gameplay Mid |
| 기본 차급 | Microcar, City Car, Subcompact, Compact, Sedan, Coupe, Wagon, SUV, Van, Pickup |
| 기본 시트 형식 | Side, Front, Rear, Top View 정사영 시트 |
| AI 모델링 전제 | 이미지 생성 후 AI 모델링 도구로 3D 후보 생성 |
| 장갑 기준 | 큰 판형 패널 중심 |
| 하드포인트 기준 | 루프 중앙 1개 기본 |
| 색상/재질 기준 | 저채도 산업적 색상, 5~6개 머티리얼 슬롯 권장 |

최상위 기준:

```text
AI 프롬프트는 멋진 자동차 이미지를 요청하는 문장이 아니라, 만들기 쉬운 게임 에셋 후보를 제한하는 제작 지시문이어야 한다.
```

---

## 3. 프롬프트 작성 원칙

CarFight 차량 프롬프트는 다음 순서로 작성한다.

1. 차급을 먼저 지정한다.
2. 차급 실루엣 문구를 넣는다.
3. 차급 비례 문구를 넣는다.
4. CarFight 스타일명을 넣는다.
5. C1 제작성과 낮은 모델링 난이도를 넣는다.
6. 큰 장갑판 규칙을 넣는다.
7. 루프 중앙 하드포인트 규칙을 넣는다.
8. 재질/색상 규칙을 넣는다.
9. 정사영 4뷰 시트 형식을 넣는다.
10. 금지 문구를 넣는다.

중요 기준:

```text
차급과 제작성 문구가 장식 문구보다 먼저 와야 한다.
```

피해야 할 작성 방식:

- 전투 역할명으로 차량을 부르지 않는다.
- 군사용 역할명이나 병기 이름을 차급처럼 쓰지 않는다.
- 실제 브랜드명, 실제 모델명, 상표명을 넣지 않는다.
- `cinematic`, `epic`, `ultra detailed` 같은 감상용 렌더 문구를 기본값으로 쓰지 않는다.
- 내부 구조, 성능, 밸런스, 무기 수치 설명을 넣지 않는다.

---

## 4. 기본 프롬프트 템플릿

AI 이미지 생성 시 아래 구조를 기본으로 사용한다.

```text
[Class Name],
[class silhouette phrase],
[class proportion phrase],
simplified near-future civilian combat car,
C1 simple game asset concept sheet,
clean readable silhouette,
low modeling complexity,
large simple shapes,
large simple armor panels,
single simple roof hardpoint,
small roof turret mount,
clear turret rotation space,
low saturation industrial colors,
matte armor panels,
painted civilian car body,
dark simple glass,
dark rubber tires,
small limited emissive sensor lights,
orthographic side view,
orthographic front view,
orthographic rear view,
orthographic top view,
plain background,
part callouts,
material color callouts
```

사용 기준:

- `[Class Name]`에는 `Microcar`, `City Car` 같은 차급명을 넣는다.
- `[class silhouette phrase]`에는 차급별 옆면 실루엣 문구를 넣는다.
- `[class proportion phrase]`에는 차급별 길이/폭/높이 비례 문구를 넣는다.
- 필요한 경우 `easy to model`, `easy to modify`, `game asset reference sheet`를 추가한다.

AI 모델링 연계가 필요할 때는 아래 문구를 추가한다.

```text
AI 3D modeling ready,
clear separated major parts,
simple mesh-friendly forms,
easy pivot and collision setup
```

사용 기준:

- 이 문구는 원화가 AI 모델링 도구의 입력 자료가 될 때 추가한다.
- 작은 디테일을 늘리기 위한 문구가 아니라 큰 파츠 구분을 강화하기 위한 문구다.

---

## 5. 기본 금지 프롬프트

아래 문구는 AI 결과가 CarFight 기준에서 벗어나지 않도록 함께 사용한다.

```text
no real car brand,
no real car copy,
no tank,
no APC,
no military armored vehicle,
no mech,
no oversized weapon,
no multiple weapon platforms,
no excessive realistic detail,
no complex panel lines,
no tiny mechanical parts,
no exposed suspension,
no exposed engine,
no excessive neon,
no racing livery,
no glossy supercar paint,
no cinematic render,
no action scene,
no dramatic perspective,
no complex background,
no inconsistent views
```

금지 문구의 목적:

- 실제 브랜드 복제 위험을 줄인다.
- 민간 차량 차급 실루엣이 탱크/APC 방향으로 변하는 것을 막는다.
- 작은 기계 디테일과 복잡한 패널 라인이 과해지는 것을 막는다.
- 정사영 4뷰 시트가 감상용 렌더나 액션 장면으로 바뀌는 것을 막는다.

---

## 6. 차급별 삽입 문구

아래 표의 문구를 기본 템플릿의 `[class silhouette phrase]`, `[class proportion phrase]` 자리에 넣는다.

| 차급 | 한국어 표시명 | 실루엣 문구 | 비례 문구 |
|---|---|---|---|
| Microcar | 초소형차 | very short tiny civilian car silhouette | very short narrow body, small cabin |
| City Car | 도심형 경차 | short tall city car silhouette | short body, tall cabin |
| Subcompact | 소형 해치백 | small hatchback silhouette | short hatchback proportions |
| Compact | 컴팩트 해치백 | balanced compact hatchback silhouette | balanced compact proportions |
| Sedan | 세단 | long low three-box sedan silhouette | long low three-box proportions |
| Coupe | 쿠페 | low coupe roofline silhouette | low body, short cabin, sloped roof |
| Wagon | 왜건 | long roof wagon silhouette | long roof, long rear cargo area |
| SUV | SUV | tall civilian SUV silhouette with large wheels | tall wide body, large wheels |
| Van | 밴 | tall boxy civilian van silhouette | long tall boxy body |
| Pickup | 픽업 | cabin and open rear bed silhouette | separated cabin and open rear bed |

주의:

- 차급 이름은 전투 역할이 아니라 민간 차량 형태 기준이다.
- 장갑과 하드포인트를 추가해도 위 실루엣 문구가 먼저 읽혀야 한다.
- AI 결과가 차급 문구보다 군용/병기 문구에 끌리면 해당 결과는 기본 후보에서 제외한다.

---

## 7. 차급별 주의 문구

차급별로 아래 보조 문구를 필요할 때 추가한다.

| 차급 | 추가 주의 |
|---|---|
| Microcar | avoid large turret, avoid oversized wheels |
| City Car | avoid oversized roof weapon, keep small city car body |
| Subcompact | keep simple hatchback rear shape |
| Compact | keep balanced civilian compact body |
| Sedan | keep low roof and three-box body |
| Coupe | keep low roofline, avoid bulky armor |
| Wagon | keep long roof and rear cargo area |
| SUV | keep civilian SUV shape, avoid APC look |
| Van | keep civilian van shape, avoid military transport look |
| Pickup | keep open rear bed, avoid technical truck look |

기준 문장:

```text
SUV, Van, Pickup은 크고 높은 차급이지만 군용 장갑차나 무장 트럭처럼 보이면 안 된다.
```

---

## 8. 시트 형식 문구

AI 프롬프트에는 원화 시트 형식을 명확히 넣는다.

권장 포함 문구:

```text
orthographic concept sheet,
orthographic side front rear top views,
plain background,
part callouts,
material color callouts,
game asset reference sheet,
easy to model
```

권장 금지 문구:

```text
no cinematic render,
no action scene,
no dramatic perspective,
no complex background,
no inconsistent views
```

시트 판단 기준:

- Side View에서 차급 실루엣이 읽혀야 한다.
- Front/Rear View에서 앞뒤 방향과 폭/높이가 읽혀야 한다.
- Top View에서 루프 중앙 하드포인트와 터렛 회전 공간이 읽혀야 한다.
- 뷰마다 같은 차량으로 보여야 한다.

---

## 9. 재질/색상 문구

AI 프롬프트에는 저채도 기능성 색상과 단순 머티리얼 구분을 넣는다.

권장 포함 문구:

```text
low saturation industrial colors,
painted civilian car body,
matte armor panels,
dark simple glass,
dark rubber tires,
utility plastic bumpers and mounts,
small limited emissive sensor lights
```

피해야 할 문구:

```text
glossy supercar paint,
chrome luxury trim,
bright racing livery,
full neon body lines,
cyberpunk neon street scene
```

기준 문장:

```text
색상은 큰 면의 구분을 돕는 도구이며, 차량 전체를 장식하는 요소가 아니다.
```

---

## 10. 실전 프롬프트 예시

아래 예시는 바로 복사해 AI 이미지 생성에 사용할 수 있는 기본형이다.

주의:

- 예시는 제작 기준을 빠르게 적용하기 위한 출발점이다.
- 원하는 차급이 다르면 `6. 차급별 삽입 문구` 표의 문구로 교체한다.
- 예시를 사용할 때도 `5. 기본 금지 프롬프트`를 함께 넣는다.
- 결과물이 복잡하면 예시를 더 화려하게 늘리지 말고 문구를 줄여 다시 생성한다.

### 10.1 Compact 기본 예시

```text
Compact,
balanced compact hatchback silhouette,
balanced compact proportions,
simplified near-future civilian combat car,
C1 simple game asset concept sheet,
clean readable silhouette,
low modeling complexity,
large simple shapes,
large simple armor panels,
single simple roof hardpoint,
small roof turret mount,
clear turret rotation space,
low saturation industrial colors,
matte armor panels,
painted civilian car body,
dark simple glass,
dark rubber tires,
small limited emissive sensor lights,
orthographic side view,
orthographic front view,
orthographic rear view,
orthographic top view,
plain background,
part callouts,
material color callouts,
no real car brand,
no real car copy,
no tank,
no APC,
no military armored vehicle,
no oversized weapon,
no excessive realistic detail,
no complex panel lines,
no tiny mechanical parts,
no excessive neon,
no cinematic render,
no dramatic perspective,
no complex background,
no inconsistent views
```

사용 목적:

- CarFight의 기본 차량 후보를 빠르게 만들 때 사용한다.
- 차급, 장갑, 루프 하드포인트, 정사영 4뷰가 균형 있게 나와야 한다.

### 10.2 Sedan 기본 예시

```text
Sedan,
long low three-box sedan silhouette,
long low three-box proportions,
simplified near-future civilian combat car,
C1 simple game asset concept sheet,
clean readable silhouette,
low modeling complexity,
large simple shapes,
large simple armor panels,
single simple roof hardpoint,
small roof turret mount,
clear turret rotation space,
keep low roof and three-box body,
low saturation industrial colors,
matte armor panels,
painted civilian car body,
dark simple glass,
dark rubber tires,
small limited emissive sensor lights,
orthographic side view,
orthographic front view,
orthographic rear view,
orthographic top view,
plain background,
part callouts,
material color callouts,
no real car brand,
no real car copy,
no tank,
no APC,
no military armored vehicle,
no oversized weapon,
no excessive realistic detail,
no complex panel lines,
no exposed suspension,
no exposed engine,
no excessive neon,
no racing livery,
no cinematic render,
no dramatic perspective,
no inconsistent views
```

사용 목적:

- 낮고 긴 승용차 실루엣을 유지한 전투 개조 차량 후보를 만들 때 사용한다.
- 장갑이 과해져 SUV나 장갑차처럼 보이면 실패로 본다.

### 10.3 SUV 기본 예시

```text
SUV,
tall civilian SUV silhouette with large wheels,
tall wide body, large wheels,
simplified near-future civilian combat car,
C1 simple game asset concept sheet,
clean readable silhouette,
low modeling complexity,
large simple shapes,
large simple armor panels,
single simple roof hardpoint,
small roof turret mount,
clear turret rotation space,
keep civilian SUV shape,
avoid APC look,
low saturation industrial colors,
matte armor panels,
painted civilian car body,
dark simple glass,
dark rubber tires,
small limited emissive sensor lights,
orthographic side view,
orthographic front view,
orthographic rear view,
orthographic top view,
plain background,
part callouts,
material color callouts,
no real car brand,
no real car copy,
no tank,
no APC,
no military armored vehicle,
no mech,
no oversized weapon,
no multiple weapon platforms,
no excessive realistic detail,
no complex panel lines,
no tiny mechanical parts,
no excessive neon,
no cinematic render,
no action scene,
no dramatic perspective,
no inconsistent views
```

사용 목적:

- 큰 차체와 높은 지상고 인상을 가진 민간 SUV 후보를 만들 때 사용한다.
- 군용 장갑차, APC, 병력 수송차처럼 보이면 기본 후보에서 제외한다.

### 10.4 Pickup 기본 예시

```text
Pickup,
cabin and open rear bed silhouette,
separated cabin and open rear bed,
simplified near-future civilian combat car,
C1 simple game asset concept sheet,
clean readable silhouette,
low modeling complexity,
large simple shapes,
large simple armor panels,
single simple roof hardpoint,
small roof turret mount,
clear turret rotation space,
keep open rear bed,
avoid technical truck look,
low saturation industrial colors,
matte armor panels,
painted civilian car body,
dark simple glass,
dark rubber tires,
small limited emissive sensor lights,
orthographic side view,
orthographic front view,
orthographic rear view,
orthographic top view,
plain background,
part callouts,
material color callouts,
no real car brand,
no real car copy,
no tank,
no APC,
no military armored vehicle,
no oversized weapon,
no multiple weapon platforms,
no excessive realistic detail,
no complex panel lines,
no tiny mechanical parts,
no exposed suspension,
no exposed engine,
no excessive neon,
no cinematic render,
no action scene,
no dramatic perspective,
no inconsistent views
```

사용 목적:

- 캐빈과 적재함이 분리되어 읽히는 픽업 후보를 만들 때 사용한다.
- 적재함에 무기 플랫폼을 많이 올려 군용 테크니컬 차량처럼 보이면 실패로 본다.

---

## 11. AI 결과 검수 순서

AI 이미지 생성 결과는 아래 순서로 검수한다.

1. 이미지를 축소해서 차급, 방향, 무장 여부가 읽히는지 확인한다.
2. Side/Front/Rear/Top View가 같은 차량으로 보이는지 확인한다.
3. 차급 실루엣과 비례가 지정한 문구와 맞는지 확인한다.
4. 장갑판이 큰 판형 4~6개 중심으로 정리될 수 있는지 확인한다.
5. 루프 중앙 하드포인트 1개와 터렛 회전 공간이 보이는지 확인한다.
6. 재질/색상이 5~6개 머티리얼 슬롯으로 정리될 수 있는지 확인한다.
7. 실제 브랜드, 탱크, APC, 군용 장갑차, 과도한 네온, 복잡한 디테일 방향으로 흐르지 않았는지 확인한다.

통과 기준:

```text
한 번의 생성 결과를 크게 고치지 않고 C1 Simple Game Asset 후보로 해석할 수 있어야 한다.
```

탈락 기준:

```text
오래 수정해야만 쓸 수 있는 복잡한 결과물은 단일 패스 최종 후보 제작 원칙에 맞지 않는다.
```

---

## 12. 통과/탈락 질문

### 12.1 통과 질문

아래 질문에 대부분 `예`라고 답할 수 있어야 한다.

- 지정한 차급이 먼저 읽히는가?
- 민간 차량 실루엣 위에 전투 개조가 붙은 형태인가?
- R1 Gameplay Mid에서 차급, 방향, 무장 여부가 읽힐 것 같은가?
- 큰 장갑판과 루프 하드포인트가 단순하게 보이는가?
- 작은 디테일을 텍스처/머티리얼로 넘길 수 있는가?
- 정사영 4뷰 시트로 모델링 참고가 가능한가?
- 실제 브랜드나 특정 실제 모델 복제로 보이지 않는가?

### 12.2 탈락 질문

아래 질문에 하나라도 강하게 `예`라면 재생성 또는 교체한다.

- 탱크, APC, 군용 장갑차처럼 보이는가?
- 무기가 차량보다 먼저 읽히는가?
- 실제 브랜드 차량을 거의 복제한 것처럼 보이는가?
- 장갑판이 너무 많은 작은 조각으로 나뉘어 있는가?
- 복잡한 패널 라인, 노출 서스펜션, 엔진룸 디테일이 중심인가?
- 네온 장식이나 레이싱 리버리가 전체 인상을 지배하는가?
- 3/4 감상용 렌더만 있고 정사영 4뷰 정보가 부족한가?

---

## 13. 현재 결정 상태

| 항목 | 결정 |
|---|---|
| 기본 프롬프트 목적 | C1 최종 후보용 AI 원화 생성 |
| 기본 스타일 문구 | simplified near-future civilian combat car |
| 기본 제작성 문구 | C1 simple game asset, low modeling complexity |
| 기본 시트 문구 | orthographic side/front/rear/top views |
| 차급 문구 | 10개 민간 차량 차급별 실루엣/비례 문구 사용 |
| 장갑 문구 | large simple armor panels |
| 하드포인트 문구 | single simple roof hardpoint |
| 색상 문구 | low saturation industrial colors |
| 금지 방향 | 실제 브랜드, 탱크, APC, 군용 장갑차, 과도한 네온, 고디테일 렌더 |
| 검수 기준 | 축소 검수와 C1 제작 가능성 우선 |
| AI 모델링 연계 | 큰 파츠가 분리되어 해석되는 프롬프트 우선 |

---

## 14. 다음 작업 연결

이 문서로 ConceptArt 기준 문서의 기본 프롬프트 규칙은 1차 완료 상태가 된다.

이후 AI 생성 작업에서는 아래 순서를 따른다.

1. `04_VehicleClasses.md`에서 차급을 고른다.
2. 이 문서의 차급별 삽입 문구를 적용한다.
3. `10_ConceptSheetFormat.md`의 정사영 4뷰 시트 형식을 요청한다.
4. 생성 결과를 `02_ComplexityBudget.md`, `03_CameraReadability.md`, `07_ArmorPanelRules.md`, `08_HardpointRules.md`, `09_MaterialColorRules.md` 기준으로 검수한다.
5. C1 후보가 아니면 오래 수정하지 않고 프롬프트를 단순화해 재생성한다.

---

## 15. Changelog

### v0.3

- AI 이미지 생성 이후 AI 모델링 도구로 3D 후보를 생성한다는 전제를 현재 확정 기준에 추가했다.
- AI 모델링 연계용 프롬프트 문구를 추가했다.
- 프롬프트의 목적을 멋진 이미지 생성뿐 아니라 큰 파츠가 분리되어 해석되는 입력 자료 생성으로 보강했다.

### v0.2

- Compact, Sedan, SUV, Pickup 기본 프롬프트 예시를 추가했다.
- 각 예시의 사용 목적과 실패 기준을 함께 정리했다.
- 기존 프롬프트 규칙은 유지하고, 실제 생성에 바로 쓸 수 있는 복사용 문구를 보강했다.

### v0.1

- AI 이미지 생성용 기본 프롬프트 구조를 정의했다.
- 차급별 실루엣 문구와 비례 문구를 정리했다.
- 필수 포함 문구와 금지 프롬프트를 분리했다.
- 정사영 Side/Front/Rear/Top View 시트 출력 문구를 추가했다.
- AI 생성 결과의 통과/탈락 검수 순서를 추가했다.

---

## 16. Migration

### v0.3 적용 시 기존 작업 영향

- 기존 v0.2 프롬프트 예시는 유지하되, AI 모델링 입력으로 쓸 때는 `AI 3D modeling ready`, `clear separated major parts` 문구를 추가할 수 있다.
- 생성 이미지가 좋아 보여도 Body, Wheel, Glass, Armor, Mount 구분이 흐리면 AI 모델링 입력 후보에서 제외한다.
- 프롬프트 결과가 작은 기계 디테일을 과생성하면 금지 문구와 단순 파츠 문구를 강화한다.

### v0.2 적용 시 기존 작업 영향

- 기존 v0.1 프롬프트 구조는 그대로 유지한다.
- 새 AI 생성 작업은 먼저 `10. 실전 프롬프트 예시` 중 가까운 차급을 복사해 시작할 수 있다.
- 예시에 없는 차급은 `6. 차급별 삽입 문구`를 사용해 같은 구조로 교체한다.
- 예시 결과가 복잡하게 나오면 문구를 추가하기보다 금지 문구와 제작성 문구를 우선 강화한다.

### v0.1 적용 시 기존 작업 영향

- 앞으로 AI 차량 원화는 이 문서의 기본 프롬프트 템플릿을 기준으로 생성한다.
- 기존 AI 생성 차량 이미지는 차급, 실루엣, 비례, 장갑, 하드포인트, 재질, 시트 형식 기준으로 재검토한다.
- 3/4 감상용 이미지는 C1 모델링 기준 시트로 바로 쓰지 않고 보조 참고 이미지로만 본다.
- 금지 프롬프트에 걸리는 결과는 오래 수정하기보다 재생성 또는 교체를 우선한다.
- 이 문서는 새 아트 방향을 만들지 않고, 기존 ConceptArt 기준을 AI 생성 문장으로 변환하는 하위 기준으로 사용한다.
