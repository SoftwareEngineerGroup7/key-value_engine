// Copyright [2018] Alibaba Cloud All rights reserved
#ifndef INCLUDE_ENGINE_H_
#define INCLUDE_ENGINE_H_
#include <string>
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
  virtual ~Engine();
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
  virtual RetCode Range(const PolarString& lower,
      const PolarString& upper,
      Visitor &visitor) = 0;
};

}  // namespace polar_race


//在这里对他的内容进行扩展
class MyEngine:public engine
{
 public:
    using Item = std::pair<PolarString, PolarString>;
    using Iter = std::list<Item>::iterator;
    using CacheList = std::shared_ptr<std::list<Item>>;
    using Hashmap   = std::shared_ptr<std::unordered_map<std::string, Iter>>;
    
    MyEngine();
    ~MyEngine();


/*
 * 键值对的存储
 * 发现redis的内部并没有对冷数据的处理
 * 所以在设计是要对当前存储大小进行计数
 * 当以下情况发生时：
 * 使用LRU算法对冷热数据区分
 * 内存共有4GB 当数据大于4GB时
 * 1.热数据存在内存里
 * 2.冷数据放在磁盘上(计划由另一种数据库来管理)
 * 
 * 热数据的存储形式我打算以std::map
 *
 *
 *``
 * 关于数据的持久化
 * 1.RDB
 *  每隔一段时间，将数据全部存在磁盘上
 * 2.AOF
 *  维护一个队列，记录下所有的操作，恢复时就是将所有队列里边存储的命令
 *  执行一遍，需要一个锁来保持同步，防止数据错乱。
 * */

 public:   
    RetCode write (const PolarString& key, const PolarString& value) 
    
    RetCode read (const PolarString& key, const polar_string& value);

    private:
    CacheList lruCache;
    Hashmap hashmap;
    uint64_t lruCacheCurSize;
    uint64_t lruCacheMaxSzie;
}


#endif  // INCLUDE_ENGINE_H_
