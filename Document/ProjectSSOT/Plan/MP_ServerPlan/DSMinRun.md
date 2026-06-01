# Dedicated Server 최소 실행선 계획

- 문서 버전: 0.2.0
- 작성일: 2026-05-22
- 최종 갱신일: 2026-05-26
- 대상 프로젝트: CarFight
- 상위 문서: `MPPlan.md`

---

## 1. 목적

이 문서는 CarFight가 Dedicated Server 환경에서 최소한으로 실행되고 검증되어야 할 기준을 정의한다.

이 단계에서는 체력, 스코어, 무기, 리스폰이 없어도 된다.

목표는 다음 하나다.

```text
Dedicated Server에서 클라이언트 2개가 접속하고,
각자 자기 차량을 소유하고,
서로의 차량 이동을 볼 수 있으며,
서버에서 UI/카메라/사운드 코드가 문제를 일으키지 않는 상태를 만든다.
```

---

## 2. 현재 진행 상태

### 2.1 완료된 환경 작업

```text
- UE 5.7 Source Build 엔진 전환 완료
- Dedicated Server 빌드 성공
- Dedicated Server 실행 성공
```

판정:

```text
빌드 환경 병목은 해결됨.
이제 런타임 멀티플레이 검증 단계로 이동한다.
```

---

### 2.2 아직 완료되지 않은 런타임 검증

```text
- 클라이언트 2개 서버 접속 확인
- 각 클라이언트의 Pawn 또는 차량 소유 확인
- 입력 분리 확인
- 차량 이동 복제 확인
- Dedicated Server 로그의 UI/카메라/로컬 사운드 오류 확인
```

판정:

```text
Dedicated Server 빌드/실행은 완료되었지만,
2클라 플레이 검증은 아직 남아 있다.
```

---

## 3. 범위

### 3.1 포함

```text
- Dedicated Server 실행 확인
- 클라이언트 2개 접속 확인
- 플레이어 스폰 확인
- 차량 Pawn 소유권 확인
- 차량 입력 분리 확인
- 차량 이동 복제 확인
- Dedicated Server 로그 확인
- 클라이언트 로그 확인
```

### 3.2 제외

```text
- 체력
- 대미지
- 스코어
- 킬/데스
- 리스폰
- 무기 시스템
- 로비
- 매치메이킹
- Steam 세션
- 서버 배포 자동화
```

---

## 4. 완료 기준 요약

이 문서의 전체 작업은 아래 조건을 만족하면 완료로 본다.

```text
[필수 완료 기준]
1. Dedicated Server가 실행된다. [완료]
2. 클라이언트 2개가 서버에 접속한다. [미확인]
3. 각 클라이언트가 자기 차량을 소유한다. [미확인]
4. 클라이언트 A의 입력은 A 차량에만 적용된다. [미확인]
5. 클라이언트 B의 입력은 B 차량에만 적용된다. [미확인]
6. 클라이언트 A 화면에서 B 차량 이동이 보인다. [미확인]
7. 클라이언트 B 화면에서 A 차량 이동이 보인다. [미확인]
8. 서버 로그에 HUD/카메라/로컬 사운드 관련 오류가 없다. [미확인]
```

---

## 5. 테스트 환경

테스트 환경은 실제 프로젝트 상태에 맞춰 갱신한다.

```text
Unreal Engine 버전:
- UE 5.7 Source Build

프로젝트 이름:
- CarFight

언리얼 프로젝트:
- UE/CarFight_Re.uproject

서버 Target:
- CarFight_ReServer
- 빌드 성공 확인됨

테스트 맵:
- /Game/Maps/TestMap.TestMap
- Dedicated Server 실행 성공 확인됨

기본 GameMode:
- 확인 필요

기본 Pawn 또는 Vehicle Pawn:
- BP_CFVehiclePawn 후보
- ACFVehiclePawn 기반

PlayerController:
- 확인 필요
```

---

## 6. 실행 전 점검

### 6.1 서버 Target 확인

- [x] `CarFight_ReServer.Target.cs` 존재 여부 확인
- [x] Source Build 엔진으로 Dedicated Server 빌드 가능 여부 확인
- [x] Dedicated Server 실행 가능 여부 확인

메모:

```text
UE 5.7 Launcher Installed Build에서는 Dedicated Server Target 빌드가 막혔고,
UE 5.7 Source Build 전환 후 Dedicated Server 빌드와 실행이 성공했다.
```

---

### 6.2 기본 맵 확인

