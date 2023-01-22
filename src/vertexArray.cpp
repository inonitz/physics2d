#include "vertexArray.hpp"
#include "glad/glad.h"




#define BUFFER_TYPE_MAX_VALUE     ((u8)(0x07 - 1) )
#define VERTEX_DATATYPE_MAX_VALUE ((u8)(0x06 - 1) )


u32 BufferType::glEnum() const 
{
	ifcrashdbg(value == DEFAULT8);

	constexpr std::array<u32, 7> convert = { GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER, GL_UNIFORM_BUFFER, GL_DRAW_INDIRECT_BUFFER, GL_DISPATCH_INDIRECT_BUFFER, GL_SHADER_STORAGE_BUFFER };
	return convert[value & BUFFER_TYPE_MAX_VALUE];
}


const char* BufferType::to_str() const 
{
	ifcrashdbg(value == DEFAULT8);

	constexpr std::array<const char*, 7> convert = { "GL_ARRAY_BUFFER", "GL_ELEMENT_ARRAY_BUFFER", "GL_TRANSFORM_FEEDBACK_BUFFER", "GL_UNIFORM_BUFFER", "GL_DRAW_INDIRECT_BUFFER,", "GL_DISPATCH_INDIRECT_BUFFER", "GL_SHADER_STORAGE_BUFFER" };
	return convert[value & BUFFER_TYPE_MAX_VALUE];
}




u32 VertexDataType::glEnum() const
{
	ifcrashdbg(value == DEFAULT8);

	constexpr std::array<u32, 5> convert = {GL_FLOAT, GL_UNSIGNED_INT, GL_INT, GL_UNSIGNED_SHORT, GL_SHORT };
	return convert[value & VERTEX_DATATYPE_MAX_VALUE];
}


u32 VertexDataType::bytes() const
{
	ifcrashdbg(value == DEFAULT8);

	constexpr std::array<u32, 5> convert = { 4, 4, 4, 2, 2 };
	return convert[value & VERTEX_DATATYPE_MAX_VALUE];
}


const char* VertexDataType::to_str() const
{
	ifcrashdbg(value == DEFAULT8);

	constexpr std::array<const char*, 5> convert = { "GL_FLOAT", "GL_UNSIGNED_INT", "GL_INT", "GL_UNSIGNED_SHORT", "GL_SHORT" };
	return convert[value & VERTEX_DATATYPE_MAX_VALUE];
}




void Buffer::create(
	BufferType 	   btype,
	BufferUsageBit usage,
	VertexDataType vdt, 
	u32 		   vcount, 
	void* 		   buffer
) {
	meta = { vdt, vcount, btype };

	glCreateBuffers(1, &id);
	glNamedBufferStorage(id, meta.bytes(), buffer, usage);
	return;
}

void Buffer::destroy() 
{ 
	ifcrashdo(id == DEFAULT32, { 
		printf("VertexBuffer::destroy() => Can't Destroy OpenGL Buffer Object that hasn't been created\n"); 
	});
	glDeleteBuffers(1, &id);
	id = DEFAULT32;
	return; 
}


void VertexArray::create()
{
	glCreateVertexArrays(1, &id);
	return;
}


void VertexArray::destroy() 
{ 
	ifcrashdo(id == DEFAULT32, { 
		printf("VertexArray::destroy() => Can't Destroy OpenGL Object[VertexArray] that hasn't been created\n"); 
	}); 
	glDeleteVertexArrays(1, &id); 
	return; 
}


VertexArray& VertexArray::addBuffer(
	u32 		  			        VAObindingPoint, 
	Buffer const& 			        buf, 
	manvec<VertexDescriptor> const& descriptors
) {
	BufferLayoutDescription full = {
		{ buf.id, VAObindingPoint, buf.meta },
		{}
	};
	u32 stride = 0;

	/* get the stride */
	for(auto& desc : descriptors) { 
		stride += desc.bytes();
	}

	/* copy var descriptors into full.bufferLayout because manvec doesn't do implicit copy constructs. */
	/* push result to description stack */
	full.bufferLayout.copy(descriptors);
	BufferDescriptions.push_back(full);
	
	switch(buf.meta.btype.value) {
		case BUFFER_TYPE_ARRAY: 		glVertexArrayVertexBuffer(id, VAObindingPoint, buf.id, 0, stride);
		break;
		case BUFFER_TYPE_ELEMENT_ARRAY: glVertexArrayElementBuffer(id, buf.id);
		break;
	}
	for(size_t i = 0; i < descriptors.len(); ++i)
	{
		glVertexArrayAttribBinding(id, descriptors[i].shaderAttributeIndex, VAObindingPoint);
		glVertexArrayAttribFormat(
			id,
			descriptors[i].shaderAttributeIndex,
			descriptors[i].len,
			descriptors[i].vtype.glEnum(),
			descriptors[i].vtype.value & 0b10000000,
			descriptors[i].relativeOffset
		);
		glEnableVertexArrayAttrib(id, descriptors[i].shaderAttributeIndex);
	}


	return *this;
}


void VertexArray::bind()   { glBindVertexArray(id); return; }
void VertexArray::unbind() { glBindVertexArray(0);  return; }