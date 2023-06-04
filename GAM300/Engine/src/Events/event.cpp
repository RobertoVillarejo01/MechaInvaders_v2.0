/*! 
******************************************************************************
    \file    event.hh
    \author  Jon Zapata
    \par     DP email: j.zapata@digipen.edu
    \par     Course: CS225
    \par     Assignment 2
    \date    10/10/2019

    \brief
     This file contains the declarations of various classes with the objective
     of handling multiple events. Classes include base Event class, EventHandler,
     FunctionHandler and MemeberFunctionHandler
      
      The functions include:

    -         ~Event: Virtual destructor. Does nothing, simply allows the class
                      to be polymorphic
    -          handle: Handles an event by calling it (HandlerFunction), calls 
                      the corresponding function to the corresponding event
                      (MemberFunctionHandler) and finds a HandlerFunction in 
                       a map to then use its own handle.
    -         ~EventHandler: clears the map
    
            
*******************************************************************************/
#include "event.h"


/*!
	\brief handles an event by calling it
	\param event - the event to handle
*/

void HandlerFunction::handle(const Event & event)
 {
     call(event);
 }
 
/*!
	 \brief finds the corresponding function handler in the map 
	 and uses its corresponding handle.
	 \param event the event to handle
*/
 
 void EventHandler::handle(const Event & event) const
 {
     //we use an std::map iterator and find the handler function
     //by using the name of the event it is handling
     std::map<std::string, HandlerFunction*>::const_iterator it;
     it = handlers.find(typeid(event).name());
     
     //if it is not the last one, we handle the event
    if(it != handlers.end())
        it->second->handle(event); 
 }
    
 
/*!
	\brief clears the map
*/

  EventHandler::~EventHandler()
  {
      std::map<std::string, HandlerFunction*> :: iterator it;
      for( it = handlers.begin(); it != handlers.end(); ++it)
         delete it->second;
    handlers.clear();
  }
