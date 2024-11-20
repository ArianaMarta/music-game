#include <Bela.h>
#include <cmath>
#include "I2C_MPR121.h"
#include <libraries/libpd/libpd.h>


// How many pins there are
#define NUM_TOUCH_PINS 12

// Define this to print data to terminal
#undef DEBUG_MPR121

// Change this to change how often the MPR121 is read (in Hz)
int readInterval = 50;

// Change this threshold to set the minimum amount of touch
int threshold = 40;

// This array holds the continuous sensor values
int sensorValue[NUM_TOUCH_PINS];


// ---- internal stuff -- do not change -----

I2C_MPR121 mpr121;			// Object to handle MPR121 sensing
AuxiliaryTask i2cTask;		// Auxiliary task to read I2C

int readCount = 0;			// How long until we read again...
int readIntervalSamples = 0; // How many samples between reads

void readMPR121(void*);


bool setup(BelaContext *context, void *userData)
{
	if(!mpr121.begin(1, 0x5A)) {
		rt_printf("Error initialising MPR121\n");
		return false;
	}

	i2cTask = Bela_createAuxiliaryTask(readMPR121, 50, "bela-mpr121");
	readIntervalSamples = context->audioSampleRate / readInterval;
	
	
	//initialize libpd. This clears the search path
	libpd_init();
	//libpd_start_message(NUM_TOUCH_PINS); //get data from all 8 MPR121 pins
	//libpd_add_float(1.0f);
	//libpd_finish_message("pd", "dsp");
	
	return true;
}

void render(BelaContext *context, void *userData)
{	
	libpd_start_message(12);
	for(int n = 0; n < 12; n++){
		libpd_add_float(sensorValue[n]);
	}
	libpd_finish_list("sensorValue");
	libpd_finish_message("pd", "dsp");
	

}

void cleanup(BelaContext *context, void *userData)
{ }

// Auxiliary task to read the I2C board
void readMPR121(void*)
{
	for(int i = 0; i < NUM_TOUCH_PINS; i++) {
		sensorValue[i] = -(mpr121.filteredData(i) - mpr121.baselineData(i));
		sensorValue[i] -= threshold;
		if(sensorValue[i] < 0)
			sensorValue[i] = 0;
#ifdef DEBUG_MPR121
		rt_printf("%d ", sensorValue[i]);
#endif
	}
#ifdef DEBUG_MPR121
	rt_printf("\n");
#endif

	// You can use this to read binary on/off touch state more easily
	//rt_printf("Touched: %x\n", mpr121.touched());
}