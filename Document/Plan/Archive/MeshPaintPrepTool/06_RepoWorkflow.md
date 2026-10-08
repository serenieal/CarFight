# MeshPaintPrepTool - Repo Workflow

> 역할: 별도 Git 저장소와 CarFight 서브모듈 운영 방식을 정의한다.
> 문서 버전: v0.4
> 마지막 정리(Asia/Seoul): 2026-06-29
> 상태: Draft / Git Repo / Submodule Plan

---

## 1. 원칙

`MeshPaintPrepTool`은 CarFight 프로젝트의 내부 전용 코드가 아니다.

운영 원칙:

1. 플러그인 원본은 별도 Git 저장소에 둔다.
2. CarFight는 해당 저장소를 서브모듈로 가져온다.
3. 플러그인 코드에는 CarFight 전용 경로를 넣지 않는다.
4. CarFight는 검증 프로젝트 역할을 한다.
5. 다른 Unreal 프로젝트도 같은 저장소를 서브모듈 또는 복사 방식으로 사용할 수 있어야 한다.

---

## 2. 목표 구조

별도 저장소:

```text
https://github.com/serenieal/MeshPaintPrepTool
```

별도 저장소 내부 구조:

```text
MeshPaintPrepTool/
  MeshPaintPrepTool.uplugin
  Source/
  Content/
  Docs/
  README.md
```

CarFight 내부:

```text
D:\Work\CarFight_git\UE\Plugins\MeshPaintPrepTool
```

Git 관리:

```text
D:\Work\CarFight_git
  -> CarFight 메인 저장소

D:\Work\CarFight_git\UE\Plugins\MeshPaintPrepTool
  -> MeshPaintPrepTool 서브모듈
```

---

## 3. 서브모듈 추가 절차

플러그인 저장소에 초기 커밋이 생긴 뒤 수행한다.

명령 후보:

```text
git submodule add https://github.com/serenieal/MeshPaintPrepTool UE/Plugins/MeshPaintPrepTool
git submodule update --init --recursive
```

주의:

1. 원격 저장소 URL은 `https://github.com/serenieal/MeshPaintPrepTool`로 확정됐다.
2. 서브모듈 추가 전 `UE/Plugins/MeshPaintPrepTool` 경로가 비어 있어야 한다.
3. 이미 같은 폴더가 있으면 백업 또는 삭제 여부를 사용자에게 묻는다.
4. CarFight 메인 저장소에는 `.gitmodules`와 서브모듈 포인터만 커밋한다.
5. 원격 저장소가 빈 상태이면 먼저 플러그인 초기 커밋을 만든다.

---

## 4. 작업 흐름

플러그인 코드 수정 흐름:

```text
1. UE/Plugins/MeshPaintPrepTool 폴더에서 플러그인 코드 수정
2. 플러그인 저장소에 커밋
3. 플러그인 저장소 원격에 push
4. CarFight 루트로 이동
5. 서브모듈 포인터 변경 확인
6. CarFight 메인 저장소에 서브모듈 포인터 커밋
```

CarFight 검증 흐름:

```text
1. CarFight에서 Tools\BuildEditor.bat 실행
2. 에디터 실행
3. 박스 / 총알 / 터렛 / 차량 메시 검증
4. 문제를 플러그인 저장소에서 수정
5. 다시 CarFight 서브모듈 포인터 갱신
```

---

## 5. 커밋 경계

플러그인 저장소 커밋에 포함할 것:

1. `.uplugin`
2. 플러그인 C++ 소스
3. 플러그인 콘텐츠
4. 플러그인 자체 문서
5. 플러그인 테스트용 최소 샘플이 있다면 해당 파일

CarFight 메인 저장소 커밋에 포함할 것:

1. `.gitmodules`
2. `UE/Plugins/MeshPaintPrepTool` 서브모듈 포인터
3. CarFight 검증 문서
4. CarFight 프로젝트에서 플러그인 활성화에 필요한 설정

섞지 말 것:

