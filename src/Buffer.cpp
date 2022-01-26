#include "Buffer.hpp"

#include <cstdint>
#include <cstring>
#include <cassert>
#include <vector>

agent::Buffer::Buffer(std::size_t _size)
    : _data(_size), _used(0)
{}

std::size_t agent::Buffer::Write(const char* _input, std::size_t _size)
{
    // If the bytes used same as the size, write nothing
    if (_used == _data.size())
        return 0;
    
    // Number bytes that would be left over after a write
    std::size_t numBytes = _size + _used;
    std::size_t written = 0;
    
    // If new potential total number bytes < _data's size then write all _size
    if (numBytes < _data.size())
        written =  _size;
    else
        written = _data.size() - _used; // Else can only write as many
    
    // Copy new data into place after current data
    std::memcpy(_data.data(), _input, written);
    _used += written;

    // Return number of bytes written
    return written;
}

std::size_t agent::Buffer::Available() const
{
    return _used;
}

const char* agent::Buffer::Data() const
{
    return _data.data();
}

void agent::Buffer::Drain()
{
    _used = 0;
}

void agent::Buffer::Shift(std::size_t _count)
{
   assert(_count < _used);

   // Get the number that will be left after shift
   std::size_t newused = _used - _count;

   // Move _count : _count + newused to front
   std::memmove(_data.data(), _data.data() + _count, newused);

   // Set _used to the new value
   _used = newused;
}