/*
 * videoencodermodule.h
 *
 *  Created on: 19 de nov de 2015
 *  Author: Joaquim neto@LME (www.github.com/joaquimmnetto)
 */

#ifndef VIDEOENCODERMODULE_H_
#define VIDEOENCODERMODULE_H_

#include <ppapi/cpp/module.h>
#include <ppapi/c/pp_instance.h>
/**
 * Classe usada pela PPAPI para criar instâncias do aplicativo NaCl. È obrigada a existir pelo Framework.
 * Têm que extender pp::Module, e implementar CreateInstance(PP_Instance)
 *
 * */
class VideoEncoderModule: public pp::Module {
public:
	VideoEncoderModule() : pp::Module() {}
	virtual ~VideoEncoderModule() {}

	virtual pp::Instance* CreateInstance(PP_Instance instance);
};


#endif /* VIDEO_ENCODER_MODULE_H_ */
