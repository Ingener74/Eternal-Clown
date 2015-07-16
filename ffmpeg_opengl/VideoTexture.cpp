#include "VideoTexture.h"

#include <chrono>

#include <GL/glew.h>

#include "FFmpegVideoSource.h"

using namespace std;
using namespace std::chrono;

uint32_t upperPowerOfTwo(uint32_t v) {
    v--;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
    v++;
    return v;
}

VideoTexture::VideoTexture(const std::string& fileName) {
    // Открываем видео
    m_videoSource.reset(new FFmpegVideoSource(fileName));

    // Получаем первый кадр
    auto frame = m_videoSource->getFrame();

    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    width = height = std::max(upperPowerOfTwo(frame.m_width), upperPowerOfTwo(frame.m_height));

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.m_width, frame.m_height, GL_RGB, GL_UNSIGNED_BYTE,
            frame.m_buffer.data());
}

VideoTexture::~VideoTexture() {
    stop();
    glDeleteTextures(1, &textureID);
}

void VideoTexture::start(function<void()> onEnd) {
    if (m_videoSource) {
        // Перематываем в начало
        m_videoSource->seek(0);

        updateTexture(m_videoSource->getFrame());

        m_videoSource->start([this](RGBFrame frame)
        {
            // Лочим и забираем изображение
                lock_guard<mutex> lock(m_frameMutex);

                m_frame = move(frame);
            }, onEnd);
    }
}

void VideoTexture::stop() {
    m_videoSource->stop();
}

void VideoTexture::draw() {
    // Если настало время показывать кадр ...
    if (m_videoSource->isPresentationTimeStampPassed()) {
        // ... тогда лочим его(кадр) и текстуру ...
        lock_guard<mutex> lock(m_frameMutex);

        // ... и загружаем новый кадр в текстуру если он не пустой
        if (!m_frame.m_buffer.empty()) {
            updateTexture(m_frame);
            m_frame.m_buffer.clear();
        }
    }
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void VideoTexture::updateTexture(const RGBFrame& frame) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.m_width, frame.m_height, GL_RGB, GL_UNSIGNED_BYTE,
            frame.m_buffer.data());
}

bool VideoTexture::isPlay() const {
    return m_videoSource->isPlay();
}
