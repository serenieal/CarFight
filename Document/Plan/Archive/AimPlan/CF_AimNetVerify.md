# CarFight - CF_AimNetVerify

> 역할: 과거 Aim Listen Server / Client 검증 절차를 Deprecated 상태로 보존한다.  
> 문서 버전: v1.1.0  
> 마지막 정리(Asia/Seoul): 2026-06-19  
> 상태: Deprecated / Network Verification Archive

---

## 1. 현재 판정

2026-06-19 기준 CarFight는 싱글플레이 게임으로 방향을 전환했다.

따라서 이 문서는 더 이상 현재 검증 절차가 아니다.

아래 검증은 중단한다.

- Listen Server + 1 Client 검증
- Listen Server + 2 Clients 검증
- Client Fire Request 서버 전달 검증
- Server Validation 결과 검증
- Server Dummy HitScan 검증
- Owner Client FireResult 수신 검증
- RepAimVisualState 원격 갱신 검증
- Dedicated Server 후보 검증

---

## 2. 현재 검증 기준

최신 Aim 검증은 `CF_AimVerify.md`를 따른다.

현재 우선 검증 모드는 다음이다.

```text
1. PIE Single Player
2. Standalone Game
3. 패키지 실행 후보
```

현재 우선 검증 항목은 다음이다.

- Local Aim State 갱신
- Reticle 표시
- Local Fire Command 생성
- Local Fire Validation 결과
- Local HitScan 더미 결과
- VehicleDebug Aim 표시
- 기존 차량 이동 / 카메라 입력 회귀 없음

---

## 3. 보존 이유

이 문서는 과거 네트워크 검증 계획을 추적하기 위해 남긴다.

다만 현재 작업자는 이 문서를 체크리스트로 사용하지 않는다.

사용 가능한 경우는 다음으로 제한한다.

- 과거 멀티플레이 작업 흔적을 해석해야 할 때
- 기존 코드명에 `Server`, `Client`, `Rep`가 남은 이유를 확인할 때
- 미래 온라인 복귀 시 새 검증 문서를 만들기 위한 참고 자료가 필요할 때

---

## 4. 온라인 복귀 전제

온라인 검증을 다시 시작하려면 먼저 아래를 최신 코드 기준으로 확인해야 한다.

1. 현재 프로젝트 목표가 온라인 또는 멀티플레이로 다시 바뀌었는가?
2. GameMode / PlayerController / Pawn Possess 구조가 존재하는가?
3. 차량 이동 복제와 물리 보정이 현재 코드에서 안정적인가?
4. Aim / Weapon / Damage 권한 모델을 새로 설계했는가?
5. Dedicated Server 실행선이 별도 문서에서 복구되었는가?

---

## ChangeLog

- v1.1.0 / 2026-06-19
  - Listen Server / Client 검증 절차를 Deprecated 처리했다.
  - 현재 검증 기준을 싱글플레이 PIE / Standalone 중심으로 전환했다.
  - 온라인 복귀 전제 조건을 명시했다.

## 마이그레이션 지침

- 현재 검증은 `CF_AimVerify.md`에서 진행한다.
- 이 문서의 체크박스나 절차를 신규 작업 완료 조건으로 사용하지 않는다.
- 네트워크 검증이 다시 필요해지면 이 문서를 그대로 복구하지 말고 최신 코드 기준으로 새 문서를 만든다.
