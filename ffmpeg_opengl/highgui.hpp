#ifndef __OPENCV_HIGHGUI_HPP__
#define __OPENCV_HIGHGUI_HPP__

#include <memory>
#include <string>

struct CvCapture;
struct Mat;

class VideoCapture
{
public:
	VideoCapture();
	VideoCapture(const std::string& filename);
	VideoCapture(int device);

	virtual ~VideoCapture();
	virtual bool open(const std::string& filename);
	virtual bool open(int device);
	virtual bool isOpened() const;
	virtual void release();

	virtual bool grab();
	virtual bool retrieve(Mat& image, int channel = 0);
	virtual VideoCapture& operator >>(Mat& image);
	virtual bool read(struct Mat& image);

	virtual bool set(int propId, double value);
	virtual double get(int propId);

protected:
	std::unique_ptr<CvCapture> cap;
};

#endif
