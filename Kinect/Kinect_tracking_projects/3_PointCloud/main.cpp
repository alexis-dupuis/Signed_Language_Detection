// IPC
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")

#include "main.h"
#include "glut.h"

#include <cmath>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <Windows.h>
#include <Ole2.h>

#include <NuiApi.h>
#include <NuiImageCamera.h>
#include <NuiSensor.h>

using namespace std;

//Socket variables
WSADATA wsa;
SOCKET s;

// OpenGL Variables
long depthToRgbMap[width*height*2];

// We'll be using buffer objects to store the kinect point cloud
GLuint vboId;
//GLuint cboId;

// Kinect variables
HANDLE depthStream;
//HANDLE rgbStream;
INuiSensor* sensor;

//Webcam parameters
float fx1 = 1463.97;
float fy1 = 1458.95;

float cx1 = 639.74 * 0.5; //*0.5 ?
float cy1 = 360.19 * 0.5; //*0.5 ?

//Kinect parameters
float fx2 = 527.17;
float fy2 = 528.32;

float cx2 = 319.99;
float cy2 = 239.99;

bool initKinect() {
    // Get a working kinect sensor
    int numSensors;
    if (NuiGetSensorCount(&numSensors) < 0 || numSensors < 1) return false;
    if (NuiCreateSensorByIndex(0, &sensor) < 0) return false;

    // Initialize sensor
    sensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH | NUI_INITIALIZE_FLAG_USES_COLOR);
    sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_DEPTH, // Depth camera or rgb camera?
        NUI_IMAGE_RESOLUTION_640x480,                // Image resolution
        0,        // Image stream flags, e.g. near mode
        2,        // Number of frames to buffer
        NULL,     // Event handle
        &depthStream);
	/*
	sensor->NuiImageStreamOpen(NUI_IMAGE_TYPE_COLOR, // Depth camera or rgb camera?
        NUI_IMAGE_RESOLUTION_640x480,                // Image resolution
        0,      // Image stream flags, e.g. near mode
        2,      // Number of frames to buffer
        NULL,   // Event handle
		&rgbStream);
		*/
    return sensor;
}


// Global Variables
const int nbrJoints = 18;
float joints_vertexarray[nbrJoints * 3];
string openPath;
//int whichJSON = 0;
//...

