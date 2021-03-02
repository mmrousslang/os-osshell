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
int allNums(std::string checkString);

int main (int argc, char **argv)
{
    std::vector<std::string> os_path_list;
    char* os_path = getenv("PATH");
    splitString(os_path, ':', os_path_list);
    struct stat buf;
    std::vector<std::string> command_list; //to store command user types in, split into its various parameters
    char **command_list_exec; // command list converted to an array of character arrays
    std::vector<std::string> history; //to story history of commands in

    char slash = '/';
    char dot = '.';
    std::string command;

    // Welcome message
    printf("Welcome to OSShell! Please enter your commands ('exit' to quit).\n");

    while(true){
        printf("osshell> ");

        std::getline(std::cin, command); //putting user input into string commmand

        if(command[0] == NULL || command[0] == ' '){ //check if only enter or space was submitted for command
            //do nothing
        }else{ //other commands besides only enter
            //vectorOfStringsToArrayOfCharArrays(history, &history_list);

            splitString(command, ' ', command_list); //splitting command on the space character
            vectorOfStringsToArrayOfCharArrays(command_list, &command_list_exec);
            
            
            if(strcmp(command_list_exec[0], "exit") == 0){ //user enters the command 'exit', quit the program
              
               break; 

            }else if(strcmp(command_list_exec[0], "history") == 0){ //user enters 'history', print command history
               
                if(command_list_exec[1] == NULL){

                    for(int i = 0; i < history.size(); i++){ 
                        std::cout << "   " << i+1 <<": " <<history.at(i) << "\n";
                    }  

                }else{

                    if(strcmp(command_list_exec[1], "clear") == 0){
                       
                        history.clear();

                    }else if(allNums(command_list_exec[1]) > 0){
                        std::cout << "num > 0\n";
                    }else {
                        std::cout << "Error: history expects an integer > 0 (or 'clear')\n";
                    }
                } 

                history.push_back(command);  

            }else if(command_list_exec[0][0] == dot || command_list_exec[0][0] == slash){//user inputs . or / check if command is a path
               
                if(stat(command.c_str(), &buf) == 0 && buf.st_mode & S_IXUSR){
                    //executable found
                    int pid = fork();
                    //child process
                    if(pid == 0){
                        //run executable
                        execv(command.c_str(), command_list_exec);
                    } else {
                        int status;
                        waitpid(pid, &status, 0);
                    }
                }else { //not a valid path provided by the user
                    std::cout << command << ": Error command not found" << std::endl;
                }       
            }else{ //search for executable
                int j = 0;
                bool commandFound = false;
                history.push_back(command);

                for(j = 0; j < os_path_list.size(); j++) {
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

        }
    }
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

/*
    checkString: text to check if it is all numbers
    returns the string as an int if checkString is an int and 0 if it is not
*/
int allNums(std::string checkString){
    for(int i = 0; i < checkString.length(); i++){
        if(isdigit(checkString[i]) == false){
            return 0;
        }
    }
    return atoi(checkString.c_str());
}
