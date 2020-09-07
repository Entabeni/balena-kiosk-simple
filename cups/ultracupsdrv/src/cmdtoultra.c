/*
 * Magicard Ltd
 *
 * CUPS Filter
 *
 * [ Linux ]
 * compile cmd: gcc -Wl,-rpath,/usr/lib -Wall -fPIC -O2 -o cmdtoultra cmdtoultra.c -lcupsimage -lcups
 * compile requires cups-devel-1.1.19-13.i386.rpm (version not neccessarily important?)
 * find cups-devel location here: http://rpmfind.net/linux/rpm2html/search.php?query=cups-devel&submit=Search+...&system=&arch=
 *
 * [ Mac OS X ]
 * compile cmd: gcc -Wall -fPIC -o cmdtoultra cmdtoultra.c -lcupsimage -lcups -arch ppc -arch i386 -DMACOSX
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
#include <cups/backend.h>
#include <stdlib.h>
#include <fcntl.h>

#include "magigen.h"
#include "gndefs.h"
//#include "command.h"
#include "comms.h"
//#include "colortab.h"
//#include "colrmtch.c"
//#include <math.h>
#include "pr2comms.h"

#ifdef RPMBUILD

#include <dlfcn.h>

typedef cups_raster_t * (*cupsRasterOpen_fndef)(int fd, cups_mode_t mode);
typedef unsigned (*cupsRasterReadHeader_fndef)(cups_raster_t *r, cups_page_header_t *h);
typedef unsigned (*cupsRasterReadPixels_fndef)(cups_raster_t *r, unsigned char *p, unsigned len);
typedef void (*cupsRasterClose_fndef)(cups_raster_t *r);

static cupsRasterOpen_fndef cupsRasterOpen_fn;
static cupsRasterReadHeader_fndef cupsRasterReadHeader_fn;
static cupsRasterReadPixels_fndef cupsRasterReadPixels_fn;
static cupsRasterClose_fndef cupsRasterClose_fn;

#define CUPSRASTEROPEN (*cupsRasterOpen_fn)
#define CUPSRASTERREADHEADER (*cupsRasterReadHeader_fn)
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
#define CUPSRASTERREADHEADER cupsRasterReadHeader
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

WORD StripTrail(char * lpBits, WORD buflen)
{
	char * bufptr;
	
    for ( bufptr = lpBits + buflen; buflen; buflen--, bufptr --)
		if (bufptr[0])
			break;    /* remove trailing nulls */
	
	return buflen;
}

int main(int argc, char *argv[])
{
    int                 fd                      = 0;        /* File descriptor providing CUPS raster data                                           */
//	char				buffer[256];
//	int				bytes=0;


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
        fputs("ERROR: cmdtoultra job-id user title copies options [file]\n", stderr);

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

    if (strlen(argv[3]))
    {
/*        if (strcmp(argv[3],"query") == 0)
        {
			fprintf(stderr, "L217:CAT about to query\n");
			printf("\x01,REQ,INF,\x1C\x03");
			
			fflush (stdout);
			
			memset(buffer,0,sizeof(buffer));
			bytes = cupsBackChannelRead(buffer,sizeof(buffer),2.0);
			if (bytes)
			{
				////if (strncmp("STA$$",(char*)&buffer[1], 5) == 0)
				////	ParseEnduroStatus(&settings, &buffer[6]);
				
				////fprintf(stderr, "CRIT: settings.sModel %s\n", settings.es.sModel);
				////fprintf(stderr, "CRIT: settings.iPrintHeadPosn %u\n", settings.es.iPrintHeadPosn);
				//fprintf(stderr, "ATTR: marker-message=%d Prints remaining\n", remprints);
				FILE *fp=fopen("/users/shared/query.txt","wb");
				fprintf(stderr, "CAT %d got something??%s\n", bytes, buffer);

				////fprintf(stderr, "file handle %d\n", (int)fp);
				if (fp)
				{
					CHAR	chQPRCommand[64];
					CHAR	szOutput[4096];
					CHAR    szReturnedData[8192];
					char *  pRetBuf=(char *)&szReturnedData;
					//DWORD   dwReturned=0;
					char *  pch;
					int		nQPRLine = 1;
					int		dwRead = 0;
					int     dwDataReceived = 0;
					int     cbRetBufSize = 8192;
					memset((char *)&chQPRCommand, 0, sizeof(chQPRCommand));
					
					do 
					{
				fprintf(stderr, "About to send request for line %d\n", nQPRLine);
						printf("\x05\x05\x05\x05\x05\x05\x05\x05\x01QPR%u,\x1C\x03",nQPRLine++);
						fflush (stdout);
						////OutputDebugStringA((PSTR)&chQPRCommand);
						//if (bWrite)
						{
							
							memset((char *)&szOutput, 0, sizeof(szOutput));

							// Read data from the port.
							//MAGMonReadPort(pIniPort, (PBYTE)&szOutput, sizeof(szOutput), &dwRead);
							dwRead = cupsBackChannelRead((char *)szOutput,sizeof(szOutput),2.0);
							fflush (stdout);
							////OutputDebugStringA((PSTR)&szOutput);
							if (dwRead != -1)
							{
				fprintf(stderr, "Received %s\n", szOutput);
								if (dwRead > 3 && szOutput[0] == '@' && szOutput[1] == '@' && szOutput[2] == '@')
									continue;
								
								pch = strrchr((char *)&szOutput, 0x03);
								if (pch)
									*pch = '\040';
								pch = strrchr((char *)&szOutput, 0x03);
								if (pch)
									*pch = '\040';
								//strip trailing nulls if returned
				fprintf(stderr, "dwRead:%u, dwDataReceived %d\n", dwRead, dwDataReceived);
								if (dwRead > 80)
									dwRead = StripTrail((char *)&szOutput, (WORD)dwRead);
								
								dwDataReceived += (dwRead + 2);
								
								if (dwDataReceived > cbRetBufSize)
									break;
								
								memcpy(pRetBuf, (char *)&szOutput, dwRead);
								pRetBuf[dwRead] = '\015';
								pRetBuf[dwRead+1] = '\012';
								
								if (strcmp((char *)&szOutput, "END")==0)
									break;
								
								memset((char *)&szOutput, 0, sizeof(szOutput));
								pRetBuf += (dwRead + 2);
							}
							else
								break;
						}
					} while (dwRead);
					
				fprintf(stderr, "about to write %d\n", bytes);
					//fwrite(buffer, 1, bytes, fp);
				fprintf(stderr, "about to write %d\n", dwDataReceived);
					//fwrite(szReturnedData, 1, dwDataReceived, fp);
					szReturnedData[dwDataReceived-5]=0;
					fprintf(fp,"%s%s",buffer, szReturnedData);
					fclose(fp);
					
				}
				
			}
			

			
            return EXIT_SUCCESS;
        }
*/		
        if (strcmp(argv[3],"Clean Print Heads") == 0)
        {
            //send a command to the printer to clean the print head
            printf("\x05\x05\x05\x05\x05\x05\x05\x05\x01,REQ,CLN,\x1C\x03");
                
            fflush (stdout);
            return EXIT_SUCCESS;
        }
        
        if (strcmp(argv[3],"Print Self-Test Page") == 0)
        {
            //send a command to the printer to request a test page
             printf("\x05\x05\x05\x05\x05\x05\x05\x05\x01,REQ,TST,\x1C\x03");
            
            fflush (stdout);
            return EXIT_SUCCESS;
        }
        
    }

    return EXIT_SUCCESS;
}

// end of cmdtoultra.c
