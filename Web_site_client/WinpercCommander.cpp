#include "WinpercCommander.h"

void WinpercCommander::init_winperc_queue()
{ 
  key_t key; 

  for(int i = 0; i < winperc_namespace::ASSETS_AMOUNT; i++)
  {
    std::string queue_file_pathname = asset_winperc_pathname_const + winperc_namespace::assets_names[i];

    FILE* temp = fopen(queue_file_pathname.c_str(), "a+");

    fclose(temp);

    if ((key = ftok(queue_file_pathname.c_str(), 0)) < 0)
    {
      printf("Can\'t generate key\n");
      return;
    }

    if((winperc_queue_fd[i] = msgget(key, 0666 | IPC_CREAT)) < 0)
    {
      printf("Can\'t get msgid\n");
      return;
    }
  }
}

void WinpercCommander::send_winperc_ping_msg(int asset)
{
  winperc_namespace::PingConnection ping = {winperc_namespace::winperc_changed_mtype, winperc_namespace::recive_mtype};

  if(msgsnd(winperc_queue_fd[asset], (winperc_namespace::PingConnection*) &ping, sizeof(ping) - sizeof(long), 0) < 0)
  {
    printf("WinpercCommander::send_winperc_ping_msg of asset %s:: Can\'t send message to queue\n", winperc_namespace::assets_names[asset].c_str());
    perror("msgsnd");
  }
  else
    printf("send_winperc_ping_msg:: Msg to %s asset successfully sent\n", winperc_namespace::assets_names[asset].c_str());
}

void WinpercCommander::recieve_winperc_ping_response(int asset)
{
  winperc_namespace::PingResult ping_result;

  if(!winperc_ping_success[asset])
  {
    int rcv_result = msgrcv(winperc_queue_fd[asset], (winperc_namespace::PingResult*) &ping_result, sizeof(ping_result) - sizeof(long), winperc_namespace::recive_mtype, IPC_NOWAIT);

    if (rcv_result < 0)
    {
      printf("WinpercCommander::recieve_winperc_ping_response for %s asset::error recieving msg\n", winperc_namespace::assets_names[asset].c_str());
      perror("msgrcv");
    }
    else
    {
      printf("WinpercCommander::recieve_winperc_ping_response:: ping message for %s asset successfully recieved!\n", winperc_namespace::assets_names[asset].c_str());

      winperc_ping_success[asset] = ping_result.ready_to_recieve;
    }
  }
}

void WinpercCommander::transmit_winperc_data()
{
  int asset_to_update = 0;
  int new_winperc = winperc_update_needed_count;
  winperc_namespace::ASSET_WINPERC post_data = {};

  for(int i = 0; i < winperc_update_needed_count && asset_to_update < winperc_namespace::ASSETS_AMOUNT; i++)
  {
    for(asset_to_update; asset_to_update < winperc_namespace::ASSETS_AMOUNT; asset_to_update++)
    {
      if(winperc_update_needed[asset_to_update])
        break;
    }

    if(!winperc_ping_success[asset_to_update])
    {
      recieve_winperc_ping_response(asset_to_update);
    }

    if(winperc_ping_success[asset_to_update])
    {
      post_data.winperc = current_status.winperc;
      post_data.mtype = winperc_namespace::winperc_changed_mtype;

      if(msgsnd(winperc_queue_fd[asset_to_update], (winperc_namespace::ASSET_WINPERC*) &post_data, sizeof(winperc_namespace::ASSET_WINPERC) - sizeof(long), 0) < 0)
      {
        printf("Asset %s:: Can\'t send update asset winperc message to queue\n", winperc_namespace::assets_names[asset_to_update].c_str());
      }  
      else
      {
        printf("Asset %s:: Update asset winperc message successfully sent\n", winperc_namespace::assets_names[asset_to_update].c_str());

        winperc_update_needed[asset_to_update] = 0;
        new_winperc--;        
      }
    }
    else
      printf("Asset %s:: Can\'t update winperc: ping success msg not recieved!\n", winperc_namespace::assets_names[asset_to_update].c_str());

    asset_to_update++;
  }

  winperc_update_needed_count = new_winperc;
}
    
