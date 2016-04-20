/*
 * libwebsockets-client
 *
 * Copyright (C) 2016 Alex Serbin
 *
 * g++ working.cpp OlymptradeWsClient.cpp EstablishConnection.cpp ConnectionData.cpp StatisticsFileSystem.cpp ParseCmdArgs.cpp json11/json11.cpp -L/usr/local/lib -lwebsockets -pthread -g -std=c++11
 * TODO:: Attemt to reconnect after n minutes after last data recieved
 */

#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "ParseCmdArgs.h"
#include "OlymptradeWsClient.h"

enum
{
  CLOSE_CONNECTION,
  ESTABLISH_CONNECTION
};

typedef struct MsgBufWsCreateNewConnection
{
  long msgtyp;
  int action;
  int asset;
} ws_msg_buf_t;

const std::string RECORDS_FILENAME = "/Creation_data";
const int MAX_PROGRAMM_PATH_LEN = 500;
const int ASSETS_AMOUNT = 20;                                   //txlib -> examples ldview -- graph algorithms doxygen help + dynamic array for using threads;
const int MAX_USERNAME_LENGTH = 200;
                                        
static std::string RECORDS_FILEPATH;                            //doxygen.org -> manual   txlib -> help in every function + tx/doc files

void delete_queue(int msgid);
int queue_get_access(const std::string& pathname_to_use);

int load_all_names(std::string names[], FILE* file_from);
int delete_bracket(const char* str_to_clean);
int kill_child_process(pid_t& pid_to_kill);
static void set_child_sigint_handler();
static void set_main_sigint_handler();

//needs initialization (first time call with olymp pointer as data)
static void main_sighandler(int sig, siginfo_t* siginfo, void* data);
static void child_sighandler(int sig, siginfo_t* siginfo, void* data);

int main(int argc, char **argv)
{
  struct PingConnection
  {
    long mtype;
    int accept_msgtype;
  };

  struct PingResult
  {
    long mtype;
    bool ready_to_recieve;
  };

  PingConnection ping = {};
  PingResult ping_response = {};

  pid_t pid = 1;
  int records_amount = 0;
  pid_t processes_running[ASSETS_AMOUNT] = {};
  char current_username[MAX_USERNAME_LENGTH] = {};
  getlogin_r(current_username, MAX_USERNAME_LENGTH);
  std::string QUEUE_PATHNAME = "/home/" + std::string(current_username) + "/WS_CLIENT_ASSET_STATUS";

  char* getcwd_result_ptr = getcwd(NULL, MAX_PROGRAMM_PATH_LEN);

  RECORDS_FILEPATH = getcwd_result_ptr + RECORDS_FILENAME;

  free(getcwd_result_ptr);

  std::string asset_names_array[ASSETS_AMOUNT];

  FILE* my_file = fopen(RECORDS_FILEPATH.c_str(), "r");
  
  if(!my_file)
  {
    printf("Error opening file with records\n");
    return -1;
  }
  
  records_amount = load_all_names(asset_names_array, my_file);
  
  fclose(my_file);

  FILE* temp = fopen(QUEUE_PATHNAME.c_str(), "a+");

  fclose(temp);

  int ws_command_queue_fd = queue_get_access(QUEUE_PATHNAME);
       
  ws_msg_buf_t recieved_cmd = {};

  main_sighandler(0, NULL, (void*)&ws_command_queue_fd);
  set_main_sigint_handler();
   
  if((msgrcv(ws_command_queue_fd, (PingConnection*) &ping, sizeof(PingConnection) - sizeof(long), 0, 0)) < 0)
  {
    printf("Can\'t receive ping message from queue\n");
    delete_queue(ws_command_queue_fd);
    exit(-1);
  }

  ping_response.ready_to_recieve = 1;
  ping_response.mtype = ping.accept_msgtype;

  printf("ping respponse type:: %d\n", ping.accept_msgtype);
  
  if(msgsnd(ws_command_queue_fd, (PingResult*) &ping_response, sizeof(PingResult) - sizeof(long), 0) < 0)
  {
    printf("Can\'t send ping_response to queue\n");
    perror("msgsnd");
  }
  else
    printf("Ping msg successfully sent\n");  

  while(1)
  {   
    if((msgrcv(ws_command_queue_fd, (ws_msg_buf_t*) &recieved_cmd, sizeof(ws_msg_buf_t) - sizeof(long), ping.mtype, 0)) < 0)
    {
      printf("Can\'t receive message from queue\n");
      delete_queue(ws_command_queue_fd);
      exit(-1);
    }
    else
    {
      if(recieved_cmd.action == ESTABLISH_CONNECTION)
      {
        if(processes_running[recieved_cmd.asset] != 0)
          kill_child_process(processes_running[recieved_cmd.asset]);

        if ((pid = fork()) < 0)
        {
          printf("Bad fork!\n");
        }

        if (pid > 0)   /***** Parent *****/
        {
          processes_running[recieved_cmd.asset] = pid;
        }
        else
        {
          OlymptradeWsClient olymp_client;
          
          child_sighandler(0, NULL, (void*)&olymp_client);
          set_child_sigint_handler();

          olymp_client.run_client(RECORDS_FILEPATH, recieved_cmd.asset, &asset_names_array[recieved_cmd.asset], ws_command_queue_fd);

          break;
        }
      }
      else if(recieved_cmd.action == CLOSE_CONNECTION)
      {
        kill_child_process(processes_running[recieved_cmd.asset]);
      }
    }
  } 

  return 0; 
}

