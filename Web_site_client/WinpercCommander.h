#pragma once
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include "AssetNames.h"
#include "ParseOlymptradeJSON.h"

namespace winperc_namespace
{
  const int MAX_USERNAME_LENGTH = 200;
  const int recive_mtype = 500;
  const int status_changed_mtype = 2000;
  const int winperc_changed_mtype = 1000;
  const std::string status_queue_filename = "NEURO_ASSETS_STATUS";

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
    int* status_ping_success;
    int* status_update_needed;
    int* asset_status;
    std::string status_queue_file_pathname;
      
    int first_time_winperc_called;
    int winperc_update_needed_count;
    int* winperc_queue_fd;
    int* winperc_status_changed;
    int* winperc_ping_success;
    int* winperc_update_needed;
    int* asset_winperc;
    std::string asset_winperc_pathname_const;

    int assets_amount;
    AssetNames names;
    ParseOlymptradeJSON current;
    AssetStatus current_status;
    const std::string asset_names_filename = "/all_assets.txt";

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