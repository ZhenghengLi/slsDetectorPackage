#ifdef SLS_RECEIVER_FUNCTION_LIST

#ifndef SLS_RECEIVER_FUNCTION_LIST_H
#define SLS_RECEIVER_FUNCTION_LIST_H
/********************************************//**
 * @file slsReceiverFunctionList.h
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 ***********************************************/


#include "sls_detector_defs.h"
#include "receiver_defs.h"
#include "genericSocket.h"
#include "circularFifo.h"


#include <string.h>
#include <pthread.h>
#include <stdio.h>


/**
 * @short does all the functions for a receiver, set/get parameters, start/stop etc.
 */

class slsReceiverFunctionList : private virtual slsDetectorDefs  {

public:
	/**
	 * Constructor
	 */
	slsReceiverFunctionList(detectorType det, bool moenchwithGotthardTest);

	/**
	 * Destructor
	 */
	virtual ~slsReceiverFunctionList();

	/**
	 * Set UDP Port Number
	 */
	void setUDPPortNo(int p){server_port = p;};

	/**
	 * Set Ethernet Interface or IP to listen to
	 */
	void setEthernetInterface(char* c);

	/**
	 * Returns status of receiver: idle, running or error
	 */
	runStatus getStatus(){return status;};

	/**
	 * Returns File Name
	 */
	char* getFileName(){return fileName;};

	/**
	 * Returns File Path
	 */
	char* getFilePath(){return filePath;};

	/**
	 * Returns File Index
	 */
	int getFileIndex(){return fileIndex;};

	/**
	 * Returns Frames Caught for each real time acquisition (eg. for each scan)
	 */
	uint32_t getFramesCaught(){return framesCaught;};

	/**
	 * Returns Total Frames Caught for an entire acquisition (including all scans)
	 */
	uint32_t getTotalFramesCaught(){return totalFramesCaught;};

	/**
	 * Returns the frame index at start of each real time acquisition (eg. for each scan)
	 */
	uint32_t getStartFrameIndex(){return startFrameIndex;};

	/**
	 * Returns current Frame Index for each real time acquisition (eg. for each scan)
	 */
	uint32_t getFrameIndex();

	/**
	 * Returns current Frame Index Caught for an entire  acquisition (including all scans)
	 */
	uint32_t getAcquisitionIndex();


	/**
	 * Returns if acquisition started
	 */
	bool getAcquistionStarted(){return acqStarted;};


	/**
	 * Returns if measurement started
	 */
	bool getMeasurementStarted(){return measurementStarted;};

	/**
	 * Set File Name (without frame index, file index and extension)
	 * @param c file name
	 */
	char* setFileName(char c[]);

	/**
	 * Set File Path
	 * @param c file path
	 */
	char* setFilePath(char c[]);

	/**
	 * Set File Index
	 * @param i file index
	 */
	int setFileIndex(int i);

	/**
	 * Set Frame Index Needed
	 * @param i frame index needed
	 */
	int setFrameIndexNeeded(int i){frameIndexNeeded = i; return frameIndexNeeded;};

	/**
	 * Set enable file write
	 * @param i file write enable
	 * Returns file write enable
	 */
	int setEnableFileWrite(int i);

	/**
	 * Resets the Total Frames Caught
	 * This is how the receiver differentiates between entire acquisitions
	 * Returns 0
	 */
	void resetTotalFramesCaught();


	/**
	 * Starts Receiver - starts to listen for packets
	 * @param message is the error message if there is an error
	 * Returns success
	 */
	int startReceiver(char message[]);

	/**
	 * Stops Receiver - stops listening for packets
	 * Returns success
	 */
	int stopReceiver();

	/**
	 * Static function - Thread started which listens to packets.
	 * Called by startReceiver()
	 * @param this_pointer pointer to this object
	 */
	static void* startListeningThread(void *this_pointer);

	/**
	 * Thread started which listens to packets.
	 * Called by startReceiver()
	 *
	 */
	int startListening();

	/**
	 * Static function - Thread started which writes packets to file.
	 * Called by startReceiver()
	 * @param this_pointer pointer to this object
	 */
	static void* startWritingThread(void *this_pointer);

	/**
	 * Thread started which writes packets to file.
	 * Called by startReceiver()
	 *
	 */
	int startWriting();

	/**
	 * Returns the buffer-current frame read by receiver
	 * @param c pointer to current file name
	 * @param raw address of pointer, pointing to current frame to send to gui
	 */
	void readFrame(char* c,char** raw);

	/**
	 * Set short frame
	 * @param i if shortframe i=1
	 */
	int setShortFrame(int i);

	/**
	 * Set the variable to send every nth frame to gui
	 * or if 0,send frame only upon gui request
	 */
	int setNFrameToGui(int i){if(i>=0) nFrameToGui = i; return nFrameToGui;};

private:

	/** detector type */
	detectorType myDetectorType;

	/** max frames per file **/
	uint32_t maxFramesPerFile;

	/** File write enable */
	int enableFileWrite;

	/** Complete File name */
	char savefilename[MAX_STR_LENGTH];

	/** File Name without frame index, file index and extension*/
	char fileName[MAX_STR_LENGTH];

