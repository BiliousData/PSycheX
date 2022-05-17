//The Strplay library (which this relies on) was made by Lameguy64 of Meido-Tek Productions.

//side note: this file appears to go nuts when compiled on linux, with it either crashing, or difficulties malfunctioning.

#include "stage.h"
#include "mem.h"
#include "psx.h"
#include "loadscr.h"
#include "movie.h"
#include "font.h"
#include "pad.h"
#include "main.h"
#include "menu.h"

#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libcd.h>

#include "strplay.c"

//#define DEBUG 1

Movie movie;

//str file list
STRFILE StrFile[] = {
	// File name	Resolution		Frame count
	{"\\STR\\INTRO.STR;1", 320, 240, 429}, //Intro cutscene
    {"\\STR\\WIDE.STR;1", 384, 240, 429}, //true widescreen
    {"\\STR\\WINTRO.STR;1", 320, 180, 428}, //16:9 version
    {"\\STR\\EARBLAST.STR;1", 320, 240, 428}, //Old fucked up audio version
    {"\\STR\\END.STR;1", 384, 240, 2313}
};

void PlayMovie() {
    // Reset and initialize stuff
	ResetCallback();
	CdInit();
	ResetGraph(0);
	SetGraphDebug(0);
}

void Movie_Tick(void) 
{

    //start movie
    if (movie.playing == 0)
    {
        
        //chooses which movie to play depending on select variable
        switch (movie.select)
        {
            case 0:
            PlayMovie();
            PlayStr(320, 240, 0, 0, &StrFile[0]);
            break;

            case 1:
            PlayMovie();
            PlayStr(320, 240, 0, 0, &StrFile[4]);
            break;
        }
        
    }

    //Switch to stage when movie is finished
    //the stage that it switches to depends on movie selected
    //OR
    //send it to the main menu if the week is complete
    if (strPlayDone == 1)
    {
        movie.playing = 2; //prevent movie from looping
        Gfx_SetClear(0, 0, 0); //make bg black before load screen appears

        if (movie.weekdone == 1)
        {
            LoadScr_Start(); //begin loading menu
            gameloop = GameLoop_Menu;
            Menu_Load(MenuPage_Story);
            LoadScr_End();
        }
        else
        {
            LoadScr_Start(); //begin loading normal game
            gameloop = GameLoop_Stage;
            switch (movie.select)
            {
                case 0:
                    Stage_Load(StageId_1_1, stage.stage_diff, true);
                break;
            }
            LoadScr_End(); //End load screen and start normal game
        }
    }

}

