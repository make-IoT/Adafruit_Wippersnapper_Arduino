/*!
 * @file WipperSnapper_I2C.cpp
 *
 * This component initiates I2C operations
 * using the Arduino generic TwoWire driver.
 *
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * Copyright (c) Brent Rubell 2021-2022 for Adafruit Industries.
 *
 * BSD license, all text here must be included in any redistribution.
 *
 */

#include "WipperSnapper_I2C.h"

/***************************************************************************************************************/
/*!
    @brief    Creates a new WipperSnapper I2C component.
    @param    msgInitRequest
              The I2C initialization request message.
*/
/***************************************************************************************************************/
WipperSnapper_Component_I2C::WipperSnapper_Component_I2C(
    wippersnapper_i2c_v1_I2CBusInitRequest *msgInitRequest) {
  WS_DEBUG_PRINTLN("EXEC: New I2C Port ");
  WS_DEBUG_PRINT("\tPort #: ");
  WS_DEBUG_PRINTLN(msgInitRequest->i2c_port_number);
  WS_DEBUG_PRINT("\tSDA Pin: ");
  WS_DEBUG_PRINTLN(msgInitRequest->i2c_pin_sda);
  WS_DEBUG_PRINT("\tSCL Pin: ");
  WS_DEBUG_PRINTLN(msgInitRequest->i2c_pin_scl);
  WS_DEBUG_PRINT("\tFrequency (Hz): ");
  WS_DEBUG_PRINTLN(msgInitRequest->i2c_frequency);

#if defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2)
  // Invert Feather ESP32-S2 pin power for I2C
  pinMode(7, OUTPUT);
  digitalWrite(7, LOW);
#elif defined(ARDUINO_ADAFRUIT_FEATHER_ESP32S2_TFT)
  // Power the AP2112 regulator
  // TODO: Remove when fixed by latest BSP release
  pinMode(TFT_I2C_POWER, OUTPUT);
  digitalWrite(TFT_I2C_POWER, HIGH);
#endif

  // Enable pullups on SCL, SDA
  pinMode(msgInitRequest->i2c_pin_scl, INPUT_PULLUP);
  pinMode(msgInitRequest->i2c_pin_sda, INPUT_PULLUP);
  delay(150);

  // Is SDA or SCL stuck low?
  if ((digitalRead(msgInitRequest->i2c_pin_scl) == 0) ||
      (digitalRead(msgInitRequest->i2c_pin_sda) == 0)) {
    _busStatusResponse =
        wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_ERROR_PULLUPS;
    _isInit = false;
  } else {
    // Reset state of SCL/SDA pins
    pinMode(msgInitRequest->i2c_pin_scl, INPUT);
    pinMode(msgInitRequest->i2c_pin_sda, INPUT);

// Initialize I2C bus
#if defined(ARDUINO_ARCH_ESP32)
    _i2c = new TwoWire(msgInitRequest->i2c_port_number);
    if (!_i2c->begin(msgInitRequest->i2c_pin_sda,
                     msgInitRequest->i2c_pin_scl)) {
      _isInit = false; // if the peripheral was configured incorrectly
    } else {
      _isInit = true; // if the peripheral was configured incorrectly
    }
    _i2c->setClock(msgInitRequest->i2c_frequency);
#elif defined(ARDUINO_ARCH_ESP8266)
    _i2c = new TwoWire();
    _i2c->begin(msgInitRequest->i2c_pin_sda, msgInitRequest->i2c_pin_scl);
    _i2c->setClock(msgInitRequest->i2c_frequency);
    _isInit = true;
#else
    // SAMD
    _i2c = new TwoWire(&PERIPH_WIRE, msgInitRequest->i2c_pin_sda,
                       msgInitRequest->i2c_pin_scl);
    _i2c->begin();
    _isInit = true;
#endif

    // set i2c obj. properties
    _portNum = msgInitRequest->i2c_port_number;
    _busStatusResponse = wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_SUCCESS;
  }
}

/*************************************************************/
/*!
    @brief    Destructor for a WipperSnapper I2C component.
*/
/*************************************************************/
WipperSnapper_Component_I2C::~WipperSnapper_Component_I2C() {
  _portNum = 100; // Invalid = 100
  _isInit = false;
}

