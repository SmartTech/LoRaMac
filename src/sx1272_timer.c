#include "sx1272_timer.h"


void TimerInit(sx1272_timer_t* timer, sx1272_timer_callback_t callback)
{
	timer->run = false;
	timer->callback = callback;
}

void TimerStop(sx1272_timer_t* timer)
{
	timer->run = false;
}

void TimerStart(sx1272_timer_t* timer)
{
	timer->run = true;
	timer->lastMillis = millis();
}

void TimerSetValue(sx1272_timer_t* timer, uint32_t value)
{
	timer->counter = value;
}

void TimerProcess(sx1272_timer_t* timer)
{
	if(timer == NULL) return;
	if(!timer->run) return;
		
	if(timer->counter > 0)
	{
		if(millis() != timer->lastMillis)
		{
			timer->counter--;
			if(!timer->counter)
			{
				TimerStop(timer);
				if(timer->callback != NULL) timer->callback();
			}
			timer->lastMillis = millis();
		}
	}
	
}
