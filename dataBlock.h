#define BLOCKSIZE 256
#define DATABLOCKSIZE BLOCKSIZE-4
#pragma pack(1)
struct dataBlock
{
    char data[DATABLOCKSIZE];
    unsigned int nextBlock;
};
#pragma pack()
