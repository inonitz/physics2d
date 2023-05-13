#pragma once
#include "util/base.hpp"
#include <vector>




struct VertexDescriptor
{
	struct DataType
	{
		u16 gltype;
		u8  count;
	};
	std::vector<DataType> attributes;


	u8  count   (u8 attributeIndex) const { return attributes[attributeIndex].count; }
	u32 size    (u8 attributeIndex) const;
	u32 typeSize(u8 attributeIndex) const;
	u32 offset  (u8 attributeIndex) const;


	u8     attributeCount() const { return attributes.size(); }
	size_t totalSize() 		const;


};


struct BufferDescriptor
{
	void* 			 data;
	u32   			 count;
	VertexDescriptor vinfo;
};




struct Buffer
{
public:
	Buffer() = default;
	
	void create(BufferDescriptor const& info, u32 usage);
	void destroy();


protected:
	u32 m_id;
	BufferDescriptor m_info;


	friend struct VertexArray;
	friend struct ShaderStorageBuffer;
};


struct ShaderStorageBuffer
{
public:
	void create(BufferDescriptor const& info, u32 usage);
	void destroy();


	void setBindingIndex(u32 binding);
	void clearBindingIndex();
	void bind();
	void unbind();
private:
	Buffer m_base;
	u32    m_bindingPoint;
};


struct VertexArray
{
public:
	VertexArray() = default;


	void create(Buffer& Vertices, Buffer& Indices);
	void destroy();
	void bind();
	void unbind();

protected:
	u32 m_vao, m_vbo, m_ebo;
};