#//(?°?°??? ???       ????( º _ º?)
project(tableflipengine)
cmake_minimum_required ( VERSION 2.8 )
#the table flip engine!



#directly linking the API's into the engine
SET(SOIL_SRCS
	SOIL/image_helper.c
	SOIL/stb_image_aug.c
	SOIL/image_DXT.c
	SOIL/SOIL.c)

#TODO comment out stuff you don't need
SET(BulletDynamics_SRCS
	bullet/BulletDynamics/Character/btCharacterControllerInterface.h
	bullet/BulletDynamics/Character/btKinematicCharacterController.cpp
    bullet/BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp
    #bullet/BulletDynamics/ConstraintSolver/btContactConstraint.cpp
    #bullet/BulletDynamics/ConstraintSolver/btGearConstraint.cpp
    bullet/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp
    #bullet/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp
    #bullet/BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp
    bullet/BulletDynamics/ConstraintSolver/btHingeConstraint.cpp
    #bullet/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp
    bullet/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp
    #bullet/BulletDynamics/ConstraintSolver/btSliderConstraint.cpp
    #bullet/BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp
    bullet/BulletDynamics/ConstraintSolver/btTypedConstraint.cpp
    #	bullet/BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp
	bullet/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp
	bullet/BulletDynamics/Dynamics/btOrbitalDynamicsWorld.cpp
	bullet/BulletDynamics/Dynamics/btRigidBody.cpp
    #bullet/BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp
	bullet/BulletDynamics/Dynamics/Bullet-C-API.cpp
    #bullet/BulletDynamics/Vehicle/btRaycastVehicle.cpp
    #bullet/BulletDynamics/Vehicle/btWheelInfo.cpp
)
	
