#include "CurlOlymptradeActions.h"

using std::exit;
using std::ostream;
using std::runtime_error;

#define SSTR( x ) static_cast < std::ostringstream & >( ( std::ostringstream() << std::dec << x ) ).str()

std::string CurlOlymptradeActions::url_encode(CURL* curl, const std::string& text)
{
  assert(curl);
  
  std::string result;
  
  char* esc_text = curl_easy_escape(curl, text.c_str(), text.length());
  
  if(!esc_text)throw runtime_error("Can not convert string to URL\n");

  result = esc_text;
   
  curl_free(esc_text);

  return result;
}

void CurlOlymptradeActions::set_host_url(CURL* curl_handle, int request_type, deal_structs::NewDeal* bet_props)
{
  assert(curl_handle);
  
  std::string current_url_configuration;  
  
  if(UPDATE_STATUS == request_type)
    current_url_configuration = update_url_pattern;
  else if(LOG_IN == request_type)
    current_url_configuration = log_in_url_pattern;
  else if(PLAY_DEMO == request_type)
  {
    current_url_configuration = switch_to_demo_acc;  
  }
  else if(PLAY_REAL == request_type)
  {
    current_url_configuration = switch_to_real_acc;  
  }
  else if(MAKE_A_BET == request_type)
  {
    current_url_configuration = bet_url_pattern_part_one + SSTR(bet_props -> deal_amount) + bet_url_pattern_part_two 
    + SSTR(bet_props -> timeframe) + bet_url_pattern_part_three + directions[bet_props -> direction] + bet_url_pattern_part_four 
    + *(names.get_asset_name(bet_props -> asset)) + bet_url_pattern_part_five;
  }
  else if(LOG_OUT == request_type)
  {
    current_url_configuration = log_out_url;
  }

  if(LOG_OUT != request_type)
    add_time_to_string(current_url_configuration, request_type);

  curl_easy_setopt(curl_handle, CURLOPT_URL, current_url_configuration.c_str());
}

void CurlOlymptradeActions::init_bet_handle()
{
  construct_request_headers(NULL, &bet_headers, MAKE_A_BET);

  for(int i = 0; i < MAX_DEAL_AMOUNT; i++)
  {
    curl_bet_handle[i] = curl_easy_init();
    curl_easy_setopt(curl_bet_handle[i], CURLOPT_HTTPGET, 1L); 
    curl_easy_setopt(curl_bet_handle[i], CURLOPT_VERBOSE, 1L);                           //print all debug info
    construct_request_headers(curl_bet_handle[i], NULL, MAKE_A_BET);
    curl_easy_setopt(curl_bet_handle[i], CURLOPT_HTTPHEADER, bet_headers);

    //разрешаем использовать cookie + загружаем их из файла 
    curl_easy_setopt(curl_bet_handle[i], CURLOPT_COOKIEFILE, "mycookiefile");

    curl_easy_setopt(curl_bet_handle[i], CURLOPT_WRITEFUNCTION, responce_data_process_pointer);
    curl_easy_setopt(curl_bet_handle[i], CURLOPT_WRITEDATA, NULL);          //TODO USER DATA SEND
  }
}

void CurlOlymptradeActions::init_logout_handle()
{
  curl_login_handle = curl_easy_init();
  curl_easy_setopt(curl_login_handle, CURLOPT_HTTPGET, 1L); 
  curl_easy_setopt(curl_login_handle, CURLOPT_VERBOSE, 1L);                           //print all debug info
  construct_request_headers(curl_login_handle, &logout_headers, LOG_OUT);
  curl_easy_setopt(curl_login_handle, CURLOPT_HTTPHEADER, logout_headers);

  //разрешаем использовать cookie + загружаем их из файла 
  curl_easy_setopt(curl_login_handle, CURLOPT_COOKIEFILE, "mycookiefile");
}

void CurlOlymptradeActions::init_status_handle()
{
  curl_status_handle = curl_easy_init();
  curl_easy_setopt(curl_status_handle, CURLOPT_HTTPGET, 1L); 
  curl_easy_setopt(curl_status_handle, CURLOPT_VERBOSE, 1L);                           //print all debug info
  construct_request_headers(curl_status_handle, &status_update_headers, UPDATE_STATUS);
  curl_easy_setopt(curl_status_handle, CURLOPT_HTTPHEADER, status_update_headers);

  //разрешаем использовать cookie + загружаем их из файла 
  curl_easy_setopt(curl_status_handle, CURLOPT_COOKIEFILE, "mycookiefile");

  curl_easy_setopt(curl_status_handle, CURLOPT_WRITEFUNCTION, responce_data_process_pointer);
  curl_easy_setopt(curl_status_handle, CURLOPT_WRITEDATA, NULL);
}

void CurlOlymptradeActions::construct_request_headers(CURL* curl_handle, struct curl_slist** headers, int request_type)
{
  if(curl_handle) 
  {       
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, user_agent.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_ACCEPT_ENCODING, accept_encoding.c_str()); 
  }
  
  if(headers)
  {
    *headers = curl_slist_append(*headers, accept_text.c_str());
    *headers = curl_slist_append(*headers, accept_language.c_str());
    *headers = curl_slist_append(*headers, x_requested_with.c_str());
    
    if(MAKE_A_BET != request_type)
      *headers = curl_slist_append(*headers, content_type.c_str());
    
    if(LOG_OUT == request_type)
      *headers = curl_slist_append(*headers, upgrade_insec_request.c_str());

    if(LOG_IN == request_type)
    {
      *headers = curl_slist_append(*headers, login_referer.c_str());
      *headers = curl_slist_append(*headers, (content_length + std::to_string(data_to_post.length())).c_str());
    }
    else
      *headers = curl_slist_append(*headers, other_query_referer.c_str());
    
    *headers = curl_slist_append(*headers, connection.c_str());
  }
}

