//
// display.c - written by Ted Burke - last modified 15-12-2009
//
// This program loads an animated stimulus from a sequence of
// numbered BMP files (one frame per file, 512x512 pixels, 24-bit).
// The frames are displayed very rapidly in sequence on the screen.
// The frame rate is currently exceeding the refresh rate on this
// computer (60Hz), but I'm trying to find a way to synchronise it.
//

#include <windows.h>
#include <stdio.h>

// OpenGL header files
#include <GL/gl.h>
#include <GL/glut.h>

// Frames info
#define number_of_frames 4
#define frame_width 512
#define frame_height 512
GLuint textures[number_of_frames];

// Initial window size
int window_width = 1024;
int window_height = 768;
int main_window; // GLUT window id

// Callback function prototypes
void display(void);
void keyboard(unsigned char, int, int);

// Time record for frame rate estimation
GLuint first_frame_time = 0;
GLuint frames_rendered = 0;

// Main function
int main(int argc, char *argv[])
{		
	// start GLUT in fullscreen game mode
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutGameModeString("1024x768:16@60");
	main_window = glutEnterGameMode();
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(display);
	glClearColor (0.5, 0.5, 0.5, 1.0);
	
	// Select an orthogonal projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, window_width, 0, window_height);

	// Load frames
	FILE *image_file;
	char file_name[12];

	// Create a texture for each frame of the stimulus
	glGenTextures(number_of_frames, textures);	
	static GLubyte frame[frame_width * frame_height * 4];
	int n, m;
	for(n = 0 ; n < number_of_frames ; ++n)
	{
		// Open the next image file
		sprintf(file_name, "%d.bmp", n+1);
		image_file = fopen(file_name, "r");
		
		// Read bitmap header - NB This is just to skip past it. We
		// completely ignore the contents of the header.
		// There's a bit of a trick with this: We're assuming that
		// the header is exactly 0x36 bytes long, which is true for
		// all the bitmap files I checked. May not be true in general.
		fread(frame, 1, 0x36, image_file);
		for (m = 0 ; m < frame_width * frame_height ; ++m)
		{
			// Pixels in BMP file are BGR, so swap R and B
			fread(frame + 4*m + 2, 1, 1, image_file);
			fread(frame + 4*m + 1, 1, 1, image_file);
			fread(frame + 4*m, 1, 1, image_file);
			*(frame + 4*m + 3) = 255; // alpha
		}
		fclose(image_file);

		// Copy the image data into the next texture
		glBindTexture(GL_TEXTURE_2D, textures[n]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_width, frame_height, 0,
						GL_RGBA, GL_UNSIGNED_BYTE, frame);
	}

	// Enable textures
	glEnable(GL_TEXTURE_2D);
	
	// GLUT main loop
	glutMainLoop();

	return 0;
}

// Draw the next frame of the stimulus
void display(void)
{
	// Draw frame
	static int n = 0;
	if (++n >= number_of_frames) n = 0;
	
	// offset to centre the image on screen
	static int x = 256, y = 128;
	
	// Texture render method
	glBindTexture(GL_TEXTURE_2D, textures[n]);
	//glColor4f(1.0, 1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex2i(x, y);
	glTexCoord2f(1.0, 0.0);
	glVertex2i(x + frame_width, y);
	glTexCoord2f(1.0, 1.0);
	glVertex2i(x + frame_width, y + frame_height);
	glTexCoord2f(0.0, 1.0);
	glVertex2i(x, y + frame_height);
	glEnd();
	
	//glFlush ();
	glutSwapBuffers();
	
	// Redraw again as soon as possible
	glutPostRedisplay();
	
	// Increment frame counter
	if (first_frame_time == 0)
	{
		glClear(GL_COLOR_BUFFER_BIT);
		first_frame_time = GetTickCount();
	}
	else frames_rendered++;
}

// This callback function handles normal keystrokes
void keyboard(unsigned char key, int x, int y)
{
	if (key == 'q')
	{
		// Print an estimate of the frame rate
		GLuint elapsed_time = GetTickCount() - first_frame_time;
		printf("%d frames rendered in %dms. That's %lf fps.\n",
				frames_rendered, elapsed_time, frames_rendered / (elapsed_time / 1000.0));
		
		// Delete the stimulus texture frames and the GLUT window
		glDeleteTextures(number_of_frames, textures);
		glutDestroyWindow(main_window);
	
		// Quit the program
		exit(0);
	}	
}
