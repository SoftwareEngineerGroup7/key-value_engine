/*************************************************************************
	> File Name: engine.cpp
	> Author: 
	> Mail: 
	> Created Time: 2018年11月15日 星期四 17时15分53秒
 ************************************************************************/
#include "engine.h" 

using namespace polar_race;
// 这块是题目给的代码 我得到想法是将他修改掉
// 就是一个构造函数的感觉就可以了
static const char Aof[] = "/tmp/Aof";

MyEngine::MyEngine()
{
    //创建LRU链表 智能指针
    lruCache = std::make_shared<std::list<Item>>();
    //hashmap
 //   hashmap = std::make_shared<std::unordered_map<PolarString, Iter>>();
    //需要重写哈希函数 和
    //make_shared 与 shared_ptr 的问题 。
    // shared_ptr  初值给null
    hashmap = std::make_shared<std::unordered_map<PolarString,
                                                    Iter,
                                                    HashFunc>>();

    lruCacheCurSize = 0;
    lruCacheMaxSize = 10000;

}

RetCode Open (const std::string& name, polar_race::Engine** eptr)

{  
    return kSucc;
}
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
 * 关于数据的持久化
 * 1.RDB
 *  每隔一段时间，将数据全部存在磁盘上
 * 2.AOF
 *  维护一个队列，记录下所有的操作，恢复时就是将所有队列里边存储的命令
 *  执行一遍，需要一个锁来保持同步，防止数据错乱。
 * */
RetCode 
MyEngine::Write (const PolarString& key, const PolarString& value) 
{
    //存数据的
    //setCommand();

    //对写指针加锁就ok

   // std::mutex mutex;
  //  std::lock_guard<std::mutex> lock(mutex);
    /*
     * 如过当前的size小于最大size 则不用淘汰
     * 大于则需要淘汰
     */

    auto item = std::make_pair(key,value);

    if (lruCacheCurSize < lruCacheMaxSize)
    {
        lruCacheCurSize++;
        lruCache->insert (lruCache->begin (), item);
        // 智能指针 shared_ptr 声明时初值给null 
        hashmap->emplace (key, lruCache->begin()); 
    }
    else
    {
        //这里是会产生 迭代器失效的问题 需要之后在来思考一下
        /*需要擦除不常用的*/
        lruCache->erase (--lruCache->end ());
        lruCache->insert (lruCache->begin (), item);
        /*更新数*/
        if (hashmap->find (key) != hashmap-> end())
        {
            (*hashmap)[key] = lruCache->begin();
        }
        /*插入队首*/
        else
        {
            hashmap->insert({key, lruCache->begin()});
        }
    }

    //在这里拿到文件锁的写指针
    //P(ptr)
    //    str = ptr;
    //    ptr+=len;
    //V(ptr)

    std::ofstream ofile;               //定义输出文件
    //ofile.open("./tmp/Aof",std::ios::app);     //作为输出文件打开
    ofile.open(Aof,std::ios::app);     //作为输出文件打开
    
    ofile<<key.ToString()<< "\r\n"<<std::endl;   //标题写入文件
    //首先将此命令转化为协议格式
    
    //写入文件中
    
    ofile.close();


    return kSucc;
}

RetCode 
MyEngine::Read(const PolarString& key, std::string* value)
{
    if (hashmap->find(key) != hashmap->end())
    {
        //
        auto iter = (*hashmap)[key];
        auto item = *iter;

        /*删除节点时 当前迭代器会失效
         * 其余迭代器不变
         * 先擦掉 然后放在队首
         * 那我觉得是不是这里需要加一个判断呢
         * 如果是首位那就不需要删除
         *
         */
        lruCache->erase(iter);
        lruCache->insert(lruCache->begin(), (item));
        (*hashmap)[key] = lruCache->begin();

        *value = (*(*hashmap)[key]).second.ToString();

    }
    else 
    {
        *value = nullptr; 
    }
    
    return kSucc;
}
