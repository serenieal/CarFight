# 18_ProjectileArtRules.md

문서 버전: v0.1  
작성일: 2026-07-08  
상태: Draft  
관련 범위: 차량용 발사체 원화 기준, Tripo 발사체 메시 생성 기준, UE `ProjectileData` / `CFProjectileActor` 적용 기준

---

## 0. 문서 목적

이 문서는 CarFight의 차량용 발사체 원화를 만들기 위한 기준 문서다.

현재 범위는 모든 탄종/투사체/이펙트가 아니라, P0 단계에서 실제 `Projectile Actor`로 날아가는 발사체 메시를 만드는 기준에 한정한다.

이 문서의 목적은 다음과 같다.

1. Tripo로 발사체 StaticMesh를 만들기 전에 원화가 지켜야 할 기준을 정한다.
2. 현재 `UCFProjectileData`와 `ACFProjectileActor` 구조에 맞는 발사체 메시 기준을 정의한다.
3. 발사체 메시와 충돌 반경, 이동 방향, 피격 이펙트, DamageData의 역할을 분리한다.
4. 발사체를 실제 탄약 제조물처럼 복잡하게 만들지 않고, 게임에서 읽히는 단순한 시각 오브젝트로 정의한다.
5. Tripo가 그림자, 하이라이트, 배경, 라벨을 메시 일부로 오해하지 않도록 입력 이미지 기준을 정한다.
6. Blender 수동 모델링 없이 스크립트 기반 정리와 UE 적용이 가능하도록 축, 피벗, 스케일 기준을 정한다.

이 문서는 감상용 일러스트 기준이 아니다.

```text
발사체 원화
→ Tripo 입력용 이미지 제작
→ Tripo 3D 생성
→ Blender에서 스크립트 실행
→ UE ProjectileStaticMesh 적용
→ ProjectileData 값으로 이동/충돌/수명 조정
```

위 제작 흐름을 위한 실무 기준이다.

---

## 1. 상위 기준 문서

이 문서는 아래 문서를 기준으로 작성한다.

| 문서 | 사용 기준 |
|---|---|
| `Document/Plan/ConceptArt/00_ArtDirection.md` | 단순화된 근미래 민간 차량 개조 전투차 방향 |
| `Document/Plan/ConceptArt/02_ComplexityBudget.md` | 1인 개발 가능 복잡도, C1 Simple Game Asset 기준 |
| `Document/Plan/ConceptArt/11_AIPromptRules.md` | AI 이미지 생성 프롬프트 기본 구조 |
| `Document/Plan/ConceptArt/16_WeaponArtRules.md` | P0 루프 터렛 무기 원화와 발사체의 무기 연계 기준 |
| `Document/Plan/ConceptArt/17_TripoInputImageRules.md` | Tripo 입력 이미지 제작 공통 규칙 |
| `Document/ProjectSSOT/CombatPlan/06_WeaponTypes.md` | 대구경포 역할, 느린 발사, 강한 한 방 기준 |
| `UE/Source/CarFight_Re/Public/CFProjectileData.h` | `ProjectileStaticMesh`, 이동, 충돌, 수명, DamageData 데이터 구조 |
| `UE/Source/CarFight_Re/Public/CFProjectileActor.h` | `CollisionComponent`, `MeshComponent`, `ProjectileMovementComponent` 구조 |
| `UE/Source/CarFight_Re/Private/CFProjectileActor.cpp` | `ProjectileStaticMesh`, 상대 회전, 상대 스케일, 충돌 반경 적용 방식 |

---

## 2. 현재 문서 범위

### 2.1 포함 범위

이 문서에서 다루는 대상은 다음이다.

```text
P0 Projectile Mesh
P0 차량용 발사체 StaticMesh
P0 Roof Turret Cannon용 물리 발사체 시각 메시
```

현재 우선 대상은 `P0 Roof Turret Cannon`에서 발사되는 대구경 저속 포탄형 발사체다.

### 2.2 제외 범위

이 문서에서는 아래 항목을 확정하지 않는다.

