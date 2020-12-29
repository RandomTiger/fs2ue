#pragma once

const char * const PassthroughPixelShader = 
"sampler2D ColorTexture : register(s0);"
""
"struct PS_IN"                                 
"{ "                                           
"    float2 UV : TEXCOORD0; "                   
"};"                                           
""                                               
"float4 main( float4 vColorIn: COLOR0, PS_IN In ) : COLOR  "              
"{"         
"	float4 Color = tex2D( ColorTexture, In.UV ) * vColorIn;"
//"	result[0] = 1.0;"
/*
"	Color.rgb = (Color.r+Color.g+Color.b)/3.0f; "
"	Color -= tex2D(ColorTexture , In.UV.xy-0.003)*2.7f;  "
"	Color += tex2D(ColorTexture , In.UV.xy+0.003)*2.7f;  "
"	Color.rgb = (Color.r+Color.g+Color.b)/3.0f; "
*/
"    return Color; "                        
"}";

const char * const NoTexturePixelShader = 
"sampler2D ColorTexture : register(s0);"
""
"struct PS_IN"                                 
"{ "                                           
"    float2 UV : TEXCOORD0; "                   
"};"                                           
""                                               
"float4 main( float4 vColorIn: COLOR0, PS_IN In ) : COLOR  "              
"{"         
"    return vColorIn; "                        
"}";