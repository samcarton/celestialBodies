// Celestial Body Gravity
// Exploring multi-body gravitation effects in the vacuum of space.
// Sam Carton 31510651
// Created 29 April 2012
// Submitted Version 21 May 2012
// Updated Version 20 June 2013
// This main file is way too big :)

#include <stdio.h>
#include <GL/freeglut.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "VecOps.h"
#include "loadTexture.h"

// Constant values
const GLdouble accelDrawFac = 0.3; // scale factor for acceleration vector line
const GLdouble PI = 3.14159265;
const GLdouble EARTHMASS = 60;
const GLdouble EARTHORBIT = 1500;
const GLdouble EARTHRAD = 60;//60
const GLdouble EARTHVEL = 120;

// Create vector type
typedef GLdouble vec3f[3];

// 0 to disable gravity effects
static GLdouble GCONST = 1.0;

// trail drawing variable
static unsigned long frameNum = 0;

// Timing Variables
static clock_t startT, prevT, nowT;
static GLdouble delT;
static int firstRunT = 1;
static GLdouble timeFact = 1.0;

//define sphereBody struct type
typedef struct
{
    GLdouble radius;
    GLdouble mass;
    vec3f pos;
    vec3f vel;
    vec3f accel;
    vec3f trail[100]; // if changing this remember to change the const int below too
    int trailLength; // length of trail array, to track if it is full or not
    int trailStart; // trail start index, to allow for wrap-around looping
    GLuint texture; // hold texture ID
    int draw; // state holder for turning spheres on/off
    clock_t collideTime; //time of last collision
    float rotPos; // current angle of rotation
    float rotVel; // angular velocity of rotation
    int axialTilt; // planetary tilt from vertical
} sphereBody;

// camera struct
typedef struct
{
    vec3f pos; // camera position, relative to target
    vec3f posOffset; //camera position with target offset, so orbit centred on target
    vec3f lookAt; // point camera is looking at
    vec3f lookVec; // vector pointing in direction of camera view (lookat - pos)
} camera;

// still need the 100 literal above (sphereBody struct) to be allowed to make the array.
const int TRAILARRAYSIZE = 100; //trail array size used for looping through trail array

// Texture ID array
static GLuint sBoxTex[6]; // Skybox texture id array

//Splash Screen Textures
static GLuint splashTex[2]; //Start and Quit Screens
GLfloat splashSquareVerts[4][3] = {{0.0,0.0,0.0}, {500.0,0.0,0.0}, {500.0,500.0,0.0}, {0.0,500.0,0.0}  };

//create sphereBody variables
sphereBody* targetSphere; //for tracking target - camera and modifications
sphereBody bodies[11]; // array of sphereBodies - planets

//create camera variables
camera cam = {.pos ={0.0, 0.0, 0.0}, .posOffset ={0.0, 0.0, 0.0}, .lookAt = {0.0, 0.0, 0.0}, .lookVec = {0.0, 0.0, 0.0} };
static GLdouble cameraPhi = 1.5707, cameraTheta = 0.0; //theta angle around y axis, phi angle of inclination
static GLdouble deltaPhi = 0.0, deltaTheta = 0.0, tempPhi = 0.0, tempTheta = 0.0; // for tracking mouse mvmt
static GLdouble camZoom = 1000.0; //default camera distance

//mouse movement variables
int xOrigin = -1;
int yOrigin = -1;


//Menu State Variables
static GLint START = 0; //start/stop
enum {TITLE, SIM, QUITSPLASH} state = TITLE; // State of program enum

// forward declaration of collision detection subroutine
void elasticColl2(sphereBody* obj1, sphereBody* obj2);

///
/// CalcSphereMvmt
/// \brief Takes the specified sphereBody, updates its velocity
/// due to acceleration, and then updates its position due to velocity.
///
/// \param pointer to sphereBody obj to update
///
void calcSphereMvmt(sphereBody* obj)
{
    vec3f tempVec;
    vecScale(obj->accel,delT,tempVec); // accel*t
    vecAdd(obj->vel, tempVec, obj->vel); // vel + accel*t
    vecScale(obj->vel,delT,tempVec); //vel*t
    vecAdd(obj->pos, tempVec, obj->pos); // pos + vel*t
    obj->rotPos = obj->rotPos + obj->rotVel*delT; //update rotation
}

///
/// resetAccel
/// \brief Sets the acceleration of all sphereBody variables
/// to zero, in order to begin summing the accelerations due to gravity
/// from all the other sphereBody variables.
///
void resetAccel()
{
    int i=0;
    for(i=0;i<11;i++)
    {
        bodies[i].accel[0] = 0.0;
        bodies[i].accel[1] = 0.0;
        bodies[i].accel[2] = 0.0;
    }
}

