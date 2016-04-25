#include "NeuroDealActions.h"

void NeuroDealAction::open_deals_queue()
{ 
  key_t key;

  FILE* temp = fopen(deals_queue_file_pathname.c_str(), "a+");

  fclose(temp);

  if ((key = ftok(deals_queue_file_pathname.c_str(), 0)) < 0)
  {
    printf("NeuroDealAction::open_deals_queue::Can\'t generate key\n");
    return;
  }

  if((asset_deals_queue_fd = msgget(key, 0666 | IPC_CREAT)) < 0)
  {
    printf("NeuroDealAction::open_deals_queue::Can\'t get msgid\n");
    return;
  }
}

void NeuroDealAction::close_deals_queue()
{
  asset_deals_queue_fd = 0;
}

int NeuroDealAction::recieve_status()
{
  int while_cycle_counter = 0;
  new_bets_status_amount = 0;

  if(!ping_success)
    get_ping_answer();


  if(ping_success)
  {
    while(1)
    {
      int bet_num = -1;
      deals_namespace::BetStatus bet_status = {};

      int rcv_result = msgrcv(asset_deals_queue_fd, (deals_namespace::BetStatus*) &bet_status, sizeof(bet_status) - sizeof(long), deals_namespace::DEAL_STATUS_MTYPE, IPC_NOWAIT);

      if (rcv_result < 0)
      {
        printf("NeuroDealAction::recieve_status::error recieving status msg\n");
        perror("msgrcv");
        break;
      }
      else
      {
        for(int i = 0; i < 2*deals_namespace::MAX_DEALS_OPEN; i++)
        {
          if(bets_array[i].bet_id == bet_status.bet_id)
          {
            bet_num = i;
            break;
          }
        }

        if(bet_num != -1)
        {
          bets_status_array[bet_num] = bet_status;
          bets_status_recieved_num[new_bets_status_amount] = bet_num;
          new_bets_status_amount++;
          while_cycle_counter++;
        }
        else
          printf("NeuroDealAction::recieve_status::no bet founded for such bet_id:%d\n", bet_status.bet_id);
      }
    }
  }

  return while_cycle_counter;
}

int NeuroDealAction::recieve_results()
{
  int while_cycle_counter = 0;

  new_bets_result_amount = 0;

  if(!ping_success)
    get_ping_answer();

  if(ping_success)
  {
    while(1)
    {
      int bet_num = -1;
      deals_namespace::DealResult deal_result = {};

      int rcv_result = msgrcv(asset_deals_queue_fd, (deals_namespace::DealResult*) &deal_result, sizeof(deal_result) - sizeof(long), deals_namespace::DEAL_RESULTS_MTYPE, IPC_NOWAIT);

      if (rcv_result < 0)
      {
        printf("NeuroDealAction::recieve_results::error recieving result msg\n");
        perror("msgrcv");
        break;
      }
      else
      {
        for(int i = 0; i < 2 * deals_namespace::MAX_DEALS_OPEN; i++)
        {
          if(bets_array[i].bet_id == deal_result.bet_id)
          {
            bet_num = i;
            break;
          }
        }

        if(bet_num != -1)
        {
          bets_results_array[bet_num] = deal_result;
          bets_result_recieved_num[new_bets_result_amount] = bet_num;
          while_cycle_counter++;
          new_bets_result_amount++;
        }
        else
          printf("NeuroDealAction::recieve_results::no bet founded for such bet_id:%d\n", deal_result.bet_id);
      }
    }
  }

  return while_cycle_counter;
}

void NeuroDealAction::ping_connection()
{
  deals_namespace::PingConnection ping = {deals_namespace::PING_CONSTANT, deals_namespace::PING_CONSTANT};

  if(msgsnd(asset_deals_queue_fd, (deals_namespace::PingConnection*) &ping, sizeof(deals_namespace::PingConnection) - sizeof(long), 0) < 0)
  {
    printf("NeuroDealAction::Can\'t send ping message to queue\n");
    perror("msgsnd");
  }
  else
    printf("NeuroDealAction::Ping msg successfully sent\n");
}

