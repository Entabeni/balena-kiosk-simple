/***************************************************************************/
/**                                                                       **/
/**                            MAGICARD LTD                               **/
/**                                                                       **/
/***************************************************************************/
/**                                                                       **/
/**  PROJECT      : Magicard Driver                                       **/
/**                                                                       **/
/**  MODULE NAME  : MagEncd.C                                             **/
/**                                                                       **/
/**  SIN No.      : 2076                                                  **/
/**                                                                       **/
/**  COPYRIGHT    : Magicard Ltd				                          **/
/**                                                                       **/
/***************************************************************************/

#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <stdlib.h>
#include <fcntl.h>
#include "magigen.h"
#include <ctype.h>
//#include "magu.h"
#include "magencd.h"
//#include "debug.h"
//#include "oem.h"

static BOOL bFoundSpecifier = FALSE;
static BOOL bFoundSpecifierEnd = FALSE;
static BOOL bFirstCommaFound = FALSE;
static int  iMagControl = 0;
static int  iConcTrack  = MAG_TRACK_NONE;

/******************************************************************************
 *  InitializeMagneticEncodeTrackInfo()
 *      Initialize MAGTRACKINFO structure.
 *
 *  Returns:
 *      None
 *****************************************************************************/

void InitializeMagneticEncodeTrackInfo(PDEVDATA pdev, struct settings_ * settings)
{
    //LPDVPDEV    lpDrvData  = pmPDev->pDrvData;
	BOOL        AutoInsert;
    //LPDOCSTICKY lpds = GetUICBDocumentProperties(pmPDev->lpdm);

    memset(&pdev->magTrackInfo, 0, sizeof(MAGTRACKINFO) * MAX_TRACK_NUMBER);
	//Mag Tracks 0 & 4 are special tracks for passing of commands by an application to a
	//printer via the header, 0 = front side, 4 = back side
	pdev->magTrackInfo[MAG_TRACK_INDEX0].ulMaxTrackLen  = MAX_NUMCHAR_TRACK0;
    pdev->magTrackInfo[MAG_TRACK_INDEX0].wEndSentinel   = ISO_ENDSENTINEL;
	pdev->magTrackInfo[MAG_TRACK_INDEX0].bSSAutoInsert  = FALSE;
    pdev->magTrackInfo[MAG_TRACK_INDEX0].bESAutoInsert  = FALSE;
    pdev->magTrackInfo[MAG_TRACK_INDEX0].wMinCharCode   = MIN_CHAR_TRACK0;
    pdev->magTrackInfo[MAG_TRACK_INDEX0].wMaxCharCode   = MAX_CHAR_TRACK0;
	pdev->magTrackInfo[MAG_TRACK_INDEX0].pValidCharList = ValidCharCodesJIS_Track1;
	
	pdev->magTrackInfo[MAG_TRACK_INDEX4].ulMaxTrackLen  = MAX_NUMCHAR_TRACK4;
    pdev->magTrackInfo[MAG_TRACK_INDEX4].wEndSentinel   = ISO_ENDSENTINEL;
	pdev->magTrackInfo[MAG_TRACK_INDEX4].bSSAutoInsert  = FALSE;
    pdev->magTrackInfo[MAG_TRACK_INDEX4].bESAutoInsert  = FALSE;
    pdev->magTrackInfo[MAG_TRACK_INDEX4].wMinCharCode   = MIN_CHAR_TRACK0;
    pdev->magTrackInfo[MAG_TRACK_INDEX4].wMaxCharCode   = MAX_CHAR_TRACK0;
	pdev->magTrackInfo[MAG_TRACK_INDEX4].pValidCharList = ValidCharCodesJIS_Track1;

    // Initialize members which will be different between each track.
	pdev->magTrackInfo[MAG_TRACK_INDEX1].wStartSentinel = ISO_STARTSENTINEL_TRACK1;
    pdev->magTrackInfo[MAG_TRACK_INDEX1].wEndSentinel   = ISO_ENDSENTINEL;
	pdev->magTrackInfo[MAG_TRACK_INDEX1].ulMaxTrackLen  = MAX_NUMCHAR_TRACK1;
    pdev->magTrackInfo[MAG_TRACK_INDEX1].wMinCharCode   = MIN_CHAR_TRACK1;
    if (settings->bJIS2Enabled)
	{
		pdev->magTrackInfo[MAG_TRACK_INDEX1].wMaxCharCode   = MAX_CHAR_ISO_TRACK1;
		pdev->magTrackInfo[MAG_TRACK_INDEX1].pValidCharList = ValidCharCodesISO_Track1;
	}
	else
	{
		pdev->magTrackInfo[MAG_TRACK_INDEX1].wMaxCharCode   = MAX_CHAR_JIS_TRACK1;
		pdev->magTrackInfo[MAG_TRACK_INDEX1].pValidCharList = ValidCharCodesJIS_Track1;
	}
	pdev->magTrackInfo[MAG_TRACK_INDEX1].wBitsPerChar   = settings->nBitsPerChar_Track1;
	pdev->magTrackInfo[MAG_TRACK_INDEX1].wBitsPerInch   = (settings->nBitsPerInch_Track1) ? 75 : 210;
	
	pdev->magTrackInfo[MAG_TRACK_INDEX1].wCoercivity    = settings->nCoercivity;  //altered if set in passed string 0=COEH 1=COEL
	pdev->magTrackInfo[MAG_TRACK_INDEX2].wCoercivity    = settings->nCoercivity;
	pdev->magTrackInfo[MAG_TRACK_INDEX3].wCoercivity    = settings->nCoercivity;
	
    pdev->magTrackInfo[MAG_TRACK_INDEX2].wStartSentinel = ISO_STARTSENTINEL_TRACK2;
    pdev->magTrackInfo[MAG_TRACK_INDEX2].wEndSentinel   = ISO_ENDSENTINEL;
    pdev->magTrackInfo[MAG_TRACK_INDEX2].wMinCharCode   = MIN_CHAR_TRACK1;
    pdev->magTrackInfo[MAG_TRACK_INDEX2].wMaxCharCode   = MAX_CHAR_ISO_TRACK1;
	pdev->magTrackInfo[MAG_TRACK_INDEX2].ulMaxTrackLen  = MAX_NUMCHAR_TRACK1;
    pdev->magTrackInfo[MAG_TRACK_INDEX2].pValidCharList = ValidCharCodesISO_Track1;
	pdev->magTrackInfo[MAG_TRACK_INDEX2].wBitsPerChar   = settings->nBitsPerChar_Track2;
	pdev->magTrackInfo[MAG_TRACK_INDEX2].wBitsPerInch   = (settings->nBitsPerInch_Track2) ? 75 : 210;
	
    pdev->magTrackInfo[MAG_TRACK_INDEX3].wStartSentinel = ISO_STARTSENTINEL_TRACK3;
    pdev->magTrackInfo[MAG_TRACK_INDEX3].wEndSentinel   = ISO_ENDSENTINEL;
    pdev->magTrackInfo[MAG_TRACK_INDEX3].wMinCharCode   = MIN_CHAR_TRACK1;
    pdev->magTrackInfo[MAG_TRACK_INDEX3].wMaxCharCode   = MAX_CHAR_ISO_TRACK1;
	pdev->magTrackInfo[MAG_TRACK_INDEX3].ulMaxTrackLen  = MAX_NUMCHAR_TRACK1;
    pdev->magTrackInfo[MAG_TRACK_INDEX3].pValidCharList = ValidCharCodesISO_Track1;
	pdev->magTrackInfo[MAG_TRACK_INDEX3].wBitsPerChar   = settings->nBitsPerChar_Track3;
	pdev->magTrackInfo[MAG_TRACK_INDEX3].wBitsPerInch   = (settings->nBitsPerInch_Track3) ? 75 : 210;
	
    if (HELIX_OEM(settings) || PRO360_OEM(settings))
	{
		AutoInsert = (settings->nTrackOptions) ? TRUE : FALSE;//eEncodingMethod == UICBVAL_Encoding_AutoInsert);

		pdev->magTrackInfo[MAG_TRACK_INDEX1].bSSAutoInsert = (settings->nBitsPerChar_Track1 > 1) ? 0 : AutoInsert;
		pdev->magTrackInfo[MAG_TRACK_INDEX1].bESAutoInsert = (settings->nBitsPerChar_Track1 > 1) ? 0 : AutoInsert;
		pdev->magTrackInfo[MAG_TRACK_INDEX2].bSSAutoInsert = (settings->nBitsPerChar_Track2 > 1) ? 0 : AutoInsert;
		pdev->magTrackInfo[MAG_TRACK_INDEX2].bESAutoInsert = (settings->nBitsPerChar_Track2 > 1) ? 0 : AutoInsert;
		pdev->magTrackInfo[MAG_TRACK_INDEX3].bSSAutoInsert = (settings->nBitsPerChar_Track3 > 1) ? 0 : AutoInsert;
		pdev->magTrackInfo[MAG_TRACK_INDEX3].bESAutoInsert = (settings->nBitsPerChar_Track3 > 1) ? 0 : AutoInsert;
		
		pdev->magTrackInfo[MAG_TRACK_INDEX1].bHexData = (settings->nBitsPerChar_Track1 == 3) ? TRUE : FALSE;
		pdev->magTrackInfo[MAG_TRACK_INDEX2].bHexData = (settings->nBitsPerChar_Track2 == 3) ? TRUE : FALSE;
		pdev->magTrackInfo[MAG_TRACK_INDEX3].bHexData = (settings->nBitsPerChar_Track3 == 3) ? TRUE : FALSE;
	}
	else
	{
		if (RIO_OEM(settings)
		||  ENDURO_OEM(settings))
		{
			AutoInsert = (settings->nTrackOptions) ? TRUE : FALSE;//eEncodingMethod == UICBVAL_Encoding_AutoInsert);
			
		}
		else
		{
			AutoInsert = TRUE;
		}

		pdev->magTrackInfo[MAG_TRACK_INDEX1].bSSAutoInsert = AutoInsert;
		pdev->magTrackInfo[MAG_TRACK_INDEX1].bESAutoInsert = AutoInsert;
		pdev->magTrackInfo[MAG_TRACK_INDEX2].bSSAutoInsert = AutoInsert;
		pdev->magTrackInfo[MAG_TRACK_INDEX2].bESAutoInsert = AutoInsert;
		pdev->magTrackInfo[MAG_TRACK_INDEX3].bSSAutoInsert = AutoInsert;
		pdev->magTrackInfo[MAG_TRACK_INDEX3].bESAutoInsert = AutoInsert;
	}

	bFoundSpecifier    = FALSE;
	bFoundSpecifierEnd = FALSE;
	bFirstCommaFound   = FALSE;
	iMagControl = 0;
	iConcTrack  = MAG_TRACK_NONE;
}

