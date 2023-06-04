#pragma once
/*! 
******************************************************************************
\*file   pool.hh
\author  Inigo Fernadez
\par     DP email: arenas.f@digipen.edu
\par     Course: CS225
\par     Assignment 4
\date    11-22-2019

\brief
     
  
  The structs included:

                /// This struct contains takes 2 arguments as templates parameters and is used to store
                /// a determined number of objects of the same type
                struct Block

                /// This struct struct contains a vector of Blocks of the same tipe and it is in charge of manageing all the memory
                /// allocating and deallocating blocks if needed
                struct Storage

*******************************************************************************/

#include <cstring>              //  std::size_t
#include <vector>               //  std::vector
#include <algorithm>            //  std::for_each

class IComp;
class GameObject;
class Space;

// -----------------------------------------------------------------------
// struct Block - This struct contains takes 2 arguments as templates parameters and is used to store a determined 
// number of objects of the same type; T is the type of the object to store and count is the number of objects to store
template <typename T, std::size_t count>
struct Block
{   
    virtual ~Block() {}
    // -----------------------------------------------------------------------
    // This function will return a pointer to the first available space in the array that will be used to store an object
    void * allocate();

    // -----------------------------------------------------------------------
    // This function will set the space that the address is pointed at as available because the object will get destroyed
    void deallocate( void* object);

    // -----------------------------------------------------------------------
    // This function will return the first available block found on the local memory array
    std::size_t first_available_slot()const;

    // -----------------------------------------------------------------------
    // This function will check if the address provided is pointing into the current Block
    bool belongs(const void* object)const;

    // -----------------------------------------------------------------------
    // This function will check if the block hasnt got any space left to store more objects
    bool is_full()const;

    // -----------------------------------------------------------------------
    // This function will check if the block is empty
    bool is_empty()const;

    // -----------------------------------------------------------------------
    // This function will return the size of the block
    const std::size_t& size()const;

    //this variable will store the element count that the Block will be able to store
    const static std::size_t element_count = count == 0 ? 1u : count;

private:
    //array of T types that will be used as the memmory space to store the objects of T type
    T mBlock[element_count];
    //array of booleans that will be used as a flag system to determine which slots are available
    bool mElements[element_count] = {false};

    //this variable will store the memmory size of the block
    std::size_t mSize = element_count*sizeof(T);
};

struct IStorage_
{
    // -----------------------------------------------------------------------
    // This destructor will destroy all the blocks allocated in the storage struct and clear the vector
    virtual ~IStorage_() {};

    // -----------------------------------------------------------------------
    // This function will return a pointer to an available slot in one of the available blocks stored in the vecotr of blocks
    virtual void* allocate() = 0;

    // -----------------------------------------------------------------------
    // This function will set as available the slot that was pointing to the address 
    // passed and will destroy the block if it is empty after the deallocation
    virtual void deallocate(void* slot_to_dealloc) = 0;

    virtual std::size_t& GetMemryInUse() = 0;
    virtual std::size_t  GetMemoryAllocated() = 0;
    virtual std::size_t& GetTypeSize() = 0;
    virtual std::size_t& GetPageSize() = 0;
};

template <typename T, std::size_t _size>
struct Storage : public IStorage_
{
    typedef Block<T, _size>       block_type;
    // -----------------------------------------------------------------------
    // This destructor will destroy all the blocks allocated in the storage struct and clear the vector
    virtual ~Storage();

    // -----------------------------------------------------------------------
    // This function will return a pointer to an available slot in one of the available blocks stored in the vecotr of blocks
    void* allocate();

    // -----------------------------------------------------------------------
    // This function will set as available the slot that was pointing to the address 
    // passed and will destroy the block if it is empty after the deallocation
    void deallocate(void* slot_to_dealloc);

    // -----------------------------------------------------------------------
    // This function will return the size of all th memmory allocated in all the blocks
    std::size_t size()const;

    // -----------------------------------------------------------------------
// This function will check if all of the blocks that are in the vector havent got any left space to store an abject
    bool is_full()const;

    // -----------------------------------------------------------------------
    // This function will return a pointer to an available block inside the vector
    block_type* available_block();

    std::size_t& GetMemryInUse()       { return MemoryInUse; }
    std::size_t  GetMemoryAllocated()  { return mBlocks.size() * _size; }
    std::size_t& GetTypeSize()         { return type_size; }
    std::size_t& GetPageSize()         { return page_size; }

private:
    //this vector will store all the blocks that will store all the memmory that is allocated
    std::vector<block_type*> mBlocks;
    std::size_t MemoryInUse = 0u;
    std::size_t type_size = sizeof(T);
    std::size_t page_size = _size;
};


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////                                         BLOCK                                                         /////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
// This function will return a pointer to the first available space in the array that will be used to store an object
template <typename T, std::size_t count>
void * Block<T,count>::allocate() 
{
    //security check if the block hasnt got any space left to store more objects
    if(is_full()) return nullptr;

    //get the index inside the array of object memory to the first available slot that will store an object
    std::size_t index = first_available_slot();

    //change the flag of that index to true because it is allocated now
    mElements[index] = true;
    //return the address that will has space to store an object of type T
    return mBlock+index;
}

