#include "SDL.h"
#include "SDL_opengl.h"
#include <iostream>
#include "FreeImage.h"
#include <stdio.h>
#include <conio.h>
#include <GL/glu.h>
#include <cmath>
#include <chrono>
#include <irrKlang.h>
#include <assimp/Importer.hpp>      
#include <assimp/scene.h> 
#include <assimp/postprocess.h>

using namespace std;
using namespace irrklang;

SDL_Window* win;
SDL_GLContext context;
bool fin = false;
bool pause = false;
bool rotateCamera = false;
//Alto y ancho de la pantalla
const float mapWidth = 12.5;
const float mapHeigth = 12.5;

//Cantidad de entradas de la matriz
const int mapMatrixHeigth = 21;
const int mapMatrixWidth = 21;

const float charMovePerFrame = .1;
const float enemyMovePerFrame = 0.0015;
const int enemiesCount = 7;

float entrySize = mapWidth / mapMatrixWidth; //Tamaño de cada entrada de la matriz que representa el mapa

//Camera perspective variables
float xCamera = mapWidth/2;
float  yCamera = -mapHeigth/2;
float zCamera = 20;

//Este punto es al que apunta la camara
float xCenter = xCamera;
float yCenter = yCamera;
float zCenter = 0;

//Este vector up indica las direcciones de la camara
float xUp = 0;
float yUp = 1;
float zUp = 0;

float degrees = 0;

//Define si estoy en la camara aerea o en la vista de primera persona, hay que ver si tenemos que agregar una nueva camara "libre".
bool VistaAerea = true;

//variable de sonido activado o desactivado
bool moneda1 = false;
ISoundEngine* engine1 = createIrrKlangDevice();

//tiempo en pausa y acumulador auxiliar
time_t globalTimeInPause;
time_t timeActualPauseInit;

std::chrono::milliseconds::rep timeBetweenFrames;
std::chrono::milliseconds::rep lastTimeRegisteredBetweenFrames;

enum ScreenType {
	Menu,
	Play,
	Options
};

//Define si estoy en el menu principal, en el juego, en las opciones, etc.
ScreenType actualScreen = Menu;
ScreenType previousScreen;


//Texturas
const int texturesCount = 12;
GLuint textures[texturesCount];


bool firstMouse = true;
float yaw = -90.0f;	// yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
float pitch = 0.0f;
int yawDirection = 0;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

int xMouse = 1920 / 2;
int yMouse = 1080 / 2;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

time_t actualTime;
time_t lastTime;

enum Orientation {
	Rigth,
	Left,
	Bottom,
	Top
};

struct Player {
	float xPosition;
	float yPosition;
	float size;
	Orientation orientation;
	int maxBombsCounter;
	time_t actualBombDurationInSeconds;
	time_t explosionRange;
	bool movingLeft = false;
	bool movingRigth = false;
	bool movingTop = false;
	bool movingBottom = false;
};

struct Metadata {
	int score;
	int lifesCount;
	time_t initTime;
	time_t totalSecondsOfGame;
	float gameVelocity = 1;
};

struct GameOptions {
	bool applyTextures = true;
	bool wireframeMode = false;
	bool facetadoInterpoladoMode = false;
	bool luzActivada = false;
};

GameOptions options;

struct Enemy {
	float xPosition;
	float yPosition;
	float size;
	Orientation orientation;
	bool isAlive;
};

struct Bomb {
	float xPosition;
	float yPosition;
	time_t startTime;
	time_t duration;
	time_t explosionStartTime;
	bool eliminatedObjects;
};

struct Vertice {
	float Posicionx;
	float Posiciony;
	float Posicionz;
	float Normalx;
	float Normaly;
	float Normalz;
	float Texturax;
	float Texturay;
};

Enemy enemies[enemiesCount];
Bomb** bombs;
Player player;
Metadata metadata;

enum ObjectType {
	Box,
	Wall
};

struct Object {
	float xPosition;
	float yPosition;
	float isAlive;
	ObjectType objectType;
};

Object* objectsInMap[mapMatrixWidth][mapMatrixHeigth];

