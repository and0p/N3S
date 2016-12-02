// Obviously thanks to http://www.directxtutorial.com/ for a bunch of this
// As well as http://www.braynzarsoft.net/viewtutorial/q16390-4-begin-drawing

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "N3s3d.hpp"

using namespace std;
Microsoft::WRL::ComPtr<ID3D11Device>            N3S3d::device;
Microsoft::WRL::ComPtr<ID3D11Device1>           N3S3d::device1;
Microsoft::WRL::ComPtr<ID3D11DeviceContext>     N3S3d::context;
Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    N3S3d::context1;
ID3D11SamplerState *N3S3d::sampleState;
ID3D11Buffer *N3S3d::worldMatrixBuffer;
ID3D11Buffer *N3S3d::viewMatrixBuffer;
ID3D11Buffer *N3S3d::projectionMatrixBuffer;
ID3D11Buffer *N3S3d::mirrorBuffer;
ID3D11Buffer *N3S3d::paletteBuffer;
ID3D11Buffer *N3S3d::paletteSelectionBuffer;
ID3D11Buffer *N3S3d::indexBuffer;
int N3S3d::paletteBufferNumber;
int N3S3d::paletteSelectionBufferNumber;
ShaderSet N3S3d::shaderSets[2];
ID3D11InputLayout *N3S3d::inputLayouts[2];
ID3D11Texture2D *N3S3d::texture2d;
ID3D11ShaderResourceView *N3S3d::textureView;
ShaderType N3S3d::activeShader;
D3D11_SUBRESOURCE_DATA N3S3d::subData;
MirrorState N3S3d::mirrorState;
D3D11_DEPTH_STENCIL_DESC N3S3d::depthStencilDesc;
D3D11_DEPTH_STENCIL_DESC N3S3d::depthDisabledStencilDesc;
ID3D11DepthStencilState* N3S3d::m_depthStencilState;
ID3D11DepthStencilState* N3S3d::m_depthDisabledStencilState;
PPUHueStandardCollection N3S3d::ppuHueStandardCollection;
ID3D11DepthStencilView* N3S3d::m_depthStencilView;
int N3S3d::selectedPalette;
int N3S3d::mirrorBufferNumber;

void N3S3d::initPipeline(VxlD3DContext c)
{
	device = c.device;
	context = c.context;
	device1 = c.device1;
	context1 = c.context1;
	// Create view buffer desc
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(XMMATRIX);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	device1->CreateBuffer(&matrixBufferDesc, NULL, &worldMatrixBuffer);
	device1->CreateBuffer(&matrixBufferDesc, NULL, &viewMatrixBuffer);
	device1->CreateBuffer(&matrixBufferDesc, NULL, &projectionMatrixBuffer);

	// Create mirror buffer desc
	D3D11_BUFFER_DESC mirrorBufferDesc;
	mirrorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	mirrorBufferDesc.ByteWidth = 16;
	mirrorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	mirrorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	mirrorBufferDesc.MiscFlags = 0;
	mirrorBufferDesc.StructureByteStride = 0;

	device1->CreateBuffer(&mirrorBufferDesc, NULL, &mirrorBuffer);

	// Create palette buffer desc
	D3D11_BUFFER_DESC paletteBufferDesc;
	paletteBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	paletteBufferDesc.ByteWidth = sizeof(float) * 96;
	paletteBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	paletteBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	paletteBufferDesc.MiscFlags = 0;
	paletteBufferDesc.StructureByteStride = 0;

	device1->CreateBuffer(&paletteBufferDesc, NULL, &paletteBuffer);

	// Create palette selection buffer desc
	D3D11_BUFFER_DESC paletteSelectionBufferDesc;
	paletteSelectionBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	paletteSelectionBufferDesc.ByteWidth = 16;
	paletteSelectionBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	paletteSelectionBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	paletteSelectionBufferDesc.MiscFlags = 0;
	paletteSelectionBufferDesc.StructureByteStride = 0;

	device1->CreateBuffer(&paletteSelectionBufferDesc, NULL, &paletteSelectionBuffer);

	initDepthStencils();
	enabledDepthBuffer();

#ifndef HOLOLENS
	initShaders();
	setShader(color);
	initSampleState();

	// select which primtive type we are using
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create texture description
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = 256;
	desc.Height = 240;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.SampleDesc.Quality = 0;

	subData.SysMemPitch = (UINT)(1024);

	device->CreateTexture2D(&desc, NULL, &texture2d);
	device->CreateShaderResourceView(texture2d, NULL, &textureView);
#endif

#ifdef HOLOLENS
	createIndexBuffer();
#endif

	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_NONE;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = true;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	ID3D11RasterizerState * m_rasterState;
	device->CreateRasterizerState(&rasterDesc, &m_rasterState);
	context->RSSetState(m_rasterState);

	mirrorState = { 0, 0 };
	updateMirroring(false, false);

	selectPalette(0);
	float paletteArray[72] =
	{
		1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f
	};
	updatePalette(paletteArray);

	// Set buffer slots based on whether we're compiling for Hololens or not
#ifdef HOLOLENS
	mirrorBufferNumber = 2;
	paletteBufferNumber = 3;
	paletteSelectionBufferNumber = 4;
#else
	mirrorBufferNumber = 3;
	paletteBufferNumber = 4;
	paletteSelectionBufferNumber = 5;
#endif
}

