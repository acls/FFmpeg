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

#include "decode_video.h"


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
static int jpeg_save(struct VDecoder* d, int frameNo) {
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
    const char *filename;
    struct VDecoder d;
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

    decoder_init(&d);

    fprintf(stderr, "sps\n");
    decode(&d, sizeof(sps), sps);
    fprintf(stderr, "pps\n");
    decode(&d, sizeof(pps), pps);

    ret = read_file_full(filename, &fileSize, &data);
    if (ret < 0) {
        fprintf(stderr, "[ERROR] Reading entire file\n");
        exit(1);
    }
    fprintf(stderr, "data\n");
    decode(&d, fileSize, data);

    fprintf(stderr, "d->frame 3 height %d width %d.\n", d.frame->height, d.frame->width);
    jpeg_save(&d, 1);

    decoder_free(&d);

    return 0;
}

int decoder_init(struct VDecoder* d) {
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
void decoder_free(struct VDecoder* d) {
    avcodec_free_context(&d->c);
    av_frame_free(&d->frame);
}

static
int decode_packet(struct VDecoder *d, AVPacket *pkt) {
    if (avcodec_send_packet(d->c, pkt) < 0) {
        return -1;
    }
    if (avcodec_receive_frame(d->c, d->frame) < 0) {
        return -1;
    }
    return 0;
}

int decode(struct VDecoder* d, long size, uint8_t *data) {
    int ret;
    AVPacket *pkt;
    pkt = av_packet_alloc();
    if (!pkt)
        return -1;
    pkt->size = size;
    pkt->data = data;

    ret = decode_packet(d, pkt);
    fprintf(stderr, "d->frame 2 height %d width %d.\n", d->frame->height, d->frame->width);

    av_packet_unref(pkt);

    return ret;
}