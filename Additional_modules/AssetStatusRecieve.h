#pragma once
#include <string>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

namespace assetstatus_namespace
{
  const int ASSETS_AMOUNT = 18;
  const int recive_mtype = 500;
  const int MAX_USERNAME_LENGTH = 200;
  const int status_changed_mtype = 2000;
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

//int* function();

//std::auto_ptr< int > ptr(function());

//std::tr1::shared_ptr< int > shared_ptr(function());
 
class AssetsStatusRecieve
{
  private:
  	int asset;
    int ping_recieved;
    bool asset_availability;
    int asset_status_queue_fd;
    std::string status_queue_file_pathname;

    void open_status_queue();
    
    void close_status_queue();

    void ping_connection();

  public:

    AssetsStatusRecieve();

    void set_serviced_asset(int asset_set_num);

    void update_availability();

    bool get_availability();
};