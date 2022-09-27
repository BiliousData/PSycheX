/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "senport.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Senport character structure
enum
{
	Senport_ArcMain_BF0,
	Senport_ArcMain_BF1,
	
	Senport_Arc_Max,
};

typedef struct
{
	//Character base structure
	Portrait portrait;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Senport_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Senport;

//Senport character definitions
static const CharFrame char_senport_frame[] = {
	{Senport_ArcMain_BF0, {  0,   0, 112, 133}, {0, 0}}, //0
	{Senport_ArcMain_BF0, {112,   0, 113, 147}, {1,14}}, //1
	{Senport_ArcMain_BF1, {  0,   0, 113, 134}, {1, 1}}, //2
	{Senport_ArcMain_BF1, {113,   0, 113, 134}, {0, 1}}, //3
};

static const Animation char_senport_anim[PortAnim_Max] = {
	{2, (const u8[]){ 0, ASCR_REPEAT}}, //normal
	{2, (const u8[]){ 1, ASCR_REPEAT}}, //shocked
	{2, (const u8[]){ 2, ASCR_REPEAT}}, //nervous
	{2, (const u8[]){ 3, ASCR_REPEAT}}, //pissed
};

//Senport character functions
void Char_Senport_SetFrame(void *user, u8 frame)
{
	Char_Senport *this = (Char_Senport*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_senport_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Senport_Tick(Portrait *portrait)
{
	Char_Senport *this = (Char_Senport*)portrait;

	//Animate and draw
	Animatable_Animate(&portrait->animatable, (void*)this, Char_Senport_SetFrame);
	DrawPort(portrait, &this->tex, &char_senport_frame[this->frame]);
}

void Char_Senport_SetAnim(Portrait *portrait, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&portrait->animatable, anim);
	Character_CheckStartSing(portrait);
}

void Char_Senport_Free(Portrait *portrait)
{
	Char_Senport *this = (Char_Senport*)portrait;
	
	//Free art
	Mem_Free(this->arc_main);
}

Portrait *Port_Senpai_New(fixed_t x, fixed_t y)
{
	//Allocate senport object
	Char_Senport *this = Mem_Alloc(sizeof(Char_Senport));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Senport_New] Failed to allocate senport object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->portrait.tick = Char_Senport_Tick;
	this->portrait.set_anim = Char_Senport_SetAnim;
	this->portrait.free = Char_Senport_Free;
	
	Animatable_Init(&this->portrait.animatable, char_senport_anim);
	Character_Init((Portrait*)this, x, y);
	
	//Load art
	this->arc_main = IO_Read("\\DIA\\SENPORT.ARC;1");
	
	const char **pathp = (const char *[]){
		"bf0.tim",
		"bf1.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Portrait*)this;
}
