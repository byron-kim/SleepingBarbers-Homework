// Modified by Byron Kim
// Changes by Byron are commented and prefixed by "BK"

#include "Shop.h"

void Shop::init(int num_barbers) 
{
   pthread_mutex_init(&mutex_, NULL);
   pthread_cond_init(&cond_customers_waiting_, NULL);
   //pthread_cond_init(&cond_customer_served_, NULL);     BK remove for multiple cond variables
   //pthread_cond_init(&cond_barber_paid_, NULL);
   //pthread_cond_init(&cond_barber_sleeping_, NULL);

   for (int i = -1; i >= num_barbers * -1; i--) 
   {
      cond_customer_served_[i] = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
      pthread_cond_init( cond_customer_served_.find(i)->second, NULL);
      
      cond_barber_paid_[i] = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
      pthread_cond_init( cond_barber_paid_.find(i)->second, NULL);
      
      cond_barber_sleeping_[i] = (pthread_cond_t*)malloc(sizeof(pthread_cond_t));
      pthread_cond_init( cond_barber_sleeping_.find(i)->second, NULL);
   }

}

string Shop::int2string(int i) 
{
   stringstream out;
   out << i;
   return out.str( );
}

void Shop::print(int person, string message)
{
   cout << ((person > 0) ? "customer[" : "barber  [" ) << person << "]: " << message << endl;
}

int Shop::get_cust_drops() const
{
    return cust_drops_;
}

int Shop::visitShop(int customer_id) 
{
   pthread_mutex_lock(&mutex_);
   
   // If all chairs are full then leave shop
   // BK this IF block stays same for multiple barbers
   if (waiting_chairs_.size() == max_waiting_cust_) 
   {
      print( customer_id,"leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return 0;
   }
   
   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service
   // BK If there are any open service chairs, or if there is space at the waiting chair
   // BK then customer go to waiting chairs.
   
   if (customer_in_chair_.size() == num_barbers || !waiting_chairs_.empty()) 
   {
      waiting_chairs_.push(customer_id);
      print(customer_id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
      waiting_chairs_.pop();
   }
   
   // iterate through customer_in_chair_ to search for empty barber
   // if the barber is not in customer_in_chair_, the barber is available. Assign barber to this customer.
   int barber_id;
   for (int i = -1; i >= num_barbers * -1; i--) {
      if (customer_in_chair_.find(i) == customer_in_chair_.end()) {
         barber_id = i;
      }
   }
         
   print(customer_id, "moves to the service chair" + int2string(barber_id) + ". # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   customer_in_chair_[barber_id] = customer_id;  // BK vector version
   in_service_[barber_id] = true;

   // wake up the barber just in case if he is sleeping
   // BK modify for map
   pthread_cond_signal(cond_barber_sleeping_.find(barber_id)->second);

   pthread_mutex_unlock(&mutex_); 
   return barber_id;
}

void Shop::leaveShop(int customer_id, int barber_id) 
{
   pthread_mutex_lock( &mutex_ );
   
   // Wait for service to be completed
   print(customer_id, "wait for " + int2string(barber_id) + " to be done with hair-cut.");
   while (in_service_.find(barber_id)->second == true)
   {
      pthread_cond_wait(cond_customer_served_.find(barber_id)->second, &mutex_);
   }
   
   // Pay the barber and signal barber appropriately
   money_paid_[barber_id] = true;    // BK map version
   pthread_cond_signal(cond_barber_paid_.find(barber_id)->second);
   print( customer_id, "says good-bye to the barber." );
   pthread_mutex_unlock(&mutex_);
}

void Shop::helloCustomer(int barber_id) 
{
   pthread_mutex_lock(&mutex_);
   
   // If no customers then barber can sleep
   if (waiting_chairs_.empty() && customer_in_chair_.find(barber_id)->second == 0 ) 
   {
      print(barber_id, "sleeps because of no customers.");
      pthread_cond_wait(cond_barber_sleeping_.find(barber_id)->second, &mutex_);
   }

   if (customer_in_chair_.find(barber_id) == customer_in_chair_.end())      // check if the customer, sit down.
   {
       pthread_cond_wait(cond_barber_sleeping_.find(barber_id)->second, &mutex_);
   }

   print(barber_id, "starts a hair-cut service for customer " + int2string( customer_in_chair_.find(barber_id)->second ) );
   pthread_mutex_unlock( &mutex_ );
}

void Shop::byeCustomer(int barber_id) 
{
  pthread_mutex_lock(&mutex_);

  // Hair Cut-Service is done so signal customer and wait for payment
  in_service_[barber_id] = false;  // BK modify for map
  print(barber_id, "says he's done with a hair-cut service for " + int2string(customer_in_chair_.find(barber_id)->second));
  money_paid_[barber_id] = false;
  pthread_cond_signal(cond_customer_served_.find(barber_id)->second);
  while (money_paid_.find(barber_id)->second == false)
  {
      pthread_cond_wait(cond_barber_paid_.find(barber_id)->second, &mutex_);
  }

  //Signal to customer to get next one
  customer_in_chair_.erase(barber_id);
  print(barber_id, "calls in another customer");
  pthread_cond_signal( &cond_customers_waiting_ );

  pthread_mutex_unlock( &mutex_ );  // unlock
}


