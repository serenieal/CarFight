# CarFight — CF_AimVerifyResult

> 역할: Aim 시스템 1차 구현과 에디터 수동 연결 이후 확인된 검증 결과를 기록한다.  
> 문서 버전: v1.0.0  
> 마지막 정리(Asia/Seoul): 2026-05-21  
> 상태: Completed / Verification Snapshot

---

## 1. 검증 목적

이 문서는 AimPlan의 1차 구현 결과를 실제 확인 기준으로 정리한다.

검증 범위는 다음과 같다.

```text
C++ 빌드 검증
UE Editor 수동 연결 검증
Single Player PIE 기본 확인
Fire 입력 확인
남은 Multiplayer 검증 항목 정리
```

---

## 2. 구현 단계별 상태

| 단계 | 이름 | 상태 | 확인 기준 |
|---:|---|---|---|
| R1/R2 | Aim 타입 + AimComp 골격 | 완료 | C++ 빌드 성공 |
| R3 | Local Aim 계산 | 완료 | C++ 빌드 성공 |
| R4 | VehicleDebug Aim | 완료 | C++ 빌드 성공 |
| R5/R6 | Fire Request + Server 검증 | 완료 | C++ 빌드 성공 |
| R7 | Server HitScan 더미 | 완료 | C++ 빌드 성공 |
| R8 | RepAimVisual | 완료 | C++ 빌드 성공 |
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
  -> FCFVehicleFireRequest 생성
  -> ServerRequestFire()
  -> ValidateFireRequestOnServer()
  -> RunServerDummyHitScan()
  -> FCFVehicleFireResult 생성
  -> ClientReceiveFireResult()
  -> UCFVehicleAimComp ServerAimState / RepAimVisual 갱신
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
- Fire Request 생성
- Server Fire Validation
- Server Dummy HitScan
- FireResult 반환
- ServerAimState 반영
- RepAimVisualState 기반 복제 준비
- Aim Reticle C++ 부모 위젯
- Pawn에서 Reticle Widget 선택 생성
- WBP_AimReticle 에디터 연결
- IA_Fire 에디터 연결
- Single Player Fire 확인

---

## 7. 아직 완료하지 않은 기능 범위

아래는 아직 완료된 것으로 보지 않는다.

- Listen Server + 1 Client 검증
- Listen Server + 2 Clients 검증
- Dedicated Server 검증
- Fire FX Multicast
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
Fire Request 1차 완료
Server Validation 1차 완료
Server Dummy HitScan 완료
Single Player 기본 확인 완료
```

아직 멀티플레이 실기 검증은 남아 있으므로, 네트워크 전투 완성으로 판정하지 않는다.

---

## 9. 다음 권장 단계

다음 단계는 기능 추가보다 검증 안정화가 우선이다.

권장 순서:

```text
1. Listen Server + 1 Client 검증
2. Listen Server + 2 Clients 검증
3. 검증 결과 기반 버그 수정
4. Fire FX Multicast
5. HitScan Damage 더미
6. WeaponComp 설계 / 분리
```

가장 먼저 진행할 문서는 다음이 적합하다.

```text
CF_AimNetVerify.md
```

이 문서는 Listen Server / Client 검증만 다루고, 새 기능 구현은 포함하지 않는다.
