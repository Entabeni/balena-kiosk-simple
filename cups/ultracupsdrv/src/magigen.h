/***************************************************************************/
/**                                                                       **/
/**                            MAGICARD LTD                               **/
/**                                                                       **/
/***************************************************************************/
/**                                                                       **/
/**  PROJECT      : UltraLinux                                            **/
/**                                                                       **/
/**  MODULE NAME  : MagiGen.h                                             **/
/**                                                                       **/
/**  COPYRIGHT    : Magicard Ltd				                          **/
/**                                                                       **/
/***************************************************************************/

#ifndef MAGI_GENERAL
#define MAGI_GENERAL

#define MAX_CARD_WIDTH  1016
#define MAX_CARD_HEIGHT 642
#define PREVIEW_WIDTH    236  /**< The width of a RGB image preview panel. */
#define PREVIEW_HEIGHT   148  /**< The height of a RGB image preview panel. */
#define FILTER_SIZE			60
#define BUFSIZE				512
#define STATUS_BUFFERSIZE   1024

#define MAX_LANGID 3

#define ENGLISH			TEXT("ENG")
#define FRENCH			TEXT("FRA")
#define GERMAN			TEXT("DEU")
#define ITALIAN			TEXT("ITA")
#define SPANISH			TEXT("SPA")
#define PORTUGUESE		TEXT("POR")
#define SIMP_CHINESE	TEXT("SCH")
#define POLISH			TEXT("POL")
#define RUSSIAN			TEXT("RUS")
#define TURKISH			TEXT("TUR")
#define ROMANIAN		TEXT("ROM")
#define KOREAN			TEXT("KOR")
#define TRAD_CHINESE	TEXT("TCH")
#define JAPANESE		TEXT("JPN")
#define GREEK			TEXT("GRK")
#define DANISH			TEXT("DAN")

// Port name
#define PORTNAME_LPT    L"LPT"
#define PORTNAME_USB    L"USB"
#define PORTNAME_TCPIP  L"IP_"
#define PORTNAME_FILE   L"FILE:"
#define PORTNAME_TS		L"TS"
#define PORTNAME_NE		L"NE"


//#############################################################################
typedef int16_t SHORT;
typedef uint16_t /*unsigned short*/ USHORT;
typedef uint8_t /*unsigned char*/ BYTE;
typedef uint8_t /*unsigned char */CHAR;
typedef uint8_t/*unsigned char*/ * PBYTE;
typedef uint8_t/*unsigned char*/ * LPBYTE;
typedef uint16_t/*unsigned short*/ WORD;
typedef uint32_t/*unsigned long*/ DWORD;
typedef DWORD *LPDWORD;
typedef uint16_t/*unsigned int*/ UINT;
typedef uint16_t/*unsigned int*/ INT;
typedef int32_t /*long*/ BOOL;
typedef uint32_t/*unsigned long*/ ULONG;
typedef int32_t/*long*/ LONG;
typedef uint8_t/*void*/ *PVOID;
typedef uint8_t/*void*/ *LPVOID;
typedef float FLOAT;
typedef uint16_t/*wchar_t*/ WCHAR;
typedef uint8_t/*wchar_t */* PWSTR;
typedef void VOID;
typedef DWORD COLORREF;
#define FALSE 0
#define TRUE  (!FALSE)
typedef struct _RECTL
    {
    LONG left;
    LONG top;
    LONG right;
    LONG bottom;
    } 	RECTL;

typedef struct _RECTL *PRECTL;

typedef struct _RECTL *LPRECTL;

typedef struct _POINTL
    {
    LONG x;
    LONG y;
    } 	POINTL;

typedef struct _RGBTRIPLE
{
    BYTE rgbtBlue;
    BYTE rgbtGreen;
    BYTE rgbtRed;
} RGBTRIPLE, *PRGBTRIPLE, *LPRGBTRIPLE;

enum {REGKEY_FPP,
              REGKEY_ES_MODEL,
              REGKEY_ES_CLEAN,
              REGKEY_ES_COLOUR,
              REGKEY_ES_RESIN,
			  REGKEY_ES_OVERLAY,
			  REGKEY_ES_FWVERSION,
			  REGKEY_DEBUG,
			  REGKEY_PAUSING,
			  REGKEY_POLLING_DISABLE,
			  REGKEY_COUNT_05,
			  REGKEY_ES_DENSITY,
			  REGKEY_DRIVER_VERSION,
			  REGKEY_STATUS_FWCHECKDATE,
			  REGKEY_DISABLE_FWUPDATES,
			  REGKEY_ES_FILMMANUFACTURER,
              REGKEY_ES_SERIALNO,
			  REGKEY_ALLOW_SWUPDATES,
			  REGKEY_ALLOW_DATAUSAGE,
			  REGKEY_EULA_AGREED,
			  REGKEY_SPLASH_DISABLE,
			  REGKEY_LOCALE,
			  REGKEY_PRO360_FILMMANUFACTURER
};

enum {OFF, ON};

enum {FRONT, BACK};

#define GEN_DEBUG_FLAG		0x01
#define LM_DEBUG_FLAG		0x02
#define SM_DEBUG_FLAG		0x04
#define ETHERNET_DEBUG_FLAG	0x08
#define GFX_DEBUG_FLAG		0x10
#define ALL_DEBUG			(GEN_DEBUG_FLAG | LM_DEBUG_FLAG | SM_DEBUG_FLAG)

#define NUMBER_05S		64

/*#include "debug.h"
////#define MAGDEBUG
#ifdef LANG_MON
  #define LM_DEBUGGING(x) ((x) & LM_DEBUG_FLAG)
  #define ERROR_MSG(x)  {if (LM_DEBUGGING(iLM_Debug)) ErrorMsg(x);}
#else
#ifdef STATUS_MON
  #define ERROR_MSG(x) {if (bSM_Debug) ErrorMsg(x);}
#else
  #define ERROR_MSG(x, y) {if (DetermineDebugStatus((x))) OutputDebugString((y));}
#endif
#endif
*/

//#############################################################################

#define MAG_PRINTER	 0x01
#define DUO_PRINTER	 0x02
#define CHIP_PRINTER 0x10

