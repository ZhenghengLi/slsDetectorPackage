#include "slsDetectorFunctionList.h"
#include "RegisterDefs.h"
#include "versionAPI.h"
#include "clogger.h"
#include "nios.h"
#include "DAC6571.h"
#include "LTC2620_Driver.h"
#include "common.h"
#include "ALTERA_PLL_CYCLONE10.h" 
#include "ASIC_Driver.h"
#ifdef VIRTUAL
#include "communication_funcs_UDP.h"
#endif

#include <string.h>
#include <unistd.h>     // usleep
#ifdef VIRTUAL
#include <pthread.h>
#include <time.h>
#endif


// Global variable from slsDetectorServer_funcs
extern int debugflag;
extern udpStruct udpDetails;

int initError = OK;
int initCheckDone = 0;
char initErrorMessage[MAX_STR_LENGTH];

#ifdef VIRTUAL
pthread_t pthread_virtual_tid;
int virtual_status = 0;
int virtual_stop = 0;
#endif

int32_t clkPhase[NUM_CLOCKS] = {0, 0, 0, 0, 0, 0};
uint32_t clkFrequency[NUM_CLOCKS] = {0, 0, 0, 0, 0, 0};
int highvoltage = 0;
int dacValues[NDAC] = {0};
int onChipdacValues[ONCHIP_NDAC][NCHIP] = {0};
int defaultDacValues[NDAC] = {0};
int defaultOnChipdacValues[ONCHIP_NDAC][NCHIP] = {0};
int injectedChannelsOffset = 0;
int injectedChannelsIncrement = 0;
int vetoReference[NCHIP][NCHAN];
int detPos[2] = {0, 0};

int isInitCheckDone() {
	return initCheckDone;
}

int getInitResult(char** mess) {
	*mess = initErrorMessage;
	return initError;
}

void basictests() {
    initError = OK;
    initCheckDone = 0;
    memset(initErrorMessage, 0, MAX_STR_LENGTH);
#ifdef VIRTUAL
    FILE_LOG(logINFOBLUE, ("******** Gotthard2 Virtual Server *****************\n"));
    if (mapCSP0() == FAIL) {
    	strcpy(initErrorMessage,
				"Could not map to memory. Dangerous to continue.\n");
		FILE_LOG(logERROR, (initErrorMessage));
		initError = FAIL;
		return;
    }
    return;
#else
	if (mapCSP0() == FAIL) {
    	strcpy(initErrorMessage,
				"Could not map to memory. Dangerous to continue.\n");
		FILE_LOG(logERROR, ("%s\n\n", initErrorMessage));
		initError = FAIL;
		return;
    }
	// does check only if flag is 0 (by default), set by command line
	if ((!debugflag) && ((testFpga() == FAIL) || (testBus() == FAIL))) {
		sprintf(initErrorMessage,
				"Could not pass basic tests of FPGA and bus. Dangerous to continue. (Firmware version:0x%llx) \n", getFirmwareVersion());
		FILE_LOG(logERROR, ("%s\n\n", initErrorMessage));
		initError = FAIL;
		return;
	}

	uint16_t hversion			= getHardwareVersionNumber();
	uint32_t ipadd				= getDetectorIP();
	uint64_t macadd				= getDetectorMAC();
	int64_t fwversion 			= getFirmwareVersion();
	int64_t swversion 			= getServerVersion();
	int64_t sw_fw_apiversion    = getFirmwareAPIVersion();
	int64_t client_sw_apiversion = getClientServerAPIVersion();
	uint32_t requiredFirmwareVersion = REQRD_FRMWRE_VRSN;

	FILE_LOG(logINFOBLUE, ("************ Gotthard2 Server *********************\n"
			"Hardware Version:\t\t 0x%x\n"
			
			"Detector IP Addr:\t\t 0x%x\n"
			"Detector MAC Addr:\t\t 0x%llx\n\n"

			"Firmware Version:\t\t 0x%llx\n"
			"Software Version:\t\t 0x%llx\n"
			"F/w-S/w API Version:\t\t 0x%llx\n"
			"Required Firmware Version:\t 0x%x\n"
			"Client-Software API Version:\t 0x%llx\n"
			"********************************************************\n",
			hversion, 
			ipadd,
			(long  long unsigned int)macadd,
			(long  long int)fwversion,
			(long  long int)swversion,
			(long  long int)sw_fw_apiversion,
			requiredFirmwareVersion,
			(long long int)client_sw_apiversion
	));

	// return if flag is not zero, debug mode
	if (debugflag) {
		return;
	}

	//cant read versions
    FILE_LOG(logINFO, ("Testing Firmware-software compatibility:\n"));
	if(!fwversion || !sw_fw_apiversion){
		strcpy(initErrorMessage,
				"Cant read versions from FPGA. Please update firmware.\n");
		FILE_LOG(logERROR, (initErrorMessage));
		initError = FAIL;
		return;
	}

	//check for API compatibility - old server
	if(sw_fw_apiversion > requiredFirmwareVersion){
		sprintf(initErrorMessage,
				"This detector software software version (0x%llx) is incompatible.\n"
				"Please update detector software (min. 0x%llx) to be compatible with this firmware.\n",
				(long long int)sw_fw_apiversion,
				(long long int)requiredFirmwareVersion);
		FILE_LOG(logERROR, (initErrorMessage));
		initError = FAIL;
		return;
	}

	//check for firmware compatibility - old firmware
	if( requiredFirmwareVersion > fwversion) {
		sprintf(initErrorMessage,
				"This firmware version (0x%llx) is incompatible.\n"
				"Please update firmware (min. 0x%llx) to be compatible with this server.\n",
				(long long int)fwversion,
				(long long int)requiredFirmwareVersion);
		FILE_LOG(logERROR, (initErrorMessage));
		initError = FAIL;
		return;
	}
	FILE_LOG(logINFO, ("Compatibility - success\n"));

#endif
}

