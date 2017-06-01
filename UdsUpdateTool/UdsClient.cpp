/***************************************************************************//**
\file          UdsClient.c
\author        huanghai
\mail          huanghai@auto-link.com
\version       0.01
\date          2016-10-24
\description   uds client code, request service throngh can and deal the response
*******************************************************************************/
#include "stdafx.h"
#include "UdsUtil.h"
#include "UdsClient.h"
#include "ControlCAN.h"
#include "UdsUpdateTool.h"
#include "UdsUpdateToolDlg.h"
#include "timeapi.h"

#pragma comment( lib,"winmm.lib" )

/*******************************************************************************
Type declaration
*******************************************************************************/
typedef struct __UDS_CLIENT_T__
{
	BYTE uds_sid;
	BYTE uds_min_len;
	void(*uds_client)  (BYTE *, WORD);
	uds_sa_lv uds_sa; /* security access */
}uds_client_t;

/*******************************************************************************
Function  declaration
*******************************************************************************/


/*******************************************************************************
Private Varaibles
*******************************************************************************/

/*******************************************************************************
Class CUdsClient
*******************************************************************************/
CUdsClient::CUdsClient() : CUdsNetwork()
{
	n_Result = 0;
	n_ResultErr = FALSE;
	m_RspNrc = NRC_NONE;
	m_FunReq = FALSE;
	m_ReqSsp = 0;

	pdri_buf = NULL;
	papp_buf = NULL;

	m_EntBoot = 0;
}

CUdsClient::~CUdsClient()
{
}


/**
* uds_timer_start - start uds timer
*
* void :
*
* returns:
*     void
*/
void CUdsClient::uds_timer_start(BYTE num)
{
	if (num >= UDS_TIMER_CNT) return;

	if (num == UDS_TIMER_FSA) {
		uds_timer[UDS_TIMER_FSA] = timeGetTime();
		TIMOUT_VALUE[UDS_TIMER_FSA] = TIMEOUT_FSA;
		uds_timeo[UDS_TIMER_FSA] = 0;
	}
	if (num == UDS_TIMER_S3client) {
		uds_timer[UDS_TIMER_S3client] = timeGetTime();
		TIMOUT_VALUE[UDS_TIMER_S3client] = TIMEOUT_S3client;
		uds_timeo[UDS_TIMER_S3client] = 0;
	}
	if (num == UDS_TIMER_P2client) {
		uds_timer[UDS_TIMER_P2client] = timeGetTime();
		TIMOUT_VALUE[UDS_TIMER_P2client] = TIMEOUT_P2client;
		uds_timeo[UDS_TIMER_P2client] = 0;
	}
	if (num == UDS_TIMER_P2client_x) {
		uds_timer[UDS_TIMER_P2client_x] = timeGetTime();
		TIMOUT_VALUE[UDS_TIMER_P2client_x] = TIMEOUT_P2client_x;
		uds_timeo[UDS_TIMER_P2client_x] = 0;
	}
}

void CUdsClient::uds_timer_stop(BYTE num)
{
	if (num >= UDS_TIMER_CNT) return;

	uds_timer[num] = 0;
}

void CUdsClient::uds_timer_clear(BYTE num)
{
	if (num >= UDS_TIMER_CNT) return;

	uds_timeo[num] = 0;
}
/**
* uds_timer_run - run a uds timer, should be invoked per 1ms
*
* void :
*
* returns:
*     0 - timer is not running, 1 - timer is running, -1 - a timeout occur
*/
int CUdsClient::uds_timer_run(BYTE num)
{
	if (num >= UDS_TIMER_CNT) return 0;

	if (uds_timer[num] == 0)
	{
		return 0;
	}
	else
	{
		DWORD Tikcs = timeGetTime();

		if (Tikcs - uds_timer[num] >= TIMOUT_VALUE[num])
		{
			uds_timer[num] = 0;
			uds_timeo[num] = 1;
			return -1;
		}
		else
		{
			return 1;
		}
	}

}

/**
* uds_timer_chk - check a uds timer and stop it
*
* num :
*
* returns:
*     0 - timer is not running, 1 - timer is running,
*/
INT CUdsClient::uds_timer_chk(BYTE num)
{
	if (num >= UDS_TIMER_CNT) return 0;

	if (uds_timeo[num] == 1)
		return -1;
	if (uds_timer[num] > 0)
		return 1;
	else
		return 0;
}


/**
* N_USData_ffindication - uds first frame indication callbacl
*
* @msg_dlc : first frame dlc
*
* returns:
*     void
*/
void CUdsClient::N_USData_ffindication(WORD msg_dlc)
{
	if (uds_timer_chk(UDS_TIMER_P2client) == -1)   /* P2client is timeout */
		return;
	uds_timer_stop(UDS_TIMER_P2client);

	if (uds_timer_chk(UDS_TIMER_P2client_x) == -1) /* P2*client is timeout */
		return;
	uds_timer_stop(UDS_TIMER_P2client_x);
}

