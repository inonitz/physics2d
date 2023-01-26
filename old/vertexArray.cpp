#include "vertexArray.hpp"
#include "glad/glad.h"




u32 BufferType::glEnum() const 
{
	ifcrashdbg(value == DEFAULT8);

	constexpr std::array<u32, 7> convert = { GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_TRANSFORM_FEEDBACK_BUFFER, GL_UNIFORM_BUFFER, GL_DRAW_INDIRECT_BUFFER, GL_DISPATCH_INDIRECT_BUFFER, GL_SHADER_STORAGE_BUFFER };
	return convert[value];
}


const char* BufferType::to_str() const 
{
	ifcrashdbg(value == DEFAULT8);

	constexpr std::array<const char*, 7> convert = { "GL_ARRAY_BUFFER", "GL_ELEMENT_ARRAY_BUFFER", "GL_TRANSFORM_FEEDBACK_BUFFER", "GL_UNIFORM_BUFFER", "GL_DRAW_INDIRECT_BUFFER,", "GL_DISPATCH_INDIRECT_BUFFER", "GL_SHADER_STORAGE_BUFFER" };
	return convert[value];
}




u32 VertexDataType::glEnum() const
{
	ifcrashdbg(value == DEFAULT8);
	
	constexpr std::array<u32, 5> convert = {GL_FLOAT, GL_UNSIGNED_INT, GL_INT, GL_UNSIGNED_SHORT, GL_SHORT };
	return convert[value & ~(1 << 7)];
}


u32 VertexDataType::bytes() const
{
	ifcrashdbg(value == DEFAULT8);

	constexpr std::array<u32, 5> convert = { 4, 4, 4, 2, 2 };
	return convert[value & ~(1 << 7)];
}


const char* VertexDataType::to_str() const
{
	ifcrashdbg(value == DEFAULT8);

	constexpr std::array<const char*, 5> convert = { "GL_FLOAT", "GL_UNSIGNED_INT", "GL_INT", "GL_UNSIGNED_SHORT", "GL_SHORT" };
	return convert[value & ~(1 << 7)];
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
	glNamedBufferData(id, meta.bytes(), buffer, usage);
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


void Buffer::print()
{
	printf("GL Buffer: %u\n    Metadata:\n        Buffer Type: %s\n        Vertex Type: %s (x%u)\n", id, meta.btype.to_str(), meta.vtype.to_str(), meta.count);
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
	u32 		  			        	 VAObindingPoint, 
	Buffer const& 			        	 buf, 
	std::vector<VertexDescriptor> const& descriptors
) {
	BufferLayoutDescription full = {
		{ buf.id, VAObindingPoint, buf.meta },
		descriptors
	};
	u32 stride = 0;


	/* get the stride */
	for(auto& desc : descriptors) { 
		stride += desc.bytes();
	}


	BufferDescriptions.push(full);
	switch(buf.meta.btype.value) {
		case BUFFER_TYPE_ARRAY: 		glVertexArrayVertexBuffer(id, VAObindingPoint, buf.id, 0, stride);
		break;
		case BUFFER_TYPE_ELEMENT_ARRAY: glVertexArrayElementBuffer(id, buf.id);
		break;
	}

	for(size_t i = 0; i < descriptors.size(); ++i)
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