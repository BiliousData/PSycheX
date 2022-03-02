/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "titlegf.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

#include "speaker.h"

//TitleGF character structure
enum
{
	TitleGF_ArcMain_GF,
	TitleGF_ArcMain_GFY,
	TitleGF_ArcMain_GFG,
	TitleGF_ArcMain_GFC,
	TitleGF_ArcMain_GFB,
	TitleGF_ArcMain_GFP,
	
	TitleGF_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[TitleGF_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
	//Speaker
	Speaker speaker;
	
	//Pico test
	u16 *pico_p;
} Char_TitleGF;

//TitleGF character definitions
static const CharFrame char_gf_frame[] = {
	{TitleGF_ArcMain_GF, {  0,   0,  74, 103}, { 37,  72}}, //0 bop left 1
	{TitleGF_ArcMain_GF, { 75,   0,  74, 103}, { 38,  72}}, //1 bop left 2
	{TitleGF_ArcMain_GF, {150,   0,  73, 102}, { 37,  72}}, //2 bop left 3
	{TitleGF_ArcMain_GF, {  0, 104,  73, 103}, { 36,  73}}, //3 bop left 4
	{TitleGF_ArcMain_GF, { 74, 104,  78, 105}, { 38,  75}}, //4 bop left 5
	{TitleGF_ArcMain_GF, {153, 103,  81, 106}, { 41,  76}}, //5 bop left 6
	
	{TitleGF_ArcMain_GFY, {  0,   0,  81, 104}, { 40,  73}}, //6 bop right 1
	{TitleGF_ArcMain_GFY, { 82,   0,  81, 104}, { 40,  73}}, //7 bop right 2
	{TitleGF_ArcMain_GFY, {164,   0,  80, 103}, { 39,  73}}, //8 bop right 3
	{TitleGF_ArcMain_GFY, {  0, 104,  79, 103}, { 38,  74}}, //9 bop right 4
	{TitleGF_ArcMain_GFY, { 80, 105,  74, 104}, { 32,  74}}, //10 bop right 5
	{TitleGF_ArcMain_GFY, {155, 104,  74, 104}, { 32,  74}}, //11 bop right 6

	{TitleGF_ArcMain_GFG, {  0,   0,  74, 103}, { 37,  72}}, //12 bop left 1
	{TitleGF_ArcMain_GFG, { 75,   0,  74, 103}, { 38,  72}}, //13 bop left 2
	{TitleGF_ArcMain_GFG, {150,   0,  73, 102}, { 37,  72}}, //14 bop left 3
	{TitleGF_ArcMain_GFG, {  0, 104,  73, 103}, { 36,  73}}, //15 bop left 4
	{TitleGF_ArcMain_GFG, { 74, 104,  78, 105}, { 38,  75}}, //16 bop left 5
	{TitleGF_ArcMain_GFG, {153, 103,  81, 106}, { 41,  76}}, //17 bop left 6

	{TitleGF_ArcMain_GFC, {  0,   0,  81, 104}, { 40,  73}}, //18 bop right 1
	{TitleGF_ArcMain_GFC, { 82,   0,  81, 104}, { 40,  73}}, //19 bop right 2
	{TitleGF_ArcMain_GFC, {164,   0,  80, 103}, { 39,  73}}, //20 bop right 3
	{TitleGF_ArcMain_GFC, {  0, 104,  79, 103}, { 38,  74}}, //21 bop right 4
	{TitleGF_ArcMain_GFC, { 80, 105,  74, 104}, { 32,  74}}, //22 bop right 5
	{TitleGF_ArcMain_GFC, {155, 104,  74, 104}, { 32,  74}}, //23 bop right 6

	{TitleGF_ArcMain_GFB, {  0,   0,  74, 103}, { 37,  72}}, //24 bop left 1
	{TitleGF_ArcMain_GFB, { 75,   0,  74, 103}, { 38,  72}}, //25 bop left 2
	{TitleGF_ArcMain_GFB, {150,   0,  73, 102}, { 37,  72}}, //26 bop left 3
	{TitleGF_ArcMain_GFB, {  0, 104,  73, 103}, { 36,  73}}, //27 bop left 4
	{TitleGF_ArcMain_GFB, { 74, 104,  78, 105}, { 38,  75}}, //28 bop left 5
	{TitleGF_ArcMain_GFB, {153, 103,  81, 106}, { 41,  76}}, //29 bop left 6

	{TitleGF_ArcMain_GFP, {  0,   0,  81, 104}, { 40,  73}}, //30 bop right 1
	{TitleGF_ArcMain_GFP, { 82,   0,  81, 104}, { 40,  73}}, //31 bop right 2
	{TitleGF_ArcMain_GFP, {164,   0,  80, 103}, { 39,  73}}, //32 bop right 3
	{TitleGF_ArcMain_GFP, {  0, 104,  79, 103}, { 38,  74}}, //33 bop right 4
	{TitleGF_ArcMain_GFP, { 80, 105,  74, 104}, { 32,  74}}, //34 bop right 5
	{TitleGF_ArcMain_GFP, {155, 104,  74, 104}, { 32,  74}}, //35 bop right 6
};

static const Animation char_gf_anim[CharAnim_Max] = {
	{0, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                                 //CharAnim_Idle
	{1, (const u8[]){ 24, 24, 25, 25, 26, 26, 27, 28, 28, 29, ASCR_BACK, 1}},         //CharAnim_Left
	{1, (const u8[]){ 0,  0,  1,  1,  2,  2,  3,  4,  4,  5, ASCR_BACK, 1}},          //CharAnim_LeftAlt
	{1, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                                 //CharAnim_Down
	{1, (const u8[]){ 12, 12, 13, 13, 14, 14, 15, 16, 16, 17, ASCR_BACK, 1}},         //CharAnim_DownAlt
	{1, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt}},                                 //CharAnim_Up
	{1, (const u8[]){ 18, 18, 19, 19, 20, 20, 21, 22, 22, 23, ASCR_BACK, 1}},          //CharAnim_UpAlt
	{1, (const u8[]){ 30, 30, 31, 31, 32, 32, 33, 34, 34, 35, ASCR_BACK, 1}},         //CharAnim_Right
	{1, (const u8[]){ 6,  6,  7,  7,  8,  8,  9, 10, 10, 11, ASCR_BACK, 1}},          //CharAnim_RightAlt
};

//TitleGF character functions
void Char_TitleGF_SetFrame(void *user, u8 frame)
{
	Char_TitleGF *this = (Char_TitleGF*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_gf_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_TitleGF_Tick(Character *character)
{
	Char_TitleGF *this = (Char_TitleGF*)character;
	
	if (this->pico_p != NULL)
	{
		if (stage.note_scroll >= 0)
		{
			//Scroll through Pico chart
			u16 substep = stage.note_scroll >> FIXED_SHIFT;
			while (substep >= ((*this->pico_p) & 0x7FFF))
			{
				//Play animation and bump speakers
				character->set_anim(character, ((*this->pico_p) & 0x8000) ? CharAnim_RightAlt : CharAnim_LeftAlt);
				Speaker_Bump(&this->speaker);
				this->pico_p++;
			}
		}
	}
	else
	{
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
			//Stage specific animations
			if (stage.note_scroll >= 0)
			{
				switch (stage.stage_id)
				{
					case StageId_1_4: //Tutorial cheer
						if (stage.song_step > 64 && stage.song_step < 192 && (stage.song_step & 0x3F) == 60)
							character->set_anim(character, CharAnim_UpAlt);
						break;
					default:
						break;
				}
			}
			
			//Perform dance
			if (stage.note_scroll >= character->sing_end && (stage.song_step % stage.gf_speed) == 0)
			{
				//Switch animation
				if (character->animatable.anim == CharAnim_LeftAlt)
					character->set_anim(character, CharAnim_RightAlt);
				else if (character->animatable.anim == CharAnim_RightAlt)
					character->set_anim(character, CharAnim_DownAlt);
				else if (character->animatable.anim == CharAnim_DownAlt)
					character->set_anim(character, CharAnim_UpAlt);
				else if (character->animatable.anim == CharAnim_UpAlt)
					character->set_anim(character, CharAnim_Left);
				else if (character->animatable.anim == CharAnim_Left)
					character->set_anim(character, CharAnim_Right);
				else if (character->animatable.anim == CharAnim_Right)
					character->set_anim(character, CharAnim_LeftAlt);
				
				//Bump speakers
				Speaker_Bump(&this->speaker);
			}
		}
	}
	
	//Get parallax
	fixed_t parallax;
	if (stage.stage_id >= StageId_1_1 && stage.stage_id <= StageId_1_4)
		parallax = FIXED_DEC(7,10);
	else
		parallax = FIXED_UNIT;
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_TitleGF_SetFrame);
	Character_DrawParallax(character, &this->tex, &char_gf_frame[this->frame], parallax);
	
	//Tick speakers
	Speaker_Tick(&this->speaker, character->x, character->y, parallax);
}

void Char_TitleGF_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_TitleGF_Free(Character *character)
{
	Char_TitleGF *this = (Char_TitleGF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_TitleGF_New(fixed_t x, fixed_t y)
{
	//Allocate gf object
	Char_TitleGF *this = Mem_Alloc(sizeof(Char_TitleGF));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_TitleGF_New] Failed to allocate gf object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_TitleGF_Tick;
	this->character.set_anim = Char_TitleGF_SetAnim;
	this->character.free = Char_TitleGF_Free;
	
	Animatable_Init(&this->character.animatable, char_gf_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(2,1);
	this->character.focus_y = FIXED_DEC(-40,1);
	this->character.focus_zoom = FIXED_DEC(2,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\TITLEGF.ARC;1");
	
	const char **pathp = (const char *[]){
		"gf.tim",
        "gfy.tim",
        "gfg.tim",
        "gfc.tim",
        "gfb.tim",
        "gfp.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize speaker
	Speaker_Init(&this->speaker);
	
	this->pico_p = NULL;
	
	return (Character*)this;
}
