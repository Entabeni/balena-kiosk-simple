/**
 * @file comms.h
 *
 * @brief The job file format for the Optima.
 *
 * This header file is shared between the printer firmware and the client
 * applications which speak to it.  Do not make any changes which will break
 * the printer-client interface, such as altering the order of symbols in
 * enumerated types or the layout of structure definitions.
 *
 * Revision information:
 *
 * $HeadURL: http://ultrasvn/svn/Magicard/Optima/SharedHeaders/trunk/comms.h $
 * $LastChangedDate: 2016-07-19 13:42:17 +0100 (Tue, 19 Jul 2016) $
 * $LastChangedRevision: 7488 $
 * $LastChangedBy: ctamblyn $
 *
 * @copyright Ultra Electronics Ltd.
 *
 ******************************************************************************/

#ifndef COMMS_H
#define COMMS_H

/******************************************************************************/

/**
 * The file format version described by comms.h.
 *
 * Clients should initialise the version field of the HDR_JOB structure to
 * this value.
 *
 * The most significant 16 bits represent the major version of the file format,
 * and the least significant 16 bits the minor version.
 */
#define COMMS_VERSION 0x00060006

/******************************************************************************/

#include "error_codes.h"
#include "optima_types.h"

/******************************************************************************/

/*
 * HDR_JOB definitions.
 */

/**
 * The magic byte sequence which starts a print job.
 */
#define HDR_JOB_MAGIC "0PT1MA00"

/**
 * The job ID type.
 */
typedef DWORD JOB_ID;
#define JOB_GET                         0  /**< JOB_ID for configuration read request and parameters. */
#define JOB_SET                         1  /**< JOB_ID for configuration write command and parameters. */
#define JOB_CLEAN                       2  /**< JOB_ID for printer cleaning cycle command. */
#define JOB_PRINT                       3  /**< JOB_ID for card printing options. */
#define JOB_UPDATE_PRINTER_FIRMWARE     4  /**< JOB_ID for printer firmware update. */
#define JOB_UPDATE_FPGA_BITSTREAM       5  /**< JOB_ID for FPGA bitstream update. */
#define JOB_UPDATE_LAMINATOR_FIRMWARE   6  /**< JOB_ID for laminator firmware update. */
#define JOB_SOAK                        7  /**< JOB_ID for soak test routines. */
#define JOB_HOLOKOTE                    8  /**< JOB_ID for holoKote image update. */
#define JOB_MAG_READ                    9  /**< JOB_ID for mag card read. */
#define JOB_CLEAR_ERROR                10  /**< JOB_ID for clear an error. */
#define JOB_MOVE_CARD                  11  /**< JOB_ID for move the card. */
#define JOB_TEST                       12  /**< JOB_ID for execute an internal test print. */
#define JOB_DIAGNOSTIC                 13  /**< JOB_ID for execute a diagnostic function. */
#define JOB_DEBUG                      14  /**< JOB_ID for reserved for debugging. */
#define JOB_DEMO                       15  /**< JOB_ID for display images on LCD. */
#define JOB_REMOTE_CONTROL             16  /**< JOB_ID for remote Control Interface for the printer. */
#define JOB_UPDATE_ANIMATIONS          17  /**< JOB_ID for update of the animation frames in Flash. */
#define JOB_UPDATE_RESOURCES           18  /**< JOB_ID for update of the GUI resources in Flash. */
#define JOB_GET_ERROR_LOG              19  /**< JOB_ID for reading the error log. */
#define JOB_MAINTENANCE                20  /**< JOB_ID for requesting maintenance mode. */
#define JOB_UPDATE_ZIP_PACKAGE         21  /**< JOB_ID for printer firmware/FPGA/GUI resources zip package update. */
#define JOB_UPDATE_CAPABILITIES        22  /**< JOB_ID for enabling/disabling printer capabilities (e.g. duplex upgrade). */
#define JOB_MOVE_CHIP_CONTACTS         23  /**< JOB_ID for making/breaking the chip reader contacts. */
#define JOB_SET_SMART_CARD_PRESENT     24  /**< JOB_ID for setting/clearing the smart card present signal. */
#define JOB_GET_HOLOKOTE_PREVIEWS      25  /**< JOB_ID for retrieving a HoloKote preview bitmap. */
#define JOB_UPDATE_LOGO                26  /**< JOB_ID for updating the logo image data. */
#define JOB_UPDATE_SPLASH_SCREEN       27  /**< JOB_ID for updating the splash screen image data. */
#define JOB_UPDATE_SAMPLE_PRN          28  /**< JOB_ID for updating the sample print PRN data. */
#define JOB_READ_TRANSFER_TEMP         29  /**< JOB_ID for reading the transfer temperature data. */
#define JOB_UPDATE_WF121_FIRMWARE      30  /**< JOB_ID for WF121 firmware update. */
#define JOB_UPDATE_STRINGS             31  /**< JOB_ID for updating the custom strings. */

#define JOB_RESET                      39  /**< JOB_ID for rebooting the printer. */

/* Internal test jobs */
#define JOB_MOTOR                      8000  /**< JOB_ID for controlling an individual motor. */
#define JOB_MOVE_CAM                   8001  /**< JOB_ID for moving a cam. */
#define JOB_WIND_FILM                  8002  /**< JOB_ID for winding film. */
#define JOB_SET_TARGET_TEMP            8003  /**< DEPRECATED. */
#define JOB_GET_CURRENT_TEMP           8004  /**< JOB_ID for reading current heated roller temperature. */
#define JOB_READ_SENSORS               8005  /**< JOB_ID for reading sensors. */
#define JOB_RUN_METHOD                 8006  /**< JOB_ID for running a method on a print engine component. */
#define JOB_READ_ADCS                  8007  /**< JOB_ID for reading the ADC values. */
#define JOB_SET_CLOCK                  8008  /**< JOB_ID for setting the real time clock module. */
#define JOB_SET_ERROR                  8009  /**< JOB_ID for injecting an artificial error. */

/**
 * The job header structure.
 */
typedef struct
{
   uint8  magic[8];      /**< The sequence of bytes defined by the HDR_JOB_MAGIC macro. */
   uint32 size;          /**< The size of the whole job in bytes, including this header. */
   JOB_ID job_id;        /**< The job type. */
   uint32 job_sub_id;    /**< The job subtype. */
   uint32 version;       /**< The data format version used to assemble this job.  This should be set to COMMS_VERSION by clients.*/
   int32  timestamp;     /**< A 32-bit timestamp indicating when this job file was assembled. */
   uint32 reserved[9];   /**< This is reserved for future use and must be filled with zeroes. */

} HDR_JOB;

