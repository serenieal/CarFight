# Reticle / FireFeedback VisualGuide

- Version: 0.1.0
- Date: 2026-07-09
- Status: Draft / Pre-Implementation Visual Direction
- Scope: Reticle / FireFeedback 구현 전 시각 방향, 상태별 모양, 색상, 표시 우선순위, WBP 구성 기준

---

## 1. 문서 목적

이 문서는 `Reticle / FireFeedback` 구현 전에 어떤 모양의 Reticle을 만들지 정리하기 위한 시각 가이드다.

현재 문서는 최종 UI 아트 사양서가 아니다.
구현 전에 C++ 상태 설계와 WBP 제작 방향이 어긋나지 않도록, **Reticle의 기본 형태와 상태별 피드백 표현 기준**을 정리하는 초안이다.

현재 CarFight의 전투 구현 기준은 **싱글플레이 로컬 차량 전투**다.
따라서 이 문서에서 Reticle은 서버 대기 / 서버 거부 상태를 표시하지 않는다.
`FirePending`은 로컬 발사 처리 피드백이고, `FireRejected`는 로컬 발사 조건 미충족 피드백이다.

---

## 2. 기준 이미지

이번 VisualGuide의 기준 이미지는 2026-07-09 대화에서 생성한 `Vehicle Reticle 예시` 컨셉 시트를 참고한다.

이미지의 역할:

```text
- 최종 UI 아트 확정안이 아니다.
- WBP 제작 전 Reticle 상태별 형태와 색상 방향을 이해하기 위한 시각 참고 자료다.
- 실제 구현에서는 더 단순한 형태로 시작한다.
```

이미지에서 사용할 수 있는 핵심 방향:

```text
- 중앙 조준점은 작고 명확하게 유지한다.
- 좌우 브라켓 형태로 차량 조준 Reticle의 방향성을 만든다.
- 기본 조준 상태는 흰색/회색 계열로 차분하게 표시한다.
- 발사 결과와 조건 미충족은 색상과 짧은 애니메이션으로 즉시 피드백한다.
- OutOfArcWarning은 주 상태를 덮어쓰는 오류 상태가 아니라 보조 경고로 표시한다.
```

---

## 3. Reticle 기본 형태

현재 P0 기준 Reticle은 아래 형태를 기본으로 한다.

```text
       ┌       ┐

          •

       └       ┘
```

실제 WBP에서는 아래 요소로 나눈다.

```text
- 중앙 점 또는 작은 원
- 좌측 브라켓
- 우측 브라켓
- 선택적 상하 짧은 Tick
- 선택적 탄 퍼짐 / 산탄 범위 원
- 상태 아이콘 또는 보조 경고 아이콘
- 짧은 상태 텍스트
```

P0에서는 너무 복잡한 조준선을 만들지 않는다.
차량 전투는 화면 중앙의 정보량이 많아질 수 있으므로, Reticle은 **읽기 쉽고 단순한 구조**를 우선한다.

---

## 4. 공통 시각 원칙

Reticle의 공통 원칙은 아래와 같다.

```text
1. 화면 중앙을 과하게 가리지 않는다.
2. 배경이 밝아도 어두워도 읽혀야 한다.
3. 기본 상태는 과하게 화려하지 않아야 한다.
4. 발사 결과는 짧고 명확해야 한다.
5. 실패 피드백은 오래 남기지 않는다.
6. 색상은 의미 전달용으로만 사용하고 장식용으로 남발하지 않는다.
7. PC / 콘솔 환경 모두에서 읽히는 굵기와 크기를 유지한다.
8. UI가 전투 판정을 바꾸는 것처럼 보이면 안 된다.
```

---

## 5. 상태 분류

Reticle 시각 상태는 크게 두 계층으로 나눈다.

```text
1. Aim Reticle 상태
   - 현재 조준 상태를 표시한다.
   - VehicleAimComp의 LocalAimState / ReticleState가 기준이다.

2. FireFeedback 상태
   - 최근 WeaponFire 결과를 짧게 표시한다.
   - LastFireResult / RejectReason / Cooldown 값이 기준이다.
```

