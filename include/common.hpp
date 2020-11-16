#ifndef COMMON
#define COMMON
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <queue>
#include <math.h>
#include <cmath>
#include <assert.h>
#include <map>

using std::cout;
using std::endl;
using std::cin;
using std::vector;
using std::string;
using std::ostringstream;
using std::ifstream;
using std::ofstream;
using std::ios;
using std::queue;
using std::ceil;
//----------------------TABLE--------------------------------------
#define MAX_COLUMN_SIZE 32
// both table name and column name
#define MAX_NAME_LEN 128
#define MAX_DATA_SIZE 3000
#define MAX_CHECK 16
#define MAX_FOREIGN_KEY 24

//----------------------Database-----------------------------------------
#define MAX_TABLE_SIZE 32

#define DATE_FORMAT "%Y-%m-%d"

using RID_t = unsigned int;
#define UNUSED(x) (void)(x)
#endif
using std::map;
