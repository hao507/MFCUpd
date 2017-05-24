#pragma once
#include "NetworkLayer.h"
#include "uds_program.h"

#define BUF_LEN       1024

typedef enum __UDS_NRC_ENUM__
{
	NRC_NONE = 0,
	NRC_GENERAL_REJECT = 0x10,
	NRC_SERVICE_NOT_SUPPORTED = 0x11,
	NRC_SUBFUNCTION_NOT_SUPPORTED = 0x12,
	NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT = 0x13,
	NRC_CONDITIONS_NOT_CORRECT = 0x22,
	NRC_REQUEST_SEQUENCE_ERROR = 0x24,
	NRC_REQUEST_OUT_OF_RANGE = 0x31,
	NRC_SECURITY_ACCESS_DENIED = 0x33,
	NRC_INVALID_KEY = 0x35,
	NRC_EXCEEDED_NUMBER_OF_ATTEMPTS = 0x36,
	NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED = 0x37,
	NRC_TRANSFER_DATA_SUSPENDED = 0x71,
	NRC_GENERAL_PROGRAMMING_FAILURE = 0x72,
	NRC_WRONG_BLOCK_SEQUENCE_COUNTER = 0x73,
	NRC_SERVICE_BUSY = 0x78,
	NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION = 0x7F,
}uds_nrc_em;

#define FUNCID 0x7DF
#define PHYSID 0x766
#define RSPNID 0x706

#define FUNCID_STR _T("0x7DF")
#define PHYSID_STR _T("0x766")
#define RSPNID_STR _T("0x706")

#define SID_10        (0x10) /* SessionControl */
#define SID_11        (0x11) /* ECUReset */
#define SID_14        (0x14) /* ClearDTC */
#define SID_18        (0x18) /* KWPReadDTC */
#define SID_19        (0x19) /* ReadDTC */
#define SID_22        (0x22) /* ReadID */
#define SID_27        (0x27) /* SecurityAccess */
#define SID_2E        (0x2E) /* WriteID */
#define SID_2F        (0x2F) /* InputOutputControlID */
#define SID_28        (0x28) /* CommunicationControl */
#define SID_31        (0x31) /* RoutineControl */
#define SID_34        (0x34) /* RequestDownload */
#define SID_36        (0x36) /* TransferData */
#define SID_37        (0x37) /* RequestTransferExit */
#define SID_3E        (0x3E) /* TesterPresent */
#define SID_85        (0x85) /* ControlDTCSetting */

#define SID_10_MIN_LEN      (0x02u)
#define SID_11_MIN_LEN      (0x02u)
#define SID_27_MIN_LEN      (0x02u)
#define SID_28_MIN_LEN      (0x03u)
#define SID_3E_MIN_LEN      (0x02u)
#define SID_85_MIN_LEN      (0x02u)
#define SID_22_MIN_LEN      (0x03u)
#define SID_2E_MIN_LEN      (0x02u)
#define SID_14_MIN_LEN      (0x04u) /* 3 Bytes DTC */
#define SID_19_MIN_LEN      (0x02u)
#define SID_2F_MIN_LEN      (0x04u)
#define SID_31_MIN_LEN      (0x04u)



#define POSITIVE_RSP 			0x40
#define NEGATIVE_RSP 			0x7F
#define USD_GET_POSITIVE_RSP(rsp_id)         (rsp_id - POSITIVE_RSP)

#define UDS_GET_SUB_FUNCTION_SUPPRESS_POSRSP(byte)    ((byte >> 7u)&0x01u)
#define UDS_GET_SUB_FUNCTION(byte)     (byte & 0x7fu)

#define TIMEOUT_FSA          (10000) /* 10s */
#define TIMEOUT_S3client     (3000)  /* 3000ms */
#define TIMEOUT_P2client     (150)   /* 150ms , according to m12 Diagnostic Specification */
#define TIMEOUT_P2client_x   (5000)  /* 5000ms, according to m12 Diagnostic Specification */
/* uds app layer timer */
typedef enum __UDS_TIMER_T__
{
	UDS_TIMER_FSA = 0,
	UDS_TIMER_S3client,
	UDS_TIMER_P2client,
	UDS_TIMER_P2client_x,
	UDS_TIMER_CNT
}uds_timer_t;

/* uds diagnostic session */
typedef enum __UDS_SESSION_T_
{
	UDS_SESSION_NONE = 0,
	UDS_SESSION_STD,
	UDS_SESSION_PROG,
	UDS_SESSION_EOL
}uds_session_t;

