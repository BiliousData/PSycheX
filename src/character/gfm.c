/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "gfm.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//GFM character structure
enum
{
	GFM_ArcMain_gfm0,
	GFM_ArcMain_gfm1,
	
	GFM_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[GFM_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_GFM;

//GFM character definitions
static const CharFrame char_GFM_frame[] = {
	{GFM_ArcMain_gfm0, {  0,   0,  89,  80}, { 42, 183}}, //0 idle 1
	{GFM_ArcMain_gfm0, { 89,   0,  88,  80}, { 41, 183}}, //1 idle 2
	{GFM_ArcMain_gfm0, {  0,  80,  89,  80}, { 42, 183}}, //2 idle 3
	{GFM_ArcMain_gfm0, { 89,  80,  88,  80}, { 41, 183}}, //3 idle 4
	{GFM_ArcMain_gfm0, {  0, 160,  89,  80}, { 42, 183}}, //4 idle 5
	{GFM_ArcMain_gfm0, { 89, 161,  88,  81}, { 41, 184}}, //5 idle 6
	
	{GFM_ArcMain_gfm1, {  0,   0,  89,  80}, { 42, 183}}, //6 idle 7
	{GFM_ArcMain_gfm1, { 89,   0,  88,  80}, { 41, 183}}, //7 idle 8
	{GFM_ArcMain_gfm1, {  0,  80,  89,  80}, { 42, 183}}, //8 idle 9
	{GFM_ArcMain_gfm1, { 89,  80,  88,  80}, { 41, 183}}, //9 idle 10
	{GFM_ArcMain_gfm1, {  0, 160,  89,  80}, { 42, 183}}, //10 idle 11
};

static const Animation char_GFM_anim[CharAnim_Max] = {
	{1, (const u8[]){ ASCR_CHGANI, CharAnim_LeftAlt, }}, //CharAnim_Idle
	{2, (const u8[]){ ASCR_CHGANI, CharAnim_LeftAlt, }}, //CharAnim_Left
	{1, (const u8[]){ 0, 1, 2, 3, 4, ASCR_BACK, 1}},   //CharAnim_LeftAlt
	{2, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt, }},    //CharAnim_Down
	{1, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt, }},   //CharAnim_DownAlt
	{2, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt, }},       //CharAnim_Up
	{1, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt, }},   //CharAnim_UpAlt
	{2, (const u8[]){ASCR_CHGANI, CharAnim_LeftAlt, }},       //CharAnim_Right
	{1, (const u8[]){ 5, 6, 7, 8, 9, 10, ASCR_BACK, 1}},   //CharAnim_RightAlt
};

//GFM character functions
void Char_GFM_SetFrame(void *user, u8 frame)
{
	Char_GFM *this = (Char_GFM*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_GFM_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_GFM_Tick(Character *character)
{
	Char_GFM *this = (Char_GFM*)character;
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{	
		//Perform dance
			if (stage.note_scroll >= character->sing_end && (stage.song_step % stage.gf_speed) == 0)
			{
				//Switch animation
				if (character->animatable.anim == CharAnim_LeftAlt || character->animatable.anim == CharAnim_Right)
					character->set_anim(character, CharAnim_RightAlt);
				else
					character->set_anim(character, CharAnim_LeftAlt);
					
			}
		
	}
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_GFM_SetFrame);
	Character_Draw(character, &this->tex, &char_GFM_frame[this->frame]);
}

void Char_GFM_SetAnim(Character *character, u8 anim)
{
	//Set animation
	if (anim == CharAnim_Left || anim == CharAnim_Down || anim == CharAnim_Up || anim == CharAnim_Right || anim == CharAnim_UpAlt)
		character->sing_end = stage.note_scroll + FIXED_DEC(22,1); //Nearly 2 steps
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_GFM_Free(Character *character)
{
	Char_GFM *this = (Char_GFM*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_GFM_New(fixed_t x, fixed_t y)
{
	//Allocate GFM object
	Char_GFM *this = Mem_Alloc(sizeof(Char_GFM));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_GFM_New] Failed to allocate GFM object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_GFM_Tick;
	this->character.set_anim = Char_GFM_SetAnim;
	this->character.free = Char_GFM_Free;
	
	Animatable_Init(&this->character.animatable, char_GFM_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\MENU\\GFM.ARC;1");
	
	const char **pathp = (const char *[]){
		"gfm0.tim", //GFM_ArcMain_Idle0
		"gfm1.tim", //GFM_ArcMain_Idle1
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
