/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "psychm.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//PsychicM character structure
enum
{
	PsychicM_ArcMain_Sheet,
	
	PsychicM_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[PsychicM_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_PsychicM;

//PsychicM character definitions
static const CharFrame char_PsychicM_frame[] = {
	{PsychicM_ArcMain_Sheet, {  0,   0,  62,  95}, { 42, 183}}, //0 idle 1
	{PsychicM_ArcMain_Sheet, { 62,   0,  61,  95}, { 42, 184}}, //1 idle 2
	{PsychicM_ArcMain_Sheet, {123,   0,  60,  95}, { 41, 184}}, //2 idle 3
	{PsychicM_ArcMain_Sheet, {183,   0,  60,  97}, { 41, 185}}, //3 idle 4
};

static const Animation char_PsychicM_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 4,  5, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 6,  7, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 8,  9, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//PsychicM character functions
void Char_PsychicM_SetFrame(void *user, u8 frame)
{
	Char_PsychicM *this = (Char_PsychicM*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_PsychicM_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_PsychicM_Tick(Character *character)
{
	Char_PsychicM *this = (Char_PsychicM*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_QuickIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_PsychicM_SetFrame);
	Character_Draw(character, &this->tex, &char_PsychicM_frame[this->frame]);
}

void Char_PsychicM_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_PsychicM_Free(Character *character)
{
	Char_PsychicM *this = (Char_PsychicM*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_PsychicM_New(fixed_t x, fixed_t y)
{
	//Allocate PsychicM object
	Char_PsychicM *this = Mem_Alloc(sizeof(Char_PsychicM));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_PsychicM_New] Failed to allocate PsychicM object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_PsychicM_Tick;
	this->character.set_anim = Char_PsychicM_SetAnim;
	this->character.free = Char_PsychicM_Free;
	
	Animatable_Init(&this->character.animatable, char_PsychicM_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\MENU\\PSYCHM.ARC;1");
	
	const char **pathp = (const char *[]){
		"psychic.tim", //PsychicM_ArcMain_Idle0
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