///
/// gravityAccel2
/// \brief Calculates and updates the accelerations due to the gravity between
/// two sphereBody variables. Uses Newton's law of universal gravitation to calculate
/// acceleration based on masses and mass separation.
/// As it calculates the separation between the two spheres, it is also convenient
/// to call the collision resolution function if a collision is detected.
///
/// \param obj1 pointer to first sphereBody
/// \param obj2 pointer to second sphereBody
///
void gravityAccel2(sphereBody* obj1, sphereBody* obj2)
{
    vec3f r12u = {0.0,0.0,0.0}, r12 = {0.0,0.0,0.0}, r21u = {0.0,0.0,0.0};
    GLdouble r12Mag = 0.0;

    //compute r12 vector : pos2 - pos1
    vecMinus(obj2->pos, obj1->pos, r12);

    //compute magnitude of r12 vector
    r12Mag = vecMag(r12);

    //printf("\nr12 Vector Mag:%f",r12Mag);

    //check for collisions
    if(r12Mag <= (obj1->radius + obj2->radius)*1.1 && (clock()-obj1->collideTime)/(GLdouble)CLOCKS_PER_SEC > 0.2 && (clock()-obj2->collideTime)/(GLdouble)CLOCKS_PER_SEC > 0.2 )
    {
        elasticColl2(obj1, obj2);
    }

    vecCopy(r12,r12u);

    //obj2 accel .pow is to power3 to make it unit vector.
    vecScale(r12u, -GCONST * (obj1->mass)/pow(r12Mag,3),r12u);
    //add to current accel
    vecAdd(r12u, obj2->accel, obj2->accel);

    //obj1 accel
    vecCopy(r12, r21u);
    vecScale(r21u, GCONST * (obj2->mass)/pow(r12Mag,3),r21u);
    //add to accel
    vecAdd(r21u, obj1->accel, obj1->accel);




}

///
/// elasticColl2
/// \brief Calculates how the velocity vectors of two colliding spheres
/// will respond to a collision. Updates velocities of the two sphereBody
/// variables and also sets each sphereBody's collideTime variable.
///
/// \param obj1 pointer to first sphereBody
/// \param obj2 pointer to second sphereBody
///
void elasticColl2(sphereBody* obj1, sphereBody* obj2)
{
    vec3f axis, U1x, U1xM, U1y, U2x, U2xM, U2y, V1x, V2x, c, d, e;
    GLdouble a, b;

    // obj1
    vecMinus(obj2->pos, obj1->pos, axis);
    vecScale(axis, 1/(vecMag(axis)), axis); // make unit
    a = vecDot(axis, obj1->vel);
    vecScale(axis, a, U1x);
    vecMinus(obj1->vel, U1x, U1y);

    // obj2
    vecMinus(obj1->pos, obj2->pos, axis);
    vecScale(axis, 1/(vecMag(axis)), axis); // make unit
    b = vecDot(axis, obj2->vel);
    vecScale(axis, b, U2x);
    vecMinus(obj2->vel, U2x, U2y);

    //Work out new vector components
    vecScale(U1x, obj1->mass, U1xM);
    vecScale(U2x, obj2->mass, U2xM);

    vecAdd(U1xM, U2xM, c);
    vecMinus(U1x, U2x, d);
    vecScale(d, obj2->mass, d);
    vecMinus(c, d, e);
    vecScale(e, 1/(obj1->mass + obj2->mass),V1x);

    vecAdd(U1xM, U2xM, c);
    vecMinus(U2x, U1x, d);
    vecScale(d, obj1->mass, d);
    vecMinus(c, d, e);
    vecScale(e, 1/(obj1->mass + obj2->mass),V2x);

    // dont need V_y, as V_y=U_y (_ means 1 or 2)
    vecAdd(V1x, U1y, obj1->vel);
    vecAdd(V2x, U2y, obj2->vel);

    obj1->collideTime = clock();
    obj2->collideTime = clock();

}

///
/// addTrail
/// \brief Adds a trail vertex position to the supplied sphereBody's trail array, based on current position.
/// Utilises wrap-around functionality to be able to keep a continuous record of
/// the sphereBody's previous positions within a limited size array.
///
/// \param obj pointer to sphereBody variable which the new trail entry is to be added.
///
void addTrail(sphereBody* obj)
{

    if(obj->trailLength < TRAILARRAYSIZE)
    {
        vecCopy(obj->pos,obj->trail[obj->trailLength]) ;
        obj->trailLength++;
    }
    else    //once full the first time
    {
        vecCopy(obj->pos, obj->trail[obj->trailStart]);

        obj->trailStart = (obj->trailStart +1) % TRAILARRAYSIZE;
    }


}

///
/// drawTrail
/// \brief Draws the sphereBody's trail, using the points found in the sphereBody's
/// trail array. Drawn as a simple line strip of connected points.
/// trailStart variable allows for correct wrap-around drawing of the entire array, starting
/// from the correct point.
///
/// \param obj1 pointer to sphereBody variable.
///
void drawTrail(sphereBody* obj1)
{
    int i = 0;
    glPushMatrix();
        glColor4f(0.3, 0.3, 0.3, 0.3); // alpha has no effect, blending off
        glBegin(GL_LINE_STRIP);

        for(i = obj1->trailStart; i< obj1->trailLength; i++)
        {
                glVertex3dv(obj1->trail[i]);
        }

        for(i = 0; i< obj1->trailStart; i++)
        {
                glVertex3dv(obj1->trail[i]);
        }

        glEnd();

    glPopMatrix();
}

