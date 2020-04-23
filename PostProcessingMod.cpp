//For x64 Visual Studio command line:  cl.exe /LD PostProcessingMod.cpp d3d11.lib dxguid.lib user32.lib kernel32.lib gdi32.lib d3dcompiler.lib

#include <d3d11.h>
#include <d3dcompiler.h>
#include <stdio.h>
 
static ID3D11Device *device;
static ID3D11DeviceContext *context;
static ID3D11VertexShader *vs;
static ID3D11PixelShader *ps;
static ID3DBlob *VSblob;
static ID3DBlob *PSblob;
static ID3DBlob *VSerror;
static ID3DBlob *PSerror;
static ID3D11ShaderResourceView *srvc;
static ID3D11ShaderResourceView *srvd;
static ID3D11Resource *colormap;
static ID3D11Resource *depthmap;
static ID3D11Buffer *buffer;
static float time;
 
typedef enum UnityGfxRenderer
{
	kUnityGfxRendererD3D11 = 2,
	kUnityGfxRendererNull = 4,
} UnityGfxRenderer;
 
typedef enum UnityGfxDeviceEventType
{
	kUnityGfxDeviceEventInitialize = 0,
	kUnityGfxDeviceEventShutdown = 1,
	kUnityGfxDeviceEventBeforeReset = 2,
	kUnityGfxDeviceEventAfterReset = 3,
} UnityGfxDeviceEventType;
 
typedef void (__stdcall* IUnityGraphicsDeviceEventCallback)(UnityGfxDeviceEventType eventType);
 
struct UnityInterfaceGUID
{
	UnityInterfaceGUID(unsigned long long high, unsigned long long low) : m_GUIDHigh(high) , m_GUIDLow(low) { }
	unsigned long long m_GUIDHigh;
	unsigned long long m_GUIDLow;
};
 
struct IUnityInterfaces
{
	void* (__stdcall* GetInterface)(UnityInterfaceGUID guid);
	void (__stdcall* RegisterInterface)(UnityInterfaceGUID guid, void *ptr);
	template<typename INTERFACE>
	INTERFACE* Get()
	{
		return static_cast<INTERFACE*>(GetInterface(UnityInterfaceGUID(0xAAB37EF87A87D748ULL, 0xBF76967F07EFB177ULL)));
	}
	void Register(void* ptr)
	{
		RegisterInterface(UnityInterfaceGUID(0xAAB37EF87A87D748ULL, 0xBF76967F07EFB177ULL), ptr);
	}
};
 
struct IUnityGraphics
{
	void (__stdcall* RegisterDeviceEventCallback)(IUnityGraphicsDeviceEventCallback callback);
};
 
struct IUnityGraphicsD3D11
{
	ID3D11Device* (__stdcall * GetDevice)();
};
 
typedef void (__stdcall* UnityRenderingEvent)(int eventId);
typedef void (__stdcall* UnregisterDeviceEventCallback)(IUnityGraphicsDeviceEventCallback callback);
 
static IUnityInterfaces* s_UnityInterfaces;
static UnityGfxRenderer DeviceType = kUnityGfxRendererNull;
 
extern "C" void __declspec(dllexport) __stdcall SetTextures (void* color, void* depth)
{
	colormap = (ID3D11Resource*)(color);
	depthmap = (ID3D11Resource*)(depth);
}

extern "C" void __declspec(dllexport) __stdcall SetTime (float t)
{
	time = t;
}

bool IsCompiled (ID3DBlob* error, HRESULT result)
{
	if (result != S_OK)
	{
		if (error)
		{
			FILE* file = fopen ("debug.log", "a");
			fwrite(error->GetBufferPointer(), 1, error->GetBufferSize(), file);
			fclose (file);
		}
		return false;
	}
	return true;
}

extern "C" bool __declspec(dllexport) __stdcall Init(LPCWSTR pFileName)
{
	DeviceType = kUnityGfxRendererD3D11;
	IUnityGraphicsD3D11* d3d = s_UnityInterfaces->Get<IUnityGraphicsD3D11>();
	device = d3d->GetDevice();
	device->GetImmediateContext(&context);
	HRESULT VSresult = D3DCompileFromFile(pFileName, 0, 0, "VSMain", "vs_5_0", 1 << 15, 0, &VSblob, &VSerror);
	if (!IsCompiled(VSerror, VSresult)) return false;
	device->CreateVertexShader(VSblob->GetBufferPointer(), VSblob->GetBufferSize(), 0, &vs);
	HRESULT PSresult = D3DCompileFromFile(pFileName, 0, 0, "PSMain", "ps_5_0", 1 << 15, 0, &PSblob, &PSerror);
	if (!IsCompiled(PSerror, PSresult)) return false;
	device->CreatePixelShader(PSblob->GetBufferPointer(), PSblob->GetBufferSize(), 0, &ps);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	D3D11_BUFFER_DESC desc = {16, D3D11_USAGE_DEFAULT, D3D11_BIND_CONSTANT_BUFFER, 0, 0, 0};
	device->CreateBuffer(&desc, 0, &buffer);
	return true;
}

void Update()
{
	context->VSSetShader(vs, 0, 0);
	context->PSSetShader(ps, 0, 0);
	device->CreateShaderResourceView(colormap, 0, &srvc);
	device->CreateShaderResourceView(depthmap, 0, &srvd);
	context->PSSetShaderResources(0, 1, &srvc);
	context->PSSetShaderResources(1, 1, &srvd);
	context->UpdateSubresource(buffer, 0, 0, &time, 4, 4);
	context->PSSetConstantBuffers(0, 1, &buffer );
	context->Draw(6, 0);
	srvc->Release();
	srvd->Release();
	colormap->Release();
	depthmap->Release();
}
 
void Release()
{
	if (vs) vs->Release();
	if (ps) ps->Release();
	if (VSblob) VSblob->Release();
	if (PSblob) PSblob->Release();
	if (buffer) buffer->Release();
	if (srvc) srvc->Release();
	if (srvd) srvd->Release();
	if (colormap) colormap->Release();
	if (depthmap) depthmap->Release();
	if (context) context->Release();
	if (device) device->Release();
}

static void __stdcall OnGraphicsDeviceEvent(UnityGfxDeviceEventType eventType)
{
	if (eventType == kUnityGfxDeviceEventShutdown)
	{
		DeviceType = kUnityGfxRendererNull;
		Release();
	}
}
 
static void __stdcall OnRenderEvent(int eventID)
{
	Update();
}
 
extern "C" void __declspec(dllexport) __stdcall UnityPluginLoad(IUnityInterfaces* unityInterfaces)
{
	s_UnityInterfaces = unityInterfaces;
	IUnityGraphics* s_Graphics = s_UnityInterfaces->Get<IUnityGraphics>();
	s_Graphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);
	OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}
 
extern "C" void __declspec(dllexport) __stdcall UnityPluginUnload()
{
	UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}
 
extern "C" UnityRenderingEvent __declspec(dllexport) __stdcall Execute()
{
	return OnRenderEvent;
}
 