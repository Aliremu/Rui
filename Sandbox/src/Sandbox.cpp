#include <Rui.h>
#include <stdio.h>
  
#include <PxConfig.h>
#include <PxPhysics.h>
#include <PxScene.h>
#include <PxPhysicsAPI.h>
  
using namespace physx;

class GameScene : public Rui::Scene {
public:
    PxDefaultAllocator		gAllocator;
    PxDefaultErrorCallback	gErrorCallback;

    PxFoundation* gFoundation = NULL;
    PxPhysics* gPhysics = NULL;
    PxCooking* gCooking = NULL;

    PxDefaultCpuDispatcher* gDispatcher = NULL;
    PxScene* gScene = NULL;

    PxMaterial* gMaterial = NULL;

    PxPvd* gPvd = NULL;

    PxReal stackZ = 10.0f;

    std::vector<PxRigidDynamic*> objects;

    GameScene() {
        gFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, gAllocator, gErrorCallback);

        gPvd = PxCreatePvd(*gFoundation);
        PxPvdTransport* transport = PxDefaultPvdSocketTransportCreate("127.0.0.1", 5425, 10);
        gPvd->connect(*transport, PxPvdInstrumentationFlag::eALL);

        gPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *gFoundation, PxTolerancesScale(), true, gPvd);
        gCooking = PxCreateCooking(PX_PHYSICS_VERSION, *gFoundation, PxCookingParams(PxTolerancesScale()));

        PxSceneDesc sceneDesc(gPhysics->getTolerancesScale());
        sceneDesc.gravity = PxVec3(0.0f, -1.81f, 0.0f);
        gDispatcher = PxDefaultCpuDispatcherCreate(2);
        sceneDesc.cpuDispatcher = gDispatcher;
        sceneDesc.filterShader = PxDefaultSimulationFilterShader;
        gScene = gPhysics->createScene(sceneDesc);

        PxPvdSceneClient* pvdClient = gScene->getScenePvdClient();
        if(pvdClient) {
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_CONTACTS, true);
            pvdClient->setScenePvdFlag(PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true);
        }
        gMaterial = gPhysics->createMaterial(0.5f, 0.5f, 0.9f);

        PxRigidStatic* groundPlane = PxCreatePlane(*gPhysics, PxPlane(0, 1, 0, 0), *gMaterial);
        gScene->addActor(*groundPlane);

        if(!false)
            CreateDynamic(PxTransform(PxVec3(0, 2, 50)), PxSphereGeometry(1), PxVec3(0, -5, -10));

        for(PxU32 i = 0; i < 5; i++)
            CreateStack(PxTransform(PxVec3(0, 0, stackZ -= 5.0f)), 10, 1.0f);
    };

    ~GameScene() {
        gCooking->release();
        gPhysics->release();
        gFoundation->release();
    };

    PxRigidDynamic* CreateDynamic(const PxTransform& t, const PxGeometry& geometry, const PxVec3& velocity) {
        PxRigidDynamic* dynamic = PxCreateDynamic(*gPhysics, t, geometry, *gMaterial, 100.0f);
        dynamic->setAngularDamping(0.5f);
        dynamic->setLinearVelocity(velocity);
        gScene->addActor(*dynamic);
        objects.push_back(dynamic);
        return dynamic;
    }

    void CreateStack(const PxTransform& t, PxU32 size, PxReal halfExtent) {
        PxShape* shape = gPhysics->createShape(PxBoxGeometry(halfExtent, halfExtent, halfExtent), *gMaterial);
        for(PxU32 i = 0; i < size; i++) {
            for(PxU32 j = 0; j < size - i; j++) {
                PxTransform localTm(PxVec3(PxReal(j * 2) - PxReal(size - i), PxReal(i * 2 + 1), 0) * halfExtent);
                PxRigidDynamic* body = gPhysics->createRigidDynamic(t.transform(localTm));
                body->attachShape(*shape);
                objects.push_back(body);
                PxRigidBodyExt::updateMassAndInertia(*body, 1.0f);
                gScene->addActor(*body);
            }
        }
        shape->release();
    }

    void OnUpdate(const Rui::Timestep& ts) override {
        //RUI_TRACE("{0}s Timestep", 1);
        
        gScene->simulate((PxReal) ts.m_dt);
        gScene->fetchResults(true);
    }

    void OnEvent(Rui::Event& event) override {}
    void OnLoad() override {}
    void OnUnload() override {}
	void OnRender(const Rui::Timestep& ts) override {
        Rui::RenderSystem::DrawTriangle();
    }
};

class Sandbox : public Rui::Application {
public:
	
	Sandbox() : Rui::Application("Hello College Person :)", 1280, 720) {
		LoadScene(new GameScene());
	}

	~Sandbox() {
        
	}
};

Rui::Application* Rui::CreateApplication() {
	return new Sandbox();
}