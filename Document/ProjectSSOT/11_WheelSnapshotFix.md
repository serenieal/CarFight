# Wheel Visual Sync Hotfix (Snapshot API)

## Symptoms
- Enhanced Input works, but wheel/anchor/mesh values (LocZ/Yaw/Pitch) are stuck at 0.
- Suspected cause: custom GetRealWheelTransform (reflection) fails to access UE/ChaosVehicle internals and returns defaults.

## Recommended Fix
- Use the official ChaosVehicle Snapshot API instead of custom reflection.
- MovementComponent: UChaosWheeledVehicleMovementComponent
- Nodes: GetSnapshot -> BreakWheeledSnapshot -> WheelSnapshots[i] -> BreakWheelSnapshot

---

## Apply to BP_ModularVehicle

### 0) Add variables (with tooltips)
1) ChaosWheeledMovement (Chaos Wheeled Vehicle Movement Component Object Reference)
   - Tooltip: "MovementComponent reference for reading wheel snapshot (GetSnapshot)."

2) SuspensionOffsetSign (float, default -1.0)
   - Tooltip: "Sign for SuspensionOffset (flip +1/-1 if Z moves opposite)."

3) WheelSpinSign (float, default -1.0)
   - Tooltip: "Sign for WheelRotationAngle (flip +1/-1 if spin direction is reversed)."

4) RightWheelYawDeg (float, default 180.0)
   - Tooltip: "Fixed yaw for right wheels (Anchor yaw stays 0, Mesh yaw uses 180)."

5) bDebugWheelSnapshot (bool, default true)
   - Tooltip: "Print snapshot values to quickly verify they are not 0 in PIE."

6) BaseAnchorRelLoc_FL/FR/RL/RR (Vector x4)
   - Tooltip: "Wheel_Anchor relative locations at BeginPlay (baseline)."

---

### 1) BeginPlay setup
1) GetVehicleMovementComponent -> Cast to ChaosWheeledVehicleMovementComponent
2) If success, store to ChaosWheeledMovement
3) For Wheel_Anchor_FL/FR/RL/RR: GetRelativeLocation -> store to BaseAnchorRelLoc_*
4) (Debug) Print ChaosWheeledMovement.GetNumWheels (enable Print to Log recommended)

---

### 2) Tick: call UpdateWheelVisuals_FromSnapshot
- Event Tick -> UpdateWheelVisuals_FromSnapshot(DeltaSeconds)

#### Function: UpdateWheelVisuals_FromSnapshot(DeltaSeconds)
1) If not IsValid(ChaosWheeledMovement): return
2) ChaosWheeledMovement.GetSnapshot
3) BreakWheeledSnapshot (WheelSnapshots array + EngineRPM)
4) For WheelIndex 0..3
   - If WheelSnapshots.Length <= WheelIndex: continue
   - WheelSnapshot = WheelSnapshots[WheelIndex]
   - BreakWheelSnapshot -> SuspensionOffset, WheelRotationAngle, SteeringAngle, WheelRadius, WheelAngularVelocity

5) Anchor (example FL)
   - NewLoc = BaseAnchorRelLoc_FL
   - NewLoc.Z += SuspensionOffsetSign * SuspensionOffset
   - Wheel_Anchor_FL.SetRelativeLocation(NewLoc)
   - Wheel_Anchor_FL.SetRelativeRotation( Rotator(0, SteeringAngle, 0) )

6) Mesh (example FL)
   - MeshRot = Rotator(WheelSpinSign * WheelRotationAngle, 0, 0)
   - Wheel_Mesh_FL.SetRelativeRotation(MeshRot)

7) Right wheel rule (FR/RR)
   - Anchor applies SteeringAngle only (mirror yaw handled on Mesh, not Anchor)
   - Wheel_Mesh_FR/RR yaw = RightWheelYawDeg fixed
     - Example: Rotator(Pitch, RightWheelYawDeg, 0)

8) Debug print
   - If bDebugWheelSnapshot is true, print WheelIndex 0 values:
     SuspensionOffset / SteeringAngle / WheelRotationAngle / WheelAngularVelocity / EngineRPM
     (enable Print to Log)

---

## Pass/Fail
- If EngineRPM changes and any of WheelAngularVelocity/SteeringAngle/SuspensionOffset changes, snapshot access is working.
- If all still 0:
  1) ChaosWheeledMovement valid (cast success / not null)
  2) GetNumWheels == 4 (0 indicates setup or component type issue)
  3) SkeletalMesh Simulate Physics ON; MechanicalSim/Suspension/WheelFriction enabled
