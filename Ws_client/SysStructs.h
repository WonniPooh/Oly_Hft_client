//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once
#include <string>
#include <time.h>
#include <inttypes.h>

static const std::string RECORDS_FILENAME                 = "/Creation_data";
static const std::string bets_result_status[2]            = {"loose", "win"};          
static const std::string asset_names_filename             = "/all_assets.txt";          
static const std::string status_queue_filename            = "NEURO_ASSETS_STATUS";      
static const std::string ws_queue_filename                = "/WS_CLIENT_ASSET_STATUS"; 

static const int WS_CLIENT_COMMANDER_PING_MTYPE           = 90;
static const int ASSET_CLOSED_MTYPE                       = 80;                   

static const int MAX_DEAL_AMOUNT                          = 10;                   
static const int MAX_USERNAME_LENGTH                      = 200;                   

static const int DEAL_SERVICE_PING_MTYPE                  = 20;                   
static const int DEAL_SERVICE_RESPONCE_PING_MTYPE         = 21;

static const int WSCLIENT_QUOTE_MTYPE                     = 70;                   
static const int WSCLIENT_PING_MSG_RESPONSE               = 71;                   

static const int NEW_DEAL_MTYPE                           = 30;
static const int DEAL_STATUS_MTYPE                        = 31;
static const int DEAL_RESULTS_MTYPE                       = 32;

static const int WINPERC_COMMANDER_STATUS_CHANGED_MTYPE   = 40;                   
static const int WINPERC_COMMANDER_WINPERC_CHANGED_MTYPE  = 50;                   
static const int WINPERC_COMMANDER_PING_RESPONSE_MTYPE    = 60;                   

namespace deal_structs
{ 
  struct NewDeal
  {
    long mtype;
    int bet_id;
    int asset;
    bool direction;
    int deal_amount;
    time_t timeframe;
  };

  struct DealStatus
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
}

namespace ping_structs
{
  struct PingConnection         
  {
    long mtype;
    int ping_data;              
  };

  struct PingResult             
	{
	    long mtype;
	    bool ready_to_recieve;
	  };
	}

namespace status_structs 
{
  struct AssetWinperc         
  {
    long mtype;
    uint16_t winperc;
  };

  struct AssetAvailable        
  {
    long mtype;
    int asset;
    bool available;
  };
}

struct MsgBufWsCreateNewConnection
{
  long mtype;
  int action;
  int asset;
};

struct NewQuoteMsg             
{
  long mtype;
  uint64_t timestamp;
  double close;
};

struct ConnectionClosedMsg  
{
  long mtype;
  int asset;
};