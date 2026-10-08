# CFNetSmooth 저장소 분리 기록

- 문서 버전: v0.2.0
- 작성일: 2026-06-18
- 대상: NetSmoothSync 독립 레포 / CarFight 서브모듈
- 작업 유형: 신규
- 상태: Active

---

## 1. 목적

이 문서는 `CFNetSmooth` 플러그인을 CarFight 프로젝트 내부 폴더에서 분리해, 다른 Unreal Engine 프로젝트에서도 재사용 가능한 독립 Git 저장소로 관리하기 위한 기준을 기록한다.

---

## 2. 현재 구조

독립 레포:

```text
https://github.com/serenieal/NetSmoothSync.git
```

CarFight 서브모듈 경로:

```text
D:\Work\CarFight_git\UE\Plugins\CFNetSmooth
```

로컬 독립 작업 폴더:

```text
D:\Work\NetSmoothSync
```

이전 원본 백업:

```text
D:\Work\CFNetSmooth_backup_20260618_0917
```

---

## 3. 이름 기준

저장소 이름은 다음을 사용한다.

```text
NetSmoothSync
```

Unreal 플러그인 descriptor와 모듈 이름은 기존 호환성을 위해 다음을 유지한다.

```text
CFNetSmooth.uplugin
Source/CFNetSmooth
UCFNetSmoothComp
ACFNetSmoothTestActor
```

판정:

- GitHub 레포 이름은 `NetSmoothSync`이다.
- Unreal 플러그인 이름과 C++ 심볼은 아직 `CFNetSmooth`이다.
- 심볼 리네이밍은 별도 대규모 마이그레이션 작업으로 분리한다.

---

## 4. 추적 대상

새 레포에 포함하는 대상:

```text
CFNetSmooth.uplugin
Source/CFNetSmooth
.gitignore
README.md
```

새 레포에 포함하지 않는 대상:

```text
Binaries
Intermediate
Saved
DerivedDataCache
RuntimeLogs
CarFight 전용 검증 스크립트
CarFight 전용 맵과 Asset
```

---

## 5. 서브모듈 등록 상태

`.gitmodules`에는 아래 항목을 추가한다.

```text
[submodule "UE/Plugins/CFNetSmooth"]
	path = UE/Plugins/CFNetSmooth
	url = https://github.com/serenieal/NetSmoothSync.git
```

현재 서브모듈 HEAD:

```text
92805c1 Initial NetSmoothSync plugin import
```

현재 릴리즈 태그:

```text
v0.12.1
```

---

## 6. 검증 결과

확인 명령:

```text
git submodule status -- UE/Plugins/CFNetSmooth
git -C UE/Plugins/CFNetSmooth status --short --ignored
Tools\BuildEditor.bat
```

결과:

```text
서브모듈 HEAD 92805c1 확인
Binaries/Intermediate는 ignored 상태 확인
Tools\BuildEditor.bat Result: Succeeded
```

---

## 7. 새 클론 검증

검증 목적:

```text
다른 PC나 새 세션에서 CarFight를 받은 뒤 NetSmoothSync 서브모듈을 정상 초기화할 수 있는지 확인한다.
```

검증 위치:

```text
D:\Work\_SubmoduleVerify\CarFight_submodule_verify_20260618_092915
```

검증 명령:

```text
git clone --filter=blob:none --no-checkout --branch codex/vehicle-rotation-jitter-debug --single-branch https://github.com/serenieal/CarFight.git <VerifyDir>
git -C <VerifyDir> sparse-checkout init --cone
git -C <VerifyDir> sparse-checkout set UE/Plugins/CFNetSmooth .gitmodules
git -C <VerifyDir> checkout codex/vehicle-rotation-jitter-debug
git -C <VerifyDir> submodule update --init --recursive -- UE/Plugins/CFNetSmooth
git -C <VerifyDir> submodule status -- UE/Plugins/CFNetSmooth
```

검증 결과:

```text
Submodule path 'UE/Plugins/CFNetSmooth': checked out '92805c11d6ebaddfefce862f4771a5acc85d4dad'
92805c11d6ebaddfefce862f4771a5acc85d4dad UE/Plugins/CFNetSmooth (v0.12.1)
```

확인된 파일:

```text
CFNetSmooth.uplugin
README.md
.gitignore
Source
```

판정:

- 새 클론 기준으로 `UE/Plugins/CFNetSmooth` 서브모듈 초기화가 정상 동작한다.
- `NetSmoothSync`의 `v0.12.1` 태그가 서브모듈 상태에 표시된다.
- 전체 Asset checkout 없이도 서브모듈 경로 검증이 가능하다.

---

## 8. 운영 기준

- 플러그인 Runtime 코드는 `NetSmoothSync` 레포에서 수정한다.
- CarFight 전용 적용 코드는 CarFight 프로젝트 쪽에 둔다.
- 플러그인에 게임 프로젝트 모듈 의존성을 추가하지 않는다.
- 다른 프로젝트에 적용할 때도 `Plugins/CFNetSmooth` 같은 경로에 서브모듈로 추가할 수 있다.
- 서브모듈 업데이트 후에는 호스트 프로젝트에서 `Tools\BuildEditor.bat` 또는 해당 프로젝트의 Editor 빌드를 수행한다.

---

## 9. Changelog

### v0.2.0

```text
- NetSmoothSync v0.12.1 태그 생성 상태 추가
- 새 클론 sparse checkout 기준으로 UE/Plugins/CFNetSmooth 서브모듈 초기화 검증 결과 추가
- 서브모듈 상태가 92805c1과 v0.12.1 태그를 정상 표시한 결과 기록
```

### v0.1.0

```text
- NetSmoothSync GitHub 독립 레포 생성 기록 추가
- CarFight의 UE/Plugins/CFNetSmooth 경로를 NetSmoothSync 서브모듈로 등록한 상태 기록
- 추적 대상, 제외 대상, 검증 결과, 운영 기준 추가
```

---

## 10. 마이그레이션 지침

- 기존 로컬 백업은 검증이 끝난 뒤 수동 삭제할 수 있다.
- CarFight에서 서브모듈 변경을 확정하려면 `.gitmodules`와 `UE/Plugins/CFNetSmooth` gitlink를 함께 커밋한다.
- 새 PC에서 받을 때는 아래 명령을 사용한다.

```text
git submodule update --init --recursive
```
