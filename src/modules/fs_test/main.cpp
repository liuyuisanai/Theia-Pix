extern "C" __EXPORT int main(int argc, const char * const * const argv);

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

int
main(int argc, const char * const * const argv)
{
    const char *filename = "/fs/microsd/test.dat";
    uint32_t blockSize = 1024;
    uint32_t totalSize = 0;
    time_t startTime;
    time_t now;
    uint32_t delta;
    bool doWrite = true;
    bool doRead = true;
    int fd = -1;

    for (int i = 0; i < argc; i++)
    {
        if (strcmp("-ro", argv[i]) == 0)
        {
            doWrite = false;
        }
        if (strcmp("-wo", argv[i]) == 0)
        {
            doRead = false;
        }

        if (strcmp("-f", argv[i]) == 0)
        {
            i++;
            if (i < argc)
            {
                filename = argv[i];
            }
        }
        if (strcmp("-bs", argv[i]) == 0)
        {
            i++;
            if (i < argc)
            {
                blockSize = atoi(argv[i]);
            }
        }
        if (strcmp("-ts", argv[i]) == 0)
        {
            i++;
            if (i < argc)
            {
                totalSize = atoll(argv[i]);
            }
        }
    }

    if (totalSize == 0)
    {
        totalSize = blockSize * 10;
    }

    char *buf = new char[blockSize];
    for (int i = 0; i < blockSize; i++)
    {
        char c = '0' + (i % ('~' - '0'));
        buf[i] = c;
    }


    if (doWrite)
    {
        printf("open write file name: %s block size: %d total size %d\n", filename, blockSize, totalSize);
        time(&startTime);

        fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC);

        if (fd == -1)
        {
            printf("Error: open failed");
            return -1;
        }

        printf("writing to file %d bytes\n", totalSize);

        uint32_t sizeWrittenTotal = 0;

        while (sizeWrittenTotal < totalSize)
        {
            uint32_t bytesToWrite = totalSize - sizeWrittenTotal;

            if (bytesToWrite > blockSize)
            {
                bytesToWrite = blockSize;
            }

            uint32_t bytesWritten = write(fd, buf, bytesToWrite);
            //printf("bytesWritten %d\n", bytesWritten);
            //printf("bytesToWrite %d\n", bytesToWrite);

            if (bytesWritten != bytesToWrite)
            {
                printf("Error: bytesWritten %d bytesToWrite %d sizeWrittenTotal %d\n",
                       bytesWritten, bytesToWrite, sizeWrittenTotal);
                return -1;
            }

            sizeWrittenTotal += bytesWritten;
            //printf("sizeWrittenTotal %d\n", sizeWrittenTotal);
        }

        close(fd);
        time(&now);

        //printf("%d %d", (int)now, (int)startTime);

        delta = now - startTime;

        printf("write done for %d seconds\n", delta);
    }

    if (doRead)
    {
        printf("open read file name: %s block size: %d\n", filename, blockSize);

        time(&startTime);

        fd = open(filename, O_RDONLY);

        if (fd == -1)
        {
            printf("Error: open failed");
            return -1;
        }

        printf("reading from file %d bytes\n", totalSize);

        uint32_t sizeReadedTotal = 0;

        while (sizeReadedTotal < totalSize)
        {
            uint32_t bytesToRead = totalSize - sizeReadedTotal;

            if (bytesToRead > blockSize)
            {
                bytesToRead = blockSize;
            }

            uint32_t bytesReaded = read(fd, buf, bytesToRead);
            //printf("bytesReaded %d\n", bytesReaded);
            //printf("bytesToRead %d\n", bytesToRead);

            if (bytesReaded != bytesToRead)
            {
                printf("Error: bytesReaded %d bytesToRead %d sizeReadedTotal %d\n",
                       bytesReaded, bytesToRead, sizeReadedTotal);
                return -1;
            }

            for (int i = 0; i < blockSize; i++)
            {
                char c = '0' + (i % ('~' - '0'));

                if (buf[i] != c)
                {
                    printf("Error: buf[%d] %d != %d\n", i, buf[i], c);
                    return -1;
                }
            }

            sizeReadedTotal += bytesReaded;
        }
            //printf("sizeReadedTotal %d\n", sizeReadedTotal);

        close(fd);

        time(&now);

        delta = now - startTime;

        printf("read done for %d seconds\n", delta);

    }


    delete[] buf;

    return 0;
}
