#include "disk.cpp"

int main(int argc, char * argv[]) {
  if(argc>1) {
    switch(argv[1][0]){
      case '1': {
        if(argc<4)return 1;
        unsigned int size=atoi(argv[2]);
        char* diskName=argv[3];
        createDisk(size, diskName);
        break;
      }
      case '2': {
        if(argc<4)return 1;
        char* fileName=argv[2];
        char* diskName=argv[3];
        copyFileToDisk(fileName, diskName);
        break;
      }
      case '3': {
        if(argc<4)return 1;
        char* diskName=argv[2];
        char* fileName=argv[3];
        copyFileFromDisk(diskName,fileName);
        break;
      }
      case '4': {
        if(argc<3)return 1;
        char* diskName=argv[2];
        removeDisk(diskName);
        break;
      }
      case '5': {
        if(argc<4)return 1;
        char* fileName=argv[2];
        char* diskName=argv[3];
        removeFromDisk(fileName, diskName);
        break;
      }
      case '6': {
        if(argc<3)return 1;
        char* diskName=argv[2];
        getDiskMap(diskName);
        break;
      }
      case '7': {
        if(argc<3)return 1;
        char* diskName=argv[2];
        listFiles(diskName);
        break;
      }
      default:
        return 1;
      }
      return 0;
    }
  
    int choice, size;
    char diskName[24];
    char fileName[24];
    while (1) {
    printf("Filesystem controller\n1. Create disk\n2. Copy file to disk\n3. Copy file from disk\n4. Remove disk\n5. Remove file from disk\n6. Get disk map\n7. List files\n0. Exit\n");
      scanf("%d", & choice);
      if (choice == 1) {
        printf("Enter disk size in KB and disk name\n");
        scanf("%d %s", & size, & diskName[0]);
        system("@cls||clear");
        createDisk(size, diskName);
      }
      if (choice == 2) {
        printf("Enter file name and disk name\n");
        scanf("%s %s", & fileName[0], & diskName[0]);
        system("@cls||clear");
        copyFileToDisk(fileName, diskName);
      }
      if (choice == 3) {
        printf("Enter file name and disk name\n");
        scanf("%s %s", & fileName[0], & diskName[0]);
        system("@cls||clear");
        copyFileFromDisk(diskName,fileName);
      }
      if (choice == 4) {
        printf("Enter disk name\n");
        scanf("%s", & diskName[0]);
        system("@cls||clear");
        removeDisk(diskName);
      }
      if (choice == 5) {
        printf("Enter file name and disk name\n");
        scanf("%s %s", & fileName[0], & diskName[0]);
        system("@cls||clear");
        removeFromDisk(fileName, diskName);
      }
      if (choice == 6) {
        system("@cls||clear");
        printf("Enter disk name\n");
        scanf("%s", & diskName[0]);
        getDiskMap(diskName);
        printf("\n\n");
      }
      if (choice == 7) {
        system("@cls||clear");
        printf("Enter disk name\n");
        scanf("%s", & diskName[0]);
        listFiles(diskName);
        printf("\n\n");
      }
      if (choice == 0)
        return 0;
    }

  }