SET(BulletCollision_SRCS
	bullet/BulletCollision/BroadphaseCollision/btAxisSweep3.cpp
	bullet/BulletCollision/BroadphaseCollision/btBroadphaseProxy.cpp
	bullet/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.cpp
	bullet/BulletCollision/BroadphaseCollision/btDbvt.cpp
	bullet/BulletCollision/BroadphaseCollision/btDbvtBroadphase.cpp
	bullet/BulletCollision/BroadphaseCollision/btDispatcher.cpp
    #bullet/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.cpp
	bullet/BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp
	bullet/BulletCollision/BroadphaseCollision/btQuantizedBvh.cpp
    #bullet/BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp
    bullet/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.cpp
	bullet/BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp
    bullet/BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.cpp
	bullet/BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp
	bullet/BulletCollision/CollisionDispatch/btCollisionDispatcher.cpp
	bullet/BulletCollision/CollisionDispatch/btCollisionObject.cpp
	bullet/BulletCollision/CollisionDispatch/btCollisionWorld.cpp
	bullet/BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.cpp
	bullet/BulletCollision/CollisionDispatch/btCompoundCompoundCollisionAlgorithm.cpp
	bullet/BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp
	bullet/BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp
	bullet/BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp
	bullet/BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.cpp
	bullet/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.cpp
	bullet/BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.cpp
	bullet/BulletCollision/CollisionDispatch/btGhostObject.cpp
	bullet/BulletCollision/CollisionDispatch/btHashedSimplePairCache.cpp
    bullet/BulletCollision/CollisionDispatch/btInternalEdgeUtility.cpp
    bullet/BulletCollision/CollisionDispatch/btInternalEdgeUtility.h
	bullet/BulletCollision/CollisionDispatch/btManifoldResult.cpp
	bullet/BulletCollision/CollisionDispatch/btSimulationIslandManager.cpp
    bullet/BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.cpp
    bullet/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.cpp
    bullet/BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.cpp
	bullet/BulletCollision/CollisionDispatch/btUnionFind.cpp
    bullet/BulletCollision/CollisionDispatch/SphereTriangleDetector.cpp
	bullet/BulletCollision/CollisionShapes/btBoxShape.cpp
	bullet/BulletCollision/CollisionShapes/btBox2dShape.cpp
	bullet/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.cpp
    bullet/BulletCollision/CollisionShapes/btCapsuleShape.cpp
	bullet/BulletCollision/CollisionShapes/btCollisionShape.cpp
	bullet/BulletCollision/CollisionShapes/btCompoundShape.cpp
	bullet/BulletCollision/CollisionShapes/btConcaveShape.cpp
    bullet/BulletCollision/CollisionShapes/btConeShape.cpp
	bullet/BulletCollision/CollisionShapes/btConvexHullShape.cpp
	bullet/BulletCollision/CollisionShapes/btConvexInternalShape.cpp
    #bullet/BulletCollision/CollisionShapes/btConvexPointCloudShape.cpp
	bullet/BulletCollision/CollisionShapes/btConvexPolyhedron.cpp
	bullet/BulletCollision/CollisionShapes/btConvexShape.cpp
    bullet/BulletCollision/CollisionShapes/btConvex2dShape.cpp
	bullet/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.cpp
    bullet/BulletCollision/CollisionShapes/btCylinderShape.cpp
	bullet/BulletCollision/CollisionShapes/btEmptyShape.cpp
    #bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.cpp
    #bullet/BulletCollision/CollisionShapes/btMinkowskiSumShape.cpp
    #bullet/BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.cpp
    bullet/BulletCollision/CollisionShapes/btMultiSphereShape.cpp
	bullet/BulletCollision/CollisionShapes/btOptimizedBvh.cpp
    bullet/BulletCollision/CollisionShapes/btPolyhedralConvexShape.cpp
    bullet/BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.cpp
    bullet/BulletCollision/CollisionShapes/btShapeHull.cpp
    bullet/BulletCollision/CollisionShapes/btSphereShape.cpp
    #bullet/BulletCollision/CollisionShapes/btStaticPlaneShape.cpp
    bullet/BulletCollision/CollisionShapes/btStridingMeshInterface.cpp
    #bullet/BulletCollision/CollisionShapes/btTetrahedronShape.cpp
	bullet/BulletCollision/CollisionShapes/btTriangleBuffer.cpp
	bullet/BulletCollision/CollisionShapes/btTriangleCallback.cpp
	bullet/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.cpp
	bullet/BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.cpp
	bullet/BulletCollision/CollisionShapes/btTriangleMesh.cpp
	bullet/BulletCollision/CollisionShapes/btTriangleMeshShape.cpp
	bullet/BulletCollision/CollisionShapes/btUniformScalingShape.cpp
	bullet/BulletCollision/Gimpact/btContactProcessing.cpp
    #bullet/BulletCollision/Gimpact/btGenericPoolAllocator.cpp
    #bullet/BulletCollision/Gimpact/btGImpactBvh.cpp
    #bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp
    #bullet/BulletCollision/Gimpact/btGImpactQuantizedBvh.cpp
    #bullet/BulletCollision/Gimpact/btGImpactShape.cpp
    #bullet/BulletCollision/Gimpact/btTriangleShapeEx.cpp
    #bullet/BulletCollision/Gimpact/gim_box_set.cpp
    #bullet/BulletCollision/Gimpact/gim_contact.cpp
    #bullet/BulletCollision/Gimpact/gim_memory.cpp
    #bullet/BulletCollision/Gimpact/gim_tri_collision.cpp
	bullet/BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.cpp
	bullet/BulletCollision/NarrowPhaseCollision/btConvexCast.cpp
	bullet/BulletCollision/NarrowPhaseCollision/btGjkConvexCast.cpp
	bullet/BulletCollision/NarrowPhaseCollision/btGjkEpa2.cpp
	bullet/BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.cpp
	bullet/BulletCollision/NarrowPhaseCollision/btGjkPairDetector.cpp
    bullet/BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.cpp
	bullet/BulletCollision/NarrowPhaseCollision/btPersistentManifold.cpp
	bullet/BulletCollision/NarrowPhaseCollision/btRaycastCallback.cpp
    bullet/BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp
    bullet/BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.cpp
    bullet/BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.cpp
    #bullet/BulletSoftBody/btSoftBodyRigidBodyCollisionConfiguration.cpp
    #bullet/BulletSoftBody/btSoftRigidCollisionAlgorithm.cpp
    #bullet/BulletSoftBody/btSoftBodyConcaveCollisionAlgorithm.cpp
    #bullet/BulletSoftBody/btSoftSoftCollisionAlgorithm.cpp
    #bullet/BulletSoftBody/btSoftRigidDynamicsWorld.cpp
    #bullet/BulletSoftBody/btDefaultSoftBodySolver.cpp
    #bullet/BulletSoftBody/btSoftBodyHelpers.cpp
    #bullet/BulletSoftBody/btSoftBody.cpp
)

