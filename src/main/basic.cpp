#include "basic.hpp"




int basic()
{
	std::array<float, 4> clear  = { 0.45f, 0.55f, 0.60f, 1.00f };
	std::array<u32, 2>   dims   = { 1920, 1080  };
	mark();
	Context 			 programContext; 
	
	mark();
	programContext.create(dims[0], dims[1], clear);
	mark();
	programContext.gl.bind();
	// game loop
	while(!programContext.glfw.shouldClose())
	{
		programContext.processUpcomingEvents();



		programContext.processOngoingEvents();
	}
    // game loop end


	programContext.destroy();
    return 0;
}


/* 

  
gpu_buffer x0, v0, p0, xi, vi, pi


uploadVelocityBuffer(v0);
uploadPositionBuffer(x0);
uploadPressureBuffer(p0);
game loop:
	processIncomingEvents();


	while(executeComputeShader()) {
		waitUntilFinished();
	}
	[Inside Compute Shader] {
		xi, vi, pi = UpdateVectorFields(x0, v0, p0);
	}


	while(executeVertexShader()) {
		waitUntilFinished();
	}
	[Inside Vertex Shader] {
		xi = ApplyMatrixTransforms(xi);
		xi = Projection(xi, fov, aspect_ratio(x, y) );
	}


	while(executeFragmentShader()) {
		waitUntilFinished();
	}
	[Inside Fragment Shader] {
		extern gpu_buffer xi, vi, pi;
		ReadVectorFields();

		for every pixel: # not precise whatsoever but its a good enough representation for now.
			fillPixel(xi, color={ pi, vi, 0.0f, 1.0f});
	}

	processUpcomingEvents();


*/

