# CFNetSmooth 릴리즈 후보 정리

- 문서 버전: v0.3.0
- 작성일: 2026-06-18
- 대상 프로젝트: CarFight
- 작업 유형: 신규
- 상태: Release Candidate

---

## 1. 목적

이 문서는 `CFNetSmooth` 플러그인 자체 개발 세션을 반복 검증 단계에서 멈추고, 다른 세션이 바로 이어받을 수 있는 릴리즈 후보 상태로 고정하기 위한 기준 문서다.

핵심 목적은 다음이다.

```text
이미 충분히 통과한 검증을 다시 반복하지 않는다.
CFNetSmooth를 테스트 Actor 기준 v1.0 후보로 동결한다.
실제 차량 적용은 별도 세션에서 VehiclePrep.md 기준으로 진행한다.
코드 변경, 기본값 변경, 실행 환경 변경이 없으면 추가 반복 검증을 요구하지 않는다.
```

---

## 2. 현재 판정

현재 판정은 아래와 같다.

```text
CFNetSmooth는 테스트 Actor 기준 v1.0 Release Candidate로 본다.
플러그인 descriptor의 VersionName은 0.12.1이다.
이 문서의 v1.0 RC 판정은 코드 태그가 아니라 기능 동결 판정이다.
```

이 판정의 의미:

- 서버 권위 Transform 스무딩 플러그인 후보로는 충분히 검증됐다.
- 테스트 Actor, Dedicated Server, Development 패키지 경로는 통과했다.
- 같은 조건의 45초/180초 반복 검증은 더 진행하지 않는다.
- 실제 차량 Pawn 적용 완료를 의미하지 않는다.
- Shipping 패키징 완료를 의미하지 않는다.
- CarFight 전용 플러그인이라는 의미가 아니다.
- CarFight는 현재 검증 호스트이며, 플러그인은 다른 UE 프로젝트에서도 재사용될 수 있어야 한다.

---

## 3. 동결 범위

릴리즈 후보로 동결하는 기능은 다음이다.

```text
서버 권위 Transform State 송신
Unreliable NetMulticast 기반 일반 Transform State 전송
Reliable NetMulticast 기반 Teleport State 전송
수신 State Buffer 유지와 최대 개수 제한
InterpBackTime 기반 위치 보간
MaxExtrapTime 기반 제한 외삽
Teleport 수신 시 즉시 적용과 이전 Buffer 정리
지연 도착 Teleport State 예외 처리
위치 Snap
FRotator 저장 + FQuat Slerp 회전 보간
회전 Snap
테스트 Actor 기반 자동 Dedicated 검증 스크립트
Development Game 패키징
Development Dedicated Server 패키징
Development 패키지 서버/클라이언트 접속
Development 패키지 가시 클라이언트 최소 렌더링/접속/송수신
차량 Visual/Shell용 읽기 전용 보간 Transform API
프로젝트 독립 Runtime 모듈 메타데이터
```

기본값은 차량 적용 전까지 유지한다.

```text
InterpBackTime=0.10
MaxExtrapTime=0.20
PosSnapDist=500
RotSnapDeg=90
일반 Transform State=Unreliable
Teleport State=Reliable
```

프로젝트 독립성 동결 기준:

```text
CFNetSmooth Runtime 모듈은 CarFight_Re 같은 게임 프로젝트 모듈에 의존하지 않는다.
플러그인 Public API는 특정 프로젝트 Pawn, GameMode, 맵 경로를 요구하지 않는다.
CarFight 전용 검증 스크립트와 로그는 플러그인 필수 구성물로 보지 않는다.
```

---

## 4. 동결 제외 범위

아래 항목은 릴리즈 후보 동결 범위에 포함하지 않는다.

