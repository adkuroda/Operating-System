/**
 * This file contains the implementation of the Shop class. The Shop class
 * represents a barber shop. It contains a queue of waiting customers, an array
 * of barbers, and mutexes and condition variables to coordinate threads.
 * Shop is a monitor that allows customers to visit the shop and barbers to
 * service customers. 
*/
#include "Shop.h"
/**
 * Constructor with parameters. Initializes all member variables
*/
Shop::Shop(int num_chairs, int num_barbers) 
   : max_waiting_cust_(num_chairs), num_barbers_(num_barbers), cust_drops_(0)
{
   init();
}
/**
 * Default constructor. Initializes all member variables
*/
Shop::Shop() 
   : max_waiting_cust_(kDefaultNumChairs), num_barbers_(1), cust_drops_(0)
{
   init();
}
/**
 * Destructor. Deletes all dynamically allocated memory and 
 * destroys all mutexes and condition variables
*/
Shop::~Shop() 
{
   pthread_mutex_destroy(&mutex_);
   pthread_cond_destroy(&cond_customers_waiting_);
   for (int i = 0; i < num_barbers_; i++){
      pthread_cond_destroy(&barbers_[i].cond_barber_paid);
      pthread_cond_destroy(&barbers_[i].cond_barber_sleeping);
      pthread_cond_destroy(&barbers_[i].cond_customer_served);
   }
   delete [] barbers_;
   delete [] customer_in_chair_;
}
/**
 * Initializes all member variables and creates mutexes and condition variables
 * for each barber and there is one condition variable for customers waiting. 
 * This does not distinguish different customers waiting for different barbers.
*/
void Shop::init() 
{ 
   // initialize a mutex and condition variable for each barber
   pthread_mutex_init(&mutex_, NULL);
   // create an array of barbers
   barbers_ = new Barber[num_barbers_];
   // create an array of customers in chair
   customer_in_chair_ = new int[num_barbers_];
   // initialize the condition variable for customers waiting
   pthread_cond_init(&cond_customers_waiting_, NULL);
   for(int i = 0; i < num_barbers_; i++){
      Barber barber = Barber();
      barber.in_service = false;
      barber.money_paid = false;
      // initialize condition variables for each barber
      pthread_cond_init(&barber.cond_customer_served, NULL);
      pthread_cond_init(&barber.cond_barber_paid, NULL);
      pthread_cond_init(&barber.cond_barber_sleeping, NULL);
      barbers_[i] = barber; // add barber to array of barbers
      customer_in_chair_[i] = 0; // index is a barber

   }
}
/**
 * Utility function to check if all barbers are busy and 
 * If there is an empty chair, than returns false.
 * Otherwise, returns true.
*/
bool::Shop::isChairFull()
{
   for(int i = 0; i < num_barbers_; i++){
     if(customer_in_chair_[i] == 0){
        return false;
     }
   }
   return true;
}
/**
 * Utility function to convert an integer to a string
 * provided as part of the assignment
*/
string Shop::int2string(int i) 
{
   stringstream out;
   out << i;
   return out.str( );
}
/**
 * Utility function to print a message to the console.
 * Provided as part of the assignment but some modifications
 * were made to print the correct information to reflect the
 * the mulitple barbers. 
*/
void Shop::print(int person, string message, bool is_customer)
{
   string type = (is_customer) ? "customer[" : "barber  [" ;
   cout << type << person << "]: " << message << endl;
}

