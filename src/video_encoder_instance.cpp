// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Modified by Joaquim neto@LME (www.github.com/joaquimmnetto)

#include "video_encoder_instance.h"
#include <sys/mount.h>

#include <nacl_io/nacl_io.h>

#include <ppapi/cpp/module.h>
#include <ppapi/cpp/var.h>
#include <ppapi/cpp/var_array_buffer.h>
#include <ppapi/utility/completion_callback_factory.h>

#include <ppapi/c/pp_errors.h>

#include <ppapi/utility/threading/simple_thread.h>

#define INSTANCE (*this)
#include "log.h"

#define FS_PATH "/persistent/"


VideoEncoderInstance::VideoEncoderInstance(PP_Instance instance,
		pp::Module* module) :
		pp::Instance(instance), handle(this),
#ifdef USING_AUDIO
				audio_enc(0),
#endif
				video_enc(0), muxer(0), video_encoder_thread(handle), audio_encoder_thread(
						handle)
{
	InitializeFileSystem(FS_PATH);
}

VideoEncoderInstance::~VideoEncoderInstance()
{
	umount(FS_PATH);
	nacl_io_uninit();
	delete_and_nulify(video_enc);
#ifdef USING_AUDIO
	delete_and_nulify(audio_enc);
#endif
	delete_and_nulify(muxer);

}

void VideoEncoderInstance::InitializeFileSystem(const std::string& fsPath)
{

	int initRes = nacl_io_init_ppapi(pp_instance(),
			pp::Module::Get()->get_browser_interface());
	if (initRes != PP_OK)
	{
		LogError(initRes, "Ao inicializar nacl_io");
	}

	int mntRes = mount("", 	//src
			FS_PATH, 		//target
			"html5fs",		//filesystem type
			0, //flags
			"type=PERSISTENT,expected_size=1073741824" //html5 stuff
			);
	if (mntRes != 0)
	{
		LogError(mntRes, "Ao montar sistema de arquivos ");
	}

}

void VideoEncoderInstance::HandleMessage(const pp::Var& var_message)
{

	if (!var_message.is_dictionary())
	{
		LogError(pp::Var(PP_ERROR_BADARGUMENT).DebugString(),
				"Mensagem inválida!");
		return;
	}

	pp::VarDictionary dict_message(var_message);
	std::string command = dict_message.Get("command").AsString();
	Log("Comando recebido:" << command);

	//
	if (command == "start")
	{
		StartEncoder(dict_message);
	}

	else if (command == "change_track")
	{
		ChangeTrack(dict_message);
	}

	else if (command == "stop")
	{
		StopEncoder(dict_message);
	}
	else
	{
		LogError(PP_ERROR_BADARGUMENT, "Comando inválido!");
	}

}

void VideoEncoderInstance::StartEncoder(pp::VarDictionary& message)
{
	if (video_enc && video_enc->is_encoding())
	{
		Log("Encode já em execução, chame stop antes de iniciar novamente.");
		return;
	}
	muxer = new WebmMuxer(*this);
	video_enc = new VideoEncoder(this, *muxer);

	pp::Size requested_size = pp::Size(message.Get("width").AsInt(),
			message.Get("height").AsInt());

	pp::Var var_video_track = message.Get("video_track");

	if (!var_video_track.is_resource())
	{
		LogError(-99, "Video_track não é um resource");
		return;
	}

	std::string file_name = message.Get("file_name").AsString();

	if (file_name.length() == 0)
	{
		LogError(-99, "Nome do arquivo não pode ser vazio");
		return;
	}

	_video_profile = PP_VIDEOPROFILE_VP8_ANY;
	_video_size = requested_size;
	_video_enc = video_enc;

	Log("Argumentos:");
	Log("Encoder: VP8");
	Log(
			"Video Size: (" << _video_size.width() << "," << _video_size.height() <<")");
	Log("File name:" << file_name);

	muxer->SetFileName(file_name);

	video_enc->SetTrack(var_video_track.AsResource());

	video_encoder_thread.Start();

	pp::CompletionCallback encoder_callback(&VideoEncoderWorker, 0);
	video_encoder_thread.message_loop().PostWork(encoder_callback, 0);

}
void VideoEncoderInstance::ChangeTrack(pp::VarDictionary& message)
{
	_new_track_res = message.Get("video_track").AsResource();
	_video_enc = video_enc;

	Log("Track modificada");

	pp::CompletionCallback change_callback(&ChangeTrackWorker, 0);
	video_encoder_thread.message_loop().PostWork(change_callback, 0);
}
void VideoEncoderInstance::StopEncoder(pp::VarDictionary& message)
{
	if (!video_enc)
	{
		Log("Inicie um encode antes de tentar parar!");
		return;
	}
	if (!video_enc->is_encoding())
	{
		Log("Encoder já parado");
	}
	else
	{
		Log("Parando encode...");
		_instance = this;
		_video_enc = video_enc;
		_muxer = muxer;
		pp::CompletionCallback stop_callback(&StopEncoderWorker, 0);
		video_encoder_thread.message_loop().PostWork(stop_callback, 0);
	}
}

//--------------------------Impl. dos Workers--------------------------------
void ChangeTrackWorker(void* params, int result)
{
	_video_enc->SetTrack(_new_track_res);
}

void VideoEncoderWorker(void* params, int result)
{
	_video_enc->Encode(_video_size, _video_profile);
}

void StopEncoderWorker(void* params, int result)
{

	if (_video_enc->StopEncode())
	{
		delete_and_nulify(_video_enc);
		delete_and_nulify(_muxer);

	}

	_instance->PostMessage("StopComplete");
}
