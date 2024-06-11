#ifndef BGEIGIEZEN_BLUETOOTH_CONNECTOR_H_
#define BGEIGIEZEN_BLUETOOTH_CONNECTOR_H_

#include <BLEDevice.h>

#include <Handler.hpp>

#include "utils/bluetooth_settings.h"
#include "workers/local_storage.h"
#include "workers/log_aggregator.h"

/**
 * Setups up bluetooth endpoint for the device, allowing it to send readings over bluetooth
 */
class BluetoothReporter : public Handler {
 public:
  typedef enum BluetoothStatus {
    e_bluetooth_client_sent,
    e_bluetooth_no_clients,
  } Status;

  explicit BluetoothReporter(LocalStorage& config);
  virtual ~BluetoothReporter() = default;

  uint32_t client_count() const;

 protected:
  bool activate(bool retry) override;
  void deactivate() override;
  int8_t handle_produced_work(const worker_map_t& workers) override;

  int8_t handle_async() override;

 private:

  class BTCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      pServer->startAdvertising(); // restart advertising
    };

    void onDisconnect(BLEServer* pServer) {
      pServer->startAdvertising(); // restart advertising
    }
  };

  bool send(const DataLine& reading) const;

  void create_ble_profile_service(BLEServer* pServer);
  void create_ble_device_service(BLEServer* pServer);
  void create_ble_data_service(BLEServer* pServer);

  LocalStorage& _config;
  BLEServer* _pServer;
  BLECharacteristic* _pDataRXCharacteristic;
  BTCallbacks _btCallbacks;

  char _log_string[LINE_BUFFER_SIZE] = "";
  uint8_t _addr[BLE_DATA_ADDR_SIZE] = BLE_DATA_ADDR;
};

#endif //BGEIGIEZEN_BLUETOOTH_CONNECTOR_H_