/******************************************************************************/

/**
 * The size of the signature, in bytes, appended to binary files.
 */
#define SIGNATURE_SIZE  (128)

/******************************************************************************/

/*
 * JOB_GET definitions.
 */

/**
 * The JOB_GET sub-ID type.
 */
typedef uint32 PARAM_ID;
#define PID_RW_PRINT       0   /**< PARAM_ID value for printing configuration. */
#define PID_RW_TRANSFER    1   /**< PARAM_ID value for transferring configuration. */
#define PID_RO_TRANSFER    2   /**< PARAM_ID value for transferring status. */
#define PID_RO_IDENTITY    3   /**< PARAM_ID value for printer identity. */
#define PID_RO_USAGE       4   /**< PARAM_ID value for usage statistics. */
#define PID_RO_MEDIA       5   /**< PARAM_ID value for media status. */
#define PID_RW_NETWORK     6   /**< PARAM_ID value for network configuration. */
#define PID_RO_NETWORK     7   /**< PARAM_ID value for network information. */
#define PID_RO_ERROR       8   /**< PARAM_ID value for printer error status. */
#define PID_RW_CLEAN       9   /**< PARAM_ID value for cleaning configuration. */
#define PID_RO_CLEAN       10  /**< PARAM_ID value for cleaning information. */
#define PID_RW_LAMINATE    11  /**< PARAM_ID value for laminator configuration. */
#define PID_RO_LAMINATE    12  /**< PARAM_ID value for laminator status. */
#define PID_RW_GUI         13  /**< PARAM_ID value for GUI configuration. */
#define PID_RW_USERS       14  /**< PARAM_ID value for user management data. */
#define PID_RW_SCREEN      15  /**< PARAM_ID value for touchscreen calibration data. */
#define PID_RO_STATUS      16  /**< PARAM_ID value for communications status data. */
#define PID_RO_CHALLENGE   17  /**< PARAM_ID value for challenge data. */
#define PID_RW_POWER       18  /**< PARAM_ID value for power-saving configuration. */
#define PID_RO_AUTH        19  /**< PARAM_ID value for SD card authentication data. */

#define PID_REMOTE_CONTROL 255 /**< PARAM_ID value for a remote control message. */

/* No extra data required beyond the header for JOB_GET. */

/******************************************************************************/

/*
 * HDR_RESPONSE definitions.
 */

/**
 * The request response header structure.
 */
typedef struct
{
   uint32   size;     /**< The size of the response in bytes, including this header. */
   PARAM_ID param_id; /**< The PARAM_ID of the reported parameter group. */

} HDR_RESPONSE;

/******************************************************************************/

/*
 * DAT_RW_PRINT definitions.
 */

/**
 * The DAT_RW_PRINT payload structure.
 */
typedef struct
{
   uint32 printhead_power;          /**< The printhead power level. */
   int32  transverse_offset;        /**< The image transverse offset ("printhead position"), in pixels. */
   uint32 duty_cycle_red;           /**< The duty cycle for the red dye LED, from 0 to 255. */
   uint32 duty_cycle_green;         /**< The duty cycle for the green dye LED, from 0 to 255. */
   uint32 duty_cycle_blue;          /**< The duty cycle for the blue dye LED, from 0 to 255. */
   int32  transverse_adj_y;         /**< Additional transverse offset for yellow, in pixels. */
   int32  transverse_adj_m;         /**< Additional transverse offset for magenta, in pixels. */
   int32  transverse_adj_c;         /**< Additional transverse offset for cyan, in pixels. */
   int32  vertical_adj_y;           /**< Additional transverse offset for yellow, in pixels. */
   int32  vertical_adj_m;           /**< Additional transverse offset for magenta, in pixels. */
   int32  vertical_adj_c;           /**< Additional transverse offset for cyan, in pixels. */

} DAT_RW_PRINT;

/******************************************************************************/

/*
 * DAT_RW_TRANSFER definitions.
 */

/**
 * The DAT_RW_TRANSFER payload structure.
 */
typedef struct
{
   int32  transfer_target_temperature; /**< The target temperature for the heated roller, in degrees Celsius. */
   uint32 transfer_start_offset;       /**< The transfer film start offset for transferring, in microns. */
   uint32 card_speed_1;                /**< The card speed when transferring side 1, in millimetres per second. */
   uint32 transfer_length;             /**< The length of film to transfer to the card, in millimetres. */
   uint32 duty_cycle_index;            /**< The duty cycle for the index LED, from 0 to 255. */
   uint32 card_speed_2;                /**< The card speed when transferring side 2, in millimetres per second. */

} DAT_RW_TRANSFER;

/******************************************************************************/

/*
 * DAT_RO_TRANSFER definitions.
 */

/**
 * The DAT_RO_TRANSFER payload structure.
 */
typedef struct
{
   int32 transfer_actual_temperature;  /**< The actual temperature of the heated roller, in degrees Celsius. */

} DAT_RO_TRANSFER;

/******************************************************************************/

/*
 * DAT_RO_IDENTITY definitions.
 */

/**
 * The printer capability flags type.
 *
 * A CAPABILITY_FLAGS value is formed by bitwise or-ing together the flags for
 * individual capabilities.  For example, a mag duplex printer would indicate a
 * CAPABILITY_FLAGS value of (CPF_ENCODE_MAG | CPF_PRINT_DUPLEX).
 */
typedef uint32 CAPABILITY_FLAGS;
#define CPF_ENCODE_MAG         0x00000001   /**< CAPABILITY_FLAGS value indicating that a magnetic encoding module is fitted. */
#define CPF_ENCODE_CHIP        0x00000002   /**< CAPABILITY_FLAGS value indicating that a chip encoding module is fitted. */
#define CPF_ENCODE_CONTACTLESS 0x00000004   /**< CAPABILITY_FLAGS value indicating that a contactless encoding module is fitted. */
#define CPF_PRINT_DUPLEX       0x00000008   /**< CAPABILITY_FLAGS value indicating that duplex printing is enabled. */
#define CPF_LAMINATE           0x00000010   /**< CAPABILITY_FLAGS value indicating that a laminator module is fitted. */
#define CPF_AUTO_CALIBRATED    0x00000020   /**< CAPABILITY_FLAGS value indicating that the printer has undergone automatic calibration. */
#define CPF_PRINT_600DPI       0x00000040   /**< CAPABILITY_FLAGS value indicating that the printer has a 600 dpi printhead. */
#define CPF_FACTORY_MODE       0x00000100   /**< CAPABILITY_FLAGS value indicating that the printer is in factory mode. */
#define CPF_DEALER_MODE        0x00000200   /**< CAPABILITY_FLAGS value indicating that the printer is in dealer mode. */

