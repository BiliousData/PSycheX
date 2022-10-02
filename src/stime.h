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
	u8 timer_tick; //increases every frame, and since PSXFunkin is 60fps, logically, 60 ticks is a second.
	u8 timer_total; //Non-visual timer, counting in seconds
	s32 timeropt; //for choosing timer type
	//variables just for preparing for countdown
	u8 mindown; //minute
	u8 secdown; //second
} Time;

typedef enum
{
	CountDown,
	CountUp,
	Disabled,
} TimerOpt;

extern Time time;

void SongTimer(void);
void SongTimerDown(void);
void ResetTimer(void);


#endif