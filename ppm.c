/*
Francesco Falcon Sm3201408
 */
#include "ppm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

static size_t ppm_header(char* buf, size_t buflen, int w, int h){
    return (size_t)snprintf(buf, buflen, "P6\n%d %d\n255\n", w, h);
}

int ppm_write_mmap(const char* path, int width, int height, const rgb24* data){
    if(!path || width<=0 || height<=0 || !data) return 1;
    const size_t headerMax = 64;
    char header[headerMax];
    size_t hlen = ppm_header(header, headerMax, width, height);
    const size_t pixBytes = (size_t)width * (size_t)height * 3u;
    const size_t total = hlen + pixBytes;

#ifdef _WIN32
    HANDLE hFile = CreateFileA(path, GENERIC_WRITE|GENERIC_READ, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if(hFile==INVALID_HANDLE_VALUE){ fprintf(stderr, "CreateFile fallita\n"); return 2; }
    LARGE_INTEGER li; li.QuadPart = (LONGLONG)total;
    if(!SetFilePointerEx(hFile, li, NULL, FILE_BEGIN) || !SetEndOfFile(hFile)){
        fprintf(stderr, "SetEndOfFile fallita\n"); CloseHandle(hFile); return 3; }
    HANDLE hMap = CreateFileMappingA(hFile, NULL, PAGE_READWRITE, 0, 0, NULL);
    if(!hMap){ fprintf(stderr, "CreateFileMapping fallita\n"); CloseHandle(hFile); return 4; }
    unsigned char* mem = (unsigned char*)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if(!mem){ fprintf(stderr, "MapViewOfFile fallita\n"); CloseHandle(hMap); CloseHandle(hFile); return 5; }

    memcpy(mem, header, hlen);
    memcpy(mem+hlen, data, pixBytes);

    FlushViewOfFile(mem, 0);
    UnmapViewOfFile(mem);
    CloseHandle(hMap);
    CloseHandle(hFile);
#else
    int fd = open(path, O_CREAT|O_RDWR, 0644);
    if(fd<0){ perror("open ppm"); return 2; }
    if(ftruncate(fd, (off_t)total)!=0){ perror("ftruncate ppm"); close(fd); return 3; }
    unsigned char* mem = (unsigned char*)mmap(NULL, total, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if(mem==MAP_FAILED){ perror("mmap ppm"); close(fd); return 4; }

    memcpy(mem, header, hlen);
    memcpy(mem+hlen, data, pixBytes);

    msync(mem, total, MS_SYNC);
    munmap(mem, total);
    close(fd);
#endif
    return 0;
}
