/* (c) 2015 Michael R. Tirado -- GPLv3, GNU General Public License, version 3.
 * contact: mtirado418@gmail.com
 *
 */ 
#include "Utilities.h"
#include "GLCapture.h"
#include <SOIL.h>
#include <stdio.h>
#include <string>

struct ThreadContext
{
    std::string filename;
    unsigned char *data;
    int threadIdx;
    int size;
    int width;
    int height;
    GLCapture *parent;
};

GLCapture::GLCapture()
{
    capturing = false;
    framerate = 25;
    file = 0;
    readBuff = 0;
    readBuff = 0;
    snapDelay = 1.0 / framerate;
    timeSinceSnap = 0.0;
    curThread = 0;
    deployedThreads = 0;
}

//must delete data when done!
static void *WriteFile(void *arg)
{
    ThreadContext *ctx = (ThreadContext *)arg;

    //invert data :[
    for( int j = 0; j*2 < ctx->height; ++j )
    {
        int index1 = j * ctx->width * 3;
        int index2 = (ctx->height - 1 - j) * ctx->width * 3;
        for( int i = ctx->width * 3; i > 0; --i )
        {
            unsigned char temp = ctx->data[index1];
            ctx->data[index1] = ctx->data[index2];
            ctx->data[index2] = temp;
            ++index1;
            ++index2;
        }
    }
    
    SOIL_save_image( ctx->filename.c_str(), SOIL_SAVE_TYPE_BMP, ctx->width, ctx->height, 3, ctx->data);

    delete[] ctx->data; //TODO this might not be getting deleted data allocated in parent process

    GLCapture *p = ctx->parent;
    p->ThreadEnded(ctx->threadIdx);
    delete ctx; //cleanup the context too.
    pthread_exit(0);
    return 0; //??
}

bool GLCapture::StartCapture(const char* filename, int w, int h)
{
    if (capturing)
        return false;
    
    width  = w;
    height = h;
    path = std::string("Data/VidFrames/") + filename;
    timeSinceSnap = 0.0;
    readBuff = new unsigned char[width * height * 3]; //SINGLE frame
    frameCount = 0;
    capturing = true;
    return true;
}

void GLCapture::Update(float dt) 
{
    if (!capturing)
        return;
    
    timeSinceSnap += dt;
    if (timeSinceSnap < snapDelay)
        return;


   // LOGOUTPUT << "actual Deployed threads: " << deployedThreads;
   // LogWarning();

    if (deployedThreads == MAX_VID_IO_THREADS)
    {
        LogError("no available threads left..");
        return;
    }

    int threadIdx = 0;
    for (int i = 0; i < MAX_VID_IO_THREADS; i++)
    {
        if (threadUsed[i] == false)
        {
            threadIdx = i;
            break;
        }
    }
    
    std::stringstream theFile;
    theFile << path.c_str() << frameCount << ".bmp";
    
    //take a new snapshot
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, readBuff);

    int size = width * height * 3;

    //will be deleted in new thread <<<<  TODO wut?? this is probably leaking
    //can we just pass readBuff as ctx->data?  if not then add a delete after
    //pthread_create, i forgot how pthread works but if it's anything like
    //fork then this is definitely leaking! TODO: RTFM
    unsigned char *localData = new unsigned char[size];
    memcpy(localData, readBuff, size);

    //create context
    ThreadContext *ctx = new ThreadContext;
    ctx->data = localData;
    ctx->filename = theFile.str().c_str(); //ugh this is wasteful
    ctx->height = height;
    ctx->width = width;
    ctx->size = size;
    ctx->threadIdx = threadIdx;
    ctx->parent = this;
    pthread_t newThread;
    int res = pthread_create(&thread[threadIdx], 0, WriteFile, ctx);
    if (res)
    {
        LogError("GLCapture::Update - Error Creating thread.");
        delete[] localData;
    }
    else
    {
        threadUsed[threadIdx] = true;
        deployedThreads++;
    }
    
    timeSinceSnap = 0.0;
    frameCount++;
}

void GLCapture::ThreadEnded(int threadId)
{
    //LogInfo("thread ending");
    threadUsed[threadId] = false;
    pthread_detach(thread[threadId]);
    thread[threadId] = 0;
    deployedThreads--;
}

/* inspired by http://antongerdelan.net/opengl/videocapture.html
 * void grab_video_frame () {
  // copy frame-buffer into 24-bit rgbrgb...rgb image
  glReadPixels (
    0, 0, g_gl_width, g_gl_height, GL_RGB, GL_UNSIGNED_BYTE, g_video_memory_ptr
  );
  // move video pointer along to the next frame's worth of bytes
  g_video_memory_ptr += g_gl_width * g_gl_height * 3;
}
 */

void GLCapture::EndCapture()
{
    if (!capturing)
        return;
    //last step
    delete[] readBuff;
    frameCount = 0;
    timeSinceSnap = 0.0;
    capturing = false;
}
