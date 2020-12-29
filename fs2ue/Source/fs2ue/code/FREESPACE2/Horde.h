#pragma once

#if defined(HORDE_MODE)
void HordeInit();
void HordeDeinit();
void HordeStart();
void HordeRender();
void HordeUpdate(float flFrametime);
void HordeUpdateInput(const int key);
#else
static void HordeInit() {}
static void HordeDeinit() {}
static void HordeStart() {}
static void HordeRender() {}
static void HordeUpdate(float flFrametime) {}
static void HordeUpdateInput(const int key) {}
#endif