//HW 0 - Moving Square
//Starter code for the first homework assignment.
//This code assumes SDL2 and OpenGL are both properly installed on your system

#include "glad.h"  //Include order can matter here
#if defined(__APPLE__) || defined(__linux__)
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#else
#include <SDL.h>
#include <SDL_opengl.h>
#endif
#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

//Name of image texture
string textureName = "goldy.ppm";

//Globals to store the state of the square (position, width, and angle)
// position is based on center of square
float g_pos_x = 0.0f;
float g_pos_y = 0.0f;

float g_size = 0.6f;
float g_angle = 0;

float vertices[] = {
    //  X     Y     R     G     B     U    V
    0.3f,  0.3f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
    0.3f, -0.3f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
    -0.3f,  0.3f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,  // top left
    -0.3f, -0.3f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // bottom left
}; 

int screen_width = 800;
int screen_height = 800;

float g_mouse_down = false;
float g_clicked_x = -1;
float g_clicked_y = -1;
float g_lastCenter_x = -1;
float g_lastCenter_y = -1;
float g_clicked_angle = -1;
float g_clicked_size = -1;

float g_clicked_x_scaled_rotated = -1;
float g_clicked_y_scaled_rotated = -1;

void mouseClicked(float mx, float my);
void mouseDragged(float mx, float my);

bool g_bTranslate = false;
bool g_bRotate = false;
bool g_bScale = false;

/// Set this flag to true to have the square rotate without any user input
bool isAutomaticallyRotating = false;

/// A point in a 2d coordinate system
struct Point {
    float x;
    float y;
};

//////////////////////////
///  Begin your code here
/////////////////////////

//TODO: Read from ASCII (P6) PPM files
//Inputs are output variables for returning the image width and heigth
unsigned char* loadImage(int& img_w, int& img_h){
    
    //Open the texture image file
    ifstream ppmFile;
    ppmFile.open(textureName.c_str());
    if (!ppmFile){
        printf("ERROR: Texture file '%s' not found.\n",textureName.c_str());
        exit(1);
    }
    
    //Check that this is an ASCII PPM (first line is P3)
    string PPM_style;
    ppmFile >> PPM_style; //Read the first line of the header
    if (PPM_style != "P3") {
        printf("ERROR: PPM Type number is %s. Not an ASCII (P3) PPM file!\n",PPM_style.c_str());
        exit(1);
    }
    
    //Read in the texture width and height
    ppmFile >> img_w >> img_h;
    int imgSize = img_w * img_h;
    int imgSizeAllChannels = 4 * imgSize;
    unsigned char* img_data = new unsigned char[imgSizeAllChannels];
    
    //Check that the 3rd line is 255 (ie., this is an 8 bit/pixel PPM)
    int maximum;
    ppmFile >> maximum;
    if (maximum != 255) {
        printf("ERROR: Maximum size is (%d) not 255.\n",maximum);
        exit(1);
    }


    // variables used for while loop
    int currentIndex = 0;
    int red, green, blue;
    
    
    // need an intermediary place to store the color data after reading it in
    unsigned char *reds = new unsigned char[imgSize];
    unsigned char *greens = new unsigned char[imgSize];
    unsigned char *blues = new unsigned char[imgSize];
    
    while(ppmFile >> red >> green >> blue){
        // darken the image before storing into the texture data
        const int darknessAmount = 60;
        red -= darknessAmount;
        green -= darknessAmount;
        blue -= darknessAmount;
        
        // make sure nothing goes below 0 as negative values are invalid
        if (red < 0) {
            red = 0;
        }
        
        if (green < 0) {
            green = 0;
        }
        
        if (blue < 0) {
            blue = 0;
        }
        
        
        reds[currentIndex] = red;
        greens[currentIndex] = green;
        blues[currentIndex] = blue;
        currentIndex++;
    }

    
    // reverses the color data to actually store it
    for (int i = 0; i < img_h; i++){
        for (int j = 0; j < img_w; j++){
            img_data[i*img_w*4 + j*4] = reds[imgSize - 1 - i * img_w + j];  //Red
            img_data[i*img_w*4 + j*4 + 1] = greens[imgSize - 1 - i * img_w + j];  //Green
            img_data[i*img_w*4 + j*4 + 2] = blues[imgSize - 1 - i * img_w + j];  //Blue
            img_data[i*img_w*4 + j*4 + 3] = 255;  //Alpha
        }
    }
    
    return img_data;
}


