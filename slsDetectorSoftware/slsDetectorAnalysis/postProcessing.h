#ifndef POSTPROCESSING_H
#define POSTPROCESSING_H


#include "detectorData.h"
#include "slsDetectorBase.h"
#include "angularConversion.h"
#include "badChannelCorrections.h"
#include "fileIO.h"
#include <string>

#include <iostream>
#include <fstream>
#include <iomanip>
#include <cstring>
#include <string>
#include <sstream>
#include <queue>
#include <math.h>

class postProcessingFuncs;



using namespace std;

#define MAX_BADCHANS 20000


#define defaultTDead {170,90,750} /**< should be changed in order to have it separate for the different detector types */


/**
   @short methods for data postprocessing 

   (including thread for writing data files and plotting in parallel with the acquisition) 
*/

class postProcessing : public angularConversion, public fileIO, public badChannelCorrections {

//public virtual angularConversion, public virtual fileIO  {

 public:
  postProcessing();
  virtual ~postProcessing();




  /**
      get bad channels correction
      \param bad pointer to array that if bad!=NULL will be filled with the bad channel list
      \returns 0 if bad channel disabled or no bad channels, >0 otherwise
  */
  virtual int getBadChannelCorrection(int *bad=NULL)=0;


  /** 
      get flat field corrections
      \param corr if !=NULL will be filled with the correction coefficients
      \param ecorr if !=NULL will be filled with the correction coefficients errors
      \returns 0 if ff correction disabled, >0 otherwise
  */
  virtual int getFlatFieldCorrection(double *corr=NULL, double *ecorr=NULL)=0;
  
  /** 
      set flat field corrections
      \param corr if !=NULL the flat field corrections will be filled with corr (NULL usets ff corrections)
      \param ecorr if !=NULL the flat field correction errors will be filled with ecorr (1 otherwise)
      \returns 0 if ff correction disabled, >0 otherwise
  */
  virtual int  setFlatFieldCorrection(double *corr, double *ecorr=NULL)=0;
  
  /**
      set bad channels correction
      \param fname file with bad channel list ("" disable)
      \returns 0 if bad channel disabled, >0 otherwise
  */
  virtual int setBadChannelCorrection(string fname="")=0;

 
  /** 
      set bad channels correction
      \param fname file with bad channel list ("" disable)
      \param nbad reference to number of bad channels
      \param badlist array of badchannels
      \returns 0 if bad channel disabled, >0 otherwise
  */
  virtual int setBadChannelCorrection(string fname, int &nbad, int *badlist, int off=0)=0;
  using badChannelCorrections::setBadChannelCorrection;
  
  /** 
      set bad channels correction
      \param nch number of bad channels
      \param chs array of channels
      \param ff 0 if normal bad channels, 1 if ff bad channels
      \returns 0 if bad channel disabled, >0 otherwise
  */
  virtual int setBadChannelCorrection(int nch, int *chs, int ff=0)=0; 
  


   int enableWriteToFile(int i=-1) {if (i>0) ((*correctionMask)|=(1<<WRITE_FILE)); else if (i==0) ((*correctionMask)&=~(1<< WRITE_FILE)); return (((*correctionMask)&(1<< WRITE_FILE ))>>WRITE_FILE) ;};


   int setAngularCorrectionMask(int i=-1){if (i==0) (*correctionMask)&=~(1<< ANGULAR_CONVERSION); if (i>0) (*correctionMask)|=(1<< ANGULAR_CONVERSION); return (((*correctionMask)&(1<< ANGULAR_CONVERSION))>>ANGULAR_CONVERSION);};



  int enableAngularConversion(int i=-1) {if (i>0) return setAngularConversionFile("default"); if (i==0) return setAngularConversionFile(""); return setAngularCorrectionMask();};


  int enableBadChannelCorrection(int i=-1) {if (i>0) return setBadChannelCorrection("default"); if (i==0) return setBadChannelCorrection(""); return (((*correctionMask)&(1<< DISCARD_BAD_CHANNELS))>>DISCARD_BAD_CHANNELS);};


  
  
  /** returns the bad channel list file */
  string getBadChannelCorrectionFile() {if ((*correctionMask)&(1<< DISCARD_BAD_CHANNELS)) return string(badChanFile); else return string("none");};


  /** 
      get flat field corrections file directory
      \returns flat field correction file directory
  */
  string getFlatFieldCorrectionDir(){return string(flatFieldDir);};
 /** 
      set flat field corrections file directory
      \param flat field correction file directory
      \returns flat field correction file directory
  */
  string setFlatFieldCorrectionDir(string dir){strcpy(flatFieldDir,dir.c_str()); return string(flatFieldDir);};
  
 /** 
      get flat field corrections file name
      \returns flat field correction file name
  */
  string getFlatFieldCorrectionFile(){  if ((*correctionMask)&(1<<FLAT_FIELD_CORRECTION)) return string(flatFieldFile); else return string("none");};



  /** 
      set/get if the data processing and file writing should be done by a separate thread
s
      \param b 0 sequencial data acquisition and file writing, 1 separate thread, -1 get
      \returns thread flag
  */

  int setThreadedProcessing(int b=-1) {if (b>=0) *threadedProcessing=b; return  *threadedProcessing;};




  /** processes the data
      \param delflag 0 leaves the data in the final data queue
      \returns nothing
      
  */
  void *processData(int delflag);

