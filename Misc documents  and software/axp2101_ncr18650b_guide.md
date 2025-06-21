# How to Programme a Discharge Curve in AXP2101 for NCR18650B 18650 Battery

To program a discharge curve for a NCR18650B 18650 battery in an AXP2101 power management IC, you'll need to configure several parameters based on the battery's characteristics. Here's how to approach this:

## Understanding the Battery Specifications

The NCR18650B is a high-capacity 18650 lithium-ion battery with these typical specs:
- Nominal voltage: 3.7V
- Capacity: 3200mAh
- Discharge voltage range: 4.2V (full) to 2.9V (cutoff)
- Chemistry: NCR (Lithium Nickel Cobalt)
- Maximum continuous discharge: 4.875A (1.5C)
- Internal resistance: ~45mΩ

## AXP2101 Configuration Steps

### 1. Battery Voltage Thresholds
Configure the voltage monitoring registers:
```c
// Battery voltage warning levels (in mV)
#define BATT_WARNING_LEVEL1    3500    // ~20% capacity
#define BATT_WARNING_LEVEL2    3300    // ~5% capacity  
#define BATT_SHUTDOWN_LEVEL    2900    // Emergency shutdown

// Configure via I2C registers
axp2101_write_reg(AXP2101_VBAT_H_TH, (BATT_WARNING_LEVEL1 >> 8) & 0xFF);
axp2101_write_reg(AXP2101_VBAT_L_TH, BATT_WARNING_LEVEL1 & 0xFF);
```

### 2. Coulomb Counter Setup
For accurate capacity tracking:
```c
// Set battery capacity (3200mAh for NCR18650B)
#define BATTERY_CAPACITY_MAH   3200

// Configure coulomb counter
axp2101_write_reg(AXP2101_COULOMB_CTL, 0x80);  // Enable coulomb counter
axp2101_write_reg(AXP2101_BAT_CAP_H, (BATTERY_CAPACITY_MAH >> 8) & 0xFF);
axp2101_write_reg(AXP2101_BAT_CAP_L, BATTERY_CAPACITY_MAH & 0xFF);
```

### 3. Discharge Curve Parameters
Create a lookup table for voltage-to-capacity mapping:
```c
// Voltage (mV) to capacity (%) mapping for NCR18650B
// This curve is optimized for moderate discharge rates (0.2C to 1C)
const uint16_t discharge_curve[][2] = {
    {4200, 100},  // 4.2V = 100%
    {4150, 98},   // 4.15V = 98%
    {4100, 95},   // 4.1V = 95%
    {4050, 90},   // 4.05V = 90%
    {4000, 85},   // 4.0V = 85%
    {3950, 80},   // 3.95V = 80%
    {3900, 75},   // 3.9V = 75%
    {3850, 70},   // 3.85V = 70%
    {3800, 65},   // 3.8V = 65%
    {3750, 55},   // 3.75V = 55%
    {3700, 45},   // 3.7V = 45%
    {3650, 35},   // 3.65V = 35%
    {3600, 25},   // 3.6V = 25%
    {3550, 20},   // 3.55V = 20%
    {3500, 15},   // 3.5V = 15%
    {3450, 12},   // 3.45V = 12%
    {3400, 9},    // 3.4V = 9%
    {3350, 6},    // 3.35V = 6%
    {3300, 4},    // 3.3V = 4%
    {3200, 2},    // 3.2V = 2%
    {3100, 1},    // 3.1V = 1%
    {3000, 0.5},  // 3.0V = 0.5%
    {2900, 0}     // 2.9V = 0% (cutoff)
};
```

### 4. Temperature Compensation
Configure temperature-based adjustments for NCR chemistry:
```c
// Temperature compensation coefficients for NCR18650B
#define TEMP_COEFF_VERY_COLD  0.80f   // Capacity at -10°C
#define TEMP_COEFF_COLD       0.90f   // Capacity at 0°C
#define TEMP_COEFF_NORMAL     1.00f   // Capacity at 25°C
#define TEMP_COEFF_WARM       1.02f   // Capacity at 40°C
#define TEMP_COEFF_HOT        0.98f   // Capacity at 60°C

uint8_t get_temperature_compensated_capacity(uint8_t raw_capacity, int16_t temp_c) {
    float compensation = TEMP_COEFF_NORMAL;
    
    if (temp_c < -5) {
        compensation = TEMP_COEFF_VERY_COLD;
    } else if (temp_c < 5) {
        compensation = TEMP_COEFF_COLD;
    } else if (temp_c > 50) {
        compensation = TEMP_COEFF_HOT;
    } else if (temp_c > 35) {
        compensation = TEMP_COEFF_WARM;
    }
    
    return (uint8_t)(raw_capacity * compensation);
}
```

