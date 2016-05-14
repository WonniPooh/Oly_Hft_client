#pragma once
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <inttypes.h>
#include "SysStructs.h"
#include "AssetNames.h"
 
class AssetsWinpercRecieve
{
  private:
  	int asset;
    int ping_recieved;
    int assets_amount;
    AssetNames names;
    uint16_t AssetWinperc;
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