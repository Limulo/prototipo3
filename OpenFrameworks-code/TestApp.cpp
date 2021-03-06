/*
 *  testApp.cpp
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



#include "testApp.h"
#include <Poco/Path.h>
using namespace std;

// colori generici
ofColor black	(0,		0,		0);
ofColor grey	(200,	200,	200);
ofColor rosso	(255,	0,		0);
ofColor green	(0,		255,	0);
ofColor blue	(0,		0,		255);
ofColor white	(255,	255,	255);

// colori estrapolati da una palette creata ad-hoc
ofColor c1		(123,	189,	224);
ofColor c2		(117,	196,	141);
ofColor c3		(132,	60,		42);
ofColor c4		(74,	155,	85);
ofColor c5		(163,	155,	85);
ofColor c6		(50,	105,	209);
ofColor c7		(59,	144,	165);
ofColor c8		(204,	150,	2);
ofColor c9		(37,	96,		54);

// assegno, a ciascun componente grafico e colori della palette
ofColor color_kick		= c3;
ofColor color_snare		= c6;
ofColor color_hihat		= c7;
ofColor color_sfondo	= c1;
ofColor color_playhead	= c8;
ofColor color_bpm		= c5;
ofColor color_bpm_corona = c4;
ofColor color_bass		= c9;
ofColor color_pads		= c2;

// ulteriore palette colori generata per differenzioare tra loro i vari accordi
ofColor color_accordo_0 = ofColor(77,	15,		52);	// Am
ofColor color_accordo_1 = ofColor(15,	76,		39);	// G
ofColor color_accordo_2 = ofColor(76,	70,		15);	// C
ofColor color_accordo_3 = ofColor(15,	21,		76);	// Em
ofColor color_accordo_4 = ofColor(34,	76,		15);	// F
ofColor color_accordo_5 = ofColor(76,	15,		40);	// D

//da Palette 2 - mode 0
ofColor verde	(68,	181,	16);
ofColor giallo	(221,	201,	48);
ofColor	azzurro (247,	54,		89);

// SETUP ///////////////////////////////////////////////////////////////
void testApp::setup() 
{

	// ----------------- SONORO ---------------- //
	int ticksPerBuffer = 8;	// 8 * 64 = buffer di 512 campioni
	//ofSoundStreamListDevices(); // da utilizzare se si voglia utilizzare una scheda audio esterna
	ofSoundStream ss;
	ss.setDeviceID(2);
	// setup del sound stream di OF 
	// (canali in output, canali in input, classe di riferimento, sample rate, campioni per buffer, numero di buffer)
	ofSoundStreamSetup(2, 0, this, 44100, ofxPd::getBlockSize()*ticksPerBuffer, 4);
	// setup di AppCore
	core.setup(2, 0, 44100, ticksPerBuffer);
	
	//------------------TUIO------------------ //
	ofAddListener(tuio.objectAdded,   this, &testApp::objectAdded);
	ofAddListener(tuio.objectRemoved, this, &testApp::objectRemoved);
	ofAddListener(tuio.objectUpdated, this, &testApp::objectUpdated);
	ofAddListener(tuio.cursorAdded,	  this, &testApp::cursorAdded);
	ofAddListener(tuio.cursorRemoved, this, &testApp::cursorRemoved);
	ofAddListener(tuio.cursorUpdated, this, &testApp::cursorUpdated);
	
	tuio.start(3333); // 3333 è la porta sulla quale si ricevono i messaggi TUIO
	
	
	//------------------ PLAY ------------------ //
	matrice.init();
	for (int i=0; i<PLANES; ++i)
	{
		matrice.print_table(i);
		cout << "\n";
	}
	
	// inizializzazione variabili di temporizzazione e gestione della messa in play
	initial_time = ofGetSystemTime();
	bpm = 120; // BPM di default
	croma_time=(60000 / (bpm*2)); // tempo di una croma in millisecondi 
	// 1 minuto = 60000 ms; 
	// 60000 ms / bpm = tempo di 1 semiminima; 
	// 60000 ms / (bpm*2) = tempo di 1 croma
	
	n_crome = 0;						// contatore di quante crome sono state messe in play fino ad ora
	last_croma_time = 8 * croma_time;	// una moltiplicazione per otto per far cominciare i conteggi dopo la prima battuta
	core.send_float("croma_time", (float)croma_time); //durata di una croma in ms, viene inviata a PD per la gestione del sequencer della linea di basso
	
	
	//------------------ SISTEMA --------------- //
	ofSetVerticalSync(true);
	ofSetFrameRate(60);
	
	
	//------------------ UTILITA' -------------- //
	// per la resa a schermo distorta
	margine = 100;
	quadro.setWidth(768);
	quadro.setHeight(768);
	wQuadro = quadro.getWidth();
	hQuadro = quadro.getHeight();
	
	wFbo  = hFbo  = wQuadro + 2*margine; 
	wMesh = hMesh = wFbo; 
	wWindow = ofGetWindowWidth();
	hWindow = ofGetWindowHeight();
	
	meshReset();

	// mesh
	mesh.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
	// mesh vertexes
	int latomezzi = wMesh*0.5;
	ofVec3f nw(-latomezzi, -latomezzi, 0);
	ofVec3f ne(+latomezzi, -latomezzi, 0);
	ofVec3f se(+latomezzi, +latomezzi, 0);
	ofVec3f sw(-latomezzi, +latomezzi, 0);
	// texture coords
	ofVec2f nwi(0,0);
	ofVec2f nei(wFbo, 0);
	ofVec2f sei(wFbo, hFbo);
	ofVec2f swi(0, hFbo);
	// mesh construction
	mesh.addVertex(nw);
	mesh.addTexCoord(nwi);
	mesh.addVertex(ne);
	mesh.addTexCoord(nei);
	mesh.addVertex(sw);
	mesh.addTexCoord(swi);
	mesh.addVertex(se);
	mesh.addTexCoord(sei);
	// allochiamo la memoria necessaria per FBO e TEXTURE
	fbo.allocate(wFbo, hFbo, GL_RGB);
	text.allocate(wFbo, hFbo, GL_RGB);
	
	
	
	//------------------ PLAY HEAD ------------- //
	playHeadPosition = 0;
	
	int larghezza = wQuadro/NCOLUMNS;
	for(int i = 0; i < NCOLUMNS; ++i) {
		striscie[i].init(0+i*larghezza, 0-margine, larghezza, hFbo, 20, color_playhead);
	}
	
	
	
	//------------------ VARIE ----------------- //
	centro.x = wFbo/2.0f;
	centro.y = hFbo/2.0f;
	
	bSetup = true; // normalmente, appena acceso il programma siamo in modalità setup
	bDebug = false;
	
	angolo = 0.0f;
	rot_vel = 0.0f;
	memoria_bpm_angle = 0.0f;
	
	digit.setup();
	
}



// UPDATE //////////////////////////////////////////////////////////////
void testApp::update() 
{
	
	if(bSetup) 
	{
		// se siamo in fase di setup iniziale 
		
		//core.update();
		tuio.getMessage();
				
		// chiamo il metodo UPDATE CONTINUOUS per i fiducial rotativi, quadrati e rotondi
		for (list<Fid_Rot*>::iterator it=rot_list.begin(); it !=rot_list.end(); ++it) 
			(*it)->update_continuos(0);
		
		for (list<Fid_Sqr*>::iterator it=sqr_list.begin(); it !=sqr_list.end(); ++it) 
			(*it)->update_continuos(0);
	
		for (list<Fid_Round*>::iterator it=rnd_list.begin(); it !=rnd_list.end(); ++it) 
			(*it)->update_continuos(0);
		
	} 
	else 
	{
		// se invece stiamo utilizzando il programma in modo normale
		
		core.update();
		tuio.getMessage();
		actual_time = ofGetSystemTime(); 
		time = actual_time - initial_time; //tempo trascorso dall'apertura del programma
		
		// TIME CONTROL ------------------------------------------------
		if(time - last_croma_time >= croma_time) //(n_crome!=storico_crome)
		{
			n_crome = n_crome%8;
			play(n_crome);
		
			last_croma_time=time;
			n_crome++;
		}

		croma_time = (60000 / (bpm*2));
		core.send_float("croma_time", (float)croma_time);

		// CENTER: update della posizione del contro tavolo -----------
		centro.x = wQuadro/2.0f;
		centro.y = hQuadro/2.0f;
	
		// chiamo il metodo UPDATE CONTINUOUS per tutti i fiducial
		for (list<Fid_Bass*>::iterator it=bass_list.begin(); it !=bass_list.end(); ++it) 
			(*it)->update_continuos(AppCore::bass_lvl);
		
		for (list<Fid_Rot*>::iterator it=rot_list.begin(); it !=rot_list.end(); ++it) 
			(*it)->update_continuos(playHeadPosition);
	
		for (list<Fid_Sqr*>::iterator it=sqr_list.begin(); it !=sqr_list.end(); ++it) 
			(*it)->update_continuos(playHeadPosition);
		
		for (list<Finger*>::iterator it=dito_list.begin(); it !=dito_list.end(); ++it) 
			(*it)->update_continuos();
		
		for (list<Fid_Round*>::iterator it=rnd_list.begin(); it !=rnd_list.end(); ++it) 
			(*it)->update_continuos(playHeadPosition);
		
		for (list<Fid_Synth*>::iterator it=sint_list.begin(); it !=sint_list.end(); ++it) 
			(*it)->update_continuos(playHeadPosition);
		
		for (list<Fid_Chords*>::iterator it=chords_list.begin(); it !=chords_list.end(); ++it) 
			(*it)->update_continuos(playHeadPosition);
		
	
		// DISEGNO DELLA PLAYHEAD -----------------------------------------
		//ofDrawBitmapString("CROMA ORA IN RIPRODUZIONE: " + ofToString(n_crome-1), 20, 20);
		playHeadPosition = ofMap( (time-last_croma_time), 0, croma_time, 0, stripe_w);
	
		// PRIMA RESA GRAFICA
		// questa però non è molto piacevole perchè rispecchia perfettamente il funzionamento 
		// del programma: il suono è emesso non appena la playhead tocca la croma. 
		// Il problema è che spesso, il disegno del fiducial si trova
		// spazialmente all'interno della striscia che rappresenta la croma in esame.
		// per questo, sentire il suono ma non vedere ancora la playhead sovrapporsi al 
		// disegno del fiducial può mandare in confusione,
		// per questo, meglio la SECONDA RESA GRAFICA
		// playHeadPosition = playHeadPosition + ((n_crome-1)*stripe_w);
	
		// SECONDA RESA GRAFICA
		// aggiungo un offset pari a metà della larghezza della stripe
		// sì da avere la playhead che passa, mediamente, sopra al fiducial al momento in cui il suono viene emesso
		playHeadPosition = playHeadPosition + ((n_crome-1)*stripe_w) + stripe_w/2;
		playHeadPosition = playHeadPosition % wQuadro;
		
		for(int i=0; i < NCOLUMNS; ++i) {
			striscie[i].update(20);
		}
		digit.update();
	}
}



// DRAW ////////////////////////////////////////////////////////////////
void testApp::draw() 
{
	
	if(bSetup) 
	{
		// se siamo in fase di setup iniziale 
		
		// COMINCIA IL DISEGNO ALL'INTERNO DELL'FBO //
		fbo.begin();
		ofEnableSmoothing();
		
		ofBackground(giallo);
	
		ofPushMatrix();
			ofTranslate(margine, margine, 0);
		
			ofPushStyle();
			ofSetColor(verde);
			ofRect(quadro);
			ofPopStyle();
		
			stripe_w = wQuadro / NUM_STRIPES;
		
			// la striscia corrispondente diventa verde se vi trovo un fiducial quadrato
			for (list<Fid_Sqr*>::iterator it=sqr_list.begin(); it !=sqr_list.end(); ++it) 
			{
				for(int i=0; i < NUM_STRIPES; ++i) 
				{
					if ( ((*it)->isAlive()) && ((*it)->getFidPos()->x >= (stripe_w*i)) && ((*it)->getFidPos()->x < (stripe_w*(i+1))) ) 
					{	
					ofPushStyle();
					ofEnableBlendMode(OF_BLENDMODE_ALPHA);
					ofSetColor(90, 200, 120, 100);
					ofFill();
					ofRect(stripe_w*i, 0, stripe_w, hQuadro );
					ofDisableBlendMode();
					ofPopStyle();
					} 			
				}
			}
				
			// la striscia corrispondente diventa verde se vi trovo un fiducial rotondo
			for (list<Fid_Round*>::iterator it=rnd_list.begin(); it !=rnd_list.end(); ++it) 
			{
				for(int i=0; i < NUM_STRIPES; ++i) 
				{
					if ( ((*it)->isAlive()) && ((*it)->getFidPos()->x >= (stripe_w*i)) && ((*it)->getFidPos()->x < (stripe_w*(i+1))) ) 
					{	
						ofPushStyle();
						ofEnableBlendMode(OF_BLENDMODE_ALPHA);
						ofSetColor(90, 200, 120, 100);
						ofFill();
						ofRect(stripe_w*i, 0, stripe_w, hQuadro );
						ofDisableBlendMode();
						ofPopStyle();
					} 			
				}	
			}
		
		
			for(int i=0; i < NUM_STRIPES; ++i) 
			{
			ofPushStyle();
			ofSetHexColor(0x000000);
			ofNoFill();
			ofRect(stripe_w*i, 0, stripe_w, hQuadro );
			ofPopStyle();
			}
		
			// DISEGNO DEI FIDUCIAL -------------------------------------------
			// chiamo il metodo DRAW per tutti i fiducial rotativi
			for (list<Fid_Rot*>::iterator it=rot_list.begin(); it !=rot_list.end(); ++it) 
				(*it)->draw();
			// chiamo il metodo DRAW per tutti i fiducial quadrati
			for (list<Fid_Sqr*>::iterator it=sqr_list.begin(); it !=sqr_list.end(); ++it) 		
				(*it)->draw();
			// chiamo il metodo DRAW per tutti i fiducial rotondi
			for (list<Fid_Round*>::iterator it=rnd_list.begin(); it !=rnd_list.end(); ++it) 
				(*it)->draw();
			
			ofPushStyle();		
			ofSetHexColor(0x000000);
		
			// CALIBRATION INFO BOX POSITION
			// settare qui la posizione dell'angolo superiore sinitro del riquadro
			// calibration info box
			int cibX	= wQuadro*0.5 + 20;
			int cibY	= hQuadro*0.5 - 200;
			ofDrawBitmapString("Rotation:           " + ofToString(meshRotation),		cibX, cibY + 0);
			ofDrawBitmapString("Mesh Center X:      " + ofToString(meshCenterX),		cibX, cibY + 20);
			ofDrawBitmapString("Mesh X Scale:       " + ofToString(meshScaleX),		cibX, cibY + 40);
			ofDrawBitmapString("Mesh Y Scale:       " + ofToString(meshScaleY),		cibX, cibY + 60);
			ofDrawBitmapString("Mesh Bottom Offset: " + ofToString(meshBottomOffset),	cibX, cibY + 80);
			ofDrawBitmapString("fps:                " + ofToString(ofGetFrameRate()),	cibX, cibY + 100);

			// CALIBRATION LEGENDA BOX POSITION
			// settare qui la posizione dell'angolo superiore sinitro del riquadro
			// calibration legenda box
			int clbX	= wQuadro*0.5 + 20;
			int clbY	= hQuadro*0.5 + 200;
			ofDrawBitmapString("ROTATE on X axis [ W - X ]", clbX, clbY + 0);
			ofDrawBitmapString("BOTTOM up/down   [ M - U ]", clbX, clbY + 20);
			ofDrawBitmapString("CENTER X pos     [ O - P ]", clbX, clbY + 40);
			ofDrawBitmapString("MESH X SCALE     [ G - J ]", clbX, clbY + 60);
			ofDrawBitmapString("MESH Y SCALE     [ N - Y ]", clbX, clbY + 80);
			ofDrawBitmapString("RESET            [   R   ]", clbX, clbY + 100);

			ofPopStyle();
		
			// disegno della crociera centrale
			ofPushMatrix();
				ofTranslate(wQuadro*0.5, hQuadro*0.5);
				int lsegmento = 50;
				int spazio = 30;
				ofPushStyle();
				ofSetLineWidth(10);
				ofSetHexColor(0x000000);
		
				ofPushMatrix();
					ofTranslate(-lsegmento-spazio, -spazio);
					ofLine(0, 0, lsegmento, 0);
				ofPopMatrix();
		
				ofPushMatrix();
					ofTranslate(-spazio, -spazio-lsegmento);
					ofLine(0, 0, 0, lsegmento);
				ofPopMatrix();
		
				ofPushMatrix();
					ofTranslate(spazio, -spazio-lsegmento);
					ofLine(0, 0, 0, lsegmento);
				ofPopMatrix();
		
				ofPushMatrix();
					ofTranslate(spazio, -spazio);
					ofLine(0, 0, lsegmento, 0);
				ofPopMatrix();
		
				ofPushMatrix();
					ofTranslate(spazio, spazio);
					ofLine(0, 0, lsegmento, 0);
				ofPopMatrix();
		
				ofPushMatrix();
					ofTranslate(spazio, spazio);
					ofLine(0, 0, 0, lsegmento);
				ofPopMatrix();
		
				ofPushMatrix();
					ofTranslate(-spazio, spazio);
					ofLine(0, 0, 0, lsegmento);
				ofPopMatrix();
		
				ofPushMatrix();
					ofTranslate(-spazio-lsegmento, spazio);
					ofLine(0, 0, lsegmento, 0);
				ofPopMatrix();
		
			
			
		
			ofPopStyle();
		ofPopMatrix(); //ofTranslate(margine, margine, 0);

		// disegno degli angoli
		ofPushStyle();
		ofSetLineWidth(10);
		ofSetHexColor(0x000000);
		
		ofPushMatrix();
			ofTranslate(0 + spazio, 0 + spazio, 0);
			ofLine(0, 0, lsegmento, 0);
			ofLine(0, 0, 0, lsegmento);
		ofPopMatrix();
		
		ofPushMatrix();
			ofTranslate(wQuadro - spazio, 0 + spazio, 0);
			ofLine(0, 0, -lsegmento, 0);
			ofLine(0, 0, 0, lsegmento);
		ofPopMatrix();
		
		ofPushMatrix();
			ofTranslate(wQuadro - spazio, hQuadro - spazio, 0);
			ofLine(0, 0, -lsegmento, 0);
			ofLine(0, 0, 0, -lsegmento);
		ofPopMatrix();
		
		ofPushMatrix();
			ofTranslate(0 + spazio, hQuadro - spazio, 0);
			ofLine(0, 0, lsegmento, 0);
			ofLine(0, 0, 0, -lsegmento);
		ofPopMatrix();
		
		
		ofPopStyle();
		
		ofPopMatrix();
		fbo.end();
		
		
	} 
	else
	{
		// se invece stiamo utilizzando il programma in modo normale
		
		// COMINCIA IL DISEGNO ALL'INTERNO DELL'FBO //
		fbo.begin(); 
		ofEnableSmoothing();
	
		ofPushStyle();
		ofEnableBlendMode(OF_BLENDMODE_MULTIPLY);
		
		backgroundGradient(ofColor(255), ofColor(10), wFbo, hFbo);
		ofSetColor(color_sfondo);
		ofRect(0, 0, wFbo, hFbo);
		ofDisableBlendMode();
		ofPopStyle();
	
		ofPushMatrix();
		ofTranslate(margine, margine);
		
		stripe_w = wQuadro / NUM_STRIPES;
	
		// se vi trovo un fiducial quadrato
		for (list<Fid_Sqr*>::iterator it=sqr_list.begin(); it !=sqr_list.end(); ++it) 
		{
			for(int i=0; i < NUM_STRIPES; ++i) 
			{
				if ( ((*it)->isAlive()) && ((*it)->getFidPos()->x >= (stripe_w*i)) && ((*it)->getFidPos()->x < (stripe_w*(i+1))) ) 
				{	
					ofPushStyle();
					ofEnableBlendMode(OF_BLENDMODE_ALPHA);
					ofSetColor(90, 200, 120, 100);
					ofFill();
					// il primo e ultimo STRIPES devono essere larghi più delle stripes centrali
					if(i == 0)
						ofRect(stripe_w*i-margine, 0-margine, stripe_w+margine, hFbo );
					else if (i == NUM_STRIPES -1)
						ofRect(stripe_w*i, 0-margine, stripe_w+margine, hFbo );
					else 
						ofRect(stripe_w*i, 0-margine, stripe_w, hFbo );
					
					ofDisableBlendMode();
					ofPopStyle();
				} 			
			}
		}
		
		// se vi trovo un fiducial rotondo
		for (list<Fid_Round*>::iterator it=rnd_list.begin(); it !=rnd_list.end(); ++it) 
		{
			for(int i=0; i < NUM_STRIPES; ++i) 
			{
				if ( ((*it)->isAlive()) && ((*it)->getFidPos()->x >= (stripe_w*i)) && ((*it)->getFidPos()->x < (stripe_w*(i+1))) ) 
				{	
					ofPushStyle();
					ofEnableBlendMode(OF_BLENDMODE_ALPHA);
					ofSetColor(90, 200, 120, 100);
					ofFill();
					// il primo e ultimo STRIPES devono essere larghi più delle stripes centrali
					if(i == 0)
						ofRect(stripe_w*i-margine, 0-margine, stripe_w+margine, hFbo );
					else if (i == NUM_STRIPES -1)
						ofRect(stripe_w*i, 0-margine, stripe_w+margine, hFbo );
					else 
						ofRect(stripe_w*i, 0-margine, stripe_w, hFbo );
					
					ofDisableBlendMode();
					ofPopStyle();
				} 			
			}
		}
	
	
		for(int i=1; i < NUM_STRIPES-1; ++i) // disegnamo i contorni delle sole stripes centrali
		{
			ofPushStyle();
			ofSetHexColor(0x000000);
			ofNoFill();
			ofRect(stripe_w*i, 0-margine, stripe_w, hFbo );
			ofPopStyle();
		}
	

		// DISEGNO DELLA PLAYHEAD -----------------------------------------
		// striscie
		
		//cout << playHeadPosition << "\n";
		for(int i=0; i < NCOLUMNS; ++i) {
			//striscie[i].update(20);
			striscie[i].checka(playHeadPosition);
		}	
		//playhead
		ofPushStyle();
		ofSetLineWidth(5); 
		ofSetColor(color_playhead);
		ofLine(playHeadPosition, 0-margine, playHeadPosition, hFbo);
		ofPopStyle();
	
	
		// DISEGNO DEI FIDUCIAL -------------------------------------------
		// chiamo il metodo DRAW per tutti i fiducial BASSO
		for (list<Fid_Bass*>::iterator it=bass_list.begin(); it !=bass_list.end(); ++it) 
			(*it)->draw();
		// chiamo il metodo DRAW per tutti i fiducial CHORDS
		for (list<Fid_Chords*>::iterator it=chords_list.begin(); it !=chords_list.end(); ++it) 
			(*it)->draw();
		// chiamo il metodo DRAW per tutti i fiducial SYNTH
		for (list<Fid_Synth*>::iterator it=sint_list.begin(); it !=sint_list.end(); ++it) 
			(*it)->draw();
		// chiamo il metodo DRAW per tutti i fiducial rotativi (BPM)
		for (list<Fid_Rot*>::iterator it=rot_list.begin(); it !=rot_list.end(); ++it) 
			(*it)->draw();
		// chiamo il metodo DRAW per tutti i fiducial quadrati (SNARE + HIHAT)
		for (list<Fid_Sqr*>::iterator it=sqr_list.begin(); it !=sqr_list.end(); ++it) 		
			(*it)->draw();
		// chiamo il metodo DRAW per tutti i fiducial rotondi (KICK)
		for (list<Fid_Round*>::iterator it=rnd_list.begin(); it !=rnd_list.end(); ++it) 
			(*it)->draw();
		// chiamo il metodo DRAW per tutti i finger (FINGER)
		for (list<Finger*>::iterator it=dito_list.begin(); it !=dito_list.end(); ++it) 
			(*it)->draw();
		
		// DISEGNO LE INFO DEL DEBUG --------------------------------------
		// se debuggo, chiamo il metodo DRAW per tutti i fiducial
		if(bDebug)
		{
			
			for (list<Fid_Bass*>::iterator it=bass_list.begin(); it !=bass_list.end(); ++it) 
				(*it)->debug();
			
			for (list<Fid_Chords*>::iterator it=chords_list.begin(); it !=chords_list.end(); ++it) 
				(*it)->debug();
			
			for (list<Fid_Synth*>::iterator it=sint_list.begin(); it !=sint_list.end(); ++it) 
				(*it)->debug();
			
			for (list<Fid_Rot*>::iterator it=rot_list.begin(); it !=rot_list.end(); ++it) 
				(*it)->debug();
		
			for (list<Fid_Sqr*>::iterator it=sqr_list.begin(); it !=sqr_list.end(); ++it) 
				(*it)->debug();
			
			for (list<Finger*>::iterator it=dito_list.begin(); it !=dito_list.end(); ++it) 
				(*it)->debug();
			
			for (list<Fid_Round*>::iterator it=rnd_list.begin(); it !=rnd_list.end(); ++it) 
				(*it)->debug();
			
	
			// centro del tavolo
			ofPushStyle();
			ofSetHexColor(0xFF0000);
			ofLine(wQuadro/2.0f - 10, hQuadro/2.0f,wQuadro/2.0f + 10, hQuadro/2.0f);
			ofLine(wQuadro/2.0f, hQuadro/2.0f - 10,wQuadro/2.0f, hQuadro/2.0f + 10);
			ofSetHexColor(0x000000);
			ofDrawBitmapString("CENTER", wQuadro*0.5+5, hQuadro*0.5-5); 	
			ofDrawBitmapString("fps: " + ofToString((int)ofGetFrameRate()), wQuadro*0.5+5, hQuadro*0.5+20);
			// dimensioni dell'fbo
			//ofSetHexColor(0x000000);
			//ofDrawBitmapString("Fbo size:\n" + ofToString((int)wFbo) + " x " + ofToString((int)hFbo) + "\n", wFbo-100, 10);
			// indicazione sull'fps
			ofDrawBitmapString("BPM: " + ofToString(bpm), wQuadro-100, 60);
	
			ofPopStyle();
		}
	 
		digit.draw(wQuadro);		

		ofDisableSmoothing();
		ofPopMatrix();
		fbo.end(); // fine del disegno su FBO
	
	}

	
	// CARICO I DATI NELLA TEXTURE //
	text = fbo.getTextureReference();
	
	// DISEGNO LA MESH CON LA TEXTURE BINDATA //
	ofPushStyle();
	
	// disegno lo sfondo
	if(bSetup) 
	{
		ofBackground(giallo);
	} 
	else 
	{
		ofBackground(0, 0, 0);
	}
	
	
	
	ofPushMatrix();
	
	ofTranslate(wWindow*0.5 + meshCenterX, hWindow-meshBottomOffset, 0);

	ofRotateX( meshRotation ); 
	ofScale(meshScaleX, meshScaleY, 1); // eventualmente servisse scalare la mesh
	
	ofTranslate(0, -wMesh*0.5+margine, 0);
	
	ofEnableBlendMode(OF_BLENDMODE_ALPHA);
	
	text.bind(); // bind the image to begin texture mapping
	
	//mesh.drawWireframe();
	mesh.draw();
	
	text.unbind();
	
	ofDisableBlendMode();
	
	ofPopMatrix();
	ofPopStyle();


}



// EXIT ////////////////////////////////////////////////////////////////
void testApp::exit() 
{
	core.exit();
}



// ALTRI METODI ////////////////////////////////////////////////////////
// ogni volta che premo un determinato pulsante sulla tastiera del computer
// questo si traduce in un comportamento del programma
void testApp::keyPressed(int key) 
{
	switch (key) 
	{
		case 'd':
		case 'D':
			if(!bSetup)
				bDebug = !bDebug;
			break;
		case 'f':
		case 'F':

			ofToggleFullscreen();
			std::cout << "Window Mode = " << ofGetWindowMode() << "\n";
			if(ofGetWindowMode() == 1) {
				wWindow = ofGetScreenWidth();
				hWindow = ofGetScreenHeight();
				std::cout << wWindow << ", " << hWindow << ";\n";
				
			} else {
				wWindow = 640;
				hWindow = 640;
				std::cout << wWindow << ", " << hWindow << ";\n";
				
			}
			std::cout << "\n";

			break;
		case 'h':
		case 'H':
			if(!bDebug)
				bSetup = !bSetup;
			break;
			
		case 'g':
		case 'G':
			if(bSetup)
				meshScaleX -= 0.005;
			break;
		case 'j':
		case 'J':
			if(bSetup)
				meshScaleX += 0.005;
			break;
			
		case 'n':
		case 'N':
			if(bSetup)
				meshScaleY -= 0.005;
			break;
		case 'y':
		case 'Y':
			if(bSetup)
				meshScaleY += 0.005;
			break;
			
		case 'w':
		case 'W':
			if(bSetup)
				meshRotation -= 1;
			break;
		case 'x':
		case 'X':
			if(bSetup)
				meshRotation += 1;
			break;
						
		case 'r':
		case 'R':
			if(bSetup) {
				meshReset();
			}
			break;
			
		case 'm':
		case 'M':
			if(bSetup) {
				meshBottomOffset -= 0.5;;
			}
			break;
			
		case 'u':
		case 'U':
			if(bSetup) {
				meshBottomOffset += 0.5;;
			}
			break;
			
		case 'o':
		case 'O':
			if(bSetup) {
				meshCenterX -= 0.5;;
			}
			break;
			
		case 'p':
		case 'P':
			if(bSetup) {
				meshCenterX += 0.5;;
			}
			break;

		default:
			printf("press key - default\n");
	}
}


void testApp::audioReceived(float * input, int bufferSize, int nChannels) 
{
	core.audioReceived(input, bufferSize, nChannels);
}

void testApp::audioRequested(float * output, int bufferSize, int nChannels) 
{
	core.audioRequested(output, bufferSize, nChannels);
}



// OBJECT ADDED ////////////////////////////////////////////////////////
// ogni volta che una oggetto viene aggiunto sulla superficie di gioco
void testApp::objectAdded(ofxTuioObject & tuioObject)
{	
	int posX; // variabile di appoggio per identificare la posizione relativa del fiducial dentro alla matrice 
	int session_id	= tuioObject.getSessionId();
	int fiducial_id = tuioObject.getFiducialId();
	int piano;
	
	angolo = tuioObject.getAngle() * (-1);			// passo anche il (-1) perchè da reactivision, l'angolo è invertito
	rot_vel = tuioObject.getRotationSpeed() * (-1);	// passo anche il (-1) perchè da reactivision, l'angolo è invertito
	// calcolo la posizione del fiducial per disegnarla graficamente
	pos.x = wQuadro * tuioObject.getX();		// i valori tuioObject.getX() e tuioObject.getY() variano tra 0 e 1
	pos.y = hQuadro * tuioObject.getY();
	

	switch(fiducial_id)
	{
		 
		case KICK:
		{

			posX = tuioObject.getX() * 8; //in che croma siamo?
			piano=1;
			matrice.add_to_table(posX, piano, session_id);
			
			rotondo = new Fid_Round(fiducial_id, session_id);
			rnd_list.push_back(rotondo);
			rnd_list.back()->setup(&pos, &centro, angolo, color_kick);
			rnd_list.back()->added();
			
			break;
		}
		case SNARE:
		{
			
			posX = tuioObject.getX() * 8;
			piano=2;
			matrice.add_to_table(posX, piano, session_id);
			
			quadrato = new Fid_Sqr(fiducial_id, session_id);
			sqr_list.push_back(quadrato);
			sqr_list.back()->setup(&pos, &centro, angolo, color_snare);
			sqr_list.back()->added();
			
			break;
		}
		case HIHAT: 
		{
			posX = tuioObject.getX() * 8;
			piano=3;
			matrice.add_to_table(posX, piano, session_id);
			
			quadrato = new Fid_Sqr(fiducial_id, session_id);
			sqr_list.push_back(quadrato);
			sqr_list.back()->setup(&pos, &centro, angolo, color_hihat);
			sqr_list.back()->added();

			break;
		}
		case BPM: 
		{			
			
			rotativo = new Fid_Rot(fiducial_id, session_id);
			rot_list.push_back(rotativo);
			rot_list.back()->setup(&pos, &centro, angolo, memoria_bpm_angle, color_bpm, color_bpm_corona);
			rot_list.back()->added();
			rot_list.back()->update_interrupt(&pos, &centro, angolo, rot_vel); //commentarlo?
			// aggiorno la mamoria dell'angolo per poterlo riutilizzare la prossima creazione di uno stsso tipo d'oggetto
			memoria_bpm_angle = rot_list.back()->get_lim_angle(); 
			digit.set_bpm(bpm);
			break;

		}
		case BASS:
		{
			core.send_float("bass", 1.0);
			
			
			//posX = tuioObject.getX() * 8; //in che croma siamo?
			//piano=1;
			//matrice.add_to_table(posX, piano, session_id);
			
			basso = new Fid_Bass(fiducial_id, session_id);
			bass_list.push_back(basso);
			bass_list.back()->setup(&pos, &centro, angolo, color_bass);
			bass_list.back()->added();
			
			break;			
		}
		case PADS:
		{
			core.send_float("pads", 1.0);
			
			//posX = tuioObject.getX() * 8;
			//piano=3;
			//matrice.add_to_table(posX, piano, session_id);
			
			pads = new Fid_Synth(fiducial_id, session_id);
			sint_list.push_back(pads);
			sint_list.back()->setup(&pos, &centro, angolo, color_pads);
			sint_list.back()->added();
			
			break;
		}
		case CHORD_0:
		{
			chord_number=0;
			core.send_float("chords", chord_number);
			
			accordo = new Fid_Chords(fiducial_id, session_id);
			chords_list.push_back(accordo);
			chords_list.back()->setup(&pos, &centro, angolo, color_accordo_0);
			chords_list.back()->added();
			chords_list.back()->update_interrupt(&pos, &centro, angolo, rot_vel); //commentarlo?

			
			break;
		}
		case CHORD_1:
		{
			chord_number=1;
			core.send_float("chords", chord_number);
			
			accordo = new Fid_Chords(fiducial_id, session_id);
			chords_list.push_back(accordo);
			chords_list.back()->setup(&pos, &centro, angolo, color_accordo_1);
			chords_list.back()->added();
			chords_list.back()->update_interrupt(&pos, &centro, angolo, rot_vel);
			
			break;
		}
		case CHORD_2:
		{
			chord_number=2;
			core.send_float("chords", chord_number);
			
			accordo = new Fid_Chords(fiducial_id, session_id);
			chords_list.push_back(accordo);
			chords_list.back()->setup(&pos, &centro, angolo, color_accordo_2);
			chords_list.back()->added();
			chords_list.back()->update_interrupt(&pos, &centro, angolo, rot_vel);
			
			break;
		}
		case CHORD_3:
		{
			chord_number=3;
			core.send_float("chords", chord_number);
			
			accordo = new Fid_Chords(fiducial_id, session_id);
			chords_list.push_back(accordo);
			chords_list.back()->setup(&pos, &centro, angolo, color_accordo_3);
			chords_list.back()->added();
			chords_list.back()->update_interrupt(&pos, &centro, angolo, rot_vel);
			
			break;
		}
		case CHORD_4:
		{
			chord_number=4;
			core.send_float("chords", chord_number);
			
			accordo = new Fid_Chords(fiducial_id, session_id);
			chords_list.push_back(accordo);
			chords_list.back()->setup(&pos, &centro, angolo, color_accordo_4);
			chords_list.back()->added();
			chords_list.back()->update_interrupt(&pos, &centro, angolo, rot_vel);
			
			break;
		}
		case CHORD_5:
		{
			chord_number=5;
			core.send_float("chords", chord_number);
			
			accordo = new Fid_Chords(fiducial_id, session_id);
			chords_list.push_back(accordo);
			chords_list.back()->setup(&pos, &centro, angolo, color_accordo_5);
			chords_list.back()->added();
			chords_list.back()->update_interrupt(&pos, &centro, angolo, rot_vel);
			
			break;
		}
		default:
		{
			cout << "Fiducial ID non valido.\n";
			break;
		}
	}
		
}



// OBJECT REMOVED //////////////////////////////////////////////////////
// ogni volta che una oggetto viene rimosso dalla superficie di gioco
void testApp::objectRemoved(ofxTuioObject & tuioObject)
{
	int session_id	= tuioObject.getSessionId();
	int fiducial_id	= tuioObject.getFiducialId();
	int piano;
	
	switch(fiducial_id)
	{
		case KICK:
		{
			piano=1;
			// forse è superfluo passare il "piano" alla funzione "matrice.remove_from_table"
			matrice.remove_from_table(piano, session_id);
			
			list<Fid_Round*>::iterator it;
			for (it=rnd_list.begin(); it !=rnd_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->removed();
					break;
				}
			}
			rnd_list.erase(it);
			
			
			break;
		}
		case SNARE:
		{
			piano=2;
			matrice.remove_from_table(piano, session_id);
			
			list<Fid_Sqr*>::iterator it;
			for (it=sqr_list.begin(); it !=sqr_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->removed();
					break;
				}
			}
			sqr_list.erase(it);
			break;
		}
		case HIHAT:
		{
			piano=3;
			matrice.remove_from_table(piano, session_id);
			
			list<Fid_Sqr*>::iterator it;
			for (it=sqr_list.begin(); it !=sqr_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->removed();
					break;
				}
			}
			sqr_list.erase(it);
			break;
		}
		case BPM:
		{

			list<Fid_Rot*>::iterator it;
			for (it=rot_list.begin(); it !=rot_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->removed();
					break;
				}
			}
			rot_list.erase(it);
			
			break;
		}
		case BASS:
		{
			core.send_float("bass", 0.0);
			
			//piano=1;
			// forse è superfluo passare il "piano" alla funzione "matrice.remove_from_table"
			//matrice.remove_from_table(piano, session_id);
			
			list<Fid_Bass*>::iterator it;
			for (it=bass_list.begin(); it !=bass_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->removed();
					break;
				}
			}
			bass_list.erase(it);
			
			break;
			
		}
		case PADS:
		{
			core.send_float("pads", 0.0);
	
			//piano=3;
			//matrice.remove_from_table(piano, session_id);
				
			list<Fid_Synth*>::iterator it;
			for (it=sint_list.begin(); it !=sint_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->removed();
					break;
				}
			}
			sint_list.erase(it);
			
			break;
		}
		case CHORD_0:
		case CHORD_1:
		case CHORD_2:
		case CHORD_3:
		case CHORD_4:
		case CHORD_5:
		{
			
			list<Fid_Chords*>::iterator it;
			for (it=chords_list.begin(); it !=chords_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->removed();
					break;
				}
			}
			chords_list.erase(it);
			
			break;
		}
		default:
		{
			cout << "Fiducial ID non valido.\n";
			break;
		}
	}
	

}



// OBJECT UPDATED //////////////////////////////////////////////////////
// ogni volta che una oggetto viene modificato sulla superficie di gioco
void testApp::objectUpdated(ofxTuioObject & tuioObject)
{	
	int posX;
	int session_id	= tuioObject.getSessionId();
	int fiducial_id	= tuioObject.getFiducialId();
	int piano;

	angolo = tuioObject.getAngle() * (-1);			// passo anche il (-1) perchè da reactivision, l'angolo è invertito
	rot_vel = tuioObject.getRotationSpeed() * (-1);	// passo anche il (-1) perchè da reactivision, l'angolo è invertito
	pos.x = wQuadro * tuioObject.getX();
	pos.y = hQuadro * tuioObject.getY();	
	
	switch(fiducial_id)
	{
		case KICK:
		{
			posX = tuioObject.getX() * 8;
			piano=1;
			matrice.update_table(posX, piano, session_id);
			
			for (list<Fid_Round*>::iterator it=rnd_list.begin(); it !=rnd_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->update_interrupt(&pos, &centro, angolo, rot_vel);	
					break;
				}
				
			}	
			break;
		}
		case SNARE:
		{
			posX = tuioObject.getX() * 8;
			piano=2;
			matrice.update_table(posX, piano, session_id);
	
			for (list<Fid_Sqr*>::iterator it=sqr_list.begin(); it !=sqr_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->update_interrupt(&pos, &centro, angolo, rot_vel);	
					break;
				}
				
			}
			break;
		}
		case HIHAT:
		{
			posX = tuioObject.getX() * 8;
			piano=3;
			matrice.update_table(posX, piano, session_id);

			for (list<Fid_Sqr*>::iterator it=sqr_list.begin(); it !=sqr_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->update_interrupt(&pos, &centro, angolo, rot_vel);	
					break;
				}
			}
			break;
		}
		case BPM:
		{

			for (list<Fid_Rot*>::iterator it=rot_list.begin(); it !=rot_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->update_interrupt(&pos, &centro, angolo, rot_vel);
					// aggiorno la mamoria dell'angolo per poterlo riutilizzare la porssima creazione di uno stsso tipo d'oggetto
					memoria_bpm_angle = rot_list.back()->get_lim_angle(); 
					
					break;
				}
				
			}
			
			bpm = ofMap(memoria_bpm_angle, -FIDUCIAL_MER, FIDUCIAL_MER, 30, 260, true);
			digit.set_bpm(bpm);
			break;

		}
		case BASS:
		{
			//posX = tuioObject.getX() * 8;
			//piano=1;
			//matrice.update_table(posX, piano, session_id);
			
			for (list<Fid_Bass*>::iterator it=bass_list.begin(); it !=bass_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->update_interrupt(&pos, &centro, angolo, rot_vel);	
					break;
				}
				
			}	
			break;
		}
		case CHORD_0:
		case CHORD_1:
		case CHORD_2:
		case CHORD_3:
		case CHORD_4:
		case CHORD_5:
		{
			
			for (list<Fid_Chords*>::iterator it=chords_list.begin(); it !=chords_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->update_interrupt(&pos, &centro, angolo, rot_vel);
					// aggiorno la mamoria dell'angolo per poterlo riutilizzare la porssima creazione di uno stsso tipo d'oggetto
					//memoria_bpm_angle = rot_list.back()->get_lim_angle(); 
					
					break;
				}
				
			}
			
			//bpm = ofMap(memoria_bpm_angle, -FIDUCIAL_MER, FIDUCIAL_MER, 30, 260, true);
			//digit.set_bpm(bpm);
			break;
			
		}
		case PADS:
		{
			//posX = tuioObject.getX() * 8;
			//piano=2;
			//matrice.update_table(posX, piano, session_id);
			
			for (list<Fid_Synth*>::iterator it=sint_list.begin(); it !=sint_list.end(); ++it) 
			{
				if ( (*it)->get_s_id() == session_id) 
				{
					(*it)->update_interrupt(&pos, &centro, angolo, rot_vel);	
					break;
				}
				
			}
			break;
		}
		default:
		{
			cout << "Fiducial ID non valido.\n";
			break;
			
		}
	}
}



// FINGER ADDED ////////////////////////////////////////////////////////
void testApp::cursorAdded(ofxTuioCursor & tuioCursor)
{
	int finger_id = tuioCursor.getFingerId();
	pos.x = wQuadro * tuioCursor.getX();
	pos.y = hQuadro * tuioCursor.getY();
	
	dito = new Finger(finger_id);
	dito_list.push_back(dito);
	dito_list.back()->setup(&pos, blue);
	dito_list.back()->added();
	
	
	
	
}


// FINGER REMOVED //////////////////////////////////////////////////////
void testApp::cursorRemoved(ofxTuioCursor & tuioCursor)
{
	int finger_id = tuioCursor.getFingerId();
	pos.x = wQuadro * tuioCursor.getX();
	pos.y = hQuadro * tuioCursor.getY();
	
	list<Finger*>::iterator it;
	for (it=dito_list.begin(); it !=dito_list.end(); ++it) 
	{
		if ( (*it)->get_finger_id() == finger_id) 
		{
			(*it)->removed();
			break;
		}
	}
	dito_list.erase(it);
}



// FINGER UPDATED //////////////////////////////////////////////////////
void testApp::cursorUpdated(ofxTuioCursor & tuioCursor)
{
	int finger_id = tuioCursor.getFingerId();
	pos.x = wFbo * tuioCursor.getX();
	pos.y = hFbo * tuioCursor.getY();
	int x_speed = tuioCursor.getXSpeed();
	int y_speed = tuioCursor.getYSpeed();
	int motion_speed = tuioCursor.getMotionSpeed();
	int motion_accel = tuioCursor.getMotionAccel();
	
	for (list<Finger*>::iterator it=dito_list.begin(); it !=dito_list.end(); ++it) 
	{
		if ( (*it)->get_finger_id() == finger_id) 
		{
			(*it)->update_interrupt(&pos);	
			break;
		}
	}
}




// PLAY ////////////////////////////////////////////////////////////////
// questo metodo prende in ingresso un numero intero che varia tra 0 - 7
// si occupa di inviare un 'bang' a PureData (inviandolo al receiver denominato "colon_bang")
// si occupa inoltre di analizzare la matrice dei suoni per capire quali debbano essere messi in riproduzione
//
void testApp::play(int colonna) 
{
	
	int piano;
	int riga;
	char *s;

	core.bang("colon_bang"); // per il sequencer del basso
	
	for (piano=1; piano<PLANES; ++piano)
	{
		for(riga=0; riga<ROWS; ++riga)
		{
			if (matrice.get_element(riga, colonna, piano)==1) 
			{
				switch (piano)
				{
					case 1:
						s="kick";
						break;
					case 2:
						s="snare";
						break;
					case 3:
						s="hihat";
						break;
					default:
						cout << "Unavailable Instrument.\n";
						break;
				}
				core.bang(s);
				break;
				// questo break è per passare immediatamente al piano successivo
				// non appena si trovi, anche soltanto uno, strumento
				// al piano attuale
			}
		}
	}
}
		

//----------------------------------------------------------
// questa è una versione modificata rispetto a quella presente di default nel parco di funzioni 
// fornite da OpenFrameworks versione 0.74. La funzione originale è la ofBackgroundGradient() .
void testApp::backgroundGradient(const ofColor& start, const ofColor& end, int w_, int h_) 
{
	float w = w_, h = h_;
	ofMesh mesh;
	mesh.setMode(OF_PRIMITIVE_TRIANGLE_FAN);
	
		// this could be optimized by building a single mesh once, then copying
		// it and just adding the colors whenever the function is called.
		ofVec2f center(w / 2, h / 2);
		mesh.addVertex(center);
		mesh.addColor(start);
		int n = 32; // circular gradient resolution
		float angleBisector = TWO_PI / (n * 2);
		float smallRadius = ofDist(0, 0, w / 2, h / 2);
		float bigRadius = smallRadius / cos(angleBisector);
		for(int i = 0; i <= n; i++) {
			float theta = i * TWO_PI / n;
			mesh.addVertex(center + ofVec2f(sin(theta), cos(theta)) * bigRadius);
			mesh.addColor(end);
		}
	
	
	glDepthMask(false);
	mesh.draw();
	glDepthMask(true);
}


void testApp::meshReset() 
{
	// nella fase di setup iniziale del tavolo può essere necessario 
	// reimpostare i vari valori a quelli iniziali.
	
	// da prototipo 9
	meshRotation		= 8;		// valore in gradi 
	meshCenterX			= 6.0;
	meshScaleX			= 1.085;
	meshScaleY			= 1.005;
	meshBottomOffset	= 56.0;
}