#define MAX_VERSION            32           /**< The maximum size of the version field (including the null terminator).*/
#define MAX_MODEL_NAME         32           /**< The maximum size of the model name field (including the null terminator).*/
#define MAX_SERIAL_NUM         32           /**< The maximum size of the serial number field (including the null terminator).*/
#define MAX_MANF_NAME          32           /**< The maximum size of the manufacturer field (including the null terminator).*/

/**
 * The DAT_RO_IDENTITY payload structure.
 */
typedef struct
{
   uint8             firmware_version[MAX_VERSION];      /**< The printer firmware version. */
   uint8             printer_serial[MAX_SERIAL_NUM];     /**< The printer serial number. */
   uint8             board_serial[MAX_SERIAL_NUM];       /**< The main PCB serial number. */
   uint8             printhead_serial[MAX_SERIAL_NUM];   /**< The printhead serial number. */
   uint8             sd_card_serial[MAX_SERIAL_NUM];     /**< The SD card serial number (CID). */
   uint8             mag_encoder_serial[MAX_SERIAL_NUM]; /**< The magnetic encoder serial number. */
   uint8             model_name[MAX_MODEL_NAME];         /**< The printer's full model name. */
   uint32            printer_dealer_code;                /**< The printer's dealer code. */
   uint32            printer_partner_code;               /**< The printer's partner code. */
   CAPABILITY_FLAGS  capability_flags;                   /**< The printer's capabilities. */
   uint32            firmware_size;                      /**< The current firmware's size, in bytes. */
   uint8             manufacturer[MAX_MANF_NAME];        /**< The manufacturer. */

} DAT_RO_IDENTITY;

/******************************************************************************/

/*
 * DAT_RO_USAGE definitions.
 */

/**
 * The DAT_RO_USAGE payload structure.
 */
typedef struct
{
   uint32  total_cards;           /**< The total number of cards printed on this printer. */
   uint32  total_panels;          /**< The total number of panels printed on this printer. */
   uint32  cards_on_printhead;    /**< The total number of cards printed with the current printhead. */
   uint32  panels_on_printhead;   /**< The total number of panels printed with the current printhead. */

} DAT_RO_USAGE;

/******************************************************************************/

/*
 * DAT_RO_MEDIA definitions.
 */

/**
 * The DAT_RO_MEDIA payload structure.
 */
typedef struct
{
   uint32 cards_low;                    /**< The "cards low" indicator. */
   uint8  colour_name[32];              /**< The part number of the currently fitted colour film. */
   uint8  colour_serial[32];            /**< The serial number of the currently fitted colour film. */
   uint32 colour_total_prints;          /**< The total prints available with the currently fitted colour film. */
   uint32 colour_prints_remaining;      /**< The number of prints remaining on the currently fitted colour film. */
   uint8  transfer_name[32];            /**< The part number of the currently fitted transfer film. */
   uint8  transfer_serial[32];          /**< The serial number of the currently fitted transfer film. */
   uint32 transfer_total_prints;        /**< The total prints available with the currently fitted transfer film. */
   uint32 transfer_prints_remaining;    /**< The number of prints remaining on the currently fitted transfer film. */
   uint32 colour_ymc_panel_length;      /**< The colour panel length in millimetres, or zero if the film has no such panel. */
   uint32 colour_black1_panel_length;   /**< The first black panel length in millimetres, or zero if the film has no such panel. */
   uint32 colour_black2_panel_length;   /**< The second black panel length in millimetres, or zero if the film has no such panel. */
   uint32 colour_overcoat_panel_length; /**< The overcoat panel length in millimetres, or zero if the film has no such panel. */
   uint32 colour_uv_panel_length;       /**< The UV panel length in millimetres, or zero if the film has no such panel. */
   uint32 colour_dealer_code;           /**< The colour film's dealer code. */
   uint32 colour_partner_code;          /**< The colour film's partner code. */
   uint32 transfer_dealer_code;         /**< The transfer film's dealer code. */
   uint32 transfer_partner_code;        /**< The transfer film's partner code. */

} DAT_RO_MEDIA;

/******************************************************************************/

/*
 * DAT_RW_NETWORK definitions.
 */

/**
 * The DAT_RW_NETWORK payload structure.
 */
typedef struct
{
   uint32 static_ip;              /**< The IP address (when not using DHCP). */
   uint32 static_gateway;         /**< The default gateway (when not using DHCP). */
   uint32 static_subnet;          /**< The subnet mask (when not using DHCP). */
   uint8  hostname[32];           /**< The local host name. */
   uint32 use_dhcp;               /**< Use DHCP to obtain network settings if non-zero, else use static settings. */
   uint32 syslog_server;          /**< The address of the syslog server. */

} DAT_RW_NETWORK;

/******************************************************************************/

/*
 * DAT_RO_NETWORK definitions.
 */

/**
 * The DAT_RO_NETWORK payload structure.
 */
typedef struct
{
   uint32 mac_hi;            /**< Bits 23-0 of this are bits 47--24 of the MAC address.  Bits 31-24 are set to zero. */
   uint32 mac_lo;            /**< Bits 23-0 of this are bits 23--0 of the MAC address.  Bits 31-24 are set to zero. */
   uint32 dhcp_ip;           /**< The IP address obtained from DHCP. */
   uint32 dhcp_gateway;      /**< The default gateway obtained from DHCP. */
   uint32 dhcp_subnet;       /**< The subnet mask obtained from DHCP. */

} DAT_RO_NETWORK;

/******************************************************************************/

/*
 * DAT_RO_ERROR definitions.
 */

/**
 * The DAT_RO_ERROR payload structure.
 *
 * The ERROR_CODE type and valid error codes values are defined in the shared
 * header error_codes.h.
 */
typedef struct
{
   ERROR_CODE  error_code;     /**< The printer's current error code. */
   uint32      error_context;  /**< The error context code */

} DAT_RO_ERROR;

/******************************************************************************/

/*
 * DAT_RW_CLEAN definitions.
 */

/**
 * The DAT_RW_CLEAN payload structure.
 */
typedef struct
{
   uint32 cards_between_prompts; /**< The number of cards between cleaning prompts. */

} DAT_RW_CLEAN;

/******************************************************************************/

/*
 * DAT_RO_CLEAN definitions.
 */

/**
 * The DAT_RO_CLEAN payload structure.
 */