enum {  
        //Printer Family assignment set to OEM value in devmode
        OEM_RIO=1, 
        OEM_AOTA,
        OEM_ENDURO,
        OEM_HELIX,
		OEM_PRO360
};

              
enum {  
//Rio Printers
        RIO_TANGO, 
        XXL,
//AOTA Printers
        ALTO,         
        OPERA,        
        TEMPO,        
        AVALON,       
        DCP100,       
//Enduro Printers
        ENDURO,
        MC200,
        PRONTO,
        PRO,  //10
        IDMAKER,
        P4500S,
        P2500S,
        PPC_ID2000,
        PPC_ID2100,
        PPC_ID3100,
        IDM_VALUE,
        IDM_ADVANTAGE,
        IDM_SECURE,
        RIO_PRO_XTENDED, //20
        FAGOO_P310E, 
        ENDURO_PLUS,
        PPC_ID2300,
        IDM_ADVANT_PLUS, 
        P4500S_PLUS, //25
        PPC_ID3300,
		REWRITEPRINTER_A,
		REWRITEPRINTER_B,
		SECUPRINT_200,
		PRIDENTO_PLUS, //30
		PRIDENTO,
		ING171, 
		IDENTITYPRO,
		DOH,
		PRIDENTOPRO, //35
		PRIDENTOPROXTD,	
		ENDURO_3E,  
		PPC_ID2350E,
		ILINKCARD_IT2600,
		DOPPIE_100, //40
		DOPPIE_200,
		DOPPIE_300,
		ESSENTRASOLO,
		ESSENTRAPLUS,
		ESSENTRAPRO, //45
		PCARD,
		ORPHICARD,
		SCREENCHECK_SC2500,
		SCREENCHECK_SC4500,
		SCREENCHECK_SC6500, //50
        XR160,
		XR260, 
		XR360, 
		MATICA_M300,
		MATICA_M500, //55
		AUTHENTYS,
		AUTHENTYS_PRO,
		AUTHENTYS_PLUS, 
		AUTHENTYS_PRONTO,
		PLASCO_RIOPRO_LE, //60
		PLASCO_ENDURO_LE,
        JKE700C,
		JKE160C, 
		JKE701C,
		PRIDENTO_PLUS_OPEN_DATA, //65
		PRIDENTO_OPEN_DATA,		
		ALPHACARD_PILOT,
		ALPHACARD_COMPASS, 
		ALPHACARD_PRO_100,
		ALPHACARD_PRO_500,//70
		ALPHACARD_PRO_700, 
		PRICECARD_PRO_LITE,
		PRICECARD_PRO_DUO, 
		PRICECARD_PRO_FLEX,
		PHILOS_DTP_MK_1, //75
		PHILOS_DTP_MK_2,
		PHILOS_DTP_MK_3,
		TITAN_T3, 
		TITAN_T5,
		ELLIADEN_CAROLINE, //80
//Helix Printers        
        HELIX,
		PPC_RTP7500W,
		AUTHENTYS_RETRAX, 
		SCREENCHECK_SC7500,
		HELIX_RT, 
//Pro 2 Printers        
        PRO360, //86
		PRO_SELECT_360,
		PRO360_XTD,
        IDM_SECURE_360,
		AUTHENTYS_PRO_360, //90
		ALPHACARD_PRO_700_360,
		PHILOS_DTP_MK_3_360,
		PLASCO_RIOPRO_LE_360,
		SCREENCHECK_SC6500_360,
		TITAN_T5_360, //95
		PPC_ID3350E_360,
		PRIDENTO_PRO_360,
		PRIDENTO_PRO_360_XTD, 
		PRICECARD_PRO_FLEX_360 //99
};              

/*//Rio Printers
#define RIO_TANGO		UICBVAL_Model_1
#define XXL				UICBVAL_Model_2
//AOTA Printers
#define ALTO			UICBVAL_Model_1
#define OPERA			UICBVAL_Model_2
#define TEMPO			UICBVAL_Model_3
#define AVALON			UICBVAL_Model_4
#define DCP100			UICBVAL_Model_5
//Enduro Printers
#define ENDURO			UICBVAL_Model_1
#define MC200			UICBVAL_Model_2
#define PRONTO			UICBVAL_Model_3
#define PRO				UICBVAL_Model_4
#define IDMAKER			UICBVAL_Model_5
#define AUTHENTYS		UICBVAL_Model_6
#define P4500S			UICBVAL_Model_7
#define P2500S			UICBVAL_Model_8
#define PPC_ID2000		UICBVAL_Model_9
#define PPC_ID2100		UICBVAL_Model_10
#define PPC_ID3100		UICBVAL_Model_11
#define IDM_VALUE		UICBVAL_Model_12
#define IDM_ADVANTAGE	UICBVAL_Model_13
#define IDM_SECURE		UICBVAL_Model_14
#define AUTHENTYS_PRO	UICBVAL_Model_15
#define RIO_PRO_XTENDED	UICBVAL_Model_16
#define AISINO_JKE700C	UICBVAL_Model_17
#define FAGOO_P310E		UICBVAL_Model_18
#define ENDURO_PLUS		UICBVAL_Model_19
#define AUTHENTYS_PLUS	UICBVAL_Model_20
#define PPC_ID2300		UICBVAL_Model_21
#define IDM_ADVANT_PLUS UICBVAL_Model_22
#define P4500S_PLUS		UICBVAL_Model_23
#define PPC_ID3300		UICBVAL_Model_24
#define AISINO_JKE160C	UICBVAL_Model_25
#define REWRITE_PRINTER_A UICBVAL_Model_26
#define REWRITE_PRINTER_B UICBVAL_Model_27
*/
#define RIO_OEM(x)          (x->OEM == OEM_RIO)
#define HELIX_OEM(x)        (x->OEM == OEM_HELIX)
#define HELIX_MODEL(x)      		(HELIX_OEM((x)) && (MODEL((x)) == HELIX))
#define HELIX_RT_MODEL(x)      		(HELIX_OEM((x)) && (MODEL((x)) == HELIX_RT))
#define PPC_RTP7500W_MODEL(x)      	(HELIX_OEM((x)) && (MODEL((x)) == PPC_RTP7500W))
#define AUTHENTYS_RETRAX_MODEL(x) 	(HELIX_OEM((x)) && (MODEL((x)) == AUTHENTYS_RETRAX))
#define SCREENCHECK_SC7500_MODEL(x) (HELIX_OEM((x)) && (MODEL((x)) == SCREENCHECK_SC7500))
#define PRO360_OEM(x)       (x->OEM == OEM_PRO360)
#define PRO360_MODEL(x)      		(PRO360_OEM((x)) && (MODEL((x)) == PRO360))
#define PRO360_XTD_MODEL(x)      	(PRO360_OEM((x)) && (MODEL((x)) == PRO360_XTD))
#define PRO360_XTD_TYPE(x)			(  PRO360_XTD_MODEL((x))  \
									|| PRIDENTO_PRO_360_XTD_MODEL((x)) \
									|| PRICECARD_PRO_FLEX_360_MODEL((x)) )					


#define MODEL(x)            (x->Printer)

#define RIO_MODEL(x)		(RIO_OEM((x)) \
							&& (MODEL((x)) == RIO_TANGO) \
                            && (x->TargetPrinter == 1))
#define TANGO_MODEL(x)		(RIO_OEM((x)) \
							&& (MODEL((x)) == RIO_TANGO) \
                            && (x->TargetPrinter == 2))
#define LAMINATOR_MODEL(x)	(RIO_OEM((x)) \
							&& (MODEL((x)) == RIO_TANGO) \
                            && (x->TargetPrinter == 3))
#define XXL_MODEL(x)        (RIO_OEM((x)) && (MODEL((x)) == XXL))

#define AOTA_OEM(x)      (x->OEM == OEM_AOTA)
#define ALTO_MODEL(x)	 (AOTA_OEM((x)) && (MODEL((x)) == ALTO))
#define OPERA_MODEL(x)	 (AOTA_OEM((x)) && (MODEL((x)) == OPERA))
#define TEMPO_MODEL(x)	 (AOTA_OEM((x)) && (MODEL((x)) == TEMPO))
#define AVALON_MODEL(x)	 (AOTA_OEM((x)) && (MODEL((x)) == AVALON))
#define DCP100_MODEL(x)	 (AOTA_OEM((x)) && (MODEL((x)) == DCP100))

