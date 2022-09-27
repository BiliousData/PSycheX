/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "demu.h"
#include "menu.h"

#include "mem.h"
#include "main.h"
#include "timer.h"
#include "io.h"
#include "gfx.h"
#include "audio.h"
#include "pad.h"
#include "archive.h"
#include "mutil.h"
#include "network.h"

#include "random.h"

#include "font.h"
#include "trans.h"
#include "loadscr.h"

#include "stage.h"
#include "character/titlegf.h"

//Demu messages
static const char *funny_messages[][2] = {
	{"I GOTTA", "BUCKET OF CHICKEN"},
	{"WE DO A LITTLE TROLLING", "H"},
	{"LITTLE HEROBRINE", "IM CUMMING IN YOUR MOM"},
	{"USE A CONTROLLER", "LOL"},
	{"PSXFUNKIN BY CUCKYDEV", "SUCK IT DOWN"},
	{"KBHGAMES", "STEAL MODS"}, //but not psx ports LOL
	{"THIS ISNT BASED ON KE", "VINE BOOM"},
	{"HES NAMED EVILSK8ER", "NOT EVILSKER"},
	{"ITS NERF", "OR NOTHING"},
	{"PENIS", "BALLS EVEN"},
	{"FNF IS NOT FOR KIDS", "GO OUTSIDE"},
	{"WHAT YOU KNOW ABOUT", "ROLLING DOWN IN THE DEEP"},
	{"DREAM STANS", "LITERALLY SCARE ME"},
	{"WEEG MOD", "CHECK IT OUT"}, //even though it's not ported
	{"WHAT DA DOG", "DOIN"},
	{"GO MANGO GO", "GO MANGO GO"},
	{"GO PICO YEAH YEAH", "TWERKS CUTELY"},
	{"WASH YOUR HANDS", "STAY SAFE BRO"},
	{"WEAR A MASK", "EVEN IF VACCINATED"},
	{"YOU DONT UNDERSTAND", "I HATE SOCIETY"},
	{"PSXFUNKIN PORT", "FATHERLESS BEHAVIOR"},
	{"I MADE A PSYCH PORT", "FREE CLOUT YAY"},
	{"WANNA WORK ON MY FNF", "FNF PORT"},
};

#ifdef PSXF_NETWORK

//Demu string type
#define MENUSTR_CHARS 0x20
typedef char DemuStr[MENUSTR_CHARS + 1];

#endif

//Demu state
static struct
{
	//Demu state
	u8 page, next_page;
	boolean page_swap;
	u8 select, next_select;
	
	fixed_t scroll;
	fixed_t trans_time;
	
	//Page specific state
	union
	{
		struct
		{
			u8 funny_message;
		} opening;
		struct
		{
			fixed_t logo_bump;
			fixed_t fade, fadespd;
		} title;
		struct
		{
			fixed_t fade, fadespd;
		} story;
		struct
		{
			fixed_t back_r, back_g, back_b;
		} freeplay;
	#ifdef PSXF_NETWORK
		struct
		{
			boolean type;
			DemuStr port;
			DemuStr pass;
		} net_host;
		struct
		{
			boolean type;
			DemuStr ip;
			DemuStr port;
			DemuStr pass;
		} net_join;
		struct
		{
			boolean swap;
		} net_op;
	#endif
	} page_state;
	
	union
	{
		struct
		{
			u8 id, diff;
			boolean story;
		} stage;
	} page_param;
	
	//Demu assets
	Gfx_Tex tex_back, tex_ng, tex_story, tex_title;
	FontData font_bold, font_arial;
	
	Character *Titlepsy; //Title psychic

} demu;

#ifdef PSXF_NETWORK

//Demu string functions
static void DemuStr_Process(DemuStr *this, s32 x, s32 y, const char *fmt, boolean select, boolean type)
{
	//Append typed input
	if (select && type)
	{
		if (pad_type[0] != '\0')
			strncat(*this, pad_type, MENUSTR_CHARS - strlen(*this));
		if (pad_backspace)
		{
			size_t i = strlen(*this);
			if (i != 0)
				(*this)[i - 1] = '\0';
		}
	}
	
	//Get text to draw
	char buf[0x100];
	sprintf(buf, fmt, *this);
	if (select && type && (animf_count & 2))
		strcat(buf, "_");
	
	//Draw text
	demu.font_arial.draw_col(&demu.font_arial, buf, x, y, FontAlign_Left, 0x80, 0x80, select ? 0x00 : 0x80);
	demu.font_arial.draw_col(&demu.font_arial, buf, x+1, y+1, FontAlign_Left, 0x00, 0x00, 0x00);
}

#endif


//Internal demu functions
char demu_text_buffer[0x100];

static const char *Demu_LowerIf(const char *text, boolean lower)
{
	//Copy text
	char *dstp = demu_text_buffer;
	if (lower)
	{
		for (const char *srcp = text; *srcp != '\0'; srcp++)
		{
			if (*srcp >= 'A' && *srcp <= 'Z')
				*dstp++ = *srcp | 0x20;
			else
				*dstp++ = *srcp;
		}
	}
	else
	{
		for (const char *srcp = text; *srcp != '\0'; srcp++)
		{
			if (*srcp >= 'a' && *srcp <= 'z')
				*dstp++ = *srcp & ~0x20;
			else
				*dstp++ = *srcp;
		}
	}
	
	//Terminate text
	*dstp++ = '\0';
	return demu_text_buffer;
}

static void Demu_DrawBack(boolean flash, s32 scroll, u8 r0, u8 g0, u8 b0, u8 r1, u8 g1, u8 b1)
{
	RECT back_src = {0, 0, 255, 255};
	RECT back_dst = {0, -scroll - SCREEN_WIDEADD2, SCREEN_WIDTH, SCREEN_WIDTH * 4 / 5};
	

   if (flash || (animf_count & 4) == 0)
   	Gfx_DrawTexCol(&demu.tex_back, &back_src, &back_dst, r0, g0, b0);
   else
   	Gfx_DrawTexCol(&demu.tex_back, &back_src, &back_dst, r1, g1, b1);

}

