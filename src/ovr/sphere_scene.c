
#include <gpac/sphere_scene.h>

#define STB_IMAGE_IMPLEMENTATION
#include <gpac/stb_image.h>

#ifdef _WIN32
#include <windows.h>
#else
#define APIENTRY
#endif


// TODO figure out alternative in Linux
static void APIENTRY DebugCallback
(
	GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam
)
{
	printf("GL_DEBUG %d, %d, %d, %s\n", source, type, severity, message);
}


void ovr_sphere_init(SphereScene* sphere, int rings, int sectors)
{

	fprintf(stderr, "gf_ovr_sphere_init\n");

	ovrMatrix4f identity = ovrMatrix4f_CreateIdentity();

	//
	sphere->projection = ovrMatrix4f_Transpose(&identity);
	sphere->model_view = ovrMatrix4f_Transpose(&identity);
	sphere->texMat = ovrMatrix4f_Transpose(&identity);

	sphere->sphereVertices = NULL;
	sphere->sphereIndices = NULL;

	sphere->h_vao = 0;
	sphere->h_vboVertices = 0;
	sphere->h_vboIndices = 0;

	sphere->h_shaderProg = ovr_sphere_load_shaders();

	fprintf(stderr, "gf_ovr_sphere_init, shaders are loaded\n");

	// get handles for uniform variables
	sphere->h_projUniform = glGetUniformLocation(sphere->h_shaderProg, "proj");
	sphere->h_modelViewUniform = glGetUniformLocation(sphere->h_shaderProg, "model_view");
	sphere->h_texMatUniform = glGetUniformLocation(sphere->h_shaderProg, "tex_mat");
	sphere->h_srcFmtUnifrom = glGetUniformLocation(sphere->h_shaderProg, "src_fmt");
	sphere->h_texUniform = glGetUniformLocation(sphere->h_shaderProg, "tex");

	float phiStep = SPHERE_PI / rings;
	float thetaStep = SPHERE_2PI / sectors;

	int count_sphere_vertices = (rings - 1) * (sectors + 1);
	sphere->sphereVertices = malloc(count_sphere_vertices * sizeof(SphereVertex));

	SphereVertex vertex;

	int c = 0;

	for (int i = 1; i <= rings - 1; i++) {

		float phi = i * phiStep;

		for (int j = 0; j <= sectors; j++) {

			float theta = j * thetaStep;

			float x = sphere->radius * sinf(SPHERE_PI) * cosf(theta);
			float y = sphere->radius * cosf(SPHERE_PI);
			float z = sphere->radius * sinf(SPHERE_PI) * sinf(theta);

			float u = theta / SPHERE_2PI;
			float v = phi / SPHERE_PI;

			vertex.position.x = x;
			vertex.position.y = y;
			vertex.position.z = z;

			vertex.texCoord.x = u;
			vertex.texCoord.y = v;

			sphere->sphereVertices[c++] = vertex;
		}
	}

	int count_sphere_indices = sectors * 3 + (rings - 2) * sectors * 6 + sectors * 3;
	sphere->sphereIndices = malloc(count_sphere_indices * sizeof(GLuint));

	c = 0;

	for (int i = 1; i <= sectors; i++) {
		sphere->sphereIndices[c++] = 0;
		sphere->sphereIndices[c++] = i + 1;
		sphere->sphereIndices[c++] = i;
	}

	int baseIndex = 0;
	int ringVertexCount = sectors + 1;

	for (int i = 0; i < rings - 2; i++) {
		for (int j = 0; j < sectors; j++) {
			sphere->sphereIndices[c++] = baseIndex + i*ringVertexCount + j;
			sphere->sphereIndices[c++] = baseIndex + i*ringVertexCount + j + 1;
			sphere->sphereIndices[c++] = baseIndex + (i + 1)*ringVertexCount + j;

			sphere->sphereIndices[c++] = baseIndex + (i + 1)*ringVertexCount + j;
			sphere->sphereIndices[c++] = baseIndex + i*ringVertexCount + j + 1;
			sphere->sphereIndices[c++] = baseIndex + (i + 1)*ringVertexCount + j + 1;

		}
	}

	int southPoleIndex = count_sphere_vertices - 1;

	baseIndex = southPoleIndex - ringVertexCount - 1;

	for (int i = 0; i < sectors; i++) {
		sphere->sphereIndices[c++] = southPoleIndex;
		sphere->sphereIndices[c++] = baseIndex + i;
		sphere->sphereIndices[c++] = baseIndex + i + 1;
	}

	sphere->indexCount = count_sphere_indices;

	// creating a VAO

	glGenVertexArrays(1, &sphere->h_vao);
	glBindVertexArray(sphere->h_vao);

	glGenBuffers(1, &sphere->h_vboVertices);
	glBindBuffer(GL_ARRAY_BUFFER, sphere->h_vboVertices);
	glBufferData(GL_ARRAY_BUFFER, count_sphere_vertices * sizeof(SphereVertex), sphere->sphereVertices, GL_STATIC_DRAW);

	glGenBuffers(1, &sphere->h_vboIndices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphere->h_vboIndices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, count_sphere_indices * sizeof(unsigned int), sphere->sphereIndices, GL_STATIC_DRAW);

	// loading texture
	ovr_sphere_load_texture(sphere);

	// setting some initial values to projection and model view 
	// (these will be overwritten by the HMD values)

	//ovrQuatf quaternion;
	//ovrMatrix4f orientation = glm::toMat4(quaternion);
	//ovrMatrix4f translation = glm::translate(vec3(0.0f, 0.0f, 0.0f));
	//model_view = translation * orientation;
	sphere->model_view = ovrMatrix4f_Transpose(&identity);

	fprintf(stderr, "ovr_sphere_init, v count=%d\n", count_sphere_vertices);
	fprintf(stderr, "ovr_sphere_init, i count=%d\n", count_sphere_indices);
}