#define RIO_OEM(x)           (x->OEM == OEM_RIO)
#define ENDURO_OEM(x)        (x->OEM == OEM_ENDURO)
#define ENDURO_MODEL(x)	     (ENDURO_OEM((x)) && (MODEL((x)) == ENDURO))
#define MC200_MODEL(x)	     (ENDURO_OEM((x)) && (MODEL((x)) == MC200))
#define PRONTO_MODEL(x)	     (ENDURO_OEM((x)) && (MODEL((x)) == PRONTO))
#define RIOPRO_MODEL(x)	     (ENDURO_OEM((x)) && (MODEL((x)) == PRO))
#define IDMAKER_MODEL(x)     (ENDURO_OEM((x)) && (MODEL((x)) == IDMAKER))
#define P4500S_MODEL(x)      (ENDURO_OEM((x)) && (MODEL((x)) == P4500S))
#define P2500S_MODEL(x)      (ENDURO_OEM((x)) && (MODEL((x)) == P2500S))
#define PPC_ID2000_MODEL(x)  (ENDURO_OEM((x)) && (MODEL((x)) == PPC_ID2000))
#define PPC_ID2100_MODEL(x)  (ENDURO_OEM((x)) && (MODEL((x)) == PPC_ID2100))
#define PPC_ID3100_MODEL(x)  (ENDURO_OEM((x)) && (MODEL((x)) == PPC_ID3100))
#define IDM_VALUE_MODEL(x)   (ENDURO_OEM((x)) && (MODEL((x)) == IDM_VALUE))
#define IDM_ADVANTAGE_MODEL(x) (ENDURO_OEM((x)) && (MODEL((x)) == IDM_ADVANTAGE))
#define IDM_SECURE_MODEL(x)  (ENDURO_OEM((x)) && (MODEL((x)) == IDM_SECURE))
#define RIO_PRO_X_MODEL(x)	  (ENDURO_OEM((x)) && (MODEL((x)) == RIO_PRO_XTENDED))
#define FAGOO_P310E_MODEL(x)  (ENDURO_OEM((x)) && (MODEL((x)) == FAGOO_P310E))
#define ENDURO_PLUS_MODEL(x)  (ENDURO_OEM((x)) && (MODEL((x)) == ENDURO_PLUS))
#define PPC_ID2300_MODEL(x)   (ENDURO_OEM((x)) && (MODEL((x)) == PPC_ID2300))
#define P4500S_PLUS_MODEL(x)  (ENDURO_OEM((x)) && (MODEL((x)) == P4500S_PLUS))
#define IDM_ADV_PLUS_MODEL(x) (ENDURO_OEM((x)) && (MODEL((x)) == IDM_ADVANT_PLUS))
#define PPC_ID3300_MODEL(x)   (ENDURO_OEM((x)) && (MODEL((x)) == PPC_ID3300))
#define SECUPRINT_200_MODEL(x)    (ENDURO_OEM((x)) && (MODEL((x)) == SECUPRINT_200))
#define PRIDENTO_PLUS_MODEL(x)    (ENDURO_OEM((x)) && (MODEL((x)) == PRIDENTO_PLUS))
#define PRIDENTO_MODEL(x)    (ENDURO_OEM((x)) && (MODEL((x)) == PRIDENTO))
#define ING171_MODEL(x)	     	(ENDURO_OEM((x)) && (MODEL((x)) == ING171))
#define IDENTITY_PRO_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == IDENTITYPRO))
#define DOH_MODEL(x)	     	(ENDURO_OEM((x)) && (MODEL((x)) == DOH))
#define PRIDENTO_PRO_MODEL(x)    (ENDURO_OEM((x)) && (MODEL((x)) == PRIDENTOPRO))
#define PRIDENTO_PRO_X_MODEL(x)    (ENDURO_OEM((x)) && (MODEL((x)) == PRIDENTOPROXTD))
#define ENDURO_3E_MODEL(x)	     (ENDURO_OEM((x)) && (MODEL((x)) == ENDURO_3E))
#define PPC_ID2350E_MODEL(x)  	(ENDURO_OEM((x)) && (MODEL((x)) == PPC_ID2350E))
#define ILINKCARD_IT2600_MODEL(x)	     (ENDURO_OEM((x)) && (MODEL((x)) == ILINKCARD_IT2600))
#define DOPPIE_100_MODEL(x)	     (ENDURO_OEM((x)) && (MODEL((x)) == DOPPIE_100))
#define DOPPIE_200_MODEL(x)	     (ENDURO_OEM((x)) && (MODEL((x)) == DOPPIE_200))
#define DOPPIE_300_MODEL(x)	     (ENDURO_OEM((x)) && (MODEL((x)) == DOPPIE_300))
#define P_CARD_MODEL(x)				(ENDURO_OEM((x)) && (MODEL((x)) == PCARD))
#define ORPHICARD_MODEL(x)    		(ENDURO_OEM((x)) && (MODEL((x)) == ORPHICARD))
#define SCREENCHECK_SC2500_MODEL(x) (ENDURO_OEM((x)) && (MODEL((x)) == SCREENCHECK_SC2500))
#define SCREENCHECK_SC4500_MODEL(x) (ENDURO_OEM((x)) && (MODEL((x)) == SCREENCHECK_SC4500))
#define SCREENCHECK_SC6500_MODEL(x) (ENDURO_OEM((x)) && (MODEL((x)) == SCREENCHECK_SC6500))
#define XR160_MODEL(x)     			(ENDURO_OEM((x)) && (MODEL((x)) == XR160))
#define XR260_MODEL(x)     			(ENDURO_OEM((x)) && (MODEL((x)) == XR260))
#define XR360_MODEL(x) 				(ENDURO_OEM((x)) && (MODEL((x)) == XR360))
#define MATICA_M300_MODEL(x)     	(ENDURO_OEM((x)) && (MODEL((x)) == MATICA_M300))
#define MATICA_M500_MODEL(x) 		(ENDURO_OEM((x)) && (MODEL((x)) == MATICA_M500))
#define AUTHENTYS_MODEL(x)   		(ENDURO_OEM((x)) && (MODEL((x)) == AUTHENTYS))
#define AUTHENTYS_PRO_MODEL(x) 		(ENDURO_OEM((x)) && (MODEL((x)) == AUTHENTYS_PRO))
#define AUTH_PLUS_MODEL(x)    		(ENDURO_OEM((x)) && (MODEL((x)) == AUTHENTYS_PLUS))
#define AUTHENTYS_PRONTO_MODEL(x)   (ENDURO_OEM((x)) && (MODEL((x)) == AUTHENTYS_PRONTO))
#define PLASCO_ENDURO_LE_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == PLASCO_ENDURO_LE))
#define PLASCO_RIOPRO_LE_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == PLASCO_RIOPRO_LE))
#define AISINO_JKE700C_MODEL(x)     (ENDURO_OEM((x)) && (MODEL((x)) == JKE700C))
#define AISINO_JKE160C_MODEL(x) 	(ENDURO_OEM((x)) && (MODEL((x)) == JKE160C))
#define AISINO_JKE701C_MODEL(x)     (ENDURO_OEM((x)) && (MODEL((x)) == JKE701C))
#define PRIDENTO_PLUS_OPEN_DATA_MODEL(x) (ENDURO_OEM((x)) && (MODEL((x)) == PRIDENTO_PLUS_OPEN_DATA))
#define PRIDENTO_OPEN_DATA_MODEL(x) (ENDURO_OEM((x)) && (MODEL((x)) == PRIDENTO_OPEN_DATA))
#define ALPHACARD_PILOT_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == ALPHACARD_PILOT))	
#define ALPHACARD_COMPASS_MODEL(x) 	(ENDURO_OEM((x)) && (MODEL((x)) == ALPHACARD_COMPASS))
#define ALPHACARD_PRO_100_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == ALPHACARD_PRO_100))
#define ALPHACARD_PRO_500_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == ALPHACARD_PRO_500))
#define ALPHACARD_PRO_700_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == ALPHACARD_PRO_700))
#define PRICECARD_PRO_LITE_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == PRICECARD_PRO_LITE))
#define PRICECARD_PRO_DUO_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == PRICECARD_PRO_DUO))
#define PRICECARD_PRO_FLEX_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == PRICECARD_PRO_FLEX))
#define PHILOS_DTP_MK_1_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == PHILOS_DTP_MK_1))
#define PHILOS_DTP_MK_2_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == PHILOS_DTP_MK_2))
#define PHILOS_DTP_MK_3_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == PHILOS_DTP_MK_3))
#define TITAN_T3_MODEL(x)			(ENDURO_OEM((x)) && (MODEL((x)) == TITAN_T3))
#define TITAN_T5_MODEL(x)			(ENDURO_OEM((x)) && (MODEL((x)) == TITAN_T5))
#define ELLIADEN_CAROLINE_MODEL(x)	(ENDURO_OEM((x)) && (MODEL((x)) == ELLIADEN_CAROLINE))