즉 기본 조준 상태 위에 FireFeedback이 잠깐 덮어씌워지는 구조다.

```text
기본 Reticle = 계속 보이는 조준 상태
FireFeedback = 짧게 나타나는 발사 결과 오버레이
```

---

## 6. Aim Reticle 상태별 시각 기준

### 6.1 Ready

의미:

```text
- 로컬 조준 상태가 정상이다.
- 현재 기준에서 발사 가능 예측 상태로 볼 수 있다.
```

표현:

```text
- 색상: 흰색 또는 약한 녹색 포인트
- 중앙점: 작고 선명하게 표시
- 좌우 브라켓: 얇은 흰색 라인
- 텍스트: 기본적으로 표시하지 않거나 Debug 옵션에서만 표시
```

추천 형태:

```text
   [      •      ]
```

주의:

```text
- Ready 상태가 항상 최종 발사 성공을 보장하는 것은 아니다.
- 실제 발사 성공 여부는 WeaponFire의 ValidateFireCommand 결과가 기준이다.
```

---

### 6.2 Blocked / AimBlocked

의미:

```text
- 조준선이 막혔다.
- 발사 조건에서 AimBlocked로 거부될 수 있다.
```

표현:

```text
- 색상: 주황색
- 중앙 아이콘: X 또는 막힘 표시
- 브라켓: 주황색으로 변경
- 텍스트 후보: 조준 가림
```

추천 형태:

```text
   [      ×      ]
```

주의:

```text
- Blocked는 플레이어가 즉시 이해해야 하므로 Ready보다 강한 피드백을 준다.
- 다만 화면을 과하게 흔들거나 가리는 연출은 P0에서 피한다.
```

---

### 6.3 OutOfArc / OutOfArcWarning

의미:

```text
- 현재 조준 방향이 기준 AimProfile 또는 무기 조준각 범위 밖이다.
- 현재 P0 싱글플레이 기준에서는 단독 발사 차단 조건이 아니다.
```

표현:

```text
- 색상: 노란색
- 주 Reticle은 Ready 또는 기존 상태를 유지할 수 있다.
- 보조 경고 아이콘 또는 작은 경고 텍스트를 추가한다.
- 텍스트 후보: 각도 경고
```

추천 형태:

```text
   [      •      ]    △
```

중요:

```text
OutOfArcWarning은 주 Reticle 상태를 무조건 덮어쓰지 않는다.
Ready + 각도 경고 보조 표시가 가능해야 한다.
```

---

### 6.4 NoWeapon

의미:

```text
- 사용할 수 있는 무기가 없다.
- 무기 데이터가 없거나 호환되지 않을 수 있다.
```

표현:

```text
- 색상: 회색
- 중앙 아이콘: 금지 표시 또는 슬래시 원
- 브라켓: 흐리게 표시
- 텍스트 후보: 무기 없음
```

추천 형태:

```text
   [      ⊘      ]
```

주의:

```text
- NoWeapon은 플레이어에게 명확해야 한다.
- 단순히 Ready가 안 보이는 상태로 만들면 원인을 알 수 없다.
```

---

## 7. FireFeedback 상태별 시각 기준

### 7.1 FireSuccess

의미:

```text
- WeaponFire가 발사를 승인했다.
- Projectile Actor 또는 Dummy HitScan 경로로 실행되었을 수 있다.
```

표현:

```text
- 색상: 녹색 또는 밝은 흰색 플래시
- 중앙점: 순간적으로 커졌다가 원래 크기로 복귀
- 브라켓: 짧게 밝아짐
- 유지 시간 후보: 0.10 ~ 0.15초
- 텍스트 후보: 발사 또는 텍스트 없음
```

추천 형태:

```text
   [     ✦•✦     ]
```

주의:

```text
- 발사 성공 피드백은 짧아야 한다.
- 지속적으로 초록색 상태가 유지되면 Ready 상태와 혼동될 수 있다.
```

---

### 7.2 FirePending

의미:

```text
- 로컬 Fire 입력 직후 짧게 표시할 수 있는 발사 처리 피드백이다.
- 서버 응답 대기 상태가 아니다.
```

표현:

```text
- 색상: 파란색 계열
- 중앙 아이콘: 작은 모래시계 또는 짧은 회전 표시 후보
- 유지 시간: 매우 짧게
- 텍스트 후보: 발사 처리 중
```

추천 형태:

```text
   [      ⧖      ]
```

주의:

```text
- 현재 싱글플레이 로컬 기준에서는 FirePending을 길게 보여줄 필요가 거의 없다.
- P0에서는 생략하거나 매우 짧게만 표시해도 된다.
```

---

### 7.3 Cooldown

의미:

```text
- 활성 무기가 쿨다운 중이다.
- WeaponCooldown으로 거부되었거나 남은 쿨다운 시간이 있다.
```

표현:

```text
- 색상: 파란색
- 중앙 또는 하단에 남은 시간 표시
- 브라켓 또는 원형 라인을 쿨다운 비율로 줄이는 후보
- 텍스트 후보: 재사용 대기
```

추천 형태:

```text
   [     1.23     ]
```

P0 추천:

```text
- 복잡한 원형 게이지보다 숫자 텍스트를 먼저 구현한다.
- 예: 1.23초
```

---

### 7.4 FireRejected

의미:

```text
- 로컬 발사 조건을 만족하지 못했다.
- 구체 사유가 NoWeapon / Cooldown / AimBlocked가 아닌 일반 실패일 수 있다.
```

표현:

```text
- 색상: 빨간색
- 중앙 아이콘: X
- 짧은 깜빡임 또는 작게 흔들림
- 유지 시간 후보: 0.25 ~ 0.35초
- 텍스트 후보: 발사 불가
```

추천 형태:

```text
   [      ✕      ]
```

주의:

```text
- FireRejected는 서버 거부가 아니다.
- 로컬 발사 조건 미충족이다.
```

---

## 8. 표시 우선순위

Reticle과 FireFeedback이 동시에 발생할 때는 아래 우선순위를 따른다.

```text
1. Hidden
2. AimBlocked / Blocked
3. NoWeapon
4. Cooldown
5. FireRejected
6. FirePending
7. FireSuccess 짧은 플래시
8. Ready
9. OutOfArcWarning 보조 표시
```

해석:

```text
- Blocked, NoWeapon, Cooldown은 플레이어 행동을 막는 직접 원인에 가깝기 때문에 우선한다.
- FireSuccess는 짧은 성공 플래시로 처리한다.
- OutOfArcWarning은 주 상태를 덮어쓰지 않는 보조 경고다.
```

---

## 9. 색상 가이드

정확한 색상값은 최종 UI 작업에서 조정한다.
현재 문서 기준 의미 색상은 아래와 같다.

| 상태 | 색상 방향 | 의미 |
| --- | --- | --- |
| `Ready` | 흰색 / 약한 녹색 | 정상 조준 |
| `FireSuccess` | 녹색 / 흰색 플래시 | 발사 성공 |
| `Cooldown` | 파란색 | 재사용 대기 |
| `FirePending` | 파란색 / 청록색 | 로컬 발사 처리 중 |
| `FireRejected` | 빨간색 | 발사 조건 미충족 |
| `NoWeapon` | 회색 | 무기 없음 |
| `AimBlocked` | 주황색 | 조준선 막힘 |
| `OutOfArcWarning` | 노란색 | 조준각 경고 |

주의:

```text
- 색상만으로 상태를 구분하지 않는다.
- 아이콘, 텍스트, 라인 변화가 함께 있어야 한다.
- 색약 접근성을 고려해 빨강/초록만으로 성공/실패를 구분하지 않는다.
```

---

## 10. WBP 구성 기준

P0 기준 `WBP_AimReticle`은 아래 Optional 위젯을 둘 수 있다.

### 10.1 기존 Optional 후보

```text
Text_ReticleState
Text_CanFire
Text_ReticleHint
```

### 10.2 신규 Optional 후보

```text
Text_FireFeedbackState
Text_FireFeedbackHint
Text_Cooldown
Image_CenterDot
Image_LeftBracket
Image_RightBracket
Image_StateIcon
Image_OutOfArcWarning
```