/**
* N_USData_indication - uds data request indication callback,
*
* @msg_buf  :
* @msg_dlc  :
* @n_result :
*
* returns:
*     void
*/

void CUdsClient::N_USData_indication(BYTE msg_buf[], WORD msg_dlc, n_result_t n_result)
{
	if (uds_timer_chk(UDS_TIMER_P2client) == -1)   /* P2client is timeout */
		return;
	uds_timer_stop(UDS_TIMER_P2client);

	if (uds_timer_chk(UDS_TIMER_P2client_x) == -1) /* P2*client is timeout */
		return;
	uds_timer_stop(UDS_TIMER_P2client_x);

	if (n_result != N_OK)
	{
		n_ResultErr = TRUE;
		n_Result = n_result;
		return;
	}

	if (msg_buf[0] == NEGATIVE_RSP)
	{
		m_RspSid = msg_buf[1];
		m_RspNrc = msg_buf[2];
		if (m_RspNrc == NRC_SERVICE_BUSY)
			uds_timer_start(UDS_TIMER_P2client_x);
	}
	else
	{
		m_RspSid = USD_GET_POSITIVE_RSP(msg_buf[0]);
		m_RspSubfunction = UDS_GET_SUB_FUNCTION(msg_buf[1]);
		if (m_ReqSsp == 0x00)
		{
			if (m_ReqSid == SID_10)
			{
				if (m_ReqSubfunction == UDS_SESSION_STD)
					uds_timer_stop(UDS_TIMER_S3client);
				else
					uds_timer_start(UDS_TIMER_S3client);
			}

			if (m_ReqSid == SID_11)
			{
				uds_timer_stop(UDS_TIMER_S3client);
			}

			/* Subsequent start the S3cleint timer */
			if (m_ReqSid == SID_3E)
			{
				uds_timer_start(UDS_TIMER_S3client);
			}
		}
		/* Subsequent start the S3cleint timer */
		if (m_ReqSsp == 0x00 && uds_timer_chk(UDS_TIMER_S3client) > 0)
			uds_timer_start(UDS_TIMER_S3client);

		/* have forgotten why do this? */
		if (m_RspSid == 0x2E)
			RspData[20] = 0x11;

		if (m_RspSid == 0x27 && (m_RspSubfunction == 0x01 || m_RspSubfunction == 0x05))
		{
			m_RspBuf[0] = msg_buf[2];
			m_RspBuf[1] = msg_buf[3];
			m_RspBuf[2] = msg_buf[4];
			m_RspBuf[3] = msg_buf[5];
		}

		if (m_RspSid == 0x10)
			m_EntBoot = 0;
	}

	RspDlc = msg_dlc;
	for (UINT i = 0; i < RspDlc; i++)
		RspData[i] = msg_buf[i];

	m_GetRsp = TRUE;
}


/**
* N_USData_confirm - uds response confirm
*
* @n_result :
*
* returns:
*     void
*/
void CUdsClient::N_USData_confirm(n_result_t n_result)
{
	if (n_result == N_OK)
	{
		uds_timer_start(UDS_TIMER_P2client);
		/* Clear UDS_TIMER_P2client_x timeout */
		uds_timer_clear(UDS_TIMER_P2client_x);

		if (m_FunReq == TRUE)
		{
			if (m_ReqSid == SID_10)
			{
				if (m_ReqSubfunction == UDS_SESSION_STD)
					uds_timer_stop(UDS_TIMER_S3client);
				else
				    uds_timer_start(UDS_TIMER_S3client);
			}
			if (m_ReqSid == SID_11)
			{
				uds_timer_stop(UDS_TIMER_S3client);
			}
		}
		else
		{
			if (m_ReqSsp == 0x01)
			{
				if (m_ReqSid == SID_10)
				{
					if (m_ReqSubfunction == UDS_SESSION_STD)
						uds_timer_stop(UDS_TIMER_S3client);
					else
						uds_timer_start(UDS_TIMER_S3client);
				}

				if (m_ReqSid == SID_11)
				{
					uds_timer_stop(UDS_TIMER_S3client);
				}

				/* Subsequent start the S3cleint timer */
				if (m_ReqSid == SID_3E)
				{
					uds_timer_start(UDS_TIMER_S3client);
				}
			}
			/* Subsequent start the S3cleint timer */
			if (m_ReqSsp == 0x01 && uds_timer_chk(UDS_TIMER_S3client) > 0)
				uds_timer_start(UDS_TIMER_S3client);
		}

	}
	else
	{
		n_ResultErr = TRUE;
		n_Result = n_result;
	}
}
/**
* read_aline - read a line from .hex file
*
* @dst_buff  :
* @buff_len  :
*
* returns:
*     read len
*/
int CUdsClient::read_aline(unsigned char *dst_buff, UINT buff_len)
{
	int rdn_n;
	CString Rdstr;

	if (myFile.ReadString(Rdstr) == TRUE)
	{
		rdn_n = UdsUtil::str2char(Rdstr, dst_buff, buff_len) - 1;
		printf("ian.h: read [%d] bytes from myFile, buff len is [%d].\n", rdn_n, buff_len);
	}
	else
	{
		rdn_n = 0;
		printf("ian.h:hex file read end***************************\n");
	}

	return rdn_n;
}

