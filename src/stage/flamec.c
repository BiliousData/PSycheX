/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "flamec.h"

#include "../archive.h"
#include "../mem.h"
#include "../stage.h"
#include "../random.h"
#include "../timer.h"
#include "../animation.h"

boolean phase2;

static const RECT wholescreen = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};

//Week 4 background structure
typedef struct
{
	//Stage background base structure
	StageBack back;

	//Fire lmao
	IO_Data arc_firel, arc_firel_ptr[2];
	IO_Data arc_firem, arc_firem_ptr[2];
	IO_Data arc_firer, arc_firer_ptr[2];
	
	//Textures
	Gfx_Tex tex_back0; //Wall left
	Gfx_Tex tex_back1; //Wall right

	Gfx_Tex tex_floor; //Floor

	Gfx_Tex tex_junk; //tipped chair, table and spilled tea

	Gfx_Tex tex_fireplace; //ruined fireplace

	//AAAHHH
	//FireL state
	Gfx_Tex tex_firel;
	u8 firel_frame, firel_tex_id;
	
	Animatable firel_animatable;

	//FireM state
	Gfx_Tex tex_firem;
	u8 firem_frame, firem_tex_id;
	
	Animatable firem_animatable;

	//FireR state
	Gfx_Tex tex_firer;
	u8 firer_frame, firer_tex_id;
	
	Animatable firer_animatable;

	u8 blendy;

	u8 countup, countdown;
	
} Back_FlameC;

//fire frame data
static const CharFrame firel_frame[4] = {
	{0, {  0,   0, 130,104}, {0,  0}},
	{0, {  0, 104, 130,110}, {0,  7}},
	{1, {  0,   0, 125,116}, {0, 14}},
	{1, {  0, 116, 129,114}, {1, 13}},
};

static const CharFrame firem_frame[4] = {
	{0, {  0,   0, 145, 125}, {0,   0}},
	{0, {  0, 125, 147, 114}, {3, -11}},
	{1, {  0,   0, 147, 118}, {0,  -6}},
	{1, {  0, 118, 141, 126}, {-2,  4}},
};

static const CharFrame firer_frame[4] = {
	{0, {  0,   0, 126, 115}, {0,  0}},
	{0, {  0, 115, 129, 114}, {0, -1}},
	{1, {  0,   0, 130, 109}, {1,-10}},
	{1, {  0, 104, 130, 109}, {2, -4}},
};

//fire animation
static const Animation fire_anim[1] = {
	{2, (const u8[]){0, 1, 2, 3, ASCR_REPEAT}}, //play frames, then repeat
};

//FireL functions
void FlameC_FireL_SetFrame(void *user, u8 frame)
{
	Back_FlameC *this = (Back_FlameC*)user;
	
	//Check if this is a new frame
	if (frame != this->firel_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &firel_frame[this->firel_frame = frame];
		if (cframe->tex != this->firel_tex_id)
			Gfx_LoadTex(&this->tex_firel, this->arc_firel_ptr[this->firel_tex_id = cframe->tex], 0);
	}
}

void FlameC_FireL_Draw(Back_FlameC *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &firel_frame[this->firel_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_firel, &src, &dst, stage.camera.bzoom);
}

//FireM functions
void FlameC_FireM_SetFrame(void *user, u8 frame)
{
	Back_FlameC *this = (Back_FlameC*)user;
	
	//Check if this is a new frame
	if (frame != this->firem_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &firem_frame[this->firem_frame = frame];
		if (cframe->tex != this->firem_tex_id)
			Gfx_LoadTex(&this->tex_firem, this->arc_firem_ptr[this->firem_tex_id = cframe->tex], 0);
	}
}

void FlameC_FireM_Draw(Back_FlameC *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &firem_frame[this->firem_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_firem, &src, &dst, stage.camera.bzoom);
}

//FireR functions
void FlameC_FireR_SetFrame(void *user, u8 frame)
{
	Back_FlameC *this = (Back_FlameC*)user;
	
	//Check if this is a new frame
	if (frame != this->firer_frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &firer_frame[this->firer_frame = frame];
		if (cframe->tex != this->firer_tex_id)
			Gfx_LoadTex(&this->tex_firer, this->arc_firer_ptr[this->firer_tex_id = cframe->tex], 0);
	}
}

