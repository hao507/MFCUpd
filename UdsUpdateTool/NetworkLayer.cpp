/***************************************************************************//**
\file          NetworkLayer.c
\author        huanghai
\mail          huanghai@auto-link.com
\version       0.02
\date          2016-09-24
\description   uds network code, base on ISO 15765
*******************************************************************************/
#include "stdafx.h"
#include "NetworkLayerPrivate.h"
#include "NetworkLayer.h"
#include "timeapi.h"

/*******************************************************************************
Type Definition
*******************************************************************************/



/*******************************************************************************
Function Constructor Destructor
*******************************************************************************/
CUdsNetwork::CUdsNetwork()
{
	nwl_st = NWL_IDLE;

	g_wait_cf = FALSE;
	g_wait_fc = FALSE;

	for (int i = 0; i < TIMER_CNT; i++)
	    nt_timer[i] = 0;

	g_rfc_stmin = 0;    /* received flowcontrol SeparationTime */
	g_rfc_bs = 0;       /* received flowcontrol block size */

	g_xcf_bc = 0;       /* transmit consecutive frame block counter */
	g_xcf_sn = 0;       /* transmit consecutive frame SequenceNumber */
	g_rcf_bc = 0;       /* received frame block counter */
	g_rcf_sn = 0;       /* received consecutive frame SequenceNumber */

	remain_len = 0;
	remain_pos = 0;

	recv_len = 0;
	recv_fdl = 0;
}


CUdsNetwork::~CUdsNetwork()
{
}

/**
* nt_timer_start - start network timer
*
* void :
*
* returns:
*     void
*/
void CUdsNetwork::nt_timer_start(BYTE num)
{
	if (num >= TIMER_CNT) return;

	if (num == TIMER_N_CR)
	{
		nt_timer[TIMER_N_CR] = timeGetTime();
		NT_TIMOUT[TIMER_N_CR] = TIMEOUT_N_CR;
	}
	if (num == TIMER_N_BS)
	{
		nt_timer[TIMER_N_BS] = timeGetTime();
		NT_TIMOUT[TIMER_N_BS] = TIMEOUT_N_BS;
	}
	if (num == TIMER_STmin)
	{
		nt_timer[TIMER_STmin] = timeGetTime();
		NT_TIMOUT[TIMER_STmin] = g_rfc_stmin;
	}
}

void CUdsNetwork::nt_timer_start_wv(BYTE num, UINT value)
{
	if (num >= TIMER_CNT) return;

	if (num == TIMER_N_CR)
	{
		nt_timer[TIMER_N_CR] = timeGetTime();
		NT_TIMOUT[TIMER_N_CR] = value;
	}
	if (num == TIMER_N_BS)
	{
		nt_timer[TIMER_N_BS] = timeGetTime();
		NT_TIMOUT[TIMER_N_BS] = value;
	}
	if (num == TIMER_STmin)
	{
		nt_timer[TIMER_STmin] = timeGetTime();
		NT_TIMOUT[TIMER_STmin] = value;
	}
}

void CUdsNetwork::nt_timer_stop(BYTE num)
{
	if (num >= TIMER_CNT) return;

	nt_timer[num] = 0;
}

/**
* nt_timer_run - run a network timer, should be invoked per 1ms
*
* void :
*
* returns:
*     0 - timer is not running, 1 - timer is running, -1 - a timeout occur
*/
int CUdsNetwork::nt_timer_run(BYTE num)
{
	if (num >= TIMER_CNT) return 0;

	if (nt_timer[num] == 0)
	{
		return 0;
	}
	else
	{
		DWORD Tikcs = timeGetTime();

		if (Tikcs - nt_timer[num] >= NT_TIMOUT[num])
		{
			nt_timer[num] = 0;
			return -1;
		}
		else
		{
			return 1;
		}
	}

}

