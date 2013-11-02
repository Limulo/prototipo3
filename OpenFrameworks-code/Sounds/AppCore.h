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


#pragma once

#include "ofMain.h"
#include "ofxPd.h"

class AppCore : public ofxPdListener 
{
public:

	// funzioni generiche
	void setup(const int numOutChannels, const int numInChannels, const int sampleRate, const int ticksPerBuffer);
	void update();
	void exit();
	
	void bang(char *receive);
	void send_float(char * receive, float val);

		
	// funzioni audio di callback
	void audioReceived(float * input, int bufferSize, int nChannels);
	void audioRequested(float * output, int bufferSize, int nChannels);
		
	// funzioni di callback di PD
	void printReceived(const std::string& message);
	void floatReceived(const std::string& dest, float value);
	
	static float bass_lvl;
	ofxPd pd;
};
