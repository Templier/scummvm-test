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

// Resource library

#include "common/util.h"
#include "common/debug.h"

#include "sci/tools.h"
#include "sci/sci_memory.h"
#include "sci/scicore/resource.h"
#include "sci/scicore/vocabulary.h"

namespace Sci {

#undef SCI_REQUIRE_RESOURCE_FILES

//#define SCI_VERBOSE_RESMGR 1

const char *sci_version_types[] = {
	"SCI version undetermined (Autodetect failed / not run)",
	"SCI version 0.xxx",
	"SCI version 0.xxx w/ 1.000 compression",
	"SCI version 1.000 w/ 0.xxx resource.map",
	"SCI version 1.000 w/ special resource.map",
	"SCI version 1.000 (early)",
	"SCI version 1.000 (late)",
	"SCI version 1.001",
	"SCI WIN/32"
};

const int sci_max_resource_nr[] = {65536, 1000, 2048, 2048, 2048, 8192, 8192, 65536};

const char *sci_error_types[] = {
	"No error",
	"I/O error",
	"Resource is empty (size 0)",
	"resource.map entry is invalid",
	"resource.map file not found",
	"No resource files found",
	"Unknown compression method",
	"Decompression failed: Decompression buffer overflow",
	"Decompression failed: Sanity check failed",
	"Decompression failed: Resource too big",
	"SCI version is unsupported"
};

// These are the 18 resource types supported by SCI1
const char *resourceTypeNames[] = {
	"view", "pic", "script", "text", "sound",
	"memory", "vocab", "font", "cursor",
	"patch", "bitmap", "palette", "cdaudio",
	"audio", "sync", "message", "map", "heap"
};

const char *resourceTypeSuffixes[] = {
	"v56", "p56", "scr", "tex", "snd",
	"   ", "voc", "fon", "cur", "pat",
	"bit", "pal", "cda", "aud", "syn",
	"msg", "map", "hep"
};

const char *getResourceTypeName(ResourceType restype) {
	return resourceTypeNames[restype];
}

const char *getResourceTypeSuffix(ResourceType restype) {
	return resourceTypeSuffixes[restype];
}

typedef int decomp_funct(Resource *result, Common::ReadStream &stream, int sci_version);
typedef void patch_sprintf_funct(char *string, Resource *res);

static decomp_funct *decompressors[] = {
	NULL,
	&decompress0,
	&decompress01,
	&decompress01,
	&decompress01,
	&decompress1,
	&decompress1,
	&decompress11,
	NULL
};

//-- Resource main functions --
Resource::Resource() {
	data = NULL;
	number = 0;
	type = kResourceTypeInvalid;
	id = 0;
	size = 0;
	file_offset = 0;
	status = SCI_STATUS_NOMALLOC;
	lockers = 0;
	source = NULL;
}

Resource::~Resource() {
	delete[] data;
	if(source && source->source_type == kSourcePatch)
		delete source;
}

void Resource::unalloc() {
	delete[] data;
	data = NULL;
	status = SCI_STATUS_NOMALLOC;
}

//-- Resmgr helper functions --

// Resource source list management

ResourceSource *ResourceManager::addExternalMap(const char *file_name) {
	ResourceSource *newsrc = new ResourceSource();

	// Add the new source to the SLL of sources
	newsrc->next = _sources;
	_sources = newsrc;

	newsrc->source_type = kSourceExtMap;
	newsrc->location_name = file_name;
	newsrc->scanned = false;
	newsrc->associated_map = NULL;

	return newsrc;
}

ResourceSource *ResourceManager::addVolume(ResourceSource *map, const char *filename, int number, int extended_addressing) {
	ResourceSource *newsrc = new ResourceSource();

	// Add the new source to the SLL of sources
	newsrc->next = _sources;
	_sources = newsrc;

	newsrc->source_type = kSourceVolume;
	newsrc->scanned = false;
	newsrc->location_name = filename;
	newsrc->volume_number = number;
	newsrc->associated_map = map;

	return 0;
}

ResourceSource *ResourceManager::addPatchDir(const char *dirname) {
	ResourceSource *newsrc = new ResourceSource();

	// Add the new source to the SLL of sources
	newsrc->next = _sources;
	_sources = newsrc;

	newsrc->source_type = kSourceDirectory;
	newsrc->scanned = false;
	newsrc->location_name = dirname;

	return 0;
}

ResourceSource *ResourceManager::getVolume(ResourceSource *map, int volume_nr) {
	ResourceSource *seeker = _sources;

	while (seeker) {
		if (seeker->source_type == kSourceVolume && seeker->associated_map == map &&
		        seeker->volume_number == volume_nr)
			return seeker;
		seeker = seeker->next;
	}

	return NULL;
}

// Resource manager constructors and operations

bool ResourceManager::loadFromPatchFile(Resource *res) {
	Common::File file;
	const char *filename=res->source->location_name.c_str();
	if (file.open(filename) == false) {
		warning("Failed to open patch file %s", filename);
		res->unalloc();
		return false;
	}
	res->data = new byte[res->size];

	if (res->data == NULL) {
		error("Can't allocate %d bytes needed for loading %s!", res->size, filename);
	}

	file.seek(res->file_offset, SEEK_SET);
	unsigned int really_read = file.read(res->data, res->size);
	if (really_read != res->size) {
		error("Read %d bytes from %s but expected %d!", really_read, filename, res->size);
	}
	res->status = SCI_STATUS_ALLOCATED;
	return true;
}

void ResourceManager::loadResource(Resource *res) {
	char filename[MAXPATHLEN];
	Common::File file;

	if (res->source->source_type == kSourcePatch && loadFromPatchFile(res))
		return;
	// Either loading from volume or patch loading failed
	strcpy(filename, res->source->location_name.c_str());

	if (!file.open(filename)) {
		warning("Failed to open %s", filename);
		res->unalloc();
		return;
	}
	file.seek(res->file_offset, SEEK_SET);

	// Check whether we support this at all
	if (decompressors[_sciVersion] == NULL)
		error("Resource manager's SCI version (%d) is invalid", _sciVersion);
	// Decompress from regular resource file
	int error = decompressors[_sciVersion](res, file, _sciVersion);

	if (error) {
		warning("Error %d occured while reading %s.%03d from resource file: %s\n",
			error, getResourceTypeName(res->type), res->number, sci_error_types[error]);
		res->unalloc();
	}
}

Resource *ResourceManager::testResource(ResourceType type, int number) {
	if (_resMap.contains(RESOURCE_HASH(type, number)))
		return _resMap.getVal(RESOURCE_HASH(type, number));
	return NULL;
}

int sci0_get_compression_method(Common::ReadStream &stream);

int sci_test_view_type(ResourceManager *mgr) {
	Common::File file;
	char filename[MAXPATHLEN];
	int compression;
	Resource *res;
	int i;

	for (i = 0; i < 1000; i++) {
		res = mgr->testResource(kResourceTypeView, i);

		if (!res)
			continue;

		if (res->source->source_type == kSourceDirectory)
			continue;

		strcpy(filename, res->source->location_name.c_str());

		if (!file.open(filename))
			continue;
		file.seek(res->file_offset, SEEK_SET);

		compression = sci0_get_compression_method(file);
		file.close();

		if (compression == 3) {
			return SCI_VERSION_01_VGA;
		}
	}

	// Try the same thing with pics
	for (i = 0; i < 1000; i++) {
		res = mgr->testResource(kResourceTypePic, i);

		if (!res)
			continue;

		if (res->source->source_type == kSourceDirectory)
			continue;

		strcpy(filename, res->source->location_name.c_str());

		if (!file.open(filename))
			continue;
		file.seek(res->file_offset, SEEK_SET);

		compression = sci0_get_compression_method(file);
		file.close();

		if (compression == 3) {
			return SCI_VERSION_01_VGA;
		}
	}

	return SCI_VERSION_AUTODETECT;
}

int ResourceManager::addAppropriateSources() {
	ResourceSource *map;

	if (!Common::File::exists("RESOURCE.MAP"))
		return 0;
	map = addExternalMap("RESOURCE.MAP");

	Common::ArchiveMemberList files;
	SearchMan.listMatchingMembers(files, "RESOURCE.0??");

	for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); ++x) {
		const Common::String name = (*x)->getName();
		const char *dot = strrchr(name.c_str(), '.');
		int number = atoi(dot + 1);

		addVolume(map, name.c_str(), number, 0);
	}
	addPatchDir("");	// FIXME: used to pass the 'current' instead of ""
	return 1;
}

