# MeshPaintPrepTool - Validation

> 역할: `MeshPaintPrepTool`의 빌드, 에디터, 페인트, 재사용 검증 기준을 정의한다.
> 문서 버전: v0.5
> 마지막 정리(Asia/Seoul): 2026-06-30
> 상태: Draft / Validation Plan

---

## 1. 검증 목표

검증 목표는 아래 세 가지다.

1. 플러그인이 Unreal Editor에서 안정적으로 로드된다.
2. 에디터에서 만든 Static Mesh를 페인트 가능 상태로 준비한다.
3. 준비 결과가 CarFight뿐 아니라 다른 프로젝트에도 적용 가능한 구조다.

---

## 2. 빌드 검증

CarFight 안에서 검증할 때는 프로젝트 규칙에 따라 아래 명령만 사용한다.

```text
D:\Work\CarFight_git\Tools\BuildEditor.bat
```

예상 결과:

1. `MeshPaintPrepToolEditor` 모듈 컴파일 오류가 없다.
2. UHT 오류가 없다.
3. CarFight 기존 모듈이 플러그인 때문에 깨지지 않는다.
4. 플러그인이 없어도 CarFight 런타임 코드가 직접 의존하지 않는다.

현재 검증 기록:

| 날짜 | 명령 | 결과 |
|---|---|---|
| 2026-06-29 | `D:\Work\CarFight_git\Tools\BuildEditor.bat` | 성공 |
| 2026-06-29 | `D:\Work\CarFight_git\Tools\BuildEditor.bat` | 성공, v0.2 탭 / 선택 검사 기반 포함 |
| 2026-06-29 | `D:\Work\CarFight_git\Tools\BuildEditor.bat` | 성공, v0.3 기본 페인트 에셋 생성 흐름 포함 |
| 2026-06-30 | `D:\Work\CarFight_git\Tools\BuildEditor.bat` | 성공, v0.4 박스 기반 Auto UV 생성 포함 |

빌드 메모:

1. `MeshPaintPrepToolEditor` 모듈 컴파일 / 링크가 통과했다.
2. `VisualStudioTools.Build.cs`의 `UnsafeTypeCastWarningLevel` deprecation 경고가 있었지만 이번 플러그인과 직접 관련된 오류는 아니다.

---

## 3. 에디터 로드 검증

검증 절차:

1. CarFight 프로젝트를 연다.
2. 플러그인 목록에서 `MeshPaintPrepTool`이 활성화되어 있는지 확인한다.
3. 에디터를 재시작한다.
4. `Mesh Paint Prep` 패널 또는 메뉴가 보이는지 확인한다.

예상 결과:

1. 에디터 시작 중 플러그인 로드 오류가 없다.
2. 플러그인 UI가 열린다.
3. 아무 메시를 선택하지 않은 상태에서는 실행 버튼이 비활성화되거나 명확한 안내가 나온다.

---

## 4. 단일 메시 검증

### 4.1 박스 메시

목적:

```text
가장 단순한 에디터 제작 Static Mesh가 페인트 가능한지 확인한다.
```

절차:

1. 모델링 모드(`Modeling Mode`)에서 박스 메시를 만든다.
2. Static Mesh로 저장한다.
3. 툴에서 대상 메시로 선택한다.
4. `Auto UV 생성` 버튼을 누른다.
5. `준비 생성` 버튼을 누른다.
6. 메시 페인트(`Mesh Paint`)에서 텍스처에 색칠한다.
7. 메시 페인트 모드를 끈다.

예상 결과:

1. `Paint` 폴더가 생긴다.
2. `T_{MeshName}_Paint` 텍스처가 생긴다.
3. `MI_{MeshName}_Paint` 머티리얼 인스턴스가 생긴다.
4. 메시 슬롯에 MI가 적용된다.
5. 메시 페인트 모드 밖에서도 칠한 색이 보인다.

### 4.2 실린더 총알 메시

목적:

```text
원형 단면 메시에서 Auto UV와 텍스처 페인트가 유지되는지 확인한다.
```

예상 결과:

1. 옆면과 앞뒤면에 페인트가 찍힌다.
2. UV가 심하게 겹쳐 한쪽만 칠했는데 모든 면이 같이 바뀌는 문제가 없어야 한다.
3. 작은 해상도에서도 결과를 볼 수 있다.

### 4.3 박스 + 실린더 터렛 메시

목적:

```text
여러 프리미티브를 붙여 만든 복합 임시 메시를 검증한다.
```

예상 결과:

1. 터렛 몸체와 포신에 페인트가 보인다.
2. Auto UV 후 각 부품에 다른 색을 칠할 수 있다.
3. 머티리얼이 한 슬롯에 적용된 경우 전체 메시가 같은 페인트 텍스처를 사용한다.

---

## 5. CarFight 차량 검증

검증 대상:

```text
/Game/CarFight/Vehicles/TestSedan/Mesh_TestSedan
/Game/CarFight/Vehicles/TestSUV/Mesh_TestSUV
```

