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
ID3D11Buffer *mirrorBuffer;
ID3D11Buffer *paletteBuffer;
ID3D11Buffer *paletteSelectionBuffer;
ID3D11Buffer *indexBuffer;
ID3D11Buffer *indexBufferReversed;
ID3D11Buffer *outlineMirrorBuffer;
ID3D11Buffer *outlineColorBuffer;
ID3D11Buffer *overlayColorBuffer;
ID3D11Buffer *cameraPosBuffer;
ID3D11SamplerState* sampleState;
ID3D11Texture2D* texture2d;
ID3D11ShaderResourceView* textureView;
ShaderType activeShader;
D3D11_SUBRESOURCE_DATA subData;
MirrorState mirrorState;
D3D11_DEPTH_STENCIL_DESC depthStencilNoWriteDesc;
D3D11_DEPTH_STENCIL_DESC depthStencilWriteDesc;
D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
D3D11_DEPTH_STENCIL_DESC depthDisabledStencilOnlyDesc;
ID3D11DepthStencilState* m_depthStencilNoWriteState;
ID3D11DepthStencilState* m_depthStencilWriteState;
ID3D11DepthStencilState* m_depthDisabledStencilState;
ID3D11DepthStencilState* m_depthDisabledStencilOnlyState;
ID3D11DepthStencilState* m_depthStencilMaskRefState;
ID3D11DepthStencilView* m_depthStencilView;
ID3D11RasterizerState* fillRasterState;
ID3D11RasterizerState* reverseRasterState;
ID3D11RasterizerState* wireframeRasterState;
int selectedPalette;
int mirrorBufferNumber;
int paletteBufferNumber;
int paletteSelectionBufferNumber;
int cameraPosBufferNumber;

int stencilReferenceNumber;

D3D11_VIEWPORT N3s3d::viewport;

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
	createIndexBuffers();

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
	updatePalette(paletteArray, { 0.0f, 0.0f, 0.0f });
	setOverlayColor(100, 100, 100, 64);

	stencilReferenceNumber = 1;
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
		{ "TEXCOORD",    0, DXGI_FORMAT_R32G32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR",    0, DXGI_FORMAT_R32_UINT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	device1->CreateInputLayout(colorLayout, 3, &s_data[color], s_size, &shaders[color].inputLayout);

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

	// Init outline shaders
	s_stream.open(L"outline_vertex.cso", ifstream::in | ifstream::binary);
	s_stream.seekg(0, ios::end);
	s_size = size_t(s_stream.tellg());
	s_data = new char[s_size];
	s_stream.seekg(0, ios::beg);
	s_stream.read(&s_data[0], s_size);
	s_stream.close();

	device1->CreateVertexShader(s_data, s_size, 0, &shaders[outline].vertexShader);

	D3D11_INPUT_ELEMENT_DESC outlineLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	device1->CreateInputLayout(outlineLayout, 1, &s_data[0], s_size, &shaders[outline].inputLayout);

	s_stream.open(L"outline_pixel.cso", ifstream::in | ifstream::binary);
	s_stream.seekg(0, ios::end);
	s_size = size_t(s_stream.tellg());
	s_data = new char[s_size];
	s_stream.seekg(0, ios::beg);
	s_stream.read(&s_data[0], s_size);
	s_stream.close();

	device1->CreatePixelShader(s_data, s_size, 0, &shaders[outline].pixelShader);
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
	paletteBufferDesc.ByteWidth = sizeof(float) * 128;
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

	// Create camera position buffer
	D3D11_BUFFER_DESC cameraPosBufferDesc;
	cameraPosBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraPosBufferDesc.ByteWidth = sizeof(float) * 4;
	cameraPosBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraPosBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cameraPosBufferDesc.MiscFlags = 0;
	cameraPosBufferDesc.StructureByteStride = 0;

	device1->CreateBuffer(&cameraPosBufferDesc, NULL, &cameraPosBuffer);

	// Create mirror buffer desc for outline shader
	D3D11_BUFFER_DESC overlayMirrorBufferDesc;
	overlayMirrorBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	overlayMirrorBufferDesc.ByteWidth = 16;
	overlayMirrorBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	overlayMirrorBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	overlayMirrorBufferDesc.MiscFlags = 0;
	overlayMirrorBufferDesc.StructureByteStride = 0;

	device1->CreateBuffer(&overlayMirrorBufferDesc, NULL, &mirrorBuffer);

	// Set buffer slots
	mirrorBufferNumber = 3;
	paletteBufferNumber = 4;
	paletteSelectionBufferNumber = 5;
	cameraPosBufferNumber = 6;

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

void N3s3d::updateWorldMatrix(float xPos, float yPos, float zPos, float xRot, float yRot, float zRot, float xScale, float yScale, float zScale)
{
	XMMATRIX worldMatrix;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;
	XMVECTOR translation = { xPos, yPos, zPos, 0.0f };
	XMVECTOR rotation = { XMConvertToRadians(xRot),	XMConvertToRadians(yRot), XMConvertToRadians(zRot), 0.0f };
	XMVECTOR scaling = { xScale, yScale, zScale, 0.0f };
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

		// Also, set index for rendering voxel meshes based on mirroring
		if (horizontal == vertical)
			context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
		else
			context->IASetIndexBuffer(indexBufferReversed, DXGI_FORMAT_R16_UINT, 0);
	}
}