void updateVertices(){
    
    // this is what actually adjusts the square when it's being automatically rotated
    if (isAutomaticallyRotating) {
        g_angle += 0.005;
    }

    
    float vx = g_size;
    float vy =  g_size;
    vertices[0] = g_pos_x + cos(g_angle) * vx - sin(g_angle) * vy;  //Top right x
    vertices[1] = g_pos_y + sin(g_angle) * vx + cos(g_angle) * vy;  //Top right y
    
    vx = g_size;
    vy = - g_size;
    vertices[7] = g_pos_x + cos(g_angle) * vx - sin(g_angle) * vy;  //Bottom right x
    vertices[8] = g_pos_y + sin(g_angle) * vx + cos(g_angle) * vy;  //Bottom right y
    
    vx = - g_size;
    vy = + g_size;
    vertices[14] =  g_pos_x + cos(g_angle) * vx - sin(g_angle) * vy;  //Top left x
    vertices[15] =  g_pos_y + sin(g_angle) * vx + cos(g_angle) * vy;  //Top left y
    
    vx = - g_size;
    vy = - g_size;
    vertices[21] =  g_pos_x + cos(g_angle) * vx - sin(g_angle) * vy;  //Bottom left x
    vertices[22] =  g_pos_y + sin(g_angle) * vx + cos(g_angle) * vy;  //Bottom left y
}

/// Get a point converted to
//struct Point getPointInSquareCoordinates() {
    //return struct Point {}
//}

// Choose between translate, rotate, and scale based on where the user clicked
void mouseClicked(float m_x, float m_y){   
    printf("Clicked at %f, %f\n",m_x,m_y);
    g_clicked_x = m_x;
    g_clicked_y = m_y;
    g_lastCenter_x = g_pos_x;
    g_lastCenter_y = g_pos_y;
    g_clicked_angle = g_angle;
    g_clicked_size = g_size;
    
    g_bTranslate = false;
    g_bRotate = false;
    g_bScale = false;
    
    // x and y is the click position normalized to size of the square, with (-1,-1) at one corner (1,1) the other
    float xScaled = m_x - g_pos_x;
    float yScaled = m_y - g_pos_y;
    
    float negativeAngle = g_angle * -1;
    
    // convert to the square's coordinate system (necessary if the square is rotated)
    float x = cos(negativeAngle) * xScaled - sin(negativeAngle) * yScaled;
    float y = sin(negativeAngle) * xScaled + cos(negativeAngle) * yScaled;
    
    // these values will be needed in mouseDragged
    g_clicked_x_scaled_rotated = x;
    g_clicked_y_scaled_rotated = y;
    
    // as x and y are only used for position checking, we only need to look at abs values
    x = abs(x / g_size);
    y = abs(y / g_size);
    
    printf("Normalized click coord: %f, %f\n",xScaled,yScaled);
    cout << "x after rotate: " << x << endl;
    cout << "y after rotate: " << y << endl;

    // if it's outside the square, do nothing
    if (x > 1.05 || y > 1.05) {
        return;
    }
    
    if (x > 0.9 && y > 0.9) {
        g_bRotate = true;
        cout << "rotating" << endl;
    } else if (x > 0.9 || y > 0.9) {
        g_bScale = true;
        cout << "scaling" << endl;
    } else {
        g_bTranslate = true;
        cout << "translating" << endl;
    }
    

}

