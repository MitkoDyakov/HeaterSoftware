# Heater Current Monitoring System

## Overview
Added comprehensive current monitoring functionality to the heater test system to prevent USB-PD shutdown due to excessive current draw when both heaters are turned on simultaneously.

## New Functions Added

### 1. `float read_ap33772s_current(void)`
- Reads current consumption from the AP33772S USB-PD controller
- Returns current in milliamps (mA)
- Returns -1.0f on error
- Uses I2C communication with CMD_CURRENT register (0x12)

### 2. `bool check_current_safety(float current_before, float current_after, float max_allowed_increase)`
- Compares current before and after turning on heaters
- Prints current readings and increase
- Returns false if increase exceeds safe limits
- Provides safety validation for heater operation

### 3. New Console Commands

#### `readcurrent`
- Standalone command to read current USB-PD current consumption
- Displays current in both mA and A
- Useful for monitoring system power usage

#### `dualheater`
- Comprehensive test for both heaters simultaneously
- Sequential heater activation with current monitoring
- Safety checks at each step
- Automatic emergency shutdown if current limits exceeded
- Detailed test summary with current measurements

## Enhanced Heater Test (`heatertest` command)

### Safety Features Added:
1. **Pre-activation Check**: Reads baseline current before turning on heaters
2. **Post-activation Check**: Verifies current increase is within safe limits
3. **Continuous Monitoring**: Displays current consumption during each measurement cycle
4. **Emergency Shutdown**: Automatically stops test if current exceeds safe limits
5. **Final Readings**: Shows current before and after heater shutdown

### Current Safety Limits:
- Single heater: 2000mA (2A) increase limit
- Dual heaters: 4500mA (4.5A) total increase limit
- Runtime monitoring: +500mA tolerance during operation
- Emergency threshold: +1000mA spike detection

## Usage Examples

### Check Current Consumption
```
readcurrent
```
Output:
```
Reading current from AP33772S...
Current consumption: 450.0 mA (0.450 A)
```

### Test Single Heater with Current Monitoring
```
heatertest -c 1 -t 5
```
Output includes current readings:
```
Reading initial current...
Current before: 450.0 mA
Current after: 2150.0 mA
Current increase: 1700.0 mA
Current increase within safe limits.
ch1: 1234 - 0.062000 V - Rntc 5678.0 R - 85.23 °C - Current: 2145.0 mA
...
Final current before shutdown: 2140.0 mA
Current after shutdown: 455.0 mA
```

### Test Both Heaters Simultaneously
```
dualheater
```
Provides comprehensive testing with step-by-step current monitoring and safety validation.

## Safety Benefits

1. **Prevents USB-PD Shutdown**: Monitors current to avoid exceeding USB-PD capabilities
2. **Progressive Testing**: Tests single heater first, then both together
3. **Real-time Monitoring**: Continuous current tracking during operation
4. **Automatic Protection**: Emergency shutdown if current spikes occur
5. **Detailed Reporting**: Complete current consumption analysis

## Technical Details

### I2C Communication
- Uses existing `PD_handle` from pdsetup module
- CMD_CURRENT register (0x12) provides 16-bit current reading
- 50ms timeout for I2C operations
- Error handling for communication failures

### Integration Points
- Leverages existing I2C bus infrastructure
- Uses existing GPIO control for heaters
- Maintains compatibility with original heatertest functionality
- Adds minimal overhead to existing operations

## Error Handling

1. **I2C Communication Errors**: Detected and reported
2. **Invalid Current Readings**: Negative values indicate errors
3. **Safety Threshold Violations**: Automatic heater shutdown
4. **Progressive Failure Detection**: Multiple safety checkpoints

This implementation provides robust current monitoring to prevent USB-PD overcurrent situations while maintaining full heater testing capability.