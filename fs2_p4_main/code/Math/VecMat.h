/*
 * Copyright (C) Volition, Inc. 1999.  All rights reserved.
 *
 * All source code herein is the property of Volition, Inc. You may not sell 
 * or otherwise commercially exploit the source or things you created based on the 
 * source.
 *
*/ 



#ifndef _VECMAT_H
#define _VECMAT_H

#ifndef UNITY_BUILD
#include <float.h>
#endif

//#define _INLINE_VECMAT

#define vm_is_vec_nan(v) (_isnan((v)->x) || _isnan((v)->y) || _isnan((v)->z))

//Macros/functions to fill in fields of structures

//macro to check if vector is zero
#define IS_VEC_NULL(v) (((v)->x == (float)0.0) && ((v)->y == (float)0.0) && ((v)->z == (float)0.0))

//macro to set a vector to zero.  we could do this with an in-line assembly
//macro, but it's probably better to let the compiler optimize it.
//Note: NO RETURN VALUE
#define vm_vec_zero(v) (v)->x=(v)->y=(v)->z=(float)0.0

/*
//macro set set a matrix to the identity. Note: NO RETURN VALUE
#define vm_set_identity(m) do {m->rvec.x = m->uvec.y = m->fvec.z = (float)1.0;	\
										m->rvec.y = m->rvec.z = \
										m->uvec.x = m->uvec.z = \
										m->fvec.x = m->fvec.y = (float)0.0;} while (0)
*/
extern void vm_set_identity(matrix *m);

#define vm_vec_make(v,_x,_y,_z) (((v)->x=(_x), (v)->y=(_y), (v)->z=(_z)), (v))

//Global constants

extern vector vmd_zero_vector;
extern vector vmd_x_vector;
extern vector vmd_y_vector;
extern vector vmd_z_vector;
extern matrix vmd_identity_matrix;

//Here's a handy constant

const vector ZERO_VECTOR(0.0f,0.0f,0.0f);
const matrix IDENTITY_MATRIX;

//fills in fields of an angle vector
#define vm_angvec_make(v,_p,_b,_h) (((v)->p=(_p), (v)->b=(_b), (v)->h=(_h)), (v))

//negate a vector
#define vm_vec_negate(v) do {(v)->x = - (v)->x; (v)->y = - (v)->y; (v)->z = - (v)->z;} while (0);

typedef struct plane {
	float	A, B, C, D;
} plane;

//Functions in library

//adds two vectors, fills in dest, returns ptr to dest
//ok for dest to equal either source, but should use vm_vec_add2() if so
#ifdef _INLINE_VECMAT
#define vm_vec_add( dst, src0, src1 ) do {	\
	(dst)->x = (src0)->x + (src1)->x;					\
	(dst)->y = (src0)->y + (src1)->y;					\
	(dst)->z = (src0)->z + (src1)->z;					\
} while(0) 
#else
void vm_vec_add(vector *dest,vector *src0,vector *src1);
#endif

//adds src onto dest vector, returns ptr to dest
#ifdef _INLINE_VECMAT
#define vm_vec_add2( dst, src ) do {	\
	(dst)->x += (src)->x;					\
	(dst)->y += (src)->y;					\
	(dst)->z += (src)->z;					\
} while(0) 
#else
void vm_vec_add2(vector *dest,vector *src);
#endif


//scales a vector and subs from to another
//dest -= k * src
#ifdef _INLINE_VECMAT
#define vm_vec_scale_sub2( dst, src, k ) do {	\
	float tmp_k = (k);								\
	(dst)->x -= (src)->x*tmp_k;					\
	(dst)->y -= (src)->y*tmp_k;					\
	(dst)->z -= (src)->z*tmp_k;					\
} while(0) 
#else
void vm_vec_scale_sub2(vector *dest,vector *src, float k);
#endif

//subs two vectors, fills in dest, returns ptr to dest
//ok for dest to equal either source, but should use vm_vec_sub2() if so
#ifdef _INLINE_VECMAT
#define vm_vec_sub( dst, src0, src1 ) do {	\
	(dst)->x = (src0)->x - (src1)->x;					\
	(dst)->y = (src0)->y - (src1)->y;					\
	(dst)->z = (src0)->z - (src1)->z;					\
} while(0) 
#else
void vm_vec_sub(vector *dest,vector *src0,vector *src1);
#endif