void FlameC_FireR_Draw(Back_FlameC *this, fixed_t x, fixed_t y)
{
	//Draw character
	const CharFrame *cframe = &firer_frame[this->firer_frame];
	
	fixed_t ox = x - ((fixed_t)cframe->off[0] << FIXED_SHIFT);
	fixed_t oy = y - ((fixed_t)cframe->off[1] << FIXED_SHIFT);
	
	RECT src = {cframe->src[0], cframe->src[1], cframe->src[2], cframe->src[3]};
	RECT_FIXED dst = {ox, oy, src.w << FIXED_SHIFT, src.h << FIXED_SHIFT};
	Stage_DrawTex(&this->tex_firer, &src, &dst, stage.camera.bzoom);
}




void Back_FlameC_DrawFG(StageBack *back)
{
	Back_FlameC *this = (Back_FlameC*)back;
	
	fixed_t fx, fy;
	//make fire play idle animation
	Animatable_Animate(&this->firel_animatable, (void*)this, FlameC_FireL_SetFrame);
	Animatable_Animate(&this->firem_animatable, (void*)this, FlameC_FireM_SetFrame);
	Animatable_Animate(&this->firer_animatable, (void*)this, FlameC_FireR_SetFrame);

	fx = stage.camera.x;
	fy = stage.camera.y;

	//black fade thing
	if (stage.song_step >= 1152)
	{	
		if (this->countup < 255)
		this->countup += 5;
	}
	if (this->countup > 0)
		Gfx_BlendRect(&wholescreen, 0, 0, 0, 1);

	if (stage.song_step == 896)
		phase2 = 1;


	if (phase2 == 1)
	{
		FlameC_FireL_Draw(this, FIXED_DEC(250,1) - fx, FIXED_DEC(41,1) - fy); //fire fg right
		FlameC_FireM_Draw(this, FIXED_DEC(-210,1) - fx, FIXED_DEC(31,1) - fy); //fire fg left
	}
	
	
}

