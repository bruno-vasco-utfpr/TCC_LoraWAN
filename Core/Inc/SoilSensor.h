#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

#include "stm32f4xx_hal.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    UART_HandleTypeDef *huart;
    int16_t temperature;  
    int16_t humidity;     
    uint16_t ec;      
    uint16_t ph;    
    uint16_t nitrogen;  
    uint16_t phosphorus; 
    uint16_t potassium;    
    uint8_t isFresh;
    uint8_t buffer[19];
} SoilSensor;

bool SoilSensor_Init(SoilSensor *sensor, UART_HandleTypeDef *huart);
// Retorno alterado para bool para checar se a leitura teve sucesso
bool SoilSensor_ReadAllVariables(SoilSensor *sensor); 

#endif