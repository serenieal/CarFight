# 서버/클라이언트 현재 상태

- 문서 버전: v0.2.0
- 작성일: 2026-06-02
- 대상 프로젝트: CarFight
- 상위 문서: `README.md`
- 상태: Active

---

## 1. 목적

이 문서는 CarFight의 서버/클라이언트 현재 상태를 서버 1차 안정화 완료 기준으로 갱신한다.

이전 v0.1.0 문서는 작업 전 병목을 정리했다.

현재 v0.2.0 문서는 다음 사실을 기준으로 한다.

```text
Dedicated Server 기반 2클라 차량 Spawn/Possess, 입력 분리, 기본 이동 복제 검증 통과
```

---

## 2. 현재 클라이언트 기준선

현재 클라이언트 기준선은 다음이다.

```text
- BP_CFVehiclePawn
- ACFVehiclePawn
- UCFVehicleDriveComp
- UCFWheelSyncComp
- UCFVehicleCameraComp
- UCFVehicleAimComp
- UCFVehicleData
- DA_PoliceCar
- DA_Cam_Default
- TestMap
```

---

## 3. 현재 서버 기준선

현재 서버 기준선은 다음 수준까지 올라왔다.

```text
- CarFight_ReServer Target 존재
- Dedicated Server 빌드 가능
- Dedicated Server 실행 가능
- ACFMPGameMode 기반 접속자별 차량 Spawn/Possess 가능
- TestMap 2클라 테스트 구조 구성 완료
- BP_CFVehiclePawn 서버 Spawn 기본 fallback 구성 완료
- 2클라 Dedicated 접속 성공
- 각 클라이언트 자기 차량 소유 성공
- 입력 분리 성공
- 기본 이동 복제 성공
- Dedicated Server 금지 코드 정밀 감사 및 Camera Tick guard 정리 완료
```

---

## 4. 현재 구현 상태 표

| 영역 | 현재 상태 | 판단 |
|---|---:|---|
| Server Target | 존재 | 완료 |
| Dedicated Server 빌드 | 성공 | 완료 |
| 전용 GameMode | `ACFMPGameMode` 추가 | 완료 |
| Config 연결 | `GlobalDefaultGameMode`, `GlobalDefaultServerGameMode` 연결 | 완료 |
| 차량 Spawn/Possess | 접속자별 서버 Spawn/Possess | 완료 |
| TestMap | PlayerStart 2개, 배치 차량 제거 | 완료 |
| BP 차량 복제 설정 | `bReplicates=true`, `bReplicateMovement=true` | 확인 완료 |
| 1클라 접속 | PASS | 완료 |
| 2클라 접속 | PASS | 완료 |
| 입력 분리 | PASS | 완료 |
| 기본 이동 복제 | PASS | 완료 |
| 서버 금지 코드 | 감사 및 Camera Tick guard 완료 | 완료 |
| 무기/체력/대미지 | 아직 1차 범위 밖 | 후속 |
| 리스폰/스코어/로비 | 아직 1차 범위 밖 | 후속 |

---

## 5. 해결된 기존 병목

### 5.1 기존 병목

이전 병목은 다음이었다.

```text
멀티플레이용 최소 Spawn/Possess 구조 부재
```

### 5.2 해결 방식

해결된 작업:

```text
- CFMPGameMode.h / CFMPGameMode.cpp 추가
- ACFMPGameMode가 PostLogin 기반으로 차량 Spawn/Possess
- BP_CFVehiclePawn 기본 fallback 추가
- DefaultPawnClass 자동 생성 흐름 차단
- DefaultEngine.ini GameMode 연결
- TestMap PlayerStart 2개 구성
- 맵 배치 차량 제거
```

### 5.3 검증 결과

검증 결과:

```text
- Dedicated Server 2클라 테스트 성공
- 각자 차량 소유 성공
- 입력 분리 성공
- 상대 차량 이동 관찰 성공
```

---

## 6. 현재 남은 후속 병목

서버 1차 안정화는 통과했지만, 다음 gameplay 확장 전에는 아래 설계가 필요하다.

```text
- 무기/발사 요청 서버 권한 구조
- 서버 검증 기반 Aim/Fire 처리
- 체력/대미지 구조
- 리스폰 구조
- CFPlayerController 도입 여부
- 장기 이동 품질 튜닝 필요 여부
```

---

## 7. 현재 우선순위 판단

이제 우선순위는 서버 실행 기반이 아니라 서버 권한 gameplay 설계다.

권장 순서:

```text
1. 무기/발사 요청 서버 권한 구조 설계
2. 최소 발사 RPC와 서버 검증 설계
3. 서버 판정 결과의 클라이언트 시각 피드백 분리
4. 체력/대미지 구조 설계
5. 리스폰 구조 설계
```

---

## 8. 변경 기록

### v0.2.0

```text
- 서버 1차 안정화 완료 상태로 현재 상태 갱신
- ACFMPGameMode, Config 연결, TestMap 정리, 2클라 테스트, 입력 분리, 이동 복제, DS 금지 코드 정리 완료 반영
- 후속 병목을 서버 권한 gameplay 설계로 재정의
```

### v0.1.0

```text
- 현재 클라이언트 기준선 정리
- 현재 서버 기준선 정리
- Listen Server / Play As Client / Dedicated 접속 관찰 결과 반영
- 서버/클라이언트 격차표 작성
- 최우선 병목을 최소 Spawn/Possess 구조 부재로 고정
```