typedef struct
{
   uint32 cleaning_overdue;   /**< Non-zero if cleaning is overdue, zero otherwise. */
   uint32 cards_since_clean;  /**< The number of cards printed since the last clean. */
   uint32 panels_since_clean; /**< The number of panels printed since the last clean. */
   uint32 cleaning_cycles;    /**< The number of cleaning cycles performed. */

} DAT_RO_CLEAN;

/******************************************************************************/

/*
 * DAT_RW_LAMINATE definitions.
 */

/**
 * The lamination type type.
 */
typedef uint32 LAMINATE_TYPE;
#define LAM_CONTINUOUS  0  /**< LAMINATE_TYPE for continuous laminate film. */
#define LAM_INDEXED     1  /**< LAMINATE_TYPE for indexed laminate film (patch film). */

/**
 * The DAT_RW_LAMINATE payload structure.
 */
typedef struct
{
   int32         laminate_target_temperature;   /**< The target temperature for the laminator roller, in degrees Celsius. */
   LAMINATE_TYPE laminate_type;                 /**< The type of laminate film fitted. */
   uint32        laminate_start_offset;         /**< The laminate film start offset, in microns. */
   uint32        laminate_takeup_speed;         /**< The laminate film take-up speed during lamination, in millimetres per second. */
   uint32        laminate_length;               /**< The lamination length, in millimetres. */
   uint32        card_start_offset;             /**< The card start offset for lamination, in microns. */
   uint32        card_speed;                    /**< The card speed during lamination, in millimetres per second. */

} DAT_RW_LAMINATE;

/******************************************************************************/

/*
 * DAT_RO_LAMINATE definitions.
 */

/**
 * The DAT_RO_LAMINATE payload structure.
 */
typedef struct
{
   int32 laminate_actual_temperature;  /**< The actual temperature of the laminator roller, in degrees Celsius. */

} DAT_RO_LAMINATE;

/******************************************************************************/

/*
 * DAT_RW_GUI definitions.
 */

#define LANG_ID_ENGLISH    0
#define LANG_ID_SPANISH    1
#define LANG_ID_PORTUGUESE 2
#define LANG_ID_FRENCH     3
#define LANG_ID_GERMAN     4
#define LANG_ID_RUSSIAN    5
#define LANG_ID_CHINESE    6
#define LANG_ID_KOREAN     7
#define LANG_ID_ARABIC     8
#define LANG_ID_JAPANESE   9
/**
 * The DAT_RW_GUI payload structure.
 */
typedef struct
{
   uint32 print_preview;              /**< Controls whether the print preview/print status screen will be displayed by the GUI. */
   uint32 secure_mode;                /**< Controls whether this is a standard Magicard printer or operates in Ultra Secure mode. */
   uint32 language;                   /**< The language used on the GUI screens. */
   uint32 run_cfg_wizard;             /**< When the system is shipped out of production, this will be set to true so a config wizard runs when the printer is first started. */
   uint32 demo_mode;                  /**< Allows the GUI to be set in a demo mode. This is useful for trade shows if we want to disable certain features. */

} DAT_RW_GUI;

/******************************************************************************/

/*
 * DAT_RW_ACCESS_RIGHTS definitions.
 */

/**
 * The DAT_RW_ACCESS_RIGHTS payload structure.
 */
typedef struct
{
    uint32 hopper;                /**< User access rights: user allowed to open the hopper. 0 is access denied, non-zero is access allowed.*/
    uint32 stacker;               /**< User access rights: user allowed to open the stacker. 0 is access denied, non-zero is access allowed. */
    uint32 door;                  /**< User access rights: user allowed to open the door. 0 is access denied, non-zero is access allowed. */
    uint32 upgrade;               /**< User access rights: user allowed to upgrade the printer, e.g. firmware upgrade. 0 is access denied, non-zero is access allowed. */
    uint32 print;                 /**< User access rights: user allowed to print. 0 is access denied, non-zero is access allowed. */
} DAT_RW_ACCESS_RIGHTS;

/******************************************************************************/

/*
 * DAT_RW_USER definitions.
 */

#define MAX_USERS 10              /**< Maximum number of users supported on the system. */
#define INVALID_USER 0xFFFFFFFF   /**< Identifies the user account as invalid. */
#define HASH_SIZE 20              /**< Size of a SHA1 hash in bytes (note no null termination) */
#define MAX_FAIL_ATTEMPTS 5       /**< The maximum number of failed login attempts before a user account is locked out. */

/**
 * The DAT_RW_USER payload structure.
 */
typedef struct
{
    uint32               user_id;           /**< The unique user id. */
    uint32               account_locked;    /**< User account has been locked out. 0 is unlocked, non-zero is locked. */
    uint32               fail_count;        /**< Count of the number of consecutive failed login attempts on this account. */
    uint32               role;              /**< The role: admin or operator. 0 is user, non-zero is admin. */
    DAT_RW_ACCESS_RIGHTS rights;            /**< User access rights. */
    uint8                digest[HASH_SIZE]; /**< The SHA1 digest of the user passcode. */
} DAT_RW_USER;

/******************************************************************************/

/*
 * DAT_RW_USERS definitions.
 */

/**
 * The DAT_RW_USERS payload structure.
 */
typedef struct
{
    DAT_RW_USER          user[MAX_USERS]; /**< An array of DAT_RW_USER structures. */
    DAT_RW_ACCESS_RIGHTS default_rights;  /**< The default rights for the system. */
} DAT_RW_USERS;

/******************************************************************************/

/*
 * DAT_RW_SCREEN definitions.
 */

/**
 * The DAT_RW_SCREEN payload structure.
 */
typedef struct
{
   uint16 ul_raw_x;              /**< Upper left raw x co-ordinate, in pixels. */
   uint16 ul_raw_y;              /**< Upper left raw y co-ordinate, in pixels. */
   uint16 ur_raw_x;              /**< Upper right raw x co-ordinate, in pixels. */
   uint16 ur_raw_y;              /**< Upper right raw y co-ordinate, in pixels. */
   uint16 ll_raw_x;              /**< Upper left raw x co-ordinate, in pixels. */
   uint16 ll_raw_y;              /**< Upper left raw y co-ordinate, in pixels. */
   uint16 lr_raw_x;              /**< Upper right raw x co-ordinate, in pixels. */
   uint16 lr_raw_y;              /**< Upper right raw y co-ordinate, in pixels. */
   uint32 clkdiv;                /**< The clock divisor. Used to specify the frequency of the pixel clock. */

} DAT_RW_SCREEN;

/******************************************************************************/

/*
 * DAT_RO_STATUS definitions.
 */

/**
 * The DAT_RO_STATUS payload structure.
 */
typedef struct
{
   uint32 communication_status;     /**< Status Word indicating comms status*/

} DAT_RO_STATUS;

