/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "xbmc_scr_dll.h"
#if defined(HAS_GLES)
#include <GLES2/gl2.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <assert.h>
#define TO_STRING(...) #__VA_ARGS__
#else
#include <GL/glew.h>
#endif
#include <iostream>
#include <algorithm>
#include <string>
#include <fstream>
#include <streambuf>
#include <ctime>
#include <p8-platform/util/timeutils.h>
#include <math.h>
#include <complex.h>
#include <limits.h>
#include <fstream>
#include <sstream>

#include "lodepng.h"

using namespace std;

string g_pathPresets;

struct Preset {
  std::string name;
  std::string file;
  int channel[4];
};

const std::vector<Preset> g_presets =
  {
   {"Ball",                            "ball.frag.glsl",                 99,-1,-1,-1},
   {"Basic Fractal",                   "basicfractal.frag.glsl",         -1,-1,-1,-1},
   {"Bleepy Blocks",                   "bleepyblocks.frag.glsl",         99,-1,-1,-1},
   {"Cellular",                        "cellular.frag.glsl",             -1,-1,-1,-1},
   {"Coloured Circles",                "colorcircles.frag.glsl",         -1,-1,-1,-1},
   {"Deform Flower",                   "deformflower.frag.glsl",         -1,-1,-1,-1},
   {"Dot Dot Dot",                     "dotdotdot.frag.glsl",            -1,-1,-1,-1},
   {"Fast Clouds",                     "fastclouds.frag.glsl",           12,-1,-1,-1},
   {"Fireball",                        "fireball.frag.glsl",             14,-1,-1,-1},
   {"Flaring",                         "flaring.frag.glsl",              12,-1,-1,-1},
   {"Fractal Tiling",                  "fractaltiling.frag.glsl",        -1,-1,-1,-1},
   {"Glowing Thing",                   "glowingthing.frag.glsl",         -1,-1,-1,-1},
   {"Infinite Fall",                   "infinitefall.frag.glsl",         98,-1,-1,-1},
   {"Input Time",                      "inputtime.frag.glsl",            -1,-1,-1,-1},
   {"Interstellar",                    "stellar.frag.glsl",              14,-1,-1,-1},
   {"Interweaving Sine bands",         "sinebands.frag.glsl",            -1,-1,-1,-1},
   {"Juliasm",                         "juliasm.frag.glsl",              -1,-1,-1,-1},
   {"Julia Trap",                      "juliatrap.frag.glsl",            -1,-1,-1,-1},
   {"Noise",                           "noise.frag.glsl",                -1,-1,-1,-1},
   {"Noise Animation Electric",        "noiseanimelectric.frag.glsl",    12,-1,-1,-1},
   {"Noise Animation Lava",            "noiseanimlava.frag.glsl",        12,-1,-1,-1},
   {"Noise Animation Watery",          "noiseanimwatery.frag.glsl",      12,-1,-1,-1},
   {"Plasma Triangle",                 "plasmatriangle.frag.glsl",       11,-1,-1,-1},
   {"Plasma2",                         "plasma2.frag.glsl",              -1,-1,-1,-1},
   {"Silexars Creation",               "silexarst.frag.glsl",            -1,-1,-1,-1},
   {"Simple Fire",                     "simplefire.frag.glsl",           -1,-1,-1,-1},
   {"Sky at Night",                    "skyatnight.frag.glsl",           -1,-1,-1,-1},
   {"Spiral",                          "spiral.frag.glsl",               -1,-1,-1,-1},
   {"Star Tunnel",                     "startunnel.frag.glsl",           -1,-1,-1,-1},
   {"Venus",                           "venus.frag.glsl",                14,-1,-1,-1},
   {"Warp",                            "warp.frag.glsl",                 99,-1,-1,-1},
   {"Water Caustic",                   "watercaustic.frag.glsl",         -1,-1,-1,-1},
  };
int g_currentPreset = 0;
char** lpresets = nullptr;

const char *g_fileTextures[] = {
  "tex00.png",
  "tex01.png",
  "tex02.png",
  "tex03.png",
  "tex04.png",
  "tex05.png",
  "tex06.png",
  "tex07.png",
  "tex08.png",
  "tex09.png",
  "tex10.png",
  "tex11.png",
  "tex12.png",
  "tex15.png",
  "tex16.png",
  "tex14.png",
};