/******************************************************************************
 *  ValidMagDataChar()
 *      Checks if the given character is in the valid mag data character list
 *
 *  Returns:
 *      BOOL = TRUE if character is in the list
 *****************************************************************************/

BOOL ValidMagDataChar
(
    LPMAGTRACKINFO lpTrackInfo,
    CHAR          CharIn
)
{
	BOOL result = FALSE;

    if (CharIn >= lpTrackInfo->wMinCharCode && CharIn <= lpTrackInfo->wMaxCharCode)
	{
	    if (lpTrackInfo->pValidCharList[((int)CharIn & 0xff) - lpTrackInfo->wMinCharCode])
	    {
			result = TRUE;
	    }
	}
	//check for hex chars
	if (result == FALSE)
	{
		CHAR cmpstr[2];
		cmpstr[0]=(BYTE)CharIn;
		cmpstr[1]=0;
		if (strtok ( (char *)&cmpstr, "0123456789ABCDEFabcdef") == NULL)
			result = TRUE;
	}
	return result;
}

/******************************************************************************
 *  JIS2CommandInString()
 *      Determines if a JIS2 Command is held in the mag encoding string
 *
 *  Returns:
 *      TRUE or FALSE
 *****************************************************************************/

BOOL JIS2CommandInString
(
    LPMAGTRACKINFO lpTrackInfo
)
{
	// Test that lpTrackInfo is not empty
    if (lpTrackInfo == NULL)
    {
        return FALSE;
    }
	
	//Check if there are any embedded commands in the string
	if (strrchr(lpTrackInfo->TrackData, CHAR_COMMA) == NULL)
	{
		return FALSE;
	}
	
	//Is there a JIS2 command is the string
	if (strstr(lpTrackInfo->TrackData, JIS2_COMMAND) != NULL)
	{
		return TRUE;
	}

    return FALSE;
}

