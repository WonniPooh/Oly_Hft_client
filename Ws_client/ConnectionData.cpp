#include "ConnectionData.h"

static const int MAX_SERVER_REQUEST_LEN = 500;

static const int MAX_SERVER_ADDRESS_LEN = 300;

static const int MAX_SERVER_PATH_LEN    = 300;

static const int MAX_ORIGIN_LEN         = 200;

static const int MAX_RECORD_LEN         = 100;

static const int MAX_PROTOCOL_LEN       = 10;

void wsclient::ConnectionData::data_dump(FILE* dump, const ConnectionData* current_connection)
{
  fprintf(dump, "===================================================================\n\n");
  fprintf(dump, "current connection class adress: %p\n", current_connection);
  fprintf(dump, "connection_protocol: %s\n", connection_protocol.c_str());
  fprintf(dump, "server_address: %s\n", server_address.c_str());
  fprintf(dump, "server_path: %s\n", server_path.c_str());
  fprintf(dump, "connection_origin: %s\n", connection_origin.c_str());
  fprintf(dump, "ietf_version: %d\n", ietf_version);
  fprintf(dump, "port_used: %d\n", port_used);
  fprintf(dump, "ssl_used: %d\n", ssl_used);
  fprintf(dump, "log_level: %d\n", log_level);
  fprintf(dump, "is_there_first_query: %d\n", is_there_first_query);
  fprintf(dump, "first_query: %s\n\n", first_query.c_str());
  fprintf(dump, "===================================================================\n\n");
}

int wsclient::ConnectionData::record_iput_data(char* record_name, char* filename_to_record_in)
{
  if(!record_name || !filename_to_record_in)
  {
    printf("ConnectionData::Invalid record or file name pointer\n");
    return -1;
  }  

  FILE* file_to_record_in = NULL;
  int is_the_name_used = 0;

  if( access( filename_to_record_in, F_OK ) != -1 ) 
  {
    file_to_record_in = fopen(filename_to_record_in, "r+");

    int is_the_name_used = check_name_existance(record_name, file_to_record_in);
  }
  else
  {
    file_to_record_in = fopen(filename_to_record_in, "w");
  
    if(NULL == file_to_record_in)
      return -1;
  }

  if(!is_the_name_used)
  {
    fprintf(file_to_record_in, "[[Record_name:%s] [connection_protocol:%s] [server_address:%s] [first_query:%s] [connection_origin:%s] [server_path:%s] [port_used:%d] [ietf_version:%d] [ssl_used:%d] [log_level:%d]]\n", record_name, connection_protocol.c_str(), server_address.c_str(), first_query.c_str(), connection_origin.c_str(), server_path.c_str(), port_used, ietf_version, ssl_used, log_level);
    fclose(file_to_record_in);
    return 0;
  }
  else
  {
    printf("Current name has already been used! Cannot make a record.\n");
    fclose(file_to_record_in);
    return -1;
  }
}  

int wsclient::ConnectionData::check_name_existance (char* record_name, FILE* file_check_in)
{
  char current_record_name[MAX_RECORD_LEN] = {};

  while(!feof(file_check_in))
  {
    fscanf(file_check_in, "[[Record_name:%s]", current_record_name);
    delete_bracket(current_record_name);
   
    if(!strcmp(record_name, current_record_name))
    {
      return 1;
    }  

    fscanf(file_check_in, "%*[^\n]\n", NULL);
  }

  return 0;
}

int wsclient::ConnectionData::load_record(FILE* file_get_from, char* record_name)
{
  if(!file_get_from || ! record_name)
    return -1;

  int name_exists = check_name_existance(record_name, file_get_from);

  if(name_exists) 
  {
    char first_query_char_array[MAX_SERVER_REQUEST_LEN];
    char connection_origin_char_array[MAX_ORIGIN_LEN];
    char server_path_char_array[MAX_SERVER_PATH_LEN];
    char connection_protocol_char_array[MAX_PROTOCOL_LEN];
    char server_address_char_array[MAX_SERVER_ADDRESS_LEN];

    int opened_brakets = 1;
    int start_parsing = 0;
    int my_counter = 0;
    char temp[10] = {};
    char ch;

    fscanf(file_get_from, " [connection_protocol:%s]", connection_protocol_char_array);
    delete_bracket(connection_protocol_char_array);

    fscanf(file_get_from, " [server_address:%s]", server_address_char_array);
    delete_bracket(server_address_char_array);

    ch = fgetc(file_get_from);
    ch = fgetc(file_get_from);

    while(opened_brakets)
    {
      ch = fgetc(file_get_from);
      
      if(ch == '[')
        opened_brakets++;

      if(ch == ']')
        opened_brakets--;

      if(start_parsing)
      {
        first_query_char_array[my_counter++] = ch;
      }

      if(ch == ':')
        start_parsing = 1;
    }

    my_counter--;

    first_query_char_array[my_counter] = '\0';

    if(strlen(first_query_char_array))
    {
      is_there_first_query = 1;      
    }

    fscanf(file_get_from, " [connection_origin:%s]", connection_origin_char_array);
    delete_bracket(connection_origin_char_array);

    fscanf(file_get_from, " [server_path:%s]", server_path_char_array);
    delete_bracket(server_path_char_array);

    fscanf(file_get_from, " [port_used:%s]", temp);
    delete_bracket(temp);
    port_used = atoi(temp);

    fscanf(file_get_from, " [ietf_version:%s]", temp);
    delete_bracket(temp);
    ietf_version = atoi(temp);

    fscanf(file_get_from, " [ssl_used:%s]", temp);
    delete_bracket(temp);
    ssl_used = atoi(temp);

    fscanf(file_get_from, " [log_level:%s]]", temp);
    delete_bracket(temp);
    log_level = atoi(temp);

    first_query = first_query_char_array;
    connection_origin = connection_origin_char_array;
    server_path = server_path_char_array;
    connection_protocol = connection_protocol_char_array;
    server_address = server_address_char_array;
  }
  else 
  {
    printf("No such name\n");
    return -1;
  }

  return 0;
}

