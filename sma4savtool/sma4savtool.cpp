#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "sma4savtool.h"
#include "compress.h"

/* Globals */
u8 saveData[MAX_SAVE_SIZE];								// SMA4 save data
u8 inputData[MAX_INPUT_SIZE];							// Level input data
u32 inputSize;											// Level input size

ELevelRecord levelRecords[MAX_LEVEL_RECORDS];			// Level records
ELevelData levelData[MAX_LEVEL_DATA];					// Saved level data

/* Prototypes */
int main(int argc, char *argv[]);
int readSaveData(char *filename);
int writeSaveData(char *filename);
int readInputData(char *filename);
int writeInputData(char *filename);
void initELevelInfo(ELevelInfo& info);
void initELevelRecord(ELevelRecord& record);
void initELevelData(ELevelData& level);
void loadLevelData();
void saveLevelData();
u32 getLevelDataAddress(u8 dataID);
void loadLevelRecords();
void saveLevelRecords();
u8 getLevelDataID(u8 recordID);
u8 getECoinID(u8 dataID);
u8 findFreeLevelDataID();
u8 findFreeRecordID();
u8 getHowManyFreeLevelData();
u8 getHowManyFreeRecord();
int compareLevelInfo(ELevelInfo& info1, ELevelInfo& info2);
int findSameInfoRecord(ELevelInfo& info);
char convertCharacterToASCII(u8 c);
void convertLevelNameToASCII(u8 *src, char *dest);
void resetCharacterMapPosition();
int getInputDataType();
int displayLevelList();
int addLevel(char *savename, char *filename, int unfinished);
int delLevel(char *savename, int recordID, int full);
int cleanupAllUnplayableLevels(char *savename);
int fixChecksum(char *savename);
void fixAllChecksum();
void calcAndFixChecksum(u32 offset, u32 size, u32 checksumAreaStart, char regionCode, u32 regionOffset);

/* Main function */
int main(int argc, char *argv[]) {
	// Show usage
	if((argc < 3) || (strcmp(argv[1],"-?") == 0) || (strcmp(argv[1],"/?") == 0)) {
		printf("SMA4 Save Data Tool version0.03 (c)2009 purplebridge001\n");
		printf("Compressor code by RANDY Ruler of Zexernet and caitsith2\n");
		printf("Decompressor code by Parasyte and Bouche\n");
		printf("Checksum code by caitsith2\n");
		printf("\n");
		
		printf("Usage: %s [Mode] [Save data] (3rd argument)\n", argv[0]);
		printf("Mode:\n");
		printf("-a Add level (3rd argument is level data)\n");
		printf("-u Add level as unfinished level (3rd argument is level data)\n");
		printf("-d Delete a level completely (3rd argument is level record ID)\n");
		printf("-r Delete a level data, but not the record (3rd argument is level record ID)\n");
		printf("-c Remove all unplayable levels (3rd argument is ignored)\n");
		printf("-f Fix all checksums (3rd argument is ignored)\n");
		printf("-l List all levels (3rd argument is ignored)\n");
		
		return -1;
	}
	
	// Read save data file
	if(!readSaveData(argv[2])) {
		printf("Couldn't read save data from %s\n", argv[2]);
		return -1;
	}
	
	loadLevelData();
	loadLevelRecords();
	
	if((strcmp(argv[1],"-a") == 0) || (strcmp(argv[1],"/a") == 0)) {
		// Add
		if(argc < 4) {
			printf("Missing 3rd argument (Level file)\n");
			return -1;
		} else {
			return addLevel(argv[2], argv[3], 0);
		}
	}
	else if((strcmp(argv[1],"-u") == 0) || (strcmp(argv[1],"/u") == 0)) {
		// add as an Unfinished level
		if(argc < 4) {
			printf("Missing 3rd argument (Level file)\n");
			return -1;
		} else {
			return addLevel(argv[2], argv[3], 1);
		}
	}
	else if((strcmp(argv[1],"-d") == 0) || (strcmp(argv[1],"/d") == 0)) {
		// Delete full
		if(argc < 4) {
			printf("Missing 3rd argument (Record ID)\n");
			return -1;
		} else {
			return delLevel(argv[2], atoi(argv[3]), 1);
		}
		return 0;
	}
	else if((strcmp(argv[1],"-r") == 0) || (strcmp(argv[1],"/r") == 0)) {
		// Delete level only
		if(argc < 4) {
			printf("Missing 3rd argument (Record ID)\n");
			return -1;
		} else {
			return delLevel(argv[2], atoi(argv[3]), 0);
		}
		return 0;
	}
	else if((strcmp(argv[1],"-c") == 0) || (strcmp(argv[1],"/c") == 0)) {
		// Cleanup
		return cleanupAllUnplayableLevels(argv[2]);
	}
	else if((strcmp(argv[1],"-f") == 0) || (strcmp(argv[1],"/f") == 0)) {
		// Fix all checksums
		return fixChecksum(argv[2]);
	}
	else if((strcmp(argv[1],"-l") == 0) || (strcmp(argv[1],"/l") == 0)) {
		// List
		return displayLevelList();
	}
	
	printf("Unknown mode: %s\n", argv[1]);
	printf("For help: %s -?\n", argv[1]);
	
	return -1;
}

