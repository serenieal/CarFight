# MeshPaintPrepTool - Design

> 역할: `MeshPaintPrepTool`의 플러그인 구조, 에디터 UI, 에셋 생성 흐름, UV 처리 방식을 정의한다.
> 문서 버전: v0.3
> 마지막 정리(Asia/Seoul): 2026-06-30
> 상태: Draft / Design

---

## 1. 설계 원칙

1. CarFight 전용 코드를 넣지 않는다.
2. 대상은 모든 `Static Mesh`다.
3. Editor-only 플러그인으로 시작한다.
4. 사용자가 직접 메시 페인트(`Mesh Paint`)를 수행할 수 있도록 준비까지만 담당한다.
5. UV가 없거나 부적절한 임시 메시를 우선 지원한다.
6. 실패하면 조용히 넘어가지 않고 이유를 표시한다.
7. v1.0은 한 번에 하나의 메시를 안정적으로 처리한다.

---

## 2. 플러그인 구조

예상 저장소 구조:

```text
MeshPaintPrepTool/
  MeshPaintPrepTool.uplugin
  Source/
    MeshPaintPrepToolEditor/
      MeshPaintPrepToolEditor.Build.cs
      Public/
      Private/
  Content/
    Materials/
      M_PaintableMesh.uasset
  Docs/
    README.md
```

CarFight 서브모듈 위치:

```text
D:\Work\CarFight_git\UE\Plugins\MeshPaintPrepTool
```

모듈 방향:

| 모듈 | 타입 | 역할 |
|---|---|---|
| `MeshPaintPrepToolEditor` | Editor | UI, 에셋 생성, UV 검사, Auto UV 실행 |
| Runtime 모듈 | 없음 | v1.0에서는 런타임 코드 불필요 |

---

## 3. 에디터 진입점

v1.0 후보 진입점:

| 진입점 | 우선순위 | 설명 |
|---|---:|---|
| 메뉴 패널 | 1 | `창(Window)` 메뉴에서 툴 탭 열기 |
| 콘텐츠 브라우저 우클릭 | 2 | Static Mesh 에셋에서 바로 실행 |
| 레벨 선택 액터 | 3 | 배치된 StaticMeshActor에서 원본 Static Mesh 추출 |

v1.0 최소안:

```text
콘텐츠 브라우저에서 Static Mesh 선택
  -> Mesh Paint Prep 패널 열기
  -> 선택 메시 자동 입력
  -> 준비 실행
```

---

## 4. 처리 흐름

기본 실행 흐름:

```text
1. 대상 Static Mesh 확인
2. 대상 머티리얼 슬롯 확인
3. UV 채널 검사
4. 필요 시 Auto UV 생성
5. 출력 폴더 계산
6. 빈 Texture2D 생성
7. Material Instance 생성
8. Material Instance 파라미터 설정
9. Static Mesh 슬롯에 Material Instance 적용
10. Mesh Paint 관련 설정 저장
11. 변경 에셋 저장
12. 결과 알림 표시
```

실패 시 중단 기준:

| 단계 | 실패 예시 | 처리 |
|---|---|---|
| 대상 확인 | Static Mesh가 아님 | 실행 중단 |
| UV 검사 | UV 없음 + Auto UV 실패 | 실행 중단 |
| 텍스처 생성 | 같은 이름 존재 + 덮어쓰기 거부 | 실행 중단 |
| 머티리얼 생성 | 부모 머티리얼 누락 | 실행 중단 |
| 슬롯 적용 | 슬롯 인덱스 잘못됨 | 실행 중단 |
| 저장 | 에셋 저장 실패 | 경고 표시 |

---

## 5. UV 설계

### 5.1 기본 정책

v1.0 기본 UV 채널은 `UV0`이다.

이유:

1. Unreal 머티리얼의 기본 텍스처 샘플은 별도 지정이 없으면 UV0을 사용한다.
2. 에디터에서 만든 임시 메시에는 보존해야 할 고급 UV가 없는 경우가 많다.
3. 사용자가 Mesh Paint에서 보는 결과와 머티리얼 결과가 일치하기 쉽다.

### 5.2 Auto UV 정책

v0.4 구현:

| 옵션 | 설명 |
|---|---|
| `Auto UV 생성` | 선택한 Static Mesh의 LOD0 / UV0에 박스 기반 UV를 생성 |
| 사용 API | `StaticMeshEditorSubsystem`의 `GenerateBoxUVChannel` |
| 저장 대상 | 원본 Static Mesh 에셋 |

주의:

1. Auto UV는 페인트 좌표를 만들어 주는 도구다.
2. Auto UV가 아트 품질의 수동 UV를 보장하지는 않는다.
3. 차량 외장 래핑처럼 정교한 도색이 필요하면 별도 UV 제작이 필요할 수 있다.
4. 박스 / 실린더 조합 프로토타입에는 충분히 유용하다.
5. v0.4 Auto UV는 모델링 모드의 XAtlas / Patch 기반 고급 언랩이 아니라 박스 투영 기반이다.

### 5.3 UV1 지원

UV1 이상은 v1.0 기본 범위에서 제외한다.

후속 조건:

1. 기존 UV0을 보존해야 하는 프로젝트 사례가 생긴다.
2. 부모 머티리얼이 선택 UV 채널을 명시적으로 샘플링할 수 있어야 한다.
3. Mesh Paint의 Texture Coordinate Index와 머티리얼 Texture Coordinate 설정이 함께 맞아야 한다.

---

