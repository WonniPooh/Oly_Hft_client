#pragma once
#include <getopt.h>
#include <signal.h>
#include <string>
#include <cstdlib>
#include <stdio.h>

namespace wsclient
{
  class ParseCmdArgs 
  {
    private:

      int session_record;
      std::string record_name;

      int query_existance;
      std::string first_query;
      
      int load_all;
      int log_level;
      int port_used;
      int load_data;
      int ietf_version;
      std::string server_info;
      std::string connection_origin;

     void print_usage();

    public:

      ParseCmdArgs();

      ParseCmdArgs(int argc, char** argv);

      int load_all_data();
      int get_port_used();
      int get_load_session();
      int get_query_existance();
      int get_session_record(); 
      char* get_char_first_query();
      char* get_char_server_info();
      char* get_char_record_name();

      std::string get_connection_origin();
      std::string get_first_query();
      std::string get_server_info();
      std::string get_record_name();
  };
}