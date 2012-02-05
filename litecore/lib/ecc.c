
//计算校验和
uint8_t cs8(const void *pData, uint_t nLen)
{
	uint_t nCS = 0;
	uint8_t *p = (uint8_t *)pData;

	for (; nLen; --nLen)
		nCS += *p++;
	return (uint8_t)nCS;
}

//计算校验和(双字节)
uint16_t cs16(const void *pData, uint_t nLen)
{
	uint16_t nCS = 0;
	uint16_t *p = (uint16_t *)pData;

	for (; nLen; --nLen)
		nCS += *p++;
	return (uint16_t)nCS;
}

const uint16_t ecc_tbl_CRC16[] = {
	0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401, 0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400,
};
uint16_t crc16(const void* pData, uint_t nLen)
{
	uint16_t nCRC = 0xFFFF;
	uint_t nChar;
	uint8_t *p = (uint8_t *)pData;

	for (; nLen; --nLen) {
		nChar = *p++;
		nCRC = ecc_tbl_CRC16[(nChar ^ nCRC) & 0x0F] ^ (nCRC >> 4);
		nCRC = ecc_tbl_CRC16[((nChar >> 4) ^ nCRC) & 0x0F] ^ (nCRC >> 4);
	}
	return nCRC;
}

