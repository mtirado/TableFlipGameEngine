#builds a JNI library to run on the android operating system


#LOCAL_PATH := $(call my-dir)
MY_LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PATH := $(MY_LOCAL_PATH)/../../ 
#painfully long list of source files....
SRC_DIR := $(LOCAL_PATH)TableFlipEngine
INC_DIR := $(NDK_APP_PROJECT_PATH)/../TableFlipEngine
#INC_DIR := $(NDK_APP_PROJECT_PATH)/TableFlipEngine
#SRC_DIR := $(INC_DIR)
BULLET_SRCS := 	$(SRC_DIR)/bullet/BulletDynamics/Character/btKinematicCharacterController.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btConeTwistConstraint.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btContactConstraint.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btGearConstraint.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btGeneric6DofConstraint.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btGeneric6DofSpringConstraint.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btHinge2Constraint.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btHingeConstraint.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btPoint2PointConstraint.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btSliderConstraint.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btSolve2LinearConstraint.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btTypedConstraint.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/ConstraintSolver/btUniversalConstraint.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/Dynamics/btDiscreteDynamicsWorld.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/Dynamics/btRigidBody.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/Dynamics/btSimpleDynamicsWorld.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/Dynamics/Bullet-C-API.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/Vehicle/btRaycastVehicle.cpp \
				$(SRC_DIR)/bullet/BulletDynamics/Vehicle/btWheelInfo.cpp \
				$(SRC_DIR)/bullet/BulletCollision/BroadphaseCollision/btAxisSweep3.cpp \
				$(SRC_DIR)/bullet/BulletCollision/BroadphaseCollision/btBroadphaseProxy.cpp \
				$(SRC_DIR)/bullet/BulletCollision/BroadphaseCollision/btCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/BroadphaseCollision/btDbvt.cpp \
				$(SRC_DIR)/bullet/BulletCollision/BroadphaseCollision/btDbvtBroadphase.cpp \
				$(SRC_DIR)/bullet/BulletCollision/BroadphaseCollision/btDispatcher.cpp \
				$(SRC_DIR)/bullet/BulletCollision/BroadphaseCollision/btMultiSapBroadphase.cpp \
				$(SRC_DIR)/bullet/BulletCollision/BroadphaseCollision/btOverlappingPairCache.cpp \
				$(SRC_DIR)/bullet/BulletCollision/BroadphaseCollision/btQuantizedBvh.cpp \
				$(SRC_DIR)/bullet/BulletCollision/BroadphaseCollision/btSimpleBroadphase.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btActivatingCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btBoxBoxCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btBox2dBox2dCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btBoxBoxDetector.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btCollisionDispatcher.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btCollisionObject.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btCollisionWorld.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btCompoundCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btCompoundCompoundCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btConvexConcaveCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btConvexConvexAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btConvexPlaneCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btConvex2dConvex2dAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btEmptyCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btGhostObject.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btHashedSimplePairCache.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btInternalEdgeUtility.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btInternalEdgeUtility.h \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btManifoldResult.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btSimulationIslandManager.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btSphereBoxCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btSphereSphereCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btSphereTriangleCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/btUnionFind.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionDispatch/SphereTriangleDetector.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btBoxShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btBox2dShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btBvhTriangleMeshShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btCapsuleShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btCollisionShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btCompoundShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btConcaveShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btConeShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btConvexHullShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btConvexInternalShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btConvexPointCloudShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btConvexPolyhedron.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btConvexShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btConvex2dShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btConvexTriangleMeshShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btCylinderShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btEmptyShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btHeightfieldTerrainShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btMinkowskiSumShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btMultimaterialTriangleMeshShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btMultiSphereShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btOptimizedBvh.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btPolyhedralConvexShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btScaledBvhTriangleMeshShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btShapeHull.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btSphereShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btStaticPlaneShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btStridingMeshInterface.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btTetrahedronShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btTriangleBuffer.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btTriangleCallback.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btTriangleIndexVertexArray.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btTriangleIndexVertexMaterialArray.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btTriangleMesh.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btTriangleMeshShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/CollisionShapes/btUniformScalingShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/Gimpact/btContactProcessing.cpp \
				$(SRC_DIR)/bullet/BulletCollision/Gimpact/btGenericPoolAllocator.cpp \
				$(SRC_DIR)/bullet/BulletCollision/Gimpact/btGImpactBvh.cpp \
				$(SRC_DIR)/bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.cpp \
				$(SRC_DIR)/bullet/BulletCollision/Gimpact/btGImpactQuantizedBvh.cpp \
				$(SRC_DIR)/bullet/BulletCollision/Gimpact/btGImpactShape.cpp \
				$(SRC_DIR)/bullet/BulletCollision/Gimpact/btTriangleShapeEx.cpp \
				$(SRC_DIR)/bullet/BulletCollision/Gimpact/gim_box_set.cpp \
				$(SRC_DIR)/bullet/BulletCollision/Gimpact/gim_contact.cpp \
				$(SRC_DIR)/bullet/BulletCollision/Gimpact/gim_memory.cpp \
				$(SRC_DIR)/bullet/BulletCollision/Gimpact/gim_tri_collision.cpp \
				$(SRC_DIR)/bullet/BulletCollision/NarrowPhaseCollision/btContinuousConvexCollision.cpp \
				$(SRC_DIR)/bullet/BulletCollision/NarrowPhaseCollision/btConvexCast.cpp \
				$(SRC_DIR)/bullet/BulletCollision/NarrowPhaseCollision/btGjkConvexCast.cpp \
				$(SRC_DIR)/bullet/BulletCollision/NarrowPhaseCollision/btGjkEpa2.cpp \
				$(SRC_DIR)/bullet/BulletCollision/NarrowPhaseCollision/btGjkEpaPenetrationDepthSolver.cpp \
				$(SRC_DIR)/bullet/BulletCollision/NarrowPhaseCollision/btGjkPairDetector.cpp \
				$(SRC_DIR)/bullet/BulletCollision/NarrowPhaseCollision/btMinkowskiPenetrationDepthSolver.cpp \
				$(SRC_DIR)/bullet/BulletCollision/NarrowPhaseCollision/btPersistentManifold.cpp \
				$(SRC_DIR)/bullet/BulletCollision/NarrowPhaseCollision/btRaycastCallback.cpp \
				$(SRC_DIR)/bullet/BulletCollision/NarrowPhaseCollision/btSubSimplexConvexCast.cpp \
				$(SRC_DIR)/bullet/BulletCollision/NarrowPhaseCollision/btVoronoiSimplexSolver.cpp \
				$(SRC_DIR)/bullet/BulletCollision/NarrowPhaseCollision/btPolyhedralContactClipping.cpp \
				$(SRC_DIR)/bullet/LinearMath/btAlignedAllocator.cpp \
				$(SRC_DIR)/bullet/LinearMath/btConvexHull.cpp \
				$(SRC_DIR)/bullet/LinearMath/btConvexHullComputer.cpp \
				$(SRC_DIR)/bullet/LinearMath/btGeometryUtil.cpp \
				$(SRC_DIR)/bullet/LinearMath/btPolarDecomposition.cpp \
				$(SRC_DIR)/bullet/LinearMath/btQuickprof.cpp \
				$(SRC_DIR)/bullet/LinearMath/btSerializer.cpp \
				$(SRC_DIR)/bullet/LinearMath/btVector3.cpp