#define STATUS_USB_ACTIVE         0x00000001   /**< Flag is set when the USB comms are active */
#define STATUS_ETH_ACTIVE         0x00000002   /**< Flag is set when Ethernet is active post DHCP IP Address assignment*/
#define STATUS_BUSY               0x00000004   /**< Flag is set when the printer is busy upgrading*/

/******************************************************************************/

/*
 * DAT_RO_CHALLENGE definitions.
 */

#define MAINTENANCE_CHALLENGE_SIZE 20 /**< The number of bytes in the challenge. */

/**
 * The DAT_RO_CHALLENGE payload structure.
 */
typedef struct {
   uint8 challenge[MAINTENANCE_CHALLENGE_SIZE]; /**< The challenge data. */

} DAT_RO_CHALLENGE;

/******************************************************************************/

/*
 * DAT_RW_POWER definitions.
 */

/**
 * The DAT_RW_POWER payload structure.
 */
typedef struct {
   uint32 timeout_low_temperature;  /**< The idle time after which the printer will enter low temperature mode, in seconds. */
   uint32 timeout_standby;          /**< The idle time after which the printer will enter standby mode, in seconds. */
   uint32 timeout_off;              /**< The idle time after which the printer will completely power off, in seconds. */
   uint32 spare[15];                /**< Reserved for future use.  These fields must be set to zero. */

} DAT_RW_POWER;

/******************************************************************************/

/*
 * DAT_RO_AUTH definitions.
 */

/*
 * Payload is just a signature of the following data obtained from the
 * printer's DAT_RO_IDENTITY data.
 */
typedef struct
{
   uint8             printer_serial[MAX_SERIAL_NUM];  /* Pad with zeroes. */
   uint8             sd_card_serial[MAX_SERIAL_NUM];  /* Pad with zeroes. */
   CAPABILITY_FLAGS  capability_flags;                /* The printer's capabilities when shipped. */

} JOB_AUTH_NR_MSG_PART;

/**
 * The DAT_RO_AUTH payload structure.
 */
typedef struct
{
   uint8 signature[SIGNATURE_SIZE];       /**< The signature for the JOB_AUTH_NR_MSG_PART data. */

} DAT_RO_AUTH;

/******************************************************************************/

/*
 * JOB_SET definitions.
 */

/*
 * The job subtype is set to the ID of the parameter group to be updated.  Only
 * the read/write parameters (those with IDs beginning PID_RW_) can be modified;
 * any attempts to modify read-only parameters (those with IDs beginning PID_RO_)
 * are silently ignored by the printer.  The payload is as per the corresponding
 * JOB_GET request. For example, if the job subtype is PID_RW_PRINT, the payload is
 * a DAT_RW_PRINT structure.
 */

/******************************************************************************/

/*
 * JOB_CLEAN definitions.
 */

/* No extra data required beyond the header. */

/******************************************************************************/

/*
 * JOB_PRINT definitions.
 */

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
 * The SIDE_OPTIONS structure.
 */
typedef struct
{
   int32  laminate_target_temperature;  /**< The target temperature for lamination, in degrees Celsius. */
   uint32 fast_print;                   /**< A non-zero value indicates that fast printing is enabled. */
   int32  image_position_adjust_x;      /**< The x-offset to apply to the image, in pixels. */
   int32  image_position_adjust_y;      /**< The y-offset to apply to the image, in pixels. */
   int32  image_size_adjust_x;          /**< The width adjustment to apply to the image, in pixels. */
   int32  image_size_adjust_y;          /**< The height adjustment to apply to the image, in pixels. */
   uint32 holokote_index;               /**< The index of the installed HoloKote image to use. */
   int32  holo_position_adjust_x;       /**< The x-offset to apply to the HoloKote image, in pixels. */
   int32  holo_position_adjust_y;       /**< The y-offset to apply to the HoloKote image, in pixels. */
   int32  holo_size_adjust_x;           /**< The width adjustment to apply to the HoloKote image, in pixels. */
   int32  holo_size_adjust_y;           /**< The height adjustment to apply to the HoloKote image, in pixels. */
   int32  overcoat_cutout1_x;           /**< The x-position of overcoat cutout 1, in pixels. */
   int32  overcoat_cutout1_y;           /**< The y-position of overcoat cutout 1, in pixels. */
   int32  overcoat_cutout1_width;       /**< The width of overcoat cutout 1, in pixels. */
   int32  overcoat_cutout1_height;      /**< The height of overcoat cutout 1, in pixels. */
   int32  overcoat_cutout2_x;           /**< The x-position of overcoat cutout 2, in pixels. */
   int32  overcoat_cutout2_y;           /**< The y-position of overcoat cutout 2, in pixels. */
   int32  overcoat_cutout2_width;       /**< The width of overcoat cutout 2, in pixels. */
   int32  overcoat_cutout2_height;      /**< The height of overcoat cutout 2, in pixels. */
   uint32 holokote_orientation;         /**< The orientation of the HoloKote image, either 0 or 180 degrees. */
   int32  power_adjust[MAX_NUM_PANELS]; /**< The printhead power adjustment for each panel, from MIN_POWER_ADJUST to MAX_POWER_ADJUST. */
   uint32 reserved[25];                 /**< For future use. Must be filled with zeroes. */

} SIDE_OPTIONS;

/**
 * The CARD_OPTIONS structure.
 */
typedef struct
{
   uint32       num_copies;           /**< The number of copies to print. */
   uint8        driver_version[16];   /**< The driver version, as a null-terminated ASCII string. */
   uint8        driver_language[4];   /**< The driver language (NUL-terminated three letter code as per ISO 639-2/T). */
   uint32       rotate_before_print;  /**< A non-zero value causes the printer to rotate the card before printing. */
   uint32       rotate_before_lam;    /**< A non-zero value causes the printer to rotate the card before ejection or lamination. */
   uint32       disable_eject;        /**< A non-zero value disables card ejection after printing is completed. */
   uint32       disable_printing;     /**< A non-zero value causes printing operations to be skipped. */
   uint32       disable_transferring; /**< A non-zero value causes transferring operations to be skipped. */
   uint32       reserved[30];         /**< Reserved for future use. Must be filled with zeroes. */
   SIDE_OPTIONS side1_options;        /**< Options for side 1. */
   SIDE_OPTIONS side2_options;        /**< Options for side 2. */

} CARD_OPTIONS;

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

/******************************************************************************/

/*
 * JOB_UPDATE_PRINTER_FIRMWARE definitions.
 */

/* Payload generated by build tools. */

/******************************************************************************/

