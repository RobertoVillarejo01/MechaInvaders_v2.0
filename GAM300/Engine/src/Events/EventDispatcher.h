/*! 
******************************************************************************
    \file    event_dispatcher.hh
    \author  Jon Zapata
    \par     DP email: j.zapata@digipen.edu
    \par     Course: CS225
    \par     Assignment 2
    \date    10/10/2019

    \brief
     This file contains the declaration of the EventDispatcher class
      
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
#pragma once

#include <list> //std::list
#include <iostream> //std::ostream
#include <vector>
#include <map> //std::map

#include "event.h"  //event class
#include "System/TypeInfo/TypeInfo.h" //typeInfo class
#include "Utilities/Singleton.h"

//an interface class
class Listener
{
    public:
    virtual ~Listener() {}
	virtual void handle_event(const Event & ev) = 0;
};

struct CollisionEvent;

class EventDispatcher
{
    MAKE_SINGLETON(EventDispatcher)

  public:
    void subscribe( Listener & listener, const TypeInfo & info); 
    friend std::ostream& operator<<(std::ostream & os, const EventDispatcher & evendisp);
    void clear();
    void trigger_event(const Event & event);
    void unsubscribe( Listener & listener, const TypeInfo & info);
    
#pragma region// COLLISION EVENTS
    std::map<unsigned, std::vector<Listener*>> collision_map;
    void subscribe_collison(Listener* listener, unsigned id);
    void unsubscribe_collision(unsigned id);
    void trigger_collision_event(const CollisionEvent& event);
#pragma endregion

    private:
    std::map <std::string, std::list<Listener*> > mInfo;
};

#define EventDisp EventDispatcher::Instance()

void trigger_event(const Event & event);