//subs one vector from another, returns ptr to dest
//dest can equal source
#ifdef _INLINE_VECMAT
#define vm_vec_sub2( dst, src ) do {	\
	(dst)->x -= (src)->x;					\
	(dst)->y -= (src)->y;					\
	(dst)->z -= (src)->z;					\
} while(0) 
#else
void vm_vec_sub2(vector *dest,vector *src);
#endif


//averages two vectors. returns ptr to dest
//dest can equal either source
vector *vm_vec_avg(vector *dest,vector *src0,vector *src1);

//averages four vectors. returns ptr to dest
//dest can equal any source
vector *vm_vec_avg4(vector *dest,vector *src0,vector *src1,vector *src2,vector *src3);

//scales a vector in place.  returns ptr to vector
#ifdef _INLINE_VECMAT
#define vm_vec_scale( dst, k ) do {	\
	float tmp_k = (k);								\
	(dst)->x *= tmp_k;					\
	(dst)->y *= tmp_k;					\
	(dst)->z *= tmp_k;					\
} while(0) 
#else
void vm_vec_scale(vector *dest,float s);
#endif

//scales and copies a vector.  returns ptr to dest
#ifdef _INLINE_VECMAT
#define vm_vec_copy_scale( dst, src, k ) do {	\
	float tmp_k = (k);								\
	(dst)->x = (src)->x * tmp_k;					\
	(dst)->y = (src)->y * tmp_k;					\
	(dst)->z = (src)->z * tmp_k;					\
} while(0) 
#else
void vm_vec_copy_scale(vector *dest,vector *src,float s);
#endif

//scales a vector, adds it to another, and stores in a 3rd vector
//dest = src1 + k * src2
#ifdef _INLINE_VECMAT
#define vm_vec_scale_add( dst, src1, src2, k ) do {	\
	float tmp_k = (k);								\
	(dst)->x = (src1)->x + (src2)->x * tmp_k;					\
	(dst)->y = (src1)->y + (src2)->y * tmp_k;					\
	(dst)->z = (src1)->z + (src2)->z * tmp_k;					\
} while(0) 
#else
void vm_vec_scale_add(vector *dest,vector *src1,vector *src2,float k);
#endif


//scales a vector and adds it to another
//dest += k * src
#ifdef _INLINE_VECMAT
#define vm_vec_scale_add2( dst, src, k ) do {	\
	float tmp_k = (k);								\
	(dst)->x += (src)->x * tmp_k;					\
	(dst)->y += (src)->y * tmp_k;					\
	(dst)->z += (src)->z * tmp_k;					\
} while(0) 
#else
void vm_vec_scale_add2(vector *dest,vector *src,float k);
#endif

//scales a vector in place, taking n/d for scale.  returns ptr to vector
//dest *= n/d
#ifdef _INLINE_VECMAT
#define vm_vec_scale2( dst, n, d ) do {	\
	float tmp_k = (n)/(d);								\
	(dst)->x *= tmp_k;					\
	(dst)->y *= tmp_k;					\
	(dst)->z *= tmp_k;					\
} while(0) 
#else
void vm_vec_scale2(vector *dest,float n,float d);
#endif

// finds the projection of source vector along a unit vector
// returns the magnitude of the component
float vm_vec_projection_parallel (vector *component, vector *src, vector *unit_vector);

// finds the projection of source vector onto a surface given by surface normal
void vm_vec_projection_onto_plane (vector *projection, vector *src, vector *normal);

//returns magnitude of a vector
float vm_vec_mag(vector *v);

// returns the square of the magnitude of a vector (useful if comparing distances)
float vm_vec_mag_squared(vector* v);

// returns the square of the distance between two points (fast and exact)
float vm_vec_dist_squared(vector *v0, vector *v1);

//computes the distance between two points. (does sub and mag)
float vm_vec_dist(vector *v0,vector *v1);

//computes an approximation of the magnitude of the vector
//uses dist = largest + next_largest*3/8 + smallest*3/16
float vm_vec_mag_quick(vector *v);

