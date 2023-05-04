#ifndef video_reader_hpp
#define video_reader_hpp

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <inttypes.h>
}

struct VideoReaderState {
	//Public things for other parts of the program to use
	int width, height;
	AVRational time_base;

	//Private things for the video reader to use
	AVFormatContext* av_format_ctx;
	AVCodecContext* av_codec_ctx;
	AVFrame* av_frame;
	AVPacket* av_packet;
	int video_stream_index;
	struct SwsContext* sws_scaler_ctx = NULL;

	
};

bool video_reader_open(VideoReaderState* state, const char* filename);
bool video_reader_read_frame(VideoReaderState* state, uint8_t* frame_buffer, int64_t* pts);
void video_reader_close(VideoReaderState* state);

#endif // !video_reader_hpp
