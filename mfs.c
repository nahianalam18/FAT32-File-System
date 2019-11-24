#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BLOCK_SIZE 8192
#define NUM_BLOCKS 4226
#define NUM_FILES 128

FILE *fd;

uint8_t blocks[NUM_BLOCKS][BLOCK_SIZE];

struct Directory_Entry
{
  int8_t valid;
  char name[255];
  uint32_t inode;
};

struct Inode
{
  uint8_t attributes;
  uint32_t size;
  char time[50];
  uint32_t blocks[1250];
};

struct Directory_Entry *dir;
struct Inode *inodeList;
uint8_t *freeBlockList;
uint8_t *freeInodeList;

void intializeBlockList()
{
  int i;
  for (i = 0; i < 4226; i++)
  {
    freeBlockList[i] = 1;
  }
}

void intializeInodeList()
{
  int i;
  for (i = 0; i < 128; i++)
  {
    freeInodeList[i] = 1;
  }
}

void intializeDirectory()
{
  int i;
  for (i = 0; i < 128; i++)
  {
    dir[i].valid = 0;
    memset(dir[i].name, 0, 255);
    dir[i].inode = -1;
  }
}

void intializeInodes()
{
  int i;
  for (i = 0; i < 128; i++)
  {
    int j;
    inodeList[i].attributes = 0;
    inodeList[i].size = 0;
    strcpy(inodeList[i].time, "");
    for (j = 0; j < 1250; j++)
    {
      inodeList[i].blocks[j] = -1;
    }
  }
}

int findFreeInode()
{
  int ret = -1;
  int i;

  for (i = 0; i < 128; i++)
  {
    if (freeInodeList[i] == 1)
    {
      ret = i;
      freeInodeList[i] = 0;
      break;
    }
  }
  return ret;
}

int findFreeDirectory()
{
  int ret = -1;
  int i;

  for (i = 0; i < 128; i++)
  {
    if (dir[i].valid == 0)
    {
      ret = i;
      break;
    }
  }
  return ret;
}

void delete(char * filename)
{
  int i;
  for(i = 0; i < NUM_FILES ; i++)
  {
    if(inodeList[dir[i].inode].attributes == 2 )
      {printf("del: That file is marked read-only.\n"); break;}
    else if(strcmp(dir[i].name,filename) == 0)
    {
      dir[i].valid = 0;
      freeInodeList[dir[i].inode] = 1;
      break;
    }
  }
}

void list()
{
  int i;

  for (i = 0; i < NUM_FILES; i++)
  {
    if (dir[i].valid == 1 && inodeList[dir[i].inode].attributes != 1)
    {
      printf("%d %s %s", inodeList[dir[i].inode].size, dir[i].name, inodeList[dir[i].inode].time);
    }
  }
}

int findFreeBlock()
{
  int ret = -1;
  int i;

  for (i = 10; i < NUM_BLOCKS; i++)
  {
    if (freeBlockList[i] == 1)
    {
      ret = i;
      freeBlockList[i] = 0;
      break;
    }
  }
  return ret;
}

void createfs(char *filename)
{
  int i;
  memset(blocks, 0, NUM_BLOCKS * BLOCK_SIZE);
  fd = fopen(filename, "w");
  intializeDirectory();
  intializeInodeList();
  intializeBlockList();
  intializeInodes();
  fwrite(blocks, BLOCK_SIZE, NUM_BLOCKS, fd);
  fclose(fd);
}

void open(char *filename)
{
  fd = fopen(filename, "r");
  fread(blocks, BLOCK_SIZE, NUM_BLOCKS, fd);
  fclose(fd);
}

void attrib(char *aType, char *filename)
{
  int i;

  for (i = 0; i < NUM_FILES; i++)
  {
    if (strcmp(filename, dir[i].name) == 0)
    {
      break;
    }
  }
    if (strcmp(aType, "+h") == 0)
      inodeList[dir[i].inode].attributes = 1;
    else if (strcmp(aType, "-h") == 0)
      inodeList[dir[i].inode].attributes = 0; 
    else if (strcmp(aType, "+r") == 0)
      inodeList[dir[i].inode].attributes = 2; 
    else if (strcmp(aType, "-r") == 0)
      inodeList[dir[i].inode].attributes = 0; 

     
    
  
}

