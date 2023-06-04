/*!
******************************************************************************
    \file    event_dispatcher.cc
    \author  Jon Zapata
    \par     DP email: j.zapata@digipen.edu
    \par     Course: CS225
    \par     Assignment 2
    \date    10/10/2019

    \brief
     This file contains the definition of the EventDispatcher class' functions
      
      The functions include:

    -         subscribe:  	 Adds a new entry to the corresponding collection of 
							 events
    -         operator<<: 	 Overloaded operator to print information about the
							 event dispatcher.
    -         clear: 	  	 Clears the map
    -         trigger_event: (Member function) Iterates through the map in order to make the 
							 corresponding listener handle the corresponding 
							 event
    -         trigger_event: (Proxy function) calls EventDispatcher's
							 trigger_event. This allows us to use a more
							 readable syntax.
	-		  Unsubscribe:   Unsubscribes the event from the event_dispatcher
            
*******************************************************************************/
#include "EventDispatcher.h" //class declaration with function prototypes
#include "Collisions/CollisionSystem.h"
#include "LogicSystem/LogicSystem.h"

/*!
	 \brief Adds a new entry to the corresponding collection of 
    		events
	 \param listener The listener that will be subscribed
	 \param info The information about the event
*/
    
void EventDispatcher::subscribe(Listener & listener, 
const TypeInfo & info)
{
	//simply add the listener to the map using the information index
    mInfo[info.get_name()].push_back(&listener);
}

/*!
	\brief overloaded operator to print information about the event 
	 dispatcher 
	\param os The std::ostream reference necessary to perform the overloading
	\param eventdisp The eventdisp whose information we want to print
    \return std::ostream &   
*/
std::ostream& operator<<(std::ostream & os, const EventDispatcher & evendisp)
    {  
	//we create two iterators, one to go through the map and another
	//to traverse the list contained in the map
        std::map <std::string, std::list<Listener*> >:: const_iterator it;
        std::list<Listener*>::const_iterator it2;
        
		//we start at the beginning and we print the type information
        for(it = evendisp.mInfo.begin(); it != evendisp.mInfo.end(); ++it)
        {
            os << "The event type " << it->first/*.get_name()*/ <<" has the following subscribers:\n";
            
			//we are now looping over the list (we use the first
			//iterator's second to access it)
            for(it2 = it->second.begin(); it2 != it->second.end(); ++it2)
			//we print the name of the listener (we dereference it2 twice
			//so that we first dereference the iterator itself and then
			//the listener * 
                os <<"\tAn instance of type "<<typeid(**it2).name()<<"\n";
        }
        
        return os;
    }
    
/*!
	\brief clears the map containing the information
*/
    void EventDispatcher::clear()
    {
        std::map <std::string, std::list<Listener*> >:: iterator it;
     
     for(it = mInfo.begin(); it != mInfo.end(); ++it)
           it->second.clear();

        mInfo.clear();
    }

/*!
	\brief  iterates through the map so that the corresponding listener
	handles the corresponding event
	\param event the event to trigger
*/
    
    void EventDispatcher::trigger_event(const Event & event)
    {
	//once again, two iterators
        std::map <std::string, std::list<Listener*> >:: iterator it;
        std::list<Listener*>::iterator it2;
        
        for(it = mInfo.begin(); it != mInfo.end(); ++it)
        {           
			//we now simply dereference the iterator so that we get
			//a listener * that calls its virtual function handle_event
            for(it2 = it->second.begin(); it2 != it->second.end(); ++it2)
                (*it2)->handle_event(event);
        }
    }
    
/*!
	\brief Proxy function that allows us to use a more readable syntax. Does
	the same as the one above
	\param event the event to trigger
*/

    void trigger_event(const Event & event)
    {
		//simply call EventDispatcher's trigger event
        EventDisp.trigger_event(event);
    }
    
/*!
	\brief Removes an entry to the corresponding collection of 
	events
    \param listener The listener that will be subscribed	
	\param info The information about the event
*/

    void EventDispatcher::unsubscribe(Listener & listener, const TypeInfo & info)
    {
        std::map <std::string, std::list<Listener*> >:: iterator it;
		
		//we look for the event in our map
        it = mInfo.find(info.get_name());
		
		//if it is not the last one, we remove it from the list
        if(it != mInfo.end())
		  it->second.remove(&listener);

        
    }

#pragma region// COLLISION EVENTS
    void EventDispatcher::subscribe_collison(Listener* listener, unsigned id)
    {
        collision_map[id].push_back(listener);
    }

    void EventDispatcher::unsubscribe_collision(unsigned id)
    {
        collision_map.erase(id);
    }

    void EventDispatcher::trigger_collision_event(const CollisionEvent& event)
    {
        if (!event.coll1->mOwner || !event.coll2->mOwner) return;

        unsigned ID_1 = event.coll1->mOwner->GetUID();
        unsigned ID_2 = event.coll2->mOwner->GetUID();

        //std::vector<Listener*>::iterator it = collision_map[ID_1];
        std::for_each(collision_map[ID_1].begin(), collision_map[ID_1].end(),
            [&](Listener* listener)
            {
                ILogic* mComp = dynamic_cast<ILogic*>(listener);
                if (mComp)
                {
                    if (mComp->mOwner == event.coll1->mOwner)
                    {
                        switch (event.mType)
                        {
                        case CollisionType::CollisionStarted:
                            mComp->OnCollisionStarted(event.coll2);
                            break;
                        case CollisionType::CollisionPersisted:
                            mComp->OnCollisionPersisted(event.coll2);
                            break;
                        case CollisionType::CollisionEnded:
                            mComp->OnCollisionEnded(event.coll2);
                            break;
                        }
                    }
                }
            });

        std::for_each(collision_map[ID_2].begin(), collision_map[ID_2].end(),
            [&](Listener* listener)
            {
                ILogic* mComp = dynamic_cast<ILogic*>(listener);
                if (mComp)
                {
                    if (mComp->mOwner == event.coll2->mOwner)
                    {
                        switch (event.mType)
                        {
                        case CollisionType::CollisionStarted:
                            mComp->OnCollisionStarted(event.coll1);
                            break;
                        case CollisionType::CollisionPersisted:
                            mComp->OnCollisionPersisted(event.coll1);
                            break;
                        case CollisionType::CollisionEnded:
                            mComp->OnCollisionEnded(event.coll1);
                            break;
                        }
                    }
                }
            });
    }
#pragma endregion

