/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/* Reads data from a resource file and stores the result in memory */

#include "common/util.h"
#include "sci/include/sci_memory.h"
#include "sci/include/sciresource.h"

#define DDEBUG if (0) printf

void decryptinit3(void);
int decrypt3(guint8* dest, guint8* src, int length, int complength);
int decrypt4(guint8* dest, guint8* src, int length, int complength);

int decompress11(resource_t *result, Common::ReadStream &stream, int sci_version) {
	guint16 compressedLength;
	guint16 compressionMethod, result_size;
	guint8 *buffer;
	guint8 tempid;

	DDEBUG("d1");

	if (stream.read(&tempid, 1) != 1)
		return SCI_ERROR_IO_ERROR;

	result->id = tempid;

	result->type = result->id & 0x7f;
	if (stream.read(&(result->number), 2) != 2)
		return SCI_ERROR_IO_ERROR;

#ifdef WORDS_BIGENDIAN
	result->number = GUINT16_SWAP_LE_BE_CONSTANT(result->number);
#endif /* WORDS_BIGENDIAN */
	if ((result->type > sci_invalid_resource))
		return SCI_ERROR_DECOMPRESSION_INSANE;

	if ((stream.read(&compressedLength, 2) != 2) ||
	        (stream.read(&result_size, 2) != 2) ||
	        (stream.read(&compressionMethod, 2) != 2))
		return SCI_ERROR_IO_ERROR;

#ifdef WORDS_BIGENDIAN
	compressedLength = GUINT16_SWAP_LE_BE_CONSTANT(compressedLength);
	result_size = GUINT16_SWAP_LE_BE_CONSTANT(result_size);
	compressionMethod = GUINT16_SWAP_LE_BE_CONSTANT(compressionMethod);
#endif
	result->size = result_size;

	/*  if ((result->size < 0) || (compressedLength < 0))
	    return SCI_ERROR_DECOMPRESSION_INSANE; */
	/* This return will never happen in SCI0 or SCI1 (does it have any use?) */

	if (result->size > SCI_MAX_RESOURCE_SIZE)
		return SCI_ERROR_RESOURCE_TOO_BIG;

	if (compressedLength > 0)
		compressedLength -= 0;
	else { /* Object has size zero (e.g. view.000 in sq3) (does this really exist?) */
		result->data = 0;
		result->status = SCI_STATUS_NOMALLOC;
		return SCI_ERROR_EMPTY_OBJECT;
	}

	buffer = (guint8*)sci_malloc(compressedLength);
	result->data = (unsigned char*)sci_malloc(result->size);

	if (stream.read(buffer, compressedLength) != compressedLength) {
		free(result->data);
		free(buffer);
		return SCI_ERROR_IO_ERROR;
	};

	if (!(compressedLength & 1)) { /* Align */
		stream.readByte();
	}

#ifdef _SCI_DECOMPRESS_DEBUG
	error("Resource %i.%s encrypted with method SCI1.1/%hi at %.2f%%"
	        " ratio\n",
	        result->number, sci_resource_type_suffixes[result->type],
	        compressionMethod,
	        (result->size == 0) ? -1.0 :
	        (100.0 * compressedLength / result->size));
	error("  compressedLength = 0x%hx, actualLength=0x%hx\n",
	        compressedLength, result->size);
#endif

	DDEBUG("/%d[%d]", compressionMethod, result->size);

	switch (compressionMethod) {

	case 0: /* no compression */
		if (result->size != compressedLength) {
			free(result->data);
			result->data = NULL;
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		memcpy(result->data, buffer, compressedLength);
		result->status = SCI_STATUS_ALLOCATED;
		break;

	case 18:
	case 19:
	case 20:
		if (decrypt4(result->data, buffer, result->size, compressedLength)) {
			free(result->data);
			result->data = 0; /* So that we know that it didn't work */
			result->status = SCI_STATUS_NOMALLOC;
			free(buffer);
			return SCI_ERROR_DECOMPRESSION_OVERFLOW;
		}
		result->status = SCI_STATUS_ALLOCATED;
		break;

	case 3:
	case 4: /* NYI */
		error("Resource %d.%s: Warning: compression type #%d not yet implemented\n",
		        result->number, sci_resource_type_suffixes[result->type], compressionMethod);
		free(result->data);
		result->data = NULL;
		result->status = SCI_STATUS_NOMALLOC;
		break;

	default:
		error("Resource %d.%s: Compression method SCI1/%hi not "
		        "supported", result->number, sci_resource_type_suffixes[result->type],
		        compressionMethod);
		free(result->data);
		result->data = NULL; /* So that we know that it didn't work */
		result->status = SCI_STATUS_NOMALLOC;
		free(buffer);
		return SCI_ERROR_UNKNOWN_COMPRESSION;
	}

	free(buffer);
	return 0;
}