- [x] Dedicated Server가 실행할 테스트 맵 확인
- [ ] 테스트 맵에 멀티플레이 테스트용 PlayerStart가 충분한지 확인
- [ ] 맵에서 싱글 전용 Actor가 강하게 의존되어 있지 않은지 확인

현재 확인:

```text
TestMap에는 PlayerStart 1개와 BP_CFVehiclePawn 1대가 확인되어 있다.
2클라 검증에는 부족할 가능성이 높다.
```

---

### 6.3 GameMode 확인

- [ ] 현재 사용하는 GameMode 확인
- [ ] GameMode에서 UI 생성 여부 확인
- [ ] GameMode에서 로컬 플레이어를 참조하는지 확인
- [ ] 서버에서 플레이어 스폰이 정상 작동하는지 확인

메모:

```text
다음 작업 T-005에서 확인한다.
```

---

### 6.4 Pawn / Vehicle 확인

- [x] 차량 Pawn 클래스 확인
- [x] 차량 Pawn의 `bReplicates` 설정 확인
- [x] 차량 Pawn의 `bReplicateMovement` 설정 확인
- [ ] 차량 이동 복제 실제 동작 확인
- [ ] 카메라가 소유 클라이언트 전용인지 실행 환경에서 확인
- [ ] 입력 바인딩이 소유자에게만 적용되는지 실행 환경에서 확인

현재 확인:

```text
BP_CFVehiclePawn Class Defaults 기준:
- bReplicates = true
- bReplicateMovement = true
- AutoPossessPlayer = Player0

AutoPossessPlayer = Player0는 Dedicated Server 2클라 테스트에서 위험 요소다.
```

---

## 7. 테스트 절차

### 7.1 서버 실행

목표:

```text
Dedicated Server가 테스트 맵을 정상적으로 실행한다.
```

체크리스트:

- [x] 서버 실행 명령 또는 에디터 실행 방식 확인
- [x] 서버 로그 시작 확인
- [x] 테스트 맵 로드 확인
- [x] 7777 포트 리슨 확인
- [x] 스테이지 서버 실행 기준 치명적 오류 없음

결과:

```text
성공.
Source Build 전환 후 Dedicated Server 빌드와 실행이 완료되었다.
CodexStagedServerTest.log 기준으로 /Game/Maps/TestMap 로드와 7777 포트 리슨이 확인되었다.

주의:
비스테이지 직접 실행 로그인 CodexDedicatedServerTest.log에는 BufferReader Assertion 크래시가 있다.
따라서 현재 서버 실행 성공 근거는 스테이지 서버 로그를 기준으로 한다.
```

---

### 7.2 클라이언트 1 접속

목표:

```text
첫 번째 클라이언트가 서버에 접속하고 차량을 소유한다.
```

체크리스트:

- [ ] 클라이언트 1 서버 접속
- [ ] PlayerController 생성 확인
- [ ] Pawn 또는 Vehicle Spawn 확인
- [ ] Possess 확인
- [ ] 입력 가능 확인
- [ ] 카메라 정상 확인
- [ ] HUD 정상 확인

결과:

```text
미확인.
다음 런타임 검증 단계에서 확인한다.
```

---

### 7.3 클라이언트 2 접속

목표:

```text
두 번째 클라이언트가 서버에 접속하고 독립적인 차량을 소유한다.
```

체크리스트:

- [ ] 클라이언트 2 서버 접속
- [ ] PlayerController 생성 확인
- [ ] 별도 Pawn 또는 Vehicle Spawn 확인
- [ ] Possess 확인
- [ ] 클라이언트 1과 다른 차량을 소유하는지 확인
- [ ] 입력 가능 확인
- [ ] 카메라 정상 확인
- [ ] HUD 정상 확인

결과:

```text
미확인.
현재 TestMap 구조상 PlayerStart와 차량 수가 부족할 가능성이 있다.
```

---

### 7.4 입력 분리 테스트

목표:

```text
각 클라이언트의 입력은 자기 차량에만 적용된다.
```

체크리스트:

- [ ] 클라이언트 1 이동 입력 시 클라이언트 1 차량만 움직임
- [ ] 클라이언트 2 이동 입력 시 클라이언트 2 차량만 움직임
- [ ] 클라이언트 1 입력이 클라이언트 2 차량에 영향을 주지 않음
- [ ] 클라이언트 2 입력이 클라이언트 1 차량에 영향을 주지 않음

결과:

```text
미확인.
```

