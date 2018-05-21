/*
 * File:   mainLogic.h
 * Author: DiNastja
 *
 * Created on 28 августа 2015 г., 11:09
 */
//------------------------------------------------------------------------------

#ifndef MAINLOGIC_H
#define  MAINLOGIC_H

#ifdef  __cplusplus
extern "C" {
#endif
//------------------------------------------------------------------------------

#define TIME_OF_CYKLE 2 // время цикла
#define SECOND_COEFF (1000/TIME_OF_CYKLE)
#define SECOND_TO_CYKLE(x) (x*SECOND_COEFF)

//------------------------------------------------------------------------------

typedef enum {pneumoCnl_1, pneumoCnl_2, pneumoCnl_3, pneumoCnl_4, pneumoCnl_num} pneumoCnl_t;
typedef enum {pumpState_Off, pumpState_On} pumpState_t;
typedef enum {pumpValveState_Open, pumpValveState_Close} pumpValveState_t;
typedef enum {reliefValveState_Open, reliefValveState_Close} reliefValveState_t;
typedef enum {channelState_PumpOut, channelState_PumpOn, channelState_Hold} channelState_t;

//------------------------------------------------------------------------------

void mainLogicHandler(void);

void PowerLavelUp (void);
void PowerLavelDown (void);
void StartStop (void);

//------------------------------------------------------------------------------
void setPneumoChannelState(pneumoCnl_t cnl, pumpState_t pumpState,
    pumpValveState_t pumpValveState, reliefValveState_t reliefValveState);
void setPneumoChannelPumpOn(pneumoCnl_t cnl);
void setPneumoChannelPumpOut(pneumoCnl_t cnl);
void setPneumoChannelHold(pneumoCnl_t cnl);
void setPneumoChannelStateTogether(pneumoCnl_t cnl, channelState_t state);

#ifdef  __cplusplus
}
#endif
//------------------------------------------------------------------------------

#endif  /* MAINLOGIC_H */

//------------------------------------------------------------------------------

