
#include <chrono>

#include <GL/glew.h>

#include "FFmpegVideoSource.h"
#include "VideoSprite.h"

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

VideoSprite::VideoSprite(const std::string& fileName)
{
	// Открываем видео
	m_videoSource.reset(new FFmpegVideoSource(fileName));

	// Получаем первый кадр
	auto frame = m_videoSource->getFrame();

	// Создаём текстуру
//	auto texture = new CCTexture2D();
//	texture->initWithData(frame.m_buffer.data(), kCCTexture2DPixelFormat_RGB888, frame.m_width, frame.m_height,
//			{ static_cast<float>(frame.m_width), static_cast<float>(frame.m_height) });
//
//	if (!initWithTexture(texture))
//	{
//		throw runtime_error(string(__FILE__) + ": " + to_string(__LINE__) + ": " + string(__PRETTY_FUNCTION__) + ": " +
//				"invalid init");
//	}
//
//	texture->release();

    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    width = height = std::max(upperPowerOfTwo(frame.m_width), upperPowerOfTwo(frame.m_height));

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.m_width, frame.m_height, GL_RGB, GL_UNSIGNED_BYTE, frame.m_buffer.data());
}

VideoSprite::~VideoSprite()
{
	stop();
}

void VideoSprite::start(function<void()> onEnd)
{
	if (m_videoSource)
	{
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

void VideoSprite::stop()
{
	m_videoSource->stop();
}

void VideoSprite::draw()
{
	// Если настало время показывать кадр ...
	if (m_videoSource->isPresentationTimeStampPassed())
	{
		// ... тогда лочим его(кадр) и текстуру ...
		lock_guard<mutex> lock(m_frameMutex);

		// ... и загружаем новый кадр в текстуру если он не пустой
		if (!m_frame.m_buffer.empty())
		{
			updateTexture(m_frame);
			m_frame.m_buffer.clear();
		}
	}
	glBindTexture(GL_TEXTURE_2D, textureID);
}

void VideoSprite::updateTexture(const RGBFrame& frame)
{
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, frame.m_width, frame.m_height, GL_RGB, GL_UNSIGNED_BYTE,
			frame.m_buffer.data());
}
