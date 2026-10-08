# CarFight Concept Art - Compact AI Mesh Prompt

문서 버전: v0.1  
작성일: 2026-07-06  
문서 위치: `Document/Plan/ConceptArt/13_CompactAIMeshPrompt.md`  
상위 기준 문서: `Document/Plan/ConceptArt/00_ArtDirection.md`  
관련 문서: `Document/Plan/ConceptArt/02_ComplexityBudget.md`, `Document/Plan/ConceptArt/10_ConceptSheetFormat.md`, `Document/Plan/ConceptArt/11_AIPromptRules.md`, `Document/Plan/ConceptArt/12_CompactBreakdown.md`  
관련 이미지: `Document/Plan/ConceptArt/11_AIPromptRules_CompactPreview.png`  
관련 범위: Compact 차량 후보의 AI 3D 모델링 입력 프롬프트, 파츠 분리 요구사항, 머티리얼 슬롯 요구사항, 금지 구조, 생성 결과 검수 기준

---

## 1. 문서 목적

이 문서는 Compact 차량 후보를 AI 3D 모델링 도구에 넣기 위한 **입력 프롬프트 기준**을 정의한다.

`11_AIPromptRules.md`가 AI 이미지 생성용 프롬프트 기준이라면, 이 문서는 AI 모델링 결과가 언리얼 적용 가능한 C1 후보에 가까워지도록 파츠, 피벗, 머티리얼, 금지 구조를 더 직접적으로 지정한다.

이 문서의 목적은 다음과 같다.

- Compact 후보를 AI 모델링 도구에 입력할 때 사용할 기본 문장을 만든다.
- 생성 결과가 큰 파츠 중심으로 분리되도록 유도한다.
- 작은 디테일이 불필요한 기하 구조로 과생성되지 않게 제한한다.
- 결과물을 1인 개발자가 검수·수정·언리얼 적용할 수 있는 수준으로 제한한다.
- 같은 방식으로 이후 Sedan, SUV, Pickup AI 모델링 입력 문서를 만들 수 있는 예시를 제공한다.

---

## 2. 현재 확정 기준

| 항목 | 기준 |
|---|---|
| 대상 차급 | Compact / 컴팩트 해치백 |
| 기본 스타일 | Simplified Near-Future Civilian Combat Car |
| 기본 제작 목표 | C1 Simple Game Asset 최종 후보 |
| AI 모델링 전제 | AI 모델링 도구로 3D 후보 생성 후 검수·후처리 |
| 권장 파츠 수 | 16개 |
| 단순화 파츠 수 | 10~12개 |
| 머티리얼 슬롯 | 6개 |
| 필수 분리 | 휠 4개, 루프 마운트, 큰 장갑판 |
| 작은 디테일 | 텍스처/노멀/머티리얼 처리 |

최상위 기준:

```text
AI 모델링 결과가 보기 좋아도 파츠, 피벗, 머티리얼, 콜리전 정리가 어렵다면 C1 후보가 아니다.
```

---

## 3. AI 모델링 입력 원칙

AI 3D 모델링 입력은 이미지 생성 프롬프트보다 더 구조적으로 작성한다.

작성 순서:

1. 차량 차급과 기본 실루엣을 지정한다.
2. C1 Simple Game Asset 기준을 지정한다.
3. 큰 파츠 분리 요구사항을 지정한다.
4. 휠과 루프 마운트의 피벗 후보를 지정한다.
5. 머티리얼 슬롯 수와 역할을 지정한다.
6. 작은 디테일은 텍스처/노멀 처리하라고 지정한다.
7. 금지 구조를 명확히 지정한다.

중요:

```text
AI 모델링 프롬프트는 멋진 외형보다 정리 가능한 구조를 먼저 요구해야 한다.
```

---

## 4. 기본 AI 모델링 프롬프트

아래 문장은 Compact 후보의 기본 AI 3D 모델링 입력으로 사용한다.

