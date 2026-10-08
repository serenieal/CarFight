# 타겟 선택 시스템 설계 로드맵

- 문서 버전: v0.11.2
- 상태: Active / M1 TS-P0-08 P0 검증과 튜닝
- 작성일: 2026-07-22
- 최근 갱신일: 2026-08-15
- 기능 ID: `CF-FQ-026`
- 기준 기획: `../Design/TargetSelect.md`
- 상세 작업: `TargetSelectPlan.md`
- 작업지시 규격: `TargetSelectWorkOrder.md`
- 구조 조사: `TargetSelectInvestigation.md`

---

## 1. 로드맵 목표

타겟 선택 시스템을 단일 대상 선택에서 시작해 센서, 유틸리티 장비, 부위 선택, 다중 타겟으로 확장한다.

각 단계는 이전 단계의 안정성을 전제로 하며, 후기 기능을 위해 P0 구조를 과도하게 복잡하게 만들지 않는다. 다만 선택·조준·장비 획득의 책임 경계는 P0부터 유지한다.

---

## 1-1. 현재 단계 체크포인트

```text
M0 문서 기획: 완료
M0 실제 구조 조사: Done
M0 공식 산출물: TargetSelectInvestigation.md
M1 TS-P0-01 계약·데이터·Pawn 최소 통합: Done
M1 TS-P0-01 Blueprint 에셋·UHT Reflection: PASS
M1 TS-P0-01 Automation Runtime Contract: PASS / Exit Code 0
M1 TS-P0-02 TargetPoint·Bounds·ActorLocation Fallback: Done
M1 TS-P0-02 Automation TargetPoint: PASS / Exit Code 0
M1 TS-P0-02 공식 빌드: Build Job c79b3418a3b2405b97ad52df7fa57bb0 / Exit Code 0
M1 TS-P0-03 후보 탐색·결정적 정렬·안정화: Done
M1 TS-P0-03 공식 빌드: Build Job 6b359cbce86449dea5cf35daacd15ed5 / 전체 3개 Automation PASS / Exit Code 0
M1 TS-P0-04 선택 수명: Done
M1 TS-P0-04 수명 경계: OnDestroyed + OnEndPlay + VehicleHealth.OnVehicleDestroyed
M1 TS-P0-04 공식 빌드: Build Job bb63b77b7a174afda89762fd0d2d712c / 전체 4개 Automation PASS / Exit Code 0
M1 TS-P0-05 입력과 플레이어 연결: Done
M1 TS-P0-05 입력 에셋: IA_SelectTarget + IA_ClearTarget + IMC_Vehicle_Default 4개 매핑
M1 TS-P0-05 공식 빌드: Build Job efdc298d3108451ca60a9906a7612333 / 전체 5개 Automation PASS / 경고 0 / Exit Code 0
M1 TS-P0-06 후보 및 선택 HUD: Done
M1 TS-P0-06 HUD 에셋: UCFTargetSelectWidget + WBP_TargetSelect / 8개 위젯 트리
M1 TS-P0-06 공식 빌드: Build Job 26ef764a7de94b9ab46b221fcbc47805 / 전체 6개 Automation PASS / 경고 0 / Exit Code 0
M1 TS-P0-07 장비 조회 연동: Done
M1 TS-P0-07 계약: CFTargetUseTypes + WeaponData.TargetUsePolicy + WeaponComp 선택 구독·결과 캐시
M1 TS-P0-07 공식 빌드: Build Job b7c2eb766e984c8ea6f5d3d847cee1f6 / 이동 사거리 갱신 포함 전체 7개 Automation PASS / 경고 0 / Exit Code 0
M1 현재 Task: TS-P0-08 P0 검증과 튜닝 / Active
M1 TS-P0-08 원격 기술: Latest Build PASS / SearchDiagnostics·SettingsPath·SingleTargetBoundary 3/3 / 최종 자산 비변경 TS-P0-01·02·03·04·07 각 1/1 PASS
M1 TS-P0-08 보호: dirty WBP_TargetSelect 때문에 Input/HUD 저장형 회귀 미실행 / 기존 USER 체크포인트 유지
다음: 원격 기술 체크포인트 정리 / 대표 workload 확정 전 full-world 20Hz scan 구조 교체 보류 → USER 가능 시 단일 차량 범위·debug Sphere·HUD 겹침·16:9/32:9 실제 검증
```

