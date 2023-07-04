#include "context.hpp"




static globalContext ctx;


globalContext* getGlobalContext() { return &ctx; }




void PVMTransform::create(
    f32 fieldOfView,
    f32 nearClippingPlane,
    f32 farClippingPlane,
    math::vec3f const& initialPosition,
    math::vec3f const& initialDirection
) {
    m_persp.create({ 
        ctx.glfw.aspectRatio<f32>(), 
        fieldOfView, 
        nearClippingPlane, 
        farClippingPlane 
    });
    m_view.create(
        initialPosition, 
        initialDirection, 
        { 5.0f, 0.3f }
    );
    return;
}


void PVMTransform::onUpdate(f32 dt)
{
    m_view.onUpdate(dt);
    return;
}


void PVMTransform::recalculateProjection()
{
    m_persp.recalculate();
    math::inv_perspective(m_persp.constref(), m_data.invPerspective);
    return;
}


void PVMTransform::writeFinalData(CameraTransformV2& in)
{
    m_data.position = m_view.position();
    math::inverseSimd(m_view.constref(), m_data.invView);
    memcpy(&in, &m_data, sizeof(CameraTransformV2));
    return;
}