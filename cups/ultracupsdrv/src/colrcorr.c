#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <stdlib.h>
#include <fcntl.h>
#include "magigen.h"
#include "gndefs.h"
#include <stdint.h>
#include "magirx.h"

#define CHUNK 16384

unsigned int  GridDelta = 4 ;// Idealy a power of 2,the driver assumes 4 though 8 works just fine                   



enum intents
{
       PERCEPTUAL = 0,
       SATURATED,
       RELATIVE_COLORIMETRIC,
       ABSOLUTE_COLORIMETRIC,
	   INTERNAL,	   
       NO_OF_INTENTS
};

enum LookupTable
{
       RIO_LUT = 0,
       ENDURO_LUT,
       HELIX_LUT
};

/* TYPEDEFS        */
/*******************/
typedef struct _charPixel {
	
	unsigned char           r;	
	unsigned char           g; 	
	unsigned char           b;	
	unsigned char           c;	
	unsigned char           m;	
	unsigned char           y;
} charPixel;

/* DEFINES         */
/*******************/
#define GridSize (unsigned char)(256 / GridDelta) + 1
#define MaxGrid  65 // used when defining the LUT's
// LUT Processing
#define CYAN	0
#define MAGENTA	1
#define YELLOW	2

/* GLOBALS */
/***********/
//                    				R    G   B  I      P,S,R & A (order of intents)
static unsigned char Cyan   [MaxGrid][MaxGrid][MaxGrid][NO_OF_INTENTS];
static unsigned char Magenta[MaxGrid][MaxGrid][MaxGrid][NO_OF_INTENTS];
static unsigned char Yellow [MaxGrid][MaxGrid][MaxGrid][NO_OF_INTENTS];

unsigned int CMY[5];
unsigned int CMYX[3];

BOOL bLUTLoaded = false;


