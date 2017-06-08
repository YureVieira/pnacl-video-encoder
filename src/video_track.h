/*
 * videotrack.h
 *
 *  Created on: 10 de dez de 2015
 *  Author: Joaquim neto@LME (www.github.com/joaquimmnetto)
 */

#ifndef VIDEO_TRACK_H_
#define VIDEO_TRACK_H_

#include <ppapi/cpp/instance.h>
#include <ppapi/cpp/size.h>
#include <ppapi/cpp/video_frame.h>
#include <ppapi/cpp/media_stream_video_track.h>

#include <ppapi/utility/completion_callback_factory.h>

#include "tipos.h"
/**
 * Classe que representa o loop da trilha de frames de vídeo.
 * Usada por video_encoder.h para fornecer frames ao encoder.
 */
class VideoTrack
{
public:
	VideoTrack(pp::Instance* instance, pp::Resource track_res);

	~VideoTrack();
	/**
	 * Inicia a track.
	 * @param frame_size - tamanho dos frames que estão na track.
	 */
	void StartTracking(pp::Size frame_size);
	/**
	 * Recicla o objeto frame já utilizado, para ser usado com outro frame.
	 * @param frame - Frame já utilizado, para ser reciclado.
	 */
	void RecycleFrame(pp::VideoFrame& frame);
	/**
	 * Encerra a track.
	 */
	void StopTracking();

	inline pp::Size GetFrameSize()
	{
		return frame_size;
	}

	/**Frame atual da track.*/
	pp::VideoFrame current_frame;
private:
	/**Instância atual da PPAPI.*/
	pp::Instance* instance;
	/**Tamanho dos frames a serem puxados da track*/
	pp::Size frame_size;
	/**Objeto da PPAPI representando a track*/
	pp::MediaStreamVideoTrack track;
	/**Factory de callbacks, necessária para vários métodos*/
	pp::CompletionCallbackFactory<VideoTrack> cb_factory;
	/**Se a track ainda está rodando*/
	bool tracking;
	/**Callback do método de configuração da trilha*/
	void ConfigureCallback(int config_res);
	/**
	 * Método contendo o loop dos frames da trilha. Também é um callback de getFrame().
	 * @param frame - Frame puxado da trilha.
	 * */
	void TrackFramesLoop(int getframe_res, pp::VideoFrame frame);
};

#endif /* VIDEO_TRACK_H_ */
