#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>

#include "NetworkService.h"
#include "Tool.h"
using namespace std;

// 指向唯一的工具类实例，只在main函数结束前delete
extern Tool *pUtils;
// 指向唯一的模拟网络环境类实例，只在main函数结束前delete
extern NetworkService *pns;

#endif