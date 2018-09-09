//=============================================================================
// This file is part of the altEngine distribution
// (https://github.com/akw0088/altEngine/)
// Copyright (c) 2018 Alexander Wright All Rights Reserved.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON - INFRINGEMENT.IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//=============================================================================

#ifndef ____HUFFMAN_H
#define ____HUFFMAN_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Required size of huffheap parameter to compress and decompress
 *
 * Note: if you change any of the data types in the _huffman_node
 * or _huffman_encode_table structs in huffman.c, this also must be
 * changed.
 */
#define HUFFHEAP_SIZE ((sizeof(double) * 257) + (((sizeof(void *) * 4) + sizeof(double) + sizeof(unsigned long)) * (257 * 3)) + ((sizeof(unsigned long) + sizeof(unsigned long)) * 257))

/**
 * Huffman encode a block of data
 *
 * @param in Input data
 * @param inlen Input data length
 * @param out Output buffer
 * @param outlen Output buffer length
 * @param huffheap Heap memory to use for compression (must be HUFFHEAP_SIZE in size)
 * @return Size of encoded result or 0 on out buffer overrun
 */
extern unsigned long huffman_compress(const unsigned char *in,unsigned long inlen,unsigned char *out,unsigned long outlen,void *huffheap);

/**
 * Huffman decode a block of data
 *
 * @param in Input data
 * @param inlen Length of input data
 * @param out Output buffer
 * @param outlen Length of output buffer
 * @param huffheap Heap memory to use for decompression (must be HUFFHEAP_SIZE in size)
 * @return Size of decoded result or 0 on out buffer overrun or corrupt input data
 */
extern unsigned long huffman_decompress(const unsigned char *in,unsigned long inlen,unsigned char *out,unsigned long outlen,void *huffheap);

#ifdef __cplusplus
}
#endif

#endif
