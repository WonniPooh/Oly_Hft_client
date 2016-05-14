#include "WinpercCommander.h"

WinpercCommander::WinpercCommander()
{
  char current_username[MAX_USERNAME_LENGTH] = {};
  getlogin_r(current_username, MAX_USERNAME_LENGTH);

  asset_winperc_pathname_const = std::string("/home/") + std::string(current_username) + std::string("/"); 
  status_queue_file_pathname =  asset_winperc_pathname_const + status_queue_filename;

  std::string assets_filepath = std::string("/home/") + std::string(current_username) + asset_names_filename;
  names.load_asset_names(&assets_filepath);
  assets_amount = names.get_assets_amount();

  winperc_queue_fd = new int[assets_amount];
  winperc_status_changed = new int[assets_amount];
  winperc_ping_success = new int[assets_amount];
  winperc_update_needed = new int[assets_amount];
  asset_winperc = new int[assets_amount];
  status_ping_success = new int[assets_amount];
  status_update_needed = new int[assets_amount];
  asset_status = new int[assets_amount];

  for(int i = 0; i < assets_amount; i++)
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
  
  current_status = {};

  status_update_needed_count = assets_amount;
  winperc_update_needed_count = 0;

  asset_status_queue_fd = 0;                  
  first_time_winperc_called = 1;
  first_time_availability_called = 1;
}

WinpercCommander::~WinpercCommander()
{
  delete [] winperc_queue_fd;
  delete [] winperc_status_changed;
  delete [] winperc_ping_success;
  delete [] winperc_update_needed;
  delete [] asset_winperc;
  delete [] status_ping_success;
  delete [] status_update_needed;
  delete [] asset_status;

  delete_status_queue();
  delete_winperc_queue();
}

void WinpercCommander::init_winperc_queue()
{ 
  key_t key; 

  for(int i = 0; i < assets_amount; i++)
  {
    std::string queue_file_pathname = asset_winperc_pathname_const + *(names.get_asset_name(i));

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
  ping_structs::PingConnection ping = {::WINPERC_COMMANDER_WINPERC_CHANGED_MTYPE, WINPERC_COMMANDER_PING_RESPONSE_MTYPE};

  if(msgsnd(winperc_queue_fd[asset], (::ping_structs::PingConnection*) &ping, sizeof(ping) - sizeof(long), 0) < 0)
  {
    printf("WinpercCommander::send_winperc_ping_msg of asset %s:: Can\'t send message to queue\n", names.get_asset_name(asset) -> c_str());
    perror("msgsnd");
  }
  else
    printf("send_winperc_ping_msg:: Msg to %s asset successfully sent\n", names.get_asset_name(asset) -> c_str());
}

void WinpercCommander::recieve_winperc_ping_response(int asset)
{
  ::ping_structs::PingResult ping_result;

  if(!winperc_ping_success[asset])
  {
    int rcv_result = msgrcv(winperc_queue_fd[asset], (::ping_structs::PingResult*) &ping_result, sizeof(ping_result) - sizeof(long), WINPERC_COMMANDER_PING_RESPONSE_MTYPE, IPC_NOWAIT);

    if (rcv_result < 0)
    {
      printf("WinpercCommander::recieve_winperc_ping_response for %s asset::error recieving msg\n", names.get_asset_name(asset) -> c_str());
      perror("msgrcv");
    }
    else
    {
      printf("WinpercCommander::recieve_winperc_ping_response:: ping message for %s asset successfully recieved!\n", names.get_asset_name(asset) -> c_str());

      winperc_ping_success[asset] = ping_result.ready_to_recieve;
    }
  }
}

void WinpercCommander::transmit_winperc_data()
{
  int asset_to_update = 0;
  int new_winperc = winperc_update_needed_count;
  status_structs::AssetWinperc post_data = {};

  for(int i = 0; i < winperc_update_needed_count && asset_to_update < assets_amount; i++)
  {
    for(asset_to_update; asset_to_update < assets_amount; asset_to_update++)
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
      post_data.mtype = ::WINPERC_COMMANDER_WINPERC_CHANGED_MTYPE;

      if(msgsnd(winperc_queue_fd[asset_to_update], (status_structs::AssetWinperc*) &post_data, sizeof(status_structs::AssetWinperc) - sizeof(long), 0) < 0)
      {
        printf("Asset %s:: Can\'t send update asset winperc message to queue\n", names.get_asset_name(asset_to_update) -> c_str());
      }  
      else
      {
        printf("Asset %s:: Update asset winperc message successfully sent\n", names.get_asset_name(asset_to_update) -> c_str());

        winperc_update_needed[asset_to_update] = 0;
        new_winperc--;        
      }
    }
    else
      printf("Asset %s:: Can\'t update winperc: ping success msg not recieved!\n", names.get_asset_name(asset_to_update) -> c_str());

    asset_to_update++;
  }

  winperc_update_needed_count = new_winperc;
}
    
