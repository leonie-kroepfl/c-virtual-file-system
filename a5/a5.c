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

#define ERROR_MESSAGE_COMMAND_PARAMETER "[ERROR] Parameters not correct\n"
#define ERROR_MESSAGE_COMMAND_PATH "[ERROR] Invalid path\n"
#define ERROR_MESSAGE_COMMAND_NODE "[ERROR] Node not found\n"
#define ERROR_MESSAGE_COMMAND_UNKNOWN "[ERROR] Command not found\n"
#define ERROR_MESSAGE_PERMISSION_READ "[ERROR] No reading permissions for: "
#define ERROR_MESSAGE_PERMISSION_WRITE "[ERROR] No writing permission for: "
#define ERROR_MESSAGE_EMPTY_FILE "Empty file\n"


#define FALSE 0
#define TRUE 1 
#define OKAY 2
#define ERROR 3
#define END_OF_FILE 4
#define NO_CHILDREN 5 
#define EXIT 6

#define BUFFER_SIZE 256 
#define MAX_TOKENS 3
#define MAX_COLUMNS 4
#define COLUMN_SPACES "    "
#define TYPE_DIRECTORY "DIR"
#define TYPE_FILE "FILE"
#define OPENING_INDICATOR '<'
#define CLOSING_INDICATOR '>'
#define WHITESPACE ' '
#define NEWLINE '\n'
#define OPENING_CONTENT_INDICATOR '{'
#define CLOSING_CONTENT_INDICATOR '}'
#define SHELL_OPENING " > "
#define PATH_SEPARATOR '/'

#define COMMAND_CWD "cwd\n"
#define COMMAND_LS "ls"
#define COMMAND_CD "cd"
#define COMMAND_INFO "info"
#define COMMAND_CAT "cat"
#define COMMAND_TREE "tree"
#define COMMAND_TOUCH "touch"
#define COMMAND_MKDIR "mkdir"
#define COMMAND_RM "rm"
#define VALID_EXIT_CODE_1 "EXIT\n"
#define VALID_EXIT_CODE_2 "ExIt\n"
#define VALID_EXIT_CODE_3 "exit\n"

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
  int file_content_size; 
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
int ignoreWhitespacesInFile(FILE* file_ptr); 
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
void printShell(Filesystem* filesystem, Node* current_directory); 
int handleInput(Filesystem* filesystem); 
int checkInputForExitCode(char tmp_input[]); 
void ignoreWhitespacesInInput(char tmp_input[]); 
void executeCommandArgument(Filesystem* filesystem, char* input[]); 
void printPath(Filesystem* filesystem, Node* current_directory); 
int printContentFromFile(Node* current_directory, char* file_name); 
void handleCatCommand(Filesystem* filesystem, Node* current_directory, char* input[]); 
void handleCwdCommand(Filesystem* filesystem, char* input[]);
void handleLsCommand(Filesystem* filesystem, char* input[]); 
int getColumnWidth(Node* parent_node);
void printContentFromDirectory(Node* child_node, int child_index,int column_width); 
int changePath(Filesystem* filesystem, Node* current_directory, char* path[], int path_index, int return_code); 
void handleCdCommand(Filesystem* filesystem, char* input[]); 
void removeNewline(char* path[]); 
void separateInput(char* input[], char* path[]); 
void handleInfoCommand(Node* child_node, char* input[]); 
void printInformationFromNode(Node* child_node); 
void handleTreeCommand(Filesystem* filesystem, char* input[]); 
void handleTouchAndMkdirCommand(Filesystem* filesystem, char* input[], char object_type[]); 
Node* saveNewNodeFromUser(char* input[], char object_type[], char object_name[]); 
int getSizeFromFileContent(Node* file_node); 
void handleRmCommand(Filesystem* filesystem, char* input[]); 
void freeChildNode(Node* target_node); 




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

  filesystem->current_working_directory_ = filesystem->root_node_; 
  // printFilesystemTree(filesystem->root_node_, 1); 
  while(1)
  {
    printShell(filesystem, filesystem->current_working_directory_); 
    if(handleInput(filesystem) == EXIT)
    {
      break;
    }
  }

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
/// @return the root node pointer or NULL if there is an error 
Node* saveRootNode(Filesystem* filesystem)
{
  filesystem->root_node_ = allocateNewNode();
  if(filesystem->root_node_ == NULL)
  {
    return NULL; 
  }

  filesystem->root_node_->type_ = saveNewString("DIR\0");
  if(filesystem->root_node_->type_ == NULL)
  {
    free(filesystem->root_node_); 
    return NULL; 
  }

  filesystem->root_node_->name_ = saveNewString("user@/isp\0");
  if(filesystem->root_node_->name_ == NULL)
  {
    free(filesystem->root_node_->type_); 
    free(filesystem->root_node_); 
    return NULL; 
  }

  filesystem->root_node_->permission_ = READ_AND_WRITE; 
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
  new_node->file_content_size = 0; 
  new_node->next_sibling_node_ = NULL;
  new_node->directory_child_node_ = NULL;
  return new_node; 
} 