///
/// drawSphereBody
/// \brief Draws the sphereBody variable, based on its position, and textures it with
/// the appropriate texture. Also draws the line representing the acceleration vector.
///
/// \param obj1 pointer to sphereBody variable.
///
void drawSphereBody(sphereBody* obj1)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, obj1->texture);

    glPushMatrix();
        glColor3f (1.0, 1.0, 1.0);
        glTranslatef(obj1->pos[0], obj1->pos[1], obj1->pos[2]);

            //begin quadric sphere rendering
        GLUquadric *qsph1 = gluNewQuadric();
        gluQuadricTexture(qsph1,GLU_TRUE);
        gluQuadricDrawStyle(qsph1, GLU_FILL);
        gluQuadricNormals(qsph1, GLU_SMOOTH);
        gluQuadricOrientation(qsph1, GLU_OUTSIDE); //normal orientation pointing out
        glRotatef((GLfloat)obj1->axialTilt,1,0,0);
        glRotatef(obj1->rotPos,0,1,0); //rotation around axis
        glRotatef(90.0,1,0,0); // make it so y is up
        gluSphere(qsph1,obj1->radius,50,44); //quadric, radius, slices, stacks

        gluDeleteQuadric(qsph1);


        //draw acceleration vector line
        glColor3f(0.9,0.5,0.5);
        glBegin(GL_LINES);
            glVertex3d(0.0,0.0,0.0);
            glVertex3d(obj1->accel[0]*accelDrawFac,obj1->accel[1]*accelDrawFac,obj1->accel[2]*accelDrawFac);
        glEnd();

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
    drawTrail(obj1);

}

