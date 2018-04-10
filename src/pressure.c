// обработчик давления
// pressure.c
// Рыбаков Д.Н.
//------------------------------------------------------------------------------

#include "pressure.h"

//------------------------------------------------------------------------------
#include "tasks.h"
#include "types.h"
#include "ledHandler.h"
#include "report.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "stm32f10x_lib.h"
#include <stdint.h>

//------------------------------------------------------------------------------

#define koef_emmission 1.008
#define base_offset 825/*+10*/

//------------------------------------------------------------------------------

u32 m_pressure = 0; // давление

//------------------------------------------------------------------------------

void adcInit(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef  ADC_InitStructure;

    //Enable ADC1 and GPIOA clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);

    // Configure PA.01 (ADC Channel1) as analog input -------------------------
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // ADC1 configuration ------------------------------------------------------
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE; // Single Channel
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // Scan on Demand
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    // ADC1 regular channel1 configuration
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_55Cycles5);

    // Enable ADC1
    ADC_Cmd(ADC1, ENABLE);

    // Enable ADC1 reset calibaration register
    ADC_ResetCalibration(ADC1);

    // Check the end of ADC1 reset calibration register
    while(ADC_GetResetCalibrationStatus(ADC1));

    // Start ADC1 calibaration
    ADC_StartCalibration(ADC1);

    // Check the end of ADC1 calibration
    while(ADC_GetCalibrationStatus(ADC1));

    // Start ADC1 Software Conversion
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

u32 get_adcValue(void)
{
    u32 adc_pressure = ADC_GetConversionValue(ADC1);

    // Probably overkill
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);

    // Start ADC1 Software Conversion
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);

    return adc_pressure;
}

/**
 * Антивыбрасывалка
 * @param val - текущее значение
 * @return - значение прошедщшее анивыбрасывалку
 */
u32 antiEmmisions (u32 val)
{
    static u32 prvVal[2];
    u32 aeval = prvVal[0];

    // проверка на выброс v0
    if ((prvVal[0] > val *koef_emmission)&&(prvVal[0] > prvVal[1] *koef_emmission))
    {
        _Report_Error("Emmission v: %d, 0: %d, 1: %d", val, prvVal[0], prvVal[1]);
        aeval = (val + prvVal[1])/2;
    }

    prvVal[1] = prvVal[0];
    prvVal[0] = val;

    return aeval;
}


/**
 * Усреднялка (временный код)
 * @param val
 * @param quan
 * @return
 */
u32 averageVal(u32 val, u32 quan)
{
    static u32 aver = 0;
    static u32 cnt = 0;
    static uint64_t sum = 0;

    ++cnt;
    sum += val;

    if (cnt >= quan)
    {
        aver = sum/cnt;
        cnt = 0;
        sum = 0;
    }

    return aver;
}

/**
 * Убиратель смещения
 * @param val
 * @return
 */
u32 antiOffset (u32 val)
{
    if (val >= base_offset)
    {
        val -= base_offset;
    }
    else
    {
        _Report_Warning("val < base_offset");
        val = 0;
    }
    return val;
}

typedef struct
{
    u32 adc;
    u32 pressure;
} press_level_t;

enum {press_levels_num = 16};
const press_level_t press_level[press_levels_num] =
{
    {.pressure = 110, .adc = 14},
    {.pressure = 120, .adc = 17},
    {.pressure = 130, .adc = 21},
    {.pressure = 140, .adc = 28},
    {.pressure = 150, .adc = 40},
    {.pressure = 160, .adc = 51},
    {.pressure = 170, .adc = 73},
    {.pressure = 180, .adc = 107},
    {.pressure = 190, .adc = 148},
    {.pressure = 200, .adc = 198},
    {.pressure = 210, .adc = 258},
    {.pressure = 220, .adc = 314},
    {.pressure = 230, .adc = 350},
    {.pressure = 240, .adc = 424},
    {.pressure = 250, .adc = 470},
    {.pressure = 260, .adc = 530},
};

/**
 * главная задача логики
 * @param pvParameters
 */
void pressureTask( void *pvParameters )
{
    NO_USE_PARAM(pvParameters);


    while(1)
    {
        u32 adc_pressure =  get_adcValue();

        float volt = ((float)adc_pressure/4096)*3.3;
        float pressure_kPa = (volt*50)/(40e-3*32.5)-2.61221084E+1;
        int pre_pressure_popugai = volt*1000 - 660;

        u32 mvolt = (u32)(volt * 1000);
        u32 mpa = (u32)(pressure_kPa*1000000);
        u32 po = pre_pressure_popugai;

        u32 ae = antiEmmisions (adc_pressure);
        u32 ao = antiOffset (ae); // TODO: добавить динамический расчет смещения
        u32 av = averageVal(ao, 10);

        u32 pressure = 0;
        u32 ind = 0;
        for (ind = 0; ind < press_levels_num; ind++)
        {
            if (press_level[ind].adc <= av)
                pressure = press_level[ind].pressure;
            else
                break;
        }
        
        if(pressure > 20)
          m_pressure = pressure - 20;
        else
          m_pressure = 0;

        Report_Debug("pr %d av %d", m_pressure, av);

        _Report_Debug("pr adc %d, ae %d, av %d, ao %d", adc_pressure, ae, av, ao);
        _Report_Debug("pr adc %d, mvolt %d, mpa %d, po %d, av %d", adc_pressure, mvolt, mpa, po, av);

        vTaskDelay(50);
    }

}

/**
 *
 */
void pressureHandler(void)
{
    adcInit(); // инициализация АЦП

    xTaskCreate( pressureTask, "pressure", configMINIMAL_STACK_SIZE, NULL, PRESSURE_PRIORITY, NULL );
}

//------------------------------------------------------------------------------
/**
 * Функция получения значения давления
 * @return Значение давления в мм.рт.ст.
 */
u32 getPressure(void)
{
    //TODO: сделать в критической секции
    return m_pressure;
}
