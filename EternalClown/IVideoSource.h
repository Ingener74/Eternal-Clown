#ifndef COMMON_SRC_GRAPHICS_IVIDEOSOURCE_H_
#define COMMON_SRC_GRAPHICS_IVIDEOSOURCE_H_

#include <vector>

struct RGBFrame
{
	int m_width, m_height;
	std::vector<uint8_t> m_buffer;
};

class IVideoSource
{
public:
	virtual ~IVideoSource() = default;

	virtual RGBFrame getFrame() = 0;
	virtual void seek(int64_t timeStamp) = 0;
	virtual void start(std::function<void(RGBFrame)>, std::function<void()> = {}) = 0;
	virtual void stop() = 0;

	/*
	 * Можно ли рисовать кадр
	 */
	virtual bool isPresentationTimeStampPassed() = 0;
	virtual bool isPlay() const = 0;
};

#endif /* COMMON_SRC_GRAPHICS_IVIDEOSOURCE_H_ */
