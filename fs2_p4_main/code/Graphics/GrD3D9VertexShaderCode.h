#pragma once

const char * const PassthroughVertexShader = 
"float4x4  matWVP : register(c0);"
"struct VS" 
"{"                                             
"    float4 Pos   : POSITION;"              
"    float4 Color    : COLOR;"   
"    float2 UV       : TEXCOORD;"
"};"                                                                                   
""                                           
"VS main( VS In )" 
"{"                                             
"    VS Out;"      
"    Out.Pos   = mul( matWVP, In.Pos );"
//"    Out.Pos.x = In.Pos.x / 1024.0 * 2.0 - 1.0;" 
//"    Out.Pos.y = 1.0 - (In.Pos.y / 768.0) * 2.0;" 
//"    Out.Pos.z = 1.0;"
//"    Out.Pos.w = 1.0;"
"    Out.Color = In.Color;"   
"    Out.UV    = In.UV;"
"    return Out;"                                 
"}";

const char * const PassthroughVertexShaderNoTexture = 
"float4x4  matWVP : register(c0);"
"struct VS" 
"{"                                             
"    float4 Pos   : POSITION;"              
"    float4 Color    : COLOR;"   
"};"                                                                                   
""                                           
"VS main( VS In )" 
"{"                                             
"    VS Out;"                               
"    Out.Pos   = mul( matWVP, In.Pos );"
"    Out.Color = In.Color;"   
"    return Out;"                                 
"}";