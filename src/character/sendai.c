/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "sendai.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Sendai character structure
enum
{
	Sendai_ArcMain_Panic,
	Sendai_ArcMain_Pain0,
	Sendai_ArcMain_Pain1,
	Sendai_ArcMain_Die0,
	Sendai_ArcMain_Die1,
	Sendai_ArcMain_Die2,
	
	Sendai_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Sendai_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Sendai;

//Sendai character definitions
static const CharFrame char_sendai_frame[] = {
	{Sendai_ArcMain_Panic, {  0,   0, 101, 100}, { 59,  89}}, //0
	{Sendai_ArcMain_Panic, {101,   0, 103,  99}, { 60,  88}}, //1
	{Sendai_ArcMain_Panic, {  0, 100, 102,  99}, { 60,  88}}, //2

	{Sendai_ArcMain_Pain0, {  0,   0, 104,  97}, { 62,  86}}, //3
	{Sendai_ArcMain_Pain0, {104,   0,  92,  88}, { 61,  77}}, //4
	{Sendai_ArcMain_Pain0, {  0,  97,  88,  89}, { 59,  78}}, //5
	{Sendai_ArcMain_Pain0, { 88,  97,  88,  89}, { 59,  78}}, //6

	{Sendai_ArcMain_Pain1, {  0,   0,  89,  90}, { 60,  79}}, //7
	{Sendai_ArcMain_Pain1, { 89,   0,  88,  89}, { 60,  78}}, //8
	{Sendai_ArcMain_Pain1, {  0,  90,  89,  90}, { 61,  79}}, //9
	{Sendai_ArcMain_Pain1, { 89,  90,  89,  90}, { 54,  79}}, //10

	{Sendai_ArcMain_Die0, {  0,   0, 172, 107}, { 116,  96}}, //11 
	{Sendai_ArcMain_Die0, {  0, 107, 172, 108}, { 119,  97}}, //12 

	{Sendai_ArcMain_Die1, {  0,   0, 174, 109}, { 121,  98}}, //13 
	{Sendai_ArcMain_Die1, {  0, 109, 173, 107}, { 120,  97}}, //14 

	{Sendai_ArcMain_Die2, {  0,   0, 174, 108}, { 123,  98}}, //15 


};

static const Animation char_sendai_anim[CharAnim_Max] = {
	{1, (const u8[]){ 0, 1, 2, ASCR_BACK, 3}}, //CharAnim_Idle
	{1, (const u8[]){ 0, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{1, (const u8[]){ 3, 4, 5, 6, 7, 8, 9, ASCR_BACK, 4}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 5, 6, 7, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{1, (const u8[]){ 10, 11, 12, 13, 14, 15, ASCR_BACK, 5}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Sendai character functions
void Char_Sendai_SetFrame(void *user, u8 frame)
{
	Char_Sendai *this = (Char_Sendai*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_sendai_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Sendai_Tick(Character *character)
{
	Char_Sendai *this = (Char_Sendai*)character;
	
	//Perform idle dance
	if (stage.song_step <= 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Sendai_SetFrame);
	Character_Draw(character, &this->tex, &char_sendai_frame[this->frame]);

	if (stage.flag & STAGE_FLAG_JUST_STEP)
    {   //Stage specific animations
		if (stage.note_scroll >= 0)
		{
			switch (stage.stage_id)
			{
				case StageId_1_6: //Animations
				if ((stage.song_step) == 16) //Duck
					    character->set_anim(character, CharAnim_Down);
				if ((stage.song_step) == 48) //Die
				        character->set_anim(character, CharAnim_Right);
				default:
					break;
			}
		}
	}
}

void Char_Sendai_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Sendai_Free(Character *character)
{
	Char_Sendai *this = (Char_Sendai*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Sendai_New(fixed_t x, fixed_t y)
{
	//Allocate sendai object
	Char_Sendai *this = Mem_Alloc(sizeof(Char_Sendai));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Sendai_New] Failed to allocate sendai object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Sendai_Tick;
	this->character.set_anim = Char_Sendai_SetAnim;
	this->character.free = Char_Sendai_Free;
	
	Animatable_Init(&this->character.animatable, char_sendai_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 2;
	
	this->character.focus_zoom = FIXED_DEC(14,10);
	this->character.focus_x = FIXED_DEC(-30,1);
	this->character.focus_y = FIXED_DEC(-65,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\SENDAI.ARC;1");
	
	const char **pathp = (const char *[]){
		"panic.tim",
		"pain0.tim",
		"pain1.tim",
		"die0.tim",
		"die1.tim",
		"die2.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
