# CarFight Active Work

- 문서 버전: v1.21
- 최근 갱신일: 2026-07-21
- 문서 상태: Current
- 역할: CarFight 게임 프로젝트에서 현재 실제로 진행 중인 작업과 각 대표 체크포인트를 연결하는 세션 복원용 색인

---

## 1. 운영 원칙

이 문서는 `main_game` 저장소의 CarFight 게임 기능만 관리한다.
별도 Git 저장소인 AssetDump와 GoPyMCP의 내부 작업, 릴리스 상태, 체크포인트는 등록하지 않는다.

```text
CarFight ActiveWork = CarFight 게임 기능의 현재 작업 선택
AssetDump ActiveWork = UE/Plugins/ue-assetdump/Documents/ActiveWork.md
GoPyMCP ActiveWork = GoPyMCP/Workspace/docs/ActiveWork.md
```

이 문서는 각 작업의 상세 설계와 구현 상태를 복사하지 않는다.
현재 활성 작업, 마지막 작업 초점, 작업 간 의존 관계와 대표 체크포인트 경로만 유지한다.

```text
ActiveWork = 현재 진행 중인 CarFight 작업을 찾는 세션 복원 색인
대표 Plan = 해당 작업의 상세 체크포인트와 다음 단계
Systems = 현재 구현 기준
ProjectSSOT = 프로젝트 우선순위와 상태 판단 기준
Git 및 실제 코드 = 문서와 충돌할 때 우선하는 최종 확인 대상
```

Candidate, Deferred, Archive 작업은 이 문서에 등록하지 않는다.

---

## 2. 현재 활성 작업

| 작업 ID | 카테고리 | 현재 상태 | 대표 체크포인트 |
| --- | --- | --- | --- |
| `CF-FQ-024` | Combat Presentation | `P0 Active / 구현순위 1위`, CF-FQ-025 사용자 PIE 완료로 재개 가능 | `Document/Plan/CombatFxAudio/ImplementationDesign.md` |

---

## 3. 마지막 작업 초점

- 작업 ID: `CF-FQ-025`
- 작업명: 이중 레티클 및 사격방향 시각화
- 현재 단계: `Done / User PIE PASS` — 계획한 조준 레티클·터렛 레티클 구조와 동작 확인 완료
- 바로 다음 논의: 투사체 착탄 위치를 Reticle UI와 분리한 월드 공간 3D 표현으로 설계
- 대표 체크포인트: `Document/Plan/ReticleAimDirection/ImplementationDesign.md`

사용자가 특정 CarFight 작업을 지정하면 이 마지막 작업 초점보다 사용자가 지정한 작업을 우선한다.
AssetDump 또는 GoPyMCP 작업을 지정하면 이 문서를 사용하지 않고 해당 독립 저장소의 `ActiveWork.md`로 진입한다.

---

## 4. 작업 간 의존 관계

```text
CF-FQ-022 Done
+
CF-FQ-023 Done
→ 신뢰 가능한 Aim Solution과 DamageHitContext 입력 계약 확보
→ CF-FQ-018 Damage Runtime 구현 가능
```

```text
CF-FQ-018 Done
→ CF-FQ-019 주행·전투 반복 및 확장 회귀 착수 가능
```

```text
CF-FQ-017 Done
→ ReticleFireFeedback Plan 완료 및 Archive 검토 대상
```

```text
CF-FQ-016 / 017 / 018 / 022 / 023 / 025 Done
→ 발사·조준·피격·피해·UI·이중 레티클 판정 기반 확보
→ CF-FQ-024 전투 FX / Audio 구현
→ CF-FQ-019 주행·전투 반복 테스트
→ CF-FQ-020 조작감·전투 템포·피드백 개선
→ CF-FQ-021 핵심 게임 루프 검증
```

---

## 5. 세션 복원 규칙

사용자가 CarFight 작업에 대해 `이전 작업 이어서 진행해줘`라고 요청하면 다음 순서로 복원한다.

```text
1. 저장소 루트 AGENTS.md 확인
2. main_game Git 상태 확인
3. Document/Document_Entry.md 확인
4. 이 ActiveWork.md 확인
5. 마지막 작업 초점의 대표 Plan 체크포인트 확인
6. 관련 Systems, ProjectSSOT, 실제 코드와 에셋 확인
7. 완료 범위, 미완료 범위, 빌드 상태, PIE 상태와 다음 단계를 먼저 보고
8. 미완료 단계부터 작업 재개
```

