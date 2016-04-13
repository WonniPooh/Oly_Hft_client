#pragma once

#include <stdlib.h>
#include <iostream> 
#include <string.h>
#include <assert.h>
#include "json11/json11.hpp"

struct AssetStatus
{
  bool locked;
  int  sentiment;
  int  winperc;
};

struct AssetDeal
{
  int amount;
  int time_open;
  int time_close;
  int winperc;
  int cancel_percent;

  double balance_change;
  double balance_result;
  double curs_close;
  double curs_current;
  double curs_open;

  bool demo;
  bool direction;

  std::string interim_status;
  std::string pair;
  std::string status;
  std::string time_close_default;
};

enum 
{
  AUDUSD,
  AUDUSD_OTC,
  EURCHF,
  EURJPY,
  EURRUB,
  EURUSD,
  EURUSD_OTC,
  GBPUSD,
  GBPUSD_OTC,
  USDCAD,
  USDCAD_OTC,
  USDCHF,
  USDCHF_OTC,
  USDJPY,
  USDJPY_OTC,
  USDRUB,
  XAGUSD,
  XAUUSD,
  BRENT
};

class ParseOlymptradeJSON
{
  private:
    static const int ASSETS_AMOUNT = 18;
    static const int MAX_DEAL_AMOUNT = 10;

    int login_result;
    std::string login_error_msg;

    int bet_result;
  	struct AssetDeal bet_status_response;

    struct AssetStatus assets_array[ASSETS_AMOUNT];
    struct AssetDeal current_deals[MAX_DEAL_AMOUNT];
    struct AssetDeal finished_deals[MAX_DEAL_AMOUNT];
    double balance;
    double demo_balance;

    std::string assets_names[ASSETS_AMOUNT] = {"AUDUSD", "AUDUSD_OTC", "EURCHF", "EURJPY", "EURRUB", 
                                         "EURUSD", "EURUSD_OTC", "GBPUSD", "GBPUSD_OTC", "USDCAD", "USDCAD_OTC",
                                         "USDCHF", "USDCHF_OTC", "USDJPY", "USDJPY_OTC", "USDRUB", "XAGUSD", "XAUUSD"};

    void parse_deals_data(std::map< std::string, json11::Json >& instance_DealsCurrentFinished, struct AssetDeal* deals_array, int cur_position);

  	void dump_asset_deal_struct(AssetDeal &struct_to_dump, FILE* dump_file);

  public:

    ParseOlymptradeJSON();
    const std::string& get_asset_name(int asset_num);
   
    void parse_update_json(std::string input);
    double get_balance();
    double get_demo_balance();
    AssetDeal& get_current_deal(int deal_num);
    AssetDeal& get_finished_deal(int deal_num);
    AssetStatus& get_asset_status(int asset);
 

    void parse_bet_json(std::string input);

   
    void parse_login_json(std::string input);
    const std::string& get_login_error();
    int get_login_result();
  
    
    void dump_bet_result_data(FILE* file_dump_to);
    void dump_update_data(FILE* file_dump_to);
};