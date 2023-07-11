#include "sprite_render.hpp"
#include <glad/glad.h>
#include <stb_image/stb_image.hpp>
#include "util/convert_tex.hpp"
#include "gl/renderer.hpp"




struct RectangleData
{
    Renderer::Transform transform;
    TextureBuffer       texture;
    Buffer              vertex;
    Buffer              indices;
    VertexArray         rectVAO;
};


globalContext* createDefaultContext(math::vec2u windowSize);
void           createRectangleData(RectangleData* rectData);



int sprite_render()
{
    auto* ctx = createDefaultContext({ 1280, 720 });
    bool running = true, paused = false;


    Renderer::RenderManager renderer;
    RectangleData           rectObject;
    const Renderer::RenderTarget target = {
        &rectObject.rectVAO,
        &rectObject.texture,
        &rectObject.transform
    };


    renderer.create();
    renderer.push(&target);
    while(running)
    {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ctx->glfw.procUpcomingEvents();


        if(!paused) {
            /* Game loop stuff here */
            renderer.renderImGui();
            renderer.render();
        }
        
        
        running = !ctx->glfw.shouldClose() && !isKeyPressed(KeyCode::ESCAPE);
        paused ^= isKeyPressed(KeyCode::P);
        paused = ctx->glfw.minimized() ? true : paused;
        ctx->glfw.procOngoingEvents();
    }
    ctx->glfw.close();
    return 0;
}




