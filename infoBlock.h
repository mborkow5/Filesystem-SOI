#define NAMESIZE 16
struct infoBlock
{
    char name[NAMESIZE];
    unsigned int firstDataBlock;
    unsigned int size;
};