	/** File Path */
	char filePath[MAX_STR_LENGTH];

	/** File Index */
	int fileIndex;

	/** if frame index required in file name */
	int frameIndexNeeded;

	/** Frames Caught for each real time acquisition (eg. for each scan) */
	uint32_t framesCaught;

	/* Acquisition started */
	bool acqStarted;

	/* Measurement started */
	bool measurementStarted;

	/** Frame index at start of each real time acquisition (eg. for each scan) */
	uint32_t startFrameIndex;

	/** Actual current frame index of each time acquisition (eg. for each scan) */
	uint32_t frameIndex;

	/** Total Frames Caught for an entire acquisition (including all scans) */
	int totalFramesCaught;

	/** Frame index at start of an entire acquisition (including all scans) */
	uint32_t startAcquisitionIndex;

	/** Actual current frame index of an entire acquisition (including all scans) */
	uint32_t acquisitionIndex;

	/** Frames currently in current file, starts new file when it reaches max */
	uint32_t framesInFile;

	/** Previous Frame number from buffer */
	uint32_t prevframenum;

	/** thread listening to packets */
	pthread_t   listening_thread;

	/** thread writing packets */
	pthread_t   writing_thread;

	/** mutex for locking variable used by different threads */
	pthread_mutex_t status_mutex;

	/** listening thread running */
	int listening_thread_running;

	/** writing thread running */
	int writing_thread_running;

	/** status of receiver */
	runStatus status;

	/** Receiver buffer */
	char* buffer;

	/** Receiver buffer */
	char *mem0, *memfull;

	/** latest data */
	char* latestData;

	/** UDP Socket between Receiver and Detector */
	genericSocket* udpSocket;

	/** Server UDP Port*/
	int server_port;

	/** ethernet interface or IP to listen to */
	char *eth;

	/** Element structure to put inside a fifo */
	struct dataStruct {
	char* buffer;
	int rc;
	};

	/** circular fifo to read and write data*/
	CircularFifo<char>* fifo;

	/** circular fifo to read and write data*/
	CircularFifo<char>* fifofree;

	/** fifo size */
	unsigned int fifosize;

	/** short frames */
	int shortFrame;

	/** buffer size can be 1286*2 or 518 or 1286*40 */
	uint32_t bufferSize;

	/** number of packets per frame*/
	uint32_t packetsPerFrame;
	
	/** gui data ready */
	int guiDataReady;

	/** points to the data to send to gui */
	char* guiData;

	/** points to the filename to send to gui */
	char* guiFileName;

	/** current frame number */
	uint32_t currframenum;

	/** send every nth frame to gui or only upon gui request*/
	int nFrameToGui;


	/**
	   callback arguments are
	   filepath
	   filename
	   fileindex
	   data size
	   
	   return value is 
	   0 callback takes care of open,close,write file
	   1 callback writes file, we have to open, close it
	   2 we open, close, write file, callback does not do anything

	*/
	int (*startAcquisitionCallBack)(char*, char*,int, int, void*);
	void *pStartAcquisition;

	/**
	   args to acquisition finished callback
	   total frames caught
	   
	*/
	void (*acquisitionFinishedCallBack)(int, void*);
	void *pAcquisitionFinished;


	/**
	  args to raw data ready callback are
	  framenum
	  datapointer
	  datasize in bytes
	  file descriptor
	  guidatapointer (NULL, no data required)
	*/
	void (*rawDataReadyCallBack)(int, char*, int, FILE*, char*, void*);
	void *pRawDataReady;

	/** The action which decides what the user and default responsibilites to save data are
	 * 0 raw data ready callback takes care of open,close,write file
	 * 1 callback writes file, we have to open, close it
	 * 2 we open, close, write file, callback does not do anything */
	int cbAction;

	/**temporary variable to test moench with gotthard module*/
	bool withGotthard;

	/** frame index mask */
	int frameIndexMask;

	/** frame index offset */
	int frameIndexOffset;





public:
	/** File Descriptor */
	static FILE *sfilefd;

	/** if the receiver threads are running*/
	static int receiver_threads_running;


	/**
	   callback arguments are
	   filepath
	   filename
	   fileindex
	   datasize
	   
	   return value is 
	   0 callback takes care of open,close,wrie file
	   1 callback writes file, we have to open, close it
	   2 we open, close, write file, callback does not do anything

	*/
	void registerCallBackStartAcquisition(int (*func)(char*, char*,int, int, void*),void *arg){startAcquisitionCallBack=func; pStartAcquisition=arg;};

	/**
	   callback argument is
	   toatal frames caught

	*/
	void registerCallBackAcquisitionFinished(void (*func)(int, void*),void *arg){acquisitionFinishedCallBack=func; pAcquisitionFinished=arg;};
	
	/**
	  args to raw data ready callback are
	  framenum
	  datapointer
	  datasize in bytes
	  file descriptor
	  guidatapointer (NULL, no data required)
	*/
	void registerCallBackRawDataReady(void (*func)(int, char*, int, FILE*, char*, void*),void *arg){rawDataReadyCallBack=func; pRawDataReady=arg;};
};


#endif

#endif
