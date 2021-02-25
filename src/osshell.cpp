#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

void splitString(std::string text, char d, std::vector<std::string>& result);
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result);
void freeArrayOfCharArrays(char **array, size_t array_length);

int main (int argc, char **argv)
{
    // Get list of paths to binary executables
    std::vector<std::string> os_path_list;
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);
    struct stat buf;

    // Example code for how to loop over NULL terminated list of strings
    // int z;
    // for (z = 0; i < os_path_list.size(); z++)
    // {
    //     printf("PATH[%2d]: %s\n", z, os_path_list[z].c_str());
    // }

    // Welcome message
    printf("Welcome to OSShell! Please enter your commands ('exit' to quit).\n");

    std::vector<std::string> command_list; //to store command user types in, split into its various parameters
    char **command_list_exec; // commannd list converted to an array of character arrays

    // Repeat:
    //  Print prompt for user input: "osshell> " (no newline)
    //  Get user input for next command
    //  If command is `exit` exit loop / quit program
    //  If command is `history` print previous N commands
    //  For all other commands, check if an executable by that name is in one of the PATH directories
    //   If yes, execute it
    //   If no, print error statement: "<command_name>: Error command not found" (do include newline)

    char *exit = "exit"; //variable holds string exit
    char *history = "history"; //variable holds string history
    int i = 0;

    std::string command;
    
//get full command and then split it using string split

    while(true){
        printf("osshell> ");
        std::getline(std::cin, command); //putting user input into string commmand
        splitString(command, ' ', command_list); //splitting command on the space char

        vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
        
        if(strcmp(command_list_exec[0], exit) == 0){
            break; //exits program
        }else if(strcmp(command_list_exec[0], history) == 0){ 
            for(int k = 0; k < i; k++){
                //std::cout << k+1 << ": " << hisory_list[k] << std::endl;
                //need to make it print out the history of commands
                std::cout << "HISTORY ...\n";
            }
        }else{ //search for executable
            int j = 0;
            bool commandFound = false;

            for(j = 0; j < os_path_list.size(); j++) {

                // std::string pathStringToSplit = os_path_list[j];
                // pathStringToSplit.append("/");
                // pathStringToSplit.append(userCommandFull);

                // std::cout << "spot 2\n";

                std::string pathString = os_path_list[j];
                pathString.append("/");
                pathString.append(command_list_exec[0]);

                if(stat(pathString.c_str(), &buf) == 0 && buf.st_mode & S_IXUSR){
                    //executable found, now need to exit this inner while loop and then execute command with thread
                    commandFound = true;

                    int pid = fork();
                    //child process
                    if(pid == 0){
                        //run executable
                        execv(pathString.c_str(), command_list_exec);
                    } else {
                        int status;

                        waitpid(pid, &status, 0);
                    }
                    break;
                    
                }
            }

            if(!commandFound){ //user entered a command that was not found
                std::cout << command << ": Error command not found" << std::endl;
            }
        }

        i++;
    }

    // //Example code - shows how to split a command and prepare for the execv() function
    // std::string example_command = "ls -lh";
    // splitString(example_command, ' ', command_list);
    // vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
    // // use command_list_exec in the execv function rather than looping and printing
    // i = 0;
    // while (command_list_exec[i] != NULL)
    // {
    //     printf("CMD[%2d]: %s\n", i, command_list_exec[i]);
    //     i++;
    // }

    // // free memory for 'command_list_exec'
    // freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
    // printf("-------\n");

    // // Second example command - reuse the 'command_list' and 'command_list_exec'
    // example_command = "echo \"Hello world\" I am alive!";
    // splitString(example_command, ' ', command_list);
    // vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
    // // use 'command_list_exec' in the execv() function rather than looping and printing
    // i = 0;
    // while (command_list_exec[i] != NULL)
    // {
    //     printf("CMD[%2d]: %s\n", i, command_list_exec[i]);
    //     i++;
    // }

    // // free memory for 'command_list_exec'
    // freeArrayOfCharArrays(command_list_exec, command_list.size() + 1);
    // printf("-------\n");
    return 0;
}

/*
   text: string to split
   d: character delimiter to split `text` on
   result: vector of strings - result will be stored here
*/
void splitString(std::string text, char d, std::vector<std::string>& result)
{
    enum states { NONE, IN_WORD, IN_STRING } state = NONE;

    int i;
    std::string token;
    result.clear();
    for (i = 0; i < text.length(); i++)
    {
        char c = text[i];
        switch (state) {
            case NONE:
                if (c != d)
                {
                    if (c == '\"')
                    {
                        state = IN_STRING;
                        token = "";
                    }
                    else
                    {
                        state = IN_WORD;
                        token = c;
                    }
                }
                break;
            case IN_WORD:
                if (c == d)
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
            case IN_STRING:
                if (c == '\"')
                {
                    result.push_back(token);
                    state = NONE;
                }
                else
                {
                    token += c;
                }
                break;
        }
    }
    if (state != NONE)
    {
        result.push_back(token);
    }
}

/*
    list: vector of strings to convert to an array of character arrays
    result: pointer to an array of character arrays when the vector of strings is copied to
*/
void vectorOfStringsToArrayOfCharArrays(std::vector<std::string>& list, char ***result)
{
    int i;
    int result_length = list.size() + 1;
    *result = new char*[result_length];
    for (i = 0; i < list.size(); i++)
    {
        (*result)[i] = new char[list[i].length() + 1];
        strcpy((*result)[i], list[i].c_str());
    }
    (*result)[list.size()] = NULL;
}

/*
   array: list of strings to be freed
   array_length: number of strings in the list to free
*/
void freeArrayOfCharArrays(char **array, size_t array_length)
{
    int i;
    for (i = 0; i < array_length; i++)
    {
        if (array[i] != NULL)
        {
            delete[] array[i];
        }
    }
    delete[] array;
}