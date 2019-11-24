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
  uint32_t blocks[1250];
};



struct Directory_Entry * dir;
struct Inode *inodeList;
uint8_t *freeBlockList;
uint8_t *freeInodeList;




void intializeBlockList()
{
  int i;
  for(i=0;i<4226;i++)
  {
    freeBlockList[i]=1;
  }
}

void intializeInodeList()
{
  int i;
  for(i=0;i<128;i++)
  {
    freeInodeList[i]=1;
  }
}

void intializeDirectory()
{
  int i;
  for (i=0;i<128;i++)
  {
    dir[i].valid=0;
    memset(dir[i].name,0,255);
    dir[i].inode=-1;
  }
}

void intializeInodes()
{
  int i;
  for (i=0;i<128;i++)
  {
    int j;
    inodeList[i].attributes=0;
    inodeList[i].size=0;
    for (j=0;j<1250;j++)
    {
      inodeList[i].blocks[j]=-1;
    }
  }
}

int findFreeInode()
{
  int ret =-1;
  int i;

  for(i=0;i<128;i++)
  {
    if(freeInodeList[i]==1)
    {
      ret=i;
      freeInodeList[i]=0;
      break;
    }
  }
  return ret;
}

int findFreeDirectory()
{
  int ret =-1;
  int i;

  for(i=0;i<128;i++)
  {
    if(dir[i].valid==0)
    {
      ret=i;
      //dir[i].valid=0;
      break;
    }
  }
  return ret;
}

void list()
{
  int i;

  for(i = 0 ; i < NUM_FILES ; i++)
  {
    if(dir[i].valid == 1 ){
    printf("%d: %s\n",i,dir[i].name);
    printf("Size: %d\n",inodeList[dir[i].inode].size);
    }
  }

}

int findFreeBlock()
{
  int ret =-1;
  int i;

  for(i=10;i<NUM_BLOCKS;i++)
  {
    if(freeBlockList[i]==1)
    {
      ret=i;
      freeBlockList[i]=0;
      break;
    }
  }
  return ret;
}

void createfs(char *filename)
{
  int i;
  memset(blocks, 0,NUM_BLOCKS*BLOCK_SIZE);
  fd=fopen(filename,"w");
  intializeDirectory();
  intializeInodeList();
  intializeBlockList();
  intializeInodes();
  fwrite(blocks,BLOCK_SIZE,NUM_BLOCKS,fd);
  fclose(fd);

}

void open(char * filename)
{
    fd = fopen(filename, "r");
  fread(blocks, BLOCK_SIZE, NUM_BLOCKS, fd);
  fclose(fd);
}

void close(char *filename)
{
  fd = fopen(filename, "w");
  fwrite(blocks, BLOCK_SIZE, NUM_BLOCKS, fd);
  fclose(fd);
  memset(blocks, 0,NUM_BLOCKS*BLOCK_SIZE);
}

