#include <unistd.h>
#include <string.h>
#include <limits.h>//can i include it ????
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <limits.h>
#include <signal.h>
#include "signals.h"
#include <stdio.h>
#include <regex>
#include <fcntl.h>
#include <algorithm>
#include <dirent.h>
#include <fcntl.h>
#include <sys/syscall.h>
using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string &s) {
    size_t start = s.find_first_not_of(WHITESPACE);
    return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s) {
    size_t end = s.find_last_not_of(WHITESPACE);
    return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s) {
    return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args) {
    FUNC_ENTRY()
    int i = 0;
    std::istringstream iss(_trim(string(cmd_line)).c_str());
    for (std::string s; iss >> s;) {
        args[i] = (char *) malloc(s.length() + 1);
        memset(args[i], 0, s.length() + 1);
        strcpy(args[i], s.c_str());
        args[++i] = NULL;
    }
    return i;

    FUNC_EXIT()
}

bool _isBackgroundComamnd(const char *cmd_line) {
    const string str(cmd_line);
    return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line) {
    const string str(cmd_line);
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos) {
        return;
    }
    // if the command line does not end with & then return
    if (cmd_line[idx] != '&') {
        return;
    }
    // replace the & (background sign) with space and then remove all tailing spaces.
    cmd_line[idx] = ' ';
    // truncate the command line string up to the last non-space character
    cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;

}


