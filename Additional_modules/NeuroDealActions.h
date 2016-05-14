#pragma once
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <inttypes.h>
#include "SysStructs.h"
#include "AssetNames.h"

class NeuroDealAction
{
  private:
    AssetNames names;
    int ping_success;
    int assets_amount;
    int open_deals_amount;
    int free_array_position;
    int asset_deals_queue_fd;
    int new_bets_status_amount;
    int new_bets_result_amount;
    int bets_status_recieved_num[2 * MAX_DEAL_AMOUNT];
    int bets_result_recieved_num[2 * MAX_DEAL_AMOUNT];

  	deal_structs::NewDeal bets_array[2 * MAX_DEAL_AMOUNT];
    deal_structs::DealStatus bets_status_array[2 * MAX_DEAL_AMOUNT];
    deal_structs::DealResult bets_results_array[2 * MAX_DEAL_AMOUNT];

    std::string deals_queue_file_pathname;

    void open_deals_queue();

    void close_deals_queue();
    
    int recieve_status();

    int recieve_results();
    
    void ping_connection();

    void get_ping_answer();

  public:

    NeuroDealAction();

    ~NeuroDealAction();

    int update_deals();

    int make_new_bet(const std::vector<deal_structs::NewDeal>& bets_vector);      //returns -1 if connection is not opened

    const std::vector<deal_structs::DealStatus>& get_updated_status();

    const std::vector<deal_structs::DealResult>& get_updated_results();
};