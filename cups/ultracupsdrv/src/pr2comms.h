/**
 * @file pr2comms.h
 *
 * @brief structures to support the job file format for the Rio Pro 2.
 *
 *
 * Revision information:
 *
 * $HeadURL:  $
 * $LastChangedDate:  $
 * $LastChangedRevision:  $
 * $LastChangedBy:  $
 *
 * @copyright Magicard Ltd.
 *
 ******************************************************************************/

#ifndef PR2COMMS_H
#define PR2COMMS_H

/******************************************************************************/


//#include "error_codes.h"
#include "optima_types.h"

/******************************************************************************/

/**
 * The printer capability flags type.
 *
 * A CAPABILITY_FLAGS value is formed by bitwise or-ing together the flags for
 * individual capabilities.  For example, a mag duplex printer would indicate a
 * CAPABILITY_FLAGS value of (CPF_ENCODE_MAG | CPF_PRINT_DUPLEX).
 */
//typedef uint32 CAPABILITY_FLAGS;
#define CAP_ENCODE_MAG         0x00000001   /**< CAPABILITY_FLAGS value indicating that a magnetic encoding module is fitted. */
#define CAP_PRINT_DUPLEX       0x00000002   /**< CAPABILITY_FLAGS value indicating that duplex printing is enabled. */
#define CAP_ENCODE_CHIP        0x00000010   /**< CAPABILITY_FLAGS value indicating that a chip encoding module is fitted. */
//#define CPF_ENCODE_CONTACTLESS 0x00000004   /**< CAPABILITY_FLAGS value indicating that a contactless encoding module is fitted. */
//#define CPF_LAMINATE           0x00000010   /**< CAPABILITY_FLAGS value indicating that a laminator module is fitted. */
//#define CPF_AUTO_CALIBRATED    0x00000020   /**< CAPABILITY_FLAGS value indicating that the printer has undergone automatic calibration. */
//#define CPF_PRINT_600DPI       0x00000040   /**< CAPABILITY_FLAGS value indicating that the printer has a 600 dpi printhead. */
//#define CPF_FACTORY_MODE       0x00000100   /**< CAPABILITY_FLAGS value indicating that the printer is in factory mode. */
//#define CPF_DEALER_MODE        0x00000200   /**< CAPABILITY_FLAGS value indicating that the printer is in dealer mode. */

#define MAX_VERSION            32           /**< The maximum size of the version field (including the null terminator).*/
#define MAX_MODEL_NAME         32           /**< The maximum size of the model name field (including the null terminator).*/
#define MAX_SERIAL_NUM         32           /**< The maximum size of the serial number field (including the null terminator).*/
#define MAX_MANF_NAME          32           /**< The maximum size of the manufacturer field (including the null terminator).*/

#define MAX_MANF_NAME          32           /**< The maximum size of the manufacturer field (including the null terminator).*/



/**
 * The PR360_RO_IDENTITY payload structure.
 */