현재 C++ 계약과 자동화 테스트를 M1 후속 구현의 회귀 기준으로 재사용한다. M0 조사에서 `ACFVehiclePawn` 소유, `VehicleCameraComp` 직접 조준 기준, 기존 Enhanced Input과 별도 Target UI 경로를 확정했다.

---

## 2. 단계 요약

| 단계 | 목표 | 핵심 산출물 | 종료 기준 |
|---|---|---|---|
| M0 | 구현 경로 확정 | 조사 결과와 실제 파일 목록 | 작업지시 가능 |
| M1 | P0 수직 절편 | 단일 선택과 기본 HUD | 핵심 검증 통과 |
| M2 | 전투 편의 | 순환, 필터, 화면 밖 표시 | 실전 조작 가능 |
| M3 | 센서·장비 | 정보 단계와 장비 획득 | 전술 연동 가능 |
| M4 | 부위·다중 | 부위 선택과 다중 타겟 | 고급 전투 지원 |

---

## 3. M0 구현 준비

### 목표

기획을 실제 CarFight 구조에 매핑한다.

### 진입 조건

- `TargetSelect.md` 확정
- 추천 결정 채택
- 문서 저장 완료

### 주요 작업

- TS-P0-00 기존 구조 조사
- 실제 Pawn/Controller/HUD/입력/장비 경로 확인
- 재사용 가능한 인터페이스와 컴포넌트 확인
- 충돌 전략 결정
- P0 네트워크 범위 결정
- 신규 파일 이름 확정

### 종료 상태

```text
Done / 2026-07-23
공식 산출물: TargetSelectInvestigation.md
```

확정된 핵심 결정:

- `UCFTargetSelectComp` 소유는 `ACFVehiclePawn`
- 차량 선택 가능 계약은 `ACFVehiclePawn`의 `ICFTargetSelectable` C++ 구현
- 직접 선택 기준은 `UCFVehicleCameraComp`의 카메라 Aim
- 선택 입력은 기존 `IMC_Vehicle_Default`에 신규 Action으로 추가
- Target HUD는 기존 Aim Reticle과 분리
- 차량 파괴는 `VehicleHealthComp.OnVehicleDestroyed` 재사용
- 신규 `TargetSelect` Trace Channel 사용
- P0는 로컬 싱글, 복제 없음

Camera Aim Trace 시작 위치, 최종 키 매핑, Target UI 소유와 ZOrder는 TS-P0-03·05·06에서 확정됐다. M0 후속 세부 항목은 모두 담당 Task에서 해소됐으며 M0 종료 상태는 유지한다.

### 주요 리스크

- 문서와 실제 프로젝트 구조 불일치
- 기존 기능 중복
- 로컬 미커밋 변경과 충돌

### 보류 항목

- 최종 튜닝 수치
- 센서 상세 모델
- 다중 타겟 네트워크 구조

---

## 4. M1 P0 수직 절편

### 목표

직접 조준 우선과 크로스헤어 근접 선택이 실제 플레이에서 동작하고, 선택 대상이 HUD와 장비 조회 API까지 연결된다.

### 포함 Task

- TS-P0-01 계약 및 데이터
- TS-P0-02 타겟 포인트
- TS-P0-03 후보 탐색
- TS-P0-04 선택 수명
- TS-P0-05 입력 연결
- TS-P0-06 HUD
- TS-P0-07 장비 조회
- TS-P0-08 검증과 튜닝

### 진입 조건

- M0 종료
- 테스트 차량과 테스트 맵 준비
- 크로스헤어 기준 위치 확인

### 종료 조건

- 직접 조준 대상이 항상 우선
- 직접 대상이 없을 때 화면상 중앙에 가까운 대상 선택
- 선택 유지·해제·무효화 정상
- 후보 흔들림이 허용 범위
- 후보와 선택 HUD 구분
- 장비가 선택 대상을 안전하게 조회
- P0 검증 시나리오 통과
- 성능상 중대한 문제 없음

### 단계 완료 후 잠금