void N3s3d::updatePalette(float palette[72], Hue bg)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	float* dataPtr;
	// Lock the constant buffer so it can be written to.
	context1->Map(paletteBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (float*)mappedResource.pData;
	// Copy the values into the constant buffer.
	for (int p = 0; p < 8; p++) // Each set of 3 colors
	{
		for (int c = 0; c < 3; c++)	// Each individual color
		{
			float* colorPtr = dataPtr + (p * 16 + c * 4);
			int colorIndex = (p * 9 + c * 3);
			for (int rgb = 0; rgb < 3; rgb++) // Each individual RGB float
			{
				*(colorPtr + rgb) = palette[colorIndex + rgb];
			}
			// Add alpha channel, always 1.0f
			*(colorPtr + 4) = 1.0f;
		}
		// Also add the background color as a "fourth" for every subpalette
		float* bgPtr = dataPtr + (p * 16) + 12;
		*(bgPtr) = bg.red;
		*(bgPtr + 1) = bg.green;
		*(bgPtr + 2) = bg.blue;
		*(bgPtr + 3) = 1.0f;
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

void N3s3d::setCameraPos(float x, float y, float z)
{
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	float* dataPtr;
	// Lock the constant buffer so it can be written to.
	context1->Map(cameraPosBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	// Get a pointer to the data in the constant buffer.
	dataPtr = (float*)mappedResource.pData;
	// Copy the values into the constant buffer.
	*dataPtr = x;
	*(dataPtr + 1) = y;
	*(dataPtr + 2) = z;
	*(dataPtr + 3) = 0.0f;
	// Unlock the constant buffer.
	context1->Unmap(cameraPosBuffer, 0);
	// Finally set the constant buffer in the pixel shader with the updated values.
	context1->VSSetConstantBuffers(cameraPosBufferNumber, 1, &cameraPosBuffer);
}

void N3s3d::setOverlayColor(float r, float g, float b, float a)
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
	*dataPtr = r;
	dataPtr++;
	*dataPtr = g;
	dataPtr++;
	*dataPtr = b;
	dataPtr++;
	*dataPtr = a;
	// Unlock the constant buffer.
	context1->Unmap(overlayColorBuffer, 0);
	// Finally set the constant buffer in the vertex shader with the updated value.
	context1->PSSetConstantBuffers(0, 1, &overlayColorBuffer);
}

void N3s3d::setOverlayColor(int r, int g, int b, int a)
{
	setOverlayColor((float)r / 255, (float)g / 255, (float)b / 255, (float)a / 255);
}

void N3s3d::updateMatricesWithCamera(Camera * camera) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;

	//worldMatrix = XMMatrixIdentity();
	projectionMatrix = getProjectionMatrix(0.1f, 30.0f, 1, 1); // TODO use screen resolution
	viewMatrix = camera->GetViewMatrix();

	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

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

	// If we're in the standard palette shader, update camera position constant buffer
	if (activeShader == color)
		setCameraPos(camera->GetPosition().x, camera->GetPosition().y, camera->GetPosition().z);
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
		context->OMSetDepthStencilState(m_depthStencilNoWriteState, 1);
	else
		context->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
}

void N3s3d::setDepthStencilState(bool depthTest, bool stencilWrite, bool stencilTest)
{
	if (depthTest)
	{
		if(stencilWrite)
			context->OMSetDepthStencilState(m_depthStencilWriteState, 1);
		else
			context->OMSetDepthStencilState(m_depthStencilNoWriteState, 1);
	}
	else
	{
		if(stencilTest)
			context->OMSetDepthStencilState(m_depthDisabledStencilOnlyState, 1);
		else
			context->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
	}
}

void N3s3d::setStencilForOutline(bool outline)
{
	if (!outline)	// Writing normal sprite
	{
		incrementStencilReference();	// Increment, since we're now writing a new value
		context->OMSetDepthStencilState(m_depthStencilWriteState, stencilReferenceNumber);
	}
	else			// Writing outline for previous sprite
	{
		context->OMSetDepthStencilState(m_depthStencilMaskRefState, stencilReferenceNumber);
	}
}

void N3s3d::setRasterFillState(bool fill)
{
	if(fill)
		context->RSSetState(fillRasterState);
	else
		context->RSSetState(wireframeRasterState);
}

void N3s3d::incrementStencilReference()
{
	stencilReferenceNumber++;
	if (stencilReferenceNumber > 254)	// assuming 255 reserved
		stencilReferenceNumber = 1;
	// TODO: clear buffer if we ever exceed 254?
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

void N3s3d::clear()
{
	stencilReferenceNumber = 1;
}

void N3s3d::updateViewport(D3D11_VIEWPORT v)
{
	viewport = v;
}

XMVECTOR N3s3d::getMouseVector(Camera * camera, int mouseX, int mouseY)
{
	// Get XMVector for mouse coordinates
	XMFLOAT3 x3 = { (float)mouseX, (float)mouseY, 0.0f };
	XMVECTOR v = XMLoadFloat3(&x3);
	// Get view and projection matrices from camera
	XMMATRIX projectionMatrix = getProjectionMatrix(0.1f, 30.0f, 1, 1); // TODO use screen resolution
	XMMATRIX viewMatrix = camera->GetViewMatrix();
	// Get world matrix from camera origin
	XMMATRIX worldMatrix = XMMatrixIdentity();
	XMFLOAT3 pos = camera->GetPosition();
	XMVECTOR translation = { pos.x, pos.y, pos.z, 0.0f };
	worldMatrix = XMMatrixTranslationFromVector(translation);
	//worldMatrix = XMMatrixTranspose(worldMatrix);
	return XMVector3Unproject(v, 0, 0, viewport.Width, viewport.Height, 0.0f, 1.0f, projectionMatrix, viewMatrix, worldMatrix);
}

XMFLOAT3 N3s3d::getPlaneIntersection(PlaneAxis axis, int pixel, Camera * camera, int mouseX, int mouseY)
{
	// Get mouse vector and store as XMFLOAT3
	XMFLOAT3 mF;
	XMStoreFloat3(&mF, getMouseVector(camera, mouseX, mouseY));
	// Get start and end points from this, using camera origin and distant point along vector
	XMVECTOR origin, distant;
	XMFLOAT3 pos = camera->GetPosition();
	origin = { pos.x, pos.y, pos.z, 0.0f };
	distant = { pos.x + (mF.x * 10), pos.y + (mF.y * 10), pos.z + (mF.z * 10) };
	// Create a plane on the specified axis with 3 vectors
	XMVECTOR c1, c2, c3;
	XMFLOAT3 source;
	float x, y, z;
	switch (axis)
	{
	case x_axis:
		x = -1.0f + (pixelSizeW * pixel) + (pixelSizeW * 0.5f);
		source = { x, 0.0f, 0.0f };
		c1 = XMLoadFloat3(&source);
		source = { x, 0.0f, 1.0f };
		c2 = XMLoadFloat3(&source);
		source = { x, 1.0f, 0.0f };
		c3 = XMLoadFloat3(&source);
		break;
	case y_axis:
		y = 1.0f - (pixelSizeW * pixel) - (pixelSizeW * 0.5f);
		source = { 0.0f, y, 0.0f };
		c1 = XMLoadFloat3(&source);
		source = { 0.0f, y, 1.0f };
		c2 = XMLoadFloat3(&source);
		source = { 1.0f, y, 0.0f };
		c3 = XMLoadFloat3(&source);
		break;
	case z_axis:
		z = 0.0f + (pixelSizeW * pixel) + (pixelSizeW * 0.5f);
		source = { 0.0f, 0.0f, z };
		c1 = XMLoadFloat3(&source);
		source = { 0.0f, 1.0f, z };
		c2 = XMLoadFloat3(&source);
		source = { 1.0f, 0.0f, z };
		c3 = XMLoadFloat3(&source);
		break;
	}
	XMVECTOR plane = XMPlaneFromPoints(c1, c2, c3);
	// Return float3 of where line from mouse/camera intersects the z-axis at 0
	XMFLOAT3 r;
	XMStoreFloat3(&r, XMPlaneIntersectLine(plane, origin, distant));
	return r;
}

Vector3D N3s3d::getPixelCoordsFromFloat3(XMFLOAT3 pos)
{
	Vector3D v = {
		roundDownPosOrNeg((pos.x + 1.0f) / pixelSizeW),
		-1 * roundDownPosOrNeg((pos.y - 1.0f) / pixelSizeW),
		roundDownPosOrNeg((pos.z / pixelSizeW))
	};
	return v;
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
		stride = sizeof(ColorVertex);
	if (activeShader == overlay)
		stride = sizeof(OverlayVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &voxelMesh->buffer, &stride, &offset);
	// TODO don't change buffer if selected is same as before
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
	if (activeShader == color)
	{
		context->DrawIndexed(voxelMesh->size / 4 * 6, 0, 0);
	}
	else
	{
		context->Draw(voxelMesh->size, 0);
	}
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
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	D3D11_RASTERIZER_DESC reverseRasterDesc;
	reverseRasterDesc.AntialiasedLineEnable = false;
	reverseRasterDesc.CullMode = D3D11_CULL_FRONT;
	reverseRasterDesc.DepthBias = 0;
	reverseRasterDesc.DepthBiasClamp = 0.0f;
	reverseRasterDesc.DepthClipEnable = true;
	reverseRasterDesc.FillMode = D3D11_FILL_SOLID;
	reverseRasterDesc.FrontCounterClockwise = false;
	reverseRasterDesc.MultisampleEnable = false;
	reverseRasterDesc.ScissorEnable = false;
	reverseRasterDesc.SlopeScaledDepthBias = 0.0f;

	D3D11_RASTERIZER_DESC wireRasterDesc;
	wireRasterDesc.AntialiasedLineEnable = true;
	wireRasterDesc.CullMode = D3D11_CULL_NONE;
	wireRasterDesc.DepthBias = 0;
	wireRasterDesc.DepthBiasClamp = 0.0f;
	wireRasterDesc.DepthClipEnable = true;
	wireRasterDesc.FillMode = D3D11_FILL_WIREFRAME;
	wireRasterDesc.FrontCounterClockwise = false;
	wireRasterDesc.MultisampleEnable = false;
	wireRasterDesc.ScissorEnable = false;
	wireRasterDesc.SlopeScaledDepthBias = 0.0f;

	device->CreateRasterizerState(&rasterDesc, &fillRasterState); 
	device->CreateRasterizerState(&reverseRasterDesc, &reverseRasterState);
	device->CreateRasterizerState(&wireRasterDesc, &wireframeRasterState);
	context->RSSetState(fillRasterState);
}

void N3s3d::createIndexBuffers()
{
	unsigned short indices[36864];
	unsigned short indicesReversed[36864];
	// For each hypothetical face
	for (int i = 0; i < 6144; i++)
	{
		int faceIndex = i * 6;
		int vertexIndex = i * 4;
		// Do normal indices
		indices[faceIndex] = vertexIndex;
		indices[faceIndex + 1] = vertexIndex + 1;
		indices[faceIndex + 2] = vertexIndex + 2;
		indices[faceIndex + 3] = vertexIndex;
		indices[faceIndex + 4] = vertexIndex + 2;
		indices[faceIndex + 5] = vertexIndex + 3;
		// Do reverse indices
		indicesReversed[faceIndex] = vertexIndex;
		indicesReversed[faceIndex + 1] = vertexIndex + 2;
		indicesReversed[faceIndex + 2] = vertexIndex + 1;
		indicesReversed[faceIndex + 3] = vertexIndex;
		indicesReversed[faceIndex + 4] = vertexIndex + 3;
		indicesReversed[faceIndex + 5] = vertexIndex + 2;
	}

	// Create buffer description, same for normal and reverse
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;						// write access access by CPU and GPU
	bd.ByteWidth = sizeof(unsigned short) * 36864;		// size is the int * indices
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;				// use as a index buffer
	bd.CPUAccessFlags = 0;								// disallow CPU to write in buffer

	D3D11_SUBRESOURCE_DATA vData;

	// Create normal and reverse buffers
	vData.pSysMem = &(indices[0]);
	device1->CreateBuffer(&bd, &vData, &indexBuffer);
	vData.pSysMem = &(indicesReversed[0]);
	device1->CreateBuffer(&bd, &vData, &indexBufferReversed);
}

bool N3s3d::initDepthStencils()
{
	// Thanks http://www.rastertek.com/dx11tut11.html
	HRESULT result = false;

#pragma region Standard depth stencil, dont write stencil

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilNoWriteDesc, sizeof(depthStencilNoWriteDesc));

	// Set up the description of the stencil state.
	depthStencilNoWriteDesc.DepthEnable = true;
	depthStencilNoWriteDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilNoWriteDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilNoWriteDesc.StencilEnable = false;

	// Create the depth stencil state.
	result = device->CreateDepthStencilState(&depthStencilNoWriteDesc, &m_depthStencilNoWriteState);
	if (FAILED(result))
	{
		return false;
	}

#pragma endregion

#pragma region Standard depth, write stencil

	// Initialize the description of the stencil state.
	ZeroMemory(&depthStencilWriteDesc, sizeof(depthStencilWriteDesc));

	// Set up the description of the stencil state.
	depthStencilWriteDesc.DepthEnable = true;
	depthStencilWriteDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilWriteDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilWriteDesc.StencilEnable = true;
	depthStencilWriteDesc.StencilReadMask = 0xFF;
	depthStencilWriteDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthStencilWriteDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilWriteDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilWriteDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilWriteDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing.
	depthStencilWriteDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilWriteDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilWriteDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
	depthStencilWriteDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	result = device->CreateDepthStencilState(&depthStencilWriteDesc, &m_depthStencilWriteState);
	if (FAILED(result))
	{
		return false;
	}

#pragma endregion

#pragma region Standard depth, mask equal stencil value

	// Initialize the description of the stencil state.
	D3D11_DEPTH_STENCIL_DESC maskStencilDesc;
	ZeroMemory(&depthStencilWriteDesc, sizeof(depthStencilWriteDesc));

	// Set up the description of the stencil state.
	maskStencilDesc.DepthEnable = true;
	maskStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	maskStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	maskStencilDesc.StencilEnable = true;
	maskStencilDesc.StencilReadMask = 0xFF;
	maskStencilDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	maskStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	maskStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	maskStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	maskStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;

	// Stencil operations if pixel is back-facing.
	maskStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	maskStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	maskStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	maskStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;

	// Create the depth stencil state.
	result = device->CreateDepthStencilState(&maskStencilDesc, &m_depthStencilMaskRefState);
	if (FAILED(result))
	{
		return false;
	}

#pragma endregion
	
#pragma region No z-buffer depth stencil

	// Clear the second depth stencil state before setting the parameters.
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

	// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
	// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthDisabledStencilDesc.StencilEnable = false;

	// Create the state using the device.
	result = device->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);
	if (FAILED(result))
	{
		return false;
	}

#pragma endregion

#pragma region No z-buffer stencil only

	// Clear the second depth stencil state before setting the parameters.
	ZeroMemory(&depthDisabledStencilOnlyDesc, sizeof(depthDisabledStencilOnlyDesc));

	// Set up the description of the stencil state.
	depthDisabledStencilOnlyDesc.DepthEnable = false;

	depthDisabledStencilOnlyDesc.StencilEnable = true;
	depthDisabledStencilOnlyDesc.StencilReadMask = 0xFF;
	depthDisabledStencilOnlyDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing.
	depthDisabledStencilOnlyDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilOnlyDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilOnlyDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilOnlyDesc.FrontFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	// Stencil operations if pixel is back-facing.
	depthDisabledStencilOnlyDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilOnlyDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilOnlyDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilOnlyDesc.BackFace.StencilFunc = D3D11_COMPARISON_EQUAL;

	// Create the state using the device.
	result = device->CreateDepthStencilState(&depthDisabledStencilOnlyDesc, &m_depthDisabledStencilOnlyState);
	if (FAILED(result))
	{
		return false;
	}

#pragma endregion

#pragma region Z-buffer enabled
	// Set the z-buffer enabled depth stencil state
	context->OMSetDepthStencilState(m_depthStencilNoWriteState, 1);

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

#pragma endregion

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

void VoxelMesh::releaseBuffers()
{
		buffer->Release();
}