/**
* nt_timer_chk - check a network timer and stop it
*
* num :
*
* returns:
*     0 - timer is not running, 1 - timer is running,
*/
int CUdsNetwork::nt_timer_chk(BYTE num)
{
	if (num >= TIMER_CNT) return 0;

	if (nt_timer[num] > 0)
	{
		nt_timer[num] = 0; /* stop timer */
		return 1;
	}
	else
	{
		nt_timer[num] = 0; /* stop timer */
		return 0;
	}
}


/**
* clear_network - clear network status
*
* void :
*
* returns:
*     void
*/
void CUdsNetwork::clear_network(void)
{
	BYTE num;
	nwl_st = NWL_IDLE;
	g_wait_cf = FALSE;
	g_wait_fc = FALSE;
	g_xcf_bc = 0;
	g_xcf_sn = 0;
	g_rcf_bc = 0;
	g_rcf_sn = 0;

	for (num = 0; num < TIMER_CNT; num++)
		nt_timer_stop(num);
}

/*******************************************************************************
Function  Definition - recieve
*******************************************************************************/
/**
* recv_singleframe - recieved a single frame from CAN
*
* @frame_buf : uds can frame data buffer
* @frame_dlc : uds can frame length
*
* returns:
*     void
*/
void CUdsNetwork::recv_singleframe(BYTE frame_buf[], BYTE frame_dlc)
{
	WORD i, uds_dlc;
	BYTE service_id;

	uds_dlc = NT_GET_SF_DL(frame_buf[0]);

	service_id = frame_buf[1];

	/************************************/

#ifdef UDS_CAN_ID_STD
	if (uds_dlc > 7 || uds_dlc == 0)
		return;
#else
	if (uds_dlc > 6 || uds_dlc == 0)
		return;
#endif

	recv_fdl = uds_dlc;
	for (i = 0; i < frame_dlc - 1; i++)
		recv_buf[i] = frame_buf[1 + i];
	recv_len = frame_dlc - 1;

	N_USData_indication(recv_buf, recv_fdl, N_OK);
}
/**
* recv_firstframe - recieved a firt frame from CAN
*
* service : L_Data.indication (FF)
* @frame_buf : uds can frame data buffer
* @frame_dlc : uds can frame length
*
* returns:
*     0 - recv a right frame, other - err
*/
int CUdsNetwork::recv_firstframe(BYTE frame_buf[], BYTE frame_dlc)
{
	WORD i;
	BYTE service_id;
	WORD uds_dlc;

	uds_dlc = ((WORD)(frame_buf[0] & 0x0f)) << 8;
	uds_dlc |= frame_buf[1];

	service_id = frame_buf[2];

	/************************************/

#ifdef UDS_CAN_ID_STD
	if (uds_dlc < 8)
		return -1;
#else
	if (uds_dlc < 7)
		return -1;
#endif
	/**
	* if FF_DL is greater than the available receiver buffer size
	* abort the message reception and send
	* an FC N_PDU with Overflow.
	*/
	if (uds_dlc > UDS_FF_DL_MAX) {
		send_flowcontrol(FS_OVFLW);
		return -2;
	}

	recv_fdl = uds_dlc;
	for (i = 0; i < frame_dlc - 2; i++)
		recv_buf[i] = frame_buf[2 + i];
	recv_len = frame_dlc - 2;

	/**
	* after received first frame,
	* send flowcontrol frame and wait consecutive frame,
	*/
	send_flowcontrol(FS_CTS);
	g_rcf_bc = 0;
	g_wait_cf = TRUE;
	nt_timer_start(TIMER_N_CR);
	/* claer the consecutive frane0 sn */
	g_rcf_sn = 0;

	N_USData_ffindication(uds_dlc);

	return 1;
}

