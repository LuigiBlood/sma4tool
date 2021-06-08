#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SHOWLOG
//#define DEBUG

/* Globals */
unsigned char *fileBuf; // File buffer
unsigned long fileSize; // File size

/* Prototypes */
int main(int argc, char *argv[]);
int loadFile(char *filename);
void calcAndFixChecksum(unsigned long offset, unsigned long size, int checksumAreaStart, char regionCode, int regionOffset);

/*
 * Main function
 * argc: Number of command line params
 * argv: Command line param strings
 */
int main(int argc, char *argv[]) {
	char regionCode;
	unsigned long offset;
	FILE *fp;
	int masterindexCount;
	unsigned long masterindexOffset;
	int saveslotCount;
	unsigned long saveslotOffsets[3];
	int ecoinCount;
	unsigned long ecoinOffset;
	
	// Show usage
	if(argc < 2) {
		printf("SMA4 Save Fix Tool version1.2\n(c)2009 purplebridge001/caitsith2\nUsage: %s [SMA4 save file]\n", argv[0]);
		return 1;
	}
	
	// Load a file
	if(!loadFile(argv[1])) {
		printf("Couldn't load from %s\n", argv[1]);
		return 2;
	}
	
	// Set region code
	regionCode = fileBuf[6];
	if(regionCode == 'E') {
		printf("Region: USA (E)\n");
	} else if(regionCode == 'J') {
		printf("Region: Japan (J)\n");
	} else if(regionCode == 'P') {
		printf("Region: Europe (P)\n");
	} else {
		printf("Region: Unknown (%c)\n", regionCode);
	}
	
	// Fix region codes and checksums
	offset = 0;
	masterindexCount = 0;
	masterindexOffset = 0;
	saveslotCount = 0;
	saveslotOffsets[0] = saveslotOffsets[1] = saveslotOffsets[2] = 0;
	ecoinCount = 0;
	ecoinOffset = 0;
	
	while(offset < fileSize - 4) {
		// SMA4MW?0
		if((fileBuf[offset + 0] == 'S') && (fileBuf[offset + 1] == 'M') && (fileBuf[offset + 2] == 'A') && (fileBuf[offset + 3] == '4')) {
			#ifdef SHOWLOG
			printf("SMA4MW?0 (master e-level index #%d) at offset %06lX\n", masterindexCount, offset);
			#endif
			
			calcAndFixChecksum(offset, 0x1000, 8, regionCode, 6);
			
			if(masterindexCount == 0) {
				masterindexOffset = offset;
			} else {
				if(memcmp(fileBuf + offset, fileBuf + masterindexOffset, 0x1000) == 0) {
					#ifdef DEBUG
					printf("Backup master e-level slot matched to primary slot\n");
					#endif
				} else {
					printf("Backup master e-level slot didn't matched to primary slot, fixing now...\n");
					memcpy(fileBuf + offset, fileBuf + masterindexOffset, 0x1000);
				}
			}
			masterindexCount++;
			offset += 0x1000;
		}
		// M3?0
		else if((fileBuf[offset + 0] == 'M') && (fileBuf[offset + 1] == '3') && (fileBuf[offset + 3] == '0')) {
			#ifdef SHOWLOG
			printf("M3?0 (save slot #%d) at offset %06lX\n", saveslotCount, offset);
			#endif
			
			calcAndFixChecksum(offset, 0x1000, 4, regionCode, 2);
			
			/*
			if(saveslotCount < 3) {
				saveslotOffsets[saveslotCount] = offset;
			} else if(saveslotCount < 6) {
				if(memcmp(fileBuf + offset, fileBuf + saveslotOffsets[saveslotCount - 3], 0x1000) == 0) {
					#ifdef DEBUG
					printf("Backup save slot #%d matched to primary slot\n", saveslotCount - 3);
					#endif
				} else {
					printf("Backup save slot #%d didn't matched to primary slot, fixing now...\n", saveslotCount - 3);
					memcpy(fileBuf + offset, fileBuf + saveslotOffsets[saveslotCount - 3], 0x1000);
				}
			} else {
				printf("Save slot #%d found, probably not a valid SMA4 save data.\n", saveslotCount);
			}
			*/
			saveslotCount++;
			
			offset += 0x1000;
		}
		// S4R?
		else if((fileBuf[offset + 0] == 'S') && (fileBuf[offset + 1] == '4') && (fileBuf[offset + 2] == 'R')) {
			#ifdef SHOWLOG
			printf("S4R? (ecoin #%d) at offset %06lX\n", ecoinCount, offset);
			#endif
			
			calcAndFixChecksum(offset, 0x2000, 4, regionCode, 3);
			
			if(ecoinCount == 0) {
				ecoinOffset = offset;
			} else {
				if(memcmp(fileBuf + offset, fileBuf + ecoinOffset, 0x2000) == 0) {
					#ifdef DEBUG
					printf("Backup ecoin slot matched to primary slot\n");
					#endif
				} else {
					printf("Backup ecoin slot didn't matched to primary slot, fixing now...\n");
					memcpy(fileBuf + offset, fileBuf + ecoinOffset, 0x2000);
				}
			}
			ecoinCount++;
			offset += 0x2000;
		}
		// S4C?
		else if((fileBuf[offset + 0] == 'S') && (fileBuf[offset + 1] == '4') && (fileBuf[offset + 2] == 'C')) {
			#ifdef SHOWLOG
			printf("S4C? (e-level) at offset %06lX\n", offset);
			#endif
			calcAndFixChecksum(offset, 0x1000, 4, regionCode, 3);
			offset += 0x1000;
		}
		// S4K?
		else if((fileBuf[offset + 0] == 'S') && (fileBuf[offset + 1] == '4') && (fileBuf[offset + 2] == 'K')) {
			#ifdef SHOWLOG
			printf("S4K? (replay) at offset %06lX\n", offset);
			#endif
			calcAndFixChecksum(offset, 0x1000, 4, regionCode, 3);
			offset += 0x1000;
		}
		// others
		else {
			#ifdef SHOWLOG
			printf("Invalid section at offset %06lX\n", offset);
			#endif
			offset += 0x1000;
		}
	}
	
	// Write
	fp = fopen(argv[1], "wb");
	if(!fp) {
		printf("Couldn't write to %s\n", argv[1]);
		return 3;
	}
	fwrite(fileBuf, fileSize, 1, fp);
	fclose(fp);
	
	free(fileBuf);
	
	printf("Done\n");
	
	return 0;
}