globalContext* createDefaultContext(math::vec2u windowSize)
{
    auto* ctx = getGlobalContext();
    stateChangeCallbacks funcs = {
        glfw_error_callback,
        glfw_framebuffer_size_callback,
        glfw_key_callback,
        glfw_cursor_position_callback,
        glfw_mouse_button_callback
    };


    ctx->glfw.create(windowSize.x, windowSize.y, funcs);
	debug( /* Enable Advanced OpenGL Debugging if enabled. */
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(gl_debug_message_callback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
		// glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	);


    ctx->shader.createFrom({
        { "C://Program Files//Programming Utillities//CProjects//physics2d//assets//shaders//default//shader.vert", GL_VERTEX_SHADER   },
        { "C://Program Files//Programming Utillities//CProjects//physics2d//assets//shaders//default//shader.frag", GL_FRAGMENT_SHADER }
    });
    ifcrash(ctx->shader.compile() == GL_FALSE);


    return ctx;
}


void createRectangleData(RectangleData* rectData)
{
    static std::vector<f32> vertices = {
        0.0f, 0.0f, 0.0f, 0.0f, 
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f
    };
    static std::vector<f32> indices = {
        0, 1, 3,
        3, 2, 0
    };


    // const char* texturePath = "C://Program Files//Programming Utillities//CProjects//physics2d//assets//sample_img.jpg";
    const char* texturePath = "C://CTools//Projects//physics2d//assets//sample_img.jpg";
    i32 TEXW, TEXH, TEXCHANNEL;
    u8*  textureBuffer;
    f32* textureBufferAsFloat;
    stbi_set_flip_vertically_on_load(true);
	textureBuffer 		 = stbi_load(texturePath, &TEXW, &TEXH, &TEXCHANNEL, 4);
	textureBufferAsFloat = amalloc_t(f32, (size_t)TEXW * TEXH * sizeof(f32) * 4, CACHE_LINE_BYTES);
	convertRGBA_U32_TO_F32(textureBuffer, textureBufferAsFloat, TEXW, TEXH);
	stbi_image_free(textureBuffer);

    // screenTexVBO.create(BufferDescriptor{ vboData.data(), 4u, 
	// 		{{
	// 			{ GL_FLOAT, 3 },
	// 			{ GL_FLOAT, 2 }
	// 		}}
	// 	},
	// 	GL_STREAM_DRAW
	// );
	// screenTexIBO.create({ iboData.data(), __scast(u32, iboData.size()), 
	// 		{{
	// 			{ GL_UNSIGNED_INT, 1 },
	// 		}}
	// 	},
	// 	GL_STREAM_DRAW
	// );
    // screenTexBuffer.create({
	// 	{ w, h },
	// 	nullptr,
	// 	{4, GL_RGBA, GL_FLOAT, GL_RGBA32F },
	// 	{
	// 		{ GL_TEXTURE_WRAP_S, 	 GL_CLAMP_TO_EDGE },
	// 		{ GL_TEXTURE_WRAP_T, 	 GL_CLAMP_TO_EDGE },
	// 		{ GL_TEXTURE_MIN_FILTER, GL_LINEAR 		  },
	// 		{ GL_TEXTURE_MAG_FILTER, GL_LINEAR 		  },
	// 	}
	// });

    // rectData->vertex.create();
    // rectData->indices.create();
    rectData->rectVAO.create(rectData->vertex, rectData->indices);
    // rectData->texture.create({
	// 	{ TEXW, TEXH },
	// 	nullptr,
	// 	{4, GL_RGBA, GL_FLOAT, GL_RGBA32F },
	// 	{
	// 		{ GL_TEXTURE_WRAP_S, 	 GL_CLAMP_TO_EDGE },
	// 		{ GL_TEXTURE_WRAP_T, 	 GL_CLAMP_TO_EDGE },
	// 		{ GL_TEXTURE_MIN_FILTER, GL_LINEAR 		  },
	// 		{ GL_TEXTURE_MAG_FILTER, GL_LINEAR 		  },
	// 	}
	// });
}



// using PhysicsHandle = imut_type_handle<struct PhysicsTarget>;
// struct Entity
// {
//     Renderer::RenderHandle rdesc;
//     PhysicsHandle pdesc;
// };




/*
MultiDrawIndirect - batching Multiple draw calls (per object)
Texture Arrays - batching/packing multiple 2D Textures into the same object


const float c_infinity = 1. / 0.;
const float c_pi         = 3.14159265358979323846264338327950288;


float toRadians(float degrees) { return degrees * c_pi / 180.0f; }
float random(in vec2 p)
{
    const vec2 r = vec2(23.1406926327792690, 2.6651441426902251);
    return fract( cos( mod( 123456789., 1e-7 + 256. * dot(p,r) ) ) );  
}

float uintToNormFloat(uint m) { return uintBitsToFloat((m & 0x007FFFFFu) | 0x3F800000u ) - 1.0f; }
float random_norm(in vec2 p)  { return uintToNormFloat(floatBitsToUint(random(p))); 			 }
vec3 simpleRandUnitSphere(in vec2 texelCoord, uint sampleNum)
{
    vec3 r3 = vec3(
        random(  vec2(u_rand, random(texelCoord)       )),
        random(  vec2(u_rand, random(vec2(sampleNum))  )), 
        0
    );
    r3.z = random(r3.xy);
    return normalize(r3);
}


C://Program Files//Programming Utillities//CProjects//physics2d//assets//shaders//default//shader.




Equation 1
\[ V^{AB} = V^B - V^A \] Note that in order to create a vector from position A to position B, you must do: endpoint - startpoint. \(V^{AB}\) is the relative velocity from A to B. This equation ought to be expressed in terms of the collision normal \(n\) - that is, we would like to know the relative velocity from A to B along the collision normal's direction:
Equation 2
\[ V^{AB} \cdot n = (V^B - V^A) \cdot n \]
Equation 3
\[ V_1 = \begin{bmatrix}x_1 \\y_1\end{bmatrix}, V_2 = \begin{bmatrix}x_2 \\y_2\end{bmatrix} \\ V_1 \cdot V_2 = x_1 * x_2 + y_2 * y_2 \]
Newton's Law of Restitution states the following (given 2 Objects A & B):
Equation 4
\[V' = min( A.restitution, B.restitution ) * V \]
Equation 5
\[ V^{AB} \cdot n = -e * (V^B - V^A) \cdot n \]
Equation 6
\[ V' = V + j * n \]
Equation 7
\[ Impulse = mass * Velocity \\ Velocity = \frac{Impulse}{mass} \therefore V' = V + \frac{j * n}{mass}\]
Equation 8
\[ V'^A = V^A + \frac{j * n}{mass^A} \\ V'^B = V^B - \frac{j * n}{mass^B} \]

All that is now required is to merge Equations 8 and 5. Our resulting equation will look something like this:
Equation 9
\[ (V^A - V^V + \frac{j * n}{mass^A} + \frac{j * n}{mass^B}) * n = -e * (V^B - V^A) \cdot n \\ \therefore \\ (V^A - V^V + \frac{j * n}{mass^A} + \frac{j * n}{mass^B}) * n + e * (V^B - V^A) \cdot n = 0 \]
Equation 10
\[ (V^B - V^A) \cdot n + j * (\frac{j * n}{mass^A} + \frac{j * n}{mass^B}) * n + e * (V^B - V^A) \cdot n = 0 \\ \therefore \\ (1 + e)((V^B - V^A) \cdot n) + j * (\frac{j * n}{mass^A} + \frac{j * n}{mass^B}) * n = 0 \\ \therefore \\ j = \frac{-(1 + e)((V^B - V^A) \cdot n)}{\frac{1}{mass^A} + \frac{1}{mass^B}} \]
Equation 11
\[ ax + by + c = 0 \\ normal = \begin{bmatrix}a \\b\end{bmatrix} \]
custom-physics-line2d



*/