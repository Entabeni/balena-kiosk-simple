/**
 * @file error_codes.h
 *
 * @brief Error codes reported to clients by the printer.
 *
 * Revision information:
 *
 * $HeadURL: http://ultrasvn/svn/Magicard/Optima/SharedHeaders/trunk/error_codes.h $
 * $LastChangedDate: 2016-05-04 16:39:04 +0100 (Wed, 04 May 2016) $
 * $LastChangedRevision: 7275 $
 * $LastChangedBy: aaicken $
 *
 * @copyright Magicard Ltd.
 *
 ******************************************************************************/

#ifndef ERROR_CODES_H_
#define ERROR_CODES_H_

/******************************************************************************/

#include "optima_types.h"

/******************************************************************************/

/**
 * The printer error code type.
 */
typedef uint32 ERROR_CODE;

/******************************************************************************/

/*
 * Error codes reported to clients by the printer.
 */

#define ERR_NO_ERROR                         0  /**< Dummy error code indicating that all is well. */

#define ERR_FLASH_WRITE_ERROR               14  /**< Failed to update the contents of flash memory. */
#define ERR_FLASH_READ_ERROR                15  /**< Failed to read the contents of flash memory. */
#define ERR_FPGA_PROGRAMMING_ERROR         104  /**< An FPGA update operation failed. */
#define ERR_FIRMWARE_UPDATE_FAILURE        111  /**< A firmware update operation failed. */
#define ERR_HOLOKOTE_UPDATE_FAILURE        112  /**< A HoloKote update operation failed. */
#define ERR_CAPABILITY_UPDATE_FAILURE      113  /**< A capability update operation failed. */
#define ERR_HOLOKOTE_VERIFICATION_FAILURE  114  /**< A HoloKote file failed verification. */
#define ERR_HOLOKOTE_SLOT_EMPTY            115  /**< The printer tried to read a HoloKote file from an empty HoloKote slot. */
#define ERR_NO_ERROR_DUPLEX_UPGRADE        116  /**< Not an error! Temporary hack to report status. TODO: fix this so driver polls for status. */
#define ERR_NO_ERROR_DUPLEX_DOWNGRADE      117  /**< Not an error! Temporary hack to report status. TODO: fix this so driver polls for status. */
#define ERR_FEATURE_UNAVAILABLE           1024  /**< The requested feature is not available. */
#define ERR_HOPPER_EMPTY                  1025  /**< A card could not be extracted from the hopper. */
#define ERR_CARD_JAM                      1026  /**< The card got stuck in the print engine. */
#define ERR_FLIPPER_JAM                   1027  /**< The flipper could not be rotated. */
#define ERR_INVALID_JOB_OPTION            1028  /**< A parameter in a job was invalid. */
#define ERR_HEATED_ROLLER_TIMEOUT         1029  /**< Timeout while waiting for heated roller to reach target. */
#define ERR_HEATED_ROLLER_CAM             1030  /**< Timeout while opening or closing the heated roller cam. */
#define ERR_PRINT_ROLLER_CAM              1031  /**< Timeout while opening or closing the print roller cam. */
#define ERR_DYE_FILM_PANEL_TIMEOUT        1032  /**< Timeout while trying to wind to next/previous dye panel. */
#define ERR_DYE_FILM_PANEL_UNAVAILABLE    1033  /**< A dye panel was requested that is not provided by the fitted film type. */
#define ERR_TRANSFER_FILM_PANEL_TIMEOUT   1034  /**< Timeout while trying to wind to next/previous transfer panel. */
#define ERR_MAG_HEAD_HOME_TIMEOUT         1035  /**< Timeout while trying to return the mag head to the home position. */
#define ERR_MAG_HEAD_MOVE_TIMEOUT         1036  /**< Timeout while trying to clear the mag head sensor. */
#define ERR_MAG_VERIFY                    1037  /**< The mag encoder failed to correctly read data on a card. */
#define ERR_COVER_OPENED                  1038  /**< The cover was opened during a job. */
#define ERR_TRANSFER_FILM_EXPIRED         1041  /**< The transfer film has run out of prints. */
#define ERR_DYE_FILM_EXPIRED              1042  /**< The dye film has run out of prints. */
#define ERR_BOTH_FILMS_EXPIRED            1043  /**< Both films expired during printing. */
#define ERR_INDEX_LED_CALIBRATION_FAILED  1044  /**< Calibration of the index LED failed. */
#define ERR_DYE_LED_CALIBRATION_FAILED    1045  /**< Calibration of the dye LED failed. */
#define ERR_DYE_FILM_NOT_FOUND            1046  /**< Dye film is needed, but none is fitted. */
#define ERR_TRANSFER_FILM_NOT_FOUND       1047  /**< Transfer film is needed, but none is fitted. */
#define ERR_FILM_FITTED                   1048  /**< Film is fitted when it should not be e.g. during cleaning. */
#define ERR_NO_FILM_FITTED                1049  /**< Neither roll of film was detected. */
#define ERR_PRINTHEAD_TIMEOUT             1050  /**< Timeout while waiting for printhead temperature to reach target. */
#define ERR_SD_READ_ERROR                 1051  /**< Failed to read from the SD card. */
#define ERR_SD_WRITE_ERROR                1052  /**< Failed to write to the SD card. */
#define ERR_INTERNAL_SD_ABSENT            1053  /**< Failed to detect the internal SD card. */
#define ERR_PARTNER_CODE_MISMATCH         1055  /**< Partner code mismatch, i.e. the printer and dongle have different codes. */
#define ERR_PC_DONGLE_CORRUPT             1056  /**< The partner code programming dongle is corrupt. */
#define ERR_FILE_READ_ERROR               1057  /**< Failed to read a file from the SD card. */

#define ERR_CLEANING_OVERDUE              2048  /**< The printer is overdue for a clean. */
#define ERR_CALIBRATION_REQUIRED          2049  /**< The printer needs to be calibrated. */
#define ERR_TOO_HOT_TO_CLEAN              2050  /**< Cleaning was requested, but the heated roller is too hot to allow this. */
#define ERR_TOO_HOT_TO_SOAK               2051  /**< Soak test was requested, but the heated roller is too hot to allow this. */
#define ERR_NO_CLEANING_CARD              2052  /**< No cleaning card provided at the start of a cleaning cycle. */
#define ERR_NOT_A_ROLLER_CLEANING_CARD    2053  /**< A card was provided at the start of a cleaning cycle, but it wasn't a roller cleaning card. */
#define ERR_CLEANING_CARD_JAM             2054  /**< The cleaning card got jammed while being moved around in the print engine. */
#define ERR_CLEANING_CARD_NOT_EJECTED     2055  /**< The cleaning card got jammed while we were trying to eject it. */

#define ERR_MISCELLANEOUS                 8191  /**< An error not covered by the other error codes. */

#define ERR_HEATER_OVERTEMPERATURE        9000  /**< The heated roller is too hot. */
#define ERR_PRINTHEAD_OVERTEMPERATURE     9001  /**< The printhead is too hot. */
#define ERR_HEATER_SENSOR_DISCONNECTED    9002  /**< The heated roller temperature sensor is returning invalid values. */
#define ERR_PRINTHEAD_DISCONNECTED        9003  /**< The printhead temperature sensor is returning invalid values. */

/******************************************************************************/

#endif /* ERROR_CODES_H_ */