1. 실제 Damage 수치
2. 장갑 관통 공식
3. 탄도학 기반 실제 탄약 구조
4. 실제 폭발물/탄약 제작 구조
5. 피격 VFX 최종 사양
6. 폭발 VFX 최종 사양
7. 파편 생성 규칙
8. 히트스캔 레이저 시각 효과
9. 미사일/로켓 전체 원화 기준
10. 연막탄/플레어 전체 원화 기준

위 항목은 별도 전투/VFX/탄종 문서에서 확장한다.

---

## 3. 현재 UE 적용 구조

### 3.1 `UCFProjectileData` 기준

현재 발사체 데이터는 `UCFProjectileData`가 소유한다.

발사체 원화와 직접 관련 있는 주요 필드는 다음이다.

| 필드 | 역할 | 원화 기준 영향 |
|---|---|---|
| `ProjectileId` | 발사체 식별 ID | 발사체 종류 이름과 파일명 기준 |
| `InitialSpeed` | 초기 속도 | 빠른 탄/느린 탄의 시각적 길이와 트레일 후보에 영향 |
| `bAffectedByGravity` | 중력 영향 여부 | 포물선 탄인지 직선 탄인지 시각 감각에 영향 |
| `GravityScale` | 중력 배율 | 무거운 포탄 감각 여부에 영향 |
| `LifeTimeSeconds` | 월드 유지 시간 | 멀리 보이는 발사체인지 짧게 사라지는 탄인지 판단 |
| `CollisionRadius` | 충돌 판정 반경 | 메시 두께/읽힘 기준과 비교해야 함 |
| `ProjectileActorClass` | 실제 발사체 Actor 클래스 | 실제 메시가 필요한 발사체인지 판단 |
| `ProjectileStaticMesh` | 표시할 발사체 StaticMesh | Tripo로 만들 실제 결과물 |
| `ProjectileMeshRelativeRotation` | 메시 방향 보정 | 메시 앞 방향이 `+X`가 아니면 보정 필요 |
| `ProjectileMeshRelativeScale` | 표시 스케일 | 메시와 충돌 반경을 분리해 조정 |
| `ImpactEffectId` | 피격 효과 후보 ID | 피격 이펙트는 메시가 아니라 별도 VFX로 분리 |
| `DefaultDamageData` | 피해 데이터 직접 참조 | 원화가 아니라 전투 데이터 영역 |

### 3.2 `ACFProjectileActor` 기준

현재 발사체 Actor는 다음 컴포넌트를 가진다.

```text
BP_CFProjectile / ACFProjectileActor
  CollisionComponent        = SphereComponent
  MeshComponent             = StaticMeshComponent
  ProjectileMovementComponent
```

역할은 다음과 같다.

| 컴포넌트 | 역할 | 원화 기준 영향 |
|---|---|---|
| `CollisionComponent` | 충돌 판정 담당 SphereComponent | 발사체 메시가 충돌을 직접 담당하지 않음 |
| `MeshComponent` | 발사체 StaticMesh 표시 | Tripo로 만드는 실제 메시가 여기에 들어감 |
| `ProjectileMovementComponent` | 이동, 속도, 중력 처리 | 메시에는 이동 기능을 만들지 않음 |

중요한 기준은 다음이다.

```text
발사체 메시 = 시각 표시용 StaticMesh
충돌 판정 = CollisionRadius 기반 SphereComponent
이동 = ProjectileMovementComponent
피해 = ProjectileData가 참조하는 DamageData
피격 표현 = ImpactEffectId 기반 후속 VFX/SFX
```

따라서 원화에는 충돌체, 이동 궤도, 피해량, 폭발 범위, 관통 구조를 모델링하지 않는다.

---

## 4. P0 발사체 정의

### 4.1 기본 명칭

| 항목 | 값 |
|---|---|
| 영문 짧은 이름 | `P0 Cannon Projectile` |
| 영문 정식 이름 | `P0 Large-Caliber Slow Cannon Projectile` |
| 한국어 이름 | `P0 대구경 저속 포탄형 발사체` |
| 기준 무기 | `P0 Roof Turret Cannon` |
| 기준 Actor | `BP_CFProjectile` |
| 기준 DataAsset | `UCFProjectileData` 계열 DataAsset |
| 기준 메시 필드 | `ProjectileStaticMesh` |

