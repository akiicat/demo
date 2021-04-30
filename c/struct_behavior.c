#include <stdio.h>
#include <stdint.h>

struct point_t {
    int x;
    int y;
};

int main() {

    int arr[] = { 0x1, 0x2, 0x3, 0x4, 0x5, 0x6 };

    struct point_t *p1, *p2; // (1,2)

    /* casting int array to struct point */
    /* expect output: (1,2) */
    p1 = (struct point_t*) arr;
    printf("(%d,%d)\n", p1->x, p1->y);
    
    /* shift struct point by 1 */
    /* expect output: (3,4) */
    p1 = p1 + 1;
    printf("(%d,%d)\n", p1->x, p1->y);

    /* update struct point element */
    /* expect output: 1,2,8,4,5,6 */
    p1->x = 8;
    for (int i = 0; i < 6; i++)
        printf("%d,", arr[i]);
    printf("\n");
    
    /* struct value copy from p2 to p1 */
    /* expect output: 8,4,8,4,5,6 */
    p1 = (struct point_t*) arr; // (1,2)
    p2 = (struct point_t*) (arr + 2); // (8,4)
    *p1 = *p2;
    for (int i = 0; i < 6; i++)
        printf("%d,", arr[i]);
    printf("\n");

    return 0;
}

