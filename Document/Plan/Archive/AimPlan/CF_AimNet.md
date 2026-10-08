# CarFight - CF_AimNet

> 역할: 과거 Aim 멀티플레이 네트워크 설계 기준을 Deprecated 상태로 보존한다.  
> 문서 버전: v0.3.0  
> 마지막 정리(Asia/Seoul): 2026-06-19  
> 상태: Deprecated / Multiplayer Archive

---

## 1. 현재 판정

2026-06-19 기준 CarFight는 싱글플레이 게임으로 방향을 전환했다.

따라서 이 문서는 현재 구현 기준 문서가 아니다.

아래 항목은 신규 작업 목표에서 제외한다.

- Aim 멀티플레이 권한 모델
- `ServerRequestFire` RPC
- `ClientReceiveFireResult` RPC
- `MulticastPlayFireFx` RPC
- `Replicated Aim Visual State`
- Listen Server 검증
- Dedicated Server 검증
- 네트워크 예측 / 보정 / 복제 최적화

---

## 2. 현재 Aim 기준

최신 Aim 작업은 다음 싱글플레이 흐름을 따른다.

```text
Local Aim
  -> 플레이어 입력과 카메라 기준 조준 방향 계산

Local Fire Validation
  -> 조준각 / 차단 / 무기 상태 검증

Local Hit Result
  -> HitScan 또는 Projectile 초기 결과 생성

Local Feedback
  -> Reticle / Debug / FX / 사운드 표시
```

현재 기준 문서는 다음을 우선한다.

- `README.md`
- `CF_AimContext.md`
- `CF_AimSpec.md`
- `CF_AimDesign.md`
- `CF_AimTasks.md`
- `CF_AimVerify.md`
- `CF_AimStatus.md`

---

## 3. 보존 이유

이 문서는 완전히 삭제하지 않는다.

이유는 다음과 같다.

- 기존 코드나 Generated 문서에 `ServerAim`, `FireRequest`, `RepAimVisual` 명칭이 남아 있을 수 있다.
- 향후 온라인 모드가 다시 필요해질 경우 권한 모델 재검토의 출발점으로 사용할 수 있다.
- 과거 의사결정이 왜 현재 싱글플레이 기준으로 폐기되었는지 추적할 수 있다.

---

## 4. 사용 금지 기준

다음 상황에서는 이 문서를 구현 근거로 사용하지 않는다.

- 싱글플레이 Aim 기능 추가
- Reticle UI 구현
- 로컬 Fire Command 구현
- 로컬 HitScan 더미 구현
- WeaponComp / DamageComp 연결
- 현재 작업 우선순위 판단

---

## 5. 온라인 복귀 조건

이 문서를 다시 활성 기준으로 쓰려면 먼저 아래 결정이 필요하다.

1. 프로젝트가 다시 멀티플레이 또는 온라인 모드를 공식 목표로 채택한다.
2. GameMode, PlayerController, Pawn Possess, Dedicated Server 실행선이 최신 코드 기준으로 재검증된다.
3. 차량 물리 복제와 전투 판정 권한 모델을 새로 감사한다.
4. 이 문서의 v0.2.0 네트워크 설계를 그대로 복구하지 않고, 최신 코드 기준으로 재작성한다.

---

## ChangeLog

- v0.3.0 / 2026-06-19
  - 프로젝트 싱글플레이 전환에 따라 네트워크 설계 문서를 Deprecated 처리했다.
  - 기존 RPC/복제/권한 모델을 현재 구현 기준에서 제외했다.
  - 온라인 복귀 조건과 현재 참조해야 할 문서를 명시했다.

## 마이그레이션 지침

- 신규 작업은 이 문서 대신 `CF_AimSpec.md`, `CF_AimDesign.md`, `CF_AimTasks.md`, `CF_AimVerify.md`를 따른다.
- 기존 C++ 이름에 `Server`, `Rep`, `Request`가 남아 있어도 이번 문서 정리만으로 리네이밍하지 않는다.
- 리네이밍은 빌드 영향 범위와 블루프린트 참조를 확인한 뒤 별도 리팩터링으로 진행한다.