- 핵심 API
- 선택 상태 소유권
- 후보 계층형 비교 순서
- 타겟 포인트 계약
- UI 이벤트 계약

---

## 5. M2 전투 편의

### 목표

고속 차량 전투에서 타겟을 잃지 않고 의도한 대상을 빠르게 다시 선택할 수 있게 한다.

### 주요 기능

- 다음·이전 후보 순환
- 적대 대상 필터
- 가장 가까운 적 명령
- 화면 밖 방향 표식
- 가림 상태 UI
- 관계 및 장비 호환 표시
- 후보 디버그 시각화
- 플레이테스트용 튜닝 UI

### 진입 조건

- M1 핵심 API 변경 가능성이 낮음
- 실제 전투 플레이테스트 가능
- HUD 레이아웃 기준 존재

### 종료 조건

- 화면 안·밖 이동 시 정보가 일관됨
- 적 필터가 기본 선택 규칙을 침범하지 않음
- 후보 순환 순서가 예측 가능
- 색상 외 형태로 상태 구분
- 디버그 도구로 후보 탈락 이유 확인 가능

### 주요 리스크

- HUD 과밀
- 순환 목록과 실시간 후보 목록 불일치
- 화면 밖 표식이 센서 가치를 약화

---

## 6. M3 센서와 장비 획득

### 목표

선택 대상을 센서 정보 단계와 미사일·해킹·수리·견인 장비의 획득 흐름에 연결한다.

### 주요 기능

- 감지·식별·스캔 정보 단계
- 시야 확보와 센서 추적 구분
- 마지막 확인 위치
- 신호 불안정과 소실
- 장비별 획득 진행도
- 장비별 고정 상태
- 사용 불가 사유
- 위협도 기반 별도 명령

### 진입 조건

- 장비 시스템의 공통 계약 존재
- 센서 능력치 또는 최소 임시 모델 존재
- M1 선택 API 안정화

### 종료 조건

- 선택 상태와 장비 고정 상태가 독립적으로 동작
- 허용된 센서 정보만 표시
- 장비별 조건이 동일 선택 대상을 각자 평가
- 대상 소실과 재획득 흐름 검증
- 직접 조준 무기 감각이 훼손되지 않음

### 주요 리스크

- 센서 시스템 범위 팽창
- 장비별 중복 로직
- 네트워크 상태 동기화 복잡도

---

## 7. M4 부위와 다중 타겟

### 목표

차량의 바퀴·엔진·무기 등 부위를 선택하고 여러 장비가 여러 타겟을 관리할 수 있게 한다.

### 주요 기능

- 대상과 부위 선택 계층
- 부위 타겟 포인트
- 부위 정보 공개
- 다중 락온
- 타겟 목록 UI
- 분대 타겟 공유
- 임무 타겟 핀

### 진입 조건

- 부위 손상 시스템 계약 존재
- M3 정보 공개 모델 안정화
- 다중 장비 운용 요구 확정

### 종료 조건

- 상위 대상 선택과 부위 선택이 충돌하지 않음
- 부위가 사라져도 상위 대상 참조 안전
- 다중 타겟 UI가 전투 시야를 과도하게 가리지 않음
- 네트워크 권한과 동기화 규칙 검증

### 주요 리스크

- UI 복잡도 급증
- 부위 수 증가에 따른 후보 검색 비용
- 다중 타겟과 장비 슬롯 소유권 충돌

---

## 8. 단계 간 의존성

```text
M0 구조 조사
  ↓
M1 단일 선택 수직 절편
  ├─→ M2 전투 편의
  └─→ M3 센서·장비 획득
          ↓
      M4 부위·다중 타겟
```

M2와 M3 일부는 병행 가능하지만, 선택 API와 HUD 이벤트 계약이 M1에서 잠긴 이후에 시작한다.

---

## 9. 중단 및 재검토 조건

다음 조건이 발생하면 현재 Task를 확장하지 말고 로드맵을 재검토한다.

