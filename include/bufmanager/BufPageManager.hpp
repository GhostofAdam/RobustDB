#ifndef BUF_PAGE_MANAGER
#define BUF_PAGE_MANAGER
#include "FindReplace.hpp"
#include "fileio/FileManager.hpp"
#include "utils/MultiList.hpp"
/*
 * BufPageManager
 * 实现了一个缓存的管理器
 */
struct BufPageManager {
public:
	int last;
	FileManager* fileManager;
	MyHashMap* hash;	// 缓存
	MultiList *list;
	FindReplace* replace;
	//MyLinkList* bpl;
	bool* dirty;		// 脏页数组
	/*
	 * 缓存页面数组
	 */
	BufType* addr;		// 缓存页面数组 unsigned int*
	BufType allocMem() {
		return new unsigned int[(PAGE_SIZE >> 2)];	// 2KB
	}
	BufType getBuf(int index) {
		return addr[index];
	}
	int fetchPage(int fileID, int pageID) {
		BufType b;
		int index = replace->find();	// 缓存页面数组中要被替换页面的下标
		b = addr[index];
		if (b == NULL) {
			b = allocMem();	// 指向2KB unsigned int数组
			addr[index] = b;
		} else {
			if (dirty[index]) {
				int k1, k2;
				hash->getKeys(index, k1, k2);	// hash表的k1、k2两个键
				fileManager->writePage(k1, k2, b);	// 更新回文件
				dirty[index] = false;
			}
		}
		list->insert(fileID, index);
		hash->replace(index, fileID, pageID);
		return index;
	}
public:
	/*
	 * @函数名allocPage
	 * @参数fileID:文件id，数据库程序在运行时，用文件id来区分正在打开的不同的文件
	 * @参数pageID:文件页号，表示在fileID指定的文件中，第几个文件页
	 * @参数index:函数返回时，用来记录缓存页面数组中的下标
	 * @参数ifRead:是否要将文件页中的内容读到缓存中
	 * 返回:缓存页面的首地址
	 * 功能:为文件中的某一个页面获取一个缓存中的页面
	 *      缓存中的页面在缓存页面数组中的下标记录在index中
	 *      并根据ifRead是否为true决定是否将文件中的内容写到获取的缓存页面中
	 * 注意:在调用函数allocPage之前，调用者必须确信(fileID,pageID)指定的文件页面不存在缓存中
	 *      如果确信指定的文件页面不在缓存中，那么就不用在hash表中进行查找，直接调用替换算法，节省时间
	 */
	int allocPage(int fileID, int pageID, bool ifRead = false) {
		int index = fetchPage(fileID, pageID);
		BufType b = getBuf(index);
		if (ifRead) {
			fileManager->readPage(fileID, pageID, b);
		}
		return index;
	}
	/*
	 * @函数名getPage
	 * @参数fileID:文件id
	 * @参数pageID:文件页号
	 * @参数index:函数返回时，用来记录缓存页面数组中的下标
	 * 返回:缓存页面的首地址
	 * 功能:为文件中的某一个页面在缓存中找到对应的缓存页面
	 *      文件页面由(fileID,pageID)指定
	 *      缓存中的页面在缓存页面数组中的下标记录在index中
	 *      首先，在hash表中查找(fileID,pageID)对应的缓存页面，
	 *      如果能找到，那么表示文件页面在缓存中
	 *      如果没有找到，那么就利用替换算法获取一个页面
	 */
	int getPage(int fileID, int pageID) {
		int index = hash->findIndex(fileID, pageID);
		if (index != -1) {
			access(index);
		} else {
			int index = fetchPage(fileID, pageID);
			BufType b = getBuf(index);
			fileManager->readPage(fileID, pageID, b);
		}
		return index;
	}
	/*
	 * @函数名access
	 * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
	 * 功能:标记index代表的缓存页面被访问过，为替换算法提供信息
	 */
	BufType* access(int index) {
		if (index == last) {
			return nullptr;
		}
		replace->access(index);
		last = index;
		return addr + index * PAGE_SIZE;
	}
	/*
	 * @函数名markDirty
	 * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
	 * 功能:标记index代表的缓存页面被写过，保证替换算法在执行时能进行必要的写回操作，
	 *           保证数据的正确性
	 */
	void markDirty(int index) {
		dirty[index] = true;
		access(index);
	}
	/*
	 * @函数名release
	 * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
	 * 功能:将index代表的缓存页面归还给缓存管理器，在归还前，缓存页面中的数据不标记写回
	 */
	void release(int index) {
		dirty[index] = false;
		replace->free(index);
		hash->remove(index);
	}
	/*
	 * @函数名writeBack
	 * @参数index:缓存页面数组中的下标，用来表示一个缓存页面
	 * 功能:将index代表的缓存页面归还给缓存管理器，在归还前，缓存页面中的数据需要根据脏页标记决定是否写到对应的文件页面中
	 */
	void writeBack(int index) {
		if (dirty[index]) {
			int f, p;
			hash->getKeys(index, f, p);
			fileManager->writePage(f, p, addr[index]);
			dirty[index] = false;
		}
		replace->free(index);
		hash->remove(index);
	}
	/*
	 * @函数名close
	 * 功能:将所有缓存页面归还给缓存管理器，归还前需要根据脏页标记决定是否写到对应的文件页面中
	 */
	void close() {
		for (int i = 0; i < CAP; ++ i) {
			writeBack(i);
		}
	}
	/*
	 * @函数名getKey
	 * @参数index:缓存页面数组中的下标，用来指定一个缓存页面
	 * @参数fileID:函数返回时，用于存储指定缓存页面所属的文件号
	 * @参数pageID:函数返回时，用于存储指定缓存页面对应的文件页号
	 */
	void getKey(int index, int& fileID, int& pageID) {
		hash->getKeys(index, fileID, pageID);
	}
	/*
	 * 构造函数
	 * @参数fm:文件管理器，缓存管理器需要利用文件管理器与磁盘进行交互
	 */
	BufPageManager() {
		int c = CAP;	// 缓存中页面个数上线
		int m = MOD;	// hash算法的模
		last = -1;
		fileManager = new FileManager();
		dirty = new bool[CAP];
		addr = new BufType[CAP];	// unsigned int*
		hash = new MyHashMap(c, m);
		list = new MultiList(CAP, MAX_FILE_NUM);
	    replace = new FindReplace(c);
		for (int i = 0; i < CAP; ++ i) {
			dirty[i] = false;
			addr[i] = NULL;
		}
	}
    ~BufPageManager() {
        delete replace;
        delete hash;
        delete list;
        delete fileManager;
        delete addr;
    }
	void closeFile(int fileID, bool ifWrite = true) {
        int index;	
        while (!list->isHead(index = list->getFirst(fileID))) {
            if (ifWrite) {
                writeBack(index);
            } else {
                release(index);
            }
        }
    }
	
	static BufPageManager &getInstance() {
        static BufPageManager instance;
        return instance;
    }
	static FileManager &getFileManager() {
        return *(getInstance().fileManager);
    }
};
#endif
