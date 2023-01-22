#pragma once
#include "manvec.hpp"
#include "glad/glad.h"
#include <string_view>




#define TEXTURE_PARAMETER_DEPTH_STENCIL_MODE ( (u32)GL_DEPTH_STENCIL_TEXTURE_MODE )
#define TEXTURE_PARAMETER_BASE_LVL           ( (u32)GL_TEXTURE_BASE_LEVEL   	  )
#define TEXTURE_PARAMETER_COMPARE_FUNC  	 ( (u32)GL_TEXTURE_COMPARE_FUNC 	  )
#define TEXTURE_PARAMETER_COMPARE_MODE  	 ( (u32)GL_TEXTURE_COMPARE_MODE 	  )
#define TEXTURE_PARAMETER_LOD_BIAS  		 ( (u32)GL_TEXTURE_LOD_BIAS     	  )
#define TEXTURE_PARAMETER_FILTER_MIN  		 ( (u32)GL_TEXTURE_MIN_FILTER   	  )
#define TEXTURE_PARAMETER_FILTER_MAG  		 ( (u32)GL_TEXTURE_MAG_FILTER   	  )
#define TEXTURE_PARAMETER_MIN_LOD  			 ( (u32)GL_TEXTURE_MIN_LOD 	    	  )
#define TEXTURE_PARAMETER_MAX_LOD  			 ( (u32)GL_TEXTURE_MAX_LOD 	    	  )
#define TEXTURE_PARAMETER_MAX_LEVEL  		 ( (u32)GL_TEXTURE_MAX_LEVEL 	 	  )
#define TEXTURE_PARAMETER_SWIZ_R  			 ( (u32)GL_TEXTURE_SWIZZLE_R 	 	  )
#define TEXTURE_PARAMETER_SWIZ_G  			 ( (u32)GL_TEXTURE_SWIZZLE_G 	 	  )
#define TEXTURE_PARAMETER_SWIZ_B  			 ( (u32)GL_TEXTURE_SWIZZLE_B 	 	  )
#define TEXTURE_PARAMETER_SWIZ_A  			 ( (u32)GL_TEXTURE_SWIZZLE_A 	 	  )
#define TEXTURE_PARAMETER_WRAP_X  			 ( (u32)GL_TEXTURE_WRAP_S 		 	  )
#define TEXTURE_PARAMETER_WRAP_Y  			 ( (u32)GL_TEXTURE_WRAP_T 		 	  )
#define TEXTURE_PARAMETER_WRAP_Z  			 ( (u32)GL_TEXTURE_WRAP_R 		 	  )


#define TEXTURE_PIXEL_FORMAT_RED             ( (u32)GL_RED 			   )
#define TEXTURE_PIXEL_FORMAT_RG              ( (u32)GL_RG  			   )
#define TEXTURE_PIXEL_FORMAT_RGB             ( (u32)GL_RGB 			   )
#define TEXTURE_PIXEL_FORMAT_BGR             ( (u32)GL_BGR 			   )
#define TEXTURE_PIXEL_FORMAT_RGBA            ( (u32)GL_RGBA 		   )
#define TEXTURE_PIXEL_FORMAT_BGRA            ( (u32)GL_BGRA 		   )
#define TEXTURE_PIXEL_FORMAT_INT_RED         ( (u32)GL_RED_INTEGER 	   )
#define TEXTURE_PIXEL_FORMAT_INT_RG          ( (u32)GL_RG_INTEGER  	   )
#define TEXTURE_PIXEL_FORMAT_INT_RGB         ( (u32)GL_RGB_INTEGER 	   )
#define TEXTURE_PIXEL_FORMAT_INT_BGR         ( (u32)GL_BGR_INTEGER 	   )
#define TEXTURE_PIXEL_FORMAT_INT_RGBA        ( (u32)GL_RGBA_INTEGER    )
#define TEXTURE_PIXEL_FORMAT_INT_BGRA        ( (u32)GL_BGRA_INTEGER    )
#define TEXTURE_PIXEL_FORMAT_STENCIL_INDEX   ( (u32)GL_STENCIL_INDEX   )
#define TEXTURE_PIXEL_FORMAT_DEPTH_COMPONENT ( (u32)GL_DEPTH_COMPONENT )
#define TEXTURE_PIXEL_FORMAT_DEPTH_STENCIL   ( (u32)GL_DEPTH_STENCIL   )


