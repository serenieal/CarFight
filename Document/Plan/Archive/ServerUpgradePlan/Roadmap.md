# 서버 고도화 단계별 로드맵

- 문서 버전: v0.2.0
- 작성일: 2026-06-02
- 대상 프로젝트: CarFight
- 상위 문서: `README.md`
- 상태: Phase 8 Completed

---

## 1. 목적

이 문서는 서버 고도화 작업을 단계별로 나누고, 각 단계의 완료 상태를 고정한다.

초기 최종 목표는 다음이었다.

```text
Dedicated Server에서 클라이언트 2개가 접속하고,
각자 자기 차량을 소유하고,
자기 입력으로 자기 차량만 조작하며,
상대 차량의 이동을 볼 수 있는 상태.
```

현재 이 목표는 통과했다.

---

## 2. 완료 요약

| Phase | 이름 | 상태 |
|---:|---|---:|
| Phase 0 | 문서/상태 정렬 | Completed |
| Phase 1 | 최소 서버 GameMode 작성 | Completed |
| Phase 2 | Config / 맵 연결 | Completed |
| Phase 3 | 1클라 Dedicated 접속 검증 | Completed |
| Phase 4 | 2클라 Dedicated 접속 검증 | Completed |
| Phase 5 | 입력 분리 검증 | Completed |
| Phase 6 | 이동 복제 검증 | Completed |
| Phase 7 | Dedicated Server 금지 코드 정리 | Completed |
| Phase 8 | 서버 안정화 1차 종료 | Completed |
| Phase 9 | 확장 단계 재계획 | Next |

---

## Phase 0. 문서/상태 정렬

### 상태

```text
Completed
```

### 완료 내용

```text
- ServerUpgradePlan 문서 세트 작성
- 기존 MP_ServerPlan과 역할 분리
- Server Target 존재 여부 확인
- 현재 병목을 Spawn/Possess 구조 부재로 고정
```

---

## Phase 1. 최소 서버 GameMode 작성

### 상태

```text
Completed
```

### 완료 내용

```text
- CFMPGameMode.h 추가
- CFMPGameMode.cpp 추가
- ACFMPGameMode 클래스 작성
- PostLogin 기반 SpawnVehicleForController 흐름 추가
- PlayerStart 기반 Spawn Transform 계산
- VehiclePawnClass 설정값 추가
- Spawn/Possess 로그 추가
- Editor / Server Target 빌드 성공
```

---

## Phase 2. Config / 맵 연결

### 상태

```text
Completed
```

### 완료 내용

```text
- DefaultEngine.ini에 GlobalDefaultGameMode 연결
- DefaultEngine.ini에 GlobalDefaultServerGameMode 연결
- BP_CFVehiclePawn 기본 fallback 추가
- DefaultPawnClass 자동 생성 흐름 차단
- TestMap PlayerStart 2개 구성
- TestMap 기존 배치 차량 제거
- BP_CFVehiclePawn 복제/AutoPossess/VehicleData 상태 확인
```

---

## Phase 3. 1클라 Dedicated 접속 검증

### 상태

```text
Completed
```

### 완료 내용

```text
- Dedicated Server 실행
- Client 1 접속
- 서버 로그에서 PostLogin 확인
- 서버 로그에서 차량 Spawn 확인
- 서버 로그에서 Possess 확인
- 클라이언트 화면 검정 화면 문제 해소
- 클라이언트 입력 가능 확인
- Dedicated Server 로그 UI/LocalPlayer 치명 오류 없음 확인
```

---

## Phase 4. 2클라 Dedicated 접속 검증

### 상태

```text
Completed
```

### 완료 내용

```text
- Client 1 접속
- Client 2 접속
- 서버에서 차량 2대 Spawn 확인
- 각 PlayerController의 Possess 대상 분리 확인
- 두 클라이언트가 서로 다른 차량 소유
- 두 클라이언트 모두 자기 차량 기준 카메라 확인
```

---

## Phase 5. 입력 분리 검증

### 상태

```text
Completed
```

### 완료 내용

```text
- Client 1 입력은 Client 1 차량에만 적용
- Client 2 입력은 Client 2 차량에만 적용
- 입력 교차 오염 없음
- 비소유 Pawn 입력 바인딩 문제 없음
```

---

## Phase 6. 이동 복제 검증

### 상태

```text
Completed
```

### 완료 내용

```text
- Client 1 차량 이동이 Client 2 화면에서 보임
- Client 2 차량 이동이 Client 1 화면에서 보임
- 위치 복제 확인
- 회전 복제 확인
- 정지 상태 확인
- 저속 / 고속 / 급조향 테스트 확인
```

---

## Phase 7. Dedicated Server 금지 코드 정리

### 상태

```text
Completed
```

### 완료 내용

```text
- UI / Viewport / LocalPlayer 접근 경로 감사
- Debug HUD / Panel 경로 감사
- Sound / FX 호출 후보 감사
- UCFVehicleCameraComp Dedicated Server Tick guard 추가
- Editor / Server Target 빌드 성공
```

---

## Phase 8. 서버 안정화 1차 종료

### 상태

```text
Completed
```

### 완료 기준 결과

```text
- 2클라 접속 성공: PASS
- 각자 차량 소유 성공: PASS
- 입력 분리 성공: PASS
- 기본 이동 복제 성공: PASS
- 서버 금지 코드 오류 없음: PASS
- 테스트 절차 문서화 완료: PASS
- 남은 문제 후속 이슈 분리: PASS
```

---

## Phase 9. 확장 단계 재계획

### 상태

```text
Next
```

### 목표

서버 기반이 생긴 뒤 다음 gameplay 시스템을 하나씩 계획한다.

권장 후보 순서:

```text
1. 무기/발사 요청 서버 권한 구조 설계
2. 최소 발사 RPC / 서버 검증 / 클라이언트 시각 피드백 분리
3. 체력/대미지 구조 설계
4. 리스폰 구조 설계
5. 스코어/킬/데스
6. 로비/세션
```

---

## 변경 기록

### v0.2.0

```text
- Phase 0~8 완료 상태로 갱신
- 서버 1차 안정화 완료 기준 PASS 반영
- Phase 9를 다음 단계로 재정의
```

### v0.1.0

```text
- Phase 0부터 Phase 9까지 서버 고도화 로드맵 작성
- 각 단계별 목표, 작업, 완료 기준 정의
- 1차 안정화 완료 기준 정의
- 확장 단계 진입 조건 정의
```
