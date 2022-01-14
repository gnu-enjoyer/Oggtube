#include "muxxer.h"
#include "utils.h"

//FFmpeg
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#define __STDC_CONSTANT_MACROS
}

//Required as C++ does not allow an int to be implicitly converted to a enum
inline AVRounding operator|(AVRounding a, AVRounding b) {
    return static_cast<AVRounding>(static_cast<int>(a) | static_cast<int>(b));

}

//Transmux is a blocking operation which can be quite slow depending on network I/O
bool Muxxer::transmux(const char *in_filename, const char *out_filename) {


    AVFormatContext *input_format_context = NULL;
    AVFormatContext *output_format_context = NULL;

    int ret;
    int reattempt = true;
    start:

    /* In */
    if ((ret = avformat_open_input(&input_format_context, in_filename, NULL, NULL)) < 0) {
        Logger::get().write("[Muxxer] Could not open input file!",true);
        if(reattempt) {
            Logger::get().write("[Muxxer] Trying again...",false);
            reattempt = false;
            goto start;
        } else return false;
    }
    if ((ret = avformat_find_stream_info(input_format_context, NULL)) < 0) {
        Logger::get().write("[Muxxer] Failed to retrieve input stream information", true);
        return false;
    }

    /* Out */
    avformat_alloc_output_context2(&output_format_context, NULL, NULL, out_filename);
    if (!output_format_context) {
        Logger::get().write("[Muxxer] Could not create output context", true);
        ret = 1;
        return false;
    }

    int *streams_list = NULL;
    int stream_index = 0;
    int number_of_streams = 0;

    number_of_streams = input_format_context->nb_streams;
    streams_list = (int *) av_mallocz_array(number_of_streams, sizeof(*streams_list));

    for (unsigned int i = 0; i < input_format_context->nb_streams; i++) {
        AVStream *out_stream;
        AVStream *in_stream = input_format_context->streams[i];
        AVCodecParameters *in_codecpar = in_stream->codecpar;
        if (in_codecpar->codec_type != AVMEDIA_TYPE_AUDIO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_VIDEO &&
            in_codecpar->codec_type != AVMEDIA_TYPE_SUBTITLE) {
            streams_list[i] = -1;
            continue;
        }
        streams_list[i] = stream_index++;
        out_stream = avformat_new_stream(output_format_context, NULL);
        if (!out_stream) {
            Logger::get().write("[Muxxer] Failed allocating output stream", true);
            ret = AVERROR_UNKNOWN;
            return false;
        }
        ret = avcodec_parameters_copy(out_stream->codecpar, in_codecpar);
        if (ret < 0) {
            Logger::get().write("[Muxxer] Failed to copy codec parameters", true);
            return false;
        }
    }

    if (!(output_format_context->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&output_format_context->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            Logger::get().write("[Muxxer] Could not open output file", true);
            return false;
        }
    }

    ret = avformat_write_header(output_format_context, NULL);
    if (ret < 0) {
        Logger::get().write("[Muxxer] Error occurred when opening output file", true);
        return false;
    }

    AVPacket packet;

    while (1) {
        AVStream *in_stream, *out_stream;
        ret = av_read_frame(input_format_context, &packet);
        if (ret < 0)
            break;
        in_stream = input_format_context->streams[packet.stream_index];
        if (packet.stream_index >= number_of_streams || streams_list[packet.stream_index] < 0) {
            av_packet_unref(&packet);
            continue;
        }
        packet.stream_index = streams_list[packet.stream_index];
        out_stream = output_format_context->streams[packet.stream_index];

        packet.pts = av_rescale_q_rnd(packet.pts, in_stream->time_base, out_stream->time_base,
                                      AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        packet.dts = av_rescale_q_rnd(packet.dts, in_stream->time_base, out_stream->time_base,
                                      AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX);
        packet.duration = av_rescale_q(packet.duration, in_stream->time_base, out_stream->time_base);
        packet.pos = -1;

        ret = av_interleaved_write_frame(output_format_context, &packet);
        if (ret < 0) {
            Logger::get().write("[Muxxer] Error muxing packet", true);
            break;
        }
        av_packet_unref(&packet);
        //write callback pos?

    }

    av_write_trailer(output_format_context);
    return true;

}
