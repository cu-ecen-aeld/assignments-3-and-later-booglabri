/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

#include "aesd-circular-buffer.h"

#define INCOFFS(off) (off +  1) % AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED

// Debug macros
#include <stdio.h>
//#define DEBUG(msg, ...)
#define DEBUG(msg, ...) fprintf(stderr, "DEBUG: " msg, ##__VA_ARGS__)

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
    /**
    * TODO: implement per description
    */
    size_t accum_offset = 0;
    int i = buffer->out_offs;
    int c = 0;
    
    DEBUG("in: %d  char_offset: %ld\n", buffer->out_offs, char_offset);
    do {
        accum_offset += buffer->entry[i].size;
        DEBUG("%d %ld %ld %ld %d\n",i,accum_offset,char_offset,accum_offset - char_offset, c);
        if (char_offset >= accum_offset) i = INCOFFS(i);
        c++;
    } while (accum_offset <= char_offset);
    DEBUG("out: %d %ld %ld %ld %d\n\n",i,accum_offset,char_offset,char_offset - (accum_offset - buffer->entry[i].size), c);

    *entry_offset_byte_rtn = char_offset - (accum_offset - buffer->entry[i].size);
    return &buffer->entry[i];
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    /**
    * TODO: implement per description
    */
    DEBUG("insert: %d %d %d -> ", buffer->in_offs, buffer->out_offs, buffer->full);
    if (buffer->full) buffer->out_offs = INCOFFS(buffer->out_offs);
    buffer->entry[buffer->in_offs] = *add_entry;
    DEBUG("|%s|",buffer->entry[buffer->in_offs].buffptr);
    if ((buffer->in_offs = INCOFFS(buffer->in_offs)) == buffer->out_offs) buffer->full = true;
    else buffer->full = false;
    DEBUG("%d %d %d\n", buffer->in_offs, buffer->out_offs, buffer->full);
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
