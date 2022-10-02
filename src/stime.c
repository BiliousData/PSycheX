/*
	This code was originally from a LUA script by MaxDev, and
	was translated to C, and modified for use with PSXFunkin by BiliousData.
*/

#include "stime.h"
#include "stage.h"

char *timer1[10];
char *timer2[10]; //I have absolutely no idea why, but don't remove this. Without it, the timer goes apeshit.

int timer_visual[2] = {0,0,}; //seconds and minutes.
Time time;

//Note from BiliousData: Unless I am stupid and misunderstanding something, the time has to be divided manually, or else it won't work for some reason.
//The time.converted_time can be found in stage.c, in a switch case as the game loads.

void SongTimer(void)
{

	if (time.timer_total < time.converted_time) //if we can still add,
	{
		time.timer_tick = time.timer_tick + 1; //As the game adds 1 to it every frame, we have to reset it as soon as it reaches 60.

		if (time.timer_tick == 60) //1 second.
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
		{
			sprintf(timer1, "%d:0%d", timer_visual[2], timer_visual[1]);
		}
		else
		{
			sprintf(timer1, "%d:%d", timer_visual[2], timer_visual[1]);
		}
		
		stage.font_cdr.draw(&stage.font_cdr, timer1, 149, 10, FontAlign_Left);
	}
	//else //but if finished,
	//{
	//	if (timer_visual[1] < 10) //Trying to avoid cases like "1:5"
	//		FntPrint("Timer's out at %d:0%d", timer_visual[2], timer_visual[1]); //Telling the player that the timer is over with a 0.
	//	else
	//		FntPrint("Timer's out at %d:%d", timer_visual[2], timer_visual[1]); //Telling the player that the timer is  over without a 0.
	//}

	//Voila!
	
}


//Same as above, but counting down
void SongTimerDown(void)
{

	if (time.timer_total < time.converted_time) //if we can still add,
	{
		time.timer_tick = time.timer_tick + 1; //As the game adds 1 to it every frame, we have to reset it as soon as it reaches 60.

		if (time.timer_tick == 60) //1 second.
		{
			time.timer_total = time.timer_total + 1; //adding 1 from total timer
			timer_visual[1] = timer_visual[1] - 1; //removing 1 from visual timer
			time.timer_tick = 0; //resetting our timer_tick.
		}

		if (timer_visual[1] == -1) //If visual timer's seconds is 60, we reset it and add 1 to visual minutes.
		{
			timer_visual[1] = 59;
			timer_visual[2] = timer_visual[2] - 1;
		}

		if (timer_visual[1] < 10) //Trying to avoid cases like "1:5"
		{
			sprintf(timer1, "%d:0%d", timer_visual[2], timer_visual[1]);
		}
		else
		{
			sprintf(timer1, "%d:%d", timer_visual[2], timer_visual[1]);
		}
		
		if (time.mindown == NULL)
		{}
		else
		{
			stage.font_cdr.draw(&stage.font_cdr, timer1, 149, 10, FontAlign_Left);

			//Draw time bar
			RECT time_fill = {0, 0, 0 + (128 * time.timer_total / time.converted_time), 8};
			RECT time_back = {0, 0, 128, 8};
			RECT_FIXED time_dst = {FIXED_DEC(-64,1), (SCREEN_HEIGHT2 - 230) << FIXED_SHIFT, 0, FIXED_DEC(8,1)};
			if (stage.downscroll)
				time_dst.y = -time_dst.y - time_dst.h;
			
			time_dst.w = time_fill.w << FIXED_SHIFT;
			Stage_DrawTexCol(&stage.tex_hud1, &time_fill, &time_dst, FIXED_DEC(1,1), 255, 255, 255); //white bar moving to the right
			time_dst.w = time_back.w << FIXED_SHIFT;
			Stage_DrawTexCol(&stage.tex_hud1, &time_back, &time_dst, FIXED_DEC(1,1), 0, 0, 0); //black background bar
		}
	}
	//else //but if finished,
	//{
	//	if (timer_visual[1] < 10) //Trying to avoid cases like "1:5"
	//		FntPrint("Timer's out at %d:0%d", timer_visual[2], timer_visual[1]); //Telling the player that the timer is over with a 0.
	//	else
	//		FntPrint("Timer's out at %d:%d", timer_visual[2], timer_visual[1]); //Telling the player that the timer is  over without a 0.
	//}

	//Voila!
	
}

void ResetTimer(void) //reset timer when song starts
{
	time.total_time = 0;
	time.timer_tick = 0;
	time.timer_total = 0;

	if (time.timeropt == CountDown)
	{
		timer_visual[1] = time.secdown;
		timer_visual[2] = time.mindown;
	}
	else
	{
		timer_visual[1] = 0;
		timer_visual[2] = 0;
	}
}