#pragma once

#include <cstdint>

struct BatteryMapping {
    static float voltage_to_percentage(uint16_t voltage_mv) {
        // Voltage to percentage mapping table
        static const struct {
            uint16_t voltage;
            float percentage;
        } voltage_table[] = {
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
        static const size_t table_size = sizeof(voltage_table) / sizeof(voltage_table[0]);

        // Handle edge cases
        if (voltage_mv >= voltage_table[0].voltage) return voltage_table[0].percentage;
        if (voltage_mv <= voltage_table[table_size-1].voltage) return voltage_table[table_size-1].percentage;

        // Find the two closest voltage points
        for (size_t i = 0; i < table_size - 1; i++) {
            if (voltage_mv <= voltage_table[i].voltage && voltage_mv > voltage_table[i+1].voltage) {
                // Linear interpolation between the two points
                float voltage_diff = voltage_table[i].voltage - voltage_table[i+1].voltage;
                float percentage_diff = voltage_table[i].percentage - voltage_table[i+1].percentage;
                float ratio = (voltage_table[i].voltage - voltage_mv) / voltage_diff;
                return voltage_table[i].percentage - (percentage_diff * ratio);
            }
        }

        return 0; // Should never reach here
    }
}; 