//computes an approximation of the distance between two points.
//uses dist = largest + next_largest*3/8 + smallest*3/16
float vm_vec_dist_quick(vector *v0,vector *v1);


//normalize a vector. returns mag of source vec
float vm_vec_copy_normalize(vector *dest,vector *src);
float vm_vec_normalize(vector *v);

//	This version of vector normalize checks for the null vector before normalization.
//	If it is detected, it generates a Warning() and returns the vector 1, 0, 0.
float vm_vec_normalize_safe(vector *v);

//normalize a vector. returns mag of source vec. uses approx mag
float vm_vec_copy_normalize_quick(vector *dest,vector *src);
float vm_vec_normalize_quick(vector *v);

//normalize a vector. returns mag of source vec. uses approx mag
float vm_vec_copy_normalize_quick_mag(vector *dest,vector *src);
float vm_vec_normalize_quick_mag(vector *v);

//return the normalized direction vector between two points
//dest = normalized(end - start).  Returns mag of direction vector
//NOTE: the order of the parameters matches the vector subtraction
float vm_vec_normalized_dir(vector *dest,vector *end,vector *start);
float vm_vec_normalized_dir_quick_mag(vector *dest,vector *end,vector *start);
// Returns mag of direction vector
float vm_vec_normalized_dir_quick(vector *dest,vector *end,vector *start);

////returns dot product of two vectors
#ifdef _INLINE_VECMAT
#define vm_vec_dotprod( v0, v1 ) (((v1)->x*(v0)->x)+((v1)->y*(v0)->y)+((v1)->z*(v0)->z))
#define vm_vec_dot( v0, v1 ) (((v1)->x*(v0)->x)+((v1)->y*(v0)->y)+((v1)->z*(v0)->z))
#else
float vm_vec_dotprod(vector *v0,vector *v1);
#define vm_vec_dot vm_vec_dotprod
#endif

#ifdef _INLINE_VECMAT
#define vm_vec_dot3( x1, y1, z1, v ) (((x1)*(v)->x)+((y1)*(v)->y)+((z1)*(v)->z))
#else
float vm_vec_dot3(float x,float y,float z,vector *v);
#endif

//computes cross product of two vectors. returns ptr to dest
//dest CANNOT equal either source
vector *vm_vec_crossprod(vector *dest,vector *src0,vector *src1);
#define vm_vec_cross vm_vec_crossprod

// test if 2 vectors are parallel or not.
int vm_test_parallel(vector *src0, vector *src1);

//computes surface normal from three points. result is normalized
//returns ptr to dest
//dest CANNOT equal either source
vector *vm_vec_normal(vector *dest,vector *p0,vector *p1,vector *p2);

//computes non-normalized surface normal from three points.
//returns ptr to dest
//dest CANNOT equal either source
vector *vm_vec_perp(vector *dest,vector *p0,vector *p1,vector *p2);

//computes the delta angle between two vectors.
//vectors need not be normalized. if they are, call vm_vec_delta_ang_norm()
//the forward vector (third parameter) can be NULL, in which case the absolute
//value of the angle in returned.  Otherwise the angle around that vector is
//returned.
float vm_vec_delta_ang(vector *v0,vector *v1,vector *fvec);

//computes the delta angle between two normalized vectors.
float vm_vec_delta_ang_norm(vector *v0,vector *v1,vector *fvec);

//computes a matrix from a set of three angles.  returns ptr to matrix
matrix *vm_angles_2_matrix(matrix *m,angles *a);

//	Computes a matrix from a single angle.
//	angle_index = 0,1,2 for p,b,h
matrix *vm_angle_2_matrix(matrix *m, float a, int angle_index);

//computes a matrix from a forward vector and an angle
matrix *vm_vec_ang_2_matrix(matrix *m,vector *v,float a);

//computes a matrix from one or more vectors. The forward vector is required,
//with the other two being optional.  If both up & right vectors are passed,
//the up vector is used.  If only the forward vector is passed, a bank of
//zero is assumed
//returns ptr to matrix
matrix *vm_vector_2_matrix(matrix *m,vector *fvec,vector *uvec,vector *rvec);

//this version of vector_2_matrix requires that the vectors be more-or-less
//normalized and close to perpendicular
matrix *vm_vector_2_matrix_norm(matrix *m,vector *fvec,vector *uvec,vector *rvec);

