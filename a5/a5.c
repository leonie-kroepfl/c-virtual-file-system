//------------------------------------------------------------------------------
// a5.c
//
// A small program "In-Memory Virtual File System" 
//
// Group: 084
//
// Author: 012311363
//------------------------------------------------------------------------------
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define RETURN_CODE_INVALID_ARGC 255 
#define ERROR_MESSAGE_INVALID_ARGC_BELOW_2 "[ERROR] No config file provided, exiting...\n"
#define ERROR_MESSAGE_INVALID_ARGC_ABOVE_2 "[ERROR] Too many arguments, exiting...\n"
#define RETURN_CODE_INVALID_FILE 254
#define ERROR_MESSAGE_IVALID_FILE "[ERROR] File could not be opened, exiting...\n" 



int checkArgumentCount(int argc); 
FILE* openFileToRead(char *file_name);


//------------------------------------------------------------------------------
/// @brief This function handels the whole program. 
/// @param argc is the count of arguments 
/// @param argv is the argument line 
/// @return 0 or the correct errorcode  
int main(int argc, char** argv)
{
  if(checkArgumentCount(argc) != 0)
  {
    return RETURN_CODE_INVALID_ARGC; 
  }

  FILE* file_ptr = openFileToRead(argv[1]); 

  if(file_ptr == NULL)
  {
    return RETURN_CODE_INVALID_FILE; 
  }

  



  fclose(file_ptr); 
  return 0; 
}

//------------------------------------------------------------------------------
/// @brief This function checks the amount of arguments from the commandline. 
/// @param argc is the count of arguments 
/// @return 0 if it is 2 or 255, if is is more or less than 2 
int checkArgumentCount(int argc)
{
  if(argc < 2)
  {
    printf("%s", ERROR_MESSAGE_INVALID_ARGC_BELOW_2);
    return RETURN_CODE_INVALID_ARGC;
  }
  else if(argc > 2)
  {
    printf("%s", ERROR_MESSAGE_INVALID_ARGC_ABOVE_2);
    return RETURN_CODE_INVALID_ARGC;
  }
  return 0; 
} 

//------------------------------------------------------------------------------
/// @brief This function opens the file wih the file name from the command line. 
/// @param file_name is the given file name from argv[1]
/// @return the file pointer or NULL if here is an error 
FILE* openFileToRead(char *file_name)
{
  FILE* file_ptr = fopen(file_name, "r");

  if(file_ptr == NULL)
  {
    printf("%s", ERROR_MESSAGE_IVALID_FILE); 
    return NULL; 
  }
  return file_ptr; 
}



