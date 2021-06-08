#ifndef _SMA4SAVETOOL_H
#define _SMA4SAVETOOL_H

#include "types.h"

// Defines
#define MAX_LEVEL_RECORDS 72						// Maximum number of level records
#define MAX_LEVEL_DATA 33							// Maximum number of level data
#define MAX_NAME_SIZE 21							// Max level name size
#define MAX_DATA_SIZE 2024							// Max compressed level data size
#define MAX_SAVE_SIZE 0x20000						// Max save data size
#define MAX_INPUT_SIZE 0x64000						// Max input buffer size
#define MAX_ECOIN_TABLE 24							// e-coin table size
#define MAX_ECOIN_A_SIZE 0x120						// e-coin data A size
#define MAX_ECOIN_B_SIZE 0x20						// e-coin data B size

#define INPUT_NOT_COMPRESSED 0						// Input data is not compressed
#define INPUT_ALREADY_COMPRESSED 1					// Input data is already compressed
#define INPUT_ECARD_BIN 2							// Input data is e-card .bin file

#define OFFSET_UNFINISHED 0x29						// Current unfinished level record ID
#define OFFSET_ACECOIN 0x30							// Ace coins available/collected flag
#define OFFSET_NAME 0x80							// Level name table
#define OFFSET_ECOIN 0x668							// e-coins available/collected flag
#define OFFSET_PLAYABLE 0x6B0						// Level playable flag
#define OFFSET_DATAIDLIST 0x6D4						// Level data ID list
#define OFFSET_LEVELINFO 0x740						// Level info (Class, level number, and icon)

#define OFFSET_CLEARED_MARIO 0x6B9					// Level completed by Mario flag
#define OFFSET_CLEARED_LUIGI 0x6C3					// Level completed by Luigi flag
#define OFFSET_PERFECT_ACECOIN 0x730				// All ace coins collected in single play flag

#define OFFSET_ECOIN_A 0x4040						// e-coin data A (0x120)
#define OFFSET_ECOIN_B 0x5B40						// e-coin data B (0x20)

#define OFFSET_CURRENT_CHARACTER 0x727				// Current character
#define OFFSET_MARIO_MAP_POS_X 0x728				// Map X position (Mario)
#define OFFSET_MARIO_MAP_POS_Y 0x72C				// Map Y position (Mario)
#define OFFSET_LUIGI_MAP_POS_X 0x72A				// Map X position (Luigi)
#define OFFSET_LUIGI_MAP_POS_Y 0x72E				// Map Y position (Luigi)

#define BITTEST(v,n) ((v) & (1 << (n)))				// Get the specific bit status
#define BITSET(v,n) v |= (1 << (n))					// Set the specific bit to 1
#define BITZERO(v,n) v &= ~(1 << (n))				// Set the specific bit to 0

// e-level info struct
struct ELevelInfo
{
	u8 name[MAX_NAME_SIZE];
	u8 levelClass;
	u8 levelNumber;
	u8 icon;
	u8 eCoinID;
	u8 aceCoinTotal;
};

// e-level record struct
struct ELevelRecord
{
	// Main data
	ELevelInfo info;
	u8 playable;
	u8 dataID;
	
	// Progess data
	u8 eCoinCollected;
	u8 aceCoinCollectedFlag;
	u8 aceCoinPerfect;
	u8 clearedByMario;
	u8 clearedByLuigi;
};

// e-level data struct
struct ELevelData
{
	ELevelInfo info;
	u8 recordID;
	u8 ecoinB[MAX_ECOIN_B_SIZE];
	u8 ecoinA[MAX_ECOIN_A_SIZE];
	u8 data[MAX_DATA_SIZE];
};

#endif // _SMA4SAVETOOL_H