/*****************************************************/
/*!
    @brief    Returns if i2c port is initialized.
    @returns  True if initialized, False otherwise.
*/
/*****************************************************/
bool WipperSnapper_Component_I2C::isInitialized() { return _isInit; }

/*****************************************************/
/*!
    @brief    Returns the state of the I2C bus.
    @returns  wippersnapper_i2c_v1_BusResponse.
*/
/*****************************************************/
wippersnapper_i2c_v1_BusResponse WipperSnapper_Component_I2C::getBusStatus() {
  return _busStatusResponse;
}

/************************************************************************/
/*!
    @brief    Scans all I2C addresses on the bus between 0x08 and 0x7F
              inclusive and returns an array of the devices found.
    @returns  wippersnapper_i2c_v1_I2CBusScanResponse
*/
/************************************************************************/
wippersnapper_i2c_v1_I2CBusScanResponse
WipperSnapper_Component_I2C::scanAddresses() {
  uint8_t endTransmissionRC;
  uint16_t address;
  wippersnapper_i2c_v1_I2CBusScanResponse scanResp =
      wippersnapper_i2c_v1_I2CBusScanResponse_init_zero;

#ifndef ARDUINO_ARCH_ESP32
  // Set I2C WDT timeout to catch I2C hangs, SAMD-specific
  WS.enableWDT(I2C_TIMEOUT_MS);
  WS.feedWDT();
#endif

  // Scan all I2C addresses between 0x08 and 0x7F inclusive and return a list of
  // those that respond.
  WS_DEBUG_PRINTLN("EXEC: I2C Scan");
  for (address = 0x08; address < 0x7F; address++) {
    _i2c->beginTransmission(address);
    endTransmissionRC = _i2c->endTransmission();

#if defined(ARDUINO_ARCH_ESP32)
    // Check endTransmission()'s return code (Arduino-ESP32 ONLY)
    // https://github.com/espressif/arduino-esp32/blob/master/libraries/Wire/src/Wire.cpp
    if (endTransmissionRC == 5) {
      WS_DEBUG_PRINTLN("ESP_ERR_TIMEOUT: I2C Bus Busy");
      scanResp.bus_response =
          wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_ERROR_HANG;
      // NOTE: ESP-IDF appears to handle this "behind the scenes" by
      // resetting/clearing the bus. The user should be prompted to
      // perform a bus scan again.
      break;
    } else if (endTransmissionRC == 7) {
      WS_DEBUG_PRINT("I2C_ESP_ERR: SDA/SCL shorted, requests queued: ");
      WS_DEBUG_PRINTLN(endTransmissionRC);
      break;
    }
#endif

    // Found device!
    if (endTransmissionRC == 0) {
      WS_DEBUG_PRINT("Found I2C Device at 0x");
      WS_DEBUG_PRINTLN(address);
      scanResp.addresses_found[scanResp.addresses_found_count] =
          (uint32_t)address;
      scanResp.addresses_found_count++;
    }
  }

#ifndef ARDUINO_ARCH_ESP32
  // re-enable WipperSnapper SAMD WDT global timeout
  WS.enableWDT(WS_WDT_TIMEOUT);
  WS.feedWDT();
#endif

  WS_DEBUG_PRINT("I2C Devices Found: ")
  WS_DEBUG_PRINTLN(scanResp.addresses_found_count);

  scanResp.bus_response = wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_SUCCESS;
  return scanResp;
}

