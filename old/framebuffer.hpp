#pragma once
#include "vec.hpp"
#include <glad/glad.h>




struct FramebufferDescriptor
{
	u32 texID;
	u8  bitfield; /* bit 0 : Read/Write */
};


struct Framebuffer
{
public:


	Framebuffer(FramebufferDescriptor const& desc);



private:
	u32 id;
	u32 type;
	math::vec2u dims;
};