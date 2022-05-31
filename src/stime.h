/*
	h file to accompany stime.c
	again, original LUA script by MaxDev
	and attempted translation by BiliousData
*/

#ifndef PSXF_GUARD_STIME_H
#define PSXF_GUARD_STIME_H

#include "psx.h"
#include "fixed.h"

typedef struct
{
	u8 total_time; //in centiseconds
	u8 converted_time; //from centiseconds to seconds
	u8 timer_tick; //???
	u8 timer_total; //Non-visual timer, counting in seconds
} Time;

extern Time time;

void TimerPrep(void);
void SongTimer(void);


#endif