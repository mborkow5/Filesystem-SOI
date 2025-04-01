#include <stdio.h>
#include <math.h>
#include <string.h>
#include "virtualDisk.h"

#pragma pack(1) // Prevent padding in structs

/*
    disk info,
    file info table
    data map
    data blocks
*/

void createDisk(unsigned int size, char * name) {
  FILE * file;
  printf("Creating disk %s\n", name);
  if ((file = fopen(name, "w+")) == NULL) {
    printf("Cannot open/create virtual disk\n");
    return;
  }
  virtualDisk disk;
  size <<= 10;
  disk.size = size;
  disk.fileCounter = 0;
  disk.freeDataBlocks = ceil(size / BLOCKSIZE);
  char z = 0;
  infoBlock ib;
  ib.firstDataBlock = -1;
  fseek(file, 0, SEEK_SET);
  fwrite( & disk, sizeof(virtualDisk), 1, file);
  char s[MAXFILESNUMBER + 1];
  for (int i = 0; i < MAXFILESNUMBER + 1; i++)
    s[i] = 0;
  fwrite( & s, sizeof(char), MAXFILESNUMBER + 1, file);
  char d[disk.freeDataBlocks + 1 + 1];
  for (int i = 0; i < disk.freeDataBlocks + 1; i++)
    d[i] = 0;
  fwrite( & d, sizeof(char), disk.freeDataBlocks + 1, file);
  fseek(file, ((sizeof(disk) + MAXFILESNUMBER + 1 + disk.freeDataBlocks + 1) / BLOCKSIZE + 1) * BLOCKSIZE, SEEK_SET);
  d[0] = 'A';
  fwrite( & d, sizeof(char), 1, file);
  fseek(file, ((sizeof(ib) * MAXFILESNUMBER) / BLOCKSIZE + 1) * BLOCKSIZE + ((sizeof(disk) + MAXFILESNUMBER + 1 + disk.freeDataBlocks + 1) / BLOCKSIZE + 1) * BLOCKSIZE, SEEK_SET);
  fwrite( & d, sizeof(char), 1, file);
  fseek(file, BLOCKSIZE * (disk.freeDataBlocks + 1) + ((sizeof(ib) * MAXFILESNUMBER) / BLOCKSIZE + 1) * BLOCKSIZE + ((sizeof(disk) + MAXFILESNUMBER + 1 + disk.freeDataBlocks + 1) / BLOCKSIZE + 1) * BLOCKSIZE, SEEK_SET);
  fwrite( & d, sizeof(char), 1, file);

  fclose(file);

}

void removeDisk(char * name) {
  printf("Removing disk %s\n", name);
  remove(name);
}
bool copyFileToDisk(char * fileName, char * diskName) {
  FILE * file, * sourceFile;
  infoBlock ib;
  virtualDisk disk;
  printf("Copying file '%s' to virtual disk\n", fileName);
  if ((file = fopen(diskName, "r+")) == NULL) {
    printf("Cannot open virtual disk\n");
    return false;
  }
  if ((sourceFile = fopen(fileName, "r+")) == NULL) {
    printf("Cannot open file '%s'\n", fileName);
    return false;
  }
  disk = readVirtualDisk(file);
  if (disk.fileCounter + 1 > MAXFILESNUMBER) {
    printf("File limit reached\n");
    return false;
  }
  fseek(sourceFile, 0, SEEK_END);
  for (int i = 0; i < 16; i++)
    ib.name[i] = 0;
  strcpy(ib.name, fileName);
  ib.size = ftell(sourceFile);
  if (ceil((double) ib.size / DATABLOCKSIZE) > disk.freeDataBlocks) {
    printf("Not enough space on virtual disk\n");
    return false;
  }
  char * dataBitmap = (char * ) malloc(sizeof(char) * ceil(disk.size / BLOCKSIZE));
  readDataBitmap(dataBitmap, file, disk.size);
  ib.firstDataBlock = findFreeBlock(dataBitmap, ceil(disk.size / BLOCKSIZE));

  struct infoBitmap ibm;
  readInfoBitmap( & ibm, file);
  int freeInfoBlockNumber = findFreeInfoBlock( & ibm);
  if (checkFileName(ib.name, file, disk.size, & ibm)) {
    printf("File '%s' already exists on virtual disk\n", fileName);
    return false;
  }
  saveInfoBlock(ib, file, freeInfoBlockNumber, disk.size);
  setInfoBlockBitmap( & ibm, file, freeInfoBlockNumber, 1);
  saveDataToVirtualDisk(file, sourceFile, dataBitmap, & disk, ib);
  disk.fileCounter++;
  saveVirtualDisk(file, & disk);
  fclose(file);
  fclose(sourceFile);
  free(dataBitmap);
  return true;
}