#if defined(HAS_GLES)
struct
{
  GLuint vertex_buffer;
  GLuint attr_vertex_e;
  GLuint attr_vertex_r, uTexture;
  GLuint effect_fb;
  GLuint framebuffer_texture;
  GLuint render_program;
  GLuint uScale;
  int fbwidth, fbheight;
  GLint viewport[4];
} state_g, *state = &state_g;
#endif

int g_numberTextures = 17;
GLint g_textures[17] = { };

void LogProps(SCR_PROPS *props) {
  cout << "Props = {" << endl
       << "\t x: " << props->x << endl
       << "\t y: " << props->y << endl
       << "\t width: " << props->width << endl
       << "\t height: " << props->height << endl
       << "\t pixelRatio: " << props->pixelRatio << endl
       << "\t name: " << props->name << endl
       << "\t presets: " << props->presets << endl
       << "\t profile: " << props->profile << endl
//       << "\t submodule: " << props->submodule << endl // Causes problems? Is it initialized?
       << "}" << endl;
}

void LogAction(const char *message) {
  cout << "Action " << message << endl;
}

void LogActionString(const char *message, const char *param) {
  cout << "Action " << message << " " << param << endl;
}

GLuint createTexture(const GLvoid *data, GLint format, unsigned int w, unsigned int h, GLint internalFormat, GLint scaling, GLint repeat) {
  GLuint texture = 0;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scaling);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scaling);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);

#if defined(HAS_GLES)
  glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
#else
  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);
#endif
  glBindTexture(GL_TEXTURE_2D, 0);

  return texture;
}

GLuint createTexture(const char *file, GLint internalFormat, GLint scaling, GLint repeat) {
  std::ostringstream ss;
  ss << g_pathPresets << "/resources/" << file;
  std::string fullPath = ss.str();

  cout << "creating texture " << fullPath << endl;


  unsigned error;
  unsigned char* image;
  unsigned width, height;

  error = lodepng_decode32_file(&image, &width, &height, fullPath.c_str());
  if (error) {
    printf("error %u: %s\n", error, lodepng_error_text(error));
    return 0;
  }

  GLuint texture = createTexture(image, GL_RGBA, width, height, internalFormat, scaling, repeat);
  free(image);
  return texture;
}

GLuint compileShader(GLenum shaderType, const char *shader) {
  GLuint s = glCreateShader(shaderType);
  if (s == 0) {
    cerr << "Failed to create shader from\n====" << endl;
    cerr << shader << endl;
    cerr << "===" << endl;

    return 0;
  }

  glShaderSource(s, 1, &shader, NULL);
  glCompileShader(s);

  GLint param;
  glGetShaderiv(s, GL_COMPILE_STATUS, &param);
  if (param != GL_TRUE) {
    cerr << "Failed to compile shader source\n====" << endl;
    cerr << shader << endl;
    cerr << "===" << endl;

    int infologLength = 0;
    char *infoLog;

    glGetShaderiv(s, GL_INFO_LOG_LENGTH, &infologLength);

    if (infologLength > 0) {
      infoLog = new char[infologLength];
      glGetShaderInfoLog(s, infologLength, NULL, infoLog);
	    cout << "<log>" << endl << infoLog << endl << "</log>" << endl;
      delete [] infoLog;
    }

    glDeleteShader(s);

    return 0;
  }

  return s;
}

GLuint compileAndLinkProgram(const char *vertexShader, const char *fragmentShader) {
  cout << "CompileAndLink " << endl;
  GLuint program = glCreateProgram();
  if (program == 0) {
    cerr << "Failed to create program" << endl;
    return 0;
  }

  GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShader);
  GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

  if (vs && fs) {
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint param;
    glGetProgramiv(program, GL_LINK_STATUS, &param);
    if (param != GL_TRUE) {
      cerr << "Failed to link shader program " << endl;
      glGetError();
      int infologLength = 0;
      char *infoLog;

      glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infologLength);

      if (infologLength > 0) {
        infoLog = new char[infologLength];
        glGetProgramInfoLog(program, infologLength, NULL, infoLog);
	      cout << "<log>" << endl << infoLog << endl << "</log>" << endl;
        delete [] infoLog;
      }

      GLchar errorLog[1024] = {0};
      glGetProgramInfoLog(program, 1024, NULL, errorLog);

      cout << "<vertexShader>" << endl << vertexShader << endl << "</vertexShader>" << endl;
      cout << "<fragmentShader>" << endl << fragmentShader << endl << "</fragmentShader>" << endl;

      glDetachShader(program, vs);
      glDeleteShader(vs);

      glDetachShader(program, fs);
      glDeleteShader(fs);

      glDeleteProgram(program);
      return 0;
    }
  } else {
  	glDeleteProgram(program);
  }

  glUseProgram(0);

  if (vs)
    glDeleteShader(vs);

  if (fs)
    glDeleteShader(fs);

  return program;
}


