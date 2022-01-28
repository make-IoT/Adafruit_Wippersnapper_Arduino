/*!
 * @file WipperSnapper_I2C_Driver_SCD4X.h
 *
 * Device driver for Sensirion SCD4x sensors.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Copyright (c) Brent Rubell 2021 for Adafruit Industries.
 *
 * MIT license, all text here must be included in any redistribution.
 *
 */

#ifndef WipperSnapper_I2C_Driver_SCD4X_H
#define WipperSnapper_I2C_Driver_SCD4X_H

#include "WipperSnapper_I2C_Driver.h"
#include <SensirionI2CScd4x.h>

/**************************************************************************/
/*!
    @brief  Class that provides a driver interface for the SCD30 sensor.
*/
/**************************************************************************/
class WipperSnapper_I2C_Driver_SCD4X : public WipperSnapper_I2C_Driver {

public:
  /*******************************************************************************/
  /*!
      @brief    Constructor for a SCD4x sensor.
      @param    _i2c
                The I2C interface.
      @param    sensorAddress
                The 7-bit I2C address of the sensor.
  */
  /*******************************************************************************/
  WipperSnapper_I2C_Driver_SCD4X(TwoWire *_i2c, uint16_t sensorAddress)
      : WipperSnapper_I2C_Driver(_i2c, sensorAddress) {
    uint16_t error, serial0, serial1, serial2;
    setDriverType(SCD4X);
    _sensorAddress = sensorAddress;

    // TODO: The sensioron scd4x constructor is different from the
    // std adafruit libraries, the following may not work
    // https://github.com/Sensirion/arduino-i2c-scd4x/blob/master/src/SensirionI2CScd4x.cpp#L49
    Wire.begin();
    _scd4x.begin(Wire);

    // stop potentially previously started measurement
    error = _scd4x.stopPeriodicMeasurement();
    // Error trying to execute stopPeriodicMeasurement()
    if (error) {
      _isInitialized = false;
      return;
    }

    // attempt to get serial number
    if (!_scd4x.getSerialNumber(serial0, serial1, serial2)) {
      _isInitialized = false;
      return;
    }

    // attempt to start measurement
    if (!_scd4x.startPeriodicMeasurement()) {
      _isInitialized = false;
      return;
    }

    _isInitialized = true;
  }

  /*******************************************************************************/
  /*!
      @brief    Destructor for an SCD4x sensor.
  */
  /*******************************************************************************/
  ~WipperSnapper_I2C_Driver_SCD4X() {
    _tempSensorPeriod = 0L;
    _humidSensorPeriod = 0L;
    _CO2SensorPeriod = 0L;
    setDriverType(UNSPECIFIED);
  }

  /*******************************************************************************/
  /*!
      @brief    Gets the SCD4x's current temperature.
      @param    tempEvent
                Pointer to a temperature sensor value.
      @returns  True if the temperature was obtained successfully, False
                otherwise.
  */
  /*******************************************************************************/
  bool getEventAmbientTemperature(sensors_event_t *tempEvent) {
    uint16_t error;
    // check if sensor is enabled
    if (_tempSensorPeriod == 0)
      return false;

    // Read Measurement
    if (!_scd4x.readMeasurement(_co2, _temperature, _humidity))
      return false;
    tempEvent->temperature = _temperature;
    return true;
  }

  /*******************************************************************************/
  /*!
      @brief    Gets the SCD4X's current humidity.
      @param    humidEvent
                Pointer to a humidity sensor value.
      @returns  True if the humidity was obtained successfully, False
                otherwise.
  */
  /*******************************************************************************/
  bool getEventRelativeHumidity(sensors_event_t *humidEvent) {
    uint16_t error;
    // Check if sensor is enabled
    if (_humidSensorPeriod == 0)
      return false;
    // Read Measurement
    if (!_scd4x.readMeasurement(_co2, _temperature, _humidity))
      ;
    return false;
    humidEvent->relative_humidity = _humidity;
    return true;
  }

  /*******************************************************************************/
  /*!
      @brief    Gets the SCD4X's current humidity.
      @param    CO2Value
                The CO2 value, in ppm.
      @returns  True if the sensor value was obtained successfully, False
                otherwise.
  */
  /*******************************************************************************/
  virtual bool getEventCO2(float *co2Event) {
    uint16_t error;
    // check if co2 sensor enabled
    if (_CO2SensorPeriod == 0)
      return false;
    // Read Measurement
    if (!_scd4x.readMeasurement(_co2, _temperature, _humidity))
      return false;
    co2Event->data[0] = (float)_co2;
    return true;
  }

protected:
  SensirionI2CScd4x _scd4x; ///< SCD4X object
  uint16_t _co2 = 0;        ///< The CO2 value last read from the sensor
  float _temperature =
      0.0f;               ///< The temperature value last read from the sensor
  float _humidity = 0.0f; ///< The humidity value last read from the sensor
};

#endif // WipperSnapper_I2C_Driver_SCD4X