/* Read save data file */
int readSaveData(char *filename) {
	FILE *fpIn = fopen(filename, "rb");
	if(fpIn == NULL) return 0;
	fread(saveData, 1, MAX_SAVE_SIZE, fpIn);
	fclose(fpIn);
	return 1;
}

/* Write save data file */
int writeSaveData(char *filename) {
	FILE *fpOut = fopen(filename, "wb");
	if(fpOut == NULL) return 0;
	fwrite(saveData, 1, MAX_SAVE_SIZE, fpOut);
	fclose(fpOut);
	return 1;
}

/* Read input data file */
int readInputData(char *filename) {
	FILE *fpIn = fopen(filename, "rb");
	if(fpIn == NULL) return 0;
	inputSize = fread(inputData, 1, MAX_INPUT_SIZE, fpIn);
	fclose(fpIn);
	return 1;
}

/* Write save data file */
int writeInputData(char *filename) {
	FILE *fpOut = fopen(filename, "wb");
	if(fpOut == NULL) return 0;
	fwrite(inputData, 1, inputSize, fpOut);
	fclose(fpOut);
	return 1;
}

/* Initialize ELevelInfo struct */
void initELevelInfo(ELevelInfo& info) {
	for(int i = 0; i < MAX_NAME_SIZE; i++) info.name[i] = 0;
	info.name[0] = 0xFF;
	info.levelClass = 0;
	info.levelNumber = 0;
	info.icon = 0;
	info.eCoinID = 0;
	info.aceCoinTotal = 0;
}

/* Initialize ELevelRecord struct */
void initELevelRecord(ELevelRecord& record) {
	initELevelInfo(record.info);
	record.playable = 0;
	record.dataID = 0xFF;
	record.eCoinCollected = 0;
	record.aceCoinCollectedFlag = 0;
	record.aceCoinPerfect = 0;
	record.clearedByMario = 0;
	record.clearedByLuigi = 0;
}

/* Initialize ELevelData struct */
void initELevelData(ELevelData& level) {
	initELevelInfo(level.info);
	level.recordID = 0;
	for(int i = 0; i < MAX_ECOIN_B_SIZE; i++) level.ecoinB[i] = 0;
	for(int i = 0; i < MAX_ECOIN_A_SIZE; i++) level.ecoinA[i] = 0;
	for(int i = 0; i < MAX_DATA_SIZE; i++) level.data[i] = 0;
}

/* Load level data from the save data */
void loadLevelData() {
	for(int i = 0; i < MAX_LEVEL_DATA; i++) {
		initELevelData(levelData[i]);
		
		u32 addr = getLevelDataAddress(i);
		
		// Record ID (0=None)
		levelData[i].recordID = saveData[addr];
		
		if(levelData[i].recordID != 0) {
			// Level data (compressed)
			for(int j = 0; j < MAX_DATA_SIZE; j++) levelData[i].data[j] = saveData[addr + 8 + j];
		}
	}
}