void N3S3d::initShaders() {
	ifstream s_stream;
	size_t s_size;
	char* s_data;
	s_stream.open(L"color_vertex.cso", ifstream::in | ifstream::binary);
	s_stream.seekg(0, ios::end);
	s_size = size_t(s_stream.tellg());
	s_data = new char[s_size];
	s_stream.seekg(0, ios::beg);
	s_stream.read(&s_data[0], s_size);
	s_stream.close();

	device1->CreateVertexShader(s_data, s_size, 0, &shaderSets[0].vertexShader);

	D3D11_INPUT_ELEMENT_DESC colorLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	device1->CreateInputLayout(colorLayout, 2, &s_data[0], s_size, &inputLayouts[0]);

	s_stream.open(L"color_pixel.cso", ifstream::in | ifstream::binary);
	s_stream.seekg(0, ios::end);
	s_size = size_t(s_stream.tellg());
	s_data = new char[s_size];
	s_stream.seekg(0, ios::beg);
	s_stream.read(&s_data[0], s_size);
	s_stream.close();

	device1->CreatePixelShader(s_data, s_size, 0, &shaderSets[0].pixelShader);

	updateMirroring(true, true); // TODO: Make mirror cbuffer init cleaner
}

void N3S3d::setShader(ShaderType type) {
	switch (type)
	{
	case color:
		context->IASetInputLayout(inputLayouts[0]);
		context->GSSetShader(NULL, 0, 0);
		context->VSSetShader(shaderSets[0].vertexShader, 0, 0);
		context->PSSetShader(shaderSets[0].pixelShader, 0, 0);
		activeShader = color;
		break;
	}
}

ID3D11Buffer* N3S3d::createBufferFromColorVertices(ColorVertex vertices[], int arraySize)
{
	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;						// write access access by CPU and GPU
	bd.ByteWidth = sizeof(ColorVertex) * arraySize;     // size is the VERTEX struct * 3in
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;			// use as a vertex buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;			// allow CPU to write in buffer

	ID3D11Buffer *pVBuffer;								// the vertex buffer
	device1->CreateBuffer(&bd, NULL, &pVBuffer);		// create the buffer

	// copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT result = context1->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	memcpy(ms.pData, vertices, arraySize);							// copy the data
	context1->Unmap(pVBuffer, NULL);                                         // unmap the buffer
	return pVBuffer;
}

ID3D11Buffer* N3S3d::createBufferFromColorVerticesV(std::vector<ColorVertex>  * vertices, int arraySize)
{
	if (vertices->size() == 0) {
		return nullptr;
	}
	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;						// write access access by CPU and GPU
	bd.ByteWidth = sizeof(ColorVertex) * arraySize;     // size is the VERTEX struct * 3in
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;			// use as a vertex buffer
	bd.CPUAccessFlags = 0;								// disallow CPU to write in buffer

	ID3D11Buffer *pVBuffer;								// the vertex buffer
	D3D11_SUBRESOURCE_DATA vData;
	vData.pSysMem = &(vertices->data()[0]);
	device1->CreateBuffer(&bd, &vData, &pVBuffer);		// create the buffer
	return pVBuffer;
}

