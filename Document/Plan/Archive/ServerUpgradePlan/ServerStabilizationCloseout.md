# 서버 1차 안정화 종료 보고

- 문서 버전: v1.0.0
- 작성일: 2026-06-02
- 대상 프로젝트: CarFight
- 관련 문서: `RuntimeTest_20260601.md`, `DedicatedServerCodeAudit.md`, `TaskList.md`
- 상태: Completed

---

## 1. 목적

이 문서는 CarFight 서버 고도화 1차 작업의 종료 상태를 정리한다.

이번 단계의 목표는 다음이었다.

```text
현재 구현된 클라이언트 차량 기능을 Dedicated Server 기준에서 안정적으로 실행, 소유, 조작, 관찰할 수 있게 만든다.
```

---

## 2. 최종 결론

서버 1차 안정화 핵심 조건은 통과했다.

현재 서버는 더 이상 단순히 Dedicated Server를 실행하는 수준이 아니다.

현재 도달 상태:

```text
Dedicated Server 기반 2클라 차량 Spawn/Possess, 입력 분리, 기본 이동 복제 검증 통과
```

---

## 3. 완료된 핵심 작업

### 3.1 서버 GameMode 기반 구축

```text
- ACFMPGameMode 추가
- 접속자별 차량 Spawn/Possess 구조 추가
- BP_CFVehiclePawn 기본 fallback 추가
- DefaultPawnClass 자동 생성 흐름 차단
- DefaultEngine.ini에 GlobalDefaultGameMode / GlobalDefaultServerGameMode 연결
```

관련 파일:

```text
UE/Source/CarFight_Re/Public/CFMPGameMode.h
UE/Source/CarFight_Re/Private/CFMPGameMode.cpp
UE/Config/DefaultEngine.ini
```

---

### 3.2 TestMap 멀티플레이 테스트 구조 정리

```text
- TestMap에 PlayerStart 2개 구성
- 기존 배치 차량 제거
- 차량은 ACFMPGameMode가 서버에서 Spawn하도록 통일
```

---

### 3.3 BP_CFVehiclePawn 서버 테스트 조건 확인

```text
- AutoPossessPlayer = Disabled 확인
- bReplicates = true 확인
- bReplicateMovement = true 확인
- VehicleData = DA_PoliceCar 확인
```

---

### 3.4 Dedicated Server 런타임 테스트 통과

사용자 보고 기준으로 아래 항목이 PASS 처리되었다.

```text
- Dedicated Server 실행
- Client 1 접속
- Client 2 접속
- 각 클라이언트 차량 Spawn/Possess
- 각 클라이언트 자기 차량 소유
- 입력 분리
- 기본 이동 복제
- 서버 금지 코드 치명 오류 없음
```

---

### 3.5 Dedicated Server 금지 코드 정리

```text
- UI / Viewport / LocalPlayer 접근 경로 감사
- Debug HUD / Panel 경로 감사
- Sound / FX 호출 후보 감사
- UCFVehicleCameraComp Dedicated Server Tick guard 추가
```

관련 파일:

```text
UE/Source/CarFight_Re/Public/CFVehicleCameraComp.h
UE/Source/CarFight_Re/Private/CFVehicleCameraComp.cpp
```

---

## 4. 빌드 검증

아래 Target 빌드가 성공했다.

```text
CarFight_ReEditor Win64 Development
CarFight_ReServer Win64 Development
```

마지막 검증 대상:

```text
- CFMPGameMode 연결 후 Editor / Server 빌드
- CFCamDSTick 작업 후 Editor / Server 빌드
```

---

## 5. 완료 기준 매칭

| 완료 기준 | 결과 |
|---|---:|
| 2클라 접속 성공 | PASS |
| 각자 차량 소유 성공 | PASS |
| 입력 분리 성공 | PASS |
| 기본 이동 복제 성공 | PASS |
| 서버 금지 코드 오류 없음 | PASS |
| 테스트 절차 문서화 | PASS |
| 남은 문제 별도 후속 이슈 분리 | PASS |

---

## 6. 남은 후속 이슈

서버 1차 안정화는 종료하지만, 아래 항목은 다음 확장 단계에서 다룬다.

```text
- CFPlayerController 도입 시점 결정
- 무기/발사 요청 서버 권한 구조 설계
- 체력/대미지/리스폰 시스템 설계
- 장기 차량 이동 품질 튜닝
- 서버 테스트 실행 절차 자동화
- Steam 세션 / 로비 / 매치메이킹 계획
```

---

## 7. 다음 권장 단계

가장 자연스러운 다음 단계는 아래 순서다.

```text
1. 무기/발사 요청의 서버 권한 구조 설계
2. 최소 발사 RPC / 서버 검증 / 클라이언트 시각 피드백 분리
3. 체력/대미지 구조 설계
4. 리스폰 구조 설계
```

다만 바로 구현에 들어가기 전, 먼저 별도 문서로 전투 서버 권한 구조를 설계하는 것이 좋다.

---

## 8. 변경 기록

### v1.0.0

```text
- 서버 1차 안정화 종료 보고 최초 작성
- 완료된 GameMode, Config, Map, BP 감사, Runtime Test, DS 금지 코드 정리 결과 요약
- 후속 이슈와 다음 권장 단계 정리
```
