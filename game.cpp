#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <cstring>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <glad/glad.h>
#include <FTGL/ftgl.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>
#include <SFML/Audio.hpp>
 
#define BROWN 0.325490f, 0.188235f, 0.101960f
#define GREY 0.35f, 0.35f, 0.35f
#define GREEN 0.0f, 1.0f, 0.0f
#define SOILGREEN 0.3f, 1.0f, 0.3f
#define BLUE 0.0f, 0.0f, 1.0f
#define SPACEBLUE 0.505882353f, 0.721568627f, 0.874509804f
#define SKYBLUE 0.866666667f, 0.905882353f, 1.0f
#define RED 1.0f, 0.0f, 0.0f
#define LIGHTBROWN 0.274509804f, 0.149019608f, 0.066666667f
#define YELLOW 1.0f, 1.0f, 0.0f

#define WINDOW_LEFT -140.f
#define WINDOW_RIGHT 1020.0f
#define WINDOW_UP 490.0f
#define WINDOW_DOWN -90.0f

const int MAX_SEGMENTS = 1500;
const int rectangleVertex = 6;
const float g = 50.0;
const int cr = 0;
float fontScaleValue = 49.607142, fontx = 100.7142857, fonty = 407.1428571;
int ballsLeft = 25, score = 6000, timeLeft = 100, mainTargets = 3;

sf::Music buffer;
char scoreboard[100];

GLfloat u_xn = WINDOW_LEFT + 100.0f;
GLfloat u_xp = WINDOW_RIGHT - 100.0f;
GLfloat u_yn = WINDOW_DOWN + 50.0f;
GLfloat u_yp = WINDOW_UP - 50.0f;


using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
  glm::mat4 projection;
  glm::mat4 model;
  glm::mat4 view;
  GLuint MatrixID;
} Matrices;

struct FTGLFont {
  FTFont* font;
  GLuint fontMatrixID;
  GLuint fontColorID;
} GL3Font;


typedef struct GLMatrices GLMatrices;

GLuint programID, fontProgramID;

glm::vec3 getRGBfromHue (int hue)
{
  float intp;
  float fracp = modff(hue/60.0, &intp);
  float x = 1.0 - abs((float)((int)intp%2)+fracp-1.0);

  if (hue < 60)
    return glm::vec3(1,x,0);
  else if (hue < 120)
    return glm::vec3(x,1,0);
  else if (hue < 180)
    return glm::vec3(0,1,x);
  else if (hue < 240)
    return glm::vec3(0,x,1);
  else if (hue < 300)
    return glm::vec3(x,0,1);
  else
    return glm::vec3(1,0,x);
}

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

  // Create the shaders
  GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  // Read the Vertex Shader code from the file
  std::string VertexShaderCode;
  std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
  if(VertexShaderStream.is_open())
  {
    std::string Line = "";
    while(getline(VertexShaderStream, Line))
      VertexShaderCode += "\n" + Line;
    VertexShaderStream.close();
  }

  // Read the Fragment Shader code from the file
  std::string FragmentShaderCode;
  std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
  if(FragmentShaderStream.is_open()){
    std::string Line = "";
    while(getline(FragmentShaderStream, Line))
      FragmentShaderCode += "\n" + Line;
    FragmentShaderStream.close();
  }

  GLint Result = GL_FALSE;
  int InfoLogLength;

  // Compile Vertex Shader
  printf("Compiling shader : %s\n", vertex_file_path);
  char const * VertexSourcePointer = VertexShaderCode.c_str();
  glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
  glCompileShader(VertexShaderID);

  // Check Vertex Shader
  glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> VertexShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

  // Compile Fragment Shader
  printf("Compiling shader : %s\n", fragment_file_path);
  char const * FragmentSourcePointer = FragmentShaderCode.c_str();
  glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
  glCompileShader(FragmentShaderID);

  // Check Fragment Shader
  glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
  glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
  glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
  fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

  // Link the program
  fprintf(stdout, "Linking program\n");
  GLuint ProgramID = glCreateProgram();
  glAttachShader(ProgramID, VertexShaderID);
  glAttachShader(ProgramID, FragmentShaderID);
  glLinkProgram(ProgramID);

  // Check the program
  glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
  glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
  std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
  glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
  fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

  glDeleteShader(VertexShaderID);
  glDeleteShader(FragmentShaderID);

  return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO 
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors 
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

void updateProjection()
{
    Matrices.projection = glm::ortho(u_xn, u_xp, u_yn, u_yp, 0.1f, 500.0f);
}

void panLeft()
{
  u_xn -= 3.0f;
  u_xp -= 3.0f;
  if(u_xn < WINDOW_LEFT){
    u_xn += 3.0f;
    u_xp += 3.0f;
  }
}

void panRight()
{
  u_xn += 3.0f;
  u_xp += 3.0f;
  if(u_xp > WINDOW_RIGHT){
    u_xn -= 3.0f;
    u_xp -= 3.0f;
  }
}

void panUp()
{
  u_yn += 1.5f;
  u_yp += 1.5f;
  if(u_yp > WINDOW_UP){
    u_yn -= 1.5f;
    u_yp -= 1.5f;
  }
}

void panDown()
{
  u_yn -= 1.5f;
  u_yp -= 1.5f;
  if(u_yn < WINDOW_DOWN){
    u_yn += 1.5f;
    u_yp += 1.5f;
  }
}

void zoomin()
{
  u_xn += 3.0f;
  u_xp -= 3.0f;
  if(u_xp - u_xn < 232.0f)
  {
    u_xn -= 3.0f;
    u_xp += 3.0f;
  }
  u_yn += 1.5f;
  u_yp -= 1.5f;
  fontScaleValue -= (48.5 / 154.0);
  fontx += (330.0 / 154.0);
  fonty -= (200.0 / 154.0);
  if(u_yp - u_yn < 116.0f)
  {
    fontScaleValue += (48.5 / 154.0); 
    fontx -= (330.0 / 154.0);
    fonty += (200.0 / 154.0);
    u_yn -= 1.5f;
    u_yp += 1.5f;
  }
}

