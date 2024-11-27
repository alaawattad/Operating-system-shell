#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"
#include <unistd.h>

using namespace std;




void ctrlCHandler(int sig_num)
{
  // TODO: Add your implementation
    cout << "smash: got ctrl-C" << endl;
    SmallShell& shell = SmallShell::getInstance();
    int pid = shell.running_pid;
    if(pid == -1)
        return;
    if(kill(pid, sig_num) == -1)
    {
        perror("smash error: kill failed");
        return;
    }
    shell.running_pid = -1;
    shell.curr_cmd = "";
   
    cout << "smash: process " << pid << " was killed" << endl;
     
}

