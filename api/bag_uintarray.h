#ifndef BAG_UINTARRAY_H
#define BAG_UINTARRAY_H

namespace BAG
{

class UintArray
{
public:
    UintArray() = default;
    UintArray(size_t len): m_len(len), m_intlist(new uint8_t[len]) {}

    uint8_t* m_intlist;  
    size_t m_len;
};

}   //namespace BAG

#endif  //BAG_UINTARRAY_H
