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

	m_FileOpen = FALSE;
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
		uds_timer[UDS_TIMER_FSA] = GetTickCount();
		TIMOUT_VALUE[UDS_TIMER_FSA] = TIMEOUT_FSA;
		uds_timeo[UDS_TIMER_FSA] = 0;
	}
	if (num == UDS_TIMER_S3client) {
		uds_timer[UDS_TIMER_S3client] = GetTickCount();
		TIMOUT_VALUE[UDS_TIMER_S3client] = TIMEOUT_S3client;
		uds_timeo[UDS_TIMER_S3client] = 0;
	}
	if (num == UDS_TIMER_P2client) {
		uds_timer[UDS_TIMER_P2client] = GetTickCount();
		TIMOUT_VALUE[UDS_TIMER_P2client] = TIMEOUT_P2client;
		uds_timeo[UDS_TIMER_P2client] = 0;
	}
	if (num == UDS_TIMER_P2client_x) {
		uds_timer[UDS_TIMER_P2client_x] = GetTickCount();
		TIMOUT_VALUE[UDS_TIMER_P2client_x] = TIMEOUT_P2client_x;
		uds_timeo[UDS_TIMER_P2client_x] = 0;
	}
}

void CUdsClient::uds_timer_stop(BYTE num)
{
	if (num >= UDS_TIMER_CNT) return;

	uds_timer[num] = 0;
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
		DWORD Tikcs = GetTickCount();

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
* do_upgrade_rsp - handle the upgrade response, Called by main_loop
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

	if (msg_buf[0] == NEGATIVE_RSP)
	{
		upgrade_step = UDS_PROG_NONE;
		return 0x01;
	}

	switch (m_RspSid)
	{
	    case SID_10:
			if (upgrade_step == UDS_PROG_EXTENDED_SESSION && m_RspSubfunction == UDS_SESSION_EOL)
			{
				m_UpdRsp = TRUE;
				upgrade_step = UDS_PROG_READ_DIDF187;
			}
			if (upgrade_step == UDS_PROG_PROGRAM_SESSION && m_RspSubfunction == UDS_SESSION_PROG)
			{
				m_UpdRsp = TRUE;
				upgrade_step = UDS_PROG_SA;
			}
			break;
		case SID_11:
			break;
		case SID_22:
			WORD did;
			did = ((WORD)msg_buf[1]) << 8;
			did |= msg_buf[2];
			if (upgrade_step == UDS_PROG_READ_DIDF187 && did == 0xF187)
			{
				m_UpdRsp = TRUE;
				memcpy(ecu_part_num, &RspData[3], 15);
				upgrade_step = UDS_PROG_DTCOFF;
			}
			break;
		case SID_2E:
			break;
		case SID_27:
			if (upgrade_step == UDS_PROG_SA && m_RspSubfunction == 0x06)
			{
				m_UpdRsp = TRUE;
				upgrade_step = UDS_PROG_FLASH_DRIVER_CRC32;
			}

			if (msg_dlc == 6 && (m_RspSubfunction == 0x01 || m_RspSubfunction == 0x05))
			{
				m_RspBuf[0] = msg_buf[2];
				m_RspBuf[1] = msg_buf[3];
				m_RspBuf[2] = msg_buf[4];
				m_RspBuf[3] = msg_buf[5];
			}
			break;
		case SID_28:
			if (upgrade_step == UDS_PROG_DISABLE_RXTX)
			{
				m_UpdRsp = TRUE;
				upgrade_step = UDS_PROG_PROGRAM_SESSION;
			}
			break;
		case SID_31:
			WORD rid;
			rid = ((WORD)msg_buf[2]) << 8;
			rid |= msg_buf[3];
			if (upgrade_step == UDS_PROG_FLASH_DRIVER_CRC32 && rid == 0xF001)
				upgrade_step = UDS_PROG_ERASE_MEMORY;
			if (upgrade_step == UDS_PROG_ERASE_MEMORY && rid == 0xFF00)
				upgrade_step = UDS_PROG_APP_REQ_DOWNLOAD;
			if (upgrade_step == UDS_PROG_APP_CRC32 && rid == 0xF001)
				upgrade_step = UDS_PROG_CHECK_DEPENDENCY;
			if (upgrade_step == UDS_PROG_CHECK_DEPENDENCY && rid == 0xFF01)
				upgrade_step = UDS_PROG_ECU_RESET;
			break;
		case SID_34:
			if (upgrade_step == UDS_PROG_APP_REQ_DOWNLOAD)
			{
				m_UpdRsp = TRUE;
				upgrade_step = UDS_PROG_APP_DOWNLOADING;
			}
			break;
		case SID_36:
			if (upgrade_step == UDS_PROG_APP_DOWNLOADING)
			{
				recv_sn = msg_buf[1];
				m_UpdRsp = TRUE;
				if (total_xmit_len >= total_mem_size)
					upgrade_step = UDS_PROG_APP_EXIT_DOWNLOAD;
			}
			break;
		case SID_37:
			if (upgrade_step == UDS_PROG_APP_EXIT_DOWNLOAD)
			{
				m_UpdRsp = TRUE;
				upgrade_step = UDS_PROG_APP_CRC32;
			}
			break;
		case SID_3E:
			break;
		case SID_85:
			if (upgrade_step == UDS_PROG_DTCOFF)
			{
				m_UpdRsp = TRUE;
				upgrade_step = UDS_PROG_DISABLE_RXTX;
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
	if (upgrade_step == UDS_PROG_NONE)
		return;

	if (m_UpdRsp == FALSE) return;
	m_UpdRsp = FALSE;
	switch (upgrade_step)
	{
		case UDS_PROG_EXTENDED_SESSION:
			//Push request cmd, extended Session 
			CmdNew.SID = SID_10;
			CmdNew.CmdBuf[0] = 0x03;
			CmdNew.CmdLen = 1;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_READ_DIDF187:
			//Push request cmd, Read DID 0xF187
			CmdNew.SID = SID_22;
			CmdNew.CmdBuf[0] = 0xF1;
			CmdNew.CmdBuf[1] = 0x87;
			CmdNew.CmdLen = 2;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_DTCOFF:
			//Push request cmd, Set DTC Off
			CmdNew.SID = SID_85;
			CmdNew.CmdBuf[0] = 0x02;
			CmdNew.CmdLen = 1;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_DISABLE_RXTX:
			CmdNew.SID = SID_28;
			CmdNew.CmdBuf[0] = 0x03;
			CmdNew.CmdBuf[1] = 0x01;
			CmdNew.CmdLen = 2;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_PROGRAM_SESSION:
			//Push request cmd, program Session 
			CmdNew.SID = SID_10;
			CmdNew.CmdBuf[0] = 0x02;
			CmdNew.CmdLen = 1;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_SA:
			//Push request cmd, request seed
			CmdNew.SID = SID_27;
			CmdNew.CmdBuf[0] = 0x05;
			CmdNew.CmdLen = 1;
			push_cmd(CmdNew);

			//Push request cmd, send key
			CmdNew.SID = SID_27;
			CmdNew.CmdBuf[0] = 0x06;
			CmdNew.CmdLen = 5;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_FLASH_DRIVER_CRC32:
			//Push Routine cmd, Check routine
			CmdNew.SID = SID_31;
			CmdNew.CmdBuf[0] = 0x01;
			CmdNew.CmdBuf[1] = 0xF0;
			CmdNew.CmdBuf[2] = 0x01;
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[3], dri_crc32);
			CmdNew.CmdLen = 7;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_ERASE_MEMORY:
			//Push Routine cmd, Erase memory
			CmdNew.SID = SID_31;
			CmdNew.CmdBuf[0] = 0x01;
			CmdNew.CmdBuf[1] = 0xFF;
			CmdNew.CmdBuf[2] = 0x00;
			CmdNew.CmdBuf[3] = 0x44;
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[4], mem_addr);
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[8], mem_size);
			CmdNew.CmdLen = 12;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_APP_REQ_DOWNLOAD:
			//Push cmd, Request Download
			CmdNew.SID = SID_34;
			CmdNew.CmdBuf[0] = 0x00;
			CmdNew.CmdBuf[1] = 0x44;
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[2], mem_addr);
			UdsUtil::host_to_canl(&CmdNew.CmdBuf[6], mem_size);
			CmdNew.CmdLen = 10;
			push_cmd(CmdNew);
			break;
		case UDS_PROG_APP_DOWNLOADING:
		{
			UINT read_len;
			//Push cmd, Transfer Data
			CmdNew.SID = SID_36;
			CmdNew.CmdBuf[0] = block_sn;
			//block_len = app_data_read(&CmdNew.CmdBuf[1], block_len);
			CmdNew.CmdLen = block_len + 2;
			push_cmd(CmdNew);
		}
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
		case UDS_PROG_WRITE_DIDF199:
		{
			CmdNew.SID = SID_2E;
			CmdNew.CmdBuf[0] = 0xF1;
			CmdNew.CmdBuf[1] = 0x98;
			memcpy(&CmdNew.CmdBuf[2], tester_sn, 10);
			CmdNew.CmdLen = 12;
			dlg->UdsClient.push_cmd(CmdNew);
			break;
		}
		case UDS_PROG_ECU_RESET:
			break;