/* Save level data to the save data */
void saveLevelData() {
	u8 appeared[MAX_LEVEL_RECORDS];
	for(int i = 0; i < MAX_LEVEL_RECORDS; i++) appeared[i] = 0;
	
	for(int i = 0; i < MAX_LEVEL_DATA; i++) {
		u32 addr = getLevelDataAddress(i);
		
		// Record ID (0=None)
		saveData[addr] = levelData[i].recordID;
		if( (levelData[i].recordID == 0) || (appeared[levelData[i].recordID - 1] >= 1) || 
		    ((!levelRecords[levelData[i].recordID - 1].playable) && (i != MAX_LEVEL_DATA - 1)) )
		{
			levelData[i].recordID = 0;
			saveData[addr] = 0;
		} else if(levelData[i].recordID >= 1) {
			appeared[levelData[i].recordID - 1]++;
		}
		
		if(levelData[i].recordID != 0) {
			// Dummy Data?
			saveData[addr + 1] = 0x00;
			saveData[addr + 2] = 0xFF;
			saveData[addr + 3] = 0xFF;
			saveData[addr + 4] = 0xFF;
			saveData[addr + 5] = 0xFF;
			saveData[addr + 6] = 0xFF;
			saveData[addr + 7] = 0x00;
			
			// Level data (compressed)
			for(int j = 0; j < MAX_DATA_SIZE; j++) saveData[addr + 8 + j] = levelData[i].data[j];
		} else {
			saveData[addr + 1] = 0x00;
			saveData[addr + 2] = 0x00;
			saveData[addr + 3] = 0x00;
			saveData[addr + 4] = 0x00;
			saveData[addr + 5] = 0x00;
			saveData[addr + 6] = 0x00;
			saveData[addr + 7] = 0x00;
			
			for(int j = 0; j < MAX_DATA_SIZE; j++) saveData[addr + 8 + j] = 0;
		}
		
		// Write to level data ID list
		if(i == MAX_LEVEL_DATA - 1) {
			saveData[OFFSET_UNFINISHED] = levelData[i].recordID;
		} else {
			saveData[OFFSET_DATAIDLIST + i] = levelData[i].recordID;
		}
	}
}

/* Get the level data address */
u32 getLevelDataAddress(u8 dataID) {
	if(dataID < 20) {
		return 0x6000 + ((dataID % 2 == 0) * 0x10) + (dataID * 0x800);
	} else if(dataID < 32) {
		return 0x16000 + ((dataID % 2 == 0) * 0x10) + ((dataID - 20) * 0x800);
	}
	return 0x800;
}

/* Load level records from the save data */
void loadLevelRecords() {
	for(int i = 0; i < MAX_LEVEL_RECORDS; i++) {
		initELevelRecord(levelRecords[i]);
		
		// Name
		for(int j = 0; j < MAX_NAME_SIZE; j++)
			levelRecords[i].info.name[j] = saveData[OFFSET_NAME + (i * MAX_NAME_SIZE) + j];
		
		// Class
		levelRecords[i].info.levelClass = (saveData[OFFSET_LEVELINFO + (i * 2) + 1] & 0xF8) >> 3;
		
		// Level number
		levelRecords[i].info.levelNumber = saveData[OFFSET_LEVELINFO + (i * 2) + 0] & 0x3F;
		
		// Level icon
		levelRecords[i].info.icon = ((saveData[OFFSET_LEVELINFO + (i * 2) + 1] & 0x07) << 2) | ((saveData[OFFSET_LEVELINFO + (i * 2) + 0] & 0xC0) >> 6);
		
		// Ace coin total
		levelRecords[i].info.aceCoinTotal = (saveData[OFFSET_ACECOIN + i] & 0xE0) >> 5;
		
		// Level data ID
		levelRecords[i].dataID = getLevelDataID(i);
		
		// e-coin ID
		levelRecords[i].info.eCoinID = getECoinID(i);
		
		// Playable flag
		levelRecords[i].playable = BITTEST(saveData[OFFSET_PLAYABLE + (i / 8)], i % 8);
		if(levelRecords[i].info.name[0] == 0xFF) levelRecords[i].playable = 0;
		if(levelRecords[i].playable != 0) levelRecords[i].playable = 1;
		
		// e-coin collected
		if(levelRecords[i].info.eCoinID != 0) {
			levelRecords[i].eCoinCollected = (saveData[OFFSET_ECOIN + levelRecords[i].info.eCoinID - 1] & 0x80) >> 7;
		} else {
			levelRecords[i].eCoinCollected = 0;
		}
		
		// Ace coin collected flag
		levelRecords[i].aceCoinCollectedFlag = saveData[OFFSET_ACECOIN + i] & 0x1F;
		
		// Ace coin perfect flag
		levelRecords[i].aceCoinPerfect = BITTEST(saveData[OFFSET_PERFECT_ACECOIN + (i / 8)], i % 8);
		if(levelRecords[i].aceCoinPerfect != 0) levelRecords[i].aceCoinPerfect = 1;
		
		// Mario cleared flag
		levelRecords[i].clearedByMario = BITTEST(saveData[OFFSET_CLEARED_MARIO + (i / 8)], i % 8);
		if(levelRecords[i].clearedByMario != 0) levelRecords[i].clearedByMario = 1;
		
		// Luigi cleared flag
		levelRecords[i].clearedByLuigi = BITTEST(saveData[OFFSET_CLEARED_LUIGI + (i / 8)], i % 8);
		if(levelRecords[i].clearedByLuigi != 0) levelRecords[i].clearedByLuigi = 1;
	}
}