///
/// drawSkyBox
/// \brief Draws the skybox, and ensures its correct rotation about the camera to match the camera's rotation.
/// For this to occur, the camera "lookVec" vector must be calculated somewhere.
/// textures are applied using textureIDs fond in a matrix called sBoxTex[6].
///
void drawSkyBox(void)
{
    glPushMatrix();
    glLoadIdentity();

    // need to make this so it only rotates when the actual camera rotates. no translation.
    // maybe have a vector for cameraPos and cameraLookAt, then take the difference and make that the glulookat
    gluLookAt (0.0, 0.0, 0.0,   cam.lookVec[0], cam.lookVec[1], cam.lookVec[2],   0.0, 1.0, 0.0);

    //enable and disable features
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_BLEND);

    //set to black just in case
    glColor3f(1.0,1.0,1.0);

    //front quad
    glBindTexture(GL_TEXTURE_2D, sBoxTex[0]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex3f(0.5, -0.5, -0.5);
        glTexCoord2f(1,0);
        glVertex3f(-0.5, -0.5, -0.5);
        glTexCoord2f(1,1);
        glVertex3f(-0.5, 0.5, -0.5);
        glTexCoord2f(0,1);
        glVertex3f(0.5, 0.5, -0.5);
    glEnd();

    //left quad
    glBindTexture(GL_TEXTURE_2D, sBoxTex[1]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex3f(0.5, -0.5, 0.5);
        glTexCoord2f(1,0);
        glVertex3f(0.5, -0.5, -0.5);
        glTexCoord2f(1,1);
        glVertex3f(0.5, 0.5, -0.5);
        glTexCoord2f(0,1);
        glVertex3f(0.5, 0.5, 0.5);
    glEnd();

    //back quad
    glBindTexture(GL_TEXTURE_2D, sBoxTex[2]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex3f(-0.5, -0.5, 0.5);
        glTexCoord2f(1,0);
        glVertex3f(0.5, -0.5, 0.5);
        glTexCoord2f(1,1);
        glVertex3f(0.5, 0.5, 0.5);
        glTexCoord2f(0,1);
        glVertex3f(-0.5, 0.5, 0.5);
    glEnd();

    //right quad
    glBindTexture(GL_TEXTURE_2D, sBoxTex[3]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex3f(-0.5, -0.5, -0.5);
        glTexCoord2f(1,0);
        glVertex3f(-0.5, -0.5, 0.5);
        glTexCoord2f(1,1);
        glVertex3f(-0.5, 0.5, 0.5);
        glTexCoord2f(0,1);
        glVertex3f(-0.5, 0.5, -0.5);
    glEnd();

    //top quad
    glBindTexture(GL_TEXTURE_2D, sBoxTex[4]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex3f(-0.5, 0.5, -0.5);
        glTexCoord2f(1,0);
        glVertex3f(-0.5, 0.5, 0.5);
        glTexCoord2f(1,1);
        glVertex3f(0.5, 0.5, 0.5);
        glTexCoord2f(0,1);
        glVertex3f(0.5, 0.5, -0.5);
    glEnd();


    //bottom quad
    glBindTexture(GL_TEXTURE_2D, sBoxTex[5]);
    glBegin(GL_QUADS);
        glTexCoord2f(0,0);
        glVertex3f(-0.5, -0.5, -0.5);
        glTexCoord2f(1,0);
        glVertex3f(-0.5, -0.5, 0.5);
        glTexCoord2f(1,1);
        glVertex3f(0.5, -0.5, 0.5);
        glTexCoord2f(0,1);
        glVertex3f(0.5, -0.5, -0.5);
    glEnd();

    glPopAttrib();
    glPopMatrix();

}



///
/// animate
/// \brief Calculates new accelerations, velocities and positions, updates variables accordingly.
/// Adds to trail arrays if required. Calculates and updates new camera position.
///
void animate(void)
{
    if(state == SIM) // in simulation mode
    {

        if(START) //menu control, allows for camera movement
        {
            frameNum++;

            //calculate and update accelerations and collision,
            //update position, velocity, and accel vectors
            resetAccel(); // reset accelerations to zero, to begin summing accelerations again.
            int i=0, j=0;
            for(i=0; i<10; i++) //calc accel for all enabled planets
            {
                for(j=i+1; j<11; j++)
                {
                    if(bodies[i].draw == 1 && bodies[j].draw == 1)
                    {
                        gravityAccel2(&(bodies[i]), &(bodies[j]) );
                    }
                }
            }

            for(i=0;i<11; i++) //update all active bodies vel and position
            {
                if(bodies[i].draw == 1)
                {
                    calcSphereMvmt(&(bodies[i]));
                }
            }

            //add to trail arrays
            if(frameNum%5==0)
            {
                for(i=0;i<11; i++)
                {
                    if(bodies[i].draw == 1)
                    {
                        addTrail(&(bodies[i]));
                    }
                }

            }
        }

        //Camera updates
        //Orbit around target
        //Basically conversion from spherical coords to cartesian, with an offset of planet position
        cam.pos[0] = camZoom*sin(cameraTheta)*sin(cameraPhi);
        cam.pos[1] = camZoom*cos(cameraPhi);
        cam.pos[2] = camZoom*cos(cameraTheta)*sin(cameraPhi);
        vecCopy((*targetSphere).pos, cam.lookAt);
        vecAdd((*targetSphere).pos, cam.pos, cam.posOffset);

        // camera angle for correct skybox orientation
        vecMinus(cam.lookAt, cam.posOffset, cam.lookVec);

    } // end SIM mode animation block


    glutPostRedisplay();
}

///
/// resetBodies
/// \brief Initialise all hard-coded data for the different planets.
/// Also used when the user calls Reset from the menu.
///
void resetBodies(void)
{
    int i = 0;
    for(i=0; i<11; i++)
    {
        bodies[i].trailLength = 0;
        bodies[i].trailStart = 0;
        bodies[i].draw = 1;
        bodies[i].accel[0] = 0.0;
        bodies[i].accel[1] = 0.0;
        bodies[i].accel[2] = 0.0;
        bodies[i].pos[0] = 0.0;
        bodies[i].pos[1] = 0.0;
        bodies[i].pos[2] = 0.0;
        bodies[i].vel[0] = 0.0;
        bodies[i].vel[1] = 0.0;
        bodies[i].vel[2] = 0.0;
        bodies[i].collideTime = 0;
        bodies[i].rotPos = 0.0;
    }

    // Mercury
    bodies[0].radius = 0.4*EARTHRAD;
    bodies[0].mass = 0.055*EARTHMASS;
    bodies[0].pos[0] = 0.4*EARTHORBIT;
    bodies[0].pos[1] = 100;
    bodies[0].vel[2] = -1.6*EARTHVEL;
    bodies[0].rotVel = 0.003*360*40000/(2440.0*2*PI);
    bodies[0].axialTilt = 0;

    // Venus
    bodies[1].radius = 0.95*EARTHRAD;
    bodies[1].mass = 0.81*EARTHMASS;
    bodies[1].pos[0] = 0.7*EARTHORBIT * cos(60/180.0*PI);
    bodies[1].pos[2] = -0.7*EARTHORBIT * sin(60/180.0*PI);
    bodies[1].pos[1] = -65.0;
    bodies[1].vel[0] = -1.176*EARTHVEL * sin(60/180.0*PI);
    bodies[1].vel[2] = -1.176*EARTHVEL * cos(60/180.0*PI);
    bodies[1].rotVel = 0.001*360*40000/(6051*2.0*PI);
    bodies[1].axialTilt = 177;

    // Earth
    bodies[2].radius = EARTHRAD;
    bodies[2].mass = EARTHMASS;
    bodies[2].pos[0] = EARTHORBIT  * cos(120/180.0*PI);
    bodies[2].pos[2] = -1* EARTHORBIT * sin(120/180.0*PI);
    bodies[2].vel[0] = -EARTHVEL * sin(120/180.0*PI);
    bodies[2].vel[2] = -EARTHVEL * cos(120/180.0*PI);
    bodies[2].rotVel = 0.0167*10000;
    bodies[2].axialTilt = 23;

    // Mars
    bodies[3].radius = 0.53*EARTHRAD;
    bodies[3].mass = 0.1*EARTHMASS;
    bodies[3].pos[0] = -1.5*EARTHORBIT;
    bodies[3].pos[1] = 22.0;
    bodies[3].vel[2] = 0.8085*EARTHVEL;
    bodies[3].rotVel = 0.241*360*40000/(3396*2.0*PI);
    bodies[3].axialTilt = 25;

    // Jupiter
    bodies[4].radius = 11.2*EARTHRAD; //11.2
    bodies[4].mass = 318*EARTHMASS;
    bodies[4].pos[0] = 5.2*EARTHORBIT * cos(240/180.0 * PI);
    bodies[4].pos[2] = -5.2*EARTHORBIT * sin(240/180.0 * PI);
    bodies[4].pos[1] = 130.0;
    bodies[4].vel[0] = -0.4389*EARTHVEL * sin(240/180.0 * PI);
    bodies[4].vel[2] = -0.4389*EARTHVEL * cos(240/180.0 * PI);
    bodies[4].rotVel = 0.013*360*40000/(69911*2.0*PI);
    bodies[4].axialTilt = 3;

    // Saturn
    bodies[5].radius = 9.45*EARTHRAD;
    bodies[5].mass = 95*EARTHMASS;
    bodies[5].pos[0] = 9.69*EARTHORBIT * cos(300/180.0 * PI);
    bodies[5].pos[2] = -9.69*EARTHORBIT * sin(300/180.0 * PI);
    bodies[5].pos[1] = -166.0;
    bodies[5].vel[0] = -0.3254*EARTHVEL * sin(300/180.0 * PI);
    bodies[5].vel[2] = -0.3254*EARTHVEL * cos(300/180.0 * PI);
    bodies[5].rotVel = 0.010*360*40000/(60268*2.0*PI);
    bodies[5].axialTilt = 27;

    // Uranus
    bodies[6].radius = 4.01*EARTHRAD;
    bodies[6].mass = 14*EARTHMASS;
    bodies[6].pos[0] = 19.6*EARTHORBIT;
    bodies[6].pos[1] = 40.0;
    bodies[6].vel[2] = -0.2287*EARTHVEL;
    bodies[6].rotVel = 0.003*360*40000/(26000*2.0*PI);
    bodies[6].axialTilt = 98;

    // Neptune
    bodies[7].radius = 3.88*EARTHRAD;
    bodies[7].mass = 17*EARTHMASS;
    bodies[7].pos[0] = 30*EARTHORBIT * cos(60/180.0*PI);
    bodies[7].pos[2] = -30*EARTHORBIT * sin(60/180.0*PI);
    bodies[7].pos[1] = -113.0;
    bodies[7].vel[0] = -0.1823*EARTHVEL * sin(60/180.0*PI);
    bodies[7].vel[2] = -0.1823*EARTHVEL * cos(60/180.0*PI);
    bodies[7].rotVel = 0.003*360*40000/(24764*2.0*PI);
    bodies[7].axialTilt = 28;

    // Sun
    bodies[8].radius = 3*EARTHRAD; //should be 109*Earth, but as separation distances arent to scale with planet size, too big
    bodies[8].mass = 332496*EARTHMASS; //huuuuge
    bodies[8].rotVel = 1.997*360*40000/(4379000);
    bodies[8].axialTilt = 0;

    // Ganymede
    bodies[9].draw = 0;
    bodies[9].radius = 0.413*EARTHRAD;
    bodies[9].mass = 0.025*EARTHMASS;
    bodies[9].pos[1] = 3000000; // out of the way.. hopefully
    bodies[9].rotVel = 300.0;
    bodies[9].axialTilt = 15;

    // Callisto
    bodies[10].draw = 0;
    bodies[10].radius = 0.378*EARTHRAD;
    bodies[10].mass = 0.018*EARTHMASS;
    bodies[10].pos[1] = -3000000; // out of the way
    bodies[10].rotVel = 800.4;
    bodies[10].axialTilt = 62;


    targetSphere = &(bodies[8]); //look at sun
}

///
/// init
/// \brief Initialise the clear colour, enable depth testing, load all the textures needed.
///
void init(void)
{
    glClearColor(0/255.0, 0/255.0, 0/255.0, 1.0);

    glEnable(GL_DEPTH_TEST);
    glLineWidth(0.5);

    // Load Splash Screens
    splashTex[0] = loadTex("media\\titlescreen.jpg");
    splashTex[1] = loadTex("media\\quitsplash.jpg");

    // Load Skybox Textures
    sBoxTex[0] = loadTex("media\\sbox_front.jpg");
    sBoxTex[1] = loadTex("media\\sbox_left.jpg");
    sBoxTex[2] = loadTex("media\\sbox_back.jpg");
    sBoxTex[3] = loadTex("media\\sbox_right.jpg");
    sBoxTex[4] = loadTex("media\\sbox_top.jpg");
    sBoxTex[5] = loadTex("media\\sbox_bottom.jpg");

    // Load planet textures
    bodies[0].texture = loadTex("media\\mercurymap.jpg");
    bodies[1].texture = loadTex("media\\venusmap.jpg");
    bodies[2].texture = loadTex("media\\earthmap.jpg");
    bodies[3].texture = loadTex("media\\marsmap.jpg");
    bodies[4].texture = loadTex("media\\jupitermap.jpg");
    bodies[5].texture = loadTex("media\\saturnmap.jpg");
    bodies[6].texture = loadTex("media\\uranusmap.jpg");
    bodies[7].texture = loadTex("media\\neptunemap.jpg");
    bodies[8].texture = loadTex("media\\sunmap.jpg");
    bodies[9].texture = loadTex("media\\ganymedemap.jpg");
    bodies[10].texture = loadTex("media\\callistomap.jpg");

    resetBodies();


}

///
/// reshape
/// \brief Handles window reshaping.
///
void reshape (int w, int h)
{
    glViewport (0, 0, (GLsizei) w, (GLsizei) h);

    glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 0.4, 60000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    if(state == SIM) // in simulation mode
    {

        gluLookAt (cam.posOffset[0], cam.posOffset[1], cam.posOffset[2],   cam.lookAt[0], cam.lookAt[1], cam.lookAt[2],   0.0, 1.0, 0.0);
    }
    else //on a splash screen
    {
        gluLookAt (250.0, 250.0, 433.013,  250.0, 250.0, 0.0,  0.0, 1.0, 0.0);
    }
}

///
/// drawSquare
/// \brief draws a flat square (or rectangle, if desired) polygon, and applies the supplied texture.
/// Used for displaying the splash (start/exit) screens.
///
/// \param bl GLfloat array of the bottom left vertex coordinates
/// \param br GLfloat array of the bottom right vertex coordinates
/// \param tr GLfloat array of the top right vertex coordinates
/// \param tl GLfloat array of the top left vertex coordinates
/// \param texID texture ID of the desired texture to be applied.
///
void drawSquare(GLfloat* bl, GLfloat* br, GLfloat* tr, GLfloat* tl, GLuint texID)
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texID);
    glColor3f(1.0,1.0,1.0);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0,1.0);
    glVertex3fv(bl); //bottom left
    glTexCoord2f(1.0,1.0);
    glVertex3fv(br); //bottom right
    glTexCoord2f(1.0,0.0);
    glVertex3fv(tr); //top right
    glTexCoord2f(0.0,0.0);
    glVertex3fv(tl); //top left
    glEnd();
}

///
/// display
/// \brief draws the appropriate objects based on the state of the program. Also calculates time scale
/// variables to be used for animation.
///
void display(void)
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(state == SIM) // SIM mode draw block
    {
        int i=0;

        if(firstRunT == 1) // del T set to zero for first run through
        {
            prevT = clock();
            nowT = prevT;
            firstRunT = 0;
        }
        else
        {
            prevT = nowT;
            nowT = clock();
        }

        delT = timeFact*(nowT - prevT)/((GLdouble)CLOCKS_PER_SEC);
        //printf("\ndelT: %E\nnowT: %d",delT,nowT);

        drawSkyBox();

        // Let the camera orbit around the origin
        glLoadIdentity();
        gluLookAt (cam.posOffset[0], cam.posOffset[1], cam.posOffset[2],   cam.lookAt[0], cam.lookAt[1], cam.lookAt[2],   0.0, 1.0, 0.0);

        //draw sphere, accel vector and trail, apply texture
        for(i=0;i<11; i++)
        {
            if(bodies[i].draw == 1)
            {
                drawSphereBody(&(bodies[i]));
            }
        }
    } // end SIM mode draw block
    else
    {
        if(state == TITLE)
        {
            glLoadIdentity();
            gluLookAt (250.0, 250.0, 433.013,  250.0, 250.0, 0.0,  0.0, 1.0, 0.0);
            drawSquare(splashSquareVerts[0],splashSquareVerts[1],splashSquareVerts[2],splashSquareVerts[3],splashTex[0]);
        }
        else // state == QUITSPLASH
        {
            glLoadIdentity();
            gluLookAt (250.0, 250.0, 433.013,  250.0, 250.0, 0.0,  0.0, 1.0, 0.0);
            drawSquare(splashSquareVerts[0],splashSquareVerts[1],splashSquareVerts[2],splashSquareVerts[3],splashTex[1]);
        }
    }

    glutSwapBuffers();

}