---

### 7.5 이동 복제 테스트

목표:

```text
다른 플레이어 차량 이동이 각 클라이언트 화면에서 보인다.
```

체크리스트:

- [ ] 클라이언트 1 화면에서 클라이언트 2 차량 이동 확인
- [ ] 클라이언트 2 화면에서 클라이언트 1 차량 이동 확인
- [ ] 위치가 크게 튀지 않는지 확인
- [ ] 회전이 복제되는지 확인
- [ ] 멈춤 상태가 복제되는지 확인

결과:

```text
미확인.
```

---

### 7.6 서버 전용 환경 오류 확인

목표:

```text
Dedicated Server에서 UI, 카메라, 로컬 사운드 코드 때문에 오류가 발생하지 않는다.
```

체크리스트:

- [ ] 서버 로그에 CreateWidget 관련 오류 없음
- [ ] 서버 로그에 카메라 관련 오류 없음
- [ ] 서버 로그에 로컬 플레이어 접근 오류 없음
- [ ] 서버 로그에 사운드/이펙트 관련 치명적 오류 없음
- [ ] Null Pointer 오류 없음

결과:

```text
미확인.
Dedicated Server 실행 성공과 별도로 로그 상세 검토가 필요하다.
```

---

## 8. 현재 테스트 기록

### 2026-05-26 환경 검증

```text
테스트 날짜:
- 2026-05-26

테스트 맵:
- /Game/Maps/TestMap.TestMap

서버 실행 방식:
- UE 5.7 Source Build 기반 Dedicated Server 실행

클라이언트 수:
- 아직 2클라 검증 전

성공 항목:
- UE 5.7 Source Build 전환 완료
- Dedicated Server 빌드 성공
- Dedicated Server 실행 성공

실패 항목:
- 없음. 단, 2클라 런타임 검증은 아직 미수행

주요 로그:
- 사용자가 Dedicated Server 빌드/실행 성공을 확인함

수정 필요 항목:
- TestMap / GameMode / PlayerController / Pawn 소유권 확인 필요

다음 액션:
- T-005 테스트 맵과 GameMode 확인
```

---

## 9. 실패 유형과 대응

### 9.1 클라이언트가 접속하지 못함

가능 원인:

```text
- 서버가 테스트 맵을 로드하지 못함
- 네트워크 주소 설정 문제
- 서버 Target 빌드 문제
- 방화벽 또는 포트 문제
```

대응:

```text
1. 서버 로그 확인
2. 맵 로드 여부 확인
3. 클라이언트 접속 주소 확인
4. 서버 빌드 상태 확인
```

---

### 9.2 차량이 스폰되지 않음

가능 원인:

```text
- GameMode DefaultPawn 설정 문제
- PlayerStart 부족
- Spawn 로직이 싱글 기준으로 작성됨
- 서버에서 Spawn하지 않음
```

대응:

```text
1. GameMode 확인
2. DefaultPawnClass 확인
3. PlayerStart 확인
4. 서버에서 Spawn되는지 로그 추가
```

---

### 9.3 입력이 적용되지 않음

가능 원인:

```text
- Pawn 소유권 문제
- 입력 바인딩 위치 문제
- 클라이언트가 소유하지 않은 Pawn에 입력 시도
- Enhanced Input Mapping Context 적용 위치 문제
```

대응:

```text
1. Possess 확인
2. IsLocallyControlled 확인
3. PlayerController 입력 초기화 확인
4. Mapping Context 적용 위치 확인
```

---

### 9.4 다른 차량이 보이지 않음

가능 원인:

```text
- Pawn bReplicates 꺼짐
- 이동 복제 설정 부족
- 서버에서 Spawn하지 않음
- Net Cull Distance 문제
```

대응:

```text
1. bReplicates 확인
2. bReplicateMovement 확인
3. 서버 Spawn 여부 확인
4. 네트워크 relevancy 확인
```

---

## 10. 변경 기록

### 0.2.0

```text
- UE 5.7 Source Build 전환 완료 상태 반영
- Dedicated Server 빌드 성공 상태 반영
- Dedicated Server 실행 성공 상태 반영
- T-004 서버 Target/빌드/실행 확인 완료 처리
- 남은 작업을 2클라 접속, 소유권, 입력, 이동 복제 검증으로 정리
```

### 0.1.0

```text
- Dedicated Server 최소 실행선 문서 최초 작성
- 실행 전 점검, 테스트 절차, 실패 유형 정의
```
