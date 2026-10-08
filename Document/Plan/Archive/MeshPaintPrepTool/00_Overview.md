# MeshPaintPrepTool - Overview

> 역할: 에디터에서 만든 Static Mesh를 텍스처 페인트 가능 상태로 준비하는 범용 Unreal Editor 플러그인 계획이다.
> 문서 버전: v0.4
> 마지막 정리(Asia/Seoul): 2026-06-29
> 상태: Draft / Plugin Repo Created / Submodule Added / Build Verified

---

## 1. 목적

`MeshPaintPrepTool`은 자동차 전용 도구가 아니다.

목적은 Unreal Editor 안에서 박스, 실린더, 콘, 임시 메시 등을 이어붙여 만든 `Static Mesh`를 빠르게 아래 상태로 만드는 것이다.

```text
선택한 Static Mesh
  -> UV 검사 또는 Auto UV 생성
  -> 빈 페인트 텍스처 생성
  -> 페인트용 머티리얼 인스턴스 생성
  -> 메시 슬롯에 머티리얼 적용
  -> Mesh Paint의 Texture Color Painting으로 바로 색칠 가능
```

대상 예시:

| 대상 | 포함 여부 | 비고 |
|---|---:|---|
| 자동차 차체 | 포함 | 현재 CarFight 테스트 차량 포함 |
| 터렛 | 포함 | 박스 / 실린더 조합 메시 가능 |
| 총알 / 탄환 | 포함 | 실린더 / 캡슐형 임시 메시 가능 |
| 임시 프롭 | 포함 | 에디터 제작 Static Mesh 가능 |
| Skeletal Mesh | 제외 | v1.0 비범위 |
| Landscape | 제외 | v1.0 비범위 |

---

## 2. 핵심 결정

| ID | 결정 | 상태 |
|---|---|---|
| D001 | 도구 대상은 차량이 아니라 범용 `Static Mesh`다. | 확정 |
| D002 | 플러그인은 별도 Git 저장소로 관리한다. | 확정 |
| D003 | CarFight 프로젝트에서는 `UE/Plugins/MeshPaintPrepTool` 서브모듈로 사용한다. | 확정 |
| D004 | v1.0부터 Auto UV 옵션을 포함한다. | 확정 |
| D005 | 기본 UV 채널은 `UV0`으로 시작한다. | 확정 |
| D006 | 생성 에셋은 선택한 메시 폴더 아래 `Paint` 하위 폴더에 만든다. | 확정 |
| D007 | 플러그인은 Editor-only로 시작한다. | 확정 |

---

## 3. 문서 경계

이 폴더는 CarFight 프로젝트 안의 작업 계획 문서다.

플러그인 원본 코드는 별도 저장소에 둔다.

```text
별도 Git 저장소:
  https://github.com/serenieal/MeshPaintPrepTool

CarFight 내 사용 위치:
  D:\Work\CarFight_git\UE\Plugins\MeshPaintPrepTool

관리 방식:
  Git Submodule
```

중요:

1. CarFight는 플러그인을 소비하는 프로젝트다.
2. 플러그인 구현의 원본은 별도 Git 저장소다.
3. CarFight 전용 경로, 에셋명, 게임 규칙을 플러그인 코드에 하드코딩하지 않는다.
4. CarFight 문서는 사용 사례와 검증 프로젝트 역할만 한다.

---

## 4. 추천 읽기 순서

새 AI 세션은 아래 순서로 읽는다.

```text
01. 00_Overview.md
02. 01_Requirements.md
03. 04_Decisions.md
04. 02_Design.md
05. 03_Roadmap.md
06. 05_Validation.md
07. 06_RepoWorkflow.md
```

이 순서를 권장하는 이유:

| 순서 | 이유 |
|---|---|
| Overview | 차량 전용이 아니라 범용 Static Mesh 도구임을 먼저 고정 |
| Requirements | v1.0 필수 기능과 비범위를 분리 |
| Decisions | 확정 사항과 사용자 결정 필요 항목을 확인 |
| Design | 플러그인 구조와 에셋 생성 흐름을 확인 |
| Roadmap | 구현 순서와 중단 기준을 확인 |
| Validation | 에디터 / 빌드 / 페인트 검증 기준 확인 |
| RepoWorkflow | 별도 Git 저장소와 CarFight 서브모듈 운영 방식 확인 |

---

## 5. 이번 문서 세션 범위

이번 세션에서 하는 작업:

1. 플러그인 목표를 차량 전용에서 범용 Static Mesh 도구로 정리한다.
2. 별도 Git 저장소와 CarFight 서브모듈 운영 방식을 문서화한다.
3. v1.0 요구사항, 설계, 로드맵, 검증 기준을 만든다.
4. 사용자 결정이 필요한 항목을 `Open Question`으로 분리한다.

이번 세션에서 하지 않는 작업:

| 항목 | 처리 |
|---|---|
| 플러그인 코드 생성 | 하지 않음 |
| `.uplugin` 생성 | 하지 않음 |
| 별도 Git 저장소 생성 | 하지 않음 |
| GitHub 업로드 | 하지 않음 |
| CarFight 서브모듈 추가 | 하지 않음 |
| Unreal Editor 실행 | 하지 않음 |
| `.uasset` 생성 / 수정 | 하지 않음 |

---

## 6. Changelog

### v0.4

- v0.3 플러그인 커밋 `cbb9e70e9bbb13234749ffc8b1f411c007d8d0d3` 생성 완료를 반영했다.
- 기본 페인트 에셋 생성 흐름을 구현 상태로 반영했다.
- `Prepare` 실행 시 Texture2D, Material, Material Instance 생성과 Static Mesh 슬롯 적용을 수행한다고 기록했다.
- Auto UV는 아직 남은 작업으로 유지했다.

### v0.3

- 플러그인 원격 저장소 초기 커밋 `27d400f16e8bb46c51eea064196b23f8ab7150b9` 생성 완료를 반영했다.
- CarFight 프로젝트에 `UE/Plugins/MeshPaintPrepTool` 서브모듈 추가 완료를 반영했다.
- `Tools\BuildEditor.bat` 빌드 통과 상태를 반영했다.

### v0.2

- 별도 Git 원격 저장소를 `https://github.com/serenieal/MeshPaintPrepTool`로 확정했다.
- 저장소 공개 범위를 비공개로 확정했다.
- 라이선스는 현재 개인 사용 전제로 만들지 않는다고 기록했다.

### v0.1

- `MeshPaintPrepTool` 작업 문서 폴더를 신규 정의했다.
- 도구 범위를 차량 전용이 아닌 범용 `Static Mesh` 페인트 준비 도구로 고정했다.
- 별도 Git 저장소와 CarFight 서브모듈 운영 방식을 상위 결정으로 기록했다.
- Auto UV를 v1.0 핵심 옵션으로 포함한다고 기록했다.

---

## 7. Migration 메모

- 기존 `CarPaint`, `VehicleSkin`, `SedanSkin` 같은 차량 중심 이름은 새 플러그인 설계의 기준 이름으로 쓰지 않는다.
- 기존 CarFight 테스트 머티리얼은 검증 사례로만 사용한다.
- 새 플러그인의 기본 이름은 `PaintableMesh`, `MeshPaint`, `Paint` 계열로 통일한다.
- CarFight 프로젝트에 임시 구현한 뒤 플러그인으로 옮기는 흐름은 선택하지 않는다. 바로 플러그인 기준으로 설계한다.