/*
 * Load a file
 * filename: Filename
 */
int loadFile(char *filename) {
	FILE *fp;
	
	// Open
	fp = fopen(filename, "rb");
	if(!fp) return 0; // Failed to open a file
	
	// Get file size
	fseek(fp, 0, SEEK_END);
	fileSize = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	// Allocate memory
	fileBuf = (unsigned char *)malloc(fileSize);
	
	// Read to the memory
	fread(fileBuf, fileSize, 1, fp);
	
	// Close
	fclose(fp);
	
	// Done
	return 1;
}

/*
 * Calculate and fix checksum and region code
 * offset: Offset to a section
 * size: Size of section
 * checksumAreaStart: 8 for SMA4MW?0 section, 4 for other sections
 * regionCode: Expected region code
 * regionOffset: Offset to actual region code
 */
void calcAndFixChecksum(unsigned long offset, unsigned long size, int checksumAreaStart, char regionCode, int regionOffset) {
	unsigned long i;
	unsigned short sum;
	unsigned short oldsum;
	char oldregion;
	sum = 0;
	
	// Check and fix region code
	oldregion = fileBuf[offset + regionOffset];
	if(oldregion != regionCode) {
		printf("%06lX: Fix region code from %c to %c\n", offset + regionOffset, oldregion, regionCode);
		fileBuf[offset + regionOffset] = regionCode;
	}
	
	// Calculate and fix checksum
	for(i = 0; i < size; i++) {
		if((i != checksumAreaStart + 0) && (i != checksumAreaStart + 1)) {
			sum += fileBuf[i + offset];
		}
	}
	
	oldsum = fileBuf[offset + checksumAreaStart + 0] + (fileBuf[offset + checksumAreaStart + 1] * 0x100);
	
	if(sum != oldsum) {
		printf("%06lX: Fix checksum %04X to %04X\n", offset + checksumAreaStart, oldsum, sum);
		fileBuf[offset + checksumAreaStart + 0] = sum;
		fileBuf[offset + checksumAreaStart + 1] = (sum & 0xFF00) >> 8;
	}
}
