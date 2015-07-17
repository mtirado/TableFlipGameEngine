/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 

#ifndef __GL_CAPTURE_H__
#define __GL_CAPTURE_H__
#include <string>
#include <pthread.h>


#define MAX_VID_IO_THREADS 64


class GLCapture
{
public:
    void ThreadEnded(int threadId); //dont call this pls
    
private:
    bool capturing; //in progress?
    float framerate;
    FILE *file;
    unsigned char *readBuff; //screen data
    int width;
    int height;

    unsigned int frameCount;
    float snapDelay;     //delay(seconds) between screengrabs
    float timeSinceSnap; //seconds since last snap
    std::string path;    //file path on disk
    pthread_t thread[MAX_VID_IO_THREADS]; //async IO
    bool threadUsed[MAX_VID_IO_THREADS];  //in use?
    unsigned int deployedThreads;         //how many deployed threads
    int curThread;
    
public:
    void setFramerate(float fps) { framerate = fps; snapDelay = 1.0 / framerate; }
    bool isCapturing() { return capturing; }
    
    //DO NOT CHANGE RESOLUTION ONCE IT HAS STARTED!!!
    bool StartCapture(const char* filename, int w, int h);
    //dt = time in seconds since last frame
    void Update(float dt); //call this after buffer swap?
    void EndCapture();

    

    GLCapture();
};

#endif