/**
* read_memaddr - read app data from .hex file
*
* @void  :
*
* returns:
*     read len
*/
INT CUdsClient::read_memaddr(UINT &mem_addr, UINT &mem_size)
{
	BYTE hex_record[RECORD_BUF_LEN];
	BYTE hex_temp[DATA_BUF_LEN];
	int  record_len;
	UINT base_addr;
	UINT ext_addr;
	UINT end_addr;
	BYTE get_addr_flag = 0;

	BYTE r_start;
	BYTE r_count;
	UINT r_addr;
	BYTE r_type;
	BYTE r_data[DATA_BUF_LEN];

	mem_addr = 0;
	mem_size = 0;

	INT  whl = 1;
	do
	{
		record_len = read_aline(hex_record, RECORD_BUF_LEN);

		if (record_len > 0)
		{
			/* Start code, one character, an ASCII colon ':'. */
			r_start = hex_record[0];
			if (r_start != ':')
				continue;

			/* hex record used two characters explain one hex data */
			/* transfer record count to hex data */
			UdsUtil::ascii_to_hex(&hex_record[1], hex_temp, 2);
			/* get record count */
			r_count = hex_temp[0];
			if (record_len != (r_count * 2 + 11)) { /*start 1,type 2,count 2, addr 4, checksum 2, CR LF 2*/
				printf("the s19 record len err.\n");
				break;
			}

			/* transfer record count to hex data */
			UdsUtil::ascii_to_hex(&hex_record[3], hex_temp, 4);
			/* get record addr */
			r_addr = 0;
			r_addr = hex_temp[0];
			r_addr = ((r_addr << 8) | hex_temp[1]);

			/* transfer record count to hex data */
			UdsUtil::ascii_to_hex(&hex_record[7], hex_temp, 2);
			/* get record type */
			r_type = hex_temp[0];

			/* get address,data and checksum,transfer to hex data*/
			UdsUtil::ascii_to_hex(&hex_record[9], r_data, r_count * 2);

			printf("s19 count = [%d],s19 record len = [%d]\n", r_count, record_len);

			switch (r_type)
			{
			case 00:
				ext_addr = ((base_addr << 16) | r_addr);
				if (get_addr_flag == 0)
				{
					get_addr_flag = 1;
					mem_addr = ext_addr;
				}
				end_addr = ext_addr + r_count - 1;
				break;
			case 01:
				/* .hex file end */
				whl = 0;
				break;
			case 04:
				base_addr = 0;
				base_addr = r_data[0];
				base_addr = ((base_addr << 8) | r_data[1]);
				break;
			case 02:
			case 03:
				base_addr = 0;
			case 05:
				/**
				* The 4 data bytes represent the 32-bit value loaded into the
				* EIP register of the 80386 and higher CPU
				* 类型05后面跟的是4字节地址，将赋给（STM8或者ARM芯片的）PC指针。
				*/
				break;
			default:
				/* .hex file err */
				whl = 0;
				break;
			}
		}

	} while (whl && record_len > 0);

	mem_size = end_addr - mem_addr + 1;
	return 0;
}

