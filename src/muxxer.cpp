#include "muxxer.h"
#include "utils.h"
#include "ut.h"

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

Muxxer::Muxxer(const char *in_filename, const char *out_filename){    
    using namespace boost::ut;

    int ret = 0;

    /* In */
    expect(avformat_open_input(&input_format_context, in_filename, nullptr, nullptr) == 0) << "Could not open input file!";

    expect(avformat_find_stream_info(input_format_context, nullptr) == 0) << "Failed to retrieve input stream information";

    /* Out */
    expect(avformat_alloc_output_context2(&output_format_context, nullptr, nullptr, out_filename) == 0) << "Could not create output context";

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

        out_stream = avformat_new_stream(output_format_context, nullptr);

        expect(out_stream) << "Failed allocating output stream";

        expect(avcodec_parameters_copy(out_stream->codecpar, in_codecpar) == 0) << "Failed to copy codec parameter";
    }

    if (!(output_format_context->oformat->flags & AVFMT_NOFILE))
        expect(avio_open(&output_format_context->pb, out_filename, AVIO_FLAG_WRITE) == 0) << "Could not open output file";

    expect(avformat_write_header(output_format_context, nullptr) == 0) << "Error occurred when opening output file";

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
            
        if(ret = av_interleaved_write_frame(output_format_context, &packet); ret !=0)
            break;

        av_packet_unref(&packet);

        //write callback pos?
        //one day
    }

    expect(ret != 0) << "Error muxing packet";

    av_write_trailer(output_format_context);
}

Muxxer::~Muxxer(){
    free(streams_list);

    avformat_close_input(&input_format_context);

    avformat_close_input(&output_format_context);
}