모든 표시 위젯은 C++에서 `BindWidgetOptional`로 처리하는 방향을 권장한다.
그래야 WBP가 아직 완성되지 않아도 C++ 위젯이 크래시 없이 동작한다.

---

## 11. P0 구현 범위

P0에서는 아래만 구현한다.

```text
- Ready 기본 Reticle
- FireSuccess 짧은 플래시
- Cooldown 숫자 표시
- NoWeapon 텍스트 / 아이콘 표시
- AimBlocked 텍스트 / 아이콘 표시
- FireRejected 텍스트 / 아이콘 표시
- OutOfArcWarning 보조 경고 표시
```

P0에서 제외한다.

```text
- 복잡한 원형 쿨다운 게이지
- 무기별 고유 Reticle
- 무기별 탄 퍼짐 동적 스케일
- 고품질 애니메이션 세트
- HUD 전체 레이아웃 개편
- Common UI 레이어 이전
- 서버 응답 상태 표시
```

---

## 12. BP 애니메이션 후보

P0에서 사용할 수 있는 짧은 애니메이션 후보는 아래다.

```text
FireSuccess:
- 중앙점 Scale 1.0 -> 1.35 -> 1.0
- 브라켓 Opacity 1.0 -> 1.4 느낌의 밝기 강조 -> 1.0
- 0.10 ~ 0.15초

FireRejected:
- 중앙 아이콘 X 표시
- Reticle 전체 좌우 2~4px 짧은 흔들림
- 0.25 ~ 0.35초

Cooldown:
- 텍스트만 갱신
- 애니메이션 없음 또는 약한 Pulse

OutOfArcWarning:
- 경고 아이콘 약한 점멸
- 주 Reticle은 유지
```

과도한 연출은 피한다.
CarFight는 차량 주행 중 시야 정보가 많으므로, Reticle 애니메이션은 짧고 읽기 쉬워야 한다.

---

## 13. 구현 연결 기준

이 VisualGuide는 아래 구현 설계와 연결된다.

```text
ACFVehiclePawn
  -> BuildFireFeedbackViewData()

UCFAimReticleWidget
  -> VehicleAimComp에서 기본 Aim 상태 읽기
  -> VehiclePawn에서 FireFeedbackViewData 읽기
  -> 최종 Reticle / FireFeedback 표시 상태 결정

WBP_AimReticle
  -> C++ 위젯이 정리한 상태를 색상 / 아이콘 / 텍스트 / 애니메이션으로 표시
```

C++ / BP 분담 기준:

```text
C++ = 상태 계산 / 우선순위 / 표시 데이터 생성
BP = 실제 이미지 / 색상 / 위치 / 애니메이션 표시
```

---

## 14. 관련 문서

```text
- Document/Systems/UI/AimReticle.md
- Document/Systems/Combat/FireFeedback.md
- Document/Systems/Combat/WeaponFire.md
- Document/Systems/Vehicles/VehicleAim.md
- Document/Systems/UI/VehicleDebugPanel.md
- Document/ProjectSSOT/03_FeatureQueue.md
- Document/ProjectSSOT/05_TestChecklist.md
```

---

## 15. 문서 갱신 조건

아래 변경이 생기면 이 문서를 갱신한다.

```text
- Reticle 기본 형태가 변경될 때
- 상태별 색상 의미가 변경될 때
- 표시 우선순위가 변경될 때
- WBP_AimReticle 위젯 구성이 확정될 때
- FireFeedback 전담 C++ 타입 또는 함수명이 확정될 때
- Cooldown 표시 방식이 숫자에서 게이지로 변경될 때
- OutOfArcWarning을 주 상태로 승격하기로 결정할 때
```

---

## 16. Changelog

### v0.1.0 - 2026-07-09

```text
- Reticle / FireFeedback VisualGuide 신규 작성
- 기준 이미지의 시각 방향을 문서화
- Ready / Blocked / OutOfArcWarning / NoWeapon / FireSuccess / FirePending / Cooldown / FireRejected 시각 기준 정리
- 표시 우선순위와 색상 가이드 추가
- WBP_AimReticle Optional 위젯 후보 추가
- P0 구현 범위와 제외 범위 정리
- C++ / BP 구현 연결 기준 추가
```
