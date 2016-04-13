#include "OlymptradeWsClient.h"

const int MAX_ASSET_NAME_LEN = 10;

int ws_service_callback(struct lws* wsi_pointer,
                           enum lws_callback_reasons reason, 
                           void *user, void *in, size_t len)
{
  OlymptradeWsClient* current_client = (OlymptradeWsClient*)user;

  switch (reason) 
  {
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
        
      if(current_client -> reconnection_attempt_num)
      {
        printf("[Main Service] Reconnection success.\n");
        current_client -> reconnection_attempt_num = 0;
      }
      else
        printf("[Main Service] Connection with server established.\n");
      
      current_client -> connection_flag = 1;
      break;

    case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
        
      printf("[Main Service] Connect with server error.\n");

      current_client -> connection_flag = 0;
      
      if(current_client -> reconnection_attempt_num < MAX_RECONNECT_ATTEMPTS_NUM)
      {
        printf("[Main Service] Attempt to reconnect %d...\n", current_client -> reconnection_attempt_num++ + 1);
        current_client -> reconnect();
        current_client -> close_connection(0);
      }
      else
      {
        current_client -> close_connection(AUTHORIZED_CONTEXT_CLOSE);
      }

      break;

    case LWS_CALLBACK_CLOSED:                                               //end of websocket session
      
      printf("[Main Service] LWS_CALLBACK_CLOSED\n");

      current_client -> connection_flag = 0;
      
      if(current_client -> reconnection_attempt_num < MAX_RECONNECT_ATTEMPTS_NUM && !current_client -> authorized_context_close_flag) 
      {
        printf("[Main Service] Attempt to reconnect %d...\n", current_client -> reconnection_attempt_num++ + 1);
        current_client -> reconnect();
        current_client -> close_connection(0);
      }
      else
      {
        current_client -> close_connection(AUTHORIZED_CONTEXT_CLOSE);
      }
      break;

    case LWS_CALLBACK_CLIENT_RECEIVE:
      if(current_client -> current_asset_statistics.update_time())
      {
        fclose(current_client -> stat_file);
        current_client -> current_statistics_file_pathname = current_client -> current_asset_statistics.get_current_filepath_to_use();
        current_client -> stat_file = fopen(current_client -> current_statistics_file_pathname.c_str(), "a+");
      }

      if(strlen((char*)in) < MAX_SERVER_RESPOND_LENGTH)
      {
        current_client -> record_current_second_data(in);
      } 
      
      break;

    default:
      
      break;
  }

  return 0;
}

void pthread_routine(void *tool_in, struct lws *wsi_pointer)
{
  if(!tool_in || !wsi_pointer)
  {
    printf("Pthread_routine: Invalid tool_in or wsi pinter");
    kill(0, SIGINT);
  }
 
  char server_request[MAX_SERVER_REQUEST_LEN] = {};

  PthreadRoutineStruct *tool = (PthreadRoutineStruct *)tool_in;

  OlymptradeWsClient* current_client = (OlymptradeWsClient*)tool -> olymptrade_pointer;

  printf("[pthread_routine] Greetings. This is pthread_routine.\n");

  //* waiting for connection with server done.*/
  while(!current_client -> connection_flag)
      usleep(1000*20);

  printf("[pthread_routine] Server is ready to recieve messages send.\n");

  if(tool -> is_there_first_request)
  {
    current_client -> websocket_write_back(wsi_pointer, tool -> first_server_request);
  
    lws_callback_on_writable(wsi_pointer);
  }

  /*
  while(1)
  {    
    fgets(server_request, MAX_SERVER_REQUEST_LEN, stdin);

    websocket_write_back(wsi, server_request);
  
    lws_callback_on_writable(wsi);
  }
  */
}

void OlymptradeWsClient::open_queue_connection()
{ 
  key_t key; 

  FILE* temp = fopen(queue_file_pathname.c_str(), "a+");

  fclose(temp);

  if ((key = ftok(queue_file_pathname.c_str(), 0)) < 0)
  {
    printf("Can\'t generate key\n");
    return;
  }

  if((queue_fd = msgget(key, 0666 | IPC_CREAT)) < 0)
  {
    printf("Can\'t get msgid\n");
    return;
  }

  PingConnection ping = {};
  ping.mtype = 1;
  ping.accept_msgtype = getpid();
  strncpy(ping.asset_name, current_asset_name -> c_str(), MAX_ASSET_NAME_LEN);

  if(msgsnd(queue_fd, (PingConnection*) &ping, sizeof(PingConnection) - sizeof(long), 0) < 0)
  {
    printf("Can\'t send message to queue\n");
  }
}

void OlymptradeWsClient::transmit_data(uint64_t current_timestamp, double price_close)
{
  if(queue_connection_eastablished)
  {
    SendData post_data = {};
    post_data.mtype = getpid();
    post_data.timestamp = current_timestamp;
    post_data.close = price_close;

    if(msgsnd(queue_fd, (SendData*) &post_data, sizeof(SendData) - sizeof(long), 0) < 0)
    {
      printf("process %d:: Can\'t send message to queue\n", getpid());
    }
  }
  else
  {
    PingSuccess test_success = {};

    int rcv_result = msgrcv(queue_fd, (PingSuccess*) &test_success, sizeof(test_success) - sizeof(long), getpid(), IPC_NOWAIT);

    if (rcv_result < 0)
    {
      perror("msgrcv");
    }
    else
    {
      if(!strcmp(test_success.asset_name, current_asset_name -> c_str()))
      {
        queue_connection_eastablished = 1;
      }
    }
  }
}

