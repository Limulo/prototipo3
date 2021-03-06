/*
 *  Fid_Chords.cpp
 *  Prototipo3
 *
 *  Created by Limulo.
 *  http://www.limulo.net 
 */

#include "Fid_Chords.h"


// COSTRUTTORE /////////////////////////////////////////////////
Fid_Chords::Fid_Chords(int _fid, int _sid)
{
	std::cout << "Constructiong derived!\n" << std::endl;
	
	fid = _fid;
	sid = _sid;
	
	c_innerRadius = FIDUCIAL_R + CORONA_OFF_CHORDS;
	c_outerRadius = (CORONA_L) + c_innerRadius;
	
	fid_pos.set(0.0f, 0.0f);
	fid_ctr_angle = 0.0f;
	
	z_axis.set(0, 0, 1);
	
	transparency	= 0;
	stato = STABLE;
	
	alive = false;
	
	// variabili utili per l'animazione del trigger
	// ossia quando la playhead raggiunge la posizione del fiducial
	bTrigger = false;
	bExpand = false;
	bCollapse = false;
	tExpand = 10;		// valore temporale (espresso in frames) per l'espansione
	tCollapse = 480;	// valore temporale (espresso in frames) per il ritorno alle normali dimensioni
	aTrigger = 80;		// ampiezza di dilatazione della forma del fiducial quando venga triggerato
	startFrame = 0;
	marginTrigger = 30;	// raggio di un cerchio immaginario, tracciato contrandolo sulla posizione del fiducial. 	
}


// SETUP ///////////////////////////////////////////////////////
void Fid_Chords::setup(ofVec2f *_fid_pos, ofVec2f *_ctr_pos, float _fid_angle, ofColor _cColor)
{
	// SETUP POSITION --------------------------------------------------------------
	fid_pos.set(_fid_pos->x, _fid_pos->y);
	fid_angle = _fid_angle;
	ofSetCircleResolution(CORONA_RES);
		
	cr = _cColor.r;
	cg = _cColor.g;
	cb = _cColor.b;
	
}

// ADDED ///////////////////////////////////////////////////////
void Fid_Chords::added(void)
{
	stato = FADE_IN;
	alive = true;
}


// UPDATES /////////////////////////////////////////////////////
void Fid_Chords::update_interrupt(ofVec2f *_fid_pos, ofVec2f *_ctr_pos, float _fid_angle, float _fid_rot_vel)
{
	
	// UPDATE POSITION --------------------------------------------------------------
	fid_pos.set(_fid_pos->x, _fid_pos->y);
	fid_angle = _fid_angle;
	
}

void Fid_Chords::update_continuos(int playHeadPos_) {
	
	// STATE UPDATE -----------------------------------------------
	if (stato == FADE_IN) 
	{
		transparency += STEP_IN;
		if(transparency > 255 )
		{ 
			transparency = 255;
			stato = STABLE;
		} 
	} 
	else if (stato == FADE_OUT) 
	{
		transparency -= STEP_OUT;
		if (transparency < 0)
		{
			stato = STABLE;
			transparency = 0;
			alive = false;
		} 
	} 	
	
	c_color.set(cr, cg, cb, transparency);
	
	// PATH C_FILL ------------------------------------------------
	c_fill.clear();
	c_fill.setFillColor(c_color);
	c_fill.setFilled(true);
	c_fill.setArcResolution(CORONA_RES);
	c_fill.setMode(c_fill.POLYLINES); 
	c_fill.arc(0, 0, c_outerRadius, c_outerRadius, 0, CORONA_CHORDS_A); 
	c_fill.arcNegative(0, 0, c_innerRadius, c_innerRadius, CORONA_CHORDS_A, 0);
	c_fill.simplify(0.3);
	c_fill.close();
	
	// PATH C_BORDO -----------------------------------------------
	c_bordo.clear();
	c_bordo.setStrokeColor(c_color);
	c_bordo.setStrokeWidth(0.7f);
	c_bordo.setFilled(false);
	c_bordo.setArcResolution(CORONA_RES);
	c_bordo.setMode(c_bordo.POLYLINES); 
	c_bordo.arc(0, 0, c_outerRadius, c_outerRadius, 0, CORONA_CHORDS_A); 
	c_bordo.arcNegative(0, 0, c_innerRadius, c_innerRadius, CORONA_CHORDS_A, 0);
	c_bordo.simplify(0.3);
	c_bordo.close();
	
	c_fill.rotate(ofGetFrameNum(), z_axis);
	c_bordo.rotate(ofGetFrameNum(), z_axis);
	c_fill.translate(fid_pos);
	c_bordo.translate(fid_pos);
	
}