  /** processes the data
      \param delflag 0 leaves the data in the final data queue
      \returns nothing
      
  */
  void processFrame(int* myData, int delflag);

  /** processes the data
      \param delflag 0 leaves the data in the final data queue
      \returns nothing
      
  */
  void doProcessing(double* myData, int delflag, string fname);


  /**
   pops the data from the data queue
    \returns pointer to the popped data  or NULL if the queue is empty. 
    \sa  dataQueue
  */ 
  int* popDataQueue();

  int* dataQueueFront();

  int dataQueueSize();

/*   /\** */
/*    pops the data from thepostprocessed data queue */
/*     \returns pointer to the popped data  or NULL if the queue is empty.  */
/*     \sa  finalDataQueue */
/*   *\/  */
/*   detectorData* popFinalDataQueue(); */


  int checkJoinThread();
  void setJoinThread(int v);


  /**
  resets the raw data queue
    \sa  dataQueue
  */ 
  void resetDataQueue();

/*   /\** */
/*   resets the postprocessed  data queue */
/*     \sa  finalDataQueue */
/*   *\/  */
/*   void resetFinalDataQueue(); */





   int fillBadChannelMask();
    



  virtual int rateCorrect(double*, double*, double*, double*)=0;
  virtual int flatFieldCorrect(double*, double*, double*, double*)=0;


  virtual int fillModuleMask(int *mM)=0;

  virtual int getNMods()=0;


  int GetCurrentPositionIndex(){ pthread_mutex_lock(&mp); int retval=getCurrentPositionIndex();  pthread_mutex_unlock(&mp); return retval;};
  void IncrementPositionIndex(){ pthread_mutex_lock(&mp); incrementPositionIndex();  pthread_mutex_unlock(&mp);};

  void IncrementFileIndex(){ pthread_mutex_lock(&mp);  incrementFileIndex();  pthread_mutex_unlock(&mp); };
  int GetFileIndex(){ pthread_mutex_lock(&mp); int i=*fileIndex;  pthread_mutex_unlock(&mp); return i;};
  
  void ResetPositionIndex(){pthread_mutex_lock(&mp); resetPositionIndex();  pthread_mutex_unlock(&mp);};


  void registerDataCallback(int( *userCallback)(detectorData*, void*),  void *pArg) {dataReady = userCallback; pCallbackArg = pArg;};
  

  void registerRawDataCallback(int( *userCallback)(double*, int, void*),  void *pArg) {rawDataReady = userCallback; pRawDataArg = pArg;};
  
  void registerProgressCallback(int( *func)(double,void*), void *pArg){progress_call=func; pProgressCallArg=pArg;};

  virtual double getRateCorrectionTau()=0;


  int positionFinished(int v=-1){pthread_mutex_lock(&mp); if (v>=0) posfinished=v; int retval=posfinished; pthread_mutex_unlock(&mp); return retval;};

  double getCurrentPosition() {double p; pthread_mutex_lock(&mp); p=currentPosition; pthread_mutex_unlock(&mp); return p;};
  int setCurrentPosition(double v) { pthread_mutex_lock(&mp); currentPosition=v; pthread_mutex_unlock(&mp); return currentPosition;};




  void initDataset(int refresh);
  void addFrame(double *data, double pos, double i0, double t, string fname, double var);
  void finalizeDataset(double *a, double *v, double *e, int &np); 

 virtual detectorType getDetectorsType(int pos=-1)=0;

 protected:
     
  int *threadedProcessing;

  int *correctionMask;

  char *flatFieldDir;
  char *flatFieldFile;
  
  int64_t *expTime;

  /** mutex to synchronize main and data processing threads */
  pthread_mutex_t mp;


 /** mutex to synchronizedata processing and plotting threads */
   pthread_mutex_t mg;

 /** sets when the acquisition is finished */
  int jointhread;

 /** sets when the position is finished */
  int posfinished;  

 /**
     data queue
  */
  queue<int*> dataQueue;
  /**
     queue containing the postprocessed data
  */
  queue<detectorData*> finalDataQueue;
  

 /** data queue size */
  int queuesize;


  

  /**
    start data processing thread
  */
  void startThread(int delflag=1); //
  /** the data processing thread */

  pthread_t dataProcessingThread;



  /** pointer to bad channel mask  0 is channel is good 1 if it is bad \sa fillBadChannelMask() */ 
  int *badChannelMask;




  /**
     I0 measured
  */
  double currentI0;
  

  int arraySize;


 private:
  double *fdata;
  
  int (*dataReady)(detectorData*,void*); 
  void *pCallbackArg; 
  
  int (*rawDataReady)(double*,int,void*); 
  void *pRawDataArg; 
  
  int (*progress_call)(double,void*);
  void *pProgressCallArg;

  
  postProcessingFuncs *ppFun;
  detectorData *thisData;
  

  double *ang;
  double *val;
  double *err;

  int numberOfChannels;








};


static void* startProcessData(void *n){\
   postProcessing *myDet=(postProcessing*)n;\
   myDet->processData(1);\
   pthread_exit(NULL);\
   
};

static void* startProcessDataNoDelete(void *n){\
  postProcessing *myDet=(postProcessing*)n;\
  myDet->processData(0);\
  pthread_exit(NULL);\

};



#endif
