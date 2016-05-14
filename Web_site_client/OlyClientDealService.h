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
#include "SysStructs.h"
#include "AssetNames.h"
#include "ParseOlymptradeJSON.h"


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
  
    deal_structs::NewDeal bets_array[MAX_DEAL_AMOUNT];
    bool bet_status_recieved[MAX_DEAL_AMOUNT];

    void open_deals_queue();

    void close_deals_queue();
    
    void get_ping_msg();

    void response_ping_msg();

    void send_deal_result(deal_structs::DealResult* deal_result);

    void send_deal_status(deal_structs::DealStatus* bet_status);

    int find_asset_num(const std::string* asset);

    int define_bet_id(const AssetDeal* deal_to_serve);

  public:

    void set_deals_status_json_handler(ParseOlymptradeJSON* parsed_deals_status);

    std::vector<deal_structs::NewDeal> get_new_bets();                  //make new bets

    void service_deal_status();                                           //deal status service

    void update_deals(ParseOlymptradeJSON* parsed_data);                  //deal result recieve

    OlyClientDealService();
};