//TODO: Update the position, rotation, or scale based on the mouse movement
//  I've implemented the logic for position, you need to do scaling and angle
//TODO: Notice how smooth draging the square is (e.g., there are no "jumps" when you click), 
//      try to make your implementation of rotate and scale as smooth
void mouseDragged(float m_x, float m_y) {
    // to only translate if inside the square - compare the mouse position to the square's area and see if it falls in the square's area; if it does, then move the square; else, do nothing
    if (g_bTranslate){
        g_pos_x = m_x-g_clicked_x+g_lastCenter_x;
        g_pos_y = m_y-g_clicked_y+g_lastCenter_y;
    }
    
    // Compute the new size, g_size, based on the mouse positions
    if (g_bScale){
        // scale when dragging on the edges - take into account drags in both x and y direction
        g_size = (m_x - g_clicked_x) + (m_y - g_clicked_y) + g_clicked_size;
        
        // we don't want the size to go greater than 1 as that means we can't see the entire square on the screen at once
        if (g_size > 1) {
            g_size = 1;
        
        // if the size can go under 0, then it'll do weird stuff - therefore, this must be prevented by making it stay at/above a really small positive value
        } else if (g_size < 0.01) {
            g_size = 0.01;
        }
    }
    
    if (g_bRotate){
        //Compute the new angle, g_angle, based on the mouse positions
        // maybe it could be based on looking at the distance from the center of the square (the mouse down point and the current point) to get an angle (with an imaginary unit circle)
        
        // need to view the positions in the square's coordinate system
        
        /*float negativeAngle = g_angle * -1;
        float xRotated = cos(negativeAngle) * m_x - sin(negativeAngle) * m_y;
        float yRotated = sin(negativeAngle) * m_x + cos(negativeAngle) * m_y;
        
        double mouseDownAngle = atan2(g_clicked_x_scaled_rotated, g_clicked_y_scaled_rotated);
        double mouseDraggedAngle = atan2(xRotated, yRotated);*/
        
        double mouseDownAngle = atan2(g_clicked_x, g_clicked_y);
        double mouseDraggedAngle = atan2(m_x, m_y);
        
        g_angle = mouseDownAngle - mouseDraggedAngle;
        cout << "angle: " << g_angle << endl;
    }
    
    updateVertices();
}

/////////////////////////////
/// ... below is OpenGL specifc code,
///     we will cover it in detail around Week 9,
///     but you should try to poke around a bit righ now.
///     I've annotated some parts with "TODO: Test ..." check those out.
////////////////////////////

// Shader sources
const GLchar* vertexSource =
"#version 150 core\n"
"in vec2 position;"
"in vec3 inColor;"
"in vec2 inTexcoord;"
"out vec3 Color;"
"out vec2 texcoord;"
"void main() {"
"   Color = inColor;"
"   gl_Position = vec4(position, 0.0, 1.0);"
"   texcoord = inTexcoord;"
"}";

const GLchar* fragmentSource =
"#version 150 core\n"
"uniform sampler2D tex0;"
"in vec2 texcoord;"
"out vec3 outColor;"
"void main() {"
"   outColor = texture(tex0, texcoord).rgb;"
"}";

bool fullscreen = false;