#if defined(HAS_GLES)

std::string vsSource = TO_STRING(
         precision mediump float;
         precision mediump int;
         attribute vec4 vertex;
         varying vec2 vTextureCoord;
         uniform vec2 uScale;
         void main(void)
         {
            gl_Position = vertex;
            vTextureCoord = vertex.xy*0.5+0.5;
            vTextureCoord.x = vTextureCoord.x * uScale.x;
            vTextureCoord.y = vTextureCoord.y * uScale.y;
         }
  );

std::string render_vsSource = TO_STRING(
         precision mediump float;
         precision mediump int;
         attribute vec4 vertex;
         varying vec2 vTextureCoord;
         void main(void)
         {
            gl_Position = vertex;
            vTextureCoord = vertex.xy*0.5+0.5;
         }
  );

std::string render_fsSource = TO_STRING(
         precision mediump float;
         precision mediump int;
         varying vec2 vTextureCoord;
         uniform sampler2D uTexture;
         void main(void)
         {
            gl_FragColor = texture2D(uTexture, vTextureCoord);
         }
  );
#else
std::string vsSource = "void main() { gl_Position = ftransform(); }";
#endif

std::string fsHeader =
"#extension GL_OES_standard_derivatives : enable\n"
"precision mediump float;\n"
"precision mediump int;\n"
"uniform vec3      iResolution;\n"
"uniform float     iGlobalTime;\n"
"uniform float     iChannelTime[4];\n"
"uniform vec4      iMouse;\n"
"uniform vec4      iDate;\n"
"uniform float     iSampleRate;\n"
"uniform vec3      iChannelResolution[4];\n"
"uniform sampler2D iChannel0;\n"
"uniform sampler2D iChannel1;\n"
"uniform sampler2D iChannel2;\n"
"uniform sampler2D iChannel3;\n";

std::string fsFooter =
"void main(void)\n"
"{\n"
"  vec4 color = vec4(0.0, 0.0, 0.0, 1.0);\n"
"  mainImage(color, gl_FragCoord.xy);\n"
"  color.w = 1.0;\n"
"  gl_FragColor = color;\n"
"}\n";

bool initialized = false;

GLuint shadertoy_shader = 0;

GLint iResolutionLoc        = 0;
GLint iGlobalTimeLoc        = 0;
GLint iChannelTimeLoc       = 0;
GLint iMouseLoc             = 0;
GLint iDateLoc              = 0;
GLint iSampleRateLoc        = 0;
GLint iChannelResolutionLoc = 0;
GLint iChannelLoc[4];
GLuint iChannel[4];

int width = 0;
int height = 0;

int64_t initial_time;
int bits_precision = 0;

static unsigned char *framebuffer;


void unloadTextures() {
  for (int i=0; i<4; i++) {
    if (iChannel[i]) {
      cout << "Unloading iChannel" << i << " " << iChannel[i] << endl;
      glDeleteTextures(1, &iChannel[i]);
      iChannel[i] = 0;
    }
  }
}

void unloadPreset() {
  if (shadertoy_shader) {
    glDeleteProgram(shadertoy_shader);
    shadertoy_shader = 0;
  }
#if defined(HAS_GLES)
  if (state->framebuffer_texture)
  {
    glDeleteTextures(1, &state->framebuffer_texture);
    state->framebuffer_texture = 0;
  }
  if (state->effect_fb)
  {
    glDeleteFramebuffers(1, &state->effect_fb);
    state->effect_fb = 0;
  }
  if (state->render_program) {
    glDeleteProgram(state->render_program);
    state->render_program = 0;
  }
#endif
}

