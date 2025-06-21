#pragma once

#include <cstdint>

struct BatteryMapping {
    static float voltage_to_percentage(uint16_t voltage_mv) {
        // Linear mapping based on empirical dataset:
        // 4.15 V  (4150 mV) → 100 %
        // 2.95 V  (2950 mV) →   0 %
        // Values beyond this range are clamped.
        constexpr uint16_t FULL_VOLTAGE_MV  = 4150; // 100 %
        constexpr uint16_t EMPTY_VOLTAGE_MV = 2950; //   0 %

        if (voltage_mv >= FULL_VOLTAGE_MV) {
            return 100.0f;
        }
        if (voltage_mv <= EMPTY_VOLTAGE_MV) {
            return 0.0f;
        }

        float span_mv = static_cast<float>(FULL_VOLTAGE_MV - EMPTY_VOLTAGE_MV);
        float delta_mv = static_cast<float>(voltage_mv - EMPTY_VOLTAGE_MV);
        return (delta_mv / span_mv) * 100.0f;

    }
}; 