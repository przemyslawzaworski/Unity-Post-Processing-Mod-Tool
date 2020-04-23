Shader "PostProcessingMod/RenderDepth"
{
	Subshader
	{
		Pass
		{
			CGPROGRAM
			#pragma vertex VSMain
			#pragma fragment PSMain
			#pragma target 5.0

			sampler2D _CameraDepthTexture;

			float4 VSMain (in float4 vertex:POSITION, inout float2 uv:TEXCOORD0) : SV_POSITION
			{
				return UnityObjectToClipPos(vertex);
			}

			float4 PSMain (float4 vertex:SV_POSITION, float2 uv:TEXCOORD0) : SV_TARGET
			{
				return tex2D(_CameraDepthTexture, uv);
			}
			ENDCG
		}
	}
}