bool** getBoxesIndexes() {
	bool** matrix = new bool* [mapMatrixHeigth];
	matrix[0] = new bool[21] {false, false, false, true, false, false, false, false, false, false, false, false, false, false, true, false, true, true, true, false, false };
	matrix[1] = new bool[21] {false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
	matrix[2] = new bool[21] {false, true, false, true, false, false, false, false, false, false, true, true, true, false, false, false, true, true, false, false, true };
	matrix[3] = new bool[21] {false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true };
	matrix[4] = new bool[21] {false, false, true, true, true, true, false, true, false, true, false, true, true, true, true, false, false, true, true, true, true };
	matrix[5] = new bool[21] {true, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
	matrix[6] = new bool[21] {true, false, false, false, false, false, false, true, true, true, true, true, false, false, false, false, false, false, false, true, false };
	matrix[7] = new bool[21] {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
	matrix[8] = new bool[21] {true, true, true, true, false, false, false, false, false, false, false, false, true, true, false, false, true, true, true, true, true };
	matrix[9] = new bool[21] {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
	matrix[10] = new bool[21] {true, false, false, false, true, true, true, false, true, false, false, false, false, true, true, true, true, false, false, false, false };
	matrix[11] = new bool[21] {true, false, false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false };
	matrix[12] = new bool[21] {true, false, false, true, true, false, false, true, true, false, true, true, true, false, false, false, false, false, false, false, false };
	matrix[13] = new bool[21] {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
	matrix[14] = new bool[21] {true, true, true, true, false, false, false, true, true, false, true, false, true, false, false, true, true, false, true, true, true };
	matrix[15] = new bool[21] {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false };
	matrix[16] = new bool[21] {true, true, true, true, true, false, false, false, false, true, true, true, false, false, false, false, false, false, true, true, true };
	matrix[17] = new bool[21] {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true };
	matrix[18] = new bool[21] {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, true };
	matrix[19] = new bool[21] {true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true };
	matrix[20] = new bool[21] {true, false, false, false, true, true, true, true, false, false, false, true, true, true, true, false, false, true, true, true, false };
	return matrix;
}

bool** getWallIndexes() {
	bool** matrix = new bool* [mapMatrixHeigth];
	for (int i = 0; i < mapMatrixHeigth; i++) {
		if (i % 2 == 1) {
			matrix[i] = new bool[mapMatrixWidth];
			for (int j = 0; j < mapMatrixWidth; j++) {
				if (j % 2 == 1) {
					matrix[i][j] = true;
				}
				else {
					matrix[i][j] = false;
				}
			}
		}
		else {
			matrix[i] = new bool[mapMatrixWidth];
			for (int j = 0; j < mapMatrixWidth; j++) {
				matrix[i][j] = false;
			}
		}
	}
	return matrix;
}

float getAngle(Orientation orientation) {
	switch (orientation) {
	case Rigth:
		return 0;
	case Left:
		return 180;
	case Bottom:
		return 90;
	case Top:
		return 270;
	}
}

float** getEnemiesInitialLocations() {

	float** enemiesLocations = new float* [enemiesCount];
	for (int i = 0; i < enemiesCount; i++) {
		enemiesLocations[i] = new float[2];
	}
	enemiesLocations[0][0] = entrySize * 2 + enemies[0].size/2;
	enemiesLocations[0][1] = - entrySize * 10 - enemies[0].size/2;

	enemiesLocations[1][0] = entrySize * 19 + enemies[0].size / 2;
	enemiesLocations[1][1] = -entrySize * 2 - enemies[0].size / 2;

	enemiesLocations[2][0] = entrySize * 5 + enemies[0].size / 2;
	enemiesLocations[2][1] = -entrySize * 16 - enemies[0].size / 2;

	enemiesLocations[3][0] = entrySize * 4 + enemies[0].size / 2;
	enemiesLocations[3][1] = -entrySize * 19 - enemies[0].size / 2;\

	enemiesLocations[4][0] = entrySize * 8 + enemies[0].size / 2;
	enemiesLocations[4][1] = -entrySize * 2 - enemies[0].size / 2;

	enemiesLocations[5][0] = entrySize * 12 + enemies[0].size / 2;
	enemiesLocations[5][1] = -entrySize * 10 - enemies[0].size / 2;

	enemiesLocations[6][0] = entrySize * 4 + enemies[0].size / 2;
	enemiesLocations[6][1] = -entrySize * 5 - enemies[0].size / 2;


	return enemiesLocations;
}

void resetEnemiesLocations() {
	float** enemiesLocations = getEnemiesInitialLocations();

	for (int i = 0; i < enemiesCount; i++) {
		enemies[i].isAlive = true;
		enemies[i].orientation = Left;
		enemies[i].xPosition = enemiesLocations[i][0];
		enemies[i].yPosition = enemiesLocations[i][1];
	}
}

void initializeObjectsInMap() {
	
	bool** boxes = getBoxesIndexes();
	bool** walls = getWallIndexes();
	
	for (int i = 0; i < mapMatrixWidth; i++) {
		for (int j = 0; j < mapMatrixHeigth; j++) {
			if (boxes[i][j]) {
				Object* box = new Object;
				box->isAlive = true;
				box->xPosition = entrySize * (j);
				box->yPosition = -entrySize * (i + 1);
				box->objectType = Box;
				objectsInMap[i][j] = box;
			}
			else if (walls[i][j]) {
				Object* wall = new Object;
				wall->isAlive = true;
				wall->xPosition = entrySize * (j);
				wall->yPosition = -entrySize * (i + 1);
				wall->objectType = Wall;
				objectsInMap[i][j] = wall;
			}
			else {
				objectsInMap[i][j] = NULL;
			}
		}
	}
	for (int i = 0; i < enemiesCount; i++) {
		enemies[i].size = entrySize - .1;
	}
	resetEnemiesLocations();
}

std::vector<Vertice> cargarModelo() {
	Assimp::Importer importador; // definimos importer
	const aiScene* bomberman = importador.ReadFile("../Modelo/scene.gltf", aiProcess_Triangulate);
	const aiMesh* malla = bomberman->mMeshes[0];
	std::vector<Vertice> arregloVertices;
	for (unsigned int i = 0; i < malla->mNumFaces; i++) {
		for (unsigned int j = 0; j < 3; j++) {
			Vertice vert;
			vert.Posicionx = malla->mVertices[malla->mFaces[i].mIndices[j]].x;
			vert.Posiciony = malla->mVertices[malla->mFaces[i].mIndices[j]].y;
			vert.Posicionz = malla->mVertices[malla->mFaces[i].mIndices[j]].z;
			vert.Normalx = malla->mNormals[malla->mFaces[i].mIndices[j]].x;
			vert.Normaly = malla->mNormals[malla->mFaces[i].mIndices[j]].y;
			vert.Normalz = malla->mNormals[malla->mFaces[i].mIndices[j]].z;
			vert.Texturax = malla->mTextureCoords[0][malla->mFaces[i].mIndices[j]].x;
			vert.Texturay = malla->mTextureCoords[0][malla->mFaces[i].mIndices[j]].y;
			arregloVertices.emplace_back(vert);
		}
	}
	return arregloVertices;
}

void dibujarModelo(std::vector<Vertice>& arreglo) {
	float angle = getAngle(player.orientation);
	glPushMatrix();
	glTranslatef(player.xPosition - .45, player.yPosition -.5, -.1);
	if (options.luzActivada) {
		glEnable(GL_LIGHTING);
	}
	//	glScalef(0.1, 0.1, 0.1);
	//	glPushMatrix();
	//	glTranslatef(1.0f, -1.0f, 0.0f);
	if (options.wireframeMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Modo de alambre
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Modo de relleno solido
	}
	if (options.facetadoInterpoladoMode) {
		glShadeModel(GL_FLAT); // Facetado
	}
	else {
		glShadeModel(GL_SMOOTH); //Interpolado
	}
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[11]);
	glBegin(GL_TRIANGLES);
	for (auto& vertice : arreglo) {
		glTexCoord2f(vertice.Texturax, vertice.Texturay);
		glNormal3f(vertice.Normalx, vertice.Normaly, vertice.Normalz);
		glVertex3f(vertice.Posicionx/15, vertice.Posiciony/15, vertice.Posicionz/15);
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	//	glPopMatrix();
	if (options.luzActivada) {
		glDisable(GL_LIGHTING);
	}
	glPopMatrix();
}

void activarSonido(int sonido) {
	if (engine1 != NULL) {

		if (sonido == 0) {
			ISound* sound0 = engine1->play2D("../sonidos/IntroSound.mp3", false, false, true);
			sound0->setVolume(0.1);
		}
		if (sonido == 1) {
			ISound* sound1 = engine1->play2D("../sonidos/BackgroundSound.mp3", false, false, true);
			sound1->setVolume(0.1);
		}
		if (sonido == 2) {
			ISound* sound2 = engine1->play2D("../sonidos/SelectSound.mp3", false, false, true);
		}
		if (sonido == 3) {
			ISound* sound3 = engine1->play2D("../sonidos/MoveSound.mp3", false, false, true);
		}
		if (sonido == 4) {
			ISound* sound4 = engine1->play2D("../sonidos/BombSound.mp3", false, false, true);
		}
		if (sonido == 5) {
			ISound* sound5 = engine1->play2D("../sonidos/WoodSound.mp3", false, false, true);
		}
		if (sonido == 6) {
			ISound* sound6 = engine1->play2D("../sonidos/MonsterSound.mp3", false, false, true);
		}
	}
}

void desactivarSonido(int sonido) {
	if (engine1 != NULL) {
		engine1->stopAllSounds();
	}
}

void initializeWindow() {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		cerr << "No se pudo iniciar SDL: " << SDL_GetError() << endl;
		exit(1);
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);
	win = SDL_CreateWindow("Bomberman",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		screenWidth, screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	context = SDL_GL_CreateContext(win);

	glMatrixMode(GL_PROJECTION);

	float color = 0;
	glClearColor(color, color, color, 1);

	gluPerspective(45, 640 / 480.f, 0.1, 100);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);

}

void initializeMetadata() {
	metadata.score = 0;
	metadata.lifesCount = 4;
	metadata.totalSecondsOfGame = 400;
	time(&metadata.initTime);
}

void initializePlayer() {
	player.xPosition = (mapWidth / (float)mapMatrixWidth) / 2; //Centrado en el primer cubo del cuadriculado
	player.yPosition = (-mapHeigth / (float)mapMatrixHeigth) / 2; //Centrado en el primer cubo del cuadriculad
	player.size = entrySize - .2;
	player.orientation = Rigth;
	player.maxBombsCounter = 1;
	player.actualBombDurationInSeconds = 2.2 * metadata.gameVelocity;
	player.explosionRange = 1;
	bombs = new Bomb * [1];
	bombs[0] = NULL;
}


GLfloat luz_posicion[4] = { 0, 0, 1, 1 };
GLfloat luz_posicion1[4] = { 6, 6, 1, 1};
GLfloat luz_posicion2[4] = { 12.5, -12.5, 1, 1};
GLfloat colorLuz[4] = {0, 1, 0, 1};
GLfloat colorLuz1[4] = { 1, 0, 0, 1 };
GLfloat colorLuz2[4] = { 0, 0, 1, 1 };

int actualVista = 0;

//Vista aerea: 0
//Vista POV: 1
//Vista libre: 2

void clearBuffersAndSetLookPoint() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	//Primeros 3 parametros marcan donde esta el ojo, segundos 3 parametros marcan donde esta el centro de la escena.
	if (actualVista == 0) {
		gluLookAt(xCamera, yCamera, zCamera, xCenter, yCenter, zCenter, xUp, yUp, zUp);
	}
	else if (actualVista == 1) {
		gluLookAt(xCamera, yCamera, zCamera, xCamera + xCenter, yCamera + yCenter, zCamera + zCenter, xUp, yUp, zUp);
	}
	else {
		gluLookAt(xCamera, yCamera, zCamera, xCamera + xCenter, yCamera + yCenter, zCamera + zCenter, xUp, yUp, zUp);
	}

	glEnable(GL_LIGHT0); // habilita la luz 0
	glLightfv(GL_LIGHT0, GL_POSITION, luz_posicion);
	glLightfv(GL_LIGHT0, GL_AMBIENT, colorLuz);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, colorLuz);
	glLightfv(GL_LIGHT0, GL_SPECULAR, colorLuz);
	
	glEnable(GL_LIGHT1); // habilita la luz 1
	glLightfv(GL_LIGHT1, GL_POSITION, luz_posicion1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, colorLuz1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, colorLuz1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, colorLuz1);

	glEnable(GL_LIGHT2); // habilita la luz 2
	glLightfv(GL_LIGHT2, GL_POSITION, luz_posicion2);
	glLightfv(GL_LIGHT2, GL_AMBIENT, colorLuz2);
	glLightfv(GL_LIGHT2, GL_DIFFUSE, colorLuz2);
	glLightfv(GL_LIGHT2, GL_SPECULAR, colorLuz2);

	if (rotateCamera && actualVista == 0) {
		degrees = degrees + 0.8f;
	}
	glTranslatef(mapWidth/2, - mapHeigth/2, 0);  // Trasladar el objeto al punto de rotación
	glRotatef(degrees, 0.0, 0.0, 1.0);   // Rotar el objeto (por ejemplo, alrededor del eje Y)
	glTranslatef(-mapWidth / 2, mapHeigth / 2, 0);
}

void createCube(float x, float y, float size, float height, int textureIndex) {
	glPushMatrix();
		//Me translado hacia donde quiero dibujar el cubo
		glTranslatef(x, y, 0);

		//Habilito las luces y texturas
		
		if (options.wireframeMode) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Modo de alambre
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Modo de relleno solido
		}
		
		if (options.facetadoInterpoladoMode) {
			glShadeModel(GL_FLAT); // Facetado
		}
		else {
			glShadeModel(GL_SMOOTH); //Interpolado
		}

		if (options.luzActivada) {
			glEnable(GL_LIGHTING);
		}
			//	glEnable(GL_COLOR_MATERIAL);
		if (options.applyTextures) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textures[textureIndex]);
		}
		glColor3f(1, 1, 1);

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(0, 0, 0);
		glTexCoord2f(1, 0);
		glVertex3f(size, 0, 0);
		glTexCoord2f(1, 1);
		glVertex3f(size, size, 0);
		glTexCoord2f(0, 1);
		glVertex3f(0, size, 0);
		glEnd();

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(0, 0, height);
		glTexCoord2f(1, 0);
		glVertex3f(size, 0, height);
		glTexCoord2f(1, 1);
		glVertex3f(size, size, height);
		glTexCoord2f(0, 1);
		glVertex3f(0, size, height);
		glEnd();

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(0, 0, height);
		glTexCoord2f(1, 0);
		glVertex3f(0, 0, 0);
		glTexCoord2f(1, 1);
		glVertex3f(size, 0, 0);
		glTexCoord2f(0, 1);
		glVertex3f(size, 0, height);
		glEnd();

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(size, size, 0);
		glTexCoord2f(1, 0);
		glVertex3f(size, size, height);
		glTexCoord2f(1, 1);
		glVertex3f(size, 0, height);
		glTexCoord2f(0, 1);
		glVertex3f(size, 0, 0);
		glEnd();

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(size, size, 0);
		glTexCoord2f(1, 0);
		glVertex3f(size, size, height);
		glTexCoord2f(1, 1);
		glVertex3f(0, size, height);
		glTexCoord2f(0, 1);
		glVertex3f(0, size, 0);
		glEnd();

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(0, size, height);
		glTexCoord2f(1, 0);
		glVertex3f(0, size, 0);
		glTexCoord2f(1, 1);
		glVertex3f(0, 0, 0);
		glTexCoord2f(0, 1);
		glVertex3f(0, 0, height);
		glEnd();

		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex3f(0, size, height);
		glTexCoord2f(1, 0);
		glVertex3f(0, size, 0);
		glTexCoord2f(1, 1);
		glVertex3f(0, 0, 0);
		glTexCoord2f(0, 1);
		glVertex3f(0, 0, height);
		glEnd();
		glDisable(GL_LIGHTING);
		glDisable(GL_TEXTURE);
	glPopMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void drawBox(Object* box) {
	if (box->isAlive) {
		createCube(box->xPosition, box->yPosition, entrySize, entrySize, 0);
	}
}