```text
실제 차량 Pawn 적용
실제 차량 Actor Transform 직접 보간
차량 Visual/Shell 생성과 연결
차량 Replicate Movement 저장값 변경
차량 AutoPhysics 정책 변경
차량 주행감 튜닝
Shipping 패키징
Marketplace 배포 품질 검수
사용자 육안 기준 최종 체감 판정
프로젝트별 샘플 맵과 샘플 Pawn 구성
```

주의:

- 실제 차량 적용 단계에서 `CFNetSmooth`가 차량 Transform 복제 역할을 맡으면 `Replicate Movement=false` 전환이 필요하다.
- 이 전환은 별도 차량 적용 세션에서 별도 패치로 진행한다.
- 현재 플러그인 제작 세션에서는 차량 코드와 Blueprint 기본값을 변경하지 않는다.

---

## 5. 검증 근거 요약

아래 검증은 이미 통과로 기록되어 있다.

| 구분 | 상태 | 근거 문서 |
|---|---|---|
| 에디터 빌드 | 통과 | `TestResults.md` |
| 컴포넌트 에디터 표시 | 통과 | `TestResults.md` |
| 서버 Send / 클라이언트 Receive | 통과 | `TestResults.md` |
| 위치 보간 입력 State 확보 | 통과 | `TestResults.md` |
| Teleport 적용 | 통과 | `TestResults.md` |
| 위치 Snap | 통과 | `TestResults.md` |
| 제한 외삽 | 통과 | `TestResults.md` |
| 회전 송수신 | 통과 | `TestResults.md` |
| 회전 Snap | 통과 | `TestResults.md` |
| Dedicated 기본 반복 | 통과 | `TestResults.md` |
| Teleport 손실 20% | 통과 | `TestResults.md` |
| Teleport 손실 20% + 지연 100ms | 통과 | `TestResults.md` |
| Teleport 손실+지연 3회 반복 | 통과 | `TestResults.md` |
| 일반 Transform 손실/지연 악조건 | 통과 | `TestResults.md` |
| Win64 Development Game 패키징 | 통과 | `TestResults.md` |
| Win64 Development Server 패키징 | 통과 | `TestResults.md` |
| 패키지 서버/클라이언트 접속 | 통과 | `TestResults.md` |
| 패키지 가시 클라이언트 최소 렌더링 | 통과 | `TestResults.md` |
| 차량 Visual/Shell용 읽기 API 빌드 | 통과 | `TestResults.md` |

최종 Development 패키지 계열 근거:

```text
RuntimeLogs/CFNetSmoothPackage/20260617_152707
RuntimeLogs/CFNetSmoothServerPackage/20260617_153553
RuntimeLogs/CFNetSmoothPackageNet/20260617_154137
RuntimeLogs/CFNetSmoothVisualClient/20260617_154800
```

0.12.0 API 추가 후 확인:

```text
Tools\BuildEditor.bat 성공
TryGetSmoothedTransform 계열 공개 API UHT/컴파일 통과
Dedicated 10초/20초 스모크는 접속/맵 로드/Fatal/Error/Timeout 0건 확인
Dedicated Send/Receive 로그는 0건이라 송수신 재판정 근거로 쓰지 않음
```

0.12.1 범용화 확인:

```text
CFNetSmooth.uplugin Description에서 CarFight 전용 표현 제거
CFNetSmooth.uplugin CreatedBy를 CFNetSmooth로 정리
CFNetSmooth.Build.cs에서 CarFight_Re 모듈 의존성 표현 제거
UE/Plugins/CFNetSmooth 검색 기준 CarFight 전용 Runtime 코드 의존성 없음
Tools\BuildEditor.bat 성공
```

해석:

```text
0.12.0 API 추가 자체는 빌드 기준 통과다.
Dedicated 로그 0건은 현재 TestMap 또는 DebugLog 조건 확인 대상으로 분리한다.
같은 조건의 스모크 시간을 늘려 반복하지 않는다.
```

---

## 6. 반복 검증 중단 기준

아래 조건에서는 추가 반복 검증을 하지 않는다.

