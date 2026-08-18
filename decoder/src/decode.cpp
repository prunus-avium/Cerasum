// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2026 prunus-avium

#include <decode.hpp>

#include <any>
#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <filesystem>
#include <memory>
#include <sys/types.h>
#include <vector>

#include <libavcodec/avcodec.h>
#include <libavcodec/codec.h>
#include <libavcodec/packet.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/channel_layout.h>
#include <libavutil/error.h>
#include <libavutil/frame.h>
#include <libavutil/mem.h>
#include <libavutil/opt.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>

#include <utils.hpp>

static cerasum::utils::error::ErrorCode toCerasumErrorCode(int32_t errorc) {
    switch (errorc) {
    case AVERROR(ENOENT):
        return cerasum::utils::error::ErrorCode::FileNotFound;
    case AVERROR(EACCES):
    case AVERROR(EPERM):
        return cerasum::utils::error::ErrorCode::PermissionDenied;
    case AVERROR(EIO):
        return cerasum::utils::error::ErrorCode::IOError;
    case AVERROR(ENOSPC):
        return cerasum::utils::error::ErrorCode::DiskFull;
    case AVERROR(ENOMEM):
        return cerasum::utils::error::ErrorCode::OutOfMemory;
    case AVERROR(EINVAL):
        return cerasum::utils::error::ErrorCode::InvalidArgument;
    case AVERROR_INVALIDDATA:
        return cerasum::utils::error::ErrorCode::CorruptedData;
    case AVERROR(EPIPE):
    case AVERROR(ECONNRESET):
    case AVERROR(ECONNREFUSED):
    case AVERROR(ETIMEDOUT):
        return cerasum::utils::error::ErrorCode::NetworkError;
    case AVERROR_DECODER_NOT_FOUND:
        return cerasum::utils::error::ErrorCode::CodecNotFound;
    case AVERROR_DEMUXER_NOT_FOUND:
        return cerasum::utils::error::ErrorCode::FormatNotSupported;
    case AVERROR_ENCODER_NOT_FOUND:
        return cerasum::utils::error::ErrorCode::EncoderNotFound;
    case AVERROR_MUXER_NOT_FOUND:
        return cerasum::utils::error::ErrorCode::MuxerNotFound;
    case AVERROR_FILTER_NOT_FOUND:
        return cerasum::utils::error::ErrorCode::FilterNotFound;
    case AVERROR_PROTOCOL_NOT_FOUND:
        return cerasum::utils::error::ErrorCode::ProtocolNotSupported;
    case AVERROR_STREAM_NOT_FOUND:
        return cerasum::utils::error::ErrorCode::StreamNotFound;
    case AVERROR_BSF_NOT_FOUND:
        return cerasum::utils::error::ErrorCode::BitstreamFilterNotFound;
    case AVERROR_OPTION_NOT_FOUND:
        return cerasum::utils::error::ErrorCode::OptionNotFound;
    case AVERROR_EOF:
        return cerasum::utils::error::ErrorCode::EndOfFile;
    case AVERROR_EXIT:
        return cerasum::utils::error::ErrorCode::Aborted;
    case AVERROR_EXTERNAL:
        return cerasum::utils::error::ErrorCode::ExternalLibraryError;
    case AVERROR_BUG:
    case AVERROR_BUG2:
        return cerasum::utils::error::ErrorCode::InternalBug;
    case AVERROR_PATCHWELCOME:
        return cerasum::utils::error::ErrorCode::NotImplemented;
    case AVERROR_EXPERIMENTAL:
        return cerasum::utils::error::ErrorCode::ExperimentalNotEnabled;
    case AVERROR_BUFFER_TOO_SMALL:
        return cerasum::utils::error::ErrorCode::BufferTooSmall;
    case AVERROR_UNKNOWN:
        return cerasum::utils::error::ErrorCode::Unknown;
    case AVERROR_HTTP_BAD_REQUEST:
        return cerasum::utils::error::ErrorCode::HttpBadRequest;
    case AVERROR_HTTP_UNAUTHORIZED:
        return cerasum::utils::error::ErrorCode::HttpUnauthorized;
    case AVERROR_HTTP_FORBIDDEN:
        return cerasum::utils::error::ErrorCode::HttpForbidden;
    case AVERROR_HTTP_NOT_FOUND:
        return cerasum::utils::error::ErrorCode::HttpNotFound;
    case AVERROR_HTTP_OTHER_4XX:
        return cerasum::utils::error::ErrorCode::HttpClientError;
    case AVERROR_HTTP_SERVER_ERROR:
        return cerasum::utils::error::ErrorCode::HttpServerError;
    case AVERROR_INPUT_CHANGED:
        return cerasum::utils::error::ErrorCode::InputFormatChanged;
    case AVERROR_OUTPUT_CHANGED:
        return cerasum::utils::error::ErrorCode::OutputFormatChanged;
    default:
        return cerasum::utils::error::ErrorCode::Unknown;
    }
}

