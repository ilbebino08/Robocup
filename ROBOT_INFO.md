# ROBOT SYSTEM CONFIGURATION - ROBOCUP 2026

**LAST_UPDATE**: 2026-02-02  
**COMMIT_HASH**: 9565883  
**PROJECT_TYPE**: RoboCup Junior Rescue Line - Arduino Mega 2560  
**REPOSITORY**: github.com/ilbebino08/Robocup (PUBLIC)

---

## PROJECT_METADATA

```yaml
platform: atmelavr
board: megaatmega2560
framework: arduino
monitor_baud: 115200
bluetooth_baud: 57600
language: C++11
build_system: PlatformIO
```

---

## SYSTEM_ARCHITECTURE

### Locomotion System
```
TYPE: omnidirectional_3wheel
CONFIGURATION: 2x_front_motors + 1x_rear_omni_servo
CONTROL: differential_drive_with_rear_steering
```

### Sensor Array
```
LINE_TRACKING: 8x digital_IR_sensors (array)
COLOR_DETECTION: 2x AS7341_spectral_sensors (left + right)
DISTANCE: 6x VL53L0X_TOF_laser_rangefinders
ACCELEROMETER: ADXL345 (optional, library present)
```

### Actuators
```
MAIN_DRIVE: 3x continuous_rotation_servos
MANIPULATOR: 2x standard_servos (shoulder + gripper)
FEEDBACK: 3x LED (red, green, yellow)
```

### Communication
```
PRIMARY: USB Serial @ 115200 baud (Serial)
BLUETOOTH: HC-05/HC-06 @ 57600 baud (Serial1)
SD_CARD: Optional logging (SD library present)
DEBUG_PORT: Serial3 @ 9600 baud
```

---

## MOTOR_SYSTEM

### MSX (Motor_Left_Front)
```cpp
PIN: 30
TYPE: continuous_rotation_servo
FUNCTION: left_front_propulsion
PWM_RANGE: [500, 2000] µs
DEADZONE: [1450, 1550] µs
CENTER: 1500 µs
INVERTED: false
CONTROL_MODE: velocity_command
```

### MDX (Motor_Right_Front)
```cpp
PIN: 29
TYPE: continuous_rotation_servo
FUNCTION: right_front_propulsion
PWM_RANGE: [500, 2000] µs
DEADZONE: [1450, 1550] µs
CENTER: 1500 µs
INVERTED: true
CONTROL_MODE: velocity_command
```

### MPO (Motor_Rear_Omni)
```cpp
PIN: 27
TYPE: continuous_rotation_servo
FUNCTION: rear_omnidirectional_steering
PWM_RANGE: [500, 2000] µs
DEADZONE: [1450, 1550] µs
CENTER: 1500 µs
INVERTED: true
CONTROL_MODE: angle_steering
```

### Motor Control Interface

```cpp
// Primary control function in motori.cpp
void Motori::Muovi(short vel, short ang);

PARAMETERS:
  - vel: velocity [-1023, +1023]
    * positive: forward
    * negative: reverse
    * zero: stop
    * affects: MSX, MDX equally
  
  - ang: steering_angle [-1750, +1750]
    * negative: rear_wheel_left
    * zero: rear_wheel_center
    * positive: rear_wheel_right
    * affects: MPO only

MAPPING_LOGIC:
  1. Constrain inputs: vel ∈ [-1023, 1023], ang ∈ [-1750, 1750]
  2. Forward (vel ≥ 0):
     - vel_sx = map(vel, 0, 1023, ZEROMIN, MAX)
     - vel_dx = map(vel, 0, 1023, ZEROMIN, MAX)
  3. Reverse (vel < 0):
     - vel_sx = map(abs(vel), 0, 1023, ZEROMAX, MIN)
     - vel_dx = map(abs(vel), 0, 1023, ZEROMAX, MIN)
  4. Apply inversions if INV flags set
  5. Steering: mpo_vel = map(ang, -1750, 1750, MIN, MAX)
  6. Output: writeMicroseconds() to all 3 servos
```

