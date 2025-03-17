/*
 * Copyright (c) 2001 Fabrice Bellard
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/**
 * @file libavcodec video decoding API usage example
 * @example decode_video.c *
 *
 * Read from an MPEG1 video file, decode frames, and generate PGM images as
 * output.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavcodec/mjpeg.h>



struct Decoder {
    // AVDictionary        *options;
    AVCodecContext      *c;
    AVFrame             *frame;

    // AVFrame             *dstFrame;
    // SwsContext          *swsCtx;
} Decoder;

static int decoder_init(struct Decoder* d) {
    const AVCodec *codec;

    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        return -1;
    }

    d->c = avcodec_alloc_context3(codec);
    if (!d->c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        return -1;
    }

    if (avcodec_open2(d->c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        return -1;
    }

    d->frame = av_frame_alloc();
    if (!d->frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        return -1;
    }

    return 0;
}
static void decoder_free(struct Decoder* d) {
    avcodec_free_context(&d->c);
    av_frame_free(&d->frame);
}

static int decode(struct Decoder *d, AVPacket *pkt,
                   const char *filename)
{
    // char filename_buf[1024];
    int ret;

    // dump(pkt->size, pkt->data);

    fprintf(stderr, "avcodec_send_packet\n");
    ret = avcodec_send_packet(d->c, pkt);
    fprintf(stderr, "avcodec_send_packet2\n");
    if (ret < 0) {
        return ret;
    }

    // while (ret >= 0) {
    //     fprintf(stderr, "while (ret >= 0) %d\n", ret);
    //     ret = avcodec_receive_frame(d->c, d->frame);
    //     if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
    //         fprintf(stderr, "avcodec_receive_frame ret %d\n", ret);
    //         return 0;
    //     } else if (ret < 0) {
    //         fprintf(stderr, "Error during decoding\n");
    //         return -1;
    //     }
    //     // printf("saving frame %3"PRId64"\n", d->c->frame_num);
    //     // fflush(stdout);
    //     // /* the picture is allocated by the d-> no need to
    //     //    free it */
    //     // snprintf(filename_buf, sizeof(filename_buf),
    //     //          "%s-%"PRId64, filename, d->c->frame_num);
    //     // fprintf(stderr, "pgm_save %s\n", filename);
    //     // pgm_save(d->frame->data[0], d->frame->linesize[0],
    //     //          d->frame->width, d->frame->height, filename_buf);
    //     fprintf(stderr, "d->frame height %d width %d.\n", d->frame->height, d->frame->width);
    // }

    ret = avcodec_receive_frame(d->c, d->frame);
	if (ret < 0) {
        return 0;
	}

    fprintf(stderr, "d->frame height %d width %d.\n", d->frame->height, d->frame->width);

    // // if frame size has changed, allocate needed objects
    // if (d->dstFrame == NULL || d->dstFrame->width != d->frame->width || d->dstFrame->height != d->frame->height) {
    //     if (d->dstFrame != NULL) {
    //         av_frame_free(&d->dstFrame);
    //     }
    //
    //     if (d->swsCtx != NULL) {
    //         sws_freeContext(d->swsCtx);
    //     }
    //
    //     d->dstFrame = av_frame_alloc();
    //     d->dstFrame->format = AV_PIX_FMT_RGBA;
    //     d->dstFrame->width = d->frame->width;
    //     d->dstFrame->height = d->frame->height;
    //     d->dstFrame->color_range = AVCOL_RANGE_JPEG;
    //     ret = av_frame_get_buffer(d->dstFrame, 1);
    //     if (ret < 0) {
    //         fprintf(stderr, "av_frame_get_buffer() failed\n");
    //         return -1;
    //     }
    //
    //     d->swsCtx = sws_getContext(d->frame->width, d->frame->height, AV_PIX_FMT_YUV420P,
    //         d->dstFrame->width, d->dstFrame->height, d->dstFrame->format, SWS_BILINEAR, NULL, NULL, NULL);
    //     if (d->swsCtx == NULL) {
    //         fprintf(stderr, "sws_getContext() failed\n");
    //         return -1;
    //     }
    //
    //     // dstFrameSize := av_image_get_buffer_size((int32)(d->dstFrame->format), d->dstFrame->width, d->dstFrame->height, 1);
    //     // d->dstFramePtr = (*[1 << 30]uint8)(unsafe.Pointer(d->dstFrame->data[0]))[:dstFrameSize:dstFrameSize];
    // }
    //
    // // convert color space from YUV420 to RGBA
    // ret = sws_scale(d->swsCtx, (const uint8_t * const *)d->frame->data, d->frame->linesize,
    //     0, d->frame->height, d->dstFrame->data, d->dstFrame->linesize);
    // if (ret < 0) {
    //     fprintf(stderr, "sws_scale() failed\n");
    //     return -1;
    // }

    fprintf(stderr, "decode ret 0\n");
    return 0;
}

