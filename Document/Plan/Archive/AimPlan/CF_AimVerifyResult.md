# CarFight — CF_AimVerifyResult

> 역할: Aim 시스템 1차 구현과 에디터 수동 연결 이후 확인된 검증 결과를 기록한다.
> 문서 버전: v1.4.0
> 마지막 정리(Asia/Seoul): 2026-06-19
> 상태: Completed / Single Player Verification Snapshot + Local Fire Rename

---

## 1. 검증 목적

이 문서는 AimPlan의 1차 구현 결과를 실제 확인 기준으로 정리한다.

검증 범위는 다음과 같다.

```text
C++ 빌드 검증
UE Editor 수동 연결 검증
Single Player PIE 기본 확인
Fire 입력 확인
남은 싱글플레이 후속 검증 항목 정리
Legacy Network Name Audit
```

---

## 2. 구현 단계별 상태

| 단계 | 이름 | 상태 | 확인 기준 |
|---:|---|---|---|
| R1/R2 | Aim 타입 + AimComp 골격 | 완료 | C++ 빌드 성공 |
| R3 | Local Aim 계산 | 완료 | C++ 빌드 성공 |
| R4 | VehicleDebug Aim | 완료 | C++ 빌드 성공 |
| R5/R6 | Fire Command + 로컬 검증 | 완료 | C++ 빌드 성공 |
| R7 | Local HitScan 더미 | 완료 | C++ 빌드 성공 |
| R8 | Fire Visual 기반 | 완료 | C++ 빌드 성공 |
| R9 | Reticle UI C++ 부모 | 완료 | C++ 빌드 성공 |
| R10 | Reticle Widget Pawn 연결 준비 | 완료 | C++ 빌드 성공 |
| Editor | WBP / IA / BP 연결 | 완료 | 사용자 수동 확인 + 일부 MCP 에셋 확인 |
| PIE | Single Player / Fire 확인 | 완료 | 사용자 수동 확인 |

---

## 3. 빌드 검증 기록

각 단계에서 `CarFight_ReEditor / Win64 / Development` 빌드를 확인했다.

확인된 빌드 결과는 모두 성공이다.

```text
R1/R2 Build: Succeeded
R3 Build: Succeeded
R4 Build: Succeeded
R5/R6 Build: Succeeded
R7 Build: Succeeded
R8 Build: Succeeded
R9 Build: Succeeded
R10 Build: Succeeded
```

마지막 확인 빌드:

```text
Target: CarFight_ReEditor
Platform: Win64
Configuration: Development
Result: Succeeded
```

---

## 4. UE Editor 연결 검증 기록

사용자 수동 확인 기준으로 다음 연결이 완료되었다.

```text
WBP_AimReticle 연결 완료
IA_Fire 연결 완료
IMC_Vehicle_Default Fire 매핑 완료
BP_CFVehiclePawn InputAction_Fire 지정 완료
BP_CFVehiclePawn AimReticleWidgetClass 지정 완료
Fire 입력 확인 완료
```

MCP 확인 기준:

```text
/Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle 개별 에셋 덤프 성공
class_name = WidgetBlueprint
resolved_class = /Game/CarFight/UI/WBP_AimReticle.WBP_AimReticle_C
Text Reticle State / Text Can Fire / Text Reticle Hint 관련 키 샘플 확인
```

주의:

```text
/Game/CarFight/UI 전체 asset list 조회는 UE bridge 기본 15초 제한으로 timeout 발생.
개별 WBP_AimReticle dump는 성공했으므로, 전체 목록 timeout은 기능 실패로 보지 않는다.
```

---

## 5. 현재 작동 흐름

현재 확인된 Aim 시스템 흐름은 다음과 같다.

```text
Look 입력
  -> UCFVehicleCameraComp Aim Trace
  -> UCFVehicleAimComp Local Aim 계산
  -> Reticle State 계산
  -> WBP_AimReticle 표시

Fire 입력
  -> InputAction_Fire
  -> ACFVehiclePawn::HandleFireStarted()
  -> ACFVehiclePawn::BuildFireCommand()
  -> FCFVehicleFireRequest 생성
     - 현재 의미는 로컬 Fire Command 데이터
  -> ACFVehiclePawn::ValidateFireCommand()
  -> ACFVehiclePawn::RunLocalDummyHitScan()
  -> FCFVehicleFireResult 생성
  -> ACFVehiclePawn::ApplyFireResult()
  -> UCFVehicleAimComp FireValidationState / AimVisualState 갱신
  -> VehicleDebug Aim에서 확인 가능
```

---

## 6. 완료된 기능 범위

현재 완료된 기능은 다음이다.