#endif
		case UDS_PROG_NONE:
		default:
			break;
	}
}
/*******************************************************************************
Function  Definition - common
*******************************************************************************/

void CUdsClient::ZTai_UDS_Send(BYTE CanData[], BYTE CanDlc)
{
	VCI_CAN_OBJ SendObj[1];

	int FrameFormat, FrameType;
	UINT i;

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
		upgrade_step = UDS_PROG_NONE;
		ret |= RET_TIMOUT_P2;
	}
	if (uds_timer_run(UDS_TIMER_P2client_x) < 0)
	{
		m_GetRsp = FALSE;
		m_ReqSid = 0;
		m_UpdRsp = 0;
		/* Stop upgrade */
		upgrade_step = UDS_PROG_NONE;
		ret |= RET_TIMOUT_P2_x;
	}
	/* Handle Cmd list */
	do_cmdlist();

	if (do_response(RspData, RspDlc) != 0)
	{
		uds_timer_stop(UDS_TIMER_S3client);
		ret |= RET_RESPONSE;
	}
	if (upd_ticks >= TIMOUT_UPGRAD_MS)
	{
		upd_ticks = 0;
		/* Handle uds upgrade */
		do_upgrade();
	}
	else
	{
		upd_ticks++;
	}
	if (upgrade_step == UDS_PROG_NONE)
	{
		if (m_FileOpen == TRUE) {
			myFile.Close();
			m_FileOpen = FALSE;
		}
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
	Ticks = GetTickCount();
	while (GetRsp == FALSE)
	{
		if ((GetTickCount() - Ticks) >= 2000)
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
	if (upgrade_step != UDS_PROG_NONE)
		return 1;
	m_UpdRsp = TRUE;
	upgrade_step = UDS_PROG_EXTENDED_SESSION;
	upd_ticks = 0;
	return 0;
}

/**
* open_upgrade_file - Open the upgrade file
*
* @void  :
*
* returns:
*     void
*/
UINT CUdsClient::open_upgrade_file(CString FilePath)
{

	CFileException fileException;

	if (myFile.Open(FilePath, (CFile::typeText | CFile::modeReadWrite), &fileException))
	{
		myFile.SeekToBegin();

		CString str;
		myFile.ReadString(str);
		AfxMessageBox(str);

		m_FileOpen = TRUE;
	}
	else
	{
		m_FileOpen = FALSE;
	}

	return fileException.m_cause;

}