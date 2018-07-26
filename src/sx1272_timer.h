#ifndef _SX1272_TIMER_H_
#define _SX1272_TIMER_H_

#include <Arduino.h>

#ifdef __cplusplus
extern "C" {
#endif 

typedef void (*sx1272_timer_callback_t)(void);

typedef struct {
	sx1272_timer_callback_t callback;
	uint32_t lastMillis;
	uint32_t counter;
	uint8_t  run;
} sx1272_timer_t;

void TimerInit(sx1272_timer_t* timer, sx1272_timer_callback_t callback);
void TimerStop(sx1272_timer_t* timer);
void TimerStart(sx1272_timer_t* timer);
void TimerSetValue(sx1272_timer_t* timer, uint32_t value);

void TimerProcess(sx1272_timer_t* timer);

#ifdef __cplusplus
}
#endif 

#endif // _SX1272_TIMER_H_
