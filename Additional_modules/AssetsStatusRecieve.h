#pragma once
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "SysStructs.h"
#include "AssetNames.h"

//int* function();

//std::auto_ptr< int > ptr(function());

//std::tr1::shared_ptr< int > shared_ptr(function());
 
class AssetsStatusRecieve
{
  private:
  	int asset;
    int ping_recieved;
    int assets_amount;
    bool asset_availability;
    int asset_status_queue_fd;
    std::string status_queue_file_pathname;
    AssetNames names;

    void open_status_queue();
    
    void close_status_queue();

    void ping_connection();

  public:

    AssetsStatusRecieve();

    void set_serviced_asset(int asset_set_num);

    void update_availability();

    int get_availability();
};