int ResourceManager::scanNewSources(ResourceSource *source) {
	if (!source)
		return SCI_ERROR_NO_RESOURCE_FILES_FOUND;

	int resource_error = 0;
	if (source->next)
		scanNewSources(source->next);

	if (!source->scanned) {
		source->scanned = true;
		switch (source->source_type) {
		case kSourceDirectory:
			readResourcePatches(source);
			break;
		case kSourceExtMap:
			if (_mapVersion < SCI_VERSION_1)
				resource_error = readResourceMapSCI0(source);
			else 
				resource_error = readResourceMapSCI1(source, getVolume(source, 0));
			if (resource_error == SCI_ERROR_RESMAP_NOT_FOUND) {
				// FIXME: Try reading w/o resource.map
				resource_error = SCI_ERROR_NO_RESOURCE_FILES_FOUND;
			}

			if (resource_error == SCI_ERROR_NO_RESOURCE_FILES_FOUND) {
				// Initialize empty resource manager
				_resMap.clear();
				resource_error = 0;
			}
			break;
		default:
			break;
		}
	}
	return resource_error;
}

void ResourceManager::freeResourceSources(ResourceSource *rss) {
	if (rss) {
		freeResourceSources(rss->next);
		delete rss;
	}
}

ResourceManager::ResourceManager(int version, int maxMemory) {
	_maxMemory = maxMemory;
	_memoryLocked = 0;
	_memoryLRU = 0;
	_LRU.clear();
	_resMap.clear();
	_sources = NULL;
	_sciVersion = version;

	addAppropriateSources();

	if (version != SCI_VERSION_AUTODETECT) {
		_mapVersion = version;
		_volVersion = version;
	} else {
		_mapVersion = detectMapVersion();
		_volVersion = detectVolVersion();
	}
	debug("Using resource map version %d %s", _mapVersion, sci_version_types[_mapVersion]);
	debug("Using volume version %d %s", _volVersion, sci_version_types[_volVersion]);

	scanNewSources(_sources);
	if (version == SCI_VERSION_AUTODETECT)
		switch (_mapVersion) {
		case SCI_VERSION_0:
			if (testResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI0_MAIN_VOCAB)) {
				version = sci_test_view_type(this) ? SCI_VERSION_01_VGA : SCI_VERSION_0;
			} else if (testResource(kResourceTypeVocab, VOCAB_RESOURCE_SCI1_MAIN_VOCAB)) {
				version = sci_test_view_type(this);
				if (version != SCI_VERSION_01_VGA) {
					version = testResource(kResourceTypeVocab, 912) ? SCI_VERSION_0 : SCI_VERSION_01;
				}
			} else {
				version = sci_test_view_type(this) ? SCI_VERSION_01_VGA : SCI_VERSION_0;
			}
			break;
		case SCI_VERSION_01_VGA_ODD:
			version = _mapVersion; 
			break;
		case SCI_VERSION_1: {
			Resource *res = testResource(kResourceTypeScript, 0);

			_sciVersion = version = SCI_VERSION_1_EARLY;
			loadResource(res);

			if (res->status == SCI_STATUS_NOMALLOC)
				version = SCI_VERSION_1_LATE;
			break;
		}
		case SCI_VERSION_1_1:
			// No need to handle SCI 1.1 here - it was done in resource_map.cpp
			version = SCI_VERSION_1_1;
			break;
		default:
			version = SCI_VERSION_AUTODETECT;
		}

	_sciVersion = version;
	// temporary version printout - should be reworked later
	switch (_sciVersion) {
	case SCI_VERSION_0:
		debug("Resmgr: Detected SCI0"); break;
	case SCI_VERSION_01:
		debug("Resmgr: Detected SCI01"); break;
	case SCI_VERSION_01_VGA:
		debug("Resmgr: Detected SCI01VGA - KQ5 or similar"); break;
	case SCI_VERSION_01_VGA_ODD:
		debug("Resmgr: Detected SCI01VGA - Jones/CD or similar"); break;
	case SCI_VERSION_1_EARLY:
		debug("Resmgr: Detected SCI1 Early"); break;
	case SCI_VERSION_1_LATE:
		debug("Resmgr: Detected SCI1 Late"); break;
	case SCI_VERSION_1_1:
		debug("Resmgr: Detected SCI1.1"); break;
	case SCI_VERSION_32:
		debug("Resmgr: Couldn't determine SCI version"); break;
	default:
		debug("Resmgr: Couldn't determine SCI version"); break;
	}
}

