//TODO::investing.com signals handler
//g++ a_olymptrade.cpp ParseOlymptradeJSON.cpp CurlOlymptradeActions.cpp json11/json11.cpp -std=c++11 -lcurl

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <stdio.h>
#include <sys/time.h>
#include <string>
#include <iostream>
#include <sstream>
#include <assert.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <inttypes.h>
#include "WinpercCommander.h"
#include "WsClientCommander.h"
#include "CurlOlymptradeActions.h"
#include "ParseOlymptradeJSON.h"

//подключаем curl
#include "curl/curl.h"
#pragma comment(lib,"curllib.lib");

const int ASSETS_AMOUNT = 18;
/*
typedef struct BET_PROPERTIES
{
  long msgtyp;
  int asset;
  std::uint64_t id; 
  std::uint32_t sum;
  std::time_t timeframe;
  bool direction;
} bet_properties_t;

typedef struct BET_RESPONSE
{
  long msgtyp;
  std::uint64_t id; 
  bool result;
} bet_response_t;
*/
size_t recieved_data_process(char *response_data, size_t size, size_t nmemb, void *user_func_args)
{
  static int times_called = 0;
  static WsClientCommander commander;
  static WinpercCommander  winperc_commander;
  static ParseOlymptradeJSON parse_data_even;
  static ParseOlymptradeJSON parse_data_odd;
  static ParseOlymptradeJSON parse_new_deals_data[ASSETS_AMOUNT];

  std::string input(response_data, size * nmemb);
  
  if(!times_called)
  {
    times_called++;
    parse_data_odd.parse_login_json(input.c_str());                //counting from one, parse login response;
    
    if(!parse_data_odd.get_login_result())
    {
      const std::string& login_error = parse_data_odd.get_login_error();
      std::cout << "Login failed: " << login_error << std::endl;
      exit(0);
    }
    else
      std::cout << "Login success" << std::endl;  
  }
  else
  {
    std::size_t found = input.find("\"content\"");

    if(found != std::string::npos)                               //bet_data
    {

    }
    else
    {
      found = input.find("\"result\"");

      if(found == std::string::npos)
      {
        times_called++;
        // parse update data; compare status for each asset; if sth changed send it to the ws_client; send winperc and other stuff to corresponding neurons
        if(times_called % 2)                                          //odd
        {
          parse_data_odd.parse_update_json(input.c_str());
          commander.process_current_status(parse_data_odd);
          winperc_commander.update(parse_data_odd);
        }
        else                                                          //even
        {
          parse_data_even.parse_update_json(input.c_str());
          commander.process_current_status(parse_data_even);
          winperc_commander.update(parse_data_even);
        }
      }
    }
  }

  return size * nmemb;
}

int main()
{
  std::string login;
  std::string password;
  std::cout <<"Enter user email: ";     getline(std::cin, login);
  std::cout <<"Enter user password: ";  getline(std::cin, password);
  std::cout << std::endl;

  CurlOlymptradeActions curl_connection;

  std::vector<bet_properties_t> ololo;

  curl_connection.log_into_platform(login, password, recieved_data_process);

  while(1)
  {
    curl_connection.send_requests(ololo);
    sleep(1);
  }

 /* while(1)
  {
    get_bet_data_from_neuro();
    add_bet_to_curl_handle();
    add_update_status_to_curl_handle();
    curl_multi_perform();
    compose_new_data_struct();
    send_new_data_to_neuro();
    check_assets_conditions();
    if_new_day_then_set_new_connection();
    wait_time_left_before_next_update();
  }
*/
/*
  bet_properties_t bet;
  bet.duration = 60;
  bet.amount = 30;
  bet.pair = "EURUSD_OTC";
  bet.direction = "down";

  bet_properties_t bet2;
  bet2.duration = 60;
  bet2.amount = 30;
  bet2.pair = "AUDUSD_OTC";
  bet2.direction = "down";


  curl_connection.switch_to_demo();


  std::vector<bet_properties_t> vec;

  vec.push_back(bet);

  vec.push_back(bet2);

  curl_connection.send_requests(vec);
*/
  return 0;  
}

/*
run client; update every sec -> update; (at the end of the second, when all neurons(theoretically) have already process data and now handle a result)
*/