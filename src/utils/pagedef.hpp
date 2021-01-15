#ifndef PAGE_DEF
#define PAGE_DEF
#ifdef __cplusplus
extern "C" {
#endif

#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifdef __cplusplus
}
#endif


//----------------------FILE---------------------------------------
#define PAGE_SIZE 8192
#define PAGE_IDX 13
#define MAX_FILE_NUM 128
#define BUF_CAPACITY 60000
#define PAGE_FOOTER_SIZE 64
#define MAX_REC_PER_PAGE 512


//----------------------TABLE--------------------------------------
#define MAX_COLUMN_SIZE 8
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

/*
 * hash算法的模
 */
#define MOD 60000
#define IN_DEBUG 0
#define DEBUG_DELETE 0
#define DEBUG_ERASE 1
#define DEBUG_NEXT 1
/*
 * 一个表中列的上限
 */
#define MAX_COL_NUM 31
/*
 * 数据库中表的个数上限
 */
#define MAX_TB_NUM 31
#define RELEASE 1
typedef unsigned int* BufType;
typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned long long ull;
typedef long long ll;
typedef double db;
typedef int INT;
typedef int(cf)(uchar*, uchar*);
// int current = 0;
// int tt = 0;
#endif