void CurlOlymptradeActions::add_time_to_string(std::string& str_time_to_add, int request_type)
{
  struct timeval time_val;
  
  gettimeofday(&time_val, NULL);
  
  std::stringstream mystream;

  if(MAKE_A_BET != request_type)
    mystream << time_val.tv_sec << "." << (time_val.tv_usec / 1000);
  else
    mystream << time_val.tv_sec << (time_val.tv_usec / 1000);
  
  str_time_to_add += mystream.str();
}

CurlOlymptradeActions::CurlOlymptradeActions()
{
  for(int i = 0; i < MAX_DEAL_AMOUNT; i++)
    curl_bet_handle[i] = NULL;

  curl_status_handle = NULL;
  curl_login_handle = NULL;

  initialized_multi_handle = 0;
  log_in_success = 0;
  bet_headers = NULL;
  login_headers = NULL;
  logout_headers = NULL;
  status_update_headers = NULL;

  char current_username[/*MAX_USERNAME_LENGTH*/200] = {};
  getlogin_r(current_username, 200);

  std::string assets_filepath = std::string("/home/") + std::string(current_username) + asset_names_filename;
  names.load_asset_names(&assets_filepath);
}

CurlOlymptradeActions::~CurlOlymptradeActions()
{
  if(log_in_success)
  {
    init_logout_handle();

    set_host_url(curl_login_handle, LOG_OUT, NULL);
               
    CURLcode res = curl_easy_perform(curl_login_handle); 

    remove("mycookiefile");
  }
}

void CurlOlymptradeActions::log_into_platform(std::string login, std::string password, size_t responce_data_process(char *, size_t, size_t, void*), OlyClientDealService* deals_service)
{
  assert(responce_data_process);
  responce_data_process_pointer = responce_data_process;

  curl_login_handle = curl_easy_init();
  user_login = login;
  user_password = password;

  //формирование запроса на основе пользовательских данных

  data_to_post += "email=" + url_encode(curl_login_handle, login);
  data_to_post += "&password=" + url_encode(curl_login_handle, password);
  data_to_post += "&remember=false";
  
  std::cout << data_to_post << std::endl;
  
  set_host_url(curl_login_handle, LOG_IN, NULL);
  
  curl_easy_setopt(curl_login_handle, CURLOPT_VERBOSE, 1L);
  
  construct_request_headers(curl_login_handle, &login_headers, LOG_IN);
   
  curl_easy_setopt(curl_login_handle, CURLOPT_HTTPHEADER, login_headers);
   
  //разрешаем использовать cookie (разрешает куки использовать в текущем curl)
  curl_easy_setopt(curl_login_handle, CURLOPT_COOKIEJAR, "mycookiefile");
               
  curl_easy_setopt(curl_login_handle, CURLOPT_WRITEFUNCTION, responce_data_process_pointer);
  curl_easy_setopt(curl_login_handle, CURLOPT_WRITEDATA, deals_service);  //3 arg pass here a pointer to data which you want tot recieve as 4 argument of responce_data_process function

  // POST - запрос с авторизацией (происходит получение кукисов)
  curl_easy_setopt(curl_login_handle, CURLOPT_POSTFIELDS, data_to_post.c_str());
  curl_easy_setopt(curl_login_handle, CURLOPT_POSTFIELDSIZE, data_to_post.length());

  CURLcode res = curl_easy_perform(curl_login_handle); //send the post request
  
  curl_easy_cleanup(curl_login_handle);

  if(!res)
    log_in_success = 1;
  else
  {
    printf("Connection was not established!\n");
    exit(0);
  }
}

void CurlOlymptradeActions::send_requests(std::vector<deal_structs::NewDeal> bet_props) 
{ 
  static int count = 0;
  unsigned int milisec = 1000;

  if(!curl_bet_handle[0])
    init_bet_handle();     

  if(!curl_status_handle)
    init_status_handle();  

  int handle_count = 0;
  int num_bet = bet_props.size();

  if(!count)
  {
    count++;
    multi_handle = curl_multi_init();
  }

  set_host_url(curl_status_handle, UPDATE_STATUS, NULL);
  curl_multi_add_handle(multi_handle, curl_status_handle);

  if(num_bet)
  {
    for(int i = 0; i < num_bet; i++)
    {
      set_host_url(curl_bet_handle[i], MAKE_A_BET, &bet_props[i]);
      curl_multi_add_handle(multi_handle, curl_bet_handle[i]);
    }
  }   

  while(1)
  {
    CURLMcode res = curl_multi_perform(multi_handle, &handle_count); 

    if(handle_count == 0)
      break;

    usleep(10 * milisec);   
  }       

  curl_multi_remove_handle(multi_handle, curl_status_handle);

  if(num_bet)
  {
    for(int i = 0; i < num_bet; i++)
    {
      curl_multi_remove_handle(multi_handle, curl_bet_handle[i]);
    }
  }  
}

void CurlOlymptradeActions::switch_to_demo()
{

  if(!curl_bet_handle[0])
    init_bet_handle();     

  set_host_url(curl_bet_handle[0], PLAY_DEMO, NULL);
               
  CURLcode res = curl_easy_perform(curl_bet_handle[0]);                               
}

void CurlOlymptradeActions::switch_to_real()
{
  set_host_url(curl_bet_handle[0], PLAY_REAL, NULL);
               
  CURLcode res = curl_easy_perform(curl_bet_handle[0]);                                
} 