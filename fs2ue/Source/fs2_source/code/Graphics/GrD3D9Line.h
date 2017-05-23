#pragma once

struct vertex;

void gr_d3d9_line(int x1,int y1,int x2,int y2);
void gr_d3d9_circle( int xc, int yc, int d );
void gr_d3d9_aaline(vertex *v1, vertex *v2);
void gr_d3d9_gradient(int x1,int y1,int x2,int y2);
