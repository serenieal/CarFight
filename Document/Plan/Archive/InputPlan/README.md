# InputPlan README

## 목적
`Document/ProjectSSOT/Plan/InputPlan/`은 CarFight 차량 입력 해석 변경안 중 **게임패드 단일 2D 입력 기반 차량 이동 규칙**을 고정하는 폴더다.

현재 확정된 핵심 방향은 아래다.

- 입력 각도/영역 해석은 CarFight 커스텀 로직이 담당
- 후진 전환의 최종 결정권은 Chaos Vehicle `bUseAutoReverse`가 담당
- 수동 기어 강제와 음수 스로틀 기반 reverse 강제는 사용하지 않음
- 게임패드는 `IA_VehicleMove` 기반 신규 조작을 사용
- 키보드는 기존 `IA_Throttle / IA_Brake / IA_Steering` 기반 조작을 유지
- 이 장치별 분리는 IMC 분리로 적용하며 실제 동작 확인까지 끝남
- P1 입력 충돌 방지(Runtime Input Ownership)를 적용해 `VehicleMove2D`와 `LegacyAxis`가 동시에 살아 있어도 한쪽만 차량 입력을 지배하도록 정리함



이 폴더는 다음 6가지를 분리해 유지한다.

1. 입력 규칙 명세
2. 네이티브 구현 설계
3. 검증 및 테스트 기준
4. 실제 작업 순서
5. 구현 착수용 작업 문서
6. 실제 C++ 반영 결과 요약

---

## 문서 목록

### 1. `CF_VehicleMoveSpec.md`
- 차량 이동 입력 해석의 최종 규칙 명세
- 각도 기준, 영역 판정, 검은 영역 유지 정책, 장치 모드 정책 고정
- 후진 전환은 Chaos Vehicle `bUseAutoReverse` 기준으로 명시

### 2. `CF_VehicleMoveDesign.md`
- 현재 `ACFVehiclePawn / UCFVehicleDriveComp` 구조 위에 어떻게 안전하게 얹을지 정리한 구현 설계
- 수동 기어 강제 제거, 브레이크 기반 후진 전환 구조 포함

### 3. `CF_VehicleMoveTest.md`
- PIE 검증 절차
- 디버그 출력 항목
- `bUseAutoReverse` 기반 후진 전환 검증 기준

### 4. `CF_VehicleMoveTasks.md`
- 실제 구현 순서
- 파일별 작업 후보
- 완료 체크리스트
- A안 검증 완료 반영

### 5. `CF_VehicleMoveWork.md`
- 구현 시작 전에 바로 보는 실행형 작업 문서
- 수정 대상 파일, 단계별 산출물, 완료 조건 정리
- 실제 PIE 후진 확인 결과 반영

### 6. `CF_VehicleMoveCpp.md`
- 실제 C++ 수정 방향과 반영 결과 요약
- 제거한 방식과 유지한 방식 기록

---

## 먼저 읽는 순서
1. `CF_VehicleMoveSpec.md`
2. `CF_VehicleMoveDesign.md`
3. `CF_VehicleMoveWork.md`
4. `CF_VehicleMoveTest.md`
5. `CF_VehicleMoveTasks.md`
6. `CF_VehicleMoveCpp.md`

---

## 문서 운영 규칙
- 물리 키 이름은 문서에 고정하지 않는다.
- 좌스틱/우스틱 등 물리 장치 배치를 문서 기준선으로 고정하지 않는다.
- 입력 액션 결과값의 **해석 규칙**만 고정한다.
- 디폴트 각도값은 조정 가능 수치로 기록한다.
- 기존 `Throttle / Steering / Brake / Handbrake` 전달 구조는 유지한다.
- 후진 전환은 Chaos Vehicle `bUseAutoReverse` 기반으로 정리한다.
- 수동 기어 강제와 음수 스로틀 기반 reverse 강제는 재도입 전에 별도 검토한다.

---

## 변경 이력
- v0.4 (2026-04-16): A안 검증 결과 반영, `bUseAutoReverse` 중심 구조와 `CF_VehicleMoveCpp.md` 역할 설명 추가.
- v0.3 (2026-04-15): `CF_VehicleMoveWork.md` 추가 및 README 읽기 순서 갱신.
- v0.2 (2026-04-15): 문서 파일명을 더 직관적인 이름으로 정리하고 README 참조를 갱신.
- v0.1 (2026-04-15): InputPlan 폴더 신규 생성, 차량 입력 명세/구현/테스트/작업 문서 분리.