void drawWall(Object* box) {
	createCube(box->xPosition, box->yPosition, entrySize, entrySize, 2);
}

void drawObjects() {
	for (int i = 0; i < mapMatrixWidth; i++) {
		for (int j = 0; j < mapMatrixHeigth; j++) {
			Object* entry = objectsInMap[i][j];
			if (entry != NULL) {
				switch (entry->objectType) {
				case Box:
					drawBox(entry);
					break;
				case Wall:
					drawWall(entry);
					break;
				}
			}
		}
	}
}

bool validMove(float x, float y, bool increaseX, bool decreaseX, bool increaseY, bool decreaseY, float objectSize) {
	//Controles de borde de mapa
	if ((x <= objectSize && decreaseX) ||
		((x >= mapWidth - (objectSize)) && increaseX) ||
		((y >= -objectSize) && increaseY) ||
		((y <= -mapHeigth + (objectSize)) && decreaseY)) {
		return false;
	}


	//Controles de movimiento por objetos
	//Calculo los bordes luego de que se movio
	float borderLeft = x - objectSize;
	float borderRigth = x + objectSize;
	float borderBottom = y - objectSize;
	float borderTop = y + objectSize;

	//Me fijo que ninguno de los cuadrantes este ocupado con altura y miro los bordes derechos e izquierdo y con altura x miro los bordes de arriba y abajo
	const float toCheckSides[4][2] = { {borderLeft, borderTop}, {borderLeft, borderBottom}, {borderRigth, borderTop}, {borderRigth, borderBottom}};

	for (int i = 0; i < 4; i++) {

		float destinationX = toCheckSides[i][0];
		float destinationY = toCheckSides[i][1];

		int xEntry = destinationX / entrySize;
		int yEntry = destinationY / entrySize * -1;
		if (yEntry < 0) {
			yEntry = 0;
		}
		if (xEntry < 0) {
			xEntry = 0;
		}
		if (xEntry > mapMatrixWidth - 1) {
			xEntry = mapMatrixWidth - 1;
		}
		if (yEntry > mapMatrixHeigth - 1) {
			yEntry = mapMatrixHeigth - 1;
		}

		Object* entryObject = objectsInMap[yEntry][xEntry];
		if ((entryObject != NULL) && (entryObject->isAlive) && (entryObject->objectType == Box || entryObject->objectType == Wall)) {
			return false;
		}
	}

	return true;
}

