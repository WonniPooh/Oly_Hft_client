
#pragma once

#include <stdexcept>
#include <string>
#include <iostream>
#include <sys/time.h>
#include <sstream>
#include <assert.h>
#include <ctime>
#include "curl/curl.h"
#include <vector>
#include "OlyClientDealService.h"
#pragma comment(lib,"curllib.lib")

#include <unistd.h>

enum REQUEST_TYPES
{
  UPDATE_STATUS,
  MAKE_A_BET,
  LOG_IN,
  PLAY_DEMO,
  PLAY_REAL,
  LOG_OUT
};

class CurlOlymptradeActions
{
  private:
    
    static const int MAX_DEAL_AMOUNT = 10;
    static const int ASSETS_AMOUNT = 18;

    int log_in_success;

    CURL* curl_login_handle;
    std::string user_password;
    std::string user_login;
    std::string data_to_post;
    
    CURL* curl_status_handle;
    
    CURL* curl_bet_handle[MAX_DEAL_AMOUNT];

    CURLM* multi_handle;
    int initialized_multi_handle;
    
    struct curl_slist *login_headers;
    struct curl_slist *logout_headers;
    struct curl_slist *bet_headers;
    struct curl_slist *status_update_headers;

    size_t (*responce_data_process_pointer)(char *, size_t, size_t, void*);

    const std::string assets_names[ASSETS_AMOUNT] = {"AUDUSD", "AUDUSD_OTC", "EURCHF", "EURJPY", "EURRUB", 
                                               "EURUSD", "EURUSD_OTC", "GBPUSD", "GBPUSD_OTC", "USDCAD", "USDCAD_OTC",
                                               "USDCHF", "USDCHF_OTC", "USDJPY", "USDJPY_OTC", "USDRUB", "XAGUSD", "XAUUSD"};

    const std::string directions[2] = {"down", "up"};

    const std::string user_agent = "Mozilla/5.0 (X11; Linux x86_64; rv:44.0) Gecko/20100101 Firefox/44.0"; 
    const std::string accept_text = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
    const std::string accept_language = "Accept-Language: en-US,en;q=0.5";
    const std::string accept_encoding = "gzip, deflate, br";
    const std::string x_requested_with = "X-Requested-With: XMLHttpRequest";
    const std::string content_type = "Content-Type: application/x-www-form-urlencoded";
    const std::string login_referer = "Referer: https://olymptrade.com/";
    const std::string other_query_referer = "Referer: https://olymptrade.com/platform";
    const std::string content_length = "Content-Length:";
    const std::string connection = "Connection: keep-alive";

    const std::string log_in_url_pattern = "https://olymptrade.com/user/login?_=";
    
    const std::string update_url_pattern = "https://olymptrade.com/platform/data?source=platform&_=";
    
    const std::string switch_to_demo_acc = "https://olymptrade.com/user/playdemo?_=";

    const std::string switch_to_real_acc = "https://olymptrade.com/user/playreal?_=";

    const std::string log_out_url = "https://olymptrade.com/user/logout";
    const std::string upgrade_insec_request = "Upgrade-Insecure-Requests: 1";
    
    const std::string bet_url_pattern_part_one = "https://olymptrade.com/deal/open?amount=";
    const std::string bet_url_pattern_part_two = "&duration=";
    const std::string bet_url_pattern_part_three = "&dir=";
    const std::string bet_url_pattern_part_four = "&pair=";
    const std::string bet_url_pattern_part_five = "&source=platform&timestamp=";

    //функция конвертации string char в url string (%XX)
    std::string url_encode(CURL* curl, const std::string& text);

    void set_host_url(CURL* curl_handle, int request_type, deals_namespace::NewBet* bet_props);

    void init_bet_handle();

    void init_logout_handle();

    void init_status_handle();

    void construct_request_headers(CURL* curl_handle, struct curl_slist** headers, int request_type);

    //function adding current system time to the string in format [sec.mili_sec]
    void add_time_to_string(std::string& str_time_to_add, int request_type);

  public:

    CurlOlymptradeActions();
 
    ~CurlOlymptradeActions();

    void log_into_platform(std::string login, std::string password, size_t responce_data_process(char *, size_t, size_t, void*), OlyClientDealService* deals_service);

    void send_requests(std::vector<deals_namespace::NewBet> bet_props); 

    void switch_to_demo();

    void switch_to_real();   
};