static void Demu_DifficultySelector(s32 x, s32 y)
{
	//Change difficulty
	if (demu.next_page == demu.page && Trans_Idle())
	{
		if (pad_state.press & PAD_LEFT)
		{
			if (demu.page_param.stage.diff > StageDiff_Easy)
				demu.page_param.stage.diff--;
			else
				demu.page_param.stage.diff = StageDiff_Hard;
		}
		if (pad_state.press & PAD_RIGHT)
		{
			if (demu.page_param.stage.diff < StageDiff_Hard)
				demu.page_param.stage.diff++;
			else
				demu.page_param.stage.diff = StageDiff_Easy;
		}
	}
	
	//Draw difficulty arrows
	static const RECT arrow_src[2][2] = {
		{{224, 64, 16, 32}, {224, 96, 16, 32}}, //left
		{{240, 64, 16, 32}, {240, 96, 16, 32}}, //right
	};
	
	Gfx_BlitTex(&demu.tex_story, &arrow_src[0][(pad_state.held & PAD_LEFT) != 0], x - 40 - 16, y - 16);
	Gfx_BlitTex(&demu.tex_story, &arrow_src[1][(pad_state.held & PAD_RIGHT) != 0], x + 40, y - 16);
	
	//Draw difficulty
	static const RECT diff_srcs[] = {
		{  0, 96, 64, 18},
		{ 64, 96, 80, 18},
		{144, 96, 64, 18},
	};
	
	const RECT *diff_src = &diff_srcs[demu.page_param.stage.diff];
	Gfx_BlitTex(&demu.tex_story, diff_src, x - (diff_src->w >> 1), y - 9 + ((pad_state.press & (PAD_LEFT | PAD_RIGHT)) != 0));
}

static void Demu_DrawWeek(const char *week, s32 x, s32 y)
{
	//Draw label
	if (week == NULL)
	{
		//Tutorial
		RECT label_src = {0, 0, 112, 32};
		Gfx_BlitTex(&demu.tex_story, &label_src, x, y);
	}
	else
	{
		//Week
		RECT label_src = {0, 32, 80, 32};
		Gfx_BlitTex(&demu.tex_story, &label_src, x, y);
		
		//Number
		x += 80;
		for (; *week != '\0'; week++)
		{
			//Draw number
			u8 i = *week - '0';
			
			RECT num_src = {128 + ((i & 3) << 5), ((i >> 2) << 5), 32, 32};
			Gfx_BlitTex(&demu.tex_story, &num_src, x, y);
			x += 32;
		}
	}
}

//Demu functions
void Demu_Load(DemuPage page)
{
	//Load demu assets
	IO_Data demo_arc = IO_Read("\\MENU\\DEMO.ARC;1");
	Gfx_LoadTex(&demu.tex_back,  Archive_Find(demo_arc, "back.tim"),  0);
	Gfx_LoadTex(&demu.tex_ng,    Archive_Find(demo_arc, "ng.tim"),    0);
	Gfx_LoadTex(&demu.tex_story, Archive_Find(demo_arc, "story.tim"), 0);
	Gfx_LoadTex(&demu.tex_title, Archive_Find(demo_arc, "title.tim"), 0);
	Mem_Free(demo_arc);
	
	FontData_Load(&demu.font_bold, Font_Bold);
	FontData_Load(&demu.font_arial, Font_Arial);
	
	demu.Titlepsy = Char_TitleGF_New(FIXED_DEC(62,1), FIXED_DEC(-12,1));
	stage.camera.x = stage.camera.y = FIXED_DEC(0,1);
	stage.camera.bzoom = FIXED_UNIT;
	stage.gf_speed = 4;
	
	//Initialize demu state
	demu.select = demu.next_select = 0;
	
	switch (demu.page = demu.next_page = page)
	{
		case DemuPage_Opening:
			//Get funny message to use
			//Do this here so timing is less reliant on VSync
			#ifdef PSXF_PC
				demu.page_state.opening.funny_message = time(NULL) % COUNT_OF(funny_messages);
			#else
				demu.page_state.opening.funny_message = ((*((volatile u32*)0xBF801120)) >> 3) % COUNT_OF(funny_messages); //sysclk seeding
			#endif
			break;
		default:
			break;
	}
	demu.page_swap = true;
	
	demu.trans_time = 0;
	Trans_Clear();
	
	stage.song_step = 0;
	
	//Play demu music
	Audio_PlayXA_Track(XA_Freeky, 0x40, 0, 1);
	Audio_WaitPlayXA();
	
	//Set background colour
	Gfx_SetClear(0, 0, 0);
}

void Demu_Unload(void)
{
	//Free title Girlfriend
	Character_Free(demu.Titlepsy);
}

void Demu_ToStage(StageId id, StageDiff diff, boolean story)
{
	demu.next_page = DemuPage_Stage;
	demu.page_param.stage.id = id;
	demu.page_param.stage.story = story;
	demu.page_param.stage.diff = diff;
	Trans_Start();
}