void createBomb() {
	Bomb* bomb = new Bomb();
	
	//Calculo posiciones centradas en un cuadrado de la grilla para poner la bomba
	float xPositionCentered = (int)(player.xPosition / entrySize) * entrySize;
	float yPositionCentered = ((int)(player.yPosition / entrySize)) * entrySize;

	bomb->xPosition = xPositionCentered + entrySize/2;
	bomb->yPosition = yPositionCentered - entrySize/2;
	bomb->duration = player.actualBombDurationInSeconds;
	bomb->explosionStartTime = NULL;
	bomb->eliminatedObjects = false;
	time(&bomb->startTime);
	for (int i = 0; i < player.maxBombsCounter; i++) {
		if (bombs[i] == NULL) {
			bombs[i] = bomb;
			break;
		}
	}
}

void killPlayer() {
	metadata.lifesCount -= 1; 
	player.xPosition = (mapWidth / (float)mapMatrixWidth) / 2; //Centrado en el primer cubo del cuadriculado
	player.yPosition = (-mapHeigth / (float)mapMatrixHeigth) / 2; //Centrado en el primer cubo del cuadriculad
	if (metadata.lifesCount == 0) {
		actualScreen = Menu;
	}
}

void explodeObejctsByBomb(Bomb* bomb) {
	int columnIndex = bomb->xPosition / entrySize;
	int rawIndex = bomb->yPosition / entrySize * -1;
	//Recorro filas y columnas en simultaneo explotando objetos
	bool killedPlayer = false;
	activarSonido(4);
	for (int i = - player.explosionRange; i <= 2 * player.explosionRange; i++) {
		
		//Elimino cajas
		int newXRawIndex = rawIndex - player.explosionRange  + i;
		if (newXRawIndex >= 0 && newXRawIndex <= mapMatrixWidth) {
			Object* rawObject = objectsInMap[newXRawIndex][columnIndex];
			if (rawObject != NULL && rawObject->objectType == Box) {
				if (objectsInMap[newXRawIndex][columnIndex]->isAlive == true) {
					activarSonido(5);
				}
				objectsInMap[newXRawIndex][columnIndex]->isAlive = false;
			}
		}
		int newYColumnIndex = columnIndex - player.explosionRange + i;
		if (newYColumnIndex >= 0 && newYColumnIndex <= mapMatrixHeigth) {
			Object* columnObject = objectsInMap[rawIndex][newYColumnIndex];
			if (columnObject != NULL && columnObject->objectType == Box) {
				if (objectsInMap[rawIndex][newYColumnIndex]->isAlive == true) {
					activarSonido(5);
				}
				objectsInMap[rawIndex][newYColumnIndex]->isAlive = false;
			}
		}

		//Mato enemigos
		for (int j = 0; j < enemiesCount; j++) {
			Enemy enemy = enemies[j];
			if ((abs(bomb->xPosition - enemy.xPosition) < player.explosionRange * entrySize + entrySize/2 
				&& (abs(bomb->yPosition - enemy.yPosition) < entrySize/2)) ||
				((abs(bomb->yPosition - enemy.yPosition) < player.explosionRange * entrySize + entrySize / 2
					&& (abs(bomb->xPosition - enemy.xPosition) < entrySize / 2)))) {
				if (enemies[j].isAlive) {
					activarSonido(6);
					metadata.score += 10;
				}
				enemies[j].isAlive = false;
			}
		}

		//Mato personaje
		if ((abs(bomb->xPosition - player.xPosition) < player.explosionRange * entrySize + entrySize / 2
			&& (abs(bomb->yPosition - player.yPosition) < entrySize / 2)) ||
			((abs(bomb->yPosition - player.yPosition) < player.explosionRange * entrySize + entrySize / 2
				&& (abs(bomb->xPosition - player.xPosition) < entrySize / 2)))) {
			if (!killedPlayer) {
				killPlayer();
				resetEnemiesLocations();
				killedPlayer = true;
			}
		}
	}
}

void drawExplosion(Bomb* bomb) {
	
	glPushMatrix();
		glTranslatef(bomb->xPosition, bomb->yPosition, 0.001f);
		
		//explosion column
		glBegin(GL_POLYGON);
			glColor3f(1, 0, 0);
			glVertex3f(entrySize/2, - entrySize/2 - (player.explosionRange * entrySize), 0);
			glVertex3f(entrySize / 2, entrySize / 2 + (player.explosionRange * entrySize), 0);
			glVertex3f(- entrySize / 2, entrySize / 2 + (player.explosionRange * entrySize), 0);
			glVertex3f(-entrySize / 2, -entrySize / 2 - (player.explosionRange * entrySize), 0);
		glEnd();
		//explosion row
		glBegin(GL_POLYGON);
			glColor3f(1, 0, 0);
			glVertex3f(entrySize / 2 + (player.explosionRange * entrySize), -entrySize / 2, 0);
			glVertex3f(entrySize / 2 + (player.explosionRange * entrySize), +entrySize / 2, 0);
			glVertex3f(-entrySize / 2 - (player.explosionRange * entrySize), +entrySize / 2, 0);
			glVertex3f(-entrySize / 2 - (player.explosionRange * entrySize), -entrySize / 2, 0);
		glEnd();
		glColor3f(1, 1, 1);
	glPopMatrix();
}

void drawBombsAndRemoveIfTimeExpired() {
	for (int i = 0; i < player.maxBombsCounter; i++) {
		if (bombs[i] != NULL) {
			Bomb* bomb = bombs[i];
			time_t maxTime = bomb->startTime + bomb->duration;
			time_t actualTime;
			time(&actualTime);
			if (difftime(actualTime, maxTime) >= 0) {
				if (bomb->explosionStartTime == 0) {
					time(&bomb->explosionStartTime);
					time(&actualTime);
				}
				if (bomb->explosionStartTime != 0 && difftime(actualTime, bomb->explosionStartTime) < 0.05 * metadata.gameVelocity) {
					drawExplosion(bomb);
					if (!bomb->eliminatedObjects) {
						explodeObejctsByBomb(bomb);
						bomb->eliminatedObjects = true;
					}
				}
				//Eliminar si ya deberia haber terminado la explotado
				else {
					bombs[i] = NULL;
				}
			}
			else {
				//Dibujar si no exploto
				glEnable(GL_LIGHTING);
				glPushMatrix();
					glTranslatef(bomb->xPosition, bomb->yPosition, player.size / 2);
					GLUquadricObj* quadratic;
					quadratic = gluNewQuadric();
					glColor3f(0, 0, 0);
					gluSphere(quadratic, player.size / 2, 10, 10);
					glColor3f(1, 1, 1);
				glPopMatrix();
				glDisable(GL_LIGHTING);

			}
		}
	}
}

void cambiarLuz() {
	if (options.luzActivada) {
		options.luzActivada = false;
	}
	else {
		options.luzActivada = true;
	}
}

/*float magnitude(float x, float y, float z) {
	return sqrt(x * x + y * y + z * z);
}

void normalize(float* x, float* y, float* z) {
	float mag = magnitude(*x, *y, *z);
	*x /= mag;
	*y /= mag;
	*z /= mag;
}*/

