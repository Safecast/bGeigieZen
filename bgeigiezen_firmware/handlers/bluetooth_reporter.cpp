#include <Arduino.h>

#include "bluetooth_reporter.h"
#include "identifiers.h"

BluetoothReporter::BluetoothReporter(LocalStorage& config) : Handler(),
                                                             _config(config),
                                                             _pServer(nullptr),
                                                             _pDataRXCharacteristic(nullptr),
                                                             _btCallbacks() {
}

bool BluetoothReporter::activate(bool) {
  if (!_config.get_device_id()) {
    return false;
  }
  if (BLEDevice::getInitialized()) {
    // Already initialized
    _pServer->getAdvertising()->start();
    return true;
  }

  char deviceName[16];
  sprintf(deviceName, "bGeigie%d", _config.get_device_id());
  BLEDevice::init(deviceName);

  if (!_pServer) {
    _pServer = BLEDevice::createServer();

    create_ble_profile_service(_pServer);
    create_ble_device_service(_pServer);
    create_ble_data_service(_pServer);
  }

  BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_PROFILE_UUID);
  pAdvertising->addServiceUUID(SERVICE_DEVICE_UUID);
  pAdvertising->addServiceUUID(SERVICE_DATA_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  BLEDevice::startAdvertising();

  _pServer->setCallbacks(&_btCallbacks); //set the callback functions to restart advertising

  M5_LOGD("Bluetooth initialized, device: %s", deviceName);
  return BLEDevice::getInitialized();
}

void BluetoothReporter::deactivate() {
  kill_task();

  _pServer->setCallbacks(nullptr); // Clear callbacks

  for (auto& peerDevice: BLEDevice::getPeerDevices(true)) {
    _pServer->disconnect(peerDevice.first);
  }
  delay(10);
  _pServer->getAdvertising()->stop();
}

int8_t BluetoothReporter::handle_produced_work(const worker_map_t& workers) {
  const auto& logData = workers.worker<LogAggregator>(k_worker_log_aggregator);
  if (!logData->is_fresh()) {
    return e_handler_idle;
  }
  // Fresh reading is produced
  const auto& reading = logData->get_data();
  strcpy(_log_string, reading.log_string);

  //  M5_LOGD("Starting task to send data over BT...");
  return start_task("bt_send", 2048 * 2);
}

void BluetoothReporter::create_ble_profile_service(BLEServer* pServer) {
  BLEService* pProfileService = pServer->createService(SERVICE_PROFILE_UUID);
  BLECharacteristic* pProfileNameCharacteristic = pProfileService->createCharacteristic(
      CHARACTERISTIC_PROFILE_NAME_UUID,
      BLECharacteristic::PROPERTY_READ);
  /*BLECharacteristic* pProfileAppearanceCharacteristic = */ pProfileService->createCharacteristic(
      CHARACTERISTIC_PROFILE_APPEARANCE_UUID,
      BLECharacteristic::PROPERTY_READ);

  pProfileNameCharacteristic->setValue(BLE_PROFILE_NAME);

  pProfileService->start();
}

void BluetoothReporter::create_ble_device_service(BLEServer* pServer) {
  BLEService* pDeviceService = pServer->createService(SERVICE_DEVICE_UUID);

  // Manufacturer name
  BLECharacteristic* pDeviceManufacturerCharacteristic = pDeviceService->createCharacteristic(
      CHARACTERISTIC_DEVICE_MANUFACTURER_UUID,
      BLECharacteristic::PROPERTY_READ);
  static BLEDescriptor* pDescriptorManufacturer = new BLEDescriptor((uint16_t) 0x2901);
  pDescriptorManufacturer->setValue("Manufacturer Name String");
  pDeviceManufacturerCharacteristic->addDescriptor(pDescriptorManufacturer);

  // Model number
  BLECharacteristic* pDeviceModelCharacteristic = pDeviceService->createCharacteristic(
      CHARACTERISTIC_DEVICE_MODEL_UUID,
      BLECharacteristic::PROPERTY_READ);
  static BLEDescriptor* pDescriptorModel = new BLEDescriptor((uint16_t) 0x2901);
  pDescriptorModel->setValue("Model Number String");
  pDeviceModelCharacteristic->addDescriptor(pDescriptorModel);

  // Firmware revision
  BLECharacteristic* pDeviceFirmwareCharacteristic = pDeviceService->createCharacteristic(
      CHARACTERISTIC_DEVICE_FIRMWARE_UUID,
      BLECharacteristic::PROPERTY_READ);
  static BLEDescriptor* pDescriptorFirmware = new BLEDescriptor((uint16_t) 0x2901);
  pDescriptorFirmware->setValue("Firmware Revision String");
  pDeviceFirmwareCharacteristic->addDescriptor(pDescriptorFirmware);

  // Hardware revision
  BLECharacteristic* pDeviceRevisionCharacteristic = pDeviceService->createCharacteristic(
      CHARACTERISTIC_DEVICE_REVISION_UUID,
      BLECharacteristic::PROPERTY_READ);
  static BLEDescriptor* pDescriptorHardware = new BLEDescriptor((uint16_t) 0x2901);
  pDescriptorHardware->setValue("Hardware Revision String");
  pDeviceRevisionCharacteristic->addDescriptor(pDescriptorHardware);

  pDeviceManufacturerCharacteristic->setValue(BLE_DEVICE_INFO_MANUFACTURER);
  pDeviceModelCharacteristic->setValue(BLE_DEVICE_INFO_MODEL);
  pDeviceFirmwareCharacteristic->setValue(BLE_DEVICE_INFO_FIRMWARE_REVISION);
  pDeviceRevisionCharacteristic->setValue(BLE_DEVICE_INFO_HARDWARE_REVISION);

  pDeviceService->start();
}

