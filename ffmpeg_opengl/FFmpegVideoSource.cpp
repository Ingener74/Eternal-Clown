
#include <iostream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

#include "FFmpegVideoSource.h"

using namespace std;
using namespace std::chrono;

string error2string(int error)
{
	const size_t errorBufferSize = 1024;
	char buffer[errorBufferSize];
	av_strerror(error, buffer, errorBufferSize);
	return {buffer};
}


FFmpegVideoSource::FFmpegVideoSource(const std::string& fileName) :
		m_work(false)
{
	if (fileName.empty())
	{
		throw runtime_error(string(__FILE__) + ": " + to_string(__LINE__) + ": " + string(__PRETTY_FUNCTION__) + ": " +
				"пустое имя файла");
	}

	int err = 0;

	err = avformat_open_input(&m_formatContext, fileName.c_str(), NULL, NULL);
	if (err < 0)
	{
		throw runtime_error(string(__FILE__) + ": " + to_string(__LINE__) + ": " + string(__PRETTY_FUNCTION__) + ": " +
				"не могу открыть файл " + fileName + ", ошибка " + error2string(err));
	}

	err = avformat_find_stream_info(m_formatContext, NULL);
	if (err < 0)
	{
		throw runtime_error(string(__FILE__) + ": " + to_string(__LINE__) + ": " + string(__PRETTY_FUNCTION__) + ": " +
				"не могу найти поток с информацией " + error2string(err));
	}

	m_streamIndex = -1;
	for (unsigned i = 0; i < m_formatContext->nb_streams; ++i)
	{
		if (m_formatContext->streams[i]->codec->coder_type == AVMEDIA_TYPE_VIDEO)
		{
			m_streamIndex = i;
			m_codecContext = m_formatContext->streams[m_streamIndex]->codec;
			if (auto videoCodec = avcodec_find_decoder(m_codecContext->codec_id))
			{
				if (avcodec_open2(m_codecContext, videoCodec, nullptr) != 0)
				{
					throw runtime_error(string(__FILE__) + ": " + to_string(__LINE__) + ": " + string(__PRETTY_FUNCTION__) + ": " +
							"Не могу открыть кодек");
				}
				break;
			}
		}
	}

	m_srcFrame = av_frame_alloc();
	m_dstFrame = av_frame_alloc();

	m_swsContext = sws_getContext(m_codecContext->width, m_codecContext->height, m_codecContext->pix_fmt,
			m_codecContext->width, m_codecContext->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
}

FFmpegVideoSource::~FFmpegVideoSource()
{
	av_frame_free(&m_srcFrame);
	av_frame_free(&m_dstFrame);
	avformat_close_input(&m_formatContext);
}

RGBFrame FFmpegVideoSource::getFrame()
{
	RGBFrame result;
	result.m_width = m_codecContext->width;
	result.m_height = m_codecContext->height;
	result.m_buffer.resize(result.m_width * result.m_height * 3);
	avpicture_fill((AVPicture*) (m_dstFrame), result.m_buffer.data(), AV_PIX_FMT_RGB24, result.m_width, result.m_height);
	m_noFrames = true;

	AVPacket pkt;
	av_init_packet(&pkt);

	while (av_read_frame(m_formatContext, &pkt) >= 0)
	{
		if (pkt.stream_index == m_streamIndex)
		{
			while (pkt.size > 0)
			{
				if (decodeFrame(m_srcFrame, m_dstFrame, &pkt))
				{
					goto output;
				}
			}
		}
	}

output:
	av_free_packet(&pkt);
	return std::move(result);
}

bool FFmpegVideoSource::decodeFrame(AVFrame* frameSrc, AVFrame* frameDst, AVPacket* pkt)
{
	int size = 0, frameFinished;
	{
		size = avcodec_decode_video2(m_codecContext, frameSrc, &frameFinished, pkt);
	}

	if (size < 0)
	{
		throw runtime_error(string(__FILE__) + ": " + to_string(__LINE__) + ": " + string(__PRETTY_FUNCTION__) + ": " +
				"ошибка во время декодирования кадра");
	}

	if (pkt->data)
	{
		pkt->data += size;
		pkt->size -= size;
	}

	if (frameFinished)
	{
		{
			sws_scale(m_swsContext, (const uint8_t* const *) (frameSrc->data), frameSrc->linesize, 0,
					m_codecContext->height, frameDst->data, frameDst->linesize);
		}
		m_noFrames = false;
		return true;
	}
	return false;
}

void FFmpegVideoSource::seek(int64_t timeStamp)
{
	if (av_seek_frame(m_formatContext, m_streamIndex, timeStamp, AVSEEK_FLAG_BACKWARD) < 0)
	{
		cerr << "seek error" << endl;
	}
	avcodec_flush_buffers(m_codecContext);
}

void FFmpegVideoSource::start(function<void(RGBFrame)> onNewFrame, function<void()> onEnd)
{
	if (m_work)
	{
		return;
	}

	if (m_thread.joinable())
	{
		m_thread.join();
	}
	m_work = true;

	seek(0);

	m_startTime = chrono::system_clock::now();

	m_thread = thread([=]
	{
		while (m_work)
		{
			std::unique_lock<mutex> decodeNewLock(m_decodeNewMutex);
			while (!m_frameDrawed)
			{
				m_decodeNew.wait(decodeNewLock);
			}

			onNewFrame(move(getFrame()));

			m_frameDrawed = false;

			if (m_noFrames)
			{
				m_work = false;
				if (onEnd)
				{
					onEnd();
				}
			}
		}
	});
}

void FFmpegVideoSource::stop()
{
	m_work = false;
	m_thread.join();
}

bool FFmpegVideoSource::isPresentationTimeStampPassed()
{
	unique_lock<mutex> lock(m_decodeNewMutex);

	// Если времени с момента старта прошло больше чем надо чтобы текущий кадр был нарисован тогда ...
	auto timeBase = av_q2d(m_formatContext->streams[m_streamIndex]->time_base);
	auto timeCounter = duration_cast<milliseconds>(system_clock::now() - m_startTime).count();

	if (timeCounter * timeBase >= m_srcFrame->pkt_pts * timeBase)
	{
		// ... рисуем его и ...
		m_frameDrawed = true;

		// и разрешаем декодировать новый кадр
		m_decodeNew.notify_all();

		return true;
	}
	return false;
}

bool FFmpegVideoSource::isPlay() const
{
	return m_work;
}
