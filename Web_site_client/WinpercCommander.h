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
  const int recive_mtype = 500;
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
    bool available;
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
    int asset_status_queue_fd;                    
    int status_update_needed_count;
    int first_time_availability_called;
    int status_ping_success[winperc_namespace::ASSETS_AMOUNT];
    int status_update_needed[winperc_namespace::ASSETS_AMOUNT];
    int asset_status[winperc_namespace::ASSETS_AMOUNT];
    std::string status_queue_file_pathname;
      
    int first_time_winperc_called;
    int winperc_update_needed_count;
    int winperc_queue_fd[winperc_namespace::ASSETS_AMOUNT];
    int winperc_status_changed[winperc_namespace::ASSETS_AMOUNT];
    int winperc_ping_success[winperc_namespace::ASSETS_AMOUNT];
    int winperc_update_needed[winperc_namespace::ASSETS_AMOUNT];
    int asset_winperc[winperc_namespace::ASSETS_AMOUNT];
    std::string asset_winperc_pathname_const;

    ParseOlymptradeJSON current;
    AssetStatus current_status;

    void init_winperc_queue();

    void send_winperc_ping_msg(int asset);

    void recieve_winperc_ping_response(int asset);

    void transmit_winperc_data();

    void delete_winperc_queue();

    void open_status_queue();

    void transmit_availability();

    void delete_status_queue();

    void send_status_ping_msg(); 

    void recieve_status_ping_response(int asset);

  public:

    WinpercCommander();

    ~WinpercCommander();

    void update(ParseOlymptradeJSON& current_parsed);
};