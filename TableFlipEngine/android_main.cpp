/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

//BEGIN_INCLUDE(all)
#include <jni.h>
#include <errno.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>
#include <unistd.h>

#include "Utilities.h"
#include "Renderer.h"
#include "Physics.h"
#include "Input.h"
#include "Resources.h"
#include "ObjectManager.h"
#include "UserInterface.h"
#include "Builder.h"
#include "TestGame.h"

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))

//SoundSystem     *soundSystem = 0;
Renderer        *renderer = 0;
Input           *input = 0;
//Scripting       *scripting = 0;
Physics         *physics = 0;
ObjectManager   *objectManager = 0;
UIManager       *uiManager = 0;
Builder         *builder = 0;

AAssetManager* assetManager = 0;

//yep, i have to track active touches, to get individual touch motion... *shoots self in face*
bool    touchStates[NUM_MULTITOUCH_CURSORS];

bool WindowCreated = false;
bool EngineInitialized = false;
unsigned int lastTime = 0;
/**
 * Our saved state data.
 */
struct saved_state {
    float angle;
    int32_t x;
    int32_t y;
};

/**
 * Shared state for our app.
 */
struct engine {
    struct android_app* app;

    ASensorManager* sensorManager;
    const ASensor* accelerometerSensor;
    ASensorEventQueue* sensorEventQueue;

    int animating;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    struct saved_state state;
};

bool InitEngine(int w, int h);

/**
 * Initialize an EGL context for the current display.
 */
static int engine_init_display(struct engine* engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     * EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
     */
    const EGLint attribs[] = {

    		EGL_DEPTH_SIZE, 8,
    		EGL_STENCIL_SIZE, 0,
    		EGL_RENDERABLE_TYPE, 4,
            EGL_SURFACE_TYPE, 1,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
    };

    const EGLint ctxAttrs[] = {
    		EGL_CONTEXT_CLIENT_VERSION, 2,
    		EGL_NONE
    };

    EGLint w, h, dummy, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, 0, 0);

    /* Here, the application chooses the configuration it desires. In this
     * sample, we have a very simplified selection process, where we pick
     * the first EGLConfig that matches our criteria */
    eglChooseConfig(display, attribs, &config, 1, &numConfigs);

    /* EGL_NATIVE_VISUAL_ID is an attribute of the EGLConfig that is
     * guaranteed to be accepted by ANativeWindow_setBuffersGeometry().
     * As soon as we picked a EGLConfig, we can safely reconfigure the
     * ANativeWindow buffers to match, using EGL_NATIVE_VISUAL_ID. */
    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

    ANativeWindow_setBuffersGeometry(engine->app->window, 0, 0, format);

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    context = eglCreateContext(display, config, NULL, ctxAttrs);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        LOGW("Unable to eglMakeCurrent");
        return -1;
    }

    int stuff;

    eglQueryContext(display, context, EGL_CONTEXT_CLIENT_VERSION, &stuff);
    char buff[256];
    sprintf(buff, "Context Version: %d", stuff);
    LogError(buff);


    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;
    engine->state.angle = 0;
    engine->animating = 0;

    // Initialize GL state.
   // glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    //glEnable(GL_CULL_FACE);
    //glShadeModel(GL_SMOOTH);
    //glDisable(GL_DEPTH_TEST);

    InitEngine(w,h);

    //init touch states
    for (int i = 0; i < NUM_MULTITOUCH_CURSORS; i++)
    	        touchStates[i] = false;

    engine->animating = 1;
    return 0;
}

/**
 * Just the current frame in the display.
 */