#define IDM_SECURE_MODEL_360(x)  		(PRO360_OEM((x)) && (MODEL((x)) == IDM_SECURE_360))
#define AUTHENTYS_PRO_MODEL_360(x) 		(PRO360_OEM((x)) && (MODEL((x)) == AUTHENTYS_PRO_360))
#define ALPHACARD_PRO_700_MODEL_360(x) 	(PRO360_OEM((x)) && (MODEL((x)) == ALPHACARD_PRO_700_360))
#define PHILOS_DTP_MK_3_MODEL_360(x) 	(PRO360_OEM((x)) && (MODEL((x)) == PHILOS_DTP_MK_3_360))
#define PLASCO_RIOPRO_LE_MODEL_360(x) 	(PRO360_OEM((x)) && (MODEL((x)) == PLASCO_RIOPRO_LE_360))
#define SCREENCHECK_SC6500_MODEL_360(x) (PRO360_OEM((x)) && (MODEL((x)) == SCREENCHECK_SC6500_360))
#define TITAN_T5_MODEL_360(x) 			(PRO360_OEM((x)) && (MODEL((x)) == TITAN_T5_360))
#define PPC_ID3350E_MODEL_360(x) 		(PRO360_OEM((x)) && (MODEL((x)) == PPC_ID3350E_360))
#define PRIDENTO_PRO_MODEL_360(x) 		(PRO360_OEM((x)) && (MODEL((x)) == PRIDENTO_PRO_360))
#define PRIDENTO_PRO_360_XTD_MODEL(x)  	(PRO360_OEM((x)) && (MODEL((x)) == PRIDENTO_PRO_360_XTD))
#define PRICECARD_PRO_FLEX_360_MODEL(x) (PRO360_OEM((x)) && (MODEL((x)) == PRICECARD_PRO_FLEX_360))

#define ELLIADEN_TYPE(x)	(	ELLIADEN_CAROLINE_MODEL((x))	)
							
#define TITAN_TYPE(x)	(	TITAN_T3_MODEL((x))	\
							|| TITAN_T5_MODEL((x)) 	)

#define PHILOS_TYPE(x)	(	PHILOS_DTP_MK_1_MODEL((x))	\
							|| PHILOS_DTP_MK_2_MODEL((x))	\
							|| PHILOS_DTP_MK_3_MODEL((x)) 	)

#define PRICECARDPRO_TYPE(x)	(  PRICECARD_PRO_LITE_MODEL((x)) \
							|| PRICECARD_PRO_DUO_MODEL((x)) \
							|| PRICECARD_PRO_FLEX_MODEL((x))	)

#define ALPHACARD_TYPE(x)	(  ALPHACARD_PILOT_MODEL((x)) \
							|| ALPHACARD_COMPASS_MODEL((x)) \
							|| ALPHACARD_PRO_100_MODEL((x)) \
							|| ALPHACARD_PRO_500_MODEL((x)) \
							|| ALPHACARD_PRO_700_MODEL((x))	)
							
#define XR_TYPE(x)			(  XR160_MODEL((x)) \
							|| XR260_MODEL((x)) \
							|| XR360_MODEL((x))	)

#define PLASCO_TYPE(x)		(   PLASCO_ENDURO_LE_MODEL((x))	\
							||	PLASCO_RIOPRO_LE_MODEL((x)) )

#define MATICA_TYPE(x)		(  MATICA_M300_MODEL((x))      \
							|| MATICA_M500_MODEL((x)))

#define SCREENCHECK_TYPE(x)	( 	SCREENCHECK_SC2500_MODEL((x))	\
							||	SCREENCHECK_SC4500_MODEL((x))	\
							||	SCREENCHECK_SC6500_MODEL((x))	\
							||	SCREENCHECK_SC7500_MODEL((x)))

#define POLAROID_TYPE(x)	(  P4500S_MODEL((x))      \
							|| P4500S_PLUS_MODEL((x)) \
							|| P2500S_MODEL((x)))
							
#define PPC_TYPE(x)			(  PPC_ID2000_MODEL((x)) \
							|| PPC_ID2100_MODEL((x)) \
							|| PPC_ID3100_MODEL((x)) \
							|| PPC_ID2300_MODEL((x)) \
							|| PPC_ID2350E_MODEL((x)) \
							|| PPC_ID3300_MODEL((x)) \
							|| PPC_RTP7500W_MODEL((x)) )

#define IDVILLE_TYPE(x)		(  IDMAKER_MODEL((x))       \
							|| IDM_VALUE_MODEL((x))     \
							|| IDM_ADVANTAGE_MODEL((x)) \
							|| IDM_SECURE_MODEL((x))   \
							|| IDM_ADV_PLUS_MODEL((x)) )

#define AUTHENTYS_TYPE(x)	(  AUTHENTYS_MODEL((x))     \
							|| AUTHENTYS_PRO_MODEL((x)) \
							|| AUTH_PLUS_MODEL((x)) 	\
							|| AUTHENTYS_RETRAX_MODEL((x)) \
							|| AUTHENTYS_PRONTO_MODEL((x)) )						

#define ENDURO_TYPE(x)		(  ENDURO_MODEL((x))        \
							|| MC200_MODEL((x))         \
							|| IDMAKER_MODEL((x))       \
							|| AUTHENTYS_MODEL((x))     \
							|| P4500S_MODEL((x))        \
							|| PPC_ID2100_MODEL((x))    \
							|| IDM_ADVANTAGE_MODEL((x))  )
							
#define PRONTO_TYPE(x)	    (  PRONTO_MODEL((x))         \
							|| P2500S_MODEL((x))         \
							|| PPC_ID2000_MODEL((x))     \
							|| IDM_VALUE_MODEL((x))	     \
							|| XR160_MODEL((x)) 		 \
							|| PRIDENTO_MODEL((x)) 		 \
							|| DOPPIE_100_MODEL((x))  	 \
							|| SCREENCHECK_SC2500_MODEL((x)) \
							|| AUTHENTYS_PRONTO_MODEL((x)) \
							|| AISINO_JKE700C_MODEL((x)) \
							|| PRIDENTO_OPEN_DATA_MODEL((x)) \
							|| ALPHACARD_PILOT_MODEL((x)) \
							|| ALPHACARD_PRO_100_MODEL((x)) \
							|| PRICECARD_PRO_LITE_MODEL((x))  \
							|| PHILOS_DTP_MK_1_MODEL((x))  )		
							
#define RIOPRO_TYPE(x)		(  RIOPRO_MODEL((x))       		\
							|| PPC_ID3100_MODEL((x))    	\
							|| IDM_SECURE_MODEL((x))    	\
							|| AUTHENTYS_PRO_MODEL((x)) 	\
							|| RIO_PRO_X_MODEL((x))     	\
							|| PRIDENTO_PRO_MODEL((x))     	\
							|| PRIDENTO_PRO_X_MODEL((x))    \
							|| PPC_ID3300_MODEL((x))     	\
							|| XR360_MODEL((x))    			\
							|| IDENTITY_PRO_MODEL((x))  	\
							|| DOPPIE_300_MODEL((x)) 		\
							|| SCREENCHECK_SC6500_MODEL((x)) \
							|| MATICA_M500_MODEL((x)) 		\
							|| AISINO_JKE160C_MODEL((x))    \
							|| PRICECARD_PRO_FLEX_MODEL((x))	\
							|| PLASCO_RIOPRO_LE_MODEL((x))	\
							|| ALPHACARD_PRO_700_MODEL((x))	\
							|| PHILOS_DTP_MK_3_MODEL((x)) 	\
							|| TITAN_T5_MODEL((x))			)		
							