/*******************************************************************************/
/*!
    @brief    Initializes I2C device driver.
    @param    msgDeviceInitReq
              A decoded I2CDevice initialization request message.
    @returns True if I2C device is initialized and attached, False otherwise.
*/
/*******************************************************************************/
bool WipperSnapper_Component_I2C::initI2CDevice(
    wippersnapper_i2c_v1_I2CDeviceInitRequest *msgDeviceInitReq) {
  WS_DEBUG_PRINTLN("Attempting to initialize an I2C device...");

  uint16_t i2cAddress = (uint16_t)msgDeviceInitReq->i2c_device_address;
  if (strcmp("aht20", msgDeviceInitReq->i2c_device_name) == 0) {
    _ahtx0 = new WipperSnapper_I2C_Driver_AHTX0(this->_i2c, i2cAddress);
    if (!_ahtx0->isInitialized()) {
      WS_DEBUG_PRINTLN("ERROR: Failed to initialize AHTX0 chip!");
      _busStatusResponse =
          wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_DEVICE_INIT_FAIL;
      return false;
    }
    WS_DEBUG_PRINTLN("AHTX0 Initialized Successfully!");
    _ahtx0->configureDriver(msgDeviceInitReq);
    drivers.push_back(_ahtx0);
  } else if (strcmp("bme280", msgDeviceInitReq->i2c_device_name) == 0) {
    _bme280 = new WipperSnapper_I2C_Driver_BME280(this->_i2c, i2cAddress);
    if (!_bme280->isInitialized()) {
      WS_DEBUG_PRINTLN("ERROR: Failed to initialize BME280!");
      _busStatusResponse =
          wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_DEVICE_INIT_FAIL;
      return false;
    }
    _bme280->configureDriver(msgDeviceInitReq);
    drivers.push_back(_bme280);
    WS_DEBUG_PRINTLN("BME280 Initialized Successfully!");
  } else if (strcmp("dps310", msgDeviceInitReq->i2c_device_name) == 0) {
    _dps310 = new WipperSnapper_I2C_Driver_DPS310(this->_i2c, i2cAddress);
    if (!_dps310->isInitialized()) {
      WS_DEBUG_PRINTLN("ERROR: Failed to initialize DPS310!");
      _busStatusResponse =
          wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_DEVICE_INIT_FAIL;
      return false;
    }
    _dps310->configureDriver(msgDeviceInitReq);
    drivers.push_back(_dps310);
    WS_DEBUG_PRINTLN("DPS310 Initialized Successfully!");
  } else if (strcmp("scd30", msgDeviceInitReq->i2c_device_name) == 0) {
    _scd30 = new WipperSnapper_I2C_Driver_SCD30(this->_i2c, i2cAddress);
    if (!_scd30->isInitialized()) {
      WS_DEBUG_PRINTLN("ERROR: Failed to initialize SCD30!");
      _busStatusResponse =
          wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_DEVICE_INIT_FAIL;
      return false;
    }
    _scd30->configureDriver(msgDeviceInitReq);
    drivers.push_back(_scd30);
    WS_DEBUG_PRINTLN("SCD30 Initialized Successfully!");
  } else if (strcmp("mcp9808", msgDeviceInitReq->i2c_device_name) == 0) {
    _mcp9808 = new WipperSnapper_I2C_Driver_MCP9808(this->_i2c, i2cAddress);
    if (!_mcp9808->isInitialized()) {
      WS_DEBUG_PRINTLN("ERROR: Failed to initialize MCP9808!");
      _busStatusResponse =
          wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_DEVICE_INIT_FAIL;
      return false;
    }
    _mcp9808->configureDriver(msgDeviceInitReq);
    drivers.push_back(_mcp9808);
    WS_DEBUG_PRINTLN("MCP9808 Initialized Successfully!");
  } else {
    WS_DEBUG_PRINTLN("ERROR: I2C device type not found!")
    _busStatusResponse =
        wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_UNSUPPORTED_SENSOR;
    return false;
  }
  _busStatusResponse = wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_SUCCESS;
  return true;
}