/**
* recv_consecutiveframe - recieved a consecutive frame from CAN
*
* service: L_Data.indication (CF)
* @frame_buf : uds can frame data buffer
* @frame_dlc : uds can frame length
*
* returns:
*     0 - recv end, 1 - recv continue, other - err
*/
int CUdsNetwork::recv_consecutiveframe(BYTE frame_buf[], BYTE frame_dlc)
{
	BYTE cf_sn;
	WORD i;
	cf_sn = NT_GET_CF_SN(frame_buf[0]);

	/* if N_Cr timeout, Abort message transmission and issue N_TIMEOUT_Cr */
	if (nt_timer_chk(TIMER_N_CR) <= 0) return -1;

	g_rcf_sn++;
	if (g_rcf_sn > 0x0f)
		g_rcf_sn = 0;
	if (g_rcf_sn != cf_sn) {
		N_USData_indication(recv_buf, recv_len, N_WRONG_SN);
		return -2;
	}

	for (i = 0; i < UDS_CF_DL_COM; i++)
	{
		recv_buf[recv_len + i] = frame_buf[1 + i];
	}
	recv_len += UDS_CF_DL_COM;

	if (recv_len >= recv_fdl)
	{
		g_wait_cf = FALSE;
		N_USData_indication(recv_buf, recv_fdl, N_OK);
		return 0;
	}
	else
	{
		if (NT_XMIT_FC_BS > 0)
		{
			g_rcf_bc++;
			if (g_rcf_bc >= NT_XMIT_FC_BS)
			{
				/**
				* after NT_XMIT_FC_BS consecutive frames,
				* send flowcontrol frame and wait consecutive frame,
				*/
				send_flowcontrol(FS_CTS);
				g_rcf_bc = 0;
			}
		}

		g_wait_cf = TRUE;
		nt_timer_start(TIMER_N_CR);
		return 1;
	}
}

/**
* recv_flowcontrolframe - process uds flowc control frame
*
* service: L_Data.indication (FC)
* @frame_buf : uds can frame data buffer
* @frame_dlc : uds can frame length
*
* returns:
*     0 - recv CTS, 1 - recv WT, other - err
*/
int CUdsNetwork::recv_flowcontrolframe(BYTE frame_buf[], BYTE frame_dlc)
{
	BYTE fc_fs;

	fc_fs = NT_GET_FC_FS(frame_buf[0]);

	/**
	* if N_Bs timeout,
	* Abort message transmission and issue N_TIMEOUT_Bs,
	* if not timeout, stop the timer.
	*/
	if (nt_timer_chk(TIMER_N_BS) <= 0) return -1;

	g_wait_fc = FALSE;
	if (fc_fs >= FS_RESERVED) {
		N_USData_confirm(N_INVALID_FS);
		return -2;
	}

	if (fc_fs == FS_OVFLW) {
		N_USData_confirm(N_BUFFER_OVFLW);
		return -3;
	}

	if (fc_fs == FS_WT) {
		g_wait_fc = TRUE;
		nt_timer_start(TIMER_N_BS);
		return 1;
	}

	/**
	* get the fc block size and stmin
	*/
	g_rfc_bs = frame_buf[1];
	if (frame_buf[2] <= 0x7f)
		g_rfc_stmin = frame_buf[2];
	else
		g_rfc_stmin = 0x7f; /* 127 ms */

							/* start to transmit consecutive frame */
	g_xcf_bc = 0;
	nt_timer_start_wv(TIMER_STmin, 1);

	return 0;
}

/*******************************************************************************
Function  Definition - send
*******************************************************************************/

/**
* send_flowcontrol - send flowcontrol frame
*
* service: L_Data.confirm (FC)
* @flow_st : flow status
*
* returns:
*     void
*/
void CUdsNetwork::send_flowcontrol(BYTE flow_st)
{
	BYTE send_buf[UDS_VALID_FRAME_LEN] = { 0 };
	send_buf[0] = NT_SET_PCI_TYPE_FC(flow_st);
	send_buf[1] = NT_XMIT_FC_BS;
	send_buf[2] = NT_XMIT_FC_STMIN;
	ZTai_UDS_Send(send_buf, UDS_VALID_FRAME_LEN);

}