### 5. Load Compensation for NCR18650B
```c
// NCR18650B has higher internal resistance, so load compensation is important
float get_load_compensated_voltage(uint16_t measured_voltage_mv, uint16_t current_ma) {
    // Internal resistance ~45mΩ
    const float internal_resistance_ohm = 0.045f;
    
    // Calculate voltage drop due to internal resistance
    float voltage_drop_mv = (current_ma / 1000.0f) * internal_resistance_ohm * 1000.0f;
    
    // Return compensated voltage (what voltage would be under no load)
    return measured_voltage_mv + voltage_drop_mv;
}
```

### 6. Implementation Function
```c
void configure_axp2101_for_ncr18650b(void) {
    // Enable battery detection
    axp2101_write_reg(AXP2101_COMM_CFG, 0x02);
    
    // Set charging parameters for NCR18650B
    axp2101_write_reg(AXP2101_CHG_CFG, 0x04);  // 4.2V charge voltage
    axp2101_write_reg(AXP2101_CHG_CUR, 0x06);  // ~1.6A charge current (0.5C)
    
    // Configure power path
    axp2101_write_reg(AXP2101_PWR_PATH_CFG, 0x01);
    
    // Enable fuel gauge
    axp2101_write_reg(AXP2101_FUEL_GAUGE_CTL, 0x01);
    
    // Configure discharge current limit (NCR18650B max 4.875A)
    axp2101_write_reg(AXP2101_DISCHARGE_LIM, 0x4C);  // ~4.8A limit
    
    // Set initial capacity lookup table
    for (int i = 0; i < sizeof(discharge_curve)/sizeof(discharge_curve[0]); i++) {
        // Store curve data in AXP2101's internal memory
        axp2101_write_curve_data(i, discharge_curve[i][0], discharge_curve[i][1]);
    }
    
    // Enable load compensation
    axp2101_write_reg(AXP2101_LOAD_COMP_EN, 0x01);
}
```

### 7. State of Charge Calculation Function
```c
uint8_t calculate_soc_ncr18650b(uint16_t voltage_mv, uint16_t current_ma, int16_t temp_c) {
    // Apply load compensation
    float compensated_voltage = get_load_compensated_voltage(voltage_mv, current_ma);
    
    // Find capacity from lookup table
    uint8_t capacity_percent = 0;
    
    for (int i = 0; i < sizeof(discharge_curve)/sizeof(discharge_curve[0]) - 1; i++) {
        if (compensated_voltage >= discharge_curve[i+1][0] && 
            compensated_voltage <= discharge_curve[i][0]) {
            
            // Linear interpolation between points
            float voltage_range = discharge_curve[i][0] - discharge_curve[i+1][0];
            float capacity_range = discharge_curve[i][1] - discharge_curve[i+1][1];
            float voltage_offset = compensated_voltage - discharge_curve[i+1][0];
            
            capacity_percent = discharge_curve[i+1][1] + 
                              (uint8_t)((voltage_offset / voltage_range) * capacity_range);
            break;
        }
    }
    
    // Apply temperature compensation
    return get_temperature_compensated_capacity(capacity_percent, temp_c);
}
```

## Important Considerations for NCR18650B

1. **Moderate Discharge Rate**: The NCR18650B is optimized for capacity rather than high discharge rates. Keep discharge current under 3A for best performance.

2. **Temperature Sensitivity**: NCR chemistry is more temperature-sensitive than other lithium-ion types. Monitor temperature closely.

3. **Voltage Plateau**: The NCR18650B has a more pronounced voltage plateau in the middle range (3.7V-3.9V), which can make capacity estimation challenging.

4. **Internal Resistance**: Higher internal resistance means load compensation is crucial for accurate readings.

5. **Aging Characteristics**: NCR batteries tend to lose capacity gradually with age, so periodic recalibration may be needed.

6. **Safety**: Always implement proper under-voltage protection at 2.9V to prevent battery damage.

7. **Charging Profile**: Use CC/CV charging with 4.2V max voltage and recommended charge current of 0.5C (1.6A) for optimal lifespan.

The exact register addresses and bit fields will depend on your AXP2101 datasheet version, so verify these against your specific IC documentation.