## 6. 머티리얼 설계

플러그인 기본 부모 머티리얼:

```text
/MeshPaintPrepTool/Materials/M_PaintableMesh
```

권장 파라미터:

| 파라미터 | 타입 | 기본값 | 역할 |
|---|---|---|---|
| `BasePaintColor` | Vector | 밝은 회색 | 텍스처가 없거나 비활성일 때 기본색 |
| `BasePaintTexture` | Texture2D | 기본 흰 텍스처 | 사용자가 칠할 페인트 텍스처 |
| `UsePaintTexture` | Scalar | 1.0 | 텍스처 사용 여부 |
| `RoughnessValue` | Scalar | 0.45 | 표면 거칠기 |
| `MetallicValue` | Scalar | 0.0 | 금속성 |

중요:

1. `BasePaintTexture` 샘플러 타입은 일반 Color 텍스처 기준이어야 한다.
2. Virtual Texture 전용 샘플러로 만들지 않는다.
3. 머티리얼 결과는 Mesh Paint 모드 밖에서도 보여야 한다.

---

## 7. 에셋 생성 설계

입력 메시:

```text
/Game/Props/Turret/Mesh_Turret
```

출력 폴더:

```text
/Game/Props/Turret/Paint
```

출력 에셋:

```text
T_Mesh_Turret_Paint
MI_Mesh_Turret_Paint
```

이름 충돌 처리:

| 상황 | 기본 처리 |
|---|---|
| 같은 텍스처 있음 | 사용자 확인 필요 |
| 같은 MI 있음 | 사용자 확인 필요 |
| 부모 머티리얼 없음 | 플러그인 콘텐츠 누락 오류 |
| 출력 폴더 없음 | 생성 |

---

## 8. 메시 설정 설계

툴은 대상 `Static Mesh`에 아래 상태를 맞춘다.

| 항목 | 기본값 | 설명 |
|---|---|---|
| Paint Support | Enabled | Texture Color Mesh Painting 준비 |
| Paint Texture Coordinate Index | 0 | UV0 기준 |
| Paint Texture Resolution | 선택 해상도 | 기본 1024, 작은 메시용 256 / 512 선택 가능 |
| Material Slot | 선택 슬롯 | 기본 0번 |

실제 Unreal 내부 속성명은 엔진 버전별로 노출 방식이 다를 수 있으므로 구현 시 프로젝트 엔진 소스 기준으로 재확인한다.

---

## 9. 에러 메시지 원칙

사용자가 이해해야 하는 메시지는 한글로 표시한다.

예시:

| 상황 | 메시지 후보 |
|---|---|
| 대상 없음 | "Static Mesh를 먼저 선택하세요." |
| UV 없음 | "선택한 메시에는 사용할 UV가 없습니다. Auto UV를 생성하세요." |
| Auto UV 실패 | "Auto UV 생성에 실패했습니다. 모델링 모드에서 UV를 직접 확인하세요." |
| 부모 머티리얼 누락 | "플러그인 기본 머티리얼 M_PaintableMesh를 찾을 수 없습니다." |
| 저장 실패 | "생성된 에셋 저장에 실패했습니다. 콘텐츠 폴더 권한과 체크아웃 상태를 확인하세요." |

---

## 10. 구현 전 확인 필요

아래 항목은 코드 작성 전에 엔진 소스에서 확인한다.

| 항목 | 확인 이유 |
|---|---|
| Auto UV를 C++에서 호출할 안정 API | 확인 완료: v0.4는 `StaticMeshEditorSubsystem::GenerateBoxUVChannel` 사용 |
| Static Mesh Paint 설정 접근 방법 | 확인 완료: `StaticMeshPaintSupport`, `MeshPaintTextureCoordinateIndex`, `MeshPaintTextureResolution` 사용 |
| Texture2D 빈 에셋 생성 방식 | 확인 완료: `Texture2D::Source.Init`으로 흰색 BGRA8 텍스처 생성 |
| 플러그인 콘텐츠 머티리얼 로딩 경로 | 보류: v0.4는 플러그인 콘텐츠 부모 머티리얼 대신 에셋별 Material을 코드로 생성 |

---

## 11. Changelog

### v0.3

- v0.4 구현 결과에 맞춰 Auto UV 정책을 박스 기반 UV0 생성으로 구체화했다.
- 모델링 모드 고급 Auto UV와 현재 구현의 차이를 명시했다.
- Auto UV 저장 대상이 원본 Static Mesh 에셋임을 기록했다.

### v0.2

- 페인트 텍스처 해상도를 고정값이 아니라 선택값으로 명시했다.
- 기본값은 1024로 유지하되 총알 같은 작은 메시에는 256 / 512를 사용할 수 있게 설계에 반영했다.

### v0.1

- Editor-only 플러그인 구조를 정의했다.
- UV0 기본 정책과 Auto UV 옵션을 설계에 포함했다.
- 부모 머티리얼 `M_PaintableMesh` 파라미터 초안을 정의했다.
- 에셋 생성 위치와 이름 규칙을 정의했다.

---

## 12. Migration 메모

- 기존 `M_CarPaintSkin`은 새 플러그인의 직접 부모가 아니다.
- 기존 머티리얼에서 검증된 `BasePaintTexture`, `UsePaintTexture`, `RoughnessValue`, `MetallicValue` 개념만 가져온다.
- CarFight 프로젝트의 차량 DataAsset 또는 스폰 시스템은 플러그인 코드에서 참조하지 않는다.