#define XXL_TYPE(x)			(  XXL_MODEL((x)) \
							|| PRIDENTO_PRO_X_MODEL((x)) \
							|| PRICECARD_PRO_FLEX_MODEL((x)) \
							|| RIO_PRO_X_MODEL((x)) )

#define RIOPRO_XTD_TYPE(x)	(  RIO_PRO_X_MODEL((x)) \
							|| PRICECARD_PRO_FLEX_MODEL((x)) \
							|| PRIDENTO_PRO_X_MODEL((x)) )
							
							
#define ENDURO_PLUS_TYPE(x)	(  ENDURO_PLUS_MODEL((x))  			\
							|| AUTH_PLUS_MODEL((x))    			\
							|| P4500S_PLUS_MODEL((x))  			\
							|| PPC_ID2300_MODEL((x))   			\
							|| IDM_ADV_PLUS_MODEL((x)) 			\
							|| FAGOO_P310E_MODEL((x))  			\
							|| SECUPRINT_200_MODEL((x))			\
							|| PRIDENTO_PLUS_MODEL((x)) 		\
							|| DOH_MODEL((x)) 					\
							|| ING171_MODEL((x)) 				\
							|| ENDURO_3E_MODEL((x)) 			\
							|| PPC_ID2350E_MODEL((x)) 			\
							|| ILINKCARD_IT2600_MODEL((x)) 		\
							|| DOPPIE_200_MODEL((x))			\
							|| P_CARD_MODEL((x)) 				\
							|| ORPHICARD_MODEL((x)) 			\
							|| XR260_MODEL((x)) 				\
							|| SCREENCHECK_SC4500_MODEL((x)) 	\
							|| MATICA_M300_MODEL((x)) 			\
							|| AISINO_JKE701C_MODEL((x)) 		\
							|| PLASCO_ENDURO_LE_MODEL((x))		\
							|| PRIDENTO_PLUS_OPEN_DATA_MODEL((x))	\
							|| ALPHACARD_COMPASS_MODEL((x)) \
							|| PRICECARD_PRO_DUO_MODEL((x)) \
							|| ALPHACARD_PRO_500_MODEL((x)) \
							|| PHILOS_DTP_MK_2_MODEL((x))	\
							|| TITAN_T3_MODEL((x))			\
							|| ELLIADEN_CAROLINE_MODEL((x)) )
							
#define PRIDENTO_TYPE(x)	(  PRIDENTO_PRO_X_MODEL((x)) 	\
							|| PRIDENTO_PRO_MODEL((x)) 		\
							|| PRIDENTO_PLUS_MODEL((x)) 	\
							|| PRIDENTO_MODEL((x)) 			\
							|| PRIDENTO_PLUS_OPEN_DATA_MODEL((x)) \
							|| PRIDENTO_OPEN_DATA_MODEL((x))	)

#define SECUPRINTER_TYPE(x) (SECUPRINT_200_MODEL(x))

#define ING171_TYPE(x) 			(ING171_MODEL(x))

#define IDENTITY_PRO_TYPE(x) 	(IDENTITY_PRO_MODEL(x))

#define HELIX_TYPE(x) 		 (	HELIX_MODEL((x))  				\
							 || PPC_RTP7500W_MODEL((x)) 		\
							 || AUTHENTYS_RETRAX_MODEL((x)) 	\
							 || SCREENCHECK_SC7500_MODEL((x)) 	\
							 || HELIX_RT_MODEL((x))  				)
								
#define ILINKCARD_IT2600_TYPE(x) 	(ILINKCARD_IT2600_MODEL(x))

#define DOPPIE_TYPE(x)      (	DOPPIE_100_MODEL((x))	\
							||	DOPPIE_200_MODEL((x))	\
							||	DOPPIE_300_MODEL((x)) )

#define P_CARD_TYPE(x) 	(P_CARD_MODEL(x))

#define ORPHICARD_TYPE(x) 	(ORPHICARD_MODEL(x))
#define IDVILLE_360_TYPE(x)		(  IDM_SECURE_MODEL_360((x)) )

#define AUTHENTYS_360_TYPE(x)	(  AUTHENTYS_PRO_MODEL_360((x)) )
#define ALPHACARD_360_TYPE(x)	(  ALPHACARD_PRO_700_MODEL_360((x)) )
#define PHILOS_360_TYPE(x)	(PHILOS_DTP_MK_3_MODEL_360((x)) )
#define PLASCO_360_TYPE(x)	(PLASCO_RIOPRO_LE_MODEL_360((x)) )
#define TITAN_360_TYPE(x)		(TITAN_T5_MODEL_360((x)) )
#define SCREENCHECK_360_TYPE(x)	(SCREENCHECK_SC6500_MODEL_360((x)) )
#define PPC_360_TYPE(x)			(PPC_ID3350E_MODEL_360((x)) )
#define PRICECARDPRO_360_TYPE(x)	(PRICECARD_PRO_FLEX_360_MODEL((x)) )

#define PRIDENTO_360_TYPE(x)	(PRIDENTO_PRO_MODEL_360((x)) \
								|| PRIDENTO_PRO_360_XTD_MODEL((x)) )

#define PRO360_TYPE(x) 		 (	PRO360_MODEL((x)) \
							||	PRO_SELECT_360_MODEL((x)) \
							||  PRIDENTO_PRO_360_XTD_MODEL((x)) \
							||	PRO360_XTD_MODEL((x)) \
							||  IDM_SECURE_MODEL_360((x)) \
							||  AUTHENTYS_PRO_MODEL_360((x)) \
							||	ALPHACARD_PRO_700_MODEL_360((x)) \
							|| 	PHILOS_DTP_MK_3_MODEL_360((x)) \
							||	PLASCO_RIOPRO_LE_MODEL_360((x))	\
							|| 	TITAN_T5_MODEL_360((x)) \
							||	SCREENCHECK_SC6500_MODEL_360((x)) \
							||	PPC_ID3350E_MODEL_360((x)) \
							|| 	PRIDENTO_PRO_MODEL_360((x))	\
							||  PRICECARD_PRO_FLEX_360_MODEL((x)) )
							

#define PRO360_WIDTH_300DPI	1013 /**< The width of a 300 dpi image, in pixels. */
#define PRO360_HEIGHT_300DPI	642  /**< The height of a 300 dpi image, in pixels. */
#define BUF_SIZE_600DPI (WIDTH_600DPI * HEIGHT_600DPI) /**< The size of a 600 dpi panel buffer, in bytes. */
#define PRO360_BUF_SIZE_300DPI (PRO360_WIDTH_300DPI * PRO360_HEIGHT_300DPI) /**< The size of a 300 dpi panel buffer, in bytes. */

//#############################################################################

#define COLOUR_RED		RGB(255,0,0)
#define COLOUR_GREEN	RGB(0,255,0)
#define COLOUR_BLUE		RGB(0,0,255)
#define COLOUR_CYAN		RGB(0,255,255)
#define COLOUR_MAGENTA	RGB(255,0,255)
#define COLOUR_YELLOW	RGB(255,255,0)
#define COLOUR_WHITE	RGB(255,255,255)
#define COLOUR_BLACK	RGB(0,0,0)

enum {CARDDISPLAY_AREASHOLES, CARDDISPLAY_ERASEAREA};

//#############################################################################

