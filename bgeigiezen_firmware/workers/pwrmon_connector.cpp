/** @brief INA3221 voltage and current monitor IC handler for Core2 
 * 
 * Separate instrumentation work to determine power requirements
 * for remote or solar powered operation.
 */

#include "pwrmon_connector.h"
#include "debugger.h"
#include "identifiers.h"


PwrmonConnector::PwrmonConnector() : ProcessWorker<PwrmonData>({.ibatt=0.0, .vbatt=0.0,
                              .iboost=0.0, .vboost=0.0, .ibus=0.0, .vbus=0}, 1000){
}

/**
 * @return true if initialized INA3221 IC, false if no connection to the IC.
 */
bool PwrmonConnector::activate(bool retry) {

#ifdef M5_CORE2
  // Initialize reading from INA3221 ammeter IC
  // INA3221 I2C address 0x40
  uint16_t ina_mid = M5.Axp.ina3221.getManufID();
  if(0x5449 == ina_mid) { // Magic number from INA3221 datasheet section 8.6.1
    DEBUG_PRINTF("INA3221: Manufacturer ID = %04x\n", ina_mid);
  } else {
    DEBUG_PRINTF("INA3221: Incorrect Manufacturer ID = %04x, not activating.\n",
                  ina_mid);
    return false;  // Not a INA3221 IC
  }

  uint16_t ina_conf = M5.Axp.ina3221.getReg(INA3221_REG_CONF);
  DEBUG_PRINTF("INA3221: Config = %04x\n", ina_conf);

  // From Core2 v1.1 schematic p.4, shunt resistors 10 milliOhm
  M5.Axp.ina3221.setShuntRes(10L, 10L, 10L);
  // From Core2 v1.1 schematic p.2, filter resistors 10 Ohm
  M5.Axp.ina3221.setFilterRes(10L, 10L, 10L);

  // Sets operating mode to continious [sic]
  M5.Axp.ina3221.setModeContinious();
  // Enables shunt-voltage measurement
  M5.Axp.ina3221.setShuntMeasEnable();
  // Enables bus-voltage measurement
  M5.Axp.ina3221.setBusMeasEnable();

  // Gets battery current in A, compensated with calculated offset voltage.
  float ina_ibatt = M5.Axp.ina3221.getCurrentCompensated(INA3221_CH1);
  DEBUG_PRINTF("INA3221: Battery Current (Compensated) = %f\n", ina_ibatt);
  // Gets battery voltage in V.
  float ina_vbatt = M5.Axp.ina3221.getVoltage(INA3221_CH1);
  DEBUG_PRINTF("INA3221: Battery Voltage = %f\n", ina_vbatt);

  // Gets 5V boost current in A, compensated with calculated offset voltage.
  float ina_iboost = M5.Axp.ina3221.getCurrentCompensated(INA3221_CH2);
  DEBUG_PRINTF("INA3221: 5V Boost Current (Compensated) = %f\n", ina_iboost);
  // Gets 5V Boost voltage in V.
  float ina_vboost = M5.Axp.ina3221.getVoltage(INA3221_CH2);
  DEBUG_PRINTF("INA3221: 5V Boost Voltage = %f\n", ina_vboost);

  // Gets bus current in A, compensated with calculated offset voltage.
  float ina_ibus = M5.Axp.ina3221.getCurrentCompensated(INA3221_CH3);
  DEBUG_PRINTF("INA3221: Bus Current (Compensated) = %f\n", ina_ibus);
  // Gets bus voltage in V.
  float ina_vbus = M5.Axp.ina3221.getVoltage(INA3221_CH3);
  DEBUG_PRINTF("INA3221: Bus Voltage = %f\n", ina_vbus);

  return true;
#endif
  return false;  // Not a CORE2, no INA3221 IC
}

int8_t PwrmonConnector::produce_data(const worker_map_t& workers) {
  auto ret_status = e_worker_idle;
#ifdef M5_CORE2

  // Gets battery current in A, compensated with calculated offset voltage.
  float ina_ibatt = M5.Axp.ina3221.getCurrentCompensated(INA3221_CH1);
  DEBUG_PRINTF("INA3221: Battery Current (Compensated) = %f\n", ina_ibatt);
  // Gets battery voltage in V.
  float ina_vbatt = M5.Axp.ina3221.getVoltage(INA3221_CH1);
  DEBUG_PRINTF("INA3221: Battery Voltage = %f\n", ina_vbatt);

  // Gets 5V boost current in A, compensated with calculated offset voltage.
  float ina_iboost = M5.Axp.ina3221.getCurrentCompensated(INA3221_CH2);
  DEBUG_PRINTF("INA3221: 5V Boost Current (Compensated) = %f\n", ina_iboost);
  // Gets 5V Boost voltage in V.
  float ina_vboost = M5.Axp.ina3221.getVoltage(INA3221_CH2);
  DEBUG_PRINTF("INA3221: 5V Boost Voltage = %f\n", ina_vboost);

  // Gets bus current in A, compensated with calculated offset voltage.
  float ina_ibus = M5.Axp.ina3221.getCurrentCompensated(INA3221_CH3);
  DEBUG_PRINTF("INA3221: Bus Current (Compensated) = %f\n", ina_ibus);
  // Gets bus voltage in V.
  float ina_vbus = M5.Axp.ina3221.getVoltage(INA3221_CH3);
  DEBUG_PRINTF("INA3221: Bus Voltage = %f\n", ina_vbus);

  ret_status = e_worker_data_read;

#endif

  return ret_status;
}