---

## LINE_FOLLOWING_SYSTEM

### PID Controller (followLine library)

```cpp
FILE: lib/followLine/followLine.cpp

FUNCTION: float calculatePID(int error);

PID_PARAMETERS:
  - Kp: proportional_gain (immediate response to error)
  - Ki: integral_gain (accumulated error correction)
  - Kd: derivative_gain (rate of change dampening)

ERROR_CALCULATION:
  - Source: weighted_average_of_IR_sensors (in sensorBoard)
  - Range: typically [-4, +4] (8 sensors centered)
  - Negative: line to the left
  - Positive: line to the right
  - Zero: line centered

OUTPUT:
  - correction_value: applied to motor velocities
  - base_speed ± correction → differential drive
```

### State Machine (lineLogic library)

```cpp
FILE: lib/lineLogic/lineLogic.cpp

STATES:
  - LINEA: normal line following
  - NO_LINEA: gap/interruption detected
  - VERDE_SX: green marker left (90° left turn)
  - VERDE_DX: green marker right (90° right turn)
  - DOPPIO_VERDE: double green (stop/end of path)
  - RADDRIZZAMENTO: straightening at intersection (recent fix)

STATE_TRANSITIONS:
  LINEA → NO_LINEA: all IR sensors read white for > timeout
  LINEA → VERDE_SX: left AS7341 detects green
  LINEA → VERDE_DX: right AS7341 detects green (with smart verification)
  LINEA → DOPPIO_VERDE: both AS7341 detect green simultaneously
  VERDE_* → RADDRIZZAMENTO → LINEA: after turn completion

KEY_FUNCTIONS:
  - void initLineLogic(): initialize state machine
  - void gestisciLinea(int stato): main state handler (call each loop)
  - int statoLinea(): read sensors and return current state
```

### Recent Bug Fix (commit 9565883)

```
ISSUE: Robot deviating at green intersections after turning
FIX: Added RADDRIZZAMENTO state
BEHAVIOR: Robot straightens before resuming line following
RESULT: More accurate path following at intersections
```

---

## SENSOR_SYSTEM

### IR Sensor Array (BottomSensor class)

```cpp
FILE: lib/sensorBoard/sensorBoard.cpp

HARDWARE: External slave board (Serial controlled)
ARCHITECTURE: Dedicated microcontroller managing 8 IR sensors
COMMUNICATION: Serial UART @ 57600 baud (Serial1)
CONTROL: Fully managed by sensorBoard library

ARRAY_SIZE: 8 digital IR sensors (9 total with front sensor)
FUNCTION: Line position detection and tracking
OUTPUT: Line position value (weighted average for PID)

POSITION_RANGE: [-1750, +1750]
  - Negative values: line to the left
  - Zero: line centered
  - Positive values: line to the right
  - LINE_LOST_BEHAVIOR: When line is lost, value remains at minimum or maximum
    based on last known position (left = -1750, right = +1750)

KEY_METHODS:
  - void start(): initialize Serial1 communication with slave board
  - int16_t line(): returns current line position [-1750, +1750]
  - bool checkLinea(): returns true if line is detected in front
  - bool checkColor(): returns true if line color matches setCheckColor() mode
  - void setCheckColor(mode): set color detection mode (1=silver, 2=black, 3=red)
  - uint8_t getLineMode(): get current color detection mode

UTILITY_METHODS (utils subclass):
  - bool calibration(): calibrate IR sensors, returns success status
  - uint16_t* calibration_val(): get calibration values array
  - uint16_t* val_sensor(): get raw sensor readings (all sensors)
  - uint16_t val_sensor(n): get raw reading from sensor n
  - uint16_t* val_sensorCal(): get calibrated sensor readings (all sensors)
  - uint16_t val_sensorCal(n): get calibrated reading from sensor n
  - void greenSxCalibration(): calibrate left green sensor
  - void greenDxCalibration(): calibrate right green sensor

ABSTRACTION: Main Arduino Mega sends serial commands via Serial1, slave board handles:
  - IR sensor reading
  - Line position calculation (weighted average)
  - Color detection (silver/black/red line modes)
  - Green marker detection (left/right sensors)
  - Data transmission back to master via status byte
```

