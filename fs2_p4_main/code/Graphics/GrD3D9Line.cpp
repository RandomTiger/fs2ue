#ifndef UNITY_BUILD
#include <D3d9.h>

#include "2d.h"
#include "tmapper.h"
#include "line.h"

#include "Graphics/GrD3D9Line.h"

#include "Graphics/GrD3D9.h"
#include "Graphics/GrD3D9Render.h"
#endif

void d3d9_make_rect( Vertex *a, Vertex *b, int x1, int y1, int x2, int y2 )
{
	a->sz = 0.99f;
	b->sz = 0.99f;

	a->sx = i2fl(x1 + gr_screen.offset_x);
	a->sy = i2fl(y1 + gr_screen.offset_y);

	b->sx = i2fl(x2 + gr_screen.offset_x);
	b->sy = i2fl(y2 + gr_screen.offset_y);

	if ( x1 == x2 )	{
		// Verticle line
		if ( a->sy < b->sy )	{
			b->sy += 0.5f;
		} else {
			a->sy += 0.5f;
		}
	} else if ( y1 == y2 )	{
		// Horizontal line
		if ( a->sx < b->sx )	{
			b->sx += 0.5f;
		} else {
			a->sx += 0.5f;
		}
	}

}

void gr_d3d9_line(int x1,int y1,int x2,int y2)
{
	int clipped = 0, swapped=0;

	// Set up Render State - flat shading - alpha blending
	gr_d3d9_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );
	DWORD color = D3DCOLOR_RGBA(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha );

	// This is an ugly define, get rid of it
	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

	Vertex d3d_verts[2];
	d3d9_make_rect(&d3d_verts[0],&d3d_verts[1],x1,y1,x2,y2);

	d3d_verts[0].color = color;
	d3d_verts[1].color = color;

	DrawPrimitive2D(D3DPT_LINELIST, 1, d3d_verts, TMAP_FLAG_NO_TEXTURE);
}

// This is generic and should be moved
void gr_d3d9_circle( int xc, int yc, int d )
{
	int r = d/2;
	int p=3-d;
	int x=0;
	int y=r;

	// Big clip
	if ( (xc+r) < gr_screen.clip_left ) return;
	if ( (xc-r) > gr_screen.clip_right ) return;
	if ( (yc+r) < gr_screen.clip_top ) return;
	if ( (yc-r) > gr_screen.clip_bottom ) return;

	while(x<y)	{
		// Draw the first octant
		gr_line( xc-y, yc-x, xc+y, yc-x );
		gr_line( xc-y, yc+x, xc+y, yc+x );

		if (p<0) 
			p=p+(x<<2)+6;
		else	{
			// Draw the second octant
			gr_line( xc-x, yc-y, xc+x, yc-y );
			gr_line( xc-x, yc+y, xc+x, yc+y );
			p=p+((x-y)<<2)+10;
			y--;
		}
		x++;
	}
	if(x==y)	{
		gr_line( xc-x, yc-y, xc+x, yc-y );
		gr_line( xc-x, yc+y, xc+x, yc+y );
	}
}

void gr_d3d9_gradient(int x1,int y1,int x2,int y2)
{
	int clipped = 0, swapped=0;

	if ( !gr_screen.current_color.is_alphacolor )	{
		gr_line( x1, y1, x2, y2 );
		return;
	}

	INT_CLIPLINE(x1,y1,x2,y2,gr_screen.clip_left,gr_screen.clip_top,gr_screen.clip_right,gr_screen.clip_bottom,return,clipped=1,swapped=1);

	uint color1, color2;

	// Set up Render State - flat shading - alpha blending
	gr_d3d9_set_state( TEXTURE_SOURCE_NONE, ALPHA_BLEND_ALPHA_BLEND_ALPHA, ZBUFFER_TYPE_NONE );

//	if (lpDevDesc->dpcLineCaps.dwShadeCaps & D3DPSHADECAPS_ALPHAGOURAUDBLEND )	{
		color1 = D3DCOLOR_RGBA(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, gr_screen.current_color.alpha );
		color2 = D3DCOLOR_RGBA(gr_screen.current_color.red, gr_screen.current_color.green, gr_screen.current_color.blue, 0 );
/*	} else if (lpDevDesc->dpcLineCaps.dwShadeCaps & D3DPSHADECAPS_COLORGOURAUDRGB )	{
		color1 = D3DCOLOR_RGBA(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,gr_screen.current_color.alpha);
		color2 = D3DCOLOR_RGBA(0,0,0,gr_screen.current_color.alpha);
	} else {
		color1 = D3DCOLOR_RGBA(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,gr_screen.current_color.alpha);
		color2 = D3DCOLOR_RGBA(gr_screen.current_color.red,gr_screen.current_color.green,gr_screen.current_color.blue,gr_screen.current_color.alpha);
	}*/

	Vertex d3d_verts[2];
	Vertex *a = d3d_verts;
	Vertex *b = d3d_verts+1;

	d3d9_make_rect( a, b, x1, y1, x2, y2 );

	if ( swapped )	{
		b->color = color1;
		a->color = color2;
	} else {
		a->color = color1;
		b->color = color2;
	}
	
	DrawPrimitive2D(D3DPT_LINELIST, 1, d3d_verts, TMAP_FLAG_NO_TEXTURE);
}

void gr_d3d9_aaline(vertex *v1, vertex *v2)
{
	gr_line( fl2i(v1->sx), fl2i(v1->sy), fl2i(v2->sx), fl2i(v2->sy) );
}

void gr_d3d9_pixel(int x, int y)
{
	gr_line(x,y,x,y);
}