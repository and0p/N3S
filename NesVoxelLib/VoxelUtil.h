// Obviously thanks to http://www.directxtutorial.com/ for a bunch of this
// As well as http://www.braynzarsoft.net/viewtutorial/q16390-4-begin-drawing

#pragma once
#include "VoxelCamera.h"

using namespace DirectX;

enum ShaderType { color, texture };

struct ColorVertex {
	XMFLOAT4 Pos;
	XMFLOAT4 Col;
};

struct TextureVertex {
	XMFLOAT4 Pos;
	XMFLOAT2 UV;
};

struct VoxelMesh {
	ID3D11Buffer *buffer;
	ShaderType type;
	int size;
};

struct MatrixBufferType
{
	XMMATRIX world;
	XMMATRIX view;
	XMMATRIX projection;
};

struct ShaderSet
{
	ID3D11VertexShader *vertexShader;
	ID3D11PixelShader *pixelShader;
};

class VoxelUtil {
public:
	static void initPipeline(
		Microsoft::WRL::ComPtr<ID3D11Device> d3dDevice, Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3dContext,
		Microsoft::WRL::ComPtr<ID3D11Device1> d3dDevice1, Microsoft::WRL::ComPtr<ID3D11DeviceContext1> d3dContext1);

	static ID3D11Buffer* createBufferFromColorVertices(ColorVertex vertices[], int arraySize);

	static VoxelMesh* CreateRectangle();

	static void updateMatricesWithCamera(VoxelCamera * camera);

	static XMMATRIX getProjectionMatrix(const float near_plane, const float far_plane, const float fov_horiz, const float fov_vert);

	static void setShader(ShaderType type);

	static void renderMesh(VoxelMesh *voxelMesh);
private:
	static Microsoft::WRL::ComPtr<ID3D11Device>            device;
	static Microsoft::WRL::ComPtr<ID3D11Device1>           device1;
	static Microsoft::WRL::ComPtr<ID3D11DeviceContext>     context;
	static Microsoft::WRL::ComPtr<ID3D11DeviceContext1>    context1;
	static void initShaders();
	static void initSampleState();
	static ShaderSet shaderSets[2];
	static ID3D11InputLayout *inputLayouts[2];
	static ID3D11Buffer *m_matrixBuffer;
	static MatrixBufferType *dataPtr;
	//static ID3D11SamplerState* sampleState;
	//static ID3D11Texture2D* texture2d;
	//static ID3D11ShaderResourceView* textureView;
};