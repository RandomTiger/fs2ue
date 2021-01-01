#pragma once

bool gr_dummy_init();
void gr_dummy_deinit();

#ifdef FS2_UE
UTexture2D *create_texture(int bitmap_handle, int bitmap_type, float &uScale, float &vScale);
#endif