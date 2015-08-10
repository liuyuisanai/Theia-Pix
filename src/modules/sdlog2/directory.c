#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <systemlib/err.h>

#include "directory.h"

const char sdlog2_root[] = "/fs/microsd/log";

static inline void
os_path_join2(char dst[/*PATH_MAX*/], const char path[], const char item[])
{ snprintf(dst, PATH_MAX, "%s/%s", path, item); }

int
sdlog2_filename(char filepath[/*PATH_MAX*/], const char dir[], sdlog2_file_kind_t kind)
{
    static const char * const name[SDLOG2_FILE_KIND_MAX] = {
        "log001.bin",             // SDLOG2_FILE_LOG
        "preflight_perf001.txt",  // SDLOG2_FILE_PREFLIGHT
        "postflight_perf001.txt", // SDLOG2_FILE_POSTFLIGHT
    };
    int ok = kind < SDLOG2_FILE_KIND_MAX;
    if (ok) { os_path_join2(filepath, dir, name[kind]); }
    else { *filepath = '\0'; }
    return ok;
}

uint64_t
sdlog2_dir_size_recursive(const char path[])
{
    DIR *dir;
    struct dirent *entry;
    uint64_t totalSize = 0;
    int path_len = strlen(path);

    dir = opendir(path);
    if (dir == NULL)
    {
        warnx("opendir failed");
        return 0;
    }

    for (entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
        int r = 0;
        struct stat st;
        char buf[PATH_MAX];
        int bufSize = 0;

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
           continue;
        }

        bufSize = path_len + 1 + strlen(entry->d_name) + 1;
        buf[bufSize - 1] = 0;

        os_path_join2(buf, path, entry->d_name);

        r = stat(buf, &st);

        if (r == -1)
        {
            // failed to get stat, try with next file
            continue;
        }

        if (st.st_size > 0)
        {
            totalSize += st.st_size;
        }

        // if directory then we should check sub directories
        if (S_ISDIR(st.st_mode))
        {
            totalSize += sdlog2_dir_size_recursive(buf);
        }
    }

    closedir(dir);

    return totalSize;
}

void
sdlog2_dir_remove_recursive(const char path[])
{
    DIR *dir;
    struct dirent *entry;
    int path_len = strlen(path);

    dir = opendir(path);
    if (dir == NULL)
    {
        warnx("opendir failed");
        return;
    }

    for (entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
        int r = 0;
        struct stat st;
        char buf[PATH_MAX];
        int bufSize = 0;

        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
        {
           continue;
        }

        bufSize = path_len + 1 + strlen(entry->d_name) + 1;
        buf[bufSize - 1] = 0;

        os_path_join2(buf, path, entry->d_name);

        r = stat(buf, &st);

        if (r == -1)
        {
            // failed to get stat, try with next file
            continue;
        }

        // if directory then we should check sub directories
        if (S_ISDIR(st.st_mode))
        {
            sdlog2_dir_remove_recursive(buf);
        }
        else
        {
            unlink(buf);
        }
    }

    closedir(dir);

    rmdir(path);
}

void
sdlog2_dir_remove_oldest(const char root[])
{
    DIR *dir;
    struct dirent *entry;
    int oldest_number = 0;
    char oldest_dir[PATH_MAX];

    dir = opendir(root);
    if (dir == NULL)
    {
        warnx("opendir failed");
        return;
    }

    for (entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
        int n = 0;
        char *end;

        n = strtol(entry->d_name, &end, 10);

        if (end != NULL && *end != 0 && *end != '-')
        {
            // conversion failed
            continue;
        }

        if (oldest_number == 0 || n < oldest_number)
        {
            oldest_number = n;
            os_path_join2(oldest_dir, root, entry->d_name);
        }
    }

    closedir(dir);

    if (oldest_number > 0)
    {
        warnx("remove %s\n", oldest_dir);
        sdlog2_dir_remove_recursive(oldest_dir);
    }
}

uint32_t
sdlog2_dir_find_closest_number_lt(char full_path[/*PATH_MAX*/], uint32_t limit, const char root[])
{
    uint32_t r = limit;
    DIR *dir;
    struct dirent *entry;

    *full_path = '\0';

    dir = opendir(root);
    if (dir == NULL)
    {
        perror("sdlog2_dir_find_closest_number_lt / opendir");
        return limit;
    }

    for (entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
        char *end;
        unsigned long n = strtoul(entry->d_name, &end, 10);

        if (*end != '\0' && *end != '-') { continue; }

        if (r < n && n < limit)
        {
            os_path_join2(full_path, root, entry->d_name);
            r = n;
        }
    }

    closedir(dir);

    return r;
}

int
sdlog2_dir_find_by_number(char full_path[/*PATH_MAX*/], uint32_t number, const char root[])
{
    int ok = 0;
    DIR *dir;
    struct dirent *entry;

    *full_path = '\0';

    dir = opendir(root);
    if (dir == NULL)
    {
        perror("sdlog2_dir_find_by_number / opendir");
        return 0;
    }

    for (entry = readdir(dir); entry != NULL; entry = readdir(dir))
    {
        char *end;
        unsigned long n = strtoul(entry->d_name, &end, 10);

        if (*end != '\0' && *end != '-') { continue; }

        if (n == number)
        {
            ok = 1;
            os_path_join2(full_path, root, entry->d_name);
            break;
        }
    }

    closedir(dir);

    return ok;
}
