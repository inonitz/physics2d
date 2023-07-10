#include "renderer.hpp"




namespace Renderer {


void RenderManager::render() const
{
    auto* ctx = getGlobalContext();
    ElementBufferRenderData currentIBOdata;
    for(auto const& obj : targets)
    {
        obj->vertexData->bind();
        obj->texture->bindToUnit(0);
        ctx->shader.uniformMatrix4fv("TRS", obj->transform->TRS.begin());

        currentIBOdata = obj->vertexData->getRenderData();
        glDrawElements(GL_TRIANGLES, currentIBOdata.count, currentIBOdata.gl_type, nullptr);
    }


    return;
}




}