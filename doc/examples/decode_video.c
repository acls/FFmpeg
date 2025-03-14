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

    fprintf(stderr, "fileSize: %ld\n", (size_t)*fileSize);
    fprintf(stderr, "strlen: %ld\n", strlen(*buffer));

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

static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt,
                   const char *filename)
{
    char filename_buf[1024];
    int ret;

    dump(pkt->size, pkt->data);

    fprintf(stderr, "avcodec_send_packet\n");
    ret = avcodec_send_packet(dec_ctx, pkt);
    fprintf(stderr, "avcodec_send_packet2\n");
    if (ret < 0) {
        fprintf(stderr, "Error sending a packet for decoding: %d\n", ret);
        // exit(1);
        return;
    }

    fprintf(stderr, "ret %d\n", ret);
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }

        printf("saving frame %3"PRId64"\n", dec_ctx->frame_num);
        fflush(stdout);

        /* the picture is allocated by the decoder. no need to
           free it */
        snprintf(filename_buf, sizeof(filename_buf),
                 "%s-%"PRId64, filename, dec_ctx->frame_num);
        fprintf(stderr, "pgm_save %s\n", filename);
        pgm_save(frame->data[0], frame->linesize[0],
                 frame->width, frame->height, filename_buf);
    }
}

// void prepend(long len, uint8_t* s, long pre_len, const uint8_t* pre) {
//     memmove(s + pre_len, s, len); // Move the original string to make space for the new string
//     memcpy(s, pre, len); // Copy the new string into the buffer
// }
// void copy(unsigned *restrict const dst, unsigned const *restrict const src, unsigned long n)
// {
//     for (unsigned long x = 0; x < n; ++x)
//     {
//         dst[x] = src[x];
//     }
// }

int main(int argc, char **argv)
{
    const char *filename, *outfilename;
    const AVCodec *codec;
    AVCodecContext *c= NULL;
    AVFrame *frame;
    long fileSize;
    uint8_t *data;
    int ret;
    // AVPacket avPkt;
    AVPacket *pkt;
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

    pkt = av_packet_alloc();
    if (!pkt)
        exit(1);


    /* find the MPEG-1 video decoder */
    codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        fprintf(stderr, "Codec not found\n");
        exit(1);
    }

    c = avcodec_alloc_context3(codec);
    if (!c) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
       MUST be initialized there because this information is not
       available in the bitstream. */

    /* open it */
    if (avcodec_open2(c, codec, NULL) < 0) {
        fprintf(stderr, "Could not open codec\n");
        exit(1);
    }

    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    pkt->size = sizeof(sps);
    pkt->data = sps;
    fprintf(stderr, "sps: %d\n", pkt->size);
    decode(c, frame, pkt, outfilename);

    pkt->size = sizeof(pps);
    pkt->data = pps;
    fprintf(stderr, "pps: %d\n", pkt->size);
    decode(c, frame, pkt, outfilename);

    ret = read_file_full(filename, &fileSize, &data);
    if (ret < 0) {
        fprintf(stderr, "[ERROR] Reading entire file\n");
        exit(1);
    }
    fprintf(stderr, "data\n");
    pkt->size = fileSize;
    pkt->data = data;
    decode(c, frame, pkt, outfilename);

    avcodec_free_context(&c);
    av_frame_free(&frame);
    av_packet_free(&pkt);

    return 0;
}
