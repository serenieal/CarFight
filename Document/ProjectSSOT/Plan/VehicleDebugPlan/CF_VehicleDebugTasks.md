# VehicleDebug v2 작업 목록

- Version: 0.1.0
- Date: 2026-04-22
- Status: Draft
- Scope: VehicleDebug v2를 실제 작업 단위로 나눈 실행 목록

---

## 1. 사용 목적

이 문서는 설계 문서를 실제 작업 항목으로 쪼개서,

- 무엇부터 시작할지,
- 어떤 순서로 진행할지,
- 어떤 파일/영역을 건드리게 될지

실무 기준으로 확인하기 위한 작업 목록이다.

---

## 2. 1차 목표

이번 1차 목표는 아래로 잡는다.

1. Snapshot 중심 구조 도입
2. 기존 Text View 호환 유지
3. Compact HUD 추가
4. Overview / Drive / Input / Runtime Panel 추가

Recent Events와 UI 소유권 이동은 2차 목표로 미뤄도 된다.

---

## 3. 선행 작업

### 3.1 기준선 정리
- [ ] 현재 `VehicleDebug.md` 다시 확인
- [ ] 현재 `GetDebugTextSingleLine()` 출력 샘플 저장
- [ ] 현재 `GetDebugTextMultiLine()` 출력 샘플 저장
- [ ] 현재 위젯 생성 흐름 확인
- [ ] 현재 표시 조건 확인

### 3.2 영향 범위 확인
- [ ] `ACFVehiclePawn` 관련 공개 API 재확인
- [ ] 기존 `WBP_VehicleDebug` 바인딩 포인트 재확인
- [ ] `BP_CFVehiclePawn` 이벤트 그래프 영향 범위 정리

---

## 4. C++ 작업

## 4.1 Snapshot 구조 재편
- [ ] `FCFVehicleDebugSnapshot` 현재 필드 정리
- [ ] `Overview` 하위 구조 설계
- [ ] `Drive` 하위 구조 설계
- [ ] `Input` 하위 구조 설계
- [ ] `Runtime` 하위 구조 설계
- [ ] 필요 시 최근 이벤트 구조 초안 추가

## 4.2 데이터 채우기 함수 정리
- [ ] Pawn에서 Overview 데이터 채우기
- [ ] DriveComp 스냅샷과 Drive 카테고리 연결
- [ ] 입력 해석 결과와 Input 카테고리 연결
- [ ] 런타임 요약과 Runtime 카테고리 연결

## 4.3 텍스트 호환 유지
- [ ] `GetDebugTextSingleLine()`를 Snapshot 기반으로 전환
- [ ] `GetDebugTextMultiLine()`를 Snapshot 기반으로 전환
- [ ] `GetDebugTextByDisplayMode()` 동작 확인
- [ ] 기존 출력 의미가 크게 달라지지 않게 조정

---

## 5. UMG / BP 작업

## 5.1 HUD 위젯
- [ ] `WBP_VehicleDebugHud` 생성
- [ ] HUD 표시 필드 확정
- [ ] State / Speed / Input / RuntimeReady 배치
- [ ] HUD 표시 토글 추가

## 5.2 Panel 위젯
- [ ] `WBP_VehicleDebugPanel` 생성
- [ ] 카테고리 블록 공통 위젯 생성
- [ ] 필드 Row 공통 위젯 생성
- [ ] `Overview` 섹션 배치
- [ ] `Drive` 섹션 배치
- [ ] `Input` 섹션 배치
- [ ] `Runtime` 섹션 배치

## 5.3 기존 텍스트 위젯 정리
- [ ] `WBP_VehicleDebug` 유지 여부 결정
- [ ] 유지 시 Legacy Text View 역할로 정리
- [ ] HUD/Panel과 동시 표시 시 레이아웃 충돌 방지

---

## 6. 표시 정책 작업

- [ ] `bShowDebugHud` 도입 여부 결정
- [ ] `bShowDebugPanel` 도입 여부 결정
- [ ] `bShowDebugTextView` 도입 여부 결정
- [ ] 카테고리별 표시 토글 도입 여부 결정
- [ ] 기존 `DriveStateDebugDisplayMode` 역할 재정의 여부 결정

---

## 7. 이벤트 작업(2차 권장)

- [ ] 최근 이벤트 구조 정의
- [ ] Drive 상태 전이 이벤트 Push
- [ ] Runtime 재초기화 이벤트 Push
- [ ] InputOwner 변경 이벤트 Push
- [ ] 이벤트 최대 보관 개수 제한
- [ ] `WBP_VehicleDebugEvents` 생성

---

## 8. 검증 작업

## 8.1 기능 검증
- [ ] 기존 텍스트 출력이 유지되는지 확인
- [ ] HUD가 정상 표시되는지 확인
- [ ] Panel이 정상 표시되는지 확인
- [ ] 카테고리 값 누락이 없는지 확인
- [ ] 로컬 제어 차량에서만 표시되는지 확인

## 8.2 사용성 검증
- [ ] HUD만 봐도 현재 핵심 상태를 이해할 수 있는지 확인
- [ ] Panel이 텍스트 덩어리보다 읽기 쉬운지 확인
- [ ] Runtime 문제 원인을 찾기 쉬운지 확인
- [ ] Drive와 Input 문제를 분리해서 읽기 쉬운지 확인

## 8.3 회귀 검증
- [ ] 기존 문서 기준 표시 조건이 깨지지 않았는지 확인
- [ ] 기존 MultiLine 출력에 있던 중요 항목이 빠지지 않았는지 확인
- [ ] 기존 상태 전이 요약이 사라지지 않았는지 확인

---

## 9. 문서 작업

- [ ] 설계 변경분을 `VehicleDebug.md`에 반영할 시점 판단
- [ ] 새 위젯 이름을 문서에 기록
- [ ] 새 토글 변수 이름을 문서에 기록
- [ ] 카테고리 구조를 Systems 문서로 승격할지 검토

---

## 10. 추천 작업 순서

### 순서 A - 안전한 시작
1. Snapshot 구조 재편
2. 텍스트 호환 유지
3. HUD 추가
4. Panel 추가
5. 이벤트 추가

### 순서 B - 빠른 가시화 우선
1. HUD 목업 먼저 제작
2. 필요한 필드 역산
3. Snapshot 구조 정리
4. Panel 확장

이번 주제는 구조 개편이 핵심이므로, 기본 추천은 `순서 A`다.

---

## 11. 오늘 바로 시작하기 좋은 최소 작업

당장 첫 커밋 수준으로 시작하기 좋은 최소 묶음은 아래다.

- [ ] `FCFVehicleDebugSnapshot` 카테고리 초안 작성
- [ ] 기존 텍스트 함수가 새 Snapshot을 쓰도록 변경
- [ ] `WBP_VehicleDebugHud` 뼈대 생성
- [ ] Overview 핵심값 4~6개 HUD에 표시

이 묶음만 끝내도 구조 개편의 방향이 확정되고, 눈에 보이는 결과가 생긴다.