int main(int argc, char *argv[]){
    SDL_Init(SDL_INIT_VIDEO);  //Initialize Graphics (for OpenGL)
    
    //Ask SDL to get a fairly recent version of OpenGL (3.2 or greater)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    
    //Create a window (offsetx, offsety, width, height, flags)
    SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screen_width, screen_height, SDL_WINDOW_OPENGL);
    //TODO: Test your understanding: Try changing the title of the window to something more personalized.
    
    //The above window cannot be resized which makes some code slightly easier.
    //Below show how to make a full screen window or allow resizing
    //SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 0, 0, screen_width, screen_height, SDL_WINDOW_FULLSCREEN|SDL_WINDOW_OPENGL);
    //SDL_Window* window = SDL_CreateWindow("My OpenGL Program", 100, 100, screen_width, screen_height, SDL_WINDOW_RESIZABLE|SDL_WINDOW_OPENGL);
    //SDL_Window* window = SDL_CreateWindow("My OpenGL Program",SDL_WINDOWPOS_UNDEFINED,SDL_WINDOWPOS_UNDEFINED,0,0,SDL_WINDOW_FULLSCREEN_DESKTOP|SDL_WINDOW_OPENGL); //Boarderless window "fake" full screen
    
    float aspect = screen_width/(float)screen_height; //aspect ratio (needs to be updated if the window is resized)
    
    updateVertices(); //set initial position of the square to match it's state
    
    //Create a context to draw in
    SDL_GLContext context = SDL_GL_CreateContext(window);
    
    //OpenGL functions using glad library
    if (gladLoadGLLoader(SDL_GL_GetProcAddress)){
        printf("OpenGL loaded\n");
        printf("Vendor:   %s\n", glGetString(GL_VENDOR));
        printf("Renderer: %s\n", glGetString(GL_RENDERER));
        printf("Version:  %s\n", glGetString(GL_VERSION));
    }
    else {
        printf("ERROR: Failed to initialize OpenGL context.\n");
        return -1;
    }
    
    //// Allocate Texture 0 (Created in Load Image) ///////
    GLuint tex0;
    glGenTextures(1, &tex0);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex0);
    
    //What to do outside 0-1 range
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //GL_LINEAR
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //GL_LINEAR
    //TODO: Test your understanding: Try GL_LINEAR instead of GL_NEAREST on the 4x4 test image. What is happening?
    
    
    int img_w, img_h;
    unsigned char* img_data = loadImage(img_w,img_h);
    printf("Loaded Image of size (%d,%d)\n",img_w,img_h);
    //memset(img_data,0,4*img_w*img_h); //Load all zeros
    //Load the texture into memory
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
    glGenerateMipmap(GL_TEXTURE_2D);
    //// End Allocate Texture ///////
    
    
    //Build a Vertex Array Object. This stores the VBO and attribute mappings in one object
    GLuint vao;
    glGenVertexArrays(1, &vao); //Create a VAO
    glBindVertexArray(vao); //Bind the above created VAO to the current context
    
    
    //Allocate memory on the graphics card to store geometry (vertex buffer object)
    GLuint vbo;
    glGenBuffers(1, &vbo);  //Create 1 buffer called vbo
    glBindBuffer(GL_ARRAY_BUFFER, vbo); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW); //upload vertices to vbo
    //GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
    //GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used
    
    
    //Load the vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
    
    //Let's double check the shader compiled
    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    if (!status){
        char buffer[512];
        glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
        printf("Vertex Shader Compile Failed. Info:\n\n%s\n",buffer);
    }
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
    
    //Double check the shader compiled
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    if (!status){
        char buffer[512];
        glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
        printf("Fragment Shader Compile Failed. Info:\n\n%s\n",buffer);
    }
    
    //Join the vertex and fragment shaders together into one program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glBindFragDataLocation(shaderProgram, 0, "outColor"); // set output
    glLinkProgram(shaderProgram); //run the linker
    
    glUseProgram(shaderProgram); //Set the active shader (only one can be used at a time)
    
    
    //Tell OpenGL how to set fragment shader input
    
    GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
    //               Attribute, vals/attrib., type, normalized?, stride, offset
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 7*sizeof(float), 0);
    glEnableVertexAttribArray(posAttrib); //Binds the VBO current GL_ARRAY_BUFFER
    
    GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");
    glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(2*sizeof(float)));
    glEnableVertexAttribArray(colAttrib);
    
    GLint texAttrib = glGetAttribLocation(shaderProgram, "inTexcoord");
    glEnableVertexAttribArray(texAttrib);
    glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 7*sizeof(float), (void*)(5*sizeof(float)));
    
    
    //Event Loop (Loop forever processing each event as fast as possible)
    SDL_Event windowEvent;
    bool done = false;
    while (!done){
        while (SDL_PollEvent(&windowEvent)){  //Process input events (e.g., mouse & keyboard)
            if (windowEvent.type == SDL_QUIT) done = true;
            //List of keycodes: https://wiki.libsdl.org/SDL_Keycode - You can catch many special keys
            //Scancode referes to a keyboard position, keycode referes to the letter (e.g., EU keyboards)
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_ESCAPE)
            done = true; //Exit event loop
            if (windowEvent.type == SDL_KEYUP && windowEvent.key.keysym.sym == SDLK_f) //If "f" is pressed
            fullscreen = !fullscreen;
            SDL_SetWindowFullscreen(window, fullscreen ? SDL_WINDOW_FULLSCREEN : 0); //Set to full screen
        }
        
        
        int mx, my;
        if (SDL_GetMouseState(&mx, &my) & SDL_BUTTON(SDL_BUTTON_LEFT)) { //Is the mouse down?
            if (g_mouse_down == false){
                mouseClicked(2*mx/(float)screen_width - 1, 1-2*my/(float)screen_height);
            }
            else{
                mouseDragged(2*mx/(float)screen_width-1, 1-2*my/(float)screen_height);
            }
            g_mouse_down = true;
        }
        else{
            g_mouse_down = false;
        }
        
        updateVertices();
        
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW); //upload vertices to vbo
        
        
        // Clear the screen to white
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //Draw the two triangles (4 vertices) making up the square
        //TODO: Test your understanding: What shape do you expect to see if you change the above 4 to 3?  Guess, then try it!
        
        SDL_GL_SwapWindow(window); //Double buffering
    }
    
    delete [] img_data;
    glDeleteProgram(shaderProgram);
    glDeleteShader(fragmentShader);
    glDeleteShader(vertexShader);
    
    glDeleteBuffers(1, &vbo);
    
    glDeleteVertexArrays(1, &vao);
    
    
    //Clean Up
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return 0;
}
