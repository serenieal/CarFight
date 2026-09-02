# VT07 VehiclePanel Asset-First

- 문서 버전: v0.2
- 날짜: 2026-08-27
- 상태: Local Frame Source 2종 + PNG PASS / Asset-First Whole Panel Review Ready / USER Visual Review Pending
- Feature: CF-FQ-039 / VPR-P0-01
- Production Visual Target: VT-VEH-01
- Outer Frame Source: VT06_FrameMaster_v5.png

## 목적

URT05에서 검증한 Asset-First 원칙을 실제 VehiclePanel에 적용한다. 완성 Concept을 먼저 그려 역추출하지 않고, 실제 독립 편집 가치가 있는 Source만 먼저 만든 뒤 기존 Production-Proven 자산과 합성한다.

## 재사용 — 재제작 금지

- RPM: 기존 Image_RPMGauge + M_UI_RPMGauge + RPMRatio 및 current Source Pack
- Armor: WBP_CFArmorSector 6개 + VT04 common Plate / Arrow / Chevron2
- Vehicle silhouette: VT05 Sedan/SUV source + Production catalog
- Defense: VT06 DefBarFrame / FillMask / Hex / Chevron + M_UI_DefFill
- Root outer shell: VT06_FrameMaster_v5

## 신규 Source — 정확히 2종

1. VT07_SpeedFrame
   - 422x272
   - Speed/RPM local static mechanical frame
   - static cyan hierarchy까지 같은 Asset에 bake
   - RPM track, speed digits, gear value는 포함하지 않음

2. VT07_ArmorFrame
   - 422x272
   - ArmorBodyMap local static mechanical frame/backdrop
   - static amber hierarchy와 open cradle detail을 같은 Asset에 bake
   - armor values, direction semantics, vehicle silhouette는 포함하지 않음

## Layer Separation Gate

Authoring 분리는 독립 편집/재사용 가치가 있을 때만 한다. Runtime 분리는 독립 state/tint/visibility가 필요할 때만 한다.

따라서 이번 두 Local Frame의 corner/accent/rail은 별도 Runtime Image로 나누지 않는다. 둘 다 각 1장의 static runtime texture 후보이며, 기존 dynamic RPM/Armor/Defense 시스템은 별도 기존 owner를 유지한다.

## 금지

- temporary safe-area box / layout debug rectangle
- central temporary seam
- 전체 VehiclePanel flat bitmap
- runtime 숫자/텍스트 bake
- 기존 RPM/Armor/Silhouette/Defense 재제작
- USER Review 전 UE Import / Production Asset mutation

## 다음 Gate

1. `VT07_SpeedFrame.json` / `VT07_ArmorFrame.json` → deterministic 422×272 PNG derivative 생성 PASS.
2. 두 Local Frame Project Image readback PASS.
3. `VT06_FrameMaster_v5` + VT07 Local Frame 2종 + current RPM MaskPack + VT04 modular Armor + VT05 Sedan silhouette + VT06 Defense Source를 조립한 `VT07_VehPanelReview.png` 896×416 생성 PASS.
4. 현재 Gate는 USER Visual Review다.
5. USER 승인 뒤에만 UE Import/Designer assembly를 수행한다.