int checkType() {
#ifdef VIRTUAL
    return OK;
#endif
	volatile u_int32_t type = ((bus_r(FPGA_VERSION_REG) & DETECTOR_TYPE_MSK) >> DETECTOR_TYPE_OFST);
	if (type != GOTTHARD2){
			FILE_LOG(logERROR, ("This is not a Gotthard2 Server (read %d, expected %d)\n", type, GOTTHARD2));
			return FAIL;
		}
	return OK;
}

int testFpga() {
#ifdef VIRTUAL
    return OK;
#endif
	FILE_LOG(logINFO, ("Testing FPGA:\n"));

	//fixed pattern
	int ret = OK;
	volatile u_int32_t val = bus_r(FIX_PATT_REG);
	if (val == FIX_PATT_VAL) {
		FILE_LOG(logINFO, ("Fixed pattern: successful match 0x%08x\n",val));
	} else {
		FILE_LOG(logERROR, ("Fixed pattern does not match! Read 0x%08x, expected 0x%08x\n", val, FIX_PATT_VAL));
		ret = FAIL;
	}
	return ret;
}

int testBus() {
#ifdef VIRTUAL
    return OK;
#endif
	FILE_LOG(logINFO, ("Testing Bus:\n"));

	int ret = OK;
	u_int32_t addr = DTA_OFFSET_REG; 
	int times = 1000 * 1000;
	int i = 0;

	for (i = 0; i < times; ++i) {
		bus_w(addr, i * 100);
		if (i * 100 != bus_r(addr)) {
			FILE_LOG(logERROR, ("Mismatch! Wrote 0x%x, read 0x%x\n",
					i * 100, bus_r(addr)));
			ret = FAIL;
		}
	}

	bus_w(addr, 0);

	if (ret == OK) {
		FILE_LOG(logINFO, ("Successfully tested bus %d times\n", times));
	}
	return ret;
}

/* Ids */

uint64_t getServerVersion() {
    return APIGOTTHARD2;
}

uint64_t getClientServerAPIVersion() {
    return APIGOTTHARD2;
}

u_int64_t getFirmwareVersion() {
#ifdef VIRTUAL
    return 0;
#endif
	return ((bus_r(FPGA_VERSION_REG) & FPGA_COMPILATION_DATE_MSK) >> FPGA_COMPILATION_DATE_OFST);
}

u_int64_t getFirmwareAPIVersion() {
#ifdef VIRTUAL
    return 0;
#endif
    return ((bus_r(API_VERSION_REG) & API_VERSION_MSK) >> API_VERSION_OFST);
}

u_int16_t getHardwareVersionNumber() {
#ifdef VIRTUAL
    return 0;
#endif
	return ((bus_r(MCB_SERIAL_NO_REG)));// & HARDWARE_VERSION_NUM_MSK) >> HARDWARE_VERSION_NUM_OFST);
}

u_int32_t getDetectorNumber(){
#ifdef VIRTUAL
    return 0;
#endif
	return bus_r(MCB_SERIAL_NO_REG);
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
	if (initError == OK) {
		setupDetector();
	}
	initCheckDone = 1;
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
    FILE_LOG(logINFO, ("This Server is for 1 Gotthard2 module \n")); 

	clkFrequency[READOUT_C0] = DEFAULT_READOUT_C0;
	clkFrequency[READOUT_C1] = DEFAULT_READOUT_C1;
	clkFrequency[SYSTEM_C0] = DEFAULT_SYSTEM_C0;
	clkFrequency[SYSTEM_C1] = DEFAULT_SYSTEM_C1;
	clkFrequency[SYSTEM_C2] = DEFAULT_SYSTEM_C2;
	clkFrequency[SYSTEM_C3] = DEFAULT_SYSTEM_C3;
	detPos[0] = 0;
	detPos[1] = 0;

	highvoltage = 0;
	injectedChannelsOffset = 0;
	injectedChannelsIncrement = 0;

	{
		int i, j;
		for (i = 0; i < NUM_CLOCKS; ++i) {
            clkPhase[i] = 0;
        }
		for (i = 0; i < NDAC; ++i) {
			dacValues[i] = 0;
			defaultDacValues[i] = 0;
		}
		for (i = 0; i < ONCHIP_NDAC; ++i) {
			for (j = 0; j < NCHIP; ++j) {
				onChipdacValues[i][j] = -1;
				defaultOnChipdacValues[i][j] = -1;
			}
		}
		for	(i = 0; i < NCHIP; ++i) {
			for (j = 0; j < NCHAN; ++j) {
				vetoReference[i][j] = 0;
			}
		}	
	}


#ifndef VIRTUAL
	// pll defines
	ALTERA_PLL_C10_SetDefines(REG_OFFSET, BASE_READOUT_PLL, BASE_SYSTEM_PLL, READOUT_PLL_RESET_REG, SYSTEM_PLL_RESET_REG, READOUT_PLL_RESET_MSK, SYSTEM_PLL_RESET_MSK, READOUT_PLL_WAIT_REG, SYSTEM_PLL_WAIT_REG, READOUT_PLL_WAIT_MSK, SYSTEM_PLL_WAIT_MSK, READOUT_PLL_VCO_FREQ_HZ, SYSTEM_PLL_VCO_FREQ_HZ);
	ALTERA_PLL_C10_ResetPLL(READOUT_PLL);
	ALTERA_PLL_C10_ResetPLL(SYSTEM_PLL);
	// hv
    DAC6571_SetDefines(HV_HARD_MAX_VOLTAGE, HV_DRIVER_FILE_NAME);
	// dacs
	LTC2620_D_SetDefines(DAC_MAX_MV, DAC_DRIVER_FILE_NAME, NDAC);
	// on chip dacs
	ASIC_Driver_SetDefines(ONCHIP_DAC_DRIVER_FILE_NAME);
#endif

	// Default values
    setHighVoltage(DEFAULT_HIGH_VOLTAGE);
	readConfigFile(); // also sets default dac and on chip dac values 
	
	// Initialization of acquistion parameters
	setNumFrames(DEFAULT_NUM_FRAMES);
	setNumTriggers(DEFAULT_NUM_CYCLES);
	setExpTime(DEFAULT_EXPTIME);
	setPeriod(DEFAULT_PERIOD);
}

