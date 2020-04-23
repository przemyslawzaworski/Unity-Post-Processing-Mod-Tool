using UnityEngine;
using System;
using System.Collections;
using System.Runtime.InteropServices;
using UnityEngine.Rendering;
using System.IO;

public class PostProcessingMod : MonoBehaviour
{
	public Shader RenderDepthShader;

	[DllImport("PostProcessingMod")]
	static extern IntPtr Execute();

	[DllImport("PostProcessingMod")]
	static extern void SetTextures(IntPtr color, IntPtr depth);

	[DllImport("PostProcessingMod")]
	static extern void SetTime(float time);

	[DllImport("PostProcessingMod")]
	static extern bool Init([MarshalAs(UnmanagedType.LPWStr)] String path);

	Camera _Camera;
	Material _Material;
	RenderTexture _TempColorBuffer, _TempDepthBuffer;
	Texture2D _ColorBuffer, _DepthBuffer;
	bool _Success = false;
	
	void Start()
	{
		try
		{
			StreamReader reader = new StreamReader(Path.Combine(Application.streamingAssetsPath, "Shaders//active.txt"));
			string info = reader.ReadLine();
			reader.Close();
			_Success = Init(System.IO.Path.Combine(Application.streamingAssetsPath, "Shaders//" + info));
			_Material = new Material(RenderDepthShader);
			_Camera = GetComponent<Camera>();
			_Camera.depthTextureMode = DepthTextureMode.Depth;
			_ColorBuffer = new Texture2D(Screen.width, Screen.height, TextureFormat.ARGB32, false);
			_DepthBuffer = new Texture2D(Screen.width, Screen.height, TextureFormat.ARGB32, false);
			_TempColorBuffer = new RenderTexture(Screen.width, Screen.height, 0, RenderTextureFormat.ARGB32);
			_TempDepthBuffer = new RenderTexture(Screen.width, Screen.height, 0, RenderTextureFormat.ARGB32);
		}
		catch (System.Exception) { _Success = false; }
	}
	
	void OnScreenResize()
	{
		if ( (_ColorBuffer.width != Screen.width) || (_ColorBuffer.height != Screen.height))
		{
			_ColorBuffer = new Texture2D(Screen.width, Screen.height, TextureFormat.ARGB32, false);
			_DepthBuffer = new Texture2D(Screen.width, Screen.height, TextureFormat.ARGB32, false);
			_TempColorBuffer.Release();
			_TempColorBuffer = new RenderTexture(Screen.width, Screen.height, 0, RenderTextureFormat.ARGB32);
			_TempDepthBuffer.Release();
			_TempDepthBuffer = new RenderTexture(Screen.width, Screen.height, 0, RenderTextureFormat.ARGB32);
		}
	}

	void OnRenderImage (RenderTexture source, RenderTexture destination)
	{
		if (_Success)
		{
			OnScreenResize();
			if (_Camera.allowHDR)
			{
				Graphics.Blit(source, _TempColorBuffer);
				Graphics.CopyTexture(_TempColorBuffer, _ColorBuffer);
			}
			else
			{
				Graphics.CopyTexture(source, _ColorBuffer);
			}
			Graphics.Blit (source, _TempDepthBuffer, _Material);
			Graphics.CopyTexture(_TempDepthBuffer, _DepthBuffer);
			SetTextures(_ColorBuffer.GetNativeTexturePtr(), _DepthBuffer.GetNativeTexturePtr());
			SetTime(Time.time);
		}
		Graphics.Blit(source, destination);
		if (_Success) GL.IssuePluginEvent(Execute(), 1);
	}
}