/**
 * Getter for number of cust. dropped due to the shop being full
*/
int Shop::get_cust_drops() const
{
    return cust_drops_;
}
/**
 * This function is utilized by the customer threads. It is called when a customer
 * arrives at the shop. The customer first checks if there are any available barbers.
 * If there are no available barbers, the customer checks if there are any available
 * waiting chairs. If there are no available waiting chairs, the customer leaves the
 * shop. If there are available waiting chairs, the customer takes a waiting chair
 * and waits for a barber to be available. If there are available barbers, the customer
 * takes the chair and signals specific barber that a customer is waiting. 
*/
int Shop::visitShop(int id) 
{
   pthread_mutex_lock(&mutex_);
   //cout << "Begin Visit Shop " << endl;
   // Check if there are any available barbers && any available waiting chairs
   if (waiting_chairs_.size() == max_waiting_cust_ && isChairFull() ) {
      print( id,"leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      // leave the shop because there are no available waiting chairs
      pthread_mutex_unlock(&mutex_); // unlock mutex
      return -1;
   }

   // if there are no available barbers, add the customer to the waiting chairs
   int barber_id = -1;
   if (isChairFull()){
      //cout << "before pushing " << endl;
      waiting_chairs_.push(id);
      print(id, "takes a waiting chair. # waiting seats available = " 
            + int2string(max_waiting_cust_ - waiting_chairs_.size()));
      while(isChairFull()){
         pthread_cond_wait(&cond_customers_waiting_, &mutex_);
         //cout << "after waiting chair to be empty" << endl;
      }
      // find the first barber that is not busy
      for (int i = 0; i < num_barbers_; i++){
         if (customer_in_chair_[i] == 0){
            barber_id = i;
            break;
         }
      }
      waiting_chairs_.pop();
   }else{
      // find the first barber that is not busy
      for (int i = 0; i < num_barbers_; i++){
         if (customer_in_chair_[i] == 0){
            barber_id = i;
            break;
         }
      }
   }
   barbers_[barber_id].in_service = true;
   customer_in_chair_[barber_id] = id;
   print(id, "moves to service chair[" + int2string(barber_id) 
         + "]. # waiting seats available = " 
         + int2string(max_waiting_cust_ - waiting_chairs_.size()));
  // signal the barber that a customer is waiting
   pthread_cond_signal(&barbers_[barber_id].cond_barber_sleeping);
   pthread_mutex_unlock(&mutex_);
   //cout << "End Visit Shop " << endl;
   return barber_id;
   
}
/**
 * This function is utilized by the customer threads. It is called after a 
 * customer gets a barber. The customer waits for the barber to be done with
 * the hair cut. Once the barber is done, the customer pays the barber and
 * signals the barber that the customer has paid. The customer then leaves
 * the shop.
*/
void Shop::leaveShop(int id, int barber_id) 
{
   pthread_mutex_lock( &mutex_ ); // lock the mutex
   //cout << "Begin Leave Shop " << endl;
   print(id, "wait for barber[" + int2string(barber_id) 
            + "] to be done with hair-cut");
   // wait for the barber to finish the hair cut
   if(barbers_[barber_id].in_service == true){
      pthread_cond_wait(&barbers_[barber_id].cond_customer_served, &mutex_);
      //cout << "after waiting for barber to be done with hair cut" << endl;
   }
   // pay the barber
   barbers_[barber_id].money_paid = true;
   //cout << "Singal the barber to be paid " << endl;
   // signal the barber that the customer has paid
   pthread_cond_signal(&barbers_[barber_id].cond_barber_paid);
   print(id, "says good-bye to the barber[" + int2string(barber_id) + "].");
   pthread_mutex_unlock(&mutex_); // unlock the mutex
   //cout << "End Leave Shop " << id << endl;
}
/**
 * This function is utilized by the barber threads. It is called when a barber
 * is ready to cut hair. The barber first checks if there are any customers
 * waiting. If there are no customers waiting, the barber goes to sleep and 
 * waits for a customer to signal that there is a customer waiting.
*/
void Shop::helloCustomer(int barber_id) 
{
   pthread_mutex_lock(&mutex_); // lock the mutex
  // cout << "Begin Hello Customer " << endl;
   // if there are no customers waiting, go to sleep
   if (customer_in_chair_[barber_id] == 0 && waiting_chairs_.empty()){
      print(barber_id, "sleeps because of no customers.", false);
      // wait for a customer to signal that there is a customer waiting
      pthread_cond_wait(&barbers_[barber_id].cond_barber_sleeping, &mutex_);
   }

   // wait to get a customer signal that there is a customer waiting
   if(customer_in_chair_[barber_id] == 0){
      pthread_cond_wait(&barbers_[barber_id].cond_barber_sleeping, &mutex_);
      // cout << "after waiting for signal from customer " << endl;
   }
   // find out the customer id that is assigned to the barber
   int cust_id = customer_in_chair_[barber_id];
   print(barber_id, "starts a hair-cut service for customer[" 
            + int2string(cust_id) + "]", false);
   pthread_mutex_unlock(&mutex_); // unlock the mutex
   //cout << "End Hello Customer " << endl;
   
}
/**
 * This function is utilized by the barber threads. It is called when a barber
 * starts to cut hair after a customer gets the barber. The barber first checks
 * finishes the hair cut and then signals the customer that the hair cut is done
 * and waits for the customer to pay. Once the customer pays, the barber signals
 * the customer that the barber is ready for another customer.
*/
void Shop::byeCustomer(int barber_id) 
{
   //cout << "Begin Bye Customer " << endl;
   pthread_mutex_lock(&mutex_);
   int cust_id = customer_in_chair_[barber_id];
   print(barber_id, "says he's done with the hair-cut service for customer[" 
                  + int2string(cust_id) + "]", false);
   barbers_[barber_id].money_paid = false;
   barbers_[barber_id].in_service = false;
   // signal the customer that the hair cut is done
   pthread_cond_signal(&barbers_[barber_id].cond_customer_served);
   // cout << "after signal customer served " << endl;
   // wait for the customer to pay
   while(barbers_[barber_id].money_paid == false){
      pthread_cond_wait(&barbers_[barber_id].cond_barber_paid, &mutex_);
      //cout << "after waiting for barber to be paid " << endl;
   }
   // reset the customer in the chair to 0 to show that current barber 
   // is done with the customer and ready for another customer
   customer_in_chair_[barber_id] = 0;
   barbers_[barber_id].money_paid = false;
   barbers_[barber_id].in_service = false;
   print(barber_id, "calls in another customer", false);
   //signal the customers that the barber is ready for another customer
   pthread_cond_signal(&cond_customers_waiting_);
   pthread_mutex_unlock( &mutex_ );  // unlock
   //cout << "End Bye Customer " << endl;
}