static void engine_draw_frame(struct engine* engine) {
    if (engine->display == NULL) {
        // No display.
        return;
    }

    // Just fill the screen with a color.
   // glClearColor(((float)engine->state.x)/engine->width, engine->state.angle,
     //       ((float)engine->state.y)/engine->height, 1);
   // glClear(GL_COLOR_BUFFER_BIT);

    //eglSwapBuffers(engine->display, engine->surface);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void engine_term_display(struct engine* engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->animating = 0;
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app* app, AInputEvent* event) {
    struct engine* engine = (struct engine*)app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION)
    {
    	int numContacts = AMotionEvent_getPointerCount(event);
    	//char buff[64];
    	//sprintf(buff, "numContacts: %d\n", numContacts);
    	//LogError(buff);

    	//for (int i = 0; i < numContacts; i++)
    	{



			int action = AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_MASK;
			int idx = (AMotionEvent_getAction(event) & AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >> AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;
			int id = AMotionEvent_getPointerId(event, idx);

			int x = AMotionEvent_getX(event, id);
			int y = AMotionEvent_getY(event, id);
			Vector2 pos = Vector2((float)x,(float)y);

			//sprintf(buff, "idx: %d\n", idx);
			    				//LogError(buff);
			    	                  //      sprintf(buff, "index: %d\n", id);
			    	                      // LogError(buff);
			switch(action)
			{
				case AMOTION_EVENT_ACTION_DOWN:
				{

					//LogError("Action Down!");
					if (id >= 0 && id < NUM_MULTITOUCH_CURSORS)
					{
                        if (id == 0)
                            input->ButtonPressed((unsigned int)1);
						input->TouchDown(id, pos);
						touchStates[id] = true;
					}
					break;
				}
				case AMOTION_EVENT_ACTION_UP:
				{
					//sprintf(buff, "Action Up! idx: %d", idx);
					    	//LogError(buff);
					//LogError(buff);
					if (id >= 0 && id < NUM_MULTITOUCH_CURSORS)
					{
                        if (id == 0)
                            input->ButtonReleased((unsigned int)1);
						input->TouchUp(id, pos);
						touchStates[id] = false;
					}
					break;
				}
				case AMOTION_EVENT_ACTION_POINTER_DOWN:
				{

					//LogError("Action Pointer Down!");
					if (id >= 0 && id < NUM_MULTITOUCH_CURSORS)
					{
                        if (id == 0)
                            input->ButtonPressed((unsigned int)1);
						input->TouchDown(id, pos);
						touchStates[id] = true;
					}
					break;
				}
				case AMOTION_EVENT_ACTION_POINTER_UP:
				{

					//LogError("Action Pointer Up!!");
					if (id >= 0 && id < NUM_MULTITOUCH_CURSORS)
					{
                        if (id == 0)
                            input->ButtonReleased((unsigned int)1);
						touchStates[id] = false;
						input->TouchUp(id, pos);
					}
					break;
				}
				case AMOTION_EVENT_ACTION_MOVE:
				{
					//ok so something moved..
					int ptrCount = AMotionEvent_getPointerCount(event);
					//sprintf(buff, "pointer count: %d", ptrCount);
						//LogError(buff);
					for (int i = 0; i < ptrCount; i++)
					{
						int newId = AMotionEvent_getPointerId(event, i);
						char buff[64];

						//basically if we have an active touch, update only those positions.
						if (input->GetTouchDown(newId))
						{
							//sprintf(buff, "TOUCH ID DOWN: %d", newId);
													//LogError(buff);
							pos.x = (float)AMotionEvent_getX(event, i);
							pos.y = (float)AMotionEvent_getY(event, i);

							input->TouchMove(newId, pos);
						}
					}
					break;
				}
				default:
				{
					return 1;
					break;
				}
			}

    	}

    	//LogError("Friggin tap event!");
        //engine->animating = 1;
        //engine->state.x = AMotionEvent_getX(event, 0);
       // engine->state.y = AMotionEvent_getY(event, 0);
        //return 1;
    }
    else if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_KEY)
    {
    	//i dont think this is needed, but just in case..
    	//wtf was this even for?
    	return 1;
    }
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
    struct engine* engine = (struct engine*)app->userData;
    switch (cmd)
    {
        case APP_CMD_SAVE_STATE:
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct saved_state));
            *((struct saved_state*)engine->app->savedState) = engine->state;
            engine->app->savedStateSize = sizeof(struct saved_state);
            break;
        case APP_CMD_INIT_WINDOW:
            // The window is being shown, get it ready.
            if (engine->app->window != NULL)
            {
                engine_init_display(engine);


               // engine_draw_frame(engine);
            }
            break;
        case APP_CMD_TERM_WINDOW:
            // The window is being hidden or closed, clean it up.
            engine_term_display(engine);
            break;
        case APP_CMD_GAINED_FOCUS:
            // When our app gains focus, we start monitoring the accelerometer.
            engine->animating = 1; //animate again
            if (engine->accelerometerSensor != NULL)
            {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                        engine->accelerometerSensor, (1000L/60)*1000);
            }
            break;
        case APP_CMD_LOST_FOCUS:
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL)
            {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                        engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->animating = 0;
            engine_draw_frame(engine);
            break;
    }
}