ResourceManager::~ResourceManager() {
	// freeing resources
	Common::HashMap<uint32, Resource *>::iterator itr = _resMap.begin();
	while (itr != _resMap.end()) {
		delete itr->_value;
		itr ++;
	}
	freeResourceSources(_sources);
	_resMap.empty();
}

void ResourceManager::removeFromLRU(Resource *res) {
	if (res->status != SCI_STATUS_ENQUEUED) {
		sciprintf("Resmgr: Oops: trying to remove resource that isn't enqueued\n");
		return;
	}
	_LRU.remove(res);
	_memoryLRU -= res->size;
	res->status = SCI_STATUS_ALLOCATED;
}

void ResourceManager::addToLRU(Resource *res) {
	if (res->status != SCI_STATUS_ALLOCATED) {
		warning("Resmgr: Oops: trying to enqueue resource with state %d", res->status);
		return;
	}
	_LRU.push_front(res);
	_memoryLRU += res->size;
#if (SCI_VERBOSE_RESMGR > 1)
	debug("Adding %s.%03d (%d bytes) to lru control: %d bytes total",
	        getResourceTypeName(res->type), res->number, res->size,
	        mgr->_memoryLRU);

#endif
	res->status = SCI_STATUS_ENQUEUED;
}

void ResourceManager::printLRU() {
	int mem = 0;
	int entries = 0;
	Common::List<Resource *>::iterator it = _LRU.begin();
	Resource *res;

	while (it != _LRU.end()) {
		res = *it;
		debug("\t%s.%03d: %d bytes", getResourceTypeName(res->type),
			res->number, res->size);
		mem += res->size;
		entries ++;
		it ++;
	}

	debug("Total: %d entries, %d bytes (mgr says %d)", entries, mem, _memoryLRU);
}

