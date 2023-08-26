#ifndef BGEIGIEZEN_BLUETOOTH_CONNECTOR_H_
#define BGEIGIEZEN_BLUETOOTH_CONNECTOR_H_

#include <BLEDevice.h>

#include <Handler.hpp>

#include "utils/bluetooth_settings.h"
#include "local_storage.h"
#include "workers/log_aggregator.h"

/**
 * Setups up bluetooth endpoint for the device, allowing it to send readings over bluetooth
 */
class BluetoothReporter : public Handler {
 public:
  typedef enum Status {
    e_handler_idle = -1,
    e_handler_clients_available,
    e_handler_no_clients,
  } Status;

  explicit BluetoothReporter(LocalStorage& config);
  virtual ~BluetoothReporter() = default;

 protected:
  bool activate(bool retry) override;
  void deactivate() override;
  int8_t handle_produced_work(const worker_map_t& workers) override;
 private:

  bool send(const DataLine& reading) const;

  void create_ble_profile_service(BLEServer* pServer);
  void create_ble_device_service(BLEServer* pServer);
  void create_ble_data_service(BLEServer* pServer);

  LocalStorage& config;
  BLEServer* _pServer;
  BLECharacteristic* pDataRXCharacteristic;

  uint8_t addr[BLE_DATA_ADDR_SIZE] = BLE_DATA_ADDR;
};

#endif //BGEIGIEZEN_BLUETOOTH_CONNECTOR_H_