### Color Sensors (Integrated in BottomSensor)

```cpp
HARDWARE: 2x color sensors on slave board (left + right)
LOCATION: Left and right sides of IR array
COMMUNICATION: Managed via same Serial1 connection as IR sensors
FUNCTION: Green and colored line detection

COLOR_DETECTION_MODES:
  - Mode 1: Silver line detection
  - Mode 2: Black line detection (default)
  - Mode 3: Red line detection

GREEN_DETECTION:
  - checkGreenSx(): returns true if left sensor detects green marker
  - checkGreenDx(): returns true if right sensor detects green marker
  - Independent green detection runs parallel to line color mode

STATUS_BYTE_FORMAT (bits 7-4):
  Bit 7: Verde DX (right green detected)
  Bit 6: Verde SX (left green detected)
  Bit 5: Linea frontale (front line detected)
  Bit 4: Linea colorata (colored line match)
  Bits 3-2: Color mode (1=silver, 2=black, 3=red)

USAGE_PATTERN:
  setCheckColor(2);           // Set to black line mode
  if (checkGreenSx()) {...}   // Check for left green marker
  if (checkGreenDx()) {...}   // Check for right green marker
  if (checkColor()) {...}     // Check if current line matches selected color
```

### Distance Sensors (VL53L0X)

```cpp
CHIP: STMicroelectronics VL53L0X Time-of-Flight
COUNT: 6 sensors
LIBRARY: Adafruit_VL53L0X@^1.2.4
RANGE: ~2000mm max
FUNCTION: Obstacle detection, victim identification (competition specific)

NOTE: Implementation may vary based on competition requirements
```

---

## MANIPULATOR_SYSTEM

### Arm & Gripper (braccio library)

```cpp
FILE: lib/braccio/braccio.cpp

SERVOS:
  1. BRACCIO (shoulder/arm):
     - PIN: 39
     - RANGE: [570, 1500] µs
     - RIP (rest): 570
     - AVANTI (forward): 1500
  
  2. PINZA (gripper):
     - PIN: 28
     - RANGE: [600, XXXX] µs
     - RIP (open): 600

KEY_METHODS:
  - void init(): initialize servos
  - void apri(): open gripper
  - void chiudi(): close gripper
  - void alza(): raise arm
  - void abbassa(): lower arm
```

---

## DEBUG_SYSTEM

### Multi-Channel Debug (debug library)

```cpp
FILE: lib/debug/debug.cpp

CHANNELS:
  - DEBUG_USB: Serial @ 115200
  - DEBUG_BLUETOOTH: Serial3 @ 9600
  - DEBUG_SD: SD card logging (optional)

USAGE:
  debug.begin(DEBUG_USB | DEBUG_BLUETOOTH);
  debug.println("message");
  debug.setBluetoothSerial(&Serial3);

ADVANTAGE: Single debug.print() outputs to multiple channels
```

---

## USER_INPUT

### MultiClickButton

```cpp
FILE: lib/MultiClickButton/src/MultiClickButton.cpp

PIN: 2 (with interrupt)
DEBOUNCE: Hardware + software
FEATURES: Single/double/triple click detection

CALLBACKS:
  - onSingleClick(): start/pause line following
  - onDoubleClick(): emergency stop
  - onTripleClick(): debug mode (sensor value printing)

IMPLEMENTATION:
  button.begin();
  button.onSingleClick(callback_function);
  button.update(); // call in loop()
```

---

## DEPENDENCIES

```ini
[PlatformIO lib_deps]
adafruit/Adafruit_VL53L0X@^1.2.4
huynhtancuong/ADXL345@^1.0.2
adafruit/Adafruit AS7341@^1.4.1
arduino-libraries/Servo@^1.3.0
arduino-libraries/SD@^1.3.0
```

---

## FILE_STRUCTURE_MAPPING