#define SERIAL_SIZE          20
#define ENDURO_MAGNETIC_MASK 0x01
#define ENDURO_DUPLEX_MASK   0x02

#define DEFAULT_RIOPRO_DENSITY 3829
#define MAX_BLACK_AREAS		10

typedef struct tag_ENDURO_STATUS
{
	BOOL  bPrinterConnected;
	DWORD eModel;
	char  sModel[30];
	DWORD ePrintheadType;
	char  sPrinterSerial[SERIAL_SIZE];
	char  sPrintheadSerial[SERIAL_SIZE];
	char  sPCBSerial[SERIAL_SIZE];
	char  sFirmwareVersion[SERIAL_SIZE];

	//These elements were char sPCBVersion[SERIAL_SIZE], which was not used
	DWORD iDummy1;
	DWORD iDummy2;
	DWORD iSmartOffset;
	DWORD iBitFields;
	DWORD iES_Density;

	DWORD iHandFeed;
	DWORD iCardsPrinted;
	DWORD iCardsOnPrinthead;
	DWORD iDyePanelsPrinted;
	DWORD iCleansSinceShipped;
	DWORD iDyePanelsSinceClean;
	DWORD iCardsSinceClean;
	DWORD iCardsBetweenCleans;

	DWORD iPrintHeadPosn;
	DWORD iImageStartPosn;
	DWORD iImageEndPosn;
	DWORD iMajorError;
	DWORD iMinorError;

	char  sTagUID[20];
	DWORD iShotsOnFilm;
	DWORD iShotsUsed;
	char  sDyeFilmType[20];
	DWORD iColourLength;
	DWORD iResinLength;
	DWORD iOvercoatLength;
	DWORD eDyeFlags;
	DWORD iCommandCode;
	DWORD iDOB;
	DWORD eDyeFilmManuf;
	DWORD eDyeFilmProg;
} ENDURO_STATUS, *PENDURO_STATUS;

#define SEM_DEFAULT 0x00000001
#define SEM_PLATEN  0x00000002
#define ERASE_SPEED	0x00000004
#define EJECT_MODE	0x00000008
#define HORZ_EJECT	0x00000010
#define SEM_XLI     0x00000020

//#############################################################################
#define GET_PROPERTY_DEFAULT(x)	(lpmcb->ModelQueryDriverProperty(lpmcb, DPGETDEFAULT((x)), lpdm, NULL))
//#############################################################################
enum tagDRVCALLBACKSTATUS
{
    DCBS_OK,                /* No error. Processing may continue            */
    DCBS_ABORTJOB,          /* Driver requests that job is aborted          */
    DCBS_FATALERROR         /* Fatal error. Terminate the job               */
} DRVCALLBACKSTATUS;


// Used in several functions to know which plane we are processing
#define PLANE_Y    0
#define PLANE_M    1
#define PLANE_C    2
#define PLANE_K    3
#define PLANE_HALO 4

#define PLANE_MAX (PLANE_HALO + 1) // This is used for allocating memory

// Fixed value : we always send out scanlines to the device as this width
#define OUTPUT_DATA_WIDTH_RIO  504  //Rio/Tango 2 Data Width
#define OUTPUT_DATA_WIDTH_ALTO 576  //Alto/Opera/Tempo Data Width
#define OUTPUT_DATA_WIDTH_HELIX    1036 //Optima Data Width
#define OUTPUT_DATA_WIDTH_PRO360    642  //pro2 Data Width

typedef struct tagSPOOLMEMINFO
{
    PBYTE lpBuffer;        ///< plane data
    ULONG  ulDataSize;      ///< how many bytes we copied on lpBuffer
    ULONG  ulColour;        ///< one of PLANE_X
    BOOL   bDataInPlane;    ///< if lpBuffer has any color
}SPOOLMEMINFO;
typedef SPOOLMEMINFO* PSPOOLMEMINFO, * LPSPOOLMEMINFO;


// Track index in int
enum {MAG_TRACK_INDEX0,		//Dummy track for passing commands to the printer in the header
	  MAG_TRACK_INDEX1,
	  MAG_TRACK_INDEX2,
	  MAG_TRACK_INDEX3,
	  MAG_TRACK_INDEX4,
	  MAX_TRACK_NUMBER};

#define MAG_TRACK_NONE   -1
#define FIRST_MAG_TRACK	 MAG_TRACK_INDEX1
#define LAST_MAG_TRACK	 MAG_TRACK_INDEX3

//#define MAG_MAXBUFFERNUM  256  //RJE 22/1/09
#define MAG_MAXBUFFERNUM  750

typedef struct tagMAGTRACKINFO
{
    BOOL  bFoundSS;         // TRUE = Found the start sentinel
    BOOL  bFoundES;         // TRUE = Found the end sentinel
    BOOL  bComplete;        // TRUE = Saved full track data in TrackData
    BOOL  bBufferFull;      // TRUE = TrackData has max. no. of byte characters.
    char  TrackData[MAG_MAXBUFFERNUM]; ///< Recorded magnetic encoding data.
    ULONG ulTrackLength;    // Length from SS to ES in TrackData
    // Members below will be dependent on track number.
    // See MagEncd.C InitializeMagneticEncodeTrackInfo()
    WORD  wStartSentinel;   // Start Sentinel (SS) for the track
    WORD  wEndSentinel;     // End Sentinel (ES) for the track
	BOOL  bSSAutoInsert;    // Auto Insert setting for SS
	BOOL  bESAutoInsert;    // Auto Insert setting for ES
    WORD  wMinCharCode;     // Minimum character code for the track.
    WORD  wMaxCharCode;     // Maxmum character code for the track.
	BOOL  bHexData;         // TRUE = Bits per Char is 4 so do not perform range checking on data as this is performed on output	
    ULONG ulMaxTrackLen;    // Max. no. of characters between SS and ES
	WORD  wBitsPerChar;		// Bits per char defaulting to UI setting and modified if passed in text string
	WORD  wBitsPerInch;		// Bits per inch defaulting to UI setting and modified if passed in text string
	WORD  wCoercivity;		// Coercivity defaulting to UI setting and modified if passed in text string
    BOOL* pValidCharList;   // List of characters to be used between SS and ES
}MAGTRACKINFO;
typedef MAGTRACKINFO* PMAGTRACKINFO, * LPMAGTRACKINFO;

// Index for the buffers to be used detect adjacent colour option
#define BUFFER_PREV  0
#define BUFFER_CURR  1
#define BUFFER_NEXT  2

typedef struct tagPATCH {
   LONG left;
   LONG width;
   LONG bottom;
   LONG height;
} PATCH, * LPPATCH;

