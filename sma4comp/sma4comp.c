#include <stdio.h>
#include <string.h>
#include "types.h"

#define MIN(x,y) ((x)<(y)?(x):(y))
#define BUFFER_SIZE 409600

/*
 * Used by compress function
 */
int range(int size, int *src, int ws, char *dst) {
	unsigned di=0,c[0x1000],t[0x1001],l=0,w=0xFFFFFFFF;
	int x,si,n;
	for(x=0;x<ws;x++)
	{
		c[x]=1;
		t[x]=x;
	}
	t[ws]=ws;
	for(si=0;si<size;si++)
	{
		n=src[si];
		l+=t[n]*(w/=t[ws]);w*=c[n]++;
		for(x=n;x<ws;t[++x]++);
		if(t[ws]>0xFFFF)
		{
			t[0]=0;
			for(x=0;x<ws;x++)
				t[x+1]=t[x]+(c[x]=c[x]>>1|1);
		}
		while((l&0xFF000000)==((l+w)&0xFF000000))
		{
			dst[di++]=l>>24;
			l<<=8;
			w<<=8;
		}
		while(w<=0xFFFF)
		{
		w=(0x10000-(l&0xFFFF))<<8;
		dst[di++]=l>>24;
		l<<=8;
		}
	}
	while((l&0xFF000000)==((l+w)&0xFF000000))
	{
		dst[di++]=l>>24;
		l<<=8;
		w<<=8;
	}
	while(l&0xFF000000)
	{
		dst[di++]=(l>>24);
		l<<=8;
	}
	return di;
}

/*
 * Compress the data
 */
int compress(u32 size, u8 *src, u8 mode, u8 *dst) {
	int data[0x10000];
	int offset[0x4000];
	int di=0,oi=0,ws=mode&0x80?0x1000:0x200,ds,os;
	int si,bo,bc,off,c;
	dst[0]='A';
	dst[1]='S';
	dst[2]='R';
	dst[3]='0';
	
	dst[4]=mode;
	dst[5]=size>>16;
	dst[6]=size>>8&0xFF;
	dst[7]=size&0xFF;

	for(si=0;si<size; )
	{
		bo = 0;
		bc = 0;
		for(off=1;off<=MIN(si,ws);off++)
		{
			for(c=0;c<MIN(size-si,0x102);c++)
				if(src[si-off+c]!=src[si+c])
					break;
			if(c>bc)
			{
				bo=off;
				bc=c;
			}
		}
		if(bc>2)
		{
			data[di++]=bc+0xFD;
			offset[oi++]=bo-1;
			si+=bc;
		}
		else 
			data[di++]=src[si++];
	}
	//di+=4;
	ds=range(di,data,0x200,(char*)dst+12);
	os=range(oi,offset,ws,(char*)dst+ds+12);
	dst[8]=(ds+12)>>24;
	dst[9]=(ds+12)>>16&0xFF;
	dst[10]=(ds+12)>>8&0xFF;
	dst[11]=(ds+12)&0xFF;
	return ds+12+os;
}

/*
 * Decompress the SMA4 e-level data
 */