void WinpercCommander::delete_winperc_queue()
{
  for(int i = 0; i < winperc_namespace::ASSETS_AMOUNT; i++)
  {
    if(msgctl(winperc_queue_fd[i], IPC_RMID, (struct msqid_ds*)NULL) < 0)
    {
      printf("WinpercCommander::close_winperc:: Can't delete queue\n");
    }

    std::string queue_file_pathname = asset_winperc_pathname_const + winperc_namespace::assets_names[i];

    remove(queue_file_pathname.c_str());
  }
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
  int asset_to_update = 0;

  winperc_namespace::ASSET_AVAILABLE availability = {};

  for(int i = 0; i < status_update_needed_count && asset_to_update < winperc_namespace::ASSETS_AMOUNT; i++)
  {
    for(asset_to_update; asset_to_update < winperc_namespace::ASSETS_AMOUNT; asset_to_update++)
    {
      if(status_update_needed[asset_to_update])
        break;
    }

    if(!status_ping_success[asset_to_update])
    {
      recieve_status_ping_response(asset_to_update);
    }

    if(status_ping_success[asset_to_update])
    {
      availability.mtype =  winperc_namespace::status_changed_mtype + asset_to_update;
      availability.asset = asset_to_update;
      availability.available = asset_status[asset_to_update];

      if(msgsnd(asset_status_queue_fd, (winperc_namespace::ASSET_AVAILABLE*) &availability, sizeof(winperc_namespace::ASSET_AVAILABLE) - sizeof(long), 0) < 0)
      {
        printf("Asset %s:: Can\'t send update asset status message to queue\n", winperc_namespace::assets_names[asset_to_update].c_str());
      }  
      else
      {
        printf("Asset %s:: Update asset status message successfully sent\n", winperc_namespace::assets_names[asset_to_update].c_str());

        status_update_needed[asset_to_update] = 0;
        status_update_needed_count--;        
      }
    }
    else
      printf("Asset %s:: Can\'t update status: ping success msg not recieved!\n", winperc_namespace::assets_names[asset_to_update].c_str());

    asset_to_update++;
  }
}
    
void WinpercCommander::delete_status_queue()
{
  if(msgctl(asset_status_queue_fd, IPC_RMID, (struct msqid_ds*)NULL) < 0)
  {
    printf("Asset status: Can't delete queue\n");
  }

  remove(status_queue_file_pathname.c_str());      
}

void WinpercCommander::send_status_ping_msg()
{
  for(int i = 0; i < winperc_namespace::ASSETS_AMOUNT; i++)
  {
    winperc_namespace::PingConnection ping = {winperc_namespace::status_changed_mtype + i, winperc_namespace::recive_mtype + i};

    if(msgsnd(asset_status_queue_fd, (winperc_namespace::PingConnection*) &ping, sizeof(ping) - sizeof(long), 0) < 0)
    {
      printf("WinpercCommander::send_status_ping_msg of asset %s:: Can\'t send message to queue\n", winperc_namespace::assets_names[i].c_str());
      perror("msgsnd");
    }
    else
      printf("send_status_ping_msg:: Msg to %s asset successfully sent\n", winperc_namespace::assets_names[i].c_str());
  }
}

