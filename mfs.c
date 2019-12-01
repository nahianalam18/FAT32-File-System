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
#define MAX_NUM_ARGUMENTS 5 // Mav shell only supports five arguments
#define WHITESPACE " \t\n"

FILE *filePTR;

uint8_t blocks[NUM_BLOCKS][BLOCK_SIZE];

struct Directory_Entry // The File structure Sturct. Each file has these 3 properties.
{
  int8_t valid;
  char name[255];
  uint32_t inode;
};
// The Inode structure. Each Files Inode contains a link to this struct which has these 4 properties
// These 4 properties are used to store file information.
struct Inode
{
  uint8_t attributes;
  uint32_t size;
  char time[50];
  uint32_t blocks[1250];
};

struct Directory_Entry *dir; // Global directory pointer used to store all directories
struct Inode *inodeList;     // Global inodeList pointer used to store all inodes and check if they are available or not
uint8_t *freeBlockList;      // Global Blocklist which keeps track of which Blocks are free for use
uint8_t *freeInodeList;      //Global InodeList which keeps track of which Inodes are free for use.

void intializeBlockList()
{
  int i;
  for (i = 0; i < 4226; i++) // All the empty blocks are being initialized to 1.
  {
    freeBlockList[i] = 1; // A value of 1 indicates that the block is free which is checked before assigning some data to the block.
  }
}

void intializeInodeList() // All Inodes are initialized to 1.
{
  int i;
  for (i = 0; i < 128; i++)
  {
    freeInodeList[i] = 1; // A value of 1 indicates that the inode is free which is checked before assigning this specific Inode to a directory
  }
}

void intializeDirectory() // All Directory valids are initialized to 0.
{
  int i;
  for (i = 0; i < 128; i++)
  {
    dir[i].valid = 0; // A value of 0 indicates that the block is free for use to assign a new file to it.
    memset(dir[i].name, 0, 255);
    dir[i].inode = -1;
  }
}

void intializeInodes() // Inode proterties are initialized.
{
  int i;
  for (i = 0; i < 128; i++)
  {
    int j;
    inodeList[i].attributes = 0;   //Attribute set to not hidden & not read only
    inodeList[i].size = 0;         // Intial size of the file set to 0.
    strcpy(inodeList[i].time, ""); // All times are set to empty string which are later to be modified in the put function.
    for (j = 0; j < 1250; j++)
    {
      inodeList[i].blocks[j] = -1;
    }
  }
}

int findFreeInode() // Function used to check which Inode is avialable. if a value is 1 in the array, it indicates that it is available for use.
{
  int ret = -1;
  int i;

  for (i = 0; i < 128; i++)
  {
    if (freeInodeList[i] == 1)
    {
      ret = i; // We found the index which is free and will use it to store file data in that index in the inodeList
      freeInodeList[i] = 0;
      break;
    }
  }
  return ret;
}

int findFreeDirectory() // Function used to check which directory is avaiable for use.
{
  int ret = -1;
  int i;

  for (i = 0; i < 128; i++)
  {
    if (dir[i].valid == 0)
    {
      ret = i; // The index that is free for use is returned and then used to store the file in it
      break;
    }
  }
  return ret;
}

void del(char *filename) // Function used to delete a file that exist in the file system.
{
  int i;
  for (i = 0; i < NUM_FILES; i++)
  {
    if (dir[i].valid == 1 && strcmp(dir[i].name, filename) == 0) // Loop to find the index of the file in the directory.
    {

      break;
    }
  }
  if (strcmp(dir[i].name, "") == 0)
  {
    printf("del error: File not found.\n"); // Checks for empty string to confirm that the file is not avialable.
    return;
  }
  //Check is the file is read-only. In that case, the file cannot be deleted.
  else if (inodeList[dir[i].inode].attributes == 2 || inodeList[dir[i].inode].attributes == 3)
  {
    printf("del: That file is marked read-only.\n");
    return;
  }

  else // Since the file is not read only, it's made available by setting its valid to 0 and the inode made available in the iNode list.
  {
    dir[i].valid = 0;
    // strcpy(dir[i].name)
    freeInodeList[dir[i].inode] = 1;
  }
}