int kill_child_process(pid_t& pid_to_kill)
{
  int status;
  kill(pid_to_kill, SIGINT);
  waitpid(-1, &status, WNOHANG);
  pid_to_kill = 0;
}

int delete_bracket(const char* str_to_clean)
{
  if(!str_to_clean)
  {
    printf("[Delete bracket]:Invalid string to clean pointer\n");
    return -1;
  }
  int str_length = strlen(str_to_clean);

  if(((char*)str_to_clean)[str_length - 2] != ']')
    ((char*)str_to_clean)[str_length - 1] = '\0';
  else
    ((char*)str_to_clean)[str_length - 2] = '\0';
}

int load_all_names(std::string names[], FILE* file_from)
{
  if(!file_from)
  {
    printf("[Load records names]: Invalid file pointer to read from\n");
  }

  int counter = 0;

  char current_name[20] = {};

  while(!feof(file_from))
  {
    fscanf(file_from, "[[Record_name:%s]", current_name);
    delete_bracket(current_name);
   
    names[counter++].assign(current_name, strlen(current_name));
   
    fscanf(file_from, "%*[^\n]\n", NULL);
  }

  return counter;
}

static void set_main_sigint_handler()                            // register the signal SIGINT handler 
{
  struct sigaction act;
  act.sa_sigaction = main_sighandler;
  act.sa_flags = SA_SIGINFO;
  sigemptyset(&act.sa_mask);
  sigaction(SIGINT, &act, 0);
}

static void main_sighandler(int sig, siginfo_t* siginfo, void* data)   
{
  static int call_counter = 0;

  static int* queue_id = NULL;

  int status = 0;

  if(!call_counter++)
    queue_id = (int*)data;
  else
  {
    for(int i = 0; i < ASSETS_AMOUNT; i++)
    {
      waitpid(-1, &status, WNOHANG);
    }

    delete_queue(*queue_id);

    exit(0);
  }
}

static void set_child_sigint_handler()                            // register the signal SIGINT handler 
{
  struct sigaction act;
  act.sa_sigaction = child_sighandler;
  act.sa_flags = SA_SIGINFO;
  sigemptyset(&act.sa_mask);
  sigaction(SIGINT, &act, 0);
}

static void child_sighandler(int sig, siginfo_t* siginfo, void* data)   
{
  static int call_counter = 0;

  static OlymptradeWsClient* current_client = NULL;

  if(!call_counter++)
    current_client = (OlymptradeWsClient*)data;
  else
    current_client -> close_connection();
}

int queue_get_access(const std::string& pathname_to_use)
{ 
  int msgid = 0;
  key_t key; 

  if ((key = ftok(pathname_to_use.c_str(), 0)) < 0)
  {
    printf("Can\'t generate key\n");
    exit(-1);
  }

  if((msgid = msgget(key, 0666 | IPC_CREAT)) < 0)
  {
    printf("Can\'t get msgid\n");
    exit(-1);
  }
   
   return msgid;
}

void delete_queue(int msgid)
{
  msgid = 0;
  /*
   if(msgctl(msgid, IPC_RMID, (struct msqid_ds*)NULL) < 0)
   {
      printf("Can't delete queue\n");
      exit(-1);
   }
   */
}
 