- Aim 관련 C++ 타입 정의
- AimComp 골격
- Local Aim State 계산
- Reticle State 계산
- VehicleDebug Aim 카테고리
- Fire Command 생성
- Local Fire Validation
- Local Dummy HitScan
- FireResult 반환
- FireValidationState 반영
- FireVisualState 기반 표시 준비
- Aim Reticle C++ 부모 위젯
- Pawn에서 Reticle Widget 선택 생성
- WBP_AimReticle 에디터 연결
- IA_Fire 에디터 연결
- Single Player Fire 확인
- 레거시 네트워크 코드명 감사
- Pawn 단위 서버/RPC Fire 경로 제거
- FireValidationState / AimVisualState 명칭 정리
- Reticle enum 싱글플레이 명칭 정리
- Local Aim Trace Debug 표시명 정리
- Aim 타입 Net Serialization 제거 빌드/에디터 검증

---

## 7. 아직 완료하지 않은 기능 범위

아래는 아직 완료된 것으로 보지 않는다.

- Standalone Game 검증
- 패키지 실행 후보 검증
- Fire FX / 사운드 연결
- 실제 무기 시스템
- Ammo / Cooldown / Reload
- Damage 적용
- Projectile
- Hit Marker
- Lock-On
- AI Combat
- 부품 파괴 연동

---

## 8. 현재 판정

Aim 시스템 1차 구현은 완료 상태로 본다.

정확한 완료 범위는 다음이다.

```text
Aim Core 1차 완료
Reticle C++/WBP 연결 완료
Fire Command 1차 완료
Local Validation 1차 완료
Local Dummy HitScan 완료
Single Player 기본 확인 완료
```

멀티플레이 실기 검증은 프로젝트 방향 변경으로 중단한다. 현재 완료 판정은 싱글플레이 Aim Core 기준이다.

---

## 9. 다음 권장 단계

다음 단계는 기능 추가보다 싱글플레이 실행 검증 안정화가 우선이다.

권장 순서:

```text
1. 싱글플레이 전환 완료 판정 기록
2. 필요 시 Standalone Game 후속 검증
```

가장 먼저 진행할 문서는 다음이 적합하다.

```text
CF_AimVerify.md
CF_AimNameAudit.md
```

이 문서는 싱글플레이 검증만 다루고, 새 기능 구현은 포함하지 않는다.

---

## ChangeLog

- v1.1.0 / 2026-06-19
  - 검증 결과 문서를 싱글플레이 Aim Core 완료 기준으로 전환했다.
  - 남은 검증을 Listen Server / Dedicated Server에서 Standalone / Local HitScan / Reticle 재확인으로 변경했다.
  - 멀티플레이 검증 중단 사유를 프로젝트 방향 변경으로 명시했다.
- v1.2.0 / 2026-06-19
  - 실제 C++ 코드에 남은 서버/RPC/복제 계열 이름을 레거시 명칭으로 명시했다.
  - `CF_AimNameAudit.md` 완료 상태를 반영했다.
  - Standalone 검증 전 Debug 표시명 / Tooltip 정리를 선행 후보로 추가했다.
- v1.3.0 / 2026-06-19
  - 현재 Fire 입력 흐름을 `BuildFireCommand -> ValidateFireCommand -> RunLocalDummyHitScan -> ApplyFireResult` 기준으로 갱신했다.
  - Pawn 단위 서버/RPC Fire 경로 제거와 Aim 상태/Reticle/Trace Debug 명칭 정리 완료를 반영했다.
  - Aim 타입 Net Serialization 제거는 빌드 완료 / 에디터 재확인 필요 항목으로 분리했다.
- v1.4.0 / 2026-06-19
  - 사용자 에디터 정상작동 확인을 반영해 Aim 타입 Net Serialization 제거 후 확인 상태를 완료로 변경했다.
  - 다음 권장 단계를 기능 추가가 아니라 싱글플레이 전환 완료 판정 기록으로 축소했다.

## 마이그레이션 지침

- 이후 검증 결과는 `CF_AimVerify.md`의 싱글플레이 기준으로 이어서 기록한다.
- `CF_AimNetVerify.md`는 Deprecated 문서이므로 다음 검증 문서로 사용하지 않는다.
- Pawn 단위 서버/RPC Fire 경로는 제거됐으므로 새 검증 문서에서 해당 경로를 현재 흐름으로 쓰지 않는다.
- 남은 `BuildFireRequest` / `FCFVehicleFireRequest` 이름은 현재 로컬 Fire Command 데이터 의미로 해석한다.

## Change Note

- 2026-06-19: 이전 검증 결과의 레거시 서버/RPC 흐름 설명을 현재 코드 흐름으로 정정했다.
- 2026-06-19: 에디터 정상작동 확인으로 재확인 필요 항목을 완료 처리했다. Standalone Game 검증은 후속 검증 후보로 유지한다.