사용자가 작업 ID나 작업명을 지정하면 해당 행의 대표 체크포인트를 선택한다.
체크포인트와 실제 저장소가 다르면 실제 Git 상태와 코드를 우선한다.
중첩 저장소 작업은 해당 저장소의 Git 상태와 독립 문서체계를 사용한다.

---

## 6. 세션 인계 규칙

사용자가 CarFight 세션에서 `새 세션 인계 준비해줘`라고 요청하면 다음을 수행한다.

```text
1. main_game Git 상태 재확인
2. 이번 세션에서 상태가 변경된 CarFight 활성 작업의 대표 Plan 체크포인트 갱신
3. 이 문서의 활성 작업 상태, 의존 관계와 마지막 작업 초점 갱신
4. 문서와 실제 CarFight 코드 상태 교차검증
5. 새 세션에서 사용할 짧은 시작 문구 제공
```

AssetDump와 GoPyMCP 상태를 이 문서에 복사하지 않는다.
장문의 대화 요약을 새 SSOT로 만들지 않는다.
세션별 로그 파일을 누적 생성하지 않는다.

---

## 7. 갱신 조건

다음 중 하나가 CarFight 게임 프로젝트에서 발생했을 때 관련 대표 Plan 체크포인트와 필요한 경우 이 문서를 갱신한다.

- 실제 CarFight 코드나 에셋 패치 적용
- 중요한 설계 결정 확정
- 빌드 성공 또는 실패
- 사용자 PIE 결과 수신
- 오류 원인 또는 차단 요소 확정
- 다음 작업 단계 변경
- CarFight 작업 카테고리 또는 마지막 작업 초점 변경
- 작업이 Active에서 Done, Blocked, Deferred 또는 Archive 대상으로 전환

AssetDump 또는 GoPyMCP 내부 상태 변화만으로는 이 문서를 갱신하지 않는다.
단순 설명, 조사 또는 상태가 변하지 않은 질의응답만으로는 갱신하지 않는다.

---

## 8. Changelog

### v1.21 - 2026-07-21

- `CF-FQ-025` 터렛 레티클 구현을 사용자 PIE PASS와 Done으로 전환.
- `CF-FQ-025`를 현재 활성 작업 목록에서 제거하고 `CF-FQ-024`를 기존 순서상 구현순위 1위로 복원.
- 다음 논의 주제를 Reticle과 분리된 투사체 착탄 위치 3D 표현으로 기록.

### v1.20 - 2026-07-21

- Weapon Aim Solution에 터렛 레티클 유효성, 월드 위치와 비교 거리를 추가.
- `Image_WeaponReticle`의 Legacy DirectImpact/LaunchDirection Preview 소비를 제거.
- 기존 Preview 필드는 Legacy Debug로 보존하고 Debug Panel에 신규 터렛 레티클 세 필드를 추가.
- `Tools\BuildEditor.bat` PASS, 다음 단계를 사용자 PIE 검증으로 전환.

### v1.19 - 2026-07-21

- `Image_CenterDot`의 현재 작동을 사용자 확인 PASS로 기록.
- 조준 레티클 입력 계약 확인을 다음 작업에서 제거하고 회귀 보호 대상으로 전환.
- 바로 다음 작업을 `Image_WeaponReticle`의 `CurrentMuzzleDirection` 기반 코드 정렬로 한정.

### v1.18 - 2026-07-21

- `Image_CenterDot`을 사용자의 화면 위치 지정용 조준 레티클로 확정.
- 터렛은 조준 레티클이 선택한 3D 지점을 추적하도록 책임 고정.
- `Image_WeaponReticle`을 탄종 독립 `CurrentMuzzleDirection` 기반 터렛 레티클로 확정.
- 투사체 착탄 위치를 Reticle UI와 분리된 후속 3D 표시로 이관.
- 다음 작업을 기존 DirectImpact/LaunchDirection Preview 소비 경로의 코드 정렬로 변경.

### v1.17 - 2026-07-20

- `Image_WeaponReticle` 사용자 PIE 화면 표시와 오른쪽 구석 좌표 문제 확인을 반영.
- Canvas Slot 앵커를 좌측 상단으로 고정하고 LaunchDirection을 Command 목표 깊이로 표시하도록 보정.
- Camera Aim Trace와 총구 Trace가 같은 Actor를 맞히면 목표 표면으로 허용해 MuzzleBlocked 오판을 보정.
- `Tools\BuildEditor.bat` 성공과 Return Code 0을 기록하고 사용자 PIE 재검증으로 전환.

### v1.16 - 2026-07-16