namespace FullDecode {
cerasum::utils::error::ErrorCode decodeInitFull1(const std::filesystem::path &file, AVFormatContext **context, uint32_t &index,
                                        AVStream **stream) {
    int32_t errorc = avformat_open_input(context, file.c_str(), nullptr, nullptr);
    if (errorc != 0) {
        return toCerasumErrorCode(errorc);
    }
    errorc = avformat_find_stream_info(*context, nullptr);
    if (errorc < 0) {
        avformat_close_input(context);
        return toCerasumErrorCode(errorc);
    }

    for (uint32_t i = 0; i < (*context)->nb_streams; i++) {
        if ((*context)->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            index = i;
            break;
        }
    }
    if (index == -1U) {
        avformat_close_input(context);
        return cerasum::utils::error::ErrorCode::StreamNotFound;
    }

    *stream = (*context)->streams[index];
    return cerasum::utils::error::ErrorCode::Succuss;
}

cerasum::utils::error::ErrorCode decodeInitFull2(AVFormatContext **context, AVStream **stream, const AVCodec *codec,
                                        AVCodecContext **codecContext, SwrContext **swrContext, AVPacket **packet,
                                        AVFrame **frame, uint8_t **buffer, size_t maxSamples) {
    *codecContext = avcodec_alloc_context3(codec);
    if (*codecContext == nullptr) {
        avformat_close_input(context);
        return cerasum::utils::error::ErrorCode::AllocateContextFault;
    }
    int32_t errorc = avcodec_parameters_to_context(*codecContext, (*stream)->codecpar);
    if (errorc < 0) {
        avcodec_free_context(codecContext);
        avformat_close_input(context);
        return toCerasumErrorCode(errorc);
    }
    errorc = avcodec_open2(*codecContext, codec, nullptr);
    if (errorc < 0) {
        avcodec_free_context(codecContext);
        avformat_close_input(context);
        return toCerasumErrorCode(errorc);
    }

    AVChannelLayout layout;
    av_channel_layout_default(&layout, 2);
    *swrContext = swr_alloc();
    if (*swrContext == nullptr) {
        avcodec_free_context(codecContext);
        avformat_close_input(context);
        return cerasum::utils::error::ErrorCode::AllocateContextFault;
    }
    errorc = swr_alloc_set_opts2(swrContext, &layout, AV_SAMPLE_FMT_S16, 44100, &(*codecContext)->ch_layout,
                                 (*codecContext)->sample_fmt, (*codecContext)->sample_rate, 0, nullptr);
    if (errorc < 0) {
        avcodec_free_context(codecContext);
        avformat_close_input(context);
        return toCerasumErrorCode(errorc);
    }
    errorc = swr_init(*swrContext);
    if (errorc < 0) {
        swr_free(swrContext);
        avcodec_free_context(codecContext);
        avformat_close_input(context);
        return toCerasumErrorCode(errorc);
    }

    *packet = av_packet_alloc();
    *frame = av_frame_alloc();
    *buffer = reinterpret_cast<uint8_t *>(av_malloc(maxSamples * 2 * 2));
    if (*packet == nullptr || *frame == nullptr || *buffer == nullptr) {
        av_free(*buffer);
        av_frame_free(frame);
        av_packet_free(packet);
        swr_free(swrContext);
        avcodec_free_context(codecContext);
        avformat_close_input(context);
        return cerasum::utils::error::ErrorCode::AllocateBufferFault;
    }

    return cerasum::utils::error::ErrorCode::Succuss;
}

cerasum::utils::error::ErrorCode decoding(AVFormatContext **context, uint32_t index, AVCodecContext **codecContext,
                                 SwrContext **swrContext, AVPacket **packet, AVFrame **frame, uint8_t **buffer,
                                 size_t maxSamples, std::vector<uint8_t> &pcm) {
    int32_t errorc;
    while (true) {
        errorc = av_read_frame(*context, *packet);
        if (errorc < 0) {
            if (errorc != AVERROR_EOF) {
                av_free(*buffer);
                av_frame_free(frame);
                av_packet_free(packet);
                swr_free(swrContext);
                avcodec_free_context(codecContext);
                avformat_close_input(context);
                return toCerasumErrorCode(errorc);
            }
            break;
        }

        if (static_cast<uint32_t>((*packet)->stream_index) != index) {
            av_packet_unref(*packet);
            continue;
        }
        errorc = avcodec_send_packet(*codecContext, *packet);
        av_packet_unref(*packet);
        if (errorc < 0 && errorc != AVERROR(EAGAIN)) {
            av_free(*buffer);
            av_frame_free(frame);
            av_packet_free(packet);
            swr_free(swrContext);
            avcodec_free_context(codecContext);
            avformat_close_input(context);
            return toCerasumErrorCode(errorc);
        }

        while ((errorc = avcodec_receive_frame(*codecContext, *frame)) >= 0) {
            int32_t outSamples =
                swr_convert(*swrContext, buffer, static_cast<int32_t>(maxSamples),
                            reinterpret_cast<const uint8_t *const *>((*frame)->extended_data), (*frame)->nb_samples);

            if (outSamples > 0) {
                int32_t outSize = av_samples_get_buffer_size(nullptr, 2, outSamples, AV_SAMPLE_FMT_S16, 1);
                size_t oldSize = pcm.size();
                pcm.resize(oldSize + outSize);
                memcpy(pcm.data() + oldSize, *buffer, outSize);
            }

            av_frame_unref(*frame);
        }
    }

    errorc = avcodec_send_packet(*codecContext, nullptr);
    if (errorc < 0 && errorc != AVERROR_EOF) {
        av_free(*buffer);
        av_frame_free(frame);
        av_packet_free(packet);
        swr_free(swrContext);
        avcodec_free_context(codecContext);
        avformat_close_input(context);
        return toCerasumErrorCode(errorc);
    }
    while (true) {
        errorc = avcodec_receive_frame(*codecContext, *frame);
        if (errorc < 0) {
            if (errorc != AVERROR_EOF) {
                av_free(*buffer);
                av_frame_free(frame);
                av_packet_free(packet);
                swr_free(swrContext);
                avcodec_free_context(codecContext);
                avformat_close_input(context);
                return toCerasumErrorCode(errorc);
            }
            break;
        }

        int32_t outSamples =
            swr_convert(*swrContext, buffer, static_cast<int32_t>(maxSamples),
                        reinterpret_cast<const uint8_t *const *>((*frame)->extended_data), (*frame)->nb_samples);
        if (outSamples > 0) {
            int32_t outSize = av_samples_get_buffer_size(nullptr, 2, outSamples, AV_SAMPLE_FMT_S16, 1);
            size_t oldSize = pcm.size();
            pcm.resize(oldSize + outSize);
            memcpy(pcm.data() + oldSize, *buffer, outSize);
        }
        av_frame_unref(*frame);
    }

    av_free(*buffer);
    av_frame_free(frame);
    av_packet_free(packet);
    swr_free(swrContext);
    avcodec_free_context(codecContext);
    avformat_close_input(context);

    return cerasum::utils::error::ErrorCode::Succuss;
}
}

