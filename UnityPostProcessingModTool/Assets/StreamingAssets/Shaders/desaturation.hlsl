static const float3 vertices[6] = {float3(1,-1,0),float3(-1,-1,0),float3(1,1,0), float3(-1,-1,0),float3(-1,1,0),float3(1,1,0)};
static const float2 uvs[6] = {float2(1,0),float2(0,0),float2(1,1), float2(0,0),float2(0,1),float2(1,1)};

void VSMain(out float4 vertex:SV_POSITION, out float2 uv:TEXCOORD0, in uint id:SV_VertexID)
{
	uv = uvs[id];
	vertex = float4(vertices[id], 1);
}

Texture2D<float4> pattern : register(t0);
SamplerState state {Filter = MIN_MAG_LINEAR_MIP_POINT;};
cbuffer Constants : register(b0) { float iTime; };

float4 PSMain(float4 vertex:SV_POSITION, float2 uv:TEXCOORD0) : SV_TARGET
{	
	uv.x+=cos(uv.y*2.0+iTime)*0.05;
	uv.y+=sin(uv.x*2.0+iTime)*0.05;
	float offset = sin(iTime *0.5) * 0.01;    
	float4 a = pattern.Sample(state, uv);    
	float4 b = pattern.Sample(state, uv-float2(sin(offset),0.0));    
	float4 c = pattern.Sample(state, uv+float2(sin(offset),0.0));    
	float4 d = pattern.Sample(state, uv-float2(0.0,sin(offset)));    
	float4 e = pattern.Sample(state, uv+float2(0.0,sin(offset)));
	float4 color = (a+b+c+d+e) / 5.0;
	float m = dot(color.rgb, float3(0.2126, 0.7152, 0.0722));
	return float4(m, m, m, 1.0);
}