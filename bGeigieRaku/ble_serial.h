#ifndef __BLE_SERIAL_H__
#define __BLE_SERIAL_H__

/**
 * @brief Setup.
 */
void ble_setup();

/**
 * @brief Sends a string over serial. append \r\n at the end.
 * Leave space for 2 char at end of buffer
 */
void ble_send(char *strbuffer);

#endif