void cambiarVista() {
	if (actualVista == 0) {
		xCamera = player.xPosition;
		yCamera = player.yPosition;
		zCamera = entrySize * 2;

		xUp = 0;
		yUp = 0;
		zUp = 1;
		degrees = 0;
		actualVista = 1;
	}
	else if (actualVista == 1){
		glTranslatef(12.5/2, -12.5/2, 0);

		xCamera = mapWidth / 2;
		yCamera = -20;
		zCamera = 5;

		xCenter = 12.5 / 2;
		yCenter = -12.5 / 2;
		zCenter = 0;

		xUp = 0;
		yUp = 0;
		zUp = 1;

		actualVista = 2;
	}
	else {
		glTranslatef(12.5/2, -12.5/2, 0);
		xCamera = mapWidth / 2;
		yCamera = -mapHeigth / 2;
		zCamera = 20;

		xCenter = mapWidth / 2;
		yCenter = -mapHeigth / 2;
		zCenter = 0;

		xUp = 0;
		yUp = 1;
		zUp = 0;

		actualVista = 0;
	}
}

void updateCameraPOV() {

	if (actualVista == 1) {
		xCamera = player.xPosition;
		yCamera = player.yPosition;

		xUp = 0;
		yUp = 0;
		zUp = 1;
	}
}

float gradesToRadians(float grades) {
	return grades * M_PI / 200.0;
}

void mouse_callback(double xposIn, double yposIn)
{
	if (actualVista == 1) {
		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = lastX - xpos;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.2f; // change this value to your liking
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		int myInt = static_cast<int>(floor(yaw));

		yawDirection = abs(myInt % 360);

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		xCenter = cos(gradesToRadians(yaw)) * cos(gradesToRadians(pitch));
		yCenter = sin(gradesToRadians(yaw)) * cos(gradesToRadians(pitch));
		zCenter = sin(gradesToRadians(pitch));
	}
	else if (actualVista == 2) {
		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = lastX - xpos;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
		lastX = xpos;
		lastY = ypos;

		float sensitivity = 0.2f; // change this value to your liking
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		int myInt = static_cast<int>(floor(yaw));

		yawDirection = abs(myInt % 360);

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		xCenter = cos(gradesToRadians(yaw)) * cos(gradesToRadians(pitch));
		yCenter = sin(gradesToRadians(yaw)) * cos(gradesToRadians(pitch)); 
		zCenter = sin(gradesToRadians(pitch));
	}
}

bool flag = true; //Este boolean lo agregamos porque estaba entrando dos veces en el while "while (SDL_PollEvent(&evento))" y si bien el movimiento quedaba mejor, nos leía dos veces todo 
                  //y no nos dejaba realizar el cambio de camara. Tambien nos iba a ocasionar problemas con el pausa. Si hay una mejor solución, bien, pero por ahora sirve el uso de esta flag.

float cameraSpeed = 0.5;


void handlePause(SDL_Event& evento) {
	while (SDL_PollEvent(&evento)) {
		switch (evento.type) {
		case SDL_KEYDOWN:
			switch (evento.key.keysym.sym) {
				case SDLK_p:
					if (actualScreen == Play) {
						time_t now;
						time(&now);
						time_t secondsPaused = difftime(now, timeActualPauseInit);
						globalTimeInPause += secondsPaused;
						pause = !pause;
					}
					break;
				}
		}
	}
}

