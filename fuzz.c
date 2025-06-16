#include <stdint.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>

/* FIXME: Add includes of project to fuzz here */

char const * FOLDER_DEFAULT = "CORPUS";
char const * FILE_DEFAULT = "corpus";


/*
 * Dummy fuzzed function; creates an overflow
 */
int testPasswordOverflow(uint8_t const *Data, size_t Size);


// helper functions
uint8_t const *zeroTerminate(const uint8_t *Data, size_t Size); // If fuzzed function accepts a zero-terminated byte sequence
char const *writeBufferToFile(const uint8_t *Data, size_t Size); // If fuzzed function accepts a file name as input
FILE *getFileHandleFromBuffer(const uint8_t *Data, size_t Size); // If fuzzed function accepts a FILE* as input; fclose() returned handle after use


// FIXME: Replace call to dummy function below with call into fuzz project
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    testPasswordOverflow(Data, Size);
    return 0;
}

#ifdef COVERAGE
// Generates a coverage report on the files in the CORPUS folder
int main(int argc, char *argv[])
{
    uint8_t buf[1024 * 1024];
    char const *folderPath = FOLDER_DEFAULT;
    if (argc > 1)
    {
        folderPath = argv[1];
    }

    struct dirent *entry;
    DIR *dir = opendir(folderPath);

    if (dir == NULL) 
    {
        fprintf(stderr, "Could not open directory %s.\n", folderPath);
        return 1;
    }

    int numFiles = 0;
    while ((entry = readdir(dir)) != NULL) 
    {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, ".."))
        {
            char filePath[256] = "";
            strncpy(filePath, folderPath, strlen(folderPath));
            strcat(filePath, "/");
            strcat(filePath, entry->d_name);

            if ((numFiles % 100) == 0)
            {
                printf(".");
                fflush(stdout);
            }

            FILE *fp = fopen(filePath, "rb");

            if (fp == NULL)
            {
                fprintf(stderr, "Could not open file %s.\n", filePath);
                return 1;                
            }

            fseek(fp, 0, SEEK_END);
            int fileSize = (int)ftell(fp);
            rewind(fp);  // Reset position to beginning

            if (fileSize > sizeof(buf))
            {
                fprintf(stderr, "File %s too long: %d bytes.\n", filePath, fileSize);
                fclose(fp);
                return 1;                     
            }

            fread(buf, 1, fileSize, fp);
            fclose(fp);

            LLVMFuzzerTestOneInput(buf, fileSize);
            numFiles++;
        }
    }

    printf("\n");
    closedir(dir);
    return 0;
}
#endif

int testPasswordOverflow(uint8_t const *Data, size_t Size)
{
    int ret = 1;
    static uint8_t const pw[] = { 0xab, 0xcd, 0xef };
    if (Size == 3)
    {
        if (Data[0] == 0xab)
        {
            if (Data[1] == 0xcd)
            {
                if (Data[2] == 0xef)
                {
                    // Buffer overflow here
                    if (Data[3] == 0x12)
                    {
                        ret = 0;
                    }
                }
            }
        }
    }
    return ret;
}

uint8_t const *zeroTerminate(const uint8_t *Data, size_t Size)
{
    static uint8_t buf[4096];
    size_t len = Size > 4095 ? 4095 : Size;
    /* right align on the buffer to find overflows early */
    uint8_t *pRet = &buf[4095 - len];
    memcpy(pRet, Data, len);
    buf[4095] = '\0';
    return pRet;
}

char const *writeBufferToFile(const uint8_t *Data, size_t Size)
{
    FILE *fp = fopen(FILE_DEFAULT, "wb");
    fwrite(Data, 1, Size, fp);
    fclose(fp);
    return FILE_DEFAULT;
}

FILE *getFileHandleFromBuffer(const uint8_t *Data, size_t Size)
{
    char const *fileName = writeBufferToFile(Data, Size);
    FILE *pf = fopen(fileName, "rb");
    return pf;
}
