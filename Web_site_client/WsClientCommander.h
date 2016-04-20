#pragma once
#include <unistd.h>
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ParseOlymptradeJSON.h"

namespace ws_namespace
{
  const int ASSETS_AMOUNT       = 18;
  const int MAX_USERNAME_LENGTH = 200;
  const int PING_MTYPE          = 300; 
  const int ASSET_CLOSED_MTYPE  = 700;
  const std::string queue_filename = "/WS_CLIENT_ASSET_STATUS";

  const std::string assets_names[ASSETS_AMOUNT] = {"AUDUSD", "AUDUSD_OTC", "EURCHF", "EURJPY", "EURRUB", 
                                                   "EURUSD", "EURUSD_OTC", "GBPUSD", "GBPUSD_OTC", "USDCAD", "USDCAD_OTC",
                                                   "USDCHF", "USDCHF_OTC", "USDJPY", "USDJPY_OTC", "USDRUB", "XAGUSD", "XAUUSD"};

  struct MsgBufWsCreateNewConnection
  {
    long msgtyp;
    int action;
    int asset;
  };

  struct ConnectionClosedMsg
  {
    long msgtyp;
    int asset;
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
}

class WsClientCommander   
{
  private:
    int ws_queue_fd;
    int connection_opened;
    int first_time_called;
    ws_namespace::MsgBufWsCreateNewConnection connection_actions;
    std::string queue_file_pathname;
    ParseOlymptradeJSON prev;
    ParseOlymptradeJSON current;
    AssetStatus prev_status;
    AssetStatus current_status;

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