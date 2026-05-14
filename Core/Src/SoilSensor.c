#include "SoilSensor.h"
#include <stdbool.h>
#include <string.h>
#include "cmsis_os.h" 

#define RS485_CONTROL_PORT  GPIOA
#define RS485_CONTROL_PIN   GPIO_PIN_1

static const uint8_t CMD_READ_ALL[8] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};

bool SoilSensor_Init(SoilSensor *sensor, UART_HandleTypeDef *huart) {
    memset(sensor, 0, sizeof(SoilSensor));
    sensor->huart = huart;

    HAL_GPIO_WritePin(RS485_CONTROL_PORT, RS485_CONTROL_PIN, GPIO_PIN_RESET);
    
    osDelay(100); 
    
    for (int i = 0; i < 3; i++) {
        if (SoilSensor_ReadAllVariables(sensor)) {
            return true;
        }
        osDelay(100);
    }
    return false;
}

bool SoilSensor_ReadAllVariables(SoilSensor *sensor) {
    __HAL_UART_CLEAR_OREFLAG(sensor->huart);

    HAL_GPIO_WritePin(RS485_CONTROL_PORT, RS485_CONTROL_PIN, GPIO_PIN_SET);

    HAL_StatusTypeDef txStatus = HAL_UART_Transmit(sensor->huart, (uint8_t*)CMD_READ_ALL, 8, 100);  

    osDelay(10);
    
    HAL_GPIO_WritePin(RS485_CONTROL_PORT, RS485_CONTROL_PIN, GPIO_PIN_RESET);

    if (txStatus != HAL_OK) return false;

    if (HAL_UART_Receive(sensor->huart, sensor->buffer, 19, 500) != HAL_OK) {
        return false;
    }

    if (sensor->buffer[0] != 0x01 || sensor->buffer[1] != 0x03) return false;

    sensor->humidity    = (float)((uint16_t)(sensor->buffer[3] << 8 | sensor->buffer[4]) & 0xFFF) / 10.0f;
    sensor->temperature = (float)((uint16_t)(sensor->buffer[5] << 8 | sensor->buffer[6]) & 0xFFF) / 10.0f;
    sensor->ec          = (uint16_t)((sensor->buffer[7] << 8 | sensor->buffer[8]) & 0xFFF);
    sensor->ph          = (float)((uint16_t)(sensor->buffer[9] << 8 | sensor->buffer[10]) & 0xFFF) / 10.0f;
    sensor->nitrogen    = (uint16_t)((sensor->buffer[11] << 8 | sensor->buffer[12]) & 0xFFF);
    sensor->phosphorus  = (uint16_t)((sensor->buffer[13] << 8 | sensor->buffer[14]) & 0xFFF);
    sensor->potassium   = (uint16_t)((sensor->buffer[15] << 8 | sensor->buffer[16]) & 0xFFF);

    sensor->isFresh = 1;
    return true;
}