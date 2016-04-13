#pragma once 
#include <unistd.h> 
#include <time.h>
#include <dirent.h> 
#include <unistd.h>
#include <string>
#include <string.h>
#include <sys/stat.h> 

#ifndef FOLDER_ACTION
#define FOLDER_ACTION(str)      if(!directory_exists(str.c_str()))\
                                { \
                                  mkdir(str.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);\
                                }\
                                chdir(str.c_str());
#endif

namespace wsclient
{
  class StatisticsFileSystem
  {
    private:

      int prev_sec_hour;
      int asset_number;
      time_t the_time;
      struct tm* current_date;

      std::string home_path;                                      //use prefics that define that variable belongs to class home_path_ <- cool at the moment
      std::string username;
      std::string assets_folder;
      std::string current_asset_const;                            //microsofte use m_ <- member of, QT like my_
      std::string years_folder;
      std::string current_year_const;
      std::string months_folder;
      std::string current_month_const;
      std::string days_folder;

      std::string global_adress;
      std::string current_asset;
      std::string current_year;
      std::string current_month;
      std::string current_filename_to_use;

      void update_date();

      void get_username();

      void configure_data();

      void generate_file_path();

      void configure_asset_name();

      void configure_global_adress();

      int directory_exists( const char* path);

    public:

      StatisticsFileSystem();

      explicit StatisticsFileSystem(int asset);                            //СОБСТВЕННЫЙ ОПЕРАТОР ПРЕОБРАЗОВАНИЯ ТИПА (НЕЯВНАЯ СМЕНА ТИПА) implicit type conver. operator

      int construct_statistics(int asset);
      
      std::string get_current_filepath_to_use();

      int update_time();
  };
}