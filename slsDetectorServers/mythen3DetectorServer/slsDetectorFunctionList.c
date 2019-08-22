#include "slsDetectorFunctionList.h"
#include "versionAPI.h"
#include "clogger.h"

#ifdef VIRTUAL
#include "communication_funcs_UDP.h"
#include <pthread.h>
#include <time.h>
#endif
#include <string.h>
#include <unistd.h>     // usleep
#include "blackfin.h"


// Global variable from slsDetectorServer_funcs
extern int debugflag;

int firmware_compatibility = OK;
int firmware_check_done = 0;
char firmware_message[MAX_STR_LENGTH];

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
int virtual_status = 0;
int virtual_stop = 0;
#endif



int isFirmwareCheckDone() {
	return firmware_check_done;
}

int getFirmwareCheckResult(char** mess) {
	*mess = firmware_message;
	return firmware_compatibility;
}

void basictests() {
    firmware_compatibility = OK;
    firmware_check_done = 0;
    memset(firmware_message, 0, MAX_STR_LENGTH);
#ifdef VIRTUAL
    FILE_LOG(logINFOBLUE, ("******** Mythen3 Virtual Server *****************\n"));
    if (mapCSP0() == FAIL) {
    	strcpy(firmware_message,
				"Could not map to memory. Dangerous to continue.\n");
		FILE_LOG(logERROR, (firmware_message));
		firmware_compatibility = FAIL;
		firmware_check_done = 1;
		return;
    }
    firmware_check_done = 1;
    return;
#else

	
#endif
}

/* Ids */

int64_t getDetectorId(enum idMode arg){
	int64_t retval = -1;

	switch(arg){
	case DETECTOR_SERIAL_NUMBER:
		return getDetectorNumber();// or getDetectorMAC()
	case DETECTOR_FIRMWARE_VERSION:
		return getFirmwareVersion();
	case SOFTWARE_FIRMWARE_API_VERSION:
	    return getFirmwareAPIVersion();
	case DETECTOR_SOFTWARE_VERSION:
	case CLIENT_SOFTWARE_API_VERSION:
		return APIMYTHEN3;
	default:
		return retval;
	}
}

u_int64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return 0;
#endif
	return 0;
}

u_int64_t getFirmwareAPIVersion() {
#ifdef VIRTUAL
    return 0;
#endif
    return 0;
}

u_int32_t getDetectorNumber(){
#ifdef VIRTUAL
    return 0;
#endif
	return 0;
}


u_int64_t  getDetectorMAC() {
#ifdef VIRTUAL
    return 0;
#else
	char output[255],mac[255]="";
	u_int64_t res=0;
	FILE* sysFile = popen("ifconfig eth0 | grep HWaddr | cut -d \" \" -f 11", "r");
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);
	//getting rid of ":"
	char * pch;
	pch = strtok (output,":");
	while (pch != NULL){
		strcat(mac,pch);
		pch = strtok (NULL, ":");
	}
	sscanf(mac,"%llx",&res);
	return res;
#endif
}

u_int32_t  getDetectorIP(){
#ifdef VIRTUAL
    return 0;
#endif
	char temp[50]="";
	u_int32_t res=0;
	//execute and get address
	char output[255];
	FILE* sysFile = popen("ifconfig  | grep 'inet addr:'| grep -v '127.0.0.1' | cut -d: -f2", "r");
	fgets(output, sizeof(output), sysFile);
	pclose(sysFile);

	//converting IPaddress to hex.
	char* pcword = strtok (output,".");
	while (pcword != NULL) {
		sprintf(output,"%02x",atoi(pcword));
		strcat(temp,output);
		pcword = strtok (NULL, ".");
	}
	strcpy(output,temp);
	sscanf(output, "%x", 	&res);
	//FILE_LOG(logINFO, ("ip:%x\n",res);

	return res;
}


/* initialization */

void initControlServer(){
	setupDetector();
}

