#pragma once
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <inttypes.h>

namespace quote_namespace
{
  const int ASSETS_AMOUNT = 18;
  const int WSCLIENT_MSG_TYPE = 300;
  const int WSCLIENT_MSG_RCV = 400;
  const int MAX_USERNAME_LENGTH = 200;
  const std::string assets_names[ASSETS_AMOUNT] = {"AUDUSD", "AUDUSD_OTC", "EURCHF", "EURJPY", "EURRUB", 
                                                   "EURUSD", "EURUSD_OTC", "GBPUSD", "GBPUSD_OTC", "USDCAD", "USDCAD_OTC",
                                                   "USDCHF", "USDCHF_OTC", "USDJPY", "USDJPY_OTC", "USDRUB", "XAGUSD", "XAUUSD"};

  struct RecieveData
  {
    long mtype;
    uint64_t timestamp;
    double close;
  };

  struct PingConnection
  {
    long mtype;
    uint16_t asset;
  };

  struct PingResult
  {
    long mtype;
    bool ready_to_recieve;
  };
};

class AssetQuoteRecieve
{
  private:
  	int asset;
    int ping_recieved;
    int asset_quote_queue_fd;
    quote_namespace::RecieveData current_quote;
    std::string quote_queue_file_pathname;

    void open_quote_queue();
    
    void close_quote_queue();

    void ping_connection();

  public:

    AssetQuoteRecieve();

    void set_serviced_asset(int asset_set_num);

    void update_quote();

    const quote_namespace::RecieveData* get_quote();
};