void ResourceManager::freeOldResources(int last_invulnerable) {
	while (_maxMemory < _memoryLRU && (!last_invulnerable || !_LRU.empty())) {
		Resource *goner = *_LRU.reverse_begin();
		if (!goner) {
			debug("Internal error: mgr->lru_last is NULL!");
			debug("LRU-mem= %d", _memoryLRU);
			debug("lru_first = %p", (void *)*_LRU.begin());
			printLRU();
		}
		removeFromLRU(goner);
		goner->unalloc();
#ifdef SCI_VERBOSE_RESMGR
		sciprintf("Resmgr-debug: LRU: Freeing %s.%03d (%d bytes)\n", getResourceTypeName(goner->type), goner->number, goner->size);
#endif
	}
}

Resource *ResourceManager::findResource(ResourceType type, int number, int lock) {
	Resource *retval;

	if (number >= sci_max_resource_nr[_sciVersion]) {
		int modded_number = number % sci_max_resource_nr[_sciVersion];
		sciprintf("[resmgr] Requested invalid resource %s.%d, mapped to %s.%d\n",
		          getResourceTypeName(type), number, getResourceTypeName(type), modded_number);
		number = modded_number;
	}

	retval = testResource(type, number);

	if (!retval)
		return NULL;

	if (!retval->status)
		loadResource(retval);

	else if (retval->status == SCI_STATUS_ENQUEUED)
		removeFromLRU(retval);
	// Unless an error occured, the resource is now either
	// locked or allocated, but never queued or freed.

	if (lock) {
		if (retval->status == SCI_STATUS_ALLOCATED) {
			retval->status = SCI_STATUS_LOCKED;
			retval->lockers = 0;
			_memoryLocked += retval->size;
		}

		++retval->lockers;

	} else if (retval->status != SCI_STATUS_LOCKED) { // Don't lock it
		if (retval->status == SCI_STATUS_ALLOCATED)
			addToLRU(retval);
	}

	freeOldResources(retval->status == SCI_STATUS_ALLOCATED);

	if (retval->data)
		return retval;
	else {
		sciprintf("Resmgr: Failed to read %s.%03d\n", getResourceTypeName(retval->type), retval->number);
		return NULL;
	}
}