/* uds security access level */
typedef enum __UDS_SA_LV__
{
	UDS_SA_NON = 0,
	UDS_SA_LV1,
	UDS_SA_LV2,
}uds_sa_lv;

typedef struct
{
	UINT SID;
	UINT CmdLen; // The unit is millisecond
	BYTE CmdBuf[BUF_LEN];
}UdsCmd, *PUdsCmd;

#define GETRSP_MS 2000
#define TIMOUT_MS 20
#define GETRSP_CNT (GETRSP_MS/TIMOUT_MS)

#define TIMOUT_RECV_MS 5

#define TIMOUT_UPGRAD_MS 500

#define RET_TIMOUT_S3    0x01
#define RET_TIMOUT_P2    0x02
#define RET_TIMOUT_P2_x  0x04
#define RET_RESPONSE     0x08
#define RET_DONE         0x10


/* for read hex file */
#define RECORD_BUF_LEN	200
#define DATA_BUF_LEN	200
/*******************************************************************************
Function  Definition
*******************************************************************************/

class CUdsClient : public CUdsNetwork
{
public:
	CUdsClient();
	~CUdsClient();

private:
	/* uds user layer timer */
	DWORD uds_timer[UDS_TIMER_CNT];
	DWORD TIMOUT_VALUE[UDS_TIMER_CNT];
	BYTE  uds_timeo[UDS_TIMER_CNT];

	BOOL m_FunReq; /* Functional communication */
	BYTE m_ReqSid;
	BYTE m_ReqSubfunction;
	BYTE m_ReqSsp;
	BOOL m_UpdRsp; /* Recieve a upgrade response or not */
	BYTE m_RspSid;
	BYTE m_RspSubfunction;
	BYTE RspData[BUF_LEN];
	UINT RspDlc;

	CStdioFile myFile;

	/* for upgrade statu management */
	uds_upgrade_t upgrade_status;

	LPBYTE pdri_buf;
	UINT dri_crc32;
	UINT dri_addr;
	UINT dri_size;

	LPBYTE papp_buf;
	UINT app_crc32;
	UINT app_addr;
	UINT app_size;

	BYTE recv_sn;
	UINT total_xmit_len;

	UINT block_len;
	BYTE block_sn;
	BYTE tester_sn[10];
	BYTE ecu_part_num[15];
	BYTE cur_seesion;

public:
	UINT m_Fucid = FUNCID;
	UINT m_Phyid = PHYSID;
	UINT m_Rspid = RSPNID;

	BYTE n_Result;
	BOOL n_ResultErr;
	CArray<UdsCmd> m_CmdList;
	BOOL m_GetRsp;

	BYTE m_RspBuf[4];
	BYTE m_RspNrc;

	UINT m_CanChnl;

	UINT total_step;
	UINT curre_step;

	UINT m_EntBoot;
private:
	void uds_timer_start(BYTE num);
	void uds_timer_stop(BYTE num);
	int  uds_timer_run(BYTE num);
	int  uds_timer_chk(BYTE num);

	void do_cmdlist(void);
	void do_upgrade(void);
	BYTE do_response(BYTE msg_buf[], WORD msg_dlc);
	void do_entboot(void);
	INT  read_aline(unsigned char *dst_buff, UINT buff_len);
	INT  read_memaddr(UINT &mem_addr, UINT &mem_size);
	INT  read_memdata(LPBYTE mem_buf, UINT mem_addr, UINT mem_size);
	int  read_block(const LPBYTE mem_buff, UINT mem_size, unsigned char *dst_buff, UINT block_len);
protected:
	void ZTai_UDS_Send(BYTE CanData[], BYTE CanDlc);
	void N_USData_ffindication(WORD msg_dlc);
	void N_USData_indication(BYTE msg_buf[], WORD msg_dlc, n_result_t n_result);
	void N_USData_confirm(n_result_t n_result);
public:
    BYTE main_loop(void);
	void request(BYTE SvcId, BYTE DidBuf[], UINT DidLen);
	UINT get_rsp(BYTE DataBuf[], UINT BufLen);
	void push_cmd(UdsCmd CmdNew);
	UINT start_upgrade(void);
	UINT stop_upgrade(void);
	UINT open_mcuapp_file(CString FilePath);
	UINT open_driver_file(CString FilePath);
};