SOIL_SRCS :=	$(SRC_DIR)/SOIL/image_helper.c \
				$(SRC_DIR)/SOIL/stb_image_aug.c \
				$(SRC_DIR)/SOIL/image_DXT.c \
				$(SRC_DIR)/SOIL/SOIL.c

#$(SRC_DIR)/TMXLoader.cpp $(SRC_DIR)/SoundResource.cpp 
ENGINE_SRCS := 	$(SRC_DIR)/Builder.cpp \
  				$(SRC_DIR)/CharacterController.cpp \
   				$(SRC_DIR)/UserInterface.cpp \
   				$(SRC_DIR)/Materials.cpp \
   				$(SRC_DIR)/Physics.cpp \
   				$(SRC_DIR)/ModelResource.cpp \
   				$(SRC_DIR)/ObjectManager.cpp \
				$(SRC_DIR)/AnimationResource.cpp \
				$(SRC_DIR)/Terrain.cpp \
				$(SRC_DIR)/ShaderResource.cpp \
				$(SRC_DIR)/GamePad.cpp \
				$(SRC_DIR)/Resources.cpp \
				$(SRC_DIR)/BoundingVolumes.cpp \
				$(SRC_DIR)/Camera.cpp \
				$(SRC_DIR)/Entity.cpp \
				$(SRC_DIR)/EngineMath.cpp \
				$(SRC_DIR)/Input.cpp \
				$(SRC_DIR)/Utilities.cpp \
				$(SRC_DIR)/Renderer.cpp \
				$(SRC_DIR)/android_main.cpp