### 4.2 발사체 정체성

P0 발사체는 실제 탄약 제작물을 재현하는 목적이 아니다.

이 발사체는 게임 안에서 아래 정보를 빠르게 읽히게 하기 위한 시각 메시다.

```text
무겁다.
느리다.
대구경 포탄이다.
차량용 터렛에서 발사되었다.
충돌하면 강한 한 방을 줄 것처럼 보인다.
```

### 4.3 전투 감각

P0 발사체의 전투 감각은 다음을 목표로 한다.

1. 빠른 기관총 탄이 아니라 무거운 포탄처럼 보인다.
2. 너무 작아서 보이지 않는 탄환이 아니라, 테스트 중 비행 방향을 확인할 수 있는 메시다.
3. 너무 길거나 복잡해서 미사일처럼 보이면 안 된다.
4. 실제 폭발물 내부 구조나 탄약 제조 디테일을 드러내지 않는다.
5. 발사체 본체는 단순하고, 발사/비행/피격 표현은 VFX로 보강한다.

---

## 5. 메시/데이터 분리 기준

### 5.1 메시가 담당하는 것

발사체 StaticMesh가 담당하는 것은 아래뿐이다.

1. 발사체 외형
2. 전방 방향 식별
3. 두께감
4. 게임 카메라에서의 가독성
5. 충돌 반경과 대략 맞는 시각 크기

### 5.2 메시가 담당하지 않는 것

발사체 StaticMesh는 아래를 담당하지 않는다.

1. 실제 충돌 판정
2. 실제 피해량
3. 실제 폭발 범위
4. 실제 장갑 관통 공식
5. 트레일 VFX
6. 총구 화염 VFX
7. 피격 폭발 VFX
8. 파편 생성
9. 사운드
10. 네트워크 판정

### 5.3 VFX로 분리할 것

아래 표현은 발사체 메시가 아니라 VFX 또는 머티리얼/나이아가라 후보로 분리한다.

| 표현 | 처리 기준 |
|---|---|
| 발사 화염 | Muzzle VFX |
| 탄도 연기 | Projectile Trail VFX |
| 예광선 | Trail 또는 Beam VFX |
| 피격 불꽃 | Impact VFX |
| 폭발 | Explosion VFX |
| 충격파 | Impact/Explosion VFX |
| 파편 | 후속 Fragment 시스템 후보 |
| 열기 왜곡 | 후속 Post/VFX 후보 |

P0 메시 원화에서는 발사체 본체만 만든다.

---

## 6. 축/피벗/스케일 기준

### 6.1 기본 축 기준

CarFight의 발사체 메시 기준 축은 다음으로 둔다.

```text
Forward = +X
Right   = +Y
Up      = +Z
```

발사체의 머리 또는 진행 방향은 반드시 `+X`를 향해야 한다.

### 6.2 피벗 기준

발사체 메시의 원점 기준은 다음 중 하나를 사용할 수 있다.

| 기준 | 용도 |
|---|---|
| 중심 피벗 | 기본값. Sphere Collision과 맞추기 쉽다. |
| 후방 피벗 | Trail/VFX 연결이 편하지만 충돌 반경 정렬이 더 어려울 수 있다. |

P0 기본값은 중심 피벗이다.

```text
P0 기본:
발사체 메시 중심 = Actor 원점 = CollisionComponent 중심
```

### 6.3 메시 길이와 충돌 반경 관계

현재 `CollisionComponent`는 SphereComponent다.

따라서 발사체 메시가 너무 길면 실제 시각 길이와 충돌 반경이 크게 달라진다.

P0 발사체 메시는 다음 기준을 따른다.

1. 지나치게 긴 막대형 발사체 금지.
2. 전차 날탄처럼 길고 얇은 형태 금지.
3. Sphere Collision으로도 대략 납득 가능한 짧고 두꺼운 형상 우선.
4. 길이는 두께보다 길 수 있지만, 충돌 반경과 너무 괴리되면 안 된다.
5. 메시의 가장 긴 축은 `+X` 방향이어야 한다.