BOOL SetUpLUT(PDEVDATA pdev, PSETTINGS settings)
{
    //PRIVATEDEVMODE ldmPrivate;
	int		i,j,k,ret;
	//FILE*   hFile= 0;
	//HANDLE  hGlobalMem = 0;
	PBYTE	pGlobalMem = 0;
	//CHAR 	szPrinterDriverDirectory[MAX_PATH];
	//CHAR 	szFilePath[MAX_PATH];
	int		nLoadTable = RIO_LUT;		//0 = Rio 1=Enduro+ 2=Helix
	BOOL 	bLoadYaInternal = FALSE;

	if (bLUTLoaded)
		return TRUE;
		
    ////ldmPrivate = *pdmPrivate;

	//determine this once for speed
	if (ENDURO_PLUS_TYPE(settings))
		nLoadTable = ENDURO_LUT;
		
	//GetSystemDirectoryA((PSTR)&szPrinterDriverDirectory,MAX_PATH);

    if (HELIX_OEM(settings))
	{
		pGlobalMem = &MAGIOX;
	}
	else
	{
////		if (GetFilmManufacturer(pdev) == 4)
		{
			//use Ya!!
			if (nLoadTable == ENDURO_LUT) 
				pGlobalMem = &MAGIEY;
			else
				pGlobalMem = &MAGIRY;
			bLoadYaInternal = TRUE;
		}
		else
		{
			if (nLoadTable == ENDURO_LUT) 
				pGlobalMem = &MAGIEX;
			else
				pGlobalMem = &MAGIRX;
		}
	}


/////////////


	//pglobal mem needs to be set to the colourtable for the correct OEM type at this point
	
	if( pGlobalMem )
	{

///////////////////////
		memcpy((PBYTE)&CMYX, pGlobalMem, sizeof( CMYX ));

		pGlobalMem += sizeof(CMYX);
		
		GridDelta = CMYX[0];// retrieve the grid  scaling; !!!!! the first 3 words are a header record the first word of which contains the GridDelta
		
		for( i = 0; i <GridSize; i++)
		{
			for( j = 0; j <GridSize; j++)
			{
				for( k = 0; k <GridSize; k++)
				{
				
					memcpy((PBYTE)&CMYX, pGlobalMem, sizeof( CMYX ));

					Cyan	[i][j][k][PERCEPTUAL]				= (unsigned char) ( CMYX[CYAN]		  & 0xFF);				
					Cyan	[i][j][k][SATURATED]				= (unsigned char) ( CMYX[CYAN]	>> 8  & 0xFF);			
					Cyan	[i][j][k][RELATIVE_COLORIMETRIC]	= (unsigned char) ( CMYX[CYAN]	>> 16 & 0xFF);			
					Cyan	[i][j][k][ABSOLUTE_COLORIMETRIC]	= (unsigned char) ( CMYX[CYAN]	>> 24 & 0xFF);

					Magenta  [i][j][k][PERCEPTUAL]				= (unsigned char) ( CMYX[MAGENTA]		  & 0xFF);				
					Magenta  [i][j][k][SATURATED]				= (unsigned char) ( CMYX[MAGENTA]	>> 8  & 0xFF);			
					Magenta  [i][j][k][RELATIVE_COLORIMETRIC]	= (unsigned char) ( CMYX[MAGENTA]	>> 16 & 0xFF);			
					Magenta  [i][j][k][ABSOLUTE_COLORIMETRIC]	= (unsigned char) ( CMYX[MAGENTA]	>> 24 & 0xFF);
		
					Yellow   [i][j][k][PERCEPTUAL]				= (unsigned char) ( CMYX[YELLOW]		  & 0xFF);				
					Yellow   [i][j][k][SATURATED]				= (unsigned char) ( CMYX[YELLOW]	>> 8  & 0xFF);			
					Yellow   [i][j][k][RELATIVE_COLORIMETRIC]	= (unsigned char) ( CMYX[YELLOW]	>> 16 & 0xFF);			
					Yellow   [i][j][k][ABSOLUTE_COLORIMETRIC]	= (unsigned char) ( CMYX[YELLOW]	>> 24 & 0xFF);
					
					pGlobalMem += sizeof(CMYX);
				}
			}
		}

		if (bLoadYaInternal)
		{
			 //PBYTE pTst,pTst1,pTst2,pTst3;
			// WCHAR szDbg[256];
			
			//now load up the internals..
			for( i = 0; i <GridSize; i++)
			{
				for( j = 0; j <GridSize; j++)
				{
					for( k = 0; k <GridSize; k+=4)
					{
					
						memcpy((PBYTE)&CMYX, pGlobalMem, sizeof( CMYX ));
						pGlobalMem += sizeof(CMYX);
						
						Cyan	[i][j][k][INTERNAL]		= (unsigned char) ( CMYX[CYAN]		  & 0xFF);		
						Magenta  [i][j][k][INTERNAL]	= (unsigned char) ( CMYX[MAGENTA]		  & 0xFF);	
						Yellow   [i][j][k][INTERNAL]	= (unsigned char) ( CMYX[YELLOW]		  & 0xFF);	
												
						if (k==64)
							break;
							
						Cyan	[i][j][k+1][INTERNAL]	= (unsigned char) ( CMYX[CYAN]	>> 8  & 0xFF);			
						Cyan	[i][j][k+2][INTERNAL]	= (unsigned char) ( CMYX[CYAN]	>> 16 & 0xFF);			
						Cyan	[i][j][k+3][INTERNAL]	= (unsigned char) ( CMYX[CYAN]	>> 24 & 0xFF);
		
						Magenta  [i][j][k+1][INTERNAL]	= (unsigned char) ( CMYX[MAGENTA]	>> 8  & 0xFF);			
						Magenta  [i][j][k+2][INTERNAL]	= (unsigned char) ( CMYX[MAGENTA]	>> 16 & 0xFF);			
						Magenta  [i][j][k+3][INTERNAL]	= (unsigned char) ( CMYX[MAGENTA]	>> 24 & 0xFF);
						 //pTst = &Cyan	[i][j][k][INTERNAL];
						 //pTst1 = &Cyan	[i][j][k+1][INTERNAL];
						 //pTst2 = &Cyan	[i][j][k+2][INTERNAL];
						 //pTst3 = &Cyan	[i][j][k+3][INTERNAL];
						 //wsprintf((PWSTR)&szDbg, L"i:%x:j:%x:k:%x::pTst:%x pTst1:%x, pTst2:%x, pTst3:%x\n",i,j,k,pTst,pTst1,pTst2,pTst3);
						 //OutputDebugString((PWSTR)&szDbg);			
			
						Yellow   [i][j][k+1][INTERNAL]	= (unsigned char) ( CMYX[YELLOW]	>> 8  & 0xFF);			
						Yellow   [i][j][k+2][INTERNAL]	= (unsigned char) ( CMYX[YELLOW]	>> 16 & 0xFF);			
						Yellow   [i][j][k+3][INTERNAL]	= (unsigned char) ( CMYX[YELLOW]	>> 24 & 0xFF);
						

					}
				}
			}
		}
   	
		
		bLUTLoaded = TRUE;
	}
	else
	{
		//DBGMSG(DBG_LEVEL_VERBOSE,L"COLRCORR.C: can't open the intents file");
		return FALSE;

	}

	
	return TRUE;
}


