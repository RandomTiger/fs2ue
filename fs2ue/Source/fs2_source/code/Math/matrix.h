#pragma once

#pragma warning( push )  
#pragma warning( disable : 4587 ) 

typedef class matrix {
public:
	matrix() 
	{
		a2d[0][0] = a2d[1][1] = a2d[2][2] = 1.0f;
		a2d[0][1] = a2d[0][2] = a2d[1][0] = a2d[1][2] = a2d[2][0] = a2d[2][1] = 0.0f;
	}
	union {
		struct {
			vector	rvec, uvec, fvec;
		};
		float a2d[3][3];
		float a1d[9];
	};

	const matrix &operator =(const matrix &rhs)
	{
		if(this == &rhs) 
		{
			// self assignment
			return *this;
		}

		memcpy(this, &rhs, sizeof(matrix));
		return *this;
	}
} matrix;

#pragma warning( pop )  
