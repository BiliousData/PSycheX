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

//Boyfriend skull fragments
static SkullFragment char_senpaib_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

//SenpaiB character structure
enum
{
	SenpaiB_ArcMain_Hit0,
	SenpaiB_ArcMain_Hit1,
	SenpaiB_ArcMain_Hit2,
	SenpaiB_ArcMain_Hit3,
	SenpaiB_ArcMain_Miss,
	SenpaiB_ArcMain_Dead0,
	
	SenpaiB_Arc_Max,
};

enum
{
	SenpaiB_ArcDead_Dead1, //Mic Drop
	SenpaiB_ArcDead_Dead2, //Twitch
	SenpaiB_ArcDead_Retry, //Retry prompt
	
	SenpaiB_ArcDead_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[SenpaiB_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;

	SkullFragment skull[COUNT_OF(char_senpaib_skull)];
	u8 skull_scale;
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

	{SenpaiB_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //17 dead0 0
	{SenpaiB_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //18 dead0 1
	{SenpaiB_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //19 dead0 2
	{SenpaiB_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //20 dead0 3
	
	{SenpaiB_ArcDead_Dead1, {  0,   0, 128, 128}, { 53,  98}}, //21 dead1 0
	{SenpaiB_ArcDead_Dead1, {128,   0, 128, 128}, { 53,  98}}, //22 dead1 1
	{SenpaiB_ArcDead_Dead1, {  0, 128, 128, 128}, { 53,  98}}, //23 dead1 2
	{SenpaiB_ArcDead_Dead1, {128, 128, 128, 128}, { 53,  98}}, //24 dead1 3
	
	{SenpaiB_ArcDead_Dead2, {  0,   0, 128, 128}, { 53,  98}}, //25 dead2 body twitch 0
	{SenpaiB_ArcDead_Dead2, {128,   0, 128, 128}, { 53,  98}}, //26 dead2 body twitch 1
	{SenpaiB_ArcDead_Dead2, {  0, 128, 128, 128}, { 53,  98}}, //27 dead2 balls twitch 0
	{SenpaiB_ArcDead_Dead2, {128, 128, 128, 128}, { 53,  98}}, //28 dead2 balls twitch 1
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

	{0, (const u8[]){0,}},
	{0, (const u8[]){0,}},

	{5, (const u8[]){17, 18, 19, 20, 20, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){20, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){21, 22, 23, 24, 24, 24, 24, 24, 24, 24, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){24, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){25, 26, 24, 24, 24, 24, 24, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){27, 28, 24, 24, 24, 24, 24, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){24, 24, 24, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){27, 28, 24, ASCR_REPEAT}},  //PlayerAnim_Dead5
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

	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_SenpaiB, skull); i++, frag++)
			{
				//Draw fragment
				RECT frag_src = {
					(i & 1) ? 112 : 96,
					(i >> 1) << 4,
					16,
					16
				};
				fixed_t skull_dim = (FIXED_DEC(16,1) * this->skull_scale) >> 6;
				fixed_t skull_rad = skull_dim >> 1;
				RECT_FIXED frag_dst = {
					character->x + (((fixed_t)frag->x << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.x,
					character->y + (((fixed_t)frag->y << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.y,
					skull_dim,
					skull_dim,
				};
				
				//Move fragment
				frag->x += frag->xsp;
				frag->y += ++frag->ysp;
			}
			
			//Decrease scale
			this->skull_scale--;
		}
	}
	
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
            	//this->character.focus_zoom = FIXED_DEC(14,10);
				//this->character.focus_x = FIXED_DEC(-30,1);
	            //this->character.focus_y = FIXED_DEC(-75,1);
				//174 32
				this->character.focus_x = FIXED_DEC(-14,1);
				this->character.focus_y = FIXED_DEC(-51,1);
				this->character.focus_zoom = FIXED_DEC(11,10);
				stage.camode = 2;
            };
            break;
	        case 191:
	        {
	        	this->character.focus_zoom = FIXED_DEC(1,1);
				this->character.focus_x = FIXED_DEC(-50,1);
	            this->character.focus_y = FIXED_DEC(-105,1);
				stage.camode = 0;
	        };
	        break;
			case 572:
                {
                	this->character.focus_x = FIXED_DEC(-14,1);
					this->character.focus_y = FIXED_DEC(-51,1);
					this->character.focus_zoom = FIXED_DEC(11,10);
					stage.camode = 2;
                };
            break;
	        case 575:
	        {
	        	this->character.focus_zoom = FIXED_DEC(1,1);
				this->character.focus_x = FIXED_DEC(-50,1);
	            this->character.focus_y = FIXED_DEC(-105,1);
				stage.camode = 0;
	        };
	        break;
			case 1084:
                {
                	this->character.focus_x = FIXED_DEC(-14,1);
					this->character.focus_y = FIXED_DEC(-51,1);
					this->character.focus_zoom = FIXED_DEC(11,10);
					stage.camode = 2;
                };
            break;
	        case 1087:
	        {
	        	this->character.focus_zoom = FIXED_DEC(1,1);
				this->character.focus_x = FIXED_DEC(-50,1);
	            this->character.focus_y = FIXED_DEC(-105,1);
				stage.camode = 0;
	        };
	        break;
			case 1280:
                {
                	this->character.focus_x = FIXED_DEC(-14,1);
					this->character.focus_y = FIXED_DEC(-51,1);
					this->character.focus_zoom = FIXED_DEC(11,10);
					stage.camode = 2;
                };
            break;
	        case 1283:
	        {
	        	this->character.focus_zoom = FIXED_DEC(1,1);
				this->character.focus_x = FIXED_DEC(-50,1);
	            this->character.focus_y = FIXED_DEC(-105,1);
				stage.camode = 0;
	        };
	        break;
	        };
		}
	}
	
}

void Char_SenpaiB_SetAnim(Character *character, u8 anim)
{

	Char_SenpaiB *this = (Char_SenpaiB*)character;

    //Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			this->arc_dead = IO_AsyncReadFile(&this->file_dead_arc);
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Unload main.arc
			Mem_Free(this->arc_main);
			this->arc_main = this->arc_dead;
			this->arc_dead = NULL;
			
			//Find dead.arc files
			const char **pathp = (const char *[]){
				"dead1.tim", //BF_ArcDead_Dead1
				"dead2.tim", //BF_ArcDead_Dead2
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			break;
	}

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
	//health icon
	this->character.health_i = 2;
	//camera
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = FIXED_DEC(-105,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	//healthbar color
	this->character.hr = 255;
	this->character.hg = 170;
	this->character.hb = 111;

	//Load art
	this->arc_main = IO_Read("\\CHAR\\SENPAIB.ARC;1");
	this->arc_dead = NULL;
	IO_FindFile(&this->file_dead_arc, "\\CHAR\\SENDEAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"hit0.tim",
        "hit1.tim",
        "hit2.tim",
        "hit3.tim",
		"miss.tim",
		"dead0.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);

	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;

	//Copy skull fragments
	memcpy(this->skull, char_senpaib_skull, sizeof(char_senpaib_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_SenpaiB, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}