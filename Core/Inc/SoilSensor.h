#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    UART_HandleTypeDef *huart;
    float temperature;
    float humidity;
    float ec;
    float ph;
    float nitrogen;
    float phosphorus;
    float potassium;

    uint8_t isFresh;
    uint8_t buffer[19];
} SoilSensor;

bool SoilSensor_Init(SoilSensor *sensor, UART_HandleTypeDef *huart);

bool SoilSensor_ReadAllVariables(SoilSensor *sensor);

#endif