///
/// mouseButton
/// \brief handles mouse button push events. Different actions occur depending on the state of the program.
///
void mouseButton(int button, int buttonState, int x, int y)
{
    if(state == TITLE)
    {
        if( button == GLUT_LEFT_BUTTON && buttonState == GLUT_DOWN)
        {
            state = SIM;
            glutAttachMenu(GLUT_RIGHT_BUTTON); // attach menu to right
        }
    }
    else
    {
        if(state == SIM)
        {
            //only start motion if left button pressed
            if ( button == GLUT_LEFT_BUTTON)
            {
                if(buttonState == GLUT_UP)
                {
                    xOrigin = -1;
                    yOrigin = -1;
                }
                else
                {
                    xOrigin = x;
                    yOrigin = y;
                    tempPhi = cameraPhi;
                    tempTheta = cameraTheta;
                }
            }

            // mouse wheel
            if( button == 3 && buttonState == GLUT_DOWN)
            {
                if(camZoom > 1500.0)
                {
                    camZoom = camZoom/2;
                }
            }else if(button == 4 && buttonState == GLUT_DOWN)
            {
                if(camZoom < 16000)
                {
                    camZoom = camZoom*2;
                }
            }


        }
        else // state == quitsplash
        {
            if(button == GLUT_LEFT_BUTTON && buttonState == GLUT_DOWN)
            {
                exit(0);
            }
        }
    }

}

