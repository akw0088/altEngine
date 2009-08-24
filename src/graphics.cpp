#include "include.h"

#ifdef DIRECTX
Graphics::gfx()
{
}

Graphics::~gfx() 
{
}

void Graphics::init(void *param1, void *param2)
{
	hwnd = *((HWND *)param1);
	hdc = *((HDC *)param2);
	HRESULT		ret;

	d3d = Direct3DCreate9(D3D_SDK_VERSION);
	if (d3d == NULL)
		return;
	ret = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, d3dpp, &device);
	if (ret != D3D_OK)
		return;
}

void Graphics::drawText(const char *str, float x, float y)
{

}

void Graphics::swap()
{
	device->Present(NULL, NULL, NULL, NULL);
}

void Graphics::resize(int width, int height)
{

}

void Graphics::clear()
{
	device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(128, 128, 128), 1.0f, 0);
}

void Graphics::destroy()
{
	if (device)
		device->Release();
	if (d3d)
		d3d->Release();

}

void Graphics::VertexArray(int numVerts, void *vert)
{
	device->CreateVertexBuffer(numVerts * sizeof(vertex_t), 0, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE, D3DPOOL_SYSTEMMEM, vertexBuffer, NULL);
	vertexBuffer->Lock(0, data->numVerts * sizeof(vertex_t), &pVert, 0);
	memcpy(pVert, vert, numVerts * sizeof(vertex_t));
	vertexBuffer->Unlock();
	device->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE);
	device->SetStreamSource(0, vertexBuffer, 0, sizeof(vertex_t));
}

void Graphics::TextureArray(void *tex)
{
}

void Graphics::NormalArray(void *normal)
{
}

void Graphics::DrawArray(const int numIndexes, void *Indexes)
{
	int *pIndex = NULL;

	device->CreateIndexBuffer(numIndexes * sizeof(int), 0, D3DFMT_INDEX16,  D3DPOOL_DEFAULT, indexBuffer, NULL);
	indexBuffer->Lock(0, indexBufferSize * sizeof(int), &pIndex, 0);
	memcpy(pIndex, Indexes, numIndexes * sizeof(int));
	indexBuffer->Unlock();
	device->SetIndices(indexBuffer);

	device->DrawIndexedPrimitive(D3DPT_TRIANGLEFAN, 0, 0, numVerts, 0, numIndexes);
}

void Graphics::SelectTexture(int index)
{
}

void Graphics::DeselectTexture()
{
}

void Graphics::InitTextures(int numTextures)
{

}

void Graphics::LoadTexture(int index, int width, int height, int components, int format, void *bytes)
{

}

#else

Graphics::Graphics()
{
	texObject = NULL;
}

Graphics::~Graphics()
{
	if (texObject)
		delete [] texObject;
}

void Graphics::init(void *param1, void *param2)
{
#ifdef _WIN32
	hwnd = *((HWND *)param1);
	hdc = *((HDC *)param2);
#else
	display = (Display *)param1;
	window = *((Window *)param2);
#endif

#ifdef _WIN32
	wglUseFontBitmaps(hdc, ' ', '~', 1000);
#else
	font = XLoadQueryFont(display, "-*-courier-bold-r-normal--14-*-*-*-*-*-*-*");
	if (font)
		glXUseXFont(font->fid, ' ', '~', 1000);
	else
		printf("Unable to load font!\n");
#endif

	glMatrixMode(GL_TEXTURE);
	glRotatef(180.0f, 0.0f, 0.0f, 1.0f);
	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_DEPTH_TEST);
	glColor3f(1.0f, 1.0f, 1.0f);
	glPointSize(5.0);
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);


}

void Graphics::drawText(const char *str, float x, float y)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
		glLoadIdentity();
		glOrtho(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.1f);
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
			glLoadIdentity();
			glRasterPos2f(x, y);
			glListBase(1000 - ' ');
			glCallLists((GLsizei)strlen(str), GL_UNSIGNED_BYTE, str);	
		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void Graphics::swap()
{
#ifdef _WIN32
	SwapBuffers(hdc);
#else
	glXSwapBuffers(display, window);
#endif
}

void Graphics::resize(int width, int height)
{
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (!height)
		height = 1;
	gluPerspective( 45.0, (float) width / height, 1.0, 2001.0 );
	glMatrixMode(GL_MODELVIEW);
}

void Graphics::clear()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void Graphics::loadMatrix(const float *matrix)
{
	glLoadMatrixf(matrix);
}

void Graphics::destroy()
{
#ifdef _WIN32
	// ;o)
#else
	if (font)
		XUnloadFont(display, font->fid);
#endif
	glDeleteLists(1000, '~' - ' ');
	glDeleteTextures(numTextures, texObject);
}

void Graphics::VertexArray(void *vert, int numVerts)
{
	glVertexPointer(3, GL_FLOAT, sizeof(vertex_t), vert);
}

void Graphics::TextureArray(void *tex, int numTexs)
{
	glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), tex );
}

void Graphics::NormalArray(void *normal_array, int num_normal)
{
	glNormalPointer(GL_FLOAT, sizeof(vertex_t), normal_array );
}

void Graphics::DrawArray(void *index_array, int num_index)
{
	glDrawElements(GL_TRIANGLES, num_index, GL_UNSIGNED_INT, index_array);
}

void Graphics::SelectTexture(int index)
{
	glBindTexture(GL_TEXTURE_2D, texObject[index]);
	glEnable(GL_TEXTURE_2D);
}

void Graphics::DeselectTexture()
{
	glDisable(GL_TEXTURE_2D);
}

void Graphics::InitTextures(int numTextures)
{
	texObject = new unsigned int [numTextures];
	glGenTextures(numTextures, texObject);
	Graphics::numTextures = numTextures;
}

void Graphics::LoadTexture(int index, int width, int height, int components, int format, void *bytes)
{
	glBindTexture(GL_TEXTURE_2D, texObject[index]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, components, width, height, 0, format, GL_UNSIGNED_BYTE, bytes);
}
#endif

