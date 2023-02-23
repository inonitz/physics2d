#pragma once
#include "vec.hpp"




struct Fluid2D
{    
    /* 
        for the simulation to be numerically stable, dt must satisfy the following condition:
        dt <= 5dx / u_max, where dx = length of the grids' unit square, u_max = maximum length of the vectors tracked in the velocity field.  
    */

    typedef u32 gridSideLength;
    typedef f32 generic_unit_value;

    typedef generic_unit_value diffusivity;
    typedef generic_unit_value density;
    typedef generic_unit_value unit_length;

    using FieldComponent = f32*;
    using ScalarField    = FieldComponent;
    struct VectorField 
    { 
        FieldComponent x; 
        FieldComponent y; 
    };


    diffusivity    d;
    density        p0;
    unit_length    dx;
    unit_length    recpdx;
    gridSideLength N;

    /* 
        Because we're using MAC grids for the velocities, the length of each axis is N(N+1). 
        Moreover, the array access would be ( i [0 => N), j [0 => N+1) ).
        Given the following grid (4x4):
          ---   ---   ---   ---
        | c00 | c01 | c02 | c03 |
          ---   ---   ---   ---  
        | c04 | c05 | c06 | c07 |
          ---   ---   ---   ---  
        | c08 | c09 | c10 | c11 | 
          ---   ---   ---   ---  
        | c12 | c13 | c14 | c15 |
          ---   ---   ---   ---
        
        All Cell Boundaries of c(i, j) marked with
        '|'   => boundaries in X direction, more precisely u->x[...].
            Moreover, Order of counting is: c00.left, c00.right, c01.left, c01.right, ...
        '---' => boundaries in Y direction, more precisely u->y[...].
            Moreover, Order of counting is: c00.up, c00.down, c01.up, c01.down, 

    */
    VectorField v;
    VectorField v0;
    ScalarField p;
    VectorField bf;



    __force_inline math::vec2f velocity(VectorField* u, u32 i, u32 j) const;
    __force_inline math::vec2f position(u32 i, u32 j) const; /* Bottom Left corner is (0, 0), axes go forward from there in both directions. */
    __force_inline math::vec2u indices(math::vec2f const& pos);
    __force_inline math::vec2f divergence(VectorField* u, u32 i, u32 j) const; 


    void advect(VectorField* u, f32 dt, VectorField* q);
    void project(f32 dt, VectorField* u);
    void applyBodyForces();
    
    void applyBoundaries();

    void stepFrame(f32 dt);
};