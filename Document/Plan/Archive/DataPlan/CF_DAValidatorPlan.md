# CarFight Vehicle DA Validator C++ 설계 계획서 v0.1.1

- 작성일: 2026-06-23
- 대상 프로젝트: `UE/CarFight_Re`
- 대상 기능: `CFVehicleData` 입력 보조용 C++ 검증 헬퍼
- 후보 클래스: `UCFVDAValidator`
- 후보 파일:
  - 신규: `UE/Source/CarFight_Re/Public/CFVDAValidator.h`
  - 신규: `UE/Source/CarFight_Re/Private/CFVDAValidator.cpp`
- 상위 계획서: `Document/Plan/DataPlan/CF_DAFillToolPlan.md`
- 화면 설계 문서: `Document/Plan/DataPlan/CF_DAFillWizardUX.md`
- 실무 체크리스트: `Document/Plan/DataPlan/CF_DAFillChecklist.md`
- 문서 상태: Draft

---

## 1. 목적

이 문서는 `EUW_VDAWizard`가 사용할 C++ 검증 헬퍼 `UCFVDAValidator`의 설계 기준을 정의한다.

목표는 에디터 위젯 블루프린트 안에 검증 규칙을 흩어놓지 않고, `CFVehicleData` 검증 규칙을 C++ 함수로 모으는 것이다.

핵심 목적:

1. 새 차량 DA의 필수 참조 누락을 자동 검사한다.
2. 차체 메쉬의 휠 소켓 존재 여부를 자동 검사한다.
3. 레이아웃, Movement, WheelVisual, DriveState의 위험값을 경고한다.
4. 기준 DA와 대상 DA의 주요 차이를 비교한다.
5. 검사 결과를 `EUW_VDAWizard`에서 바로 표시할 수 있는 구조로 반환한다.

---

## 2. 범위

### 2.1 포함 범위

`UCFVDAValidator` v0.1 구현 후보 범위는 아래다.

- `UCFVehicleData` 대상 검사
- 필수 자산 참조 검사
- Static Mesh 소켓 존재 검사
- 레이아웃 앵커 값 기본 검사
- Movement 기본 수치 범위 검사
- WheelVisual 기본 수치 검사
- DriveState 기본 수치 검사
- 기준 DA와 대상 DA의 주요 필드 비교
- 검사 결과를 오류, 경고, 정보로 분류

### 2.2 제외 범위

이번 설계에서 제외하는 항목은 아래다.

- DA 값을 자동 수정하지 않는다.
- 기준 DA를 자동 덮어쓰지 않는다.
- 새 DA 자산 생성을 담당하지 않는다.
- PIE를 자동 실행하지 않는다.
- 차량 물리 결과를 자동 평가하지 않는다.
- 최적 튜닝값을 추천하지 않는다.
- 모든 `CFVehicleData` 필드를 완전 비교하지 않는다.

---

## 3. 제약

### 3.1 읽기 전용 원칙

v0.1의 `UCFVDAValidator`는 읽기 전용이어야 한다.

허용:
- 대상 DA 읽기
- 기준 DA 읽기
- Static Mesh 소켓 읽기
- 검사 결과 생성

금지:
- DA 필드 수정
- 자산 저장
- 기준 DA 변경
- 레이아웃 캡처 실행
- 에디터 Selection 변경

### 3.2 BP 위젯과 C++ 책임 분리

`EUW_VDAWizard`는 표시와 버튼 입력을 담당한다.

`UCFVDAValidator`는 검사 규칙과 결과 생성을 담당한다.

즉, 위젯 블루프린트는 아래 판단을 직접 하지 않는다.

- 어떤 필드가 오류인지
- 어떤 필드가 경고인지
- 어떤 소켓이 필수인지
- 어떤 Movement 값이 위험한지
- 기준 DA와 대상 DA 차이를 어떻게 요약할지

### 3.3 런타임 의존 최소화

검증 대상은 차량 Pawn 인스턴스가 아니라 `UCFVehicleData` 자산이다.

