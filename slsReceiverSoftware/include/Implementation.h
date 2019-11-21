#pragma once
#include "container_utils.h"
#include "logger.h"
#include "receiver_defs.h"
#include "network_utils.h"
class GeneralData;
class Listener;
class DataProcessor;
class DataStreamer;
class Fifo;
class slsDetectorDefs;

#include <atomic>
#include <exception>
#include <memory>
#include <vector>

class Implementation : private virtual slsDetectorDefs {
  public:
    Implementation();
    virtual ~Implementation();


    /**************************************************
     *                                                 *
     *   Configuration Parameters                      *
     *                                                 *
     * ************************************************/
    /**
     * Set receiver type. It is the first function called by the client when
     * connecting to receiver
     * @param d detector type
     * @return OK or FAIL
     */
    int setDetectorType(const detectorType d);
    int *getMultiDetectorSize() const;
    void setMultiDetectorSize(const int *size);
    int getDetectorPositionId() const;
    void setDetectorPositionId(const int id);
    std::string getDetectorHostname() const;
    void setDetectorHostname(const std::string& c);
    bool getSilentMode() const;
    void setSilentMode(const bool i);
    uint32_t getFifoDepth() const;
    int setFifoDepth(const uint32_t i);
    frameDiscardPolicy getFrameDiscardPolicy() const;
    void setFrameDiscardPolicy(const frameDiscardPolicy i);
    bool getFramePaddingEnable() const;
    void setFramePaddingEnable(const bool i);

    /**************************************************
     *                                                 *
     *   File Parameters                               *
     *                                                 *
     * ************************************************/
    fileFormat getFileFormat() const;
    void setFileFormat(slsDetectorDefs::fileFormat f);
    std::string getFilePath() const;
    /* check for existence */
    void setFilePath(const std::string& c);
    std::string getFileName() const;
    void setFileName(const std::string& c);
    uint64_t getFileIndex() const;
    void setFileIndex(const uint64_t i);
    bool getFileWriteEnable() const;
    void setFileWriteEnable(const bool b);
    bool getMasterFileWriteEnable() const;
    void setMasterFileWriteEnable(const bool b);
    bool getOverwriteEnable() const;
    void setOverwriteEnable(const bool b);
    uint32_t getFramesPerFile() const;
    /* 0 means infinite */
    void setFramesPerFile(const uint32_t i);

    /**************************************************
     *                                                 *
     *   Acquisition                                   *
     *                                                 *
     * ************************************************/
    runStatus getStatus() const;
    uint64_t getFramesCaught() const;
    uint64_t getAcquisitionIndex() const;
    std::vector<uint64_t> getNumMissingPackets() const;
    int startReceiver(std::string& err);
    void setStoppedFlag(bool stopped);
    void stopReceiver();
    void startReadout();
    void shutDownUDPSockets();
    void closeFiles();
    int restreamStop();


    /**************************************************
     *                                                 *
     *   Network Configuration (UDP)                   *
     *                                                 *
     * ************************************************/
    int getNumberofUDPInterfaces() const;
    /* [Jungfrau] */
    int setNumberofUDPInterfaces(const int n);
    std::string getEthernetInterface() const;
    void setEthernetInterface(const std::string &c);
    std::string getEthernetInterface2() const;
    /* [Jungfrau] */
    void setEthernetInterface2(const std::string &c);
    uint32_t getUDPPortNumber() const;
    void setUDPPortNumber(const uint32_t i);
    uint32_t getUDPPortNumber2() const;
    /* [Eiger][Jungfrau] */
    void setUDPPortNumber2(const uint32_t i);
    int64_t getUDPSocketBufferSize() const;
    int setUDPSocketBufferSize(const int64_t s);
    int64_t getActualUDPSocketBufferSize() const;    


    /**************************************************
     *                                                 *
     *   ZMQ Streaming Parameters (ZMQ)                *
     *                                                 *
     * ************************************************/
    bool getDataStreamEnable() const;
    int setDataStreamEnable(const bool enable);
    uint32_t getStreamingFrequency() const;
    /* 0 for timer */    
    int setStreamingFrequency(const uint32_t freq);
    uint32_t getStreamingTimer() const;
    void setStreamingTimer(const uint32_t time_in_ms);
    uint32_t getStreamingPort() const;
    void setStreamingPort(const uint32_t i);
    sls::IpAddr getStreamingSourceIP() const;
    void setStreamingSourceIP(const  sls::IpAddr ip);
    std::string getAdditionalJsonHeader() const;
    void setAdditionalJsonHeader(const std::string& c);

