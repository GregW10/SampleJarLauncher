//
// Created by Gregor Hartl Watters on 03/09/2022.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef _WIN32
#include <shlobj.h>
#else
#include <sys/stat.h>
#include <pwd.h>
#endif

#ifndef errno
extern int errno;
#endif

typedef enum {
    false, true
} bool;

#ifdef _WIN32
size_t wcslen_c(const wchar_t *str) {
    if (str == NULL || *str == 0) {
        return 0;
    }
    size_t count = 0;
    while (*str++) ++count;
    return count;
}

size_t wcscat_c(wchar_t *strDest, const wchar_t *strSource) {
    if (strDest == NULL || strSource == NULL || *strSource == 0) {
        return 0;
    }
    size_t count = 0;
    if (*strDest == 0) {
        goto start;
    }
    while (*++strDest);
    start:
    while ((*strDest++ = *strSource++)) ++count;
    return count;
}
#else
size_t strlen_c(const char *str) {
    if (str == NULL || *str == 0) {
        return 0;
    }
    size_t count = 0;
    while (*str++) ++count;
    return count;
}

size_t strcat_c(char *strDest, const char *strSource) {
    if (strDest == NULL || strSource == NULL || *strSource == 0) {
        return 0;
    }
    size_t count = 0;
    if (*strDest == 0) {
        goto start;
    }
    while (*++strDest);
    start:
    while ((*strDest++ = *strSource++)) ++count;
    return count;
}
#endif
void memset_c(void *str, size_t num, char c) {
    char *ptr = (char *) str;
    while (num --> 0) {
        *ptr++ = c;
    }
}

#ifdef _WIN32
bool move_file(const wchar_t *src, const wchar_t *dst) {
#else
bool move_file(const char *src, const char *dst) {
#endif
    if (src == NULL || dst == NULL || *src == 0 || *dst == 0) {
        return false;
    }
    FILE *f;
    FILE *fp;
#ifdef _WIN32
    f = _wfopen(src, L"rb");
    fp = _wfopen(dst, L"wb");
#else
    f = fopen(src, "rb");
    fp = fopen(dst, "wb");
#endif
    if (f == NULL || fp == NULL) {
        return false;
    }
    int c;
    while ((c = fgetc(f)) != EOF) {
        fputc(c, fp);
    }
    fclose(f);
    fclose(fp);
#ifdef _WIN32
    DeleteFileW(src);
#else
    remove(src);
#endif
    return true; // returns true even if remove() or DeleteFileW fail - as long as file has been copied
}

int main() {
#ifdef _WIN32
    wchar_t *homePath;
    homePath = malloc(MAX_PATH*sizeof(wchar_t));
    if (SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, SHGFP_TYPE_CURRENT, homePath) != S_OK) {
        MessageBoxA(NULL, "Home directory path could not be determined.", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return -1;
    }
#else
    char *homePath;
    struct passwd *pwd = getpwuid(getuid());
    if (pwd == NULL) {
        fprintf(stderr, "Home directory path could not be determined.\n");
        perror("Error");
        return -1;
    }
    homePath = pwd->pw_dir;
#endif
#ifdef _WIN32
    wchar_t *fullPath;
    fullPath = malloc((wcslen_c(homePath) + 19)*sizeof(wchar_t));
    *fullPath = 0;
    wcscat_c(fullPath, homePath);
    free(homePath);
    wcscat_c(fullPath, L"\\MovieWishList.jar");
    unsigned calc = (14 + wcslen_c(fullPath))*sizeof(wchar_t);
    wchar_t *fullCommand = malloc(calc);
    memset_c(fullCommand, calc, 0);
    wcscat_c(fullCommand, L"javaw -jar \"");
    wcscat_c(fullCommand, fullPath);
    wcscat_c(fullCommand, L"\"");
#else
    char *fullPath;
    fullPath = malloc((strlen_c(homePath) + 19)*sizeof(char));
    *fullPath = 0;
    wcscat_c(fullPath, homePath);
    free(homePath);
    strcat_c(fullPath, L"\\MovieWishList.jar");
    unsigned calc = (14 + strlen_c(fullPath))*sizeof(char);
    char *fullCommand = malloc(calc);
    memset_c(fullCommand, calc, 0);
    strcat_c(fullCommand, L"javaw -jar \"");
    strcat_c(fullCommand, fullPath);
    strcat_c(fullCommand, L"\"");
#endif
#ifdef _WIN32
    DWORD fileAttr = GetFileAttributesW(fullPath);
    if (fileAttr == INVALID_FILE_ATTRIBUTES) {
        if (!move_file(L"MovieWishList.jar", fullPath)) {
            MessageBoxA(NULL, ".jar file could not be moved to home directory.", "Error!", MB_ICONEXCLAMATION | MB_OK);
            return -1;
        }
    }
    else if ((fileAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) {
        MessageBoxA(NULL, "Target .jar file is a directory.", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return -1;
    }
    intptr_t retval;
    const wchar_t *const argv[] = {L"javaw", L"-jar", fullPath, NULL};
    if ((retval = _wexecvp(L"javaw", argv)) == -1) {
        MessageBoxA(NULL, "Please ensure you have the latest version of the JRE (Java Runtime Environment).",
                    "Java Error!", MB_ICONEXCLAMATION | MB_OK);
        return (int) retval;
    }
#else
    struct stat64 buff = {};
#endif
    return 0;
}