///
/// mouseMove
/// \brief function to allow for the mouse to be used to move the camera.
///
void mouseMove(int x, int y)
{
    //only true when left button pressed down
    if(xOrigin>= 0)
    {
        deltaTheta = -1*(x-xOrigin) * 0.01f; //-1 to make more natural drag motion
        deltaPhi = -1*(y-yOrigin)* 0.01;
        cameraTheta = tempTheta + deltaTheta;
        // limit elevation to only go up and down 180 degrees total, to stop from flipping over
        if((tempPhi+deltaPhi>(0)) && (tempPhi+deltaPhi<(PI)) )
        {
            cameraPhi = tempPhi + deltaPhi;
        }
    }
}

///
/// processCamMenu
/// \brief callback for the Camera Target menu. Updates targetSphere to the chosen target, if that
/// target is enabled.
///
void processCamMenu(int option)
{
    if(bodies[option].draw == 1)
    {
        targetSphere = &(bodies[option]);
    }

}

///
/// processMainMenu
/// \brief Handles the options available on the main menu.
///
void processMainMenu(int option)
{
    int i =0;
    switch(option)
    {
        case 1: //Start Sim
            START = 1;
            break;
        case 2: //Stop
            START = 0;
            break;
        case 3: //enable/disable grav
            if(GCONST == 1.00)
            {
                GCONST = 0.00;
            }
            else
            {
                GCONST = 1.00;
            }
            break;
        case 4: //reset
            resetBodies();
            START = 0;
            break;
        case 5: //Disable all
            for(i=0;i<11;i++)
            {
                bodies[i].draw = 0;
            }
            break;
        case 6: //Enable all
            for(i=0;i<11;i++)
            {
                bodies[i].draw = 1;
            }
            break;
        case 7: //fire gany
            bodies[9].pos[0] = cam.posOffset[0];
            bodies[9].pos[1] = cam.posOffset[1];
            bodies[9].pos[2] = cam.posOffset[2];
            bodies[9].vel[0] = cam.lookVec[0];
            bodies[9].vel[1] = cam.lookVec[1];
            bodies[9].vel[2] = cam.lookVec[2];
            bodies[9].draw = 1;
            bodies[9].trailLength = 0;
            bodies[9].trailStart = 0;
            break;
        case 8: //fire callisto
            bodies[10].pos[0] = cam.posOffset[0];
            bodies[10].pos[1] = cam.posOffset[1];
            bodies[10].pos[2] = cam.posOffset[2];
            bodies[10].vel[0] = cam.lookVec[0];
            bodies[10].vel[1] = cam.lookVec[1];
            bodies[10].vel[2] = cam.lookVec[2];
            bodies[10].draw = 1;
            bodies[10].trailLength = 0;
            bodies[10].trailStart = 0;
            break;
            break;

    }
}

