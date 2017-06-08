/*
 * WebmMuxer.cpp
 *
 *  Created on: 24 de nov de 2015
 *  Author: Joaquim neto@LME (www.github.com/joaquimmnetto)
 */

#include "webm_muxer.h"

#define INSTANCE instance
#include "log.h"

#include <cerrno>

WebmMuxer::WebmMuxer(pp::Instance& _instance) :
		instance(_instance), pSegment(0), last_frame_ts(0), delayed_frame_count(0), initialized(
				false), finished(false)
{
}

WebmMuxer::~WebmMuxer()
{
	writer.Close();
	delete_and_nulify(pSegment);
}

int WebmMuxer::Init(std::string file_name)
{
	if (initialized)
	{
		return true;
	}

	std::stringstream sfilename;
	sfilename << "/persistent/" << file_name;
	if (!writer.Open(sfilename.str().c_str()))
	{
		Log("Arquivo não pôde ser aberto");
		return false;
	}
	pSegment = new mkvmuxer::Segment();
	pSegment->Init(&writer);
	pSegment->set_mode(mkvmuxer::Segment::kFile);

	mkvmuxer::SegmentInfo* info = pSegment->GetSegmentInfo();
	info->set_writing_app("Lousa Digital");

	video_track_num = pSegment->AddVideoTrack(video_width, video_height, 1);

	pSegment->OutputCues(true);
	pSegment->CuesTrack(video_track_num);


	initialized = true;
	finished = false;


	return true;

}

void WebmMuxer::ConfigureVideo(int _video_width, int _video_height)
{
	this->video_width = _video_width;
	this->video_height = _video_height;
}

bool WebmMuxer::AddVideoFrame(byte* data, uint32 length, uint64 timestamp,
		bool key_frame)
{

	if (!Init(file_name))
	{
		LogError(-99, "Enquanto inicializando o muxer");
		return false;
	}

	mkvmuxer::Frame frame;
	CreateFrame(data, length, timestamp, video_track_num, key_frame, frame);

	if ( !frame.IsValid() || ( frame.timestamp() < last_frame_ts ) )
	{
		Log("Frame " << frame.timestamp() << "atrasado na reprodução, pulando frame...");
	}
	else
	{
		if (pSegment->AddGenericFrame(&frame))
		{
			if (key_frame)
			{
				pSegment->AddCuePoint(frame.timestamp(), video_track_num);
			}
			last_frame_ts = frame.timestamp();
			return true;
		}
	}

	return false;
}

bool WebmMuxer::Finish()
{
	if (finished)
	{
		return true;
	}

	int segRes = 0;
	if ((segRes = pSegment->Finalize()) < 0)
	{
		LogError(errno, "Errno value");
		LogError(segRes, "Erro ao finalizar o arquivo webm");
		return false;
	}

	writer.Close();

	finished = true;
	initialized = false;

	delete_and_nulify(pSegment);

	return true;
}

void WebmMuxer::CreateFrame(byte* data, uint32 length, uint64 timestamp,
		int track, bool key_frame,/*out*/mkvmuxer::Frame& frame)
{

	if (frame.Init(data, length))
	{
		frame.set_is_key(key_frame);
		frame.set_track_number(track);
		frame.set_timestamp(timestamp);
	}
}

