#include "AssetQuoteRecieve.h"

AssetQuoteRecieve::AssetQuoteRecieve()
{
  asset = -1;
  ping_recieved = 0;
  asset_quote_queue_fd = 0;
  current_quote = {};  

  char current_username[MAX_USERNAME_LENGTH] = {};
  getlogin_r(current_username, MAX_USERNAME_LENGTH);

  std::string assets_filepath = std::string("/home/") + std::string(current_username) + asset_names_filename;
  names.load_asset_names(&assets_filepath);
  assets_amount = names.get_assets_amount();
}
 
void AssetQuoteRecieve::open_quote_queue()
{ 
  key_t key;

  FILE* temp = fopen(quote_queue_file_pathname.c_str(), "a+");

  printf("%s\n", quote_queue_file_pathname.c_str());

  fclose(temp);

  if ((key = ftok(quote_queue_file_pathname.c_str(), 0)) < 0)
  {
    printf("Can\'t generate key\n");
    return;
  }

  if((asset_quote_queue_fd = msgget(key, 0666 | IPC_CREAT)) < 0)
  {
    printf("Can\'t get msgid\n");
    return;
  }
}

void AssetQuoteRecieve::close_quote_queue()
{
  asset_quote_queue_fd = 0;
}

void AssetQuoteRecieve::ping_connection()
{
  ping_structs::PingConnection recieved_ping = {};

  int rcv_result = msgrcv(asset_quote_queue_fd, (ping_structs::PingConnection*) &recieved_ping, sizeof(recieved_ping) - sizeof(long), WSCLIENT_QUOTE_MTYPE, IPC_NOWAIT);

  if (rcv_result < 0)
  {
    printf("AssetQuoteRecieve::ping connection::error recieving ping msg\n");
    perror("msgrcv");
  }
  else
  {
    ping_recieved = 1;
    printf("AssetQuoteRecieve::ping connection::ping msg successfully recieved\n");    

    ping_structs::PingResult ping = {};
    ping.mtype = WSCLIENT_PING_MSG_RESPONSE;
    ping.ready_to_recieve = recieved_ping.ping_data;

    if(msgsnd(asset_quote_queue_fd, (ping_structs::PingResult*) &ping, sizeof(ping) - sizeof(long), 0) < 0)
    {
      printf("AssetQuoteRecieve::Can\'t respond to ping message\n");
    }  
  }
} 

void AssetQuoteRecieve::update_quote()
{
  if(asset == -1)
  {
    printf("Set please asset to serve\n");
    return;
  }

  if(!ping_recieved)
    ping_connection();

  if(asset_quote_queue_fd && ping_recieved)
  {
    current_quote = {};

    int rcv_result = msgrcv(asset_quote_queue_fd, (NewQuoteMsg*) &current_quote, sizeof(current_quote) - sizeof(long), WSCLIENT_QUOTE_MTYPE, IPC_NOWAIT);

    if (rcv_result < 0)
    {
      printf("AssetQuoteRecieve:: Asset %s:: update_quote::error recieving msg\n", names.get_asset_name(asset) -> c_str());
      perror("msgrcv");
    }
    else
    {
      printf("AssetQuoteRecieve:: Asset %s:: update_quote::update msg successfully recieved\n", names.get_asset_name(asset) -> c_str());
    }
  }
  else  
  {
    if(!asset_quote_queue_fd)
      printf("AssetQuoteRecieve:: queue connection closed; Can't update quote\n", names.get_asset_name(asset) -> c_str());
  }
}

void AssetQuoteRecieve::set_serviced_asset(int asset_set_num)
{
  if(asset == -1 && asset_set_num < assets_amount)
  {
    asset = asset_set_num;

    char current_username[MAX_USERNAME_LENGTH] = {};
    getlogin_r(current_username, MAX_USERNAME_LENGTH);

    quote_queue_file_pathname =  std::string("/home/") + std::string(current_username) + std::string("/") + *names.get_asset_name(asset);

    open_quote_queue();
    ping_connection();
  }
  else
  {
    printf("AssetQuoteRecieve:: Current asset is already set or error asset num; Create new variable to serve new asset or correct your input\n");
  }
}

const NewQuoteMsg* AssetQuoteRecieve::get_quote()
{
  if(asset == -1)
  {
    printf("Set please asset to serve\n");
    return NULL;
  }

  if(ping_recieved)
    return &current_quote;
  else
  {
    printf("Ping msg does not recieved! No data to return\n");
    return NULL;
  }
}