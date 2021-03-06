/*
 *  Fid_Bass.h
 *  Prototipo3
 *
 *  Created by Limulo.
 *  http://www.limulo.net 
 *  
 * Questa sottoclasse serve per dare una rappresentazione grafica al fiducial che rappresenta il BASS
 *
 */


#ifndef _INC_FID_BASS
#define _INC_FID_BASS

#include "ofMain.h"
#include <math.h>
#include <stdio.h>
#include <iostream>
#include "Fid_Base.h"

#define CIRCLE_RES		35		// risoluzione delcerchio

#define BASS_MAX_AMP	15		// quando suona il basso questa è la massima area del cerchio

#define STEP_IN			25		// velocità che impiega il fiducial a comparire sullo schermo
#define	STEP_OUT		12		// velocità che impiega il fiducial a scomparire dallo schermo


class Fid_Bass: virtual public Fid_Base 
{
private:
	
protected:
	float	c_innerRadius;
	float	bass_level;
	ofColor	c_color;
	ofVec3f	z_axis;				// asse-z
		
	int fr, fg, fb;				// fiducial color (colore del tondo pieno)
	
public:
	Fid_Bass(int _fid, int _sid);
	~Fid_Bass() {cout << "De-constructiong derived: Fid_Bass\n"; }
	
	void setup(ofVec2f *_fid_pos, ofVec2f *_ctr_pos, float _fid_angle, ofColor _fColor);
	
	void added();
	void update_interrupt(ofVec2f *_fid_pos, ofVec2f *_ctr_pos, float _fid_angle, float _fid_rot_vel);
	void update_continuos(int bass_level_); 
	void removed();
	
	void draw();
	
	ofVec2f* getFidPos();
	bool isAlive();	
	
	void inside(ofVec2f *p);
	void debug();
	
	
};

#endif