- 기존 프로젝트에 동일 책임의 시스템이 발견됨
- 타겟 선택이 서버 권한이어야 하는 요구가 새로 확정됨
- 현재 HUD 구조가 월드 마커를 지원하지 않음
- 후보 검색이 목표 성능을 충족하지 못함
- 센서 시스템이 별도 프로젝트 단위로 커짐
- 부위 손상 시스템의 데이터 모델이 변경됨
- 사용자 조작 테스트에서 직접 조준 우선 규칙이 불편하다고 확인됨

---

## 10. 문서 게이트

각 단계 시작 전 다음 문서 상태를 확인한다.

| 단계 | 필수 문서 |
|---|---|
| M0 | TargetSelect.md |
| M1 | TargetSelectInvestigation.md, TargetSelectPlan.md, TS-P0-01 작업지시 |
| M2 | P0 검증 결과, P1 작업지시 |
| M3 | 센서 계약 문서, 장비 계약 문서 |
| M4 | 부위 손상 문서, 다중 타겟 UX 문서 |

---

## 11. 변경 이력

### v0.11.2 - 2026-08-15

- 저장된 BP CDO 실제 설정 source와 7도·1200m·15% fallback 값을 Technical evidence로 확정했다.
- 런타임 후보 검색 진단과 의미 보존 LOS 사전필터를 추가해 후보 배열은 유지하면서 불필요한 LOS Trace를 줄였다.
- 최신 Build PASS, TS-P0-08 3/3, 최종 자산 비변경 TS-P0-01·02·03·04·07 각 1/1 PASS를 기록했다.
- 20Hz 전체 Actor 순회는 대표 workload가 없는 상태에서 구조 교체하지 않고 남은 scalability 항목으로 유지한다.
- USER 범위 체감·debug 가시성·HUD·화면비 검증은 미완료 상태를 유지한다.

### v0.11.1 - 2026-08-15

- 사용자 지시로 TS-P0-08을 Active로 재개하고 원격에서 가능한 기술 검증부터 수행했다.
- 공식 Build `2aa5462fbd5445379416423210054fec` PASS, SingleTargetBoundary 1/1과 자산 비변경 TS-P0-01·02·03·04·07 회귀를 각각 1/1 PASS로 확인했다.
- 7도 단일 후보 경계는 16:9·32:9에서 좌우 대칭임을 고정했으며 실제 후보 범위 체감값은 변경하지 않았다.
- 자동 후보 debug Sphere를 후보 갱신 주기 동안 유지하도록 보강했지만 실제 시각성은 USER Pending으로 유지한다.
- dirty WBP_TargetSelect 보호 때문에 Input/HUD 저장형 테스트와 전체 suite는 이번 재개에서 실행하지 않았다.

### v0.11.0 - 2026-07-24

- 장비별 선택 대상 사용 조건을 공용 정책·요청·결과와 11개 실패 사유로 분리했다.
- TargetSelectComp를 선택 상태의 단일 소유자로 유지하고 장비는 읽기 전용 평가만 수행하도록 M1 계약을 잠갔다.
- WeaponData에 TargetUsePolicy를 추가하고 WeaponComp를 선택 변경·해제·유효성·추적 상태 이벤트와 0.10초 저빈도 거리 재평가 기반 첫 소비 예제로 연결했다.
- 선택 존재·유효성과 장비 준비·호환·거리·최종 사용 가능 상태를 분리하고 장비 평가 실패가 공용 선택을 변경하지 않도록 했다.
- 직접 조준 FireOrigin, AimSolution과 발사 방향이 선택 대상 평가로 자동 보정되지 않음을 Automation으로 확인했다.
- 최종 Build Job `b7c2eb766e984c8ea6f5d3d847cee1f6`에서 선택 대상 이동 사거리 이탈·복귀를 포함한 TS-P0-01~07 전체 7개 Automation 성공, 경고 0, 오류 0, Exit Code 0을 확인했다.
- TS-P0-07을 Done으로 전환하고 M1 현재 Task를 TS-P0-08 P0 검증과 튜닝으로 이동했다.

### v0.10.0 - 2026-07-24

