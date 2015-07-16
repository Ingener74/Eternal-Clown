#ifndef COMMON_SRC_GRAPHICS_FFMPEGVIDEOSOURCE_H_
#define COMMON_SRC_GRAPHICS_FFMPEGVIDEOSOURCE_H_

#include <thread>
#include <atomic>
#include <memory>
#include <string>
#include <chrono>
#include <condition_variable>

#include "IVideoSource.h"

#define ffassert(condition, message) \
            if(!(condition)) \
                throw std::runtime_error(std::string(__FILE__) + ": " + to_string(__LINE__) + ": " + std::string(__PRETTY_FUNCTION__) + ": " + message)

struct AVFrame;
struct AVPacket;

class FFmpegVideoSource: public IVideoSource {
public:
    FFmpegVideoSource(const std::string& fileName);
    virtual ~FFmpegVideoSource();

    virtual RGBFrame getFrame() override;
    virtual void seek(int64_t timeStamp) override;
    virtual void start(std::function<void(RGBFrame)>, std::function<void()> = { }) override;
    virtual void stop() override;
    virtual bool isPresentationTimeStampPassed() override;
    virtual bool isPlay() const override;

private:
    bool decodeFrame(AVPacket*);

    struct AVFormatContext* m_formatContext = nullptr;
    struct AVCodecContext* m_codecContext = nullptr;
    int m_streamIndex = 0;
    struct SwsContext* m_swsContext = nullptr;

    bool m_noFrames = false;

    RGBFrame m_rgbImage;
    AVFrame* m_yuvFrame = nullptr;
    AVFrame* m_rgbFrame = nullptr;

    std::chrono::system_clock::time_point m_startTime;

    bool m_frameDrawed = false;
    std::mutex m_decodeNewMutex;
    std::condition_variable m_decodeNew;

    std::thread m_thread;
    std::atomic<bool> m_work;
};

#endif /* COMMON_SRC_GRAPHICS_FFMPEGVIDEOSOURCE_H_ */
