/*!
 * @file WipperSnapper_I2C_Driver.h
 *
 * Base implementation for I2C device drivers.
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

#ifndef WipperSnapper_I2C_Driver_H
#define WipperSnapper_I2C_Driver_H

#include "Wippersnapper.h"

/**************************************************************************/
/*!
    @brief  Base class for I2C Drivers.
*/
/**************************************************************************/
class WipperSnapper_I2C_Driver {

public:
  /*******************************************************************************/
  /*!
      @brief    Constructor for an I2C sensor.
  */
  /*******************************************************************************/
  WipperSnapper_I2C_Driver(TwoWire *_i2c, uint16_t sensorAddress) {
    _sensorAddress = sensorAddress;
  }

  /*******************************************************************************/
  /*!
      @brief    Destructor for an I2C sensor.
  */
  /*******************************************************************************/
  ~WipperSnapper_I2C_Driver() { _sensorAddress = 0; }

  /*******************************************************************************/
  /*!
      @brief    Gets the initialization status of an I2C driver.
      @returns  True if I2C device is initialized successfully, False otherwise.
  */
  /*******************************************************************************/
  bool getInitialized() { return _isInitialized; }

  /*******************************************************************************/
  /*!
      @brief    Gets the I2C device's address.
      @returns  The I2C device's unique i2c address.
  */
  /*******************************************************************************/
  uint16_t getSensorAddress() { return _sensorAddress; }

  /*********************************************************************************/
  /*!
      @brief    Base implementation - Returns the humidity sensor's period, if
     set.
  */
  /*********************************************************************************/
  virtual long getTempSensorPeriod() { return _tempSensorPeriod; }

  /*******************************************************************************/
  /*!
      @brief    Set the temperature sensor's return frequency.
      @param    tempPeriod
                The time interval at which to return new data from the
     temperature sensor.
  */
  /*******************************************************************************/
  virtual void setTemperatureSensorPeriod(float tempPeriod) {
    // Period is in seconds, cast it to long and convert it to milliseconds
    _tempSensorPeriod = (long)tempPeriod * 1000;
  }

  /*********************************************************************************/
  /*!
      @brief    Base implementation - Returns the previous time interval at
     which the temperature sensor was queried last.
  */
  /*********************************************************************************/
  virtual long getTempSensorPeriodPrv() { return _tempSensorPeriodPrv; }

  /*******************************************************************************/
  /*!
      @brief    Sets a timestamp for when the temperature sensor was queried.
      @param    tempPeriodPrv
                The time when the temperature sensor was queried last.
  */
  /*******************************************************************************/
  virtual void setTemperatureSensorPeriodPrv(float tempPeriodPrv) {
    // Period is in seconds, cast it to long and convert it to milliseconds
    _tempSensorPeriodPrv = (long)tempPeriodPrv * 1000;
  }

  /*******************************************************************************/
  /*!
      @brief    Base implementation - Reads a temperature sensor. Expects value
                to return in the proper SI unit.
  */
  /*******************************************************************************/
  virtual void updateTempSensor(float *temperature) {
    // no-op
  }

  /*********************************************************************************/
  /*!
      @brief    Base implementation - Returns the humidity sensor's period, if
     set.
  */
  /*********************************************************************************/
  virtual long getHumidSensorPeriod() { return _humidSensorPeriod; }

  /*******************************************************************************/
  /*!
      @brief    Set the humidity sensor's return frequency.
      @param    humidPeriod
                The time interval at which to return new data from the humidity
                sensor.
  */
  /*******************************************************************************/
  void setHumiditySensorPeriod(float humidPeriod) {
    // Period is in seconds, cast it to long and convert it to milliseconds
    _humidSensorPeriod = (long)humidPeriod * 1000;
  }

  /*********************************************************************************/
  /*!
      @brief    Base implementation - Returns the previous time interval at
     which the humidity sensor was queried last.
  */
  /*********************************************************************************/
  virtual long getHumidSensorPeriodPrv() { return _humidSensorPeriodPrv; }

  /*******************************************************************************/
  /*!
      @brief    Sets a timestamp for when the humidity sensor was queried.
      @param    humidPeriodPrv
                The time when the humidity sensor was queried last.
  */
  /*******************************************************************************/
  virtual void setHumiditySensorPeriodPrv(float humidPeriodPrv) {
    // Period is in seconds, cast it to long and convert it to milliseconds
    _humidSensorPeriodPrv = (long)humidPeriodPrv * 1000;
  }

  /*******************************************************************************/
  /*!
      @brief    Base implementation - Reads a humidity sensor and converts
                the reading into the expected SI unit.
  */
  /*******************************************************************************/
  virtual void updateHumidSensor(float *humidity) {
    // no-op
  }

protected:
  bool _isInitialized = false; ///< True if the I2C device was initialized
                               ///< successfully, False otherwise.
  uint16_t _sensorAddress;     ///< The I2C device's unique I2C address.
  long _tempSensorPeriod =
      -1L; ///< The time period between reading the temperature sensor's value.
  long _humidSensorPeriod =
      -1L; ///< The time period between reading the humidity sensor's value.
  long _tempSensorPeriodPrv;  ///< The time period when the temperature sensor
                              ///< was last read.
  long _humidSensorPeriodPrv; ///< The time period when the humidity sensor was
                              ///< last read.
};

#endif // WipperSnapper_I2C_Driver_H