static void decode_nalu(struct Decoder* d, long size, uint8_t *data,
                   const char *filename) {
    AVPacket *pkt;
    pkt = av_packet_alloc();
    if (!pkt)
        return;
    pkt->size = size;
    pkt->data = data;

    fprintf(stderr, "sps: %d\n", pkt->size);
    decode(d, pkt, filename);
    fprintf(stderr, "d->frame 2 height %d width %d.\n", d->frame->height, d->frame->width);

    av_packet_unref(pkt);
}

static void dump(long len, uint8_t* buffer) {
    size_t i;
    size_t readsz = 32;
    if (readsz > len) {
        readsz = len;
    }
    for (i = 0; i < readsz; i++)
        fprintf(stderr," 0x%02x", buffer[i]);
    fprintf(stderr,"\n");
}

static int read_file_full(const char* filename, long* fileSize, uint8_t** buffer) {
    FILE* file;
    size_t bytesRead;

    fprintf(stderr, "open: %s\n", filename);
    file = fopen(filename, "rb");
    if (file == NULL) {
        fprintf(stderr, "[ERROR] Failed to open file: %s\n", filename);
        return -1;
    }

    fprintf(stderr, "seek end: %s\n", filename);
    // Get the size of the file.
    fseek(file, 0, SEEK_END);
    *fileSize = ftell(file) + 4;
    fprintf(stderr, "seek start: %s\n", filename);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the buffer.
    fprintf(stderr, "malloc : %ld\n", (size_t)*fileSize);
    *buffer = (uint8_t*)malloc((size_t)*fileSize);
    if (*buffer == NULL) {
        fprintf(stderr, "[ERROR] Failed to allocate memory for buffer.\n");
        fclose(file);
        return -1;
    }

    // Read the file into the buffer.
// Leave 4 bytes in front for 0x00 0x00 0x00 0x01 prefix
    bytesRead = fread(*buffer+4, 1, ((size_t)*fileSize)-4, file);
    if (bytesRead != ((size_t)*fileSize)-4) {
        fprintf(stderr, "[ERROR] Failed to read file into buffer. %ld != %ld\n", bytesRead, (size_t)*fileSize);
        fclose(file);
        free(*buffer);
        return -1;
    }

    (*buffer)[0] = 0;
    (*buffer)[1] = 0;
    (*buffer)[2] = 0;
    (*buffer)[3] = 1;
    dump((size_t)*fileSize, *buffer);

    fclose(file);
    return 0;
}