void ColourCorrect(charPixel * pixel, BYTE intent)
{
	//call this function as a LUT algorithm in the same way Naive is an algorithm.

	//  Algorithm based on :    (Red - Red_i)/(Red_i+1 -Red_i) = (Cyan- Cyan_i)/(Cyan_i+1 - Cyan_i) where i goes from 0 to 255 in steps of 4

	unsigned char upper[3];
	unsigned char lower[3];
	unsigned char offset[3];

	unsigned char r = 0; //just a convenience
	unsigned char g = 1;
	unsigned char b = 2;

	lower[r] = (unsigned char)(pixel->r)/GridDelta;  upper[r] = lower[r]+1; offset[r] = (unsigned char)(pixel->r - lower[r]*GridDelta);
	lower[g] = (unsigned char)(pixel->g)/GridDelta;  upper[g] = lower[g]+1; offset[g] = (unsigned char)(pixel->g - lower[g]*GridDelta);
	lower[b] = (unsigned char)(pixel->b)/GridDelta;  upper[b] = lower[b]+1; offset[b] = (unsigned char)(pixel->b - lower[b]*GridDelta);

	pixel->c = Cyan[lower[r]][lower[g]][lower[b]][intent];
	pixel->c += (unsigned char)((offset[r])? (1.0 * offset[r] * ( Cyan[upper[r]][upper[g]][upper[b]][intent] - Cyan[lower[r]][lower[g]][lower[b]][intent]))/GridDelta : 0);
	
	
	pixel->m = Magenta[lower[r]][lower[g]][lower[b]][intent];
	pixel->m += (unsigned char)((offset[g])?  (1.0 * offset[g] * ( Magenta[upper[r]][upper[g]][upper[b]][intent] - Magenta[lower[r]][lower[g]][lower[b]][intent]))/GridDelta : 0);
	
	
	pixel->y = Yellow[lower[r]][lower[g]][lower[b]][intent];
	pixel->y += (unsigned char)((offset[b])?  (1.0 * offset[b] * ( Yellow[upper[r]][upper[g]][upper[b]][intent] - Yellow[lower[r]][lower[g]][lower[b]][intent]))/GridDelta : 0);
	
}

void ColourCorrectHelix(charPixel * pixel, BYTE intent)
{
	unsigned char upper[3];
	unsigned char lower[3];
	unsigned char offset[3];

	double  adjustment = 0;
	int adj = 0;

	unsigned char r = 0; //just a convenience
	unsigned char g = 1;
	unsigned char b = 2;

	lower[r] = (unsigned char)(pixel->r)/GridDelta;  upper[r] = lower[r]+1; offset[r] = (unsigned char)(pixel->r - lower[r]*GridDelta);
	lower[g] = (unsigned char)(pixel->g)/GridDelta;  upper[g] = lower[g]+1; offset[g] = (unsigned char)(pixel->g - lower[g]*GridDelta);
	lower[b] = (unsigned char)(pixel->b)/GridDelta;  upper[b] = lower[b]+1; offset[b] = (unsigned char)(pixel->b - lower[b]*GridDelta);

	
	pixel->c = Cyan[lower[r]][lower[g]][lower[b]][intent];
	adjustment =  1.0 * offset[r] * ( Cyan[upper[r]][upper[g]][upper[b]][intent] - Cyan[lower[r]][lower[g]][lower[b]][intent])/GridDelta ;
	adj = adjustment < 0.0 ? (int)ceil(adjustment - 0.5) : (int)floor(adjustment + (double)0.5);
	pixel->c += (unsigned char)adj;
	
	pixel->m = Magenta[lower[r]][lower[g]][lower[b]][intent];
	adjustment =  1.0 * offset[g] * ( Magenta[upper[r]][upper[g]][upper[b]][intent] - Magenta[lower[r]][lower[g]][lower[b]][intent])/GridDelta ;
	adj = adjustment < 0.0 ? (int)ceil(adjustment - 0.5) : (int)floor(adjustment + 0.5);
	pixel->m += (unsigned char)adj;
	
	pixel->y = Yellow[lower[r]][lower[g]][lower[b]][intent];
	adjustment =  1.0 * offset[b] * ( Yellow[upper[r]][upper[g]][upper[b]][intent] - Yellow[lower[r]][lower[g]][lower[b]][intent])/GridDelta ;
	adj = adjustment < 0.0 ? (int)ceil(adjustment - 0.5) : (int)floor(adjustment + 0.5);
	pixel->y += (unsigned char)adj;

}


