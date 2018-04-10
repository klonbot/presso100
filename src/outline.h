/*
 * File:   outline.h
 * Author: DiNastja
 *
 * Created on 2 сентября 2015 г., 21:21
 */

#ifndef OUTLINE_H
#define	OUTLINE_H

#ifdef	__cplusplus
extern "C" {
#endif

//------------------------------------------------------------------------------

typedef enum
{
    direct_logic = 0,
    invert_ligic = 1
} logic_t;

typedef struct
{
    GPIO_TypeDef* Port;     // Порт
    u16 Pin;                // Пин
    u32 Ix;                 // Индекс
    u32 State;              // Состояние
    u32 isNew;              // флаг нового состояния
    logic_t invertLogic;       // флаг инверсной логики
} outline_t;

/**
 * Инициализация светодиода
 * @param GPIOx
 * @param GPIO_Pin
 */
inline void outlineInit(GPIO_TypeDef* GPIOx, u16 GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin;
    GPIO_Init(GPIOx, &GPIO_InitStructure);
}

//------------------------------------------------------------------------------

#ifdef	__cplusplus
}
#endif

#endif	/* OUTLINE_H */