void OlymptradeWsClient::close_queue_connection()
{
  queue_fd = 0;
  /*
  if(msgctl(queue_fd, IPC_RMID, (struct msqid_ds*)NULL) < 0)
  {
    printf("process:: %d; Can't delete queue\n", getpid());
  }
  */
}

void OlymptradeWsClient::record_current_second_data(void* in_str)
{
  /*
  {
    static struct timespec last_ts = {0};
    struct timespec cur_ts;
    clock_gettime(CLOCK_REALTIME, &cur_ts);

    double delta = cur_ts.tv_sec + cur_ts.tv_nsec/1e9 - (last_ts.tv_sec + last_ts.tv_nsec/1e9);
    last_ts = cur_ts;

    if (delta < 2)
      printf("record_current_second_data: pid %d delta %f data '%s'\n", getpid(), delta, (char*)in);
    else
      printf("record_current_second_data: pid %d WARNING delta %f data '%s'\n", getpid(), delta, (char*)in);
  }
  */
  
  std::string in_error;
  json11::Json in_json = json11::Json::parse((const char *)in_str, in_error);
  
  if (!in_error.empty()) 
  {
    printf("[pid %d] json '%s' parse error '%s', skipping\n", getpid(), in_str, in_error.c_str());
    return;
  }

  assert(in_json.is_object());
  auto in = in_json.object_items();

  if(in.count("time")) 
  {
    printf("[pid %d] received data: '%s'\n", getpid(), in_str);

    assert(in["time"].is_number());
    assert(in.count("close") && in["close"].is_number());

    Record r;
    r.timestamp = in["time"].number_value();
    r.close = in["close"].number_value();

    assert(last.timestamp < r.timestamp);

    transmit_data(r.timestamp, r.close);

    if (last.timestamp != 0) {
      for (uint64_t t = last.timestamp + 1; t <= r.timestamp; ++t) {
        fprintf(stat_file, "%llu %f\n", t, last.close);
      }
      fflush(stat_file);
    }

    last = r;
  } else {
    printf("[pid %d] received non-data: '%s'\n", getpid(), in_str);

    assert(in.count("servertime") && in["servertime"].is_number());
    uint64_t timestamp = in["servertime"].number_value();

    /* never print _current_ timestamp -- it may get superseded by next "full" data
     * if the server goes mad */
    if (last.timestamp != 0) {
      for (uint64_t t = last.timestamp + 1; t < timestamp; ++t) {
        fprintf(stat_file, "%llu %f\n", t, last.close);
        last.timestamp = t;
      }
      fflush(stat_file);
    }
  }
}

OlymptradeWsClient::OlymptradeWsClient()
{
  queue_file_pathname = "/usr/tmp/";
  queue_connection_eastablished = 0;
  queue_fd = 0;
  authorized_context_close_flag = 0;
  reconnection_attempt_num = 0;
  connection_flag  = 0;
  stat_file = NULL;
  last = {};
}

int OlymptradeWsClient::run_client(std::string current_records_filename, int current_asset_num, std::string* asset_name)
{
  records_filename = current_records_filename;
  current_asset_name = asset_name;
  queue_file_pathname += *asset_name;

  current_asset_statistics.construct_statistics(current_asset_num);
  current_statistics_file_pathname = current_asset_statistics.get_current_filepath_to_use();
  
  stat_file = fopen(current_statistics_file_pathname.c_str(), "a+");

  open_queue_connection();

  if(!stat_file)
  {
    printf("Error opening statistics file\n");
    exit(0);
  }

  if(asset_name -> length())
  {
    con_data.LoadSession(asset_name -> c_str(), records_filename.c_str());
  }
  else
    return -1;

  struct PthreadRoutineStruct tool;
    tool.is_there_first_request = con_data.is_there_query();

  if(tool.is_there_first_request);
    con_data.get_first_query(tool.first_server_request);

  tool.olymptrade_pointer = this;

  current_ws_connection.connect(ws_service_callback, con_data, (void*)&tool, pthread_routine, this);

  fclose(stat_file);
}

int OlymptradeWsClient::websocket_write_back(struct lws *wsi_in, char *str_data_to_send) 
{
  if(!wsi_in || !str_data_to_send)
  {
    printf("Websocket_write_back: Invalid wsi_in or str_data_to_send pointer.\n");
    return -1;
  }

  int bytes_amount_written = 0;
  int string_length = strlen(str_data_to_send);
  char *str_to_send_out = NULL;

  str_to_send_out = (char*)malloc((LWS_SEND_BUFFER_PRE_PADDING + string_length + LWS_SEND_BUFFER_POST_PADDING) * sizeof(char));
  //* setup the buffer*/
  memcpy (str_to_send_out + LWS_SEND_BUFFER_PRE_PADDING, str_data_to_send, string_length);
  //* write out*/
  bytes_amount_written = lws_write(wsi_in, (unsigned char*)str_to_send_out + LWS_SEND_BUFFER_PRE_PADDING, string_length, LWS_WRITE_TEXT);

  printf("[websocket_write_back] %s\n", str_data_to_send);
  //* free the buffer*/
  free(str_to_send_out);

  return bytes_amount_written;
}

void OlymptradeWsClient::reconnect()
{
  current_ws_connection.try_to_reconnect(); 
}

void OlymptradeWsClient::close_connection(int parameter)
{ 
  if(connection_flag)
  {     
    if(parameter == AUTHORIZED_CONTEXT_CLOSE)
    {
      authorized_context_close_flag = 1;
      close_queue_connection();
    }

    current_ws_connection.close_connection();
  }
  else
    printf("OlymptradeWsClient:: No connection to close\n");
}