COMPONENT_SRCS := 	$(SRC_DIR)/Components/Components.cpp \
					$(SRC_DIR)/Components/Logic.cpp \
					$(SRC_DIR)/Components/Transform3d.cpp \
			    	$(SRC_DIR)/Components/Mesh.cpp \
			    	$(SRC_DIR)/Components/SkeletalMesh.cpp \
			    	$(SRC_DIR)/Components/AnimationController.cpp \
			    	$(SRC_DIR)/Components/Sprite.cpp

UTILITY_SRCS := $(SRC_DIR)/utilities/b64_ntop.cpp \
				$(SRC_DIR)/utilities/Logger.cpp \
				$(SRC_DIR)/utilities/sha1.cpp \
				$(SRC_DIR)/utilities/fmemopen.c

NETWORKING_SRCS := 	$(SRC_DIR)/yaknet/NetEvents.cpp \
					$(SRC_DIR)/yaknet/NetBuffer.cpp \
					$(SRC_DIR)/yaknet/Connection.cpp \
					$(SRC_DIR)/yaknet/TCPBuffer.cpp \
					$(SRC_DIR)/yaknet/WebSockets.cpp

SOUND_SRCS := 	$(SRC_DIR)/Sound/SynthEffect.cpp \
				$(SRC_DIR)/Sound/SoundSystem.cpp \
				$(SRC_DIR)/Sound/WaveGen.cpp

GAME_SRCS := 	$(SRC_DIR)/GameCode/AndroidTest.cpp

#$(NETWORKING_SRCS) $(SOUND_SRCS) 
SOURCES := 		$(BULLET_SRCS) \
				$(SOIL_SRCS) \
				$(UTILITY_SRCS) \
				$(COMPONENT_SRCS) \
				$(ENGINE_SRCS) \
				$(GAME_SRCS)


include $(CLEAR_VARS)

LOCAL_MODULE    := native-activity
#include dir
#$(INC_DIR)/yaknet
LOCAL_C_INCLUDES := $(INC_DIR) \
					$(INC_DIR)/bullet \
					$(INC_DIR)/DataStructures \
					$(INC_DIR)/utilities \
					$(INC_DIR)/GameCode \
					$(INC_DIR)/Sound \
					$(INC_DIR)/Components \
					$(INC_DIR)/SOIL \
					$(INC_DIR)/TinyXML2
					 
					
LOCAL_SRC_FILES := 	$(SOURCES)
LOCAL_LDLIBS    := -llog -landroid -lEGL -lGLESv2
LOCAL_STATIC_LIBRARIES := android_native_app_glue

include $(BUILD_SHARED_LIBRARY)

$(call import-module,android/native_app_glue)
