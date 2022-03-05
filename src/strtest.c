/*	strplay.c example by Lameguy64 of Meido-Tek Productions
	
	Compile with: ccpsx -O -Xo$80010000 strtest.c -o strtest.cpe
	
*/

//#define DEBUG		1	// Uncomment so that the library will printf any error messages

#include <sys/types.h>
#include <libetc.h>
#include <libgte.h>
#include <libgpu.h>
#include <libcd.h>


// Include the STR player library
#include "strplay.c"


// STR file entry (it is a lot easier to deal with STR files this way)
STRFILE StrFile[] = {
	// File name	Resolution		Frame count
	"\\STR\\UGH.STR;1", 320, 240, 6572
};


strcool() {
	
	// Reset and initialize stuff
	ResetCallback();
	CdInit();
	PadInit(0);
	ResetGraph(0);
	SetGraphDebug(0);
	
	// Play the video in loop
	while (1) {
		
		if (PlayStr(320, 240, 0, 0, &StrFile[0]) == 0)	// If player presses Start
			break;	// Exit the loop
		
	}
	
}