# P0 Implementation Plan

## Scope
- P0-001 Enhanced Input: assets not under Developers, mapping context injection validated
- P0-002 Wheel visual sync: Wheel_Anchor + Wheel_Mesh follow Chaos wheel state

## Locked Facts
- Wheel order: 0 FL, 1 FR, 2 RL, 3 RR
- Components: Wheel_Anchor_* parent of Wheel_Mesh_*
- Right wheels: keep Anchor baseline rot 0; Mesh has Yaw 180 constant
- Utility: GetRealWheelTransform(VehicleMovement, WheelIndex) returns relative transform

## P0-001 Tasks (Editor)
1. Move IMC/IA from /Developers/chaeksong/Input to /Game/CarFight/Input
2. Fix up redirectors, Save All
3. Ensure mapping context is added on local possession
   - LocalPlayer Subsystem -> Add Mapping Context (IMC_Vehicle_Default, Priority 0)
4. Ensure IA events drive VehicleMovement input (Throttle, Steer, Brake, Handbrake)

## P0-002 Tasks (BP_ModularVehicle)
1. Create refs: Wheel_Anchor_FL/FR/RL/RR and Wheel_Mesh_FL/FR/RL/RR
2. Add bool: bEnableWheelVisualSync (default true)
3. Event Tick (if enabled): for each wheel index
   - T = GetRealWheelTransform(Movement, Index)
   - Anchor: SetRelativeLocation(T.Location)
   - Anchor: SetRelativeRotation(YawOnlyFrom(T.Rotation))
   - Mesh: SetRelativeRotation(PitchFrom(T.Rotation), RightYawOffset(0 or 180))
4. Optional debug: bDebugWheelSync prints Pitch/Yaw per wheel

## DoD
- Steering turns front wheel visuals; throttle spins wheels; suspension moves anchors
- No right-wheel mirror twist (Anchor stays clean; Mesh handles 180 yaw)
- Input works after asset migration out of Developers