namespace cerasum::decoder {
std::expected<Stream, utils::error::ErrorCode> decode(const std::filesystem::path &file) {
    AVFormatContext *context = nullptr;
    uint32_t index;
    AVStream *stream;

    utils::error::ErrorCode error;

    error = FullDecode::decodeInitFull1(file, &context, index, &stream);
    if (error != utils::error::ErrorCode::Succuss) {
        return std::unexpected(error);
    }

    const AVCodec *codec = avcodec_find_decoder(stream->codecpar->codec_id);
    if (codec == nullptr) {
        avformat_close_input(&context);
        return std::unexpected(utils::error::ErrorCode::CodecNotFound);
    }

    AVCodecContext *codecContext;
    SwrContext *swrContext;
    AVPacket *packet;
    AVFrame *frame;
    uint8_t *buffer;

    constexpr size_t maxSamples = 4096;

    error = FullDecode::decodeInitFull2(&context, &stream, codec, &codecContext, &swrContext, &packet, &frame, &buffer, maxSamples);
    if (error != utils::error::ErrorCode::Succuss) {
        return std::unexpected(error);
    }

    std::vector<uint8_t> pcm;

    error = FullDecode::decoding(&context, index, &codecContext, &swrContext, &packet, &frame, &buffer, maxSamples, pcm);
    if (error != utils::error::ErrorCode::Succuss) {
        return std::unexpected(error);
    }

    if (pcm.empty()) {
        return std::unexpected(utils::error::ErrorCode::NoPcmData);
    }

    Stream result;

    result.data = std::make_unique<uint8_t[]>(pcm.size()); // NOLINT
    memcpy(result.data.get(), pcm.data(), pcm.size());
    result.sampleRate = 44100;
    result.channels = 2;

    return result;
}

std::expected<std::any, utils::error::ErrorCode> encode(const Stream &pcm) {}
} // namespace cerasum::decoder