권장 비율은 다음이다.

| 항목 | 권장 기준 |
|---|---:|
| 길이 | 두께의 약 2.0~3.5배 |
| 최대 두께 | CollisionRadius 시각 기준과 크게 다르지 않게 |
| 후방 핀/날개 | P0 대구경 포탄에서는 기본 금지 |
| 매우 얇은 꼬리 | 금지 |

### 6.4 상대 회전 보정 기준

`ProjectileMeshRelativeRotation`은 보정용 필드다.

하지만 원화/Tripo/Blender 기준에서는 가능한 한 보정 없이 쓰는 것을 목표로 한다.

```text
목표:
ProjectileMeshRelativeRotation = 0, 0, 0
```

즉, 메시 자체가 처음부터 `+X` 방향을 향하도록 정리한다.

---

## 7. P0 대구경 포탄형 발사체 실루엣 기준

### 7.1 권장 실루엣

P0 대구경 포탄형 발사체는 다음 실루엣을 우선한다.

```text
짧고 두꺼운 탄두형 본체
+ 둔중한 전방부
+ 단순한 후방부
+ 명확한 진행 방향
```

### 7.2 권장 형태

권장 형태는 다음과 같다.

1. 짧고 두꺼운 포탄형 실루엣
2. 진행 방향이 명확한 앞쪽 형태
3. 너무 날카롭지 않은 전방부
4. 단순한 원통형 또는 약간 각진 하드서피스 본체
5. 후방은 VFX Trail이 붙을 수 있는 단순한 면
6. 카메라에서 작게 보여도 탄 방향이 읽히는 형태
7. 텍스처 없이도 실루엣만으로 방향이 읽히는 형태

### 7.3 피해야 할 형태

아래 형태는 금지한다.

1. 실제 총알처럼 지나치게 사실적인 탄약 형태
2. 실제 탄피/뇌관/탄두 결합 구조가 자세히 드러나는 형태
3. 미사일처럼 보이는 날개/핀/로켓 노즐 구조
4. 로켓탄처럼 긴 실린더와 후방 노즐이 강조된 형태
5. 드릴탄처럼 복잡한 회전날/날개 구조
6. 파편탄 내부 구조처럼 보이는 세부 표현
7. 실제 군용 탄약 식별색/마킹을 모사한 표현
8. 너무 얇고 긴 막대형 관통탄 형태
9. 사람 손에 들 수 있는 총알처럼 보이는 소형 실탄 느낌
10. 과도한 로고, 문자, 숫자, 경고 라벨

---

## 8. Tripo 입력 이미지 기준

### 8.1 상위 규칙

발사체 Tripo 입력 이미지는 `17_TripoInputImageRules.md`의 범용 규칙을 따른다.

특히 아래 조건은 반드시 적용한다.

```text
single object only
plain background
no shadows
no cast shadow
no contact shadow
no ground shadow
no gradient shadow
shadowless lighting
no text
no callouts
no dimension lines
```

### 8.2 발사체 입력 이미지 구성

발사체는 터렛처럼 여러 메시 파츠로 나누지 않는다.

P0 기본 입력 이미지는 하나의 발사체 메시를 위한 단일 오브젝트 시트다.

```text
Projectile_InputSheet 1장
  - Left Large Area: Quarter View
  - Right Top Left: Front View
  - Right Top Right: Back View
  - Right Bottom Left: Left View
  - Right Bottom Right: Right View
```

### 8.3 발사체 뷰 방향 기준

발사체의 뷰 방향은 다음처럼 정의한다.

| 뷰 | 의미 |
|---|---|
| Quarter View | 전방 `+X`와 상부가 동시에 보이는 3/4 뷰 |
| Front View | 발사체 머리 쪽, 즉 `+X` 방향에서 본 모습 |
| Back View | 발사체 후방, 즉 `-X` 방향에서 본 모습 |
| Left View | 발사체 측면, 길이와 실루엣 확인 |
| Right View | 반대 측면, 좌우 대칭 또는 비대칭 확인 |

