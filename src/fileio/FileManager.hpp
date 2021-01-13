#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <cstdio>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <map>
#include <fstream>
#ifdef __cplusplus
}
#endif
#include "../utils/pagedef.hpp"
#include "../backend/common.hpp"
class FileManager {
    friend class BufPageManager;
    // fileID: 区分程序运行时通过FileManager打开的所有文件(不包括关闭的文件)
    // pageID: 文件页号，对应的存储空间为文件中[pageID*8KB, (pageID+1)*8KB]
    int fileList[MAX_FILE_NUM];     // fileID --> 文件描述符
    int filePermID[MAX_FILE_NUM];   // fileID --> permID
    int idStack[MAX_FILE_NUM];      // id栈
    bool isOpen[MAX_FILE_NUM];      // 文件是否打开
    int idStackTop;
    std::map<std::string, int> permID;  // filename --> permID
    std::map<int, int> perm2temp;   // permID --> fileID
    int nextID;

    FileManager() {
        idStackTop = 0;
        /*
         * idStack[0] = 127
         * idStack[127] = 0
         * idStackTop = 128
         */
        for (int i = MAX_FILE_NUM - 1; i >= 0; i--) {
            idStack[idStackTop++] = i;
        }
        memset(isOpen, 0, sizeof(isOpen));
        nextID = 0;

        std::ifstream stm("perm.id");
        if (stm.is_open()) {    // 从文件中读入permID
            std::string tab;
            int id;
            while (stm >> tab >> id) {
                permID[tab] = id;
                if (nextID <= id) nextID = id + 1;
            }
        }
    }

    ~FileManager() {
        std::ofstream stm("perm.id");   // permID写回文件
        for (auto itm : permID) {
            stm << itm.first << " " << itm.second << "\n";
        }
    }

public:
    // 更新文件某一页的内容, buf指针指向要写入内容的首地址(unsigned int*)
    // 若pageID超过文件大小, 会对文件进行扩充
    void writePage(int fileID, int pageID, BufType buf) {
        assert(0 <= fileID && fileID < MAX_FILE_NUM && isOpen[fileID]);
        int file = fileList[fileID];
        off_t offset = pageID;
        offset <<= PAGE_SIZE_IDX;
        assert(lseek(file, offset, SEEK_SET) == offset);
        assert(write(file, (void *) buf, PAGE_SIZE) == PAGE_SIZE);
    }

    // 读取文件某一页的内容, buf表示要将文件页的内容读到哪里
    void readPage(int fileID, int pageID, BufType buf) {
        assert(0 <= fileID && fileID < MAX_FILE_NUM && isOpen[fileID]); // fileID合法且文件打开着
        int file = fileList[fileID];    // 文件描述符
        off_t offset = pageID;
        offset <<= PAGE_SIZE_IDX;   // 8192 * pageID(KB)
        assert(lseek(file, offset, SEEK_SET) == offset);    // SEEK_SET表示参数offset即为新的读写位置，lseek返回目前的读写位置
        assert(read(file, (void *) buf, PAGE_SIZE) == PAGE_SIZE);   // read返回读取到的字节数
    }

    void createFile(const char *name) {
        FILE *file = fopen(name, "a+");
        assert(file);
        fclose(file);
        permID[name] = nextID++;    // 创建唯一的permID
    }

    int openFile(const char *name) {
        assert(idStackTop);
        int fileID = idStack[--idStackTop]; // 从栈中分配一格作fileID
        isOpen[fileID] = 1;
        filePermID[fileID] = permID[name];  // fileID --> permID
        perm2temp[filePermID[fileID]] = fileID; // permID --> fileID
        int file = open(name, O_RDWR);  // 成功则返回文件描述符，否则返回-1
        assert(file != -1);
        fileList[fileID] = file;        // fileID --> 文件描述符
        return fileID;
    }

    void closeFile(int fileID) {
        assert(isOpen[fileID]);
        isOpen[fileID] = 0;
        int file = fileList[fileID];    // 文件描述符
        perm2temp.erase(filePermID[fileID]);
        ::close(file);
        idStack[idStackTop++] = fileID;
    }

    // void close() {
    //   for (int i = 0; i < MAX_FILE_NUM; i++) {
    //     if (isOpen[i]) closeFile(i);
    //     isOpen[i] = 0;
    //     idStack[idStackTop++] = i;
    //   }
    // }

    int getFilePermID(int fileID) {
        assert(isOpen[fileID]);
        return filePermID[fileID];
    }

    int getFileTempID(int permID) {
        return perm2temp[permID];
    }
};

#endif