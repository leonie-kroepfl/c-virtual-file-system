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
// #include <ctype.h>
#include <assert.h>

#define RETURN_CODE_INVALID_ARGC 255 
#define ERROR_MESSAGE_INVALID_ARGC_BELOW_2 "[ERROR] No config file provided, exiting...\n"
#define ERROR_MESSAGE_INVALID_ARGC_ABOVE_2 "[ERROR] Too many arguments, exiting...\n"
#define RETURN_CODE_INVALID_FILE 254
#define ERROR_MESSAGE_INVALID_FILE "[ERROR] File could not be opened, exiting...\n" 
#define ERROR_MESSAGE_INVALID_MAGIC_NUMBER "[ERROR] Invalid config file, exiting...\n" 
#define RETURN_CODE_OUT_OF_MEMORY 123
#define ERROR_MESSAGE_OUT_OF_MEMORY "[ERROR] Not able to allocate space, exiting...\n" 

#define FALSE 0
#define TRUE 1 
#define OKAY 2
#define ERROR 3
#define END_OF_FILE 4
#define NO_CHILDREN 5 

#define BUFFER_SIZE 256 
#define TYPE_DIRECTORY "DIR"
#define TYPE_FILE "FILE"
#define OPENING_INDICATOR '<'
#define CLOSING_INDICATOR '>'
#define WHITESPACE ' '
#define NEWLINE '\n'
#define OPENING_CONTENT_INDICATOR '{'
#define CLOSING_CONTENT_INDICATOR '}'

typedef enum _Permission_
{
  NONE,
  WRITE,
  READ,
  READ_AND_WRITE,
  INVALID_PERMISSION,
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
  int directory_child_count_; 
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
Node* allocateNewNode(void); 
Node* saveNewNode(void); 
Pointer allocateNewString(int length_of_string); 
Pointer reallocateNewString(int length_of_new_string, char* old_string); 
Pointer saveNewString(char node_detail[]);
Node* saveRootNode(Filesystem* filesystem);
Node* saveNewNodeFromFile(FILE* file_ptr); 
int ignoreWhitespaces(FILE* file_ptr); 
Pointer getValidNodeType(FILE* file_ptr); 
int checkValidationOfType(char node_type[]); 
Pointer getValidNodeName(FILE* file_ptr); 
int checkValidationOfName(char tmp_character); 
Permission getValidNodePermission(FILE* file_ptr); 
int checkValidationOfPermission(Permission node_permission); 
Pointer getValidNodeContentForFiles(FILE* file_ptr); 
void freeAllocatedStrings(Node** current_node);
void freeAllocatedNode(Node* current_node);
Node* parseFile(FILE* file_ptr, Filesystem* filesystem);
int parseObjectFromFile(FILE* file_ptr, Node* parent_node); 
Node* createBasisOfTreeNode(FILE* file_ptr, Node* parent_node); 
int createContentOfTreeNode(FILE* file_ptr, Node* new_node); 
void linkNewNode(Node* new_node, Node* parent_node); 
void printFilesystemTree(Node* current_node, int tree_depth); 


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
    printf("%s", ERROR_MESSAGE_OUT_OF_MEMORY); 
    fclose(file_ptr);
    return RETURN_CODE_OUT_OF_MEMORY; 
  }

  filesystem->root_node_ = parseFile(file_ptr, filesystem); 
  if(filesystem->root_node_ == NULL)
  {
    printf("%s", ERROR_MESSAGE_OUT_OF_MEMORY); 
    fclose(file_ptr);
    free(filesystem);
    filesystem = NULL; 
    return RETURN_CODE_OUT_OF_MEMORY; 
  } 

  printFilesystemTree(filesystem->root_node_, 1); 


  fclose(file_ptr); 
  freeAllocatedNode(filesystem->root_node_);
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
    printf("%s", ERROR_MESSAGE_INVALID_FILE); 
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

  printf("%s", ERROR_MESSAGE_INVALID_MAGIC_NUMBER);
  return RETURN_CODE_INVALID_FILE; 
} 