void Back_FlameC_DrawBG(StageBack *back)
{
	Back_FlameC *this = (Back_FlameC*)back;
	

	
	fixed_t fx, fy;

	
	
	
	//Draw room
	fx = stage.camera.x;
	fy = stage.camera.y;
	
	RECT halll_src = {0, 0, 255, 256};
	RECT_FIXED halll_dst = {
		FIXED_DEC(-165 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-140,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(256,1)
	};

	RECT hallr_src = {0, 0, 256, 256};
	RECT_FIXED hallr_dst = {
		FIXED_DEC(90 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-140,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(256,1)
	};

	RECT floorl_src = {0, 0, 255, 125};
	RECT_FIXED floorl_dst = {
		FIXED_DEC(-165 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(76,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(128,1)
	};

	RECT floorr_src = {0, 125, 255, 125};
	RECT_FIXED floorr_dst = {
		FIXED_DEC(91 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(74,1) - fy,
		FIXED_DEC(256 + SCREEN_WIDEOADD,1),
		FIXED_DEC(128,1)
	};

	RECT chair_src = {0, 0, 122, 123};
	RECT_FIXED chair_dst = {
		FIXED_DEC(-170 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(25,1) - fy,
		FIXED_DEC(122 + SCREEN_WIDEOADD,1),
		FIXED_DEC(123,1)
	};

	RECT table_src = {122, 0, 70, 63};
	RECT_FIXED table_dst = {
		FIXED_DEC(280 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(50,1) - fy,
		FIXED_DEC(70 + SCREEN_WIDEOADD,1),
		FIXED_DEC(63,1)
	};

	RECT tea_src = {122, 63, 48, 33};
	RECT_FIXED tea_dst = {
		FIXED_DEC(290 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(113,1) - fy,
		FIXED_DEC(48 + SCREEN_WIDEOADD,1),
		FIXED_DEC(33,1)
	};

	RECT fireplace_src = {0, 0, 213, 256};
	RECT_FIXED fireplace_dst = {
		FIXED_DEC(8 - SCREEN_WIDEOADD2,1) - fx,
		FIXED_DEC(-145,1) - fy,
		FIXED_DEC(213 + SCREEN_WIDEOADD,1),
		FIXED_DEC(256,1)
	};

	Stage_DrawTex(&this->tex_junk, &table_src, &table_dst, stage.camera.bzoom); //tipped table
	Stage_DrawTex(&this->tex_junk, &tea_src, &tea_dst, stage.camera.bzoom); //spilled tea
	Stage_DrawTex(&this->tex_junk, &chair_src, &chair_dst, stage.camera.bzoom); //tipped chair

	if (phase2 == 1)
	{
		FlameC_FireM_Draw(this, FIXED_DEC(-112,1) - fx, FIXED_DEC(-5,1) - fy); //fire phase 2 left
		FlameC_FireR_Draw(this, FIXED_DEC(232,1) - fx, FIXED_DEC(1,1) - fy); //fire phase 2 right
	}

	FlameC_FireM_Draw(this, FIXED_DEC(62,1) - fx, FIXED_DEC(-11,1) - fy); //fire middle
	FlameC_FireL_Draw(this, FIXED_DEC(-22,1) - fx, FIXED_DEC(11,1) - fy); //fire left
	FlameC_FireR_Draw(this, FIXED_DEC(162,1) - fx, FIXED_DEC(1,1) - fy); //fire right

	
	Stage_DrawTex(&this->tex_fireplace, &fireplace_src, &fireplace_dst, stage.camera.bzoom); //ruined fireplace
	Stage_DrawTex(&this->tex_floor, &floorl_src, &floorl_dst, stage.camera.bzoom); //floor left
	Stage_DrawTex(&this->tex_floor, &floorr_src, &floorr_dst, stage.camera.bzoom); //floor right
	if (phase2 == 1)
	{
		Gfx_BlendRect(&wholescreen, 147, 0, 34, 0); //phase 2 red overlay
	}

	Stage_DrawTex(&this->tex_back0, &halll_src, &halll_dst, stage.camera.bzoom); //wall left
	Stage_DrawTex(&this->tex_back1, &hallr_src, &hallr_dst, stage.camera.bzoom); //wall right
}

void Back_FlameC_Free(StageBack *back)
{
	Back_FlameC *this = (Back_FlameC*)back;

	//unload fire arcs
	Mem_Free(this->arc_firel);
	Mem_Free(this->arc_firem);
	Mem_Free(this->arc_firer);
	
	
	//unload rest of stage
	Mem_Free(this);
}

StageBack *Back_FlameC_New(void)
{
	//Allocate background structure
	Back_FlameC *this = (Back_FlameC*)Mem_Alloc(sizeof(Back_FlameC));
	if (this == NULL)
		return NULL;
	
	//Set background functions
	this->back.draw_fg = Back_FlameC_DrawFG;
	this->back.draw_md = NULL;
	this->back.draw_bg = Back_FlameC_DrawBG;
	this->back.free = Back_FlameC_Free;

	phase2 = 0;

	this->blendy = 0;
	
	//Load background textures
	IO_Data arc_back = IO_Read("\\PSYCHE\\INFERNO.ARC;1");
	Gfx_LoadTex(&this->tex_back0, Archive_Find(arc_back, "back0.tim"), 0);
	Gfx_LoadTex(&this->tex_back1, Archive_Find(arc_back, "back1.tim"), 0);
	Gfx_LoadTex(&this->tex_floor, Archive_Find(arc_back, "floor.tim"), 0);
	Gfx_LoadTex(&this->tex_junk, Archive_Find(arc_back, "junk.tim"), 0);
	Gfx_LoadTex(&this->tex_fireplace, Archive_Find(arc_back, "fplace.tim"), 0);
	Mem_Free(arc_back);

	//load the flame arcs
	this->arc_firel = IO_Read("\\PSYCHE\\FLAMEL.ARC;1");
	this->arc_firel_ptr[0] = Archive_Find(this->arc_firel, "firel0.tim");
	this->arc_firel_ptr[1] = Archive_Find(this->arc_firel, "firel1.tim");

	this->arc_firem = IO_Read("\\PSYCHE\\FLAMEM.ARC;1");
	this->arc_firem_ptr[0] = Archive_Find(this->arc_firem, "firem0.tim");
	this->arc_firem_ptr[1] = Archive_Find(this->arc_firem, "firem1.tim");

	this->arc_firer = IO_Read("\\PSYCHE\\FLAMER.ARC;1");
	this->arc_firer_ptr[0] = Archive_Find(this->arc_firer, "firer0.tim");
	this->arc_firer_ptr[1] = Archive_Find(this->arc_firer, "firer1.tim");

	Gfx_SetClear(0, 0, 0);

	//Initialize fire state
	Animatable_Init(&this->firel_animatable, fire_anim);
	Animatable_SetAnim(&this->firel_animatable, 0);
	this->firel_frame = this->firel_tex_id = 0xFF; //Force art load

	Animatable_Init(&this->firem_animatable, fire_anim);
	Animatable_SetAnim(&this->firem_animatable, 0);
	this->firem_frame = this->firem_tex_id = 0xFF; //Force art load

	Animatable_Init(&this->firer_animatable, fire_anim);
	Animatable_SetAnim(&this->firer_animatable, 0);
	this->firer_frame = this->firer_tex_id = 0xFF; //Force art load
	
	
	
	return (StageBack*)this;
}
