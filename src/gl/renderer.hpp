#pragma once
#include "context.hpp"
#include "gl/vertexArray.hpp"
#include "gl/texture.hpp"




namespace Renderer {


struct Transform
{
    math::vec2f position;
    math::vec2f rotateAngle;
    math::vec2f scale;
    math::mat4f TRS; /* Translate Rotate Scale */
};
using TransformHandle = imut_type_handle<Transform>;
using TextureHandle   = value_ptr<TextureBuffer>;
using VAOHandle       = imut_type_handle<VertexArray>;


struct RenderTarget
{
    VAOHandle       vertexData;
    TextureHandle   texture;
    TransformHandle transform;
};
using RenderHandle  = imut_type_handle<RenderTarget>;




class RenderManager {
    std::vector<RenderHandle> targets;

public:
    void create();
    void destroy();

    void push(RenderHandle toRender) { targets.push_back(toRender); return; }
    void pop()                       { targets.pop_back();          return; }
    void render() const;
    void renderImGui();
};




}