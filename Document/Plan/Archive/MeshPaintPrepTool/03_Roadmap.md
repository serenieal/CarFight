# MeshPaintPrepTool - Roadmap

> 역할: 문서 작성 이후 플러그인 구현, 검증, 서브모듈 운영까지의 단계별 계획을 정의한다.
> 문서 버전: v0.6
> 마지막 정리(Asia/Seoul): 2026-06-30
> 상태: Draft / Roadmap

---

## 1. 전체 단계

```text
M0. 문서 기준선 작성
M1. 별도 Git 저장소 생성
M2. 플러그인 스켈레톤 생성
M3. 기본 머티리얼 콘텐츠 추가
M4. 단일 Static Mesh 준비 기능 구현
M5. Auto UV 통합
M6. CarFight 서브모듈 연결
M7. 터렛 / 총알 / 차량 검증
M8. 배치 처리와 UV1 확장 검토
```

---

## 2. M0. 문서 기준선 작성

목표:

```text
구현 전에 범위, 설계, 결정 사항, 검증 기준을 고정한다.
```

작업:

1. `Document/Plan/MeshPaintPrepTool` 폴더 생성.
2. Overview / Requirements / Design / Roadmap / Decisions / Validation / RepoWorkflow 문서 작성.
3. 사용자 결정 필요 항목을 `Open Question`으로 분리.

완료 기준:

1. 차량 전용이 아닌 범용 Static Mesh 도구임이 문서에 명시되어 있다.
2. 별도 Git 저장소 + CarFight 서브모듈 방식이 문서에 명시되어 있다.
3. v1.0 범위와 비범위가 분리되어 있다.

---

## 3. M1. 별도 Git 저장소 생성

목표:

```text
MeshPaintPrepTool의 원본 저장소를 CarFight 밖에 만든다.
```

사용자 결정 상태:

| 항목 | 상태 |
|---|---|
| 원격 저장소 URL | 확정: `https://github.com/serenieal/MeshPaintPrepTool` |
| 공개 / 비공개 여부 | 확정: 비공개 |
| 라이선스 | 확정: 현재 만들지 않음 |

작업:

1. 저장소 이름을 `MeshPaintPrepTool`로 만든다.
2. README를 추가한다.
3. 초기 브랜치 정책을 정한다.
4. Unreal 플러그인 폴더 구조를 저장소 루트에 둔다.

완료 기준:

1. 별도 Git 저장소가 존재한다.
2. 원격 저장소에 초기 커밋이 올라가 있다.
3. CarFight 프로젝트 없이도 플러그인 소스 폴더 구조를 볼 수 있다.

현재 상태:

1. GitHub 비공개 원격 저장소는 생성 완료됐다.
2. 플러그인 초기 커밋 `27d400f16e8bb46c51eea064196b23f8ab7150b9`이 `main` 브랜치에 push됐다.
3. CarFight 프로젝트에 `UE/Plugins/MeshPaintPrepTool` 서브모듈 추가가 완료됐다.

---

## 4. M2. 플러그인 스켈레톤 생성

목표:

```text
Unreal Editor에서 로드되는 Editor-only 플러그인을 만든다.
```

작업:

1. `MeshPaintPrepTool.uplugin` 생성.
2. `MeshPaintPrepToolEditor` 모듈 생성.
3. 기본 시작 / 종료 모듈 코드를 만든다.
4. 메뉴 또는 탭 등록 골격을 만든다.

완료 기준:

1. CarFight에 플러그인을 붙였을 때 에디터가 로드된다.
2. `Tools\BuildEditor.bat` 빌드가 통과한다.
3. 플러그인 활성화 상태를 확인할 수 있다.

현재 상태:

1. `.uplugin`과 `MeshPaintPrepToolEditor` 모듈 스켈레톤이 생성됐다.
2. `UE/CarFight_Re.uproject`에 `MeshPaintPrepTool` 활성화 항목을 추가했다.
3. `Tools\BuildEditor.bat` 빌드가 통과했다.
4. `Mesh Paint Prep` 에디터 탭과 메뉴 진입점이 구현됐다.
5. 콘텐츠 브라우저 / 레벨 선택에서 Static Mesh를 읽는 기반이 구현됐다.
6. v0.2 플러그인 커밋은 `70deb85e710dbd38e4ed9942a00091894ad0b0c4`다.

---

## 5. M3. 기본 머티리얼 콘텐츠 추가