발사체는 길쭉한 오브젝트이므로 Side/Left View에서 전체 길이가 가장 잘 읽혀야 한다.

### 8.4 Tripo 입력 이미지 금지 요소

Tripo 입력 이미지에는 아래를 넣지 않는다.

1. 발사 궤적
2. 연기 Trail
3. 불꽃 Trail
4. 폭발 장면
5. 피격 장면
6. 총구 화염
7. 차량 또는 터렛
8. 여러 발사체 묶음
9. 탄약 상자
10. 실제 탄약 단면도
11. 텍스트 라벨
12. 치수선
13. 콜아웃
14. 그림자
15. 접지면
16. 복잡한 배경

---

## 9. 컨셉이미지 기준

### 9.1 컨셉이미지와 Tripo 입력 이미지 구분

발사체도 컨셉이미지와 Tripo 입력 이미지를 구분한다.

| 구분 | 목적 | 구성 |
|---|---|---|
| 컨셉이미지 | 사람이 발사체 정체성, 방향성, 무기 연계를 검수 | 발사체 단독 4~5뷰, 필요 시 무기와의 크기 비교 보조 |
| Tripo 입력 이미지 | 실제 Tripo 모델 생성 입력 | 발사체 단독 쿼터뷰 + 4뷰 이미지 시트 |

컨셉이미지에는 보조 설명을 넣을 수 있다.

하지만 Tripo 입력 이미지에는 설명, 라벨, 치수선을 넣지 않는다.

### 9.2 선택 보조 자료

컨셉이미지에는 아래 보조 자료를 둘 수 있다.

1. `P0 Roof Turret Cannon` 포구 대비 발사체 크기 비교
2. `CollisionRadius` 시각 기준 원형 가이드
3. `+X Forward` 방향 표시
4. Trail/VFX 연결 후보 위치 표시
5. ImpactEffectId 후보 설명

단, 위 보조 자료는 Tripo 입력 이미지로 사용하지 않는다.

---

## 10. AI 원화 프롬프트 기준

### 10.1 P0 발사체 컨셉이미지 프롬프트

컨셉이미지용 프롬프트는 사람이 검수하기 위한 이미지에 사용한다.

```text
vehicle cannon projectile concept sheet,
P0 large caliber slow cannon projectile,
simplified near-future game asset,
single heavy projectile body,
short thick shell-like projectile,
clear forward direction along +X,
blunt heavy nose,
simple rear surface for trail VFX attachment,
large simple hard-surface form,
low modeling complexity,
AI 3D modeling ready,
one large quarter view,
front view, back view, left view, right view,
plain background,
no shadows,
no cast shadow,
no contact shadow,
no ground shadow,
no gradient shadow,
shadowless lighting,
matte dark gunmetal,
subtle low saturation metal,
no realistic ammunition cutaway,
no real military markings,
no casing and primer detail,
no explosive internals,
no missile fins,
no rocket nozzle,
no smoke trail,
no fire trail,
no explosion,
no vehicle,
no turret,
no text labels in Tripo input version,
no dimension lines,
no complex background
```

### 10.2 Tripo 입력용 발사체 프롬프트

Tripo 입력 이미지용 프롬프트는 실제 모델 생성 입력에 사용한다.

```text
single object only,
vehicle cannon projectile mesh,
large caliber slow cannon projectile,
short thick shell-like body,
clear forward direction along +X,
blunt heavy nose,
simple cylindrical or slightly faceted hard-surface body,
simple rear face for trail VFX attachment,
low modeling complexity,
AI 3D modeling ready,
single image sheet with one large quarter view and one 2x2 orthographic view grid, left large quarter view, right top left front view, right top right back view, right bottom left left view, right bottom right right view,
plain background,
no shadows,
no cast shadow,
no contact shadow,
no ground shadow,
no gradient shadow,
shadowless lighting,
no text,
no callouts,
no dimension lines,
no vehicle,
no turret,
no multiple projectiles,
no ammunition box,
no cutaway,
no casing primer detail,
no missile fins,
no rocket nozzle,
no smoke trail,
no fire trail,
no explosion,
no complex background
```