void initStopServer() {

	usleep(CTRL_SRVR_INIT_TIME_US);
	if (mapCSP0() == FAIL) {
		FILE_LOG(logERROR, ("Stop Server: Map Fail. Dangerous to continue. Goodbye!\n"));
		exit(EXIT_FAILURE);
	}
}


/* set up detector */

void setupDetector() {
    FILE_LOG(logINFO, ("This Server is for 1 Mythen3 module \n")); 
}


/* set parameters -  dr, roi */

int setDynamicRange(int dr){
	return -1;
}


/* parameters - speed, readout */

void setSpeed(enum speedVariable ind, int val) {

}

int getSpeed(enum speedVariable ind) {
    return -1;
}

int64_t setTimer(enum timerIndex ind, int64_t val) {

	switch(ind){

	case FRAME_NUMBER:
		
	case ACQUISITION_TIME:
		
	case FRAME_PERIOD:
		
	case CYCLES_NUMBER:
        return -1;

	default:
		break;
	}

	return -1;

}

int validateTimer(enum timerIndex ind, int64_t val, int64_t retval) {

    switch(ind) {
    case ACQUISITION_TIME:
  
    case FRAME_PERIOD:

    default:
        break;
    }
    return OK;
}


int64_t getTimeLeft(enum timerIndex ind){
#ifdef VIRTUAL
    return 0;
#endif

	return -1;
}

int startStateMachine(){
#ifdef VIRTUAL
	// create udp socket
	if(createUDPSocket(0) != OK) {
		return FAIL;
	}
	FILE_LOG(logINFOBLUE, ("starting state machine\n"));
	virtual_status = 1;
	virtual_stop = 0;
	if(pthread_create(&pthread_virtual_tid, NULL, &start_timer, NULL)) {
		FILE_LOG(logERROR, ("Could not start Virtual acquisition thread\n"));
		virtual_status = 0;
		return FAIL;
	}
	FILE_LOG(logINFOGREEN, ("Virtual Acquisition started\n"));
	return OK;
#endif
    return OK;
}


#ifdef VIRTUAL
void* start_timer(void* arg) {
	int64_t periodns = setTimer(FRAME_PERIOD, -1);
	int numFrames = (setTimer(FRAME_NUMBER, -1) *
						setTimer(CYCLES_NUMBER, -1) );
	int64_t exp_ns = 	setTimer(ACQUISITION_TIME, -1);


    int frameNr = 0;
    for(frameNr=0; frameNr!= numFrames; ++frameNr ) {
        int srcOffset = 0;
    
        struct timespec begin, end;
        clock_gettime(CLOCK_REALTIME, &begin);

        usleep(exp_ns / 1000);

        clock_gettime(CLOCK_REALTIME, &end);
        int64_t time_ns = ((end.tv_sec - begin.tv_sec) * 1E9 +
                (end.tv_nsec - begin.tv_nsec));

        if (periodns > time_ns) {
            usleep((periodns - time_ns)/ 1000);
        }
    }

	virtual_status = 0;
	return NULL;
}
#endif


int stopStateMachine(){
	FILE_LOG(logINFORED, ("Stopping State Machine\n"));
#ifdef VIRTUAL
	virtual_stop = 0;
	return OK;
#endif
    return OK;
}

enum runStatus getRunStatus(){
#ifdef VIRTUAL
	if(virtual_status == 0){
		FILE_LOG(logINFOBLUE, ("Status: IDLE\n"));
		return IDLE;
	}else{
		FILE_LOG(logINFOBLUE, ("Status: RUNNING\n"));
		return RUNNING;
	}
#endif
    return IDLE;
}

void readFrame(int *ret, char *mess){
#ifdef VIRTUAL
	FILE_LOG(logINFOGREEN, ("acquisition successfully finished\n"));
	return;
#endif
}

/* common */

int calculateDataBytes(){
	return 0;
}

int getTotalNumberOfChannels(){return  ((int)getNumberOfChannelsPerChip() * (int)getNumberOfChips());}
int getNumberOfChips(){return  NCHIP;}
int getNumberOfDACs(){return  NDAC;}
int getNumberOfChannelsPerChip(){return  NCHAN;}