#ifndef INCLUDE_ENGINE_H_
#define INCLUDE_ENGINE_H_

#include <string>
#include <iostream>
#include <fstream>
#include <list>
#include <map>
#include <unordered_map>
#include <memory>
#include <string>
#include <mutex>
#include <utility>

#include "polar_string.h"

namespace polar_race {

enum RetCode {
  kSucc = 0,
  kNotFound = 1,
  kCorruption = 2,
  kNotSupported = 3,
  kInvalidArgument = 4,
  kIOError = 5,
  kIncomplete = 6,
  kTimedOut = 7,
  kFull = 8,
  kOutOfMemory = 9,
};

// Pass to Engine::Range for callback
class Visitor {
 public:
  virtual ~Visitor() {}

  virtual void Visit(const PolarString &key, const PolarString &value) = 0;
};

class Engine {
 public:
  // Open engine
  static RetCode Open(const std::string& name,
      Engine** eptr);

  Engine() { }

  // Close engine
  virtual ~Engine() = default;
/*纯虚函数*/
  // Write a key-value pair into engine
  virtual RetCode Write(const PolarString& key,
      const PolarString& value) = 0;

  // Read value of a key
  virtual RetCode Read(const PolarString& key,
      std::string* value) = 0;


  /*
   * NOTICE: Implement 'Range' in quarter-final,
   *         you can skip it in preliminary.
   */
  // Applies the given Vistor::Visit function to the result
  // of every key-value pair in the key range [first, last),
  // in order
  // lower=="" is treated as a key before all keys in the database.
  // upper=="" is treated as a key after all keys in the database.
  // Therefore the following call will traverse the entire database:
  //   Range("", "", visitor)
 // virtual RetCode Range(const PolarString& lower,
 //     const PolarString& upper,
 //     Visitor &visitor) = 0;
};


struct HashFunc
{
	std::size_t operator()(const PolarString &key) const 
	{
        unsigned long h = 0;
        const char *p = key.data_;
        
        for (; *p; ++p)
             h =5*h + *p;

        return size_t(h);
	}
};


class MyEngine : public Engine
{
 public:
    using Item = std::pair<PolarString, PolarString>;
    using Iter = std::list<Item>::iterator;
    using CacheList = std::shared_ptr<std::list<Item>>;
    using Hashmap   = std::shared_ptr<std::unordered_map<PolarString,
                                                         Iter,
                                                     HashFunc>>;
    
    //using Hashmap   = std::shared_ptr<std::unordered_map<int, Iter>>;
    
    MyEngine ();
    ~MyEngine() = default;
 public:   
    
    RetCode Write (const PolarString& key, const PolarString& value);
    
    RetCode Read (const PolarString& key, std::string* value);

  // RetCode Range(const PolarString& lower,
    //  const PolarString& upper, Visitor &visitor);
 private:
    
    CacheList lruCache;
    Hashmap hashmap;
    uint64_t lruCacheCurSize;
    uint64_t lruCacheMaxSize;
};

}  // namespace polar_race

#endif  // INCLUDE_ENGINE_H_