int put(char *source)
{
//   char fname[100];

//   struct stat status;
//   // printf("Enter file name\n");
//   // scanf("%s",fname);
//   // stat(fname,&status);

//  // printf("Time is: %s\n",ctime(&status.st_atime)); 

//   char time[100];

//   strcpy(time,ctime(&status.st_atime));

//   int freeBlockIn = findFreeDirectory();
//   int freeInodeIn = findFreeInode();
//   dir[freeBlockIn].valid = 1;
//   strcpy(dir[freeBlockIn].name, "Hawa");
//   dir[freeBlockIn].inode = freeInodeIn;
//   inodeList[freeInodeIn].attributes = 0;
//   inodeList[freeInodeIn].size = 0;

 int    status;                   // Hold the status of all return values.
  struct stat buf;                 // stat struct to hold the returns from the stat call

  // Call stat with out input filename to verify that the file exists.  It will also 
  // allow us to get the file size. We also get interesting file system info about the
  // file such as inode number, block size, and number of blocks.  For now, we don't 
  // care about anything but the filesize.
  status =  stat( source, &buf ); 

  // If stat did not return -1 then we know the input file exists and we can use it.
  if( status != -1 )
  {
 
    // Open the input file read-only 
    
    
    int freeBlockIndex = findFreeDirectory();
    int freeInodeIndex = findFreeInode();
    strcpy(dir[freeBlockIndex].name,source);
    dir[freeBlockIndex].valid = 1;
    dir[freeBlockIndex].inode = freeInodeIndex;

    inodeList[freeInodeIndex].attributes = 0;
    int copy_size   = buf . st_size;
    inodeList[freeInodeIndex].size = copy_size;

    FILE *ifp = fopen ( source, "r" ); 
    printf("Reading %d bytes from %s\n", (int) buf . st_size, source );

    
 
    // Save off the size of the input file since we'll use it in a couple of places and 
    // also initialize our index variables to zero. 
     

    // We want to copy and write in chunks of BLOCK_SIZE. So to do this 
    // we are going to use fseek to move along our file stream in chunks of BLOCK_SIZE.
    // We will copy bytes, increment our file pointer by BLOCK_SIZE and repeat.
     int offset      = 0;               

    // We are going to copy and store our file in BLOCK_SIZE chunks instead of one big 
    // memory pool. Why? We are simulating the way the file system stores file data in
    // blocks of space on the disk. block_index will keep us pointing to the area of
    // the area that we will read from or write to.
    int block_index = 0;

    while( copy_size >= 8192 )
    {

      // Index into the input file by offset number of bytes.  Initially offset is set to
      // zero so we copy BLOCK_SIZE number of bytes from the front of the file.  We 
      // then increase the offset by BLOCK_SIZE and continue the process.  This will
      // make us copy from offsets 0, BLOCK_SIZE, 2*BLOCK_SIZE, 3*BLOCK_SIZE, etc.
      fseek( ifp, offset, SEEK_SET );
 
      // Read BLOCK_SIZE number of bytes from the input file and store them in our
      // data array. 
      int bytes  = fread( blocks[block_index], BLOCK_SIZE, 1, ifp );

      // If bytes == 0 and we haven't reached the end of the file then something is 
      // wrong. If 0 is returned and we also have the EOF flag set then that is OK.
      // It means we've reached the end of our input file.
      if( bytes == 0 && !feof( ifp ) )
      {
        printf("An error occured reading from the input file.\n");
        return -1;
      }

      // Clear the EOF file flag.
      clearerr( ifp );

      // Reduce copy_size by the BLOCK_SIZE bytes.
      copy_size -= BLOCK_SIZE;
      
      // Increase the offset into our input file by BLOCK_SIZE.  This will allow
      // the fseek at the top of the loop to position us to the correct spot.
      offset    += BLOCK_SIZE;

      // Increment the index into the block array 
      inodeList[freeInodeIndex].blocks[block_index] = freeBlockIndex;
      block_index ++;
    }


    if(copy_size > 0)
      {

      int freeBlockIndex = findFreeBlock();
      // Index into the input file by offset number of bytes.  Initially offset is set to
      // zero so we copy BLOCK_SIZE number of bytes from the front of the file.  We 
      // then increase the offset by BLOCK_SIZE and continue the process.  This will
      // make us copy from offsets 0, BLOCK_SIZE, 2*BLOCK_SIZE, 3*BLOCK_SIZE, etc.
      fseek( ifp, offset, SEEK_SET );
 
      // Read BLOCK_SIZE number of bytes from the input file and store them in our
      // data array. 
      int bytes  = fread( blocks[block_index], BLOCK_SIZE, 1, ifp );

      // If bytes == 0 and we haven't reached the end of the file then something is 
      // wrong. If 0 is returned and we also have the EOF flag set then that is OK.
      // It means we've reached the end of our input file.
      if( bytes == 0 && !feof( ifp ) )
      {
        printf("An error occured reading from the input file.\n");
        return -1;
      }

      // Clear the EOF file flag.
      clearerr( ifp );

      // Reduce copy_size by the BLOCK_SIZE bytes.
    //  copy_size -= BLOCK_SIZE;
      
      // Increase the offset into our input file by BLOCK_SIZE.  This will allow
      // the fseek at the top of the loop to position us to the correct spot.
    //  offset    += BLOCK_SIZE;
        inodeList[freeInodeIndex].blocks[block_index] = freeBlockIndex;
      // Increment the index into the block array 
    //  block_index ++;
    }

 
    // copy_size is initialized to the size of the input file so each loop iteration we
    // will copy BLOCK_SIZE bytes from the file then reduce our copy_size counter by
    // BLOCK_SIZE number of bytes. When copy_size is less than or equal to zero we know
    // we have copied all the data from the input file.
   // while( copy_size > 0 )
    // {

    //   // Index into the input file by offset number of bytes.  Initially offset is set to
    //   // zero so we copy BLOCK_SIZE number of bytes from the front of the file.  We 
    //   // then increase the offset by BLOCK_SIZE and continue the process.  This will
    //   // make us copy from offsets 0, BLOCK_SIZE, 2*BLOCK_SIZE, 3*BLOCK_SIZE, etc.
    //   fseek( ifp, offset, SEEK_SET );
 
    //   // Read BLOCK_SIZE number of bytes from the input file and store them in our
    //   // data array. 
    //   int bytes  = fread( blocks[block_index], BLOCK_SIZE, 1, ifp );

    //   // If bytes == 0 and we haven't reached the end of the file then something is 
    //   // wrong. If 0 is returned and we also have the EOF flag set then that is OK.
    //   // It means we've reached the end of our input file.
    //   if( bytes == 0 && !feof( ifp ) )
    //   {
    //     printf("An error occured reading from the input file.\n");
    //     return -1;
    //   }

    //   // Clear the EOF file flag.
    //   clearerr( ifp );

    //   // Reduce copy_size by the BLOCK_SIZE bytes.
    //   copy_size -= BLOCK_SIZE;
      
    //   // Increase the offset into our input file by BLOCK_SIZE.  This will allow
    //   // the fseek at the top of the loop to position us to the correct spot.
    //   offset    += BLOCK_SIZE;

    //   // Increment the index into the block array 
    //   block_index ++;
    // }

    // We are done copying from the input file so close it out.
    fclose( ifp );
  // GET STARTS HERE
    //*********************************************************************************
    //
    // The following chunk of code demonstrates similar functionality to your get command
    //

    // Now, open the output file that we are going to write the data to.
  }
  else
  {
    printf("Unable to open file: %s\n", source );
    perror("Opening the input file returned: ");
    return -1;
  }



}

