/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bfspirit.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//BFS character structure
enum
{
	BFS_ArcMain_Idle0,
	BFS_ArcMain_Idle1,
	BFS_ArcMain_Idle2,
	BFS_ArcMain_Idle3,
	BFS_ArcMain_Idle4,
	BFS_ArcMain_Idle5,
	BFS_ArcMain_Idle6,
	BFS_ArcMain_Left0,
	BFS_ArcMain_Left1,
	BFS_ArcMain_Down0,
	BFS_ArcMain_Down1,
	BFS_ArcMain_Up0,
	BFS_ArcMain_Up1,
	BFS_ArcMain_Right0,
	BFS_ArcMain_Right1,
	BFS_ArcMain_Right2,
	BFS_ArcMain_Right3,
	BFS_ArcMain_Upalt,
	
	BFS_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[BFS_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_BFS;

//Psychic character definitions
static const CharFrame char_BFS_frame[] = { 
	{BFS_ArcMain_Idle0, {  0,   0, 100, 201}, { 42, 189}}, //0 idle 1
	{BFS_ArcMain_Idle1, {  0,   0, 100, 201}, { 41, 189}}, //1 idle 2
	{BFS_ArcMain_Idle2, {  0,   0, 100, 201}, { 42, 189}}, //2 idle 3
	{BFS_ArcMain_Idle3, {  0,   0, 100, 201}, { 42, 189}}, //3 idle 4
	{BFS_ArcMain_Idle4, {  0,   0, 100, 201}, { 41, 189}}, //4
	{BFS_ArcMain_Idle5, {  0,   0, 100, 201}, { 42, 189}}, //5
	{BFS_ArcMain_Idle6, {  0,   0, 100, 201}, { 42, 189}}, //6

	{BFS_ArcMain_Left0, {  0,   0, 117, 197}, { 62, 185}}, //7
	{BFS_ArcMain_Left0, {128,   0, 116, 197}, { 61, 185}}, //8
	{BFS_ArcMain_Left1, {  0,   0, 118, 197}, { 63, 185}}, //9
	{BFS_ArcMain_Left1, {125,   0, 131, 197}, { 76, 185}}, //10

	{BFS_ArcMain_Down0, {  0,   0, 126, 197}, { 40, 185}}, //11
	{BFS_ArcMain_Down0, {130,   0, 120, 197}, { 43, 185}}, //12
	{BFS_ArcMain_Down1, {  0,   0, 114, 197}, { 38, 185}}, //13
	{BFS_ArcMain_Down1, {114,   0, 115, 197}, { 37, 185}}, //14

	{BFS_ArcMain_Up0,    {  0,   0, 96, 250}, { 39, 238}}, //15 86
	{BFS_ArcMain_Up0,    { 104,   0, 96, 250}, { 39, 238}}, //16 86
	{BFS_ArcMain_Up1,    {  0,   0, 90, 250}, { 39, 238}}, //17 80
	{BFS_ArcMain_Up1,    { 101,   0, 99, 250}, { 41, 238}}, //18 87

	{BFS_ArcMain_Right0, {  0,   0, 126, 230}, { 29, 218}},//19
	{BFS_ArcMain_Right1, {  0,   0, 115, 222}, { 29, 210}},//20
	{BFS_ArcMain_Right2, {  0,   0, 114, 220}, { 29, 208}},//21
	{BFS_ArcMain_Right3, {  0,   0, 116, 221}, { 29, 209}},//22
	
	{BFS_ArcMain_Upalt,    {  0,   0, 103, 228}, { 42, 242}}, //23
	{BFS_ArcMain_Upalt,    { 103,   0, 95, 228}, { 42, 242}}, //24
	
};

static const Animation char_BFS_anim[CharAnim_Max] = {
	{1, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, ASCR_BACK, 3}}, //CharAnim_Idle
	{2, (const u8[]){ 7, 8, 9, 10, ASCR_BACK, 0}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 11, 12, 13, 14, ASCR_BACK, 0}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 17, 18, 16, 15, ASCR_BACK, 0}},         //CharAnim_Up
	{2, (const u8[]){ 23, 24, ASCR_BACK, 0}},  //CharAnim_UpAlt
	{2, (const u8[]){22, 21, 20, 19, ASCR_BACK, 0}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
	
};

//Psychic character functions
void Char_BFS_SetFrame(void *user, u8 frame)
{
	Char_BFS *this = (Char_BFS*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_BFS_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFS_Tick(Character *character)
{
	Char_BFS *this = (Char_BFS*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_BFS_SetFrame);
	Character_Draw(character, &this->tex, &char_BFS_frame[this->frame]);
	
}

void Char_BFS_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BFS_Free(Character *character)
{
	Char_BFS *this = (Char_BFS*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_BFS_New(fixed_t x, fixed_t y)
{
	//Allocate psychic object
	Char_BFS *this = Mem_Alloc(sizeof(Char_BFS));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFS_New] Failed to allocate bf spirit object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFS_Tick;
	this->character.set_anim = Char_BFS_SetAnim;
	this->character.free = Char_BFS_Free;
	
	Animatable_Init(&this->character.animatable, char_BFS_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 3;
	
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(-105,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFSPIRIT.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", 
		"idle1.tim", 
		"idle2.tim", 
		"idle3.tim", 
		"idle4.tim", 
		"idle5.tim",
		"idle6.tim", 
		"left0.tim", 
		"left1.tim", 
		"down0.tim", 
		"down1.tim",
		"up0.tim",
		"up1.tim", 
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
