#include "framebuffer.hpp"




Framebuffer::Framebuffer(FramebufferDescriptor const& desc)
{
	glCreateFramebuffers(1, &id);
	GL_DRAW_FRAMEBUFFER
}