int CUdsClient::read_memdata(LPBYTE mem_buf, UINT mem_addr, UINT mem_size)
{
	BYTE hex_record[RECORD_BUF_LEN];
	BYTE hex_temp[DATA_BUF_LEN];
	int  record_len;
	int  read_len;
	UINT base_addr;
	UINT ext_addr;

	BYTE r_start;
	BYTE r_count;
	UINT r_addr;
	BYTE r_type;
	BYTE r_data[DATA_BUF_LEN];

	read_len = 0;

	int whl = 1;
	do
	{
		record_len = read_aline(hex_record, RECORD_BUF_LEN);

		if (record_len > 0)
		{
			/* Start code, one character, an ASCII colon ':'. */
			r_start = hex_record[0];
			if (r_start != ':')
				continue;

			/* hex record used two characters explain one hex data */
			/* transfer record count to hex data */
			UdsUtil::ascii_to_hex(&hex_record[1], hex_temp, 2);
			/* get record count */
			r_count = hex_temp[0];
			if (record_len != (r_count * 2 + 11)) { /*start 1,type 2,count 2, addr 4, checksum 2, CR LF 2*/
				printf("the s19 record len err.\n");
				break;
			}

			/* transfer record count to hex data */
			UdsUtil::ascii_to_hex(&hex_record[3], hex_temp, 4);
			/* get record addr */
			r_addr = 0;
			r_addr = hex_temp[0];
			r_addr = ((r_addr << 8) | hex_temp[1]);

			/* transfer record count to hex data */
			UdsUtil::ascii_to_hex(&hex_record[7], hex_temp, 2);
			/* get record type */
			r_type = hex_temp[0];

			/* get address,data and checksum,transfer to hex data*/
			UdsUtil::ascii_to_hex(&hex_record[9], r_data, r_count * 2);

			printf("s19 count = [%d],s19 record len = [%d]\n", r_count, record_len);

			switch (r_type)
			{
			case 00:
				ext_addr = ((base_addr << 16) | r_addr);

				if (ext_addr >= (mem_addr + read_len))
				{
					memcpy(mem_buf + ext_addr - mem_addr, r_data, r_count);

					if (ext_addr > (mem_addr + read_len))
					{
						memset(mem_buf + read_len, 0, ext_addr - mem_addr - read_len);
					}
			
					read_len = ext_addr - mem_addr + r_count;
				}
				else
				{
					/* r_addr err */
					whl = 0;
				}
				break;
			case 01:
				/* file end */
				whl = 0;
				break;
			case 04:
				base_addr = 0;
				base_addr = r_data[0];
				base_addr = ((base_addr << 8) | r_data[1]);
				break;
			case 02:
			case 03:
			case 05:
			default:
				break;
			}
		}

	} while (whl && record_len > 0);

	return read_len;
}


/**
* read_block - read a block data from pmem_buf
*
* @dst_buff  :
* @block_len :
*
* returns:
*     read len
*/
int CUdsClient::read_block(const LPBYTE mem_buff, UINT mem_size, unsigned char *dst_buff, UINT block_len)
{
	if (dst_buff == NULL || mem_buff == NULL) return -1;
	int read_len;
	if ((mem_size - total_xmit_len) >= block_len)
		read_len = block_len;
	else
		read_len = mem_size - total_xmit_len;
	memcpy(dst_buff, mem_buff + total_xmit_len, read_len);
	return read_len;
}

/**
* do_cmdlist - pop cmd from cmdlist,and send the uds cmd
*
* @void  :
*
* returns:
*     void
*/
void CUdsClient::do_cmdlist(void)
{
	if (uds_timer_chk(UDS_TIMER_P2client) > 0)   /* P2client is running */
		return;

	if (uds_timer_chk(UDS_TIMER_P2client_x) > 0) /* P2*client is running */
		return;
	UdsCmd m_CmdNow;
	INT_PTR CmdSize = m_CmdList.GetSize();
	if (CmdSize <= 0) return;
	m_CmdNow = m_CmdList[0];
	m_CmdList.RemoveAt(0, 1);
	if (m_CmdNow.SID == SID_27 && (m_CmdNow.CmdBuf[0] == 0x02 || m_CmdNow.CmdBuf[0] == 0x06))
	{
		UdsUtil::KeyCalcu(m_RspBuf, &m_CmdNow.CmdBuf[1]);
	}

	request(m_CmdNow.SID, m_CmdNow.CmdBuf, m_CmdNow.CmdLen);
}