```text
코드가 바뀌지 않았다.
기본값이 바뀌지 않았다.
엔진 경로가 바뀌지 않았다.
패키징 설정이 바뀌지 않았다.
테스트 맵과 테스트 Actor 조건이 바뀌지 않았다.
이미 통과한 명령을 같은 조건으로 다시 실행하는 상황이다.
```

추가 검증이 필요한 경우는 아래로 제한한다.

```text
CFNetSmooth 코드가 바뀐다.
CFNetSmooth 기본값이 바뀐다.
테스트 Actor 또는 테스트 맵 조건이 바뀐다.
Development가 아니라 Shipping으로 패키징한다.
실제 차량 Visual/Shell에 연결한다.
실제 차량 Replicate Movement 정책을 바꾼다.
사용자가 직접 체감 문제를 보고한다.
```

판정:

```text
같은 조건의 45초 검증과 180초 검증은 의미 차이가 작다.
새로운 실패 원인이 추가되지 않는 한 장시간 반복보다 적용 설계와 인계 정리가 우선이다.
```

---

## 7. 남은 제한사항

남은 제한사항은 다음이다.

```text
Shipping 패키징은 아직 통과 기록이 없다.
실제 차량 Pawn에는 아직 연결하지 않았다.
사용자 육안 기준 최종 부드러움 평가는 별도이다.
Teleport 가시 체감은 자동 로그가 아니라 사람이 보는 확인이 더 정확하다.
SceneComponent 대상 직접 적용 API 또는 delegate가 필요할 수 있다.
고속 차량 적용 시 SendRate와 위치 정밀도는 재검토할 수 있다.
회전 외삽은 현재 구현 범위가 아니다.
```

이 제한사항은 릴리즈 후보 판정을 막지 않는다.

사유:

```text
위 항목들은 플러그인 자체 송수신/보간 구현의 결함이 아니라
배포 환경, 실제 차량 통합, 사용자 체감, 확장 API 범위에 해당한다.
```

---

## 8. 차량 적용 세션 인계 기준

차량 적용 세션은 아래 순서로 문서를 읽는다.

```text
1. README.md
2. ReleaseCandidate.md
3. VehiclePrep.md
4. Decisions.md
5. TestResults.md
6. Api.md
7. Presets.md
8. Verify.md
```

차량 적용 세션의 시작 원칙:

```text
기존 차량 기준선은 먼저 Default + RepMove=true + AutoPhysics=true로 유지한다.
CFNetSmooth를 실제 차량 Transform 복제 역할로 연결하는 순간에는 RepMove=false 전환을 별도 패치로 수행한다.
실제 차량 Actor나 VehicleMesh에는 CFNetSmooth 수신 Transform을 직접 적용하지 않는다.
원격 Visual/Shell 표시용 대상에만 적용하는 구조를 우선한다.
플러그인 API 수정이 필요하면 차량 적용 세션에서 즉시 고치지 말고 CFNetSmooth 제작 세션으로 되돌린다.
차량 적용 세션에서 필요한 CarFight 전용 코드는 플러그인 Runtime 코드가 아니라 호스트 프로젝트 쪽에 둔다.
```

차량 적용 세션에서 먼저 확인할 체크리스트:

```text
[ ] VehiclePrep.md의 Transform 책임 지도를 읽었다.
[ ] 실제 차량 Actor Transform을 누가 소유하는지 확인했다.
[ ] Visual/Shell 후보가 충돌/물리/입력/서버 판정에서 분리되어 있다.
[ ] RepMove=false 전환이 CFNetSmooth 연결과 같은 패치 또는 명확한 단계로 묶여 있다.
[ ] 롤백 시 RepMove=true + AutoPhysics=true 기준선으로 돌아가는 방법이 준비되어 있다.
```

---

## 9. 변경 관리 기준