/*
 * JOB_UPDATE_FPGA_BITSTREAM definitions.
 */

/* Payload generated by build tools. */

/******************************************************************************/

/*
 * JOB_UPDATE_LAMINATOR_FIRMWARE definitions.
 */

/* To be determined. */

/******************************************************************************/

/*
 * JOB_SOAK definitions.
 */

/**
 * The sub ID of JOB_SOAK selects the type of soak test.
 */
typedef uint32 SOAK_TEST_TYPE;
#define STT_FEED           0  /**< Feed cards from the hopper through the platen and eject them. */
#define STT_FLIP           1  /**< Feed a single card from the hopper and repeatedly flip it. */
#define STT_ENCODER        2  /**< Feed a single card from the hopper and repeatedly feed it into/out of the encoder unit. */
#define STT_PRINTER        3  /**< Calibrate the printer and soak all of the print engine's motors. */
#define STT_CHIP           4  /**< Feed a single card into the encoder and repeatedly open/close the chip contacts. */
#define STT_SOLENOIDS      5  /**< Open/close the locking solenoids repeatedly. */
#define STT_NUM_ROUTINES   6  /**< The number of soak test routines. */

/**
 * The JOB_SOAK_OPTIONS structure.
 *
 * This is the payload for a JOB_SOAK request.
 */
typedef struct
{
   uint32 num_cycles; /**< The number of soak test cycles to perform. */

} JOB_SOAK_OPTIONS;


/******************************************************************************/

/*
 * JOB_HOLOKOTE definitions.
 *
 * The job sub-ID is set to the slot number of the watermark being overwritten.
 *
 * The payload is the data embedded into the ULTRAIDHOLOKOTE extension APP0
 * segment in the JFIF file returned by the HoloKote web service (without the
 * APP0 header).
 */

/******************************************************************************/

/*
 * JOB_MAG_READ definitions.
 *
 * The sub-ID selects the reading direction, and should be either
 * MAG_ENC_FORWARDS or MAG_ENC_BACKWARDS.
 */

/* No extra data required beyond the header. */

/******************************************************************************/

/*
 * JOB_CLEAR_ERROR definitions.
 */

/**
 * The error clearing type.
 */
typedef uint32 ERROR_CLEAR_TYPE;
#define ECL_RETRY    0  /**< ERROR_CLEAR_TYPE to clear the error and retry the failed action. */
#define ECL_CANCEL   1  /**< ERROR_CLEAR_TYPE to clear the error and cancel the failed action. */

/* No extra data required beyond the header. */

/******************************************************************************/

/*
 * JOB_MOVE_CARD definitions.
 */

/**
 * The card position type.
 */
typedef uint32 CARD_POSITION;
#define CPS_TRANSFER_STANDBY  0  /**< CARD_POSITION value to move the card to the transfer standby position. */
#define CPS_HEATED_ROLLER     1  /**< CARD_POSITION value to move the card to the heated roller position. */
#define CPS_FLIP_STANDBY      2  /**< CARD_POSITION value to move the card to the flip standby position. */
#define CPS_ROTATE            3  /**< CARD_POSITION value to rotate the card and return it to the transfer standby position. */
#define CPS_MAG_ENCODER       4  /**< CARD_POSITION value to move the card to the mag encoder. */
#define CPS_CONTACT_ENCODER     5  /**< CARD_POSITION value to move the card to the smart encoder. */
#define CPS_EJECT             6  /**< CARD_POSITION value to eject the card. */
#define CPS_REJECT            7  /**< CARD_POSITION value to reject the card. */
#define CPS_LAMINATOR         8  /**< CARD_POSITION value to give the card to the laminator. */
#define CPS_CONTACTLESS_ENCODER  9  /**< CARD_POSITION value to move the card to the smart encoder. */

/* No extra data required beyond the header. */

/******************************************************************************/

/*
 * JOB_TEST definitions.
 */

/* No extra data required beyond the header. */

/******************************************************************************/

/*
 * JOB_DIAGNOSTIC definitions.
 */

/**
 * The test function type.
 */
typedef uint32 TEST_FUNCTION;
#define TEST_RESET_FACTORY_DEFAULTS 0x1000  /**< TEST_FUNCTION value to reset to factory defaults */
#define TEST_READ_FPGA_VERSION      0x1001  /**< TEST_FUNCTION value to read the FPGA bitstream version */
#define TEST_READ_COMMS_VERSION     0x1002  /**< TEST_FUNCTION value to read COMMS_VERSION */

/* No extra data required beyond the header. */

/******************************************************************************/

/*
 * JOB_DEBUG definitions.
 */

/* No extra data required beyond the header. */

/**
 * The debug response header.
 */
typedef struct
{
   uint32 size;  /**< The size of the response in bytes, including this header. */
   uint32 value; /**< The returned value. */

} DEBUG_RESPONSE;

/******************************************************************************/

/*
 * JOB_DEMO definitions.
 *
 * Data following header is a file, in a format defined by the sub-ID.
 */

/**
 * The image format type.
 */
typedef uint32 DEMO_FORMAT_ID;
#define IMAGE_JPG    1  /**< DEMO_FORMAT_ID value indicating that image data following header is a JPG file */
#define IMAGE_GIF    2  /**< DEMO_FORMAT_ID value indicating that image data following header is a GIF file */
#define IMAGE_PNG    3  /**< DEMO_FORMAT_ID value indicating that image data following header is a PNG file */
#define IMAGE_BMP    4  /**< DEMO_FORMAT_ID value indicating that image data following header is a BMP file */
#define IMAGE_RAW    5  /**< DEMO_FORMAT_ID value indicating that image data following header is 16-bit 480 x 272 RGB565 */

/******************************************************************************/

/*
 * JOB_REMOTE_CONTROL definitions.
 */

/**
 * The DAT_REMOTE_CONTROL payload structure.
 */
typedef struct
{
   uint32         type;           /**< The message type. */
   uint32         extParams[2];   /**< The extension parameters (these are optional and should be set to 0 if not used). */
} DAT_REMOTE_CONTROL;

/******************************************************************************/

/*
 * JOB_UPDATE_ANIMATIONS definitions.
 */

#define ANI_NO_SPLIT    0  /**< Indicates that this is a complete file transferred in a single message. */
#define ANI_SPLIT_START 1  /**< Indicates that this is the first packet in a split transaction. */
#define ANI_SPLIT_NEXT  2  /**< Indicates that this is the next packet in a split transaction and subsequent packets will follow. */
#define ANI_SPLIT_END   3  /**< Indicates that this is the final packet in a split transaction. */

/******************************************************************************/