```
ENTRY_POINT: src/main.cpp
  ├─ setup(): Initialize all subsystems
  │   ├─ Serial communications (USB, Bluetooth)
  │   ├─ IR_board.start() → sensor array init
  │   ├─ motori.init() → motor servo attach
  │   ├─ braccio.init() → arm servo attach
  │   ├─ button.begin() → input setup
  │   └─ initLineLogic() → state machine reset
  │
  └─ loop(): Main control loop
      ├─ button.update() → check for user input
      ├─ stato = statoLinea() → read sensors
      └─ gestisciLinea(stato) → execute state logic
          ├─ LINEA → calculatePID() → Muovi()
          ├─ VERDE_SX → gestisciVerdeSinistra()
          ├─ VERDE_DX → gestisciVerdeDestra()
          └─ NO_LINEA → gestisciNoLinea()

CONFIGURATION: include/robot.h
  - All pin definitions
  - Motor calibration values (MIN, MAX, ZERO, INV)
  - Servo ranges for arm/gripper
  - LED pins

LIBRARIES (lib/):
  motori/        → Motor control abstraction
  sensorBoard/   → IR array + color sensors
  lineLogic/     → State machine logic
  followLine/    → PID implementation
  braccio/       → Arm/gripper control
  debug/         → Multi-channel logging
  MultiClickButton/ → Input handling
```

---

## KNOWN_ISSUES

### RESOLVED
```
✓ [2026-01-30] Robot deviation at green intersections
  FIX: Added RADDRIZZAMENTO state for straightening
  COMMIT: 9565883
```

### POTENTIAL_AREAS_FOR_IMPROVEMENT
```
- PID tuning: May need adjustment for different track conditions
- Green detection: Threshold calibration for varying lighting
- Gap detection: Timeout tuning for different gap sizes
- Battery voltage monitoring: Not implemented (can cause speed drift)
```

---

## AI_AGENT_GUIDELINES

When modifying this codebase:

1. **Motor Control**: Always use `motori.Muovi(vel, ang)` - never directly write to servo pins
2. **Sensor Reading**: Use `statoLinea()` for state, `IR_board.getLinePosition()` for PID error
3. **State Management**: All line logic goes through `gestisciLinea()` state machine
4. **Debug Output**: Use `debug.println()` instead of `Serial.print()` for multi-channel support
5. **Constants**: Modify hardware parameters in `include/robot.h`, NOT in library files
6. **PID Tuning**: Adjust Kp/Ki/Kd in `lib/followLine/followLine.h`
7. **State Transitions**: Add new states in `lineLogic.h` enum and handler in `lineLogic.cpp`
8. **Testing**: Always test motor inversions with `Muovi(500, 0)` to verify forward motion

### Critical Files for Common Tasks
```
Speed adjustment     → lib/lineLogic/lineLogic.cpp (base_speed variable)
Turn angles          → lib/lineLogic/lineLogic.cpp (rotation durations)
Sensor thresholds    → lib/sensorBoard/sensorBoard.cpp (color detection)
Pin changes          → include/robot.h (ALL pin definitions)
PID behavior         → lib/followLine/followLine.h + .cpp
New states           → lib/lineLogic/lineLogic.h (enum) + .cpp (handler)
```

---

## COMPETITION_CONTEXT

```
EVENT: RoboCup Junior 2026
CATEGORY: Rescue Line
TEAM: ilbebino08
ROBOT_NAME: PIPPO (legacy name in old configs)

TASKS:
  - Follow black line on white surface
  - Handle 90° turns marked with green
  - Navigate line gaps/interruptions
  - Detect and avoid obstacles (VL53L0X sensors)
  - Identify victims (not fully implemented)
  - Collect and deposit objects (evacuation zone)

TRACK_FEATURES:
  - Black line (~1-2cm width)
  - Green markers at intersections
  - Double green = end/decision point
  - Line gaps up to ~20cm
  - Possible slopes/inclines
  - Obstacles and victims
```

---

**END OF ROBOT_INFO.md**
