/*
 * This is the c-port of the php-code for generating gold-cards, which has
 * been published in the xda forum.
 * This port uses libtomcrypt for the 3DES-encryption.
 */
#include <stdio.h>
#include <stdint.h>
#include <tomcrypt.h>

#define GCBUFSIZE	0x180


static const uint8_t timeval[] =
    {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

static const uint32_t deviceid = 0x21000006;
static const uint8_t* deviceid_ary = (uint8_t*)&deviceid;

static const uint32_t crc32_table[] = {
	0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
	0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
	0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
	0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
	0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
	0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
	0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
	0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
	0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
	0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
	0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
	0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
	0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
	0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
	0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
	0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
	0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
	0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
	0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
	0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
	0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
	0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
	0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
	0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
	0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
	0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
	0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
	0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
	0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
	0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
	0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
	0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
	0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
	0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
	0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
	0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
	0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
	0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
	0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
	0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
	0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
	0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
	0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
	0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
	0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
	0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
	0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
	0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
	0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
	0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
	0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
	0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
	0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
	0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
	0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
	0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
	0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
	0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
	0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
	0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
	0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
	0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
	0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
	0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

static const uint8_t val114_table1[] =
    {0x16, 0x2D, 0x03, 0x39, 0x44, 0x4E, 0x21, 0x09};
static const uint8_t val114_table2[] =
    {0x56, 0x19, 0x47, 0x05, 0x32, 0x22, 0x0C, 0x3A};
static const uint8_t val114_table3[] =
    {0x37, 0x46, 0x1C, 0x25, 0x06, 0x0F, 0x3E, 0x54};
static const uint8_t val84_table1[] =
    {0x40, 0x27, 0x12, 0x51, 0x49, 0x07, 0x1F, 0x38};

static uint32_t CalcCRC32HTC(uint8_t* buf, int pos, int filesize, uint32_t nval)
{
	uint32_t nCrc32 = nval;
	while (filesize) {
		nCrc32 =
		    ((nCrc32 >> 8) & 0xffffff) ^
		    crc32_table[(nCrc32 ^ (buf[pos])) & 0xFF];
		pos++;
		filesize--;
	}
	return nCrc32;
}

static void HTC3DES(int mode, const uint8_t* key, uint8_t* input, int loopval)
{
	int i;
	int err;
	symmetric_key skey;

	if (mode == 0) {
		for (i = 0; i < loopval; i++) {
			if ((err = des3_setup(key, 24, 0, &skey)) != CRYPT_OK) {
				printf("Setup error: %s\n", error_to_string(err));
				return;
			}
			des3_ecb_decrypt(input, input, &skey);
			des3_done(&skey);
		}
	}

	if (mode == 1) {
		for (i = 0; i < loopval; i++) {
			if ((err = des3_setup(key, 24, 0, &skey)) != CRYPT_OK) {
				printf("Setup error: %s\n", error_to_string(err));
				return;
			}
			des3_ecb_encrypt(input, input, &skey);
			des3_done(&skey);
		}
	}
}

static uint8_t htccalcval(uint32_t* val)
{
	uint32_t retval;
	uint32_t aval = 0x343FD;
	uint32_t bval = 0x269EC3;
	*val = *val * aval + bval;
	retval = *val;
	retval >>= 16;
	return (uint8_t)retval;
}

int genGc(uint8_t* cid_ary, int mode, uint8_t** gcbuf, int* size)
{
	int i;
	int loopval;
	uint32_t crc1;
	uint32_t crc2;
	uint32_t htcval;
	uint8_t* goldcardbuf = malloc(GCBUFSIZE);
	uint8_t R4;
	uint8_t R7;
	uint8_t R10;
	uint8_t R9;
	uint8_t firstcardid[8];
	uint8_t secondcardid[8];
	uint8_t	deviceidenc[8];
	uint8_t* baddress;
	uint8_t* deskey1_ary;

/*
 * These are dummy keys. For creating real gold-cards the real
 * keys must be inserted here.
 */
	uint8_t deskey1_android[8] = {0xef,
				      0xbe,
				      0xad,
				      0xde,
				      0xef,
				      0xbe,
				      0xad,
				      0xde
	};

	uint8_t deskey1_wm7[8] = {0xef,
				  0xbe,
				  0xad,
				  0xde,
				  0xef,
				  0xbe,
				  0xad,
				  0xde
	};

	uint8_t tripledeskey_ary[0x18] = {0x14,
					  0x99,
					  0x6D,
					  0x65,
					  0x58,
					  0xD0,
					  0x79,
					  0xF1,
					  0xE8,
					  0x2E,
					  0xA6,
					  0x4C,
					  0xC4,
					  0x6A,
					  0xE2,
					  0x85,
					  0x8F,
					  0x67,
					  0x3C,
					  0xB4,
					  0x6B,
					  0x13,
					  0x76,
					  0xF7
	};

	// Mode selection. Not implemented (yet)
	if (mode)
		deskey1_ary = deskey1_wm7;
	else
		deskey1_ary = deskey1_android;

	// Primary init
	cid_ary[0] = 0x00;

	for (i = 0; i < 8; i++)
		deskey1_ary[i] -= 0x60;

	// End primary init

	for (i = 0; i < 0x180; i++)
		goldcardbuf[i] = 0;

	for (i = 0; i < 4; i++)
		goldcardbuf[i + 0x100] = deviceid_ary[i];

	for (i = 0; i < 8; i++)
		goldcardbuf[i + 0x104] = timeval[i];

	// Write Header
	goldcardbuf[0x80] = 'S';
	goldcardbuf[0x81] = 'A';
	goldcardbuf[0x82] = '0';
	goldcardbuf[0x83] = '0';

	goldcardbuf[0x110] = 'S';
	goldcardbuf[0x111] = 'A';
	goldcardbuf[0x112] = '0';
	goldcardbuf[0x113] = '0';

	// Generate Seed
	htcval = 1;
	R4 = htccalcval(&htcval);
	R7 = htccalcval(&htcval);
	R10 = htccalcval(&htcval);
	R9 = htccalcval(&htcval);
	goldcardbuf[0xc7] = R4;
	goldcardbuf[0x99] = R7;
	goldcardbuf[0xD0] = R10;
	goldcardbuf[0xAF] = R9;
	
	/// Encrypt first eight bytes of Card ID
	loopval = R4 + 0xC80;
	loopval += 0xA;
	for (i = 0; i < 8; i++)
		firstcardid[i] = cid_ary[i];

	HTC3DES(1, tripledeskey_ary, firstcardid, loopval);

	for (i = 0; i < 8; i++)
		goldcardbuf[0x114 + val114_table1[i]] = firstcardid[i];

	/// Modify 3DES key
	for (i = 0; i < 3; i++) {
		int j;
		for (j = 0; j < 8; j++) {
			tripledeskey_ary[j + i * 8] =
				(tripledeskey_ary[j + i * 8] - 0x79);
		}
	}

	/// Second 3DES loop
	loopval = R7 + 0xE80;
	loopval += 0x9;

	for (i = 8; i < 16; i++)
		secondcardid[i - 8] = cid_ary[i];

	HTC3DES(1, tripledeskey_ary, secondcardid, loopval);

	for (i = 0; i < 8; i++)
		goldcardbuf[0x114 + val114_table2[i]] = secondcardid[i];

	/// Modify 3DES key (second loop)
	for (i = 0; i < 3; i++) {
		int j;
		for (j = 0; j < 8; j++) {
			tripledeskey_ary[j + i * 8] =
			    tripledeskey_ary[j + i * 8] ^ 0x5A;
		}
	}

	/// Third 3DES loop
	loopval = R10 + 0xFB0;
	loopval += 0xA;

	/// Device ID Encryption
	for (i = 0; i < 8; i++)
		deviceidenc[i] = (deviceid >> (i << 2)) & 0xFF;

	HTC3DES(1, tripledeskey_ary, deviceidenc, loopval);

	for (i = 0; i < 8; i++)
		goldcardbuf[0x114 + val114_table3[i]] = deviceidenc[i];

	/// Fourth 3DES loop
	loopval = R9 + 0x1D00;
	loopval += 0xE;

	/// DES Key Encryption
	for (i = 0; i < 8; i++)
		tripledeskey_ary[i] = firstcardid[i];

	for (i = 8; i < 16; i++)
		tripledeskey_ary[i] = secondcardid[i - 8];

	for (i = 16; i < 24; i++)
		tripledeskey_ary[i] = deviceidenc[i - 16];

	HTC3DES(1, tripledeskey_ary, deskey1_ary, loopval);

	for (i = 0; i < 8; i++)
		goldcardbuf[0x84 + val84_table1[i]] = deskey1_ary[i];

	crc1 = CalcCRC32HTC(goldcardbuf, 0x80, 0xDC - 0x80, 0);
	baddress = (uint8_t*)&crc1;
	for (i = 0; i < 4; i++)
		goldcardbuf[0xDC + i] = baddress[i];

	crc2 = CalcCRC32HTC(goldcardbuf, 0x110, 0x16C - 0x110, 0);
	baddress = (uint8_t*)&crc2;
	for (i = 0; i < 4; i++)
		goldcardbuf[0x16C + i] = baddress[i];

	// Store File Buffer
	*size = GCBUFSIZE;
	*gcbuf = goldcardbuf;

	return 0;
}