/**
* send_singleframe - send a single frame msg
*
* @msg_buf : uds msg data buffer
* @msg_dlc : uds msg length
*
* returns:
*     void
*/
void CUdsNetwork::send_singleframe(BYTE msg_buf[], WORD msg_dlc)
{
	WORD i;
	BYTE send_buf[UDS_VALID_FRAME_LEN] = { 0 };

	if (msg_dlc == 0 || msg_dlc > UDS_SF_DL_MAX) return;

	send_buf[0] = NT_SET_PCI_TYPE_SF((BYTE)msg_dlc);
	for (i = 0; i < msg_dlc; i++)
		send_buf[1 + i] = msg_buf[i];

	N_USData_confirm(N_OK);

	ZTai_UDS_Send(send_buf, UDS_VALID_FRAME_LEN);

}


/**
* send_firstframe - send a first frame data
*
* service : L_Data.confirm (FF)
* @msg_buf : uds msg data buffer
* @msg_dlc : uds msg length
*
* returns:
*     int
*/
int CUdsNetwork::send_firstframe(BYTE msg_buf[], WORD msg_dlc)
{
	WORD i;
	BYTE send_buf[UDS_VALID_FRAME_LEN] = { 0 };

	if (msg_dlc < UDS_FF_DL_MIN || msg_dlc > UDS_FF_DL_MAX) return 0;

	send_buf[0] = NT_SET_PCI_TYPE_FF((BYTE)(msg_dlc >> 8));
	send_buf[1] = (BYTE)(msg_dlc & 0x00ff);
	for (i = 0; i < UDS_VALID_FRAME_LEN - 2; i++)
		send_buf[2 + i] = msg_buf[i];

	ZTai_UDS_Send(send_buf, UDS_VALID_FRAME_LEN);

	/**
	* start N_Bs and wait for a fc.
	*/
	g_wait_fc = TRUE;
	nt_timer_start(TIMER_N_BS);

	return UDS_VALID_FRAME_LEN - 2;
}


/**
* send_consecutiveframe - send consecutive frame data
*
* service : L_Data.confirm (CF)
* @msg_buf : uds msg data buffer
* @msg_dlc : uds msg length
*
* returns:
*     int
*/
int CUdsNetwork::send_consecutiveframe(BYTE msg_buf[], WORD msg_dlc, BYTE frame_sn)
{
	WORD i;
	BYTE send_buf[UDS_VALID_FRAME_LEN] = { 0 };

	send_buf[0] = NT_SET_PCI_TYPE_CF(frame_sn);
	for (i = 0; i < msg_dlc && i < UDS_CF_DL_COM; i++)
		send_buf[1 + i] = msg_buf[i];
	for (; i < UDS_CF_DL_COM; i++)
		send_buf[1 + i] = 0;

	ZTai_UDS_Send(send_buf, UDS_VALID_FRAME_LEN);

	if (msg_dlc > UDS_CF_DL_COM)
		return UDS_CF_DL_COM;
	else
		return msg_dlc;
}
/**
* send_multipleframe - send a multiple frame msg
*
* @msg_buf : uds msg data buffer
* @msg_dlc : uds msg length
*
* returns:
*     void
*/
void CUdsNetwork::send_multipleframe(BYTE msg_buf[], WORD msg_dlc)
{
	WORD i;
	BYTE send_len;

	if (msg_dlc < UDS_FF_DL_MIN || msg_dlc > UDS_FF_DL_MAX) return;

	for (i = 0; i < msg_dlc; i++)
		remain_buf[i] = msg_buf[i];

	g_xcf_sn = 0;
	send_len = send_firstframe(msg_buf, msg_dlc);

	remain_pos = send_len;
	remain_len = msg_dlc - send_len;
}
/*******************************************************************************
Function  Definition - external API
*******************************************************************************/

