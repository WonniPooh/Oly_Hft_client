#pragma once

#include <mutex>
#include <stdlib.h>
#include <iostream> 
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include "json11/json11.hpp"
#include "AssetNames.h"
#include <vector>

struct AssetStatus
{
  bool locked;
  int  sentiment;
  int  winperc;
};

struct AssetDeal
{
  int result;
  int amount;
  int time_open;
  int time_close;
  int winperc;
  int deal_id;
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

class ParseOlymptradeJSON
{
  private:
	
	  std::string asset_names_filename = "/all_assets.txt";
    static const int MAX_DEAL_AMOUNT = 10;
	  int assets_amount;
    AssetNames names;

    int login_result;
    std::string login_error_msg;

    int bets_new_status_counter;
    std::vector<AssetDeal>bet_status_response;

    std::vector<AssetStatus>assets_array;
    AssetDeal current_deals[MAX_DEAL_AMOUNT];
    AssetDeal finished_deals[MAX_DEAL_AMOUNT];
    double balance;
    double demo_balance;

    void parse_deals_data(std::map< std::string, json11::Json >& instance_DealsCurrentFinished, AssetDeal* deals_array, int cur_position);

  	void dump_asset_deal_struct(AssetDeal &struct_to_dump, FILE* dump_file);

  public:

    ParseOlymptradeJSON();

    const std::string& get_asset_name(int asset_num);
   
    void parse_update_json(std::string input);
    double get_balance();
    double get_demo_balance();

    const AssetDeal* get_current_deal(int deal_num);
    const AssetDeal* get_finished_deal(int deal_num);
  
    const AssetStatus& get_asset_status(int asset);

    int get_new_bet_status_amount();
    void parse_bet_response_json(std::string input);
    AssetDeal get_deal_status();

    void parse_login_json(std::string input);
    int get_login_result();
    const std::string& get_login_error();
      
    void dump_bet_result_data(FILE* file_dump_to);
    void dump_update_data(FILE* file_dump_to);
};