### 10.3 프롬프트 금지어/주의어

아래 표현은 사용하지 않거나 매우 조심해서 사용한다.

| 표현 | 이유 |
|---|---|
| `real bullet` | 실제 총알처럼 생성될 가능성이 높음 |
| `realistic ammunition` | 실제 탄약 재현 방향으로 흐를 수 있음 |
| `armor piercing round` | 실제 군용 탄종 느낌이 강해질 수 있음 |
| `tank shell` | 전차용 포탄처럼 과대/현실 군용으로 생성될 수 있음 |
| `missile` | 발사체가 로켓/미사일로 오해될 수 있음 |
| `rocket` | 후방 노즐/핀/추진체가 생길 수 있음 |
| `cutaway` | 내부 구조가 드러날 수 있음 |
| `explosive core` | 실제 폭발물 구조처럼 보일 수 있음 |
| `highly detailed markings` | 불필요한 텍스트/문자/군용 마킹이 생길 수 있음 |

---

## 11. 검수 체크리스트

### 11.1 컨셉이미지 검수

컨셉이미지는 아래 질문에 모두 `예`가 나와야 한다.

1. 차량용 대구경 발사체처럼 보이는가?
2. P0 Roof Turret Cannon에서 발사될 법한 크기와 무게감인가?
3. 미사일이나 로켓처럼 보이지 않는가?
4. 실제 탄약 제작물처럼 지나치게 사실적이지 않은가?
5. 전방 `+X` 방향이 명확하게 읽히는가?
6. 짧고 두꺼운 포탄형 실루엣인가?
7. CollisionRadius와 지나치게 괴리되는 길고 얇은 형태가 아닌가?
8. Trail/VFX가 붙을 후방 기준면을 예상할 수 있는가?
9. 피격 VFX와 발사체 본체가 분리되어 있는가?
10. 작은 문자/숫자/라벨/마킹이 과하지 않은가?
11. 25% 축소 상태에서도 진행 방향이 읽히는가?
12. Tripo가 단일 오브젝트로 해석할 수 있을 만큼 단순한가?

### 11.2 Tripo 입력 이미지 검수

Tripo 입력 이미지는 아래 질문에 모두 `예`가 나와야 한다.

1. 발사체 하나만 단독으로 표시되는가?
2. 여러 발사체 묶음이 아닌가?
3. 차량, 터렛, 탄약 상자, 배경 오브젝트가 없는가?
4. Quarter / Front / Back / Left / Right 뷰가 같은 발사체로 일관되는가?
5. Front View가 `+X` 전방 기준을 보여주는가?
6. Back View가 `-X` 후방 기준을 보여주는가?
7. Left/Right View에서 전체 길이와 두께가 명확한가?
8. 텍스트, 치수선, 콜아웃이 없는가?
9. 그림자가 완전히 없는가?
10. 접지 그림자, 그라데이션 그림자, 강한 명암 음영이 모두 없는가?
11. 조명이 shadowless lighting 기준으로 균일한가?
12. 배경이 단색 또는 매우 단순한가?
13. 메시가 너무 얇거나 길게 보이지 않는가?
14. 미사일 핀, 로켓 노즐, 실제 탄약 단면이 없는가?

### 11.3 Tripo 메시 생성 후 검수

Tripo 결과물은 아래 기준으로 검수한다.

1. 단일 StaticMesh로 사용할 수 있는가?
2. 메시 전방이 `+X`로 정리 가능한가?
3. 중심 피벗을 Actor 원점에 맞출 수 있는가?
4. `ProjectileMeshRelativeRotation = 0,0,0`에 가깝게 사용할 수 있는가?
5. `ProjectileMeshRelativeScale`로 크기만 조정하면 되는가?
6. `CollisionRadius`와 시각 크기가 크게 어긋나지 않는가?
7. 너무 얇은 파츠가 없어 자동 정리 중 깨질 가능성이 낮은가?
8. 불필요한 내부 구조나 작은 부품이 없는가?
9. VFX로 처리할 Trail/Fire/Explosion이 메시로 붙어 있지 않은가?
10. UE `ProjectileStaticMesh`에 직접 넣을 수 있는가?