/*********************************************************************************/
/*!
    @brief    Updates the properties of an I2C device driver.
    @param    msgDeviceUpdateReq
              A decoded I2CDeviceUpdateRequest.
*/
/*********************************************************************************/
void WipperSnapper_Component_I2C::updateI2CDeviceProperties(
    wippersnapper_i2c_v1_I2CDeviceUpdateRequest *msgDeviceUpdateReq) {
  uint16_t i2cAddress = (uint16_t)msgDeviceUpdateReq->i2c_device_address;

  // Loop thru vector of drivers to find the unique address
  for (int i = 0; i < drivers.size(); i++) {
    if (drivers[i]->getI2CAddress() == i2cAddress) {
      // Update the properties of each driver
      for (int j = 0; j < msgDeviceUpdateReq->i2c_device_properties_count;
           j++) {
        switch (msgDeviceUpdateReq->i2c_device_properties[j].sensor_type) {
        case wippersnapper_i2c_v1_SensorType_SENSOR_TYPE_AMBIENT_TEMPERATURE:
          drivers[i]->updateSensorAmbientTemperature(
              msgDeviceUpdateReq->i2c_device_properties[j].sensor_period);
          break;
        case wippersnapper_i2c_v1_SensorType_SENSOR_TYPE_RELATIVE_HUMIDITY:
          drivers[i]->updateSensorRelativeHumidity(
              msgDeviceUpdateReq->i2c_device_properties[j].sensor_period);
          break;
        case wippersnapper_i2c_v1_SensorType_SENSOR_TYPE_PRESSURE:
          drivers[i]->updateSensorPressure(
              msgDeviceUpdateReq->i2c_device_properties[j].sensor_period);
          break;
        case wippersnapper_i2c_v1_SensorType_SENSOR_TYPE_CO2:
          drivers[i]->updateSensorCO2(
              msgDeviceUpdateReq->i2c_device_properties[j].sensor_period);
          break;
        case wippersnapper_i2c_v1_SensorType_SENSOR_TYPE_ALTITUDE:
          drivers[i]->updateSensorAltitude(
              msgDeviceUpdateReq->i2c_device_properties[j].sensor_period);
          break;
        default:
          _busStatusResponse =
              wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_UNSUPPORTED_SENSOR;
          WS_DEBUG_PRINTLN("ERROR: Unable to determine sensor_type!");
          break;
        }
      }
    }
  }
  _busStatusResponse = wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_SUCCESS;
}

/*******************************************************************************/
/*!
    @brief    Deinitializes and deletes an I2C device driver object.
    @param    msgDeviceDeinitReq
              A decoded I2CDeviceDeinitRequest.
*/
/*******************************************************************************/
void WipperSnapper_Component_I2C::deinitI2CDevice(
    wippersnapper_i2c_v1_I2CDeviceDeinitRequest *msgDeviceDeinitReq) {
  uint16_t deviceAddr = (uint16_t)msgDeviceDeinitReq->i2c_device_address;
  std::vector<WipperSnapper_I2C_Driver *>::iterator iter, end;

  for (iter = drivers.begin(), end = drivers.end(); iter != end; ++iter) {
    if ((*iter)->getI2CAddress() == deviceAddr) {
      // Delete the object that iter points to
      delete *iter;
// ESP-IDF, Erase–remove iter ptr from driver vector
#if defined(ARDUINO_ARCH_ESP32) || defined(ARDUINO_ARCH_ESP8266)
      *iter = nullptr;
      drivers.erase(remove(drivers.begin(), drivers.end(), nullptr),
                    drivers.end());
#else
      // Arduino can not erase-remove, erase only
      drivers.erase(iter);
#endif
      WS_DEBUG_PRINTLN("I2C Device De-initialized!");
    }
  }
  _busStatusResponse = wippersnapper_i2c_v1_BusResponse_BUS_RESPONSE_SUCCESS;
}

/*******************************************************************************/
/*!
    @brief    Encodes an I2C sensor device's signal message.
    @param    msgi2cResponse
              Pointer to an I2CResponse signal message.
    @param    sensorAddress
              The unique I2C address of the sensor.
    @returns  True if message encoded successfully, False otherwise.
*/
/*******************************************************************************/
bool WipperSnapper_Component_I2C::encodePublishI2CDeviceEventMsg(
    wippersnapper_signal_v1_I2CResponse *msgi2cResponse,
    uint32_t sensorAddress) {
  // Encode I2CResponse msg
  msgi2cResponse->payload.resp_i2c_device_event.sensor_address = sensorAddress;
  memset(WS._buffer_outgoing, 0, sizeof(WS._buffer_outgoing));
  pb_ostream_t ostream =
      pb_ostream_from_buffer(WS._buffer_outgoing, sizeof(WS._buffer_outgoing));
  if (!pb_encode(&ostream, wippersnapper_signal_v1_I2CResponse_fields,
                 msgi2cResponse)) {
    WS_DEBUG_PRINTLN(
        "ERROR: Unable to encode I2C device event response message!");
    return false;
  }

  // Publish I2CResponse msg
  size_t msgSz;
  pb_get_encoded_size(&msgSz, wippersnapper_signal_v1_I2CResponse_fields,
                      msgi2cResponse);
  WS_DEBUG_PRINT("PUBLISHING -> I2C Device Sensor Event Message...");
  if (!WS._mqtt->publish(WS._topic_signal_i2c_device, WS._buffer_outgoing,
                         msgSz, 1)) {
    return false;
  };
  WS_DEBUG_PRINTLN("PUBLISHED!");
  return true;
}

