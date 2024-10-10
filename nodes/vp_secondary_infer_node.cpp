
#include "vp_secondary_infer_node.h"

namespace vp_nodes {
        
    vp_secondary_infer_node::vp_secondary_infer_node(std::string node_name, 
                            std::string model_path, 
			    std::string convert_type,
                            std::string model_config_path,
                            std::string labels_path,
                            int input_width, 
                            int input_height, 
                            int batch_size,
                            std::vector<int> p_class_ids_applied_to,
                            int crop_padding,
                            float scale,
                            cv::Scalar mean,
                            cv::Scalar,
                            bool swap_rb):
                            vp_infer_node(node_name, 
                                        vp_nodes::vp_infer_type::SECONDARY, 
                                        model_path, 
					convert_type,
                                        model_config_path, 
                                        labels_path, 
                                        input_width, 
                                        input_height,
                                        batch_size, 
                                        scale, 
                                        mean, 
                                        std, 
                                        swap_rb),
                            p_class_ids_applied_to(p_class_ids_applied_to),
                            crop_padding(crop_padding) {
    }
    
    vp_secondary_infer_node::~vp_secondary_infer_node() {
    }

    void vp_secondary_infer_node::prepare(const std::vector<std::shared_ptr<vp_objects::vp_frame_meta>>& frame_meta_with_batch, std::vector<cv::Mat>& mats_to_infer) {
        // only one by one for secondary infer node
        assert(frame_meta_with_batch.size() == 1);

        // crop to get small images in frame 
        auto& frame_meta = frame_meta_with_batch[0];

        // batch by batch inside single frame
        for (auto& i : frame_meta->face_targets) {
            // check if we need infer on the target
            //if (!need_apply(i->primary_class_id)) {
               //continue;
            //}
            
            // simulate croping operations, no data copyed here
            auto box = cv::Rect(i->x, i->y, i->width, i->height);

            // add a padding when crop, check value range
            if (crop_padding != 0) {
                box = cv::Rect(box.x - crop_padding, box.y - crop_padding, box.width + crop_padding * 2, box.height + crop_padding * 2);
                box.x = std::max(box.x, 0);
                box.y = std::max(box.y, 0);
                box.width = std::min(box.width, frame_meta->frame.cols - box.x);
                box.height = std::min(box.height, frame_meta->frame.rows - box.y);
            }
            
            mats_to_infer.push_back(frame_meta->frame(box)); 
        }
    }

    bool vp_secondary_infer_node::need_apply(int primary_class_id) {
        if (p_class_ids_applied_to.size() == 0) {
            return true;
        }
        
        return std::find(p_class_ids_applied_to.begin(), p_class_ids_applied_to.end(), primary_class_id) != p_class_ids_applied_to.end();
    }
}