std::string createShader(const std::string &file)
{
  std::ostringstream ss;
  ss << g_pathPresets << "/resources/" << file;
  std::string fullPath = ss.str();

  cout << "Creating shader from " << fullPath << endl;

  std::ifstream t(fullPath);
  std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

  //std::string str = "void mainImage( out vec4 fragColor, in vec2 fragCoord ) { fragColor = texture2D(iChannel1, fragCoord.xy / iResolution.xy); }";

  std::string fsSource = fsHeader + "\n" + str + "\n" + fsFooter;
  return fsSource;
}

GLint loadTexture(int number)
{
  if (number == 98) { // random texture
    number = (rand() >> 12) % 15;
  }
  if (number >= 0 && number < g_numberTextures) {
    GLint format = GL_RGBA;
    GLint scaling = GL_LINEAR;
    GLint repeat = GL_REPEAT;
    return createTexture(g_fileTextures[number], format, scaling, repeat);
  }
  else if (number == 99) { // framebuffer
    return createTexture(framebuffer, GL_RGBA, width, height, GL_RGBA, GL_LINEAR, GL_REPEAT);
  }
  return 0;
}

void loadPreset(int preset, std::string vsSource, std::string fsSource)
{
  unloadPreset();
  shadertoy_shader = compileAndLinkProgram(vsSource.c_str(), fsSource.c_str());

  iResolutionLoc        = glGetUniformLocation(shadertoy_shader, "iResolution");
  iGlobalTimeLoc        = glGetUniformLocation(shadertoy_shader, "iGlobalTime");
  iChannelTimeLoc       = glGetUniformLocation(shadertoy_shader, "iChannelTime");
  iMouseLoc             = glGetUniformLocation(shadertoy_shader, "iMouse");
  iDateLoc              = glGetUniformLocation(shadertoy_shader, "iDate");
  iSampleRateLoc        = glGetUniformLocation(shadertoy_shader, "iSampleRate");
  iChannelResolutionLoc = glGetUniformLocation(shadertoy_shader, "iChannelResolution");
  iChannelLoc[0]        = glGetUniformLocation(shadertoy_shader, "iChannel0");
  iChannelLoc[1]        = glGetUniformLocation(shadertoy_shader, "iChannel1");
  iChannelLoc[2]        = glGetUniformLocation(shadertoy_shader, "iChannel2");
  iChannelLoc[3]        = glGetUniformLocation(shadertoy_shader, "iChannel3");

#if defined(HAS_GLES)
  state->uScale         = glGetUniformLocation(shadertoy_shader, "uScale");
  state->attr_vertex_e  = glGetAttribLocation(shadertoy_shader,  "vertex");

  state->render_program = compileAndLinkProgram(render_vsSource.c_str(), render_fsSource.c_str());
  state->uTexture       = glGetUniformLocation(state->render_program, "uTexture");
  state->attr_vertex_r  = glGetAttribLocation(state->render_program,  "vertex");
  // Prepare a texture to render to
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &state->framebuffer_texture);
  glBindTexture(GL_TEXTURE_2D, state->framebuffer_texture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, state->fbwidth, state->fbheight, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // Prepare a framebuffer for rendering
  glGenFramebuffers(1, &state->effect_fb);
  glBindFramebuffer(GL_FRAMEBUFFER, state->effect_fb);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state->framebuffer_texture, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  initial_time = P8PLATFORM::GetTimeMs();
#endif
}

static uint64_t GetTimeStamp() {
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec*(uint64_t)1000000+tv.tv_usec;
}