### 11.4 UE 적용 전 검수

UE 적용 전에는 아래를 확인한다.

1. 메시가 `ProjectileStaticMesh`에 들어갈 단일 StaticMesh인가?
2. 발사체 진행 방향이 `+X`인가?
3. 메시 중심이 Actor 원점과 맞는가?
4. 필요하면 `ProjectileMeshRelativeRotation`으로 단순 보정 가능한가?
5. 필요하면 `ProjectileMeshRelativeScale`로 단순 보정 가능한가?
6. `CollisionRadius` 값과 시각 크기가 납득 가능한가?
7. `InitialSpeed`, `GravityScale`, `LifeTimeSeconds`로 표현할 전투 감각과 외형이 충돌하지 않는가?
8. `ImpactEffectId`와 연결될 피격 표현을 메시 안에 과하게 만들지 않았는가?
9. 게임 카메라에서 발사체 방향을 읽을 수 있는가?
10. 성능상 너무 복잡한 메시가 아닌가?

---

## 12. 금지 기준

아래 조건 중 하나라도 강하게 해당하면 P0 발사체 원화 후보에서 제외한다.

1. 실제 총알/포탄을 그대로 복제한 것처럼 보임
2. 실제 탄약 내부 구조가 드러남
3. 탄피, 뇌관, 장약, 내부 폭발물 구조가 세부 표현됨
4. 미사일/로켓처럼 보임
5. 긴 관통탄/날탄처럼 너무 얇고 긺
6. 후방 날개/핀/노즐이 과도함
7. 발사체가 아니라 VFX 덩어리처럼 보임
8. 연기/불꽃/폭발이 메시 일부로 붙어 있음
9. 여러 발사체가 한 덩어리로 묶여 있음
10. 텍스트/라벨/마킹이 외형을 지배함
11. 그림자나 접지면이 Tripo 입력 이미지에 들어 있음
12. 방향성이 불명확함
13. CollisionRadius와 시각 크기가 크게 충돌함
14. Blender 수동 모델링 없이는 정리하기 어려움

---

## 13. 현재 구현과 문서 기준의 차이

현재 프로젝트에는 이미 발사체 Actor와 데이터 구조가 있다.

확인된 현재 구조는 다음과 같다.

```text
UCFProjectileData
  ProjectileStaticMesh
  ProjectileMeshRelativeRotation
  ProjectileMeshRelativeScale
  CollisionRadius
  InitialSpeed
  bAffectedByGravity
  GravityScale
  LifeTimeSeconds
  ImpactEffectId
  DefaultDamageData

ACFProjectileActor / BP_CFProjectile
  CollisionComponent = SphereComponent
  MeshComponent = StaticMeshComponent
  ProjectileMovementComponent
```

현재 프로젝트에는 `/Game/CarFight/Weapons/Projectiles/Bullet/Mesh_Bullet` 계열 메시와 `DA_DummyProjectile`이 존재한다.

다만 이 문서는 기존 임시 Bullet 메시를 최종 기준으로 고정하지 않는다.

P0 기준에서는 `P0 Roof Turret Cannon`과 어울리는 짧고 두꺼운 대구경 포탄형 발사체를 새 기준으로 삼는다.

---

## 14. 결정 사항 요약

현재 확정 기준은 다음과 같다.

