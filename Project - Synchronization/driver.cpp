#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include "Shop.h"
//using namespace std;

// Function prototypes
void *barber(void *);
void *customer(void *);
int validateInput(char* arg);

// ThreadParam class
// This class is used as a way to pass more
// than one argument to a thread. 

class ThreadParam
{
public:
    ThreadParam(Shop* shop, int id, int service_time) :
        shop(shop), id(id), service_time(service_time) {};
    Shop* shop;         
    int id;             
    int service_time;    
};

int main(int argc, char *argv[]) 
{

   // Read arguments from command line
   // TODO: Validate values
   if (argc != 5)
   {
       cout << "Usage: num_chairs num_customers service_time" << endl;
       return -1;
   }
   int num_barbers = validateInput(argv[1]);
   int num_chairs = validateInput(argv[2]);
   int num_customers = validateInput(argv[3]);
   int service_time = validateInput(argv[4]);

   if(num_barbers < 0 || num_chairs < 0 || num_customers < 0 || service_time < 0){
       cerr << "Invalid input" << endl;
       return -1;
   }

   //Single barber, one shop, many customers
   pthread_t barber_thread[num_barbers];
   pthread_t customer_threads[num_customers];
   Shop shop(num_chairs, num_barbers);
    // create barber threads
    for(int i = 0; i < num_barbers; i++){
        ThreadParam* barber_param = new ThreadParam(&shop, i, service_time);
        pthread_create(&barber_thread[i], NULL, barber, barber_param);
    }

   for (int i = 0; i < num_customers; i++) 
   {
      usleep(rand() % 1000);
      int id = i + 1;
      ThreadParam* customer_param = new ThreadParam(&shop, id, 0);
      pthread_create(&customer_threads[i], NULL, customer, customer_param);
   }

   // Wait for customers to finish and cancel barber
   for (int i = 0; i < num_customers; i++)
   {
       pthread_join(customer_threads[i], NULL);
   }
    // cancel all the barber threads 
   for(int i = 0; i < num_barbers; i++){
        pthread_cancel(barber_thread[i]);
   }

   cout << "# customers who didn't receive a service = " << shop.get_cust_drops() << endl;
   return 0;
}

void *barber(void *arg) 
{
   ThreadParam* barber_param = (ThreadParam*) arg;
   Shop& shop = *barber_param->shop;
   int service_time = barber_param->service_time;
   int barb_id = barber_param->id;
   delete barber_param;

   while(true) 
   {
      shop.helloCustomer(barb_id);
      usleep(service_time);
      shop.byeCustomer(barb_id);
   }
   return nullptr;
}

void *customer(void *arg) 
{
   ThreadParam* customer_param = (ThreadParam*)arg;
   Shop& shop = *customer_param->shop;
   int id = customer_param->id;
   delete customer_param;
   int barber_id = shop.visitShop(id);
   if (barber_id >= 0) 
   {
      shop.leaveShop(id, barber_id);
   }
   return nullptr;
}
/**
 * Utility function to validate input
*/
int validateInput(char* arg){
    for(int i = 0; i < strlen(arg); i++){
        if(!isdigit(arg[i])){
            return -1;
        }
    }
    return atoi(arg);
}