void BluetoothReporter::create_ble_data_service(BLEServer* pServer) {
  BLEService* pDataService = pServer->createService(SERVICE_DATA_UUID);

  // DB Addr
  BLECharacteristic* pDataDBAddrCharacteristic = pDataService->createCharacteristic(
      CHARACTERISTIC_DATA_BDADDR_UUID,
      BLECharacteristic::PROPERTY_READ);
  static BLEDescriptor* pDescriptorAddr = new BLEDescriptor((uint16_t) 0x2901);
  pDescriptorAddr->setValue("DB-Addr");
  pDataDBAddrCharacteristic->addDescriptor(pDescriptorAddr);

  // Baudrate
  BLECharacteristic* pDataBaudCharacteristic = pDataService->createCharacteristic(
      CHARACTERISTIC_DATA_BAUD_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  static BLEDescriptor* pDescriptorBaud = new BLEDescriptor((uint16_t) 0x2901);
  pDescriptorBaud->setValue("Baudrate");
  pDataBaudCharacteristic->addDescriptor(pDescriptorBaud);

  // RX
  _pDataRXCharacteristic = pDataService->createCharacteristic(
      CHARACTERISTIC_DATA_RX_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
  static BLEDescriptor* pDescriptorRX = new BLEDescriptor((uint16_t) 0x2901);
  pDescriptorRX->setValue("RX");
  _pDataRXCharacteristic->addDescriptor(pDescriptorRX);
  static BLEDescriptor* pDescriptorRXNotifications = new BLEDescriptor((uint16_t) 0x2902);
  pDescriptorRXNotifications->setValue("Notifications and indications disabled");
  _pDataRXCharacteristic->addDescriptor(pDescriptorRXNotifications);

  // TX
  BLECharacteristic* pDataTXCharacteristic = pDataService->createCharacteristic(
      CHARACTERISTIC_DATA_TX_UUID,
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  static BLEDescriptor* pDescriptorTX = new BLEDescriptor((uint16_t) 0x2901);
  pDescriptorTX->setValue("TX");
  pDataTXCharacteristic->addDescriptor(pDescriptorTX);

  pDataDBAddrCharacteristic->setValue(_addr, BLE_DATA_ADDR_SIZE);

  pDataService->start();
}

int8_t BluetoothReporter::handle_async() {
  if (_pServer->getConnectedCount() == 0) {
    // No clients to send data to
    return e_bluetooth_no_clients;
  }
  size_t size = strlen(_log_string);

  int segment = 0;
  const static uint8_t max_segment_size = 20; // Max that can be sent over bluetooth
  do {
    ++segment;
    uint8_t segment_size = segment * max_segment_size > size ? size % max_segment_size : max_segment_size;
    char to_send[segment_size];
    strncpy(to_send, _log_string + ((segment - 1) * max_segment_size), segment_size);

    _pDataRXCharacteristic->setValue((uint8_t*) to_send, segment_size);
    _pDataRXCharacteristic->notify();
  } while (segment * max_segment_size < size);
  return e_bluetooth_client_sent;
}

uint32_t BluetoothReporter::client_count() const {
  return _pServer->getConnectedCount();
}
