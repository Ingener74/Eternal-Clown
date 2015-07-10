#ifndef COMMON_SRC_GRAPHICS_FFMPEGVIDEOSOURCE_H_
#define COMMON_SRC_GRAPHICS_FFMPEGVIDEOSOURCE_H_

#include <thread>
#include <atomic>
#include <memory>
#include <string>
#include <chrono>
#include <condition_variable>

#include "IVideoSource.h"

class FFmpegVideoSource: public IVideoSource
{
public:
	FFmpegVideoSource(const std::string& fileName);
	virtual ~FFmpegVideoSource();

	virtual RGBFrame getFrame() override;
	virtual void seek(int64_t timeStamp) override;
	virtual void start(std::function<void(RGBFrame)>, std::function<void()> = {}) override;
	virtual void stop() override;
	virtual bool isPresentationTimeStampPassed() override;
	virtual bool isPlay() const override;

private:
	bool decodeFrame(struct AVFrame* frameSrc, AVFrame* frameDst, struct AVPacket*);

	struct AVFormatContext*					m_formatContext = nullptr;
	struct AVCodecContext*					m_codecContext = nullptr;
	int										m_streamIndex = 0;
	struct SwsContext*						m_swsContext = nullptr;

	AVFrame*								m_srcFrame = nullptr;
	AVFrame*								m_dstFrame = nullptr;

	bool									m_noFrames = false;

	std::chrono::system_clock::time_point	m_startTime;

	bool									m_frameDrawed = false;
	std::mutex								m_decodeNewMutex;
	std::condition_variable					m_decodeNew;

	std::thread								m_thread;
	std::atomic<bool>						m_work;
};

#endif /* COMMON_SRC_GRAPHICS_FFMPEGVIDEOSOURCE_H_ */