```text
Create a simplified near-future civilian combat compact hatchback as a C1 simple game asset.
The vehicle must keep a balanced compact hatchback silhouette with a short rear hatch, simple cabin, four visible wheels, and a civilian car body first.
Generate simple mesh-friendly forms with low modeling complexity.
Use large simple shapes and avoid excessive realistic car detail.

Separate the vehicle into clear major parts:
Body_Main,
Wheel_FL,
Wheel_FR,
Wheel_RL,
Wheel_RR,
Glass_Main,
Armor_Front,
Armor_Left,
Armor_Right,
Armor_Rear,
Bumper_Front,
Bumper_Rear,
WheelGuard_FLFR,
WheelGuard_RLRR,
Mount_Roof,
Sensor_Light.

Keep the four wheels as separate round wheel parts with clear center pivots.
Keep the roof hardpoint as one separate simple mount at the center of the roof.
Keep armor as large flat or slightly beveled panels attached to the civilian car body.
Keep glass as simple dark window surfaces.
Keep small bolts, panel lines, scratches, labels, tire grooves, and surface wear as texture or normal map details, not separate geometry.

Use six simple material groups:
M01 Painted_Body,
M02 Matte_Armor,
M03 Dark_Glass,
M04 Rubber_Tire,
M05 Utility_Plastic,
M06 Emissive_Decal.

The final model should be easy to inspect, easy to simplify, easy to set pivots, easy to create simple collision, and suitable for Unreal Engine import.
```

사용 기준:

- AI 모델링 도구가 파츠 이름을 지원하지 않더라도, 파츠 구조를 유도하기 위해 이름을 넣는다.
- 결과가 16개 파츠로 정확히 나오지 않아도 큰 분리 기준이 읽히면 검토 후보로 본다.
- 휠과 루프 마운트는 기능 연결 가능성이 있으므로 가능하면 분리되어야 한다.

---

## 5. 단순화 프롬프트

기본 프롬프트 결과가 너무 복잡하면 아래 단순화 문구를 추가한다.

```text
Simplify the model further.
Use fewer separate parts.
Keep only the main body, four wheels, one glass group, four large armor panels, one front bumper block, one rear bumper block, and one roof mount.
Remove tiny geometry details.
Do not model small bolts, thin panel lines, exposed suspension, engine parts, cables, antennas, or complex mechanical pieces.
The model should remain readable from gameplay camera distance.
```

적용 기준:

- 파츠 수가 20개를 넘는 결과가 나왔을 때 사용한다.
- 장갑판이 너무 잘게 쪼개졌을 때 사용한다.
- 작은 기계 디테일이 메쉬로 많이 생성되었을 때 사용한다.
- 차급 실루엣보다 장갑/무기가 먼저 보일 때 사용한다.

---

## 6. 금지 프롬프트

아래 금지 문구는 AI 모델링 입력에 함께 사용한다.

```text
no real car brand,
no real car copy,
no tank,
no APC,
no military armored vehicle,
no mech,
no oversized weapon,
no multiple weapon platforms,
no complex exposed suspension,
no exposed engine,
no detailed undercarriage,
no many small armor plates,
no tiny mechanical greebles,
no dense cables,
no thin antennas,
no complex interior,
no opening doors or opening hood requirement,
no excessive realistic panel lines,
no glossy supercar body,
no racing livery,
no excessive neon,
no high-detail hard-surface showcase model
```

금지 기준:

- 실제 브랜드 복제 위험이 있으면 탈락한다.
- 탱크/APC/군용 장갑차처럼 보이면 탈락한다.
- 작은 기계 디테일이 메쉬의 대부분을 차지하면 탈락한다.
- 언리얼에서 피벗/콜리전/LOD 정리가 어려우면 탈락한다.

---

## 7. 파츠별 생성 요구사항

| 파츠 | 요구사항 |
|---|---|
| Body_Main | Compact 해치백 실루엣을 만드는 가장 큰 덩어리 |
| Wheel_FL/FR/RL/RR | 각각 분리된 원형 휠. 중심 피벗을 잡기 쉬워야 함 |
| Glass_Main | 전면/측면/후면 유리를 어두운 단순 면으로 통합 |
| Armor_Front | 전면의 큰 판형 장갑 |
| Armor_Left/Right | 좌우 측면의 큰 판형 장갑. 가능하면 좌우 대칭 |
| Armor_Rear | 후면의 큰 판형 장갑 |
| Bumper_Front/Rear | 전후 방향을 읽게 하는 단순 보강 범퍼 |
| WheelGuard_FLFR/RLRR | 휠 주변 실루엣을 보강하는 단순 휠가드 |
| Mount_Roof | 루프 중앙의 단순 하드포인트. 회전 중심이 예측 가능해야 함 |
| Sensor_Light | 작은 발광 포인트. 필요하면 머티리얼로 통합 가능 |

우선순위:

```text
Body_Main → Wheels → Glass_Main → Armor → Mount_Roof → Bumper/WheelGuard → Sensor_Light
```

---

## 8. 머티리얼 요구사항

AI 모델링 결과는 아래 머티리얼 슬롯으로 정리 가능해야 한다.