void ResourceManager::unlockResource(Resource *res, int resnum, ResourceType restype) {
	if (!res) {
		if (restype == kResourceTypeInvalid)
			sciprintf("Resmgr: Warning: Attempt to unlock non-existant resource %03d.%03d!\n", restype, resnum);
		else
			sciprintf("Resmgr: Warning: Attempt to unlock non-existant resource %s.%03d!\n", getResourceTypeName(restype), resnum);
		return;
	}

	if (res->status != SCI_STATUS_LOCKED) {
		sciprintf("Resmgr: Warning: Attempt to unlock unlocked resource %s.%03d\n",
		          getResourceTypeName(res->type), res->number);
		return;
	}

	if (!--res->lockers) { // No more lockers?
		res->status = SCI_STATUS_ALLOCATED;
		_memoryLocked -= res->size;
		addToLRU(res);
	}

	freeOldResources(0);
}

int ResourceManager::detectMapVersion() {
	Common::File file;
	byte buff[6];
	ResourceSource *rsrc = _sources;
	// looking for extarnal map among sources
	while (rsrc) {
		if (rsrc->source_type == kSourceExtMap) {
			file.open(rsrc->location_name);
			break;
		}
		rsrc = rsrc->next;
	}
	if (file.isOpen() == false) {
		warning("Failed to open resource map file");	
		return SCI_VERSION_AUTODETECT;
	}
	// detection
	// SCI0 and SCI01 maps have last 6 bytes set to FF
	file.seek(-4, SEEK_END);
	uint32 uEnd = file.readUint32LE();
	if (uEnd == 0xFFFFFFFF) {
		// check if 0 or 01 - try to read resources in SCI0 format and see if exists
		file.seek(0, SEEK_SET);
		while (file.read(buff, 6) == 6 && !(buff[0] == 0xFF && buff[1] == 0xFF && buff[2] == 0xFF)) {
			if (getVolume(rsrc, (buff[5] & 0xFC) >> 2) == NULL)
				return SCI_VERSION_01_VGA_ODD;
		}
		return SCI_VERSION_0;
	}
	// SCI1E/L and some SCI1.1 maps have last directory entry set to 0xFF 
	// and offset set to filesize
	// SCI1 have 6-bytes entries, while SCI1.1 have 5-byte entries
	file.seek(1, SEEK_SET);
	uint16 off1, off = file.readUint16LE();
	uint16 nEntries  = off / 3;
	file.seek(1, SEEK_CUR);
	file.seek(off - 3, SEEK_SET);
	if (file.readByte() == 0xFF && file.readUint16LE() == file.size()) {
		file.seek(3, SEEK_SET);
		for (int i = 0; i < nEntries; i++) {
			file.seek(1, SEEK_CUR);
			off1 = file.readUint16LE();
			if ((off1 - off) % 5 && (off1 - off) % 6 == 0)
				return SCI_VERSION_1;
			if ((off1 - off) % 5 == 0 && (off1 - off) % 6)
				return SCI_VERSION_1_1;
			off = off1;
		}
		return SCI_VERSION_1;
	}
	// late SCI1.1 and SCI32 maps have last directory entry set to 0xFF
	// offset set to filesize and 4 more bytes
	file.seek(off - 7, SEEK_SET);
	if (file.readByte() == 0xFF && file.readUint16LE() == file.size())
		return SCI_VERSION_1_1; // TODO : check if there is a difference between these maps

	return SCI_VERSION_AUTODETECT;
}

