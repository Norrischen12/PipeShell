/*
 * Copyright ©2023 Travis McGaha.  All rights reserved.  Permission is
 * hereby granted to students registered for University of Pennsylvania
 * CIT 5950 for use solely during Spring Semester 2023 for purposes of
 * the course.  No other use, copying, distribution, or modification
 * is permitted without prior written consent. Copyrights for
 * third-party components of this work must be honored.  Instructors
 * interested in reusing these course materials should contact the
 * author.
 */

#include <unistd.h>    // for fork()
#include <sys/types.h> // for pid_t
#include <sys/wait.h>  // for wait(), waitpid(), etc.
#include <cstring>
#include <cstdlib>  // for exit(), EXIT_SUCCESS, and EXIT_FAILURE
#include <vector>
#include <iostream>
#include <boost/algorithm/string.hpp>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::getline;
using namespace std;
using namespace boost::algorithm;


void pipe_shell() {
    string input;
    cout << "$ ";
    while (true) {
        if(!getline(cin, input)) {
            if(!cin.eof()){
            cerr << "Warning: fatal error while reading input from user" << endl;
            }
            break;
        }
        boost::trim(input);
        if (input == "exit") {
            break;
        }
        vector<string> commands;
        boost::split(commands, input, boost::is_any_of("|"), boost::token_compress_on);
        for (int i = 0; i < (int)commands.size(); i++){
            boost::trim(commands.at(i));
        }


        int pipe_fds[commands.size()-1][2];
        
        //Making all the pipe
        for (int i = 0; i < (int)commands.size() - 1; i++) {
            if (pipe(pipe_fds[i]) == -1) {
                cerr << "Error: pipe() failed\n";
                exit(EXIT_FAILURE);
            }
        }


        //For the case of NO pipes:
        // if ((int)commands.size() == 1) {
        //     pid_t pid = fork();
        //     if (pid == -1) {
        //         cerr << "Error: fork() failed\n";
        //         exit(EXIT_FAILURE);
        //     //Child Process
        //     } else if (pid == 0) {
        //         //call exec vp
        //         vector<string> tokens;
        //         boost::split(tokens, commands.at(0), boost::is_any_of(" "), boost::token_compress_on);
        //         char* argument_list[tokens.size() + 1];
        //         for (int i = 0; i < (int)tokens.size(); i++) {
        //             argument_list[i] = const_cast<char*>(tokens.at(i).c_str());
        //         }
        //         argument_list[tokens.size()] = NULL;
        //         execvp(const_cast<char*>(tokens.at(0).c_str()), argument_list);
        //         cerr << strerror(errno) << endl;
        //         exit(EXIT_FAILURE);


        //         waitpid(-1, NULL, 0);
        //         cout << "$ ";
                
        //     }
        //MORE THAN 1 PIPE 
        for (int con = 0; con < (int)commands.size() ; con++) {
            pid_t pid = fork();
            if (pid == 0) {
                if (commands.size() > 1) {
                    //If its the beginning case
                    //REMEMBER NOT TO USE I FOR ANYTHING ELSE
                    if (con == 0) {
                        //std_out to pipe1 w
                        dup2(pipe_fds[0][1], STDOUT_FILENO);
                    //Case of the END
                    } else if(con == (int)commands.size() - 1) {
                        //std_in redirect to read end of pipe
                        dup2(pipe_fds[commands.size()-2][0], STDIN_FILENO);

                    //Middle cases
                    } else {
                        dup2(pipe_fds[con-1][0], STDIN_FILENO);
                        dup2(pipe_fds[con][1], STDOUT_FILENO);
                    }
                    for (int j = 0; j < (int)commands.size()-1; j++) {
                        for (int k = 0; k < 2; k++){
                            close(pipe_fds[j][k]);
                        }
                    }
                }


                vector<string> tokens;
                boost::split(tokens, commands.at(con), boost::is_any_of(" "), boost::token_compress_on);
                char* argument_list[tokens.size() + 1];
                for (int m = 0; m < (int)tokens.size(); m++) {
                    argument_list[m] = const_cast<char*>(tokens.at(m).c_str());
                }
                argument_list[tokens.size()] = NULL;

                execvp(const_cast<char*>(tokens.at(0).c_str()), argument_list);
                cerr << strerror(errno) << endl;
                exit(EXIT_FAILURE);
            }

            
        }
        
        for (int j = 0; j < (int)commands.size() - 1; j++) {
            for (int k = 0; k < 2; k++){
                close(pipe_fds[j][k]);
            }
        }


        for (int l = 0; l < (int)commands.size(); l++){
            waitpid(-1, NULL, 0);
        }

        cout << "$ ";
    }

}

int main() {
    pipe_shell();
    return EXIT_SUCCESS;
}
