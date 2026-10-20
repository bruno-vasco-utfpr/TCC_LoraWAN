#include "cmsis_os.h"
#include "stm32f4xx_hal.h"
#include "radioenge_modem.h"
#include "SoilSensor.h"
#include "main.h"
#include <string.h>
#include <math.h>

extern osTimerId_t PeriodicSendTimerHandle;
extern osThreadId_t AppSendTaskHandle;
extern ADC_HandleTypeDef hadc1;
extern osEventFlagsId_t ModemStatusFlagsHandle;
extern TIM_HandleTypeDef htim1, htim3;
extern osMessageQueueId_t setPWMDutyQueueHandle;

typedef struct {
    uint32_t seq_no;
    int16_t umidade;
    int16_t temperatura;
    uint16_t ec;
    uint16_t ph;
    uint16_t nitrogenio;
    uint16_t fosforo; 
    uint16_t potassio; 
}__attribute__((packed))  LoRaSoilData_t;  

static LoRaSoilData_t dados_solo={
    .seq_no = 0,
    .umidade = 0,
    .temperatura = 0,
    .ec = 0,
    .ph = 0,
    .nitrogenio = 0,
    .fosforo = 0,
    .potassio = 0
};


void LoRaWAN_RxEventCallback(uint8_t *data, uint32_t length, uint32_t port, int32_t rssi, int32_t snr)
{
    osMessageQueuePut(setPWMDutyQueueHandle, data, 0U, 0U); 
}

void PeriodicSendTimerCallback(void *argument)
{
    osThreadFlagsSet(AppSendTaskHandle, 0x01);
}



void setPWMDutyTaskCode(void *argument)
{
    PUMP_CONTROLLER_OBJ_t *data,pwmData;
    osStatus_t pwmEvent; 
    while(1)
    {
        pwmEvent = osMessageQueueGet(setPWMDutyQueueHandle, &data, NULL, osWaitForever);   // wait for message
        memcpy(&pwmData,data,sizeof(PUMP_CONTROLLER_OBJ_t));
        if (pwmEvent == osOK)
        {
            htim1.Instance->CCR1 = (htim1.Instance->ARR * pwmData.irrigacao);
            
            htim3.Instance->CCR2 = (htim3.Instance->ARR * pwmData.irrigacao);

        }
    }
}


extern UART_HandleTypeDef huart2;
SoilSensor sensorDeSolo;

void SoilSensorTaskCode(void const * argument) {

    if (!SoilSensor_Init(&sensorDeSolo, &huart2)) {
        // Erro: Sensor não encontrado ou cabo solto
    }
    
    while (1) 
    {
        if (SoilSensor_ReadAllVariables(&sensorDeSolo)) {
            dados_solo.umidade     = sensorDeSolo.humidity;
            dados_solo.temperatura = sensorDeSolo.temperature;
            dados_solo.ec          = sensorDeSolo.ec;
            dados_solo.ph          = sensorDeSolo.ph;
            dados_solo.nitrogenio  = sensorDeSolo.nitrogen;
            dados_solo.fosforo     = sensorDeSolo.phosphorus;
            dados_solo.potassio    = sensorDeSolo.potassium;
        } else {
            // Falha na leitura
        }
        
        osDelay(5000); 
    }
}

void AppSendTaskCode(void *argument)
{
    /* USER CODE BEGIN 5 */
    /* Infinite loop */
    LoRaSoilData_t dados;
    osDelay(5000);
    dados.seq_no = 0;

    while (1){
        dados.umidade = dados_solo.umidade;
        dados.temperatura = dados_solo.temperatura;
        dados.ec = dados_solo.ec;
        dados.ph = dados_solo.ph;
        dados.nitrogenio = dados_solo.nitrogenio;
        dados.fosforo = dados_solo.fosforo;
        dados.potassio = dados_solo.potassio;
        dados.seq_no++;
        LoRaSendBNow(2, (uint8_t *)&dados, sizeof(LoRaSoilData_t));
        osDelay(30000);
    }
}