#define TEXTURE_PIXEL_DATATYPE_U8             ( (u32)GL_UNSIGNED_BYTE				 )
#define TEXTURE_PIXEL_DATATYPE_I8             ( (u32)GL_BYTE						 )
#define TEXTURE_PIXEL_DATATYPE_U16            ( (u32)GL_UNSIGNED_SHORT				 )
#define TEXTURE_PIXEL_DATATYPE_I16            ( (u32)GL_SHORT						 )
#define TEXTURE_PIXEL_DATATYPE_U32            ( (u32)GL_UNSIGNED_INT				 )
#define TEXTURE_PIXEL_DATATYPE_I32            ( (u32)GL_INT						     )
#define TEXTURE_PIXEL_DATATYPE_F32            ( (u32)GL_FLOAT						 )
#define TEXTURE_PIXEL_DATATYPE_U8_332         ( (u32)GL_UNSIGNED_BYTE_3_3_2		     )
#define TEXTURE_PIXEL_DATATYPE_U8_233REV      ( (u32)GL_UNSIGNED_BYTE_2_3_3_REV	     )
#define TEXTURE_PIXEL_DATATYPE_U16_565        ( (u32)GL_UNSIGNED_SHORT_5_6_5		 )
#define TEXTURE_PIXEL_DATATYPE_U16_565REV     ( (u32)GL_UNSIGNED_SHORT_5_6_5_REV	 )
#define TEXTURE_PIXEL_DATATYPE_U16_4444       ( (u32)GL_UNSIGNED_SHORT_4_4_4_4		 )
#define TEXTURE_PIXEL_DATATYPE_U16_4444REV    ( (u32)GL_UNSIGNED_SHORT_4_4_4_4_REV	 )
#define TEXTURE_PIXEL_DATATYPE_U16_5551       ( (u32)GL_UNSIGNED_SHORT_5_5_5_1		 )
#define TEXTURE_PIXEL_DATATYPE_U16_1555REV    ( (u32)GL_UNSIGNED_SHORT_1_5_5_5_REV	 )
#define TEXTURE_PIXEL_DATATYPE_U32_8888       ( (u32)GL_UNSIGNED_INT_8_8_8_8		 )
#define TEXTURE_PIXEL_DATATYPE_U32_8888REV    ( (u32)GL_UNSIGNED_INT_8_8_8_8_REV	 )
#define TEXTURE_PIXEL_DATATYPE_U32_1010102    ( (u32)GL_UNSIGNED_INT_10_10_10_2	     )
#define TEXTURE_PIXEL_DATATYPE_U32_2101010REV ( (u32)GL_UNSIGNED_INT_2_10_10_10_REV  )


using TextureParameter        = u32;
using TextureInternalFormat   = u32;
using TextureInternalDatatype = u32;


struct TextureDescriptor {
    manvec<std::pair<TextureParameter, u32>> param;
    TextureInternalFormat    fmt;
    TextureInternalDatatype  dtype;

    
    TextureDescriptor() : param(0), fmt{DEFAULT32}, dtype{DEFAULT32} {}


    TextureDescriptor(
        manvec<std::pair<TextureParameter, u32>> const& glParameters, 
        TextureInternalFormat   dataFormat,
        TextureInternalDatatype dataType
    ) : fmt(dataFormat), dtype(dataType) 
    {
        param.copy(glParameters);
        return;
    }


    TextureDescriptor& operator=(TextureDescriptor const& cpy)
    {
        param.copy(cpy.param);
        fmt   = cpy.fmt;
        dtype = cpy.dtype;
        return *this;
    }


    static u32 InternalFormatToNumOfComponents(TextureInternalFormat fmt)
    {
        constexpr std::array<u32, 2> convert[15] = 
        {
            { TEXTURE_PIXEL_FORMAT_RED            , 1u },
            { TEXTURE_PIXEL_FORMAT_RG             , 2u },
            { TEXTURE_PIXEL_FORMAT_RGB            , 3u },
            { TEXTURE_PIXEL_FORMAT_BGR            , 3u },
            { TEXTURE_PIXEL_FORMAT_RGBA           , 4u },
            { TEXTURE_PIXEL_FORMAT_BGRA           , 4u },
            { TEXTURE_PIXEL_FORMAT_INT_RED        , 1u },
            { TEXTURE_PIXEL_FORMAT_INT_RG         , 2u },
            { TEXTURE_PIXEL_FORMAT_INT_RGB        , 3u },
            { TEXTURE_PIXEL_FORMAT_INT_BGR        , 3u },
            { TEXTURE_PIXEL_FORMAT_INT_RGBA       , 4u },
            { TEXTURE_PIXEL_FORMAT_INT_BGRA       , 4u },
            { TEXTURE_PIXEL_FORMAT_STENCIL_INDEX  , 1u },
            { TEXTURE_PIXEL_FORMAT_DEPTH_COMPONENT, 1u },
            { TEXTURE_PIXEL_FORMAT_DEPTH_STENCIL  , 2u },
        };

        size_t i = 0;
        while(convert[i][0] != fmt && i < 15) {
            ++i;
        };
        ifcrashmsg(i == 15, "TextureDescriptor::InternalFormatToNumOfComponents() => fmt paramter given is unkown/invalid\n", NULL);


        return convert[i][1];
    }
};


struct Texture2D 
{
private:
    void recreateImage(void* cpuBuffer = nullptr);

public:
    u32                id;
    u32                boundTextureUnit;
    TextureDescriptor  desc;
    std::array<u32, 2> dimensions;


    void create(
        u32   dimX, 
        u32   dimY, 
        TextureDescriptor const& desc, 
        void* textureCPUData = nullptr
    );
    void destroy();


    void resize(u32 dimX, u32 dimY);


    __force_inline void bindToTextureUnit(u32 textureUnit) { 
        boundTextureUnit = textureUnit;
        glBindTextureUnit(boundTextureUnit, id); 
    }
    
    __force_inline void unbindFromTextureUnit() {
        glBindTextureUnit(boundTextureUnit, 0); 
        boundTextureUnit = 0;
    }


    static void fromFile(
        Texture2D&               toInitialize,
        const std::string_view & path, 
        TextureDescriptor const& glParams
    );


    static void setActiveTexture(u32 textureID) { /* glActiveTexture(GL_TEXTURE0 + ( textureID & 0x1F )); */ glActiveTexture(textureID); }
};




void TextureConvertRGBAUI32ToRGBA32F(u8* inData, float* outData, i32 dimx, i32 dimy);