SET(LinearMath_SRCS
	bullet/LinearMath/btAlignedAllocator.cpp
	bullet/LinearMath/btConvexHull.cpp
	bullet/LinearMath/btConvexHullComputer.cpp
	bullet/LinearMath/btGeometryUtil.cpp
	bullet/LinearMath/btPolarDecomposition.cpp
	bullet/LinearMath/btQuickprof.cpp
	bullet/LinearMath/btSerializer.cpp
	bullet/LinearMath/btVector3.cpp
)

SET(LinearMath_HDRS
	bullet/LinearMath/btAabbUtil2.h
	bullet/LinearMath/btAlignedAllocator.h
	bullet/LinearMath/btAlignedObjectArray.h
	bullet/LinearMath/btConvexHull.h
	bullet/LinearMath/btConvexHullComputer.h
	bullet/LinearMath/btDefaultMotionState.h
	bullet/LinearMath/btGeometryUtil.h
	bullet/LinearMath/btGrahamScan2dConvexHull.h
	bullet/LinearMath/btHashMap.h
	bullet/LinearMath/btIDebugDraw.h
	bullet/LinearMath/btList.h
	bullet/LinearMath/btMatrix3x3.h
	bullet/LinearMath/btMinMax.h
	bullet/LinearMath/btMotionState.h
	bullet/LinearMath/btPolarDecomposition.h
	bullet/LinearMath/btPoolAllocator.h
	bullet/LinearMath/btQuadWord.h
	bullet/LinearMath/btQuaternion.h
	bullet/LinearMath/btQuickprof.h
	bullet/LinearMath/btRandom.h
	bullet/LinearMath/btScalar.h
	bullet/LinearMath/btSerializer.h
	bullet/LinearMath/btStackAlloc.h
	bullet/LinearMath/btTransform.h
	bullet/LinearMath/btTransformUtil.h
	bullet/LinearMath/btVector3.h
	
)


SET(Root_HDRS
	
	bullet/Bullet-C-Api.h
	bullet/vectormath/vmInclude.h
	bullet/btBulletDynamicsCommon.h
	bullet/btBulletCollisionCommon.h
	bullet/GLDebugDrawer.h
	bullet/GLDebugDrawer.cpp
)

SET(AngelScript_SRCS
	AngelScript/scriptbuilder/scriptbuilder.cpp
	AngelScript/scripthandle/scripthandle.cpp
	AngelScript/scriptstdstring/scriptstdstring.cpp
)

SET(FREETYPE_GL_SRC
        freetype-gl/mat4.c
        freetype-gl/texture-atlas.c
        freetype-gl/texture-font.c
        freetype-gl/vertex-buffer.c
        freetype-gl/vertex-attribute.c
        freetype-gl/font-manager.c
        freetype-gl/text-buffer.c
        freetype-gl/shader.c
        freetype-gl/vector.c
        freetype-gl/platform.c
    )

