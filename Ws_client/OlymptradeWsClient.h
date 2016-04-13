#pragma once
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <inttypes.h>
#include <sys/types.h>
#include <libwebsockets.h>
#include "json11/json11.hpp"
#include "StatisticsFileSystem.h"
#include "EstablishConnection.h"

static const int MAX_SERVER_REQUEST_LEN = 500;
static const int MAX_SERVER_RESPOND_LENGTH = 200;
static const int MAX_RECONNECT_ATTEMPTS_NUM = 5;
static const int AUTHORIZED_CONTEXT_CLOSE = 1;

struct PthreadRoutineStruct 
{
  char first_server_request[MAX_SERVER_REQUEST_LEN];
  int is_there_first_request;
  void* olymptrade_pointer;
};

struct Record 
{
  uint64_t timestamp;
  double close;
};	

struct PingConnection
{
  long mtype;
  std::uint16_t asset;
  int accept_msgtype;
};

struct PingSuccess
{
  long mtype;
  std::uint16_t asset;
};

struct SendData
{
  long mtype;
  uint64_t timestamp;
	double close;
};

class OlymptradeWsClient
{
	private:

    int queue_connection_eastablished;
    int queue_fd;

		int authorized_context_close_flag;
		int reconnection_attempt_num;
    int current_asset_number;
		int connection_flag;
		std::string current_statistics_file_pathname;
    std::string* current_asset_name;
    std::string queue_file_pathname;
    std::string records_filename;
		FILE* stat_file;
		Record last;

		wsclient::StatisticsFileSystem current_asset_statistics;
		wsclient::EstablishConnection current_ws_connection;
		wsclient::ConnectionData con_data;

		int websocket_write_back(struct lws *wsi_in, char *str_data_to_send); 
    void transmit_data(uint64_t current_timestamp, double price_close);
    void record_current_second_data(void* in_str);
    void close_queue_connection();
    void open_queue_connection();
		void reconnect();

		friend int ws_service_callback(struct lws* wsi,
                           enum lws_callback_reasons reason, 
                           void *user, void *in, size_t len);

		friend void pthread_routine(void *tool_in, struct lws* wsi_pointer);

	public:

		OlymptradeWsClient();
		int run_client(std::string current_records_filename, int current_asset_num, std::string* asset_name);
		void close_connection(int parameter);
 };
