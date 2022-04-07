

#include "stage.h"
#include "mem.h"
#include "psx.h"
#include "loadscr.h"
#include "movie.h"
#include "font.h"
#include "pad.h"
#include "main.h"

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
	"\\STR\\INTRO.STR;1", 320, 240, 429, //Intro cutscene
    "\\STR\\EARBLAST.STR;1", 320, 240, 428 //Old fucked up audio version
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
    //auto start movie
    //if (movie.playing == 0)
    //    movie.playing = 1;

    //start movie when variable is one
    if (movie.playing == 0)
    {
        if (movie.ratio == MovieRatio_Standard)
        {
            //chooses which movie to play depending on select variable
            switch (movie.select)
            {
                case 0:
                PlayMovie();
                PlayStr(320, 240, 0, 0, &StrFile[0]);
                break;
            }
        }
        else
        {
            //chooses which movie to play depending on select variable
            switch (movie.select)
            {
                case 0:
                PlayMovie();
                PlayStr(320, 240, 0, 0, &StrFile[1]);
                break;
            }
        }
    }

    //Switch to stage when movie is finished
    //the stage that it switches to depends on movie selected
    if (strPlayDone == 1)
    {
        movie.playing = 2; //prevent movie from looping
        Gfx_SetClear(0, 0, 0); //make bg black before load screen appears
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

