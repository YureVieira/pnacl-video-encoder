// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// Modified by Joaquim neto@LME (www.github.com/joaquimmnetto)

#include "video_encoder.h"
#include "video_encoder_instance.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include <algorithm>
#include <deque>
#include <iostream>
#include <sstream>
#include <vector>

#include <sys/mount.h>

#include <nacl_io/nacl_io.h>

#include <ppapi/c/pp_errors.h>
#include <ppapi/c/ppb_console.h>
#include <ppapi/cpp/input_event.h>

#include <ppapi/cpp/module.h>
#include <ppapi/cpp/rect.h>
#include <ppapi/cpp/var.h>
#include <ppapi/cpp/var_array_buffer.h>
#include <ppapi/cpp/var_dictionary.h>

#include <ppapi/cpp/video_frame.h>
#include <ppapi/utility/threading/simple_thread.h>

#define INSTANCE (*instance)
#include "log.h"

#define NS_EXP 1000000000; //10^9

#define FS_PATH "/persistent/"

//2 keyframe/s.
#define KEY_FRAME_RATIO 15

static bool probed_encoder = false;

VideoEncoder::VideoEncoder(pp::Instance* _instance, WebmMuxer& _muxer) :
		instance(_instance), handle(instance), muxer(_muxer), track(NULL), video_profile(PP_VIDEOPROFILE_VP8_ANY), frame_format(
				PP_VIDEOFRAME_FORMAT_I420), cb_factory(this), encoding(false), encode_ticking(
				false), force_key_frame(false),last_tick(0), last_ts(
				0), frame_count(0)
{

	if (!probed_encoder)
	{
		ProbeEncoder();
	}

}

VideoEncoder::~VideoEncoder()
{
	delete_and_nulify(track);
	tracks.clear();

}
void VideoEncoder::Encode(pp::Size _requested_size,
		PP_VideoProfile _video_profile)
{

	frame_size = _requested_size;
	video_profile = _video_profile;
	Log(frame_size.width() << "," << frame_size.height());
	muxer.ConfigureVideo(frame_size.width(), frame_size.height());
	StartTrackFrames();
	StartEncoder();
}

void VideoEncoder::SetTrack(pp::Resource track_res)
{

	if (!pp::MediaStreamVideoTrack::IsMediaStreamVideoTrack(track_res))
	{
		LogError(-99, "Track não é um recurso válido");
		return;
	}

	if (track != NULL)
	{
		StopTrackingFrames();
		delete_and_nulify(track);
	}

	track = new VideoTrack(instance, track_res);
	if(encoding){
		StartTrackFrames();
	}
	force_key_frame = true;
}

void VideoEncoder::ProbeEncoder()
{
	encoder = pp::VideoEncoder(handle);
	encoder.GetSupportedProfiles(
			cb_factory.NewCallbackWithOutput(&VideoEncoder::OnEncoderProbed));
}

void VideoEncoder::OnEncoderProbed(int32 result,
		const std::vector<PP_VideoProfileDescription> profiles)
{
	probed_encoder = (result == PP_OK);
}

void VideoEncoder::StartEncoder()
{
	encoder = pp::VideoEncoder(handle);
	timestamps.clear();
	Log("Inicializando Encoder");
	Log(
			"Tamanho do frame base do encoder: (" << frame_size.width() << "," << frame_size.height() << ")");
	int32 error = encoder.Initialize(frame_format, frame_size, video_profile,
			2000000, PP_HARDWAREACCELERATION_WITHFALLBACK,
			cb_factory.NewCallback(&VideoEncoder::OnInitializedEncoder));

	if (error != PP_OK_COMPLETIONPENDING)
	{
		LogError(error, "Não foi possível inicializar encoder");
		return;
	}

}

void VideoEncoder::OnInitializedEncoder(int32 result)
{
	Log("Encoder Inicializado");
	if (result != PP_OK)
	{
		LogError(result, "Falha na inicialização do encoder");
		return;
	}

	pp::Size encoder_size;
	if (encoder.GetFrameCodedSize(&encoder_size) != PP_OK)
	{
		LogError(result,
				"Não foi possível adquirir o tamanho do frame do encoder");
		return;
	}
	if (!encoder_size.IsEmpty())
	{
		frame_size = encoder_size;
	}

	encoding = true;
	encoder.GetBitstreamBuffer(
			cb_factory.NewCallbackWithOutput(
					&VideoEncoder::OnGetBitstreamBuffer));

	ScheduleNextEncode();

}

void VideoEncoder::ScheduleNextEncode()
{

	if (encode_ticking)
		return;

	PP_Time now = pp::Module::Get()->core()->GetTime();

	//framerate hard-coded para 30fps(frame rate máximo, caso não haja limitações de hardware)
	PP_Time tick = 1.0 / 30.0;

	PP_Time delta = tick
			- std::max(std::min(now - last_tick - tick, tick), 0.0);

	(static_cast<VideoEncoderInstance*>(instance))->encoderThread().message_loop().PostWork(
			cb_factory.NewCallback(&VideoEncoder::GetEncoderFrameTick),
			delta * 1000);

	last_tick = now;
	encode_ticking = true;
}

