#pragma once
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <stdlib.h>
#include <sys/ipc.h>
#include <assert.h>
#include <sys/msg.h>
#include <inttypes.h>
#include "AssetNames.h"
#include "ParseOlymptradeJSON.h"

namespace deals_namespace
{
  const int PING_RESPONSE_CONSTANT        = 228;
  const int PING_CONSTANT                 = 322;
  const int MAX_DEALS_OPEN                = 10;
  const int DEAL_STATUS_MTYPE             = 100;
  const int DEAL_RESULTS_MTYPE            = 200;
  const int NEW_DEAL_MTYPE                = 400;
  const int MAX_USERNAME_LENGTH           = 200;
  
  const std::string asset_names_filename = "/all_assets.txt";
  const std::string bets_result_status[2] = {"loose", "win"};
  
  struct NewBet
  {
    long mtype;
    int bet_id;
    int asset;
    bool direction;
    int deal_amount;
    time_t timeframe;
  };

  struct BetStatus
  {
    long mtype;
    int bet_id;
    bool opening_result;
  };

  struct DealResult
  {
    long mtype;
    int bet_id;
    bool result; 
    double balance_change;
    double balance_result;
  };

  struct PingConnection
  {
    long mtype;
    int ping_const;
  };

  struct PingResult
  {
    long mtype;
    bool ready_to_recieve;
  };
};

class OlyClientDealService
{
  private:

    AssetNames names;
    ParseOlymptradeJSON* parsed_deals_status;
    int ping_success;
    int asset_deals_queue_fd;
    int free_bet_array_positions_amount;
    int assets_amount;
    std::string deals_queue_file_pathname;
  
    deals_namespace::NewBet bets_array[deals_namespace::MAX_DEALS_OPEN];
    bool bet_status_recieved[deals_namespace::MAX_DEALS_OPEN];

    void open_deals_queue();

    void close_deals_queue();
    
    void get_ping_msg();

    void response_ping_msg();

    void send_deal_result(deals_namespace::DealResult* deal_result);

    void send_deal_status(deals_namespace::BetStatus* bet_status);

    int find_asset_num(const std::string* asset);

    int define_bet_id(const AssetDeal* deal_to_serve);

  public:

    void set_deals_status_json_handler(ParseOlymptradeJSON* parsed_deals_status);

    std::vector<deals_namespace::NewBet> get_new_bets();                  //make new bets

    void service_deal_status();                                           //deal status service

    void update_deals(ParseOlymptradeJSON* parsed_data);                  //deal result recieve

    OlyClientDealService();
};