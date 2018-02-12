#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <stdio.h>
#include "ShaderProgram.h"
#include "Matrix.h"


#define STB_IMAGE_IMPLEMENTATION 
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

GLuint LoadTexture(const char *filepath);

SDL_Window* displayWindow;
SDL_Surface* surface;

//SDL_Renderer* imageRenderer;


int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	surface = SDL_GetWindowSurface(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 1280, 720);

	ShaderProgram program;
	ShaderProgram program_untex;

	Matrix projection_matrix;
	Matrix view_matrix;

	Matrix model_matrix1;
	Matrix model_matrix2;
	Matrix model_matrix3;

	GLuint potato = LoadTexture(RESOURCE_FOLDER"potato.png");
	GLuint ship = LoadTexture(RESOURCE_FOLDER"playerShip1_red.png");
	GLuint rock = LoadTexture(RESOURCE_FOLDER"meteorBrown_big1.png");
	GLuint really = LoadTexture(RESOURCE_FOLDER"Really.png");

	program_untex.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
	program.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	
	projection_matrix.SetOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	
	glUseProgram(program.programID);
	glUseProgram(program_untex.programID);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	SDL_Event event;
	bool done = false;

	float lastframeticks = 0.0f;

	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastframeticks;
		lastframeticks = ticks;
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		program.SetProjectionMatrix(projection_matrix);
		program.SetViewMatrix(view_matrix);
		program.SetModelMatrix(model_matrix1);
		glBindTexture(GL_TEXTURE_2D, potato);

		float vertices1[] = { 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices1);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoord1[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoord1);
		glEnableVertexAttribArray(program.texCoordAttribute);
		model_matrix1.Rotate(5.0f * (3.1415926f / 180.0));
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glDisableVertexAttribArray(program_untex.positionAttribute);
		glDisableVertexAttribArray(program_untex.positionAttribute);
		
		program.SetModelMatrix(model_matrix2);
		glBindTexture(GL_TEXTURE_2D, ship);
		//program.SetColor(1.0, 1.0, 0.0, 1.0);
		model_matrix2.Identity();
		model_matrix2.Translate(-2.0f, 1.0f, 0.0f);
		float vertices2[] = { 0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f, -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices2);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoord2[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, texCoord2);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		program.SetModelMatrix(model_matrix3);
		program.SetColor(0.0, 1.0, 1.0, 1.0);
		glBindTexture(GL_TEXTURE_2D, really);
		model_matrix3.Identity();
		model_matrix3.Translate(2.0f, -1.0f, 0.0f);
		model_matrix3.Scale(1, 2, 0);
		float vertices3[] = { -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  0.5f, 0.5f, -0.5f, 0.5f, -0.5f, -0.5f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices3);
		glEnableVertexAttribArray(program.positionAttribute);

		float texCoord3[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0,   1.0, 0.0, 0.0, 0.0, 0.0, 1.0 };
		glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoord3);
		glEnableVertexAttribArray(program.texCoordAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		glDisableVertexAttribArray(program.texCoordAttribute);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
GLuint LoadTexture(const char *filepath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filepath, &w, &h, &comp, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image. Check path\n";
		assert(false);
	}

	GLuint reTexture;
	glGenTextures(1, &reTexture);
	glBindTexture(GL_TEXTURE_2D, reTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(image);
	return reTexture;
}

