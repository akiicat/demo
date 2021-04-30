#include <stdio.h>
#include <stdint.h>

/*
 * padding = (align - (offset mod align)) mod align
 * aligned = offset + padding
 *         = offset + ((align - (offset mod align)) mod align)
 */

struct MixedData
{
    /*
     * For Data1
     *
     *   offset = 0;
     *   align = 1;
     *
     *   padding = (align - (offset mod align)) mod align = 0
     *   aligned = offset + padding = 0
     *
     */
    char Data1;

    /*
     * For Data2
     *
     *   offset = 1;
     *   align = 2;
     *
     *   padding = (align - (offset mod align)) mod align = 1
     *   aligned = offset + padding = 2
     *
     */
    short Data2;

    /*
     * For Data3
     *
     *   offset = 4;
     *   align = 4;
     *
     *   padding = (align - (offset mod align)) mod align = 0
     *   aligned = offset + padding = 4
     *
     */
    int Data3;

    /*
     * For Data2
     *
     *   offset = 8;
     *   align = 1;
     *
     *   padding = (align - (offset mod align)) mod align = 0
     *   aligned = offset + padding = 8
     *
     */
    char Data4;
};

int main() {
    
    uint32_t arr[6] = { 0 };

    printf("%lu\n", sizeof(struct MixedData)); // 12
    
    struct MixedData a;
    
    a.Data1 = 0x1A;
    a.Data2 = 0x2BBB;
    a.Data3 = 0x3CCCCCCC;
    a.Data4 = 0x4D;
    
    memcpy(arr, &a, sizeof(struct MixedData));
    
    // output: 2bbb7f1a 3ccccccc 0000554d 00000000 00000000 00000000
    for (int i = 0; i < 6; i++)
        printf("%08x ", arr[i]);
    printf("\n");
    
    return 0;
}


