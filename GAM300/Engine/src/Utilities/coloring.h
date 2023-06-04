/*!
*****************************************************************************
\file    coloring.h
\author  Alba Alonso
\par     DP email: a.alonso@digipen.edu
\par     Course: CS260 Fall19
\par     Programming Assignment #2
\date    10-22-2019

\brief
	This file contains the interface for the printing with colors
*******************************************************************************/
#pragma once

namespace colors
{
    const int white = 7;	
    const int green = 2;	
    const int red = 4;		
    const int yellow = 6;	
    const int blue = 1;		
}

#if (  ( defined(__WIN32) || defined(_WIN32) || defined(_WIN64) ))

#define WIN32_LEAN_AND_MEAN

#include <Windows.h>
#include <iostream>

//print colored output 
template <typename T>
void print( const T& msg, int color = colors::white )
{
    HANDLE console = ::GetStdHandle(STD_OUTPUT_HANDLE);
    ::SetConsoleTextAttribute(console, color);
    ::SetConsoleTextAttribute(console, colors::white);
}

#else // colored output does not have an effect if 
      // disabled or run on a non-windows platform

#include <iostream>

//print without colored output
template <typename T>
void print( const T& msg, int )
{
}

#endif