/* Save level records to the save data */
void saveLevelRecords() {
	for(int i = 0; i < MAX_ECOIN_TABLE; i++) saveData[OFFSET_ECOIN + i] = 0;
	
	for(int i = 0; i < MAX_LEVEL_RECORDS; i++) {
		// Name
		int terminate = 0;
		int emptyname = 1;
		for(int j = 0; j < MAX_NAME_SIZE; j++) {
			if(!terminate) {
				saveData[OFFSET_NAME + (i * MAX_NAME_SIZE) + j] = levelRecords[i].info.name[j];
				if(levelRecords[i].info.name[j] == 0xFF) terminate = 1;
				else emptyname = 0;
			} else {
				saveData[OFFSET_NAME + (i * MAX_NAME_SIZE) + j] = 0;
			}
		}
		
		// Icon type and level number
		saveData[OFFSET_LEVELINFO + (i * 2) + 0] = (levelRecords[i].info.levelNumber) | (levelRecords[i].info.icon << 6);
		
		// Class and icon
		saveData[OFFSET_LEVELINFO + (i * 2) + 1] = (levelRecords[i].info.levelClass << 3) | ((levelRecords[i].info.icon >> 2) & 0x07);
		
		// Ace coin total and collected flag
		saveData[OFFSET_ACECOIN + i] = (levelRecords[i].info.aceCoinTotal << 5) | (levelRecords[i].aceCoinCollectedFlag);
		
		// Playable flag
		if((getLevelDataID(i) >= 32) || (emptyname)) {
			BITZERO(saveData[OFFSET_PLAYABLE + (i / 8)], i % 8);
		} else {
			BITSET(saveData[OFFSET_PLAYABLE + (i / 8)], i % 8);
		}
		
		// Ace coin perfect flag
		if(!levelRecords[i].aceCoinPerfect) {
			BITZERO(saveData[OFFSET_PERFECT_ACECOIN + (i / 8)], i % 8);
		} else {
			BITSET(saveData[OFFSET_PERFECT_ACECOIN + (i / 8)], i % 8);
		}
		
		// Mario cleared flag
		if(!levelRecords[i].clearedByMario) {
			BITZERO(saveData[OFFSET_CLEARED_MARIO + (i / 8)], i % 8);
		} else {
			BITSET(saveData[OFFSET_CLEARED_MARIO + (i / 8)], i % 8);
		}
		
		// Luigi cleared flag
		if(!levelRecords[i].clearedByLuigi) {
			BITZERO(saveData[OFFSET_CLEARED_LUIGI + (i / 8)], i % 8);
		} else {
			BITSET(saveData[OFFSET_CLEARED_LUIGI + (i / 8)], i % 8);
		}
		
		// e-coin
		if(levelRecords[i].info.eCoinID != 0) {
			saveData[OFFSET_ECOIN + levelRecords[i].info.eCoinID - 1] = (levelRecords[i].eCoinCollected << 7) | ((i + 1) & 0x7F);
		}
	}
}

/* Find this record's level data ID (255=Not found) */
u8 getLevelDataID(u8 recordID) {
	for(int i = 0; i < MAX_LEVEL_DATA; i++) {
		if(levelData[i].recordID == recordID + 1)
			return i;
	}
	return 0xFF;
}

/* Get e-coin ID (0=Not found) */
u8 getECoinID(u8 id) {
	for(int i = 0; i < MAX_ECOIN_TABLE; i++) {
		if(id + 1 == (saveData[OFFSET_ECOIN + i] & 0x7F))
			return i + 1;
	}
	
	return 0;
}

/* Find a free level data ID (255=Not found) */
u8 findFreeLevelDataID() {
	for(int i = 0; i < MAX_LEVEL_DATA; i++) {
		for(int j = 0; j < MAX_LEVEL_RECORDS; j++) {
			if(levelRecords[j].dataID == i)
				break;
			else if(j == MAX_LEVEL_RECORDS - 1)
				return i;
		}
	}
	return 0xFF;
}

/* Find a free level record ID (0=Not found) */
u8 findFreeRecordID() {
	for(int i = 0; i < MAX_LEVEL_RECORDS; i++) {
		if(levelRecords[i].info.name[0] == 0xFF)
			return i + 1;
	}
	return 0;
}

/* Get how many level data remain */
u8 getHowManyFreeLevelData() {
	u8 count = 0;
	
	for(int i = 0; i < MAX_LEVEL_DATA; i++) {
		for(int j = 0; j < MAX_LEVEL_RECORDS; j++) {
			if(levelRecords[j].dataID == i)
				break;
			else if(j == MAX_LEVEL_RECORDS - 1)
				count++;
		}
	}
	
	return count;
}

