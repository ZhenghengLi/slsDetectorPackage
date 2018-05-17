/************************************************
 * @file BinaryFile.cpp
 * @short sets/gets properties for the binary file,
 * creates/closes the file and writes data to it
 ***********************************************/

#include "BinaryFile.h"
#include "receiver_defs.h"
#include "Fifo.h"

#include <iostream>
using namespace std;


FILE* BinaryFile::masterfd = 0;

BinaryFile::BinaryFile(int ind, uint32_t maxf,
		int* nd, char* fname, char* fpath, uint64_t* findex, bool* owenable,
		int* dindex, int* nunits, uint64_t* nf, uint32_t* dr, uint32_t* portno,
		bool* smode):
		File(ind, maxf, nd, fname, fpath, findex, owenable, dindex, nunits, nf, dr, portno, smode),
		filefd(0),
		numFramesInFile(0),
		numActualPacketsInFile(0)
{
#ifdef VERBOSE
	PrintMembers();
#endif
}

BinaryFile::~BinaryFile() {
	CloseAllFiles();
}

void BinaryFile::PrintMembers() {
	File::PrintMembers();
	FILE_LOG(logINFO) << "Max Frames Per File: " << maxFramesPerFile;
	FILE_LOG(logINFO) << "Number of Frames in File: " << numFramesInFile;
}

slsReceiverDefs::fileFormat BinaryFile::GetFileType() {
	return BINARY;
}


int BinaryFile::CreateFile(uint64_t fnum) {
	numFramesInFile = 0;
	numActualPacketsInFile = 0;

	currentFileName = BinaryFileStatic::CreateFileName(filePath, fileNamePrefix, *fileIndex,
			(*numImages > 1), fnum, *detIndex, *numUnitsPerDetector, index);

	if (BinaryFileStatic::CreateDataFile(filefd, *overWriteEnable, currentFileName, FILE_BUFFER_SIZE) == FAIL)
		return FAIL;

	if(!silentMode) {
		FILE_LOG(logINFO) << "[" << *udpPortNumber << "]: Binary File created: " << currentFileName;
	}
	return OK;
}

void BinaryFile::CloseCurrentFile() {
	BinaryFileStatic::CloseDataFile(filefd);
}

void BinaryFile::CloseAllFiles() {
	BinaryFileStatic::CloseDataFile(filefd);
	if (master && (*detIndex==0))
		BinaryFileStatic::CloseDataFile(masterfd);
}

int BinaryFile::WriteToFile(char* buffer, int buffersize, uint64_t fnum, uint32_t nump) {
	if (numFramesInFile >= maxFramesPerFile) {
		CloseCurrentFile();
		CreateFile(fnum);
	}
	numFramesInFile++;
	numActualPacketsInFile += nump;
	if (BinaryFileStatic::WriteDataFile(filefd, buffer, buffersize, fnum) == buffersize)
		return OK;
	cprintf(RED,"%d Error: Write to file failed for image number %lld\n", index, (long long int)fnum);
	return FAIL;
}


int BinaryFile::CreateMasterFile(bool en, uint32_t size,
		uint32_t nx, uint32_t ny, uint64_t at,  uint64_t st, uint64_t sp,
		uint64_t ap) {
	//beginning of every acquisition
	numFramesInFile = 0;
	numActualPacketsInFile = 0;

	if (master && (*detIndex==0)) {
		masterFileName = BinaryFileStatic::CreateMasterFileName(filePath,
				fileNamePrefix, *fileIndex);
		if(!silentMode) {
			FILE_LOG(logINFO) << "Master File: " << masterFileName;
		}
		return BinaryFileStatic::CreateMasterDataFile(masterfd, masterFileName,
				*overWriteEnable,
				*dynamicRange, en, size, nx, ny, *numImages,
				at, st, sp, ap, BINARY_WRITER_VERSION);
	}
	return OK;
}


