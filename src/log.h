
/*
 * Author: Joaquim neto@LME (www.github.com/joaquimmnetto)
 *
 */

#ifndef LOG_H
#define LOG_H

//Se definido, adciona log mostrando estado de cada frame. Use somente para debug.
//#define LOG_FRAMES

//necessário ser definido com um pp::Instance, objeto que tem o comando de log da PPAPI.
#ifndef INSTANCE
	#define INSTANCE
#endif

#include <sstream>
#include <ppapi/cpp/var.h>


std::stringstream __logStream;
//Log no formato Em 'file.cpp':'xx' - 'exp'.
//exp se comporta como um input em sstream(sstream << exp).
#define Log(exp) __logStream.str(std::string()); __logStream << "Em " << __FILE__ <<":"<< __LINE__ <<" - " << exp; INSTANCE.LogToConsole(PP_LOGLEVEL_LOG,__logStream.str())
//Log no formato Em 'file.cpp':'xx' - Erro:'error':'exp'.
//Tanto error como exp se comportam como um input em sstream(sstream<<error<<exp)
#define LogError(error, exp) __logStream.str(std::string()); __logStream << "Em " << __FILE__ <<":"<< __LINE__ <<" - " << "Erro: "<< error << " : " << exp; INSTANCE.LogToConsole(PP_LOGLEVEL_ERROR,__logStream.str())

#endif