/**
* do_response - handle the upgrade response, Called by main_loop
*
* @void  :
*
* returns:
*     void
*/
BYTE CUdsClient::do_response(BYTE msg_buf[], WORD msg_dlc)
{
	BYTE Ret = 0;
	if (m_GetRsp == FALSE) return 0;
	m_GetRsp = FALSE;

	if (msg_buf[0] == NEGATIVE_RSP && m_RspNrc != NRC_SERVICE_BUSY)
	{
		upgrade_status = UDS_PROG_NONE;
		return 0x01;
	}

	switch (m_RspSid)
	{
	    case SID_10:
			if (upgrade_status == UDS_PROG_EXTENDED_SESSION && m_RspSubfunction == UDS_SESSION_EOL)
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_READ_DIDF187;
			}
			if (upgrade_status == UDS_PROG_PROGRAM_SESSION && m_RspSubfunction == UDS_SESSION_PROG)
			{
				Sleep(1500); /* sleep 1s for sa */
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_SA_SEED;
			}
			break;
		case SID_11:
			break;
		case SID_22:
			WORD did;
			did = ((WORD)msg_buf[1]) << 8;
			did |= msg_buf[2];
			if (did == 0xF187)
			{
				memcpy(ecu_part_num, &RspData[3], 15);
				if (upgrade_status == UDS_PROG_READ_DIDF187)
				{
					m_UpdRsp = TRUE;
					upgrade_status = UDS_PROG_DTCOFF;
				}
			}
			if (did == 0xF186)
			{
				cur_seesion = RspData[3];
				if (upgrade_status == UDS_PROG_READ_DIDF186)
				{
					m_UpdRsp = TRUE;
					if (cur_seesion == UDS_SESSION_STD)
						upgrade_status = UDS_PROG_EXTENDED_SESSION;
					if (cur_seesion == UDS_SESSION_EOL)
						upgrade_status = UDS_PROG_READ_DIDF187;
					if (cur_seesion == UDS_SESSION_PROG)
						upgrade_status = UDS_PROG_SA_SEED;
				}
			}

			break;
		case SID_2E:
			upgrade_status = UDS_PROG_ENUM_MAX;
			break;
		case SID_27:
			if (upgrade_status == UDS_PROG_SA_KEY && m_RspSubfunction == 0x06)
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_FLASH_DRIVER_REQ_DOWLOAD;
			}

			if (upgrade_status == UDS_PROG_SA_SEED && (m_RspSubfunction == 0x05))
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_SA_KEY;
			}
			break;
		case SID_28:
			if (upgrade_status == UDS_PROG_DISABLE_RXTX)
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_PROGRAM_SESSION;
			}
			break;
		case SID_31:
			WORD rid;
			rid = ((WORD)msg_buf[2]) << 8;
			rid |= msg_buf[3];
			if (upgrade_status == UDS_PROG_FLASH_DRIVER_CRC32 && rid == 0xF001)
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_ERASE_MEMORY;
			}
			if (upgrade_status == UDS_PROG_ERASE_MEMORY && rid == 0xFF00)
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_APP_REQ_DOWNLOAD;
			}
			if (upgrade_status == UDS_PROG_APP_CRC32 && rid == 0xF001)
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_CHECK_DEPENDENCY;
			}
			if (upgrade_status == UDS_PROG_CHECK_DEPENDENCY && rid == 0xFF01)
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_WRITE_DIDF199;
			}
			break;
		case SID_34:
			if (upgrade_status == UDS_PROG_FLASH_DRIVER_REQ_DOWLOAD)
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_FLASH_DRIVER_DOWNLOADING;
				block_len = UdsUtil::can_to_hostl(&msg_buf[2]);
			}
			if (upgrade_status == UDS_PROG_APP_REQ_DOWNLOAD)
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_APP_DOWNLOADING;
				block_len = UdsUtil::can_to_hostl(&msg_buf[2]);
			}
			break;
		case SID_36:
			if (upgrade_status == UDS_PROG_FLASH_DRIVER_DOWNLOADING)
			{
				recv_sn = msg_buf[1];
				m_UpdRsp = TRUE;
				if (total_xmit_len >= dri_size)
					upgrade_status = UDS_PROG_FLASH_DRIVER_EXIT_DOWNLOAD;
				if (block_sn != recv_sn)
					Ret = 0x02;
			}

			if (upgrade_status == UDS_PROG_APP_DOWNLOADING)
			{
				recv_sn = msg_buf[1];
				m_UpdRsp = TRUE;
				if (total_xmit_len >= app_size)
					upgrade_status = UDS_PROG_APP_EXIT_DOWNLOAD;
				if (block_sn != recv_sn)
					Ret = 0x03;
			}
			break;
		case SID_37:
			if (upgrade_status == UDS_PROG_FLASH_DRIVER_EXIT_DOWNLOAD)
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_FLASH_DRIVER_CRC32;
			}
			if (upgrade_status == UDS_PROG_APP_EXIT_DOWNLOAD)
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_APP_CRC32;
			}
			break;
		case SID_3E:
			break;
		case SID_85:
			if (upgrade_status == UDS_PROG_DTCOFF)
			{
				m_UpdRsp = TRUE;
				upgrade_status = UDS_PROG_DISABLE_RXTX;
			}
			break;
		default:
			break;
	}
	return 0;
}
/**
* do_upgrade - handle the upgrade task, Called by main_loop
*
* @void  :
*
* returns:
*     void
*/
void CUdsClient::do_upgrade(void)
{
	UdsCmd CmdNew;
	if (upgrade_status == UDS_PROG_NONE)
		return;

	if (m_UpdRsp == FALSE) return;
	m_UpdRsp = FALSE;
	m_FunReq = FALSE;
	switch (upgrade_status)
	{
	    case UDS_PROG_READ_DIDF186:
			//Push cmd, Read DID 0xF186
			CmdNew.SID = SID_22;
			CmdNew.CmdBuf[0] = 0xF1;
			CmdNew.CmdBuf[1] = 0x86;
			CmdNew.CmdLen = 2;
			push_cmd(CmdNew);
		    break;
		case UDS_PROG_EXTENDED_SESSION:
			//Push cmd, extended Session 
			m_FunReq = TRUE;
			CmdNew.SID = SID_10;
			CmdNew.CmdBuf[0] = 0x03;
			CmdNew.CmdLen = 1;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_READ_DIDF187:
			//Push cmd, Read DID 0xF187
			CmdNew.SID = SID_22;
			CmdNew.CmdBuf[0] = 0xF1;
			CmdNew.CmdBuf[1] = 0x87;
			CmdNew.CmdLen = 2;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_DTCOFF:
			//Push cmd, Set DTC Off
			m_FunReq = TRUE;
			CmdNew.SID = SID_85;
			CmdNew.CmdBuf[0] = 0x02;
			CmdNew.CmdLen = 1;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_DISABLE_RXTX:
			m_FunReq = TRUE;
			CmdNew.SID = SID_28;
			CmdNew.CmdBuf[0] = 0x03;
			CmdNew.CmdBuf[1] = 0x01;
			CmdNew.CmdLen = 2;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_PROGRAM_SESSION:
			//Push cmd, program Session 
			CmdNew.SID = SID_10;
			CmdNew.CmdBuf[0] = 0x02;
			CmdNew.CmdLen = 1;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_SA_SEED:
			//Push cmd, request seed
			CmdNew.SID = SID_27;
			CmdNew.CmdBuf[0] = 0x05;
			CmdNew.CmdLen = 1;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_SA_KEY:
			//Push cmd, send key
			CmdNew.SID = SID_27;
			CmdNew.CmdBuf[0] = 0x06;
			CmdNew.CmdLen = 5;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_FLASH_DRIVER_REQ_DOWLOAD:
			block_sn = 0;
			total_xmit_len = 0;
			dri_crc32 = CRC32_INIT;

			CmdNew.SID = SID_34;
			CmdNew.CmdBuf[0] = 0x00;
			CmdNew.CmdBuf[1] = 0x44;
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[2], dri_addr);
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[6], dri_size);
			CmdNew.CmdLen = 10;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_FLASH_DRIVER_DOWNLOADING:
			UINT read_len;
			block_sn++;
			//Push cmd, Transfer Data
			CmdNew.SID = SID_36;
			CmdNew.CmdBuf[0] = block_sn;
			read_len = read_block(pdri_buf, dri_size, &CmdNew.CmdBuf[1], block_len);
			CmdNew.CmdLen = read_len + 1;
			push_cmd(CmdNew);
			total_xmit_len += read_len;
			dri_crc32 = UdsUtil::crc32_discontinue(dri_crc32, &CmdNew.CmdBuf[1], read_len);
			break;
		case UDS_PROG_FLASH_DRIVER_EXIT_DOWNLOAD:
			//Push cmd, RequestTransferExit
			CmdNew.SID = SID_37;
			CmdNew.CmdLen = 0;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_FLASH_DRIVER_CRC32:
			//Push cmd, Check routine
			CmdNew.SID = SID_31;
			CmdNew.CmdBuf[0] = 0x01;
			CmdNew.CmdBuf[1] = 0xF0;
			CmdNew.CmdBuf[2] = 0x01;
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[3], dri_crc32);
			CmdNew.CmdLen = 7;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_ERASE_MEMORY:
			//Push cmd, Erase memory
			CmdNew.SID = SID_31;
			CmdNew.CmdBuf[0] = 0x01;
			CmdNew.CmdBuf[1] = 0xFF;
			CmdNew.CmdBuf[2] = 0x00;
			CmdNew.CmdBuf[3] = 0x44;
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[4], app_addr);
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[8], app_size);
			CmdNew.CmdLen = 12;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_APP_REQ_DOWNLOAD:
			block_sn = 0;
			total_xmit_len = 0;
			app_crc32 = CRC32_INIT;
			//Push cmd, Request Download
			CmdNew.SID = SID_34;
			CmdNew.CmdBuf[0] = 0x00;
			CmdNew.CmdBuf[1] = 0x44;
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[2], app_addr);
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[6], app_size);
			CmdNew.CmdLen = 10;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_APP_DOWNLOADING:
		{
			UINT read_len;
			block_sn++;
			//Push cmd, Transfer Data
			CmdNew.SID = SID_36;
			CmdNew.CmdBuf[0] = block_sn;
			memset(&CmdNew.CmdBuf[1], 0, block_len);
			read_len = read_block(papp_buf, app_size, &CmdNew.CmdBuf[1], block_len);
			CmdNew.CmdLen = read_len + 1;
			push_cmd(CmdNew);
			total_xmit_len += read_len;
			app_crc32 = UdsUtil::crc32_discontinue(app_crc32, &CmdNew.CmdBuf[1], read_len);
		}
			break;
		case UDS_PROG_APP_EXIT_DOWNLOAD:
			//Push request cmd, RequestTransferExit
			CmdNew.SID = SID_37;
			CmdNew.CmdLen = 0;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_APP_CRC32:
			//Push Routine cmd, Check routine
			CmdNew.SID = SID_31;
			CmdNew.CmdBuf[0] = 0x01;
			CmdNew.CmdBuf[1] = 0xF0;
			CmdNew.CmdBuf[2] = 0x01;
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[3], app_crc32);
			CmdNew.CmdLen = 7;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_CHECK_DEPENDENCY:
			//Push Routine cmd, Check Programming Dependencies 
			CmdNew.SID = SID_31;
			CmdNew.CmdBuf[0] = 0x01;
			CmdNew.CmdBuf[1] = 0xFF;
			CmdNew.CmdBuf[2] = 0x01;
			CmdNew.CmdLen = 3;
			push_cmd(CmdNew);
			break;
