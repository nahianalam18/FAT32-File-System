#include <stdint.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define BLOCK_SIZE 8192
#define NUM_BLOCKS 4226
#define NUM_FILES 128
#define MAX_COMMAND_SIZE 255
#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments
#define WHITESPACE " \t\n"

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
    // if(inodeList[dir[i].inode].attributes == 2  || inodeList[dir[i].inode].attributes == 3 )
    //   {printf("del: That file is marked read-only.\n"); break;}
     if(strcmp(dir[i].name,filename) == 0)
    {
      
      break;
    }
  }
  if(inodeList[dir[i].inode].attributes == 2  || inodeList[dir[i].inode].attributes == 3 )
      {printf("del: That file is marked read-only.\n"); return;}

      else
      {
        dir[i].valid = 0;
      freeInodeList[dir[i].inode] = 1;
      }
      

}

void list()
{
  int i;

  for (i = 0; i < NUM_FILES; i++)
  {
    if (dir[i].valid == 1 && inodeList[dir[i].inode].attributes != 1 && inodeList[dir[i].inode].attributes != 3 )
    {
     // if(inodeList[dir[i].inode].attributes != 1)
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
  int status;      // Hold the status of all return values.
  struct stat buf; // stat struct to hold the returns from the stat call

  status = stat(filename, &buf);

  if (status != -1)
  {

  fd = fopen(filename, "r");
  fread(blocks, BLOCK_SIZE, NUM_BLOCKS, fd);
  fclose(fd);
  }
  else
  {
    printf("open error: File does not exist\n");
    return;
  }
  
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
    {
      if(inodeList[dir[i].inode].attributes == 2 || inodeList[dir[i].inode].attributes == 3)
      {
        inodeList[dir[i].inode].attributes = 3; 
      }

      else 
      {
        inodeList[dir[i].inode].attributes = 1;
      }

    }
      
    else if (strcmp(aType, "-h") == 0)
    {
      if(inodeList[dir[i].inode].attributes == 3 || inodeList[dir[i].inode].attributes == 2)
      {
        inodeList[dir[i].inode].attributes = 2;
      }

      else 
      {
        inodeList[dir[i].inode].attributes = 0;
      }
    }
      
    else if (strcmp(aType, "+r") == 0)
    {
      if(inodeList[dir[i].inode].attributes == 1 || inodeList[dir[i].inode].attributes == 3)
      {
        inodeList[dir[i].inode].attributes = 3; 
      }
      else 
      {
        inodeList[dir[i].inode].attributes = 2;
      }
    }
      
    else if (strcmp(aType, "-r") == 0)
      {
      if(inodeList[dir[i].inode].attributes == 3 || inodeList[dir[i].inode].attributes == 1 )
      {
        inodeList[dir[i].inode].attributes = 1;
      }

      else 
      {
        inodeList[dir[i].inode].attributes = 0;
      }
    }

     
    
  
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
  int status;      // Hold the status of all return values.
  struct stat buf; // stat struct to hold the returns from the stat call

  status = stat(filename, &buf);

  if (status != -1)
  {
  fd = fopen(filename, "w");
  fwrite(blocks, BLOCK_SIZE, NUM_BLOCKS, fd);
  fclose(fd);
  memset(blocks, 0, NUM_BLOCKS * BLOCK_SIZE);
  }
  else 
  {
    printf("close error: File does not exist.\n");
    return;
  }
}

int get(char * source, char*destination)
{

  int    status;                   // Hold the status of all return values.
  struct stat buf;                 // stat struct to hold the returns from the stat call

  // Call stat with out input filename to verify that the file exists.  It will also 
  // allow us to get the file size. We also get interesting file system info about the
  // file such as inode number, block size, and number of blocks.  For now, we don't 
  // care about anything but the filesize.
  //status =  stat( source, &buf ); 

  int i;

  for(i = 0; i < NUM_FILES ; i++)
  {
    if(strcmp(dir[i].name,source)== 0)
      break;
  }

  if(strcmp(dir[i].name,"") == 0)
  {
    printf("get error: %s not in the File System!\n",source);
    return -1;
  }

  printf("The file I am about to get from the file system is %s\n",dir[i].name);





  // If stat did not return -1 then we know the input file exists and we can use it.
  // if( status != -1 )
  // {
  FILE *ofp;
    ofp = fopen(destination, "w");

    if( ofp == NULL )
    {
      printf("Could not open output file: %s\n", destination );
      perror("Opening output file returned");
      return -1;
    }

    // Initialize our offsets and pointers just we did above when reading from the file.
    int block_index = 0;
    int copy_size   = inodeList[dir[i].inode].size;
    int offset      = 0;

   printf("Writing %d bytes to %s\n", copy_size, destination );

  //   // Using copy_size as a count to determine when we've copied enough bytes to the output file.
  //   // Each time through the loop, except the last time, we will copy BLOCK_SIZE number of bytes from
  //   // our stored data to the file fp, then we will increment the offset into the file we are writing to.
  //   // On the last iteration of the loop, instead of copying BLOCK_SIZE number of bytes we just copy
  //   // how ever much is remaining ( copy_size % BLOCK_SIZE ).  If we just copied BLOCK_SIZE on the
  //   // last iteration we'd end up with gibberish at the end of our file. 
    while( copy_size > 0 )
    { 

      int num_bytes;

      // If the remaining number of bytes we need to copy is less than BLOCK_SIZE then
      // only copy the amount that remains. If we copied BLOCK_SIZE number of bytes we'd
      // end up with garbage at the end of the file.
      if( copy_size < BLOCK_SIZE )
      {
        num_bytes = copy_size;
      }
      else 
      {
        num_bytes = BLOCK_SIZE;
      }

      // Write num_bytes number of bytes from our data array into our output file.
      fwrite( &blocks[inodeList[dir[i].inode].blocks[block_index]], num_bytes, 1, ofp ); 

      // Reduce the amount of bytes remaining to copy, increase the offset into the file
      // and increment the block_index to move us to the next data block.
      copy_size -= BLOCK_SIZE;
      offset    += BLOCK_SIZE;
      block_index ++;

      // Since we've copied from the point pointed to by our current file pointer, increment
      // offset number of bytes so we will be ready to copy to the next area of our output file.
      fseek( ofp, offset, SEEK_SET );
    }

  //   // Close the output file, we're done. 
     fclose( ofp );
  }


  int Get(char * source)
{

  int    status;                   // Hold the status of all return values.
  struct stat buf;                 // stat struct to hold the returns from the stat call

  // Call stat with out input filename to verify that the file exists.  It will also 
  // allow us to get the file size. We also get interesting file system info about the
  // file such as inode number, block size, and number of blocks.  For now, we don't 
  // care about anything but the filesize.
  //status =  stat( source, &buf ); 

  int i;

  for(i = 0; i < NUM_FILES ; i++)
  {
    if(strcmp(dir[i].name,source)== 0)
      break;
  }

  if(strcmp(dir[i].name,"") == 0)
  {
    printf("get error: %s not in the File System!\n",source);
    return -1;
  }

  printf("The file I am about to get from the file system is %s\n",dir[i].name);





  // If stat did not return -1 then we know the input file exists and we can use it.
  // if( status != -1 )
  // {
  FILE *ofp;
    ofp = fopen(source, "w");

    if( ofp == NULL )
    {
      printf("Could not open output file: %s\n", source );
      perror("Opening output file returned");
      return -1;
    }

    // Initialize our offsets and pointers just we did above when reading from the file.
    int block_index = 0;
    int copy_size   = inodeList[dir[i].inode].size;
    int offset      = 0;

   printf("Writing %d bytes to %s\n", copy_size, source );

  //   // Using copy_size as a count to determine when we've copied enough bytes to the output file.
  //   // Each time through the loop, except the last time, we will copy BLOCK_SIZE number of bytes from
  //   // our stored data to the file fp, then we will increment the offset into the file we are writing to.
  //   // On the last iteration of the loop, instead of copying BLOCK_SIZE number of bytes we just copy
  //   // how ever much is remaining ( copy_size % BLOCK_SIZE ).  If we just copied BLOCK_SIZE on the
  //   // last iteration we'd end up with gibberish at the end of our file. 
    while( copy_size > 0 )
    { 

      int num_bytes;

      // If the remaining number of bytes we need to copy is less than BLOCK_SIZE then
      // only copy the amount that remains. If we copied BLOCK_SIZE number of bytes we'd
      // end up with garbage at the end of the file.
      if( copy_size < BLOCK_SIZE )
      {
        num_bytes = copy_size;
      }
      else 
      {
        num_bytes = BLOCK_SIZE;
      }

      // Write num_bytes number of bytes from our data array into our output file.
      fwrite( &blocks[inodeList[dir[i].inode].blocks[block_index]], num_bytes, 1, ofp ); 

      // Reduce the amount of bytes remaining to copy, increase the offset into the file
      // and increment the block_index to move us to the next data block.
      copy_size -= BLOCK_SIZE;
      offset    += BLOCK_SIZE;
      block_index ++;

      // Since we've copied from the point pointed to by our current file pointer, increment
      // offset number of bytes so we will be ready to copy to the next area of our output file.
      fseek( ofp, offset, SEEK_SET );
    }

  //   // Close the output file, we're done. 
     fclose( ofp );
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
    int currentFileSize = (int)buf.st_size;
    size_t length;
    length = strlen(source);

    if(currentFileSize >= 10,240,000)
    {
      printf("put error: Not enough disk space.\n");
      return -1;
    }
    if(currentFileSize > df())
    {
      printf("put error: Not enough disk space.\n");
      return -1;
    }

    if(length>32)
    {
      printf("put error: File name too long\n");
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
    // printf("Reading %d bytes from %s\n", currentFileSize, source);

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

 // inodeList = (struct Inode*)&blocks[i+3][0]; 

  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
 // void* fileBlock = malloc(BLOCK_SIZE * NUM_BLOCKS);
  while( 1 )
  {
    // Print out the mfs prompt
    printf ("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    int token_index  = 0;
    for( token_index = 0; token_index < token_count; token_index ++ ) 
    {
      //printf("\n%s %d\n",token[token_index],token_index);
      if(token[token_index]!= NULL && (strcmp(token[token_index],"exit") == 0 || strcmp(token[token_index],"quit") == 0))
      {
        exit(0);
      }
     else if(token[token_index]!= NULL && strcmp(token[token_index],"createfs") == 0)
      {
        if(token[token_index+1] == NULL)
          {printf("createfs error: Please enter name of File System\n");break;}
        createfs(token[token_index+1]);
      }

     else if(token[token_index]!= NULL && strcmp(token[token_index],"put") == 0)
      {
        if(token[token_index+1] == NULL)
          {printf("put error: Please enter name of file\n");break;}
        put(token[token_index+1]);
      }
     else if(token[token_index]!= NULL && strcmp(token[token_index],"list") == 0)
      {
        list();
      }

       else if(token[token_index]!= NULL && strcmp(token[token_index],"open") == 0)
      {
        if(token[token_index+1] == NULL)
          {printf("open error: Please enter name of file to be opened\n");break;}
        open(token[token_index+1]);
      }

      else if(token[token_index]!= NULL && strcmp(token[token_index],"close") == 0)
      {
        if(token[token_index+1] == NULL)
          {printf("close error: Please enter name of file to be closed\n");break;}
        close(token[token_index+1]);
      }

      else if(token[token_index]!= NULL && strcmp(token[token_index],"delete") == 0)
      {
        if(token[token_index+1] == NULL)
          {printf("delete error: Please enter name of file to be deleted\n");break;}
        delete(token[token_index+1]);
      }
      else if(token[token_index]!= NULL && strcmp(token[token_index],"df") == 0)
      {
        printf("%ld\n",df());
      }
      else if(token[token_index]!= NULL && strcmp(token[token_index],"attrib") == 0)
      {
        if(token[token_index+1] == NULL)
          {printf("attrib error: Type of attrib missing\n");break;}
        else if(token[token_index+2] == NULL)
          {printf("attrib error: Filename missing\n");break;}
        attrib(token[token_index+1],token[token_index+2]);
      }

       else if(token[token_index]!= NULL && strcmp(token[token_index],"get") == 0)
      {
        if(token[token_index+1] == NULL)
          {printf("get error: filename missing\n");break;}
       if(token[token_index+2] == NULL)
          {
            Get(token[token_index+1]);
          }
          else
          {
            get(token[token_index+1],token[token_index+2]);
          }
      }

      
     // printf("token[%d] = %s\n", token_index, token[token_index] );  
    }

    free( working_root );

  }
  return 0;

//   // createfs("disk.image");
//   open("disk.image");
//  // put("jackal.txt");
//  //delete("samisGay.txt");
// // attrib("-r","samisGay.txt");
//   // delete("lama.txt");
//   list();
  
//   printf("\n\n");
//  for(i = 0 ; i < NUM_FILES ; i++)
//  {
//    if(dir[i].valid == 1)
//    printf("%d : %s %d\n",i,dir[i].name,inodeList[dir[i].inode].attributes);
//  }

//  //get("job.txt","samisGay.txt");
//  //get("lama.txt","LULU.txt");

// // Get("job.txt");

// // printf("Size = %ld\n",df());
  
  

//   close("disk.image");
}