typedef struct settings_
{
    int		modelNumber;
	SHORT   Printer;                    //*Current Printer Model 
    float	pageWidth;
    float	pageHeight;

    int		pageType;
    int		focusArea;

    int		overcoatType;

    int		bytesPerScanLine;
    int		bytesPerScanLineStd;
    int     orientation;
//ULTRA stuff    
    
    SHORT       OEM;                        //*Printer Family 1=RIO, 2=ALTO 3=Enduro
    SHORT       TargetPrinter;              //*Rio/Tango target printer model 
    SHORT       LangNo;                     //*Selected language
    //BOOL        bNGUI;
    ENDURO_STATUS es;
    WORD        nPrintHeadPosition;         // retrieve from printer :!
    SHORT       iTotalJobPages;             // populated by pp?
    
    BOOL        bOvercoat;                  // enable overcoat printing
    BOOL        bSecureShield;              // enable SecureShield(TM)
    BOOL        bUltraSecureOverlay;        // enable Overlay Feature
    BOOL        nUltraSecureOverlayImageNo; // selection of secure image to be printed 1-4
    LONG        lHoloKote;                  // HoloKote tile positions to be activated
    WORD        nOvercoatX1;                // bottom left X of Overcoat region
    WORD        nOvercoatY1;                // bottom left Y of Overcoat region
    WORD        nOvercoatX2;                // top right X of Overcoat region
    WORD        nOvercoatY2;                // top right Y of Overcoat region
    
    WORD        Duplex;                     // desired duplex
    WORD        CardSize;                   // selected card size
    BOOL        AppDeterminesOrient;        // does app drive orientation
    BOOL        HandFeed;                   // hand feed card
    BOOL        CardReject;                 // reject faulty cards
    WORD        EjectSide;                  // eject side
	WORD		XXL_ImageType;				// selected image type xxl type models only
	WORD		PaperHeight;				// custom image length
	WORD		XXL_TapeSize;				// selected tape length xxl type models only
    //CARDFRONT
    WORD        CF_ColourFormat;
    BOOL        CF_PrintOvercoat;
    BOOL        CF_HoloKote;
    BOOL        CF_HoloPatch;
    BOOL        CF_Rotate180;
    WORD        CF_CardOrient;
    WORD        CF_BlackOptions_Halftone;
    WORD        CF_BlackOptions_AllBlackAs;
    WORD        CF_BlackOptions_PicturesUseYMConly;
    WORD        CF_BlackOptions_BlackTextUsesKResin;
    WORD        CF_BlackOptions_MonoBitmapsUseKResin;
    WORD        CF_BlackOptions_BlackPolygonsUseKResin;
    WORD        CF_OvercoatOptions_bUserDefined;
    WORD        CF_OvercoatOptions_Holes;
    LONG        CF_FrontArea1_Left;
    LONG        CF_FrontArea1_Width;
    LONG        CF_FrontArea1_Bottom;
    LONG        CF_FrontArea1_Height;
    LONG        CF_FrontArea2_Left;
    LONG        CF_FrontArea2_Width;
    LONG        CF_FrontArea2_Bottom;
    LONG        CF_FrontArea2_Height;
    LONG        CF_FrontHole1_Left;
    LONG        CF_FrontHole1_Width;
    LONG        CF_FrontHole1_Bottom;
    LONG        CF_FrontHole1_Height;
    LONG        CF_FrontHole2_Left;
    LONG        CF_FrontHole2_Width;
    LONG        CF_FrontHole2_Bottom;
    LONG        CF_FrontHole2_Height;
	PATCH		CF_BlackAreas[MAX_BLACK_AREAS];
	WORD		CF_NoBlackAreas;	
    WORD        CF_SecurityOptions_SecurityType;
    WORD        CF_SecurityOptions_UsewithLaminate;
    WORD        CF_SecurityOptions_DisableCustomHoloKoteKey;
    DWORD       CF_SecurityOptions_HoloKoteMap;
    WORD        CF_SecurityOptions_Rotation;
    WORD        CF_SecurityOptions_ColourHole;
    DWORD       CF_SecurityOptions_HoloKotePatch;
    LONG        CF_SecurityOptions_HoloPatchOffset_Left;
    LONG        CF_SecurityOptions_HoloPatchOffset_Width;
    LONG        CF_SecurityOptions_HoloPatchOffset_Bottom;
    LONG        CF_SecurityOptions_HoloPatchOffset_Height;
    SHORT       CF_SecurityOptions_HoloKote_XAdjust;
    SHORT       CF_SecurityOptions_HoloKote_YAdjust;

    //CARDBACK
    WORD        CB_ColourFormat;
    BOOL        CB_PrintOvercoat;
    BOOL        CB_HoloKote;
    BOOL        CB_HoloPatch;
    BOOL        CB_Rotate180;
    WORD        CB_CardOrient;
    WORD        CB_BlackOptions_Halftone;
    WORD        CB_BlackOptions_AllBlackAs;
    WORD        CB_BlackOptions_PicturesUseYMConly;
    WORD        CB_BlackOptions_BlackTextUsesKResin;
    WORD        CB_BlackOptions_MonoBitmapsUseKResin;
    WORD        CB_BlackOptions_BlackPolygonsUseKResin;
    WORD        CB_OvercoatOptions_bUserDefined;
    WORD        CB_OvercoatOptions_Holes;
    LONG        CB_BackArea1_Left;
    LONG        CB_BackArea1_Width;
    LONG        CB_BackArea1_Bottom;
    LONG        CB_BackArea1_Height;
    LONG        CB_BackArea2_Left;
    LONG        CB_BackArea2_Width;
    LONG        CB_BackArea2_Bottom;
    LONG        CB_BackArea2_Height;

    LONG        CB_BackHole1_Left;
    LONG        CB_BackHole1_Width;
    LONG        CB_BackHole1_Bottom;
    LONG        CB_BackHole1_Height;
    LONG        CB_BackHole2_Left;
    LONG        CB_BackHole2_Width;
    LONG        CB_BackHole2_Bottom;
    LONG        CB_BackHole2_Height;
	PATCH		CB_BlackAreas[MAX_BLACK_AREAS];
	WORD		CB_NoBlackAreas;
    WORD        CB_SecurityOptions_SecurityType;
    WORD        CB_SecurityOptions_UsewithLaminate;
    WORD        CB_SecurityOptions_DisableCustomHoloKoteKey;
    DWORD       CB_SecurityOptions_HoloKoteMap;
    WORD        CB_SecurityOptions_Rotation;
    WORD        CB_SecurityOptions_ColourHole;
    DWORD       CB_SecurityOptions_HoloKotePatch;
    SHORT       CB_SecurityOptions_HoloKote_XAdjust;
    SHORT       CB_SecurityOptions_HoloKote_YAdjust;
    //ADVANCED
    WORD        nColourCorrection;      // the selected colour correction
    BOOL        bColourSure;            // colour sure setting 
    WORD        nPrintSpeed;            // print speed
    SHORT       nSharpness;             // sharpness to be applied
    WORD        nDyeFilmOption;         // selected dye film option
//    BOOL        bSplit;                 // split multi page files 
    BOOL        bPauseSpooler;          // pause spooler during a print
    BOOL        bDisableStatusPolling;  // stop polling
    BOOL        bUsePRNSplit;           // use utility to split jobs

    SHORT       nBrightness;            //
    SHORT       nContrast;              //
    SHORT       nSaturation;            //
    SHORT       nRedStrength;           //
    SHORT       nGreenStrength;           //
    SHORT       nBlueStrength;           //
    SHORT       nPrintHeadPower_YMC;        //
    SHORT       nPrintHeadPower_BlackResin; //
    SHORT       nPrintHeadPower_Overcoat;   //
    //
    SHORT       nImagePosition_UpDown;      //15 +/-
    SHORT       nImagePosition_Start;       //50 +/-
    SHORT       nImagePosition_End;         //50 +/-
    //REWRITEABLE CARDS
    BOOL        bEraseBeforePrint;
    LONG        EraseArea_Left;
    LONG        EraseArea_Width;
    LONG        EraseArea_Bottom;
    LONG        EraseArea_Height;
    
    WORD        ErasePower_Start;           //0-100 
    WORD        ErasePower_End;             //0-100 
    WORD        WritePowerAdjustment;       //0-100
    //MAGNETIC ENCODING
    BOOL        bEncodeOnly;           
    BOOL        bPerformVerification;  
    BOOL        bConcatenateMagStrings;
    BOOL        bJIS2Enabled;
    WORD        nTrackOptions;         
    WORD        nCoercivity;           
    WORD        nBitsPerChar_Track1;   
    WORD        nBitsPerChar_Track2;   
    WORD        nBitsPerChar_Track3;   
    WORD        nBitsPerInch_Track1;   
    WORD        nBitsPerInch_Track2;   
    WORD        nBitsPerInch_Track3;   
    //LAMINATING
    BOOL        bLaminateOnly;
    WORD        nLaminateSide;
    WORD        nProfile;
    wchar_t     wProfileDescription[256];
    WORD        nRollerTemperature;
    WORD        nCardSpeed;
    WORD        nPreLaminationDelay;
    WORD        nLaminationLength;
    SHORT       nStartOffset;
    WORD        nEndOffset;
    WORD        nFilmType;
    SHORT       nHLXPrintHeadPowerYMC;        //
    SHORT       nHLXPrintHeadPowerK; //
    SHORT       nHLXPrintHeadPowerOvercoat;   //
	WORD		ISO7810;
	WORD		CFHolokoteSlot;
	WORD		CBHolokoteSlot;
	
} SETTINGS, * PSETTINGS;