- Phase 2 C++ 화면 투영 구현 결과를 반영.
- `UCFAimReticleWidget`이 `WeaponPreviewWorldLocation`을 화면 좌표로 투영하고 Optional `Image_WeaponReticle`을 화면 안에서만 표시하도록 확장.
- `Tools\BuildEditor.bat` 성공과 Return Code 0을 기록.
- 현재 단계를 WBP 바인딩 및 사용자 PIE 확인 대기로 전환.

### v1.15 - 2026-07-16

- Heavy Cannon 사용자 PIE에서 MuzzleBlocked 회귀와 실제 중력탄 초기 발사 방향 일치가 정상임을 반영.
- `CF-FQ-025` LaunchDirection 단계를 코드, 빌드, PIE PASS로 전환.
- 바로 다음 작업을 Phase 2 World To Screen과 WBP Weapon Reticle 표시 계약 준비로 변경.

### v1.14 - 2026-07-16

- `CF-FQ-025` LaunchDirection Codex 실행 결과를 반영.
- `ECFWeaponReticleMode`와 `FCFVehicleWeaponAimSolution.WeaponReticleMode` 추가, 중력 Projectile의 LaunchDirection 데이터 제공, VehicleDebug Mode 행 추가를 완료 범위로 기록.
- `Tools\BuildEditor.bat` 성공과 Return Code 0을 기록.
- 현재 단계를 Codex 실행 대기에서 Heavy Cannon 사용자 PIE 대기로 전환.
- 사용자 PIE에서 Heavy Cannon 회전 시 LaunchDirection 모드, Preview Valid, Blocking Hit false, 거리 10000.0, 월드 위치 갱신을 부분 확인으로 기록.
- Phase 2 World To Screen과 WBP Weapon Reticle 표시는 사용자 PIE PASS 이후 진행하도록 유지.

### v1.13 - 2026-07-16

- 중력 Projectile의 Weapon Reticle을 `LaunchDirection`으로 표시하고 예상 탄착점은 후속 `BallisticImpact`로 분리하는 사용자 결정을 반영.
- `Hidden / DirectImpact / LaunchDirection` 모드 계약과 LaunchDirection의 비충돌·비탄착 의미를 확정.
- 상세 TaskSource `ReticleAimDirection/TaskSource_LaunchDirection.md`를 생성.
- 최종 Codex 입력 `ReticleAimDirection/Generated/Final/P1_LaunchDirection.md`를 생성.
- 현재 상태를 LaunchDirection 결정 대기에서 설계·Codex 계약 준비 완료와 Codex 실행 대기로 전환.
- 대표 Plan v0.2.4의 구현 계약 및 다음 검수 단계로 연결.

### v1.12 - 2026-07-16

- 사용자 PIE에서 실사용 Heavy Cannon의 Weapon Preview가 계속 무효로 표시된 결과를 반영.
- HeavyCannon이 중력 Projectile을 사용해 DirectImpact 전용 Phase 1 범위 밖임을 확인.
- 상태를 단순 사용자 PIE 대기에서 실사용 무기 범위 불일치와 LaunchDirection 결정 대기로 변경.
- 다음 작업을 중력탄의 초기 발사 방향 표시와 후속 BallisticImpact 분리 결정으로 전환.
- 대표 Plan v0.2.3의 에셋 조사와 차단 상태에 연결.

### v1.11 - 2026-07-16

- `CF-FQ-025` Phase 1 Codex 실행 결과와 브라우저 scoped diff 검수를 PASS로 반영.
- 독립 Editor 빌드 `build_79aa1777becd65e5` 성공과 Return Code 0을 기록.
- 현재 단계를 사용자 PIE 대기로 전환하고 HitScan·Projectile Preview 및 기존 발사 정책 회귀 확인을 다음 작업으로 지정.
- 대표 Plan v0.2.2의 검수 증거와 다음 단계로 연결.

### v1.10 - 2026-07-16

- `CF-FQ-025`의 현재 단계를 Phase 1 Codex 검토·보정 계약 준비 완료와 Codex 실행 대기로 갱신.
- 마지막 작업의 다음 실행 입력을 `ReticleAimDirection/Generated/Final/P1_WeaponPreviewReview.md`로 지정.
- Codex 실행 후 실제 scoped diff와 `Tools\BuildEditor.bat` 결과를 브라우저 AI가 검수하도록 다음 단계를 갱신.
- 세부 정책 위반 상태와 빌드·PIE 증거는 대표 Plan v0.2.1에서 관리하도록 연결.

### v1.9 - 2026-07-16