///
/// processDisableMenu
/// \brief callback used in the disable menu to enable/disable certain sphereBodies.
///
void processDisableMenu(int option)
{
    if((bodies[option]).draw==1)
    {
        (bodies[option]).draw = 0;
    }
    else
    {
        (bodies[option]).draw = 1;
    }
}

///
/// processTargetOps
/// \brief callback for the Modify Target menu.
/// Performs the appropriate mass (and thus radius) modifications on the target, if enabled
///
void processTargetOps(int option)
{
    if( (*targetSphere).draw == 1)
    {
        switch(option)
        {
            case 0:
                (*targetSphere).mass /= 1000;
                (*targetSphere).radius /= 1.44;
                break;
            case 1:
                (*targetSphere).mass /= 250;
                (*targetSphere).radius /= 1.2;
                break;
            case 2:
                (*targetSphere).mass *= 250;
                (*targetSphere).radius *= 1.2;
                break;
            case 3:
                (*targetSphere).mass *= 1000;
                (*targetSphere).radius *= 1.44;
                break;

        }
    }
}

///
/// processTimeFact
/// \brief callback for the Modify Time Factor menu. Simple modification of the time factor based on powers of 2.
///
void processTimeFact(int option)
{
    timeFact = pow(2,option);
}

///
/// initMenus
/// \brief initialises menus, options, and associated callback functions.
///
void initMenus()
{
    int timeFactorMenu = glutCreateMenu(processTimeFact);
    glutAddMenuEntry("Default Time Factor",0);
    glutAddMenuEntry("2  * Default",1);
    glutAddMenuEntry("4  * Default",2);
    glutAddMenuEntry("8  * Default",3);
    glutAddMenuEntry("16 * Default",5);
    glutAddMenuEntry("32 * Default",6);
    glutAddMenuEntry("64 * Default",7);

    int targetOpsMenu = glutCreateMenu(processTargetOps);
    glutAddMenuEntry("-- Mass",0);
    glutAddMenuEntry("-  Mass",1);
    glutAddMenuEntry("+  Mass",2);
    glutAddMenuEntry("++ Mass",3);

    int camTargetMenu = glutCreateMenu(processCamMenu);
    glutAddMenuEntry("Sun",8);
    glutAddMenuEntry("Mercury",0);
    glutAddMenuEntry("Venus",1);
    glutAddMenuEntry("Earth",2);
    glutAddMenuEntry("Mars",3);
    glutAddMenuEntry("Jupiter",4);
    glutAddMenuEntry("Ganymede",9);
    glutAddMenuEntry("Callisto",10);
    glutAddMenuEntry("Saturn",5);
    glutAddMenuEntry("Uranus",6);
    glutAddMenuEntry("Neptune",7);

    int disablePlanetMenu = glutCreateMenu(processDisableMenu);
    glutAddMenuEntry("Sun",8);
    glutAddMenuEntry("Mercury",0);
    glutAddMenuEntry("Venus",1);
    glutAddMenuEntry("Earth",2);
    glutAddMenuEntry("Mars",3);
    glutAddMenuEntry("Jupiter",4);
    //glutAddMenuEntry("Ganymede",9);
    //glutAddMenuEntry("Callisto",10);
    glutAddMenuEntry("Saturn",5);
    glutAddMenuEntry("Uranus",6);
    glutAddMenuEntry("Neptune",7);

    int mainMenu = glutCreateMenu(processMainMenu);
    glutAddMenuEntry("START SIMULATION",1);
    glutAddMenuEntry("STOP SIMULATION",2);
    glutAddMenuEntry("DISABLE/ENABLE GRAVITY",3);
    glutAddMenuEntry("RESET SIZES/POSITIONS",4);
    glutAddMenuEntry("DISABLE ALL",5);
    glutAddMenuEntry("ENABLE ALL",6);
    glutAddMenuEntry("FIRE GANYMEDE!",7);
    glutAddMenuEntry("FIRE CALLISTO!",8);
    glutAddSubMenu("Modify Time Factor", timeFactorMenu);
    glutAddSubMenu("Modify Target", targetOpsMenu);
    glutAddSubMenu("Disable/Enable Planets", disablePlanetMenu);

    glutAddSubMenu("Camera Target",camTargetMenu);


    //glutAttachMenu(GLUT_RIGHT_BUTTON); // dont attach during splash screens

}

