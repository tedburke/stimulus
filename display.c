//
// display.c - written by Ted Burke - last modified 16-12-2009
//
// This program loads an animated stimulus from a sequence of
// numbered BMP files (one frame per file, 512x512 pixels, 24-bit).
// The frames are displayed very rapidly in sequence on the screen.
//
// The following is a typical command line:
//
//		display.exe 100 1280x800:32@60
//
// which tells the program to load files from 1.bmp to 100.bmp and
// specifies a screen size of 1280x800 @ 60Hz refresh rate and 32bpp.
//

#include <windows.h>
#include <stdio.h>

// OpenGL header files
#include <GL/gl.h>
#include <GL/glut.h>

// Default frames info
int number_of_frames = 2;
#define frame_width 512
#define frame_height 512
GLuint *textures;

// Default screen size and display parameters
int screen_width = 1024, screen_height = 768;
int pixel_depth = 32, refresh_rate = 60;
int image_offset_x, image_offset_y;
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
	// Parse any GLUT specific command line arguments
	glutInit(&argc, argv);
	
	// Parse other command line arguments
	// Was the number of frames specfied on command line?
	if (argc > 1)
	{
		number_of_frames = atoi(argv[1]);
	}
	
	char game_mode_string[50];

	if (argc > 2)
	{
		// Copy second command line argument into game_mode_string
		// and extract parameter values
		sprintf(game_mode_string, "%s", argv[2]);
	}
	else
	{
		sprintf(game_mode_string, "%dx%d:%d@%d", screen_width, screen_height, pixel_depth, refresh_rate);
		// Faster, but no vsync: sprintf(game_mode_string, "%dx%d:8@60", screen_width, screen_height);
	}
	
	// Start GLUT in fullscreen game mode
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutGameModeString(game_mode_string);
	main_window = glutEnterGameMode();
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(display);
	glClearColor (0.5, 0.5, 0.5, 1.0);

	// Read and display actual game mode parameters
	screen_width = glutGameModeGet(GLUT_GAME_MODE_WIDTH);
	screen_height = glutGameModeGet(GLUT_GAME_MODE_HEIGHT);
	pixel_depth = glutGameModeGet(GLUT_GAME_MODE_PIXEL_DEPTH);
	refresh_rate = glutGameModeGet(GLUT_GAME_MODE_REFRESH_RATE);
	image_offset_x = (screen_width - frame_width) / 2;
	image_offset_y = (screen_height - frame_height) / 2;
	printf("screen_width = %d\n", screen_width);
	printf("screen_height = %d\n", screen_height);
	printf("pixel_depth = %d\n", pixel_depth);
	printf("refresh_rate = %d\n", refresh_rate);
	
	// Select an orthogonal projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, screen_width, 0, screen_height);

	// Load frames
	FILE *image_file;
	char file_name[12];
	
	// Create a texture for each frame of the stimulus
	textures = malloc(number_of_frames * sizeof(GLuint));
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
		// NB This seems to work best when the textures are stored internally
		// as RGBA pixel values.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, frame_width, frame_height, 0,
						GL_RGBA, GL_UNSIGNED_BYTE, frame);
	}

	// Enable textures
	//glEnable(GL_TEXTURE_2D);
	
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
	
	// Slide image to the right 1 pixel to visualise frame rate as speed of motion
	//if (++image_offset_x >= screen_width - 512) image_offset_x = 0;
	
	// Clearing background doesn't seem to be slowing the program
	// down on this (newer) computer, so I'm putting it back in for
	// the moment.
	glClear(GL_COLOR_BUFFER_BIT);
	
	// Enable textures
	glEnable(GL_TEXTURE_2D);

	// Texture render method
	glBindTexture(GL_TEXTURE_2D, textures[n]);
	//Slower cycling: glBindTexture(GL_TEXTURE_2D, textures[(frames_rendered/30)%number_of_frames]);
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex2i(image_offset_x, image_offset_y);
	glTexCoord2f(1.0, 0.0);
	glVertex2i(image_offset_x + frame_width, image_offset_y);
	glTexCoord2f(1.0, 1.0);
	glVertex2i(image_offset_x + frame_width, image_offset_y + frame_height);
	glTexCoord2f(0.0, 1.0);
	glVertex2i(image_offset_x, image_offset_y + frame_height);
	glEnd();

	// Disable textures
	glDisable(GL_TEXTURE_2D);	
	
	// Show flashing box at bottom left
	int c = frames_rendered%2;
	glColor4f(c, c, c, 1.0);
	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(50, 0);
	glVertex2i(50, 50);
	glVertex2i(0, 50);
	glEnd();
	
	// Show red dot at centre of stimulus
	glColor4f(1, 0, 0, 1.0);
	glBegin(GL_QUADS);
	glVertex2i(screen_width/2 - 5, screen_height/2 - 5);
	glVertex2i(screen_width/2 + 5, screen_height/2 - 5);
	glVertex2i(screen_width/2 + 5, screen_height/2 + 5);
	glVertex2i(screen_width/2 - 5, screen_height/2 + 5);
	glEnd();
	
	glutSwapBuffers();

	// Redraw again as soon as possible
	glutPostRedisplay();
	
	// Make sure background is clear in front and back buffers
	// This is important if the background is not being cleared
	// each time a frame is rendered - see above.
	if (frames_rendered < 2) glClear(GL_COLOR_BUFFER_BIT);

	// Increment frame counter
	if (first_frame_time == 0)
	{
		// Remember time that first frame finished rendering,
		// then count all subsequent frames rendered.
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
		free(textures);
		glutDestroyWindow(main_window);
	
		// Quit the program
		exit(0);
	}	
}