/******************************************************************************
 *  StartSentinel()
 *      Returns the Start Sentinel character for the selected track and encoding
 *      method
 *
 *  Returns:
 *      End Sentinel character
 *****************************************************************************/

int StartSentinel
(
	PDEVDATA pdev,
    LPMAGTRACKINFO lpTrackInfo,
struct settings_ * settings
)
{

//	if (lpds->bJIS2Enabled 
//	|| (lpds->eEncodingType == UICBVAL_Encoding_JIS)
//	|| JIS2CommandInString(lpTrackInfo))

	if (settings->bJIS2Enabled
	|| JIS2CommandInString(lpTrackInfo))
	{
		return JIS_STARTSENTINEL;
	}
	else
	{
		return lpTrackInfo->wStartSentinel;
	}
}

/******************************************************************************
 *  EndSentinel()
 *      Returns the End Sentinel character for the selected encoding method
 *
 *  Returns:
 *      End Sentinel character
 *****************************************************************************/

int EndSentinel
(
	PDEVDATA pdev,
    LPMAGTRACKINFO lpTrackInfo,
struct settings_ * settings

)
{
	if (settings->bJIS2Enabled
	|| JIS2CommandInString(lpTrackInfo))
	{
		return JIS_ENDSENTINEL;
	}
	else
	{
		return ISO_ENDSENTINEL;
	}
}