#if 0
		case UDS_PROG_WRITE_DIDF198:
		{
			CmdNew.SID = SID_2E;
			CmdNew.CmdBuf[0] = 0xF1;
			CmdNew.CmdBuf[1] = 0x98;
			memcpy(&CmdNew.CmdBuf[2], tester_sn, 10);
			CmdNew.CmdLen = 12;
			dlg->UdsClient.push_cmd(CmdNew);
			break;
		}
#endif

		case UDS_PROG_WRITE_DIDF199:
		{
			SYSTEMTIME   systime;
			GetLocalTime(&systime);
			CmdNew.SID = SID_2E;
			CmdNew.CmdBuf[0] = 0xF1;
			CmdNew.CmdBuf[1] = 0x99;
			CmdNew.CmdBuf[2] = UdsUtil::HEX2BCD(systime.wYear % 1000);
			CmdNew.CmdBuf[3] = UdsUtil::HEX2BCD(systime.wMonth);
			CmdNew.CmdBuf[4] = UdsUtil::HEX2BCD(systime.wDay);
			CmdNew.CmdLen = 5;
			push_cmd(CmdNew);
			break;
		}
		case UDS_PROG_ECU_RESET:
			break;
		case UDS_PROG_NONE:
		default:
			break;
	}

	/*0-7 8-16 17-20 21-94 95-100 */
	if (upgrade_status < UDS_PROG_FLASH_DRIVER_DOWNLOADING)
	    curre_step = upgrade_status;
	if (upgrade_status == UDS_PROG_FLASH_DRIVER_DOWNLOADING)
		curre_step = UDS_PROG_FLASH_DRIVER_DOWNLOADING + (8 * total_xmit_len / dri_size);
	if (upgrade_status > UDS_PROG_FLASH_DRIVER_DOWNLOADING && upgrade_status < UDS_PROG_APP_DOWNLOADING)
		curre_step = upgrade_status + 8;
	if (upgrade_status == UDS_PROG_APP_DOWNLOADING)
		curre_step = 21 + (73 * total_xmit_len / app_size);
	if (upgrade_status > UDS_PROG_APP_DOWNLOADING)
		curre_step = upgrade_status + 80;
}

