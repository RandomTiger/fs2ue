// This file is mainly for windows related functionality:
// Startup, device init, minimize and restore

#ifndef UNITY_BUILD
#include "osapi.h"
#include "2d.h"
#include "GrInternal.h"
#include "bmpman.h"

#include <D3d9.h>
#include <assert.h>
#include <d3d9types.h>
#include "Graphics/Font.h"
#include <windows.h>

#include "Cmdline/cmdline.h"
#include "Graphics/GrD3D9Render.h"
#include "Graphics/GrD3D9Texture.h"
#include "Graphics/GrD3D9Shader.h"
#include "Graphics/GrD3D9Line.h"

#include "Io/MouseController.h"
#include "DebugSys/DebugSys.h"
#include "TomLib/src/Debug/Profiler.h"

#include "AntTweakBar.h"
#endif

void SetupFunctionPointers();
void SetupWindow();
bool SetupDevice();
void Reset();

IDirect3D9       *pDirect3D;
IDirect3DDevice9 *pD3DDevice = 0;

D3DPRESENT_PARAMETERS presentationParameters;

volatile bool D3D9_running = false;
volatile int D3D9_activate = 0;
volatile int D3D9_deactivate = 0;
bool D3D9_window = false;

RECT D3D9_cursor_clip_rect;


void gr_d3d9_start_frame()
{
	HRESULT hr = S_OK;

	hr = pD3DDevice->BeginScene();
	assert(SUCCEEDED(hr));
}
void gr_d3d9_stop_frame()
{
	HRESULT hr = S_OK;
	hr = pD3DDevice->EndScene();
	assert(SUCCEEDED(hr));

	hr = pD3DDevice->Present( NULL, NULL, NULL, NULL );
	if(hr != D3DERR_DEVICELOST)
	{
		assert(SUCCEEDED(hr));
	}	
}

void gr_d3d9_activate(int active)
{
	if ( !D3D9_running )	{
		return;
	}
	mprintf(( "Direct3D activate: %d\n", active ));

	HWND hwnd = (HWND)os_get_window();
	
	if ( active  )	{
		D3D9_activate++;

		Reset();

		if ( hwnd )	{
			ClipCursor(&D3D9_cursor_clip_rect);
			ShowWindow(hwnd,SW_RESTORE);
		}

	} else {
		D3D9_deactivate++;

		if ( hwnd )	{
			ClipCursor(NULL);
			ShowWindow(hwnd,SW_MINIMIZE);
		}
	}
}

bool gr_d3d9_init()
{
	mprintf(( "Initializing D3D9 graphics device...\n" ));

	SetupFunctionPointers();
	SetupWindow();
	if(SetupDevice() == false)
	{
		return false;
	}

	SetupTextureSystem();
	SetupRenderSystem();
	SetupShaderSystem();

	D3D9_running = true;
	return true;
}

void SetupWindow()
{
	HWND hwnd = (HWND) os_get_window();
	D3D9_window = Cmdline_window != 0;

	if(D3D9_window){
		SetWindowPos(hwnd, HWND_TOP, 0, 0, gr_screen.max_w, gr_screen.max_h, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE | SWP_DRAWFRAME);
		SetForegroundWindow(hwnd);
		SetActiveWindow(hwnd);	
		
		D3D9_cursor_clip_rect.left   = 0;
		D3D9_cursor_clip_rect.top    = 0;
		D3D9_cursor_clip_rect.right  = gr_screen.max_w-1;
		D3D9_cursor_clip_rect.bottom = gr_screen.max_h-1;
	} 
	else 
	{
		SetWindowLong( hwnd, GWL_EXSTYLE, 0 );
		SetWindowLong( hwnd, GWL_STYLE, WS_POPUP );
		ShowWindow(hwnd, SW_SHOWNORMAL );

		SetActiveWindow(hwnd);
		SetForegroundWindow(hwnd);

		// Prepare the window to go full screen
	#ifndef NDEBUG
		mprintf(( "Window in debugging mode... mouse clicking may cause problems!\n" ));

		RECT work_rect;
		SystemParametersInfo( SPI_GETWORKAREA, 0, &work_rect, 0 );
		SetWindowPos( hwnd, HWND_TOPMOST, work_rect.left, work_rect.top, gr_screen.max_w, gr_screen.max_h, 0 );	

		D3D9_cursor_clip_rect.left   = work_rect.left;
		D3D9_cursor_clip_rect.top    = work_rect.top;
		D3D9_cursor_clip_rect.right  = work_rect.left + gr_screen.max_w - 1;
		D3D9_cursor_clip_rect.bottom = work_rect.top  + gr_screen.max_h - 1;
	#else
	
		SetWindowPos( hwnd, HWND_TOPMOST, 0, 0, gr_screen.max_w, gr_screen.max_h, 0 );	

		D3D9_cursor_clip_rect.left   = 0;
		D3D9_cursor_clip_rect.top    = 0;
		D3D9_cursor_clip_rect.right  = gr_screen.max_w-1;
		D3D9_cursor_clip_rect.bottom = gr_screen.max_h-1;
	#endif
	}
}