/*
 * JOB_UPDATE_RESOURCES definitions.
 */

/**
 * The UPDATE_RESOURCES structure.
 */
typedef struct
{
   uint8  region[8]; /**< region identifier; eg "RESOURCE". */
   uint32 length;    /**< length of PBR data (not including signature) to follow, in bytes. */

} UPDATE_RESOURCES;
/*  PBR data of variable length goes here */
/*  64 byte signature goes here */

/******************************************************************************/

/*
 * JOB_GET_ERROR_LOG definitions.
 * Defines structures and constant required to decode the error log
 *
 * Note the time and date values are all stored as BCD
 */

typedef struct
{
  uint32 error_number;   // from errors_codes.h (external errors only)
  uint32 error_instance; // information to enable tracing the error in the code.

} ERROR_PROPERTIES;


typedef struct
{
  uint8 seconds; // 0 - 59
  uint8 minutes; // 0 - 59
  uint8 hours;   // 0 - 23
  uint8 spare;   // unused

} PACKED_TIME;


typedef struct
{
  uint8 day;      // 1 - 31 : day of the month
  uint8 month;    // 1 -12  : January - December.
  uint8 year;     // 0 - 99 : year
  uint8 dotw;     // 0 - 6  : Sunday - Saturday.

} PACKED_DATE;


typedef struct
{
  PACKED_TIME time;     // time info (BCD)
  PACKED_DATE date;     // date info (BCD)

} ERROR_TIME_AND_DATE;


typedef struct
{
  ERROR_PROPERTIES     error_routing_info; // 2*32 bits
  ERROR_TIME_AND_DATE  time_and_date;      // 2*32 bits
  uint32               spare[4];

} ERROR_DATA;


typedef struct
{
  uint32 elements_used; // zero based index into log.
  uint32 head;          // the location of the latest log entry.
  uint32 tail;          // the location of the oldest log entry.
  uint32 version;       // version number of the log protocol.
  uint32 spare[4];      // for future development, no current use.

} CNTRL_BLOCK;


typedef struct
{
  CNTRL_BLOCK  control_block;
  ERROR_DATA   entry[127];  // MAX_FLASH_LOG_ENTRY in "flash_log.h"

} FLASH_LOG;

/******************************************************************************/

/*
 * JOB_MAINTENANCE definitions.
 */

/* Payload is just a signature of the DAT_RO_CHALLENGE obtained from the printer. */

/******************************************************************************/

/*
 * JOB_UPDATE_ZIP_PACKAGE definitions.
 */

/* Payload is a ZIP file containing update binaries and a manifest. */

/******************************************************************************/

/*
 * JOB_UPDATE_CAPABILITIES definitions.
 */

/* Payload is an encrypted update script. */

/******************************************************************************/

/*
 * JOB_MOVE_CHIP_CONTACTS definitions.
 */

/**
 * The chip contact operation type.
 */
typedef uint32 CHIP_CONTACT_OPERATION;
#define CCO_OPEN     0x00000000  /**< CHIP_CONTACT_OPERATION value for opening the contacts. */
#define CCO_CLOSE    0x00000001  /**< CHIP_CONTACT_OPERATION value for closing the contacts. */

/* No extra data required beyond the header. */

/******************************************************************************/

/*
 * JOB_SET_SMART_CARD_PRESENT definitions.
 */

/**
 * The smart card present code type.
 */
typedef uint32 SMART_CARD_PRESENT_CODE;
#define SCP_CARD_PRESENT      0x00000000  /**< SMART_CARD_PRESENT_CODE indicating the card is present. */
#define SCP_CARD_NOT_PRESENT  0x00000001  /**< SMART_CARD_PRESENT_CODE indicating the card is not present. */

/* No extra data required beyond the header. */

/******************************************************************************/

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

/**
 * The HoloKote preview response header.
 */
typedef struct
{
   uint32 size;                  /**< The size of the response in bytes, including this header. */
   uint32 offset[NUM_HOLOKOTES]; /**< The position of each bitmap in the response, relative to the start of the header, or 0 if the bitmap is absent. */
   uint32 spare[32];             /**< Reserved for future use.  Must be filled with zeroes. */

} HKT_PREVIEWS_HDR;

/******************************************************************************/

/*
 * JOB_RESET definitions.
 */

/* No extra data required beyond the header. */

/******************************************************************************/

/*
 * JOB_MOTOR definitions.
 */

/**
 * The motor type.
 */
typedef uint16 TEST_MOTOR;
#define DC_PRINT_ROLLER_CAM         0x0001  /**< TEST_MOTOR value for DC print roller cam motor. */
#define DC_HEATED_ROLLER_CAM        0x0002  /**< TEST_MOTOR value for DC heated roller cam motor. */
#define DC_CARD_FEED                0x0003  /**< TEST_MOTOR value for DC card feed motor. */
#define DC_TRANSFER_REWIND          0x0004  /**< TEST_MOTOR value for DC transfer rewind motor. */
#define DC_DYE_TAKEUP               0x0005  /**< TEST_MOTOR value for DC dye takeup motor. */
#define DC_DYE_REWIND               0x0006  /**< TEST_MOTOR value for DC dye rewind motor. */
#define STEPPER_FLIP_ROTATION       0x0007  /**< TEST_MOTOR value for STEPPER flip rotation motor. */
#define STEPPER_FLIP_POSITION       0x0008  /**< TEST_MOTOR value for STEPPER flip position motor. */
#define STEPPER_TRANSFER_TAKEUP     0x0009  /**< TEST_MOTOR value for STEPPER transfer takeup motor. */
#define STEPPER_CARD_DRIVE          0x000a  /**< TEST_MOTOR value for STEPPER card drive motor. */

/**
 * The motor action.
 */
typedef uint16 MOTOR_ACTION;
#define MOTOR_STOP                  0x0000
#define MOTOR_RUN_CLOCKWISE         0x0001
#define MOTOR_RUN_ANTICLOCKWISE     0x0002

/* No extra data required beyond the header. */

/******************************************************************************/

/*
 * JOB_MOVE_CAM definitions.
 * Cam motor definitions as above.
 */

/**
 * The action to perform with the cam.
 */
typedef uint16 CAM_ACTION;
#define CAM_INIT   0
#define CAM_CLOSE  1

/* No extra data required beyond the header. */

/******************************************************************************/

/*
 * JOB_WIND_FILM definitions.
 */

/**
 * The type of film to test.
 */
typedef uint16 TEST_FILM;
#define RT_FILM      0
#define COL_FILM     1

/**
 * The film test operation to perform.
 */
typedef uint16 FILM_WIND;
#define PANEL_PREVIOUS   0
#define PANEL_NEXT       1
#define PANEL_INIT       2