long df()
{
  int i;
  long space = 0;;
  for(i = 0; i < NUM_FILES ; i++)
  {
    if(dir[i].valid == 0)
      space += BLOCK_SIZE * (NUM_BLOCKS/NUM_FILES);
  }

  return space;
}

void close(char *filename)
{
  fd = fopen(filename, "w");
  fwrite(blocks, BLOCK_SIZE, NUM_BLOCKS, fd);
  fclose(fd);
  memset(blocks, 0, NUM_BLOCKS * BLOCK_SIZE);
}

int put(char *source)
{

  //  // printf("Time is: %s\n",ctime(&status.st_atime));

  int status;      // Hold the status of all return values.
  struct stat buf; // stat struct to hold the returns from the stat call

  status = stat(source, &buf);

  if (status != -1)
  {

    int i;

    // for(i = 0 ; i < NUM_FILES ; i++)
    // {
    //   if(strcmp(dir[i].name,source) == 0)
    //   printf("put error: File already in the system!\n");
    //   return -1;
    // }
    int currentFileSize = (int)buf.st_size;

    if(currentFileSize >= 10,240,000)
    {
      printf("put error: File size too big.\n");
      return -1;
    }
    if(currentFileSize > df())
    {
      printf("put error: Not enough disk space.\n");
      return -1;
    }
    int freeBlockIndex = findFreeDirectory();
    int freeInodeIndex = findFreeInode();
    strcpy(dir[freeBlockIndex].name, source);
    dir[freeBlockIndex].valid = 1;
    dir[freeBlockIndex].inode = freeInodeIndex;

    inodeList[freeInodeIndex].attributes = 0;
    int copy_size = buf.st_size;
    inodeList[freeInodeIndex].size = copy_size;
    strcpy(inodeList[freeInodeIndex].time, ctime(&buf.st_atime));

    FILE *ifp = fopen(source, "r");
    printf("Reading %d bytes from %s\n", currentFileSize, source);

    int offset = 0;

    int block_index = 0;

    while (copy_size >= 8192)
    {
      int freeBlockIndex = findFreeBlock();

      fseek(ifp, offset, SEEK_SET);

      int bytes = fread(&blocks[freeBlockIndex], BLOCK_SIZE, 1, ifp);

      if (bytes == 0 && !feof(ifp))
      {
        printf("An error occured reading from the input file.\n");
        return -1;
      }

      clearerr(ifp);

      copy_size -= BLOCK_SIZE;

      offset += BLOCK_SIZE;

      inodeList[freeInodeIndex].blocks[block_index] = freeBlockIndex;
      block_index++;
    }

    if (copy_size > 0)
    {

      int freeBlockIndex = findFreeBlock();

      fseek(ifp, offset, SEEK_SET);

      int bytes = fread(&blocks[freeBlockIndex], BLOCK_SIZE, 1, ifp);

      if (bytes == 0 && !feof(ifp))
      {
        printf("An error occured reading from the input file.\n");
        return -1;
      }

      clearerr(ifp);

      inodeList[freeInodeIndex].blocks[block_index] = freeBlockIndex;
    }

    fclose(ifp);
  }
  else
  {
    printf("Unable to open file: %s\n", source);
    perror("Opening the input file returned: ");
    return -1;
  }
}

int main()
{
  dir = (struct Directory_Entry *)&blocks[0];
  inodeList = (struct Inode *)&blocks[6];
  freeInodeList = (uint8_t *)&blocks[4];
  freeBlockList = (uint8_t *)&blocks[5];

  int i;

  // createfs("disk.image");
  open("disk.image");
  put("lama.txt");
 // delete("testFile.txt");
 // attrib("-h","jackal.txt");
  // delete("lama.txt");
  list();

 printf("Size = %ld\n",df());
  
  

  close("disk.image");
}
