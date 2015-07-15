
#include "highgui.hpp"
#include "cap_ffmpeg_api.hpp"

using namespace std;

VideoCapture::VideoCapture()
{
}

VideoCapture::VideoCapture(const string& filename)
{
	open(filename);
}

VideoCapture::VideoCapture(int device)
{
	open(device);
}

VideoCapture::~VideoCapture()
{
	cap.reset();
}

bool VideoCapture::open(const string& filename)
{
	if (isOpened())
		release();
	cap = cvCreateFileCapture(filename.c_str());
	return isOpened();
}

bool VideoCapture::open(int device)
{
	if (isOpened())
		release();
	cap = cvCreateCameraCapture(device);
	return isOpened();
}

bool VideoCapture::isOpened() const
{
	return !cap.empty();
}

void VideoCapture::release()
{
	cap.release();
}

bool VideoCapture::grab()
{
	return cvGrabFrame(cap) != 0;
}

bool VideoCapture::retrieve(Mat& image, int channel)
{
	IplImage* _img = cvRetrieveFrame(cap, channel);
	if (!_img)
	{
		image.release();
		return false;
	}
	if (_img->origin == IPL_ORIGIN_TL)
		Mat(_img).copyTo(image);
	else
	{
		Mat temp(_img);
		flip(temp, image, 0);
	}
	return true;
}

bool VideoCapture::read(Mat& image)
{
	if (grab())
		retrieve(image);
	else
		image.release();
	return !image.empty();
}

VideoCapture& VideoCapture::operator >>(Mat& image)
{
	read(image);
	return *this;
}

bool VideoCapture::set(int propId, double value)
{
//	return cvSetCaptureProperty(cap, propId, value) != 0;
	return false;
}

double VideoCapture::get(int propId)
{
//	return cvGetCaptureProperty(cap, propId);
	return false;
}

