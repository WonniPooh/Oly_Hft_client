#pragma once
#include <unistd.h>
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "SysStructs.h"
#include "AssetNames.h"
#include "ParseOlymptradeJSON.h"

class WsClientCommander   
{
  private:
    int assets_amount;
    int ws_queue_fd;
    int connection_opened;
    int first_time_called;
    MsgBufWsCreateNewConnection connection_actions;
    std::string queue_file_pathname;
    ParseOlymptradeJSON prev;
    ParseOlymptradeJSON current;
    AssetStatus prev_status;
    AssetStatus current_status;
    AssetNames names;

    void queue_get_access();

    void delete_queue();

    void send_ping_msg();

    void recieve_ping_response();

    void send_message();
    
    void recieve_asset_close_msg();

  public:    

    WsClientCommander();

    void process_current_status(ParseOlymptradeJSON& current_parsed);
};