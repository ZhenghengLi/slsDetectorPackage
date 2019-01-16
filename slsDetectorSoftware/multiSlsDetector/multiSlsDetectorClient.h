#pragma once
#include <iostream>
#include <string>

#include "container_utils.h"
#include "multiSlsDetector.h"
#include "multiSlsDetectorCommand.h"
#include "sls_detector_exceptions.h"

#include <cstdlib>
#include <memory>

inline int dummyCallback(detectorData *d, int p, void *) {
    std::cout << "got data " << p << std::endl;
    return 0;
};

class multiSlsDetectorClient {
  public:
    multiSlsDetectorClient(int argc, char *argv[], int action, multiSlsDetector *myDetector = nullptr) {
        int id = -1, pos = -1, iv = 0;
        bool verify = true, update = true;
        char cmd[100] = "";

        if (action == slsDetectorDefs::PUT_ACTION && argc < 2) {
            std::cout << "Wrong usage - should be: " << argv[0] << "[id-][pos:]channel arg" << std::endl;
            std::cout << std::endl;
            return;
        };
        if (action == slsDetectorDefs::GET_ACTION && argc < 1) {
            std::cout << "Wrong usage - should be: " << argv[0] << "[id-][pos:]channel arg" << std::endl;
            std::cout << std::endl;
            return;
        };

        if (action == slsDetectorDefs::READOUT_ACTION) {
            id = 0;
            pos = -1;
            if (argc) { // multi id scanned
                if (strchr(argv[0], '-')) {
                    iv = sscanf(argv[0], "%d-%s", &id, cmd); //%s needn't be there (if not 1:), so 1 or 2 arguments scanned
                    if (iv >= 1 && id >= 0) {
                        argv[0] = cmd;
                        std::cout << id << "-";
                    } else {
                        id = 0;
                    }
                } // single id scanned
                if (strchr(argv[0], ':')) {
                    iv = sscanf(argv[0], "%d:", &pos);
                    if (iv == 1 && pos >= 0) {
                        std::cout << "pos " << pos << " is not allowed for readout!" << std::endl;
                        return;
                    }
                }
            }
        } else {                                     // multi id scanned
            iv = sscanf(argv[0], "%d-%s", &id, cmd); // scan success
            if (iv == 2 && id >= 0) {
                argv[0] = cmd;
                std::cout << id << "-";
            } else {
                id = 0;
            }                                         // sls pos scanned
            iv = sscanf(argv[0], "%d:%s", &pos, cmd); // scan success
            if (iv == 2 && pos >= 0) {
                argv[0] = cmd;
                std::cout << pos << ":";
            }
            if (iv != 2) {
                pos = -1;
            } // remove the %d- and %d:
            if (strlen(cmd) == 0u) {
                strcpy(cmd, argv[0]);
            }                       
			
			// special commands
            std::string scmd = cmd; // free without calling multiSlsDetector constructor
            if (scmd == "free") {
                multiSlsDetector::freeSharedMemory(id, pos);
                return;
            } // get user details without verify sharedMultiSlsDetector version
            else if ((scmd == "user") && (action == slsDetectorDefs::GET_ACTION)) {
                verify = false;
                update = false;
            }
        }
        //std::cout<<"id:"<<id<<" pos:"<<pos<<std::endl;
        // create multiSlsDetector class if required
        std::unique_ptr<multiSlsDetector> localDet;
        if (myDetector == nullptr) {
            try {
                localDet = sls::make_unique<multiSlsDetector>(id, verify, update);
                myDetector = localDet.get();
            } catch (const SlsDetectorPackageExceptions &e) {
                /*std::cout << e.GetMessage() << std::endl;*/
                return;
            } catch (...) {
                std::cout << " caught exception" << std::endl;
                return;
            }
        }
        if (pos >= myDetector->getNumberOfDetectors()) {
            std::cout << "position is out of bounds." << std::endl;
            return;
        }

        // call multi detector command line
        multiSlsDetectorCommand myCmd(myDetector);
        std::string answer = myCmd.executeLine(argc, argv, action, pos);

        if (action != slsDetectorDefs::READOUT_ACTION) {
            std::cout << argv[0] << " ";
        }
        std::cout << answer << std::endl;
    }
};