void removeFromDisk(char * fileName, char * diskName) {
  FILE * file;
  infoBlock ib;
  infoBitmap ibm;
  virtualDisk disk;
  printf("Removing file '%s' from virtual disk\n", fileName);
  if ((file = fopen(diskName, "r+")) == NULL) {
    printf("Cannot open virtual disk\n");
    return;
  }
  disk = readVirtualDisk(file);
  if (disk.fileCounter == 0) {
    printf("No files on virtual disk\n");
    return;
  }
  readInfoBitmap( & ibm, file);
  if (checkFileName(fileName, file, disk.size, & ibm) == false) {
    printf("File '%s' doesn't exists on virtual disk\n", fileName);
    return;
  }
  char * dataBitmap = (char * ) calloc(ceil((double) disk.size / BLOCKSIZE), sizeof(char));
  readDataBitmap(dataBitmap, file, disk.size);
  int ibn = readInfoBlock(fileName, file, disk.size);
  fseek(file, ((sizeof(virtualDisk) + MAXFILESNUMBER + 1 + ceil(disk.size / BLOCKSIZE) + 1) / BLOCKSIZE + 1) * BLOCKSIZE + ibn * sizeof(ib), SEEK_SET);
  fread( & ib, sizeof(ib), 1, file);
  dataBlock db;
  long offset = ceil((sizeof(ib) * MAXFILESNUMBER) / BLOCKSIZE + 1) * BLOCKSIZE + ceil((sizeof(disk) + MAXFILESNUMBER + 1 + ceil(disk.size / BLOCKSIZE) + 1) / BLOCKSIZE + 1) * BLOCKSIZE;
  if (ib.firstDataBlock != -1) {
    setSingleBlockBitmap(dataBitmap, file, ib.firstDataBlock, 0);
    fseek(file, offset + ib.firstDataBlock * BLOCKSIZE, SEEK_SET);
    fread( & db, sizeof(dataBlock), 1, file);
    disk.freeDataBlocks++;
    while (db.nextBlock != -1) {
      setSingleBlockBitmap(dataBitmap, file, db.nextBlock, 0);
      fseek(file, offset + db.nextBlock * BLOCKSIZE, SEEK_SET);
      fread( & db, sizeof(db), 1, file);
      disk.freeDataBlocks++;
    }
  }
  setInfoBlockBitmap( & ibm, file, ibn, 0);
  disk.fileCounter--;
  saveVirtualDisk(file, & disk);
  free(dataBitmap);
  fclose(file);
}
void listFiles(char * diskName) {
  FILE * file;
  virtualDisk disk;
  printf("Listing files on virtual disk\n");
  if ((file = fopen(diskName, "r")) == NULL) {
    printf("Cannot open virtual disk\n");
    return;
  }
  disk = readVirtualDisk(file);
  if (disk.fileCounter == 0) {
    printf("No files on virtual disk\n");
    return;
  }
  infoBitmap ibm;
  infoBlock ib;
  readInfoBitmap( & ibm, file);
  for (int i = 0; i < MAXFILESNUMBER; i++) {
    if (ibm.isFree[i] == 1) {
      fseek(file, ((sizeof(virtualDisk) + MAXFILESNUMBER + 1 + ceil(disk.size / BLOCKSIZE) + 1) / BLOCKSIZE + 1) * BLOCKSIZE + i * sizeof(ib), SEEK_SET);
      fread( & ib, sizeof(ib), 1, file);
      printf("%s\n", ib.name);
    }
  }
  fclose(file);
}
void getDiskMap(char * diskName) {

  FILE * file;
  virtualDisk disk;
  printf("Mapping disk memory\n");
  if ((file = fopen(diskName, "r")) == NULL) {
    printf("Cannot open virtual disk\n");
    return;
  }
  disk = readVirtualDisk(file);
  char * dataBitmap = (char * ) calloc(ceil((double) disk.size / BLOCKSIZE), sizeof(char));
  readDataBitmap(dataBitmap, file, disk.size);
  for (int i = 0; i < ceil((double) disk.size / BLOCKSIZE); i++) {
    printf("%d", dataBitmap[i]);
    if ((i + 1) % 16 == 0) {
      printf("\n");
    }
  }
  if ((int) ceil((double) disk.size / BLOCKSIZE) % 16 != 0) {
    printf("\n");
  }
  free(dataBitmap);
}
void copyFileFromDisk(char * diskName, char * fileName) {
  FILE * file, * destFile;
  infoBlock ib;
  virtualDisk disk;
  printf("Copying file '%s' from virtual disk\n", fileName);
  if ((file = fopen(diskName, "r")) == NULL) {
    printf("Cannot open virtual disk\n");
    return;
  }
  if ((destFile = fopen(fileName, "w+")) == NULL) {
    printf("Cannot create file '%s'\n", fileName);
    return;
  }
  disk = readVirtualDisk(file);
  if (disk.fileCounter == 0) {
    printf("No files on virtual disk\n");
    return;
  }
  infoBitmap ibm;
  readInfoBitmap( & ibm, file);
  if (checkFileName(fileName, file, disk.size, & ibm) == false) {
    printf("File '%s' doesn't exists on virtual disk\n", fileName);
    return;
  }
  char * dataBitmap = (char * ) calloc(ceil((double) disk.size / BLOCKSIZE), sizeof(char));
  readDataBitmap(dataBitmap, file, disk.size);
  int ibn = readInfoBlock(fileName, file, disk.size);
  fseek(file, ((sizeof(virtualDisk) + MAXFILESNUMBER + 1 + ceil(disk.size / BLOCKSIZE) + 1) / BLOCKSIZE + 1) * BLOCKSIZE + ibn * sizeof(ib), SEEK_SET);
  fread( & ib, sizeof(ib), 1, file);
  dataBlock db;
  long offset = ceil((sizeof(ib) * MAXFILESNUMBER) / BLOCKSIZE + 1) * BLOCKSIZE + ceil((sizeof(disk) + MAXFILESNUMBER + 1 + ceil(disk.size / BLOCKSIZE) + 1) / BLOCKSIZE + 1) * BLOCKSIZE;
  // unsigned int total_size = ib.size;
  if (ib.firstDataBlock != -1) {
    fseek(file, offset + ib.firstDataBlock * BLOCKSIZE, SEEK_SET);
    fread( & db, sizeof(db), 1, file);
    fwrite( & db.data, sizeof(char), sizeof(db.data), destFile);
    // ib.size
    while (db.nextBlock != -1) {
      fseek(file, offset + db.nextBlock * BLOCKSIZE, SEEK_SET);
      fread( & db, sizeof(db), 1, file);
      fwrite( & db.data, sizeof(char), sizeof(db.data), destFile);
    }
  }
  free(dataBitmap);
  fclose(file);
  fclose(destFile);

}
#pragma pack()