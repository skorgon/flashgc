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

#define SDCARDDEV "/dev/block/mmcblk1"
#define BACKUPFILE "/sdcard/sdcardMbr-backup.img"

int writePartition(const char* pImageFile, const char* pPartition);
int backupMbr(const char* pPartition, const char* pBackupFile);
long filesize(FILE* fd);

int main(int argc, char* argv[])
{
	if (argc != 2) {
		printf("ERROR: Missing argument.\n");
		printf("Usage:\n\t%s <goldcard.img>\n", argv[0]);
		return -1;
	}

	if (backupMbr(SDCARDDEV, BACKUPFILE)) {
		printf("Error backing up the sd-cards MBR.\n");
		return -1;
	}
	printf("Success: Creating backup \"%s\".\n", argv[1]);

	if (writePartition(argv[1], SDCARDDEV)) {
		printf
		    ("Error writing the goldcard image to sd-card. SDcard may be corrupted. :(\n");
		return -1;
	}
	printf("Success: Writing image \"%s\" to sdcard.\n", argv[1]);

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

	printf("Writing image \"%s\" to sd-card (%s) ...\n", pImageFile,
	       pPartition);

	fdin = fopen(pImageFile, "rb");
	if (fdin == NULL) {
		printf("Error opening input image.\n");
		return -1;
	}

	if (filesize(fdin) > 512) {
		printf("Error: Gold card image exceeds 512 byte boundary. Aborting.\n");
		ret = -1;
		goto cleanup2;
	}

	fdout = fopen(pPartition, "wb");
	if (fdout == NULL) {
		printf("Error opening output partition.\n");
		ret = -1;
		goto cleanup2;
	}

	//  copy the image to the partition
	while (!feof(fdin)) {
		ch = fgetc(fdin);
		if (ferror(fdin)) {
			printf("Error reading input image.\n");
			ret = 1;
			goto cleanup1;
		}
		if (!feof(fdin))
			fputc(ch, fdout);
		if (ferror(fdout)) {
			printf("Error writing output partition.\n");
			ret = 1;
			goto cleanup1;
		}
	}

cleanup1:
	if (fclose(fdout) == EOF) {
		printf("Error closing output partition.\n");
		ret = 1;
	}

cleanup2:
	if (fclose(fdin) == EOF) {
		printf("Error closing input image.\n");
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
		printf("Error opening input partition.\n");
		return -1;
	}

	fdout = fopen(pBackupFile, "wb");
	if (fdout == NULL) {
		printf("Error opening backup file.\n");
		ret = -1;
		goto cleanup2;
	}

//  create a copy of the partition
	bytec = 0;
	while (!feof(fdin) && (bytec < 512)) {
		ch = fgetc(fdin);
		if (ferror(fdin)) {
			printf("Error reading input partition.\n");
			ret = 1;
			goto cleanup1;
		}
		if (!feof(fdin))
			fputc(ch, fdout);
		if (ferror(fdout)) {
			printf("Error writing backup file.\n");
			ret = 1;
			goto cleanup1;
		}
		bytec++;
	}

cleanup1:
	if (fclose(fdout) == EOF) {
		printf("Error closing backup file.\n");
		ret = 1;
	}

cleanup2:
	if (fclose(fdin) == EOF) {
		printf("Error closing input partition.\n");
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
