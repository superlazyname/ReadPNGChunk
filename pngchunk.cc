#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdint.h>
#include <cstdlib>

struct Int32Vec2
{
    int32_t X, Y;
};

struct OffsetResult
{
    Int32Vec2 Offset;
    bool Valid;
};

constexpr size_t TokenLength = 4;
constexpr size_t OffsetFieldLength = 4;

inline int32_t BufferToInt32(const char* buffer)
{
    int32_t int32 = (buffer[0] << 24) | (buffer[1] << 16) | (buffer[2] << 8) | buffer[3];

    return int32;
}

inline bool CompareToken(const char* buffer, const char* toFind)
{
    int result = memcmp(buffer, toFind, TokenLength);

    bool matches = (result == 0);

    return matches;
}

OffsetResult GetOffsetFromFile(const char* filePath)
{
    FILE* file = fopen(filePath, "rb");

    // skip the first 33 bytes of the file 
    fseek(file, 33, SEEK_SET);

    OffsetResult result = {0, 0, false};
    
    // I guess it's not a big deal to just read through the whole file in the (unlikely, in my case) chance the field is missing,
    // my files are really small.
    while(true)
    {
        char buffer[TokenLength] = {0};
        size_t readCount = fread(&buffer[0], TokenLength, 1, file);

        // where 1 means "1 set of 4 bytes"
        if(readCount != 1)
        {
            // this is usually the error you get when you attempt to read a png that doesn't have the chunk
            printf("fread returned non-4 byte chunk, stopping read\n");
            break;
        }

        if(CompareToken(&buffer[0], "grAb"))
        {
            printf("Found grAB field\n");
            char offsetXBuffer[OffsetFieldLength] = {0};
            char offsetYBuffer[OffsetFieldLength] = {0};
            size_t xReadCount = fread(&offsetXBuffer[0], OffsetFieldLength, 1, file);

            if(xReadCount != 1)
            {
                printf("Error: Incomplete X offset field read.\n");
                break;
            }

            size_t yReadCount = fread(&offsetYBuffer[0], OffsetFieldLength, 1, file);

            if(yReadCount != 1)
            {
                printf("Error: Incomplete Y offset field read.\n");
                break;
            }

            // you probably want to also check the CRC, I didn't do that here

            int32_t offsetX = BufferToInt32(&offsetXBuffer[0]);
            int32_t offsetY = BufferToInt32(&offsetYBuffer[0]);

            result = {offsetX, offsetY, true};   
                     
            break;
        }

    }

    fclose(file);

    return result;
}

int main()
{
    // this image has an offset of (12345, 12346)
    printf("Attempting to extract the offset from test.png\n");

    OffsetResult result = GetOffsetFromFile("HasOffset.png");

    if(result.Valid)
    {
        printf("Result valid, (%d, %d)\n", result.Offset.X, result.Offset.Y);
    }
    else
    {
        printf("Unable to find offset.\n");
    }

    printf("\n");

    // this file has no grAb offset field
    OffsetResult result2 = GetOffsetFromFile("NoOffset.png");

    printf("Attempting to extract the offset from NoOffset.png\n");

    if(result2.Valid)
    {
        printf("Result valid, (%d, %d)\n", result2.Offset.X, result2.Offset.Y);
    }
    else
    {
        printf("Unable to find offset.\n");
    }

    return 0;
}