void WinpercCommander::delete_winperc_queue()
{
  for(int i = 0; i < assets_amount; i++)
  {
    if(msgctl(winperc_queue_fd[i], IPC_RMID, (struct msqid_ds*)NULL) < 0)
    {
      printf("WinpercCommander::close_winperc:: Can't delete queue\n");
    }

    std::string queue_file_pathname = asset_winperc_pathname_const + *(names.get_asset_name(i));

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

  ::status_structs::AssetAvailable availability = {};

  for(int i = 0; i < status_update_needed_count && asset_to_update < assets_amount; i++)
  {
    for(asset_to_update; asset_to_update < assets_amount; asset_to_update++)
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
      availability.mtype = WINPERC_COMMANDER_STATUS_CHANGED_MTYPE + asset_to_update;
      availability.asset = asset_to_update;
      availability.available = asset_status[asset_to_update];

      if(msgsnd(asset_status_queue_fd, (::status_structs::AssetAvailable*) &availability, sizeof(::status_structs::AssetAvailable) - sizeof(long), 0) < 0)
      {
        printf("Asset %s:: Can\'t send update asset status message to queue\n", names.get_asset_name(asset_to_update) -> c_str());
      }  
      else
      {
        printf("Asset %s:: Update asset status message successfully sent\n", names.get_asset_name(asset_to_update) -> c_str());

        status_update_needed[asset_to_update] = 0;
        status_update_needed_count--;        
      }
    }
    else
      printf("Asset %s:: Can\'t update status: ping success msg not recieved!\n", names.get_asset_name(asset_to_update) -> c_str());

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
  for(int i = 0; i < assets_amount; i++)
  {
    ping_structs::PingConnection ping = {WINPERC_COMMANDER_STATUS_CHANGED_MTYPE + i, WINPERC_COMMANDER_PING_RESPONSE_MTYPE + i};

    if(msgsnd(asset_status_queue_fd, (ping_structs::PingConnection*) &ping, sizeof(ping) - sizeof(long), 0) < 0)
    {
      printf("WinpercCommander::send_status_ping_msg of asset %s:: Can\'t send message to queue\n", names.get_asset_name(i) -> c_str());
      perror("msgsnd");
    }
    else
      printf("send_status_ping_msg:: Msg to %s asset successfully sent\n", names.get_asset_name(i) -> c_str());
  }
}

void WinpercCommander::recieve_status_ping_response(int asset)
{
  ::ping_structs::PingResult ping_result;

  if(!status_ping_success[asset])
  {
    int rcv_result = msgrcv(asset_status_queue_fd, (::ping_structs::PingResult*) &ping_result, sizeof(ping_result) - sizeof(long), WINPERC_COMMANDER_PING_RESPONSE_MTYPE + asset, IPC_NOWAIT);

    if (rcv_result < 0)
    {
      printf("WinpercCommander::recieve_status_ping_response for %s asset::error recieving msg\n", names.get_asset_name(asset) -> c_str());
      perror("msgrcv");
    }
    else
    {
      printf("WinpercCommander::recieve_status_ping_response:: ping message for %s asset successfully recieved!\n", names.get_asset_name(asset) -> c_str());

      status_ping_success[asset] = ping_result.ready_to_recieve;
    }
  }
}

void WinpercCommander::update(ParseOlymptradeJSON& current_parsed)
{
  current = current_parsed;
  
  if(first_time_availability_called)
  {
    first_time_availability_called = 0;
    open_status_queue();
    send_status_ping_msg();

    for(int i = 0; i < assets_amount; i++)
    {
      current_status = current.get_asset_status(i);

      asset_status[i] = !current_status.locked;
    }

    transmit_availability();
  }
  
  if(!first_time_availability_called)
  {
    for(int i = 0; i < assets_amount; i++)
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

    for(int i = 0; i < assets_amount; i++)
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
    for(int i = 0; i < assets_amount; i++)
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