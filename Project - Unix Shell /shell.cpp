//  Author: Adilet Kuroda
//  Date: 04/10/2023
//  Description: This program is a shell that can execute commands
//  with arguments, background processes, and redirection. It can also
//  execute two commands with a pipe.

#include <iostream> // cout, cin, cerr
#include <unistd.h> // fork, execvp, dup2, pipe
#include <sys/wait.h> // waitpid
#include <string> // string
#include <sstream> // stringstream
#include <cstring> // strtok
#include <fcntl.h> // open
#include <sys/types.h> // open



#define MAX_LINE 80 // The maximum length command
using namespace std; // use the standard namespace

// function prototypes
void forkInput(char *args[], bool status[], string& filename);
void pipeInput(char *args[], char *args1[], bool status[], 
        string& filename);
void deleteArgs(char *args[]);
void parseLine(string input, char *args[], char *args1[], bool status[], 
                bool& pipe, string& filename);


int main(int argc, char *argv[])
{
    int status;
    char *args[MAX_LINE/2 + 1]; //command line arguments
    char *args1[MAX_LINE/2 + 1]; //command line arguments for pipe
    int should_run = 1;

    string history; //stores the last command
    while (should_run) { //loop until exit or quit
        cout << "osh>";
        fflush(stdout); //flush the output buffer
        string input; //input from the user
        getline(cin, input);
        //check if the input is exit or quit
        if (input == "exit" || input == "quit"){
            should_run = 0;
        }
        else{
            //check if the input is !! and replace it with the last command
            if (input == "!!"){
                input = history;
            }else{ //store the command in history
                history = input;
            }
            string filename; //filename for redirection
            bool status[3] = {false, false, false}; // [out, in, background]
            bool pipe = false;
            //parse the input for execution
            parseLine(input, args, args1, status,pipe, filename);
            if (pipe){
                //execute the command with a pipe
                pipeInput(args, args1, status, filename);
            }else{
                //fork the process and execute the command
                forkInput(args, status, filename);
            }
            
            //delete dynamically allocated memory
            deleteArgs(args);
            deleteArgs(args1);
        }
      
    }
    return 0;
}

// Executes two commands using pipe and possibly redirection
// args: first command
// args1: second command
// status: [out, in, background]
// filename: filename for redirection
void pipeInput(char *args[], char *args1[], bool status[], 
        string& filename) {
    int pipeFD[2];
    if (pipe(pipeFD) < 0) { //create pipe and check for error
        cerr << "Error: Pipe failed" << endl;
        exit(EXIT_FAILURE);
    }

    int pid = fork();
    if (pid < 0) { // Fork failed ?
        cerr << "Error: Fork failed" << endl;
        exit(EXIT_FAILURE); 
    }
    else if (pid == 0) { // Child process 1
        close(pipeFD[0]); // Close read end of pipe
        if (dup2(pipeFD[1], 1) < 0) { // Redirect stdout to pipe
            cerr << "Error: Dup2 failed to redirect" << endl;
            exit(EXIT_FAILURE);
        }
        close(pipeFD[1]); // Close write end of pipe

        // Execute first command
        execvp(args[0], args);
            cerr << "Error: Not valid command" << endl;
            exit(EXIT_FAILURE);
    } else { // Parent process
        int pid1 = fork(); // Fork again to create child process 2
        if (pid1 == -1) {
            cerr << "Error: Fork failed" << endl;
            exit(EXIT_FAILURE);
        }
        else if (pid1 == 0) { // Child process 2 in parent process
            close(pipeFD[1]); // Close write end of pipe
            if (dup2(pipeFD[0], 0) < 0) { 
                cerr << "Error: Dup2 failed to redirect" << endl;
                exit(EXIT_FAILURE);
            }
            close(pipeFD[0]); // Close read end of pipe

            // Execute second command
            if (status[0] || status[1]) { // If output or input is redirected
                int fd;
                if (status[0]) { // If output is redirected
                    fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd < 0) { // If something went wrong
                        cerr << "Error: File does not exist" << endl;
                        exit(EXIT_FAILURE);
                    }
                } else { // If input is redirected
                    fd = open(filename.c_str(), O_RDONLY);
                    if (fd < 0) { // If file does not exist
                        cerr << "Error: File does not exist" << endl;
                        exit(EXIT_FAILURE);
                    }
                }
                dup2(fd, 1); // Redirect stdout to file
                close(fd); // Close file
                execvp(args1[0], args1);
                cerr << "Error: Not valid command" << endl;
                exit(EXIT_FAILURE);
            }else{
                execvp(args1[0], args1);
                cerr << "Error: Not valid command" << endl;
                exit(EXIT_FAILURE);
            }
        }
        else { // Parent process
            close(pipeFD[0]); // Close both ends of pipe
            close(pipeFD[1]); // Close both ends of pipe

            if (!status[2]) { // If not a background process
                waitpid(pid, NULL, 0); // Wait for child process 1 to finish
                waitpid(pid1, NULL, 0); // Wait for child process 2 to finish
            }else{
                // cout << "Background process " << pid << " started" << endl;
                // cout << "Background process " << pid1 << " started" << endl;
            }
        }
    }
}



