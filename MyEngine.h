/*************************************************************************
	> File Name: myengine.h
	> Author: 
	> Mail: 
	> Created Time: 2018年11月17日 星期六 12时12分58秒
 ************************************************************************/

#ifndef _MYENGINE_H
#define _MYENGINE_H

#include "Engine.h"

// 继承Engine
class MyEngine:public Engine
{
 //计划将LRU定义在这里
 public:
  
 private:
  /*总计4GB数据*/
  unsigned long long int allocation; 
 
}

#endif