- TargetSelect HUD를 기존 Aim Reticle과 별도 Viewport 위젯으로 구현하고 C++ 상태·투영과 Blueprint 배치·스타일 책임을 분리했다.
- 후보와 선택 대상의 이름·거리, 선택 관계와 추적 상태를 표시하고 후보 `◇`, 선택 `▣`, 가림 `▧` 형태 구분을 추가했다.
- 같은 Actor의 후보·선택 중복 표시를 억제하고 화면 밖에서는 마커만 숨기며 선택 상태를 유지하도록 M1 HUD 정책을 잠갔다.
- Pawn에 TargetSelect HUD 기본 클래스, 표시 토글, ZOrder 20과 BeginPlay·Input 준비·EndPlay 수명을 연결했다.
- Build Job `26ef764a7de94b9ab46b221fcbc47805`에서 TS-P0-01~06 전체 6개 Automation 성공, 경고 0, 오류 0, Exit Code 0을 확인했다.
- UI AssetDump 7/7 성공과 WBP_TargetSelect의 C++ 부모 및 8개 위젯 트리를 확인했다.
- TS-P0-06을 Done으로 전환하고 M1 현재 Task를 TS-P0-07 장비 조회 연동으로 이동했다.

### v0.9.0 - 2026-07-24

- `ACFVehiclePawn`에 선택·해제 Input Action, 현재 후보 선택 확정과 Manual 선택 해제 명령을 연결했다.
- `IA_SelectTarget`, `IA_ClearTarget`을 생성하고 `IMC_Vehicle_Default`에 가운데 마우스·오른쪽 스틱 클릭 선택과 오른쪽 마우스·FaceButton Right 해제를 저장했다.
- 후보 없음 선택 입력은 기존 선택을 유지하고 Manual 해제는 후보를 보존하며 자동 다음 타겟을 발생시키지 않도록 잠갔다.
- Build Job `efdc298d3108451ca60a9906a7612333`에서 TS-P0-01~05 전체 5개 Automation 성공, 경고 0, 오류 0, Exit Code 0을 확인했다.
- Input AssetDump 15/15 성공을 확인하고 TS-P0-05를 Done으로 전환했다.
- M1 현재 Task를 TS-P0-06 후보 및 선택 HUD로 이동하고 Aim Reticle·입력·장비와의 책임 분리를 유지했다.

### v0.8.0 - 2026-07-24

- Automation Actor의 `Destroy()`를 `OnDestroyed`로 즉시 처리하고 `OnEndPlay`와 VehicleHealth 파괴 경계를 함께 유지하도록 TS-P0-04 수명 처리를 보강했다.
- Build Job `bb63b77b7a174afda89762fd0d2d712c`에서 UHT와 Editor 컴파일 성공을 확인했다.
- TS-P0-01~04 전체 4개 Automation이 성공하고 실패 0, 경고 0, 오류 0, Exit Code 0임을 확인했다.
- TS-P0-04를 Done으로 전환하고 M1 현재 Task를 TS-P0-05 입력과 플레이어 연결로 이동했다.
- 다음 범위를 신규 입력 Action, 기존 Mapping Context와 Pawn 입력 처리 연결로 고정하고 HUD·장비 책임을 제외했다.

### v0.7.0 - 2026-07-24

- TS-P0-03의 TargetSelect 전용 Trace, 카메라 Aim 후보 수집, 화면 근접도 우선의 결정적 정렬과 후보 안정화를 M1 완료 기반으로 등록했다.
- Build Job `6b359cbce86449dea5cf35daacd15ed5`에서 TS-P0-01~03 전체 3개 Automation 성공과 Exit Code 0을 TS-P0-03 공식 종료 증거로 기록했다.
- TS-P0-04 선택 대상 EndPlay·VehicleHealth 파괴 구독, 가림 유예, 거리 이탈과 시스템 비활성화 해제 구현 및 Editor 컴파일 성공을 기록했다.
- Build Job `7204a1766a084306aedd3d274017b091`의 최신 통합 결과를 4개 중 2 성공 / 2 실패, 경고 0 / 오류 7, 최종 Exit Code 255로 기록했다.
- M1 현재 Task를 Actor `Destroy()` 직후 선택 자동 해제 회귀 수정으로 유지하고 TS-P0-05 진입을 보류했다.

### v0.6.0 - 2026-07-24

