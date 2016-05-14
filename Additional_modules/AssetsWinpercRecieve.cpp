#include "AssetsWinpercRecieve.h"

AssetsWinpercRecieve::AssetsWinpercRecieve()
{
  asset = -1;
  ping_recieved = 0;
  AssetWinperc = 0;
  asset_winperc_queue_fd = 0;

  char current_username[MAX_USERNAME_LENGTH] = {};
  getlogin_r(current_username, MAX_USERNAME_LENGTH);

  std::string assets_filepath = std::string("/home/") + std::string(current_username) + asset_names_filename;
  names.load_asset_names(&assets_filepath);
  assets_amount = names.get_assets_amount();
}

void AssetsWinpercRecieve::open_winperc_queue()
{ 
  key_t key;

  FILE* temp = fopen(winperc_queue_file_pathname.c_str(), "a+");

  fclose(temp);

  if ((key = ftok(winperc_queue_file_pathname.c_str(), 0)) < 0)
  {
    printf("Can\'t generate key\n");
    return;
  }

  if((asset_winperc_queue_fd = msgget(key, 0666 | IPC_CREAT)) < 0)
  {
    printf("Can\'t get msgid\n");
    return;
  }
}

void AssetsWinpercRecieve::close_winperc_queue()
{
  asset_winperc_queue_fd = 0;
}

void AssetsWinpercRecieve::ping_connection()
{
  ping_structs::PingConnection recieved_ping = {};

  int rcv_result = msgrcv(asset_winperc_queue_fd, (ping_structs::PingConnection*) &recieved_ping, sizeof(recieved_ping) - sizeof(long), WINPERC_COMMANDER_WINPERC_CHANGED_MTYPE, IPC_NOWAIT);

  if (rcv_result < 0)
  {
    printf("AssetsWinpercRecieve::ping connection::error recieving ping msg\n");
    perror("msgrcv");
  }
  else
  {
    printf("AssetsWinpercRecieve::ping connection::ping msg successfully recieved\n");    

    ping_recieved = 1;

    ping_structs::PingResult ping = {};
    ping.mtype = recieved_ping.ping_data;
    ping.ready_to_recieve  = 1;

    if(msgsnd(asset_winperc_queue_fd, (ping_structs::PingResult*) &ping, sizeof(ping) - sizeof(long), 0) < 0)
    {
      printf("AssetsWinpercRecieve::Can\'t respond to ping message\n");
    }  
  }
} 

void AssetsWinpercRecieve::update_winperc()
{
  status_structs::AssetWinperc win_percentage;

  if(asset == -1)
  {
    printf("Set please asset to serve\n");
    return;
  }

  if(!ping_recieved)
    ping_connection();

  if(asset_winperc_queue_fd && ping_recieved)
  {
    win_percentage = {};

    int rcv_result = msgrcv(asset_winperc_queue_fd, (status_structs::AssetWinperc*) &win_percentage, sizeof(win_percentage) - sizeof(long), WINPERC_COMMANDER_WINPERC_CHANGED_MTYPE, IPC_NOWAIT);

    if (rcv_result < 0)
    {
      printf("AssetsWinpercRecieve:: Asset %s:: update_winperc::error recieving msg\n", names.get_asset_name(asset) -> c_str());
      perror("msgrcv");
    }
    else
    {
      printf("AssetsWinpercRecieve:: Asset %s:: update_winperc::update msg successfully recieved\n", names.get_asset_name(asset) -> c_str());
      AssetWinperc = win_percentage.winperc;
    }
  }
  else  
  {
    if(!asset_winperc_queue_fd)
      printf("AssetsWinpercRecieve:: queue connection closed; Can't update availability\n", names.get_asset_name(asset) -> c_str());
  }
}

void AssetsWinpercRecieve::set_serviced_asset(int asset_set_num)
{
  if(asset == -1 && asset_set_num < assets_amount)
  {
    asset = asset_set_num;
    
    char current_username[MAX_USERNAME_LENGTH] = {};
    getlogin_r(current_username, MAX_USERNAME_LENGTH);

    winperc_queue_file_pathname = std::string("/home/") + std::string(current_username) + std::string("/") + *names.get_asset_name(asset);
    open_winperc_queue();
    ping_connection();
  }
  else
  {
    printf("AssetsWinpercRecieve:: Current asset is already set or error asset num; Create new variable to serve new asset or correct your input\n");
  }
}

uint16_t AssetsWinpercRecieve::get_winperc()
{
  if(asset == -1)
  {
    printf("Set please asset to serve\n");
    return 0;
  }

  if(ping_recieved)
    return AssetWinperc;
  else
  {
    printf("Ping msg does not recieved! No data to return\n");
    return -1;
  }
}