//rotates a vector through a matrix. returns ptr to dest vector
//dest CANNOT equal either source
vector *vm_vec_rotate(vector *dest,vector *src,matrix *m);

//rotates a vector through the transpose of the given matrix. 
//returns ptr to dest vector
//dest CANNOT equal source
// This is a faster replacement for this common code sequence:
//    vm_copy_transpose_matrix(&tempm,src_matrix);
//    vm_vec_rotate(dst_vec,src_vect,&tempm);
// Replace with:
//    vm_vec_unrotate(dst_vec,src_vect, src_matrix)
//
// THIS DOES NOT ACTUALLY TRANSPOSE THE SOURCE MATRIX!!! So if
// you need it transposed later on, you should use the 
// vm_vec_transpose() / vm_vec_rotate() technique.
vector *vm_vec_unrotate(vector *dest,vector *src,matrix *m);

//transpose a matrix in place. returns ptr to matrix
matrix *vm_transpose_matrix(matrix *m);
#define vm_transpose(m) vm_transpose_matrix(m)

//copy and transpose a matrix. returns ptr to matrix
//dest CANNOT equal source. use vm_transpose_matrix() if this is the case
matrix *vm_copy_transpose_matrix(matrix *dest,matrix *src);
#define vm_copy_transpose(dest,src) vm_copy_transpose_matrix((dest),(src))

//mulitply 2 matrices, fill in dest.  returns ptr to dest
//dest CANNOT equal either source
matrix *vm_matrix_x_matrix(matrix *dest,matrix *src0,matrix *src1);

//extract angles from a matrix
angles *vm_extract_angles_matrix(angles *a,matrix *m);

//extract heading and pitch from a vector, assuming bank==0
angles *vm_extract_angles_vector(angles *a,vector *v);

//make sure matrix is orthogonal
void vm_orthogonalize_matrix(matrix *m_src);

// like vm_orthogonalize_matrix(), except that zero vectors can exist within the
// matrix without causing problems.  Valid vectors will be created where needed.
void vm_fix_matrix(matrix *m);

//Rotates the orient matrix by the angles in tangles and then
//makes sure that the matrix is orthogonal.
void vm_rotate_matrix_by_angles( matrix *orient, angles *tangles );

//compute the distance from a point to a plane.  takes the normalized normal
//of the plane (ebx), a point on the plane (edi), and the point to check (esi).
//returns distance in eax
//distance is signed, so negative dist is on the back of the plane
float vm_dist_to_plane(vector *checkp,vector *norm,vector *planep);

// Given mouse movement in dx, dy, returns a 3x3 rotation matrix in RotMat.
// Taken from Graphics Gems III, page 51, "The Rolling Ball"
// Example:
//if ( (Mouse.dx!=0) || (Mouse.dy!=0) ) {
//   vm_trackball( Mouse.dx, Mouse.dy, &MouseRotMat );
//   vm_matrix_x_matrix(&tempm,&LargeView.ev_matrix,&MouseRotMat);
//   LargeView.ev_matrix = tempm;
//}
void vm_trackball( int idx, int idy, matrix * RotMat );

//	Find the point on the line between p0 and p1 that is nearest to int_pnt.
//	Stuff result in nearest_point.
//	Return value indicated where on the line *nearest_point lies.  Between 0.0f and 1.0f means it's
//	in the line segment.  Positive means beyond *p1, negative means before *p0.  2.0f means it's
//	beyond *p1 by 2x.
float find_nearest_point_on_line(vector *nearest_point, vector *p0, vector *p1, vector *int_pnt);

float vm_vec_dot_to_point(vector *dir, vector *p1, vector *p2);

void compute_point_on_plane(vector *q, plane *planep, vector *p);

// ----------------------------------------------------------------------------
// computes the point on a plane closest to a given point (which may be on the plane)
// 
//		inputs:		new_point		=>		point on the plane [result]
//						point				=>		point to compute closest plane point
//						plane_normal	=>		plane normal
//						plane_point		=>		plane point
void vm_project_point_onto_plane(vector *new_point, vector *point, vector *plane_normal, vector *plane_point);


