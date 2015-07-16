#ifndef COMMON_SRC_GRAPHICS_VIDEOSPRITE_H_
#define COMMON_SRC_GRAPHICS_VIDEOSPRITE_H_

#include <memory>
#include <mutex>

#include "IVideoSource.h"

class VideoTexture {
public:
    VideoTexture(const std::string& fileName = { });
    virtual ~VideoTexture();

    void start(std::function<void()> = { });
    void stop();

    bool isPlay() const;

    void draw();

private:
    std::unique_ptr<IVideoSource> m_videoSource;

    std::mutex m_frameMutex;
    RGBFrame m_frame;

    unsigned int textureID = 0;
    int width, height;

    void updateTexture(const RGBFrame&);
};

#endif /* COMMON_SRC_GRAPHICS_VIDEOSPRITE_H_ */