목표:

```text
모든 프로젝트에서 공통으로 쓸 페인트 부모 머티리얼을 플러그인 콘텐츠로 제공한다.
```

작업:

1. `M_PaintableMesh` 생성.
2. `BasePaintTexture`, `BasePaintColor`, `UsePaintTexture`, `RoughnessValue`, `MetallicValue` 파라미터를 만든다.
3. 일반 Texture2D Color 샘플러를 사용한다.
4. 기본 텍스처가 Virtual Texture 전용이 아닌지 확인한다.

완료 기준:

1. Material Instance에서 페인트 텍스처를 지정하면 메시 페인트 모드 밖에서도 색이 보인다.
2. Virtual Texture 샘플러 오류가 없다.

---

## 6. M4. 단일 Static Mesh 준비 기능 구현

목표:

```text
선택한 Static Mesh 하나를 페인트 준비 상태로 만든다.
```

작업:

1. 선택한 Static Mesh 가져오기.
2. 출력 폴더 계산.
3. 빈 Texture2D 생성.
4. Material Instance 생성.
5. 파라미터 연결.
6. 메시 슬롯에 MI 적용.
7. Static Mesh와 생성 에셋 저장.

완료 기준:

1. 박스 메시 하나를 선택해 준비할 수 있다.
2. `Paint` 폴더에 텍스처와 MI가 생성된다.
3. 메시 슬롯에 생성된 MI가 들어간다.
4. Mesh Paint에서 칠한 결과가 일반 에디터 모드에서도 보인다.

현재 상태:

1. 대상 Static Mesh 선택 기반은 구현됐다.
2. 해상도 프리셋 선택 기반은 구현됐다.
3. 머티리얼 슬롯 인덱스 선택 기반은 구현됐다.
4. UV 채널 수와 머티리얼 슬롯 수 검사는 구현됐다.
5. `Paint` 출력 폴더 아래 빈 Texture2D 생성이 구현됐다.
6. 페인트 텍스처를 BaseColor로 읽는 Material 생성이 구현됐다.
7. Material Instance 생성과 `BasePaintTexture` 파라미터 지정이 구현됐다.
8. 선택한 Static Mesh 슬롯에 Material Instance 적용이 구현됐다.
9. Static Mesh의 `Support Texture Color Mesh Painting`, UV0, 선택 해상도 설정이 구현됐다.
10. 기존 에셋 재사용 / 덮어쓰기 UI는 아직 남은 작업이다.
11. v0.3 플러그인 커밋은 `cbb9e70e9bbb13234749ffc8b1f411c007d8d0d3`다.

---

## 7. M5. Auto UV 통합

목표:

```text
UV가 없는 임시 메시를 툴 안에서 바로 페인트 가능하게 만든다.
```

작업:

1. UV 채널 검사.
2. 사용자 선택 시 `Auto UV 생성` 실행.
3. 생성 후 Static Mesh 저장.
4. UV 실패 시 명확한 오류 표시.

완료 기준:

1. 모델링 모드에서 만든 단순 박스 메시가 Auto UV 후 페인트 가능하다.
2. 실린더형 총알 메시가 Auto UV 후 페인트 가능하다.
3. 박스 + 실린더 터렛 메시가 Auto UV 후 페인트 가능하다.

현재 상태:

1. `Auto UV 생성` 버튼이 구현됐다.
2. LOD0 / UV0에 박스 기반 UV를 생성한다.
3. 생성 후 Static Mesh 저장을 시도한다.
4. `Tools\BuildEditor.bat` 빌드가 통과했다.
5. XAtlas / Patch 기반 고급 Auto UV는 후속 확장으로 남겼다.
6. v0.4 플러그인 커밋은 `1ea2ff0a56c98c2a3e8a269adc15e7a2929e9024`다.

---

## 8. M6. CarFight 서브모듈 연결

목표:

```text
CarFight 프로젝트가 별도 저장소의 플러그인을 서브모듈로 사용한다.
```

작업:

1. CarFight에서 `UE/Plugins/MeshPaintPrepTool` 경로로 서브모듈 추가.
2. `.gitmodules` 확인.
3. 플러그인 활성화.
4. `Tools\BuildEditor.bat` 실행.

완료 기준:

1. CarFight 메인 저장소에는 플러그인 코드가 직접 복사되지 않는다.
2. `UE/Plugins/MeshPaintPrepTool`은 서브모듈로 표시된다.
3. CarFight 빌드가 통과한다.