- `UCFTargetPointComp`와 `TargetPoint → Actor Bounds → Actor Location` 공용 위치 해석을 M1 완료 기반으로 등록했다.
- Build Job `c79b3418a3b2405b97ad52df7fa57bb0`에서 Editor 빌드 성공과 TargetSelect 전체 Automation Exit Code 0을 확인했다.
- `TS-P0-01 RuntimeContract`, `TS-P0-02 TargetPoint` 전체 2개 테스트가 성공하고 실패·경고·오류가 없음을 공식 증거로 기록했다.
- M1 현재 Task를 TS-P0-03 후보 탐색과 정렬로 유지하고 다음 범위를 카메라 Aim 직접 조준 우선과 화면 중앙 근접 후보 평가로 고정했다.

### v0.3.0 - 2026-07-23

- `TargetSelectInvestigation.md`를 M0 공식 산출물로 등록하고 M0 구조 조사를 Done으로 전환했다.
- M1 현재 단계를 TS-P0-01 Blueprint 계약 검증과 `ACFVehiclePawn` 최소 통합으로 갱신했다.
- 직접 선택 조준, 입력, HUD, 파괴 상태, 관계, 장비와 충돌 경계의 확정 결정을 로드맵에 연결했다.
- M1 문서 게이트에 구조 조사 문서를 추가했다.

### v0.2.0 - 2026-07-23

- `CF-FQ-026` Active 상태와 M1 진입 준비 체크포인트를 추가했다.
- TS-P0-01 소스 작성과 Editor 빌드 PASS, Blueprint·PIE Pending을 기록했다.
- M0 조사 산출물은 재조사로 소스를 폐기하지 않고 현재 상태 기준으로 복구하도록 정했다.

### v0.1.0 - 2026-07-22

- M0~M4 로드맵 작성
- 진입·종료 조건과 리스크 정의
- 단계 간 의존성과 중단 조건 추가

---

## 12. 마이그레이션

### v0.11.0 적용 안내

- 장비 시스템은 TargetSelectComp의 선택을 복제하거나 별도 보유하지 않고 `FCFTargetUseRequest`로 현재 선택을 평가한다.
- 선택 대상 존재와 장비 사용 가능 여부는 별도 상태이며 장비 비호환·준비 실패가 선택 해제로 이어지지 않는다.
- WeaponData의 빈 TargetUsePolicy는 기존 에셋 호환을 위해 제한 없음으로 해석하고 최대 사용 거리 0 이하는 거리 제한 없음으로 처리한다.
- WeaponComp의 결과 캐시와 이벤트는 장비 호환 피드백용이며 장비별 획득·고정 상태는 M3 범위로 유지한다.
- 직접 조준 무기의 FireOrigin, AimSolution, 터렛 방향과 발사 방향은 선택 대상 평가에서 변경하지 않는다.
- TS-P0-08은 실제 PIE에서 입력·후보·선택·HUD·장비 평가·파괴 수명을 통합하고 화면비·UI Scale·성능을 확인한다.

### v0.10.0 적용 안내

- `WBP_TargetSelect`는 Aim Reticle과 독립된 HUD이며 TargetSelectComp 이벤트와 TargetPoint 결과만 소비한다.
- 후보·선택·가림은 형태와 색상을 함께 사용하고, 동일 Actor의 후보 마커는 선택 마커와 중복 표시하지 않는다.
- P0 화면 밖 정책은 마커 숨김과 선택 상태 유지이며 방향 표시는 M2의 화면 밖 표시 범위로 유지한다.
- TS-P0-07은 HUD 계층을 변경하지 않고 장비가 선택 대상과 표시 정보를 안전하게 조회하는 계약만 추가한다.
- 선택은 직접 조준 무기의 발사 방향을 자동 보정하지 않으며 장비 사용 가능 여부와 별도 상태로 유지한다.
- 실제 HUD 가독성과 화면비 검증은 TS-P0-08 통합 PIE에서 수행한다.

### v0.9.0 적용 안내

