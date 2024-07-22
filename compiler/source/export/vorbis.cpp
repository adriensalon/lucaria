#include <fstream>
#include <vector>
#include <iostream>

#include <vorbis/vorbisenc.h>
#include <ogg/ogg.h>

#include <export/vorbis.hpp>

void export_vorbis(const audio_data& data, const std::filesystem::path& output_path)
{
    // Set up Vorbis and Ogg
    ogg_stream_state ogg_stream;
    ogg_page ogg_page;
    ogg_packet ogg_packet;

    vorbis_info vorbis_info;
    vorbis_comment vorbis_comment;
    vorbis_dsp_state vorbis_dsp;
    vorbis_block vorbis_block;

    vorbis_info_init(&vorbis_info);

    // Set up Vorbis codec (44100 Hz, 2 channels, 128 kbps)
    int ret = vorbis_encode_init_vbr(&vorbis_info, data.channels, 44100, 0.5f);
    if (ret) {
        std::cerr << "Failed to initialize Vorbis encoder" << std::endl;
        return;
    }

    vorbis_comment_init(&vorbis_comment);
    vorbis_analysis_init(&vorbis_dsp, &vorbis_info);
    vorbis_block_init(&vorbis_dsp, &vorbis_block);

    // Set up Ogg stream
    ogg_stream_init(&ogg_stream, rand());

    // Start the Ogg stream
    vorbis_analysis_headerout(&vorbis_dsp, &vorbis_comment, &ogg_packet, &ogg_packet, &ogg_packet);
    ogg_stream_packetin(&ogg_stream, &ogg_packet);

    // Open output file
    std::ofstream output_file(output_path, std::ios::binary);
    if (!output_file.is_open()) {
        std::cerr << "Failed to open output file" << std::endl;
        return;
    }

    // Write the initial header
    while (ogg_stream_pageout(&ogg_stream, &ogg_page)) {
        output_file.write(reinterpret_cast<const char*>(ogg_page.header), ogg_page.header_len);
        output_file.write(reinterpret_cast<const char*>(ogg_page.body), ogg_page.body_len);
    }

    // // Process the audio data
    // std::vector<float> buffer(data.length * data.channels);

    // // Convert audio_data to float buffer
    // for (size_t i = 0; i < data.samples.size(); ++i) {
    //     buffer[i] = data.samples[i];
    // }

    // // Feed data to Vorbis encoder
    // vorbis_analysis_buffer(&vorbis_dsp, 1024);

    // // Encode the audio data
    // while (vorbis_analysis_blockout(&vorbis_dsp, &vorbis_block) == 1) {
    //     vorbis_analysis(&vorbis_block, nullptr);
    //     vorbis_bitrate_addblock(&vorbis_block);

    //     while (vorbis_bitrate_flushpacket(&vorbis_dsp, &ogg_packet)) {
    //         ogg_stream_packetin(&ogg_stream, &ogg_packet);

    //         while (ogg_stream_pageout(&ogg_stream, &ogg_page)) {
    //             output_file.write(reinterpret_cast<const char*>(ogg_page.header), ogg_page.header_len);
    //             output_file.write(reinterpret_cast<const char*>(ogg_page.body), ogg_page.body_len);
    //         }
    //     }
    // }

    // // Finish the Ogg stream
    // ogg_stream_flush(&ogg_stream, &ogg_page);
    // output_file.write(reinterpret_cast<const char*>(ogg_page.header), ogg_page.header_len);
    // output_file.write(reinterpret_cast<const char*>(ogg_page.body), ogg_page.body_len);

    // // Clean up
    // vorbis_block_clear(&vorbis_block);
    // vorbis_dsp_clear(&vorbis_dsp);
    // vorbis_comment_clear(&vorbis_comment);
    // vorbis_info_clear(&vorbis_info);
    // ogg_stream_clear(&ogg_stream);
    
    // output_file.close();
    std::cout << "   Exporting vorbis audio " << output_path.filename() << std::endl;
}