/**
* do_entboot - try to enter bootloader(program session)
*
* @void  :
*
* returns:
*     void
*/
void CUdsClient::do_entboot(void)
{
	if (m_EntBoot > 0)
	{
		m_EntBoot--;
		if (m_EntBoot % 10) return;
		UdsCmd m_CmdNow;
		m_CmdNow.SID = 0x10;
		m_CmdNow.CmdBuf[0] = 0x03;
		m_CmdNow.CmdLen = 1;
		request(m_CmdNow.SID, m_CmdNow.CmdBuf, m_CmdNow.CmdLen);

		m_CmdNow.SID = 0x10;
		m_CmdNow.CmdBuf[0] = 0x02;
		m_CmdNow.CmdLen = 1;
		request(m_CmdNow.SID, m_CmdNow.CmdBuf, m_CmdNow.CmdLen);
	}

}

/*******************************************************************************
Function  Definition - common
*******************************************************************************/

void CUdsClient::ZTai_UDS_Send(BYTE CanData[], BYTE CanDlc)
{
	UINT i;
	VCI_CAN_OBJ SendObj[1];

	int FrameFormat, FrameType;

	FrameFormat = FRMFMT_STD;
	FrameType   = FRMTYP_DAT;

	SendObj->SendType = 0;
	SendObj->ExternFlag = FrameType;
	SendObj->DataLen = CanDlc;
	SendObj->RemoteFlag = FrameFormat;
	if (FrameFormat == 1)//if remote frame, data area is invalid
	{
		for (i = 0; i < CanDlc; i++)
			CanData[i] = 0;
	}

	if (m_FunReq == TRUE)
		SendObj->ID = m_Fucid;
	else
	    SendObj->ID = m_Phyid;

	for (i = 0; i<CanDlc; i++)
		SendObj->Data[i] = CanData[i];

	CUdsUpdateToolDlg::TransmitCanmsg(SendObj);
}


