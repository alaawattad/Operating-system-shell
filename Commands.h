#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_
using namespace std;

#include <vector>
#include <list>


#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sys/stat.h>

#define COMMAND_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define COMMANDS_NUM (10)


///****************************** commands typessss *******************************//////////////////////



class JobsList;
class Command {
// TODO: Add your data members
public:
    const char* cmd_line;
    Command(const char *cmd_line): cmd_line(cmd_line) {}

    virtual ~Command(){}

    virtual void execute() = 0;

};

class BuiltInCommand : public Command {
public:
    JobsList* jobs;
    BuiltInCommand(const char *cmd_line, JobsList* jobs = nullptr) : Command(cmd_line), jobs(jobs) {}

    virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command {
public:
    bool isBackground;
    const char *cmd;
    ExternalCommand(const char *old_cmd ,const char *cmd_line):Command(cmd_line),cmd(old_cmd) {}

    virtual ~ExternalCommand() {}

    void execute() override;
    void executeSimpleCommand(const char* cmd_line);
    void executeComplexCommand(const char* cmd_line);
    void parentHandle(pid_t pid);
};

class PipeCommand : public Command {
public:
    string cmd1;
    string cmd2;
    int fd_id;
    enum Status {READ, WRITE};

    PipeCommand(const char *cmd_line) : Command(cmd_line) {}

    virtual ~PipeCommand() {}

    void execute() override;
};

class RedirectionCommand : public Command {
    // TODO: Add your data members
public:
    int stdout_fd;
    int fd;
    explicit RedirectionCommand(const char *cmd_line) : Command(cmd_line){};
    bool prepare(int status,int index , int trim);
    virtual ~RedirectionCommand() {}

    void execute() override;
};




///********************************* built in commands *****************************************************/// nico


////************* promptCommand 1 ****************///
class ChPromptCommand : public BuiltInCommand {
public:
    string new_name;

    ChPromptCommand(const char* cmd_line) : BuiltInCommand(cmd_line){}
    virtual ~ChPromptCommand() {}
    void execute() override;
};


////************* ShowPidCommand 2 ****************///
class ShowPidCommand : public BuiltInCommand {
public:
    ShowPidCommand(const char *cmd_line): BuiltInCommand(cmd_line) {};

    virtual ~ShowPidCommand() {}

    void execute() override;
};


////************* GetCurrDirCommand 3 ****************///
class GetCurrDirCommand : public BuiltInCommand {
public:
    GetCurrDirCommand(const char *cmd_line): BuiltInCommand(cmd_line) {};

    virtual ~GetCurrDirCommand() {}

    void execute() override;
};


////************* ChangeDirCommand 4 ****************///
class ChangeDirCommand : public BuiltInCommand {
public:
    string* last_dir_ptr;
    ChangeDirCommand(const char *cmd_line,string* plastPwd): BuiltInCommand(cmd_line) , last_dir_ptr(plastPwd)
    {}
    virtual ~ChangeDirCommand() {}
    void execute() override;
};




///********************** job list ****************************//////


class JobsList;


//// *********************************** more commands ********************************************** ////


////************* JobsCommand 5 ****************///
class JobsCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    JobsCommand(const char *cmd_line, JobsList *jobs) : BuiltInCommand(cmd_line, jobs) {}

    virtual ~JobsCommand() {}

    void execute() override;
};


////************* ForegroundCommand 6 ****************///
class ForegroundCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    ForegroundCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line, jobs){} //error_complete

    virtual ~ForegroundCommand() {}

    void execute() override;
};


//7 quit
class QuitCommand : public BuiltInCommand {
// TODO: Add your data members public:
public:
    QuitCommand(const char *cmd_line, JobsList *jobs): BuiltInCommand(cmd_line, jobs){}

    virtual ~QuitCommand(){}

    void execute() override;
};

//8
class KillCommand : public BuiltInCommand {
    // TODO: Add your data members
public:
    KillCommand(const char *cmd_line, JobsList *jobs);

    virtual ~KillCommand() {}

    void execute() override;
};

//9
class aliasCommand : public BuiltInCommand {
    string alias_name;
    string command_name;
public:
    aliasCommand(const char *cmd_line): BuiltInCommand(cmd_line) {}

    virtual ~aliasCommand() {}

    void execute() override;
};

//10
class unaliasCommand : public BuiltInCommand {
public:

    unaliasCommand(const char *cmd_line) : BuiltInCommand(cmd_line) {}

    virtual ~unaliasCommand() {}

    void execute() override;
};

////********************************* end of built in commands *****************************************************///


class ListDirCommand : public BuiltInCommand {
public:
    ListDirCommand(const char *cmd_line): BuiltInCommand(cmd_line) {};

    virtual ~ListDirCommand() {}

    void execute() override;
};


class GetUserCommand : public BuiltInCommand {
public:
	int user_id;
	int group_id;
	
    GetUserCommand(const char *cmd_line) :  BuiltInCommand(cmd_line) {}
    virtual ~GetUserCommand() {}

    void execute() override;
};

/////////////////////////************** jobs class *************//////////////////////////

class JobsList {
public:
    class JobEntry {
    public:

        int job_ID;

        pid_t process_ID; //pid of this job

        string job_cmd;

        //insert_time
        JobEntry(int job,pid_t ID,const char* cmd) :job_ID(job),process_ID(ID),job_cmd(cmd){}
    };
    list<JobEntry*> job_list;
public:
    JobsList()=default;

    ~JobsList()=default;

    void addJob(const char *cmd,pid_t Process_id, bool isStopped = false); // i did change the arg so ask about itttttt plssss

    void printJobsList();

    void killAllJobs();

    void removeFinishedJobs();

    JobEntry *getJobById(int jobId);

    void removeJobById(int jobId);

    JobEntry *getLastJob(int *lastJobId);

    JobEntry *getLastStoppedJob(int *jobId);
    // TODO: Add extra methods or modify exisitng ones as needed
};



////******************************** smash ***********************************////

class SmallShell {
public:
    // TODO: Add your data members
    string default_name;

    string shell_name;

    pid_t pid_num; //smash_pid

    string prev_dir;
    string new_cmd = "";

    JobsList jobs;

    string curr_cmd;

    pid_t running_pid;

    list<pair<string,string>> aliasesMap;

    string commands_array[10]  = {"chrompt",
                                  "showpid",
                                  "pwd",
                                  "cd",
                                  "jobs",
                                  "fg",
                                  "quit",
                                  "kill",
                                  "alias",
                                  "unalias"
    };



    SmallShell();

    //creating an array with an empty list in each index,
    //each list at index i contains the aliases of the command in commandsArray[i]
    //for now i set the size of array to be 10 aka the num of built in commands, its easy to modify later



    Command *CreateCommand(const char *cmd_line);

    SmallShell(SmallShell const &) = delete; // disable copy ctor
    void operator=(SmallShell const &) = delete; // disable = operator
    static SmallShell &getInstance() // make SmallShell singleton
    {
        static SmallShell instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    ~SmallShell();

    void executeCommand(const char *cmd_line);
    // TODO: add extra methods as needed

    string getSmashName(){
        return shell_name;
    }

    void setSmashName(string new_name){
        shell_name = new_name;
    }

    Command* createCommand(const char* cmd_line);

    void setPrevDir(char* str){
        prev_dir = str;
    }

};

#endif //SMASH_COMMAND_H_
