# How to Programme a Discharge Curve in AXP2101 for Vapecell N40 18650 Battery

To program a discharge curve for a Vapecell N40 18650 battery in an AXP2101 power management IC, you'll need to configure several parameters based on the battery's characteristics. Here's how to approach this:

## Understanding the Battery Specifications

The Vapecell N40 is a high-drain 18650 lithium-ion battery with these typical specs:
- Nominal voltage: 3.7V
- Capacity: ~4000mAh
- Discharge voltage range: 4.2V (full) to 2.9V (cutoff)
- Chemistry: INR (Lithium Nickel Manganese)

## AXP2101 Configuration Steps

### 1. Battery Voltage Thresholds
Configure the voltage monitoring registers:
```c
// Battery voltage warning levels (in mV)
#define BATT_WARNING_LEVEL1    3400    // ~20% capacity
#define BATT_WARNING_LEVEL2    3200    // ~5% capacity  
#define BATT_SHUTDOWN_LEVEL    2900    // Emergency shutdown

// Configure via I2C registers
axp2101_write_reg(AXP2101_VBAT_H_TH, (BATT_WARNING_LEVEL1 >> 8) & 0xFF);
axp2101_write_reg(AXP2101_VBAT_L_TH, BATT_WARNING_LEVEL1 & 0xFF);
```

### 2. Coulomb Counter Setup
For accurate capacity tracking:
```c
// Set battery capacity (4000mAh for N40)
#define BATTERY_CAPACITY_MAH   4000

// Configure coulomb counter
axp2101_write_reg(AXP2101_COULOMB_CTL, 0x80);  // Enable coulomb counter
axp2101_write_reg(AXP2101_BAT_CAP_H, (BATTERY_CAPACITY_MAH >> 8) & 0xFF);
axp2101_write_reg(AXP2101_BAT_CAP_L, BATTERY_CAPACITY_MAH & 0xFF);
```

### 3. Discharge Curve Parameters
Create a lookup table for voltage-to-capacity mapping:
```c
// Voltage (mV) to capacity (%) mapping for Vapecell N40
// This curve uses the same gradient as NCR18650B for consistency
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
Configure temperature-based adjustments:
```c
// Temperature compensation coefficients
#define TEMP_COEFF_COLD    0.95f   // Capacity at 0°C
#define TEMP_COEFF_HOT     1.05f   // Capacity at 40°C

uint8_t get_temperature_compensated_capacity(uint8_t raw_capacity, int16_t temp_c) {
    float compensation = 1.0f;
    
    if (temp_c < 10) {
        compensation = TEMP_COEFF_COLD;
    } else if (temp_c > 35) {
        compensation = TEMP_COEFF_HOT;
    }
    
    return (uint8_t)(raw_capacity * compensation);
}
```

### 5. Implementation Function
```c
void configure_axp2101_for_vapecell_n40(void) {
    // Enable battery detection
    axp2101_write_reg(AXP2101_COMM_CFG, 0x02);
    
    // Set charging parameters for 18650
    axp2101_write_reg(AXP2101_CHG_CFG, 0x04);  // 4.2V charge voltage
    axp2101_write_reg(AXP2101_CHG_CUR, 0x08);  // ~2A charge current
    
    // Configure power path
    axp2101_write_reg(AXP2101_PWR_PATH_CFG, 0x01);
    
    // Enable fuel gauge
    axp2101_write_reg(AXP2101_FUEL_GAUGE_CTL, 0x01);
    
    // Set initial capacity lookup table
    for (int i = 0; i < sizeof(discharge_curve)/sizeof(discharge_curve[0]); i++) {
        // Store curve data in AXP2101's internal memory
        axp2101_write_curve_data(i, discharge_curve[i][0], discharge_curve[i][1]);
    }
}
```

## Important Considerations

1. **Calibration**: The discharge curve should be calibrated through actual testing with your specific load conditions, as the curve varies significantly with discharge current.

2. **Safety**: Always implement proper under-voltage protection at 2.9V to prevent battery damage.

3. **Temperature Monitoring**: High-drain batteries like the N40 can get warm during discharge, affecting capacity calculations.

4. **Load Compensation**: The AXP2101 should account for the actual load current when estimating remaining capacity.

The exact register addresses and bit fields will depend on your AXP2101 datasheet version, so verify these against your specific IC documentation.