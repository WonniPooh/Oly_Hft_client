#pragma once
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>

namespace assetwinperc_namespace
{
  const int ASSETS_AMOUNT = 18;
  const int recive_mtype = 500;
  const int MAX_USERNAME_LENGTH = 200;
  const int winperc_changed_mtype = 1000;
  
  const std::string assets_names[ASSETS_AMOUNT] = {"AUDUSD", "AUDUSD_OTC", "EURCHF", "EURJPY", "EURRUB", 
                                                   "EURUSD", "EURUSD_OTC", "GBPUSD", "GBPUSD_OTC", "USDCAD", "USDCAD_OTC",
                                                   "USDCHF", "USDCHF_OTC", "USDJPY", "USDJPY_OTC", "USDRUB", "XAGUSD", "XAUUSD"};

  struct ASSET_WINPERC
  {
    long mtype;
    uint16_t winperc;
  };

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
};
 
class AssetsWinpercRecieve
{
  private:
  	int asset;
    int ping_recieved;
    uint16_t asset_winperc;
    int asset_winperc_queue_fd;
    std::string winperc_queue_file_pathname;

    void open_winperc_queue();
    
    void close_winperc_queue();

    void ping_connection();

  public:

    AssetsWinpercRecieve();

    void set_serviced_asset(int asset_set_num);

    void update_winperc();

    uint16_t get_winperc();
};