// REMOVED /////////////////////////////////////////////////////
void Fid_Chords::removed(void)
{
	stato = FADE_OUT;
}


// DRAW ////////////////////////////////////////////////////////
void Fid_Chords::draw(void)
{
	ofPushStyle();
	ofEnableAlphaBlending();
	
	if(transparency != 0) 
	{
		// FIDUCIAL: disegno della corona ---------------------------
		c_fill.draw(); //ha il colore già impostato dall'update
		ofSetColor(c_color);
		c_bordo.getOutline().begin()->draw();
	}
	
	ofDisableAlphaBlending();
	ofPopStyle();
}


// GETTERS /////////////////////////////////////////////////////
ofVec2f* Fid_Chords::getFidPos() 
{
	return &fid_pos;
}

bool Fid_Chords::isAlive() 
{
	return alive;
}

float Fid_Chords::get_lim_angle() 
{
	return fid_limited_angle;
}


// OTHER ///////////////////////////////////////////////////////
void Fid_Chords::inside(ofVec2f *p)
{
	if( c_bordo.getOutline().begin()->inside(*p) ) 
	{
		c_color.set(0, 255, 0, transparency);
	} 
	else 
	{
		c_color.set(255, 0, 0, transparency);
	}
	
}



void Fid_Chords::debug() {
	char * nome_accordo;
	
	// TESTO a schermo ------------------------------------------
	ofPushMatrix();
	ofPushStyle();
	ofTranslate(fid_pos);
	
	ofSetHexColor(0x000000);
	
	// RIQUADRO 1 -----------------------------------------------
	ofPushMatrix(); 
	ofTranslate(-50, -140 , 0);
	switch(fid)
	{
		case 39:
				nome_accordo = "Am";
				break;
		case 47:
				nome_accordo = "G";
				break;
		case 60:
				nome_accordo = "C";
				break;
		case 63:
				nome_accordo = "Em";
				break;
		case 79:
				nome_accordo = "F";
				break;
		case 84:
				nome_accordo = "D";
				break;
		default:
				nome_accordo = "Am";
				break;
	}
	ofDrawBitmapString("FIDUCIAL CHORDS (" + ofToString(nome_accordo) + ")\nFid_Pos X: " + ofToString(fid_pos.x) + "\nFid_Pos Y: " + ofToString(fid_pos.y) + "\nFid_Angle: " + ofToString(fid_angle), 0, 0);
	ofPopMatrix();

	// RIQUADRO 4 -----------------------------------------------
	ofPushMatrix();
	ofTranslate(-50, 120 , 0);
	
	ofDrawBitmapString("f-id: " + ofToString((int)fid) + ";\t s-id: " + ofToString((int)sid), 0, 0);
	
	if (alive)
		ofDrawBitmapString("alive!\n", 0, 13);
	
	switch (stato){
		case STABLE:
			ofDrawBitmapString("state: STABLE\n", 0, 26);
			break;
		case FADE_IN:
			ofDrawBitmapString("state: FADE IN\n", 0, 26);
			break;
		case FADE_OUT:
			ofDrawBitmapString("state: FADE OUT\n", 0, 26);
			break;
		default:
			ofDrawBitmapString("NOTHING\n", 0, 26);
	}
	
	ofDrawBitmapString("transparency: " + ofToString((int)transparency), 0, 39); 
	
	ofPopMatrix();
	ofPopStyle();
	ofPopMatrix();
}







