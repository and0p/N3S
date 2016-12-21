// Obviously thanks to http://www.directxtutorial.com/ for a bunch of this
// As well as http://www.braynzarsoft.net/viewtutorial/q16390-4-begin-drawing

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "N3s3d.hpp"

using namespace std;

Microsoft::WRL::ComPtr<ID3D11Device>            device;
Microsoft::WRL::ComPtr<ID3D11Device1>           device1;
Microsoft::WRL::ComPtr<ID3D11DeviceContext>     context;
Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    context1;

Shader shaders[shaderCount];
//ID3D11InputLayout *inputLayouts[2];
//ID3D11Buffer *worldMatrixBuffer;
//ID3D11Buffer *viewMatrixBuffer;
//ID3D11Buffer *projectionMatrixBuffer;
ID3D11Buffer *mirrorBuffer;
ID3D11Buffer *paletteBuffer;
ID3D11Buffer *paletteSelectionBuffer;
ID3D11Buffer *indexBuffer;
ID3D11Buffer *overlayColorBuffer;
//MatrixBuffer *worldMatrixPtr;
//MatrixBuffer *viewMatrixPtr;
//MatrixBuffer *projectionMatrixPtr;
ID3D11SamplerState* sampleState;
ID3D11Texture2D* texture2d;
ID3D11ShaderResourceView* textureView;
ShaderType activeShader;
D3D11_SUBRESOURCE_DATA subData;
MirrorState mirrorState;
D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
ID3D11DepthStencilState* m_depthDisabledStencilState;
ID3D11DepthStencilState* m_depthStencilState;
ID3D11DepthStencilView* m_depthStencilView;
ID3D11RasterizerState* fillRasterState;
ID3D11RasterizerState* wireframeRasterState;
int selectedPalette;
int mirrorBufferNumber;
int paletteBufferNumber;
int paletteSelectionBufferNumber;

D3D11_VIEWPORT N3s3d::viewport;
PPUHueStandardCollection N3s3d::ppuHueStandardCollection;

void N3s3d::initPipeline(N3sD3dContext c)
{
	// Get device/context handles
	device = c.device;
	context = c.context;
	device1 = c.device1;
	context1 = c.context1;

	// Init D3D pipeline
	initShaders();
	initShaderExtras();
	setShader(color);
	initDepthStencils();
	setDepthBufferState(true);
	initSampleState();
	initRasterDesc();

	// select which primtive type we are using
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set mirroring constant buffer to be false
	mirrorState = { 0, 0 };
	updateMirroring(false, false);

	// Fill palette with null data TODO: is this necessary still?
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
	setOverlayColor(100, 100, 100, 64);
}

void N3s3d::initShaders() {
	// Create view buffer descriptions and give to each shader
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(XMMATRIX);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	for (int i = 0; i < shaderCount; i++)
	{
		device1->CreateBuffer(&matrixBufferDesc, NULL, &shaders[i].matrices.worldMatrixBuffer);
		device1->CreateBuffer(&matrixBufferDesc, NULL, &shaders[i].matrices.viewMatrixBuffer);
		device1->CreateBuffer(&matrixBufferDesc, NULL, &shaders[i].matrices.projectionMatrixBuffer);
	}
	ifstream s_stream;
	size_t s_size;
	char* s_data;

	// Init palette shaders
	s_stream.open(L"color_vertex.cso", ifstream::in | ifstream::binary);
	s_stream.seekg(0, ios::end);
	s_size = size_t(s_stream.tellg());
	s_data = new char[s_size];
	s_stream.seekg(0, ios::beg);
	s_stream.read(&s_data[0], s_size);
	s_stream.close();

	device1->CreateVertexShader(s_data, s_size, 0, &shaders[0].vertexShader);

	D3D11_INPUT_ELEMENT_DESC colorLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	device1->CreateInputLayout(colorLayout, 2, &s_data[color], s_size, &shaders[color].inputLayout);

	s_stream.open(L"color_pixel.cso", ifstream::in | ifstream::binary);
	s_stream.seekg(0, ios::end);
	s_size = size_t(s_stream.tellg());
	s_data = new char[s_size];
	s_stream.seekg(0, ios::beg);
	s_stream.read(&s_data[0], s_size);
	s_stream.close();

	device1->CreatePixelShader(s_data, s_size, 0, &shaders[color].pixelShader);

	// Init overlay shaders
	s_stream.open(L"overlay_vertex.cso", ifstream::in | ifstream::binary);
	s_stream.seekg(0, ios::end);
	s_size = size_t(s_stream.tellg());
	s_data = new char[s_size];
	s_stream.seekg(0, ios::beg);
	s_stream.read(&s_data[0], s_size);
	s_stream.close();

	device1->CreateVertexShader(s_data, s_size, 0, &shaders[overlay].vertexShader);

	D3D11_INPUT_ELEMENT_DESC overlayLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	device1->CreateInputLayout(overlayLayout, 1, &s_data[0], s_size, &shaders[overlay].inputLayout);

	s_stream.open(L"overlay_pixel.cso", ifstream::in | ifstream::binary);
	s_stream.seekg(0, ios::end);
	s_size = size_t(s_stream.tellg());
	s_data = new char[s_size];
	s_stream.seekg(0, ios::beg);
	s_stream.read(&s_data[0], s_size);
	s_stream.close();

	device1->CreatePixelShader(s_data, s_size, 0, &shaders[overlay].pixelShader);
}