#set include dir
	MESSAGE ( STATUS "Setting Include Directories..." )
	#include_directories( ${CMAKE_SOURCE_DIR}/AngelScript)
	include_directories( ${CMAKE_SOURCE_DIR}/DataStructures )
	include_directories( ${CMAKE_SOURCE_DIR}/utilities )
	include_directories( ${CMAKE_SOURCE_DIR} )
	include_directories( ${CMAKE_SOURCE_DIR}/UserInterface )
	include_directories( ${CMAKE_SOURCE_DIR}/GameCode )
	include_directories( ${CMAKE_SOURCE_DIR}/Sound )
	include_directories( ${CMAKE_SOURCE_DIR}/Resources )
	include_directories( ${CMAKE_SOURCE_DIR}/Components )
	include_directories( ${CMAKE_SOURCE_DIR}/yaknet )
	include_directories( ${CMAKE_SOURCE_DIR}/AL)
	include_directories( ${CMAKE_SOURCE_DIR}/SOIL)
	include_directories( ${CMAKE_SOURCE_DIR}/bullet)
	include_directories( ${CMAKE_SOURCE_DIR}/TinyXML2)
	include_directories( ${CMAKE_SOURCE_DIR}/freetype-gl )
	include_directories( ${OPENGL_INCLUDE_DIRS} ${X11_INCLUDE_DIR} )

	
	#add components here
	SET(Components_SRCS
				Components/Components.cpp
				Components/Logic.cpp
				Components/Transform3d.cpp
			    Components/Mesh.cpp
			    Components/SkeletalMesh.cpp
			    Components/AnimationController.cpp
			    Components/Sprite.cpp
			    Components/ParticleSystem.cpp
			    )

    SET(Resources_SRCS
        Resources/Resources.cpp
        Resources/AnimationResource.cpp
        Resources/ModelResource.cpp
        Resources/ShaderResource.cpp
        Resources/SoundResource.cpp
    )

	SET(Utility_SRCS
	utilities/b64_ntop.cpp
	utilities/Logger.cpp
	utilities/sha1.cpp
	utilities/GLCapture.cpp
	)

    SET(UserInterface_SRCS
    UserInterface/UISkin.cpp
    UserInterface/UIElement.cpp
    UserInterface/UIButton.cpp
    UserInterface/UISlider.cpp
    UserInterface/UIScrollbar.cpp
    UserInterface/UITextOut.cpp
    UserInterface/UITextField.cpp
    UserInterface/UITitleBar.cpp
    UserInterface/UIWindow.cpp
    UserInterface/UIView.cpp
    UserInterface/UIListBox.cpp
    UserInterface/UITextView.cpp
    #UserInterface/UICreateElement.cpp
    UserInterface/UserInterface.cpp
    )
    
	#networking -- make sure you are using latest version of yaknet
    #and add whatever files you need here.
	SET(Networking_SRCS
        #yaknet/NetEvents.cpp
        #yaknet/NetBuffer.cpp
        #yaknet/Connection.cpp
        #yaknet/TCPBuffer.cpp
        #yaknet/WebSockets.cpp
        )
	
	#game specific code added to this list
	SET(GameCode_SRCS
        #GameCode/Examples/Octree-Test.cpp
        GameCode/Examples/UI-Stuff.cpp
        #GameCode/GameUI.cpp
        #GameCode/Lander.cpp
        #GameCode/Instrumentation.cpp
	)

	SET(Sound_SRCS
		Sound/SynthEffect.cpp
		Sound/SoundSystem.cpp
		Sound/WaveGen.cpp
	)

#webgl build is going to be a liiittle bit different too much voodoo happening here
if (WEBGL_BUILD_TARGET)
	MESSAGE ( STATUS "================================" )
	MESSAGE ( STATUS "= building emscripten makefile =" )
	MESSAGE ( STATUS "================================" )

	#tellem!
	add_definitions(-D__WEB_GL__)
	add_definitions(-DMOBILE_SHADOWS)

	#include_directories( /home/michael/emscripten/system/include/libcxx)
	#set (CMAKE_CXX_FLAGS "-nostdinc++")

	#set assets directory to be baked in
	set(CMAKE_CXX_FLAGS "-Wno-warn-absolute-paths -O3 -DNDEBUG --preload-file Data/ --llvm-opts 2 --llvm-lto 2")
	set(CMAKE_C_FLAGS "-Wno-warn-absolute-paths -O3 -DNDEBUG")
      
	set(CMAKE_EXE_LINKER_FLAGS "-O3")
	set(CMAKE_SHARED_LINKER_FLAGS "-O3")
	set(CMAKE_MODULE_LINKER_FLAGS "-O3")

	#always release because i dont know how to debug this, and you dont either.
	#also define the memory we are going to need, as arrays cannot be resized in javascript
	SET(linkFlags "-O2 --llvm-opts 3 -s TOTAL_MEMORY=54000000") 

	#all engine specific code goes here
	SET(Engine_SRCS TMXLoader.cpp
		Builder.cpp CharacterController.cpp Materials.cpp Physics.cpp ObjectManager.cpp
		Terrain.cpp GamePad.cpp
		Camera.cpp Entity.cpp EngineMath.cpp Input.cpp Utilities.cpp BoundingVolumes.cpp
		Renderer.cpp webgl_main.cpp
	)


	#this will build a test html file see emscripten/tests/cmake directory for .js build options
	#SET(CMAKE_EXECUTABLE_SUFFIX ".html")
	SET(CMAKE_EXECUTABLE_SUFFIX ".js")

	add_executable(webgl_build 
					

					${FREETYPE_GL_SRC}
					${SOIL_SRCS}
					${Root_HDRS}
					${BulletDynamics_SRCS}
					${BulletCollision_SRCS}
					${LinearMath_SRCS}
					${LinearMath_HDRS}
					TinyXML2/tinyxml2.cpp
					${Utility_SRCS}
					${Sound_SRCS}
					${Resources_SRCS}
					${Components_SRCS}
					${UserInterface_SRCS}
					${Engine_SRCS}
					${GameCode_SRCS}
					
					) 

	set_target_properties(webgl_build PROPERTIES LINK_FLAGS "${linkFlags}")

	#find_package( X11 REQUIRED )
	target_link_libraries( webgl_build  ${X11_LIBRARIES} )

	install(TARGETS webgl_build RUNTIME DESTINATION bin)

	# Validating asm.js requires SpiderMonkey JS VM - detect its presence via the SPIDERMONKEY environment variable.
	#if (DEFINED ENV{SPIDERMONKEY})
	#	MESSAGE ( STATUS "================================" )
	#	MESSAGE ( STATUS "=    Validating asm.js file    =" )
	#	MESSAGE ( STATUS "================================" )
	#	em_validate_asmjs_after_build(cmake-emscripten)
	#endif()