int ResourceManager::detectVolVersion() {
	Common::File file;
	ResourceSource *rsrc = _sources;
	uint32 pos;
	// looking for a volume among sources
	while (rsrc) {
		if (rsrc->source_type == kSourceVolume) {
			file.open(rsrc->location_name);
			break;
		}
		rsrc = rsrc->next;
	}
	if (file.isOpen() == false) {
		warning("Failed to open volume file");	
		return SCI_VERSION_AUTODETECT;
	}
	// SCI0 volume format:  {wResId wPacked+4 wUnpacked wCompression} = 8 bytes
	// SCI1 volume format:  {bResType wResNumber wPacked+4 wUnpacked wCompression} = 9 bytes
	// SCI1.1 volume format:  {bResType wResNumber wPacked+4 wUnpacked wCompression} = 9 bytes
	// SCI32 volume format :  {bResType wResNumber dwPacked+4 dwUnpacked wCompression} = 13 bytes
	// Try to parse volume with SCI0 scheme to see if it make sense
	// Checking 1MB of data should be enough to determine the version
	uint16 resId, wCompression;
	uint32 dwPacked, dwUnpacked;
	bool bFailed = false;
	while(!file.eos() && !bFailed && file.pos() < 0x100000) {
		resId = file.readUint16LE();
		dwPacked = file.readUint16LE();
		dwUnpacked = file.readUint16LE();
		wCompression = file.readUint16LE();
		if(file.eos())
			break;
		if ((wCompression > 4) || (wCompression == 0 && dwPacked != dwUnpacked + 4) 
			|| (dwUnpacked < dwPacked - 4)) {
			bFailed = true;
			break;
		}
		file.seek(dwPacked - 4, SEEK_CUR);
	}
	if (!bFailed)
		return SCI_VERSION_0;
	//
	// Check for SCI1 format
	bFailed = false;
	file.seek(0, SEEK_SET);
	while(!file.eos() && !bFailed && file.pos() < 0x100000) {
		pos = file.pos();
		file.seek(1, SEEK_CUR); 
		resId = file.readUint16LE();
		dwPacked = file.readUint16LE();
		dwUnpacked = file.readUint16LE();
		wCompression = file.readUint16LE();
		if(file.eos())
			break;
		if ((wCompression > 20) || (wCompression == 0 && dwPacked != dwUnpacked + 4) 
			|| (dwUnpacked < dwPacked - 4)) {
			bFailed = true;
			break;
		}
		file.seek(dwPacked - 4, SEEK_CUR);
	}
	if (!bFailed)
		return SCI_VERSION_1;
	//
	// Check for SCI1.1 format
	bFailed = false;
	file.seek(0, SEEK_SET);
	while(!file.eos() && !bFailed && file.pos() < 0x100000) {
		pos = file.pos();
		file.seek(1, SEEK_CUR); 
		resId = file.readUint16LE();
		dwPacked = file.readUint16LE();
		dwUnpacked = file.readUint16LE();
		wCompression = file.readUint16LE();
		if(file.eos())
			break;
		if ((wCompression > 20) || (wCompression == 0 && dwPacked != dwUnpacked) 
			|| (dwUnpacked < dwPacked)) {
			bFailed = true;
			break;
		}
		file.seek((9 + dwPacked) % 2 ? dwPacked + 1 : dwPacked, SEEK_CUR);
	}
	if (!bFailed)
		return SCI_VERSION_1_1;
	//
	// Check for SCI32 v2 format (Gabriel Knight 1 CD)
	bFailed = false;
	file.seek(0, SEEK_SET);
	while(!file.eos() && !bFailed && file.pos() < 0x100000) {
		pos = file.pos();
		file.seek(1, SEEK_CUR); 
		resId = file.readUint16LE();
		dwPacked = file.readUint32LE();
		dwUnpacked = file.readUint32LE();
		wCompression = file.readUint16LE();
		if(file.eos())
			break;
		if ((wCompression != 0 && wCompression != 32) 
			|| (wCompression == 0 && dwPacked != dwUnpacked) 
			|| (dwUnpacked < dwPacked)) {
			bFailed = true;
			break;
		}
		file.seek(dwPacked, SEEK_CUR);//(9 + wPacked) % 2 ? wPacked + 1 : wPacked, SEEK_CUR);
	}
	if (!bFailed)
		return SCI_VERSION_32;

	// Failed to detect volume version
	return SCI_VERSION_AUTODETECT;
}

