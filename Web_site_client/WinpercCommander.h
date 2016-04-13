#pragma once
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include "ParseOlymptradeJSON.h"

namespace winperc_namespace
{
  const int MAX_USERNAME_LENGTH = 200;
  const int ASSETS_AMOUNT = 18;
  const int recive_mtype = 666;
  const int status_changed_mtype = 2000;
  const int winperc_changed_mtype = 1000;
  const std::string status_queue_filename = "NEURO_ASSETS_STATUS";

  const std::string assets_names[ASSETS_AMOUNT] = {"AUDUSD", "AUDUSD_OTC", "EURCHF", "EURJPY", "EURRUB", 
                                             "EURUSD", "EURUSD_OTC", "GBPUSD", "GBPUSD_OTC", "USDCAD", "USDCAD_OTC",
                                             "USDCHF", "USDCHF_OTC", "USDJPY", "USDJPY_OTC", "USDRUB", "XAGUSD", "XAUUSD"};

  struct ASSET_AVAILABLE
  {
    long mtype;
    int asset;
    bool locked;
  };

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

class WinpercCommander
{
  private:
    int connection_opened;
    int first_time_availability_called;
    int first_time_winperc_called;

    int asset_status_queue_fd;                    
    int winperc_queue_fd[winperc_namespace::ASSETS_AMOUNT];
    winperc_namespace::ASSET_AVAILABLE availability;            
    winperc_namespace::ASSET_WINPERC win_percentage;
    std::string status_queue_file_pathname;
    std::string asset_winperc_pathname_const;
    ParseOlymptradeJSON prev;
    ParseOlymptradeJSON current;
    AssetStatus prev_status;
    AssetStatus current_status;

    void init_winperc_queue(int asset);

    void transmit_winperc_data(int asset);
    
    void delete_winperc_queue(int asset);

    void open_status_queue();

    void transmit_availability();
    
    void delete_status_queue();

    void send_ping_msg();

    void recieve_from_queue();

  public:

    WinpercCommander();

    void update(ParseOlymptradeJSON& current_parsed);
};