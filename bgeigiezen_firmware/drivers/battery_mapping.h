#pragma once

#include <cstdint>

struct BatteryMapping {
    static float voltage_to_percentage(uint16_t voltage_mv) {
        // Discharge curve table for Vapecell N40 (values from provided MD file)
        // Linear-in-time curve for Vapecell N40 (4000 mAh)
        // Derived from 16.5 h constant-current discharge test. Percentage is
        // computed as (1 – t / 16.5 h) so that remaining-percentage drops almost
        // perfectly linearly with time when the cell is under a similar load.
        // Measurements were then reduced to ~30 points and forced monotonic to
        // avoid interpolation artefacts.
        static const struct {
            uint16_t voltage_mv; // millivolts
            float    percentage; // 0-100 % remaining
        } curve[] = {
            {4200, 100}, {4054, 97},  {4035, 93.9}, {4028, 90.9}, {4014, 86.1},
            {4008, 84.8}, {4000, 81.8}, {3980, 78.8}, {3967, 75.8}, {3941, 71.5},
            {3912, 69.7}, {3890, 66.7}, {3854, 63.6}, {3819, 60.6}, {3781, 57.6},
            {3725, 54.5}, {3688, 51.5}, {3656, 48.5}, {3614, 45.5}, {3571, 42.4},
            {3525, 39.4}, {3475, 36.4}, {3427, 33.3}, {3381, 30.3}, {3340, 27.3},
            {3293, 24.2}, {3246, 21.2}, {3203, 18.2}, {3162, 15.2}, {3120, 12.1},
            {3077, 9.1},  {3030, 6.1},  {2973, 3.0},  {2931, 0}
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