void ovr_sphere_load_texture(SphereScene* sphere)
{
	int imgWidth;
	int imgHeight;
	int imgComp;

	unsigned char* image = stbi_load(GF_OVR_SPHERE_TEXTURE, &imgWidth, &imgHeight, &imgComp, STBI_rgb);

	if (image == NULL)
	{
		fprintf(stderr, "ovr_sphere_load_texture, Failed to load texture image..\n");
	}

	fprintf(stderr, "ovr_sphere_load_texture, Image was loaded from disk..\n");

	// generate a texture object and bind it
	glGenTextures(1, &sphere->h_texture);
	glBindTexture(GL_TEXTURE_2D, sphere->h_texture);

	// set the texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if (imgComp == 3)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imgWidth, imgHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	}
	else if (imgComp == 4)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, imgWidth, imgHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	}

	// unbind texture
	glBindTexture(GL_TEXTURE_2D, 0);

	fprintf(stderr, "ovr_sphere_load_texture, Texture created!\n");
}

void ovr_sphere_draw(SphereScene* sphere)
{
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// use our shader
	glUseProgram(sphere->h_shaderProg);

	//
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sphere->h_texture);

	glUniformMatrix4fv(sphere->h_projUniform, 1, GL_FALSE, &sphere->projection.M[0][0]);
	glUniformMatrix4fv(sphere->h_modelViewUniform, 1, GL_FALSE, &sphere->model_view.M[0][0]);
	glUniform1i(sphere->h_texUniform, 0);  // texture bound to GL_TEXTURE0
	glUniform1f(sphere->h_srcFmtUnifrom, 0);
	glUniformMatrix4fv(sphere->h_texMatUniform, 1, GL_FALSE, &sphere->texMat.M[0][0]);

	glBindVertexArray(sphere->h_vao);

	glBindBuffer(GL_ARRAY_BUFFER, sphere->h_vboVertices);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(SphereVertex),
		(void*)0
	);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(SphereVertex),
		(void*)(offsetof(SphereVertex, texCoord))
	);

	// now draw the triangles
	glDrawElements(
		GL_TRIANGLES,
		sphere->indexCount,
		GL_UNSIGNED_INT,
		(void*)0
	);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);


	// unbind VAO
	glBindVertexArray(0);
}



