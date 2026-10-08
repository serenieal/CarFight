# AimPlan Generated 문서 안내

> 역할: `AimPlan/Generated` 폴더의 과거 자동 생성 문서 사용 기준을 명시한다.  
> 문서 버전: v1.0.0  
> 마지막 정리(Asia/Seoul): 2026-06-19  
> 상태: Historical / Do Not Use As Current Plan

---

## 현재 판정

이 폴더의 문서는 과거 Codex 작업지시서와 자동 생성 산출물이다.

2026-06-19 기준 CarFight는 싱글플레이 게임으로 방향을 전환했다.

따라서 이 폴더 안의 문서에 남아 있는 아래 표현은 현재 구현 목표가 아니다.

- 멀티플레이
- Dedicated Server
- Listen Server
- RPC
- Replication
- Server Validation
- RepAimVisual
- Client / Owner Client 기준 검증

---

## 사용 기준

현재 작업자는 이 폴더를 최신 계획서로 사용하지 않는다.

최신 기준은 상위 `AimPlan` 폴더의 다음 문서를 따른다.

- `../README.md`
- `../CF_AimContext.md`
- `../CF_AimSpec.md`
- `../CF_AimDesign.md`
- `../CF_AimTasks.md`
- `../CF_AimVerify.md`
- `../CF_AimStatus.md`

---

## 보존 이유

이 폴더는 과거 구현 흐름과 코드명 출처를 추적하기 위해 남긴다.

기존 코드에 `Server`, `Rep`, `Request` 같은 이름이 남아 있을 때, 왜 그런 이름이 생겼는지 확인하는 참고 자료로만 사용한다.

---

## ChangeLog

- v1.0.0 / 2026-06-19
  - Generated 문서가 현재 싱글플레이 계획 기준이 아님을 명시했다.
  - 최신 참조 문서 목록을 추가했다.

## 마이그레이션 지침

- 이 폴더의 문서를 근거로 신규 작업지시서를 만들지 않는다.
- 필요한 경우 상위 최신 문서를 먼저 읽고, 이 폴더 내용은 과거 참고 자료로만 확인한다.
