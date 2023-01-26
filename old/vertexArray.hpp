#pragma once
#include "base.hpp"
#include "glad/glad.h"
#include <stack>
#include <vector>



#define BUFFER_USAGE_STATIC_DRAW      ( (u32)GL_STATIC_DRAW	 )
#define BUFFER_USAGE_STREAM_DRAW      ( (u32)GL_STREAM_DRAW	 )
#define BUFFER_USAGE_DYNAMIC_DRAW     ( (u32)GL_DYNAMIC_DRAW )
#define BUFFER_USAGE_STATIC_READ      ( (u32)GL_STATIC_READ	  )
#define BUFFER_USAGE_STREAM_READ      ( (u32)GL_STREAM_READ	  )
#define BUFFER_USAGE_DYNAMIC_READ     ( (u32)GL_DYNAMIC_READ  )
#define BUFFER_USAGE_STATIC_COPY      ( (u32)GL_STATIC_COPY	   )
#define BUFFER_USAGE_STREAM_COPY      ( (u32)GL_STREAM_COPY	   )
#define BUFFER_USAGE_DYNAMIC_COPY     ( (u32)GL_DYNAMIC_COPY   ) 
#define BUFFER_USAGE_BIT_DYNAMIC 	  ( (u16)GL_DYNAMIC_STORAGE_BIT			  )
#define BUFFER_USAGE_BIT_READ  		  ( (u16)GL_MAP_READ_BIT  				  )
#define BUFFER_USAGE_BIT_WRITE 		  ( (u16)GL_MAP_WRITE_BIT 				  )
#define BUFFER_USAGE_BIT_MAP_PERSIST  ( (u16)GL_MAP_PERSISTENT_BIT 			  )
#define BUFFER_USAGE_BIT_MAP_COHERENT ( (u16)GL_MAP_COHERENT_BIT   			  )
#define BUFFER_USAGE_BIT_CLIENT       ( (u16)and GL_CLIENT_STORAGE_BIT        )

#define BUFFER_TYPE_ARRAY   		   ( (u8)0 )
#define BUFFER_TYPE_ELEMENT_ARRAY 	   ( (u8)1 )
#define BUFFER_TYPE_TRANSFORM_FEEDBACK ( (u8)2 )
#define BUFFER_TYPE_UNIFORM			   ( (u8)3 )
#define BUFFER_TYPE_DRAW_INDIRECT	   ( (u8)4 )
#define BUFFER_TYPE_DISPATCH_INDIRECT  ( (u8)5 )
#define BUFFER_TYPE_SHADER_STORAGE	   ( (u8)6 )

#define VERTEX_DATATYPE_F32 ( (u8)0 )
#define VERTEX_DATATYPE_U32 ( (u8)1 )
#define VERTEX_DATATYPE_I32 ( (u8)2 )
#define VERTEX_DATATYPE_U16 ( (u8)3 )
#define VERTEX_DATATYPE_I16 ( (u8)4 )


using BufferUsageBit = u16;


struct BufferType {
	u8 value{DEFAULT8};

	u32 		glEnum() const;
	const char* to_str() const;


	BufferType(u8 v) : value{v} {}
};


struct VertexDataType {
	u8 value{DEFAULT8};

	u32 		glEnum() const;
	u32 		bytes()  const;
	const char* to_str() const;


	VertexDataType(u8 v) : value{v} {}
};


struct alignsz(8) bufMeta {
	VertexDataType vtype;
	BufferType     btype; 
	u32 		   count;


	bufMeta() : vtype{DEFAULT8}, btype{DEFAULT8}, count{DEFAULT32} {}
	bufMeta(VertexDataType VertexType, u32 NumOfElements, BufferType bufType) : vtype{VertexType}, btype{bufType}, count{NumOfElements} {}

	size_t bytes()  const { return static_cast<size_t>(vtype.bytes()) * count; }
	u32    glType() const { return vtype.glEnum(); }
};




struct Buffer
{
public:
	u32 	id;
	bufMeta meta;


	Buffer() : id(DEFAULT32), meta{} {}

	void create(
		BufferType 	   btype,
		BufferUsageBit usage,
		VertexDataType vdt, 
		u32 		   vcount, 
		void* 		   buffer = nullptr
	);
	void destroy();


	void print();
};


struct VertexBuffer : public Buffer {};
struct IndexBuffer : public Buffer {};




struct VertexArray 
{
public:
	struct VertexDescriptor {
		i32  		   len;
		VertexDataType vtype; 			     /* last bit is if data 'normalized' */
		u32  		   relativeOffset;
		u32 		   shaderAttributeIndex; /* Shader Attribute Index (location=X) */


		size_t bytes() const { return static_cast<size_t>(vtype.bytes()) * len; }
	};


	struct BufferDescriptor {
		u32     glID;
		u32     vaoBinding; /* binding index for the VAO.  */
		bufMeta buffer;
	};


	struct BufferLayoutDescription {
		BufferDescriptor 	    	  glMemoryDescriptor;
		std::vector<VertexDescriptor> bufferLayout;
	};


	u32 id;
	std::stack<BufferLayoutDescription> BufferDescriptions;

public:
	void create();
	void destroy();


	VertexArray& addBuffer(
		u32 		  			        	 VAObindingPoint, 
		Buffer const& 			        	 buf, 
		std::vector<VertexDescriptor> const& descriptors
	);

	void bind();
	void unbind();


	void print(); /* lets not implement this for now >:)))) */
};