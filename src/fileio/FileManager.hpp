#ifndef __FILE_MANAGER_H__
#define __FILE_MANAGER_H__
#include <fstream>
#include "../utils/pagedef.hpp"
#ifdef __cplusplus
extern "C" {
#endif
#include <cstdio>
#include <cassert>
#include <unistd.h>
#include <fcntl.h>
#include <map>


class FileManager {
    friend class BufPageManager;

    int fileList[MAX_FILE_NUM];
    int filePermID[MAX_FILE_NUM];
    int idStack[MAX_FILE_NUM];
    bool isOpen[MAX_FILE_NUM];
    int idStackTop;
    std::map<std::string, int> permID;
    std::map<int, int> perm2temp;
    int nextID;

    FileManager() {
        idStackTop = 0;
        for (int i = MAX_FILE_NUM - 1; i >= 0; i--) {
            idStack[idStackTop++] = i;
        }
        memset(isOpen, 0, sizeof(isOpen));
        nextID = 0;

        std::ifstream stm("perm.id");
        if (stm.is_open()) {
            std::string tab;
            int id;
            while (stm >> tab >> id) {
                permID[tab] = id;
                if (nextID <= id) nextID = id + 1;
            }
        }
    }

    ~FileManager() {
        std::ofstream stm("perm.id");
        for (auto itm : permID) {
            stm << itm.first << " " << itm.second << "\n";
        }
    }

public:
    void writePage(int fileID, int pageID, char *buf) {
        
        int file = fileList[fileID];
        off_t offset = pageID;
        offset <<= PAGE_IDX;
        int res = lseek(file , offset, SEEK_SET);
        res = write(file , (void *) buf, PAGE_SIZE);
    }

    void readPage(int fileID, int pageID, char *buf) {
        int file = fileList[fileID];
        off_t offset = pageID;
        offset <<= PAGE_IDX;
        int res = lseek(file , offset, SEEK_SET);
        res = read(file , (void *) buf, PAGE_SIZE);
    }

    void createFile(const char *name) {
        FILE *file = fopen(name, "a+");
        fclose(file);
        permID[name] = nextID++;
    }

    void closeFile(int fileID) {
        isOpen[fileID] = 0;
        int file = fileList[fileID];
        perm2temp.erase(filePermID[fileID]);
        int res = close(file);
        idStack[idStackTop++] = fileID;
    }

    int openFile(const char *name) {
        int fileID = idStack[--idStackTop];
        isOpen[fileID] = 1;
        filePermID[fileID] = permID[name];
        perm2temp[filePermID[fileID]] = fileID;
        int file = open(name, O_RDWR);
        fileList[fileID] = file;
        return fileID;
    }

    int getFilePermID(int fileID) {
        return filePermID[fileID];
    }

    int getFileTempID(int permID) {
        return perm2temp[permID];
    }
};
#ifdef __cplusplus
}
#endif
#endif
