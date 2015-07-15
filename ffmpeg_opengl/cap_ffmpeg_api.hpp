#ifndef __OPENCV_FFMPEG_H__
#define __OPENCV_FFMPEG_H__

enum
{
    CV_FFMPEG_CAP_PROP_POS_MSEC=0,
    CV_FFMPEG_CAP_PROP_POS_FRAMES=1,
    CV_FFMPEG_CAP_PROP_POS_AVI_RATIO=2,
    CV_FFMPEG_CAP_PROP_FRAME_WIDTH=3,
    CV_FFMPEG_CAP_PROP_FRAME_HEIGHT=4,
    CV_FFMPEG_CAP_PROP_FPS=5,
    CV_FFMPEG_CAP_PROP_FOURCC=6,
    CV_FFMPEG_CAP_PROP_FRAME_COUNT=7
};

struct CvCapture_FFMPEG* cvCreateFileCapture_FFMPEG(const char* filename);
struct CvCapture_FFMPEG_2* cvCreateFileCapture_FFMPEG_2(const char* filename);
int cvSetCaptureProperty_FFMPEG(struct CvCapture_FFMPEG* cap,
                                int prop, double value);
int cvSetCaptureProperty_FFMPEG_2(struct CvCapture_FFMPEG_2* cap,
                                  int prop, double value);
double cvGetCaptureProperty_FFMPEG(struct CvCapture_FFMPEG* cap, int prop);
double cvGetCaptureProperty_FFMPEG_2(struct CvCapture_FFMPEG_2* cap, int prop);
int cvGrabFrame_FFMPEG(struct CvCapture_FFMPEG* cap);
int cvGrabFrame_FFMPEG_2(struct CvCapture_FFMPEG_2* cap);
int cvRetrieveFrame_FFMPEG(struct CvCapture_FFMPEG* capture, unsigned char** data,
                           int* step, int* width, int* height, int* cn);
int cvRetrieveFrame_FFMPEG_2(struct CvCapture_FFMPEG_2* capture, unsigned char** data,
                           int* step, int* width, int* height, int* cn);
void cvReleaseCapture_FFMPEG(struct CvCapture_FFMPEG** cap);
void cvReleaseCapture_FFMPEG_2(struct CvCapture_FFMPEG_2** cap);
struct CvVideoWriter_FFMPEG* cvCreateVideoWriter_FFMPEG(const char* filename,
            int fourcc, double fps, int width, int height, int isColor );
struct CvVideoWriter_FFMPEG_2* cvCreateVideoWriter_FFMPEG_2(const char* filename,
            int fourcc, double fps, int width, int height, int isColor );

int cvWriteFrame_FFMPEG(struct CvVideoWriter_FFMPEG* writer, const unsigned char* data,
                                          int step, int width, int height, int cn, int origin);

void cvReleaseVideoWriter_FFMPEG(struct CvVideoWriter_FFMPEG** writer);

typedef void* (*CvCreateFileCapture_Plugin)( const char* filename );
typedef void* (*CvCreateCameraCapture_Plugin)( int index );
typedef int (*CvGrabFrame_Plugin)( void* capture_handle );
typedef int (*CvRetrieveFrame_Plugin)( void* capture_handle, unsigned char** data, int* step,
                                       int* width, int* height, int* cn );
typedef int (*CvSetCaptureProperty_Plugin)( void* capture_handle, int prop_id, double value );
typedef double (*CvGetCaptureProperty_Plugin)( void* capture_handle, int prop_id );
typedef void (*CvReleaseCapture_Plugin)( void** capture_handle );
typedef void* (*CvCreateVideoWriter_Plugin)( const char* filename, int fourcc,
                                             double fps, int width, int height, int iscolor );
typedef int (*CvWriteFrame_Plugin)( void* writer_handle, const unsigned char* data, int step,
                                    int width, int height, int cn, int origin);
typedef void (*CvReleaseVideoWriter_Plugin)( void** writer );

/*
 * For CUDA encoder
 */

struct OutputMediaStream_FFMPEG* create_OutputMediaStream_FFMPEG(const char* fileName, int width, int height, double fps);
void release_OutputMediaStream_FFMPEG(struct OutputMediaStream_FFMPEG* stream);
void write_OutputMediaStream_FFMPEG(struct OutputMediaStream_FFMPEG* stream, unsigned char* data, int size, int keyFrame);

typedef struct OutputMediaStream_FFMPEG* (*Create_OutputMediaStream_FFMPEG_Plugin)(const char* fileName, int width, int height, double fps);
typedef void (*Release_OutputMediaStream_FFMPEG_Plugin)(struct OutputMediaStream_FFMPEG* stream);
typedef void (*Write_OutputMediaStream_FFMPEG_Plugin)(struct OutputMediaStream_FFMPEG* stream, unsigned char* data, int size, int keyFrame);

/*
 * For CUDA decoder
 */

struct InputMediaStream_FFMPEG* create_InputMediaStream_FFMPEG(const char* fileName, int* codec, int* chroma_format, int* width, int* height);
void release_InputMediaStream_FFMPEG(struct InputMediaStream_FFMPEG* stream);
int read_InputMediaStream_FFMPEG(struct InputMediaStream_FFMPEG* stream, unsigned char** data, int* size, int* endOfFile);

typedef struct InputMediaStream_FFMPEG* (*Create_InputMediaStream_FFMPEG_Plugin)(const char* fileName, int* codec, int* chroma_format, int* width, int* height);
typedef void (*Release_InputMediaStream_FFMPEG_Plugin)(struct InputMediaStream_FFMPEG* stream);
typedef int (*Read_InputMediaStream_FFMPEG_Plugin)(struct InputMediaStream_FFMPEG* stream, unsigned char** data, int* size, int* endOfFile);

#endif
