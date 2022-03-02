/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "psychic.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Psychic character structure
enum
{
	Psychic_ArcMain_Idle0,
	Psychic_ArcMain_Idle1,
	Psychic_ArcMain_Idle2,
	Psychic_ArcMain_Idle3,
	Psychic_ArcMain_Idle4,
	Psychic_ArcMain_Idle5,
	Psychic_ArcMain_Left0,
	Psychic_ArcMain_Left1,
	Psychic_ArcMain_Down0,
	Psychic_ArcMain_Down1,
	Psychic_ArcMain_Up,
	Psychic_ArcMain_Right0,
	Psychic_ArcMain_Right1,
	Psychic_ArcMain_Right2,
	Psychic_ArcMain_Right3,
	
	Psychic_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Psychic_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Psychic;

//Psychic character definitions
static const CharFrame char_psychic_frame[] = {
	{Psychic_ArcMain_Idle0, {  0,   0, 144, 220}, { 42, 183}}, //0 idle 1
	{Psychic_ArcMain_Idle1, {  0,   0, 142, 220}, { 42, 185}}, //1 idle 2
	{Psychic_ArcMain_Idle2, {  0,   0, 140, 223}, { 41, 189}}, //2 idle 3
	{Psychic_ArcMain_Idle3, {  0,   0, 139, 225}, { 41, 191}}, //3 idle 4
	{Psychic_ArcMain_Idle4, {  0,   0, 140, 222}, { 41, 189}}, //4
	{Psychic_ArcMain_Idle5, {  0,   0, 140, 224}, { 41, 191}}, //5

	{Psychic_ArcMain_Left0, {  0,   0, 115, 224}, { 45, 192}}, //6
	{Psychic_ArcMain_Left0, {115,   0, 113, 226}, { 45, 193}}, //7
	{Psychic_ArcMain_Left1, {  0,   0, 113, 226}, { 45, 193}}, //8
	{Psychic_ArcMain_Left1, {113,   0, 113, 225}, { 45, 193}}, //9

	{Psychic_ArcMain_Down0, {  0,   0, 126, 191}, { 44, 157}}, //10
	{Psychic_ArcMain_Down0, {126,   0, 128, 194}, { 44, 159}}, //11
	{Psychic_ArcMain_Down1, {  0,   0, 128, 194}, { 44, 159}}, //12
	{Psychic_ArcMain_Down1, {128,   0, 128, 195}, { 44, 160}}, //13

	{Psychic_ArcMain_Up,    {  0,   0, 107, 237}, { 35, 202}}, //14
	{Psychic_ArcMain_Up,    {107,   0, 110, 235}, { 36, 200}}, //15

	{Psychic_ArcMain_Right0, {  0,   0, 136, 206}, { 43, 171}},//16
	{Psychic_ArcMain_Right1, {  0,   0, 138, 210}, { 42, 175}},//17
	{Psychic_ArcMain_Right2, {  0,   0, 139, 212}, { 42, 177}},//18
	{Psychic_ArcMain_Right3, {  0,   0, 139, 211}, { 42, 177}},//19
	
};

static const Animation char_psychic_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, 1, 2, 3, 4, 5, ASCR_BACK, 3}}, //CharAnim_Idle
	{2, (const u8[]){ 6, 7, 8, 9, ASCR_BACK, 2}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 10, 11, 12, 13, ASCR_BACK, 2}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 14, 15, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){16, 17, 18, 19, ASCR_BACK, 2}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Psychic character functions
void Char_Psychic_SetFrame(void *user, u8 frame)
{
	Char_Psychic *this = (Char_Psychic*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_psychic_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Psychic_Tick(Character *character)
{
	Char_Psychic *this = (Char_Psychic*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Psychic_SetFrame);
	Character_Draw(character, &this->tex, &char_psychic_frame[this->frame]);
}

void Char_Psychic_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Psychic_Free(Character *character)
{
	Char_Psychic *this = (Char_Psychic*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Psychic_New(fixed_t x, fixed_t y)
{
	//Allocate psychic object
	Char_Psychic *this = Mem_Alloc(sizeof(Char_Psychic));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Psychic_New] Failed to allocate psychic object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Psychic_Tick;
	this->character.set_anim = Char_Psychic_SetAnim;
	this->character.free = Char_Psychic_Free;
	
	Animatable_Init(&this->character.animatable, char_psychic_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-95,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\PSYCHIC.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", 
		"idle1.tim", 
		"idle2.tim", 
		"idle3.tim", 
		"idle4.tim", 
		"idle5.tim", 
		"left0.tim", 
		"left1.tim", 
		"down0.tim", 
		"down1.tim", 
		"up.tim", 
		"right0.tim", 
		"right1.tim", 
		"right2.tim", 
		"right3.tim",  
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