/**
* network_main - network main task, should be schedule every one ms
*
* @void
*
* returns:
*     void
*/
void CUdsNetwork::network_main(void)
{
	BYTE send_len;
	if (nt_timer_run(TIMER_N_CR) < 0)
	{
		clear_network();
		N_USData_indication(recv_buf, recv_len, N_TIMEOUT_Cr);
	}
	if (nt_timer_run(TIMER_N_BS) < 0)
	{
		clear_network();
		N_USData_confirm(N_TIMEOUT_Bs);
	}

	if (nt_timer_run(TIMER_STmin) < 0)
	{
		g_xcf_sn++;
		if (g_xcf_sn > 0x0f)
			g_xcf_sn = 0;
		m_CriticalSection.Lock();
		send_len = send_consecutiveframe(&remain_buf[remain_pos], remain_len, g_xcf_sn);
		remain_pos += send_len;
		remain_len -= send_len;

		if (remain_len > 0)
		{
			if (g_rfc_bs > 0)
			{
				g_xcf_bc++;
				if (g_xcf_bc < g_rfc_bs)
				{
					nt_timer_start(TIMER_STmin);
				}
				else
				{
					/**
					* start N_Bs and wait for a fc.
					*/
					g_wait_fc = TRUE;
					nt_timer_start(TIMER_N_BS);
				}
			}
			else
			{
				nt_timer_start(TIMER_STmin);
			}
		}
		else
		{
			N_USData_confirm(N_OK);
			clear_network();
		}
		m_CriticalSection.Unlock();
	}
}
/**
* netowrk_recv_frame - recieved uds network can frame
*
* @func_addr : 0 - physical addr, 1 - functional addr
* @frame_buf : uds can frame data buffer
* @frame_dlc : uds can frame length
*
* returns:
*     void
*/
void CUdsNetwork::netowrk_recv_frame(BYTE func_addr, BYTE frame_buf[], BYTE frame_dlc)
{

	BYTE pci_type; /* protocol control information type */


					  /**
					  * The reception of a CAN frame with a DLC value
					  * smaller than expected shall be ignored by the
					  * network layer without any further action
					  */

	if (frame_dlc != UDS_VALID_FRAME_LEN) return;

	if (func_addr == 0)
		g_tatype = N_TATYPE_PHYSICAL;
	else
		g_tatype = N_TATYPE_FUNCTIONAL;

	m_CriticalSection.Lock();
	pci_type = NT_GET_PCI_TYPE(frame_buf[0]);
	switch (pci_type)
	{
	case PCI_SF:
		if (nwl_st == NWL_RECV || nwl_st == NWL_IDLE)
		{
			clear_network();
			if (nwl_st == NWL_RECV)
				N_USData_indication(recv_buf, recv_len, N_UNEXP_PDU);
			recv_singleframe(frame_buf, frame_dlc);
		}
		break;
	case PCI_FF:
		if (nwl_st == NWL_RECV || nwl_st == NWL_IDLE)
		{
			clear_network();
			if (nwl_st == NWL_RECV)
				N_USData_indication(recv_buf, recv_len, N_UNEXP_PDU);

			if (recv_firstframe(frame_buf, frame_dlc) > 0)
				nwl_st = NWL_RECV;
			else
				nwl_st = NWL_IDLE;
		}
		break;
	case PCI_CF:
		if (nwl_st == NWL_RECV && g_wait_cf == TRUE)
		{
			if (recv_consecutiveframe(frame_buf, frame_dlc) <= 0)
			{
				clear_network();
				nwl_st = NWL_IDLE;
			}
		}
		break;
	case PCI_FC:
		if (nwl_st == NWL_XMIT && g_wait_fc == TRUE)
			if (recv_flowcontrolframe(frame_buf, frame_dlc) < 0)
			{
				clear_network();
				nwl_st = NWL_IDLE;
			}
		break;
	default:
		break;
	}
	m_CriticalSection.Unlock();
}

/**
* netowrk_send_udsmsg - send a uds msg by can
*
* @msg_buf : uds msg data buffer
* @msg_dlc : uds msg length
*
* returns:
*     void
*/
void CUdsNetwork::netowrk_send_udsmsg(BYTE msg_buf[], WORD msg_dlc)
{

	if (msg_dlc == 0 || msg_dlc > UDS_FF_DL_MAX) return;

	if (msg_dlc <= UDS_SF_DL_MAX)
	{
		send_singleframe(msg_buf, msg_dlc);
	}
	else
	{
		nwl_st = NWL_XMIT;
		send_multipleframe(msg_buf, msg_dlc);
	}
}

/****************EOF****************/