VOID PerformPostProcessing(PDEVDATA pdev, LPMAGTRACKINFO lpTrackInfo, struct settings_ * settings
)
{
int	 nTrackLen = strlen(lpTrackInfo->TrackData);

	// is the last character a space in the string? if so remove it as it's more than likely not needed
	if (nTrackLen)
		if (lpTrackInfo->TrackData[nTrackLen-1] == 0x20)	//space
			lpTrackInfo->TrackData[nTrackLen-1] = 0;
		
	//if Helix search for MPC or BPI entries
	if (settings->OEM == OEM_HELIX || settings->OEM == OEM_PRO360)
	{
		char * pFoundStr;
		char * pStartStr;
		ULONG i         = 0;		
		WORD  bpi,bpc,trunc=7;
		ULONG ulTrackLen = strlen(lpTrackInfo->TrackData);
		
		if (ulTrackLen < 10)
			return ;
		//determine if the input string contains BPI210
		bpi=210;
		pFoundStr = strstr ( lpTrackInfo->TrackData, "BPI210,");
		if (!pFoundStr)	
		{
			bpi=75;
			trunc=6;
			pFoundStr = strstr ( lpTrackInfo->TrackData, "BPI75,");
		}
		if (pFoundStr)
		{
			lpTrackInfo->wBitsPerInch = bpi;
			//now remove BPI210, from the string
            pStartStr = lpTrackInfo->TrackData;
            i= (ULONG)(pFoundStr - pStartStr);
            strncpy((char *)&lpTrackInfo->TrackData[i], pFoundStr + trunc, ulTrackLen - trunc);
        }

        ulTrackLen = strlen(lpTrackInfo->TrackData);
        if (ulTrackLen < 10)
			return ;
		//determine if the input string contains MPC5
		bpc=2;
		pFoundStr = strstr ( lpTrackInfo->TrackData, "MPC1,");
		if (!pFoundStr)
		{
			bpc=3;
			pFoundStr = strstr ( lpTrackInfo->TrackData, "MPC4,");
		}
		if (!pFoundStr)
		{
			bpc=1;		
			pFoundStr = strstr ( lpTrackInfo->TrackData, "MPC5,");
		}			
		if (!pFoundStr)
		{
			bpc=0;		
			pFoundStr = strstr ( lpTrackInfo->TrackData, "MPC7,");
		}			
		if (pFoundStr)
		{
			lpTrackInfo->wBitsPerChar = bpc;
			if (bpc > 1) //binary or hex never insert sentinals
			{
				lpTrackInfo->bESAutoInsert = lpTrackInfo->bSSAutoInsert = FALSE;
			}
			//now remove MPCn, from the string
            pStartStr = lpTrackInfo->TrackData;
            i= (ULONG)(pFoundStr - pStartStr);
            strncpy((char *)&lpTrackInfo->TrackData[i], pFoundStr + 5, ulTrackLen - 5);
        }
		
        ulTrackLen = strlen(lpTrackInfo->TrackData);
        if (ulTrackLen < 10)
			return ;
		
		//determine if the input string contains COEH or COEL
		{
			BOOL bFoundCOEH = FALSE;
			BOOL bFoundCOEL = FALSE;
		
			pFoundStr = strstr ( lpTrackInfo->TrackData, "COEH,");
			if (pFoundStr)
				bFoundCOEH = TRUE;

			if (!pFoundStr)
			{
				pFoundStr = strstr ( lpTrackInfo->TrackData, "COEL,");
				if (pFoundStr)
					bFoundCOEL = TRUE;
			}
			
			if (pFoundStr)
			{
				//now remove COEH or COEL, from the string
				pStartStr = lpTrackInfo->TrackData;
				i= (ULONG)(pFoundStr - pStartStr);
				strncpy((char *)&lpTrackInfo->TrackData[i], pFoundStr + 5, ulTrackLen - 5);
			}
			if (bFoundCOEH || bFoundCOEL)
				lpTrackInfo->wCoercivity = (bFoundCOEL) ? 1 : 0;  	

		}		

	}
	
	// Perform any required post-processing of the magnetic encoding string
	if (!lpTrackInfo->bSSAutoInsert)
	{
		//If we are not required to automatically insert a start sentinel
		//assume that we have found one, then one will not get inserted.
		lpTrackInfo->bFoundSS = TRUE;
	}
	
	if (!lpTrackInfo->bESAutoInsert)
	{
		//If we are not required to automatically insert an end sentinel
		//assume that we have found one, then one will not get inserted.
		lpTrackInfo->bFoundES = TRUE;
	}

	if (lpTrackInfo->bFoundSS && lpTrackInfo->bFoundES)
	{
		// We are finished recording the track
		lpTrackInfo->bComplete = TRUE;
	}
	else if (lpTrackInfo->bFoundSS)
	{
		// Although a start sentinel was found, the end sentinel was not.
		// This is either because a buffer limit was reached, or the end sentinel
		// was missing from the test case. Either way an end sentinel must be added.

		ULONG ulESPos = (ULONG)strlen(lpTrackInfo->TrackData); // Default to add ES after the last recorded character

		if (lpTrackInfo->bBufferFull)
		{
			ulESPos--;   // Overwrite the last recorded character with an end sentinel
		}

		lpTrackInfo->TrackData[ulESPos] = (char)(EndSentinel(pdev, lpTrackInfo, settings) & 0xff);

		// We are finished recording the track
		lpTrackInfo->bComplete = TRUE;
	}
	else // No start sentinel found - (end sentinel may or may not have been
		 // seen but will be dealt with automatically)
	{
		ULONG ulSSPos = 0;
		ULONG ulESPos = 0;
		ULONG i       = 0;

		/*
		 * The start sentinel is missing. This is due to one of the following...
		 *  1) The SS was missing in the string provided by the application
		 *  2) There is no track data - just commands
		 * Now call LocateSSPosition() which parses the current string to find
		 * a boundary between track commands and track data (if this exists).
		 */
		ulSSPos = LocateStartSentinelPosition(lpTrackInfo);

		if (ulSSPos > 0)
		{
			// Valid track data was found

			// Calculate a new position for the end sentinel
			ulESPos = (ULONG)strlen(lpTrackInfo->TrackData);
			if (!lpTrackInfo->bFoundES)
			{
				ulESPos++;
			}

			// Ensure that we do not overrun the buffer length limit
			if (ulESPos >= MAG_MAXBUFFERNUM)
			{
				ulESPos = MAG_MAXBUFFERNUM - 1;
			}

			// Ensure that we do not overrun the track data limit
			if ((ulESPos - ulSSPos) > lpTrackInfo->ulMaxTrackLen)
			{
				ulESPos = ulSSPos + lpTrackInfo->ulMaxTrackLen - 1;
				lpTrackInfo->TrackData[ulESPos + 1] = (char)(0x00);
			}

			// Record the length of the track data
			lpTrackInfo->ulTrackLength = ulESPos - ulSSPos;

			// Move all track data across by one position
			for (i = ulESPos - 1; i > ulSSPos; i--)
			{
				lpTrackInfo->TrackData[i] = lpTrackInfo->TrackData[i - 1];
			}

			// Add in the start and end sentinel characters to complete the magnetic encoding string
			lpTrackInfo->TrackData[ulSSPos] = (char)(StartSentinel(pdev, lpTrackInfo, settings) & 0xff);
			lpTrackInfo->TrackData[ulESPos] = (char)(EndSentinel  (pdev, lpTrackInfo, settings) & 0xff);
		}
		else
		{
			// No track data was found, so do not add a start sentinel
		}

		// We are finished recording the track
		lpTrackInfo->bComplete = TRUE;

	}

	
	//Ready to handle another track
	iMagControl = 0;
	bFoundSpecifier    = FALSE;
	bFoundSpecifierEnd = FALSE;
	
	
	
	//ERROR_MSG(pmPDev->lpdm, TEXT("=======================\n"));
}