string _RemoveBackgroundSign(const char* cmd_line)
{
    const string str(cmd_line);
    string s=str;
    // find last character other than spaces
    unsigned int idx = str.find_last_not_of(WHITESPACE);
    // if all characters are spaces then return
    if (idx == string::npos)
        return s;

    // if the command line does not end with & then return
    if (cmd_line[idx] != '&')
        return s;

    // replace the & (background sign) with space and then remove all tailing spaces.
    s[idx] = ' ';
    // truncate the command line string up to the last non-space character
    s[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
    return s;
}


///********************* helper funcs ******************/// this might cause problems !!
void freeArray(char* arr[], int len)
{
    for(int i=0; i<len; i++){
        free(arr[i]);
    }
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
std::pair<std::string, std::string> findPairByFirst(const std::list<std::pair<std::string, std::string>>& myList, const std::string& searchTerm) {
    for (const auto& element : myList) {
        if (element.first == searchTerm) {
            return element;
        }
    }
    return {"", ""};  // Indicate not found
}


bool findFirstInPair(const std::list<std::pair<std::string, std::string>>& myList, const std::string& searchTerm) {
    for (const auto& element : myList) {
        if (element.first == searchTerm) {
            return true;
        }
    }
    return false;
}


bool createAlias(string aliasCommand) {
        // Regex to match the alias format
        std::regex aliasRegex(R"(^alias .+='[^']*' *&?$)");
         char* argsArray[COMMAND_MAX_ARGS + 1];
        int argsNum = _parseCommandLine(aliasCommand.c_str(), argsArray);
       
        if(argsNum == 1 && string(argsArray[0]) == "alias") return false;
        if (!std::regex_match(aliasCommand, aliasRegex) ) {
           
            return true;
        }
        return false;
}


Command* SmallShell::createCommand(const char* cmd_line)
{
	const char* old_cmd = cmd_line;
	
    string cmd_name = _trim(string(cmd_line));
    SmallShell& shell = SmallShell::getInstance();

    string finalCmd;
    char* argsArray[COMMAND_MAX_ARGS + 1];
    char* forAlias[COMMAND_MAX_ARGS + 1];
    
    int argsNum = _parseCommandLine(cmd_line, argsArray);
    if(argsNum == 0) return nullptr;
    
    
    auto it = findPairByFirst(shell.aliasesMap,string(argsArray[0]));

    if(it.first != "" && it.second != "")
    {
         new_cmd = it.second;
        
        for (int i = 1; i < argsNum; ++i) {
            new_cmd += " ";
            new_cmd += argsArray[i];
        }
        cmd_line = new_cmd.c_str();  // Convert std::string back to const char*
        _parseCommandLine(cmd_line, forAlias);
        finalCmd = forAlias[0];
        cmd_name = _trim(string(cmd_line));
    }
    else
    {
        finalCmd = argsArray[0];
    }
    ///POSSIBLE PROBLEM IN CMD
    if(!cmd_name.empty() && cmd_name.back() == '\n') {
        finalCmd = cmd_name.erase(cmd_name[cmd_name.size() -1]);
    }
   
    bool format =  createAlias(string(cmd_line));

    // Special Commands
    if ((finalCmd.compare("alias") == 0 || finalCmd.compare("alias&") == 0) && !format)
        return new aliasCommand(cmd_line);
        
    else if(cmd_name.find(">") != string::npos)
        return new RedirectionCommand(cmd_line);
        
    else if(cmd_name.find("|") != string::npos)
        return new PipeCommand(cmd_line);

    /// Built-in Commands
    else if (finalCmd == "chprompt" || finalCmd.compare("chprompt&") == 0)
        return new ChPromptCommand(cmd_line);

    else if (finalCmd == "showpid" || finalCmd.compare("showpid&") == 0)
        return new ShowPidCommand(cmd_line);

    else if (finalCmd == "pwd" || finalCmd == "pwd&")
        return new GetCurrDirCommand(cmd_line);


    else if (finalCmd.compare("cd") == 0)
        return new ChangeDirCommand(cmd_line, &(prev_dir));


    else if (finalCmd.compare("jobs") == 0 || finalCmd.compare("jobs&") == 0)
        return new JobsCommand(cmd_line, &(shell.jobs));


    else if (finalCmd.compare("fg") == 0 || finalCmd.compare("fg&") == 0)
        return new ForegroundCommand(cmd_line, &(shell.jobs));
        
        
    else if (finalCmd.compare("unalias") == 0 || finalCmd.compare("unalias&") == 0)
        return new unaliasCommand(cmd_line);


    else if(finalCmd.compare("quit") == 0 || finalCmd.compare("quit&") == 0)
        return new QuitCommand(cmd_line, &(shell.jobs));

    else if(finalCmd.compare("kill") == 0 )
        return new KillCommand(cmd_line, &(shell.jobs));

    else if(finalCmd.compare("listdir") == 0 || finalCmd.compare("listdir&") == 0)
        return new ListDirCommand(cmd_line);

     else if(finalCmd.compare("getuser")==0 || finalCmd.compare("getuser&")==0)
        return new GetUserCommand(cmd_line);

    else if (cmd_name == "")
        return nullptr;

    else
    {

        return new ExternalCommand(old_cmd,cmd_line);
	}
}



///***********************************************************************************//


// TODO: Add your implementation for classes in Commands.h

SmallShell::SmallShell() : default_name("smash> "), shell_name(default_name), pid_num(getpid()), prev_dir(""), jobs(), running_pid(-1) { //deal with runnung_pid


}
// TODO: add your implementation


SmallShell::~SmallShell() {
// TODO: add your implementation
}

void SmallShell::executeCommand(const char *cmd_line) {
    // TODO: Add your implementation here
    // for example:
    // Command* cmd = CreateCommand(cmd_line);
    // cmd->execute();
    // Please note that you must fork smash process for some commands (e.g., external commands....)

    Command* cmd = createCommand(cmd_line);
    if(cmd == nullptr){
        return;
    }
    cmd->execute();

    ///handle jobs and create shell instance
}


///******** built in commands *********** ///

//1
void ChPromptCommand::execute()
{
    char* argsArray[COMMAND_MAX_ARGS +1];
    int argsNum = _parseCommandLine(cmd_line, argsArray);
    SmallShell& smash = SmallShell::getInstance();

    if(argsNum == 1){
        new_name = smash.default_name;
    }
    else {
        new_name = (string(argsArray[1])+"> ");
    }
    smash.setSmashName(new_name);
    ///free array
    freeArray(argsArray, argsNum);
}

//2
void ShowPidCommand::execute()
{
    SmallShell& smash = SmallShell::getInstance();
    cout << "smash pid is " << smash.pid_num << endl;
}


//3
void GetCurrDirCommand::execute()
{
    char dirNameBuffer[PATH_MAX];
    if(getcwd(dirNameBuffer, PATH_MAX) != nullptr)
    {
        cout << dirNameBuffer << endl;
    }
    ///maybe handle error?
}

//4
void ChangeDirCommand::execute()
{
    char* argsArray[COMMAND_MAX_ARGS +1];
    char pathNameBuffer[PATH_MAX];

    int argsNum = _parseCommandLine(cmd_line, argsArray);
    SmallShell& smash = SmallShell::getInstance();

    if(argsNum == 1 || getcwd(pathNameBuffer, PATH_MAX) == nullptr){
        perror("smash error: getcwd failed");
        return;
    }
    if(argsNum > 2) {
        cerr << "smash error: cd: too many arguments" << endl;
        return;
    }
    string s(argsArray[1]);
    if(s == "-")  //we have only one arg
    {
        if(smash.prev_dir == "") {
            cerr << "smash error: cd: OLDPWD not set" << endl;
            return;
        }
        else{
            if(chdir((last_dir_ptr)->c_str()) != 0 )
            {
                perror("smash error: chdir failed");
                return;
            }
            else {
                smash.setPrevDir(pathNameBuffer);
            }
        }
    }

    else { //an actual dir path
        if(chdir(argsArray[1]) != 0 )
        {
            perror("smash error: chdir failed");
            return;
        }
        else {
            smash.setPrevDir(pathNameBuffer);
        }
    }
}



//5
void JobsCommand::execute()
{
    jobs->removeFinishedJobs();
    jobs->printJobsList();
}



// 6.
void ForegroundCommand::execute()
{
    char* argsArray[COMMAND_MAX_ARGS +1];
    int num_of_args = _parseCommandLine(cmd_line, argsArray);
    JobsList::JobEntry *job = nullptr;
    int jobID = 0;
    jobs->removeFinishedJobs();

    if(num_of_args > 2) {
        cerr << "smash error: fg: invalid arguments" << endl;
        return;
    }
    if(num_of_args == 1) { //fg
        job = jobs->getLastJob(&jobID);
        if(!job) {
            cerr <<  "smash error: fg: jobs list is empty" << endl;
            return;
        }
    }
    if(num_of_args == 2){ 
	try{
        jobID  = stoi(string(argsArray[1])); // Extract the substring between the quotes
	} catch(...) {
		cerr << "smash error: fg: invalid arguments" << endl;
		return;
	}
	if(jobID <=0){
		cerr << "smash error: fg: invalid arguments" << endl;
		return;
	}
        job = jobs->getJobById(jobID);
        if(!job) {
            cerr <<"smash error: fg: job-id " << jobID << " does not exist" << endl;
            return;
        }
    }
    cout << job->job_cmd << " " << job->process_ID << endl;

    if(kill(job->process_ID, SIGCONT) == -1) {
        perror("smash error: kill failed");
        return;
    }

    SmallShell& smash = SmallShell::getInstance();
  
    smash.curr_cmd = job->job_cmd;
    smash.running_pid = job->process_ID;
    if(waitpid(job->process_ID,nullptr,WUNTRACED) == -1)
    {
        perror("smash error: waitpid failed");
        return;
    }

}



//7
void QuitCommand::execute()
{
    ///quit initial
    char* argsArray[COMMAND_MAX_ARGS + 1];
    int num_of_args = _parseCommandLine(cmd_line, argsArray);
    std::list<JobsList::JobEntry*>& jobs = (BuiltInCommand::jobs)->job_list;
    (BuiltInCommand::jobs)->removeFinishedJobs();

    if(num_of_args >= 2 && string(argsArray[1]) == "kill")
    {
        cout << "smash: sending SIGKILL signal to " << jobs.size() << " jobs:" << endl;
        for(list<JobsList::JobEntry*>::iterator it = jobs.begin(); it != jobs.end(); ++it)
        {
            cout << (*it)->process_ID << ": " << (*it)->job_cmd << endl;
            if(kill((*it)->process_ID, SIGKILL) == -1)
            {
				perror("smash error: kill failed");
				return;
			}
        }
    }

    freeArray(argsArray, num_of_args);
    exit(0);
}


//8
KillCommand::KillCommand(const char* cmd_line, JobsList* jobs) : BuiltInCommand(cmd_line, jobs){}



void KillCommand::execute()
{
    char* argsArray[COMMAND_MAX_ARGS + 1];
    int num_of_args = _parseCommandLine(cmd_line, argsArray);
    const char * firstArg = argsArray[1];
    int signum;
    int jobId=0;
    
     if(num_of_args != 3 || firstArg[0] != '-')
    {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    
    try{
		jobId = stoi(string(argsArray[2]));
        signum = stoi(string(argsArray[1]+1));
    } catch (...) {
        cerr << "smash error: kill: invalid arguments" << endl;
        return;
    }
    if(jobId <= 0 ){
		cerr << "smash error: kill: invalid arguments" << endl;
        return;
	}
    
    //here jobId is a valid Id and we want to check if it really exists , and the format is correct
    JobsList::JobEntry* job = jobs->getJobById(jobId);
    
    if(jobs->getJobById(jobId) == nullptr)
    {
        cerr << "smash error: kill: job-id " << jobId << " does not exist" << endl;
        return;
    }

	cout << "signal number " << signum << " was sent to pid " << job->process_ID << endl;
    if(kill(job->process_ID, signum) == -1)
    {
        perror("smash error: kill failed");
        return;
    }
    if(signum == 9){
		jobs->removeJobById(jobs->getJobById(jobId)->job_ID);
	}
    
}

////* 9 alias command *//

void printPairs(const std::list<std::pair<std::string, std::string>>& pairs) {
    for (const auto& pair : pairs) {
        std::cout << pair.first << "='" << pair.second << "'" << endl;
    }
}




bool isValidString(const string& str)
{
    for (char c : str) {
        if (!std::isalpha(c) && !std::isdigit(c) && c!='_') {
            return false;
        }
    }
    return true;
}

//trim
string extractBefore(string input) {

    size_t pos = input.find("=");
    return input.substr(0,pos); // Create a std::string from the beginning to pos

}

string extractAfter(string input) {
size_t pos = input.find('=');
    if (pos != std::string::npos) {
        size_t start = input.find('\'', pos) + 1; // Find the position after the first single quote
        size_t end = input.find('\'', start);     // Find the ending quote position
        if (start != std::string::npos && end != std::string::npos) {
            return input.substr(start, end - start); // Extract the substring between the quotes
        }
    }
    return ""; // Return empty string if extraction fails

}

void aliasCommand::execute()
{
	
	char* argsArray[COMMAND_MAX_ARGS +1];
    SmallShell& smash = SmallShell::getInstance();
    int num_args = _parseCommandLine(cmd_line, argsArray);
     SmallShell& shell = SmallShell::getInstance();
    if(num_args == 1)
    {
        printPairs(shell.aliasesMap);
        return;
    }
    
    aliasCommand::alias_name = extractBefore(argsArray[1]);
    aliasCommand::command_name = extractAfter(string(cmd_line));
  
    if(!isValidString(alias_name)){
        cerr << "smash error: alias: invalid alias format" << endl;
        return;
    }
    if(findFirstInPair(smash.aliasesMap,alias_name)) {
        cerr << "smash error: alias: " << alias_name << " already exists or is a reserved command" << endl;
        return;
    }
    for(int i=0; i<COMMANDS_NUM; i++) {
        if(alias_name == smash.commands_array[i]) {
            cerr << "smash error: alias: " << alias_name << " already exists or is a reserved command" << endl;
            return;
        }
    }
    
  

    smash.aliasesMap.push_back({alias_name, command_name});
}


bool removePairWithFirst(std::list<std::pair<std::string, std::string>>& pairs, const std::string& targetFirst) {
    for (auto it = pairs.begin(); it != pairs.end(); ++it) {
        if (it->first == targetFirst) {
            pairs.erase(it);
            return true; // Element found and removed
        }
    }
    return false; // Element not found
}


///* 10 unalias command *//
void unaliasCommand::execute()
{
    char* argsArray[COMMAND_MAX_ARGS +1];
    int argsNum = _parseCommandLine(cmd_line, argsArray);
    SmallShell& smash = SmallShell::getInstance();

    if(argsNum < 2){
        cerr << "smash error: unalias: not enough arguments" << endl;
    }
    for(int i = 1 ; i < argsNum ; i++)
    {		
		if(removePairWithFirst(smash.aliasesMap,argsArray[i]))
		{
              continue;
		}
        else{
            cerr << "smash error: unalias: " << argsArray[i] << " alias does not exist" << endl;
            return;
		}  
    }

}



///* 11 I/O-redirection command *//
bool RedirectionCommand::prepare(int std,int pos , int trim)
{
    string temp = _trim(string(cmd_line).substr(pos+trim));
    string file_name = _RemoveBackgroundSign(temp.c_str());
    file_name =  _trim(file_name);
    stdout_fd = dup(1);
    if(stdout_fd == -1)
    {
        perror("smash error: dup failed");
        return true;
    }
    if( (fd = open(file_name.c_str(), O_CREAT|std|O_RDWR, 0664)) == -1)
    {
        perror("smash error: open failed");
        return true;
    }
    if( dup2(fd, 1) == -1)
    {
        perror("smash error: dup2 failed");
        return true;
    }
    return false;
}


bool isRedirectSymbolAtEnd(const std::string& input) {
    size_t pos = input.find('>');
    if (pos != std::string::npos) {
        // Check if '>' is the last character or if it's followed by a space
        if (pos == input.length() - 1) {
            return true; // '>' is the last character
        } else if (input[pos + 1] == ' ') {
            size_t nextCharPos = input.find_first_not_of(' ', pos + 1);
            if (nextCharPos == std::string::npos) {
                return true; // '>' followed only by spaces until the end
            }
        }
    }
    return false; // '>' is followed by some other characters
}


void RedirectionCommand::execute()
{
	char* argsArray[COMMAND_MAX_ARGS +1];
  _parseCommandLine(cmd_line, argsArray);
   if (isRedirectSymbolAtEnd(string(cmd_line))) {
		cerr << "smash error: open failed: No such file or directory" << endl;
        return;
    }  
    string cmd_line(Command::cmd_line);
    size_t pos = cmd_line.find(">");
    string cmd = cmd_line.substr(0, pos);
    cmd = _RemoveBackgroundSign(cmd.c_str());
   
    if( cmd_line[pos+1] == '>' )
    {
        if(prepare(O_APPEND,pos,2))return;
    }
    else
    {
        if(prepare(O_TRUNC,pos,1))return;
    }
    SmallShell& smash = SmallShell::getInstance();
    Command* execute_cmd = smash.createCommand(cmd.c_str());
    execute_cmd->execute();

    if(close(fd) == -1){
        perror("smash error: close failed");
        return;
    }
    if(dup2(stdout_fd, 1) == -1){
        perror("smash error: dup2 failed");
        return;
    }
    if(close(stdout_fd) == -1){
        perror("smash error: close failed");
        return;
    }

    delete execute_cmd;
}

void ListDirCommand::execute() {

   
    char buf[PATH_MAX];
    vector<string> files, directories, links;
    char* argsArray[COMMAND_MAX_ARGS +1];
    int argsNum = _parseCommandLine(cmd_line, argsArray);
    
    if (argsNum > 2) {
        cerr << "smash error: listdir: too many arguments" << endl;
        return;
    }

    
    string directory = (argsNum == 2) ? string(argsArray[1]) : ".";
    
    int fo = open(directory.c_str(), O_RDONLY | O_DIRECTORY);
    if (fo == -1) {
        perror("smash error: open failed");
        return;
    }
    // Read dir entries
    ssize_t numRead;
    while ((numRead = syscall(SYS_getdents64, fo, buf, PATH_MAX)) > 0) 
    {		
        for (int pos = 0; pos < numRead;) 
        {
            struct dirent64 *d = (struct dirent64 *) (buf + pos);
            
            string name(d->d_name);
            if (name == "." || name == "..") {
                pos += d->d_reclen;
                continue;
            }

            switch (d->d_type) {
                case DT_REG: files.push_back(name); break;
                case DT_DIR: directories.push_back(name); break;
                default: break;
            }

            pos += d->d_reclen;
        }
    }

    if (numRead == -1) {
        perror("smash error: getdents64");
    }

    close(fo);
    
    sort(files.begin(), files.end());
    for (const auto &file : files) {
        cout << "file: " << file << endl;
    }

    sort(directories.begin(), directories.end());
    for (const auto &directory : directories) {
        cout << "directory: " << directory << endl;
    }
}


///12. PIPES - will be tested with simple commands , wont be run in the background
//(cmd1 | cmd2) || (cmd1 |& cmd2)
void PipeCommand:: execute()
{
    string cmd(cmd_line);
    size_t loc = cmd.find("|");
    cmd1 = _RemoveBackgroundSign(cmd.substr(0,loc).c_str());
    //case of 'cmd1 |& cmd2'
    if(cmd.at(loc+1) == '&'){
        cmd2 = _RemoveBackgroundSign(cmd.substr(loc+2).c_str());
        fd_id = 2;
    }
    else {
        cmd2 = _RemoveBackgroundSign(cmd.substr(loc+1).c_str());
        fd_id = 1;
    }

    SmallShell& smash = SmallShell::getInstance();
    pid_t pid = fork();
    if(pid == -1)
    {
        perror("smash error: fork failed");
        return;
    }
    else if(pid > 0)
    {
        if(waitpid(pid, nullptr, WUNTRACED) == -1)
        {
            perror("smash error: waitpid failed");
            return;
        }
    }
    // son = pipe
    else if(pid == 0)
    {
        int fd[2]; // creating a new pipe creates fd[0] for reading from the pipe, fd[1] for writing
        if(pipe(fd) == -1)
        {
            perror("smash error: pipe failed");
            return;
        }

        setpgrp();
        pid_t left_pid = fork();
        if(left_pid == -1)
        {
            perror("smash error: fork failed");
            return;
        }

        else if(left_pid > 0) // father, which is right
        {
            if ( waitpid(left_pid,nullptr, WUNTRACED) == -1)
            {
                perror("smash error: waitpid failed");
                exit(0);
            }

            if( dup2(fd[READ], 0) == -1){
                perror("smash error: dup2 failed");
                exit(0);
            }
            if(close(fd[WRITE]) == -1 || close(fd[READ]) == -1){
                perror("smash error: close failed");
                exit(0);
            }

            Command* new_cmd = smash.createCommand(cmd2.c_str());
            new_cmd->execute();
            delete new_cmd;
            exit(0);
        }
        else if(left_pid == 0)
        {
            setpgrp();
            if( dup2(fd[WRITE], fd_id) == -1){
                perror("smash error: dup2 failed");
                exit(0);
            }
            if(close(fd[WRITE]) == -1 || close(fd[READ]) == -1){
                perror("smash error: close failed");
                exit(0);
            }
            Command* new_cmd = smash.createCommand(cmd1.c_str());
            new_cmd->execute();
            delete new_cmd;
            exit(0);
        }
    }
}

//*13 GetUserCommand *///
void GetUserCommand::execute()
{
	string username, groupname;
	
	//get the pid
	char* argsArray[COMMAND_MAX_ARGS +1];
    int argsNum = _parseCommandLine(cmd_line, argsArray);
    
    if(argsNum > 2) {
        cerr << "smash error: getuser: too many arguments" << endl;
        return;
    }

   string arg1(argsArray[1]);
   int pid_num = stoi(arg1);

  

   string path = "/proc/" + to_string(pid_num) +"/status";
    

    // Open file
    int fd = open(path.c_str(), O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }



	struct stat s;
	int res = stat(path.c_str(), &s);
	
	 if (res == -1) {
        cerr << "smash error: getuser: process " << pid_num << " does not exist" << endl;
        return;
    }
	
	int user_id = s.st_uid;
	int group_id = s.st_gid;
    
    close(fd);
    
    struct passwd *pw = getpwuid(user_id);
    struct group *grp = getgrgid(group_id);
 
    username =  pw->pw_name;
	groupname = grp->gr_name;
	

	cout << "User: " << username << endl;
    cout << "Group: " << groupname << endl;
	
}



/////////**************************job Functions**********************/////////////////////////
class JobEntry;

void JobsList::printJobsList(){
    for(list<JobEntry*>::iterator it = job_list.begin(); it != job_list.end(); it++){
        JobEntry* currJob = (*it);
        cout << "[" << currJob->job_ID << "] " << currJob->job_cmd << endl;
        //TIME ?? STOPPED??
    }
}


void JobsList::addJob(const char *cmd, pid_t process_id, bool isStopped )
{


    SmallShell& smallShell = SmallShell::getInstance();
    std::list<JobEntry*>& jobs = smallShell.jobs.job_list;

    if(jobs.empty())
    {
        JobEntry* newJob = new JobEntry(1, process_id, cmd);
        smallShell.jobs.job_list.push_back(newJob);
        return;
    }
    removeFinishedJobs();
    int newPid = (jobs.size() == 0 ? 1 : (jobs.back()->job_ID + 1));
    JobEntry* newJob = new JobEntry(newPid, process_id, cmd);
    smallShell.jobs.job_list.push_back(newJob);
    return;
}




JobsList::JobEntry* JobsList::getJobById(int jobId)
{
    SmallShell& smallShell = SmallShell::getInstance();
    std::list<JobEntry*>& jobs = smallShell.jobs.job_list;
    for(list<JobsList::JobEntry*>::iterator it = jobs.begin(); it != jobs.end(); it++)
        if((*it)->job_ID == jobId)
            return (*it);
    return nullptr;
}



void JobsList::removeJobById(int jobId)
{
    SmallShell& smallShell = SmallShell::getInstance();
    std::list<JobEntry*>& jobs = smallShell.jobs.job_list;
    for(list<JobsList::JobEntry*>::iterator it = jobs.begin(); it != jobs.end(); it++)
        if((*it)->job_ID == jobId)
        {
            jobs.erase(it);
            return;
        }
}


JobsList::JobEntry *JobsList::getLastJob(int *lastJobId)
{
    SmallShell& smallShell = SmallShell::getInstance();
    std::list<JobEntry*>& jobs = smallShell.jobs.job_list;
    if(jobs.empty())
        return nullptr;
    *lastJobId = jobs.back()->job_ID;
    return jobs.back();
}


void JobsList::removeFinishedJobs()
{
    SmallShell& shell = SmallShell::getInstance();
    std::list<JobEntry*>& jobs = shell.jobs.job_list;
    
    JobEntry** still_running = new JobEntry*[jobs.size()];
    
    int  i = 0;
    for(list<JobsList::JobEntry*>::iterator it = jobs.begin(); it != jobs.end(); it++)
    {
        int done = waitpid((*it)->process_ID, nullptr, WNOHANG);
        if(done == -1 && errno == ECHILD)
        {
			continue;
		}
        if(done == 0)
        {
            still_running[i++] = *it;
		}
    }
    jobs.clear();
    for(int j=0; j < i; j++)
        jobs.push_back(still_running[j]);
    
    delete[] still_running;
}


JobEntry JobsList::*getLastStoppedJob(int *jobId);

void JobsList::killAllJobs(){}




////********************************* EXTERNAL COMMANDS ************************************** //
void ExternalCommand::execute()
{
    isBackground =_isBackgroundComamnd(cmd_line);
    // SmallShell& smash = SmallShell::getInstance();
    string cmd = _trim(cmd_line); //potential error

    if(cmd.find("*") == string::npos && cmd.find("?") == string::npos)
        executeSimpleCommand(cmd.c_str());
    else
        executeComplexCommand(cmd.c_str());
}

//POSSIBLE ERROR: is passing the pid as an argument ok??it may not be
void ExternalCommand::parentHandle(pid_t pid)
{
    SmallShell& smash = SmallShell::getInstance();
    if (isBackground) {
        smash.jobs.removeFinishedJobs();
        smash.jobs.addJob(cmd, pid, false); //CHECK FALSE
    } else {
        smash.curr_cmd = cmd_line;
        smash.running_pid = pid;

        if (waitpid(pid, NULL, WUNTRACED) == -1) {
            perror("smash error: waitpid failed");
        }
        smash.curr_cmd = "";
        smash.running_pid = -1;
    }
}

void ExternalCommand::executeSimpleCommand(const char *cmd_line)
{
    char* cmd = const_cast<char*>(cmd_line);
    _removeBackgroundSign(cmd);
    char* argsArray[COMMAND_MAX_ARGS + 1];
    _parseCommandLine(cmd_line, argsArray);

    //all of the above I may move to ExternalCommand::execute() later but for now lets not lmao
    // SmallShell& smash = SmallShell::getInstance();

    pid_t pid = fork();
    if(pid == -1)
    {
        perror("smash error: fork failed");
    }
    else if (pid == 0)
    {
        setpgrp();  // to change the group ID of all you forked children.
        int result = execvp(argsArray[0], argsArray); //first arg is file name, second is array of all the args
        if(result == -1) {
            perror("smash error: execvp failed");
            exit(0);
        }
        exit(0);
    }
    else // A positive int - the PID of the child process in the **parent** process.
    {
        ///IDK WHAT TO DO
        parentHandle(pid);
    }
}

void ExternalCommand::executeComplexCommand(const char *cmd_line)
{
    char* cmd = const_cast<char*>(cmd_line);
    _removeBackgroundSign(cmd);
    char* argsArray[COMMAND_MAX_ARGS + 1];
    _parseCommandLine(cmd_line, argsArray);
    string cmd_str(cmd);
    SmallShell& smash = SmallShell::getInstance();

    pid_t pid = fork();
    char* path = (char*)("/bin/bash");

    if(pid == -1 )
    {
        perror("smash error: fork failed");
    }
    else if(pid == 0)
    {
        setpgrp();
        char* argv[] = {path, (char*)"-c", (char*)cmd_str.c_str(), NULL};
        int result = execv(path, argv);
        if(result == -1)
        {
            perror("smash error: execvp failed");
            exit(0);
        }
        else
        {
            if(isBackground){
                smash.jobs.removeFinishedJobs();
                smash.jobs.addJob(cmd, pid, false);
            }
        }

    }
    else {
        parentHandle(pid);
    }
}

