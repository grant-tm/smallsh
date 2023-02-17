#define _GNU_SOURCE

//standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

//command object
struct command{
    char *name; //string representing command name
    char *args[512]; //array of strings representing arguments
    char *inputFile; //string representing input filename
    char *outputFile; //string representing output filename
    int backgroundProcess; //0 for foreground, 1 for background
};

//Gives command default values, used to initialize newly created command
struct command *initializeCommand(struct command* commandToInitialize){
    
    //Initialize all args to NULL
    int i = 0;
    while(i < 512){
        commandToInitialize->args[i] = NULL;
        i++;
    }
    
    //initialize name, inputFile, and outputFile to NULL
    commandToInitialize->name = NULL;
    commandToInitialize->inputFile = NULL;
    commandToInitialize->outputFile = NULL;
    
    //Initialize backgroundProcess to 0 (implies false/foreground)
    commandToInitialize->backgroundProcess = 0; 
    
    return commandToInitialize;
}

//Frees all data in a command struct
void freeCommand(struct command* command){
    
    //free array of args
    int i=0;
    while(command->args[i] != NULL && i < 512){
        free(command->args[i]);
        i++;
    }
    
    //free name, inputFile, outputFile, and command object itself
    free(command->name);
    free(command->inputFile);
    free(command->outputFile);
    free(command);
}

//Prints the data stored in a command object.
void printCommand(struct command* command){
    
    //print command name
    printf("Name: %s\n", command->name);
    
    //print arguments in format "Arguments: [arg1, arg2, arg3]"
    printf("Arguments: [");
    int i=0;
    while(command->args[i] != NULL){
        if(i == 511 || command->args[i+1] == NULL){
            printf("%s", command->args[i]);
            break;
        }
        printf("%s, ", command->args[i]);
        i++;
    }
    printf("]\n");
    
    
    //print input file name, output file name, and background process flag
    printf("Input File: %s\n", command->inputFile);
    printf("Output File: %s\n", command->outputFile);
    printf("Background Process: %d\n", command->backgroundProcess);
}

//Determines if a string is a command or not. Returns 1 if yes, 0 if no.
int isCommand(char *input){
    if(input == NULL || strlen(input) < strlen("tt")){
        return 0;
    }
    if(input[0] == '\0' || input[0] != ':'){
        return 0;
    }
    if(input[1] == '\0' || input[1] != ' '){
        return 0;
    }
    return 1;
}


//Gets string input from user, stores in container.
char *getInput(char* container){
    //get user input with fgets, verify input
    while(!fgets(container, 2048, stdin)){
        printf("Error: Invalid Input\n");
    }
    return container;
}

//Expands all instances of "$$" in a string into the calling process PID.
char *expandVariable(char *string){
    
    //allocate and intitialize container for variable to expand ("$$")
    char *varToExpand = malloc(3*sizeof(char));
    memset(varToExpand, '\0', 3*sizeof(char));
    strcpy(varToExpand, "$$");
    
    //store memory location of first instance of "$$"
    //if "$$" does not appear in the string result is NULL
    char *varMemLoc = strstr(string, varToExpand);
    if(varMemLoc == NULL){
        return varMemLoc;
    }
    
    //calculate index of beginning of first instance of "$$"
    size_t varIndex = (size_t)(varMemLoc - string);
    
    //string prefix stores all characters preceding first instance of "$$"
    char *prefix = malloc((varIndex+1) * sizeof(char));
    memset(prefix, '\0', (varIndex+1) * sizeof(char));
    strncpy(prefix, string, (varIndex));
    
    //string suffix stores all characters after first isntance of "$$"
    char *suffix = malloc(((strlen(string) - varIndex)-1) * sizeof(char));
    memset(suffix, '\0', ((strlen(string) - varIndex)-1) * sizeof(char));
    memcpy(suffix, varMemLoc+2, ((strlen(string) - varIndex)-1));
    
    //get PID of calling process as a string
    //from https://stackoverflow.com/questions/53230155
    int pid = getpid();
    char *expandedVar = malloc(25*sizeof(char));
    memset(expandedVar, '\0', 25*sizeof(char));
    sprintf(expandedVar, "%d", pid);
    
    //concatenate prefix, pid, and suffix in that order
    prefix = realloc(prefix, strlen(prefix) + strlen(expandedVar) + strlen(suffix) + 2);
    strcat(prefix, expandedVar);
    strcat(prefix, suffix);
    
    //resize original string and store expanded string
    string = realloc(string, (strlen(prefix)+1) * sizeof(char));
    memset(string, '\0', (strlen(prefix)+1) * sizeof(char));
    strcpy(string, prefix);
    
    //free memory
    free(prefix);
    free(suffix);
    free(expandedVar);
    
    //recursive step
    //if expanded string contains more instances of "$$", run again.
    if(strstr(string, varToExpand) != NULL){
        free(varToExpand);
        string = expandVariable(string);
        return string;
    }
    
    //string contains no more instances of '$$"
    free(varToExpand);
    return string;
}

