#ifndef SERVER_DEFS_H
#define SERVER_DEFS_H

#include "sls_detector_defs.h"

#include <stdint.h> 


// Hardware definitions

#define NCHAN 128
#define NCHIP 10
#define NMAXMODX  1
#define NMAXMODY 1
#define NMAXMOD NMAXMODX*NMAXMODY
#define NDAC 8
#define NADC 5

#define NCHANS NCHAN*NCHIP*NMAXMOD
#define NDACS NDAC*NMAXMOD

#define NTRIMBITS 6
#define NCOUNTBITS 24

//#define TRIM_DR ((2**NTRIMBITS)-1)
//#define COUNT_DR ((2**NCOUNTBITS)-1) 
#define TRIM_DR (((int)pow(2,NTRIMBITS))-1)
#define COUNT_DR (((int)pow(2,NCOUNTBITS))-1)


#define ALLMOD 0xffff
#define ALLFIFO 0xffff



#define ADCSYNC_VAL   			0x32214
#define TOKEN_RESTART_DELAY		0x88000000
#define TOKEN_RESTART_DELAY_ROI         0x1b000000
#define TOKEN_TIMING_REV1               0x1f16
#define TOKEN_TIMING_REV2               0x1f0f

#ifdef VIRTUAL
#define DEBUGOUT
#endif

#define CLK_FREQ 32.1E+6


#define THIS_SOFTWARE_VERSION 0x20121210
#endif