typedef struct
{
   uint8  	id_firmware_version[MAX_VERSION];        /**< The printer firmware version. */
   uint8  	id_printer_serial[MAX_SERIAL_NUM];       /**< The printer serial number. */
   uint8  	id_main_board_serial[MAX_SERIAL_NUM];         /**< The main PCB serial number. */
   uint8  	id_printhead_serial[MAX_SERIAL_NUM];     /**< The printhead serial number. */
   uint8  	id_sd_card_serial[MAX_SERIAL_NUM];       /**< The SD card serial number (CID). */
   uint8  	id_mag_encoder_serial[MAX_SERIAL_NUM];   /**< The magnetic encoder serial number. */
   uint8  	id_model_name[MAX_MODEL_NAME];           /**< The printer's full model name. */
   uint32 	printer_dealer_code;                  /**< The printer's dealer code. */
   uint32 	printer_partner_code;                 /**< The printer's partner code. */
   CAPABILITY_FLAGS  id_capability_flags;                     /**< The printer's capabilities. */
   uint32 	firmware_size;                        /**< The current firmware's size, in bytes. */
   uint8  	id_manufacturer[MAX_MANF_NAME];          /**< The manufacturer. */
   uint32 	stats_total_cards;           /**< The total number of cards printed on this printer. */
   uint32 	stats_total_panels;          /**< The total number of panels printed on this printer. */
   uint32 	stats_cards_on_printhead;    /**< The total number of cards printed with the current printhead. */
   uint32 	stats_panels_on_printhead;   /**< The total number of panels printed with the current printhead. */
   uint32 	stats_cards_since_clean;  /**< The number of cards printed since the last clean. */
   uint32 	stats_panels_since_clean; /**< The number of panels printed since the last clean. */
   uint32 	stats_cleaning_cycles_performed;    /**< The number of cleaning cycles performed. */
   uint8  	dye_name[32];              /**< The part number of the currently fitted colour film. */
   uint8  	dye_serial[32];            /**< The serial number of the currently fitted colour film. */
   uint32	dye_ribbon_manufacturer;		/**<The dye ribbon manufacturer code. */

   uint32 	dye_total_prints;          /**< The total prints available with the currently fitted colour film. */
   uint32	dye_prints_remaining;      /**< The number of prints remaining on the currently fitted colour film. */
   uint32 	dye_ymc_panel_length;      /**< The colour panel length in millimetres, or zero if the film has no such panel. */
   uint32 	dye_black1_panel_length;   /**< The first black panel length in millimetres, or zero if the film has no such panel. */
   uint32 	dye_black2_panel_length;   /**< The second black panel length in millimetres, or zero if the film has no such panel. */
   uint32 	dye_overcoat_panel_length; /**< The overcoat panel length in millimetres, or zero if the film has no such panel. */
   uint32   clean_cards_between_prompts; /**< The number of cards since the last clean at which a cleaning prompt is triggered. */
   uint32 	clean_overdue;   			/**< Non-zero if cleaning is overdue, zero otherwise. */

   uint8   operation_hand_feed[32];
   
//   uint32 cards_low;                    /**< The "cards low" indicator. */

   uint32 colour_uv_panel_length;       /**< The UV panel length in millimetres, or zero if the film has no such panel. */
   uint32 colour_dealer_code;           /**< The colour film's dealer code. */
   uint32 colour_partner_code;          /**< The colour film's partner code. */
   uint32 transfer_dealer_code;         /**< The transfer film's dealer code. */
   uint32 transfer_partner_code;        /**< The transfer film's partner code. */
   
    uint32 mac_hi;            /**< Bits 23-0 of this are bits 47--24 of the MAC address.  Bits 31-24 are set to zero. */
   uint32 mac_lo;            /**< Bits 23-0 of this are bits 23--0 of the MAC address.  Bits 31-24 are set to zero. */
   uint32 dhcp_ip;           /**< The IP address obtained from DHCP. */
   uint32 dhcp_gateway;      /**< The default gateway obtained from DHCP. */
   uint32 dhcp_subnet;       /**< The subnet mask obtained from DHCP. */
 

//   int32 laminate_actual_temperature;  /**< The actual temperature of the laminator roller, in degrees Celsius. */

   uint32 communication_status;     /**< Status Word indicating comms status*/
   
} PR360_RO_STATUS;

/******************************************************************************/

/*
 * PR360_RO_ERROR definitions.
 */

/**
 * The PR360_RO_ERROR payload structure.
 *
 * The ERROR_CODE type and valid error codes values are defined in the shared
 * header error_codes.h.
 */
typedef struct
{
   ERROR_CODE  error_code;     /**< The printer's current error code. */
   uint32      error_context;  /**< The error context code */

} PR360_RO_ERROR;

/**
 * The PR360_RO_IDENTITY payload structure.
 */
typedef struct
{
   uint8  	net_ip[16];        	/**< The printers ip address. */
   uint8  	net_gateway[16];    /**< The printer gateway address. */
   uint8  	net_subnet[16];     /**< The printer subnet address. */
   uint8    use_dhcp;			/**< 0 if static address in use. */
   uint32	power_off_timeout;  /**< 5 - 5250000 */
} PR360_RO_TOOLQUERY;


#define STATUS_USB_ACTIVE         0x00000001   /**< Flag is set when the USB comms are active */
#define STATUS_ETH_ACTIVE         0x00000002   /**< Flag is set when Ethernet is active post DHCP IP Address assignment*/
#define STATUS_BUSY               0x00000004   /**< Flag is set when the printer is busy upgrading*/


#if 0
/**
 * The print option flags type.
 *
 * This is formed by bitwise or-ing together individual print options flags.
 * For example, if a job requires black on the front and back of the card,
 * the print option flags value would be (POF_SIDE_1K | POF_SIDE_2K).
 */
