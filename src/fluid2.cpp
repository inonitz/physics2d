#include "fluid2.hpp"
#include "flat_hash.hpp"




#define CELL_TYPE_INVALID ((u8)0b00) 
#define CELL_TYPE_AIR     ((u8)0b01) 
#define CELL_TYPE_FLUID   ((u8)0b10) 
#define CELL_TYPE_SOLID   ((u8)0b11)


struct Cell {
	Vec4 	     u_p; /* ux uy uz p */
	GridPosition x;  /* Calculate key from position P in grid. */
	/* 
		layer: 0->5 (6 bits) [0b00 = Invalid, 0b01 = subStep0 ]
		type : 6->7 (2 bits) [0b00 = Invalid, 0b01 = Air, 0b10 = Fluid, 0b11 = Solid]
	*/
	u8 			 bitFields;


	u8 layer() const { return bitFields & 0b00111111; }
	u8 type()  const { return bitFields & 0b11000000; }
};


struct Marker {
	GridPosition pos;
};


struct Simulation2D
{
	math::vec3u bounds;
	f32 dx;
	f32 k_cfl;
	f32 uMax;
	f32 dt;

	flat_hash<u32, Cell> Cells;
	std::vector<Marker>  markers;


	bool inBounds(GridPosition& pos) {
		return (pos.x < bounds.x) && (pos.y < bounds.y) && (pos.z < bounds.z);
	}


	u32 toIndexKey(GridPosition& pos) {
		return pos.z + bounds.z * (pos.y + bounds.y * pos.x);
	}

	
	void toGridPosition(u32 indexKey, GridPosition& set) {
		/* 
			Z index increases fastest, (Z is Width,  or Row)
			Y index after that, 	   (Y is Height, or Column)
			X index last 			   (X is Depth)
		*/
		u32 kRC, ColumnRowBounds = bounds.y * bounds.z;
		
		set.x = indexKey / ColumnRowBounds;
		kRC = set.x * ColumnRowBounds;

		set.y = (indexKey - kRC) / bounds.z;
		set.z = indexKey - kRC - (set.y * bounds.z);
		return;
	}


	void initialize(u32 boundX, u32 boundY, u32 boundZ, f32 h = 1e-3, f32 subSteps = 8);


	void update()
	{
		dt = k_cfl * dx / uMax;


		updateGrid();
	}




	void updateGrid()
	{
		/* default insert to hashtable should have layer = -1 unless specified otherwise. */
		bool shouldInsert = true;
		u8 status = 0;
		u64  k = 0;
		Cell c = {
			{ 0.0f, 0.0f, 0.0f, 0.0f },
			{ 0, 0, 0 },
			 0b000000 | (CELL_TYPE_FLUID << 6)
		};
		Cell toInsert;
		Cell* found = nullptr;
		Cell* tmp   = nullptr;

		
		
		/* [NOTE]:
			Instead of using ternary (or IF statement) make sure the 
			hash_table has a key for which a default value can be acquired from.
			That way we can just multiply pointers using conditionals
			for choosing memory accesses - and that way, we avoid if statements.


			Update Cells that are already marked by the markers.
		*/
		for(auto& p : markers)
		{
			k 	  = toIndexKey(p.pos);
			found = Cells.lookup(k);
			tmp   = __rcast(Cell*, /* Use condition to determine correct memory location to read from. */
				(found == nullptr) * __rcast(size_t, &c) +
				!(found == nullptr) * __rcast(size_t, &found)
			); 


			toInsert.x   = p.pos;
			toInsert.u_p = tmp->u_p;
			shouldInsert = shouldInsert && (  (found == nullptr) && inBounds(p.pos)  		        );
			shouldInsert = shouldInsert || (  (found != nullptr) && found->type() != CELL_TYPE_SOLID);
			if(shouldInsert) {
				status = Cells.insert(k, toInsert); /* Need to check for status incase of error. */
			}


			shouldInsert = true; /* reset boolean */
		}


		/* create buffer zone of air around fluid. */
		for(u32 i = 1; i < std::max( 2u, __scast( u32, std::ceilf(k_cfl) ) ); ++i)
		{

			for(auto& [value_ptr, index] : Cells.asMapIterator()) {
				
			}
		}
		return;
	}


};


void advect(Simulation2D& )
{

}


void applyForces();
void applyViscosity();
void project();


void extrapolate();
