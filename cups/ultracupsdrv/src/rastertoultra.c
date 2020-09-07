/*
 * Magicard Ltd
 *
 * CUPS Filter
 *
 * [ Linux ]
 * compile cmd: gcc -Wl,-rpath,/usr/lib -Wall -fPIC -O2 -o rastertoultra rastertoultra.c magencd.c -lcupsimage -lcups
 * compile requires cups-devel-1.1.19-13.i386.rpm (version not neccessarily important?)
 * find cups-devel location here: http://rpmfind.net/linux/rpm2html/search.php?query=cups-devel&submit=Search+...&system=&arch=
 *
 * [ Mac OS X ]
 * compile cmd: gcc -Wall -fPIC -o rastertoultra rastertoultra.c magencd.c -lcupsimage -lcups -arch i386 -DMACOSX
 * compile requires cupsddk-1.2.3.dmg (version not neccessarily important?)
 * find cupsddk location here: http://www.cups.org/software.php
 */

/*
 * Copyright (C) 2018 Magicard Ltd
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/*
 * Include necessary headers...
 */

#include <cups/cups.h>
#include <cups/ppd.h>
#include <cups/raster.h>
#include <stdlib.h>
#include <fcntl.h>


#include "magigen.h"
#undef COMMS_H
#include "comms.h"
#include "pr2comms.h"

#include "gndefs.h"
#include <stdint.h>
#include "command.h"
#include "colortab.h"
#include "colrmtch.c"
///#include "colrcorr.c"
#include "magiry.h"
#define FROM_ULTRA
#include "magencd.h"
#include <math.h>

#ifdef RPMBUILD

#include <dlfcn.h>

typedef cups_raster_t * (*cupsRasterOpen_fndef)(int fd, cups_mode_t mode);
typedef unsigned (*cupsRasterReadHeader2_fndef)(cups_raster_t *r, cups_page_header2_t *h);
typedef unsigned (*cupsRasterReadPixels_fndef)(cups_raster_t *r, unsigned char *p, unsigned len);
typedef void (*cupsRasterClose_fndef)(cups_raster_t *r);

static cupsRasterOpen_fndef cupsRasterOpen_fn;
static cupsRasterReadHeader2_fndef cupsRasterReadHeader2_fn;
static cupsRasterReadPixels_fndef cupsRasterReadPixels_fn;
static cupsRasterClose_fndef cupsRasterClose_fn;

#define CUPSRASTEROPEN (*cupsRasterOpen_fn)
#define CUPSRASTERREADHEADER (*cupsRasterReadHeader2_fn)
#define CUPSRASTERREADPIXELS (*cupsRasterReadPixels_fn)
#define CUPSRASTERCLOSE (*cupsRasterClose_fn)

typedef void (*ppdClose_fndef)(ppd_file_t *ppd);
typedef ppd_choice_t * (*ppdFindChoice_fndef)(ppd_option_t *o, const char *option);
typedef ppd_choice_t * (*ppdFindMarkedChoice_fndef)(ppd_file_t *ppd, const char *keyword);
typedef ppd_option_t * (*ppdFindOption_fndef)(ppd_file_t *ppd, const char *keyword);
typedef void (*ppdMarkDefaults_fndef)(ppd_file_t *ppd);
typedef ppd_file_t * (*ppdOpenFile_fndef)(const char *filename);

typedef void (*cupsFreeOptions_fndef)(int num_options, cups_option_t *options);
typedef int (*cupsParseOptions_fndef)(const char *arg, int num_options, cups_option_t **options);
typedef int (*cupsMarkOptions_fndef)(ppd_file_t *ppd, int num_options, cups_option_t *options);

static ppdClose_fndef ppdClose_fn;
static ppdFindChoice_fndef ppdFindChoice_fn;
static ppdFindMarkedChoice_fndef ppdFindMarkedChoice_fn;
static ppdFindOption_fndef ppdFindOption_fn;
static ppdMarkDefaults_fndef ppdMarkDefaults_fn;
static ppdOpenFile_fndef ppdOpenFile_fn;

static cupsFreeOptions_fndef cupsFreeOptions_fn;
static cupsParseOptions_fndef cupsParseOptions_fn;
static cupsMarkOptions_fndef cupsMarkOptions_fn;

#define PPDCLOSE            (*ppdClose_fn)
#define PPDFINDCHOICE       (*ppdFindChoice_fn)
#define PPDFINDMARKEDCHOICE (*ppdFindMarkedChoice_fn)
#define PPDFINDOPTION       (*ppdFindOption_fn)
#define PPDMARKDEFAULTS     (*ppdMarkDefaults_fn)
#define PPDOPENFILE         (*ppdOpenFile_fn)

#define CUPSFREEOPTIONS     (*cupsFreeOptions_fn)
#define CUPSPARSEOPTIONS    (*cupsParseOptions_fn)
#define CUPSMARKOPTIONS     (*cupsMarkOptions_fn)

#else

#define CUPSRASTEROPEN cupsRasterOpen
#define CUPSRASTERREADHEADER cupsRasterReadHeader2
#define CUPSRASTERREADPIXELS cupsRasterReadPixels
#define CUPSRASTERCLOSE cupsRasterClose

#define PPDCLOSE ppdClose
#define PPDFINDCHOICE ppdFindChoice
#define PPDFINDMARKEDCHOICE ppdFindMarkedChoice
#define PPDFINDOPTION ppdFindOption
#define PPDMARKDEFAULTS ppdMarkDefaults
#define PPDOPENFILE ppdOpenFile

#define CUPSFREEOPTIONS cupsFreeOptions
#define CUPSPARSEOPTIONS cupsParseOptions
#define CUPSMARKOPTIONS cupsMarkOptions

#endif

#define MIN(a,b) ( ((a) < (b)) ? (a) : (b) )
#define MAX(a,b) ( ((a) > (b)) ? (a) : (b) )

#define RgbToGray(r,g,b) (((r)*74L + (g)*155L + (b)*27L) >> 8)

#define GetCValue(cmyk) ((BYTE)((cmyk)>>24))
#define GetMValue(cmyk) ((BYTE)((cmyk)>>16))
#define GetYValue(cmyk) ((BYTE)((cmyk)>>8))
#define GetKValue(cmyk) ((BYTE)(cmyk))

#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define CMYK(c,m,y,k)  ((DWORD)(((BYTE)(c)<<24|((BYTE)(m)<<16))|((BYTE)(y)<<8))|((BYTE)(k)))

#define FALSE 0
#define TRUE  (!FALSE)

#define FOCUS_LEFT      0
#define FOCUS_CENTER    1
#define FOCUS_RIGHT     2

#define DATACANCEL_NO_USE 0
#define DATACANCEL_DOC    1
//TODO??
#define LOBYTE(w)		((BYTE)(w))
#define HIBYTE(w)		((BYTE)(((WORD)(w) >> 8) & 0xFF))
#define LOWORD(l)		((WORD)(l))
#define HIWORD(l)		((WORD)(((DWORD)(l) >> 16) & 0xFFFF))
#define MAKELONG(a,b)	((LONG)(((WORD)(((DWORD)(a)) & 0xffff)) | ((DWORD)((WORD)(((DWORD )(b)) & 0xffff))) << 16))
//((a) << 16 & (b))
#define MAKEWORD(a,b)	((b & 0xFF) | ((a & 0xff)<<8))
#define GetRValue(x)		(LOBYTE(x))
#define GetGValue(x)		(LOBYTE(((WORD)(x)) >> 8 ))
#define GetBValue(x)		(LOBYTE((x) >> 16))
#define HALO_WIDTH		(HALO_RADIUS * 2 + 1)
#define MAX_HALO_RADIUS		50
#define MAX_HALO_WIDTH		(MAX_HALO_RADIUS * 2 + 1)
#define min(a,b)            ((a) < (b) ? (a) : (b))

// macro converts RGB888 to RGB565
#define RGB_565(x)	((((unsigned)(GetRValue(x)) << 8) & 0xF800) | \
(((unsigned)(GetGValue(x)) << 3) & 0x7E0)  |  \
(((unsigned)(GetBValue(x)) >> 3)))

// PDEVDATA flag definitions

#define PDEV_PGSAVED            0x00000001  // 1 = page level save in effect
#define PDEV_PRINTCOLOR         0x00000002  // 1 = using color
#define PDEV_STARTDOC           0x00000004  // 1 = Escape(STARTDOC) called
#define PDEV_CANCELDOC          0x00000008  // 1 = EngWrite failed
#define PDEV_DOIMAGEMASK        0x00000010  // 1 = doing image mask now
#define PDEV_NOTSRCBLT          0x00000020  // 1 = not src first
#define PDEV_MANUALFEED         0x00000100  // 1 = using manual feed
#define PDEV_UTILSSENT          0x00000200  // 1 = Utils Procset sent
#define PDEV_BMPPATSENT         0x00000400  // 1 = Pattern Bmp Procset sent
#define PDEV_IMAGESENT          0x00000800  // 1 = Image Procset sent
#define PDEV_PROCSET            0x00004000  // 1 = procset part of header sent
#define PDEV_WITHINPAGE         0x00008000  // 1 = withing save/restore of page
#define PDEV_EPSPRINTING_ESCAPE 0x00010000  // 1 = this escape called
#define PDEV_ADDMSTT            0x00020000  // 1 = prefix TT font name with MSTT
#define PDEV_NOFIRSTSAVE        0x00040000  // 1 = don't want 1st page save/restore.
#define PDEV_RAWBEFOREPROCSET   0x00080000  // 1 = rawdata sent before procset sent.
#define PDEV_RESETPDEV          0x00100000  // set following a ResetPDEV, cleared at StartDoc
#define PDEV_IGNORE_GDI         0x00200000  // set to ignore GDI calls, cleared at StartPage
#define PDEV_IGNORE_STARTPAGE   0x00400000  // set to ingore DrvStartPage
#define PDEV_SAME_FORMTRAY      0x00800000  // same form/tray after DrvResetPDEV

#define DRIVER_SIGNATURE        'MAGD'      // driver signature

typedef struct _RGB
{
    BYTE  R;
    BYTE  G;
    BYTE  B;
} RGB;

typedef struct _charPixel {
	
	unsigned char           r;	
	unsigned char           g; 	
	unsigned char           b;	
	unsigned char           c;	
	unsigned char           m;	
	unsigned char           y;
} CHARPIXEL;

#define CHUNK 16384

unsigned int  GridDelta = 4 ;// Idealy a power of 2,the driver assumes 4 though 8 works just fine
#define GridSize (unsigned char)(256 / GridDelta) + 1
#define MaxGrid  65 // used when defining the LUT's

// LUT Processing
#define CYAN	0
#define MAGENTA	1
#define YELLOW	2

//#include "crc32.h"
/*----------------------------------------------------------------------------*/

unsigned int table[256] = {
    0x00000000u, 0x04c11db7u, 0x09823b6eu, 0x0d4326d9u,
    0x130476dcu, 0x17c56b6bu, 0x1a864db2u, 0x1e475005u,
    0x2608edb8u, 0x22c9f00fu, 0x2f8ad6d6u, 0x2b4bcb61u,
    0x350c9b64u, 0x31cd86d3u, 0x3c8ea00au, 0x384fbdbdu,
    0x4c11db70u, 0x48d0c6c7u, 0x4593e01eu, 0x4152fda9u,
    0x5f15adacu, 0x5bd4b01bu, 0x569796c2u, 0x52568b75u,
    0x6a1936c8u, 0x6ed82b7fu, 0x639b0da6u, 0x675a1011u,
    0x791d4014u, 0x7ddc5da3u, 0x709f7b7au, 0x745e66cdu,
    0x9823b6e0u, 0x9ce2ab57u, 0x91a18d8eu, 0x95609039u,
    0x8b27c03cu, 0x8fe6dd8bu, 0x82a5fb52u, 0x8664e6e5u,
    0xbe2b5b58u, 0xbaea46efu, 0xb7a96036u, 0xb3687d81u,
    0xad2f2d84u, 0xa9ee3033u, 0xa4ad16eau, 0xa06c0b5du,
    0xd4326d90u, 0xd0f37027u, 0xddb056feu, 0xd9714b49u,
    0xc7361b4cu, 0xc3f706fbu, 0xceb42022u, 0xca753d95u,
    0xf23a8028u, 0xf6fb9d9fu, 0xfbb8bb46u, 0xff79a6f1u,
    0xe13ef6f4u, 0xe5ffeb43u, 0xe8bccd9au, 0xec7dd02du,
    0x34867077u, 0x30476dc0u, 0x3d044b19u, 0x39c556aeu,
    0x278206abu, 0x23431b1cu, 0x2e003dc5u, 0x2ac12072u,
    0x128e9dcfu, 0x164f8078u, 0x1b0ca6a1u, 0x1fcdbb16u,
    0x018aeb13u, 0x054bf6a4u, 0x0808d07du, 0x0cc9cdcau,
    0x7897ab07u, 0x7c56b6b0u, 0x71159069u, 0x75d48ddeu,
    0x6b93dddbu, 0x6f52c06cu, 0x6211e6b5u, 0x66d0fb02u,
    0x5e9f46bfu, 0x5a5e5b08u, 0x571d7dd1u, 0x53dc6066u,
    0x4d9b3063u, 0x495a2dd4u, 0x44190b0du, 0x40d816bau,
    0xaca5c697u, 0xa864db20u, 0xa527fdf9u, 0xa1e6e04eu,
    0xbfa1b04bu, 0xbb60adfcu, 0xb6238b25u, 0xb2e29692u,
    0x8aad2b2fu, 0x8e6c3698u, 0x832f1041u, 0x87ee0df6u,
    0x99a95df3u, 0x9d684044u, 0x902b669du, 0x94ea7b2au,
    0xe0b41de7u, 0xe4750050u, 0xe9362689u, 0xedf73b3eu,
    0xf3b06b3bu, 0xf771768cu, 0xfa325055u, 0xfef34de2u,
    0xc6bcf05fu, 0xc27dede8u, 0xcf3ecb31u, 0xcbffd686u,
    0xd5b88683u, 0xd1799b34u, 0xdc3abdedu, 0xd8fba05au,
    0x690ce0eeu, 0x6dcdfd59u, 0x608edb80u, 0x644fc637u,
    0x7a089632u, 0x7ec98b85u, 0x738aad5cu, 0x774bb0ebu,
    0x4f040d56u, 0x4bc510e1u, 0x46863638u, 0x42472b8fu,
    0x5c007b8au, 0x58c1663du, 0x558240e4u, 0x51435d53u,
    0x251d3b9eu, 0x21dc2629u, 0x2c9f00f0u, 0x285e1d47u,
    0x36194d42u, 0x32d850f5u, 0x3f9b762cu, 0x3b5a6b9bu,
    0x0315d626u, 0x07d4cb91u, 0x0a97ed48u, 0x0e56f0ffu,
    0x1011a0fau, 0x14d0bd4du, 0x19939b94u, 0x1d528623u,
    0xf12f560eu, 0xf5ee4bb9u, 0xf8ad6d60u, 0xfc6c70d7u,
    0xe22b20d2u, 0xe6ea3d65u, 0xeba91bbcu, 0xef68060bu,
    0xd727bbb6u, 0xd3e6a601u, 0xdea580d8u, 0xda649d6fu,
    0xc423cd6au, 0xc0e2d0ddu, 0xcda1f604u, 0xc960ebb3u,
    0xbd3e8d7eu, 0xb9ff90c9u, 0xb4bcb610u, 0xb07daba7u,
    0xae3afba2u, 0xaafbe615u, 0xa7b8c0ccu, 0xa379dd7bu,
    0x9b3660c6u, 0x9ff77d71u, 0x92b45ba8u, 0x9675461fu,
    0x8832161au, 0x8cf30badu, 0x81b02d74u, 0x857130c3u,
    0x5d8a9099u, 0x594b8d2eu, 0x5408abf7u, 0x50c9b640u,
    0x4e8ee645u, 0x4a4ffbf2u, 0x470cdd2bu, 0x43cdc09cu,
    0x7b827d21u, 0x7f436096u, 0x7200464fu, 0x76c15bf8u,
    0x68860bfdu, 0x6c47164au, 0x61043093u, 0x65c52d24u,
    0x119b4be9u, 0x155a565eu, 0x18197087u, 0x1cd86d30u,
    0x029f3d35u, 0x065e2082u, 0x0b1d065bu, 0x0fdc1becu,
    0x3793a651u, 0x3352bbe6u, 0x3e119d3fu, 0x3ad08088u,
    0x2497d08du, 0x2056cd3au, 0x2d15ebe3u, 0x29d4f654u,
    0xc5a92679u, 0xc1683bceu, 0xcc2b1d17u, 0xc8ea00a0u,
    0xd6ad50a5u, 0xd26c4d12u, 0xdf2f6bcbu, 0xdbee767cu,
    0xe3a1cbc1u, 0xe760d676u, 0xea23f0afu, 0xeee2ed18u,
    0xf0a5bd1du, 0xf464a0aau, 0xf9278673u, 0xfde69bc4u,
    0x89b8fd09u, 0x8d79e0beu, 0x803ac667u, 0x84fbdbd0u,
    0x9abc8bd5u, 0x9e7d9662u, 0x933eb0bbu, 0x97ffad0cu,
    0xafb010b1u, 0xab710d06u, 0xa6322bdfu, 0xa2f33668u,
    0xbcb4666du, 0xb8757bdau, 0xb5365d03u, 0xb1f740b4u
};

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



struct command
{
    int    length;
    char* command;
};
//local prototypes

void initializeSettings(char * commandLineOptionSettings, struct settings_ * settings, PDEVDATA pdev);
void debugPrintSettings(struct settings_ * settings);
int getOptionChoiceIndex(const char * choiceName, ppd_file_t * ppd);
void getPageWidthPageHeight(ppd_file_t * ppd, struct settings_ * settings);
void myWrite( PDEVDATA  pdev, char *chData, DWORD dwSize );
void myWritecs( PDEVDATA  pdev, char *chData, DWORD dwSize,	SHORT indentlevel);
void crc32update(PDEVDATA pdev, const void *data, DWORD length);

ULONG CMYK_to_RGB(BYTE,BYTE,BYTE,BYTE,WORD);

VOID CopyKPlaneToBuffer
(
    PDEVDATA   pdev,        // Pointer to our PDEVICE
    LPBYTE     lpSrc,       // Pointer to the image data. If this is null, use pStripObj.
    PBYTE      pSurface,    // Pointer to the surface object
    PSETTINGS  pSettings,   // Pointer to the surface object
    LONG       lYOffset		// Current Y position
);

static unsigned char Cyan   [MaxGrid][MaxGrid][MaxGrid][NO_OF_INTENTS];
static unsigned char Magenta[MaxGrid][MaxGrid][MaxGrid][NO_OF_INTENTS];
static unsigned char Yellow [MaxGrid][MaxGrid][MaxGrid][NO_OF_INTENTS];

unsigned int CMY[5];
unsigned int CMYX[3];

BOOL bLUTLoaded = FALSE;

inline void debugPrintSettings(struct settings_ * settings)
{
  fprintf(stderr, "DEBUG: pageType = %d\n"    , settings->pageType);

}


/*****************************************************************************
 *  myWrite()
 *      Output to file or spooler, binary data
 *
 *  Returns:
 *      None
 *****************************************************************************/
inline void myWrite
(
    PDEVDATA  pdev,     // Pointer to our PDEV
	char     *chData,
    DWORD     dwSize
)
{
    DWORD i = 0;
	char * pByte = chData;

    for (; i < dwSize; i++)
    {
        putchar(*pByte++);
    }
}

inline void myWritecs
(
    PDEVDATA    pdev,
    char     *chData,
    DWORD     cbbuf,
	SHORT		indentlevel
)
{
CHAR 	szOutputBuf[256];
WORD	outputbuf_idx = 0;
WORD	cnt;
DWORD i = 0;
char * pbuf = chData;

	//sort the length
	if (cbbuf == 0)
	{
		cbbuf = strlen(pbuf);

		//clear out local copy
		memset((PBYTE)szOutputBuf, 0, sizeof(szOutputBuf));

		if (indentlevel > 0)
		{
			for (cnt = indentlevel; cnt; cnt--)
				szOutputBuf[outputbuf_idx++] = 9; //tab
		}
		// Buffer the data before sending it to the device
		memcpy ((PBYTE)&szOutputBuf[outputbuf_idx], pbuf, cbbuf);
		cbbuf += indentlevel;	
		pbuf = (char *)&szOutputBuf;
	}
	//pdev->epjobSize += cbbuf;
	//keep the crc checksum uptodate

	crc32update(pdev, pbuf, cbbuf);
	
	for (; i < cbbuf; i++)
        putchar(*pbuf++);

  /*   //used for splitting jobs.. invoked from a private DrvEscape SPLITJOB invoked in DrvDocumentEvent :)
    if (pdev->hFile)
    {
        WriteFile(pdev->hFile, pbuf, cbbuf, &cbwritten, NULL);
        return TRUE;
     }  */ 
	
	
 //  if (pdev->writebuf.count + cbbuf > pdev->writebuf.max) {

        // Flush the write buffer

        /* if (! myWriteSpool(pdev))
            return FALSE; */

        // Don't buffer large amount of data

        // while (cbbuf > BUFFERSIZE / 4) {

            // // Don't write large chunk of data at once
            // // to avoid choking the spooler

            // cb = (cbbuf > BUFFERSIZE) ? BUFFERSIZE : cbbuf;

            // if (! WritePrinter(pdev->hPrinter, pbuf, cb, &cbwritten) ||
                // cb != cbwritten)
            // {
                // DBGERRMSG("WRITEPRINTER");
                // pdev->dwFlags |= PDEV_CANCELDOC;
                // return FALSE;
            // }

            // cbbuf -= cb;
            // pbuf += cb;
        // }
 //   }

    // if (cbbuf > 0) {

        // memcpy(pdev->writebuf.pnext, pbuf, cbbuf);
        // pdev->writebuf.pnext += cbbuf;
        // pdev->writebuf.count += cbbuf;
    // }

    return;
}

inline void outputAsciiEncodedLength(int length)
{
    printf("%d",length);
}

inline void outputNullTerminator()
{
    putchar(0x00);
}

inline int getOptionChoiceIndex(const char * choiceName, ppd_file_t * ppd)
{
    ppd_choice_t * choice;
    ppd_option_t * option;

    choice = PPDFINDMARKEDCHOICE(ppd, choiceName);
    if (choice == NULL)
    {
        if ((option = PPDFINDOPTION(ppd, choiceName))          == NULL) return -1;
        if ((choice = PPDFINDCHOICE(option,option->defchoice)) == NULL) return -1;
    }

    return atoi(choice->choice);
}

inline void getPageWidthPageHeight(ppd_file_t * ppd, struct settings_ * settings)
{
    ppd_choice_t * choice;
    ppd_option_t * option;

    char width[20];
    int widthIdx;

    char height[20];
    int heightIdx;

    char * pageSize;
    int idx;

    int state;

    choice = PPDFINDMARKEDCHOICE(ppd, "PageSize");
    if (choice == NULL)
    {
        option = PPDFINDOPTION(ppd, "PageSize");
        choice = PPDFINDCHOICE(option,option->defchoice);
    }

    widthIdx = 0;
    memset(width, 0x00, sizeof(width));

    heightIdx = 0;
    memset(height, 0x00, sizeof(height));

    pageSize = choice->choice;
    idx = 0;

    state = 0; // 0 = init, 1 = width, 2 = height, 3 = complete, 4 = fail

    while (pageSize[idx] != 0x00)
    {
        if (state == 0)
        {
            if (pageSize[idx] == 'X')
            {
                state = 1;

                idx++;
                continue;
            }
        }
        else if (state == 1)
        {
            if ((pageSize[idx] >= '0') && (pageSize[idx] <= '9'))
            {
                width[widthIdx++] = pageSize[idx];

                idx++;
                continue;
            }
            else if (pageSize[idx] == 'D')
            {
                width[widthIdx++] = '.';

                idx++;
                continue;
            }
            else if (pageSize[idx] == 'M')
            {
                idx++;
                continue;
            }
            else if (pageSize[idx] == 'Y')
            {
                state = 2;

                idx++;
                continue;
            }
        }
        else if (state == 2)
        {
            if ((pageSize[idx] >= '0') && (pageSize[idx] <= '9'))
            {
                height[heightIdx++] = pageSize[idx];

                idx++;
                continue;
            }
            else if (pageSize[idx] == 'D')
            {
                height[heightIdx++] = '.';

                idx++;
                continue;
            }
            else if (pageSize[idx] == 'M')
            {
                state = 3;
                break;
            }
        }

        state = 4;
        break;
    }

    if (state == 3)
    {
        settings->pageWidth = atof(width);
        settings->pageHeight = atof(height);
    }
    else
    {
        settings->pageWidth = 0;
        settings->pageHeight = 0;
    }
}

BOOL
SetDefaultSettings(PSETTINGS pSettings,
    BOOL        bMetric 
    )
{
    pSettings->TargetPrinter    = 0;
    
    fprintf(stderr, "SETDEFAULTSETTINGS CATULTRA MODEL! %u\n",pSettings->Printer);
    //set a default..
    pSettings->OEM     = OEM_ENDURO;
	if (pSettings->Printer)
	{
		switch (pSettings->Printer)
		{
			case PRO360:
			case PRO_SELECT_360:
			case IDM_SECURE_360:
			case ALPHACARD_PRO_700_360:
			case AUTHENTYS_PRO_360:
			case PHILOS_DTP_MK_3_360:
			case PLASCO_RIOPRO_LE_360:
			case SCREENCHECK_SC6500_360:
			case TITAN_T5_360:
			case PPC_ID3350E_360:		
			case PRIDENTO_PRO_360:
				{
				pSettings->OEM     = OEM_PRO360;

				pSettings->nPrintSpeed = UICBVAL_DefaultSpeed;
				pSettings->PaperHeight = 857;
				pSettings->nSharpness  = 0;
				
				break;
				}

			case PRO360_XTD:
			case PRIDENTO_PRO_360_XTD:
				{
				pSettings->OEM     = OEM_PRO360;
				pSettings->nPrintSpeed = UICBVAL_DefaultSpeed;

				//pSettings->nColourCorrection = 3;
				if ( pSettings->pageType == 2) //extd colour card
				{
					pSettings->PaperHeight = 1080;
					pSettings->XXL_ImageType = UICBVAL_DoubleImage;
				}
				else	
				if ( pSettings->pageType == 3) //extd mono card
				{
					pSettings->PaperHeight = 1400;		
					pSettings->XXL_ImageType = UICBVAL_Extended_Monochrome;
				}
				else	
				if ( pSettings->pageType == 4) //extd mono card
				{
					pSettings->PaperHeight = 1090;		
					pSettings->XXL_ImageType = UICBVAL_Extended_Monochrome;
				}				
				else	
				if ( pSettings->pageType == 5) //extd mono card
				{
					pSettings->PaperHeight = 1280;		
					pSettings->XXL_ImageType = UICBVAL_Extended_Monochrome;
				}				
				else
				{
					pSettings->XXL_ImageType		 = UICBVAL_SingleImage;	//riopro xtd only
					pSettings->PaperHeight			 = 860;
				}
				break;
				}

			case PRICECARD_PRO_FLEX_360:
				{
				pSettings->OEM     = OEM_PRO360;
				pSettings->nPrintSpeed = UICBVAL_DefaultSpeed;
				if ( pSettings->pageType == 2) //extd mono card
				{
					pSettings->PaperHeight = 1400;		
					pSettings->XXL_ImageType = UICBVAL_Extended_Monochrome;
				}
				else	
				if ( pSettings->pageType == 3) //extd mono card
				{
					pSettings->PaperHeight = 1090;		
					pSettings->XXL_ImageType = UICBVAL_Extended_Monochrome;
				}				
				else	
				if ( pSettings->pageType == 4) //extd mono card
				{
					pSettings->PaperHeight = 1280;		
					pSettings->XXL_ImageType = UICBVAL_Extended_Monochrome;
				}				
				else
				{
					pSettings->XXL_ImageType		 = UICBVAL_SingleImage;	//riopro xtd only
					pSettings->PaperHeight			 = 860;
				}
				break;
				}
			case HELIX:
			case PPC_RTP7500W:
			case AUTHENTYS_RETRAX:
			case SCREENCHECK_SC7500:
			case HELIX_RT:
				{
				pSettings->OEM     = OEM_HELIX;
 
				pSettings->nPrintSpeed = UICBVAL_DefaultSpeed;
				pSettings->nColourCorrection = 3;
				pSettings->PaperHeight			 = 877;
				
				break;
				}
            
			case PRO:
			case IDM_SECURE:
			case AUTHENTYS_PRO:
			case PPC_ID3100:
			case PPC_ID3300:
			case RIO_PRO_XTENDED:
			case IDENTITYPRO:
			case PRIDENTOPRO:
			case PRIDENTOPROXTD:	
			case DOPPIE_300:
			case SCREENCHECK_SC6500:
			case XR360:
			case MATICA_M500:
			case PLASCO_RIOPRO_LE:
			case ALPHACARD_PRO_700:
			case JKE160C:	          
			case PHILOS_DTP_MK_3:
			case TITAN_T5:
				{
				fprintf(stderr, "L676:CATULTRA PRO settings->OEM  %u\n",pSettings->OEM );
				pSettings->OEM     = OEM_ENDURO;
				pSettings->nPrintSpeed = UICBVAL_DefaultSpeed;
				pSettings->nColourCorrection = 3;
				if ( pSettings->pageType == 2) //extd colour card
				{
					pSettings->PaperHeight = 1080;
					pSettings->XXL_ImageType = UICBVAL_DoubleImage;
				}
				else	
				if ( pSettings->pageType == 3) //extd mono card
				{
					pSettings->PaperHeight = 1400;		
					pSettings->XXL_ImageType = UICBVAL_Extended_Monochrome;
				}
				else	
				if ( pSettings->pageType == 4) //extd mono card
				{
					pSettings->PaperHeight = 1090;		
					pSettings->XXL_ImageType = UICBVAL_Extended_Monochrome;
				}				
				else	
				if ( pSettings->pageType == 5) //extd mono card
				{
					pSettings->PaperHeight = 1280;		
					pSettings->XXL_ImageType = UICBVAL_Extended_Monochrome;
				}				
				else
				{
					pSettings->XXL_ImageType		 = UICBVAL_SingleImage;	//riopro xtd only
					pSettings->PaperHeight			 = 860;
				}
				break;
				}
			case PRICECARD_PRO_FLEX:
				{
				pSettings->OEM     = OEM_ENDURO;
				pSettings->nPrintSpeed = UICBVAL_DefaultSpeed;
				pSettings->nColourCorrection = 3;

				if ( pSettings->pageType == 1) //extd mono card
				{
					pSettings->PaperHeight = 1400;		
					pSettings->XXL_ImageType = UICBVAL_Extended_Monochrome;
				}
				else	
				if ( pSettings->pageType == 3) //extd mono card
				{
					pSettings->PaperHeight = 1090;		
					pSettings->XXL_ImageType = UICBVAL_Extended_Monochrome;
				}				
				else	
				if ( pSettings->pageType == 4) //extd mono card
				{
					pSettings->PaperHeight = 1280;		
					pSettings->XXL_ImageType = UICBVAL_Extended_Monochrome;
				}				
				else
				{
					pSettings->XXL_ImageType		 = UICBVAL_SingleImage;	//riopro xtd only
					pSettings->PaperHeight			 = 860;
				}
				break;
				}
			case ENDURO:
			case MC200:
			case IDMAKER:
			case IDM_ADVANTAGE:
			case AUTHENTYS:

			case P4500S:
			case PPC_ID2100:
			case PPC_ID2300:
			case FAGOO_P310E:	
				{
				pSettings->OEM     = OEM_ENDURO;
				pSettings->nPrintSpeed = UICBVAL_DefaultSpeed;
				pSettings->nColourCorrection = (pSettings->Printer == PPC_ID2300 || pSettings->Printer == FAGOO_P310E) ? 3 : 1;
				break;
				}
			case RIO_TANGO:
			case XXL:				
				{
				pSettings->OEM     = OEM_RIO;


				pSettings->TargetPrinter    = 1;
				pSettings->nColourCorrection = 1;
				pSettings->nRollerTemperature   = 140;
				pSettings->nCardSpeed           = 4;
				pSettings->nPreLaminationDelay  = 500;
				pSettings->nLaminationLength    = 86;
				pSettings->nStartOffset         = 0;
				pSettings->nEndOffset           = 16;
				pSettings->nFilmType            = 0;
				pSettings->XXL_ImageType		 = UICBVAL_SingleImage;
				
				break;
				}     
			case PRONTO:
			case IDM_VALUE:	
			case P2500S:
			case PPC_ID2000:	
			case PRIDENTO:	
			case DOPPIE_100:
			case SCREENCHECK_SC2500:
			case XR160:
			case AUTHENTYS_PRONTO:
			case JKE700C:
			case PRIDENTO_OPEN_DATA:
			case ALPHACARD_PILOT:
			case PRICECARD_PRO_LITE:
			case PHILOS_DTP_MK_1:
				{
				pSettings->OEM     = OEM_ENDURO;


				pSettings->nPrintSpeed = UICBVAL_DefaultSpeed;
				pSettings->nColourCorrection = 1;
				break;
				}       
			case ENDURO_PLUS:
			case AUTHENTYS_PLUS:	
			case P4500S_PLUS:
			case IDM_ADVANT_PLUS:	
			case SECUPRINT_200:
			case PRIDENTO_PLUS:
			case ING171:
			case DOH:		
			case ENDURO_3E:	
			case PPC_ID2350E:	
			case ILINKCARD_IT2600:	
			case DOPPIE_200:
			case PCARD:
			case ORPHICARD:
			case SCREENCHECK_SC4500:
			case XR260:
			case MATICA_M300:
			case PLASCO_ENDURO_LE:
			case ALPHACARD_PRO_500:
			case ALPHACARD_COMPASS:
			case JKE701C:
			case PRICECARD_PRO_DUO:
			case PHILOS_DTP_MK_2:
			case TITAN_T3:
			case ELLIADEN_CAROLINE:
				{
				pSettings->OEM     = OEM_ENDURO;

				pSettings->nPrintSpeed = UICBVAL_DefaultSpeed;
				pSettings->nColourCorrection = 3;
				break;
				}
			case PRIDENTO_PLUS_OPEN_DATA:
				{
				pSettings->OEM     = OEM_ENDURO;

				pSettings->nPrintSpeed = UICBVAL_HighQuality;
				pSettings->nColourCorrection = 1;
				break;
				}
		}
	}
	else
	{	
		fprintf(stderr, "L811:CATULTRA Enduro settings->OEM  %u\n",pSettings->OEM );
		pSettings->Printer = ENDURO;
		pSettings->OEM     = OEM_ENDURO;

		pSettings->nPrintSpeed = UICBVAL_DefaultSpeed;
		pSettings->nColourCorrection = 1;
	}		
    pSettings->nPrintHeadPosition  = 50;   //to be populated by status monitor feedback
    if (pSettings->Printer == P4500S_PLUS || pSettings->Printer == P2500S)
        pSettings->CB_SecurityOptions_SecurityType = pSettings->CF_SecurityOptions_SecurityType = 4;

	if (pSettings->Printer == IDENTITYPRO)
		pSettings->CB_SecurityOptions_SecurityType = pSettings->CF_SecurityOptions_SecurityType = 1;
			
    pSettings->CF_ColourFormat = UICBVAL_YMCK;     //default card front to YMCK
    pSettings->CB_ColourFormat = UICBVAL_KResin;   //default card back to K
    pSettings->CF_PrintOvercoat = TRUE;
    pSettings->CF_HoloKote      = TRUE;
    pSettings->CF_SecurityOptions_ColourHole = 1;         //default on
    pSettings->CF_SecurityOptions_HoloKotePatch = 0x20;    //default to Area 6   
    pSettings->CB_SecurityOptions_HoloKotePatch = 0x20;   
    
    pSettings->CF_SecurityOptions_HoloKoteMap = 0xffffff;
    pSettings->CB_SecurityOptions_HoloKoteMap = 0xffffff;
	
	//following polaroid model requested to default to YMC 
	if (pSettings->Printer == P4500S_PLUS)
	{
		pSettings->CF_BlackOptions_AllBlackAs = 1;
		pSettings->CB_BlackOptions_AllBlackAs = 1;
	}
	
	if (IDVILLE_TYPE(pSettings))
	{
		pSettings->CF_BlackOptions_PicturesUseYMConly      = 0;
		pSettings->CB_BlackOptions_PicturesUseYMConly      = 0;
	}
	else
	{
		pSettings->CF_BlackOptions_PicturesUseYMConly      = 1;
		pSettings->CB_BlackOptions_PicturesUseYMConly      = 1;
	}
    pSettings->CF_BlackOptions_BlackTextUsesKResin     = 1;
    pSettings->CF_BlackOptions_MonoBitmapsUseKResin    = 1;
    pSettings->CF_BlackOptions_BlackPolygonsUseKResin  = 1;


    pSettings->CB_BlackOptions_BlackTextUsesKResin     = 1;
    pSettings->CB_BlackOptions_MonoBitmapsUseKResin    = 1;
    pSettings->CB_BlackOptions_BlackPolygonsUseKResin  = 1;
    
    pSettings->AppDeterminesOrient = TRUE;
    pSettings->CF_CardOrient = pSettings->CB_CardOrient = 2;//DMORIENT_LANDSCAPE;
    pSettings->bPauseSpooler = TRUE;
    
    pSettings->nPrintHeadPower_YMC        = 50;
    pSettings->nPrintHeadPower_BlackResin = 50;
    pSettings->nPrintHeadPower_Overcoat   = 50;

    pSettings->ErasePower_Start     = 50;
    pSettings->ErasePower_End       = 50;
    pSettings->WritePowerAdjustment = 50;
    
    pSettings->EraseArea_Left = 0;       //model dependant?
    pSettings->EraseArea_Width = 1016;
    pSettings->EraseArea_Bottom = 0;
    pSettings->EraseArea_Height = 642;
    
    pSettings->nBitsPerChar_Track2 = 1;
    pSettings->nBitsPerChar_Track3 = 1;
    pSettings->nBitsPerInch_Track2 = 1;
    
    return TRUE;
}

inline void initializeSettings(char * commandLineOptionSettings, struct settings_ * settings, PDEVDATA pdev)
{
    ppd_file_t *    ppd         = NULL;
    cups_option_t * options     = NULL;
	char *          pOrientReq  = 0;
	char *          pCmdLineOptionTrack1  = 0;
	char *          pCmdLineOptionTrack2  = 0;
	char *          pCmdLineOptionTrack3  = 0;
	char *          pCmdLineOptionEnd  = 0;
    int             numOptions  = 0;
    //int             modelNumber = 0;
	char buff[256];
	
    ppd = PPDOPENFILE(getenv("PPD"));
fprintf(stderr, "LCATULTRA cmdline! %s\n",commandLineOptionSettings);
    PPDMARKDEFAULTS(ppd);

    numOptions = CUPSPARSEOPTIONS(commandLineOptionSettings, 0, &options);
    if ((numOptions != 0) && (options != NULL))
    {
        CUPSMARKOPTIONS(ppd, numOptions, options);

        CUPSFREEOPTIONS(numOptions, options);
    }
    memset(settings, 0x00, sizeof(struct settings_));
	//lpr -P<printername> -o"TRACK1:~1,%123123123?" -o"TRACK2:~2,2323232323?" -o"TRACK3:~3,;2323232323#"
	//lpr -P<printername> -o"TRACK1:~1,BPI75,MPC5,%123123123?" -o"TRACK2:~2,2323232323?" -o"TRACK3:~3,;2323232323#"

	InitializeMagneticEncodeTrackInfo(pdev, settings);

	pCmdLineOptionTrack1 = strstr(commandLineOptionSettings, "TRACK1");
	if (pCmdLineOptionTrack1)
	{
		pCmdLineOptionTrack1 += 7;
		pCmdLineOptionEnd = strstr(pCmdLineOptionTrack1, " ");
		if (pCmdLineOptionEnd)
		{
		    memset(buff, 0x00, sizeof(buff));
			memcpy((PBYTE)&buff, pCmdLineOptionTrack1, (int)(pCmdLineOptionEnd - pCmdLineOptionTrack1));
fprintf(stderr, "Passing to record %s\n",buff);
			RecordMagneticEncodeTrackInfo(pdev,buff,settings);
fprintf(stderr, "FINISHED RECORD MAG TRACK1\n");
		}
	}
	
	pCmdLineOptionTrack2 = strstr(commandLineOptionSettings, "TRACK2");
	if (pCmdLineOptionTrack2)
	{
		pCmdLineOptionTrack2 += 7;
		pCmdLineOptionEnd = strstr(pCmdLineOptionTrack2, " ");
		if (pCmdLineOptionEnd)
		{
			memset(buff, 0x00, sizeof(buff));
			memcpy((PBYTE)&buff, pCmdLineOptionTrack2, (int)(pCmdLineOptionEnd - pCmdLineOptionTrack2));
fprintf(stderr, "Passing to record %s\n",buff);
			RecordMagneticEncodeTrackInfo(pdev,buff,settings);
fprintf(stderr, "FINISHED RECORD MAG TRACK2\n");
		}
	}

	pCmdLineOptionTrack3 = strstr(commandLineOptionSettings, "TRACK3");
	if (pCmdLineOptionTrack3)
	{
		pCmdLineOptionTrack3 += 7;
		pCmdLineOptionEnd = strstr(pCmdLineOptionTrack3, " ");
		if (pCmdLineOptionEnd)
		{
		    memset(buff, 0x00, sizeof(buff));
			memcpy((PBYTE)&buff, pCmdLineOptionTrack3, (int)(pCmdLineOptionEnd - pCmdLineOptionTrack3));
fprintf(stderr, "Passing to record %s\n",buff);
			RecordMagneticEncodeTrackInfo(pdev,buff,settings);
fprintf(stderr, "FINISHED RECORD MAG TRACK3\n");
		}
	}
	
	pOrientReq = strstr(commandLineOptionSettings, "orientation-requested");
	if (pOrientReq)
	{
fprintf(stderr, "LCATULTRA ORIENTATION REQUEST..\n");		
			if (pOrientReq[22] == '4')
			{
				fprintf(stderr, "LCATULTRA SETTING LANDSCAPE.. \n");	
				settings->orientation = 1;
			}
	}
	settings->orientation = 1;

	//retrieve printer model
    settings->Printer = ppd->model_number;
fprintf(stderr, "L973:CATULTRA MODEL! %u\n",settings->Printer);
    if (settings->Printer >= HELIX)
    {
        settings->bytesPerScanLine = 3108;	//1036 * 3
        settings->bytesPerScanLineStd = 3108;
    }
    else
    {
        settings->bytesPerScanLine = 1926;	//642 * 3
        settings->bytesPerScanLineStd = 1926;
	}
    
	//selected page size
    settings->pageType                       = getOptionChoiceIndex("PageSize"                      , ppd);
fprintf(stderr, "L987:CATULTRA settings->OEM  %u\n",settings->OEM );
	SetDefaultSettings(settings, FALSE);
fprintf(stderr, "L989:CATULTRA settings->OEM  %u\n",settings->OEM );
    if (settings->OEM == OEM_HELIX)
	{
//         settings->OEM = OEM_HELIX;
		 settings->ISO7810					 = getOptionChoiceIndex("ISO7810"			, ppd);
		 settings->CFHolokoteSlot			 = getOptionChoiceIndex("CFHolokoteSlot"			, ppd);
		 settings->CBHolokoteSlot			 = getOptionChoiceIndex("CBHolokoteSlot"			, ppd);
		 settings->nHLXPrintHeadPowerYMC	 = getOptionChoiceIndex("PrintheadPowerYMC"			, ppd);
		 settings->nHLXPrintHeadPowerK		 = getOptionChoiceIndex("PrintheadPowerK"			, ppd);
		 settings->nHLXPrintHeadPowerOvercoat = getOptionChoiceIndex("PrintheadPowerOvercoat"			, ppd);
	}
	else
	if (settings->OEM == OEM_PRO360)
	{
		settings->CFHolokoteSlot			 = getOptionChoiceIndex("CFHolokoteSlot"			, ppd) + 1;
		settings->CBHolokoteSlot			 = getOptionChoiceIndex("CBHolokoteSlot"			, ppd) + 1;
		settings->CF_SecurityOptions_HoloKote_XAdjust	 = getOptionChoiceIndex("CFHolokoteXAdjust"			, ppd) - 25;
		settings->CF_SecurityOptions_HoloKote_YAdjust	 = getOptionChoiceIndex("CFHolokoteYAdjust"			, ppd) - 25;
		settings->CB_SecurityOptions_HoloKote_XAdjust	 = getOptionChoiceIndex("CBHolokoteXAdjust"			, ppd) - 25;
		settings->CB_SecurityOptions_HoloKote_YAdjust	 = getOptionChoiceIndex("CBHolokoteYAdjust"			, ppd) - 25;
		settings->CF_SecurityOptions_HoloPatchOffset_Left =   808;  //hard code to patch 6
		settings->CF_SecurityOptions_HoloPatchOffset_Width =  175;
		settings->CF_SecurityOptions_HoloPatchOffset_Bottom = 447;
		settings->CF_SecurityOptions_HoloPatchOffset_Height = 175;
		settings->CF_SecurityOptions_UsewithLaminate     = getOptionChoiceIndex("CFUsewithLaminate"			, ppd);
		settings->CB_SecurityOptions_UsewithLaminate     = getOptionChoiceIndex("CBUsewithLaminate"			, ppd);
	
		fprintf(stderr, "L975:CATULTRA MODEL - 360! %u\n",settings->Printer);	
		debugPrintSettings(settings);
	}		 

    settings->Duplex							 = getOptionChoiceIndex("UTDuplex"                  , ppd);
	
    settings->CF_ColourFormat					 = getOptionChoiceIndex("CFColourFormat"			, ppd);

    settings->CF_BlackOptions_AllBlackAs		 = getOptionChoiceIndex("CFBlackResin"  			, ppd);	
    settings->CF_PrintOvercoat					 = getOptionChoiceIndex("CFOverCoat"				, ppd);
    settings->CF_HoloKote						 = getOptionChoiceIndex("CFHoloKote"				, ppd);
    settings->CF_HoloPatch						 = getOptionChoiceIndex("CFHoloPatch"				, ppd);
    if (settings->CF_HoloPatch && (settings->Printer == ENDURO || settings->Printer == ENDURO_PLUS))
		settings->CF_SecurityOptions_ColourHole = 1;

    settings->CF_CardOrient						 = getOptionChoiceIndex("Orientation"				, ppd);
    settings->CF_Rotate180						 = getOptionChoiceIndex("CFRotation"				, ppd);
    settings->CF_OvercoatOptions_Holes			 = getOptionChoiceIndex("CFOvercoatHole"			, ppd);	
    settings->CF_SecurityOptions_SecurityType	 = getOptionChoiceIndex("CFHolokoteType"			, ppd);	




    settings->CF_SecurityOptions_Rotation		 = getOptionChoiceIndex("CFHolokoteRotation"		, ppd);		
	//
    settings->CB_ColourFormat					 = getOptionChoiceIndex("CBColourFormat"			, ppd);
    settings->CB_BlackOptions_AllBlackAs		 = getOptionChoiceIndex("CBBlackResin"  			, ppd);		
    settings->CB_PrintOvercoat					 = getOptionChoiceIndex("CBOverCoat"				, ppd);
    settings->CB_HoloKote						 = getOptionChoiceIndex("CBHoloKote"				, ppd);
    settings->CB_HoloPatch						 = getOptionChoiceIndex("CBHoloPatch"				, ppd);
    settings->CB_CardOrient						 = getOptionChoiceIndex("Orientation"				, ppd);
    settings->CB_Rotate180						 = getOptionChoiceIndex("CBRotation"				, ppd);

    settings->CB_OvercoatOptions_Holes			 = getOptionChoiceIndex("CBOvercoatHole"			, ppd);
    settings->CB_SecurityOptions_SecurityType	 = getOptionChoiceIndex("CBHolokoteType"			, ppd);	
    settings->CB_SecurityOptions_Rotation		 = getOptionChoiceIndex("CBHolokoteRotation"		, ppd);	
	//
	settings->nColourCorrection					 = getOptionChoiceIndex("ColourCorrection"			, ppd);

    settings->nPrintSpeed						 = getOptionChoiceIndex("ResinQuality"				, ppd);
    settings->bColourSure						 = getOptionChoiceIndex("ColourSure"				, ppd);
    settings->nSharpness						 = -2 + getOptionChoiceIndex("Sharpness"					, ppd);
    settings->bDisableStatusPolling				 = getOptionChoiceIndex("StatusPolling"				, ppd);

    settings->nPrintHeadPower_YMC				 = getOptionChoiceIndex("PrintheadPowerYMC"			, ppd);

    settings->nPrintHeadPower_BlackResin		 = getOptionChoiceIndex("PrintheadPowerK"			, ppd);
    settings->nPrintHeadPower_Overcoat			 = getOptionChoiceIndex("PrintheadPowerOvercoat"	, ppd);
    settings->nImagePosition_UpDown				 = -15 + getOptionChoiceIndex("PrintHeadPos"				, ppd);
    settings->nImagePosition_Start				 = -50 + getOptionChoiceIndex("ImagePosStart"				, ppd);
    settings->nImagePosition_End				 = -50 + getOptionChoiceIndex("ImagePosEnd"				, ppd);
    settings->bEraseBeforePrint					 = getOptionChoiceIndex("RewritablecardsEraseBeforePrint"	, ppd);
    if (settings->bEraseBeforePrint < 0)
	settings->bEraseBeforePrint = FALSE;
    settings->ErasePower_Start					 = getOptionChoiceIndex("ErasePowerStart"				, ppd);
    settings->ErasePower_End					 = getOptionChoiceIndex("ErasePowerEnd"				, ppd);
    settings->WritePowerAdjustment				 = getOptionChoiceIndex("WritePower"				, ppd);


    getPageWidthPageHeight(ppd, settings);

    PPDCLOSE(ppd);
fprintf(stderr, "L975:CATULTRA MODEL! %u\n",settings->Printer);	
    debugPrintSettings(settings);
}

void jobSetup(struct settings_ settings, char *argv[])
{
	//cups_page_header_t  header;                             // CUPS Page header  
	


}


/*****************************************************************************
 *  OutputComma()
 *      Send command for comma
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID OutputComma
(
    PDEVDATA pdev // Pointer to our PDEV
)
{
    // Send command for comma
    printf(CMD_STR_COMMA);
}

/*****************************************************************************
 *  OutputFileSeparator()
 *      Send command for file separator
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID OutputFileSeparator
(
    PDEVDATA pdev // Pointer to our PDEV
)
{
    // Send command for file separator
    printf(CMD_STR_FILE_SEPARATOR);
}

/*****************************************************************************
 *  OutputETXCommand()
 *      Send command for End of Transmission
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID OutputETXCommand
(
    PDEVDATA pdev // Pointer to our PDEV
)
{
    // Send command for end-of-page
    printf(CMD_STR_ETX);
}


#ifdef BUMPVERSION
extern WORD ICCValue;
#endif

//##############################################################################

/*****************************************************************************
 *  AreaHoleActive()
 *      Determines if the given Area/Hole is active by checking if both the
 *      height and width are non-zero
 *
 *  Returns:
 *      TRUE  if both Height and Width are non-zero for the given Area/Hole
 *      FALSE otherwise
 *****************************************************************************/

BOOL AreaHoleActive(PDEVDATA pdev, struct settings_ * settings, AREA_HOLE_ID AreaHoleIdentifier)
{
    //PRIVATEDEVMODE *pdmPrivate = (PRIVATEDEVMODE *)GetPrivateDevMode((PDEVMODE)&pdev->dm.dmPublic);
    BOOL result = FALSE;

	switch (AreaHoleIdentifier)
	{
	case FRONT_AREA1:
        if ((settings->CF_FrontArea1_Width  != 0)  
        &&  (settings->CF_FrontArea1_Height != 0))
		{
			result = TRUE;
		}
		break;

	case FRONT_AREA2:
        if ((settings->CF_FrontArea2_Width  != 0)  
        &&  (settings->CF_FrontArea2_Height != 0))
		{
			result = TRUE;
		}
		break;

	case FRONT_HOLE1:
        if ((settings->CF_FrontHole1_Width  != 0)  
        &&  (settings->CF_FrontHole1_Height != 0))
		{
			result = TRUE;
		}
		break;

	case FRONT_HOLE2:
        if ((settings->CF_FrontHole2_Width  != 0)  
        &&  (settings->CF_FrontHole2_Height != 0))
		{
			result = TRUE;
		}
		break;

	case BACK_AREA1:
        if ((settings->CB_BackArea1_Width  != 0)  
        &&  (settings->CB_BackArea1_Height != 0))
		{
			result = TRUE;
		}
		break;

	case BACK_AREA2:
        if ((settings->CB_BackArea2_Width  != 0)  
        &&  (settings->CB_BackArea2_Height != 0))
		{
			result = TRUE;
		}
		break;

	case BACK_HOLE1:
        if ((settings->CB_BackHole1_Width  != 0)  
        &&  (settings->CB_BackHole1_Height != 0))
		{
			result = TRUE;
		}
		break;

	case BACK_HOLE2:
        if ((settings->CB_BackHole2_Width  != 0)  
        &&  (settings->CB_BackHole2_Height != 0))
		{
			result = TRUE;
		}
		break;
	}

	return result;
}

/*****************************************************************************
 *  WriteBitsPerChar()
 *      Outputs the number of Bits per char if not default (i.e. ISO)
 *      Alto/Opera/Tempo are always ISO
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID WriteBitsPerChar
(
    PDEVDATA pdev, 
	struct 	 settings_ * settings,
	ULONG Track_no
)
{
    
    switch (Track_no)
	{
	case MAG_TRACK_INDEX1:
        switch (settings->nBitsPerChar_Track1)
		{
        //Default (ISO standard) is 7 bits - no header entry required
        case 1://UICBVAL_BitsPerChar_Track1_5:
            printf( CMD_STR_BITS_PER_CHAR, BITS_PER_CHAR_5);
			break;
        case 2://UICBVAL_BitsPerChar_Track1_1:
            printf(CMD_STR_BITS_PER_CHAR,
								BITS_PER_CHAR_1);
			break;
		}
		break;

	case MAG_TRACK_INDEX2:
        switch (settings->nBitsPerChar_Track2)
		{
        //Default (ISO standard) is 5 bits - no header entry required
        case 0://UICBVAL_BitsPerChar_Track2_7:
            printf(CMD_STR_BITS_PER_CHAR,
								BITS_PER_CHAR_7);
			break;
        case 2://UICBVAL_BitsPerChar_Track2_1:
            printf(CMD_STR_BITS_PER_CHAR,
								BITS_PER_CHAR_1);
			break;
		}
		break;

	case MAG_TRACK_INDEX3:
        switch (settings->nBitsPerChar_Track3)
		{
        //Default (ISO standard) is 5 bits - no header entry required
        case 0://UICBVAL_BitsPerChar_Track3_7:
            printf(CMD_STR_BITS_PER_CHAR,
								BITS_PER_CHAR_7);
			break;
        case 2://UICBVAL_BitsPerChar_Track3_1:
            printf(CMD_STR_BITS_PER_CHAR,
								BITS_PER_CHAR_1);
			break;
		}
		break;
	}
}

/*****************************************************************************
 *  WriteBitsPerInch()
 *      Outputs the number of Bits per Inch if not default (i.e. ISO)
 *      Alto/Opera/Tempo are always ISO
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID WriteBitsPerInch
(
    PDEVDATA pdev, 
	struct 	 settings_ * settings,
	ULONG Track_no
)
{

	switch (Track_no)
	{
	case MAG_TRACK_INDEX1:
        //Default (ISO standard) is 210 bits per inch.
        if (settings->nBitsPerInch_Track1 == 1/*UICBVAL_BitsPerInch_Track1_75*/)
		{
            printf(CMD_STR_BITS_PER_INCH,
								BITS_PER_INCH_75);
		}
		break;

	case MAG_TRACK_INDEX2:
        //Default (ISO standard) is 75 bits per inch.
        if (settings->nBitsPerInch_Track2 == 0/*UICBVAL_BitsPerInch_Track2_210*/)
		{
            printf(CMD_STR_BITS_PER_INCH,
                                BITS_PER_INCH_210);
		}
		break;

	case MAG_TRACK_INDEX3:
        //Default (ISO standard) is 210 bits per inch.
        if (settings->nBitsPerInch_Track3 == 1/*UICBVAL_BitsPerInch_Track3_75*/)
		{
            printf(CMD_STR_BITS_PER_INCH, BITS_PER_INCH_75);
		}
		break;
	}
}

/*****************************************************************************
 *  GetDataPresent()
 *      Check if we have seen any colour in any plane.
 *
 *  Returns:
 *      Return a mask indicating whether data is in a plane
 *            Bit 0 = Yellow
 *            Bit 1 = Magenta
 *            Bit 2 = Cyan
 *            Bit 3 = Black
 *****************************************************************************/

BYTE GetDataPresent
(
    PDEVDATA pdev, // Pointer to our PDEV
    BOOL     bFront // TRUE = Front page (or simplex) : FALSE = Back page
)
{
    LPSPOOLMEMINFO lpSplMem        = 0;
    ULONG          ulColour        = 0;
	BYTE           ColourMask      = 0;

	for (ulColour = 0 ; ulColour < 4; ulColour ++)
    {
        if (bFront)
            lpSplMem = &pdev->lpSplMemFront[ulColour];
        else
            lpSplMem = &pdev->lpSplMemBack[ulColour];

		if ((lpSplMem->lpBuffer != 0) 
		&&   lpSplMem->bDataInPlane
		&&  (lpSplMem->ulDataSize != 0))
 		{
			//Set mask bit to show there is data in this plane
			ColourMask |= (1 << ulColour);
		}
	}
    return ColourMask;
}

/*****************************************************************************
 *  LaminatorCommand()
 *      Output laminator commands.
 *      includes the appropriate laminator command in the header.
 *      Dependent on the lamination settings and the print settings (front, 
 *      back or both sides)
 *
 *  Returns:
 *      None
 *****************************************************************************/

void LaminatorCommand
(
    PDEVDATA pdev, // Pointer to our PDEV
	struct 	 settings_ * settings
)
{
//PRIVATEDEVMODE *pdmPrivate = (PRIVATEDEVMODE *)GetPrivateDevMode((PDEVMODE)&pdev->dm.dmPublic);
   //LPDOCSTICKY lpds = GetUICBDocumentProperties(pPDev->lpdm);
	int LamSetting   = 0;
	int EjectSetting = 1;
	int FilmType     = OVERLAY_FILM;

    if (settings->bLaminateOnly)
	{
        switch (settings->nLaminateSide)
		{
        case UICBVAL_Laminating_None:  LamSetting = 0; EjectSetting = 1; break;
        case UICBVAL_Laminating_Front: LamSetting = 1; EjectSetting = 1; break;
        case UICBVAL_Laminating_Back:  LamSetting = 2; EjectSetting = 2; break;
        case UICBVAL_Laminating_Both:  LamSetting = 3; EjectSetting = 2; break;
		}
	}
	
	else
	{
        if ((settings->Duplex == UICBVAL_Duplex_BothSides)
        &&  (GetDataPresent(pdev, FALSE)))
		{
			//Duplex with data on both sides
            switch (settings->nLaminateSide)
		    {
			case UICBVAL_Laminating_None:  LamSetting = 8;  EjectSetting = 2; break;
			case UICBVAL_Laminating_Front: LamSetting = 9;  EjectSetting = 1; break;
			case UICBVAL_Laminating_Back:  LamSetting = 10; EjectSetting = 2; break;
			case UICBVAL_Laminating_Both:  LamSetting = 11; EjectSetting = 1; break;
			}
		}
		else
		{
			//Duplex_Front, Duplex_Back, or Duplex_BothSides with data on front only
            switch (settings->nLaminateSide)
		    {
			case UICBVAL_Laminating_None:  LamSetting = 4; EjectSetting = 1; break;
			case UICBVAL_Laminating_Front: LamSetting = 5; EjectSetting = 1; break;
			case UICBVAL_Laminating_Back:  LamSetting = 6; EjectSetting = 2; break;
			case UICBVAL_Laminating_Both:  LamSetting = 7; EjectSetting = 2; break;
			}
		}
	}	

	//Eject Side
    printf(CMD_STR_EJECTSIDE, EjectSetting);
	
	//LAM Command
    printf(CMD_STR_LAMINATOR_COMMAND, LamSetting);

	//Roller Temperature
    printf(CMD_STR_ROLLER_TEMP, settings->nRollerTemperature);
	
	//Card Speed
    printf(CMD_STR_CARD_SPEED, settings->nCardSpeed);

	//Pre-Lamination Delay
    printf(CMD_STR_PRELAM_DELAY, settings->nPreLaminationDelay);

	//Card Length
    printf(CMD_STR_CARD_LENGTH, settings->nLaminationLength);

	//Start Offset
    printf(CMD_STR_START_OFFSET, settings->nStartOffset);
	
	//End Offset
    printf(CMD_STR_END_OFFSET, settings->nEndOffset);

	//Film Type
    switch (settings->nFilmType)
	{
	case UICBVAL_LamFilm_Overlay: FilmType = OVERLAY_FILM; break;
	case UICBVAL_LamFilm_Patch:   FilmType = PATCH_FILM;   break;
	}
    printf(CMD_STR_FILM_TYPE, FilmType);
}

/*****************************************************************************
 *
 * GetFirmwareVersion()
 *     Reads the firmware version from the registry (Enduro OEM Only)
 *
 *  Returns:
 *      Pointer to FirmwareVersion String
 *
 *****************************************************************************/
/*
BOOL GetFirmwareVersion
(
    LPPCLDEVMODE lpdm,
    HANDLE  hCurPrinter,
	LPTSTR	   lpszFirmwareVersion
)
{
    HANDLE hPrinter = INVALID_HANDLE_VALUE;
	PRINTER_DEFAULTS PrinterDefault = {NULL, NULL, PRINTER_ALL_ACCESS};
	BOOL			 bResult = FALSE;
	TCHAR            tcPrinterName[100];

    GetPrinterName(hCurPrinter, (LPTSTR)tcPrinterName);
	if (OpenPrinter(tcPrinterName, &hPrinter, NULL)) 
	{
		DWORD dwReturn;
		DWORD dwTemp;
		DWORD dwType;
		WCHAR szStoredString[50];

		dwReturn = GetPrinterData(hPrinter, 
					              TEXT("ES_FWVersion"), 
							      (LPDWORD)&dwType,
							      (LPBYTE)szStoredString,
							      sizeof(szStoredString),
								  (LPDWORD)&dwTemp);

		if ((dwReturn == ERROR_SUCCESS) 
		&&  (dwType == REG_SZ))
		{
			wcscpy(lpszFirmwareVersion, szStoredString);
			bResult = TRUE;
		}
	}
    ClosePrinter(hPrinter);
	return bResult;
}
*/
/*****************************************************************************
 *  RioProHKTValue()
 *      Determines the HKT Value to be sent to a RioPro (Type) printer
 *
 *  Returns:
 *      int value for the HKT command
 *****************************************************************************/

int RioProHKTValue
(
	struct 	 settings_ * settings,
	int UICB_Value
)
{
	//This code is dependent on the fact that the settings for the back are the
	//same as those for the front.  This is because we cannot have multiple case
	//labels with the same value
	if (IDVILLE_TYPE(settings))
	{
		switch (UICB_Value)
		{
		default:
		case 0: return 6; //IDVille Logo
		case 1: return 7; //Seal
		case 2: return 8; //Eagle
		case 3: return 9; //Globe
		case 4: return 5; //Flex
		}
	}
	else
	if (PRIDENTO_TYPE(settings))
	{
		switch (UICB_Value)
		{
		default:
		case 0: return 254; //Pridento Holokote
		case 1: return 255; //Pridento Flex
		case 2: return 2; //Rings
		case 3: return 3; //Globe
		case 4: return 4; //Waves
		}
	}
	else
	if (AUTHENTYS_TYPE(settings))
	{
		switch (UICB_Value)
		{
		default:
		case 0: return 1; //UltraSecure Key
		case 1: return 2; //Rings
		case 2: return 3; //Globe
		case 3: return 5; //Flex
		case 4: return 4; //Authentys A
		}
	}	
	else
	if (IDENTITY_PRO_TYPE(settings))
	{
		switch (UICB_Value)
		{
		default:
		case 0: return 2; //Rings
		case 1: return 3; //Globe
		case 2: return 4; //Cubes
		case 3: return 5; //Flex
		}
	}
	else
	if (ALPHACARD_TYPE(settings) )
	{	
		switch (UICB_Value)
		{	
		default:
		case 0: return 1; //Flex
		case 1:	return 2; //Rings
		case 2:	return 3; //Globe
		case 3:	return 4; //Cubes
		case 4:	return 5; //Flex
		}
	}
	else
	if (PHILOS_TYPE(settings))
	{	
		switch (UICB_Value)
		{	
		default:
		case 0: return 1; //Flex
		case 1:	return 2; //Rings
		case 2:	return 3; //Globe
		case 3:	return 4; //Wavelength
		}
	}
	else
	if (TITAN_TYPE(settings))
	{	
		switch (UICB_Value)
		{	
		default:
		case 0: return 1; //Flex
		case 1:	return 2; //Rings
		case 2:	return 3; //Globe
		case 3:	return 4; //Wavelength
		}
	}
	else
	if (ELLIADEN_TYPE(settings))
	{	
		switch (UICB_Value)
		{	
		default:
		case 0: return 1; //Flex
		case 1:	return 2; //Rings
		case 2:	return 3; //Globe
		case 3:	return 4; //Wavelength
		}
	}
	else
	{
		switch (UICB_Value)
		{
		default:
		case 0: return 1; //UltraSecure Key
		case 1: return 2; //Rings
		case 2: return 3; //Globe
		case 3: return 4; //Cubes
		case 4: return 5; //Flex
		}
	}

/*switch (UICB_Value)
	{
	default:
	case UICBVAL_HoloKote_Key_RioPro_Front:
	case UICBVAL_HoloKote_IDville_Logo_RioPro_Front:	return 1;

	case UICBVAL_HoloKote_Rings_RioPro_Front:
	case UICBVAL_HoloKote_IDville_Seal_RioPro_Front:	return 2;

	case UICBVAL_HoloKote_Globe_RioPro_Front:
	case UICBVAL_HoloKote_IDville_Eagle_RioPro_Front:	return 3;

	case UICBVAL_HoloKote_Cubes_RioPro_Front:
	case UICBVAL_HoloKote_Authentys_RioPro_Front:
	case UICBVAL_HoloKote_IDville_Globe_RioPro_Front:	return 4;

	case UICBVAL_HoloKote_Flex_RioPro_Front:
	case UICBVAL_HoloKote_IDville_Flex_RioPro_Front:	return 5;
	}*/
}

BOOL IgnoreImageData(PDEVDATA pdev, struct settings_ * settings)
{

	if (settings->bEncodeOnly || settings->bLaminateOnly)
		return TRUE;
		
return FALSE;		
}

/*****************************************************************************
 *
 * GetChannelOption()
 *     Returns the colour format to be used for the given side of the card
 *     Checks the registry for Enduro family to determine if the dye film is
 *     different to the setting of the driver interface.  The appropriate dye
 *     film setting is returned.
 *
 *  Returns:
 *      Appropriate dye film setting
 *****************************************************************************/

static WORD GetChannelOption
(
    PDEVDATA        pdev,
	struct			settings_ * settings,
	int		        iFrontBack
)
{
WORD        eOption;

    eOption = (iFrontBack == FRONT || RIO_PRO_X_MODEL(settings)) ? settings->CF_ColourFormat 
                                                : settings->CB_ColourFormat; 

////    if (ENDURO_OEM(settings))
////	{
//TODO
/*
        if ((ReadRegistry((LPPCLDEVMODE)&pdev->dm, pdev->hPrinter, REGKEY_ES_COLOUR) == 0)
        &&  (ReadRegistry((LPPCLDEVMODE)&pdev->dm, pdev->hPrinter, REGKEY_ES_RESIN)  != 0)
		&&  (eOption != UICBVAL_KResin))
		{
            ////ERROR_MSG(lpdm, TEXT("--- Forcing KResin ---\n"));
			eOption = UICBVAL_KResin;
		}
		
	}
*/  
	return eOption;
}

/*****************************************************************************
 *  OutputPlaneDataSizeCommand()
 *      Send SZX commands to device.
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID OutputPlaneDataSizeCommand
(
    PDEVDATA	pdev,         // Pointer to our PDEV
    ULONG		ulDataLen,       // The data length which is used as command parameter
    ULONG		ulColour         // The colour to sent out. PLANE_Y, PLANE_M, PLANE_C, PLANE_K
)
{
    switch (ulColour)
    {
        case PLANE_C:
            // Send command to set the length of the Cyan plane
            printf(CMD_STR_CYANSIZE, (long unsigned int)ulDataLen );
            break;

        case PLANE_M:
            // Send command to set the length of the Magenta plane
            printf(CMD_STR_MAGENTASIZE, (long unsigned int)ulDataLen );
            break;

        case PLANE_Y:
            // Send command to set the length of the Yellow plane
            printf(CMD_STR_YELLOWSIZE, (long unsigned int)ulDataLen );
            break;

        case PLANE_K:
            // Send command to set the length of the Black plane
            printf(CMD_STR_KSIZE, (long unsigned int)ulDataLen );
            break;

        default:
            // unknown colour
            break;
    }
}

/*****************************************************************************
 *  OutputHeaderCommand()
 *      Output header commands.
 *      The type of commands can be different depends on duplex setting.
 *      So this function requires which side (front or back) we are printing.
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID OutputHeaderCommand
(
    PDEVDATA pdev,     // Pointer to our PDEV
	cups_page_header2_t header,
	PSETTINGS psettings,	
    BOOL     bFrontPage // TRUE = Front page (simplex or duplex); FALSE = Back page (simplex or duplex)
)
{
    int  iCopyCount        = header.NumCopies;
    BOOL bOverCoat         = FALSE;
    BOOL bUseWithLaminate  = FALSE;
    BOOL bHoloKote         = FALSE;
    int  iHoloKoteImage    = 0;
    int  iHoloKoteMap      = 0;
    BOOL bCustomKeyDisable = FALSE;
    BOOL bHoloPatch        = FALSE;
    BOOL bHoloPatchHole    = FALSE;
    BOOL bArea_UserDefined = FALSE;
    BOOL bHole_MagNormal   = FALSE;
    BOOL bHole_MagWide     = FALSE;
    BOOL bHole_ChipNormal  = FALSE;
    BOOL bHole_ChipLarge   = FALSE;
    BOOL bHole_UserDefined = FALSE;
    int  iTileRotate       = 0;
    BOOL bMagUserDefined   = (psettings->nTrackOptions == UICBVAL_Encoding_UserDefined);
	time_t ltime;
	//CHAR  tchLangID[MAX_LANGID + 1];
	char   temp_string[20];
	//WCHAR  wsTemp[BUFSIZE];
	int    i;
	DWORD  dwPowerTemp;
	LPMAGTRACKINFO lpMagTrackInfo = NULL;
	//int density;
    //CHAR            tcPrinterName[100];
    BOOL bMagDataNotPresent = FALSE;  

		
    if ((psettings->bPerformVerification || psettings->bEncodeOnly)
    && (!LAMINATOR_MODEL(psettings) || !psettings->bLaminateOnly)
    && (pdev->magTrackInfo[MAG_TRACK_INDEX1].TrackData[0] == 0)
    && (pdev->magTrackInfo[MAG_TRACK_INDEX2].TrackData[0] == 0)
    && (pdev->magTrackInfo[MAG_TRACK_INDEX3].TrackData[0] == 0))
	{
		bMagDataNotPresent = TRUE;
		
		if (psettings->bEncodeOnly)
		{
/*			DWORD physicalConsoleSession = WTSGetActiveConsoleSessionId();
 
			if (0xFFFFFFFF != physicalConsoleSession) 
			{
			DWORD response;
			WCHAR szMessage[256];
			WCHAR szTitle[256];
			
			wcscpy_s((PWSTR)&szTitle, sizeof(szTitle)-1, MSG_s(IDMAGENCODING_CAPTION));
			wcscpy_s((PWSTR)&szMessage, sizeof(szMessage)-1, MSG_s(IDMAGENCODING_NODATA));
			
			WTSSendMessage(WTS_CURRENT_SERVER_HANDLE, 
						  physicalConsoleSession, 
						  (PWSTR)&szTitle, 
						  wcslen((PWSTR)&szTitle) * sizeof(WCHAR),
						  (PWSTR)&szMessage, 
						  wcslen((PWSTR)&szMessage) * sizeof(WCHAR),
						  MB_OK | MB_ICONINFORMATION,
						  0,
						  &response, 
						  FALSE);

			//fall through which sends a minimal header to the printer 
				
			}		
*/
		}

	}

	// Send commands for leading characters and Start Of Header (SOH)
	for (i = 0; i < NUMBER_05S; i++)
	{
        printf(CMD_STR_LEADCHAR);
	}
    printf(CMD_STR_SOH);
	
	if (iCopyCount == 0) 
		iCopyCount = 1;
	
    if (!ENDURO_OEM(psettings) && IgnoreImageData(pdev, psettings))
    {
        // Send commands for printed and magnetic copies when "Magnetic encode only" is selected
        printf(CMD_STR_NUMBEROFCOPY, 0);
        printf(CMD_STR_NUMBEROFCOPY_MAG, iCopyCount);
    }
    else
    {
        // Send command for hardware copies
        printf(CMD_STR_NUMBEROFCOPY, iCopyCount);
    }

	if ((bFrontPage)
    || (ENDURO_OEM(psettings) && (psettings->Duplex == UICBVAL_Duplex_Back)))
	{
		//These commands are sent only for front page if image is double-sided
		
        if (psettings->Duplex == UICBVAL_Duplex_Back)
            printf(CMD_STR_BACKONLY);


		// Send commands for driver version
/*        WideCharToMultiByte(CP_ACP, 
							0, 
                            DRIVER_VERSION,     //set in oem.h 
							-1, 
							temp_string, 
							sizeof(temp_string), 
							NULL, 
                            NULL);*/
//TODO
        strcpy(temp_string, "1.16"/*DRV_VERSION*/);
        printf(CMD_STR_DRIVERVERSION, temp_string);

		//If Enduro Family, add the firmware version to the header
        if (ENDURO_OEM(psettings))
		{ 
			printf(CMD_STR_FWVERSION, psettings->es.sFirmwareVersion);
            /*if (GetFirmwareVersion((LPPCLDEVMODE)&pdev->dm, pdev->hPrinter, wsTemp))
			{
				WideCharToMultiByte(CP_ACP, 
									0, 
									wsTemp, 
									-1, 
									temp_string, 
									sizeof(temp_string), 
									NULL, 
									NULL);
                printf(CMD_STR_FWVERSION), temp_string);
			}*/
		}

        OutputComma(pdev);

		// Send commands for language
//TODO
/*        wsprintfW((PWSTR)&tchLangID, L"%ws", MSG_s(IDINSTALLEDLANGUAGE));
        WideCharToMultiByte(CP_ACP, 
							0, 
                            tchLangID, 
							-1, 
							temp_string, 
							sizeof(temp_string), 
							NULL, 
							NULL);*/
        printf(CMD_STR_LANGUAGE, "ENG"/*temp_string*/);

		//Put the current time and date in the header
		time(&ltime);
		sprintf(temp_string, "%X", (unsigned int)ltime);
        printf(CMD_STR_TIME_DATE, temp_string);
	
		//Insert the Card Size if it is CR79
        if (psettings->pageType == 2 && psettings->Printer != RIO_PRO_XTENDED && psettings->Printer != PRIDENTOPROXTD && psettings->Printer != PRICECARD_PRO_FLEX)
        {    
            printf(CMD_STR_CR79);
		}

        if (ENDURO_OEM(psettings))
		{
            if (!RIOPRO_TYPE(psettings))
			{
//                switch (lpds->eResinQuality)
                switch (psettings->nPrintSpeed)
				{
				case UICBVAL_DefaultSpeed: 
                    printf(CMD_STR_RESIN_QUALITY, 
										  CMD_STR_RESIN_QUALITY_DEFAULT);  
					break;
				case UICBVAL_HighSpeed:    
                    printf(CMD_STR_RESIN_QUALITY, 
										  CMD_STR_RESIN_QUALITY_FAST); 
					break;
				case UICBVAL_HighQuality:  
                    printf(CMD_STR_RESIN_QUALITY, 
										  CMD_STR_RESIN_QUALITY_HIGH);   
					break;
				}
			}

			//Commands for Rewritable Cards......
			//Erase On
            if (!psettings->bEraseBeforePrint)
			{
                printf(CMD_STR_ERASE_ON);
                printf(CMD_STR_REWRITE_ERASE_AREA,
                                    (unsigned int)psettings->EraseArea_Left,
                                    (unsigned int)psettings->EraseArea_Bottom,
                                    (unsigned int)psettings->EraseArea_Left   + (unsigned int)psettings->EraseArea_Width,
                                    (unsigned int)psettings->EraseArea_Bottom + (unsigned int)psettings->EraseArea_Height);
			}
fprintf(stderr, "CATULTRA ErasePower_Start: %u\n",psettings->ErasePower_Start);
fprintf(stderr, "CATULTRA ErasePower_End: %u\n",psettings->ErasePower_End);
			//Erase Start
            if (psettings->ErasePower_Start != DEFAULT_ERASE_START_POWER)
			{
                dwPowerTemp = (psettings->ErasePower_Start * 65535) / 100;
                printf(CMD_STR_ERASE_START, (unsigned int)dwPowerTemp);
			}

			//Erase End
            if (psettings->ErasePower_End != DEFAULT_ERASE_END_POWER)
			{
                dwPowerTemp = (psettings->ErasePower_End * 65535) / 100;
                printf(CMD_STR_ERASE_END, (unsigned int)dwPowerTemp);
			}

			//Erase Write
            if (psettings->WritePowerAdjustment != DEFAULT_ERASE_WRITE_POWER)
			{
                printf(CMD_STR_ERASE_WRITE, psettings->WritePowerAdjustment);
			}
		}

        switch (psettings->nDyeFilmOption/*lpds->eDyeFilmInstalled*/)
		{
		case UICBVAL_DyeFilm_LC1:
		case UICBVAL_DyeFilm_AV1: printf(CMD_STR_LC1); break;
		case UICBVAL_DyeFilm_LC3: printf(CMD_STR_LC3); break;
		case UICBVAL_DyeFilm_LC6: printf(CMD_STR_LC6); break;
		case UICBVAL_DyeFilm_LC8: printf(CMD_STR_LC8); break;
		default:
		case UICBVAL_DyeFilm_Auto: 
			//No command to be inserted - this is default and can only be changed in Rio
			break;
		}

		//Magnetic Encoding Commands
        if (psettings->bJIS2Enabled)
		{
            printf(CMD_STR_JIS2_ENCODING);
		}

        if (!LAMINATOR_MODEL(psettings) || !psettings->bLaminateOnly)
		{
			ULONG          ulTrack        = 0;

            if (RIO_OEM(psettings)
            ||  AVALON_MODEL(psettings)
            ||  ENDURO_OEM(psettings))
			{
				// Send magnetic encoding command and data to the device.
                if (psettings->bPerformVerification && bMagDataNotPresent == FALSE)
				{
					// Send command for Mag Verify On
                    printf(CMD_STR_MAGVERIFY);
				}
			}

            if ((RIO_OEM(psettings)      && !bMagUserDefined)
            ||  (RIOPRO_TYPE(psettings) && !bMagUserDefined)
            ||  ENDURO_TYPE(psettings)
			||  ENDURO_PLUS_TYPE(psettings)
            ||  PRONTO_TYPE(psettings))
			{
                switch (psettings->nCoercivity)
				{
					case UICBVAL_Coercivity_HiCo:
						// Send command for High Coercivity
                        printf(CMD_STR_HICOERCIVITY);
						break;

					case UICBVAL_Coercivity_LoCo:
						// Send command for Low Coercivity
                        printf(CMD_STR_LOCOERCIVITY);
						break;
				}
			}

			for (ulTrack = FIRST_MAG_TRACK; ulTrack <= LAST_MAG_TRACK; ulTrack++)
			{
                lpMagTrackInfo = &pdev->magTrackInfo[ulTrack];

				if (lpMagTrackInfo->TrackData[0] != 0)
				{
                    printf(CMD_STR_STARTMAGDATA);

					//Write the track number and comma from the track data
                    myWrite(pdev, lpMagTrackInfo->TrackData, 2);

					if (!bMagUserDefined)
					{
                        WriteBitsPerChar(pdev, psettings, ulTrack);
                        WriteBitsPerInch(pdev, psettings, ulTrack);
					}

                    myWrite(pdev, &lpMagTrackInfo->TrackData[2],
								   (ULONG)(strlen(lpMagTrackInfo->TrackData) - 2));

                    OutputComma(pdev);
				}
			}
		}

        //only issue handfeed command if printer is not remote
        if (pdev->epPrinterRemote == FALSE)
        {
            if (RIO_OEM(psettings) && psettings->HandFeed)
            {
                // Send command for hand feed paper source
                printf(CMD_STR_HANDFEED, CMD_STR_PARAM_ON );
            }
        }
      
        if (TANGO_MODEL(psettings) || LAMINATOR_MODEL(psettings))
		{
			// Send command for setting reject on or off
            printf(CMD_STR_CARDREJECT,
                                  psettings->CardReject ? CMD_STR_PARAM_ON : CMD_STR_PARAM_OFF);

            if (TANGO_MODEL(psettings))
			{
				//Tango Specific options
				// Send command for selecting the card eject side
                printf(CMD_STR_EJECTSIDE,
                                    psettings->EjectSide == UICBVAL_EjectSide_Front 
										? FRONT_PAGE : BACK_PAGE);
			}
			else
			{
				//Laminator Specific options
				//Insert Laminator Commands
                LaminatorCommand(pdev, psettings);
			}
		}
	}
	
	//Output the Printhead identifier
    if (RIO_OEM(psettings)
    ||  AVALON_MODEL(psettings))
	{
        printf(CMD_STR_PRINTHEAD_RIO);
	}
    else if (ENDURO_OEM(psettings))
	{
        switch (psettings->es.ePrintheadType)
		{
		default:
		case UICBVAL_PrintheadType_KGE2:
            printf(CMD_STR_PRINTHEAD_ENDURO_KGE2); 
			break;
		case UICBVAL_PrintheadType_KEE1:
            printf(CMD_STR_PRINTHEAD_ENDURO_KEE1); 
			break;
		case UICBVAL_PrintheadType_KEE4:
            printf(CMD_STR_PRINTHEAD_ENDURO_KEE4); 
			break;
		}

	}
	else
	{
        printf(CMD_STR_PRINTHEAD_ALTO);
	}
fprintf(stderr, "L1914:CATULTRA FLUSHING OutputCommandHdr %u\n",bFrontPage);
fflush(stdout);
    if (IgnoreImageData(pdev, psettings))
    {
        // Send commands for end-of-header and end-of-page
        OutputFileSeparator(pdev);
        OutputETXCommand(pdev);

        // skip rest of job data since doing magnetic encoding or laminate only
        return;
    }

	//Add a comma after the Printhead Identifier as we are not doing mag encode only
    OutputComma(pdev);

    if (RIO_OEM(psettings))
	{
        printf(CMD_STR_RIOTANGO2);

		// Send command for ColourSure On/Off
        printf(CMD_STR_COLOURSURE,
                              psettings->bColourSure ? CMD_STR_PARAM_ON : CMD_STR_PARAM_OFF);
	}
	
    if (RIOPRO_TYPE(psettings))
	{
		// Send command for ColourSure On/Off
        printf(CMD_STR_COLOURSURE,
                              psettings->nPrintSpeed ? CMD_STR_PARAM_ON : CMD_STR_PARAM_OFF);
	}

    if (psettings->Printer == RIO_PRO_XTENDED || psettings->Printer == PRIDENTOPROXTD || psettings->Printer == PRICECARD_PRO_FLEX/*XXL_TYPE(psettings)*/)
	{
		//1400 1654
		float fcx;
		int icx;
		if (psettings->Printer == PRICECARD_PRO_FLEX)
		{
			if ( psettings->pageType == 1) //extd mono card 140mm
				psettings->PaperHeight = 1400;		
			else
			if ( psettings->pageType == 3) //extd mono card 109mm
				psettings->PaperHeight = 1090;	
			else
			if ( psettings->pageType == 4) //extd mono card 128mm 
				psettings->PaperHeight = 1280;	
			
		}
		else
		{
		//just handle xtd for now
			if ( psettings->pageType == 2) //extd colour card
				psettings->PaperHeight = 1080;
			else
			if ( psettings->pageType == 3) //extd mono card 140mm
				psettings->PaperHeight = 1400;		
			else
			if ( psettings->pageType == 4) //extd mono card 109mm
				psettings->PaperHeight = 1090;	
			else
			if ( psettings->pageType == 5) //extd mono card 128mm 
				psettings->PaperHeight = 1280;	
		}
		fcx = (psettings->PaperHeight * (float)300) / (float)254;
        icx = (int) (fcx + 0.5f);	
	
		// Send command to define the XXL Image Type & Size
        printf(CMD_STR_XXL_IMAGE_TYPE, 
							psettings->XXL_ImageType,
							(psettings->XXL_ImageType == UICBVAL_SingleImage)
								? 1026 : (RIO_OEM(psettings)) ? pdev->epPaperXdots : (short)icx);
	}

    if (RIO_OEM(psettings)
    ||  AVALON_MODEL(psettings)
    ||  ENDURO_OEM(psettings))
	{
		// Send command for duplex on/off
        printf(CMD_STR_DUPLEX,
                              pdev->bDuplex ? CMD_STR_PARAM_ON : CMD_STR_PARAM_OFF);
	}

    if (bFrontPage)
    {
        int iImageFormat = 0;

        // Back page image format
        if (pdev->bDuplex)  //Two sided
        {
            iImageFormat = GetChannelOption(pdev, psettings, BACK);

            if (psettings->CB_PrintOvercoat
            &&  !GetDataPresent(pdev, FALSE)) //No data on the back?
            {
                printf(CMD_STR_BACKPAGEFORMAT, CMD_STR_PARAM_O_BAC);
			}
            else if ((psettings->CB_PrintOvercoat)
				 || (RIO_PRO_X_MODEL(psettings) && psettings->CF_PrintOvercoat))
            {
                // Send back page format command on the NEXT page (with overcoat)
                switch (iImageFormat)
                {
                    case UICBVAL_YMCK_Back:
                    default:
                        // Send command for back side YMCK with overcoat (next page)
                        printf(CMD_STR_BACKPAGEFORMAT,
											  CMD_STR_PARAM_CMYKO_BAC);
                        break;

                    case UICBVAL_YMC_Back:
                        // Send command for back side YMC with overcoat (next page)
                        printf(CMD_STR_BACKPAGEFORMAT,
											  CMD_STR_PARAM_CMYO_BAC);
                        break;

                    case UICBVAL_KResin_Back:
                        // Send command for back side K-resin with overcoat (next page)
                        printf(CMD_STR_BACKPAGEFORMAT,
											  CMD_STR_PARAM_KO_BAC);
                        break;
                }
            }
            else if (GetDataPresent(pdev, FALSE)) //Data on the back?
            {
                // Send back page format command (with no overcoat)
                switch (iImageFormat)
                {
                    case UICBVAL_YMCK_Back:
                    default:
                        // Send command for back side YMCK with no overcoat (next page)
                        printf(CMD_STR_BACKPAGEFORMAT,
											  CMD_STR_PARAM_CMYK_BAC);
                        break;

                    case UICBVAL_YMC_Back:
                        // Send command for back side YMC with no overcoat (next page)
                        printf(CMD_STR_BACKPAGEFORMAT,
											  CMD_STR_PARAM_CMY_BAC);
                        break;

                    case UICBVAL_KResin_Back:
                        // Send command for back side K-resin with no overcoat (next page)
                        printf(CMD_STR_BACKPAGEFORMAT,
											  CMD_STR_PARAM_K_BAC);
                        break;
                }
            }
        }

        // Send command to say that this is a front page
        printf(CMD_STR_CURRENTPAGE, FRONT_PAGE);

		// Send command for X-Axis Adjust
//      printf(CMD_STR_X_AXIS_ADJUST, lpds->iXAxisAdjust_Front);

        if (GetDataPresent(pdev, TRUE)) //Data on the front?
		{

            iImageFormat = GetChannelOption(pdev, psettings, FRONT);
fprintf(stderr, "L2026:CATULTRA data on the front :iImageFormat:%u\n",iImageFormat);	
            // Send front page format command. Overcoat is determined below.
            switch (iImageFormat)
            {
                case UICBVAL_YMC_Front:
                    // Send command for front side YMC
                    printf(CMD_STR_IMAGEFORMAT,
						    			  CMD_STR_PARAM_CMY);
                    break;

                case UICBVAL_YMCK_Front:
                default:
                    // Send command for front side YMCK
                    printf(CMD_STR_IMAGEFORMAT,
						    			  CMD_STR_PARAM_CMYK);
                    break;

                case UICBVAL_KResin_Front:
                    // Send command for front side K-resin
                    printf(CMD_STR_IMAGEFORMAT,
										  CMD_STR_PARAM_K);
                    break;
			}
        }
        else    //emulate a blank page to maintain wysiwyg 
        {
            printf(CMD_STR_IMAGEFORMAT,
								  CMD_STR_PARAM_K);
        }
    }

    else // bFrontPage == FALSE
    {
 ////       if (GetDataPresent(pdev, FALSE)) //Data on the back?
		{
            int iImageFormat = GetChannelOption(pdev, psettings, BACK);

            // Send command to say that this is a back page
            printf(CMD_STR_CURRENTPAGE, 
                                       ENDURO_OEM(psettings) 
                                                && (psettings->Duplex == UICBVAL_Duplex_Back)
														? FRONT_PAGE : BACK_PAGE);

			// Send command for X-Axis Adjust
//          printf(CMD_STR_X_AXIS_ADJUST, lpds->iXAxisAdjust_Back);

            // Send back page format command. Overcoat is determined below.
            switch (iImageFormat)
            {
                case UICBVAL_YMC_Back:
                    // Send command for back side YMC
                    printf(CMD_STR_IMAGEFORMAT, CMD_STR_PARAM_CMY);
                    break;

                case UICBVAL_YMCK_Back:
                default:
                    // Send command for back side YMCK
                    printf(CMD_STR_IMAGEFORMAT, CMD_STR_PARAM_CMYK);
                    break;

                case UICBVAL_KResin_Back:
                    // Send command for back side K-resin
                    printf(CMD_STR_IMAGEFORMAT, CMD_STR_PARAM_K);
                    break;
			}
        }
    }

    // Send command to set page origin
    printf(CMD_STR_ORIGIN);

/*    if (pdev->xImage > pdev->yImage)
    {
        // Send command to set page area
        printf(CMD_STR_IMAGESIZE,
                            pdev->xImage,
                            pdev->yImage);
    }
    else*/
    {
        // Send command to set page area
        printf(CMD_STR_IMAGESIZE,
                            pdev->epPaperXdots,
                            pdev->epPaperYdots);
    }

    //Record the overcoat options so that they can be used to determine
    //commands that need to be sent (see later)
    if (bFrontPage)
    {
        bOverCoat = psettings->CF_PrintOvercoat;

        if (bOverCoat)
        {
			// Record Front Overcoat Options
            bHoloKote    = psettings->CF_HoloKote;
            iHoloKoteMap = psettings->CF_SecurityOptions_HoloKoteMap;

            if (RIO_OEM(psettings)
            ||  ENDURO_OEM(psettings))
			{
                bCustomKeyDisable = psettings->CF_SecurityOptions_DisableCustomHoloKoteKey;
                bUseWithLaminate  = psettings->CF_SecurityOptions_UsewithLaminate;
                bHoloPatch        = psettings->CF_HoloPatch;//psettings->CF_SecurityOptions_HoloKotePatch;
                bHoloPatchHole    = psettings->CF_SecurityOptions_ColourHole;
                
                if (RIO_OEM(psettings))
				{
					//Rio/Tango front overcoat options
                    iHoloKoteImage = psettings->CF_SecurityOptions_SecurityType + 1;
				}
                else if (RIOPRO_TYPE(psettings) ||  PRIDENTO_TYPE(psettings)  ||  IDVILLE_TYPE(psettings))
				{
					//Rio Pro front overcoat options
                    iHoloKoteImage = RioProHKTValue(psettings,psettings->CF_SecurityOptions_SecurityType);
				}
				else
				{
					//Other Enduro OEM front overcoat options
                    iHoloKoteImage = psettings->CF_SecurityOptions_SecurityType + 1;
                    if (POLAROID_TYPE(psettings))
					{
////                        bCustomKeyDisable = (lpds->eHoloKoteImage_Enduro_Front != UICBVAL_HoloKote_Polaroid_Enduro_Front);
                        bCustomKeyDisable = (psettings->CF_SecurityOptions_SecurityType != UICBVAL_HoloKote_Polaroid_Enduro_Front);
				}
				}
			}

			else 
			{
			    //AOTA Front overcoat options
				//Colour hole always on if holopatch is enabled
                bHoloPatch     = 
                bHoloPatchHole = psettings->CF_HoloPatch;

                if (AVALON_MODEL(psettings))
			    {
                    bUseWithLaminate = psettings->CF_SecurityOptions_UsewithLaminate;
				}
			}
            
            switch (psettings->CF_SecurityOptions_Rotation)
            {
			default:
			case UICBVAL_TileRotate_Front_0:   iTileRotate = 0;   break;
			case UICBVAL_TileRotate_Front_90:  iTileRotate = 90;  break;
			case UICBVAL_TileRotate_Front_180: iTileRotate = 180; break;
			case UICBVAL_TileRotate_Front_270: iTileRotate = 270; break;
            }

            bArea_UserDefined = psettings->CF_OvercoatOptions_bUserDefined;

            switch (psettings->CF_OvercoatOptions_Holes)
            {
                case UICBVAL_MagStripeNormal: bHole_MagNormal   = TRUE; break;
                case UICBVAL_MagStripeWide:   bHole_MagWide     = TRUE; break;
                case UICBVAL_ChipNormal:      bHole_ChipNormal  = TRUE; break;
                case UICBVAL_ChipLarge:       bHole_ChipLarge   = TRUE; break;
                case UICBVAL_UserDefined:     bHole_UserDefined = TRUE; break;
            }
        }
    }

	else // bFrontPage == FALSE (i.e. This is the back)
    {
        bOverCoat = RIO_PRO_X_MODEL(psettings) ? psettings->CF_PrintOvercoat : psettings->CB_PrintOvercoat;

        if (bOverCoat)
        {
			// Record Back Overcoat Options
			bHoloKote    = RIO_PRO_X_MODEL(psettings) ? psettings->CF_HoloKote 
														: psettings->CB_HoloKote;
			iHoloKoteMap = RIO_PRO_X_MODEL(psettings) ? psettings->CF_SecurityOptions_HoloKoteMap 
														: psettings->CB_SecurityOptions_HoloKoteMap;
            
 			
            if (RIO_OEM(psettings)
            ||  ENDURO_OEM(psettings))
			{
				bCustomKeyDisable = RIO_PRO_X_MODEL(psettings) ?  psettings->CF_SecurityOptions_DisableCustomHoloKoteKey
																 :  psettings->CB_SecurityOptions_DisableCustomHoloKoteKey;
				bUseWithLaminate  = RIO_PRO_X_MODEL(psettings) ? psettings->CF_SecurityOptions_UsewithLaminate 
																 : psettings->CB_SecurityOptions_UsewithLaminate;

                if (RIO_OEM(psettings))
				{
					//Rio/Tango back overcoat options
                    //iHoloKoteImage = lpds->eHoloKoteImage_Rio_Back + 1;
                    iHoloKoteImage = psettings->CB_SecurityOptions_SecurityType + 1;
				}
                else if (RIOPRO_TYPE(psettings) ||  PRIDENTO_TYPE(psettings)  ||  IDVILLE_TYPE(psettings))
				{
					//Rio Pro back overcoat options
                    //iHoloKoteImage = RioProHKTValue(lpds->eHoloKoteImage_RioPro_Back);
					iHoloKoteImage = RioProHKTValue(psettings, RIO_PRO_X_MODEL(psettings) 
													? psettings->CF_SecurityOptions_SecurityType 
													: psettings->CB_SecurityOptions_SecurityType);
                    
				}
				else
				{
					//Other Enduro OEM back overcoat options
                    //iHoloKoteImage = lpds->eHoloKoteImage_Enduro_Back + 1;
                    iHoloKoteImage = psettings->CB_SecurityOptions_SecurityType + 1;
                    if (POLAROID_TYPE(psettings))
					{
                        //bCustomKeyDisable = (lpds->eHoloKoteImage_Enduro_Back != UICBVAL_HoloKote_Polaroid_Enduro_Back);
                        bCustomKeyDisable = (psettings->CB_SecurityOptions_SecurityType != UICBVAL_HoloKote_Polaroid_Enduro_Back);
				}
				}
			}
			else
			{
			    //AOTA Back overcoat options
                if (AVALON_MODEL(psettings))
			    {
                    bUseWithLaminate = psettings->CB_SecurityOptions_UsewithLaminate;
				}
			}
			switch (RIO_PRO_X_MODEL(psettings) ? psettings->CF_SecurityOptions_Rotation 
												 : psettings->CB_SecurityOptions_Rotation)
			{
			default:
			case UICBVAL_TileRotate_Back_0:   iTileRotate = 0;   break;
			case UICBVAL_TileRotate_Back_90:  iTileRotate = 90;  break;
			case UICBVAL_TileRotate_Back_180: iTileRotate = 180; break;
			case UICBVAL_TileRotate_Back_270: iTileRotate = 270; break;
			}

			bArea_UserDefined = RIO_PRO_X_MODEL(psettings) ? psettings->CF_OvercoatOptions_bUserDefined 
															 : psettings->CB_OvercoatOptions_bUserDefined;

            switch (RIO_PRO_X_MODEL(psettings) ? psettings->CF_OvercoatOptions_Holes 
												 : psettings->CB_OvercoatOptions_Holes)
			{
            case UICBVAL_MagStripeNormal: bHole_MagNormal   = TRUE; break;
            case UICBVAL_MagStripeWide:   bHole_MagWide     = TRUE; break;
            case UICBVAL_ChipNormal:      bHole_ChipNormal  = TRUE; break;
            case UICBVAL_ChipLarge:       bHole_ChipLarge   = TRUE; break;
            case UICBVAL_UserDefined:     bHole_UserDefined = TRUE; break;
			}
        }
    }

	//Send Overcoat options to header
	if (!bOverCoat)
    {
        // Send command for not printing overcoat
        printf(CMD_STR_OVERCOAT, CMD_STR_PARAM_OFF);
    }
    else
    {
        // Send command for printing overcoat
        printf(CMD_STR_OVERCOAT, CMD_STR_PARAM_ON);

        // Send command for printing SecureShield/Use With Laminate
        printf(CMD_STR_USEWITHLAMINATE,
                              bUseWithLaminate ? CMD_STR_PARAM_ON : CMD_STR_PARAM_OFF);

        // Send command for printing HoloKote
        printf(CMD_STR_HOLOKOTE,
                              bHoloKote ? CMD_STR_PARAM_ON : CMD_STR_PARAM_OFF);

        if (bHoloKote || bUseWithLaminate || bHoloPatch)
        {
            if (RIO_OEM(psettings)
            ||  ENDURO_OEM(psettings))
			{
                //Send Command for HoloKote Image Selection
                printf(CMD_STR_HOLOKOTE_IMAGE, iHoloKoteImage);

				//Send Command for Custom Key Disable
                printf(CMD_STR_CUSTOMKEY_DISABLE,
									  bCustomKeyDisable 
											? CMD_STR_PARAM_OFF : CMD_STR_PARAM_ON );

                if (RIO_OEM(psettings)
                ||  ENDURO_OEM(psettings))
				{
					//Send Command for HoloKoteMap
					char sHoloKoteMap[7];
					sprintf(sHoloKoteMap, "%06X", iHoloKoteMap); 

                    printf(CMD_STR_HOLOKOTE_MAP,
										  sHoloKoteMap);
				}

				//Send command for TileRotate (Rio/Tango/Enduro)
                switch (iTileRotate)
                {
			    case 0 :
			    default:
                    printf(CMD_STR_TILEROTATE,
                                          CMD_STR_ROTATE_NONE);
				    break;

			    case 90 :
                    printf(CMD_STR_TILEROTATE,
                                          CMD_STR_ROTATE_90);
				    break;

			    case 180 :
                    printf(CMD_STR_TILEROTATE,
                                          CMD_STR_ROTATE_180);
				    break;

			    case 270 :
                    printf(CMD_STR_TILEROTATE,
                                          CMD_STR_ROTATE_270);
				    break;
			    }
			}

            else if (AVALON_MODEL(psettings))
			{
				//Send command for TileRotate (Avalon)
                switch (iTileRotate)
				{
			    case 0 :
			    default:
                    printf(CMD_STR_TILEROTATE,
                                          CMD_STR_PARAM_OFF);
				    break;

			    case 180 :
                    printf(CMD_STR_TILEROTATE,
                                          CMD_STR_PARAM_ON);
				    break;
				}
			}

			else
			{
				//Send command for TileRotate (Alto/Opera/Tempo)
                switch (iTileRotate)
                {
			    case 0 :
			    default:
                    printf(CMD_STR_TILEROTATE,
                                          CMD_STR_PARAM_OFF);
                    printf(CMD_STR_HOLOKOTE_ORIENT,
                                          CMD_STR_HOLOKOTE_LANDSCAPE);
				    break;

			    case 90 :
                    printf(CMD_STR_TILEROTATE,
                                          CMD_STR_PARAM_OFF);
                    printf(CMD_STR_HOLOKOTE_ORIENT,
                                          CMD_STR_HOLOKOTE_PORTRAIT);
				    break;

			    case 180 :
                    printf(CMD_STR_TILEROTATE,
                                          CMD_STR_PARAM_ON);
                    printf(CMD_STR_HOLOKOTE_ORIENT,
                                          CMD_STR_HOLOKOTE_LANDSCAPE);
				    break;

			    case 270 :
                    printf(CMD_STR_TILEROTATE,
                                          CMD_STR_PARAM_ON );
                    printf(CMD_STR_HOLOKOTE_ORIENT,
                                          CMD_STR_HOLOKOTE_PORTRAIT);
				    break;
				}
			}
        }

		if (!bArea_UserDefined)
        {
            if (RIO_OEM(psettings)
            ||  AVALON_MODEL(psettings))
			{
                printf(CMD_STR_OVERCOAT_RIO);
			}
            else if (ENDURO_OEM(psettings))
			{
                printf(CMD_STR_OVERCOAT_ENDURO);
			}
			else
			{
                printf(CMD_STR_OVERCOAT_ALTO);
			}
        }
        else
        {
            if (bFrontPage)
            {
                if (AreaHoleActive(pdev, psettings, FRONT_AREA1))
                {
                    printf(CMD_STR_OVERCOAT_USER,
                                        (long unsigned int)psettings->CF_FrontArea1_Left,
                                        (long unsigned int)psettings->CF_FrontArea1_Bottom,
                                        (long unsigned int)psettings->CF_FrontArea1_Left + psettings->CF_FrontArea1_Width,
                                        (long unsigned int)psettings->CF_FrontArea1_Bottom + psettings->CF_FrontArea1_Height);
                }
                if (AreaHoleActive(pdev, psettings, FRONT_AREA2))
                {
                    printf(CMD_STR_OVERCOAT_USER,
                                        (long unsigned int)psettings->CF_FrontArea2_Left,
                                        (long unsigned int)psettings->CF_FrontArea2_Bottom,
                                        (long unsigned int)psettings->CF_FrontArea2_Left + psettings->CF_FrontArea2_Width,
                                        (long unsigned int)psettings->CF_FrontArea2_Bottom + psettings->CF_FrontArea2_Height);
                }
            }
            else
            {
                if (AreaHoleActive(pdev, psettings, BACK_AREA1))
                {
                    printf(CMD_STR_OVERCOAT_USER,
                                        (long unsigned int)psettings->CB_BackArea1_Left,
                                        (long unsigned int)psettings->CB_BackArea1_Bottom,
                                        (long unsigned int)psettings->CB_BackArea1_Left + psettings->CB_BackArea1_Width,
                                        (long unsigned int)psettings->CB_BackArea1_Bottom + psettings->CB_BackArea1_Height);
                }
                if (AreaHoleActive(pdev, psettings, BACK_AREA2))
                {
                    printf(CMD_STR_OVERCOAT_USER,
                                        (long unsigned int)psettings->CB_BackArea2_Left,
                                        (long unsigned int)psettings->CB_BackArea2_Bottom,
                                        (long unsigned int)psettings->CB_BackArea2_Left + psettings->CB_BackArea2_Width,
                                        (long unsigned int)psettings->CB_BackArea2_Bottom + psettings->CB_BackArea2_Height);
                }
            }
        }

        if (bHoloPatch)
        {
            // Send command for HoloPatch
            for (i=0; i<24; i++)
                if (psettings->CF_SecurityOptions_HoloKotePatch & (1 << i))
                    break;
                    
            printf(CMD_STR_HOLOPATCHPOS, i+1/*lpds->iHolopatchArea*/);

            // Send command for HoloPatchHole
            printf(CMD_STR_HOLOPATCHHOLE,
				                  bHoloPatchHole ? CMD_STR_PARAM_ON : CMD_STR_PARAM_OFF);
        }

        if (bHole_MagNormal)
		{
            printf(CMD_STR_MAG_STRIP_NORMAL);
		}
        else if (bHole_MagWide)
		{
            printf(CMD_STR_MAG_STRIP_WIDE);
		}
        else if (bHole_ChipNormal)
		{
            printf(CMD_STR_CHIP_NORMAL);
		}
        else if (bHole_ChipLarge)
		{
            printf(CMD_STR_CHIP_LARGE);
		}
        else if (bHole_UserDefined)
        {
            if (bFrontPage)
            {
                if (AreaHoleActive(pdev, psettings, FRONT_HOLE1))
                {
                    printf(CMD_STR_HOLE_USER_DEFINED,
                                        (long unsigned int)psettings->CF_FrontHole1_Left,
                                        (long unsigned int)psettings->CF_FrontHole1_Bottom,
                                        (long unsigned int)psettings->CF_FrontHole1_Left + psettings->CF_FrontHole1_Width,
                                        (long unsigned int)psettings->CF_FrontHole1_Bottom + psettings->CF_FrontHole1_Height);
                }
                if (AreaHoleActive(pdev, psettings, FRONT_HOLE2))
                {
                    printf(CMD_STR_HOLE_USER_DEFINED,
                                        (long unsigned int)psettings->CF_FrontHole2_Left,
                                        (long unsigned int)psettings->CF_FrontHole2_Bottom,
                                        (long unsigned int)psettings->CF_FrontHole2_Left + psettings->CF_FrontHole2_Width,
                                        (long unsigned int)psettings->CF_FrontHole2_Bottom + psettings->CF_FrontHole2_Height);
                }
            }
            else
            {
                if (AreaHoleActive(pdev, psettings, BACK_HOLE1))
                {
                    printf(CMD_STR_HOLE_USER_DEFINED,
                                        (long unsigned int)psettings->CB_BackHole1_Left,
                                        (long unsigned int)psettings->CB_BackHole1_Bottom,
                                        (long unsigned int)psettings->CB_BackHole1_Left + psettings->CB_BackHole1_Width,
                                        (long unsigned int)psettings->CB_BackHole1_Bottom + psettings->CB_BackHole1_Height);
                }
                if (AreaHoleActive(pdev, psettings, BACK_HOLE2))
                {
                    printf(CMD_STR_HOLE_USER_DEFINED,
                                        (long unsigned int)psettings->CB_BackHole2_Left,
                                        (long unsigned int)psettings->CB_BackHole2_Bottom,
                                        (long unsigned int)psettings->CB_BackHole2_Left + psettings->CB_BackHole2_Width,
                                        (long unsigned int)psettings->CB_BackHole2_Bottom + psettings->CB_BackHole2_Height);
                }
            }
        }
    }

	// Send Power commands to header...
	// ...Send command for YMC printhead power
    // send command for ICC profile selected.. informs firmware of base heat to apply
    // always send to all models for tech support diagnosis

	printf(CMD_STR_ICC, psettings->nColourCorrection);

    if ((psettings->nColourCorrection == UICBVAL_ColourCorrection_ICC_Internal))
//    ||  (psettings->nColourCorrection == UICBVAL_ColourCorrection_ICC_External))
	{
        printf(CMD_STR_PH_YMC, psettings->nPrintHeadPower_YMC);
	}
    else if (psettings->nPrintHeadPower_YMC  != DEFAULT_PRINTHEAD_POWER_YMC)
    {
        printf(CMD_STR_PH_YMC, psettings->nPrintHeadPower_YMC);
    }

    if (psettings->nPrintHeadPower_BlackResin  != DEFAULT_PRINTHEAD_POWER_RESIN)
    {
        //...Send command for K-resin printhead power
        printf(CMD_STR_PH_K, psettings->nPrintHeadPower_BlackResin);
    }

    if (psettings->nPrintHeadPower_Overcoat != DEFAULT_PRINTHEAD_POWER_OVERCOAT)
    {
        //...Send command for overcoat printhead power
        printf(CMD_STR_PH_OVER, psettings->nPrintHeadPower_Overcoat);
    }

    if (psettings->nPrintHeadPosition != 0)
    {
        //...Send command for printhead position
        printf(CMD_STR_PH_POSITION,
                            psettings->nPrintHeadPosition);

    }

    if (psettings->nImagePosition_Start != 0)
    {
        //...Send command for image start position
        if (psettings->OEM != OEM_ENDURO)        
            printf(CMD_STR_IS_POSITION,DEFAULT_IMAGE_START_POSITION - psettings->nImagePosition_Start);
        else
            printf(CMD_STR_IS_POSITION,DEFAULT_IMAGE_START_POSITION + psettings->nImagePosition_Start);
    }

    if (psettings->nImagePosition_End != 0)
    {
        //...Send command for image end position
        if (psettings->OEM != OEM_ENDURO)            
            printf(CMD_STR_IE_POSITION,DEFAULT_IMAGE_END_POSITION - psettings->nImagePosition_End);
        else
            printf(CMD_STR_IE_POSITION,DEFAULT_IMAGE_END_POSITION + psettings->nImagePosition_End);        
    }

	//Handle ~0 commands that have come in from an application.  Just add
	//them into the header
    if (ENDURO_OEM(psettings))
	{
		lpMagTrackInfo = bFrontPage ? &pdev->magTrackInfo[MAG_TRACK_INDEX0]
									: &pdev->magTrackInfo[MAG_TRACK_INDEX4];     
                                    
		if (lpMagTrackInfo->TrackData[0] != 0)
		{
			//Do not include ? on the end if it exists
			i = strlen(lpMagTrackInfo->TrackData) - 2;
			if (lpMagTrackInfo->TrackData[strlen(lpMagTrackInfo->TrackData) - 1] == '?')
			{
				i--;
			}
			if (i > 255) i = 255;
            myWrite(pdev, &lpMagTrackInfo->TrackData[2], i);
            OutputComma(pdev);
		}
	}
//#endif
	// Flush the channel buffers
    ////PmChannelFlush(pdev);

    return;
}

/*****************************************************************************
 *  OutputPlaneCommand()
 *      Send plane data and command.
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID OutputPlaneCommand
(
    PDEVDATA pdev,			// Pointer to our PDEV
	struct 	 settings_ * settings,
    PBYTE pPlane,			// Pointer to output plane data
    ULONG ulColour,			// The colour to send out. PLANE_Y, PLANE_M, PLANE_C, PLANE_K
    ULONG ulOutBufLen		// The data length in pPlane
)
{	
//fprintf(stderr, "L3134:CATULTRA OutputPlaneCommand:pPlane:%x:ulColour:%u,ulOutBufLen:%u\n",pPlane,ulColour,ulOutBufLen);

	if (settings->OEM == OEM_PRO360)
		myWritecs(pdev, (char *)pPlane, ulOutBufLen, 0);
	else
		// Send the plane data
		myWrite(pdev, (char *)pPlane, ulOutBufLen);

    if (settings->OEM != OEM_HELIX && settings->OEM != OEM_PRO360)
    {
        // Send command to complete the plane data
        OutputFileSeparator(pdev);

        // Colour
        switch (ulColour)
        {
            case PLANE_C: printf(CMD_STR_OUTPUT_C); break;
            case PLANE_M: printf(CMD_STR_OUTPUT_M); break;
            case PLANE_Y: printf(CMD_STR_OUTPUT_Y); break;
            case PLANE_K: printf(CMD_STR_OUTPUT_K); break;
        }
    }
}

VOID OutputHelixHeaderCommand
(
 PDEVDATA pdev,     // Pointer to our PDEV
 cups_page_header2_t header,
 PSETTINGS psettings,
 BOOL     bFrontPage // TRUE = Front page (simplex or duplex); FALSE = Back page (simplex or duplex)
)
{
    HDR_JOB jobHeader;
    CARD_OPTIONS    cardOptions;
//    MAG          mag  = {0};
////	int          lead_zero_count = 0;	//?needed?
    LPSPOOLMEMINFO lpSplMemFront   = NULL;
    LPSPOOLMEMINFO lpSplMemBack    = NULL;
	
    int  iCopyCount        = header.NumCopies;
/*    BOOL bOverCoat         = FALSE;
    BOOL bUseWithLaminate  = FALSE;
    BOOL bHoloKote         = FALSE;
    int  iHoloKoteImage    = 0;
    int  iHoloKoteMap      = 0;
    BOOL bCustomKeyDisable = FALSE;
    BOOL bHoloPatch        = FALSE;
    BOOL bHoloPatchHole    = FALSE;
    BOOL bArea_UserDefined = FALSE;
    BOOL bHole_MagNormal   = FALSE;
    BOOL bHole_MagWide     = FALSE;
    BOOL bHole_ChipNormal  = FALSE;
    BOOL bHole_ChipLarge   = FALSE;
    BOOL bHole_UserDefined = FALSE;*/
////    int  iTileRotate       = 0;
//    BOOL bMagUserDefined   = (pdmPrivate->nTrackOptions == UICBVAL_Encoding_UserDefined);
////	time_t ltime;
////	CHAR  tchLangID[MAX_LANGID + 1];
////	char   temp_string[20];
////	CHAR  wsTemp[BUFSIZE];
	int    ulColour;
////	DWORD  dwPowerTemp;
//	LPMAGTRACKINFO lpMagTrackInfo = NULL;
////	int density;
	BOOL	bBlankFrontPage		= TRUE;
	BOOL	bBlankBackPage		= TRUE;
	BOOL    bSendMagEncodeJob   = FALSE;
////	ULONG          ulTrack        = 0;
    //
////	int iImageFormatFront = GetChannelOption(pdev, psettings, FRONT);
////	int iImageFormatBack = GetChannelOption(pdev, psettings, BACK);
	
    //ensure message dll is loaded
//    GetModuleFileName(pdev->hModule, DllName, MAX_PATH);
//    if (!CheckLangLoad(DllName,(PDEVMODE)&pdev->dm))
//        return;
    
    /////////////////////
	memset ((PBYTE)&jobHeader, 0, sizeof(HDR_JOB));
	//initialisation
////	pdev->bEncodeOnlyJob = FALSE;
	
    // Initialise the lookup tables
//    mag_encoder_setup_5bit_look_up();
//    mag_encoder_setup_7bit_look_up();
/*
	for (ulTrack = FIRST_MAG_TRACK; ulTrack <= LAST_MAG_TRACK; ulTrack++)
	{
		lpMagTrackInfo = &pdev->magTrackInfo[ulTrack];
        
		if (lpMagTrackInfo->TrackData[0] != 0)
		{
			bSendMagEncodeJob   = TRUE;
			
			mag.coercivity = (pdmPrivate->nCoercivity == UICBVAL_Coercivity_HiCo) ? COE_HIGH : COE_LOW;  //if user defined TODO?
			mag.start_position = pdmPrivate->EncodeStartPosition;				//0-85000
			mag.encode_length = 0;
			mag.verify_attempts = (pdmPrivate->bPerformVerification) ? 3 : 0;
			mag.encode_dir = (!pdmPrivate->bJIS2Enabled) ? MAG_ENC_FORWARDS : MAG_ENC_BACKWARDS; // assume fwd == ISO, BWD == JIS2
			lead_zero_count = 0;//atoi(buf);  //needed?
            
			if (ulTrack == 1)
			{
				// ISO=7, JIS2=8
				if (!pdmPrivate->bJIS2Enabled)
				{
					if (lpMagTrackInfo->wBitsPerChar == 3)	//UICBVAL_BitsPerChar_Track1_4
						mag.tracks[0].num_bits = mag_encoder_setup_4bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[0].track_data);
					else
                        if (lpMagTrackInfo->wBitsPerChar == 2)	//UICBVAL_BitsPerChar_Track1_1
                            mag.tracks[0].num_bits = mag_encoder_setup_1bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[0].track_data);
                        else
                            if (lpMagTrackInfo->wBitsPerChar == 1)	//UICBVAL_BitsPerChar_Track1_5
                                mag.tracks[0].num_bits = mag_encoder_setup_5bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[0].track_data,_PARITY_EVEN,lead_zero_count);
                            else
                                mag.tracks[0].num_bits = mag_encoder_setup_7bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[0].track_data,lead_zero_count);
				}
				else
					mag.tracks[0].num_bits = mag_encoder_setup_8bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[0].track_data,lead_zero_count);
                
				mag.tracks[0].bits_per_inch = lpMagTrackInfo->wBitsPerInch;//SetBitsPerInch(pdev,ulTrack);
			}
			else
                if (ulTrack == 2)
                {
                    if (lpMagTrackInfo->wBitsPerChar == 3)	//UICBVAL_BitsPerChar_Track2_4
                        mag.tracks[1].num_bits = mag_encoder_setup_4bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[1].track_data);
                    else
                        if (lpMagTrackInfo->wBitsPerChar == 2)	//UICBVAL_BitsPerChar_Track2_1
                            mag.tracks[1].num_bits = mag_encoder_setup_1bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[1].track_data);
                        else
                            if (lpMagTrackInfo->wBitsPerChar == 1)	//UICBVAL_BitsPerChar_Track1_5
                                mag.tracks[1].num_bits = mag_encoder_setup_5bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[1].track_data,_PARITY_EVEN,lead_zero_count);
                            else
                                mag.tracks[1].num_bits = mag_encoder_setup_7bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[1].track_data,lead_zero_count);
					
                    mag.tracks[1].bits_per_inch = lpMagTrackInfo->wBitsPerInch;//SetBitsPerInch(pdev,ulTrack);
                }
                else
                    if (ulTrack == 3)
                    {
                        if (lpMagTrackInfo->wBitsPerChar == 3)	//UICBVAL_BitsPerChar_Track3_4
                            mag.tracks[2].num_bits = mag_encoder_setup_4bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[2].track_data);
                        else
                            if (lpMagTrackInfo->wBitsPerChar == 2)	//UICBVAL_BitsPerChar_Track3_1
                                mag.tracks[2].num_bits = mag_encoder_setup_1bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[2].track_data);
                            else
                                if (lpMagTrackInfo->wBitsPerChar == 1)	//UICBVAL_BitsPerChar_Track1_5
                                    mag.tracks[2].num_bits = mag_encoder_setup_5bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[2].track_data,_PARITY_EVEN,lead_zero_count);
                                else
                                    mag.tracks[2].num_bits = mag_encoder_setup_7bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[2].track_data,lead_zero_count);
                        mag.tracks[2].bits_per_inch = lpMagTrackInfo->wBitsPerInch;//SetBitsPerInch(pdev,ulTrack);
                    }
            
		}
	}
 */
    ////
	//see if any planes actually have data so we know whether to encode only..
	for (ulColour = 0 ; ulColour < 4; ulColour ++)
	{
		lpSplMemFront = &pdev->lpSplMemFront[ulColour];
		lpSplMemBack = &pdev->lpSplMemBack[ulColour];
        
		if (lpSplMemFront->lpBuffer != NULL && lpSplMemFront->bDataInPlane)
			bBlankFrontPage = FALSE;
		
		if (lpSplMemBack->lpBuffer != NULL && lpSplMemBack->bDataInPlane)
			bBlankBackPage = FALSE;
	}
/*
	if ((bSendMagEncodeJob && pdmPrivate->bEncodeOnly) || (bSendMagEncodeJob && bBlankBackPage && bBlankFrontPage))
	{
		// send down a mag encoding job only
		// inform flushbuffer not to send the preview plane
		pdev->bEncodeOnlyJob = TRUE;
		
		memcpy((PBYTE)&jobHeader.magic,(PBYTE)"0PT1MA00",8);
		jobHeader.job_id = (JOB_ID)JOB_PRINT;
		jobHeader.size = sizeof( HDR_JOB ) + sizeof( CARD_OPTIONS ) + sizeof( MAG );
		jobHeader.job_sub_id = POF_MAG_ENCODE;
		jobHeader.version = COMMS_VERSION;
		time((time_t *)&jobHeader.timestamp);
		myWrite(pdev, (PBYTE)&jobHeader, sizeof(HDR_JOB));
        
		//now send a CardOptions struct
		memset ((PBYTE)&cardOptions, 0, sizeof(CARD_OPTIONS));
		cardOptions.num_copies = iCopyCount;
		strcpy_s((PBYTE)&cardOptions.driver_version, 16, DRV_VERSION);
		strcpy_s((PBYTE)&cardOptions.driver_language, 16, "ENG");
		cardOptions.rotate_before_print = 0;
		cardOptions.rotate_before_lam = 0;
		cardOptions.disable_eject = 0;
		cardOptions.side1_options.laminate_target_temperature = 64;
		cardOptions.side2_options.laminate_target_temperature = 64;
        
		myWrite(pdev, (PBYTE)&cardOptions, sizeof(CARD_OPTIONS));
        
		//now the composed mag struct
		myWrite(pdev, (PBYTE)&mag, sizeof(MAG));
		
		return;
        
	}
*/
    ////////////////////
    
    memcpy((PBYTE)&jobHeader.magic,(PBYTE)"0PT1MA00",8);
    
    jobHeader.job_id = (JOB_ID)3;
	jobHeader.size = sizeof(HDR_JOB) + sizeof(CARD_OPTIONS) + ((pdev->bDuplex) ? PREVIEW_SIZE * 2 : PREVIEW_SIZE);
	jobHeader.job_sub_id = (pdev->bDuplex) ? POF_SIDE_1_PREVIEW | POF_SIDE_2_PREVIEW : POF_SIDE_1_PREVIEW;

	if (pdev->bDuplex)
	{
		if (psettings->CF_HoloKote)
		{
			jobHeader.job_sub_id |= POF_SIDE_1O;
		}
		if (psettings->CB_HoloKote)
		{
			jobHeader.job_sub_id |= POF_SIDE_2O;
		}
	}
	else
	{
		if (bFrontPage && psettings->CF_HoloKote)
		{
			jobHeader.job_sub_id |= POF_SIDE_1O;
		}	
		if (bFrontPage == FALSE && psettings->CB_HoloKote)
		{
			jobHeader.job_sub_id |= POF_SIDE_1O;
		}			
	}
    
	if (psettings->ISO7810 && bSendMagEncodeJob == FALSE && pdev->bDuplex == FALSE)
        jobHeader.job_sub_id |= POF_FLATTEN_CARD;
        
	
	/*	if (bSendMagEncodeJob)
	{
		jobHeader.size += sizeof( MAG );
		jobHeader.job_sub_id |= POF_MAG_ENCODE;
	}*/
	//which planes actually have data
	for (ulColour = 0 ; ulColour < 4; ulColour ++)
	{
		lpSplMemFront = &pdev->lpSplMemFront[ulColour];
		lpSplMemBack = &pdev->lpSplMemBack[ulColour];
        
		if (lpSplMemFront->lpBuffer != NULL && lpSplMemFront->bDataInPlane)
		{
            if (ulColour == 0)	//yellow
                jobHeader.job_sub_id |= POF_SIDE_1Y;
            else
				if (ulColour == 1)	//magenta
					jobHeader.job_sub_id |= POF_SIDE_1M;
				else
                    if (ulColour == 2)	//cyan
                        jobHeader.job_sub_id |= POF_SIDE_1C;
                    else
                        jobHeader.job_sub_id |= POF_SIDE_1K;
            
			jobHeader.size += BUF_SIZE_300DPI;
			bBlankFrontPage = FALSE;
		}

		if (lpSplMemBack->lpBuffer != NULL && lpSplMemBack->bDataInPlane)
		{
				if (pdev->bDuplex)
				{
					if (ulColour == 0)	//yellow
						jobHeader.job_sub_id |= POF_SIDE_2Y;
					else
					if (ulColour == 1)	//magenta
						jobHeader.job_sub_id |= POF_SIDE_2M;
					else			
					if (ulColour == 2)	//cyan
						jobHeader.job_sub_id |= POF_SIDE_2C;
					else			
						jobHeader.job_sub_id |= POF_SIDE_2K;	
				}	
				else
				{
					//set the job header to flag side 1 for back only
					if (ulColour == 0)	//yellow
					{
						jobHeader.job_sub_id |= POF_SIDE_1Y;
						bBlankFrontPage = FALSE;
					}
					else
					if (ulColour == 1)	//magenta
					{
						jobHeader.job_sub_id |= POF_SIDE_1M;
						bBlankFrontPage = FALSE;						
					}
					else			
					if (ulColour == 2)	//cyan
					{
						jobHeader.job_sub_id |= POF_SIDE_1C;
						bBlankFrontPage = FALSE;								
					}
					else	
					{						
						jobHeader.job_sub_id |= POF_SIDE_1K;					
						bBlankFrontPage = FALSE;								
					}
					
				}
			jobHeader.size += BUF_SIZE_300DPI;
			bBlankBackPage = FALSE;
		}
	}
	
	//ensure we send something for a blank page
	if (bBlankFrontPage)
	{
		jobHeader.job_sub_id |= POF_SIDE_1K;
		jobHeader.size += BUF_SIZE_300DPI;
	}
	if (pdev->bDuplex && bBlankBackPage)
	{
		jobHeader.job_sub_id |= POF_SIDE_2K;
		jobHeader.size += BUF_SIZE_300DPI;
	}	
    
    jobHeader.version = COMMS_VERSION;
    time((time_t *)&jobHeader.timestamp);
    myWrite(pdev, (char *)&jobHeader, sizeof(HDR_JOB));
    
    //now send a CardOptions struct
    memset ((PBYTE)&cardOptions, 0, sizeof(CARD_OPTIONS));
    cardOptions.num_copies = iCopyCount;
    strcpy((char *)&cardOptions.driver_version, "1.0.9"/*DRV_VERSION*/);
    strcpy((char *)&cardOptions.driver_language, "ENG");
    cardOptions.rotate_before_print = (pdev->bDuplex == FALSE && bFrontPage == FALSE) ? 1 : 0;
    cardOptions.rotate_before_lam = 0;
    cardOptions.disable_eject = 0;
    cardOptions.side1_options.laminate_target_temperature = 64;
    cardOptions.side2_options.laminate_target_temperature = 64;
	
	if (pdev->bDuplex)
	{
		if (psettings->CF_HoloKote)
		{
			 cardOptions.side1_options.holokote_index = psettings->CFHolokoteSlot;
			 cardOptions.side1_options.holokote_orientation = (psettings->CF_SecurityOptions_Rotation) ? 180 : 0;////
		}
		if (psettings->CB_HoloKote)
		{
			cardOptions.side2_options.holokote_index = psettings->CBHolokoteSlot;
			cardOptions.side2_options.holokote_orientation = (psettings->CB_SecurityOptions_Rotation) ? 180 : 0;////
		}
	}	
	else
	{
    	if (bFrontPage && psettings->CF_HoloKote)
		{
			 cardOptions.side1_options.holokote_index = psettings->CFHolokoteSlot;
			 cardOptions.side1_options.holokote_orientation = (psettings->CF_SecurityOptions_Rotation) ? 180 : 0;////
		}
		if (bFrontPage == FALSE && psettings->CB_HoloKote)
		{
			cardOptions.side1_options.holokote_index = psettings->CBHolokoteSlot;
			cardOptions.side1_options.holokote_orientation = (psettings->CB_SecurityOptions_Rotation) ? 180 : 0;////
		}			
		
	}
fprintf(stderr, "L3147:CATULTRA POWERYMC:%d\n",-5000 + (psettings->nHLXPrintHeadPowerYMC * 100));	
	cardOptions.side1_options.power_adjust[PANEL_YELLOW]=
	cardOptions.side1_options.power_adjust[PANEL_MAGENTA]=
	cardOptions.side1_options.power_adjust[PANEL_CYAN]= -5000 + (psettings->nHLXPrintHeadPowerYMC * 100);
	cardOptions.side1_options.power_adjust[PANEL_RESIN]=psettings->nHLXPrintHeadPowerK;
	cardOptions.side1_options.power_adjust[PANEL_OVERCOAT]=psettings->nHLXPrintHeadPowerOvercoat;
	
	cardOptions.side2_options.power_adjust[PANEL_YELLOW]=
	cardOptions.side2_options.power_adjust[PANEL_MAGENTA]=
	cardOptions.side2_options.power_adjust[PANEL_CYAN]=psettings->nHLXPrintHeadPowerYMC;
	cardOptions.side2_options.power_adjust[PANEL_RESIN]=psettings->nHLXPrintHeadPowerK;
	cardOptions.side2_options.power_adjust[PANEL_OVERCOAT]=psettings->nHLXPrintHeadPowerOvercoat;


	cardOptions.side1_options.image_position_adjust_x = 0;
	cardOptions.side1_options.image_position_adjust_x = 0;
	cardOptions.side2_options.image_position_adjust_y = 0;
	cardOptions.side2_options.image_position_adjust_y = 0;
    myWrite(pdev, (char *)&cardOptions, sizeof(CARD_OPTIONS));
/*
	if (bSendMagEncodeJob)
	{	
		//now the composed mag struct
		myWrite(pdev, (PBYTE)&mag, sizeof(MAG));
	}		
  */
}
SHORT SendHole(PDEVDATA pdev, PSETTINGS psettings, BOOL bFront, BOOL bHole2, SHORT indentlevel)
{
char   temp_string[64]={0};	
		myWritecs(pdev,"<hole>\015",0,indentlevel++);
		if (bHole2 == FALSE)
		{
			if (bFront)
			{
				sprintf(temp_string,"<x>%d</x>\015",psettings->CF_FrontHole1_Left );
				myWritecs(pdev,temp_string,0,indentlevel);		
				sprintf(temp_string,"<y>%d</y>\015",pdev->xImage - psettings->CF_FrontHole1_Bottom - psettings->CF_FrontHole1_Height);
				myWritecs(pdev,temp_string,0,indentlevel);	
				sprintf(temp_string,"<width>%d</width>\015",psettings->CF_FrontHole1_Width);
				myWritecs(pdev,temp_string,0,indentlevel);		
				sprintf(temp_string,"<height>%d</height>\015",psettings->CF_FrontHole1_Height);
				myWritecs(pdev,temp_string,0,indentlevel--);		
			}
			else
			{
				sprintf(temp_string,"<x>%d</x>\015",psettings->CB_BackHole1_Left );
				myWritecs(pdev,temp_string,0,indentlevel);		
				sprintf(temp_string,"<y>%d</y>\015",pdev->xImage - psettings->CB_BackHole1_Bottom - psettings->CB_BackHole1_Height);
				myWritecs(pdev,temp_string,0,indentlevel);	
				sprintf(temp_string,"<width>%d</width>\015",psettings->CB_BackHole1_Width);
				myWritecs(pdev,temp_string,0,indentlevel);		
				sprintf(temp_string,"<height>%d</height>\015",psettings->CB_BackHole1_Height);
				myWritecs(pdev,temp_string,0,indentlevel--);		
			}
		}
		else
		{
			if (bFront)
			{
				sprintf(temp_string,"<x>%d</x>\015",psettings->CF_FrontHole2_Left );
				myWritecs(pdev,temp_string,0,indentlevel);		
				sprintf(temp_string,"<y>%d</y>\015",pdev->xImage - psettings->CF_FrontHole2_Bottom - psettings->CF_FrontHole2_Height);
				myWritecs(pdev,temp_string,0,indentlevel);	
				sprintf(temp_string,"<width>%d</width>\015",psettings->CF_FrontHole2_Width);
				myWritecs(pdev,temp_string,0,indentlevel);		
				sprintf(temp_string,"<height>%d</height>\015",psettings->CF_FrontHole2_Height);
				myWritecs(pdev,temp_string,0,indentlevel--);		
			}
			else
			{
				sprintf(temp_string,"<x>%d</x>\015",psettings->CB_BackHole2_Left );
				myWritecs(pdev,temp_string,0,indentlevel);		
				sprintf(temp_string,"<y>%d</y>\015",pdev->xImage - psettings->CB_BackHole2_Bottom - psettings->CB_BackHole2_Height);
				myWritecs(pdev,temp_string,0,indentlevel);	
				sprintf(temp_string,"<width>%d</width>\015",psettings->CB_BackHole2_Width);
				myWritecs(pdev,temp_string,0,indentlevel);		
				sprintf(temp_string,"<height>%d</height>\015",psettings->CB_BackHole2_Height);
				myWritecs(pdev,temp_string,0,indentlevel--);		
			}
		}
			
		myWritecs(pdev,"</hole>\015",0,indentlevel);							
	return indentlevel;
}	

VOID OutputPro360HeaderCommand
(
    PDEVDATA pdev,     // Pointer to our PDEV
	cups_page_header2_t header,
	PSETTINGS psettings,
    BOOL     bFrontPage // TRUE = Front page (simplex or duplex); FALSE = Back page (simplex or duplex)
)
{
    /* PRIVATEDEVMODE *pdmPrivate = (PRIVATEDEVMODE *)GetPrivateDevMode((PDEVMODE)&pdev->dm.dmPublic);
    PRIVATEDEVMODE ldmPrivate = *pdmPrivate; */
    HDR_JOB jobHeader;
    //CARD_OPTIONS    cardOptions;
    MAG          mag  = {0};	
	//int          lead_zero_count = 0;	//?needed?
    LPSPOOLMEMINFO lpSplMemFront   = NULL;
    LPSPOOLMEMINFO lpSplMemBack    = NULL;
	
    int  iCopyCount        = header.NumCopies;
    int icx = (psettings->pageType == 2) ? 991 : 1013;

    //BOOL bOverCoat         = FALSE;
    //BOOL bUseWithLaminate  = FALSE;
    //BOOL bHoloKote         = FALSE;
    //int  iHoloKoteImage    = 0;
    //int  iHoloKoteMap      = 0; 
    //BOOL bCustomKeyDisable = FALSE;
    //BOOL bHoloPatch        = FALSE;
    //BOOL bHoloPatchHole    = FALSE;
    //BOOL bArea_UserDefined = FALSE;
    BOOL bHole_MagNormal   = FALSE;
    BOOL bHole_MagWide     = FALSE;
    BOOL bHole_ChipNormal  = FALSE;
    BOOL bHole_ChipLarge   = FALSE;
    //BOOL bHole_UserDefined = FALSE; 
    //int  iTileRotate       = 0;
    //BOOL bMagUserDefined   = (psettings->nTrackOptions == UICBVAL_Encoding_UserDefined);
	//time_t ltime;
	//char  tchLangID[MAX_LANGID + 1];
	char   temp_string[256];
	//char  wsTemp[BUFSIZE];
	int    ulColour;
	int icy 	 = 615;//OUTPUT_DATA_WIDTH_PRO360_CR79; //cr79 height 615

	//DWORD  dwPowerTemp;
	//LPMAGTRACKINFO lpMagTrackInfo = NULL;
	//int density;
	BOOL	bBlankFrontPage		= TRUE;
	BOOL	bBlankBackPage		= TRUE;	
	BOOL    bSendMagEncodeJob   = FALSE;
	//ULONG          ulTrack        = 0;
    //
	/* int iImageFormatFront = GetChannelOption(pdev, psettings, FRONT);	
	int iImageFormatBack = GetChannelOption(pdev, psettings, BACK); */
	
	SHORT indentlevel = 0;
    //ensure message dll is loaded
   /*  GetModuleFileName(pdev->hModule, DllName, MAX_PATH);
    if (!CheckLangLoad(DllName,(PDEVMODE)&pdev->dm))
        return; */
   
/////////////////////
	//
	
	//initialisation
	/* pdev->bEncodeOnlyJob = FALSE;
   // Initialise the lookup tables
	
	for (ulTrack = FIRST_MAG_TRACK; ulTrack <= LAST_MAG_TRACK; ulTrack++)
	{
		lpMagTrackInfo = &pdev->magTrackInfo[ulTrack];

		if (lpMagTrackInfo->TrackData[0] != 0)
		{
			bSendMagEncodeJob   = TRUE; */
			
			//mag.start_position = psettings->EncodeStartPosition;				//0-85000
			/* mag.encode_length = 0;
			mag.verify_attempts = (psettings->bPerformVerification) ? 3 : 0;
			mag.encode_dir = (!settings->bJIS2Enabled) ? MAG_ENC_FORWARDS : MAG_ENC_BACKWARDS; */ // assume fwd == ISO, BWD == JIS2
/* 			lead_zero_count = 0;//atoi(buf);  //needed?

			if (ulTrack == 1)
			{
				// ISO=7, JIS2=8
				if (!psettings->bJIS2Enabled)
				{
					if (lpMagTrackInfo->wBitsPerChar == 3)	//UICBVAL_BitsPerChar_Track1_4
						mag.tracks[0].num_bits = mag_encoder_setup_4bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[0].track_data);
					else				
					if (lpMagTrackInfo->wBitsPerChar == 2)	//UICBVAL_BitsPerChar_Track1_1
						mag.tracks[0].num_bits = mag_encoder_setup_1bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[0].track_data);
					else
					if (lpMagTrackInfo->wBitsPerChar == 1)	//UICBVAL_BitsPerChar_Track1_5
						mag.tracks[0].num_bits = mag_encoder_setup_5bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[0].track_data,_PARITY_EVEN,lead_zero_count);
					else
						mag.tracks[0].num_bits = mag_encoder_setup_7bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[0].track_data,lead_zero_count);
				}
				else
					mag.tracks[0].num_bits = mag_encoder_setup_8bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[0].track_data,lead_zero_count);
					
				mag.tracks[0].bits_per_inch = lpMagTrackInfo->wBitsPerInch;//SetBitsPerInch(pdev,ulTrack);
				mag.coercivity = lpMagTrackInfo->wCoercivity;

			}
			else
			if (ulTrack == 2)
			{	
				if (lpMagTrackInfo->wBitsPerChar == 3)	//UICBVAL_BitsPerChar_Track2_4
					mag.tracks[1].num_bits = mag_encoder_setup_4bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[1].track_data);
				else				
				if (lpMagTrackInfo->wBitsPerChar == 2)	//UICBVAL_BitsPerChar_Track2_1
					mag.tracks[1].num_bits = mag_encoder_setup_1bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[1].track_data);
				else			
				if (lpMagTrackInfo->wBitsPerChar == 1)	//UICBVAL_BitsPerChar_Track1_5
					mag.tracks[1].num_bits = mag_encoder_setup_5bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[1].track_data,_PARITY_EVEN,lead_zero_count);
				else
					mag.tracks[1].num_bits = mag_encoder_setup_7bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[1].track_data,lead_zero_count);
					
				mag.tracks[1].bits_per_inch = lpMagTrackInfo->wBitsPerInch;//SetBitsPerInch(pdev,ulTrack);
				mag.coercivity = lpMagTrackInfo->wCoercivity;				
			}
			else
			if (ulTrack == 3)
			{
				if (lpMagTrackInfo->wBitsPerChar == 3)	//UICBVAL_BitsPerChar_Track3_4
					mag.tracks[2].num_bits = mag_encoder_setup_4bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[2].track_data);
				else				
				if (lpMagTrackInfo->wBitsPerChar == 2)	//UICBVAL_BitsPerChar_Track3_1
					mag.tracks[2].num_bits = mag_encoder_setup_1bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[2].track_data);
				else			
				if (lpMagTrackInfo->wBitsPerChar == 1)	//UICBVAL_BitsPerChar_Track1_5
					mag.tracks[2].num_bits = mag_encoder_setup_5bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[2].track_data,_PARITY_EVEN,lead_zero_count);
				else
					mag.tracks[2].num_bits = mag_encoder_setup_7bit_data(&lpMagTrackInfo->TrackData[2],(char *)mag.tracks[2].track_data,lead_zero_count);
				mag.tracks[2].bits_per_inch = lpMagTrackInfo->wBitsPerInch;//SetBitsPerInch(pdev,ulTrack);	
				mag.coercivity = lpMagTrackInfo->wCoercivity;				
			}

		}
	}  */
	//see if any planes actually have data so we know whether to encode only..
	for (ulColour = 0 ; ulColour < 4; ulColour ++)
	{
		lpSplMemFront = &pdev->lpSplMemFront[ulColour];
		lpSplMemBack = &pdev->lpSplMemBack[ulColour];

		if (lpSplMemFront->lpBuffer != NULL && lpSplMemFront->bDataInPlane)
			bBlankFrontPage = FALSE;
		
		if (lpSplMemBack->lpBuffer != NULL && lpSplMemBack->bDataInPlane)
			bBlankBackPage = FALSE;
	}			

	/* if ((bSendMagEncodeJob && settings->bEncodeOnly) || (bSendMagEncodeJob && bBlankBackPage && bBlankFrontPage))
	{
		// send down a mag encoding job only
		// inform flushbuffer not to send the preview plane
		pdev->bEncodeOnlyJob = TRUE;
#if 0			
		memcpy((PBYTE)&jobHeader.magic,(PBYTE)"0PT1MA00",8);
		jobHeader.job_id = (JOB_ID)JOB_PRINT;	
		jobHeader.size = sizeof( HDR_JOB ) + sizeof( CARD_OPTIONS ) + sizeof( MAG );	
		jobHeader.job_sub_id = POF_MAG_ENCODE;
		jobHeader.version = COMMS_VERSION;
		time((time_t *)&jobHeader.timestamp);
		myWrite(pdev, (PBYTE)&jobHeader, sizeof(HDR_JOB));

		//now send a CardOptions struct
		memset ((PBYTE)&cardOptions, 0, sizeof(CARD_OPTIONS));
		cardOptions.num_copies = iCopyCount;
		strcpy_s((PBYTE)&cardOptions.driver_version, 16, DRV_VERSION);
		strcpy_s((PBYTE)&cardOptions.driver_language, 16, "ENG");
		cardOptions.rotate_before_print = 0;
		cardOptions.rotate_before_lam = 0;
		cardOptions.disable_eject = 0;
		cardOptions.side1_options.laminate_target_temperature = 64;
		cardOptions.side2_options.laminate_target_temperature = 64;

		myWrite(pdev, (PBYTE)&cardOptions, sizeof(CARD_OPTIONS));

		//now the composed mag struct
		myWrite(pdev, (PBYTE)&mag, sizeof(MAG));
		
		return;
#endif			
	} */

////////////////////  
	pdev->epjobSize = 0;  
	memset ((PBYTE)&jobHeader, 0, sizeof(HDR_JOB));

	if (pdev->bEncodeOnlyJob == FALSE)
	{
		if (pdev->bDuplex)
		{
			if ((psettings->CF_HoloKote && psettings->CF_PrintOvercoat) || (psettings->CF_HoloKote == FALSE && psettings->CF_PrintOvercoat))
			{
				jobHeader.job_sub_id |= POF_SIDE_1O;
			}
			if ((psettings->CB_HoloKote && psettings->CB_PrintOvercoat) || (psettings->CB_HoloKote == FALSE && psettings->CB_PrintOvercoat))
			{
				jobHeader.job_sub_id |= POF_SIDE_2O;
			}
		}
		else
		{
			if (bFrontPage && psettings->Duplex == UICBVAL_Duplex_Back)
			{ 
				if ((psettings->CB_HoloKote && psettings->CB_PrintOvercoat) || (psettings->CB_HoloKote == FALSE && psettings->CB_PrintOvercoat))
				{
					jobHeader.job_sub_id |= POF_SIDE_2O;
				}
			}
			else
			{
				if (bFrontPage && 
					((psettings->CF_HoloKote && psettings->CF_PrintOvercoat) || (psettings->CF_HoloKote == FALSE && psettings->CF_PrintOvercoat)))
				{
					jobHeader.job_sub_id |= POF_SIDE_1O;
				}	
				if (bFrontPage == FALSE && 
					((psettings->CB_HoloKote && psettings->CB_PrintOvercoat) || (psettings->CB_HoloKote == FALSE && psettings->CB_PrintOvercoat)))
				{
					jobHeader.job_sub_id |= POF_SIDE_2O;
				}	
			}
			
		}
		
		// if (bSendMagEncodeJob)
		// {
			// jobHeader.size += sizeof( MAG );
			// jobHeader.job_sub_id |= POF_MAG_ENCODE;
		// }

		//which planes actually have data 
		for (ulColour = 0 ; ulColour < 4; ulColour ++)
		{
			lpSplMemFront = &pdev->lpSplMemFront[ulColour];
			lpSplMemBack = &pdev->lpSplMemBack[ulColour];

			if (lpSplMemFront->lpBuffer != NULL && lpSplMemFront->bDataInPlane)
			{
					if (ulColour == 0)	//yellow
						jobHeader.job_sub_id |= POF_SIDE_1Y;
					else
					if (ulColour == 1)	//magenta
						jobHeader.job_sub_id |= POF_SIDE_1M;
					else			
					if (ulColour == 2)	//cyan
						jobHeader.job_sub_id |= POF_SIDE_1C;
					else			
						jobHeader.job_sub_id |= POF_SIDE_1K;	
						
				pdev->epjobSize += pdev->xImage * pdev->yImage;//PRO360_BUF_SIZE_300DPI;
				bBlankFrontPage = FALSE;
			}
			
			if (lpSplMemBack->lpBuffer != NULL && lpSplMemBack->bDataInPlane)
			{
					if (pdev->bDuplex)
					{
						if (ulColour == 0)	//yellow
							jobHeader.job_sub_id |= POF_SIDE_2Y;
						else
						if (ulColour == 1)	//magenta
							jobHeader.job_sub_id |= POF_SIDE_2M;
						else			
						if (ulColour == 2)	//cyan
							jobHeader.job_sub_id |= POF_SIDE_2C;
						else			
							jobHeader.job_sub_id |= POF_SIDE_2K;	
					}	
					else
					{
						//set the job header to flag side 1 for back only
						if (ulColour == 0)	//yellow
						{
							jobHeader.job_sub_id |= POF_SIDE_1Y;
							bBlankFrontPage = FALSE;
						}
						else
						if (ulColour == 1)	//magenta
						{
							jobHeader.job_sub_id |= POF_SIDE_1M;
							bBlankFrontPage = FALSE;						
						}
						else			
						if (ulColour == 2)	//cyan
						{
							jobHeader.job_sub_id |= POF_SIDE_1C;
							bBlankFrontPage = FALSE;								
						}
						else	
						{						
							jobHeader.job_sub_id |= POF_SIDE_1K;					
							bBlankFrontPage = FALSE;								
						}
						
					}
				pdev->epjobSize += pdev->xImage * pdev->yImage;//PRO360_BUF_SIZE_300DPI;
				bBlankBackPage = FALSE;
			}		
		}
		
		//ensure we send something for a blank page
		if (bBlankFrontPage)
		{
			jobHeader.job_sub_id |= POF_SIDE_1K;
			pdev->epjobSize += pdev->xImage * pdev->yImage;//PRO360_BUF_SIZE_300DPI;
		}
		if (pdev->bDuplex && bBlankBackPage)
		{
			jobHeader.job_sub_id |= POF_SIDE_2K;
			pdev->epjobSize += pdev->xImage * pdev->yImage;//PRO360_BUF_SIZE_300DPI;
		}	
	}	
	
	myWritecs(pdev,"<?xml version=\042\061.0\042 encoding=\042UTF-8\042?>\015",0,indentlevel);
	myWritecs(pdev,"<prn>\015",0,indentlevel++);
	myWritecs(pdev,"<card>\015",0,indentlevel++);

	if (pdev->bEncodeOnlyJob)
		myWritecs(pdev,"<card-options />\015",0,indentlevel);
	else
	{
		myWritecs(pdev,"<card-options>\015",0,indentlevel++);	
		memset((PBYTE)&temp_string,0, 64);
		sprintf(temp_string,"<num-copies>%u</num-copies>\015",iCopyCount);
		myWritecs(pdev,temp_string,0,indentlevel);
		memset((PBYTE)&temp_string,0, 64);

		if (PRO360_XTD_TYPE(psettings))
		{
			icx = (int)pdev->yImage;
			//0 - cr80
			//1- cr80dup
			//2 - ext col
			//3 - ext mono
			if (psettings->XXL_ImageType >= UICBVAL_DoubleImage)
				sprintf(temp_string,"<card-size>extended</card-size>\015");
			else
				sprintf(temp_string,"<card-size>cr80</card-size>\015");
		}
		else	
		{
			//Insert the Card Size if it is CR79
			if (psettings->pageType == 2)
				sprintf(temp_string,"<card-size>cr79</card-size>\015");
			else
				sprintf(temp_string,"<card-size>cr80</card-size>\015");
		}
		myWritecs(pdev,temp_string,0,indentlevel);
		
		memset((PBYTE)&temp_string,0, 64);
		sprintf(temp_string,"<width>%u</width>\015",icx);
		myWritecs(pdev,temp_string,0,indentlevel);	
		if (psettings->pageType == 2)
		{
			sprintf(temp_string,"<height>%u</height>\015",icy);
			myWritecs(pdev,temp_string,0,indentlevel);	
		}		
		indentlevel--;		
		myWritecs(pdev,"</card-options>\015",0,indentlevel);	
	
	}
	if (pdev->bEncodeOnlyJob == FALSE)
	{
		if (bFrontPage && psettings->Duplex != UICBVAL_Duplex_Back)
		{
			myWritecs(pdev,"<side-options side=\042\060\042>\015",0,indentlevel++);
			if (jobHeader.job_sub_id & POF_SIDE_1Y)
			{
				myWritecs(pdev,"<panel-options panel=\042y\042>\015",0,indentlevel++);
				memset((PBYTE)&temp_string,0, 64);
				sprintf(temp_string,"<power-adjust>%d</power-adjust>\015",(psettings->nPrintHeadPower_YMC - 50) * 655);
				myWritecs(pdev,temp_string,0,indentlevel--);	
				myWritecs(pdev,"</panel-options>\015",0,indentlevel);
			}
			if (jobHeader.job_sub_id & POF_SIDE_1M)
			{
				myWritecs(pdev,"<panel-options panel=\042m\042>\015",0,indentlevel++);
				memset((PBYTE)&temp_string,0, 64);
				sprintf(temp_string,"<power-adjust>%d</power-adjust>\015",(psettings->nPrintHeadPower_YMC - 50) * 655);
				myWritecs(pdev,temp_string,0,indentlevel--);	
				myWritecs(pdev,"</panel-options>\015",0,indentlevel);
			}
			if (jobHeader.job_sub_id & POF_SIDE_1C)
			{
				myWritecs(pdev,"<panel-options panel=\042c\042>\015",0,indentlevel++);
				memset((PBYTE)&temp_string,0, 64);
				sprintf(temp_string,"<power-adjust>%d</power-adjust>\015",(psettings->nPrintHeadPower_YMC - 50) * 655);
				myWritecs(pdev,temp_string,0,indentlevel--);	
				myWritecs(pdev,"</panel-options>\015",0,indentlevel);
			}
			if (jobHeader.job_sub_id & POF_SIDE_1K)
			{
				myWritecs(pdev,"<panel-options panel=\042k\042>\015",0,indentlevel++);
				memset((PBYTE)&temp_string,0, 64);
				sprintf(temp_string,"<power-adjust>%d</power-adjust>\015",(psettings->bEraseBeforePrint) ? (psettings->WritePowerAdjustment - 50) * 655: (psettings->nPrintHeadPower_BlackResin - 50) * 655);
				myWritecs(pdev,temp_string,0,indentlevel);
/*				if (psettings->bEraseBeforePrint)
				{
					for (i=0; i < psettings->CF_NoRewritableAreas && i < MAX_REWRITABLE_AREAS; i++)
					{
						if (psettings->CF_RewritableAreas[i].left == 0 && 
							psettings->CF_RewritableAreas[i].bottom == 0 && 
							psettings->CF_RewritableAreas[i].width == 0 && 
							psettings->CF_RewritableAreas[i].height ==0 )
							continue;					
							myWritecs(pdev,"<area>\015",0,indentlevel++);
									
							sprintf(temp_string,"<x>%d</x>\015",psettings->CF_RewritableAreas[i].left );
							myWritecs(pdev,temp_string,0,indentlevel);		
							sprintf(temp_string,"<y>%d</y>\015",pdev->xImage - psettings->CF_RewritableAreas[i].bottom - psettings->CF_RewritableAreas[i].height);
							myWritecs(pdev,temp_string,0,indentlevel);	
							sprintf(temp_string,"<width>%d</width>\015",psettings->CF_RewritableAreas[i].width);
							myWritecs(pdev,temp_string,0,indentlevel);		
							sprintf(temp_string,"<height>%d</height>\015",psettings->CF_RewritableAreas[i].height);
							myWritecs(pdev,temp_string,0,indentlevel--);		
							
							myWritecs(pdev,"</area>\015",0,indentlevel);
					}
				}
*/				
				myWritecs(pdev,"</panel-options>\015",0,--indentlevel);
			}
			if (jobHeader.job_sub_id & POF_SIDE_1O)
			{
				myWritecs(pdev,"<panel-options panel=\042o\042>\015",0,indentlevel++);
				memset((PBYTE)&temp_string,0, 64);
				sprintf(temp_string,"<x-adjust>%d</x-adjust>\015", psettings->CF_SecurityOptions_HoloKote_XAdjust);
				myWritecs(pdev,temp_string,0,indentlevel);				
				sprintf(temp_string,"<y-adjust>%d</y-adjust>\015", psettings->CF_SecurityOptions_HoloKote_YAdjust );
				myWritecs(pdev,temp_string,0,indentlevel);				
				sprintf(temp_string,"<power-adjust>%d</power-adjust>\015",(psettings->nPrintHeadPower_Overcoat - 50) * 655);
				myWritecs(pdev,temp_string,0,indentlevel);	
				if (psettings->CF_SecurityOptions_Rotation)
				{
					sprintf(temp_string,"<watermark-rotation>180</watermark-rotation>\015");
					myWritecs(pdev,temp_string,0,indentlevel);
				}
				if (psettings->CF_SecurityOptions_UsewithLaminate)
				{
					sprintf(temp_string,"<watermark-secure-shield>true</watermark-secure-shield>\015");
					myWritecs(pdev,temp_string,0,indentlevel);
				}			
				if (psettings->CF_HoloKote && psettings->CF_PrintOvercoat)
				{
					sprintf(temp_string,"<watermark-index>%d</watermark-index>\015",psettings->CFHolokoteSlot );
					myWritecs(pdev,temp_string,0,indentlevel);		
				}

				if (psettings->CF_HoloKote || psettings->CF_PrintOvercoat)
				{
					if (psettings->CF_OvercoatOptions_bUserDefined)
					{
						if (AreaHoleActive(pdev, psettings, FRONT_AREA1))
						{
							myWritecs(pdev,"<area>\015",0,indentlevel++);
									
							sprintf(temp_string,"<x>%d</x>\015",psettings->CF_FrontArea1_Left );
							myWritecs(pdev,temp_string,0,indentlevel);		
							sprintf(temp_string,"<y>%d</y>\015",pdev->xImage - psettings->CF_FrontArea1_Bottom - psettings->CF_FrontArea1_Height);
							myWritecs(pdev,temp_string,0,indentlevel);	
							sprintf(temp_string,"<width>%d</width>\015",psettings->CF_FrontArea1_Width);
							myWritecs(pdev,temp_string,0,indentlevel);		
							sprintf(temp_string,"<height>%d</height>\015",psettings->CF_FrontArea1_Height);
							myWritecs(pdev,temp_string,0,indentlevel--);		
							
							myWritecs(pdev,"</area>\015",0,indentlevel);
						}
						if (AreaHoleActive(pdev, psettings, FRONT_AREA2))
						{
							myWritecs(pdev,"<area>\015",0,indentlevel++);
									
							sprintf(temp_string,"<x>%d</x>\015",psettings->CF_FrontArea2_Left );
							myWritecs(pdev,temp_string,0,indentlevel);		
							sprintf(temp_string,"<y>%d</y>\015",pdev->xImage - psettings->CF_FrontArea2_Bottom - psettings->CF_FrontArea2_Height);
							myWritecs(pdev,temp_string,0,indentlevel);	
							sprintf(temp_string,"<width>%d</width>\015",psettings->CF_FrontArea2_Width);
							myWritecs(pdev,temp_string,0,indentlevel);		
							sprintf(temp_string,"<height>%d</height>\015",psettings->CF_FrontArea2_Height);
							myWritecs(pdev,temp_string,0,indentlevel--);		
							
							myWritecs(pdev,"</area>\015",0,indentlevel);
						}
					}
					switch (psettings->CF_OvercoatOptions_Holes)
					{
						case UICBVAL_MagStripeNormal: bHole_MagNormal   = TRUE; break;
						case UICBVAL_MagStripeWide:   bHole_MagWide     = TRUE; break;
						case UICBVAL_ChipNormal:      bHole_ChipNormal  = TRUE; break;
						case UICBVAL_ChipLarge:       bHole_ChipLarge   = TRUE; break;
						//case UICBVAL_UserDefined:     bHole_UserDefined = TRUE; break;
					}

					if (bHole_MagNormal)
					{
						psettings->CF_FrontHole1_Left = 0;
						psettings->CF_FrontHole1_Bottom = 420;
						psettings->CF_FrontHole1_Width = 1012;
						psettings->CF_FrontHole1_Height = 170;
						
						indentlevel = SendHole(pdev, psettings, TRUE, FALSE, indentlevel);
					}
					else if (bHole_MagWide)
					{
						psettings->CF_FrontHole1_Left = 0;
						psettings->CF_FrontHole1_Bottom = 400;
						psettings->CF_FrontHole1_Width = 1012;
						psettings->CF_FrontHole1_Height = 200;
						
						indentlevel = SendHole(pdev, psettings, TRUE, FALSE, indentlevel);
					}
					else if (bHole_ChipNormal)
					{
						psettings->CF_FrontHole1_Left = 106;
						psettings->CF_FrontHole1_Bottom = 310;
						psettings->CF_FrontHole1_Width = 136;
						psettings->CF_FrontHole1_Height = 112;
						
						indentlevel = SendHole(pdev, psettings, TRUE, FALSE, indentlevel);

					}
					else if (bHole_ChipLarge)
					{
						psettings->CF_FrontHole1_Left = 96;
						psettings->CF_FrontHole1_Bottom = 288;
						psettings->CF_FrontHole1_Width = 162;
						psettings->CF_FrontHole1_Height = 144;
						
						indentlevel = SendHole(pdev, psettings, TRUE, FALSE, indentlevel);
					}
/*					else if (bHole_UserDefined)
					{
						if (AreaHoleActive(pdev, FRONT_HOLE1))
							indentlevel = SendHole(pdev, psettings, TRUE, FALSE, indentlevel);
						
						if (AreaHoleActive(pdev, FRONT_HOLE2))
							indentlevel = SendHole(pdev, psettings, TRUE, TRUE, indentlevel);
					}	*/					
				}
				indentlevel--;
				myWritecs(pdev,"</panel-options>\015",0,indentlevel);
			}
			if (psettings->CF_HoloPatch && psettings->bEraseBeforePrint == FALSE)
			{
				myWritecs(pdev,"<patch-options>\015",0,indentlevel++);
						
				sprintf(temp_string,"<x>%d</x>\015",psettings->CF_SecurityOptions_HoloPatchOffset_Left );
				myWritecs(pdev,temp_string,0,indentlevel);		
				sprintf(temp_string,"<y>%d</y>\015",pdev->xImage - psettings->CF_SecurityOptions_HoloPatchOffset_Bottom - psettings->CF_SecurityOptions_HoloPatchOffset_Height);
				myWritecs(pdev,temp_string,0,indentlevel);	
				sprintf(temp_string,"<width>%d</width>\015",psettings->CF_SecurityOptions_HoloPatchOffset_Width );
				myWritecs(pdev,temp_string,0,indentlevel);		
				sprintf(temp_string,"<height>%d</height>\015",psettings->CF_SecurityOptions_HoloPatchOffset_Height);
				myWritecs(pdev,temp_string,0,indentlevel--);		
				
				myWritecs(pdev,"</patch-options>\015",0,indentlevel);
				
			}
			
			indentlevel--;
			myWritecs(pdev,"</side-options>\015",0,indentlevel--);
		}
		
		if (pdev->bDuplex || psettings->Duplex == UICBVAL_Duplex_Back)
		{
			indentlevel++;
			myWritecs(pdev,"<side-options side=\042\061\042>\015",0,indentlevel++);
			if (jobHeader.job_sub_id & POF_SIDE_2Y)
			{
				myWritecs(pdev,"<panel-options panel=\042y\042>\015",0,indentlevel++);
				memset((PBYTE)&temp_string,0, 64);
				sprintf(temp_string,"<power-adjust>%d</power-adjust>\015",psettings->nPrintHeadPower_YMC * 1000);
				myWritecs(pdev,temp_string,0,indentlevel--);	
				myWritecs(pdev,"</panel-options>\015",0,indentlevel);
			}
			if (jobHeader.job_sub_id & POF_SIDE_2M)
			{
				myWritecs(pdev,"<panel-options panel=\042m\042>\015",0,indentlevel++);
				memset((PBYTE)&temp_string,0, 64);
				sprintf(temp_string,"<power-adjust>%d</power-adjust>\015",psettings->nPrintHeadPower_YMC * 1000 );
				myWritecs(pdev,temp_string,0,indentlevel--);	
				myWritecs(pdev,"</panel-options>\015",0,indentlevel);
			}
			if (jobHeader.job_sub_id & POF_SIDE_2C)
			{
				myWritecs(pdev,"<panel-options panel=\042c\042>\015",0,indentlevel++);
				memset((PBYTE)&temp_string,0, 64);
				sprintf(temp_string,"<power-adjust>%d</power-adjust>\015",psettings->nPrintHeadPower_YMC * 1000);
				myWritecs(pdev,temp_string,0,indentlevel--);	
				myWritecs(pdev,"</panel-options>\015",0,indentlevel);
			}
			if (jobHeader.job_sub_id & POF_SIDE_2K)
			{
				myWritecs(pdev,"<panel-options panel=\042k\042>\015",0,indentlevel++);
				memset((PBYTE)&temp_string,0, 64);
				sprintf(temp_string,"<power-adjust>%d</power-adjust>\015",(psettings->bEraseBeforePrint) ? psettings->WritePowerAdjustment * 1000: psettings->nPrintHeadPower_BlackResin * 1000);
				myWritecs(pdev,temp_string,0,indentlevel);	
/*				if (psettings->bEraseBeforePrint)
				{
					for (i=0; i < psettings->CB_NoRewritableAreas && i < MAX_REWRITABLE_AREAS; i++)
					{
						if (psettings->CB_RewritableAreas[i].left == 0 && 
							psettings->CB_RewritableAreas[i].bottom == 0 && 
							psettings->CB_RewritableAreas[i].width == 0 && 
							psettings->CB_RewritableAreas[i].height ==0 )
							continue;					
							myWritecs(pdev,"<area>\015",0,indentlevel++);
									
							sprintf(temp_string,"<x>%d</x>\015",psettings->CB_RewritableAreas[i].left );
							myWritecs(pdev,temp_string,0,indentlevel);		
							sprintf(temp_string,"<y>%d</y>\015",pdev->xImage - psettings->CB_RewritableAreas[i].bottom - psettings->CF_RewritableAreas[i].height);
							myWritecs(pdev,temp_string,0,indentlevel);	
							sprintf(temp_string,"<width>%d</width>\015",psettings->CB_RewritableAreas[i].width);
							myWritecs(pdev,temp_string,0,indentlevel);		
							sprintf(temp_string,"<height>%d</height>\015",psettings->CB_RewritableAreas[i].height);
							myWritecs(pdev,temp_string,0,indentlevel--);		
							
							myWritecs(pdev,"</area>\015",0,indentlevel);
					}
				}
*/
				myWritecs(pdev,"</panel-options>\015",0,--indentlevel);
			}
			if (jobHeader.job_sub_id & POF_SIDE_2O)
			{
				myWritecs(pdev,"<panel-options panel=\042o\042>\015",0,indentlevel++);
				memset((PBYTE)&temp_string,0, 64);
				sprintf(temp_string,"<x-adjust>%d</x-adjust>\015", psettings->CB_SecurityOptions_HoloKote_XAdjust);
				myWritecs(pdev,temp_string,0,indentlevel);				
				sprintf(temp_string,"<y-adjust>%d</y-adjust>\015", psettings->CB_SecurityOptions_HoloKote_YAdjust );
				myWritecs(pdev,temp_string,0,indentlevel);				
				sprintf(temp_string,"<power-adjust>%d</power-adjust>\015",(psettings->nPrintHeadPower_Overcoat - 50) * 655);
				myWritecs(pdev,temp_string,0,indentlevel);
				if (psettings->CB_SecurityOptions_Rotation)
				{
					sprintf(temp_string,"<watermark-rotation>180</watermark-rotation>\015");
					myWritecs(pdev,temp_string,0,indentlevel);
				}
				if (psettings->CB_SecurityOptions_UsewithLaminate)
				{
					sprintf(temp_string,"<watermark-secure-shield>true</watermark-secure-shield>\015");
					myWritecs(pdev,temp_string,0,indentlevel);
				}				
				if (psettings->CB_HoloKote && psettings->CB_PrintOvercoat)
				{
					sprintf(temp_string,"<watermark-index>%d</watermark-index>\015",psettings->CBHolokoteSlot );
					myWritecs(pdev,temp_string,0,indentlevel);
				}
				if (psettings->CB_HoloKote || psettings->CB_PrintOvercoat)
				{
					if (psettings->CB_OvercoatOptions_bUserDefined)
					{
						if (AreaHoleActive(pdev, psettings, BACK_AREA1))
						{
							myWritecs(pdev,"<area>\015",0,indentlevel++);
									
							sprintf(temp_string,"<x>%d</x>\015",psettings->CB_BackArea1_Left );
							myWritecs(pdev,temp_string,0,indentlevel);		
							sprintf(temp_string,"<y>%d</y>\015",pdev->xImage - psettings->CB_BackArea1_Bottom - psettings->CB_BackArea1_Height);
							myWritecs(pdev,temp_string,0,indentlevel);	
							sprintf(temp_string,"<width>%d</width>\015",psettings->CB_BackArea1_Width);
							myWritecs(pdev,temp_string,0,indentlevel);		
							sprintf(temp_string,"<height>%d</height>\015",psettings->CB_BackArea1_Height);
							myWritecs(pdev,temp_string,0,indentlevel--);		
							
							myWritecs(pdev,"</area>\015",0,indentlevel);
						}
						if (AreaHoleActive(pdev, psettings, BACK_AREA1))
						{
							myWritecs(pdev,"<area>\015",0,indentlevel++);
									
							sprintf(temp_string,"<x>%d</x>\015",psettings->CB_BackArea2_Left );
							myWritecs(pdev,temp_string,0,indentlevel);		
							sprintf(temp_string,"<y>%d</y>\015",pdev->xImage - psettings->CB_BackArea2_Bottom - psettings->CB_BackArea2_Height);
							myWritecs(pdev,temp_string,0,indentlevel);	
							sprintf(temp_string,"<width>%d</width>\015",psettings->CB_BackArea2_Width);
							myWritecs(pdev,temp_string,0,indentlevel);		
							sprintf(temp_string,"<height>%d</height>\015",psettings->CB_BackArea2_Height);
							myWritecs(pdev,temp_string,0,indentlevel--);		
							
							myWritecs(pdev,"</area>\015",0,indentlevel);
						}
					}
					bHole_MagNormal = FALSE;
					bHole_MagWide = FALSE;
					bHole_ChipNormal = FALSE;
					bHole_ChipLarge = FALSE;
					//bHole_UserDefined = FALSE;
					
					switch (psettings->CB_OvercoatOptions_Holes)
					{
						case UICBVAL_MagStripeNormal: bHole_MagNormal   = TRUE; break;
						case UICBVAL_MagStripeWide:   bHole_MagWide     = TRUE; break;
						case UICBVAL_ChipNormal:      bHole_ChipNormal  = TRUE; break;
						case UICBVAL_ChipLarge:       bHole_ChipLarge   = TRUE; break;
						//case UICBVAL_UserDefined:     bHole_UserDefined = TRUE; break;
					}

					if (bHole_MagNormal)
					{
						psettings->CB_BackHole1_Left = 0;
						psettings->CB_BackHole1_Bottom = 420;
						psettings->CB_BackHole1_Width = 1012;
						psettings->CB_BackHole1_Height = 170;
						
						indentlevel = SendHole(pdev, psettings, FALSE, FALSE, indentlevel);
					}
					else if (bHole_MagWide)
					{
						psettings->CB_BackHole1_Left = 0;
						psettings->CB_BackHole1_Bottom = 400;
						psettings->CB_BackHole1_Width = 1012;
						psettings->CB_BackHole1_Height = 200;
						
						indentlevel = SendHole(pdev, psettings, FALSE, FALSE, indentlevel);
					}
					else if (bHole_ChipNormal)
					{
						psettings->CB_BackHole1_Left = 90;
						psettings->CB_BackHole1_Bottom = 295;
						psettings->CB_BackHole1_Width = 260;
						psettings->CB_BackHole1_Height = 190;
						
						indentlevel = SendHole(pdev, psettings, FALSE, FALSE, indentlevel);

					}
					else if (bHole_ChipLarge)
					{
						psettings->CB_BackHole1_Left = 75;
						psettings->CB_BackHole1_Bottom = 275;
						psettings->CB_BackHole1_Width = 280;
						psettings->CB_BackHole1_Height = 190;
						
						indentlevel = SendHole(pdev, psettings, FALSE, FALSE, indentlevel);
					}
/*					else if (bHole_UserDefined)
					{
						if (AreaHoleActive(pdev, psettings, BACK_HOLE1))
							indentlevel = SendHole(pdev, psettings, FALSE, FALSE, indentlevel);
						
						if (AreaHoleActive(pdev, psettings, BACK_HOLE2))
							indentlevel = SendHole(pdev, psettings, FALSE, TRUE, indentlevel);
					}						
*/					
				}

				indentlevel--;				
				myWritecs(pdev,"</panel-options>\015",0,indentlevel);
			}	
			indentlevel--;
			myWritecs(pdev,"</side-options>\015",0,indentlevel--);		
		}
	}
	if (bSendMagEncodeJob)
	{	
		indentlevel++;
		//now the composed mag struct
		myWritecs(pdev,"<mag-options>\015",0,indentlevel++);
		if (mag.coercivity == 0)
			myWritecs(pdev,"<coercivity>high</coercivity>\015",0,indentlevel);
		else
			myWritecs(pdev,"<coercivity>low</coercivity>\015",0,indentlevel);		

		if (psettings->bPerformVerification)
			myWritecs(pdev,"<verify-attempts>3</verify-attempts>\015",0,indentlevel);
		else
			myWritecs(pdev,"<verify-attempts>0</verify-attempts>\015",0,indentlevel);

		/* if (mag.tracks[0].num_bits)
		{
			myWritecs(pdev,"<track-options track=\042\061\042>\015",0,indentlevel++);
			memset((PBYTE)&temp_string,0, 64);
			sprintf(temp_string,"<num-bits>%u</num-bits>\015",mag.tracks[0].num_bits);
			myWritecs(pdev,temp_string,0,indentlevel);				
			sprintf(temp_string,"<bpi>%u</bpi>\015",mag.tracks[0].bits_per_inch);
			myWritecs(pdev,temp_string,0,indentlevel--);				
			myWritecs(pdev,"</track-options>\015",0,indentlevel--);
		}
		if (mag.tracks[1].num_bits)
		{
			indentlevel++;
			myWritecs(pdev,"<track-options track=\042\062\042>\015",0,indentlevel++);
			memset((PBYTE)&temp_string,0, 64);
			sprintf(temp_string,"<num-bits>%u</num-bits>\015",mag.tracks[1].num_bits);
			myWritecs(pdev,temp_string,0,indentlevel);				
			sprintf(temp_string,"<bpi>%u</bpi>\015",mag.tracks[1].bits_per_inch);
			myWritecs(pdev,temp_string,0,indentlevel--);				
			myWritecs(pdev,"</track-options>\015",0,indentlevel--);
		}
		if (mag.tracks[2].num_bits)
		{
			indentlevel++;			
			myWritecs(pdev,"<track-options track=\042\063\042>\015",0,indentlevel++);
			memset((PBYTE)&temp_string,0, 64);
			sprintf(temp_string,"<num-bits>%u</num-bits>\015",mag.tracks[2].num_bits);
			myWritecs(pdev,temp_string,0,indentlevel);				
			sprintf(temp_string,"<bpi>%u</bpi>\015",mag.tracks[2].bits_per_inch);
			myWritecs(pdev,temp_string,0,indentlevel--);				
			myWritecs(pdev,"</track-options>\015",0,indentlevel--);
		}		
		myWritecs(pdev,"</mag-options>\015",0,indentlevel--); */
		
	}	
	myWritecs(pdev,"</card>\015",0,indentlevel);	
	
	
#if 0
    //jobHeader.version = COMMS_VERSION;
    time((time_t *)&jobHeader.timestamp);
    myWrite(pdev, (PBYTE)&jobHeader, sizeof(HDR_JOB));
    
    //now send a CardOptions struct
    memset ((PBYTE)&cardOptions, 0, sizeof(CARD_OPTIONS));
    cardOptions.num_copies = iCopyCount;
    strcpy_s((PBYTE)&cardOptions.driver_version, 16, DRV_VERSION);
    strcpy_s((PBYTE)&cardOptions.driver_language, 16, "ENG");
    cardOptions.rotate_before_print = (pdev->bDuplex == FALSE && bFrontPage == FALSE) ? 1 : 0;
    cardOptions.rotate_before_lam = 0;
    cardOptions.disable_eject = 0;
    cardOptions.side1_options.laminate_target_temperature = 64;
    cardOptions.side2_options.laminate_target_temperature = 64;
	
	if (pdev->bDuplex)
	{
		if (settings->CF_HoloKote)
		{
			 cardOptions.side1_options.holokote_index = settings->CF_SecurityOptions_SecurityType;
			 cardOptions.side1_options.holokote_orientation = (settings->CF_SecurityOptions_Rotation) ? 180 : 0;////
		}
		if (settings->CB_HoloKote)
		{
			cardOptions.side2_options.holokote_index = settings->CB_SecurityOptions_SecurityType;
			cardOptions.side2_options.holokote_orientation = (settings->CB_SecurityOptions_Rotation) ? 180 : 0;////
		}
	}	
	else
	{
    	if (bFrontPage && settings->CF_HoloKote)
		{
			 cardOptions.side1_options.holokote_index = settings->CF_SecurityOptions_SecurityType;
			 cardOptions.side1_options.holokote_orientation = (settings->CF_SecurityOptions_Rotation) ? 180 : 0;////
		}
		if (bFrontPage == FALSE && settings->CB_HoloKote)
		{
			cardOptions.side1_options.holokote_index = settings->CB_SecurityOptions_SecurityType;
			cardOptions.side1_options.holokote_orientation = (settings->CB_SecurityOptions_Rotation) ? 180 : 0;////
		}			
		
	}

	cardOptions.side1_options.power_adjust[PANEL_YELLOW]=
	cardOptions.side1_options.power_adjust[PANEL_MAGENTA]=
	cardOptions.side1_options.power_adjust[PANEL_CYAN]=settings->nPrintHeadPower_YMC;
	cardOptions.side1_options.power_adjust[PANEL_RESIN]=settings->nPrintHeadPower_BlackResin;
	cardOptions.side1_options.power_adjust[PANEL_OVERCOAT]=settings->nPrintHeadPower_Overcoat;
	
	cardOptions.side2_options.power_adjust[PANEL_YELLOW]=
	cardOptions.side2_options.power_adjust[PANEL_MAGENTA]=
	cardOptions.side2_options.power_adjust[PANEL_CYAN]=settings->nPrintHeadPower_YMC;
	cardOptions.side2_options.power_adjust[PANEL_RESIN]=settings->nPrintHeadPower_BlackResin;
	cardOptions.side2_options.power_adjust[PANEL_OVERCOAT]=settings->nPrintHeadPower_Overcoat;
	
	
	cardOptions.side1_options.image_position_adjust_x = 
	cardOptions.side2_options.image_position_adjust_x = 0;
	cardOptions.side1_options.image_position_adjust_y = 
	cardOptions.side2_options.image_position_adjust_y = 0;
	
    myWrite(pdev, (PBYTE)&cardOptions, sizeof(CARD_OPTIONS));
	if (bSendMagEncodeJob)
	{	
		//now the composed mag struct
		myWrite(pdev, (PBYTE)&mag, sizeof(MAG));
	}		
#endif    
	//1951038
	//1951281 //243
	//
	memset((PBYTE)&temp_string,0, sizeof(temp_string));
	sprintf(temp_string,"<binary-size>%lu</binary-size>\015",pdev->epjobSize + strlen((const char*)mag.tracks[0].track_data) + strlen((const char*)mag.tracks[1].track_data) + strlen((const char*)mag.tracks[2].track_data)); 
	myWritecs(pdev,temp_string,0,indentlevel);
	time((time_t *)&jobHeader.timestamp);
	sprintf(temp_string,"<time>%u</time>\015",jobHeader.timestamp );
	myWritecs(pdev,temp_string,0,indentlevel);

	myWritecs(pdev,"</prn>\015\000",8,0);

	/* if (mag.tracks[0].num_bits)
	{	
		myWritecs(pdev,mag.tracks[0].track_data,(char *)mag.tracks[0].track_data,0);
	}
	if (mag.tracks[0].num_bits)
	{	
		myWritecs(pdev,mag.tracks[1].track_data,strlen(mag.tracks[1].track_data),0);
	}
	if (mag.tracks[0].num_bits)
	{	
		myWritecs(pdev,mag.tracks[2].track_data,strlen(mag.tracks[2].track_data),0);
	}	 
	 Flush the channel buffers
     PmChannelFlush(pdev); */

    return;
}
/*******************************************************************************
 *      GetDuplexInfo()
 *      This function finds if the page is going to be duplexed and which side.
 *
 *  Returns:
 *      None
*******************************************************************************/

VOID GetDuplexInfo
(
    PDEVDATA pdev,         // Pointer to our PDEV
	struct 	 settings_ * settings, 
    int      iPageNumber    // Current page number
)
{
    //PRIVATEDEVMODE *pdmPrivate = (PRIVATEDEVMODE *)GetPrivateDevMode((PDEVMODE)&pdev->dm.dmPublic);
    int      iLastLogicalPage = settings->iTotalJobPages;//TODO:where we getting this from then????//GetUICBScratchProperties(pdev->lpdm)->iTotalJobPages;
    int      iDuplex          = settings->Duplex;
	
	//note: if direct printing is enabled this will fail and will rely on duplex set in application
	if (settings->Printer == RIO_PRO_XTENDED || settings->Printer == PRIDENTOPROXTD || settings->Printer == PRICECARD_PRO_FLEX)
	{
fprintf(stderr, "CATULTRA settings->XXL_ImageType: %u\n",settings->XXL_ImageType);
		if (settings->XXL_ImageType == UICBVAL_DoubleImage)
			////if (settings->iTotalJobPages > 1)
				iDuplex = UICBVAL_Duplex_BothSides;
	}
	
    if (iDuplex == UICBVAL_Duplex_BothSides)
    {
fprintf(stderr, "CATULTRA iDuplex: %u, iPageNumber: %u\n",iDuplex, iPageNumber);

		//Print direct to printer enabled so we have no detail of the number of pages in the document	
		if (iLastLogicalPage == 0)	
		{
			pdev->bFrontPage = (BOOL)(iPageNumber + 2) % 2;
			pdev->bDuplex = TRUE;
fprintf(stderr, "CATULTRA CATCAT pdev->bFrontPage: %u, pdev->bDuplex: %u\n",pdev->bFrontPage, pdev->bDuplex);
			
			return;
		}
		
        if (iLastLogicalPage > 1)
        {
            pdev->bFrontPage = (BOOL)(iPageNumber + 2) % 2;

            // We have more than 2 pages left so it is going to be duplexed
            if (iLastLogicalPage == iPageNumber && pdev->bFrontPage)
            {
                // But if only a page left, we treat it as simplex
                pdev->bDuplex = FALSE;
            }
            else
            {
                pdev->bDuplex = TRUE;
            }
        }
        else // iLastLogicalPage <= 1
        {
            pdev->bFrontPage = TRUE;
            pdev->bDuplex = FALSE;
        }
    }

    else // iDuplex != UICBVAL_Duplex_BothSides
    {
        pdev->bDuplex = FALSE;

        // Even simplex case, we still to know if it is back or front on UI
        // Because some option such as ribbon type can be different
        if (iDuplex == UICBVAL_Duplex_Back)
        {
            pdev->bFrontPage = FALSE;
        }
        else
        {
            pdev->bFrontPage = TRUE;
        }
    }
}
/*******************************************************************************
 *  DetectEdge()
 *      Search through the K-resin surface and decide the pixels
 *      to be modified at DetectAdjacentColour
 *
 *  Returns:
 *      None
*******************************************************************************/

VOID DetectEdge
(
    PDEVDATA pdev ///< Pointer to our PDEV
)
{
    LPBYTE   lpBlack    = NULL;
    LPBYTE   lpTemp     = NULL;
    LPBYTE   lpHalo     = NULL;
    LONG     x          = 0;
    LONG     y          = 0;
    LONG     TblX       = 0;
    LONG     TblY       = 0;
    LONG     StartX     = 0;
    LONG     StartY     = 0;
    LONG     EndX       = 0;
    LONG     EndY       = 0;
    BOOL     bHalo      = FALSE;
//	POINTL   ptlPosition[HALO_WIDTH][HALO_WIDTH];
    POINTL   ptlPosition[MAX_HALO_WIDTH][MAX_HALO_WIDTH];
#define HALO_RADIUS	pdev->iHaloRadius

    /******************************************************************************
	 * Create matrix to get the pointer of the surrounding pixels.
     * If HALO_WIDTH == 3, the table will be...
     *
     *    (-1, -1) | (0, -1)  | (1, -1)
     *  ----------------------------------
     *    (-1,  0) | (0,  0)  | (1,  0)
     *  ----------------------------------
     *    (-1,  1) | (0,  1)  | (1,  1)
     *
     *******************************************************************************/
    for (TblY = 0; TblY < HALO_WIDTH; TblY++)
    {
        for (TblX = 0; TblX < HALO_WIDTH; TblX++)
        {
            if (TblX < HALO_RADIUS)
            {
                ptlPosition[TblX][TblY].x = -(HALO_RADIUS - TblX);
            }
            else if (TblX == HALO_RADIUS)
            {
                ptlPosition[TblX][TblY].x = 0;
            }
            else if (TblX > HALO_RADIUS)
            {
                ptlPosition[TblX][TblY].x = TblX - HALO_RADIUS;
            }

            if (TblY < HALO_RADIUS)
            {
                ptlPosition[TblX][TblY].y = -(HALO_RADIUS - TblY);
            }
            else if (TblY == HALO_RADIUS)
            {
                ptlPosition[TblX][TblY].y = 0;
            }
            else if (TblY > HALO_RADIUS)
            {
                ptlPosition[TblX][TblY].y = TblY - HALO_RADIUS;
            }
        }
    }

    for (y = 0; y < pdev->yImage; y++)
    {
        // Set up pointers to pixels in the Black and Halo planes
        lpBlack = pdev->lpPageBuffer[PLANE_K] + (pdev->ulCMYInPlaneWidth * y);
        lpHalo  = pdev->lpPageBuffer[PLANE_HALO] + (pdev->ulCMYInPlaneWidth * y);

        // NOTE: lpHalo can be 1bpp to save memory and performance.

        // Decide the range to refer using a table.
        if (y < HALO_RADIUS) // top
        {
            StartY = HALO_RADIUS - y;
            EndY   = HALO_WIDTH - 1;
        }
        else if (pdev->yImage - (y + 1) < HALO_RADIUS) // bottom
        {
            StartY = 0;
            EndY   = pdev->yImage - (y + 1) + HALO_RADIUS;
        }
        else // middle
        {
            StartY = 0;
            EndY   = HALO_WIDTH - 1;
        }

        for (x = 0; x < (LONG)pdev->ulCMYInPlaneWidth; x++)
        {
            /*******************************************************************************
             * Test the darkness of this pixel
             *
             * A threshold could be used in the following if (instead of testing for white) so that
             * light grey pixels are not included in the halo plane either
             *******************************************************************************/

            if (*lpBlack == 0)
            {
                // Since this pixel is white it should not form part of a halo for edge detection
                *lpHalo = 0x0;
            }
            else
            {
                // This pixel is black or greyscale

                if (x < HALO_RADIUS) // left edge
                {
                    StartX = HALO_RADIUS - x;
                    EndX   = HALO_WIDTH  - 1;
                }
                else if ((LONG)pdev->ulCMYInPlaneWidth - (x + 1) < HALO_RADIUS) // right edge
                {
                    StartX = 0;
                    EndX   = pdev->ulCMYInPlaneWidth - (x + 1) + HALO_RADIUS;
                }
                else // middle
                {
                    StartX = 0;
                    EndX   = HALO_WIDTH - 1;
                }

                // Start with the assumption that we are not near to a black boundary
                bHalo = FALSE;

                // Look around the pixel ... if it is different object or white.
                for (TblY = StartY; TblY <= EndY; TblY++)
                {
                    for (TblX = StartX; TblX <= EndX; TblX++)
                    {
                        // Test a surrounding pixel in the black plane
                        lpTemp =   lpBlack 
                                 + ((LONG)pdev->ulCMYInPlaneWidth * ptlPosition[TblX][TblY].y) 
								 + ptlPosition[TblX][TblY].x;

                        if (*lpTemp == 0x0)
                        {
                            // We have found a surrounding pixel that is white, so flag this as a halo point
                            // and break out of the x loop
                            bHalo = TRUE;
                            break;
                        }
                    }

                    if (bHalo)
                    {
                        // We already know that this is a halo point, so break out of the y loop
                        break;
                    }
                }

                if (bHalo)
                {
                    *lpHalo = 0xff; // This is a halo pixel
                }
                else
                {
                    *lpHalo = 0x0; // This is NOT a halo pixel
                }
            }

            // Move on to the next pixel in the x-scanline for both planes
            lpBlack++;
            lpHalo++;
        }
    }

    return;
}


/*******************************************************************************
 *  ReplaceYMCPixelColour()
 *      Calculate the average value of surrounding 8 coloured pixels.
 *
 *  Returns:
 *      None
*******************************************************************************/

VOID ReplaceYMCPixelColour
(
    LPBYTE  lpPrev,    // The pixel directly above the one being processed
    LPBYTE  lpCurrent, // The pixel to be processed
    LPBYTE  lpNext,    // The pixel directly below the one being processed
    BOOL    bLeft,     // TRUE = This is the left pixel on the strip
    BOOL    bRight     // TRUE = This is the right pixel on the strip
)
{
    ULONG   ulPixel  = 0;
    ULONG   ulDiv    = 0;

    /*******************************************************************************
     * For this pixel, calculate a weighted average of the valid surrounding colours
     * Surrounding pixels must be non-white, and within scanline bounds to be included
     * in this average
     *******************************************************************************/
    if (lpPrev != NULL)
    {
//		if (*(lpPrev - 1) != 0 && !bLeft)
//		{
//			ulPixel += *(lpPrev - 1);
//			ulDiv++;
//		}

        if (*lpPrev != 0)
        {
            ulPixel += *lpPrev;
            ulDiv++;
        }

//		if (*(lpPrev + 1) != 0 && !bRight)
//		{
//			ulPixel += *(lpPrev + 1);
//			ulDiv++;
//		}
        lpPrev ++;
    }

//	if (lpCurrent != NULL)
//	{
//		if (*(lpCurrent - 1) != 0 && !bLeft)
//		{
//			ulPixel += *(lpCurrent - 1);
//			ulDiv++;
//		}
//
//		if (*(lpCurrent + 1) != 0 && !bRight)
//		{
//			ulPixel += *(lpCurrent + 1);
//			ulDiv++;
//		}
//	}

    if (lpNext != NULL)
    {
//		if (*(lpNext - 1) != 0 && !bLeft)
//		{
//			ulPixel += *(lpNext - 1);
//			ulDiv++;
//		}

        if (*lpNext != 0)
        {
            ulPixel += *lpNext;
            ulDiv++;
        }

//		if (*(lpNext + 1) != 0 && !bRight)
//		{
//			ulPixel += *(lpNext + 1);
//			ulDiv++;
//		}
        lpNext ++;
    }

    if (ulDiv != 0)
    {
        *lpCurrent = (BYTE)(ulPixel / ulDiv);
    }
}


/*******************************************************************************
 *      CreateKPrintingObject()
 *      Create strip obj and halftone obj.
 *
 *  Returns:
 *      FALSE if it fails to create objects.
*******************************************************************************/

BOOL CreateKPrintingObject
(
    PDEVDATA   pdev,       		// Pointer to our PDEV
    struct 	 settings_ * settings,
    cups_page_header2_t header
)
{
    long     lWidth     = 0;
    long     lHeight    = 0;
	DWORD    cb;
	
    if ((pdev->eChannelOption == UICBVAL_YMCK)
    ||  (pdev->eChannelOption == UICBVAL_KResin))
    {
        if (HELIX_OEM(settings))
        {
            lWidth  = pdev->epPaperXdots;
            lHeight = pdev->epPaperYdots;
        }
        else
        {
        //create a kplane
		if (header.cupsHeight == 1016 || header.cupsHeight == 991 || header.cupsHeight == 1654)
        //if (header.Orientation == 1 || header.Orientation == 3/*pdev->dm.dmPublic.dmOrientation == DMORIENT_LANDSCAPE*/)    
        {
            lWidth  = pdev->epPaperYdots;
            lHeight = pdev->epPaperXdots;
        }
        else
        {
            lWidth  = pdev->epPaperXdots;
            lHeight = pdev->epPaperYdots;
        }
        }
        // Create masking surface 8bpp duping the 24bpp orientation
		cb=(lWidth * 3) * lHeight;
        pdev->lpKSrcStrip = malloc(cb);
        if (pdev->lpKSrcStrip == 0)
            return FALSE;
		memset(pdev->lpKSrcStrip, 0x00, cb);
            
        //if we are landscape create a portrait masking surface
		pdev->lpKPSrcStrip = malloc(cb);
        if (pdev->lpKPSrcStrip == 0)
            return FALSE;
        memset(pdev->lpKPSrcStrip, 0x00, cb);
		
        // Create destination surface 8bpp
		pdev->lpKDstStrip = malloc(cb);
        if (pdev->lpKDstStrip == 0)
            return FALSE;
        memset(pdev->lpKDstStrip, 0x00, cb);
		

    }
//TODO:what to do with this...
/*    
    switch (pdev->eHalftoning)
    {
        case UICBVAL_Halftoning_ErrorDiffusion:
        default:
            htMethod = HALFTONETYPE_DIFFUSION;
            break;

        case UICBVAL_Halftoning_LineArt:
            htMethod = HALFTONETYPE_LINEART;
            break;
    }
*/
	// Create Halftone 1bpp monochrome surface
	cb=lWidth * lHeight;
	pdev->lpHalftone = malloc(cb);
	if (pdev->lpHalftone == 0)
		return FALSE;
	memset(pdev->lpHalftone, 0x00, cb);	


	return TRUE;
}

/*******************************************************************************
 *  DeleteOutputBuffer()
 *      Clean up buffers which were allocated at InitializeOutputBuffer.
 *      Need to be called in the end of a page.
 *
 *  Returns:
 *      None
*******************************************************************************/

VOID DeleteOutputBuffer
(
    PDEVDATA pdev  // Pointer to our PDEV
)
{

    if (pdev->lpCMYIn[0] != 0)
    {
        free(pdev->lpCMYIn[0]);
        pdev->lpCMYIn[0] = 0;
    }

    if (pdev->lpCMYOut[0] != 0)
    {
        free(pdev->lpCMYOut[0]);
        pdev->lpCMYOut[0] = 0;
    }

    if (pdev->lpKOut != 0)
    {
        free(pdev->lpKOut);
        pdev->lpKOut = 0;
    }

    if (pdev->lpPageBuffer[0] != 0)
    {
        free(pdev->lpPageBuffer[0]);
        pdev->lpPageBuffer[0] = 0;
    }

}

/*******************************************************************************
 *  InitializeOutputBuffer()
 *      Allocate buffers
 *      Those buffers are used to convert strips into device format.
 *      Need to be called at the beginning of the page.
 *
 *  Returns:
 *      None
*******************************************************************************/

VOID InitializeOutputBuffer
(
    PDEVDATA   pdev,       // Pointer to our PDEV
    struct 	 settings_ * settings
)
{
    ULONG    ulChannelOption = pdev->eChannelOption;
    ULONG    ulColour        = 0;

    if ((pdev->lpCMYIn[0]  != NULL)
    &&  (pdev->lpCMYOut[0] != NULL)
    &&  (pdev->lpKOut      != NULL))
    {
        // All buffer is available
        return;
    }
fprintf(stderr, "CATULTRA InitializeOutputBuffer:ulChannelOption: %ul\n",ulChannelOption);
fprintf(stderr, "CATULTRA InitializeOutputBuffer:pdev->xImage: %u\n",pdev->xImage);
	pdev->ulCMYInPlaneWidth = pdev->xImage;//642 
	
    if ((ulChannelOption == UICBVAL_YMC)
    ||  (ulChannelOption == UICBVAL_YMCK))
    {
        LPBYTE lpCMYIn  = NULL;
        LPBYTE lpCMYOut = NULL;

        // Create 8 bit and 6 bit plane buffer
                                                   //2592.. 648 x288
//        pdev->ulCMYInPlaneWidth = (((pStripObj->Bitmap.ulPlaneWidth / 4) + 7) / 8) * 8;

        //so in  plane width always 0x288 in original ....

        if (RIO_OEM(settings)
        ||  AVALON_MODEL(settings))
	    {
            pdev->ulCMYOutPlaneWidth = OUTPUT_DATA_WIDTH_RIO;
		}
		else
        if (HELIX_OEM(settings))
            pdev->ulCMYOutPlaneWidth = OUTPUT_DATA_WIDTH_HELIX;
		else
        if (PRO360_OEM(settings))
            pdev->ulCMYOutPlaneWidth = OUTPUT_DATA_WIDTH_PRO360;
        else
            pdev->ulCMYOutPlaneWidth = OUTPUT_DATA_WIDTH_ALTO;

        lpCMYIn  = (LPBYTE)malloc(pdev->ulCMYInPlaneWidth  * 3);
        lpCMYOut = (LPBYTE)malloc(pdev->ulCMYOutPlaneWidth * 3);

        if ((lpCMYIn == NULL)
		||  (lpCMYOut == NULL))
        {
            //pPDev->eCallBackStatus = DCBS_FATALERROR;
            goto cleanup_buffer;
        }

        memset(lpCMYIn, 0, pdev->ulCMYInPlaneWidth  * 3);
        memset(lpCMYOut, 0, pdev->ulCMYOutPlaneWidth * 3);

        for (ulColour = 0; ulColour < 3; ulColour++)
        {
            pdev->lpCMYIn[ulColour]  = lpCMYIn  + ulColour * pdev->ulCMYInPlaneWidth;
            pdev->lpCMYOut[ulColour] = lpCMYOut + ulColour * pdev->ulCMYOutPlaneWidth;
        }
    }

    if ((ulChannelOption == UICBVAL_KResin)
	||  (ulChannelOption == UICBVAL_YMCK))
    {
        // Fixed as format_printhead_data() requires this value
        if (RIO_OEM(settings)
        ||  AVALON_MODEL(settings))
	    {
           pdev->ulKOutPlaneWidth = OUTPUT_DATA_WIDTH_RIO / 6;
		}
		else
		if (PRO360_OEM(settings))  
           pdev->ulKOutPlaneWidth = OUTPUT_DATA_WIDTH_PRO360;
		else
        if (HELIX_OEM(settings))
           pdev->ulKOutPlaneWidth = OUTPUT_DATA_WIDTH_HELIX;
		else
	    {
           pdev->ulKOutPlaneWidth = OUTPUT_DATA_WIDTH_ALTO / 6;
		}

        pdev->lpKOut = (LPBYTE)malloc(pdev->ulKOutPlaneWidth);
        if (pdev->lpKOut == 0)
        {
            pdev->eCallBackStatus = DCBS_FATALERROR;
            goto cleanup_buffer;
        }
        memset(pdev->lpKOut, 0, pdev->ulKOutPlaneWidth);
    }

    if ((ulChannelOption == UICBVAL_YMCK)
//  &&  pdev->bDetectAdjacentColour)
    ||  (ulChannelOption == UICBVAL_YMC))
    {
        LPBYTE lpPageBuffer = 0;

        lpPageBuffer = (LPBYTE)malloc(pdev->ulCMYInPlaneWidth * pdev->yImage * PLANE_MAX);
        if (lpPageBuffer == 0)
        {
            pdev->eCallBackStatus = DCBS_FATALERROR;
            goto cleanup_buffer;
        }
fprintf(stderr, "CATULTRA InitializeOutputBuffer:pdev->yImage: %u\n",pdev->yImage);
        memset(lpPageBuffer, 0, pdev->ulCMYInPlaneWidth * pdev->yImage * PLANE_MAX);
        for (ulColour = 0; ulColour < PLANE_MAX; ulColour++)
        {                                                                   //x282..642             1016
            pdev->lpPageBuffer[ulColour] = lpPageBuffer + ulColour * (pdev->ulCMYInPlaneWidth * pdev->yImage) ;
        }
    }

cleanup_buffer:
    if (pdev->eCallBackStatus == DCBS_FATALERROR)
        DeleteOutputBuffer(pdev);

	return;
}
/*****************************************************************************
 *  _initialize_BufferInfo()
 *      Initialize SPOOLMEMINFO structure.
 *      Supply memory alloc function both for user and kernel mode.
 *
 *  Returns:
 *      None
 *****************************************************************************/

BOOL _initialize_BufferInfo
(
    LPSPOOLMEMINFO lpSplMemInfo,    // Pointer to SPOOLMEMINFO
    ULONG          ulSize,          // Required memory size
    ULONG          ulColour         // The colour to sent out. It should be one of: PLANE_Y, PLANE_M, PLANE_C, PLANE_K
)
{
    /*
     * Call memory allocation function.
     * For Kernel mode we use EngAllocMem and use paged pool.
     */

    lpSplMemInfo->lpBuffer = malloc( ulSize );

    if (lpSplMemInfo->lpBuffer == NULL)
    {
        return FALSE;
    }

    lpSplMemInfo->ulDataSize    = 0;
    lpSplMemInfo->ulColour      = ulColour;
    lpSplMemInfo->bDataInPlane  = FALSE;

    return TRUE;
}
/*****************************************************************************
 *  FreeBuffer()
 *      Clean up memory which were allocated at AllocSpoolMem
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID FreeBuffer
(
    PDEVDATA pdev, // Pointer to our PDEV
    BOOL     bFront // TRUE = Front page (or simplex) : FALSE = Back page
)
{
    ULONG          ulColour  = 0;
    LPSPOOLMEMINFO lpSplMem  = NULL;

    for (ulColour = 0 ; ulColour < 4; ulColour ++)
    {
        if (bFront)
        {
            lpSplMem = &pdev->lpSplMemFront[ ulColour ];
        }
        else
        {
            lpSplMem = &pdev->lpSplMemBack[ ulColour ];
        }

        if (lpSplMem->lpBuffer != NULL)
        {
            free( lpSplMem->lpBuffer );
        }

        lpSplMem->lpBuffer      = NULL;
        lpSplMem->ulDataSize    = 0;
        lpSplMem->ulColour      = 0;
        lpSplMem->bDataInPlane  = FALSE;
    }
    return;
}

/*****************************************************************************
 *  AllocBuffer()
 *      Alloc memory for page data spool.
 *      In duplex, we spool both front and back pages.
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID AllocBuffer
(
    PDEVDATA pdev, // Pointer to our PDEV,
	struct settings_ * settings,
    BOOL     bFront // TRUE = Front page (or simplex) : FALSE = Back page
)
{
    ULONG    ulMemSize      = 0;
    ULONG    ulMemSizeMono  = 0;
    BOOL     bRet           = FALSE;

    if (RIO_OEM(settings)
    ||  AVALON_MODEL(settings))
	{
        ulMemSize     = OUTPUT_DATA_WIDTH_RIO       * pdev->yImage;
        ulMemSizeMono = (OUTPUT_DATA_WIDTH_RIO / 6) * pdev->yImage;
	}
	else
    if (PRO360_OEM(settings))
    {
        ulMemSize = ulMemSizeMono = OUTPUT_DATA_WIDTH_PRO360 * pdev->yImage;
    }
	else
    if (HELIX_OEM(settings))
    {
        ulMemSize = ulMemSizeMono = OUTPUT_DATA_WIDTH_HELIX * pdev->yImage;
//        ulMemSizeMono = (OUTPUT_DATA_WIDTH_OPTIMA / 6) * pdev->yImage;    
    }
    else
	{
        ulMemSize     = OUTPUT_DATA_WIDTH_ALTO       * pdev->yImage;
        ulMemSizeMono = (OUTPUT_DATA_WIDTH_ALTO / 6) * pdev->yImage;
	}

    if (XXL_TYPE(settings)
    && (pdev->eChannelOption == UICBVAL_KResin))
	{
		//For extended Monochrome on an XXL type printer, adjust the buffer size
		//for K Resin to be the same as a colour plane
		ulMemSizeMono = ulMemSize;
	}
fprintf(stderr, "L5972:CATULTRA Alloc %u\n",bFront);

    if (bFront)
    {
		
        switch (pdev->eChannelOption)
        {
            case UICBVAL_YMC:
                // Switch depends on which side
                bRet = _initialize_BufferInfo( &pdev->lpSplMemFront[PLANE_C], ulMemSize, PLANE_C );
                bRet = _initialize_BufferInfo( &pdev->lpSplMemFront[PLANE_M], ulMemSize, PLANE_M );
                bRet = _initialize_BufferInfo( &pdev->lpSplMemFront[PLANE_Y], ulMemSize, PLANE_Y );
                break;

            case UICBVAL_YMCK:
            default:
                bRet = _initialize_BufferInfo( &pdev->lpSplMemFront[PLANE_C], ulMemSize, PLANE_C );
                bRet = _initialize_BufferInfo( &pdev->lpSplMemFront[PLANE_M], ulMemSize, PLANE_M );
                bRet = _initialize_BufferInfo( &pdev->lpSplMemFront[PLANE_Y], ulMemSize, PLANE_Y );
                bRet = _initialize_BufferInfo( &pdev->lpSplMemFront[PLANE_K], ulMemSizeMono, PLANE_K );
                break;

            case UICBVAL_KResin:
                bRet = _initialize_BufferInfo( &pdev->lpSplMemFront[PLANE_K],  ulMemSizeMono, PLANE_K );
                break;
        }
    }
    else
    {
fprintf(stderr, "L6003:CATULTRA eChannelOption %u\n",pdev->eChannelOption);
		
        switch (pdev->eChannelOption)
        {
            case UICBVAL_YMC:
                // Switch depends on which side
fprintf(stderr, "L6009:CATULTRA eChannelOption %u\n",pdev->eChannelOption);
                bRet = _initialize_BufferInfo( &pdev->lpSplMemBack[PLANE_C], ulMemSize, PLANE_C );
                bRet = _initialize_BufferInfo( &pdev->lpSplMemBack[PLANE_M], ulMemSize, PLANE_M );
                bRet = _initialize_BufferInfo( &pdev->lpSplMemBack[PLANE_Y], ulMemSize, PLANE_Y );
                bRet = _initialize_BufferInfo( &pdev->lpSplMemBack[PLANE_K], ulMemSizeMono, PLANE_K );

                break;

            case UICBVAL_YMCK:
            default:
fprintf(stderr, "L6018:CATULTRA eChannelOption %u\n",pdev->eChannelOption);
                bRet = _initialize_BufferInfo( &pdev->lpSplMemBack[PLANE_C], ulMemSize, PLANE_C );
                bRet = _initialize_BufferInfo( &pdev->lpSplMemBack[PLANE_M], ulMemSize, PLANE_M );
                bRet = _initialize_BufferInfo( &pdev->lpSplMemBack[PLANE_Y], ulMemSize, PLANE_Y );
                bRet = _initialize_BufferInfo( &pdev->lpSplMemBack[PLANE_K], ulMemSizeMono, PLANE_K );
                break;

            case UICBVAL_KResin:
fprintf(stderr, "L6025:CATULTRA eChannelOption %u\n",pdev->eChannelOption);
                bRet = _initialize_BufferInfo( &pdev->lpSplMemBack[PLANE_K], ulMemSizeMono, PLANE_K );
                break;
        }
    }

    if (!bRet)
    {
fprintf(stderr, "L6027:CATULTRA Alloc %u\n",bRet);
		
        pdev->eCallBackStatus = DCBS_FATALERROR;
        FreeBuffer( pdev, bFront );
    }

    return;
}


/*****************************************************************************
 *  CopyToBuffer()
 *      Copy data into spool buffer.
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID CopyToBuffer
(
    PDEVDATA pdev,         // Pointer to our PDEV
    LPBYTE   lpOutBuf,      // Pointer to plane data
    ULONG    ulOutBufSize,  // The data length in pPlane
    ULONG    ulColour,      // The colour to sent out. It should be one of: PLANE_Y, PLANE_M, PLANE_C, PLANE_K
    BOOL     bDataInPlane   // TRUE if there is any colour in lpOutBuf
)
{
////    LPDVPDEV       lpDrvData = pdev->pDrvData;
    LPSPOOLMEMINFO lpSplMem  = NULL;

    if (!pdev->bFrontPage)
    {
        lpSplMem = &pdev->lpSplMemBack[ ulColour ];
    }
    else
    {
        lpSplMem = &pdev->lpSplMemFront[ ulColour ];
    }

    if (bDataInPlane && !lpSplMem->bDataInPlane)
    {
        // If there is colour we will send out this plane to the device at EndPage.
        lpSplMem->bDataInPlane = TRUE;
    }

    if (lpSplMem->lpBuffer != NULL)
    {
        memcpy( lpSplMem->lpBuffer + lpSplMem->ulDataSize, lpOutBuf, ulOutBufSize );
        lpSplMem->ulDataSize += ulOutBufSize;
    }

    return;
}

VOID GetPixelRGB(PDEVDATA pdev, PBYTE pSurface, PRGBTRIPLE pRGB, WORD x, WORD y)
{
    PBYTE pBits;

    pBits = (PBYTE)pSurface;
    pBits += (y* (pdev->epPaperXdots*3));
    pBits += (x*3);

    pRGB->rgbtRed  = pBits[0];
    pRGB->rgbtGreen = pBits[1];
    pRGB->rgbtBlue   = pBits[2];
    
}

VOID SetPixelRGB(PDEVDATA pdev, PBYTE pSurface, int x, int y, BYTE r, BYTE g, BYTE b)
{
    PBYTE pBits = (PBYTE)pSurface + (y* (PREVIEW_WIDTH*3)) + x*3;
    
    *pBits++ = b;
    *pBits++ = g;
    *pBits = r;
    
}

VOID Resize(PDEVDATA pdev, PBYTE pSrc, PBYTE pDst )
{
    int nWidth = PREVIEW_WIDTH;
    int nHeight = PREVIEW_HEIGHT;
    int x,y;
    double nXFactor;
    double nYFactor;
    double fraction_x, fraction_y, one_minus_x, one_minus_y;
    int ceil_x, ceil_y, floor_x, floor_y;
    RGBTRIPLE c1;
    RGBTRIPLE c2;
    RGBTRIPLE c3;
    RGBTRIPLE c4;
    BYTE red, green, blue;
    BYTE b1, b2;
 
    nXFactor = (double)pdev->epPaperXdots/(double)nWidth;
    nYFactor = (double)pdev->epPaperYdots/(double)nHeight;

    for (y = 0; y < nHeight; y++)
    {
        for (x = 0; x < nWidth; x++)
        {
            // Setup
            
            floor_x = (int)floor(x * nXFactor);
            floor_y = (int)floor(y * nYFactor);
            ceil_x = floor_x + 1;
            if (ceil_x >= pdev->epPaperXdots) ceil_x = floor_x;
            ceil_y = floor_y + 1;
            if (ceil_y >= pdev->epPaperYdots) ceil_y = floor_y;
            fraction_x = x * nXFactor - floor_x;
            fraction_y = y * nYFactor - floor_y;
            one_minus_x = 1.0 - fraction_x;
            one_minus_y = 1.0 - fraction_y;
            
            GetPixelRGB(pdev, pSrc,(PRGBTRIPLE)&c1, floor_x, floor_y);
            GetPixelRGB(pdev, pSrc,(PRGBTRIPLE)&c2, ceil_x, floor_y);
            GetPixelRGB(pdev, pSrc,(PRGBTRIPLE)&c3, floor_x, ceil_y);
            GetPixelRGB(pdev, pSrc,(PRGBTRIPLE)&c4, ceil_x, ceil_y);
            
            // Blue
            b1 = (BYTE)(one_minus_x * c1.rgbtBlue + fraction_x * c2.rgbtBlue);
            b2 = (BYTE)(one_minus_x * c3.rgbtBlue + fraction_x * c4.rgbtBlue);
            blue = (BYTE)(one_minus_y * (double)(b1) + fraction_y * (double)(b2));
            
            // Green
            b1 = (BYTE)(one_minus_x * c1.rgbtGreen + fraction_x * c2.rgbtGreen);
            b2 = (BYTE)(one_minus_x * c3.rgbtGreen + fraction_x * c4.rgbtGreen);
            green = (BYTE)(one_minus_y * (double)(b1) + fraction_y * (double)(b2));
            
            // Red
            b1 = (BYTE)(one_minus_x * c1.rgbtRed + fraction_x * c2.rgbtRed);
            b2 = (BYTE)(one_minus_x * c3.rgbtRed + fraction_x * c4.rgbtRed);
            red = (BYTE)(one_minus_y * (double)(b1) + fraction_y * (double)(b2));
            //this works..
            SetPixelRGB(pdev, pDst, x,y, red, green, blue);
        }
    }
}
   
ULONG CMYK_to_RGB(BYTE C, BYTE M, BYTE Y, BYTE K, WORD ny)
{
    ULONG t;
        float r,g,b;
        float c=(float)C;
        float m=(float)M;
        float y=(float)Y;
        float k=(float)K;
        
        c = MIN(255,c+k);
        m = MIN(255,m+k);
        y = MIN(255,y+k);
        r= 255-c;
        g= 255-m;
        b= 255-y;
    
    t= (BYTE)r << 16;
    t+= (BYTE)g << 8;
    t+= (BYTE)b;
    
        return (t);
}
    
/*****************************************************************************
 *  FlushBuffer()
 *      Send out all data in spool buffer to the device.
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID FlushBuffer
(
    PDEVDATA pdev, // Pointer to our PDEV
	struct settings_ * settings,
    BOOL     bFront // TRUE = Front page (or simplex) : FALSE = Back page
)
{
    LPSPOOLMEMINFO lpSplMem   = 0;
    ULONG          ulColour   = 0;
    BOOL            bBlank    = TRUE;
	
	if (settings->OEM == OEM_PRO360)
    {
        for (ulColour = 0 ; ulColour < 4; ulColour ++)
        {
            if (bFront)
            {
                lpSplMem = &pdev->lpSplMemFront[ulColour];
            }
            else
            {
                lpSplMem = &pdev->lpSplMemBack[ulColour];
            }

            if (lpSplMem->lpBuffer != NULL && lpSplMem->bDataInPlane)
            {
                OutputPlaneCommand(pdev, 
								   settings,
                                   lpSplMem->lpBuffer, 
                                   lpSplMem->ulColour, 
                                   lpSplMem->ulDataSize);
                bBlank = FALSE;
            }
fprintf(stderr, "L6225:CATULTRA Flushbuffer:%u\n",1);

		}
        
fprintf(stderr, "L6227:CATULTRA Flushbuffer:%u\n",1);
		
        //we have a blank card so print a blank card! 
        if (bBlank && (pdev->magTrackInfo[MAG_TRACK_INDEX1].TrackData[0] == 0)
        && (pdev->magTrackInfo[MAG_TRACK_INDEX2].TrackData[0] == 0)
        && (pdev->magTrackInfo[MAG_TRACK_INDEX3].TrackData[0] == 0))
        {
			if (bFront)
				lpSplMem = &pdev->lpSplMemFront[PLANE_K];
			else
				lpSplMem = &pdev->lpSplMemBack[PLANE_K];
fprintf(stderr, "L6240:CATULTRA Flushbuffer:%u\n",1);
				
            OutputPlaneCommand(pdev,
								settings,
                                lpSplMem->lpBuffer, 
                                lpSplMem->ulColour, 
                                pdev->ulKOutPlaneWidth * 1013/*lpSplMem->ulDataSize*/);
fprintf(stderr, "L6247:CATULTRA Flushbuffer:%u\n",1);
								
        }  

    } //ends if pro360
	else
    if (settings->OEM == OEM_HELIX)
    {
        for (ulColour = 0 ; ulColour < 4; ulColour ++)
        {
            if (bFront)
            {
                lpSplMem = &pdev->lpSplMemFront[ulColour];
            }
            else
            {
                lpSplMem = &pdev->lpSplMemBack[ulColour];
            }

            if (lpSplMem->lpBuffer != NULL && lpSplMem->bDataInPlane)
            {
                OutputPlaneCommand(pdev,
                                   settings,
                                   lpSplMem->lpBuffer, 
                                   lpSplMem->ulColour, 
                                   lpSplMem->ulDataSize);
                bBlank = FALSE;
            }
        }
        
        //we have a blank card so print a blank card!
        if (bBlank && (pdev->magTrackInfo[MAG_TRACK_INDEX1].TrackData[0] == 0)
            && (pdev->magTrackInfo[MAG_TRACK_INDEX2].TrackData[0] == 0)
            && (pdev->magTrackInfo[MAG_TRACK_INDEX3].TrackData[0] == 0))
        {
			if (bFront)
				lpSplMem = &pdev->lpSplMemFront[PLANE_K];
			else
				lpSplMem = &pdev->lpSplMemBack[PLANE_K];

            OutputPlaneCommand(pdev,
                               settings,
                               lpSplMem->lpBuffer,
                               lpSplMem->ulColour,
                               lpSplMem->ulDataSize);
        }

		//generate our preview data
		//now throw out the preview data for the side
		//generated from 32bit CMYK data
 
		if (pdev->bEncodeOnlyJob == FALSE)
        {

            ////ULONG       cx,cy;
            PBYTE       pPreviewSurface = 0;
            PBYTE		pOutputSurface = 0;
            int         rgb888,row,col;
            WORD			x,y,x2,y2;
            
            pPreviewSurface = malloc((PREVIEW_WIDTH * PREVIEW_HEIGHT) * 3);
            pOutputSurface = malloc((pdev->epPaperXdots * pdev->epPaperYdots) * 3);
            
            if (pPreviewSurface)
            {
                PBYTE       lpSrc,lpDst;
                LPSPOOLMEMINFO lpSplMemY   = NULL;
                LPSPOOLMEMINFO lpSplMemM   = NULL;
                LPSPOOLMEMINFO lpSplMemC   = NULL;
                LPSPOOLMEMINFO lpSplMemK   = NULL;
                ////ULONG          ulColour   = 0;

                int			yVal=0,mVal=0,cVal=0,kVal=0;
                ULONG		rgbval;
                
                //generate preview here :)
                
                lpSrc = (PBYTE)pOutputSurface;//soSrc->pvBits;
                if (bFront)
                {
                    lpSplMemY = &pdev->lpSplMemFront[0];
                    lpSplMemM = &pdev->lpSplMemFront[1];
                    lpSplMemC = &pdev->lpSplMemFront[2];
                    lpSplMemK = &pdev->lpSplMemFront[3];
                    
                    for (y=0; y<pdev->epPaperYdots; y++)
                        for (x=0; x<pdev->epPaperXdots; x++)
                        {
                            if (lpSplMemY->ulDataSize)
                                yVal=lpSplMemY->lpBuffer[(y*OUTPUT_DATA_WIDTH_HELIX)+x];
                            if (lpSplMemM->ulDataSize)
                                mVal=lpSplMemM->lpBuffer[(y*OUTPUT_DATA_WIDTH_HELIX)+x];
                            if (lpSplMemC->ulDataSize)
                                cVal=lpSplMemC->lpBuffer[(y*OUTPUT_DATA_WIDTH_HELIX)+x];
                            if (lpSplMemK->ulDataSize)
                                kVal=lpSplMemK->lpBuffer[(y*OUTPUT_DATA_WIDTH_HELIX)+x];
                            
                            rgbval=CMYK_to_RGB((BYTE)cVal,(BYTE)mVal,(BYTE)yVal,(BYTE)kVal,y);
                            lpSrc = (PBYTE)pOutputSurface;
                            lpSrc += (y*(pdev->epPaperXdots*3));
                            lpSrc += (x * 3);
                            *lpSrc++ = GetRValue(rgbval);
                            *lpSrc++ = GetGValue(rgbval);
                            *lpSrc = GetBValue(rgbval);
                        }
                }
                else
                {
                    lpSplMemY = &pdev->lpSplMemBack[0];
                    lpSplMemM = &pdev->lpSplMemBack[1];
                    lpSplMemC = &pdev->lpSplMemBack[2];
                    lpSplMemK = &pdev->lpSplMemBack[3];
                    
                    for (y2=0,y=pdev->epPaperYdots-1; y; y--,y2++)
                        for (x2=0,x=pdev->epPaperXdots-3; x; x--,x2++)
                        {
                            if (lpSplMemY->ulDataSize)
                                yVal=lpSplMemY->lpBuffer[(y*OUTPUT_DATA_WIDTH_HELIX)+x];
                            if (lpSplMemM->ulDataSize)
                                mVal=lpSplMemM->lpBuffer[(y*OUTPUT_DATA_WIDTH_HELIX)+x];
                            if (lpSplMemC->ulDataSize)
                                cVal=lpSplMemC->lpBuffer[(y*OUTPUT_DATA_WIDTH_HELIX)+x];
                            if (lpSplMemK->ulDataSize)
                                kVal=lpSplMemK->lpBuffer[(y*OUTPUT_DATA_WIDTH_HELIX)+x];
                            
                            rgbval=CMYK_to_RGB((BYTE)yVal,(BYTE)mVal,(BYTE)cVal,(BYTE)kVal,y2);
                            lpSrc = (PBYTE)pOutputSurface;
                            lpSrc += (y2*(pdev->epPaperXdots*3));
                            lpSrc += x2 * 3;
                            *lpSrc++ = GetRValue(rgbval);
                            *lpSrc++ = GetGValue(rgbval);
                            *lpSrc = GetBValue(rgbval);
                        }
                    
                }
                
                

                Resize (pdev,pOutputSurface, pPreviewSurface);

#if 0
                {
                    FILE *p = NULL;
                    BITMAPFILEHEADER bf;
                    BITMAPINFO bi;
                    RGBQUAD rgbq;
                    SURFOBJ * 	pso = psoDst;
                    PBYTE pOutputImage = (PBYTE)pso->pvBits;
                    char *file = "C:\\alook\\job\\arrggh.bmp";
                    size_t len = 0;
                    int x;
                    
                    memset (&bf, 0, sizeof (BITMAPFILEHEADER));
                    bf.bfType = 0x4d42;
                    bf.bfSize = pso->lDelta * pso->sizlBitmap.cy + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);// + 256 * sizeof(RGBQUAD);
                    bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFO);// + (255 * sizeof(RGBQUAD));
                    
                    memset (&bi, 0, sizeof (BITMAPINFO));
                    bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                    bi.bmiHeader.biWidth = pso->sizlBitmap.cx;
                    bi.bmiHeader.biHeight = pso->sizlBitmap.cy;
                    bi.bmiHeader.biPlanes = 1;
                    bi.bmiHeader.biBitCount = 24;
                    bi.bmiHeader.biCompression = 0;
                    bi.bmiHeader.biSizeImage = pso->lDelta * pso->sizlBitmap.cy;
                    
                    p = fopen(file, "wb");
                    if (p) 
                    {
                        fwrite((PBYTE)&bf, sizeof(BITMAPFILEHEADER), 1, p);
                        fwrite((PBYTE)&bi, sizeof(BITMAPINFO), 1, p);
                        /*       for (x=0; x<256; x++)
                         {
                         rgbq.rgbBlue = x;
                         rgbq.rgbGreen = x;
                         rgbq.rgbRed = x;
                         rgbq.rgbReserved = 0;
                         fwrite(&rgbq, 4, 1, p);
                         }*/
                        len = pso->lDelta * pso->sizlBitmap.cy;
                        fwrite(pOutputImage, len, 1, p);
                        fclose(p);
                        
                    } 
#endif	
                    
                    
                    for( row=0; row<PREVIEW_HEIGHT; row++ )
                    {
                        for( col=0; col<PREVIEW_WIDTH; col++ )
                        {
                            lpDst = (PBYTE)pPreviewSurface + (row * (PREVIEW_WIDTH * 3)) + (col * 3);
                            rgb888 = (ULONG)RGB(*lpDst,*(lpDst +1),*(lpDst + 2));
                            pdev->HelixPreview[(PREVIEW_HEIGHT-1) - row][col]=(WORD)RGB_565(rgb888);
                        }
                    }
                    //discard our created surfaces
                    free(pPreviewSurface);
                    free(pOutputSurface);
                }
                
                // Send the preview data
                OutputPlaneCommand(pdev,
                                   settings,
                                   (PBYTE)&pdev->HelixPreview,
                                   0, 
                                   (PREVIEW_HEIGHT * PREVIEW_WIDTH)*2);
                
            }

        } //ends if helix
    else
    {
    
        for (ulColour = 0 ; ulColour < 4; ulColour ++)
        {
            if (bFront)
            {
                lpSplMem = &pdev->lpSplMemFront[ulColour];
            }
            else
            {
                lpSplMem = &pdev->lpSplMemBack[ulColour];
            }

            if (lpSplMem->lpBuffer != NULL && lpSplMem->bDataInPlane)
            {
                OutputPlaneCommand(pdev, 
								   settings,
                                   lpSplMem->lpBuffer, 
                                   lpSplMem->ulColour, 
                                   lpSplMem->ulDataSize);
                bBlank = FALSE;
            }
        }
        
        //we have a blank card so print a blank card! removed due to firmware not feeding a card <groan>   
        if (bBlank && (pdev->magTrackInfo[MAG_TRACK_INDEX1].TrackData[0] == 0)
        && (pdev->magTrackInfo[MAG_TRACK_INDEX2].TrackData[0] == 0)
        && (pdev->magTrackInfo[MAG_TRACK_INDEX3].TrackData[0] == 0))
        {
			if (bFront)
				lpSplMem = &pdev->lpSplMemFront[PLANE_K];
			else
				lpSplMem = &pdev->lpSplMemBack[PLANE_K];
			
            OutputPlaneCommand(pdev, 
								settings,
                                lpSplMem->lpBuffer, 
                                lpSplMem->ulColour, 
                                97536/*lpSplMem->ulDataSize*/);
        }  
  
        OutputETXCommand(pdev);
    }
}


/*****************************************************************************
 *  SetDataSize()
 *      Send SZX commands.
 *      If we have not seen any colour in a plane, the command won't be out.
 *
 *  Returns:
 *      None
 *****************************************************************************/

VOID SetDataSize
(
    PDEVDATA pdev, // Pointer to our PDEV
    BOOL     bFront // TRUE = Front page (or simplex) : FALSE = Back page
)
{
////    LPDVPDEV       lpDrvData       = pdev->pDrvData;
    LPSPOOLMEMINFO lpSplMem        = NULL;
    ULONG          ulColour        = 0;
	BYTE           ColourMask      = 0;
    BOOL           bBlank = TRUE;

    ColourMask = GetDataPresent(pdev, bFront);

	for (ulColour = 0 ; ulColour < 4; ulColour ++)
    {
        if (bFront)
        {
            lpSplMem = &pdev->lpSplMemFront[ulColour];
        }
        else
        {
            lpSplMem = &pdev->lpSplMemBack[ulColour];
        }

		if (ColourMask & (1 << ulColour))
        {
            OutputPlaneDataSizeCommand( pdev, lpSplMem->ulDataSize, lpSplMem->ulColour/*, ulChannelOption*/ );
            
            //Clear bit for this colour
		    ColourMask &= ~(1 << ulColour);
		    if (ColourMask != 0)
		    {
			    //There are more colours to handle so output a comma
                OutputComma(pdev);
		    }
            bBlank = FALSE;
		}
    }

    //removed due to firmware not feeding a card <groan>   
    if (bBlank && (pdev->magTrackInfo[MAG_TRACK_INDEX1].TrackData[0] == 0)
    && (pdev->magTrackInfo[MAG_TRACK_INDEX2].TrackData[0] == 0)
    && (pdev->magTrackInfo[MAG_TRACK_INDEX3].TrackData[0] == 0))
    {
        //need to determine which plane is available on the printer CMY or K resin installed?
        lpSplMem = &pdev->lpSplMemFront[PLANE_K];
        OutputPlaneDataSizeCommand( pdev, 97536/*lpSplMem->ulDataSize*/, PLANE_K/*lpSplMem->ulColour*//*, ulChannelOption*/ );
    }
    OutputFileSeparator(pdev);
}

/*******************************************************************************
 *  NewPage()
 *      Initialize information per page.
 *
 *  Returns:
 *      True/False
*******************************************************************************/

BOOL NewPage
(
    PDEVDATA   pdev,       			// Pointer to our PDEV
    struct settings_ * settings,
    cups_page_header2_t header
)
{
    int         iCurrentPage = pdev->iPageNumber;
	
    GetDuplexInfo(pdev, settings, iCurrentPage);

    if (pdev->eChannelOption == UICBVAL_YMCK)
    {
        if (pdev->bFrontPage)
        {
            pdev->bBlackTextUseK        = settings->CF_BlackOptions_BlackTextUsesKResin;
            pdev->bBlackPolygonUseK     = settings->CF_BlackOptions_BlackPolygonsUseKResin;
            pdev->bMonoBitmapUseK       = settings->CF_BlackOptions_MonoBitmapsUseKResin;
            pdev->bPhotoUseYMC          = settings->CF_BlackOptions_PicturesUseYMConly;
            pdev->bDetectAdjacentColour = TRUE;//settings->bDetectAdjacentColour_Front;
        }
        else
        {
            pdev->bBlackTextUseK        = settings->CB_BlackOptions_BlackTextUsesKResin;
            pdev->bBlackPolygonUseK     = settings->CB_BlackOptions_BlackPolygonsUseKResin;
            pdev->bMonoBitmapUseK       = settings->CB_BlackOptions_MonoBitmapsUseKResin;
            pdev->bPhotoUseYMC          = settings->CB_BlackOptions_PicturesUseYMConly;
            pdev->bDetectAdjacentColour = TRUE;//settings->bDetectAdjacentColour_Back;
        }
    }

    if ((pdev->eChannelOption == UICBVAL_YMCK)
    ||  (pdev->eChannelOption == UICBVAL_KResin))
    {
        // Create Strip for KObject
        if (CreateKPrintingObject(pdev, settings, header) == FALSE)
		{
			// Failed to alloc buffer
			return FALSE;
		}
    }

    InitializeOutputBuffer(pdev, settings);

    if (pdev->eCallBackStatus == DCBS_FATALERROR)
    {
        // Failed to alloc buffer
        return FALSE;
    }

    AllocBuffer(pdev, settings, pdev->bFrontPage);

    if (pdev->eCallBackStatus == DCBS_FATALERROR)
    {
        // Failed to alloc buffer
        return FALSE;
    }

    pdev->bPageStarted = TRUE;

    return TRUE;

}

void pageSetup(PDEVDATA pdev, struct settings_ settings, cups_page_header2_t header)
{
DWORD x;
BOOL bBackPage = FALSE;
////BOOL bInitMagData = TRUE;
struct settings_ * psettings = &settings;

    
	fprintf(stderr, "CATULTRA: settings.Duplex = %d \n"    , settings.Duplex);	
	fprintf(stderr, "DEBUG: pageType = %d \n"    , settings.pageType);
	fprintf(stderr, "CATULTRA: settings.OEM = %d \n"    , settings.OEM);
	fprintf(stderr, "CATULTRA: settings.Printer = %d \n"    , settings.Printer);
    if (settings.OEM == OEM_PRO360)
    {
        fprintf(stderr, "CATULTRA: Pro360 - pageSetup\n ");
	pdev->xImage = 642;//1013;
        pdev->yImage = 1013;//642;
    }
    else		
	if (settings.OEM == OEM_HELIX)
    {
        pdev->xImage = 1036;//664;//pso->sizlBitmap.cx;
        pdev->yImage = 664;//1036;//pdev->epPaperXdots;//1016;//pso->sizlBitmap.cy;
    }
    else
    {
        pdev->xImage = 642;////648;//pso->sizlBitmap.cx;
        pdev->yImage = 1016;//pdev->epPaperXdots;//1016;//pso->sizlBitmap.cy;
    }

    pdev->epOffsetX = 0;
    pdev->epOffsetY = 0;
	
	 if (settings.OEM == OEM_PRO360)
    {
		//init the pro360 job crc value
		pdev->epCheckSum=0xffffffffu; 
	
        pdev->epPhysPaperXdots = 642;//1013;
        pdev->epPhysPaperYdots = 1013;//642;    
        pdev->epPaperXdots = 642;//1013;  //set to pro360 panel size
        pdev->epPaperYdots = 1013;//642;
    }
    else
    if (settings.OEM == OEM_HELIX)
    {
        pdev->epPhysPaperXdots = 1036;
        pdev->epPhysPaperYdots = 664;    
        pdev->epPaperXdots = 1036;  //set to optima panel size
        pdev->epPaperYdots = 664;   

        //sWidth = (pdev->epPaperXdots * 254) / pdmPrivate->xdpi;
        //sHeight = (pdev->epPaperYdots * 254) / pdmPrivate->ydpi;
    }
    else
    if ((settings.Printer == XXL ) && settings.XXL_ImageType > UICBVAL_DoubleImage)  //handle extended monochrome and tape for xxl
    {
        float fcx = (settings.PaperHeight * (float)300) / (float)254;
        int icx = (int) (fcx + 0.5f);
        pdev->epPhysPaperXdots = pdev->epPaperXdots = (short)icx;
        pdev->epPhysPaperYdots = pdev->epPaperYdots = 642;   //fixed for now..
         //1016 * 254 / 300
//        sWidth = (pdev->epPaperXdots * 254) / pdmPrivate->xdpi;
//        sHeight= 544;   
    }
	else
	if (settings.Printer == RIO_PRO_XTENDED || settings.Printer == PRIDENTOPROXTD || settings.Printer == PRICECARD_PRO_FLEX)
	{
	fprintf(stderr, "PIGULTRA L6452: settings.XXL_ImageType = %d \n"    , settings.XXL_ImageType);	
	fprintf(stderr, "PIGULTRA L6453: UICBVAL_DoubleImage = %d \n"    , UICBVAL_DoubleImage);	
		if (settings.Printer == PRICECARD_PRO_FLEX) //different index in PPD :(
		{

			if (settings.XXL_ImageType == 1 || settings.XXL_ImageType == 3 || settings.XXL_ImageType == 4)
			{
				float fcx = (settings.PaperHeight * (float)300) / (float)254;
				int icx = (int) (fcx + 0.5f);
				pdev->epPhysPaperXdots = pdev->epPaperXdots = (short)icx;
				pdev->epPhysPaperYdots = pdev->epPaperYdots = 642;   //fixed for now..
				pdev->xImage = 642;////648;//pso->sizlBitmap.cx;
				if ( psettings->pageType == 1) //extd mono card 140mm
					pdev->yImage = 1654;
				else
				if ( psettings->pageType == 3) //extd mono card 109mm
					pdev->yImage = 1287;
				else
				if ( psettings->pageType == 4) //extd mono card 128mm
					pdev->yImage = 1512;
	fprintf(stderr, "PIGULTRA: L6471 settings.PaperHeight = %d \n"    , settings.PaperHeight);		
	fprintf(stderr, "PIGULTRA: L6472 icx = %d \n"    , icx);			
				 //1016 * 254 / 300
				//sWidth = (pdev->epPaperXdots * 254) / pdmPrivate->xdpi;//862;//(SHORT)(pdev->CurForm.ImageArea.right - 
			//                               pdev->CurForm.ImageArea.left);
				//sHeight= 544;//(SHORT)(pdev->CurForm.ImageArea.bottom - 
			//                               pdev->CurForm.ImageArea.top);   
			}	
			else
			{
				pdev->epPhysPaperXdots = 1016;
				pdev->epPhysPaperYdots = 642;        
				pdev->epPaperXdots = 1016;  //set to cr80 size
				pdev->epPaperYdots = 642; 
	fprintf(stderr, "PIGULTRA: L6485 YUK.. = %d \n"    , settings.PaperHeight);		
	fprintf(stderr, "PIGULTRA: L6472 YUK = %d \n"    , 1);
			}			
		}	
		else
		{
			if (settings.XXL_ImageType > UICBVAL_DoubleImage)
			{
				float fcx = (settings.PaperHeight * (float)300) / (float)254;
				int icx = (int) (fcx + 0.5f);
				pdev->epPhysPaperXdots = pdev->epPaperXdots = (short)icx;
				pdev->epPhysPaperYdots = pdev->epPaperYdots = 642;   //fixed for now..
				pdev->xImage = 642;////648;//pso->sizlBitmap.cx;
				if ( psettings->pageType == 3) //extd mono card 140mm
					pdev->yImage = 1654;
				else
				if ( psettings->pageType == 4) //extd mono card 109mm
					pdev->yImage = 1287;
				else
				if ( psettings->pageType == 5) //extd mono card 128mm
					pdev->yImage = 1512;
	fprintf(stderr, "PIGULTRA: L6471 settings.PaperHeight = %d \n"    , settings.PaperHeight);		
	fprintf(stderr, "PIGULTRA: L6472 icx = %d \n"    , icx);			
				 //1016 * 254 / 300
				//sWidth = (pdev->epPaperXdots * 254) / pdmPrivate->xdpi;//862;//(SHORT)(pdev->CurForm.ImageArea.right - 
			//                               pdev->CurForm.ImageArea.left);
				//sHeight= 544;//(SHORT)(pdev->CurForm.ImageArea.bottom - 
			//                               pdev->CurForm.ImageArea.top);   
			}	
			else
			{
				pdev->epPhysPaperXdots = 1016;
				pdev->epPhysPaperYdots = 642;        
				pdev->epPaperXdots = 1016;  //set to cr80 size
				pdev->epPaperYdots = 642; 
	fprintf(stderr, "PIGULTRA: L6485 YUK.. = %d \n"    , settings.PaperHeight);		
	fprintf(stderr, "PIGULTRA: L6472 YUK = %d \n"    , 1);
			}
		}

	}
    else
    {

        if (settings.pageType == 3)  //cr80 white border
        {
            pdev->epPhysPaperXdots = 1016;
            pdev->epPhysPaperYdots = 642;
            pdev->epPaperXdots = 1016;//986;  //cr80 wb
            pdev->epPaperYdots = 642;//596;   
            pdev->epOffsetX = 0;
			pdev->epOffsetY = 23;
 
        }
        else
        {
            pdev->epPhysPaperXdots = 1016;
            pdev->epPhysPaperYdots = 642;        
            pdev->epPaperXdots = 1016;  //set to cr80 size
            pdev->epPaperYdots = 642; 
           

        }
        
    }

	
	pdev->iPaperHeight = settings.PaperHeight; //paper height required for xtended models
	
    //
    /*
     * Determine whether we are printing on the front or back of a card.
     * This depends on both the Duplex setting and physical page number.
     */
    if (settings.Duplex == UICBVAL_Duplex_BothSides)
    {
        if ((pdev->iPageNumber % 2) == 0)   // Even page
        {
            bBackPage = TRUE;
////            bInitMagData = FALSE;
        }
        else // Odd page
        {
            bBackPage = FALSE;
////            bInitMagData = TRUE;
        }
    }
    else if (settings.Duplex == UICBVAL_Duplex_Back)
    {
        bBackPage = TRUE;
    }
    else // lpds->eDuplex == UICBVAL_Duplex_Front
    {
        bBackPage = FALSE;
    }
//TODO
////    if (bInitMagData)
////        InitializeMagneticEncodeTrackInfo(pdev);

    /*
     * Record various UICBs for use in evaluating driver dependencies when we call ModelUICBtoDM() below.
     * These depend on whether we are currently printing a back or front page.
     */
    if (bBackPage)
    {
        pdev->eChannelOption    = GetChannelOption(pdev, &settings, BACK);
        pdev->eBlackStartOption = settings.CB_BlackOptions_AllBlackAs;
        pdev->eOrientation      = settings.CB_CardOrient;
        pdev->eHalftoning       = settings.CB_BlackOptions_Halftone;
        pdev->NoBlackAreas		= 0;
        if (settings.CB_NoBlackAreas < MAX_BLACK_AREAS)
        {        
            for (x=0; x<settings.CB_NoBlackAreas; x++)
            {
                pdev->BlackAreas[x].left		= settings.CB_BlackAreas[x].left;
                pdev->BlackAreas[x].top			= settings.CB_BlackAreas[x].bottom;
                pdev->BlackAreas[x].right		= settings.CB_BlackAreas[x].left + settings.CB_BlackAreas[x].width;
                pdev->BlackAreas[x].bottom		= settings.CB_BlackAreas[x].bottom + settings.CB_BlackAreas[x].height;
                pdev->NoBlackAreas ++;
            }		
        }
    }
    else
    {
        pdev->eChannelOption    = GetChannelOption(pdev, &settings, FRONT);
        pdev->eBlackStartOption = settings.CF_BlackOptions_AllBlackAs;
        pdev->eOrientation      = settings.CF_CardOrient;
        pdev->eHalftoning       = settings.CF_BlackOptions_Halftone;
		pdev->NoBlackAreas		= 0;
		
		if (settings.CF_NoBlackAreas < MAX_BLACK_AREAS)
        {
            for (x=0; x<settings.CF_NoBlackAreas; x++)
            {
                if (settings.CF_BlackAreas[x].left == 0 && 
                    settings.CF_BlackAreas[x].bottom == 0 &&
                    settings.CF_BlackAreas[x].width == 0 &&
                    settings.CF_BlackAreas[x].height == 0)
                        continue;
                    
                    pdev->BlackAreas[x].left		= settings.CF_BlackAreas[x].left;
                    pdev->BlackAreas[x].top			= 642 - settings.CF_BlackAreas[x].bottom - settings.CF_BlackAreas[x].height;
                    pdev->BlackAreas[x].right		= settings.CF_BlackAreas[x].left + settings.CF_BlackAreas[x].width;
                    pdev->BlackAreas[x].bottom		= 642 - settings.CF_BlackAreas[x].bottom;
                    pdev->NoBlackAreas ++;
            }
        }
    }
/*	DBGMSG1(DBG_LEVEL_VERBOSE,"pdev->BlackAreas[0].left:%u\n",pdev->BlackAreas[0].left );
	DBGMSG1(DBG_LEVEL_VERBOSE,"pdev->BlackAreas[0].top:%u\n",pdev->BlackAreas[0].top );
	DBGMSG1(DBG_LEVEL_VERBOSE,"pdev->BlackAreas[0].right:%u\n",pdev->BlackAreas[0].right );
	DBGMSG1(DBG_LEVEL_VERBOSE,"pdev->BlackAreas[0].bottom:%u\n",pdev->BlackAreas[0].bottom );*/
    /*
     * bImageClipped becomes TRUE (below) if the orientation differs between
     * the application and the driver.
     */
    pdev->bImageClipped = FALSE;
	if (settings.orientation == 0)//header.cupsHeight != 1016 && header.cupsHeight != 991 && header.cupsHeight != 1654)
//	if (header.Orientation == 0/*DMORIENT_PORTRAIT*/)
		pdev->iOrientation = UICBVAL_Portrait;
	else
		pdev->iOrientation = UICBVAL_Landscape;

	// Record rotate setting
    if (bBackPage)
    {
        if (pdev->iOrientation == UICBVAL_Portrait)
        {
            /*
             * Please note that with portrait printing on the back side, the device flips the card...
             * Logical page: No rotate  -> Physical result 180 degrees rotation against the first page
             * Logical page: 180 rotate -> Physical result no rotation against the first page
             */
            pdev->bRotate = !settings.CB_Rotate180;//lpds->bRotate_Back;
        }
        else // Landscape on the back side
        {
            pdev->bRotate = settings.CB_Rotate180;//lpds->bRotate_Back;
        }
    }
    else // Front page
    {
        pdev->bRotate = settings.CF_Rotate180;//lpds->bRotate_Front;
    }

//#if OEM_ID == UICBVAL_OEM_Enduro
    //we default to a TOPDOWN surface which is the opposite of the existing 
    if (RIO_OEM(psettings))
        pdev->bRotate = !pdev->bRotate;
    else
    if (ENDURO_OEM(psettings))
    {
        if (RIO_PRO_X_MODEL(psettings) )
        {
/*            //Enduro printing is rotated by 180 degrees relative to other printers
//            pdev->bRotate = !pdev->bRotate;
            pdev->bRotate = pdev->bRotate;
        }
        else
        {*/
            //Its a Rio Pro Xtended -
            //If image is Extended Colour Card:
            //	Landscape: Do not rotate either front or back
            //	Portrait:  Rotate the front page
            if ((settings.XXL_ImageType == UICBVAL_DoubleImage)
            &&  (settings.orientation == 0/*DMORIENT_PORTRAIT*/)
            &&  (bBackPage == FALSE))
            {
				//Rotate the front page
				pdev->bRotate = pdev->bRotate;
            }
            else
                pdev->bRotate = !pdev->bRotate;
        }
pdev->bRotate = !pdev->bRotate;
    }

//#endif
	//create our 32bit surface
	x = (pdev->xImage * 4) * pdev->yImage;
fprintf(stderr, "CATULTRA pdev->bRotate:%u pdev->xImage: %u\n",pdev->bRotate, pdev->xImage);
fprintf(stderr, "CATULTRA pdev->yImage: %u\n",pdev->yImage);
fprintf(stderr, "CATULTRA x: %x\n",x);
	pdev->pso32bitSurface = malloc(x * 2);
	
    if (NewPage(pdev, psettings, header) == FALSE)
	{
		//DBGMSG(DBG_LEVEL_VERBOSE,"Error creating surface\n");
		return ;
	}
	
}
/*******************************************************************************
 *  DeleteKPrintingObject()
 *      Delete objects which were created at CreateKPrintingObject(),
 *
 *  Returns:
 *      None
*******************************************************************************/

VOID DeleteKPrintingObject
(
    PDEVDATA pdev  // Pointer to our PDEV
)
{
    //8bpp...
    if (pdev->lpKSrcStrip != NULL)
    {
        free(pdev->lpKSrcStrip);
        pdev->lpKSrcStrip = NULL;
    }

    if (pdev->lpKPSrcStrip != NULL)
    {
        free(pdev->lpKPSrcStrip);
        pdev->lpKPSrcStrip = NULL;
    }
	
    if (pdev->lpKDstStrip != NULL)
    {
        free(pdev->lpKDstStrip);
        pdev->lpKDstStrip = NULL;
    }

    //1bpp
    if (pdev->lpHalftone != NULL)
    {
        free(pdev->lpHalftone);
        pdev->lpHalftone = NULL;
    }

}

#define BITS_FOR_COLOUR_DATA 6
#define BITS_FOR_BLACK_DATA  1
#define DEFAULT_PRINTHEAD_POSN 50
#define PRINTHEAD_WIDTH        672

//Full Bleed CR80 Card Sizing
#define FULL_BLEED_WIDTH       642
#define FULL_BLEED_MARGIN      (PRINTHEAD_WIDTH - FULL_BLEED_WIDTH)/2
#define FULL_BLEED_MAX_MARGIN  (DEFAULT_PRINTHEAD_POSN + FULL_BLEED_MARGIN)
#define FULL_BLEED_MIN_MARGIN  (DEFAULT_PRINTHEAD_POSN - FULL_BLEED_MARGIN)

//White Border CR80 Card Sizing
#define BORDER_CARD_WIDTH      596
#define BORDER_CARD_MARGIN     (PRINTHEAD_WIDTH - BORDER_CARD_WIDTH)/2
#define BORDER_CARD_MAX_MARGIN (DEFAULT_PRINTHEAD_POSN + BORDER_CARD_MARGIN)
#define BORDER_CARD_MIN_MARGIN (DEFAULT_PRINTHEAD_POSN - BORDER_CARD_MARGIN)

#define MARGIN(x)		(PRINTHEAD_WIDTH - (x))/2
#define MAX_MARGIN(x)	(DEFAULT_PRINTHEAD_POSN + MARGIN((x)))
#define MIN_MARGIN(x)	(DEFAULT_PRINTHEAD_POSN - MARGIN((x)))
#define RJE_FULL_BLEED_MAX_MARGIN	MAX_MARGIN(FULL_BLEED_WIDTH)
#define RJE_FULL_BLEED_MIN_MARGIN	MIN_MARGIN(FULL_BLEED_WIDTH)
#define RJE_WHITE_BORDER_MAX_MARGIN	MAX_MARGIN(BORDER_CARD_WIDTH)
#define RJE_WHITE_BORDER_MIN_MARGIN	MIN_MARGIN(BORDER_CARD_WIDTH)


//#############################################################################

DWORD GetPrintheadPosition
(
    PDEVDATA pdev,      // Pointer to our PDEVICE
	PSETTINGS pSettings
)
{
	DWORD	  dwPrintheadPosition = DEFAULT_PRINTHEAD_POSITION;

	if (pSettings->es.iPrintHeadPosn)
		dwPrintheadPosition = pSettings->es.iPrintHeadPosn;

	return dwPrintheadPosition;
}

//#############################################################################
//##                                                                         ##
//##                              RIO2 PRINTHEAD                             ##
//##                                                                         ##
//#############################################################################

/******************************************************************************
 *  format_printhead_data_KEE()
 *      Processes data line by line and places it into the Magicard Rio/Tango 2e
 *      or Avalon Printhead Format.
 *
 *  Returns:
 *     None
 ******************************************************************************/

void format_printhead_data_KEE
(
    PDEVDATA pdev,     // Pointer to our PDEVICE
	PSETTINGS psettings,
    LPBYTE   lpData,    // Input Data Buffer
    LPBYTE   lpOutBuff, // Output Data Buffer
    int      nNoOfBits, // Colour or Monochrome data
    int      nSize      // Size of dOutput Data Buffer
)
{
    int      pixel          = 0;
    int      y_index        = 0;
    int      line_length    = 0;
    int      print_bit      = 0;
    int      margin_index   = 0;
    int      a              = 0;
    DWORD    density        = 0;
    DWORD *  temp           = 0;
    DWORD    dwtemp         = 0;
    DWORD    temp_word[BITS_FOR_COLOUR_DATA];
    
    //registry setting aquired from language monitor value returned..
    //other value from UI..range 15 to -15...
    int nPrintheadPosition  = GetPrintheadPosition(pdev,psettings) - psettings->nImagePosition_UpDown;

		                      
    memset(lpOutBuff, 0, nSize);

    if (psettings->pageType == 3/*DMPAPER_WHITEBORDER*/)    
    {
        nPrintheadPosition = MIN(nPrintheadPosition, BORDER_CARD_MAX_MARGIN);
        nPrintheadPosition = MAX(nPrintheadPosition, BORDER_CARD_MIN_MARGIN);
        line_length        = BORDER_CARD_WIDTH;
        margin_index       = BORDER_CARD_MAX_MARGIN - nPrintheadPosition;
    }
    else
    {
        nPrintheadPosition = MIN(nPrintheadPosition, FULL_BLEED_MAX_MARGIN);
        nPrintheadPosition = MAX(nPrintheadPosition, FULL_BLEED_MIN_MARGIN);
        line_length        = FULL_BLEED_WIDTH;
        margin_index       = FULL_BLEED_MAX_MARGIN - nPrintheadPosition;
    }

    temp = (DWORD *)(lpOutBuff);

	if (nNoOfBits > 1)
    {
        while (y_index < PRINTHEAD_WIDTH)
        {
            memset((LPBYTE)temp_word, 0, sizeof(temp_word));

            for (print_bit = 0; print_bit < 32; print_bit++)
            {
                if ((y_index++ > margin_index) && (pixel < line_length))
                {
                    pixel++;
                    density = *lpData++;
                    temp_word[0] |= ((density & 0x01) << print_bit);
                    density >>= 1;
                    temp_word[1] |= ((density & 0x01) << print_bit);
                    density >>= 1;
                    temp_word[2] |= ((density & 0x01) << print_bit);
                    density >>= 1;
                    temp_word[3] |= ((density & 0x01) << print_bit);
                    density >>= 1;
                    temp_word[4] |= ((density & 0x01) << print_bit);
                    density >>= 1;
                    temp_word[5] |= ((density & 0x01) << print_bit);
                }
            }

            *temp |= temp_word[0];
            temp += 21;
            *temp |= temp_word[1];
            temp += 21;
            *temp |= temp_word[2];
            temp += 21;
            *temp |= temp_word[3];
            temp += 21;
            *temp |= temp_word[4];
            temp += 21;
            *temp |= temp_word[5];
            temp -= 104; // - (5 * 21) + 1
        }// end while ( pixel < PRINTHEAD_WIDTH )
    }
    else
    {
        BYTE bSrcBit = 0x80;

        while (y_index < PRINTHEAD_WIDTH)
        {
            temp_word[0] = 0;
            for (print_bit = 0; print_bit < 32; print_bit++)
            {
                if (((y_index++) > margin_index) && (pixel < line_length))
                {
                    if (*lpData & bSrcBit)
                    {
                        temp_word[0] |= 0x01 << print_bit;
                    }

                    if (bSrcBit == 0x01)
                    {
                        lpData++;
                        bSrcBit = 0x80;
                    }
                    else
                    {
                        bSrcBit >>= 1;
                    }

                    pixel++;
                }
            }
            *temp |= temp_word[0];
            temp++;
        }
    }

    // Re-order bytes/bits
    if (AVALON_MODEL(psettings))
	{
        /*
         * DWORD order will be changed here
         * (7-0) (15-8) (23-16) (31-24)
         */
        for (a = 0; a < (nNoOfBits * 21); a++)
        {
            dwtemp = 0;
            temp = (DWORD *) (lpOutBuff + a * 4);
            dwtemp |=  *temp >> 24;
            dwtemp |= (*temp >> 8)  & 0x0000FF00;

            dwtemp |= (*temp << 8)  & 0x00FF0000;
            dwtemp |= (*temp << 24) & 0xFF000000;
            *temp = dwtemp;
		}
	}
	else
	{
        for (a = 0; a < (nNoOfBits * 21); a++)
        {
            temp = (DWORD *)(lpOutBuff + (a * 4));
            *temp = (((*temp & 0xaaaaaaaa) >> 1) | ((*temp & 0x55555555) << 1));
            *temp = (((*temp & 0xcccccccc) >> 2) | ((*temp & 0x33333333) << 2));
            *temp = (((*temp & 0xf0f0f0f0) >> 4) | ((*temp & 0x0f0f0f0f) << 4));
		}
    }
}

//#############################################################################
//##                                                                         ##
//##                              ALTO PRINTHEAD                             ##
//##                                                                         ##
//#############################################################################

#define PIXELS_PART_1 288

typedef struct {
    int first_bit;
	int first_word;
} PIXEL_LAYOUT;

//This table defines the pixel organisation for the data format for the KGE printhead
//For pixels 1 to 288, the table is defined as:
//      print_format[a].first_bit  = (a % 16) * 2;
//      print_format[a].first_word = (a / 16) + 6;
//For pixels 289 to 672 the table is defined as
//      print_format[a].first_bit  = ((a - 288) % 16) * 2 + 1;
//      print_format[a].first_word =  (a - 288)/ 16;

const PIXEL_LAYOUT print_format[PRINTHEAD_WIDTH] = {
{0,  6},  {2,  6},  {4,  6},  {6,  6},  {8,  6},  {10, 6},  {12, 6},  {14, 6},   //Pixels   0 -   7
{16, 6},  {18, 6},  {20, 6},  {22, 6},  {24, 6},  {26, 6},  {28, 6},  {30, 6},   //Pixels   8 -  15
{0,  7},  {2,  7},  {4,  7},  {6,  7},  {8,  7},  {10, 7},  {12, 7},  {14, 7},   //Pixels  16 -  23
{16, 7},  {18, 7},  {20, 7},  {22, 7},  {24, 7},  {26, 7},  {28, 7},  {30, 7},   //Pixels  24 -  31
{0,  8},  {2,  8},  {4,  8},  {6,  8},  {8,  8},  {10, 8},  {12, 8},  {14, 8},   //Pixels  32 -  39
{16, 8},  {18, 8},  {20, 8},  {22, 8},  {24, 8},  {26, 8},  {28, 8},  {30, 8},   //Pixels  40 -  47
{0,  9},  {2,  9},  {4,  9},  {6,  9},  {8,  9},  {10, 9},  {12, 9},  {14, 9},   //Pixels  48 -  55
{16, 9},  {18, 9},  {20, 9},  {22, 9},  {24, 9},  {26, 9},  {28, 9},  {30, 9},   //Pixels  56 -  63
{0,  10}, {2,  10}, {4,  10}, {6,  10}, {8, 10},  {10, 10}, {12, 10}, {14, 10},  //Pixels  64 -  71
{16, 10}, {18, 10}, {20, 10}, {22, 10}, {24, 10}, {26, 10}, {28, 10}, {30, 10},  //Pixels  72 -  79
{0,  11}, {2,  11}, {4,  11}, {6,  11}, {8, 11},  {10, 11}, {12, 11}, {14, 11},  //Pixels  80 -  87
{16, 11}, {18, 11}, {20, 11}, {22, 11}, {24, 11}, {26, 11}, {28, 11}, {30, 11},  //Pixels  88 -  95
{0,  12}, {2,  12}, {4,  12}, {6,  12}, {8, 12},  {10, 12}, {12, 12}, {14, 12},  //Pixels  96 - 103
{16, 12}, {18, 12}, {20, 12}, {22, 12}, {24, 12}, {26, 12}, {28, 12}, {30, 12},  //Pixels 104 - 111
{0,  13}, {2,  13}, {4,  13}, {6,  13}, {8, 13},  {10, 13}, {12, 13}, {14, 13},  //Pixels 112 - 119
{16, 13}, {18, 13}, {20, 13}, {22, 13}, {24, 13}, {26, 13}, {28, 13}, {30, 13},  //Pixels 120 - 127
{0,  14}, {2,  14}, {4,  14}, {6,  14}, {8, 14},  {10, 14}, {12, 14}, {14, 14},  //Pixels 128 - 135
{16, 14}, {18, 14}, {20, 14}, {22, 14}, {24, 14}, {26, 14}, {28, 14}, {30, 14},  //Pixels 136 - 143
{0,  15}, {2,  15}, {4,  15}, {6,  15}, {8, 15},  {10, 15}, {12, 15}, {14, 15},  //Pixels 144 - 151
{16, 15}, {18, 15}, {20, 15}, {22, 15}, {24, 15}, {26, 15}, {28, 15}, {30, 15},  //Pixels 152 - 159
{0,  16}, {2,  16}, {4,  16}, {6,  16}, {8, 16},  {10, 16}, {12, 16}, {14, 16},  //Pixels 160 - 167
{16, 16}, {18, 16}, {20, 16}, {22, 16}, {24, 16}, {26, 16}, {28, 16}, {30, 16},  //Pixels 168 - 175
{0,  17}, {2,  17}, {4,  17}, {6,  17}, {8, 17},  {10, 17}, {12, 17}, {14, 17},  //Pixels 176 - 183
{16, 17}, {18, 17}, {20, 17}, {22, 17}, {24, 17}, {26, 17}, {28, 17}, {30, 17},  //Pixels 184 - 191
{0,  18}, {2,  18}, {4,  18}, {6,  18}, {8, 18},  {10, 18}, {12, 18}, {14, 18},  //Pixels 192 - 199
{16, 18}, {18, 18}, {20, 18}, {22, 18}, {24, 18}, {26, 18}, {28, 18}, {30, 18},  //Pixels 200 - 207
{0,  19}, {2,  19}, {4,  19}, {6,  19}, {8, 19},  {10, 19}, {12, 19}, {14, 19},  //Pixels 208 - 215
{16, 19}, {18, 19}, {20, 19}, {22, 19}, {24, 19}, {26, 19}, {28, 19}, {30, 19},  //Pixels 216 - 223
{0,  20}, {2,  20}, {4,  20}, {6,  20}, {8, 20},  {10, 20}, {12, 20}, {14, 20},  //Pixels 224 - 231
{16, 20}, {18, 20}, {20, 20}, {22, 20}, {24, 20}, {26, 20}, {28, 20}, {30, 20},  //Pixels 232 - 239
{0,  21}, {2,  21}, {4,  21}, {6,  21}, {8, 21},  {10, 21}, {12, 21}, {14, 21},  //Pixels 240 - 247
{16, 21}, {18, 21}, {20, 21}, {22, 21}, {24, 21}, {26, 21}, {28, 21}, {30, 21},  //Pixels 248 - 255
{0,  22}, {2,  22}, {4,  22}, {6,  22}, {8, 22},  {10, 22}, {12, 22}, {14, 22},  //Pixels 256 - 263
{16, 22}, {18, 22}, {20, 22}, {22, 22}, {24, 22}, {26, 22}, {28, 22}, {30, 22},  //Pixels 264 - 271
{0,  23}, {2,  23}, {4,  23}, {6,  23}, {8, 23},  {10, 23}, {12, 23}, {14, 23},  //Pixels 272 - 279
{16, 23}, {18, 23}, {20, 23}, {22, 23}, {24, 23}, {26, 23}, {28, 23}, {30, 23},  //Pixels 280 - 287
{1,  0},  {3,  0},  {5,  0},  {7,  0},  {9, 0},   {11, 0},  {13, 0},  {15, 0},   //Pixels 288 - 295
{17, 0},  {19, 0},  {21, 0},  {23, 0},  {25, 0},  {27, 0},  {29, 0},  {31, 0},   //Pixels 296 - 303
{1,  1},  {3,  1},  {5,  1},  {7,  1},  {9, 1},   {11, 1},  {13, 1},  {15, 1},   //Pixels 304 - 311
{17, 1},  {19, 1},  {21, 1},  {23, 1},  {25, 1},  {27, 1},  {29, 1},  {31, 1},   //Pixels 312 - 319
{1,  2},  {3,  2},  {5,  2},  {7,  2},  {9, 2},   {11, 2},  {13, 2},  {15, 2},   //Pixels 320 - 327
{17, 2},  {19, 2},  {21, 2},  {23, 2},  {25, 2},  {27, 2},  {29, 2},  {31, 2},   //Pixels 328 - 335
{1,  3},  {3,  3},  {5,  3},  {7,  3},  {9, 3},   {11, 3},  {13, 3},  {15, 3},   //Pixels 336 - 343
{17, 3},  {19, 3},  {21, 3},  {23, 3},  {25, 3},  {27, 3},  {29, 3},  {31, 3},   //Pixels 344 - 351
{1,  4},  {3,  4},  {5,  4},  {7,  4},  {9, 4},   {11, 4},  {13, 4},  {15, 4},   //Pixels 352 - 359
{17, 4},  {19, 4},  {21, 4},  {23, 4},  {25, 4},  {27, 4},  {29, 4},  {31, 4},   //Pixels 360 - 367
{1,  5},  {3,  5},  {5,  5},  {7,  5},  {9, 5},   {11, 5},  {13, 5},  {15, 5},   //Pixels 368 - 375
{17, 5},  {19, 5},  {21, 5},  {23, 5},  {25, 5},  {27, 5},  {29, 5},  {31, 5},   //Pixels 376 - 383
{1,  6},  {3,  6},  {5,  6},  {7,  6},  {9, 6},   {11, 6},  {13, 6},  {15, 6},   //Pixels 384 - 391
{17, 6},  {19, 6},  {21, 6},  {23, 6},  {25, 6},  {27, 6},  {29, 6},  {31, 6},   //Pixels 392 - 399
{1,  7},  {3,  7},  {5,  7},  {7,  7},  {9, 7},   {11, 7},  {13, 7},  {15, 7},   //Pixels 400 - 407
{17, 7},  {19, 7},  {21, 7},  {23, 7},  {25, 7},  {27, 7},  {29, 7},  {31, 7},   //Pixels 408 - 415
{1,  8},  {3,  8},  {5,  8},  {7,  8},  {9, 8},   {11, 8},  {13, 8},  {15, 8},   //Pixels 416 - 423
{17, 8},  {19, 8},  {21, 8},  {23, 8},  {25, 8},  {27, 8},  {29, 8},  {31, 8},   //Pixels 424 - 431
{1,  9},  {3,  9},  {5,  9},  {7,  9},  {9, 9},   {11, 9},  {13, 9},  {15, 9},   //Pixels 432 - 439
{17, 9},  {19, 9},  {21, 9},  {23, 9},  {25, 9},  {27, 9},  {29, 9},  {31, 9},   //Pixels 440 - 447
{1,  10}, {3,  10}, {5,  10}, {7,  10}, {9, 10},  {11, 10}, {13, 10}, {15, 10},  //Pixels 448 - 455
{17, 10}, {19, 10}, {21, 10}, {23, 10}, {25, 10}, {27, 10}, {29, 10}, {31, 10},  //Pixels 456 - 463
{1,  11}, {3,  11}, {5,  11}, {7,  11}, {9, 11},  {11, 11}, {13, 11}, {15, 11},  //Pixels 464 - 471
{17, 11}, {19, 11}, {21, 11}, {23, 11}, {25, 11}, {27, 11}, {29, 11}, {31, 11},  //Pixels 472 - 479
{1,  12}, {3,  12}, {5,  12}, {7,  12}, {9, 12},  {11, 12}, {13, 12}, {15, 12},  //Pixels 480 - 487
{17, 12}, {19, 12}, {21, 12}, {23, 12}, {25, 12}, {27, 12}, {29, 12}, {31, 12},  //Pixels 488 - 495
{1,  13}, {3,  13}, {5,  13}, {7,  13}, {9, 13},  {11, 13}, {13, 13}, {15, 13},  //Pixels 496 - 503
{17, 13}, {19, 13}, {21, 13}, {23, 13}, {25, 13}, {27, 13}, {29, 13}, {31, 13},  //Pixels 504 - 511
{1,  14}, {3,  14}, {5,  14}, {7,  14}, {9, 14},  {11, 14}, {13, 14}, {15, 14},  //Pixels 512 - 519
{17, 14}, {19, 14}, {21, 14}, {23, 14}, {25, 14}, {27, 14}, {29, 14}, {31, 14},  //Pixels 520 - 527
{1,  15}, {3,  15}, {5,  15}, {7,  15}, {9, 15},  {11, 15}, {13, 15}, {15, 15},  //Pixels 528 - 535
{17, 15}, {19, 15}, {21, 15}, {23, 15}, {25, 15}, {27, 15}, {29, 15}, {31, 15},  //Pixels 536 - 543
{1,  16}, {3,  16}, {5,  16}, {7,  16}, {9, 16},  {11, 16}, {13, 16}, {15, 16},  //Pixels 544 - 551
{17, 16}, {19, 16}, {21, 16}, {23, 16}, {25, 16}, {27, 16}, {29, 16}, {31, 16},  //Pixels 552 - 559
{1,  17}, {3,  17}, {5,  17}, {7,  17}, {9, 17},  {11, 17}, {13, 17}, {15, 17},  //Pixels 560 - 567
{17, 17}, {19, 17}, {21, 17}, {23, 17}, {25, 17}, {27, 17}, {29, 17}, {31, 17},  //Pixels 568 - 575
{1,  18}, {3,  18}, {5,  18}, {7,  18}, {9, 18},  {11, 18}, {13, 18}, {15, 18},  //Pixels 576 - 583
{17, 18}, {19, 18}, {21, 18}, {23, 18}, {25, 18}, {27, 18}, {29, 18}, {31, 18},  //Pixels 584 - 591
{1,  19}, {3,  19}, {5,  19}, {7,  19}, {9, 19},  {11, 19}, {13, 19}, {15, 19},  //Pixels 592 - 599
{17, 19}, {19, 19}, {21, 19}, {23, 19}, {25, 19}, {27, 19}, {29, 19}, {31, 19},  //Pixels 600 - 607
{1,  20}, {3,  20}, {5,  20}, {7,  20}, {9, 20},  {11, 20}, {13, 20}, {15, 20},  //Pixels 608 - 615
{17, 20}, {19, 20}, {21, 20}, {23, 20}, {25, 20}, {27, 20}, {29, 20}, {31, 20},  //Pixels 616 - 623
{1,  21}, {3,  21}, {5,  21}, {7,  21}, {9, 21},  {11, 21}, {13, 21}, {15, 21},  //Pixels 624 - 631
{17, 21}, {19, 21}, {21, 21}, {23, 21}, {25, 21}, {27, 21}, {29, 21}, {31, 21},  //Pixels 632 - 639
{1,  22}, {3,  22}, {5,  22}, {7,  22}, {9, 22},  {11, 22}, {13, 22}, {15, 22},  //Pixels 640 - 647
{17, 22}, {19, 22}, {21, 22}, {23, 22}, {25, 22}, {27, 22}, {29, 22}, {31, 22},  //Pixels 648 - 655
{1,  23}, {3,  23}, {5,  23}, {7,  23}, {9, 23},  {11, 23}, {13, 23}, {15, 23},  //Pixels 656 - 663
{17, 23}, {19, 23}, {21, 23}, {23, 23}, {25, 23}, {27, 23}, {29, 23}, {31, 23}   //Pixels 664 - 671
};

/******************************************************************************
 *  format_printhead_data_KGE()
 *      Processes data line by line and places it into the Magicard Alto/Opera/Tempo
 *      or Enduro Printhead Format.
 *
 *  Returns:
 *     None
 *
 ******************************************************************************/

void format_printhead_data_KGE
(
    PDEVDATA pdev,     // Pointer to our PDEVICE
	PSETTINGS psettings,
    PBYTE    lpData,    // Input Data Buffer
    PBYTE    lpOutBuff, // Output Data Buffer
    int      nNoOfBits, // Colour or Monochrome data
    int      nSize      // Size of Output Data Buffer
)
{
	int     margin_index;
    int     line_length = 0;
	int     pixel = 0;
	int     first_word;
	int     first_bit;
	int     change_point = PIXELS_PART_1;
	int     print_bit;
	int     which_bit = 0;
	int     a;
    DWORD   temp_word[BITS_FOR_COLOUR_DATA];
	DWORD   density;
    DWORD   dwtemp;
    LPDWORD temp = NULL;

    int nPrintheadPosition  = GetPrintheadPosition(pdev,psettings) - psettings->nImagePosition_UpDown;
		                      //+ GetUICBDocumentProperties(pPDev->lpdm)->iPrintheadPosition;
//myWrite(pdev, "DEBUG", 5);
//myWrite(pdev, lpData, FULL_BLEED_WIDTH);

	memset(lpOutBuff, 0, nSize);

    if (psettings->pageType == 3/*DMPAPER_WHITEBORDER*/)   
    {
        nPrintheadPosition = MIN(nPrintheadPosition, BORDER_CARD_MAX_MARGIN);
        nPrintheadPosition = MAX(nPrintheadPosition, BORDER_CARD_MIN_MARGIN);
        line_length        = BORDER_CARD_WIDTH;
        margin_index       = BORDER_CARD_MAX_MARGIN - nPrintheadPosition;
    }
    else
    {
        nPrintheadPosition = MIN(nPrintheadPosition, FULL_BLEED_MAX_MARGIN);
        nPrintheadPosition = MAX(nPrintheadPosition, FULL_BLEED_MIN_MARGIN);
        line_length        = FULL_BLEED_WIDTH;
        margin_index       = FULL_BLEED_MAX_MARGIN - nPrintheadPosition;
    }
	
    if (nNoOfBits > 1)
    {
        while (pixel < line_length)
        {
            first_word = print_format[margin_index + pixel].first_word;
            first_bit  = print_format[margin_index + pixel].first_bit;
            temp = (LPDWORD)(lpOutBuff + (first_word * 4));

            while ( (pixel < line_length)
			   &&  ((margin_index + pixel) < change_point))
            {
                memset((LPBYTE)temp_word, 0, sizeof(temp_word));
                for (print_bit = first_bit;
                     ((margin_index + pixel) < change_point)
					   && (print_bit < 32)
					   && (pixel < line_length);
                     print_bit += 2)
                {
                    pixel++;
                    density = *lpData++;
					temp_word[0] |= ((density & 0x01) << print_bit);
                    density >>= 1;
                    temp_word[1] |= ((density & 0x01) << print_bit);
                    density >>= 1;
                    temp_word[2] |= ((density & 0x01) << print_bit);
                    density >>= 1;
                    temp_word[3] |= ((density & 0x01) << print_bit);
                    density >>= 1;
                    temp_word[4] |= ((density & 0x01) << print_bit);
                    density >>= 1;
                    temp_word[5] |= ((density & 0x01) << print_bit);
                 }

				 *temp |= temp_word[0];
                 temp += 24;
                 *temp |= temp_word[1];
                 temp += 24;
                 *temp |= temp_word[2];
                 temp += 24;
                 *temp |= temp_word[3];
                 temp += 24;
                 *temp |= temp_word[4];
                 temp += 24;
                 *temp |= temp_word[5];
                 temp -= 119; // - ((24 * 5) - 1)

                 first_bit = which_bit;

            } //end while ((margin_index + pixel ) < change_point)

            change_point = margin_index + line_length;
            which_bit = 1;

        } //end while  y_index < line_length
    }
    else
    {
        BYTE bSrcBit = 0x80;

        //1 bit resin data
         while (pixel < line_length)
        {
            first_word = print_format[margin_index + pixel].first_word;
            first_bit  = print_format[margin_index + pixel].first_bit;
            temp = (LPDWORD)(lpOutBuff + (first_word * 4));

            while ((pixel < line_length) && ((margin_index + pixel) < change_point))
            {
                temp_word[0] = 0;
                for (print_bit = first_bit;
		             ((margin_index + pixel) < change_point)
					   && (print_bit < 32)
					   && (pixel < line_length);
		             print_bit += 2)
                {
                    if (*lpData & bSrcBit)
                    {
                        temp_word[0] |= 0x01 << print_bit;
                    }

                    if (bSrcBit == 0x01)
                    {
                        lpData++;
                        bSrcBit = 0x80;
                    }
                    else
                    {
                        bSrcBit >>= 1;
                    }

                    pixel++;
                }
                *temp |= temp_word[0];
                first_bit = which_bit;
                temp++;
            }
            change_point = margin_index + line_length;
            which_bit =  1;
        } //end for printhead_section
    }

    if (ENDURO_OEM(psettings))
	{
        for (a = 0; a < (nNoOfBits * 24); a++)
        {
            temp = (LPDWORD)(lpOutBuff + (a * 4));
            *temp = (((*temp & 0xaaaaaaaa) >> 1)  | ((*temp & 0x55555555) << 1));
            *temp = (((*temp & 0xcccccccc) >> 2)  | ((*temp & 0x33333333) << 2));
            *temp = (((*temp & 0xf0f0f0f0) >> 4)  | ((*temp & 0x0f0f0f0f) << 4));
		}
    }
    else
	{
		// Now place the 4 bytes in the right order
		for (a = 0; a < (nNoOfBits * 24); a++)
		{
			dwtemp = 0;
			temp = (LPDWORD)(lpOutBuff + (a * 4));
			dwtemp |=  *temp >> 24;
			dwtemp |= (*temp >> 8)  & 0x0000FF00;
			dwtemp |= (*temp << 8)  & 0x00FF0000;
			dwtemp |= (*temp << 24) & 0xFF000000;
			*temp = dwtemp;
		}
	}

}

/******************************************************************************
 *  format_printhead_data()
 *      Determines which printhead format is required and calls the appropriate
 *      function.
 *  Returns:
 *     None
 ******************************************************************************/

void format_printhead_data
(
    PDEVDATA pdev,      // Pointer to our PDEVICE
	PSETTINGS psettings,
    LPBYTE   lpData,    // Input Data Buffer
    LPBYTE   lpOutBuff, // Output Data Buffer
    int      nNoOfBits, // Colour or Monochrome data
    int      nSize      // Size of dOutput Data Buffer
)
{
  
  if (RIO_OEM(psettings)
  ||  AVALON_MODEL(psettings))
	{
        format_printhead_data_KEE(pdev, psettings, lpData, lpOutBuff, nNoOfBits, nSize);
    }
    else
  {
      
        //													  6         576
      format_printhead_data_KGE(pdev, psettings, lpData, lpOutBuff, nNoOfBits, nSize);
  }
}

/******************************************************************************
 * 
 *  SharpenImage()
 *
 *  Apply the following convolution filter to the input image:
 * 
 *        /  0.25 -1.00  0.25 \     / 0.50 \
 *        | -1.00  4.00 -1.00 |  =  | 2.00 | * (0.50  2.00  0.50)
 *        \  0.25 -1.00  0.25 /     \ 0.50 /
 *
 *  Since the 2D filter is xy-separable, we apply two 1D-filters in the
 *  vertical and horizontal directions.  This is computationally more
 *  efficient.
 *
 *  Returns:
 *    None.
 *
 *  Note: I have been a little lazy.  The corner pixels at (0,0), (0,height),
 *  (width,0) and (width,height) don't get altered by this algorithm.  This has
 *  the benefit of making the code much easier to follow.  If we put this into
 *  production code then this issue may have to be addressed.
 *
 *****************************************************************************/
typedef enum {SHARPNESS_NONE = -2,
				SHARPNESS_LESS,
				SHARPNESS_NORMAL,
				SHARPNESS_MORE,
				SHARPNESS_MAX
				} SHARPNESS;
				
void SharpenImage
(
    PDEVDATA  pdev,         ///< Pointer to our PDEV
    SHARPNESS ulSharpLevel,   ///< The level of sharpening desired.
	BOOL      b6bit				///  clamp to 64 level output or not
)
{
    LONG     lWidth       = pdev->ulCMYInPlaneWidth;
    LONG     lHeight      = pdev->yImage;
	LPBYTE   lpCurrent    = NULL;
	LPBYTE   lpNext       = NULL;
	ULONG    ulColour     = 0;
	BYTE     PrevInPixVal = 0;
	LONG     lX           = 0;
	LONG     lY           = 0;
	LONG     lOutPixVal   = 0; // larger than BYTE so we can detect overflow
    LONG     lWeightCurrent   = 1;
    LONG     lWeightNeighbour = 0;
    LONG     lWeightDivisor   = 1;

    //If we have no YMC data (i.e K Plane only) then we do no sharpening
    if (lWidth == 0)
    {
        ////ERROR_MSG(pdev->lpdm, TEXT("### Sharpening - No data to sharpen ###\n"));
        return;
    }

    // set up sharpening parameters
    switch(ulSharpLevel)
    {
        case SHARPNESS_LESS:
            //ERROR_MSG(pdev->lpdm, TEXT("### Sharpening - Less ###\n"));

            // subtle sharpnening (image may be slightly soft-looking, but not as
            // much as an unprocessed image)
            lWeightCurrent   = 6;
            lWeightNeighbour = 1;
            lWeightDivisor   = 4;
            break;

        case SHARPNESS_NORMAL:
            //ERROR_MSG(pdev->lpdm, TEXT("### Sharpening - Normal ###\n"));

            // 'normal' sharpening (best level for images which contain lots of
            // fine detail)
            lWeightCurrent   = 4;
            lWeightNeighbour = 1;
            lWeightDivisor   = 2;
            break;

        case SHARPNESS_MORE:
            //ERROR_MSG(pdev->lpdm, TEXT("### Sharpening - More ###\n"));

            // strong sharpening (if the source image is slightly blurred and needs
            // 'fixing', then this may help
            lWeightCurrent   = 10;
            lWeightNeighbour = 3;
            lWeightDivisor   = 4;
            break;

        case SHARPNESS_MAX:
            //ERROR_MSG(pdev->lpdm, TEXT("### Sharpening - Max ###\n"));

            // maximum sharpening (this might 'fix' blurry photos, but will look
            // dreadful on most images)
            lWeightCurrent   = 3;
            lWeightNeighbour = 1;
            lWeightDivisor   = 1;
            break;

        default:
            //ERROR_MSG(pdev->lpdm, TEXT("### Sharpening - None ###\n"));

            // no sharpening
            return;
    }

	for (ulColour = 0; ulColour < 3; ++ulColour)
	{
        // convolve in vertical direction (y) for each x-value
		for (lX = 0; lX < lWidth; ++lX)
		{
            lpCurrent = pdev->lpPageBuffer[ulColour] + lX + lWidth;
            lpNext    = lpCurrent + lWidth;

			PrevInPixVal = *(lpCurrent - lWidth);
			
			for (lY = 1; lY < lHeight - 1; ++lY)
			{
                lOutPixVal =  ((lWeightCurrent   * (LONG)*lpCurrent)
                             - (lWeightNeighbour * ((LONG)*lpNext + (LONG)PrevInPixVal)))
                             / lWeightDivisor;
				PrevInPixVal = *lpCurrent;
				
                if (b6bit == FALSE)
                {
				// clamp to [0,255] if necessary
				*lpCurrent = (lOutPixVal > 255) ? (BYTE)255 :
							 (lOutPixVal < 0)   ? (BYTE)0 :
												  (BYTE)lOutPixVal;
                }
                else
                {
                *lpCurrent = (lOutPixVal > 63) ? (BYTE)63 :
							 (lOutPixVal < 0)  ? (BYTE)0 :
												 (BYTE)lOutPixVal;
				}
				// advance to next set of pixels
				lpCurrent += lWidth;
				lpNext    += lWidth;
			}
		}
		
        // convolve in the horizontal direction (x) for each y-value
		for (lY = 0; lY < lHeight; ++lY)
		{
            lpCurrent = pdev->lpPageBuffer[ulColour] + (lWidth * lY) + 1;
            lpNext    = lpCurrent + 1;
			
            PrevInPixVal = *(lpCurrent - 1);
			
			for (lX = 1; lX < lWidth - 1; ++lX)
			{
                lOutPixVal =  ((lWeightCurrent * (LONG)*lpCurrent)
                             - (lWeightNeighbour * ((LONG)*lpNext + (LONG)PrevInPixVal)))
                             / lWeightDivisor;
				PrevInPixVal = *lpCurrent;
                
                if (b6bit == FALSE)
                {
				// clamp to [0,255] if necessary
				*lpCurrent = (lOutPixVal > 255) ? (BYTE)255 :
							 (lOutPixVal < 0)   ? (BYTE)0 :
												  (BYTE)lOutPixVal;
                }
                else
                {
                *lpCurrent = (lOutPixVal > 63) ? (BYTE)63 :
							 (lOutPixVal < 0)  ? (BYTE)0 :
												 (BYTE)lOutPixVal;
				}
				// advance to next set of pixels
				++lpCurrent;
				++lpNext;
			}
		}
	}
}

/*******************************************************************************
 *  CopyColourPlaneToBuffer()
 *      Receive plane and write it on the buffer.
 *
 *  Returns:
 *      None
*******************************************************************************/

VOID CopyColourPlaneToBuffer      
(
    PDEVDATA pdev,     // Pointer to our PDEVICE
	PSETTINGS psettings,
    LPBYTE   lpSrc,    // Pointer to the image data. This can be packed or planar.
    ULONG    ulColour, // Colour index
    BOOL     bPlanar   // The pixel order in lpSrc. TRUE=Planar, FALSE=Packed
)
{
    LPBYTE    lpPlane      = NULL;
    BOOL      bDataInPlane = FALSE;
    ULONG     x            = 0;
    ULONG     ulXMove      = 0;
	ULONG     ulDataWidth  = 0;

    lpPlane = pdev->lpCMYIn[ulColour];


    if (RIO_OEM(psettings)
    ||  AVALON_MODEL(psettings))
	{
		ulDataWidth = OUTPUT_DATA_WIDTH_RIO;
	}
	else
	if (PRO360_OEM(psettings))
        ulDataWidth = OUTPUT_DATA_WIDTH_PRO360;
	else
    if (HELIX_OEM(psettings))
        ulDataWidth = OUTPUT_DATA_WIDTH_HELIX;
    else
		ulDataWidth = OUTPUT_DATA_WIDTH_ALTO;

	if (bPlanar)
    {
        ulXMove = 1;
    }
    else
    {
        ulXMove = 4;
    }

    for (x = 0; x < pdev->ulCMYInPlaneWidth; x++)
    {
        *lpPlane++ = *lpSrc;
        if (*lpSrc != 0x00)
        {
            bDataInPlane = TRUE;
        }

        lpSrc += ulXMove;
    }

    if (bDataInPlane)
    {
        // Convert source strip into device format
		 if (PRO360_OEM(psettings))
        {
            CopyToBuffer(pdev, pdev->lpCMYIn[ulColour], ulDataWidth, ulColour, TRUE);
        }
		else
        if (HELIX_OEM(psettings))
        {
            CopyToBuffer(pdev, pdev->lpCMYIn[ulColour], ulDataWidth, ulColour, TRUE);
        }
        else    
        {
            format_printhead_data(pdev,
								  psettings,
                                  pdev->lpCMYIn[ulColour],
                                  pdev->lpCMYOut[ulColour],
                                  BITS_FOR_COLOUR_DATA,
                                  pdev->ulCMYOutPlaneWidth);
//myWrite(pdev, "DEBUG", 5);
//myWrite(pdev, pdev->lpCMYIn[ulColour], pdev->ulCMYInPlaneWidth);
//myWrite(pdev, pdev->lpCMYOut[ulColour], pdev->ulCMYOutPlaneWidth);
            CopyToBuffer(pdev, pdev->lpCMYOut[ulColour], ulDataWidth, ulColour, TRUE);
        }
    }
    else
    {
        /*******************************************************************************
         * If scanline does not have any colour, copy white pixels.
         * Because device does not have any command to move position, we still need to keep white datas.
         *******************************************************************************/
        CopyToBuffer(pdev, pdev->lpCMYIn[ulColour], ulDataWidth, ulColour, FALSE);
    }

    return;
}

// Pointer to our PDEVICE
VOID CopyYMCPageBuffToOutputBuff(PDEVDATA pdev, PSETTINGS psettings) 
{
    ULONG     lWidth      = pdev->ulCMYInPlaneWidth; // Width of the CMY plane (may include padding)
    ULONG     y           = 0;
    ULONG    ulColour    = 0;
    LPBYTE   lpCurrent   = NULL;

	// Copy colour plane to the output buffer.
    //                   1016
    for (y = 0; y < pdev->yImage; y++)
	{
		for (ulColour = 0; ulColour < 3; ulColour++)
        {                                               //648
            lpCurrent = pdev->lpPageBuffer[ulColour] + (lWidth * y );//pdev->epOffsetX;
            CopyColourPlaneToBuffer( pdev, psettings, lpCurrent, ulColour, TRUE );
        }
	}


}

/*******************************************************************************
 *  DetectAdjacentColour()
 *      Copy SURFOBJ which contains K-resin object information recorded during journaling.
 *      Also rotate it depends on the orientation setting.
 *
 *  Returns:
 *      None
*******************************************************************************/
BOOL DetectAdjacentColour
(
    PDEVDATA   pdev, ///< Pointer to our PDEV
	PSETTINGS  pSettings//,
//	PBYTE	   pSurface
)
{
    LPBYTE   lpKPlane    = NULL;
    LPBYTE   lpPrev      = NULL;
    LPBYTE   lpCurrent   = NULL;
    LPBYTE   lpNext      = NULL;
    LONG     lWidth      = pdev->ulCMYInPlaneWidth; // Width of the CMY plane (may include padding)
    LONG     lHeight     = pdev->yImage;
    LONG     x           = 0;
    LONG     y           = 0;
    LONG     lXMove      = 0;
    ULONG    ulColour    = 0;
    BOOL     bFirstLeft  = FALSE;
    BOOL     bFirstRight = FALSE;
	
    if (pSettings->OEM != OEM_HELIX && pSettings->OEM != OEM_PRO360)
        DetectEdge(pdev);

    /*******************************************************************************
     * Start from the bottom of the bitmap and work upwards since this is more
     * suitable to counteract the physical registration problems of the device
     *
     * lpCurrent is used to point to the pixel that we wish to process
     * lpPrev points to the pixel directly above lpCurrent
     * lpNext points to the pixel directly below lpCurrent
     *******************************************************************************/

    // --- The Bottom line ---  NB 'Bottom line' is the last line of the image
    for (ulColour = 0; ulColour < 3; ulColour++)
    {
        lpKPlane  = pdev->lpPageBuffer[PLANE_HALO] + lWidth * (lHeight - 1);
        lpCurrent = pdev->lpPageBuffer[ulColour] + lWidth * (lHeight - 1);
        lpPrev    = lpCurrent - lWidth;

        for (x = 0; x < lWidth ; x++)
        {
            if (*lpKPlane != 0x0)
            {
                if (x == 0)
                {
                    // The left most pixel in the strip.
                    ReplaceYMCPixelColour(lpPrev, lpCurrent, NULL, TRUE, FALSE);
                }
                else if (x + 1 == lWidth)
                {
                    // The right most pixel in the strip.
                    ReplaceYMCPixelColour(lpPrev, lpCurrent, NULL, FALSE, TRUE);
                }
                else
                {
                    // Somewhere in the middle of the strip
                    ReplaceYMCPixelColour(lpPrev, lpCurrent, NULL, FALSE, FALSE);
                }
            }

            // Move along the scanline
            lpKPlane ++;
            lpPrev ++;
            lpCurrent ++;
        }
    }

    // ---- Middle lines ----
    for (ulColour = 0; ulColour < 3; ulColour++)
    {
        lXMove = 1;

        // From (height - 1) to the second line
        for (y = lHeight - 1; y != 1; y--)
        {
            lpKPlane  = pdev->lpPageBuffer[PLANE_HALO] + (lWidth * y);
            lpCurrent = pdev->lpPageBuffer[ulColour] + (lWidth * y);
            lpPrev    = lpCurrent - lWidth;
            lpNext    = lpCurrent + lWidth;

            if (lXMove == -1)
            {
                // Do next search from left to right
                lXMove = 1;
                bFirstLeft  = TRUE;
                bFirstRight = FALSE;
            }
            else
            {
                // Do next search from right to left
                lXMove = -1;
                bFirstLeft  = FALSE;
                bFirstRight = TRUE;

                // Move pointers to the right edge.
                lpKPlane   += lWidth - 1;
                lpCurrent  += lWidth - 1;
                lpPrev     += lWidth - 1;
                lpNext     += lWidth - 1;
            }

            for (x = 0; x < lWidth ; x++)
            {
                if (*lpKPlane != 0x0)
                {
                    if (x == 0)
                    {
                        // The left pixel in the strip.
                        ReplaceYMCPixelColour(lpPrev, lpCurrent, lpNext, bFirstLeft, bFirstRight);
                    }
                    else if (x + 1 == lWidth)
                    {
                        // The right pixel in the strip.
                        ReplaceYMCPixelColour(lpPrev , lpCurrent, lpNext, !bFirstLeft, !bFirstRight);
                    }
                    else
                    {
                        // Somewhere in the middle of the strip
                        ReplaceYMCPixelColour(lpPrev, lpCurrent, lpNext, FALSE, FALSE);
                    }
                }

                // Move along the scanline in the desired direction
                lpKPlane  += lXMove;
                lpCurrent += lXMove;
                lpPrev    += lXMove;
                lpNext    += lXMove;
            }
        }
    }


    // --- The top line ---  NB 'Top Line' is the first line of the image
    for (ulColour = 0; ulColour < 3; ulColour++)
    {
        lpKPlane  = pdev->lpPageBuffer[PLANE_HALO];
        lpCurrent = pdev->lpPageBuffer[ulColour];
        lpNext    = lpCurrent + lWidth;

        for (x = 0; x < lWidth ; x++)
        {
            // lpNext points the pixel under the K-resin object.
            if (*lpKPlane != 0x0)
            {
                if (x == 0)
                {
                    // The left pixel in the strip.
                    ReplaceYMCPixelColour(NULL, lpCurrent, lpNext, TRUE, FALSE);
                }
                else if (x + 1 == lWidth)
                {
                    // The right pixel in the strip.
                    ReplaceYMCPixelColour(NULL, lpCurrent, lpNext, FALSE, TRUE);
                }
                else
                {
                    // Somewhere in the middle of the strip
                    ReplaceYMCPixelColour(NULL, lpCurrent, lpNext, FALSE, FALSE);
                }
            }

            // Move along the scanline
            lpKPlane ++;
            lpCurrent ++;
            lpNext ++;
        }
    }

    SharpenImage(pdev, pSettings->nSharpness, (pSettings->OEM == OEM_HELIX || pSettings->OEM == OEM_PRO360) ? FALSE : TRUE);
    
	//if white border selected offset n pixels
/*	if (pdev->dm.dmPublic.dmOrientation == DMORIENT_LANDSCAPE)
		if (pdev->dm.dmPublic.dmPaperSize == DMPAPER_WHITEBORDER)
			prnOffset = WHITEBORDER_XOFFSET;
  */          
	// Copy colour plane to the output buffer.
    for (y = 0; y < pdev->yImage; y++)
    {
//for (z=0,pBits = pdev->lpPageBuffer[PLANE_K] + (lWidth * y); z<lWidth; z++)
//	putchar(pBits[z]);
	
        CopyKPlaneToBuffer(pdev, 
                           pdev->lpPageBuffer[PLANE_K] + (lWidth * y),
                           pdev->lpKSrcStrip, 
						   pSettings,
						   y);
        for (ulColour = 0; ulColour < 3; ulColour++)
        {
            lpCurrent = pdev->lpPageBuffer[ulColour] + (lWidth * y);
            CopyColourPlaneToBuffer(pdev, pSettings, lpCurrent, ulColour, TRUE);
        }
    }

    return TRUE;
}

void endPage(PDEVDATA pdev, PSETTINGS psettings, cups_page_header2_t header)
{

fprintf(stderr, "CATULTRA endPage:pdev->eChannelOption %u\n",pdev->eChannelOption);

	// Check whether the current page requires the "Detect adjacent colour" option to be processed
	if ((pdev->eChannelOption == UICBVAL_YMCK)
	&&  pdev->bDetectAdjacentColour)
	{
		// Modify colours underneath the edges of black areas to mask any colour mis-registration
		// Sharpening will then occur afterwards
		pdev->iHaloRadius = 25;///lpds->iHaloRadius;
		////wsprintf(wsTemp, TEXT("**** HALO_RADIUS = %d ****\n"), pdev->iHaloRadius);   
	     
		DetectAdjacentColour(pdev, psettings);
	}
	else
	if (pdev->eChannelOption == UICBVAL_YMC)
	{
		SharpenImage(pdev, psettings->nSharpness, (psettings->OEM == OEM_HELIX || psettings->OEM == OEM_PRO360) ? FALSE : TRUE);
		CopyYMCPageBuffToOutputBuff(pdev, psettings);
	}
	///////////////

fprintf(stderr, "CATULTRA pdev->bDuplex %u: pdev->bFrontPage :%u\n",pdev->bDuplex,pdev->bFrontPage);
	
	if (pdev->bDuplex && pdev->bFrontPage)
	{
		// Skip output page data to wait for next page
fprintf(stderr, "CATULTRA Skip output page data to wait for next page%u\n",1);

	}
	else if (pdev->bDuplex && !pdev->bFrontPage)
	{
	
		// Output Front Page
fprintf(stderr, "CATULTRA Output Front Page:%u\n",1);

		if (psettings->OEM == OEM_PRO360)			
			OutputPro360HeaderCommand( pdev, header, psettings, TRUE );
		else
		if (psettings->OEM == OEM_HELIX)
			OutputHelixHeaderCommand( pdev, header, psettings, TRUE );
		else
			OutputHeaderCommand( pdev, header, psettings, TRUE );

		if (!IgnoreImageData(pdev, psettings))
		{
			if (psettings->OEM != OEM_HELIX && psettings->OEM != OEM_PRO360)
				SetDataSize( pdev, TRUE );
			FlushBuffer( pdev, psettings, TRUE );
		}

		FreeBuffer( pdev, TRUE );

		// Output Back Page
fprintf(stderr, "L8052:CATULTRA Output Back Page:%u\n",1);
		if (psettings->OEM != OEM_HELIX && psettings->OEM != OEM_PRO360)
			OutputHeaderCommand( pdev, header, psettings, FALSE );

		if (!IgnoreImageData(pdev, psettings))
		{
			if (psettings->OEM != OEM_HELIX && psettings->OEM != OEM_PRO360)
				SetDataSize( pdev, FALSE );
fprintf(stderr, "L8060:CATULTRA Output Back Page:%u\n",1);
			FlushBuffer( pdev, psettings, FALSE );
fprintf(stderr, "L8062:CATULTRA returned from flushbuffer:%u\n",1);
		}
		if (psettings->OEM == OEM_PRO360)
		{
			pdev->epCheckSum =  pdev->epCheckSum ^ 0xffffffffu;
			// if (pdev->hFile)
			// {
				// DWORD cbwritten;
				// WriteFile(pdev->hFile, (PBYTE)&pdev->epCheckSum, 4, &cbwritten, NULL);
				// pdev->epCheckSum=0xffffffffu; 
			// }
			// else
				myWrite(pdev, (char *)&pdev->epCheckSum, 4);
			//myWriteSpool(pdev);
		}
		FreeBuffer( pdev, FALSE );	
	}
	else
	{
		// Simplex case
fprintf(stderr, "CATULTRA Simplex case:%u\n",1);
		// Send out header command
		if (psettings->OEM == OEM_PRO360)
		{
			fprintf(stderr, "CATULTRA: Pro360 - OutputPro360HeaderCommand\n");
			OutputPro360HeaderCommand( pdev, header, psettings, pdev->bFrontPage );
		}
		else			
		if (psettings->OEM == OEM_HELIX)
		{
			fprintf(stderr, "CATULTRA: Pro360 - OutputHelixHeaderCommand\n ");
			OutputHelixHeaderCommand( pdev, header, psettings, pdev->bFrontPage );
		
		}
		else
		{
			fprintf(stderr, "CATULTRA: OutputHeaderCommand\n ");
			OutputHeaderCommand( pdev, header, psettings, pdev->bFrontPage );
		}

		// When magnetic encoding only option is set, we will not send any page data.
		if (!IgnoreImageData(pdev, psettings))
		{
			// Send out plane data size command
			if (psettings->OEM != OEM_HELIX && psettings->OEM != OEM_PRO360)
				SetDataSize( pdev, pdev->bFrontPage );

			// Send out plane data which we saved during dvOutputStrip
			FlushBuffer( pdev, psettings, pdev->bFrontPage );
		}
		if (psettings->OEM == OEM_PRO360)
		{
			pdev->epCheckSum =  pdev->epCheckSum ^ 0xffffffffu;
			// if (pdev->hFile)
			// {
				// DWORD cbwritten;
				// WriteFile(pdev->hFile, (PBYTE)&pdev->epCheckSum, 4, &cbwritten, NULL);
				// pdev->epCheckSum=0xffffffffu; 
			// }
			// else
				myWrite(pdev, (char *)&pdev->epCheckSum, 4);
			//myWriteSpool(pdev);
		}
		// Clean up memory for buffering
		FreeBuffer( pdev, pdev->bFrontPage );
	}

    // Clean buffers
    DeleteOutputBuffer( pdev );

    // Cleanup halftoning information
    if ((pdev->eChannelOption == UICBVAL_YMCK)
    ||  (pdev->eChannelOption == UICBVAL_KResin))
    {
        DeleteKPrintingObject( pdev );
    }

	fflush (stdout);

}

void endJob(PDEVDATA pdev, struct settings_ settings, cups_page_header2_t header)
{
    //odd page duplex jobs we need to send the last page simplex
	if (pdev->bDuplex && settings.iTotalJobPages == 0 && (pdev->iPageNumber % 2))
	{
		pdev->bDuplex = FALSE;
		// Output back page
		if (settings.OEM != OEM_HELIX && settings.OEM != OEM_PRO360)
			OutputHeaderCommand( pdev, header, &settings, TRUE/*pdev->bFrontPage*/ );

		if (!IgnoreImageData(pdev, &settings))
		{
			// Send out plane data size command
			if (settings.OEM != OEM_HELIX && settings.OEM != OEM_PRO360)
				SetDataSize( pdev, TRUE );

			FlushBuffer( pdev, &settings, pdev->bFrontPage );
		}
		if (settings.OEM == OEM_PRO360)
		{
			pdev->epCheckSum =  pdev->epCheckSum ^ 0xffffffffu;
			// if (pdev->hFile)
			// {
				// DWORD cbwritten;
				// WriteFile(pdev->hFile, (PBYTE)&pdev->epCheckSum, 4, &cbwritten, NULL);
				// pdev->epCheckSum=0xffffffffu; 
			// }
			// else
				myWrite(pdev, (char *)&pdev->epCheckSum, 4);
			//myWriteSpool(pdev);
		}	
		// Clean up memory for buffering
		FreeBuffer( pdev, pdev->bFrontPage );
	}	
}

#define GET_LIB_FN_OR_EXIT_FAILURE(fn_ptr,lib,fn_name)                                      \
{                                                                                           \
    fn_ptr = dlsym(lib, fn_name);                                                           \
    if ((dlerror()) != NULL)                                                                \
    {                                                                                       \
        fputs("ERROR: required fn not exported from dynamically loaded libary\n", stderr);  \
        if (libCupsImage != 0) dlclose(libCupsImage);                                       \
        if (libCups      != 0) dlclose(libCups);                                            \
        return EXIT_FAILURE;                                                                \
    }                                                                                       \
}

#ifdef RPMBUILD
#define CLEANUP                                                         \
{                                                                       \
    if (originalRasterDataPtr   != NULL) free(originalRasterDataPtr);   \
    CUPSRASTERCLOSE(ras);                                               \
    if (fd != 0)                                                        \
    {                                                                   \
        close(fd);                                                      \
    }                                                                   \
    dlclose(libCupsImage);                                              \
    dlclose(libCups);                                                   \
}
#else
#define CLEANUP                                                         \
{                                                                       \
    if (originalRasterDataPtr   != NULL) free(originalRasterDataPtr);   \
    CUPSRASTERCLOSE(ras);                                               \
    if (fd != 0)                                                        \
    {                                                                   \
        close(fd);                                                      \
    }                                                                   \
}
#endif

ULONG RGB_to_CMY(ULONG *pColor)
{
    BYTE r,g,b,c,m,y;
	
	r = GetRValue(*pColor);
	g = GetGValue(*pColor);
	b = GetBValue(*pColor);
	    
    c = 255 - r;       //
    m = 255 - g;       //
    y = 255 - b;       //
		         
    return (CMYK( c, m, y, 0 ));
}

ULONG RGB_to_CMYK(ULONG *pColor)
{
    BYTE r,g,b,c,m,y;//,k;

    if (*pColor == 0)
        return (CMYK( 0, 0, 0, 255 ));

	r = GetRValue(*pColor);
	g = GetGValue(*pColor);
	b = GetBValue(*pColor);
    
    c = 255 - r;       //
    m = 255 - g;       //
    y = 255 - b;       //
                         
    return (CMYK( c, m, y, 0 ));
}

void error_diffusion(PDEVDATA pdev,
					 PBYTE    pSurface)     // Pointer to the surface
{ 
#define RAND(RN) (((seed = 1103515245 * seed + 12345) >> 12) % (RN))
#define INITERR(X, Y) (((int) X) - (((int) Y) ? WHITE : BLACK) + ((WHITE/2)-((int)X)) / 2)
#define WHITE  255
#define BLACK  0

PBYTE pOutputImage = malloc(pdev->yImage * pdev->lDelta);
PBYTE pInputImage = pSurface;
//PBYTE pOutByte;
//unsigned int i;
int seed = 0;
int x, y, p, pixel, threshold, error;
int width, height;//, pitch;
int *lerr;
int *cerr;
int *terr;
BYTE *bits, *new_bits;

    // Floyd & Steinberg error diffusion dithering
    // This algorithm use the following filter
    //          *   7
    //      3   5   1     (1/16)

	width = pdev->lDelta;
	height = pdev->yImage;

	// allocate space for error arrays
	lerr = (int*)malloc (width * sizeof(int));
	cerr = (int*)malloc (width * sizeof(int));
	memset(lerr, 0, width * sizeof(int));
	memset(cerr, 0, width * sizeof(int));

	// left border
	error = 0;
	for(y = 0; y < height; y++) {
		bits = pInputImage + (y * pdev->lDelta);
		new_bits = pOutputImage + (y * pdev->lDelta);

		threshold = (WHITE / 2 + RAND(129) - 64);
		pixel = bits[0] + error;
		p = (pixel > threshold) ? WHITE : BLACK;
		error = pixel - p;
		new_bits[0] = (BYTE)p;
	}
	// right border
	error = 0;
	for(y = 0; y < height; y++) {
		bits = pInputImage + (y * pdev->lDelta); 
		new_bits = pOutputImage + (y * pdev->lDelta);

		threshold = (WHITE / 2 + RAND(129) - 64);
		pixel = bits[width-1] + error;
		p = (pixel > threshold) ? WHITE : BLACK;
		error = pixel - p;
		new_bits[width-1] = (BYTE)p;
	}
	// top border
	bits = pInputImage;
	new_bits = pOutputImage;
	error = 0;
	for(x = 0; x < width; x++) {
		threshold = (WHITE / 2 + RAND(129) - 64);
		pixel = bits[x] + error;
		p = (pixel > threshold) ? WHITE : BLACK;
		error = pixel - p;
		new_bits[x] = (BYTE)p;
		lerr[x] = INITERR(bits[x], p);
	}

	// interior bits
	for(y = 1; y < height; y++) {
		// scan left to right
		bits = pInputImage + (y * pdev->lDelta);
		new_bits = pOutputImage + (y * pdev->lDelta);

	    cerr[0] = INITERR(bits[0], new_bits[0]);
		for(x = 1; x < width - 1; x++) {
			error = (lerr[x-1] + 5 * lerr[x] + 3 * lerr[x+1] + 7 * cerr[x-1]) / 16;
			pixel = bits[x] + error;
			if(pixel > (WHITE / 2)) {		
				new_bits[x] = WHITE;
				cerr[x] = pixel - WHITE; 
			} else {
				new_bits[x] = BLACK;
				cerr[x] = pixel - BLACK; 
			}
		}
		// set errors for ends of the row
		cerr[0] = INITERR (bits[0], new_bits[0]);
		cerr[width - 1] = INITERR (bits[width - 1], new_bits[width - 1]);

		// swap error buffers
		terr = lerr; lerr = cerr; cerr = terr;
	}

	free(lerr);
	free(cerr);
    //update the target with the resulting 8bpp image
    memcpy (pSurface, pOutputImage, pdev->lDelta * pdev->yImage);


    free(pOutputImage);

}            

/******************************************************************************
 *  YMC_OutputStrip()
 *      Create 3 plane (YMC) 6 bit format data and save to memory.
 *
 *  Returns:
 *      None
 ****************************************************************************/

VOID YMC_OutputStrip
(
    PDEVDATA pdev,        // Pointer to our PDEVICE
    PBYTE    pSurface     // Pointer to the surface
)
{
    ULONG    x         = 0;
    ULONG    y         = 0;
    PBYTE   lpSrc     = 0;
	PBYTE   lpPlane   = 0;
    ULONG    ulColour  = 0;
fprintf(stderr, "L8385:CATULTRA pdev->lDelta:%u\n",pdev->lDelta);  	
fprintf(stderr, "L8386:CATULTRA pdev->xImage:%u\n",pdev->xImage);  	
fprintf(stderr, "L8387:CATULTRA pdev->yImage:%u\n",pdev->yImage);  	
fprintf(stderr, "L8387:CATULTRA pdev->ulCMYInPlaneWidth:%u\n",pdev->ulCMYInPlaneWidth);  	

    for (y = 0; y < pdev->yImage; y++)  //portrait 1016 shown for 32bit surface
    {
        for (ulColour = 0; ulColour < 3; ulColour++)
        {
            lpSrc   = pSurface + y * pdev->lDelta + (2 - ulColour);
            lpPlane = pdev->lpPageBuffer[ulColour] + y * pdev->ulCMYInPlaneWidth;

            for (x = 0; x < pdev->ulCMYInPlaneWidth; x++)
			{
				*lpPlane++ = lpSrc[0];
				lpSrc += 4;
			}
        }
    }

    return;
}
/*******************************************************************************
 *  HalftoneKStrip()
 *      Halftone 8bit gray to 1bit mono.
 *
 *  Returns:
 *      None
*******************************************************************************/

VOID HalftoneKStrip
(
    PDEVDATA    pdev,      // Pointer to our PDEV
    PBYTE       pSurface,  // Pointer to our SURFOBJ
    LONG        y          // scanline offset
)
{
    PBYTE      lpHalftoned = (PBYTE)pdev->lpHalftone;
    LPBYTE     lpPlane      = NULL;
    LPBYTE     lpDst        = NULL;
    WORD       cnt          = 0;
    LONG      x            = 0;
//	LONG		z=0;
//	PBYTE		pBits;

    //ok so 8bit monochrome filled so convert to halftoned 1bpp
////fprintf(stderr, "CATULTRA HalftoneKStrip:pdev->lDelta:%u,pdev->xImage:%u\n",pdev->lDelta,pdev->xImage);
    lpPlane = (PBYTE)pSurface + pdev->xImage/*pdev->lDelta*/ * y;

//for (z=0,pBits = pSurface + (pdev->xImage * y); z<pdev->xImage; z++)
//	putchar(pBits[z]);

    lpDst = (PBYTE)lpHalftoned;

    for (x = 0; x < pdev->xImage/*pdev->lDelta*//*sizlBitmap.cx*/; x++)
    {
        BYTE a;
        BYTE b;
        b=~(lpPlane[0]);
        a=128 >> cnt;
            
        if (b)
            *lpDst |= a;
        else
            *lpDst &= ~a;
        
        if (cnt == 7)
        {
            lpDst++;
            cnt = 0;
        }
        else
            cnt ++;
        lpPlane++;
        
    }
    
}

/*******************************************************************************
 *  CopyKPlaneToBuffer()
 *      Receive plane and write it on the buffer.
 *
 *  Returns:
 *      None
*******************************************************************************/

VOID CopyKPlaneToBuffer
(
    PDEVDATA   pdev,        // Pointer to our PDEVICE
    LPBYTE     lpSrc,       // Pointer to the image data. If this is null, use pStripObj.
    PBYTE      pSurface,    // Pointer to the surface object
    PSETTINGS  pSettings,   // Pointer to current settings
    LONG       lYOffset		// Current Y position
)
{
    LPBYTE     lpPlane      = NULL;
    BOOL       bDataInPlane = FALSE;
    PBYTE      lpHalftoned  = NULL;
    LONG      x            = 0;

    if (lpSrc != NULL)
    {
        /*******************************************************************************
         * Usually we have bitmap data in the pStripObj.
         * But when Detect Adjacent Colour option is set, we create it on another buffer.
         *******************************************************************************/
        PBYTE pBits = (PBYTE)pSurface;
		
		if (PRO360_OEM(pSettings))
            memcpy(pBits, lpSrc, pdev->ulCMYInPlaneWidth);
        else
        if (HELIX_OEM(pSettings))
            memcpy(pBits, lpSrc, pdev->ulCMYInPlaneWidth);
        else
        {
            for (x=0; x<(LONG)pdev->ulCMYInPlaneWidth; x++)
                pBits[x]=~lpSrc[x];				
        }
    }

    //8bpp monochrome 

    lpPlane = pSurface;
    if (lpSrc == NULL)
        lpPlane += lYOffset * pdev->lDelta;//Bitmap.pvData;
	
	if (PRO360_OEM(pSettings))
	{
		for (x = 0; x < pdev->ulCMYInPlaneWidth; x++)
		{
			if (*lpPlane != 0x00)
			{
				bDataInPlane = TRUE;
				break;
			}
			lpPlane++;
		}
	}
	else	
	if (HELIX_OEM(pSettings))
	{
		for (x = 0; x < pdev->ulCMYInPlaneWidth; x++)
		{
			if (*lpPlane != 0x00)
			{
				bDataInPlane = TRUE;
				break;
			}
			lpPlane++;
		}
	}
	else
    {
        for (x = 0; x < pdev->ulCMYInPlaneWidth; x++)
        {
            if (*lpPlane != 0xFF)
            {
                bDataInPlane = TRUE;
                break;
            }
            lpPlane++;
        }
    }
/*{
LONG z;
PBYTE pBits;	
for (z=0,pBits = pSurface + (pdev->ulCMYInPlaneWidth * lYOffset); z<pdev->ulCMYInPlaneWidth; z++)
	putchar(pBits[z]);
}*/
    if (bDataInPlane)
    {
        HalftoneKStrip(pdev, pSurface, (lpSrc) ? 0 : lYOffset);
        lpHalftoned = (PBYTE)pdev->lpHalftone;

        // Convert source strip into device format
        if (HELIX_OEM(pSettings) || PRO360_OEM(pSettings))
        {
			CopyToBuffer(pdev, (lpSrc == NULL) ? (PBYTE)pSurface + (lYOffset * pdev->lDelta) : pSurface, pdev->ulKOutPlaneWidth, PLANE_K, TRUE);
			//CopyToBuffer(pdev, pSurface/*lpHalftoned->pvBits*/, pdev->ulKOutPlaneWidth, PLANE_K, TRUE);
        }
        else    
        {
        format_printhead_data(pdev,
							  pSettings,
                              lpHalftoned,//->pvBits,
                              pdev->lpKOut,
                              BITS_FOR_BLACK_DATA,
                              pdev->ulKOutPlaneWidth);

        CopyToBuffer(pdev, pdev->lpKOut, pdev->ulKOutPlaneWidth, PLANE_K, TRUE);
		}
    }
    else
    {
        memset((PBYTE)pSurface, 0, pdev->lDelta);
        CopyToBuffer(pdev, pSurface/*pStripObj->Bitmap.pvData*/, pdev->ulKOutPlaneWidth, PLANE_K, FALSE);
    }

    return;
}

/******************************************************************************
 *  K_OutputStrip()
 *      Create 1 bit (mono)  format data and save to memory.
 *
 *  Returns:
 *      None
 ****************************************************************************/

VOID K_OutputStrip
(
    PDEVDATA pdev,      // Pointer to our PDEVICE
	PSETTINGS pSettings,
 //   SURFOBJ * pso24bpp,  // Pointer to 24bpp surface
    PBYTE	   pSurface        // Pointer to the strip object

)
{
    LONG   y = 0;

    //apply halftoning here..
////    if (pdev->eHalftoning)
////        line_art(pdev,pso24bpp,pso);
////    else
        error_diffusion(pdev, pSurface);
        
    for (y = 0; y < pdev->yImage; y++)
        CopyKPlaneToBuffer( pdev, NULL, pSurface, pSettings, y );

    return;
}
/******************************************************************************
 *  YMCK_OutputStrip()
 *      Create 3 plane (YMC) 6 bit + 1 bit (mono)  format data and save to memory.
 *
 *  Returns:
 *      None
 ****************************************************************************/

VOID YMCK_OutputStrip
(
    PDEVDATA pdev,   // Pointer to our PDEVICE
	PSETTINGS pSettings,	
    PBYTE pSurface    //Pointer to the strip object
)
{
    ULONG      x            = 0;
    ULONG      y            = 0;
    PBYTE      pKSrcStrip   = pdev->lpKSrcStrip;
    PBYTE      lpPlane      = NULL;
    PBYTE      lpSrc        = NULL;
    ULONG      ulColour     = 0;
       
    //apply halftoning to K-resin plane if monobitmaps have been requested to be printed on the kresin plane..
	//not supported currently
//    if (pdev->bExtracted) 
//        error_diffusion(pdev,pKSrcStrip);    
fprintf(stderr, "CATULTRA YMCK_OutputStrip:pdev->xImage:%u,pdev->epPaperXdots:%u\n",pdev->xImage,pdev->epPaperXdots);
fprintf(stderr, "CATULTRA YMCK_OutputStrip:pdev->ulCMYInPlaneWidth:%u,pdev->lDelta:%u\n",pdev->ulCMYInPlaneWidth,pdev->lDelta);
    for (y = 0; y < pdev->yImage; y++)
    {

        // Check whether the "Detect adjacent colour" option is set for this CMYK page
        if (pdev->bDetectAdjacentColour)
        {
            // NB bDetectAdjacentColour is ALWAYS set!!
            // Save off the strip into four page buffers (one for each colour plane).
            // These planes will be processed via DetectAdjacentColour() at dvEndPage().
            // Note that CopyToBuffer() is called as the final step in DetectAdjacentColour()
            // so it is bypassed at this point.

			lpSrc   = (LPBYTE)pSurface + (y * pdev->lDelta) + 3;
			lpPlane = pdev->lpPageBuffer[PLANE_K] + y * pdev->ulCMYInPlaneWidth;

			for (x = 0; x < pdev->ulCMYInPlaneWidth; x++)
			{
				*lpPlane++ = *lpSrc;
				lpSrc += 4;
			}

            // Copy colour plane from STRIPOBJ.
            for (ulColour = 0; ulColour < 3; ulColour++)
            {
                lpSrc   = (LPBYTE)pSurface + y * pdev->lDelta + (2 - ulColour);
                lpPlane = pdev->lpPageBuffer[ulColour] + y * pdev->ulCMYInPlaneWidth;

                for (x = 0; x < pdev->ulCMYInPlaneWidth; x++)
                {
                    *lpPlane++ = *lpSrc;
                    lpSrc += 4;
                }
            }
        }
         else
        {
            //this is never called 
            CopyKPlaneToBuffer( pdev, NULL, pKSrcStrip, pSettings, y);

				for (ulColour = 0; ulColour < 3; ulColour++)
				{
					lpSrc = (LPBYTE)pSurface + y * pdev->lDelta + (2 - ulColour);
					CopyColourPlaneToBuffer( pdev, pSettings, lpSrc, ulColour, FALSE );
				}
			
		}
		
    }

    return;
}

/******************************************************************************
 *  dvOutputStrip()
 *      Send a strip of raster data to the printer
 *
 *  Returns:
 *      None
 ****************************************************************************/

VOID dvOutputStrip
(
    PDEVDATA   pdev,      // Pointer to our PDEV
	PSETTINGS  pSettings, //
    PBYTE	   pSurface  // Pointer to the output surface
)
{
    if (IgnoreImageData(pdev, pSettings))
    {
        return;
    }
    
    switch (pdev->eChannelOption)
    {
        case UICBVAL_YMC:
            YMC_OutputStrip( pdev, pSurface );
            break;

        case UICBVAL_YMCK:
        default:
            YMCK_OutputStrip( pdev, pSettings, pSurface );
            break;

        case UICBVAL_KResin:
////			error_diffusion(pdev, pSurface);
            K_OutputStrip( pdev, pSettings, pSurface );
            break;
    }

    return;
}
    void ColourCorrect(CHARPIXEL * pixel, BYTE intent)
    {
        //call this function as a LUT algorithm in the same way Naive is an algorithm.
        
        //  Algorithm based on :    (Red - Red_i)/(Red_i+1 -Red_i) = (Cyan- Cyan_i)/(Cyan_i+1 - Cyan_i) where i goes from 0 to 255 in steps of 4
        
        unsigned char upper[3];
        unsigned char lower[3];
        unsigned char offset[3];
        
        unsigned char r = 0; //just a convenience
        unsigned char g = 1;
        unsigned char b = 2;
    // fprintf(stderr, "CATULTRA: IN pixel->r:%x:%x:%x \n",pixel->r,pixel->g,pixel->b);           
        lower[r] = (unsigned char)(pixel->r)/GridDelta;  upper[r] = lower[r]+1; offset[r] = (unsigned char)(pixel->r - lower[r]*GridDelta);
        lower[g] = (unsigned char)(pixel->g)/GridDelta;  upper[g] = lower[g]+1; offset[g] = (unsigned char)(pixel->g - lower[g]*GridDelta);
        lower[b] = (unsigned char)(pixel->b)/GridDelta;  upper[b] = lower[b]+1; offset[b] = (unsigned char)(pixel->b - lower[b]*GridDelta);
        
        pixel->c = Cyan[lower[r]][lower[g]][lower[b]][intent];
        pixel->c += (unsigned char)((offset[r])? (1.0 * offset[r] * ( Cyan[upper[r]][upper[g]][upper[b]][intent] - Cyan[lower[r]][lower[g]][lower[b]][intent]))/GridDelta : 0);
        
        
        pixel->m = Magenta[lower[r]][lower[g]][lower[b]][intent];
        pixel->m += (unsigned char)((offset[g])?  (1.0 * offset[g] * ( Magenta[upper[r]][upper[g]][upper[b]][intent] - Magenta[lower[r]][lower[g]][lower[b]][intent]))/GridDelta : 0);
        
        
        pixel->y = Yellow[lower[r]][lower[g]][lower[b]][intent];
        pixel->y += (unsigned char)((offset[b])?  (1.0 * offset[b] * ( Yellow[upper[r]][upper[g]][upper[b]][intent] - Yellow[lower[r]][lower[g]][lower[b]][intent]))/GridDelta : 0);
    //  fprintf(stderr, "CATULTRA: pixel->c:%x:%x:%x \n",pixel->c,pixel->m,pixel->y);           
    }
    
    void ColourCorrectHelix(CHARPIXEL * pixel, BYTE intent)
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
BOOL SendPage(PDEVDATA pdev, PSETTINGS psettings, cups_page_header2_t header, cups_raster_t * ras, PBYTE rasterData)
{
    PBYTE	pSurface=NULL;     //target 32bpp/8bpp surface
    ULONG    x,y;

    //firmware job so bail
    if (pdev->bFWDownload)
    {
        return TRUE;
    }
        

    //initialisation of layers and counters
    pdev->epbCyanLayer = FALSE;         //initialise whether there is a Cyan layer to send
    pdev->epbMagentaLayer = FALSE;      //initialise whether there is a Magenta layer to send
    pdev->epbYellowLayer = FALSE;       //initialise whether there is a Yellow layer to send
    pdev->epbBlackLayer = FALSE;        //initialise whether there is a black layer to send
    
    pdev->eplCyanImageDataLen  = 0L;    //initialise size of the Cyan layer data  
    pdev->eplMagentaImageDataLen = 0L;  //initialise size of the Magenta layer data  
    pdev->eplYellowImageDataLen   = 0L; //initialise size of the Yellow layer data  
    pdev->eplBlackImageDataLen = 0L;    //initialise size of the black layer data  


    if (pdev->pso32bitSurface)
    {
        PBYTE  lpSrc;
        PBYTE  lpDst;///*unsigned char */*/*LPBYTE*/  lpDst;
        ULONG   srcColour;    
        ULONG   cmyk;
        ULONG   NextPixel;
	
        BOOL    bColourMatch = FALSE;
        PCOLOURMATCHTABLE pct = NULL;
fprintf(stderr, "L6946:CATULTRA pdev->epOffsetX:%u\n",pdev->epOffsetX);        
//		pdev->epOffsetX = ((1926 - header.cupsWidth) / 2);
//		pdev->epOffsetY = ((1926 - header.cupsWidth) / 2);
			
        if (pdev->eChannelOption == UICBVAL_KResin)
        {
/*			if (header.Orientation == 1 || header.Orientation == 3) //landscape        
			{
				RotateMaskSurface(pdev, pdev->lpKPSrcStrip, pdev->lpKSrcStrip);        
				DeleteKStripObject(pdev->lpKSrcStrip);
				pdev->lpKSrcStrip = pdev->lpKPSrcStrip;
			}	*/	
	
            pSurface = pdev->lpKPSrcStrip;
            NextPixel = 1;
			pdev->lDelta = pdev->xImage;
fprintf(stderr, "L6096:CATULTRA UICBVAL_KResin:pdev->lDelta:%u\n",pdev->lDelta);

        }
        else
        {
            pSurface = pdev->pso32bitSurface;
            NextPixel = 4;
			//PORTRAIT!!? 642 * 4
			pdev->lDelta = pdev->xImage * 4;

        }
//fprintf(stderr, "CATULTRA pSurface %x\n",pSurface);
fprintf(stderr, "CATULTRA lDelta %u\n",pdev->lDelta);
/*
    //now merge mask surface with monochrome surface and white out from 32bpp surface
    //
    if (pdev->eChannelOption == UICBVAL_YMCK)    
    {
        PBYTE	psoDest;
		PBYTE	psoSrc;
        BOOL    bExtracted = FALSE;
        
        //determine if we should halftone the k-resin plane for YMCK output
        pdev->bExtracted = FALSE;
        //NOTE:!!so far if K selected (default) use K in src plane and disregard anything recorded.
 
        //if (pdmPrivate->OEM != OEM_HELIX)
        {
			
            // if source landscape move resin surface to match our CMYK surface orientation 648,1016...
            if (header.Orientation == CUPS_ORIENT_0 || header.Orientation == CUPS_ORIENT_180) //landscape        
            {
                RotateMaskSurface(pdev, pdev->lpKPSrcStrip, pdev->lpKSrcStrip);        
                DeleteKStripObject(pdev->lpKSrcStrip);
                pdev->lpKSrcStrip = pdev->lpKPSrcStrip;
            }
        }
        // Clear the destination resin surface before we copy any data from the source resin surface
        // which ddi hooks will have blt/set mask values into        
        psoDest = pdev->lpKDstStrip;
        if (psoDest->pvScan0)
            memset(psoDest->pvScan0, 0x00, psoDest->cjBits);

        if (pdev->eBlackStartOption == UICBVAL_Start_K)
        {
            // Modify pixels in the STRIPOBJ
            if (pdev->bPhotoUseYMC)
                PhotoUseYMCOnly(pdev, pso);
        }

        if (pdev->eBlackStartOption == UICBVAL_Start_YMC)
        {
            // Cut and copy monochrome objects from the pStripObj to the pKSrcStrip.
            if (pdev->bBlackTextUseK || pdev->bBlackPolygonUseK)
            {
                ExtractBlackTextAndPolygon(pdev, pso, ((SURFOBJ *)pdev->lpKDstStrip)->pvScan0,((SURFOBJ *)pdev->lpKSrcStrip)->pvScan0);
                bExtracted = TRUE;
            }
        }

        if (pdev->bMonoBitmapUseK)
        {
            // Cut and copy monochrome bitmap objects from the pStripObj to the pKSrcStrip.
            ExtractMonoBitmap(pdev, pso, ((SURFOBJ *)pdev->lpKSrcStrip)->pvScan0);
            bExtracted = TRUE;
            
        }
        
        if (bExtracted)
        {
            DeleteKStripObject(pdev->lpKSrcStrip);
            pdev->lpKSrcStrip = NULL;
            //now that all the objects have been extracted point to the new surface
            pdev->lpKSrcStrip = pdev->lpKDstStrip;
        }
        else
        {
            //clear out the kresin mask surface as we have extracted nothing
            psoDest = pdev->lpKSrcStrip;
            if (psoDest->pvScan0)
                memset(psoDest->pvScan0, 0x00, psoDest->cjBits);
        }
    }
*/        
     
        if ((ENDURO_PLUS_TYPE(psettings) || RIOPRO_TYPE(psettings) || XXL_TYPE(psettings)) &&
            (psettings->nColourCorrection >= UICBVAL_ColourCorrection_ICC_Internal))
        {
            bColourMatch = TRUE;
fprintf(stderr, "CATULTRA bCOLORMATCH ENABLED %u\n",psettings->nColourCorrection);
            pct = (ENDURO_PLUS_TYPE(psettings)) ? (PCOLOURMATCHTABLE)&ColorMatchTableEnduro : (PCOLOURMATCHTABLE)&ColorMatchTableRioPro;
        }
        
        //move 24bpp data into our 32bpp/8bpp surface
        //!648
        //1016
		if (psettings->OEM == OEM_PRO360)
        {
            LONG ycnt, xcnt;
            fprintf(stderr, "CAT pdev->epPaperXdots:%x\n",pdev->epPaperXdots);
            fprintf(stderr, "CAT pdev->epPaperYdots:%x\n",pdev->epPaperYdots);
            fprintf(stderr, "CAT psettings->orientation:%u\n",psettings->orientation);
            
            if (psettings->orientation == 0/*header.cupsHeight != 664*/)// && header.cupsHeight != 991 && header.cupsHeight != 1654)
            {
                xcnt = pdev->epPaperYdots;
                ycnt = (pdev->epPaperXdots - pdev->epOffsetX);
            }
            else
            {
                xcnt = (pdev->epPaperXdots);// - pdev->epOffsetX);
                ycnt = (pdev->epPaperYdots);// - pdev->epOffsetY);

            }
            
            fprintf(stderr, "CATULTRA passed SendPage header.cupsHeight %u\n",header.cupsHeight);
            fprintf(stderr, "CATULTRA passed SendPage header.cupsBytesPerLine %u\n",header.cupsBytesPerLine);
            fprintf(stderr, "CATULTRA - 360 passed SendPage header.Orientation %u\n",header.Orientation);
            for (y = 0; y < header.cupsHeight; y++)
            {
//		fprintf(stderr, "CAT processing y:%u\n",y);
		if (CUPSRASTERREADPIXELS(ras, rasterData, header.cupsBytesPerLine) < 1)
		{
			fprintf(stderr, "CAT breaking out L6253!!%u\n",y);
			break;
		}
		lpSrc = rasterData;
				
                for (x = 0; x < (header.cupsBytesPerLine / 3); x++)
                {
                    //           1013                               642     
                    //    --------------------------        ----------------  
                    //    -oooo                             -               
                    //    -                                 -                 
                    //    -                                 -                 
                    //642 -                                 -                 
                    //    -                                 -                 
                    //    -                                 -                 
                    //    -                                 -                 
                    //                                      -                 
                    //                                      -                 
                    //                                      -o                
                                                            
                    //landscape
                    if (psettings->orientation == 0)
                    {
                        if (pdev->bRotate)
							lpDst = (PBYTE)pSurface + (((xcnt - 1) - x ) * pdev->lDelta) + (((ycnt - 1) - y) * NextPixel);
                        else
                            lpDst = (PBYTE)pSurface + ( x * pdev->lDelta) + (y * NextPixel);
					}
                    else
                    {
                        if (pdev->bRotate) //180 flip
                            lpDst = (PBYTE)pSurface + (y * pdev->lDelta) + (((xcnt - 1) - x) * NextPixel);
                        else
                            lpDst = (PBYTE)pSurface + (((ycnt - 1) - y) * pdev->lDelta) + (x * NextPixel);
                        
                    }
                    // set RGB pixels into 32bpp/8bpp plane
                    srcColour = RGB(lpSrc[x*3],lpSrc[(x*3)+1],lpSrc[(x*3)+2]);
                    if (pdev->eBlackStartOption == UICBVAL_Start_YMC || pdev->eChannelOption == UICBVAL_YMC)
                        cmyk = RGB_to_CMY(&srcColour);
                    else
						cmyk = RGB_to_CMYK((ULONG *)&srcColour);
					
                    if (pdev->eChannelOption == UICBVAL_KResin)
                        *lpDst       = ~RgbToGray(lpSrc[x*3],lpSrc[(x*3)+1],lpSrc[(x*3)+2]);
                    else
                    {
						//CHARPIXEL cp;
						
                        //built in colormatching requested?
                        if (bColourMatch && cmyk != (LONG)0xff)
                        {
                        rgb in;
                        hsv out;
                        
                            in.r = *lpSrc;
                            in.g = *(lpSrc +1);
                            in.b = *(lpSrc +2);
                            out = rgb2hsv(in); 
                            cmyk = hsvToCMY((WORD)out.h, (WORD)out.s, (WORD)out.v, pct);
       
                        }

                        *lpDst       = GetCValue(cmyk);
                        *(lpDst + 1) = GetMValue(cmyk);
                        *(lpDst + 2) = GetYValue(cmyk);                
                        *(lpDst + 3) = GetKValue(cmyk);               
                
                    }
                        
                }
				
            }
	fprintf(stderr, "CATULTRA - 360 passed SendPage Complete\n");
        }
        else
        if (psettings->OEM == OEM_HELIX)
        {
            LONG ycnt, xcnt;
            fprintf(stderr, "CAT pdev->epPaperXdots:%x\n",pdev->epPaperXdots);
            fprintf(stderr, "CAT pdev->epPaperYdots:%x\n",pdev->epPaperYdots);
            
            if (psettings->orientation == 0/*header.cupsHeight != 664*/)// && header.cupsHeight != 991 && header.cupsHeight != 1654)
            {
                xcnt = pdev->epPaperYdots;
                ycnt = (pdev->epPaperXdots - pdev->epOffsetX);
            }
            else
            {
                xcnt = (pdev->epPaperXdots);// - pdev->epOffsetX);
                ycnt = (pdev->epPaperYdots);// - pdev->epOffsetY);

            }
            
            fprintf(stderr, "CATULTRA passed SendPage header.cupsHeight %u\n",header.cupsHeight);
            fprintf(stderr, "CATULTRA passed SendPage header.cupsBytesPerLine %u\n",header.cupsBytesPerLine);
            fprintf(stderr, "CATULTRA passed SendPage header.Orientation %u\n",header.Orientation);
            for (y = 0; y < header.cupsHeight; y++)
            {
		//fprintf(stderr, "CAT processing y:%u\n",y);
		if (CUPSRASTERREADPIXELS(ras, rasterData, header.cupsBytesPerLine) < 1)
		{
			fprintf(stderr, "CAT breaking out L6253!!%u\n",y);
			break;
		}
		lpSrc = rasterData;
				
                for (x = 0; x < (header.cupsBytesPerLine / 3); x++)
                {
                   //           1036                               664
                    //    --------------------------        ----------------  
                    //    -oooo                             -               
                    //    -                                 -                 
                    //    -                                 -                 
                    //648 -                                 -                 
                    //    -                                 -                 
                    //    -                                 -                 
                    //    -                                 -                 
                    //                                      -                 
                    //                                      -                 
                    //                                      -o                
                                                            
                    //landscape
                    if (psettings->orientation == 0)////header.cupsHeight != 664)// && header.cupsHeight != 991 && header.cupsHeight != 1654)
                    {
                        if (pdev->bRotate)
							lpDst = (PBYTE)pSurface + (((xcnt - 1) - x ) * pdev->lDelta) + (((ycnt - 1) - y) * NextPixel);
                        else
                            lpDst = (PBYTE)pSurface + ( x * pdev->lDelta) + (y * NextPixel);
					}
                    else
                    {
                        if (pdev->bRotate) //180 flip
                            lpDst = (PBYTE)pSurface + (y * pdev->lDelta) + (((xcnt - 1) - x) * NextPixel);
                        else
                            lpDst = (PBYTE)pSurface + (((ycnt - 1) - y) * pdev->lDelta) + (x * NextPixel);
                        
                    }
                    // set RGB pixels into 32bpp/8bpp plane
                    srcColour = RGB(lpSrc[x*3],lpSrc[(x*3)+1],lpSrc[(x*3)+2]);
                    
                    if (pdev->eBlackStartOption == UICBVAL_Start_YMC || pdev->eChannelOption == UICBVAL_YMC)
                        cmyk = RGB_to_CMY(&srcColour);
                    else
						cmyk = RGB_to_CMYK((ULONG *)&srcColour);
					
                    if (pdev->eChannelOption == UICBVAL_KResin)
                        *lpDst       = ~RgbToGray(lpSrc[x*3],lpSrc[(x*3)+1],lpSrc[(x*3)+2]);
                    else
                    {
						//CHARPIXEL cp;
						
                        //built in colormatching requested?
                        if (bColourMatch && cmyk != (LONG)0xff)
                        {
                        rgb in;
                        hsv out;
                        
                            in.r = *lpSrc;
                            in.g = *(lpSrc +1);
                            in.b = *(lpSrc +2);
                            out = rgb2hsv(in); 
                            cmyk = hsvToCMY((WORD)out.h, (WORD)out.s, (WORD)out.v, pct);
       
                        }

                        *lpDst       = GetCValue(cmyk);
                        *(lpDst + 1) = GetMValue(cmyk);
                        *(lpDst + 2) = GetYValue(cmyk);                
                        *(lpDst + 3) = GetKValue(cmyk);               
                
                    }
                    
                }
            }
        }
        else
        {
            LONG ycnt, xcnt;
 			//static CHARPIXEL cp2;
			//cp2.r = 255;
fprintf(stderr, "CAT pdev->epPaperXdots:%x\n",pdev->epPaperXdots);	
fprintf(stderr, "CAT pdev->epPaperYdots:%x\n",pdev->epPaperYdots);	

            if (psettings->orientation == 0)///*header.cupsHeight > header.cupsBytesPerLine*/header.cupsHeight != 1016 && header.cupsHeight != 991 && header.cupsHeight != 1654)
            {
                ycnt = pdev->epPaperYdots;
                xcnt = (pdev->epPaperXdots - pdev->epOffsetX);
            }
            else
            {
                ycnt = (pdev->epPaperXdots);// - pdev->epOffsetY);
                xcnt = (pdev->epPaperYdots);// - pdev->epOffsetX);
            }
   
fprintf(stderr, "CATULTRA passed SendPage header.cupsHeight %u\n",header.cupsHeight);         
fprintf(stderr, "CATULTRA passed SendPage header.cupsBytesPerLine %u\n",header.cupsBytesPerLine);         
fprintf(stderr, "CATULTRA passed SendPage header.Orientation %u\n",header.Orientation);
fprintf(stderr, "CATULTRA psettings->nColourCorrection %u\n",psettings->nColourCorrection);
            for (y = 0; y < header.cupsHeight; y++)
            {
		//fprintf(stderr, "CAT processing y:%u\n",y);	
		if (CUPSRASTERREADPIXELS(ras, rasterData, header.cupsBytesPerLine) < 1)
		{
			fprintf(stderr, "CAT breaking out L6253!!%u\n",y);				
			break;
		}
		lpSrc = rasterData;
				
                for (x = 0; x < (header.cupsBytesPerLine / 3); x++)
                {
                    //           1016                               648     
                    //    --------------------------        ----------------  
                    //    -oooo                             -               
                    //    -                                 -                 
                    //    -                                 -                 
                    //648 -                                 -                 
                    //    -                                 -                 
                    //    -                                 -                 
                    //    -                                 -                 
                    //                                      -                 
                    //                                      -                 
                    //                                      -o                
                                                            
                    //landscape
                    if (psettings->orientation == 0)//header.cupsHeight != 1016 && header.cupsHeight != 991 && header.cupsHeight != 1654)
                    {
                        //90
                        //image[original_height - x][y] /* 90 degrees cw */
                        if (pdev->bRotate)
                            lpDst = (PBYTE)pSurface + (((xcnt - 1) - x) * pdev->lDelta) + ((y /*+ pdev->epOffsetY*/) * NextPixel);
                        //270
                        //image[x][original_width - y] /* rotated 90 degrees ccw */
                        else
                            lpDst = (PBYTE)pSurface + ((x /*+ pdev->epOffsetX*/) * pdev->lDelta) + (((ycnt - 1) - y) * NextPixel);
                    }    
                    else
                    {
                        if (pdev->bRotate) //180 flip
                            lpDst = (PBYTE)pSurface + (((ycnt - 1) - y ) * pdev->lDelta) + (((xcnt - 1) - x) * NextPixel);
                        else  
                            lpDst = (PBYTE)pSurface + (y * pdev->lDelta) + ((x + pdev->epOffsetX) * NextPixel);
                    }
                    // set RGB pixels into 32bpp/8bpp plane
                    srcColour = RGB(lpSrc[x*3],lpSrc[(x*3)+1],lpSrc[(x*3)+2]);
					

                    if (pdev->eBlackStartOption == UICBVAL_Start_YMC || pdev->eChannelOption == UICBVAL_YMC)
					{
                        cmyk = RGB_to_CMY(&srcColour);
			//fprintf(stderr, "cmyk:%x\n",cmyk);		
					}
                    else			   
						cmyk = RGB_to_CMYK((ULONG *)&srcColour);
              
                   if (pdev->eChannelOption == UICBVAL_KResin)
                        *lpDst       = RgbToGray(lpSrc[x*3],lpSrc[(x*3)+1],lpSrc[(x*3)+2]);
                    else
                    {
 						static CHARPIXEL cp;
						
                        //built in colormatching requested?
                        if (bColourMatch && cmyk != (LONG)0xff)
                        {
							
							/*if (bYaFilm == FALSE && psettings->nColourCorrection == UICBVAL_ColourCorrection_ICC_Internal)
							{							
								rgb in;
								hsv out;
							
								in.r = *lpSrc;
								in.g = *(lpSrc +1);
								in.b = *(lpSrc +2);
								out = rgb2hsv(in); 
								cmyk = hsvToCMY((WORD)out.h, (WORD)out.s, (WORD)out.v, pct);
							}
							else*/
							{
								cp.r = lpSrc[x*3];
								cp.g = lpSrc[(x*3)+1];
								cp.b = lpSrc[(x*3)+2];
  
								if (/*bYaFilm && */psettings->nColourCorrection == UICBVAL_ColourCorrection_ICC_Internal)
									ColourCorrect(&cp, 4);
								else
									ColourCorrect(&cp, psettings->nColourCorrection - 2);

								cmyk = ((DWORD)(((BYTE)(cp.c)<<24|((BYTE)(cp.m)<<16))|((BYTE)(cp.y)<<8))|((BYTE)(0)));								
							}
       
                        }

                        *lpDst       = GetCValue(cmyk) >> 2;
                        *(lpDst + 1) = GetMValue(cmyk) >> 2;
                        *(lpDst + 2) = GetYValue(cmyk) >> 2;                
                        *(lpDst + 3) = GetKValue(cmyk) >> 2;               
                
                    }


                    
                }

            }
        }
    }
	
    dvOutputStrip(pdev, psettings, pSurface);
	
return TRUE;
}

/****************************************************************************
 *
 *  DecStringToDWORD()
 *      Converts a decimal string to a DWORD value.
 *
 *  Type:
 *      Global function.
 *
 *  Parameters:
 *      chDecString      String containing decimal data.
 *      pdwDecValue      Pointer to DWORD that will return value.
 *
 *  Returns:
 *      Normally TRUE.  However if the function is passed a string of
 *      zero length or contains non-decimal data, it will abort and
 *      return FALSE.
 *
 ****************************************************************************/

BOOL DecStringToDWORD(LPBYTE chDecString, DWORD *pdwDecValue)
{
    int   i;
    int   iNumChars = (int)strlen((char *)chDecString);
    DWORD dwTemp    = 0;
    BOOL  fReturn   = FALSE;

    if (iNumChars != 0)
	{
		// Loop through the string and construct the number
		for (i = 0; i < iNumChars; i++)
		{
			int iDigit     = 0;
			int iPowerOf10 = (int)pow((double)10, (double)iNumChars - i - 1);

			if (*(chDecString + i) >= '0' && *(chDecString + i) <= '9')
				iDigit = *(chDecString + i) - '0';
			else
				goto error_recovery;

			dwTemp += iDigit * iPowerOf10;
		}

		// We have constructed a number! Set return value to TRUE
		*pdwDecValue = dwTemp;

		fReturn = TRUE;
	}

error_recovery:
    return fReturn;
}

/****************************************************************************
 *
 *  ParseEnduroStatus()
 *     Called to parse the Enduro Status string. 
 *
 *  Parameters:
 *      pLMInst           Language Monitor Instance data
 *      pEnduroStatusData Pointer to buffer containing data from Printer
 *
 *  Returns:
 *      None.
 *
 ****************************************************************************/

VOID ParseEnduroStatus
(
	SETTINGS *	pSettings,
	char *pEnduroStatusData
)
{
    //PCINSTANCE     pCInst   = (PCINSTANCE)pIniPort->CustomInstanceData;
    //PCPUTBLOCK pCPut = (PCPUTBLOCK)pIniPort->CustomPutBlock;
	PENDURO_STATUS pEStatus = &pSettings->es;
	LPBYTE         pszData  = (LPBYTE)pEnduroStatusData;
	BYTE           DataID   = 0;

	pEStatus->bPrinterConnected = TRUE;

	// Walk the string. When one of the separators is met, then we change this
	// to a NULL. We then process the current data and reset the pointers to 
	// the start of the next data entry
	while (*pEnduroStatusData != 0)
    {
		switch (*pEnduroStatusData)
        {
		case ':':
		case ',':
		case 0x03:
            // Change the ',' to a NULL to terminate the data section
            *pEnduroStatusData++ = 0;

            // Process the data strings
            switch (DataID)
            {
			case 0:  
				DecStringToDWORD(pszData, &pEStatus->eModel);
                ////WriteToRegistry(pIniPort, REGKEY_ES_MODEL, pEStatus->eModel);
				break;
			case 1:  strcpy(pEStatus->sModel,           (char *)pszData);       break;   
			case 2:  DecStringToDWORD(pszData, &pEStatus->ePrintheadType);      break;
			case 3:  strcpy(pEStatus->sPrinterSerial,   (char *)pszData);       break;   
			case 4:  strcpy(pEStatus->sPrintheadSerial, (char *)pszData);       break;   
			case 5:  strcpy(pEStatus->sPCBSerial,       (char *)pszData);       break;   
			case 6:  strcpy(pEStatus->sFirmwareVersion, (char *)pszData);       break;  
/*			case 6:  
                {
                //HANDLE hPrinter = 0;
                DWORD dwType;
                //HKEY hKey = NULL;
                //time_t theTime   = time(NULL);
                //struct tm *aTime = localtime(&theTime);
                //DWORD day    = (DWORD)aTime->tm_mday;
                //DWORD dwday  = 0;
                //DWORD dwValue;
                //BOOL bPollFW = FALSE; 
                
                    //MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszData, -1, 
                    //                    pEStatus->sFirmwareVersion, SERIAL_SIZE);
                    //WriteStringToRegistry(pIniPort, REGKEY_ES_FWVERSION, pEStatus->sFirmwareVersion);

                    //perform a check via the status monitor for new fw version????
                    //place last day checked in registry
                    if (OpenPrinter(pIniPort->pszPrinterName, &hPrinter, &PrinterDefault)) 
                    {
                        DWORD dwTemp;
						//ensure fw checking hasn't been disabled.. requested by support
                        if (GetPrinterDataW(hPrinter, 
                                           STATUS_DISABLEFWCHECK, 
                                           NULL,
                                           (LPBYTE)&dwValue,
                                           sizeof(DWORD),
                                           (LPDWORD)&dwTemp) != ERROR_SUCCESS)
                        {
							
							if (GetPrinterDataW(hPrinter, 
											   STATUS_FWCHECKDATE, 
											   NULL,
											   (LPBYTE)&dwday,
											   sizeof(DWORD),
											   (LPDWORD)&dwTemp)== ERROR_SUCCESS)
							{
								if (dwday != (DWORD)0xffff)
								{
									if (dwday != day)
										bPollFW = TRUE;
								}
							}
									
						
							if (bPollFW)
							{
								pEStatus->iMajorError = 0;
								pCPut->bClose = 999;

								NotifyStatusMonitor(pIniPort);

								if (IsStatusMonitorActive(pIniPort) == FALSE)
									StartStatusMonitor(pIniPort);
								
								//HandleEnduroError(pIniPort, pEStatus, TRUE);                       
								//only poll once a day!  
								SetPrinterDataW(hPrinter, STATUS_FWCHECKDATE, REG_DWORD, (LPBYTE)&day, sizeof(DWORD));   
								
							}
							else
								pCPut->bClose = 0; //clear 
						}

                        ClosePrinter(hPrinter);
                    }

                }
				break;   
*/
			case 7:  

				if (*pszData != '3')
				{
					pEStatus->iES_Density = DEFAULT_RIOPRO_DENSITY;
				}
				else
				{
					pszData += 2;	//Move to density value
					DecStringToDWORD(pszData, &pEStatus->iES_Density);
				}
                //WriteToRegistry(pIniPort, REGKEY_ES_DENSITY, pEStatus->iES_Density);
				break;
			case 8:  DecStringToDWORD(pszData, &pEStatus->iHandFeed);           break;
			case 9:  DecStringToDWORD(pszData, &pEStatus->iCardsPrinted);       break;
			case 10: DecStringToDWORD(pszData, &pEStatus->iCardsOnPrinthead);   break;
			case 11: DecStringToDWORD(pszData, &pEStatus->iDyePanelsPrinted);   break;
			case 12: DecStringToDWORD(pszData, &pEStatus->iCleansSinceShipped); break;
			case 13: DecStringToDWORD(pszData, &pEStatus->iDyePanelsSinceClean);break;
			case 14: DecStringToDWORD(pszData, &pEStatus->iCardsSinceClean);    break;
			case 15: 
				DecStringToDWORD(pszData, &pEStatus->iCardsBetweenCleans);
                //WriteToRegistry(pIniPort, 
				//				  REGKEY_ES_CLEAN, 
				//				  pEStatus->iCardsSinceClean >= pEStatus->iCardsBetweenCleans 
				//											    ? ON : OFF);
				break;
			case 16: 
				DecStringToDWORD(pszData, &pEStatus->iPrintHeadPosn);
                //WriteToRegistry(pIniPort, REGKEY_FPP, pEStatus->iPrintHeadPosn);     
				break;
			case 17: DecStringToDWORD(pszData, &pEStatus->iImageStartPosn); break;
			case 18: DecStringToDWORD(pszData, &pEStatus->iImageEndPosn);   break;
			case 19: DecStringToDWORD(pszData, &pEStatus->iMajorError);     break;
			case 20: 
				DecStringToDWORD(pszData, &pEStatus->iMinorError);
                ////HandleEnduroError(pIniPort, pEStatus);
				break;
			case 21: strcpy(pEStatus->sTagUID, (char *)pszData);            break;
			case 22: DecStringToDWORD(pszData, &pEStatus->iShotsOnFilm);    break;
			case 23: DecStringToDWORD(pszData, &pEStatus->iShotsUsed);      break;
			case 24: strcpy(pEStatus->sDyeFilmType, (char *)pszData);       break;     
			case 25: 
				DecStringToDWORD(pszData, &pEStatus->iColourLength);   
                //WriteToRegistry(pIniPort, REGKEY_ES_COLOUR, pEStatus->iColourLength);
				break;
			case 26: 
				DecStringToDWORD(pszData, &pEStatus->iResinLength);
                //WriteToRegistry(pIniPort, REGKEY_ES_RESIN, pEStatus->iResinLength);
				break;
			case 27: 
				DecStringToDWORD(pszData, &pEStatus->iOvercoatLength);
                //WriteToRegistry(pIniPort, REGKEY_ES_OVERLAY, pEStatus->iOvercoatLength);
				break;
			case 28: DecStringToDWORD(pszData, &pEStatus->eDyeFlags);       break;
			case 29: DecStringToDWORD(pszData, &pEStatus->iCommandCode);    break;
			case 30: DecStringToDWORD(pszData, &pEStatus->iDOB);            break;
			case 31: DecStringToDWORD(pszData, &pEStatus->eDyeFilmManuf);   break;
			case 32: DecStringToDWORD(pszData, &pEStatus->eDyeFilmProg);    break;
			case 33:
				pEStatus->iBitFields &= ~(SEM_DEFAULT | SEM_PLATEN);
				switch (*pszData)
				{
				case '0': 
//					DEBUG_MSG("LM - SEM_DEFAULT\n");
					pEStatus->iBitFields |= SEM_DEFAULT; 
					break;
				case '1': 
//					DEBUG_MSG("LM - SEM_PLATEN\n");
					pEStatus->iBitFields |= SEM_PLATEN;  
					break;
				default:
//					DEBUG_MSG("LM - SEM_UNKNOWN\n");
					break;
				}
				break;
			
			case 34: DecStringToDWORD(pszData, &pEStatus->iDummy1); break;
            }

			DataID++;

            // Set the data pointer to the character after the ',' or ':'
			pszData = (PBYTE)pEnduroStatusData;
			break;

		default:
			// Increment the pointer to the Enduro Status buffer
			pEnduroStatusData++;
			break;
        }
    }
}

BOOL SetUpLUT(PDEVDATA pdev, PSETTINGS settings)
{
        int		i,j,k;//ret;
        PBYTE	pGlobalMem = 0;
//        int		nLoadTable = RIO_LUT;		//0 = Rio 1=Enduro+ 2=Helix
        BOOL 	bLoadYaInternal = FALSE;
        
        if (bLUTLoaded)
            return TRUE;
        
fprintf(stderr, "CATULTRA: Entering SetUpLUT \n");
        //determine this once for speed
/*         if (ENDURO_PLUS_TYPE(settings))
            nLoadTable = ENDURO_LUT; */
        
        //GetSystemDirectoryA((PSTR)&szPrinterDriverDirectory,MAX_PATH);
        
/*       if (HELIX_OEM(settings))
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
                    pGlobalMem = (PBYTE)&MAGIRX;
 //           }
  //      }
  */
		//ya film only for now..
        pGlobalMem = (PBYTE)&MAGIRY;
		bLoadYaInternal = TRUE;
        
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
   fprintf(stderr, "CATULTRA: bLoadYaInternal:%x \n",bLoadYaInternal);           
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
            
fprintf(stderr, "CATULTRA: LUT Set \n");
            bLUTLoaded = TRUE;
        }
        else
        {
            //DBGMSG(DBG_LEVEL_VERBOSE,L"COLRCORR.C: can't open the intents file");
            return FALSE;
            
        }
        
        
        return TRUE;
}
    
/*----------------------------------------------------------------------------*/
void crc32update(PDEVDATA pdev, const void *data, DWORD length)
{
    const uint8 *ptr = (uint8 *)data;
    const uint8 *const ptrEnd = ptr + length;

    while (ptr != ptrEnd) {
        pdev->epCheckSum = table[*ptr++ ^ ((pdev->epCheckSum >> 24) & 0xff)] ^ (pdev->epCheckSum << 8);
    }
}
    
/*----------------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    int                 fd                      = 0;        /* File descriptor providing CUPS raster data                                           */
    cups_raster_t *     ras                     = NULL;     /* Raster stream for printing                                                           */
    cups_page_header2_t  header;                             /* CUPS Page header                                                                     */
    int                 page                    = 0;        /* Current page                                                                         */

////    int                 y                       = 0;        /* Vertical position in page 0 <= y <= header.cupsHeight                                */
////    int                 i                       = 0;

    unsigned char *     rasterData              = NULL;     /* Pointer to raster data buffer                                                        */
    unsigned char *     originalRasterDataPtr   = NULL;     /* Copy of original pointer for freeing buffer                                          */
////    int                 leftByteDiff            = 0;        /* Bytes on left to discard                                                             */
////    int                 numBlankScanLines       = 0;        /* Number of scanlines that were entirely black                                         */
    struct settings_    settings;                           /* Configuration settings                                                               */
//	SETTINGS *			psettings = &settings;
	PDEVDATA            pdev;
	
//	struct BITMAPFILEHEADER bmf;
//	struct BITMAPINFOHEADER bmi;
//	unsigned char *		Planes[3];							/* Output buffers */
//	unsigned char *     pbmi=(unsigned char *)&bmi;
//	unsigned long		offset;
//	unsigned long		x;//,cb;
////	unsigned long		outputheight;//,yBorderOffset=0;
	char				buffer[256];
	int					bytes					=0;
	int					retries					=2;			/* number of retries for status */

#ifdef RPMBUILD
    void * libCupsImage = NULL;                             /* Pointer to libCupsImage library                                                      */
    void * libCups      = NULL;                             /* Pointer to libCups library                                                           */

    libCups = dlopen ("libcups.so", RTLD_NOW | RTLD_GLOBAL);
    if (! libCups)
    {
        fputs("ERROR: libcups.so load failure\n", stderr);
        return EXIT_FAILURE;
    }

    libCupsImage = dlopen ("libcupsimage.so", RTLD_NOW | RTLD_GLOBAL);
    if (! libCupsImage)
    {
        fputs("ERROR: libcupsimage.so load failure\n", stderr);
        dlclose(libCups);
        return EXIT_FAILURE;
    }

    GET_LIB_FN_OR_EXIT_FAILURE(ppdClose_fn,             libCups,      "ppdClose"             );
    GET_LIB_FN_OR_EXIT_FAILURE(ppdFindChoice_fn,        libCups,      "ppdFindChoice"        );
    GET_LIB_FN_OR_EXIT_FAILURE(ppdFindMarkedChoice_fn,  libCups,      "ppdFindMarkedChoice"  );
    GET_LIB_FN_OR_EXIT_FAILURE(ppdFindOption_fn,        libCups,      "ppdFindOption"        );
    GET_LIB_FN_OR_EXIT_FAILURE(ppdMarkDefaults_fn,      libCups,      "ppdMarkDefaults"      );
    GET_LIB_FN_OR_EXIT_FAILURE(ppdOpenFile_fn,          libCups,      "ppdOpenFile"          );
    GET_LIB_FN_OR_EXIT_FAILURE(cupsFreeOptions_fn,      libCups,      "cupsFreeOptions"      );
    GET_LIB_FN_OR_EXIT_FAILURE(cupsParseOptions_fn,     libCups,      "cupsParseOptions"     );
    GET_LIB_FN_OR_EXIT_FAILURE(cupsMarkOptions_fn,      libCups,      "cupsMarkOptions"      );
    GET_LIB_FN_OR_EXIT_FAILURE(cupsRasterOpen_fn,       libCupsImage, "cupsRasterOpen"       );
    GET_LIB_FN_OR_EXIT_FAILURE(cupsRasterReadHeader_fn, libCupsImage, "cupsRasterReadHeader" );
    GET_LIB_FN_OR_EXIT_FAILURE(cupsRasterReadPixels_fn, libCupsImage, "cupsRasterReadPixels" );
    GET_LIB_FN_OR_EXIT_FAILURE(cupsRasterClose_fn,      libCupsImage, "cupsRasterClose"      );
#endif

    if (argc < 6 || argc > 7)
    {
        fputs("ERROR: rastertoultra job-id user title copies options [file]\n", stderr);

        #ifdef RPMBUILD
            dlclose(libCupsImage);
            dlclose(libCups);
        #endif

        return EXIT_FAILURE;
    }

    if (argc == 7)
    {
        if ((fd = open(argv[6], O_RDONLY)) == -1)
        {
            perror("ERROR: Unable to open raster file - ");
            sleep(1);

            #ifdef RPMBUILD
                dlclose(libCupsImage);
                dlclose(libCups);
            #endif

            return EXIT_FAILURE;
        }
    }
    else
    {
        fd = 0;
    }
	fflush (stdout);

    	pdev=malloc(sizeof(DEVDATA));
	if (pdev == 0)
		return EXIT_FAILURE;

	memset(pdev, 0, sizeof(DEVDATA));
	
	initializeSettings(argv[5], &settings, pdev);


fprintf(stderr, "DOGULTRA L7683 settings.modelNumber %u\n",settings.modelNumber);		
    jobSetup(settings,argv);
fprintf(stderr, "DOGULTRA L7685 settings.Printer %u\n",settings.Printer);	
    ras = CUPSRASTEROPEN(fd, CUPS_RASTER_READ);

    page = 0;
// retrieve enduro status
    if (settings.OEM != OEM_HELIX && settings.OEM != OEM_PRO360)
    {
		//try this bob!
		for (; retries; retries --)
		{
			
			printf("\x01,REQ,INF,\x1C\x03");

			fflush (stdout);
			
			memset(buffer,0,sizeof(buffer));
			bytes = cupsBackChannelRead(buffer,sizeof(buffer),2.0);
			if (bytes)
			{
				fprintf(stderr, "CAT %d got something??%s\n", bytes, buffer);
					if (strncmp("STA$$",(char*)&buffer[1], 5) == 0)
						ParseEnduroStatus(&settings, &buffer[6]);
						
					fprintf(stderr, "CRIT: settings.sModel %s\n", settings.es.sModel);
					fprintf(stderr, "CRIT: settings.iPrintHeadPosn %u\n", settings.es.iPrintHeadPosn);
					//fprintf(stderr, "ATTR: marker-message=%d Prints remaining\n", remprints);
				break;	
			}
			else
				continue;
		}
    //initialise LUT
    SetUpLUT(pdev, &settings);
	}

    
    while (CUPSRASTERREADHEADER(ras, &header))
    {
        if ((header.cupsHeight == 0) || (header.cupsBytesPerLine == 0))
        {
			////fprintf(stderr, "CAT breaking out L620 PAGE %u\n",page);
            break;
        }

        if (rasterData == NULL)
        {
            rasterData = malloc(header.cupsBytesPerLine);
            if (rasterData == NULL)
            {
                CLEANUP;
                return EXIT_FAILURE;

            }
            originalRasterDataPtr = rasterData;  // used to later free the memory
        }
fprintf(stderr, "CATULTRA: header.Duplex = %d \n"    , header.Duplex);	
fprintf(stderr, "CATULTRA: header.LeadingEdge = %d \n"    , header.LeadingEdge);	
        page++;
		pdev->iPageNumber = page;
		
		//output our header data for the page
        pageSetup(pdev, settings, header);


		

//        numBlankScanLines = 0;
		//1926 .. 642 * 3
		//passed cups line width less than normal width?
 /*       if (header.cupsBytesPerLine <= settings.bytesPerScanLine)
        {
            settings.bytesPerScanLine = header.cupsBytesPerLine;
            leftByteDiff = 0;
        }
        else
        {
            settings.bytesPerScanLine = settings.bytesPerScanLineStd;

            switch (settings.focusArea)
            {
                case FOCUS_LEFT:
                    leftByteDiff = 0;
                    break;
                case FOCUS_CENTER:
                    leftByteDiff = ((header.cupsBytesPerLine - settings.bytesPerScanLine) / 2);
                    break;
                case FOCUS_RIGHT:
                    leftByteDiff = (header.cupsBytesPerLine - settings.bytesPerScanLine);
                    break;
            }
        }
		//center output
*/
		//leftByteDiff = ((1920 - header.cupsWidth) / 2);		
		//allocate memory for our 3 output planes 
		//outputheight = header.cupsHeight;	

		//put some constraints in here..
fprintf(stderr, "CATULTRA header.cupsHeight: %u, header.cupsBytesPerLine:%u\n",header.cupsHeight, header.cupsBytesPerLine); 		
fprintf(stderr, "DOGULTRA L7779 settings.Printer %u\n",settings.Printer);		
////////HERE WE GO!!
		SendPage(pdev, &settings, header, ras, rasterData);
fprintf(stderr, "CATULTRA passed SendPage PAGE %u\n",page);

        endPage(pdev, &settings, header);

////fprintf(stderr, "CAT FREEING PLANES!!%u\n",page);			
//		free(Planes[0]);													
//		free(Planes[1]);													
//		free(Planes[2]);
    }
	
    endJob(pdev, settings, header);

    CLEANUP;
	
	if (page == 0 )
    {
        fputs("ERROR: No pages found!\n", stderr);
    }
    else
    {
        fputs("INFO: Ready to print.\n", stderr);
    }

    return (page == 0)?EXIT_FAILURE:EXIT_SUCCESS;
}
 
    // end of rastertoultra.c
