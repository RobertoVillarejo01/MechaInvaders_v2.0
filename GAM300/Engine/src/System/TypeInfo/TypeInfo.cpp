/*!
*****************************************************************************
\file    type_info.cc
\author  Nestor Uriarte
\par     DP email: nestor.uriarte@lauro20.org
\par     Course: CS225
\par     Programming Assignment #2
\date    18/10/2018

\brief
    This file contains the implementations for the functions declared on the
    type_info.hh file
    
  - TypeInfo(const std::type_info& type)
    conversion constructor for the class
    
  - std::string get_name()const
    returns the type
    
  - operator ==       
    The overload of the == operator for TypeInfo object.
    
  - operator !=       
    The overload of the != operator for TypeInfo object.
    
  - operator <       
    The overload of the < operator for TypeInfo object.
*******************************************************************************/
#include "TypeInfo.h"
/*!
*****************************************************************************
    \fn TypeInfo(const std::type_info& type)
    
    \brief 
        A conversion constructor for the class

  \param const std::type_info& type
    A constant reference to a std::type_info object 

  \return 
    Does not return anything
*******************************************************************************/
    TypeInfo::TypeInfo(const std::type_info& type)
    {
        name_ = type.name();//assigning the value for the private part of the class
    }
/*!
*****************************************************************************
    \fn std::string TypeInfo::get_name()const
    
    \brief 
        Returns if the name of the type

  \param 
    it takes no arguments

  \return std::string
    the string that contains the type
*******************************************************************************/    
    std::string TypeInfo::get_name()const {return name_;}
/*!
*****************************************************************************
    \fn bool operator==(const TypeInfo& compare) const
    
    \brief 
        Returns if the types are equal

  \param const TypeInfo& compare
    The TypeInfo to check if is equal

  \return bool
    if is the same or not
*******************************************************************************/    
    bool TypeInfo::operator==(const TypeInfo& compare) const
    {
        return (name_ == compare.get_name());//checking if are equal
    }
/*!
*****************************************************************************
    \fn bool operator!=(const TypeInfo& compare) const
    
    \brief 
        Returns if the types are not equal

  \param const TypeInfo& compare
    The TypeInfo to check if is not equal

  \return bool
    if is not the same or not
********************************************************************************/   
    bool TypeInfo::operator!=(const TypeInfo& compare) const
    {
        return (name_ != compare.get_name());//checking if not are equal
    }
/*!
*****************************************************************************
    \fn bool operator <(const TypeInfo& compare) const
    
    \brief 
        overload for the < operator in order to use the map container

  \param const TypeInfo& compare
    The TypeInfo to check if is lower

  \return bool
    if is lower or not
********************************************************************************/   
    bool TypeInfo::operator<(const TypeInfo& compare) const
    {
        return (name_ < compare.get_name());//checking if is lower or not
    }