// delete dynamically allocated memory for the arguments
// ensure no memory leaks
// args: arguments
void deleteArgs(char *args[]){
    int i = 0;
    while(args[i] != NULL){
        delete[] args[i++];
   }
}
// fork the process and execute the command. Also wait for 
//the child process to finish if it is not a background process
// args: arguments
// status: [out, in, background]
// filename: filename for redirection
void forkInput(char *args[], bool status[], string& filename){
    int pid = fork();
    if (pid < 0){ //if fork failed
        cerr << "Error: Fork failed" << endl;
        exit(EXIT_FAILURE);
    }
    else if (pid == 0){ //child process
        if (status[0]){ //if output is redirected
            int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0){ //if something went wrong
                cout << "Error: File does not exist" << endl;
                exit(EXIT_FAILURE);
            }
            dup2(fd, 1); //redirect stdout to file
        }
        if (status[1]){ // if input is redirected
            int fd = open(filename.c_str(), O_RDONLY);
            if (fd < 0){ //if file does not exist
                cout << "Error: File does not exist" << endl;
                exit(EXIT_FAILURE);
            }
            dup2(fd, 0); //redirect stdin to file
            close(fd);//close file
        }
        execvp(args[0], args); //execute the command
        //if execvp fails
        cerr << "Error: Not valid command" << endl;
        exit(EXIT_FAILURE);
    }else{
        if (!status[2]){ //if not a background process
            waitpid(pid, NULL, 0); //wait for child process to finish
        }else{
           // cout << "Process " << pid << " running in background" << endl;  
        }
    }
}

//parses the input from the user for the command
// input: input from user
// args: arguments
// status: [out, in, background]
// pipe: if pipe is used
// filename: filename for redirection
// args1: arguments for second command if pipe is used
void parseLine(string input, char *args[], char *args1[], bool status[], 
                bool& pipe, string& filename){
    istringstream iss(input);
    string token;
    int i = 0;
    int j = 0;
    while (getline(iss, token, ' ')) {
        if (token == "&"){ //if background process
            status[2] = true;
            args[i] = NULL;
            args1[j] = NULL;
            break;
        }
        if (token == "|"){ //if pipe
            pipe = true;
            args[i] = NULL;
            getline(iss, token, ' ');

        }
        if (token == ">"){ //if output is redirected
            status[0] = true; //set status to true
            getline(iss, filename, ' '); 
            if (pipe){
                args1[j] = NULL;
            }else{
                args[i] = NULL;
            }
            break;
             
        }  
        if (token == "<"){ //if input is redirected
            status[1] = true; //set status to true
            getline(iss, filename, ' '); 
            if (pipe){
                args1[j] = NULL;
            }else{
                args[i] = NULL;
            }
            break;
        }
        if (!pipe){
            //store the arguments in args
            args[i] = new char[token.length() + 1];
            strcpy(args[i], token.c_str());
            i++;
        }else{
            //store the arguments in args1
            args1[j] = new char[token.length() + 1];
            strcpy(args1[j], token.c_str());
            j++;
        } 
    }
    //set the last argument to NULL
    args[i] = NULL;
    args1[j] = NULL;
}
