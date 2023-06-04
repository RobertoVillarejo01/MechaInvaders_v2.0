/*!
*****************************************************************************
\file    type_info.hh
\author  Nestor Uriarte
\par     DP email: nestor.uriarte@lauro20.org
\par     Course: CS225
\par     Programming Assignment #2
\date    18/10/2018

\brief
    this file conatinst the classes TypeInfo and the template function type_of
*******************************************************************************/
#pragma once
#include <typeinfo>
#include <string>
#include <iostream>

    class TypeInfo
    {
        public:
            template <typename T>
            TypeInfo(const T& x);
            
            TypeInfo(const type_info& type);
            
            bool operator==(const TypeInfo& compare)const ;
            bool operator!=(const TypeInfo& compare)const;
            bool operator<(const TypeInfo& compare)const;
            
            std::string get_name() const;
        
        private:
            std::string name_;
    };
/*!
*****************************************************************************
    \fn TypeInfo(const T& x)
    
    \brief 
        The non default constructor for the class

  \param const T& x
    A reference to a run time known object

  \return 
    Does not return anything
*******************************************************************************/    
    template <typename T>
    TypeInfo::TypeInfo(const T& x)
    {
        name_ = typeid(x).name();//storing the type
    }
/*!
*****************************************************************************
    \fn TypeInfo(const T& x)
    
    \brief 
        This function returns a TypeInfo object that contains the type of 
        what we pass to it

  \param const T& x
    A reference to a run time known object

  \return TypeInfo
    returns a TypeInfo object
*******************************************************************************/    
    template <typename T>
    TypeInfo type_of(const T& type){return TypeInfo(type);}//returning a TypeInfo object
/*!
*****************************************************************************
    \fn TypeInfo type_of()
    
    \brief 
        This function returns a TypeInfo object that contains the type of 
        what we want

  \param 
    it takes no argument

  \return TypeInfo
    returns a TypeInfo object
*******************************************************************************/
    template <typename T>
    TypeInfo type_of()
    {
        return TypeInfo(typeid(T));//returning a TypeInfo object
    }