int setDefaultDacs() {
	int ret = OK;
	FILE_LOG(logINFOBLUE, ("Setting Default Dac values\n"));
	{
		int idac = 0;
		for(idac = 0; idac < NDAC; ++idac) {
			setDAC((enum DACINDEX)idac, defaultDacValues[idac], 0);
		}
	}
	return ret;
}

int setDefaultOnChipDacs() {
	int ret = OK;
	FILE_LOG(logINFOBLUE, ("Setting Default On chip Dac values\n"));
	{
		int idac = 0, ichip = 0;
		for(idac = 0; idac < ONCHIP_NDAC; ++idac) {
			for(ichip = 0; ichip < NCHIP; ++ichip) {
				setOnChipDAC((enum ONCHIP_DACINDEX)idac, ichip, defaultOnChipdacValues[idac][ichip]);
			}
		}
	}	
	return ret;
}


int readConfigFile() {

	if (initError == FAIL) {
		return initError;
	}

    FILE* fd = fopen(CONFIG_FILE, "r");
    if(fd == NULL) {
		sprintf(initErrorMessage, "Could not open on-board detector server config file [%s].\n", CONFIG_FILE);
		initError = FAIL;		
		FILE_LOG(logERROR, ("%s\n\n", initErrorMessage));
        return FAIL;
    }

    FILE_LOG(logINFOBLUE, ("Reading config file %s\n", CONFIG_FILE));

    // Initialization
    const size_t LZ = 256;
    char line[LZ];
    memset(line, 0, LZ);
    char command[LZ];

    // keep reading a line
    while (fgets(line, LZ, fd)) {

		// ignore comments
        if (line[0] == '#') {
			FILE_LOG(logDEBUG1, ("Ignoring Comment\n"));
            continue;
		}

		// ignore empty lines
		if (strlen(line) <= 1) {
			FILE_LOG(logDEBUG1, ("Ignoring Empty line\n"));
			continue;
		}

		FILE_LOG(logDEBUG1, ("Command to process: (size:%d) %.*s\n", strlen(line), strlen(line) -1, line));
		memset(command, 0, LZ);

        // vchip command
        if (!strncmp(line, "vchip_", strlen("vchip_"))) {

			enum ONCHIP_DACINDEX idac = 0;
			int ichip = -1;
			int value = 0;

			// cannot scan values
			if (sscanf(line, "%s %d 0x%x", command, &ichip, &value) != 3) {
				sprintf(initErrorMessage, "Could not scan on-chip dac commands from on-board server config file. Line:[%s].\n", line);
				break;
			}

            if  (!strcmp(command,"vchip_comp_fe")) {
                idac = G2_VCHIP_COMP_FE;
            } else if (!strcasecmp(command,"vchip_opa_1st")) {
                idac = G2_VCHIP_OPA_1ST;
            } else if (!strcasecmp(command,"vchip_opa_fd")) {
                idac = G2_VCHIP_OPA_FD;
            } else if (!strcasecmp(command,"vchip_comp_adc")) {
                idac = G2_VCHIP_COMP_ADC;
            } else if (!strcasecmp(command,"vchip_ref_comp_fe")) {
                idac = G2_VCHIP_REF_COMP_FE;
            } else if (!strcasecmp(command,"vchip_cs")) {
                idac = G2_VCHIP_CS;
            } else {
				sprintf(initErrorMessage, "Unknown on-chip dac command in on-board server config file. Command:[%s].\n", command);
                break;
            }

			// set on chip dac
			if (setOnChipDAC(idac, ichip, value) == FAIL) {
				sprintf(initErrorMessage, "Set on-chip dac failed from on-board server config file. Command:[%s].\n", command);
                break;				
			}
        }

        // dac command
        else {

			enum DACINDEX idac = 0;
			int value = 0;

			// cannot scan values
			if (sscanf(line, "%s %d", command, &value) != 2) {
				sprintf(initErrorMessage, "Could not scan dac commands from on-board server config file. Line:[%s].\n", line);
				break;
			}

            if  (!strcmp(command,"vref_h_adc")) {
                idac = G2_VREF_H_ADC;
            } else if (!strcasecmp(command,"vb_comp_fe")) {
                idac = G2_VB_COMP_FE;
            } else if (!strcasecmp(command,"vb_comp_adc")) {
                idac = G2_VB_COMP_ADC;
            } else if (!strcasecmp(command,"vcom_cds")) {
                idac = G2_VCOM_CDS;
            } else if (!strcasecmp(command,"vref_restore")) {
                idac = G2_VREF_RESTORE;
            } else if (!strcasecmp(command,"vb_opa_1st")) {
                idac = G2_VB_OPA_1ST;
            } else if (!strcasecmp(command,"vref_comp_fe")) {
                idac = G2_VREF_COMP_FE;
            } else if (!strcasecmp(command,"vcom_adc1")) {
                idac = G2_VCOM_ADC1;
            } else if (!strcasecmp(command,"vref_prech")) {
                idac = G2_VREF_PRECH;
            } else if (!strcasecmp(command,"vref_l_adc")) {
                idac = G2_VREF_L_ADC;
            } else if (!strcasecmp(command,"vref_cds")) {
                idac = G2_VREF_CDS;
            } else if (!strcasecmp(command,"vb_cs")) {
                idac = G2_VB_CS;
            } else if (!strcasecmp(command,"vb_opa_fd")) {
                idac = G2_VB_OPA_FD;
            } else if (!strcasecmp(command,"vcom_adc2")) {
                idac = G2_VCOM_ADC2;
            } else {
				sprintf(initErrorMessage, "Unknown command in on-board server config file. Command:[%s].\n", command);
                break;
            }

			// set dac
			setDAC(idac, value, 0);
			int retval = getDAC(idac, 0);
			if (retval != value) {
				sprintf(initErrorMessage, "Set dac %s failed from on-board server config file. Set %d, got %d.\n", command, value, retval);
                break;				
			}
        }
		memset(line, 0, LZ);
    }
    fclose(fd);

	if (strlen(initErrorMessage)) {
		initError = FAIL;		
		FILE_LOG(logERROR, ("%s\n\n", initErrorMessage));
	} else {
		FILE_LOG(logINFOBLUE, ("Successfully read config file\n"));
	}
    return initError;
}