따라서 v0.1 검증은 레벨에 차량이 배치되어 있지 않아도 동작해야 한다.

---

## 4. 결정 사항

### 4.1 클래스 형태

후보 클래스는 `UBlueprintFunctionLibrary` 기반으로 둔다.

이유:
- 에디터 유틸리티 위젯(Editor Utility Widget)에서 호출하기 쉽다.
- 상태를 오래 들고 있을 필요가 없다.
- 검증 함수는 입력 DA를 읽고 결과 구조체를 반환하는 정적 함수 형태가 적합하다.

후보 클래스:
- `UCFVDAValidator`

후보 파일:
- `UE/Source/CarFight_Re/Public/CFVDAValidator.h`
- `UE/Source/CarFight_Re/Private/CFVDAValidator.cpp`

### 4.2 결과 심각도

검사 결과는 아래 심각도로 분리한다.

| 심각도 | 표시 이름 | 의미 |
|---|---|---|
| `Pass` | 정상 | 검사 통과 |
| `Info` | 정보 | 참고용 차이 또는 안내 |
| `Warning` | 경고 | 진행 가능하지만 의도 확인 필요 |
| `Error` | 오류 | 테스트 전 수정 필요 |
| `Blocked` | 보류 | 선행 조건이 없어 검사 불가 |

### 4.3 결과 항목 구조

검사 결과 항목은 아래 정보를 가져야 한다.

| 항목 | 의미 |
|---|---|
| 심각도 | 오류/경고/정보 구분 |
| 그룹 이름 | RequiredRefs, Socket, Layout, Movement 같은 검사 그룹 |
| 필드 경로 | `VehicleVisualConfig.ChassisMesh` 같은 내부 경로 |
| 표시 이름 | 사용자가 읽을 한글 표시명 |
| 메시지 | 문제가 무엇인지 설명 |
| 권장 조치 | 사용자가 다음에 해야 할 일 |

### 4.4 전체 리포트 구조

전체 검사 리포트는 아래 정보를 가져야 한다.

| 항목 | 의미 |
|---|---|
| 대상 DA 이름 | 검사한 대상 차량 DA 이름 |
| 기준 DA 이름 | 비교 기준 차량 DA 이름 |
| 전체 상태 | 가장 높은 심각도 |
| 오류 수 | Error 항목 수 |
| 경고 수 | Warning 항목 수 |
| 정보 수 | Info 항목 수 |
| 결과 목록 | 세부 검사 항목 배열 |
| 요약 문자열 | UI에 바로 표시할 짧은 요약 |

---

## 5. 후보 타입

후보 타입은 실제 구현 시 아래 방향으로 둔다.

### 5.1 `ECFVDASeverity`

역할:
- 검사 결과 심각도를 나타내는 enum이다.

후보 값:
- `Pass`
- `Info`
- `Warning`
- `Error`
- `Blocked`

### 5.2 `FCFVDAValidationItem`

역할:
- 검사 결과 한 줄을 표현한다.

필수 필드:
- 심각도
- 그룹 이름
- 필드 경로
- 표시 이름
- 메시지
- 권장 조치

### 5.3 `FCFVDAValidationReport`

역할:
- 대상 DA 전체 검사 결과를 묶는다.

필수 필드:
- 전체 상태
- 대상 DA 이름
- 기준 DA 이름
- 오류 수
- 경고 수
- 정보 수
- 결과 목록
- 요약 문자열

### 5.4 `FCFVDAFieldCompareItem`

역할:
- 기준 DA와 대상 DA의 특정 필드 차이를 표현한다.

필수 필드:
- 그룹 이름
- 필드 경로
- 표시 이름
- 기준 값 문자열
- 대상 값 문자열
- 차이 설명
- 위험도

비고:
- v0.1에서는 `FCFVDAValidationItem`만 먼저 구현하고, 비교 전용 구조체는 v0.2로 미룰 수 있다.

---

## 6. 후보 함수

### 6.1 전체 검사

후보 함수:
- `ValidateVehicleData`

역할:
- 대상 DA를 전체 검사하고 리포트를 반환한다.

