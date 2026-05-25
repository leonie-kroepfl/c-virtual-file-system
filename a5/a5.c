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
#define ERROR_MESSAGE_IVALID_MAGIC_NUMBER "[ERROR] Invalid config file, exiting...\n" 
#define RETURN_CODE_OUT_OF_MEMORY 123
#define ERROR_MESSAGE_OUT_OF_MEMORY "[ERROR] Not able to allocate space, exiting...\n" 

#define FALSE 1
#define TRUE 0 

#define BUFFER_SIZE 256 
#define TYPE_DIRECTORY "DIR"
#define TYPE_FILE "FILE"
#define OPENING_INDICATOR '<'
#define CLOSING_INDICATOR '>'
#define WHITESPACE ' '

typedef enum _Permission_
{
  NONE,
  WRITE,
  READ,
  READ_AND_WRITE,
} Permission;

typedef char* Pointer;
typedef struct _Node_ Node;

typedef struct _Node_
{
  Pointer type_; 
  Pointer name_; 
  Permission permission_;
  Node* parent_node_; 
  Node* next_sibling_node_; 
  Node* directory_child_node_; 
  Pointer file_content_; 
} Node; 

typedef struct _Filesystem_
{
  Node* root_node_;
  Node* current_working_directory_;  
} Filesystem;

int checkArgumentCount(int argc); 
FILE* openFileToRead(char *file_name);
int checkForMagicNumber(FILE* file_ptr); 
Node* allocateNewNode(); 
Node* saveNewNode(); 
Pointer allocateNewString(int length_of_string); 
Pointer saveNewString(char node_detail[]);
int saveRootNode(Filesystem* filesystem);
Node* saveNewNodeFromFile(FILE* file_ptr); 
void ignoreWhitespaces(FILE* file_ptr); 
Pointer getValidNodeType(FILE* file_ptr); 
int checkValidationOfType(char* node_type); 
Pointer getValidNodeName(FILE* file_ptr); 
int checkValidationOfName(char tmp_character); 
Permission getValidNodePermission(FILE* file_ptr); 
int checkValidationOfPermission(Permission node_permission); 
Pointer getValidNodeContent(FILE* file_ptr); 


