#include "ParseCmdArgs.h"

static struct option options[] = 
{
  { "all",     no_argument,         NULL, 'a' },
  { "help",    no_argument,         NULL, 'h' },
  { "debug",   required_argument,   NULL, 'd' },
  { "port",    required_argument,   NULL, 'p' },
  { "version", required_argument,   NULL, 'v' },
  { "record",  required_argument,   NULL, 'r' },
  { "load",    required_argument,   NULL, 'l' },
  { "query",   required_argument,   NULL, 'q' },
  { "origin",  required_argument,   NULL, 'o' },
  { NULL, 0, 0, 0 }
};

void wsclient::ParseCmdArgs::print_usage()
{
  fprintf(stderr, "Usage: libwebsockets-client(program name)\n"
      "<server address>(ws://echo.websocket.org)\n" 
      "[--port=<p>] [--origin=<o>](localhost) [--all=<a>]\n"
      "[--record=<r>(name)] [--query=<q>](\"my_query\")\n"
      "[-v <ver> (ietf_version)] [-d <log bitfield>]\n");
}

wsclient::ParseCmdArgs::ParseCmdArgs()
{
  load_all = 0;
  log_level = 0;
  port_used = 0;
  load_data = 0;
  ietf_version = -1;
  session_record = 0;
  query_existance = 0;
}

wsclient::ParseCmdArgs::ParseCmdArgs(int argc, char** argv)
{

  load_all = 0;
  log_level = 0;
  port_used = 0;
  load_data = 0;
  ietf_version = -1;
  session_record = 0;
  query_existance = 0;

  int current_param = 0;

  if (argc < 2 || NULL == argv)
  {
    print_usage();
    exit(0);
  }

  server_info = argv[1];

  while (current_param >= 0) 
  {
    current_param = getopt_long(argc, argv, "hp:d:v:o:r:l:q:a", options, NULL);

    if (current_param < 0)
      continue;
    switch (current_param) 
    {
      case 'a':
        load_all = 1;
        break;
      case 'd':
        log_level = atoi(optarg);
        break;
      case 'p':
        port_used = atoi(optarg);
        break;
      case 'v':
        ietf_version = atoi(optarg);
        break; 
      case 'o':
        connection_origin = optarg;
        break; 
      case 'r':
        record_name = optarg;
        session_record = 1;
        break;
      case 'q':
        first_query = optarg;
        query_existance = 1;
        break;
      case 'l':
        record_name = optarg;
        load_data = 1;
        break;  
      case 'h':
        kill(0, SIGINT);
    }
  }
}

int wsclient::ParseCmdArgs::get_load_session()
{
  return load_data;
}

int wsclient::ParseCmdArgs::get_port_used()
{
  return port_used;
} 

int wsclient::ParseCmdArgs::get_query_existance()
{
  return query_existance;
}

char* wsclient::ParseCmdArgs::get_char_first_query()
{
  return (char*)first_query.c_str();
}

char* wsclient::ParseCmdArgs::get_char_server_info()
{
  return (char*)server_info.c_str();
}

char* wsclient::ParseCmdArgs::get_char_record_name()
{
  return (char*)record_name.c_str();
}

std::string wsclient::ParseCmdArgs::get_first_query()
{
  return first_query;
}

std::string wsclient::ParseCmdArgs::get_connection_origin()
{
  return connection_origin;
}

std::string wsclient::ParseCmdArgs::get_server_info()
{
  return server_info;
}

std::string wsclient::ParseCmdArgs::get_record_name()
{
  return record_name;
}

int wsclient::ParseCmdArgs::get_session_record()
{
  return session_record;
}

int wsclient::ParseCmdArgs::load_all_data()
{
  return load_all;
}