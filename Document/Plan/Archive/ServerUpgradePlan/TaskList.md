# 서버 고도화 작업 체크리스트

- 문서 버전: v0.1.6
- 작성일: 2026-06-01
- 대상 프로젝트: CarFight
- 상위 문서: `Roadmap.md`
- 상태: Active

---

## 1. 목적

이 문서는 서버 고도화 로드맵을 실제 작업 단위로 쪼갠 체크리스트다.

작업할 때는 이 문서에서 하나의 작업을 고르고, 완료 후 결과를 기록한다.

---

## 2. 상태 표기

```text
[ ] 아직 시작하지 않음
[-] 진행 중
[x] 완료
[!] 문제 발생 / 보류
```

---

## 3. Phase 0. 문서/상태 정렬

- [x] `ServerUpgradePlan` 폴더 생성
- [x] `README.md` 작성
- [x] `Scope.md` 작성
- [x] `CurrentState.md` 작성
- [x] `ServerDesign.md` 작성
- [x] `Roadmap.md` 작성
- [x] `TaskList.md` 작성
- [x] `TestPlan.md` 작성
- [x] `DecisionLog.md` 작성
- [x] `AuditResult.md` 작성
- [ ] 기존 `MP_ServerPlan/FeatureList.md`의 Server Target 상태 갱신 여부 판단
- [ ] `Document/ProjectSSOT/README.md`에 새 폴더 링크 추가 여부 판단

---

## 4. Phase 1. 최소 GameMode 코드 작성

### 4.1 신규 파일

- [x] `UE/Source/CarFight_Re/Public/CFMPGameMode.h` 생성
- [x] `UE/Source/CarFight_Re/Private/CFMPGameMode.cpp` 생성

### 4.2 클래스 기본 구조

- [x] `ACFMPGameMode` 클래스 선언
- [x] `AGameModeBase` 또는 `AGameMode` 상속 선택
- [x] `VehiclePawnClass` UPROPERTY 추가
- [x] `bSpawnVehicleOnPostLogin` UPROPERTY 추가
- [x] 접속 순번 또는 Spawn Index 관리 변수 추가

### 4.3 함수

- [x] `PostLogin` override 작성
- [x] `Logout` override 필요 여부 판단
- [x] `SpawnVehicleForController` 작성
- [x] `FindVehicleSpawnTransform` 작성
- [x] `ResolveVehiclePawnClass` 작성
- [x] 로그 helper 필요 여부 판단

### 4.4 로그

- [x] PlayerController 접속 로그
- [x] Spawn Transform 로그
- [x] Spawn 성공/실패 로그
- [x] Possess 성공/실패 로그
- [x] VehiclePawnClass 누락 로그

### 4.5 빌드 검증

- [x] `CarFight_ReEditor Win64 Development` 빌드 성공
- [x] `CarFight_ReServer Win64 Development` 빌드 성공
- [x] `CFMPGameMode_Review.md` 작성

---

## 5. Phase 2. Config / 맵 연결

### 5.1 Config

- [x] `UE/Config/DefaultEngine.ini` 백업 전 상태 확인
- [x] `GlobalDefaultGameMode` 추가 여부 결정
- [x] `GlobalDefaultServerGameMode` 추가 여부 결정
- [x] Config 수정 후 에디터/빌드 영향 확인
- [x] `CFMPGameModeConnect_Review.md` 작성

### 5.2 GameMode 런타임 기본값

- [x] `VehiclePawnClass` 기본 fallback 추가
- [x] 기본 BP 경로 `/Game/CarFight/Vehicles/BP_CFVehiclePawn` 사용
- [x] `DefaultPawnClass = nullptr` 설정
- [x] `HandleStartingNewPlayer_Implementation`에서 기본 Pawn 자동 시작 흐름 차단
- [x] `CarFight_ReEditor Win64 Development` 빌드 성공
- [x] `CarFight_ReServer Win64 Development` 빌드 성공

### 5.3 Blueprint 설정

- [x] `BP_CFVehiclePawn`의 AutoPossessPlayer 상태 확인
- [x] `BP_CFVehiclePawn`의 bReplicates 확인
- [x] `BP_CFVehiclePawn`의 bReplicateMovement 확인
- [x] `VehicleData = DA_PoliceCar` 유지 확인
- [x] `AssetMapAudit.md` 작성

### 5.4 TestMap

- [x] PlayerStart 개수 확인
- [x] PlayerStart 최소 2개 배치
- [x] PlayerStart 간 거리 확보
- [x] 배치 차량 제거 여부 결정
- [x] 배치 차량 제거 실행
- [x] Floor / 충돌 테스트 환경 유지 확인
- [x] `EditorTasks/TestMapMPSetup.md` 작성

---

## 6. Phase 3. 1클라 Dedicated 접속 검증

- [x] Dedicated Server 실행
- [x] Client 1 실행
- [x] Client 1 서버 접속
- [x] 서버 로그에서 PostLogin 확인
- [x] 서버 로그에서 차량 Spawn 확인
- [x] 서버 로그에서 Possess 확인
- [x] Client 1 화면 검정 화면 여부 확인
- [x] Client 1 카메라 정상 여부 확인
- [x] Client 1 입력 가능 여부 확인
- [x] 서버 로그 UI/LocalPlayer 오류 확인

---

## 7. Phase 4. 2클라 Dedicated 접속 검증