/******************************************************************************
 *  RecordMagneticEncodeTrackInfo()
 *      Hook DDI TextOut call and search magnetic encoding data.
 *
 *  Returns:
 *      GCB_SUPPORTED if callback handled, GCB_NOT_SUPPORTED if not
 *****************************************************************************/

int RecordMagneticEncodeTrackInfo
(
    PDEVDATA pdev,   // Pointer to our PDEV
    char *    lpStrIn,
    struct settings_ * settings

)
{
   
    //LPDVPDEV       lpDrvData   = pmPDev->pDrvData;
    LPMAGTRACKINFO lpTrackInfo = NULL;
    //PIFIMETRICS    pifi        = NULL;
    //LPWSTR         lpwStrIn    =rd pstro->pwszOrg;
    CHAR *         lpwTrans    = NULL;
    CHAR           CharIn     = 0;
    int            iNumChar, cGlyphs    = strlen(lpStrIn);
    int            ichar       = 0;
    int            iCount      = 0;
    BOOL           bRet        = FALSE;
    BOOL           bFoundTilde = FALSE;             // TRUE = Found a tilde as the first character in the string
    BOOL           bFoundTrack = FALSE;             // TRUE = Found a valid track number as the second character
    int            iTrack      = MAG_TRACK_NONE;    // Track index track being processed
	////LPDOCSTICKY    lpds = GetUICBDocumentProperties(pmPDev->lpdm);
	int			   i;
	LONG		   CurrentTextY = -1;
//	CHAR          wsTemp[100];

//   const char *magstring;
//   int maglen;
//   char *ret;
fprintf(stderr, "CATULTRA:cGlyphs:%u\n",cGlyphs);
//fprintf(stderr, (char *)lpStrIn);
//fprintf(stderr, "\n");
/*
	magstring = strstr(lpStrIn, " ");
	if (magstring)
		maglen = strlen(lpStrIn) - strlen(magstring);
	else
		maglen = strlen(lpStrIn);
	if (maglen > 512)
		maglen = 512;
	ret = malloc(maglen + 1);
	memset(ret, 0, maglen);
	memcpy(lpStrIn, lpStrIn, maglen);
	lpStrIn[maglen] = '\0';

   free (ret);
*/
fprintf(stderr, "cGlyphs:%u\n",cGlyphs);
//fprintf(stderr, "iNumChar:%u\n",iNumChar);

    /*
     * Please note that we start with the assumption that text will be
     * printed as usual (without extracting it as magnetic encoding data).
     * If magnetic encoding data is found, bRet will be set to TRUE
     * This prevents the text from being drawn.
     */





	/*
     * Please note that applications may split strings up into a series of smaller strings
     * (down to individual characters) - but we are only interested in completed strings.
     * The driver expects strings to be at least long enough to contain "~1,", "~2,", or "~3,"
     * so now we will disallow any strings that are too short, and validate the beginning
     * characters of the string
     */

	//Grab the current y coordinate of the string being passed so we close the track 
	//if no glyph position passed do not use it..
	//if (pstro->pgp)	
	//	CurrentTextY = pstro->pgp->ptl.y;
//fprintf(stderr, "L556\n");
	// Check if the string begins with tilde.
    if (!settings->bConcatenateMagStrings)
	{
		//fprintf(stderr, "L560\n");
		if (cGlyphs < 3)
		{
			fprintf(stderr, "L563\n");
			// Do not process the string in this case - (it will be printed as normal)
			bRet = FALSE;
			goto FINISH_RECORD;
		}
		//added check as to whether tilde is the first character passed in the string..
		if (ichar == 0 && lpStrIn[ichar++] == CHAR_TILDE)
		{
			// Flag that a tilde has been found
			bFoundTilde = TRUE;
			fprintf(stderr, "CATULTRA: Tilde~ Mag Specifier found.....\n");
			//DBGMSG(DBG_LEVEL_VERBOSE,L"Tilde~ Mag Specifier.....");
			// Fall through so that other characters can be processed below.
		}
		else
		{
			// This string is not magnetic encoding data so do not process it.
			bRet = FALSE;
			goto FINISH_RECORD;
		}

		// Check for the character immediately following the tilde
		if (bFoundTilde)
		{
			// Obtain and test for a valid track number in the string provided.
			CharIn = lpStrIn[ichar++];
fprintf(stderr, "CharIn:%c\n",CharIn);
			// If the character after the track number is a comma, ignore it as
			// one will be inserted anyway
			if (lpStrIn[ichar] == CHAR_COMMA)
			{
				ichar++;
			}

			switch (CharIn)
			{
				case INDEX_CHAR_TRACK0: iTrack = MAG_TRACK_INDEX0; break;
				case INDEX_CHAR_TRACK1: iTrack = MAG_TRACK_INDEX1; break;
				case INDEX_CHAR_TRACK2: iTrack = MAG_TRACK_INDEX2; break;
				case INDEX_CHAR_TRACK3: iTrack = MAG_TRACK_INDEX3; break;
				case INDEX_CHAR_TRACK4: iTrack = MAG_TRACK_INDEX4; break;

				default:
					// WARNING : We could not find any valid track number.
					iTrack = MAG_TRACK_NONE;
					fprintf(stderr, "CATULTRA: Printing string as normal\n");

					// Do not process the string in this case - (it will be printed as normal)
					bRet = FALSE;
					goto FINISH_RECORD;
					break;
			}

			//wsprintf(wsTemp, TEXT(".....Track %d"), iTrack);
			//DBGMSG(DBG_LEVEL_VERBOSE,wsTemp);

			// Obtain track info structure for the appropriate track number
			lpTrackInfo = &pdev->magTrackInfo[iTrack];

			// Test whether there is already data recorded for this track on this page
			if (lpTrackInfo->TrackData[0] != 0)
			{
				// We recorded this track earlier on within the page.
				// Do do not process the string further - (it will be printed as normal)
				iTrack = MAG_TRACK_NONE;
    			bRet = FALSE;
				goto FINISH_RECORD;
			}
			else
			{
				// Flag that we are processing a new track for this page
				bFoundTrack = TRUE;

				//Clear the memory buffer for this track
				memset(lpTrackInfo->TrackData, 0, sizeof(lpTrackInfo->TrackData));

				// Record the track number (and comma) in the output string.
				lpTrackInfo->TrackData[strlen(lpTrackInfo->TrackData)] = (char)(CharIn & 0xff);
				lpTrackInfo->TrackData[strlen(lpTrackInfo->TrackData)] = (char)(CHAR_COMMA & 0xff);

				// Process the string
    			bRet = TRUE;

				// Fall through so that other characters can be processed below.
			}
		}

		// Look for (and process) characters which follow after the tilde, track number and comma
		if (bFoundTilde && bFoundTrack && iTrack != MAG_TRACK_NONE)
		{
			// Record the number of characters remaining in the hooked text
			iNumChar = cGlyphs - ichar;
fprintf(stderr, "iNumChar:%u\n",iNumChar);
			// The string is being processed by the mini-driver and will not be printed
			bRet = TRUE;

			// 
			for (iCount = 0; iCount < iNumChar; iCount++)
			{
				CharIn = lpStrIn[ichar++];
fprintf(stderr, "CAT:CharIn:%c\n",CharIn);
				// Check that this character is in the allowable range for this track.
				if (ValidMagDataChar(lpTrackInfo, CharIn)
				||  isupper(CharIn)
				||  isdigit(CharIn)
				||  (CharIn == ','))
					{
					// Store the valid character into the track data for this track
					lpTrackInfo->TrackData[strlen(lpTrackInfo->TrackData)] = (char)(CharIn & 0xff);

				   // Check for the start sentinel of this track
					if (CharIn == StartSentinel(pdev, lpTrackInfo, settings))
					{
						// Flag that we found the SS so that we start counting track data characters
						lpTrackInfo->bFoundSS = TRUE;
					}

					// Check for the end sentinel of this track
					if (CharIn == EndSentinel(pdev, lpTrackInfo, settings))
					{
						// Flag that we have found an ES
						lpTrackInfo->bFoundES = TRUE;

						// Break out of the string construction loop
						break;
					}

					if (lpTrackInfo->bFoundSS)
					{
						// Increase the count of the track data length
						lpTrackInfo->ulTrackLength++;

						// Ensure that we do not overrun the track data limit
						if (lpTrackInfo->ulTrackLength >= lpTrackInfo->ulMaxTrackLen)
						{
							/*
							 * WARNING: flag that we have hit the track data length limit
							 * and break out of the string construction loop
							 */
							lpTrackInfo->bBufferFull = TRUE;
							break;
						}
					}

					// Ensure that we do not overrun the overall track buffer limit (commands + track data)
					if (strlen(lpTrackInfo->TrackData) >= MAG_MAXBUFFERNUM)
					{
						/*
						 * WARNING: flag that we have hit the track buffer length limit
						 * and break out of the string construction loop
						 */
						lpTrackInfo->bBufferFull = TRUE;
						break;
					}
				}
			}

			// Perform any required post-processing of the magnetic encoding string
fprintf(stderr, "CAT:about to call post\n");
			PerformPostProcessing(pdev,lpTrackInfo, settings);	
fprintf(stderr, "CAT:returned o call post\n");
		}
	}
	else
	{
//		+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//		For now, this is a duplication of the code used with tilde specifier.  This
//		is in order to ensure that normal use of tilde for magnetic encoding is not
//		compromised.
//		+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

		//DBGMSG(DBG_LEVEL_VERBOSE,L"Alternate Mag Specifier");
		//if we are recording the string and the next string passed is on a different line close the track and start analysing the passed string
		if (iMagControl == 2)// && pdev->LastMagTextY != -1 && CurrentTextY != -1 && CurrentTextY > pdev->LastMagTextY )
		{
			// Obtain track info structure for the appropriate track number
			lpTrackInfo = &pdev->magTrackInfo[iConcTrack];		
			PerformPostProcessing(pdev,lpTrackInfo, settings);
			bFirstCommaFound   = FALSE;
		}	
		
		if (bFoundSpecifierEnd)
		{
			//DBGMSG(DBG_LEVEL_VERBOSE,L"Mag String already handled");
			// Magnetic encoding data already sorted so do not process it.

			bRet = FALSE;
			goto FINISH_RECORD;
		}
		
		while ((iCount++ < (int)cGlyphs) && !bFoundSpecifierEnd)
		{
			switch (iMagControl)
			{
			case 0:
				//added check as to whether tilde is the first character passed in the string..
				if (ichar == 0 && lpStrIn[ichar++] == CHAR_TILDE)
//				if (lpStrIn[ichar] == lpds->sMagSpecifier[0])
				{
					fprintf(stderr, "CATULTRA: Mag Specifier FOUND\n");

					//DBGMSG(DBG_LEVEL_VERBOSE,L"Mag Specifier FOUND");
					bFoundSpecifier = TRUE;
					iMagControl = 1;
					pdev->LastMagTextY = CurrentTextY;					
				}
				break;

			case 1:
				//DBGMSG(DBG_LEVEL_VERBOSE,L"Handling Track No.");
				CharIn = lpStrIn[ichar++];
				switch (CharIn)
				{
				case INDEX_CHAR_TRACK0: iConcTrack = MAG_TRACK_INDEX0; break;
				case INDEX_CHAR_TRACK1: iConcTrack = MAG_TRACK_INDEX1; break;
				case INDEX_CHAR_TRACK2: iConcTrack = MAG_TRACK_INDEX2; break;
				case INDEX_CHAR_TRACK3: iConcTrack = MAG_TRACK_INDEX3; break;
				case INDEX_CHAR_TRACK4: iConcTrack = MAG_TRACK_INDEX4; break;

				default:
					// WARNING : We could not find any valid track number.
					iConcTrack = MAG_TRACK_NONE;

					// Do not process the string in this case - (it will be printed as normal)
					bRet = FALSE;
					goto FINISH_RECORD;
					break;
				}
				bFirstCommaFound = FALSE;
				// Obtain track info structure for the appropriate track number
				lpTrackInfo = &pdev->magTrackInfo[iConcTrack];

				//Clear the memory buffer for this track
				memset(lpTrackInfo->TrackData, 0, sizeof(lpTrackInfo->TrackData));
				
				// Record the track number (and comma) in the output string.
				lpTrackInfo->TrackData[0] = (char)(CharIn & 0xff);
				lpTrackInfo->TrackData[1] = (char)(CHAR_COMMA & 0xff);
				iMagControl = 2;
				break;
			
			case 2:
				if (lpStrIn[ichar] == CHAR_COMMA && !bFirstCommaFound)
				{
					//Skip over the comma if one has been included
					ichar++;
					//mark the fact we've passed the first comma delimiter as we want the rest 
					bFirstCommaFound = TRUE;
				}

				if (bFoundSpecifier 
				&& !bFoundSpecifierEnd
				&& (iConcTrack != MAG_TRACK_NONE))
				{
					// Record the number of characters remaining in the hooked text
					iNumChar = cGlyphs - ichar;

					// The string is being processed by the mini-driver and will not be printed
					bRet = TRUE;

					lpTrackInfo = &pdev->magTrackInfo[iConcTrack];

					// Convert Unicode characters into ascii.
					for (i = 0; i < iNumChar; i++)
					{
						CharIn = lpStrIn[ichar++];

						if (CharIn == CHAR_TILDE )
//						if (lpStrIn[ichar] == lpds->sMagSpecifier[0])
						{
							bFoundSpecifierEnd = TRUE;
							break;
						}

						// Check that this character is in the allowable range for this track.
						if (ValidMagDataChar(lpTrackInfo, CharIn)
						||  isupper(CharIn)
						||  isdigit(CharIn)
						||  (CharIn == ','))
						{	
							// Store the valid character into the track data for this track
							lpTrackInfo->TrackData[strlen(lpTrackInfo->TrackData)] 
															= (char)(CharIn & 0xff);

							// Check for the start sentinel of this track
							if (CharIn == StartSentinel(pdev,lpTrackInfo,settings))
							{
								// Flag that we found the SS so that we start counting track data characters
								lpTrackInfo->bFoundSS = TRUE;
							}

							// Check for the end sentinel of this track
							if (CharIn == EndSentinel(pdev,lpTrackInfo,settings))
							{
								// Flag that we have found an ES
								lpTrackInfo->bFoundES = TRUE;
								bFoundSpecifierEnd = TRUE;
								iMagControl = 0;
							}

							if (lpTrackInfo->bFoundSS)
							{
								// Increase the count of the track data length
								lpTrackInfo->ulTrackLength++;

								// Ensure that we do not overrun the track data limit
								if (lpTrackInfo->ulTrackLength >= lpTrackInfo->ulMaxTrackLen)
								{
									/*
									 * WARNING: flag that we have hit the track data length limit
									 * and break out of the string construction loop
									 */
									lpTrackInfo->bBufferFull = TRUE;
									break;
								}
							}

							// Ensure that we do not overrun the overall track buffer limit (commands + track data)
							if (strlen(lpTrackInfo->TrackData) >= MAG_MAXBUFFERNUM)
							{
								/*
								 * WARNING: flag that we have hit the track buffer length limit
								 * and break out of the string construction loop
								 */
								lpTrackInfo->bBufferFull = TRUE;
								break;
							}
						}
					}
				}
				break;
			}
		}

		// Process the string only if a tilde has been sent
		if (bFoundSpecifier)
            bRet = TRUE;

		if (bFoundSpecifierEnd)
			PerformPostProcessing(pdev,lpTrackInfo,settings);		

	}
//	++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

FINISH_RECORD:


    // Free up any device data here
    if (lpwTrans != NULL)
    {
        free(lpwTrans);
    }

	return bRet;
}


