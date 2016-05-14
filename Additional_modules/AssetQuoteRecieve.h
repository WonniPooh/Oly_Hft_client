#pragma once
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <inttypes.h>
#include "SysStructs.h"
#include "AssetNames.h"

class AssetQuoteRecieve
{
  private:
  	int asset;
    int assets_amount;
    int ping_recieved;
    int asset_quote_queue_fd;
    AssetNames names;
    NewQuoteMsg current_quote;
    std::string quote_queue_file_pathname;

    void open_quote_queue();
    
    void close_quote_queue();

    void ping_connection();

  public:

    AssetQuoteRecieve();

    void set_serviced_asset(int asset_set_num);

    void update_quote();

    const NewQuoteMsg* get_quote();
};