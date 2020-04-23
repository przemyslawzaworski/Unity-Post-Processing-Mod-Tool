static const float3 vertices[6] = {float3(1,-1,0),float3(-1,-1,0),float3(1,1,0), float3(-1,-1,0),float3(-1,1,0),float3(1,1,0)};
static const float2 uvs[6] = {float2(1,0), float2(0,0), float2(1,1), float2(0,0), float2(0,1), float2(1,1)};

void VSMain(out float4 vertex:SV_POSITION, out float2 uv:TEXCOORD0, in uint id:SV_VertexID)
{
	uv = uvs[id];
	vertex = float4(vertices[id], 1);
}

Texture2D<float4> colormap : register(t0);
Texture2D<float4> depthmap : register(t1);
SamplerState state {Filter = MIN_MAG_LINEAR_MIP_POINT;};
cbuffer Constants : register(b0) { float iTime; };

float4 PSMain(float4 vertex:SV_POSITION, float2 texcoord:TEXCOORD0) : SV_TARGET
{
	if (texcoord.x < 0.5)
	{
		float4 p = colormap.Sample(state, texcoord);
		float4 a = float4(0,0,1,1);
		float4 b = float4(1,1,0,1);
		float4 c = float4(1,0,0,1);
		float lum = (p.r+p.g+p.b)/3.0;
		return (lum < 0.5) ? lerp(a,b,(lum)/0.5) : lerp(b,c,(lum-0.5)/0.5);		
	}
	else
	{
		float depth = depthmap.Sample(state, texcoord).r;
		return float4(depth.xxx, 1.0);
	} 
}