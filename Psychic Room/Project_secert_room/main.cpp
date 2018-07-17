#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;
#include <GL/glut.h>
#include "SOIL.h"
#include "fmod.h"
#include "fmod_errors.h"
#define PI 3.14159265
#define PARTICLE_NUMBER 25

typedef struct VEC3 {
	GLfloat x, y, z;
}VEC3;

typedef struct VEC2 {
	GLfloat x, y;
}VEC2;

typedef struct BOUNDINGBOX {
	GLfloat xmin;
	GLfloat xmax;
	GLfloat ymin;
	GLfloat ymax;
	GLfloat zmin;
	GLfloat zmax;
}BOUNDINGBOX;

typedef struct OBJECT {
	//vertice
	vector<VEC3> v;
	vector<VEC3> vn;
	vector<VEC3> vt;
	//vertice index
	vector<GLushort> ev;
	vector<GLushort> en;
	vector<GLushort> et;
	BOUNDINGBOX bb;
	VEC3 pos;
	bool changed;
}OBJECT;

typedef struct ZOMBIE {
	OBJECT part[9];
	bool alive;
	bool chasing;
	float healthPoint;
	int groaning;
	VEC3 pos;
	GLfloat angle;
}ZOMBIE;

typedef struct PARTICLE {
	GLfloat lifetime;
	VEC3 pos;                 
	VEC3 speed;
	boolean active;
}PARTICLE;

//window
GLsizei ww = 1280, wh = 720;

//keyboard buffer
bool keyState[256];

//matrix
GLint viewport[4];
GLdouble modelview[16];
GLdouble projection[16];
GLdouble modelview2d[16];
GLdouble projection2d[16];

//camera
VEC3 position = {0, 1.3, 0};
VEC3 lookat = {1, 0, 0};
VEC2 angle = {0, 0};

//effect 
bool gameinitial = true;
bool gameover = false;
bool TheEnd = false;
bool pause = true;
bool gun_fire = false;
int gun_fire_lifetime = 0;
bool damage = false;
bool alma_appear = false;
float recoil = 0;
bool turn = false;
bool bloodspilt = false;
int particle_color = 31;
int HP_color = 12;
bool start_flash = false;
bool exit_flash = false;
bool exit_over_flash = false;
bool GoBack_flash = false;
bool paralyze = false;
bool start_timer = false;
int last_atk;             // zombie
int last_t = 0;           // alma
bool stop = false;        // alma stops moving

//objects arguments
int playerHP = 100;
bool backpack[4];
bool event_over = false;
bool code1_event = false; 
bool code2_event = false;
bool code3_event = false;
bool code4_event = false;
bool code_correct = false; 
char code1[2] = "0";
char code2[2] = "0";
char code3[2] = "0";
char code4[2] = "0";
const string code = "4172";

//static objects
OBJECT map[55];
OBJECT box;
OBJECT door2;
OBJECT codebox;
OBJECT bottom;
OBJECT ceiling;
OBJECT pistol;
OBJECT alma[5];

//moving object
short L_CALF = 0, R_CALF = 1, R_HAND = 2, L_HAND = 3, HEAD = 4, BODY = 5, HAIR = 6, R_THIGH = 8, L_THIGH = 7;  //zombie
short R_HAND2 = 1, L_HAND2 = 2, HEAD2 = 4, BODY2 = 0, HAIR2 = 3;   //alma
ZOMBIE zombie[4];
BOUNDINGBOX zombie_origin_bb[9];

OBJECT item[4];
BOUNDINGBOX item_origin_bb[4];

//animation
VEC3 r_pivot, l_pivot;
VEC3 r_pivot_alma, l_pivot_alma;
GLfloat leg_angle = 0.0;
GLfloat moving = 0;
GLint anglex= 0;

//texture
GLuint texture[37];
GLuint textureAlma[3];

//particle
PARTICLE particle[PARTICLE_NUMBER];
VEC3 hitpoint;

//sound
FMOD_SYSTEM *fmodSystem;
FMOD_SOUND *sound, *sound1, *Background, *groan, *hurt, *scream, *requiem;
FMOD_CHANNEL *BGMchannel = 0;
FMOD_RESULT result;

void ERRCHECK(FMOD_RESULT result) {
	if(result != FMOD_OK) {
		printf("FMOD error! (%d) %s\n", result, FMOD_ErrorString(result));
		exit(-1);
	}
}

void GameInit(){

	 position.x = 0;
	 position.y = 1.3;
	 position.z = 0;
	 
	 lookat.x = 1;
	 lookat.y = 0;
	 lookat.z = 0;

	 // zombie
	 angle.x = 0;
	 angle.y = 0;
	 leg_angle = 0.0;

	 // Alma
	 moving = 0;
	 anglex= 0;
	 
	 alma_appear = false;
     gameinitial = true;
	 gameover = false;
	 TheEnd = false;
	 pause = true;
	 gun_fire = false;
	 gun_fire_lifetime = 0;
	 damage = false;
	 recoil = 0;
	 turn = false;
	 bloodspilt = false;
	 particle_color = 31;
	 HP_color = 12;
	 paralyze = false;
	 start_timer = false; 
	 last_t = 0;           // alma
	 stop = false;        // alma stops moving

	//objects arguments
	 playerHP = 100;
	 event_over = false;
     code1_event = false; 
	 code2_event = false;
	 code3_event = false;
	 code4_event = false;
	 code_correct = false; 
	 door2.changed = false;
	 codebox.changed = false;
	
	 code1[0] = 48;
	 code2[0] = 48;
	 code3[0] = 48;
	 code4[0] = 48;

	for(int i = 0; i < 4; ++i) {
		item[i].changed = false;
		backpack[i] = false;
	}

	for(int i = 0; i < 4; ++i) {
		for(int j = 0; j < 9; ++j) {
			zombie[i].part[j] = zombie[0].part[j];
			zombie[i].alive = true;
			zombie[i].healthPoint = 100.0;
			zombie[i].chasing = false;
			zombie[i].groaning = 0;
		}
	}

		zombie[0].pos.x = 6.0;
		zombie[0].pos.y = 0;
		zombie[0].pos.z = 15.0;

		zombie[1].pos.x = 6.0;
		zombie[1].pos.y = 0;
		zombie[1].pos.z = -1.7;

		zombie[2].pos.x = 39.0;
		zombie[2].pos.y = 0;
		zombie[2].pos.z = 3.0;

		zombie[3].pos.x = 10.4;
		zombie[3].pos.y = 0;
		zombie[3].pos.z = -10.0;
}

void SoundSystemInit() {

	//FMOD init
	result = FMOD_System_Create(&fmodSystem);
	ERRCHECK(result);

	result = FMOD_System_Init(fmodSystem, 32, FMOD_INIT_NORMAL, NULL);
	ERRCHECK(result);

	//load sound
	result = FMOD_System_CreateSound(fmodSystem, "deagle-1.wav", FMOD_HARDWARE, 0, &sound);
	ERRCHECK(result);

	result = FMOD_System_CreateSound(fmodSystem, "gunpickup2.wav", FMOD_HARDWARE, 0, &sound1);
	ERRCHECK(result);

	result = FMOD_System_CreateSound(fmodSystem, "background_sound.wav", FMOD_LOOP_NORMAL| FMOD_2D | FMOD_CREATECOMPRESSEDSAMPLE, 0, &Background);
	ERRCHECK(result);
	
	result = FMOD_System_CreateSound(fmodSystem, "groan.wav", FMOD_HARDWARE, 0, &groan);
	ERRCHECK(result);

	result = FMOD_System_CreateSound(fmodSystem, "hurt.wav", FMOD_HARDWARE, 0, &hurt);
	ERRCHECK(result);

	result = FMOD_System_CreateSound(fmodSystem, "scream.wav", FMOD_HARDWARE, 0, &scream);
	ERRCHECK(result);

	result = FMOD_System_CreateSound(fmodSystem, "requiem.wav", FMOD_HARDWARE|FMOD_LOOP_NORMAL, 0, &requiem);
	ERRCHECK(result);

	FMOD_Channel_SetVolume(BGMchannel, 0.1);  

}

void SoundSystemClose() {
	//FMOD close
	result = FMOD_Sound_Release(sound);
	ERRCHECK(result);

	result = FMOD_Sound_Release(sound1);
	ERRCHECK(result);

	result = FMOD_Sound_Release(scream);
	ERRCHECK(result);

	result = FMOD_Sound_Release(Background);
	ERRCHECK(result);

    result = FMOD_Sound_Release(groan);
	ERRCHECK(result);

	result = FMOD_Sound_Release(hurt);
	ERRCHECK(result);

	result = FMOD_Sound_Release(requiem);
	ERRCHECK(result);

    result = FMOD_System_Close(fmodSystem);
    ERRCHECK(result);

    result = FMOD_System_Release(fmodSystem);
    ERRCHECK(result);
}