bool SetupDevice()
{
	HRESULT hr;
	pDirect3D = Direct3DCreate9(D3D_SDK_VERSION);
	assert(pDirect3D);

	D3DDISPLAYMODE d3ddm;
    if( FAILED( pDirect3D->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &d3ddm ) ) )
	{
		// TO DO: Respond to failure of GetAdapterDisplayMode
		return false;
	}

	if( FAILED( hr = pDirect3D->CheckDeviceFormat( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, 
												d3ddm.Format, D3DUSAGE_DEPTHSTENCIL,
												D3DRTYPE_SURFACE, D3DFMT_D16 ) ) )
	{
		//if( hr == D3DERR_NOTAVAILABLE )
			// POTENTIAL PROBLEM: We need at least a 16-bit z-buffer!
			return false;
	}

	D3DCAPS9 d3dCaps;
	if( FAILED( pDirect3D->GetDeviceCaps( D3DADAPTER_DEFAULT, 
		                               D3DDEVTYPE_HAL, &d3dCaps ) ) )
	{
		// TO DO: Respond to failure of GetDeviceCaps
		return false;
	}

	ZeroMemory( &presentationParameters, sizeof(presentationParameters) );
    presentationParameters.BackBufferFormat			= d3ddm.Format;
	presentationParameters.SwapEffect				= D3DSWAPEFFECT_DISCARD;
	presentationParameters.Windowed					= Cmdline_window;
    presentationParameters.EnableAutoDepthStencil	= TRUE;
    presentationParameters.AutoDepthStencilFormat	= D3DFMT_D16;
    presentationParameters.PresentationInterval		= D3DPRESENT_INTERVAL_ONE;

	presentationParameters.BackBufferWidth			= 1024;
	presentationParameters.BackBufferHeight			= 768;
	presentationParameters.hDeviceWindow			= (HWND) os_get_window();
	presentationParameters.BackBufferCount			= 1;

	hr = pDirect3D->CreateDevice(
		D3DADAPTER_DEFAULT, 
		D3DDEVTYPE_HAL, 
		(HWND) os_get_window(),
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
		&presentationParameters,
		&pD3DDevice);
	switch(hr)
	{
		case D3DERR_DEVICELOST: mprintf(("D3DERR_DEVICELOST")); break; 
		case D3DERR_INVALIDCALL: mprintf(("D3DERR_INVALIDCALL")); break; 
		case D3DERR_NOTAVAILABLE: mprintf(("D3DERR_NOTAVAILABLE")); break;
		case D3DERR_OUTOFVIDEOMEMORY: mprintf(("D3DERR_OUTOFVIDEOMEMORY")); break;
	default: break;
	}
	assert(SUCCEEDED(hr));

	gr_start_frame();
 
	return hr == S_OK;
}

void Reset()
{
	pD3DDevice->Reset(&presentationParameters);
	ResetTextureSystem();
	ResetRenderSystem();
	ResetShaderSystem();
}

void gr_d3d9_cleanup()
{
	CleanupShaderSystem();
	CleanupTextureSystem();
	CleanupRenderSystem();

	if(pD3DDevice)
	{
		pD3DDevice->Release();
	}

	if(pDirect3D)
	{
		pDirect3D->Release();
	}
}

void clip_cursor(int active)
{
	if ( active  )	{
		ClipCursor(&D3D9_cursor_clip_rect);
	} else {
		ClipCursor(NULL);
	}
}

void gr_d3d9_flip()
{
	gr_reset_clip();	

	g_MouseController.EvalDeltas();
//	Gr_d3d_mouse_saved = 0;		// assume not saved		

	if ( g_MouseController.IsVisible() )	{				
		gr_reset_clip();
		int mx, my;
		g_MouseController.GetPos( &mx, &my );
		
//		gr_d3d_save_mouse_area(mx,my,32,32);	
		if ( Gr_cursor == -1 )	{
			#ifndef NDEBUG
				gr_set_color(255,255,255);
				gr_line( mx, my, mx+7, my + 7 );
				gr_line( mx, my, mx+5, my );
				gr_line( mx, my, mx, my+5 );
			#endif
		} else {	
			gr_set_bitmap(Gr_cursor);				
			gr_bitmap( mx, my );
		}		
	} 	

	DebugSys::GetProfiler()->EndBlock();
	DebugSys::GetProfiler()->EndFrame();
	DebugSys::Render();
	DebugSys::GetProfiler()->StartFrame();


	DebugSys::GetProfiler()->StartBlock(0xff0000ff);

	gr_stop_frame();

	DebugSys::GetProfiler()->EndBlock();
	DebugSys::GetProfiler()->StartBlock(0xffff0000);

	d3d9_tcache_frame();

	int cnt = D3D9_activate;
	if ( cnt )	{
		D3D9_activate-=cnt;
//		d3d_tcache_flush();
		clip_cursor(1);
	}

	cnt = D3D9_deactivate; 
	if ( cnt )	{
		D3D9_deactivate-=cnt;
		clip_cursor(0);
	}

	// Wait a second
//	os_sleep(1000);	
	os_poll();

	gr_start_frame();
}