u32 decompress(u8 *src, u8 *dest) {
	u32 i;
	u32 count = 0;
	u32 len = (src[5] << 0x10) | (src[6] << 8) | src[7];
	u32 tmp4 = (src[8] << 0x18) | (src[9] << 0x10) | (src[10] << 8) | src[11];

	u32 temp1 = 0x0200;
	u32 temp2 = 12;
	u32 temp3 = 0xFFFFFFFF;
	u32 temp4 = 0xFFFFFFFF;
	u32 temp5 = 0;
	u32 temp6 = 0;

	u32 tarray3[0x0200];
	u32 tarray[0x0201];
	u32 tarray4[0x1000];
	u32 tarray2[0x1001];

	u32 buffer1;
	u32 buffer2;

	u32 shitvar;
	u32 var;
	u32 someshit;
	u32 wtfshit;
	s32 wtfshit2;

	u32 tmpr0;
	//u32 tmpr1; // 'tmpr1' might be used uninitialized in this function
	u32 tmpr1 = 0;

	if (src[4] & 0x80) temp1 = 0x1000;

	//setup the buffer table thingies!
	tarray[0] = 0;
	for (i = 0; i < 0x0200; i++) {
		tarray3[i] = 1;
		tarray[i+1] = (tarray[i] + 1);
	}
	tarray2[0] = 0;
	for (i = 0; i < temp1; i++) {
		tarray4[i] = 1;
		tarray2[i+1] = (tarray2[i] + 1);
	}

	//fill initial input buffers
	buffer1 = (src[temp2] << 0x18) | (src[temp2+1] << 0x10) | (src[temp2+2] << 8) | src[temp2+3];
	buffer2 = (src[tmp4] << 0x18) | (src[tmp4+1] << 0x10) | (src[tmp4+2] << 8) | src[tmp4+3];
	temp2 += 4;
	tmp4 += 4;

	while (count < len) {
		//printf("count: %d  len: %d\n",count,len);
		shitvar = (tarray[0x0200] ? (temp4 / tarray[0x0200]) : 0);
		var = (shitvar ? ((buffer1 - temp6) / shitvar) : 0);
		someshit = 0;
		wtfshit = 0x0200;

		while (someshit < wtfshit) {
			wtfshit2 = (someshit + wtfshit) / 2;
			if (var < tarray[wtfshit2])
				wtfshit = wtfshit2;
			else
				someshit = wtfshit2 + 1;
		}

		while (wtfshit2 >= 0) {
			if ((tarray[wtfshit2] <= var) && (tarray[wtfshit2+1] > var)) break;
			wtfshit2--;
		}

		temp6 += (tarray[wtfshit2] * shitvar);
		temp4 = (tarray3[wtfshit2]++ * shitvar);

		for (i = (wtfshit2+1); i <= 0x0200; i++)
			tarray[i]++;
		if (tarray[0x0200] > 0xFFFF) {
			tarray[0] = 0;
			i = 0;
			while (i++ < 0x0200) {
				tarray3[i] = (tarray3[i] << 1) | 1;
				tarray[i+1] = (tarray[i] + tarray3[i]);
			}
		}
		while ((temp6 & 0xFF000000) == ((temp6 + temp4) & 0xFF000000)) {
			buffer1 = ((buffer1<<8) | src[temp2++]);
			temp6 <<= 8;
			temp4 <<= 8;
		}
		while (temp4 < 0x10000) {
			temp4 = (0x10000 - (temp6 & 0xFFFF)) << 8;
			buffer1 = (buffer1<<8) | src[temp2++];
			temp6 <<= 8;
		}

		if (wtfshit2 < 0x100)
			dest[count++] = wtfshit2;
		else {
			shitvar = (tarray2[temp1] ? (temp3 / tarray2[temp1]) : 0);
			var = (shitvar ? ((buffer2 - temp5) / shitvar) : 0);
			someshit = 0;
			wtfshit = temp1;

			while (someshit < wtfshit) {
				tmpr1 = (someshit + wtfshit) / 2;
				if (var < tarray2[tmpr1])
					wtfshit = tmpr1;
				else
					someshit = (tmpr1 + 1);
			}

			while (tmpr1 >= 0) {
				if ((tarray2[tmpr1] <= var) && (tarray2[tmpr1+1] > var)) break;
				tmpr1--;
			}
			tmpr0 = (count-tmpr1-1);

			for (i = (wtfshit2 - 0xFD); i > 0; i--)
				dest[count++] = dest[tmpr0++];

			temp5 += (tarray2[tmpr1] * shitvar);
			temp3 = (tarray4[tmpr1]++ * shitvar);
			for (i = (tmpr1+1); i <= temp1; i++)
				tarray2[i]++;

			if (tarray2[temp1] > 0xFFFF) {
				tarray2[0] = 0;
				i=0;
				while (i++ < temp1) {
					tarray4[i] = (tarray4[i] << 1) | 1;
					tarray2[i+1] = (tarray2[i] + tarray4[i]);
				}
			}

			while ((temp5 & 0xFF000000) == ((temp5+temp3) & 0xFF000000)) {
				buffer2 = (buffer2 << 8) | src[tmp4++];
				temp5 <<= 8;
				temp3 <<= 8;
			}
			while (temp3 < 0x10000) {
				temp3 = (0x10000 - (temp5 & 0xFFFF)) << 8;
				buffer2 = (buffer2 << 8) | src[tmp4++];
				temp5 <<= 8;
			}
		}
	}

	return count;
}

/*
 * Main function
 */
int main(int argc, char *argv[]) {
	FILE *fpIn, *fpOut;
	u8 buf1[BUFFER_SIZE], buf2[BUFFER_SIZE], buf3[BUFFER_SIZE];
	int sizeIn, sizeOut, sizeOut2;
	
	if((argc < 4) || (strcmp(argv[1], "-?") == 0) || (strcmp(argv[1], "/?") == 0)) {
		printf("SMA4 Alternate Compressor version0.01\n");
		printf("(c)2009 purplebridge001\n");
		printf("Compressor code by RANDY Ruler of Zexernet and caitsith2\n");
		printf("Decompressor code by Parasyte and Bouche\n");
		printf("\n");
		
		printf("Usage: %s [Mode] [In] [Out]\n", argv[0]);
		printf("Mode:\n");
		printf("-c Compress\n");
		printf("-d Decompress\n");
		return -1;
	}
	
	if((strcmp(argv[1], "-c") == 0) || (strcmp(argv[1], "/c") == 0) || (strcmp(argv[1], "-d") == 0) || (strcmp(argv[1], "/d") == 0)) {
		fpIn = fopen(argv[2], "rb");
		if(fpIn == NULL) {
			printf("Couldn't load a file from %s\n", argv[2]);
			return -2;
		}
		sizeIn = fread(buf1, 1, BUFFER_SIZE, fpIn);
		fclose(fpIn);
		printf("%s: %d bytes\n", argv[2], sizeIn);
		
		fpOut = fopen(argv[3], "wb");
		if(fpIn == NULL) {
			printf("Couldn't create a file to %s\n", argv[3]);
			return -2;
		}
		
		if((strcmp(argv[1], "-c") == 0) || (strcmp(argv[1], "/c") == 0)) {
			sizeOut = compress(sizeIn, (u8*)buf1, 0, (u8*)buf2);
			printf("Mode 0x00: %d bytes\n", sizeOut);
			sizeOut2 = compress(sizeIn, (u8*)buf1, 0x80, (u8*)buf3);
			printf("Mode 0x80: %d bytes\n", sizeOut2);
			
			if(sizeOut <= sizeOut2) {
				printf("Used mode 0x00\n");
				fwrite(buf2, 1, sizeOut, fpOut);
				if(sizeOut >= 0x7C6) printf("This level will not work on an e-reader.\n");
			} else {
				printf("Used mode 0x80\n");
				fwrite(buf3, 1, sizeOut2, fpOut);
				if(sizeOut2 >= 0x7C6) printf("This level will not work on an e-reader.\n");
			}
		} else {
			sizeOut = decompress(buf1, buf2);
			printf("Decompressed: %d bytes\n", sizeOut);
			fwrite(buf2, 1, sizeOut, fpOut);
		}
		
		fclose(fpOut);
	}
	
	return 0;
}