    /**************************************************
     *                                                 *
     *   Detector Parameters                           *
     *                                                 *
     * ************************************************/
    uint64_t getNumberOfFrames() const;
    void setNumberOfFrames(const uint64_t i);
    uint64_t getAcquisitionTime() const;
    void setAcquisitionTime(const uint64_t i);
    uint64_t getAcquisitionPeriod() const;
    void setAcquisitionPeriod(const uint64_t i);
    uint64_t getSubExpTime() const;
    /* [Eiger] */
    void setSubExpTime(const uint64_t i);
    uint64_t getSubPeriod() const;
    /* [Eiger] */
    void setSubPeriod(const uint64_t i);
    uint32_t getNumberofAnalogSamples() const;
    /**[Ctb][Moench] */
    int setNumberofAnalogSamples(const uint32_t i);
    uint32_t getNumberofDigitalSamples() const;
    /**[Ctb] */
    int setNumberofDigitalSamples(const uint32_t i);
    uint32_t getDynamicRange() const;
    int setDynamicRange(const uint32_t i);
    ROI getROI() const;
    /* [Gotthard] */
    int setROI(ROI arg);
    bool getTenGigaEnable() const;
    /* [Eiger][Ctb] */    
    int setTenGigaEnable(const bool b);
    int getFlippedDataX() const;
    void setFlippedDataX(int enable = -1);
    bool getGapPixelsEnable() const;
    /* [Eiger] */
    int setGapPixelsEnable(const bool b);
    bool getQuad() const;
    /* [Eiger] */
    int setQuad(const bool b);
    bool getActivate() const;
    /** [Eiger] If deactivated, receiver will create dummy data if deactivated padding is enabled (as it will receive nothing from detector) */
    bool setActivate(const bool enable);
    bool getDeactivatedPadding() const;
    /* [Eiger] */
    bool setDeactivatedPadding(const bool enable);
    int getReadNLines() const;
    /* [Eiger] */
    void setReadNLines(const int value);
    readoutMode getReadoutMode() const;
    /* [Ctb] */
    int setReadoutMode(const readoutMode f);
    uint32_t getADCEnableMask() const;
    /* [Ctb][Moench] */
    int setADCEnableMask(const uint32_t mask);
    std::vector<int> getDbitList() const;
    /* [Ctb] */
    void setDbitList(const std::vector<int> v);
    int getDbitOffset() const;
    /* [Ctb] */
    void setDbitOffset(const int s);

    /**************************************************
     *                                                *
     *    Callbacks                                   *
     *                                                *
     * ************************************************/
    void registerCallBackStartAcquisition(
      int (*func)(std::string, std::string, uint64_t, uint32_t, void *), void *arg);
    void registerCallBackAcquisitionFinished(
      void (*func)(uint64_t, void *), void *arg);
    void registerCallBackRawDataReady(
      void (*func)(char *, char *, uint32_t, void *), void *arg);
    void registerCallBackRawDataModifyReady(
      void (*func)(char *, char *, uint32_t &, void *), void *arg);

  private:
    void DeleteMembers();
    void InitializeMembers();
    void SetLocalNetworkParameters();
    void SetThreadPriorities();
    int SetupFifoStructure();
    
    void ResetParametersforNewAcquisition();
    int CreateUDPSockets();
    int SetupWriter();
    void StartRunning();


    /**************************************************
     *                                                *
     *    Class Members                               *
     *                                                *
     * ************************************************/
 
    // config parameters
    int numThreads;
    const static int MAX_NUMBER_OF_LISTENING_THREADS = 2;
    detectorType myDetectorType;
    int numDet[MAX_DIMENSIONS];
    int detID;
    std::string detHostname;
    bool silentMode;
    uint32_t fifoDepth;
    frameDiscardPolicy frameDiscardMode;
    bool framePadding;

    // file parameters
    fileFormat fileFormatType;
    std::string filePath;
    std::string fileName;
    uint64_t fileIndex;
    bool fileWriteEnable;
    bool masterFileWriteEnable;
    bool overwriteEnable;
    uint32_t framesPerFile;

    // acquisition
    std::atomic<runStatus> status;
    bool stoppedFlag;

    // network configuration (UDP)
    int numUDPInterfaces;
    std::vector <std::string> eth;
    uint32_t udpPortNum[MAX_NUMBER_OF_LISTENING_THREADS];
    int64_t udpSocketBufferSize;
    int64_t actualUDPSocketBufferSize;

    // zmq parameters
    bool dataStreamEnable;
    uint32_t streamingFrequency;
    uint32_t streamingTimerInMs;
    uint32_t streamingPort;
    sls::IpAddr streamingSrcIP;
    std::string additionalJsonHeader;

    // detector parameters
    uint64_t numberOfFrames;
    uint64_t acquisitionPeriod;
    uint64_t acquisitionTime;
    uint64_t subExpTime;
    uint64_t subPeriod;
    uint64_t numberOfAnalogSamples;
    uint64_t numberOfDigitalSamples;
    uint32_t dynamicRange;
    ROI roi;
    bool tengigaEnable;
    int flippedDataX;
    bool gapPixelsEnable;
    bool quadEnable; 
    bool activated;
    bool deactivatedPaddingEnable;
    int numLinesReadout;
    readoutMode readoutType;
    uint32_t adcEnableMask;
    std::vector<int> ctbDbitList;
    int ctbDbitOffset;
    int ctbAnalogDataBytes;

    // callbacks
    int (*startAcquisitionCallBack)(std::string, std::string, uint64_t, uint32_t, void *);
    void *pStartAcquisition;
    void (*acquisitionFinishedCallBack)(uint64_t, void *);
    void *pAcquisitionFinished;
    void (*rawDataReadyCallBack)(char *, char *, uint32_t, void *);
    void (*rawDataModifyReadyCallBack)(char *, char *, uint32_t &, void *);
    void *pRawDataReady;

    // class objects
    GeneralData *generalData;
    std::vector<std::unique_ptr<Listener>> listener;
    std::vector<std::unique_ptr<DataProcessor>> dataProcessor;
    std::vector<std::unique_ptr<DataStreamer>> dataStreamer;
    std::vector<std::unique_ptr<Fifo>> fifo;
};