void SetupFunctionPointers()
{
	gr_screen.gf_flip = gr_d3d9_flip;
	gr_screen.gf_set_clip = gr_d3d9_set_clip;
	gr_screen.gf_reset_clip = gr_d3d9_reset_clip;
	gr_screen.gf_set_font = grx_set_font;

	gr_screen.gf_init_color = gr_d3d9_init_color;
	gr_screen.gf_set_color_fast = gr_d3d9_set_color_fast;
	gr_screen.gf_set_color = gr_d3d9_set_color;
	gr_screen.gf_init_color = gr_d3d9_init_color;
	gr_screen.gf_init_alphacolor_ptr = gr_d3d9_init_alphacolor;

	gr_screen.gf_set_bitmap_ptr = gr_d3d9_set_bitmap;
	gr_screen.gf_create_shader = gr_d3d9_create_shader;
	gr_screen.gf_set_shader = gr_d3d9_set_shader;
	gr_screen.gf_clear = gr_d3d9_clear;
	gr_screen.gf_bitmap = gr_d3d9_bitmap;
	gr_screen.gf_aabitmap = gr_d3d9_aabitmap;
	gr_screen.gf_aabitmap_ex = gr_d3d9_aabitmap_ex;

	gr_screen.gf_string = gr_d3d9_string;
	gr_screen.gf_circle = gr_d3d9_circle;

	gr_screen.gf_line = gr_d3d9_line;
	gr_screen.gf_aaline = gr_d3d9_aaline;
	gr_screen.gf_pixel = gr_d3d9_pixel;
	gr_screen.gf_scaler = gr_d3d9_scaler;
	gr_screen.gf_aascaler = gr_d3d9_aascaler;
	gr_screen.gf_tmapper = gr_d3d9_tmapper;

	gr_screen.gf_gradient = gr_d3d9_gradient;

	gr_screen.gf_set_palette_ptr = gr_d3d9_set_palette;
	gr_screen.gf_print_screen = gr_d3d9_print_screen;

	gr_screen.gf_flash = gr_d3d9_flash;

	gr_screen.gf_zbuffer_get = gr_d3d9_zbuffer_get;
	gr_screen.gf_zbuffer_set = gr_d3d9_zbuffer_set;
	gr_screen.gf_zbuffer_clear = gr_d3d9_zbuffer_clear;

	gr_screen.gf_save_screen = gr_d3d9_save_screen;
	gr_screen.gf_restore_screen = gr_d3d9_restore_screen;
	gr_screen.gf_free_screen = gr_d3d9_free_screen;

	// Screen dumping stuff
	gr_screen.gf_dump_frame_start = gr_d3d9_dump_frame_start;
	gr_screen.gf_dump_frame_stop = gr_d3d9_dump_frame_stop;
	gr_screen.gf_dump_frame = gr_d3d9_dump_frame;

	gr_screen.gf_set_gamma = gr_d3d9_set_gamma; 

	// Lock/unlock stuff
	gr_screen.gf_lock = gr_d3d9_lock;
	gr_screen.gf_unlock = gr_d3d9_unlock;

	// screen region
	gr_screen.gf_get_region = gr_d3d9_get_region;

	// fog stuff
	gr_screen.gf_fog_set_ptr = gr_d3d9_fog_set;

	// poly culling
	gr_screen.gf_set_cull = gr_d3d9_set_cull;

	// cross fade
	gr_screen.gf_cross_fade = gr_d3d9_cross_fade;

	// texture cache
	gr_screen.gf_tcache_set_ptr = gr_d3d9_tcache_set;

	// set clear color
	gr_screen.gf_set_clear_color = gr_d3d9_set_clear_color;

	gr_screen.gf_clean = gr_d3d9_cleanup;

	gr_screen.gf_zbias		  = gr_d3d9_zbias;

	gr_screen.gf_start_frame  = gr_d3d9_start_frame;
	gr_screen.gf_stop_frame   = gr_d3d9_stop_frame;

	gr_screen.gf_tcache_flush = gr_d3d9_tcache_flush;
}

IDirect3DDevice9 *GetDevice()
{
	return pD3DDevice;
}