void N3s3d::initShaderExtras()
{
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

	// Create overlay color selection buffer desc
	D3D11_BUFFER_DESC overlayColorBufferDesc;
	overlayColorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	overlayColorBufferDesc.ByteWidth = sizeof(float) * 4;
	overlayColorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	overlayColorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	overlayColorBufferDesc.MiscFlags = 0;
	overlayColorBufferDesc.StructureByteStride = 0;

	device1->CreateBuffer(&overlayColorBufferDesc, NULL, &overlayColorBuffer);

	// Set buffer slots
	mirrorBufferNumber = 3;
	paletteBufferNumber = 4;
	paletteSelectionBufferNumber = 5;
}

void N3s3d::setShader(ShaderType type) {
	// Type is enum 0,1,2...
	context->IASetInputLayout(shaders[type].inputLayout);
	context->GSSetShader(NULL, 0, 0);
	context->VSSetShader(shaders[type].vertexShader, 0, 0);
	context->PSSetShader(shaders[type].pixelShader, 0, 0);
	activeShader = type;
}

ID3D11Buffer* N3s3d::createBufferFromColorVertices(std::vector<ColorVertex> * vertices, int arraySize)
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

ID3D11Buffer * N3s3d::createBufferFromOverlayVertices(std::vector<OverlayVertex>* vertices, int arraySize)
{
	if (vertices->size() == 0) {
		return nullptr;
	}
	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;						// write access access by CPU and GPU
	bd.ByteWidth = sizeof(OverlayVertex) * arraySize;	// size is the VERTEX struct * 3in
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;			// use as a vertex buffer
	bd.CPUAccessFlags = 0;								// disallow CPU to write in buffer

	ID3D11Buffer *pVBuffer;								// the vertex buffer
	D3D11_SUBRESOURCE_DATA vData;
	vData.pSysMem = &(vertices->data()[0]);
	device1->CreateBuffer(&bd, &vData, &pVBuffer);		// create the buffer
	return pVBuffer;
}

