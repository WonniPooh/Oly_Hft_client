#pragma once
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include "SysStructs.h"
#include "AssetNames.h"
#include "ParseOlymptradeJSON.h"

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