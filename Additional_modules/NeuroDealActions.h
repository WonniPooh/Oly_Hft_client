#pragma once
#include <string>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <inttypes.h>

namespace deals_namespace
{
  const int PING_RESPONSE_CONSTANT       	= 228;
  const int PING_CONSTANT       					= 322;
  const int ASSETS_AMOUNT       					= 18;
  const int MAX_DEALS_OPEN      					= 10;
  const int DEAL_STATUS_MTYPE   					= 100;
  const int DEAL_RESULTS_MTYPE  					= 200;
  const int NEW_DEAL_MTYPE      					= 400;
  const int MAX_USERNAME_LENGTH 					= 200;

  const std::string assets_names[ASSETS_AMOUNT] = {"AUDUSD", "AUDUSD_OTC", "EURCHF", "EURJPY", "EURRUB", 
                                                   "EURUSD", "EURUSD_OTC", "GBPUSD", "GBPUSD_OTC", "USDCAD", "USDCAD_OTC",
                                                   "USDCHF", "USDCHF_OTC", "USDJPY", "USDJPY_OTC", "USDRUB", "XAGUSD", "XAUUSD"};
 
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

class NeuroDealAction
{
  private:
    int ping_success;
    int open_deals_amount;
    int free_array_position;
    int asset_deals_queue_fd;
    int new_bets_status_amount;
    int new_bets_result_amount;
    int bets_status_recieved_num[2 * deals_namespace::MAX_DEALS_OPEN];
    int bets_result_recieved_num[2 * deals_namespace::MAX_DEALS_OPEN];

  	deals_namespace::NewBet bets_array[2 * deals_namespace::MAX_DEALS_OPEN];
    deals_namespace::BetStatus bets_status_array[2 * deals_namespace::MAX_DEALS_OPEN];
    deals_namespace::DealResult bets_results_array[2 * deals_namespace::MAX_DEALS_OPEN];

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

    int make_new_bet(const std::vector<deals_namespace::NewBet>& bets_vector);      //returns -1 if connection is not opened

    const std::vector<deals_namespace::BetStatus>& get_updated_status();

    const std::vector<deals_namespace::DealResult>& get_updated_results();
};