| 슬롯 | 역할 | 색상/재질 방향 |
|---|---|---|
| M01 Painted_Body | 차체 도장 | 저채도 민간 차량 도장 |
| M02 Matte_Armor | 장갑판 | 무광 또는 반무광 어두운 장갑 |
| M03 Dark_Glass | 유리 | 어두운 틴트 유리 |
| M04 Rubber_Tire | 타이어 | 어두운 고무 |
| M05 Utility_Plastic | 트림/휠가드/마운트 | 무광 플라스틱 또는 기능성 하우징 |
| M06 Emissive_Decal | 센서등/작은 발광 | 제한적인 작은 포인트 |

주의:

- 머티리얼 슬롯이 8개를 넘으면 단순화한다.
- 유리와 차체가 같은 머티리얼로 붙어 있으면 후처리 부담이 커진다.
- 발광은 작은 센서 포인트로만 사용한다.

---

## 9. AI 모델링 결과 검수표

AI 모델링 결과를 받은 뒤 아래 항목으로 통과 여부를 판단한다.

| 체크 | 통과 기준 |
|---|---|
| 차급 | Compact 해치백으로 먼저 읽힘 |
| 파츠 수 | 10~16개 권장, 20개 이하 |
| 휠 | 4개가 분리되어 있고 위치가 명확함 |
| 루프 마운트 | 중앙에 있고 회전 중심을 잡기 쉬움 |
| 장갑 | 큰 판형 중심. 작은 조각 남발 없음 |
| 유리 | 어두운 단순 면으로 구분됨 |
| 머티리얼 | 5~6개 슬롯으로 정리 가능 |
| 콜리전 | 차체를 단순 박스/컨벡스로 근사 가능 |
| LOD | 작은 디테일을 지워도 실루엣 유지 |
| 금지 방향 | 탱크/APC/군용 장갑차/실제 브랜드가 아님 |

최종 판정:

```text
파츠가 조금 다르더라도 검수·후처리로 C1 기준에 맞출 수 있으면 유지한다.
파츠가 엉켜 있거나 작은 기하 디테일이 과하면 재생성한다.
```

---

## 10. 재생성 조건

아래 조건에 해당하면 오래 수정하지 않고 재생성한다.

- Body, Wheel, Glass, Armor, Mount 구분이 흐리다.
- 휠 4개 위치가 불명확하거나 차체에 붙어 있다.
- 루프 마운트가 없거나 차량보다 과하게 크다.
- 장갑이 수십 개의 작은 조각으로 생성되었다.
- 실제 브랜드 차량처럼 보인다.
- 탱크, APC, 군용 장갑차, 테크니컬 트럭처럼 보인다.
- 노출 서스펜션, 엔진, 케이블, 내부 구조가 과하다.
- 머티리얼 슬롯을 8개 이하로 줄이기 어렵다.
- 언리얼에서 피벗과 콜리전을 잡기 어렵다.

재생성 원칙:

```text
복잡한 결과를 고치는 것보다 단순한 프롬프트로 다시 생성하는 것이 기본이다.
```

---

## 11. 현재 결정 상태

| 항목 | 결정 |
|---|---|
| 문서 성격 | Compact AI 3D 모델링 입력 기준 |
| 대상 후보 | `11_AIPromptRules_CompactPreview.png` |
| 기본 프롬프트 | 큰 파츠 16개 분리 유도 |
| 단순화 프롬프트 | 10~12개 파츠까지 축소 허용 |
| 머티리얼 슬롯 | 6개 |
| 필수 검수 | 차급, 휠, 루프 마운트, 장갑, 머티리얼, 콜리전 |
| 실패 처리 | 장시간 수정보다 재생성 우선 |

---

## 12. Changelog

### v0.1

- Compact 후보용 AI 3D 모델링 입력 프롬프트를 작성했다.
- 기본 프롬프트, 단순화 프롬프트, 금지 프롬프트를 분리했다.
- 파츠별 생성 요구사항과 머티리얼 슬롯 요구사항을 정리했다.
- AI 모델링 결과 검수표와 재생성 조건을 추가했다.

---

## 13. Migration

### v0.1 적용 시 기존 작업 영향

- `11_AIPromptRules.md`는 이미지 생성 프롬프트 기준으로 유지한다.
- `12_CompactBreakdown.md`는 파츠 분해와 검수 기준으로 유지한다.
- 이 문서는 Compact 후보를 실제 AI 3D 모델링 도구에 입력할 때 사용하는 문장 기준으로 사용한다.
- 이후 다른 차급의 AI 모델링 입력 문서는 이 문서 구조를 복사하되, 차급 실루엣과 파츠 기준만 바꾼다.
