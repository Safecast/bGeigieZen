#pragma once

#include <cstdint>

struct BatteryMapping {
    static float voltage_to_percentage(uint16_t voltage_mv) {
        // Discharge curve table for Vapecell N40 (values from provided MD file)
        static const struct {
            uint16_t voltage_mv;
            float percentage;
        } curve[] = {
            {4200, 100}, {4150, 98}, {4100, 95}, {4050, 90}, {4000, 85},
            {3950, 80}, {3900, 75}, {3850, 70}, {3800, 65}, {3750, 55},
            {3700, 45}, {3650, 35}, {3600, 25}, {3550, 20}, {3500, 15},
            {3450, 12}, {3400, 9},  {3350, 6},  {3300, 4},  {3200, 2},
            {3100, 1},  {3000, 0.5f}, {2900, 0}
        };
        constexpr size_t N = sizeof(curve) / sizeof(curve[0]);

        // Clamp outside range
        if (voltage_mv >= curve[0].voltage_mv) return curve[0].percentage;
        if (voltage_mv <= curve[N-1].voltage_mv) return curve[N-1].percentage;

        // Locate segment for interpolation
        for (size_t i = 0; i < N - 1; ++i) {
            if (voltage_mv <= curve[i].voltage_mv && voltage_mv > curve[i+1].voltage_mv) {
                const float v_high = curve[i].voltage_mv;
                const float p_high = curve[i].percentage;
                const float v_low  = curve[i+1].voltage_mv;
                const float p_low  = curve[i+1].percentage;
                float ratio = (v_high - voltage_mv) / (v_high - v_low);
                return p_high - ratio * (p_high - p_low);
            }
        }
        return 0.0f; // Fallback, should not happen

    }
}; 