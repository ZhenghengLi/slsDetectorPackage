#include "Receiver.h"
#include "sls_detector_exceptions.h"
#include "versionAPI.h"
#include "container_utils.h" 
#include "logger.h"

#include <cstdlib>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

Receiver::Receiver(int argc, char *argv[]):
		tcpipInterface (nullptr) {

	// options
	std::map<std::string, std::string> configuration_map;
	int tcpip_port_no = 1954;

	//parse command line for config
	static struct option long_options[] = {
			// These options set a flag.
			//{"verbose", no_argument,       &verbose_flag, 1},
			// These options don’t set a flag. We distinguish them by their indices.
			{"rx_tcpport",  required_argument,  nullptr, 't'},
			{"version",  	no_argument,  		nullptr, 'v'},
			{"help",  		no_argument,       	nullptr, 'h'},
			{nullptr, 			0, 					nullptr, 	0}
	};

	//initialize global optind variable (required when instantiating multiple receivers in the same process)
	optind = 1;
	// getopt_long stores the option index here.
	int option_index = 0;
	int c = 0;

	while ( c != -1 ){
		c = getopt_long (argc, argv, "hvf:t:", long_options, &option_index);

		// Detect the end of the options.
		if (c == -1)
			break;

		switch(c){

		case 't':
			sscanf(optarg, "%d", &tcpip_port_no);
			break;

		case 'v':
			std::cout << "SLS Receiver " << GITBRANCH << " (0x" << std::hex << APIRECEIVER << ")" << std::endl;
			throw sls::RuntimeError();

		case 'h':
		default:
			std::string help_message = "\n"
					+ std::string(argv[0]) + "\n"
					+ "Usage: " + std::string(argv[0]) + " [arguments]\n"
					+ "Possible arguments are:\n"
					+ "\t-t, --rx_tcpport <port> : TCP Communication Port with client. \n"
					+ "\t                          Default: 1954. Required for multiple \n"
					+ "\t                          receivers\n\n";

			FILE_LOG(logINFO) << help_message << std::endl;
			throw sls::RuntimeError();

		}
	}

	// might throw an exception
	tcpipInterface = sls::make_unique<ClientInterface>(tcpip_port_no);
}


Receiver::Receiver(int tcpip_port_no)
{
	// might throw an exception
	tcpipInterface = sls::make_unique<ClientInterface>(tcpip_port_no);
}

int64_t Receiver::getReceiverVersion(){
	return tcpipInterface->getReceiverVersion();
}


void Receiver::registerCallBackStartAcquisition(int (*func)(
		std::string, std::string, uint64_t, uint32_t, void*),void *arg){
	tcpipInterface->registerCallBackStartAcquisition(func,arg);
}


void Receiver::registerCallBackAcquisitionFinished(
		void (*func)(uint64_t, void*),void *arg){
	tcpipInterface->registerCallBackAcquisitionFinished(func,arg);
}


void Receiver::registerCallBackRawDataReady(void (*func)(char*,
		char*, uint32_t, void*),void *arg){
	tcpipInterface->registerCallBackRawDataReady(func,arg);
}


void Receiver::registerCallBackRawDataModifyReady(void (*func)(char*,
        char*, uint32_t &, void*),void *arg){
	tcpipInterface->registerCallBackRawDataModifyReady(func,arg);
}
