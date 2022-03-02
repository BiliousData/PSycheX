/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "senpaib.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//SenpaiB character structure
enum
{
	SenpaiB_ArcMain_Hit0,
	SenpaiB_ArcMain_Hit1,
	SenpaiB_ArcMain_Hit2,
	SenpaiB_ArcMain_Hit3,
	SenpaiB_ArcMain_Miss,
	
	SenpaiB_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[SenpaiB_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_SenpaiB;

//SenpaiB character definitions
static const CharFrame char_senpaib_frame[] = {
	{SenpaiB_ArcMain_Hit0, {  0,   0,  92, 103}, { 53,  92}}, //0 idle 1
	{SenpaiB_ArcMain_Hit0, { 92,   0,  92, 105}, { 52,  94}}, //1 idle 2
	{SenpaiB_ArcMain_Hit0, {  0, 105,  92, 105}, { 53,  94}}, //2 idle 3
	{SenpaiB_ArcMain_Hit0, { 92, 105,  89, 106}, { 53,  95}}, //3 idle 4
	{SenpaiB_ArcMain_Hit1, {  0,   0,  89, 106}, { 53,  95}}, //4 idle 5
	 
	{SenpaiB_ArcMain_Hit1, { 89,   0,  87, 102}, { 58,  91}}, //5 left 1
	{SenpaiB_ArcMain_Hit1, {  0, 106,  87, 102}, { 58,  91}}, //6 left 2
	 
	{SenpaiB_ArcMain_Hit1,  { 87, 106,  89,  92}, { 56,  81}}, //7 down 1
	{SenpaiB_ArcMain_Hit2,  {  0,   0,  86,  93}, { 55,  82}}, //8 down 2
	 
	{SenpaiB_ArcMain_Hit2,   { 86,   0,  85, 116}, { 49, 105}}, //9 up 1
	{SenpaiB_ArcMain_Hit2,   {  0, 116,  87, 114}, { 50, 103}}, //10 up 2
	 
	{SenpaiB_ArcMain_Hit2, { 87, 116,  92, 116}, { 52,  95}}, //11 right 1
	{SenpaiB_ArcMain_Hit3, {  0,   0,  91, 106}, { 52,  95}}, //12 right 2

	{SenpaiB_ArcMain_Miss, {  0,   0,  87, 112}, { 58, 101}}, //13
	{SenpaiB_ArcMain_Miss, { 87,   0,  87, 104}, { 56,  94}}, //14
	{SenpaiB_ArcMain_Miss, {  0, 112,  86, 124}, { 49, 111}}, //15
	{SenpaiB_ArcMain_Miss, { 86, 112,  91, 114}, { 52, 102}}, //16
};

static const Animation char_senpaib_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0, 1,  2,  3, 4,  ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 5, 6, ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 9,  10, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){11, 12, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt

	{1, (const u8[]){ 5, 13, 13, 13, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){ 7, 14, 14, 14, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){ 9, 15, 15, 15, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){11, 16, 16, 16, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
};

//SenpaiB character functions
void Char_SenpaiB_SetFrame(void *user, u8 frame)
{
	Char_SenpaiB *this = (Char_SenpaiB*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_senpaib_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_SenpaiB_Tick(Character *character)
{
	Char_SenpaiB *this = (Char_SenpaiB*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_SenpaiB_SetFrame);
	Character_Draw(character, &this->tex, &char_senpaib_frame[this->frame]);

	//big chungus operating the camera
    if (stage.stage_id == StageId_1_2)
	{
	    if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
		    switch (stage.song_step)
			{
            case 188:
                {
                	this->character.focus_zoom = FIXED_DEC(14,10);
					this->character.focus_x = FIXED_DEC(-30,1);
	                this->character.focus_y = FIXED_DEC(-75,1);
                };
            break;
	        case 191:
	        {
	        	this->character.focus_zoom = FIXED_DEC(1,1);
				this->character.focus_x = FIXED_DEC(-50,1);
	            this->character.focus_y = FIXED_DEC(-105,1);
	        };
	        break;
			case 572:
                {
                	this->character.focus_zoom = FIXED_DEC(14,10);
					this->character.focus_x = FIXED_DEC(-30,1);
	                this->character.focus_y = FIXED_DEC(-75,1);
                };
            break;
	        case 575:
	        {
	        	this->character.focus_zoom = FIXED_DEC(1,1);
				this->character.focus_x = FIXED_DEC(-50,1);
	            this->character.focus_y = FIXED_DEC(-105,1);
	        };
	        break;
			case 1084:
                {
                	this->character.focus_zoom = FIXED_DEC(14,10);
					this->character.focus_x = FIXED_DEC(-30,1);
	                this->character.focus_y = FIXED_DEC(-75,1);
                };
            break;
	        case 1087:
	        {
	        	this->character.focus_zoom = FIXED_DEC(1,1);
				this->character.focus_x = FIXED_DEC(-50,1);
	            this->character.focus_y = FIXED_DEC(-105,1);
	        };
	        break;
			case 1280:
                {
                	this->character.focus_zoom = FIXED_DEC(14,10);
					this->character.focus_x = FIXED_DEC(-30,1);
	                this->character.focus_y = FIXED_DEC(-75,1);
                };
            break;
	        case 1283:
	        {
	        	this->character.focus_zoom = FIXED_DEC(1,1);
				this->character.focus_x = FIXED_DEC(-50,1);
	            this->character.focus_y = FIXED_DEC(-105,1);
	        };
	        break;
	        };
		}
	}
	
}

void Char_SenpaiB_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_SenpaiB_Free(Character *character)
{
	Char_SenpaiB *this = (Char_SenpaiB*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_SenpaiB_New(fixed_t x, fixed_t y)
{
	//Allocate senpaib object
	Char_SenpaiB *this = Mem_Alloc(sizeof(Char_SenpaiB));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_SenpaiB_New] Failed to allocate senpaib object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_SenpaiB_Tick;
	this->character.set_anim = Char_SenpaiB_SetAnim;
	this->character.free = Char_SenpaiB_Free;
	
	Animatable_Init(&this->character.animatable, char_senpaib_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;
	
	this->character.health_i = 2;
	
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(-105,1);
	this->character.focus_zoom = FIXED_DEC(1,1);

	//Load art
	this->arc_main = IO_Read("\\CHAR\\SENPAIB.ARC;1");
	
	const char **pathp = (const char *[]){
		"hit0.tim",
        "hit1.tim",
        "hit2.tim",
        "hit3.tim",
		"miss.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);

	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
