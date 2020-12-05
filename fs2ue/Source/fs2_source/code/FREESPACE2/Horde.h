#pragma once

#if defined(HORDE_MODE)
void HordeInit();
void HordeDeinit();
void HordeStart();
void HordeRender();
void HordeUpdate(float flFrametime);
void HordeUpdateInput(const int key);
#else
void HordeInit() {}
void HordeDeinit() {}
void HordeStart() {}
void HordeRender() {}
void HordeUpdate(float flFrametime) {}
void HordeUpdateInput(const int key) {}
#endif