bool InitEngine(int w, int h)
{
	InitMasterTimecode();

	Input::InitInstance();
	input = Input::GetInstance();
	input->Init();
    input->mouseXSensitivity = 0.0033f;
    input->mouseYSensitivity = 0.0033f;
	Renderer::InitInstance();
	renderer = Renderer::GetInstance();
	if (!renderer->Init())
	{
		LogError("Renderer failed to initialize!?");
		return false;
	}
	renderer->ResizeWindow(w, h);

	LogInfo("Physics Init");
	Physics::InitInstance();
	physics = Physics::GetInstance();
	physics->Init();

	LogInfo("Objects Init");
	ObjectManager::InitInstance();
	objectManager = ObjectManager::GetInstance();
	objectManager->Init();

	LogInfo("UI Init");
	UIManager::InitInstance();
	uiManager = UIManager::GetInstance();
	uiManager->Init();

    Builder::InitInstance();
    builder = Builder::GetInstance();

	LogInfo("Game Init");
	GameInit();


	lastTime = GetTimecode();

	LogError("Engine initted!");
	EngineInitialized = true;
}

/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app* state)
{
    struct engine engine;

    // Make sure glue isn't stripped.
    app_dummy();

    memset(&engine, 0, sizeof(engine));
    state->userData = &engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;
    engine.app = state;

    assetManager = state->activity->assetManager;
    if (assetManager == 0)
    {
    	LogError("NO ASSET MANAGER!?!?!, this is catastrophic. and unforseen...");
    	return;
    }

    SetAndroidAssetManager(assetManager);
    //env->NewGlobalRef(assetManager);
    char buffr[64];
    sprintf(buffr, "asset manager passing to engine: %d", assetManager);
    LogError(buffr);
//ASENSOR_TYPE_ACCELEROMETER
    // Prepare to monitor accelerometer
    engine.sensorManager = ASensorManager_getInstance();
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(engine.sensorManager,
            ASENSOR_TYPE_GYROSCOPE );
    engine.sensorEventQueue = ASensorManager_createEventQueue(engine.sensorManager,
            state->looper, LOOPER_ID_USER, NULL, NULL);

    if (state->savedState != NULL)
    {
        // We are starting with a previous saved state; restore from it.
        engine.state = *(struct saved_state*)state->savedState;
    }



    while (1) //uhhS
    {

        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source* source;

        if (input)
        	input->Update(); // MUST BE UPDATED BEFORE WE SET ANYTHING!!

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident=ALooper_pollAll(engine.animating ? 0 : -1, NULL, &events, (void**)&source)) >= 0)
        {

            // Process this event.
            if (source != NULL)
            {
                source->process(state, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER)
            {
                if (engine.accelerometerSensor != NULL)
                {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue, &event, 1) > 0)
                    {
                       LOGI("accelerometer: x=%f y=%f z=%f",
                                event.acceleration.x, event.acceleration.y,
                                event.acceleration.z);
                    }
                }
            }




			// Check if we are exiting.
			if (state->destroyRequested != 0)
			{
				LogError("DESTROYING!!");


			   GameShutdown();
			   //shutdown modules
			   objectManager->Shutdown();
			   CleanResources(0);

			   renderer->Shutdown();
			   input->Shutdown();
			   physics->Shutdown();
			   uiManager->Shutdown();

			   objectManager->DeleteInstance();
			   physics->DeleteInstance();
			   renderer->DeleteInstance();
			   input->DeleteInstance();
			   uiManager->DeleteInstance();
               builder->DeleteInstance();

               //is this really needed?
               CleanResources(0);

			   engine_term_display(&engine);
			   return;
			}
        }
		UpdateMasterTimecode();
		//acts sort of like a state reset.


		//CheckGLErrors();


		if (engine.animating)
		{
			float dt = (float)(GetTimecode() - lastTime);
			objectManager->Update(dt);
			physics->Update(0.001f * (float)(GetTimecode() - lastTime));
			uiManager->Update();
			GameUpdate();
			lastTime = GetTimecode();


			//  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			renderer->PreRender();

			renderer->Render3D();
			
			renderer->Render2D();
			//renderer->PostRender();

			eglSwapBuffers(engine.display, engine.surface);

		}

		input->KeyReleased(KB_LSHIFT);
		usleep(1000);
    }
}
//END_INCLUDE(all)
