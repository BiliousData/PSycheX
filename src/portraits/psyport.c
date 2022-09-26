/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "psyport.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Psyport character structure
enum
{
	Psyport_ArcMain_Talk,
	Psyport_ArcMain_Piss,
	Psyport_ArcMain_Erect0,
	Psyport_ArcMain_Erect1,
	Psyport_ArcMain_Annoy,
	Psyport_ArcMain_Confuse,
	Psyport_ArcMain_Shock,
	
	Psyport_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Psyport_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Psyport;

//Psyport character definitions
static const CharFrame char_psyport_frame[] = {
	//normal
	{Psyport_ArcMain_Talk, {  0,   0, 111, 114}, {0, 0}}, //0
	{Psyport_ArcMain_Talk, {111,   0, 109, 114}, {-2, 0}}, //1
	{Psyport_ArcMain_Talk, {  0, 114, 108, 114}, {-3, 0}}, //2
	{Psyport_ArcMain_Talk, {108, 114, 108, 112}, {-3, -2}}, //3
	//piss
	{Psyport_ArcMain_Piss, {   0,   0,  87, 106}, {-17, -8}}, //4
	{Psyport_ArcMain_Piss, {  87,   0,  87, 106}, {-16, -8}}, //5
	{Psyport_ArcMain_Piss, {   0, 106,  90, 105}, {-17, -9}}, //6
	{Psyport_ArcMain_Piss, {  90, 106,  90, 105}, {-17, -9}}, //7
	//erect
	{Psyport_ArcMain_Erect0, {   0,   0, 130, 119}, {-2, 4}}, //8
	{Psyport_ArcMain_Erect0, {   0, 119, 129, 119}, {-3, 4}}, //9
	{Psyport_ArcMain_Erect1, {   0,   0, 128, 120}, {-4, 5}}, //10
	{Psyport_ArcMain_Erect1, {   0, 120, 128, 117}, {-5, 3}}, //11
	//annoyed
	{Psyport_ArcMain_Annoy, {   0,   0, 111, 114}, {0, 0}}, //12
	{Psyport_ArcMain_Annoy, { 111,   0, 109, 114}, {-2, 0}}, //13
	{Psyport_ArcMain_Annoy, {   0, 114, 108, 114}, {-3, 0}}, //14
	{Psyport_ArcMain_Annoy, { 108, 114, 108, 112}, {-3, -2}}, //15
	//confused
	{Psyport_ArcMain_Confuse, {   0,   0,  86, 116}, {4, 1}}, //16
	{Psyport_ArcMain_Confuse, {  86,   0,  85, 117}, {2, 2}}, //17
	{Psyport_ArcMain_Confuse, {   0, 117,  83, 118}, {1, 3}}, //18
	{Psyport_ArcMain_Confuse, {  83, 117,  83, 115}, {0, 0}}, //19
	//shock
	{Psyport_ArcMain_Shock, {   0,   0,  97, 124}, {-12, 0}}, //20
	{Psyport_ArcMain_Shock, {  97,   0,  96, 124}, {-13, 0}}, //21
	{Psyport_ArcMain_Shock, {   0, 124,  96, 124}, {-13, 0}}, //22
	{Psyport_ArcMain_Shock, {  96, 124,  95, 123}, {-13, 0}}, //23
};

static const Animation char_psyport_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, ASCR_REPEAT}}, //CharAnim_Idle
	{2, (const u8[]){ 4, 5, 6, 7, ASCR_REPEAT}},         //CharAnim_Left
	{2, (const u8[]){ 8, 9, 10, 11, ASCR_REPEAT}},   //CharAnim_LeftAlt
	{2, (const u8[]){12, 13, 14, 15, ASCR_REPEAT}},         //CharAnim_Down
	{2, (const u8[]){16, 17, 18, 19, ASCR_REPEAT}},   //CharAnim_DownAlt
	{2, (const u8[]){20, 21, 22, 23, ASCR_REPEAT}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){10, 11, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Psyport character functions
void Char_Psyport_SetFrame(void *user, u8 frame)
{
	Char_Psyport *this = (Char_Psyport*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_psyport_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Psyport_Tick(Character *character)
{
	Char_Psyport *this = (Char_Psyport*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Psyport_SetFrame);
	Character_DrawPort(character, &this->tex, &char_psyport_frame[this->frame]);
}

void Char_Psyport_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Psyport_Free(Character *character)
{
	Char_Psyport *this = (Char_Psyport*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Port_Psy_New(fixed_t x, fixed_t y)
{
	//Allocate psyport object
	Char_Psyport *this = Mem_Alloc(sizeof(Char_Psyport));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Psyport_New] Failed to allocate psyport object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Psyport_Tick;
	this->character.set_anim = Char_Psyport_SetAnim;
	this->character.free = Char_Psyport_Free;
	
	Animatable_Init(&this->character.animatable, char_psyport_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\DIA\\PSYTALK.ARC;1");
	
	const char **pathp = (const char *[]){
		"talk.tim", //Psyport_ArcMain_Idle0
		"piss.tim", //Psyport_ArcMain_Idle1
		"erect0.tim",  //Psyport_ArcMain_Left
		"erect1.tim",  //Psyport_ArcMain_Down
		"annoy.tim",    //Psyport_ArcMain_Up
		"confuse.tim", //Psyport_ArcMain_Right
		"shock.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