//------------------------------------------------------------------------------
/// @brief This function ignores the whitespaces in the file before the formated information. 
/// @param file_ptr is the pointer to the open file 
/// @return 0 or EOF if the file is at the end 
int ignoreWhitespacesInFile(FILE* file_ptr)
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
  }
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
  else if(tmp_character == '.' || tmp_character == ' ')
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

//------------------------------------------------------------------------------
/// @brief This function creates a new node from the file. 
/// @param file_ptr is the pointer to the open file 
/// @param parent_node is the parent node from the new child 
/// @return the string pointer node or NULL if here is an error 
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

//------------------------------------------------------------------------------
/// @brief This function gets the size of the file content. 
/// @param file_node is the new file node 
/// @return the size of the file content 
int getSizeFromFileContent(Node* file_node)
{
  int content_index = 0; 

  while(file_node->file_content_[content_index] != '\0')
  {
    content_index++; 
  }

  return content_index; 
}

//------------------------------------------------------------------------------
/// @brief This function gets the content for a file object or handels the children for a directory object. 
/// @param file_ptr is the pointer to the open file 
/// @param new_node is the new node object created from the file 
/// @return the correct return code 
int createContentOfTreeNode(FILE* file_ptr, Node* new_node)
{
  int returned_separator = ignoreWhitespacesInFile(file_ptr); 

  if(strcmp(new_node->type_, TYPE_FILE) == 0)
  {
    new_node->file_content_ = getValidNodeContentForFiles(file_ptr); 
    new_node->file_content_size = getSizeFromFileContent(new_node); 
    return NO_CHILDREN; 
  }
  else if(strcmp(new_node->type_, TYPE_DIRECTORY) == 0)
  {
    if(returned_separator == OPENING_CONTENT_INDICATOR) 
    {
      int object_return_code = 0; 

      while(1)
      {
        returned_separator = ignoreWhitespacesInFile(file_ptr); // less than or curved bracked closed or else 

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

//------------------------------------------------------------------------------
/// @brief This function links the new node object to its parent. 
/// @param new_node is the new node object created from the file 
/// @param parent_node is the parent node from the new child 
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

//------------------------------------------------------------------------------
/// @brief This function parses one whole object from the file. 
/// @param file_ptr is the pointer to the open file 
/// @param parent_node is the parent node from the new child object 
/// @return the correct return code 
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

//------------------------------------------------------------------------------
/// @brief This function handels the parsing into the tree from the file. 
/// @param file_ptr is the pointer to the open file 
/// @param filesystem is the pointer to the tree 
/// @return the root pointer node from the tree or NULL if here is an error 
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
    int returned_separator = ignoreWhitespacesInFile(file_ptr); // more than or else 

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

//------------------------------------------------------------------------------
/// @brief This function prints the whole tree system. 
/// @param current_node is the pointer to the current node position 
/// @param tree_depth is the depth of the tree from the root 
void printFilesystemTree(Node* current_node, int tree_depth)
{
  if(current_node == NULL)
  {
    return; 
  }

  for(int depth_index = 0; depth_index < tree_depth; depth_index++)
  {
    printf("    "); 
  }

  printf("|-- %s\n", current_node->name_);
  printFilesystemTree(current_node->directory_child_node_, tree_depth + 1); 
  printFilesystemTree(current_node->next_sibling_node_, tree_depth); 
}

//------------------------------------------------------------------------------
/// @brief This function prints the shell path before the input. 
/// @param filesystem is the pointer to the tree 
/// @param current_directory is the pointer to the current directory position 
void printShell(Filesystem* filesystem, Node* current_directory)
{
  printf("%s", filesystem->root_node_->name_); 
  
  if(current_directory != NULL && current_directory->parent_node_ != NULL)
  {
    if(current_directory != filesystem->root_node_)
    {
      if(current_directory->parent_node_ == filesystem->root_node_)
      {
        printPath(filesystem, filesystem->current_working_directory_); 
      } 
      else 
      {
        printf("%c..", PATH_SEPARATOR); 
        printf("%s%c", current_directory->name_, PATH_SEPARATOR); 
      }
    }
  }

  printf("%s", SHELL_OPENING); 
}

//------------------------------------------------------------------------------
/// @brief This function prints the path within the shell. 
/// @param filesystem is the pointer to the tree 
/// @param current_directory is the pointer to the current directory position 
void printPath(Filesystem* filesystem, Node* current_directory)
{
  if(current_directory == NULL)
  {
    return; 
  }
  else if(current_directory->parent_node_ != NULL)
  {
    printPath(filesystem, current_directory->parent_node_); 
  }

  if(current_directory != filesystem->root_node_)
  {
    printf("%c%s", PATH_SEPARATOR, current_directory->name_); 
  }
}

//------------------------------------------------------------------------------
/// @brief This function prints the content from the file in the current directory. 
/// @param current_child is the pointer to the current child position 
/// @param input is the inputed file name from the user, which should be printed
/// @return OKAY or FALSE if there is an error 
int printContentFromFile(Node* current_child, char* file_name)
{
  if(current_child != NULL)
  {
    if(strcmp(current_child->name_, file_name) != 0)
    {

      int return_code = printContentFromFile(current_child->next_sibling_node_, file_name); 
      if(return_code == OKAY)
      {
        return OKAY;
      } 
      else if(return_code == ERROR)
      {
        return ERROR; 
      }
    }

    if(current_child->file_content_ != NULL)
    {
      printf("%s\n", current_child->file_content_); 
      return OKAY; 
    }
    else if(current_child->file_content_ == NULL && current_child->next_sibling_node_ != NULL)
    {
      printf("%s", ERROR_MESSAGE_EMPTY_FILE); 
      return ERROR; 
    }
  }

  return FALSE; 
}

//------------------------------------------------------------------------------
/// @brief This function ignores the useless whitespaces in the input from the user. 
/// @param tmp_input is the inputed data from the user with any amount of whitespaces
void ignoreWhitespacesInInput(char tmp_input[])
{
  int input_index = 0; 

  while(tmp_input[input_index] == WHITESPACE)
  {
    char tmp_char = tmp_input[input_index]; 
    tmp_input[input_index] = tmp_input[input_index + 1];
    tmp_input[input_index + 1] = tmp_char; 
  }
}

//------------------------------------------------------------------------------
/// @brief This function handles the input from the user. 
/// @param filesystem is the pointer to the tree 
/// @return OKAY or the correct exit code if there is an error 
int handleInput(Filesystem* filesystem)
{
  char tmp_input[BUFFER_SIZE]; 
  if(fgets(tmp_input, BUFFER_SIZE, stdin) == NULL)
  {
    printf("EMPTY"); 
    return ERROR; 
  }

  if(strchr(tmp_input, NEWLINE) == NULL)
  {
    int lost_input;

    while((lost_input = getchar()) != NEWLINE && lost_input != EOF)
    {
      continue;
    }
  }

  if(checkInputForExitCode(tmp_input) == TRUE)
  {
    return EXIT; 
  }

  ignoreWhitespacesInInput(tmp_input); 
  char* input[MAX_TOKENS]; 
  input[0] = strtok(tmp_input, " ");
  if(input[0] == NULL)
  {
    return ERROR; 
  }

  char *tmp_token; 

  for(int token_index = 1; token_index < MAX_TOKENS; token_index++)
  {
    tmp_token = strtok(NULL, " ");
    input[token_index] = tmp_token; 
  }

  executeCommandArgument(filesystem, input); 
  return OKAY; 
}

//------------------------------------------------------------------------------
/// @brief This function checks the input from the user for exit. 
/// @param tmp_input is the inputed data from the user
/// @return TRUE or FALSE if there is no exit inputed from the user 
int checkInputForExitCode(char tmp_input[])
{
  if((strcmp(tmp_input, VALID_EXIT_CODE_1) == 0) ||
    (strcmp(tmp_input, VALID_EXIT_CODE_2) == 0) ||
    (strcmp(tmp_input, VALID_EXIT_CODE_3) == 0))
  {
    return TRUE;
  }

  return FALSE; 
}

//------------------------------------------------------------------------------
/// @brief This function executes the inputed command for the user. 
/// @param filesystem is the pointer to the tree 
/// @param input is the inputed data from the user
void executeCommandArgument(Filesystem* filesystem, char* input[])
{
  if((strcmp(input[0], "exit") == 0) || (strcmp(input[0], "ExIt") == 0) || 
    (strcmp(input[0], "EXIT") == 0))
  {
    printf("%s", ERROR_MESSAGE_COMMAND_PARAMETER); 
    return; 
  }

  if(strcmp(input[0], COMMAND_CWD) == 0)
  {
    handleCwdCommand(filesystem, input); 
  }
  else if(strstr(input[0], COMMAND_CAT) != NULL)
  {
    handleCatCommand(filesystem, filesystem->current_working_directory_, input); 
  }
  else if(strstr(input[0], COMMAND_LS) != NULL)
  {
    handleLsCommand(filesystem, input);
  }
  else if(strstr(input[0], COMMAND_CD) != NULL)
  {
    handleCdCommand(filesystem, input);
  }
  else if(strstr(input[0], COMMAND_INFO) != NULL)
  {
    handleInfoCommand(filesystem->current_working_directory_->directory_child_node_, input);
  }
  else if(strstr(input[0], COMMAND_TREE) != NULL)
  {
    handleTreeCommand(filesystem, input);
  }
  else if(strstr(input[0], COMMAND_TOUCH) != NULL)
  {
    handleTouchAndMkdirCommand(filesystem, input, "FILE\0");
  }
  else if(strstr(input[0], COMMAND_MKDIR) != NULL)
  {
    handleTouchAndMkdirCommand(filesystem, input, "DIR\0");
  }
  else if(strstr(input[0], COMMAND_RM) != NULL)
  {
    handleRmCommand(filesystem, input);
  }
  else 
  {
    printf("%s", ERROR_MESSAGE_COMMAND_UNKNOWN); 
  }
  return; 
} 

//------------------------------------------------------------------------------
/// @brief This function executes the cat command for the user. 
/// @param filesystem is the pointer to the tree 
/// @param current_directory is the pointer to the current directory position 
/// @param input is the inputed data from the user
void handleCatCommand(Filesystem* filesystem, Node* current_directory, char* input[])
{
  Node* tmp_current_directory = filesystem->current_working_directory_; 

  if(input[1] != NULL)
  {
    if(input[2] != NULL)
    {
      printf("%s", ERROR_MESSAGE_COMMAND_PARAMETER); 
      return; 
    }

    char* path[MAX_TOKENS]; 
    separateInput(input, path); 

    if(path[0] != NULL)
    {
      for(int path_index = 0; path_index < MAX_TOKENS - 1; path_index++)
      {
        if(path[path_index + 1] == NULL)
        {
          char* file_name = path[path_index]; 
          path[path_index] = 0; 
          
          if(changePath(filesystem, current_directory, path, path_index, 0) == ERROR)
          {
            return; 
          } 

          printContentFromFile(filesystem->current_working_directory_->directory_child_node_, file_name);
          break; 
        }
      }
    }
  }

  filesystem->current_working_directory_ = tmp_current_directory; 
}

//------------------------------------------------------------------------------
/// @brief This function executes the cwd command for the user. 
/// @param filesystem is the pointer to the tree 
/// @param input is the inputed data from the user
void handleCwdCommand(Filesystem* filesystem, char* input[])
{
  if(strchr(input[0], NEWLINE) == NULL)
  {
    printf("%s", ERROR_MESSAGE_COMMAND_PARAMETER); 
    return; 
  }

  if(filesystem->current_working_directory_ == filesystem->root_node_)
  {
    printf("%c\n", PATH_SEPARATOR); 
    return; 
  }

  printPath(filesystem, filesystem->current_working_directory_); 
  printf("\n"); 
}

//------------------------------------------------------------------------------
/// @brief This function executes the ls command for the user. 
/// @param filesystem is the pointer to the tree 
/// @param input is the inputed data from the user
void handleLsCommand(Filesystem* filesystem, char* input[])
{
  if(input[1] != NULL)
  {
    if(input[2] != NULL)
    {
      printf("%s", ERROR_MESSAGE_COMMAND_PARAMETER); 
      return; 
    }

    char* path[MAX_TOKENS]; 
    separateInput(input, path); 

    if(path[0] != NULL)
    {
      Node* tmp_current_directory = filesystem->current_working_directory_->directory_child_node_; 
      if(changePath(filesystem, tmp_current_directory, path, 0, 0) == ERROR)
      {
        return; 
      }
    }
  }

  if(filesystem->current_working_directory_->permission_ == READ || 
  filesystem->current_working_directory_->permission_ == READ_AND_WRITE)
  {
    int column_width = getColumnWidth(filesystem->current_working_directory_); 
    printContentFromDirectory(filesystem->current_working_directory_->directory_child_node_, 0, column_width);
  }
  else 
  {
    printf("%s'%s'\n", ERROR_MESSAGE_PERMISSION_READ, filesystem->current_working_directory_->name_); 
    
    if(filesystem->current_working_directory_->parent_node_ != NULL)
    {
      filesystem->current_working_directory_ = filesystem->current_working_directory_->parent_node_; 
    }
  }
} 

//------------------------------------------------------------------------------
/// @brief This function removes the newline from the inputed path. 
/// @param path is the inputed path from the user separated with /
void removeNewline(char* path[])
{
  for(int path_index = 0; path_index < MAX_TOKENS; path_index++)
  {
    if(path[path_index] != NULL)
    {
      char* newline_ptr = strchr(path[path_index], NEWLINE);

      if(newline_ptr != NULL)
      {
        int newline_position = newline_ptr - path[path_index]; 
        path[path_index][newline_position] = '\0'; 
        return; 
      }
    }
  }
}

//------------------------------------------------------------------------------
/// @brief This function separates the inputed path. 
/// @param input is the inputed data from the user
/// @param path is the inputed path from the user separated with /
void separateInput(char* input[], char* path[])
{
  if(input[1] != NULL)
  {
    path[0] = strtok(input[1], "/");
    char *tmp_token; 

    for(int token_index = 1; token_index < MAX_TOKENS; token_index++)
    {
      path[token_index] = NULL; 
      tmp_token = strtok(NULL, "/");
      path[token_index] = tmp_token; 
    }

    removeNewline(path); 
  }
}

//------------------------------------------------------------------------------
/// @brief This function changes the path like inputed from the user. 
/// @param filesystem is the pointer to the tree 
/// @param current_directory is the pointer to the current directory position 
/// @param path is the inputed path from the user separated with /
/// @param path_index is the / index from the inputed path from the user 
/// @param return_code is the index if a path segment was found or not 
/// @return OKAY or the correct exit code if there is an error 
int changePath(Filesystem* filesystem, Node* current_directory, char* path[], int path_index, int return_code)
{
  if(filesystem->current_working_directory_->permission_ != READ && 
  filesystem->current_working_directory_->permission_ != READ_AND_WRITE) 
  {
    printf("%s'%s'\n", ERROR_MESSAGE_PERMISSION_READ, filesystem->current_working_directory_->name_); 

    if(filesystem->current_working_directory_->parent_node_ != NULL)
    {
      filesystem->current_working_directory_ = filesystem->current_working_directory_->parent_node_; 
    }
    return ERROR; 
  }

  if(path[path_index] != NULL)
  {
    if(strcmp(path[path_index], ".") == 0)
    {
      return OKAY; 
    }
    else if(strcmp(path[0], "..") == 0)
    {
      filesystem->current_working_directory_ = filesystem->current_working_directory_->parent_node_; 
      path_index++; 
    }
    else if(strcmp(path[0], "") == 0)
    {
      filesystem->current_working_directory_ = filesystem->root_node_; 
      path_index++; 
    }

    if(current_directory == NULL)
    {
      printf("%s", ERROR_MESSAGE_COMMAND_PATH);
      return ERROR; 
    }
    else if(path[path_index] != 0)
    {
      if(strcmp(current_directory->name_, path[path_index]) == 0)
      {
        if((path[path_index + 1] == NULL) || (path_index == (MAX_TOKENS - 1)))
        {
          if(strcmp(current_directory->type_, TYPE_FILE) == 0) 
          {
            printf("[ERROR] %s is a file, not a directory!\n", current_directory->name_); 
            return ERROR; 
          }

          filesystem->current_working_directory_ = current_directory; 
          return OKAY; 
        }

        return_code = 
        changePath(filesystem, current_directory->directory_child_node_, path, path_index + 1, return_code); 
        if(return_code == ERROR) 
        {
          return ERROR; 
        }
        else if(return_code == OKAY)
        {
          return OKAY; 
        }
      }

      return_code = 
      changePath(filesystem, current_directory->next_sibling_node_, path, path_index, return_code); 
      if(return_code == ERROR) 
      {
        return ERROR; 
      }
      else if(return_code == OKAY)
      {
        return OKAY; 
      }
    }
  }
  else if(current_directory == NULL)
  {
    printf("%s", ERROR_MESSAGE_COMMAND_PATH);
    return ERROR; 
  }

  return 0; 
}

//------------------------------------------------------------------------------
/// @brief This function prints the children from the current directory. 
/// @param child_node is the pointer to the current child position 
/// @param child_index is the index of the child from the current directory 
void printContentFromDirectory(Node* child_node, int child_index, int column_width)
{
  if(child_node != NULL)
  {  
    int child_count = child_node->parent_node_->directory_child_count_; 
    int newline_count = (child_count / MAX_COLUMNS) + 1; 

    printf("%s", child_node->name_); 
    child_index += 1; 

    if(child_node->parent_node_->directory_child_count_ > 0 && child_index < child_count)
    {
      column_width += MAX_COLUMNS; 
      int amount_of_spaces = column_width - (int)strlen(child_node->name_); 

      for(int space_index = 0; space_index < amount_of_spaces; space_index++)
      {
        printf(" "); 
      }
    }

    if(child_index == child_count || child_index == MAX_COLUMNS || child_index == MAX_COLUMNS * newline_count)
    {
      printf("\n"); 
    }

    if(child_index < child_count)
    {
      printContentFromDirectory(child_node->next_sibling_node_, child_index, column_width); 
    }
  }
  return; 
} 

//------------------------------------------------------------------------------
/// @brief This function gets the number of the longest name in the current directory. 
/// @param child_node is the pointer to the current child position 
/// @return the maximum length 
int getColumnWidth(Node* parent_node)
{
  Node* child_node = parent_node->directory_child_node_; 
  int child_count = parent_node->directory_child_count_; 
  int length_of_objects[child_count]; 
  int max_length = 0; 

  for(int child_index = 0; child_index < child_count; child_index++)
  {
    length_of_objects[child_index] = (int)strlen(child_node->name_);
    child_node = child_node->next_sibling_node_; 

    if(length_of_objects[child_index] > max_length)
    {
      max_length = length_of_objects[child_index]; 
    }
  }
  return max_length; 
}

//------------------------------------------------------------------------------
/// @brief This function executes the cd command for the user. 
/// @param filesystem is the pointer to the tree 
/// @param input is the inputed data from the user
void handleCdCommand(Filesystem* filesystem, char* input[])
{
  if(input[1] != NULL)
  {
    if(input[2] != NULL)
    {
      printf("%s", ERROR_MESSAGE_COMMAND_PARAMETER); 
      return; 
    }

    char* path[MAX_TOKENS]; 
    separateInput(input, path); 

    if(path[0] != NULL)
    {
      Node* tmp_current_directory = filesystem->current_working_directory_->directory_child_node_; 

      if(changePath(filesystem, tmp_current_directory, path, 0, 0) == ERROR)
      {
        return; 
      }

      if(filesystem->current_working_directory_->permission_ != READ && 
        filesystem->current_working_directory_->permission_ != READ_AND_WRITE)
      {
        printf("%s", ERROR_MESSAGE_PERMISSION_READ); 
        printf("'%s'\n", filesystem->current_working_directory_->name_); 
        
        if(filesystem->current_working_directory_->parent_node_ != NULL)
        {
          filesystem->current_working_directory_ = filesystem->current_working_directory_->parent_node_; 
        }
        return; 
      }
    }
  }
  else
  {
    filesystem->current_working_directory_ = filesystem->root_node_; 
  } 
}

//------------------------------------------------------------------------------
/// @brief This function executes the info command for the user. 
/// @param child_node is the pointer to the current child position 
/// @param input is the inputed data from the user
void handleInfoCommand(Node* child_node, char* input[])
{
  if(input[1] != NULL && child_node != NULL)
  {
    if(input[2] != NULL)
    {
      printf("%s", ERROR_MESSAGE_COMMAND_PARAMETER); 
      return; 
    }

    Node* current_child_node = child_node; 

    if(strcmp(current_child_node->name_, input[1]) != 0)
    {
      for(int child_index = 1; child_index < current_child_node->parent_node_->directory_child_count_; child_index++)
      {
        current_child_node = current_child_node->next_sibling_node_; 

        if(current_child_node != NULL)
        {
          if(strcmp(current_child_node->name_, input[1]) == 0)
          {
            break; 
          }
        }
        else
        {
          printf("%s", ERROR_MESSAGE_COMMAND_NODE); 
          return; 
        }
      }
    }

    printInformationFromNode(current_child_node); 
  }
  else 
  {
    printf("%s", ERROR_MESSAGE_COMMAND_PARAMETER); 
    return; 
  }
}

//------------------------------------------------------------------------------
/// @brief This function prints the details from the wanted object node. 
/// @param child_node is the pointer to the current child 
void printInformationFromNode(Node* child_node)
{
  printf("----------------------\n");
  printf("Name: %s\n", child_node->name_);
  printf("Permissions: %d\n", child_node->permission_); 
  printf("Type: "); 

  if(strcmp(child_node->type_, TYPE_FILE) == 0)
  {
    printf("File\n"); 
  }
  else
  {
    printf("Directory\n"); 
  }

  printf("Size: "); 

  if(child_node->permission_ >= READ && child_node->permission_ <= READ_AND_WRITE)
  {
    int size_of_object = 0; 

    if(strcmp(child_node->type_, TYPE_FILE) == 0)
    {
      size_of_object = child_node->file_content_size; 
    }
    else 
    {
      size_of_object = child_node->directory_child_count_; 
    }

    printf("%d\n", size_of_object); 
  }
  else 
  {
    printf("~\n"); 
  }
}

//------------------------------------------------------------------------------
/// @brief This function executes the tree command for the user. 
/// @param filesystem is the pointer to the tree 
/// @param input is the inputed data from the user
void handleTreeCommand(Filesystem* filesystem, char* input[])
{
  Node* tmp_current_directory = filesystem->current_working_directory_; 

  if(input[1] != NULL)
  {
    if(input[2] != NULL)
    {
      printf("%s", ERROR_MESSAGE_COMMAND_PARAMETER); 
      return; 
    }

    char* path[MAX_TOKENS]; 
    separateInput(input, path); 

    if(path[0] != NULL)
    {
      if(changePath(filesystem, tmp_current_directory, path, 0, 0) == ERROR)
      {
        return; 
      }
    }
  }
  else
  {
    if(filesystem->root_node_ != NULL)
    {
      filesystem->current_working_directory_ = filesystem->root_node_; 
    }
  } 

  printFilesystemTree(filesystem->current_working_directory_->directory_child_node_, 0); 
}

//------------------------------------------------------------------------------
/// @brief This function executes the touch and mkdir command for the user. 
/// @param filesystem is the pointer to the tree 
/// @param input is the inputed data from the user
/// @param object_type is the object type from the new node 
void handleTouchAndMkdirCommand(Filesystem* filesystem, char* input[], char object_type[])
{
  Node* current_directory = filesystem->current_working_directory_->directory_child_node_; 
  Node* new_node; 

  if(input[1] != NULL)
  {
    char* path[MAX_TOKENS]; 
    separateInput(input, path); 

    if(path[0] != NULL)
    {
      if(input[2] != NULL)
      {
        for(int path_index = 0; path_index < MAX_TOKENS - 1; path_index++)
        {
          if(path[path_index + 1] == NULL)
          {
            new_node = saveNewNodeFromUser(input, object_type, path[path_index]); 
            if(new_node == NULL)
            {
              return; 
            }

            path[path_index] = 0; 
            break; 
          }
        }

        if(path[0] != NULL)
        {
          if(strcmp(path[0], ".") == 0)
          {
            current_directory = filesystem->current_working_directory_; 
          }
          else
          {
            if(changePath(filesystem, current_directory, path, 0, 0) == ERROR)
            {
              freeChildNode(new_node); 
              new_node = NULL; 
              return; 
            }

            Node* new_path_directory = filesystem->current_working_directory_; 
            filesystem->current_working_directory_ = current_directory->parent_node_; 
            current_directory = new_path_directory; 
          }
        }
        else
        {
          current_directory = filesystem->current_working_directory_; 
        }
        
        if(current_directory->permission_ != WRITE && 
          current_directory->permission_ != READ_AND_WRITE)
        {
          printf("%s", ERROR_MESSAGE_PERMISSION_WRITE); 
          printf("'%s'\n", current_directory->name_); 

          if(current_directory->parent_node_ != NULL)
          {
            filesystem->current_working_directory_ = current_directory->parent_node_; 
          }

          return; 
        }

        new_node->parent_node_ = current_directory; 
        linkNewNode(new_node, current_directory); 
      }
      else 
      {
        printf("%s", ERROR_MESSAGE_COMMAND_PARAMETER); 
        return; 
      }
    }
  }
}

//------------------------------------------------------------------------------
/// @brief This function saves a new node with details. 
/// @param input is the inputed data from the user
/// @param object_type is the the type from the new node object
/// @param object_name is the the name from the new node object
/// @return the node pointer or NULL if there is an error 
Node* saveNewNodeFromUser(char* input[], char object_type[], char object_name[])
{
  Node* new_node = allocateNewNode();
  if(new_node == NULL)
  {
    return NULL; 
  }

  new_node->type_ = saveNewString(object_type);
  if(new_node->type_ == NULL)
  {
    free(new_node); 
    return NULL; 
  }

  new_node->name_ = saveNewString(object_name);
  if(new_node->name_ == NULL)
  {
    free(new_node->type_); 
    free(new_node); 
    return NULL; 
  }

  removeNewline(input); 
  new_node->permission_ = atoi(input[2]); 
  new_node->next_sibling_node_ = NULL; 
  new_node->directory_child_node_ = NULL; 
  new_node->directory_child_count_ = 0; 
  new_node->file_content_size = 0; 
  return new_node; 
}

//------------------------------------------------------------------------------
/// @brief This function executes the rm command for the user. 
/// @param filesystem is the pointer to the tree 
/// @param input is the inputed data from the user
void handleRmCommand(Filesystem* filesystem, char* input[])
{
  if(input[1] != NULL)
  {
    if(strcmp(input[1], "root\n") == 0)
    {
      filesystem->current_working_directory_ = filesystem->current_working_directory_->parent_node_; 
      freeAllocatedNode(filesystem->root_node_); 
      filesystem->root_node_ = NULL; 
      return; 
    }

    handleCdCommand(filesystem, input);   
    Node* target_node = filesystem->current_working_directory_; 
    Node* tmp_parent_directory = filesystem->current_working_directory_->parent_node_; 

    if(target_node != filesystem->root_node_ && target_node->parent_node_ != NULL)
    {
      Node* current_child_node = tmp_parent_directory->directory_child_node_; 

      if(tmp_parent_directory->directory_child_count_ == 1)
      {
        tmp_parent_directory->directory_child_node_ = NULL; 
        tmp_parent_directory->directory_child_count_--; 
        filesystem->current_working_directory_ = tmp_parent_directory; 
        freeAllocatedNode(target_node); 
        target_node = NULL; 
        return; 
      }

      while(current_child_node != target_node)
      {
        if(current_child_node != NULL && current_child_node->next_sibling_node_ != NULL)
        {
          if(current_child_node->next_sibling_node_ == target_node)
          {
            break;
          }

          current_child_node = current_child_node->next_sibling_node_; 
        }
        else 
        {
          break; 
        }
      } 

      if(current_child_node->next_sibling_node_ == target_node)
      {
        current_child_node->next_sibling_node_ = target_node->next_sibling_node_; 
        tmp_parent_directory->directory_child_count_--; 
      }
      else if(current_child_node == target_node)
      {
        tmp_parent_directory->directory_child_node_ = target_node->next_sibling_node_; 
        tmp_parent_directory->directory_child_count_--; 
      }

      filesystem->current_working_directory_ = tmp_parent_directory; 
      freeChildNode(target_node); 
      target_node = NULL; 
      return; 
    }
  }
  else 
  {
    printf("%s", ERROR_MESSAGE_COMMAND_PARAMETER);
    return; 
  }
}

//------------------------------------------------------------------------------
/// @brief This function frees the target node from the rm command. 
/// @param target_node is the node which gets freed now 
void freeChildNode(Node* target_node)
{
  if(target_node == NULL)
  {
    return; 
  }

  freeAllocatedStrings(&target_node); 
  freeAllocatedNode(target_node->directory_child_node_); 
  free(target_node); 
  target_node = NULL; 
}

