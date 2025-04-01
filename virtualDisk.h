#include "infoBlock.h"
#include "infoBitmap.h"
#include "dataBlock.h"

#pragma pack(1)

struct virtualDisk {
  unsigned int fileCounter;
  unsigned int freeDataBlocks;
  unsigned int size;
};
virtualDisk readVirtualDisk(FILE * file) {
  fseek(file, 0, SEEK_SET);
  struct virtualDisk disk;
  fread( & disk, sizeof(disk), 1, file);
  return disk;
}
void readDataBitmap(char * pointerToBuffer, FILE * file, unsigned int size) {
  fseek(file, sizeof(virtualDisk) + MAXFILESNUMBER + 1, SEEK_SET);
  fread(pointerToBuffer, sizeof(char), ceil(size / BLOCKSIZE), file);
}
int findFreeBlock(char * pointerToBuffer, int dataBlockNumber) {
  for (int it = 0; it < dataBlockNumber; it++) {
    if (pointerToBuffer[it] == 0)
      return it;
  }
  return -1;
}

void readInfoBitmap(infoBitmap * pointerToBitmap, FILE * file) {
  fseek(file, sizeof(virtualDisk), SEEK_SET);
  fread(pointerToBitmap, sizeof(infoBitmap), 1, file);
}
int findFreeInfoBlock(infoBitmap * pointerToBitmap) {
  for (int it = 0; it < MAXFILESNUMBER; it++) {
    if (( * pointerToBitmap).isFree[it] == 0)
      return it;
  }
  return -1;
}
bool checkFileName(char * fileName, FILE * file, int diskSize, infoBitmap * pointerToBitmap) {
  fseek(file, ((sizeof(virtualDisk) + MAXFILESNUMBER + 1 + ceil(diskSize / BLOCKSIZE) + 1) / BLOCKSIZE + 1) * BLOCKSIZE, SEEK_SET);

  struct infoBlock ib;
  for (int it = 0; it < MAXFILESNUMBER; it++) {
    fread( & ib, sizeof(ib), 1, file);
    if (( * pointerToBitmap).isFree[it] == 0) continue;
    if (strcmp(fileName, ib.name) == 0)
      return true;
  }
  return false;
}
unsigned int readInfoBlock(char * fileName, FILE * file, int diskSize) {
  fseek(file, ((sizeof(virtualDisk) + MAXFILESNUMBER + 1 + ceil(diskSize / BLOCKSIZE) + 1) / BLOCKSIZE + 1) * BLOCKSIZE, SEEK_SET);
  struct infoBlock ib;
  for (int it = 0; it < MAXFILESNUMBER; it++) {
    fread( & ib, sizeof(ib), 1, file);
    if (strcmp(fileName, ib.name) == 0)
      return it;
  }
  return -1;
}
void saveInfoBlock(infoBlock ib, FILE * file, int freeInfoBlockNumber, int diskSize) {
  fseek(file, ((sizeof(virtualDisk) + MAXFILESNUMBER + 1 + ceil(diskSize / BLOCKSIZE) + 1) / BLOCKSIZE + 1) * BLOCKSIZE + freeInfoBlockNumber * sizeof(ib), SEEK_SET);
  fwrite( & ib, sizeof(ib), 1, file);
}
void setInfoBlockBitmap(struct infoBitmap * pointerToBitmap, FILE * file, int freeInfoBlockNumber, char sign) {
  ( * pointerToBitmap).isFree[freeInfoBlockNumber] = sign;
  fseek(file, sizeof(virtualDisk), SEEK_SET);
  fwrite(pointerToBitmap, sizeof(infoBitmap), 1, file);
}

void setSingleBlockBitmap(char * dataBitmap, FILE * file, unsigned int dataBlockPosition, char sign) {
  dataBitmap[dataBlockPosition] = sign;
  fseek(file, sizeof(virtualDisk) + MAXFILESNUMBER + 1, SEEK_SET);
  fwrite(dataBitmap, sizeof(char), dataBlockPosition + 1, file);
}

