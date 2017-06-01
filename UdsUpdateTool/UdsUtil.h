#pragma once

/* SECURITYACCESS */
#define UNLOCKKEY					0x00000000
#define UNLOCKSEED					0x00000000
#define UNDEFINESEED				0xFFFFFFFF
#define SEEDMASK					0x80000000
#define SHIFTBIT					1
#define ALGORITHMASK				0x4D313232


/* CRC */
#define CRC32_INIT 0xFFFFFFFFul
#define CRC32_POLY 0x04c11db7ul

class UdsUtil
{
public:
	UdsUtil();
	~UdsUtil();
public:
	static LONG str2HEX(PBYTE src_str, PBYTE dst_buf);
	static INT  str2char(CString str, PBYTE ptr, UINT dst_len);
	static LONG str2DEC(PBYTE src_str, PBYTE dst_buf);
	static UINT seedTOKey(UINT Seed);
	static void KeyCalcu(PBYTE SeedBuf, PBYTE KeyBuf);
	static INT  host_to_canl(BYTE buf[], UINT val);
	static UINT can_to_hostl(BYTE buf[]);
	static void ascii_to_hex(unsigned char *src_buff, unsigned char *dst_buff, int src_len);
	static UINT crc32_continue(BYTE *data, UINT len);
	static UINT crc32_discontinue(UINT org_rst, BYTE *data, UINT len);
	static BYTE BCD2HEX(UINT bcd_data);
	static UINT HEX2BCD(BYTE hex_data);
};

