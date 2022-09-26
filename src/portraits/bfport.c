/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bfport.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//BFport character structure
enum
{
	BFport_ArcMain_BF0,
	
	BFport_Arc_Max,
};

typedef struct
{
	//Character base structure
	Portrait portrait;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[BFport_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_BFport;

//BFport character definitions
static const CharFrame char_bfport_frame[] = {
	{BFport_ArcMain_BF0, {  0,   0, 102, 103}, {0, 0}}, //0
	{BFport_ArcMain_BF0, {102,   0, 100, 103}, {-2, 0}}, //1
};

static const Animation char_bfport_anim[PortAnim_Max] = {
	{2, (const u8[]){ 0, ASCR_REPEAT}},
	{2, (const u8[]){ 1, ASCR_REPEAT}},
};

//BFport character functions
void Char_BFport_SetFrame(void *user, u8 frame)
{
	Char_BFport *this = (Char_BFport*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bfport_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFport_Tick(Portrait *portrait)
{
	Char_BFport *this = (Char_BFport*)portrait;

	//Animate and draw
	Animatable_Animate(&portrait->animatable, (void*)this, Char_BFport_SetFrame);
	DrawPort(portrait, &this->tex, &char_bfport_frame[this->frame]);
}

void Char_BFport_SetAnim(Portrait *portrait, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&portrait->animatable, anim);
	Character_CheckStartSing(portrait);
}

void Char_BFport_Free(Portrait *portrait)
{
	Char_BFport *this = (Char_BFport*)portrait;
	
	//Free art
	Mem_Free(this->arc_main);
}

Portrait *Port_BF_New(fixed_t x, fixed_t y)
{
	//Allocate bfport object
	Char_BFport *this = Mem_Alloc(sizeof(Char_BFport));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFport_New] Failed to allocate bfport object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->portrait.tick = Char_BFport_Tick;
	this->portrait.set_anim = Char_BFport_SetAnim;
	this->portrait.free = Char_BFport_Free;
	
	Animatable_Init(&this->portrait.animatable, char_bfport_anim);
	Character_Init((Portrait*)this, x, y);
	
	//Load art
	this->arc_main = IO_Read("\\DIA\\BF.ARC;1");
	
	const char **pathp = (const char *[]){
		"bf0.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Portrait*)this;
}