- [x] Client 1 접속
- [x] Client 2 접속
- [x] 서버에서 PlayerController 2개 확인
- [x] 서버에서 차량 2대 Spawn 확인
- [x] Client 1 Possess 대상 확인
- [x] Client 2 Possess 대상 확인
- [x] 두 클라이언트가 같은 차량을 소유하지 않는지 확인
- [x] 각 클라이언트 카메라 기준 확인
- [x] `RuntimeTest_20260601.md` 작성

---

## 8. Phase 5. 입력 분리 검증

- [x] Client 1 가속 입력 테스트
- [x] Client 1 조향 입력 테스트
- [x] Client 1 브레이크 입력 테스트
- [x] Client 1 핸드브레이크 입력 테스트
- [x] Client 2 가속 입력 테스트
- [x] Client 2 조향 입력 테스트
- [x] Client 2 브레이크 입력 테스트
- [x] Client 2 핸드브레이크 입력 테스트
- [x] Client 1 입력이 Client 2 차량에 적용되지 않는지 확인
- [x] Client 2 입력이 Client 1 차량에 적용되지 않는지 확인

---

## 9. Phase 6. 이동 복제 검증

- [x] Client 1 이동이 Client 2 화면에 보이는지 확인
- [x] Client 2 이동이 Client 1 화면에 보이는지 확인
- [x] 위치 복제 확인
- [x] 회전 복제 확인
- [x] 정지 상태 확인
- [x] 저속 주행 복제 확인
- [x] 고속 주행 복제 확인
- [x] 급조향 복제 확인
- [x] 심한 튐/지연 여부 기록

---

## 10. Phase 7. Dedicated Server 금지 코드 정리

### 10.1 검색 대상

- [x] 위젯 생성 경로 검색
- [x] Viewport 추가 경로 검색
- [x] LocalPlayer 접근 경로 검색
- [x] 카메라 Tick 경로 검색
- [x] 사운드 호출 후보 검색
- [x] 이펙트 호출 후보 검색
- [x] Debug HUD 생성 경로 검색
- [x] `DedicatedServerCodeAudit.md` 작성

### 10.2 수정 대상 후보

- [x] `ACFVehiclePawn` UI 생성 guard 확인
- [x] `UCFVehicleCameraComp` Dedicated Server Tick 차단 필요 여부 확인
- [x] `UCFVehicleAimComp` 서버/클라 Tick 책임 확인
- [x] Debug HUD/Panel 생성 guard 확인
- [x] 사운드/이펙트 후보 guard 확인
- [x] `CFCamDSTick_Review.md` 작성
- [x] `CarFight_ReEditor Win64 Development` 빌드 성공
- [x] `CarFight_ReServer Win64 Development` 빌드 성공

---

## 11. Phase 8. 1차 안정화 종료

- [x] 2클라 접속 PASS
- [x] 각자 차량 소유 PASS
- [x] 입력 분리 PASS
- [x] 기본 이동 복제 PASS
- [x] 서버 금지 코드 오류 없음 PASS
- [x] 남은 이슈 목록 작성
- [x] `DecisionLog.md` 갱신
- [x] `CurrentState.md` 갱신
- [x] `Roadmap.md`에서 Phase 8 완료 처리
- [x] `ServerStabilizationCloseout.md` 작성

---

## 12. 변경 기록

### v0.1.6

```text
- Phase 5 입력 분리와 Phase 6 이동 복제 PASS 상태 반영
- Phase 7 Dedicated Server 금지 코드 정리 완료 상태 반영
- Phase 8 서버 1차 안정화 종료 상태 반영
- `ServerStabilizationCloseout.md` 작성 완료 반영
```

### v0.1.5

```text
- 사용자 보고 기준 2클라 Dedicated Server 테스트 성공 상태 반영
- Phase 3 / Phase 4 주요 접속, Spawn, Possess 항목 완료 처리
- Phase 5 입력 분리와 Phase 6 이동 복제는 별도 검증으로 유지
- `RuntimeTest_20260601.md` 작성 완료 반영
```

### v0.1.4

```text
- BP_CFVehiclePawn 감사 완료 상태 반영
- TestMap 감사 결과 반영
- 배치 차량 제거 방침 결정 반영
- TestMap 2클라 서버 테스트용 에디터 작업지시서 작성 완료 반영
```

### v0.1.3

```text
- CFMPGameMode Connect 작업 완료 상태 반영
- Config 연결 완료 상태 반영
- GameMode 런타임 기본값과 기본 Pawn 차단 완료 상태 반영
- Phase 2의 남은 작업을 Blueprint 설정 확인과 TestMap 정리로 분리
```

### v0.1.2

```text
- Phase 1 최소 GameMode 코드 작성 완료 상태 반영
- Editor Target / Server Target 빌드 성공 상태 반영
- `CFMPGameMode_Review.md` 작성 완료 상태 반영
```

### v0.1.1

```text
- `TestPlan.md`, `DecisionLog.md`, `AuditResult.md` 작성 완료 상태 반영
- Phase 0 체크리스트 최신화
```

### v0.1.0

```text
- 서버 고도화 작업 체크리스트 최초 작성
- Phase 0부터 Phase 8까지 실제 작업 단위 분해
- 파일 생성, Config, 맵, 테스트, 서버 금지 코드 점검 항목 작성
```