///
/// pressKey
/// \brief callback for glutSpecialFunc, to process what happens when special keys are hit.
/// In this case, handles the zooming in/out of the camera.
///
void pressKey(int key, int x, int y)
{
    switch(key)
    {
        case GLUT_KEY_UP : //zoom in
            if(camZoom > 1500.0)
            {
                camZoom = camZoom/2;
            }
            break;
        case GLUT_KEY_DOWN : //zoom out
            if(camZoom < 16000)
            {
                camZoom = camZoom*2;
            }
    }
}

///
/// processNormalKeys
/// \brief callback for glutKeyboardFunc, used to process normal character presses.
/// Shows the quit splash screen if the ESC key is hit during simulation mode.
///
void processNormalKeys(unsigned char key, int x, int y)
{
    if(state == SIM)
    {
        if(key == 27)
        {
            state = QUITSPLASH;
            glutDetachMenu(GLUT_RIGHT_BUTTON); // detach menu
        }
    }
}

///
/// main
/// \brief entry point and initialisation
///
int main(int argc, char** argv)
{
    // start your engines
    startT = clock();

    // init glut
    glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    //window setup
    glutInitWindowSize (1280, 720);
    glutInitWindowPosition (500, 200);
    glutCreateWindow ("Celestial Bodies");

    // run init
    init ();

    // set callbacks
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(animate);

    // key callbacks
    glutSpecialFunc(pressKey);
    glutKeyboardFunc(processNormalKeys);

    //mouse callbacks
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMove);

    // setup menus
    initMenus();

    // and off we go
    glutMainLoop();

    return 0;
}
