/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "titlepsy.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

#include "speaker.h"


//GF Weeb character structure
enum
{
	Titlepsy_ArcMain_Idle0,
	Titlepsy_ArcMain_Idle1,
	Titlepsy_ArcMain_Idle2,
	Titlepsy_ArcMain_Idle3,
	Titlepsy_ArcMain_Idle4,
	Titlepsy_ArcMain_Idle5,
	
	Titlepsy_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Titlepsy_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Titlepsy;

//GF character definitions
static const CharFrame char_Titlepsy_frame[] = {
	{Titlepsy_ArcMain_Idle0, {  0,   0, 174, 240}, { 42, 189}}, //0 idle 1
	{Titlepsy_ArcMain_Idle1, {  0,   0, 174, 240}, { 42, 189}}, //1 idle 2
	{Titlepsy_ArcMain_Idle2, {  0,   0, 174, 240}, { 41, 189}}, //2 idle 3
	{Titlepsy_ArcMain_Idle3, {  0,   0, 174, 240}, { 41, 189}}, //3 idle 4
	{Titlepsy_ArcMain_Idle4, {  0,   0, 174, 240}, { 41, 189}}, //4
	{Titlepsy_ArcMain_Idle5, {  0,   0, 174, 240}, { 41, 189}}, //5

	
};

static const Animation char_Titlepsy_anim[CharAnim_Max] = {
	{1, (const u8[]){ 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, ASCR_BACK, 6}}, //CharAnim_Idle
};

//GF Weeb character functions
void Char_Titlepsy_SetFrame(void *user, u8 frame)
{
	Char_Titlepsy *this = (Char_Titlepsy*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_Titlepsy_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Titlepsy_Tick(Character *character)
{
	Char_Titlepsy *this = (Char_Titlepsy*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Get parallax
	fixed_t parallax;
	parallax = FIXED_DEC(85,100);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Titlepsy_SetFrame);
	Character_DrawParallax(character, &this->tex, &char_Titlepsy_frame[this->frame], parallax);
}

void Char_Titlepsy_SetAnim(Character *character, u8 anim)
{
	//Set animation
	if (anim != CharAnim_Idle && anim != CharAnim_Left && anim != CharAnim_Right)
		return;
	Animatable_SetAnim(&character->animatable, anim);
}

void Char_Titlepsy_Free(Character *character)
{
	Char_Titlepsy *this = (Char_Titlepsy*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Titlepsy_New(fixed_t x, fixed_t y)
{
	//Allocate gf weeb object
	Char_Titlepsy *this = Mem_Alloc(sizeof(Char_Titlepsy));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Titlepsy_New] Failed to allocate gf weeb object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Titlepsy_Tick;
	this->character.set_anim = Char_Titlepsy_SetAnim;
	this->character.free = Char_Titlepsy_Free;
	
	Animatable_Init(&this->character.animatable, char_Titlepsy_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(16,1);
	this->character.focus_y = FIXED_DEC(-50,1);
	this->character.focus_zoom = FIXED_DEC(13,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\TITLEPSY.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim",  //Titlepsy_ArcMain_idle0
		"idle1.tim",  //Titlepsy_ArcMain_idle1
		"idle2.tim",  //Titlepsy_ArcMain_idle2
		"idle3.tim",  //Titlepsy_ArcMain_idle3
		"idle4.tim",  //Titlepsy_ArcMain_idle4
		"idle5.tim",  //Titlepsy_ArcMain_idle5
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
