/*
 * video_encoder_instance.h
 *
 *  Created on: 18 de nov de 2015
 *  Author: Joaquim neto@LME (www.github.com/joaquimmnetto)
 */

#ifndef VIDEOENCODERINSTANCE_H_
#define VIDEOENCODERINSTANCE_H_

#include <string>
#include <vector>
#include <deque>

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/instance_handle.h>
#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var_dictionary.h>

#include <ppapi/c/pp_codecs.h>

#include <ppapi/utility/threading/simple_thread.h>

#include "libwebm/mkvmuxer.hpp"
#include "libwebm/mkvwriter.hpp"

#include "video_encoder.h"
#include "tipos.h"

/**
 * Classe representando a instância do módulo NaCl na aplicação.
 * Principal função é receber as mensagens do navegador. È obrigatória para o funcionamento da aplicação.
 */
class VideoEncoderInstance: public pp::Instance
{
public:
	VideoEncoderInstance(PP_Instance instance, pp::Module* module);
	virtual ~VideoEncoderInstance();
	/**
	 * Método executado toda vez que uma mensagem é enviada pelo navegador.
	 *
	 * @var_message pp::Var contendo a mensagem enviada.
	 *
	 */
	virtual void HandleMessage(const pp::Var& var_message);

	inline pp::SimpleThread& encoderThread()
	{
		return video_encoder_thread;
	}

private:
	/**
	 * Inicializa o sistema de arquivos do HTMl5
	 * @param fsPath - Caminho do html5 em qual se deve montar o sistema de arquivos.	 *
	 */
	void InitializeFileSystem(const std::string& fsPath);
	/**
	 * Ação executada quando a mensagem de iniciar encoder é enviada pelo navegador.
	 * @param message - Mensagem enviada pelo navegador, no formato de dicionário.
	 */
	void StartEncoder(pp::VarDictionary& message);
	/**
	 * Ação executada quando a mensagem de mudar a trilha é enviada pelo navegador.
	 * @param message - Mensagem enviada pelo navegador, no formato de dicionário.
	 */
	void ChangeTrack(pp::VarDictionary& message);
	/**
	 * Ação executada quando a mensagem de para encoding é enviada pelo navegador.
	 * @param message - Mensagem enviada pelo navegador, no formato de dicionário.
	 */
	void StopEncoder(pp::VarDictionary& message);
	/**Necessário para a inicialização das threads*/
	pp::InstanceHandle handle;
	//Se definida, permite o uso do encoder de áudio, que não está funcionando atualmente.
	/**Encoder de vídeo*/
	VideoEncoder* video_enc;
	/**Muxer que será usado para criar o arquivo de video(ou video+audio) resultante*/
	WebmMuxer* muxer;
	/**Thread em que o encoding de vídeo roda*/
	pp::SimpleThread video_encoder_thread;
	/**Thread em que o encoding de áudio roda*/
	pp::SimpleThread audio_encoder_thread;
	/**Nome do arquivo a ser salvo pelo muxer*/
//	std::string file_name;
	pp::Resource audio_track_res;
};

//-----------------------Workers para executar encoders em paralelo-------------------------------
//Necessários pois a thread principal do Pepper(chrome) não permite bloqueios.
//Variáveis 'soltas' são parâmetros para as funções.

//Parâmetros usados em todos os workers.
pp::Instance* _instance = 0;
VideoEncoder* _video_enc = 0;
//--------------------------------------------------VideoEncoder----------------------------------------
//Tamanho do vídeo a ser encodado.
pp::Size _video_size;
//Tipo de encoder a ser usado(em Jan/16, somente VP8/VP9 são suportados por todas as versões do chrome).
PP_VideoProfile _video_profile;
void VideoEncoderWorker(void* params, int result);

//--------------------------------------------------ChangeTrack-----------------------------------------
//Novo pp::Resource indicando a track que deve ser colocada no local da atual
pp::Resource _new_track_res;
void ChangeTrackWorker(void* params, int result);
//-------------------------------------------------StopEncoder-----------------------------------------
//Muxer que foi passado juntamente ao encoder na inicialização do mesmo.
WebmMuxer* _muxer = 0;
void StopEncoderWorker(void* params, int result);
//-----------------------------------------------------------------------------------------------------

#endif /* VIDEO_ENCODER_INSTANCE_H_ */
