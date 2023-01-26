#include "texture.hpp"
#include <immintrin.h>
#include <stb_image/stb_image.hpp>
#include <stdint.h>




void Texture2D::recreateImage(void* cpuBuffer)
{
    for(auto& paramName : desc.param) { glTextureParameteri(id, paramName.first, paramName.second); }
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        desc.fmt,
        dimensions[0],
        dimensions[1],
        0,
        desc.fmt,
        desc.dtype,
        cpuBuffer
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return;
}


void Texture2D::create(u32 dimX, u32 dimY, TextureDescriptor const& description, void* textureCPUData)
{
    glCreateTextures(GL_TEXTURE_2D, 1, &id);
    desc             = description;
    dimensions       = { dimX, dimY };
    boundTextureUnit = DEFAULT32;
    boundImageUnit = {
        DEFAULT32,
        DEFAULT16,
        DEFAULT16,
        DEFAULT32
    };

    recreateImage(textureCPUData);
    return;
}


void Texture2D::destroy()
{
    glDeleteTextures(1, &id);
}


void Texture2D::resize(u32 dimX, u32 dimY)
{
    dimensions = { dimX, dimY };
    recreateImage();
}


void Texture2D::fromFile(
    Texture2D&               toInitialize,
    const std::string_view & path, 
    TextureDescriptor const& glParams
) {
    i32 dimx, dimy, channels, desiredChannels = 4;
    u8*  textureBuffer   = nullptr;
    f32* texBufferFloats = nullptr;


    textureBuffer = stbi_load(path.begin(), &dimx, &dimy, &channels, desiredChannels);
    texBufferFloats = amalloc_t(f32, (size_t)dimx * dimy * desiredChannels * sizeof(f32), CACHE_LINE_BYTES);
    TextureConvertRGBAUI32ToRGBA32F( textureBuffer, texBufferFloats, dimx, dimy );
    stbi_image_free(textureBuffer);


    toInitialize.create(
        dimx, 
        dimy,  
        {
            glParams.param,
            TEXTURE_PIXEL_FORMAT_RGBA,
            TEXTURE_PIXEL_DATATYPE_F32
        },
        texBufferFloats
    );

    
    afree_t(texBufferFloats);
    return;
}