static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
                     char *filename)
{
    FILE *f;
    int i;

    f = fopen(filename,"wb");
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}
static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
                   FILE *outfile)
{
    int ret;

    /* send the frame to the encoder */
    if (frame)
        printf("Send frame %3"PRId64"\n", frame->pts);

    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0) {
        fprintf(stderr, "Error sending a frame for encoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_packet(enc_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }

        printf("Write packet %3"PRId64" (size=%5d)\n", pkt->pts, pkt->size);
        fwrite(pkt->data, 1, pkt->size, outfile);
        av_packet_unref(pkt);
    }
}
static int jpeg_save(struct Decoder* d, int frameNo) {
    const AVCodec *jpegCodec;
    AVCodecContext *jpegContext;
    FILE *jpegFile;
    char jpegFilename[256];
    AVPacket *pkt;

    jpegCodec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
    if (!jpegCodec) {
        fprintf(stderr, "[ERROR] MJPEG codec not found.\n");
        return -1;
    }
    jpegContext = avcodec_alloc_context3(jpegCodec);
    if (!jpegContext) {
        fprintf(stderr, "[ERROR] Failed to allocate jpeg codec context.\n");
        return -1;
    }
    jpegContext->pix_fmt = d->c->pix_fmt;
    fprintf(stderr, "time_base before %d %d.\n", jpegContext->time_base.num, jpegContext->time_base.den);
    jpegContext->time_base = (AVRational){1,1};;
    fprintf(stderr, "time_base after %d %d.\n", jpegContext->time_base.num, jpegContext->time_base.den);
    fprintf(stderr, "height %d width %d.\n", d->frame->height, d->frame->width);
    jpegContext->height = d->frame->height;
    jpegContext->width = d->frame->width;

    if (avcodec_open2(jpegContext, jpegCodec, NULL) < 0) {
        fprintf(stderr, "eroar!!! 01.\n");
        return -1;
    }

    pkt = av_packet_alloc();
    if (!pkt){
        fprintf(stderr, "eroar!!! 02.\n");
        return -1;
    }

    // if (avcodec_encode_video2(jpegContext, pkt, pFrame, &gotFrame) < 0) {
    //     return -1;
    // }

    sprintf(jpegFilename, "dvr-%06d.jpg", frameNo);
    jpegFile = fopen(jpegFilename, "wb");
    encode(jpegContext, d->frame, pkt, jpegFile);
    // fwrite(pkt->data, 1, pkt->size, jpegFile);
    fclose(jpegFile);

    av_packet_unref(pkt);
    avcodec_free_context(&jpegContext);
    return 0;
}

int main(int argc, char **argv)
{
    const char *filename, *outfilename;
    struct Decoder d;
    // AVCodecContext *c= NULL;
    // AVFrame *frame;
    long fileSize;
    uint8_t *data;
    int ret;
    // AVPacket *pkt;
    // 00000000  00 00 00 01 67 4d 40 2a  8d 8d 20 0f 00 44 fc b8  |....gM@*.. ..D..|
    // 00000010  0b 70 10 10 10 20                                 |.p... |
    char sps[] = {  0x00, 0x00, 0x00, 0x01, 0x67, 0x4d, 0x40, 0x2a,
                    0x8d, 0x8d, 0x20, 0x0f, 0x00, 0x44, 0xfc, 0xb8,
                    0x0b, 0x70, 0x10, 0x10, 0x10, 0x20 };
    // 00000000  00 00 00 01 68 ee 38 80                           |....h.8.|
    char pps[] = {  0x00, 0x00, 0x00, 0x01, 0x68, 0xee, 0x38, 0x80 };



    if (argc <= 2) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n"
                "And check your input file is encoded by mpeg1video please.\n", argv[0]);
        exit(0);
    }
    filename    = argv[1];
    outfilename = argv[2];

    decoder_init(&d);

    fprintf(stderr, "sps\n");
    decode_nalu(&d, sizeof(sps), sps, outfilename);
    fprintf(stderr, "pps\n");
    decode_nalu(&d, sizeof(pps), pps, outfilename);

    ret = read_file_full(filename, &fileSize, &data);
    if (ret < 0) {
        fprintf(stderr, "[ERROR] Reading entire file\n");
        exit(1);
    }
    fprintf(stderr, "data\n");
    decode_nalu(&d, fileSize, data, outfilename);

    fprintf(stderr, "d->frame 3 height %d width %d.\n", d.frame->height, d.frame->width);
    jpeg_save(&d, 1);

    decoder_free(&d);

    return 0;
}
