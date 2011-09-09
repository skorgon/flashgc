/*
 *  Copyright (C) 2011  skorgon
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  The vital parts of this program are copied from gfree.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "gopt.h"

#define SDCARDDEV "/dev/block/mmcblk1"
#define BACKUPFILE "./sdcardMbr-backup.img"

int writePartition(const char* pImageFile, const char* pPartition);
int backupMbr(const char* pPartition, const char* pBackupFile);
long filesize(FILE* fd);
void printHelp(const char* exec);
int reverseCid(const char* cidFile);

int main(int argc, const char* argv[])
{
	char* imgIn;
	char* imgOut;
	int doBackup = 1;

	void* options = gopt_sort(&argc, argv, gopt_start(
				gopt_option('h', 0, gopt_shorts('h'), gopt_longs("help")),
				gopt_option('c', GOPT_ARG, gopt_shorts('c'), gopt_longs("cid", "CID")),
				gopt_option('r', 0, gopt_shorts('r'), gopt_longs("restore"))
				));

	/* print help text and exit if help option is given */
	if (gopt(options, 'h')) {
		printHelp(argv[0]);
		return 0;
	}

	/* skip the backup process if restore option is given */
	if (gopt(options, 'r')) {
		doBackup = 0;
	}

	/* read CID if cid option valid */
	if (gopt_arg(options, 'c', &imgIn)) {
		return reverseCid(imgIn);
	}

	gopt_free(options);


	if (argc != 2) {
		printf("ERROR: Missing argument.\n");
		printHelp(argv[0]);
		return -1;
	}

	/*
	 * Check if the image path given on the command line is identical to
	 * the proposed backup path. If so, assume a backup restore and skip
	 * creating a backup.
	 */
	imgIn = realpath(argv[1], NULL);
	imgOut = realpath(BACKUPFILE, NULL);
	if (imgIn == NULL) {
		printf("ERROR: Input image \"%s\" not found.\n", argv[1]);
		return -1;
	}
	if (imgOut != NULL) {
		if (!strcmp(imgIn, imgOut))
			doBackup = 0;
		free(imgOut);
	}
	free(imgIn);

	if (!doBackup)
		printf("Restoring backup image.\n");

	if (doBackup) {
		if (backupMbr(SDCARDDEV, BACKUPFILE)) {
			printf("ERROR: Backing up the sd-card's MBR failed.\n");
			return -1;
		}
		printf("Success: Backup file \"%s\" has been created.\n", BACKUPFILE);
	}

	if (writePartition(argv[1], SDCARDDEV)) {
		printf
		    ("ERROR: Writing the image to sd-card failed. SD-card may be corrupted. :(\n");
		return -1;
	}
	printf("Success: Image \"%s\" has been written to sd-card.\n", argv[1]);

	return 0;
}

/*
 * writePartition function copied from gfree and modified to overwrite only
 * the first 512 bytes of the sd-card at max
 */
int writePartition(const char* pImageFile, const char* pPartition)
{
	FILE* fdin;
	FILE* fdout;
	char ch;
	int ret = 0;
	int bytec;

	printf("Writing image \"%s\" to sd-card (%s) ...\n", pImageFile,
	       pPartition);

	fdin = fopen(pImageFile, "rb");
	if (fdin == NULL) {
		printf("ERROR: Opening input image failed.\n");
		return -1;
	}

	if (filesize(fdin) > 512) {
		printf("ERROR: Image exceeds 512 byte boundary.\n");
		ret = -1;
		goto cleanup2;
	}

	fdout = fopen(pPartition, "wb");
	if (fdout == NULL) {
		printf("ERROR: Opening output partition failed.\n");
		ret = -1;
		goto cleanup2;
	}

	//  copy the image to the partition
	bytec = 0;
	while (!feof(fdin) && (bytec < 512)) {
		ch = fgetc(fdin);
		if (ferror(fdin)) {
			printf("ERROR: Reading from input image failed.\n");
			ret = 1;
			goto cleanup1;
		}
		if (!feof(fdin))
			fputc(ch, fdout);
		if (ferror(fdout)) {
			printf("ERROR: Writing to output partition failed.\n");
			ret = 1;
			goto cleanup1;
		}
		bytec++;
	}

cleanup1:
	if (fclose(fdout) == EOF) {
		printf("ERROR: Closing output partition failed.\n");
		ret = 1;
	}

cleanup2:
	if (fclose(fdin) == EOF) {
		printf("ERROR: Closing input image failed.\n");
		ret = 1;
	}

	return ret;
}

/*
 * backupPartition function copied from gfree and modified to only backup
 * the first 512 bytes of the partition
 */
int backupMbr(const char* pPartition, const char* pBackupFile)
{
	FILE* fdin;
	FILE* fdout;
	char ch;
	int bytec;
	int ret = 0;

	printf("Backing up sd-card MBR (%s) to \"%s\" ...\n", pPartition, pBackupFile);
	fdin = fopen(pPartition, "rb");
	if (fdin == NULL) {
		printf("ERROR: Opening input partition failed.\n");
		return -1;
	}

	fdout = fopen(pBackupFile, "wb");
	if (fdout == NULL) {
		printf("ERROR: Opening backup file failed.\n");
		ret = -1;
		goto cleanup2;
	}

//  create a copy of the partition
	bytec = 0;
	while (!feof(fdin) && (bytec < 512)) {
		ch = fgetc(fdin);
		if (ferror(fdin)) {
			printf("ERROR: Reading from input partition failed.\n");
			ret = 1;
			goto cleanup1;
		}
		if (!feof(fdin))
			fputc(ch, fdout);
		if (ferror(fdout)) {
			printf("ERROR: Writing to backup file failed.\n");
			ret = 1;
			goto cleanup1;
		}
		bytec++;
	}

cleanup1:
	if (fclose(fdout) == EOF) {
		printf("ERROR: Closing backup file failed.\n");
		ret = 1;
	}

cleanup2:
	if (fclose(fdin) == EOF) {
		printf("ERROR: Closing input partition failed.\n");
		ret = 1;
	}

	return ret;
}

/* Returns the size of file "fd" in bytes */
long filesize(FILE* fd)
{
	long size;
	long fpi = ftell(fd);

	fseek(fd, 0L, SEEK_END);
	size = ftell(fd);
	fseek(fd, fpi, SEEK_SET);

	return size;
}

void printHelp(const char* exec)
{
	printf("Usage:\n\t%s [options] <goldcard.img>\n", exec);
	printf("Options:\n");
	printf("\t--help\t\tPrint this help message and exit.\n");
	printf("\t--cid <cid>\tRead CID from file <cid> and print reversed CID.\n");
	printf("\t--restore\tRestore an backup.\n");
}

int reverseCid(const char* cidFile)
{
	FILE* fdin;
	char buf[100];
	char* ch;

	fdin = fopen(cidFile, "r");
	if (fdin == NULL) {
		printf("ERROR: Opening CID file failed.\n");
		return -1;
	}

	fgets(buf, 100, fdin);
	ch = buf;
	while ((*ch != '\0') && (*ch != '\n'))
		ch++;
	if (*ch == '\n')
		*ch = '\0';

	printf("CID          = %s\n", buf);
	printf("Reversed CID = ");
	while(ch > buf) {
		ch--;
		printf("%c%c", *(ch-1), *ch);
		ch--;
	}
	printf("\n");

	if (fclose(fdin) == EOF) {
		printf("ERROR: Closing CID file failed.\n");
		return -1;
	}

	return 0;
}
