#include "stdafx.h"
#include "UdsUtil.h"


UdsUtil::UdsUtil()
{
}


UdsUtil::~UdsUtil()
{
}


LONG UdsUtil::str2char(CString str, PBYTE ptr)

{

	if (ptr == NULL) return 0;

#ifdef _UNICODE

	LONG len;

	len = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);

	WideCharToMultiByte(CP_ACP, 0, str, -1, (LPSTR)ptr, len + 1, NULL, NULL);

#else
	LONG len;

	len = str.GetAllocLength();

	sprintf(ptr, _T("%s"), str);

#endif

	return len;

}

LONG UdsUtil::str2HEX(PBYTE src_str, PBYTE dst_buf)
{

	BYTE hex;
	BYTE char_tmp;
	LONG i, len, dst_pos;

	len = strlen((char *)src_str);

	if (src_str[0] == '0' && (src_str[1] == 'x' || src_str[1] == 'X'))
	{
		len -= 2;
		src_str += 2;
	}
	dst_pos = 0;

	if (len % 2)
	{
		for (i = 0; i < len; i++)
		{

			char_tmp = src_str[i];

			if (' ' == char_tmp || '\n' == char_tmp || '\t' == char_tmp || '\r' == char_tmp)
				break;

			if (char_tmp >= '0' && char_tmp <= '9')
				hex = char_tmp - '0';

			if (char_tmp >= 'a' && char_tmp <= 'z')
				hex = char_tmp - 'a' + 10;

			if (char_tmp >= 'A' && char_tmp <= 'Z')
				hex = char_tmp - 'A' + 10;

			dst_pos = (i + 1) / 2;

			if (i % 2)
			{
				dst_buf[dst_pos] = hex << 4;
			}
			else
			{
				dst_buf[dst_pos] |= hex;
			}
		}
	}
	else
	{
		for (i = 0; i < len; i++)
		{

			char_tmp = src_str[i];

			if (' ' == char_tmp || '\n' == char_tmp || '\t' == char_tmp || '\r' == char_tmp)
				break;

			if (char_tmp >= '0' && char_tmp <= '9')
				hex = char_tmp - '0';

			if (char_tmp >= 'a' && char_tmp <= 'z')
				hex = char_tmp - 'a' + 10;

			if (char_tmp >= 'A' && char_tmp <= 'Z')
				hex = char_tmp - 'A' + 10;

			dst_pos = i / 2;

			if (!(i % 2))
			{
				dst_buf[dst_pos] = hex << 4;
			}
			else
			{
				dst_buf[dst_pos] |= hex;
			}

		}
	}


	return dst_pos + 1;
}

LONG UdsUtil::str2DEC(PBYTE src_str, PBYTE dst_buf)
{

	BYTE hex;
	BYTE char_tmp;
	LONG i, len;

	len = strlen((char *)src_str);


	for (i = 0; i < len; i++)
	{

		char_tmp = src_str[i];

		if (' ' == char_tmp || '\n' == char_tmp || '\t' == char_tmp || '\r' == char_tmp)
			break;

		if (char_tmp >= '0' && char_tmp <= '9')
			hex = char_tmp - '0';
		else
			hex = 0;

		dst_buf[i] = hex;

	}

	return len;
}

UINT UdsUtil::seedTOKey(UINT Seed)
{
	BYTE i;
	UINT key;
	key = UNLOCKKEY;
	if (!((Seed == UNLOCKSEED)
		|| (Seed == UNDEFINESEED)))
	{
		for (i = 0; i < 35; i++)
		{
			if (Seed & SEEDMASK)
			{
				Seed = Seed << SHIFTBIT;
				Seed = Seed ^ ALGORITHMASK;
			}
			else
			{
				Seed = Seed << SHIFTBIT;
			}
		}
		key = Seed;
	}
	return key;
}


void UdsUtil::KeyCalcu(PBYTE SeedBuf, PBYTE KeyBuf)
{
	UINT Seed;
	UINT Key;
	Seed = 0;
	Seed |= ((UINT)SeedBuf[0]) << 24;
	Seed |= ((UINT)SeedBuf[1]) << 16;
	Seed |= ((UINT)SeedBuf[2]) << 8;
	Seed |= ((UINT)SeedBuf[3]) << 0;

	Key = seedTOKey(Seed);


	KeyBuf[0] = (Key >> 24) & 0xff;
	KeyBuf[1] = (Key >> 16) & 0xff;
	KeyBuf[2] = (Key >> 8) & 0xff;
	KeyBuf[3] = (Key >> 0) & 0xff;
}

/**
* host_to_canl - transmit a long or short int to can-net endian
*
* @buf: ther buffer to storage the result
* @val: the value to be transformed
*
* returns:
*     0 - ok, -1 - err
*/
INT
UdsUtil::host_to_canl(BYTE buf[], UINT val)
{
	if (buf == NULL) return -1;

	buf[0] = (val >> 24) & 0xff;
	buf[1] = (val >> 16) & 0xff;
	buf[2] = (val >> 8) & 0xff;
	buf[3] = (val >> 0) & 0xff;

	return 0;
}