/* No extra data required beyond the header. */

/******************************************************************************/

/*
 * JOB_GET_CURRENT_TEMP definitions.
 * Defines results structure returned by printer.
 */

/**
 * The CURRENT_TEMPERATURE structure.
 */
typedef struct
{
  float current_heated_roller_temperature;   /**< The current heated roller temperature, in degrees Celsius. */
  float current_printhead_temperature;       /**< The current printhead temperature, in degrees Celsius. */

} CURRENT_TEMPERATURES;

/******************************************************************************/

/*
 * JOB_READ_SENSORS definitions.
 * Defines results structure returned by printer.
 */

#define REQUEST_OPTO_SENSORS  0    // use as job_sub_id; others may be added

typedef uint32 OPTO_SENSOR_STATE;
#define SENSOR_CLEAR     0
#define SENSOR_OBSCURED  1

/**
 * The OPTO_SENSOR_STATUS structure returned by the printer.
 */
typedef struct
{
  OPTO_SENSOR_STATE  sensor_print_position;     /**< Non-zero if blocked, zero if unblocked. */
  OPTO_SENSOR_STATE  sensor_lid;                /**< Non-zero if blocked, zero if unblocked. */
  OPTO_SENSOR_STATE  sensor_feed;               /**< Non-zero if blocked, zero if unblocked. */
  OPTO_SENSOR_STATE  sensor_eject;              /**< Non-zero if blocked, zero if unblocked. */
  OPTO_SENSOR_STATE  sensor_flipper;            /**< Non-zero if blocked, zero if unblocked. */
  OPTO_SENSOR_STATE  sensor_encoder_card;       /**< Non-zero if blocked, zero if unblocked. */
  OPTO_SENSOR_STATE  sensor_encoder_head;       /**< Non-zero if blocked, zero if unblocked. */
  OPTO_SENSOR_STATE  sensor_heated_roller_cam;  /**< Non-zero if blocked, zero if unblocked. */
  OPTO_SENSOR_STATE  sensor_print_roller_cam;   /**< Non-zero if blocked, zero if unblocked. */

} OPTO_SENSOR_STATUS;

/******************************************************************************/

/*
 * JOB_RUN_METHOD definitions.
 */

/**
 * The RUN_METHOD_OPTIONS structure.
 *
 * This is the payload for a JOB_RUN_METHOD request.
 */
typedef struct
{
   char   component[64]; /**< A null-terminated string naming the component whose method will be called. */
   char   method[64];    /**< A null-terminated string naming the method to be called. */
   uint32 param;         /**< The parameter to pass to the method (if one is required). Range 0 to 65535. */

} RUN_METHOD_OPTIONS;

/**
 * The RUN_METHOD_RESPONSE structure returned by the printer.
 */
typedef struct
{
   uint32 retval; /**< The value returned from the method. */

} RUN_METHOD_RESPONSE;

/******************************************************************************/

/*
 * JOB_READ_ADCS definitions.
 * Defines results structure returned by printer.
 */

typedef int32 ADC_VOLTAGE;

/**
 * The ADC_READINGS structure returned by the printer.
 */
typedef struct
{
  ADC_VOLTAGE  adc_channel_0;    /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_1;    /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_2;    /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_3;    /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_4;    /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_5;    /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_6;    /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_7;    /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_8;    /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_9;    /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_10;   /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_11;   /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_12;   /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_13;   /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_14;   /**< The ADC channel reading, from 0 to 255. */
  ADC_VOLTAGE  adc_channel_15;   /**< The ADC channel reading, from 0 to 255. */

} ADC_READINGS;

/******************************************************************************/

/*
 * JOB_SET_CLOCK definitions.
 * Defines structure sent to printer.
   This mirrors the internal registers of the RTC module.
 */

typedef struct
{
  uint32  second;
  uint32  minute;
  uint32  hour;
  uint32  day;
  uint32  month;
  uint32  year;
  uint32  dotw;

} RTC_REGS;

/******************************************************************************/

/*
 * JOB_SET_ERROR definitions.
 * Defines structure sent to printer.
 */

typedef struct
{
   ERROR_CODE  error_code;     /**< The error code to set. */
   uint32      error_context;  /**< The error context to set. */

} DAT_SET_ERROR;

/******************************************************************************/

/*
 * Convenience structures.
 */

/* Complete JOB_GET response structures */

#ifdef __DEF_GET_RESPONSE__
#error __DEF_GET_RESPONSE__ already defined!
#endif

#define __DEF_GET_RESPONSE__(type)     \
   typedef struct                      \
   {                                   \
      HDR_RESPONSE  hdr_response;      \
      DAT_##type    payload;           \
   }                                   \
   type##_RESPONSE;

__DEF_GET_RESPONSE__(RW_PRINT)       /* RW_PRINT_RESPONSE */
__DEF_GET_RESPONSE__(RW_TRANSFER)    /* RW_TRANSFER_RESPONSE */
__DEF_GET_RESPONSE__(RO_TRANSFER)    /* RO_TRANSFER_RESPONSE */
__DEF_GET_RESPONSE__(RO_IDENTITY)    /* RO_IDENTITY_RESPONSE */
__DEF_GET_RESPONSE__(RO_USAGE)       /* RO_USAGE_RESPONSE */
__DEF_GET_RESPONSE__(RO_MEDIA)       /* RO_MEDIA_RESPONSE */
__DEF_GET_RESPONSE__(RW_NETWORK)     /* RW_NETWORK_RESPONSE */
__DEF_GET_RESPONSE__(RO_NETWORK)     /* RO_NETWORK_RESPONSE */
__DEF_GET_RESPONSE__(RO_ERROR)       /* RO_ERROR_RESPONSE */
__DEF_GET_RESPONSE__(RW_CLEAN)       /* RW_CLEAN_RESPONSE */
__DEF_GET_RESPONSE__(RO_CLEAN)       /* RO_CLEAN_RESPONSE */
__DEF_GET_RESPONSE__(RW_LAMINATE)    /* RW_LAMINATE_RESPONSE */
__DEF_GET_RESPONSE__(RO_LAMINATE)    /* RO_LAMINATE_RESPONSE */

#undef __DEF_GET_RESPONSE__

/**
 * A complete JOB_MAG_READ response structure.
 */
typedef struct
{
   HDR_RESPONSE hdr_response; /**< The MAG_READ_RESPONSE header. */
   MAG_TRACK    tracks[3];    /**< The MAG_READ_RESPONSE payload. */

} MAG_READ_RESPONSE;

/******************************************************************************/

#endif