void getDepthData(GLubyte* dest) {

	//ofstream log;
	//log.open("log.txt");

	/* //Use this if you use a stream of JSON files instead of a single JSON. The stream has to be synched with kinect's frequency...
	stringstream ss;
	ss << whichJSON;
	string str_whichJSON = ss.str(); //conversion of whichJSON in string
	int howManyZeroes = 12 - str_whichJSON.length();


	stringstream ss_openPath; //building the openPath string
	ss_openPath << "C:/Users/Alexis/Documents/GitHub/tf-openpose/output/"; // first the path
	for(int i = 0; i < howManyZeroes; i++) //then we write the zeroes
		ss_openPath << '0';
	ss_openPath << str_whichJSON << "_keypoints.json";*/

	openPath = "C:/Users/Alexis/Documents/GitHub/tf-openpose/output/000000000000_keypoints.json"; //we might open the same openpose frame multiple times in a row but we don't care
	//we might however get issues if openpose re-write the file while this program is reading it...

	ifstream in_JSON; //input JSON stream

	in_JSON.open(openPath.c_str()); //give him the file

	if ( !(in_JSON.is_open()) ) { //check that everything is ok
		return;
	}

	float* fdest = (float*) dest;
	long* depth2rgb = (long*) depthToRgbMap;

	char beginningChar = ' ';
	while(beginningChar != '[') {//throw away the beginning characters
		in_JSON >> beginningChar;
	}

	int joints_coords[nbrJoints * 2];

	for (int i = 0; i < nbrJoints * 2; i++) { //then fill joints_coords

		char currentChar;
		int coord;

		string str_coord;

		in_JSON >> currentChar;
		while ((currentChar != ',') && (currentChar != ']')) {//first we'll store it in a string
			str_coord += currentChar;
			in_JSON >> currentChar; //read the next char
		}

		//then we convert it to an int
		stringstream ss_str_coord(str_coord); // something to stream our string
		ss_str_coord >> coord;

		joints_coords[i] = (int) coord;
	}

	in_JSON.close();

	/////////////VERIFICATION/////////////
	/*ofstream log;
	log.open("log.txt");
	for (int i = 0; i < nbrJoints * 2; i++) {
		log << joints_coords[i] << endl;
	}
	log.close();*/
	/////////////VERIFICATION/////////////


    NUI_IMAGE_FRAME imageFrame;
    NUI_LOCKED_RECT LockedRect;

    if (sensor->NuiImageStreamGetNextFrame(depthStream, 0, &imageFrame) < 0) return;

    INuiFrameTexture* texture = imageFrame.pFrameTexture;
    texture->LockRect(0, &LockedRect, NULL, 0);

    if (LockedRect.Pitch != 0) {

        const USHORT* curr = (const USHORT*) LockedRect.pBits;

		for (int i = 0; i < nbrJoints; i++) {

			int a = joints_coords[2 * i];
			//Reprojection
			float a_conv = fx2/fx1 * ((float) a - cx1) + cx2;
			a = (int) a_conv;

			int b = joints_coords[2 * i + 1];
			//Reprojection
			float b_conv = fy2 / fy1 * ((float) b - cy1) + cy2;
			b = (int) b_conv;

			const USHORT* depth_ind = curr + a + b * 640;

			USHORT depth = NuiDepthPixelToDepth(*depth_ind);
			Vector4 pos = NuiTransformDepthImageToSkeleton(a, b, depth << 3, NUI_IMAGE_RESOLUTION_640x480);

			joints_vertexarray[3 * i] = (float)( pos.x / pos.w);
			joints_vertexarray[3 * i + 1] = (float)(pos.y / pos.w);
			joints_vertexarray[3 * i + 2] = (float)(pos.z / pos.w);

			//log << joints_vertexarray[3 * i]  << " " << joints_vertexarray[3 * i + 1] << " " << joints_vertexarray[3 * i + 2] << endl;


		}

        for (int j = 0; j < height; ++j) {
			for (int i = 0; i < width; ++i) {

				// Get depth of pixel in millimeters
				USHORT depth = NuiDepthPixelToDepth(*curr++);

				// Store coordinates of the point corresponding to this pixel
				Vector4 pos = NuiTransformDepthImageToSkeleton(i, j, depth<<3, NUI_IMAGE_RESOLUTION_640x480);
				*fdest++ = pos.x/pos.w;
				*fdest++ = pos.y/pos.w;
				*fdest++ = pos.z/pos.w;

				// Store the index into the color array corresponding to this pixel
				/*NuiImageGetColorPixelCoordinatesFromDepthPixelAtResolution(
					NUI_IMAGE_RESOLUTION_640x480, NUI_IMAGE_RESOLUTION_640x480, NULL,
					i, j, depth<<3, depth2rgb, depth2rgb+1);

				depth2rgb += 2;*/
			}
		}

		//log.close();

    }
    texture->UnlockRect(0);
    sensor->NuiImageStreamReleaseFrame(depthStream, &imageFrame);

}
/*
void getRgbData(GLubyte* dest) {
	float* fdest = (float*) dest;
	long* depth2rgb = (long*) depthToRgbMap;
	NUI_IMAGE_FRAME imageFrame;
    NUI_LOCKED_RECT LockedRect;
    if (sensor->NuiImageStreamGetNextFrame(rgbStream, 0, &imageFrame) < 0) return;
    INuiFrameTexture* texture = imageFrame.pFrameTexture;
    texture->LockRect(0, &LockedRect, NULL, 0);
    if (LockedRect.Pitch != 0) {
        const BYTE* start = (const BYTE*) LockedRect.pBits;
        for (int j = 0; j < height; ++j) {
			for (int i = 0; i < width; ++i) {
				// Determine rgb color for each depth pixel
				long x = *depth2rgb++;
				long y = *depth2rgb++;
				// If out of bounds, then don't color it at all
				if (x < 0 || y < 0 || x > width || y > height) {
					for (int n = 0; n < 3; ++n) *(fdest++) = 0.0f;
				}
				else {
					const BYTE* curr = start + (x + width*y)*4;
					for (int n = 0; n < 3; ++n) *(fdest++) = curr[2-n]/255.0f;
				}

			}
		}
    }
    texture->UnlockRect(0);
    sensor->NuiImageStreamReleaseFrame(rgbStream, &imageFrame);
}
*/
void getKinectData() {
	GLubyte* ptr;

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	ptr = (GLubyte*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (ptr) {
		getDepthData(ptr);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	/*
	glBindBuffer(GL_ARRAY_BUFFER, cboId);
	ptr = (GLubyte*) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
	if (ptr) {
		getRgbData(ptr);
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);*/
}

void rotateCamera() {
	static double angle = 0.;
	static double radius = 3.;
	double x = radius*sin(angle);
	double z = radius*(1-cos(angle)) - radius/2;
	glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	gluLookAt(x,0,z,0,0,radius/2,0,1,0);
	angle += 0.0;
}




void publishData(const char* message) {

	//Send some data
	//char *message = "Hello From client";
	char server_reply[2000];
	int recv_size;

	if (send(s, message, strlen(message), 0) < 0)
	{
		std::cout << "Send failed" << std::endl;
		return;
	}
	//std::cout << "Data Send" << std::endl;

	//Receive a reply from the server
	if ((recv_size = recv(s, server_reply, 2000, 0)) == SOCKET_ERROR)
	{
		std::cout << "recv failed" << std::endl;
	}

	//std::cout << "Reply received" << std::endl;

	//Add a NULL terminating character to make it a proper string before printing
	server_reply[recv_size] = '\0';
	//std::cout << server_reply << std::endl;
}




void drawKinectData() {
	getKinectData();


	/* //Output result in a file
	ofstream log;
	log.open("log.json");
	for (int i = 0; i < nbrJoints * 3; i++) {
		log << joints_vertexarray[i] << endl;
	}
	log.close();*/

	//Publish data

	for (int i = 0; i < nbrJoints; i++) {
		std::string str_message;
		std::stringstream str_message_iss(str_message);

		str_message_iss << i
			<< " " << joints_vertexarray[i]
			<< " " << joints_vertexarray[i + 1]
			<< " " << joints_vertexarray[i + 2];

		str_message = str_message_iss.str();

		const char* message = str_message.c_str();

		publishData(message);
	}

	/*
	rotateCamera();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//############################################################
	glColor3f(255, 0, 0); //red pixels at joints position

	//glBegin(GL_POINTS);
	for (int i = 0; i < nbrJoints; ++i) {

		if (joints_vertexarray[i * 3] + joints_vertexarray[i * 3 + 1] + joints_vertexarray[i * 3 + 2] != 0.0) {

			GLUquadric *quad;
			quad = gluNewQuadric();
			glTranslatef(joints_vertexarray[i * 3], joints_vertexarray[i * 3 + 1], joints_vertexarray[i * 3 + 2]);
			gluSphere(quad, 0.05, 10, 10);
			glTranslatef(-joints_vertexarray[i * 3], -joints_vertexarray[i * 3 + 1], -joints_vertexarray[i * 3 + 2]);
			//glVertex3f(joints_vertexarray[i * 3], joints_vertexarray[i * 3 + 1], joints_vertexarray[i * 3 + 2]);
		}
	}
	//glEnd();

	glColor3f(0, 0, 255); //blue lines
	for (int i = 0; i < nbrJoints - 1; ++i) { // a line between each joint and the next one

		int j = i + 1;

		if ( (joints_vertexarray[i * 3] + joints_vertexarray[i * 3 + 1] + joints_vertexarray[i * 3 + 2]) != 0.0 &&
			 (joints_vertexarray[j * 3] + joints_vertexarray[j * 3 + 1] + joints_vertexarray[j * 3 + 2]) != 0.0 ) {//don't consider points at 0,0,0

			glBegin(GL_LINES);
			glVertex3f(joints_vertexarray[i * 3], joints_vertexarray[i * 3 + 1], joints_vertexarray[i * 3 + 2]);
			glVertex3f(joints_vertexarray[j * 3], joints_vertexarray[j * 3 + 1], joints_vertexarray[j * 3 + 2]);
			glEnd();
		}

	}
	//############################################################
	glColor3f(50, 20, 50);

	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_COLOR_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	//glBindBuffer(GL_ARRAY_BUFFER, cboId);
	//glColorPointer(3, GL_FLOAT, 0, NULL);

	glPointSize(1.f);
	glDrawArrays(GL_POINTS, 0, width*height);

	glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_COLOR_ARRAY);
	*/
}

int main(int argc, char* argv[]) {
    if (!init(argc, argv)) return 1;
    if (!initKinect()) return 1;



	// Socket
	const char * cIP = "130.66.204.217";//"192.168.43.236"; //"192.168.1.124";

	struct sockaddr_in server;
	char *message, server_reply[2000];
	int recv_size;

	//Initilasing the winsock libraray
	std::cout << "Initialising Winsock..." << std::endl;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		std::cout << "Failed. Error Code " << WSAGetLastError() << std::endl;

		return 1;
	}
	std::cout << "Initialised." << std::endl;

	//Creating socket
	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		std::cout << "Could not create socket beacuse of " << WSAGetLastError() << std::endl;
	}
	std::cout << "Socket created" << std::endl;

	//Local server
	server.sin_addr.s_addr = inet_addr(cIP);
	server.sin_family = AF_INET;
	server.sin_port = htons(12800);


	//Connect to remote server
	if (connect(s, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		std::cout << "connect error" << std::endl;
		return 1;
	}

	std::cout << "Connected" << std::endl;

	//Send some data
	message = "Hello From client";
	if (send(s, message, strlen(message), 0) < 0)
	{
		std::cout << "Send failed" << std::endl;
		return 1;
	}
	std::cout << "Data Send" << std::endl;

	//Receive a reply from the server
	if ((recv_size = recv(s, server_reply, 2000, 0)) == SOCKET_ERROR)
	{
		std::cout << "recv failed" << std::endl;
	}

	std::cout << "Reply received" << std::endl;

	//Add a NULL terminating character to make it a proper string before printing
	server_reply[recv_size] = '\0';
	std::cout << server_reply << std::endl;




    // OpenGL setup
    glClearColor(0,0,0,0);
    glClearDepth(1.0f);

	// Set up array buffers
	const int dataSize = width*height * 3 * 4;
	glGenBuffers(1, &vboId);
	glBindBuffer(GL_ARRAY_BUFFER, vboId);
	glBufferData(GL_ARRAY_BUFFER, dataSize, 0, GL_DYNAMIC_DRAW);
	//glGenBuffers(1, &cboId);
	//glBindBuffer(GL_ARRAY_BUFFER, cboId);
	//glBufferData(GL_ARRAY_BUFFER, dataSize, 0, GL_DYNAMIC_DRAW);

    // Camera setup
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	gluPerspective(45, width /(GLdouble) height, 0.1, 1000);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
	gluLookAt(0,0,0,0,0,1,0,1,0);

    // Main loop
    execute();
    return 0;
}