- TS-P0-05는 Done이며 후속 Task에서 Input Action, Mapping Context 또는 Pawn 선택 명령을 다시 구현하지 않는다.
- 선택 기본키는 `MiddleMouseButton`과 `Gamepad_RightThumbstick`, 해제 기본키는 `RightMouseButton`과 `Gamepad_FaceButton_Right`다.
- 후보 없음 유지, Manual 해제 시 후보 보존과 자동 다음 타겟 금지 정책을 HUD와 장비 연동에서도 유지한다.
- TS-P0-06은 기존 Aim Reticle에 타겟 표시를 혼합하지 않고 별도 TargetSelect 위젯이 TargetSelectComp 이벤트를 소비하도록 설계한다.
- 실제 입력 감각, UI 포커스, 재시작과 Possess 변경은 TS-P0-08 통합 PIE 종료 게이트로 유지한다.

### v0.8.0 적용 안내

- TS-P0-04는 Done이며 선택 수명, 가림 유예, 거리 이탈과 파괴 해제 경로를 후속 Task에서 재구현하지 않는다.
- 명시적 파괴는 `OnDestroyed`, 일반 EndPlay와 스트리밍 제거는 `OnEndPlay`, 차량 전투 파괴는 `VehicleHealth.OnVehicleDestroyed`를 사용한다.
- TS-P0-05는 기존 Enhanced Input 경계 안에서 선택과 수동 해제 입력만 연결한다.
- 후보가 없을 때 선택 입력은 기존 선택을 유지하고 자동 다음 타겟을 발생시키지 않는다.
- HUD 표시와 장비 조회는 각각 TS-P0-06과 TS-P0-07까지 분리한다.

### v0.7.0 적용 안내

- TS-P0-03은 Done이며 M1 후속 작업은 확정된 후보 정렬 순서와 TargetPoint 계약을 재구현하지 않는다.
- TS-P0-04는 UHT와 Editor 컴파일은 통과했지만 Automation 종료 게이트를 통과하지 못했으므로 Active 상태를 유지한다.
- 최신 실패는 Actor `Destroy()` 직후 OnEndPlay 기반 선택 자동 해제가 같은 테스트 흐름에서 관찰되지 않은 범위다.
- EndPlay 전달 시점 또는 Automation 월드 진행 절차를 보정하고 전체 TargetSelect 테스트가 PASS한 뒤에만 TS-P0-05 입력 연결로 이동한다.
- 자동 다음 타겟 금지와 화면 밖 선택 유지 정책은 수정 과정에서도 유지한다.

### v0.6.0 적용 안내

- TS-P0-02는 완료됐으며 후속 Task는 TargetPoint 컴포넌트나 위치 Fallback을 다시 구현하지 않는다.
- 기존 차량은 `bUseAsTargetPoint=false` 기본값으로 Bounds 중심을 유지하고, 차량별 명시 위치가 필요한 Blueprint만 TargetPoint를 이동하고 활성화한다.
- TS-P0-03 후보 평가는 공용 TargetPoint 결과를 사용하고 입력, HUD, 선택 수명과 장비 연동 책임을 섞지 않는다.
- 공식 검증 증거는 Build Job `c79b3418a3b2405b97ad52df7fa57bb0`과 `UE/Saved/Automation/TargetSelect/index.json`이다.

### v0.3.0 적용 안내

- M0는 완료 상태이며 같은 구조 조사를 반복하지 않는다.
- M1 구현은 `TargetSelectInvestigation.md`의 실제 경로와 책임 배분을 따른다.
- TS-P0-01에서 검증할 최소 통합은 Pawn 컴포넌트 생성, getter와 차량 선택 가능 기본 구현이다.
- 후보 Trace, TargetPoint, 입력과 HUD는 각각 정의된 후속 Task에서 수행한다.
- Blueprint·PIE PASS 전에는 TS-P0-01 종료 조건을 만족한 것으로 보지 않는다.

### v0.2.0 적용 안내

- M0 조사 결과 복구와 TS-P0-01 검증을 병행 가능한 문서 정렬 작업으로 본다.
- 현재 C++ 계약은 M1 입력으로 유지하며 검증 실패가 확인된 부분만 수정한다.
- Blueprint·PIE PASS 전에는 M1의 TS-P0-01 종료 조건을 만족한 것으로 보지 않는다.
- 단계 완료 시 상위 기획의 확정 결정을 변경해야 한다면 먼저 `TargetSelect.md` 버전을 올리고 변경 근거와 영향 범위를 기록한다.