GLuint ovr_sphere_load_shaders(void)
{
	fprintf(stderr, "ovr_sphere_load_shaders\n");

	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);
	glDebugMessageCallbackARB(DebugCallback, 0);
	glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);

	FILE* file_v = fopen(GF_OVR_SPHERE_VERTEX_SHADER, "rb");

	if (file_v == NULL)
	{
		fprintf(stderr, "ovr_sphere_load_shaders, unable to open %s\n", GF_OVR_SPHERE_VERTEX_SHADER);
	}

	fseek(file_v, 0, SEEK_END);
	long size = ftell(file_v);
	fseek(file_v, 0, SEEK_SET);
	fprintf(stderr, "ovr_sphere_load_shaders, size=%d\n", size);

	char * fcontentV = malloc(size + 1);
	long sz = fread(fcontentV, 1, size, file_v);
	fcontentV[size] = 0;

	if (sz != size)
	{
		fprintf(stderr, "ovr_sphere_load_shaders, sz=%d\n", sz);
	}

	fclose(file_v);

	FILE* file_f = fopen(GF_OVR_SPHERE_FRAGMENT_SHADER, "rb");

	fseek(file_f, 0, SEEK_END);
	size = ftell(file_f);
	fseek(file_f, 0, SEEK_SET);
	fprintf(stderr, "ovr_sphere_load_shaders, size=%d\n", size);


	char * fcontentF = malloc(size + 1);
	fread(fcontentF, 1, size, file_f);
	fcontentF[size] = 0;

	fclose(file_f);

	GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

	fprintf(stderr, "ovr_sphere_load_shaders, vertex shader id = %d\n", vertShader);
	fprintf(stderr, "ovr_sphere_load_shaders, fragment shader id = %d\n", fragShader);

	fprintf(stderr, "ovr_sphere_load_shaders, vertex:\n%s\n", fcontentV);
	fprintf(stderr, "ovr_sphere_load_shaders, fragment:\n%s\n", fcontentF);


	glShaderSource(vertShader, 1, &fcontentV, NULL);
	glShaderSource(fragShader, 1, &fcontentF, NULL);

	fprintf(stderr, "ovr_sphere_load_shaders, call glCompileShader\n");

	glCompileShader(vertShader);
	GLenum err = glGetError();
	if (err != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error %08x \n", err);
	}
	GLint result = GL_TRUE;
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		fprintf(stderr, "ovr_sphere_load_shaders, Vertex shader compilation failed..\n");
	}
	glCompileShader(fragShader);
	err = glGetError();
	if (err != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error %08x \n", err);
	}
	result = GL_TRUE;
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		fprintf(stderr, "ovr_sphere_load_shaders, Fragment shader compilation failed..\n");
	}

	GLuint program = glCreateProgram();
	glAttachShader(program, vertShader);
	glAttachShader(program, fragShader);
	glLinkProgram(program);

	err = glGetError();
	if (err != GL_NO_ERROR) {
		fprintf(stderr, "OpenGL error %08x \n", err);

		GLint maxLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

		GLchar log[1024];
		glGetProgramInfoLog(program, maxLength, &maxLength, &log[0]);
		fprintf(stderr, "OpenGL shader program log: %s \n", log);
	}

	result = GL_TRUE;
	glGetProgramiv(program, GL_LINK_STATUS, &result);
	if (result != GL_TRUE) {
		fprintf(stderr, "ovr_sphere_load_shaders, Linking failed..\n");

	}

	// clean up
	free(fcontentV);
	free(fcontentF);

	glDetachShader(program, vertShader);
	glDetachShader(program, fragShader);

	glDeleteShader(vertShader);
	glDeleteShader(fragShader);


	fprintf(stderr, "ovr_sphere_load_shaders, Shader program ready..\n");

	return program;
}