입력:
- 대상 차량 DA
- 선택 사항: 기준 차량 DA

출력:
- `FCFVDAValidationReport`

Blueprint 표시명:
- `차량 DA 전체 검사 (Validate Vehicle Data)`

툴팁:
- 대상 차량 DA의 필수 참조, 소켓, 레이아웃, Movement, WheelVisual, DriveState 값을 검사하고 결과 리포트를 반환합니다.

### 6.2 필수 참조 검사

후보 함수:
- `ValidateRequiredReferences`

역할:
- 차체 메쉬, 휠 메쉬, Wheel Class 누락을 검사한다.

오류:
- 대상 DA가 없음
- `ChassisMesh` 없음
- `WheelMeshFL` 없음
- `FrontWheelClass` 없음
- `RearWheelClass` 없음

경고:
- `WheelMeshFR` 없음
- `WheelMeshRL` 없음
- `WheelMeshRR` 없음

Blueprint 표시명:
- `필수 참조 검사 (Validate Required References)`

툴팁:
- 대상 차량 DA에서 차체, 휠 메쉬, 전륜/후륜 Wheel Class 참조가 채워졌는지 검사합니다.

### 6.3 휠 소켓 검사

후보 함수:
- `ValidateWheelSockets`

역할:
- `VehicleVisualConfig.ChassisMesh`에 휠 앵커 소켓이 있는지 검사한다.

필수 소켓:
- `VehicleLayoutConfig.BodyWheelSocketFL`
- `VehicleLayoutConfig.BodyWheelSocketFR`
- `VehicleLayoutConfig.BodyWheelSocketRL`
- `VehicleLayoutConfig.BodyWheelSocketRR`

오류:
- `ChassisMesh` 없음
- 필수 소켓 없음

정보:
- 표준 소켓 이름이 아닌 이름을 사용 중

Blueprint 표시명:
- `휠 소켓 검사 (Validate Wheel Sockets)`

툴팁:
- 대상 차량 DA의 차체 메쉬에 Wheel_Anchor 계열 휠 앵커 소켓이 존재하는지 검사합니다.

### 6.4 레이아웃 검사

후보 함수:
- `ValidateLayoutConfig`

역할:
- `VehicleLayoutConfig`가 캡처 가능한 상태인지 검사한다.

오류:
- `bUseLayoutOverrides = true`인데 네 앵커 위치가 모두 0에 가까움

경고:
- `bUseLayoutOverrides = false`
- 앞/뒤 또는 좌/우 앵커 값이 의심됨

Blueprint 표시명:
- `레이아웃 설정 검사 (Validate Layout Config)`

툴팁:
- 대상 차량 DA의 휠 레이아웃 덮어쓰기 사용 여부와 네 휠 앵커 값이 테스트 가능한 상태인지 검사합니다.

### 6.5 Movement 검사

후보 함수:
- `ValidateMovementConfig`

역할:
- `VehicleMovementConfig`의 기본 수치 범위를 검사한다.

오류:
- `bUseMovementOverrides = true`인데 전륜/후륜 휠 반지름이 0 이하
- `bUseMovementOverrides = true`인데 전륜/후륜 휠 폭이 0 이하
- `EngineMaxRPM`이 `EngineIdleRPM`보다 작거나 같음

경고:
- 조향각이 지나치게 큼
- 중심질량 오버라이드가 켜져 있음
- 엔진 토크가 기준 DA와 크게 다름
- 마찰 배수가 기준 DA와 크게 다름

Blueprint 표시명:
- `Movement 설정 검사 (Validate Movement Config)`

툴팁:
- 대상 차량 DA의 VehicleMovementConfig 값 중 테스트 전 확인해야 할 위험 수치와 기본 범위 오류를 검사합니다.

### 6.6 WheelVisual 검사

후보 함수:
- `ValidateWheelVisualConfig`

역할:
- `WheelVisualConfig`의 기본 수치를 검사한다.

오류:
- `bUseWheelVisualOverrides = true`인데 `ExpectedWheelCount`가 0 이하