static void RenderTo(GLuint shader, GLuint effect_fb)
{
  glUseProgram(shader);

#if !defined(HAS_GLES)
  glDisable(GL_BLEND);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(-1, 1, -1, 1, -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  glClear(GL_DEPTH_BUFFER_BIT);
  glPushMatrix();
#endif

  if (shader == shadertoy_shader) {
    GLuint w = width, h = height;
#if defined(HAS_GLES)
    if (state->fbwidth && state->fbheight)
      w = state->fbwidth, h = state->fbheight;
#endif
    int64_t intt = P8PLATFORM::GetTimeMs() - initial_time;
    if (bits_precision)
      intt &= (1<<bits_precision)-1;

    float t = intt / 1000.0f;
    GLfloat tv[] = { t, t, t, t };

    glUniform3f(iResolutionLoc, w, h, 0.0f);
    glUniform1f(iGlobalTimeLoc, t);
    glUniform1f(iSampleRateLoc, 48000);
    glUniform1fv(iChannelTimeLoc, 4, tv);
#if defined(HAS_GLES)
    glUniform2f(state->uScale, (GLfloat)width/state->fbwidth, (GLfloat)height/state->fbheight);
#endif
    struct timeval tval;
    gettimeofday(&tval, NULL);
    tm *ltm = localtime(&tval.tv_sec);

    float year = 1900 + ltm->tm_year;
    float month = ltm->tm_mon;
    float day = ltm->tm_mday;
    float sec = (ltm->tm_hour * 60 * 60) + (ltm->tm_min * 60) + ltm->tm_sec + 1e-6 * tval.tv_usec;

    glUniform4f(iDateLoc, year, month, day, sec);

    for (int i=0; i<4; i++) {
      glActiveTexture(GL_TEXTURE0 + i);
#if !defined(HAS_GLES)
      glEnable(GL_TEXTURE_2D);
#endif
      glUniform1i(iChannelLoc[i], i);
      glBindTexture(GL_TEXTURE_2D, iChannel[i]);
    }
  } else {
#if defined(HAS_GLES)
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, state->framebuffer_texture);
    glUniform1i(state->uTexture, 0); // first currently bound texture "GL_TEXTURE0"
#endif
  }

#if defined(HAS_GLES)
  // default viewport
  if (effect_fb)
    glViewport(0, 0, state->fbwidth, state->fbheight);

  // Draw the effect to a texture or direct to framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, effect_fb);

  GLuint attr_vertex = shader == shadertoy_shader ? state->attr_vertex_e : state->attr_vertex_r;
  glBindBuffer(GL_ARRAY_BUFFER, state->vertex_buffer);
  glVertexAttribPointer(attr_vertex, 4, GL_FLOAT, 0, 16, 0);
  glEnableVertexAttribArray(attr_vertex);
  glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
  glDisableVertexAttribArray(attr_vertex);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  if (effect_fb)
    glViewport(state->viewport[0], state->viewport[1], state->viewport[2], state->viewport[3]);
#else
  glBegin(GL_QUADS);
    glVertex3f(-1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f, 1.0f, 0.0f);
    glVertex3f( 1.0f,-1.0f, 0.0f);
    glVertex3f(-1.0f,-1.0f, 0.0f);
  glEnd();
#endif

  for (int i=0; i<4; i++) {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
#if !defined(HAS_GLES)
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
#endif
  glUseProgram(0);
}


//-- Render -------------------------------------------------------------------
// Called once per frame. Do all rendering here.
//-----------------------------------------------------------------------------
extern "C" void Render()
{
  glGetError();
  //cout << "Render" << std::endl;
  if (initialized) {
#if defined(HAS_GLES)
    if (state->fbwidth && state->fbheight) {
      RenderTo(shadertoy_shader, state->effect_fb);
      RenderTo(state->render_program, 0);
    } else {
      RenderTo(shadertoy_shader, 0);
    }
#else
    RenderTo(shadertoy_shader, 0);
#endif
    static int frames = 0;
    static uint64_t ts;
    if (frames == 0) {
      ts = GetTimeStamp();
    }
    frames++;
    uint64_t ts2 = GetTimeStamp();
    if (ts2 - ts > 1e6)
    {
     printf("%d fps\n", frames);
     ts += 1e6;
     frames = 0;
    }
  }
}

#ifdef HAS_GLES
static int determine_bits_precision()
{
  std::string vsPrecisionSource = TO_STRING(
	void mainImage( out vec4 fragColor, in vec2 fragCoord )
	{
	    float y = ( fragCoord.y / iResolution.y ) * 26.0;
	    float x = 1.0 - ( fragCoord.x / iResolution.x );
	    float b = fract( pow( 2.0, floor(y) ) + x );
	    if (fract(y) >= 0.9)
		b = 0.0;
	    fragColor = vec4(b, b, b, 1.0 );
	}
  );
  std::string fsPrecisionSource = fsHeader + "\n" + vsPrecisionSource + "\n" + fsFooter;

  state->fbwidth = 32, state->fbheight = 26*10;
  loadPreset(0, vsSource, fsPrecisionSource);
  RenderTo(shadertoy_shader, state->effect_fb);
  glFinish();

  unsigned char *buffer = new unsigned char[state->fbwidth * state->fbheight * 4];
  if (buffer)
    glReadPixels(0, 0, state->fbwidth, state->fbheight, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
  #if 0
  for (int j=0; j<state->fbheight; j++) {
    for (int i=0; i<state->fbwidth; i++) {
      printf("%02x ", buffer[4*(j*state->fbwidth+i)]);
    }
    printf("\n");
  }
  #endif
  int bits = 0;
  unsigned char b = 0; 
  for (int j=0; j<state->fbheight; j++) {
    unsigned char c = buffer[4*(j*state->fbwidth+(state->fbwidth>>1))];
    if (c && !b)
      bits++;
    b = c;
  }
  delete buffer;
  unloadPreset();
  return bits;
}

static double measure_performance(int preset, int size)
{
  int iterations = -1;
  std::string fsSource = createShader(g_presets[preset].file);

  state->fbwidth = state->fbheight = size;
  loadPreset(preset, vsSource, fsSource);

  int64_t end, start;
  do {
#if defined(HAS_GLES)
    RenderTo(shadertoy_shader, state->effect_fb);
    RenderTo(state->render_program, state->effect_fb);
#else
    RenderTo(shadertoy_shader, 0);
#endif
    glFinish();
    if (++iterations == 0)
      start = P8PLATFORM::GetTimeMs();
    end = P8PLATFORM::GetTimeMs();
  } while (end - start < 50);
  double t = (double)(end - start)/iterations;
  //printf("%s %dx%d %.1fms = %.2f fps\n", __func__, size, size, t, 1000.0/t);
  unloadPreset();
  return t;
}
#endif

static void launch(int preset)
{
#ifdef HAS_GLES
  //get current viewport
  glGetIntegerv(GL_VIEWPORT, state->viewport);

  bits_precision = determine_bits_precision();
  // mali-400 has only 10 bits which means milliseond timer wraps after ~1 second.
  // we'll fudge that up a bit as having a larger range is more important than ms accuracy
  bits_precision = max(bits_precision, 13);
  printf("bits=%d\n", bits_precision);
#endif
  
  unloadTextures();
  for (int i=0; i<4; i++) {
    if (g_presets[preset].channel[i] >= 0)
      iChannel[i] = loadTexture(g_presets[preset].channel[i]);
  }

#ifdef HAS_GLES
  const int size1 = 256, size2=512;
  double t1 = measure_performance(preset, size1);
  double t2 = measure_performance(preset, size2);
 
  double expected_fps = 40.0;
  // time per pixel for rendering fragment shader
  double B = (t2-t1)/(size2*size2-size1*size1);
  // time to render to screen
  double A = t2 - size2*size2 * B;
  // how many pixels get the desired fps
  double pixels = (1000.0/expected_fps - A) / B;
  state->fbwidth = sqrtf(pixels * width / height);
  if (state->fbwidth * 4 >= width * 3)
    state->fbwidth = 0;
  else if (state->fbwidth < 320)
    state->fbwidth = 320;
  state->fbheight = state->fbwidth * height / width;

  printf("expected fps=%f, pixels=%f %dx%d (A:%f B:%f t1:%.1f t2:%.1f)\n", expected_fps, pixels, state->fbwidth, state->fbheight, A, B, t1, t2);
#endif

  std::string fsSource = createShader(g_presets[preset].file);
  loadPreset(preset, vsSource, fsSource);
}

extern "C" void Start()
{
  cout << "Start " << std::endl;
  launch(g_currentPreset);
}

//-- GetSubModules ------------------------------------------------------------
// Return any sub modules supported by this vis
//-----------------------------------------------------------------------------
extern "C" unsigned int GetSubModules(char ***names)
{
  cout << "GetSubModules" << std::endl;
  return 0; // this vis supports 0 sub modules
}

//-- GetPresets ---------------------------------------------------------------
// Return a list of presets to XBMC for display
//-----------------------------------------------------------------------------
extern "C" unsigned int GetPresets(char ***presets)
{
  cout << "GetPresets " << g_presets.size() << std::endl;

  if (!lpresets) {
    lpresets = new char*[g_presets.size()];
    size_t i=0;
    for (auto p : g_presets)
      lpresets[i++] = const_cast<char*>(p.name.c_str());
  }

  *presets = lpresets;
  return g_presets.size();
}

//-- GetPreset ----------------------------------------------------------------
// Return the index of the current playing preset
//-----------------------------------------------------------------------------
extern "C" unsigned GetPreset()
{
  return g_currentPreset;
}

//-- IsLocked -----------------------------------------------------------------
// Returns true if this add-on use settings
//-----------------------------------------------------------------------------
extern "C" bool IsLocked()
{
  cout << "IsLocked" << std::endl;
  return false;
}

//-- Create -------------------------------------------------------------------
// Called on load. Addon should fully initalize or return error status
//-----------------------------------------------------------------------------
ADDON_STATUS ADDON_Create(void* hdl, void* props)
{
  cout << "ADDON_Create" << std::endl;
  SCR_PROPS *p = (SCR_PROPS *)props;

  LogProps(p);

  g_pathPresets = p->presets;
  width = p->width;
  height = p->height;

#if !defined(HAS_GLES)
  if (GLEW_OK != glewInit()) {
	  std::cout << "Failed to initialize glew";
  }
#endif
  if (!initialized)
  {
#if defined(HAS_GLES)
    static const GLfloat vertex_data[] = {
        -1.0,1.0,1.0,1.0,
        1.0,1.0,1.0,1.0,
        1.0,-1.0,1.0,1.0,
        -1.0,-1.0,1.0,1.0,
    };
    glGetError();
    // Upload vertex data to a buffer
    glGenBuffers(1, &state->vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, state->vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STATIC_DRAW);
#endif
    framebuffer = new unsigned char[width * height * 4];
    if (framebuffer)
      glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, framebuffer);

    initialized = true;
  }

  if (!props)
    return ADDON_STATUS_UNKNOWN;

  return ADDON_STATUS_NEED_SAVEDSETTINGS;
}

//-- Stop ---------------------------------------------------------------------
// This dll must cease all runtime activities
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
extern "C" void ADDON_Stop()
{
  cout << "ADDON_Stop" << std::endl;
}

//-- Destroy ------------------------------------------------------------------
// Do everything before unload of this add-on
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
extern "C" void ADDON_Destroy()
{
  cout << "ADDON_Destroy" << std::endl;

  unloadPreset();
  unloadTextures();

  if (lpresets)
    delete[] lpresets, lpresets = nullptr;

  if (framebuffer) {
    delete framebuffer;
    framebuffer = NULL;
  }

#if defined(HAS_GLES)
  glDeleteBuffers(1, &state->vertex_buffer);
#endif

  initialized = false;
}

//-- GetStatus ---------------------------------------------------------------
// Returns the current Status of this visualisation
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
extern "C" ADDON_STATUS ADDON_GetStatus()
{
  cout << "ADDON_GetStatus" << std::endl;
  return ADDON_STATUS_OK;
}

//-- SetSetting ---------------------------------------------------------------
// Set a specific Setting value (called from XBMC)
// !!! Add-on master function !!!
//-----------------------------------------------------------------------------
extern "C" ADDON_STATUS ADDON_SetSetting(const char *strSetting, const void* value)
{
  cout << "ADDON_SetSetting " << strSetting << std::endl;
  if (!strSetting || !value)
    return ADDON_STATUS_UNKNOWN;

  // TODO Someone _needs_ to fix this API in kodi, its terrible.
  // a) Why not use GetSettings instead of hacking SetSettings like this?
  // b) Why does it give index and not settings key?
  // c) Seemingly random ###End which if you never write will while(true) the app
  // d) Writes into const setting and value...
  if (strcmp(strSetting, "###GetSavedSettings") == 0)
  {
    cout << "WTF...." << endl;
    if (strcmp((char*)value, "0") == 0)
    {
      strcpy((char*)strSetting, "lastpresetidx");
      sprintf ((char*)value, "%i", (int)g_currentPreset);
    }
    if (strcmp((char*)value, "1") == 0)
    {
      strcpy((char*)strSetting, "###End");
    }

    return ADDON_STATUS_OK;
  }

  int c = atoi((const char *)value);
  if (strcmp(strSetting,"preset") == 0 && c >= 0 && c < g_presets.size()+1)
  {
    cout << "Setting preset from " << g_currentPreset << " to " << c << endl;
    if (c == 0)
      c = (rand() >> 12) % g_presets.size();
    else
      c--;
    g_currentPreset = c;
  }
  return ADDON_STATUS_OK;
}