typedef uint32 PRINT_OPTION_FLAGS;
#define POF_SIDE_1Y         0x00000001   /**< PRINT_OPTION_FLAGS bit which indicates that yellow data is present for side 1. */
#define POF_SIDE_1M         0x00000002   /**< PRINT_OPTION_FLAGS bit which indicates that magenta data is present for side 1. */
#define POF_SIDE_1C         0x00000004   /**< PRINT_OPTION_FLAGS bit which indicates that cyan data is present for side 1. */
#define POF_SIDE_1K         0x00000008   /**< PRINT_OPTION_FLAGS bit which indicates that black/resin data is present for side 1. */
#define POF_SIDE_1O         0x00000010   /**< PRINT_OPTION_FLAGS bit which indicates that overcoat is active for side 1. */
#define POF_SIDE_1U         0x00000020   /**< PRINT_OPTION_FLAGS bit which indicates that UV data is present for side 1. */
#define POF_SIDE_1_DPI600   0x00000040   /**< PRINT_OPTION_FLAGS bit which indicates that 600 d.p.i. printing is enabled for side 1. */
#define POF_SIDE_1_PREVIEW  0x00000080   /**< PRINT_OPTION_FLAGS bit which indicates that print preview data is present for side 1. */
#define POF_SIDE_2Y         0x00000100   /**< PRINT_OPTION_FLAGS bit which indicates that yellow data is present for side 2. */
#define POF_SIDE_2M         0x00000200   /**< PRINT_OPTION_FLAGS bit which indicates that magenta data is present for side 2. */
#define POF_SIDE_2C         0x00000400   /**< PRINT_OPTION_FLAGS bit which indicates that cyan data is present for side 2. */
#define POF_SIDE_2K         0x00000800   /**< PRINT_OPTION_FLAGS bit which indicates that black/resin data is present for side 2. */
#define POF_SIDE_2O         0x00001000   /**< PRINT_OPTION_FLAGS bit which indicates that overcoat is active for side 2. */
#define POF_SIDE_2U         0x00002000   /**< PRINT_OPTION_FLAGS bit which indicates that UV data is present for side 2. */
#define POF_SIDE_2_DPI600   0x00004000   /**< PRINT_OPTION_FLAGS bit which indicates that 600 d.p.i. printing is enabled for side 2. */
#define POF_SIDE_2_PREVIEW  0x00008000   /**< PRINT_OPTION_FLAGS bit which indicates that print preview data is present for side 2. */
#define POF_MAG_ENCODE      0x00010000   /**< PRINT_OPTION_FLAGS bit which indicates that this job contains magnetic encoding data. */
#define POF_SMART_ENCODE    0x00020000   /**< PRINT_OPTION_FLAGS bit which indicates that this job contains smart encoding data. */
#define POF_FLATTEN_CARD    0x00040000   /**< PRINT_OPTION_FLAGS bit which indicates that card flattening is enabled.  This feature is only available for single-sided print jobs. */
#define POF_SIDE_1_MARKERS  0x00080000   /**< PRINT_OPTION_FLAGS bit which indicates that the print engine should add alignment markers to side 1 of the card. */
#define POF_SIDE_1_ADD_INFO 0x00100000   /**< PRINT_OPTION_FLAGS bit which indicates that the print engine should auto-generate the printer serial number and setup info in the side 1 K panel. */
#define POF_SIDE_2_MARKERS  0x00200000   /**< PRINT_OPTION_FLAGS bit which indicates that the print engine should add alignment markers to side 2 of the card. */
#define POF_SIDE_2_ADD_INFO 0x00400000   /**< PRINT_OPTION_FLAGS bit which indicates that the print engine should auto-generate the printer serial number and setup info in the side 2 K panel. */
#define POF_SIDE_2_SHIFT    8            /**< How much to left-shift a side 1 print option flag to get the equivalent side 2 print option flag. */

/*
 * Useful combinations of flags.
 */

/** All panel flags for side 1. */
#define POF_SIDE_1_MASK (POF_SIDE_1Y | POF_SIDE_1M | POF_SIDE_1C | POF_SIDE_1K | POF_SIDE_1O | POF_SIDE_1U)

/** All panel flags for side 2. */
#define POF_SIDE_2_MASK (POF_SIDE_2Y | POF_SIDE_2M | POF_SIDE_2C | POF_SIDE_2K | POF_SIDE_2O | POF_SIDE_2U)

/**
 * The sides of a card.
 */
typedef enum {
   SIDE_1 = 0,          /**< The front side. */
   SIDE_2 = 1,          /**< The back side. */

   MAX_NUM_SIDES = 2,   /**< The maximum number of sides. */

   SIDE_FRONT = SIDE_1, /**< An alias for SIDE_1. */
   SIDE_BACK = SIDE_2   /**< An alias for SIDE_2. */
} SIDE;

/**
 * The dye-film panels which may be selected in a print job.
 */
typedef enum {
   PANEL_YELLOW,     /**< The yellow panel. */
   PANEL_MAGENTA,    /**< The magenta panel. */
   PANEL_CYAN,       /**< The cyan panel. */
   PANEL_RESIN,      /**< The resin panel. */
   PANEL_OVERCOAT,   /**< The overcoat panel. */
   PANEL_UV,         /**< The fluorescent panel. */

   MAX_NUM_PANELS,   /**< The maximum number of panels. */

   PANEL_SENTINEL = 0xffffffff  /**< Special value used to terminate lists. */
} PANEL;