경고:
- `FrontWheelCountForSteering`이 `ExpectedWheelCount`보다 큼
- 4륜 기본 차량인데 `ExpectedWheelCount`가 4가 아님

Blueprint 표시명:
- `WheelVisual 설정 검사 (Validate Wheel Visual Config)`

툴팁:
- 대상 차량 DA의 WheelVisualConfig에서 예상 휠 개수와 조향 전륜 개수가 유효한지 검사합니다.

### 6.7 DriveState 검사

후보 함수:
- `ValidateDriveStateConfig`

역할:
- `DriveStateConfig`의 상태 판정값을 검사한다.

오류:
- `ActiveInputThreshold`가 0보다 작거나 1보다 큼
- 유지 시간이 음수

경고:
- `bUseDriveStateOverrides = false`
- Idle 진입/이탈 임계값 관계가 의심됨
- Reversing 진입/이탈 임계값 관계가 의심됨
- Airborne 임계값이 0에 너무 가까움

Blueprint 표시명:
- `DriveState 설정 검사 (Validate Drive State Config)`

툴팁:
- 대상 차량 DA의 DriveStateConfig에서 상태 전환 임계값과 최소 유지 시간이 테스트 가능한 범위인지 검사합니다.

### 6.8 기준 DA 비교

후보 함수:
- `CompareVehicleData`

역할:
- 기준 DA와 대상 DA의 주요 필드 차이를 검사 결과로 만든다.

초기 비교 대상:
- 엔진 토크
- 엔진 RPM
- 조향각
- 브레이크 토크
- 휠 반지름
- 마찰 배수
- 서스펜션 강성
- 중심질량 오버라이드
- DriveState 주요 임계값

Blueprint 표시명:
- `기준 DA와 비교 (Compare Vehicle Data)`

툴팁:
- 기준 차량 DA와 대상 차량 DA의 주요 튜닝값 차이를 비교해 정보 또는 경고 항목으로 반환합니다.

---

## 7. 검사 규칙 초안

### 7.1 오류 규칙

오류는 PIE 테스트 전에 반드시 수정해야 하는 상태다.

| 그룹 | 조건 | 메시지 |
|---|---|---|
| RequiredRefs | 대상 DA 없음 | 대상 차량 DA가 없습니다. |
| RequiredRefs | `ChassisMesh` 없음 | 차체 메쉬가 비어 있습니다. |
| RequiredRefs | `WheelMeshFL` 없음 | 앞왼쪽 휠 메쉬가 비어 있습니다. |
| RequiredRefs | `FrontWheelClass` 없음 | 전륜 Wheel Class가 비어 있습니다. |
| RequiredRefs | `RearWheelClass` 없음 | 후륜 Wheel Class가 비어 있습니다. |
| Socket | `ChassisMesh` 없음 | 차체 메쉬가 없어 소켓 검사를 할 수 없습니다. |
| Socket | 필수 소켓 없음 | 차체 메쉬에 필요한 휠 앵커 소켓이 없습니다. |
| Layout | 레이아웃 사용 중인데 앵커가 전부 0 | 레이아웃 덮어쓰기가 켜졌지만 휠 앵커 값이 비어 있습니다. |
| Movement | 휠 반지름 0 이하 | 휠 반지름은 0보다 커야 합니다. |
| Movement | 엔진 최대 RPM <= Idle RPM | 엔진 최대 RPM은 Idle RPM보다 커야 합니다. |
| WheelVisual | 예상 휠 개수 0 이하 | 예상 휠 개수는 1 이상이어야 합니다. |
| DriveState | 입력 임계값 범위 이탈 | ActiveInputThreshold는 0~1 범위여야 합니다. |

### 7.2 경고 규칙

경고는 진행 가능하지만 의도 확인과 테스트가 필요한 상태다.