void N3S3d::updateWorldMatrix(float x, float y, float z) {
	XMMATRIX worldMatrix = XMMatrixIdentity();
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;
	XMVECTOR translation = { x, y, z, 0.0f };
	worldMatrix = XMMatrixTranslationFromVector(translation);
	worldMatrix = XMMatrixTranspose(worldMatrix);
	// Lock the constant buffer so it can be written to.
	context1->Map(worldMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = worldMatrix;
	// Unlock the constant buffer.
	context1->Unmap(worldMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(0, 1, &worldMatrixBuffer);
}

void N3S3d::updateMirroring(bool horizontal, bool vertical) {
	int x, y;
	if (horizontal)
		x = -1;
	else
		x = 1;
	if (vertical)
		y = -1;
	else
		y = 1;
	if (mirrorState.x != x || mirrorState.y != y)
	{
		mirrorState.x = x;
		mirrorState.y = y;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		MirrorState* dataPtr;
		// Lock the constant buffer so it can be written to.
		context1->Map(mirrorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		// Get a pointer to the data in the constant buffer.
		dataPtr = (MirrorState*)mappedResource.pData;
		// Copy the values into the constant buffer.
		dataPtr->x = x;
		dataPtr->y = y;
		// Unlock the constant buffer.
		context1->Unmap(mirrorBuffer, 0);
		// Finally set the constant buffer in the vertex shader with the updated values.
		context1->VSSetConstantBuffers(mirrorBufferNumber, 1, &mirrorBuffer);
	}
}

void N3S3d::updatePalette(float palette[72])
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	float* dataPtr;
	// Lock the constant buffer so it can be written to.
	context1->Map(paletteBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (float*)mappedResource.pData;
	// Copy the values into the constant buffer.
	int count = 0;
	for (int i = 0; i < 96; i++)
	{
		if ((i + 1) % 4 != 0)
		{
			*dataPtr = palette[count];
			count++;
		}
		else
			*dataPtr = 0.0f;
		dataPtr++;
	}
	// Unlock the constant buffer.
	context1->Unmap(paletteBuffer, 0);
	// Finally set the constant buffer in the pixel shader with the updated values.
	context1->VSSetConstantBuffers(paletteBufferNumber, 1, &paletteBuffer);
}

void N3S3d::selectPalette(int palette)
{
	if (palette != selectedPalette)
	{
		selectedPalette = palette;
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		int* dataPtr;
		// Lock the constant buffer so it can be written to.
		context1->Map(paletteSelectionBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		// Get a pointer to the data in the constant buffer.
		dataPtr = (int*)mappedResource.pData;
		// Copy the values into the constant buffer.
		*dataPtr = palette;
		// Unlock the constant buffer.
		context1->Unmap(paletteSelectionBuffer, 0);
		// Finally set the constant buffer in the pixel shader with the updated values.
		context1->VSSetConstantBuffers(paletteSelectionBufferNumber, 1, &paletteSelectionBuffer);
	}
}

void N3S3d::updateMatricesWithCamera(VxlCamera * camera) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;

	worldMatrix = XMMatrixIdentity();
	projectionMatrix = getProjectionMatrix(0.1f, 1000.0f, 1, 1);
	viewMatrix = camera->GetViewMatrix();

	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	context1->Map(worldMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = worldMatrix;
	// Unlock the constant buffer.
	context1->Unmap(worldMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(0, 1, &worldMatrixBuffer);

	// Lock the constant buffer so it can be written to.
	context1->Map(viewMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = viewMatrix;
	// Unlock the constant buffer.
	context1->Unmap(viewMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(1, 1, &viewMatrixBuffer);

	// Lock the constant buffer so it can be written to.
	context1->Map(projectionMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = projectionMatrix;
	// Unlock the constant buffer.
	context1->Unmap(projectionMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(2, 1, &projectionMatrixBuffer);
}

void N3S3d::updateViewMatrices(XMFLOAT4X4 view, XMFLOAT4X4 projection)
{
	XMMATRIX viewMatrix, projectionMatrix;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;

	viewMatrix = XMLoadFloat4x4(&view);
	projectionMatrix = XMLoadFloat4x4(&projection);

	// Lock the constant buffer so it can be written to.
	context1->Map(viewMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = viewMatrix;
	// Unlock the constant buffer.
	context1->Unmap(viewMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(1, 1, &viewMatrixBuffer);

	// Lock the constant buffer so it can be written to.
	context1->Map(projectionMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = projectionMatrix;
	// Unlock the constant buffer.
	context1->Unmap(projectionMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(2, 1, &projectionMatrixBuffer);
}

XMMATRIX N3S3d::getProjectionMatrix(const float near_plane, const float far_plane, const float fov_horiz, const float fov_vert)
{
	float    h, w, Q;

	w = (float)1 / tan(fov_horiz*0.5);  // 1/tan(x) == cot(x)
	h = (float)1 / tan(fov_vert*0.5);   // 1/tan(x) == cot(x)
	Q = far_plane / (far_plane - near_plane);

	XMMATRIX ret;
	ZeroMemory(&ret, sizeof(ret));

	XMFLOAT4X4 tmp;
	XMStoreFloat4x4(&tmp, ret);
	
	tmp(0, 0) = w;
	tmp(1, 1) = h;
	tmp(2, 2) = Q;
	tmp(3, 2) = -Q*near_plane;
	tmp(2, 3) = 1;
	return XMLoadFloat4x4(&tmp);
}

void N3S3d::setIndexBuffer()
{
	context->IASetIndexBuffer(
		indexBuffer,
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);
}

void N3S3d::enabledDepthBuffer()
{
	context->OMSetDepthStencilState(m_depthStencilState, 1);
}

void N3S3d::disableDepthBuffer()
{
	context->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
}

void N3S3d::renderMesh(VoxelMesh *voxelMesh) {
	ShaderType type = voxelMesh->type;
	UINT stride = sizeof(ColorVertex); // TODO optimize
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &voxelMesh->buffer, &stride, &offset);
	//context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// draw the vertex buffer to the back buffer
#ifdef HOLOLENS
		context->DrawIndexedInstanced(
			voxelMesh->size,   // Index count per instance.
		    2,              // Instance count.
		    0,              // Start index location.
		    0,              // Base vertex location.
		    0               // Start instance location.
		    );
#else
	context->Draw(voxelMesh->size, 0);
#endif
}

void N3S3d::initSampleState() {
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	device->CreateSamplerState(&samplerDesc, &sampleState);
}

void N3S3d::createIndexBuffer()
{
	unsigned short indices[73728];
	for (int i = 0; i < 73728; i++)
	{
		indices[i] = i;
	}
	D3D11_SUBRESOURCE_DATA indexBufferData = { 0 };
	indexBufferData.pSysMem = indices;
	indexBufferData.SysMemPitch = 0;
	indexBufferData.SysMemSlicePitch = 0;
	const CD3D11_BUFFER_DESC indexBufferDesc(sizeof(indices), D3D11_BIND_INDEX_BUFFER);
	device->CreateBuffer(
		&indexBufferDesc,
		&indexBufferData,
		&indexBuffer
		);
}

bool N3S3d::initDepthStencils()
{
	// Thanks http://www.rastertek.com/dx11tut11.html
	HRESULT result = false;
	
	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);
	if (FAILED(result))
	{
		return false;
	}

	// Clear the second depth stencil state before setting the parameters.
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

	// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
	// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = true;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the state using the device.
	result = device->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);
	if (FAILED(result))
	{
		return false;
	}

	// Set the z-buffer enabled depth stencil state
	context->OMSetDepthStencilState(m_depthStencilState, 1);
	
	// If it all succeeded, return true
	return true;
}

bool toggleBool(bool b)
{
	if (b == false)
		return true;
	else
		return false;;
}

// thx https://www.safaribooksonline.com/library/view/c-cookbook/0596007612/ch10s17.html
string replaceExt(string input, string newExt) {
	string::size_type i = input.rfind('.', input.length());

	if (i != string::npos) {
		input.replace(i + 1, newExt.length(), newExt);
	}
	return input;
}

bool Crop::zeroed()
{
	if (top + left + right + bottom == 0)
		return true;
	else
		return false;
}
