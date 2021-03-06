/*
 *  testApp.h
 *  Prototipo3
 *
 *  Created by Limulo.
 *  http://www.limulo.net 
 *  con contributi esterni:
 *
 *	1) ofxPd
 *  Copyright (c) Dan Wilcox 2011-2013
 *  BSD Simplified License.
 *	https://github.com/danomatika/ofxPd
 *  
 *	2) ofxTuio
 *	permette di creare e gestire direttamente nell'ambiente 
 *	di sviluppo di openFrameworks un server e un client che 
 *	comunicano tramite protocollo TUIO
 *	https://github.com/patriciogonzalezvivo/ofxTuio
 *
 */


#pragma once

#include <list>
#include <cassert>
#include <cmath>

#include "Table.h"
#include "Stripes.h"
#include "Digit.h"

#include "ofxTuio.h"
#include "AppCore.h"

#include "Fid_Rot.h"
#include "Fid_Sqr.h"
#include "Fid_Round.h"
#include "Fid_Bass.h"
#include "Fid_Synth.h"
#include "Fid_Chords.h"
#include "Finger.h"

// fiducial IDs
#define KICK	0		
#define SNARE	13
#define HIHAT	49
#define BASS	15
#define PADS	26
#define CHORD_0 39
#define CHORD_1 47
#define CHORD_2 60
#define CHORD_3 63
#define CHORD_4 79
#define CHORD_5 84
#define BPM		67

#define NUM_STRIPES 8	//	numero di colonne per battuta (numero di crome)
#define NCOLUMNS 32		//	vedi Stripes.h

#define PI		3.14159265358979323846


using namespace std;

class testApp : public ofBaseApp
{
public:
	void setup();
	void update();
	void draw();
	void exit();
	void keyPressed  (int key);
	
	// funzioni di callback per LibPD
	void audioReceived(float * input, int bufferSize, int nChannels);
	void audioRequested(float * output, int bufferSize, int nChannels);
	
	// funzioni di callback per TUIO
	// oggetti
	void objectAdded(ofxTuioObject & tuioObject);
	void objectRemoved(ofxTuioObject & tuioObject);
	void objectUpdated(ofxTuioObject & tuioObject);
	// finger
	void cursorAdded(ofxTuioCursor & tuioCursor);
	void cursorRemoved(ofxTuioCursor & tuioCursor);
	void cursorUpdated(ofxTuioCursor & tuioCursor);
	
	
	// funzioni e variabili per la gestione della riproduzione audio
	void play(int colonna);
	void time_control();
			
	long unsigned int	initial_time;
	long unsigned int	actual_time;
	long unsigned int	time;
	static unsigned int bpm;
	static int			croma_time;
	static float		chord_number;
	int					n_crome;
	int					last_croma_time;
	
	// variabili di utilità per la resa grafica e la gestione della playhead
	Stripe striscie[NCOLUMNS];
	int playHeadPosition;	// posizione della playHead
	float stripe_w;
	
	// variabili per la gestione del BPM e della resa grafica dello stesso
	float bpm_fid;		
	float memoria_bpm_angle;
	
		
	// variabili e utilità per correggere la distorsione introdotta dal proiettore 
	ofTexture	text;
	ofMesh		mesh;
	ofFbo		fbo;
	ofRectangle	quadro;
	int		wQuadro, hQuadro;
	int		margine; // valore in pixel del margine che separa il perimetro del quadro dal perimetro dell'FBO
	int		wFbo, hFbo;
	int		wMesh, hMesh;
	int		wWindow, hWindow;
	bool	bSetup;
	int		meshRotation;
	float	meshScaleX;
	float	meshScaleY;
	float	meshBottomOffset;
	float	meshCenterX;
	
	void	meshReset();


	// variabili e strutture dati per la manipolazione dei fiducials
	Fid_Rot *rotativo;
	Fid_Sqr *quadrato;
	Fid_Round *rotondo;
	Fid_Bass *basso;
	Fid_Synth *pads;
	Fid_Chords * accordo;
	Finger *dito;
	list<Fid_Rot*> rot_list;
	list<Fid_Sqr*> sqr_list;
	list<Fid_Round*> rnd_list;
	list<Fid_Bass*> bass_list;
	list<Fid_Synth*> sint_list;
	list<Finger*> dito_list;
	list<Fid_Chords*> chords_list;
	ofVec2f		centro;	// centro del tavolo
	ofVec2f		pos;	// posizione (x, y) del fiducial nella resa grafica a display
	float		angolo;
	float		rot_vel;
	
	// utility varie
	bool bDebug; 
	void backgroundGradient(const ofColor& start, const ofColor& end, int w_, int h_); // funzione per disegnare lo sfondo

	
	Digit			digit;
	AppCore			core;
	myTuioClient	tuio;
	Table			matrice;
	
};

