#pragma once
#include <d3d11_1.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <wrl/client.h>

using namespace DirectX;

struct N3sD3dContext {
	Microsoft::WRL::ComPtr<ID3D11Device>            device;
	Microsoft::WRL::ComPtr<ID3D11Device1>           device1;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext>     context;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    context1;
};