void zoomout()
{
  u_xn -= 3.0f;
  fontScaleValue += (48.5 / 154.0);
  fontx -= (330.0 / 154.0);
  fonty += (200.0 / 154.0);
  if(u_xn < WINDOW_LEFT){
    u_xn = WINDOW_LEFT;
    fontScaleValue -= (48.5 / 154.0); 
    fontx += (330.0 / 154.0);
    fonty -= (200.0 / 154.0);
  }
  u_xp += 3.0f;
  if(u_xp > WINDOW_RIGHT)
    u_xp = WINDOW_RIGHT;
  u_yn -= 1.5f;
  if(u_yn < WINDOW_DOWN)
    u_yn = WINDOW_DOWN;
  u_yp += 1.5f;
  if(u_yp > WINDOW_UP)
    u_yp = WINDOW_UP;
  
}

/**************************
 * Customizable functions *
 **************************/



/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

  GLfloat fov = 90.0f;

  // sets the viewport of openGL renderer
  glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

  // set the projection matrix as perspective
  /* glMatrixMode (GL_PROJECTION);
     glLoadIdentity ();
     gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
  // Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(u_xn, u_xp, u_yn, u_yp, 0.1f, 500.0f);
}

class Circle
{
  private:
    int m_numSegments, m_degrees;
    VAO *m_circle;
    float m_radius, m_x, m_y;
    GLfloat m_vertex_buffer_data[MAX_SEGMENTS * 9];
    GLfloat  m_color_buffer_data[MAX_SEGMENTS * 9];

  public:
    Circle(float radius, float x, float y, int degrees);
    Circle(Circle &C);
    void createCircle(GLfloat red, GLfloat green, GLfloat blue);
    void changeCenter(float x, float y);
    void changeRadius(float radius);
    void draw();
};

Circle::Circle(float radius, float x, float y, int degrees = 360)
{
  m_numSegments = 360;
  m_degrees = 3 * degrees + 3;
  m_radius = radius;
  m_x = x;
  m_y = y;
  m_circle = NULL;
}

Circle::Circle(Circle &C)
{
  m_numSegments = C.m_numSegments;
  m_degrees = C.m_degrees;
  m_radius = C.m_radius;
  m_x = C.m_x;
  m_y = C.m_y;
  m_circle = C.m_circle;
  for(int i = 0; i < MAX_SEGMENTS * 9; i++)
  {
    m_vertex_buffer_data[i] = C.m_vertex_buffer_data[i];
    m_color_buffer_data[i] = C.m_color_buffer_data[i];
  }
}


void Circle::createCircle(GLfloat red, GLfloat green, GLfloat blue)
{
  if(m_circle != NULL)
    delete m_circle;
  float theta;
  for(int i = 0; i <= m_numSegments; i++)
  {
    theta = 2.0 * M_PI * float(i) / float(m_numSegments);
    m_vertex_buffer_data[9*i + 0] = 0;
    m_vertex_buffer_data[9*i + 1] = 0;
    m_vertex_buffer_data[9*i + 2] = 0;

    if(i == 0)
    {
      m_vertex_buffer_data[9*i + 3] = m_radius;
      m_vertex_buffer_data[9*i + 4] = 0;
      m_vertex_buffer_data[9*i + 5] = 0;
    }

    else
    {
      m_vertex_buffer_data[9*i + 3] = m_vertex_buffer_data[9*i - 3];
      m_vertex_buffer_data[9*i + 4] = m_vertex_buffer_data[9*i - 2];
      m_vertex_buffer_data[9*i + 5] = m_vertex_buffer_data[9*i - 1];
    }

    m_vertex_buffer_data[9*i + 6] = m_radius * cosf(theta);
    m_vertex_buffer_data[9*i + 7] = m_radius * sinf(theta);
    m_vertex_buffer_data[9*i + 8] = 0;

    m_color_buffer_data[9*i + 0] = red + 0.2f;
    m_color_buffer_data[9*i + 1] = green + 0.2f;
    m_color_buffer_data[9*i + 2] = blue + 0.2f;


    for(int j = 3; j < 9; j++)
    {
      //Adding color
      int k = j % 3;
      switch(k)
      {
        case 0:
          m_color_buffer_data[9*i + j] = red;
          break;
        case 1:
          m_color_buffer_data[9*i + j] = green;
          break;
        case 2:
          m_color_buffer_data[9*i + j] = blue;
          break;
      }
    }
  }

  m_circle = create3DObject(GL_TRIANGLES, m_degrees, m_vertex_buffer_data, m_color_buffer_data, GL_FILL);
}

void Circle::changeRadius(float radius)
{
  m_radius = radius;
}

void Circle::changeCenter(float x, float y)
{
  m_x = x;
  m_y = y;
}

void Circle::draw()
{
  glm::mat4 VP = Matrices.projection * Matrices.view;
  glm::mat4 MVP; 

  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateCircle = glm::translate (glm::vec3(m_x, m_y, 0.0f)); // glTranslatef
  glm::mat4 circleTransform = translateCircle;
  Matrices.model *= circleTransform; 
  MVP = VP * Matrices.model;

  //  Don't change unless you are sure!!
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  draw3DObject(m_circle);

}

class Rectangle
{
  private:
    float m_x, m_y, m_length, m_width, m_angle, m_rotationspeed;
    float m_minangle, m_maxangle;
    VAO *m_rectangle;
    GLfloat m_vertex_buffer_data[3 * rectangleVertex];
    GLfloat  m_color_buffer_data[3 * rectangleVertex];

  public:
    Rectangle(float x, float y, float m_length, float width, float angle, float rotationspeed);
    Rectangle(Rectangle &R);
    void createRectangle(float red, float green, float blue);
    void changeAngle(float angle);
    void changeBoundAngle(float minangle, float maxangle);
    void rotateRectangleClock();
    void rotateRectangleAntiClock();
    void changeLength(float length) { m_length = length; }
    void changeY(float y) { m_y = y; }
    float getLength(){ return m_length; }
    float getAngle(){ return m_angle; }
    void draw();

};

Rectangle::Rectangle(float x, float y, float length, float width, float angle = 0, float rotationspeed = 2.0f)
{
  m_x = x;
  m_y = y;
  m_length = length;
  m_width = width;
  m_angle = angle;
  m_rotationspeed = rotationspeed;
  m_minangle =   0;
  m_maxangle = 180;
  m_rectangle = NULL;
}

Rectangle::Rectangle(Rectangle &R)
{
  m_x = R.m_x;
  m_y = R.m_y;
  m_length = R.m_length;
  m_width = R.m_width;
  m_angle = R.m_angle;
  m_rotationspeed = R.m_rotationspeed;
  m_minangle = R.m_minangle;
  m_maxangle = R.m_maxangle;
  m_rectangle = R.m_rectangle;
  for(int i = 0; i < 3 * rectangleVertex; i++)
  {
    m_vertex_buffer_data[i] = R.m_vertex_buffer_data[i];
    m_color_buffer_data[i] = R.m_color_buffer_data[i];
  }
  m_rectangle = NULL;
}

void Rectangle::createRectangle(float red = 1.0f, float green = 0.0f, float blue = 0.0f)
{
  if(m_rectangle != NULL)
    delete m_rectangle;
  for(int i = 0; i < 3 * rectangleVertex; i++)
  {
    if(i == 3 || i == 6 || i == 9)
      m_vertex_buffer_data[i] = m_length;

    else if(i == 7 || i == 10 || i == 13)
      m_vertex_buffer_data[i] = -m_width;

    else
      m_vertex_buffer_data[i] = 0;
  }

  for(int i = 0; i < rectangleVertex; i++)
  {
    m_color_buffer_data[3*i + 0] = red;
    m_color_buffer_data[3*i + 1] = green;
    m_color_buffer_data[3*i + 2] = blue;
  }

  m_rectangle = create3DObject(GL_TRIANGLES, rectangleVertex, m_vertex_buffer_data, m_color_buffer_data, GL_FILL);
}

void Rectangle::changeAngle(float angle)
{
  m_angle = angle;
}

void Rectangle::rotateRectangleClock()
{
  if( m_angle - m_rotationspeed < m_minangle )
    m_angle = m_minangle;
  else
    m_angle -= m_rotationspeed;
}

void Rectangle::rotateRectangleAntiClock()
{
  if( m_angle + m_rotationspeed > m_maxangle )
    m_angle = m_maxangle;
  else
    m_angle += m_rotationspeed;
}

void Rectangle::changeBoundAngle(float minangle, float maxangle)
{
  m_maxangle = maxangle;
  m_minangle = minangle;
}

void Rectangle::draw()
{
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;  // MVP = Projection * View * Model
  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translateRectangle = glm::translate (glm::vec3( m_x, m_y, 0));        // glTranslatef
  glm::mat4 rotateRectangle = glm::rotate((float)( m_angle * M_PI / 180.0f ), glm::vec3(0, 0, 1)); // rotate about vector (-1,1,1)
  Matrices.model *= (rotateRectangle * translateRectangle );
  // Matrices.model *= translateRectangle;
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(m_rectangle);
}

class Cannon
{
  private:
    Circle *wheel;
    Rectangle *barrel, *power;
    float m_velocity;

  public:
    Cannon();
    void createCannon();
    void drawCannon();
    void barrelUp();
    void barrelDown();
    void changelBarrelangle(float angle) 
    { 
      barrel->changeAngle(angle); 
      power->changeAngle(angle); 
    }
    float getLength(){ return barrel->getLength(); }
    float getAngle(){ return barrel->getAngle(); }
    float getVelocity(){ return m_velocity; }
    void increasePower(float);
    void decreasePower();
    ~Cannon();

};

Cannon::Cannon()
{
  wheel = new Circle(21.0f, 0.0f, 0.0f);
  barrel = new Rectangle( -13.0f, 15.0f, 65.0f, 28.0f, 45 );
  barrel->changeBoundAngle(0, 180);
  m_velocity = 80;
  power = new Rectangle (-13.0f, 15.0f, 35.0f, 28.0f, 45);
  power->changeBoundAngle(0, 180);
}

void Cannon::increasePower(float velocity = 0)
{
  if(velocity != 0){
    m_velocity = velocity;
    float l_temp = 35.0f + (velocity -80)/4;
    power->changeLength(l_temp);
    power->createRectangle(0.0, 0.0, 0.0);
    return;
  }

  if(m_velocity == 200)
    return;
  m_velocity += 4;
  power->changeLength(power->getLength() + 1);
  power->createRectangle(0.0, 0.0, 0.0);
}

void Cannon::decreasePower()
{
  if(m_velocity == 80)
    return;
  m_velocity -= 4;
  power->changeLength(power->getLength() - 1);
  power->createRectangle(0.0, 0.0, 0.0);
}

void Cannon::createCannon()
{
  wheel->createCircle( GREY );
  barrel->createRectangle();
  power->createRectangle(0.0, 0.0, 0.0);
}

void Cannon::drawCannon()
{
  barrel->draw();
  power->draw();
  wheel->draw();
}

void Cannon::barrelUp()
{
  barrel->rotateRectangleAntiClock();
  power->rotateRectangleAntiClock();
}

void Cannon::barrelDown()
{
  barrel->rotateRectangleClock();
  power->rotateRectangleClock();
}

Cannon::~Cannon()
{
  delete wheel;
  delete barrel;
  delete power;
}

class Shot
{
  private:
    int m_air;
    float m_x_velocity, m_y_velocity, m_angle, m_ground, m_rightbound;
    float m_x, m_y, m_radius;
    Circle *m_shot;


  public:
    float m_mass;
    Shot(float x, float y, float velocity, float angle, float ground, float rightbound);
    void createShot();
    void motion(float timeSlice);
    void drawShot();
    float getX(){ return m_x; }
    float getXV(){ return m_x_velocity; }
    float getY(){ return m_y; }
    float getYV(){ return m_y_velocity; }
    float getRadius(){ return m_radius; }
    void setVelocity(float x_v, float y_v){ m_x_velocity = x_v; m_y_velocity = y_v; }
    void changeBoundary(float, float, float);
    void setPosition(float x, float y){ m_x = x; m_y = y; }
    ~Shot();
};

Shot::Shot(float x, float y, float velocity, float angle, float ground = -8.0f, float rightbound = u_xp)
{
  m_x = x;
  m_y = y;
  m_mass = 1;
  m_radius = 13.0f;
  m_shot = new Circle(m_radius, m_x, m_y);
  m_angle = angle * M_PI / 180.0;
  m_x_velocity = velocity * cosf(m_angle);
  m_y_velocity = velocity * sinf(m_angle);
  m_ground = ground;
  m_rightbound = rightbound;
  m_air = 1;
}

Shot::~Shot()
{
  delete m_shot;
}

void Shot::createShot()
{
  m_shot->createCircle(0.0f, 0.0f, 1.0f);
}

void Shot::drawShot()
{
  m_shot->draw();
}

void Shot::motion(float timeSlice)
{
  if(m_air != 1)
    return;
  float x_movement, y_movement, theta;
  if(m_y != m_ground )
  {
    y_movement = (m_y_velocity * timeSlice) - (g * timeSlice * timeSlice / 2.0);
    m_y += y_movement;

    if(fabs(m_y_velocity) < 0.01f && m_y - m_ground < 0.01f)
    {
      m_y = m_ground;
      m_y_velocity = 0;
    }

    else if( m_y - m_ground < 0.01f && !(y_movement > 0))
    {
      m_y_velocity = -1 * m_y_velocity * 0.6;

    }

    else
      m_y_velocity -= g * timeSlice;

  }

  if(m_x_velocity != 0)
  {
    if(m_y - m_ground < 0.01f)
    {
      m_x_velocity *= 0.6;
      if( fabs(m_x_velocity) < 0.01f )
        m_x_velocity = 0;
    }
    m_x += m_x_velocity * timeSlice;
    
  }

  m_shot->changeCenter(m_x, m_y);
}

void Shot::changeBoundary(float bound, float lbound, float length)
{
  if( m_x + m_radius >= lbound && m_x - m_radius <= lbound + length ){
    m_ground = bound + m_radius + 3.0f;
    m_x_velocity >= 0 ? m_x_velocity+=0.1f : m_x_velocity -= 0.1f;
  }
  else{
    m_ground = -8.0f;
    return;
  }
  if(m_y < m_ground)
    m_y = m_ground +0.00001f;
}

class Target
{
  private:
    int m_alive, m_disappear, m_done;
    float m_x, m_y, m_radius, m_angle;
    float m_boundx1, m_boundx2, m_boundy1, m_boundy2;
    float m_x_velocity, m_y_velocity, m_mass;
    Circle *m_target;

  public:
    Target(float x, float y, float radius, float velocity, float angle, int disappear);
    void createTarget(float, float, float);
    void motion(float);
    void destroy();
    void drawTarget();
    void changeBoundary(float);
    int checkCollision(Shot *temp);
    ~Target();
};

Target::Target(float x, float y, float radius = 20.0f, float velocity = 0.0, float angle = 0.0, int disappear = 1)
{
  m_x = x;
  m_done = 0;
  m_mass = 2;
  m_y = y;
  m_boundx1 = x - radius;
  m_boundx2 = x + radius;
  m_radius = radius;
  m_target = new Circle( m_radius, m_x, m_y );
  m_alive = 1;
  m_disappear = disappear;
  m_angle = angle * M_PI / 180.0;
  m_x_velocity = velocity * cosf( m_angle );
  m_y_velocity = velocity * sinf( m_angle );
}

void Target::changeBoundary(float bound)
{
  if( m_x - m_radius >= m_boundx1 && m_x - m_radius <= m_boundx2 )
    m_boundy1 = bound + m_radius + 3.0f;
  else{
    m_boundy1 = -1.0f;
    return;
  }
  if(m_y < m_boundy1)
    m_y = m_boundy1 ;
}

void Target::createTarget(float red = -1, float green = -1, float blue = -1)
{
  if( red < 0 || green < 0 || blue < 0 )
    m_target->createCircle( YELLOW );
  else
    m_target->createCircle( red, green, blue );
}

void Target::destroy()
{
  m_alive = 0;
}

int Target::checkCollision( Shot *temp )
{
  float distance = sqrt( (temp->getX() - m_x) * (temp->getX() - m_x) + ( temp->getY() - m_y ) * ( temp->getY() - m_y ) );
  float offset = m_y > m_x ? m_y * 0.01 : m_x*0.01;
  if( ( distance < temp->getRadius() + m_radius ) && m_disappear == 1)
  {   
      destroy();
      return 1;
        
  }
  else if ( distance < temp->getRadius() + m_radius + 5.0f)
  // else
    {
      float x2 = temp->getX();
      float xv = temp->getXV()* 0.01;
      float y2 = temp->getY();
      float yv = temp->getYV()* 0.01;

      m_x_velocity *= 0.01;
      m_y_velocity *= 0.01;
      
      float cx = m_x - x2;
      float cy = m_y - y2;

      float unitx = cx/distance;
      float unity = cy/distance;

      float firstInitComp = m_x_velocity * unitx + m_y_velocity * unity;
      float secondInitComp = xv * unitx + yv * unity;

      float firstFinalComp = firstInitComp*(m_mass - temp->m_mass) + 2*temp->m_mass*secondInitComp/(m_mass + temp->m_mass);
      float secondFinalComp = secondInitComp*(temp->m_mass - m_mass) + 2*m_mass*firstInitComp/(m_mass + temp->m_mass);

      float firstChange = firstFinalComp - firstInitComp;
      float secondChange = secondFinalComp - secondInitComp;

      m_x_velocity += firstChange * unitx;
      m_y_velocity += firstChange * unity;

      

      xv += secondChange * unitx;
      yv += secondChange * unity;

      float magFirstChangeX = (firstChange * unitx) > 0.0f ? (firstChange * unitx) : (-1.0f * firstChange * unitx);
      float magFirstChangeY = (firstChange * unity) > 0.0f ? (firstChange * unity) : (-1.0f * firstChange * unity);
      
      float magSecondChangeX = (secondChange * unitx) > 0.0f ? (secondChange * unitx) : (-1.0f * secondChange * unitx);
      float magSecondChangeY = (secondChange * unity) > 0.0f ? (secondChange * unity) : (-1.0f * secondChange * unity);

      if(true){
              m_x += (firstChange * unitx)/magFirstChangeX * 1.0f;
              m_y += (firstChange * unity)/magSecondChangeX * 1.0f;
              x2 += (secondChange * unitx)/magSecondChangeX * 1.0f;
              y2 += (secondChange * unity)/magSecondChangeY * 1.0f;
      }

      m_x_velocity /= 0.01;
      m_y_velocity /= 0.01;
      xv /=0.01;
      yv /=0.01;

      if(fabs(m_x_velocity) > 200 || fabs(m_y_velocity) > 200)
      {
        if(!buffer.openFromFile("explode.wav"))
          cout<<"Failed";
        buffer.play();
        if(fabs(m_x_velocity < 200))
          m_x_velocity = 200;

        createTarget(RED);
        if(m_done == 0){
          score += 500;
          m_done = 1;
          mainTargets--;
        }
      }
      else
      {
        if(!buffer.openFromFile("collide.wav"))
          cout<<"Failed";
        buffer.play();
      } 

      // cout<<m_x_velocity<<" "<<m_y_velocity<<endl;
      temp->setPosition(x2, y2);
      temp->setVelocity(xv, yv);
      return 1;
    }
  return 0;
}

void Target::motion(float timeSlice)
{
  float x_movement, y_movement;
  if(m_y != m_boundy1 )
  {
    y_movement = (m_y_velocity * timeSlice) - (g * timeSlice * timeSlice / 2.0);
    m_y += y_movement;

    if(fabs(m_y_velocity) < 0.01f && m_y - m_boundy1 < 0.01f)
    {
      m_y = m_boundy1;
      m_y_velocity = 0;
    }

    else if( m_y - m_boundy1 < 0.01f && !(y_movement > 0))
    {
      m_y_velocity = -1 * m_y_velocity * 0.6;

    }

    else
      m_y_velocity -= g * timeSlice;

  }

  if(m_x_velocity != 0)
  {
    if(m_y - m_boundy1 < 0.01f)
    {
      m_x_velocity *= 0.9;
      if( fabs(m_x_velocity) < 0.01f )
        m_x_velocity = 0;
    }
    m_x += m_x_velocity * timeSlice;
    // cout<<m_x_velocity<<endl;
  }

  if(m_done == 0 && m_y <= 1.0f)
  {
    m_done = 1;
    score += 300;
    mainTargets--;
  }

  m_target->changeCenter(m_x, m_y);
}

void Target::drawTarget()
{
  if( m_alive == 0 )
    return;

  m_target->draw();
}

Target::~Target()
{
  delete m_target;
}

class Barrier
{
  int a;
  float m_velocity, m_x, m_y;
  float m_length, m_width;
  float m_bound1, m_bound2;
  Rectangle *m_bar;

  public:
    Barrier(float, float, int);
    void motion(float );
    void createBarrier();
    void drawBarrier();
    float getX() { return m_x; }
    float getY() { return m_y; }
    float getLength() { return m_length; }
    void checkCollision(Shot *);
    ~Barrier(){ delete m_bar; }

};

Barrier::Barrier(float x, float y, int velocity)
{
  m_velocity = 100 + ( velocity % 200 );
  m_x = x;
  m_y = y;
  m_length = 40;
  m_width = 900;
  m_bound1 = -20.0f;
  m_bound2 = 250.0f;
  m_bar = new Rectangle(m_x, m_y, m_length, m_width);

}

void Barrier::createBarrier()
{
  m_bar->createRectangle( 0.0f, 0.0f, 0.0f );
}

void Barrier::motion(float timeSlice)
{
  float y_movement = m_velocity * timeSlice;
  m_y += y_movement;
  if(m_y > m_bound2)
  {
    m_y = m_bound2;
    m_velocity *= -1;
  }
  if(m_y < m_bound1){
    m_velocity *= -1;
    m_y = m_bound1;
  }
  m_bar->changeY(m_y);
  createBarrier();
}

void Barrier::drawBarrier()
{
  m_bar->draw();
}

void Barrier::checkCollision(Shot *temp)
{
  float yv = temp->getYV();
  float xv = temp->getXV();
  if(temp->getX() + temp->getRadius() + xv*0.01 >= m_x && temp->getX() + temp->getRadius() + xv*0.01 <= m_x + m_length)
  {
    if(temp->getY() - temp->getRadius() < m_y)
    {
      yv *= 0.6;
      xv *= -0.6;
    }
  }
  else if(temp->getX() >= m_x + m_length && temp->getX() - temp->getRadius() + xv*0.01 <= m_length + m_x)
  {
    if(temp->getY() - temp->getRadius() < m_y)
    {
      yv *= 0.6;
      xv *= -0.6;
    }
  }
  temp->setVelocity(xv, yv);
}

// Rectangle r( -13.0f, 15.0f, 65.0f, 28.0f, 45 );
Cannon c;
vector<Shot *> bombs;
double xpos, ypos, xposold, yposold, aimx, aimy;
// Target coin(520.0f, 80.0f, 20.0 , 0.0, 0.0, 0);
int pan = 0; 
GLFWwindow* window;
// Barrier bar(500.0, 100.0);
vector<Barrier *> bars;
vector<Target *> roller;
float shootTime = 0;

void initObstacles()
{
  time_t t;
  srand((unsigned) time(&t));
  Barrier *tempbar = new Barrier(500.0, 100.0, rand());
  Target *temproller = new Target(520.0f, 80.0f, 20.0, 0.0, 0.0, 0);
  tempbar->createBarrier();
  temproller->createTarget();
  bars.push_back(tempbar);
  roller.push_back(temproller);

  tempbar = new Barrier(700.0, 100.0, rand());
  temproller = new Target(720.0f, 80.0f, 20.0, 0.0, 0.0, 0);
  tempbar->createBarrier();
  temproller->createTarget();
  bars.push_back(tempbar);
  roller.push_back(temproller);

  tempbar = new Barrier(300.0, 100.0, rand());
  temproller = new Target(320.0f, 80.0f, 20.0, 0.0, 0.0, 0);
  tempbar->createBarrier();
  temproller->createTarget();
  bars.push_back(tempbar);
  roller.push_back(temproller);
}

void drawObstacles()
{
    for(vector<Target *>::iterator it1 = roller.begin(); it1 != roller.end(); ++it1)
      (*it1)->drawTarget();
    for(vector<Barrier *>::iterator it1 = bars.begin(); it1 != bars.end(); ++it1)
      (*it1)->drawBarrier();
}

void shootBomb(float velocity = c.getVelocity())
{
  if(ballsLeft == 0)
    return;
  score -= 200;
  ballsLeft--;
  float x, y;
  x = c.getLength() * cosf( c.getAngle() * M_PI / 180.0f );
  y = c.getLength() * sinf( c.getAngle() * M_PI / 180.0f );
  Shot *tempBomb = new Shot( x, y, velocity, c.getAngle() );
  tempBomb->createShot();
  bombs.push_back(tempBomb);
  shootTime = glfwGetTime();
  if(!buffer.openFromFile("fire.wav"))
    cout<<"Failed";
  buffer.play();
}

void mouseShoot()
{
  // cout<<aimx<<" "<<aimy<<endl;
  int winx, winy;
  glfwGetWindowSize(window, &winx, &winy);
  // cout<<winx<<" "<<winy<<endl;
  aimy = winy - aimy;
  float slope = atan2(aimy, aimx) * 180.0f / M_PI;
  float cursorY = (u_yp - u_yn) * aimy/(float)winy;
  cursorY += u_yn;
  float cursorX = (u_xp - u_xn) * aimx/(float)winx;
  cursorX += u_xn;
  c.changelBarrelangle(slope);
  int distance = (int)sqrt((cursorX * cursorX) + (cursorY * cursorY));
  float velocity = 80;
  if(distance > 340)
    velocity = 200;
  else if(distance < 100)
    velocity = 80;
  else
  {
    distance -= 100;
    distance /= 2;
    velocity += distance;
  }
  c.increasePower(velocity);
  shootBomb();
}

void drawBomb()
{
  for(vector<Shot *>::iterator it = bombs.begin(); it != bombs.end(); ++it )
  {
    (*it)->drawShot();
  }
}

void motion(float timeSlice)
{
  vector<Barrier *>::iterator it2 = bars.begin();
  for(vector<Target *>::iterator it = roller.begin(); it != roller.end() && it2 != bars.end(); ++it, ++it2 )
  {
    (*it)->changeBoundary((*it2)->getY());
  }
  // coin.changeBoundary(bar.getY());
  for(vector<Shot *>::iterator it = bombs.begin(); it != bombs.end(); ++it )
  {
    (*it)->motion(timeSlice);
    // coin.checkCollision((*it));
    for(vector<Target *>::iterator it1 = roller.begin(); it1 != roller.end(); ++it1)
      (*it1)->checkCollision((*it));
    for(vector<Barrier *>::iterator it1 = bars.begin(); it1 != bars.end(); ++it1){
      (*it1)->checkCollision((*it));
      (*it)->changeBoundary((*it1)->getY(), (*it1)->getX(), (*it1)->getLength());

    }
    // bar.checkCollision((*it));
  }
  for(vector<Target *>::iterator it1 = roller.begin(); it1 != roller.end(); ++it1)
    (*it1)->motion(timeSlice);
  for(vector<Barrier *>::iterator it1 = bars.begin(); it1 != bars.end(); ++it1)
    (*it1)->motion(timeSlice);
}

void mousespan()
{
  glfwGetCursorPos(window, &xpos, &ypos);
  double xratio = (u_xp - u_xn)/1300;
  double yratio = (u_yp - u_yn)/600;
  u_xp += (xposold - xpos) * xratio;
  u_xn += (xposold - xpos) * xratio;
  u_yp += (ypos - yposold) * yratio;
  u_yn += (ypos - yposold) * yratio;

  if(u_xn < WINDOW_LEFT){
    u_xn = WINDOW_LEFT;
    u_xp -= (xposold - xpos) * xratio;
  }
  if(u_xp > WINDOW_RIGHT){
    u_xp = WINDOW_RIGHT;
    u_xn -= (xposold - xpos) * xratio;
  }
  if(u_yn < WINDOW_DOWN){
    u_yn = WINDOW_DOWN;
    u_yp -= (ypos - yposold) * yratio;
  }
  if(u_yp > WINDOW_UP){
    u_yp = WINDOW_UP;
    u_yn -= (ypos - yposold) * yratio;
  }
  xposold = xpos;
  yposold = ypos;
  updateProjection();

}

vector<Rectangle *> world;
void createWorld()
{
  Rectangle *soil = new Rectangle( WINDOW_LEFT, WINDOW_DOWN, WINDOW_RIGHT - WINDOW_LEFT, u_yn - 29.0f, 0);
  soil->createRectangle( LIGHTBROWN ); 
  Rectangle *grass = new Rectangle( WINDOW_LEFT, u_yn + 19.0f, WINDOW_RIGHT - WINDOW_LEFT, -50.0f, 0);
  grass->createRectangle( SOILGREEN );
  Rectangle *horizon = new Rectangle( WINDOW_LEFT, u_yn + 69.0f, WINDOW_RIGHT - WINDOW_LEFT, - 250.0f, 0);
  horizon->createRectangle( SKYBLUE );
  Rectangle *space = new Rectangle( WINDOW_LEFT, WINDOW_UP, WINDOW_RIGHT - WINDOW_LEFT, 300.0f, 0);
  space->createRectangle( SPACEBLUE );

  world.push_back(soil);
  world.push_back(grass);
  world.push_back(horizon);
  world.push_back(space);
  return;
}

void drawWorld()
{
  for(vector<Rectangle *>::iterator it = world.begin(); it != world.end(); ++it )
  {
    (*it)->draw();
  }
}

void resetInit();

/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) 
    {
      switch (key) 
      {
        case GLFW_KEY_F:
          c.increasePower();
          break;
        case GLFW_KEY_S:
          c.decreasePower();
          break;
        case GLFW_KEY_A:
          c.barrelUp();
          break;
        case GLFW_KEY_B:
          c.barrelDown();
          break;
        case GLFW_KEY_UP:
          zoomin();
          updateProjection();
          break;
        case GLFW_KEY_DOWN:
          zoomout();
          updateProjection();
          break;
        case GLFW_KEY_R:
          resetInit();
          break;
        case GLFW_KEY_SPACE:
          shootBomb();
          break;
        case GLFW_KEY_I:
          panUp();
          updateProjection();
          break;
        case GLFW_KEY_K:
          panDown();
          updateProjection();
          break;
        case GLFW_KEY_J:
        case GLFW_KEY_LEFT:
          panLeft();
          updateProjection();
          break;
        case GLFW_KEY_L:
        case GLFW_KEY_RIGHT:
          panRight();
          updateProjection();
          break;
        default:
          break;
      }
        
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
    else if(action == GLFW_REPEAT)
    {
      switch(key)
      {
        case GLFW_KEY_F:
          c.increasePower();
          break;
        case GLFW_KEY_S:
          c.decreasePower();
          break;
        case GLFW_KEY_UP:
          zoomin();
          updateProjection();
          break;
        case GLFW_KEY_DOWN:
          zoomout();
          updateProjection();
          break;
        case GLFW_KEY_I:
          panUp();
          updateProjection();
          break;
        case GLFW_KEY_K:
          panDown();
          updateProjection();
          break;
        case GLFW_KEY_LEFT:
        case GLFW_KEY_J:
          panLeft();
          updateProjection();
          break;
        case GLFW_KEY_RIGHT:
        case GLFW_KEY_L:
          panRight();
          updateProjection();
          break;

        case GLFW_KEY_A:
          c.barrelUp();
          break;
        case GLFW_KEY_B:
          c.barrelDown();
          break;
      }
    }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
  switch (key) {
    case 'Q':
    case 'q':
            quit(window);
            break;
    default:
      break;
  }
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
    switch (button) 
    {
        case GLFW_MOUSE_BUTTON_LEFT:

            if (action == GLFW_RELEASE)
            {
              glfwGetCursorPos(window, &aimx, &aimy);
              mouseShoot();
              break;
            }
            else
              break;
                
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            if (action == GLFW_PRESS) 
            {
              glfwGetCursorPos(window, &xpos, &ypos);
              mousespan();
              pan = 1;
              break;
            }
            else if( action == GLFW_RELEASE )
            {
              pan = 0;
              break;
            }
            break;
        default:
            break;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  if(yoffset > 0)
    zoomin();
  else
    zoomout();
  updateProjection();
}

float camera_rotation_angle = 90;

void drawBasics()
{
  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5 * cos(camera_rotation_angle * M_PI / 180.0f), 0, 5 * sin(camera_rotation_angle * M_PI / 180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0, 0, 3), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)); // Fixed camera for 2D (ortho) in XY plane
}

void drawFont()
{
  glm::mat4 MVP;
  static int fontScale = 0;
  glm::vec3 fontColor = getRGBfromHue (fontScale);

  // Use font Shaders for next part of code
  glUseProgram(fontProgramID);
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Transform the text
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translateText = glm::translate(glm::vec3(fontx, fonty, 0));
  glm::mat4 scaleText = glm::scale(glm::vec3(fontScaleValue, fontScaleValue, fontScaleValue));
  Matrices.model *= (translateText * scaleText);
  MVP = Matrices.projection * Matrices.view * Matrices.model;
  // send font's MVP and font color to fond shaders
  glUniformMatrix4fv(GL3Font.fontMatrixID, 1, GL_FALSE, &MVP[0][0]);
  glUniform3fv(GL3Font.fontColorID, 1, &fontColor[0]);

  // Render font
  GL3Font.font->Render(scoreboard);

  // font size and color changes
  fontScale = (fontScale + 1) % 360;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Cannon Fire", NULL, NULL);

    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks

    glfwSetScrollCallback(window, scroll_callback);

    return window;
}

/* Initialize the OpenGL rendering properties */
/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
  // Create the models
  // createTriangle (); // Generate the VAO, VBOs, vertices data & copy into the array buffer
  // createRectangle ();
  
  // Create and compile our GLSL program from the shaders
  programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
  // Get a handle for our "MVP" uniform
  Matrices.MatrixID = glGetUniformLocation(programID, "MVP");
  // GLuint image = loadBMP_custom("./my_texture.bmp");s

  
  reshapeWindow (window, width, height);

    // Background color of the scene
  glClearColor (1.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
  glClearDepth (1.0f);

  glEnable (GL_DEPTH_TEST);
  glDepthFunc (GL_LEQUAL);

  const char* fontfile = "bow.ttf";
  GL3Font.font = new FTExtrudeFont(fontfile); // 3D extrude style rendering

  if(GL3Font.font->Error())
  {
    cout << "Error: Could not load font `" << fontfile << "'" << endl;
    glfwTerminate();
    exit(EXIT_FAILURE);
  }

  // Create and compile our GLSL program from the font shaders
  fontProgramID = LoadShaders( "fontrender.vert", "fontrender.frag" );
  GLint fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform;
  fontVertexCoordAttrib = glGetAttribLocation(fontProgramID, "vertexPosition");
  fontVertexNormalAttrib = glGetAttribLocation(fontProgramID, "vertexNormal");
  fontVertexOffsetUniform = glGetUniformLocation(fontProgramID, "pen");
  GL3Font.fontMatrixID = glGetUniformLocation(fontProgramID, "MVP");
  GL3Font.fontColorID = glGetUniformLocation(fontProgramID, "fontColor");

  GL3Font.font->ShaderLocations(fontVertexCoordAttrib, fontVertexNormalAttrib, fontVertexOffsetUniform);
  GL3Font.font->FaceSize(1);
  GL3Font.font->Depth(0);
  GL3Font.font->Outset(0, 0);
  GL3Font.font->CharMap(ft_encoding_unicode);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

void resetInit()
{
  fontScaleValue = 49.607142; fontx = 100.7142857; fonty = 407.1428571;
  ballsLeft = 25; score = 6000; timeLeft = 100; mainTargets = 3;
  u_xn = WINDOW_LEFT + 100.0f;
  u_xp = WINDOW_RIGHT - 100.0f;
  u_yn = WINDOW_DOWN + 50.0f;
  u_yp = WINDOW_UP - 50.0f;

  for(vector<Target *>::reverse_iterator it1 = roller.rbegin(); it1 != roller.rend(); ++it1){
    Target *temp = (*it1);
    delete temp;
    roller.pop_back();
  }
  for(vector<Barrier *>::reverse_iterator it1 = bars.rbegin(); it1 != bars.rend(); ++it1){
    Barrier *temp = (*it1);
    delete temp;
    bars.pop_back();
  }
  for(vector<Shot *>::reverse_iterator it1 = bombs.rbegin(); it1 != bombs.rend(); ++it1){
    Shot *temp = (*it1);
    delete temp;
    bombs.pop_back();
  }
  initObstacles();
}


int main (int argc, char** argv)
{
  int width = 1300;
  int height = 600;

  window = initGLFW(width, height);

  createWorld();
  c.createCannon();
  initObstacles();

  initGL (window, width, height);

  double last_update_time = glfwGetTime(), current_time;
  double last_time = glfwGetTime(), cur_time;
  sprintf(scoreboard, "Balls: %d  Score: %d Time: %d", ballsLeft, score, timeLeft);

    /* Draw in loop */
  while (!glfwWindowShouldClose(window)) 
  {
    // OpenGL Draw commands
    drawBasics();
    drawWorld();

    drawBomb();
    c.drawCannon();
    drawObstacles();
    drawFont();

    // Swap Frame Buffer in double buffering
    glfwSwapBuffers(window);

    // Poll for Keyboard and mouse events
    glfwPollEvents();

    if(pan == 1)
        mousespan();
    else
      glfwGetCursorPos(window, &xposold, &yposold);

    // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
    current_time = glfwGetTime(); // Time in seconds
    cur_time = glfwGetTime();
    if ((current_time - last_update_time) >= 0.01) 
    { 
      motion(0.01);
      // atleast 0.01s elapsed since last frame
      // do something every 0.01 seconds ..
      last_update_time = current_time;
    }

    if((cur_time - last_time) >= 1)
    {
      if(mainTargets != 0)
        score -= 10;
      last_time = cur_time;
      timeLeft--;
    }

    if(timeLeft == 0)
    {
      strcpy(scoreboard, "You Lost! Press R to restart!");
      continue;
    }

    if(mainTargets == 0)
    {
      sprintf(scoreboard, "You Won! Score: %d! R to restart!", score);
      continue;
    }

    if(ballsLeft == 0)
    {
      if(mainTargets == 0)
      {
        sprintf(scoreboard, "You Won! Score: %d! R to restart!", score);
        continue;
      }
      if(glfwGetTime() - shootTime >= 15){
        strcpy(scoreboard, "You Lost! Press R to restart!");
        continue;
      }

    }

    if(sprintf(scoreboard, "Balls: %d  Score: %d  Time: %d", ballsLeft, score, timeLeft) < 0)
      exit(EXIT_FAILURE);

  }

  glfwTerminate();
  exit(EXIT_SUCCESS);
}