검증 목적:

1. 기존 수동 작업과 플러그인 결과가 같은지 확인한다.
2. 스폰 차량에서도 페인트가 보이는지 확인한다.
3. 차량 전용 코드 없이도 Static Mesh 기준으로 처리되는지 확인한다.

체크포인트:

1. 생성된 MI가 메시 슬롯에 적용되어 있다.
2. MI의 `BasePaintTexture`가 생성 텍스처를 가리킨다.
3. `UsePaintTexture`가 켜져 있다.
4. 텍스처가 Virtual Texture 전용으로 생성되지 않았다.
5. 스폰되는 차량의 시각 메시가 같은 Static Mesh와 같은 MI를 사용한다.

---

## 6. 저장 / 재시작 검증

절차:

1. 툴 실행 후 생성 에셋을 저장한다.
2. 메시 페인트로 색칠한다.
3. 페인트 텍스처를 저장한다.
4. 에디터를 닫는다.
5. 에디터를 다시 연다.
6. 같은 맵 또는 메시 에셋을 확인한다.

예상 결과:

1. 칠한 내용이 유지된다.
2. 생성된 텍스처가 사라지지 않는다.
3. 메시 슬롯의 MI 연결이 유지된다.
4. 스폰된 액터도 같은 MI를 쓰면 칠한 결과가 보인다.

---

## 7. 실패 케이스 검증

| 케이스 | 예상 처리 |
|---|---|
| 아무것도 선택하지 않음 | "Static Mesh를 먼저 선택하세요." 안내 |
| Skeletal Mesh 선택 | 대상 타입 아님 안내 |
| UV 없음 + Auto UV 끔 | UV 필요 안내 |
| Auto UV 실패 | 실패 안내와 Static Mesh LOD0 / UV 채널 상태 확인 안내 |
| 부모 머티리얼 누락 | 플러그인 콘텐츠 누락 안내 |
| 같은 이름 에셋 존재 | 덮어쓰기 또는 새 이름 선택 요구 |
| 저장 실패 | 저장 실패 안내 |

---

## 8. 완료 판정

v1.0 완료 판정:

```text
박스, 실린더 총알, 박스+실린더 터렛, TestSedan, TestSUV에서
페인트 준비 -> 메시 페인트 -> 저장 -> 재시작 확인이 통과한다.
```

추가 확인:

1. CarFight 전용 경로를 플러그인 코드에서 검색했을 때 나오지 않는다.
2. 플러그인 삭제 시 CarFight 런타임 빌드가 직접 깨지지 않는다.
3. 다른 테스트 프로젝트에 플러그인 폴더를 넣어도 로드 가능해야 한다.

---

## 9. Changelog

### v0.5

- v0.4 Auto UV 생성 기능 포함 빌드 성공 기록을 추가했다.
- 단일 박스 메시 검증 절차를 `Auto UV 생성` 버튼 기준으로 갱신했다.
- Auto UV 실패 메시지 기준을 현재 구현에 맞게 조정했다.
- v0.4 플러그인 커밋은 `1ea2ff0a56c98c2a3e8a269adc15e7a2929e9024`다.

### v0.4

- v0.3 기본 페인트 에셋 생성 흐름 구현 후 빌드 성공 기록을 추가했다.
- Texture2D / Material / Material Instance 생성, Static Mesh 슬롯 적용, Mesh Painting 설정 적용을 검증 대상에 포함했다.
- 실제 에디터 버튼 클릭과 생성 에셋 육안 확인은 다음 에디터 실행 세션에서 확인할 항목으로 남겼다.
- v0.3 플러그인 커밋은 `cbb9e70e9bbb13234749ffc8b1f411c007d8d0d3`다.

### v0.3

- `Mesh Paint Prep` 에디터 탭과 Static Mesh 선택 / UV 검사 기반 구현 후 빌드 성공 기록을 추가했다.
- 실제 에디터 UI 클릭 검증은 다음 에디터 실행 세션에서 확인할 항목으로 남겼다.
- v0.2 플러그인 커밋은 `70deb85e710dbd38e4ed9942a00091894ad0b0c4`다.

### v0.2

- `Tools\BuildEditor.bat` 빌드 성공 기록을 추가했다.
- `MeshPaintPrepToolEditor` 모듈 컴파일 / 링크 통과 상태를 기록했다.
- VisualStudioTools 기존 경고를 플러그인 직접 오류가 아닌 빌드 메모로 분리했다.

### v0.1

- 빌드, 에디터 로드, 단일 메시, 차량, 저장 / 재시작, 실패 케이스 검증 기준을 추가했다.
- CarFight 검증 대상을 차량 외에 총알과 터렛까지 확장했다.

---

## 10. Migration 메모

- 기존 수동 페인트 준비 절차는 플러그인 검증의 비교 기준으로 사용한다.
- 기존 차량 스폰 문제 검증은 "스폰 액터가 같은 Static Mesh와 MI를 사용하는지" 항목으로 일반화한다.