int main()
{
  dir=(struct Directory_Entry*)&blocks[0];
  inodeList =(struct Inode*)&blocks[6];
  freeInodeList=(uint8_t*)&blocks[4];
  freeBlockList=(uint8_t*)&blocks[5];

  

  //dir[0].valid=1;
  int i;
  


  // dir=(struct Directory_Entry*)&blocks[0][0];
  // freeInodeList=(uint8_t*)&blocks[1][0];
  // freeBlockList=(uint8_t*)&blocks[2][0];
  //
  //
  // inodeList=(struct Inode*)&blocks[3][0];
  //
  // intializeDirectory();
  // intializeInodeList();
  // intializeBlockList();
  // intializeInodes();
  //
  // dir[0].valid=1;
  // dir[1].valid=1;
  //
  // inodeList[4].blocks[0]=42;
  // inodeList[4].blocks[1]=24;
  //
  // printf("%d %d %d\n",dir[0].valid,dir[1].valid,dir[127].valid);
  // printf("%d %d %d\n",inodeList[4].blocks[0],inodeList[4].blocks[1],inodeList[4].blocks[2]);
  //
// createfs("disk.image");
  open("disk.image");

 // put("testFile.txt");

 list();

  // for(i=0;i<128;i++)
  // {
  //   if(dir[i].valid != 0)
  //   printf("%d: %s\n",i,dir[i].name);
  // }
  // // modify it here
  // inodeList[4].blocks[0]=420;
  // inodeList[4].blocks[1]=240;
  //
  // printf("%d %d %d\n",inodeList[4].blocks[0],inodeList[4].blocks[1],inodeList[4].blocks[2]);
  //
  close("disk.image");
  // // zero it out
  // memset(blocks, 0,NUM_BLOCKS*BLOCK_SIZE);
  //
  // open("disk.image");
  // //read it
  // printf("%d %d %d\n",inodeList[4].blocks[0],inodeList[4].blocks[1],inodeList[4].blocks[2]);

}
