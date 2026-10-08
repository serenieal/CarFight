# CarFight - P0 메시 소켓 감사

> 역할: P0 후보 StaticMesh의 Socket 목록과 하드포인트 작성 보조 가능 여부를 기록한다.
> 문서 버전: v0.3
> 마지막 정리(Asia/Seoul): 2026-06-25
> 상태: Draft / Socket Evidence Before Code

---

## 1. 목적

이 문서는 P0 차량 플랫폼 코드 작업 전에 실제 차체 메시의 Socket 상태를 고정한다.

확인 목표:

```text
1. Combined_Body에 Socket이 없는지 다시 확인한다.
2. Mesh_TestSedan / Mesh_TestSUV에 사용자가 추가한 Socket이 실제로 존재하는지 확인한다.
3. 해당 Socket을 하드포인트 위치 작성 보조로 쓸 수 있는지 판단한다.
4. Transform 값을 C++ 하드코딩값으로 오해하지 않게 DataAsset 캡처 기준을 명시한다.
```

---

## 2. 실행 기준

AssetDump는 소스 엔진 기준으로 실행했다.

```text
Editor Commandlet:
  D:\UnrealEngine_Source\Engine\Binaries\Win64\UnrealEditor-Cmd.exe

Project:
  D:\Work\CarFight_git\UE\CarFight_Re.uproject

Mode:
  -run=AssetDump
  -Mode=bpdump
  -IncludeDetails=true
  -IncludeSummary=true
  -IncludeGraphs=false
  -IncludeReferences=false
```

중요:

```text
bpdump 기본값은 IncludeDetails=false다.
StaticMesh Socket 전체 목록을 보려면 IncludeDetails=true가 필요하다.
summary.static_mesh_socket_preview는 최대 5개까지만 표시하므로 전체 목록 근거로 쓰지 않는다.
```

근거 파일:

```text
Document/Plan/VehiclePlatformPlan/SocketAudit_20260625/
  Combined_Body_bpdump_details/
  Mesh_TestSedan_bpdump_details/
  Mesh_TestSUV_bpdump_details/
```

---

## 3. 요약 결과

| 메시 | AssetDump 상태 | Socket 수 | 판단 |
|---|---:|---:|---|
| `/Game/CarFight/Vehicles/police_car/StaticMeshes/Combined/Combined_Body` | success | 0 | 임시 감사 자산. 하드포인트 Socket 없음 |
| `/Game/CarFight/Vehicles/TestSedan/Mesh_TestSedan` | success | 6 | 휠 4개 + 하드포인트 후보 2개 |
| `/Game/CarFight/Vehicles/TestSUV/Mesh_TestSUV` | success | 7 | 휠 4개 + 하드포인트 후보 3개 |

결론:

```text
Mesh_TestSedan / Mesh_TestSUV의 Socket은 하드포인트 위치 작성 보조로 참고 가능하다.
하지만 P0 런타임 원본은 계속 차량 DataAsset의 LocalTransform이다.
Socket Transform 값은 C++ 하드코딩 대상이 아니라 DataAsset 캡처 후보값이다.
```

---

## 4. Combined_Body

| SocketName | LocalLocation | LocalRotation | 판단 |
|---|---|---|---|
| 없음 | 없음 | 없음 | 기존 사용자 에디터 확인과 동일하게 0 소켓 |

메모:

```text
Combined_Body는 현재 CFVehicleData 적용 경로를 확인하기 위한 임시 감사 자산이다.
P0 최종 하드포인트 수치 원본으로 사용하지 않는다.
```

---

## 5. Mesh_TestSedan

| SocketName | LocalLocation | LocalRotation | 용도 판단 |
|---|---|---|---|
| `Wheel_Anchor_FL` | `(160, -75, 0)` | `(0, 0, 0)` | 휠 작성 기준 |
| `Wheel_Anchor_FR` | `(160, 75, 0)` | `(0, 0, 0)` | 휠 작성 기준 |
| `Wheel_Anchor_RL` | `(-110, -75, 0)` | `(0, 0, 0)` | 휠 작성 기준 |
| `Wheel_Anchor_RR` | `(-110, 75, 0)` | `(0, 0, 0)` | 휠 작성 기준 |
| `Front_01` | `(175, 0, 60)` | `(0, 0, 0)` | 하드포인트 위치 후보 |
| `Top_01` | `(-10, 0, 120)` | `(0, 0, 0)` | 하드포인트 위치 후보 |

P0 위치 슬롯 후보:

| LocationSlotId | SocketName | LocationCategory | 캡처 후보 Transform |
|---|---|---|---|
| `Front_01` | `Front_01` | `Front` | Location `(175, 0, 60)`, Rotation `(0, 0, 0)` |
| `Top_01` | `Top_01` | `Top` | Location `(-10, 0, 120)`, Rotation `(0, 0, 0)` |

주의:

```text
Mesh_TestSedan에는 현재 Front_02 소켓이 없다.
이는 P0 차단 조건이 아니다.
Front_02 + Gimbal은 전방 하드포인트가 늘어날 수 있다는 가능성으로만 다룬다.
```

---

## 6. Mesh_TestSUV

