# ROBOT SYSTEM CONFIGURATION - PIPPO

## SYSTEM_TYPE
omnidirectional_3wheel: 2x_front_motors + 1x_rear_omni_servo

## MOTORS

### MSX (left_front)
- type: servo_motor
- function: front_left_propulsion
- deadzone_center: ZERO
- inverted: INV_flag
- control_mode: velocity_mapping
- velocity_range: [-1023, 1023]
- output: vel_sx

### MDX (right_front)
- type: servo_motor
- function: front_right_propulsion
- deadzone_center: ZERO
- inverted: INV_flag
- control_mode: velocity_mapping
- velocity_range: [-1023, 1023]
- output: vel_dx

### MPO (rear_omni)
- type: servo_motor
- function: rear_omnidirectional_steering
- deadzone_center: ZERO
- inverted: INV_flag
- control_mode: angle_mapping
- angle_range: [-1750, 1750]
- output: mpo_vel

## FUNCTION_MUOVI

### Function_signature
```cpp
void Muovi(short vel, short ang)
```

### Parameters
- vel: velocity command [-1023, 1023]
  - positive: forward motion
  - negative: reverse motion
  - controls: MSX, MDX equally
- ang: steering angle [-1750, 1750]
  - negative: rotate rear wheel left
  - zero: rear wheel center
  - positive: rotate rear wheel right
  - controls: MPO only

### Control_logic
```
INPUT: vel, ang
  ↓
1. vel constrain: [-1023, 1023]
2. ang constrain: [-1750, 1750]
3. IF vel >= 0:
     vel_sx = map(vel, 0, 1023, ZEROMIN, MAX)
     vel_dx = map(vel, 0, 1023, ZEROMIN, MAX)
   ELSE:
     vel_sx = map(-vel, 0, 1023, ZEROMAX, MIN)
     vel_dx = map(-vel, 0, 1023, ZEROMAX, MIN)
4. Apply_inversion_MSX: if(MSX_INV) vel_sx = ZERO - (vel_sx - ZERO)
5. Apply_inversion_MDX: if(MDX_INV) vel_dx = ZERO - (vel_dx - ZERO)
6. mpo_vel = map(ang, -1750, 1750, MIN, MAX)
7. Apply_inversion_MPO: if(MPO_INV) mpo_vel = ZERO - (mpo_vel - ZERO)
8. OUTPUT: writeMicroseconds(msx_vel, mdx_vel, mpo_vel)
```

### Motor_behavior
- MIN → max_reverse
- ZERO → stop
- MAX → max_forward
- ZEROMIN/ZEROMAX → deadzone boundaries

### Velocity_mapping
- forward (vel ≥ 0): 0→1023 maps to ZEROMIN→MAX
- reverse (vel < 0): 0→1023 maps to ZEROMAX→MIN

### Angle_mapping
- -1750 → MIN
- 0 → ZERO
- 1750 → MAX

## INVERSION_FORMULA
If motor inverted: output = ZERO - (value - ZERO)
Effect: reflects value around center point, reversing direction

## BUG_ALERT
Current implementation applies steering to MSX/MDX motors (if/else block with ang mapping).
INCORRECT for omnidirectional system.
ISSUE: Steering should only control MPO, velocity should be equal on both front motors.
FIX: Remove ang steering logic from vel_sx/vel_dx calculation.