void list() // Function used to list all the files in the file system.
{
  int i, track = 0;

  for (i = 0; i < NUM_FILES; i++)
  {
    // Checks if the file is available by checking its valid. Then it checks if the file is hidden or not.
    if (dir[i].valid == 1 && inodeList[dir[i].inode].attributes != 1 && inodeList[dir[i].inode].attributes != 3)
    {
      track++; // rack checks if there are more than 1 file in the file system.
      // Since the file is valid and not hidden, it will be printed.
      printf("%d %s %s", inodeList[dir[i].inode].size, dir[i].name, inodeList[dir[i].inode].time);
    }
  }
  if (track == 0) // Since track was not incremented, it means that there is no file in the file system
    printf("list: No files found.\n");
}

int findFreeBlock() // Function used to check if a block is free or not.
{
  int ret = -1;
  int i;

  for (i = 10; i < NUM_BLOCKS; i++)
  {
    if (freeBlockList[i] == 1)
    {
      ret = i; // Found the index of a free block and will be retunred so that it can be used to store data
      freeBlockList[i] = 0;
      break;
    }
  }
  return ret;
}

void createfs(char *filename) // Used to create a file system
{
  int i;
  memset(blocks, 0, NUM_BLOCKS * BLOCK_SIZE);
  filePTR = fopen(filename, "w");
  intializeDirectory(); // Directories of the file system are initialzed.
  intializeInodeList(); // Inodelist is initialized
  intializeBlockList(); // Block list is initialized
  intializeInodes();    // Inodes are initialized
  fwrite(blocks, BLOCK_SIZE, NUM_BLOCKS, filePTR);
  fclose(filePTR);
}

void open(char *filename) // file is opened
{
  int status;      // Hold the status of all return values.
  struct stat buf; // stat struct to hold the returns from the stat call

  status = stat(filename, &buf);

  if (status != -1)
  {

    filePTR = fopen(filename, "r");
    fread(blocks, BLOCK_SIZE, NUM_BLOCKS, filePTR);
    fclose(filePTR);
  }
  else
  {
    printf("open: File not found\n");
    return;
  }
}

void attrib(char *aType, char *filename) // Function that provides attributes to a file
{
  int i;

  for (i = 0; i < NUM_FILES; i++)
  {
    if (dir[i].valid == 1 && strcmp(filename, dir[i].name) == 0)
    {
      break; // Getting the index of the file that was inserted
    }
  }
  if (strcmp(dir[i].name, "") == 0)
  {
    printf("attrib: File not found\n"); // Checks to see if file is in the file system
    return;
  }
  else if (strcmp(aType, "+h") == 0)
  {
    if (inodeList[dir[i].inode].attributes == 2 || inodeList[dir[i].inode].attributes == 3) // Checks if the file is already read-only and hidden
    {
      inodeList[dir[i].inode].attributes = 3; // Since it was already read-only/ both it is assigned to both.
    }

    else
    {
      inodeList[dir[i].inode].attributes = 1; // Since the file was neither read-only nor hidden, it is set to hidden.
    }
  }

  else if (strcmp(aType, "-h") == 0)
  {
    //Checks if the file is already hidden or read-only or both.
    if (inodeList[dir[i].inode].attributes == 3 || inodeList[dir[i].inode].attributes == 2)
    {
      inodeList[dir[i].inode].attributes = 2; // if read only then just keep it as read-only
    }

    else
    {
      inodeList[dir[i].inode].attributes = 0; // Change it back to not hidden not read-only
    }
  }

  else if (strcmp(aType, "+r") == 0)
  { //Check if the file is already hidden/both
    if (inodeList[dir[i].inode].attributes == 1 || inodeList[dir[i].inode].attributes == 3)
    {
      inodeList[dir[i].inode].attributes = 3; // set to both
    }
    else
    {
      inodeList[dir[i].inode].attributes = 2; // Since it was neither, then make it read only.
    }
  }

  else if (strcmp(aType, "-r") == 0)
  {
    // Check if the file is hidden / both hidden and read only.
    if (inodeList[dir[i].inode].attributes == 3 || inodeList[dir[i].inode].attributes == 1)
    {
      inodeList[dir[i].inode].attributes = 1; // Set to just hidden
    }

    else
    {
      inodeList[dir[i].inode].attributes = 0; // Since it was not hidden take away read only if it was.
    }
  }
}

