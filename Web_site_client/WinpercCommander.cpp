#include "WinpercCommander.h"

void WinpercCommander::init_winperc_queue(int asset)
{ 
  key_t key; 

  std::string queue_file_pathname = asset_winperc_pathname_const + winperc_namespace::assets_names[asset];

  FILE* temp = fopen(queue_file_pathname.c_str(), "a+");

  fclose(temp);

  if ((key = ftok(queue_file_pathname.c_str(), 0)) < 0)
  {
    printf("Can\'t generate key\n");
    return;
  }

  if((winperc_queue_fd[asset] = msgget(key, 0666 | IPC_CREAT)) < 0)
  {
    printf("Can\'t get msgid\n");
    return;
  }
}

void WinpercCommander::transmit_winperc_data(int asset)
{
  winperc_namespace::ASSET_WINPERC post_data = {};
  post_data.winperc = current_status.winperc;
  post_data.mtype = asset + winperc_namespace::winperc_changed_mtype;

  if(msgsnd(winperc_queue_fd[asset], (winperc_namespace::ASSET_WINPERC*) &post_data, sizeof(winperc_namespace::ASSET_WINPERC) - sizeof(long), 0) < 0)
  {
    printf("Asset %s:: Can\'t send message to queue\n", winperc_namespace::assets_names[asset].c_str());
  }
}
    
void WinpercCommander::delete_winperc_queue(int asset)
{
  if(msgctl(winperc_queue_fd[asset], IPC_RMID, (struct msqid_ds*)NULL) < 0)
  {
    printf("WinpercCommander::close_winperc:: Can't delete queue\n");
  }

  //remove(queue_file_pathname[i].c_str());       //TODO
}

void WinpercCommander::open_status_queue()
{ 
  key_t key;

  FILE* temp = fopen(status_queue_file_pathname.c_str(), "a+");

  fclose(temp);

  if ((key = ftok(status_queue_file_pathname.c_str(), 0)) < 0)
  {
    printf("Can\'t generate key\n");
    return;
  }

  if((asset_status_queue_fd = msgget(key, 0666 | IPC_CREAT)) < 0)
  {
    printf("Can\'t get msgid\n");
    return;
  }
}

void WinpercCommander::transmit_availability()  
{ 
  if(msgsnd(asset_status_queue_fd, (winperc_namespace::ASSET_AVAILABLE*) &availability, sizeof(winperc_namespace::ASSET_AVAILABLE) - sizeof(long), 0) < 0)
  {
    printf("Asset %s:: Can\'t send message to queue\n", winperc_namespace::assets_names[availability.asset].c_str());
  }  
}
    
void WinpercCommander::delete_status_queue()
{
  if(msgctl(asset_status_queue_fd, IPC_RMID, (struct msqid_ds*)NULL) < 0)
  {
    printf("Asset status: Can't delete queue\n");
  }

  //remove(queue_file_pathname[i].c_str());       //TODO
}

void WinpercCommander::send_ping_msg()
{
  winperc_namespace::PingConnection ping = {1, winperc_namespace::recive_mtype};

  if(msgsnd(asset_status_queue_fd, (winperc_namespace::PingConnection*) &ping, sizeof(ping) - sizeof(long), 0) < 0)
  {
    printf("WsC1ientCommander::send_ping_msg:: Can\'t send message to queue\n");
    perror("msgsnd");
  }
  else
    printf("send_ping_msg:: Msg successfully sent\n");
}

void WinpercCommander::recieve_from_queue()
{
  winperc_namespace::PingResult ping_result;

  int rcv_result = msgrcv(asset_status_queue_fd, (winperc_namespace::PingResult*) &ping_result, sizeof(ping_result) - sizeof(long), winperc_namespace::recive_mtype, IPC_NOWAIT);

  if (rcv_result < 0)
  {
    printf("WsC1ientCommander::recieve_ping_response::error recieving msg\n");
    perror("msgrcv");
  }
  else
  {
    if (ping_result.ready_to_recieve == false)
     {
       connection_opened = 0;
       first_time_availability_called = 1;
       first_time_winperc_called = 1;         
     }

    if(ping_result.ready_to_recieve == true)
      connection_opened = 1;
  }
}

