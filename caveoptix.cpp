#include <omega.h>
#include <omegaGl.h>
#include <iostream>
#include <vector>

#include "app.h"

using namespace std;
using namespace omega;

///////////////////////////////////////////////////////////////////////////////
class OptixRenderModule : public EngineModule
{
public:
    OptixRenderModule() :
        EngineModule("OptixRenderModule"), visible(true), app(0), initalized(false)
    {

    }

    virtual void initializeRenderer(Renderer* r);

    virtual void update(const UpdateContext& context)
    {
        // After a frame all render passes had a chance to update their
        // textures. reset the raster update flag.       
    }
    
    virtual void dispose()
    {

    }

    void initOptix() {
        app = new OptixApp();
    }

    bool visible;
    bool initalized;
    OptixApp* app;
};

///////////////////////////////////////////////////////////////////////////////
class OptixRenderPass : public RenderPass
{
public:
    OptixRenderPass(Renderer* client, OptixRenderModule* prm) : 
        RenderPass(client, "OptixRenderPass"), 
        module(prm) {}
    
    virtual void initialize()
    {
        RenderPass::initialize();
    }

    virtual void render(Renderer* client, const DrawContext& context)
    {
    	if(context.task == DrawContext::SceneDrawTask)
        {
            glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
            client->getRenderer()->beginDraw3D(context);

            if(!module->initalized && module->app) {
                module->app->init(context.viewport.width(), context.viewport.height());
                module->initalized = true;
            }
	
    	    if(module->initalized && module->visible)
    	    { 
		        Camera* cam = context.camera;

                Vector3f cp = context.camera->getPosition();
                float campos[3] = {cp[0], cp[1], cp[2]};
                float MV[16], P[16];
                for (int i=0; i < 4; i++) {
                    for(int j=0; j < 4; j++) {
                        MV[i*4+j] = context.modelview(j, i);
                        P[i*4+j] = context.projection(j, i);
                    }
                }
                
		        module->app->display(MV, P, campos);

                if(oglError) return;
    	    }
            
            client->getRenderer()->endDraw();
            glPopAttrib();
        }
        
    }

private:
    OptixRenderModule* module;

};

///////////////////////////////////////////////////////////////////////////////
void OptixRenderModule::initializeRenderer(Renderer* r)
{
    r->addRenderPass(new OptixRenderPass(r, this));
}

///////////////////////////////////////////////////////////////////////////////
OptixRenderModule* initialize()
{
    OptixRenderModule* prm = new OptixRenderModule();
    ModuleServices::addModule(prm);
    prm->doInitialize(Engine::instance());
    return prm;
}

///////////////////////////////////////////////////////////////////////////////
// Python API
#include "omega/PythonInterpreterWrapper.h"
BOOST_PYTHON_MODULE(caveoptix)
{
    //
    PYAPI_REF_BASE_CLASS(OptixRenderModule)
    PYAPI_METHOD(OptixRenderModule, initOptix)
    ;

    def("initialize", initialize, PYAPI_RETURN_REF);
}
