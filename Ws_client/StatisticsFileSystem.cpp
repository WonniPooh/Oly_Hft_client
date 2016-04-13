#include "StatisticsFileSystem.h"

void wsclient::StatisticsFileSystem::update_date()
{
  the_time = time(NULL);
  current_date = localtime(&the_time);
}

void wsclient::StatisticsFileSystem::get_username()
{
  char char_username[40] = {};
  getlogin_r(char_username, 40);
  username.assign(char_username, strlen(char_username));
}

int wsclient::StatisticsFileSystem::directory_exists(const char* path)
{
  if ( path == NULL) 
    return -1;

  DIR *current_dir = opendir (path);
  int dir_exists = 0;

  if (current_dir != NULL)
  {
      dir_exists = 1;    
      (void) closedir (current_dir);
  }

  return dir_exists;
}

void wsclient::StatisticsFileSystem::configure_asset_name()
{
  current_asset =  current_asset_const + std::to_string(asset_number);
}

void wsclient::StatisticsFileSystem::configure_global_adress()
{
  global_adress = home_path + "/" + username + "/" + assets_folder + "/" + current_asset + "/" + years_folder + "/" 
  + current_year + "/" + months_folder + "/" + current_month + "/" + days_folder + "/" + current_filename_to_use;
}

void wsclient::StatisticsFileSystem::configure_data()
{
  current_year = current_year_const + std::to_string(current_date -> tm_year + 1900);

  current_month = current_month_const + std::to_string(current_date -> tm_mon + 1);

  current_filename_to_use = current_asset_const + std::to_string(asset_number) + current_year_const + std::to_string(current_date -> tm_year + 1900) 
  + current_month_const + std::to_string(current_date -> tm_mon + 1) + "D" + std::to_string(current_date -> tm_mday) + ".txt"; 
}

void wsclient::StatisticsFileSystem::generate_file_path()
{
  chdir(home_path.c_str());
  chdir(username.c_str());

  FOLDER_ACTION(assets_folder);
  FOLDER_ACTION(current_asset);
  FOLDER_ACTION(years_folder);
  FOLDER_ACTION(current_year);
  FOLDER_ACTION(months_folder);
  FOLDER_ACTION(current_month);
  FOLDER_ACTION(days_folder);
}  

wsclient::StatisticsFileSystem::StatisticsFileSystem()
{
  prev_sec_hour = 0;
  asset_number = 0;
  the_time = {};
  current_date = NULL;
}

wsclient::StatisticsFileSystem::StatisticsFileSystem(int asset)       //вызов конструкторов по умолчанию  int&* --  ссылка на указатель 
{  
  home_path = ("/home");
  assets_folder = ("Assets");
  current_asset_const = "A";
  years_folder = "Years";
  current_year_const = "Y";
  months_folder = "Month";
  current_month_const = "M";
  days_folder = "Days";

  asset_number = asset;
  
  configure_asset_name();
  get_username();
  update_date();
  configure_data();
  generate_file_path();

  configure_global_adress();
  prev_sec_hour = 0;
}

int wsclient::StatisticsFileSystem::construct_statistics(int asset)
{
  home_path = "/home";
  assets_folder ="Assets";
  current_asset_const = "A";
  years_folder = "Years";
  current_year_const = "Y";
  months_folder = "Month";
  current_month_const = "M";
  days_folder = "Days";

  asset_number = asset;
  configure_asset_name();
  get_username();
  update_date(); 
  configure_data();
  generate_file_path();

  configure_global_adress();
  prev_sec_hour = 0;
}

int wsclient::StatisticsFileSystem::update_time()
{
  update_date();

  if((current_date -> tm_hour - prev_sec_hour) < 0)
  {
    configure_data();
    generate_file_path();
    configure_global_adress();

    return 1;
  }
  
  prev_sec_hour = current_date -> tm_hour;

  return 0;
}

std::string wsclient::StatisticsFileSystem::get_current_filepath_to_use()
{
  return global_adress;
}