long df() // Function used to calculate the available space in the file structure.
{
  int i;
  long space = 0;
  ;
  for (i = 0; i < NUM_FILES; i++)
  {
    if (dir[i].valid == 0)                            // Checks for available file directories that have not been used because only they are free
      space += BLOCK_SIZE * (NUM_BLOCKS / NUM_FILES); // Each Block size is 8192 and each file has NumBlocks/NumFiles number of blocks
  }

  return space;
}

void close(char *filename) // Function used to close the file that was opened.
{
  int status;      // Hold the status of all return values.
  struct stat buf; // stat struct to hold the returns from the stat call

  status = stat(filename, &buf);

  if (status != -1)
  {
    filePTR = fopen(filename, "w");
    fwrite(blocks, BLOCK_SIZE, NUM_BLOCKS, filePTR);
    fclose(filePTR);
    memset(blocks, 0, NUM_BLOCKS * BLOCK_SIZE);
  }
  else
  {
    printf("close error: File does not exist.\n"); // Checks if the file is available or not
    return;
  }
}

int get(char *source, char *destination)
{

  int status;      // Hold the status of all return values.
  struct stat buf; // stat struct to hold the returns from the stat call

  // Call stat with out input filename to verify that the file exists.  It will also
  // allow us to get the file size. We also get interesting file system info about the
  // file such as inode number, block size, and number of blocks.  For now, we don't
  // care about anything but the filesize.
  //status =  stat( source, &buf );

  int i;

  for (i = 0; i < NUM_FILES; i++)
  {
    if (dir[i].valid == 1 && strcmp(dir[i].name, source) == 0)
      break; // Get the index of the file that the user has put in the terminal
  }

  if (strcmp(dir[i].name, "") == 0)
  {
    printf("get error: File not found.\n");
    return -1;
  }

  // If stat did not return -1 then we know the input file exists and we can use it.
  // if( status != -1 )
  // {
  FILE *ofp;
  ofp = fopen(destination, "w");

  if (ofp == NULL)
  {
    printf("Could not open output file: %s\n", destination);
    perror("Opening output file returned");
    return -1;
  }

  // Initialize our offsets and pointers just we did above when reading from the file.
  int block_index = 0;
  int copy_size = inodeList[dir[i].inode].size;
  int offset = 0;

  //   // Using copy_size as a count to determine when we've copied enough bytes to the output file.
  //   // Each time through the loop, except the last time, we will copy BLOCK_SIZE number of bytes from
  //   // our stored data to the file fp, then we will increment the offset into the file we are writing to.
  //   // On the last iteration of the loop, instead of copying BLOCK_SIZE number of bytes we just copy
  //   // how ever much is remaining ( copy_size % BLOCK_SIZE ).  If we just copied BLOCK_SIZE on the
  //   // last iteration we'd end up with gibberish at the end of our file.
  while (copy_size > 0)
  {

    int num_bytes;

    // If the remaining number of bytes we need to copy is less than BLOCK_SIZE then
    // only copy the amount that remains. If we copied BLOCK_SIZE number of bytes we'd
    // end up with garbage at the end of the file.
    if (copy_size < BLOCK_SIZE)
    {
      num_bytes = copy_size;
    }
    else
    {
      num_bytes = BLOCK_SIZE;
    }

    // Write num_bytes number of bytes from our data array into our output file.
    fwrite(&blocks[inodeList[dir[i].inode].blocks[block_index]], num_bytes, 1, ofp);

    // Reduce the amount of bytes remaining to copy, increase the offset into the file
    // and increment the block_index to move us to the next data block.
    copy_size -= BLOCK_SIZE;
    offset += BLOCK_SIZE;
    block_index++;

    // Since we've copied from the point pointed to by our current file pointer, increment
    // offset number of bytes so we will be ready to copy to the next area of our output file.
    fseek(ofp, offset, SEEK_SET);
  }

  //   // Close the output file, we're done.
  fclose(ofp);
}

