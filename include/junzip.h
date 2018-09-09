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

/**
* JUnzip library by Joonas Pihlajamaa (firstname.lastname@iki.fi).
* Released into public domain. https://github.com/jokkebk/JUnzip
*/

#ifndef __JUNZIP_H
#define __JUNZIP_H

#define Z_ERRNO 1
#define Z_OK 0


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "deflate_tinfl.h"
#include <stdint.h>

	typedef struct
	{
		char *file;
		unsigned char *data;
		int size;
	} userdata_t;

	// If you don't have stdint.h, the following two lines should work for most 32/64 bit systems
	// typedef unsigned int uint32_t;
	// typedef unsigned short uint16_t;

	typedef struct JZFile JZFile;

	struct JZFile {
		size_t(*read)(JZFile *file, void *buf, size_t size);
		size_t(*tell)(JZFile *file);
		int(*seek)(JZFile *file, size_t offset, int whence);
		int(*error)(JZFile *file);
		void(*close)(JZFile *file);
	};

	JZFile *
		jzfile_from_stdio_file(FILE *fp);
#pragma pack(push)
#pragma pack(1)
	typedef struct {
		uint32_t signature;
		uint16_t versionNeededToExtract; // unsupported
		uint16_t generalPurposeBitFlag; // unsupported
		uint16_t compressionMethod;
		uint16_t lastModFileTime;
		uint16_t lastModFileDate;
		uint32_t crc32;
		uint32_t compressedSize;
		uint32_t uncompressedSize;
		uint16_t fileNameLength;
		uint16_t extraFieldLength; // unsupported
	} JZLocalFileHeader;

	typedef struct {
		uint32_t signature;
		uint16_t versionMadeBy; // unsupported
		uint16_t versionNeededToExtract; // unsupported
		uint16_t generalPurposeBitFlag; // unsupported
		uint16_t compressionMethod;
		uint16_t lastModFileTime;
		uint16_t lastModFileDate;
		uint32_t crc32;
		uint32_t compressedSize;
		uint32_t uncompressedSize;
		uint16_t fileNameLength;
		uint16_t extraFieldLength; // unsupported
		uint16_t fileCommentLength; // unsupported
		uint16_t diskNumberStart; // unsupported
		uint16_t internalFileAttributes; // unsupported
		uint32_t externalFileAttributes; // unsupported
		uint32_t relativeOffsetOflocalHeader;
	} JZGlobalFileHeader;

	typedef struct {
		uint16_t compressionMethod;
		uint16_t lastModFileTime;
		uint16_t lastModFileDate;
		uint32_t crc32;
		uint32_t compressedSize;
		uint32_t uncompressedSize;
		uint32_t offset;
	} JZFileHeader;

	typedef struct {
		uint32_t signature; // 0x06054b50
		uint16_t diskNumber; // unsupported
		uint16_t centralDirectoryDiskNumber; // unsupported
		uint16_t numEntriesThisDisk; // unsupported
		uint16_t numEntries;
		uint32_t centralDirectorySize;
		uint32_t centralDirectoryOffset;
		uint16_t zipCommentLength;
		// Followed by .ZIP file comment (variable size)
	} JZEndRecord;
#pragma pack(pop)

	// Callback prototype for central and local file record reading functions
	typedef int(*JZRecordCallback)(JZFile *zip, int index, JZFileHeader *header,
		char *filename, void *user_data);

#define JZ_BUFFER_SIZE 65536

	// Read ZIP file end record. Will move within file.
	int jzReadEndRecord(JZFile *zip, JZEndRecord *endRecord);

	// Read ZIP file global directory. Will move within file.
	// Callback is called for each record, until callback returns zero
	int jzReadCentralDirectory(JZFile *zip, JZEndRecord *endRecord,
		JZRecordCallback callback, void *user_data);

	// Read local ZIP file header. Silent on errors so optimistic reading possible.
	int jzReadLocalFileHeader(JZFile *zip, JZFileHeader *header,
		char *filename, int len);

	// Read data from file stream, described by header, to preallocated buffer
	// Return value is zlib coded, e.g. Z_OK, or error code
	int jzReadData(JZFile *zip, JZFileHeader *header, void *buffer);

#ifdef __cplusplus
};
#endif /* __cplusplus */

#endif