릴리즈 후보 이후 `UE/Plugins/CFNetSmooth`를 수정해야 한다면 아래 규칙을 따른다.

```text
1. 수정 사유를 Decisions.md에 먼저 기록한다.
2. 코드 변경 버전을 올린다.
3. Api.md에 API 변경을 기록한다.
4. TestResults.md에는 변경 후 최소 재검증만 추가한다.
5. 같은 조건의 반복 검증을 늘리지 않는다.
```

최소 재검증 기준:

```text
Tools\BuildEditor.bat
Tools\RunCFNetSmoothDedicated.bat 10 1
수정한 기능에 해당하는 단일 악조건 테스트 1회
```

단, `RunCFNetSmoothDedicated.bat` Summary의 Send/Receive가 0이면 송수신 검증으로 판정하지 않는다.
이 경우 같은 명령을 반복하지 말고 DebugLog 조건 또는 명령줄 override 필요 여부를 먼저 확인한다.

패키징 관련 코드를 바꾼 경우에만 Development 패키징을 다시 수행한다.

Shipping 검증은 별도 배포 준비 작업으로 분리한다.

---

## 10. 최종 결론

현재 세션 기준 최종 결론은 다음이다.

```text
CFNetSmooth 자체 개발은 테스트 Actor 기준 완료로 본다.
이후 같은 조건의 검증을 반복하지 않는다.
다음 의미 있는 작업은 차량 적용 세션 준비 또는 Shipping 배포 검증이다.
차량 적용은 VehiclePrep.md 기준으로 실제 차량 Transform 책임을 먼저 정리한 뒤 진행한다.
```

---

## 11. Changelog

### v0.3.0

```text
- CFNetSmooth 0.12.1 기준으로 플러그인 descriptor VersionName 갱신
- CarFight는 검증 호스트이며 CFNetSmooth 자체는 다른 UE 프로젝트에서도 재사용 가능한 Runtime 플러그인이라는 기준 추가
- 프로젝트 독립성 동결 기준과 0.12.1 manifest/Build.cs 범용화 확인 내용 추가
```

### v0.2.1

```text
- CFNetSmooth 0.12.0 차량 Visual/Shell용 읽기 API의 BuildEditor 통과 결과 추가
- 0.12.0 이후 Dedicated 스모크 로그 0건은 송수신 재판정 근거로 쓰지 않고 DebugLog 조건 확인 대상으로 분리
- 최소 재검증에서 Send/Receive 0건일 때 같은 명령 반복 대신 로그 조건을 먼저 확인하도록 기준 추가
```

### v0.2.0

```text
- CFNetSmooth 0.12.0 기준 차량 Visual/Shell용 읽기 전용 보간 Transform API 추가 상태 반영
- TryGetSmoothedTransform 계열 API를 릴리즈 후보 동결 범위에 추가
- 남은 제한사항을 SceneComponent 직접 적용 API/delegate 보류로 갱신
```

### v0.1.0

```text
- CFNetSmooth 릴리즈 후보 정리 문서 최초 작성
- 반복 검증 중단 기준, 동결 범위, 제한사항, 차량 적용 세션 인계 기준 추가
- 테스트 Actor 기준 v1.0 Release Candidate 판정 명시
```

---

## 12. 마이그레이션 지침

- 이 문서는 차량 적용 지시서가 아니라 CFNetSmooth 자체 개발 완료 판정 문서다.
- 차량 적용은 반드시 `VehiclePrep.md`를 함께 읽고 진행한다.
- 기존 검증 결과를 삭제하지 말고, 이후 변경은 Changelog로 누적한다.
- 같은 조건의 장시간 검증을 추가하기 전에 6장의 반복 검증 중단 기준을 먼저 확인한다.
- 다른 프로젝트 적용 시 CarFight 검증 결과는 참고 자료로만 사용하고, 호스트 프로젝트 전용 타입/맵/Build Target을 플러그인 Runtime 코드에 추가하지 않는다.