/* Get how many level records remain */
u8 getHowManyFreeRecord() {
	u8 count = 0;
	for(int i = 0; i < MAX_LEVEL_RECORDS; i++) {
		if(levelRecords[i].info.name[0] == 0xFF)
			count++;
	}
	return count;
}

/* Compare two levels info */
int compareLevelInfo(ELevelInfo& info1, ELevelInfo& info2) {
	// Name
	for(int i = 0; i < MAX_NAME_SIZE; i++) {
		if(info1.name[i] != info2.name[i])
			return 0;
		else if((info1.name[i] == 0xFF) && (info2.name[i] == 0xFF))
			break;
	}
	
	// Class
	//if(info1.levelClass != info2.levelClass) return 0;
	
	// Level Number
	//if(info1.levelNumber != info2.levelNumber) return 0;
	
	return 1;
}

/* Find a same level record ID (0=Not found) */
int findSameInfoRecord(ELevelInfo& info) {
	for(int i = 0; i < MAX_LEVEL_RECORDS; i++) {
		if(compareLevelInfo(info, levelRecords[i].info))
			return i + 1;
	}
	return 0;
}

/* Converts a SMA4 level name character code to displayable format */
char convertCharacterToASCII(u8 c) {
	// Null
	if(c == 0xFF) return '\0';
	
	// A-Z
	if(c <= 0x19) return 0x41 + c;
	
	// a-z
	if((c >= 0x20) && (c <= 0x39)) return 0x61 + (c - 0x20);
	
	// 0-9
	if((c >= 0x76) && (c <= 0x7F)) return 0x30 + (c - 0x76);
	
	if(c == 0x1C) return '\'';		// '
	if(c == 0x1D) return ',';		// ,
	if(c == 0x1E) return '.';		// .
	if(c == 0xE0) return '?';		// ?
	if(c == 0xE1) return '!';		// !
	if(c == 0xE2) return '-';		// -
	
	return ' ';
}

/* Reset Mario/Luigi's position in the map screen */
void resetCharacterMapPosition() {
	if(saveData[OFFSET_CURRENT_CHARACTER] == 0) {
		saveData[OFFSET_MARIO_MAP_POS_X] = 0x90;
		saveData[OFFSET_MARIO_MAP_POS_Y] = 0x48;
		saveData[OFFSET_LUIGI_MAP_POS_X] = 0x60;
		saveData[OFFSET_LUIGI_MAP_POS_Y] = 0x38;
	} else {
		saveData[OFFSET_MARIO_MAP_POS_X] = 0x60;
		saveData[OFFSET_MARIO_MAP_POS_Y] = 0x38;
		saveData[OFFSET_LUIGI_MAP_POS_X] = 0x90;
		saveData[OFFSET_LUIGI_MAP_POS_Y] = 0x48;
	}
}

/* Converts a SMA4 level name array to a displayable string (size of dest must be MAX_NAME_SIZE+1) */
void convertLevelNameToASCII(u8 *src, char *dest) {
	for(int i = 0; i < MAX_NAME_SIZE; i++) {
		dest[i] = convertCharacterToASCII(src[i]);
	}
	dest[MAX_NAME_SIZE] = '\0';
}

/* Get input data type (checks already compressed or not) */
int getInputDataType() {
	if((inputData[0] == 'A') && (inputData[1] == 'S') && (inputData[2] == 'R') && (inputData[3] == '0'))
		return INPUT_ALREADY_COMPRESSED;
	
	if((inputData[0x56] == 'A') && (inputData[0x57] == 'S') && (inputData[0x58] == 'R') && (inputData[0x59] == '0'))
		return INPUT_ECARD_BIN;
	
	return INPUT_NOT_COMPRESSED;
}

/* Display level list */
int displayLevelList() {
	printf("ID:Lvl: e:E:Ace:A:M:L:Cl:No:Name\n");
	
	for(int i = 0; i < MAX_LEVEL_RECORDS; i++) {
		// Load level name
		char name[MAX_NAME_SIZE+1];
		convertLevelNameToASCII(levelRecords[i].info.name, name);
		
		// Load level record
		ELevelRecord& record = levelRecords[i];
		
		// Count collected ace coins
		int aceCoins = 0;
		if(record.aceCoinCollectedFlag & 0x01) aceCoins++;
		if(record.aceCoinCollectedFlag & 0x02) aceCoins++;
		if(record.aceCoinCollectedFlag & 0x04) aceCoins++;
		if(record.aceCoinCollectedFlag & 0x08) aceCoins++;
		if(record.aceCoinCollectedFlag & 0x10) aceCoins++;
		
		// Display
		printf("%2d:%3d:%2d:%d:%d/%d:%d:%d:%d:%2d:%2d:%s\n",
		       i + 1,
		       record.dataID,
		       record.info.eCoinID,
		       record.eCoinCollected,
		       aceCoins,
		       record.info.aceCoinTotal,
		       record.aceCoinPerfect,
		       record.clearedByMario,
		       record.clearedByLuigi,
		       record.info.levelClass,
		       record.info.levelNumber,
		       name);
	}
	
	printf("\nRemain level data: %d\n", getHowManyFreeLevelData());
	printf("Remain level records: %d\n", getHowManyFreeRecord());
	
	return 0;
}