/**
* main_loop - uds main loop, should be schedule every 1 ms
*
* @void  :
*
* returns:
*     void
*/
BYTE CUdsClient::main_loop(void)
{
	BYTE ret = 0;
	network_main();

	if (uds_timer_run(UDS_TIMER_S3client) < 0)
	{
		UdsCmd CmdNew;
		//TesterPresent request cmd
		CmdNew.SID = SID_3E;
		CmdNew.CmdBuf[0] = 0x00;
		CmdNew.CmdLen = 1;
		push_cmd(CmdNew);

	}
	if (uds_timer_run(UDS_TIMER_P2client) < 0)
	{
		m_GetRsp = FALSE;
		m_ReqSid = 0;
		m_UpdRsp = 0;
		/* Stop upgrade */
		upgrade_status = UDS_PROG_NONE;
		ret |= RET_TIMOUT_P2;
	}
	if (uds_timer_run(UDS_TIMER_P2client_x) < 0)
	{
		m_GetRsp = FALSE;
		m_ReqSid = 0;
		m_UpdRsp = 0;
		/* Stop upgrade */
		upgrade_status = UDS_PROG_NONE;
		ret |= RET_TIMOUT_P2_x;
	}
	/* Handle uds upgrade */
	do_upgrade();

	/* Handle Cmd list */
	do_cmdlist();
	/* Try enter bootloader */
	do_entboot();

	if (do_response(RspData, RspDlc) != 0)
	{
		uds_timer_stop(UDS_TIMER_S3client);
		ret |= RET_RESPONSE;
	}

	if (upgrade_status == UDS_PROG_ENUM_MAX)
	{
		upgrade_status = UDS_PROG_NONE;
		ret |= RET_DONE;

	}
	return ret;
}

/**
* sv_request - uds service request
*
* @void  :
*
* returns:
*     void
*/
void CUdsClient::request(BYTE SvcId, BYTE ReqBuf[], UINT ReqLen)
{
	BYTE cmd_buf[BUF_LEN];

	m_ReqSid = SvcId;
	m_ReqSsp = UDS_GET_SUB_FUNCTION_SUPPRESS_POSRSP(ReqBuf[0]);
	m_ReqSubfunction = UDS_GET_SUB_FUNCTION(ReqBuf[0]);

	cmd_buf[0] = SvcId;
	for (UINT i = 0; i < ReqLen; i++)
	{
		cmd_buf[1 + i] = ReqBuf[i];
	}
	netowrk_send_udsmsg(cmd_buf, ReqLen+1);
}


UINT CUdsClient::get_rsp(BYTE DataBuf[], UINT BufLen)
{
	/**
	DWORD Ticks;
	Ticks = timeGetTime();
	while (GetRsp == FALSE)
	{
		if ((timeGetTime() - Ticks) >= 2000)
			break;
		Sleep(200);
	}*/

	if (m_GetRsp == TRUE)
	{
		UINT i;
		for (i = 0; i < RspDlc && i < BufLen; i++)
			DataBuf[i] = RspData[i];
		return i;
	}
	else
	{
		return 0;
	}
}



/**
* push_cmd - push a cmd to cmdlist
*
* @void  :
*
* returns:
*     void
*/
void CUdsClient::push_cmd(UdsCmd Cmd)
{
	m_CmdList.Add(Cmd);
}

/**
* start_upgrade - start upgrade task
*
* @void  :
*
* returns:
*     void
*/
UINT CUdsClient::start_upgrade(void)
{
	if (upgrade_status != UDS_PROG_NONE)
		return 1;
	m_UpdRsp = TRUE;
	upgrade_status = UDS_PROG_READ_DIDF186;

	curre_step = 0;
	m_EntBoot = 0;
	return 0;
}

/**
* start_upgrade - start upgrade task
*
* @void  :
*
* returns:
*     void
*/
UINT CUdsClient::stop_upgrade(void)
{
	m_UpdRsp = FALSE;
	curre_step = 0;
	upgrade_status = UDS_PROG_NONE;
	uds_timer_stop(UDS_TIMER_S3client);

	return 0;
}

/**
* open_mcuapp_file - Open the upgrade file
*
* @void  :
*
* returns:
*     void
*/
UINT CUdsClient::open_mcuapp_file(CString FilePath)
{

	CFileException fileException;

	if (myFile.Open(FilePath, (CFile::typeText | CFile::modeReadWrite), &fileException))
	{
		myFile.SeekToBegin();
		read_memaddr(app_addr, app_size);

		if (papp_buf != NULL)
			delete[] papp_buf;
		papp_buf = new BYTE[app_size + DATA_BUF_LEN];

		myFile.SeekToBegin();
		read_memdata(papp_buf, app_addr, app_size);

		myFile.Close();

	}

	return fileException.m_cause;

}

/**
* open_driver_file - Open the driver file
*
* @void  :
*
* returns:
*     void
*/
UINT CUdsClient::open_driver_file(CString FilePath)
{

	CFileException fileException;

	if (myFile.Open(FilePath, (CFile::typeText | CFile::modeReadWrite), &fileException))
	{
		myFile.SeekToBegin();
		read_memaddr(dri_addr, dri_size);
		
		if (pdri_buf != NULL)
			delete[] pdri_buf;
		pdri_buf = new BYTE[dri_size + DATA_BUF_LEN];

		myFile.SeekToBegin();
		read_memdata(pdri_buf, dri_addr, dri_size);
		myFile.Close();
	}

	return fileException.m_cause;

}