void load_texture() {
	glEnable(GL_TEXTURE_2D);

	texture[0] = SOIL_load_OGL_texture
	(
	"wall9.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[0] == 0) {
		exit(1);
	}

	texture[1] = SOIL_load_OGL_texture
	(
	"wood2.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[1] == 0) {
		exit(1);
	}

	texture[2] = SOIL_load_OGL_texture
	(
	"wall.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[2] == 0) {
		exit(1);
	}

   
	texture[3] = SOIL_load_OGL_texture
	(
	"metal3.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[3] == 0) {
		exit(1);
	}

	texture[4] = SOIL_load_OGL_texture
	(
	"Door3.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[4] == 0) {
		exit(1);
	}

   texture[5] = SOIL_load_OGL_texture
	(
	"background2.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[5] == 0) {
		exit(1);
	}

   texture[6] = SOIL_load_OGL_texture
	(
	"floor4_2.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[6] == 0) {
		exit(1);
	}

	texture[7] = SOIL_load_OGL_texture
	(
	"ceiling7.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[7] == 0) {
		exit(1);
	}

	texture[8] = SOIL_load_OGL_texture
	(
	"zombie_hair.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[8] == 0) {
		exit(1);
	}

	texture[9] = SOIL_load_OGL_texture
	(
	"zombie_cloth.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[9] == 0) {
		exit(1);
	}
 
	texture[10] = SOIL_load_OGL_texture
	(
	"zombie_skin.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[10] == 0) {
		exit(1);
	}

	texture[11] = SOIL_load_OGL_texture
	(
	"fireball3.png",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[11] == 0) {
		exit(1);
	}

	texture[12] = SOIL_load_OGL_texture
	(
	"HP.png",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[12] == 0) {
		exit(1);
	}

	texture[13] = SOIL_load_OGL_texture
	(
		"4.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[13] == 0) {
		exit(1);
	}

	texture[14] = SOIL_load_OGL_texture
	(
		"1.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
   if(texture[14] == 0) {
		exit(1);
	}

	texture[15] = SOIL_load_OGL_texture
	(
		"7.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(texture[15] == 0) {
		exit(1);
	}

	texture[16] = SOIL_load_OGL_texture
	(
		"zombie_hair.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	if(texture[16] == 0) {
		exit(1);
	}
	
	texture[18] = SOIL_load_OGL_texture
	(
		"GameOver2.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[18] == 0) {
		exit(1);
	}
	
	texture[19] = SOIL_load_OGL_texture
	(
		"2.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[19] == 0) {
		exit(1);
	}

	texture[20] = SOIL_load_OGL_texture
	(
		"button.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[20] == 0) {
		exit(1);
	}

	 texture[21] = SOIL_load_OGL_texture
	(
		"buttonOK.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[21] == 0) {
		exit(1);
	}

	  texture[22] = SOIL_load_OGL_texture
	(
		"metal.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[22] == 0) {
		exit(1);
	}

	  texture[23] = SOIL_load_OGL_texture
	(
		"Blood.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[23] == 0) {
		exit(1);
	}

	texture[24] = SOIL_load_OGL_texture
	(
		"aim_vertical.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[24] == 0) {
		exit(1);
	}

	texture[25] = SOIL_load_OGL_texture
	(
		"aim_horizon.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[25] == 0) {
		exit(1);
	}

	texture[26] = SOIL_load_OGL_texture
	(
		"MainMenu.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[26] == 0) {
		exit(1);
	}

	texture[27] = SOIL_load_OGL_texture
	(
		"start_ico.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[27] == 0) {
		exit(1);
	}

		texture[28] = SOIL_load_OGL_texture
	(
		"exit_ico.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[28] == 0) {
		exit(1);
	}

	texture[29] = SOIL_load_OGL_texture
	(
		"exit_over_ico.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[29] == 0) {
		exit(1);
	}

	texture[30] = SOIL_load_OGL_texture
	(
		"go_back_ico.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[30] == 0) {
		exit(1);
	}

	texture[31] = SOIL_load_OGL_texture
	(
		"fragment.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[31] == 0) {
		exit(1);
	}

	texture[32] = SOIL_load_OGL_texture
	(
		"TheEnd.jpg",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[32] == 0) {
		exit(1);
	}

		texture[33] = SOIL_load_OGL_texture
	(
		"exit_over_end_ico.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[33] == 0) {
		exit(1);
	}

	texture[34] = SOIL_load_OGL_texture
	(
		"go_back_end_ico.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[34] == 0) {
		exit(1);
	}

	texture[35] = SOIL_load_OGL_texture
	(
		"HP_frame.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[35] == 0) {
		exit(1);
	}

	texture[36] = SOIL_load_OGL_texture
	(
		"HP_red.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	); 
	if(texture[36] == 0) {
		exit(1);
	}

	textureAlma[0] = SOIL_load_OGL_texture
	(
	"Alma_body.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

    if(textureAlma[0] == 0) {
        exit(1);
	}

	 textureAlma[1] = SOIL_load_OGL_texture
	(
	"Alma_hair.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

    if(textureAlma[1] == 0) {
        exit(1);
	}

	 textureAlma[2] = SOIL_load_OGL_texture
	(
	"Alma_head.jpg",
	SOIL_LOAD_AUTO,
	SOIL_CREATE_NEW_ID,
	SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);

    if(textureAlma[2] == 0) {
        exit(1);
	}


}

void load_obj(const char *filename, OBJECT *obj) {
	ifstream file;
	file.open(filename, ios::in | ios::binary);
	if(!file) {
		cerr << "Cannot open " << filename << endl; 
		exit(1);
	}

	int index = 0, adjust_ev = 1, adjust_et = 1, adjust_en = 1;
	string line;
	while(getline(file, line)) {
		if(line.substr(0,2) == "v ") {
			istringstream strm(line.substr(2));
			VEC3 tmp_vec;
			strm >> tmp_vec.x; strm >> tmp_vec.y; strm >> tmp_vec.z;
			obj[index].v.push_back(tmp_vec);
			if(obj[index].v.size() == 1) {
				obj[index].bb.xmin = tmp_vec.x; obj[index].bb.xmax = tmp_vec.x;
				obj[index].bb.ymin = tmp_vec.y; obj[index].bb.ymax = tmp_vec.y;
				obj[index].bb.zmin = tmp_vec.z; obj[index].bb.zmax = tmp_vec.z;
			}
			if(obj[index].bb.xmin > tmp_vec.x)
				obj[index].bb.xmin = tmp_vec.x;
			if(obj[index].bb.xmax < tmp_vec.x)
				obj[index].bb.xmax = tmp_vec.x;
			if(obj[index].bb.ymin > tmp_vec.y)
				obj[index].bb.ymin = tmp_vec.y;
			if(obj[index].bb.ymax < tmp_vec.y)
				obj[index].bb.ymax = tmp_vec.y;
			if(obj[index].bb.zmin > tmp_vec.z)
				obj[index].bb.zmin = tmp_vec.z;
			if(obj[index].bb.zmax < tmp_vec.z)
				obj[index].bb.zmax = tmp_vec.z;
		}
		else if(line.substr(0,2) == "vn") {
			istringstream strm(line.substr(3));
			VEC3 tmp_vec;
			strm >> tmp_vec.x; strm >> tmp_vec.y; strm >> tmp_vec.z;
			obj[index].vn.push_back(tmp_vec);
		}
		else if(line.substr(0,2) == "vt") {
			istringstream strm(line.substr(3));
			VEC3 tmp_vec;
			strm >> tmp_vec.x; strm >> tmp_vec.y; strm >> tmp_vec.z;
			obj[index].vt.push_back(tmp_vec);
		}
		else if(line.substr(0,1) == "f") {
			istringstream strm(line.substr(2));
			GLushort v0, v1, v2;
			GLushort t0, t1, t2;
			GLushort n0, n1, n2;
			char slash;
			strm >> v0; strm >> slash; strm >> t0; strm >> slash; strm >> n0;
			strm >> v1; strm >> slash; strm >> t1; strm >> slash; strm >> n1;
			strm >> v2; strm >> slash; strm >> t2; strm >> slash; strm >> n2; 
			v0 -= adjust_ev; v1 -= adjust_ev; v2 -= adjust_ev;
			t0 -= adjust_et; t1 -= adjust_et; t2 -= adjust_et;
			n0 -= adjust_en; n1 -= adjust_en; n2 -= adjust_en;
			obj[index].ev.push_back(v0); obj[index].ev.push_back(v1); obj[index].ev.push_back(v2);
			obj[index].et.push_back(t0); obj[index].et.push_back(t1); obj[index].et.push_back(t2);
			obj[index].en.push_back(n0); obj[index].en.push_back(n1); obj[index].en.push_back(n2);
		}
		else if(line.substr(0,8) == "# object" && obj[0].v.size() != 0) {
			adjust_ev += obj[index].v.size();
			adjust_et += obj[index].vt.size();
			adjust_en += obj[index].vn.size();
			index++;
		}
	}
}

void computeBB(BOUNDINGBOX* current, BOUNDINGBOX origin, double *transform) {
	GLdouble w;
	VEC3 tmp_vec[8];
	VEC3 min, max;
	*current = origin;

	//8 vertex of bounding box
	tmp_vec[0].x = current->xmin; tmp_vec[0].y = current->ymin; tmp_vec[0].z = current->zmin;
	tmp_vec[1].x = current->xmin; tmp_vec[1].y = current->ymax; tmp_vec[1].z = current->zmin;
	tmp_vec[2].x = current->xmin; tmp_vec[2].y = current->ymin; tmp_vec[2].z = current->zmax;
	tmp_vec[3].x = current->xmin; tmp_vec[3].y = current->ymax; tmp_vec[3].z = current->zmax;
	tmp_vec[4].x = current->xmax; tmp_vec[4].y = current->ymin; tmp_vec[4].z = current->zmin;
	tmp_vec[5].x = current->xmax; tmp_vec[5].y = current->ymax; tmp_vec[5].z = current->zmin;
	tmp_vec[6].x = current->xmax; tmp_vec[6].y = current->ymin; tmp_vec[6].z = current->zmax;
	tmp_vec[7].x = current->xmax; tmp_vec[7].y = current->ymax; tmp_vec[7].z = current->zmax;

	//transfrom bounding box
	w = transform[3] * tmp_vec[0].x + transform[7] * tmp_vec[0].y + transform[11] * tmp_vec[0].z + transform[15] * 1.0;
	min.x = (transform[0] * tmp_vec[0].x + transform[4] * tmp_vec[0].y + transform[8] * tmp_vec[0].z + transform[12] * 1.0)/w;
	min.y = (transform[1] * tmp_vec[0].x + transform[5] * tmp_vec[0].y + transform[9] * tmp_vec[0].z + transform[13] * 1.0)/w;
	min.z = (transform[2] * tmp_vec[0].x + transform[6] * tmp_vec[0].y + transform[10] * tmp_vec[0].z + transform[14] * 1.0)/w;

	max.x = min.x; max.y = min.y; max.z = min.z;

	for(int i = 1; i < 8; ++i) {
		VEC3 tmp;
		w = transform[3] * tmp_vec[i].x + transform[7] * tmp_vec[i].y + transform[11] * tmp_vec[i].z + transform[15] * 1.0;
		tmp.x = (transform[0] * tmp_vec[i].x + transform[4] * tmp_vec[i].y + transform[8] * tmp_vec[i].z + transform[12] * 1.0)/w;
		tmp.y = (transform[1] * tmp_vec[i].x + transform[5] * tmp_vec[i].y + transform[9] * tmp_vec[i].z + transform[13] * 1.0)/w;
		tmp.z = (transform[2] * tmp_vec[i].x + transform[6] * tmp_vec[i].y + transform[10] * tmp_vec[i].z + transform[14] * 1.0)/w;
		if(min.x > tmp.x)
			min.x = tmp.x;
		if(max.x < tmp.x)
			max.x = tmp.x;
		if(min.y > tmp.y)
			min.y = tmp.y;
		if(max.y < tmp.y)
			max.y = tmp.y;
		if(min.z > tmp.z)
			min.z = tmp.z;
		if(max.z < tmp.z)
			max.z = tmp.z;
	}
	
	current->xmin = min.x; current->ymin = min.y; current->zmin = min.z; 
	current->xmax = max.x; current->ymax = max.y; current->zmax = max.z; 
}

bool isInBoundingBox(float x, float y, float z, BOUNDINGBOX bb) {
	if(x < bb.xmax && x > bb.xmin && y < bb.ymax && y > bb.ymin && z < bb.zmax && z > bb.zmin) {
		return true;
	}
	return false;
}

void ProcessPick(double x, double y, double z) {
	for(int i = 0; i < 4; ++i) {
		for(int j = 0; j < 9; ++j) {
			if(isInBoundingBox(x, y, z, zombie[i].part[j].bb)) {
				zombie[i].part[j].changed = true;
                bloodspilt = true;
			}
		}
	}
	glutPostRedisplay();
}

void ProcessPickR(double x, double y, double z) {
	//scale x, y, z to make sure if player is near code box 
	if(isInBoundingBox(x - lookat.x/10, y - lookat.y/10, z - lookat.z/10, codebox.bb)) {
		codebox.changed = true;
		pause = true;
	}
	glutPostRedisplay();
}

void ProcessPick2D(double x, double y) {

	if(x > -0.95 && x < -0.55 && y < 0.25 && y > -0.25)
		code1_event= true;
	if(x > -0.45 && x < -0.05 && y < 0.25 && y > -0.25)
		code2_event= true;
	if(x > 0.05 && x < 0.45 && y < 0.25 && y > -0.25)
		code3_event= true;
	if(x > 0.55 && x < 0.95 && y < 0.25 && y > -0.25)
		code4_event= true;
	if(x > 1.1 && x < 1.3 && y < 0.1 && y > -0.1 && code_correct) {
		event_over = true;
		pause = false;
	if(code_correct)
		door2.changed = true ; // door disappear	
	}

	// buttons in select menu
	if(gameinitial){      
		if(x > 2.3 && x < 3.4 && y < -1.40 + 0.4  && y > -1.80 + 0.4 ){  //start
			pause = false;
			gameinitial = false;
		//	result = FMOD_System_PlaySound(fmodSystem, FMOD_CHANNEL_FREE, Background, 0, &BGMchannel);
		//	ERRCHECK(result);
		}
		else if (x > 2.3 && x < 3.4 && y < -1.45 && y > -1.85 )  //exit
			exit(0);
	}

	// buttons in gameover menu
	if(gameover || TheEnd){        
	    if(x > -(0.7 + 0.25) && x < 0.7 + 0.25 && y < -1.0 + 0.5 && y > -1.4 + 0.5 ){
			GameInit();     // Initialization
		}
		else if(  x > -0.55 && x < 0.65 && y < -1.05 && y > -1.5 ){
			exit(0);           
		}
	}

	glutPostRedisplay();
}

void CreateParticle() {
	
	if(bloodspilt)
		particle_color = 23;
	else
		particle_color = 31;

	for(int i = 0; i < PARTICLE_NUMBER; ++i) {
		particle[i].lifetime = 0.4;
		particle[i].pos.x = hitpoint.x;
		particle[i].pos.y = hitpoint.y;
		particle[i].pos.z = hitpoint.z;
		particle[i].speed.x = (float)((rand()%20)/100.0 - 0.1) * 50;
		particle[i].speed.y = (float)((rand()%20)/100.0 - 0.1) * 50;
		particle[i].speed.z = (float)((rand()%20)/100.0 - 0.1) * 50;
		particle[i].active = true;
	}

}

void EvolveParticle(float dt) {
	for(int i = 0; i < PARTICLE_NUMBER; ++i) {
		if(particle[i].active) {
			particle[i].lifetime -= dt;
			particle[i].pos.x += particle[i].speed.x * dt;
			particle[i].pos.y += particle[i].speed.y * dt;
			particle[i].pos.z += particle[i].speed.z * dt;
			particle[i].speed.y -= 0.1 * dt;
			if(particle[i].lifetime < 0.0) {
				particle[i].active = false;
				bloodspilt = false;
			}
		}
	}
}

void ButtonFlash(double x, double y){
	
	if( gameinitial ){
		if(x > 2.3 && x < 3.4 && y < -1.40 + 0.4  && y > -1.80 + 0.4 ){
			start_flash = true;
			exit_flash = false;
		}
		else if(x > 2.3 && x < 3.4 && y < -1.45 && y > -1.85 ){
			exit_flash = true;
			start_flash = false;
		}
		else{
			exit_flash = false;
			start_flash = false;
		}
	}
	if( gameover || TheEnd ){
		if(  x > -0.55 && x < 0.65 && y < -1.05 && y > -1.5 ){
			exit_over_flash = true;
			GoBack_flash = false;
		}
		else if(x > -(0.7 + 0.25) && x < 0.7 + 0.25 && y < -1.0 + 0.5 && y > -1.4 + 0.5 ){
			exit_over_flash = false;
			GoBack_flash = true;
		}
		else{
		    exit_over_flash = false;
			GoBack_flash = false;
		}
   }

	glutPostRedisplay();
}

void mouse(int button, int state, int x, int y) {
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	glGetIntegerv(GL_VIEWPORT, viewport);
	winX = (float)x;
	winY = (float)viewport[3] - (float)y;
	
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if(pause || paralyze) {
			glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
			gluUnProject(winX, winY, winZ, modelview2d, projection2d, viewport, &posX, &posY, &posZ);
			ProcessPick2D(posX, posY);
		}
		else {
			glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
			gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
			ProcessPick(posX+lookat.x/10, posY+lookat.y/10, posZ+lookat.z/10); 
			
			recoil = 1;

			gun_fire = true;
			gun_fire_lifetime = 5;

			hitpoint.x = posX;
			hitpoint.y = posY;
			hitpoint.z = posZ;
		}    
	}
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN) {
		glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
		gluUnProject(winX, winY, winZ, modelview, projection, viewport, &posX, &posY, &posZ);
		ProcessPickR(posX+lookat.x/10, posY+lookat.y/10, posZ+lookat.z/10); 
	}
}

bool collision() {
	//bounding sphere for player
	const float r = 0.2;
	float c = r * cos(PI/4);
	//wall collision
	for(int i = 0; i < 55; ++i) {
		if(isInBoundingBox(position.x+r, position.y, position.z, map[i].bb))
			return true;
		else if(isInBoundingBox(position.x-r, position.y, position.z, map[i].bb))
			return true;
		else if(isInBoundingBox(position.x, position.y, position.z+r, map[i].bb))
			return true;
		else if(isInBoundingBox(position.x, position.y, position.z-r, map[i].bb))
			return true;
		else if(isInBoundingBox(position.x+c, position.y, position.z+c, map[i].bb))
			return true;
		else if(isInBoundingBox(position.x-c, position.y, position.z+c, map[i].bb))
			return true;
		else if(isInBoundingBox(position.x-c, position.y, position.z-c, map[i].bb))
			return true;
		else if(isInBoundingBox(position.x+c, position.y, position.z-c, map[i].bb))
			return true;
	}
	//door collision
	if(!code_correct) {
		if(isInBoundingBox(position.x+r, position.y, position.z, door2.bb))
			return true;
		else if(isInBoundingBox(position.x-r, position.y, position.z, door2.bb))
			return true;
		else if(isInBoundingBox(position.x, position.y, position.z+r, door2.bb))
			return true;
		else if(isInBoundingBox(position.x, position.y, position.z-r, door2.bb))
			return true;
		else if(isInBoundingBox(position.x+c, position.y, position.z+c, door2.bb))
			return true;
		else if(isInBoundingBox(position.x-c, position.y, position.z+c, door2.bb))
			return true;
		else if(isInBoundingBox(position.x-c, position.y, position.z-c, door2.bb))
			return true;
		else if(isInBoundingBox(position.x+c, position.y, position.z-c, door2.bb))
			return true;
	}
	//touch item
	for(int i = 0; i < 4; ++i) {
		//item changed: drop to floor
		if(item[i].changed) {
			if(position.x >= item[i].bb.xmin && position.x <= item[i].bb.xmax &&position.z >= item[i].bb.zmin && position.z <= item[i].bb.zmax) {
				item[i].changed = false;
				backpack[i] = true;
				result = FMOD_System_PlaySound(fmodSystem, FMOD_CHANNEL_FREE, sound1, 0, &BGMchannel);
			}
		}
	}

	return false;
}

void mouseMotion(int x, int y) {
	const float mousesensitive = 0.001;
	static bool wrap = false;
	
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	glGetIntegerv(GL_VIEWPORT, viewport);
	winX = (float)x;
	winY = (float)viewport[3] - (float)y;

	if(pause) {         
		glutSetCursor(GLUT_CURSOR_LEFT_ARROW);
	
		glReadPixels(x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
		gluUnProject(winX, winY, winZ, modelview2d, projection2d, viewport, &posX, &posY, &posZ);
        ButtonFlash(posX, posY);        
	}
	else {
		glutSetCursor(GLUT_CURSOR_NONE);
		float dx = x - ww/2;
		float dy = y - wh/2;
		if(!wrap) {
			angle.x -= dx * mousesensitive;
			angle.y -= dy * mousesensitive;

			if(angle.x < -PI) {
				angle.x += PI * 2;
			}
			else if(angle.x > PI) {
				angle.x -= PI * 2;
			}
			if(angle.y < -PI / 3  ) {
				angle.y = -PI / 3 ;
			}
			else if(angle.y > PI / 3 ) {    
				angle.y = PI / 3;
			}

			lookat.x = sin(angle.x) * cos(angle.y);
			lookat.y = sin(angle.y);
			lookat.z = cos(angle.x) * cos(angle.y);

			wrap = true;
			glutWarpPointer(ww/2, wh/2);	
		}
		else {
			wrap = false;
		}
		glutPostRedisplay();
	}
}

bool m_forward, m_backward, m_left, m_right;

void keyOperation() {

	//moving forward
	if(keyState['w'] || keyState['W'])
		m_forward = true;
	else
		m_forward = false;
	//moving right
	if(keyState['d'] || keyState['D'])
		m_right = true;
	else
		m_right = false;
	//moving left
	if(keyState['a'] || keyState['A'])
		m_left = true;
	else
		m_left = false;
	//moving backward
	if(keyState['s'] || keyState['S'])
		m_backward = true;
	else
		m_backward = false;

	//pause
	if(!gameinitial){
		if( !gameover || !TheEnd ){
			if(keyState['p'] || keyState['P']) {
				pause = !pause;
				codebox.changed =false;
			}
		}
	}
	//exit
	if(keyState[27])
		exit(0);

	glutPostRedisplay();
}

void keyDown(unsigned char key, int x, int y) {
	keyState[key] = true;
	keyOperation();
}

void keyUp(unsigned char key, int x, int y) {
	keyState[key] = false;
	keyOperation();
}

void idle() {
   static int pt = 0;
   const float movespeed = 5;
   const float zombie_movespeed = 0.5;
   int t = glutGet(GLUT_ELAPSED_TIME);
   float dt = (t - pt) * 0.001;
	pt = t;

	//init timer
	if(!start_timer) {
		dt = 0;
		start_timer = !start_timer;
	}

	//moving
	float d_pos_x = sin(angle.x);
	float d_pos_z = cos(angle.x);

	 if(!pause) {
	 
   	 if(!paralyze){
		if(m_left) {
			position.x -= -d_pos_z * movespeed * dt;
			position.z -= d_pos_x * movespeed * dt;
			if(collision()){
				position.x += -d_pos_z * movespeed * dt;
				position.z += d_pos_x * movespeed * dt;
			}
		}
		if(m_right) {
			position.x += -d_pos_z * movespeed * dt;
			position.z += d_pos_x * movespeed * dt;
			if(collision()){
				position.x -= -d_pos_z * movespeed * dt;
				position.z -= d_pos_x * movespeed * dt;
			}
		}
		if(m_forward) {
			position.x += d_pos_x * movespeed * dt;
			position.z += d_pos_z * movespeed * dt;
			if(collision()){
				position.x -= d_pos_x * movespeed * dt;
				position.z -= d_pos_z * movespeed * dt;
			}
		}
		if(m_backward) {
			position.x -= d_pos_x * movespeed * dt;
			position.z -= d_pos_z * movespeed * dt;
			if(collision()){
				position.x += d_pos_x * movespeed * dt;
				position.z += d_pos_z * movespeed * dt;
			}
		}
	 }
		
		//zombie's direction
		for(int i = 0; i < 4; ++i) {
			if(zombie[i].alive) {
				if(zombie[i].chasing) {
					VEC3 dir_vec;
					dir_vec.x = position.x - zombie[i].pos.x;
					dir_vec.z = position.z - zombie[i].pos.z;
				
					// animation
					if(dir_vec.x != 0 && dir_vec.z != 0) {
						if( zombie[0].chasing && zombie[1].chasing && zombie[0].alive && zombie[1].alive ){
							leg_angle = 25.0 * sin(t * 0.2);  
						}
						else{
							leg_angle = 25.0 * sin(t * 0.005);
						}
					}

					//facing player
					if(dir_vec.z > 0) {
						zombie[i].angle = acos(-dir_vec.x / sqrt(dir_vec.x * dir_vec.x + dir_vec.z * dir_vec.z)) * 180 / PI;
					}
					else {
						zombie[i].angle = acos(dir_vec.x / sqrt(dir_vec.x * dir_vec.x + dir_vec.z * dir_vec.z)) * 180 / PI;
						zombie[i].angle += 180.0;
					}

					//moving
						zombie[i].pos.x += dir_vec.x * zombie_movespeed * dt;
						zombie[i].pos.z += dir_vec.z * zombie_movespeed * dt;
				}

				if(i == 0 || i == 1){
						if(position.x < 24.18 && position.x > 5.32 && position.z < 18.4 && position.z > -1.75){
							zombie[i].chasing =true;
							if(zombie[i].groaning < 2){
								zombie[i].groaning++;
							}
						}
						else{
							zombie[i].chasing =false;
							zombie[i].groaning = 0;
						}
				}
				else if(i == 2){
						if(position.x < 44.5 && position.x > 37.0 && position.z < 11.9 && position.z > -2.3){
							zombie[i].chasing =true;
							if(zombie[i].groaning < 2){
								zombie[i].groaning++;
							}
						}
						else{
							zombie[i].chasing =false;
							zombie[i].groaning = 0;
						}
				}		
				else if(i == 3){
						if(position.x < 23.4 && position.x > 10.1 && position.z < -5.5 && position.z > -11.1){
							zombie[i].chasing =true;		
							if(zombie[i].groaning < 2){
								zombie[i].groaning++;		
							}
						}
						else{
							zombie[i].chasing =false;
							zombie[i].groaning = 0;	
						}
				}

              //zombie groan
				if(zombie[i].groaning == 1){
						result = FMOD_System_PlaySound(fmodSystem, FMOD_CHANNEL_FREE, groan, 0, &BGMchannel);
						ERRCHECK(result);
				}
			

				//attack player
				if((t - last_atk) * 0.001 > 1) {
					if(abs(zombie[i].pos.x - position.x) < 1 && abs(zombie[i].pos.z - position.z) < 1) {
						damage = true;
						playerHP -= 20;
						if(playerHP > 0) {

							if(playerHP > 40)
								HP_color = 12;
							else
								HP_color = 36;

								last_atk = t;

								result = FMOD_System_PlaySound(fmodSystem, FMOD_CHANNEL_FREE, hurt, 0, &BGMchannel);
								ERRCHECK(result);
						}
						else if(playerHP <= 0) {

							gameover = true;
							last_atk = t;
							
							result = FMOD_System_PlaySound(fmodSystem, FMOD_CHANNEL_FREE, scream, 0, &BGMchannel);
							
						//	Sleep(1000);
					  	    
							FMOD_Channel_Stop(BGMchannel);
				     	//	FMOD_Channel_SetPaused(BGMchannel, FALSE);
					    //  FMOD_Channel_IsPlaying(BGMchannel, FALSE);
						//	result = FMOD_System_Update(fmodSystem);
						//	ERRCHECK(result);

						//	result = FMOD_System_PlaySound(fmodSystem, FMOD_CHANNEL_FREE, requiem, 0, &BGMchannel);
						//	ERRCHECK(result);
							
							zombie[i].alive = false;
							pause = true;
						}
					}
					    else {
						    damage = false;
					    }
				}
		  }
		}

		//zombie injury
		for(int i = 0; i < 4; ++i) {
			for(int j = 0; j < 9; ++j) {
				if(zombie[i].part[j].changed){
					if(i == HAIR) {
						zombie[i].healthPoint -= 30;
						zombie[i].part[j].changed = false;
					}
					else {
						zombie[i].healthPoint -= 10;
						zombie[i].part[j].changed = false;
					}
				}
			}

			if(zombie[i].healthPoint <= 0) {
				zombie[i].healthPoint = 0;
				zombie[i].alive = false;
				if(!backpack[i]) {
					item[i].changed = true;
				}
			}
		}

		// recoil of pistol
		if(recoil >= 1 && !turn) {
			if(recoil >= 2)
				turn = true;
			else
				recoil += 150 * dt * 0.5;	
		}
		if(recoil >= 1 && turn) {
			recoil -= 150 * dt * 0.5;
		}
		else if(recoil <1){
			turn = false;
			recoil = 0;
		}


		if(gun_fire) {
			CreateParticle();
			result = FMOD_System_PlaySound(fmodSystem, FMOD_CHANNEL_FREE, sound, 0, &BGMchannel);
			ERRCHECK(result);
			gun_fire = false;
		}

		if(gun_fire_lifetime > 0) {
			gun_fire_lifetime -= dt;
		}

		// alma move
		if(door2.changed){
			paralyze = true;
			
		  if( (t - last_t)* 0.001 > 1 ){
			 alma_appear = true;
			 if(moving < 4 && !stop){	
			  last_t = t;
			  moving += 1 ;
			 }
		  }
		  else{
			 alma_appear = false;
		  }
	
		  if(moving >= 3){
			  anglex = 90;
		  }

		  if(moving == 4){
			 stop = true;
			 TheEnd = true;
			 pause = true;
			 result = FMOD_System_PlaySound(fmodSystem, FMOD_CHANNEL_FREE, scream, 0, &BGMchannel);
			 ERRCHECK(result);
			 moving++;
		  }

		}

		EvolveParticle(dt);
	}
	
	 glutPostRedisplay();
}

void drawHead(int id) {	
	glBindTexture(GL_TEXTURE_2D, texture[10]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_TRIANGLES);
	glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < zombie[id].part[HEAD].ev.size(); ++i) {
		VEC3 tmpv = zombie[id].part[HEAD].v[zombie[id].part[HEAD].ev[i]];
		VEC3 tmpn = zombie[id].part[HEAD].vn[zombie[id].part[HEAD].en[i]];
		VEC3 tmpt = zombie[id].part[HEAD].vt[zombie[id].part[HEAD].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[8]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_TRIANGLES);
	glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < zombie[id].part[HAIR].ev.size(); ++i) {
		VEC3 tmpv = zombie[id].part[HAIR].v[zombie[id].part[HAIR].ev[i]];
		VEC3 tmpn = zombie[id].part[HAIR].vn[zombie[id].part[HAIR].en[i]];
		VEC3 tmpt = zombie[id].part[HAIR].vt[zombie[id].part[HAIR].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	glEnd();
}

void drawBody(int id) {
	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_TRIANGLES);
	glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < zombie[id].part[BODY].ev.size(); ++i) {
		VEC3 tmpv = zombie[id].part[BODY].v[zombie[id].part[BODY].ev[i]];
		VEC3 tmpn = zombie[id].part[BODY].vn[zombie[id].part[BODY].en[i]];
		VEC3 tmpt = zombie[id].part[BODY].vt[zombie[id].part[BODY].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	glEnd();  
}

void drawLeftHand(int id) {
	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glBegin(GL_TRIANGLES);
	glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < zombie[id].part[L_HAND].ev.size(); ++i) {
		VEC3 tmpv = zombie[id].part[L_HAND].v[zombie[id].part[L_HAND].ev[i]];
		VEC3 tmpn = zombie[id].part[L_HAND].vn[zombie[id].part[L_HAND].en[i]];
		VEC3 tmpt = zombie[id].part[L_HAND].vt[zombie[id].part[L_HAND].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	glEnd();
}

void drawRightHand(int id) {
	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_TRIANGLES);
	glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < zombie[id].part[R_HAND].ev.size(); ++i) {
		VEC3 tmpv = zombie[id].part[R_HAND].v[zombie[id].part[R_HAND].ev[i]];
		VEC3 tmpn = zombie[id].part[R_HAND].vn[zombie[id].part[R_HAND].en[i]];
		VEC3 tmpt = zombie[id].part[R_HAND].vt[zombie[id].part[R_HAND].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	glEnd();
}

void drawLeftCalf(int id) {
	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
	glBegin(GL_TRIANGLES);
	glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < zombie[id].part[L_CALF].ev.size(); ++i) {
		VEC3 tmpv = zombie[id].part[L_CALF].v[zombie[id].part[L_CALF].ev[i]];
		VEC3 tmpn = zombie[id].part[L_CALF].vn[zombie[id].part[L_CALF].en[i]];
		VEC3 tmpt = zombie[id].part[L_CALF].vt[zombie[id].part[L_CALF].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	glEnd();
}

void drawRightCalf(int id) {
	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_TRIANGLES);
	glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < zombie[id].part[R_CALF].ev.size(); ++i) {
			VEC3 tmpv = zombie[id].part[R_CALF].v[zombie[id].part[R_CALF].ev[i]];
			VEC3 tmpn = zombie[id].part[R_CALF].vn[zombie[id].part[R_CALF].en[i]];
			VEC3 tmpt = zombie[id].part[R_CALF].vt[zombie[id].part[R_CALF].et[i]];
			glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	glEnd();
}

void drawLeftThigh(int id) {
	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_TRIANGLES);
	glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < zombie[id].part[L_THIGH].ev.size(); ++i) {
		VEC3 tmpv = zombie[id].part[L_THIGH].v[zombie[id].part[L_THIGH].ev[i]];
		VEC3 tmpn = zombie[id].part[L_THIGH].vn[zombie[id].part[L_THIGH].en[i]];
		VEC3 tmpt = zombie[id].part[L_THIGH].vt[zombie[id].part[L_THIGH].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	glEnd();
}

void drawRightThigh(int id) {
	glBindTexture(GL_TEXTURE_2D, texture[9]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_TRIANGLES);
	glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < zombie[id].part[R_THIGH].ev.size(); ++i) {
		VEC3 tmpv = zombie[id].part[R_THIGH].v[zombie[id].part[R_THIGH].ev[i]];
		VEC3 tmpn = zombie[id].part[R_THIGH].vn[zombie[id].part[R_THIGH].en[i]];
		VEC3 tmpt = zombie[id].part[R_THIGH].vt[zombie[id].part[R_THIGH].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	glEnd();
}

void drawMap() {
	//wall
   glBindTexture(GL_TEXTURE_2D, texture[0]);
   glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
   glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	for(unsigned int j = 0; j < 55; ++j) {
		glPushMatrix();
		glBegin(GL_TRIANGLES);
		glColor4f(0.5, 0.5, 0.5, 0.5);
		for(unsigned int i = 0; i < map[j].ev.size(); ++i) {
			VEC3 tmpv = map[j].v[map[j].ev[i]];
			VEC3 tmpn = map[j].vn[map[j].en[i]];
			VEC3 tmpt = map[j].vt[map[j].et[i]];
			glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
		}
		glEnd();
		glPopMatrix();
	}

	//bottom
	glBindTexture(GL_TEXTURE_2D, texture[6]);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	//glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_TRIANGLES);
	glColor4f(0.0, 0.0, 1.0, 0.5);
	for(unsigned int i = 0; i < bottom.ev.size(); ++i) {
		VEC3 tmpv = bottom.v[bottom.ev[i]];
		VEC3 tmpn = bottom.vn[bottom.en[i]];
		VEC3 tmpt = bottom.vt[bottom.et[i]];
		glTexCoord2f(tmpt.x*4, tmpt.y*4); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	glEnd();

	//ceiling
	glBindTexture(GL_TEXTURE_2D, texture[7]);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	//glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	//glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_TRIANGLES);
	glColor4f(0.0, 0.0, 1.0, 0.5);
	for(unsigned int i = 0; i < ceiling.ev.size(); ++i) {
		VEC3 tmpv = ceiling.v[ceiling.ev[i]];
		VEC3 tmpn = ceiling.vn[ceiling.en[i]];
		VEC3 tmpt = ceiling.vt[ceiling.et[i]];
		glTexCoord2f(tmpt.x*10, tmpt.y*10); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	glEnd();
}

void drawCodeBox() {
		glBindTexture(GL_TEXTURE_2D, texture[4]);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

		glBegin(GL_TRIANGLES);
		glColor4f(0.0, 0.0, 1.0, 0.5);
		for(unsigned int i = 0; i < door2.ev.size(); ++i) {
			VEC3 tmpv = door2.v[door2.ev[i]];
			VEC3 tmpn = door2.vn[door2.en[i]];
			VEC3 tmpt = door2.vt[door2.et[i]];
			glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
		}
		glEnd();
	
		glBindTexture(GL_TEXTURE_2D, texture[22]);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

		glBegin(GL_TRIANGLES);
		glColor4f(0.0, 0.0, 1.0, 0.5);
		for(unsigned int i = 0; i < codebox.ev.size(); ++i) {
			VEC3 tmpv = codebox.v[codebox.ev[i]];
			VEC3 tmpn = codebox.vn[codebox.en[i]];
			VEC3 tmpt = codebox.vt[codebox.et[i]];
			glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
		}
		glEnd();
}

void drawPistol() {
	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_TRIANGLES);
	glColor4f(0.4, 0.4, 0.4, 0.5);
	for(unsigned int i = 0; i < pistol.ev.size(); ++i) {
		VEC3 tmpv = pistol.v[pistol.ev[i]];
		VEC3 tmpn = pistol.vn[pistol.en[i]];
		VEC3 tmpt = pistol.vt[pistol.et[i]];
		glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	glEnd();
} 

void body_alma(){
   	    	
	   glBindTexture(GL_TEXTURE_2D, textureAlma[0]);
       glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
       glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

		glBegin(GL_TRIANGLES);
	    glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < alma[BODY2].ev.size(); ++i) {
		VEC3 tmpv = alma[BODY2].v[alma[BODY2].ev[i]];
		VEC3 tmpn = alma[BODY2].vn[alma[BODY2].en[i]];
		VEC3 tmpt = alma[BODY2].vt[alma[BODY2].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y);
		glNormal3f(tmpn.x, tmpn.y, tmpn.z);
		glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	    glEnd();

	   glBindTexture(GL_TEXTURE_2D, textureAlma[1]);
       glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
       glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

		glBegin(GL_TRIANGLES);
	    glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < alma[HAIR2].ev.size(); ++i) {
		VEC3 tmpv = alma[HAIR2].v[alma[HAIR2].ev[i]];
		VEC3 tmpn = alma[HAIR2].vn[alma[HAIR2].en[i]];
		VEC3 tmpt = alma[HAIR2].vt[alma[HAIR2].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y);
		glNormal3f(tmpn.x, tmpn.y, tmpn.z);
		glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	    glEnd();

	   glBindTexture(GL_TEXTURE_2D, textureAlma[2]);
       glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
       glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

		glBegin(GL_TRIANGLES);
	    glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < alma[HEAD2].ev.size(); ++i) {
		VEC3 tmpv = alma[HEAD2].v[alma[HEAD2].ev[i]];
		VEC3 tmpn = alma[HEAD2].vn[alma[HEAD2].en[i]];
		VEC3 tmpt = alma[HEAD2].vt[alma[HEAD2].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y);
		glNormal3f(tmpn.x, tmpn.y, tmpn.z);
		glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	    glEnd();

}

void Rhand_alma(){
   	    	
	   glBindTexture(GL_TEXTURE_2D, textureAlma[0]);
       glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
       glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

		glBegin(GL_TRIANGLES);
	    glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < alma[R_HAND2].ev.size(); ++i) {
		VEC3 tmpv = alma[R_HAND2].v[alma[R_HAND2].ev[i]];
		VEC3 tmpn = alma[R_HAND2].vn[alma[R_HAND2].en[i]];
		VEC3 tmpt = alma[R_HAND2].vt[alma[R_HAND2].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y);
		glNormal3f(tmpn.x, tmpn.y, tmpn.z);
		glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	    glEnd();
	
}

void Lhand_alma(){
   	    	
	   glBindTexture(GL_TEXTURE_2D, textureAlma[0]);
       glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
       glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

		glBegin(GL_TRIANGLES);
	    glColor4f(1.0, 1.0, 1.0, 0.5f);
	for(unsigned int i = 0; i < alma[L_HAND2].ev.size(); ++i) {
		VEC3 tmpv = alma[L_HAND2].v[alma[L_HAND2].ev[i]];
		VEC3 tmpn = alma[L_HAND2].vn[alma[L_HAND2].en[i]];
		VEC3 tmpt = alma[L_HAND2].vt[alma[L_HAND2].et[i]];
		glTexCoord2f(tmpt.x, tmpt.y);
		glNormal3f(tmpn.x, tmpn.y, tmpn.z);
		glVertex3f(tmpv.x, tmpv.y, tmpv.z);
	}
	    glEnd();
}

void draw_3d() {
	drawMap();

	if(!door2.changed) {
		drawCodeBox();
	}
	else{  
	//Alma
		if(moving != 5){
			glPushMatrix();
			glTranslatef( 36, 0.5, position.z);
			glTranslatef( -moving, 0, 0);
			body_alma();
			glPopMatrix();	

			glPushMatrix();
			glTranslatef( 36-moving, 0.5, position.z);
			glTranslatef(r_pivot_alma.x + 0.1, r_pivot_alma.y - 0.04, 0 );
			glRotatef(-anglex, 0, 0, 1);
			glTranslatef(-r_pivot_alma.x - 0.1, -r_pivot_alma.y + 0.04, 0 );
			 Rhand_alma();
		     Lhand_alma();
			glPopMatrix();
		}
	}

	if(!paralyze){
		glPushMatrix();
		glTranslatef(position.x + lookat.x, position.y + lookat.y - 0.5, position.z + lookat.z);   // position of pistol 
		glRotatef(angle.x*180/PI, 0, 1, 0);
		glRotatef(angle.y*180/PI + recoil*10, -1, 0, 0);
		 drawPistol();
		glPopMatrix();
	}

	GLdouble transform_mat[16];

	// zombie			     
	for(int i = 0; i < 4; ++i) {
		if(zombie[i].alive) {
			//get matrix
			glPushMatrix();
				glLoadIdentity();
				glTranslatef(zombie[i].pos.x, 0, zombie[i].pos.z);
				glRotatef(zombie[i].angle, 0, 1, 0);
				glGetDoublev(GL_MODELVIEW_MATRIX, transform_mat);
			glPopMatrix();
			//compute bounding box
			computeBB(&(zombie[i].part[R_HAND].bb), zombie_origin_bb[R_HAND], transform_mat);
			computeBB(&(zombie[i].part[L_HAND].bb), zombie_origin_bb[L_HAND], transform_mat);
			computeBB(&(zombie[i].part[BODY].bb), zombie_origin_bb[BODY], transform_mat);
			computeBB(&(zombie[i].part[HEAD].bb), zombie_origin_bb[HEAD], transform_mat);
			//draw
			glPushMatrix();
			glTranslatef(zombie[i].pos.x, 0, zombie[i].pos.z);
			glRotatef(zombie[i].angle, 0, 1, 0);
				drawRightHand(i);
				drawLeftHand(i);
				drawBody(i);
				drawHead(i);
			glPopMatrix();

			//get matrix
			glPushMatrix();
				glLoadIdentity();
				glTranslatef(zombie[i].pos.x, 0, zombie[i].pos.z);
				glRotatef(zombie[i].angle, 0, 1, 0);
				glTranslatef(l_pivot.x, l_pivot.y, 0);
				glRotatef(leg_angle, 0, 0, 1);
				glTranslatef(-l_pivot.x, -l_pivot.y, 0);
				glGetDoublev(GL_MODELVIEW_MATRIX, transform_mat);
			glPopMatrix();
			//compute bounding box
			computeBB(&(zombie[i].part[L_THIGH].bb), zombie_origin_bb[L_THIGH], transform_mat);
			computeBB(&(zombie[i].part[L_CALF].bb), zombie_origin_bb[L_CALF], transform_mat);
			//draw
			glPushMatrix();
			glTranslatef(zombie[i].pos.x, 0, zombie[i].pos.z);
			glRotatef(zombie[i].angle, 0, 1, 0);
			glTranslatef(l_pivot.x, l_pivot.y, 0);
			glRotatef(leg_angle, 0, 0, 1);
			glTranslatef(-l_pivot.x, -l_pivot.y, 0);
				drawLeftThigh(i);
				drawLeftCalf(i);
			glPopMatrix();

			//get matrix
			glPushMatrix();
				glLoadIdentity();
				glTranslatef(zombie[i].pos.x, 0, zombie[i].pos.z);
				glRotatef(zombie[i].angle, 0, 1, 0);
				glTranslatef(r_pivot.x, r_pivot.y, 0);
				glRotatef(-leg_angle, 0, 0, 1);
				glTranslatef(-r_pivot.x, -r_pivot.y, 0);
				glGetDoublev(GL_MODELVIEW_MATRIX, transform_mat);
			glPopMatrix();
			//compute bounding box
			computeBB(&(zombie[i].part[R_THIGH].bb), zombie_origin_bb[R_THIGH], transform_mat);
			computeBB(&(zombie[i].part[R_CALF].bb), zombie_origin_bb[R_CALF], transform_mat);
			//draw
			glPushMatrix();
			glTranslatef(zombie[i].pos.x, 0, zombie[i].pos.z);
			glRotatef(zombie[i].angle, 0, 1, 0);
			glTranslatef(r_pivot.x, r_pivot.y, 0);
			glRotatef(-leg_angle, 0, 0, 1);
			glTranslatef(-r_pivot.x, -r_pivot.y, 0);
				drawRightThigh(i);
				drawRightCalf(i);
			glPopMatrix();
		}
	}

	//paricle
	for(int i = 0; i < PARTICLE_NUMBER; ++i) {
		if(particle[i].active) {
			glPushMatrix();
			glTranslatef(particle[i].pos.x, particle[i].pos.y, particle[i].pos.z);
		    glEnable(GL_TEXTURE_GEN_S); //enable texture coordinate generation
		    glEnable(GL_TEXTURE_GEN_T);
			glBindTexture(GL_TEXTURE_2D, texture[particle_color]);
			glColor3f(1.0, 1.0, 1.0);
			glutSolidCube(0.03);
		    glDisable(GL_TEXTURE_GEN_S); 
		    glDisable(GL_TEXTURE_GEN_T);
			glPopMatrix();
		}
	}

	//light effect
	if(gun_fire_lifetime) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glBindTexture(GL_TEXTURE_2D, texture[11]);
		glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	   glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

		glPushMatrix();
		glTranslatef(position.x + lookat.x*2.2, position.y + lookat.y*2.2 - 0.25, position.z+lookat.z*2.2);
		glRotatef(angle.x*180/PI, 0, 1, 0);
		glRotatef(angle.y*180/PI, -1, 0, 0);
		glBegin(GL_POLYGON);
		glColor4f(1.0, 1.0, 1.0, 0.0);
		glTexCoord2f(1,1);	glVertex3f(0.4, 0.4,  0);
		glTexCoord2f(1,0);	glVertex3f(0.4, -0.4, 0);
		glTexCoord2f(0,0);	glVertex3f(-0.4, -0.4, 0);
		glTexCoord2f(0,1);	glVertex3f(-0.4, 0.4,  0);
		glEnd();
		glPopMatrix();

		glDisable(GL_BLEND);
	}

	//code
	for(int id = 0; id < 4; ++id){
		if(item[id].changed) {
			if(id == 0) {
				glBindTexture(GL_TEXTURE_2D, texture[13]);
				glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			}
			if(id == 1) {
				glBindTexture(GL_TEXTURE_2D, texture[14]);
				glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			}
			if(id == 2) {
				glBindTexture(GL_TEXTURE_2D, texture[15]);
				glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			}
			if(id == 3) {
				glBindTexture(GL_TEXTURE_2D, texture[15]);
				glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
				glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			}

			glPushMatrix();
				glLoadIdentity();
				glTranslatef(zombie[id].pos.x, 0, zombie[id].pos.z);
				glGetDoublev(GL_MODELVIEW_MATRIX, transform_mat);
			glPopMatrix();
			computeBB(&(item[id].bb), item_origin_bb[id], transform_mat);
				
			glPushMatrix();
			glTranslatef(zombie[id].pos.x, 0, zombie[id].pos.z);
			glBegin(GL_TRIANGLES);
			glColor4f(1.0, 1.0, 1.0, 0.5);
			for(unsigned int i = 0; i < item[id].ev.size(); ++i) {
				VEC3 tmpv = item[id].v[item[id].ev[i]];
				VEC3 tmpn = item[id].vn[item[id].en[i]];
				VEC3 tmpt = item[id].vt[item[id].et[i]];
				glTexCoord2f(tmpt.x, tmpt.y); glNormal3f(tmpn.x, tmpn.y, tmpn.z); glVertex3f(tmpv.x, tmpv.y, tmpv.z);
			}
			glEnd();
			glPopMatrix();
		}
	}
}

void drawBitmapText(char *string, float x, float y, int R, int G, int B) {  
	glColor3ub(R, G, B);
	glRasterPos2f(x, y);
	for (int i = 0; string[i] != '\0'; i++) {
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, (int)string[i]);
	}
}

void drawCursor() {

	glBindTexture(GL_TEXTURE_2D, texture[25]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
		glColor3f(1.0, 1.0, 1.0);
		glVertex2f(0.1, 0.005);
		glVertex2f(0.02, 0.005);
		glVertex2f(0.02, -0.005);
		glVertex2f(0.1, -0.005);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[24]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
		glColor3f(1.0, 1.0, 1.0);
		glVertex2f(0.005, 0.1);
		glVertex2f(-0.005, 0.1);
		glVertex2f(-0.005, 0.02);
		glVertex2f(0.005, 0.02);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[25]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
		glColor3f(1.0, 1.0, 1.0);
		glVertex2f(-0.1, 0.005);
		glVertex2f(-0.02, 0.005);
		glVertex2f(-0.02, -0.005);
		glVertex2f(-0.1, -0.005);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[24]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
		glColor3f(1.0, 1.0, 1.0);
		glVertex2f(0.005, -0.1);
		glVertex2f(-0.005, -0.1);
		glVertex2f(-0.005, -0.02);
		glVertex2f(0.005, -0.02);
	glEnd();
}

void drawHPBar() {
	
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glBindTexture(GL_TEXTURE_2D, texture[HP_color]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
		glColor4f(1.0, 1.0, 1.0, 0.9);
		glTexCoord2f(1, 1);	glVertex2f(-2.19 -(1- playerHP*0.01), -1.7);     
		glTexCoord2f(1, 0);	glVertex2f(-2.19 -(1- playerHP*0.01), -1.8);
		glTexCoord2f(0, 0);	glVertex2f(-3.19, -1.8);
		glTexCoord2f(0, 1);	glVertex2f(-3.19 , -1.7);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, texture[35]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
		glColor4f(1.0, 1.0, 1.0, 0.9);
		glTexCoord2f(1, 1);	glVertex2f(-2.15, -1.5);     
		glTexCoord2f(1, 0);	glVertex2f(-2.15, -1.9);
		glTexCoord2f(0, 0);	glVertex2f(-3.55, -1.9);
		glTexCoord2f(0, 1);	glVertex2f(-3.55, -1.5);
	glEnd();

	glDisable(GL_BLEND);
}

void drawitem(){
	    float modi = 0.65;
		if(backpack[0]){
			glBindTexture(GL_TEXTURE_2D, texture[13]);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
		    glBegin(GL_POLYGON);
		    glColor3f(1.0, 1.0, 1.0);
				glTexCoord2f(1,1);	glVertex2f( 1.5, 1.90 );
				glTexCoord2f(1,0);	glVertex2f( 1.5, 1.50 );
				glTexCoord2f(0,0);	glVertex2f( 1.1, 1.50 );
				glTexCoord2f(0,1);	glVertex2f( 1.1, 1.90);
			 glEnd();
		}
		if(backpack[1]) {
			glBindTexture(GL_TEXTURE_2D, texture[14]);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			glBegin(GL_POLYGON);
			glColor3f(1.0, 1.0, 1.0);
			glTexCoord2f(1,1);	glVertex2f( 1.5 + modi+0.012, 1.90 );
			glTexCoord2f(1,0);	glVertex2f( 1.5 + modi+0.012, 1.50 );
			glTexCoord2f(0,0);	glVertex2f( 1.1 + modi+0.012, 1.50 );
			glTexCoord2f(0,1);	glVertex2f( 1.1 + modi+0.012, 1.90);
			glEnd();
		}
		if(backpack[2]) {
		   glBindTexture(GL_TEXTURE_2D, texture[15]);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			glBegin(GL_POLYGON);
			glColor3f(1.0, 1.0, 1.0);
			glTexCoord2f(1,1);	glVertex2f( 1.5 + 2*modi, 1.9 );
			glTexCoord2f(1,0);	glVertex2f( 1.5 + 2*modi, 1.5 );
			glTexCoord2f(0,0);	glVertex2f( 1.1 + 2*modi, 1.5 );
			glTexCoord2f(0,1);	glVertex2f( 1.1 + 2*modi, 1.9);
		   glEnd();
		}
		 if(backpack[3]) {
		    glBindTexture(GL_TEXTURE_2D, texture[19]);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			glBegin(GL_POLYGON);
			glColor3f(1.0, 1.0, 1.0);
			glTexCoord2f(1,1);	glVertex2f( 1.5 + 3*modi, 1.9 );
			glTexCoord2f(1,0);	glVertex2f( 1.5 + 3*modi, 1.5 );
			glTexCoord2f(0,0);	glVertex2f( 1.1 + 3*modi, 1.5 );
			glTexCoord2f(0,1);	glVertex2f( 1.1 + 3*modi, 1.9);
		   glEnd();
		}

}

void MainMenu(){

	if(exit_flash){
	glBindTexture(GL_TEXTURE_2D, texture[28]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
		glTexCoord2f(1,1);	glVertex2f( 3.4, -1.45 );
		glTexCoord2f(1,0);	glVertex2f( 3.4, -1.85 );
		glTexCoord2f(0,0);	glVertex2f( 2.3, -1.85 );
		glTexCoord2f(0,1);	glVertex2f( 2.3, -1.45 );
	glEnd(); 
	}

	if(start_flash){
	glBindTexture(GL_TEXTURE_2D, texture[27]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
		glTexCoord2f(1,1);	glVertex2f( 3.4, -1.40 + 0.4 );
		glTexCoord2f(1,0);	glVertex2f( 3.4, -1.80 + 0.4 );
		glTexCoord2f(0,0);	glVertex2f( 2.3, -1.80 + 0.4 );
		glTexCoord2f(0,1);	glVertex2f( 2.3, -1.40 + 0.4 );
	glEnd(); 
	}

	glBindTexture(GL_TEXTURE_2D, texture[26]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
		glTexCoord2f(1,1);	glVertex2f( 3.6,  2.0);
		glTexCoord2f(1,0);	glVertex2f( 3.6, -2.0);
		glTexCoord2f(0,0);	glVertex2f(-3.6, -2.0);
		glTexCoord2f(0,1);	glVertex2f(-3.6,  2.0);
	glEnd(); 

}

void GameOver(){

	if( GoBack_flash ){
	glBindTexture(GL_TEXTURE_2D, texture[30]); // Go back
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
		glTexCoord2f(1,1);	glVertex2f(  0.7 + 0.25,  -1.0 + 0.5 );
		glTexCoord2f(1,0);	glVertex2f(  0.7 + 0.25,  -1.4 + 0.5 );
		glTexCoord2f(0,0);	glVertex2f(-(0.7 + 0.25), -1.4 + 0.5 );
		glTexCoord2f(0,1);	glVertex2f(-(0.7 + 0.25), -1.0 + 0.5 );
	glEnd();
	}

	if( exit_over_flash ){
	glBindTexture(GL_TEXTURE_2D, texture[29]); // Exit
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
		glTexCoord2f(1,1);	glVertex2f( 0.65, -1.05 );
		glTexCoord2f(1,0);	glVertex2f( 0.65, -1.5 );
		glTexCoord2f(0,0);	glVertex2f(-0.55, -1.5 );
		glTexCoord2f(0,1);	glVertex2f(-0.55, -1.05 );
	glEnd();
	}
	glBindTexture(GL_TEXTURE_2D, texture[18]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
		glTexCoord2f(1,1);	glVertex2f( 3.6,  2.0);
		glTexCoord2f(1,0);	glVertex2f( 3.6, -2.0);
		glTexCoord2f(0,0);	glVertex2f(-3.6, -2.0);
		glTexCoord2f(0,1);	glVertex2f(-3.6,  2.0);
	glEnd();
}

void EndScene(){
	
	if( GoBack_flash ){
	glBindTexture(GL_TEXTURE_2D, texture[34]); // Go back
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
		glTexCoord2f(1,1);	glVertex2f(  0.7 + 0.25,  -1.0 + 0.5 );
		glTexCoord2f(1,0);	glVertex2f(  0.7 + 0.25,  -1.4 + 0.5 );
		glTexCoord2f(0,0);	glVertex2f(-(0.7 + 0.25), -1.4 + 0.5 );
		glTexCoord2f(0,1);	glVertex2f(-(0.7 + 0.25), -1.0 + 0.5 );
	glEnd();
	}

	if( exit_over_flash ){
	glBindTexture(GL_TEXTURE_2D, texture[33]); // Exit
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
		glTexCoord2f(1,1);	glVertex2f( 0.65, -1.05 );
		glTexCoord2f(1,0);	glVertex2f( 0.65, -1.5 );
		glTexCoord2f(0,0);	glVertex2f(-0.55, -1.5 );
		glTexCoord2f(0,1);	glVertex2f(-0.55, -1.05 );
	glEnd();
	}
	glBindTexture(GL_TEXTURE_2D, texture[32]);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);

	glBegin(GL_POLYGON);
	glColor3f(1.0, 1.0, 1.0);
		glTexCoord2f(1,1);	glVertex2f( 3.6,  2.0);
		glTexCoord2f(1,0);	glVertex2f( 3.6, -2.0);
		glTexCoord2f(0,0);	glVertex2f(-3.6, -2.0);
		glTexCoord2f(0,1);	glVertex2f(-3.6,  2.0);
	glEnd();

}

void draw_2d(){
	
	if(gameinitial){
	    MainMenu();
	}

	if(!pause) {	
		drawHPBar();
		if(playerHP){
		 drawitem();
		 drawCursor();
		}
	}
	else {
		if(event_over && !TheEnd) {
			pause = !pause;
		}
		if(codebox.changed && !event_over) {
			if(code1_event) {
				code1[0]++;
				code1_event = false;
				if(code1[0] > 57)
					code1[0] = 48;
			}
			drawBitmapText(code1, -0.8, 0.0, 14, 159, 42);
			if(code2_event) {
				code2[0]++;
				code2_event = false;
				if(code2[0]>57)
					code2[0] = 48;
		   }
			drawBitmapText(code2, -0.3, 0.0, 14, 159, 42);
			if(code3_event) {
				code3[0]++;
				code3_event = false;
				if(code3[0]>57)
					code3[0] = 48;
			}
			drawBitmapText(code3, 0.20, 0.0, 14, 159, 42);
			if(code4_event) {
				code4[0]++;
				code4_event = false;
				if(code4[0]>57)
					code4[0] = 48;		   
			}
			drawBitmapText(code4,  0.7, 0.0, 14, 159, 42);
			if(code1[0] == code[0] && code2[0] == code[1] && code3[0] == code[2] && code4[0] == code[3]) {
				code_correct = true;
			}

			glBindTexture(GL_TEXTURE_2D, texture[20]);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			glBegin(GL_POLYGON);
				glColor3f(1.0, 1.0, 1.0);
				glTexCoord2f(0,0); glVertex2f(-0.95, -0.25);
				glTexCoord2f(0,1); glVertex2f(-0.95, 0.25);
				glTexCoord2f(1,1); glVertex2f(-0.55, 0.25);
				glTexCoord2f(1,0); glVertex2f(-0.55, -0.25);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, texture[20]);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			glBegin(GL_POLYGON);
				glColor3f(1.0, 1.0, 1.0);
				glTexCoord2f(0,0);  glVertex2f(-0.45, -0.25);
				glTexCoord2f(0,1);  glVertex2f(-0.45, 0.25);
				glTexCoord2f(1,1);  glVertex2f(-0.05, 0.25);
				glTexCoord2f(1,0);  glVertex2f(-0.05, -0.25);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, texture[20]);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			glBegin(GL_POLYGON);
				glColor3f(1.0, 1.0, 1.0);
				glTexCoord2f(0,0); glVertex2f(0.05, -0.25);
				glTexCoord2f(0,1); glVertex2f(0.05, 0.25);
				glTexCoord2f(1,1); glVertex2f(0.45, 0.25);
				glTexCoord2f(1,0); glVertex2f(0.45, -0.25);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, texture[20]);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			glBegin(GL_POLYGON);
			    glColor3f(1.0, 1.0, 1.0);
				glTexCoord2f(0,0); glVertex2f(0.55, -0.25);
				glTexCoord2f(0,1); glVertex2f(0.55, 0.25);
				glTexCoord2f(1,1); glVertex2f(0.95, 0.25);
				glTexCoord2f(1,0); glVertex2f(0.95, -0.25);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, texture[21]);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			glBegin(GL_POLYGON);
				glColor3f(1.0, 1.0, 1.0);
				glTexCoord2f(0,0); glVertex2f(1.1, -0.1);
				glTexCoord2f(0,1); glVertex2f(1.1, 0.1);
				glTexCoord2f(1,1); glVertex2f(1.3, 0.1);
				glTexCoord2f(1,0); glVertex2f(1.3, -0.1);
			glEnd();

			glBindTexture(GL_TEXTURE_2D, texture[22]);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST);
			glBegin(GL_POLYGON);
				glColor3f(1.0, 1.0, 1.0);
				glTexCoord2f(0,0); glVertex2f(-1.1, -0.4);
				glTexCoord2f(0,1); glVertex2f(-1.1, 0.4);
				glTexCoord2f(1,1); glVertex2f(1.5, 0.4);
				glTexCoord2f(1,0); glVertex2f(1.5, -0.4);
			glEnd();
		}
	}

	if(gameover) {
		GameOver();
	}

	if(TheEnd){
		EndScene();
	}

}

void Shade_damage() {
	GLfloat mat_specular[] = {1.0, 0.1, 0.1, 1.0};
	GLfloat mat_diffuse[] = {1.0, 0.1, 0.1, 1.0};
	GLfloat mat_ambient[] = {1.0, 0.0, 0.0, 1.0};
	
	GLfloat mat_shininess = {80.0};

	GLfloat light_specular[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat light_diffuse[] = {1.0, 1.0, 1.0, 1.0};
	GLfloat light_ambient[] = {1.0, 1.0, 1.0, 1.0};

	GLfloat lightPosition[] = {5.0f, 5.0f, 0.0f, 1.0f};
	
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);

	glShadeModel(GL_SMOOTH);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

void Shade_normal(){

	GLfloat mat_specular[] = {0.1, 0.1, 0.1, 1.0};
	GLfloat mat_diffuse[] = {0.1, 0.1, 0.1, 1.0};
	GLfloat mat_ambient[] = {1.0, 0.6, 0.6, 1.0};
	
	GLfloat mat_shininess={40.0};

	GLfloat light_specular[] = {0.4, 0.0, 0.0, 1.0};
	GLfloat light_diffuse[] = {0.4, 0.4, 0.4, 1.0};
	GLfloat light_ambient[] = {0.9, 0.5, 0.5, 1.0};

	GLfloat lightPosition[] = {5.0f, 5.0f, 0.0f, 1.0f};

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	
	glShadeModel(GL_SMOOTH); /* enable smooth shading */
	glEnable(GL_LIGHTING); /* enable lighting */
	glEnable(GL_LIGHT0);  /* enable light 0 */
}

void Shade_alma(){

	GLfloat mat_specular[] = {0.1, 0.1, 0.1, 1.0};
	GLfloat mat_diffuse[] = {0.1, 0.1, 0.1, 1.0};
	GLfloat mat_ambient[] = {1.0, 0.6, 0.6, 1.0};
	
	GLfloat mat_shininess={40.0};

	GLfloat light_specular[] = {0.0, 0.0, 0.2, 1.0};
	GLfloat light_diffuse[] = {0.1, 0.1, 0.1, 1.0};
	GLfloat light_ambient[] = {0.2, 0.2, 0.2, 1.0};

	GLfloat lightPosition[] = {5.0f, 5.0f, 0.0f, 1.0f};

	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
	
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	
	glShadeModel(GL_SMOOTH); /* enable smooth shading */
	glEnable(GL_LIGHTING); /* enable lighting */
	glEnable(GL_LIGHT0);  /* enable light 0 */

}

void display() {

	//clear window
   glClearDepth(1.0);
   glClearColor(0.0f, 0.0f, 0.0f, 1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//game world
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluPerspective(45.0, (float)ww/wh, 0.1, 10000.0);
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(position.x, position.y, position.z,
				 position.x + lookat.x, position.y + lookat.y, position.z + lookat.z,
				 0, 1, 0);

	//screen turn red
	if(damage)
		Shade_damage();
	else if(alma_appear)
		Shade_alma();
	else
		Shade_normal();

	draw_3d();
	glFlush();

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

	glEnable(GL_DEPTH_TEST);

   //interface
	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);
	
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(((GLfloat) ww /(GLfloat) wh) <= (16 / 9))
		gluOrtho2D(-2.0, 2.0, -2.0 * (GLfloat) wh / (GLfloat) ww, 2.0 * (GLfloat) wh / (GLfloat) ww);
	else
		gluOrtho2D(-2.0 * (GLfloat) ww / wh, 2.0 * (GLfloat) ww / wh, -2.0, 2.0);
	glGetDoublev(GL_PROJECTION_MATRIX, projection2d);

	glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

	draw_2d();
	glFlush();

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview2d);

	glutSwapBuffers();
}

void reshape(int w, int h){
    glViewport(0, 0, w, h);
	 ww = w;
	 wh = h;
}


void main(int argc, char **argv) {
	load_obj("map.obj", map);
	load_obj("box.obj", &box);
	load_obj("door2.obj", &door2);
	load_obj("code box.obj", &codebox);
	load_obj("floor.obj", &bottom);
	load_obj("ceiling.obj", &ceiling);
	load_obj("gun3D.obj", &pistol);
	load_obj("zombieC.obj", zombie[0].part);
	load_obj("Items.obj", item);
	load_obj("alma.obj", alma);

	r_pivot.x = (zombie[0].part[R_THIGH].bb.xmax + zombie[0].part[R_THIGH].bb.xmin) / 2;
	r_pivot.y = zombie[0].part[R_THIGH].bb.ymax;

	l_pivot.x = (zombie[0].part[L_THIGH].bb.xmax + zombie[0].part[L_THIGH].bb.xmin) / 2;
	l_pivot.y = zombie[0].part[L_THIGH].bb.ymax;

	r_pivot_alma.x = (alma[R_HAND2].bb.xmax + alma[R_HAND2].bb.xmin)/2;
    r_pivot_alma.y = alma[R_HAND2].bb.ymax ;

	for(int i = 0; i < 9; ++i) {
		zombie_origin_bb[i] = zombie[0].part[i].bb;
	}

	for(int i = 0; i < 4; ++i) {
		item_origin_bb[i] = item[i].bb;
	}

	for(int i = 0; i < 4; ++i) {
		for(int j = 0; j < 9; ++j) {
			zombie[i].part[j] = zombie[0].part[j];
			zombie[i].alive = true;
			zombie[i].healthPoint = 100.0;
			zombie[i].chasing = false;
			zombie[i].groaning = 0;
		}
	}

	zombie[0].pos.x = 6.0;
	zombie[0].pos.y = 0;
	zombie[0].pos.z = 15.0;

	zombie[1].pos.x = 6.0;
	zombie[1].pos.y = 0;
	zombie[1].pos.z = -1.7;

	zombie[2].pos.x = 39.0;
	zombie[2].pos.y = 0;
	zombie[2].pos.z = 3.0;

	zombie[3].pos.x = 10.4;
	zombie[3].pos.y = 0;
	zombie[3].pos.z = -10.0;


	SoundSystemInit();
	glutInit(&argc, argv);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(ww, wh);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutCreateWindow("Psychic Prison");
	load_texture();
	glutReshapeFunc(reshape);
	result = FMOD_System_PlaySound(fmodSystem, FMOD_CHANNEL_FREE, requiem, 0, &BGMchannel);
	ERRCHECK(result);
	glutDisplayFunc(display); 
	glutPassiveMotionFunc(mouseMotion);
	glutIgnoreKeyRepeat(true);
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);
	glutIdleFunc(idle);
	glutMouseFunc(mouse);
	glutMainLoop();

	SoundSystemClose();
}