else()
	MESSAGE ( STATUS "================================" )
	MESSAGE ( STATUS "=   building GNU C++ makefile  =" )
	MESSAGE ( STATUS "================================" )
	#all engine specific code goes here
	SET(Engine_SRCS TMXLoader.cpp
		Builder.cpp CharacterController.cpp Materials.cpp Physics.cpp ObjectManager.cpp
		Terrain.cpp GamePad.cpp
		Camera.cpp Entity.cpp EngineMath.cpp Input.cpp Utilities.cpp BoundingVolumes.cpp 
		Renderer.cpp Scripting.cpp linux_main.cpp
	)

	

	#compiling options  -pg is for profiling info
	#add_definitions(-pg) 

	#will tell renderer to use mobile version of shadows
	add_definitions(-DMOBILE_SHADOWS)

#${AngelScript_SRCS}
	add_executable(tableflipengine 
			
			${FREETYPE_GL_SRC}
			${SOIL_SRCS}
			${Root_HDRS}
			${BulletDynamics_SRCS}
			${BulletCollision_SRCS}
			${LinearMath_SRCS}
			${LinearMath_HDRS}
			TinyXML2/tinyxml2.cpp
			${Utility_SRCS}
			${UserInterface_SRCS}
			${Sound_SRCS}
			${Resources_SRCS}
			${Components_SRCS}
			${Networking_SRCS}
			${GameCode_SRCS}
			${Engine_SRCS}

			)


	#make sure we have the dependencies
	MESSAGE ( STATUS "Searching for Dependencies..." )
	find_package( OpenGL REQUIRED )
	find_package( X11 REQUIRED )

	#find_package( OpenAL REQUIRED ) #Moved to prebuilt lib folder
	find_package( Threads REQUIRED )
	find_package( Freetype REQUIRED )

	

	# link the libs!
	MESSAGE ( STATUS "Linking Dependencies..." )
	target_link_libraries( tableflipengine ${CMAKE_THREAD_LIBS_INIT} )
	target_link_libraries( tableflipengine ${CMAKE_SOURCE_DIR}/lib/libGLEW.a )
	#target_link_libraries( tableflipengine ${CMAKE_SOURCE_DIR}/lib/libangelscript.a )
	#target_link_libraries( tableflipengine ${CMAKE_SOURCE_DIR}/lib/libangelscript.a )
	target_link_libraries( tableflipengine ${CMAKE_SOURCE_DIR}/lib/libopenal.so )
	target_link_libraries( tableflipengine ${CMAKE_SOURCE_DIR}/lib/libalut.a )
	target_link_libraries( tableflipengine ${FREETYPE_LIBRARY})

	#now code is directly compiled, not library linked
	#target_link_libraries( tableflipengine ${CMAKE_SOURCE_DIR}/lib/libSOIL.a )
	#target_link_libraries( tableflipengine ${CMAKE_SOURCE_DIR}/lib/libBulletDynamics.a )
	#target_link_libraries( tableflipengine ${CMAKE_SOURCE_DIR}/lib/libBulletCollision.a )
	#target_link_libraries( tableflipengine ${CMAKE_SOURCE_DIR}/lib/libLinearMath.a )
	#target_link_libraries( tableflipengine ${CMAKE_SOURCE_DIR}/lib/libBulletSoftBody.a )


	target_link_libraries( tableflipengine ${OPENGL_LIBRARIES} ${X11_LIBRARIES} ${X11_LIBRARY_DIR}/libXi.so )
endif()