void N3s3d::updateWorldMatrix(float xPos, float yPos, float zPos) {
	XMMATRIX worldMatrix = XMMatrixIdentity();
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;
	XMVECTOR translation = { xPos, yPos, zPos, 0.0f };
	worldMatrix = XMMatrixTranslationFromVector(translation);
	worldMatrix = XMMatrixTranspose(worldMatrix);
	// Lock the constant buffer so it can be written to.
	context1->Map(shaders[activeShader].matrices.worldMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = worldMatrix;
	// Unlock the constant buffer.
	context1->Unmap(shaders[activeShader].matrices.worldMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(0, 1, &shaders[activeShader].matrices.worldMatrixBuffer);
}

void N3s3d::updateWorldMatrix(float xPos, float yPos, float zPos, float xRot, float yRot, float zRot, float scale) {
	XMMATRIX worldMatrix;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;
	XMVECTOR translation = { xPos, yPos, zPos, 0.0f };
	XMVECTOR rotation = { XMConvertToRadians(xRot),	XMConvertToRadians(yRot), XMConvertToRadians(zRot), 0.0f };
	XMVECTOR scaling = { scale, scale, scale, 0.0f };
	XMMATRIX translationMatrix = XMMatrixTranslationFromVector(translation);
	XMMATRIX rotationMatrix = XMMatrixRotationRollPitchYawFromVector(rotation);
	XMMATRIX scaleMatrix = XMMatrixScalingFromVector(scaling);
	XMMATRIX temp = DirectX::XMMatrixMultiply(scaleMatrix, rotationMatrix);
	temp = DirectX::XMMatrixMultiply(temp, translationMatrix);
	worldMatrix = XMMatrixTranspose(temp);
	// Lock the constant buffer so it can be written to.
	context1->Map(shaders[activeShader].matrices.worldMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = worldMatrix;
	// Unlock the constant buffer.
	context1->Unmap(shaders[activeShader].matrices.worldMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(0, 1, &shaders[activeShader].matrices.worldMatrixBuffer);
}

void N3s3d::updateMirroring(bool horizontal, bool vertical) {
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

void N3s3d::updatePalette(float palette[72])
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

void N3s3d::selectPalette(int palette)
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

void N3s3d::setOverlayColor(int r, int g, int b, int a)
{
	// Make sure the overlay shader is active
	if (activeShader != overlay)
		setShader(overlay);
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	float* dataPtr;
	// Lock the constant buffer so it can be written to.
	context1->Map(overlayColorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (float*)mappedResource.pData;
	// Copy the values into the constant buffer.
	*dataPtr = ((float)r / 256);
	dataPtr++;
	*dataPtr = ((float)g / 256);
	dataPtr++;
	*dataPtr = ((float)b / 256);
	dataPtr++;
	*dataPtr = ((float)a / 256);
	// Unlock the constant buffer.
	context1->Unmap(overlayColorBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated value.
	context1->PSSetConstantBuffers(0, 1, &overlayColorBuffer);
}

void N3s3d::updateMatricesWithCamera(Camera * camera) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;

	//worldMatrix = XMMatrixIdentity();
	projectionMatrix = getProjectionMatrix(0.1f, 1000.0f, 1, 1); // TODO use screen resolution
	viewMatrix = camera->GetViewMatrix();

	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	//// Lock the constant buffer so it can be written to.
	//context1->Map(shaders[activeShader].matrices.worldMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	//// Get a pointer to the data in the constant buffer.
	//dataPtr = (MatrixBuffer*)mappedResource.pData;
	//// Copy the matrices into the constant buffer.
	//dataPtr->matrix = worldMatrix;
	//// Unlock the constant buffer.
	//context1->Unmap(shaders[activeShader].matrices.worldMatrixBuffer, 0);
	//// Finally set the constant buffer in the vertex shader with the updated values.
	//context1->VSSetConstantBuffers(0, 1, &shaders[activeShader].matrices.worldMatrixBuffer);

	// Lock the constant buffer so it can be written to.
	context1->Map(shaders[activeShader].matrices.viewMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = viewMatrix;
	// Unlock the constant buffer.
	context1->Unmap(shaders[activeShader].matrices.viewMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(1, 1, &shaders[activeShader].matrices.viewMatrixBuffer);

	// Lock the constant buffer so it can be written to.
	context1->Map(shaders[activeShader].matrices.projectionMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = projectionMatrix;
	// Unlock the constant buffer.
	context1->Unmap(shaders[activeShader].matrices.projectionMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(2, 1, &shaders[activeShader].matrices.projectionMatrixBuffer);
}

void N3s3d::updateViewMatrices(XMFLOAT4X4 view, XMFLOAT4X4 projection)
{
	XMMATRIX viewMatrix, projectionMatrix;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;

	viewMatrix = XMLoadFloat4x4(&view);
	projectionMatrix = XMLoadFloat4x4(&projection);

	// Lock the constant buffer so it can be written to.
	context1->Map(shaders[activeShader].matrices.viewMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = viewMatrix;
	// Unlock the constant buffer.
	context1->Unmap(shaders[activeShader].matrices.viewMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(1, 1, &shaders[activeShader].matrices.viewMatrixBuffer);

	// Lock the constant buffer so it can be written to.
	context1->Map(shaders[activeShader].matrices.projectionMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = projectionMatrix;
	// Unlock the constant buffer.
	context1->Unmap(shaders[activeShader].matrices.projectionMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(2, 1, &shaders[activeShader].matrices.projectionMatrixBuffer);
}

XMMATRIX N3s3d::getProjectionMatrix(const float near_plane, const float far_plane, const float fov_horiz, const float fov_vert)
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

void N3s3d::setIndexBuffer()
{
	context->IASetIndexBuffer(
		indexBuffer,
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);
}

void N3s3d::setDepthBufferState(bool active)
{
	if(active)
		context->OMSetDepthStencilState(m_depthStencilState, 1);
	else
		context->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
}

void N3s3d::setRasterFillState(bool fill)
{
	if(fill)
		context->RSSetState(fillRasterState);
	else
		context->RSSetState(wireframeRasterState);
}

void N3s3d::setGuiProjection()
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;

	// Create projection matrix based on screen size
	XMMATRIX projectionMatrix = XMMatrixOrthographicLH(viewport.Width, viewport.Height, 0, 1000);

	// Create view matrix looking at origin
	XMMATRIX viewMatrix = XMMatrixLookAtLH({ 0, 0, -2, 0 }, { 0, 0, 0, 0 }, { 0, 5, 0, 0 });

	// Lock the constant buffer so it can be written to.
	context1->Map(shaders[activeShader].matrices.viewMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = viewMatrix;
	// Unlock the constant buffer.
	context1->Unmap(shaders[activeShader].matrices.viewMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(1, 1, &shaders[activeShader].matrices.viewMatrixBuffer);
	
	// Lock the constant buffer so it can be written to.
	context1->Map(shaders[activeShader].matrices.projectionMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBuffer*)mappedResource.pData;
	// Copy the matrices into the constant buffer.
	dataPtr->matrix = projectionMatrix;
	// Unlock the constant buffer.
	context1->Unmap(shaders[activeShader].matrices.projectionMatrixBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(2, 1, &shaders[activeShader].matrices.projectionMatrixBuffer);
}

void N3s3d::updateViewport(D3D11_VIEWPORT v)
{
	viewport = v;
}

void N3s3d::renderMesh(VoxelMesh *voxelMesh) {
	ShaderType type = voxelMesh->type;
	if (type != activeShader)
	{
		setShader(type);
		activeShader = type;
	}
	UINT stride;
	if(activeShader == color)
		stride = sizeof(ColorVertex); // TODO optimize?
	if (activeShader == overlay)
		stride = sizeof(OverlayVertex);
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

void N3s3d::initSampleState() {
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

void N3s3d::initRasterDesc()
{
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

	D3D11_RASTERIZER_DESC wireRasterDesc;
	wireRasterDesc.AntialiasedLineEnable = false;
	wireRasterDesc.CullMode = D3D11_CULL_NONE;
	wireRasterDesc.DepthBias = 0;
	wireRasterDesc.DepthBiasClamp = 0.0f;
	wireRasterDesc.DepthClipEnable = true;
	wireRasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireRasterDesc.FrontCounterClockwise = true;
	wireRasterDesc.MultisampleEnable = false;
	wireRasterDesc.ScissorEnable = false;
	wireRasterDesc.SlopeScaledDepthBias = 0.0f;

	device->CreateRasterizerState(&rasterDesc, &fillRasterState);
	device->CreateRasterizerState(&wireRasterDesc, &wireframeRasterState);
	context->RSSetState(fillRasterState);
}

void N3s3d::createIndexBuffer()
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

bool N3s3d::initDepthStencils()
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

	// Enable alpha blending in output merger
	ID3D11BlendState1* g_pBlendStateNoBlend = NULL;

	D3D11_BLEND_DESC1 BlendState;
	ZeroMemory(&BlendState, sizeof(D3D11_BLEND_DESC1));
	//BlendState.AlphaToCoverageEnable = false;
	BlendState.IndependentBlendEnable = true;
	BlendState.RenderTarget[0].BlendEnable = true;
	BlendState.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendState.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendState.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendState.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendState.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	device1->CreateBlendState1(&BlendState, &g_pBlendStateNoBlend);

	float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	UINT sampleMask = 0xffffffff;

	context->OMSetBlendState(g_pBlendStateNoBlend, blendFactor, sampleMask);
	
	// If it all succeeded, return true
	return true;
}

bool Crop::zeroed()
{
	if (top + left + right + bottom == 0)
		return true;
	else
		return false;
}