#define MIN_POWER_ADJUST   (-10000) /**< The minimum permissible printhead power adjustment for a panel. */
#define MAX_POWER_ADJUST   (+10000) /**< The maximum permissible printhead power adjustment for a panel. */


/**
 * The magnetic encoding coercivity type.
 */
typedef uint32 COERCIVITY;
#define COE_HIGH  0  /**< COERCIVITY value indicating high coercivity. */
#define COE_LOW   1  /**< COERCIVITY value indicating low coercivity. */

/**
 * The magnetic encoding direction type.
 */
typedef uint32 MAG_ENCODE_DIR;
#define MAG_ENC_FORWARDS   0  /**< MAG_ENCODE_DIR value to encode forwards. */
#define MAG_ENC_BACKWARDS  1  /**< MAG_ENCODE_DIR value to encode backwards. */

#define MAG_BUFSIZE  112   /**< The size of the track data buffer in the MAG_TRACK structure. */

/**
 * The MAG_TRACK structure.
 */
typedef struct
{
   uint32 num_bits;                /**< The number of bits to encode. A value of zero deactivates the track. */
   uint32 bits_per_inch;           /**< The bit density for this track, in bits per inch. */
   uint8  track_data[MAG_BUFSIZE]; /**< The binary data to be encoded on this track. */

} MAG_TRACK;

#define NUM_MAG_TRACKS 3   /**< The number of MAG_TRACK structures in the MAG structure. */

/**
 * The MAG structure.
 */
typedef struct
{
   COERCIVITY     coercivity;             /**< Specifies the coercivity of the card. */
   uint32         start_position;         /**< The offset (in microns from the card edge) before encoding begins. */
   uint32         encode_length;          /**< The maximum length (in microns) to encode. */
   uint32         verify_attempts;        /**< How many verification attempts to make.  Zero disables verification. */
   MAG_ENCODE_DIR encode_dir;             /**< The encoding direction. */
   MAG_TRACK      tracks[NUM_MAG_TRACKS]; /**< An array of three MAG_TRACK structures, one for each track. */

} MAG;

#define SMART_DATA_SIZE 32768 /**< The number of 32-bit words in the SMART structure's data array. */

/**
 * The SMART structure.
 */
typedef struct
{
   uint32 data[SMART_DATA_SIZE];   /**< Smart encoding data. */

} SMART;

#define WIDTH_600DPI    2072  /**< The width of a 600 dpi image, in pixels. */
#define HEIGHT_600DPI   1328  /**< The height of a 600 dpi image, in pixels. */
#define WIDTH_300DPI    1036  /**< The width of a 300 dpi image, in pixels. */
#define HEIGHT_300DPI    664  /**< The height of a 300 dpi image, in pixels. */
#define PREVIEW_WIDTH    236  /**< The width of a RGB image preview panel. */
#define PREVIEW_HEIGHT   148  /**< The height of a RGB image preview panel. */

#define BUF_SIZE_600DPI (WIDTH_600DPI * HEIGHT_600DPI) /**< The size of a 600 dpi panel buffer, in bytes. */
#define BUF_SIZE_300DPI (WIDTH_300DPI * HEIGHT_300DPI) /**< The size of a 300 dpi panel buffer, in bytes. */
#define PREVIEW_SIZE    (PREVIEW_WIDTH*PREVIEW_HEIGHT*2) /**< The size of a preview panel. Multiplied by 2 as each pixel is a 16 bit RGB value. */

#endif
/*
 * JOB_GET_HOLOKOTE_PREVIEWS definitions.
 */

/* No extra data required beyond the header. */

/** The number of HoloKote slots available. */
#define NUM_HOLOKOTES                  10

/** The length of the unique key associated with a HoloKote watermark. */
#define HOLOKOTE_UNIQUE_KEY_LEN        256

/** The width of a HoloKote preview image, in pixels. */
#define HOLOKOTE_PREVIEW_WIDTH         (WIDTH_300DPI / 4)

/** The height of a HoloKote preview image, in pixels. */
#define HOLOKOTE_PREVIEW_HEIGHT        (HEIGHT_300DPI / 4)

/** The size of a 1 bit per pixel HoloKote thumbnail image, in bytes.  Padded to 32-bit boundary.  */
#define HOLOKOTE_PREVIEW_SIZE_1BPP     ((((HOLOKOTE_PREVIEW_WIDTH * HOLOKOTE_PREVIEW_HEIGHT) + 31) & ~31) / 8)

/** The size of a 16 bits per pixel HoloKote thumbnail image, in bytes. */
#define HOLOKOTE_PREVIEW_SIZE_16BPP    (HOLOKOTE_PREVIEW_WIDTH * HOLOKOTE_PREVIEW_HEIGHT * 2)

#define HOLOKOTE_PREVIEW_SIZE_HEADER     256




#endif
