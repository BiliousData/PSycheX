/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef PSXF_GUARD_AUDIO_H
#define PSXF_GUARD_AUDIO_H

#include "psx.h"

//XA enumerations
typedef enum
{
	XA_Menu,   //MENU.XA
	XA_Week1A, //WEEK1A.XA
	XA_Week1B, //WEEK1B.XA
	XA_Week2A, //WEEK2A.XA
	XA_Week2B, //WEEK2B.XA
	XA_CutA,
	XA_CutB,
	XA_Extra,
	XA_Custom,
	XA_Dialog,
	
	XA_Max,
} XA_File;

typedef enum
{
	//MENU.XA
	XA_GettinFreaky, //Gettin' Freaky
	XA_GameOver,     //Game Over
	XA_TeaTime,      //Tea Time
	//WEEK1A.XA
	XA_Bopeebo, //Bopeebo
	XA_Fresh,   //Fresh
	//WEEK1B.XA
	XA_Dadbattle, //DadBattle
	XA_Tutorial,  //Tutorial
	//WEEK2A.XA
	XA_Spookeez, //Spookeez
	XA_South,    //South
	//WEEK2B.XA
	XA_Monster, //Monster
	XA_Clucked, //Clucked
	//CUT.XA
	XA_BfTransform,
	XA_Sendai,
	//EXTRA
	XA_Freeky,
	XA_Bftdemo,
	//CUSTOM
	XA_Ludum, //Ludum Dare theme (for customization menu)
	//DIALOG
	XA_Psy, //Psychic Dialogue theme
	XA_LoveIsAFlower, //Wilter Dialogue theme
	XA_SpiritualUnrest, //Uproar Dialogue theme
	XA_ItWillBeOkay, //Late Drive Dialogue theme (Name comes from the FL Studio track in Salvati's song "let me out", where the song originates.)
	
	XA_TrackMax,
} XA_Track;

//Audio functions
void Audio_Init(void);
void Audio_Quit(void);
void Audio_PlayXA_Track(XA_Track track, u8 volume, u8 channel, boolean loop);
void Audio_SeekXA_Track(XA_Track track);
void Audio_PauseXA(void);
void Audio_StopXA(void);
void Audio_ChannelXA(u8 channel);
s32 Audio_TellXA_Sector(void);
s32 Audio_TellXA_Milli(void);
boolean Audio_PlayingXA(void);
void Audio_WaitPlayXA(void);
void Audio_ProcessXA(void);

#endif