| 금지 | 이유 |
|---|---|
| 플러그인 코드를 CarFight 메인 저장소에 직접 복사 | 원본 저장소와 분리 실패 |
| CarFight 전용 에셋을 플러그인 저장소에 넣기 | 재사용성 저하 |
| 플러그인 커밋 없이 CarFight 포인터만 변경 | 재현 불가 상태 |
| 원격 URL 미정 상태에서 서브모듈 추가 | 나중에 경로 변경 비용 발생 |

---

## 6. 결정 상태

아래 항목은 현재 결정 상태를 기록한다.

| ID | 질문 | 현재 상태 |
|---|---|---|
| Q001 | 원격 저장소 URL | 확정: `https://github.com/serenieal/MeshPaintPrepTool` |
| Q002 | 공개 / 비공개 여부 | 확정: 비공개 |
| Q003 | 라이선스 | 확정: 현재 만들지 않음 |
| Q004 | 기본 브랜치 이름 | 확정: `main` |
| Q005 | 저장소 생성 위치 | 서브모듈 경로: `UE/Plugins/MeshPaintPrepTool` |

현재 연결 상태:

| 항목 | 값 |
|---|---|
| 원격 저장소 | `https://github.com/serenieal/MeshPaintPrepTool` |
| 공개 범위 | 비공개 |
| 초기 커밋 | `27d400f16e8bb46c51eea064196b23f8ab7150b9` |
| v0.2 패널 커밋 | `70deb85e710dbd38e4ed9942a00091894ad0b0c4` |
| v0.3 생성 흐름 커밋 | `cbb9e70e9bbb13234749ffc8b1f411c007d8d0d3` |
| CarFight 서브모듈 경로 | `UE/Plugins/MeshPaintPrepTool` |
| CarFight 빌드 검증 | `Tools\BuildEditor.bat` 성공 |

---

## 7. 검증 기준

서브모듈 검증:

1. CarFight 루트에서 `git status --short`를 봤을 때 `UE/Plugins/MeshPaintPrepTool`이 서브모듈로 표시된다.
2. `git submodule status`에 플러그인 커밋 해시가 표시된다.
3. 클린 체크아웃 후 `git submodule update --init --recursive`로 플러그인이 받아진다.
4. `Tools\BuildEditor.bat`가 통과한다.
5. 에디터에서 플러그인을 활성화할 수 있다.

---

## 8. Changelog

### v0.4

- v0.3 생성 흐름 커밋 `cbb9e70e9bbb13234749ffc8b1f411c007d8d0d3`를 기록했다.
- CarFight 서브모듈 포인터가 v0.3 커밋을 가리켜야 한다고 상태를 갱신했다.

### v0.3

- 기본 브랜치를 `main`으로 확정했다.
- 초기 커밋 해시와 CarFight 서브모듈 연결 상태를 기록했다.
- CarFight 빌드 검증 성공 상태를 기록했다.
- v0.2 패널 커밋 `70deb85e710dbd38e4ed9942a00091894ad0b0c4`를 기록했다.

### v0.2

- 원격 저장소 URL을 `https://github.com/serenieal/MeshPaintPrepTool`로 확정했다.
- 저장소 공개 범위를 비공개로 기록했다.
- 라이선스는 현재 만들지 않는다고 기록했다.
- 서브모듈 추가 명령 후보를 실제 URL 기준으로 갱신했다.
- 원격 저장소가 빈 상태이면 초기 커밋 후 서브모듈을 추가한다고 명시했다.

### v0.1

- 별도 Git 저장소와 CarFight 서브모듈 운영 원칙을 문서화했다.
- 서브모듈 추가 절차와 커밋 경계를 정의했다.
- 원격 저장소 URL, 공개 범위, 라이선스 등 사용자 결정 필요 항목을 분리했다.

---

## 9. Migration 메모

- CarFight 내부에 플러그인 코드를 먼저 만든 경우, 별도 저장소로 이동한 뒤 CarFight에는 서브모듈만 남긴다.
- 서브모듈 전환 시 기존 복사본이 있으면 삭제 여부를 사용자에게 확인한다.
- 원격 저장소 초기 커밋과 CarFight 서브모듈 추가는 완료됐다.