- `CF-FQ-025` 이중 레티클 및 사격방향 시각화를 P0 Active 구현순위 1위로 등록.
- 마지막 작업 초점과 세션 복원 대표 체크포인트를 `ReticleAimDirection/ImplementationDesign.md`로 전환.
- `CF-FQ-024`는 취소하지 않고 P0 Active 구현순위 2위로 유지.
- 진행 순서를 `CF-FQ-025 → CF-FQ-024 → CF-FQ-019 → CF-FQ-020 → CF-FQ-021`로 갱신.

### v1.8 - 2026-07-15

- `CF-FQ-024` 전투 FX 및 사운드 구현을 새 활성 작업으로 등록.
- 발사·Impact·차량 파괴 연출의 대표 Plan을 `CombatFxAudio/ImplementationDesign.md`로 연결.
- 다음 진행 순서를 `CF-FQ-024 → CF-FQ-019 → CF-FQ-020 → CF-FQ-021`로 갱신.

### v1.7 - 2026-07-15

- `CF-FQ-017`의 NoWeapon, AimBlocked와 정상 발사 회귀 사용자 PIE PASS를 반영.
- `CF-FQ-017`을 활성 작업에서 제거하고 현재 활성 작업 없음으로 전환.
- `CF-FQ-019`는 자동 착수하지 않고 Candidate 상태를 유지.

### v1.6 - 2026-07-15

- `CF-FQ-018` 사용자 PIE PASS와 Done 판정을 반영해 활성 작업 목록에서 제거.
- 마지막 작업 초점을 `CF-FQ-017`의 `NoWeapon` / `AimBlocked` 최종 검증으로 전환.
- `CF-FQ-019`가 Damage Runtime 완료 이후 착수 가능한 확장 회귀 후보임을 기록.

### v1.5 - 2026-07-14

- `CF-FQ-018` 최소 Damage Runtime 코드와 공식 Editor 빌드 완료 상태를 반영.
- 사용자 PIE 전이므로 기능은 Active로 유지하고 피해 누적·파괴 전환 검증을 다음 작업으로 지정.
- HitScan과 Projectile 공용 피해 적용, Projectile 첫 Impact 1회 적용과 Pool 결과 복사 구조를 복원 기준으로 기록.

### v1.4 - 2026-07-14

- `CF-FQ-022`와 `CF-FQ-023`의 Done / P0 사용자 PIE 완료 상태를 반영해 활성 작업 목록에서 제거.
- 마지막 작업 초점을 `CF-FQ-018 최소 Damage Runtime`으로 전환.
- `CF-FQ-018`의 선행 조건 완료와 바로 다음 작업을 DamageData·체력 소유 구조·피해 인터페이스 확정으로 기록.
- 완료된 Aim/Projectile 작업은 대표 Plan과 Systems에서 조회하고 ActiveWork에는 복사하지 않도록 정리.

### v1.3 - 2026-07-14

- CarFight ActiveWork의 관리 범위를 `main_game` 게임 기능으로 제한.
- 잘못 등록된 `CF-TOOL-ADUMP` 활성 작업과 AssetDump 마지막 작업 초점 및 의존 관계 제거.
- 마지막 작업 초점을 기존 CarFight 작업인 `CF-FQ-023`으로 복원.
- AssetDump와 GoPyMCP 작업은 각 독립 저장소의 `ActiveWork.md`로 진입하도록 경계 추가.

### v1.2 - 2026-07-14

- `CF-TOOL-ADUMP`을 현재 활성 도구 작업으로 등록하고 `AssetDumpPlan/README.md`에 연결했다.
- 이 등록은 독립 저장소 경계 원칙에 어긋나 v1.3에서 철회했다.

### v1.1 - 2026-07-14

- CF-FQ-022, 023, 018, 017 대표 Plan에 표준 `현재 작업 체크포인트`를 적용.
- CF-FQ-023 공식 Editor 빌드 성공 상태를 반영하고 다음 작업을 사용자 PIE 검증으로 변경.
- CF-FQ-018의 선행 조건을 코드 구현이 아니라 CF-FQ-022·023 사용자 PIE 검증으로 정정.

### v1.0 - 2026-07-14

- 다중 작업 세션 복원을 위한 ActiveWork 색인 최초 작성.
- CF-FQ-017, 018, 022, 023 활성 작업과 대표 Plan 연결.
- 마지막 작업 초점을 CF-FQ-023으로 기록.
- 짧은 세션 인계 및 복원 명령의 처리 규칙 추가.

---

## 9. Migration

### v1.13 적용 안내