// -----------------------------------------------------------------------
// This function will set the space that the address is pointed at as available because the object will get destroyed
template <typename T, std::size_t count>
void Block<T,count>::deallocate( void* object_address)
{
    //security check if the address is pointing to the current block 
    if(!belongs(object_address)) return;

    //get the index where the address passed is pointing at inside the space memory
    std::size_t index = reinterpret_cast<T*>(object_address)-mBlock;

    //set that slot to available
    mElements[index] = false;
}

// -----------------------------------------------------------------------
// This function will return the first available slot found on the local memmory array
template <typename T, std::size_t count>
std::size_t Block<T,count>::first_available_slot()const
{   
    //iterate through all the array of flags and return the index if a slot is available
    for(std::size_t index = 0u; index < element_count; index++)
    {
        if(!mElements[index]) return index;
    }
    //return -1 if no available slots were found
    return -1;
}

// -----------------------------------------------------------------------
// This function will check if the address provided is pointing into the current Block
template <typename T, std::size_t count>
bool Block<T,count>::belongs(const void* object_address)const
{
    //check if the address is pointing between the first slot and the last one
    return object_address >= mBlock && object_address <= mBlock + element_count;
}

// -----------------------------------------------------------------------
// This function will check if the block hasnt got any space left to store more objects
template <typename T, std::size_t count>
bool Block<T,count>::is_full()const
{
    //iterate through all the array of flags and return false if an available slot is found
    for(std::size_t index = 0u; index < element_count; index++)
    {
        if(!mElements[index]) return false;
    }
    //return true if no available slots were found
    return true;
}

// -----------------------------------------------------------------------
// This function will check if the block is empty
template <typename T, std::size_t count>
bool Block<T,count>::is_empty()const
{
    //iterate through all the array of flags and return false if an allocated slot is found
    for(std::size_t index = 0u; index < element_count; index++)
    {
        if(mElements[index]) return false;
    }
    //return true if no allocated slots were found
    return true;
}

// -----------------------------------------------------------------------
// This function will return the size of the block
template <typename T, std::size_t count>
const std::size_t& Block<T,count>::size()const 
{
    //simply return the size of the block
    return mSize;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////                                       STORAGE                                                         /////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------
// This destructor will destroy all the blocks allocated in the storage struct and clear the vector
template <typename T, std::size_t _size>
Storage<T, _size>::~Storage() 
{
    //delete all the blocks allocated in the storage
    for(block_type* block : mBlocks)
        delete block;

    //clear the vector of blocks
    mBlocks.clear();
}

// -----------------------------------------------------------------------
// This function will return a pointer to an available slot in one of the available blocks stored in the vecotr of blocks
template <typename T, std::size_t _size>
void* Storage<T, _size>::allocate()
{
    //check if there is no blocks allocated or all the blocks run out of space so we need to allocate a new one
    if (mBlocks.empty() || is_full())
        mBlocks.push_back(new block_type());

    MemoryInUse++;
    //return the first available slot found in the first one of tha blocks that has at least one available slot
    return available_block()->allocate();
}

// -----------------------------------------------------------------------
// This function will set as available the slot that was pointing to the address 
// passed and will destroy the block if it is empty after the deallocation
template <typename T, std::size_t _size>
void Storage<T, _size>::deallocate(void* slot_to_dealloc)
{
    auto it = std::find_if(mBlocks.begin(), mBlocks.end(),
        [&](auto block)
        {
            return block->belongs(slot_to_dealloc);
        });

    if (it != mBlocks.end())
    {
        (*it)->deallocate(slot_to_dealloc);
        MemoryInUse--;
        if ((*it)->is_empty())
        {
            block_type* temp = *it;
            //erase the block from the mBlock vector
            mBlocks.erase(it, it + 1);
            //free the block 
            delete temp;
        }
    }
}

// -----------------------------------------------------------------------
// This function will return the size of all th memmory allocated in all the blocks
template <typename T, std::size_t _size>
std::size_t Storage<T, _size>::size()const
{       
    //initialize the size to 0
    std::size_t mSize = 0u;

    //iterate through all of the blocks adding all the sizes of each block
    std::for_each(mBlocks.begin(), mBlocks.end(), [&]( block_type* block){ mSize += block->size(); });
    
    //return the total size
    return mSize;
}

// -----------------------------------------------------------------------
// This function will check if all of the blocks that are in the vector havent got any left space to store an abject
template <typename T, std::size_t _size >
bool Storage<T, _size>::is_full()const
{
    //iterate through all of the blocks, if one of the blocks isnt full then return false
    for(block_type* block : mBlocks)
        if(!block->is_full()) return false;
    //return true if all of the blocks are full
    return true;
}

// -----------------------------------------------------------------------
// This function will return a pointer to an available block inside the vector
template <typename T, std::size_t _size>
Block<T, _size>* Storage<T, _size>::available_block()
{
    //iterate throw all of the blocks, if the block isnt full(it has at leats one available space) return that block
    for(block_type* block : mBlocks)
        if(!block->is_full()) return block;

    //if no available blocks were found then return null
    return nullptr;
}