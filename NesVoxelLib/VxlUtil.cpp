// Obviously thanks to http://www.directxtutorial.com/ for a bunch of this
// As well as http://www.braynzarsoft.net/viewtutorial/q16390-4-begin-drawing

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include "VxlUtil.h"

using namespace std;
Microsoft::WRL::ComPtr<ID3D11Device>            VxlUtil::device;
Microsoft::WRL::ComPtr<ID3D11Device1>           VxlUtil::device1;
Microsoft::WRL::ComPtr<ID3D11DeviceContext>     VxlUtil::context;
Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    VxlUtil::context1;
ID3D11SamplerState *VxlUtil::sampleState;
ID3D11Buffer *VxlUtil::worldMatrixBuffer;
ID3D11Buffer *VxlUtil::viewMatrixBuffer;
ID3D11Buffer *VxlUtil::projectionMatrixBuffer;
ID3D11Buffer *VxlUtil::mirrorBuffer;
ID3D11Buffer *VxlUtil::paletteBuffer;
ID3D11Buffer *VxlUtil::paletteSelectionBuffer;
ID3D11Buffer *VxlUtil::indexBuffer;
int VxlUtil::paletteBufferNumber;
int VxlUtil::paletteSelectionBufferNumber;
ShaderSet VxlUtil::shaderSets[2];
ID3D11InputLayout *VxlUtil::inputLayouts[2];
ID3D11Texture2D *VxlUtil::texture2d;
ID3D11ShaderResourceView *VxlUtil::textureView;
ShaderType VxlUtil::activeShader;
D3D11_SUBRESOURCE_DATA VxlUtil::subData;
MirrorState VxlUtil::mirrorState;
PPUHueStandardCollection VxlUtil::ppuHueStandardCollection;
int VxlUtil::selectedPalette;
int VxlUtil::mirrorBufferNumber;

void VxlUtil::initPipeline(VxlD3DContext c)
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

void VxlUtil::initShaders() {
	ifstream s_stream;
	size_t s_size;
	char* s_data;
	s_stream.open("color_vertex.cso", ifstream::in | ifstream::binary);
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

	device1->CreateInputLayout(colorLayout, 3, &s_data[0], s_size, &inputLayouts[0]);

	s_stream.open("color_pixel.cso", ifstream::in | ifstream::binary);
	s_stream.seekg(0, ios::end);
	s_size = size_t(s_stream.tellg());
	s_data = new char[s_size];
	s_stream.seekg(0, ios::beg);
	s_stream.read(&s_data[0], s_size);
	s_stream.close();

	device1->CreatePixelShader(s_data, s_size, 0, &shaderSets[0].pixelShader);
}

void VxlUtil::setShader(ShaderType type) {
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

VoxelMesh* VxlUtil::CreateRectangle(ShaderType type)
{
	VoxelMesh *rectangle;
	rectangle = new VoxelMesh;

	if (type == color)
	{
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
		rectangle->type = color;
		
	}
	else if (type == texture)
	{
		TextureVertex *vertices = new TextureVertex[6]
		{
			{ XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
			{ XMFLOAT4(-1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT4(-1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
			{ XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT4(1.0f, -1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) }
		};
		rectangle->buffer = createBufferFromTextureVertices(vertices, 144);
		rectangle->type = texture;
	}

	rectangle->size = 6;
	return rectangle;
}

VoxelMesh *VxlUtil::CreateSpriteMarker() {
	VoxelMesh *marker;
	marker = new VoxelMesh;
	marker->size = 3;
	marker->type = color;
	ColorVertex *vertices = new ColorVertex[3]
	{
		{ XMFLOAT4(0.0f, (pixelSizeH * -8), 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4((pixelSizeW * 4), 0.0f, 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4((pixelSizeW * 8), (pixelSizeH * -8), 0.0f, 1.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }
	};
	marker->buffer = createBufferFromColorVertices(vertices, 96);
	return marker;
}

ID3D11Buffer* VxlUtil::createBufferFromColorVertices(ColorVertex vertices[], int arraySize)
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

ID3D11Buffer* VxlUtil::createBufferFromColorVerticesV(std::vector<ColorVertex>  * vertices, int arraySize)
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

ID3D11Buffer* VxlUtil::createBufferFromTextureVertices(TextureVertex vertices[], int arraySize)
{
	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DEFAULT;                // write access access by CPU and GPU
	bd.ByteWidth = sizeof(TextureVertex) * arraySize;             // size is the VERTEX struct * 3in
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;       // use as a vertex buffer
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;    // allow CPU to write in buffer

	ID3D11Buffer *pVBuffer;								// the vertex buffer
	device1->CreateBuffer(&bd, NULL, &pVBuffer);		// create the buffer

	// copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT result = context1->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);		// map the buffer
	memcpy(ms.pData, vertices, arraySize);													// copy the data
	context1->Unmap(pVBuffer, NULL);														// unmap the buffer
	return pVBuffer;
}

void VxlUtil::updateWorldMatrix(float x, float y, float z) {
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

void VxlUtil::updateMirroring(bool horizontal, bool vertical) {
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

void VxlUtil::updatePalette(float palette[72])
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

void VxlUtil::selectPalette(int palette)
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

void VxlUtil::updateMatricesWithCamera(VxlCamera * camera) {

	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBuffer* dataPtr;

	worldMatrix = XMMatrixIdentity();
	projectionMatrix = getProjectionMatrix(0.1f, 1000.0f, 1, 1);
	camera->GetViewMatrix(viewMatrix);

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

void VxlUtil::updateViewMatrices(XMFLOAT4X4 view, XMFLOAT4X4 projection)
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

XMMATRIX VxlUtil::getProjectionMatrix(const float near_plane, const float far_plane, const float fov_horiz, const float fov_vert)
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

void VxlUtil::setIndexBuffer()
{
	context->IASetIndexBuffer(
		indexBuffer,
		DXGI_FORMAT_R16_UINT, // Each index is one 16-bit unsigned integer (short).
		0
	);
}

void VxlUtil::renderMesh(VoxelMesh *voxelMesh) {
	ShaderType type = voxelMesh->type;
	UINT stride = sizeof(ColorVertex); // TODO optimize
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &voxelMesh->buffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// draw the vertex buffer to the back buffer
#ifdef HOLOLENS
		context->DrawIndexedInstanced(
			voxelMesh->size,   // Index count per instance.
		    1,              // Instance count.
		    0,              // Start index location.
		    0,              // Base vertex location.
		    0               // Start instance location.
		    );
#else
	context->Draw(voxelMesh->size, 0);
#endif
}

void VxlUtil::initSampleState() {
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

void VxlUtil::createIndexBuffer()
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

void VxlUtil::updateGameTexture(const void *data) {
	
	//data = ((char*)data) + 2;
	setShader(texture);
	D3D11_MAPPED_SUBRESOURCE ms;
	HRESULT result = context1->Map(texture2d, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	//memcpy(ms.pData, data, 144480);
	memcpy(ms.pData, data, 245760);
	context1->Unmap(texture2d, NULL);
	context->PSSetSamplers(0, 1, &sampleState);
	context->PSSetShaderResources(0, 1, &textureView);
}

bool toggleBool(bool b)
{
	if (b == false)
		return true;
	else
		return false;;
}