int Get(char *source)
{

  int status;      // Hold the status of all return values.
  struct stat buf; // stat struct to hold the returns from the stat call

  // Call stat with out input filename to verify that the file exists.  It will also
  // allow us to get the file size. We also get interesting file system info about the
  // file such as inode number, block size, and number of blocks.  For now, we don't
  // care about anything but the filesize.
  //status =  stat( source, &buf );

  int i;

  for (i = 0; i < NUM_FILES; i++)
  {
    if (dir[i].valid == 1 && strcmp(dir[i].name, source) == 0)
      break;
  }

  if (strcmp(dir[i].name, "") == 0)
  {
    printf("get error: File not found.\n");
    return -1;
  }

  // If stat did not return -1 then we know the input file exists and we can use it.
  // if( status != -1 )
  // {
  FILE *ofp;
  ofp = fopen(source, "w");

  if (ofp == NULL)
  {
    printf("Could not open output file: %s\n", source);
    perror("Opening output file returned");
    return -1;
  }

  // Initialize our offsets and pointers just we did above when reading from the file.
  int block_index = 0;
  int copy_size = inodeList[dir[i].inode].size;
  int offset = 0;

  //   // Using copy_size as a count to determine when we've copied enough bytes to the output file.
  //   // Each time through the loop, except the last time, we will copy BLOCK_SIZE number of bytes from
  //   // our stored data to the file fp, then we will increment the offset into the file we are writing to.
  //   // On the last iteration of the loop, instead of copying BLOCK_SIZE number of bytes we just copy
  //   // how ever much is remaining ( copy_size % BLOCK_SIZE ).  If we just copied BLOCK_SIZE on the
  //   // last iteration we'd end up with gibberish at the end of our file.
  while (copy_size > 0)
  {

    int num_bytes;

    // If the remaining number of bytes we need to copy is less than BLOCK_SIZE then
    // only copy the amount that remains. If we copied BLOCK_SIZE number of bytes we'd
    // end up with garbage at the end of the file.
    if (copy_size < BLOCK_SIZE)
    {
      num_bytes = copy_size;
    }
    else
    {
      num_bytes = BLOCK_SIZE;
    }

    // Write num_bytes number of bytes from our data array into our output file.
    fwrite(&blocks[inodeList[dir[i].inode].blocks[block_index]], num_bytes, 1, ofp);

    // Reduce the amount of bytes remaining to copy, increase the offset into the file
    // and increment the block_index to move us to the next data block.
    copy_size -= BLOCK_SIZE;
    offset += BLOCK_SIZE;
    block_index++;

    // Since we've copied from the point pointed to by our current file pointer, increment
    // offset number of bytes so we will be ready to copy to the next area of our output file.
    fseek(ofp, offset, SEEK_SET);
  }

  //   // Close the output file, we're done.
  fclose(ofp);
}