1. 발사체 원화 문서는 `18_ProjectileArtRules.md`로 관리한다.
2. 첫 대상은 `P0 Large-Caliber Slow Cannon Projectile`로 한다.
3. 한국어 명칭은 `P0 대구경 저속 포탄형 발사체`로 한다.
4. 발사체 메시는 `ProjectileStaticMesh`에 들어갈 단일 StaticMesh로 만든다.
5. 발사체 Actor 구조는 `CollisionComponent`, `MeshComponent`, `ProjectileMovementComponent`를 기준으로 한다.
6. 발사체 메시는 시각 표시용이고, 실제 충돌은 `CollisionRadius` 기반 SphereComponent가 담당한다.
7. 발사체 진행 방향은 `+X`로 둔다.
8. P0 기본 피벗은 중심 피벗으로 둔다.
9. `ProjectileMeshRelativeRotation = 0,0,0`에 가깝게 쓸 수 있도록 메시 자체를 정리한다.
10. 발사체는 짧고 두꺼운 포탄형 실루엣을 우선한다.
11. 미사일, 로켓, 실제 탄약, 탄약 단면도, 실제 군용 마킹처럼 보이는 형태는 금지한다.
12. Trail, Fire, Explosion, Impact는 메시가 아니라 VFX로 분리한다.
13. Tripo 입력 이미지는 발사체 단일 오브젝트 쿼터뷰 + 4뷰 이미지 시트 1장으로 준비한다.
14. Tripo 입력 이미지에는 텍스트, 치수선, 콜아웃, 차량, 터렛, 탄약 상자를 넣지 않는다.
15. Tripo 입력 이미지에는 모든 종류의 그림자를 넣지 않는다.
16. Tripo 입력 이미지는 `shadowless lighting` 기준의 균일 조명을 사용한다.
17. Blender 수동 모델링은 기본 파이프라인에 포함하지 않는다.
18. Blender는 스크립트 기반 정리와 Export용 보조 도구로만 사용한다.

---

## 15. Changelog

### v0.1 - 2026-07-08

- P0 발사체 원화 기준 문서 신규 작성.
- 첫 제작 대상을 `P0 Large-Caliber Slow Cannon Projectile`로 정의.
- 현재 `UCFProjectileData`와 `ACFProjectileActor` 구조를 문서 기준에 반영.
- `ProjectileStaticMesh`, `ProjectileMeshRelativeRotation`, `ProjectileMeshRelativeScale`, `CollisionRadius` 기준 추가.
- 발사체 진행 방향 `+X`, 중심 피벗, 단일 StaticMesh 기준 정의.
- 발사체 메시와 충돌/이동/Damage/VFX 역할 분리 기준 추가.
- Tripo 입력 이미지 기준과 그림자 금지 기준 추가.
- 실제 탄약/미사일/로켓/내부 구조 재현 금지 기준 추가.

---

## 16. Migration

### 기존 문서와의 관계

이 문서는 기존 ConceptArt 문서를 교체하지 않는다.

기존 문서의 역할은 그대로 유지한다.

| 기존 문서 | 유지 역할 |
|---|---|
| `16_WeaponArtRules.md` | 무기 본체/터렛 메시 기준 |
| `17_TripoInputImageRules.md` | Tripo 입력 이미지 공통 규칙 |

발사체 메시를 만들 때만 이 문서를 사용한다.

```text
터렛 무기 원화:
16_WeaponArtRules 기준

Tripo 입력 이미지 공통 규칙:
17_TripoInputImageRules 기준

발사체 메시 원화:
18_ProjectileArtRules 기준
```

### 현재 구현 적용 기준

현재 구현은 이미 `ProjectileStaticMesh`를 데이터로 받아 `MeshComponent`에 적용한다.

따라서 이 문서 도입만으로 C++ 구조를 즉시 바꿀 필요는 없다.

단, 신규 발사체 메시를 만들 때는 아래 기준을 따른다.

```text
StaticMesh forward = +X
Pivot = center
Mesh purpose = visual only
Collision = CollisionRadius SphereComponent
Movement = ProjectileMovementComponent
Damage = DefaultDamageData
Impact = ImpactEffectId / VFX
```

### 기존 임시 메시 처리

기존 `/Game/CarFight/Weapons/Projectiles/Bullet/Mesh_Bullet` 계열 메시가 있더라도, 이 문서는 해당 메시를 최종 P0 기준으로 고정하지 않는다.

기존 메시는 테스트용 또는 임시 기준으로 유지할 수 있다.

P0 Roof Turret Cannon용 발사체를 새로 만들 경우, 이 문서 기준의 짧고 두꺼운 대구경 포탄형 발사체를 우선한다.
