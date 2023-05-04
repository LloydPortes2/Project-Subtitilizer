#include <stdio.h>
#include <GLFW/glfw3.h>
#include "video_reader.hpp"


int main(int argc, const char** argv) {
	GLFWwindow* window;

	if(!glfwInit()) {
		printf("couldn't init GLFW\n");
		return 1;
	}

	window = glfwCreateWindow(1920, 1080, "Hello World", NULL, NULL);
	if (!window) {
		printf("Couldn't open Window\n");
	}

	
	//read a new frame and load into texture
	VideoReaderState vr_state;
	if (!video_reader_open(&vr_state, "D:\\Youtube Videos\\Needs Edtiting\\[Reaktor] Fullmetal Alchemist Brotherhood - E01 v2 [1080p][x265][10-bit][Dual-Audio].mkv")) {
		printf("couldn't open file\n");
			return 1;
	}
	//Generates texture
	glfwMakeContextCurrent(window);
	GLuint tex_handle;
	glGenTextures(1, &tex_handle);
	glBindTexture(GL_TEXTURE_2D, tex_handle);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//allocates frame buffer
	const int frame_width =vr_state.width;
	const int frame_height = vr_state.height;
	uint8_t* frame_data = new uint8_t[frame_width * frame_height * 4];

	
	double first_first_time;
	glfwGetTime();

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Setting up orphogrpahic projection
		int window_width, window_height;
		glfwGetFramebufferSize(window, &window_width, &window_height);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, window_width, window_height, 0, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		int x_center = window_width / 2 - frame_width / 2;
		int y_center = window_height / 2 - frame_height / 2;
		
		int64_t pts;
		if (!video_reader_read_frame(&vr_state, frame_data, &pts)) {
			printf("couldn't read frame\n");
			return 1;
		}

		static bool first_frame = true;
		if (first_frame) {
			glfwSetTime(0.0);
			first_frame = false;
		}

		//double time = glfwGetTime();
		double pt_in_seconds = pts * (double)vr_state.time_base.num / (double)vr_state.time_base.den;
		double glfw_time;
		while (pt_in_seconds > glfwGetTime()) {
			glfwWaitEventsTimeout(pt_in_seconds - glfwGetTime());
		}

		glBindTexture(GL_TEXTURE_2D, tex_handle);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_width, frame_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, frame_data);


		//renderer
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, tex_handle);
		glBegin(GL_QUADS);
		glTexCoord2d(0, 0); glVertex2i(x_center, y_center);
		glTexCoord2d(1, 0); glVertex2i(x_center + frame_width, y_center);
		glTexCoord2d(1, 1); glVertex2i(x_center + frame_width, y_center + frame_height);
		glTexCoord2d(0, 1); glVertex2i(x_center, y_center + frame_height);
		glEnd();
		glDisable(GL_TEXTURE_2D);
		glfwSwapBuffers(window);
		glfwWaitEvents();
	}

	video_reader_close(&vr_state);
	return 0;
}