//Parses string input using tokens to fill in command struct
struct command *parseCommand(char *commandLineInput){
    
    //create an initialize command object
    struct command *newCommand = malloc(sizeof(struct command));
    initializeCommand(newCommand);

    //Create saveptr for use with strtok_r. Get first token ":" and throw it away.
    char *saveptr;
    char *token = strtok_r(commandLineInput, " ", &saveptr);
    if(token == NULL){
        return newCommand;
    }
    
    //Token is command name at this point.
    //Store command name and receive next token.
    token = strtok_r(NULL, " ", &saveptr);
    newCommand->name = calloc(strlen(token) + 1, sizeof(char));
    strcpy(newCommand->name, token);
    newCommand->name = expandVariable(newCommand->name);
    token = strtok_r(NULL, " ", &saveptr);
    if(token == NULL){
        return newCommand;
    }
    
    //Token may be argument, "<", ">", or "&" at this point. Token cannot be NULL.
    //Process tokens as arguments until reaching input/output/background flags or end of command.
    int i=0;
    printf("token: %s\n", token);
    printf("background: %d\n", strcmp(token, "&\n") != 0);
    while(strcmp(token, "<") != 0 && strcmp(token, ">") != 0 && strcmp(token, "&\n") != 0){
 
        //Token is an argument at this point. Store argument and receive next token.
        newCommand->args[i] = calloc(strlen(token) + 1, sizeof(char));
        strcpy(newCommand->args[i], token);
        newCommand->args[i] = expandVariable(newCommand->args[i]);
        ++i;
        
        //Receive next token. If token is not NULL, process next token.
        token = strtok_r(NULL, " ", &saveptr);
        if(token == NULL){
            return newCommand;
        }
    }

    //Token may be input/output/background flag at this point. Token cannot be NULL.
    //If token is input flag, get input file name and store it, then receive next token.
    if(strcmp(token, "<") == 0){
        
        //Get and store input file name.
        token = strtok_r(NULL, " ", &saveptr);
        //token = expandVariable(token);
        newCommand->inputFile = calloc(strlen(token)+1, sizeof(char));
        strcpy(newCommand->inputFile, token);
        newCommand->inputFile = expandVariable(newCommand->inputFile);
        
        //Receive next token. If NULL, end of command is reached.
        token = strtok_r(NULL, " ", &saveptr);
        if(token == NULL){
            return newCommand;
        }
    }
    
    //Token may be output/background flag at this point. Token cannot be NULL.
    if(strcmp(token, ">") == 0){
        //Next token is output file name. Process token and receive next token.
        token = strtok_r(NULL, " ", &saveptr);
        //token = expandVariable(token);
        newCommand->outputFile = calloc(strlen(token)+1, sizeof(char));
        strcpy(newCommand->outputFile, token);
        newCommand->outputFile = expandVariable(newCommand->outputFile);
        
        //Receive next token. If NULL, end of command is reached.
        token = strtok_r(NULL, " \n", &saveptr);
        if(token == NULL){
            return newCommand;
        }
    }

    //Token may be background flag at this point. Token cannot be NULL.
    if(strcmp(token, "&") == 0 || strcmp(token, "&\n") == 0){
        //Mark command to run in the background.
        newCommand->backgroundProcess = 1;
    }
    
    return newCommand;
}

int main(int argc, char *argv[]){
  
  struct command* command;
  char input[2048];
  int ext_code = 1;
  
  while(ext_code){
      getInput(input);
      if(isCommand(input) == 1){
          command = parseCommand(input);
          printCommand(command);
          freeCommand(command);
      }
      else{
          ext_code = 0;
      }
  }
  
  return EXIT_SUCCESS;
}