/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "stage.h"
#include "demu.h"

#include "mem.h"
#include "timer.h"
#include "audio.h"
#include "pad.h"
#include "main.h"
#include "random.h"
#include "movie.h"
#include "network.h"

#include "menu.h"
#include "trans.h"
#include "loadscr.h"

#include "object/combo.h"
#include "object/splash.h"

#include "stime.h"

//Stage constants

//psychic portrait animation stuff
static const CharFrame psytalk_frame[] =
{
	//normal
	{0, {  0,   0, 111, 114}, {0, 0}}, //0
	{0, {111,   0, 109, 114}, {-2, 0}}, //1
	{0, {  0, 114, 108, 114}, {-3, 0}}, //2
	{0, {108, 114, 108, 112}, {-3, -2}}, //3
	//piss
	{1, {   0,   0,  87, 106}, {0, 0}}, //4
	{1, {  87,   0,  87, 106}, {0, 0}}, //5
	{1, {   0, 106,  90, 105}, {0, 0}}, //6
	{1, {  90, 106,  90, 105}, {0, 0}}, //7
	//erect
	{2, {   0,   0, 130, 119}, {0, 0}}, //8
	{2, {   0, 119, 129, 119}, {0, 0}}, //9
	{3, {   0,   0, 128, 120}, {0, 0}}, //10
	{3, {   0, 120, 128, 117}, {0, 0}}, //11
	//annoyed
	{4, {   0,   0, 111, 114}, {0, 0}}, //12
	{4, { 111,   0, 109, 114}, {0, 0}}, //13
	{4, {   0, 114, 108, 114}, {0, 0}}, //14
	{4, { 108, 114, 108, 112}, {0, 0}}, //15
	//confused
	{5, {   0,   0,  86, 116}, {0, 0}}, //16
	{5, {  86,   0,  85, 117}, {0, 0}}, //17
	{5, {   0, 117,  83, 118}, {0, 0}}, //18
	{5, {  83, 117,  83, 115}, {0, 0}}, //19
	//shock
	{6, {   0,   0,  97, 124}, {0, 0}}, //20
	{6, {  97,   0,  96, 124}, {0, 0}}, //21
	{6, {   0, 124,  96, 124}, {0, 0}}, //22
	{6, {  96, 124,  95, 123}, {0, 0}}, //23
};

//animations
static const Animation psytalk_anim[] = {
	{1, (const u8[]){0, 0, 1, 1, 2, 2, 3, ASCR_REPEAT}}, //normal
	{1, (const u8[]){4, 4, 5, 5, 6, 6, 7, ASCR_REPEAT}}, //piss
	{1, (const u8[]){8, 8, 9, 9, 10, 10, 11, ASCR_REPEAT}}, //erect
	{1, (const u8[]){12, 12, 13, 13, 14, 14, 15, ASCR_REPEAT}}, //annoyed
	{1, (const u8[]){16, 16, 17, 17, 18, 18, 19, ASCR_REPEAT}}, //confused
	{1, (const u8[]){20, 20, 21, 21, 22, 22, 23, ASCR_REPEAT}}, //shock
};

//Ratings
static const struct
{
	const char *text;
}ratings[] = {
	{"You Suck!"}, //0% to 19%
	{"Shit"}, //20% to 39%
	{"Bad"}, //40% to 49%
	{"Bruh"}, //50% to 59%
	{"Meh"}, //60% to 68%
	{"Nice"}, //69%
	{"Good"}, //70% to 79%
	{"Great"}, //80% to 89%
	{"Sick!"}, //90% to 99%
	{"Perfect!!"}, //100%
};


