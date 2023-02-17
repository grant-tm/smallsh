#define _GNU_SOURCE

//standard libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    token = strtok_r(NULL, " ", &saveptr);
    if(token == NULL){
        return newCommand;
    }
    
    //Token may be argument, "<", ">", or "&" at this point. Token cannot be NULL.
    //Process tokens as arguments until reaching input/output/background flags or end of command.
    int i=0;
    while(strcmp(token, "<") != 0 && strcmp(token, ">") != 0 && strcmp(token, "&") != 0){
 
        //Token is an argument at this point. Store argument and receive next token.
        newCommand->args[i] = calloc(strlen(token) + 1, sizeof(char));
        strcpy(newCommand->args[i], token);
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
        newCommand->inputFile = calloc(strlen(token)+1, sizeof(char));
        strcpy(newCommand->inputFile, token);
        
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
        newCommand->outputFile = calloc(strlen(token)+1, sizeof(char));
        strcpy(newCommand->outputFile, token);
        
        //Receive next token. If NULL, end of command is reached.
        token = strtok_r(NULL, " ", &saveptr);
        if(token == NULL){
            return newCommand;
        }
    }

    //Token may be background flag at this point. Token cannot be NULL.
    if(strcmp(token, "&") == 0){
        //Mark command to run in the background.
        newCommand->backgroundProcess = 1;
    }
    
    return newCommand;
}

int main(int argc, char *argv[]){
  
  char input1[2048] = ": commandName arg1 arg2 arg3 < input > output &";
  
  struct command* newCommand;
  
  if(isCommand(input1) == 1){
      newCommand = parseCommand(input1);
      printCommand(newCommand);
      freeCommand(newCommand);
  }
  
  return EXIT_SUCCESS;
}