void saveDataToVirtualDisk(FILE * file, FILE * sourceFile, char * dataBitmap, struct virtualDisk * disk, struct infoBlock ib) {
  char * dbm = (char * ) calloc(ceil((double) disk -> size / BLOCKSIZE), sizeof(char));
  if (dbm == NULL) {
    // Handle memory allocation failure
    fprintf(stderr, "Memory allocation failed\n");
    return;
  }
  readDataBitmap(dbm, file, disk -> size);
  struct dataBlock db, db1;
  int reservedMem = DATABLOCKSIZE;
  setSingleBlockBitmap(dbm, file, ib.firstDataBlock, 1);
  long offset = ceil((sizeof(ib) * MAXFILESNUMBER) / BLOCKSIZE + 1) * BLOCKSIZE + ceil((sizeof(disk) + MAXFILESNUMBER + 1 + ceil(disk -> size / BLOCKSIZE) + 1) / BLOCKSIZE + 1) * BLOCKSIZE;
  if (reservedMem >= ib.size) {
    if (fseek(file, offset + ib.firstDataBlock * BLOCKSIZE, SEEK_SET) != 0) {
      fprintf(stderr, "fseek failed\n");
      free(dbm);
      return;
    }
    if (fseek(sourceFile, 0, SEEK_SET) != 0) {
      fprintf(stderr, "fseek sourceFile failed\n");
      free(dbm);
      return;
    }
    db.nextBlock = -1;
    for (int i = 0; i < DATABLOCKSIZE; i++) {
      db.data[i] = 0;
    }
    size_t bytesRead = fread( & db.data, sizeof(char), ib.size, sourceFile);
    if (bytesRead != ib.size) {
      fprintf(stderr, "fread failed\n");
      free(dbm);
      return;
    }
    size_t bytesWritten = fwrite( & db, sizeof(dataBlock), 1, file);
    fseek(file, offset + ib.firstDataBlock * BLOCKSIZE, SEEK_SET);
    fread( & db1, sizeof(dataBlock), 1, file);
    if (bytesWritten != 1) {
      fprintf(stderr, "fwrite failed\n");
      free(dbm);
      return;
    }
    disk -> freeDataBlocks--;
    free(dbm);
    return;
  } else {
    unsigned int lasBlock = ib.firstDataBlock;
    fseek(sourceFile, 0, SEEK_SET);
    while (reservedMem < ib.size) {
      unsigned int nextFreeBlock = findFreeBlock(dbm, ceil(disk -> size / BLOCKSIZE));
      db.nextBlock = nextFreeBlock;
      setSingleBlockBitmap(dbm, file, nextFreeBlock, 1);
      fseek(file, offset + lasBlock * BLOCKSIZE, SEEK_SET);
      for (int i = 0; i < DATABLOCKSIZE; i++) {
        db.data[i] = 0;
      }
      fread( & db.data, sizeof(char), DATABLOCKSIZE, sourceFile);
      fwrite( & db, sizeof(dataBlock), 1, file);

      fseek(file, offset + lasBlock * BLOCKSIZE, SEEK_SET);
      fread( & db1, sizeof(dataBlock), 1, file);
      lasBlock = nextFreeBlock;
      reservedMem += DATABLOCKSIZE;
      disk -> freeDataBlocks--;
    }

    fseek(file, offset + db.nextBlock * BLOCKSIZE, SEEK_SET);
    reservedMem = db.nextBlock;
    db.nextBlock = -1;
    for (int i = 0; i < DATABLOCKSIZE; i++) {
      db.data[i] = 0;
    }
    fread( & db.data, sizeof(char), DATABLOCKSIZE, sourceFile);
    fwrite( & db, sizeof(dataBlock), 1, file);
    fseek(file, offset + reservedMem * BLOCKSIZE, SEEK_SET);
    fread( & db1, sizeof(dataBlock), 1, file);
    disk -> freeDataBlocks--;
  }
  free(dbm);
}
void saveVirtualDisk(FILE * file, struct virtualDisk * disk) {
  fseek(file, 0, SEEK_SET);
  fwrite(disk, sizeof(disk), 1, file);
}
#pragma pack()