/* Add a new e-level */
int addLevel(char *savename, char *filename, int unfinished) {
	u8 buf1[MAX_INPUT_SIZE];		// Temporary buffer 1
	u8 buf2[MAX_INPUT_SIZE];		// Temporary buffer 2
	ELevelData level;				// Level data
	ELevelRecord record;			// New level record
	u8 recordID;					// Record ID
	u8 dataID;						// Level data ID
	u8 overwrite;					// Overwrite flag
	
	initELevelData(level);
	initELevelRecord(record);
	
	printf("Loading level data from %s\n", filename);
	
	if(!readInputData(filename)) {
		printf("Couldn't read level data from %s\n", filename);
		return -1;
	}
	printf("%s is %d bytes\n", filename, inputSize);
	
	int inputType = getInputDataType();		// Input type
	
	if(inputType == INPUT_NOT_COMPRESSED) {
		printf("This level data is not compressed\n");
		
		// Load level info
		level.info.eCoinID = inputData[0];
		level.info.aceCoinTotal = inputData[1];
		level.info.levelClass = inputData[2];
		level.info.levelNumber = inputData[3];
		level.info.icon = inputData[4];
		
		// Load level name
		int nameStart = 0x40;
		if(level.info.eCoinID != 0) nameStart = 0x180;
		
		for(int i = 0; i < MAX_NAME_SIZE; i++) {
			level.info.name[i] = inputData[nameStart + i];
			if(level.info.name[i] == 0xFF) break;
		}
		
		if(level.info.name[0] == 0xFF) {
			printf("This level has no name\n");
			return -2;
		}
		
		// Load e-coin data
		if(level.info.eCoinID != 0) {
			for(int i = 0; i < MAX_ECOIN_B_SIZE; i++) level.ecoinB[i] = inputData[0x40 + i];
			for(int i = 0; i < MAX_ECOIN_A_SIZE; i++) level.ecoinA[i] = inputData[0x60 + i];
		}
		
		// Compress
		int size1, size2;
		size1 = compress(inputSize, inputData, 0x00, buf1);
		printf("Compressed size (mode 0x00): %d\n", size1);
		size2 = compress(inputSize, inputData, 0x80, buf2);
		printf("Compressed size (mode 0x80): %d\n", size2);
		
		if((size1 >= MAX_DATA_SIZE) && (size2 >= MAX_DATA_SIZE)) {
			printf("This level is too big\n");
			return -2;
		}
		
		if(size1 <= size2) {
			printf("Compression mode: 0x00\n");
			for(int i = 0; i < size1; i++) level.data[i] = buf1[i];
		} else {
			printf("Compression mode: 0x80\n");
			for(int i = 0; i < size2; i++) level.data[i] = buf2[i];
		}
	} else {
		u32 compressDataOffset = 0;
		if(inputType == INPUT_ALREADY_COMPRESSED) {
			printf("This level data is already compressed\n");
		} else if(inputType == INPUT_ECARD_BIN) {
			printf("This level data is card bin file\n");
			compressDataOffset = 0x56;
		}
		
		if(inputSize >= MAX_DATA_SIZE + compressDataOffset) {
			printf("This level is too big\n");
			return -2;
		}
		
		for(u32 i = 0; i < inputSize; i++) level.data[i] = inputData[compressDataOffset + i];
		
		// Decompress
		u32 decompSize = decompress(inputData, buf1);
		printf("Decompressed size: %d\n", decompSize);
		
		// Load level info
		level.info.eCoinID = buf1[0];
		level.info.aceCoinTotal = buf1[1];
		level.info.levelClass = buf1[2];
		level.info.levelNumber = buf1[3];
		level.info.icon = buf1[4];
		
		// Load level name
		int nameStart = 0x40;
		if(level.info.eCoinID != 0) nameStart = 0x180;
		
		for(int i = 0; i < MAX_NAME_SIZE; i++) {
			level.info.name[i] = buf1[nameStart + i];
			if(level.info.name[i] == 0xFF) break;
		}
		
		if(level.info.name[0] == 0xFF) {
			printf("This level has no name\n");
			return -2;
		}
		
		// Load e-coin data
		if(level.info.eCoinID != 0) {
			for(int i = 0; i < MAX_ECOIN_B_SIZE; i++) level.ecoinB[i] = buf1[0x40 + i];
			for(int i = 0; i < MAX_ECOIN_A_SIZE; i++) level.ecoinA[i] = buf1[0x60 + i];
		}
	}
	
	recordID = findSameInfoRecord(level.info);
	if(recordID == 0) {
		printf("No same level record found, insert as an new level\n");
		overwrite = 0;
		recordID = findFreeRecordID();
		
		if(recordID == 0) {
			printf("No free level record found\n");
			return -2;
		}
		if(!unfinished) {
			dataID = findFreeLevelDataID();
			if(dataID == 0xFF) {
				printf("No free level data found\n");
				return -2;
			}
		} else {
			dataID = 32;
		}
	} else {
		printf("Overwrite to level record %d\n", recordID);
		overwrite = 1;
		dataID = levelRecords[recordID - 1].dataID;
		
		if(dataID == 0xFF) {
			if(!unfinished) dataID = findFreeLevelDataID();
			else dataID = 32;
		}
		if(dataID == 0xFF) {
			printf("No free level data found\n");
			return -2;
		}
	}
	level.recordID = recordID;
	printf("Record ID: %d\nData ID: %d\n", recordID, dataID);
	
	record.info = level.info;
	record.playable = (dataID < 32);
	record.dataID = dataID;
	
	if(overwrite) {
		record.eCoinCollected = levelRecords[recordID - 1].eCoinCollected;
		record.aceCoinCollectedFlag = levelRecords[recordID - 1].aceCoinCollectedFlag;
		record.aceCoinPerfect = levelRecords[recordID - 1].aceCoinPerfect;
		record.clearedByMario = levelRecords[recordID - 1].clearedByMario;
		record.clearedByLuigi = levelRecords[recordID - 1].clearedByLuigi;
	}
	
	printf("Inserting level record & data...\n");
	levelRecords[recordID - 1] = record;
	levelData[dataID] = level;
	
	if((level.info.eCoinID != 0) && (dataID < 32)) {
		printf("Inserting e-coin data...\n");
		for(int i = 0; i < MAX_ECOIN_A_SIZE; i++)
			saveData[OFFSET_ECOIN_A + ((level.info.eCoinID - 1) * MAX_ECOIN_A_SIZE) + i] = level.ecoinA[i];
		for(int i = 0; i < MAX_ECOIN_B_SIZE; i++)
			saveData[OFFSET_ECOIN_B + ((level.info.eCoinID - 1) * MAX_ECOIN_B_SIZE) + i] = level.ecoinB[i];
	}
	
	printf("Saving...\n");
	saveLevelData();
	saveLevelRecords();
	fixAllChecksum();
	
	printf("Writing to %s...\n", savename);
	if(!writeSaveData(savename)) {
		printf("Couldn't write to %s\n", savename);
		return -1;
	}
	
	printf("Done!\n");
	
	return 0;
}