void wsclient::ConnectionData::delete_bracket(const char* str_to_clean)
{
  int str_length = strlen(str_to_clean);

  if(((char*)str_to_clean)[str_length - 2] != ']')
    ((char*)str_to_clean)[str_length - 1] = '\0';
  else
    ((char*)str_to_clean)[str_length - 2] = '\0';
}

void wsclient::ConnectionData::print_usage()
{
  fprintf(stderr, "Usage: libwebsockets-client(programm name)\n"
      "<server address>(ws://echo.websocket.org)\n" 
      "[--port=<p>] [--origin=<o>](localhost)\n"
      "[--record=<r>(name)] [--query=<q>](\"my_query\")\n"
      "[-v <ver> (ietf_version)] [-d <log bitfield>]\n");
}

wsclient::ConnectionData::ConnectionData()
{
  connection_origin = "localhost";
  server_path = "/";
  is_there_first_query = 0;
  ietf_version = -1;
  port_used = 0;
  log_level = 0;
  ssl_used = 0;
}

void wsclient::ConnectionData::load_session (std::string record_to_load, const char* filename_to_open)
{
  if(!filename_to_open)
  {
    printf("Invalid filename to open!\n");
    exit(0);
  }

  FILE* record_filename = fopen(filename_to_open, "r");
  
  if(!record_filename)
  {
    printf("Cannot open file with records.\n");
    exit(0);
  }
  
  if(load_record(record_filename, (char*)record_to_load.c_str()))
  {
    fclose(record_filename);
    exit(0);
  }

  fclose(record_filename);
}

void wsclient::ConnectionData::create_session(wsclient::ParseCmdArgs args_parsed, char* file_to_record_in)
{
  const char *parsed_path_ptr = NULL;
  const char *server_address_char_array = NULL;
  const char *connection_protocol_char_array = NULL;

  port_used = args_parsed.get_port_used();
  
  is_there_first_query = args_parsed.get_query_existance();

  if(is_there_first_query)
  {
    first_query = args_parsed.get_first_query();
  }

  if (lws_parse_uri(args_parsed.get_char_server_info(), &connection_protocol_char_array, &server_address_char_array, &port_used, &parsed_path_ptr))
  {
    print_usage();
    exit(0);
  }

  server_path += parsed_path_ptr;

  if (!strcmp(connection_protocol_char_array, "ws"))
  {
    ssl_used = 0;

    if(!port_used)
      port_used = 80;
  }

  if (!strcmp(connection_protocol_char_array, "wss"))
  {
    ssl_used = 1;

    if(!port_used)
      port_used = 443;
  }

  connection_origin = args_parsed.get_connection_origin();
  connection_protocol = connection_protocol_char_array;
  server_address = server_address_char_array;

  if(args_parsed.get_session_record())
    record_iput_data(args_parsed.get_char_record_name(), file_to_record_in);
}

bool wsclient::ConnectionData::is_there_query()
{
  return is_there_first_query;
}

int wsclient::ConnectionData::get_first_query(std::string* str_to_write)
{
  if(!is_there_first_query || NULL == str_to_write)
    return -1;
  else
    *str_to_write = first_query;
}

int wsclient::ConnectionData::get_first_query(char* str_to_write)
{
  if(!is_there_first_query || NULL == str_to_write)
    return -1;
  else
    strncpy(str_to_write, first_query.c_str(), MAX_SERVER_REQUEST_LEN);
}

int wsclient::ConnectionData::construct_creation_info(struct lws_client_connect_info* client_connect_info, struct lws_context* context, void* userdata_to_use)
{
  client_connect_info -> address = server_address.c_str();
  client_connect_info -> path = server_path.c_str();
  client_connect_info -> port = port_used;
  client_connect_info -> userdata = userdata_to_use;
  client_connect_info -> ietf_version_or_minus_one = ietf_version;
  client_connect_info -> host = client_connect_info -> address;
  client_connect_info -> ssl_connection = ssl_used;
  client_connect_info -> origin = connection_origin.c_str();
  client_connect_info -> context = context;
  client_connect_info -> protocol = "";                             //name of the protocol which will be used for the connection       
}