void WinpercCommander::recieve_status_ping_response(int asset)
{
  winperc_namespace::PingResult ping_result;

  if(!status_ping_success[asset])
  {
    int rcv_result = msgrcv(asset_status_queue_fd, (winperc_namespace::PingResult*) &ping_result, sizeof(ping_result) - sizeof(long), winperc_namespace::recive_mtype + asset, IPC_NOWAIT);

    if (rcv_result < 0)
    {
      printf("WinpercCommander::recieve_status_ping_response for %s asset::error recieving msg\n", winperc_namespace::assets_names[asset].c_str());
      perror("msgrcv");
    }
    else
    {
      printf("WinpercCommander::recieve_status_ping_response:: ping message for %s asset successfully recieved!\n", winperc_namespace::assets_names[asset].c_str());

      status_ping_success[asset] = ping_result.ready_to_recieve;
    }
  }
}

WinpercCommander::WinpercCommander()
{
  for(int i = 0; i < winperc_namespace::ASSETS_AMOUNT; i++)
  {
    winperc_queue_fd[i] = 0; 
    winperc_status_changed[i] = 0;
    winperc_ping_success[i] = 0;
    winperc_update_needed[i] = 0;
    asset_winperc[i] = 0;

    status_ping_success[i] = 0;
    status_update_needed[i] = 1;
    asset_status[i] = 0;
  }
  
  current = {};
  current_status = {};

  status_update_needed_count = winperc_namespace::ASSETS_AMOUNT;
  winperc_update_needed_count = 0;

  asset_status_queue_fd = 0;                  
  first_time_winperc_called = 1;
  first_time_availability_called = 1;

  char current_username[winperc_namespace::MAX_USERNAME_LENGTH] = {};
  getlogin_r(current_username, winperc_namespace::MAX_USERNAME_LENGTH);

  asset_winperc_pathname_const = std::string("/home/") + std::string(current_username) + std::string("/"); 
  status_queue_file_pathname =  asset_winperc_pathname_const + winperc_namespace::status_queue_filename;
}

WinpercCommander::~WinpercCommander()
{
  delete_status_queue();
  delete_winperc_queue();
}

void WinpercCommander::update(ParseOlymptradeJSON& current_parsed)
{
  current = current_parsed;
  
  if(first_time_availability_called)
  {
    first_time_availability_called = 0;
    open_status_queue();
    send_status_ping_msg();

    for(int i = 0; i < winperc_namespace::ASSETS_AMOUNT; i++)
    {
      current_status = current.get_asset_status(i);

      asset_status[i] = !current_status.locked;
    }

    transmit_availability();
  }
  
  if(!first_time_availability_called)
  {
    for(int i = 0; i < winperc_namespace::ASSETS_AMOUNT; i++)
    {
      current_status = current.get_asset_status(i);
      
      if(asset_status[i] != !current_status.locked)
      {
        winperc_status_changed[i] = 1;
        asset_status[i] = !current_status.locked;
        status_update_needed[i] = 1;
        status_update_needed_count++;
      }
    }

    transmit_availability();
  }
 
  if(first_time_winperc_called)
  {
    first_time_winperc_called = 0;
    init_winperc_queue();

    for(int i = 0; i < winperc_namespace::ASSETS_AMOUNT; i++)
    {
      current_status = current.get_asset_status(i);
      
      if(asset_status[i])
      {
        send_winperc_ping_msg(i);
        current_status = current.get_asset_status(i);
        asset_winperc[i] = current_status.winperc;
        winperc_update_needed[i] = 1;
        winperc_update_needed_count++;
      }
    }
    transmit_winperc_data();
  }
  
  if(!first_time_winperc_called)
  {
    for(int i = 0; i < winperc_namespace::ASSETS_AMOUNT; i++)
    {
      current_status = current.get_asset_status(i);

      if(asset_status[i])
      {
        if(winperc_status_changed[i])
        {
          winperc_status_changed[i] = 0;
          winperc_ping_success[i] = 0;
          send_winperc_ping_msg(i);
        }

        if(asset_winperc[i] != current_status.winperc)
        {
          asset_winperc[i] = !current_status.locked;
          winperc_update_needed[i] = 1;
          winperc_update_needed_count++;
        }
      }      
    }
    transmit_winperc_data();    
  }
}