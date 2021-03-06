/*
 *  FId_Sqr.h
 *  Prototipo3
 *
 *  Created by Limulo.
 *  http://www.limulo.net 
 *  
 * Questa sottoclasse serve per dare una rappresentazione grafica ai fiducial cui è stata assegnata una forma quadrata
 * si tratta dei fiducial SNARE e HIHAT
 */

#ifndef _INC_FID_SQR
#define _INC_FID_SQR


#include "Fid_Base.h"
#include "ofMain.h"
#include <math.h>
#include <stdio.h>

#define STEP_IN			25		// velocità che impiega il fiducial a comparire sullo schermo
#define	STEP_OUT		12		// velocità che impiega il fiducial a scomparire dallo schermo


class Fid_Sqr: virtual public Fid_Base {
private:
	int r;
	int g;
	int b;
	
	
public:
	Fid_Sqr(int _fid, int _sid);
	~Fid_Sqr() {std::cout << "De-constructiong derived: Fid_Sqr!\n" << std::endl; }
	
	void setup(ofVec2f *_fid_pos, ofVec2f *_ctr_pos, float _fid_angle, ofColor _color);
	void added();
	
	void update_interrupt(ofVec2f *fiducial_pos, ofVec2f *centro, float angolo_attuale, float vel);
	void update_continuos(int playHeadPos_);
	
	void removed();
	
	void draw();
	
	ofVec2f* getFidPos();
	bool isAlive();	
	
	void inside(ofVec2f *p);
	void debug();
	
};

#endif


