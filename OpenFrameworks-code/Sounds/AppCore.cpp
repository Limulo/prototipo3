/*
 *  AppCore.h
 *
 *  Copyright (c) Dan Wilcox 2011-2013
 *  BSD Simplified License.
 *
 *	https://github.com/danomatika/ofxPd
 *  
 *	ofxPd is an Open Frameworks addon for running an instance of the Pure Data 
 *	audio environment within an OpenFrameworks application.
 */

#include "AppCore.h"
#include <Poco/Path.h>

// da qui imposto i volumi dei diversi suoni (si tratta di un mixer!)

#define KICK_VOL		0.65
#define SNARE_VOL		0.04
#define HIHAT_VOL		0.20
#define DRUM_VOL		0.4
#define SNARE_DLY_VOL	0.3
#define PADS_VOL		0.35
#define BASS_VOL		0.95

using namespace std;

void AppCore::setup(const int numOutChannels, const int numInChannels, const int sampleRate, const int ticksPerBuffer) 
{	
	if(!pd.init(numOutChannels, numInChannels, sampleRate, ticksPerBuffer)) 
	{
		ofLog(OF_LOG_ERROR, "Could not init pd");
		OF_EXIT_APP(1);
	}
	
	// receive source names
	pd.addSource("loadbang");
	pd.addSource("kick");
	pd.addSource("snare");
	pd.addSource("hihat");
	pd.addSource("kick_vol");
	pd.addSource("snare_vol");
	pd.addSource("hihat_vol");
	pd.addSource("croma_time");
	pd.addSource("bass");
	pd.addSource("pads");
	pd.addSource("bass_vol");
	pd.addSource("pads_vol");
	pd.addSource("chords");
	pd.addSource("colon_bang");
	pd.addSource("snare_dly_vol");
	pd.addSource("drum_vol");
	pd.addSource("bass_lvl_to_OF"); // originariamente pensato per fare pulsare il colore rosso a tempo di musica
	
	
	// add listener (per il send dallo sketch di PureData
	pd.addListener(*this);
	pd.subscribe(*this, "bass_lvl_to_OF");	// Ascolta da PD
		
	// add the data/pd folder to the search path
	pd.addToSearchPath("little_synth");
	
	// audio processing on
	pd.dspOn();
	
	// open patch
	Patch patch = pd.openPatch("little_synth/synth.pd");
	
	pd.sendBang("loadbang");
	pd.sendFloat("kick_vol", KICK_VOL);
	pd.sendFloat("snare_vol", SNARE_VOL );
	pd.sendFloat("hihat_vol", HIHAT_VOL);
	pd.sendFloat("pads_vol", PADS_VOL);
	pd.sendFloat("bass_vol", BASS_VOL );
	pd.sendFloat("snare_dly_vol", SNARE_DLY_VOL);
	pd.sendFloat("drum_vol", DRUM_VOL);

}
	
void AppCore::update() {}

void AppCore::exit() 
{
	cout << "Ending PD\n";
	pd.closePatch("little_synth/synth.pd");
}

void AppCore::bang(char *receive)
{
	pd.sendBang(receive);
}

void AppCore::send_float(char * receive, float val)
{
	pd.sendFloat(receive, val);
}

void AppCore::audioReceived(float * input, int bufferSize, int nChannels) 
{
	pd.audioIn(input, bufferSize, nChannels);
}

void AppCore::audioRequested(float * output, int bufferSize, int nChannels) 
{
	pd.audioOut(output, bufferSize, nChannels);
}

void AppCore::printReceived(const std::string& message) 
{
	cout << message << endl;
}
void AppCore::floatReceived(const std::string& dest, float value)
{
	if (dest=="bass_lvl_to_OF")
	{
		// questo è il valore che la patch di PureData restituisce
		// si tratta del volume del basso
		bass_lvl = value; // * 0.0175; 

	}
		
}