void handleKeyboardAndMouse(SDL_Event &evento) {
	float newXIncreased, newXDecreased, newYIncreased, newYDecreased;

	while (SDL_PollEvent(&evento)) {
		switch (evento.type) {
		case SDL_MOUSEBUTTONDOWN:
			rotateCamera = true;
			cout << "ROT\n";
			break;
		case SDL_MOUSEBUTTONUP:
			rotateCamera = false;
			break;
		case SDL_QUIT:
			if (actualScreen == Play) {
				actualScreen = Menu;
			}
			else {
				fin = true;
			}
			break;
		case SDL_KEYUP:
			switch (evento.key.keysym.sym) {
				case SDLK_ESCAPE:
					desactivarSonido(1);
					if (actualScreen == Play) {
						actualScreen = Menu;
					}
					else {
						fin = true;
					}
					break;
			}
		case SDL_KEYDOWN: 
			if (flag) {
				switch (evento.key.keysym.sym) {
				case SDLK_RIGHT:
					if (actualVista == 1) {
						activarSonido(3);
						if (yawDirection > 315 || yawDirection <= 45) {
							player.orientation = Bottom;
							newYDecreased = player.yPosition - metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
							if (validMove(player.xPosition, newYDecreased, false, false, false, true, player.size / 2)) {
								player.yPosition = newYDecreased;
							}
							break;
						}
						else if (yawDirection > 45 && yawDirection <= 135) {
							player.orientation = Left;
							newXDecreased = player.xPosition - metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
							if (validMove(newXDecreased, player.yPosition, false, true, false, false, player.size / 2)) {
								player.xPosition = newXDecreased;
							}
							break;
						}
						else if (yawDirection > 135 && yawDirection <= 225) {
							player.orientation = Top;
							newYIncreased = player.yPosition + metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
							if (validMove(player.xPosition, newYIncreased, false, false, true, false, player.size / 2)) {
								player.yPosition = newYIncreased;
							}
							break;
						}
						else {
							player.orientation = Rigth;
							newXIncreased = player.xPosition + metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
							if (validMove(newXIncreased, player.yPosition, true, false, false, false, player.size / 2)) {
								player.xPosition = newXIncreased;
							}
							break;
						}
					}
					else if (actualVista == 0) {
						activarSonido(3);
						player.orientation = Rigth;
						newXIncreased = player.xPosition + metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
						if (validMove(newXIncreased, player.yPosition, true, false, false, false, player.size / 2)) {
							player.xPosition = newXIncreased;
						}
						break;
					}
					else {
						xCamera += (yCenter * zUp - zCenter * yUp) * cameraSpeed;
						yCamera += (zCenter * xUp - xCenter * zUp) * cameraSpeed;
						zCamera += (xCenter * yUp - yCenter * xUp) * cameraSpeed;
						break;
					}
				case SDLK_LEFT:
					if (actualVista == 1) {
						activarSonido(3);
						if (yawDirection > 315 || yawDirection <= 45) {
							player.orientation = Top;
							newYIncreased = player.yPosition + metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
							if (validMove(player.xPosition, newYIncreased, false, false, true, false, player.size / 2)) {
								player.yPosition = newYIncreased;
							}
							break;
						}
						else if (yawDirection > 45 && yawDirection <= 135) {

							player.orientation = Rigth;
							newXIncreased = player.xPosition + metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
							if (validMove(newXIncreased, player.yPosition, true, false, false, false, player.size / 2)) {
								player.xPosition = newXIncreased;
							}
							break;
						}
						else if (yawDirection > 135 && yawDirection <= 225) {
							player.orientation = Bottom;
							newYDecreased = player.yPosition - metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
							if (validMove(player.xPosition, newYDecreased, false, false, false, true, player.size / 2)) {
								player.yPosition = newYDecreased;
							}
							break;
						}
						else {
							player.orientation = Left;
							newXDecreased = player.xPosition - metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
							if (validMove(newXDecreased, player.yPosition, false, true, false, false, player.size / 2)) {
								player.xPosition = newXDecreased;
							}
							break;
						}
					}
					else if (actualVista == 0) {
						activarSonido(3);
						player.orientation = Left;
						newXDecreased = player.xPosition - metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
						if (validMove(newXDecreased, player.yPosition, false, true, false, false, player.size / 2)) {
							player.xPosition = newXDecreased;
						}
						break;
					}
					else {
						xCamera -= (yCenter * zUp - zCenter * yUp) * cameraSpeed;
						yCamera -= (zCenter * xUp - xCenter * zUp) * cameraSpeed;
						zCamera -= (xCenter * yUp - yCenter * xUp) * cameraSpeed;
						break;
					}
				case SDLK_UP:
					if (actualScreen == Play) {

						if (actualVista == 1) {
							activarSonido(3);
							if (yawDirection > 315 || yawDirection <= 45) {
								player.orientation = Rigth;
								newXIncreased = player.xPosition + metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
								if (validMove(newXIncreased, player.yPosition, true, false, false, false, player.size / 2)) {
									player.xPosition = newXIncreased;
								}
								break;
							}
							else if (yawDirection > 45 && yawDirection <= 135) {
								player.orientation = Bottom;
								newYDecreased = player.yPosition - metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
								if (validMove(player.xPosition, newYDecreased, false, false, false, true, player.size / 2)) {
									player.yPosition = newYDecreased;
								}
								break;
							}
							else if (yawDirection > 135 && yawDirection <= 225) {
								player.orientation = Left;
								newXDecreased = player.xPosition - metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
								if (validMove(newXDecreased, player.yPosition, false, true, false, false, player.size / 2)) {
									player.xPosition = newXDecreased;
								}
								break;
							}
							else {
								player.orientation = Top;
								newYIncreased = player.yPosition + metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
								if (validMove(player.xPosition, newYIncreased, false, false, true, false, player.size / 2)) {
									player.yPosition = newYIncreased;
								}
								break;
							}
						}
						else if (actualVista == 0) {
							activarSonido(3);
							player.orientation = Top;
							newYIncreased = player.yPosition + metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
							if (validMove(player.xPosition, newYIncreased, false, false, true, false, player.size / 2)) {
								player.yPosition = newYIncreased;
							}
							break;
						}
						else {
							xCamera += cameraSpeed * xCenter;
							yCamera += cameraSpeed * yCenter;
							zCamera += cameraSpeed * zCenter;
							break;
						}
					}
					else if (actualScreen == Options) {
						if (metadata.gameVelocity <= 1.9) {
							metadata.gameVelocity += 0.1;
						}
					}
					break;
				case SDLK_DOWN:
					if (actualScreen == Play) {
						if (actualVista == 1) {
							activarSonido(3);
							if (yawDirection > 315 || yawDirection <= 45) {
								player.orientation = Left;
								newXDecreased = player.xPosition - metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
								if (validMove(newXDecreased, player.yPosition, false, true, false, false, player.size / 2)) {
									player.xPosition = newXDecreased;
								}
								break;
							}
							else if (yawDirection > 45 && yawDirection <= 135) {
								player.orientation = Top;
								newYIncreased = player.yPosition + metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
								if (validMove(player.xPosition, newYIncreased, false, false, true, false, player.size / 2)) {
									player.yPosition = newYIncreased;
								}
								break;
							}
							else if (yawDirection > 135 && yawDirection <= 225) {
								player.orientation = Rigth;
								newXIncreased = player.xPosition + metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
								if (validMove(newXIncreased, player.yPosition, true, false, false, false, player.size / 2)) {
									player.xPosition = newXIncreased;
								}
								break;
							}
							else {
								player.orientation = Bottom;
								newYDecreased = player.yPosition - metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
								if (validMove(player.xPosition, newYDecreased, false, false, false, true, player.size / 2)) {
									player.yPosition = newYDecreased;
								}
								break;
							}
						}
						else if (actualVista == 0) {
							activarSonido(3);
							player.orientation = Bottom;
							newYDecreased = player.yPosition - metadata.gameVelocity * charMovePerFrame * timeBetweenFrames;
							if (validMove(player.xPosition, newYDecreased, false, false, false, true, player.size / 2)) {
								player.yPosition = newYDecreased;
							}
							break;
						}
						else {
							xCamera -= cameraSpeed * xCenter;
							yCamera -= cameraSpeed * yCenter;
							zCamera -= cameraSpeed * zCenter;
							break;
						}
					}
					if (actualScreen == Options) {
						if (metadata.gameVelocity >= 0.2) {
							metadata.gameVelocity -= 0.1;
						}
					}
				case SDLK_p:
					if (actualScreen == Play) {
						pause = !pause;
						time(&timeActualPauseInit);
					}
					break;
				case SDLK_SPACE:
					createBomb();
					break;
				case SDLK_v:
					activarSonido(2);
					cambiarVista();
					break;
				case SDLK_l:
					activarSonido(2);
					cambiarLuz();
					break;
				case SDLK_o:
					activarSonido(2);
					if (actualScreen == Menu || actualScreen == Play) { //Si estoy en el menu tiene efecto
						previousScreen = actualScreen;
						actualScreen = Options;
					}
					else if (actualScreen == Options) {
						actualScreen = previousScreen;
					}
					break;
				case SDLK_j:
					if (actualScreen == Menu) { //Si estoy en el menu tiene efecto
						globalTimeInPause = 0;
						initializeObjectsInMap();
						initializePlayer();
						initializeMetadata();
						desactivarSonido(0);
						activarSonido(2);
						activarSonido(1);
						actualScreen = Play;
					}
					break;
				case SDLK_t:
					activarSonido(2);
					options.applyTextures = !options.applyTextures;
					break;
				case SDLK_w:
					activarSonido(2);
					options.wireframeMode = !options.wireframeMode;
					break;
				case SDLK_f:
					activarSonido(2);
					options.facetadoInterpoladoMode = !options.facetadoInterpoladoMode;
					break;
				}
				flag = false;
			}
			else {
				flag = true;
			}
		case SDL_MOUSEMOTION:
			{
				evento.motion.xrel;
				evento.motion.yrel;
				SDL_GetMouseState(&xMouse, &yMouse);
				mouse_callback(xMouse, yMouse);
			}
		}
	}
}

