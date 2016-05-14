#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <assert.h>
#include <stdlib.h>
#include <string.h>


class AssetNames
{
  private:
  	int asset_amount;

    std::vector<std::string> asset_names;

    void delete_bracket(const char* str_to_clean);

  public:
  	
  	int get_assets_amount();

  	void load_asset_names(std::string* asset_file_name);

   	const std::string* get_asset_name(int asset_num);

  	AssetNames();
};