#include "video_reader.hpp"
bool video_reader_open(VideoReaderState* state, const char* filename) {
	
	//unpack members of state
	auto& width = state->width;
	auto& height = state->height;
	auto& time_base =state->time_base;
	auto& av_format_ctx = state->av_format_ctx;
	auto& av_codec_ctx = state->av_codec_ctx;
	auto& av_frame = state->av_frame;
	auto& av_packet = state->av_packet;
	auto& video_stream_index = state->video_stream_index;
	

	printf("Beginning to encode\n");
	// declare format and codec contexts, also codec for decoding
	AVCodecParameters* av_codec_params;
	AVStream* in_stream = nullptr;
	AVStream* out_stream = nullptr;
	const AVCodec* av_codec = nullptr;

	const AVCodec* Codec = NULL;
	video_stream_index = -1;

	AVFrame* frame = NULL;
	AVPacket* packet = NULL;

	//open input file
	av_format_ctx = avformat_alloc_context();
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
	for (int i = 0; i < av_format_ctx->nb_streams; i++){
		AVStream* av_stream = av_format_ctx->streams[i];
		av_codec_params = av_format_ctx->streams[i]->codecpar;
		av_codec = avcodec_find_decoder(av_codec_params->codec_id);

		if (!av_codec) {
			continue;
		}
		if (av_codec_params->codec_type == AVMEDIA_TYPE_VIDEO) {
			video_stream_index = i;
			width = av_codec_params->width;
			height = av_codec_params->height;
			time_base =av_format_ctx->streams[i]->time_base;
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
	av_codec_ctx = avcodec_alloc_context3(av_codec);


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

	av_frame = av_frame_alloc();
	if (!av_frame) {
		printf("Couldn't allocate AVFrame\n");
		return false;
	}
	av_packet = av_packet_alloc();
	if (!av_packet) {
		printf("Couldn't allocate AVPacket\n");
		return false;
	}

	return true;
}

bool video_reader_read_frame(VideoReaderState* state, uint8_t* frame_buffer, int64_t* pts) {
	auto& width = state->width;
	auto& height = state->height;
	auto& av_format_ctx = state->av_format_ctx;
	auto& av_codec_ctx = state->av_codec_ctx;
	auto& av_frame = state->av_frame;
	auto& av_packet = state->av_packet;
	auto& video_stream_index = state->video_stream_index;
	auto& sws_scaler_ctx = state->sws_scaler_ctx;


	//Decodes one frame
	int response;
	while (av_read_frame(av_format_ctx, av_packet) >= 0) {
		if (av_packet->stream_index != video_stream_index) {
			av_packet_unref(av_packet);
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
			av_packet_unref(av_packet);
			continue;
		}
		else if (response < 0) {
			char errbuf[AV_ERROR_MAX_STRING_SIZE];
			av_strerror(response, errbuf, AV_ERROR_MAX_STRING_SIZE);
			printf("Failed to decode packet2: %s\n", errbuf);
			return false;
		}


		av_packet_unref(av_packet);
		break;
	}

	*pts = av_frame->pts;

	printf(
		"Frame %c (%d) pts %d dts %d key_frame %d [coded_picture_number %d, display_picture_number %d\n]",
		av_get_picture_type_char(av_frame->pict_type),
		av_codec_ctx->frame_number,
		av_frame->pts,
		av_frame->pkt_dts,
		av_frame->key_frame,
		av_frame->coded_picture_number,
		av_frame->display_picture_number
	);
	// Sets up sws scaler
	if (!sws_scaler_ctx) {
		sws_scaler_ctx = sws_getContext(width, height, av_codec_ctx->pix_fmt,
										width, height, AV_PIX_FMT_RGB0,
										SWS_BILINEAR, NULL, NULL, NULL);
	}

	if (!sws_scaler_ctx) {
		printf("couldn't initialize sws scaler\n");
		return false;
	}

	uint8_t* dest[4] = { frame_buffer, NULL, NULL, NULL };
	int dest_linesize[4] = { av_frame->width * 4, 0, 0, 0 };
	sws_scale(sws_scaler_ctx, av_frame->data, av_frame->linesize, 0, av_frame->height, dest, dest_linesize);

	return true;

}
void video_reader_close(VideoReaderState* state) {
	sws_freeContext(state->sws_scaler_ctx);
	avformat_close_input(&state->av_format_ctx);
	avformat_free_context(state->av_format_ctx);
	av_frame_free(&state->av_frame);
	av_packet_free(&state->av_packet);
	avcodec_free_context(&state->av_codec_ctx);

}