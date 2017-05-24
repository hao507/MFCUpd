/***************************************************************************//**
    \file          uds-program.h
    \author        huanghai
    \mail          huanghai@auto-link.com
    \version       0
    \date          2017-05-04
    \description   uds service
*******************************************************************************/
#ifndef	__UDS_PROGRAM_H_
#define	__UDS_PROGRAM_H_
/*******************************************************************************
    Include Files
*******************************************************************************/

/*******************************************************************************
    Type Definition
*******************************************************************************/

/* uds programming status */
typedef enum __UDS_UPGRADE_T_
{
	UDS_PROG_NONE = 0,
	UDS_PROG_READ_DIDF186,
    UDS_PROG_EXTENDED_SESSION,
	UDS_PROG_READ_DIDF187,
	UDS_PROG_DTCOFF,
	UDS_PROG_DISABLE_RXTX,
	UDS_PROG_PROGRAM_SESSION,
	UDS_PROG_SA,
	UDS_PROG_FLASH_DRIVER_REQ_DOWLOAD,
	UDS_PROG_FLASH_DRIVER_DOWNLOADING,
	UDS_PROG_FLASH_DRIVER_EXIT_DOWNLOAD,
	UDS_PROG_FLASH_DRIVER_CRC32,
    UDS_PROG_ERASE_MEMORY,
	UDS_PROG_APP_REQ_DOWNLOAD,
    UDS_PROG_APP_DOWNLOADING,
	UDS_PROG_APP_EXIT_DOWNLOAD,
    UDS_PROG_APP_CRC32,
	UDS_PROG_CHECK_DEPENDENCY,
	UDS_PROG_WRITE_DIDF198,
	UDS_PROG_WRITE_DIDF199,
	UDS_PROG_ECU_RESET,
    UDS_PROG_ENUM_MAX
}uds_upgrade_t;


typedef enum __UDS_DOWNLOAD_T_
{
    UDS_DOWN_NONE = 0,
    UDS_DOWN_FLASH_DRIVER,
    UDS_DOWN_DATA,
    UDS_DOWN_INVLID
}uds_down_st;


typedef enum __UDS_UPDATE_T_
{
	UDS_UPGRADE_NONE = 0,
	UDS_UPGRADE_RUNNING,
	UDS_UPGRADE_STOP,
	UDS_UPGRADE_CNT
}uds_upgrade_st;

/* for uds data update/download */
#define UDS_VALID_DATA_FORMAT             (0x00)
#define UDS_VALID_ADDR_LEN_FORMAT         (0x44)
#define UDS_MAX_NUM_BLOCK_LEN             (0x20) /* maxNumberOfBlockLength 128 */

#define UDS_VALID_START_MEM_ADDR          (0x8000400ul)
#define UDS_MAX_MEM_SIZE

#define UDS_LENGTH_FORMAT_ID              (0x40) /* lengthFormatIdentifier  */


#define P_DRIVER_START_ADDR  (0x0ul)
#define P_DRIVER_ENDxx_ADDR  (0x0ul)
#define P_APP_START_ADDR     (0x0ul)
#define P_APP_ENDxx_ADDR     (0x0ul)
#define P_FLASH_SECTOR_SIZE  (128)

/*board_version*/

#define CONF_STAY_IN_BOOT    1


#define APP_START_ID_PAGE   (0xD)
#define APP_START_ID_ADDR   (0x4000u)

#define APP_ENDxx_ID_PAGE   (0xE)
#define APP_ENDxx_ID_ADDR   (0xBFFCu)

#define APP_START_ID        (0x12A00ABCul)
#define APP_ENDxx_ID        (0x12E00BCDul)


/*******************************************************************************
    extern Varaibles
*******************************************************************************/

#endif
/* end of file*/