/******************************************************************************
 *
 *  LocateStartSentinelPosition()
 *      Find the correct position for a start sentinel if it is missing from the string.
 *      This function does not modify the string in any way.
 *
 *  Returns:
 *      int     Position
 *              0 if a suitable position is not found
 *
 *****************************************************************************/

static ULONG LocateStartSentinelPosition
(
    LPMAGTRACKINFO  lpTrackInfo     // Pointer to the magnetic track information structure
)
{
    ULONG   ulPos     = 0;
    int     i, j      = 0;
    BOOL    bFoundCmd = TRUE;
    char    strData[MAG_MAXBUFFERNUM];  // For copy of lpTrackInfo->TrackData

    // Test that lpTrackInfo is not empty
    if (lpTrackInfo == NULL)
    {
        return 0;
    }

    // Make a local (upper-case) copy of lpTrackInfo->TrackData for processing
    strcpy( strData, lpTrackInfo->TrackData);

    // Loop from the first comma to the last but one character
    for (i = 1; i < (int)strlen(strData) - 1; i++)
    {
        // Locate a comma
        if (lpTrackInfo->TrackData[i] == CHAR_COMMA)
        {
            // Assume that the following command is invalid
            bFoundCmd = FALSE;

            // Loop the list of valid commands for the track
            for (j = 0; j < sizeof(ValidCommands) / MAX_COMMAND_SIZE; j++)
            {
                // Compare the characters immediately following the comma against a command
                if (strstr(strData + i + 1, ValidCommands[j]) == strData + i + 1)
                {
                    // Flag that a valid command was found, and break out of the inner loop
                    bFoundCmd = TRUE;
                    break;
                }
            }
        }

        // If no valid command was found then we must have found the start sentinel position
        if (!bFoundCmd)
        {
            // Record the current position and break out of the outer loop
            ulPos = i + 1;
            break;
        }
    }

	// Prevent start sentinel from being added to the very end of the string
    if (ulPos == (int)strlen(strData))
    {
        ulPos = 0;
    }

    return ulPos;
}