void NeuroDealAction::get_ping_answer()
{
  deals_namespace::PingResult ping_result = {};

  int rcv_result = msgrcv(asset_deals_queue_fd, (deals_namespace::PingConnection*) &ping_result, sizeof(ping_result) - sizeof(long), deals_namespace::PING_RESPONSE_CONSTANT, IPC_NOWAIT);

  if (rcv_result < 0)
  {
    printf("NeuroDealAction::get_ping_answer::error recieving ping msg\n");
    perror("msgrcv");
  }
  else
  {
    ping_success = 1;
    printf("NeuroDealAction::get_ping_answer::ping msg successfully recieved\n");
  }
} 

NeuroDealAction::NeuroDealAction()
{
  for(int i = 0; i < 2 * deals_namespace::MAX_DEALS_OPEN; i++)
  {
    bets_array[i] = {};
    bets_status_array[i] = {};
    bets_results_array[i] = {};
    bets_status_recieved_num[i] = 0;
    bets_result_recieved_num[i] = 0;
  }

  ping_success = 0;
  open_deals_amount = 0;
  free_array_position = 0;
  asset_deals_queue_fd = 0;
  new_bets_status_amount = 0;
  new_bets_result_amount = 0;

  char current_username[deals_namespace::MAX_USERNAME_LENGTH] = {};
  getlogin_r(current_username, deals_namespace::MAX_USERNAME_LENGTH);

  deals_queue_file_pathname =  std::string("/home/") + std::string(current_username) + std::string("/NEURO_DEAL_ACTIONS");
  open_deals_queue();
  ping_connection();
}

NeuroDealAction::~NeuroDealAction()
{
  close_deals_queue();
}

int NeuroDealAction::update_deals()
{
  int is_there_smth_new = 0;
  is_there_smth_new += recieve_status();
  is_there_smth_new += recieve_results();

  return !!is_there_smth_new;
}

int NeuroDealAction::make_new_bet(const std::vector<deals_namespace::NewBet>& bets_vector)
{
  if(!ping_success)
    get_ping_answer();

  int msg_sent_counter = 0;

  deals_namespace::NewBet current_bet = {};

  if(ping_success)
  {
    for(int i = 0; i < bets_vector.size(); i++)
    {

      bets_array[free_array_position] = bets_vector[i];

      bets_array[free_array_position].mtype = deals_namespace::NEW_DEAL_MTYPE;

      if(msgsnd(asset_deals_queue_fd, (deals_namespace::NewBet*) &bets_array[free_array_position], sizeof(deals_namespace::NewBet) - sizeof(long), 0) < 0)
      {
        printf("NeuroDealAction::make_new_bet::Can\'t send bet message with id %d to queue\n", bets_array[free_array_position].bet_id);
        perror("msgsnd");
      }
      else
      {
        msg_sent_counter ++;
        printf("NeuroDealAction::Bet msg with id %d successfully sent\n", bets_array[free_array_position].bet_id);
      }

      free_array_position = (free_array_position + 1) % (2 * deals_namespace::MAX_DEALS_OPEN);
    }
  }
  else
  {
    printf("NeuroDealAction::make_new_bet:: Can\'t make bets: ping answer did not recieve\n");
    return -1;
  }

  return msg_sent_counter;
}

const std::vector<deals_namespace::BetStatus>& NeuroDealAction::get_updated_status()
{
  static std::vector<deals_namespace::BetStatus> status_to_return;
  status_to_return.clear();

  for(int i = 0; i < new_bets_status_amount; i++)
  {
    status_to_return.push_back(bets_status_array[bets_status_recieved_num[i]]);
  }

  new_bets_status_amount = 0;

  return status_to_return;
}

const std::vector<deals_namespace::DealResult>& NeuroDealAction::get_updated_results()
{
  static std::vector<deals_namespace::DealResult> results_to_return;
  results_to_return.clear();

  for(int i = 0; i < new_bets_result_amount; i++)
  {
    results_to_return.push_back(bets_results_array[bets_result_recieved_num[i]]);
  }

  new_bets_result_amount = 0;

  return results_to_return;
}
