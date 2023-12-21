#ifndef SHOP_H_
#define SHOP_H_
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
#include <cstring> 


using namespace std;

#define kDefaultNumChairs 3

class Shop
{
public:
// Constructor and destructor
   Shop(int num_chairs, int num_barbers);
   Shop();
   ~Shop();

// Member functions
   int visitShop(int id);   // return true only when a customer got a service
   void leaveShop(int id, int barber_id);
   void helloCustomer(int barber_id);
   void byeCustomer(int barber_id);
   int get_cust_drops() const;

 private:
   const int max_waiting_cust_;  // the max number of threads that can wait
   const int num_barbers_;  // the number of barbers
   //int customer_in_chair_;
   int* customer_in_chair_; // index is barber id, value is customer id
   // bool in_service_;            
   // bool money_paid_;
   queue<int> waiting_chairs_;

  
   int cust_drops_;

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;
   pthread_cond_t  cond_customers_waiting_;
   // pthread_cond_t  cond_customer_served_;
   // pthread_cond_t  cond_barber_paid_;
   // pthread_cond_t  cond_barber_sleeping_;

   //static const int barber = 0; // the id of the barber thread


   // Represents a barber thread and all the condition variables
   // In addition, it holds a boolean value to indicate if the barber is in service
   // and if the customer has paid
   struct Barber
   {
      bool in_service;
      bool money_paid;
      pthread_cond_t cond_barber_paid;
      pthread_cond_t cond_barber_sleeping;
      pthread_cond_t cond_customer_served;
   };
   // Holds all the barber threads
   Barber* barbers_;
  
   // Utility functions
   void init();
   string int2string(int i);
   void print(int person, string message, bool is_customer = true);
   bool isChairFull();
};
#endif