//------------------------------------------------------------------------------
/// @brief This function allocates memory for a string. 
/// @param length_of_string is the given length of the new string
/// @return the string pointer or NULL if there is an error 
Pointer allocateNewString(int length_of_string)
{
  Pointer tmp_ptr = calloc(length_of_string + 1, sizeof(char)); 

  if(tmp_ptr == NULL)
  {
    printf("%s", ERROR_MESSAGE_OUT_OF_MEMORY);
    return NULL;
  } 

  return tmp_ptr; 
} 

//------------------------------------------------------------------------------
/// @brief This function reallocates memory for a new string andcopys the old one in it. 
/// @param length_of_new_string is the given length of the new string
/// @param old_string is the given old string which should be rallocated 
/// @return the string pointer or NULL if there is an error 
Pointer reallocateNewString(int length_of_new_string, char* old_string)
{
  Pointer tmp_ptr = realloc(old_string, (length_of_new_string + 1) * sizeof(char)); 

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

  strcpy(tmp_ptr, node_detail); 
  return tmp_ptr; 
}

//------------------------------------------------------------------------------
/// @brief This function allocates memory for a tree node. 
/// @return the node pointer or NULL if there is an error 
Node* allocateNewNode(void)
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
/// @brief This function frees all the strings allocated earlier. 
/// @param current_node is the node with the strings which get freed now 
void freeAllocatedStrings(Node** current_node)
{
  if((*current_node)->type_ != NULL)
  {
    if(strcmp((*current_node)->type_, TYPE_FILE) == 0)
    {
      free((*current_node)->file_content_);
      (*current_node)->file_content_ = NULL; 
    }

    free((*current_node)->type_);
    (*current_node)->type_= NULL;
  }

  if((*current_node)->name_ != NULL)
  {
    free((*current_node)->name_);
    (*current_node)->name_ = NULL; 
  }
}

//------------------------------------------------------------------------------
/// @brief This function frees all the nodes allocated earlier. 
/// @param current_node is the node which gets freed now 
void freeAllocatedNode(Node* current_node)
{
  if(current_node == NULL)
  {
    return; 
  }

  freeAllocatedStrings(&current_node); 
  freeAllocatedNode(current_node->directory_child_node_); 
  freeAllocatedNode(current_node->next_sibling_node_); 
  free(current_node); 
  current_node = NULL; 
}

//------------------------------------------------------------------------------
/// @brief This function saves the new allocated root node with details. 
/// @param filesystem is pointer to the filesystem 
/// @return 0 or the individual exit errorcode 
Node* saveRootNode(Filesystem* filesystem)
{
  filesystem->root_node_ = allocateNewNode();
  if(filesystem->root_node_ == NULL)
  {
    return NULL; 
  }

  filesystem->root_node_->type_ = saveNewString("root\0");
  if(filesystem->root_node_->type_ == NULL)
  {
    free(filesystem->root_node_); 
    return NULL; 
  }

  filesystem->root_node_->name_ = saveNewString("root\0");
  if(filesystem->root_node_->name_ == NULL)
  {
    free(filesystem->root_node_->type_); 
    free(filesystem->root_node_); 
    return NULL; 
  }

  filesystem->root_node_->permission_ = NONE; 
  filesystem->root_node_->parent_node_ = NULL; 
  filesystem->root_node_->next_sibling_node_ = NULL; 
  filesystem->root_node_->directory_child_node_ = NULL; 
  return filesystem->root_node_; 
}

//------------------------------------------------------------------------------
/// @brief This function saves a new valid node from the file. 
/// @param file_ptr is the pointer to the open file 

/// @return the node pointer or NULL if here is an error 
Node* saveNewNodeFromFile(FILE* file_ptr)
{
  Node* new_node = allocateNewNode(); 
  if(new_node == NULL)
  {
    return NULL; 
  }

  new_node->type_ = getValidNodeType(file_ptr); 
  if(new_node->type_ == NULL)
  {
    free(new_node); 
    return NULL; 
  }

  new_node->name_ = getValidNodeName(file_ptr); 
  if(new_node->name_ == NULL)
  {
    free(new_node->type_); 
    free(new_node);
    return NULL; 
  }

  new_node->permission_ = getValidNodePermission(file_ptr);
  if(new_node->permission_ == INVALID_PERMISSION)
  {
    free(new_node->type_); 
    free(new_node->name_); 
    free(new_node);
    return NULL; 
  }

  new_node->parent_node_ = NULL; 
  new_node->directory_child_count_ = 0; 
  new_node->next_sibling_node_ = NULL;
  new_node->directory_child_node_ = NULL;
  return new_node; 
} 