/* Delete a level */
int delLevel(char *savename, int recordID, int full) {
	if((recordID <= 0) || (recordID > MAX_LEVEL_RECORDS)) {
		printf("Record ID %d is invalid\n", recordID);
		return -1;
	}
	
	u8 dataID = levelRecords[recordID - 1].dataID;
	
	if(full) {
		if(dataID != 0xFF) initELevelData(levelData[dataID]);
		initELevelRecord(levelRecords[recordID - 1]);
		resetCharacterMapPosition();
		printf("Deleted level record %d (level data %d) completely\n", recordID, dataID);
	} else {
		if(levelRecords[recordID - 1].playable || levelRecords[recordID - 1].dataID || (dataID == 32)) {
			if(dataID == 32) initELevelData(levelData[dataID]);
			levelRecords[recordID - 1].playable = 0;
			levelRecords[recordID - 1].dataID = 0;
			printf("Deleted level record %d's level data\n", recordID);
		} else {
			printf("Level record %d has no level data\n", recordID);
		}
	}
	
	printf("Saving...\n");
	saveLevelData();
	saveLevelRecords();
	fixAllChecksum();
	
	printf("Writing to %s...\n", savename);
	if(!writeSaveData(savename)) {
		printf("Couldn't write to %s\n", savename);
		return -1;
	}
	
	printf("Done!\n");
	
	return 0;
}

