/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bft.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//BFT character structure
enum
{
	BFT_ArcMain_Idle,
	BFT_ArcMain_Oof,
	BFT_ArcMain_Eh,
	BFT_ArcMain_Eey,
	BFT_ArcMain_Ack,
	BFT_ArcMain_Beh,
	BFT_ArcMain_Agga,
	BFT_ArcMain_Deh,
	BFT_ArcMain_Ded,
	BFT_ArcMain_Switch0,
	BFT_ArcMain_Switch1,
	BFT_ArcMain_Switch2,
	BFT_ArcMain_Switch3,
	BFT_ArcMain_Switch4,
	BFT_ArcMain_Switch5,
	BFT_ArcMain_Switch6,
	BFT_ArcMain_Wink0,
	BFT_ArcMain_Wink1,
	BFT_ArcMain_Wink2,
	
	BFT_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[BFT_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_BFT;

//BFT character definitions
static const CharFrame char_bft_frame[] = {
	//this isn't too many frames at all

	{BFT_ArcMain_Idle,     {  0,   0, 103, 104}, { 53,  97}}, //Idle 0

	{BFT_ArcMain_Oof,      {  0,   0,  99,  89}, { 55,  83}}, //Anim 1 1
	{BFT_ArcMain_Oof,      { 99,   0,  99,  90}, { 56,  83}}, //Anim 1 2
	{BFT_ArcMain_Oof,      {  0,  90,  98,  91}, { 56,  84}}, //Anim 1 3
	{BFT_ArcMain_Oof,      { 98,  90,  98,  91}, { 56,  84}}, //Anim 1 4

	{BFT_ArcMain_Eh,       {  0,   0,  90,  87}, { 44,  81}}, //Anim 2 5
	{BFT_ArcMain_Eh,       { 90,   0,  88,  88}, { 44,  81}}, //Anim 2 6
	{BFT_ArcMain_Eh,       {  0,  88,  88,  90}, { 45,  83}}, //Anim 2 7
	{BFT_ArcMain_Eh,       { 88,  88,  91,  90}, { 48,  83}}, //Anim 2 8

	{BFT_ArcMain_Eey,      {  0,   0, 106,  85}, { 65,  79}}, //Anim 3 9
	{BFT_ArcMain_Eey,      {106,   0, 102,  84}, { 61,  78}}, //Anim 3 10
	{BFT_ArcMain_Eey,      {  0,  85,  99,  84}, { 59,  78}}, //Anim 3 11
	{BFT_ArcMain_Eey,      { 99,  85,  98,  87}, { 58,  81}}, //Anim 3 12

	{BFT_ArcMain_Ack,      {  0,   0,  88,  94}, { 42,  87}}, //Anim 4 13
	{BFT_ArcMain_Ack,      { 88,   0,  96,  97}, { 43,  90}}, //Anim 4 14
	{BFT_ArcMain_Ack,      {  0,  97,  99,  97}, { 44,  90}}, //Anim 4 15
	{BFT_ArcMain_Ack,      { 99,  97,  99,  97}, { 44,  90}}, //Anim 4 16
	
	{BFT_ArcMain_Beh,      {  0,   0,  99,  87}, { 44,  81}}, //Anim 5 17
	{BFT_ArcMain_Beh,      { 99,   0,  99,  88}, { 44,  81}}, //Anim 5 18
	{BFT_ArcMain_Beh,      {  0,  88, 100,  90}, { 45,  83}}, //Anim 5 19
	{BFT_ArcMain_Beh,      {100,  88, 102,  90}, { 48,  83}}, //Anim 5 20
	{BFT_ArcMain_Agga,     {  0,   0,  93,  92}, { 48,  85}}, //Anim 5 21

	{BFT_ArcMain_Agga,     { 93,   0,  98, 108}, { 42,  99}}, //Anim 6 22
	{BFT_ArcMain_Agga,     {  0, 108,  99, 109}, { 41, 100}}, //Anim 6 23
	{BFT_ArcMain_Agga,     { 99, 108,  99, 109}, { 41, 100}}, //Anim 6 24
	{BFT_ArcMain_Deh,      {  0,   0,  99, 109}, { 41, 100}}, //Anim 6 25

	{BFT_ArcMain_Deh,      { 99,   0, 100, 109}, { 43,  99}}, //Anim 7 26
	{BFT_ArcMain_Deh,      {  0, 109, 117,  94}, { 64,  87}}, //Anim 7 27
	{BFT_ArcMain_Deh,      {117, 109, 118,  90}, { 64,  83}}, //Anim 7 28
	{BFT_ArcMain_Ded,      {  0,   0, 121,  91}, { 66,  84}}, //Anim 7 29

	{BFT_ArcMain_Switch0,  {  0,   0, 110,  91}, { 66,  84}}, //switch 30
	{BFT_ArcMain_Switch0,  {110,   0, 107,  96}, { 65,  89}}, //switch 31
	{BFT_ArcMain_Switch0,  {  0,  96,  99, 121}, { 55, 114}}, //switch 32
	{BFT_ArcMain_Switch0,  { 99,  96,  94, 155}, { 50, 148}}, //switch 33
	{BFT_ArcMain_Switch1,  {  0,   0,  98, 168}, { 57, 161}}, //switch 34
	{BFT_ArcMain_Switch1,  { 98,   0,  98, 168}, { 57, 161}}, //switch 35
	{BFT_ArcMain_Switch2,  {  0,   0,  88, 171}, { 47, 165}}, //switch 36
	{BFT_ArcMain_Switch2,  { 88,   0,  88, 171}, { 47, 165}}, //switch 37
	{BFT_ArcMain_Switch3,  {  0,   0,  83, 172}, { 41, 165}}, //switch 38
	{BFT_ArcMain_Switch3,  { 83,   0,  83, 172}, { 41, 165}}, //switch 39
	{BFT_ArcMain_Switch3,  {166,   0,  83, 172}, { 42, 165}}, //switch 40
	{BFT_ArcMain_Switch4,  {  0,   0,  83, 172}, { 42, 165}}, //switch 41
	{BFT_ArcMain_Switch4,  { 83,   0,  83, 168}, { 42, 161}}, //switch 42
	{BFT_ArcMain_Switch4,  {166,   0,  83, 155}, { 42, 148}}, //switch 43
	{BFT_ArcMain_Switch5,  {  0,   0,  86, 130}, { 42, 123}}, //switch 44
	{BFT_ArcMain_Switch5,  { 86,   0,  88, 108}, { 42, 101}}, //switch 45
	{BFT_ArcMain_Switch5,  {  0, 130,  91, 104}, { 42,  97}}, //switch 46
	{BFT_ArcMain_Switch5,  { 91, 130,  94, 102}, { 43,  96}}, //switch 47
	{BFT_ArcMain_Switch6,  {  0,   0,  88, 102}, { 42,  96}}, //switch 48

	{BFT_ArcMain_Wink0,    {  0,   0,  88, 104}, { 42,  97}}, //wink 49
	{BFT_ArcMain_Wink0,    { 88,   0,  93, 104}, { 43,  97}}, //wink 50
	{BFT_ArcMain_Wink0,    {  0, 104,  93, 104}, { 43,  97}}, //wink 51
	{BFT_ArcMain_Wink0,    { 93, 104,  90, 104}, { 42,  97}}, //wink 52
	{BFT_ArcMain_Wink1,    {  0,   0,  90, 104}, { 42,  97}}, //wink 53
	{BFT_ArcMain_Wink1,    { 90,   0,  90, 104}, { 42,  97}}, //wink 54
	{BFT_ArcMain_Wink1,    {  0, 104,  90, 104}, { 42,  97}}, //wink 55
	{BFT_ArcMain_Wink1,    { 90, 104,  90, 104}, { 42,  97}}, //wink 56
	{BFT_ArcMain_Wink2,    {  0,   0,  90, 104}, { 42,  97}}, //wink 57
	{BFT_ArcMain_Wink2,    { 90,   0,  90, 104}, { 42,  97}}, //wink 58




};

static const Animation char_bft_anim[CharAnim_Max] = {
	{2, (const u8[]){ 0, ASCR_BACK, 1}},                     //CharAnim_Idle
	{2, (const u8[]){ 9, 10, 11, 12, ASCR_BACK, 1}},         //CharAnim_Left
	{2, (const u8[]){ 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, ASCR_BACK, 1}},           //CharAnim_LeftAlt
	{2, (const u8[]){ 1, 2, 3, 4, ASCR_BACK, 1}},            //CharAnim_Down
	{2, (const u8[]){ 17, 18, 19, 20, 21, ASCR_BACK, 1}},    //CharAnim_DownAlt
	{2, (const u8[]){ 13, 14, 15, 16, ASCR_BACK, 1}},         //CharAnim_Up
	{3, (const u8[]){ 22, 23, 24, 25, 25, 25, 25, 25, 25, 25, 26, 27, 28, 29, ASCR_BACK, 1}},           //CharAnim_UpAlt
	{2, (const u8[]){ 5, 6, 7, 8, ASCR_BACK, 1}},            //CharAnim_Right
	{1, (const u8[]){ 30, 31, 31, 32, 32, 33, 33,  34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 46, 47, 47, 48, ASCR_BACK, 1}},           //CharAnim_RightAlt
};

/** The order of animations are as follows:
 *  
 * Down
 * Right
 * Left
 * Up
 * DownAlt
 * UpAlt
 * 
 * (wait)
 * 
 * RightAlt
 * LeftAlt
 */

//BFT character functions
void Char_BFT_SetFrame(void *user, u8 frame)
{
	Char_BFT *this = (Char_BFT*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bft_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BFT_Tick(Character *character)
{
	Char_BFT *this = (Char_BFT*)character;
	
	//Perform idle dance
	if (stage.song_step <= 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_BFT_SetFrame);
	Character_Draw(character, &this->tex, &char_bft_frame[this->frame]);

	if (stage.flag & STAGE_FLAG_JUST_STEP)
    {   //Stage specific animations
		if (stage.note_scroll >= 0)
		{
			switch (stage.stage_id)
			{
				case StageId_1_5: //Animations
				{
					switch (stage.song_step)
					{
						case 0:
							this->character.focus_x = FIXED_DEC(-16,1);
							this->character.focus_y = FIXED_DEC(-55,1);
							this->character.focus_zoom = FIXED_DEC(15,10);
							stage.camode = 1;
							break;
						case 22:
							character->set_anim(character, CharAnim_Down);
							break;
						case 31:
						    character->set_anim(character, CharAnim_Right);
							break;
						case 37:
						    character->set_anim(character, CharAnim_Left);
							break;
						case 46:
							character->set_anim(character, CharAnim_Up);
							break;
						case 53:
							character->set_anim(character, CharAnim_DownAlt);
							break;
						case 60:
							character->set_anim(character, CharAnim_UpAlt);
							break;
						case 102:
							character->set_anim(character, CharAnim_RightAlt);
							break;
						case 122:
							character->set_anim(character, CharAnim_LeftAlt);
							break;
					}
				}
				default:
					break;
			}
		}
	}
}

void Char_BFT_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BFT_Free(Character *character)
{
	Char_BFT *this = (Char_BFT*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_BFT_New(fixed_t x, fixed_t y)
{
	//Allocate bft object
	Char_BFT *this = Mem_Alloc(sizeof(Char_BFT));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BFT_New] Failed to allocate bft object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BFT_Tick;
	this->character.set_anim = Char_BFT_SetAnim;
	this->character.free = Char_BFT_Free;
	
	Animatable_Init(&this->character.animatable, char_bft_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 2;
	
	this->character.focus_zoom = FIXED_DEC(1,1);
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = (stage.stage_id == StageId_1_4) ? FIXED_DEC(-105,1) : FIXED_DEC(-105,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\BFT.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle.tim",
		"oof.tim",
		"eh.tim",
		"eey.tim",
		"ack.tim",
		"beh.tim",
		"agga.tim",
		"deh.tim",
		"ded.tim",
		"switch0.tim",
		"switch1.tim",
		"switch2.tim",
		"switch3.tim",
		"switch4.tim",
		"switch5.tim",
		"switch6.tim",
		"wink0.tim",
		"wink1.tim",
		"wink2.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