| 그룹 | 조건 | 메시지 |
|---|---|---|
| RequiredRefs | FR/RL/RR 휠 메쉬 없음 | 일부 휠 메쉬가 비어 있습니다. 의도한 임시 설정인지 확인하세요. |
| Layout | 레이아웃 덮어쓰기 꺼짐 | DA 레이아웃을 사용하지 않고 BP 수동 배치를 유지합니다. |
| Movement | 중심질량 오버라이드 켜짐 | 중심질량 오버라이드는 전복과 롤링에 큰 영향을 줍니다. |
| Movement | 조향각이 큼 | 조향각이 크면 고속 불안정이 생길 수 있습니다. |
| WheelVisual | 조향 전륜 수가 예상 휠 수보다 큼 | 조향 전륜 수가 예상 휠 수보다 큽니다. |
| DriveState | DriveState 덮어쓰기 꺼짐 | 차량별 DriveState 설정이 적용되지 않습니다. |
| Compare | 기준 DA와 큰 차이 | 기준 DA와 차이가 큽니다. 의도한 변경인지 기록하세요. |

### 7.3 정보 규칙

정보는 작업자에게 현재 상태를 알려주는 참고 항목이다.

| 그룹 | 조건 | 메시지 |
|---|---|---|
| Socket | 표준 소켓명이 아님 | 표준 Wheel_Anchor 이름 대신 커스텀 소켓 이름을 사용 중입니다. |
| Compare | 기준 DA와 값이 다름 | 기준 DA와 다른 값입니다. |
| Movement | MovementProfileName 있음 | Movement 프로필 이름이 지정되어 있습니다. |

---

## 8. UI 연동 방식

`EUW_VDAWizard`는 아래 방식으로 `UCFVDAValidator`를 사용한다.

1. 사용자가 기준 DA와 대상 DA를 선택한다.
2. 위젯이 `ValidateVehicleData`를 호출한다.
3. 반환된 리포트의 결과 목록을 오른쪽 결과 영역에 표시한다.
4. 왼쪽 단계 목록은 그룹별 최고 심각도를 배지로 표시한다.
5. 사용자가 특정 결과를 선택하면 필드 도움말과 권장 조치를 표시한다.

위젯은 검사 규칙을 직접 갖지 않는다.

---

## 9. 구현 순서

### Phase 0. 설계 문서 작성

작업:
- 이 문서를 작성한다.

완료 기준:
- 후보 타입, 후보 함수, 검사 규칙이 문서화되어 있다.

### Phase 1. 결과 타입 추가

작업:
- `ECFVDASeverity` 추가
- `FCFVDAValidationItem` 추가
- `FCFVDAValidationReport` 추가

완료 기준:
- 블루프린트에서 검사 결과 배열을 읽을 수 있다.

### Phase 2. 필수 참조 검사 구현

작업:
- `ValidateRequiredReferences` 구현
- `ValidateVehicleData`에서 호출

완료 기준:
- 차체, 휠 메쉬, Wheel Class 누락이 오류/경고로 분리된다.

### Phase 3. 소켓/레이아웃 검사 구현

작업:
- `ValidateWheelSockets` 구현
- `ValidateLayoutConfig` 구현

완료 기준:
- 차체 소켓 누락과 레이아웃 앵커 비어 있음이 잡힌다.

### Phase 4. Movement/WheelVisual/DriveState 검사 구현

작업:
- `ValidateMovementConfig` 구현
- `ValidateWheelVisualConfig` 구현
- `ValidateDriveStateConfig` 구현

완료 기준:
- 기본 수치 오류와 위험값 경고가 리포트에 포함된다.

### Phase 5. 기준 DA 비교 구현

작업:
- `CompareVehicleData` 구현
- 주요 Movement/DriveState 필드 차이를 정보/경고로 반환

완료 기준:
- `DA_TestSedan`과 새 DA의 주요 차이를 위젯에서 볼 수 있다.

### Phase 6. `EUW_VDAWizard` 연동

작업:
- 위젯에서 `ValidateVehicleData` 호출
- 결과 목록 표시
- 단계 배지 표시

완료 기준:
- 위젯 블루프린트는 검사 규칙 없이 결과 표시만 담당한다.

---

## 10. 코드 작성 규칙

실제 C++ 구현 시 아래 규칙을 따른다.

