#pragma once
#include <unistd.h>
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ParseOlymptradeJSON.h"

namespace ws_namespace
{
  const int ASSETS_AMOUNT = 18;
  const int recive_type = 123; 
  const int MAX_USERNAME_LENGTH = 200;
  const std::string queue_filename = "/WS_CLIENT_ASSET_STATUS";

  const std::string assets_names[ASSETS_AMOUNT] = {"AUDUSD", "AUDUSD_OTC", "EURCHF", "EURJPY", "EURRUB", 
                                                   "EURUSD", "EURUSD_OTC", "GBPUSD", "GBPUSD_OTC", "USDCAD", "USDCAD_OTC",
                                                   "USDCHF", "USDCHF_OTC", "USDJPY", "USDJPY_OTC", "USDRUB", "XAGUSD", "XAUUSD"};

  typedef struct MsgBufWsCreateNewConnection
  {
    long msgtyp;
    int action;
    int asset;
  } ws_msg_buf_t;

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
    ws_namespace::ws_msg_buf_t connection_actions;
    std::string queue_file_pathname;
    ParseOlymptradeJSON prev;
    ParseOlymptradeJSON current;
    AssetStatus prev_status;
    AssetStatus current_status;

    void queue_get_access();

    void send_message();

    void recieve_from_queue();

    void delete_queue();

    void send_ping_msg();

  public:    

    WsClientCommander();

    void process_current_status(ParseOlymptradeJSON& current_parsed);
};