/* Cleanup all unplayable levels */
int cleanupAllUnplayableLevels(char *savename) {
	int cleanCount = 0;
	
	for(int i = 0; i < MAX_LEVEL_RECORDS; i++) {
		if(levelRecords[i].dataID == 0xFF) {
			if(levelRecords[i].info.name[0] != 0xFF) cleanCount++;
			initELevelRecord(levelRecords[i]);
		}
	}
	
	if(cleanCount >= 1) {
		printf("Cleaned up %d unplayable level records\n", cleanCount);
		resetCharacterMapPosition();
	} else {
		printf("No unplayable level records found\n");
	}
	
	printf("Saving...\n");
	saveLevelData();
	saveLevelRecords();
	fixAllChecksum();
	
	printf("Writing to %s...\n", savename);
	if(!writeSaveData(savename)) {
		printf("Couldn't write to %s\n", savename);
		return -1;
	}
	
	printf("Done!\n");
	
	return 0;
}

/* Fix checksum only */
int fixChecksum(char *savename) {
	printf("Fixing all checksums...\n");
	fixAllChecksum();
	
	printf("Writing to %s...\n", savename);
	if(!writeSaveData(savename)) {
		printf("Couldn't write to %s\n", savename);
		return -1;
	}
	
	printf("Done!\n");
	
	return 0;
}

/* Fix all checksums */
void fixAllChecksum() {
	char regionCode;
	u32 offset = 0;
	int masterindexCount = 0;
	u32 masterindexOffset = 0;
	int ecoinCount = 0;
	u32 ecoinOffset = 0;
	
	// Set region code
	regionCode = saveData[6];
	
	// Fix region codes and checksums
	while(offset < MAX_SAVE_SIZE - 4) {
		// SMA4MW?0
		if((saveData[offset + 0] == 'S') && (saveData[offset + 1] == 'M') && (saveData[offset + 2] == 'A') && (saveData[offset + 3] == '4')) {
			calcAndFixChecksum(offset, 0x1000, 8, regionCode, 6);
			
			if(masterindexCount == 0) {
				masterindexOffset = offset;
			} else {
				if(memcmp(saveData + offset, saveData + masterindexOffset, 0x1000) != 0) {
					memcpy(saveData + offset, saveData + masterindexOffset, 0x1000);
				}
			}
			masterindexCount++;
			offset += 0x1000;
		}
		// M3?0
		else if((saveData[offset + 0] == 'M') && (saveData[offset + 1] == '3') && (saveData[offset + 3] == '0')) {
			calcAndFixChecksum(offset, 0x1000, 4, regionCode, 2);
			offset += 0x1000;
		}
		// S4R?
		else if((saveData[offset + 0] == 'S') && (saveData[offset + 1] == '4') && (saveData[offset + 2] == 'R')) {
			calcAndFixChecksum(offset, 0x2000, 4, regionCode, 3);
			
			if(ecoinCount == 0) {
				ecoinOffset = offset;
			} else {
				if(memcmp(saveData + offset, saveData + ecoinOffset, 0x2000) != 0) {
					memcpy(saveData + offset, saveData + ecoinOffset, 0x2000);
				}
			}
			ecoinCount++;
			offset += 0x2000;
		}
		// S4C?
		else if((saveData[offset + 0] == 'S') && (saveData[offset + 1] == '4') && (saveData[offset + 2] == 'C')) {
			calcAndFixChecksum(offset, 0x1000, 4, regionCode, 3);
			offset += 0x1000;
		}
		// S4K?
		else if((saveData[offset + 0] == 'S') && (saveData[offset + 1] == '4') && (saveData[offset + 2] == 'K')) {
			calcAndFixChecksum(offset, 0x1000, 4, regionCode, 3);
			offset += 0x1000;
		}
		// others
		else {
			offset += 0x1000;
		}
	}
}

/* Calculate and fix checksum and region code */
void calcAndFixChecksum(u32 offset, u32 size, u32 checksumAreaStart, char regionCode, u32 regionOffset) {
	u16 sum;
	u16 oldsum;
	char oldregion;
	sum = 0;
	
	// Check and fix region code
	oldregion = saveData[offset + regionOffset];
	if(oldregion != regionCode) {
		saveData[offset + regionOffset] = regionCode;
	}
	
	// Calculate and fix checksum
	for(u32 i = 0; i < size; i++) {
		if((i != checksumAreaStart + 0) && (i != checksumAreaStart + 1)) {
			sum += saveData[i + offset];
		}
	}
	
	oldsum = saveData[offset + checksumAreaStart + 0] + (saveData[offset + checksumAreaStart + 1] * 0x100);
	
	if(sum != oldsum) {
		saveData[offset + checksumAreaStart + 0] = sum;
		saveData[offset + checksumAreaStart + 1] = (sum & 0xFF00) >> 8;
	}
}
