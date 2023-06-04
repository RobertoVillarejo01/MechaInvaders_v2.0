#pragma once
/*! 
******************************************************************************
\*file   allocator.hh
\author  Inigo Fernadez
\par     DP email: arenas.f@digipen.edu
\par     Course: CS225
\par     Assignment 4
\date    11-22-2019

\brief
    this program contains the definition of the struct SimpleAllocator which will work as a simple substitute of the std::allocator,
    we will create this allocator to understand how allocators works 
  
  The structs included:

                /// This struct is used to alloc a number of objects of the type that the struct was created, it also deallocs the memory
                /// this allocator will be in charge of managing all the memory requested but since we are not providing any memory customization it
                /// will simply call the global new and delete to creates the memory 
                struct SimpleAllocator
			   
                /// This struct is used as an allocator but with a custom memory so it doesnt use the new keyword 
                /// to allocate memory, instead it use the arena memory that is provided to alloc everything needed
                struct ArenaAllocator
*******************************************************************************/

#include <cstring> //std::size_t ; std::ptrdiff_t
#include <memory>  //std::allocator
#include "pool.hh"






// -----------------------------------------------------------------------
// struct ArenaAllocator - This struct is used to alloc a number of objects of the type that the struct was created, it also deallocs the memory
/// this allocator will be in charge of managing all the memory requested but since we are not providing any memory customization it
/// will simply call the global new and delete to creates the memory
//template <typename T>
//struct SimpleAllocator
//{
//    //typedefs that will be helpfull in the member functions of the struct
//    typedef T                   value_type;
//    typedef value_type*         pointer;
//    typedef const value_type*   const_pointer;
//    typedef value_type&         reference;
//    typedef const value_type&   const_reference;
//    typedef std::size_t         size_type;
//
//    // -----------------------------------------------------------------------
//    // SimpleAllocator() - default constructor for the SimpleAllocator
//    SimpleAllocator() {}
//
//    // -----------------------------------------------------------------------
//    // struct rebind - convert an SimpleAllocator<T> to an ArenaAllocator<U>
//    template <typename U>
//    struct rebind {typedef SimpleAllocator<U> other; };
//
//    // -----------------------------------------------------------------------
//    // SimpleAllocator(SimpleAllocator<U> const& mAlloc) - explicit conversion constructor of the simple allocator 
//    template <typename U>
//    explicit SimpleAllocator(SimpleAllocator<U> const&){}
//
//    // -----------------------------------------------------------------------
//    // pointer address(reference r) - return the address of the object type
//    inline pointer address(reference r){return &r;}
//    // -----------------------------------------------------------------------
//    // const_pointer address(const_reference r) - return the const address of the object type
//    inline const_pointer address(const_reference r){return &r;}
//
//    // -----------------------------------------------------------------------
//    // allocate(size_type count = 1, typename std::allocator<void>::const_pointer = 0) - this function will return a pointer to the memory address
//    // which has enough space to store the memory requested in this case using the global new function that will alloc dynamically the space in memory
//    // size_type count = 1 - number of objects that the user wants to allocate
//    virtual pointer allocate(size_type count = 1, typename std::allocator<void>::const_pointer = 0)
//    {
//        //return the address of the amount of memory allocated
//        return reinterpret_cast<pointer>(::new char[count* sizeof(T)]);
//    }
//
//    // -----------------------------------------------------------------------
//    // void deallocate(pointer p, size_type = 0) - this function will destroy the memory allocated as an array
//    // pointer p - delete the memory
//    void deallocate(pointer p, size_type = 0)
//    {
//        //free all the memory
//        ::delete [] p;
//    }
//
//    // -----------------------------------------------------------------------
//    // void construct(pointer p, const T& t) - this function will construct the object into the buffer passed to the function
//    // pointer p - pointer to the memory where we wil construct the object, const T& t = object that will be constructed
//    inline void construct(pointer p, const T& t) { new (p) T(t); }
//
//    // -----------------------------------------------------------------------
//    // void destroy(pointer p) - this function will call the destructor of the object passed
//    // pointer p - object that will be "destoryed"
//    inline void destroy(pointer p) {p->~T();}
//};
//
//template <typename T>
//struct PoolAllocator
//{
//    typedef T                               value_type;
//    typedef value_type*                     pointer;
//    typedef const value_type*               const_pointer;
//    typedef value_type&                     reference;
//    typedef const value_type&               const_reference;
//    typedef std::size_t                     size_type;
//    typedef Storage<Block<T>>         Storage;
//
//    // -----------------------------------------------------------------------
//    // PoolAllocator() - default constructor for the PoolAllocator
//    PoolAllocator() : mStorage{get_storage<value_type>()} {}
//
//    // -----------------------------------------------------------------------
//    // struct rebind - convert an PoolAllocator<T> to an PoolAllocator<U>
//    template <typename U>
//    struct rebind {typedef PoolAllocator<U> other; };
//
//    // -----------------------------------------------------------------------
//    // PoolAllocator(PoolAllocator<U> const& mAlloc) - explicit conversion constructor of the pool allocator 
//    template <typename U>
//    explicit PoolAllocator(PoolAllocator<U> const& ) : mStorage{get_storage<value_type>()} { }
//
//    // -----------------------------------------------------------------------
//    // pointer address(reference r) - return the address of the object type
//    inline pointer address(reference r){return &r;}
//    // -----------------------------------------------------------------------
//    // const_pointer address(const_reference r) - return the const address of the object type
//    inline const_pointer address(const_reference r){return &r;}
//
//    // -----------------------------------------------------------------------
//    // allocate(size_type count = 1, typename std::allocator<void>::const_pointer = 0) - this function will return a pointer to the memory address
//    // which has enough space to store the memory requested in this case using the global new function that will alloc dynamically the space in memory
//    virtual pointer allocate(size_type = 1, typename std::allocator<void>::const_pointer = 0)
//    {
//        return reinterpret_cast<pointer>(mStorage.allocate());
//    }
//
//    // -----------------------------------------------------------------------
//    // void deallocate(pointer p, size_type = 0) - this function will deallocate using the pool deallocate function
//    void deallocate(pointer p, size_type = 0)
//    {
//        //deallcate all the memory
//        mStorage.deallocate(p);
//    }
//
//    // -----------------------------------------------------------------------
//    // void construct(pointer p, const T& t) - this function will construct the object into the buffer passed to the function
//    // pointer p - pointer to the memory where we wil construct the object, const T& t = object that will be constructed
//    inline void construct(pointer p, const T& t) { new (p) T(t);}
//
//    // -----------------------------------------------------------------------
//    // void destroy(pointer p) - this function will call the destructor of the object passed
//    // pointer p - object that will be "destoryed"
//    inline void destroy(pointer p) {p->~T();}
//
//    //variable that will store my pool
//    Storage mStorage;
//};


