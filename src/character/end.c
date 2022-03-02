/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "end.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Clucky character structure
enum
{
	End_ArcMain_Cut0,
	End_ArcMain_Cut1,
	End_ArcMain_Cut2,
	End_ArcMain_Cut3,
	End_ArcMain_Cut4,
	
	End_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[End_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_End;

//Clucky character definitions
static const CharFrame char_End_frame[] = {
	{End_ArcMain_Cut0, {  0,   0, 256, 256}, { 180, 20}}, //0 idle 1
	
	{End_ArcMain_Cut1, {  0,   0, 256, 256}, { 180, 20}}, //1 left 1
	
	{End_ArcMain_Cut2, {  0,   0, 256, 256}, { 180, 20}}, //2 down 1
	
	{End_ArcMain_Cut3, {  0,   0, 256, 256}, { 180, 20}}, //3 up 1
	
	{End_ArcMain_Cut4, {  0,   0, 256, 256}, { 180, 20}}, //4 right 1
};

static const Animation char_End_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0,  0,  0,  0, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 1,  1, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 2,  2, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 3,  3, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){ 4,  4, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Clucky character functions
void Char_End_SetFrame(void *user, u8 frame)
{
	Char_End *this = (Char_End*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_End_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_End_Tick(Character *character)
{
	Char_End *this = (Char_End*)character;
	
	//Perform idle dance
	if (stage.song_step <= 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_End_SetFrame);
	Character_Draw(character, &this->tex, &char_End_frame[this->frame]);

	if (stage.flag & STAGE_FLAG_JUST_STEP)
    {   //Stage specific animations
		if (stage.note_scroll >= 0)
		{
			switch (stage.stage_id)
			{
				case StageId_2_3: //Animations
				if ((stage.song_step) == 356) //Duck
					    character->set_anim(character, CharAnim_Left);
				if ((stage.song_step) == 1270) //Die
				        character->set_anim(character, CharAnim_Down);
				if ((stage.song_step) == 1360) //Die
				        character->set_anim(character, CharAnim_Up);
				if ((stage.song_step) == 1680) //Die
				        character->set_anim(character, CharAnim_Right);
				default:
					break;
			}
		}
	}
}

void Char_End_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_End_Free(Character *character)
{
	Char_End *this = (Char_End*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_End_New(fixed_t x, fixed_t y)
{
	//Allocate clucky object
	Char_End *this = Mem_Alloc(sizeof(Char_End));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_End_New] Failed to allocate end object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_End_Tick;
	this->character.set_anim = Char_End_SetAnim;
	this->character.free = Char_End_Free;
	
	Animatable_Init(&this->character.animatable, char_End_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(100,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\CUT.ARC;1");
	
	const char **pathp = (const char *[]){
		"cut0.tim", //Clucky_ArcMain_Idle0
		"cut1.tim", //Clucky_ArcMain_Idle1
		"cut2.tim",  //Clucky_ArcMain_Left
		"cut3.tim",  //Clucky_ArcMain_Down
		"cut4.tim",    //Clucky_ArcMain_Up
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
