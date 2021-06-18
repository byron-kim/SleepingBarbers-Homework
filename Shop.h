// Modified by Byron Kim
// Changes by Byron are commented and prefixed by "BK"

#ifndef SHOP_H_
#define SHOP_H_
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
#include <map>
using namespace std;

#define kDefaultNumChairs 3
#define kDefaultBarbers 1

class Shop 
{
public:
   // BK remove from initializer customer_in_chair, in_service, money_paid; move to code block
   Shop(int num_barbers, int num_chairs) : num_barbers(num_barbers), max_waiting_cust_((num_chairs > 0 ) ? num_chairs : kDefaultNumChairs), cust_drops_(0)
   { 
      init(num_barbers); 
   };
   Shop() : num_barbers(kDefaultBarbers), max_waiting_cust_(kDefaultNumChairs), cust_drops_(0)
   { 
      init(num_barbers); 
   };

   // BK returns ID of barber if a customer is serviced by barber. returns -1 if customer is not serviced.
   int visitShop(int customer_id);   
   void leaveShop(int customer_id, int barber_id);
   void helloCustomer(int barber_id);
   void byeCustomer(int barber_id);
   int get_cust_drops() const;

 private:
   int num_barbers;               // BK add num_barbers as parameter for driver
   const int max_waiting_cust_;              // the max number of threads that can wait
   map<int, int> customer_in_chair_;      // BK change to vector. key = barber_id, value = customer_id
   map<int, bool> in_service_;            // BK change to vector. key = barber_id
   map<int, bool> money_paid_;            // BK change to vector. key = barber_id
   queue<int> waiting_chairs_;    // includes the ids of all waiting threads
   int cust_drops_;

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;                  
   pthread_cond_t  cond_customers_waiting_;
   map<int, pthread_cond_t*>  cond_customer_served_;     // BK key barber_id, value condition variable 
   map<int, pthread_cond_t*>  cond_barber_paid_;         // BK key barber_id, value condition variable 
   map<int, pthread_cond_t*>  cond_barber_sleeping_;     // BK key barber_id, value condition variable 

   map<int, pthread_cond_t>  barber_sleeping_;


   static const int barber = 0;    // the id of the barber thread
   
   void init(int num_barbers);
   string int2string(int i);
   void print(int person, string message);
};


   
#endif
