#pragma once
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <inttypes.h>
#include <sys/types.h>
#include "SysStructs.h"
#include <libwebsockets.h>
#include "json11/json11.hpp"
#include "StatisticsFileSystem.h"
#include "EstablishConnection.h"
  
struct Record 
{
  uint64_t timestamp;
  double close;
};  

class OlymptradeWsClient
{
	private:

    int queue_connection_eastablished;
    pid_t ws_command_queue_fd;
    int queue_fd;

    int connection_flag;
    int current_asset_number;
    std::string current_statistics_file_pathname;
    const std::string* current_asset_name;
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
    void open_queue_connection();
    void close_queue_connection();
    void send_asset_closed_msg();

    friend int ws_service_callback(struct lws* wsi,
                           enum lws_callback_reasons reason, 
                           void *user, void *in, size_t len);

    friend void pthread_routine(void *tool_in, struct lws* wsi_pointer);

	public:

		OlymptradeWsClient();
		int run_client(std::string current_records_filename, int current_asset_num, const std::string* asset_name, pid_t main_queue_fd);
		void close_connection();
 };