// version-agnostic patch application
void ResourceManager::processPatch(ResourceSource *source,
	const char *filename, ResourceType restype, int resnumber) {
	Common::File file;
	Resource *newrsc;
	uint32 resId = RESOURCE_HASH(restype, resnumber);
	byte patchtype, patch_data_offset;
	int fsize;

	if (resnumber == -1)
		return;
	if (!file.open(filename)) {
		perror("""__FILE__"": (""__LINE__""): failed to open");
		return;
	}
	fsize = file.size();
	if (fsize < 3) {
		debug("Patching %s failed - file too small", filename);
		return;
	}
	
	patchtype = file.readByte() & 0x7F;
	patch_data_offset = file.readByte();

	if (patchtype != restype) {
		debug("Patching %s failed - resource type mismatch", filename);
		return;
	}
	if (patch_data_offset + 2 >= fsize) {
		debug("Patching %s failed - patch starting at offset %d can't be in file of size %d",
			filename, patch_data_offset + 2, fsize);
		return;
	}
	// Prepare destination, if neccessary
	if (_resMap.contains(resId) == false) {
		newrsc = new Resource;
		_resMap.setVal(resId, newrsc);
	} else 
		newrsc = _resMap.getVal(resId);
	// Overwrite everything, because we're patching
	newrsc->id = resId;
	newrsc->number = resnumber;
	newrsc->status = SCI_STATUS_NOMALLOC;
	newrsc->type = restype;
	newrsc->source = source;
	newrsc->size = fsize - patch_data_offset - 2;
	newrsc->file_offset = 2 + patch_data_offset;
	debug("Patching %s - OK", filename);
}


void ResourceManager::readResourcePatches(ResourceSource *source) {
// Note: since some SCI1 games(KQ5 floppy, SQ4) might use SCI0 naming scheme for patch files
// this function tries to read patch file with any supported naming scheme,
// regardless of _sciVersion value

	Common::String mask, name;
	Common::ArchiveMemberList files;
	int number;
	const char *szResType;
	ResourceSource *psrcPatch;

	for (int i = kResourceTypeView; i < kResourceTypeInvalid; i ++) {
		files.clear();
		szResType = getResourceTypeName((ResourceType)i);
		// SCI0 naming - type.nnn
		mask = szResType;
		mask += ".???";
		SearchMan.listMatchingMembers(files, mask);
		// SCI1 and later naming - nnn.typ
		mask = "*.";
		mask += getResourceTypeSuffix((ResourceType)i);
		SearchMan.listMatchingMembers(files, mask);
		for (Common::ArchiveMemberList::const_iterator x = files.begin(); x != files.end(); x++) {
			number = -1;
			name = (*x)->getName();
			if (isdigit(name[0])) {
				// SCI1 scheme
				number = atoi(name.c_str());
			} else {
				// SCI0 scheme
				int resname_len = strlen(szResType);
				if (scumm_strnicmp(name.c_str(), szResType, resname_len) == 0) {
					number = atoi(name.c_str() + resname_len + 1);
				}
			}
			psrcPatch = new ResourceSource;
			psrcPatch->source_type = kSourcePatch;
			psrcPatch->location_name = name;
			processPatch(psrcPatch, name.c_str(), (ResourceType)i, number);
		}
	}
}

int ResourceManager::readResourceMapSCI0(ResourceSource *map) {
	Common::File file;
	Resource *res;
	ResourceType type;
	uint16 number, id;
	uint32 offset;

	if (!file.open(map->location_name))
		return SCI_ERROR_RESMAP_NOT_FOUND;

	file.seek(0, SEEK_SET);

	byte bMask = _mapVersion == SCI_VERSION_01_VGA_ODD ? 0xF0: 0xFC;
	byte bShift = _mapVersion == SCI_VERSION_01_VGA_ODD ? 28 : 26;

	do {
		id = file.readUint16LE();
		offset = file.readUint32LE();

		if (file.ioFailed()) {
			warning("Error while reading %s: ", map->location_name.c_str());
			perror("");
			return SCI_ERROR_RESMAP_NOT_FOUND;
		}
		if(offset == 0xFFFFFFFF)
			break;

		type = (ResourceType)(id >> 11);
		number = id & 0x7FF;
		uint32 resId = RESOURCE_HASH(type, number);
		// adding a new resource
		if (_resMap.contains(resId) == false) {
			res = new Resource;
			res->id = id;
			res->file_offset = offset & (((~bMask) << 24) | 0xFFFFFF);
			res->number = number;
			res->type = type;
			res->source = getVolume(map, offset >> bShift);
			_resMap.setVal(resId, res);
		}
	} while (!file.eos());
	return 0;
}

int ResourceManager::readResourceMapSCI1(ResourceSource *map, ResourceSource *vol) {
	Common::File file;
	Resource *res;
	if (!file.open(map->location_name))
		return SCI_ERROR_RESMAP_NOT_FOUND;

	resource_index_t resMap[32];
	memset(resMap, 0, sizeof(resource_index_t) * 32);
	byte type = 0, prevtype = 0;
	byte nEntrySize = _mapVersion == SCI_VERSION_1_1 ? SCI11_RESMAP_ENTRIES_SIZE : SCI1_RESMAP_ENTRIES_SIZE;
	uint32 resId;

	// Read resource type and offsets to resource offsets block from .MAP file
	// The last entry has type=0xFF (0x1F) and offset equals to map file length
	do {
		type = file.readByte() & 0x1F;
		resMap[type].wOffset = file.readUint16LE();
		resMap[prevtype].wSize = (resMap[type].wOffset
				- resMap[prevtype].wOffset) / nEntrySize;
		prevtype = type;
	} while (type != 0x1F); // the last entry is FF

	// reading each type's offsets
	uint32 off = 0;
	for (type = 0; type < 32; type++) {
		if (resMap[type].wOffset == 0) // this resource does not exist in map
			continue;
		file.seek(resMap[type].wOffset);
		for (int i = 0; i < resMap[type].wSize; i++) {
			uint16 number = file.readUint16LE();
			file.read(&off, nEntrySize - 2);
			if (file.ioFailed()) {
				warning("Error while reading %s: ", map->location_name.c_str());
				perror("");
				return SCI_ERROR_RESMAP_NOT_FOUND;
			}
			resId = RESOURCE_HASH(type, number);
			// adding new resource only if it does not exist
			if (_resMap.contains(resId) == false) {
				res = new Resource;
				_resMap.setVal(resId, res);
				res->type = (ResourceType)type;
				res->number = number;
				res->id = res->number | (res->type << 16);
				res->source = _mapVersion < SCI_VERSION_1_1 ? getVolume(map, off  >> 28) : vol;
				if (_mapVersion < SCI_VERSION_32)
					res->file_offset = _mapVersion < SCI_VERSION_1_1 ? off & 0x0FFFFFFF : off << 1;
				else
					res->file_offset = off; // in SCI32 it's a plain offset
			}
		}
	}
	return 0;
}

} // End of namespace Sci
