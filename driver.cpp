// Modified by Byron Kim
// Changes by Byron are commented and prefixed by "BK"

#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>  // BK malloc
#include "Shop.h"
using namespace std;

void *barber(void *);
void *customer(void *);

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
   // BK changed to argc != 5 (from argc != 4) to include argument input for number of barbers
   if (argc != 5)
   {
       cout << "Usage: num_barbers, num_chairs, num_customers, service_time." << endl;
       return -1;
   }
   int num_barbers = atoi(argv[1]);       // BK add num_barbers
   int num_chairs = atoi(argv[2]);
   int num_customers = atoi(argv[3]);
   int service_time = atoi(argv[4]);

   // Single barber, one shop, many customers - BK changed to Multiple Barbers.
   // BK Change shop constructor to take argument num_barbers
   pthread_t barber_threads[num_barbers];
   pthread_t customer_threads[num_customers];
   Shop shop(num_barbers, num_chairs);   // BK constructor with num_barbers parameter
  
   // BK create for loop block to create multiple barbers
   for (int i = 0; i < num_barbers; i++)
   {
      int barber_id = i + 1;
      ThreadParam* barber_param = new ThreadParam(&shop, barber_id, service_time);
      pthread_create(&barber_threads[i], NULL, barber, barber_param);
   }

   for (int i = 0; i < num_customers; i++) 
   {
      usleep(rand() % 1000);
      int customer_id = i + 1;
      ThreadParam* customer_param = new ThreadParam(&shop, customer_id, 0);
      pthread_create(&customer_threads[i], NULL, customer, customer_param);
   }

   // Wait for customers to finish and cancel barber
   for (int i = 0; i < num_customers; i++)
   {
       pthread_join(customer_threads[i], NULL);
   }
   // BK add for loop to cancel multiple barbers
   for (int i = 0; i < num_barbers; i++) {
      pthread_cancel(barber_threads[i]);
   }

   cout << "# customers who didn't receive a service = " << shop.get_cust_drops() << endl;
   return 0;
}

void *barber(void *arg) 
{
   ThreadParam &barber_param = *(ThreadParam*) arg;
   Shop& shop = *(barber_param.shop);
   int barber_id = barber_param.id * -1;  // make negative id for barbers
   int service_time = barber_param.service_time;
   delete &barber_param;

   while(true) 
   {
      shop.helloCustomer(barber_id);   // pick up new customer
      usleep(service_time);
      shop.byeCustomer(barber_id);     // release the customer
   }
   return nullptr;
}

void *customer(void *arg) 
{
   ThreadParam &customer_param = *(ThreadParam*)arg;
   Shop& shop = *(customer_param.shop);
   int customer_id = customer_param.id;
   delete &customer_param;

   // if assigned to barber i then wait for service to finish
   // 0 means did not get barber
   int barber;
   if ( (barber = shop.visitShop(customer_id)) != 0)
   {
       shop.leaveShop(customer_id, barber);  // wait until my service is finished
   }
   return nullptr;
}