void loadTextures() {
	//TEXTURA
	const char* archivos[] = {
		"../textures/texturaCaja.jpg",
		"../textures/texturaPasto.jpg",
		"../textures/texturaPared.jpg",
		"../textures/bombermanFace.jpg",
		"../textures/texturaBug.jpg",
		"../textures/MenuPrincipal.jpg",
		"../textures/numbers2.png",
		"../textures/score2.png",
		"../textures/vida.png",
		"../textures/MenuOpciones2.jpg",
		"../textures/finish.png",
		"../Modelo/textures/palette_baseColor.png"
	};

	glGenTextures(texturesCount, textures);

	for (int i = 0; i < texturesCount; i++) {
		const char* rutaArchivo = archivos[i];
		FREE_IMAGE_FORMAT fif = FreeImage_GetFIFFromFilename(rutaArchivo);
		FIBITMAP* bitmap = FreeImage_Load(fif, rutaArchivo);
		bitmap = FreeImage_ConvertTo24Bits(bitmap);
		int w = FreeImage_GetWidth(bitmap);
		int h = FreeImage_GetHeight(bitmap);
		void* datos = FreeImage_GetBits(bitmap);

		glBindTexture(GL_TEXTURE_2D, textures[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_BGR, GL_UNSIGNED_BYTE, datos);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	}
}

void drawPlayer() {
	glPushMatrix();
		
		if (options.wireframeMode) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Modo de alambre
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Modo de relleno solido
		}
		float angle =  getAngle(player.orientation);
		glTranslatef(player.xPosition, player.yPosition, player.size / 2);
		glRotatef(angle, 0, 0, 1);
		GLUquadricObj* quadratic;
		quadratic = gluNewQuadric();
		gluQuadricDrawStyle(quadratic, GLU_FILL);
		glBindTexture(GL_TEXTURE_2D, 4);
		gluQuadricTexture(quadratic, true);
		gluQuadricNormals(quadratic, GLU_SMOOTH);
		gluSphere(quadratic, player.size /2,10,10);
		glColor3f(1, 1, 1);
	
	glPopMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void drawEnemies() {
	
	for (int i = 0; i < enemiesCount; i++) {
		Enemy enemy = enemies[i];
		if (enemy.isAlive) {
			glPushMatrix();
				if (options.wireframeMode) {
					glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Modo de alambre
				}
				else {
					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Modo de relleno solido
				}
				float angle = getAngle(enemy.orientation);
				glTranslatef(enemy.xPosition, enemy.yPosition, enemy.size / 2);
				glRotatef(angle, 0, 0, 1);
				glColor3f(1, 1, 1);
				GLUquadricObj* quadratic;
				quadratic = gluNewQuadric();
				gluQuadricDrawStyle(quadratic, GLU_FILL);
				glBindTexture(GL_TEXTURE_2D, 5);
				gluQuadricTexture(quadratic, true);
				gluQuadricNormals(quadratic, GLU_SMOOTH);
				gluSphere(quadratic, enemy.size/2, 10, 10);
				glColor3f(1, 1, 1);
		glPopMatrix();
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		if (enemy.isAlive && abs(enemy.xPosition - player.xPosition) < entrySize / 2 && abs(enemy.yPosition - player.yPosition) < entrySize / 2) {
			killPlayer();
			resetEnemiesLocations();
		}
	}
}

void randomMoveEnemies() {
	for (int i = 0; i < enemiesCount; i++) {
		Enemy enemy = enemies[i];
		float newX = enemy.xPosition;
		float newY = enemy.yPosition;
		if (enemy.orientation == Rigth) {
			newX = enemy.xPosition + metadata.gameVelocity * enemyMovePerFrame * timeBetweenFrames;
		}
		if (enemy.orientation == Left) {
			newX = enemy.xPosition - metadata.gameVelocity * enemyMovePerFrame * timeBetweenFrames;
		}
		if (enemy.orientation == Bottom) {
			newY = enemy.yPosition - metadata.gameVelocity * enemyMovePerFrame * timeBetweenFrames;
		}
		if (enemy.orientation == Top) {
			newY = enemy.yPosition + metadata.gameVelocity * enemyMovePerFrame * timeBetweenFrames;
		}
		bool validMovement = validMove(newX, newY, enemy.orientation == Rigth, enemy.orientation == Left, enemy.orientation == Top, enemy.orientation == Bottom, enemy.size / 2);
		if (validMovement) {
			enemies[i].xPosition = newX;
			enemies[i].yPosition = newY;
		}
		else {
			int randomOrientationIndex = rand() % 4;
			enemies[i].orientation = (Orientation)randomOrientationIndex;
		}
	}
}

void createFloor() {
	glPushMatrix();
		if (options.wireframeMode) {
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Modo de alambre
		}
		else {
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Modo de relleno solido
		}

		if (options.luzActivada) {
			glEnable(GL_LIGHTING);
		}
		if (options.applyTextures) {
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D, textures[1]);
		}

		glBegin(GL_QUADS);

		//Creamos la base grande, es decir la superficie de pasto. 
		//Quedo la parte de arriba de esa superficie mas grande porque nos dimos cuenta tarde que asi tenia que ser para que los cubos entren bien

		// Base
		glTexCoord2f(0, 0);
		glVertex3f(0, -mapHeigth, 0);
		glTexCoord2f(10, 0);
		glVertex3f(mapWidth, -mapHeigth, 0);
		glTexCoord2f(10, 10);
		glVertex3f(mapWidth, 0, 0);
		glTexCoord2f(0, 10);
		glVertex3f(0, 0, 0);

		glEnd();
		glDisable(GL_TEXTURE);
		glDisable(GL_LIGHTING);
	glPopMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void showHud() {
	int hudWidth = 720;
	int hudHeight = 580;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0.0, hudWidth, 0.0, hudHeight, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	int lifesAux = metadata.lifesCount;

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[8]); //Textura de vidas
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	

	glBegin(GL_QUADS);
	

	while (lifesAux > 0) {
		glTexCoord2f(0, 1);
		glVertex2f((lifesAux - 1) * 50, hudHeight);
		glTexCoord2f(1, 1);
		glVertex2f((lifesAux * 50), hudHeight);
		glTexCoord2f(1, 0);
		glVertex2f((lifesAux * 50), hudHeight - 50);
		glTexCoord2f(0, 0);
		glVertex2f((lifesAux - 1) * 50, hudHeight - 50);
		lifesAux--;
	}

	glEnd();
	glDisable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, textures[6]); //Textura de score:
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	glBegin(GL_QUADS);

	int numWidth = 20;
	int numHeight = 30;

	int scoreAux = metadata.score;
	bool end = false;
	int i = 0;

	while (!end) {
		int digit = scoreAux % 10;
		int mod = (digit - 1) % 5;
		float leftCol = (float)mod * 0.2;
		float topRow = 1;
		if (digit > 5 || digit == 0) {
			topRow = 0.5;
		}
		glTexCoord2f(leftCol, topRow);
		glVertex2f(hudWidth - (numWidth * (i + 1)), hudHeight);
		glTexCoord2f(leftCol + 0.2, topRow);
		glVertex2f(hudWidth - (numWidth * i), hudHeight);
		glTexCoord2f(leftCol + 0.2, topRow - 0.5);
		glVertex2f(hudWidth - (numWidth * i), hudHeight - numHeight);
		glTexCoord2f(leftCol, topRow - 0.5);
		glVertex2f(hudWidth - (numWidth * (i + 1)), hudHeight - numHeight);
		i++;
		if (scoreAux < 10) {
			end = true;
		}
		scoreAux = scoreAux / 10;
	}

	time_t timeAux = metadata.initTime;
	time_t actualTime;
	time(&actualTime);
	time_t timePassed = difftime(actualTime, timeAux);
	time_t timeToDraw = difftime(metadata.totalSecondsOfGame, timePassed);
	int timeToDrawInt = (int)timeToDraw + (int)globalTimeInPause;
	
	if (timeToDrawInt == 0) {
		actualScreen = Menu;
	}
	int j = 10;
	bool end_draw_time = false;
	while (!end_draw_time) {
		int digit = timeToDrawInt % 10;
		int mod = (digit - 1) % 5;
		float leftCol = (float)mod * 0.2;
		float topRow = 1;
		if (digit > 5 || digit == 0) {
			topRow = 0.5;
		}
		glTexCoord2f(leftCol, topRow);
		glVertex2f(hudWidth - (numWidth * (j + 1)), hudHeight);
		glTexCoord2f(leftCol + 0.2, topRow);
		glVertex2f(hudWidth - (numWidth * j), hudHeight);
		glTexCoord2f(leftCol + 0.2, topRow - 0.5);
		glVertex2f(hudWidth - (numWidth * j), hudHeight - numHeight);
		glTexCoord2f(leftCol, topRow - 0.5);
		glVertex2f(hudWidth - (numWidth * (j + 1)), hudHeight - numHeight);
		j++;
		if (timeToDrawInt < 10) {
			end_draw_time = true;
		}
		timeToDrawInt = timeToDrawInt / 10;
	}

	glEnd();
	glDisable(GL_BLEND);

	glBindTexture(GL_TEXTURE_2D, textures[7]); //Textura de puntaje
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	glBegin(GL_QUADS);

	glTexCoord2f(0, 1);
	glVertex2f(hudWidth - (numWidth * (i + 3)), hudHeight);
	glTexCoord2f(1, 1);
	glVertex2f(hudWidth - (numWidth * i), hudHeight);
	glTexCoord2f(1, 0);
	glVertex2f(hudWidth - (numWidth * i), hudHeight - numHeight);
	glTexCoord2f(0, 0);
	glVertex2f(hudWidth - (numWidth * (i + 3)), hudHeight - numHeight);

	glEnd();
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);

	glColor4f(1, 1, 1, 1);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void showMenu(SDL_Event& evento) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(6.75, -5, 112, 6.75, -5, 100, 0, 1, 0);//Posicionamos la camara para que mire al nuevo cuadrado generado con la imagen del menu principal.

	//En primer lugar generamos un rectángulo y le cargamos la textura, que en este caso es el menu principal.
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[5]);
	glBegin(GL_QUADS);

	// Menu principal
	glTexCoord2f(0, 0);
	glVertex3f(0, -10, 100);
	glTexCoord2f(1, 0);
	glVertex3f(13.5, -10, 100);
	glTexCoord2f(1, 1);
	glVertex3f(13.5, 0, 100);
	glTexCoord2f(0, 1);
	glVertex3f(0, 0, 100);

	glEnd();

	glDisable(GL_TEXTURE);

	//MANEJO DE EVENTOS
	handleKeyboardAndMouse(evento);

	//FIN MANEJO DE EVENTOS
	SDL_GL_SwapWindow(win);
}

