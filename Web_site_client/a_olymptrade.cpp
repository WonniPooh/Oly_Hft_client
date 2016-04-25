//TODO::investing.com signals handler
//g++ a_olymptrade.cpp ParseOlymptradeJSON.cpp OlyClientDealService.cpp WinpercCommander.cpp WsClientCommander.cpp CurlOlymptradeActions.cpp json11/json11.cpp -std=c++11 -lcurl

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
#include "OlyClientDealService.h"
#include "ParseOlymptradeJSON.h"

//подключаем curl
#include "curl/curl.h"
#pragma comment(lib,"curllib.lib");

const int ASSETS_AMOUNT = 18;

size_t recieved_data_process(char *response_data, size_t size, size_t nmemb, void *user_func_args);
void wait_time_left_before_next_update();

int main()
{
  std::string login;
  std::string password;
  bool playing_real = 0;
  std::cout <<"Enter user email: ";     getline(std::cin, login);
  std::cout <<"Enter user password: ";  getline(std::cin, password);
  std::cout <<"Will we play real? (1/0): ";  std::cin >> playing_real;
  std::cout << std::endl;

  OlyClientDealService deals_service;
  CurlOlymptradeActions curl_connection;

  curl_connection.log_into_platform(login, password, recieved_data_process, &deals_service);

  if(!playing_real)
    curl_connection.switch_to_demo();

  wait_time_left_before_next_update();

  std::vector<deals_namespace::NewBet> bet_props;

  while(1)
  {
    deals_service.service_deal_status();
    
    bet_props = deals_service.get_new_bets();

    printf("Bet amount:: %d\n\n\n\n", bet_props.size());

    if(bet_props.size())
      printf("%d %d %d %d %d\n\n", bet_props[0].bet_id, bet_props[0].asset, bet_props[0].direction, bet_props[0].deal_amount, bet_props[0].timeframe);

    curl_connection.send_requests(bet_props);
    
    wait_time_left_before_next_update();
  }

  return 0;  
}

void wait_time_left_before_next_update()
{
    unsigned int sec_in_nano = 1000000000;

    unsigned int update_sec_part = 700000000;

    struct timespec spec;
    long current_second_nano_part;

    clock_gettime(CLOCK_REALTIME, &spec);

    current_second_nano_part = spec.tv_nsec; // Convert nanoseconds to milliseconds

    if(current_second_nano_part > update_sec_part)
    {
      usleep((sec_in_nano + update_sec_part - current_second_nano_part) / 1000);
    }
    else
      usleep((update_sec_part - current_second_nano_part) / 1000);
}

size_t recieved_data_process(char *response_data, size_t size, size_t nmemb, void *user_func_args)
{
  static int times_called = 0;
  static WsClientCommander commander;
  static WinpercCommander  winperc_commander;
  static OlyClientDealService* deals_service = (OlyClientDealService*)user_func_args;
  static ParseOlymptradeJSON parse_bet_data;
  static ParseOlymptradeJSON parse_data_even;
  static ParseOlymptradeJSON parse_data_odd;

  std::string input(response_data, size * nmemb);
  
  if(!times_called)
  {
    times_called++;

    deals_service -> set_deals_status_json_handler(&parse_bet_data);

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

    if(found != std::string::npos)                                    //bet_data
    {
      parse_bet_data.parse_bet_response_json(input.c_str());
    }
    else
    {
      found = input.find("\"result\"");

      if(found == std::string::npos)
      {
        times_called++;
        
        if(times_called % 2)                                          //odd
        {
          parse_data_odd.parse_update_json(input.c_str());
          commander.process_current_status(parse_data_odd);
          winperc_commander.update(parse_data_odd);
          deals_service -> update_deals(&parse_data_odd);
        }
        else                                                          //even
        {
          parse_data_even.parse_update_json(input.c_str());
          commander.process_current_status(parse_data_even);
          winperc_commander.update(parse_data_even);
          deals_service -> update_deals(&parse_data_even);
        }
      }
    }
  }

  return size * nmemb;
}