WinpercCommander::WinpercCommander()
{
  connection_opened = 0;
  asset_status_queue_fd = 0;                  
  first_time_winperc_called = 1;
  first_time_availability_called = 1;
  winperc_queue_fd[winperc_namespace::ASSETS_AMOUNT] = {};

  char current_username[winperc_namespace::MAX_USERNAME_LENGTH] = {};
  getlogin_r(current_username, winperc_namespace::MAX_USERNAME_LENGTH);

  asset_winperc_pathname_const = std::string("/home/") + std::string(current_username) + std::string("/"); 
  status_queue_file_pathname =  asset_winperc_pathname_const + winperc_namespace::status_queue_filename;

  prev = {};
  current = {};
  prev_status = {};
  current_status = {};
  win_percentage = {};
  
  availability = {};            
  availability.mtype = winperc_namespace::status_changed_mtype;
}

void WinpercCommander::update(ParseOlymptradeJSON& current_parsed)
{
  prev = current;
  current = current_parsed;
  
  if(first_time_availability_called)
  {
    first_time_availability_called = 0;
    open_status_queue();
    send_ping_msg();

    for(int i = 0; i < winperc_namespace::ASSETS_AMOUNT; i++)
    {
      current_status = current.get_asset_status(i);

      if(!current_status.locked)
      {
        availability.locked = !current_status.locked;
        availability.asset = i;

        printf("connection_actions.asset_name::%s\n", winperc_namespace::assets_names[i].c_str());
        printf("current_status.locked::%d\n", current_status.locked);

        transmit_availability();
      }
    }
  }
  else
  {
    recieve_from_queue();

    if(connection_opened)
    {
      for(int i = 0; i < winperc_namespace::ASSETS_AMOUNT; i++)
      {
        prev_status = prev.get_asset_status(i);
        current_status = current.get_asset_status(i);

        if(prev_status.locked != current_status.locked)
        {
          availability.locked = !current_status.locked;
          availability.asset = i;
          
          printf("connection_actions.asset_name::%s\n", winperc_namespace::assets_names[i].c_str());
          printf("current_status.locked::%d\n", current_status.locked);

          transmit_availability();
        }

        if(prev_status.locked == false && current_status.locked == true)
          delete_winperc_queue(i);

      }

      if(first_time_winperc_called)
      {
        first_time_winperc_called = 0;

        for(int i = 0; i < winperc_namespace::ASSETS_AMOUNT; i++)
        {
          current_status = current.get_asset_status(i);

          if(!current_status.locked)
          {
            init_winperc_queue(i);

            win_percentage.winperc = current_status.winperc;
            win_percentage.mtype = i + winperc_namespace::winperc_changed_mtype;

            printf("connection_actions.asset_name::%s\n", winperc_namespace::assets_names[i].c_str());
            printf("current_status.locked::%d\n", current_status.locked);

            transmit_winperc_data(i);
          }
        }
      }
      else
      {
        for(int i = 0; i < winperc_namespace::ASSETS_AMOUNT; i++)
        {
          prev_status = prev.get_asset_status(i);
          current_status = current.get_asset_status(i);

          if(prev_status.winperc != current_status.winperc && current_status.locked != 1)
          {
            win_percentage.winperc = current_status.winperc;
            win_percentage.mtype = i + winperc_namespace::winperc_changed_mtype;
            
            printf("connection_actions.asset_name::%s\n", winperc_namespace::assets_names[i].c_str());
            printf("current_status.locked::%d\n", current_status.locked);

            transmit_winperc_data(i);
          }
        }
      }
    }
    else
    {
      printf("WinpercCommander:: Connection is not currently opened; Can't update\n");
    }
  }
}