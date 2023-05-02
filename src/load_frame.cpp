extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavcodec/codec_id.h>
#include <libswscale/swscale.h>
#include <inttypes.h>
#include <libavutil/error.h>
#include <libavutil/imgutils.h>
#include <libavutil/avutil.h>
}

bool load_frame(const char* filename, int* width_out, int* height_out, unsigned char** data_out) {
	printf("Beginning to encode\n");
	// declare format and codec contexts, also codec for decoding
	AVFormatContext* fmt_ctx = avformat_alloc_context();
	AVCodecParameters* av_codec_params;
	AVStream* in_stream = nullptr;
	AVStream* out_stream = nullptr;
	const AVCodec* av_codec = nullptr;

	const AVCodec* Codec = NULL;
	int ret = 0;
	int video_stream_index = -1;

	AVFrame* frame = NULL;
	AVPacket* packet = NULL;

	AVFormatContext* av_format_ctx = avformat_alloc_context();

	if (!av_format_ctx) {
		printf("Couldn't create AVFormatContext\n");
		return false;
	}
	//open input file
	if (avformat_open_input(&av_format_ctx, filename, NULL, NULL) != 0) {
		av_log(NULL, AV_LOG_ERROR, "Couldn't open file\n");
		return false;
	}

	// get video stream index
	for (int i = 0; i < av_format_ctx->nb_streams; i++)
	{
		//auto stream = av_format_ctx->streams[i];
		av_codec_params = av_format_ctx->streams[i]->codecpar;
		av_codec = avcodec_find_decoder(av_codec_params->codec_id);

		if (!av_codec) {
			continue;
		}
		if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			break;
		}
	}

	// if no video stream found, exit
	if (video_stream_index == -1)
	{
		av_log(NULL, AV_LOG_ERROR, "No video stream\n");
		return false;
	}
	av_dump_format(av_format_ctx, video_stream_index, filename, false);


	//Setting up codec context for decoding
	AVCodecContext* av_codec_ctx = avcodec_alloc_context3(av_codec);


	if (!av_codec_ctx) {
		printf("Couldn't create AVCodecContext\n");
		return false;
	}

	if (avcodec_parameters_to_context(av_codec_ctx, av_codec_params) < 0) {
		printf("Couldn't initialize AVCodecContext\n");
		return false;
	}

	if (avcodec_open2(av_codec_ctx, av_codec, NULL) < 0) {
		printf("Couldn't open codec\n");
		return false;
	}

	AVFrame* av_frame = av_frame_alloc();
	if (!av_frame) {
		printf("Couldn't allocate AVFrame\n");
		return false;
	}
	AVPacket* av_packet = av_packet_alloc();
	if (!av_packet) {
		printf("Couldn't allocate AVPacket\n");
		return false;
	}


	// set output parameters
	//*width_out = av_codec_ctx->width;
	//*height_out = av_codec_ctx->height;


	if (*data_out == NULL) {
		printf("Couldn't allocate memory for output data\n");
		return false;
	}


	//printf("\n");
	// av_packet = av_packet_alloc();
	//// Read packets from input file
	//ret = av_read_frame(av_format_ctx, av_packet);
	//if (ret < 0) {
	//	printf("Error reading frame: %s\n");
	//	return false;
	//}

	//printf("Packet data: ");
	//for (int i = 0; i < av_packet->size; i++) {
	//	printf("%02x ", av_packet->data[i]);
	//}
	//printf("\n");

	int response;
	// encode each frame
	//int frame_count = 0;
	while (av_read_frame(av_format_ctx, av_packet) >= 0) {
		if (av_packet->stream_index != video_stream_index) {
			continue;
		}
		response = avcodec_send_packet(av_codec_ctx, av_packet);
		if (response < 0) {
			char errbuf[AV_ERROR_MAX_STRING_SIZE];
			av_strerror(response, errbuf, AV_ERROR_MAX_STRING_SIZE);
			printf("Failed to decode packet1: %s\n", errbuf);
			return false;
		}
		
		response = avcodec_receive_frame(av_codec_ctx, av_frame);
		if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
				continue;
		}
		else if (response < 0) {
			char errbuf[AV_ERROR_MAX_STRING_SIZE];
			av_strerror(response, errbuf, AV_ERROR_MAX_STRING_SIZE);
			printf("Failed to decode packet2: %s\n", errbuf);
			return false;
		}
		

		av_packet_unref(av_packet);
	}

	uint8_t* data = new uint8_t[av_frame->width * av_frame->height * 4];

	SwsContext* sws_scaler_ctx = sws_getContext(av_frame->width, av_frame->height, av_codec_ctx->pix_fmt, 
												av_frame->width, av_frame->height, AV_PIX_FMT_RGB0, 
												SWS_BILINEAR, NULL, NULL,NULL);
	if (!sws_scaler_ctx) {
		printf("couldn't initialize sws scaler\n");
		return false;
	}
	uint8_t* dest[4] = { data, NULL, NULL, NULL };
	int dest_linesize[4] = { av_frame->width * 4, 0, 0, 0 };
	sws_scale(sws_scaler_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, dest, dest_linesize);
	sws_freeContext(sws_scaler_ctx);

	*width_out = av_frame->width;
	*height_out = av_frame->height;
	*data_out = data;



	// clean up
	avformat_close_input(&av_format_ctx);
	av_frame_free(&av_frame);
	av_packet_free(&av_packet);
	avcodec_free_context(&av_codec_ctx);

	return true;

}