//welcome to the shitshow
int note_x[8] = {
	//BF
	 FIXED_DEC(26,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 26
	 FIXED_DEC(60,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 60
	 FIXED_DEC(94,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 94
	 FIXED_DEC(128,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 128
	//Opponent
	 FIXED_DEC(-128,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -128
	 FIXED_DEC(-94,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -94
	 FIXED_DEC(-60,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -60
	 FIXED_DEC(-26,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -26
};

int note_flip[8] = {
	//BF
	 FIXED_DEC(-128,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 26
	 FIXED_DEC(-94,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 60
	 FIXED_DEC(-60,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 94
	 FIXED_DEC(-26,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 128
	//Opponent
	 FIXED_DEC(26,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -128
	 FIXED_DEC(60,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -94
	 FIXED_DEC(94,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -60
	 FIXED_DEC(128,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -26
};
//for noflip notes
int note_norm[8] = {
	//BF
	 FIXED_DEC(26,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 26
	 FIXED_DEC(60,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 60
	 FIXED_DEC(94,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 94
	 FIXED_DEC(128,1) + FIXED_DEC(SCREEN_WIDEADD,4),//default is 128
	//Opponent
	 FIXED_DEC(-128,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -128
	 FIXED_DEC(-94,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -94
	 FIXED_DEC(-60,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -60
	 FIXED_DEC(-26,1) - FIXED_DEC(SCREEN_WIDEADD,4),//default is -26
};

static const fixed_t note_y = FIXED_DEC(32 - SCREEN_HEIGHT2, 1);

static const u16 note_key[] = {INPUT_LEFT, INPUT_DOWN, INPUT_UP, INPUT_RIGHT};
static const u8 note_anims[4][3] = {
	{CharAnim_Left,  CharAnim_LeftAlt,  PlayerAnim_LeftMiss},
	{CharAnim_Down,  CharAnim_DownAlt,  PlayerAnim_DownMiss},
	{CharAnim_Up,    CharAnim_UpAlt,    PlayerAnim_UpMiss},
	{CharAnim_Right, CharAnim_RightAlt, PlayerAnim_RightMiss},
};

//Stage definitions
boolean nohud;

#include "character/bf.h"
#include "character/bfweeb.h"
#include "character/dad.h"
#include "character/gf.h"
#include "character/senpaib.h"
#include "character/psychic.h"
#include "character/sendai.h"
#include "character/bfspirit.h"
#include "character/end.h"
#include "character/bft.h"

#include "stage/dummy.h"
#include "stage/fplace.h"
#include "stage/flames.h"
#include "stage/space.h"
#include "stage/chop.h"
#include "stage/flamec.h"

static const StageDef stage_defs[StageId_Max] = {
	#include "stagedef_disc1.h"
};

//Stage state
Stage stage;

//Stage music functions
static void Stage_StartVocal(void)
{
	if (!(stage.flag & STAGE_FLAG_VOCAL_ACTIVE))
	{
		Audio_ChannelXA(stage.stage_def->music_channel);
		stage.flag |= STAGE_FLAG_VOCAL_ACTIVE;
	}
}

static void Stage_CutVocal(void)
{
	if (stage.flag & STAGE_FLAG_VOCAL_ACTIVE)
	{
		Audio_ChannelXA(stage.stage_def->music_channel + 1);
		stage.flag &= ~STAGE_FLAG_VOCAL_ACTIVE;
	}
}

//Stage camera functions
static void Stage_FocusCharacter(Character *ch, fixed_t div)
{
	//Use character focus settings to update target position and zoom
	stage.camera.tx = ch->x + ch->focus_x;
	stage.camera.ty = ch->y + ch->focus_y;
	stage.camera.tz = ch->focus_zoom;
	stage.camera.td = div;
}

static void Stage_ScrollCamera(void)
{
	if (stage.freecam == 1)
	{
		if (pad_state.held & PAD_LEFT)
			stage.camera.x -= FIXED_DEC(2,1);
		if (pad_state.held & PAD_UP)
			stage.camera.y -= FIXED_DEC(2,1);
		if (pad_state.held & PAD_RIGHT)
			stage.camera.x += FIXED_DEC(2,1);
		if (pad_state.held & PAD_DOWN)
			stage.camera.y += FIXED_DEC(2,1);
		if (pad_state.held & PAD_TRIANGLE)
			stage.camera.zoom -= FIXED_DEC(1,100);
		if (pad_state.held & PAD_CROSS)
			stage.camera.zoom += FIXED_DEC(1,100);
	}
	else if (stage.freecam == 0)
	{
		//Get delta position
		fixed_t dx = stage.camera.tx - stage.camera.x;
		fixed_t dy = stage.camera.ty - stage.camera.y;
		fixed_t dz = stage.camera.tz - stage.camera.zoom;
		
		//Scroll based off current divisor
		stage.camera.x += FIXED_MUL(dx, stage.camera.td);
		stage.camera.y += FIXED_MUL(dy, stage.camera.td);
		stage.camera.zoom += FIXED_MUL(dz, stage.camera.td);
		
		////Shake in Week 4
		//if (stage.stage_id >= StageId_4_1 && stage.stage_id <= StageId_4_3)
		//{
		//	stage.camera.x += RandomRange(FIXED_DEC(-1,10),FIXED_DEC(1,10));
		//	stage.camera.y += RandomRange(FIXED_DEC(-25,100),FIXED_DEC(25,100));
		//}
	}
	
	//Update other camera stuff
	stage.camera.bzoom = FIXED_MUL(stage.camera.zoom, stage.bump);
}

static void Stage_MoveChar(void)
{
	//move player 1 with second controller's d pad when debug is 3
	if (stage.debug == 3)
	{
		if (pad_state_2.held & INPUT_LEFT)
		    stage.player->x -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_DOWN)
		    stage.player->y += FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_UP)
		    stage.player->y -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_RIGHT)
		    stage.player->x += FIXED_DEC(1,1);
	}

//move player 2 with second controller's d pad when debug is 4
	if (stage.debug == 4)
	{
		if (pad_state_2.held & INPUT_LEFT)
		    stage.opponent->x -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_DOWN)
		    stage.opponent->y += FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_UP)
		    stage.opponent->y -= FIXED_DEC(1,1);
		if (pad_state_2.held & INPUT_RIGHT)
		    stage.opponent->x += FIXED_DEC(1,1);
	}
}

//Stage section functions
static void Stage_ChangeBPM(u16 bpm, u16 step)
{
	//Update last BPM
	stage.last_bpm = bpm;
	
	//Update timing base
	if (stage.step_crochet)
		stage.time_base += FIXED_DIV(((fixed_t)step - stage.step_base) << FIXED_SHIFT, stage.step_crochet);
	stage.step_base = step;
	
	//Get new crochet and times
	stage.step_crochet = ((fixed_t)bpm << FIXED_SHIFT) * 8 / 240; //15/12/24
	stage.step_time = FIXED_DIV(FIXED_DEC(12,1), stage.step_crochet);
	
	//Get new crochet based values
	stage.early_safe = stage.late_safe = stage.step_crochet / 6; //10 frames
	stage.late_sus_safe = stage.late_safe;
	stage.early_sus_safe = stage.early_safe * 2 / 5;
}

static Section *Stage_GetPrevSection(Section *section)
{
	if (section > stage.sections)
		return section - 1;
	return NULL;
}

static u16 Stage_GetSectionStart(Section *section)
{
	Section *prev = Stage_GetPrevSection(section);
	if (prev == NULL)
		return 0;
	return prev->end;
}

//Section scroll structure
typedef struct
{
	fixed_t start;   //Seconds
	fixed_t length;  //Seconds
	u16 start_step;  //Sub-steps
	u16 length_step; //Sub-steps
	
	fixed_t size; //Note height
} SectionScroll;

static void Stage_GetSectionScroll(SectionScroll *scroll, Section *section)
{
	//Get BPM
	u16 bpm = section->flag & SECTION_FLAG_BPM_MASK;
	
	//Get section step info
	scroll->start_step = Stage_GetSectionStart(section);
	scroll->length_step = section->end - scroll->start_step;
	
	//Get section time length
	scroll->length = (scroll->length_step * FIXED_DEC(15,1) / 12) * 24 / bpm;
	
	//Get note height
	scroll->size = FIXED_MUL(stage.speed, scroll->length * (12 * 150) / scroll->length_step) + FIXED_UNIT;
}

//Note hit detection
static u8 Stage_HitNote(PlayerState *this, u8 type, fixed_t offset)
{
	//Get hit type
	if (offset < 0)
		offset = -offset;
	
	u8 hit_type;
	if (offset > stage.late_safe * 9 / 11)
		hit_type = 3; //SHIT
	else if (offset > stage.late_safe * 6 / 11)
		hit_type = 2; //BAD
	else if (offset > stage.late_safe * 3 / 11)
		hit_type = 1; //GOOD
	else
		hit_type = 0; //SICK
	
	//Increment combo and score
	this->combo++;
	
	static const s32 score_inc[] = {
		35, //SICK
		20, //GOOD
		10, //BAD
		 5, //SHIT
	};
	this->score += score_inc[hit_type];
	this->refresh_score = true;

	stage.notes_passed++;
	stage.notes_played++;
	
	//Restore vocals and health
	Stage_StartVocal();
	this->health += 230;
	
	//Create combo object telling of our combo
	Obj_Combo *combo = Obj_Combo_New(
		this->character->focus_x,
		this->character->focus_y,
		hit_type,
		this->combo >= 10 ? this->combo : 0xFFFF
	);
	if (combo != NULL)
		ObjectList_Add(&stage.objlist_fg, (Object*)combo);
	
	//Create note splashes if SICK
	
	if (hit_type == 0)
	{
		for (int i = 0; i < 3; i++)
		{
			//Create splash object
			Obj_Splash *splash = Obj_Splash_New(
				
				note_x[type ^ stage.note_swap],
				note_y * (stage.downscroll ? -1 : 1),
				type & 0x3
			);
			if (splash != NULL)
				ObjectList_Add(&stage.objlist_splash, (Object*)splash);
		}
	}
	
	
	return hit_type;
}

static void Stage_MissNote(PlayerState *this)
{
	if (this->combo)
	{
		//Kill combo
		if (stage.gf != NULL && this->combo > 5)
			stage.gf->set_anim(stage.gf, CharAnim_DownAlt); //Cry if we lost a large combo
		this->combo = 0;
		
		//Create combo object telling of our lost combo
		Obj_Combo *combo = Obj_Combo_New(
			this->character->focus_x,
			this->character->focus_y,
			0xFF,
			0
		);
		if (combo != NULL)
			ObjectList_Add(&stage.objlist_fg, (Object*)combo);
	}
}

static void Stage_NoteCheck(PlayerState *this, u8 type)
{
	//Perform note check
	for (Note *note = stage.cur_note;; note++)
	{
		if (!(note->type & NOTE_FLAG_MINE))
		{
			//Check if note can be hit
			fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
			if (note_fp - stage.early_safe > stage.note_scroll)
				break;
			if (note_fp + stage.late_safe < stage.note_scroll)
				continue;
			if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || (note->type & NOTE_FLAG_SUSTAIN))
				continue;
			
			//Hit the note
			note->type |= NOTE_FLAG_HIT;
			
			this->character->set_anim(this->character, note_anims[type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);
			u8 hit_type = Stage_HitNote(this, type, stage.note_scroll - note_fp);
			this->arrow_hitan[type & 0x3] = stage.step_time;
			
			#ifdef PSXF_NETWORK
				if (stage.mode >= StageMode_Net1)
				{
					//Send note hit packet
					Packet note_hit;
					note_hit[0] = PacketType_NoteHit;
					
					u16 note_i = note - stage.notes;
					note_hit[1] = note_i >> 0;
					note_hit[2] = note_i >> 8;
					
					note_hit[3] = this->score >> 0;
					note_hit[4] = this->score >> 8;
					note_hit[5] = this->score >> 16;
					note_hit[6] = this->score >> 24;
					
					note_hit[7] = hit_type;
					
					note_hit[8] = this->combo >> 0;
					note_hit[9] = this->combo >> 8;
					
					Network_Send(&note_hit);
				}
			#else
				(void)hit_type;
			#endif
			return;
		}
		else
		{
			//Check if mine can be hit
			fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
			if (note_fp - (stage.late_safe * 3 / 5) > stage.note_scroll)
				break;
			if (note_fp + (stage.late_safe * 2 / 5) < stage.note_scroll)
				continue;
			if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || (note->type & NOTE_FLAG_SUSTAIN))
				continue;
			
			//Hit the mine
			note->type |= NOTE_FLAG_HIT;
			
			if (stage.stage_id == StageId_1_4)
				this->health = -0x7000;
			else
				this->health -= 2000;
			if (this->character->spec & CHAR_SPEC_MISSANIM)
				this->character->set_anim(this->character, note_anims[type & 0x3][2]);
			else
				this->character->set_anim(this->character, note_anims[type & 0x3][0]);
			this->arrow_hitan[type & 0x3] = -1;
			
			#ifdef PSXF_NETWORK
				if (stage.mode >= StageMode_Net1)
				{
					//Send note hit packet
					Packet note_hit;
					note_hit[0] = PacketType_NoteHit;
					
					u16 note_i = note - stage.notes;
					note_hit[1] = note_i >> 0;
					note_hit[2] = note_i >> 8;
					
					note_hit[3] = this->score >> 0;
					note_hit[4] = this->score >> 8;
					note_hit[5] = this->score >> 16;
					note_hit[6] = this->score >> 24;
					
					/*
					note_hit[7] = 0xFF;
					
					note_hit[8] = this->combo >> 0;
					note_hit[9] = this->combo >> 8;
					*/
					
					Network_Send(&note_hit);
				}
			#endif
			return;
		}
	}
	
	//Missed a note
	this->arrow_hitan[type & 0x3] = -1;
	stage.notes_passed++;
	stage.misses++;
	
	if (!stage.ghost)
	{
		if (this->character->spec & CHAR_SPEC_MISSANIM)
			this->character->set_anim(this->character, note_anims[type & 0x3][2]);
		else
			this->character->set_anim(this->character, note_anims[type & 0x3][0]);
		Stage_MissNote(this);
		
		this->health -= 400;
		this->score -= 1;
		this->refresh_score = true;
		
		#ifdef PSXF_NETWORK
			if (stage.mode >= StageMode_Net1)
			{
				//Send note hit packet
				Packet note_hit;
				note_hit[0] = PacketType_NoteMiss;
				note_hit[1] = type & 0x3;
				
				note_hit[2] = this->score >> 0;
				note_hit[3] = this->score >> 8;
				note_hit[4] = this->score >> 16;
				note_hit[5] = this->score >> 24;
				
				Network_Send(&note_hit);
			}
		#endif
	}
}

static void Stage_SustainCheck(PlayerState *this, u8 type)
{
	//Perform note check
	for (Note *note = stage.cur_note;; note++)
	{
		//Check if note can be hit
		fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
		if (note_fp - stage.early_sus_safe > stage.note_scroll)
			break;
		if (note_fp + stage.late_sus_safe < stage.note_scroll)
			continue;
		if ((note->type & NOTE_FLAG_HIT) || (note->type & (NOTE_FLAG_OPPONENT | 0x3)) != type || !(note->type & NOTE_FLAG_SUSTAIN))
			continue;
		
		//Hit the note
		note->type |= NOTE_FLAG_HIT;
		
		this->character->set_anim(this->character, note_anims[type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);
		
		Stage_StartVocal();
		this->health += 230;
		this->arrow_hitan[type & 0x3] = stage.step_time;
			
		#ifdef PSXF_NETWORK
			if (stage.mode >= StageMode_Net1)
			{
				//Send note hit packet
				Packet note_hit;
				note_hit[0] = PacketType_NoteHit;
				
				u16 note_i = note - stage.notes;
				note_hit[1] = note_i >> 0;
				note_hit[2] = note_i >> 8;
				
				note_hit[3] = this->score >> 0;
				note_hit[4] = this->score >> 8;
				note_hit[5] = this->score >> 16;
				note_hit[6] = this->score >> 24;
				
				/*
				note_hit[7] = 0xFF;
				
				note_hit[8] = this->combo >> 0;
				note_hit[9] = this->combo >> 8;
				*/
				
				Network_Send(&note_hit);
			}
		#endif
	}
}

static void Stage_ProcessPlayer(PlayerState *this, Pad *pad, boolean playing)
{
	//Handle player note presses
	#ifndef STAGE_PERFECT
		if (playing)
		{
			u8 i = (this->character == stage.opponent) ? NOTE_FLAG_OPPONENT : 0;
			
			this->pad_held = this->character->pad_held = pad->held;
			this->pad_press = pad->press;
			
			if (this->pad_held & INPUT_LEFT)
				Stage_SustainCheck(this, 0 | i);
			if (this->pad_held & INPUT_DOWN)
				Stage_SustainCheck(this, 1 | i);
			if (this->pad_held & INPUT_UP)
				Stage_SustainCheck(this, 2 | i);
			if (this->pad_held & INPUT_RIGHT)
				Stage_SustainCheck(this, 3 | i);
			
			if (this->pad_press & INPUT_LEFT)
				Stage_NoteCheck(this, 0 | i);
			if (this->pad_press & INPUT_DOWN)
				Stage_NoteCheck(this, 1 | i);
			if (this->pad_press & INPUT_UP)
				Stage_NoteCheck(this, 2 | i);
			if (this->pad_press & INPUT_RIGHT)
				Stage_NoteCheck(this, 3 | i);
			
			if (this->pad_press & FREECAM_ON)
			    stage.freecam = 1;
			if (this->pad_press & FREECAM_OFF)
			    stage.freecam = 0;

			if (this->pad_press & DEBUG_SWITCH)
			{
				if (stage.debug < 5)
				    stage.debug += 1;
				else 
				    stage.debug = 0;
			}
		}
		else
		{
			this->pad_held = this->character->pad_held = 0;
			this->pad_press = 0;
		}
	#endif
	
	if (stage.botplay)
	{
		//Do perfect note checks
		if (playing)
		{
			u8 i = (this->character == stage.opponent) ? NOTE_FLAG_OPPONENT : 0;
			
			u8 hit[4] = {0, 0, 0, 0};
			for (Note *note = stage.cur_note;; note++)
			{
				//Check if note can be hit
				fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
				if (note_fp - stage.early_safe - FIXED_DEC(12,1) > stage.note_scroll)
					break;
				if (note_fp + stage.late_safe < stage.note_scroll)
					continue;
				if ((note->type & NOTE_FLAG_MINE) || (note->type & NOTE_FLAG_OPPONENT) != i)
					continue;
				
				//Handle note hit
				if (!(note->type & NOTE_FLAG_SUSTAIN))
				{
					if (note->type & NOTE_FLAG_HIT)
						continue;
					if (stage.note_scroll >= note_fp)
						hit[note->type & 0x3] |= 1;
					else if (!(hit[note->type & 0x3] & 8))
						hit[note->type & 0x3] |= 2;
				}
				else if (!(hit[note->type & 0x3] & 2))
				{
					if (stage.note_scroll <= note_fp)
						hit[note->type & 0x3] |= 4;
					hit[note->type & 0x3] |= 8;
				}
			}
			
			//Handle input
			this->pad_held = 0;
			this->pad_press = 0;
			
			for (u8 j = 0; j < 4; j++)
			{
				if (hit[j] & 5)
				{
					this->pad_held |= note_key[j];
					Stage_SustainCheck(this, j | i);
				}
				if (hit[j] & 1)
				{
					this->pad_press |= note_key[j];
					Stage_NoteCheck(this, j | i);
				}
			}
			
			this->character->pad_held = this->pad_held;
		}
		else
		{
			this->pad_held = this->character->pad_held = 0;
			this->pad_press = 0;
		}
	}
}

static void Stage_ProcessPsyche(PlayerState *this, Pad *pad, boolean playing)
{
	//Handle player note presses
	if (stage.mode == StageMode_Normal)
	{
		if (playing)
		{
			u8 i = (this->character == stage.opponent) ? NOTE_FLAG_OPPONENT : 0;
			
			this->pad_held = this->character->pad_held = pad->held;
			this->pad_press = pad->press;
			
			if (this->pad_held & INPUT_LEFT)
				Stage_SustainCheck(this, 0 | i);
			if (this->pad_held & INPUT_DOWN)
				Stage_SustainCheck(this, 1 | i);
			if (this->pad_held & INPUT_UP)
				Stage_SustainCheck(this, 2 | i);
			if (this->pad_held & INPUT_RIGHT)
				Stage_SustainCheck(this, 3 | i);
			
			if (this->pad_press & INPUT_LEFT)
				Stage_NoteCheck(this, 0 | i);
			if (this->pad_press & INPUT_DOWN)
				Stage_NoteCheck(this, 1 | i);
			if (this->pad_press & INPUT_UP)
				Stage_NoteCheck(this, 2 | i);
			if (this->pad_press & INPUT_RIGHT)
				Stage_NoteCheck(this, 3 | i);
		}
		else
		{
			this->pad_held = this->character->pad_held = 0;
			this->pad_press = 0;
		}
	}
	
	if (stage.mode == StageMode_Normal)
	{
		//Do perfect note checks
		if (playing)
		{
			u8 i = (this->character == stage.opponent) ? NOTE_FLAG_OPPONENT : 0;
			
			u8 hit[4] = {0, 0, 0, 0};
			for (Note *note = stage.cur_note;; note++)
			{
				//Check if note can be hit
				fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
				if (note_fp - stage.early_safe - FIXED_DEC(12,1) > stage.note_scroll)
					break;
				if (note_fp + stage.late_safe < stage.note_scroll)
					continue;
				if ((note->type & NOTE_FLAG_MINE) || (note->type & NOTE_FLAG_OPPONENT) != i)
					continue;
				
				//Handle note hit
				if (!(note->type & NOTE_FLAG_SUSTAIN))
				{
					if (note->type & NOTE_FLAG_HIT)
						continue;
					if (stage.note_scroll >= note_fp)
						hit[note->type & 0x3] |= 1;
					else if (!(hit[note->type & 0x3] & 8))
						hit[note->type & 0x3] |= 2;
				}
				else if (!(hit[note->type & 0x3] & 2))
				{
					if (stage.note_scroll <= note_fp)
						hit[note->type & 0x3] |= 4;
					hit[note->type & 0x3] |= 8;
				}
			}
			
			//Handle input
			this->pad_held = 0;
			this->pad_press = 0;
			
			for (u8 j = 0; j < 4; j++)
			{
				if (hit[j] & 5)
				{
					this->pad_held |= note_key[j];
					Stage_SustainCheck(this, j | i);
				}
				if (hit[j] & 1)
				{
					this->pad_press |= note_key[j];
					Stage_NoteCheck(this, j | i);
				}
			}
			
			this->character->pad_held = this->pad_held;
		}
		else
		{
			this->pad_held = this->character->pad_held = 0;
			this->pad_press = 0;
		}
	}
}

//Stage drawing functions
void Stage_DrawTexCol(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom, u8 cr, u8 cg, u8 cb)
{
	fixed_t xz = dst->x;
	fixed_t yz = dst->y;
	fixed_t wz = dst->w;
	fixed_t hz = dst->h;
	
	//Don't draw if HUD and is disabled
	if (tex == &stage.tex_hud0 || tex == &stage.tex_hud1)
	{
		if (nohud)
			return;
	}
	
	fixed_t l = (SCREEN_WIDTH2  << FIXED_SHIFT) + FIXED_MUL(xz, zoom);// + FIXED_DEC(1,2);
	fixed_t t = (SCREEN_HEIGHT2 << FIXED_SHIFT) + FIXED_MUL(yz, zoom);// + FIXED_DEC(1,2);
	fixed_t r = l + FIXED_MUL(wz, zoom);
	fixed_t b = t + FIXED_MUL(hz, zoom);
	
	l >>= FIXED_SHIFT;
	t >>= FIXED_SHIFT;
	r >>= FIXED_SHIFT;
	b >>= FIXED_SHIFT;
	
	RECT sdst = {
		l,
		t,
		r - l,
		b - t,
	};
	Gfx_DrawTexCol(tex, src, &sdst, cr, cg, cb);
}

void Stage_DrawTex(Gfx_Tex *tex, const RECT *src, const RECT_FIXED *dst, fixed_t zoom)
{
	Stage_DrawTexCol(tex, src, dst, zoom, 0x80, 0x80, 0x80);
}

void Stage_DrawTexArb(Gfx_Tex *tex, const RECT *src, const POINT_FIXED *p0, const POINT_FIXED *p1, const POINT_FIXED *p2, const POINT_FIXED *p3, fixed_t zoom)
{
	//Don't draw if HUD and HUD is disabled
	if (nohud) {
		if (tex == &stage.tex_hud0 || tex == &stage.tex_hud1)
			return;
	}

	//Get screen-space points
	POINT s0 = {SCREEN_WIDTH2 + (FIXED_MUL(p0->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p0->y, zoom) >> FIXED_SHIFT)};
	POINT s1 = {SCREEN_WIDTH2 + (FIXED_MUL(p1->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p1->y, zoom) >> FIXED_SHIFT)};
	POINT s2 = {SCREEN_WIDTH2 + (FIXED_MUL(p2->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p2->y, zoom) >> FIXED_SHIFT)};
	POINT s3 = {SCREEN_WIDTH2 + (FIXED_MUL(p3->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p3->y, zoom) >> FIXED_SHIFT)};
	
	Gfx_DrawTexArb(tex, src, &s0, &s1, &s2, &s3);
}

void Stage_BlendTexArb(Gfx_Tex *tex, const RECT *src, const POINT_FIXED *p0, const POINT_FIXED *p1, const POINT_FIXED *p2, const POINT_FIXED *p3, fixed_t zoom, u8 mode)
{
	//Don't draw if HUD and HUD is disabled
	if (nohud) {
		if (tex == &stage.tex_hud0 || tex == &stage.tex_hud1)
			return;
	}
	
	//Get screen-space points
	POINT s0 = {SCREEN_WIDTH2 + (FIXED_MUL(p0->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p0->y, zoom) >> FIXED_SHIFT)};
	POINT s1 = {SCREEN_WIDTH2 + (FIXED_MUL(p1->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p1->y, zoom) >> FIXED_SHIFT)};
	POINT s2 = {SCREEN_WIDTH2 + (FIXED_MUL(p2->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p2->y, zoom) >> FIXED_SHIFT)};
	POINT s3 = {SCREEN_WIDTH2 + (FIXED_MUL(p3->x, zoom) >> FIXED_SHIFT), SCREEN_HEIGHT2 + (FIXED_MUL(p3->y, zoom) >> FIXED_SHIFT)};
	
	Gfx_BlendTexArb(tex, src, &s0, &s1, &s2, &s3, mode);
}

//Stage HUD functions
static void Stage_DrawHealth(s16 health, u8 i, s8 ox)
{
	//Check if we should use 'dying' frame
	s8 dying;
	if (ox < 0)
		dying = (health >= 18000) * 35;
	else
		dying = (health <= 2000) * 35;
	
	//Get src and dst
	fixed_t hx = (128 << FIXED_SHIFT) * (10000 - health) / 10000;
	RECT src = {
		(i % 3) * 70 + dying,
		48 + (i / 3) * 35,
		35,
		35
	};
	RECT_FIXED dst = {
		hx + ox * FIXED_DEC(15,1) - FIXED_DEC(16,1),
		FIXED_DEC(SCREEN_HEIGHT2 - 35 + 4 - 18, 1),
		src.w << FIXED_SHIFT,
		src.h << FIXED_SHIFT
	};
	if (stage.downscroll)
		dst.y = -dst.y - dst.h;
	
	//Draw health icon
	Stage_DrawTex(&stage.tex_hud1, &src, &dst, FIXED_MUL(stage.bump, stage.sbump));
}

static void Stage_DrawStrum(u8 i, RECT *note_src, RECT_FIXED *note_dst)
{
	(void)note_dst;
	
	PlayerState *this = &stage.player_state[(i & NOTE_FLAG_OPPONENT) != 0];
	i &= 0x3;
	
	if (this->arrow_hitan[i] > 0)
	{
		//Play hit animation
		u8 frame = ((this->arrow_hitan[i] << 1) / stage.step_time) & 1;
		note_src->x = (i + 1) << 5;
		note_src->y = 64 - (frame << 5);
		
		this->arrow_hitan[i] -= timer_dt;
		if (this->arrow_hitan[i] <= 0)
		{
			if (this->pad_held & note_key[i])
				this->arrow_hitan[i] = 1;
			else
				this->arrow_hitan[i] = 0;
		}
	}
	else if (this->arrow_hitan[i] < 0)
	{
		//Play depress animation
		note_src->x = (i + 1) << 5;
		note_src->y = 96;
		if (!(this->pad_held & note_key[i]))
			this->arrow_hitan[i] = 0;
	}
	else
	{
		note_src->x = 0;
		note_src->y = i << 5;
	}
}

static void Stage_DrawNotes(void)
{
	//Check if opponent should draw as bot
	u8 bot = (stage.mode == StageMode_2P) ? 0 : NOTE_FLAG_OPPONENT;
	
	//Initialize scroll state
	SectionScroll scroll;
	scroll.start = stage.time_base;
	
	Section *scroll_section = stage.section_base;
	Stage_GetSectionScroll(&scroll, scroll_section);
	
	//Push scroll back until cur_note is properly contained
	while (scroll.start_step > stage.cur_note->pos)
	{
		//Look for previous section
		Section *prev_section = Stage_GetPrevSection(scroll_section);
		if (prev_section == NULL)
			break;
		
		//Push scroll back
		scroll_section = prev_section;
		Stage_GetSectionScroll(&scroll, scroll_section);
		scroll.start -= scroll.length;
	}
	
	//Draw notes
	for (Note *note = stage.cur_note; note->pos != 0xFFFF; note++)
	{
		//Update scroll
		while (note->pos >= scroll_section->end)
		{
			//Push scroll forward
			scroll.start += scroll.length;
			Stage_GetSectionScroll(&scroll, ++scroll_section);
		}

		
		//Get note information
		u8 i = (note->type & NOTE_FLAG_OPPONENT) != 0;
		PlayerState *this = &stage.player_state[i];
		
		fixed_t note_fp = (fixed_t)note->pos << FIXED_SHIFT;
		fixed_t time = (scroll.start - stage.song_time) + (scroll.length * (note->pos - scroll.start_step) / scroll.length_step);
		fixed_t y = note_y + FIXED_MUL(stage.speed, time * 150);
		
		//Check if went above screen
		if (y < FIXED_DEC(-16 - SCREEN_HEIGHT2, 1))
		{
			//Wait for note to exit late time
			if (note_fp + stage.late_safe >= stage.note_scroll)
				continue;
			
			//Miss note if player's note
			if (!(note->type & (bot | NOTE_FLAG_HIT | NOTE_FLAG_MINE)))
			{
				if (stage.mode < StageMode_Net1 || i == ((stage.mode == StageMode_Net1) ? 0 : 1))
				{
					//Missed note
					Stage_CutVocal();
					Stage_MissNote(this);
					this->health -= 475;
					stage.notes_passed++;
					stage.misses++;
					stage.player_state[0].refresh_score = true;
					
					//Send miss packet
					#ifdef PSXF_NETWORK
						if (stage.mode >= StageMode_Net1)
						{
							//Send note hit packet
							Packet note_hit;
							note_hit[0] = PacketType_NoteMiss;
							note_hit[1] = 0xFF;
							
							note_hit[2] = this->score >> 0;
							note_hit[3] = this->score >> 8;
							note_hit[4] = this->score >> 16;
							note_hit[5] = this->score >> 24;
							
							Network_Send(&note_hit);
						}
					#endif
				}
			}
			
			//Update current note
			stage.cur_note++;
		}
		else
		{
			//Don't draw if below screen
			RECT note_src;
			RECT_FIXED note_dst;
			if (y > (FIXED_DEC(SCREEN_HEIGHT,2) + scroll.size) || note->pos == 0xFFFF)
				break;
			
			//Draw note
			if (note->type & NOTE_FLAG_SUSTAIN)
			{
				//Check for sustain clipping
				fixed_t clip;
				y -= scroll.size;
				if ((note->type & (bot | NOTE_FLAG_HIT)) || ((this->pad_held & note_key[note->type & 0x3]) && (note_fp + stage.late_sus_safe >= stage.note_scroll)))
				{
					clip = FIXED_DEC(32 - SCREEN_HEIGHT2, 1) - y;
					if (clip < 0)
						clip = 0;
				}
				else
				{
					clip = 0;
				}
				
				//Draw sustain
				if (note->type & NOTE_FLAG_SUSTAIN_END)
				{
					if (clip < (24 << FIXED_SHIFT))
					{
						note_src.x = 160;
						note_src.y = ((note->type & 0x3) << 5) + 4 + (clip >> FIXED_SHIFT);
						note_src.w = 32;
						note_src.h = 28 - (clip >> FIXED_SHIFT);
						
					
					    note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
					    note_dst.y = y + clip;
					    note_dst.w = note_src.w << FIXED_SHIFT;
					    note_dst.h = (note_src.h << FIXED_SHIFT);
					
						
						if (stage.downscroll)
						{
							note_dst.y = -note_dst.y;
							note_dst.h = -note_dst.h;
						}
						Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
					}
				}
				else
				{
					//Get note height
					fixed_t next_time = (scroll.start - stage.song_time) + (scroll.length * (note->pos + 12 - scroll.start_step) / scroll.length_step);
					fixed_t next_y = note_y + FIXED_MUL(stage.speed, next_time * 150) - scroll.size;
					fixed_t next_size = next_y - y;
					
					if (clip < next_size)
					{
						note_src.x = 160;
						note_src.y = ((note->type & 0x3) << 5);
						note_src.w = 32;
						note_src.h = 16;
						
						note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
						note_dst.y = y + clip;
						note_dst.w = note_src.w << FIXED_SHIFT;
						note_dst.h = (next_y - y) - clip;
						
						if (stage.downscroll)
							note_dst.y = -note_dst.y - note_dst.h;
						Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
					}
				}
			}
			else if (note->type & NOTE_FLAG_FLIPX)
			{
				//Don't draw if already hit
				if (note->type & NOTE_FLAG_HIT)
					continue;
				
				//Draw note
				note_src.x = 32 + ((note->type & 0x3) << 5);
				note_src.y = 0;
				note_src.w = 32;
				note_src.h = 32;
			
			    note_dst.x = note_flip[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
			    note_dst.y = y - FIXED_DEC(16,1);
			    note_dst.w = note_src.w << FIXED_SHIFT;
			    note_dst.h = note_src.h << FIXED_SHIFT;
			
				if (stage.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
			}
			else if (note->type & NOTE_FLAG_NOFLIP)
			{
				//Don't draw if already hit
				if (note->type & NOTE_FLAG_HIT)
					continue;
				
				//Draw note
				note_src.x = 32 + ((note->type & 0x3) << 5);
				note_src.y = 0;
				note_src.w = 32;
				note_src.h = 32;
			
			    note_dst.x = note_norm[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
			    note_dst.y = y - FIXED_DEC(16,1);
			    note_dst.w = note_src.w << FIXED_SHIFT;
			    note_dst.h = note_src.h << FIXED_SHIFT;
			
				if (stage.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
			}
			else if (note->type & NOTE_FLAG_MINE)
			{
				//Don't draw if already hit
				if (note->type & NOTE_FLAG_HIT)
					continue;
				
				//Draw note body
				note_src.x = 192 + ((note->type & 0x1) << 5);
				note_src.y = (note->type & 0x2) << 4;
				note_src.w = 32;
				note_src.h = 32;
				
				note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
				note_dst.y = y - FIXED_DEC(16,1);
				note_dst.w = note_src.w << FIXED_SHIFT;
				note_dst.h = note_src.h << FIXED_SHIFT;
				
				if (stage.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
				
				if (stage.stage_id == StageId_1_4)
				{
					//Draw note halo
					note_src.x = 160;
					note_src.y = 128 + ((animf_count & 0x3) << 3);
					note_src.w = 32;
					note_src.h = 8;
					
					note_dst.y -= FIXED_DEC(6,1);
					note_dst.h >>= 2;
					
					Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
				}
				else
				{
					//Draw note fire
					note_src.x = 192 + ((animf_count & 0x1) << 5);
					note_src.y = 64 + ((animf_count & 0x2) * 24);
					note_src.w = 32;
					note_src.h = 48;
					
					if (stage.downscroll)
					{
						note_dst.y += note_dst.h;
						note_dst.h = note_dst.h * -3 / 2;
					}
					else
					{
						note_dst.h = note_dst.h * 3 / 2;
					}
					Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
				}
			}
			else
			{
				//Don't draw if already hit
				if (note->type & NOTE_FLAG_HIT)
					continue;
				
				//Draw note
				note_src.x = 32 + ((note->type & 0x3) << 5);
				note_src.y = 0;
				note_src.w = 32;
				note_src.h = 32;
			
			    if (stage.stage_id == StageId_1_2)
				{
					note_dst.x = note_norm[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
				}
				else
					note_dst.x = note_x[(note->type & 0x7) ^ stage.note_swap] - FIXED_DEC(16,1);
			    note_dst.y = y - FIXED_DEC(16,1);
			    note_dst.w = note_src.w << FIXED_SHIFT;
			    note_dst.h = note_src.h << FIXED_SHIFT;
			
				if (stage.downscroll)
					note_dst.y = -note_dst.y - note_dst.h;
				Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
			}
		}
	}
}

void Stage_DrawBox()
{
	RECT dia_src = {0, 0, 227, 63};
	RECT_FIXED dia_dst = {FIXED_DEC(0,1), FIXED_DEC(0,1), FIXED_DEC(120,1), FIXED_DEC(63,1)};

	Stage_DrawTex(&stage.tex_dia, &dia_src, &dia_dst, stage.bump);
}

void PsyTalk_Draw(Stage *this, fixed_t x, fixed_t y)
{
	//Draw animated object
	const CharFrame *cframe = &psytalk_frame[this->psytalk_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&stage.tex_psytalk, &src, &dst, stage.camera.bzoom);
}

void PsyTalk_SetFrame(void *user, u8 frame)
{
	Stage *this = (Stage*)user;
	
	//Check if this is a new frame
	if (frame != stage.psytalk_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &psytalk_frame[this->psytalk_frame = frame];
		if (cframe->tex != this->psytalk_tex_id)
			Gfx_LoadTex(&this->tex_psytalk, this->arc_psytalk_ptr[this->psytalk_tex_id = cframe->tex], 0);
	}
}

//Stage loads
static void Stage_SwapChars(void)
{
	if (stage.mode == StageMode_Swap)
	{
		Character *temp = stage.player;
		stage.player = stage.opponent;
		stage.opponent = temp;
	}
}

static void Stage_LoadPlayer(void)
{
	//Load player character
	Character_Free(stage.player);
	stage.player = stage.stage_def->pchar.new(stage.stage_def->pchar.x, stage.stage_def->pchar.y);
}

static void Stage_LoadOpponent(void)
{
	//Load opponent character
	Character_Free(stage.opponent);
	stage.opponent = stage.stage_def->ochar.new(stage.stage_def->ochar.x, stage.stage_def->ochar.y);
}

static void Stage_LoadGirlfriend(void)
{
	//Load girlfriend character
	Character_Free(stage.gf);
	if (stage.stage_def->gchar.new != NULL)
		stage.gf = stage.stage_def->gchar.new(stage.stage_def->gchar.x, stage.stage_def->gchar.y);
	else
		stage.gf = NULL;
}

static void Stage_LoadStage(void)
{
	//Load back
	if (stage.back != NULL)
		stage.back->free(stage.back);
	stage.back = stage.stage_def->back();
}

static void Stage_LoadChart(void)
{
	//Load stage data
	char chart_path[64];
	
	//Use standard path convention
	sprintf(chart_path, "\\WEEK%d\\%d.%d%c.CHT;1", stage.stage_def->week, stage.stage_def->week, stage.stage_def->week_song, "ENH"[stage.stage_diff]);
	
	
	if (stage.chart_data != NULL)
		Mem_Free(stage.chart_data);
	stage.chart_data = IO_Read(chart_path);
	u8 *chart_byte = (u8*)stage.chart_data;
	
	#ifdef PSXF_PC
		//Get lengths
		u16 note_off = chart_byte[0] | (chart_byte[1] << 8);
		
		u8 *section_p = chart_byte + 2;
		u8 *note_p = chart_byte + note_off;
		
		u8 *section_pp;
		u8 *note_pp;
		
		size_t sections = (note_off - 2) >> 2;
		size_t notes = 0;
		
		for (note_pp = note_p;; note_pp += 4)
		{
			notes++;
			u16 pos = note_pp[0] | (note_pp[1] << 8);
			if (pos == 0xFFFF)
				break;
		}
		
		if (notes)
			stage.num_notes = notes - 1;
		else
			stage.num_notes = 0;
		
		//Realloc for separate structs
		size_t sections_size = sections * sizeof(Section);
		size_t notes_size = notes * sizeof(Note);
		size_t notes_off = MEM_ALIGN(sections_size);
		
		u8 *nchart = Mem_Alloc(notes_off + notes_size);
		
		Section *nsection_p = stage.sections = (Section*)nchart;
		section_pp = section_p;
		for (size_t i = 0; i < sections; i++, section_pp += 4, nsection_p++)
		{
			nsection_p->end = section_pp[0] | (section_pp[1] << 8);
			nsection_p->flag = section_pp[2] | (section_pp[3] << 8);
		}
		
		Note *nnote_p = stage.notes = (Note*)(nchart + notes_off);
		note_pp = note_p;
		for (size_t i = 0; i < notes; i++, note_pp += 4, nnote_p++)
		{
			nnote_p->pos = note_pp[0] | (note_pp[1] << 8);
			nnote_p->type = note_pp[2] | (note_pp[3] << 8);
		}
		
		//Use reformatted chart
		Mem_Free(stage.chart_data);
		stage.chart_data = (IO_Data)nchart;
	#else
		//Directly use section and notes pointers
		stage.sections = (Section*)(chart_byte + 2);
		stage.notes = (Note*)(chart_byte + *((u16*)stage.chart_data));
		
		for (Note *note = stage.notes; note->pos != 0xFFFF; note++)
			stage.num_notes++;
	#endif
	
	//Swap chart
	if (stage.mode == StageMode_Swap)
	{
		for (Note *note = stage.notes; note->pos != 0xFFFF; note++)
			note->type ^= NOTE_FLAG_OPPONENT;
		for (Section *section = stage.sections;; section++)
		{
			section->flag ^= SECTION_FLAG_OPPFOCUS;
			if (section->end == 0xFFFF)
				break;
		}
	}
	
	//Count max scores
	stage.player_state[0].max_score = 0;
	stage.player_state[1].max_score = 0;
	for (Note *note = stage.notes; note->pos != 0xFFFF; note++)
	{
		if (note->type & (NOTE_FLAG_SUSTAIN | NOTE_FLAG_MINE))
			continue;
		if (note->type & NOTE_FLAG_OPPONENT)
			stage.player_state[1].max_score += 35;
		else
			stage.player_state[0].max_score += 35;
	}
	if (stage.mode >= StageMode_2P && stage.player_state[1].max_score > stage.player_state[0].max_score)
		stage.max_score = stage.player_state[1].max_score;
	else
		stage.max_score = stage.player_state[0].max_score;
	
	stage.cur_section = stage.sections;
	stage.cur_note = stage.notes;
	
	stage.speed = stage.stage_def->speed[stage.stage_diff];
	
	stage.step_crochet = 0;
	stage.time_base = 0;
	stage.step_base = 0;
	stage.section_base = stage.cur_section;
	Stage_ChangeBPM(stage.cur_section->flag & SECTION_FLAG_BPM_MASK, 0);
}

static void Stage_LoadMusic(void)
{
	//Offset sing ends
	stage.player->sing_end -= stage.note_scroll;
	stage.opponent->sing_end -= stage.note_scroll;
	if (stage.gf != NULL)
		stage.gf->sing_end -= stage.note_scroll;
	
	//Find music file and begin seeking to it
	Audio_SeekXA_Track(stage.stage_def->music_track);
	
	//Initialize music state
	stage.note_scroll = FIXED_DEC(-5 * 4 * 12,1);
	stage.song_time = FIXED_DIV(stage.note_scroll, stage.step_crochet);
	stage.interp_time = 0;
	stage.interp_ms = 0;
	stage.interp_speed = 0;
	
	//Offset sing ends again
	stage.player->sing_end += stage.note_scroll;
	stage.opponent->sing_end += stage.note_scroll;
	if (stage.gf != NULL)
		stage.gf->sing_end += stage.note_scroll;
}

static void Stage_LoadState(void)
{
	//Initialize stage state
	stage.flag = STAGE_FLAG_VOCAL_ACTIVE;
	
	stage.gf_speed = 1 << 2;

	//check if song has dialogue
	if (stage.story)
	{
		printf("Checking for dialogue...\n");
		if (stage.stage_def->dialogue == 1)
		{
			printf("Dialogue found!\n");
			Stage_LoadDia();
			stage.state = StageState_Dialogue;
		}
		else
		{
			printf("Dialogue not found.\n");
		    stage.state = StageState_Play;
		}
	}
	else
	{
		FontData_Load(&stage.font_cdr, Font_CDR);
	   	stage.state = StageState_Play;
	}

	//prepare timer
	//set variables for counting down
	switch (stage.stage_id)
	{
		case StageId_1_1:
		{
			time.mindown = 1;
			time.secdown = 58;
			break;
		}
		default:
			break;
	}

	//reset timer
	ResetTimer();

	//set converted time
	switch (stage.stage_id)
	{
		case StageId_1_1:
		{
			time.converted_time = 118;
			break;
		}
		case StageId_1_2:
		{
			time.converted_time = 114;
			break;
		}
		case StageId_1_3:
		{
			time.converted_time = 120;
		}
		default:
			break;
	}

	
	stage.player_state[0].character = stage.player;
	stage.player_state[1].character = stage.opponent;
	for (int i = 0; i < 2; i++)
	{
		memset(stage.player_state[i].arrow_hitan, 0, sizeof(stage.player_state[i].arrow_hitan));
		
		stage.player_state[i].health = 10000;
		stage.player_state[i].combo = 0;
		
		stage.player_state[i].refresh_score = false;
		stage.player_state[i].score = 0;
		strcpy(stage.player_state[i].score_text, "0");
		
		stage.delect = 0;
		
		stage.player_state[i].pad_held = stage.player_state[i].pad_press = 0;
	}

	stage.misses = 0;
	stage.notes_passed = 0;
	stage.notes_played = 0;

	//refresh score for psych hud
	stage.player_state[0].refresh_score = true;
	
	ObjectList_Free(&stage.objlist_splash);
	ObjectList_Free(&stage.objlist_fg);
	ObjectList_Free(&stage.objlist_bg);

	stage.notemode = 0;
}

int note1x = 26;
int note2x = 60;
int note3x = 94;
int note4x = 128;

int note5x = -128;
int note6x = -94;
int note7x = -60;
int note8x = -26;

//Welcome to the shitshow 2: Electric Boogaloo
void Stage_Note_Move(void)
{

	note_x[0] = FIXED_DEC(note1x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
	note_x[1] = FIXED_DEC(note2x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
	note_x[2] = FIXED_DEC(note3x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
	note_x[3] = FIXED_DEC(note4x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
	note_x[4] = FIXED_DEC(note5x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
	note_x[5] = FIXED_DEC(note6x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
	note_x[6] = FIXED_DEC(note7x,1) + FIXED_DEC(SCREEN_WIDEADD,4);
	note_x[7] = FIXED_DEC(note8x,1) + FIXED_DEC(SCREEN_WIDEADD,4);

	switch (stage.notemode)
	{
		case 0: //normal, just for when stage starts
		{
			note1x = 26;
  			note2x = 60;
  			note3x = 94;
  			note4x = 128;

			note5x = -128;
			note6x = -94;
			note7x = -60;
			note8x = -26;
			break;
		}
		case 1: //left and right flip
		{
			if (note1x < 128)
				note1x += 8;
			if (note4x > 26)
				note4x -= 8;
			//READJUST CUZ' IT LANDS LIKE 2 PIXELS OFF
			if (note1x == 130)
				note1x = 128;
			if (note4x == 24)
				note4x = 26;
			break;
		}
		case 2: //return to normal
		{
			if (note1x > 26)
				note1x -= 8;
			if (note1x == 24)
				note1x = 26;
			
			if (note2x > 60)
				note2x -= 4;
			if (note2x == 58)
				note2x = 60;

			if (note3x < 94)
				note3x += 4;
			if (note3x == 96)
				note3x = 94;

			if (note4x < 128)
				note4x += 8;
			if (note4x == 130)
				note4x = 128;
			break;
		}
		case 3: //full flip
		{
			if (note1x < 128)
				note1x += 8;

			if (note2x < 94)
				note2x += 4;

			if (note3x > 60)
				note3x -= 4;

			if (note4x > 26)
				note4x -= 8;

			//READJUST CUZ' IT LANDS LIKE 2 PIXELS OFF
			if (note1x == 130)
				note1x = 128;
			if (note2x == 96)
				note2x = 94;
			if (note3x == 58)
				note3x = 60;
			if (note4x == 24)
				note4x = 26;
			break;
		}
		case 4: //swap player 1 and 2's notes
		{
			if (note1x > -128)
				note1x -= 8;
			
			if (note2x > -94)
				note2x -= 8;
			
			if (note3x > -60)
				note3x -= 8;

			if (note4x > -26)
				note4x -= 8;

			if (note5x < 26)
				note5x += 8;

			if (note6x < 60)
				note6x += 8;

			if (note7x < 94)
				note7x += 8;

			if (note8x < 128)
				note8x += 8;
			//READJUUUUUUUUUST
			if (note1x == -134)
				note1x = -128;

			if (note2x == -100)
				note2x = -94;

			if (note3x == -66)
				note3x = -60;

			if (note4x == -32)
				note4x = -26;

			if (note5x == 32)
				note5x = 26;

			if (note6x == 66)
				note6x = 60;

			if (note7x == 100)
				note7x = 94;
				
			if (note8x == 134)
				note8x = 128;
			break;
		}
		case 5:
		{
			if (note1x < 26)
				note1x += 8;
			
			if (note2x < 60)
				note2x += 8;
			
			if (note3x < 94)
				note3x += 8;

			if (note4x < 128)
				note4x += 8;

			if (note5x > -128)
				note5x -= 8;

			if (note6x > -94)
				note6x -= 8;

			if (note7x > -60)
				note7x -= 8;

			if (note8x > -26)
				note8x -= 8;
			//it's such a baaaad sign
			if (note5x == -134)
				note5x = -128;

			if (note6x == -100)
				note6x = -94;

			if (note7x == -66)
				note7x = -60;

			if (note8x == -32)
				note8x = -26;

			if (note1x == 32)
				note1x = 26;

			if (note2x == 66)
				note2x = 60;

			if (note3x == 100)
				note3x = 94;
				
			if (note4x == 134)
				note4x = 128;
		}
	}
}

//Stage functions
void Stage_Load(StageId id, StageDiff difficulty, boolean story)
{
	//Get stage definition
	stage.stage_def = &stage_defs[stage.stage_id = id];
	stage.stage_diff = difficulty;
	stage.story = story;
	
	//Load HUD textures
	Gfx_LoadTex(&stage.tex_hud0, IO_Read("\\STAGE\\HUD0.TIM;1"), GFX_LOADTEX_FREE);
	Gfx_LoadTex(&stage.tex_hud1, IO_Read("\\STAGE\\HUD1.TIM;1"), GFX_LOADTEX_FREE);
	
	//Load stage background
	Stage_LoadStage();
	
	//Load characters
	Stage_LoadPlayer();
	Stage_LoadOpponent();
	Stage_LoadGirlfriend();
	Stage_SwapChars();
	
	//Load stage chart
	Stage_LoadChart();
	
	//Initialize stage state
	stage.story = story;
	
	Stage_LoadState();
	
	//Initialize camera
	if (stage.cur_section->flag & SECTION_FLAG_OPPFOCUS)
		Stage_FocusCharacter(stage.opponent, FIXED_UNIT);
	else
		Stage_FocusCharacter(stage.player, FIXED_UNIT);
	stage.camera.x = stage.camera.tx;
	stage.camera.y = stage.camera.ty;
	stage.camera.zoom = stage.camera.tz;
	
	stage.bump = FIXED_UNIT;
	stage.sbump = FIXED_UNIT;
	
	//Initialize stage according to mode
	stage.note_swap = (stage.mode == StageMode_Swap) ? 4 : 0;
	
	//Load music
	stage.note_scroll = 0;
	Stage_LoadMusic();
	
	//Test offset
	stage.offset = 0;
	
	#ifdef PSXF_NETWORK
	if (stage.mode >= StageMode_Net1 && Network_IsHost())
	{
		//Send ready packet to peer
		Packet ready;
		ready[0] = PacketType_Ready;
		ready[1] = id;
		ready[2] = difficulty;
		ready[3] = (stage.mode == StageMode_Net1) ? 1 : 0;
		Network_Send(&ready);
	}
	#endif
}

void Stage_Unload(void)
{
	printf("Unloading Stage\n");
	//Disable net mode to not break the game
	if (stage.mode >= StageMode_Net1)
		stage.mode = StageMode_Normal;
	
	//Unload stage background
	if (stage.back != NULL)
		stage.back->free(stage.back);
	stage.back = NULL;
	
	//Unload stage data
	Mem_Free(stage.chart_data);
	stage.chart_data = NULL;
	
	//Free objects
	ObjectList_Free(&stage.objlist_splash);
	ObjectList_Free(&stage.objlist_fg);
	ObjectList_Free(&stage.objlist_bg);
	
	//Free characters
	Character_Free(stage.player);
	stage.player = NULL;
	Character_Free(stage.opponent);
	stage.opponent = NULL;
	Character_Free(stage.gf);
	stage.gf = NULL;
}

static boolean Stage_NextLoad(void)
{
	u8 load = stage.stage_def->next_load;
	if (load == 0)
	{
		//Do stage transition if full reload
		stage.trans = StageTrans_NextSong;
		Trans_Start();
		return false;
	}
	else
	{
		//Get stage definition
		stage.stage_def = &stage_defs[stage.stage_id = stage.stage_def->next_stage];
		
		//Load stage background
		if (load & STAGE_LOAD_STAGE)
			Stage_LoadStage();
		
		//Load characters
		Stage_SwapChars();
		if (load & STAGE_LOAD_PLAYER)
		{
			Stage_LoadPlayer();
		}
		else
		{
			stage.player->x = stage.stage_def->pchar.x;
			stage.player->y = stage.stage_def->pchar.y;
		}
		if (load & STAGE_LOAD_OPPONENT)
		{
			Stage_LoadOpponent();
		}
		else
		{
			stage.opponent->x = stage.stage_def->ochar.x;
			stage.opponent->y = stage.stage_def->ochar.y;
		}
		Stage_SwapChars();
		if (load & STAGE_LOAD_GIRLFRIEND)
		{
			Stage_LoadGirlfriend();
		}
		else if (stage.gf != NULL)
		{
			stage.gf->x = stage.stage_def->gchar.x;
			stage.gf->y = stage.stage_def->gchar.y;
		}
		
		//Load stage chart
		Stage_LoadChart();
		
		//Initialize stage state
		Stage_LoadState();
		
		//Load music
		Stage_LoadMusic();
		
		//Reset timer
		Timer_Reset();
		return true;
	}
}

//load dialogue related files
void Stage_LoadDia(void)
{
	Stage *this = (Stage*)Mem_Alloc(sizeof(Stage));

	Gfx_LoadTex(&stage.tex_dia, IO_Read("\\STAGE\\DIA.TIM;1"), GFX_LOADTEX_FREE);

	//load different assets depending on stage
	switch (stage.stage_id)
	{
		case StageId_1_1:
		{
			Animatable_Init(&stage.psytalk_animatable, psytalk_anim);
	        Animatable_SetAnim(&stage.psytalk_animatable, 0);
	
			this->arc_psytalk = IO_Read("\\DIA\\PSYTALK.ARC;1");
			this->arc_psytalk_ptr[0] = Archive_Find(this->arc_psytalk, "talk.tim");
			this->arc_psytalk_ptr[1] = Archive_Find(this->arc_psytalk, "piss.tim");
			this->arc_psytalk_ptr[2] = Archive_Find(this->arc_psytalk, "erect0.tim");
			this->arc_psytalk_ptr[3] = Archive_Find(this->arc_psytalk, "erect1.tim");
			this->arc_psytalk_ptr[4] = Archive_Find(this->arc_psytalk, "annoy.tim");
			this->arc_psytalk_ptr[5] = Archive_Find(this->arc_psytalk, "confuse.tim");
			this->arc_psytalk_ptr[6] = Archive_Find(this->arc_psytalk, "shock.tim");
	
	        //Initialize render state
	        this->psytalk_tex_id = this->psytalk_frame = 0xFF;
			break;
		}
		default:
		    break;
	}

	FontData_Load(&stage.font_arial, Font_Arial);
}

void Stage_Tick(void)
{
	SeamLoad:;
	
	//Tick transition
	#ifdef PSXF_NETWORK
	if (stage.mode >= StageMode_Net1)
	{
		//Show disconnect screen when disconnected
		if (!(Network_Connected() && Network_HasPeer()))
		{
			stage.trans = StageTrans_Disconnect;
			Trans_Start();
		}
	}
	else
	#endif
	{
		//Return to menu when start is pressed
		//Or skip dialogue, if applicable
		if (pad_state.press & PAD_START)
		{
			switch (stage.state)
			{
				case StageState_Play:
				    stage.trans = StageTrans_Menu;
					Trans_Start();
					break;
				case StageState_Dialogue:
				    break;
				default:
				    stage.trans = StageTrans_Reload;
					Trans_Start();
					break;
				    
			}
		}
	}
	
	if (Trans_Tick())
	{
		switch (stage.trans)
		{
			case StageTrans_Menu:
			    if (stage.demo == 1)
				{
			    	//Load appropriate menu
			    	Stage_Unload();
			    	
			    	LoadScr_Start();
			    	#ifdef PSXF_NETWORK
			    	if (Network_Connected())
			    	{
			    		if (Network_IsHost())
			    			Demu_Load(DemuPage_NetOp);
			    		else
			    			Demu_Load(DemuPage_NetLobby);
			    	}
			    	else
			    	#endif
			    	{
			    		if (stage.stage_id <= StageId_LastVanilla)
			    		{
			    			if (stage.story)
			    				Demu_Load(DemuPage_Story);
			    			else
			    				Demu_Load(DemuPage_Freeplay);
			    		}
			    		else
			    		{
			    			Demu_Load(DemuPage_Credits);
			    		}
			    	}
			    	LoadScr_End();
			    	
			    	gameloop = GameLoop_Demu;
			    	return;
				}
				else
				{
			    	//Load appropriate menu
			    	Stage_Unload();
					//play end if uproar
					if (stage.stage_id == StageId_1_3 && stage.story)
						Movie_Play("\\STR\\END.STR;1", 2313);
			    	
			    	LoadScr_Start();
			    	#ifdef PSXF_NETWORK
			    	if (Network_Connected())
			    	{
			    		if (Network_IsHost())
			    			Menu_Load(MenuPage_NetOp);
			    		else
			    			Menu_Load(MenuPage_NetLobby);
			    	}
			    	else
			    	#endif
			    	{
			    		if (stage.stage_id <= StageId_LastVanilla)
			    		{
			    			if (stage.story)
			    				Menu_Load(MenuPage_Story);
			    			else
			    				Menu_Load(MenuPage_Freeplay);
			    		}
			    		else
			    		{
			    			Menu_Load(MenuPage_Credits);
			    		}
			    	}
			    	LoadScr_End();
			    	
			    	gameloop = GameLoop_Menu;
			    	return;
				}
			case StageTrans_NextSong:
				//Load next song
				Stage_Unload();
				
				LoadScr_Start();
				Stage_Load(stage.stage_def->next_stage, stage.stage_diff, stage.story);
				LoadScr_End();
				break;
			case StageTrans_Reload:
				//Reload song
				Stage_Unload();
				
				LoadScr_Start();
				Stage_Load(stage.stage_id, stage.stage_diff, stage.story);
				LoadScr_End();
				break;
			case StageTrans_Disconnect:
		#ifdef PSXF_NETWORK
				//Disconnect screen
				Stage_Unload();
				
				LoadScr_Start();
				if (Network_Connected() && Network_IsHost())
					Menu_Load(MenuPage_NetOpWait);
				else
					Menu_Load(MenuPage_NetFail);
				LoadScr_End();
				
				gameloop = GameLoop_Menu;
		#endif
				return;
			case StageTrans_Movie:
			{
				Stage_Unload();

				LoadScr_Start();
				LoadScr_End();
				break;
			}
		}
	}
	
	switch (stage.state)
	{
		case StageState_Play:
		{
			//Clear per-frame flags
			stage.flag &= ~(STAGE_FLAG_JUST_STEP | STAGE_FLAG_SCORE_REFRESH);
			
			//Get song position
			boolean playing;
			fixed_t next_scroll;

			//trackers
			//pressing select switches selected tracker

			if (stage.song_step >= 0)
			{
				switch (time.timeropt)
				{
					case CountDown:
						SongTimerDown();
						break;
					case CountUp:
						SongTimer();
						break;
					case Disabled:
						break;
				}
			}

			switch (stage.debug)
			{
				case 1: //step counter
			        FntPrint("current step is %d\n", stage.notes_passed);
					break;
				case 2: //camera position
				    FntPrint("camera X %d Y %d zoom %d", stage.camera.x/1024, stage.camera.y/1024, stage.camera.zoom);
					break;
				case 3: //Player 1 (bf) position
				    FntPrint("player1 pos X %d Y %d", stage.player->x/1024, stage.player->y/1024);
					Stage_MoveChar();
					break;
				case 4: //Player 2 (dad) position
				    FntPrint("player2 pos X %d Y %d", stage.opponent->x/1024, stage.opponent->y/1024);
					Stage_MoveChar();
					break;
				case 5: //Bg char (gf) position
				    FntPrint("bg char pos X %d Y %d", stage.gf->x/1024, stage.gf->y/1024);
					break;
				case 6: //ass
					FntPrint("STRIKELINE X\n%d %d %d %d %d %d %d %d", note1x, note2x, note3x, note4x, note5x, note6x, note7x, note8x);
					break;
			}
			//FntPrint("uh %d", stage.fadeblack);

			Stage_Note_Move();


			if (stage.stage_id == StageId_1_1)
		    {
                switch (stage.song_step)
				{
		            case 190:
		                stage.notemode = 1;
				    	break;
					case 318:
						stage.notemode = 2;
						break;
					case 572:
					    stage.notemode = 3;
						break;
					case 701:
					    stage.notemode = 2;
						break;
				}
		    }

			if (stage.stage_id == StageId_1_2)
			{
				switch (stage.song_step)
				{
					case 381:
					    stage.notemode = 4;
						break;
					case 510:
					    stage.notemode = 5;
						break;
					case 702:
					    stage.notemode = 4;
						break;
					case 1022:
					    stage.notemode = 5;
						break;
					
				}
			}

			if (stage.stage_id == StageId_1_3)
			{
				switch (stage.song_step)
				{
					case 253:
					    stage.notemode = 4;
						break;
					case 378:
					    stage.notemode = 5;
						break;
					case 441:
					    stage.notemode = 1;
						break;
					case 509:
					    stage.notemode = 2;
						break;
					case 637:
					    stage.notemode = 4;
						break;
					case 765:
					    stage.notemode = 5;
						break;
					case 956:
					    stage.notemode = 3;
						break;
					case 1019:
					    stage.notemode = 2;
						break;
				}
			}

			if (stage.stage_id == StageId_2_2)
			{
				switch (stage.song_step)
				{
					case 531:
					    stage.notemode = 6;
						break;
					case 532:
					    stage.notemode = 7;
						break;
					case 533:
					    stage.notemode = 2;
					    break;
					case 767:
					    stage.notemode = 7;
						break;
					case 768:
					    stage.notemode = 6;
						break;
					case 769:
					    stage.notemode = 0;
						break;
				}
			}

			if (stage.stage_id == StageId_2_1)
			{
				switch (stage.song_step)
				{
					case 190:
					    stage.notemode = 6;
						break;
					case 191:
					    stage.notemode = 7;
						break;
		            case 192:
		                stage.notemode = 2;
				    	break;
					case 316:
					    stage.notemode = 7;
						break;
					case 317:
					    stage.notemode = 6;
						break;
				    case 318:
				        stage.notemode = 0;
				    	break;
					case 572:
					    stage.notemode = 6;
						break;
					case 573:
					    stage.notemode = 7;
						break;
					case 574:
					    stage.notemode = 2;
						break;
					case 701:
					    stage.notemode = 7;
						break;
					case 702:
					    stage.notemode = 6;
						break;
					case 703:
					    stage.notemode = 0;
						break;
				}
		    }



			if (stage.stage_id == StageId_1_1)
			{
			stage.healthe = 0;
			stage.healthp = 8;
			}
			else if (stage.stage_id == StageId_1_2)
			{
            stage.healthe = 0;
			stage.healthp = 16;
			}
			else if (stage.stage_id == StageId_1_3)
			{
			stage.healthe = 0;
			stage.healthp = 24;
			}
			else if (stage.stage_id == StageId_2_1)
			{
				stage.healthe = 40;
				stage.healthp = 32;
			}
			else if (stage.stage_id == StageId_2_2)
			{
			stage.healthe = 40;
			stage.healthp = 32;
			};
			
			if (stage.stage_id == StageId_1_1)
			   nohud = 0;
			if (stage.stage_id == StageId_1_2)
			   nohud = 0;
			if (stage.stage_id == StageId_1_3)
			   nohud = 0;
			if (stage.stage_id == StageId_1_4)
			   nohud = 1;
			if (stage.stage_id == StageId_1_5)
			   nohud = 1;
			if (stage.stage_id == StageId_1_6)
			   nohud = 1;
			if (stage.stage_id == StageId_2_1)
			   nohud = 0;
			if (stage.stage_id == StageId_2_2)
			   nohud = 0;
			if (stage.stage_id == StageId_2_3)
			   nohud = 1;

			if (stage.fadewhite > 0)
			{
         		static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
				u8 flash_col = stage.fadewhite >> FIXED_SHIFT;
				u8 flash_col2 = stage.fadeextra >> FIXED_SHIFT;
				Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 1);
				stage.fadewhite -= FIXED_MUL(stage.fadespeed, timer_dt*3);  

            
			}

			if (stage.stage_id == StageId_1_3)
			{
				switch (stage.song_step)
				{
					case 896:
					    {
					    	stage.fadewhite = FIXED_DEC(255,1);
			                stage.fadeextra = FIXED_DEC(0,1);
			                stage.fadespeed = FIXED_DEC(90,1);
							break;
					    }
					case 1150:
					    {
					    	stage.fadeblack = 265200;
				        	stage.fadeextra = FIXED_DEC(0,1);
				        	stage.fadespeed = FIXED_DEC(80,1);
					    	break;
					    }
					case 1213:
						{
							stage.fadeblack = 0;
							stage.fadewhite = FIXED_DEC(255,1);
							stage.fadespeed = FIXED_DEC(100,1);
						}
				}
			}




			
			#ifdef PSXF_NETWORK
			if (stage.mode >= StageMode_Net1 && !Network_IsReady())
			{
				if (!Network_IsHost())
				{
					//Send ready packet
					Packet ready;
					ready[0] = PacketType_Ready;
					Network_Send(&ready);
					Network_SetReady(true);
				}
				next_scroll = stage.note_scroll;
			}
			else
			#endif
			{
				const fixed_t interp_int = FIXED_UNIT * 8 / 75;
				if (stage.note_scroll < 0)
				{
					//Play countdown sequence
					stage.song_time += timer_dt;
					
					//Update song
					if (stage.song_time >= 0)
					{
						//Song has started
						playing = true;
						Audio_PlayXA_Track(stage.stage_def->music_track, 0x40, stage.stage_def->music_channel, 0);
						
						//Update song time
						fixed_t audio_time = (fixed_t)Audio_TellXA_Milli() - stage.offset;
						if (audio_time < 0)
							audio_time = 0;
						stage.interp_ms = (audio_time << FIXED_SHIFT) / 1000;
						stage.interp_time = 0;
						stage.song_time = stage.interp_ms;
					}
					else
					{
						//Still scrolling
						playing = false;
					}
					
					//Update scroll
					next_scroll = FIXED_MUL(stage.song_time, stage.step_crochet);
				}
				else if (Audio_PlayingXA())
				{
					fixed_t audio_time_pof = (fixed_t)Audio_TellXA_Milli();
					fixed_t audio_time = (audio_time_pof > 0) ? (audio_time_pof - stage.offset) : 0;
					
					if (stage.expsync)
					{
						//Get playing song position
						if (audio_time_pof > 0)
						{
							stage.song_time += timer_dt;
							stage.interp_time += timer_dt;
						}
						
						if (stage.interp_time >= interp_int)
						{
							//Update interp state
							while (stage.interp_time >= interp_int)
								stage.interp_time -= interp_int;
							stage.interp_ms = (audio_time << FIXED_SHIFT) / 1000;
						}
						
						//Resync
						fixed_t next_time = stage.interp_ms + stage.interp_time;
						if (stage.song_time >= next_time + FIXED_DEC(25,1000) || stage.song_time <= next_time - FIXED_DEC(25,1000))
						{
							stage.song_time = next_time;
						}
						else
						{
							if (stage.song_time < next_time - FIXED_DEC(1,1000))
								stage.song_time += FIXED_DEC(1,1000);
							if (stage.song_time > next_time + FIXED_DEC(1,1000))
								stage.song_time -= FIXED_DEC(1,1000);
						}
					}
					else
					{
						//Old sync
						stage.interp_ms = (audio_time << FIXED_SHIFT) / 1000;
						stage.interp_time = 0;
						stage.song_time = stage.interp_ms;
					}
					
					playing = true;
					
					//Update scroll
					next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(stage.song_time - stage.time_base, stage.step_crochet);
				}
				else
				{
					//Song has ended
					playing = false;
					stage.song_time += timer_dt;
					
					//Update scroll
					next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(stage.song_time - stage.time_base, stage.step_crochet);
					
					//Transition to menu or next song
					if (stage.story && stage.stage_def->next_stage != stage.stage_id)
					{
						if (Stage_NextLoad())
							goto SeamLoad;
					}
					else
					{
						stage.trans = StageTrans_Menu;
						Trans_Start();
					}
				}
			}
			
			RecalcScroll:;
			//Update song scroll and step
			if (next_scroll > stage.note_scroll)
			{
				if (((stage.note_scroll / 12) & FIXED_UAND) != ((next_scroll / 12) & FIXED_UAND))
					stage.flag |= STAGE_FLAG_JUST_STEP;
				stage.note_scroll = next_scroll;
				stage.song_step = (stage.note_scroll >> FIXED_SHIFT);
				if (stage.note_scroll < 0)
					stage.song_step -= 11;
				stage.song_step /= 12;
			}
			
			//Update section
			if (stage.note_scroll >= 0)
			{
				//Check if current section has ended
				u16 end = stage.cur_section->end;
				if ((stage.note_scroll >> FIXED_SHIFT) >= end)
				{
					//Increment section pointer
					stage.cur_section++;
					
					//Update BPM
					u16 next_bpm = stage.cur_section->flag & SECTION_FLAG_BPM_MASK;
					Stage_ChangeBPM(next_bpm, end);
					stage.section_base = stage.cur_section;
					
					//Recalculate scroll based off new BPM
					next_scroll = ((fixed_t)stage.step_base << FIXED_SHIFT) + FIXED_MUL(stage.song_time - stage.time_base, stage.step_crochet);
					goto RecalcScroll;
				}
			}
			
			//Handle bump
			if ((stage.bump = FIXED_UNIT + FIXED_MUL(stage.bump - FIXED_UNIT, FIXED_DEC(95,100))) <= FIXED_DEC(1003,1000))
				stage.bump = FIXED_UNIT;
			stage.sbump = FIXED_UNIT + FIXED_MUL(stage.sbump - FIXED_UNIT, FIXED_DEC(60,100));
			
			if (playing && (stage.flag & STAGE_FLAG_JUST_STEP))
			{
				//Check if screen should bump
				boolean is_bump_step = (stage.song_step & 0xF) == 0;

				//if stage is cutscene, or cool uproar moment, don't bump
				if (stage.stage_id == StageId_1_5)
					is_bump_step = NULL;
				if (stage.stage_id == StageId_1_3)
				{
					if (stage.song_step >= 1150 && stage.song_step <= 1213)
						is_bump_step = NULL;
				}
				
				//M.I.L.F bumps
				//if (stage.stage_id == StageId_4_3 && stage.song_step >= (168 << 2) && stage.song_step < (200 << 2))
				//	is_bump_step = (stage.song_step & 0x3) == 0;
				
				//Bump screen
				if (is_bump_step)
					stage.bump = FIXED_DEC(103,100);
				
				//Bump health every 4 steps
				if ((stage.song_step & 0x3) == 0)
					stage.sbump = FIXED_DEC(103,100);
			}
			
			//Scroll camera
			if (stage.cur_section->flag & SECTION_FLAG_OPPFOCUS)
				Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
			else
				Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
			Stage_ScrollCamera();
			
			switch (stage.mode)
			{
				case StageMode_Normal:
				case StageMode_Swap:
				{
					//Handle player 1 inputs
					Stage_ProcessPlayer(&stage.player_state[0], &pad_state, playing);
					
					//Handle opponent notes
					u8 opponent_anote = CharAnim_Idle;
					u8 opponent_snote = CharAnim_Idle;
					
					for (Note *note = stage.cur_note;; note++)
					{
						if (note->pos > (stage.note_scroll >> FIXED_SHIFT))
							break;
						
						//Opponent note hits
						if (playing && (note->type & NOTE_FLAG_OPPONENT) && !(note->type & NOTE_FLAG_HIT))
						{
							//Opponent hits note
							stage.player_state[1].arrow_hitan[note->type & 0x3] = stage.step_time;
							Stage_StartVocal();
							if (note->type & NOTE_FLAG_SUSTAIN)
								opponent_snote = note_anims[note->type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0];
							else
								opponent_anote = note_anims[note->type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0];
							note->type |= NOTE_FLAG_HIT;
						}
					}
					
					if (opponent_anote != CharAnim_Idle)
						stage.opponent->set_anim(stage.opponent, opponent_anote);
					else if (opponent_snote != CharAnim_Idle)
						stage.opponent->set_anim(stage.opponent, opponent_snote);
					break;
				}
				case StageMode_2P:
				{
					//Handle player 1 and 2 inputs
					Stage_ProcessPlayer(&stage.player_state[0], &pad_state, playing);
					Stage_ProcessPlayer(&stage.player_state[1], &pad_state_2, playing);
					break;
				}
			#ifdef PSXF_NETWORK
				case StageMode_Net1:
				{
					//Handle player 1 inputs
					Stage_ProcessPlayer(&stage.player_state[0], &pad_state, playing);
					break;
				}
				case StageMode_Net2:
				{
					//Handle player 2 inputs
					Stage_ProcessPlayer(&stage.player_state[1], &pad_state, playing);
					break;
				}
			#endif
			}
			
			//Tick note splashes
			ObjectList_Tick(&stage.objlist_splash);
			
			//Draw stage notes
			Stage_DrawNotes();
			
			//Draw note HUD
			RECT note_src = {0, 0, 32, 32};
			RECT_FIXED note_dst = {0, note_y - FIXED_DEC(16,1), FIXED_DEC(32,1), FIXED_DEC(32,1)};
			if (stage.downscroll)
				note_dst.y = -note_dst.y - note_dst.h;
			
			for (u8 i = 0; i < 4; i++)
			{
			
			   //BF
			   note_dst.x = note_x[i ^ stage.note_swap] - FIXED_DEC(16,1);
			   Stage_DrawStrum(i, &note_src, &note_dst);
			   Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
			   
			   //Opponent
			   note_dst.x = note_x[(i | 0x4) ^ stage.note_swap] - FIXED_DEC(16,1);
			   Stage_DrawStrum(i | 4, &note_src, &note_dst);
			   Stage_DrawTex(&stage.tex_hud0, &note_src, &note_dst, stage.bump);
			
			}
			
			//Draw score
			for (int i = 0; i < ((stage.mode >= StageMode_2P) ? 2 : 1); i++)
			{
				PlayerState *this = &stage.player_state[i];
				

				if (stage.coolhud == 0)
				{
					//Get string representing number
					if (this->refresh_score)
					{
						if (this->score != 0)
							sprintf(this->score_text, "%d0", this->score * stage.max_score / this->max_score);
						else
							strcpy(this->score_text, "0");
						this->refresh_score = false;
					}
					
					//Display score
					RECT score_src = {80, 224, 40, 10};
					RECT_FIXED score_dst = {(i ^ (stage.mode == StageMode_Swap)) ? FIXED_DEC(-100,1) : FIXED_DEC(14,1), (SCREEN_HEIGHT2 - 42) << FIXED_SHIFT, FIXED_DEC(40,1), FIXED_DEC(10,1)};
					if (stage.downscroll)
						score_dst.y = -score_dst.y - score_dst.h;
					
					Stage_DrawTex(&stage.tex_hud0, &score_src, &score_dst, stage.bump);
					
					//Draw number
					score_src.y = 240;
					score_src.w = 8;
					score_dst.x += FIXED_DEC(40,1);
					score_dst.w = FIXED_DEC(8,1);
					
					for (const char *p = this->score_text; ; p++)
					{
						//Get character
						char c = *p;
						if (c == '\0')
							break;
						
						//Draw character
						if (c == '-')
							score_src.x = 160;
						else //Should be a number
							score_src.x = 80 + ((c - '0') << 3);
						
						Stage_DrawTex(&stage.tex_hud0, &score_src, &score_dst, stage.bump);
						
						//Move character right
						score_dst.x += FIXED_DEC(7,1);
					}
				}
				else
				{
					if (this->refresh_score)
					{
						if (this->score != 0)
						{

							//Calculate accuracy by dividing the notes that have passed by the notes that were actually hit, and then multiplying that by 100
							stage.ratingpercent = stage.notes_played * 100 / stage.notes_passed;

							//get rating
							//can't use a switch case because it needs less than and greater than, so this SUCKS
							if (stage.ratingpercent >= 0 && stage.ratingpercent <= 19) //You Suck!
								stage.ratingselect = 0;
							else if (stage.ratingpercent >= 20 && stage.ratingpercent <= 39) //Shit
								stage.ratingselect = 1;
							else if (stage.ratingpercent >= 40 && stage.ratingpercent <= 49) //Bad
								stage.ratingselect = 2;
							else if (stage.ratingpercent >= 50 && stage.ratingpercent <= 59) //Bruh
								stage.ratingselect = 3;
							else if (stage.ratingpercent >= 60 && stage.ratingpercent <= 68) //Meh
								stage.ratingselect = 4;
							else if (stage.ratingpercent == 69) //Nice
								stage.ratingselect = 5;
							else if (stage.ratingpercent >= 70 && stage.ratingpercent <= 79) //Good
								stage.ratingselect = 6;
							else if (stage.ratingpercent >= 80 && stage.ratingpercent <= 89) //Great
								stage.ratingselect = 7;
							else if (stage.ratingpercent >= 90 && stage.ratingpercent <= 99) //Sick!
								stage.ratingselect = 8;
							else if (stage.ratingpercent ==100) //Perfect!!
								stage.ratingselect = 9;

							sprintf(this->score_text, "Score:%d0  |  Misses:%d  |  Rating:%s (%d%%)", this->score * stage.max_score / this->max_score, stage.misses, ratings[stage.ratingselect].text, stage.ratingpercent);
						}
						else
							sprintf(this->score_text, "Score:0  |  Misses:?  |  Rating:? (?%%)");
						this->refresh_score = false;
					}

					//Display Score
					stage.font_cdr.draw(&stage.font_cdr, this->score_text, 65, 220, FontAlign_Left);
				}

			}
			
			if (stage.mode < StageMode_2P)
			{
				//Perform health checks
				if (stage.player_state[0].health <= 0)
				{
					//Player has died
					stage.player_state[0].health = 0;
					stage.state = StageState_Dead;
				}
				if (stage.player_state[0].health > 20000)
					stage.player_state[0].health = 20000;
				
				//Draw health heads
				Stage_DrawHealth(stage.player_state[0].health, stage.player->health_i,    1);
				Stage_DrawHealth(stage.player_state[0].health, stage.opponent->health_i, -1);
				
				//Draw health bar
				RECT health_fill = {0, stage.healthe, 256 - (256 * stage.player_state[0].health / 20000), 8};
				RECT health_back = {0, stage.healthp, 256, 8};
				RECT_FIXED health_dst = {FIXED_DEC(-128,1), (SCREEN_HEIGHT2 - 32) << FIXED_SHIFT, 0, FIXED_DEC(8,1)};
				if (stage.downscroll)
					health_dst.y = -health_dst.y - health_dst.h;
				
				health_dst.w = health_fill.w << FIXED_SHIFT;
				Stage_DrawTex(&stage.tex_hud1, &health_fill, &health_dst, stage.bump);
				health_dst.w = health_back.w << FIXED_SHIFT;
				Stage_DrawTex(&stage.tex_hud1, &health_back, &health_dst, stage.bump);
			}

			//uproar fade shit
			if (stage.fadeblack > 0)
			{
				if (stage.fadeblack == 522000)
					break;
				else
				{
         			static const RECT flash = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
					u8 flash_col = stage.fadeblack >> FIXED_SHIFT;
					u8 flash_col2 = stage.fadeextra >> FIXED_SHIFT;
					Gfx_BlendRect(&flash, flash_col, flash_col, flash_col, 2);
					stage.fadeblack += FIXED_MUL(stage.fadespeed, timer_dt*3);  
				}

            
			}
			
			//Hardcoded stage stuff
			switch (stage.stage_id)
			{
				case StageId_1_2: //Fresh GF bop
					switch (stage.song_step)
					{
						case 16 << 2:
							stage.gf_speed = 2 << 2;
							break;
						case 48 << 2:
							stage.gf_speed = 1 << 2;
							break;
						case 80 << 2:
							stage.gf_speed = 2 << 2;
							break;
						case 112 << 2:
							stage.gf_speed = 1 << 2;
							break;
					}
					break;
				default:
					break;
			}
			
			//Draw stage foreground
			if (stage.back->draw_fg != NULL)
				stage.back->draw_fg(stage.back);
			
			//Tick foreground objects
			ObjectList_Tick(&stage.objlist_fg);
			
			//Tick characters
			stage.player->tick(stage.player);
			stage.opponent->tick(stage.opponent);
			
			//Draw stage middle
			if (stage.back->draw_md != NULL)
				stage.back->draw_md(stage.back);
			
			//Tick girlfriend
			if (stage.gf != NULL)
				stage.gf->tick(stage.gf);
			
			//Tick background objects
			ObjectList_Tick(&stage.objlist_bg);
			
			//Draw stage background
			if (stage.back->draw_bg != NULL)
				stage.back->draw_bg(stage.back);
			break;
		}
		case StageState_Dead: //Start BREAK animation and reading extra data from CD
		{
			//Stop music immediately
			Audio_StopXA();
			
			//Unload stage data
			Mem_Free(stage.chart_data);
			stage.chart_data = NULL;
			
			//Free background
			stage.back->free(stage.back);
			stage.back = NULL;
			
			//Free objects
			ObjectList_Free(&stage.objlist_fg);
			ObjectList_Free(&stage.objlist_bg);
			
			//Free opponent and girlfriend
			Stage_SwapChars();
			Character_Free(stage.opponent);
			stage.opponent = NULL;
			Character_Free(stage.gf);
			stage.gf = NULL;
			
			//Reset stage state
			stage.flag = 0;
			stage.bump = stage.sbump = FIXED_UNIT;
			
			//Change background colour to black
			Gfx_SetClear(0, 0, 0);
			
			//Run death animation, focus on player, and change state
			stage.player->set_anim(stage.player, PlayerAnim_Dead0);
			
			Stage_FocusCharacter(stage.player, 0);
			stage.song_time = 0;
			
			stage.state = StageState_DeadLoad;
		}
	//Fallthrough
		case StageState_DeadLoad:
		{
			//Scroll camera and tick player
			if (stage.song_time < FIXED_UNIT)
				stage.song_time += FIXED_UNIT / 60;
			stage.camera.td = FIXED_DEC(-2, 100) + FIXED_MUL(stage.song_time, FIXED_DEC(45, 1000));
			if (stage.camera.td > 0)
				Stage_ScrollCamera();
			stage.player->tick(stage.player);
			
			//Drop mic and change state if CD has finished reading and animation has ended
			if (IO_IsReading() || stage.player->animatable.anim != PlayerAnim_Dead1)
				break;
			
			stage.player->set_anim(stage.player, PlayerAnim_Dead2);
			stage.camera.td = FIXED_DEC(25, 1000);
			stage.state = StageState_DeadDrop;
			break;
		}
		case StageState_DeadDrop:
		{
			//Scroll camera and tick player
			Stage_ScrollCamera();
			stage.player->tick(stage.player);
			
			//Enter next state once mic has been dropped
			if (stage.player->animatable.anim == PlayerAnim_Dead3)
			{
				stage.state = StageState_DeadRetry;
				Audio_PlayXA_Track(XA_GameOver, 0x40, 1, true);
			}
			break;
		}
		case StageState_DeadRetry:
		{
			//Randomly twitch
			if (stage.player->animatable.anim == PlayerAnim_Dead3)
			{
				if (RandomRange(0, 29) == 0)
					stage.player->set_anim(stage.player, PlayerAnim_Dead4);
				if (RandomRange(0, 29) == 0)
					stage.player->set_anim(stage.player, PlayerAnim_Dead5);
			}
			
			//Scroll camera and tick player
			Stage_ScrollCamera();
			stage.player->tick(stage.player);
			break;
		}
		case StageState_Dialogue:
		{
			//oh boy

			RECT dia_src = {0, 0, 227, 63};
	        RECT_FIXED dia_dst = {FIXED_DEC(-150,1), FIXED_DEC(30,1), FIXED_DEC(297,1), FIXED_DEC(76,1)};

			//???
			Stage *this = (Stage*)this;

			static const struct
			{
				const char *text; //The text that is displayed
				u8 camera; //Who the camera is pointing at, 0 for bf, 1 for dad
				u8 charcount; //how many characters are in the line of dialogue
				//s16 p1port; //player 1's portrait
				u8 p2port; //player 2's portrait
			}psydia[] = {
				{"What brings you here so late at night?",1,38,1},
				{"Beep.",0,5},
				{"Drop the act already.",1,21,3},
				{"I could feel your malicious intent the\nmoment you set foot in here.",1,67,1},
				{"Bep bee aa skoo dep?",0,20},
				{"I wouldn't try the door if I were you.",1,38,6},
				{"Now...",1,6,5},
				{"I have a couple of questions\nto ask you...",1,41,1},
				{"And you WILL answer them.",1,25,3},
			};
			

			static const struct
			{
				const char *text;
				u8 camera;
			}wiltdia[] = {
				{"Welp, you got me!",0},
				{"You're very clever, I'll give you\nthat much.",0},
				{"No ordinary person would have seen\nthrough my facade.",0},
				{"Yeah, um...",1},
				{"...Who are you again?",1},
				{"Kh...!",0},
				{"You don't even remember me?!",0},
				{"Not in the slightest.",1},
				{"Seriously?! W-Whatever!",0},
				{"Now listen here!",0},
				{"I've taken this body hostage, so\ndon't even try anything!",0},
				{"Summon Daddy Dearest here this instant,\nor else he gets it!",0},
				{"...Daddy Dearest, huh..?",1},
				{"I don't know what your deal is, but...",1},
				{"I don't take commands from freaks of\nnature like you.",1},
				{"What did you just call me?!",0},
			};

			char uproardia[9] [150] = {
				"At least that guy's gone, he was\ngetting on my nerves.",
				"Let me guess, you're another thorn\nin my side, huh?",
				". . .",
				"...You took the words straight\nfrom my mouth...",
				"I had finally escaped that wretched\ngame, and of course YOU are here to\ngreet me...",
				"...No matter...",
				"...I'll just kill you and finally get\nmy revenge... It's really that simple...",
				"...You don't mind your body being\ndissipated, right? It's only fair...",
				"You took the words straight from\nmy mouth.",
			};

			static const struct
			{
				const char *text;
				u8 camera;
			}latedrivedia[] = {
				{"Huh?",1},
				{"Where are we?",1},
				{"Beep?",0},
				{"That much is obvious.",1},
				{"It seems that we are stuck in some sort of\nalternate reality...",1},
				{"Eep skee dah?",0},
				{"Since we're here anyway, I suppose one song\ncouldn't hurt.",1},
			};

			//Clear per-frame flags
			stage.flag &= ~(STAGE_FLAG_JUST_STEP | STAGE_FLAG_SCORE_REFRESH);

			//play dialogue song
			if (Audio_PlayingXA() != 1)
			{
				Audio_PlayXA_Track(stage.stage_def->diasong, 0x40, stage.stage_def->dia_channel, true); //read stagedef and play song
			}

			//text drawing shit
			switch (stage.stage_id)
			{
				case StageId_1_1:
				{
					//Animatable_Animate(&this->psytalk_animatable, (void*)this, PsyTalk_SetFrame);

					stage.font_arial.draw_col(&stage.font_arial,
						psydia[stage.delect].text,
						25,
						170,
						FontAlign_Left,
						0 >> 1,
						0 >> 1,
						0 >> 1
					);
					if (stage.delect == 9)
					{
						Audio_StopXA();
						Mem_Free(this->arc_psytalk);
						FontData_Load(&stage.font_cdr, Font_CDR);
			            stage.state = StageState_Play;
					}

					//camera shit
					if (psydia[stage.delect].camera == 1)
					    Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
					else
					    Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
					    
					break;
				}

				case StageId_1_2:
				{
					stage.font_arial.draw_col(&stage.font_arial,
						wiltdia[stage.delect].text,
						25,
						170,
						FontAlign_Left,
						0 >> 1,
						0 >> 1,
						0 >> 1
					);

					if (stage.delect == 16)
					{
						Audio_StopXA();
			            stage.state = StageState_Play;
					}

					if (wiltdia[stage.delect].camera == 1)
					    Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
					else
					    Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
					break;
				}

				case StageId_1_3:
				{
					FntPrint("%s", uproardia[stage.delect]);
					if (stage.delect == 9)
					{
						Audio_StopXA();
						stage.state = StageState_Play;
					}
					break;
				}

				case StageId_2_2:
				{
					stage.font_arial.draw_col(&stage.font_arial,
						latedrivedia[stage.delect].text,
						25,
						170,
						FontAlign_Left,
						0 >> 1,
						0 >> 1,
						0 >> 1
					);

					if (stage.delect == 7)
					{
						Audio_StopXA();
			            stage.state = StageState_Play;
					}

					if (latedrivedia[stage.delect].camera == 1)
					    Stage_FocusCharacter(stage.opponent, FIXED_UNIT / 24);
					else
					    Stage_FocusCharacter(stage.player, FIXED_UNIT / 24);
					break;
				}

				default:
				    break;
			}

			Stage_DrawTex(&stage.tex_dia, &dia_src, &dia_dst, stage.bump);

			//portrait shit
			//there has to be a better way of doing this
			//but I'm too stupid to figure it out
			switch (stage.stage_id)
			{
				case StageId_1_1:	
				{		
			        switch (psydia[stage.delect].p2port)
			        {
			        	//normal
			        	case 1:
							Animatable_SetAnim(&this->psytalk_animatable, 0);
			        		PsyTalk_Draw(this, FIXED_DEC(-90,1), FIXED_DEC(-70,1));
			        		break;
						//piss
						case 2:
							Animatable_SetAnim(&this->psytalk_animatable, 1);
			        		PsyTalk_Draw(this, FIXED_DEC(-90,1), FIXED_DEC(-70,1));
			        		break;
						//erect
						case 3:
							Animatable_SetAnim(&this->psytalk_animatable, 2);
			        		PsyTalk_Draw(this, FIXED_DEC(-90,1), FIXED_DEC(-70,1));
			        		break;
						//annoyed
						case 4:
							Animatable_SetAnim(&this->psytalk_animatable, 3);
			        		PsyTalk_Draw(this, FIXED_DEC(-90,1), FIXED_DEC(-70,1));
			        		break;
						//confused
						case 5:
							Animatable_SetAnim(&this->psytalk_animatable, 4);
			        		PsyTalk_Draw(this, FIXED_DEC(-90,1), FIXED_DEC(-70,1));
			        		break;
						//shock
						case 6:
							Animatable_SetAnim(&this->psytalk_animatable, 5);
			        		PsyTalk_Draw(this, FIXED_DEC(-90,1), FIXED_DEC(-70,1));
			        		break;
			        	//nothing
			        	default:
			        	    break;
			        }
				}
				
			}
			static const RECT walterwhite = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

			Gfx_BlendRect(&walterwhite, 255, 255, 255, 255);


			//Draw stage foreground
			if (stage.back->draw_fg != NULL)
				stage.back->draw_fg(stage.back);
			
			//Tick foreground objects
			ObjectList_Tick(&stage.objlist_fg);
			
			//Tick characters
			stage.player->tick(stage.player);
			stage.opponent->tick(stage.opponent);
			
			//Draw stage middle
			if (stage.back->draw_md != NULL)
				stage.back->draw_md(stage.back);
			
			//Tick girlfriend
			if (stage.gf != NULL)
				stage.gf->tick(stage.gf);
			
			//Tick background objects
			ObjectList_Tick(&stage.objlist_bg);
			
			//Draw stage background
			if (stage.back->draw_bg != NULL)
				stage.back->draw_bg(stage.back);

			//skip dialogue
			if (pad_state.press & PAD_START)
			{
			    Audio_StopXA();
			    stage.state = StageState_Play;
			}
			
			//progress to next message
			if (pad_state.press & PAD_CROSS)
			{
				stage.delect++;
			}

			Stage_ScrollCamera();






			
			break;
		}
		default:
			break;
	}
}

#ifdef PSXF_NETWORK
void Stage_NetHit(Packet *packet)
{
	//Reject if not in stage
	if (gameloop != GameLoop_Stage)
		return;
	
	//Get packet info
	u16 i = ((*packet)[1] << 0) | ((*packet)[2] << 8);
	u32 hit_score = ((*packet)[3] << 0) | ((*packet)[4] << 8) | ((*packet)[5] << 16) | ((*packet)[6] << 24);
	u8 hit_type = (*packet)[7];
	u16 hit_combo = ((*packet)[8] << 0) | ((*packet)[9] << 8);
	
	//Get note pointer
	if (i >= stage.num_notes)
		return;
	
	Note *note = &stage.notes[i];
	u8 type = note->type & 0x3;
	
	u8 opp_flag = (stage.mode == StageMode_Net1) ? NOTE_FLAG_OPPONENT : 0;
	if ((note->type & NOTE_FLAG_OPPONENT) != opp_flag)
		return;
	
	//Update game state
	PlayerState *this = &stage.player_state[(stage.mode == StageMode_Net1) ? 1 : 0];
	stage.notes[i].type |= NOTE_FLAG_HIT;
	
	this->score = hit_score;
	this->refresh_score = true;
	this->combo = hit_combo;
	
	if (note->type & NOTE_FLAG_SUSTAIN)
	{
		//Hit a sustain
		Stage_StartVocal();
		this->arrow_hitan[type] = stage.step_time;
		this->character->set_anim(this->character, note_anims[type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);
	}
	else if (!(note->type & NOTE_FLAG_MINE))
	{
		//Hit a note
		Stage_StartVocal();
		this->arrow_hitan[type] = stage.step_time;
		this->character->set_anim(this->character, note_anims[type & 0x3][(note->type & NOTE_FLAG_ALT_ANIM) != 0]);
		
		//Create combo object
		Obj_Combo *combo = Obj_Combo_New(
			this->character->focus_x,
			this->character->focus_y,
			hit_type,
			this->combo >= 10 ? this->combo : 0xFFFF
		);
		if (combo != NULL)
			ObjectList_Add(&stage.objlist_fg, (Object*)combo);
		
		//Create note splashes if SICK
		if (hit_type == 0)
		{
			for (int i = 0; i < 3; i++)
			{
				//Create splash object
				Obj_Splash *splash = Obj_Splash_New(
					note_x[(note->type & 0x7) ^ stage.note_swap],
					note_y * (stage.downscroll ? -1 : 1),
					type
				);
				if (splash != NULL)
					ObjectList_Add(&stage.objlist_splash, (Object*)splash);
			}
		}
	}
	else
	{
		//Hit a mine
		this->arrow_hitan[type & 0x3] = -1;
		if (this->character->spec & CHAR_SPEC_MISSANIM)
			this->character->set_anim(this->character, note_anims[type & 0x3][2]);
		else
			this->character->set_anim(this->character, note_anims[type & 0x3][0]);
	}
}

void Stage_NetMiss(Packet *packet)
{
	//Reject if not in stage
	if (gameloop != GameLoop_Stage)
		return;
	
	//Get packet info
	u8 type = (*packet)[1];
	u32 hit_score = ((*packet)[2] << 0) | ((*packet)[3] << 8) | ((*packet)[4] << 16) | ((*packet)[5] << 24);
	
	//Update game state
	PlayerState *this = &stage.player_state[(stage.mode == StageMode_Net1) ? 1 : 0];
	
	this->score = hit_score;
	this->refresh_score = true;
	
	//Missed
	if (!(type & ~0x3))
	{
		this->arrow_hitan[type] = -1;
		if (this->character->spec & CHAR_SPEC_MISSANIM)
			this->character->set_anim(this->character, note_anims[type][2]);
		else
			this->character->set_anim(this->character, note_anims[type][0]);
	}
	Stage_MissNote(this);
}
#endif
