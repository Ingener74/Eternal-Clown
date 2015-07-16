#ifndef __OPENCV_FFMPEG_H__
#define __OPENCV_FFMPEG_H__

enum
{
	CV_FFMPEG_CAP_PROP_POS_MSEC = 0,
	CV_FFMPEG_CAP_PROP_POS_FRAMES = 1,
	CV_FFMPEG_CAP_PROP_POS_AVI_RATIO = 2,
	CV_FFMPEG_CAP_PROP_FRAME_WIDTH = 3,
	CV_FFMPEG_CAP_PROP_FRAME_HEIGHT = 4,
	CV_FFMPEG_CAP_PROP_FPS = 5,
	CV_FFMPEG_CAP_PROP_FOURCC = 6,
	CV_FFMPEG_CAP_PROP_FRAME_COUNT = 7
};

struct CvCapture* cvCreateFileCapture(const char* filename);

int cvSetCaptureProperty(struct CvCapture* cap, int prop, double value);

double cvGetCaptureProperty(struct CvCapture* cap, int prop);

int cvGrabFrame(struct CvCapture* cap);

int cvRetrieveFrame(struct CvCapture* capture, unsigned char** data, int* step, int* width, int* height, int* cn);

void cvReleaseCapture(struct CvCapture** cap);

struct CvVideoWriter* cvCreateVideoWriter(const char* filename, int fourcc, double fps, int width, int height,
    int isColor);

int cvWriteFrame(struct CvVideoWriter* writer, const unsigned char* data, int step, int width, int height, int cn,
    int origin);

void cvReleaseVideoWriter(struct CvVideoWriter** writer);

#endif
