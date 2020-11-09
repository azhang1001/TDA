#ifndef _DARTLIB_BASIC_H_
#define _DARTLIB_BASIC_H_

#include <algorithm>
#include <vector>

namespace DartLib
{
// TODO: merge EdgeMapKey and FaceMapKey using templete
struct EdgeMapKey
{
    int indices[2];

    EdgeMapKey(const int* indices)
    {
        this->indices[0] = indices[0];
        this->indices[1] = indices[1];
        std::sort(std::begin(this->indices), std::end(this->indices));
    }

    bool operator==(const EdgeMapKey& other) const 
    { 
        return this->indices[0] == other.indices[0] &&
               this->indices[1] == other.indices[1]; 
    }
};

struct EdgeMapKey_hasher
{
    size_t operator()(EdgeMapKey const& key) const
    {
        // TODO: the number of the vertices depends on the design
        //       method of computing hash value
        //       if we consider to use double instead of size_t,
        //       then the range of hash value could be larger.
        size_t hash = (size_t) key.indices[0] * 1000000000L +
                      (size_t) key.indices[1];
        return hash;
    }
};

struct FaceMapKey
{
    std::vector<int> indices;

    FaceMapKey(const int* indices)
    {
        this->indices.resize(3);
        this->indices[0] = indices[0];
        this->indices[1] = indices[1];
        this->indices[2] = indices[2];
        std::sort(std::begin(this->indices), std::end(this->indices));
    }

    bool operator==(const FaceMapKey& other) const
    {
        return this->indices[0] == other.indices[0] &&
               this->indices[1] == other.indices[1] && 
               this->indices[2] == other.indices[2];
    }
};

struct FaceMapKey_hasher
{
    size_t operator()(FaceMapKey const& key) const
    {
        // TODO: the number of the vertices depends on the design
        //       method of computing hash value
        size_t hash = (size_t) key.indices[0] * 1000000000000L +
                      (size_t) key.indices[1] * 1000000L +
                      (size_t) key.indices[2];
        return hash;
    }
};

}
#endif