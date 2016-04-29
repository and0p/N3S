// Obviously thanks to http://www.directxtutorial.com/ for a bunch of this
// As well as http://www.braynzarsoft.net/viewtutorial/q16390-4-begin-drawing

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "VoxelUtil.h"

using namespace std;

Microsoft::WRL::ComPtr<ID3D11Device>            VoxelUtil::device;
Microsoft::WRL::ComPtr<ID3D11Device1>           VoxelUtil::device1;
Microsoft::WRL::ComPtr<ID3D11DeviceContext>     VoxelUtil::context;
Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    VoxelUtil::context1;
ID3D11SamplerState *VoxelUtil::sampleState;
ID3D11Buffer *VoxelUtil::m_matrixBuffer;
MatrixBufferType *VoxelUtil::dataPtr;
ShaderSet VoxelUtil::shaderSets[2];
ID3D11InputLayout *VoxelUtil::inputLayouts[2];
ID3D11Texture2D *VoxelUtil::texture2d;
ID3D11ShaderResourceView *VoxelUtil::textureView;

void VoxelUtil::initPipeline(Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dContext,
						   Microsoft::WRL::ComPtr<ID3D11Device1> d3dDevice1, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dContext1)
{
	device = d3dDevice;
	context = d3dContext;
	device1 = d3dDevice1;
	context1 = d3dContext1;
	// Create view cbuffer desc
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	device1->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);

	initShaders();
	setShader(color);
	initSampleState();

	// select which primtive type we are using
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	context->IASetInputLayout(inputLayouts[0]);

	// Create texture description
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = 256;
	desc.Height = 256;
	desc.MipLevels = desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.SampleDesc.Quality = 0;

	device->CreateTexture2D(&desc, NULL, &texture2d);
}

void VoxelUtil::initShaders() {
	ifstream s_stream;
	size_t s_size;
	char* s_data;
	s_stream.open("C:\\Users\\and0\\Source\\Repos\\NesVoxel\\Debug\\color_vertex.cso", ifstream::in | ifstream::binary);
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
		{ "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	device1->CreateInputLayout(colorLayout, 2, &s_data[0], s_size, &inputLayouts[0]);

	s_stream.open("C:\\Users\\and0\\Source\\Repos\\NesVoxel\\Debug\\color_pixel.cso", ifstream::in | ifstream::binary);
	s_stream.seekg(0, ios::end);
	s_size = size_t(s_stream.tellg());
	s_data = new char[s_size];
	s_stream.seekg(0, ios::beg);
	s_stream.read(&s_data[0], s_size);
	s_stream.close();

	device1->CreatePixelShader(s_data, s_size, 0, &shaderSets[0].pixelShader);

	s_stream.open("C:\\Users\\and0\\Source\\Repos\\NesVoxel\\Debug\\texture_vertex.cso", ifstream::in | ifstream::binary);
	s_stream.seekg(0, ios::end);
	s_size = size_t(s_stream.tellg());
	s_data = new char[s_size];
	s_stream.seekg(0, ios::beg);
	s_stream.read(&s_data[0], s_size);
	s_stream.close();

	device1->CreateVertexShader(s_data, s_size, 0, &shaderSets[1].vertexShader);

	D3D11_INPUT_ELEMENT_DESC textureLayout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXTURE",    0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	device1->CreateInputLayout(textureLayout, 2, &s_data[0], s_size, &inputLayouts[1]);

	s_stream.open("C:\\Users\\and0\\Source\\Repos\\NesVoxel\\Debug\\texture_pixel.cso", ifstream::in | ifstream::binary);
	s_stream.seekg(0, ios::end);
	s_size = size_t(s_stream.tellg());
	s_data = new char[s_size];
	s_stream.seekg(0, ios::beg);
	s_stream.read(&s_data[0], s_size);
	s_stream.close();

	device1->CreatePixelShader(s_data, s_size, 0, &shaderSets[1].pixelShader);
}

void VoxelUtil::setShader(ShaderType type) {
	switch (type)
	{
	case color:
		context->GSSetShader(NULL, 0, 0);
		context->VSSetShader(shaderSets[0].vertexShader, 0, 0);
		context->PSSetShader(shaderSets[0].pixelShader, 0, 0);
		break;
	case texture:
		context->GSSetShader(NULL, 0, 0);
		context->VSSetShader(shaderSets[1].vertexShader, 0, 0);
		context->PSSetShader(shaderSets[1].pixelShader, 0, 0);
		break;
	}
}

VoxelMesh* VoxelUtil::CreateRectangle()
{
	VoxelMesh *rectangle;
	rectangle = new VoxelMesh;

	ColorVertex *vertices = new ColorVertex[6]
	{
		{ XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
		{ XMFLOAT4(1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }
	};

	rectangle->buffer = createBufferFromColorVertices(vertices, 192);
	rectangle->size = 6;
	rectangle->type = color;

	return rectangle;
}

ID3D11Buffer* VoxelUtil::createBufferFromColorVertices(ColorVertex vertices[], int arraySize)
{
	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(ColorVertex) * arraySize;             // size is the VERTEX struct * 3in
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

	ID3D11Buffer *pVBuffer;								// the vertex buffer
	device1->CreateBuffer(&bd, NULL, &pVBuffer);		// create the buffer

	// copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT result = context1->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);    // map the buffer
	memcpy(ms.pData, vertices, arraySize);							// copy the data
	context1->Unmap(pVBuffer, NULL);                                         // unmap the buffer
	return pVBuffer;
}

void VoxelUtil::updateMatricesWithCamera(VoxelCamera * camera) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	unsigned int bufferNumber;

	worldMatrix = XMMatrixIdentity();
	projectionMatrix = getProjectionMatrix(0.1f, 1000.0f, 1, 1);
	camera->GetViewMatrix(viewMatrix);

	// Transpose the matrices to prepare them for the shader.
	worldMatrix = XMMatrixTranspose(worldMatrix);
	viewMatrix = XMMatrixTranspose(viewMatrix);
	projectionMatrix = XMMatrixTranspose(projectionMatrix);

	// Lock the constant buffer so it can be written to.
	context1->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);

	// Get a pointer to the data in the constant buffer.
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	// Copy the matrices into the constant buffer.
	dataPtr->world = worldMatrix;
	dataPtr->view = viewMatrix;
	dataPtr->projection = projectionMatrix;

	// Unlock the constant buffer.
	context1->Unmap(m_matrixBuffer, 0);

	// Set the position of the constant buffer in the vertex shader.
	bufferNumber = 0;

	// Finally set the constant buffer in the vertex shader with the updated values.
	context1->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);
}

XMMATRIX VoxelUtil::getProjectionMatrix(const float near_plane, const float far_plane, const float fov_horiz, const float fov_vert)
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

void VoxelUtil::renderMesh(VoxelMesh *voxelMesh) {
	// Switch based on type
	if (voxelMesh->type == color) {
		UINT stride = sizeof(ColorVertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &voxelMesh->buffer, &stride, &offset);
		// draw the vertex buffer to the back buffer
		context->Draw(voxelMesh->size, 0);
	}
}

void VoxelUtil::initSampleState() {
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