void Demu_Tick(void)
{
	//Clear per-frame flags
	stage.flag &= ~STAGE_FLAG_JUST_STEP;
	
	//Get song position
	u16 next_step = Audio_TellXA_Milli() / 147; //100 BPM
	if (next_step != stage.song_step)
	{
		if (next_step >= stage.song_step)
			stage.flag |= STAGE_FLAG_JUST_STEP;
		stage.song_step = next_step;
	}
	
	//Handle transition out
	if (Trans_Tick())
	{
		//Change to set next page
		demu.page_swap = true;
		demu.page = demu.next_page;
		demu.select = demu.next_select;
	}
	
	//Tick demu page
	DemuPage exec_page;
	switch (exec_page = demu.page)
	{
		case DemuPage_Opening:
		{
			u16 beat = stage.song_step >> 2;
			
			//Start title screen if opening ended
			if (beat >= 12)
			{
				demu.page = demu.next_page = DemuPage_Title;
				demu.page_swap = true;
				//Fallthrough
			}
			else
			{
				//Start title screen if start pressed
				if (pad_state.held & PAD_START)
					demu.page = demu.next_page = DemuPage_Title;
				
				//Draw different text depending on beat
				RECT src_ng = {0, 0, 212, 132};
				const char **funny_message = funny_messages[demu.page_state.opening.funny_message];
				
				switch (beat)
				{
					
					case 3:
						demu.font_bold.draw(&demu.font_bold, " ",    SCREEN_WIDTH2, SCREEN_HEIGHT2 - 32, FontAlign_Center);
						Gfx_BlitTex(&demu.tex_ng, &src_ng, (SCREEN_WIDTH - 212) >> 1, SCREEN_HEIGHT2 - 32);
				//Fallthrough
					case 1:
					case 2:
						demu.font_bold.draw(&demu.font_bold, "RHYTHM GAME", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 64, FontAlign_Center);
						demu.font_bold.draw(&demu.font_bold, " ",           SCREEN_WIDTH2, SCREEN_HEIGHT2 - 48, FontAlign_Center);
						break;
					
					case 7:
						demu.font_bold.draw(&demu.font_bold, funny_message[1], SCREEN_WIDTH2, SCREEN_HEIGHT2, FontAlign_Center);
				//Fallthrough
					case 6:
					case 5:
						demu.font_bold.draw(&demu.font_bold, funny_message[0], SCREEN_WIDTH2, SCREEN_HEIGHT2 - 16, FontAlign_Center);
						break;
					
					case 11:
						demu.font_bold.draw(&demu.font_bold, "FUNKIN", SCREEN_WIDTH2, SCREEN_HEIGHT2 + 8, FontAlign_Center);
				//Fallthrough
					case 10:
						demu.font_bold.draw(&demu.font_bold, "NIGHT", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 8, FontAlign_Center);
				//Fallthrough
					case 9:
						demu.font_bold.draw(&demu.font_bold, "FRIDAY", SCREEN_WIDTH2, SCREEN_HEIGHT2 - 24, FontAlign_Center);
						break;
				}
				break;
			}
		}
	//Fallthrough
		case DemuPage_Title:
		{
			//Initialize page
			if (demu.page_swap)
			{
				demu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 20) - 1;
				demu.page_state.title.fade = FIXED_DEC(255,1);
				demu.page_state.title.fadespd = FIXED_DEC(90,1);
			}
			
			//Draw white fade
			if (demu.page_state.title.fade > 0)
			{
				static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
				u8 flash_col = demu.page_state.title.fade >> FIXED_SHIFT;
				Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
				demu.page_state.title.fade -= FIXED_MUL(demu.page_state.title.fadespd, timer_dt);
			}
			
			//Go to main demu when start is pressed
			if (demu.trans_time > 0 && (demu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if ((pad_state.press & PAD_START) && demu.next_page == demu.page && Trans_Idle())
			{
				demu.trans_time = FIXED_UNIT;
				demu.page_state.title.fade = FIXED_DEC(255,1);
				demu.page_state.title.fadespd = FIXED_DEC(300,1);
				demu.next_page = DemuPage_Main;
				demu.next_select = 0;
			}

			if (pad_state.press & PAD_R1)
			{
				stage.demo = 0;
				demu.next_page = DemuPage_Update;
				demu.trans_time = FIXED_UNIT;
				demu.page_state.title.fade = FIXED_DEC(255,1);
				demu.page_state.title.fadespd = FIXED_DEC(300,1);
			}
			
			//Draw Friday Night Funkin' logo
			if ((stage.flag & STAGE_FLAG_JUST_STEP) && (stage.song_step & 0x3) == 0 && demu.page_state.title.logo_bump == 0)
				demu.page_state.title.logo_bump = (FIXED_DEC(7,1) / 24) - 1;
			
			static const fixed_t logo_scales[] = {
				FIXED_DEC(1,1),
				FIXED_DEC(101,100),
				FIXED_DEC(102,100),
				FIXED_DEC(103,100),
				FIXED_DEC(105,100),
				FIXED_DEC(110,100),
				FIXED_DEC(97,100),
			};
			fixed_t logo_scale = logo_scales[(demu.page_state.title.logo_bump * 24) >> FIXED_SHIFT];
			u32 x_rad = (logo_scale * (176 >> 1)) >> FIXED_SHIFT;
			u32 y_rad = (logo_scale * (112 >> 1)) >> FIXED_SHIFT;
			
			RECT logo_src = {0, 0, 176, 112};
			RECT logo_dst = {
				100 - x_rad + (SCREEN_WIDEADD2 >> 1),
				68 - y_rad,
				x_rad << 1,
				y_rad << 1
			};
			Gfx_DrawTex(&demu.tex_title, &logo_src, &logo_dst);
			
			if (demu.page_state.title.logo_bump > 0)
				if ((demu.page_state.title.logo_bump -= timer_dt) < 0)
					demu.page_state.title.logo_bump = 0;
			
			//Draw "Press Start to Begin"
			if (demu.next_page == demu.page)
			{
				//Blinking blue
				s16 press_lerp = (MUtil_Cos(animf_count << 3) + 0x100) >> 1;
				u8 press_r = 51 >> 1;
				u8 press_g = (58  + ((press_lerp * (255 - 58))  >> 8)) >> 1;
				u8 press_b = (206 + ((press_lerp * (255 - 206)) >> 8)) >> 1;
				
				RECT press_src = {0, 112, 256, 32};
				Gfx_BlitTexCol(&demu.tex_title, &press_src, (SCREEN_WIDTH - 256) / 2, SCREEN_HEIGHT - 48, press_r, press_g, press_b);
			}
			else
			{
				//Flash white
				RECT press_src = {0, (animf_count & 1) ? 144 : 112, 256, 32};
				Gfx_BlitTex(&demu.tex_title, &press_src, (SCREEN_WIDTH - 256) / 2, SCREEN_HEIGHT - 48);
			}
			
			//Draw Girlfriend
			demu.Titlepsy->tick(demu.Titlepsy);
			break;
		}
		case DemuPage_Main:
		{
			static const char *demu_options[] = {
				"STORY MODE",
				"FREEPLAY",
				"CREDITS",
				"OPTIONS",
				#ifdef PSXF_NETWORK
					"JOIN SERVER",
					"HOST SERVER",
				#endif
			};
			
			//Initialize page
			if (demu.page_swap)
				demu.scroll = demu.select *
				#ifndef PSXF_NETWORK
					FIXED_DEC(8,1);
				#else
					FIXED_DEC(12,1);
				#endif
			

			if (demu.page_swap)
			{
				stage.backpick = RandomRange(0, 2);
			}
			
			//Draw version identification
			demu.font_bold.draw(&demu.font_bold,
				"PSX PSYCHIC",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Handle option and selection
			if (demu.trans_time > 0 && (demu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if (demu.next_page == demu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (demu.select > 0)
						demu.select--;
					else
						demu.select = COUNT_OF(demu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (demu.select < COUNT_OF(demu_options) - 1)
						demu.select++;
					else
						demu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					switch (demu.select)
					{
						case 0: //Story Mode
							demu.next_page = DemuPage_Story;
							break;
						case 1: //Freeplay
							demu.next_page = DemuPage_Freeplay;
							break;
						case 2: //Mods
							demu.next_page = DemuPage_Credits;
							break;
						case 3: //Options
							demu.next_page = DemuPage_Options;
							break;
					#ifdef PSXF_NETWORK
						case 4: //Join Server
							demu.next_page = Network_Inited() ? DemuPage_NetJoin : DemuPage_NetInitFail;
							break;
						case 5: //Host Server
							demu.next_page = Network_Inited() ? DemuPage_NetHost : DemuPage_NetInitFail;
							break;
					#endif
					}
					demu.next_select = 0;
					demu.trans_time = FIXED_UNIT;
				}
				
				//Return to title screen if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					demu.next_page = DemuPage_Title;
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = demu.select *
			#ifndef PSXF_NETWORK
				FIXED_DEC(8,1);
			#else
				FIXED_DEC(12,1);
			#endif
			demu.scroll += (next_scroll - demu.scroll) >> 2;
			
			if (demu.next_page == demu.page || demu.next_page == DemuPage_Title)
			{
				//Draw all options
				for (u8 i = 0; i < COUNT_OF(demu_options); i++)
				{
					demu.font_bold.draw(&demu.font_bold,
						Demu_LowerIf(demu_options[i], demu.select != i),
						SCREEN_WIDTH2,
						SCREEN_HEIGHT2 + (i << 5) - 48 - (demu.scroll >> FIXED_SHIFT),
						FontAlign_Center
					);
				}
			}
			else if (animf_count & 2)
			{
				//Draw selected option
				demu.font_bold.draw(&demu.font_bold,
					demu_options[demu.select],
					SCREEN_WIDTH2,
					SCREEN_HEIGHT2 + (demu.select << 5) - 48 - (demu.scroll >> FIXED_SHIFT),
					FontAlign_Center
				);
			}
			
			//Draw background
			
			Demu_DrawBack(
				demu.next_page == demu.page || demu.next_page == DemuPage_Title,
			#ifndef PSXF_NETWORK
				demu.scroll >> (FIXED_SHIFT + 1),
			#else
				demu.scroll >> (FIXED_SHIFT + 3),
			#endif
				253 >> 1, 231 >> 1, 113 >> 1,
				253 >> 1, 113 >> 1, 155 >> 1
				
			);
			

			break;
		}
		case DemuPage_Story:
		{
			static const struct
			{
				const char *week;
				StageId stage;
				const char *name;
				const char *tracks[3];
			} demu_options[] = {
				{"1", StageId_2_1, "MIND GAMES", {"PSYCHIC", NULL, NULL}},
			};
			
			//Initialize page
			if (demu.page_swap)
			{
				demu.scroll = 0;
				demu.page_param.stage.diff = StageDiff_Normal;
				demu.page_state.title.fade = FIXED_DEC(0,1);
				demu.page_state.title.fadespd = FIXED_DEC(0,1);
				stage.backpick = RandomRange(0, 2);
			}
			
			//Draw white fade
			if (demu.page_state.title.fade > 0)
			{
				static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
				u8 flash_col = demu.page_state.title.fade >> FIXED_SHIFT;
				Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
				demu.page_state.title.fade -= FIXED_MUL(demu.page_state.title.fadespd, timer_dt);
			}
			
			//Draw difficulty selector
			Demu_DifficultySelector(SCREEN_WIDTH - 75, 80);
			
			//Handle option and selection
			if (demu.trans_time > 0 && (demu.trans_time -= timer_dt) <= 0)
				Trans_Start();
			
			if (demu.next_page == demu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (demu.select > 0)
						demu.select--;
					else
						demu.select = COUNT_OF(demu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (demu.select < COUNT_OF(demu_options) - 1)
						demu.select++;
					else
						demu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					demu.next_page = DemuPage_Stage;
					demu.page_param.stage.id = demu_options[demu.select].stage;
					demu.page_param.stage.story = true;
					demu.trans_time = FIXED_UNIT;
					demu.page_state.title.fade = FIXED_DEC(255,1);
					demu.page_state.title.fadespd = FIXED_DEC(510,1);
				}
				
				//Return to main demu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					demu.next_page = DemuPage_Main;
					demu.next_select = 0; //Story Mode
					Trans_Start();
				}
			}
			
			//Draw week name and tracks
			demu.font_bold.draw(&demu.font_bold,
				demu_options[demu.select].name,
				SCREEN_WIDTH - 16,
				24,
				FontAlign_Right
			);
			
			const char * const *trackp = demu_options[demu.select].tracks;
			for (size_t i = 0; i < COUNT_OF(demu_options[demu.select].tracks); i++, trackp++)
			{
				if (*trackp != NULL)
					demu.font_bold.draw(&demu.font_bold,
						*trackp,
						SCREEN_WIDTH - 16,
						SCREEN_HEIGHT - (4 * 24) + (i * 24),
						FontAlign_Right
					);
			}
			
			//Draw upper strip
			RECT name_bar = {0, 16, SCREEN_WIDTH, 32};
			Gfx_DrawRect(&name_bar, 249, 207, 81);
			
			//Draw options
			s32 next_scroll = demu.select * FIXED_DEC(48,1);
			demu.scroll += (next_scroll - demu.scroll) >> 3;
			
			if (demu.next_page == demu.page || demu.next_page == DemuPage_Main)
			{
				//Draw all options
				for (u8 i = 0; i < COUNT_OF(demu_options); i++)
				{
					s32 y = 64 + (i * 48) - (demu.scroll >> FIXED_SHIFT);
					if (y <= 16)
						continue;
					if (y >= SCREEN_HEIGHT)
						break;
					Demu_DrawWeek(demu_options[i].week, 48, y);
				}
			}
			else if (animf_count & 2)
			{
				//Draw selected option
				Demu_DrawWeek(demu_options[demu.select].week, 48, 64 + (demu.select * 48) - (demu.scroll >> FIXED_SHIFT));
			}
			
			break;
		}
		case DemuPage_Freeplay:
		{
			static const struct
			{
				StageId stage;
				u32 col;
				const char *text;
			} demu_options[] = {
				//{StageId_4_4, 0xFFFC96D7, "TEST"},
				{StageId_2_1, 0x510088, "PSYCHIC"},
				{StageId_2_2, 0x510088, "LATE DRIVE"},
			};
			
			//Initialize page
			if (demu.page_swap)
			{
				demu.scroll = COUNT_OF(demu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
				demu.page_param.stage.diff = StageDiff_Normal;
				demu.page_state.freeplay.back_r = FIXED_DEC(255,1);
				demu.page_state.freeplay.back_g = FIXED_DEC(255,1);
				demu.page_state.freeplay.back_b = FIXED_DEC(255,1);
				stage.backpick = RandomRange(0, 2);
			}
			
			//Draw page label
			demu.font_bold.draw(&demu.font_bold,
				"FREEPLAY",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Draw difficulty selector
			Demu_DifficultySelector(SCREEN_WIDTH - 100, SCREEN_HEIGHT2 - 48);
			
			//Handle option and selection
			if (demu.next_page == demu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (demu.select > 0)
						demu.select--;
					else
						demu.select = COUNT_OF(demu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (demu.select < COUNT_OF(demu_options) - 1)
						demu.select++;
					else
						demu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					demu.next_page = DemuPage_Stage;
					demu.page_param.stage.id = demu_options[demu.select].stage;
					demu.page_param.stage.story = false;
					Trans_Start();
				}
				
				//Return to main demu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					demu.next_page = DemuPage_Main;
					demu.next_select = 1; //Freeplay
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = demu.select * FIXED_DEC(24,1);
			demu.scroll += (next_scroll - demu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(demu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (demu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				demu.font_bold.draw(&demu.font_bold,
					Demu_LowerIf(demu_options[i].text, demu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			fixed_t tgt_r = (fixed_t)((demu_options[demu.select].col >> 16) & 0xFF) << FIXED_SHIFT;
			fixed_t tgt_g = (fixed_t)((demu_options[demu.select].col >>  8) & 0xFF) << FIXED_SHIFT;
			fixed_t tgt_b = (fixed_t)((demu_options[demu.select].col >>  0) & 0xFF) << FIXED_SHIFT;
			
			demu.page_state.freeplay.back_r += (tgt_r - demu.page_state.freeplay.back_r) >> 4;
			demu.page_state.freeplay.back_g += (tgt_g - demu.page_state.freeplay.back_g) >> 4;
			demu.page_state.freeplay.back_b += (tgt_b - demu.page_state.freeplay.back_b) >> 4;
			
			Demu_DrawBack(
				true,
				8,
				demu.page_state.freeplay.back_r >> (FIXED_SHIFT + 1),
				demu.page_state.freeplay.back_g >> (FIXED_SHIFT + 1),
				demu.page_state.freeplay.back_b >> (FIXED_SHIFT + 1),
				0, 0, 0
			);
			break;
		}
		case DemuPage_Credits:
		{
                       static const struct
			{
				StageId stage;
				u32 col;
				const char *text;
			} demu_options[] = {
				//{StageId_4_4, 0xFFFC96D7, "TEST"},
				{StageId_1_4, 0xFF9B30F3,"OG MIND GAMES CREW"},
				{StageId_1_4, 0xFFb4007b, " "},
				{StageId_1_4, 0xFFb4007b,"RIVEROAKEN"},
				{StageId_1_4, 0xFFb4007b,"PROJECT LEAD AND"},
				{StageId_1_4, 0xFFb4007b,"ANIMATOR"},
				{StageId_1_4, 0xFFb4007b,"WAIT I MADE THIS"},
				{StageId_1_4, 0xFF3f3f3f, " "},
				{StageId_1_4, 0xFFb644cb,"IFLICKY"},
				{StageId_1_4, 0xFFb644cb,"MAIN COMPOSER"},
				{StageId_1_4, 0xFFb644cb,"BERR DIG A NERR NERR"},
				{StageId_1_4, 0xFFb644cb, " "},
				{StageId_1_4, 0xffff33,"SHADOW MARIO"},
				{StageId_1_4, 0xffff33,"MAIN PROGRAMMER"},
				{StageId_1_4, 0xffff33,"WIKIHOW HOW TO"},
				{StageId_1_4, 0xffff33,"HANDLE FAME"},
				{StageId_1_1, 0xFF2fecec, " "},
				{StageId_1_4, 0xFF2fecec,"SALVATI"},
				{StageId_1_4, 0xFF2fecec,"PSYCHICS SOUNDFONT"},
				{StageId_1_4, 0xFF2fecec,"CREATOR"},
				{StageId_1_4, 0xFF2fecec,"BOTTOM TEXT"},
				{StageId_1_1, 0xFFae4c20, " "},
				{StageId_1_4, 0xFFae4c20,"SLABRACCOON"},
				{StageId_1_4, 0xFFae4c20,"EMOTIONAL SUPPORT RACCOON"},
				{StageId_1_4, 0xFFae4c20,"FERAL GROWLING"},
				{StageId_1_4, 0xFF4b5de4, " "},
				{StageId_1_4, 0xFF4b5de4,"JOIN OUR DISCORD!"},
				{StageId_1_4, 0xFF4b5de4,"IF YOU DARE..."},
				{StageId_1_4, 0xFF9B30F3, " "},
				{StageId_1_1, 0xFF9B30F3, "MIND GAMES PSX BY"},
				{StageId_1_1, 0xFF9B30F3, " "},
				{StageId_1_2, 0xFF9B30F3, "NINTENDOBRO"},
				{StageId_1_2, 0xFF9B30F3, "CHARTED"},
				{StageId_1_2, 0xFF9B30F3, "MENU ASSETS"},
				{StageId_1_3, 0xFF9B30F3, "LATE DRIVE BG"},
				{StageId_1_4, 0xFF9B30F3, " "},
				{StageId_1_3, 0xFF9B30F3, "BILIOUSDATA"},
				{StageId_1_3, 0xFF9B30F3, "BACKGROUNDS"},
				{StageId_1_3, 0xFF9B30F3, "PSYCHIC"},
				{StageId_1_3, 0xFF9B30F3, "ARROW MOVEMENT"},
				{StageId_1_3, 0xFF9B30F3, "CODE"},
				{StageId_1_4, 0xFF9B30F3, " "},
				{StageId_1_3, 0xFF9B30F3, "MRRUMBLEROSES"},
				{StageId_1_3, 0xFF9B30F3, "GOT PERMISSION"},
				{StageId_1_4, 0xFF9B30F3, " "},
				{StageId_1_4, 0xFF9B30F3, "PSX FUNKIN BY"},
				{StageId_1_4, 0xFF9B30F3, "CUCKYDEV"},


			};
			
			//Initialize page
			if (demu.page_swap)
			{
				demu.scroll = COUNT_OF(demu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
				demu.page_param.stage.diff = StageDiff_Normal;
				demu.page_state.freeplay.back_r = FIXED_DEC(255,1);
				demu.page_state.freeplay.back_g = FIXED_DEC(255,1);
				demu.page_state.freeplay.back_b = FIXED_DEC(255,1);
				stage.backpick = RandomRange(0, 2);
			}
			
			//Draw page label
			demu.font_bold.draw(&demu.font_bold,
				"CREDITS",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Handle option and selection
			if (demu.next_page == demu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (demu.select > 0)
						demu.select--;
					else
						demu.select = COUNT_OF(demu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (demu.select < COUNT_OF(demu_options) - 1)
						demu.select++;
					else
						demu.select = 0;
				}
				
				//Return to main demu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					demu.next_page = DemuPage_Main;
					demu.next_select = 2; //Freeplay
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = demu.select * FIXED_DEC(24,1);
			demu.scroll += (next_scroll - demu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(demu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (demu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				demu.font_bold.draw(&demu.font_bold,
					Demu_LowerIf(demu_options[i].text, demu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			fixed_t tgt_r = (fixed_t)((demu_options[demu.select].col >> 16) & 0xFF) << FIXED_SHIFT;
			fixed_t tgt_g = (fixed_t)((demu_options[demu.select].col >>  8) & 0xFF) << FIXED_SHIFT;
			fixed_t tgt_b = (fixed_t)((demu_options[demu.select].col >>  0) & 0xFF) << FIXED_SHIFT;
			
			demu.page_state.freeplay.back_r += (tgt_r - demu.page_state.freeplay.back_r) >> 4;
			demu.page_state.freeplay.back_g += (tgt_g - demu.page_state.freeplay.back_g) >> 4;
			demu.page_state.freeplay.back_b += (tgt_b - demu.page_state.freeplay.back_b) >> 4;
			
			Demu_DrawBack(
				true,
				8,
				demu.page_state.freeplay.back_r >> (FIXED_SHIFT + 1),
				demu.page_state.freeplay.back_g >> (FIXED_SHIFT + 1),
				demu.page_state.freeplay.back_b >> (FIXED_SHIFT + 1),
				0, 0, 0
			);
			break;
		}
		case DemuPage_Options:
		{
			static const char *gamemode_strs[] = {"NORMAL", "SWAP", "TWO PLAYER"};
			static const struct
			{
				enum
				{
					OptType_Boolean,
					OptType_Enum,
				} type;
				const char *text;
				void *value;
				union
				{
					struct
					{
						int a;
					} spec_boolean;
					struct
					{
						s32 max;
						const char **strs;
					} spec_enum;
				} spec;
			} demu_options[] = {
				{OptType_Enum,    "GAMEMODE", &stage.mode, {.spec_enum = {COUNT_OF(gamemode_strs), gamemode_strs}}},
				//{OptType_Boolean, "INTERPOLATION", &stage.expsync},
				{OptType_Boolean, "DOWNSCROLL", &stage.downscroll, {.spec_boolean = {0}}},
				{OptType_Boolean, "CLASSIC HUD", &stage.oldhud,     {.spec_boolean = {0}}},
			};
			
			//Initialize page
			if (demu.page_swap)
				demu.scroll = COUNT_OF(demu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);

			if (demu.page_swap)
			    {
					stage.backpick = RandomRange(0, 2);
				}
			
			//Draw page label
			demu.font_bold.draw(&demu.font_bold,
				"OPTIONS",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Handle option and selection
			if (demu.next_page == demu.page && Trans_Idle())
			{
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (demu.select > 0)
						demu.select--;
					else
						demu.select = COUNT_OF(demu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (demu.select < COUNT_OF(demu_options) - 1)
						demu.select++;
					else
						demu.select = 0;
				}
				
				//Handle option changing
				switch (demu_options[demu.select].type)
				{
					case OptType_Boolean:
						if (pad_state.press & (PAD_CROSS | PAD_LEFT | PAD_RIGHT))
							*((boolean*)demu_options[demu.select].value) ^= 1;
						break;
					case OptType_Enum:
						if (pad_state.press & PAD_LEFT)
							if (--*((s32*)demu_options[demu.select].value) < 0)
								*((s32*)demu_options[demu.select].value) = demu_options[demu.select].spec.spec_enum.max - 1;
						if (pad_state.press & PAD_RIGHT)
							if (++*((s32*)demu_options[demu.select].value) >= demu_options[demu.select].spec.spec_enum.max)
								*((s32*)demu_options[demu.select].value) = 0;
						break;
				}
				
				//Return to main demu if circle is pressed
				if (pad_state.press & PAD_CIRCLE)
				{
					demu.next_page = DemuPage_Main;
					demu.next_select = 3; //Options
					Trans_Start();
				}
			}
			
			//Draw options
			s32 next_scroll = demu.select * FIXED_DEC(24,1);
			demu.scroll += (next_scroll - demu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(demu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (demu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				char text[0x80];
				switch (demu_options[i].type)
				{
					case OptType_Boolean:
						sprintf(text, "%s %s", demu_options[i].text, *((boolean*)demu_options[i].value) ? "ON" : "OFF");
						break;
					case OptType_Enum:
						sprintf(text, "%s %s", demu_options[i].text, demu_options[i].spec.spec_enum.strs[*((s32*)demu_options[i].value)]);
						break;
				}
				demu.font_bold.draw(&demu.font_bold,
					Demu_LowerIf(text, demu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			Demu_DrawBack(
				true,
				8,
				253 >> 1, 113 >> 1, 155 >> 1,
				0, 0, 0
			);
			break;
		}
	#ifdef PSXF_NETWORK
		case DemuPage_NetHost:
		{
			const size_t demu_options = 3;
			
			//Initialize page
			if (demu.page_swap)
			{
				demu.page_state.net_host.type = false;
				demu.page_state.net_host.port[0] = '\0';
				demu.page_state.net_host.pass[0] = '\0';
			}
			
			//Handle option and selection
			if (demu.next_page == demu.page && Trans_Idle())
			{
				if (!demu.page_state.net_host.type)
				{
					//Change option
					if (pad_state.press & PAD_UP)
					{
						if (demu.select > 0)
							demu.select--;
						else
							demu.select = demu_options - 1;
					}
					if (pad_state.press & PAD_DOWN)
					{
						if (demu.select < demu_options - 1)
							demu.select++;
						else
							demu.select = 0;
					}
					
					//Handle selection when cross is pressed
					if (pad_state.press & (PAD_START | PAD_CROSS))
					{
						switch (demu.select)
						{
							case 0: //Port
							case 1: //Pass
								demu.page_state.net_host.type = true;
								break;
							case 2: //Host
								if (!Network_HostPort(demu.page_state.net_host.port, demu.page_state.net_host.pass))
								{
									demu.next_page = DemuPage_NetOpWait;
									demu.next_select = 0;
									Trans_Start();
								}
								break;
						}
					}
					
					//Return to main demu if circle is pressed
					if (pad_state.press & PAD_CIRCLE)
					{
						demu.next_page = DemuPage_Main;
						demu.next_select = 5; //Host Server
						Trans_Start();
					}
				}
				else
				{
					//Stop typing when start is pressed
					if (pad_state.press & PAD_START)
					{
						switch (demu.select)
						{
							case 0: //Port
							case 1: //Pass
								demu.page_state.net_host.type = false;
								break;
						}
					}
				}
			}
			
			//Draw page label
			demu.font_bold.draw(&demu.font_bold,
				"HOST SERVER",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Draw options
			DemuStr_Process(&demu.page_state.net_host.port, 64 + 3 * 0, 64 + 16 * 0, "Port: %s", demu.select == 0, demu.page_state.net_host.type);
			DemuStr_Process(&demu.page_state.net_host.pass, 64 + 3 * 1, 64 + 16 * 1, "Pass: %s", demu.select == 1, demu.page_state.net_host.type);
			demu.font_bold.draw(&demu.font_bold, Demu_LowerIf("HOST", demu.select != 2), 64 + 3 * 2, 64 + 16 * 2, FontAlign_Left);
			
			//Draw background
			Demu_DrawBack(
				true,
				8,
				146 >> 1, 113 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case DemuPage_NetJoin:
		{
			const size_t demu_options = 4;
			
			//Initialize page
			if (demu.page_swap)
			{
				demu.page_state.net_join.type = false;
				demu.page_state.net_join.ip[0] = '\0';
				demu.page_state.net_join.port[0] = '\0';
				demu.page_state.net_join.pass[0] = '\0';
			}
			
			//Handle option and selection
			if (demu.next_page == demu.page && Trans_Idle())
			{
				if (!demu.page_state.net_join.type)
				{
					//Change option
					if (pad_state.press & PAD_UP)
					{
						if (demu.select > 0)
							demu.select--;
						else
							demu.select = demu_options - 1;
					}
					if (pad_state.press & PAD_DOWN)
					{
						if (demu.select < demu_options - 1)
							demu.select++;
						else
							demu.select = 0;
					}
					
					//Handle selection when cross is pressed
					if (pad_state.press & (PAD_START | PAD_CROSS))
					{
						switch (demu.select)
						{
							case 0: //Ip
							case 1: //Port
							case 2: //Pass
								demu.page_state.net_join.type = true;
								break;
							case 3: //Join
								if (!Network_Join(demu.page_state.net_join.ip, demu.page_state.net_join.port, demu.page_state.net_join.pass))
								{
									demu.next_page = DemuPage_NetConnect;
									demu.next_select = 0;
									Trans_Start();
								}
								break;
						}
					}
					
					//Return to main demu if circle is pressed
					if (pad_state.press & PAD_CIRCLE)
					{
						demu.next_page = DemuPage_Main;
						demu.next_select = 4; //Join Server
						Trans_Start();
					}
				}
				else
				{
					//Stop typing when start is pressed
					if (pad_state.press & PAD_START)
					{
						switch (demu.select)
						{
							case 0: //Join
							case 1: //Port
							case 2: //Pass
								demu.page_state.net_join.type = false;
								break;
						}
					}
				}
			}
			
			//Draw page label
			demu.font_bold.draw(&demu.font_bold,
				"JOIN SERVER",
				16,
				SCREEN_HEIGHT - 32,
				FontAlign_Left
			);
			
			//Draw options
			DemuStr_Process(&demu.page_state.net_join.ip, 64 + 3 * 0, 64 + 16 * 0, "Address: %s", demu.select == 0, demu.page_state.net_join.type);
			DemuStr_Process(&demu.page_state.net_join.port, 64 + 3 * 1, 64 + 16 * 1, "Port: %s", demu.select == 1, demu.page_state.net_join.type);
			DemuStr_Process(&demu.page_state.net_join.pass, 64 + 3 * 2, 64 + 16 * 2, "Pass: %s", demu.select == 2, demu.page_state.net_join.type);
			demu.font_bold.draw(&demu.font_bold, Demu_LowerIf("JOIN", demu.select != 3), 64 + 3 * 3, 64 + 16 * 3, FontAlign_Left);
			
			//Draw background
			Demu_DrawBack(
				true,
				8,
				146 >> 1, 113 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case DemuPage_NetConnect:
		{
			//Change state according to network state
			if (demu.next_page == demu.page && Trans_Idle())
			{
				if (!Network_Connected())
				{
					//Disconnected
					demu.next_page = DemuPage_NetFail;
					demu.next_select = 0;
					Trans_Start();
				}
				else if (Network_Allowed())
				{
					//Allowed to join
					demu.next_page = DemuPage_NetLobby;
					demu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			demu.font_bold.draw(&demu.font_bold,
				"CONNECTING",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Demu_DrawBack(
				true,
				8,
				113 >> 1, 146 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case DemuPage_NetOpWait:
		{
			//Change state according to network state
			if (demu.next_page == demu.page && Trans_Idle())
			{
				if (!Network_Connected())
				{
					//Disconnected
					demu.next_page = DemuPage_NetFail;
					demu.next_select = 0;
					Trans_Start();
				}
				else if (Network_HasPeer())
				{
					//Peer has joined
					demu.next_page = DemuPage_NetOp;
					demu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			demu.font_bold.draw(&demu.font_bold,
				"WAITING FOR PEER",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Demu_DrawBack(
				true,
				8,
				113 >> 1, 146 >> 1, 253 >> 1,
				0, 0, 0
			);
			break;
		}
		case DemuPage_NetOp:
		{
			static const struct
			{
				boolean diff;
				StageId stage;
				const char *text;
			} demu_options[] = {
				//{StageId_4_4, "TEST"},
				{true,  StageId_1_4, "TUTORIAL"},
				{true,  StageId_1_1, "BOPEEBO"},
				{true,  StageId_1_2, "FRESH"},
				{true,  StageId_1_3, "DADBATTLE"},
				{true,  StageId_2_1, "SPOOKEEZ"},
				{true,  StageId_2_2, "SOUTH"},
				{true,  StageId_2_3, "MONSTER"},
				{true,  StageId_3_1, "PICO"},
				{true,  StageId_3_2, "PHILLY NICE"},
				{true,  StageId_3_3, "BLAMMED"},
				{true,  StageId_4_1, "SATIN PANTIES"},
				{true,  StageId_4_2, "HIGH"},
				{true,  StageId_4_3, "MILF"},
				{true,  StageId_5_1, "COCOA"},
				{true,  StageId_5_2, "EGGNOG"},
				{true,  StageId_5_3, "WINTER HORRORLAND"},
				{true,  StageId_6_1, "SENPAI"},
				{true,  StageId_6_2, "ROSES"},
				{true,  StageId_6_3, "THORNS"},
				{true,  StageId_7_1, "UGH"},
				{true,  StageId_7_2, "GUNS"},
				{true,  StageId_7_3, "STRESS"},
				{false, StageId_Kapi_1, "WOCKY"},
				{false, StageId_Kapi_2, "BEATHOVEN"},
				{false, StageId_Kapi_3, "HAIRBALL"},
				{false, StageId_Kapi_4, "NYAW"},
				{true,  StageId_Clwn_1, "IMPROBABLE OUTSET"},
				{true,  StageId_Clwn_2, "MADNESS"},
				{true,  StageId_Clwn_3, "HELLCLOWN"},
				{false, StageId_Clwn_4, "EXPURGATION"},
				{false, StageId_2_4, "CLUCKED"},
			};
			
			//Initialize page
			if (demu.page_swap)
			{
				demu.scroll = COUNT_OF(demu_options) * FIXED_DEC(24 + SCREEN_HEIGHT2,1);
				demu.page_param.stage.diff = StageDiff_Normal;
				demu.page_state.net_op.swap = false;
			}
			
			//Handle option and selection
			if (demu.next_page == demu.page && Trans_Idle())
			{
				//Check network state
				if (!Network_Connected())
				{
					//Disconnected
					demu.next_page = DemuPage_NetFail;
					demu.next_select = 0;
					Trans_Start();
				}
				else if (!Network_HasPeer())
				{
					//Peer disconnected
					demu.next_page = DemuPage_NetOpWait;
					demu.next_select = 0;
					Trans_Start();
				}
				
				//Change option
				if (pad_state.press & PAD_UP)
				{
					if (demu.select > 0)
						demu.select--;
					else
						demu.select = COUNT_OF(demu_options) - 1;
				}
				if (pad_state.press & PAD_DOWN)
				{
					if (demu.select < COUNT_OF(demu_options) - 1)
						demu.select++;
					else
						demu.select = 0;
				}
				
				//Select option if cross is pressed
				if (pad_state.press & (PAD_START | PAD_CROSS))
				{
					//Load stage
					Network_SetReady(false);
					stage.mode = demu.page_state.net_op.swap ? StageMode_Net2 : StageMode_Net1;
					demu.next_page = DemuPage_Stage;
					demu.page_param.stage.id = demu_options[demu.select].stage;
					if (!demu_options[demu.select].diff)
						demu.page_param.stage.diff = StageDiff_Hard;
					demu.page_param.stage.story = false;
					Trans_Start();
				}
				
				//Swap characters if triangle is pressed
				if (pad_state.press & PAD_TRIANGLE)
					demu.page_state.net_op.swap ^= true;
			}
			
			//Draw op controls
			const char *control_txt;
			
			control_txt = demu.page_state.net_op.swap ? "You will be Player 2. Press Triangle to swap." : "You will be Player 1. Press Triangle to swap.";
			demu.font_arial.draw_col(&demu.font_arial, control_txt, 24, SCREEN_HEIGHT - 24 - 12, FontAlign_Left, 0x80, 0x80, 0x80);
			demu.font_arial.draw_col(&demu.font_arial, control_txt, 24 + 1, SCREEN_HEIGHT - 24 - 12 + 1, FontAlign_Left, 0x00, 0x00, 0x00);
			
			//Draw difficulty selector
			if (demu_options[demu.select].diff)
				Demu_DifficultySelector(SCREEN_WIDTH - 100, SCREEN_HEIGHT2 - 48);
			
			//Draw options
			s32 next_scroll = demu.select * FIXED_DEC(24,1);
			demu.scroll += (next_scroll - demu.scroll) >> 4;
			
			for (u8 i = 0; i < COUNT_OF(demu_options); i++)
			{
				//Get position on screen
				s32 y = (i * 24) - 8 - (demu.scroll >> FIXED_SHIFT);
				if (y <= -SCREEN_HEIGHT2 - 8)
					continue;
				if (y >= SCREEN_HEIGHT2 + 8)
					break;
				
				//Draw text
				demu.font_bold.draw(&demu.font_bold,
					Demu_LowerIf(demu_options[i].text, demu.select != i),
					48 + (y >> 2),
					SCREEN_HEIGHT2 + y - 8,
					FontAlign_Left
				);
			}
			
			//Draw background
			Demu_DrawBack(
				true,
				8,
				113 >> 1, 253 >> 1, 146 >> 1,
				0, 0, 0
			);
			break;
		}
		case DemuPage_NetLobby:
		{
			//Check network state
			if (demu.next_page == demu.page && Trans_Idle())
			{
				if (!Network_Connected())
				{
					//Disconnected
					demu.next_page = DemuPage_NetFail;
					demu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			demu.font_bold.draw(&demu.font_bold,
				"WAITING FOR HOST",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Demu_DrawBack(
				true,
				8,
				253 >> 1, 146 >> 1, 113 >> 1,
				0, 0, 0
			);
			break;
		}
		case DemuPage_NetFail:
		{
			//Return to main demu if circle or start is pressed
			if (demu.next_page == demu.page && Trans_Idle())
			{
				if (pad_state.press & (PAD_CIRCLE | PAD_START))
				{
					demu.next_page = DemuPage_Main;
					demu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			demu.font_bold.draw(&demu.font_bold,
				"DISCONNECTED",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Demu_DrawBack(
				true,
				8,
				253 >> 1, 30 >> 1, 15 >> 1,
				0, 0, 0
			);
			break;
		}
		case DemuPage_NetInitFail:
		{
			//Return to main demu if circle or start is pressed
			if (demu.next_page == demu.page && Trans_Idle())
			{
				if (pad_state.press & (PAD_CIRCLE | PAD_START))
				{
					demu.next_page = DemuPage_Main;
					demu.next_select = 0;
					Trans_Start();
				}
			}
			
			//Draw page label
			demu.font_bold.draw(&demu.font_bold,
				"WSA INIT FAILED",
				SCREEN_WIDTH2,
				SCREEN_HEIGHT2 - 8,
				FontAlign_Center
			);
			
			//Draw background
			Demu_DrawBack(
				true,
				8,
				253 >> 1, 30 >> 1, 15 >> 1,
				0, 0, 0
			);
			break;
		}
	#endif
		case DemuPage_Stage:
		{
			//Unload demu state
			Demu_Unload();
			
			//Load new stage
			LoadScr_Start();
			Stage_Load(demu.page_param.stage.id, demu.page_param.stage.diff, demu.page_param.stage.story);
			gameloop = GameLoop_Stage;
			LoadScr_End();
			break;
		}
		case DemuPage_Update:
		{
			//Unload demo
			Demu_Unload();

			//Load Main Game
			LoadScr_Start();
			Menu_Load(MenuPage_Opening);
			gameloop = GameLoop_Menu;
			LoadScr_End();
			break;
		}
		default:
			break;
	}
	
	//Clear page swap flag
	demu.page_swap = demu.page != exec_page;
}