| SocketName | LocalLocation | LocalRotation | 용도 판단 |
|---|---|---|---|
| `Wheel_Anchor_FL` | `(160, -80, 0)` | `(0, 0, 0)` | 휠 작성 기준 |
| `Wheel_Anchor_FR` | `(160, 80, 0)` | `(0, 0, 0)` | 휠 작성 기준 |
| `Wheel_Anchor_RL` | `(-130, -80, 0)` | `(0, 0, 0)` | 휠 작성 기준 |
| `Wheel_Anchor_RR` | `(-130, 80, 0)` | `(0, 0, 0)` | 휠 작성 기준 |
| `Front_01` | `(190, 0, 80)` | `(0, 0, 0)` | 하드포인트 위치 후보 |
| `Top_01` | `(-10, -45, 140)` | `(0, 0, 0)` | 하드포인트 위치 후보 |
| `Top_02` | `(-10, 45, 140)` | `(0, 0, 0)` | 하드포인트 위치 후보 |

P0 위치 슬롯 후보:

| LocationSlotId | SocketName | LocationCategory | 캡처 후보 Transform |
|---|---|---|---|
| `Front_01` | `Front_01` | `Front` | Location `(190, 0, 80)`, Rotation `(0, 0, 0)` |
| `Top_01` | `Top_01` | `Top` | Location `(-10, -45, 140)`, Rotation `(0, 0, 0)` |
| `Top_02` | `Top_02` | `Top` | Location `(-10, 45, 140)`, Rotation `(0, 0, 0)` |

주의:

```text
Mesh_TestSUV에는 Top_02가 있어 복수 루프 슬롯 검증 후보로 좋다.
Mesh_TestSUV에도 현재 Front_02 소켓은 없다.
```

---

## 7. 명명 결정

2026-06-25 AssetDump 당시 실제 메시 Socket 이름은 아래 형태다.

```text
Front_01
Top_01
Top_02
```

확정된 표준 SocketName은 아래 형태다.

```text
HP_Front_01
HP_Top_01
HP_Top_02
```

판단:

```text
현재 Socket 이름은 위치 슬롯 인스턴스만 표현하므로 Fixed / Gimbal / Turret 타입을 섞지 않는 원칙은 만족한다.
하지만 프로젝트 표준은 HP_ prefix를 붙이는 쪽으로 확정한다.
따라서 이 소켓을 캡처 입력으로 계속 사용할 경우 Mesh_TestSedan / Mesh_TestSUV의 하드포인트 Socket은 코드 착수 전에 리네임해야 한다.
```

리네임 대상:

| 현재 이름 | 표준 이름 |
|---|---|
| `Front_01` | `HP_Front_01` |
| `Top_01` | `HP_Top_01` |
| `Top_02` | `HP_Top_02` |

P0 안전 기준:

```text
자동 캡처 기본값은 HP_ prefix 표준명을 사용한다.
단, 외부 메시 예외를 위해 차량 DataAsset의 SocketName override 필드는 유지할 수 있다.
차량마다 하드포인트 소켓은 해당 차량 설정상 있을 수도 있고 없을 수도 있다.
캡처가 끝나면 런타임 Fire / Aim은 SocketName이 아니라 DataAsset LocalTransform을 읽는다.
```

---

## 8. 코드 착수 전 남은 작업

| 항목 | 상태 | 다음 작업 |
|---|---:|---|
| P0 메시 소켓 존재 | 확인됨 | Sedan 2개, SUV 3개 하드포인트 후보 |
| `Front_02` 후보 | 확장 가능성 | 하드포인트가 늘어날 수 있음을 문서에만 유지 |
| SocketName prefix | 확정 | 캡처 입력으로 사용할 하드포인트 소켓은 `HP_` prefix로 리네임 |
| DataAsset 캡처 방식 | 확정 | 바퀴 위치 캡처 버튼에 하드포인트 캡처 통합 |
| 런타임 원본 | 확정 | DataAsset LocalTransform |

---

## 9. Changelog

### v0.3

- `Front_02 + Gimbal`을 P0 결정 항목이 아니라 하드포인트 증가 가능성으로 낮췄다.
- 하드포인트 소켓은 차량별 설정에 따라 있을 수도 있고 없을 수도 있음을 명시했다.
- SocketName 리네임 기준을 캡처 입력으로 계속 사용할 소켓에 적용하는 규칙으로 좁혔다.

### v0.2

- SocketName prefix 정책을 `HP_` prefix 리네임으로 확정했다.
- DataAsset 캡처 방식을 바퀴 위치 캡처 버튼에 통합하는 자동 버튼 방식으로 확정했다.
- 현재 AssetDump 이름은 감사 기록으로 유지하되, 코드 착수 전 리네임 작업이 필요함을 명시했다.

### v0.1

- `Combined_Body`, `Mesh_TestSedan`, `Mesh_TestSUV` StaticMesh Socket을 AssetDump `bpdump -IncludeDetails=true`로 확인했다.
- `Mesh_TestSedan`에는 `Front_01`, `Top_01` 하드포인트 후보가 있음을 기록했다.
- `Mesh_TestSUV`에는 `Front_01`, `Top_01`, `Top_02` 하드포인트 후보가 있음을 기록했다.
- 현재 Socket 이름이 문서 권장 예시의 `HP_` prefix와 다르므로 코드 착수 전 명명 결정을 남겼다.

---

## 10. Migration 메모

- 이 문서는 코드 또는 `.uasset` 변경 지시서가 아니다.
- Socket Transform은 C++ 하드코딩값이 아니라 DataAsset 캡처 후보값이다.
- P0 런타임은 Mesh Socket 존재에 의존하지 않는다.
