
#include <iostream>
#include <algorithm>
#include <cassert>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <string>
#include <utility>
#include <vector>
#include <time.h>
#include "edgetpu_c.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

#include <opencv2/opencv.hpp>

#include "../utils/logger/vp_logger.h"
#include "vp_file_src_node.h"


namespace vp_nodes {
        
    vp_file_src_node::vp_file_src_node(std::string node_name, 
                                        int channel_index, 
                                        std::string file_path, 
                                        float resize_ratio, 
                                        bool cycle): 
                                        vp_src_node(node_name, channel_index, resize_ratio), 
                                        file_path(file_path), 
                                        cycle(cycle) {
        initialized();
    }
    
    vp_file_src_node::~vp_file_src_node() {

    }
    
    // define how to read video from local file, create frame meta etc.
    // please refer to the implementation of vp_node::handle_run.
    void vp_file_src_node::handle_run() {
        cv::Mat frame;
        int video_width = 0;
        int video_height = 0;
        int fps = 0;
        std::chrono::milliseconds delta;

        while(true) {
            // check if need work
            gate.knock();

            auto last_time = std::chrono::system_clock::now();
            // try to open capture
            if (!file_capture.isOpened()) {
                if(!file_capture.open(this->file_path)) {
                    VP_WARN(vp_utils::string_format("[%s] open file failed, try again...", node_name.c_str()));
                    continue;
                }
            }

            // video properties
            if (video_width == 0 || video_height == 0 || fps == 0) {
                video_width = file_capture.get(cv::CAP_PROP_FRAME_WIDTH);
                video_height = file_capture.get(cv::CAP_PROP_FRAME_HEIGHT);
                fps = file_capture.get(cv::CAP_PROP_FPS);
                delta = std::chrono::milliseconds(1000 / fps);
    
                original_fps = fps;
                original_width = video_width;
                original_height = video_height;

                // stream_info_hooker activated if need
                if (stream_info_hooker) {
                    vp_stream_info stream_info {channel_index, fps, video_width, video_height, file_path};
                    stream_info_hooker(node_name, stream_info);
                }
            }
            
            file_capture >> frame;
            //vp_nodes::FRAME = frame.clone();
            //cv::resize(frame, vp_nodes::FRAME, cv::Size(320,320));
            //vp_nodes::FRAME = resize_frame;
            if(frame.empty()) {
                VP_INFO(vp_utils::string_format("[%s] reading frame complete, total frame==>%d", node_name.c_str(), frame_index));
                if (cycle) {
                    VP_INFO(vp_utils::string_format("[%s] cycle flag is true, continue!", node_name.c_str()));
                    file_capture.set(cv::CAP_PROP_POS_FRAMES, 0);
                }
                continue;
            }

            // need resize
            cv::Mat resize_frame;
            if (this->resize_ratio != 1.0f) {                 
                cv::resize(frame, resize_frame, cv::Size(640, 480));
            }
            else {
                resize_frame = frame.clone();  // clone!
            }
            

            this->frame_index++;
            // create frame meta
            auto out_meta = 
                std::make_shared<vp_objects::vp_frame_meta>(resize_frame, this->frame_index, this->channel_index, video_width, video_height, fps);

            if (out_meta != nullptr) {
                this->out_queue.push(out_meta);

                // handled hooker activated if need
                if (this->meta_handled_hooker) {
                    meta_handled_hooker(node_name, out_queue.size(), out_meta);
                }

                // important! notify consumer of out_queue in case it is waiting.
                this->out_queue_semaphore.signal();
                VP_DEBUG(vp_utils::string_format("[%s] after handling meta, out_queue.size()==>%d", node_name.c_str(), out_queue.size()));
            }

            // for fps
            auto snap = std::chrono::system_clock::now() - last_time;
            snap = std::chrono::duration_cast<std::chrono::milliseconds>(snap);
            if (snap < delta) {
                std::this_thread::sleep_for(delta - snap);
            }
        }
    }

    // return stream path
    std::string vp_file_src_node::to_string() {
        return file_path;
    }
}