int put(char *source) // put a file in the file System
{

  //  // printf("Time is: %s\n",ctime(&status.st_atime));

  int status;      // Hold the status of all return values.
  struct stat buf; // stat struct to hold the returns from the stat call
  time_t sysTime;
  struct tm *timeDetails;
  status = stat(source, &buf);

  if (status != -1)
  {

    int i;
    time(&sysTime);
    timeDetails = localtime(&sysTime);
    int currentFileSize = (int)buf.st_size;
    size_t length;
    length = strlen(source);

    for (i = 0; i < NUM_FILES; i++)
    {
      if (dir[i].valid == 1 && strcmp(dir[i].name, source) == 0)
      {

        printf("put error: File already exists in the file system.\n");
        return -1;
      }
    }

    if (currentFileSize >= 10, 240, 000) // Check to see if the current file is < 10 MB
    {
      printf("put error: Not enough disk space.\n");
      return -1;
    }
    if (currentFileSize > df()) // Check to see if there is enough block space available for the current file
    {
      printf("put error: Not enough disk space.\n");
      return -1;
    }

    if (length > 32) // Check to see if the filename has more than 32 characters/
    {
      printf("put error: File name too long\n");
      return -1;
    }
    int freeBlockIndex = findFreeDirectory();   // A free block is found
    int freeInodeIndex = findFreeInode();       // A free inode is found
    strcpy(dir[freeBlockIndex].name, source);   // Filename is saved in the directory name
    dir[freeBlockIndex].valid = 1;              // File set to used.
    dir[freeBlockIndex].inode = freeInodeIndex; // an Inode index is set for the file.

    inodeList[freeInodeIndex].attributes = 0;                     // attribute is set to not hidden, not read-only
    int copy_size = buf.st_size;                                  // the copy size of the file is set
    inodeList[freeInodeIndex].size = copy_size;                   // The file size is set in the inode
    strcpy(inodeList[freeInodeIndex].time, asctime(timeDetails)); // The time it was inserted in the file system is saved

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

  char *cmd_str = (char *)malloc(MAX_COMMAND_SIZE);
  // void* fileBlock = malloc(BLOCK_SIZE * NUM_BLOCKS);
  while (1)
  {
    // Print out the mfs prompt
    printf("msh> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while (!fgets(cmd_str, MAX_COMMAND_SIZE, stdin))
      ;

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int token_count = 0;

    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;

    char *working_str = strdup(cmd_str);

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while (((arg_ptr = strsep(&working_str, WHITESPACE)) != NULL) &&
           (token_count < MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup(arg_ptr, MAX_COMMAND_SIZE);
      if (strlen(token[token_count]) == 0)
      {
        token[token_count] = NULL;
      }
      token_count++;
    }

    // Now print the tokenized input as a debug check
    // \TODO Remove this code and replace with your shell functionality

    int token_index = 0;
    for (token_index = 0; token_index < token_count; token_index++)
    {
      // Exit the shell on commans Exit or Quit
      if (token[token_index] != NULL && (strcmp(token[token_index], "exit") == 0 || strcmp(token[token_index], "quit") == 0))
      {
        exit(0);
      }
      // Create a new file system on the command create fs
      else if (token[token_index] != NULL && strcmp(token[token_index], "createfs") == 0)
      {
        if (token[token_index + 1] == NULL) // Check to see if a name has been inserted
        {
          printf("createfs: File not found\n");
          break;
        }
        createfs(token[token_index + 1]);
      }
      // A file is put into the system
      else if (token[token_index] != NULL && strcmp(token[token_index], "put") == 0)
      {
        if (token[token_index + 1] == NULL)
        {
          printf("put error: Please enter name of file\n");
          break;
        }
        put(token[token_index + 1]);
      }
      // Current files in the file system are listed.
      else if (token[token_index] != NULL && strcmp(token[token_index], "list") == 0)
      {
        list();
      }
      // A file is opened
      else if (token[token_index] != NULL && strcmp(token[token_index], "open") == 0)
      {
        if (token[token_index + 1] == NULL)
        {
          printf("open error: Please enter name of file to be opened\n");
          break;
        }
        open(token[token_index + 1]);
      }
      // A file is closed
      else if (token[token_index] != NULL && strcmp(token[token_index], "close") == 0)
      {
        if (token[token_index + 1] == NULL)
        {
          printf("close error: Please enter name of file to be closed\n");
          break;
        }
        close(token[token_index + 1]);
      }
      // A file is deleted from the file system
      else if (token[token_index] != NULL && strcmp(token[token_index], "del") == 0)
      {
        if (token[token_index + 1] == NULL)
        {
          printf("del error: Please enter name of file to be deleted\n");
          break;
        }
        del(token[token_index + 1]);
      }
      // The available space of the directory is returned
      else if (token[token_index] != NULL && strcmp(token[token_index], "df") == 0)
      {
        printf("%ld bytes free\n", df());
      }
      // Attributes are set for the file in the iNode based on the command given by the user
      else if (token[token_index] != NULL && strcmp(token[token_index], "attrib") == 0)
      {
        if (token[token_index + 1] == NULL)
        {
          printf("attrib error: Type of attrib missing\n");
          break;
        }
        else if (token[token_index + 2] == NULL)
        {
          printf("attrib error: Filename missing\n");
          break;
        }
        attrib(token[token_index + 1], token[token_index + 2]);
      }

      // A file is copied into the current directory from the file system

      else if (token[token_index] != NULL && strcmp(token[token_index], "get") == 0)
      {
        if (token[token_index + 1] == NULL)
        {
          printf("get error: filename missing\n");
          break;
        }
        if (token[token_index + 2] == NULL)
        {
          Get(token[token_index + 1]);
        }
        else
        {
          get(token[token_index + 1], token[token_index + 2]);
        }
      }
    }

    free(working_root);
  }
  return 0;
}
