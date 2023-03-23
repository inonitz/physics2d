#include "fluid2.hpp"
#include "flat_hash.hpp"




struct Cell {
	Vec4 u_p; /* ux uy uz p */
};


struct Marker {

};


struct Simulation2D
{
	f32 dx;
	f32 k_cfl;
	f32 uMax;
	f32 dt;

	flat_hash<u32, Marker> markerCells;
};


void advect(Simulation2D& )
{

}


void applyForces();
void applyViscosity();
void project();


void extrapolate();