---

## 9. M7. 검증 대상

CarFight에서 아래 메시로 검증한다.

| 대상 | 목적 |
|---|---|
| 박스 임시 메시 | 가장 단순한 Static Mesh 검증 |
| 실린더 총알 메시 | 원형 단면 UV 검증 |
| 박스 + 실린더 터렛 | 복합 프리미티브 메시 검증 |
| `Mesh_TestSedan` | 차량 차체 검증 |
| `Mesh_TestSUV` | 다른 차체 메시 검증 |

완료 기준:

1. 모든 대상에서 페인트가 Mesh Paint 모드 밖에서도 보인다.
2. 생성된 텍스처를 저장 후 에디터를 재시작해도 유지된다.
3. 스폰된 액터가 같은 Static Mesh와 MI를 쓰면 페인트가 보인다.

---

## 10. M8. 후속 확장

후속 후보:

| 기능 | 시점 |
|---|---|
| 여러 Static Mesh 일괄 처리 | v1.1 이후 |
| UV1 선택 지원 | v1.1 이후 |
| UV 겹침 / 스트레칭 경고 | v1.2 이후 |
| 머티리얼 슬롯별 다중 텍스처 생성 | v1.2 이후 |
| 프로젝트별 프리셋 저장 | v1.3 이후 |
| 플러그인 릴리즈 패키징 | v1.0 안정화 후 |

---

## 11. Changelog

### v0.6

- M5 Auto UV 통합의 v0.4 구현 상태를 반영했다.
- Auto UV의 현재 범위를 박스 기반 UV0 생성으로 명시했다.
- 고급 XAtlas / Patch 기반 Auto UV는 후속 확장으로 분리했다.
- v0.4 플러그인 커밋 `1ea2ff0a56c98c2a3e8a269adc15e7a2929e9024`를 기록했다.

### v0.5

- `Prepare` 버튼 기반 기본 페인트 에셋 생성 흐름 구현 상태를 반영했다.
- Texture2D / Material / Material Instance 생성과 Static Mesh 슬롯 적용 완료 상태를 반영했다.
- Static Mesh Mesh Painting 설정 적용 완료 상태를 반영했다.
- Auto UV와 기존 에셋 재사용 / 덮어쓰기 UI는 남은 작업으로 유지했다.
- v0.3 플러그인 커밋 `cbb9e70e9bbb13234749ffc8b1f411c007d8d0d3`를 기록했다.

### v0.4

- `Mesh Paint Prep` 에디터 탭과 `창(Window)` 메뉴 진입점 구현 상태를 반영했다.
- 콘텐츠 브라우저 / 레벨 선택 Static Mesh 읽기 기반 구현 상태를 반영했다.
- 해상도 프리셋, 슬롯 인덱스, UV 채널 / 머티리얼 슬롯 검사 구현 상태를 반영했다.
- Texture2D / Material Instance 생성은 아직 남은 작업으로 분리했다.
- v0.2 플러그인 커밋 `70deb85e710dbd38e4ed9942a00091894ad0b0c4`를 기록했다.

### v0.3

- 플러그인 초기 커밋과 CarFight 서브모듈 추가 완료 상태를 반영했다.
- `MeshPaintPrepToolEditor` 스켈레톤 빌드 통과 상태를 반영했다.
- 다음 남은 작업을 에디터 패널과 실제 준비 기능 구현으로 좁혔다.

### v0.2

- M1 원격 저장소 URL과 비공개 상태를 확정했다.
- 라이선스는 현재 만들지 않는다고 기록했다.
- 원격 저장소는 생성됐지만 초기 커밋과 서브모듈 추가는 아직 남은 작업으로 분리했다.

### v0.1

- 문서 작성부터 서브모듈 검증까지의 로드맵을 정의했다.
- Auto UV 통합을 별도 마일스톤으로 분리했다.
- CarFight 검증 대상을 차량, 터렛, 총알, 임시 메시로 확장했다.

---

## 12. Migration 메모

- CarFight 내부 모듈에 먼저 구현한 뒤 플러그인으로 옮기는 방식은 기본안이 아니다.
- 플러그인 원본 저장소를 먼저 만들고, CarFight는 서브모듈로 받아 검증한다.
- CarFight 전용 검증 결과는 플러그인 요구사항으로 승격하기 전에 일반화 여부를 확인한다.
