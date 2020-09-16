#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

 
template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
  	std::unique_lock<std::mutex> receiveMsgLock(_mutexMsgQueue);
  
  	_condition.wait(receiveMsgLock, [this] {
        return !_queue.empty();
    });
  
  
  
  	T msg = std::move(_queue.back());
    _queue.pop_back();
  return msg; 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
  	std::lock_guard<std::mutex> uLock(_mutexMsgQueue);
  	_queue.push_back(std::move(msg));
  	_condition.notify_one();
  	
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
  	_msgQueue = std::make_shared<MessageQueue<TrafficLightPhase>>();

}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
  while (true){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (_msgQueue->receive() == TrafficLightPhase::green){
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::setCurrentPhase(const TrafficLightPhase & newPhase)
{
    // Should we not protect this with a lock? More than one thread could ask for the status of the light.
    // One intersection has one light at this time ... So as long as multiple cars in multiple threads as
    // update from each intersection and not directly from the light, I guess we good ... 
    // One Intersection from multiple threads could however ask for the information?

    _currentPhase = newPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class. 
  	threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}
// virtual function which is executed in a thread
long RandomTime(const long & min, const long & max)
{
    if ((min > 0) && (max > 0) && (min<10000) && (max<10000) && (min<max))
    {
        std::random_device rd;  //Used to obtain a seed for the random number engine
        std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
        std::uniform_int_distribution<> dis(min, max);
        return (long)dis(gen);
    }

    return 5000;  // In case input is invalid, return 5 secs 
}

void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 
  long cycleDuration = RandomTime(4000,6000);
  std::chrono::time_point<std::chrono::system_clock> lastUpdate;
  
  lastUpdate = std::chrono::system_clock::now();
  
    while (true)
    {
        // sleep at every iteration to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        long timeSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - lastUpdate).count();
        if (timeSinceLastUpdate >= cycleDuration)
        {
			auto message = getCurrentPhase();
           
            if (message == TrafficLightPhase::red){
                message = TrafficLightPhase::green;
            } else {
                message = TrafficLightPhase::red;
            }
            setCurrentPhase(message);           

        
            _msgQueue->send(std::move(message));

            
            lastUpdate = std::chrono::system_clock::now();
            
            
            cycleDuration = RandomTime(4000,6000);
        }
    } // eof simulation loop
} 

