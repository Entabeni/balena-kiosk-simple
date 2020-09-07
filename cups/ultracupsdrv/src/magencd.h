/***************************************************************************/
/**                                                                       **/
/**                            MAGICARD LTD                               **/
/**                                                                       **/
/***************************************************************************/
/**                                                                       **/
/**  PROJECT      : Magicard Driver                                       **/
/**                                                                       **/
/**  MODULE NAME  : MagEncd.h                                             **/
/**                                                                       **/
/**  SIN No.      : 2076                                                  **/
/**                                                                       **/
/**  COPYRIGHT    : Magicard Ltd				                          **/
/**                                                                       **/
/***************************************************************************/

/*
 * Definitions for magnetic strip encoding tracks
 * Those values are based from ISO 7810, 7811 and 7813
 * Character codes are all in UNICODE
 */

#define CHAR_TILDE           0x7e // Tilde
#define CHAR_COMMA           0x2c // Comma

// Maximum numbers of character (incl. SS and ES)
#define MAX_NUMCHAR_TRACK0      MAG_MAXBUFFERNUM
#define MAX_NUMCHAR_TRACK1      MAG_MAXBUFFERNUM //was 78
#define MAX_NUMCHAR_TRACK2      MAG_MAXBUFFERNUM //was 39
#define MAX_NUMCHAR_TRACK3      MAG_MAXBUFFERNUM //was 106
#define MAX_NUMCHAR_TRACK4      MAG_MAXBUFFERNUM

// ISO sentinels for each track
#define ISO_STARTSENTINEL_TRACK1    0x0025  // Percent sign
#define ISO_STARTSENTINEL_TRACK2    0x003b  // Semi colon
#define ISO_STARTSENTINEL_TRACK3    0x003b  // Semi colon
#define ISO_ENDSENTINEL             0x003f  // Question mark - for all tracks

// JIS sentinels
#define JIS_STARTSENTINEL			0x007f
#define JIS_ENDSENTINEL				0x007f

// Supported character range
#define MIN_CHAR_TRACK0         0x0020  // Space
#define MAX_CHAR_TRACK0         0x007f  // DEL
#define MIN_CHAR_TRACK1         0x0020  // Space
#define MAX_CHAR_ISO_TRACK1     0x005f  // Underscore
#define MAX_CHAR_JIS_TRACK1     0x007f  // DEL
#define MIN_CHAR_TRACK2         0x0030  // Digit 0
#define MAX_CHAR_TRACK2         0x003f  // Question mark
#define MIN_CHAR_TRACK3         0x0030  // Digit 0
#define MAX_CHAR_TRACK3         0x003f  // Question mark

// Track index in character
#define INDEX_CHAR_TRACK0       0x0030  // Digit 0
#define INDEX_CHAR_TRACK1       0x0031  // Digit 1
#define INDEX_CHAR_TRACK2       0x0032  // Digit 2
#define INDEX_CHAR_TRACK3       0x0033  // Digit 3
#define INDEX_CHAR_TRACK4       0x0034  // Digit 4

#define MAG_DENSITY_210         210
#define MAG_DENSITY_75          75

#define MAG_BITPERCHAR_7        7
#define MAG_BITPERCHAR_5        5

#define MAG_COERCIVITY_LOW      0
#define MAG_COERCIVITY_HIGH     1

/*
 * Valid magnetic encoding commands
 *
 * These strings are used in LocateStartSentinelPosition() for finding
 * the correct position for a missing start sentinel.
 *
 * Please note that these should all be uppercase
 */
#define MAX_COMMAND_SIZE 5
#define JIS2_COMMAND "JIS2"

#ifndef FROM_ULTRA
static char ValidCommands[][MAX_COMMAND_SIZE] =
{
	"BPI", "MPC",  "COE", "GEM", "CHP",
	"EJT", "CHIP", JIS2_COMMAND
};

/*
 * Valid Character Table for Tracks 1, 2 and 3
 *
 * The table is defined from MIN_CHAR_TRACKn to MAX_CHAR_TRACKn
 *      1 = If the character can be used between the SS and the ES.
 *      0 = If the character can NOT be used between the SS and the ES.
 *  Please note that the ES itself is defined as 1
 *  Relate the table with MAGTRACKINFO, at InitializeMagneticEncodeTrackInfo()
 */

// Allow all characters to be valid between start and end sentinel (SPR 20092 part 2)

static BOOL ValidCharCodesISO_Track1[] =
{
//  sp  !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

//  0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

//  @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

//  P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1
};

static BOOL ValidCharCodesJIS_Track1[] =
{
//  sp  !   "   #   $   %   &   '   (   )   *   +   ,   -   .   /
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

//  0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

//  @   A   B   C   D   E   F   G   H   I   J   K   L   M   N   O
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

//  P   Q   R   S   T   U   V   W   X   Y   Z   [   \   ]   ^   _
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

//  '   a   b   c   d   e   f   g   h   i   j   k   l   m   n   o
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,

//  p   q   r   s   t   u   v   w   x   y   z   {   |   }   ~   DEL
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1
};
static ULONG LocateStartSentinelPosition
(
    LPMAGTRACKINFO  lpTrackInfo     ///< Pointer to the magnetic track information structure
);
#endif
/*
static BOOL ValidCharCodes_Track2[] =
{
//  0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1
};

static BOOL ValidCharCodes_Track3[] =
{
//  0   1   2   3   4   5   6   7   8   9   :   ;   <   =   >   ?
    1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1
};
*/
void InitializeMagneticEncodeTrackInfo
(
    PDEVDATA pdev, // Pointer to our PDEV
    struct settings_ * settings
);

BOOL ValidMagDataChar
(
    LPMAGTRACKINFO lpTrackInfo, ///< Pointer to the track info
    CHAR          CharIn      ///< Character for mag encoding
);

int RecordMagneticEncodeTrackInfo
(
    PDEVDATA pdev,   // Pointer to our PDEV
    char *    lpStrIn,
    struct settings_ * settings
);



