#ifndef DEVICE_STATE_HPP
#define DEVICE_STATE_HPP

struct DeviceState {
    enum class Mode {
        e_mode_not_set,
        e_mode_advanced,
        e_mode_simple,
        e_mode_fixed,
        e_mode_flight,
        e_mode_survey,
        e_mode_drive
    };
};

#endif // DEVICE_STATE_HPP