- 새 세션은 `CF-FQ-025`의 LaunchDirection 설계가 확정됐고 Codex 실행 계약까지 준비된 상태로 복원한다.
- 실행 입력은 `Document/Plan/ReticleAimDirection/Generated/Final/P1_LaunchDirection.md`다.
- 중력 Projectile은 LaunchDirection으로 실제 초기 발사 방향을 제공하되 Blocking Hit와 예상 탄착점을 의미하지 않는다.
- 예상 탄착점은 후속 Ballistic Solver의 BallisticImpact 단계로 유지한다.
- Codex 실행과 Browser scoped diff·Editor 빌드 검수 전에는 LaunchDirection 코드 구현 완료로 해석하지 않는다.
- 검수 후 사용자 PIE에서 Heavy Cannon의 Mode=LaunchDirection, Preview Valid=Yes, Blocking Hit=No를 확인한다.

### v1.12 적용 안내

- 새 세션은 `CF-FQ-025` DirectImpact 코드와 빌드는 검수됐지만 실사용 Heavy Cannon이 중력 Projectile이라 기능 검증이 차단된 상태로 복원한다.
- 현재 Heavy Cannon의 Preview 무효 결과를 단순 코드 고장으로 판단하거나 `DA_HeavyShell.bAffectedByGravity`를 임시 변경하지 않는다.
- Phase 2 UI 작업 전에 `LaunchDirection` 표시를 도입할지 확정한다.
- 권장 구조는 중력 Projectile에서 실제 초기 발사 방향을 Weapon Reticle로 표시하고 예상 탄착점은 후속 `BallisticImpact`로 분리하는 것이다.
- 사용자 결정 후 TaskSource와 Codex 계약을 새로 작성한다.

### v1.11 적용 안내

- 새 세션은 `CF-FQ-025` Phase 1 코드와 빌드가 검수 PASS이고 사용자 PIE만 Pending인 상태로 복원한다.
- Preview World 데이터는 코드에 존재하지만 사용자 PIE 완료 전까지 Phase 1 최종 완료로 해석하지 않는다.
- 다음 구현 TaskSource를 만들기 전에 HitScan, 중력 없는 Projectile, 중력 Projectile 무효, MuzzleBlocked와 정렬 정책 회귀를 확인한다.
- Phase 1 PIE PASS 후에만 Phase 2 Weapon Reticle C++ 투영 작업으로 진행한다.

### v1.10 적용 안내

- 새 세션은 `CF-FQ-025` 대표 Plan의 정책 위반 복구 체크포인트를 먼저 읽는다.
- Phase 1의 실행 입력은 `Document/Plan/ReticleAimDirection/Generated/Final/P1_WeaponPreviewReview.md`이며, 상세 요구사항은 연결된 `TaskSource_Phase1.md`를 기준으로 한다.
- Codex 실행과 브라우저 diff·빌드 검수가 끝나기 전에는 Phase 1 완료 또는 Current System으로 해석하지 않는다.
- 현재 연결에 Codex 실행 도구가 없으면 소스 직접 수정 예외를 자동 적용하지 않고 `Codex 실행 대기` 상태를 유지한다.

### v1.9 적용 안내

- 신규 CarFight 구현 세션은 `CF-FQ-025`와 `Document/Plan/ReticleAimDirection/ImplementationDesign.md`를 우선 복원한다.
- `CF-FQ-024`는 삭제하거나 보류 상태로 바꾸지 않고 구현순위 2위 Active 작업으로 유지한다.
- `CF-FQ-025` Phase 0~4와 사용자 PIE 완료 또는 사용자 명시적 전환 전에는 신규 구현 초점을 `CF-FQ-025`에 둔다.
- Weapon Reticle과 Weapon Preview는 구현 및 사용자 PIE 완료 전까지 Current System으로 해석하지 않는다.
- 현재 완료 구현 판단은 계속 관련 `Document/Systems/` 문서를 우선한다.

### 기존 적용 안내

- `CF-TOOL-ADUMP`과 `Document/Plan/AssetDumpPlan/`은 CarFight 활성 작업 체계에서 제거한다.
- AssetDump 상태와 릴리스 판정은 `UE/Plugins/ue-assetdump/Documents/`에서만 관리한다.
- GoPyMCP 상태와 내부 로드맵은 `GoPyMCP/Workspace/docs/`에서만 관리한다.
- CarFight가 도구의 공개 계약에 의존하는 경우 의존성만 CarFight 문서에 기록하고 도구 내부 진행 상태는 복사하지 않는다.
- 기존 FeatureQueue, Plan Index, Systems Index의 역할은 변경하지 않는다.
- 새 세션에서는 장문의 인계 프롬프트 대신 `이전 작업 이어서 진행해줘`를 사용할 수 있다.
- 세션 이동 전에는 `새 세션 인계 준비해줘`를 사용할 수 있다.
