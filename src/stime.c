/*
	This code was originally from a LUA script by MaxDev, and
	was (probably badly) translated to C by BiliousData.
*/

#include "stime.h"


int timer_visual[2] = {0,0,}; //seconds and minutes.
Time time;

//Unless I am stupid and misunderstanding something, the time has to be divided manually, or else it won't work for some reason.

void SongTimer(void)
{
	if (time.timer_total < time.converted_time) //if we can still add,
	{
		time.timer_tick = time.timer_tick + 1; //As the game adds 1 to it every frame, we have to reset it as soon as it reaches 60.

		if (time.timer_tick == 60)
		{
			time.timer_total = time.timer_total + 1; //adding 1 to total timer
			timer_visual[1] = timer_visual[1] + 1; //adding 1 to visual timer
			time.timer_tick = 0; //resetting our timer_tick.
		}

		if (timer_visual[1] == 60) //If visual timer's seconds is 60, we reset it and add 1 to visual minutes.
		{
			timer_visual[1] = 0;
			timer_visual[2] = timer_visual[2] + 1;
		}

		if (timer_visual[1] < 10) //Trying to avoid cases like "1:5"
			FntPrint("%d:0%d", timer_visual[2], timer_visual[1]);
		else
			FntPrint("%d:%d", timer_visual[2], timer_visual[1]);
	}
	else //but if finished,
	{
		if (timer_visual[1] < 10) //Trying to avoid cases like "1:5"
			FntPrint("Timer's out at %d:0%d", timer_visual[2], timer_visual[1]);
		else
			FntPrint("Timer's out at %d:%d", timer_visual[2], timer_visual[1]);
	}

	//Voila!
	
}