/*****************************************************************************

	 IMPORTANT: You *must* use palloc0 and pfree, not malloc and free, in your
	 implementation.  This will allow your filter to integrate into PostgreSQL.

******************************************************************************/

#include "postgres.h"
#include "utils/cmsketch.h"

/* initialize the count-min sketch for the specified width and depth */
cmsketch* init_sketch(uint32 width, uint32 depth) {
    struct cmsketch *ret;
    uint32_t i,j;
    ret = (cmsketch*)palloc(sizeof(cmsketch));
    if (ret == NULL)
        return NULL;
    ret->depth = depth;
    ret->width = width;
    ret->array = palloc(sizeof(uint32_t *) * depth);
    for(i=0;i<depth;i++)
        ret->array[i] = palloc(sizeof(uint32_t) * width);
    for(i=0; i<depth;i++)
        for(j=0;j<width;j++)
            ret->array[i][j] = 0;
    return ret;
}

/* increment 'bits' in each sketch by 1. 
 * 'bits' is an array of indices into each row of the sketch.
 *    Thus, each index is between 0 and 'width', and there are 'depth' of them.
 */
void increment_bits(cmsketch* sketch, uint32 *bits) {
    uint32_t i;
    for(i=0;i<sketch->depth;i++)
        sketch->array[i][bits[i]]++;
}

/* decrement 'bits' in each sketch by 1.
 * 'bits' is an array of indices into each row of the sketch.
 *    Thus, each index is between 0 and 'width', and there are 'depth' of them.
 */
void decrement_bits(cmsketch* sketch, uint32 *bits) {
    uint32_t i;
    for(i=0;i<sketch->depth;i++)
        sketch->array[i][bits[i]]--;
}

/* return the minimum among the indicies pointed to by 'bits'
 * 'bits' is an array of indices into each row of the sketch.
 *    Thus, each index is between 0 and 'width', and there are 'depth' of them.
 */
uint32 estimate(cmsketch* sketch, uint32 *bits) {
    uint32_t i;
    uint32_t min = sketch->array[1][bits[1]];
    for(i=1;i<sketch->depth;i++)
        if (min > sketch->array[i][bits[i]])
            min = sketch->array[i][bits[i]];
    return min;
}

/* set all values in the sketch to zero */
void reset_sketch(cmsketch* sketch) {
    int i,j;
    for(i=0;i<sketch->depth;i++)
        for(j=0;j<sketch->width;j++)
            sketch->array[i][j]=0;
    sketch->depth = 0;
    sketch->width = 0;
}

/* destroy the sketch, freeing any memory it might be using */
void destroy_sketch(cmsketch* sketch) {
    int i;
    for(i=0;i<sketch->depth;i++)
        pfree(sketch->array[i]);
    pfree(sketch->array);
    pfree(sketch);
    sketch = NULL;
}