/*******************************************************************************/
/*!
    @brief    Fills a sensor_event message with the sensor's value and type.
    @param    msgi2cResponse
              A pointer to the signal's I2CResponse message.
    @param    value
              The value read by the sensor.
    @param    sensorType
              The SI unit represented by the sensor's value.
*/
/*******************************************************************************/
void WipperSnapper_Component_I2C::fillEventMessage(
    wippersnapper_signal_v1_I2CResponse *msgi2cResponse, float value,
    wippersnapper_i2c_v1_SensorType sensorType) {
  // fill sensor value
  msgi2cResponse->payload.resp_i2c_device_event
      .sensor_event[msgi2cResponse->payload.resp_i2c_device_event
                        .sensor_event_count]
      .value = value;
  // fill sensor type
  msgi2cResponse->payload.resp_i2c_device_event
      .sensor_event[msgi2cResponse->payload.resp_i2c_device_event
                        .sensor_event_count]
      .type = sensorType;
  msgi2cResponse->payload.resp_i2c_device_event.sensor_event_count++;
}

/*******************************************************************************/
/*!
    @brief    Queries all I2C device drivers for new values. Fills and sends an
              I2CSensorEvent with the sensor event data.
*/
/*******************************************************************************/
void WipperSnapper_Component_I2C::update() {

  // Create response message
  wippersnapper_signal_v1_I2CResponse msgi2cResponse =
      wippersnapper_signal_v1_I2CResponse_init_zero;
  msgi2cResponse.which_payload =
      wippersnapper_signal_v1_I2CResponse_resp_i2c_device_event_tag;

  long curTime;
  std::vector<WipperSnapper_I2C_Driver *>::iterator iter, end;
  for (iter = drivers.begin(), end = drivers.end(); iter != end; ++iter) {
    // Number of events which occured for this driver
    msgi2cResponse.payload.resp_i2c_device_event.sensor_event_count = 0;

    // Event struct
    sensors_event_t event;

    // AMBIENT_TEMPERATURE sensor
    curTime = millis();
    if ((*iter)->sensorAmbientTemperaturePeriod() != 0L &&
        curTime - (*iter)->sensorAmbientTemperaturePeriodPrv() >
            (*iter)->sensorAmbientTemperaturePeriod()) {
      if ((*iter)->getEventAmbientTemperature(&event)) {
        WS_DEBUG_PRINT("Sensor 0x");
        WS_DEBUG_PRINTHEX((*iter)->getI2CAddress());
        WS_DEBUG_PRINTLN("");
        WS_DEBUG_PRINT("\tTemperature: ");
        WS_DEBUG_PRINT(event.temperature);
        WS_DEBUG_PRINTLN(" degrees C");

        // pack event data into msg
        fillEventMessage(
            &msgi2cResponse, event.temperature,
            wippersnapper_i2c_v1_SensorType_SENSOR_TYPE_AMBIENT_TEMPERATURE);

        (*iter)->setSensorAmbientTemperaturePeriodPrv(curTime);
      } else {
        WS_DEBUG_PRINTLN(
            "ERROR: Failed to get ambient temperature sensor reading!");
      }
    }

    // RELATIVE_HUMIDITY sensor
    curTime = millis();
    if ((*iter)->sensorRelativeHumidityPeriod() != 0L &&
        curTime - (*iter)->sensorRelativeHumidityPeriodPrv() >
            (*iter)->sensorRelativeHumidityPeriod()) {
      if ((*iter)->getEventRelativeHumidity(&event)) {
        WS_DEBUG_PRINT("Sensor 0x");
        WS_DEBUG_PRINTHEX((*iter)->getI2CAddress());
        WS_DEBUG_PRINTLN("");
        WS_DEBUG_PRINT("\tHumidity: ");
        WS_DEBUG_PRINT(event.relative_humidity);
        WS_DEBUG_PRINTLN("%RH");

        // pack event data into msg
        fillEventMessage(
            &msgi2cResponse, event.relative_humidity,
            wippersnapper_i2c_v1_SensorType_SENSOR_TYPE_RELATIVE_HUMIDITY);

        (*iter)->setSensorRelativeHumidityPeriodPrv(curTime);
      } else {
        WS_DEBUG_PRINTLN("ERROR: Failed to get humidity sensor reading!");
      }
    }

    // PRESSURE sensor
    curTime = millis();
    if ((*iter)->sensorPressurePeriod() != 0L &&
        curTime - (*iter)->sensorPressurePeriodPrv() >
            (*iter)->sensorPressurePeriod()) {
      if ((*iter)->getEventPressure(&event)) {
        WS_DEBUG_PRINT("Sensor 0x");
        WS_DEBUG_PRINTHEX((*iter)->getI2CAddress());
        WS_DEBUG_PRINTLN("");
        WS_DEBUG_PRINT("\tPressure: ");
        WS_DEBUG_PRINT(event.pressure);
        WS_DEBUG_PRINTLN(" hPa");

        // pack event data into msg
        fillEventMessage(&msgi2cResponse, event.pressure,
                         wippersnapper_i2c_v1_SensorType_SENSOR_TYPE_PRESSURE);

        (*iter)->setSensorPressurePeriodPrv(curTime);
      } else {
        WS_DEBUG_PRINTLN("ERROR: Failed to get Pressure sensor reading!");
      }
    }

    // CO2 sensor
    curTime = millis();
    if ((*iter)->sensorCO2Period() != 0L &&
        curTime - (*iter)->sensorCO2PeriodPrv() > (*iter)->sensorCO2Period()) {
      if ((*iter)->getEventCO2(&event)) {
        WS_DEBUG_PRINT("Sensor 0x");
        WS_DEBUG_PRINTHEX((*iter)->getI2CAddress());
        WS_DEBUG_PRINTLN("");
        WS_DEBUG_PRINT("\tCO2: ");
        WS_DEBUG_PRINT(event.data[0]);
        WS_DEBUG_PRINTLN(" ppm");

        fillEventMessage(&msgi2cResponse, event.data[0],
                         wippersnapper_i2c_v1_SensorType_SENSOR_TYPE_CO2);
        (*iter)->setSensorCO2PeriodPrv(curTime);
      } else {
        WS_DEBUG_PRINTLN("ERROR: Failed to obtain CO2 sensor reading!");
      }
    }

    // Altitude sensor
    curTime = millis();
    if ((*iter)->sensorAltitudePeriod() != 0L &&
        curTime - (*iter)->sensorAltitudePeriodPrv() >
            (*iter)->sensorAltitudePeriod()) {
      if ((*iter)->getEventAltitude(&event)) {
        WS_DEBUG_PRINT("Sensor 0x");
        WS_DEBUG_PRINTHEX((*iter)->getI2CAddress());
        WS_DEBUG_PRINTLN("");
        WS_DEBUG_PRINT("\tAltitude: ");
        WS_DEBUG_PRINT(event.data[0]);
        WS_DEBUG_PRINTLN(" m");

        // pack event data into msg
        fillEventMessage(&msgi2cResponse, event.data[0],
                         wippersnapper_i2c_v1_SensorType_SENSOR_TYPE_ALTITUDE);

        (*iter)->setSensorAltitudePeriodPrv(curTime);
      } else {
        WS_DEBUG_PRINTLN("ERROR: Failed to get altitude sensor reading!");
      }
    }

    // Did this driver obtain data from sensors?
    if (msgi2cResponse.payload.resp_i2c_device_event.sensor_event_count == 0)
      continue;

    // Encode and publish I2CDeviceEvent message
    if (!encodePublishI2CDeviceEventMsg(&msgi2cResponse,
                                        (*iter)->getI2CAddress())) {
      WS_DEBUG_PRINTLN("ERROR: Failed to encode and publish I2CDeviceEvent!");
      continue;
    }
  }
}