/* set parameters -  dr, roi */

int setDynamicRange(int dr){
	return DYNAMIC_RANGE;
}


/* parameters  */
void setNumFrames(int64_t val) {
    if (val > 0) {
        FILE_LOG(logINFO, ("Setting number of frames %lld\n", (long long int)val));
		set64BitReg(val, SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
    }
}

int64_t getNumFrames() {
    return get64BitReg(SET_FRAMES_LSB_REG, SET_FRAMES_MSB_REG);
}

void setNumTriggers(int64_t val) {
    if (val > 0) {
		FILE_LOG(logINFO, ("Setting number of triggers %lld\n", (long long int)val));
        set64BitReg(val, SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
    } 
}

int64_t getNumTriggers() {
    return get64BitReg(SET_CYCLES_LSB_REG, SET_CYCLES_MSB_REG);
}

int setExpTime(int64_t val) {
    if (val < 0) {
        FILE_LOG(logERROR, ("Invalid exptime: %lld ns\n", (long long int)val));
        return FAIL;
    }
	FILE_LOG(logINFO, ("Setting exptime %lld ns\n", (long long int)val));
    val *= (1E-9 * READOUT_C0);
    set64BitReg(val, SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG);

    // validate for tolerance
    int64_t retval = getExpTime();
    val /= (1E-9 * READOUT_C0);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getExpTime() {
    return get64BitReg(SET_EXPTIME_LSB_REG, SET_EXPTIME_MSB_REG) / (1E-9 * READOUT_C0);
}

int setPeriod(int64_t val) {
    if (val < 0) {
        FILE_LOG(logERROR, ("Invalid period: %lld ns\n", (long long int)val));
        return FAIL;
    }
	FILE_LOG(logINFO, ("Setting period %lld ns\n", (long long int)val));
    val *= (1E-9 * SYSTEM_C0);
    set64BitReg(val, SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG);

    // validate for tolerance
    int64_t retval = getPeriod();
    val /= (1E-9 * SYSTEM_C0);
    if (val != retval) {
        return FAIL;
    }
    return OK;
}

int64_t getPeriod() {
    return get64BitReg(SET_PERIOD_LSB_REG, SET_PERIOD_MSB_REG)/ (1E-9 * SYSTEM_C0);
}

int64_t getNumFramesLeft() {
    return get64BitReg(GET_FRAMES_LSB_REG, GET_FRAMES_MSB_REG);
}

int64_t getNumTriggersLeft() {
    return get64BitReg(GET_CYCLES_LSB_REG, GET_CYCLES_MSB_REG);
}



/* parameters - dac, hv */
int	setOnChipDAC(enum ONCHIP_DACINDEX ind, int chipIndex, int val) {
	char* names[] = {ONCHIP_DAC_NAMES};
	FILE_LOG(logDEBUG1, ("Setting on chip dac[%d - %s]: 0x%x\n", (int)ind, names[ind], val));

	if (ind >= ONCHIP_NDAC) {
		FILE_LOG(logERROR, ("Invalid dac index %d\n", (int)ind));
		return FAIL;
	}
	if (chipIndex >= NCHIP) {
		FILE_LOG(logERROR, ("Invalid chip index %d\n", chipIndex));
		return FAIL;		
	}
	if (val > ONCHIP_DAC_MAX_VAL) {
		FILE_LOG(logERROR, ("Invalid val %d\n", val));
		return FAIL;			
	}
	FILE_LOG(logINFO, ("Setting on chip dac[%d - %s]: 0x%x\n", (int)ind, names[ind], val));

	char buffer[2];
	memset(buffer, 0, sizeof(buffer));
	buffer[1] = ((val & 0xF) << 4) | (((int)ind) & 0xF); // LSB (4 bits) + ADDR (4 bits)
	buffer[0] = (val >> 4) & 0x3F; // MSB (6 bits)
		
	if (ASIC_Driver_Set(chipIndex, sizeof(buffer), buffer) == FAIL) {
		return FAIL;				
	}
	// all chips
	if (chipIndex == -1) {
		int ichip = 0;
		for (ichip = 0; ichip < NCHIP; ++ichip) {
			onChipdacValues[ind][ichip] = val;
		}
	} 
	
	// specific chip
	else {
		onChipdacValues[ind][chipIndex] = val;
	}
	return OK;
}

int	getOnChipDAC(enum ONCHIP_DACINDEX ind, int chipIndex) {
	// all chips
	if (chipIndex == -1) {
		int retval = onChipdacValues[ind][0];
		int ichip = 0;
		// check if same value for remaining chips
		for (ichip = 1; ichip < NCHIP; ++ichip) {
			if (onChipdacValues[ind][ichip] != retval) {
				return -1;
			}
		}
		return retval;
	}
	// specific chip
	return onChipdacValues[ind][chipIndex];
}

void setDAC(enum DACINDEX ind, int val, int mV) {
    if (val < 0) {
        return;
	}

	char* dac_names[] = {DAC_NAMES};
    FILE_LOG(logDEBUG1, ("Setting dac[%d - %s]: %d %s \n", (int)ind, dac_names[ind], val, (mV ? "mV" : "dac units")));
    int dacval = val;
#ifdef VIRTUAL
    FILE_LOG(logINFO, ("Setting dac[%d - %s]: %d %s \n", (int)ind, dac_names[ind], val, (mV ? "mV" : "dac units")));
    if (!mV) {
        dacValues[ind] = val;
    }
    // convert to dac units
    else if (LTC2620_D_VoltageToDac(val, &dacval) == OK) {
        dacValues[ind] = dacval;
    }
#else
    if (LTC2620_D_SetDACValue((int)ind, val, mV, dac_names[ind], &dacval) == OK) {
        dacValues[ind] = dacval;
    }
#endif
}

int getDAC(enum DACINDEX ind, int mV) {
    if (!mV) {
        FILE_LOG(logDEBUG1, ("Getting DAC %d : %d dac\n",ind, dacValues[ind]));
        return dacValues[ind];
    }
    int voltage = -1;
    LTC2620_D_DacToVoltage(dacValues[ind], &voltage);
    FILE_LOG(logDEBUG1, ("Getting DAC %d : %d dac (%d mV)\n",ind, dacValues[ind], voltage));
    return voltage;
}

int getMaxDacSteps() {
    return LTC2620_D_GetMaxNumSteps();
}

int setHighVoltage(int val){
	if (val > HV_SOFT_MAX_VOLTAGE) {
		val = HV_SOFT_MAX_VOLTAGE;
	}
	
#ifdef VIRTUAL
    if (val >= 0)
        highvoltage = val;
    return highvoltage;
#endif

	// setting hv
	if (val >= 0) {
	    FILE_LOG(logINFO, ("Setting High voltage: %d V\n", val));
	    DAC6571_Set(val);
	    highvoltage = val;
	}
	return highvoltage;
}


int configureMAC() {

	uint32_t srcip = udpDetails.srcip;
	uint32_t dstip = udpDetails.dstip;
	uint64_t srcmac = udpDetails.srcmac;
	uint64_t dstmac = udpDetails.dstmac;
	int srcport = udpDetails.srcport;
	int dstport = udpDetails.dstport;		

#ifdef VIRTUAL
	char cDestIp[MAX_STR_LENGTH];
	memset(cDestIp, 0, MAX_STR_LENGTH);
	sprintf(cDestIp, "%d.%d.%d.%d", (dstip>>24)&0xff,(dstip>>16)&0xff,(dstip>>8)&0xff,(dstip)&0xff);
	FILE_LOG(logINFO, ("1G UDP: Destination (IP: %s, port:%d)\n", cDestIp, dstport));
	if (setUDPDestinationDetails(0, cDestIp, dstport) == FAIL) {
		FILE_LOG(logERROR, ("could not set udp destination IP and port\n"));
		return FAIL;
	}
    return OK;
#endif
	FILE_LOG(logINFOBLUE, ("Configuring MAC\n"));
	
	FILE_LOG(logINFO, ("\tSource IP   : %d.%d.%d.%d \t\t(0x%08x)\n",
	        (srcip>>24)&0xff,(srcip>>16)&0xff,(srcip>>8)&0xff,(srcip)&0xff, srcip));
	FILE_LOG(logINFO, ("\tSource MAC  : %02x:%02x:%02x:%02x:%02x:%02x \t(0x%010llx)\n",
			(unsigned int)((srcmac>>40)&0xFF),
			(unsigned int)((srcmac>>32)&0xFF),
			(unsigned int)((srcmac>>24)&0xFF),
			(unsigned int)((srcmac>>16)&0xFF),
			(unsigned int)((srcmac>>8)&0xFF),
			(unsigned int)((srcmac>>0)&0xFF),
			(long  long unsigned int)srcmac));
	FILE_LOG(logINFO, ("\tSource Port : %d \t\t\t(0x%08x)\n", srcport, srcport));

	FILE_LOG(logINFO, ("\tDest. IP    : %d.%d.%d.%d \t\t(0x%08x)\n",
	        (dstip>>24)&0xff,(dstip>>16)&0xff,(dstip>>8)&0xff,(dstip)&0xff, dstip));
	FILE_LOG(logINFO, ("\tDest. MAC   : %02x:%02x:%02x:%02x:%02x:%02x \t(0x%010llx)\n",
			(unsigned int)((dstmac>>40)&0xFF),
			(unsigned int)((dstmac>>32)&0xFF),
			(unsigned int)((dstmac>>24)&0xFF),
			(unsigned int)((dstmac>>16)&0xFF),
			(unsigned int)((dstmac>>8)&0xFF),
			(unsigned int)((dstmac>>0)&0xFF),
			(long  long unsigned int)dstmac));
	FILE_LOG(logINFO, ("\tDest. Port  : %d \t\t\t(0x%08x)\n\n",dstport, dstport));

	// start addr
	uint32_t addr = BASE_UDP_RAM;
	// calculate rxr endpoint offset
	//addr += (iRxEntry * RXR_ENDPOINT_OFST);//TODO: is there round robin already implemented?
	// get struct memory
	udp_header *udp = (udp_header*) (Nios_getBaseAddress() + addr/(sizeof(u_int32_t)));
	memset(udp, 0, sizeof(udp_header));

	//  mac addresses	
	// msb (32) + lsb (16)
	udp->udp_destmac_msb	= ((dstmac >> 16) & BIT32_MASK);
	udp->udp_destmac_lsb	= ((dstmac >> 0) & BIT16_MASK);
	// msb (16) + lsb (32)
	udp->udp_srcmac_msb		= ((srcmac >> 32) & BIT16_MASK);
	udp->udp_srcmac_lsb		= ((srcmac >> 0) & BIT32_MASK);

	// ip addresses
	udp->ip_srcip_msb		= ((srcip >> 16) & BIT16_MASK);
	udp->ip_srcip_lsb		= ((srcip >> 0) & BIT16_MASK);	
	udp->ip_destip_msb		= ((dstip >> 16) & BIT16_MASK);
	udp->ip_destip_lsb		= ((dstip >> 0) & BIT16_MASK);	

	// source port
	udp->udp_srcport 		= srcport;
	udp->udp_destport		= dstport;

	// other defines
	udp->udp_ethertype		= 0x800;
	udp->ip_ver				= 0x4;
	udp->ip_ihl				= 0x5;
	udp->ip_flags			= 0x2; //FIXME
	udp->ip_ttl           	= 0x40;
	udp->ip_protocol      	= 0x11;
	// total length is redefined in firmware

	calcChecksum(udp);

	//TODO?
	//cleanFifos();
	//resetCore();

	return OK;
}

void calcChecksum(udp_header* udp) {
	int count = IP_HEADER_SIZE;
	long int sum = 0;
	
	// start at ip_tos as the memory is not continous for ip header
	uint16_t *addr = (uint16_t*) (&(udp->ip_tos)); 

	sum += *addr++;
	count -= 2;

	// ignore ethertype (from udp header)
	addr++;

	// from identification to srcip_lsb
    while( count > 2 )  {
		sum += *addr++;
		count -= 2;
	}

	// ignore src udp port (from udp header)
	addr++;
	
	if (count > 0)
	    sum += *addr;                     // Add left-over byte, if any
	while (sum >> 16)
	    sum = (sum & 0xffff) + (sum >> 16);// Fold 32-bit sum to 16 bits
	long int checksum = sum & 0xffff;
	checksum += UDP_IP_HEADER_LENGTH_BYTES;
	FILE_LOG(logINFO, ("\tIP checksum is 0x%lx\n",checksum));
	udp->ip_checksum = checksum;
}

// Detector Specific

int setPhase(enum CLKINDEX ind, int val, int degrees) {
   if (ind < 0 || ind >= NUM_CLOCKS) {
		FILE_LOG(logERROR, ("Unknown clock index %d to set phase\n", ind));
	    return FAIL;
	}
	char* clock_names[] = {CLK_NAMES};
    FILE_LOG(logINFO, ("Setting %s clock (%d) phase to %d %s\n", clock_names[ind], ind, val, degrees == 0 ? "" : "degrees"));
	int maxShift = getMaxPhase(ind);
	// validation
	if (degrees && (val < 0 || val > 359)) {
		 FILE_LOG(logERROR, ("\tPhase outside limits (0 - 359°C)\n"));
		 return FAIL;
	}
	if (!degrees && (val < 0 || val > maxShift - 1)) {
		 FILE_LOG(logERROR, ("\tPhase outside limits (0 - %d phase shifts)\n", maxShift - 1));
		 return FAIL;
	}

	int valShift = val;
	// convert to phase shift
	if (degrees) {
		ConvertToDifferentRange(0, 359, 0, maxShift - 1, val, &valShift);
	}
	FILE_LOG(logDEBUG1, ("\tphase shift: %d (degrees/shift: %d)\n", valShift, val));

	int relativePhase = valShift - clkPhase[ind];
	FILE_LOG(logDEBUG1, ("\trelative phase shift: %d (Current phase: %d)\n", relativePhase, clkPhase[ind]));

    // same phase
    if (!relativePhase) {
    	FILE_LOG(logINFO, ("\tNothing to do in Phase Shift\n"));
    	return OK;
    }
    FILE_LOG(logINFOBLUE, ("Configuring Phase\n"));

    int phase = 0;
    if (relativePhase > 0) {
        phase = (maxShift - relativePhase);
    } else {
    	phase = (-1) * relativePhase;
    }
    FILE_LOG(logDEBUG1, ("\t[Single Direction] Phase:%d (0x%x). Max Phase shifts:%d\n", phase, phase, maxShift));

	int pllIndex = ind >= SYSTEM_C0 ? SYSTEM_PLL : READOUT_PLL;
	int clkIndex = ind >= SYSTEM_C0 ? ind - SYSTEM_C0 : ind;
    int ret = ALTERA_PLL_C10_SetPhaseShift(pllIndex, clkIndex, phase, 0);

    clkPhase[ind] = valShift;
	return ret;
}

int getPhase(enum CLKINDEX ind, int degrees) {
   if (ind < 0 || ind >= NUM_CLOCKS) {
		FILE_LOG(logERROR, ("Unknown clock index %d to get phase\n", ind));
	    return -1;
	}
	if (!degrees)
		return clkPhase[ind];
	// convert back to degrees
	int val = 0;
	ConvertToDifferentRange(0, getMaxPhase(ind) - 1, 0, 359, clkPhase[ind], &val);
	return val;
}

int getMaxPhase(enum CLKINDEX ind) {
   if (ind < 0 || ind >= NUM_CLOCKS) {
		FILE_LOG(logERROR, ("Unknown clock index %d to get max phase\n", ind));
	    return -1;
	}
	int vcofreq = getVCOFrequency(ind);
	int maxshiftstep = ALTERA_PLL_C10_GetMaxPhaseShiftStepsofVCO();
	int ret = ((double)vcofreq / (double)clkFrequency[ind]) * maxshiftstep;

	char* clock_names[] = {CLK_NAMES};
	FILE_LOG(logDEBUG1, ("\tMax Phase Shift (%s): %d (Clock: %d Hz, VCO:%d Hz)\n",
			clock_names[ind], ret, clkFrequency[ind], vcofreq));

	return ret;
}

int validatePhaseinDegrees(enum CLKINDEX ind, int val, int retval) {
   if (ind < 0 || ind >= NUM_CLOCKS) {
		FILE_LOG(logERROR, ("Unknown clock index %d to validate phase in degrees\n", ind));
	    return FAIL;
	}
	if (val == -1) {
		return OK;
	}
	FILE_LOG(logDEBUG1, ("validating phase in degrees for clk %d\n", (int)ind));
	int maxShift = getMaxPhase(ind);
	// convert degrees to shift
	int valShift = 0;
	ConvertToDifferentRange(0, 359, 0, maxShift - 1, val, &valShift);
	// convert back to degrees
	ConvertToDifferentRange(0, maxShift - 1, 0, 359, valShift, &val);

	if (val == retval)
		return OK;
	return FAIL;
}



int getFrequency(enum CLKINDEX ind) {
   if (ind < 0 || ind >= NUM_CLOCKS) {
		FILE_LOG(logERROR, ("Unknown clock index %d to get frequency\n", ind));
	    return -1;
	}
    return clkFrequency[ind];
}

int getVCOFrequency(enum CLKINDEX ind) {
   if (ind < 0 || ind >= NUM_CLOCKS) {
		FILE_LOG(logERROR, ("Unknown clock index %d to get vco frequency\n", ind));
	    return -1;
	}
	int pllIndex = ind >= SYSTEM_C0 ? SYSTEM_PLL : READOUT_PLL;
	return ALTERA_PLL_C10_GetVCOFrequency(pllIndex);
}

int getMaxClockDivider() {
	return ALTERA_PLL_C10_GetMaxClockDivider();
}

int setClockDivider(enum CLKINDEX ind, int val) {
   if (ind < 0 || ind >= NUM_CLOCKS) {
		FILE_LOG(logERROR, ("Unknown clock index %d to set clock divider\n", ind));
	    return FAIL;
	}
	if (val < 2 || val > getMaxClockDivider()) {
		return FAIL;
	}
	char* clock_names[] = {CLK_NAMES};
	int vcofreq = getVCOFrequency(ind);
	int currentdiv = vcofreq / clkFrequency[ind];
	int newfreq = vcofreq / val;

    FILE_LOG(logINFO, ("\tSetting %s clock (%d) divider from %d (%d Hz) to %d (%d Hz). \n\t(Vcofreq: %d Hz)\n", clock_names[ind], ind, currentdiv, clkFrequency[ind], val, newfreq, vcofreq));

    // Remembering old phases in degrees
    int oldPhases[NUM_CLOCKS];
	{ 
		int i = 0;
		for (i = 0; i < NUM_CLOCKS; ++i) {
			oldPhases	[i] = getPhase(i, 1);
			FILE_LOG(logDEBUG1, ("\tRemembering %s clock (%d) phase: %d degrees\n", clock_names[ind], ind, oldPhases[i]));
		}
	}

    // Calculate and set output frequency
	int pllIndex = ind >= SYSTEM_C0 ? SYSTEM_PLL : READOUT_PLL;
	int clkIndex = ind >= SYSTEM_C0 ? ind - SYSTEM_C0 : ind;
    int ret = ALTERA_PLL_C10_SetOuputFrequency (pllIndex, clkIndex, newfreq);
	clkFrequency[ind] = newfreq;
    FILE_LOG(logINFO, ("\t%s clock (%d) divider set to %d (%d Hz)\n", clock_names[ind], ind, val, clkFrequency[ind]));
   
    // phase is reset by pll (when setting output frequency)
	if (ind >= READOUT_C0) {
    	clkPhase[READOUT_C0] = 0;
    	clkPhase[READOUT_C1] = 0;		
	} else {
    	clkPhase[SYSTEM_C0] = 0;
    	clkPhase[SYSTEM_C1] = 0;
    	clkPhase[SYSTEM_C2] = 0;
    	clkPhase[SYSTEM_C3] = 0;
	}

    // set the phase in degreesif custom set
	{ 
		int i = 0;
		for (i = 0; i < NUM_CLOCKS; ++i) {
			FILE_LOG(logINFO, ("\tPhase reset by PLL\n\tCorrecting %s clock (%d) phase to %d degrees\n", clock_names[i], i, oldPhases[i]));
			setPhase(i, oldPhases[i], 1);
		}
	}
	return ret;
}

int getClockDivider(enum CLKINDEX ind) {
   if (ind < 0 || ind >= NUM_CLOCKS) {
		FILE_LOG(logERROR, ("Unknown clock index %d to get clock divider\n", ind));
	    return -1;
	}
	return (getVCOFrequency(ind) / clkFrequency[ind]);
}

int setInjectChannel(int offset, int increment) {
	if (offset < 0 || increment  < 1) {
		FILE_LOG(logERROR, ("Cannot inject channel. Invalid offset %d or increment %d\n", offset, increment));
		return FAIL;
	}

	FILE_LOG(logINFO, ("Injecting channels [offset:%d, increment:%d]\n", offset, increment));
	
	// 4 bits of padding + 128 bits + 4 bits for address = 136 bits
	char buffer[17]; 
	memset(buffer, 0, sizeof(buffer));
	int startCh = 4; // 4 due to padding
	int ich = 0; 
	for (ich = startCh + offset; ich < startCh + NCHAN; ich = ich + increment) {
		int byteIndex = ich / 8;
		int bitIndex = ich % 8;
		buffer[byteIndex] |= (1 << (8 - 1 - bitIndex));
	}

	// address at the end
	buffer[16] |= (ASIC_CURRENT_INJECT_ADDR);

	int chipIndex = -1; // for all chips
	if (ASIC_Driver_Set(chipIndex, sizeof(buffer), buffer) == FAIL) {
		return FAIL;				
	}

	injectedChannelsOffset = offset;
	injectedChannelsIncrement = increment;
	return OK;
}

void getInjectedChannels(int* offset, int* increment) {
	*offset = injectedChannelsOffset;
	*increment = injectedChannelsIncrement;
}

int	setVetoPhoton(int chipIndex, int gainIndex, int* values) {
	FILE_LOG(logINFO, ("Setting veto photon [chip:%d, G%d]\n", chipIndex, gainIndex));

	// add gain bits
	{
		int gainValue = 0;
		switch (gainIndex) {
			case 0:
				gainValue = ASIC_G0_VAL;
				break;
			case 1:
				gainValue = ASIC_G1_VAL;
				break;
			case 2:
				gainValue = ASIC_G2_VAL;
				break;	
			default:
				FILE_LOG(logERROR, ("Unknown gain index %d\n", gainIndex));
				return FAIL;			
		}
		FILE_LOG(logDEBUG1, ("Adding gain bits\n"));
		int i = 0;
		for (i = 0; i < NCHAN; ++i) {
			values[i] |= gainValue;
			FILE_LOG(logDEBUG1, ("Value %d: 0x%x\n", i, values[i]));
		}
	}
	// create command
	const int lenAduBits = ASIC_GAIN_MAX_BITS + ADU_MAX_BITS;
	const int lenBits = lenAduBits * NCHAN;
	const int len = lenBits / 8;
	char buffer[len];
	memset(buffer, 0, len);
	int iBit = 4; // 4 due to padding
	int ich = 0; 
	for (ich = 0; ich < NCHAN; ++ich) {
		// copy 14 bits for each channel
		int totalToCopy = lenAduBits;
		while (totalToCopy > 0) {
			int byteIndex = iBit / 8;
			int bitIndex = iBit % 8;
			// how much to copy in a byte
			int toCopy = 8 - bitIndex;
			if (toCopy > totalToCopy) {
				toCopy = totalToCopy;
			}
			int copyMask = (1 << toCopy) - 1;
			// value pushed out by whats left and masked 
			int val = (values[ich] >> (totalToCopy - toCopy)) & copyMask;
			if (toCopy + bitIndex != 8) {
				val = val << (8 - bitIndex - toCopy);
			}
			buffer[byteIndex] |= val;
			// incrememnt indices
			iBit += toCopy;
			totalToCopy -= toCopy;
		}
	}

	// address at the end
	buffer[16] |= (ASIC_VETO_REF_ADDR);

	if (ASIC_Driver_Set(chipIndex, sizeof(buffer), buffer) == FAIL) {
		return FAIL;				
	}

	// all chips
	if (chipIndex == -1) {
		int ichip = 0;
		int ichan = 0;
		for (ichan = 0; ichan < NCHAN; ++ichan) {
			for (ichip = 0; ichip < NCHIP; ++ichip) {
				vetoReference[ichip][ichan] = values[ichan];
			}
		}
	} 
	
	// specific chip
	else {
		int ichan = 0;
		for (ichan = 0; ichan < NCHAN; ++ichan) {
			vetoReference[chipIndex][chipIndex] = values[ichan];;
		}
	}
	return OK;
} 

int getVetoPhoton(int chipIndex, int* retvals) {
	if (chipIndex == -1) {
		int i = 0, j = 0;
		for	(i = 0; i < NCHAN; ++i) {
			int val = vetoReference[0][i];
			for (j = 1; j < NCHIP; ++j) {
				if (vetoReference[j][i] != val) {
					FILE_LOG(logERROR, ("Get vet photon fail for chipIndex:%d. Different values between [nchip:%d, nchan:%d, value:%d] and [nchip:0, nchan:%d, value:%d]\n", chipIndex, j, i, vetoReference[j][i], i, val));
					return FAIL;
				}
			}
		}
		chipIndex = 0;
	}
	memcpy((char*)retvals, ((char*)vetoReference) + NCHAN * chipIndex * sizeof(int), sizeof(int) * NCHAN);
	return OK;
}



/* aquisition */

int setDetectorPosition(int pos[]) {
    memcpy(detPos, pos, sizeof(detPos));
	return OK;
}

int* getDetectorPosition() {
    return detPos;
}

int startStateMachine(){
#ifdef VIRTUAL
	// create udp socket
	if(createUDPSocket(0) != OK) {
		return FAIL;
	}
	FILE_LOG(logINFOBLUE, ("starting state machine\n"));
	// set status to running
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
	int64_t periodns = getPeriod();
	int numFrames = (getNumFrames() *
						getNumTriggers() );
	int64_t exp_ns = 	getExpTime();


    int frameNr = 0;
	// loop over number of frames
    for(frameNr=0; frameNr!= numFrames; ++frameNr ) {

		//check if virtual_stop is high
		if(virtual_stop == 1){
			break;
		}

		// sleep for exposure time
        struct timespec begin, end;
        clock_gettime(CLOCK_REALTIME, &begin);
        usleep(exp_ns / 1000);
        clock_gettime(CLOCK_REALTIME, &end);

		// calculate time left in period
        int64_t time_ns = ((end.tv_sec - begin.tv_sec) * 1E9 +
                (end.tv_nsec - begin.tv_nsec));

		// sleep for (period - exptime)
		if (frameNr < numFrames) { // if there is a next frame
			if (periodns > time_ns) {
				usleep((periodns - time_ns)/ 1000);
			}
		}

		// set register frames left
    }

	// set status to idle
	virtual_status = 0;
	FILE_LOG(logINFOBLUE, ("Finished Acquiring\n"));
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
	// wait for status to be done
	while(runBusy()){
		usleep(500);
	}
#ifdef VIRTUAL
	FILE_LOG(logINFOGREEN, ("acquisition successfully finished\n"));
	return;
#endif
}

u_int32_t runBusy() {
#ifdef VIRTUAL
    return virtual_status;
#endif
#ifdef VIRTUAL
	u_int32_t s = (bus_r(STATUS_REG) & RUN_BUSY_MSK);
	FILE_LOG(logDEBUG1, ("Status Register: %08x\n", s));
	return s;
#endif
	return -1;
}



/* common */

int calculateDataBytes(){
	return getTotalNumberOfChannels() * DYNAMIC_RANGE;
}

int getTotalNumberOfChannels(){return  ((int)getNumberOfChannelsPerChip() * (int)getNumberOfChips());}
int getNumberOfChips(){return  NCHIP;}
int getNumberOfDACs(){return  NDAC;}
int getNumberOfChannelsPerChip(){return  NCHAN;}