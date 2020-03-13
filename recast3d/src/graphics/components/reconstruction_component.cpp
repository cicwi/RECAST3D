#include <iostream>
#include <tomop/tomop.hpp>

#include <glm/gtc/constants.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/transform.hpp>
#include <imgui.h>

#include "graphics/colormap.hpp"
#include "graphics/components/reconstruction_component.hpp"
#include "graphics/primitives.hpp"
#include "graphics/scene_camera_3d.hpp"

namespace tomovis {

using namespace tomop;

ReconstructionComponent::ReconstructionComponent(SceneObject& object,
                                                 int scene_id)
    : object_(object), volume_texture_(16, 16, 16), scene_id_(scene_id) {
    glGenVertexArrays(1, &vao_handle_);
    glBindVertexArray(vao_handle_);
    glGenBuffers(1, &vbo_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), square(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &line_vao_handle_);
    glBindVertexArray(line_vao_handle_);
    glGenBuffers(1, &line_vbo_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), line(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenVertexArrays(1, &cube_vao_handle_);
    glBindVertexArray(cube_vao_handle_);

    cube_index_count_ = 24;
    glGenBuffers(1, &cube_index_handle_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube_index_handle_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cube_index_count_ * sizeof(GLuint),
                 cube_wireframe_idxs(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glGenBuffers(1, &cube_vbo_handle_);
    glBindBuffer(GL_ARRAY_BUFFER, cube_vbo_handle_);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), cube_wireframe(),
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    auto simple_vert =
#include "../src/shaders/simple_3d.vert"
        ;
    auto simple_frag =
#include "../src/shaders/simple_3d.frag"
        ;

    program_ = std::make_unique<ShaderProgram>(simple_vert, simple_frag, false);

    auto cube_vert =
#include "../src/shaders/wireframe_cube.vert"
        ;
    auto cube_frag =
#include "../src/shaders/wireframe_cube.frag"
        ;
    cube_program_ =
        std::make_unique<ShaderProgram>(cube_vert, cube_frag, false);

    slices_[0] = std::make_unique<slice>(0);
    slices_[1] = std::make_unique<slice>(1);
    slices_[2] = std::make_unique<slice>(2);

    // slice along axis 0 = x
    slices_[0]->set_orientation(glm::vec3(0.0f, -1.0f, -1.0f),
                                glm::vec3(0.0f, 2.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, 2.0f));

    // slice along axis 1 = y
    slices_[1]->set_orientation(glm::vec3(-1.0f, 0.0f, -1.0f),
                                glm::vec3(2.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 0.0f, 2.0f));

    // slice along axis 2 = z
    slices_[2]->set_orientation(glm::vec3(-1.0f, -1.0f, 0.0f),
                                glm::vec3(2.0f, 0.0f, 0.0f),
                                glm::vec3(0.0f, 2.0f, 0.0f));

    send_slices();

    set_volume_position(glm::vec3(-1.0f), glm::vec3(1.0f));
    colormap_texture_ = object.camera().colormap();
}

ReconstructionComponent::~ReconstructionComponent() {
    glDeleteVertexArrays(1, &cube_vao_handle_);
    glDeleteBuffers(1, &cube_vbo_handle_);
    glDeleteVertexArrays(1, &vao_handle_);
    glDeleteBuffers(1, &vbo_handle_);
    // FIXME FULLY DELETE CUBE AND LINE
    glDeleteVertexArrays(1, &cube_vao_handle_);
    glDeleteBuffers(1, &cube_vbo_handle_);
    glDeleteBuffers(1, &cube_index_handle_);
    glDeleteVertexArrays(1, &line_vao_handle_);
    glDeleteBuffers(1, &line_vbo_handle_);
}

void ReconstructionComponent::send_slices() {
    for (auto& slice : slices_) {
        auto packet = SetSlicePacket(scene_id_, slice.first,
                                     slice.second->packed_orientation());
        object_.send(packet);
    }

    for (auto& slice : fixed_slices_) {
        auto packet = SetSlicePacket(scene_id_, slice.first,
                                     slice.second->packed_orientation());
        object_.send(packet);
    }
}

void ReconstructionComponent::set_data(std::vector<float>& data,
                                       std::array<int32_t, 2> size, int slice_idx,
                                       bool additive) {
    slice* s = nullptr;
    if (slices_.find(slice_idx) != slices_.end()) {
        s = slices_[slice_idx].get();
    } else if (fixed_slices_.find(slice_idx) != fixed_slices_.end()) {
        s = fixed_slices_[slice_idx].get();
    } else {
        std::cout << "Updating inactive slice: " << slice_idx << "\n";
        return;
    }

    if (s == dragged_slice_) {
        return;
    }

    if (!additive || !s->has_data()) {
        s->size = size;
        s->data = data;
    } else {
        assert(s->size == size);
        s->add_data(data);
    }

    s->min_value = *std::min_element(s->data.begin(),
                                                  s->data.end());
    s->max_value = *std::max_element(s->data.begin(),
                                                  s->data.end());

    update_image_(s);
}

void ReconstructionComponent::update_partial_slice(
    std::vector<float>& data, std::array<int32_t, 2> offset,
    std::array<int32_t, 2> size, std::array<int32_t, 2> global_size, int slice,
    bool additive) {
    if (slices_.find(slice) == slices_.end()) {
        std::cout << "Updating inactive slice: " << slice << "\n";
        return;
    }
    auto& the_slice = slices_[slice];
    if (!additive || !slices_[slice]->has_data()) {
        the_slice->size = global_size;
        the_slice->data.resize(size[0] * size[1]);
        std::fill(the_slice->data.begin(), the_slice->data.end(), 0);
        the_slice->add_partial_data(data, offset, size);
    } else {
        assert(global_size == the_slice->size);
        slices_[slice]->add_partial_data(data, offset, size);
    }

    slices_[slice]->min_value = *std::min_element(slices_[slice]->data.begin(),
                                                  slices_[slice]->data.end());
    slices_[slice]->max_value = *std::max_element(slices_[slice]->data.begin(),
                                                  slices_[slice]->data.end());

    update_image_(slices_[slice].get());
}

void ReconstructionComponent::set_volume_data(
    std::vector<float>& data, std::array<int32_t, 3>& volume_size) {
    volume_data_ = data;
    volume_texture_.set_data(volume_size[0], volume_size[1], volume_size[2],
                             data);
    update_histogram(data);
}

void ReconstructionComponent::update_partial_volume(
    std::vector<float>& data, std::array<int32_t, 3>& offset,
    std::array<int32_t, 3>& size, std::array<int32_t, 3>& global_size) {
    if ((int)volume_data_.size() !=
        global_size[0] * global_size[1] * global_size[2]) {
        volume_data_ = std::vector<float>(
            global_size[0] * global_size[1] * global_size[2], 0.0f);
    }

    int idx = 0;
    for (auto k = offset[2]; k < size[2] + offset[2]; ++k) {
        for (auto j = offset[1]; j < size[1] + offset[1]; ++j) {
            for (auto i = offset[0]; i < size[0] + offset[0]; ++i) {
                volume_data_[k * global_size[0] * global_size[1] +
                             j * global_size[0] + i] = data[idx++];
            }
        }
    }

    volume_texture_.set_data(global_size[0], global_size[1], global_size[2],
                             volume_data_);
    update_histogram(volume_data_);
}

void ReconstructionComponent::update_histogram(const std::vector<float>& data) {
    auto bins = 30;
    auto min = *std::min_element(data.begin(), data.end());
    auto max = *std::max_element(data.begin(), data.end());
    if (max == min) {
        max = min + 1;
    }

    histogram_.clear();
    histogram_.resize(bins);

    for (auto x : data) {
        auto bin = (int)(((x - min) / (max - min)) * (bins - 1));
        if (bin < 0) {
            bin = 0;
        }
        if (bin >= bins) {
            bin = bins - 1;
        }
        histogram_[bin] += 1.0f;
    }

    volume_min_ = min;
    volume_max_ = max;
}

void ReconstructionComponent::describe() {
    ImGui::Checkbox("Show reconstruction", &show_);
    ImGui::Checkbox("Transparent mode (experimental)", &transparency_mode_);

    auto window_size = ImGui::GetWindowSize();
    ImGui::PlotHistogram("Reconstruction histogram", histogram_.data(),
                         histogram_.size(), 0, NULL, FLT_MAX, FLT_MAX,
                         ImVec2(window_size.x, 128));

    auto minmax = overall_min_and_max();
    ImGui::SliderFloat("Lower", &lower_value_, minmax.first, minmax.second);
    ImGui::SliderFloat("Upper", &upper_value_, minmax.first, minmax.second);

    if (fixed_slices_.size() > 0)  {
        ImGui::Begin("Slices");
        auto to_remove = std::vector<int>{};
        for (auto&& [slice_idx, slice] : fixed_slices_) {
            ImGui::Image((void*)(intptr_t)slice->get_texture().id(), ImVec2(200, 200));
            if (ImGui::Button((std::string("remove##") + std::to_string(slice_idx)).c_str())) {
                to_remove.push_back(slice_idx);
            }
        }
        for (auto remove : to_remove) {
            fixed_slices_.erase(remove);
        }
        ImGui::End();
    }
}

std::pair<float, float> ReconstructionComponent::overall_min_and_max() {
    auto overall_min = std::numeric_limits<float>::max();
    auto overall_max = std::numeric_limits<float>::min();
    for (auto&& [slice_idx, slice] : slices_) {
        (void)slice_idx;
        overall_min =
            slice->min_value < overall_min ? slice->min_value : overall_min;
        overall_max =
            slice->max_value > overall_max ? slice->max_value : overall_max;
    }

    return {overall_min - (0.2f * (overall_max - overall_min)),
            overall_max + (0.2f * (overall_max - overall_min))};
}

void ReconstructionComponent::update_image_(slice* s) {
    auto nonzero = std::fabs(s->min_value) > 1e-6 &&
                   std::fabs(s->max_value) > 1e-6;
    if (value_not_set_ && nonzero) {
        lower_value_ = s->min_value;
        upper_value_ = s->max_value;
        value_not_set_ = false;
    }
    s->update_texture();
}

void ReconstructionComponent::set_volume_position(glm::vec3 min_pt,
                                                  glm::vec3 max_pt) {
    auto center = 0.5f * (min_pt + max_pt);
    volume_transform_ = glm::translate(center) *
                        glm::scale(glm::vec3(max_pt - min_pt)) *
                        glm::scale(glm::vec3(0.5f));
    object_.camera().set_look_at(center);
}

void ReconstructionComponent::draw(glm::mat4 world_to_screen) {
    if (!show_) {
        return;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    program_->use();

    program_->uniform("texture_sampler", 0);
    program_->uniform("colormap_sampler", 1);
    program_->uniform("volume_data_sampler", 3);

    program_->uniform("min_value", lower_value_);
    program_->uniform("max_value", upper_value_);
    program_->uniform("volume_min_value", volume_min_);
    program_->uniform("volume_max_value", volume_max_);
    program_->uniform("transparency_mode", (int)transparency_mode_);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_1D, colormap_texture_);

    auto full_transform = world_to_screen * volume_transform_;

    auto draw_slice = [&](slice& the_slice) {
        the_slice.get_texture().bind();

        program_->uniform("world_to_screen_matrix", full_transform);
        program_->uniform("orientation_matrix",
                          the_slice.orientation *
                              glm::translate(glm::vec3(0.0, 0.0, 1.0)));
        program_->uniform("hovered", (int)(the_slice.hovered));
        program_->uniform("has_data", (int)(the_slice.has_data()));

        glBindVertexArray(vao_handle_);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

        the_slice.get_texture().unbind();
    };

    std::vector<slice*> slices;
    for (auto& id_slice : slices_) {
        if (id_slice.second.get()->inactive) {
            continue;
        }
        slices.push_back(id_slice.second.get());
    }
    std::sort(slices.begin(), slices.end(), [](auto& lhs, auto& rhs) -> bool {
        if (rhs->transparent() == lhs->transparent()) {
            return rhs->id < lhs->id;
        }
        return rhs->transparent();
    });

    volume_texture_.bind();
    for (auto& slice : slices) {
        draw_slice(*slice);
    }
    volume_texture_.unbind();

    cube_program_->use();
    cube_program_->uniform("transform_matrix", full_transform);
    cube_program_->uniform("line_color", glm::vec4(0.5f, 0.5f, 0.5f, 0.3f));

    glBindVertexArray(cube_vao_handle_);
    glLineWidth(3.0f);
    glDrawElements(GL_LINES, cube_index_count_, GL_UNSIGNED_INT, nullptr);

    glDisable(GL_DEPTH_TEST);

    if (drag_machine_ &&
        drag_machine_->kind() == recon_drag_machine_kind::rotator) {
        auto& rotator = *(SliceRotator*)drag_machine_.get();
        cube_program_->uniform(
            "transform_matrix",
            full_transform * glm::translate(rotator.rot_base) *
                glm::scale(rotator.rot_end - rotator.rot_base));
        cube_program_->uniform("line_color", glm::vec4(1.0f, 1.0f, 0.5f, 0.5f));
        glBindVertexArray(line_vao_handle_);
        glLineWidth(5.0f);
        glDrawArrays(GL_LINES, 0, 2);
    }

    glDisable(GL_BLEND);
}

bool ReconstructionComponent::handle_mouse_button(int button, bool down) {
    if (!show_) {
        return false;
    }

    if (down) {
        if (button == 0) {
            if (hovering_) {
                switch_if_necessary(recon_drag_machine_kind::translator);
                dragging_ = true;
                return true;
            }
        }
        if (button == 1) {
            if (hovering_) {
                switch_if_necessary(recon_drag_machine_kind::rotator);
                dragging_ = true;
                return true;
            }
        }
        if (button == 2) {
            if (hovering_) {
                // hovered over slice gets fixed
                int new_id = generate_slice_idx();
                auto new_slice = std::make_unique<slice>(new_id);
                new_slice->orientation = hovered_slice_->orientation;

                auto packet = SetSlicePacket(scene_id_, new_slice->id,
                    new_slice->packed_orientation());
                object_.send(packet);

                fixed_slices_[new_slice->id] = std::move(new_slice);
            }
        }
    }

    if (!down) {
        if (dragged_slice_) {
            auto packet = SetSlicePacket(scene_id_, dragged_slice_->id,
                                         dragged_slice_->packed_orientation());
            object_.send(packet);

            dragged_slice_ = nullptr;
            dragging_ = false;
            drag_machine_ = nullptr;
            return true;
        }
        dragging_ = false;
        drag_machine_ = nullptr;
    }

    return false;
}

bool ReconstructionComponent::handle_mouse_moved(float x, float y) {
    if (!show_) {
        return false;
    }

    // update slice that is being hovered over
    y = -y;

    if (prev_y_ < -1.0) {
        prev_x_ = x;
        prev_y_ = y;
    }

    glm::vec2 delta(x - prev_x_, y - prev_y_);
    prev_x_ = x;
    prev_y_ = y;

    // TODO: fix for screen ratio ratio
    if (dragging_) {
        drag_machine_->on_drag(delta);
        return true;
    } else {
        check_hovered(x, y);
    }

    return false;
}

std::tuple<bool, float, glm::vec3> ReconstructionComponent::intersection_point(
    glm::mat4 inv_matrix, glm::mat4 orientation, glm::vec2 point) {
    auto intersect_ray_plane = [](glm::vec3 origin, glm::vec3 direction,
                                  glm::vec3 base, glm::vec3 normal,
                                  float& distance) -> bool {
        auto alpha = glm::dot(normal, direction);
        if (glm::abs(alpha) > 0.001f) {
            distance = glm::dot((base - origin), normal) / alpha;
            if (distance >= 0.001f)
                return true;
        }
        return false;
    };

    // how do we want to do this
    // end points of plane/line?
    // first see where the end
    // points of the square end up
    // within the box.
    // in world space:
    auto o = orientation;
    auto axis1 = glm::vec3(o[0][0], o[0][1], o[0][2]);
    auto axis2 = glm::vec3(o[1][0], o[1][1], o[1][2]);
    auto base = glm::vec3(o[2][0], o[2][1], o[2][2]);
    base += 0.5f * (axis1 + axis2);
    auto normal = glm::normalize(glm::cross(axis1, axis2));
    float distance = -1.0f;

    auto from = inv_matrix * glm::vec4(point.x, point.y, -1.0f, 1.0f);
    from /= from[3];
    auto to = inv_matrix * glm::vec4(point.x, point.y, 1.0f, 1.0f);
    to /= to[3];
    auto direction = glm::normalize(glm::vec3(to) - glm::vec3(from));

    bool does_intersect =
        intersect_ray_plane(glm::vec3(from), direction, base, normal, distance);

    // now check if the actual point is inside the plane
    auto intersection_point = glm::vec3(from) + direction * distance;
    intersection_point -= base;
    auto along_1 = glm::dot(intersection_point, glm::normalize(axis1));
    auto along_2 = glm::dot(intersection_point, glm::normalize(axis2));
    if (glm::abs(along_1) > 0.5f * glm::length(axis1) ||
        glm::abs(along_2) > 0.5f * glm::length(axis2)) {
        does_intersect = false;
    }

    return std::make_tuple(does_intersect, distance, intersection_point);
}

int ReconstructionComponent::index_hovering_over(float x, float y) {
    auto inv_matrix =
        glm::inverse(object_.camera().matrix() * volume_transform_);
    int best_slice_index = -1;
    float best_z = std::numeric_limits<float>::max();
    for (auto& id_slice : slices_) {
        auto& slice = id_slice.second;
        if (slice->inactive) {
            continue;
        }
        slice->hovered = false;
        auto maybe_point =
            intersection_point(inv_matrix, slice->orientation, glm::vec2(x, y));
        if (std::get<0>(maybe_point)) {
            auto z = std::get<1>(maybe_point);
            if (z < best_z) {
                best_z = z;
                best_slice_index = id_slice.first;
            }
        }
    }

    return best_slice_index;
}

void ReconstructionComponent::check_hovered(float x, float y) {
    int best_slice_index = index_hovering_over(x, y);

    if (best_slice_index >= 0) {
        slices_[best_slice_index]->hovered = true;
        hovering_ = true;
        hovered_slice_ = slices_[best_slice_index].get();
    } else {
        hovering_ = false;
        hovered_slice_ = nullptr;
    }
}

void ReconstructionComponent::switch_if_necessary(
    recon_drag_machine_kind kind) {
    if (!drag_machine_ || drag_machine_->kind() != kind) {
        switch (kind) {
        case recon_drag_machine_kind::translator:
            drag_machine_ = std::make_unique<SliceTranslator>(
                *this, glm::vec2{prev_x_, prev_y_});
            break;
        case recon_drag_machine_kind::rotator:
            drag_machine_ = std::make_unique<SliceRotator>(
                *this, glm::vec2{prev_x_, prev_y_});
            break;
        default:
            break;
        }
    }
}

void SliceTranslator::on_drag(glm::vec2 delta) {
    // 1) what are we dragging, and does it have data?
    // if it does then we need to make a new slice
    // else we drag the current slice along the normal
    if (!comp_.dragged_slice()) {
        std::unique_ptr<slice> new_slice;
        int id = comp_.generate_slice_idx();
        int to_remove = -1;
        for (auto& id_the_slice : comp_.get_slices()) {
            auto& the_slice = id_the_slice.second;
            if (the_slice->hovered) {
                if (the_slice->has_data()) {
                    new_slice = std::make_unique<slice>(id);
                    new_slice->orientation = the_slice->orientation;
                    to_remove = the_slice->id;
                    // FIXME need to generate a new id and upon 'popping'
                    // send a UpdateSlice packet
                    comp_.dragged_slice() = new_slice.get();
                } else {
                    comp_.dragged_slice() = the_slice.get();
                }
                break;
            }
        }
        if (new_slice) {
            comp_.get_slices()[new_slice->id] = std::move(new_slice);
        }
        if (to_remove >= 0) {
            comp_.get_slices().erase(to_remove);
            // send slice packet
            auto packet = RemoveSlicePacket(comp_.scene_id(), to_remove);
            comp_.object().send(packet);
        }
        if (!comp_.dragged_slice()) {
            std::cout << "WARNING: No dragged slice found." << std::endl;
            return;
        }
    }

    auto slice = comp_.dragged_slice();
    auto& o = slice->orientation;

    auto axis1 = glm::vec3(o[0][0], o[0][1], o[0][2]);
    auto axis2 = glm::vec3(o[1][0], o[1][1], o[1][2]);
    auto normal = glm::normalize(glm::cross(axis1, axis2));

    // project the normal vector to screen coordinates
    // FIXME maybe need window matrix here too which would be kind of
    // painful maybe
    auto base_point_normal =
        glm::vec3(o[2][0], o[2][1], o[2][2]) + 0.5f * (axis1 + axis2);
    auto end_point_normal = base_point_normal + normal;

    auto a = comp_.object().camera().matrix() * comp_.volume_transform() *
             glm::vec4(base_point_normal, 1.0f);
    auto b = comp_.object().camera().matrix() * comp_.volume_transform() *
             glm::vec4(end_point_normal, 1.0f);
    auto normal_delta = b - a;
    float difference =
        glm::dot(glm::vec2(normal_delta.x, normal_delta.y), delta);

    // take the inner product of delta x and this normal vector

    auto dx = difference * normal;
    // FIXME check if it is still inside the bounding box of the volume
    // probably by checking all four corners are inside bounding box, should
    // define this box somewhere
    o[2][0] += dx[0];
    o[2][1] += dx[1];
    o[2][2] += dx[2];
}

SliceRotator::SliceRotator(ReconstructionComponent& comp, glm::vec2 initial)
    : ReconDragMachine(comp, initial) {
    // 1. need to identify the opposite axis
    // a) get the position within the slice
    auto tf = comp.object().camera().matrix() * comp.volume_transform();
    auto inv_matrix = glm::inverse(tf);

    auto slice = comp.hovered_slice();
    assert(slice);
    auto o = slice->orientation;

    auto maybe_point = comp.intersection_point(inv_matrix, o, initial_);
    assert(std::get<0>(maybe_point));

    auto axis1 = glm::vec3(o[0][0], o[0][1], o[0][2]);
    auto axis2 = glm::vec3(o[1][0], o[1][1], o[1][2]);
    auto base = glm::vec3(o[2][0], o[2][1], o[2][2]);

    auto in_world = std::get<2>(maybe_point);
    auto rel = in_world - base;

    auto x = 0.5f * glm::dot(rel, axis1) - 1.0f;
    auto y = 0.5f * glm::dot(rel, axis2) - 1.0f;

    // 2. need to rotate around that at on drag
    auto other = glm::vec3();
    if (glm::abs(x) > glm::abs(y)) {
        if (x > 0.0f) {
            rot_base = base;
            rot_end = rot_base + axis2;
            other = axis1;
        } else {
            rot_base = base + axis1;
            rot_end = rot_base + axis2;
            other = -axis1;
        }
    } else {
        if (y > 0.0f) {
            rot_base = base;
            rot_end = rot_base + axis1;
            other = axis2;
        } else {
            rot_base = base + axis2;
            rot_end = rot_base + axis1;
            other = -axis2;
        }
    }

    auto center = 0.5f * (rot_end + rot_base);
    auto opposite_center = 0.5f * (rot_end + rot_base) + other;
    auto from = tf * glm::vec4(glm::rotate(rot_base - center,
                                           glm::half_pi<float>(), other) +
                                   opposite_center,
                               1.0f);
    auto to = tf * glm::vec4(glm::rotate(rot_end - center,
                                         glm::half_pi<float>(), other) +
                                 opposite_center,
                             1.0f);

    screen_direction = glm::normalize(from - to);
}

void SliceRotator::on_drag(glm::vec2 delta) {
    // 1) what are we dragging, and does it have data?
    // if it does then we need to make a new slice
    // else we drag the current slice along the normal
    if (!comp_.dragged_slice()) {
        std::unique_ptr<slice> new_slice;
        int id = comp_.generate_slice_idx();
        int to_remove = -1;
        for (auto& id_the_slice : comp_.get_slices()) {
            auto& the_slice = id_the_slice.second;
            if (the_slice->hovered) {
                if (the_slice->has_data()) {
                    new_slice = std::make_unique<slice>(id);
                    new_slice->orientation = the_slice->orientation;
                    to_remove = the_slice->id;
                    // FIXME need to generate a new id and upon 'popping'
                    // send a UpdateSlice packet
                    comp_.dragged_slice() = new_slice.get();
                } else {
                    comp_.dragged_slice() = the_slice.get();
                }
                break;
            }
        }
        if (new_slice) {
            comp_.get_slices()[new_slice->id] = std::move(new_slice);
        }
        if (to_remove >= 0) {
            comp_.get_slices().erase(to_remove);
            // send slice packet
            auto packet = RemoveSlicePacket(comp_.scene_id(), to_remove);
            comp_.object().send(packet);
        }
        assert(comp_.dragged_slice());
    }

    auto slice = comp_.dragged_slice();
    auto& o = slice->orientation;

    auto axis1 = glm::vec3(o[0][0], o[0][1], o[0][2]);
    auto axis2 = glm::vec3(o[1][0], o[1][1], o[1][2]);
    auto base = glm::vec3(o[2][0], o[2][1], o[2][2]);

    auto a = base - rot_base;
    auto b = base + axis1 - rot_base;
    auto c = base + axis2 - rot_base;

    auto weight = glm::dot(delta, screen_direction);
    a = glm::rotate(a, weight, rot_end - rot_base) + rot_base;
    b = glm::rotate(b, weight, rot_end - rot_base) + rot_base;
    c = glm::rotate(c, weight, rot_end - rot_base) + rot_base;

    slice->set_orientation(a, b - a, c - a);
}

} // namespace tomovis