//------------------------------------------------------------------------------
/// @brief This function ignores the whitespaces in the file before the formated information. 
/// @param file_ptr is the pointer to the open file 
/// @return 0 or EOF if the file is at the end 
int ignoreWhitespaces(FILE* file_ptr)
{
  int lost_character; 

  while(1)
  {
    lost_character = fgetc(file_ptr); 
    if(lost_character == WHITESPACE || lost_character == NEWLINE)
    {
      continue;
    }

    return lost_character; 

    switch (lost_character) 
    {
      case EOF:
        return EOF; 
        break;
      case CLOSING_CONTENT_INDICATOR:
        return CLOSING_CONTENT_INDICATOR; 
        break; 
      case CLOSING_INDICATOR:
        break; 
      case NEWLINE:
        return NEWLINE; 
        break; 
      case OPENING_INDICATOR:
        return OPENING_INDICATOR; 
        break; 
      case OPENING_CONTENT_INDICATOR:
        return OKAY; 
        break; 
      default:
        break; 
    }
  }

  return lost_character; 
}

//------------------------------------------------------------------------------
/// @brief This function gets the valid node type from the file. 
/// @param file_ptr is the pointer to the open file 
/// @return the string pointer or NULL if here is an error 
Pointer getValidNodeType(FILE* file_ptr)
{
  char tmp_character; 
  char node_type[BUFFER_SIZE]; 
  int length_of_string = 0; 
  tmp_character = fgetc(file_ptr); // whitespace 

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
int checkValidationOfType(char node_type[])
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
  char node_name[BUFFER_SIZE]; 
  int length_of_string = 0; 
  tmp_character = fgetc(file_ptr); // whitespace 
  tmp_character = fgetc(file_ptr); // quotation mark 

  while((tmp_character = fgetc(file_ptr)) != '"') 
  {
    if(checkValidationOfName(tmp_character) == FALSE)
    {
      return NULL; 
    }

    node_name[length_of_string] = tmp_character; 
    length_of_string++; 
  }
  
  tmp_character = fgetc(file_ptr); // comma 
  node_name[length_of_string] = '\0'; 
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
Permission getValidNodePermission(FILE* file_ptr)
{
  char node_permssion = fgetc(file_ptr); // whitespace 
  node_permssion = fgetc(file_ptr); 
  node_permssion -= '0';
  char lost_character = fgetc(file_ptr); // comma 
  if(lost_character == WHITESPACE)
  {
    return INVALID_PERMISSION; 
  }

  if(checkValidationOfPermission(node_permssion) == FALSE)
  {
    return INVALID_PERMISSION; 
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
Pointer getValidNodeContentForFiles(FILE* file_ptr)
{
  int length_of_string = 0; 
  char tmp_character; 
  int buffer_size = BUFFER_SIZE; 
  char *node_content = allocateNewString(buffer_size); 
  if(node_content == NULL)
  {
    printf("%s", ERROR_MESSAGE_OUT_OF_MEMORY); 
    return NULL;
  }

  while((tmp_character = fgetc(file_ptr)) != CLOSING_CONTENT_INDICATOR && (int)tmp_character != EOF) 
  {
    if(buffer_size < length_of_string)
    {
      buffer_size += BUFFER_SIZE; 
      char* tmp_new_ptr = reallocateNewString(buffer_size, node_content);
      if(tmp_new_ptr == NULL)
      {
        printf("%s", ERROR_MESSAGE_OUT_OF_MEMORY); 
        return NULL;
      }
      node_content = tmp_new_ptr; 
    }

    node_content[length_of_string] = tmp_character; 
    length_of_string++; 
  }
  
  node_content[length_of_string] = '\0'; 
  char* tmp_new_ptr = reallocateNewString(length_of_string, node_content);

  if(length_of_string == 0)
  {
    return NULL; 
  }
  else if(tmp_new_ptr == NULL)
  {
    printf("%s", ERROR_MESSAGE_OUT_OF_MEMORY); 
    return NULL;
  }

  return tmp_new_ptr; 
}




Node* createBasisOfTreeNode(FILE* file_ptr, Node* parent_node)
{
  Node* new_node = saveNewNodeFromFile(file_ptr); 
  if(new_node == NULL)
  {
    return NULL; 
  }

  new_node->parent_node_ = parent_node; 
  linkNewNode(new_node, parent_node); 
  return new_node; 
}

int createContentOfTreeNode(FILE* file_ptr, Node* new_node)
{
  int returned_separator = ignoreWhitespaces(file_ptr); 

  if(strcmp(new_node->type_, TYPE_FILE) == 0)
  {
    new_node->file_content_ = getValidNodeContentForFiles(file_ptr); 
    return NO_CHILDREN; 
  }
  else if(strcmp(new_node->type_, TYPE_DIRECTORY) == 0)
  {
    if(returned_separator == OPENING_CONTENT_INDICATOR) 
    {
      int object_return_code = 0; 

      while(1)
      {
        returned_separator = ignoreWhitespaces(file_ptr); // less than or curved bracked closed or else 

        if(returned_separator == CLOSING_CONTENT_INDICATOR)
        {
          break; 
        }
        else if(returned_separator == OPENING_INDICATOR)
        {
          object_return_code = parseObjectFromFile(file_ptr, new_node);

          if(object_return_code == EOF)
          {
            return EOF; 
          }
          else if(object_return_code == ERROR)
          {
            return ERROR; 
          }
        }
      }
      return OKAY; 
    }
    else 
    {
      return ERROR; 
    }
  }
  return ERROR; 
}


void linkNewNode(Node* new_node, Node* parent_node)
{
  if(parent_node->directory_child_count_ == 0)
  {
    parent_node->directory_child_node_ = new_node; 
    parent_node->directory_child_count_++; 
    return; 
  }

  Node* current_node = parent_node->directory_child_node_; 

  while(current_node->next_sibling_node_ != NULL)
  {
    current_node = current_node->next_sibling_node_; 
  }

  if(current_node->next_sibling_node_ == NULL)
  {
    current_node->next_sibling_node_ = new_node; 
    parent_node->directory_child_count_++; 
  }

  return; 
}

int parseObjectFromFile(FILE* file_ptr, Node* parent_node) 
{
  Node* new_node = createBasisOfTreeNode(file_ptr, parent_node); 
  if(new_node == NULL)
  {
    return ERROR; 
  }
  
  int content_return_code = createContentOfTreeNode(file_ptr, new_node); 

  if(content_return_code == ERROR)
  {
    return ERROR; 
  }
  else if(content_return_code == EOF)
  {
    return END_OF_FILE; 
  }

  return OKAY; 
}

Node* parseFile(FILE* file_ptr, Filesystem* filesystem)
{
  filesystem->root_node_ = saveRootNode(filesystem);
  if(filesystem->root_node_ == NULL)
  {
    return NULL; 
  }

  while(1)
  {
    int return_code = 0; 
    int returned_separator = ignoreWhitespaces(file_ptr); // more than or else 

    if(returned_separator == OPENING_INDICATOR)
    {
      return_code = parseObjectFromFile(file_ptr, filesystem->root_node_); 

      if(return_code == ERROR)
      {
        printFilesystemTree(filesystem->root_node_, 1); 
        freeAllocatedNode(filesystem->root_node_);
        return NULL; 
      }
      else if(return_code == END_OF_FILE)
      {
        return filesystem->root_node_; 
      }
      else if(return_code == OKAY)
      {
        continue;
      }
    }
    
    if(returned_separator == EOF)
    {
      return filesystem->root_node_; 
    }
  }
  return filesystem->root_node_; 
}



void printFilesystemTree(Node* current_node, int tree_depth)
{
  if(current_node == NULL)
  {
    return; 
  }

  for(int depth_index = 0; depth_index < tree_depth; depth_index++)
  {
    printf("\t"); 
  }

  printf("|-- %s\n", current_node->name_);
    printf("|-- %s\n", current_node->type_);
    printf("|-- %d\n", current_node->permission_);
    printf("|-- %s\n", current_node->file_content_);
    printf("|-- %p\n", current_node->directory_child_node_); 
    printf("|-- %p\n", current_node->next_sibling_node_);
    printf("|-- %p\n", current_node->parent_node_);

  printFilesystemTree(current_node->directory_child_node_, tree_depth + 1); 
  printFilesystemTree(current_node->next_sibling_node_, tree_depth); 
}