void drawSelector(float x, float y, bool isEnabled) {
	glPushMatrix();
		glTranslatef(x, y, 102);
		if (isEnabled) {
			glColor3f(0, 128, 0);
		}
		else {
			glColor3f(139, 0, 0);
		}
		GLUquadric* quad = gluNewQuadric();
		gluQuadricDrawStyle(quad, GLU_FILL);
		gluDisk(quad, 0.0, .3, 64, 1);
		gluDeleteQuadric(quad);

	glPopMatrix();
	glColor3f(1, 1, 1);
}

void drawFinishLine() {
	glPushMatrix();
	float xCoord = entrySize * (mapMatrixWidth - 1);
	float yCoord = -entrySize * (mapMatrixHeigth - 1);
	//Me translado hacia donde quiero dibujar el cubo
	glTranslatef(xCoord, yCoord, 0.001f);

	//Habilito las luces y texturas

	if (options.wireframeMode) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // Modo de alambre
	}
	else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Modo de relleno solido
	}

	if (options.facetadoInterpoladoMode) {
		glShadeModel(GL_FLAT); // Facetado
	}
	else {
		glShadeModel(GL_SMOOTH); //Interpolado
	}

	if (options.luzActivada) {
		glEnable(GL_LIGHTING);
	}
	//	glEnable(GL_COLOR_MATERIAL);
	if (options.applyTextures) {
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, textures[10]);
	}
	glColor3f(1, 1, 1);

	glBegin(GL_POLYGON);
	glTexCoord2f(0, 0);
	glVertex3f(0, 0, 0);
	glTexCoord2f(1, 0);
	glVertex3f(entrySize, 0, 0);
	glTexCoord2f(1, 1);
	glVertex3f(entrySize, -entrySize, 0);
	glTexCoord2f(0, 1);
	glVertex3f(0, -entrySize, 0);
	glEnd();

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE);
	glPopMatrix();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if ((abs(player.xPosition - (xCoord + entrySize / 2)) < entrySize / 6) &&
		(abs(player.yPosition - (yCoord - entrySize / 2)) < entrySize / 6)) {
		actualScreen = Menu;
	}
}

void drawGameVelocitiy() {
	
	int velocityToDraw = metadata.gameVelocity/0.1;

	glPushMatrix();
		float yPos = -8;
		float xPos = 9.4;
		
		glTranslatef(xPos, yPos, 102);
		
		float squareHeight = .5;

		glColor3f(0, 0, 0);

		for (int i=0; i< velocityToDraw; i++) {

			float xLeft = (.1 * i);
			float xRigth = (.1 * i) + .08;
			glBegin(GL_QUADS);
				glVertex2f(xLeft, - squareHeight);
				glVertex2f(xRigth, - squareHeight);
				glVertex2f(xRigth, 0);
				glVertex2f(xLeft, 0);
			glEnd();
		}
		glColor3f(1, 1, 1);
	glPopMatrix();
}

void showOptions(SDL_Event& evento) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(6.75, -5, 112, 6.75, -5, 100, 0, 1, 0);//Posicionamos la camara para que mire al nuevo cuadrado generado con la imagen del menu principal.

	//En primer lugar generamos un rectángulo y le cargamos la textura, que en este caso es el menu principal.
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textures[9]);
	
	glBegin(GL_QUADS);

	// Menu principal
	glTexCoord2f(0, 0);
	glVertex3f(0, -10, 100);
	glTexCoord2f(1, 0);
	glVertex3f(13.5, -10, 100);
	glTexCoord2f(1, 1);
	glVertex3f(13.5, 0, 100);
	glTexCoord2f(0, 1);
	glVertex3f(0, 0, 100);

	glEnd();

	glDisable(GL_TEXTURE);
	
	drawGameVelocitiy();
	drawSelector(10, -5.1, options.applyTextures);
	drawSelector(10, -5.9, options.wireframeMode);
	drawSelector(10, -6.7, options.luzActivada);

	drawSelector(9.65, -7.5, options.facetadoInterpoladoMode);

	drawSelector(10.35, -7.5, !options.facetadoInterpoladoMode);

	//MANEJO DE EVENTOS
	handleKeyboardAndMouse(evento);

	//FIN MANEJO DE EVENTOS
	SDL_GL_SwapWindow(win);
}

void showGame(SDL_Event& evento) {
	clearBuffersAndSetLookPoint();
	showHud();
	static std::vector<Vertice> jugador = cargarModelo();
	dibujarModelo(jugador);
	createFloor();
	drawObjects();
	drawFinishLine();
	//drawPlayer();
	drawEnemies();
	drawBombsAndRemoveIfTimeExpired();
	randomMoveEnemies();
	updateCameraPOV();

	//MANEJO DE EVENTOS
	handleKeyboardAndMouse(evento);

	//FIN MANEJO DE EVENTOS
	SDL_GL_SwapWindow(win);
}

void getTimeInMilisecondsAndSaveInParameter(std::chrono::milliseconds::rep &time) {
	auto now = std::chrono::system_clock::now();

	auto ms_since_epoch = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch();

	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ms_since_epoch).count();

	time = ms;
}

void updateTimeBetweenFrames() {
	std::chrono::milliseconds::rep now;
	getTimeInMilisecondsAndSaveInParameter(now);
	timeBetweenFrames = now - lastTimeRegisteredBetweenFrames;
	lastTimeRegisteredBetweenFrames = now;
}

int main(int argc, char* argv[]) {
	initializeWindow();
	activarSonido(0);
	SDL_Event evento;
	loadTextures();
	getTimeInMilisecondsAndSaveInParameter(lastTimeRegisteredBetweenFrames);
	do {
		updateTimeBetweenFrames();

		if (actualScreen == Menu) {
			showMenu(evento);
		}
		else if (actualScreen == Options) {
			showOptions(evento);
		}
		else if(actualScreen == Play) {
			if (!pause) {
				showGame(evento);
			}
			else if (actualScreen == Play) {
				handlePause(evento);
			}
		}
	} while (!fin);
	//FIN LOOP PRINCIPAL
	// LIMPIEZA
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(win);
	if (engine1 != NULL) {
		engine1->drop();
	}
	SDL_Quit();
	return 0;
}