//	Returns fairly random vector, "quick" normalized
void vm_vec_rand_vec_quick(vector *rvec);

// Given an point "in" rotate it by "angle" around an
// arbritary line defined by a point on the line "line_point" 
// and the normalized line direction, "line_dir"
// Returns the rotated point in "out".
void vm_rot_point_around_line(vector *out, vector *in, float angle, vector *line_point, vector *line_dir);

// Given two position vectors, return 0 if the same, else non-zero.
int vm_vec_cmp( vector * a, vector * b );

// Given two orientation matrices, return 0 if the same, else non-zero.
int vm_matrix_cmp( matrix * a, matrix * b );

// Moves angle 'h' towards 'desired_angle', taking the shortest
// route possible.   It will move a maximum of 'step_size' radians
// each call.   All angles in radians.
void vm_interp_angle( float *h, float desired_angle, float step_size );

// check a matrix for zero rows and columns
int vm_check_matrix_for_zeros(matrix *m);

// see if two vectors are identical
int vm_vec_same(vector *v1, vector *v2);

//	Interpolate from a start matrix toward a goal matrix, minimizing time between orientations.
// Moves at maximum rotational acceleration toward the goal when far and then max deceleration when close.
// Subject to constaints on rotational velocity and angular accleleration.
// Returns next_orientation valid at time delta_t.
void vm_matrix_interpolate(matrix *goal_orient, matrix *start_orient, vector *rotvel_in, float delta_t, 
		matrix *next_orient, vector *rotvel_out, vector *rotvel_limit, vector *acc_limit, int no_overshoot=0);

//	Interpolate from a start forward vec toward a goal forward vec, minimizing time between orientations.
// Moves at maximum rotational acceleration toward the goal when far and then max deceleration when close.
// Subject to constaints on rotational velocity and angular accleleration.
// Returns next forward vec valid at time delta_t.
void vm_forward_interpolate(vector *goal_fvec, matrix *orient, vector *rotvel_in, float delta_t, float delta_bank,
		matrix *next_orient, vector *rotvel_out, vector *vel_limit, vector *acc_limit, int no_overshoot=0);

// Find the bounding sphere for a set of points (center and radius are output parameters)
void vm_find_bounding_sphere(vector *pnts, int num_pnts, vector *center, float *radius);

// Version of atan2() that is safe for optimized builds
float atan2_safe(float x, float y);

// Translates from world coordinates to body coordinates
vector* vm_rotate_vec_to_body(vector *body_vec, vector *world_vec, matrix *orient);

// Translates from body coordinates to world coordiantes
vector* vm_rotate_vec_to_world(vector *world_vec, vector *body_vec, matrix *orient);

// estimate next orientation matrix as extrapolation of last and current
void vm_estimate_next_orientation(matrix *last_orient, matrix *current_orient, matrix *next_orient);

//	Return true if all elements of *vec are legal, that is, not a NAN.
int is_valid_vec(vector *vec);

//	Return true if all elements of *m are legal, that is, not a NAN.
int is_valid_matrix(matrix *m);

// Finds the rotation matrix corresponding to a rotation of theta about axis u
void vm_quaternion_rotate(matrix *m, float theta, vector *u);

// Takes a rotation matrix and returns the axis and angle needed to generate it
void vm_matrix_to_rot_axis_and_angle(matrix *m, float *theta, vector *rot_axis);

// interpolate between 2 vectors. t goes from 0.0 to 1.0. at
void vm_vec_interp_constant(vector *out, vector *v1, vector *v2, float t);

// randomly perturb a vector around a given (normalized vector) or optional orientation matrix
void vm_vec_random_cone(vector *out, vector *in, float max_angle, matrix *orient = NULL);

// given a start vector, an orientation and a radius, give a point on the plane of the circle
// if on_edge is 1, the point is on the very edge of the circle
void vm_vec_random_in_circle(vector *out, vector *in, matrix *orient, float radius, int on_edge);

// find the nearest point on the line to p. if dist is non-NULL, it is filled in
// returns 0 if the point is inside the line segment, -1 if "before" the line segment and 1 ir "after" the line segment
int vm_vec_dist_to_line(vector *p, vector *l0, vector *l1, vector *nearest, float *dist);

#endif