//------------------------------------------------------------------------------
/// @brief This function handels the whole program. 
/// @param argc is the count of arguments 
/// @param argv is the argument line 
/// @return 0 or the correct errorcode  
int main(int argc, char** argv)
{
  if(checkArgumentCount(argc) != TRUE)
  {
    return RETURN_CODE_INVALID_ARGC; 
  }

  FILE* file_ptr = openFileToRead(argv[1]); 
  if(file_ptr == NULL)
  {
    return RETURN_CODE_INVALID_FILE; 
  }

  if(checkForMagicNumber(file_ptr) != TRUE)
  {
    fclose(file_ptr); 
    return RETURN_CODE_INVALID_FILE; 
  }

  Filesystem *filesystem = calloc(1, sizeof(Filesystem)); 
  if(filesystem == NULL)
  {
    fclose(file_ptr);
    return RETURN_CODE_OUT_OF_MEMORY; 
  }

  filesystem->root_node_ = allocateNewNode();
  if(filesystem->root_node_ == NULL)
  {
    fclose(file_ptr);
    return RETURN_CODE_OUT_OF_MEMORY; 
  }

  if(saveRootNode(filesystem) != TRUE)
  {
    printf("%s", ERROR_MESSAGE_OUT_OF_MEMORY); 
    fclose(file_ptr);
    free(filesystem->root_node_);
    free(filesystem);
    filesystem = NULL; 
    return RETURN_CODE_OUT_OF_MEMORY; 
  }

  filesystem->root_node_->directory_child_node_ = saveNewNodeFromFile(file_ptr); 
  if(filesystem->root_node_->directory_child_node_ == NULL)
  {
    printf("%s", ERROR_MESSAGE_OUT_OF_MEMORY); 
    fclose(file_ptr);
    free(filesystem->root_node_->directory_child_node_);
    free(filesystem->root_node_);
    free(filesystem);
    filesystem = NULL; 
    return RETURN_CODE_OUT_OF_MEMORY; 
  }



  fclose(file_ptr); 
    free(filesystem->root_node_->directory_child_node_);
    free(filesystem->root_node_);
    free(filesystem);
    filesystem = NULL; 
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
  return TRUE; 
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

//------------------------------------------------------------------------------
/// @brief This function checks the file for the magic number. 
/// @param file_ptr is the pointer to the open file 
/// @return 0 or the individual exit errorcode 
int checkForMagicNumber(FILE* file_ptr)
{
  char first_line[BUFFER_SIZE]; 
  if(fgets(first_line, BUFFER_SIZE, file_ptr) == NULL)
  {
    return RETURN_CODE_INVALID_FILE; 
  }

  if(strcmp(first_line, "# ISP 2026 A5 valid config #\n") == 0)
  {
    return TRUE; 
  }

  printf("%s", ERROR_MESSAGE_IVALID_MAGIC_NUMBER);
  return RETURN_CODE_INVALID_FILE; 
} 

//------------------------------------------------------------------------------
/// @brief This function allocates memory for a string. 
/// @param length_of_string is the given length of the new string
/// @return the string pointer or NULL if there is an error 
Pointer allocateNewString(int length_of_string)
{
  Pointer tmp_ptr = calloc(length_of_string, sizeof(char) + 1); 

  if(tmp_ptr == NULL)
  {
    printf("%s", ERROR_MESSAGE_OUT_OF_MEMORY);
    return NULL;
  } 

  return tmp_ptr; 
} 

//------------------------------------------------------------------------------
/// @brief This function saves the new allocated memory in the node with details. 
/// @param node_detail is the detail for the node 
/// @return the string pointer or NULL if there is an error 
Pointer saveNewString(char node_detail[])
{
  int length_of_string = (int)strlen(node_detail); 
  Pointer tmp_ptr = allocateNewString(length_of_string); 
  if(tmp_ptr == NULL)
  {
    return NULL; 
  }

  node_detail[length_of_string] = '\0'; 
  strcpy(tmp_ptr, node_detail); 
  return tmp_ptr; 
}

//------------------------------------------------------------------------------
/// @brief This function allocates memory for a tree node. 
/// @return the node pointer or NULL if there is an error 
Node* allocateNewNode()
{
  Node* tmp_ptr = calloc(1, sizeof(Node));

  if(tmp_ptr == NULL)
  {
    printf("%s", ERROR_MESSAGE_OUT_OF_MEMORY);
    return NULL;
  } 

  return tmp_ptr; 
}

//------------------------------------------------------------------------------
/// @brief This function saves the new allocated root node with details. 
/// @param filesystem is pointer to the filesystem 
/// @return 0 or the individual exit errorcode 
int saveRootNode(Filesystem* filesystem)
{
  filesystem->root_node_->type_ = saveNewString("root");
  if(filesystem->root_node_->type_ == NULL)
  {
    return RETURN_CODE_OUT_OF_MEMORY; 
  }

  filesystem->root_node_->name_ = saveNewString("root");
  if(filesystem->root_node_->name_ == NULL)
  {
    return RETURN_CODE_OUT_OF_MEMORY; 
  }

  filesystem->root_node_->permission_ = NONE; 
  filesystem->root_node_->parent_node_ = NULL; 
  filesystem->root_node_->file_content_ = NULL; 
  filesystem->root_node_->next_sibling_node_ = NULL; 
  filesystem->root_node_->directory_child_node_ = NULL; 
  return TRUE; 
}

//------------------------------------------------------------------------------
/// @brief This function saves a new valid node from the file. 
/// @param file_ptr is the pointer to the open file 
/// @return the node pointer or NULL if here is an error 
Node* saveNewNodeFromFile(FILE* file_ptr)
{
  Node* tmp_node; 
  ignoreWhitespaces(file_ptr); 
  tmp_node->type_ = getValidNodeType(file_ptr); 
  if(tmp_node->type_ == NULL)
  {
    return NULL; 
  }

  tmp_node->name_ = getValidNodeName(file_ptr); 
  if(tmp_node->name_ == NULL)
  {
    return NULL; 
  }

  tmp_node->permission_ = getValidNodePermission(file_ptr);
  if(tmp_node->permission_ == -1)
  {
    return NULL; 
  }

  tmp_node->file_content_ = getValidNodeContent(file_ptr); 
  if(tmp_node->file_content_ == NULL)
  {
    return NULL; 
  }

  return tmp_node; 
} 

//------------------------------------------------------------------------------
/// @brief This function ignores the whitespaces in the file before the formated information. 
/// @param file_ptr is the pointer to the open file 
void ignoreWhitespaces(FILE* file_ptr)
{
  char lost_character; 

  while((lost_character = fgetc(file_ptr)) != OPENING_INDICATOR)
  {
    continue;
  }
}

//------------------------------------------------------------------------------
/// @brief This function gets the valid node type from the file. 
/// @param file_ptr is the pointer to the open file 
/// @return the string pointer or NULL if here is an error 
Pointer getValidNodeType(FILE* file_ptr)
{
  char tmp_character; 
  char *node_type; 
  int length_of_string = 0; 
  char single_whitespace = fgetc(file_ptr); 

  while((tmp_character = fgetc(file_ptr)) != ',') 
  {
    node_type[length_of_string] = tmp_character; 
    length_of_string++; 
  }

  node_type[length_of_string] = '\0'; 

  if(checkValidationOfType(node_type) == TRUE)
  {
    return saveNewString(node_type); 
  }

  return NULL;
}

//------------------------------------------------------------------------------
/// @brief This function checks the infromaton from the file for validation. 
/// @param node_type is the information from the file 
/// @return TRUE or FALSE if the string is valid or not 
int checkValidationOfType(char* node_type)
{
  if(strcmp(node_type, "DIR") == 0)
  {
    return TRUE; 
  }
  else if(strcmp(node_type, "FILE") == 0)
  {
    return TRUE; 
  }

  return FALSE; 
}

//------------------------------------------------------------------------------
/// @brief This function gets the valid node name from the file. 
/// @param file_ptr is the pointer to the open file 
/// @return the string pointer or NULL if here is an error 
Pointer getValidNodeName(FILE* file_ptr)
{
  char tmp_character; 
  char *node_name; 
  int length_of_string = 0; 
  char single_whitespace = fgetc(file_ptr); 
  char lost_character = fgetc(file_ptr); 

  while((tmp_character = fgetc(file_ptr)) != '"') 
  {
    if(checkValidationOfName(tmp_character) == FALSE)
    {
      return NULL; 
    }

    node_name[length_of_string] = tmp_character; 
    length_of_string++; 
  }
  
  lost_character = fgetc(file_ptr); 
  return saveNewString(node_name); 
}

//------------------------------------------------------------------------------
/// @brief This function checks the infromaton from the file for validation. 
/// @param tmp_character is one character from the information from the file 
/// @return TRUE or FALSE if the string is valid or not 
int checkValidationOfName(char tmp_character)
{
  if(tmp_character >= 'a' && tmp_character <= 'z')
  {
    return TRUE;
  }
  else if(tmp_character >= 'A' && tmp_character <= 'Z')
  {
    return TRUE;
  }
  else if(tmp_character >= '#' && tmp_character <= '%')
  {
    return TRUE;
  }
  else if(tmp_character == '@' || tmp_character == '!' || tmp_character == '_')
  {
    return TRUE;
  }
  else if(tmp_character >= '0' && tmp_character <= '9')
  {
    return TRUE;
  }

  return FALSE; 
} 

//------------------------------------------------------------------------------
/// @brief This function gets the valid node permission from the file. 
/// @param file_ptr is the pointer to the open file 
/// @return the permission status or NONE if there is an error 
Permission getNodePermission(FILE* file_ptr)
{
  char single_whitespace = fgetc(file_ptr); 
  Permission node_permssion = fgetc(file_ptr);   
  char lost_character = fgetc(file_ptr); 

  if(checkValidationOfPermission(node_permssion) == FALSE)
  {
    return NONE; 
  }

  return node_permssion; 
}

//------------------------------------------------------------------------------
/// @brief This function checks the infromaton from the file for validation. 
/// @param node_permission is one number from the file for the permission status 
/// @return TRUE or FALSE if the string is valid or not 
int checkValidationOfPermission(Permission node_permission)
{
  if(node_permission >= NONE && node_permission <= READ_AND_WRITE)
  {
    return TRUE;
  }

  return FALSE; 
} 

//------------------------------------------------------------------------------
/// @brief This function gets the valid node content from the file. 
/// @param file_ptr is the pointer to the open file 
/// @return the string pointer or NULL if here is an error 
Pointer getValidNodeContent(FILE* file_ptr)
{
  char tmp_character; 
  char *node_content; 
  int length_of_string = 0; 
  char single_whitespace = fgetc(file_ptr); 
  char lost_character = fgetc(file_ptr); 

  while((tmp_character = fgetc(file_ptr)) != '}' && (int)tmp_character != 0) 
  {
    node_content[length_of_string] = tmp_character; 
    length_of_string++; 
  }
  
  single_whitespace = fgetc(file_ptr); 
  lost_character = fgetc(file_ptr); 
  return saveNewString(node_content); 
}





