#include "AssetNames.h"

void AssetNames::load_asset_names(std::string* asset_file_name)
{
  char current_asset_name[20] = {};

  FILE* asset_file = fopen(asset_file_name -> c_str(), "r");
  assert(asset_file);

  std::string new_asset_name;

  while(!feof(asset_file))
  {
    fscanf(asset_file, "[[Record_name:%s]", current_asset_name);
    delete_bracket(current_asset_name);
   
    new_asset_name.clear();

    new_asset_name.assign(current_asset_name, strlen(current_asset_name));
   
    asset_names.push_back(new_asset_name);

    asset_amount++;

    fscanf(asset_file, "%*[^\n]\n", NULL);
  }
}

void AssetNames::delete_bracket(const char* str_to_clean)
{
  if(!str_to_clean)
  {
    printf("[Delete bracket]:Invalid string to clean pointer\n");
    return;
  }
  int str_length = strlen(str_to_clean);

  if(((char*)str_to_clean)[str_length - 2] != ']')
    ((char*)str_to_clean)[str_length - 1] = '\0';
  else
    ((char*)str_to_clean)[str_length - 2] = '\0';
}

const std::string* AssetNames::get_asset_name(int asset_num)
{
  return &asset_names[asset_num];
}

int AssetNames::get_assets_amount()
{
  return asset_amount;
}

AssetNames::AssetNames()
{
  asset_amount = 0;
}