void TextureConvertRGBAUI32ToRGBA32F(u8* inData, float* outData, i32 dimx, i32 dimy)
{
    size_t currPixel = 0;
    size_t bufLength = 4 * static_cast<size_t>(dimx) * dimy;

#ifdef __AVX2__
    markstr("AVX2\n");
    ifcrashdo( ( (size_t)outData & (8 * sizeof(float) - 1) ) != 0, {
        printf("outData isn't aligned on (atleast) a 32 Byte memory boundary\n");
    });
	__m256  constf32;
	__m128i A; 
	__m256i B;
	__m256  C;
	__m256  D;
	constf32 = _mm256_broadcastss_ps(_mm_set_ss(0.00392156862745098f)); /* load constant 1/255 to each element */
    // for(i32 c = 0; c < 4; ++c) 
	// {
	// 	for(i32 y = 0; y < dimy; ++y) 
	// 	{
	// 		for(i32 x = 0; x < dimx; x += 8) 
	// 		{
	// 			A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* Load 16 bytes 				       */
	// 			B = _mm256_cvtepu8_epi32 (A); 	     	                   /* Convert first 8 bytes to u32's       */
	// 			C = _mm256_cvtepi32_ps(B);   	     	                   /* Convert u32's to f32's 		       */
	// 			D = _mm256_mul_ps(C, constf32);                            /* Multiply f32's by (1/255). 		   */
	// 		    _mm256_store_ps(&outData[currPixel], D);                   /* Store result in appropriate location */
	// 			currPixel += 8;
	// 		}
	// 	}  
	// }
    for(currPixel = 0; currPixel < bufLength; currPixel += 8) {
        A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* Load 16 bytes 				       */
        B = _mm256_cvtepu8_epi32 (A); 	     	                   /* Convert first 8 bytes to u32's       */
        C = _mm256_cvtepi32_ps(B);   	     	                   /* Convert u32's to f32's 		       */
        D = _mm256_mul_ps(C, constf32);                            /* Multiply f32's by (1/255). 		   */
        _mm256_store_ps(&outData[currPixel], D);                   /* Store result in appropriate location */
    }
#elif defined __SSE4_1__
    markstr("SSE4.1\n");
    ifcrashdo( ( (size_t)outData & (4 * sizeof(float) - 1) ) != 0, {
        printf("outData isn't aligned on (atleast) a 16 Byte memory boundary\n");
    });
    alignsz(16) std::array<f32, 4> const128 = { 0.00392156862745098f, 0.00392156862745098f, 0.00392156862745098f, 0.00392156862745098f };
    __m128 cf32;
    __m128i A, B;
	__m128  C, D;


    cf32 = _mm_load_ps(const128.begin()); /* SSE */
    // for(i32 c = 0; c < 4; ++c) 
	// {
	// 	for(i32 y = 0; y < dimy; ++y) {
    //         for(i32 x = 0; x < dimx; x += 4) 
	// 		{
    //             A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* SSE3 */
    //             B = _mm_cvtepu8_epi32(A); /* SSE4.1 */
    //             C = _mm_cvtepi32_ps(B);   /* SSE2   */
    //             D = _mm_mul_ps(C, cf32);  /* SSE    */
    //             currPixel += 4;
    //         }
    //     }
    // }
    for(currPixel = 0; currPixel < bufLength; currPixel += 4) {
        A = _mm_lddqu_si128((const __m128i*) &inData[currPixel] ); /* SSE3 */
        B = _mm_cvtepu8_epi32(A); /* SSE4.1 */
        C = _mm_cvtepi32_ps(B);   /* SSE2   */
        D = _mm_mul_ps(C, cf32);  /* SSE    */
        _mm_store_ps(&outData[currPixel], D);
    }
#else
    markstr("FALLBACK (COMPILER_OPTIMIZE march=native, mtune=native\n");
    alignsz(16) std::array<f32, 4> tmpf{};
    alignsz(16) std::array<u32, 4> tmpu{};
    u32   tmp = 0;


    // for(i32 c = 0; c < 4; ++c) 
	// {   
    //     for(i32 y = 0; y < dimy; ++y) {
    //         for(i32 x = 0; x < dimx; x += 4) {
    //             memcpy(&tmp, &inData[currPixel], 4);
    //             tmpu = {
    //                 tmp & 0x000000FF, /* First  Byte */
    //                 tmp & 0x0000FF00, /* Second Byte */
    //                 tmp & 0x00FF0000, /* Third  Byte */
    //                 tmp & 0xFF000000  /* Fourth Byte */
    //             };
    //             tmpf = {
    //                 static_cast<f32>(tmpu[0]),
    //                 static_cast<f32>(tmpu[1]),
    //                 static_cast<f32>(tmpu[2]),
    //                 static_cast<f32>(tmpu[3]),
    //             };
    //             memcpy(&outData[currPixel], tmpf.begin(), sizeof(f32) * 4);

    //             currPixel += 4;
    //         }
    //     }
    // }
    for(currPixel = 0; currPixel < bufLength; currPixel += 4) 
    {
        memcpy(&tmp, &inData[currPixel], 4);
        tmpu = {
            tmp & 0x000000FF, /* First  Byte */
            tmp & 0x0000FF00, /* Second Byte */
            tmp & 0x00FF0000, /* Third  Byte */
            tmp & 0xFF000000  /* Fourth Byte */
        };
        tmpf = {
            static_cast<f32>(tmpu[0]),
            static_cast<f32>(tmpu[1]),
            static_cast<f32>(tmpu[2]),
            static_cast<f32>(tmpu[3]),
        };
        memcpy(&outData[currPixel], tmpf.begin(), sizeof(f32) * 4);
    }
#endif

    return;
}