- 모든 변수 바로 위에 변수 역할 주석을 작성한다.
- 모든 함수 바로 위에 함수 역할 주석을 작성한다.
- `UFUNCTION`에는 한국어(English) 병기 DisplayName을 작성한다.
- `UFUNCTION`에는 초보자가 이해할 수 있는 ToolTip을 작성한다.
- `UPROPERTY`에는 한국어 DisplayName과 ToolTip을 작성한다.
- 파일 상단에 Version, Date, Description, Scope, Changelog, Migration을 기록한다.
- `Public/Private` 경로 규칙을 지킨다.
- 새 파일명과 클래스명은 32자를 넘기지 않는다.

---

## 11. 검증 방법

### 11.1 문서 검증

- [ ] 목적, 범위, 제약, 결정 사항, 미결 사항, 검증 방법이 분리되어 있다.
- [ ] 후보 파일 경로가 `Public/Private` 규칙을 따른다.
- [ ] 후보 클래스명과 파일명이 32자를 넘지 않는다.
- [ ] `EUW_VDAWizard`와 책임이 겹치지 않는다.

### 11.2 코드 구현 후 검증

구현 후에는 아래 명령으로 에디터 빌드를 검증한다.

```powershell
Tools\BuildEditor.bat
```

검증 항목:

- [ ] 빌드 오류가 없다.
- [ ] 블루프린트에서 `차량 DA 전체 검사 (Validate Vehicle Data)` 함수를 찾을 수 있다.
- [ ] 대상 DA가 없을 때 오류 리포트가 반환된다.
- [ ] `DA_TestSedan`을 대상 DA로 넣었을 때 치명 오류가 없다.
- [ ] 필수 참조를 일부 비운 테스트 DA에서 오류/경고가 분리된다.

---

## 12. 미결 사항

아래 항목은 구현 전 또는 구현 중에 다시 결정한다.

- `FCFVDAFieldCompareItem`을 v0.1에 포함할지, v0.2로 미룰지 결정 필요
- 큰 차이 경고를 위한 수치 임계값을 고정할지, 기준 DA 대비 비율로 계산할지 결정 필요
- `UCFVDAValidator`를 런타임 모듈에 둘지, Editor 전용 모듈로 분리할지 장기 판단 필요
- 테스트 기록 저장 기능을 C++에서 담당할지, 위젯 문서 출력으로 유지할지 결정 필요
- 레이아웃 캡처 함수 직접 호출 버튼을 Validator 범위에 둘지 별도 Editor Action 범위로 둘지 결정 필요

현재 v0.1 판단:
- Validator는 읽기 전용 검증만 담당한다.
- 캡처 실행과 자산 수정은 Validator 범위 밖으로 둔다.

---

## 13. Changelog

### v0.1.1 - 2026-06-25

- `DA_PoliceCar` 폐기 결정에 맞춰 Validator 기준 비교와 대표 검증 대상을 `DA_TestSedan`으로 변경했다.

### v0.1.0 - 2026-06-23

- `UCFVDAValidator` C++ 검증 헬퍼 설계 계획서 최초 작성
- 후보 파일, 후보 클래스, 결과 타입, 후보 함수 목록을 정의
- 필수 참조, 소켓, 레이아웃, Movement, WheelVisual, DriveState 검사 규칙 초안을 작성
- `EUW_VDAWizard`와 C++ 검증 헬퍼의 책임 분리를 명시
- 읽기 전용 검증 원칙과 구현 단계별 계획을 추가

---

## 14. Migration 메모

- 이 문서는 `CF_DAFillToolPlan.md`의 Phase 3 산출물이다.
- 실제 `CFVDAValidator.h/.cpp` 구현 후 후보 타입과 함수명이 달라지면 이 문서를 갱신한다.
- `CF_DAFillWizardUX.md`의 함수 후보와 실제 C++ 함수명이 달라지면 두 문서를 함께 맞춘다.
- 검증 규칙이 바뀌면 `CF_DAFillChecklist.md`의 오류/경고 기준도 함께 갱신한다.
