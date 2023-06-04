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
    -          call: Does nothing in HandlerFunction, it will be overriden in 
                      child classes. MemberFunctionHandler will have this function
                      implemented, calls a specific event.
    -          MemberFunctionHandler: Custom constructor.    
    -          register_handler: registers a handler to the map with its specific
                     name
    
            
*******************************************************************************/
#pragma once
#include <typeinfo> //typeid
#include <map> //std::map
#include <string> //std::string

//polymorphic class to create the events
class Event
{
    public:
    virtual ~Event(){}
};

//another class to handle these events.
class HandlerFunction
{
    public:
    virtual ~HandlerFunction() {}
    void handle(const Event & event);
    
    private:
    virtual void call(const Event & event) = 0;
};

//a template class that inherits from the previous one
template<typename T, typename EVENT>
class MemberFunctionHandler : public HandlerFunction
{
    public:
    //this is the function pointer that will help us with event handling
    typedef void (T::*MemberFunction)(const EVENT &);
            
    /*!
		\brief Custom constructor
    */
    
    MemberFunctionHandler(T* object, MemberFunction fn) : 
    instance(object), 
    function(fn)
    {}
    
    /*!
		\brief calls the corresponding handling function accrding to the instantiated
		object (T) and to the event
    
		\param event the event to handle
	*/
   
    void call(const Event & event)
    {
        (instance->*function)(static_cast<const EVENT&>(event));
    }
    
    private:
    //our two templated parameters, one the object to create
    //and then the function to handle it
    T * instance;
    MemberFunction function;
    
};

//yet another handler class, this time to handle events themselves
class EventHandler
{
    public:
    
    /*!
		\brief Template function that will store a memberfunctionHandler in the map
		according to the event type and the instantiated object
		\param object template parameter, represents the object that is instantiated
		\param Function Pointer
	*/
   
    template<typename T, typename EVENT>
    void register_handler(T & object, void(T::*MemberFunction)(const EVENT&))
    {
	std::map<std::string, HandlerFunction*>::iterator it;
        it = handlers.find(typeid(EVENT).name());
        
        //if it is not the last one, we return, since it means
		//there is another one of this type
       if(it != handlers.end())
		return;
		//we register it
        handlers[typeid(EVENT).name()] = new MemberFunctionHandler<T, EVENT>(&object, MemberFunction);
    }

    void handle(const Event & event) const;

    ~EventHandler();
    
    private:
    std::map<std::string, HandlerFunction*> handlers;
};