void VideoEncoder::GetEncoderFrameTick(int32 result)
{

	if (encoding)
	{
		if (receiving_frames && track && !track->current_frame.is_null())
		{
			pp::VideoFrame frame = track->current_frame;
			track->current_frame.detach();
			GetEncoderFrame(frame);
		}

		encode_ticking = false;
		ScheduleNextEncode();
	}
}

void VideoEncoder::GetEncoderFrame(const pp::VideoFrame& track_frame)
{
	encoder.GetVideoFrame(
			cb_factory.NewCallbackWithOutput(&VideoEncoder::OnEncoderFrame,
					track_frame));
}

void VideoEncoder::OnEncoderFrame(int32 result, pp::VideoFrame encoder_frame,
		pp::VideoFrame track_frame)
{

	if (result == PP_ERROR_ABORTED)
	{
		track->RecycleFrame(track_frame);
		return;
	}
	if (result != PP_OK)
	{
		track->RecycleFrame(track_frame);
		LogError(result,
				"Não foi possível pegar o frame do encoder de vídeo, pulando frame...");
		return;
	}

	if (track_frame.is_null())
	{
		Log(
				"Frame " << track_frame.GetTimestamp() << " é nulo, pulando frame...");
		return;
	}

	if (CopyVideoFrame(encoder_frame, track_frame) == PP_OK)
	{
		EncodeFrame(encoder_frame);
	}

	track->RecycleFrame(track_frame);
}

int32 VideoEncoder::CopyVideoFrame(pp::VideoFrame dest, pp::VideoFrame src)
{
	if (dest.GetDataBufferSize() < src.GetDataBufferSize())
	{
		LogError(-99,
				"Ao copiar o frame : "<< dest.GetDataBufferSize() << " < " << src.GetDataBufferSize());
		return -99;
	}

	dest.SetTimestamp(src.GetTimestamp());
	memcpy(dest.GetDataBuffer(), src.GetDataBuffer(), src.GetDataBufferSize());
	return PP_OK;
}

void VideoEncoder::EncodeFrame(const pp::VideoFrame& frame)
{
	timestamps.push_back(frame.GetTimestamp());

	force_key_frame = frame_count > KEY_FRAME_RATIO;

#ifdef LOG_FRAMES
	Log("Encodando frame " << frame.GetTimestamp() * NS_EXP );
	if(force_key_frame)
	{	Log("key frame");}
#endif

	PP_Bool key_frame = force_key_frame ? PP_TRUE : PP_FALSE;
	encoder.Encode(frame, key_frame,
			cb_factory.NewCallback(&VideoEncoder::OnEncodeDone));
}

void VideoEncoder::OnEncodeDone(int32 result)
{
	if (result == PP_ERROR_ABORTED)
	{
		return;
	}
	if (result != PP_OK)
	{
		LogError(result, "Falha ao encodar");
	}
	else
	{
		if (force_key_frame)
		{
			frame_count = 0;
			force_key_frame = false;
		}
		frame_count++;
	}
}

void VideoEncoder::OnGetBitstreamBuffer(int32 result, PP_BitstreamBuffer buffer)
{
	if (result == PP_ERROR_ABORTED)
	{
		return;
	}
	if (result != PP_OK)
	{
		LogError(result, "Não foi possível adquirir o Bitstream Buffer");
		return;
	}

	uint64 timestamp_ns = timestamps.front() * NS_EXP;


	byte* frame_data = static_cast<byte*>(buffer.buffer);
	uint64 size = buffer.size;
	bool key_frame = buffer.key_frame;

#ifdef LOG_FRAMES
	Log("Salvando frame " << timestamp_ns << " de tamanho "<< size << key frame?" key frame" : "");
#endif
	//arquivos webm tem que ter timestamps que aumentem monotonicamente.
	if (timestamp_ns >= last_ts)
	{
		last_ts = timestamp_ns;
		if (!muxer.AddVideoFrame(frame_data, size, timestamp_ns, key_frame))
		{
			LogError(-99, "Enquanto salvando frame "<< timestamp_ns);
		}

	}
	else
	{
		Log("Frame "<< timestamp_ns << " atrasado, não salvando");
	}

	timestamps.pop_front();
	encoder.RecycleBitstreamBuffer(buffer);

	encoder.GetBitstreamBuffer(
			cb_factory.NewCallbackWithOutput(
					&VideoEncoder::OnGetBitstreamBuffer));
}

void VideoEncoder::StartTrackFrames()
{
	track->StartTracking(frame_size);
	receiving_frames = true;
}

bool VideoEncoder::StopEncode()
{
	StopTrackingFrames();
	encoder.Close();

	tracks.clear();
	encoding = false;
	return muxer.Finish();
}

void VideoEncoder::StopTrackingFrames()
{
	receiving_frames = false;
	track->StopTracking();
}