typedef struct _DEVDATA
{
    PWSTR           pwstrDocName;       // pointer to document name.

    DWORD           cCopies;            // count of copies.
    
    WORD    epPrinterModel;
    BOOL    epPrinterRemote;    
    SHORT   epLabelX;			/* width of label  */
    SHORT   epLabelY;			/* height of label */
    SHORT   epLabelXmm;         /* width of label in mm */
    SHORT   epLabelYmm;         /* height of label in mm */
    BOOL    bFWDownload;        //set if firmware job.. informs driver to send only data passed into PASSTHROUGH escape
//image control settings

    BOOL        epbCyanLayer;                 //is there a cyan layer to send
    LONG        eplCyanImageDataLen;          //size of the cyan layer data  
    BOOL        epbMagentaLayer;              //is there a magenta layer to send
    LONG        eplMagentaImageDataLen;       //size of the magenta layer data  
    BOOL        epbYellowLayer;               //is there a yellow layer to send
    LONG        eplYellowImageDataLen;        //size of the yellow layer data  
    BOOL        epbBlackLayer;                //is there a black layer to send
    LONG        eplBlackImageDataLen;         //size of the black layer data  
////////     
    //HANDLE          h32bitSurface;                  ///< Handle of the surface for k-resin printing.
    VOID *          pso32bitSurface;               ///< Pointer to the surface for k-resin printing.
    
    BOOL            bPageStarted;                   ///< if we finished initialize information per page
    BOOL            bFrontPage;                     ///< TRUE = Front Page, FALSE = Back Page
    BOOL            bDuplex;                        ///< TRUE = Duplex on and need to output backside info

    SPOOLMEMINFO    lpSplMemFront[4];               ///< Used to spool plane data in a page
    SPOOLMEMINFO    lpSplMemBack[4];                ///< Used to spool plane data in a page
	WORD    		HelixPreview[PREVIEW_HEIGHT][PREVIEW_WIDTH];	///< Used to store helix preview data for card

    ULONG           ulCMYInPlaneWidth;              ///< Plane width
    LPBYTE          lpCMYIn[3];                     ///< Plane pointer used to convert 32-bit RGB to 3-plane 8-bit

    ULONG           ulCMYOutPlaneWidth;             ///< Plane width
    LPBYTE          lpCMYOut[3];                    ///< Plane pointer used to convert 8-bit plane into 6-bit plane

    ULONG           ulKOutPlaneWidth;               ///< 1-bit plane width
    LPBYTE          lpKOut;                         ///< 1-bit plane pointer used to save result of halftoning
//    LPBYTE          lpEDOut;                         ///< 1-bit plane pointer used to save result of halftoning

    //HANDLE          h8bitSrcSurface;                ///< Handle of the MASK surface for k-resin printing matching the orientation of the RGB.
    VOID *          lpKSrcStrip;                    ///< 8-bit surface
    
    //HANDLE          h8bitPSrcSurface;               ///< Handle of the MASK surface for k-resin printing (always portrait).
    LPVOID          lpKPSrcStrip;                   ///< 8-bit surface
    
    //HANDLE          h8bitSurface;                   ///< Handle of the surface for k-resin printing.
    LPVOID          lpKDstStrip;                    ///< 8-bit grayscale strip
	
	RECTL			BlackAreas[MAX_BLACK_AREAS];
	WORD			NoBlackAreas;    
    //HANDLE          h8bitAreasSurface;              ///< Handle of the surface for resultant k-resin areas printing.
    LPVOID          lpKAreasStrip;                  ///< 8-bit grayscale strip

    //HANDLE          hHalftoneSurface;               ///< Halftone object
    LPVOID          lpHalftone;                     ///< surface object ptr

//    ULONG           ulJnlDataIn;                    ///< For recording the journal data type of custom journal records

//    HANDLE          hKresinSurface;                 ///< Handle of the surface for k-resin printing.
    VOID *          psoKResinSurface;               ///< Pointer to the surface for k-resin printing.

    BOOL            bBlackTextUseK;                 ///< TRUE = Black Text uses K-resin option is ON
    BOOL            bBlackPolygonUseK;              ///< TRUE = Black Polygon uses K-resin option is ON
    BOOL            bMonoBitmapUseK;                ///< TRUE = Mono bitmaps use K-resin option is ON
    BOOL            bDetectAdjacentColour;          ///< TRUE = Detect adjacent colour option is ON
    BOOL            bPhotoUseYMC;                   ///< TRUE = Photographs use YMC only option is ON
    BOOL            bExtracted;                     ///< TRUE = mono bitmap extracted
    BOOL            bEncodeOnlyJob;
    MAGTRACKINFO    magTrackInfo[MAX_TRACK_NUMBER]; ///< Magnetic track information
	LONG			LastMagTextY;					///< Last Y coordinate of text initialised to -1 in startpage
    LPBYTE          lpPageBuffer[PLANE_MAX];        ///< Pointers used for page buffers with detect adjacent colour option

    BOOL            bSplitJob;                      ///< Job to be split
	SHORT			iPaperHeight;					///< xxl paperheight	
	int				iHaloRadius;
    
    SHORT           xPrintingOffset;                ///< x offset
    SHORT           yPrintingOffset;                ///< y offset
    SHORT           xImage;                         ///< width of card image
    SHORT           yImage;                         ///< height of card image

/////////////////   
//these might need moving to settings.. 
    WORD            eBlackStartOption;
    WORD            eOrientation;
    WORD            eHalftoning;
    WORD            bImageClipped;
    WORD            iOrientation;
    BOOL            bRotate;
    WORD            eChannelOption;
    WORD            eCallBackStatus;            //error flag    
    
///////////////    
    
    SHORT       epPrinter;                    //*Printer selection

    SHORT       epPaperX;                     //*Paper X 
    SHORT       epPaperY;                     //*Paper Y 
    SHORT       epPaperXdots;                   //*Label width in dotsfor txtout 
    SHORT       epPaperYdots;                   //*Label height in dots for txtout 
    SHORT       epPhysPaperXdots;             //*width in dots for physical size
    SHORT       epPhysPaperYdots;             //*height in dots for physical size
    SHORT       epOffsetX;                    //*Offset X 
    SHORT       epOffsetY;                    //*Offset Y 
	SHORT		lDelta;							//bytes to next line of surface

    SHORT           epXdpi;
    SHORT           epYdpi;

    PBYTE           pSurfaceBitmap;     // own managed storage for surface bitmap
    DWORD           SurfaceBitmapSize;  // own store for size of bitmap

    DWORD           iPageNumber;        // page number of current page.
	DWORD			epjobSize;			// pro360 job size
	DWORD           dwFlags;            // a bunch of flags defined above.
	DWORD			epCheckSum;			// pro360 job checksum
	DWORD           dwEndPDEV;          // end of pdev signature.
} DEVDATA;
typedef DEVDATA *PDEVDATA;


#endif // MAGI_GENERAL

