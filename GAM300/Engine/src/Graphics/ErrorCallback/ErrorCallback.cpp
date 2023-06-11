#include <fstream>
#include <sstream>
#include <iostream>
#include <GL/glew.h>
#include <GL/GL.h>

namespace GFX {

	void __stdcall openglCallbackFunction(GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei /* length */, const GLchar* message, const void* /* userParam */)
	{
		// If it is a notification, we can simply ognore it
		if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) return;

		// Instead of printing error codes, print the representation
		const char* type_string = "";
		const char* severity_string = "";

		switch (type)
		{
		case GL_DEBUG_TYPE_ERROR:
			type_string = "** GL ERROR **";
			break;

		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
			type_string = "** GL DEPRECATED BEHAVIOR **";
			break;

		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			type_string = "** GL UNDEFINED BEHAVIOR **";
			break;

		case GL_DEBUG_TYPE_PORTABILITY:
			type_string = "** GL PORTABILITY **";
			break;

		case GL_DEBUG_TYPE_PERFORMANCE:
			type_string = "** GL PERFORMANCE **";
			break;

		case GL_DEBUG_TYPE_MARKER:
			type_string = "** GL TYPE MARKER **";
			break;

		case GL_DEBUG_TYPE_PUSH_GROUP:
			type_string = "** GL PUSH GROUP **";
			break;

		case GL_DEBUG_TYPE_POP_GROUP:
			type_string = "** GL POP GROUP **";
			break;

		default:
			type_string = "** GL OTHER **";
			break;
		}

		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:
			severity_string = "HIGH SEVERITY";
			break;

		case GL_DEBUG_SEVERITY_MEDIUM:
			severity_string = "MEDIUM SEVERITY";
			break;

		case GL_DEBUG_SEVERITY_LOW:
			severity_string = "LOW SEVERITY";
			break;

		case GL_DEBUG_SEVERITY_NOTIFICATION:
			severity_string = "NOTIFICATION";
			break;
		}

		// Print the actual message
		fprintf(stderr, "GL: %s. Severity = %s. Message = %s\n",
			type_string, severity_string, message);
	}

	/**
	 * Enables OpenGL callbacks. This callbacks will be useful to intercept errors.
	 */
	void EnableGLErrorCallbacks()
	{
		//return;
		// Enable the debug output and set our custom callback function
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(openglCallbackFunction, 0);

		// This should give an error, since we are binding a buffer that has not
		// been generated
		///glBindBuffer(GL_ARRAY_BUFFER, 101); 
	}

	/**
	 * Another debugging function, for manual checking
	 */
	void  _check_gl_error(const char* file, int line)
	{
		GLenum err = glGetError();

		while (err != GL_NO_ERROR) {
			std::string error;

			switch (err) {
			case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
			case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
			case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
			case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
			}

			const char* only_file = file;
			while (*file) {
				if (*file == '/' || *file++ == '\\') {
					only_file = file;
				}

				file++;
			}

			// format error message
			std::stringstream errorMsg;
			errorMsg << "GL_" << error.c_str() << " - File: " << only_file
				<< "    Line: " << line << std::endl;

			// send to VS outpput

			// repeat
			err = glGetError();
		}
	}


}